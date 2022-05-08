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

class Mp3Decoder{
public:
	static Mp3Decoder* getInstance();

	int enable();
	int disable();

	int recvPcmFrame(unsigned char*const dataBuff, const unsigned int dataSize);
	int test();
	int pcmFileResample(const char *dstPcmPath, long long int dstRate, long long int dstChLayout, AVSampleFormat dstAvSampleFmt, \
			const char *srcPcmPath, long long int srcRate, long long int srcChLayout, AVSampleFormat srcAvSampleFmt);
	
private:
	Mp3Decoder();
	~Mp3Decoder();
	Mp3Decoder(const Mp3Decoder&);
	Mp3Decoder& operator=(const Mp3Decoder&);

	bool bEnable = false;
	bool bRunning = false;		// stream 线程的运行状态。
};

