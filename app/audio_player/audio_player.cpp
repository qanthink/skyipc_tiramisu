/*---------------------------------------------------------------- 
qanthink 版权所有。
作者：
时间：2022.2.13
----------------------------------------------------------------*/

/*
*/

#define RUNDE_LICENSE 1
#define FREE_COUNT 3	// 100 -> 3s.

#include "audio_player.h"
#include "ffmpeg.h"
#include "mp3decoder.h"

#include <thread>
#include <unistd.h>
#include <fstream>
#include <iomanip>
#include <string.h>
#include <iostream>

using namespace std;

AudioPlayer* AudioPlayer::getInstance()
{
	static AudioPlayer audioPlayer;
	return &audioPlayer;
}

AudioPlayer::AudioPlayer()
{
}

AudioPlayer::~AudioPlayer()
{
}

/*
	功能：	播放PCM 文件，用户接口。
	返回：	
	注意：	
*/
int AudioPlayer::playPCM(const char *filePath)
{
	cout << "Call AudioPlayer::playPCM()." << endl;
	thread th(thPlayRoutePCM, filePath);
	th.detach();
	cout << "Call AudioPlayer::playPCM() end." << endl;
	return 0;
}

/*
	功能：	播放PCM 文件，内部实现，类内线程转换。
	返回：	
	注意：	
*/
int AudioPlayer::thPlayRoutePCM(const char *filePath)
{
	AudioPlayer *pThis = (AudioPlayer *)filePath;
	return pThis->playRoutePCM(filePath);
}

/*
	功能：	播放PCM 文件，内部实现。
	返回：	0, 成功；-1, 参数无效；-2, 文明打开失败。
	注意：	
*/
int AudioPlayer::playRoutePCM(const char *filePath)
{
	cout << "Call AudioPlayer::playRoutePMC()." << endl;
	if(NULL == filePath)
	{
		cerr << "Fail to call AudioPlayer::playRoutePMC(), argument has null value!" << endl;
		return -1;
	}

#if 0 == RUNDE_LICENSE
	static int cnt = 0;
	++cnt;
	if(cnt > FREE_COUNT)
	{
		cout << "Copyright by qanthink@163.com." << endl;
		return -3;
	}
#endif


	ifstream ifs((const char *)filePath, ios::in);
	if(ifs.fail())
	{
		cerr << "Fail to open file: " << filePath << ". " << strerror(errno) << endl;
		return -2;
	}
	cout << "Success to open " << filePath << endl;

	while(!ifs.eof())
	{
		int readBytes = 0;
		unsigned int dataBufMaxSize = 1 * 1024;
		char dataBuf[dataBufMaxSize] = {0};
		
		//cout << "Ready to call read() in routeAo()." << endl;
		ifs.read(dataBuf, dataBufMaxSize);
		readBytes = ifs.gcount();
		if(!ifs)
		{
			cout << "error: only " << readBytes << " could be read";
			break;
		}
		//cout << "readBytes = " << readBytes << endl;

		//cout << "Send pcm stream" << endl;
		AudioOut::getInstance()->sendStream(dataBuf, readBytes);
	}


	ifs.close();

	cout << "Call AudioPlayer::playRoutePMC() end." << endl;
	return 0;
}

/*
============================== WAV =====================================
============================== WAV =====================================
*/

/*
	功能：	播放WAV 文件，用户接口。
	返回：	
	注意：	
*/
int AudioPlayer::playWAV(const char *filePath)
{
	cout << "Call AudioPlayer::playWAV()." << endl;
	thread th(thPlayRouteWAV, filePath);
	th.detach();
	cout << "Call AudioPlayer::playWAV() end." << endl;
	return 0;
}

/*
	功能：	播放WAV 文件，内部实现，类内线程转换。
	返回：	
	注意：	
*/
int AudioPlayer::thPlayRouteWAV(const char *filePath)
{
	AudioPlayer *pThis = (AudioPlayer *)filePath;
	return pThis->playRouteWAV(filePath);
}

/*
	功能：	播放WAV 文件，内部实现。
	返回：	0, 成功；-1, 参数无效；-2, 文件打开失败；-3, 文件读取失败。
	注意：	
*/
int AudioPlayer::playRouteWAV(const char *filePath)
{
	cout << "Call AudioPlayer::playRouteWAV()." << endl;
	if(NULL == filePath)
	{
		cerr << "Fail to call AudioPlayer::playRouteWAV(), argument has null value!" << endl;
		return -1;
	}

	
#if 0 == RUNDE_LICENSE
	static int cnt = 0;
	++cnt;
	if(cnt > FREE_COUNT)
	{
		cout << "Copyright by qanthink@163.com." << endl;
		return -3;
	}
#endif

	// 打开文件
	ifstream ifs((const char *)filePath, ios::in);
	if(ifs.fail())
	{
		cerr << "Fail to open file: " << filePath << ". " << strerror(errno) << endl;
		return -2;
	}
	cout << "Success to open " << filePath << endl;

	// 获取WAV 头部字节数。
	unsigned int wavHeaderBytes = 0;
	wavHeaderBytes = getWavHeaderBytes(filePath);
	//cout << "Wav file header Bytes = " << wavHeaderBytes << endl;
	
	int readBytes = 0;
	unsigned int dataBufMaxSize = 1024 * 1;
	char dataBuf[dataBufMaxSize] = {0};
	
	ifs.read(dataBuf, wavHeaderBytes);
	if(!ifs)
	{
		cerr << "In AudioPlayer::playRouteWAV(): read fail." << endl;
		ifs.close();
		return -3;
	}

	// 跳过头部，循环读取音频正文。
	while(!ifs.eof())
	{
		ifs.read(dataBuf, dataBufMaxSize);
		readBytes = ifs.gcount();
		if(!ifs)
		{
			//cout << "error: only " << readBytes << " could be read";
			cout << "read over." << endl;
			break;
		}
		//cout << "readBytes = " << readBytes << endl;

		AudioOut::getInstance()->sendStream(dataBuf, readBytes);
	}

	ifs.close();

	cout << "Call AudioPlayer::playRouteWAV() end." << endl;
	return 0;
}

/*
	功能：	获取WAV 头部信息字节数。
	返回：	成功，返回头部信息的字节数；失败，返回0.
	注意：	调用该函数统计头部信息字节数，从而可以跳过这些字节，读取音频正文数据。
*/
int AudioPlayer::getWavHeaderBytes(const char *filePath)
{
	cout << "Call AudioPlayer::getWavHeaderBytes()." << endl;
	if(NULL == filePath)
	{
		cerr << "Fail to call AudioPlayer::getWavHeaderBytes(), argument has null value!" << endl;
		return 0;
	}
	
	ifstream ifs((const char *)filePath, ios::in);
	if(ifs.fail())
	{
		cerr << "Fail to open file: " << filePath << ". " << strerror(errno) << endl;
		return 0;
	}

	int headerBytes = 0;
	unsigned readBytes = 0;
	char dataBuf[sizeof(stWavFileHeader_t)] = {0};

	// 读取WAV 头部信息。不包含'd', 'a', 't', 'a' 字段。
	ifs.read(dataBuf, sizeof(stWavFileHeader_t));
	readBytes = ifs.gcount();
	if(!ifs)
	{
		cerr << "Fail to analyze wav file." << endl;
		ifs.close();
		return 0;
	}

	// 寻找'd', 'a', 't', 'a' 字段，每次偏移2 字节，读取4 字节。
	const unsigned int dataLen = 4;
	while(!ifs.eof())
	{
		memset(dataBuf, 0, dataLen);

		readBytes += 4;
		ifs.read(dataBuf, dataLen);
		if('d' == dataBuf[0] && 'a' == dataBuf[1] && 't' == dataBuf[2] && 'a' == dataBuf[3])
		{
			cout << "Find format segment of 'data'" << endl;
			//cout << "This wav file has " << readBytes << " + 4 Bytes header data." << endl;
			break;
		}
		//cout << dataBuf[0] << dataBuf[1] << dataBuf[2] << dataBuf[3] << endl;
		
		if(!ifs)
		{
			cerr << "Fail to call read() in AudioPlayer::getWavHeaderBytes()." << endl;
			ifs.close();
			return 0;
		}

		readBytes -= 2;
		ifs.seekg(-2, ios::cur);
	}

	// 获取WAV 文件音频数据正文的长度。
	readBytes += 4;
	ifs.read(dataBuf, dataLen);
	if(!ifs)
	{
		cerr << "Fail to call read() in AudioPlayer::getWavHeaderBytes()." << endl;
		ifs.close();
		return readBytes;
	}
	#if 0	// debug
	cout << hex << setfill('0') << setw(2) << (int)dataBuf[0] << setw(2) << (int)dataBuf[1] 
		<< setw(2) << (int)dataBuf[2] << setw(2) << (int)dataBuf[3] << endl;
	cout << dec;
	#endif

	unsigned int realBytes = 0;
	realBytes |= dataBuf[3];
	realBytes <<= 8;
	realBytes |= dataBuf[2];
	realBytes <<= 8;
	realBytes |= dataBuf[1];
	realBytes <<= 8;
	realBytes |= dataBuf[0];
	//cout << "This wav file has " << dec << realBytes << " Bytes audio data." << endl;

	ifs.close();
	return readBytes;
}

int readWavHead(const char *filePath)
{
	int i = 0; //用作循环计数
	unsigned char ch[100]; //用来存储wav文件的头信息

	FILE *fp;
	fp=fopen(filePath,"rb");//为读，打开一个wav文件

	//现在只能读取头文件，可是头文件后的数据要怎么读出来，求大神指教
	/**********输出wav文件的所有信息**********/
	printf("该wav文件的所有信息:");
	for(i=0;i<58;i++)
	{
		ch[i]=fgetc(fp); //每次读取一个字符，存在数组ch中
		if(i%16==0) //每行输出16个字符对应的十六进制数
		printf("\n");
		if(ch[i]<16) //对小于16的数，在前面加0，使其用8bit显示出来
		printf("0%x",ch[i]);
		else
		printf("%x ",ch[i]);
	}

	/*********RIFF WAVEChunk的输出*********/
	printf("\n\nRIFF WAVE Chunk信息:");
	//输出RIFF标志
	printf("\nRIFF标志:");
	for(i=0;i<4;i++)
	{
		printf("%x ",ch[i]);
	}
	//输出size大小
	printf("\nsize:ox");
	for(i=7;i>=4;i--) //低字节表示数值低位，高字节表示数值高位
	{
		if(ch[i]<16)
		printf("0%x",ch[i]);
		else
		printf("%x",ch[i]);
	}
	
	//输出WAVE标志
	printf("\nWAVE标志:");
		for(i=8;i<12;i++)
		{
		if(ch[i]<16)
		printf("0%x ",ch[i]);
		else
		printf("%x ",ch[i]);
	}

	/*******Format Chunk的输出*******/
	printf("\n\nFormat Chunk信息:");
	//输出fmt 标志
	printf("\nfmt 标志:");
	for(i=12;i<16;i++)
	{
		if(ch[i]<16)
		printf("0%x ",ch[i]);
		else
		printf("%x ",ch[i]);
	}

	//输出size段
	printf("\nsize:ox");
	for(i=19;i>15;i--)
	{
		if(ch[i]<16)
		printf("0%x",ch[i]);
		else
		printf("%x",ch[i]);
	}

	//输出编码方式
	printf("\n编码方式:ox");
	for(i=21;i>19;i--)
	{
		if(ch[i]<16)
		printf("0%x",ch[i]);
		else
		printf("%x",ch[i]);
	}

	//输出声道数目
	printf("\n声道数目:ox");
	for(i=23;i>21;i--)
	{
		if(ch[i]<16)
		printf("0%x",ch[i]);
		else
		printf("%x",ch[i]);
	}
	if(ch[i+1]==1) //1表示单声道，2表示双声道
		printf(" 单声道");
	else
		printf(" 双声道");

	//输出采样频率
	printf("\n采样频率:ox");
	for(i=27;i>23;i--)
	{
	if(ch[i]<16)	
		printf("0%x",ch[i]);
	else
		printf("%x",ch[i]);
	}

	//输出每秒所需字节数
	printf("\n每秒所需字节数:ox");
	for(i=31;i>27;i--)
	{
		if(ch[i]<16)
			printf("0%x",ch[i]);
		else
			printf("%x",ch[i]);
	}

	//输出数据块对齐单位
	printf("\n数据块对齐单位:ox");
	for(i=33;i>31;i--)
	{
		if(ch[i]<16)
			printf("0%x",ch[i]);
		else
			printf("%x",ch[i]);
	}

	//输出每个采样所需bit数
	printf("\n每个采样所需bit数:ox");
	for(i=35;i>33;i--)
	{
		if(ch[i]<16)
			printf("0%x",ch[i]);
		else
			printf("%x",ch[i]);
	}

	//输出附加信息
	if(ch[16]==18) //若Format Chunk的size大小为18，则该模块的最后两个字节为附加信息
	{ //若为16，则无附加信息
		printf("\n附加信息:ox");
		for(i=37;i>35;i--)
		{
			if(ch[i]<16)
			printf("0%x",ch[i]);
				else
					printf("%x",ch[i]);
		}
	}

	/*******Fact Chunk的输出*******/
	printf("\n\nFact Chunk信息:");
	//输出fact标志
	printf("\nfact标志:");
	for(i=38;i<42;i++)
	{
		if(ch[i]<16)
			printf("0%x ",ch[i]);
		else
			printf("%x ",ch[i]);
	}

	//输出size
	printf("\nsize:ox");
	for(i=45;i>41;i--)
	{
		if(ch[i]<16)
			printf("0%x",ch[i]);
		else
			printf("%x",ch[i]);
	}

	//输出data段数据
	printf("\ndata段数据:");
	for(i=46;i<50;i++)
	{
		if(ch[i]<16)
			printf("0%x ",ch[i]);
		else
			printf("%x ",ch[i]);
	}

	/*******Data Chunk的输出*******/
	printf("\n\nData Chunk信息:");
	//输出data标志
	printf("\ndata标志:");
	for(i=50;i<54;i++)
	{
		if(ch[i]<16)
			printf("0%x ",ch[i]);
		else
			printf("%x ",ch[i]);
	}

	//输出数据大小
	printf("\n数据大小:ox");
	for(i=57;i>53;i--)
	{
		if(ch[i]<16)
			printf("0%x",ch[i]);
		else
			printf("%x",ch[i]);
	}

	printf("\n");
	fclose(fp);
	return 0;
}

/*
============================== MP3 =====================================
============================== MP3 =====================================
*/

/*
	功能：	播放MP3 文件，用户接口。
	返回：	
	注意：	
*/
int AudioPlayer::playMP3(const char *filePath)
{
	cout << "Call AudioPlayer::playMP3()." << endl;
	thread th(thPlayRouteMP3, filePath);
	th.detach();
	cout << "Call AudioPlayer::playMP3() end." << endl;
	return 0;
}

/*
	功能：	播放MP3 文件，内部实现，类内线程转换。
	返回：	
	注意：	
*/
int AudioPlayer::thPlayRouteMP3(const char *filePath)
{
	AudioPlayer *pThis = (AudioPlayer *)filePath;
	return pThis->playRouteMP3(filePath);
}

/*
	功能：	播放MP3 文件，内部实现。
	返回：	0, 成功；-1, 参数无效；-2, 文件打开失败；-3, 文件读取失败。
	注意：	
*/
int AudioPlayer::playRouteMP3(const char *filePath)
{
	cout << "Call AudioPlayer::playRouteMP3()." << endl;
	// 打开MP3 输入文件
	int ret = 0;
	AVFormatContext *avFmtCtx = NULL;
	avFmtCtx = avformat_alloc_context();
	ret = avformat_open_input(&avFmtCtx, filePath, NULL,NULL);
	if(ret < 0)
	{
		cerr << "In Mp3Decoder::mp3ToPcm(): Fail to open file " << filePath << endl;
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
	av_dump_format(avFmtCtx, 0, filePath, 0);
	//cout << avFmtCtx->streams[0]->codecpar->sample_rate << endl;

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
#if 0 == RUNDE_LICENSE
	static int cnt = 0;
	++cnt;
	if(cnt > 50)
	{
		cout << "Copyright by qanthink@163.com." << endl;
		return -3;
	}
#endif
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
				const unsigned int pcmLen = 1024 * 16;
				unsigned char pcmData[pcmLen] = {0};
				
				//pcm播放时是LRLRLR格式，所以要交错保存数据
				for(i = 0; i < avFrame->nb_samples; ++i)
				{
					int ch = 0;
					for(ch = 0; ch < avCodecCtx->channels; ++ch)
					{
						// 获取每次采样得到的字节数。此处为4 字节。
						//数据：(char*)avFrame->data[ch] + numBytes * i; 长度：numBytes;
						numBytes = av_get_bytes_per_sample(avCodecCtx->sample_fmt);
						#if 1	// stero
						memcpy(pcmData + numBytes * i * 2 + numBytes * ch, avFrame->data[ch] + numBytes * i, numBytes);
						#else	// MONO
						if(0 == ch)
						{
							memcpy(pcmData + numBytes * i * 1 + numBytes * ch, avFrame->data[ch] + numBytes * i, numBytes);
						}
						#endif
					}
				}

				int pcmRealLen = 0;
				#if 1	//stero
				//cout << "i = " << i << endl;
				pcmRealLen = numBytes * i * 2;
				#else	// mono
				pcmRealLen = numBytes * i * 1;
				#endif
				//cout << "avFrame->nb_samples, realBytes = " << avFrame->nb_samples << ", " << pcmRealLen << endl;
				
				#if 1
				Mp3Decoder *pMp3Decoder = Mp3Decoder::getInstance();
				const unsigned int dstPcmLen = 1024 * 16;
				char dstPcmData[dstPcmLen] = {0};
				int realLen = 0;
				int srcSamples = avFmtCtx->streams[audioStreamIndex]->codecpar->sample_rate;
				realLen = pMp3Decoder->pcmDataResample(dstPcmData, dstPcmLen, 16000, AV_CH_LAYOUT_MONO, AV_SAMPLE_FMT_S16, \
					(char *)pcmData, pcmRealLen, srcSamples, AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_FLT, avFrame->nb_samples);
				#endif

				AudioOut *pAudioPout = AudioOut::getInstance();
				pAudioPout->sendStream(dstPcmData, realLen);
			}
		}

		av_packet_unref(avPacket);
	}

	av_frame_free(&avFrame);
	av_packet_free(&avPacket);
	avcodec_close(avCodecCtx);
	avcodec_free_context(&avCodecCtx);
	avformat_free_context(avFmtCtx);

	cout << "Call AudioPlayer::playRouteMP3() end." << endl;
	return 0;
}

/*
	功能：	获取MP3 头部信息字节数。
	返回：	成功，返回头部信息的字节数；失败，返回0.
	注意：	调用该函数统计头部信息字节数，从而可以跳过这些字节，读取音频正文数据。
*/
int AudioPlayer::getMP3HeaderBytes(const char *filePath)
{
	cout << "Call AudioPlayer::getMP3HeaderBytes()." << endl;
	if(NULL == filePath)
	{
		cerr << "Fail to call AudioPlayer::getMP3HeaderBytes(), argument has null value!" << endl;
		return 0;
	}
	
	ifstream ifs((const char *)filePath, ios::in);
	if(ifs.fail())
	{
		cerr << "Fail to open file: " << filePath << ". " << strerror(errno) << endl;
		return 0;
	}

	int headerBytes = 0;
	unsigned readBytes = 0;
	char dataBuf[sizeof(stWavFileHeader_t)] = {0};

	// 读取MP3 头部信息。不包含'd', 'a', 't', 'a' 字段。
	ifs.read(dataBuf, sizeof(stWavFileHeader_t));
	readBytes = ifs.gcount();
	if(!ifs)
	{
		cerr << "Fail to analyze wav file." << endl;
		ifs.close();
		return 0;
	}

	// 寻找'd', 'a', 't', 'a' 字段，每次偏移2 字节，读取4 字节。
	const unsigned int dataLen = 4;
	while(!ifs.eof())
	{
		memset(dataBuf, 0, dataLen);

		readBytes += 4;
		ifs.read(dataBuf, dataLen);
		if('d' == dataBuf[0] && 'a' == dataBuf[1] && 't' == dataBuf[2] && 'a' == dataBuf[3])
		{
			cout << "Find format segment of 'data'" << endl;
			//cout << "This wav file has " << readBytes << " + 4 Bytes header data." << endl;
			break;
		}
		//cout << dataBuf[0] << dataBuf[1] << dataBuf[2] << dataBuf[3] << endl;
		
		if(!ifs)
		{
			cerr << "Fail to call read() in AudioPlayer::getMP3HeaderBytes()." << endl;
			ifs.close();
			return 0;
		}

		readBytes -= 2;
		ifs.seekg(-2, ios::cur);
	}

	// 获取MP3 文件音频数据正文的长度。
	readBytes += 4;
	ifs.read(dataBuf, dataLen);
	if(!ifs)
	{
		cerr << "Fail to call read() in AudioPlayer::getMP3HeaderBytes()." << endl;
		ifs.close();
		return readBytes;
	}
	#if 0	// debug
	cout << hex << setfill('0') << setw(2) << (int)dataBuf[0] << setw(2) << (int)dataBuf[1] 
		<< setw(2) << (int)dataBuf[2] << setw(2) << (int)dataBuf[3] << endl;
	cout << dec;
	#endif

	unsigned int realBytes = 0;
	realBytes |= dataBuf[3];
	realBytes <<= 8;
	realBytes |= dataBuf[2];
	realBytes <<= 8;
	realBytes |= dataBuf[1];
	realBytes <<= 8;
	realBytes |= dataBuf[0];
	//cout << "This wav file has " << dec << realBytes << " Bytes audio data." << endl;

	ifs.close();
	return readBytes;
}


