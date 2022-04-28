/*---------------------------------------------------------------- 
xxx版权所有。
作者：
时间：2020.7.10
----------------------------------------------------------------*/

#include "ao.hpp"
#include "string.h"
#include "iostream"
#include "errno.h"

using namespace std;

audioOut::audioOut()
{
	enable();
}

audioOut::~audioOut()
{
	disable();
}

audioOut* audioOut::getInstance()
{
	static audioOut audioOut;
	return &audioOut;
};


int audioOut::enable()
{
	if(bInitialized)
	{
		cerr << "AO has been initialized. Don't repeat initialization." << endl;
		return -1;
	}

	setPubAttr();
	enableDev();
	enableChanel();
	setVolume(28);
	bInitialized = true;

	return 0;
}

int audioOut::disable()
{
	disableChanel();
	disableDev();
	bInitialized = false;

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
描--述：设置AO 公有属性。
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
int audioOut::setPubAttr(MI_AUDIO_BitWidth_e eBitWidth, MI_AUDIO_SampleRate_e eSample)
{
	MI_AUDIO_Attr_t stAttr;
	memset(&stAttr, 0, sizeof(MI_AUDIO_Attr_t));

	stAttr.eWorkmode = E_MI_AUDIO_MODE_I2S_MASTER;
	stAttr.WorkModeSetting.stI2sConfig.bSyncClock = FALSE;
	stAttr.WorkModeSetting.stI2sConfig.eFmt = E_MI_AUDIO_I2S_FMT_I2S_MSB;
	stAttr.WorkModeSetting.stI2sConfig.eMclk = E_MI_AUDIO_I2S_MCLK_0;
	stAttr.eBitwidth = eBitWidth;
	stAttr.u32PtNumPerFrm = u32PtNumPerFrm;
	stAttr.u32ChnCnt = u32ChnCnt;
	stAttr.eSamplerate = eSample;
	if(1 == u32ChnCnt)
	{
		stAttr.eSoundmode = E_MI_AUDIO_SOUND_MODE_MONO;
	}
	else if(2 == u32ChnCnt)
	{
		stAttr.eSoundmode = E_MI_AUDIO_SOUND_MODE_STEREO;
	}

	// MI_S32 MI_AO_SetPubAttr(MI_AUDIO_DEV AoDevId, MI_AUDIO_Attr_t *pstAttr);
	MI_S32 s32Ret = 0;
	s32Ret = MI_AO_SetPubAttr(audioDev, &stAttr);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_AO_SetPubAttr(). s32Ret = 0x" << hex << s32Ret << endl;
	}
	cout << "Success to call MI_AO_SetPubAttr()." << endl;
	
	return s32Ret;
}

/*-----------------------------------------------------------------------------
描--述：使能AO 设备
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
int audioOut::enableDev()
{	
	// MI_S32 MI_AO_Enable(MI_AUDIO_DEV AoDevId);
	MI_S32 s32Ret = 0;
	s32Ret = MI_AO_Enable(audioDev);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_AO_Enable(). s32Ret = 0x" << hex << s32Ret << endl;
	}
	cout << "Success to call MI_AO_Enable()." << endl;
	
	return s32Ret;
}

/*-----------------------------------------------------------------------------
描--述：禁用AO 设备
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
int audioOut::disableDev()
{
	// MI_S32 MI_AO_Disable(MI_AUDIO_DEV AoDevId);
	MI_S32 s32Ret = 0;
	s32Ret = MI_AO_Disable(audioDev);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_AO_Disable(). s32Ret = 0x" << hex << s32Ret << endl;
	}
	cout << "Success to call MI_AO_Disable()." << endl;

	return s32Ret;
}


/*-----------------------------------------------------------------------------
描--述：使能AO 设备下的chanel
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
int audioOut::enableChanel()
{
	// MI_S32 MI_AO_EnableChn(MI_AUDIO_DEV AoDevId, MI_AO_CHN AoChn);
	MI_S32 s32Ret = 0;
	s32Ret = MI_AO_EnableChn(audioDev, audioChn);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_AO_EnableChn(). s32Ret = 0x" << hex << s32Ret << endl;
	}
	cout << "Success to call MI_AO_EnableChn()." << endl;

	return s32Ret;
}

/*-----------------------------------------------------------------------------
描--述：禁用AO 设备下的chanel
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
int audioOut::disableChanel()
{
	// MI_S32 MI_AO_EnableChn(MI_AUDIO_DEV AoDevId, MI_AO_CHN AoChn);
	MI_S32 s32Ret = 0;
	s32Ret = MI_AO_DisableChn(audioDev, audioChn);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_AO_DisableChn(). s32Ret = 0x" << hex << s32Ret << endl;
	}
	cout << "Success to call MI_AO_DisableChn()." << endl;

	return s32Ret;
}

/*-----------------------------------------------------------------------------
描--述：
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
int audioOut::sendStream(MI_AUDIO_Frame_t *pstAudioFrame)
{
	if(NULL == pstAudioFrame)
	{
		cerr << "Fail to call sendStream(), argument has NULL value!" << endl;
		return -1;
	}

	MI_S32 s32Ret = 0;
	s32Ret = MI_AO_SendFrame(audioDev, audioChn, pstAudioFrame, -1);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_AO_SendFrame() in audioOut::sendStream(). s32Ret = " << s32Ret << endl;
		return s32Ret;
	}

	return 0;
}

/*-----------------------------------------------------------------------------
描--述：设置音量
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
int audioOut::setVolume(int val)
{
	int valMin = -60;
	int valMax = 30;
	
	if(val < valMin || val > valMax)
	{
		cerr << "Fail to call setVolume(), bad argument!" << endl;
		cerr << "Volume range: [" << valMin << ", " << valMax << "]" << endl;
	}

	MI_S32 s32Ret = 0;
	MI_AO_CHN AoChn;
	MI_S32 s32VolumeDb;
	MI_AO_GainFading_e eFading;
	s32Ret = MI_AO_SetVolume(audioDev, AoChn, s32VolumeDb, eFading);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_AO_SetVolume()." << endl;
	}

	return s32Ret;
}

