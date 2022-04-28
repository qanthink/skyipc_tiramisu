/*---------------------------------------------------------------- 
xxx版权所有。
作者：
时间：2020.7.15
----------------------------------------------------------------*/

/*
	本文件基于live555库，开发了rtspserver.
*/

/*
	=================RTSP Server推本地流的方法=================
	第一步：
	在程序的开始处，创建任务调度器、使用环境、RTSP服务器；
	在结束处，开始任务调度。
	
	TaskScheduler *scheduler = BasicTaskScheduler::createNew();
	UsageEnvironment *env = BasicUsageEnvironment::createNew(*scheduler);
	RTSPServer *rtspServer = RTSPServer::createNew(*env, ourPort);
	...
	env->taskScheduler().doEventLoop();
	
	第二步（可选项）：
	打印服务器地址信息。
	*env << rtspServer->rtspURLPrefix();

	第三步：
	创建ServerMediaSession服务器主会话，指定会话名称，并添加视频子会话和音频子会话。
	ServerMediaSession *sms = ServerMediaSession::createNew(*env, "sessionName");
	sms->addSubsession(H265VideoFileServerMediaSubsession::createNew(*env, "test.265", reuseFirstSource));

	第四步：
	将主会话加入RTSP Server.
	rtspServer->addServerMediaSession(sms);
*/

/*
	======================= 熟悉live555的类 =======================
	参考本路径下"classOnDemandServerMediaSubsession__inherit__graph.png",
	或"http://www.live555.com/liveMedia/doxygen/html/classOnDemandServerMediaSubsession.html".
	有如下继承关系：
	Medium <-- ServerMediaSession <-- OnDemandServerMediaSubsession <-- FileServerMediaSubsession <-- H265VideoFileServerMediaSubsession
	
	重点关注 OnDemandServerMediaSubsession类，该类是ServermediaSubSession 的一个中间实现，定义了一些点播服务的特性。
	我们推文件流、实时流的方法，都要通过继承该类来实现。
	参考代码推文件流，继承该类后定义的新类是H265VideoFileServerMediaSubsession.
	我们推实时流，继承该类后定义的新类为H265VideoLiveServerMediaSubssion.
*/

/*
	学习资料：
	https://www.jianshu.com/p/60fcc41c2369
	https://blog.csdn.net/marcosun_sw/article/details/86149356
	https://www.cnblogs.com/weixinhum/p/3916676.html	最有用！！！
*/

#include "live555rtsp.h"
#include <iostream>

using namespace std;

Live555Rtsp::Live555Rtsp()
{
	enable();
	bEnable = true;
}

Live555Rtsp::~Live555Rtsp()
{
	bEnable = false;
	disable();
}

/*  ---------------------------------------------------------------------------
描--述：VENC 模块获取实例的唯一入口
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
Live555Rtsp* Live555Rtsp::getInstance()
{
	static Live555Rtsp live555Rtsp;
	return &live555Rtsp;
}

/* ---------------------------------------------------------------------------
描--述：
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
int Live555Rtsp::enable()
{
	cout << "Call Live555Rtsp::enable()." << endl;

	if(bEnable)
	{
		cerr << "Fail to call Live555Rtsp::enable(). Module has been initialized." << endl;
		return -1;
	}

	pTh = make_shared<thread>(thRouteLiveStream, this);
	bEnable = true;
	
	cout << "Call Live555Rtsp::enable() end." << endl;
	return 0;
}

/*-----------------------------------------------------------------------------
描--述：
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
int Live555Rtsp::disable()
{
	cout << "Call Live555Rtsp::disable()." << endl;

	
	bRunning = false;
	condVar.notify_all();
	if(NULL != rtspServer)
	{
		cout << "close(RTSPServer)." << endl;
		Medium::close(rtspServer);
		rtspServer = NULL;
	}
	bEnable = false;

	cout << "Call Live555Rtsp::disable() end." << endl;
	return 0;
}

/*-----------------------------------------------------------------------------
描--述：显示RTSP流信息。
参--数：rtspServer 指向RTSP服务器对象的指针；sms 指向会话的指针；streamName 流的名称
返回值：无
注--意：
-----------------------------------------------------------------------------*/
void Live555Rtsp::announceStream(RTSPServer* rtspServer, ServerMediaSession* sms,char const* streamName)
{
	char *url = rtspServer->rtspURL(sms);
	UsageEnvironment &env = rtspServer->envir();
	env << streamName << "\n";
	env << "Play this stream using the URL \"" << url << "\"\n";
	delete[] url;
}

/*-----------------------------------------------------------------------------
描--述：显示RTSP流信息。
参--数：rtspServer 指向RTSP服务器对象的指针；sms 指向会话的指针；streamName 流的名称
返回值：无
注--意：
-----------------------------------------------------------------------------*/
int Live555Rtsp::sendH26xFrame(const unsigned char *const dataBuff, const unsigned int dataSize)
{
	//cout << "Call Live555Rtsp::sendH26xFrame()." << endl;
	if(!bRunning)
	{
		return -1;
	}
	
	std::unique_lock<std::mutex> lock(mutex);
	mDataBuff = dataBuff;
	mDataSize = dataSize;
	lock.unlock();
	condVar.notify_one();

	//cout << "End of call Live555Rtsp::sendH26xFrame()." << endl;	
	return 0;
}

/*-----------------------------------------------------------------------------
描--述：显示RTSP流信息。
参--数：rtspServer 指向RTSP服务器对象的指针；sms 指向会话的指针；streamName 流的名称
返回值：无
注--意：
-----------------------------------------------------------------------------*/
int Live555Rtsp::sendH26xFrame_block(const unsigned char *const dataBuff, const unsigned int dataSize)
{
	//cout << "Call Live555Rtsp::sendH26xFrame()." << endl;
	if(!bRunning)
	{
		return -1;
	}

	std::unique_lock<std::mutex> lock(mutex);
	while(bRunning && 0 != mDataSize)
	{
		condVar.wait(lock);
	}
	mDataBuff = dataBuff;
	mDataSize = dataSize;
	lock.unlock();
	condVar.notify_one();

	//cout << "End of call Live555Rtsp::sendH26xFrame()." << endl;	
	return 0;
}

/*-----------------------------------------------------------------------------
描--述：live555Rtsp 获取H.26X 数据。
参--数：
返回值：无
注--意：
-----------------------------------------------------------------------------*/
int Live555Rtsp::recvH26xFrame(unsigned char *const dataBuff, const unsigned int dataSize)
{
	//cout << "Call Live555Rtsp::recvH26xFrame()." << endl;
	std::unique_lock<std::mutex> lock(mutex);
	while(bRunning && 0 == mDataSize)
	{
		condVar.wait(lock);
	}
	
	int realSize = 0;
	realSize = (dataSize > mDataSize) ? mDataSize : dataSize;
	memcpy(dataBuff, mDataBuff, realSize);
	mDataSize = 0;
	lock.unlock();

	//cout << "End of call Live555Rtsp::recvH26xFrame()." << endl;	
	return realSize;
}

/*-----------------------------------------------------------------------------
描--述：创建RTSP服务器推本地H265流。
参--数：h265FilePath 本地265文件路径。
返回值：无
注--意：
-----------------------------------------------------------------------------*/
void *createRtspServerBy265LocalFile(void *h265FilePath)
{
	OutPacketBuffer::maxSize = 2880000;	// for debug
	
	// step1: 创建任务调度器、使用环境、RTSP服务器。
	TaskScheduler *scheduler = BasicTaskScheduler::createNew();
	UsageEnvironment *env = BasicUsageEnvironment::createNew(*scheduler);
	Port ourPort = 544;
	RTSPServer *rtspServer = RTSPServer::createNew(*env, ourPort);

	// step2: 打印Server IP address.
	*env << "RTSP Address:\n"
		<< rtspServer->rtspURLPrefix() << "sessionName\n";
	
	// step3: 创建服务器会话，指定推流的文件，并添加视频子会话。
	Boolean reuseFirstSource = false;
	ServerMediaSession *sms = ServerMediaSession::createNew(*env, "localfile");	// 注意第二个参数"localfile"不要用大写字母。
	sms->addSubsession(H265VideoFileServerMediaSubsession::createNew(*env, (const char *)h265FilePath, reuseFirstSource));

	// step4: 将会话加入RTPS Server.
	rtspServer->addServerMediaSession(sms);
	cout << "File name = " << (const char *)h265FilePath << endl;
	//announceStream(rtspServer, sms, main_stream);
	env->taskScheduler().doEventLoop();
	return (void *)0;
}

/*-----------------------------------------------------------------------------
描--述：
参--数：
返回值：无
注--意：
-----------------------------------------------------------------------------*/
void *Live555Rtsp::thRouteLiveStream(void *arg)
{
	Live555Rtsp *pThis = (Live555Rtsp *)arg;
	return pThis->routeLiveStream(NULL);
}

/*-----------------------------------------------------------------------------
描--述：
参--数：
返回值：无
注--意：
-----------------------------------------------------------------------------*/
void *Live555Rtsp::routeLiveStream(void *arg)
{
	bRunning = true;

	cout << "Call Live555Rtsp::routeLiveStream()." << endl;
	OutPacketBuffer::maxSize = 128 * 1024;	// for debug
	
	// 循环事件中需要使用BasicTaskScheduler0 类型的调度对象。
	BasicTaskScheduler0* basicscheduler = NULL;
	basicscheduler = BasicTaskScheduler::createNew();

	// 创建RTSP服务器的过程中使用的是TaskScheduler 类型的任务调度对象。故而需要类型转换。
	TaskScheduler* scheduler = NULL;
	scheduler = basicscheduler;

	// 创建RTSP环境。
	UsageEnvironment* env = NULL;
	env = BasicUsageEnvironment::createNew(*scheduler);

	// 设置端口号
	Port ourPort = 554;		// Port为0意味着让RTSP服务器自己选择端口。
	UserAuthenticationDatabase* authDB = NULL;	// 无认证。
	rtspServer = RTSPServer::createNew(*env, ourPort, authDB);
	if(NULL == rtspServer)
	{
		*env << "Failed to create RTSP server: " << env->getResultMsg() << "\n";
		return (void *)-1;
	}

	// 建立名为streamName的RTSP流。
	char const* streamName = "livestream";
	ServerMediaSession* sms = ServerMediaSession::createNew(*env, streamName);

	// 添加会话。
	Boolean reuseFirstSource = true;
	sms->addSubsession(H265VideoLiveServerMediaSubssion::createNew(*env, reuseFirstSource));
	rtspServer->addServerMediaSession(sms);

	// 打印RTSP服务器信息，流信息；然后执行循环事件。
	announceStream(rtspServer, sms, streamName);
	doEventLoop(basicscheduler);

	cout << "Call Live555Rtsp::routeLiveStream() end." << endl;
	return (void *)0;
}

/*-----------------------------------------------------------------------------
描--述：
参--数：
返回值：无
注--意：
-----------------------------------------------------------------------------*/
void Live555Rtsp::doEventLoop(BasicTaskScheduler0 *Basicscheduler)
{
	bRunning = true;
	while(bRunning)
	{
		Basicscheduler->SingleStep();
	}
}

