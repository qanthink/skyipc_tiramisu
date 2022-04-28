/*---------------------------------------------------------------- 
xxx版权所有。
作者：
时间：2020.7.10
----------------------------------------------------------------*/

#include "venc.h"
#include <iostream>
#include <string.h>

using namespace std;

Venc::Venc()
{
	enable();
	bEnable = true;
}

Venc::~Venc()
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
Venc* Venc::getInstance()
{
	static Venc venc;
	return &venc;
}

/* ---------------------------------------------------------------------------
描--述：
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
MI_S32 Venc::enable()
{
	cout << "Call Venc::enable()." << endl;

	bEnable = true;	
	cout << "Call Venc::enable() end." << endl;
	return 0;
}

/*-----------------------------------------------------------------------------
描--述：
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
MI_S32 Venc::disable()
{
	cout << "Call Venc::disable()." << endl;

	bEnable = false;
	
	//stopRecvPic(vencSubChn);
	//destroyChn(vencSubChn);
	stopRecvPic(vencMainChn);
	destroyChn(vencMainChn);

	cout << "Call Venc::disable() end." << endl;
	return 0;
}

/*-----------------------------------------------------------------------------
描--述：创建VENC通道。
参--数：vencChn VENC通道；pstChnAttr 指向通道属性的结构体指针。
返回值：成功，返回0; 失败，返回错误码。
注--意：
-----------------------------------------------------------------------------*/
MI_S32 Venc::createChn(MI_VENC_CHN vencChn, MI_VENC_ChnAttr_t *pstChnAttr)
{
	cout << "Call Venc::createChn()." << endl;

	MI_S32 s32Ret = 0;
	MI_VENC_ChnAttr_t stChnAttr;
	
	if(NULL == pstChnAttr)
	{
		cout << "Fail to call Venc::createChn(). Argument has null value!" << endl;
		return -1;
	}

	s32Ret = MI_VENC_CreateChn(vencDev, vencChn, pstChnAttr);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_VENC_CreateChn(), errno = " << s32Ret << endl;
	}

	cout << "Call Venc::createChn() end." << endl;
	return s32Ret;
}

/*-----------------------------------------------------------------------------
描--述：获取通道属性。
参--数：vencChn VENC通道；pstChnAttr 指向通道属性的结构体指针。
返回值：成功，返回0; 失败，返回错误码。
注--意：
-----------------------------------------------------------------------------*/
MI_S32 Venc::getChnAttr(MI_VENC_CHN vencChn, MI_VENC_ChnAttr_t *pstChnAttr)
{
	cout << "Call Venc::getChnAttr()." << endl;

	bool bShow = false;
	MI_VENC_ChnAttr_t stChnAttr;
	
	if(NULL == pstChnAttr)
	{
		cerr << "Call Venc::getChnAttr() with null pointer!" << endl;
		pstChnAttr = &stChnAttr;
		bShow = true;
		
		memset(pstChnAttr, 0, sizeof(MI_VENC_ChnAttr_t));
	}

	// MI_S32 MI_VENC_GetChnAttr(MI_VENC_CHN vencChn, MI_VENC_ChnAttr_t *pstChnAttr);
	MI_S32 s32Ret = 0;
	s32Ret = MI_VENC_GetChnAttr(vencDev, vencChn, pstChnAttr);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_VENC_GetChnAttr(), errno = " << s32Ret << endl;
	}

	if(0 == s32Ret && bShow)
	{
		cout << "stChnAttr.stRcAttr.eRcMode = " << stChnAttr.stRcAttr.eRcMode << endl;
		cout << "stChnAttr.stRcAttr.stAttrMjpegCbr.u32BitRate  = " << stChnAttr.stRcAttr.stAttrMjpegCbr.u32BitRate << endl;
		cout << "stChnAttr.stRcAttr.stAttrMjpegFixQp.u32Qfactor = " << stChnAttr.stRcAttr.stAttrMjpegFixQp.u32Qfactor << endl;
		cout << "stChnAttr.stVeAttr.eType = " << stChnAttr.stVeAttr.eType << endl;
		cout << "stChnAttr.stVeAttr.stAttrJpeg.u32MaxPicWidth = " << stChnAttr.stVeAttr.stAttrJpeg.u32MaxPicWidth << endl;
		cout << "stChnAttr.stVeAttr.stAttrJpeg.u32MaxPicHeight = " << stChnAttr.stVeAttr.stAttrJpeg.u32MaxPicHeight << endl;
		cout << "stChnAttr.stVeAttr.stAttrJpeg.u32PicWidth = " << stChnAttr.stVeAttr.stAttrJpeg.u32PicWidth << endl;
		cout << "stChnAttr.stVeAttr.stAttrJpeg.u32PicHeight = " << stChnAttr.stVeAttr.stAttrJpeg.u32PicHeight << endl;
	}

	cout << "Call Venc::getChnAttr() end." << endl;
	return s32Ret;
}

/*-----------------------------------------------------------------------------
描--述：设置通道属性。
参--数：vencChn VENC通道；pstChnAttr 指向通道属性的结构体指针。
返回值：成功，返回0; 失败，返回错误码。
注--意：
-----------------------------------------------------------------------------*/
MI_S32 Venc::setChnAttr(MI_VENC_CHN vencChn, MI_VENC_ChnAttr_t *pstChnAttr)
{
	cout << "Call Venc::setChnAttr()." << endl;
	if(NULL == pstChnAttr)
	{
		cerr << "Fail to call Venc::setChnAttr(). Argument has null valu.e" << endl;
		return -1;
	}

	// MI_S32 MI_VENC_SetChnAttr(MI_VENC_CHN vencChn, MI_VENC_ChnAttr_t *pstAttr);
	MI_S32 s32Ret = 0;
	s32Ret = MI_VENC_SetChnAttr(vencDev, vencChn, pstChnAttr);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_VENC_SetChnAttr(), errno = " << s32Ret << endl;
	}

	cout << "Call Venc::setChnAttr() end." << endl;
	return s32Ret;
}

/*-----------------------------------------------------------------------------
描--述：设置通道属性。
参--数：vencChn VENC通道；pstChnAttr 指向通道属性的结构体指针。
返回值：成功，返回0; 失败，返回错误码。
注--意：
-----------------------------------------------------------------------------*/
MI_S32 Venc::setCrop(MI_VENC_CHN vencChn, MI_U32 x, MI_U32 y, MI_U32 w, MI_U32 h)
{
	cout << "Call Venc::setCrop() end." << endl;
	MI_S32 s32Ret = 0;
	MI_VENC_CropCfg_t stCropCfg;

	stCropCfg.bEnable = true;
	stCropCfg.stRect.u32Left = x;
	stCropCfg.stRect.u32Top = y;
	stCropCfg.stRect.u32Width = w;
	stCropCfg.stRect.u32Height = h;
	s32Ret = MI_VENC_SetCrop(vencDev, vencChn, &stCropCfg);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_VENC_SetCrop(), errno = " << s32Ret << endl;
	}

	cout << "Call Venc::setCrop() end." << endl;
	return s32Ret;
}

/*-----------------------------------------------------------------------------
描--述：改变比特率。
参--数：vencChn VENC通道号；u32Bitrate 传输的比特率 bit/s
返回值：成功，返回0; 失败，返回错误码。
注--意：
-----------------------------------------------------------------------------*/
MI_S32 Venc::changeBitrate(MI_VENC_CHN vencChn, MI_U32 u32BitrateKb)
{
	cout << "Call Venc::changeBitrate()." << endl;

	MI_VENC_ChnAttr_t stChnAttr;
	memset(&stChnAttr, 0, sizeof(MI_VENC_ChnAttr_t));

	MI_S32 s32Ret = 0;
	s32Ret = getChnAttr(vencChn, &stChnAttr);
	if(0 != s32Ret)
	{
		cerr << "Fail to call Venc::getChnAttr(), errno = " << s32Ret << endl;
		return s32Ret;
	}

	stChnAttr.stRcAttr.stAttrH265Cbr.u32BitRate = u32BitrateKb * 1024;	// 转换为bps.
	s32Ret = setChnAttr(vencChn, &stChnAttr);	
	if(0 != s32Ret)
	{
		cerr << "Fail to call Venc::setChnAttr(), errno = " << s32Ret << endl;
	}

	cout << "Call Venc::changeBitrate() end." << endl;
	return s32Ret;
}

/*-----------------------------------------------------------------------------
描--述：销毁通道。
参--数：vencChn 通道号。
返回值：成功，返回0; 失败，返回错误码。
注--意：
-----------------------------------------------------------------------------*/
MI_S32 Venc::destroyChn(MI_VENC_CHN vencChn)
{
	cout << "Call Venc::destroyChn()." << endl;

	// MI_S32 MI_VENC_DestroyChn(MI_VENC_CHN vencChn);
	MI_S32 s32Ret = 0;
	s32Ret = MI_VENC_DestroyChn(vencDev, vencChn);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_VENC_DestroyChn(), errno = " << s32Ret << endl;
	}

	cout << "Call Venc::destroyChn() end." << endl;
	return s32Ret;
}

/*-----------------------------------------------------------------------------
描--述：VENC开始接收图像。
参--数：vencChn 通道号。
返回值：成功，返回0; 失败，返回错误码。
注--意：
-----------------------------------------------------------------------------*/
MI_S32 Venc::startRecvPic(MI_VENC_CHN vencChn)
{
	cout << "Call Venc::startRecvPic()." << endl;

	// MI_S32 MI_VENC_StartRecvPic(MI_VENC_CHN vencChn);
	MI_S32 s32Ret = 0;

	s32Ret = MI_VENC_StartRecvPic(vencDev, vencChn);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_VENC_StartRecvPic(), errno = " << s32Ret << endl;
	}

	cout << "Call Venc::startRecvPic() end." << endl;
	return s32Ret;
}

/*-----------------------------------------------------------------------------
描--述：VENC停止接收图像。
参--数：vencChn 通道号。
返回值：成功，返回0; 失败，返回错误码。
注--意：
-----------------------------------------------------------------------------*/
MI_S32 Venc::stopRecvPic(MI_VENC_CHN vencChn)
{
	cout << "Call Venc::stopRecvPic()." << endl;

	// MI_S32 MI_VENC_StopRecvPic(MI_VENC_CHN vencChn);
	MI_S32 s32Ret = 0;

	s32Ret = MI_VENC_StopRecvPic(vencDev, vencChn);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_VENC_StopRecvPic(), errno = " << s32Ret << endl;
	}

	cout << "Call Venc::stopRecvPic() end." << endl;
	return s32Ret;
}

/*-----------------------------------------------------------------------------
描--述：查询VENC 状态。
参--数：vencChn 通道号；pstChnStat 指向状态结构体的指针。
返回值：成功，返回0; 失败，返回错误码。
注--意：关注结构体中如下参数：u32LeftPics 待编码帧个数。
	u32CurPacks 当前帧的码流包个数。调用MI_VENC_GetStream() 之前应确保该参数大于0.
-----------------------------------------------------------------------------*/
MI_S32 Venc::query(MI_VENC_CHN vencChn, MI_VENC_ChnStat_t *pstChnStat)
{
	// cout << "Call Venc::query()." << endl;

	bool bShow = false;
	MI_VENC_ChnStat_t stChnStat;
	if(NULL == pstChnStat)
	{
		cerr << "Call Venc::query() with null pointer! Use default argument!" << endl;
		memset(&stChnStat, 0, sizeof(MI_VENC_ChnStat_t));
		pstChnStat = &stChnStat;
		bShow = true;
	}

	// MI_S32 MI_VENC_Query(MI_VENC_CHN vencChn, MI_VENC_ChnStat_t *pstStat);
	MI_S32 s32Ret = 0;
	s32Ret = MI_VENC_Query(vencDev, vencChn, pstChnStat);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_VENC_Query(), errno = " << s32Ret << endl;
	}

	if(0 != s32Ret && bShow)
	{
		cout << "pstChnStat->u32LeftPics = " << pstChnStat->u32LeftPics << endl;
		cout << "pstChnStat->u32LeftStreamBytes = " << pstChnStat->u32LeftStreamBytes << endl;
		cout << "pstChnStat->u32LeftStreamFrames = " << pstChnStat->u32LeftStreamFrames << endl;
		cout << "pstChnStat->u32LeftStreamMillisec = " << pstChnStat->u32LeftStreamMillisec << endl;
		cout << "pstChnStat->u32LeftRecvPics = " << pstChnStat->u32LeftRecvPics << endl;
		cout << "pstChnStat->u32LeftEncPics = " << pstChnStat->u32LeftEncPics << endl;
		cout << "pstChnStat->u32FrmRateNum = " << pstChnStat->u32FrmRateNum << endl;
		cout << "pstChnStat->u32FrmRateDen = " << pstChnStat->u32FrmRateDen << endl;
		cout << "pstChnStat->u32BitRate = " << pstChnStat->u32BitRate << endl;
	}

	// cout << "Call Venc::query() end." << endl;
	return s32Ret;
}

/*-----------------------------------------------------------------------------
描--述：获取编码后的流数据。
参--数：vencChn 通道号；pstStream 指向流结构体的指针；s32MilliSec 超时参数，取值范围>= -1:
	-1, 以阻塞方式等待编码数据；0, 不阻塞；>0, 超时等待，单位为ms.
返回值：传递非法参数，返回-1; 成功，返回0; 失败，返回错误码。
注--意：必须与releaseStream() 成对使用，避免内存泄漏。
-----------------------------------------------------------------------------*/
MI_S32 Venc::getStream(MI_VENC_CHN vencChn, MI_VENC_Stream_t *pstStream, MI_S32 s32MilliSec)
{
	// cout << "Call Venc::getStream()." << endl;
	
	if(NULL == pstStream)
	{
		cerr << "Fail to call Venc::getStream(), argument has null value!" << endl;
		return -1;		
	}

	// MI_S32 MI_VENC_GetStream(MI_VENC_CHN vencChn, MI_VENC_Stream_t *pstStream, MI_S32 s32MilliSec);
	MI_S32 s32Ret = 0;
	s32Ret = MI_VENC_GetStream(vencDev, vencChn, pstStream, s32MilliSec);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_VENC_GetStream(), errno = " << s32Ret << endl;
	}

	// cout << "Call Venc::getStream() end." << endl;
	return s32Ret;
}

/*-----------------------------------------------------------------------------
描--述：释放码流缓存。
参--数：vencChn 通道号；pstStream 指向流结构体的指针。
返回值：成功，返回0; 失败，返回错误码。
注--意：在rcvStream() 后必须主动调用该函数释放缓冲区内存，避免内存泄漏。
	ispahanV20中调整为，先调用SDK接口MI_VENC_ReleaseStream, 再释放用户空间free(pstStream->pstPack)
-----------------------------------------------------------------------------*/
MI_S32 Venc::releaseStream(MI_VENC_CHN vencChn, MI_VENC_Stream_t *pstStream)
{
	// cout << "Call Venc::releaseStream()." << endl;
	
	if(NULL == pstStream)
	{
		cerr << "Fail to call Venc::releaseStream(), argument has null value!" << endl;
		return -1;		
	}

	MI_S32 s32Ret = 0;
	s32Ret = MI_VENC_ReleaseStream(vencDev, vencChn, pstStream);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_VENC_ReleaseStream(), errno = " << s32Ret << endl;
	}

	free(pstStream->pstPack);
	pstStream->pstPack = NULL;

	// cout << "Call Venc::releaseStream() end." << endl;
	return s32Ret;
}

/*-----------------------------------------------------------------------------
描--述：获取编码通道对应的设备文件句柄。
参--数：vencChn 通道号。
返回值：成功返回文件描述符，失败返回错误码。
注--意：用于调用select/poll监听是否有编码数据，与轮询CPU相比有效降低了cpu使用率。
-----------------------------------------------------------------------------*/
MI_S32 Venc::getFd(MI_VENC_CHN vencChn)
{
	// cout << "Call Venc::getFd()." << endl;

	// MI_S32 MI_VENC_GetFd(MI_VENC_CHN vencChn);
	MI_S32 s32Fd = 0;
	s32Fd = MI_VENC_GetFd(vencDev, vencChn);
	if(-1 == s32Fd)
	{
		cerr << "Fail to call MI_VENC_GetFd()." << strerror(errno) << endl;
	}

	// cout << "Call Venc::getFd() end." << endl;
	return s32Fd;
}

/*-----------------------------------------------------------------------------
描--述：
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
MI_S32 Venc::setJpegParam(MI_VENC_CHN vencChn, MI_VENC_ParamJpeg_t *pstJpegParam)
{
	cout << "Call Venc::setJpegParam()." << endl;

	MI_VENC_ParamJpeg_t stJpegParam;
	
	if(NULL == pstJpegParam)
	{
		cerr << "Call Venc::setJpegParam() with null pointer! Use default argument!" << endl;
		pstJpegParam = &stJpegParam;

		MI_S32 s32Ret = 0;
		memset(&stJpegParam, 0, sizeof(MI_VENC_ParamJpeg_t));

		s32Ret = getJpegParam(vencChn, pstJpegParam);
		if(0 != s32Ret)
		{
			cerr << "Fail to call Venc::getJpegParam(), errno = " << s32Ret << endl;
		}
		
		stJpegParam.u32Qfactor = 50;
	}

	// MI_S32 MI_VENC_SetJpegParam(MI_VENC_CHN vencChn, MI_VENC_ParamJpeg_t *pstJpegParam);
	MI_S32 s32Ret = 0;

	s32Ret = MI_VENC_SetJpegParam(vencDev, vencChn, pstJpegParam);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_VENC_SetJpegParam(), errno = " << s32Ret << endl;
	}

	cout << "Call Venc::setJpegParam() end." << endl;
	return s32Ret;
}

/*-----------------------------------------------------------------------------
描--述：
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
MI_S32 Venc::getJpegParam(MI_VENC_CHN vencChn, MI_VENC_ParamJpeg_t *pstJpegParam)
{
	cout << "Call Venc::getJpegParam()." << endl;

	bool bShow = false;
	MI_VENC_ParamJpeg_t stJpegParam;
	
	if(NULL == pstJpegParam)
	{
		cout << "Call Venc::getJpegParam() with null pointer!" << endl;
		pstJpegParam = &stJpegParam;
		bShow = true;
	}

	MI_S32 s32Ret = 0;
	memset(&stJpegParam, 0, sizeof(MI_VENC_ParamJpeg_t));
	
	// MI_S32 MI_VENC_GetJpegParam(MI_VENC_CHN vencChn, MI_VENC_ParamJpeg_t *pstJpegParam);
	s32Ret = MI_VENC_GetJpegParam(vencDev, vencChn, pstJpegParam);	
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_VENC_GetJpegParam(), errno = " << s32Ret << endl;
	}

	if(0 != s32Ret && bShow)
	{
		cout << "pstJpegParam->u32McuPerEcs = " << pstJpegParam->u32McuPerEcs<< endl;
		cout << "pstJpegParam->u32Qfactor = " << pstJpegParam->u32Qfactor << endl;
	}

	cout << "Call Venc::getJpegParam() end." << endl;
	return s32Ret;
}

/*-----------------------------------------------------------------------------
描--述：设置H265 的slice 分割属性。
参--数：vencChn 通道号；pstSliceSplit 指向分割参数的结构体指针。
返回值：传递非法参数，返回-1; 成功，返回0; 失败，返回错误码。
注--意：
-----------------------------------------------------------------------------*/
MI_S32 Venc::setH265SliceSplit(MI_VENC_CHN vencChn, MI_VENC_ParamH265SliceSplit_t *pstSliceSplit)
{
	cout << "Call Venc::setH265SliceSplit()." << endl;

	MI_VENC_ParamH265SliceSplit_t stSliceSplit;
	if(NULL == pstSliceSplit)
	{
		cerr << "Call Venc::setH265SliceSplit() with null value. Use default argument!" << endl;
		pstSliceSplit = &stSliceSplit;

		stSliceSplit.bSplitEnable = true;
		stSliceSplit.u32SliceRowCount = 8;	// 0 <= u32SliceRowCount <= (height+31)/height;
											// 2 -> 17 4 -> 9 8 -> 5
	}
	
	// MI_S32 MI_VENC_SetH265SliceSplit(MI_VENC_CHN vencChn, MI_VENC_ParamH265SliceSplit_t *pstSliceSplit);
	MI_S32 s32Ret = 0;
	s32Ret = MI_VENC_SetH265SliceSplit(vencDev, vencChn, pstSliceSplit);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_VENC_SetH265SliceSplit(), errno = " << s32Ret << endl;
	}

	cout << "Call Venc::setH265SliceSplit() end." << endl;
	return s32Ret;
}

/*-----------------------------------------------------------------------------
描--述：请求I帧。
参--数：vencChn 通道号；bInstant 是否立刻获取，1, 下一帧即为I帧; 0, 以编码器最快的速度形成I帧。
返回值：返回错误码。
注--意：
-----------------------------------------------------------------------------*/
MI_S32 Venc::requestIdr(MI_VENC_CHN veChn, MI_BOOL bInstant)
{
	//cout << "Call Venc::requestIdr()." << endl;

	MI_S32 s32Ret = 0;
	s32Ret = MI_VENC_RequestIdr(vencDev, veChn, bInstant);
	if(0 != s32Ret)
	{
		cerr << "Fail to call Venc::requestIdr(), s32Ret = " << s32Ret << endl;
		return s32Ret;
	}
	
	cout << "Call Venc::requestIdr() end." << endl;
	return s32Ret;
}

/*-----------------------------------------------------------------------------
描--述：创建主码流。
参--数：vencChn 通道号；pstChnAttr 指向频道属性的结构体指针，传入NULL, 使用默认属性。
返回值：返回错误码。
注--意：
-----------------------------------------------------------------------------*/
MI_S32 Venc::createMainStream(MI_VENC_CHN vencChn, MI_VENC_ChnAttr_t *pstChnAttr)
{
	cout << "Call Venc::createMainStream() end." << endl;

	MI_VENC_ChnAttr_t stChnAttr;
	memset(&stChnAttr, 0, sizeof(MI_VENC_ChnAttr_t));
	if(NULL == pstChnAttr)
	{
		cerr << "Call Venc::createMainStream() with null pointer. Use default argument!" << endl;
		pstChnAttr = &stChnAttr;
		memset(&stChnAttr, 0, sizeof(MI_VENC_ChnAttr_t));

		switch(emMainResol)
		{
			case emResolFHD:
			{
				stChnAttr.stVeAttr.stAttrH265e.u32PicWidth = 1920;
				stChnAttr.stVeAttr.stAttrH265e.u32PicHeight = 1080;
				stChnAttr.stVeAttr.stAttrH265e.u32MaxPicWidth = 1920;
				stChnAttr.stVeAttr.stAttrH265e.u32MaxPicHeight = 1080;
				break;
			}
			case emResol4MP:
			{
				stChnAttr.stVeAttr.stAttrH265e.u32PicWidth = 2560;
				stChnAttr.stVeAttr.stAttrH265e.u32PicHeight = 1440;
				stChnAttr.stVeAttr.stAttrH265e.u32MaxPicWidth = 2560;
				stChnAttr.stVeAttr.stAttrH265e.u32MaxPicHeight = 1440;
				break;
			}
			default:
			{
				stChnAttr.stVeAttr.stAttrH265e.u32PicWidth = 1920;
				stChnAttr.stVeAttr.stAttrH265e.u32PicHeight = 1080;
				stChnAttr.stVeAttr.stAttrH265e.u32MaxPicWidth = 2560;
				stChnAttr.stVeAttr.stAttrH265e.u32MaxPicHeight = 1440;
				break;
			}
		}
		
		if(bUseSliceMode)
		{
			stChnAttr.stVeAttr.stAttrH265e.bByFrame = false;	// 按帧/包获取码流：true/false
		}
		else
		{
			stChnAttr.stVeAttr.stAttrH265e.bByFrame = true;		// 按帧/包获取码流：true/false
		}
		//stChnAttr.stVeAttr.stAttrH265e.u32Profile = 0;		// 编码优先级级，暂不支持。

		stChnAttr.stVeAttr.eType = E_MI_VENC_MODTYPE_H265E;
		stChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_H265CBR;
		stChnAttr.stRcAttr.stAttrH265Cbr.u32BitRate = mainBitRateKb * 1024;
		stChnAttr.stRcAttr.stAttrH265Cbr.u32SrcFrmRateNum = 30;	// FPS = 分母Num / 分子Den
		stChnAttr.stRcAttr.stAttrH265Cbr.u32SrcFrmRateDen = 1;
		stChnAttr.stRcAttr.stAttrH265Cbr.u32Gop = mainGop;
		stChnAttr.stRcAttr.stAttrH265Cbr.u32FluctuateLevel = 0;	// 暂不支持
		stChnAttr.stRcAttr.stAttrH265Cbr.u32StatTime = 0;		// 暂不支持
	}
	
	MI_S32 s32Ret = 0;
	s32Ret = createChn(vencChn, pstChnAttr);
	if(0 != s32Ret)
	{
		cerr << "Fail to call Venc::createChn(), s32Ret = " << s32Ret << endl;
		return s32Ret;
	}

	if(bUseSliceMode)
	{
		s32Ret = setH265SliceSplit(vencChn, NULL);	// slice 分片模式
		if(0 != s32Ret)
		{
			cerr << "Fail to call Venc::setH265SliceSplit(), s32Ret = " << s32Ret << endl;
			return s32Ret;
		}
	}
	
	s32Ret = startRecvPic(vencChn);					// 开始接受通道数据
	if(0 != s32Ret)
	{
		cerr << "Fail to call Venc::startRecvPic(), s32Ret = " << s32Ret << endl;
		return s32Ret;
	}
	
	cout << "Call Venc::createMainStream() end." << endl;
	return s32Ret;
}

/*-----------------------------------------------------------------------------
描--述：创建子码流。
参--数：vencChn 通道号；pstChnAttr 指向频道属性的结构体指针，传入NULL, 使用默认属性。
返回值：返回错误码。
注--意：
-----------------------------------------------------------------------------*/
MI_S32 Venc::createSubStream(MI_VENC_CHN vencChn, MI_VENC_ChnAttr_t *pstChnAttr)
{
	cout << "Call Venc::createSubStream() end." << endl;

	MI_VENC_ChnAttr_t stChnAttr;
	if(NULL == pstChnAttr)
	{
		cerr << "Call Venc::createSubStream() with null pointer. Use default argument!" << endl;
		pstChnAttr = &stChnAttr;
		memset(&stChnAttr, 0, sizeof(MI_VENC_ChnAttr_t));

		switch(emSubResol)
		{
			case emResolHD:
			{
				stChnAttr.stVeAttr.stAttrH265e.u32PicWidth = 1280;
				stChnAttr.stVeAttr.stAttrH265e.u32PicHeight = 720;
				stChnAttr.stVeAttr.stAttrH265e.u32MaxPicWidth = 1280;
				stChnAttr.stVeAttr.stAttrH265e.u32MaxPicHeight = 720;
				break;
			}
			case emResol4MP:
			{
				stChnAttr.stVeAttr.stAttrH265e.u32PicWidth = 1920;
				stChnAttr.stVeAttr.stAttrH265e.u32PicHeight = 1080;
				stChnAttr.stVeAttr.stAttrH265e.u32MaxPicWidth = 1920;
				stChnAttr.stVeAttr.stAttrH265e.u32MaxPicHeight = 1080;
				break;
			}
			default:
			{
				stChnAttr.stVeAttr.stAttrH265e.u32PicWidth = 1920;
				stChnAttr.stVeAttr.stAttrH265e.u32PicHeight = 1080;
				stChnAttr.stVeAttr.stAttrH265e.u32MaxPicWidth = 1920;
				stChnAttr.stVeAttr.stAttrH265e.u32MaxPicHeight = 1080;
				break;
			}
		}

		//stChnAttr.stVeAttr.stAttrH265e.u32Profile = 0;		// 编码优先级级，暂不支持。
		stChnAttr.stVeAttr.eType = E_MI_VENC_MODTYPE_H265E;
		stChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_H265CBR;
		stChnAttr.stRcAttr.stAttrH265Cbr.u32BitRate = subBitRateKb * 1024;
		stChnAttr.stRcAttr.stAttrH265Cbr.u32SrcFrmRateNum = 30;	// FPS = 分母Num / 分子Den
		stChnAttr.stRcAttr.stAttrH265Cbr.u32SrcFrmRateDen = 1;
		stChnAttr.stRcAttr.stAttrH265Cbr.u32Gop = subGop;
		stChnAttr.stRcAttr.stAttrH265Cbr.u32FluctuateLevel = 0;	// 暂不支持
		stChnAttr.stRcAttr.stAttrH265Cbr.u32StatTime = 0;		// 暂不支持
	}
	
	MI_S32 s32Ret = 0;
	s32Ret = createChn(vencChn, pstChnAttr);
	if(0 != s32Ret)
	{
		cerr << "Fail to call Venc::createChn(), s32Ret = " << s32Ret << endl;
		return s32Ret;
	}

	s32Ret = startRecvPic(vencChn);
	if(0 != s32Ret)
	{
		cerr << "Fail to call Venc::startRecvPic(), s32Ret = " << s32Ret << endl;
		return s32Ret;
	}
	
	cout << "Call Venc::createSubStream() end." << endl;
	return s32Ret;
}

/*-----------------------------------------------------------------------------
描--述：创建JPEG码流。
参--数：vencChn 通道号；pstChnAttr 指向频道属性的结构体指针，传入NULL, 使用默认属性。
返回值：返回错误码。
注--意：
-----------------------------------------------------------------------------*/
MI_S32 Venc::createJpegStream(MI_VENC_CHN vencChn, MI_VENC_ChnAttr_t *pstChnAttr)
{
	cout << "Call Venc::createJpegStream() end." << endl;

	MI_VENC_ChnAttr_t stChnAttr;
	if(NULL == pstChnAttr)
	{
		cerr << "Call Venc::createJpegStream() with null pointer. Use default argument!" << endl;
		pstChnAttr = &stChnAttr;
		memset(&stChnAttr, 0, sizeof(MI_VENC_ChnAttr_t));

		stChnAttr.stVeAttr.stAttrJpeg.u32BufSize = 2 * 1920 * 1080;	// 不小于最大宽高的乘积
		stChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_MJPEGFIXQP;
		stChnAttr.stVeAttr.stAttrJpeg.u32PicWidth = 1920;
		stChnAttr.stVeAttr.stAttrJpeg.u32PicHeight = 1080;
		stChnAttr.stVeAttr.stAttrJpeg.u32MaxPicWidth = 1920;
		stChnAttr.stVeAttr.stAttrJpeg.u32MaxPicHeight = 1080;
		stChnAttr.stVeAttr.stAttrJpeg.bByFrame = TRUE;
		stChnAttr.stVeAttr.eType = E_MI_VENC_MODTYPE_JPEGE;
		stChnAttr.stRcAttr.stAttrMjpegFixQp.u32SrcFrmRateNum = 30;
		stChnAttr.stRcAttr.stAttrMjpegFixQp.u32SrcFrmRateDen = 1;
	}
	
	MI_S32 s32Ret = 0;
	s32Ret = createChn(vencChn, pstChnAttr);
	if(0 != s32Ret)
	{
		cerr << "Fail to call Venc::createChn(), s32Ret = " << s32Ret << endl;
		return s32Ret;
	}

	s32Ret = startRecvPic(vencChn);
	if(0 != s32Ret)
	{
		cerr << "Fail to call Venc::startRecvPic(), s32Ret = " << s32Ret << endl;
		return s32Ret;
	}
	
	cout << "Call Venc::createJpegStream() end." << endl;
	return s32Ret;
}

/*-----------------------------------------------------------------------------
描--述：从VENC 中获取编码好的数据，存入pstStreamPack 指向的结构体中。
参--数：
返回值：成功，返回0; 失败，返回错误码。
	-1, 传递非法参数
	-2, 获取VENC 通道的文件描述符失败。
	-3, select(2) 出错。
	-4, select(2) 超时。
	-5, VENC 通道无数据。
	-6, malloc(3) 出错。
注--意：原则上函数体不应超过屏幕的最大显示量，然而取流的过程确实比较繁琐，不得已而写了这个100多行的函数。
-----------------------------------------------------------------------------*/
int Venc::rcvStream(MI_VENC_CHN vencChn, MI_VENC_Stream_t *pstStream)
{
	//cout << "Call Venc::rcvStream()." << endl;

	// 入参检测
	if(NULL == pstStream)
	{
		cerr << "Fail to call rcvStream(), argument has null value!" << endl;
		return -1;
	}
	
	// step1: 获取文件描述符。
	MI_S32 s32Fd = 0;
	s32Fd = getFd(vencChn);
	if(-1 == s32Fd)
	{
		cerr << "Fail to call Venc::getFd()." << endl;
		return -2;
	}
	
	// step2: 监听。加入文件描述符集。
	fd_set readFds;
	FD_ZERO(&readFds);
	FD_SET(s32Fd, &readFds);

	// 设置超时时间。
	struct timeval timeOutVal = {0};
	timeOutVal.tv_sec = 0;
	timeOutVal.tv_usec = 500 * 1000; // N * 1000 = N ms
	
	int ret = 0;
	ret = select(s32Fd + 1, &readFds, NULL, NULL, &timeOutVal);
	if(-1 == ret)
	{
		cerr << "Fail to call select(2), " << strerror(errno) << endl;
		return -3;
	}
	else if(0 == ret)
	{
		cout << "Venc select(2) timeout!" << endl;
		return -4;
	}
	else
	{
		// do next;
	}
	
	// step3: 查询。
	MI_S32 s32Ret = 0;
	MI_VENC_ChnStat_t stChnStat;
	memset(&stChnStat, 0, sizeof(MI_VENC_ChnStat_t));
	s32Ret = query(vencChn, &stChnStat);
	if(0 != s32Ret)
	{
		cerr << "Fail to call Venc::query(), errno = " << s32Ret << endl;
		return s32Ret;
	}

	if(0 == stChnStat.u32CurPacks)
	{
		cerr <<"Call Venc::query() end. 0 == stChnStat.u32CurPacks!" << endl;
		return -5;
	}

	// step4: 创建数据缓存区。
	memset(pstStream, 0, sizeof(MI_VENC_Stream_t));
	// 存在malloc 申请的自由内存，在releaseStream() 之后，务必手动释放。
	pstStream->pstPack = (MI_VENC_Pack_t *)malloc(sizeof(MI_VENC_Pack_t) * stChnStat.u32CurPacks);
	if(NULL == pstStream->pstPack)
	{
		cerr << "Fail to call malloc(3)." << endl;
		return -6;
	}
	memset(pstStream->pstPack, 0, sizeof(MI_VENC_Pack_t) * stChnStat.u32CurPacks);
	pstStream->u32PackCount = stChnStat.u32CurPacks;
	
	// step5: 获取流。
	MI_S32 s32MilliSec = 40;
	s32Ret = getStream(vencChn, pstStream, s32MilliSec);
	if(0 != s32Ret)
	{
		cerr << "Fail to call Venc::getStream(), errno = " << s32Ret << endl;
		free(pstStream->pstPack);
		pstStream->pstPack = NULL;
		return s32Ret;
	}

	// step6: 用户处理流。
	#if 0	//debug
	printVencStreamInfo(pstStream);
	#endif
	// step7: 释放流，该过程必须有。但可以在后续处理完流之后再释放。
	
	//cout << "Call Venc::rcvStream() end." << endl;
	return 0;
}

/*-----------------------------------------------------------------------------
描--述：
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
void Venc::printStreamInfo(const MI_VENC_Stream_t *pstVencStream)
{
	if(NULL == pstVencStream)
	{
		cerr << "Fail to call Venc::printVencStreamInfo(). Pointer has null value." << endl;
		return;
	}


	int i = 0;
	for(i = 0; i < pstVencStream->u32PackCount; ++i)
	{
		if(pstVencStream->pstPack[i].u32Len > stStreamPack_t::u32PackMaxLen)
		{
			cerr << "pstVencStream->pstPack[" << i << "].u32Len is out of range." << endl;
			break;
		}
		
		printf("pstVencStream->u32PackCount = %u.\n", i, pstVencStream->u32PackCount);
		printf("pstVencStream->u32Seq = %u.\n", i, pstVencStream->u32Seq);
		printf("pstVencStream->pstPack[%d].u32Len = %u.\n", i, pstVencStream->pstPack[i].u32Len);
		printf("pstVencStream->pstPack[%d].u64PTS = %u.\n", i, pstVencStream->pstPack[i].u64PTS);
		printf("pstVencStream->pstPack[%d].u32Offset = %u.\n", i, pstVencStream->pstPack[i].u32Offset);
		printf("pstVencStream->pstPack[%d].u32DataNum = %u.\n", i, pstVencStream->pstPack[i].u32DataNum);
		
#if 0	// PTS debug
		static unsigned int u64PTS = 0;
		unsigned int u64Dif = 0;
		if(0 != u64PTS)
		{
			u64Dif = pstVencStream->pstPack[0].u64PTS - u64PTS;
			u64PTS = pstVencStream->pstPack[0].u64PTS;
	
			if(u64Dif < 33330 || u64Dif > 33336)
			{
				cout << "Dif is out of range." << endl;
			}
		}
#endif
	}
}

