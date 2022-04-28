/*---------------------------------------------------------------- 
xxx版权所有。
作者：
时间：2020.7.10
----------------------------------------------------------------*/

/*
免责声明：
本程序基于faad 开源代码进行开发，请遵守faad 开源规则。
*/

#include "aad.h"
#include <iostream>

using namespace std;

Aad *Aad::getInstance()
{
	static Aad aad;
	return &aad;
}

Aad::Aad()
{
	enable();
}

Aad::~Aad()
{
	disable();
}

/*-----------------------------------------------------------------------------
描--述：
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
int Aad::enable()
{
	cout << "Call Aad::enable()." << endl;

	if(bEnable)
	{
		return 0;
	}

	openDecoder();
	setDecoderConf(NULL);
	getDecoderConf(true);
	bEnable = true;
	
	cout << "Call Aad::enable() end." << endl;
	return 0;
}

/*-----------------------------------------------------------------------------
描--述：
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
int Aad::disable()
{
	cout << "Call Aad::disable()." << endl;

	closeDecoder();
	hDecoder = NULL;
	bEnable = false;

	cout << "Call Aad::disable() end." << endl;
	return 0;
}

/*-----------------------------------------------------------------------------
描--述：
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
int Aad::getFaadVersion()
{
	cout << "Call Aad::getFaadVersion()." << endl;

	bool bShow = true;

	if(bShow)
	{
		cout << "FAAD version = " << FAAD2_VERSION << endl;
	}
	
	cout << "Call Aad::getFaadVersion() end." << endl;
	return 0;
}

/*-----------------------------------------------------------------------------
描--述：
参--数：
返回值：返回解码器文件描述符
注--意：
-----------------------------------------------------------------------------*/
NeAACDecHandle Aad::openDecoder()
{
	cout << "Call Aad::openDecoder()." << endl;

	hDecoder = NeAACDecOpen();
	if(NULL == hDecoder)
	{
		cerr << "Fail to call NeAACDecOpen()." << endl;
	}

	cout << "Call Aad::openDecoder() end." << endl;
	return hDecoder;
}

/*-----------------------------------------------------------------------------
描--述：关闭解码器
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
void Aad::closeDecoder()
{
	cout << "Call Aad::closeDecoder()." << endl;

	if(NULL == hDecoder)
	{
		NeAACDecClose(hDecoder);
		hDecoder = NULL;
	}
	
	cout << "Call Aad::closeDecoder() end." << endl;
}

/*-----------------------------------------------------------------------------
描--述：获取解码器参数
参--数：bShow 是否打印解码器参数
返回值：成功，返回指向解码器参数结构体的指针；失败，返回NULL.
注--意：
-----------------------------------------------------------------------------*/
NeAACDecConfigurationPtr Aad::getDecoderConf(bool bShow)
{
	cout << "Call Aad::getDecoderConf()." << endl;

	NeAACDecConfigurationPtr pDecoderConf = NULL;
	pDecoderConf = NeAACDecGetCurrentConfiguration(hDecoder);
	if(NULL == pDecoderConf)
	{
		cerr << "Fail to call NeAACDecGetCurrentConfiguration()." << endl;
		return NULL;
	}

	if(bShow)
	{
		cout << "pDecoderConf->defObjectType = " << pDecoderConf->defObjectType << endl;
		cout << "pDecoderConf->defObjectType = " << pDecoderConf->defSampleRate << endl;
		cout << "pDecoderConf->outputFormat = " << pDecoderConf->outputFormat << endl;
		cout << "pDecoderConf->downMatrix = " << pDecoderConf->downMatrix << endl;
		cout << "pDecoderConf->useOldADTSFormat = " << pDecoderConf->useOldADTSFormat << endl;
		cout << "pDecoderConf->dontUpSampleImplicitSBR = " << pDecoderConf->dontUpSampleImplicitSBR << endl;
	}

	cout << "Call Aad::getDecoderConf() end." << endl;
	return pDecoderConf;
}

/*-----------------------------------------------------------------------------
描--述：设置解码器配置信息。
参--数：pDecoderConf 指向解码器参数结构体的指针
返回值：成功，返回0；失败，返回-1.
注--意：
-----------------------------------------------------------------------------*/
unsigned char Aad::setDecoderConf(NeAACDecConfigurationPtr pDecoderConf)
{
	cout << "Call Aad::setDecoderConf()." << endl;

	if(NULL == pDecoderConf)
	{
		cerr << "Call Aad::setDecoderConf() with null argument. Use default argument!" << endl;
		pDecoderConf = getDecoderConf(false);
	}

	if(NULL == pDecoderConf)
	{
		cerr << "Fail to call Aad::getDecoderConf()." << endl;
		return -1;
	}

	pDecoderConf->dontUpSampleImplicitSBR = 1;
	pDecoderConf->defSampleRate = sampleRate;
	pDecoderConf->outputFormat = FAAD_FMT_16BIT;

	unsigned char ret = 0;
	ret = NeAACDecSetConfiguration(hDecoder, pDecoderConf);
	if(1 != ret)
	{
		cerr << "Fail to call NeAACDecSetConfiguration()." << endl;
	}
	
	cout << "Call Aad::setDecoderConf() end." << endl;
	return 0;
}

/*-----------------------------------------------------------------------------
描--述：初始化解码器。
参--数：buf 待解码的音频数据缓冲区；bufSize 缓冲区长度；
返回值：成功，返回0；失败返回-1.
注--意：1.NeAACDecInit() 返回值的含义没搞懂。
		2.解码器只需要调用initDecoder() 初始化一次即可。
-----------------------------------------------------------------------------*/
long Aad::initDecoder(unsigned char *buf, unsigned long bufSize)
{
	cout << "Call Aad::initDecoder()." << endl;

	if(NULL == buf || 0 == bufSize)
	{
		cerr << "Fail to call Aad::initDecoder() with null argument! " << endl;
		return -1;
	}
	
	long ret = 0;
	ret = NeAACDecInit(hDecoder, buf, bufSize, &sampleRate, &channels);
	if(-1 == ret)
	{
		cerr << "Fail to call NeAACDecInit()." << endl;
		return -1;
	}
	cout << "sampleRate, channels = " << sampleRate << ", " << channels << endl;
	bInitDecoder = true;

	cout << "Call Aad::initDecoder() end." << endl;
	return ret;
}

/*-----------------------------------------------------------------------------
描--述：使能解码器，开始解码音频帧。
参--数：decFrameInfo 解码后的信息；buf 待解码数据的缓冲区；bufSize 缓冲区长度。
返回值：成功，返回解码后的数据的缓冲区；失败，返回NULL.
注--意：NeAACDecDecode() 返回的解码数据缓冲区，位于FAAD 内核，用户可以直接使用返回值。
-----------------------------------------------------------------------------*/
void* Aad::decDecode(NeAACDecFrameInfo *decFrameInfo, unsigned char *buf, unsigned long bufSize)
{
	//cout << "Call Aad::enDecoder()." << endl;
	if(NULL == decFrameInfo || NULL == buf || 0 == bufSize)
	{
		cerr << "Fail to call Aad::enDecoder(). Argument has null or zero value!" << endl;
		return NULL;
	}
	
	if(!bInitDecoder)
	{
		initDecoder(buf, bufSize);
	}

	if(!bInitDecoder)
	{
		cerr << "Fail to call Aad::enDecoder(). Decoder is not initialized!" << endl;
	}

	void *pcmBuf = NULL;	
	pcmBuf = NeAACDecDecode(hDecoder, decFrameInfo, buf, bufSize);

	//cout << "Call Aad::enDecoder() end." << endl;
	return pcmBuf;
}

/*-----------------------------------------------------------------------------
描--述：立体声数据转为单声道数据。
参--数：stereo16bitPcmBuf 立体声数据缓冲区；bufLen 缓冲区长度。
		leftChanelBuf 左声道数据缓冲区，设为NULL 表示不关心。
		rightChanelBuf 右声道数据缓冲区，设为NULL 表示不关心。
返回值：成功，返回0；失败，返回-1.
注--意：16bit的pcm每四个字节记录一帧，左低，左高，右低，右高。
-----------------------------------------------------------------------------*/
int Aad::stereo2mono(const unsigned char *stereo16bitPcmBuf, unsigned int bufLen, \
							unsigned char *leftChanelBuf, unsigned char *rightChanelBuf)
{
	if(NULL == stereo16bitPcmBuf || 0 == bufLen)
	{
		cerr << "Fail to call Aad::stereo2mono(). Argument has null or zero value!" << endl;
		return -1;
	}

	int i = 0;
	int j = 0;
	for(i = 0, j = 0; i < bufLen; j += 2, i += 4)
	{
		if(NULL != leftChanelBuf)
		{
			leftChanelBuf[j] = stereo16bitPcmBuf[i];
			leftChanelBuf[j + 1] = stereo16bitPcmBuf[i + 1];
		}
		if(NULL != rightChanelBuf)
		{
			rightChanelBuf[j] = stereo16bitPcmBuf[i + 2];
			rightChanelBuf[j + 1] = stereo16bitPcmBuf[i + 3];
		}
	}

	return 0;
}

