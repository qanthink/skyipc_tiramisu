/*---------------------------------------------------------------- 
xxx版权所有。
作者：
时间：2020.7.10
----------------------------------------------------------------*/

#pragma once

#include "H265FramedLiveSource.hh"
#include "H265LiveVideoFileServerMediaSubsession.hh"

#include <mutex>
#include <thread>
#include <condition_variable>

void *createRtspServerBy265LocalFile(void *h265FilePath);

class Live555Rtsp{
public:
	static Live555Rtsp *getInstance();
	int enable();
	int disable();

	int recvH26xFrame(unsigned char *const dataBuff, const unsigned int dataSize);
	int sendH26xFrame(const unsigned char *const dataBuff, const unsigned int dataSize);
	int sendH26xFrame_block(const unsigned char *const dataBuff, const unsigned int dataSize);
	
private:
	Live555Rtsp();
	~Live555Rtsp();
	Live555Rtsp(const Live555Rtsp&);
	Live555Rtsp& operator=(const Live555Rtsp&);

	bool bEnable;
	volatile bool bRunning = false;
	RTSPServer* rtspServer = NULL;
	
	unsigned int mDataSize = 0;
	const unsigned char *mDataBuff = NULL;

	// 类中创建线程，必须使用一个空格函数传递this 指针。
	void *routeLiveStream(void *arg);
	static void *thRouteLiveStream(void *arg);
	std::mutex mutex;
	std::condition_variable condVar;
	std::shared_ptr<std::thread> pTh = NULL;

	void announceStream(RTSPServer* rtspServer, ServerMediaSession* sms,char const* streamName);
	void doEventLoop(BasicTaskScheduler0 *Basicscheduler);
};
