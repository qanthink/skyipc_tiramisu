/*---------------------------------------------------------------- 
xxx版权所有。
作者：
时间：2020.7.10
----------------------------------------------------------------*/

#include "sys.h"
#include "vpe.h"
#include <iostream>
#include <string.h>

using namespace std;

Sys::Sys()
{
	MI_SYS_Init(0);
}

Sys::~Sys()
{
	MI_SYS_Exit(0);
}

/*-----------------------------------------------------------------------------
描--述：SYS 模块获取实例的唯一接口函数。
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
Sys* Sys::getInstance()
{
	static Sys sys;
	return &sys;
}

/*-----------------------------------------------------------------------------
描--述：绑定VIF 到VPE
参--数：u32VifPortID VIF端口号，一般取0; u32VpeChnID VPE通道号，一般取0; u32SrcFrmrate 源帧率；
		u32DstFrmrate 目标帧率；eBindType 绑定类型，用realtime.
返回值：成功，返回0; 失败，返回错误码。
注--意：
-----------------------------------------------------------------------------*/
MI_S32 Sys::bindVif2Vpe(MI_U32 u32VifPortID, MI_U32 u32VpeChnID, MI_U32 u32SrcFrmrate, MI_U32 u32DstFrmrate, MI_SYS_BindType_e eBindType, MI_U32 u32BindParam)
{
	cout << "Call Sys::bindVif2Vpe()." << endl;

	MI_SYS_ChnPort_t stSrcChnPort;
	memset(&stSrcChnPort, 0, sizeof(MI_SYS_ChnPort_t));
	stSrcChnPort.eModId = E_MI_MODULE_ID_VIF;
	stSrcChnPort.u32DevId = 0;
	stSrcChnPort.u32ChnId = 0;
	stSrcChnPort.u32PortId = u32VifPortID;

	MI_SYS_ChnPort_t stDstChnPort;
	memset(&stDstChnPort, 0, sizeof(MI_SYS_ChnPort_t));
	stDstChnPort.eModId = E_MI_MODULE_ID_VPE;
	stSrcChnPort.u32DevId = 0;
	stSrcChnPort.u32ChnId = u32VpeChnID;
	stSrcChnPort.u32PortId = 0;
	
	//MI_S32 MI_SYS_BindChnPort(MI_SYS_ChnPort_t *pstSrcChnPort, MI_SYS_ChnPort_t *pstDstChnPort, MI_U32 u32SrcFrmrate,  MI_U32 u32DstFrmrate);
	MI_S32 s32Ret = 0;
	MI_U16 u16SocId;
	s32Ret = MI_SYS_BindChnPort2(u16SocId, &stSrcChnPort, &stDstChnPort, u32SrcFrmrate, u32DstFrmrate, eBindType, u32BindParam);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_SYS_BindChnPort2(), errno = " << s32Ret << endl;
	}

	cout << "Call Sys::bindVif2Vpe() end." << endl;
	return s32Ret;
}

/*-----------------------------------------------------------------------------
描--述：绑定VPE 到VENC
参--数：u32VpePortID VPE端口号; u32VencChnID VENC通道号；u32SrcFrmrate 源帧率；u32DstFrmrate 目标帧率。
返回值：成功，返回0; 失败，返回错误码。
注--意：
-----------------------------------------------------------------------------*/
MI_S32 Sys::bindVpe2Venc(MI_U32 u32VpePortID, MI_U32 u32VencChnID, MI_U32 u32SrcFrmrate,  MI_U32 u32DstFrmrate, MI_SYS_BindType_e eBindType, MI_U32 u32BindParam)
{
	cout << "Call Sys::bindVpe2Venc()." << endl;

	MI_SYS_ChnPort_t stSrcChnPort;
	memset(&stSrcChnPort, 0, sizeof(MI_SYS_ChnPort_t));
	stSrcChnPort.eModId = E_MI_MODULE_ID_VPE;
	stSrcChnPort.u32DevId = 0;
	stSrcChnPort.u32ChnId = 0;
	stSrcChnPort.u32PortId = u32VpePortID;

	MI_SYS_ChnPort_t stDstChnPort;
	memset(&stDstChnPort, 0, sizeof(MI_SYS_ChnPort_t));
	stDstChnPort.eModId = E_MI_MODULE_ID_VENC;
	if(Vpe::vpeJpegPort == u32VpePortID)
	{
		stDstChnPort.u32DevId = 1;
	}
	else
	{
		stDstChnPort.u32DevId = 0;
	}
	stDstChnPort.u32ChnId = u32VencChnID;
	stDstChnPort.u32PortId = 0;

	// MI_S32 MI_SYS_BindChnPort(MI_SYS_ChnPort_t *pstSrcChnPort, MI_SYS_ChnPort_t *pstDstChnPort, MI_U32 u32SrcFrmrate,  MI_U32 u32DstFrmrate);
	MI_S32 s32Ret = 0;
	MI_U16 u16SocId;
	s32Ret = MI_SYS_BindChnPort2(u16SocId, &stSrcChnPort, &stDstChnPort, u32SrcFrmrate, u32DstFrmrate, eBindType, u32BindParam);
	//s32Ret = MI_SYS_BindChnPort(&stSrcChnPort, &stDstChnPort, u32SrcFrmrate, u32DstFrmrate);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_SYS_BindChnPort2(), errno = " << s32Ret << endl;
	}

	cout << "Call Sys::bindVpe2Venc() end." << endl;
	return s32Ret;
}

