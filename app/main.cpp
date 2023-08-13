/*---------------------------------------------------------------- 
xxx 版权所有。
作者：
时间：2020.7.10
----------------------------------------------------------------*/

#include <thread>
#include <iostream>
#include <signal.h>

#include "sensor.h"
#include "vif.h"
#include "isp.h"
#include "scl.h"
#include "venc.h"
#include "sys.h"
#include "testing.h"

#if (1 == (USE_AI))
#include "ai.hpp"
#endif

#if (1 == (USE_AO))
#include "ao.hpp"
#endif

#if (1 == (USE_OSD))
#include "rgn.h"
#endif

#if (1 == (USE_DIVP))
#include "divp.h"
#endif

#if (1 == (USE_UVC))
#include "uvc_uac.h"
#endif

#if (1 == (USE_FAAC_FAAD))
#include "aac.h"
#include "aad.h"
#endif

#if (1 == (USE_AVTP_AUDIO) || 1 == (USE_AVTP_VIDEO))
#include "avtp.h"
#endif

#if (1 == (TEST_IRCUT))
#include "ircutled.h"
#endif

#if (1 == (USE_PANEL))
#include "spipanel.h"
#endif

#if (1 == (USE_ETHERNET))
#include "ethernet.h"
#endif

#if (1 == (USE_RTSP_LOCALFILE) || 1 == (USE_RTSP_LIVESTREAM))
#include "live555rtsp.h"
#endif

#if (1 == (USE_FFMPEG_SAVE_MP4))
#include "mp4container.h"
#endif

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

	/* ==================== 第一部分，系统初始化 ==================== */
	#if (1 == (TEST_IRCUT))
	IrCutLed *pIrCutLed = IrCutLed::getInstance();
	IrCutLed::getInstance()->openFilter();
	#endif

	// SDK 系统初始化
	Sys *pSys = Sys::getInstance();

	// Sensor 初始化。
	// 数据流向：sensor -> vif -> vpe(或ISP) -> (SCL) -> (DIVP) -> venc -> 应用处理。
	Sensor *pSensor = Sensor::getInstance();
	pSensor->setFps(30);

	unsigned int snrW = 0;
	unsigned int snrH = 0;
	pSensor->getSnrWH(&snrW, &snrH);
	cout << "snrW, snrH = " << snrW << ", " << snrH << endl;

	// VIF 初始化
	Vif *pVif = Vif::getInstance();

	// ISP 初始化，并绑定前级VIF.
	Isp *pIsp = Isp::getInstance();
	pSys->bindVif2Isp(Vif::vifDevID, Isp::ispDevId, 30, 30, E_MI_SYS_BIND_TYPE_FRAME_BASE, 0);

	// SCL 初始化，并绑定前级ISP.
	Scl *pScl = Scl::getInstance();
	pSys->bindIsp2Scl(Isp::ispDevId, Scl::sclDevId, 30, 30, E_MI_SYS_BIND_TYPE_REALTIME, 0);

	// 创建DIVP 缩放、剪裁通道、格式转换
	#if (1 == (USE_DIVP))
	Divp *pDivp = Divp::getInstance();
	pDivp->createChn(snrW, snrH, snrW, snrH);
	#endif

	Venc *pVenc = Venc::getInstance();
	#if (1 == (USE_IPC))
	// 创建主码流
	#if (1 == (USE_VENC_MAIN))
	pScl->createPort(Scl::sclPortMain, snrW, snrH);
	pVenc->createH264Stream(MI_VENC_DEV_ID_H264_H265_0, Venc::vencMainChn, snrW, snrH);
	pVenc->changeBitrate(MI_VENC_DEV_ID_H264_H265_0, Venc::vencMainChn, 1 * 1024);
	pVenc->setInputBufMode(MI_VENC_DEV_ID_H264_H265_0, Venc::vencMainChn, E_MI_VENC_INPUT_MODE_RING_ONE_FRM);
	pVenc->startRecvPic(MI_VENC_DEV_ID_H264_H265_0, Venc::vencMainChn);
	pSys->bindScl2Venc(Scl::sclPortMain, MI_VENC_DEV_ID_H264_H265_0, Venc::vencMainChn, 30, 30, E_MI_SYS_BIND_TYPE_HW_RING, snrH);
	#if (1 == (USE_DIVP))
	#else
	#endif	// End of USE_DIVP
	#endif	// End of USE_VENC_MAIN

	// 创建子码流
	#if (1 == (USE_VENC_SUB))
	unsigned int subW = 1280;
	unsigned int subH = 720;
	pScl->createPort(Scl::sclPortSub, subW, subH);
	pVenc->createH264Stream(MI_VENC_DEV_ID_H264_H265_0, Venc::vencSubChn, subW, subH);
	pVenc->changeBitrate(MI_VENC_DEV_ID_H264_H265_0, Venc::vencSubChn, 0.25 * 1024);
	pVenc->setInputBufMode(MI_VENC_DEV_ID_H264_H265_0, Venc::vencSubChn, E_MI_VENC_INPUT_MODE_NORMAL_FRMBASE);
	pVenc->startRecvPic(MI_VENC_DEV_ID_H264_H265_0, Venc::vencSubChn);
	pSys->bindScl2Venc(Scl::sclPortSub, MI_VENC_DEV_ID_H264_H265_0, Venc::vencSubChn, 30, 30, E_MI_SYS_BIND_TYPE_FRAME_BASE, snrH);
	#endif

	// 创建JPEG 码流
	#if (1 == (USE_VENC_JPEG))
	pScl->createPort(Scl::sclPortJpeg, snrW, snrH, true);
	pVenc->createJpegStream(MI_VENC_DEV_ID_JPEG_0, Venc::vencJpegChn, snrW, snrH);
	pVenc->changeBitrate(MI_VENC_DEV_ID_JPEG_0, Venc::vencJpegChn, 50);;
	pSys->bindScl2Venc(Scl::sclPortJpeg, MI_VENC_DEV_ID_JPEG_0, Venc::vencJpegChn, 30, 30, E_MI_SYS_BIND_TYPE_REALTIME, 0);
	#endif	// End of USE_VENC_JPEG
	#endif	// End of USE_IPC

	#if(1 == (USE_IQ_SERVER))
	pIsp->openIqServer();
	char iqFilePath[128] = "/config/iqfile/335_imx291_day.bin";
	pIsp->loadBinFile(iqFilePath);
	#endif

	// 初始化OSD
	#if (1 == (USE_OSD))
	Rgn *pRgn = Rgn::getInstance();
	#endif
	
	// 音频编/解码初始化。aac = audio coder; aad = audio decoder.
	#if (1 == (USE_FAAC_FAAD))
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

	// 测试UVC
	#if (1 == (USE_UVC))
	UvcUac *pUvcUac = UvcUac::getInstance();
	pUvcUac->startUvc();
	#endif

	/*
		至此，SENSOR, VIF, ISP, VENC, AI, AO, OSD 均已初始化完成。
	*/

	/* ==================== 第二部分，应用初始化 ==================== */
	// 音视频传输协议测试
	#if (1 == (USE_AVTP_AUDIO))
	AvtpAudioClient avtpAudioClient("192.168.0.200");
	pAvtpAudioClient = &avtpAudioClient;
	#endif

	#if (1 == (USE_AVTP_VIDEO))
	AvtpVideoClient avtpVideoClient("192.168.0.200");
	pAvtpVideoClient = &avtpVideoClient;
	thread thChangeBitrate(avtpChangeKbps, pAvtpVideoClient, 3);
	#endif

	// RTSP 推流
	#if ((1 == (USE_RTSP_LOCALFILE)) || (1 == (USE_RTSP_LIVESTREAM)))
	Live555Rtsp *pLive555Rtsp = Live555Rtsp::getInstance();
	// 推本地流：
	#if (1 == (USE_RTSP_LOCALFILE))
	const char *filePath = NULL;
	//filePath = "/mnt/linux/Downloads/videotest/1.mp4";
	filePath = "/mnt/linux/Downloads/material/test.264";
	const char *streamName = "stream";
	pLive555Rtsp->addStream(filePath, streamName, emEncTypeH264);
	#endif	// End of USE_RTSP_LOCALFILE
	#endif	// 
	

	// FFMPEG 保存MP4.
	#if (1 == (USE_FFMPEG_SAVE_MP4))
	Mp4Container *pMp4Container = Mp4Container::getInstance();
	#endif

	// 获取音频AI 的线程
	#if (1 == (USE_AI))
	thread thAi(routeAi, (void *)NULL);
	#endif

	// AO 音频测试
	#if (1 == (USE_AO))
	
	// AO. 参数为本地音频文件的路径。写死的，16位宽 16000采样率
	#if (1 == (USE_AO_LOCAL_FILE))
	thread thAo(routeAoFromFile, (void *)"/mnt/linux/Audios/pcm/xiaopingguo_mono_16b_16000.pcm");
	#endif	// End of USE_AO_LOCAL_FILE

	// AO 音频测试，播放网络音频
	#if (1 == (USE_AO_NET_PCM))
	thread thAo(routeAoNetPcm, (void *)NULL);
	#endif	// End of USE_AO_NET_PCM
	#endif	// End of USE_AO

	// OSD 功能
	#if(1 == (USE_OSD))
	thread thOsd(routeOsd, (void *)NULL);
	#endif

	// 测试主码流
	#if (1 == (USE_VENC_MAIN))
	cout << "routeVideoMain" << endl;
	thread thVideoMain(routeVideo, (int)MI_VENC_DEV_ID_H264_H265_0, (int)Venc::vencMainChn);
	#endif

	// 测试子码流
	#if (1 == (USE_VENC_SUB))
	cout << "routeVideoSub" << endl;
	thread thVideoSub(routeVideo, (int)MI_VENC_DEV_ID_H264_H265_0, (int)Venc::vencSubChn);
	#endif

	// 测试JPEG 码流
	#if (1 == (USE_VENC_JPEG))
	cout << "routeVideoJpeg" << endl;
	thread thVideoJpeg(routeVideo, (int)MI_VENC_DEV_ID_JPEG_0, (int)Venc::vencJpegChn);
	#endif

	int ret = 0;
	this_thread::sleep_for(chrono::microseconds(1));	// sleep for rtsp file read.
	#if ((1 == (USE_RTSP_LOCALFILE)) || (1 == (USE_RTSP_LIVESTREAM)))
	thread thRtsp([&](){pLive555Rtsp->eventLoop();});		// lambda 表达式太好用啦！
	thRtsp.detach();
	#endif

	// 测试SPI 屏。
	#if (1 == (USE_SPI_PANEL))
	thread thSpiPanel(routeSpiPanel, (void *)NULL);
	#endif

	// 测试网络
	#if (1 == (TEST_ETHERNET))
	testEthernet();
	Ethernet *pEthernet = pEthernet->getInstance();
	pEthernet->showWlanInfOnPanel();
	#endif

	g_bRunning = true;		// sigHandler() 对其取反。
	while(g_bRunning)
	{
		//int ret = 0;
		#if (1 == USE_INTERACTION)
		ret = interAction();
		if(1 == ret)
		{
			break;
		}
		#else
		sleep(1);
		static int sleepCntSec = 0;
		if(15 == sleepCntSec++)
		{
			cout << "Progress running." << endl;
			sleepCntSec = 0;
		}
		#endif
	}
	cout << "jump out from while(g_bRunning)" << endl;

	/* ==================== 第三部分，应用析构 ==================== */
	#if ((1 == (USE_RTSP_LOCALFILE)) || (1 == (USE_RTSP_LIVESTREAM)))
	//pLive555Rtsp->~Live555Rtsp();
	cout << "rtsp join()" << endl;
	//thRtsp.join();	// 前期做了detach.
	cout << "rtsp join." << endl;
	#endif
	
	#if (1 == (USE_AI))
	thAi.join();
	cout << "ai join." << endl;
	#endif
	
	#if (1 == (USE_AO))
	thAo.join();
	cout << "ao join." << endl;
	#endif
									
	#if (1 == (USE_VENC_JPEG))
	thVideoJpeg.join();
	cout << "vencJpeg join." << endl;
	#endif
	
	#if (1 == (USE_VENC_SUB))
	thVideoSub.join();
	cout << "vencSub join." << endl;
	#endif

	#if (1 == (USE_VENC_MAIN))
	thVideoMain.join();
	cout << "vencMain join." << endl;
	#endif

	#if (1 == (USE_OSD))
	thOsd.join();
	cout << "osd join." << endl;
	#endif

	#if (1 == (USE_AVTP_VIDEO))
	thChangeBitrate.join();
	cout << "avtp video join." << endl;
	#endif

	#if (1 == (USE_SPI_PANEL))
	thSpiPanel.join();
	cout << "spi panel join." << endl;
	#endif

	#if (1 == (USE_UVC))
	pUvcUac->stopUvc();
	#endif

	#if(1 == (USE_IQ_SERVER))
	pIsp->closeIqServer();
	#endif

	cout << "Sleep()" << endl;
	this_thread::sleep_for(chrono::milliseconds(500));
	
	return 0;
}

/*-----------------------------------------------------------------------------
描--述：信号处理函数。
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
void sigHandler(int sig)
{
	cout << "Call sigHandler()." << endl;
	
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

	cout << "Call sigHandler() end." << endl;
	return;
}

