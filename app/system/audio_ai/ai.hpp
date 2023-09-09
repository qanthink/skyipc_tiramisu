/*---------------------------------------------------------------- 
xxx版权所有。
作者：
时间：2022.4.20
----------------------------------------------------------------*/

#pragma once

#include "mi_ai.h"

class AudioIn{
public:
	static AudioIn *getInstance();

private:
	AudioIn();
	~AudioIn();
	AudioIn(const AudioIn&);
	AudioIn& operator=(const AudioIn&);

public:
	int enable();
	int disable();	

	int setMute(bool bMute);
	int setVolume(int volumeDb);		// 设置音量

	int getFrame(MI_AUDIO_Frame_t *pstFrm);
	int releaseFrame(MI_AUDIO_Frame_t *pstFrm);
	
	int enableDev();					// 启用AI 设备
	int disableDev();					// 禁用AI 设备
	
	int enableChanel();					// 启用AI 通道
	int disableChanel();				// 禁用AI 通道
	
	int enableAed();					// 启用AED 音频事件监测
	int disableAed();					// 禁用AED 音频事件监测

	int setPubAttr();					// 设置公有属性
	int setChnOutputPortDepth(MI_U32 u32UserFrameDepth, MI_U32 u32BufQueueDepth);	// 设置输出端口队列深度
	
public:
	/* AI 初始化相关参数 */
	const MI_AUDIO_DEV audioDev = 0;	// AI 设备号
	const MI_AO_CHN audioChn = 0;		// AI 通道号
	const MI_AUDIO_BitWidth_e eBitWidth = E_MI_AUDIO_BIT_WIDTH_16;		// 位宽
	const MI_AUDIO_SampleRate_e eSample = E_MI_AUDIO_SAMPLE_RATE_16000;	// 采样率
	const MI_AUDIO_SoundMode_e eSoundmode = E_MI_AUDIO_SOUND_MODE_STEREO;	// 单声道和立体声。

	const int volume = 21;							// 默认音量
};

