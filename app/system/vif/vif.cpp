/*---------------------------------------------------------------- 
xxx版权所有。
作者：
时间：2020.7.10
----------------------------------------------------------------*/

#include "sys.h"
#include "vif.h"
#include "sensor.h"
#include <iostream>
#include <string.h>

using namespace std;

Vif::Vif()
{
	cout << "Call Vif::Vif()." << endl;
	enable();
	cout << "Call Vif::Vif() end." << endl;
}

Vif::~Vif()
{
	cout << "Call Vif::Vif() ~Vif." << endl;
	disable();
	cout << "Call Vif::Vif() ~Vif." << endl;
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

	MI_S32 s32Ret = 0;
	
	// step1: 读取sensor 信息。
	MI_SNR_PADInfo_t stPadInfo;
	memset(&stPadInfo, 0, sizeof(MI_SNR_PADInfo_t));
	
	Sensor *pSensor = Sensor::getInstance();
	s32Ret = pSensor->getPadInfo(Sensor::ePADId, &stPadInfo);
	if(0 != s32Ret)
	{
		cerr << "Fail to call pSensor->getPadInfo() in Vif::enable()." 
			<< "errno = 0x" << hex << s32Ret << dec << endl;
		return s32Ret;
	}

	MI_SNR_PlaneInfo_t stPlaneInfo;
	memset(&stPlaneInfo, 0, sizeof(MI_SNR_PlaneInfo_t));
	
	s32Ret = pSensor->getPlaneInfo(Sensor::ePADId, &stPlaneInfo);
	if(0 != s32Ret)
	{
		cerr << "Fail to call pSensor->getPlaneInfo() in Vif::enable()." 
			<< "errno = 0x" << hex << s32Ret << dec << endl;
		return s32Ret;
	}

	// step2: 创建VIF 设备组
	MI_VIF_GROUP vifGroupId = 0;		// 有复杂的计算关系。
	MI_VIF_GroupAttr_t stVifGroupAttr;
	memset(&stVifGroupAttr, 0, sizeof(MI_VIF_GroupAttr_t));
	stVifGroupAttr.eIntfMode = E_MI_VIF_MODE_MIPI;
	stVifGroupAttr.eWorkMode = E_MI_VIF_WORK_MODE_1MULTIPLEX;
	stVifGroupAttr.eHDRType = E_MI_VIF_HDR_TYPE_OFF;
	stVifGroupAttr.eClkEdge = E_MI_VIF_CLK_EDGE_DOUBLE;
	//stVifGroupAttr.eClkEdge = (MI_VIF_ClkEdge_e)stSnrPadInfo.unIntfAttr.stBt656Attr.eClkEdge;

	s32Ret = MI_VIF_CreateDevGroup(vifGroupId, &stVifGroupAttr);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_VIF_CreateDevGroup() in Vif::enable()." 
			<< "errno = 0x" << hex << s32Ret << dec << endl;
		return s32Ret;
	}

	// step3: 创建VIF 设备，设置设备属性。
	MI_VIF_DEV vifDevId = 0;			// 有复杂的计算关系。
	MI_VIF_DevAttr_t stVifDevAttr;
	memset(&stVifDevAttr, 0, sizeof(MI_VIF_DevAttr_t));
	stVifDevAttr.stInputRect.u16X = stPlaneInfo.stCapRect.u16X;
	stVifDevAttr.stInputRect.u16Y = stPlaneInfo.stCapRect.u16Y;
	stVifDevAttr.stInputRect.u16Width = stPlaneInfo.stCapRect.u16Width;
	stVifDevAttr.stInputRect.u16Height = stPlaneInfo.stCapRect.u16Height;
	if(stPlaneInfo.eBayerId >= E_MI_SYS_PIXEL_BAYERID_MAX)
	{
		stVifDevAttr.eInputPixel = stPlaneInfo.ePixel;
	}
	else
	{
		stVifDevAttr.eInputPixel = (MI_SYS_PixelFormat_e)RGB_BAYER_PIXEL(stPlaneInfo.ePixPrecision, stPlaneInfo.eBayerId);
	}
	
	s32Ret = MI_VIF_SetDevAttr(vifDevId, &stVifDevAttr);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_VIF_SetDevAttr() in Vif::enable()." 
			<< "errno = 0x" << hex << s32Ret << dec << endl;
		return s32Ret;
	}

	// step4: 激活VIF 设备
	s32Ret = MI_VIF_EnableDev(vifDevId);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_VIF_EnableDev() in Vif::enable()." 
			<< "errno = 0x" << hex << s32Ret << dec << endl;
		return s32Ret;
	}

	// step5: 设置输出端口输出属性、深度。
	MI_VIF_OutputPortAttr_t stVifPortAttr;
	memset(&stVifPortAttr, 0, sizeof(MI_VIF_OutputPortAttr_t));

	stVifPortAttr.stCapRect.u16X = stPlaneInfo.stCapRect.u16X;
	stVifPortAttr.stCapRect.u16Y = stPlaneInfo.stCapRect.u16Y;
	stVifPortAttr.stCapRect.u16Width =  stPlaneInfo.stCapRect.u16Width;
	stVifPortAttr.stCapRect.u16Height = stPlaneInfo.stCapRect.u16Height;
	stVifPortAttr.stDestSize.u16Width = stPlaneInfo.stCapRect.u16Width;
	stVifPortAttr.stDestSize.u16Height = stPlaneInfo.stCapRect.u16Height;
	stVifPortAttr.eFrameRate = E_MI_VIF_FRAMERATE_FULL;
	if(stPlaneInfo.eBayerId >= E_MI_SYS_PIXEL_BAYERID_MAX)
	{
		stVifPortAttr.ePixFormat = stPlaneInfo.ePixel;
	}
	else
	{
		stVifPortAttr.ePixFormat = (MI_SYS_PixelFormat_e)RGB_BAYER_PIXEL(stPlaneInfo.ePixPrecision, stPlaneInfo.eBayerId);
	}
	
	s32Ret = MI_VIF_SetOutputPortAttr(vifDevId, vifPortId, &stVifPortAttr);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_VIF_SetOutputPortAttr() in Vif::enable()." 
			<< "errno = 0x" << hex << s32Ret << dec << endl;
		return s32Ret;
	}

	MI_SYS_ChnPort_t stChnPort;
	MI_VIF_DEV vifChnId = 0;
	memset(&stChnPort, 0, sizeof(MI_SYS_ChnPort_t));
	
	stChnPort.eModId = E_MI_MODULE_ID_VIF;
	stChnPort.u32DevId = vifDevId;
	stChnPort.u32ChnId = vifChnId;
	stChnPort.u32PortId = vifPortId;
	s32Ret = MI_SYS_SetChnOutputPortDepth(Sys::u16SocId, &stChnPort, 0, 3 + 3);	// 此处可优化。
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_SYS_SetChnOutputPortDepth() in Vif::enable()." 
			<< "errno = 0x" << hex << s32Ret << dec << endl;
		return s32Ret;
	}

	// step6: 使能输出端口。
	s32Ret = MI_VIF_EnableOutputPort(vifDevId, vifPortId);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_VIF_EnableOutputPort() in Vif::enable()." 
			<< "errno = 0x" << hex << s32Ret << dec << endl;
		return s32Ret;
	}

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

	//disableChnPort(vifPort);
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
	s32Ret = pSensor->getPadInfo(Sensor::ePADId, &stPadInfo);
	if(0 != s32Ret)
	{
		cerr << "Fail to call pSensor->getPadInfo()." << endl;
		return s32Ret;
	}

	MI_SNR_PlaneInfo_t stPlaneInfo;
	memset(&stPlaneInfo, 0, sizeof(MI_SNR_PlaneInfo_t));
	
	s32Ret = pSensor->getPlaneInfo(Sensor::ePADId, &stPlaneInfo);
	if(0 != s32Ret)
	{
		cerr << "Fail to call pSensor->getPlaneInfo()." << endl;
		return s32Ret;
	}
	
	// step2: 填充VIF 设备属性结构体。
	//MI_S32 MI_VIF_SetDevAttr(MI_VIF_DEV u32VifDev, MI_VIF_DevAttr_t *pstDevAttr);
	MI_VIF_DevAttr_t stDevAttr;
	memset(&stDevAttr, 0, sizeof(MI_VIF_DevAttr_t));
#if 0
	stDevAttr.eIntfMode = stPadInfo.eIntfMode;
	stDevAttr.eHDRType = E_MI_VIF_HDR_TYPE_OFF;				// 参考mi_demo/jedi/rtsp/st_main_rtsp.c
	stDevAttr.eBitOrder = E_MI_VIF_BITORDER_NORMAL;
	stDevAttr.eWorkMode = E_MI_VIF_WORK_MODE_RGB_REALTIME;	// 参考mi_demo/jedi/internal/vif/st_vif.c
	//stDevAttr.eWorkMode = E_MI_VIF_WORK_MODE_RGB_FRAMEMODE;	// 参考mi_demo/jedi/internal/vif/st_vif.c
	
	if(E_MI_VIF_MODE_BT656 == stDevAttr.eIntfMode)
	{
		stDevAttr.eClkEdge = stPadInfo.unIntfAttr.stBt656Attr.eClkEdge;
	}
	else
	{
		stDevAttr.eClkEdge = E_MI_VIF_CLK_EDGE_DOUBLE;
	}
	
	if(E_MI_VIF_MODE_MIPI == stDevAttr.eIntfMode)
	{
		cout << "E_MI_VIF_MODE_MIPI == stDevAttr.eIntfMode" << endl;
		stDevAttr.eDataSeq = stPadInfo.unIntfAttr.stMipiAttr.eDataYUVOrder;
	}
	else
	{
		stDevAttr.eDataSeq = E_MI_VIF_INPUT_DATA_YUYV;
	}

	if(E_MI_VIF_MODE_BT656 == stDevAttr.eIntfMode)
	{
		memcpy(&stDevAttr.stSyncAttr, &stPadInfo.unIntfAttr.stBt656Attr.stSyncAttr, sizeof(MI_VIF_SyncAttr_t));
	}
	#endif

	// step3: 设置VIF 设备属性结构体。
	s32Ret = MI_VIF_SetDevAttr(vifDevID, &stDevAttr);
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
	//MI_S32 MI_VIF_EnableDev(MI_VIF_DEV u32VifDev);

	MI_S32 s32Ret = 0;
	s32Ret = MI_VIF_EnableDev(vifDevID);
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

	// MI_S32 MI_VIF_DisableDev(MI_VIF_DEV u32VifDev);
	MI_S32 s32Ret = 0;

	
	s32Ret = MI_VIF_DisableDev(vifDevID);
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
	s32Ret = pSensor->getPlaneInfo(Sensor::ePADId, &stPlaneInfo);
	if(0 != s32Ret)
	{
		cerr << "Fail to call pSensor->getPlaneInfo()." << endl;
		return s32Ret;
	}

	#if 0
	// step2: 获取VIF 模块属性信息。
	// step3: 根据sensor 和VIF 信息填充VIF 通道端口属性结构体。
	MI_VIF_ChnPortAttr_t stChnPortAttr;
	memset(&stChnPortAttr, 0, sizeof(MI_VIF_ChnPortAttr_t));
	
	stChnPortAttr.stCapRect.u16X = 0;
	stChnPortAttr.stCapRect.u16Y = 0;
	stChnPortAttr.stCapRect.u16Width = stPlaneInfo.stCapRect.u16Width;
	stChnPortAttr.stCapRect.u16Height = stPlaneInfo.stCapRect.u16Height;
	stChnPortAttr.stDestSize.u16Width = stPlaneInfo.stCapRect.u16Width;
	stChnPortAttr.stDestSize.u16Height = stPlaneInfo.stCapRect.u16Height;

	MI_U32 u32IsInterlace = 0;
	if(u32IsInterlace)
	{
		stChnPortAttr.eScanMode = E_MI_SYS_FRAME_SCAN_MODE_INTERLACE;
	}
	else
	{
		stChnPortAttr.eScanMode = E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE;
	}
	
	MI_SYS_PixelFormat_e ePixFormat = 
		(MI_SYS_PixelFormat_e)RGB_BAYER_PIXEL(stPlaneInfo.ePixPrecision, stPlaneInfo.eBayerId);
	stChnPortAttr.ePixFormat = ePixFormat;
	
	stChnPortAttr.eFrameRate = E_MI_VIF_FRAMERATE_FULL;

	// step4: 设置VIF 通道端口属性。
	s32Ret = MI_VIF_SetChnPortAttr(u32VifChn, u32ChnPort, &stChnPortAttr);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_VIF_SetChnPortAttr(), errno = " << s32Ret << endl;
	}
	#endif
	
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

	MI_S32 s32Ret = 0;
	MI_SYS_ChnPort_t stChnPort;
	memset(&stChnPort, 0x0, sizeof(MI_SYS_ChnPort_t));
	
	stChnPort.eModId = E_MI_MODULE_ID_VIF;
	stChnPort.u32DevId = vifDevID;
	stChnPort.u32ChnId = vifChnID;
	stChnPort.u32PortId = u32ChnPort;
	//s32Ret = MI_SYS_SetChnOutputPortDepth(&stChnPort, 0, 6);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_SYS_SetChnOutputPortDepth(), errno = " << s32Ret << endl;
	}
	
	//MI_S32 MI_VIF_EnableChnPort(MI_VIF_CHN u32VifChn, MI_VIF_PORT u32ChnPort);

	s32Ret = MI_VIF_EnableOutputPort(vifChnID, u32ChnPort);
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

	// MI_S32 MI_VIF_DisableChnPort(MI_VIF_CHN u32VifChn, MI_VIF_PORT u32ChnPort);
	MI_S32 s32Ret = 0;

	s32Ret = MI_VIF_DisableOutputPort(vifChnID, u32ChnPort);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_VIF_DisableChnPort(), errno = " << s32Ret << endl;
	}

	cout << "Call Vif::disableChnPort() end." << endl;
	return s32Ret;
}

