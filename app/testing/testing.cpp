/*---------------------------------------------------------------- 
xxx版权所有。
作者：
时间：2020.7.15
----------------------------------------------------------------*/

/*
	本文件为测试用例，仅用于验证硬件完好、音视频通讯正常，不能用于客户开发上层应用。
	客户在熟悉接口之后，请删除本文件，根据自身需求开发上层应用。
*/

#include <fstream>
#include <iostream>
#include <string.h>
#include <fcntl.h>
#include <thread>

#include "testing.h"
#include "sensor.h"
#include "scl.h"
#include "isp.h"
#include "venc.h"
#include "sys.h"

#if (1 == (USE_AI))
#include "ai.hpp"
#endif

#if (1 == (USE_AO))
#include "ao.hpp"
#endif

#if (1 == (TEST_IRCUT))
#include "ircutled.h"
#endif

#if (1 == (USE_FAAC_FAAD))
#include "aac.h"
#include "aad.h"
#endif

#if (1 == (USE_OSD))
#include "rgn.h"
#endif

#if (1 == (USE_AVTP_AUDIO) || 1 == (USE_AVTP_VIDEO))
#include "avtp.h"
#if (1 == (USE_AVTP_AUDIO))
#include "atp_client.h"
#endif
#if (1 == (USE_AVTP_VIDEO))
#include "vtp_client.h"
#endif
#endif

#if (1 == (USE_RTSP_LOCALFILE) || 1 == (USE_RTSP_LIVESTREAM))
#include "live555rtsp.h"
#endif

#include "myfifo.h"
#if (1 == (USE_FFMPEG_SAVE_MP4))
#include "mp4container.h"
#endif

#if (1 == (USE_SPI_PANEL))
#include "spipanel.h"
#endif

#if (1 == (TEST_ETHERNET))
#include "ethernet.h"
#endif

using namespace std;

volatile bool g_bRunning = true;

#if (1 == (USE_AVTP_AUDIO))
AvtpAudioClient *pAvtpAudioClient = NULL;
#endif

#if (1 == (USE_AVTP_VIDEO))
AvtpVideoClient *pAvtpVideoClient = NULL;
#endif

#if (1 == (USE_AI))
/*-----------------------------------------------------------------------------
描--述：AI 线程。不断地获取音频原始数据。
参--数：无
返回值：无
注--意：AI 处理流程：获取PCM流 -> 编码为AAC流 -> 传输。
		注意循环结束后，释放自由空间的内存。
-----------------------------------------------------------------------------*/
void *routeAi(void *arg)
{
	cout << "Call routeAi()." << endl;

	unsigned char *pcmBuf = NULL;
	unsigned char *aacBuf = NULL;

	#if (1 == (USE_FAAC_FAAD))
	Aac *pAac = Aac::getInstance();
	pcmBuf = (unsigned char*)malloc(pAac->getInputSamples());
	if(NULL == pcmBuf)
	{
		cerr << "Fail to call malloc(3)." << endl;	
	}

	aacBuf = (unsigned char*)malloc(pAac->getMaxOutputBytes());
	if(NULL == aacBuf)
	{
		cerr << "Fail to call malloc(3)." << endl;	
	}
	#endif

	//unsigned int uFrameCnt = 3600 * 72 * 16000 / 1000 * 1.05;	// n * samples / 1000 = n 秒。
	//unsigned int uFrameCnt = 60 * 16000 / 1000 * 1.05;	// n * samples / 1000 = n 秒。
	unsigned int uFrameCnt = 20 * 16000 / 1024;				// n * samples / u32PtNumPerFrm = n 秒。

	#if (1 == (USE_AI_SAVE_LOCAL_PCM))
	const char *filePathPcm = "/mnt/linux/Downloads/audio.pcm";
	ofstream ofsPcm;
	ofsPcm.open(filePathPcm, ios::trunc);
	if(ofsPcm.fail())
	{
		cerr << "Fail to open " << filePathPcm << ", " << strerror(errno) << endl;
	}
		else
	{
		cout << "Success to open file " << filePathPcm << endl;
	}
	#endif
	
	#if (1 == (USE_AI_SAVE_LOCAL_AAC))
	const char *filePathAac = "/mnt/linux/Downloads/audio.aac";
	ofstream ofsAac;
	ofsAac.open(filePathAac, ios::trunc);
	if(ofsAac.fail())
	{
		cerr << "Fail to open " << filePathAac << ", " << strerror(errno) << endl;
	}
	else
	{
		cout << "Success to open file " << filePathAac << endl;
	}
	#endif

	while(g_bRunning)
	{
		int ret = 0;
		stAIFrame_t stAudioFrame;
		memset(&stAudioFrame, 0, sizeof(stAudioFrame));
		
		ret = AudioIn::getInstance()->recvStream(&stAudioFrame);
		if(0 != ret)
		{
			cerr << "Fail to call pAudioIn->rcvStream(). ret = " << ret << endl;
			continue;
		}

		#if 0	// debug
		cout << "[AI bLoudSoundDetected] = " << (int)stAudioFrame.bLoudSoundDetected << endl;
		cout << "stAudioFrame.u32Len = " << stAudioFrame.u32Len << endl;
		//cout << "stAudioFrame.eBitWidth = " << stAudioFrame.eBitWidth << endl;
		//cout << "stAudioFrame.eSoundmode = " << stAudioFrame.eSoundmode << endl;
		#endif

		int aacBytes = 0;
		#if (1 == (USE_FAAC_FAAD))
		aacBytes = pAac->encEncode((int32_t *)stAudioFrame.apFrameBuf, stAudioFrame.u32Len / 2, aacBuf, pAac->getMaxOutputBytes());
		if(-1 == aacBytes || 0 == aacBytes)
		{
			cerr << "Fail to call pAac->enEncode(), ret = " << aacBytes << endl;
			continue;
		}
		#endif

		#if 0	//debug
		cout << "Success to call pAac->encEncode(), ret = " << ret << endl;
		#endif

		if(0 != uFrameCnt)
		{
			--uFrameCnt;
		}

		#if (1 == (USE_AI_SAVE_LOCAL_PCM))
		if(0 != uFrameCnt)
		{
			//ofsPcm.write((const char *)stAudioFrame.apFrameBuf, stAudioFrame.u32Len / 2);
			ofsPcm.write((const char *)stAudioFrame.apFrameBuf, stAudioFrame.u32Len);
		}
		else
		{
			if(ofsPcm.is_open())
			{
				ofsPcm.close();
				cout << "Write local PCM file over. Close file." << endl;
			}
		}
		#endif

		#if (1 == (USE_AI_SAVE_LOCAL_AAC))
		if(0 != uFrameCnt)
		{
			ofsAac.write((const char *)aacBuf, aacBytes);
		}
		else
		{
			if(ofsAac.is_open())
			{
				ofsAac.close();
				cout << "Write local AAC file over. Close file." << endl;
			}
		}
		#endif
		
		#if (1 == (USE_AVTP_AUDIO))
		if(pAvtpAudioClient->isAllowTalking())
		{
			pAvtpAudioClient->sendAudioFrame(stAudioFrame.apFrameBuf, stAudioFrame.u32Len);
		}
		else
		{
			// do nothing.
		}
		//cout << "avtpAudio send size = " << stAudioFrame.u32Len << endl;
		#endif
	}

	#if (1 == (USE_FAAC_FAAD))
	free(aacBuf);
	aacBuf = NULL;
	free(pcmBuf);
	pcmBuf = NULL;
	#endif

	#if (1 == (USE_AI_SAVE_LOCAL_PCM))
	if(ofsPcm.is_open())
	{
		ofsPcm.close();
	}
	#endif

	#if (1 == (USE_AI_SAVE_LOCAL_AAC))
	if(ofsAac.is_open())
	{
		ofsAac.close();
	}
	#endif

	cout << "Call routeAi() end." << endl;
	return NULL;
}
#endif

#if (1 == (USE_AO_LOCAL_FILE))
/*-----------------------------------------------------------------------------
描--述：AO 线程。从本地PCM 文件中读取数据，塞给AO 模块进行播放。
参--数：arg 本地文件的路径
返回值：无
注--意：
-----------------------------------------------------------------------------*/
void *routeAoFromFile(void *arg)
{
	cout << "Call routeAoFromFile()." << endl;

	if(NULL == arg)
	{
		cerr << "Fail to call readPcmRoute(), argument has null value!" << endl;
		return NULL;
	}

	const char *filePath = (char *)arg;
	int fd = -1; 
	fd = open(filePath, O_RDONLY);
	if(-1 == fd) 
	{
		cerr << "Fail to call open(2) in routeAoFromFile(), " << strerror(errno) << endl;
		return NULL;
	}

	cout << "Success to call open() in routeAo()." << endl;

	do{
		int readBytes = 0;
		unsigned int dataBufMaxSize = 2 * 1024;
		char dataBuf[dataBufMaxSize] = {0};
		
		//cout << "Ready to call read() in routeAo()." << endl;
		readBytes = read(fd, dataBuf, dataBufMaxSize);
		if(-1 == readBytes)
		{
			cerr << "Fail to call read(2), " << strerror(errno) << endl;
			break;
		}
		else if(0 == readBytes)
		{
			cout << "Read file over!" << endl;
			break;
		}

		//cout << "Send pcm stream" << endl;
		AudioOut::getInstance()->sendStream(dataBuf, readBytes);
	}while(g_bRunning);

	if(-1 != fd)
	{
		close(fd);
		fd = -1;
	}

	cout << "Call routeAoFromFile() end." << endl;
	return NULL;
}
#endif

#if (1 == (USE_AO_NET_PCM))
/*-----------------------------------------------------------------------------
描--述：AO 线程。从网络中获取数据，塞给AO 模块进行播放。
参--数：无
返回值：无
注--意：
-----------------------------------------------------------------------------*/
void *routeAoNetPcm(void *arg)
{
	while(g_bRunning)
	{
		unsigned int realSize = 0;
		const unsigned int bufSize = 8 * 1024;
		unsigned char dataBuf[bufSize] = {0};

		#if (1 == (USE_AVTP_AUDIO))
		if(pAvtpAudioClient->isAllowTalking())
		{
			
			realSize = pAvtpAudioClient->recvAudioFrame(dataBuf, bufSize);
		}
		else
		{
			this_thread::sleep_for(chrono::milliseconds(100));
			continue;
		}
		#endif

		if(realSize > 0)
		{
			AudioOut::getInstance()->sendStream(dataBuf, realSize);
		}
		else
		{
			this_thread::sleep_for(chrono::milliseconds(10));
			continue;
		}
	}

	return NULL;
}
#endif

/*-----------------------------------------------------------------------------
描--述：VIDEO 线程：从VENC 中获取若干帧数据保存到本地。
参--数：arg, VENC 通道号。
返回值：
注--意：数据保存的路径为/customer/video.h265
-----------------------------------------------------------------------------*/
void *routeVideo(int vencDev, int vencCh)
{
	cout << "Call routeVideo()." << endl;

	/* sava to local file. */
	#if (1 == (USE_VENC_SAVE_LOCAL_FILE))
	int fd = -1;
	unsigned int uFrameCnt = SAVE_TIME_SECONDS * 30;		// 写入文件的帧数。N * FPS = N秒。
	//const char *filePath = "/customer/video.265";
	const unsigned int pathLen = 1024;
	char filePath[pathLen] = "/mnt/linux/Downloads/video";
	snprintf(filePath, pathLen, "%s%d", filePath, u32VencChn);
	mode_t mode = 0666;
	
	fd = open(filePath, O_WRONLY | O_CREAT | O_TRUNC, mode);		// 只写 | 如果不存在则创建 | 如果存在则清空
	if(-1 == fd)
	{
		cerr << "Fail to open " << filePath << ", " << strerror(errno) << endl;
	}
	#endif

	/* create named fifo & rtsp session */
	const size_t nameSize = 8;
	char videoName[nameSize] = "";
	const char *streamName = videoName;
	auto vesType = Venc::getInstance()->getVesType(vencDev, vencCh);

	#if (1 == (USE_RTSP_LIVESTREAM))
	snprintf(videoName, nameSize, "%s%d", "video", vencCh);
	cout << "live555 rtsp file : " << videoName << endl;
	unlink(videoName);
	MyNameFifo myNameFifo(videoName, Venc::superMaxISize);

	emEncType_t emEncType = emEncTypeInvalid;

	switch(vesType)
	{
		case E_MI_VENC_MODTYPE_H264E:
			emEncType = emEncTypeH264;
			break;
		case E_MI_VENC_MODTYPE_H265E:
			emEncType = emEncTypeH265;
			break;
		case E_MI_VENC_MODTYPE_JPEGE:
			emEncType = emEncTypeJpeg;
			break;
		default:
			emEncType = emEncTypeH264;
			break;
	}

	int retRtsp = 0;
	Live555Rtsp *pLive555Rtsp = Live555Rtsp::getInstance();
	retRtsp = pLive555Rtsp->addStream(videoName, streamName, emEncType);
	if(0 != retRtsp)
	{
		cerr << "Fail to call pLive555Rtsp->addStream() in routeVideo(). retRtsp = " << retRtsp << endl;
	}
	#endif

	while(g_bRunning)
	{
		MI_S32 s32Ret = 0;
		MI_VENC_Stream_t stStream;
		Venc *pVenc = Venc::getInstance();
		s32Ret = pVenc->rcvStream(vencDev, vencCh, &stStream);
		if(0 != s32Ret)
		{
			cerr << "Fail to call pVenc->rcvStream(). s32Ret = " << s32Ret << endl;
			continue;
		}

		int i = 0;		// 2020.7.22 增加for 循环，适配slice mode 下数据需要多片分发。
		for(i = 0; i < stStream.u32PackCount; ++i)
		{
			if(stStream.pstPack[i].u32Len > Venc::superMaxISize && E_MI_VENC_MODTYPE_JPEGE != vesType)
			{
				cerr << "stStream.pstPack[" << i << "].u32Len is out of range." << endl;
				break;
			}

			#if 0	// debug
			//pVenc->printStreamInfo(&stStream);
			cout << "u32Len = " << stStream.pstPack[i].u32Len << endl;
			#endif
			
			#if (1 == (USE_FFMPEG_SAVE_MP4))
			Mp4Container *pMp4Container = Mp4Container::getInstance();
			s32Ret = pMp4Container->sendH26xFrame(stStream.pstPack[i].pu8Addr, stStream.pstPack[i].u32Len);
			if(0 != s32Ret)
			{
				cerr << "Fail to call pFfmpeg->sendH26xFrame(). s32Ret = " << s32Ret << endl;
			}
			#endif

			#if (1 == (USE_RTSP_LIVESTREAM))
			if(0 == retRtsp)	// 正常返回值。
			{
				int fifoFd = myNameFifo.getFdWrite();
				if(-1 != fifoFd)
				{
					s32Ret = write(fifoFd, stStream.pstPack[i].pu8Addr, stStream.pstPack[i].u32Len);
					if(-1 == s32Ret && EPIPE == errno)
					{
						//cerr << "Fail to call write(2) in routeVideoMain() with named fifo. Fifo not be read." << endl;
					}
					else if(-1 == s32Ret)
					{
						cerr << "Fail to call write(2) in routeVideoMain() with named fifo. " << strerror(errno) << endl;
					}
				}
				else
				{
					cerr << "In routeVideo(), -1 == fifoFd." << endl;
				}
			}
			else
			{
				cerr << "retRtsp = " << retRtsp << "Rtsp server maybe not created." << endl;
			}
			#endif

			#if (1 == (USE_VENC_SAVE_LOCAL_FILE))
			if(0 != uFrameCnt)
			{
				--uFrameCnt;
				if(-1 != fd)
				{
					s32Ret = write(fd, stStream.pstPack[i].pu8Addr, stStream.pstPack[i].u32Len);
					if(-1 == s32Ret)
					{
						cerr << "Fail to call write(2), " << strerror(errno) << endl;
					}
				}
			}
			else
			{
				if(-1 != fd)
				{
					close(fd);
					fd = -1;
					cout << "Write local H.26X file over. Close file." << endl;
				}
			}
			#endif
			
			#if (1 == (USE_AVTP_VIDEO))
			cout << stStream.pstPack[i].u32Len << endl;
			pAvtpVideoClient->sendVideoFrame(stStream.pstPack[i].pu8Addr, stStream.pstPack[i].u32Len);
			if(!g_bRunning)
			{
				//avtpVideClient.stop();
			}
			#endif
		}

		s32Ret = pVenc->releaseStream(vencDev, vencCh, &stStream);
		if(0 != s32Ret)
		{
			cerr << "Fail to call Venc::releaseStream(), errno = " << s32Ret << endl;;
		}
	}

	#if (1 == (USE_VENC_SAVE_LOCAL_FILE))
	if(-1 != fd)
	{
		close(fd);
		fd = -1;
	}
	#endif

	cout << "Call routeVideo() end." << endl;
	return NULL;
}

#if (1 == (USE_AVTP_VIDEO))
/*-----------------------------------------------------------------------------
描--述：码率调整线程：每隔若干秒调整一次码率。
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
int avtpChangeKbps(AvtpVideoClient *pAvtpVideoClient, unsigned int timeSec)
{
	cout << "Call avtpChangeKbps()." << endl;

	while(g_bRunning)
	{
		this_thread::sleep_for(chrono::seconds(timeSec));
	
		Venc *pVenc = Venc::getInstance();
		Sensor *pSensor = Sensor::getInstance();
		double lossRate = pAvtpVideoClient->getLossRate();
		cout << "lossRate = " << lossRate << endl;
		#if 1
		if(lossRate > 0.9)
		{
			pVenc->changeBitrate(Venc::vencMainChn, 0.1 * 1024);
			//pSensor->setFps(15);
			pAvtpVideoClient->changeFps10s(10);
		}
		else if(lossRate > 0.7)
		{
			pVenc->changeBitrate(Venc::vencMainChn, 0.25 * 1024);
			pAvtpVideoClient->changeFps10s(20);
		}
		else if(lossRate > 0.5)
		{
			pVenc->changeBitrate(Venc::vencMainChn, 0.4 * 1024);
			pAvtpVideoClient->changeFps10s(50);
		}
		else if(lossRate > 0.3)
		{
			pVenc->changeBitrate(Venc::vencMainChn, 0.55 * 1024);
			pAvtpVideoClient->changeFps10s(100);
		}
		else if(lossRate > 0.1)
		{
			pVenc->changeBitrate(Venc::vencMainChn, 0.7 * 1024);
			pAvtpVideoClient->changeFps10s(150);
		}
		else
		{
			pVenc->changeBitrate(Venc::vencMainChn, 0.85 * 1024);
			pAvtpVideoClient->changeFps10s(200);
		}
		#else
		pVenc->changeBitrate(Venc::vencMainChn, 0.5 * 1024);
		#endif
	}

	cout << "Call avtpChangeKbps() end." << endl;
	return 0;
}
#endif

#if (1 == (USE_OSD))
/*-----------------------------------------------------------------------------
描--述：OSD 线程：每秒更新网络状况信息
参--数：无
返回值：
注--意：
-----------------------------------------------------------------------------*/
void *routeOsd(void *arg)
{
	while(g_bRunning)
	{
		const MI_U32 u32StrSize = 128;
		char str[u32StrSize] = {0};
		Rgn *pRgn = Rgn::getInstance();

		// 第0行，显示音视频码率
		MI_S32 s32AudioKbps = 0;
		MI_S32 s32VideoKbps = 0;
		
		snprintf(str, u32StrSize, "Audio:%2dkbps Video:%4dkbps", s32AudioKbps, s32VideoKbps);
		pRgn->setText(Rgn::rgnHandle0, str, I4_RED, DMF_Font_Size_64x64);

		#if 0
		// 第1行，显示wifi 信号强度
		MI_S32 s32WifiSignal = 0;
		Wifi wifi;
		s32WifiSignal = wifi.getApRssi();
		snprintf(str, u32StrSize, "Wifi: %2d dB", s32WifiSignal);
		pRgn->setText(Rgn::rgnHandle1, str, I4_RED, DMF_Font_Size_64x64);
		#endif

		#if 0
		// 第2/3行，显示本地MAC 和IP
		const unsigned ipMacBufSize = 32;
		char ipMacBuf[ipMacBufSize] = {0};
		wifi.getMac(ipMacBuf, ipMacBufSize, "wlan0");
		snprintf(str, u32StrSize, "MAC: %s", ipMacBuf);
		pRgn->setText(Rgn::rgnHandle2, str, I4_RED, DMF_Font_Size_64x64);

		wifi.getIp(ipMacBuf, ipMacBufSize, "wlan0");
		snprintf(str, u32StrSize, "IP %s", ipMacBuf);
		pRgn->setText(Rgn::rgnHandle3, str, I4_RED, DMF_Font_Size_64x64);
		#endif

		// 第4行，显示视频码率
		double dBitRate = 0;
		snprintf(str, u32StrSize, "Video bitrate: %3.2fMbps", dBitRate);
		pRgn->setText(Rgn::rgnHandle4, str, I4_RED, DMF_Font_Size_64x64);

		// 第5/6行，预留
		pRgn->setText(Rgn::rgnHandle5, "line 5: green, green, green", I4_GREEN, DMF_Font_Size_64x64);
		pRgn->setText(Rgn::rgnHandle6, "line 6: black, black, black", I4_BLACK, DMF_Font_Size_64x64);
	
		this_thread::sleep_for(chrono::seconds(1));
	}

	return NULL;
}
#endif

#if (1 == (USE_SPI_PANEL))
/*-----------------------------------------------------------------------------
描--述：SPI PANEL 线程
参--数：无
返回值：
注--意：
-----------------------------------------------------------------------------*/
void *routeSpiPanel(void *arg)
{
	SpiPanel *pSpiPanel = SpiPanel::getInstance();
	int fbColor = 0;
	srand((unsigned)time(NULL));
	fbColor = rand();

	if(0)
	{
		pSpiPanel->panelFill(0, 0, PANEL_WIDTH, PANEL_HEIGHT, fbColor);
	}
	
	unsigned int fontColor = 0xFFFF;
	unsigned int backColor = 0x0000;
	
#if 0
	pSpiPanel->panelDrawPoint(10, 10, fontColor);
	pSpiPanel->panelDrawPoint(20, 10, fontColor);
	pSpiPanel->panelDrawLine(30, 10, 50, 20, fontColor);
	pSpiPanel->panelDrawRectangle(60, 10, 80, 20, fontColor);
	pSpiPanel->panelDrawCircle(100, 20, 10, fontColor);
	
	pSpiPanel->panelShowChar(10, 30, 'a', fontColor, backColor, 12, true);
	pSpiPanel->panelShowChar(30, 30, 'b', fontColor, backColor, 16, true);
	pSpiPanel->panelShowChar(50, 30, 'C', fontColor, backColor, 24, true);
	pSpiPanel->panelShowChar(80, 30, 'D', fontColor, backColor, 32, true);
	
	pSpiPanel->panelShowString(0, 60, "i love you0123456789", fontColor, backColor, 24, true);
	pSpiPanel->panelShowString(0, 90, "LOVE 0123456789", fontColor, backColor, 32, true);

	pSpiPanel->panelShowIntNum(10, 130, -9223372036854775807, fontColor, backColor, 16, true);
	pSpiPanel->panelShowFloatNum(10, 150, -123456789.0123456, fontColor, backColor, 16, true);

	cout << "sizeof(\"中\") = " << sizeof("中") << endl;
	pSpiPanel->panelShowChineseFont(10, 170, "我", fontColor, backColor, 12, true);
	pSpiPanel->panelShowChineseFont(30, 170, "爱", fontColor, backColor, 16, false);
	pSpiPanel->panelShowChineseFont(50, 170, "中", fontColor, backColor, 24, false);
	pSpiPanel->panelShowChineseFont(80, 170, "华", fontColor, backColor, 32, true);
	pSpiPanel->panelShowChineseText(120, 170, "", fontColor, backColor, 24, true);
	pSpiPanel->panelShowPicture(0, 200, 40, 40, pPicQQImage);

	while(g_bRunning)
	{
		sleep(1);
	}

#endif
	return NULL;
}
#endif

#if (1 == (TEST_ETHERNET))
int testEthernet()
{
	const char *interface = NULL;;
	const unsigned int ipBufLen = 128;
	char ipBuf[ipBufLen] = {0};
	
	Ethernet *pEthernet = pEthernet->getInstance();
	
#if 1
	interface = "lo";
	memset(ipBuf, 0, ipBufLen);
	pEthernet->getInterfaceIP(interface, ipBuf, ipBufLen);
	cout << interface << " IP: " << ipBuf << endl;
#endif

#if 1
	interface = "eth0";
	memset(ipBuf, 0, ipBufLen);
	pEthernet->getInterfaceIP(interface, ipBuf, ipBufLen);
	cout << interface << " IP: " << ipBuf << endl;
#endif

#if 1
	interface = "wlan0";
	memset(ipBuf, 0, ipBufLen);
	pEthernet->getInterfaceIP(interface, ipBuf, ipBufLen);
	cout << interface << " IP: " << ipBuf << endl;
#endif

#if 1
	interface = "waln1";
	memset(ipBuf, 0, ipBufLen);
	pEthernet->getInterfaceIP(interface, ipBuf, ipBufLen);
	cout << interface << " IP: " << ipBuf << endl;
#endif

#if 1
	interface = "br0";
	memset(ipBuf, 0, ipBufLen);
	pEthernet->getInterfaceIP(interface, ipBuf, ipBufLen);
	cout << interface << " IP: " << ipBuf << endl;
#endif

#if 1
	interface = "mon.wlan1";
	memset(ipBuf, 0, ipBufLen);
	pEthernet->getInterfaceIP(interface, ipBuf, ipBufLen);
	cout << interface << " IP: " << ipBuf << endl;
#endif
	
	return -1;
}
#endif

#if (1 == (USE_INTERACTION))
int interAction()
{
	cout << endl << endl << endl;
	cout << "Please enter command letter, eg: " << endl;
	cout << "Q, for Quit." << endl;
	cout << "B, for encoder Bitrate." << endl;
	cout << "E, for sensor Exposure time." << endl;
	cout << "F, for sensor frame rate FPS." << endl;
	cout << "W, for isp WDR." << endl;
	cout << "P, for photo sensor value." << endl;
	cout << "L, for LED." << endl;
	cout << "I, for IR-CUT." << endl;
	cout << "T, for changing encoding Type: H.264, H.265, MJPEG. Just support IPC." << endl;
	cout << "R, for changing sub stream Resolution. Just support IPC." << endl;
	cout << "D, for changing Day or Night." << endl;
	cout << "Ctrl + Backspace = delete error input." << endl;
	cout << endl;

	int cmdVal = 0;
	string strCmd = "";
	cin >> strCmd;

	if("Q" == strCmd)
	{
		g_bRunning = false;
	}
	#if (1 == (TEST_CHG_BITRATE) && 1 == (USE_UVC))
	else if("B" == strCmd)
	{
		cout << "Please enter encoder bitrate for H.26x and Mjpeg stream. Not support YUV." << endl;
		cout << "Bitrate range = [2, 100000] Kbps." << endl;

		int bitRate = -1;
		cin >> bitRate;
		while(cin.fail())
		{
			cin.clear();
			cin.ignore();
			cout << "Bad input, please pay attention to the format." << endl;
			cin >> bitRate;
		}

		Venc *pVenc = Venc::getInstance();
		if(pVenc->isChannelExists(MI_VENC_DEV_ID_H264_H265_0, Venc::vencMainChn))
		{
			pVenc->changeBitrate(MI_VENC_DEV_ID_H264_H265_0, Venc::vencMainChn, bitRate);
		}

		if(pVenc->isChannelExists(MI_VENC_DEV_ID_JPEG_0, Venc::vencJpegChn))
		{
			pVenc->changeBitrate(MI_VENC_DEV_ID_JPEG_0, Venc::vencJpegChn, bitRate);
		}
	}
	#endif
	#if (1 == (TEST_CHG_BITRATE) && 1 != (USE_UVC))
	else if("B" == strCmd)
	{
		cout << "Please enter encoder bitrate and which stream do you want to change." << endl;
		cout << "Bitrate range = [2,100000] Kbps. Stream index [0 = sub stream, 1 = main stream]." << endl;
		cout << "For example, change main stream to 1024 Kbps: 1,1024" << endl;
		cout << "For example, change sub stream to 1024 Kbps: 0,1024" << endl;
		
		int bitRate = -1;
		int streamIndex = -1;
		scanf("%d,%d", &streamIndex, &bitRate);

		if(-1 == bitRate || -1 == streamIndex)
		{
			cerr << "Bad input. Please attention format." << endl;
			cin >> strCmd;
		}
		else
		{
			MI_VENC_CHN vencCh = streamIndex;
			Venc *pVenc = Venc::getInstance();
			if(Venc::vencMainChn == vencCh || Venc::vencSubChn == vencCh)
			{
				pVenc->changeBitrate(MI_VENC_DEV_ID_H264_H265_0, vencCh, bitRate);
			}
			else if(Venc::vencJpegChn == vencCh)
			{
				pVenc->changeBitrate(MI_VENC_DEV_ID_JPEG_0, vencCh, bitRate);
			}
			else
			{
				cerr << "No such venc channel, vencCh = " << vencCh << endl;
			}
		}
	}
	#endif
	#if (1 == (TEST_ISP_EXPO))
	else if("E" == strCmd)
	{
		cout << "Please enter sensor exposure time. Range = [1, 1 000 000], unit: us." << endl;

		cin >> cmdVal;
		while(cin.fail())
		{
			cin.clear();
			cin.ignore();
			cout << "Bad input, please pay attention to the format." << endl;
			cin >> cmdVal;
		}

		Isp::getInstance()->setExpoTimeUs(cmdVal);
	}
	#endif
	#if (1 == (TEST_SNR_FPS))
	else if("F" == strCmd)
	{
		cout << "Please enter sensor frame rate FPS. Range = [3, 30], unit: FPS." << endl;

		cin >> cmdVal;
		while(cin.fail())
		{
			cin.clear();
			cin.ignore();
			cout << "Bad input, please pay attention to the format." << endl;
			cin >> cmdVal;
		}

		Sensor *pSensor = Sensor::getInstance();
		pSensor->setFps(cmdVal);
	}
	#endif
	#if (1 == (TEST_ISP_WDR))
	else if("W" == strCmd)
	{
		cout << "Enable WDR? 0: disable, 1: enable." << endl;

		cin >> cmdVal;
		while(cin.fail())
		{
			cin.clear();
			cin.ignore();
			cout << "Bad input, please pay attention to the format." << endl;
			cin >> cmdVal;
		}

		if(0 == cmdVal)
		{
			Isp::getInstance()->disableWDR();
		}
		else
		{
			Isp::getInstance()->enableWDR(1);
		}
	}
	#endif
	#if (1 == (TEST_PHOTO_SENS))
	else if("P" == strCmd)
	{
		int photoVal = 0;
		IrCutLed *pIrCutLed = IrCutLed::getInstance();
		photoVal = pIrCutLed->getPhotoSensVal();
		cout << "Photo sensor value = " << photoVal << endl;
	}
	#endif
	#if (1 == (TEST_LED))
	else if("L" == strCmd)
	{
		cout << "Test led. 0 = all led closed; 1 = while opened; 2 = ir opened; 3 = white and ir opened." << endl;
		cout << "Hbit IR, Lbit White" << endl;
		IrCutLed *pIrCutLed = IrCutLed::getInstance();
		
		cin >> cmdVal;
		while(cin.fail())
		{
			cin.clear();
			cin.ignore();
			cout << "Bad input, please pay attention to the format." << endl;
			cin >> cmdVal;
		}

		switch(cmdVal)
		{
			case 0:
			{
				pIrCutLed->closeLedIr();
				pIrCutLed->closeLedWhite();
				break;
			}
			case 1:
			{
				pIrCutLed->closeLedIr();
				pIrCutLed->openLedWhite();
				break;
			}
			case 2:
			{
				pIrCutLed->openLedIr();
				pIrCutLed->closeLedWhite();
				break;
			}
			case 3:
			{
				pIrCutLed->openLedIr();
				pIrCutLed->openLedWhite();
				break;
			}
			default:
			{
				cerr << "Wrong value." << endl;
				break;
			}
		}
	}
	#endif
	#if (1 == (TEST_IRCUT))
	else if("I" == strCmd)
	{
		cout << "Enable IR-CUT? 0: close, 1: open." << endl;

		cin >> cmdVal;
		while(cin.fail())
		{
			cin.clear();
			cin.ignore();
			cout << "Bad input, please pay attention to the format." << endl;
			cin >> cmdVal;
		}

		IrCutLed *pIrCutLed = IrCutLed::getInstance();
		if(0 == cmdVal)
		{
			pIrCutLed->closeFilter();
		}
		else
		{
			pIrCutLed->openFilter();
		}
	}
	#endif
	#if (1 == TEST_CHG_ENCTYPE && 1 != USE_UVC)
	else if("T" == strCmd)
	{
		cout << "Select encoding Type: 0 = H.264, 1 = H.265." << endl;

		cin >> cmdVal;
		while(cin.fail())
		{
			cin.clear();
			cin.ignore();
			cout << "Bad input, please pay attention to the format." << endl;
			cin >> cmdVal;
		}

		MI_VENC_ModType_e vesType = E_MI_VENC_MODTYPE_VENC;
		if(0 == cmdVal)
		{
			vesType = E_MI_VENC_MODTYPE_H264E;
		}
		else if(1 == cmdVal)
		{
			vesType = E_MI_VENC_MODTYPE_H265E;
		}

		#if (1 == (USE_RTSP_LIVESTREAM))
		auto emEncType = emEncTypeH264;
		if(0 == cmdVal)
		{
			emEncType = emEncTypeH264;
		}
		else if(1 == cmdVal)
		{
			emEncType = emEncTypeH265;
		}
		#endif
	
		int oldWidthSub = 0;
		int oldHeightSub = 0;
		int oldWidthMain = 0;
		int oldHeightMain = 0;
		Venc *pVenc = Venc::getInstance();
		pVenc->getResolution(MI_VENC_DEV_ID_H264_H265_0, Venc::vencSubChn, &oldWidthSub, &oldHeightSub);
		pVenc->getResolution(MI_VENC_DEV_ID_H264_H265_0, Venc::vencMainChn, &oldWidthMain, &oldHeightMain);
		printf("oldWidthSub = %d, oldHeightSub = %d.\n", oldWidthSub, oldHeightSub);
		printf("oldWidthMain = %d, oldHeightMain = %d.\n", oldWidthMain, oldHeightMain);

		#if (1 == (USE_RTSP_LIVESTREAM))
		Live555Rtsp *pLive555Rtsp = Live555Rtsp::getInstance();
		pLive555Rtsp->removeStream("video0");
		pLive555Rtsp->removeStream("video1");
		#endif

 		Scl *pScl = Scl::getInstance();
		// 销毁主码流
		pVenc->stopRecvPic(MI_VENC_DEV_ID_H264_H265_0, Venc::vencMainChn);
		pVenc->destroyChn(MI_VENC_DEV_ID_H264_H265_0, Venc::vencMainChn);
		pScl->destoryPort(Scl::sclPortMain);

		// 销毁子码流
		pVenc->stopRecvPic(MI_VENC_DEV_ID_H264_H265_0, Venc::vencSubChn);
		pVenc->destroyChn(MI_VENC_DEV_ID_H264_H265_0, Venc::vencSubChn);
		pScl->destoryPort(Scl::sclPortSub);

		Sys *pSys = Sys::getInstance();
		// 创建子码流
		unsigned int subW = 1280;
		unsigned int subH = 720;
		pScl->createPort(Scl::sclPortSub, oldWidthSub, oldHeightSub);
		//pVenc->createH26xStream(Venc::vencSubChn, oldWidthSub, oldHeightSub, vesType);
		pVenc->changeBitrate(MI_VENC_DEV_ID_H264_H265_0, Venc::vencSubChn, 0.25 * 1024);
		//pSys->bindScl2Venc(Scl::SclSubPort, Venc::vencSubChn, 30, 30, E_MI_SYS_BIND_TYPE_FRAME_BASE, 0);

		// 创建主码流
		//pVpe->createPort(Vpe::vpeMainPort, oldWidthMain, oldHeightMain);
		//pVenc->createH26xStream(Venc::vencMainChn, oldWidthMain, oldHeightMain, vesType);
		pVenc->changeBitrate(MI_VENC_DEV_ID_H264_H265_0, Venc::vencMainChn, 1 * 1024);
		//pSys->bindVpe2Venc(Vpe::vpeMainPort, Venc::vencMainChn, 30, 30, E_MI_SYS_BIND_TYPE_FRAME_BASE, 0);

		#if (1 == (USE_RTSP_LIVESTREAM))
		int ret = 0;
		ret = pLive555Rtsp->addStream("video1", "video1", emEncType);
		if(0 != ret)
		{
			cerr << "Fail to call pLive555Rtsp->addStream() in routeVideo(). retRtsp = " << ret << endl;
		}

		ret = pLive555Rtsp->addStream("video0", "video0", emEncType);
		if(0 != ret)
		{
			cerr << "Fail to call pLive555Rtsp->addStream() in routeVideo(). retRtsp = " << ret << endl;
		}
		#endif
	}
	#endif
	#if(1 == TEST_CHG_RESO && 1 != USE_UVC)
	else if("R" == strCmd)
	{
		cout << "Please enter sub stream Resolution" << endl;
		cout << "Range = [(192,128), (1920,1080)]. eg: 1280,720" << endl;
		
		int subW = -1;
		int subH = -1;

		do{
			scanf("%d,%d", &subW, &subH);
			if(0 > subW || 0 > subH || 192 > subW || 128 > subH || 1920 < subW || 1080 < subH)
			{
				cerr << "Bad input. Range = [(192,128), (1920,1080)]. eg: 1280,720" << endl;
				cin >> strCmd;
			}
		}while(0 > subW || 0 > subH || 192 > subW || 128 > subH || 1920 < subW || 1080 < subH);
	
		Scl *pScl = Scl::getInstance();
		Venc *pVenc = Venc::getInstance();

		// 获取旧的编码信息
		MI_VENC_ChnAttr_t stChAttr;
		memset(&stChAttr, 0, sizeof(MI_VENC_ChnAttr_t));
		pVenc->getChnAttr(MI_VENC_DEV_ID_H264_H265_0, Venc::vencSubChn, &stChAttr);
		
		// 销毁子码流
		pVenc->stopRecvPic(MI_VENC_DEV_ID_H264_H265_0, Venc::vencSubChn);
		pVenc->destroyChn(MI_VENC_DEV_ID_H264_H265_0, Venc::vencSubChn);
		pScl->destoryPort(Scl::sclPortSub);
		Sys *pSys = Sys::getInstance();
		
		// 创建子码流
		pScl->createPort(Scl::sclPortSub, subW, subH);
		stChAttr.stVeAttr.stAttrH264e.u32MaxPicWidth = subW;
		stChAttr.stVeAttr.stAttrH264e.u32MaxPicHeight = subH;
		stChAttr.stVeAttr.stAttrH264e.u32PicWidth = subW;
		stChAttr.stVeAttr.stAttrH264e.u32PicHeight = subH;
		pVenc->createStreamWithAttr(MI_VENC_DEV_ID_H264_H265_0, Venc::vencSubChn, &stChAttr);
		pSys->bindScl2Venc(Scl::sclPortSub, MI_VENC_DEV_ID_H264_H265_0, Venc::vencSubChn, 30, 30, E_MI_SYS_BIND_TYPE_FRAME_BASE, 0);
	}
	#endif
	#if(1 == TEST_DAY_NIGHT)
	else if("D" == strCmd)
	{
		cout << "Change day or night: 0 = day, 1 = night." << endl;

		cin >> cmdVal;
		while(cin.fail())
		{
			cin.clear();
			cin.ignore();
			cout << "Bad input, please pay attention to the format." << endl;
			cin >> cmdVal;
		}

		// 晚上到白天：关红外灯 -> 开IRCUT -> 加载白天IQ.
		// 白天切晚上：加载夜视IQ -> 关IRCUT -> 开红外灯。
		if(0 == cmdVal)
		{
			IrCutLed *pIrCutLed = IrCutLed::getInstance();
			Isp *pIsp = Isp::getInstance();
			pIrCutLed->closeLedIr();
			pIrCutLed->openFilter();
			pIsp->loadBinFile((char *)"/config/iqfile/335_imx291_day.bin");
		}
		else if(1 == cmdVal)
		{
			IrCutLed *pIrCutLed = IrCutLed::getInstance();
			Isp *pIsp = Isp::getInstance();
			pIsp->loadBinFile((char *)"/config/iqfile/335_imx291_night.bin");
			pIrCutLed->closeFilter();
			pIrCutLed->openLedIr();
		}
		else
		{
			cout << "Error input. 0 = day, 1 = night." << endl;
		}
	}
	#endif
	else
	{
		cout << "Command not recognized." << endl;
	}
	return 0;
}
#endif

