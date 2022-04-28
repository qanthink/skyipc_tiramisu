/*---------------------------------------------------------------- 
xxx版权所有。
作者：
时间：2020.7.10
----------------------------------------------------------------*/

#pragma once

#include "mi_ao.h"

class audioOut{
public:
	static audioOut *getInstance();
	
	int enable();
	int disable();

	int enableDev();
	int disableDev();
	
	int enableChanel();
	int disableChanel();

	int setPubAttr(MI_AUDIO_BitWidth_e eBitWidth = E_MI_AUDIO_BIT_WIDTH_16, 
							MI_AUDIO_SampleRate_e eSample = E_MI_AUDIO_SAMPLE_RATE_16000);

	int setVolume(int val);
	int sendStream(MI_AUDIO_Frame_t *pstAudioFrame);

public:
	const static unsigned int frameMaxSize = 4 * 1024;		// n * 1024 = n KB.

private:
	// 单例模式需要将如下4个函数声明为private 的。
	audioOut();
	~audioOut();
	audioOut(const audioOut&);
	audioOut& operator=(const audioOut&);
	
	bool bInitialized = false;				// stream 线程的运行状态。
	const static MI_AUDIO_DEV audioDev = 0;	// AO 设备号
	const static MI_AO_CHN audioChn = 0;	// 使能设备下的chanel
	const static unsigned u32ChnCnt = 1;	// 声道数。1=单声道；2=立体声。
	const static unsigned u32PtNumPerFrm = 128 * 4;		// 每帧的采样点个数。取值范围为：128, 128*2,…,128*
};

