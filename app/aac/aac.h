/*---------------------------------------------------------------- 
xxx版权所有。
作者：
时间：2020.7.10
----------------------------------------------------------------*/

/*
免责声明：
本程序基于faac 开源代码进行开发，请遵守faac 开源规则。
*/

#pragma once

#include "faac.h"

/*
参考网站：
https://www.jianshu.com/p/cd0e0269a005
*/

/*
主要的函数介绍：

函数：faacEncHandle faacEncOpen(unsigned longsampleRate, unsigned intnumChannels, \
									unsigned long*inputSamples, unsigned long*maxOutputBytes);
功能：打开并初始化编码器
参数：sampleRate, 编码输入信息的采样率; numChannels, 编码输入信息的通道数量，1-单声道 2-立体声
		inputSamples, 编码后的数据长度; maxOutputBytes, 编码后的信息最大长度

函数：int faacEncClose(faacEncHandle hEncoder);
功能：关闭编码器
参数：hEncoder, faacEncOpen返回的编码器句柄

函数：faacEncConfigurationPtr faacEncGetCurrentConfiguration(faacEncHandle hEncoder);
功能：获取当前编码器的配置信息
参数：hEncoder, faacEncOpen返回的编码器句柄

函数：int faacEncSetConfiguration(faacEncHandle hEncoder, faacEncConfigurationPtr config);
功能：配置解码器的参数
参数：hEncoder, faacEncOpen返回的编码器句柄; config, 编码器的配置信息

函数：int faacEncEncode(faacEncHandle hEncoder, int32_t * inputBuffer, 
						unsigned intsamplesInput, unsigned char*outputBuffer, unsigned intbufferSize);
功能：编码一桢信息
参数：hEncoder, faacEncOpen返回的编码器句柄; inputBuffer, 输入信息缓冲区;
	samplesInput, faacEncOpen编码后的数据长度，即缓冲区长度; outputBuffer, 编码后输出信息缓冲区
	bufferSize, 输出信息长度

函数：int faacEncGetVersion(char**faac_id_string, char**faac_copyright_string);
功能：获取FAAC的版本信息，用以参考作用，非必须API
参数：faac_id_string, faac的版本号; faac_copyright_string, 版权信息。
*/

/*
优化参考：https://blog.csdn.net/qinglongzhan/article/details/81315532
修改MAX_CHANNELS 的值。默认64, 修改为2.
*/

class Aac{
public:
	static Aac* getInstance();

	int enable();
	int disable();

	int getFaacVersion();
	int encEncode(int32_t *inputBuf, unsigned int samplesInput, unsigned char *outputBuf, unsigned int bufSize);
	
	unsigned long getInputSamples() const {return inputSamples;};
	unsigned long getMaxOutputBytes() const {return maxOutputBytes;};
private:
	Aac();
	~Aac();
	Aac(const Aac&);
	Aac& operator=(const Aac&);

	bool bEnable = false;
	faacEncHandle hEncoder = 0;						// 编码器句柄。
	unsigned long inputSamples = 0;					// 输入采样率，被openEncoder() 修改。
	unsigned long maxOutputBytes = 0;				// 最大输出字节，被openEncoder() 修改。
	const static unsigned int numChannels = 1;		// 声道数。
	const static unsigned long sampleRate = 16000;	// 采样率，
	
	faacEncConfigurationPtr getEncoderConf();
	int setEncoderConf(faacEncConfigurationPtr pEncoderConf);
	faacEncHandle openEncoder();
	int closeEncoder();
};

