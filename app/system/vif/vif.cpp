/*---------------------------------------------------------------- 
xxx版权所有。
作者：
时间：2020.7.10
----------------------------------------------------------------*/

#include "sys.h"
#include "vif.h"
#include "sensor.hpp"
#include <iostream>
#include <string.h>

using namespace std;

Vif::Vif()
{
	enable();
}

Vif::~Vif()
{
	disable();
}

/*-----------------------------------------------------------------------------
描--述：VIF 模块获取实例的唯一入口函数。
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
Vif* Vif::getInstance()
{
	static Vif vif;
	return &vif;
}

/*-----------------------------------------------------------------------------
描--述：
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
MI_S32 Vif::enable()
{
	cout << "Call Vif::enable()." << endl;
	
	if(bEnable)
	{
		cerr << "Fail to call Vif::enable(). Modules is already been enabled!" << endl;
		return 0;
	}

	setDevAttr();
	enableDev();
	setChnPortAttr(vifPort);
	enableChnPort(vifPort);

	bEnable = true;

	cout << "Call Vif::enable() end." << endl;
	return 0;	
}

/*-----------------------------------------------------------------------------
描--述：
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
MI_S32 Vif::disable()
{
	cout << "Call Vif::disable()." << endl;

	disableChnPort(vifPort);
	disableDev();
	bEnable = false;

	cout << "Call Vif::disable() end." << endl;
	return 0;
}

/*-----------------------------------------------------------------------------
描--述：
参--数：
返回值：
注--意：首选获取sensor 模块的pad、plane 信息，然后引用相关数据设置VIF 设备属性。
-----------------------------------------------------------------------------*/
MI_S32 Vif::setDevAttr()
{
	cout << "Call Vif::setDevAttr()." << endl;
	
	MI_S32 s32Ret = 0;
	
	// step1: 读取sensor 信息。
	MI_SNR_PADInfo_t stPadInfo;
	memset(&stPadInfo, 0, sizeof(MI_SNR_PADInfo_t));
	Sensor *pSensor = Sensor::getInstance();
	s32Ret = pSensor->getPadInfo(0, &stPadInfo);
	if(0 != s32Ret)
	{
		cerr << "Fail to call pSensor->getPadInfo()." << endl;
		return s32Ret;
	}

	MI_SNR_PlaneInfo_t stPlaneInfo;
	memset(&stPlaneInfo, 0, sizeof(MI_SNR_PlaneInfo_t));
	s32Ret = pSensor->getPlaneInfo(0, &stPlaneInfo);
	if(0 != s32Ret)
	{
		cerr << "Fail to call pSensor->getPlaneInfo()." << endl;
		return s32Ret;
	}

	MI_VIF_GROUP groupId = 0;
	MI_VIF_GroupAttr_t stGroupAttr;
	memset(&stGroupAttr, 0, sizeof(MI_VIF_GroupAttr_t));

	stGroupAttr.eIntfMode = E_MI_VIF_MODE_MIPI;
	stGroupAttr.eWorkMode = E_MI_VIF_WORK_MODE_1MULTIPLEX;
	stGroupAttr.eHDRType = E_MI_VIF_HDR_TYPE_OFF;
	if(E_MI_VIF_MODE_BT656 == stGroupAttr.eIntfMode)
	{
		stGroupAttr.eClkEdge = (MI_VIF_ClkEdge_e)stPadInfo.unIntfAttr.stBt656Attr.eClkEdge;
	}
	else
	{
		stGroupAttr.eClkEdge = E_MI_VIF_CLK_EDGE_DOUBLE;
	}
	
	s32Ret = MI_VIF_CreateDevGroup(groupId, &stGroupAttr);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_VIF_CreateDevGroup()." << endl;
		return s32Ret;
	}
	
	// step2: 填充VIF 设备属性结构体。
	memset(&stDevAttr, 0, sizeof(MI_VIF_DevAttr_t));
	stDevAttr.stInputRect.u16X = stPlaneInfo.stCapRect.u16X;
	stDevAttr.stInputRect.u16Y = stPlaneInfo.stCapRect.u16Y;
	stDevAttr.stInputRect.u16Width = stPlaneInfo.stCapRect.u16Width;
	stDevAttr.stInputRect.u16Height = stPlaneInfo.stCapRect.u16Height;
	stDevAttr.bEnH2T1PMode = false;
	stDevAttr.eField = E_MI_SYS_FIELDTYPE_NONE;
	if(stDevAttr.eInputPixel >= E_MI_SYS_PIXEL_FRAME_FORMAT_MAX)
	{
		if(stPlaneInfo.eBayerId >= E_MI_SYS_PIXEL_BAYERID_MAX)
		{
			stDevAttr.eInputPixel = stPlaneInfo.ePixel;
		}
		else
		{
			stDevAttr.eInputPixel = (MI_SYS_PixelFormat_e)RGB_BAYER_PIXEL(stPlaneInfo.ePixPrecision, stPlaneInfo.eBayerId);
		}
	}
	stDevAttr.eInputPixel = E_MI_SYS_PIXEL_FRAME_YUV420_PLANAR;

	// step3: 设置VIF 设备属性结构体。
	s32Ret = MI_VIF_SetDevAttr(vifDev, &stDevAttr);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_VIF_SetDevAttr(), errno = " << s32Ret << endl;
	}

	cout << "Call Vif::setDevAttr() end." << endl;
	return s32Ret;
}

/*-----------------------------------------------------------------------------
描--述：
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
MI_S32 Vif::enableDev()
{
	cout << "Call Vif::enableDev()." << endl;
	//MI_S32 MI_VIF_EnableDev(MI_VIF_DEV vifDev);

	MI_S32 s32Ret = 0;
	s32Ret = MI_VIF_EnableDev(vifDev);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_VIF_EnableDev(), errno = " << s32Ret << endl;
	}

	cout << "Call Vif::enableDev() end." << endl;
	return s32Ret;
}

/*-----------------------------------------------------------------------------
描--述：
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
MI_S32 Vif::disableDev()
{
	cout << "Call Vif::disableDev()." << endl;

	// MI_S32 MI_VIF_DisableDev(MI_VIF_DEV vifDev);
	MI_S32 s32Ret = 0;

	
	s32Ret = MI_VIF_DisableDev(vifDev);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_VIF_DisableDev(), errno = " << s32Ret << endl;
	}

	cout << "Call Vif::disableDev() end." << endl;
	return s32Ret;
}

/*-----------------------------------------------------------------------------
描--述：
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
MI_S32 Vif::setChnPortAttr(MI_VIF_PORT u32ChnPort)
{
	cout << "Call Vif::setChnPortAttr()." << endl;

	//MI_S32 MI_VIF_SetChnPortAttr(MI_VIF_CHN u32VifChn, MI_VIF_PORT u32ChnPort, MI_VIF_ChnPortAttr_t *pstAttr);
	MI_S32 s32Ret = 0;

	// step1: 获取sensor 模块plane 信息。
	MI_SNR_PlaneInfo_t stPlaneInfo;
	memset(&stPlaneInfo, 0, sizeof(MI_SNR_PlaneInfo_t));
	
	Sensor *pSensor = Sensor::getInstance();
	s32Ret = pSensor->getPlaneInfo(0, &stPlaneInfo);
	if(0 != s32Ret)
	{
		cerr << "Fail to call pSensor->getPlaneInfo()." << endl;
		return s32Ret;
	}

	MI_VIF_OutputPortAttr_t stVifPortInfo;
	memset(&stVifPortInfo, 0, sizeof(MI_VIF_OutputPortAttr_t));

	stVifPortInfo.stCapRect.u16X = stPlaneInfo.stCapRect.u16X;
	stVifPortInfo.stCapRect.u16Y = stPlaneInfo.stCapRect.u16Y;
	stVifPortInfo.stCapRect.u16Width = stPlaneInfo.stCapRect.u16Width;
	stVifPortInfo.stCapRect.u16Height = stPlaneInfo.stCapRect.u16Height;
	stVifPortInfo.stDestSize.u16Width = stPlaneInfo.stCapRect.u16Width;
	stVifPortInfo.stDestSize.u16Height = stPlaneInfo.stCapRect.u16Height;

	if(stVifPortInfo.ePixFormat >= E_MI_SYS_PIXEL_FRAME_FORMAT_MAX)
	{
		stVifPortInfo.ePixFormat = stDevAttr.eInputPixel;
	}

	if(true == stDevAttr.bEnH2T1PMode)
	{
		stVifPortInfo.stCapRect.u16Width = stVifPortInfo.stCapRect.u16Width / 2;
		stVifPortInfo.stDestSize.u16Width = stVifPortInfo.stDestSize.u16Width / 2;
	}

	stVifPortInfo.stCapRect.u16X = stVifPortInfo.stCapRect.u16X;
	stVifPortInfo.stCapRect.u16Y = stVifPortInfo.stCapRect.u16Y;
	stVifPortInfo.stCapRect.u16Width =  stVifPortInfo.stCapRect.u16Width;
	stVifPortInfo.stCapRect.u16Height = stVifPortInfo.stCapRect.u16Height;
	stVifPortInfo.stDestSize.u16Width = stVifPortInfo.stDestSize.u16Width;
	stVifPortInfo.stDestSize.u16Height = stVifPortInfo.stDestSize.u16Height;
	printf("sensor bayerid %d, bit mode %d \n", stPlaneInfo.eBayerId, stPlaneInfo.ePixPrecision);
	stVifPortInfo.ePixFormat = stVifPortInfo.ePixFormat;
	//stVifPortInfo.u32FrameModeLineCount for lowlantancy mode
	stVifPortInfo.eFrameRate = stVifPortInfo.eFrameRate;

	MI_VIF_SetOutputPortAttr(vifDev, vifPort, &stVifPortInfo);
	
	cout << "Call Vif::setChnPortAttr() end." << endl;
	return s32Ret;
}

/*-----------------------------------------------------------------------------
描--述：
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
MI_S32 Vif::enableChnPort(MI_VIF_PORT u32ChnPort)
{
	cout << "Call Vif::enableChnPort()." << endl;

	MI_SYS_ChnPort_t stChnPort;
	memset(&stChnPort, 0x0, sizeof(MI_SYS_ChnPort_t));
	stChnPort.eModId = E_MI_MODULE_ID_VIF;
	stChnPort.u32DevId = vifDev;
	//stChnPort.u32ChnId = vifChn;
	stChnPort.u32PortId = u32ChnPort;
	//MI_SYS_SetChnOutputPortDepth(&stChnPort, 0, 6);
	
	//MI_S32 MI_VIF_EnableChnPort(MI_VIF_CHN vifChn, MI_VIF_PORT u32ChnPort);
	MI_S32 s32Ret = 0;

	s32Ret = MI_VIF_EnableOutputPort(vifDev, u32ChnPort);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_VIF_EnableChnPort(), errno = " << s32Ret << endl;
	}

	cout << "Call Vif::enableChnPort() end." << endl;
	return s32Ret;
}

/*-----------------------------------------------------------------------------
描--述：
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
MI_S32 Vif::disableChnPort(MI_VIF_PORT u32ChnPort)
{
	cout << "Call Vif::disableChnPort()." << endl;

	// MI_S32 MI_VIF_DisableChnPort(MI_VIF_CHN vifChn, MI_VIF_PORT u32ChnPort);
	MI_S32 s32Ret = 0;

	s32Ret = MI_VIF_DisableOutputPort(vifDev, u32ChnPort);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_VIF_DisableChnPort(), errno = " << s32Ret << endl;
	}

	cout << "Call Vif::disableChnPort() end." << endl;
	return s32Ret;
}

