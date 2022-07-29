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
	Mp3Decoder();
	~Mp3Decoder();

	int enable();
	int disable();

	int mp3Decoding(const char *filePath);
	int recvPcmFrame(unsigned char*const dataBuff, const unsigned int dataSize);

	int getMp3Attr(const char *filePath, long long int *pSampleRate, long long int *pChLayout, AVSampleFormat *pAvSampleFmt, int *pNbSamples);

	int mp3File2PcmFile(const char *mp3Path, const char *pcmPath);

private:
	bool bRunning = false;		// stream 线程的运行状态。
	bool bIsDecoding = false;	// 正在解码中

	std::mutex mutex;
	std::condition_variable cvProduce;
	std::condition_variable cvConsume;
	unsigned int mDataSize = 0;
	unsigned char *mDataBuf = NULL;

	std::shared_ptr<std::thread> pTh = NULL;
	
	int routeMp3Decoding(const char *filePath);
	static int thRouteMp3Decoding(void *arg, const char *filePath);
};

/* PCM 采样器 */
class PcmResampler{
public:
	PcmResampler(long long int dstSampleRate, long long int dstChLayout, AVSampleFormat _dstAvSampleFmt, 
		long long int srcSampleRate, long long int srcChLayout, AVSampleFormat srcAvSampleFmt, int _srcNbSamples);
	~PcmResampler();

	int pcmDataResample(void *dstPcmData, unsigned int dstPcmLen, 
						const void *srcPcmData, unsigned int srcPcmLen);
	int pcmFileResample(const char *dstPcmPath, const char *srcPcmPath);
private:
	bool bInit = false;
	SwrContext *swrCtx = NULL;

	int inNbChs = 0;		// 输入声道数
	int outNbChs = 0;		// 输出声道数
	int srcBufSize = 0;		// 源数据大小
	int dstBufSize = 0;		// 目标数据大小
	int srcLineSize = 0;
	int dstLineSize = 0;
	unsigned char **srcData = NULL;
	unsigned char **dstData = NULL;
	long long int dstNbSamples = 0;
	long long int dstNbSamplesMax = 0;
	int srcNbSamples = 0;
	AVSampleFormat dstAvSampleFmt = AV_SAMPLE_FMT_NONE;
};
