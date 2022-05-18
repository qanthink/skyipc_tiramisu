/*---------------------------------------------------------------- 
qanthink 版权所有。
作者：
时间：2021.8.13
----------------------------------------------------------------*/

/*
免责声明：
本程序基于ffmpeg 开源代码进行开发，请遵守ffmpeg 开源规则。
*/

#pragma once

// FFMPEG需要使用C风格编译
extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/timestamp.h>
#include <libavutil/file.h>
}

#include <mutex>
#include <thread>
#include <condition_variable>

/*
主要的函数介绍：
void av_dump_format(AVFormatContext *ic, int index, const char *url, int is_output);
功能：
参数：

int avformat_alloc_output_context2(AVFormatContext **ctx, const AVOutputFormat *oformat,
	const char *format_name, const char *filename);
功能：
参数：

AVStream *avformat_new_stream(AVFormatContext *s, const AVCodec *c);
功能：
参数：

void avformat_free_context(AVFormatContext *s);
功能：
参数：

int avformat_write_header(AVFormatContext *s, AVDictionary **options);
功能：
参数：

int av_write_trailer(AVFormatContext *s);
功能：
参数：

int av_interleaved_write_frame(AVFormatContext *s, AVPacket *pkt);
功能：
参数：
*/

/*
设计思路：
建立MP4封装器线程，等候H.26X帧数据。每给对象喂进一个H.26X Frame, 就将该Frame放进容器。
设置视频时长X秒，每存够X秒的视频，就新建下一个视频。
*/

class Mp4Container{
public:
	static Mp4Container* getInstance();

	int enable();
	int disable();

	int sendH26xFrame(const unsigned char*const dataBuff, const unsigned int dataSize);
	
	double setRecordSec(double _recordSec);
	const char *setFileSavePath(const char *path);
	int setCodecPar(unsigned int _width, unsigned int _height, unsigned int _outFPS);

	int isRequestIDR();
	bool isH265VPSFrame(const unsigned char *dataBuf, unsigned int dataSize);
	bool isH265IDRFrame(const unsigned char *dataBuf, unsigned int dataSize);
	//bool isH265SPSFrame(const unsigned char *dataBuf, unsigned int dataSize);
	//bool isH265PPSFrame(const unsigned char *dataBuf, unsigned int dataSize);
private:
	Mp4Container();
	~Mp4Container();
	Mp4Container(const Mp4Container&);
	Mp4Container& operator=(const Mp4Container&);

	bool bEnable = false;
	bool bRunning = false;		// stream 线程的运行状态。
	double recordSec = 60.0;
	unsigned int outFPS = 30;
	unsigned int width = 1920;
	unsigned int height = 1080;
	const char *fileSavePath = "/mnt/linux/Downloads/videotest";

	unsigned int mDataSize = 0;
	const unsigned char*mDataBuf = NULL;

	// 类中创建线程，必须使用一个空格函数传递this 指针。
	void *route(void *arg);
	static void *thRoute(void *arg);
	std::mutex mutex;
	std::condition_variable cvProduce;
	std::condition_variable cvConsume;
	std::shared_ptr<std::thread> pTh = NULL;

	const char *getTimeString(char *timeStrBuf, const unsigned int timeStrSize);
	const char *getFileFullName(char *fileNameBuf, const unsigned int fileNameSize);
};

