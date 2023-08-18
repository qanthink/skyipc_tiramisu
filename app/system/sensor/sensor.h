/*---------------------------------------------------------------- 
xxx版权所有。
作者：
时间：2020.7.10
----------------------------------------------------------------*/

#pragma once

#include "mi_sensor.h"

class Sensor{
public:
	static Sensor *getInstance();
	
private:
	Sensor();
	~Sensor();
	Sensor(const Sensor&);
	Sensor& operator=(const Sensor&);

public:
	const static MI_SNR_PADID ePADId = 0;

	int enable();									// 启用sensor
	int disable();									// 禁用sensor

	int getFps(MI_U32 *u32Fps);						// 获取帧率
	int setFps(MI_U32 u32Fps);						// 设置帧率

	int getPadInfo(MI_SNR_PADID ePADId, MI_SNR_PADInfo_t *pstPadInfo);				// 获取sensor 设备信息
	int getPlaneInfo(MI_SNR_PADID ePADId, MI_SNR_PlaneInfo_t *pstPlaneInfo);		// 获取sensor 通道信息

	int getSnrWH(unsigned int *pSnrW, unsigned int *pSnrH);							// 获取sensor 宽高

private:
	bool bEnable = false;

	/*	与sensor 的配置相关，同一颗sensor 可能出不同配置。
		例如IMX415 可以出：3840x2160@30fps; 2560x1440@30fps; 1920x1080@60fps; */
	const static MI_U8 u8SnrResIndex = 0;
	
	const static MI_U32 u32DefFps = 30;
	const static MI_U32 u32PlaneID = 0;
	const static MI_BOOL bEnableHDR = false;
};

