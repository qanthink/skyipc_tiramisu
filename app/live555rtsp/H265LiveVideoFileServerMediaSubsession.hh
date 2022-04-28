#ifndef _H265_LIVE_VIDEO_SERVER_MEDIA_SUBSESSION_HH
#define _H265_LIVE_VIDEO_SERVER_MEDIA_SUBSESSION_HH

#include "H265VideoFileServerMediaSubsession.hh"
#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"

class H265VideoLiveServerMediaSubssion : public H265VideoFileServerMediaSubsession{
public:
	static H265VideoLiveServerMediaSubssion* createNew(UsageEnvironment& env, Boolean reuseFirstSource);

protected:
	H265VideoLiveServerMediaSubssion(UsageEnvironment& env, Boolean reuseFirstSource);
	~H265VideoLiveServerMediaSubssion();

protected:
	FramedSource* createNewStreamSource(unsigned clientSessionId, unsigned &estBitrate);
public:
	//char fFileName[128];	// 必须定义，构造H265VideoLiveServerMediaSubssion 时会用到
	//bool *pServerDosent;	// 发送标示
};
#endif
