/*---------------------------------------------------------------- 
qanthink 版权所有。
作者：
时间：2022.4.30
----------------------------------------------------------------*/

/*
*/

#pragma once

#include "ao.hpp"
#include <thread>

class AudioPlayer{
public:
	AudioPlayer();
	~AudioPlayer();

	int playPCM(const char *filePath, bool bBlock);
	int playWAV(const char *filePath, bool bBlock);
	int playMP3(const char *filePath, bool bBlock);
	
	int readWavHead(const char *filePath);

private:
	bool bPlaying = false;					// 控制播放暂停、停止。
	std::shared_ptr<std::thread> pTh = NULL;

	int playRoutePCM(const char *filePath);
	static int thPlayRoutePCM(AudioPlayer* pThis, const char *filePath);
	
	int playRouteWAV(const char *filePath);
	static int thPlayRouteWAV(AudioPlayer* pThis, const char *filePath);
	
	int playRouteMP3(const char *filePath);
	static int thPlayRouteMP3(AudioPlayer* pThis, const char *filePath);

	int getWavHeaderBytes(const char *filePath);
};


// 16 Bytes WAV FMT
typedef struct
{
	signed short wFormatTag;
	signed short wChannels;
	unsigned int dwSamplesPerSec;
	unsigned int dwAvgBytesPerSec;
	signed short wBlockAlign;
	signed short wBitsPerSample;
}stWavFmt_t;

// 36(44-8) Bytes File Header
typedef struct
{
	char chRIFF[4];
	unsigned int dwRIFFLen;
	char chWAVE[4];
	char chFMT[4];
	unsigned int dwFMTLen;
	stWavFmt_t wave;
	//char chDATA[4];
	//unsigned int dwDATALen;
}stWavFileHeader_t;

