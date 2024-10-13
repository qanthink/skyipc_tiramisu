/*---------------------------------------------------------------- 
xxx版权所有。
作者：
时间：2022.2.13
----------------------------------------------------------------*/

/*
*/

#pragma once

#include <net/if.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>

class Ethernet{
public:
	static Ethernet* getInstance();

	int isInterfaceExist(const char *interface);
	int isInterfaceUpOrDown(const char *interface);
	const char *getInterfaceIP(const char *interface, char *ipBuf, unsigned int ipBufLen);
	int getKeywordValue(const char *inputBuf, unsigned int inputBufLen, char *outputBuf, unsigned int outputBufLen, const char *keyWordBegin, const char *keyWordEnd);
	const char *getWifiSsid(const char *wpaConfPath, char *buf, unsigned int bufLen);
	const char *getWifiPsk(const char *wpaConfPath, char *buf, unsigned int bufLen);

private:
	Ethernet();
	~Ethernet();
	Ethernet(const Ethernet&);
	Ethernet& operator=(const Ethernet&);
};

