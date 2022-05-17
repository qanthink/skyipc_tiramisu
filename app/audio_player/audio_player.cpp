/*---------------------------------------------------------------- 
qanthink 版权所有。
作者：
时间：2022.2.13
----------------------------------------------------------------*/

/*
*/

#define RUNDE_LICENSE 1
#define FREE_COUNT 5	// 100 -> 3s.

#include "audio_player.h"
#include "mp3decoder.h"

#include <fstream>
#include <iostream>

using namespace std;

AudioPlayer* AudioPlayer::getInstance()
{
	static AudioPlayer audioPlayer;
	return &audioPlayer;
}

AudioPlayer::AudioPlayer()
{
	cout << "Call AudioPlayer::AudioPlayer()." << endl;
	cout << "Call AudioPlayer::AudioPlayer() end." << endl;
}

AudioPlayer::~AudioPlayer()
{
	cout << "Call AudioPlayer::~AudioPlayer()." << endl;
	bPlaying = false;
	if(NULL != pTh)
	{
		pTh->join();
		pTh = NULL;
	}
	cout << "Call AudioPlayer::~AudioPlayer() end." << endl;
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
	cout << "Call AudioPlayer::thPlayRoutePCM()." << endl;

	AudioPlayer *pThis = (AudioPlayer *)filePath;
	return pThis->playRoutePCM(filePath);
}

/*
	功能：	播放PCM 文件，内部实现。
	返回：	0, 成功；-1, 参数无效；-2, 文件打开失败。
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

		#if 0	// debug
		cout << "readBytes = " << readBytes << endl;
		cout << "Send pcm stream" << endl;
		#endif
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
	cout << "Call AudioPlayer::thPlayRouteWAV()." << endl;
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
	
	cout << "Call AudioPlayer::getWavHeaderBytes() end." << endl;
	return readBytes;
}

int AudioPlayer::readWavHead(const char *filePath)
{
	int i = 0;				//用作循环计数
	unsigned char ch[100];	//用来存储wav文件的头信息

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
	for(i=7;i>=4;i--)		//低字节表示数值低位，高字节表示数值高位
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
	pTh = make_shared<thread>(thPlayRouteMP3, filePath);
	//th.detach();
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
	cout << "Call AudioPlayer::thPlayRouteMP3()." << endl;
	AudioPlayer *pThis = (AudioPlayer *)filePath;
	return pThis->playRouteMP3(filePath);
}

/*
	功能：	播放MP3 文件，内部实现。
	返回：	0, 成功；-1, 参数无效；-2, 文件打开失败；-3, 文件读取失败。
	注意：	目前只实现了立体声转单声道，暂不支持立体声转立体声。
*/
int AudioPlayer::playRouteMP3(const char *filePath)
{
	cout << "Call AudioPlayer::playRouteMP3()." << endl;

	// 1. 获取MP3 属性：声道数、采样率等。
	int srcNbSamples = 0;
	long long int srcChLayout = 0;
	long long int srcSampleRate = 0;
	AVSampleFormat srcAvSampleFmt = AV_SAMPLE_FMT_NONE;

	Mp3Decoder *pMp3Decoder = Mp3Decoder::getInstance();
	pMp3Decoder->getMp3Attr(filePath, &srcSampleRate, &srcChLayout, &srcAvSampleFmt, &srcNbSamples);
	#if 1	// debug
	cout << "In AudioPlayer::playRouteMP3(). Mp3 attr: " 
		<< "srcSampleRate = " << srcSampleRate << ", srcChLayout = " << srcChLayout 
		<< ", srcAvSampleFmt = " << srcAvSampleFmt << ",  srcNbSamples = " << srcNbSamples << endl;
	#endif
	
	// 2. 开始解码MP3 文件。
	pMp3Decoder->mp3Decoding(filePath);
	
	// 3. 获取解码数据
	// 源音频数据
	unsigned int srcRealSize = 0;
	const unsigned int srcDataSize = 1024 * 16;
	unsigned char srcDataBuff[srcDataSize] = {0};
	while((srcRealSize = pMp3Decoder->recvPcmFrame(srcDataBuff, srcDataSize)) > 0 && bPlaying)
	{
#if 0 == RUNDE_LICENSE
		static int cnt = 0;
		++cnt;
		if(cnt > FREE_COUNT + 200)
		{
			cout << "Copyright by qanthink@163.com." << endl;
			return -3;
		}
#endif
		//cout << "srcRealSize = " << srcRealSize << endl;

		// 目标音频数据
		int dstRealSize = 0;
		const unsigned int dstDataSize = 1024 * 16;
		unsigned char dstDataBuff[dstDataSize] = {0};

		// 目标音频格式
		const long long int dstSampleRate = 16000;
		const long long int dstChLayout = AV_CH_LAYOUT_MONO;
		const AVSampleFormat dstAvSampleFmt = AV_SAMPLE_FMT_S16;

		// 4. 重采样。
		dstRealSize = pMp3Decoder->pcmDataResample(
						dstDataBuff, dstDataSize, dstSampleRate, dstChLayout, dstAvSampleFmt, 
						srcDataBuff, srcRealSize, srcSampleRate, srcChLayout, srcAvSampleFmt, srcNbSamples);

		// 5. AO 播放。
		if(dstRealSize > 0)
		{
			AudioOut *pAudioOut = AudioOut::getInstance();
			pAudioOut->sendStream(dstDataBuff, dstRealSize);
		}

		#if 0	// debug
		cout << "srcRealSize = " << srcRealSize << endl;
		cout << "dstRealSize = " << dstRealSize << endl;
		#endif
	}

	bPlaying = false;

	cout << "Call AudioPlayer::playRouteMP3() end." << endl;
	return 0;
}

