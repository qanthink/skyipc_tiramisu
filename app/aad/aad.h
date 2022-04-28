/*---------------------------------------------------------------- 
xxx版权所有。
作者：
时间：2020.7.10
----------------------------------------------------------------*/

/*
免责声明：
本程序基于faad 开源代码进行开发，请遵守faad 开源规则。
*/

#pragma once

#include "faad.h"

/*
参考网站：
https://www.jianshu.com/p/dd23c4132eb7
*/

/*
常用函数：
创建解码器：
NeAACDecHandle NeAACDecOpen(void);
创建成功则返回解码器句柄，创建失败则返回0

关闭解码器
void NeAACDecClose(NeAACDecHandle hDecoder);
参数为解码器的句柄。

配置解码器
NeAACDecConfigurationPtr NeAACDecGetCurrentConfiguration(NeAACDecHandle hDecoder);
输入的参数为第一步创建的解码器的句柄，返回为解码器配置参数的结构体的指针。获取失败则会返回为空。
参考配置：
conf->defObjectType = LC;
conf->defSampleRate = sampleRate;
conf->outputFormat = FAAD_FMT_16BIT;
conf->dontUpSampleImplicitSBR = 1;
参数设置非常简单，第一个为类型；第二个为采样率，第三个为保存音频的位数；第四个为SBR标志。
参数在头文件有详细说明，可参照头文件及实际情况进行设置。

配置生效
unsigned char NeAACDecSetConfiguration(NeAACDecHandle hDecoder,
                                                   NeAACDecConfigurationPtr config);
第一个参数为解码器句柄，第二个为我们的的配置结构体的指针对象。
返回值为0时代表配置失败。

解码AAC数据
long NeAACDecInit(NeAACDecHandle hDecoder,    //解码器句柄
                              unsigned char *buffer,        //输入数据的指针
                              unsigned long buffer_size,    //  输入数据的长度
                              unsigned long *samplerate,    //保存从aac数据中获取的编码的采样率
                              unsigned char *channels);    //保存从aac数据中获取的编码的通道数
返回非0则代表成功。此方法只需要在解码数据前调用一次，无需重复调用。其中输入的数据最好多余一个aac数据包，以便正确解析数据。

解码函数
void* NeAACDecDecode(NeAACDecHandle hDecoder,      //解码器句柄
                                 NeAACDecFrameInfo *hInfo,      //解码frame的信息
                                 unsigned char *buffer,         //需要解码的数据指针
                                 unsigned long buffer_size);    //需要解码的数据的长度

每次填入的解码数据为一个ADTS帧，ADTS帧的同步数据为0xfff，我们以此来分割AAC数据进行解码。
解码后我们需要获取解码是否成功，此时我们通过第二个参数的error属性进行判断，若error大于0则代表解码失败。
解码成功则读取解码后的PCM数据，我们可以根据第二个参数返回的 unsigned long samples; unsigned char channels;
两个属性进行手动计算PCM数据的大小，然后从返回指针的起始地址读取相应的字节数，这时候此段内存中保存的数据即为解码的PCM数据。
我们循环输入每一帧ADTS数据，得到解码后的PCM数据，直到所有数据解码完毕。获取的PCM数据可以根据用户自己的用途进行保存或者播放。
*/

class Aad{
public:
	static Aad *getInstance();
	
	int enable();
	int disable();

	int getFaadVersion();
	void* decDecode(NeAACDecFrameInfo *decFrameInfo, unsigned char *buf, unsigned long bufSize);
	int stereo2mono(const unsigned char *stereo16bitPcmBuf, unsigned int bufLen, \
						unsigned char *leftChanelBuf, unsigned char *rightChanelBuf);
	
private:
	Aad();
	~Aad();
	Aad(const Aad&);
	Aad& operator=(const Aad&);

	bool bEnable = false;
	bool bInitDecoder = false;
	unsigned char channels = 0;			// 会被NeAACDecInit() 修改
	NeAACDecHandle hDecoder = 0;		// 解码器句柄
	unsigned long sampleRate = 16000;	// 采样率，
	
	long initDecoder(unsigned char *buf, unsigned long bufSize);
	NeAACDecHandle openDecoder();
	void closeDecoder();
	
	NeAACDecConfigurationPtr getDecoderConf(bool bShow);
	unsigned char setDecoderConf(NeAACDecConfigurationPtr pDecoderConf);
};

