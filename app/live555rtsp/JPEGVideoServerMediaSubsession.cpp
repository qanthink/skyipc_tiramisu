/*---------------------------------------------------------------- 
xxx 版权所有。
作者：
时间：2023.7.7
----------------------------------------------------------------*/

#include "JPEGVideoServerMediaSubsession.hh"
#include "JPEGVideoRTPSink.hh"
#include "JPEGByteStreamFileSource.hh"
#include "JPEGVideoStreamFramer.hh"

JPEGVideoServerMediaSubsession*
JPEGVideoServerMediaSubsession::createNew(UsageEnvironment& env, 
					const char *filePath, Boolean reuseFirstSource)
{
	printf("%s", "JPEGVideoServerMediaSubsession::createNew\n");
	return new JPEGVideoServerMediaSubsession(env, reuseFirstSource, filePath);
}

JPEGVideoServerMediaSubsession::JPEGVideoServerMediaSubsession(UsageEnvironment& env,
								Boolean reuseFirstSource, const char *filePath)
	: OnDemandServerMediaSubsession(env, reuseFirstSource), 
	fAuxSDPLine(NULL),
	fDoneFlag(0),
	mFilePath(filePath)
{
}

JPEGVideoServerMediaSubsession::~JPEGVideoServerMediaSubsession()
{
	if(fAuxSDPLine)
	{
		delete[] fAuxSDPLine;
		fAuxSDPLine = NULL;
	}
}

static void afterPlayingDummy(void* clientData)
{
	JPEGVideoServerMediaSubsession* subsess = (JPEGVideoServerMediaSubsession*)clientData;
	subsess->afterPlayingDummy1();
}

void JPEGVideoServerMediaSubsession::afterPlayingDummy1()
{
	// Unschedule any pending 'checking' task:
	envir().taskScheduler().unscheduleDelayedTask(nextTask());
	// Signal the event loop that we're done:
	setDoneFlag();
}

char const* JPEGVideoServerMediaSubsession::getAuxSDPLine(RTPSink* rtpSink, FramedSource* inputSource)
{
	JPEGVideoStreamSource *fDummySource = (JPEGVideoStreamSource *)inputSource;

	if(fAuxSDPLine != NULL)
	{
		return fAuxSDPLine; // it's already been set up (for a previous client)
	}

	// we're not already setting it up for another, concurrent stream
	// Note: For H265 video files, the 'config' information (used for several payload-format
	// specific parameters in the SDP description) isn't known until we start reading the file.
	// This means that "rtpSink"s "auxSDPLine()" will be NULL initially,
	// and we need to start reading data from our file until this changes.

	// Start reading the file:
	rtpSink->startPlaying(*inputSource, afterPlayingDummy, this);

	// Check whether the sink's 'auxSDPLine()' is ready:
	// checkForAuxSDPLine(this);

#if 0	// 2024.9.28 源码；
	char const* addSDPFormat =
            "b=AS:%d\r\n"
            "a=framerate:%d\r\n"
            "a=framesize:%d %d-%d\r\n"
            ;

	unsigned addSDPFormatSize = strlen(addSDPFormat)
                            + 2 /* max char len */
                            + 4 /* max char len */
                            + 2 + 4 + 4
                            + 1;

	char* fmtp = new char[addSDPFormatSize];
	sprintf(fmtp,
		    addSDPFormat,
		    4096,
		    30,
		    rtpSink->rtpPayloadType(),
		    fDummySource->widthPixels(),
		    fDummySource->heightPixels()
	    );
#else	// 2024.9.28 意欲解决MJPEG 宽度无法超过2048的问题；
	char const* addSDPFormat = "a=x-dimensions:%d,%d\r\n";
	unsigned addSDPFormatSize = strlen(addSDPFormat) + 4 + 4 + 1;
	char* fmtp = new char[addSDPFormatSize];
	
	sprintf(fmtp, addSDPFormat,
		fDummySource->widthPixels(),
		fDummySource->heightPixels()
	);
#endif

	fAuxSDPLine = strDup(fmtp);
	delete[] fmtp;

	return fAuxSDPLine;
}

FramedSource* JPEGVideoServerMediaSubsession::createNewStreamSource(unsigned /*clientSessionId*/, unsigned& estBitrate)
{
	estBitrate = 500; // kbps, estimate

	// Create the video source:
	JpegByteStreamFileSource* fileSource = JpegByteStreamFileSource::createNew(envir(), mFilePath);
	if(fileSource == NULL)
	{
		return NULL;
	}

	// Create a framer for the Video Elementary Stream:
	return JPEGVideoStreamSource::createNew(envir(), fileSource);
}

RTPSink* JPEGVideoServerMediaSubsession::createNewRTPSink(Groupsock* rtpGroupsock,
				unsigned char rtpPayloadTypeIfDynamic, FramedSource* inputSource)
{
	return JPEGVideoRTPSink::createNew(envir(), rtpGroupsock);
}

