/*---------------------------------------------------------------- 
xxx版权所有。
作者：
时间：2020.7.10
----------------------------------------------------------------*/

#include "ai.hpp"
#include "venc.h"
#include "sys.h"
#include "sensor.h"
#include "isp.h"
#include "scl.h"
#include "uac_uvc.h"
#include "st_common.h"
#include "thread"

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
	startUvc();
	startUac();
	cout << "Call UacUvc::UacUvc() end." << endl;
}

UacUvc::~UacUvc()
{
	cout << "Call UacUvc::~UacUvc()." << endl;
	stopUvc();
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
	//return MI_SUCCESS;

	ST_UvcDev_t *pstDev = (ST_UvcDev_t *)uvc;
	if(NULL == pstDev)
	{
		cerr << "Fail to call UVC_MM_FillBuffer(). Argument has null value. Uvc device does not exists" << endl;
		return -1;
	}

	MI_U8 *u8CopyData = (MI_U8 *)bufInfo->b.buf;
	MI_U32 *pu32length = (MI_U32 *)&bufInfo->length;

	MI_S32 s32Ret = MI_SUCCESS;
	MI_VENC_Stream_t stStream;
	MI_VENC_Pack_t stPack[4];
	MI_VENC_ChnStat_t stStat;

	MI_U32 vencChn = 0;
	switch(pstDev->setting.fcc)
	{
		case V4L2_PIX_FMT_YUYV:
		{
			#if 0
			memset(&stBufInfo, 0, sizeof(MI_SYS_BufInfo_t));
            memset(&stBufHandle, 0, sizeof(MI_SYS_BUF_HANDLE));

            s32Ret = MI_SYS_ChnOutputPortGetBuf(&dstChnPort, &stBufInfo, &stBufHandle);
            if(MI_SUCCESS!=s32Ret)
            {
                return -EINVAL;
            }

            *pu32length = stBufInfo.stFrameData.u16Height * stBufInfo.stFrameData.u32Stride[0];
            memcpy(u8CopyData, stBufInfo.stFrameData.pVirAddr[0], *pu32length);

            s32Ret = MI_SYS_ChnOutputPortPutBuf(stBufHandle);
            if(MI_SUCCESS!=s32Ret)
                printf("%s Release Frame Failed\n", __func__);
            #endif
		}
		case V4L2_PIX_FMT_NV12:
		{
			#if 0
			memset(&stBufInfo, 0, sizeof(MI_SYS_BufInfo_t));
            memset(&stBufHandle, 0, sizeof(MI_SYS_BUF_HANDLE));

            s32Ret = MI_SYS_ChnOutputPortGetBuf(&dstChnPort, &stBufInfo, &stBufHandle);
            if(MI_SUCCESS!=s32Ret)
                return -EINVAL;

            *pu32length = stBufInfo.stFrameData.u16Height
                    * (stBufInfo.stFrameData.u32Stride[0] + stBufInfo.stFrameData.u32Stride[1] / 2);
            memcpy(u8CopyData, stBufInfo.stFrameData.pVirAddr[0],
                            stBufInfo.stFrameData.u16Height * stBufInfo.stFrameData.u32Stride[0]);
            u8CopyData += stBufInfo.stFrameData.u16Height * stBufInfo.stFrameData.u32Stride[0];
            memcpy(u8CopyData, stBufInfo.stFrameData.pVirAddr[1],
                            stBufInfo.stFrameData.u16Height * stBufInfo.stFrameData.u32Stride[1]/2);

            s32Ret = MI_SYS_ChnOutputPortPutBuf(stBufHandle);
            if(MI_SUCCESS!=s32Ret)
                printf("%s Release Frame Failed\n", __func__);
            #endif
			break;
		}
		case V4L2_PIX_FMT_MJPEG:
		case V4L2_PIX_FMT_H264:
		case V4L2_PIX_FMT_H265:
		{
			int vencChnId = 0;
			int vencDevId = 0;
			vencDevId = (V4L2_PIX_FMT_MJPEG == pstDev->setting.fcc) ? MI_VENC_DEV_ID_JPEG_0 : MI_VENC_DEV_ID_H264_H265_0;
			memset(&stStream, 0, sizeof(MI_VENC_Stream_t));
			memset(&stPack, 0, sizeof(MI_VENC_Pack_t) * 4);
			stStream.pstPack = stPack;

			s32Ret = MI_VENC_Query(vencDevId, vencChnId, &stStat);
			if(0 != s32Ret || 0 == stStat.u32CurPacks)
			{
				//cerr << "Fail to call pVenc->query() in UVC_MM_FillBuffer()."
				//	<< "errno = 0x" << hex << s32Ret << dec << endl;
				return -EINVAL;
			}

			stStream.u32PackCount = stStat.u32CurPacks;
			s32Ret = MI_VENC_GetStream(vencDevId, vencChnId, &stStream, 5);
			if(0 != s32Ret)
			{
				cerr << "Fail to MI_VENC_GetStream() in UVC_MM_FillBuffer()."
					<< "errno = 0x" << hex << s32Ret << dec << endl;
				return -EINVAL;
			}
			
			if(((V4L2_PIX_FMT_H264 == pstDev->setting.fcc) && (E_MI_VENC_BASE_IDR == stStream.stH264Info.eRefType)) ||
				((V4L2_PIX_FMT_H265 == pstDev->setting.fcc) && (E_MI_VENC_BASE_IDR == stStream.stH265Info.eRefType)))
			{
				bufInfo->is_keyframe = TRUE;
			}
			else
			{
				bufInfo->is_keyframe = FALSE;
			}

			int i = 0;
			for(i = 0; i < stStat.u32CurPacks; ++i)
			{
				MI_U32 u32Size = 0;
				u32Size = stStream.pstPack[i].u32Len;
				memcpy(u8CopyData, stStream.pstPack[i].pu8Addr, u32Size);
				u8CopyData += u32Size;
			}
			*pu32length = u8CopyData - (MI_U8 *)bufInfo->b.buf;
			bufInfo->is_tail = TRUE; //default is frameEnd

			s32Ret = MI_VENC_ReleaseStream(vencDevId, vencChnId, &stStream);
			if(0 != s32Ret)
			{
				cerr << "Fail to call pVenc->releaseStream() in UVC_MM_FillBuffer()."
					<< "errno = 0x" << hex << s32Ret << dec << endl;
			}

			if(pstDev->bForceIdr && (V4L2_PIX_FMT_MJPEG != pstDev->setting.fcc))
			{
				pstDev->bForceIdr = FALSE;
				s32Ret = MI_VENC_RequestIdr(vencDevId, vencChnId, TRUE);
				if(0 != s32Ret)
				{
					cerr << "Fail to call pVenc->requestIdr() in UVC_MM_FillBuffer()."
						<< "errno = 0x" << hex << s32Ret << dec << endl;
				}
			}
			break;
		}
		default:
		{
			cerr << "In UVC_MM_FillBuffer(). Format format = " << pstDev->setting.fcc
				<< ". Just support: H.264/265, MJPEG, YUYV422." << endl;
			return -EINVAL;
			break;
		}
	}

	//cout << "Call UVC_MM_FillBuffer() end." << endl;
	return MI_SUCCESS;
}

/*-----------------------------------------------------------------------------
描--述：
参--数：
返回值：
注--意：可以实现为空。
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


	/************************************************
	Step1:  Init Isp Output Param
	*************************************************/
	Isp *pIsp = Isp::getInstance();
	MI_ISP_OutPortParam_t stIspOutputParam;
	memset(&stIspOutputParam, 0, sizeof(MI_ISP_OutPortParam_t));
	pIsp->getInputPortCrop(&stIspOutputParam.stCropRect);

	/************************************************
	Step2:  Init Scl Output Param
	*************************************************/

	MI_SCL_OutPortParam_t stSclOutputParam;
	memset(&stSclOutputParam, 0x0, sizeof(MI_SCL_OutPortParam_t));
	stSclOutputParam.stSCLOutCropRect.u16X = stIspOutputParam.stCropRect.u16X;
	stSclOutputParam.stSCLOutCropRect.u16Y = stIspOutputParam.stCropRect.u16Y;
	stSclOutputParam.stSCLOutCropRect.u16Width = stIspOutputParam.stCropRect.u16Width;
	stSclOutputParam.stSCLOutCropRect.u16Height = stIspOutputParam.stCropRect.u16Height;
	stSclOutputParam.stSCLOutputSize.u16Width = format.width;
	stSclOutputParam.stSCLOutputSize.u16Height = format.height;
	stSclOutputParam.bMirror = FALSE;
	stSclOutputParam.bFlip = FALSE;
	stSclOutputParam.eCompressMode= E_MI_SYS_COMPRESS_MODE_NONE;

	/************************************************
	Step3:  Init Venc Param
	*************************************************/
	MI_VENC_ChnAttr_t stVencChnAttr;
	MI_VENC_InputSourceConfig_t stVencInputSourceConfig;
	memset(&stVencChnAttr, 0, sizeof(MI_VENC_ChnAttr_t));
	memset(&stVencInputSourceConfig, 0, sizeof(MI_VENC_InputSourceConfig_t));

	MI_U32 u32VenBitRate = 1 * 1024 * 1024;
	MI_U32 u32VenQfactor = 45;
	bool bByFrame = TRUE;

	/************************************************
	Step4:  Init Bind Param
	*************************************************/
	Sensor *pSensor = Sensor::getInstance();
	pSensor->setFps(format.frameRate);

	/************************************************
	Step5:  Start
	*************************************************/

	MI_S32 s32Ret = 0;
	Sys *pSys = Sys::getInstance();
	Scl *pScl = Scl::getInstance();
	Venc *pVenc = Venc::getInstance();

	switch(pstDev->setting.fcc)
	{
		case V4L2_PIX_FMT_YUYV:
		case V4L2_PIX_FMT_NV12:
		{
			cerr << "V4L2 pixel format = V4L2_PIX_FMT_NV12. Not support."
					<< "Just support: H.264/265, MJPEG, YUYV422." << endl;
			#if 0
            stIspOutputParam.ePixelFormat = (fcc == V4L2_PIX_FMT_YUYV) ? E_MI_SYS_PIXEL_FRAME_YUV422_YUYV : E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
            stSclOutputParam.ePixelFormat = (fcc == V4L2_PIX_FMT_YUYV) ? E_MI_SYS_PIXEL_FRAME_YUV422_YUYV : E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;

            STCHECKRESULT(MI_ISP_SetOutputPortParam(IspDevId, IspChnId, IspPortId, &stIspOutputParam));
            STCHECKRESULT(MI_SCL_SetOutputPortParam(SclDevId, SclChnId, SclPortId, &stSclOutputParam));

            STCHECKRESULT(ST_Sys_Bind(&stBindInfo[0]));
            STCHECKRESULT(MI_SYS_SetChnOutputPortDepth(0, &stChnPort[0] , 0, g_maxbuf_cnt+2));
            STCHECKRESULT(MI_SYS_SetChnOutputPortDepth(0, &stChnPort[1] , g_maxbuf_cnt+1, g_maxbuf_cnt+2));
            if (g_bEnableLDC)
                STCHECKRESULT(MI_SYS_SetChnOutputPortDepth(0, &stChnPort[2] , 2, g_maxbuf_cnt+2));
            STCHECKRESULT(MI_ISP_EnableOutputPort(IspDevId, IspChnId, IspPortId));
            STCHECKRESULT(MI_SCL_EnableOutputPort(SclDevId, SclChnId, SclPortId));
            *dstChnPort = stBindInfo[0].stDstChnPort;
			#endif
			break;
		}
		case V4L2_PIX_FMT_MJPEG:
		case V4L2_PIX_FMT_H264:
		case V4L2_PIX_FMT_H265:
		{
			if(V4L2_PIX_FMT_MJPEG == format.fcc)
			{
				stIspOutputParam.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;
				stSclOutputParam.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;
			}
			else
			{
				stIspOutputParam.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
				stSclOutputParam.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
			}

			int sclPortId = Scl::sclPortMain;
			pScl->setOutputPortParam(sclPortId, &stSclOutputParam);
			pIsp->setOutputPortParam(&stIspOutputParam);

			int vencDevId = 0;
			int vencChnId = 0;
			vencDevId = (V4L2_PIX_FMT_MJPEG == format.fcc) ? MI_VENC_DEV_ID_JPEG_0 : MI_VENC_DEV_ID_H264_H265_0;

			if(V4L2_PIX_FMT_MJPEG == format.fcc)
			{
				stVencChnAttr.stVeAttr.stAttrJpeg.u32PicWidth = format.width;
				stVencChnAttr.stVeAttr.stAttrJpeg.u32PicHeight = format.height;
				stVencChnAttr.stVeAttr.stAttrJpeg.u32MaxPicWidth = format.width;
				stVencChnAttr.stVeAttr.stAttrJpeg.u32MaxPicHeight = ALIGN_UP(format.height, 16);
				stVencChnAttr.stVeAttr.stAttrJpeg.bByFrame = bByFrame;
				stVencChnAttr.stVeAttr.eType = E_MI_VENC_MODTYPE_JPEGE;
				stVencChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_MJPEGFIXQP;
				stVencChnAttr.stRcAttr.stAttrMjpegFixQp.u32Qfactor = u32VenQfactor;
				stVencChnAttr.stRcAttr.stAttrMjpegFixQp.u32SrcFrmRateNum = format.frameRate;
				stVencChnAttr.stRcAttr.stAttrMjpegFixQp.u32SrcFrmRateDen = 1;
			}
			else if(V4L2_PIX_FMT_H264 == format.fcc)
			{
				stVencChnAttr.stVeAttr.stAttrH264e.u32PicWidth = format.width;
				stVencChnAttr.stVeAttr.stAttrH264e.u32PicHeight = format.height;
				stVencChnAttr.stVeAttr.stAttrH264e.u32MaxPicWidth = format.width;
				stVencChnAttr.stVeAttr.stAttrH264e.u32MaxPicHeight = ALIGN_UP(format.height, 16);
				stVencChnAttr.stVeAttr.stAttrH264e.bByFrame = bByFrame;
				stVencChnAttr.stVeAttr.stAttrH264e.u32BFrameNum = 2;
				stVencChnAttr.stVeAttr.stAttrH264e.u32Profile = 1;
				stVencChnAttr.stVeAttr.eType = E_MI_VENC_MODTYPE_H264E;
				stVencChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_H264CBR;
				stVencChnAttr.stRcAttr.stAttrH265Cbr.u32BitRate = u32VenBitRate;
				stVencChnAttr.stRcAttr.stAttrH265Cbr.u32SrcFrmRateNum = format.frameRate;
				stVencChnAttr.stRcAttr.stAttrH264Cbr.u32SrcFrmRateDen = 1;
				stVencChnAttr.stRcAttr.stAttrH264Cbr.u32Gop = 30;
				stVencChnAttr.stRcAttr.stAttrH264Cbr.u32FluctuateLevel = 0;
				stVencChnAttr.stRcAttr.stAttrH264Cbr.u32StatTime = 0;
			}
			else if(V4L2_PIX_FMT_H265 == format.fcc)
			{
				stVencChnAttr.stVeAttr.stAttrH265e.u32PicWidth = format.width;
				stVencChnAttr.stVeAttr.stAttrH265e.u32PicHeight = format.height;
				stVencChnAttr.stVeAttr.stAttrH265e.u32MaxPicWidth = format.width;
				stVencChnAttr.stVeAttr.stAttrH265e.u32MaxPicHeight = ALIGN_UP(format.height, 16);
				stVencChnAttr.stVeAttr.stAttrH265e.bByFrame = bByFrame;
				stVencChnAttr.stVeAttr.eType = E_MI_VENC_MODTYPE_H265E;
				stVencChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_H265CBR;
				stVencChnAttr.stRcAttr.stAttrH265Cbr.u32BitRate = u32VenBitRate;
				stVencChnAttr.stRcAttr.stAttrH265Cbr.u32SrcFrmRateNum = format.frameRate;
				stVencChnAttr.stRcAttr.stAttrH265Cbr.u32SrcFrmRateDen = 1;
				stVencChnAttr.stRcAttr.stAttrH265Cbr.u32Gop = 30;
				stVencChnAttr.stRcAttr.stAttrH265Cbr.u32FluctuateLevel = 0;
				stVencChnAttr.stRcAttr.stAttrH265Cbr.u32StatTime = 0;
			}

			pVenc->createChn(vencDevId, vencChnId, &stVencChnAttr);
			if((V4L2_PIX_FMT_H264 == format.fcc) || (V4L2_PIX_FMT_H265 == format.fcc))
			{
				stVencInputSourceConfig.eInputSrcBufferMode = E_MI_VENC_INPUT_MODE_RING_ONE_FRM;
				pVenc->setInputSourceConf(vencDevId, vencChnId, &stVencInputSourceConfig);
			}

			MI_SYS_BindType_e eBindType = E_MI_SYS_BIND_TYPE_REALTIME;
			MI_U32 bindParam = 0;
			if((V4L2_PIX_FMT_H264 == format.fcc) || (V4L2_PIX_FMT_H265 == format.fcc))
			{
				eBindType = E_MI_SYS_BIND_TYPE_HW_RING;
				bindParam = format.height;
			}

			pVenc->setMaxStreamCnt(vencDevId, vencChnId, 3);
			pSys->bindIsp2Scl(Isp::ispDevId, Scl::sclDevId, 30, 30, E_MI_SYS_BIND_TYPE_REALTIME, 0);
			pSys->bindScl2Venc(sclPortId, vencDevId, vencChnId, 30, 30, eBindType, bindParam);
			pIsp->enablePort(Isp::ispPortId);
			pScl->enablePort(sclPortId);
			pVenc->startRecvPic(vencDevId, vencChnId);
			break;
		}
		default:
		{
			cout << "V4L2 pixel format = what? Format is no recognized."
				<< "Just support: H.264/265, MJPEG, YUYV422." << endl;
			break;
		}
	}

	static bool bFirstRun = true;
	if(bFirstRun)
	{
		cout << "First run app, load iqfile." << endl;
		bFirstRun = false;
		this_thread::sleep_for(chrono::milliseconds(100));
		Isp *pIsp = Isp::getInstance();
		pIsp->loadBinFile((char *)"/config/iqfile/268G_imx415_v3.bin");
		this_thread::sleep_for(chrono::milliseconds(100));

		MI_ISP_AE_FLICKER_TYPE_e eFlickerType = SS_AE_FLICKER_TYPE_50HZ;
		pIsp->setFlicker(&eFlickerType);
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
		case V4L2_PIX_FMT_NV12:
		{
			#if 0
			STCHECKRESULT(MI_SCL_DisableOutputPort(SclDevId, SclChnId, SclPortId));
            if(!g_bEnableHDMI)STCHECKRESULT(MI_ISP_DisableOutputPort(IspDevId, IspChnId, IspPortId));
            if(!g_bEnableHDMI)STCHECKRESULT(ST_Sys_UnBind(&stBindInfo[0]));
            #endif
			break;
		}
		case V4L2_PIX_FMT_MJPEG:
		case V4L2_PIX_FMT_H264:
		case V4L2_PIX_FMT_H265:
		{
			int vencDevId = 0;
			int vencChnId = 0;
			Venc *pVenc = Venc::getInstance();
			vencDevId = (V4L2_PIX_FMT_MJPEG == pstDev->setting.fcc) ? MI_VENC_DEV_ID_JPEG_0 : MI_VENC_DEV_ID_H264_H265_0;
			pVenc->stopRecvPic(vencDevId, vencChnId);

			Scl *pScl = Scl::getInstance();
			int sclPort = Scl::sclPortMain;
			pScl->disablePort(sclPort);

			Isp *pIsp = Isp::getInstance();
			int ispPort = Isp::ispPortId;
			pIsp->disablePort(ispPort);

			Sys *pSys = Sys::getInstance();
			pSys->unbindScl2Venc(sclPort, vencDevId, vencChnId);
			pSys->unbindIsp2Scl(Isp::ispDevId, Scl::sclDevId);

			pVenc->destroyChn(vencDevId, vencChnId);
			break;
		}
		default:
			cout << "V4L2 pixel format = what? Format is no recognized." << endl;
			break;
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
	//uint32_t maxpacket = 64;	// 不知道啥含义，但会影响帧率。1080P30, 4MP15.
	//uint32_t maxpacket = 192;	// 不知道啥含义，但会影响帧率。4KP10.
	uint32_t maxpacket = 1024;	// 不知道啥含义，4K20.
	//uint32_t maxpacket = 2048;	// 不知道啥含义
	MI_U8 mult = 2;				// 不知道啥含义
	MI_U8 burst = 0;			// 突发传输，与USB2.0  不支持突发传输。USB3.0, burst 取8 或13.
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

	if(NULL != stUvcDev.handle)
	{
		ST_UVC_StopDev(&stUvcDev.handle);
		ST_UVC_DestroyDev(stUvcDev.handle);
		ST_UVC_Uninit(stUvcDev.handle);
		memset(&stUvcDev, 0, sizeof(ST_UvcDev_t));
	}

	cout << "Call UacUvc::stopUvc() end." << endl;
	return 0;
}













/*-----------------------------------------------------------------------------
描--述：
参--数：
返回值：
注--意：可以实现为空。
-----------------------------------------------------------------------------*/
static MI_S32 ST_AI_Init(MI_U32 u32AiSampleRate, MI_U8 chn)
{
	cout << "Call ST_AI_Init()." << endl;
	cout << "Call ST_AI_Init() end." << endl;
	return MI_SUCCESS;
}

/*-----------------------------------------------------------------------------
描--述：
参--数：
返回值：
注--意：可以实现为空。
-----------------------------------------------------------------------------*/
static MI_S32 ST_AI_Deinit(void)
{
	cout << "Call ST_AI_Deinit()." << endl;
	cout << "Call ST_AI_Deinit() end." << endl;
	return MI_SUCCESS;
}

/*-----------------------------------------------------------------------------
描--述：填充UAC 音频缓冲区。
参--数：pstUacFrame, 指向音频帧数据。
返回值：成功，返回MI_SUCCESS;
		失败，返回错误码。
注--意：
-----------------------------------------------------------------------------*/
static MI_S32 AI_FillBuffer(ST_UAC_Frame_t *pstUacFrame)
{
	//cout << "Call AI_FillBuffer()." << endl;

	MI_S32 s32Ret = MI_SUCCESS;
	MI_AUDIO_Frame_t stAiChFrame;
	memset(&stAiChFrame, 0, sizeof(MI_AUDIO_Frame_t));

	AudioIn *pAudioIn = AudioIn::getInstance();
	s32Ret = pAudioIn->getFrame(&stAiChFrame);
	if(s32Ret = MI_SUCCESS)
	{
		cerr << "Fail to call pAudioIn->getFrame() in AI_FillBuffer(). "
			<< "errno = 0x" << hex << s32Ret << dec << endl;
		return s32Ret;
	}

	#if 0
	memcpy(pstUacFrame->data, stAiChFrame.apVirAddr[0], stAiChFrame.u32Len[0]);
	pstUacFrame->length = stAiChFrame.u32Len[0];
	#else
	memcpy(pstUacFrame->data, stAiChFrame.apVirAddr[0], stAiChFrame.u32Len[0]);
	memcpy(pstUacFrame->data + stAiChFrame.u32Len[0], stAiChFrame.apVirAddr[1], stAiChFrame.u32Len[1]);
	pstUacFrame->length = stAiChFrame.u32Len[0] + stAiChFrame.u32Len[1];
	#endif

	s32Ret = pAudioIn->releaseFrame(&stAiChFrame);
	if(s32Ret = MI_SUCCESS)
	{
		cerr << "Fail to call pAudioIn->releaseFrame() in AI_FillBuffer(). "
			<< "errno = 0x" << hex << s32Ret << dec << endl;
		return s32Ret;
	}

	//cout << "Call AI_FillBuffer() end." << endl;
	return MI_SUCCESS;
}

#define AI_VOLUME_AMIC_MIN      (0)
#define AI_VOLUME_AMIC_MAX      (21)

static inline MI_S32 Vol_to_Db(ST_UAC_Volume_t stVolume)
{
	return ((stVolume.s32Volume - stVolume.s32Min) * 
		(AI_VOLUME_AMIC_MAX - AI_VOLUME_AMIC_MIN) / 
		(stVolume.s32Max - stVolume.s32Min)) + AI_VOLUME_AMIC_MIN;
}

/*-----------------------------------------------------------------------------
描--述：设置音量大小
参--数：stVolume 音量。
返回值：成功，返回MI_SUCCESS;
		失败，返回错误码。
注--意：
-----------------------------------------------------------------------------*/
static MI_S32 ST_AI_SetVqeVolume(ST_UAC_Volume_t stVolume)
{
	cout << "Call ST_AI_SetVqeVolume()." << endl;
	
	MI_S32 s32Ret = MI_SUCCESS;
	MI_S32 s32AiVolDb = 0;
	s32AiVolDb = Vol_to_Db(stVolume);

	AudioIn *pAudioIn = AudioIn::getInstance();
	s32Ret = pAudioIn->setVolume(s32AiVolDb);
	if(s32Ret = MI_SUCCESS)
	{
		cerr << "Fail to call pAudioIn->setVolume() in ST_AI_SetVqeVolume(). "
			<< "errno = 0x" << hex << s32Ret << dec << endl;
		return s32Ret;
	}

	cout << "Call ST_AI_SetVqeVolume() end." << endl;
	return s32Ret;
}


/*-----------------------------------------------------------------------------
描--述：设置静音
参--数：bMute, 是否为静音。
返回值：成功，返回MI_SUCCESS;
		失败，返回错误码。
注--意：
-----------------------------------------------------------------------------*/
static MI_S32 ST_AI_SetMute(bool bMute)
{
	cout << "Call ST_AI_SetMute()." << endl;
	
	MI_S32 s32Ret = MI_SUCCESS;
	MI_S32 s32AiVolDb = 0;

	AudioIn *pAudioIn = AudioIn::getInstance();
	s32Ret = pAudioIn->setMute(bMute);
	if(s32Ret = MI_SUCCESS)
	{
		cerr << "Fail to call pAudioIn->setMute() in ST_AI_SetMute(). "
			<< "errno = 0x" << hex << s32Ret << dec << endl;
		return s32Ret;
	}

	cout << "Call ST_AI_SetMute() end." << endl;
	return s32Ret;
}

static MI_S32 ST_AO_Init(MI_U32 u32AoSampleRate, MI_U8 chn)
{
	cout << "Call ST_AO_Init()." << endl;
	return MI_SUCCESS;
}

static MI_S32 AO_TakeBuffer(ST_UAC_Frame_t *stUacFrame)
{
	cout << "Call AO_TakeBuffer()." << endl;
	MI_S32 ret = MI_SUCCESS;
	return ret;
}

static MI_S32 ST_AO_Deinit(void)
{
	cout << "Call ST_AO_Deinit()." << endl;
	return MI_SUCCESS;
}

/*-----------------------------------------------------------------------------
描--述：开启UAC
参--数：
返回值：返回0.
注--意：
-----------------------------------------------------------------------------*/
int UacUvc::startUac()
{
	cout << "Call UacUvc::startUac()." << endl;
	MI_S32 trace_level = UVC_DBG_ERR;
	ST_UAC_SetTraceLevel(trace_level);
	
	MI_S32 s32Ret = MI_SUCCESS;
	ST_UAC_OPS_t opsAo = {ST_AO_Init, AO_TakeBuffer, ST_AO_Deinit, NULL, NULL};
	ST_UAC_OPS_t opsAi = {ST_AI_Init, AI_FillBuffer, ST_AI_Deinit, ST_AI_SetVqeVolume, ST_AI_SetMute};

	memset(&stUacHandle, 0, sizeof(ST_UAC_Handle_h));
	s32Ret = ST_UAC_AllocStream(&stUacHandle);
	if(MI_SUCCESS != s32Ret)
	{
		printf("ST_UAC_AllocStream failed!\n");
		return -1;
	}

	MI_S32 eMode = 0;
	//eMode |= AS_OUT_MODE;
	eMode |= AS_IN_MODE;

	ST_UAC_Device_t *pstUacDev;
	pstUacDev = (ST_UAC_Device_t *)stUacHandle;
	pstUacDev->mode = eMode;
	//pstUacDev->opsAo = opsAo;
	pstUacDev->opsAi = opsAi;
	#if 1	// Demo 中的，我认为没必要设置。
	pstUacDev->config[1]->pcm_config.rate = 48000;
	pstUacDev->config[1]->pcm_config.channels = 2;
	pstUacDev->config[0]->pcm_config.rate = 0;
	pstUacDev->config[0]->pcm_config.channels = 2;
	pstUacDev->config[0]->volume.s32Volume = -EINVAL;
	#endif

	s32Ret = ST_UAC_StartDev(stUacHandle);
	if(MI_SUCCESS != s32Ret)
	{
		printf("ST_UAC_StartDev failed!\n");
		return -1;
	}
	cout << "Call UacUvc::startUac() end." << endl;
	return 0;
}

/*-----------------------------------------------------------------------------
描--述：停止UAC
参--数：
返回值：返回0.
注--意：
-----------------------------------------------------------------------------*/
int UacUvc::stopUac()
{
	cout << "Call UacUvc::stopUac()." << endl;

	MI_S32 s32Ret = MI_SUCCESS;
	ST_UAC_StoptDev(stUacHandle);
	s32Ret = ST_UAC_FreeStream(stUacHandle);
	if(MI_SUCCESS != s32Ret)
	{
		printf("ST_UAC_FreeStream failed.\n");
	}

	cout << "Call UacUvc::stopUac() end." << endl;
	return 0;
}

