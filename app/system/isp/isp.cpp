/*---------------------------------------------------------------- 
xxx版权所有。
作者：
时间：2023.6.14
----------------------------------------------------------------*/

#include "isp.h"
#include "mi_isp_iq.h"
#include "mi_iqserver.h"
#include "mi_isp_cus3a_api.h"

#include <iostream>
#include <string.h>

using namespace std;

Isp::Isp()
{
	cout << "Call Isp::Isp()." << endl;
	enable();
	cout << "Call Isp::Isp() end." << endl;
}

Isp::~Isp()
{
	cout << "Call Isp::~Isp()." << endl;
	disable();
	cout << "Call Isp::~Isp() end." << endl;
}

/*-----------------------------------------------------------------------------
描--述：ISP 模块获取实例的唯一入口
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
Isp* Isp::getInstance()
{
	static Isp isp;
	return &isp;
}

int Isp::enable()
{
	cout << "Call Isp::enable()." << endl;

	MI_S32 s32Ret = 0;
	MI_ISP_DevAttr_t stIspDevAttr;
	memset(&stIspDevAttr, 0x0, sizeof(MI_ISP_DevAttr_t));
	stIspDevAttr.u32DevStitchMask = E_MI_ISP_DEVICEMASK_ID0;
	
	s32Ret = MI_ISP_CreateDevice(ispDevId, &stIspDevAttr);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_ISP_CreateDevice() in Isp::enable(). " 
			<< "errno = 0x" << hex << s32Ret << dec << endl;
		return s32Ret;
	}

	MI_ISP_ChannelAttr_t stIspChnAttr;
	memset(&stIspChnAttr, 0x0, sizeof(MI_ISP_ChannelAttr_t));
	stIspChnAttr.u32SensorBindId = E_MI_ISP_SENSOR0;	// 有计算关系。sensor pad0 -> E_MI_ISP_SENSOR0
	
	s32Ret = MI_ISP_CreateChannel(ispDevId, ispChnId, &stIspChnAttr);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_ISP_CreateChannel() in Isp::enable(). " 
			<< "errno = 0x" << hex << s32Ret << dec << endl;
		return s32Ret;
	}
	
	MI_ISP_ChnParam_t stIspChnParam;
	memset(&stIspChnParam, 0x0, sizeof(MI_ISP_ChnParam_t));
	stIspChnParam.eHDRType = E_MI_ISP_HDR_TYPE_OFF;
	stIspChnParam.e3DNRLevel = E_MI_ISP_3DNR_LEVEL_OFF;
	stIspChnParam.bMirror = FALSE;
	stIspChnParam.bFlip = FALSE;
	stIspChnParam.eRot = E_MI_SYS_ROTATE_NONE;
	s32Ret = MI_ISP_SetChnParam(ispDevId, ispChnId, &stIspChnParam);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_ISP_SetChnParam() in Isp::enable(). " 
			<< "errno = 0x" << hex << s32Ret << dec << endl;
		return s32Ret;
	}
	
	s32Ret = MI_ISP_StartChannel(ispDevId, ispChnId);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_ISP_StartChannel() in Isp::enable(). " 
			<< "errno = 0x" << hex << s32Ret << dec << endl;
		return s32Ret;
	}

	#if 0
	MI_ISP_OutPortParam_t stIspOutputParam;
	memset(&stIspOutputParam, 0x0, sizeof(MI_ISP_OutPortParam_t));
	s32Ret = MI_ISP_GetInputPortCrop(ispDevId, ispChnId, &stIspOutputParam.stCropRect);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_ISP_StartChannel() in Isp::enable(). " 
			<< "errno = 0x" << hex << s32Ret << dec << endl;
		return s32Ret;
	}

	// JPEG 需要YUV422, H.26X 需要YUV420. 但此处ISP 可以都输出YUV422, 在VENC 的前级调整即可。
	stIspOutputParam.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;
	//stIspOutputParam.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
	s32Ret = MI_ISP_SetOutputPortParam(ispDevId, ispChnId, ispPortId, &stIspOutputParam);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_ISP_StartChannel() in Isp::enable(). " 
			<< "errno = 0x" << hex << s32Ret << dec << endl;
		return s32Ret;
	}

	s32Ret = MI_ISP_EnableOutputPort(ispDevId, ispChnId, ispPortId);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_ISP_EnableOutputPort() in Isp::enable(). " 
			<< "errno = 0x" << hex << s32Ret << dec << endl;
		return s32Ret;
	}
	#endif
	cout << "Call Isp::enable() end." << endl;
	return 0;
}

int Isp::disable()
{
	cout << "Call Isp::disable()." << endl;
	
	MI_S32 s32Ret = 0;
	s32Ret = MI_ISP_DisableOutputPort(ispDevId, ispChnId, ispPortId);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_ISP_DisableOutputPort() in Isp::disable(). " 
			<< "errno = 0x" << hex << s32Ret << dec << endl;
	}

	s32Ret = MI_ISP_StopChannel(ispDevId, ispChnId);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_ISP_StopChannel() in Isp::disable(). " 
			<< "errno = 0x" << hex << s32Ret << dec << endl;
	}

	s32Ret = MI_ISP_DestroyChannel(ispDevId, ispChnId);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_ISP_DestroyChannel() in Isp::disable(). " 
			<< "errno = 0x" << hex << s32Ret << dec << endl;
	}

	s32Ret = MI_ISP_DestoryDevice(ispDevId);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_ISP_DestoryDevice() in Isp::disable(). " 
			<< "errno = 0x" << hex << s32Ret << dec << endl;
	}
	
	cout << "Call Isp::disable() end." << endl;
	return 0;
}

/*-----------------------------------------------------------------------------
描--述：使能端口。
参--数：portId, 端口号。
返回值：成功，返回0. 失败，返回错误码。
注--意：
-----------------------------------------------------------------------------*/
int Isp::enablePort(MI_ISP_PORT portId)
{
	cout << "Call Isp::enablePort()." << endl;
	MI_S32 s32Ret = 0;
	s32Ret = MI_ISP_EnableOutputPort(ispDevId, ispChnId, ispPortId);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_ISP_EnableOutputPort() in Isp::enablePort(). " 
			<< "errno = 0x" << hex << s32Ret << dec << endl;
	}
	cout << "Call Isp::enablePort() end." << endl;
	return s32Ret;
}

/*-----------------------------------------------------------------------------
描--述：禁用端口。
参--数：portId, 端口号。
返回值：成功，返回0. 失败，返回错误码。
注--意：
-----------------------------------------------------------------------------*/
int Isp::disablePort(MI_ISP_PORT portId)
{
	cout << "Call Isp::disablePort()." << endl;
	MI_S32 s32Ret = 0;
	s32Ret = MI_ISP_DisableOutputPort(ispDevId, ispChnId, ispPortId);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_ISP_DisableOutputPort() in Isp::disablePort(). " 
			<< "errno = 0x" << hex << s32Ret << dec << endl;
	}
	cout << "Call Isp::disablePort() end." << endl;
	return s32Ret;
}

/*-----------------------------------------------------------------------------
描--述：获取输入端口剪裁信息。
参--数：pstCropInfo, 指向端口剪裁信息。
返回值：成功，返回0. 失败，返回错误码。
注--意：
-----------------------------------------------------------------------------*/
int Isp::getInputPortCrop(MI_SYS_WindowRect_t *pstCropInfo)
{
	cout << "Call Isp::getInputPortCrop()." << endl;
	
	MI_S32 s32Ret = 0;
	s32Ret = MI_ISP_GetInputPortCrop(ispDevId, ispChnId, pstCropInfo);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_ISP_GetInputPortCrop() in Isp::getInputPortCrop(). " 
			<< "errno = 0x" << hex << s32Ret << dec << endl;
	}

	cout << "Call Isp::getInputPortCrop() end." << endl;
	return s32Ret;
}

/*-----------------------------------------------------------------------------
描--述：设置输出端口参数。
参--数：pstOutPortParam, 指向端口参数。
返回值：成功，返回0. 失败，返回错误码。
注--意：
-----------------------------------------------------------------------------*/
int Isp::setOutputPortParam(MI_ISP_OutPortParam_t *pstOutPortParam)
{
	cout << "Call Isp::setOutputPortParam()." << endl;
	MI_S32 s32Ret = 0;
	s32Ret = MI_ISP_SetOutputPortParam(ispDevId, ispChnId, ispPortId, pstOutPortParam);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_ISP_SetOutputPortParam() in Isp::setOutputPortParam(). " 
			<< "errno = 0x" << hex << s32Ret << dec << endl;
	}

	cout << "Call Isp::setOutputPortParam() end." << endl;
	return s32Ret;
}

#if 0	// ispahan
/*-----------------------------------------------------------------------------
描--述：设置自动曝光变化范围限制参数值。
参--数：
返回值：成功，返回0. 失败，返回-1.
注--意：
-----------------------------------------------------------------------------*/
int Isp::setExposureLimit(MI_ISP_AE_EXPO_LIMIT_TYPE_t *pExpoLimit)
{
	cout << "Call Isp::setExposureLimit()." << endl;

	MI_ISP_AE_EXPO_LIMIT_TYPE_t expoLimit;
	if(NULL == pExpoLimit)
	{
		cout << "Call Isp::setExposureLimit() with argument null value. Use default value." << endl;
		pExpoLimit = &expoLimit;
		memset(&expoLimit, 0, sizeof(MI_ISP_AE_EXPO_LIMIT_TYPE_t));

		pExpoLimit->u32MinShutterUS = 100;		// n / 1000 = n ms.取值[1, 1 000 000]
		pExpoLimit->u32MaxShutterUS = 100;	// n / 1000 = n ms.
		pExpoLimit->u32MinFNx10 = 18;			// 光圈，F1.8 = 18. 值域范围：10 ~ 220
		pExpoLimit->u32MaxFNx10 = 18;
		pExpoLimit->u32MinSensorGain = 1024;	// 最小Sensor gain(1024等于1倍)。值域范围：1024 ~ 5242880
		pExpoLimit->u32MinISPGain = 1024;		// 最小ISP gain(1024等于1倍)。值域范围：1024 ~ 262144
		pExpoLimit->u32MaxSensorGain = 8192;
		pExpoLimit->u32MaxISPGain = 1024;
	}

	int ret = 0;
	ret = MI_ISP_AE_SetExposureLimit(iSpCh, pExpoLimit);
	if(ret == MI_ISP_FAILURE)
	{
		cerr << "Fail to call int Isp::setExposureLimit(MI_ISP_AE_EXPO_LIMIT_TYPE_t *pExpoLimit))." << endl;
	}
	
	cout << "Call Isp::setExposureLimit() end." << endl;
}

/*-----------------------------------------------------------------------------
描--述：获取自动曝光变化范围限制参数值。
参--数：
返回值：成功，返回0. 失败，返回-1.
注--意：
-----------------------------------------------------------------------------*/
int Isp::getExposureLimit(MI_ISP_AE_EXPO_LIMIT_TYPE_t *pExpoLimit)
{
	cout << "Call Isp::getExposureLimit()." << endl;

	MI_ISP_AE_EXPO_LIMIT_TYPE_t expoLimit;
	if(NULL == pExpoLimit)
	{
		cout << "Call Isp::getExposureLimit() with argument null value. Print them." << endl;
		pExpoLimit = &expoLimit;
	}

	memset(&expoLimit, 0, sizeof(MI_ISP_AE_EXPO_LIMIT_TYPE_t));
	
	int ret = 0;
	ret = MI_ISP_AE_GetExposureLimit(iSpCh, pExpoLimit);
	if(MI_ISP_FAILURE == ret)
	{
		cerr << "Fail to call MI_ISP_AE_GetExposureLimit() in Isp::getExposureLimit()." << endl;
	}
	else
	{
		cout << "u32MinShutterUS = " << pExpoLimit->u32MinShutterUS << endl;
		cout << "u32MaxShutterUS = " << pExpoLimit->u32MaxShutterUS << endl;
		cout << "u32MinFNx10 = " << pExpoLimit->u32MinFNx10 << endl;
		cout << "u32MaxFNx10 = " << pExpoLimit->u32MaxFNx10 << endl;
		cout << "u32MinSensorGain = " << pExpoLimit->u32MinSensorGain << endl;
		cout << "u32MinISPGain = " << pExpoLimit->u32MinISPGain << endl;
		cout << "u32MaxSensorGain = " << pExpoLimit->u32MaxSensorGain << endl;
		cout << "u32MaxISPGain = " << pExpoLimit->u32MaxISPGain << endl;
	}
	
	cout << "Call Isp::getExposureLimit() end." << endl;
}
#endif

/*-----------------------------------------------------------------------------
描--述：设置自动曝光。
参--数：
返回值：成功，返回0. 失败：
        -1, 获取曝光信息；
        -2, 设置曝光失败。
注--意：
-----------------------------------------------------------------------------*/
int Isp::setExpoAuto()
{
	cout << "Call Isp::setExpoAuto()." << endl;

	MI_ISP_AE_EXPO_LIMIT_TYPE_t expoLimit;
	int ret = 0;
	ret = MI_ISP_AE_GetExposureLimit(ispDevId, ispChnId, &expoLimit);
	if(MI_ISP_FAILURE == ret)
	{
		cerr << "Fail to call MI_ISP_AE_GetExposureLimit() in Isp::setExpoAuto()." << endl;
		return -1;
	}

	expoLimit.u32MinShutterUS = 1;
	expoLimit.u32MaxShutterUS = 1000000;

	ret = MI_ISP_AE_SetExposureLimit(ispDevId, ispChnId, &expoLimit);
	if(MI_ISP_FAILURE == ret)
	{
		cerr << "Fail to call MI_ISP_AE_SetExposureLimit() in Isp::setExpoAuto()." << endl;
		return -2;
	}
	
	cout << "Call Isp::setExpoAuto() end." << endl;
	return 0;
}

/*-----------------------------------------------------------------------------
描--述：设置曝光时间。
参--数：expoTimeUs, 曝光时间，微妙级，取值范围[1, 1 000 000]
返回值：成功，返回0. 失败：
        -1, 获取曝光失败；
        -2, 设置曝光失败。
注--意：
-----------------------------------------------------------------------------*/
int Isp::setExpoTimeUs(unsigned int expoTimeUs)
{
	cout << "Call Isp::setExpoTimeUs()." << endl;

	MI_ISP_AE_EXPO_LIMIT_TYPE_t expoLimit;
	int ret = 0;
	ret = MI_ISP_AE_GetExposureLimit(ispDevId, ispChnId, &expoLimit);
	if(MI_ISP_FAILURE == ret)
	{
		cerr << "Fail to call MI_ISP_AE_GetExposureLimit() in Isp::setExpoTimeUs()." << endl;
		return -1;
	}

	expoLimit.u32MinShutterUS = expoTimeUs;
	expoLimit.u32MaxShutterUS = expoTimeUs;

	ret = MI_ISP_AE_SetExposureLimit(ispDevId, ispChnId, &expoLimit);
	if(MI_ISP_FAILURE == ret)
	{
		cerr << "Fail to call MI_ISP_AE_SetExposureLimit() in Isp::setExpoTimeUs()." << endl;
		return -2;
	}
	
	cout << "Call Isp::setExpoTimeUs() end." << endl;
	return 0;
}

#if 0	// ispahan
/*-----------------------------------------------------------------------------
描--述：获取WDR 信息。
参--数：
返回值：成功，返回0. 失败，返回-1.
注--意：
-----------------------------------------------------------------------------*/
int Isp::getWDRParam(MI_ISP_IQ_WDR_TYPE_t *pWdrParam)
{
	cout << "Call Isp::getWDRParam()." << endl;

	MI_ISP_IQ_WDR_TYPE_t wdrParam;
	if(NULL == pWdrParam)
	{
		pWdrParam = &wdrParam;
		cout << "Call Isp::getWDRParam() with argument null value." << endl;
	}
	
	memset(pWdrParam, 0, sizeof(MI_ISP_IQ_WDR_TYPE_t));
	int ret = 0;
	ret = MI_ISP_IQ_GetWDR(iSpCh, pWdrParam);
	if(MI_ISP_FAILURE == ret)
	{
		cerr << "Fail to call MI_ISP_IQ_GetWDR() in Isp::getWDRParam()." << endl;
		return -1;
	}

	cout << "pWdrParam->bEnable = " << (unsigned int)pWdrParam->bEnable << endl;
	cout << "pWdrParam->bEnable = " << pWdrParam->enOpType << endl;

	int i = 0;
	const WDR_PARAM_t *pParam = NULL;
	pParam = pWdrParam->stAuto.stParaAPI;

	#if 0
	for(i = 0; i < MI_ISP_AUTO_NUM; ++i)
	{
		cout << " = " << pParam->bAutoDetailEnhance << endl;
		cout << " = " << pParam->bGammaSyncEn << endl;
		cout << " = " << pParam->u8BoxNum << endl;
		cout << " = " << pParam->u8BrightLimit << endl;
		cout << " = " << pParam->u8DarkLimit << endl;
		cout << " = " << pParam->u8GlobalDarkToneEnhance << endl;
		cout << " = " << pParam->u8ManualDetailEnhance << endl;
		cout << " = " << pParam->u8PreEnhance << endl;
		cout << " = " << pParam->u8Strength << endl;
		++pWdrParam;
	}
	#endif

	#if 0
	pParam = &pWdrParam->stManual.stParaAPI;
	cout << "output Manual Param." << endl;
	cout << "pParam->bAutoDetailEnhance = " << (unsigned int)pParam->bAutoDetailEnhance << endl;
	cout << "pParam->bGammaSyncEn = " << (unsigned int)pParam->bGammaSyncEn << endl;
	cout << "pParam->u8BoxNum = " << (unsigned int)pParam->u8BoxNum << endl;
	cout << "pParam->u8BrightLimit = " << (unsigned int)pParam->u8BrightLimit << endl;
	cout << "pParam->u8DarkLimit = " << (unsigned int)pParam->u8DarkLimit << endl;
	cout << "pParam->u8GlobalDarkToneEnhance = " << (unsigned int)pParam->u8GlobalDarkToneEnhance << endl;
	cout << "pParam->u8ManualDetailEnhance = " << (unsigned int)pParam->u8ManualDetailEnhance << endl;
	cout << "pParam->u8PreEnhance = " << (unsigned int)pParam->u8PreEnhance << endl;
	cout << "pParam->u8Strength = " << (unsigned int)pParam->u8Strength << endl;
	#endif
	
	cout << "Call Isp::getWDRParam() end." << endl;
	return 0;
}

/*-----------------------------------------------------------------------------
描--述：设置WDR 信息
参--数：
返回值：成功，返回0. 失败：
        -1, 入参错误；
        -2, 
注--意：
-----------------------------------------------------------------------------*/
int Isp::setWDRParam(MI_ISP_IQ_WDR_TYPE_t *pWdrParam)
{
	cout << "Call Isp::setWDRParam()." << endl;

	if(NULL == pWdrParam)
	{
		cout << "Call Isp::setWDRParam() with argument null value." << endl;
		return -1;
	}
	
	int ret = 0;
	ret = MI_ISP_IQ_SetWDR(iSpCh, pWdrParam);
	if(MI_ISP_FAILURE == ret)
	{
		cerr << "Fail to call MI_ISP_IQ_SetWDR() in Isp::setWDRParam()." << endl;
		return -2;
	}

	cout << "Call Isp::setWDRParam() end." << endl;
	return 0;
}
#endif

/*-----------------------------------------------------------------------------
描--述：使能WDR
参--数：autoOrManual, 自动启用还是手动启用，0, 自动；1, 手动。
返回值：成功，返回0. 失败：
        -1, 获取WDR 参数失败；
        -2, 设置WDR 参数失败。
注--意：
-----------------------------------------------------------------------------*/
int Isp::enableWDR(int autoOrManual)
{
	cout << "Call Isp::enableWDR()." << endl;

	MI_ISP_IQ_WDR_TYPE_t wdrParam;
	memset(&wdrParam, 0, sizeof(MI_ISP_IQ_WDR_TYPE_t));
	
	int ret = 0;
	ret = MI_ISP_IQ_GetWDR(ispDevId, ispChnId, &wdrParam);
	if(MI_ISP_OK != ret)
	{
		cerr << "Fail to call MI_ISP_IQ_GetWDR() in Isp::enableWDR()." << endl;
		return -1;
	}

	wdrParam.bEnable = SS_TRUE;
	if(0 == autoOrManual)
	{
		wdrParam.enOpType = SS_OP_TYP_AUTO;
	}
	else if(1 == autoOrManual)
	{
		wdrParam.enOpType = SS_OP_TYP_MANUAL;
	}
	else
	{
		cerr << "Bad argument in Isp::enableWDR()." << endl;
		wdrParam.enOpType = SS_OP_TYP_AUTO;
	}

	ret = MI_ISP_IQ_SetWDR(ispDevId, ispChnId, &wdrParam);
	if(MI_ISP_OK != ret)
	{
		cerr << "Fail to call MI_ISP_IQ_SetWDR() in Isp::enableWDR()." << endl;
		return -2;
	}

	cout << "Call Isp::enableWDR() end." << endl;
	return 0;
}

/*-----------------------------------------------------------------------------
描--述：禁用WDR
参--数：
返回值：成功，返回0. 失败：
        -1, 获取WDR 参数失败；
        -2, 设置WDR 参数失败。
注--意：
-----------------------------------------------------------------------------*/
int Isp::disableWDR()
{
	cout << "Call Isp::disableWDR()." << endl;

	MI_ISP_IQ_WDR_TYPE_t wdrParam;
	memset(&wdrParam, 0, sizeof(MI_ISP_IQ_WDR_TYPE_t));
	
	int ret = 0;
	ret = MI_ISP_IQ_GetWDR(ispDevId, ispChnId, &wdrParam);
	if(MI_ISP_OK != ret)
	{
		cerr << "Fail to call MI_ISP_IQ_GetWDR() in Isp::disableWDR()." << endl;
		return -1;
	}

	wdrParam.bEnable = SS_FALSE;
	ret = MI_ISP_IQ_SetWDR(ispDevId, ispChnId, &wdrParam);
	if(MI_ISP_OK != ret)
	{
		cerr << "Fail to call MI_ISP_IQ_SetWDR() in Isp::disableWDR()." << endl;
		return -2;
	}

	cout << "Call Isp::disableWDR() end." << endl;
	return 0;
}

/*-----------------------------------------------------------------------------
描--述：打开IQ Server
参--数：
返回值：成功，返回0.
		失败，返回-1.
注--意：打开后才能连接IQ Tool
-----------------------------------------------------------------------------*/
int Isp::openIqServer()
{
	cout << "Call Isp::openIqServer()." << endl;
	MI_S32 s32Ret = 0;
	s32Ret = MI_IQSERVER_Open();
	if(MI_IQSERVER_OK != s32Ret)
	{
		cerr << "Fail to call MI_IQSERVER_Open() in Isp::openIqServer(). " 
			<< "s32Ret = " << s32Ret << endl;
		return -1;
	}

	cout << "Call Isp::openIqServer() end." << endl;
	return 0;
}

/*-----------------------------------------------------------------------------
描--述：关闭IQ Server
参--数：
返回值：成功，返回0.
		失败，返回-1.
注--意：
-----------------------------------------------------------------------------*/
int Isp::closeIqServer()
{
	cout << "Call Isp::closeIqServer() end." << endl;
	MI_S32 s32Ret = 0;
	s32Ret = MI_IQSERVER_Close();
	if(MI_IQSERVER_OK != s32Ret)
	{
		cerr << "Fail to call MI_IQSERVER_Close() in Isp::closeIqServer(). " 
			<< "s32Ret = " << s32Ret << endl;
		return -1;
	}

	cout << "Call Isp::closeIqServer() end." << endl;
	return 0;
}

/*-----------------------------------------------------------------------------
描--述：设置IQ 文件路径
参--数：
返回值：成功，返回0.
		失败，返回-1.
注--意：
-----------------------------------------------------------------------------*/
int Isp::setIqServerDataPath(char *iqFilePath)
{
	cout << "Call Isp::setIqServerDataPath()." << endl;

	MI_S32 s32Ret = 0;
	s32Ret = MI_IQSERVER_SetDataPath(iqFilePath);
	if(MI_IQSERVER_OK != s32Ret)
	{
		cerr << "Fail to call MI_IQSERVER_SetDataPath() in Isp::setIqServerDataPath(). " 
			<< "s32Ret = " << s32Ret << endl;
		return -1;
	}
	
	cout << "Call Isp::setIqServerDataPath() end." << endl;
	return s32Ret;
}

/*-----------------------------------------------------------------------------
描--述：加载IQ 文件
参--数：
返回值：成功，返回0.
		失败，返回-1.
注--意：
-----------------------------------------------------------------------------*/
int Isp::loadBinFile(char* filepath)
{
	cout << "Call Isp::loadBinFile()." << endl;
	cout << "IQ filepath = " << filepath << endl;
	
	MI_S32 s32Ret = 0;
	s32Ret = MI_ISP_API_CmdLoadBinFile(0, 0, filepath, 1234);
	if(MI_IQSERVER_OK != s32Ret)
	{
		cerr << "Fail to call MI_ISP_API_CmdLoadBinFile() in Isp::loadBinFile(). " 
			<< "s32Ret = " << s32Ret << endl;
		return -1;
	}
	
	cout << "Call Isp::loadBinFile() end." << endl;
	return s32Ret;
}


