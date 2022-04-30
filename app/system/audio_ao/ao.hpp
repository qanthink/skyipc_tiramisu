/*---------------------------------------------------------------- 
xxx版权所有。
作者：
时间：2020.7.10
----------------------------------------------------------------*/

#pragma once

#include "mi_ao.h"

class AudioOut{
public:
	static AudioOut *getInstance();			// 获取对象。
	int sendStream(void *pDataBuf, const unsigned int dataLen);		// 送PCM 数据到AO 模块。
	
	int enable();
	int disable();

	int enableDev();			// 使能设备
	int disableDev();			// 禁用设备
	
	int enableChanel();			// 使能通道
	int disableChanel();		// 禁用通道

	int setPubAttr();			// 设置公共属性
	
	int setVolume(int volumeDb);	// 设置音量

private:
	// 单例模式需要将如下4个函数声明为private 的。
	AudioOut();
	~AudioOut();
	AudioOut(const AudioOut&);
	AudioOut& operator=(const AudioOut&);
	
	bool bInitialized = false;
	
	const MI_AUDIO_DEV audioDev = 0;	// AO 设备号
	const MI_AO_CHN audioChn = 0;		// AO 通道号
	const MI_AUDIO_BitWidth_e eBitWidth = E_MI_AUDIO_BIT_WIDTH_16;		// 位宽
	const MI_AUDIO_SampleRate_e eSample = E_MI_AUDIO_SAMPLE_RATE_16000;	// 采样率
	const MI_AUDIO_SoundMode_e eSoundmode = E_MI_AUDIO_SOUND_MODE_MONO;	// 单声道和立体声。
	const unsigned int u32PtNumPerFrm = 128 * 1;	// 每帧的采样点个数。取值范围为：128, 128*2,…,128*n.
	const int defVol = 1;				// 默认音量。
};

