/*---------------------------------------------------------------- 
xxx版权所有。
作者：
时间：2020.7.10
----------------------------------------------------------------*/

#include "venc.h"
#include "sys.h"
#include "sensor.h"
#include "isp.h"
#include "scl.h"
#include "uac_uvc.h"
#include "st_common.h"

#include <iostream>

using namespace std;

/*  ---------------------------------------------------------------------------
描--述：VENC 模块获取实例的唯一入口
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
UacUvc* UacUvc::getInstance()
{
	static UacUvc uacUvc;
	return &uacUvc;
}

UacUvc::UacUvc()
{
	cout << "Call UacUvc::UacUvc()." << endl;
	cout << "Call UacUvc::UacUvc() end." << endl;
}

UacUvc::~UacUvc()
{
	cout << "Call UacUvc::~UacUvc()." << endl;
	cout << "Call UacUvc::~UacUvc() end." << endl;
}

/*-----------------------------------------------------------------------------
描--述：填充UVC MMA 视频缓冲区。
参--数：uvc, 指向UVC 设备；pUvcBufInfo 指向Buff 信息。
返回值：成功，返回MI_SUCCESS;
		失败：返回-EINVAL；
		-1, 无法获取UVC 设备信息；
注--意：1. 该函数被底层SDK 调用，uvc 指向UVC 设备，pUvcBufInfo 需要我们修改。
		2. 过程中失败，只能返回-EINVAL, 不能返回别的数值，底层无法解析。
-----------------------------------------------------------------------------*/
static MI_S32 UVC_MM_FillBuffer(void *uvc, ST_UVC_BufInfo_t *bufInfo)
{
	//cout << "Call UVC_MM_FillBuffer()." << endl;

	ST_UvcDev_t *pstDev = (ST_UvcDev_t *)uvc;
	if(NULL == pstDev)
	{
		cerr << "Fail to call UVC_MM_FillBuffer(). Argument has null value. Uvc device does not exists" << endl;
		return -1;
	}

	MI_U32 vencChn = 0;
	switch(pstDev->setting.fcc)
	{
		case V4L2_PIX_FMT_YUYV:
			break;
		case V4L2_PIX_FMT_MJPEG:
			vencChn = Venc::vencJpegChn;
			break;
		case V4L2_PIX_FMT_H264:
			vencChn = Venc::vencMainChn;
			break;
		case V4L2_PIX_FMT_H265:
			vencChn = Venc::vencMainChn;
			break;
		default:
			cerr << "In UVC_MM_FillBuffer(). Format format = " << pstDev->setting.fcc 
				<< ". Just support: H.264/265, MJPEG, YUYV422." << endl;
			return -EINVAL;
			break;
	}

	MI_U8 *u8CopyData = (MI_U8 *)bufInfo->b.buf;
	MI_U32 *pu32length = (MI_U32 *)&bufInfo->length;

	MI_S32 s32Ret = MI_SUCCESS;
	if((V4L2_PIX_FMT_H264 == pstDev->setting.fcc) 
		|| (V4L2_PIX_FMT_H265 == pstDev->setting.fcc))
	{
		MI_VENC_Stream_t stStream;
		const unsigned int vencPackNum = 4;
		MI_VENC_Pack_t stPack[vencPackNum];
		memset(&stStream, 0, sizeof(MI_VENC_Stream_t));
		memset(&stPack, 0, sizeof(MI_VENC_Pack_t) * vencPackNum);
		stStream.pstPack = stPack;

		MI_VENC_ChnStat_t stStat;
		memset(&stStat, 0, sizeof(MI_VENC_ChnStat_t));
		s32Ret = MI_VENC_Query(MI_VENC_DEV_ID_H264_H265_0, vencChn, &stStat);
		if(MI_SUCCESS != s32Ret)	// 如果出错了，
		{
			cerr << "Fail to call MI_VENC_Query() in UVC_MM_FillBuffer()."
				<< "errno = 0x" << hex << s32Ret << dec << endl;
			return -EINVAL;
		}
		else if(0 == stStat.u32CurPacks)	// 查询成功，但没数据，则直接返回。
		{
			return -EINVAL;
		}
		stStream.u32PackCount = stStat.u32CurPacks;

		const unsigned int outTimeMs = 40;				// 40ms 没获得视频数据，就返回。
		s32Ret = MI_VENC_GetStream(MI_VENC_DEV_ID_H264_H265_0, vencChn, &stStream, outTimeMs);
		if(MI_SUCCESS != s32Ret)
		{
			cerr << "Fail to call MI_VENC_GetStream() in UVC_MM_FillBuffer()."
				<< "errno = 0x" << hex << s32Ret << dec << endl;
			return -EINVAL;
		}

		if(((V4L2_PIX_FMT_H264 == pstDev->setting.fcc) && (E_MI_VENC_BASE_IDR == stStream.stH264Info.eRefType)) ||
			((V4L2_PIX_FMT_H265 == pstDev->setting.fcc) && (E_MI_VENC_BASE_IDR == stStream.stH265Info.eRefType)))
		{
			bufInfo->is_keyframe = true;
		}
		else
		{
			bufInfo->is_keyframe = false;
		}

		int i = 0;
		for(i = 0; i < stStat.u32CurPacks; ++i)
		{
			MI_U32 u32Size = 0;
			u32Size = stStream.pstPack[i].u32Len;
			memcpy(u8CopyData,stStream.pstPack[i].pu8Addr, u32Size);
			u8CopyData += u32Size;
		}
		*pu32length = u8CopyData - (MI_U8 *)bufInfo->b.buf;

		bufInfo->is_tail = true;	//default is frameEnd
		s32Ret = MI_VENC_ReleaseStream(MI_VENC_DEV_ID_H264_H265_0, vencChn, &stStream);
		if(MI_SUCCESS != s32Ret)
		{
			cerr << "Fail to call MI_VENC_ReleaseStream() in UVC_MM_FillBuffer()."
				<< "errno = 0x" << hex << s32Ret << dec << endl;
		}

		if(pstDev->bForceIdr && (Venc::vencJpegChn != vencChn))
		{
			cout << "In UVC_MM_FillBuffer(). Force Idr." << endl;
			s32Ret = MI_VENC_RequestIdr(MI_VENC_DEV_ID_H264_H265_0, vencChn, TRUE);
			if(MI_SUCCESS != s32Ret)
			{
				cerr << "Fail to call MI_VENC_RequestIdr() in UVC_MM_FillBuffer()."
					<< "errno = 0x" << hex << s32Ret << dec << endl;
			}
			else
			{
				pstDev->bForceIdr = false;
			}
		}
	}
	else if(V4L2_PIX_FMT_MJPEG == pstDev->setting.fcc)
	{
		MI_VENC_Stream_t stStream;
		const unsigned int vencPackNum = 4;
		MI_VENC_Pack_t stPack[vencPackNum];
		memset(&stStream, 0, sizeof(MI_VENC_Stream_t));
		memset(&stPack, 0, sizeof(MI_VENC_Pack_t) * vencPackNum);
		stStream.pstPack = stPack;

		MI_VENC_ChnStat_t stStat;
		memset(&stStat, 0, sizeof(MI_VENC_ChnStat_t));
		s32Ret = MI_VENC_Query(MI_VENC_DEV_ID_JPEG_0, vencChn, &stStat);
		if(MI_SUCCESS != s32Ret)	// 如果出错了，
		{
			cerr << "Fail to call MI_VENC_Query() in UVC_MM_FillBuffer()."
				<< "errno = 0x" << hex << s32Ret << dec << endl;
			return -EINVAL;
		}
		else if(0 == stStat.u32CurPacks)	// 查询成功，但没数据，则直接返回。
		{
			return -EINVAL;
		}
		stStream.u32PackCount = stStat.u32CurPacks;

		const unsigned int outTimeMs = 40;				// 40ms 没获得视频数据，就返回。
		s32Ret = MI_VENC_GetStream(MI_VENC_DEV_ID_JPEG_0, vencChn, &stStream, outTimeMs);
		if(MI_SUCCESS != s32Ret)
		{
			cerr << "Fail to call MI_VENC_GetStream() in UVC_MM_FillBuffer()."
				<< "errno = 0x" << hex << s32Ret << dec << endl;
			return -EINVAL;
		}

		int i = 0;
		for(i = 0; i < stStat.u32CurPacks; ++i)
		{
			MI_U32 u32Size = 0;
			u32Size = stStream.pstPack[i].u32Len;
			memcpy(u8CopyData,stStream.pstPack[i].pu8Addr, u32Size);
			u8CopyData += u32Size;
		}
		*pu32length = u8CopyData - (MI_U8 *)bufInfo->b.buf;

		bufInfo->is_tail = true;	//default is frameEnd
		s32Ret = MI_VENC_ReleaseStream(MI_VENC_DEV_ID_JPEG_0, vencChn, &stStream);
		if(MI_SUCCESS != s32Ret)
		{
			cerr << "Fail to call MI_VENC_ReleaseStream() in UVC_MM_FillBuffer()."
				<< "errno = 0x" << hex << s32Ret << dec << endl;
		}
	}
	else if(V4L2_PIX_FMT_YUYV == pstDev->setting.fcc)
	{
		MI_SYS_BufInfo_t stBufInfo;
		memset(&stBufInfo, 0, sizeof(MI_SYS_BufInfo_t));
		MI_SYS_ChnPort_t dstChnPort;
		memset(&dstChnPort, 0, sizeof(MI_SYS_ChnPort_t));
		MI_SYS_BUF_HANDLE stBufHandle;
		memset(&stBufHandle, 0, sizeof(MI_SYS_BUF_HANDLE));
		
		dstChnPort.eModId = E_MI_MODULE_ID_VPE;
		dstChnPort.u32DevId = 0;
		//dstChnPort.u32PortId = Vpe::vpeSubPort;
		//dstChnPort.u32ChnId = Vpe::vpeCh;
		s32Ret = MI_SYS_ChnOutputPortGetBuf(&dstChnPort, &stBufInfo, &stBufHandle);
		if(MI_SUCCESS != s32Ret)
		{
			// 正常现象，不用打印。
			//cerr << "Fail to call MI_SYS_ChnOutputPortGetBuf() in UVC_MM_FillBuffer(). "
			//	<< "s32Ret = " << s32Ret << endl;
			return -EINVAL;
		}
		
		*pu32length = stBufInfo.stFrameData.u16Height * stBufInfo.stFrameData.u32Stride[0];
		memcpy(u8CopyData, stBufInfo.stFrameData.pVirAddr[0], *pu32length);

		s32Ret = MI_SYS_ChnOutputPortPutBuf(stBufHandle);
		if(MI_SUCCESS != s32Ret)
		{
			cerr << "Fail to call MI_SYS_ChnOutputPortPutBuf() in UVC_MM_FillBuffer(). "
				<< "errno = 0x" << hex << s32Ret << dec << endl;
		}
	}

	return MI_SUCCESS;
}

/*-----------------------------------------------------------------------------
描--述：
参--数：
返回值：
注--意：实现为空，没有看到被调用。
-----------------------------------------------------------------------------*/
static MI_S32 UVC_Init(void *uvc)
{
	cout << "Call UVC_Init()." << endl;
	cout << "Call UVC_Init() end." << endl;
	return MI_SUCCESS;
}

/*-----------------------------------------------------------------------------
描--述：
参--数：
返回值：
注--意：实现为空，没有看到被调用。
-----------------------------------------------------------------------------*/
static MI_S32 UVC_Deinit(void *uvc)
{
	cout << "Call UVC_Deinit()." << endl;
	cout << "Call UVC_Deinit() end." << endl;
	return MI_SUCCESS;
}

/*-----------------------------------------------------------------------------
描--述：请求开始传图
参--数：uvc, 指向UVC 设备；
		format 包含视频信息。
返回值：成功，返回MI_SUCCESS.
注--意：1.当PC 播放器请求数据时，该函数被底层SDK 调用，并将值传给format.
		我们从format 中获取视频的目标分辨率、帧率、格式等信息。
		2.只在被请求数据时调用一次。
		3.capture 是传图而非抓图的意思
-----------------------------------------------------------------------------*/
static MI_S32 UVC_StartCapture(void *uvc, Stream_Params_t format)
{
	cout << "Call UVC_StartCapture()." << endl;
	
	ST_UvcDev_t *pstDev = (ST_UvcDev_t *)uvc;
	if(NULL == pstDev)
	{
		cerr << "Fail to call UVC_StartCapture(). Argument has null value. Uvc device does not exists" << endl;
		return -1;
	}

	memset(&pstDev->setting, 0, sizeof(ST_UvcSetting_Attr_T));
	pstDev->setting.fcc = format.fcc;
	pstDev->setting.u32Width = format.width;
	pstDev->setting.u32Height = format.height;
	pstDev->setting.u32FrameRate = format.frameRate;

	printf("In UVC_StartCapture, {width, height, fps} = {%d, %d, %2f}.\n", 
						format.width, format.height, format.frameRate);

	MI_S32 s32Ret = 0;
	MI_VENC_CHN vencCh = 0;
	Sensor *pSensor = Sensor::getInstance();
	Sys *pSys = Sys::getInstance();
	Scl *pScl = Scl::getInstance();
	Venc *pVenc = Venc::getInstance();

	switch(pstDev->setting.fcc)
	{
		case V4L2_PIX_FMT_YUYV:
		{
			cout << "V4L2 pixel format = V4L2_PIX_FMT_YUYV" << endl;
			//MI_VPE_PortMode_t stVpeMode;
			//memset(&stVpeMode, 0, sizeof(stVpeMode));
			//stVpeMode.bFlip = FALSE;
			//stVpeMode.bMirror = FALSE;
			//stVpeMode.u16Width = format.width;
			//stVpeMode.u16Height= format.height;
			//stVpeMode.eCompressMode = E_MI_SYS_COMPRESS_MODE_NONE;
			//stVpeMode.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;

			//s32Ret = MI_VPE_SetPortMode(Vpe::vpeCh, Vpe::vpeSubPort, &stVpeMode);
			if(0 != s32Ret)
			{
				cerr << "Fail to call MI_VPE_SetPortMode() in UVC_StartCapture(). "
					<< "errno = 0x" << hex << s32Ret << dec << endl;
			}

			MI_SYS_ChnPort_t stChnPort;
			memset(&stChnPort, 0, sizeof(MI_SYS_ChnPort_t));
			stChnPort.eModId = E_MI_MODULE_ID_VPE;
			stChnPort.u32DevId = 0;
			//stChnPort.u32ChnId = Vpe::vpeCh;
			//stChnPort.u32PortId = Vpe::vpeSubPort;

			/* 虽然后续会再次调用 MI_SYS_SetChnOutputPortDepth(), 但此处的也不可少 */
			s32Ret = MI_SYS_SetChnOutputPortDepth(MI_VENC_DEV_ID_H264_H265_0, &stChnPort, 0, 5);
			if(0 != s32Ret)
			{
				cerr << "Fail to call MI_SYS_SetChnOutputPortDepth() in UVC_StartCapture(). "
					<< "errno = 0x" << hex << s32Ret << dec << endl;
			}

			//s32Ret = MI_VPE_EnablePort(Vpe::vpeCh, Vpe::vpeSubPort);
			if(0 != s32Ret)
			{
				cerr << "Fail to call MI_VPE_EnablePort(). s32Ret = " << s32Ret << endl;
			}

			s32Ret = MI_SYS_SetChnOutputPortDepth(0, &stChnPort, 4, 5);
			if(0 != s32Ret)
			{
				cerr << "Fail to call MI_SYS_SetChnOutputPortDepth() in UVC_StartCapture(). "
					<< "errno = 0x" << hex << s32Ret << dec << endl;
			}
			break;
		}
		case V4L2_PIX_FMT_NV12:
		{
			cerr << "V4L2 pixel format = V4L2_PIX_FMT_NV12. Not support." 
					<< "Just support: H.264/265, MJPEG, YUYV422." << endl;
			break;
		}
		case V4L2_PIX_FMT_MJPEG:
		{
			cout << "V4L2 pixel format = V4L2_PIX_FMT_MJPEG" << endl;
			bool bIsJpeg = true;
			vencCh = Venc::vencJpegChn;
			pScl->createPort(Scl::sclPortMain, format.width, format.height, bIsJpeg);
			pVenc->createJpegStream(MI_VENC_DEV_ID_JPEG_0, vencCh, format.width, format.height);
			//pVenc->changeBitrate(MI_VENC_DEV_ID_JPEG_0, vencCh, 5);
			//pVenc->setInputBufMode(MI_VENC_DEV_ID_H264_H265_0, vencCh, E_MI_VENC_INPUT_MODE_RING_ONE_FRM);
			pVenc->startRecvPic(MI_VENC_DEV_ID_JPEG_0, vencCh);
			pSys->bindScl2Venc(Scl::sclPortMain, MI_VENC_DEV_ID_JPEG_0, vencCh, 30, 30, E_MI_SYS_BIND_TYPE_REALTIME, 0);
			break;
		}
		case V4L2_PIX_FMT_H264:
		{
			cout << "V4L2 pixel format = V4L2_PIX_FMT_H264" << endl;
			vencCh = Venc::vencMainChn;
			pScl->createPort(Scl::sclPortMain, format.width, format.height);
			pVenc->createH264Stream(MI_VENC_DEV_ID_H264_H265_0, vencCh, format.width, format.height);
			pVenc->changeBitrate(MI_VENC_DEV_ID_H264_H265_0, vencCh, 1 * 1024);
			pVenc->setInputBufMode(MI_VENC_DEV_ID_H264_H265_0, vencCh, E_MI_VENC_INPUT_MODE_RING_ONE_FRM);
			pVenc->startRecvPic(MI_VENC_DEV_ID_H264_H265_0, vencCh);
			pSys->bindScl2Venc(Scl::sclPortMain, MI_VENC_DEV_ID_H264_H265_0, vencCh, 30, 30, E_MI_SYS_BIND_TYPE_HW_RING, format.height);
			break;
		}
		case V4L2_PIX_FMT_H265:
		{
			cout << "V4L2 pixel format = V4L2_PIX_FMT_H265" << endl;
			vencCh = Venc::vencMainChn;
			pScl->createPort(Scl::sclPortMain, format.width, format.height);
			pVenc->createH265Stream(MI_VENC_DEV_ID_H264_H265_0, vencCh, format.width, format.height);
			pVenc->changeBitrate(MI_VENC_DEV_ID_H264_H265_0, vencCh, 1 * 1024);
			pVenc->setInputBufMode(MI_VENC_DEV_ID_H264_H265_0, vencCh, E_MI_VENC_INPUT_MODE_RING_ONE_FRM);
			pVenc->startRecvPic(MI_VENC_DEV_ID_H264_H265_0, vencCh);
			pSys->bindScl2Venc(Scl::sclPortMain, MI_VENC_DEV_ID_H264_H265_0, vencCh, 30, 30, E_MI_SYS_BIND_TYPE_HW_RING, format.height);
			break;
		}
		default:
			cout << "V4L2 pixel format = what? Format is no recognized."
				<< "Just support: H.264/265, MJPEG, YUYV422." << endl;
			break;
	}

	if((V4L2_PIX_FMT_H264 == pstDev->setting.fcc)
		|| (V4L2_PIX_FMT_H265 == pstDev->setting.fcc))
	{
		s32Ret = MI_VENC_SetMaxStreamCnt(MI_VENC_DEV_ID_H264_H265_0, vencCh, pstDev->u8MaxBufCnt + 1);
		if(MI_SUCCESS != s32Ret)
		{
			cerr << "Fail to call MI_VENC_SetMaxStreamCnt() in UVC_StartCapture(). " 
			<< "errno = 0x" << hex << s32Ret << dec << endl;
			return s32Ret;
		}
	}

	if(V4L2_PIX_FMT_MJPEG == pstDev->setting.fcc)
	{
		s32Ret = MI_VENC_SetMaxStreamCnt(MI_VENC_DEV_ID_JPEG_0, vencCh, pstDev->u8MaxBufCnt + 1);
		if(MI_SUCCESS != s32Ret)
		{
			cerr << "Fail to call MI_VENC_SetMaxStreamCnt() in UVC_StartCapture(). " 
			<< "errno = 0x" << hex << s32Ret << dec << endl;
			return s32Ret;
		}
	}

	pSensor->setFps(format.frameRate);

	static bool bFirstRun = true;
	if(bFirstRun)
	{
		cout << "First run app, load iqfile." << endl;
		bFirstRun = false;
		Isp *pIsp = Isp::getInstance();
		pIsp->loadBinFile((char *)"/config/iqfile/335_imx291_day.bin");
	}

	cout << "Call UVC_StartCapture() end." << endl;
	return MI_SUCCESS;
}

/*-----------------------------------------------------------------------------
描--述：请求停止传图
参--数：uvc, 指向UVC 设备；
返回值：成功，返回MI_SUCCESS.
注--意：1.当PC 播放器请求停止传输时，该函数被底层SDK 调用，并将值传给uvc.
		2.只在被请求数据时调用一次。
		3.capture 是传图而非抓图的意思
-----------------------------------------------------------------------------*/
static MI_S32 UVC_StopCapture(void *uvc)
{
	cout << "Call UVC_StopCapture()." << endl;
	
	ST_UvcDev_t *pstDev = (ST_UvcDev_t *)uvc;
	if(NULL == pstDev)
	{
		cerr << "Fail to call UVC_StopCapture(). Argument has null value. Uvc device does not exists" << endl;
		return -1;
	}

	MI_VENC_CHN vencCh = -1;
	switch(pstDev->setting.fcc)
	{
		case V4L2_PIX_FMT_YUYV:
		{
			cout << "V4L2 pixel format = V4L2_PIX_FMT_YUYV" << endl;
			//Vpe *pVpe = Vpe::getInstance();
			//pVpe->stopChannel();
			//pVpe->destroyChannel();
			//pVpe->disablePort(Vpe::vpeSubPort);
			return MI_SUCCESS;
			break;
		}
		case V4L2_PIX_FMT_NV12:
		{
			cout << "V4L2 pixel format = V4L2_PIX_FMT_NV12" << endl;
			break;
		}
		case V4L2_PIX_FMT_MJPEG:
		{
			cout << "V4L2 pixel format = V4L2_PIX_FMT_MJPEG" << endl;
			vencCh = Venc::vencJpegChn;
			break;
		}
		case V4L2_PIX_FMT_H264:
		{
			cout << "V4L2 pixel format = V4L2_PIX_FMT_H264" << endl;
			vencCh = Venc::vencMainChn;
			break;
		}
		case V4L2_PIX_FMT_H265:
		{
			cout << "V4L2 pixel format = V4L2_PIX_FMT_H265" << endl;
			vencCh = Venc::vencMainChn;
			break;
		}
		default:
			cout << "V4L2 pixel format = what? Format is no recognized." << endl;
			break;
	}

	Scl *pScl = Scl::getInstance();
	Venc *pVenc = Venc::getInstance();
	if((V4L2_PIX_FMT_H264 == pstDev->setting.fcc)
		|| (V4L2_PIX_FMT_H265 == pstDev->setting.fcc))
	{
		pVenc->stopRecvPic(MI_VENC_DEV_ID_H264_H265_0, vencCh);
		pVenc->destroyChn(MI_VENC_DEV_ID_H264_H265_0, vencCh);
		pVenc->destroyDev(MI_VENC_DEV_ID_H264_H265_0);
		pScl->destoryPort(Scl::sclPortMain);
	}

	if(V4L2_PIX_FMT_MJPEG == pstDev->setting.fcc)
	{
		pVenc->stopRecvPic(MI_VENC_DEV_ID_JPEG_0, vencCh);
		pVenc->destroyChn(MI_VENC_DEV_ID_JPEG_0, vencCh);
		pVenc->destroyDev(MI_VENC_DEV_ID_JPEG_0);
		pScl->destoryPort(Scl::sclPortMain);
	}

	cout << "Call UVC_StopCapture() end." << endl;
	return MI_SUCCESS;
}

/*-----------------------------------------------------------------------------
描--述：请求I 帧。
参--数：
返回值：
注--意：可以实现为空，没有看到被调用。
-----------------------------------------------------------------------------*/
static void UVC_ForceIdr(void *uvc)
{
	cout << "Call UVC_ForceIdr()." << endl;
	ST_UvcDev_t *pstDev = (ST_UvcDev_t *)uvc;
	if(NULL == pstDev)
	{
		cerr << "Fail to call UVC_ForceIdr(). Argument has null value. Uvc device does not exists" << endl;
		return;
	}

	pstDev->bForceIdr = true;

	cout << "Call UVC_ForceIdr() end." << endl;
	return;
}

/*-----------------------------------------------------------------------------
描--述：开启UVC。
参--数：
返回值：返回0.
注--意：
-----------------------------------------------------------------------------*/
int UacUvc::startUvc()
{
	cout << "Call UacUvc::startUvc()." << endl;
	// step 1
	MI_S32 trace_level = UVC_DBG_ERR;
	ST_UVC_SetTraceLevel(trace_level);

	memset(&stUvcDev, 0, sizeof(ST_UvcDev_t));
	memcpy(stUvcDev.name, "/dev/video0", sizeof(stUvcDev.name));
	stUvcDev.u8MaxBufCnt = u8MaxBufCnt;

	// step 2
	uint32_t maxpacket = 1024;	// 不知道啥含义
	MI_U8 mult = 2;				// 不知道啥含义
	MI_U8 burst = 13;			// 不知道啥含义
	MI_U8 c_intf = 0;			// 不知道啥含义
	MI_U8 s_intf = 0;			// 不知道啥含义
	UVC_IO_MODE_e mode = UVC_MEMORY_MMAP;	// MMA 映射方式
	Transfer_Mode_e type = USB_ISOC_MODE;	// 不知道啥含义
	ST_UVC_Setting_t pstSet={u8MaxBufCnt, maxpacket, mult, burst, c_intf, s_intf, mode, type};

	ST_UVC_MMAP_BufOpts_t mmaOpts = {UVC_MM_FillBuffer};
	ST_UVC_OPS_t fops = {	UVC_Init,			// 初始化，实现为空。
							UVC_Deinit,			// 反初始化，实现为空。
							{{}},
							UVC_StartCapture,	// 开始传图。
							UVC_StopCapture,	// 停止传图。
							UVC_ForceIdr		// 强制I 帧，实现为空。
						};
	fops.m = mmaOpts;
	ST_UVC_ChnAttr_t stUvcChAttr = {pstSet,fops};

	printf(ASCII_COLOR_YELLOW "ST_UvcInitDev: name:%s bufcnt:%d mult:%d burst:%d ci:%d si:%d, Mode:%s, Type:%s" ASCII_COLOR_END "\n",
			stUvcDev.name, stUvcDev.u8MaxBufCnt, mult, burst, c_intf, s_intf, mode==UVC_MEMORY_MMAP?"mmap":"userptr", type==USB_ISOC_MODE?"isoc":"bulk");

	// step 3
	ST_UVC_Init(stUvcDev.name, &stUvcDev.handle);
	ST_UVC_CreateDev(stUvcDev.handle, &stUvcChAttr);
	ST_UVC_StartDev(stUvcDev.handle);
	
	cout << "Call UacUvc::startUvc() end." << endl;
	return 0;
}

/*-----------------------------------------------------------------------------
描--述：停止UVC。
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
int UacUvc::stopUvc()
{
	cout << "Call UacUvc::stopUvc()." << endl;

	//Vpe *pVpe = Vpe::getInstance();
	Venc *pVenc = Venc::getInstance();
	pVenc->stopRecvPic(MI_VENC_DEV_ID_H264_H265_0, Venc::vencMainChn);
	pVenc->destroyChn(MI_VENC_DEV_ID_H264_H265_0, Venc::vencMainChn);
	//pVpe->disablePort(Vpe::vpeMainPort);

	/* 与SDK 不同，SDK 中是：
	ST_UVC_StopDev((dev->handle));		// 不是指针。
	ST_UVC_DestroyDev(dev->handle);
	ST_UVC_Uninit(dev->handle);
	*/
	ST_UVC_StopDev(&stUvcDev.handle);
	ST_UVC_DestroyDev(stUvcDev.handle);
	ST_UVC_Uninit(stUvcDev.handle);

	cout << "Call UacUvc::stopUvc() end." << endl;
	return 0;
}

