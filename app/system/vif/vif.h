/*---------------------------------------------------------------- 
xxx版权所有。
作者：
时间：2020.7.10
----------------------------------------------------------------*/

#pragma once

#include "mi_vif.h"

class Vif{
public:
	static Vif* getInstance();

	const static MI_VIF_DEV vifDevID = 0;
	const static MI_VIF_DEV vifChnID = 0;
	const static MI_VIF_PORT vifPortId = 0;

	MI_S32 enable();
	MI_S32 disable();
	
	MI_S32 enableDev();											// 启用设备
	MI_S32 disableDev();										// 禁用设备
	
	MI_S32 setDevAttr();										// 设置设备属性
	MI_S32 setChnPortAttr(MI_VIF_PORT u32ChnPort);				// 设置通道端口属性

	MI_S32 enableChnPort(MI_VIF_PORT u32ChnPort);				// 启用通道端口
	MI_S32 disableChnPort(MI_VIF_PORT u32ChnPort);				// 禁用通道端口

private:
	Vif();
	~Vif();
	Vif(const Vif&);
	Vif& operator=(const Vif&);

	bool bEnable = false;
};

