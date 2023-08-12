/*---------------------------------------------------------------- 
xxx版权所有。
作者：
时间：2020.7.10
----------------------------------------------------------------*/

/*
2023.06.11  不清楚怎么退出，故而计划用New 新建RTSP 服务器用Delete 删除。
*/

#pragma once

#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"

#include <map>
#include <string>

typedef enum{
	emEncTypeInvalid = 0x00,
	emEncTypeH264,
	emEncTypeH265,
	emEncTypeJpeg,
}emEncType_t;

class Live555Rtsp{
public:
	Live555Rtsp();
	~Live555Rtsp();

	int addStream(const char *filePath, const char *streamName, emEncType_t emEncType);
	int removeStream(const char *streamName);
	void eventLoop();

private:
	UsageEnvironment* mEnv = NULL;
	TaskScheduler* mScheduler = NULL;
	RTSPServer *pRtspServer = NULL;
	const unsigned int mPort = 554;

	std::map<std::string, ServerMediaSession*> streamMap;
};

int testMJPEG(int argc, char *argv);

