/*---------------------------------------------------------------- 
xxx版权所有。
作者：
时间：2020.7.10
----------------------------------------------------------------*/

#pragma once

#include "mi_ai.h"

struct stAIFrame_t
{
	const static unsigned int uFrameBufSize = 2 * 1024; // N * 1024 = N KB
	char apFrameBuf[uFrameBufSize];
	MI_U32 u32Len;
	
	MI_U64 u64TimeStamp;
	MI_BOOL bLoudSoundDetected;
	MI_BOOL bAcousticEventDetected;
	MI_AUDIO_BitWidth_e eBitWidth;
	MI_AUDIO_SoundMode_e eSoundmode;
};

class AudioIn{
public:
	static AudioIn *getInstance();
	
	int enable();
	int disable();	

	int setVqeVolume(int val);			// 设置音量
	int recvStream(stAIFrame_t *pAudioFrame);		// 获取一帧音频数据
	
	int enableDev();					// 启用AI 设备
	int disableDev();					// 禁用AI 设备
	
	int enableChanel();					// 启用AI 通道
	int disableChanel();				// 禁用AI 通道
	
	int enableAed();					// 启用AED 音频事件监测
	int disableAed();					// 禁用AED 音频事件监测

	int setPubAttr(MI_AUDIO_BitWidth_e eBitWidth, MI_AUDIO_SampleRate_e eSample);		// 设置公有属性
	int setChnOutputPortDepth(MI_U32 u32UserFrameDepth = 1, MI_U32 u32BufQueueDepth = 4);	// 设置输出端口队列深度
	
private:
	// 单例模式需要将如下4个函数声明为private 的。
	AudioIn();
	~AudioIn();
	AudioIn(const AudioIn&);
	AudioIn& operator=(const AudioIn&);

	/* AI 初始化相关参数 */
	const MI_S32 s32DefVol = 19;		// 默认音量
	const MI_AI_CHN audioChn = 0;		// AI 通道
	const MI_AUDIO_DEV audioDev = 0;	// AI 设备号
	const MI_U32 u32ChnCnt = 1;			// 1 = 单声道, 2 = 立体声。
	const MI_U32 u32PtNumPerFrm = 160;	// 每一帧的采样点数，可以不必128 字对齐

	bool enAed = false;
	bool bRunning = false;					// 运行状态
	bool bFrameBufBusy = false;				// 音频帧缓冲区有数据
};

