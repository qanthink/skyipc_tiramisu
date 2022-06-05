/*---------------------------------------------------------------- 
xxx版权所有。
作者：
时间：2020.7.10
----------------------------------------------------------------*/

#pragma once

// audio
#define USE_FAAC_FAAD 0

#define USE_AI 0
#define USE_AI_SAVE_LOCAL_PCM 0
#define USE_AI_SAVE_LOCAL_AAC 0

#define USE_AO 0

// osd
#define USE_OSD 0

// venc
#define USE_VENC 1
#define USE_VENC_SAVE_LOCAL_FILE 0

// wifi and avtp
#define USE_AVTP_AUDIO 0
#define USE_AVTP_VIDEO 1

#define TEST_WIFI 0

#define USE_FFMPEG_SAVE_MP4 0
#define USE_RTSPSERVER_LOCALFILE 0
#define USE_RTSPSERVER_LIVESTREAM 0

#define USE_SPIPANEL 0
#define TEST_ETHERNET 0

extern volatile bool g_bRunning;

void *routeAi(void *arg);
void *routeAo(void *arg);
void *routeVideo(void *arg);
void *routeOsd(void *arg);
void *routeSpiPanel(void *arg);
void recvAudio(unsigned char* buf, unsigned short len);

void startVideo(unsigned char chn);
void stopVideo(unsigned char chn);
void getNextViFrame(unsigned char chn);
void changeBit(unsigned int bitrate, unsigned char);

int testEthernet();

