/*---------------------------------------------------------------- 
xxx版权所有。
作者：
时间：2020.7.10
----------------------------------------------------------------*/

#include "rgn.h"
#include "vpe.h"
#include <string.h>
#include <iostream>

using namespace std;

Rgn::Rgn()
{
	enable();

	bEnable = true;
}

Rgn::~Rgn()
{
	bEnable = false;
	
	disable();
}

/*  ---------------------------------------------------------------------------
描--述：VENC 模块获取实例的唯一入口
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
Rgn* Rgn::getInstance()
{
	static Rgn rgn;
	return &rgn;
}

/* ---------------------------------------------------------------------------
描--述：
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
MI_S32 Rgn::enable()
{
	cout << "Call Rgn::enable()." << endl;

	osdInit();
	osdCreate_N(u32OsdUsedNum);

	bEnable = true;	
	cout << "Call Rgn::enable() end." << endl;
	return 0;
}

/*-----------------------------------------------------------------------------
描--述：
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
MI_S32 Rgn::disable()
{
	cout << "Call Rgn::disable()." << endl;
	bEnable = false;

	osdDestroy_N(u32OsdUsedNum);
	osdDeinit();
	
	cout << "Call Rgn::disable() end." << endl;
	return 0;
}

/*

					第一部分：使用MI_RGN_XXX 封装的函数（暂未用到）

*/

/*-----------------------------------------------------------------------------
描--述：RGN 初始化
参--数：pstPaletteTable 指向调色板的结构体指针。
返回值：成功，返回MI_RGN_OK; 已经初始化，返回MI_ERR_RGN_BUSY
注--意：I2/I4/I8 的图像格式共用一份palette table, palette table只能在初始化时做一次，不可再次设置。
		RGB格式不会参考palette.
		Palette的第0号成员为透明色，上层无法指定。
-----------------------------------------------------------------------------*/
MI_S32 Rgn::rgnInit(MI_RGN_PaletteTable_t *pstPaletteTable)
{
	cout << "Call Rgn::rgnInit()." << endl;

	MI_RGN_PaletteTable_t stPaletteTable;
	if(NULL == pstPaletteTable)
	{
		cerr << "Call Rgn::rgnInit() with null pointer. Use default argument!" << endl;
		pstPaletteTable = &stPaletteTable;
		memset(&stPaletteTable, 0, sizeof(MI_RGN_PaletteTable_t));

		stPaletteTable.astElement[1].u8Alpha = 255;		// 红
		stPaletteTable.astElement[1].u8Red = 255;
		stPaletteTable.astElement[1].u8Green = 0;
		stPaletteTable.astElement[1].u8Blue = 0;
		stPaletteTable.astElement[2].u8Alpha = 255;		// 绿
		stPaletteTable.astElement[2].u8Red = 0;
		stPaletteTable.astElement[2].u8Green = 255;
		stPaletteTable.astElement[2].u8Blue = 0;
		stPaletteTable.astElement[3].u8Alpha = 255;		// 蓝
		stPaletteTable.astElement[3].u8Red = 0;
		stPaletteTable.astElement[3].u8Green = 0;
		stPaletteTable.astElement[3].u8Blue = 255;
	}

	MI_S32 s32Ret = 0;
	MI_U16 u16SocId;
	s32Ret = MI_RGN_Init(u16SocId, pstPaletteTable);
	switch(s32Ret)
	{
		case MI_RGN_OK:
			break;
		case MI_ERR_RGN_BUSY:
			cerr << "Fail to call Rgn::rgnInit(), modules is already initialized!" << endl;
			break;
		default:
			cerr << "Fail to call Rgn::rgnInit(), errno = " << errno << endl;
			break;
	}

	cout << "Call Rgn::rgnInit() end." << endl;
	return s32Ret;
}

/*-----------------------------------------------------------------------------
描--述：RGN 反初始化。
参--数：无
返回值：成功，返回MI_RGN_OK; 未初始化，返回MI_ERR_RGN_BUSY
注--意：
-----------------------------------------------------------------------------*/
MI_S32 Rgn::rgnDeInit()
{
	cout << "Call Rgn::rgnDeInit()." << endl;
	
	MI_S32 s32Ret = 0;
	MI_U16 u16SocId;
	s32Ret = MI_RGN_DeInit(u16SocId);
	switch(s32Ret)
	{
		case MI_RGN_OK:
			break;
		case MI_ERR_RGN_BUSY:
			cerr << "Fail to call Rgn::rgnDeInit(), modules is already initialized!" << endl;
			break;
		default:
			cerr << "Fail to call Rgn::rgnDeInit(), errno = " << errno << endl;
			break;
	}
	
	cout << "Call Rgn::rgnDeInit() end." << endl;
	return s32Ret;
}

/*-----------------------------------------------------------------------------
描--述：RGN 创建区域。
参--数：hHandle 区域句柄号，[0, MI_RGN_MAX_HANDLE]; pstRegion 指向区域属性的结构体指针。
取值范围：[0, MI_RGN_MAX_HANDLE]。
返回值：成功，返回MI_RGN_OK; 失败，返回错误码；参数无效，返回-1;
注--意：hHandle 必须是未使用的hHandle号，不支持重复创建。
-----------------------------------------------------------------------------*/
MI_S32 Rgn::rgnCreate(MI_RGN_HANDLE hHandle, MI_RGN_Attr_t *pstRegion)
{
	cout << "Call Rgn::rgnCreate()." << endl;

	if(hHandle > MI_RGN_MAX_HANDLE)
	{
		cerr << "Fail to call Rgn::rgnCreate(). Argument hHandle is out of range!" << endl;
		return -1;
	}

	if(NULL == pstRegion)
	{
		cerr << "Fail to call Rgn::rgnCreate(). Argument has null value!" << endl;
		return -1;
	}
	
	MI_S32 s32Ret = 0;
	MI_U16 u16SocId;
	s32Ret = MI_RGN_Create(u16SocId, hHandle, pstRegion);
	if(MI_RGN_OK != s32Ret)
	{
		cerr << "Fail to call Rgn::rgnCreate(), errno = " << s32Ret << endl;
	}
	
	cout << "Call Rgn::rgnCreate() end." << endl;
	return s32Ret;
}

/*-----------------------------------------------------------------------------
描--述：RGN 销毁区域。
参--数：hHandle 区域句柄号。
返回值：成功，返回MI_RGN_OK; 失败，返回错误码；参数无效，返回-1;
注--意：区域必须已创建。
-----------------------------------------------------------------------------*/
MI_S32 Rgn::rgnDestroy(MI_RGN_HANDLE hHandle)
{
	cout << "Call Rgn::rgnDestroy()." << endl;

	if(hHandle > MI_RGN_MAX_HANDLE)
	{
		cerr << "Fail to call Rgn::rgnDestroy(). Argument hHandle is out of range!" << endl;
		return -1;
	}
		
	MI_S32 s32Ret = 0;
	MI_U16 u16SocId;
	s32Ret = MI_RGN_Destroy(u16SocId, hHandle);
	if(MI_RGN_OK != s32Ret)
	{
		cerr << "Fail to call Rgn::rgnDestroy(), errno = " << s32Ret << endl;
	}
	
	cout << "Call Rgn::rgnDestroy() end." << endl;
	return s32Ret;
}

/*-----------------------------------------------------------------------------
描--述：将区域叠加到通道上。
参--数：hHandle 区域句柄号；pstChnPort 通道端口结构体指针；pstChnAttr 区域通道显示属性指针。
返回值：成功，返回MI_RGN_OK; 失败，返回错误码；参数无效，返回-1;
注--意：多个osd区域类叠加到同一个通道上，每个osd必须是同一个图像格式。
-----------------------------------------------------------------------------*/
MI_S32 Rgn::rgnAttachToChn(MI_RGN_HANDLE hHandle, MI_RGN_ChnPort_t* pstChnPort, MI_RGN_ChnPortParam_t *pstChnAttr)
{
	cout << "Call Rgn::rgnAttachToChn()." << endl;

	if(hHandle > MI_RGN_MAX_HANDLE)
	{
		cerr << "Fail to call Rgn::rgnAttachToChn(). Argument hHandle is out of range!" << endl;
		return -1;
	}

	if(NULL == pstChnPort || NULL == pstChnAttr)
	{
		cerr << "Fail to call Rgn::rgnAttachToChn(). Argument has null value!" << endl;
		return -1;
	}
		
	MI_S32 s32Ret = 0;
	MI_U16 u16SocId;
	s32Ret = MI_RGN_AttachToChn(u16SocId, hHandle, pstChnPort, pstChnAttr);
	if(MI_RGN_OK != s32Ret)
	{
		cerr << "Fail to call Rgn::rgnAttachToChn(), errno = " << s32Ret << endl;
	}
	
	cout << "Call Rgn::rgnAttachToChn() end." << endl;
	return s32Ret;
}

/*-----------------------------------------------------------------------------
描--述：将区域从通道中撤出。
参--数：hHandle 区域句柄号；pstChnPort 指向通道端口结构体的指针。
返回值：成功，返回MI_RGN_OK; 失败，返回错误码；参数无效，返回-1;
注--意：
-----------------------------------------------------------------------------*/
MI_S32 Rgn::rgnDetachFromChn(MI_RGN_HANDLE hHandle, MI_RGN_ChnPort_t *pstChnPort)
{
	cout << "Call Rgn::disable()." << endl;

	if(hHandle > MI_RGN_MAX_HANDLE)
	{
		cerr << "Fail to call Rgn::rgnDetachFromChn(). Argument hHandle is out of range!" << endl;
		return -1;
	}

	if(NULL == pstChnPort)
	{
		cerr << "Fail to call Rgn::rgnDetachFromChn(). Argument has null value!" << endl;
		return -1;
	}
		
	MI_S32 s32Ret = 0;
	MI_U16 u16SocId;
	s32Ret = MI_RGN_DetachFromChn(u16SocId, hHandle, pstChnPort);
	if(MI_RGN_OK != s32Ret)
	{
		cerr << "Fail to call Rgn::rgnDetachFromChn(), errno = " << s32Ret << endl;
	}
	
	cout << "Call Rgn::rgnDetachFromChn() end." << endl;
	return s32Ret;
}

/*-----------------------------------------------------------------------------
描--述：获取区域的显示画布信息。
参--数：hHandle 区域句柄号；pstCanvasInfo 指向画布信息的结构体指针。
返回值：成功，返回MI_RGN_OK; 失败，返回错误码；参数无效，返回-1;
注--意：
-----------------------------------------------------------------------------*/
MI_S32 Rgn::rgnGetCanvasInfo(MI_RGN_HANDLE hHandle, MI_RGN_CanvasInfo_t* pstCanvasInfo)
{
	cout << "Call Rgn::rgnGetCanvasInfo()." << endl;

	if(hHandle > MI_RGN_MAX_HANDLE)
	{
		cerr << "Fail to call Rgn::rgnGetCanvasInfo(). Argument hHandle is out of range!" << endl;
		return -1;
	}

	if(NULL == pstCanvasInfo)
	{
		cerr << "Fail to call Rgn::rgnGetCanvasInfo(). Argument has null value!" << endl;
		return -1;
	}
	
	MI_S32 s32Ret = 0;
	MI_U16 u16SocId;
	s32Ret = MI_RGN_GetCanvasInfo(u16SocId, hHandle, pstCanvasInfo);
	if(MI_RGN_OK != s32Ret)
	{
		cerr << "Fail to call Rgn::rgnGetCanvasInfo(), errno = " << s32Ret << endl;
	}
	
	cout << "Call Rgn::rgnGetCanvasInfo() end." << endl;
	return s32Ret;
}

/*-----------------------------------------------------------------------------
描--述：更新画布。
参--数：hHandle 区域句柄号；
返回值：成功，返回MI_RGN_OK; 失败，返回错误码；参数无效，返回-1;
注--意：1.若画布没有叠加到通道，则需要先执行叠加操作，然后才会在通道上显示出画布的内容。
	2.本接口必须与MI_RGN_GetCanvasInfo 成对调用：获取画布信息 -> 画图 -> 更新画布。
		若遗漏了MI_RGN_GetCanvasInfo(), 本函数返回MI_ERR_RGN_NOT_PERM 
	3.多个线程同时调用该接口可能出现OSD闪烁或不刷新的问题，应对策略请参考MI 手册。	
-----------------------------------------------------------------------------*/
MI_S32 Rgn::rgnUpdateCanvas(MI_RGN_HANDLE hHandle)
{
	cout << "Call Rgn::rgnUpdateCanvas()." << endl;
	
	if(hHandle > MI_RGN_MAX_HANDLE)
	{
		cerr << "Fail to call Rgn::rgnUpdateCanvas(). Argument hHandle is out of range!" << endl;
		return -1;
	}
	
	MI_S32 s32Ret = 0;
	MI_U16 u16SocId;
	s32Ret = MI_RGN_UpdateCanvas(u16SocId, hHandle);
	if(MI_RGN_OK != s32Ret)
	{
		cerr << "Fail to call Rgn::rgnUpdateCanvas(), errno = " << s32Ret << endl;
	}
	
	cout << "Call Rgn::rgnUpdateCanvas() end." << endl;
	return s32Ret;
}

/*

					第二部分：使用ST_OSD_XXX 封装的函数

*/

/*-----------------------------------------------------------------------------
描--述：OSD 初始化
参--数：无
返回值：成功，返回MI_RGN_OK; 失败，返回错误码;
注--意：
-----------------------------------------------------------------------------*/
MI_S32 Rgn::osdInit()
{
	cout << "Call Rgn::osdInit()." << endl;

	MI_S32 s32Ret = 0;
	s32Ret = ST_OSD_Init();
	if(MI_RGN_OK != s32Ret)
	{
		cerr << "Fail to call Rgn::osdInit(), errno = " << s32Ret << endl;
	}
	
	cout << "Call Rgn::osdInit() end." << endl;
	return s32Ret;
}

/*-----------------------------------------------------------------------------
描--述：OSD 反初始化
参--数：无
返回值：成功，返回MI_RGN_OK; 失败，返回错误码;
注--意：
-----------------------------------------------------------------------------*/
MI_S32 Rgn::osdDeinit()
{
	cout << "Call Rgn::osdDeinit()." << endl;

	MI_S32 s32Ret = 0;
	s32Ret = ST_OSD_Deinit();
	if(MI_RGN_OK != s32Ret)
	{
		cerr << "Fail to call Rgn::osdDeinit(), errno = " << s32Ret << endl;
	}
	
	cout << "Call Rgn::osdDeinit() end." << endl;
	return s32Ret;
}

/*-----------------------------------------------------------------------------
描--述：创建若干个OSD
参--数：u32OsdUsedNum OSD数量
返回值：返回成功创建的OSD 的数量;
注--意：
-----------------------------------------------------------------------------*/
MI_U32 Rgn::osdCreate_N(MI_U32 u32OsdUsedNum)
{
	cout << "Call Rgn::osdCreate_N()." << endl;

	const MI_U16 u16Sx = 100;			// 若干个OSD 的起点坐标
	const MI_U16 u16Sy = 100;
	const MI_U16 u16Width = 960;		// 每个OSD 的宽高
	const MI_U16 u16Height = 64;
	const MI_U16 u16IntervalX = 10;		// OSD 之间的间隔
	const MI_U16 u16IntervalY = 0;

	// OSD 区域属性，用于设置宽高、像素格式。
	MI_RGN_Attr_t stRgnAttr;
	memset(&stRgnAttr, 0, sizeof(MI_RGN_Attr_t));
	stRgnAttr.eType = E_MI_RGN_TYPE_OSD;    // 有cover和osd两种，文字或者贴图选择OSD
	stRgnAttr.stOsdInitParam.ePixelFmt = E_MI_RGN_PIXEL_FORMAT_I4;
	stRgnAttr.stOsdInitParam.stSize.u32Width = u16Width;	// OSD区域的宽高
	stRgnAttr.stOsdInitParam.stSize.u32Height = u16Height;

	// OSD 端口属性，用于选择绑定的模块，默认绑定VPE.
	MI_RGN_ChnPort_t stChnPort;
	memset(&stChnPort, 0, sizeof(MI_RGN_ChnPort_t));
	stChnPort.eModId = E_MI_RGN_MODID_VPE;
	stChnPort.s32DevId = 0;
	stChnPort.s32ChnId = 0;
	stChnPort.s32OutputPortId = Vpe::vpeMainPort;	// 将OSD 绑定到主码流

	// OSD 通道属性用于设置起始位置等信息。
	MI_RGN_ChnPortParam_t stChnAttr;
	memset(&stChnAttr, 0, sizeof(MI_RGN_ChnPortParam_t));
	stChnAttr.bShow = true;
	stChnAttr.unPara.stOsdChnPort.u32Layer = 3;		// OSD区域层次，数值越低越在底层，越会被其它OSD被遮挡
	stChnAttr.unPara.stOsdChnPort.stOsdAlphaAttr.eAlphaMode = E_MI_RGN_PIXEL_ALPHA;
	stChnAttr.unPara.stOsdChnPort.stOsdAlphaAttr.stAlphaPara.u8ConstantAlpha = 0xFF;
	
	int i = 0;
	MI_U32 u32CreatedCnt = 0;
	for(i = 0; i < u32OsdUsedNum; ++i)
	{
		MI_S32 s32Ret = 0;
		s32Ret = ST_OSD_Create(i, &stRgnAttr);
		if(MI_RGN_OK != s32Ret)
		{
			cerr << "Fail to call Rgn::ST_OSD_Create(), errno = " << s32Ret << endl;
			continue;
		}

		// 设置起始位置。
		//stChnAttr.stPoint.u32X = u16Sx + (u16IntervalX + u16Width) * i;
		stChnAttr.stPoint.u32X = u16Sx;
		stChnAttr.stPoint.u32Y = u16Sy + (u16IntervalY + u16Height) * i;
		s32Ret = rgnAttachToChn(i, &stChnPort, &stChnAttr);
		if(MI_RGN_OK != s32Ret)
		{
			cerr << "Fail to call Rgn::rgnAttachToChn(), errno = " << s32Ret << endl;
			continue;
		}

		++u32CreatedCnt;
	}
	
	cout << "Call Rgn::osdCreate_N() end. Success to create " << u32CreatedCnt << "OSD." << endl;
	return u32CreatedCnt;
}

/*-----------------------------------------------------------------------------
描--述：销毁若干个OSD
参--数：u32OsdUsedNum OSD数量
返回值：返回成功销毁的OSD 的数量;
注--意：
-----------------------------------------------------------------------------*/
MI_U32 Rgn::osdDestroy_N(MI_U32 u32OsdUsedNum)
{
	cout << "Call Rgn::osdInit()." << endl;

	// OSD 端口
	MI_RGN_ChnPort_t stChnPort;
	memset(&stChnPort, 0, sizeof(MI_RGN_ChnPort_t));
	stChnPort.eModId = E_MI_RGN_MODID_VPE;
	stChnPort.s32DevId = 0;
	stChnPort.s32ChnId = 0;
	stChnPort.s32OutputPortId = Vpe::vpeMainPort;	// 将OSD 绑定到主码流
	
	int i = 0;
	MI_U32 u32DestroiedCnt = 0;
	for(i = 0; i < u32OsdUsedNum; ++i)
	{
		MI_S32 s32Ret = 0;
		s32Ret = rgnDetachFromChn(i, &stChnPort);
		if(MI_RGN_OK != s32Ret)
		{
			cerr << "Fail to call Rgn::rgnDetachFromChn(), errno = " << s32Ret << endl;
		}

		s32Ret = ST_OSD_Destroy(i);
		if(MI_RGN_OK != s32Ret)
		{
			cerr << "Fail to call Rgn::ST_OSD_Destroy(), errno = " << s32Ret << endl;
		}

		++u32DestroiedCnt;
	}
	
	cout << "Call Rgn::osdDestroy_N() end. Success to destroy " << u32DestroiedCnt << "OSD." << endl;
	return u32DestroiedCnt;
}

/*-----------------------------------------------------------------------------
描--述：获取画布信息。
参--数：hHandle 区域句柄号。
返回值：成功，返回MI_RGN_OK; 失败，返回错误码；参数无效，返回-1;
注--意：这个函数必须调用，与ST_OSD_XXX 函数配合使用即可。如果只想单独获取画布信息，请参考rgnGetCanvasInfo()
-----------------------------------------------------------------------------*/
MI_S32 Rgn::osdGetCanvasInfo(MI_RGN_HANDLE hHandle)
{
	//cout << "Call Rgn::osdGetCanvasInfo()." << endl;

	MI_S32 s32Ret = 0;
	MI_RGN_CanvasInfo_t *pstCanvasInfo = NULL;
	s32Ret = ST_OSD_GetCanvasInfo(hHandle, &pstCanvasInfo);
	if(MI_RGN_OK != s32Ret)
	{
		cerr << "Fail to call Rgn::osdGetCanvasInfo(), errno = " << s32Ret << endl;
	}
	
	//cout << "Call Rgn::osdGetCanvasInfo() end." << endl;
	return s32Ret;
}

/*-----------------------------------------------------------------------------
描--述：清除OSD
参--数：hHandle 区域句柄号。
返回值：成功，返回MI_RGN_OK; 失败，返回错误码；参数无效，返回-1;
注--意：
-----------------------------------------------------------------------------*/
MI_S32 Rgn::osdClear(MI_RGN_HANDLE hHandle)
{
	//cout << "Call Rgn::osdClear()." << endl;

	MI_S32 s32Ret = 0;
	s32Ret = ST_OSD_Clear(hHandle, NULL);
	if(MI_RGN_OK != s32Ret)
	{
		cerr << "Fail to call Rgn::osdClear(), errno = " << s32Ret << endl;
	}
	
	//cout << "Call Rgn::osdClear() end." << endl;
	return s32Ret;
}

/*-----------------------------------------------------------------------------
描--述：在OSD上画字符串
参--数：hHandle 区域句柄号；stPoint 相对位置信息；str 字符串；
	u32Color 文字颜色，取值范围I4_RED, I4_GREE, I4_BLUE, I4_BLACK
	DMF_Font_Size_E 字体大小，取值范围DMF_FONT_ASCII_32x32, DMF_FONT_HZ_32x32, DMF_FONT_HZ_64x64
返回值：成功，返回MI_RGN_OK; 失败，返回错误码；参数无效，返回-1;
注--意：
-----------------------------------------------------------------------------*/
MI_S32 Rgn::osdDrawText(MI_RGN_HANDLE hHandle, ST_Point_T stPoint, const char *str, MI_U32 u32Color, DMF_Font_Size_E enSize)
{
	//cout << "Call Rgn::osdDrawText()." << endl;

	MI_S32 s32Ret = 0;
	s32Ret = ST_OSD_DrawText(hHandle, stPoint, str, u32Color, enSize);
	if(MI_RGN_OK != s32Ret)
	{
		cerr << "Fail to call Rgn::osdDrawText(), errno = " << s32Ret << endl;
	}
	
	//cout << "Call Rgn::osdDrawText() end." << endl;
	return s32Ret;
}

/*-----------------------------------------------------------------------------
描--述：更新OSD 区域
参--数：hHandle 区域句柄号；
返回值：成功，返回MI_RGN_OK; 失败，返回错误码；参数无效，返回-1;
注--意：
-----------------------------------------------------------------------------*/
MI_S32 Rgn::osdUpdate(MI_RGN_HANDLE hHandle)
{
	//cout << "Call Rgn::osdUpdate()." << endl;

	MI_S32 s32Ret = 0;
	s32Ret = ST_OSD_Update(hHandle);
	if(MI_RGN_OK != s32Ret)
	{
		cerr << "Fail to call Rgn::osdUpdate(), errno = " << s32Ret << endl;
	}
	
	//cout << "Call Rgn::osdUpdate() end." << endl;
	return s32Ret;
}

/*-----------------------------------------------------------------------------
描--述：设置OSD 要显示文字
参--数：hHandle 区域句柄号；str 字符串；u32Color 颜色，取值范围：I4_RED, I4_GREE, I4_BLUE, I4_BLACK
		enSize 字体大小，取值范围：DMF_FONT_ASCII_32x32, DMF_FONT_HZ_32x32, DMF_FONT_HZ_64x64
返回值：返回0
注--意：
-----------------------------------------------------------------------------*/
int Rgn::setText(MI_RGN_HANDLE hHandle, const char *str, MI_U32 u32Color, DMF_Font_Size_E enSize)
{
	ST_Point_T stPoint;
	stPoint.u32X = 0;
	stPoint.u32Y = 0;

	osdGetCanvasInfo(hHandle);
	osdClear(hHandle);
	osdDrawText(hHandle, stPoint, str, u32Color, enSize);
	osdUpdate(hHandle);

	return 0;
}

