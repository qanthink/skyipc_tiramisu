/*---------------------------------------------------------------- 
qanthink 版权所有。
作者：
时间：2020.7.10
----------------------------------------------------------------*/

#include "ao.hpp"
#include "string.h"
#include "iostream"

using namespace std;

AudioOut::AudioOut()
{
	enable();
}

AudioOut::~AudioOut()
{
	disable();
}

AudioOut* AudioOut::getInstance()
{
	static AudioOut AudioOut;
	return &AudioOut;
};


int AudioOut::enable()
{
	if(bInitialized)
	{
		cerr << "AO has been initialized. Don't repeat initialization." << endl;
		return -1;
	}

	setPubAttr();
	enableDev();
	setVolume(defVol);
	enableChanel();
	
	bInitialized = true;
	return 0;
}

int AudioOut::disable()
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
int AudioOut::setPubAttr()
{
	MI_AUDIO_Attr_t stAoAttr;
	memset(&stAoAttr, 0, sizeof(MI_AUDIO_Attr_t));

	stAoAttr.u32ChnCnt = 1;
	stAoAttr.eSamplerate = eSample;
	stAoAttr.eBitwidth = eBitWidth;
	stAoAttr.eSoundmode = eSoundmode;
	stAoAttr.u32PtNumPerFrm = u32PtNumPerFrm;
	stAoAttr.eWorkmode = E_MI_AUDIO_MODE_I2S_MASTER;
	stAoAttr.WorkModeSetting.stI2sConfig.u32TdmSlots = 0;		// 不懂。
	stAoAttr.WorkModeSetting.stI2sConfig.bSyncClock = FALSE;	// 不懂。
	stAoAttr.WorkModeSetting.stI2sConfig.eI2sBitWidth = eBitWidth;
	stAoAttr.WorkModeSetting.stI2sConfig.eMclk = E_MI_AUDIO_I2S_MCLK_0;
	stAoAttr.WorkModeSetting.stI2sConfig.eFmt = E_MI_AUDIO_I2S_FMT_I2S_MSB;

	// MI_S32 MI_AO_SetPubAttr(MI_AUDIO_DEV AoDevId, MI_AUDIO_Attr_t *pstAoAttr);
	MI_S32 s32Ret = 0;
	s32Ret = MI_AO_SetPubAttr(audioDev, &stAoAttr);
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
int AudioOut::enableDev()
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
int AudioOut::disableDev()
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
int AudioOut::enableChanel()
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
int AudioOut::disableChanel()
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
int AudioOut::sendStream(void *pDataBuf, const unsigned int dataLen)
{
	if(NULL == pDataBuf)
	{
		cerr << "Fail to call sendStream(), argument has NULL value!" << endl;
		return -1;
	}

	MI_S32 s32Ret = 0;
	MI_AUDIO_Frame_t stAudioFrame;
	memset(&stAudioFrame, 0, sizeof(MI_AUDIO_Frame_t));

	stAudioFrame.apVirAddr[0] = pDataBuf;
	stAudioFrame.u32Len[0] = dataLen;
	stAudioFrame.apSrcPcmVirAddr[0] = pDataBuf;
	//stAudioFrame.u32SrcPcmLen[0] = 2 * 1024;
	stAudioFrame.u32SrcPcmLen[0] = dataLen;
	stAudioFrame.eBitwidth = eBitWidth;
	stAudioFrame.eSoundmode = eSoundmode;
	
	s32Ret = MI_AO_SendFrame(audioDev, audioChn, &stAudioFrame, 100);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_AO_SendFrame() in AudioOut::sendStream(). s32Ret = " << s32Ret << endl;
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
int AudioOut::setVolume(int volumeDb)
{
	MI_S32 s32Ret = 0;
	MI_AO_GainFading_e eFading = E_MI_AO_GAIN_FADING_OFF;
	s32Ret = MI_AO_SetVolume(audioDev, audioChn, volumeDb, eFading);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_AO_SetVolume()." << endl;
	}

	return s32Ret;
}

