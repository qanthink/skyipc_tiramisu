#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <iostream>
#include <thread>
#include <mutex>
#include <map>
#include <set>
#include <vector>
#include <list>
#include <functional>
#include <memory.h>
#include <assert.h>
#include <signal.h> 
#include <semaphore.h>
#include <sys/un.h>
#include <sys/timeb.h>
#include <dlfcn.h>
#include <stddef.h>
#include <fcntl.h>
#include <stdint.h>
#include <algorithm>
#include <sys/prctl.h>
#include <sys/select.h>
#include <atomic>
#include <sys/time.h>
#include <unistd.h>
#include "package.h"
using namespace std;

#define ERR_EXIT(m) \
    do { \
    perror(m); \
    exit(EXIT_FAILURE); \
    } while (0)

typedef void (*CallBack)(const unsigned char*, int);
typedef void (*ReSendCallBack)(unsigned short, unsigned short);
typedef void (*ReSendViCallBack)(unsigned short, unsigned short, unsigned char);

#define DE_NvrViChnNum 8

class Mindsdk_Video;

class Udper
{
	friend Mindsdk_Video;
	struct sockaddr_in nvrAddr;
	bool chnZeroAddr = false;
	struct sockaddr_in nvrAddrAr[DE_NvrViChnNum];
	list<unsigned char> openViChnList;
	mutex openViChnListMutex;

	int port = 0;
	int serverSock = 0;
	ReSendCallBack audioReSend_CB = nullptr;//重发音频数据包的回调
	ReSendViCallBack videoReSend_CB = nullptr;//重发视频数据包的回调
	CallBack getAudio_CB = nullptr;//获取音频数据回调
	CallBack getVideo_CB = nullptr;//获取视频数据回调
	CallBack startTalk_CB = nullptr;//开始通话回调
	CallBack stopTalk_CB = nullptr;//结束通话回调
	CallBack startVideo_CB = nullptr;//开始获取视频的回调
	CallBack stopVideo_CB = nullptr;//结束获取视频的回调
	CallBack startHc_CB = nullptr;
	CallBack stopHc_CB = nullptr;

	Udper();
	~Udper();
public:
	void (*usersCmd_CB)(const unsigned char*, int) = nullptr;//用户自己定义的控制命令回调函数
	static Udper* getInstance();
	void setCallBack(ReSendCallBack reSendCallBack_audio, ReSendViCallBack reSendCallBack_video, CallBack audioCb, CallBack videoCb, CallBack startAudio, CallBack stopAudio, CallBack startVideo, CallBack stopVideo, CallBack startHc = nullptr, CallBack stopHc = nullptr);
	void bindAddr(int loacalPort);//绑定本地端口
	void startReveThread();
	static void showPackage(UdpProPackage* p);

	//mutex sendingMutex;
	void sendData(const unsigned char* data, int len, bool isAudio = false, bool isVideo = false, bool isIFrame = false, unsigned char chn = 0);//isIFrame是否是I帧，isVideo为true的时候才用

	unsigned short nextReliablePackageNum = 0;
	void sendReliablePackage(const unsigned char* data, int len);//发送可靠数据包
	void removeReliablePackage(unsigned short packageNum);//从发送列表中删除可靠数据包
	map<unsigned short, shared_ptr<vector<unsigned char>>> reliablePackageMap;
	mutex reliablePackageMapMutex;
	static void sendReliablePackageThread(Udper* pThis);

	list<unsigned short> controlPackageHasDealWihtList;//用于记录已经处理过的控制包列表，避免重复处理
	const int controlPackageHasDealWihtList_MaxSize = 20;
	bool checkControlPackageHasDealWiht(unsigned short packageNum);//判断该控制包是否已经处理过
	static void recvThread(Udper* pThis);

	unsigned long getLastTime(timeval& start);//计算经历了多长时间，单位毫秒
	atomic<int> audioBitRate;
	atomic<int> videoBitRate;//音视频的码流，用于显示osd

};

