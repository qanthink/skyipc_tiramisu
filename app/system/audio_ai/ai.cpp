/*---------------------------------------------------------------- 
xxx版权所有。
作者：
时间：2020.7.10
----------------------------------------------------------------*/

#include "mi_sys.h"
#include "ai.hpp"

#include <string.h>
#include <iostream>

using namespace std;

AudioIn* AudioIn::getInstance()
{
	static AudioIn audioIn;
	return &audioIn;
};

AudioIn::AudioIn()
{
	enable();
}

AudioIn::~AudioIn()
{
	disable();
}

int AudioIn::enable()
{
	setPubAttr();
	enableDev();
	enableChanel();
	setChnOutputPortDepth(1, 3);
	#if (1 == (USE_AED))
	enableAed();
	#endif
	setVolume(volume);

	return 0;
}

int AudioIn::disable()
{
	disableAed();
	disableChanel();
	disableDev();

	return 0;
}

/*
eSamplerate	音频采样率。
静态属性。
eBitwidth	音频采样精度（从模式下，此参数必须和音频AD/DA 的采样精度匹配）。
静态属性。
eWorkmode	音频输入输出工作模式。
静态属性。
eSoundmode	音频声道模式。
静态属性。
u32FrmNum	缓存帧数目。
保留，未使用。
u32PtNumPerFrm	每帧的采样点个数。
取值范围为：128, 128*2,…,128*N。
静态属性。
u32CodecChnCnt	支持的codec通道数目。
保留，未使用。
u32ChnCnt	支持的通道数目，实际可使能的最大通道数。取值：1、2、4、8、16。
			（输入最多支持MI_AUDIO_MAX_CHN_NUM 个通道，输出最多支持2 个通道）
MI_AUDIO_I2sConfig_t stI2sConfig;	设置I2S 工作属性
*/

/*-----------------------------------------------------------------------------
描--述：设置AI 公有属性。
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
int AudioIn::setPubAttr()
{
	MI_AUDIO_Attr_t stAiAttr;
	memset(&stAiAttr, 0, sizeof(MI_AUDIO_Attr_t));

	stAiAttr.u32ChnCnt = 1;
	stAiAttr.eBitwidth = eBitWidth;
	stAiAttr.eSamplerate = eSample;
	stAiAttr.eSoundmode = eSoundmode;
	stAiAttr.u32PtNumPerFrm = u32PtNumPerFrm;				// demo 和文档不符
	stAiAttr.eWorkmode = E_MI_AUDIO_MODE_I2S_MASTER;		// demo 和文档不符
	//stAiAttr.eWorkmode = E_MI_AUDIO_MODE_I2S_SLAVE;		// MI_API DOC
	//stAiAttr.u32PtNumPerFrm = stAiAttr.eSamplerate / 16;	// for aec
		
	//stAiAttr.WorkModeSetting.stI2sConfig.eFmt = E_MI_AUDIO_I2S_FMT_I2S_MSB;
	//stAiAttr.WorkModeSetting.stI2sConfig.eMclk = E_MI_AUDIO_I2S_MCLK_0;

	// MI_S32 MI_AI_SetPubAttr(MI_AUDIO_DEV AIDevId, MI_AUDIO_Attr_t *pstAiAttr);
	MI_S32 s32Ret = 0;
	s32Ret = MI_AI_SetPubAttr(audioDev, &stAiAttr);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_AI_SetPubAttr(). s32Ret = 0x" << hex << s32Ret << endl;
		return s32Ret;
	}
	cout << "Success to call MI_AI_SetPubAttr()." << endl;
	
	return s32Ret;
}

/*-----------------------------------------------------------------------------
描--述：设置AI 通道输出端口的队列深度。
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
int AudioIn::setChnOutputPortDepth(MI_U32 u32UserFrameDepth, MI_U32 u32BufQueueDepth)
{
	MI_SYS_ChnPort_t stChnOutputPort;
	memset(&stChnOutputPort, 0, sizeof(MI_SYS_ChnPort_t));

	stChnOutputPort.eModId = E_MI_MODULE_ID_AI;
	stChnOutputPort.u32DevId = audioDev;
	stChnOutputPort.u32ChnId = audioChn;
	stChnOutputPort.u32PortId = 0;

	MI_S32 s32Ret = 0;
	MI_U16 u16SocId = 0;		// default ID = 0; demo.
	s32Ret = MI_SYS_SetChnOutputPortDepth(u16SocId, &stChnOutputPort, u32UserFrameDepth, u32BufQueueDepth);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_SYS_SetChnOutputPortDepth(). s32Ret = 0x" << hex << s32Ret << endl;
		return s32Ret;
	}

	cout << "Success to call AudioIn::setChnOutputPortDepth()." << endl;
	return s32Ret;
}


/*-----------------------------------------------------------------------------
描--述：使能AI 设备
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
int AudioIn::enableDev()
{	
	// MI_S32 MI_AI_Enable(MI_AUDIO_DEV AIDevId);
	MI_S32 s32Ret = 0;
	s32Ret = MI_AI_Enable(audioDev);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_AI_Enable(). s32Ret = 0x" << hex << s32Ret << endl;
	}
	cout << "Success to call MI_AI_Enable()." << endl;
	
	return s32Ret;
}

/*-----------------------------------------------------------------------------
描--述：禁用AI 设备
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
int AudioIn::disableDev()
{
	// MI_S32 MI_AI_Disable(MI_AUDIO_DEV AIDevId);
	MI_S32 s32Ret = 0;
	s32Ret = MI_AI_Disable(audioDev);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_AI_Disable(). s32Ret = 0x" << hex << s32Ret << endl;
	}
	cout << "Success to call MI_AI_Disable()." << endl;

	return s32Ret;
}


/*-----------------------------------------------------------------------------
描--述：使能AI 设备下的chanel
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
int AudioIn::enableChanel()
{
	// MI_S32 MI_AI_EnableChn(MI_AUDIO_DEV AIDevId, MI_AI_CHN AIChn);
	MI_S32 s32Ret = 0;
	s32Ret = MI_AI_EnableChn(audioDev, audioChn);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_AI_EnableChn(). s32Ret = 0x" << hex << s32Ret << endl;
	}
	cout << "Success to call MI_AI_EnableChn()." << endl;

	return s32Ret;
}

/*-----------------------------------------------------------------------------
描--述：禁用AI 设备下的chanel
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
int AudioIn::disableChanel()
{
	// MI_S32 MI_AI_EnableChn(MI_AUDIO_DEV AIDevId, MI_AI_CHN AIChn);
	MI_S32 s32Ret = 0;
	s32Ret = MI_AI_DisableChn(audioDev, audioChn);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_AI_DisableChn(). s32Ret = 0x" << hex << s32Ret << endl;
	}
	cout << "Success to call MI_AI_DisableChn()." << endl;

	return s32Ret;
}

/*
bEnableNr			是否启用婴儿哭声检测的降噪功能
eSensitivity		婴儿哭声检测的灵敏度
s32OperatingPoint	婴儿哭声检测的操作点。范围[-10, 10]，步长为1, 默认值为0
					注：提高操作点将会降低误报率，减小操作点将会降低漏测率
s32VadThresholdDb	婴儿哭声检测的Vad门槛值(dB), 范围[-80, 0]，步长为1, 默认值为-40
*/
/*-----------------------------------------------------------------------------
描--述：启用声音事件检测功能。
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
int AudioIn::enableAed()
{
	MI_AI_AedConfig_t stAedConfig;
	memset(&stAedConfig, 0, sizeof(MI_AI_AedConfig_t));

	#if 0
	stAedConfig.bEnableNr = TRUE;
	stAedConfig.eSensitivity = E_MI_AUDIO_AED_SEN_HIGH;
	stAedConfig.s32OperatingPoint = -5;
	stAedConfig.s32VadThresholdDb = -40;
	stAedConfig.s32LsdThresholdDb = -15;
	#endif
	
	// MI_S32 MI_AI_SetAedAttr(MI_AUDIO_DEV AiDevId, MI_AI_CHN AiChn, MI_AI_AedConfig_t *pstAedConfig);
	MI_S32 s32Ret = 0;
	s32Ret = MI_AI_SetAedAttr(audioDev, audioChn, &stAedConfig);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_AI_SetAedAttr(). s32Ret = 0x" << hex << s32Ret << endl;
		return s32Ret;
	}

	// MI_S32 MI_AI_EnableAed(MI_AUDIO_DEV AiDevId, MI_AI_CHN AiChn);
	s32Ret = MI_AI_EnableAed(audioDev, audioChn);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_AI_EnableAed(). s32Ret = 0x" << hex << s32Ret << endl;
		return s32Ret;
	}

	return s32Ret;
}

/*-----------------------------------------------------------------------------
描--述：禁用声音事件检测功能。
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
int AudioIn::disableAed()
{
	// MI_S32 MI_AI_DisableAed(MI_AUDIO_DEV AiDevId, MI_AI_CHN AiChn);
	MI_S32 s32Ret = 0;
	s32Ret = MI_AI_DisableAed(audioDev, audioChn);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_AI_EnableAed(). s32Ret = 0x" << hex << s32Ret << endl;
		return s32Ret;
	}
	
	return s32Ret;
}

/*-----------------------------------------------------------------------------
描--述：获取一帧AI 数据。
参--数：
返回值：成功，返回0; 失败，返回错误码。
注--意：含AED 结果。
-----------------------------------------------------------------------------*/
int AudioIn::recvStream(stAIFrame_t *pstAIFrame)
{
	if(NULL == pstAIFrame)
	{
		cerr << "Fail to call AudioIn::recvStream(). Argument has null value." << endl;
		return -1;
	}

	MI_S32 s32Ret = 0;
	MI_AUDIO_Frame_t stAudioFrame;
	MI_AUDIO_AecFrame_t stAecFrame;
	memset(&stAudioFrame, 0, sizeof(MI_AUDIO_Frame_t));
	memset(&stAecFrame, 0, sizeof(MI_AUDIO_AecFrame_t));
	s32Ret = MI_AI_GetFrame(audioDev, audioChn, &stAudioFrame, &stAecFrame, -1);
	//s32Ret = MI_AI_GetFrame(audioDev, audioChn, &stAudioFrame, &stAecFrame, 100);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_AI_GetFrame(). s32Ret = 0x" << hex << s32Ret << endl;
		return s32Ret;
	}

	//cout << "stAudioFrame.u32Len = " << stAudioFrame.u32Len << endl;

	#if (1 == (USE_AED))
	// 获取AED声音事件检测结果。占用CPU 和FLASH 很高。
	MI_AI_AedResult_t stAedResult;
	memset(&stAedResult, 0, sizeof(stAedResult));
	s32Ret = MI_AI_GetAedResult(audioDev, audioChn, &stAedResult);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_AI_GetAedResult(). s32Ret = 0x" << hex << s32Ret << endl;
		return s32Ret;
	}
	#endif

	#if 0	// debug
	cout << "bAcousticEventDetected : bLoudSoundDetected = " << (int)stAedResult.bAcousticEventDetected << 
		" : " << (int)stAedResult.bLoudSoundDetected << endl;
	#endif

	// 拷贝数据到用户层。
	memset(pstAIFrame, 0, sizeof(stAIFrame_t));
	pstAIFrame->u64TimeStamp = stAudioFrame.u64TimeStamp;
	#if 0	// ispahan, pudding
	pstAIFrame->u32Len = stAudioFrame.u32SrcPcmLen;
	memcpy(pstAIFrame->apFrameBuf, stAudioFrame.apSrcPcmVirAddr[0], stAudioFrame.u32SrcPcmLen);
	#else	// tiramisu
	pstAIFrame->u32Len = stAudioFrame.u32SrcPcmLen[0];	
	memcpy(pstAIFrame->apFrameBuf, stAudioFrame.apSrcPcmVirAddr[0], stAudioFrame.u32SrcPcmLen[0]);
	#endif

	#if (1 == (USE_AED))
	pstAIFrame->bAcousticEventDetected = stAedResult.bAcousticEventDetected;
	pstAIFrame->bLoudSoundDetected = stAedResult.bLoudSoundDetected;
	#endif

	s32Ret = MI_AI_ReleaseFrame(audioDev, audioChn, &stAudioFrame, &stAecFrame);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_AI_ReleaseFrame(). s32Ret = 0x" << hex << s32Ret << endl;
		return s32Ret;
	}

	return 0;
}

/*-----------------------------------------------------------------------------
描--述：设置音量。
参--数：volumeDb, 音量值，取值范围[0, 21].
		针对Amic, 映射的增益值为[-6, 57], volumeDb 有效值[0, 21];
		针对Line in, 映射的增益值为[-6, 15], volumeDb 有效值[0, 7];
		针对DMIC, 映射的增益值为[0-24], volumeDb 有效值[0, 4];
返回值：
注--意：
-----------------------------------------------------------------------------*/
int AudioIn::setVolume(int volumeDb)
{
	MI_S32 s32Ret = 0;
	s32Ret = MI_AI_SetVqeVolume(audioDev, audioChn, volumeDb);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_AI_SetVqeVolume(). s32Ret = 0x" << hex << s32Ret << endl;
	}

	return s32Ret;
}

