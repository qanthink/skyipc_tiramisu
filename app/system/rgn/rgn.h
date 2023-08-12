/*---------------------------------------------------------------- 
xxx版权所有。
作者：
时间：2020.8.12
----------------------------------------------------------------*/

/*
	OSD 功能对应MI 模块的RGN, 将位图(bitmap) 叠加到VPE 模块，送给VENC 进行编码。
	通常情况下，客户需要用OSD 显示字母、数字、符号、汉子，这些都不是位图。
	将字符转化为位图需要一系列繁琐的操作，故而MI 的同仁对RGN 进行了二次封装，
	函数以ST_OSD 的形式开头，供客户开发。
*/

/*
MI_S32 MI_RGN_Init(MI_RGN_PaletteTable_t *pstPaletteTable);
MI_S32 MI_RGN_DeInit(void);

MI_S32 MI_RGN_Create(MI_RGN_HANDLE hHandle, MI_RGN_Attr_t *pstRegion);
MI_S32 MI_RGN_Destroy (MI_RGN_HANDLE hHandle);

MI_S32 MI_RGN_AttachToChn(MI_RGN_HANDLE hHandle, MI_RGN_ChnPort_t* pstChnPort, MI_RGN_ChnPortParam_t *pstChnAttr);
MI_S32 MI_RGN_DetachFromChn(MI_RGN_HANDLE hHandle, MI_RGN_ChnPort_t *pstChnPort);

MI_S32 MI_RGN_GetCanvasInfo(MI_RGN_HANDLE hHandle, MI_RGN_CanvasInfo_t* pstCanvasInfo);
MI_S32 MI_RGN_UpdateCanvas(MI_RGN_HANDLE hHandle);

MI_S32 ST_OSD_Init(void);
MI_S32 ST_OSD_Deinit(void);
MI_S32 ST_OSD_Create(MI_RGN_HANDLE hHandle, MI_RGN_Attr_t *pstRegion);
MI_S32 ST_OSD_Destroy(MI_RGN_HANDLE hHandle);
MI_S32 ST_OSD_GetCanvasInfo(MI_RGN_HANDLE hHandle, MI_RGN_CanvasInfo_t** ppstCanvasInfo);
MI_S32 ST_OSD_Clear(MI_RGN_HANDLE hHandle, ST_Rect_T *pstRect);
MI_S32 ST_OSD_DrawText(MI_RGN_HANDLE hHandle, ST_Point_T stPoint, const char *szString, MI_U32 u32Color, DMF_Font_Size_E enSize);
MI_S32 ST_OSD_Update(MI_RGN_HANDLE hHandle);
*/

#pragma once

#include "mi_rgn.h"
#include "st_rgn.h"

class Rgn{
public:
	const static MI_RGN_HANDLE rgnHandle0 = 0;
	const static MI_RGN_HANDLE rgnHandle1 = 1;
	const static MI_RGN_HANDLE rgnHandle2 = 2;
	const static MI_RGN_HANDLE rgnHandle3 = 3;
	const static MI_RGN_HANDLE rgnHandle4 = 4;
	const static MI_RGN_HANDLE rgnHandle5 = 5;
	const static MI_RGN_HANDLE rgnHandle6 = 6;
	const static MI_RGN_HANDLE rgnHandle7 = 7;

public:
	static Rgn *getInstance();
	
	MI_S32 enable();
	MI_S32 disable();

	/*
		第一部分：使用MI_RGN_XXX 封装的函数（暂未用到）
	*/
	MI_S32 rgnInit(MI_RGN_PaletteTable_t *pstPaletteTable);
	MI_S32 rgnDeInit();

	MI_S32 rgnCreate(MI_RGN_HANDLE hHandle, MI_RGN_Attr_t *pstRegion);
	MI_S32 rgnDestroy(MI_RGN_HANDLE hHandle);

	MI_S32 rgnAttachToChn(MI_RGN_HANDLE hHandle, MI_RGN_ChnPort_t* pstChnPort, MI_RGN_ChnPortParam_t *pstChnAttr);
	MI_S32 rgnDetachFromChn(MI_RGN_HANDLE hHandle, MI_RGN_ChnPort_t *pstChnPort);

	MI_S32 rgnGetCanvasInfo(MI_RGN_HANDLE hHandle, MI_RGN_CanvasInfo_t* pstCanvasInfo);
	MI_S32 rgnUpdateCanvas(MI_RGN_HANDLE hHandle);

	/*
		第二部分：使用ST_OSD_XXX 封装的函数
	*/

	MI_S32 osdInit();
	MI_S32 osdDeinit();

	MI_U32 osdCreate_N(MI_U32 u32OsdNum);
	MI_U32 osdDestroy_N(MI_U32 u32OsdNum);

	MI_S32 osdGetCanvasInfo(MI_RGN_HANDLE hHandle);
	MI_S32 osdClear(MI_RGN_HANDLE hHandle);
	MI_S32 osdDrawText(MI_RGN_HANDLE hHandle, ST_Point_T stPoint, const char *szString, MI_U32 u32Color, DMF_Font_Size_E enSize);
	MI_S32 osdUpdate(MI_RGN_HANDLE hHandle);

	#if 0	// ST_OSD_ 常用函数
	MI_S32 ST_OSD_Init(void);
	MI_S32 ST_OSD_Deinit(void);
	MI_S32 ST_OSD_Create(MI_RGN_HANDLE hHandle, MI_RGN_Attr_t *pstRegion);
	MI_S32 ST_OSD_Destroy(MI_RGN_HANDLE hHandle);
	MI_S32 ST_OSD_GetCanvasInfo(MI_RGN_HANDLE hHandle, MI_RGN_CanvasInfo_t** ppstCanvasInfo);
	MI_S32 ST_OSD_Clear(MI_RGN_HANDLE hHandle, ST_Rect_T *pstRect);
	MI_S32 ST_OSD_DrawText(MI_RGN_HANDLE hHandle, ST_Point_T stPoint, const char *szString, MI_U32 u32Color, DMF_Font_Size_E enSize);
	MI_S32 ST_OSD_Update(MI_RGN_HANDLE hHandle);
	#endif
	
	int setText(MI_RGN_HANDLE hHandle, const char *str, MI_U32 u32Color, DMF_Font_Size_E enSize);
	
private:
	Rgn();
	~Rgn();
	Rgn(const Rgn&);
	Rgn& operator=(const Rgn&);

	bool bEnable = false;
	MI_U32 u32OsdUsedNum = 8;
	const MI_U32 u32OsdMaxNum = 8;
};

