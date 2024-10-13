/*---------------------------------------------------------------- 
xxx版权所有。
作者：
时间：2022.2.13
----------------------------------------------------------------*/

/*
	2024.10.13修改，移除屏显相关内容，与网络模块无关；
*/

#include "ethernet.h"

#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>
#include <string.h>

using namespace std;

Ethernet* Ethernet::getInstance()
{
	static Ethernet ethernet;
	return &ethernet;
}

Ethernet::Ethernet()
{
}

Ethernet::~Ethernet()
{
}

/*-----------------------------------------------------------------------------
描--述：判断网口是否存在；
参--数：网口名称，例如eth0, ens33, wlan0.
返回值：0, 网口不存在；非0, 网口存在；
注--意：
-----------------------------------------------------------------------------*/
int Ethernet::isInterfaceExist(const char *interface)
{
	if(NULL == interface)
	{
		cerr << "Fail to call Ethernet::getInterfaceIP(). Argument has null value." << endl;
		return 0;
	}

	int ret = 0;
	struct ifaddrs *ifc = NULL;
	ret = getifaddrs(&ifc);
	if(ret < 0)
	{
		cerr << "Fail to call getifaddrs(3)." << strerror(errno) << endl;
		return 0;
	}
	
	struct ifaddrs *ifc_header = NULL;
	ifc_header = ifc;
	ret = 0;

	for(; NULL != ifc; ifc = ifc->ifa_next)
	{
		if(0 == strcmp(ifc->ifa_name, interface))
		{
			if(AF_INET == ifc->ifa_addr->sa_family)
			{
				//cout << "Find interface and family is AF_INET." << endl;
				//inet_ntop(AF_INET, &((struct sockaddr_in *)ifc->ifa_addr)->sin_addr, ipBuf, ipBufLen);
				//cout << ifc->ifa_name << endl;
				//cout << ip << endl;
				ret = 1;
				break;
			}
			else
			{
				//cerr << "Not AF_INET." << endl;
				continue;
			}
		}
		else
		{
			//cerr << "next." << endl;
			continue;
		}
	}
	
	freeifaddrs(ifc_header);
	ifc_header = NULL;

	return ret;
}

/*-----------------------------------------------------------------------------
描--述：判断网口为UP 还是Down.
参--数：网口名称，例如eth0.
返回值：0, Down; 1, Up; -1, error
注--意：
-----------------------------------------------------------------------------*/
int Ethernet::isInterfaceUpOrDown(const char *interface)
{
	if(NULL == interface)
	{
		cerr << "Fail to call Ethernet::getInterfaceIP(). Argument has null value." << endl;
		return -1;
	}

	int skfd = 0;
	struct ifreq ifr;
	memset(&ifr, 0, sizeof(struct ifreq));

	skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(-1 == skfd)
	{
		printf("%s:%d Open socket error!\n", __FILE__, __LINE__);
		return -1;
	}

	strcpy(ifr.ifr_name, interface);

	int ret = 0;
	ret = ioctl(skfd, SIOCGIFFLAGS, &ifr);
	if(-1 == ret)
	{
		//printf("%s:%d IOCTL error!\n", __FILE__, __LINE__);
		//printf("Maybe ethernet inferface %s is not valid!", ifr.ifr_name);
		close(skfd);
		skfd = -1;
		return -1;
	}

	close(skfd);
	skfd = -1;

	if(ifr.ifr_flags & IFF_RUNNING)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

/*-----------------------------------------------------------------------------
描--述：获取网口的IP地址；
参--数：interface, 网口名称，例如eth0;
		ipBuf, ipBufLen, 网口IP 地址字符串缓冲区及长度；
返回值：返回网口地址；
注--意：
-----------------------------------------------------------------------------*/
const char *Ethernet::getInterfaceIP(const char *interface, char *ipBuf, unsigned int ipBufLen)
{
	if((NULL == interface) || (NULL == ipBuf))
	{
		cerr << "Fail to call Ethernet::getInterfaceIP(). Argument has null value." << endl;
		return NULL;
	}

	int ret = 0;
	struct ifaddrs *ifc = NULL;

	ret = getifaddrs(&ifc);
	if(ret < 0)
	{
		cerr << "Fail to call getifaddrs(3)." << strerror(errno) << endl;
		return NULL;
	}
	
	struct ifaddrs *ifc_header = NULL;
	ifc_header = ifc;

	for(; NULL != ifc; ifc = ifc->ifa_next)
	{
		if(0 == strcmp(ifc->ifa_name, interface))
		{
			if(AF_INET == ifc->ifa_addr->sa_family)
			{
				//cout << "Find interface and family is AF_INET." << endl;
				inet_ntop(AF_INET, &((struct sockaddr_in *)ifc->ifa_addr)->sin_addr, ipBuf, ipBufLen);
				//cout << ifc->ifa_name << endl;
				//cout << ip << endl;
				break;
			}
			else
			{
				//cerr << "Not AF_INET." << endl;
				continue;
			}
		}
		else
		{
			//cerr << "next." << endl;
			continue;
		}
	}
	
	freeifaddrs(ifc_header);
	ifc_header = NULL;

	if(NULL != ifc)
	{
		return ipBuf;
	}
	else
	{
		return NULL;
	}
}

/*-----------------------------------------------------------------------------
描--述：
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
int Ethernet::getKeywordValue(const char *inputBuf, unsigned int inputBufLen, char *outputBuf, unsigned int outputBufLen, const char *keyWordBegin, const char *keyWordEnd)
{
	if(NULL == inputBuf || NULL == outputBuf || NULL == keyWordBegin || NULL == keyWordEnd)
	{
		cout << "Fail to call Ethernet::getKeywordValue(). Argument has null value." << endl;
		return -1;
	}

	string str(inputBuf);
	string::size_type posBegin = -1;
	posBegin = str.find(keyWordBegin);
	if(string::npos == posBegin)
	{
		cerr << "Can not find keywords in Ethernet::getKeywordValue." << endl;
		return -2;
	}
	posBegin += strlen(keyWordBegin);
	
	string::size_type posEnd = -1;
	posEnd = str.find(keyWordEnd, posBegin);
	if(string::npos == posEnd)
	{
		cerr << "Can not find keywords in Ethernet::getKeywordValue." << endl;
		return -3;
	}

	//cout << inputBuf[posBegin] << endl;
	//cout << inputBuf[posEnd] << endl;
	unsigned int len = 0;
	len = posEnd - posBegin;
	len > outputBufLen ? len = outputBufLen : len;
	memcpy(outputBuf, inputBuf + posBegin, len);
	//cout << outputBuf << endl;

	return 0;
}

/*-----------------------------------------------------------------------------
描--述：获取WiFi 名称，SSID.
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
const char *Ethernet::getWifiSsid(const char *wpaConfPath, char *buf, unsigned int bufLen)
{
	if(NULL == wpaConfPath || NULL == buf)
	{
		cerr << "Fail to call Ethernet::getWifiSsid(). Argument has null value." << endl;
		return NULL;
	}

	int fd = -1;
	fd = open(wpaConfPath, O_RDONLY);
	if(-1 == fd)
	{
		cerr << "Fail to call open(2) in Ethernet::getWifiSsid(). " << strerror(errno) << endl;
		return NULL;
	}

	int ret = 0;
	unsigned int tmpBufSize = 256;
	char tmpBuf[tmpBufSize] = {0};
	ret = read(fd, tmpBuf, tmpBufSize-1);
	if(-1 == ret)
	{
		cerr << "Fail to call read(2) in Ethernet::getWifiSsid(). " << strerror(errno) << endl;
		close(fd);
		fd = -1;
		return NULL;
	}
	
	close(fd);
	fd = -1;

	const char *keywordBegin = "ssid=\"";
	const char *keywordEnd = "\"";
	getKeywordValue(tmpBuf, tmpBufSize, buf, bufLen, keywordBegin, keywordEnd);
	//cout << buf << endl;

	return buf;
}

/*-----------------------------------------------------------------------------
描--述：
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
const char *Ethernet::getWifiPsk(const char *wpaConfPath, char *buf, unsigned int bufLen)
{
	if(NULL == wpaConfPath || NULL == buf)
	{
		cerr << "Fail to call Ethernet::getWifiPsk(). Argument has null value." << endl;
		return NULL;
	}

	int fd = -1;
	fd = open(wpaConfPath, O_RDONLY);
	if(-1 == fd)
	{
		cerr << "Fail to call open(2) in Ethernet::getWifiPsk(). " << strerror(errno) << endl;
		return NULL;
	}

	int ret = 0;
	unsigned int tmpBufSize = 256;
	char tmpBuf[tmpBufSize] = {0};
	ret = read(fd, tmpBuf, tmpBufSize-1);
	if(-1 == ret)
	{
		cerr << "Fail to call read(2) in Ethernet::getWifiPsk(). " << strerror(errno) << endl;
		close(fd);
		fd = -1;
		return NULL;
	}
	
	close(fd);
	fd = -1;

	const char *keywordBegin = "psk=\"";
	const char *keywordEnd = "\"";
	getKeywordValue(tmpBuf, tmpBufSize, buf, bufLen, keywordBegin, keywordEnd);
	//cout << buf << endl;

	return buf;

}

