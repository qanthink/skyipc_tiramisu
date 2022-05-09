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

#if 1
/*
	功能：	
	返回：	
	注意：	
*/
int Mp3Decoder::recvPcmFrame(unsigned char*const dataBuff, const unsigned int dataSize)
{
	cout << "Call Mp3Decoder::recvPcmFrame()." << endl;

	const char *filePath = "/mnt";
	ifstream ifs((const char *)filePath, ios::in);
	if(!ifs)
	{
		cerr << "Fail to open mp3 file " << filePath << endl;
		return -1;
	}
	
	// 注册解码器
	//avcodec_register_all();
	//av_register_all();
	
	// 首先为mp3文件分配一个AVFormatContext数据结构
	AVFormatContext *mavFormatContext = avformat_alloc_context();

	// 打开文件，并解析文件，然后填充AVFormatContext数据结构
	// avformat_open_input(AVFormatContext **ps, const char *filename, AVInputFormat *fmt, AVDictionary **options);
	// 1. AVFormatContext数据结构 2. mp3文件名 3. 指定AVInputFormat，设置为NULL则自动检测 4.AVDictionary附加选项，一般设为NULL 
	int result = avformat_open_input(&mavFormatContext, filePath, NULL, NULL);
	if (result != 0) {
		//LOGI("can't open file %s result %s", fileString, av_err2str(result));
		return -1;
	}
	else
	{
		//LOGI("open file %s success and result is %s", fileString, av_err2str(result));
	}

	// avformat_find_stream_info(AVFormatContext *ic, AVDictionary **options);
	// 读取一部分视音频数据并且获得一些相关的信息
	result = avformat_find_stream_info(mavFormatContext, NULL);
	if (result < 0)
	{
		//LOGI("fail avformat avformat_find_stream_info %s result %s", filePath, av_err2str(result));
		return -1;
	}
	else
	{
		//LOGI("avformat_find_stream_info success result is %s", filePath, av_err2str(result));
	}

	// 获取mp3文件中音频对应的stream_index
	// 1. AVFormatContext 2. 指定为查找音频AVMEDIA_TYPE_AUDIO 3. wanted_stream_nb指定的stream号，-1为自动检测 4. related_stream找相关的stream 5. decoder_ret如果不为NULL，则返回选择的stream的decoder 6. 相关的flags 7. 返回值：返回相关的index
	int mstream_index = av_find_best_stream(mavFormatContext, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
	//LOGI("stream_index is %d", mstream_index);
	if (mstream_index == -1)
	{
		//LOGI("no audio stream");
		return -1;
	}
	AVStream* audioStream = mavFormatContext->streams[mstream_index];
	AVCodecParameters *mavCodecContext = audioStream->codecpar;
	//LOGI("avCodecContext->codec_id is %d AV_CODEC_ID_AAC is %d", mavCodecContext->codec_id, AV_CODEC_ID_AAC);
	// 找音频流解码器
	const AVCodec* avCodec = avcodec_find_decoder(mavCodecContext->codec_id);
	if (avCodec == NULL)
	{
		//LOGI("Unsupported codec");
		return -1;
	}
	// 打开音频流解码器
	AVCodecContext *avCtx = NULL;
	result = avcodec_open2(avCtx, avCodec, NULL);
	if (result < 0)
	{
		//LOGI("avcodec_open2 fail avformat_find_stream_info result is %s", av_err2str(result));
		return -1;
	}
	else
	{
		//LOGI("avcodec_open2 sucess avformat_find_stream_info result is %s", av_err2str(result));
	}

	// 读取流内容到packet中
	while(av_read_frame(mavFormatContext, &mpacket) >= 0)
	{
		if (mpacket.stream_index == mstream_index)
		{
			// avcodec_decode_audio4(AVCodecContext * avctx, AVFrame * frame, int * got_frame_ptr, const AVPacket * avpkt)
			// 1. 解码器 2. 输出数据frame 3. 是否获取到frame 4. 输入数据packet
			int len = avcodec_decode_audio4(mavCodecContext, mpAudioFrame, &gotframe, &mpacket);
			if (len < 0) {
			LOGI("decode audio error, skip packet");
			return -1;
		}
		if (gotframe)
		{
			// av_samples_get_buffer_size (int *linesize, int nb_channels, int nb_samples, enum AVSampleFormat sample_fmt, int align)
			// 1. linesize 2. 通道数 3. 单个声道的样本数 4. 输出采样格式 5. 对齐
			// 这个函数得到的结果是错的
			// AV_CH_LAYOUT_STEREO是3声道，av_get_default_channel_layout(OUT_PUT_CHANNELS)为3
			//int out_buffer_size=av_samples_get_buffer_size(NULL, AV_CH_LAYOUT_STEREO,
			//        mpAudioFrame->nb_samples, out_sample_fmt, 1);
			// 每一份frame的样本数 * 输出pcm的通道数 * 样本的采样格式（16bit）
			int out_buffer_size=mpAudioFrame->nb_samples *
			OUT_PUT_CHANNELS *
			av_get_bytes_per_sample(out_sample_fmt);
			LOGI("mpAudioFrame->nb_samples = %d", mpAudioFrame->nb_samples);
			int numChannels = OUT_PUT_CHANNELS;
			int numFrames = 0;
			void* audioData;
			// 重新采样
			if (mswrContext)
			{
				// swr_convert (struct SwrContext *s, uint8_t **out, int out_count, const uint8_t **in, int in_count)
				// 1. SwrContext上下文 2. 输出buffer 3. 输出的单通道的样本数 4. 输入数据 5. 输入的单通道的样本数
				numFrames = swr_convert(mswrContext, &out_buffer,
				mpAudioFrame->nb_samples,
				(const uint8_t **)mpAudioFrame->data,
				mpAudioFrame->nb_samples);
				if (numFrames < 0) {
				LOGI("fail resample audio");
				ret = -1;
				break;
			}
			LOGI("index:%5d\t pts:%lld\t packet size:%d out_buffer_size: %d\n",index,mpacket.pts,mpacket.size, out_buffer_size);
			//Write PCM
			// 写到pcm文件中
			fwrite(out_buffer, 1, 4608, pcmFile);
			index++;
			av_packet_unref(&mpacket);
		}
		}
		LOGI(" free");
		}
	}

	ifs.close();

	cout << "Call Mp3Decoder::recvPcmFrame() end." << endl;
	return 0;
}

#endif

int Mp3Decoder::getMp3Frame4Bytes(const char *filePath, unsigned char *p4BytesData)
{
	ifstream ifs(filePath, ios::in);
	if(!ifs)
	{
		cerr << "Fail to open file in Mp3Decoder::analyzeMp3Frame()." << endl;
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
			cerr << "Fail to call read() in Mp3Decoder::analyzeMp3Frame()." << endl;
			break;
		}
		
		if(0xFF == readData[0])
		{
			ifs.read(readData + 1, 1);
			if(!ifs)
			{
				cerr << "Fail to call read() in Mp3Decoder::analyzeMp3Frame()." << endl;
				break;
			}
		}
		
		bIsMP3FrameData = (0xFF == readData[0] && 0xFB == readData[1]);
	}

	if(!bIsMP3FrameData)
	{
		cerr << "In Mp3Decoder::analyzeMp3Frame(). Fail to analyze MP3 file." << endl;
		ifs.close();
		return -2;
	}

	ifs.read(readData + 2, 2);
	if(!ifs)
	{
		cerr << "Fail to call read() in Mp3Decoder::analyzeMp3Frame()." << endl;
		ifs.close();
		return -3;
	}

	ifs.close();
	#if 1	// debug
	cout << "In Mp3Decoder::getMp3Frame4Bytes: get 4 bytes: " << "0x" << hex << setfill('0') << 
		setw(2) << (int)readData[0] << setw(2) << (int)readData[1] << 
		setw(2) << (int)readData[2] << setw(2) << (int)readData[3] << endl;
	#endif

	if(NULL == p4BytesData)
	{
		cerr << "In Mp3Decoder::getMp3Frame4Bytes(). Argument has null value." << endl;
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

int Mp3Decoder::analyzeMp3Frame(const char *filePath, long long int *pRate, long long int *pChLayout, AVSampleFormat *pAvSampleFmt)
{
	int ret = 0;
	const unsigned int dataSize = 4;
	unsigned char p4BytesData[dataSize] = {0};

	ret = getMp3Frame4Bytes(filePath, p4BytesData);
	if(0 != ret)
	{
		cerr << "Fail to call getMp3Frame4Bytes() in Mp3Decoder::analyzeMp3Frame()." << endl;
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

	if(NULL != pRate)
	{
		*pRate = bitRate;
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

int Mp3Decoder::pcmFileResample(const char *dstPcmPath, long long int dstRate, long long int dstChLayout, AVSampleFormat dstAvSampleFmt, \
		const char *srcPcmPath, long long int srcRate, long long int srcChLayout, AVSampleFormat srcAvSampleFmt)
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
	av_opt_set_int(swrCtx, "in_sample_rate", srcRate, 0);
	av_opt_set_sample_fmt(swrCtx, "in_sample_fmt", srcAvSampleFmt, 0);

	av_opt_set_int(swrCtx, "out_channel_layout", dstChLayout, 0);
	av_opt_set_int(swrCtx, "out_sample_rate", dstRate, 0);
	av_opt_set_sample_fmt(swrCtx, "out_sample_fmt", dstAvSampleFmt, 0);

	// 3、初始化上下文
	int ret = 0;
	ret = swr_init(swrCtx);
	if(ret < 0) 
	{
		cerr << "In Mp3Decoder::pcmFileResample(): swr_init() fail" << endl;
		return -3;
	}

	int inNbChannels = 0;		// 声道数
	int outNbChannels = 0;
	inNbChannels = av_get_channel_layout_nb_channels(srcChLayout);
	outNbChannels = av_get_channel_layout_nb_channels(dstChLayout);
	
	// 根据srcAvSampleFmt、srcNbSamples、inNbChannels 为inData 分配内存空间，和设置对应的的linesize 的值；返回分配的总内存的大小
	int srcBufSize = 0;
	int dstBufSize = 0;
	
	int srcLineSize = 0;
	int dstLineSize = 0;
	
	long long int dstNbSamples = 0;
	long long int dstMaxNbSamples = 0;
	const int srcNbSamples = 1024;
	
	unsigned char **inData = NULL;
	unsigned char **outData = NULL;
	
	srcBufSize = av_samples_alloc_array_and_samples(&inData, &srcLineSize, inNbChannels, srcNbSamples, srcAvSampleFmt, 0);
	// 根据srcNbSamples*dstRate/srcRate公式初步估算重采样后音频的nb_samples大小
	dstNbSamples = av_rescale_rnd(srcNbSamples, dstRate, srcRate, AV_ROUND_UP);
	dstMaxNbSamples = dstNbSamples;
	dstBufSize = av_samples_alloc_array_and_samples(&outData, &dstLineSize, outNbChannels, dstNbSamples, dstAvSampleFmt, 0);

	while(ifs)
	{
		size_t readBytes = 0;
		ifs.read((char *)inData[0], srcBufSize);
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
		dstNbSamples = av_rescale_rnd(swr_get_delay(swrCtx, srcRate) + srcNbSamples, dstRate, srcRate, AV_ROUND_UP);
		if(dstNbSamples > dstMaxNbSamples)
		{
			cerr << "In Mp3Decoder::pcmFileResample(): realloc memory." << endl;
			// 先释放以前的内存，不管sample_fmt 是planner 还是packet 方式，av_samples_alloc_array_and_samples()函数都是分配的一整块连续的内存
			av_freep(&outData[0]);
			dstBufSize = av_samples_alloc_array_and_samples(&outData, &dstLineSize, outNbChannels, dstNbSamples, dstAvSampleFmt, 0);
			dstMaxNbSamples = dstNbSamples;
		}
 
		// 开始重采样，重采样后的数据将根据前面指定的存储方式写入ds_data 内存块中，返回每个声道实际的采样数。
		/*	swr_convert() 返回的result 是实际转换的采样个数，该值小于或等于预计采样数dstNbSamples, 
			所以写入文件的时候不能用dstNbSamples的  值，而应该用result值。*/
		int result = swr_convert(swrCtx, outData, dstNbSamples, (const unsigned char **)inData, srcNbSamples);
		if(result < 0)
		{
			cerr << "In Mp3Decoder::pcmFileResample() swr_convert() fail. Result = " << result << endl;
			break;
		}

		#if 0	// debug
		printf("readBytes %d, dstNbSamples %d, srcNbSamples %d, result %d.", readBytes, dstNbSamples, srcNbSamples, result);
		#endif
		
		// 将音频数据写入pcm文件
		if(av_sample_fmt_is_planar(dstAvSampleFmt))
		{
			// planner方式。pcm文件写入时一般都是packet方式，所以这里要注意转换一下。
			int perSampleBytes = 0;
			perSampleBytes = av_get_bytes_per_sample(dstAvSampleFmt);
			// 这里必须是result，而不能用dstNbSamples,因为result 才代表此次实际转换的采样数，它肯定小于或等于dstNbSamples.
			int i = 0;
			for(i = 0; i < result; ++i)
			{
				int j = 0;
				for(j = 0; j < outNbChannels; ++j)
				{
					ofs.write((char *)outData[j] + i * perSampleBytes, perSampleBytes);
				}
			}
		}
		else
		{
			// 最后一个参数必须为1, 否则会因为cpu 对齐算出来的大小大于实际的数据大小，导致多写入数据 造成错误。
			dstBufSize = av_samples_get_buffer_size(&dstLineSize, outNbChannels, result, dstAvSampleFmt, 1);
			ofs.write((char *)outData[0], dstBufSize);
		}
	}

	// 还有剩余的缓存数据没有转换，第三个传递NULL, 第四个传递0, 即可将缓存中的全部取出
	do{
		int realNbSamples = 0;
		realNbSamples = swr_convert(swrCtx, outData, dstNbSamples, NULL, 0);
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
				for(j = 0; j < outNbChannels; ++j)
				{
					ofs.write((char *)outData[j] + i * perSampleBytes, perSampleBytes);
				}
			}
		}
		else
		{
			int size = 0;
			size = av_samples_get_buffer_size(NULL, outNbChannels, realNbSamples, dstAvSampleFmt, 1);
			ofs.write((char *)outData[0], size);
		}
	}while(true);

	// 释放资源
	av_freep(&inData[0]);
	av_freep(&outData[0]);
	swr_free(&swrCtx);
	ifs.close();
	ofs.close();

	return 0;
}


