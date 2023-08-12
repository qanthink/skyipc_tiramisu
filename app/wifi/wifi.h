/*---------------------------------------------------------------- 
版权所有。
作者：
时间：2020.8.14
----------------------------------------------------------------*/

/*
	wifi 上层应用，用于获取MAC 地址、wifi 信号强度等信息。
*/

#pragma once

class Wifi{
public:
	int getApRssi();
	int getIp(char *ipBuf, unsigned ipBufSize, const char *interfaceName);
	int getMac(char *macBuf, unsigned macBufSize, const char *interfaceName);
	
private:
};

