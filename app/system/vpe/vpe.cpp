/*---------------------------------------------------------------- 
xxx版权所有。
作者：
时间：2020.7.10
----------------------------------------------------------------*/

#include "mi_sys.h"
#include "vpe.h"
#include "sensor.h"
#include <iostream>
#include <string.h>

using namespace std;

Vpe::Vpe()
{
	bEnable = false;

	enable();
}

Vpe::~Vpe()
{
	disable();
	
	bEnable = false;
}

/*-----------------------------------------------------------------------------
描--述：VPE 模块获取实例的唯一入口
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
Vpe* Vpe::getInstance()
{
	static Vpe vpe;
	return &vpe;
}

/*-----------------------------------------------------------------------------
描--述：
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
MI_S32 Vpe::enable()
{
	cout << "Call Vpe::enable()." << endl;

	if(bEnable)
	{
		cerr << "Fail to call Vpe::enable(). Module has been already enabled!" << endl;
		return 0;
	}

	createChannel();
	setChannelParam();
	startChannel();

	bEnable = true;
	cout << "Call Vpe::enable() end." << endl;
	return 0;
}

/*-----------------------------------------------------------------------------
描--述：
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
MI_S32 Vpe::disable()
{
	cout << "Call Vpe::disable()." << endl;

	//disablePort(Vpe::vpeJpegPort);
	disablePort(Vpe::vpeSubPort);
	disablePort(Vpe::vpeMainPort);
	stopChannel();
	destroyChannel();
	bEnable = false;

	cout << "Call Vpe::disable() end." << endl;
	return 0;
}

/*-----------------------------------------------------------------------------
描--述：设置输出端口的深度信息。
参--数：vpePort 端口号；u32UserFrameDepth 用户帧深度；u32BufQueueDepth 队列深度
返回值：成功，返回0; 传递非法参数, 返回-1; 失败，返回错误码。
注--意：如果要dump vpe, 则需要u32UserFrameDepth 至少为1, 如果内存够的话，u32BufQueueDepth 可以设大一点。
-----------------------------------------------------------------------------*/
MI_S32 Vpe::setChnOutputPortDepth(MI_VPE_PORT vpePort, MI_U32 u32UserFrameDepth, MI_U32 u32BufQueueDepth)
{
	cout << "Call Vpe::setChnOutputPortDepth()." << endl;

	MI_SYS_ChnPort_t stChnPort;
	memset(&stChnPort, 0, sizeof(MI_SYS_ChnPort_t));
	stChnPort.eModId = E_MI_MODULE_ID_VPE;
	stChnPort.u32DevId = 0;
	stChnPort.u32ChnId = vpeCh;
	stChnPort.u32PortId = vpePort;

	// MI_S32 MI_SYS_SetChnOutputPortDepth(MI_SYS_ChnPort_t *pstChnPort , MI_U32 u32UserFrameDepth , MI_U32 u32BufQueueDepth);
	MI_S32 s32Ret = 0;
	s32Ret = MI_SYS_SetChnOutputPortDepth(&stChnPort, u32UserFrameDepth, u32BufQueueDepth);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_SYS_SetChnOutputPortDepth(), errno = " << s32Ret << endl;
		return s32Ret;
	}

	cout << "Call Vpe::setChnOutputPortDepth() end." << endl;
	return s32Ret;
}

/*-----------------------------------------------------------------------------
描--述：创建VPE 通道
参--数：无
返回值：成功，返回0; 失败，返回错误码。
注--意：
-----------------------------------------------------------------------------*/
MI_S32 Vpe::createChannel()
{
	cout << "Call Vpe::createChannel()." << endl;

	// step1: 获取sensor 信息。
	MI_S32 s32Ret = 0;
	MI_SNR_PlaneInfo_t stPlaneInfo;
	memset(&stPlaneInfo, 0, sizeof(MI_SNR_PlaneInfo_t ));	
	Sensor *pSensor = Sensor::getInstance();
	s32Ret = pSensor->getPlaneInfo(E_MI_SNR_PAD_ID_0, &stPlaneInfo);
	if(0 != s32Ret)
	{
		cerr << "Fail to call pSensor->getPlaneInfo(), errno = " << s32Ret << endl;
		return s32Ret;
	}

	// step2: 用sensor 相关信息填充VPE 通道属性结构体。
	MI_VPE_ChannelAttr_t stVpeChAttr;
	memset(&stVpeChAttr, 0, sizeof(MI_VPE_ChannelAttr_t));
	stVpeChAttr.u16MaxW = stPlaneInfo.stCapRect.u16Width;
	stVpeChAttr.u16MaxH = stPlaneInfo.stCapRect.u16Height;
	stVpeChAttr.bNrEn = FALSE;
	stVpeChAttr.bEdgeEn = FALSE;
	stVpeChAttr.bEsEn = FALSE;
	stVpeChAttr.bContrastEn = FALSE;
	stVpeChAttr.bUvInvert = FALSE;
	stVpeChAttr.ePixFmt = (MI_SYS_PixelFormat_e)RGB_BAYER_PIXEL(stPlaneInfo.ePixPrecision, stPlaneInfo.eBayerId);
	stVpeChAttr.eRunningMode = E_MI_VPE_RUN_REALTIME_MODE;
	stVpeChAttr.bRotation = FALSE;
	stVpeChAttr.u32ChnPortMode = E_MI_VPE_ZOOM_LDC_NULL;
	stVpeChAttr.bEnLdc = FALSE;
	stVpeChAttr.eHDRType = E_MI_VPE_HDR_TYPE_OFF;
	stVpeChAttr.eSensorBindId = E_MI_VPE_SENSOR0;	// 争议：sensor0 or sensor_invalid
	//stVpeChAttr.eSensorBindId = (MI_VPE_SensorChannel_e)(0x01<<pstVifDevAttr->stBindSensor.eSensorPadID);
	//stVpeChAttr.u32ChnPortMode = pstChannelInfo->u32ChnPortMode;
	
	// step3: 创建VPE 通道。
	s32Ret = MI_VPE_CreateChannel(vpeCh, &stVpeChAttr);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_VPE_CreateChannel(), errno = " << s32Ret << endl;
	}

	cout << "Call Vpe::createChannel() end." << endl;
	return s32Ret;
}

/*-----------------------------------------------------------------------------
描--述：销毁通道。
参--数：无
返回值：成功，返回0; 失败，返回-1.
注--意：
-----------------------------------------------------------------------------*/
MI_S32 Vpe::destroyChannel()
{
	cout << "Call Vpe::destroyChannel()." << endl;

	// MI_S32 MI_VPE_DestroyChannel(MI_VPE_CHANNEL VpeCh);
	MI_S32 s32Ret = 0;
	s32Ret = MI_VPE_DestroyChannel(vpeCh);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_VPE_DestroyChannel(), errno = " << s32Ret << endl;
	}

	cout << "Call Vpe::destroyChannel() end." << endl;
	return s32Ret;
}

/*-----------------------------------------------------------------------------
描--述：开启通道。
参--数：无
返回值：成功，返回0; 失败，返回-1.
注--意：
-----------------------------------------------------------------------------*/
MI_S32 Vpe::startChannel()
{
	cout << "Call Vpe::startChannel()." << endl;

	// MI_S32 MI_VPE_StartChannel(MI_VPE_CHANNEL VpeCh);
	MI_S32 s32Ret = 0;	
	s32Ret = MI_VPE_StartChannel(vpeCh);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_VPE_StartChannel(), errno = " << s32Ret << endl;
	}

	cout << "Call Vpe::startChannel() end." << endl;
	return s32Ret;
}

/*-----------------------------------------------------------------------------
描--述：停止通道。
参--数：无
返回值：成功，返回0; 失败，返回-1.
注--意：
-----------------------------------------------------------------------------*/
MI_S32 Vpe::stopChannel()
{
	cout << "Call Vpe::stopChannel()." << endl;

	// MI_S32 MI_VPE_StopChannel(MI_VPE_CHANNEL VpeCh);
	MI_S32 s32Ret = 0;
	s32Ret = MI_VPE_StopChannel(vpeCh);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_VPE_StopChannel(), errno = " << s32Ret << endl;
	}

	cout << "Call Vpe::stopChannel() end." << endl;
	return s32Ret;
}

/*-----------------------------------------------------------------------------
描--述：使能VPE 端口。
参--数：vpePort 端口号。
返回值：成功，返回0; 失败，返回错误码。
注--意：
-----------------------------------------------------------------------------*/
MI_S32 Vpe::enablePort(MI_VPE_PORT vpePort)
{
	cout << "Call Vpe::enablePort()." << endl;
	
	// MI_S32 MI_VPE_EnablePort(MI_VPE_CHANNEL VpeCh, MI_VPE_PORT VpePort);
	MI_S32 s32Ret = 0;
	s32Ret = MI_VPE_EnablePort(vpeCh, vpePort);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_VPE_EnablePort(), errno = " << s32Ret << endl;
	}

	cout << "Call Vpe::enablePort() end." << endl;
	return s32Ret;
}

/*-----------------------------------------------------------------------------
描--述：禁用VPE 端口。
参--数：vpePort 端口号。
返回值：传递非法参数，返回-1; 成功，返回0; 失败，返回错误码。
注--意：
-----------------------------------------------------------------------------*/
MI_S32 Vpe::disablePort(MI_VPE_PORT vpePort)
{
	cout << "Call Vpe::disablePort()." << endl;

	// MI_S32 MI_VPE_DisablePort(MI_VPE_CHANNEL VpeCh, MI_VPE_PORT VpePort);
	MI_S32 s32Ret = 0;
	s32Ret = MI_VPE_DisablePort(vpeCh, vpePort);
	switch(s32Ret)
	{
		case 0xA0078005:
			cerr << "Fail to call MI_VPE_DisablePort() in Vpe::disablePort(). MI_ERR_VPE_UNEXIST." << endl;
			break;
		case MI_SUCCESS:
			break;
		default:
			cerr << "Fail to call MI_VPE_DisablePort() in Vpe::disablePort(). errno = " << s32Ret << endl;
			break;
	}

	cout << "Call Vpe::disablePort() end." << endl;
	return s32Ret;
}

/*-----------------------------------------------------------------------------
描--述：设置VPE 端口模式。
参--数：vpePort 端口号号；pstVpeMode 指向端口模式的结构体指针, 传入NULL 使用默认参数。。
返回值：传递非法参数返回-1, 成功返回0, 失败返回错误码。
注--意：
-----------------------------------------------------------------------------*/
MI_S32 Vpe::setPortMode(MI_VPE_PORT vpePort, MI_VPE_PortMode_t *pstVpeMode)
{
	cout << "Call Vpe::setPortMode()." << endl;

	MI_S32 s32Ret = 0;
	MI_VPE_PortMode_t stVpeMode;	
	if(NULL == pstVpeMode)	// 如果用户传入空指针，则使用如下默认参数。
	{
		cout << "call Vpe::setPortMode() with null pointer. Use default argument!" << endl;
		pstVpeMode = &stVpeMode;

		// 获取sensor 信息并填充VPE 结构体。
		MI_SNR_PlaneInfo_t stSnrPlaneInfo;
		memset(&stSnrPlaneInfo, 0, sizeof(&stSnrPlaneInfo));
		Sensor *pSensor = Sensor::getInstance();
		s32Ret = pSensor->getPlaneInfo(E_MI_SNR_PAD_ID_0, &stSnrPlaneInfo);
		if(0 != s32Ret)
		{
			cerr << "Fail to call pSensor->getPlaneInfo(), errno = " << s32Ret << endl;
			return s32Ret;
		}
		
		memset(&stVpeMode, 0, sizeof(MI_VPE_PortMode_t));
		stVpeMode.u16Width = stSnrPlaneInfo.stCapRect.u16Width;
		stVpeMode.u16Height = stSnrPlaneInfo.stCapRect.u16Height;
		stVpeMode.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
		stVpeMode.eCompressMode = E_MI_SYS_COMPRESS_MODE_NONE;
		stVpeMode.bMirror = FALSE;
		stVpeMode.bFlip = FALSE;
	}	

	// MI_S32 MI_VPE_SetPortMode(MI_VPE_CHANNEL VpeCh, MI_VPE_PORT VpePort, MI_VPE_PortMode_t *pstVpeMode);
	s32Ret = MI_VPE_SetPortMode(vpeCh, vpePort, pstVpeMode);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_VPE_SetPortMode(), errno = " << s32Ret << endl;
	}

	cout << "Call Vpe::setPortMode() end." << endl;
	return s32Ret;
}

/*-----------------------------------------------------------------------------
描--述：设置通道参数。
参--数：无
返回值：成功，返回0; 失败，返回-1.
注--意：
-----------------------------------------------------------------------------*/
MI_S32 Vpe::setChannelParam()
{
	cout << "Call Vpe::setChannelParam()." << endl;

	// MI_S32 MI_VPE_SetChannelParam(MI_VPE_CHANNEL VpeCh, MI_VPE_ChannelPara_t *pstVpeParam);
	MI_VPE_ChannelPara_t stVpeChParam;
	memset(&stVpeChParam, 0, sizeof(MI_VPE_ChannelPara_t));
	stVpeChParam.bFlip = FALSE;
	stVpeChParam.bMirror = FALSE;
	stVpeChParam.bEnLdc = FALSE;
	stVpeChParam.eHDRType = E_MI_VPE_HDR_TYPE_OFF;
	stVpeChParam.e3DNRLevel = E_MI_VPE_3DNR_LEVEL1;

	MI_S32 s32Ret = 0;
	s32Ret = MI_VPE_SetChannelParam(vpeCh, &stVpeChParam);
	if(0 != s32Ret)
	{
		cerr << "Fail to call MI_VPE_SetChannelParam(), errno = " << s32Ret << endl;
	}

	cout << "Call Vpe::setChannelParam() end." << endl;
	return s32Ret;
}

/*-----------------------------------------------------------------------------
描--述：设置端口模式
参--数：vpePort 端口号
返回值：成功，返回0; 失败，返回错误码。
注--意：子码流端口号为2.
-----------------------------------------------------------------------------*/
MI_S32 Vpe::createPort(MI_VPE_PORT vpePort, unsigned int width, unsigned int height)
{
	cout << "Call Vpe::createPort()." << endl;

	// 填充VPE 结构体
	MI_VPE_PortMode_t stVpeMode;
	memset(&stVpeMode, 0, sizeof(MI_VPE_PortMode_t));
	stVpeMode.u16Width = width;
	stVpeMode.u16Height = height;
	stVpeMode.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
	stVpeMode.eCompressMode = E_MI_SYS_COMPRESS_MODE_NONE;
	stVpeMode.bMirror = FALSE;
	stVpeMode.bFlip = FALSE;

	// 设置端口模式
	MI_S32 s32Ret = 0;
	s32Ret = setPortMode(vpePort, &stVpeMode);
	if(0 != s32Ret)
	{
		cerr << "Fail to call setPortMode(), errno = " << s32Ret << endl;
	}

	s32Ret = setChnOutputPortDepth(vpePort, 0, 4);
	if(0 != s32Ret)
	{
		cerr << "Fail to call setChnOutputPortDepth(), errno = " << s32Ret << endl;
	}

	s32Ret = enablePort(vpePort);
	if(0 != s32Ret)
	{
		cerr << "Fail to call enablePort(), errno = " << s32Ret << endl;
	}
	
	cout << "Call Vpe::createPort() end." << endl;
	return s32Ret;
}
 
