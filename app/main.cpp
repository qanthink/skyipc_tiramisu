/*---------------------------------------------------------------- 
xxx 版权所有。
作者：
时间：2020.7.10
----------------------------------------------------------------*/

#include <iostream>
#include <signal.h>

#include "sys.h"
#include "sensor.hpp"
#include "vif.h"
#include "vpe.h"
//#include "divp.h"
#include "venc.h"
#include "ai.hpp"
#include "ao.hpp"
#include "rgn.h"
//#include "wifi.h"

#include "aac.h"
#include "aad.h"
//#include "avtp_client.h"
#include "ircut.h"
#include "queue.h"
#include "spipanel.h"
#include "ethernet.h"
#include "live555rtsp.h"
#include "mp4container.h"

#include "testing.h"

using namespace std;

void sigHandler(int sig);

int main(int argc, const char *argv[])
{
	/*
		统一采用C++单例设计模式，getInstance() 为各模块实例的统一入口函数。
		单例模式不存在重复初始化的问题，在调用之初执行构造，后续其它地方的调用时间开销小。
		风格尽量趋近于C, 避免C++11 及之后的高级用法。
		库函数尽可能使用linux 标准库函数，高效，可调试性高。暂不考虑linux->otherOS 的移植问题。
	*/

	signal(SIGINT, sigHandler);

#if 1
	IrCut::getInstance()->resetFilter();	// ircut滤波片复位
	IrCut::getInstance()->closeFilter();

	// 系统初始化
	Sys *pSys = Sys::getInstance();
	// 出图模块初始化。数据流向：sensor -> vif -> vpe -> venc -> 应用处理。
	Sensor *pSensor = Sensor::getInstance();// sensor 初始化
	pSensor->setFps(30);
	Vif *pVif = Vif::getInstance();			// VIF 初始化
	Vpe *pVpe = Vpe::getInstance();			// VPE 初始化
	pVpe->createMainPort(Vpe::vpeMainPort);	// 创建VPE 主码流
	//pVpe->setPortCropScale(Vpe::vpeMainPort, 0, 0, 0, 0, 1920, 1080);
	//pVpe->createSubPort(Vpe::vpeSubPort);	// 创建VPE 子码流
	//pVpe->createJpegPort(Vpe::vpeJpegPort);	// 创建VPE JPEG码流
	
	Venc *pVenc = Venc::getInstance();		// VENC 初始化
	pVenc->createMainStream(Venc::vencMainChn, NULL);	// 创建VENC主码流
	//pVenc->createSubStream(Venc::vencSubChn, NULL);		// 创建VENC子码流
	//pVenc->createJpegStream(Venc::vencJpegChn, NULL);	// 创建VENC-JPEG码流

	// VENC 也可以实现图像的Crop 和Scale, 但是建议在VPE 中做。
	//pVenc->setCrop(Venc::vencMainChn, (2560 - 1920) / 2, (1440 - 1080) / 2, 1920, 1080);
	
	// 绑定VIF -> VPE. 只需要绑定一次，用REALTIME
	pSys->bindVif2Vpe(Vif::vifPort, Vpe::vpeInputPort, 30, 30, E_MI_SYS_BIND_TYPE_REALTIME, 0);
	// 绑定VPE -> VENC, 如果有多路码流，则需要绑定多次。
	//pSys->bindVpe2Venc(Vpe::vpeMainPort, Venc::vencMainChn, 30, 30, E_MI_SYS_BIND_TYPE_FRAME_BASE, 0);	// FrameMode 导致帧率降低。2022.4.17
	pSys->bindVpe2Venc(Vpe::vpeMainPort, Venc::vencMainChn, 30, 30, E_MI_SYS_BIND_TYPE_HW_RING, 1080);
	//pSys->bindVpe2Venc(Vpe::vpeSubPort, Venc::vencSubChn, 30, 30, E_MI_SYS_BIND_TYPE_REALTIME, 0);
	//pSys->bindVpe2Venc(Vpe::vpeJpegPort, Venc::vencJpegChn, 30, 30, E_MI_SYS_BIND_TYPE_FRAME_BASE, 0);
#endif

	// 初始化OSD
	#if (1 == (USE_OSD))
	Rgn *pRgn = Rgn::getInstance();
	#endif
	
	#if (1 == (USE_FAAC_FAAD))
	// 音频编/解码初始化。aac = audio coder; aad = audio decoder.
	Aac *pAac = Aac::getInstance();
	Aad *pAad = Aad::getInstance();
	#endif

	// AI 和AO 属于两个模块，分别初始化。
#if (1 == (USE_AI))
	AudioIn *pAudioIn = AudioIn::getInstance();
#endif
	
#if (1 == (USE_AO))
	AudioOut *pAudioOut = AudioOut::getInstance();
#endif

	#if (1 == (USE_WIFILINK))
	Wifi *pWifi = Wifi::getInstance();
	pWifi->enable();
	#endif

	/*
		至此，SENSOR, VIF, VPE, VENC, AI, AO, OSD 均已初始化完成。
		以下代码的功能为通过音视频传输协议将数据传输给另一台设备。
	*/
	
	#if (1 == (USE_MINDSDK_AUDIO))
	// avtp 传输协议。
	Mindsdk_Audio& audio = *Mindsdk_Audio::getInstance();
	audio.cb_recvAudio = recvAudio;
	audio.start();
	#endif

	#if (1 == (USE_MINDSDK_VIDEO))
	// avtp 传输协议。
	Mindsdk_Video &Video = *Mindsdk_Video::getInstance();
	Video.cb_startVideo = startVideo;
	Video.cb_stopVideo = stopVideo;
	Video.cb_getNextViFrame = getNextViFrame;
	Video.cb_changeBit = changeBit;
	Video.start();
	#endif

	int ret = 0;
	#if (1 == (USE_RTSPSERVER_LOCALFILE))
	const char *fileName = NULL;
	//fileName = "/mnt/linux/Downloads/videotest/1.mp4";
	fileName = "/mnt/linux/Downloads/material/test.264";
	thread thRtsp = thread(createRtspServerBy265LocalFile, (void *)fileName);
	#endif
	
	#if (1 == (USE_RTSPSERVER_LIVESTREAM))
	Live555Rtsp *pLive555Rtsp = Live555Rtsp::getInstance();
	#endif

	#if (1 == (USE_FFMPEG_SAVE_MP4))
	Mp4Container *pMp4Container = Mp4Container::getInstance();
	#endif

	// 创建3个线程，分别用于测试AI, AO, 出图。
	#if (1 == (USE_AI))
	thread thAi = thread(routeAi, (void *)NULL);
	#endif

	#if (1 == (USE_AO))
	// AO. 参数为本地音频文件的路径。写死的，16位宽 16000采样率
	thread thAo = thread(routeAo, (void *)"/mnt/linux/Music/pcm_16000_16bit.pcm");
	#endif

	#if(1 == (USE_OSD))
	// OSD 功能
	thread thOsd = thread(routeOsd, (void *)NULL);
	#endif

	#if (1 == (USE_VENC))
	cout << "routeVideo" << endl;
	// 测试出图的线程。参数为VENC 的通道号，支持主码流和子码流。
	thread thVideo = thread(routeVideo, (void *)Venc::vencMainChn);
	#endif

	#if (1 == (USE_SPIPANEL))
	cout << "route Spi panel" << endl;
	thread thSpiPanel = thread(routeSpiPanel, (void *)NULL);
	#endif

	#if (1 == (TEST_ETHERNET))
	testEthernet();
	Ethernet *pEthernet = pEthernet->getInstance();
	pEthernet->showWlanInfOnPanel();
	#endif

	g_bRunning = true;		// sigHandler() 对其取反。
	while(g_bRunning)
	{
		sleep(1);
		static int sleepCntSec = 0;
		if(10 == sleepCntSec++)
		{
			cout << "Progress running." << endl;
			sleepCntSec = 0;
		}
	}

	#if (1 == (USE_AI))
	thAi.join();
	#endif
	
	#if (1 == (USE_AO))
	thAo.join();
	#endif
	
	#if (1 == (USE_VENC))
	thVideo.join();
	#endif

	#if (1 == (USE_OSD))
	thOsd.join();
	#endif

	#if (1 == (USE_RTSPSERVER_LIVESTREAM))
	pLive555Rtsp->disable();
	#endif

	#if (1 == (USE_RTSPSERVER_LOCALFILE))
	thRtsp.join();
	#endif

	cout << "Sleep()" << endl;
	sleep(0.5);
	
	return 0;
}

void sigHandler(int sig)
{
	switch (sig)
	{
	case SIGINT:
		cerr << "Receive SIGINT!!!" << endl;
		g_bRunning = false;
		break;
	default:
		g_bRunning = false;
		break;
	}
}

