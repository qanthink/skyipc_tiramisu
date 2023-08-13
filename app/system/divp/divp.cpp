/*---------------------------------------------------------------- 
xxx版权所有。
作者：
时间：2022.6.5
----------------------------------------------------------------*/

#include "divp.h"
#include <iostream>
#include <string.h>

using namespace std;

Divp::Divp()
{
	cout << "Call Divp::Divp()." << endl;
	cout << "Call Divp::Divp() end." << endl;
}

Divp::~Divp()
{
	cout << "Call Divp::~Divp()." << endl;
	cout << "Call Divp::~Divp() end." << endl;
}

/*  ---------------------------------------------------------------------------
描--述：VENC 模块获取实例的唯一入口
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
Divp* Divp::getInstance()
{
	static Divp divp;
	return &divp;
}

/*-----------------------------------------------------------------------------
描--述：创建DIVP 通道。
参--数：divpChn DIVP 通道；pstChnAttr 指向通道属性的结构体指针。
返回值：成功，返回0; 失败，返回错误码。
注--意：
-----------------------------------------------------------------------------*/
MI_S32 Divp::createChn(unsigned int wIn, unsigned int hIn, 
						unsigned int wOut, unsigned int hOut)
{
	cout << "Call Divp::createChn() end." << endl;

	MI_DIVP_ChnAttr_t stChnAttr;

	stChnAttr.bHorMirror = false;
	stChnAttr.bVerMirror = false;
	stChnAttr.eDiType = E_MI_DIVP_DI_TYPE_OFF;
	stChnAttr.eRotateType = E_MI_SYS_ROTATE_NONE;
	stChnAttr.eTnrLevel = E_MI_DIVP_TNR_LEVEL_OFF;
	stChnAttr.u32MaxWidth = wIn;
	stChnAttr.u32MaxHeight = wOut;

	// Crop. 本程序暂不做Crop.
	//stChnAttr.stCropRect.u16Width = stChnAttr.u32MaxWidth;
	//stChnAttr.stCropRect.u16Height = stChnAttr.u32MaxHeight;
	stChnAttr.stCropRect.u16X = 0;
	stChnAttr.stCropRect.u16Y = 0;
	stChnAttr.stCropRect.u16Width = 0;
	stChnAttr.stCropRect.u16Height = 0;

	MI_S32 s32Ret = 0;
	s32Ret = MI_DIVP_CreateChn(divpChn, &stChnAttr);
	switch(s32Ret)
	{
		case MI_SUCCESS:
		{
			cout << "Success to call MI_DIVP_CreateChn() in Divp::createChn()." << endl;
			break;
		}
		case MI_DIVP_ERR_FAILED:
		{
			cerr << "Fail to call MI_DIVP_CreateChn() in Divp::createChn()." << endl;
			return s32Ret;
		}
		case MI_DIVP_ERR_NO_RESOUCE:
		{
			cerr << "Fail to call MI_DIVP_CreateChn() in Divp::createChn()." << endl;
			cerr << "System resource is not enough." << endl;
			return s32Ret;
		}
		default:
		{
			cerr << "Fail to call MI_DIVP_CreateChn() in Divp::createChn(). " 
				<< "s32Ret = " << s32Ret << endl;
			return s32Ret;
		}
	}

	MI_DIVP_OutputPortAttr_t stOutputPortAttr;
	memset(&stOutputPortAttr, 0, sizeof(MI_DIVP_OutputPortAttr_t));
	stOutputPortAttr.eCompMode = E_MI_SYS_COMPRESS_MODE_NONE;
	stOutputPortAttr.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;

	stOutputPortAttr.u32Width = wOut;
	stOutputPortAttr.u32Height = hOut;
	
	s32Ret = MI_DIVP_SetOutputPortAttr(divpChn, &stOutputPortAttr);
	if(MI_SUCCESS != s32Ret)
	{
		cerr << "Fail to call MI_DIVP_SetOutputPortAttr() in Divp::createChn()." << endl;
		return s32Ret;
	}
	
	s32Ret = MI_DIVP_StartChn(divpChn);
	if(MI_SUCCESS != s32Ret)
	{
		cerr << "Fail to call MI_DIVP_StartChn() in Divp::createChn()." << endl;
		return s32Ret;
	}

	cout << "Call Divp::createChn() end." << endl;
	return 0;
}
