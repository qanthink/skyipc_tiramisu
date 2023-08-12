/*---------------------------------------------------------------- 
xxx版权所有。
作者：
时间：2023.6.14
----------------------------------------------------------------*/

#pragma once

#include "mi_isp.h"

class Isp{
public:
	static Isp* getInstance();

	int enable();
	int disable();

	#if 0	// ispahan
	int getExposureLimit(MI_ISP_AE_EXPO_LIMIT_TYPE_t *pExpoLimit);
	int setExposureLimit(MI_ISP_AE_EXPO_LIMIT_TYPE_t *pExpoLimit);
	#endif

	int setExpoTimeUs(unsigned int expoTimeUs);
	int setExpoAuto();

	#if 0	// ispahan
	int getWDRParam(MI_ISP_IQ_WDR_TYPE_t *pWdrParam);
	int setWDRParam(MI_ISP_IQ_WDR_TYPE_t *pWdrParam);
	#endif

	int enableWDR(int autoOrManual = 0);
	int disableWDR();

	int openIqServer();
	int closeIqServer();
	int setIqServerDataPath(char *iqFilePath);

	int loadBinFile(char* filepath);

public:
	const static MI_ISP_DEV ispDevId = 0;
	const static MI_ISP_CHANNEL ispChnId = 0;

private:
	Isp();
	~Isp();
	Isp(const Isp&);
	Isp& operator=(const Isp&);

private:
	//const unsigned int iSpCh = 0;
};

