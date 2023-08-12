/*---------------------------------------------------------------- 
xxx 版权所有。
作者：
时间：2023.7.7
----------------------------------------------------------------*/

#pragma once

#include "OnDemandServerMediaSubsession.hh"

class JPEGVideoServerMediaSubsession: public OnDemandServerMediaSubsession{
public:
	static JPEGVideoServerMediaSubsession*createNew(UsageEnvironment& env,
					const char *filePath, Boolean reuseFirstSource = true);

	// Used to implement "getAuxSDPLine()":
	// void checkForAuxSDPLine1();
	void afterPlayingDummy1();

protected:
	// called only by createNew();
	JPEGVideoServerMediaSubsession(UsageEnvironment& env,
					Boolean reuseFirstSource, const char *filePath);
	virtual ~JPEGVideoServerMediaSubsession();

	void setDoneFlag(){fDoneFlag = ~0;}

protected: // redefined virtual functions
	virtual char const* getAuxSDPLine(RTPSink* rtpSink, FramedSource* inputSource);
	virtual FramedSource* createNewStreamSource(unsigned clientSessionId, unsigned& estBitrate);
	virtual RTPSink* createNewRTPSink(Groupsock* rtpGroupsock, 
		unsigned char rtpPayloadTypeIfDynamic, FramedSource* inputSource);

private:
	char* fAuxSDPLine;
	char fDoneFlag; // used when setting up "fAuxSDPLine"
	const char* mFilePath = NULL;
};

