/*---------------------------------------------------------------- 
xxx版权所有。
作者：
时间：2020.7.15
----------------------------------------------------------------*/

/*
	本文件基于live555库，开发了rtspserver.
*/

#include "live555rtsp.h"
#include <iostream>
#include <thread>

#include "../system/venc/venc.h"
#include "JPEGVideoServerMediaSubsession.hh"

using namespace std;

void *openStream(char const *szStreamName)
{
	cout <<"openStream" << endl;
	return NULL;
}

int closeStream(void *arg)
{
	cout <<"closeStream" << endl;
	return 0;
}

int videoReadStream(unsigned char *ucpBuf, int BufLen, struct timeval *p_Timestamp)
{
	Venc *pVenc = Venc::getInstance();
	MI_VENC_Stream_t stStream;
	int s32Ret = 0;
	memset(&stStream, 0, sizeof(MI_VENC_Stream_t));
	pVenc->rcvStream(MI_VENC_DEV_ID_H264_H265_0, Venc::vencJpegChn, &stStream);
	if(0 != s32Ret)
	{
		cerr << "Fail to call pVenc->rcvStream(). s32Ret = " << s32Ret << endl;
		return 0;
	}
	if(0 == stStream.u32PackCount)
	{
		cerr << "Fail to call pVenc->rcvStream(). u32PackCount = " << stStream.u32PackCount << endl;
		return 0;
	}
	
	memcpy((char *)ucpBuf, stStream.pstPack[0].pu8Addr, stStream.pstPack[0].u32Len);
	s32Ret = MI_VENC_ReleaseStream(0, Venc::vencJpegChn, &stStream);
	//cout << s32Ret << endl;
	
	return stStream.pstPack[0].u32Len;
}

using namespace std;

/*-----------------------------------------------------------------------------
描--述：打印流信息。
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
static void announceStream(RTSPServer* pRtspServer, ServerMediaSession* sms, 
								char const* streamName, char const* inputFilePath)
{
	char *url = pRtspServer->rtspURL(sms);
	UsageEnvironment& mEnv = pRtspServer->envir();
	mEnv << "\n\"" << streamName << "\" stream, from the file \"" << inputFilePath << "\"\n";
	mEnv << "Play this stream using the URL \"" << url << "\"\n";
	delete[] url;
}

/*-----------------------------------------------------------------------------
描--述：构造Live555RTSP服务器
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
Live555Rtsp::Live555Rtsp()
{
	cout << "Call Live555Rtsp::Live555Rtsp()." << endl;
	mScheduler = BasicTaskScheduler::createNew();
	mEnv = BasicUsageEnvironment::createNew(*mScheduler);

	UserAuthenticationDatabase* authDB = NULL;
	#if 0
	// To implement client access control to the RTSP server, do the following:
	authDB = new UserAuthenticationDatabase;
	authDB->addUserRecord("username1", "password1"); // replace these with real strings
	// Repeat the above with each <username>, <password> that you wish to allow access to the server.
	#endif

	pRtspServer = RTSPServer::createNew(*mEnv, mPort, authDB);
	if(NULL == pRtspServer)
	{
		*mEnv << "Failed to create RTSP server: " << mEnv->getResultMsg() << "\0";
		return;
	}

	#if 0
	const char *descString = "Session streamed by \"testOnDemandRTSPServer\"";

	// Set up each of the possible streams that can be served by the
	// RTSP server.  Each such stream is implemented using a
	// "ServerMediaSession" object, plus one or more
	// "ServerMediaSubsession" objects for each audio/video substream.

	// A H.264 video elementary stream:
	const char *streamName = "stream";
	ServerMediaSession* sms = ServerMediaSession::createNew(*mEnv, streamName, streamName, descString);
	
	Boolean reuseFirstSource = false;
	sms->addSubsession(H265VideoFileServerMediaSubsession::createNew(*mEnv, filePath, reuseFirstSource));
	pRtspServer->addServerMediaSession(sms);
	announceStream(pRtspServer, sms, streamName, filePaht);
	
	mEnv->taskScheduler().doEventLoop(); // does not return
	#endif

	cout << "Call Live555Rtsp::Live555Rtsp() end." << endl;
}

/*-----------------------------------------------------------------------------
描--述：析构Live555RTSP服务器
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
Live555Rtsp::~Live555Rtsp()
{
	cout << "Call Live555Rtsp::~Live555Rtsp()." << endl;

	auto it = streamMap.begin();
	for(it = streamMap.begin(); streamMap.end() != it; ++it)
	{
		cout << "Remove " << it->first;
		//it->second->deleteAllSubsessions();
		//pRtspServer->deleteServerMediaSession(it->first.c_str());
		pRtspServer->deleteServerMediaSession(it->second);
		streamMap.erase(it);
	}
	#if 1

	if(NULL != pRtspServer)
	{
		mEnv = NULL;
		//sleep(0.125);
		//sleep(0.625);
		mScheduler = NULL;
		pRtspServer = NULL;
	}
	#endif
	cout << "Call Live555Rtsp::~Live555Rtsp() end." << endl;
}

/*-----------------------------------------------------------------------------
描--述：增加服务器子会话
参--数：filePath, 本地文件名；streamName, 网络流的名字; emEncType, 编码类型26x
返回值：成功，返回0. 失败，返回错误码：
        -1, 入参错误；
        -2, RTSP 服务器未建立。
        -3, 编码类型未识别。
        -4, 会话已存在。
注--意：
-----------------------------------------------------------------------------*/
int Live555Rtsp::addStream(const char *filePath, const char *streamName, emEncType_t emEncType)
{
	cout << "Call Live555Rtsp::addStream()." << endl;
	if(NULL == filePath || NULL == streamName)
	{
		cerr << "Fail to call Live555Rtsp::addStream(). Argument has null value." << endl;
		return -1;
	}

	if(NULL == mEnv || NULL == mScheduler || NULL == pRtspServer)
	{
		cerr << "Fail to call Live555Rtsp::addStream(). Rtsp server is not created." << endl;
		return -2;
	}

	const char *descString = "rtsp description string";	// 不清楚干嘛的。
	
	// Set up each of the possible streams that can be served by the
	// RTSP server.  Each such stream is implemented using a
	// "ServerMediaSession" object, plus one or more
	// "ServerMediaSubsession" objects for each audio/video substream.

	/* 超大I 帧Size. 解决 The input frame data was too large for our buffer size (60972). */
	OutPacketBuffer::maxSize = 512 * 1024;

	#if 0
	// 创建媒体会话。
	ServerMediaSession* sms = ServerMediaSession::createNew(*mEnv, streamName, streamName, descString);

	// 增加子会话。
	Boolean reuseFirstSource = False;
	if(emEncTypeH264 == emEncType)
	{
		sms->addSubsession(H264VideoFileServerMediaSubsession::createNew(*mEnv, filePath, reuseFirstSource));
	}
	else if(emEncTypeH265 == emEncType)
	{
		sms->addSubsession(H265VideoFileServerMediaSubsession::createNew(*mEnv, filePath, reuseFirstSource));
	}
	else if(emEncTypeJpeg == emEncType)
	{
		sms->addSubsession(JPEGVideoServerMediaSubsession::createNew(*mEnv, filePath, reuseFirstSource));
	}
	else
	{
		cerr << "In Live555Rtsp::addStream(), can not recognize emEncType." << endl;
		return -3;
	}

	// 讲子会话加入到RTSP 服务器。
	pRtspServer->addServerMediaSession(sms);

	// 打印流信息。
	announceStream(pRtspServer, sms, streamName, (const char *)filePath);
	#else
	// 创建媒体会话。
	ServerMediaSession* sms = ServerMediaSession::createNew(*mEnv, streamName, streamName, descString);
	// 增加子会话。
	Boolean reuseFirstSource = False;
	if(emEncTypeH264 == emEncType)
	{
		sms->addSubsession(H264VideoFileServerMediaSubsession::createNew(*mEnv, filePath, reuseFirstSource));
	}
	else if(emEncTypeH265 == emEncType)
	{
		sms->addSubsession(H265VideoFileServerMediaSubsession::createNew(*mEnv, filePath, reuseFirstSource));
	}
	else if(emEncTypeJpeg == emEncType)
	{
		sms->addSubsession(JPEGVideoServerMediaSubsession::createNew(*mEnv, filePath, reuseFirstSource));
	}
	else
	{
		cerr << "In Live555Rtsp::addStream(), can not recognize emEncType." << endl;
		return -3;
	}

	//int i = 0;
	auto streamPair = make_pair(streamName, sms);
	pair<map<string, ServerMediaSession*>::iterator, bool> ret;
	ret = streamMap.insert(streamPair);
	if(false == ret.second)
	{
		cerr << "Fail to call insert() in Live555Rtsp::addStream(). ServerMediaSession exist." << endl;
		return -4;
	}

	// 讲子会话加入到RTSP 服务器。
	pRtspServer->addServerMediaSession(sms);

	// 打印流信息。
	announceStream(pRtspServer, sms, streamName, (const char *)filePath);

	#endif

	// 进入循环事件。
	//mEnv->taskScheduler().doEventLoop();	// no return;

	#if 1
	// Lambda表达式，捕捉值[=]。详见：https://blog.csdn.net/zhang_si_hang/article/details/127117260
	//thread th([=](){sessionLoopRoute(filePath, streamName);});
	//th.detach();
	#endif
	
	cout << "Call Live555Rtsp::addStream() end." << endl;
	return 0;
}

/*-----------------------------------------------------------------------------
描--述：删除服务器子会话
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
int Live555Rtsp::removeStream(const char *streamName)
{
	cout << "Call Live555Rtsp::removeStream()." << endl;
	pRtspServer->deleteServerMediaSession(streamName);
	streamMap.erase(streamName);
	cout << "Call Live555Rtsp::removeStream() end." << endl;
	return 0;
}

/*-----------------------------------------------------------------------------
描--述：服务器子会话线程
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
void Live555Rtsp::eventLoop()
{
	cout << "Call Live555Rtsp::eventLoop()." << endl;

	#if 0
	const char *descString = "Session streamed by \"testOnDemandRTSPServer\"";	// 不清楚干嘛的。

	// Set up each of the possible streams that can be served by the
	// RTSP server.  Each such stream is implemented using a
	// "ServerMediaSession" object, plus one or more
	// "ServerMediaSubsession" objects for each audio/video substream.

	// 创建媒体会话。
	ServerMediaSession* sms = ServerMediaSession::createNew(*mEnv, streamName, streamName, descString);

	// 增加子会话。
	Boolean reuseFirstSource = False;
	sms->addSubsession(H265VideoFileServerMediaSubsession::createNew(*mEnv, filePath, reuseFirstSource));

	// 讲子会话加入到RTSP 服务器。
	pRtspServer->addServerMediaSession(sms);

	// 打印流信息。
	announceStream(pRtspServer, sms, streamName, filePath);

	// 进入循环事件。
	//mEnv->taskScheduler().doEventLoop();	// no return;
	#else
	if(NULL == mEnv || NULL == mScheduler || NULL == pRtspServer)
	{
		cerr << "Fail to call Live555Rtsp::eventLoop(). Rtsp server is not created." << endl;
		return;
	}
	else
	{
		mEnv->taskScheduler().doEventLoop();	// no return;
	}
	#endif
	
	cout << "Call Live555Rtsp::eventLoop() end." << endl;
	return;
}

