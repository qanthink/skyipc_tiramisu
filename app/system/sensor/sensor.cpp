/*---------------------------------------------------------------- 
xxx版权所有。
作者：
时间：2020.7.10
----------------------------------------------------------------*/

#include "sensor.h"
#include <iostream>
#include <string.h>

using namespace std;

Sensor::Sensor()
{
	enable();
}

Sensor::~Sensor()
{
	disable();
	bEnable = false;
}

/*-----------------------------------------------------------------------------
描--述：Sensor 模块获取实例的唯一入口函数。
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
Sensor* Sensor::getInstance()
{
	static Sensor sensor;
	return &sensor;
}


/*-----------------------------------------------------------------------------
描--述：
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
int Sensor::enable()
{
	cout << "Call Sensor::enable()." << endl;

	if(bEnable)
	{
		cerr << "Fail to call Sensor::enable(). Modules has already been enabled!" << endl;
		return 0;
	}

	// 设置HDR 开关。
	MI_S32 s32Ret = 0;
	s32Ret = MI_SNR_SetPlaneMode(ePADId, bEnableHDR);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_SNR_SetPlaneMode() in Sensor::enable(). " 
			<< "errno = 0x" << hex << s32Ret << dec << endl;
		return s32Ret;
	}

	// 查询配置。
	MI_U32 u32ResCount = 0;
	s32Ret = MI_SNR_QueryResCount(ePADId, &u32ResCount);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_SNR_QueryResCount() in Sensor::enable(). " 
			<< "errno = 0x" << hex << s32Ret << dec << endl;
		return s32Ret;
	}

	// 显示所有配置。
	int i = 0;
	for(i = 0; i < u32ResCount; ++i)
	{
		MI_SNR_Res_t stSnrRes;
		memset(&stSnrRes, 0, sizeof(MI_SNR_Res_t));
		
		s32Ret = MI_SNR_GetRes(ePADId, i, &stSnrRes);
		if(0 != s32Ret)
		{
			cerr << "Fail to call MI_SNR_GetRes() in Sensor::enable(). "
				<< "errno = 0x" << hex << s32Ret << dec << endl;
			continue;
		}

		printf("index %d, Crop(%d,%d,%d,%d), outputsize(%d,%d), maxfps %d, minfps %d, ResDesc %s\n",
		i,
		stSnrRes.stCropRect.u16X, stSnrRes.stCropRect.u16Y, stSnrRes.stCropRect.u16Width,stSnrRes.stCropRect.u16Height,
		stSnrRes.stOutputSize.u16Width, stSnrRes.stOutputSize.u16Height,
		stSnrRes.u32MaxFps,stSnrRes.u32MinFps,
		stSnrRes.strResDesc);
	}

	// 设置配置
	cout << "In Sensor::enable(), u8SnrResIndex = " << (int)u8SnrResIndex << endl;
	s32Ret = MI_SNR_SetRes(ePADId, u8SnrResIndex);	// 单sensor 方案中，一般使用0 配置。
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_SNR_SetRes() in Sensor::enable(). " 
			<< "errno = 0x" << hex << s32Ret << dec << endl;
		return s32Ret;
	}

	// 设置帧率。
	s32Ret = MI_SNR_SetFps(ePADId, u32DefFps);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_SNR_SetFps() in Sensor::enable(). " 
			<< "errno = 0x" << hex << s32Ret << dec << endl;
		return s32Ret;
	}

	// 启用sensor.
	s32Ret = MI_SNR_Enable(ePADId);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_SNR_Enable() in Sensor::enable(). " 
			<< "errno = 0x" << hex << s32Ret << dec << endl;
		return s32Ret;
	}
	
	bEnable = true;
	cout << "Call Sensor::enable() end." << endl;
	return 0;
}

/*-----------------------------------------------------------------------------
描--述：
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
int Sensor::disable()
{
	cout << "Call Sensor::disable()." << endl;
	
	MI_S32 s32Ret = 0;
	s32Ret = MI_SNR_Disable(ePADId);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_SNR_Disable(), errno = " << s32Ret << endl;
	}
	bEnable = false;

	cout << "Call Sensor::disable() end." << endl;
	return 0;
}

/*-----------------------------------------------------------------------------
描--述：
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
int Sensor::getFps(MI_U32 *pFps)
{
	cout << "Call Sensor::getFps()." << endl;
	//MI_S32 MI_SNR_GetFps(MI_SNR_PAD_ID_e	  ePADId, MI_U32 *pFps);
	
	MI_S32 s32Ret = 0;
	s32Ret = MI_SNR_GetFps(ePADId, pFps);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_SNR_GetFps(), errno = " << hex << s32Ret << endl;
	}
	cout << "FPS = " << *pFps << endl;

	cout << "Call Sensor::getFps() end." << endl;
	return s32Ret;
}

/*-----------------------------------------------------------------------------
描--述：
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
int Sensor::setFps(MI_U32 u32Fps)
{
	cout << "Call Sensor::setFps()." << endl;
	
	//MI_S32 MI_SNR_SetFps(MI_SNR_PAD_ID_e	  ePADId, MI_U32  u32Fps);
	MI_S32 s32Ret = 0;
	s32Ret = MI_SNR_SetFps(ePADId, u32Fps);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_SNR_SetFps(), errno = " << hex << s32Ret << endl;
	}

	cout << "Call Sensor::setFps() end." << endl;
	return s32Ret;
}

/*-----------------------------------------------------------------------------
描--述：
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
int Sensor::getPadInfo(MI_SNR_PADID ePADId, MI_SNR_PADInfo_t *pstPadInfo)
{
	cout << "Call Sensor::getPadInfo()." << endl;
	
	if(NULL == pstPadInfo)
	{
		cerr << "Fail to call getPadInfo(), argument has null value!"<< endl;
		return -1;
	}

	// MI_S32 MI_SNR_GetPadInfo(MI_SNR_PAD_ID_e ePADId, MI_SNR_PADInfo_t *pstPadInfo);
	MI_S32 s32Ret = 0;
	memset(pstPadInfo, 0, sizeof(MI_SNR_PADInfo_t));
	s32Ret = MI_SNR_GetPadInfo(ePADId, pstPadInfo);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_VIF_SetDevAttr(), errno = " << s32Ret << endl;
	}

	cout << "Call Sensor::getPadInfo() end." << endl;
	return s32Ret;	
}

/*-----------------------------------------------------------------------------
描--述：
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
int Sensor::getPlaneInfo(MI_SNR_PADID ePADId, MI_SNR_PlaneInfo_t *pstPlaneInfo)
{
	cout << "Call Sensor::getPlaneInfo()." << endl;
	
	//MI_S32 MI_SNR_GetPlaneInfo(MI_SNR_PAD_ID_e ePADId, MI_U32 u32PlaneID, MI_SNR_PlaneInfo_t *pstPlaneInfo);
	MI_S32 s32Ret = 0;
	memset(pstPlaneInfo, 0, sizeof(MI_SNR_PlaneInfo_t));
	s32Ret = MI_SNR_GetPlaneInfo(ePADId, u32PlaneID, pstPlaneInfo);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_SNR_GetPlaneInfo() in Sensor::getPlaneInfo(), errno = " << s32Ret << endl;
	}

	cout << "Call Sensor::getPlaneInfo() end." << endl;
	return s32Ret;	
}

/*-----------------------------------------------------------------------------
描--述：获取sensor 分辨率。
参--数：指向宽、高。
返回值：成功，返回0; 失败，返回错误码。
注--意：
-----------------------------------------------------------------------------*/
int Sensor::getSnrWH(unsigned int *pSnrW, unsigned int *pSnrH)
{
	cout << "Call Sensor::getSnrWH()." << endl;

	MI_S32 s32Ret = 0;
	MI_SNR_PlaneInfo_t stPlaneInfo;
	
	memset(&stPlaneInfo, 0, sizeof(MI_SNR_PlaneInfo_t));
	s32Ret = MI_SNR_GetPlaneInfo(ePADId, u32PlaneID, &stPlaneInfo);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_SNR_GetPlaneInfo() in Sensor::getSnrWH(), errno = " << s32Ret << endl;
		return s32Ret;
	}

	*pSnrW = stPlaneInfo.stCapRect.u16Width;
	*pSnrH = stPlaneInfo.stCapRect.u16Height;
	
	cout << "Call Sensor::getSnrWH() end." << endl;
	return 0;
}


