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

#include "testing.h"
#include "venc.h"
#include "ai.hpp"
#include "ao.hpp"
#include "aac.h"
#include "aad.h"
#include "rgn.h"
#include "avtp.h"
#include "wifi.h"
#include "ffmpeg.h"
#include "live555rtsp.h"
#include "spipanel.h"
#include "ethernet.h"
#include "avtp_client.h"

using namespace std;

volatile bool g_bRunning = true;

/*-----------------------------------------------------------------------------
描--述：开启视频流。
参--数：chn 通道号，0 表示主码流，1 表示子码流。
返回值：无
注--意：非必要接口，设计原意是在此调用MI 接口初始化VENC. 实际情况是，软件运行一开始就已经对VENC 做了初始化。
	个人认为，"音视频传输协议"只需要考虑数据的稳定传输，不需要考虑开启或者停止MI 底层相关的东西。
	最好的方法是去掉这个接口，在命令控制的回调函数中给一个开启的消息即可，用户根据消息去做对应的行为。
-----------------------------------------------------------------------------*/
void startVideo(unsigned char chn)
{
	cout << "Call MindSdk::startVideo(), chn = " << (MI_U32)chn << endl;
	cout << "Call MindSdk::startVideo() end." << endl;
	return;
}

/*-----------------------------------------------------------------------------
描--述：停止视频流。
参--数：chn 通道号，0 表示主码流，1 表示子码流。
返回值：无
注--意：非必要接口，设计原意是在此调用MI 接口禁用VENC. 
	但个人认为，"音视频传输协议"只需要考虑数据的稳定传输，不需要考虑开启或者停止MI 底层相关的东西。
	最好的方法是：去掉这个接口，在命令控制的回调函数中给一个停止的消息即可，用户根据消息去做对应的行为。
-----------------------------------------------------------------------------*/
void stopVideo(unsigned char chn)
{
	cout << "Call MindSdk::stopVideo(), chn = " << (MI_U32)chn << endl;
	cout << "Call MindSdk::stopVideo() end." << endl;
	return;
}

/*-----------------------------------------------------------------------------
描--述：改变码流速率，MindSDK 所需的回调函数。
参--数：bitrate 比特率，单位是bits.
返回值：无
注--意：非必要接口，如果不想改变码流速率，可以不调用。详情咨询jerry.
-----------------------------------------------------------------------------*/
void changeBit(unsigned int bitrate, unsigned char) 	// bit 2000 * 1000, 1200 * 1000, 600 * 1000, 300 * 1000.
{
	cout << "Call MindSdk::changeBit(), bitrate = " << bitrate << endl;

	MI_S32 s32Ret = 0;
	Venc *pVenc = Venc::getInstance();
	s32Ret = pVenc->changeBitrate(0, bitrate / 1024);
	if(0 != s32Ret)
	{
		cerr << "Fail to call pVenc->changeBitrate(), errno = " << s32Ret << endl;
	}

	cout << "Call MindSdk::changeBit() end." << endl;	
	return;
}

#if (1 == (USE_MINDSDK_VIDEO))
/*-----------------------------------------------------------------------------
描--述：获取下一帧数据
参--数：chn 通道号，0 表示主码流，1 表示子码流。
返回值：无
注--意：必要接口！
	必须通过该接口获取MI 的数据，紧接着在该函数内部调用Mindsdk_Video::insertViFrame() 将数据喂给传输协议。
	异常用法：
	有用户存在这样的错误用法：没有设置cb_getNextViFrame() 这个回调函数，
	然后从VENC 中recStream()并立刻调用insertViFrame() 喂数据给音视频传输协议。
	这样做会导致网络环境较差时出现花屏现象，因此，务必设置此回调，在此回调中传输数据。
-----------------------------------------------------------------------------*/
void getNextViFrame(unsigned char chn)
{
	//cout << "Call MindSdk::getNextViFrame(), chn = " << (MI_U32)chn << endl;

	if(Venc::vencMainChn != (MI_U32)chn && Venc::vencSubChn != (MI_U32)chn)
	{
		cerr << "Fail to call getNextViFrame(), invalid venc chanel!" << endl;
		return;
	}
	
	int ret = 0;
	#if 0
	Venc::StreamPack_t streamPack;
	memset(&streamPack, 0, sizeof(Venc::StreamPack_t));
	Venc *pVenc = Venc::getInstance();
	ret = pVenc->rcvStream((MI_U32)chn, &streamPack);
	if(0 != ret)
	{
		cerr << "Fail to call pVenc->rcvStream(), ret = " << ret << endl;
		return;
	}

	#if 0	//debug
	cout << "streamPack.u32Len = " << streamPack.u32Len << endl;
	#endif

	bool bIframe = true;
	Mindsdk_Video *pMindsdk_video = Mindsdk_Video::getInstance();
	pMindsdk_video->insertViFrame((void *)streamPack.u8Pack, streamPack.u32Len, bIframe, streamPack.u64PTS, chn);
	#endif
	
	//cout << "Call MindSdk::getNextViFrame() end." << endl;
	return;
}
#endif

/*-----------------------------------------------------------------------------
描--述：mindSDK 获取音频传输数据的回掉函数。
参--数：buf 音频帧缓冲区；len 音频帧长度。
返回值：无
注--意：1.音频数据为AAC 格式，需要通过faad 进行解码。
		2.该回调函数不“重入”，一次调用结束，才会进行二次调用。详情咨询jerry.
-----------------------------------------------------------------------------*/
void recvAudio(unsigned char *buf, unsigned short len)
{
	//cout << "Call recvAudio(), len = " << len << endl;

	void *pcmBuf = NULL;
	Aad *pAad = pAad->getInstance();
	NeAACDecFrameInfo decoderFrameInfo;
	memset(&decoderFrameInfo, 0, sizeof(NeAACDecFrameInfo));

	// step1: 解码一帧数据。
	pcmBuf = pAad->decDecode(&decoderFrameInfo, buf, len);
	if(NULL == pcmBuf)
	{
		cerr << "Fail to call pAad->enDecoder(), return null value!" << endl;
		return;
	}
	else if(decoderFrameInfo.error > 0 || decoderFrameInfo.samples <= 0)
	{
		cerr << "Fail to call pAad->enDecoder(): " << endl;
		return;
	}

	#if 0	// debug
	printf("frame info: bytesconsumed %ld, channels %d, samples %ld, samplerate %ld\n", \
		decoderFrameInfo.bytesconsumed, decoderFrameInfo.channels, decoderFrameInfo.samples, decoderFrameInfo.samplerate);
	#endif

#if 0
	// step2: 立体声转单声道，并用转换后的数据填充AO 结构体。
	stAOFrame_t stAOframe;
	memset(&stAOframe, 0, sizeof(stAOFrame_t));

	pAad->stereo2mono((unsigned char*)pcmBuf, decoderFrameInfo.samples * 2, stAOframe.audioBuf, NULL);
	stAOframe.stAudioFrame.u32Len = decoderFrameInfo.samples;

	// step3: 将AO 数据塞给AO 模块播放。
	audioOut::getInstance()->sendStream(&stAOframe);
	//usleep(1 * 1000);		// 若不阻塞，存在AO 拿数据不及时，踩内存的现象。2020.7.15 15:00
							// 2020.7.15 23:00 更新AO 模块，修复该问题。
#endif
	return;
}

/*-----------------------------------------------------------------------------
描--述：AI 线程。不断地获取音频原始数据，编码后塞给mindSDK 传输。
参--数：无
返回值：无
注--意：AI 处理流程：获取PCM流 -> 编码为AAC流 -> mindSDK 传输。
		注意循环结束后，释放自由空间的内存。
-----------------------------------------------------------------------------*/
void *routeAi(void *arg)
{
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

	unsigned int uFrameCnt = 20 * 16000 / 1000 * 1.05;	// n * samples / 1000 = n 秒。

	#if (1 == (USE_AI_SAVE_LOCAL_PCM))
	const char *filePathPcm = "/mnt/linux/Downloads/audio.pcm";
	ofstream ofsPcm;
	ofsPcm.open(filePathPcm, ios::trunc);
	if(ofsPcm.fail())
	{
		cerr << "Fail to open " << filePathPcm << ", " << strerror(errno) << endl;
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
	#endif

	while(g_bRunning)
	{
		MI_S32 s32Ret = 0;
		stAIFrame_t stAudioFrame;
		memset(&stAudioFrame, 0, sizeof(stAudioFrame));
		
		s32Ret = AudioIn::getInstance()->recvStream(&stAudioFrame);
		if(0 != s32Ret)
		{
			cerr << "Fail to call pAudioIn->rcvStream(). s32Ret = " << s32Ret << endl;
			continue;
		}

		#if 0	// debug
		cout << "[AI bLoudSoundDetected] = " << (int)stAudioFrame.bLoudSoundDetected << endl;
		cout << "stAudioFrame.u32Len = " << stAudioFrame.u32Len << endl;
		cout << "stAudioFrame.eBitWidth = " << stAudioFrame.eBitWidth << endl;
		cout << "stAudioFrame.eSoundmode = " << stAudioFrame.eSoundmode << endl;
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
			ofsPcm.write((const char *)stAudioFrame.apFrameBuf, stAudioFrame.u32Len / 2);
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

		//audioOut::getInstance()->sendStream(&stAudioFrame);

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
		
		#if (1 == (USE_MINDSDK_AUDIO))
		Mindsdk_Audio::getInstance()->sendAudio(aacBuf, ret);
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

	return NULL;
}

/*-----------------------------------------------------------------------------
描--述：AO 线程。从本地PCM 文件中读取数据，塞给AO 模块进行播放。
参--数：arg 本地文件的路径
返回值：无
注--意：
-----------------------------------------------------------------------------*/
void *routeAo(void *arg)
{
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
		cerr << "Fail to call open(2), " << strerror(errno) << endl;
		return NULL;
	}

	cout << "Success to call open() in routeAo()." << endl;

	do{
		int readBytes = 0;
		unsigned int dataBufMaxSize = 2 * 1024;
		char dataBuf[dataBufMaxSize] = {0};
		
		cout << "Ready to call read() in routeAo()." << endl;
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
		audioOut::getInstance()->sendStream(dataBuf, readBytes);
	}while(g_bRunning);

	if(-1 != fd)
	{
		close(fd);
		fd = -1;
	}

	return NULL;
}

/*-----------------------------------------------------------------------------
描--述：VIDEO 线程：从VENC 中获取若干帧数据保存到本地。
参--数：arg, VENC 通道号。
返回值：
注--意：数据保存的路径为/customer/video.h265
-----------------------------------------------------------------------------*/
void *routeVideo(void *arg)
{
	MI_U32 u32VencChn = (MI_U32)arg;
	if(Venc::vencMainChn != u32VencChn && Venc::vencSubChn != u32VencChn && Venc::vencJpegChn != u32VencChn)
	{
		cerr << "Fail to call routeVideo(), invalid chanel!" << endl;
		return NULL;
	}

	#if (1 == (USE_VENC_SAVE_LOCAL_FILE))
	int fd = -1;
	unsigned int uFrameCnt = 10 * 30;		// 写入文件的帧数。N * FPS = N秒。
	//const char *filePath = "/customer/video.265";
	const char *filePath = "/mnt/linux/Downloads/video.265";
	mode_t mode = 0766;
	
	fd = open(filePath, O_WRONLY | O_CREAT | O_TRUNC, mode);		// 只写 | 如果不存在则创建 | 如果存在则清空
	if(-1 == fd)
	{
		cerr << "Fail to open " << filePath << ", " << strerror(errno) << endl;
	}
	#endif

	#if (1 == (USE_AVTP_VIDEO))
	AvtpVideoClient avtpVideClient("192.168.0.100", "192.168.0.5");
	#endif
	
	while(g_bRunning)
	{
		MI_S32 s32Ret = 0;
		MI_VENC_Stream_t stStream;
		Venc *pVenc = Venc::getInstance();
		s32Ret = pVenc->rcvStream(u32VencChn, &stStream);
		if(0 != s32Ret)
		{
			cerr << "Fail to call pVenc->rcvStream(). s32Ret = " << s32Ret << endl;
			continue;
		}

		int i = 0;		// 2020.7.22 增加for 循环，适配slice mode 下数据需要多片分发。
		for(i = 0; i < stStream.u32PackCount; ++i)
		{
			if(stStream.pstPack[i].u32Len > stStreamPack_t::u32PackMaxLen)
			{
				cerr << "stStream.pstPack[" << i << "].u32Len is out of range." << endl;
				break;
			}

			#if 0	// debug
			//pVenc->printStreamInfo(&stStream);
			cout << "stStream.pstPack[i].u32Len = " << stStream.pstPack[i].u32Len << endl;
			#endif
			
			#if (1 == (USE_FFMPEG_SAVE_MP4))
			Ffmpeg *pFfmpeg = Ffmpeg::getInstance();
			s32Ret = pFfmpeg->sendH26xFrame(stStream.pstPack[i].pu8Addr, stStream.pstPack[i].u32Len);
			if(0 != s32Ret)
			{
				cerr << "Fail to call pFfmpeg->sendH26xFrame(). s32Ret = " << s32Ret << endl;
			}
			#endif

			#if (1 == (USE_RTSPSERVER_LIVESTREAM))
			Live555Rtsp *pLive555Rtsp = Live555Rtsp::getInstance();
			//pLive555Rtsp->sendH26xFrame_block(stStream.pstPack[i].pu8Addr, stStream.pstPack[i].u32Len);
			pLive555Rtsp->sendH26xFrame(stStream.pstPack[i].pu8Addr, stStream.pstPack[i].u32Len);
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
			avtpVideClient.sendVideoFrame(stStream.pstPack[i].pu8Addr, stStream.pstPack[i].u32Len);
			if(!g_bRunning)
			{
				avtpVideClient.stop();
			}
			double lossRate = avtpVideClient.getLossRate();
			if(lossRate >= 0.9)
			{
				pVenc->changeBitrate(Venc::vencMainChn, 0.5 * 1024);
			}
			else if(lossRate >= 0.7)
			{
				pVenc->changeBitrate(Venc::vencMainChn, 1 * 1024);
			}
			else if(lossRate > 0.4)
			{
				pVenc->changeBitrate(Venc::vencMainChn, 2 * 1024);
			}
			else
			{
				pVenc->changeBitrate(Venc::vencMainChn, 4 * 1024);
			}
			#endif
		}

		s32Ret = pVenc->releaseStream(u32VencChn, &stStream);
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

	return NULL;
}

/*-----------------------------------------------------------------------------
描--述：OSD 线程：每秒更新网络状况信息
参--数：无
返回值：
注--意：
-----------------------------------------------------------------------------*/
#if (1 == (USE_MINDSDK_VIDEO))
extern std::atomic<int> wifiSignal;		// wifi信号强度
extern std::string preWifiMac;			// 上级wifi的mac地址
extern std::string localMac;			// 本地mac地址
extern std::atomic<int> currentBitRate;	// 当前码率
#else
int currentBitRate = 0;	// 当前码率
#endif
void *routeOsd(void *arg)
{
	while(g_bRunning)
	{
		const MI_U32 u32StrSize = 128;
		char str[u32StrSize] = {0};
		Rgn *pRgn = Rgn::getInstance();
		#if (1 == (USE_MINDSDK_VIDEO))
		Udper *pUdper = Udper::getInstance();
		#endif

		// 第0行，显示音视频码率
		MI_S32 s32AudioKbps = 0;
		MI_S32 s32VideoKbps = 0;
		#if (1 == (USE_MINDSDK_VIDEO))
		s32AudioKbps = Udper::getInstance()->audioBitRate * 8;
		s32VideoKbps = Udper::getInstance()->videoBitRate * 8;
		#endif
		snprintf(str, u32StrSize, "Audio:%2dkbps Video:%4dkbps", s32AudioKbps, s32VideoKbps);
		pRgn->setText(Rgn::rgnHandle0, str, I4_RED, DMF_Font_Size_32x32);

		// 第1行，显示wifi 信号强度
		MI_S32 s32WifiSignal = 0;
		#if (1 == (TEST_WIFI))
		s32WifiSignal = Wifi::getInstance()->getApRssi();
		#endif
		snprintf(str, u32StrSize, "Wifi: %2d dB", s32WifiSignal);
		pRgn->setText(Rgn::rgnHandle1, str, I4_RED, DMF_Font_Size_32x32);

		// 第2/3行，显示本地和上级设备MAC
		const unsigned macBufSize = 32;
		char macBuf[macBufSize] = {0};
		#if (1 == (TEST_WIFI))
		Wifi::getInstance()->getLocalMac(macBuf, macBufSize, "eth0");
		#endif
		snprintf(str, u32StrSize, "Local MAC: %s", macBuf);
		pRgn->setText(Rgn::rgnHandle2, str, I4_RED, DMF_Font_Size_32x32);

		#if (1 == (TEST_WIFI))
		Wifi::getInstance()->getPreMac(macBuf, macBufSize);
		#endif
		snprintf(str, u32StrSize, "AP MAC: %s", macBuf);
		pRgn->setText(Rgn::rgnHandle3, str, I4_RED, DMF_Font_Size_32x32);

		// 第4行，显示视频码率
		double dBitRate = (double)currentBitRate / (1000 * 1000);
		snprintf(str, u32StrSize, "Video bitrate: %3.2fMbps", dBitRate);
		pRgn->setText(Rgn::rgnHandle4, str, I4_RED, DMF_Font_Size_32x32);

		// 第5/6行，预留
		pRgn->setText(Rgn::rgnHandle5, "line 5: green, green, green", I4_GREEN, DMF_Font_Size_32x32);
		pRgn->setText(Rgn::rgnHandle6, "line 6: black, black, black", I4_BLACK, DMF_Font_Size_32x32);
	
		this_thread::sleep_for(chrono::seconds(1));
	}

	return NULL;
}

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
