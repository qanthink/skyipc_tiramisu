/*---------------------------------------------------------------- 
xxx版权所有。
作者：
时间：2020.7.10
----------------------------------------------------------------*/

#pragma once

// 人机交互
#define USE_INTERACTION		1

// 改变码率
#define TEST_CHG_BITRATE	1

// ISP EXPO
#define TEST_ISP_EXPO		1

// ISP WDR
#define TEST_ISP_WDR		1

// SNR FPS
#define TEST_SNR_FPS		1

// Photosensor
#define TEST_PHOTO_SENS		1

// LED, White and IR
#define TEST_LED			1

// IR-CUT
#define TEST_IRCUT			1

// Change ENC Type
#define TEST_CHG_ENCTYPE	1

// Change Resolution
#define TEST_CHG_RESO		1

// Change Day and Night
#define TEST_DAY_NIGHT		1

// UVC or IPC
//#define USE_IPC				1	// 在Cmake 中定义了

// IQ Server
#define USE_IQ_SERVER		1

// audio
#define USE_AI				1
#define USE_AI_SAVE_LOCAL_PCM		0
#define USE_AI_SAVE_LOCAL_AAC		0

#define USE_AO				1
#define USE_AO_LOCAL_FILE	0
#define USE_AO_NET_PCM		0

#define USE_FAAC_FAAD		0

// osd
#define USE_OSD				0

// divp
#define USE_DIVP			0

// ffmpeg
#define USE_FFMPEG_SAVE_MP4	0

#if (1 == (USE_IPC))
// venc
#define USE_VENC_MAIN		1
#define USE_VENC_SUB		0
#define USE_VENC_JPEG		0
#define USE_VENC_SAVE_LOCAL_FILE	0
#define SAVE_TIME_SECONDS	10

// rtsp
#define USE_RTSP_LOCALFILE	0
#define USE_RTSP_LIVESTREAM	1
#endif

#if (1 == (USE_UVC))
// venc
#define USE_VENC_MAIN		0
#define USE_VENC_SUB		0
#define USE_VENC_JPEG		0
#define USE_VENC_SAVE_LOCAL_FILE	0
#define SAVE_TIME_SECONDS	10

// rtsp
#define USE_RTSP_LOCALFILE	0
#define USE_RTSP_LIVESTREAM	0
#endif

// wifi and avtp
#define USE_AVTP_AUDIO		0
#define USE_AVTP_VIDEO		0

#define USE_SPI_PANEL		0
#define TEST_ETHERNET		0

extern volatile bool g_bRunning;

#if (1 == (USE_AVTP_AUDIO))
#include "atp_client.h"
extern AvtpAudioClient *pAvtpAudioClient;
#endif

#if (1 == (USE_AVTP_VIDEO))
#include "vtp_client.h"
extern AvtpVideoClient *pAvtpVideoClient;
#endif

#if (1 == (USE_RTSP_LOCALFILE) || 1 == (USE_RTSP_LIVESTREAM))
#include "live555rtsp.h"
#endif

void *routeAi(void *arg);
void *routeAoFromFile(void *arg);
void *routeAoNetPcm(void *arg);
void *routeVideo(int vencDev, int vencCh);
void *routeOsd(void *arg);
void *routeSpiPanel(void *arg);
void recvAudio(unsigned char* buf, unsigned short len);

int testEthernet();

#if (1 == (USE_INTERACTION))
int interAction();
#endif

