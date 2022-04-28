/*---------------------------------------------------------------- 
xxx版权所有。
作者：
时间：2020.7.10
----------------------------------------------------------------*/

/*
免责声明：
本程序基于faac 开源代码进行开发，请遵守faac 开源规则。
*/


#include "aac.h"
#include <iostream>

using namespace std;

Aac* Aac::getInstance()
{
	static Aac aac;
	return &aac;
}

Aac::Aac()
{
	hEncoder = NULL;
	bEnable = false;
	
	enable();
}

Aac::~Aac()
{
	disable();

	bEnable = false;
	hEncoder = NULL;
}

/*-----------------------------------------------------------------------------
描--述：
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
int Aac::enable()
{
	cout << "Call Aac::enable()." << endl;

	if(bEnable)
	{
		return 0;
	}

	getFaacVersion();
	openEncoder();
	setEncoderConf(NULL);
	cout << "inputSamples, maxOutputBytes = " << inputSamples << "," << maxOutputBytes << endl;

	bEnable = true;

	cout << "Call Aac::enable() end." << endl;
	return 0;
}

/*-----------------------------------------------------------------------------
描--述：
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
int Aac::disable()
{
	cout << "Call Aac::disable()." << endl;

	bEnable = false;
	
	closeEncoder();
	
	cout << "Call Aac::disable() end." << endl;
	return 0;
}

/*-----------------------------------------------------------------------------
描--述：打开编码器。
参--数：sampleRate， 编码输入信息的采样率；numChannels 编码输入信息的通道数量, 1-单声道 2-立体声；
		inputSamples, 编码后的数据长度；maxOutputBytes, 编码后的信息最大长度。
返回值：成功，返回编码器描述符；失败，返回NULL.
注--意：
-----------------------------------------------------------------------------*/
faacEncHandle Aac::openEncoder()
{
	cout << "Call Aac::openEncoder()." << endl;

	hEncoder = faacEncOpen(sampleRate, numChannels, &inputSamples, &maxOutputBytes);
	if(NULL == hEncoder)
	{
		cerr << "Fail to call faacEncOpen()." << endl;
	}

	cout << "Call Aac::openEncoder() end." << endl;
	return hEncoder;
}

/*-----------------------------------------------------------------------------
描--述：关闭编码器
参--数：hEncoder, 编码器描述符。
返回值：失败返回非0值。
注--意：
-----------------------------------------------------------------------------*/
int Aac::closeEncoder()
{
	cout << "Call Aac::closeEncoder()." << endl;

	int ret = 0;
	if(NULL != hEncoder)
	{
		ret = faacEncClose(hEncoder);
		hEncoder = NULL;
	}

	cout << "Call Aac::closeEncoder() end." << endl;
	return ret;
}

/*-----------------------------------------------------------------------------
描--述：获取编码器配置信息
参--数：hEncoder, 编码器描述符。
返回值：成功，返回编码器配置信息的结构体指针；失败，返回NULL.
注--意：
-----------------------------------------------------------------------------*/
faacEncConfigurationPtr Aac::getEncoderConf()
{
	cout << "Call Aac::getEncoderConf()." << endl;

	if(NULL == hEncoder)
	{
		cerr << "Fail to call Aac::getEncoderConf(). hEncoder has null value!" << endl;
		return NULL;
	}

	faacEncConfigurationPtr encoderConf = NULL;
	encoderConf = faacEncGetCurrentConfiguration(hEncoder);

	cout << "Call Aac::getEncoderConf() end." << endl;
	return encoderConf;
}

/*-----------------------------------------------------------------------------
描--述：设置编码器配置信息。
参--数：hEncoder, 编码器描述符；encoderConf, 编码配置信息。
返回值：成功，返回1; 失败返回0.
注--意：
-----------------------------------------------------------------------------*/
int Aac::setEncoderConf(faacEncConfigurationPtr pEncoderConf)
{
	cout << "Call Aac::setEncoderConf()." << endl;

	if(NULL == pEncoderConf)
	{
		cerr << "Call Aac::setEncoderConf() with null value! Use default argument!" << endl;

		pEncoderConf = getEncoderConf();
		/* FAAC_INPUT_16BIT, FAAC_INPUT_24BIT, FAAC_INPUT_32BIT, FAAC_INPUT_FLOAT.*/
		pEncoderConf->inputFormat = FAAC_INPUT_16BIT;
		#if 0
		pEncoderConf->outputFormat = 1;		// 0 = Raw; 1 = ADTS
		pEncoderConf->aacObjectType = LOW;	/* AAC object type */ // MAIN 1, LOW  2, SSR  3, LTP  4	//LC编码
		pEncoderConf->mpegVersion  = MPEG4;
		pEncoderConf->useTns = 1;			//时域噪音控制,大概就是消爆音
		pEncoderConf->allowMidside = 0;		/* Allow mid/side coding */
		pEncoderConf->useLfe = 0;			/* Use one of the channels as LFE channel */
		pEncoderConf->bitRate = 16000;		/* bitrate / channel of AAC file */
		pEncoderConf->bandWidth = 32000;	/* AAC file frequency bandwidth */ //频宽
		pEncoderConf->bitRate = pEncoderConf->bitRate / 1;
		pEncoderConf->outputFormat = false;	//输出是否包含ADTS头
		pEncoderConf->quantqual = 50 ;
		#endif
	}

	int ret = 0;
	ret = faacEncSetConfiguration(hEncoder, pEncoderConf);
	if(1 != ret)
	{
		cerr << "Fail to call faacEncSetConfiguration()." << endl;
	}
	
	cout << "Call Aac::setEncoderConf() end." << endl;
	return ret;
}

/*-----------------------------------------------------------------------------
描--述：开始编码，对一帧数据进行编码。
参--数：hEncoder, 编码设备描述符；inputBuf, 输入数据的缓存区；samplesInput, 输入数据的采样率；
		outputBuf, 输出数据的缓存区；bufSize, 输出数据缓存区的大小。
返回值：成功，返回编码后的字节数（描述得比较模糊，详细内容需要参考源码）；
		失败，返回-1, 非法的输入参数;
注--意：
-----------------------------------------------------------------------------*/
int Aac::encEncode(int32_t *inputBuf, unsigned int samplesInput, unsigned char *outputBuf, unsigned int bufSize)
{
	//cout << "Call Aac::encEncode()." << endl;

	if(NULL == hEncoder || NULL == inputBuf || NULL == outputBuf)
	{
		cerr << "Fail to call Aac::encEncode(). Argument has null value!" << endl;
		return -1;
	}

	int ret = 0;
	ret = faacEncEncode(hEncoder, inputBuf, samplesInput, outputBuf, bufSize);

	//cout << "Call Aac::encEncode() end." << endl;
	return ret;
}

/*-----------------------------------------------------------------------------
描--述：返回faac 的版本信息。
参--数：faacIdString, 版本号；faacCopyrightString, 版权字符串。
返回值：返回FAAC_CFG_VERSION.
注--意：
-----------------------------------------------------------------------------*/
int Aac::getFaacVersion()
{
	cout << "Call Aac::getFaacVersion()." << endl;

	char *verionID = NULL;
	char *copyright = NULL;

	faacEncGetVersion(&verionID, &copyright);
	cout << "FAAC VersionID: " << verionID << endl;
	cout << "FAAC copyright: " << endl <<  copyright << endl;
	
	
	cout << "Call Aac::getFaacVersion() end." << endl;
	return 0;
}

