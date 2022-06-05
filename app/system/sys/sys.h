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

	MI_S32 init(void);
	MI_S32 exit(void);
	MI_S32 bindChnPort(MI_SYS_ChnPort_t *pstSrcChnPort, MI_SYS_ChnPort_t *pstDstChnPort, MI_U32 u32SrcFrmrate, MI_U32 u32DstFrmrate);
	MI_S32 bindChnPort2(MI_SYS_ChnPort_t *pstSrcChnPort, MI_SYS_ChnPort_t *pstDstChnPort, MI_U32 u32SrcFrmrate, MI_U32 u32DstFrmrate, \
							MI_SYS_BindType_e eBindType, MI_U32 u32BindParam);
	MI_S32 unBindChnPort(MI_SYS_ChnPort_t *pstSrcChnPort, MI_SYS_ChnPort_t *pstDstChnPort);
	MI_S32 bindVif2Vpe(MI_U32 u32VifPortID, MI_U32 u32VpePortID, MI_U32 u32SrcFrmrate, MI_U32 u32DstFrmrate, MI_SYS_BindType_e eBindType, MI_U32 u32BindParam);
	MI_S32 bindVpe2Venc(MI_U32 u32VpePortID, MI_U32 u32VencChnID, MI_U32 u32SrcFrmrate, MI_U32 u32DstFrmrate, MI_SYS_BindType_e eBindType, MI_U32 u32BindParam);
	
	MI_S32 bindVpe2Divp(MI_U32 u32VpePortID, MI_U32 u32DivpChnID, MI_U32 u32SrcFrmrate,  MI_U32 u32DstFrmrate, MI_SYS_BindType_e eBindType, MI_U32 u32BindParam);
	MI_S32 bindDivp2Venc(MI_U32 u32DivpPortID, MI_U32 u32VencChnID, MI_U32 u32SrcFrmrate,  MI_U32 u32DstFrmrate, MI_SYS_BindType_e eBindType, MI_U32 u32BindParam);
	
	MI_S32 bindVdecChn2DispPort(MI_U32 u32SrcChnID, MI_U32 u32DstPortID);
	MI_S32 bindVdecChn2DispPort_N(MI_U32 u32PortNum);

	MI_S32 bindVdecChn2DivpChn(MI_U32 u32SrcChnID, MI_U32 u32DstChnID);
	MI_S32 bindVdecChn2DivpChn_N(MI_U32 u32ChnNum);

	MI_S32 bindDivpChn2DispPort(MI_U32 u32SrcChnID, MI_U32 u32DstPortID);
	MI_S32 bindDivpChn2DispPort_N(MI_U32 u32PortNum);
	
private:
	Sys();
	~Sys();
	Sys(const Sys&);
	Sys& operator=(const Sys&);
};

