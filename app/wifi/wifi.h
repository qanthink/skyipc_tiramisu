/*---------------------------------------------------------------- 
sigma star版权所有。
作者：lison.guo
时间：2020.8.14
----------------------------------------------------------------*/

/*
	基于自组网功能开发的wifi 上层应用，用于获取MAC 地址、wifi 信号强度等信息。
	依赖自组网的头文件和库：wifilink.h libwifilink.a, 技术支持will.zhou
*/

#ifndef __WIFI_H__
#define __WIFI_H__

#include "wifilink.h"

#define __IPC__ 1	// 自组网程序在IPC 和NVR 端设置的参数不同，需要区分

class Wifi{
public:	
	static Wifi *getInstance();
	int enable();
	int disable();
	
	int getApRssi();
	void getPreMac(char *macBuf, unsigned macBufSize);
	int getLocalMac(char *macBuf, unsigned macBufSize, const char *interfaceName);
	
private:
	Wifi();
	~Wifi();
	Wifi(const Wifi&);
	Wifi& operator=(const Wifi&);

	bool bEnable;
};

#endif

