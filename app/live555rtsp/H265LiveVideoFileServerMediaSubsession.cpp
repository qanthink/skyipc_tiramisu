#include "H265LiveVideoFileServerMediaSubsession.hh"
#include "H265FramedLiveSource.hh"
#include "iostream"

using namespace std;

H265VideoLiveServerMediaSubssion* H265VideoLiveServerMediaSubssion::createNew(
		UsageEnvironment& env, Boolean reuseFirstSource)
{
	cout << "Call H265VideoLiveServerMediaSubssion::createNew()" << endl;
	return new H265VideoLiveServerMediaSubssion(env, reuseFirstSource);
}

H265VideoLiveServerMediaSubssion::H265VideoLiveServerMediaSubssion(
		UsageEnvironment& env, Boolean reuseFirstSource)
: H265VideoFileServerMediaSubsession(env, NULL, reuseFirstSource)
{
	cout << "Call H265VideoLiveServerMediaSubssion::H265VideoLiveServerMediaSubssion()" << endl;
}

H265VideoLiveServerMediaSubssion::~H265VideoLiveServerMediaSubssion()
{
	cout << "Call H265VideoLiveServerMediaSubssion::~H265VideoLiveServerMediaSubssion()" << endl;
}

FramedSource* H265VideoLiveServerMediaSubssion::createNewStreamSource(
		unsigned clientSessionId, unsigned& estBitrate)
{
	cout << "Call H265VideoLiveServerMediaSubssion::createNewStreamSource()" << endl;
	estBitrate = 4000; // kbps, estimate

	//创建实时流视频源
	H265FramedLiveSource* liveSource = NULL;
	liveSource = H265FramedLiveSource::createNew(envir());
	if(NULL == liveSource)
	{
		cerr << "Fail to call H265FramedLiveSource::createNew()." << endl;
		return NULL;
	}

	return H265VideoStreamFramer::createNew(envir(), liveSource);
}

