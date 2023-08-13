/*---------------------------------------------------------------- 
xxx版权所有。
作者：
时间：2020.7.10
----------------------------------------------------------------*/

#include "scl.h"
#include "isp.h"
#include "sensor.h"
#include <iostream>
#include <string.h>

using namespace std;

Scl::Scl()
{
	enable();
}

Scl::~Scl()
{
	disable();
	bEnable = false;
}

/*-----------------------------------------------------------------------------
描--述：SCL 模块获取实例的唯一入口函数。
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
Scl* Scl::getInstance()
{
	static Scl scl;
	return &scl;
}

/*-----------------------------------------------------------------------------
描--述：
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
MI_S32 Scl::enable()
{
	cout << "Call Scl::enable()." << endl;
	
	if(bEnable)
	{
		cerr << "Fail to call Scl::enable(). Modules is already been enabled!" << endl;
		return 0;
	}

	MI_S32 s32Ret = 0;
	MI_SCL_DevAttr_t stSclDevAttr;
	memset(&stSclDevAttr, 0x0, sizeof(MI_SCL_DevAttr_t));
	
	//Port0->HWSCL2, //Port1->HWSCL3, //Port2->HWSCL4
	stSclDevAttr.u32NeedUseHWOutPortMask = E_MI_SCL_HWSCL2 | E_MI_SCL_HWSCL3 | E_MI_SCL_HWSCL4;
	
	s32Ret = MI_SCL_CreateDevice(sclDevId, &stSclDevAttr);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_SCL_CreateDevice() in Scl::enable(). " 
			<< "errno = 0x" << hex << s32Ret << dec << endl;
		return s32Ret;
	}
	
	MI_SCL_ChannelAttr_t stSclChnAttr;
	memset(&stSclChnAttr, 0x0, sizeof(MI_SCL_ChannelAttr_t));
	s32Ret = MI_SCL_CreateChannel(sclDevId, sclChnId, &stSclChnAttr);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_SCL_CreateChannel() in Scl::enable(). " 
			<< "errno = 0x" << hex << s32Ret << dec << endl;
		return s32Ret;
	}

	MI_SCL_ChnParam_t stSclChnParam;
	memset(&stSclChnParam, 0x0, sizeof(MI_SCL_ChnParam_t));
	s32Ret = MI_SCL_SetChnParam(sclDevId, sclChnId, &stSclChnParam);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_SCL_SetChnParam() in Scl::enable(). " 
			<< "errno = 0x" << hex << s32Ret << dec << endl;
		return s32Ret;
	}
	
	s32Ret = MI_SCL_StartChannel(sclDevId, sclChnId);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_SCL_StartChannel() in Scl::enable(). " 
			<< "errno = 0x" << hex << s32Ret << dec << endl;
		return s32Ret;
	}
	
	bEnable = true;

	cout << "Call Scl::enable() end." << endl;
	return 0;	
}

/*-----------------------------------------------------------------------------
描--述：
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
MI_S32 Scl::disable()
{
	cout << "Call Scl::disable()." << endl;

	//disableChnPort(vifPort);
	bEnable = false;

	cout << "Call Scl::disable() end." << endl;
	return 0;
}

/*-----------------------------------------------------------------------------
描--述：创建端口。
参--数：sclPortId, 端口号，tiramisu 平台最多6个端口；
		widthOut, heightOut, 图像输出宽高。
返回值：
注--意：
-----------------------------------------------------------------------------*/
int Scl::createPort(MI_SCL_PORT sclPortId, unsigned int widthOut, unsigned int heightOut, bool bIsJpeg)
{
	cout << "Call Scl::createPort()." << endl;

	MI_S32 s32Ret = 0;
	#if 1
	MI_ISP_OutPortParam_t  stIspOutputParam;
	memset(&stIspOutputParam, 0x0, sizeof(MI_ISP_OutPortParam_t));
	s32Ret = MI_ISP_GetInputPortCrop(Isp::ispDevId, Isp::ispChnId, &stIspOutputParam.stCropRect);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_ISP_GetInputPortCrop() in Scl::createPort(). " 
			<< "errno = 0x" << hex << s32Ret << dec << endl;
		return s32Ret;
	}

	MI_SCL_OutPortParam_t  stSclOutputParam;
	memset(&stSclOutputParam, 0x0, sizeof(MI_SCL_OutPortParam_t));
	stSclOutputParam.stSCLOutCropRect.u16X = stIspOutputParam.stCropRect.u16X;
	stSclOutputParam.stSCLOutCropRect.u16Y = stIspOutputParam.stCropRect.u16Y;
	stSclOutputParam.stSCLOutCropRect.u16Width = stIspOutputParam.stCropRect.u16Width;		// 如果没剪裁，则值为0.
	stSclOutputParam.stSCLOutCropRect.u16Height = stIspOutputParam.stCropRect.u16Height;
	stSclOutputParam.stSCLOutputSize.u16Width = widthOut;
	stSclOutputParam.stSCLOutputSize.u16Height = heightOut;
	#else
	MI_SCL_OutPortParam_t  stSclOutputParam;
	memset(&stSclOutputParam, 0x0, sizeof(MI_SCL_OutPortParam_t));
	stSclOutputParam.stSCLOutCropRect.u16X = 0;
	stSclOutputParam.stSCLOutCropRect.u16Y = 0;
	stSclOutputParam.stSCLOutCropRect.u16Width = 0;		// 如果没剪裁，则值为0.
	stSclOutputParam.stSCLOutCropRect.u16Height = 0;
	stSclOutputParam.stSCLOutputSize.u16Width = widthOut;
	stSclOutputParam.stSCLOutputSize.u16Height = heightOut;
	#endif
	stSclOutputParam.bMirror = FALSE;
	stSclOutputParam.bFlip = FALSE;
	stSclOutputParam.eCompressMode= E_MI_SYS_COMPRESS_MODE_NONE;
	// JPEG 需要YUV422, H.26X 需要YUV420.
	//stSclOutputParam.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;
	//stSclOutputParam.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
	if(bIsJpeg)
	{
		stSclOutputParam.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;
	}
	else
	{
		stSclOutputParam.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
	}
	
	s32Ret = MI_SCL_SetOutputPortParam(sclDevId, sclChnId, sclPortId, &stSclOutputParam);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_SCL_SetOutputPortParam() in Scl::createPort(). " 
			<< "errno = 0x" << hex << s32Ret << dec << endl;
		return s32Ret;
	}

	s32Ret = MI_SCL_EnableOutputPort(sclDevId, sclChnId, sclPortId);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_SCL_EnableOutputPort() in Scl::createPort(). " 
			<< "errno = 0x" << hex << s32Ret << dec << endl;
		return s32Ret;
	}
	
	cout << "Call Scl::createPort() end." << endl;
	return 0;
}

/*-----------------------------------------------------------------------------
描--述：销毁端口
参--数：sclPortId, 端口号，tiramisu 平台最多6个端口；
返回值：
注--意：
-----------------------------------------------------------------------------*/
int Scl::destoryPort(MI_SCL_PORT sclPortId)
{
	cout << "Call Scl::destoryPort()." << endl;
	cout << "Call Scl::destoryPort() end." << endl;
	return 0;
}

