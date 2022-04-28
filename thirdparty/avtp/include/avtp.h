#pragma once
#include <unistd.h>
#include "udper.h"
#include "memorypool.h"

#define Mind_SDK_VERSION "Mind_SDK_VERSION_2020-08-06-15-00"

#ifndef DE_LocalPort
#define DE_LocalPort 10086
#endif // !DE_LocalPort

#ifndef DE_SendListMaxSize
#define DE_SendListMaxSize 10
#endif // !DE_SendListMaxSize

#ifndef DE_RecvListMaxSize
#define DE_RecvListMaxSize 8
#endif // !DE_RecvListMaxSize


typedef unsigned char(*BufAr)[1380];//指向buf的分包指针，每个分包数据长度最大是1380

struct VideoFrame
{
	unsigned short frameNum = 0;
	int  bufLen = 0;
	int  dataLen = 0;
	bool isIFrame = false;
	unsigned long long  pts;//时间戳
	unsigned char* buf = nullptr;
	BufAr bufAr = nullptr;
	VideoFrame(int size);
	VideoFrame();
	~VideoFrame();
	VideoFrame(const VideoFrame& frame);
	void setFrame(const unsigned char* data, int len, unsigned short frameNum);
	const VideoFrame& operator=(const VideoFrame& frame);
};


class Mindsdk_Video
{
	friend Udper;
	friend void resendThread();
	Mindsdk_Video();
	static bool running;
	static bool chnRunning[DE_NvrViChnNum];
	static Udper* videoUdper;
	static unsigned short nextFrameNum[DE_NvrViChnNum];
	static mutex videoFramMapMutex[DE_NvrViChnNum];
	static int videoFramMapMaxSize;//视频帧map缓冲的最大长度
	static map<unsigned short, shared_ptr<VideoFrame>> videoFramMap[DE_NvrViChnNum];//key 为帧编号
	static bool theFrameFinish[DE_NvrViChnNum];//true表示当前帧发送完毕
	static unsigned short sendingFrameNum[DE_NvrViChnNum];//正在发送的帧编号
	static mutex sendingMutex[DE_NvrViChnNum];
	static map<unsigned short, shared_ptr<vector<unsigned char>>> sendingMap[DE_NvrViChnNum];//待发送map，key为帧的分包编号
	static bool getResendPackage[DE_NvrViChnNum];//接收到对方的接收反馈，设为true，来让发送线程确认要不要全部重发
	static void deleteViFrame(unsigned short frameNum, unsigned char chn);
	static void cleanViFrame(unsigned char chn);
	//发送线程发送完一帧后执行此函数获取新的数据帧
	static shared_ptr<VideoFrame> getNextViFrame(unsigned char chn);
	static void cleanSendMap(unsigned char chn);
	//插入一帧数据，并将数据按包分拆，拆入到待发送map里头
	static void setSendingMap(VideoFrame& videoFrame, unsigned char chn);
	//发送线程，从videoFramMap拿一帧分包插入到sendingMap中后逐包发送，
	//接收线程接收到重发请求后插入到resendList，如果发送线程超时后还发现resendList没有
	//重发请求，又没有收到已经接收完毕的数据包，那就重发所有数据包
	static void sendFrameThread(unsigned char chn);
	//回调函数
	static void startVideo(const unsigned char* data, int len);
	//回调函数
	static void stopVideo(const unsigned char* data, int len);
	//回调函数
	static void resendVideo(unsigned short frameNum, unsigned short serialNum, unsigned char chn);//serialNum为-1表示该帧发送给完毕
	static void changeBit(uint32_t bitrate, unsigned char chn);//动态修改帧率
	//发送一个chn的vi数据
	static void sendChnFrame(const unsigned char chn);
public:
	static void setvVdeoFramMapMaxSize(int maxSize);
	static int getVideoFramMapSize(unsigned char chn = 0);
public:
	///////////以下函数、回调函数，是用户需要调用或设置的 
	static Mindsdk_Video* getInstance();
	static void start();//需要调用此函数启动
	static void(*cb_startVideo)(unsigned char chn);//用户需自己实现的开始播放视频的回调函数，chn参数是要开启的vi通道号
	static void(*cb_stopVideo)(unsigned char chn);//用户需要自己实现的停止播放视频的回调函数，chn参数是要停止的vi通道号
	static void (*cb_getNextViFrame)(unsigned char chn);//用户需要自己实现的获取下一帧的回调函数，读取数据帧后，需要调用insertViFrame插入进来，chn参数是要读取数据的vi通道号
	static void insertViFrame(void* data, int dataLen, bool isIFrame, const unsigned long long& pts, unsigned char chn = 0);//调用此函数插入vi数据，chn是视频所属通道号
	static void (*cb_changeBit)(uint32_t bit, unsigned char chn);//用户需要自己实现的修改视频码率的回调函数，chn参数是要调整帧率的vi通道号

	static void setCmdCallBack(void (*usersCmd_CB)(const unsigned char*, int));//设置用户自定义命令的处理回调函数，回调函数第一个参数是返回的数据，第二个参数是数据长度
	static void sendCmd(const unsigned char* cmd, int len);//发送自定义的命令给NVR
};


//改变码率示例代码，要把函数指针赋值给Mindsdk_Video::cb_changeBit
/*
void changeBit(uint32_t bitrate)
{
	VencRcParamEx_t rp;
	MI_VENC_GetRcParamEx(0, &rp);
	rp.type.h264Cbr.bitrate = bitrate;
	rp.type.h264Vbr.maxBitrate = bitrate;
	rp.type.h265Cbr.bitrate = bitrate;
	rp.type.h265Vbr.maxBitrate = bitrate;
	MI_VENC_SetRcParamEx(0, &rp);
	printf("change bitrate = %d\n", bitrate);
	VencFramerate_t fr;
	MI_VENC_GetFrameRate(0, &fr);
	printf("FrameRate = %d\n", fr.dstFrmRate);
}
*/






class Mindsdk_Audio
{
	friend Udper;
	static Udper* pUdper;
	static MemoryPool memoryPoll;
	static unsigned short nextSendAudioFrameNum;//下一个产生的audio数据包序号
	static mutex sendListMutex;
	static list<UdpProPackage*> sendList;
	static mutex recvMapMutex;
	static map<unsigned short, UdpProPackage*> recvMap;

	Mindsdk_Audio();//对象创建即开始接收数据包并可以进行处理，已经要求重发中间丢失的数据包
	~Mindsdk_Audio();
	static void startAudio(const unsigned char* data, int len);//对方请求 开始通话
	static void stopAudio(const unsigned char* data, int len);//对方请求 结束通话
	static void requestReSendAudio(unsigned short frameNum);//要求重发某个数据包
	static void reSendAudio(unsigned short frameNum, unsigned short serialNum);//重发某个数据包
	static void getAudio(const unsigned char* data, int len);//接收到对方发送来的音频数据
	static void insertAudioIntoRecvList(UdpProPackage* pPackage);//把接收到的数据插入接收队列合适的位置
	static void distributeAudioThread();//把接手到的音频数据派发给用户
public:
	static void setCmdCallBack(void (*usersCmd_CB)(const unsigned char*, int));//设置用户自定义命令的处理回调函数，回调函数第一个参数是返回的数据，第二个参数是数据长度
	static void sendCmd(const unsigned char* cmd, int len);//发送自定义的命令给NVR

	///////////以下函数、回调函数，是用户需要调用或设置的
	static Mindsdk_Audio* getInstance();
	static void start();//需要调用此函数启动
	static void sendAudio(const unsigned char* data, int len);//发用户送音频数据给NVR
	static void (*cb_startAudio)();//用户自己实现的开始音频通话的回调
	static void (*cb_stopAudio)();//用户自己实现的结束音频通话的回调
	static void (*cb_recvAudio)(unsigned char* buf, unsigned short len);//用户自己实现的获取到音频数据的回调，接收到NVR发来的音频数据时调用传回数据给用户
};