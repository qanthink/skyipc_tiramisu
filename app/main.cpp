/*---------------------------------------------------------------- 
xxx 版权所有。
作者：
时间：2020.7.10
----------------------------------------------------------------*/

#include <iostream>
#include <signal.h>
#include <thread>

#include "testing.h"
#include "sys.h"
#include "sensor.h"
#include "vif.h"
#include "isp.h"
//#include "vpe.h"
#include "scl.h"

#if (1 == (USE_DIVP))
#include "divp.h"
#endif

#include "venc.h"
#include "ai.hpp"
#include "ao.hpp"
#include "rgn.h"

#if (1 == (USE_UVC))
#include "uvc_uac.h"
#endif

//#include "aac.h"
//#include "aad.h"
//#include "avtp.h"
#include "ircutled.h"
//#include "spipanel.h"
//#include "ethernet.h"
#include "live555rtsp.h"
//#include "mp4container.h"


using namespace std;

void sigHandler(int sig);

int testSys()
{
	MI_SNR_PADID u8SnrPad = 0;
	STCHECKRESULT(MI_SNR_SetPlaneMode(u8SnrPad, FALSE));

	MI_U32 u32ResCount =0;
	STCHECKRESULT(MI_SNR_QueryResCount(u8SnrPad, &u32ResCount));

	MI_U8 u8ResIndex =0;
	MI_SNR_Res_t stSnrRes;
	for(u8ResIndex = 0; u8ResIndex < u32ResCount; u8ResIndex++)
    {
        STCHECKRESULT(MI_SNR_GetRes(u8SnrPad, u8ResIndex, &stSnrRes));
        printf("index %d, Crop(%d,%d,%d,%d), outputsize(%d,%d), maxfps %d, minfps %d, ResDesc %s\n",
        u8ResIndex,
        stSnrRes.stCropRect.u16X, stSnrRes.stCropRect.u16Y, stSnrRes.stCropRect.u16Width,stSnrRes.stCropRect.u16Height,
        stSnrRes.stOutputSize.u16Width, stSnrRes.stOutputSize.u16Height,
        stSnrRes.u32MaxFps,stSnrRes.u32MinFps,
        stSnrRes.strResDesc);
    }

	MI_S8 s8SnrResIndex = 0;

	STCHECKRESULT(MI_SNR_GetRes(u8SnrPad, s8SnrResIndex, &stSnrRes));

    STCHECKRESULT(MI_SNR_SetRes(u8SnrPad,s8SnrResIndex));
    STCHECKRESULT(MI_SNR_Enable(u8SnrPad));

	// step 2 vif.
	MI_SNR_PADInfo_t  stSnrPadInfo;
	MI_SNR_PlaneInfo_t stSnrPlaneInfo;
	STCHECKRESULT(MI_SNR_GetPadInfo(u8SnrPad, &stSnrPadInfo));
    STCHECKRESULT(MI_SNR_GetPlaneInfo(u8SnrPad, 0, &stSnrPlaneInfo));

    MI_VIF_GROUP VifGroupId = 0;
    MI_VIF_DEV VifDevId = 0;
    
    MI_VIF_GroupAttr_t stVifGroupAttr;
    memset(&stVifGroupAttr, 0x0, sizeof(MI_VIF_GroupAttr_t));
    stVifGroupAttr.eIntfMode = E_MI_VIF_MODE_MIPI;
    stVifGroupAttr.eWorkMode = E_MI_VIF_WORK_MODE_1MULTIPLEX;
    stVifGroupAttr.eHDRType = E_MI_VIF_HDR_TYPE_OFF;
    if(stVifGroupAttr.eIntfMode == E_MI_VIF_MODE_BT656)
        stVifGroupAttr.eClkEdge = (MI_VIF_ClkEdge_e)stSnrPadInfo.unIntfAttr.stBt656Attr.eClkEdge;
    else
        stVifGroupAttr.eClkEdge = E_MI_VIF_CLK_EDGE_DOUBLE;
    STCHECKRESULT(MI_VIF_CreateDevGroup(VifGroupId, &stVifGroupAttr));

	MI_VIF_DevAttr_t stVifDevAttr;
	memset(&stVifDevAttr, 0, sizeof(MI_VIF_DevAttr_t));
    stVifDevAttr.stInputRect.u16X = stSnrPlaneInfo.stCapRect.u16X;
    stVifDevAttr.stInputRect.u16Y = stSnrPlaneInfo.stCapRect.u16Y;
    stVifDevAttr.stInputRect.u16Width = stSnrPlaneInfo.stCapRect.u16Width;
    stVifDevAttr.stInputRect.u16Height = stSnrPlaneInfo.stCapRect.u16Height;
    if(stSnrPlaneInfo.eBayerId >= E_MI_SYS_PIXEL_BAYERID_MAX)
        stVifDevAttr.eInputPixel = stSnrPlaneInfo.ePixel;
    else
        stVifDevAttr.eInputPixel = (MI_SYS_PixelFormat_e)RGB_BAYER_PIXEL(stSnrPlaneInfo.ePixPrecision, stSnrPlaneInfo.eBayerId);
    STCHECKRESULT(MI_VIF_SetDevAttr(VifDevId, &stVifDevAttr));
    STCHECKRESULT(MI_VIF_EnableDev(VifDevId));

	MI_VIF_PORT VifPortId = 0;
	MI_VIF_OutputPortAttr_t stVifPortAttr;
    stVifPortAttr.stCapRect.u16X = stSnrPlaneInfo.stCapRect.u16X;
    stVifPortAttr.stCapRect.u16Y = stSnrPlaneInfo.stCapRect.u16Y;
    stVifPortAttr.stCapRect.u16Width =  stSnrPlaneInfo.stCapRect.u16Width;
    stVifPortAttr.stCapRect.u16Height = stSnrPlaneInfo.stCapRect.u16Height;
    stVifPortAttr.stDestSize.u16Width = stSnrPlaneInfo.stCapRect.u16Width;
    stVifPortAttr.stDestSize.u16Height = stSnrPlaneInfo.stCapRect.u16Height;
    stVifPortAttr.eFrameRate = E_MI_VIF_FRAMERATE_FULL;
    if(stSnrPlaneInfo.eBayerId >= E_MI_SYS_PIXEL_BAYERID_MAX)
        stVifPortAttr.ePixFormat = stSnrPlaneInfo.ePixel;
    else
        stVifPortAttr.ePixFormat = (MI_SYS_PixelFormat_e)RGB_BAYER_PIXEL(stSnrPlaneInfo.ePixPrecision, stSnrPlaneInfo.eBayerId);
    STCHECKRESULT(MI_VIF_SetOutputPortAttr(VifDevId, VifPortId, &stVifPortAttr));

	MI_SYS_ChnPort_t stChnPort;
	MI_VIF_DEV VifChnId = 0;
    stChnPort.eModId = E_MI_MODULE_ID_VIF;
    stChnPort.u32DevId = VifDevId;
    stChnPort.u32ChnId = VifChnId;
    stChnPort.u32PortId = VifPortId;
    STCHECKRESULT(MI_SYS_SetChnOutputPortDepth(0, &stChnPort, 0, 3+3));
    STCHECKRESULT(MI_VIF_EnableOutputPort(VifDevId, VifPortId));

	int ISP_DEV_ID = 0;
	int ISP_CHN_ID = 0;
    MI_ISP_DEV IspDevId = ISP_DEV_ID;
    MI_ISP_CHANNEL IspChnId = ISP_CHN_ID;

	// step3 isp
    MI_ISP_DevAttr_t stIspDevAttr;
    MI_ISP_ChannelAttr_t  stIspChnAttr;
    MI_ISP_ChnParam_t stIspChnParam;
    memset(&stIspDevAttr, 0x0, sizeof(MI_ISP_DevAttr_t));
    memset(&stIspChnAttr, 0x0, sizeof(MI_ISP_ChannelAttr_t));
    memset(&stIspChnParam, 0x0, sizeof(MI_ISP_ChnParam_t));

    stIspDevAttr.u32DevStitchMask = E_MI_ISP_DEVICEMASK_ID0;
    STCHECKRESULT(MI_ISP_CreateDevice(IspDevId, &stIspDevAttr));

    switch(u8SnrPad)
    {
        case 0:
            stIspChnAttr.u32SensorBindId = E_MI_ISP_SENSOR0;
            break;
        case 1:
            stIspChnAttr.u32SensorBindId = E_MI_ISP_SENSOR1;
            break;
        case 2:
            stIspChnAttr.u32SensorBindId = E_MI_ISP_SENSOR2;
            break;
        case 3:
            stIspChnAttr.u32SensorBindId = E_MI_ISP_SENSOR3;
            break;
        default:
            ST_ERR("Invalid Snr pad id:%d\n", (int)u8SnrPad);
            return -1;
    }

    stIspChnParam.eHDRType = E_MI_ISP_HDR_TYPE_OFF;
    stIspChnParam.e3DNRLevel = E_MI_ISP_3DNR_LEVEL1;
    stIspChnParam.bMirror = FALSE;
    stIspChnParam.bFlip = FALSE;
    stIspChnParam.eRot = E_MI_SYS_ROTATE_NONE;
    STCHECKRESULT(MI_ISP_CreateChannel(IspDevId, IspChnId, &stIspChnAttr));
    STCHECKRESULT(MI_ISP_SetChnParam(IspDevId, IspChnId, &stIspChnParam));
    STCHECKRESULT(MI_ISP_StartChannel(IspDevId, IspChnId));

	// bind
    ST_Sys_BindInfo_T stBindInfo;
    memset(&stBindInfo, 0x0, sizeof(ST_Sys_BindInfo_T));

    stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_VIF;
    stBindInfo.stSrcChnPort.u32DevId = VifDevId;
    stBindInfo.stSrcChnPort.u32ChnId = VifChnId;
    stBindInfo.stSrcChnPort.u32PortId = VifPortId;
    stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_ISP;
    stBindInfo.stDstChnPort.u32DevId = IspDevId;
    stBindInfo.stDstChnPort.u32ChnId = IspChnId;
    stBindInfo.u32SrcFrmrate = 30;
    stBindInfo.u32DstFrmrate = 30;
    //stBindInfo.eBindType = E_MI_SYS_BIND_TYPE_REALTIME;
    stBindInfo.eBindType = E_MI_SYS_BIND_TYPE_FRAME_BASE;
    STCHECKRESULT(ST_Sys_Bind(&stBindInfo));

    #if 0	// pri pool
    MI_SYS_GlobalPrivPoolConfig_t stGlobalPrivPoolConf;
	memset(&stGlobalPrivPoolConf, 0x0, sizeof(stGlobalPrivPoolConf));
	stGlobalPrivPoolConf.bCreate = TRUE;
	stGlobalPrivPoolConf.eConfigType = E_MI_SYS_PER_CHN_PORT_OUTPUT_POOL;
	stGlobalPrivPoolConf.uConfig.stPreChnPortOutputPrivPool.eModule = E_MI_MODULE_ID_VIF;
	stGlobalPrivPoolConf.uConfig.stPreChnPortOutputPrivPool.u32Devid = VifDevId;
	stGlobalPrivPoolConf.uConfig.stPreChnPortOutputPrivPool.u32Channel = VifChnId;
	stGlobalPrivPoolConf.uConfig.stPreChnPortOutputPrivPool.u32Port = VifPortId;
	stGlobalPrivPoolConf.uConfig.stPreChnPortOutputPrivPool.u32PrivateHeapSize = 0xfb6000*3;
	STCHECKRESULT(MI_SYS_ConfigPrivateMMAPool(0,&stGlobalPrivPoolConf));
    #endif

	// scl
	int SCL_DEV_ID_RT = 0;
	int SCL_PORT_ID = 0;
    MI_SCL_DEV SclDevId = SCL_DEV_ID_RT;

    MI_SCL_DevAttr_t stSclDevAttr;
    MI_SCL_ChannelAttr_t  stSclChnAttr;
    MI_SCL_ChnParam_t  stSclChnParam;
    memset(&stSclDevAttr, 0x0, sizeof(MI_SCL_DevAttr_t));
    memset(&stSclChnAttr, 0x0, sizeof(MI_SCL_ChannelAttr_t));
    memset(&stSclChnParam, 0x0, sizeof(MI_SCL_ChnParam_t));

    switch(SCL_PORT_ID)
    {
        case 0:
            stSclDevAttr.u32NeedUseHWOutPortMask = E_MI_SCL_HWSCL2; //Port0->HWSCL2
            break;
        case 1:
            stSclDevAttr.u32NeedUseHWOutPortMask = E_MI_SCL_HWSCL3; //Port1->HWSCL3
            break;
        case 2:
            stSclDevAttr.u32NeedUseHWOutPortMask = E_MI_SCL_HWSCL4; //Port2->HWSCL4
            break;
        default:
            ST_ERR("Invalid Scl Port Id:%d\n", SCL_PORT_ID);
            return -1;
    }
    STCHECKRESULT(MI_SCL_CreateDevice(SclDevId, &stSclDevAttr));

    for(int i = 0; i < 1; i++)
    {
        STCHECKRESULT(MI_SCL_CreateChannel(SclDevId, i, &stSclChnAttr));
        STCHECKRESULT(MI_SCL_SetChnParam(SclDevId, i, &stSclChnParam));
        STCHECKRESULT(MI_SCL_StartChannel(SclDevId, i));
    }

    STCHECKRESULT(MI_ISP_EnableOutputPort(IspDevId, IspChnId, 0));
    STCHECKRESULT(MI_SCL_EnableOutputPort(SclDevId, 0, 0));

	return 0;
}

int testV()
{
	int u32IspDev = 0;
	int u32IspChn = 0;
	int u32IspPort = 0;
	
	MI_ISP_DEV IspDevId = (MI_ISP_DEV)u32IspDev;
    MI_ISP_CHANNEL IspChnId = (MI_ISP_CHANNEL)u32IspChn;
    MI_ISP_PORT  IspPortId = (MI_ISP_PORT)u32IspPort;

    MI_ISP_OutPortParam_t  stIspOutputParam;
    memset(&stIspOutputParam, 0x0, sizeof(MI_ISP_OutPortParam_t));
    STCHECKRESULT(MI_ISP_GetInputPortCrop(IspDevId, IspChnId, &stIspOutputParam.stCropRect));

	int u32SclDev = 0;
	int u32SclChn = 0;
	int u32SclPort = 0;
    MI_SCL_DEV SclDevId = (MI_SCL_DEV)u32SclDev;
    MI_SCL_CHANNEL SclChnId = (MI_SCL_CHANNEL)u32SclChn;
    MI_SCL_PORT SclPortId = (MI_SCL_PORT)u32SclPort;

    MI_SCL_OutPortParam_t  stSclOutputParam;
    memset(&stSclOutputParam, 0x0, sizeof(MI_SCL_OutPortParam_t));
    stSclOutputParam.stSCLOutCropRect.u16X = stIspOutputParam.stCropRect.u16X;
    stSclOutputParam.stSCLOutCropRect.u16Y = stIspOutputParam.stCropRect.u16Y;
    stSclOutputParam.stSCLOutCropRect.u16Width = stIspOutputParam.stCropRect.u16Width;
    stSclOutputParam.stSCLOutCropRect.u16Height = stIspOutputParam.stCropRect.u16Height;
    stSclOutputParam.stSCLOutputSize.u16Width = 3840;
    stSclOutputParam.stSCLOutputSize.u16Height = 2160;
    stSclOutputParam.bMirror = FALSE;
    stSclOutputParam.bFlip = FALSE;
    stSclOutputParam.eCompressMode= E_MI_SYS_COMPRESS_MODE_NONE;

    MI_VENC_DEV VencDevId;
    int u32VencChn = 0;
    MI_VENC_CHN VencChnId = (MI_VENC_CHN)u32VencChn;

    MI_VENC_ChnAttr_t stVencChnAttr;
    MI_VENC_InputSourceConfig_t stVencInputSourceConfig;
    memset(&stVencChnAttr, 0, sizeof(MI_VENC_ChnAttr_t));
    memset(&stVencInputSourceConfig, 0, sizeof(MI_VENC_InputSourceConfig_t));

    MI_U32 u32VenBitRate;
    MI_U32 u32VenQfactor;
    bool bByFrame = TRUE;

    /************************************************
    Step4:  Init Bind Param
    *************************************************/
    ST_Sys_BindInfo_T stBindInfo[2];
    MI_SYS_ChnPort_t stChnPort[2];

    memset(&stBindInfo[0], 0x0, sizeof(ST_Sys_BindInfo_T));
    stBindInfo[0].stSrcChnPort.eModId = E_MI_MODULE_ID_ISP;
    stBindInfo[0].stSrcChnPort.u32DevId = IspDevId;
    stBindInfo[0].stSrcChnPort.u32ChnId = IspChnId;
    stBindInfo[0].stSrcChnPort.u32PortId = IspPortId;
    stBindInfo[0].stDstChnPort.eModId = E_MI_MODULE_ID_SCL;
    stBindInfo[0].stDstChnPort.u32DevId = SclDevId;
    stBindInfo[0].stDstChnPort.u32ChnId = SclChnId;
    stBindInfo[0].u32SrcFrmrate = 30;
    stBindInfo[0].u32DstFrmrate = 30;
    //stBindInfo[0].eBindType = E_MI_SYS_BIND_TYPE_REALTIME : E_MI_SYS_BIND_TYPE_FRAME_BASE;
    stBindInfo[0].eBindType = E_MI_SYS_BIND_TYPE_REALTIME;

    memset(&stBindInfo[1], 0x0, sizeof(ST_Sys_BindInfo_T));
    stBindInfo[1].stSrcChnPort.eModId = E_MI_MODULE_ID_SCL;
    stBindInfo[1].stSrcChnPort.u32DevId = SclDevId;
    stBindInfo[1].stSrcChnPort.u32ChnId = SclChnId;
    stBindInfo[1].stSrcChnPort.u32PortId = SclPortId;
    stBindInfo[1].stDstChnPort.eModId = E_MI_MODULE_ID_VENC;
    stBindInfo[1].stDstChnPort.u32ChnId = VencChnId;
    stBindInfo[1].u32SrcFrmrate = 30;
    stBindInfo[1].u32DstFrmrate = 30;
    //if(fcc == V4L2_PIX_FMT_MJPEG)
    //    stBindInfo[1].eBindType = E_MI_SYS_BIND_TYPE_REALTIME;
    //else if(fcc == V4L2_PIX_FMT_H264 || fcc == V4L2_PIX_FMT_H265)
        stBindInfo[1].eBindType = E_MI_SYS_BIND_TYPE_HW_RING;
    //stBindInfo[1].eBindType = E_MI_SYS_BIND_TYPE_FRAME_BASE;

	memset(&stChnPort[0], 0x0, sizeof(MI_SYS_ChnPort_t));
    stChnPort[0].eModId = E_MI_MODULE_ID_ISP;
    stChnPort[0].u32DevId = IspDevId;
    stChnPort[0].u32ChnId = IspChnId;
    stChnPort[0].u32PortId = IspPortId;

    memset(&stChnPort[1], 0x0, sizeof(MI_SYS_ChnPort_t));
    stChnPort[1].eModId = E_MI_MODULE_ID_SCL;
    stChnPort[1].u32DevId = SclDevId;
    stChnPort[1].u32ChnId = SclChnId;
    stChnPort[1].u32PortId = SclPortId;

	/************************************************
    Step5:  Init User Param
    *************************************************/
    u32VenBitRate = 2 * 1024 * 1024;
    u32VenQfactor = 80;

    /************************************************
    Step5:  Start
    *************************************************/
    stIspOutputParam.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
	stSclOutputParam.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;

	STCHECKRESULT(MI_SCL_SetOutputPortParam(SclDevId, SclChnId, SclPortId, &stSclOutputParam));
	STCHECKRESULT(MI_ISP_SetOutputPortParam(IspDevId, IspChnId, IspPortId, &stIspOutputParam));

	VencDevId = MI_VENC_DEV_ID_H264_H265_0;
	stBindInfo[1].stDstChnPort.u32DevId = VencDevId;

	int u32Width = 3840;
	int u32Height = 2160;
	stVencChnAttr.stVeAttr.stAttrH264e.u32PicWidth = u32Width;
    stVencChnAttr.stVeAttr.stAttrH264e.u32PicHeight = u32Height;
    stVencChnAttr.stVeAttr.stAttrH264e.u32MaxPicWidth = u32Width;
    stVencChnAttr.stVeAttr.stAttrH264e.u32MaxPicHeight = u32Height;
    stVencChnAttr.stVeAttr.stAttrH264e.bByFrame = bByFrame;
    stVencChnAttr.stVeAttr.stAttrH264e.u32BFrameNum = 2;
    stVencChnAttr.stVeAttr.stAttrH264e.u32Profile = 1;
    stVencChnAttr.stVeAttr.eType = E_MI_VENC_MODTYPE_H264E;
    stVencChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_H264CBR;
    stVencChnAttr.stRcAttr.stAttrH265Cbr.u32BitRate = u32VenBitRate;
    stVencChnAttr.stRcAttr.stAttrH265Cbr.u32SrcFrmRateNum = 30;
    stVencChnAttr.stRcAttr.stAttrH264Cbr.u32SrcFrmRateDen = 1;
    stVencChnAttr.stRcAttr.stAttrH264Cbr.u32Gop = 30;
    stVencChnAttr.stRcAttr.stAttrH264Cbr.u32FluctuateLevel = 0;
    stVencChnAttr.stRcAttr.stAttrH264Cbr.u32StatTime = 0;

    STCHECKRESULT(MI_VENC_CreateChn(VencDevId, VencChnId, &stVencChnAttr));


    stVencInputSourceConfig.eInputSrcBufferMode = E_MI_VENC_INPUT_MODE_RING_ONE_FRM;
	stBindInfo[1].u32BindParam = u32Height;
	STCHECKRESULT(MI_VENC_SetInputSourceConfig(VencDevId, VencChnId, &stVencInputSourceConfig));

	STCHECKRESULT(MI_VENC_SetMaxStreamCnt(VencDevId, VencChnId, 3 + 1));
    STCHECKRESULT(MI_VENC_StartRecvPic(VencDevId, VencChnId));
    STCHECKRESULT(ST_Sys_Bind(&stBindInfo[0]));
    STCHECKRESULT(ST_Sys_Bind(&stBindInfo[1]));
	//sleep(5);
    //STCHECKRESULT(MI_ISP_EnableOutputPort(IspDevId, IspChnId, IspPortId));
    //STCHECKRESULT(MI_SCL_EnableOutputPort(SclDevId, SclChnId, SclPortId));
    //sleep(5);
	
    
	return 0;
}

int main(int argc, const char *argv[])
{
	/*
		统一采用C++单例设计模式，getInstance() 为各模块实例的统一入口函数。
		单例模式不存在重复初始化的问题，在调用之初执行构造，后续其它地方的调用时间开销小。
		风格尽量趋近于C, 避免C++11 及之后的高级用法。
		库函数尽可能使用linux 标准库函数，高效，可调试性高。暂不考虑linux->otherOS 的移植问题。
	*/

	signal(SIGINT, sigHandler);

	#if 0
	testSys();
	testV();
	#endif
	
	//sleep(66);
	//return 0;

	#if 1

	/* ==================== 第一部分，系统初始化 ==================== */
	IrCutLed *pIrCutLed = IrCutLed::getInstance();
	IrCutLed::getInstance()->openFilter();

	// 系统初始化
	Sys *pSys = Sys::getInstance();

	// Sensor 初始化。数据流向：sensor -> vif -> vpe -> (DIVP) -> venc -> 应用处理。
	Sensor *pSensor = Sensor::getInstance();	// sensor 初始化
	pSensor->setFps(30);

	unsigned int snrW = 0;
	unsigned int snrH = 0;
	pSensor->getSnrWH(&snrW, &snrH);
	cout << "snrW, snrH = " << snrW << ", " << snrH << endl;

	// VIF 初始化
	Vif *pVif = Vif::getInstance();

	// ISP 初始化，并绑定前级VIF.
	Isp *pIsp = Isp::getInstance();
	pSys->bindVif2Isp(Vif::vifDevID, Isp::ispDevId, 30, 30, E_MI_SYS_BIND_TYPE_FRAME_BASE, 0);

	// SCL 初始化，并绑定前级ISP.
	unsigned int subW = 1280;
	unsigned int subH = 720;
	Scl *pScl = Scl::getInstance();
	pScl->createPort(Scl::sclPortMain, snrW, snrH);
	pScl->createPort(Scl::sclPortSub, subW, subH);
	pSys->bindIsp2Scl(Isp::ispDevId, Scl::sclDevId, 30, 30, E_MI_SYS_BIND_TYPE_REALTIME, 0);
	sleep(2);

	// 创建DIVP 缩放、剪裁通道
	#if (1 == (USE_DIVP))
	Divp *pDivp = Divp::getInstance();
	pDivp->createChn(snrW, snrH, snrW, snrH);
	#endif

	Venc *pVenc = Venc::getInstance();
	#if (1 == (USE_IPC))

	sleep(2);
	// 创建主码流
	#if (1 == (USE_VENC_MAIN))
	pVenc->createH264Stream(Venc::vencMainChn, snrW, snrH);
	pVenc->changeBitrate(MI_VENC_DEV_ID_H264_H265_0, Venc::vencMainChn, 1 * 1024);
	sleep(2);
	//pSys->bindScl2Venc(Scl::sclPortId + 1, Venc::vencMainChn, 30, 30, E_MI_SYS_BIND_TYPE_HW_RING, snrH);
	pSys->bindScl2Venc(Scl::sclPortMain, Venc::vencMainChn, 30, 30, E_MI_SYS_BIND_TYPE_HW_RING, snrH);
	#if (1 == (USE_DIVP))
	pSys->bindVpe2Divp(Vpe::vpeMainPort, Divp::divpChn, 30, 30, E_MI_SYS_BIND_TYPE_FRAME_BASE, 0);
	pSys->bindDivp2Venc(Divp::divpChn, Venc::vencMainChn, 30, 30, E_MI_SYS_BIND_TYPE_FRAME_BASE, 0);
	#else
	//pSys->bindVpe2Venc(Vpe::vpeMainPort, Venc::vencMainChn, 30, 30, E_MI_SYS_BIND_TYPE_FRAME_BASE, 0);
	#endif	// End of USE_DIVP
	#endif	// End of USE_VENC_MAIN

	// 创建子码流
	#if (1 == (USE_VENC_SUB))
	pVenc->createH264Stream(Venc::vencSubChn, subW, subH);
	pVenc->changeBitrate(MI_VENC_DEV_ID_H264_H265_0, Venc::vencSubChn, 0.25 * 1024);
	pSys->bindScl2Venc(Scl::sclPortSub, Venc::vencSubChn, 30, 30, E_MI_SYS_BIND_TYPE_FRAME_BASE, subH);
	#endif

	// 创建jpeg码流
	#if (1 == (USE_VENC_JPEG))
	//pVpe->createPort(Vpe::vpeMainPort, snrW, snrH);
	pVenc->createJpegStream(Venc::vencJpegChn, snrW, snrH);
	pVenc->changeBitrate(MI_VENC_DEV_ID_JPEG_0, Venc::vencJpegChn, 10);
	pSys->bindVpe2Venc(Vpe::vpeMainPort, Venc::vencJpegChn, 30, 30, E_MI_SYS_BIND_TYPE_FRAME_BASE, 0);
	#endif

	// VENC 也可以实现图像的Crop 和Scale, 但是建议在VPE 中做。
	//pVenc->setCrop(Venc::vencMainChn, (2560 - 1920) / 2, (1440 - 1080) / 2, 1920, 1080);
	#endif	// End of USE_UVC

	#if(1 == (USE_IQ_SERVER))
	//Isp *pIsp = Isp::getInstance();
	pIsp->openIqServer();
	char iqFilePath[128] = "/config/iqfile/335_imx291_day.bin";
	pIsp->loadBinFile(iqFilePath);
	#endif

	// 初始化OSD
	#if (1 == (USE_OSD))
	Rgn *pRgn = Rgn::getInstance();
	#endif
	
	// 音频编/解码初始化。aac = audio coder; aad = audio decoder.
	#if (1 == (USE_FAAC_FAAD))
	Aac *pAac = Aac::getInstance();
	Aad *pAad = Aad::getInstance();
	#endif

	// AI 和AO 属于两个模块，分别初始化。
	#if (1 == (USE_AI))
	AudioIn *pAudioIn = AudioIn::getInstance();
	#endif
	
	#if (1 == (USE_AO))
	AudioOut *pAudioOut = AudioOut::getInstance();
	#endif

	/*
		至此，SENSOR, VIF, VPE, VENC, AI, AO, OSD 均已初始化完成。
	*/
	#endif

	/* ==================== 第二部分，应用初始化 ==================== */
	// 音视频传输协议测试
	#if (1 == (USE_AVTP_AUDIO))
	AvtpAudioClient avtpAudioClient("192.168.0.200");
	pAvtpAudioClient = &avtpAudioClient;
	#endif

	#if (1 == (USE_AVTP_VIDEO))
	AvtpVideoClient avtpVideoClient("192.168.0.200");
	pAvtpVideoClient = &avtpVideoClient;
	thread thChangeBitrate(avtpChangeKbps, pAvtpVideoClient, 3);
	#endif

	// RTSP 推网络流
	#if ((1 == (USE_RTSPSERVER_LOCALFILE)) || (1 == (USE_RTSPSERVER_LIVESTREAM_MAIN)) \
			|| (1 == (USE_RTSPSERVER_LIVESTREAM_SUB)) || 1 == (USE_RTSPSERVER_LIVESTREAM_JPEG))
	Live555Rtsp live555Rtsp;
	pLive555Rtsp = &live555Rtsp;
	#endif
	
	// RTSP 推本地流
	#if (1 == (USE_RTSPSERVER_LOCALFILE))
	const char *filePath = NULL;
	//filePath = "/mnt/linux/Downloads/videotest/1.mp4";
	filePath = "/mnt/linux/Downloads/material/test.264";
	const char *streamName = "stream";
	pLive555Rtsp->addStream(filePath, streamName, emEncTypeH264);
	#endif

	// FFMPEG 保存MP4.
	#if (1 == (USE_FFMPEG_SAVE_MP4))
	Mp4Container *pMp4Container = Mp4Container::getInstance();
	#endif

	// 获取音频AI 的线程
	#if (1 == (USE_AI))
	thread thAi(routeAi, (void *)NULL);
	#endif

	// AO 音频测试
	#if (1 == (USE_AO))
	// AO. 参数为本地音频文件的路径。写死的，16位宽 16000采样率
	#if (1 == (USE_AO_LOCAL_FILE))
	thread thAo(routeAoFromFile, (void *)"/mnt/linux/Audios/pcm/xiaopingguo_mono_16b_16000.pcm");
	#endif

	// AO 音频测试，播放网络音频
	#if (1 == (USE_AO_NET_PCM))
	thread thAo(routeAoNetPcm, (void *)NULL);
	#endif
	#endif

	// OSD 功能
	#if(1 == (USE_OSD))
	thread thOsd(routeOsd, (void *)NULL);
	#endif

	// 测试主码流
	#if (1 == (USE_VENC_MAIN))
	cout << "routeVideoMain" << endl;
	thread thVideoMain(routeVideo, (void *)Venc::vencMainChn);
	#endif

	// 测试子码流
	#if (1 == (USE_VENC_SUB))
	cout << "routeVideoSub" << endl;
	thread thVideoSub(routeVideo, (void *)Venc::vencSubChn);
	#endif

	// 测试JPEG 码流
	#if (1 == (USE_VENC_JPEG))
	cout << "routeVideoJpeg" << endl;
	thread thVideoJpeg(routeVideo, (void *)Venc::vencJpegChn);
	#endif

	int ret = 0;
	this_thread::sleep_for(chrono::microseconds(1));	// sleep for rtsp file read.
	#if ((1 == (USE_RTSPSERVER_LOCALFILE)) || (1 == (USE_RTSPSERVER_LIVESTREAM_MAIN)) \
		|| (1 == (USE_RTSPSERVER_LIVESTREAM_SUB)) || (1 == (USE_RTSPSERVER_LIVESTREAM_JPEG)))
	thread thRtsp([&](){pLive555Rtsp->eventLoop();});		// lambda 表达式太好用啦！
	thRtsp.detach();
	#endif

	// 测试SPI 屏。
	#if (1 == (USE_SPI_PANEL))
	thread thSpiPanel(routeSpiPanel, (void *)NULL);
	#endif

	// 测试网络
	#if (1 == (TEST_ETHERNET))
	testEthernet();
	Ethernet *pEthernet = pEthernet->getInstance();
	pEthernet->showWlanInfOnPanel();
	#endif

	// 测试UVC
	#if (1 == (USE_UVC))
	UvcUac *pUvcUac = UvcUac::getInstance();
	pUvcUac->startUvc();
	#endif

	g_bRunning = true;		// sigHandler() 对其取反。
	while(g_bRunning)
	{
		//int ret = 0;
		#if (1 == USE_INTERACTION)
		ret = interAction();
		if(1 == ret)
		{
			break;
		}
		#else
		sleep(1);
		static int sleepCntSec = 0;
		if(15 == sleepCntSec++)
		{
			cout << "Progress running." << endl;
			sleepCntSec = 0;
		}
		#endif
	}
	cout << "jump out from while(g_bRunning)" << endl;

	/* ==================== 第三部分，应用析构 ==================== */
	#if ((1 == (USE_RTSPSERVER_LOCALFILE)) || (1 == (USE_RTSPSERVER_LIVESTREAM_MAIN)) \
		|| (1 == (USE_RTSPSERVER_LIVESTREAM_SUB)) || (1 == (USE_RTSPSERVER_LIVESTREAM_JPEG)))
	//pLive555Rtsp->~Live555Rtsp();
	cout << "rtsp join()" << endl;
	//thRtsp.join();	// 前期做了detach.
	cout << "rtsp join." << endl;
	#endif
	
	#if (1 == (USE_AI))
	thAi.join();
	cout << "ai join." << endl;
	#endif
	
	#if (1 == (USE_AO))
	thAo.join();
	cout << "ao join." << endl;
	#endif
									
	#if (1 == (USE_VENC_JPEG))
	thVideoJpeg.join();
	cout << "vencJpeg join." << endl;
	#endif
	
	#if (1 == (USE_VENC_SUB))
	thVideoSub.join();
	cout << "vencSub join." << endl;
	#endif

	#if (1 == (USE_VENC_MAIN))
	thVideoMain.join();
	cout << "vencMain join." << endl;
	#endif

	#if (1 == (USE_OSD))
	thOsd.join();
	cout << "osd join." << endl;
	#endif

	#if (1 == (USE_AVTP_VIDEO))
	thChangeBitrate.join();
	cout << "avtp video join." << endl;
	#endif

	#if (1 == (USE_SPI_PANEL))
	thSpiPanel.join();
	cout << "spi panel join." << endl;
	#endif

	#if (1 == (USE_UVC))
	pUvcUac->stopUvc();
	#endif

	#if(1 == (USE_IQ_SERVER))
	//pIsp->closeIqServer();
	#endif

	cout << "Sleep()" << endl;
	sleep(0.5);
	
	return 0;
}

/*-----------------------------------------------------------------------------
描--述：信号处理函数。
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
void sigHandler(int sig)
{
	cout << "Call sigHandler()." << endl;
	
	switch (sig)
	{
	case SIGINT:
		cerr << "Receive SIGINT!!!" << endl;
		g_bRunning = false;
		break;
	default:
		g_bRunning = false;
		break;
	}

	cout << "Call sigHandler() end." << endl;
	return;
}

