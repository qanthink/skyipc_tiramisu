/*---------------------------------------------------------------- 
xxx版权所有。
作者：
时间：2023.6.21
----------------------------------------------------------------*/

/*
	UVC 功能，依赖于ST_UVC
	核心函数：
	int32_t ST_UVC_Init(char *uvc_name, ST_UVC_Handle_h*);
	int32_t ST_UVC_Uninit(ST_UVC_Handle_h);
	int32_t ST_UVC_CreateDev(ST_UVC_Handle_h, const ST_UVC_ChnAttr_t*);
	int32_t ST_UVC_DestroyDev(ST_UVC_Handle_h);
	int32_t ST_UVC_StartDev(ST_UVC_Handle_h);
	int32_t ST_UVC_StopDev(ST_UVC_Handle_h);
	int ST_UVC_SetTraceLevel(int level);

	重点关注：
	int32_t ST_UVC_Init(char *uvc_name, ST_UVC_Handle_h*);
	int32_t ST_UVC_CreateDev(ST_UVC_Handle_h, const ST_UVC_ChnAttr_t*);
	MI_S32 UVC_MM_FillBuffer(void *uvc, ST_UVC_BufInfo_t *bufInfo)
*/

#pragma once

#include "uvc.h"
#include "st_uvc.h"

typedef struct{
	MI_U32 fcc;
	MI_U32 u32Width;
	MI_U32 u32Height;
	MI_U32 u32FrameRate;
}ST_UvcSetting_Attr_T;

typedef struct{
	char name[16];
	bool bForceIdr;
	MI_U8 u8MaxBufCnt;
	ST_UVC_Handle_h handle;
	ST_UvcSetting_Attr_T setting;
}ST_UvcDev_t;

class UvcUac{
public:
	static UvcUac *getInstance();

	int startUvc();
	int stopUvc();

private:
	UvcUac();
	~UvcUac();
	UvcUac(const UvcUac&);
	UvcUac& operator=(const UvcUac&);

	ST_UvcDev_t stUvcDev;
	const static MI_U8 u8MaxBufCnt = 3;
};

