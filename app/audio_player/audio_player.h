/*---------------------------------------------------------------- 
qanthink 版权所有。
作者：
时间：2022.4.30
----------------------------------------------------------------*/

/*
*/

#pragma once

#include "ao.hpp"

class AudioPlayer{
public:
	static AudioPlayer* getInstance();

	int playPCM(const char *filePath);
	int playWAV(const char *filePath);
	int playMP3(const char *filePath);
	
	int readWavHead(const char *filePath);

private:
	AudioPlayer();
	~AudioPlayer();
	AudioPlayer(const AudioPlayer&);
	AudioPlayer& operator=(const AudioPlayer&);

	bool bPlaying = false;

	int playRoutePCM(const char *filePath);
	static int thPlayRoutePCM(const char *filePath);
	
	int playRouteWAV(const char *filePath);
	static int thPlayRouteWAV(const char *filePath);
	
	int playRouteMP3(const char *filePath);
	static int thPlayRouteMP3(const char *filePath);

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

