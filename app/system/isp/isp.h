/*---------------------------------------------------------------- 
xxx版权所有。
作者：
时间：2023.6.14
----------------------------------------------------------------*/

#pragma once

#include "mi_isp.h"
#include "mi_isp_3a_datatype.h"

/*
	ISP 可以创建多个PORT？
*/

class Isp{
public:
	static Isp* getInstance();

private:
	Isp();
	~Isp();
	Isp(const Isp&);
	Isp& operator=(const Isp&);

public:
	const static MI_ISP_DEV ispDevId = 0;
	const static MI_ISP_CHANNEL ispChnId = 0;
	const static MI_ISP_PORT ispPortId = 0;

	int enable();
	int disable();

	int enablePort(MI_ISP_PORT portId);
	int disablePort(MI_ISP_PORT portId);

	int getInputPortCrop(MI_SYS_WindowRect_t *pstCropInfo);
	
	int setOutputPortParam(MI_ISP_OutPortParam_t *pstOutPortParam);

	#if 0	// ispahan
	int getExposureLimit(MI_ISP_AE_EXPO_LIMIT_TYPE_t *pExpoLimit);
	int setExposureLimit(MI_ISP_AE_EXPO_LIMIT_TYPE_t *pExpoLimit);
	#endif

	int setExpoTimeUs(unsigned int expoTimeUs);
	int setExpoAuto();

	int getFlicker(MI_ISP_AE_FLICKER_TYPE_e *peFlickerType);
	int setFlicker(MI_ISP_AE_FLICKER_TYPE_e *peFlickerType);

	#if 0	// ispahan
	int getWDRParam(MI_ISP_IQ_WDR_TYPE_t *pWdrParam);
	int setWDRParam(MI_ISP_IQ_WDR_TYPE_t *pWdrParam);
	#endif

	int enableWDR(int autoOrManual = 0);
	int disableWDR();

	int setAwbCurCTWB(unsigned int u32CT);		// 错误调用。该参数只能GET 不能SET.
	int getAwbAttr(MI_ISP_AWB_ATTR_TYPE_t *pAwbAttr);
	int setAwbAttr(MI_ISP_AWB_ATTR_TYPE_t *pAwbAttr);

	int openIqServer();
	int closeIqServer();
	int setIqServerDataPath(char *iqFilePath);

	int loadBinFile(char* filepath);
};

