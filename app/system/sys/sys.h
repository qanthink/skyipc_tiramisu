/*---------------------------------------------------------------- 
xxx版权所有。
作者：
时间：2020.7.10
----------------------------------------------------------------*/

#pragma once

#include "mi_sys.h"

class Sys{
public:
	static Sys *getInstance();

	MI_S32 bindVif2Vpe(MI_U32 u32VifPortID, MI_U32 u32VpePortID, MI_U32 u32SrcFrmrate, MI_U32 u32DstFrmrate, MI_SYS_BindType_e eBindType, MI_U32 u32BindParam);
	MI_S32 bindVpe2Venc(MI_U32 u32VpePortID, MI_U32 u32VencChnID, MI_U32 u32SrcFrmrate, MI_U32 u32DstFrmrate, MI_SYS_BindType_e eBindType, MI_U32 u32BindParam);	
	
private:
	Sys();
	~Sys();
	Sys(const Sys&);
	Sys& operator=(const Sys&);
};

