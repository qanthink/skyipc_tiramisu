/*---------------------------------------------------------------- 
qanthink 版权所有。
作者：
时间：2022.5.8
----------------------------------------------------------------*/

/*
免责声明：
本程序基于ffmpeg 开源代码进行开发，请遵守ffmpeg 开源规则。
*/

#pragma once

// FFMPEG需要使用C风格编译
extern "C" {
#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
}

#include <mutex>
#include <thread>
#include <condition_variable>

class Mp3Decoder{
public:
	#if 0
	static Mp3Decoder* getInstance();
	#else
	Mp3Decoder();
	~Mp3Decoder();
	#endif

	int enable();
	int disable();

	int mp3Decoding(const char *filePath);
	int recvPcmFrame(unsigned char*const dataBuff, const unsigned int dataSize);

	int getMp3Attr(const char *filePath, long long int *pSampleRate, long long int *pChLayout, AVSampleFormat *pAvSampleFmt, int *pNbSamples);

	int mp3ToPcm(const char *mp3Path, const char *pcmPath);
	
	int pcmFileResample(const char *dstPcmPath, long long int dstSampleRate, 
			long long int dstChLayout, AVSampleFormat dstAvSampleFmt, 
			const char *srcPcmPath, long long int srcSampleRate, 
			long long int srcChLayout, AVSampleFormat srcAvSampleFmt, int srcNbSamples);
	int pcmDataResample(void *dstPcmData, unsigned int dstPcmLen, long long int dstSampleRate, 
			long long int dstChLayout, AVSampleFormat dstAvSampleFmt, 
			const void *srcPcmData, unsigned int srcPcmLen, long long int srcSampleRate, 
			long long int srcChLayout, AVSampleFormat srcAvSampleFmt, int srcNbSamples);

private:
	#if 0
	Mp3Decoder();
	~Mp3Decoder();
	Mp3Decoder(const Mp3Decoder&);
	Mp3Decoder& operator=(const Mp3Decoder&);
	#endif

	bool bEnable = false;
	bool bRunning = false;		// stream 线程的运行状态。
	bool bIsDecoding = false;

	std::mutex mutex;
	std::condition_variable cvProduce;
	std::condition_variable cvConsume;
	unsigned int mDataSize = 0;
	unsigned char *mDataBuf = NULL;

	std::shared_ptr<std::thread> pTh = NULL;
	
	int routeMp3Decoding(const char *filePath);
	static int thRouteMp3Decoding(void *arg, const char *filePath);
};

