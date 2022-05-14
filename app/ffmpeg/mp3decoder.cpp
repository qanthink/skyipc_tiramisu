/*---------------------------------------------------------------- 
qanthink 版权所有。
作者：
时间：2022.5.8
----------------------------------------------------------------*/

/*
免责声明：
本程序基于ffmpeg 开源代码进行开发，请遵守ffmpeg 开源规则。
*/

#include "mp3decoder.h"
#include <fstream>
#include <iostream>
#include <iomanip>

using namespace std;

Mp3Decoder* Mp3Decoder::getInstance()
{
	static Mp3Decoder ffmpeg;
	return &ffmpeg;
}

Mp3Decoder::Mp3Decoder()
{
	enable();
}

Mp3Decoder::~Mp3Decoder()
{
	disable();
}

/*-----------------------------------------------------------------------------
描--述：
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
int Mp3Decoder::enable()
{
	cout << "Call Mp3Decoder::enable()." << endl;

	if(bEnable)
	{
		return 0;
	}

	bEnable = true;

	cout << "Call Mp3Decoder::enable() end." << endl;
	return 0;
}

/*-----------------------------------------------------------------------------
描--述：
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
int Mp3Decoder::disable()
{
	cout << "Call Mp3Decoder::disable()." << endl;

	bEnable = false;
	
	cout << "Call Mp3Decoder::disable() end." << endl;
	return 0;
}

/*
	功能：	解码MP3 文件
	返回：	
	注意：	与recvPcmFrame 配对使用。
*/
int Mp3Decoder::mp3Decoding(const char *filePath)
{

	cout << "Call AudioPlayer::mp3Decoding()." << endl;
	thread th(thRouteMp3Decoding, this, filePath);
	th.detach();
	cout << "Call AudioPlayer::mp3Decoding() end." << endl;
	return 0;
}

int Mp3Decoder::thRouteMp3Decoding(void *arg, const char *filePath)
{
	Mp3Decoder *pThis = (Mp3Decoder *)arg;
	return pThis->routeMp3Decoding(filePath);
}

/*
	功能：	解码MP3 文件。线程函数，内部实现。
	返回：	成功，返回0；
			-1, 文件无法打开。
			-2, 音频解码器初始化失败。
	注意：	与recvPcmFrame 配对使用。
*/
int Mp3Decoder::routeMp3Decoding(const char *filePath)
{
	// 打开MP3 输入文件
	AVFormatContext *avFmtCtx = NULL;
	avFmtCtx = avformat_alloc_context();
	
	int ret = 0;
	ret = avformat_open_input(&avFmtCtx, filePath, NULL,NULL);
	if(ret < 0)
	{
		cerr << "In Mp3Decoder::mp3Decoding(): Fail to open file " << filePath << endl;
		return -1;
	}

	// 寻找音视频流
	ret = avformat_find_stream_info(avFmtCtx, NULL);
	if(ret < 0)
	{
		cerr << "In Mp3Decoder::mp3Decoding(): Cannot find any stream in file." << endl;
		return -2;
	}

	// dump 流信息。
	av_dump_format(avFmtCtx, 0, filePath, 0);

	// 寻找音频流索引值
	int i = 0;
	int ASIndex = -1;
	for(i = 0; i < avFmtCtx->nb_streams; ++i)
	{
		if(AVMEDIA_TYPE_AUDIO == avFmtCtx->streams[i]->codecpar->codec_type)
		{
			ASIndex = i;
			break;
		}
	}
	
	if(-1 == ASIndex)
	{
		cerr << "In  Mp3Decoder::mp3Decoding(): Cannot find audio stream.\n" << endl;
		return -2;
	}

	// 根据音频流寻找解码器。
	AVCodec *codec = NULL;
	AVCodecParameters *aCodecPara = NULL;
	
	aCodecPara = avFmtCtx->streams[ASIndex]->codecpar;
	codec = avcodec_find_decoder(aCodecPara->codec_id);
	if(!codec)
	{
		cerr << "In  Mp3Decoder::mp3Decoding(): Cannot find any codec for audio." << endl;
		return -2;
	}

	// 使用参数初始化解码器上下文。
	AVCodecContext *avCodecCtx = NULL;
	avCodecCtx = avcodec_alloc_context3(codec);
	ret = avcodec_parameters_to_context(avCodecCtx, aCodecPara);
	if(ret < 0)
	{
		cerr << "In  Mp3Decoder::mp3Decoding(): Cannot alloc codec context." << endl;
		return -2;
	}

	// 打开解码器
	avCodecCtx->pkt_timebase = avFmtCtx->streams[ASIndex]->time_base;
	ret = avcodec_open2(avCodecCtx, codec, NULL);
	if(ret < 0)
	{
		printf("In  Mp3Decoder::mp3Decoding(): Cannot open audio codec.\n");
		return -2;
	}

	AVPacket *avPacket = NULL;
	avPacket = av_packet_alloc();
	AVFrame *avFrame = NULL;
	avFrame = av_frame_alloc();

	while(av_read_frame(avFmtCtx, avPacket) >= 0)
	{
		if(avPacket->stream_index != ASIndex)
		{
			continue;
		}
		
		ret = avcodec_send_packet(avCodecCtx, avPacket);
		if(ret < 0)
		{
			continue;
		}

		while(avcodec_receive_frame(avCodecCtx, avFrame) >= 0)
		{
			/* Planar（平面），其数据格式排列方式为 (特别记住，该处是以点nb_samples采样点来交错，不是以字节交错）:
			LLLLLLRRRRRRLLLLLLRRRRRRLLLLLLRRRRRRL...（每个LLLLLLRRRRRR为一个音频帧）
			而不带P的数据格式（即交错排列）排列方式为：
			LRLRLRLRLRLRLRLRLRLRLRLRLRLRLRLRLRLRL...（每个LR为一个音频样本）*/
			if(av_sample_fmt_is_planar(avCodecCtx->sample_fmt))
			{
				int i = 0;
				int numBytes = 0;
				const unsigned int pcmLen = 1024 * 16;
				unsigned char pcmData[pcmLen] = {0};
				numBytes = av_get_bytes_per_sample(avCodecCtx->sample_fmt);
				
				//pcm播放时是LRLRLR格式，所以要交错保存数据
				for(i = 0; i < avFrame->nb_samples; ++i)
				{
					int ch = 0;
					for(ch = 0; ch < avCodecCtx->channels; ++ch)
					{
						memcpy(pcmData + numBytes * (i * avCodecCtx->channels + ch), avFrame->data[ch] + numBytes * i, numBytes);
					}
				}
				
				unique_lock<std::mutex> lock(mutex);
				while(0 != mDataSize)			// mData 不为0, 表示ffmpeg 依然有未处理完的数据。
				{
					cvProduce.wait(lock);
				}
				mDataBuf = pcmData;
				mDataSize = numBytes * i * avCodecCtx->channels;
				cvConsume.notify_one();
			}
		}

		av_packet_unref(avPacket);
	}

	av_frame_free(&avFrame);
	av_packet_free(&avPacket);
	avcodec_close(avCodecCtx);
	avcodec_free_context(&avCodecCtx);
	avformat_free_context(avFmtCtx);

	return 0;
}

/*
	功能：	
	返回：	
	注意：	
*/
int Mp3Decoder::recvPcmFrame(unsigned char *const dataBuff, const unsigned int dataSize)
{
	//cout << "Call Mp3Decoder::recvPcmFrame()." << endl;

	unique_lock<std::mutex> lock(mutex);
	while(0 == mDataSize)		// mDataSize 为0 表示已经没有要处理的数据，通知生产者传递数据。
	{
		cvConsume.wait(lock);
	}

	//cout << "received." << endl;
	int realSize = 0;
	if(dataSize < mDataSize)
	{
		cerr << "In Mp3Decoder::recvPcmFrame(). Truncation. Data buffer space is not enough." << endl;
		realSize = dataSize;
	}
	else
	{
		realSize = mDataSize;
	}

	memcpy(dataBuff, mDataBuf, realSize);

	mDataSize = 0;
	cvProduce.notify_one();

	//cout << "Call Mp3Decoder::recvPcmFrame() end." << endl;
	return realSize;
}

int Mp3Decoder::mp3ToPcm(const char *mp3Path, const char *pcmPath)
{
	// 打开PCM 输出文件
	ofstream ofs(pcmPath, ios::out);
	if(!ofs)
	{
		cerr << "In Mp3Decoder::mp3ToPcm(): Fail to open file " << pcmPath << endl;
		return -1;
	}

	// 打开MP3 输入文件
	AVFormatContext *avFmtCtx = NULL;
	avFmtCtx = avformat_alloc_context();
	
	int ret = 0;
	ret = avformat_open_input(&avFmtCtx, mp3Path, NULL,NULL);
	if(ret < 0)
	{
		cerr << "In Mp3Decoder::mp3ToPcm(): Fail to open file " << mp3Path << endl;
		return -1;
	}

	// 寻找音视频流
	ret = avformat_find_stream_info(avFmtCtx, NULL);
	if(ret < 0)
	{
		cerr << "In Mp3Decoder::mp3ToPcm(): Cannot find any stream in file." << endl;
		return -2;
	}

	// dump 流信息。
	av_dump_format(avFmtCtx, 0, mp3Path, 0);

	// 寻找音频流索引值
	int i = 0;
	int audioStreamIndex = -1;
	for(i = 0; i < avFmtCtx->nb_streams; ++i)
	{
		if(AVMEDIA_TYPE_AUDIO == avFmtCtx->streams[i]->codecpar->codec_type)
		{
			audioStreamIndex = i;
			break;
		}
	}
	
	if(-1 == audioStreamIndex)
	{
		cerr << "In  Mp3Decoder::mp3ToPcm(): Cannot find audio stream.\n" << endl;
		return -2;
	}

	// 根据音频流寻找解码器。
	AVCodec *codec = NULL;
	AVCodecParameters *aCodecPara = NULL;
	
	aCodecPara = avFmtCtx->streams[audioStreamIndex]->codecpar;
	codec = avcodec_find_decoder(aCodecPara->codec_id);
	if(!codec)
	{
		cerr << "In  Mp3Decoder::mp3ToPcm(): Cannot find any codec for audio." << endl;
		return -2;
	}

	// 使用参数初始化解码器上下文。
	AVCodecContext *avCodecCtx = NULL;
	avCodecCtx = avcodec_alloc_context3(codec);
	ret = avcodec_parameters_to_context(avCodecCtx, aCodecPara);
	if(ret < 0)
	{
		cerr << "In  Mp3Decoder::mp3ToPcm(): Cannot alloc codec context." << endl;
		return -2;
	}

	// 打开解码器
	avCodecCtx->pkt_timebase = avFmtCtx->streams[audioStreamIndex]->time_base;
	ret = avcodec_open2(avCodecCtx, codec, NULL);
	if(ret < 0)
	{
		printf("In  Mp3Decoder::mp3ToPcm(): Cannot open audio codec.\n");
		return -2;
	}

	AVPacket *avPacket = NULL;
	avPacket = av_packet_alloc();
	AVFrame *avFrame = NULL;
	avFrame = av_frame_alloc();

	while(av_read_frame(avFmtCtx, avPacket) >= 0)
	{
		if(avPacket->stream_index != audioStreamIndex)
		{
			continue;
		}
		
		ret = avcodec_send_packet(avCodecCtx, avPacket);
		if(ret < 0)
		{
			continue;
		}
		
		while(avcodec_receive_frame(avCodecCtx, avFrame) >= 0)
		{
			/* Planar（平面），其数据格式排列方式为 (特别记住，该处是以点nb_samples采样点来交错，不是以字节交错）:
			LLLLLLRRRRRRLLLLLLRRRRRRLLLLLLRRRRRRL...（每个LLLLLLRRRRRR为一个音频帧）
			而不带P的数据格式（即交错排列）排列方式为：
			LRLRLRLRLRLRLRLRLRLRLRLRLRLRLRLRLRLRL...（每个LR为一个音频样本）*/
			if(av_sample_fmt_is_planar(avCodecCtx->sample_fmt))
			{
				int i = 0;
				int numBytes = 0;
				//pcm播放时是LRLRLR格式，所以要交错保存数据
				for(i = 0; i < avFrame->nb_samples; ++i)
				{
					int ch = 0;
					for(ch = 0; ch < avCodecCtx->channels; ++ch)
					{
						numBytes = av_get_bytes_per_sample(avCodecCtx->sample_fmt);
						ofs.write((char*)avFrame->data[ch] + numBytes * i, numBytes);
						if(!ofs)
						{
							cerr << "Fail to write file." << endl;
							break;
						}
					}
				}
			}
		}

		av_packet_unref(avPacket);
	}

	av_frame_free(&avFrame);
	av_packet_free(&avPacket);
	avcodec_close(avCodecCtx);
	avcodec_free_context(&avCodecCtx);
	avformat_free_context(avFmtCtx);

	return 0;
}

/*
	功能：	获取MP3 音频帧数据标志对应的4个字节内容。
	返回：	成功，返回0；
			-1, 文件无法打开。
	注意：	p4BytesData 必须为4 字节空间。
*/
int Mp3Decoder::getMp3FrameFlag4Bytes(const char *filePath, unsigned char *p4BytesData)
{
	ifstream ifs(filePath, ios::in);
	if(!ifs)
	{
		cerr << "Fail to open file in Mp3Decoder::getMp3FrameFlag4Bytes()." << endl;
		return -1;
	}

	const unsigned int dataSize = 4;
	char readData[dataSize] = {0};
	bool bIsMP3FrameData = false;
	while(!bIsMP3FrameData)
	{
		ifs.read(readData + 0, 1);
		if(!ifs)
		{
			cerr << "Fail to call read() in Mp3Decoder::getMp3FrameFlag4Bytes()." << endl;
			break;
		}
		
		if(0xFF == readData[0])
		{
			ifs.read(readData + 1, 1);
			if(!ifs)
			{
				cerr << "Fail to call read() in Mp3Decoder::getMp3FrameFlag4Bytes()." << endl;
				break;
			}
		}
		
		bIsMP3FrameData = (0xFF == readData[0] && 0xFB == readData[1]);
	}

	if(!bIsMP3FrameData)
	{
		cerr << "In Mp3Decoder::getMp3FrameFlag4Bytes(). Maybe this is not MP3 file." << endl;
		ifs.close();
		return -2;
	}

	ifs.read(readData + 2, 2);
	if(!ifs)
	{
		cerr << "Fail to call read 4 bytes in Mp3Decoder::getMp3FrameFlag4Bytes()." << endl;
		ifs.close();
		return -3;
	}

	ifs.close();
	#if 1	// debug
	cout << "In Mp3Decoder::getMp3FrameFlag4Bytes: get 4 bytes: " << "0x" << hex << setfill('0') << 
		setw(2) << (int)readData[0] << setw(2) << (int)readData[1] << 
		setw(2) << (int)readData[2] << setw(2) << (int)readData[3] << endl;
	#endif

	if(NULL == p4BytesData)
	{
		cerr << "In Mp3Decoder::getMp3FrameFlag4Bytes(). Argument has null value." << endl;
	}
	else
	{
		memcpy(p4BytesData, readData, 4);
	}
	
	return 0;
}

static unsigned int bitRateArray[16][6]
{
	{0, 0, 0, 0, 0, 0,},				// 0000
	{32, 32, 32, 32, 32, 32,},
	{64, 48, 40, 64, 48, 16,},
	{96, 56, 48, 96, 56, 24,},
	{128, 64, 56, 128, 64, 32,},		// 0100
	{160, 80, 64, 160, 80, 64,},
	{192, 96, 80, 192, 96, 80,},
	{224, 112, 96, 224, 112, 96,},
	{256, 128, 116, 256, 128, 64,},		// 1000
	{288, 160, 128, 288, 160, 128,},
	{320, 192, 160, 320, 192, 160,},
	{352, 224, 192, 352, 224, 112,},
	{384, 256, 224, 384, 256, 128,},	// 1100
	{416, 320, 256, 416, 320, 256,},
	{448, 384, 320, 448, 384, 320,},
	{0, 0, 0, 0, 0, 0,},				// 1111
};

static float sampleRateArray[4][4]
{
	{11.025, 12.0, 8.0, 0.0},
	{0.0, 0.0, 0.0, 0.0},
	{22.05, 24.0, 16.0, 0.0},
	{44.1, 48.0, 32.0, 0.0},
};

/*
	功能：	解析MP3 音频帧。
	返回：	成功，返回0；
	注意：	
*/
int Mp3Decoder::analyzeMp3Frame(const char *filePath, long long int *pSampleRate, long long int *pChLayout, AVSampleFormat *pAvSampleFmt)
{
	int ret = 0;
	const unsigned int dataSize = 4;
	unsigned char p4BytesData[dataSize] = {0};

	ret = getMp3FrameFlag4Bytes(filePath, p4BytesData);
	if(0 != ret)
	{
		cerr << "Fail to call getMp3FrameFlag4Bytes() in Mp3Decoder::getMp3FrameFlag4Bytes()." << endl;
		return -1;
	}
	#if 1	// debug
	cout << "In Mp3Decoder::analyzeMp3Frame: get 4 bytes: " << "0x" << hex << setfill('0') << 
		setw(2) << (int)p4BytesData[0] << setw(2) << (int)p4BytesData[1] << 
		setw(2) << (int)p4BytesData[2] << setw(2) << (int)p4BytesData[3] << endl;
	#endif

	unsigned int version = 0;			// 版本
	unsigned int layer = 0;				// 声道数
	bool bCRC  = false;					// CRC校验
	unsigned int bitRate = 0;			// 比特率
	int bitRateIndex = 0;
	float sampleRate = 0.0;				// 采样率
	unsigned int sampleRateIndex = 0;
	bool bFrameLenAdj = 0;				// 帧长调整
	unsigned int soundChMode = 0;		// 声道模式

	version = ((p4BytesData[1] & 0x18) >> 3);
	layer = ((p4BytesData[1] & 0x06) >> 1);
	bCRC = ((p4BytesData[1] & 0x01) >> 0);
	bitRateIndex = ((p4BytesData[2] & 0xF0) >> 4);
	sampleRateIndex = ((p4BytesData[2] & 0x0C) >> 2);
	bFrameLenAdj = ((p4BytesData[2] & 0x02) >> 1);
	soundChMode = ((p4BytesData[3] & 0x0C) >> 6);

	switch(version)
	{
		case 0:
		{
			cout << "version: MPEG 2.5" << endl;
			break;
		}
		case 1:
		{
			cout << "version: undefined" << endl;
			break;
		}
		case 2:
		{
			cout << "version: MPEG 2" << endl;
			break;
		}
		case 3:
		{
			cout << "version: MPEG 1" << endl;
			break;
		}
	}

	switch(layer)
	{
		case 0:
		{
			cout << "layer: undefined" << endl;
			break;
		}
		case 1:
		{
			cout << "layer: 3" << endl;
			break;
		}
		case 2:
		{
			cout << "layer: 2" << endl;
			break;
		}
		case 3:
		{
			cout << "layer: 1" << endl;
			break;
		}
	}

	if(bCRC)
	{
		cout << "CRC: true" << endl;
	}
	else
	{
		cout << "CRC: false" << endl;
	}

	int index = 0;
	if(3 == version && 3 == layer)
	{
		index = 0;
	}
	else if(3 == version && 2 == layer)
	{
		index = 1;
	}
	else if(3 == version && 1 == layer)
	{
		index = 2;
	}
	else if(2 == version && 3 == layer)
	{
		index = 3;
	}
	else if(2 == version && 2 == layer)
	{
		index = 4;
	}
	else if(2 == version && 1 == layer)
	{
		index = 5;
	}
	else
	{
		index = -1;
	}

	if(-1 != index)
	{
		bitRate = bitRateArray[bitRateIndex][index];
	}
	cout << "_index: " << dec << index << endl;
	cout << "bitRateIndex: " << (unsigned int)bitRateIndex << endl;
	cout << "bitRate: " << bitRate << "kbps" << endl;

	sampleRate = sampleRateArray[version][sampleRateIndex];
	cout << "sampleRate : " << sampleRate << "KHz" << endl;
	if(NULL != pSampleRate)
	{
		*pSampleRate = sampleRate * 1000;
	}

	if(bFrameLenAdj)
	{
		cout << "bFrameLenAdj: " << true << endl;
	}
	else
	{
		cout << "bFrameLenAdj: " << false << endl;
	}

	switch(soundChMode)
	{
		case 0:
		{
			cout << "soundChMode: stereo." << endl;
			break;
		}
		case 1:
		{
			cout << "soundChMode: joint stereo." << endl;
			break;
		}
		case 2:
		{
			cout << "soundChMode: dual track." << endl;
			break;
		}
		case 3:
		{
			cout << "soundChMode: mono." << endl;
			break;
		}
	}

	if(NULL != pChLayout)
	{
		if(3 == soundChMode)
		{
			*pChLayout = 1;
		}
		else
		{
			*pChLayout = 2;
		}
	}

	if(NULL != pAvSampleFmt)
	{
		*pAvSampleFmt = AV_SAMPLE_FMT_FLT;
	}
	
	return 0;
}

/*
	功能：	PCM 文件重采样。调整声道数、采样率、采样格式。
	返回：	成功，返回0；
			-1, 参数错误；
			-2, 重采样上下文分配失败或初始化失败；
	注意：	
*/
int Mp3Decoder::pcmFileResample(const char *dstPcmPath, long long int dstSampleRate, long long int dstChLayout, AVSampleFormat dstAvSampleFmt, 
		const char *srcPcmPath, long long int srcSampleRate, long long int srcChLayout, AVSampleFormat srcAvSampleFmt, int srcNbSamples)
{
	if(NULL == srcPcmPath || NULL == dstPcmPath)
	{
		cerr << "Fail to call Mp3Decoder::pcmFileResample(). File path has null value." << endl;
		return -1;
	}

	ifstream ifs(srcPcmPath, ios::in);
	if(!ifs)
	{
		cerr << "Fail to open file " << srcPcmPath << endl;
		return -1;
	}
	
	ofstream ofs(dstPcmPath, ios::out);
	if(!ofs)
	{
		cerr << "Fail to open file " << dstPcmPath << endl;
		return -1;
	}

	// 1、创建上下文SwrContext
	SwrContext *swrCtx = NULL;
	swrCtx = swr_alloc();
	if(NULL == swrCtx)
	{
		cerr << "In Mp3Decoder::pcmFileResample(): swr_allock() fail" << endl;
		return -2;
	}

	// 2、设置重采样的相关参数 这些函数位于头文件 <libavutil/opt.h>
	av_opt_set_int(swrCtx, "in_channel_layout", srcChLayout, 0);
	av_opt_set_int(swrCtx, "in_sample_rate", srcSampleRate, 0);
	av_opt_set_sample_fmt(swrCtx, "in_sample_fmt", srcAvSampleFmt, 0);

	av_opt_set_int(swrCtx, "out_channel_layout", dstChLayout, 0);
	av_opt_set_int(swrCtx, "out_sample_rate", dstSampleRate, 0);
	av_opt_set_sample_fmt(swrCtx, "out_sample_fmt", dstAvSampleFmt, 0);

	// 3、初始化上下文
	int ret = 0;
	ret = swr_init(swrCtx);
	if(ret < 0) 
	{
		cerr << "In Mp3Decoder::pcmFileResample(): swr_init() fail" << endl;
		return -2;
	}

	int inNbChs = 0;		// 声道数
	int outNbChs = 0;
	inNbChs = av_get_channel_layout_nb_channels(srcChLayout);
	outNbChs = av_get_channel_layout_nb_channels(dstChLayout);
	
	// 根据srcAvSampleFmt、srcNbSamples、inNbChs 为srcData 分配内存空间，和设置对应的的linesize 的值；返回分配的总内存的大小
	int srcBufSize = 0;
	int dstBufSize = 0;
	int srcLineSize = 0;
	int dstLineSize = 0;	
	long long int dstNbSamples = 0;
	long long int dstNbSamplesMax = 0;
	//const int srcNbSamples = 1024;
	unsigned char **srcData = NULL;
	unsigned char **dstData = NULL;

	srcBufSize = av_samples_alloc_array_and_samples(&srcData, &srcLineSize, inNbChs, srcNbSamples, srcAvSampleFmt, 0);
	// 根据srcNbSamples*dstSampleRate/srcSampleRate公式初步估算重采样后音频的nb_samples大小
	dstNbSamples = av_rescale_rnd(srcNbSamples, dstSampleRate, srcSampleRate, AV_ROUND_UP);
	dstNbSamplesMax = dstNbSamples;
	dstBufSize = av_samples_alloc_array_and_samples(&dstData, &dstLineSize, outNbChs, dstNbSamples, dstAvSampleFmt, 0);

	while(ifs)
	{
		size_t readBytes = 0;
		ifs.read((char *)srcData[0], srcBufSize);
		if(!ifs)
		{
			readBytes = ifs.gcount();
			if(0 == readBytes)
			{
				cout << "Read over in Mp3Decoder::pcmFileResample()." << endl;
			}
			else if(readBytes < srcBufSize)
			{
				cerr << "Bytes actually read are " << readBytes << endl;
			}
			else
			{
				cerr << "Fail to read file " << srcPcmPath << endl;
			}
			
			break;
		}

		/*	因为转换需要缓存，所以要不停的调整转换后的内存的大小，估算重采样后的nb_samples 的大小。
			这里swr_get_delay() 用于获取重采样的缓冲延迟。
			dstNbSamples的值会经过多次调整后区域稳定 */
		dstNbSamples = av_rescale_rnd(swr_get_delay(swrCtx, srcSampleRate) + srcNbSamples, dstSampleRate, srcSampleRate, AV_ROUND_UP);
		if(dstNbSamples > dstNbSamplesMax)
		{
			cerr << "In Mp3Decoder::pcmFileResample(): realloc memory." << endl;
			// 先释放以前的内存，不管sample_fmt 是planner 还是packet 方式，av_samples_alloc_array_and_samples()函数都是分配的一整块连续的内存
			av_freep(&dstData[0]);
			dstBufSize = av_samples_alloc_array_and_samples(&dstData, &dstLineSize, outNbChs, dstNbSamples, dstAvSampleFmt, 0);
			dstNbSamplesMax = dstNbSamples;
		}
 
		// 开始重采样，重采样后的数据将根据前面指定的存储方式写入ds_data 内存块中，返回每个声道实际的采样数。
		/*	swr_convert() 返回的outNbSmples 是实际转换的采样个数，该值小于或等于预计采样数dstNbSamples, 
			所以写入文件的时候不能用dstNbSamples的  值，而应该用outNbSmples值。*/
		int outNbSmples = swr_convert(swrCtx, dstData, dstNbSamples, (const unsigned char **)srcData, srcNbSamples);
		if(outNbSmples < 0)
		{
			cerr << "In Mp3Decoder::pcmFileResample() swr_convert() fail. Result = " << outNbSmples << endl;
			break;
		}

		// 将音频数据写入pcm文件
		if(av_sample_fmt_is_planar(dstAvSampleFmt))
		{
			// planner方式。pcm文件写入时一般都是packet方式，所以这里要注意转换一下。
			int perSampleBytes = 0;
			perSampleBytes = av_get_bytes_per_sample(dstAvSampleFmt);
			// 这里必须是outNbSmples，而不能用dstNbSamples,因为outNbSmples 才代表此次实际转换的采样数，它肯定小于或等于dstNbSamples.
			int i = 0;
			for(i = 0; i < outNbSmples; ++i)
			{
				int ch = 0;
				for(ch = 0; ch < outNbChs; ++ch)
				{
					ofs.write((char *)dstData[ch] + i * perSampleBytes, perSampleBytes);
				}
			}
		}
		else
		{
			// 最后一个参数必须为1, 否则会因为cpu 对齐算出来的大小大于实际的数据大小，导致多写入数据 造成错误。
			dstBufSize = av_samples_get_buffer_size(&dstLineSize, outNbChs, outNbSmples, dstAvSampleFmt, 1);
			ofs.write((char *)dstData[0], dstBufSize);
		}
	}

	// 还有剩余的缓存数据没有转换，第三个传递NULL, 第四个传递0, 即可将缓存中的全部取出
	do{
		int realNbSamples = 0;
		realNbSamples = swr_convert(swrCtx, dstData, dstNbSamples, NULL, 0);
		if(realNbSamples <= 0)
		{
			break;
		}
		
		cout << "In Mp3Decoder::pcmFileResample(), left " << realNbSamples << endl;
		if(av_sample_fmt_is_planar(dstAvSampleFmt))
		{
			int perSampleBytes = 0;
			perSampleBytes = av_get_bytes_per_sample(dstAvSampleFmt);
			int i = 0;
			for(i = 0; i < realNbSamples; ++i)
			{
				int j = 0;
				for(j = 0; j < outNbChs; ++j)
				{
					ofs.write((char *)dstData[j] + i * perSampleBytes, perSampleBytes);
				}
			}
		}
		else
		{
			int size = 0;
			size = av_samples_get_buffer_size(NULL, outNbChs, realNbSamples, dstAvSampleFmt, 1);
			ofs.write((char *)dstData[0], size);
		}
	}while(true);

	// 释放资源
	int ch = 0;
	for(ch = 0; ch < outNbChs; ++ch)
	{
		av_freep(&dstData[0]);
	}
	av_freep(&srcData[0]);
	swr_free(&swrCtx);
	
	ifs.close();
	ofs.close();

	return 0;
}

/*
	功能：	PCM 数据重采样。调整声道数、采样率、采样格式。
	返回：	成功，返回重采样后的数据长度；
			-1, 参数错误；
			-2, 重采样上下文分配失败或初始化失败；
			-3, PCM 目标数据空间不足或源数据长度不足。
	注意：	
*/
int Mp3Decoder::pcmDataResample(void *dstPcmData, unsigned int dstPcmLen, 
			long long int dstSampleRate, long long int dstChLayout, AVSampleFormat dstAvSampleFmt, 
			const void *srcPcmData, unsigned int srcPcmLen, 
			long long int srcSampleRate, long long int srcChLayout, AVSampleFormat srcAvSampleFmt, int srcNbSamples)
{
	if(NULL == srcPcmData || NULL == dstPcmData)
	{
		cerr << "Fail to call Mp3Decoder::pcmFileResample(). Argument has null value." << endl;
		return -1;
	}

	// 1、创建上下文SwrContext
	SwrContext *swrCtx = NULL;
	swrCtx = swr_alloc();
	if(NULL == swrCtx)
	{
		cerr << "In Mp3Decoder::pcmDataResample(): swr_allock() fail" << endl;
		return -2;
	}

	// 2、设置重采样的相关参数 这些函数位于头文件 <libavutil/opt.h>
	av_opt_set_int(swrCtx, "in_channel_layout", srcChLayout, 0);
	av_opt_set_int(swrCtx, "in_sample_rate", srcSampleRate, 0);
	av_opt_set_sample_fmt(swrCtx, "in_sample_fmt", srcAvSampleFmt, 0);

	av_opt_set_int(swrCtx, "out_channel_layout", dstChLayout, 0);
	av_opt_set_int(swrCtx, "out_sample_rate", dstSampleRate, 0);
	av_opt_set_sample_fmt(swrCtx, "out_sample_fmt", dstAvSampleFmt, 0);

	// 3、初始化上下文
	int ret = 0;
	ret = swr_init(swrCtx);
	if(ret < 0) 
	{
		cerr << "In Mp3Decoder::pcmDataResample(): swr_init() fail" << endl;
		swr_free(&swrCtx);
		return -2;
	}

	// 根据声道布局计算声道数。
	int inNbChs = 0;		// 声道数
	int outNbChs = 0;
	inNbChs = av_get_channel_layout_nb_channels(srcChLayout);
	outNbChs = av_get_channel_layout_nb_channels(dstChLayout);

	int srcBufSize = 0;
	int dstBufSize = 0;
	int srcLineSize = 0;
	int dstLineSize = 0;	
	unsigned char **srcData = NULL;
	unsigned char **dstData = NULL;
	long long int dstNbSamples = 0;
	
	// 根据srcAvSampleFmt、srcNbSamples、inNbChs 为srcData 分配内存空间，和设置对应的的linesize 的值；返回分配的总内存的大小
	srcBufSize = av_samples_alloc_array_and_samples(&srcData, &srcLineSize, inNbChs, srcNbSamples, srcAvSampleFmt, 0);
	// 根据srcNbSamples * dstSampleRate / srcSampleRate公式初步估算重采样后音频的nb_samples 大小。
	// eg: 1152 * 16000 / 44100 = 417.96
	dstNbSamples = av_rescale_rnd(srcNbSamples, dstSampleRate, srcSampleRate, AV_ROUND_UP);
	dstBufSize = av_samples_alloc_array_and_samples(&dstData, &dstLineSize, outNbChs, dstNbSamples, dstAvSampleFmt, 0);
	
	long long int dstNbSamplesMax = 0;
	dstNbSamplesMax = dstNbSamples;

	bool bOOM = false;
	if(srcBufSize > srcPcmLen)
	{
		cerr << "In Mp3Decoder::pcmDataResample(), Truncation: srcBufSize > srcPcmLen." << endl;
		srcBufSize = srcPcmLen;
		bOOM = true;
	}
	memcpy(srcData[0], srcPcmData, srcBufSize);

	/*	因为转换需要缓存，所以要不停的调整转换后的内存的大小，估算重采样后的nb_samples 的大小。
		这里swr_get_delay() 用于获取重采样的缓冲延迟。
		dstNbSamples的值会经过多次调整后区域稳定 */
	dstNbSamples = av_rescale_rnd(swr_get_delay(swrCtx, srcSampleRate) + srcNbSamples, dstSampleRate, srcSampleRate, AV_ROUND_UP);
	if(dstNbSamples > dstNbSamplesMax)
	{
		cerr << "In Mp3Decoder::pcmDataResample(): realloc memory." << endl;
		// 先释放以前的内存，不管sample_fmt 是planner 还是packet 方式，av_samples_alloc_array_and_samples()函数都是分配的一整块连续的内存
		av_freep(&dstData[0]);
		dstBufSize = av_samples_alloc_array_and_samples(&dstData, &dstLineSize, outNbChs, dstNbSamples, dstAvSampleFmt, 0);
		dstNbSamplesMax = dstNbSamples;
	}

	// 开始重采样，重采样后的数据将根据前面指定的存储方式写入ds_data 内存块中，返回每个声道实际的采样数。
	/*	swr_convert() 返回的outNbSmples 是实际转换的采样个数，该值小于或等于预计采样数dstNbSamples, 
		所以写入文件的时候不能用dstNbSamples的  值，而应该用outNbSmples值。*/
	int outNbSmples = 0;
	outNbSmples = swr_convert(swrCtx, dstData, dstNbSamples, (const unsigned char **)srcData, srcNbSamples);
	if(outNbSmples < 0)
	{
		cerr << "In Mp3Decoder::pcmDataResample() swr_convert() fail. outNbSmples = " << outNbSmples << endl;
	}

	// 将音频数据写入pcm文件
	// planner方式。pcm文件写入时一般都是packet方式，所以这里要注意转换一下。
	if(av_sample_fmt_is_planar(dstAvSampleFmt))
	{
		int i = 0;
		int perSampleBytes = 0;
		
		perSampleBytes = av_get_bytes_per_sample(dstAvSampleFmt);
		// 这里必须是outNbSmples，而不能用dstNbSamples,因为outNbSmples 才代表此次实际转换的采样数，它肯定小于或等于dstNbSamples.
		for(i = 0; i < outNbSmples; ++i)
		{
			int ch = 0;
			for(ch = 0; ch < outNbChs; ++ch)
			{
				if(perSampleBytes * (i + ch) < dstPcmLen)
				{
					memcpy(dstPcmData + perSampleBytes * (i + ch), dstData[ch] + i * perSampleBytes, perSampleBytes);
				}
				else
				{
					bOOM = true;
					break;
				}
			}
		}
	}
	else
	{
		// 最后一个参数必须为1, 否则会因为cpu 对齐算出来的大小大于实际的数据大小，导致多写入数据 造成错误。
		dstBufSize = av_samples_get_buffer_size(&dstLineSize, outNbChs, outNbSmples, dstAvSampleFmt, 1);
		if(dstPcmLen < dstBufSize)
		{
			cerr << "In Mp3Decoder::pcmDataResample(): space is not enough. dstPcmData is less than need." << endl;
			dstBufSize = dstPcmLen;
			bOOM = true;
		}
		memcpy(dstPcmData, dstData[0], dstBufSize);
	}

	// 释放资源
	int ch = 0;
	for(ch = 0; ch < outNbChs; ++ch)
	{
		av_freep(&dstData[ch]);
	}
	av_freep(&srcData[0]);
	swr_free(&swrCtx);

	if(bOOM)
	{
		return -3;
	}
	
	return dstBufSize;
}


