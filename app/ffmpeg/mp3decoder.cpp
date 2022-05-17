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

using namespace std;

Mp3Decoder* Mp3Decoder::getInstance()
{
	static Mp3Decoder ffmpeg;
	return &ffmpeg;
}

Mp3Decoder::Mp3Decoder()
{
	cout << "Call Mp3Decoder::Mp3Decoder()." << endl;
	enable();
	cout << "Call Mp3Decoder::Mp3Decoder() end." << endl;
}

Mp3Decoder::~Mp3Decoder()
{
	cout << "Call Mp3Decoder::~Mp3Decoder()." << endl;
	disable();
	cout << "Call Mp3Decoder::~Mp3Decoder() end." << endl;
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
	bRunning = true;

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
	bRunning = false;
	cvProduce.notify_all();
	cvConsume.notify_all();

	#if 0		// free后导致recv() 中的memcpy 执行错误。
	if(NULL != mDataBuf)
	{
		free(mDataBuf);
		mDataBuf = NULL;
	}
	#endif

	if(NULL != pTh)
	{
		pTh->join();
		pTh = NULL;
	}
	
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
	cout << "Call Mp3Decoder::mp3Decoding()." << endl;
	bIsDecoding = true;
	pTh = make_shared<thread>(thRouteMp3Decoding, this, filePath);
	cout << "Call Mp3Decoder::mp3Decoding() end." << endl;
	return 0;
}

int Mp3Decoder::thRouteMp3Decoding(void *arg, const char *filePath)
{
	cout << "Call Mp3Decoder::thRouteMp3Decoding()." << endl;
	Mp3Decoder *pThis = (Mp3Decoder *)arg;
	return pThis->routeMp3Decoding(filePath);
}

/*
	功能：	解码MP3 文件。线程函数，内部实现。
	返回：	成功，返回0;
			-1, 文件无法打开。
			-2, 音频解码器初始化失败。
	注意：	与recvPcmFrame 配对使用。
*/
int Mp3Decoder::routeMp3Decoding(const char *filePath)
{
	cout << "Call Mp3Decoder::routeMp3Decoding()." << endl;
	
	// 打开MP3 输入文件
	AVFormatContext *avFmtCtx = NULL;
	avFmtCtx = avformat_alloc_context();
	
	int ret = 0;
	ret = avformat_open_input(&avFmtCtx, filePath, NULL, NULL);
	if(ret < 0)
	{
		cerr << "In Mp3Decoder::routeMp3Decoding(): Fail to open file " << filePath << endl;
		bIsDecoding = false;
		return -1;
	}

	// 寻找音视频流
	ret = avformat_find_stream_info(avFmtCtx, NULL);
	if(ret < 0)
	{
		cerr << "In Mp3Decoder::routeMp3Decoding(): Cannot find any stream in file." << endl;
		avformat_close_input(&avFmtCtx);
		bIsDecoding = false;
		return -2;
	}

	// dump 流信息。
	//av_dump_format(avFmtCtx, 0, filePath, 0);

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
		cerr << "In Mp3Decoder::routeMp3Decoding(): Cannot find audio stream.\n" << endl;
		avformat_close_input(&avFmtCtx);
		bIsDecoding = false;
		return -2;
	}

	// 根据音频流寻找解码器。
	AVCodec *avCodec = NULL;
	AVCodecParameters *avCodecPar = NULL;

	avCodecPar = avFmtCtx->streams[ASIndex]->codecpar;
	avCodec = avcodec_find_decoder(avCodecPar->codec_id);
	if(!avCodec)
	{
		cerr << "In Mp3Decoder::routeMp3Decoding(): Cannot find any avCodec for audio." << endl;
		avformat_close_input(&avFmtCtx);
		bIsDecoding = false;
		return -2;
	}

	// 使用参数初始化解码器上下文。
	AVCodecContext *avCodecCtx = NULL;
	avCodecCtx = avcodec_alloc_context3(avCodec);
	ret = avcodec_parameters_to_context(avCodecCtx, avCodecPar);
	if(ret < 0)
	{
		cerr << "In Mp3Decoder::routeMp3Decoding(): Cannot alloc avCodec context." << endl;
		avformat_close_input(&avFmtCtx);
		bIsDecoding = false;
		return -2;
	}

	// 打开解码器
	avCodecCtx->pkt_timebase = avFmtCtx->streams[ASIndex]->time_base;
	ret = avcodec_open2(avCodecCtx, avCodec, NULL);
	if(ret < 0)
	{
		cerr << "In Mp3Decoder::routeMp3Decoding(): Cannot open audio avCodec." << endl;
		avformat_close_input(&avFmtCtx);
		bIsDecoding = false;
		return -2;
	}

	AVPacket *avPacket = NULL;
	avPacket = av_packet_alloc();
	AVFrame *avFrame = NULL;
	avFrame = av_frame_alloc();

	while(av_read_frame(avFmtCtx, avPacket) >= 0 && bRunning)
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

		while(0 == (ret = avcodec_receive_frame(avCodecCtx, avFrame)))
		{
			const unsigned int bytesPerSample = av_get_bytes_per_sample(avCodecCtx->sample_fmt);
			const unsigned int pcmSize = bytesPerSample * avFrame->nb_samples * avCodecCtx->channels;
			const unsigned int channels = avCodecCtx->channels;
			#if 0
			cout << "bytesPerSample = " << bytesPerSample << ", pcmSize = " << pcmSize 
				<< ", avFrame->nb_samples = " << avFrame->nb_samples << ", channels = " << channels << endl;
			#endif

			unique_lock<std::mutex> lock(mutex);
			while(0 != mDataSize && bRunning)			// mData 不为0, 表示ffmpeg 依然有未处理完的数据。
			{
				cvProduce.wait(lock);
			}
			
			if(NULL == mDataBuf)
			{
				mDataBuf = (unsigned char *)malloc(pcmSize);
			}
			memset(mDataBuf, 0, pcmSize);
			
			#if 0	// packet
			int i = 0;
			for(i = 0; i < avFrame->nb_samples; ++i)
			{
				//cout << "i = " << i << endl;
				int ch = 0;
				for(ch = 0; ch < channels; ++ch)
				{
					//cout << "ch = " << ch << endl;
					//ofs.write((char *)avFrame->data[ch] + bytesPerSample*i, bytesPerSample);
					memcpy(mDataBuf + (i * channels + ch) * bytesPerSample, avFrame->data[ch] + bytesPerSample * i, bytesPerSample);
					//memcpy(mDataBuf + channels * ch, avFrame->data[ch], pcmSize / channels);
				}
			}
			#else	// planer
			int ch = 0;
			for(ch = 0; ch < channels; ++ch)
			{
				memcpy(mDataBuf + pcmSize / channels * ch, avFrame->data[ch], pcmSize / channels);
			}
			#endif

			mDataSize = pcmSize;
			cvConsume.notify_one();
		}

		av_packet_unref(avPacket);
	}

	bIsDecoding = false;

	av_frame_free(&avFrame);
	av_packet_free(&avPacket);
	avcodec_close(avCodecCtx);
	avcodec_free_context(&avCodecCtx);
	//avformat_free_context(avFmtCtx);
	avformat_close_input(&avFmtCtx);

	cout << "Call Mp3Decoder::routeMp3Decoding() end." << endl;
	return 0;
}

/*
	功能：	接收PCM 数据。
	返回：	返回PCM 数据的长度。
	注意：	前序调用为mp3Decoding().
*/
int Mp3Decoder::recvPcmFrame(unsigned char *const dataBuff, const unsigned int dataSize)
{
	//cout << "Call Mp3Decoder::recvPcmFrame()." << endl;

	unique_lock<std::mutex> lock(mutex);
	while(0 == mDataSize && bIsDecoding)		// mDataSize 为0 表示已经没有要处理的数据，通知生产者传递数据。
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
	free(mDataBuf);
	mDataBuf = NULL;
	mDataSize = 0;
	cvProduce.notify_one();
	
	if(!bIsDecoding && NULL != mDataBuf)
	{
		//cout << "free " << endl;
		free(mDataBuf);
		mDataBuf = NULL;
	}

	//cout << "Call Mp3Decoder::recvPcmFrame() end." << endl;
	return realSize;
}

/*
	功能：	将MP3 转为PCM 格式。
	返回：	
	注意：	
*/
int Mp3Decoder::mp3ToPcm(const char *mp3Path, const char *pcmPath)
{
	cout << "Call Mp3Decoder::mp3ToPcm()." << endl;
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
		cerr << "In Mp3Decoder::mp3ToPcm(): Cannot find audio stream.\n" << endl;
		return -2;
	}

	// 根据音频流寻找解码器。
	AVCodec *avCodec = NULL;
	AVCodecParameters *avCodecPar = NULL;
	
	avCodecPar = avFmtCtx->streams[audioStreamIndex]->codecpar;
	avCodec = avcodec_find_decoder(avCodecPar->codec_id);
	if(!avCodec)
	{
		cerr << "In Mp3Decoder::mp3ToPcm(): Cannot find any avCodec for audio." << endl;
		return -2;
	}

	// 使用参数初始化解码器上下文。
	AVCodecContext *avCodecCtx = NULL;
	avCodecCtx = avcodec_alloc_context3(avCodec);
	ret = avcodec_parameters_to_context(avCodecCtx, avCodecPar);
	if(ret < 0)
	{
		cerr << "In Mp3Decoder::mp3ToPcm(): Cannot alloc avCodec context." << endl;
		return -2;
	}

	// 打开解码器
	avCodecCtx->pkt_timebase = avFmtCtx->streams[audioStreamIndex]->time_base;
	ret = avcodec_open2(avCodecCtx, avCodec, NULL);
	if(ret < 0)
	{
		cerr << "In Mp3Decoder::mp3ToPcm(): Cannot open audio avCodec." << endl;
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

	cout << "Call Mp3Decoder::mp3ToPcm() end." << endl;
	return 0;
}

/*
	功能：	PCM 文件重采样。调整声道数、采样率、采样格式。
	返回：	成功，返回0;
			-1, 参数错误；
			-2, 重采样上下文分配失败或初始化失败；
	注意：	
*/
int Mp3Decoder::pcmFileResample(const char *dstPcmPath, long long int dstSampleRate, long long int dstChLayout, AVSampleFormat dstAvSampleFmt, 
		const char *srcPcmPath, long long int srcSampleRate, long long int srcChLayout, AVSampleFormat srcAvSampleFmt, int srcNbSamples)
{
	//cout << "Call Mp3Decoder::pcmFileResample()." << endl;

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

		// 最后一个参数必须为1, 否则会因为cpu 对齐算出来的大小大于实际的数据大小，导致多写入数据造成错误。
		dstBufSize = av_samples_get_buffer_size(&dstLineSize, outNbChs, outNbSmples, dstAvSampleFmt, 1);
		ofs.write((char *)dstData[0], dstBufSize);
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

	//cout << "Call Mp3Decoder::pcmFileResample() end." << endl;
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
	//cout << "Call Mp3Decoder::pcmDataResample()." << endl;

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

	// 4、根据声道布局计算声道数。
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

	#if 0	// debug
	cout << "inNbChs = " << inNbChs << ", srcNbSamples = " << srcNbSamples << ", srcAvSampleFmt = " << srcAvSampleFmt << endl;
	cout << "srcNbSamples = " << srcNbSamples << ", dstSampleRate = " << dstSampleRate << ", srcSampleRate = " << srcSampleRate << endl;
	#endif
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

	// 最后一个参数必须为1, 否则会因为cpu 对齐算出来的大小大于实际的数据大小，导致多写入数据造成错误。
	dstBufSize = av_samples_get_buffer_size(&dstLineSize, outNbChs, outNbSmples, dstAvSampleFmt, 1);
	if(dstPcmLen < dstBufSize)
	{
		cerr << "In Mp3Decoder::pcmDataResample(): space is not enough. dstPcmData is less than need." << endl;
		dstBufSize = dstPcmLen;
		bOOM = true;
	}
	memcpy(dstPcmData, dstData[0], dstBufSize);

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

	//cout << "Call Mp3Decoder::pcmDataResample() end." << endl;
	return dstBufSize;
}

/*
	功能：	获取MP3 属性，声道数、采样率、采样格式。
	返回：	成功，返回0;
			-1, 文件无法打开。
			-2, 无法找到音频信息或音频解码器初始化失败。
	注意：	
*/
int Mp3Decoder::getMp3Attr(const char *filePath, long long int *pSampleRate, long long int *pChLayout, AVSampleFormat *pAvSampleFmt, int *pNbSamples)
{
	cout << "Call Mp3Decoder::getMp3Attr()." << endl;
	// 打开MP3 输入文件
	AVFormatContext *avFmtCtx = NULL;
	avFmtCtx = avformat_alloc_context();
	
	int ret = 0;
	ret = avformat_open_input(&avFmtCtx, filePath, NULL,NULL);
	if(ret < 0)
	{
		cerr << "In Mp3Decoder::getMp3Attr(): Fail to open file " << filePath << endl;
		return -1;
	}

	// 寻找音视频流
	ret = avformat_find_stream_info(avFmtCtx, NULL);
	if(ret < 0)
	{
		cerr << "In Mp3Decoder::getMp3Attr(): Cannot find any stream in file." << endl;
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
		cerr << "In Mp3Decoder::getMp3Attr(): Cannot find audio stream.\n" << endl;
		return -2;
	}

	// 根据音频流寻找解码器
	AVCodec *avCodec = NULL;
	AVCodecParameters *avCodecPar = NULL;

	avCodecPar = avFmtCtx->streams[ASIndex]->codecpar;
	avCodec = avcodec_find_decoder(avCodecPar->codec_id);
	if(!avCodec)
	{
		cerr << "In Mp3Decoder::getMp3Attr(): Cannot find any avCodec for audio." << endl;
		return -2;
	}

	if(NULL != pSampleRate)
	{
		*pSampleRate = avCodecPar->sample_rate;
		cout << "In Mp3Decoder::mp3Decoding(). *pSampleRate = " << *pSampleRate << endl;
	}

	if(NULL != pAvSampleFmt)
	{
		*pAvSampleFmt = (AVSampleFormat)(avCodecPar->format);
		cout << "In Mp3Decoder::mp3Decoding(). *pAvSampleFmt = " << *pAvSampleFmt << endl;
	}

	if(NULL != pChLayout)
	{
		*pChLayout = avCodecPar->channel_layout;
		cout << "In Mp3Decoder::mp3Decoding(). *pChLayout = " << *pChLayout << endl;
	}

	// 使用参数初始化解码器上下文。
	AVCodecContext *avCodecCtx = NULL;
	avCodecCtx = avcodec_alloc_context3(avCodec);
	ret = avcodec_parameters_to_context(avCodecCtx, avCodecPar);
	if(ret < 0)
	{
		cerr << "In Mp3Decoder::mp3Decoding(): Cannot alloc avCodec context." << endl;
		return -2;
	}

	// 打开解码器
	avCodecCtx->pkt_timebase = avFmtCtx->streams[ASIndex]->time_base;
	ret = avcodec_open2(avCodecCtx, avCodec, NULL);
	if(ret < 0)
	{
		cerr << "In Mp3Decoder::mp3Decoding(): Cannot open audio avCodec." << endl;
		return -2;
	}

	AVPacket *avPacket = NULL;
	avPacket = av_packet_alloc();
	AVFrame *avFrame = NULL;
	avFrame = av_frame_alloc();

	unsigned int cnt = 0;
	while(av_read_frame(avFmtCtx, avPacket) >= 0 && 3 != cnt)
	{
		if(avPacket->stream_index != ASIndex)
		{
			continue;
		}
		++cnt;
		
		ret = avcodec_send_packet(avCodecCtx, avPacket);
		if(ret < 0)
		{
			continue;
		}

		while(avcodec_receive_frame(avCodecCtx, avFrame) >= 0)
		{
			if(NULL != pNbSamples)
			{
				*pNbSamples = avFrame->nb_samples;
				//cout << "In Mp3Decoder::mp3Decoding(). *pNbSamples = " << *pNbSamples << endl;
			}
		}

		av_packet_unref(avPacket);
	}
	cout << "In Mp3Decoder::mp3Decoding(). *pNbSamples = " << *pNbSamples << endl;

	//av_packet_unref(avPacket);
	av_frame_free(&avFrame);
	av_packet_free(&avPacket);
	avcodec_close(avCodecCtx);
	avcodec_free_context(&avCodecCtx);
	//avformat_free_context(avFmtCtx);	// 不得与avformat_close_input() 同时调用，避免重复释放资源。
	avformat_close_input(&avFmtCtx);

	cout << "Call Mp3Decoder::getMp3Attr() end." << endl;
	return 0;
}

