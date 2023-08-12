/*---------------------------------------------------------------- 
xxx版权所有。
作者：
时间：2022.2.13
----------------------------------------------------------------*/

/*
*/

#include "ethernet.h"
#include "spipanel.h"

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
描--述：
参--数：
返回值：
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
描--述：
参--数：
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
描--述：
参--数：
返回值：
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
描--述：
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

int Ethernet::showWlanInfOnPanel()
{
	SpiPanel *pSpipanel = pSpipanel->getInstance();
	pSpipanel->panelFill(0, 0, PANEL_WIDTH, PANEL_HEIGHT, SPIPANEL_COLOR_BLACK);

	unsigned int fontSize = 16;
	unsigned int xStart = 0;
	unsigned int xSecondCol = PANEL_WIDTH / 2 - 10;
	unsigned int yStart = 0;
	unsigned deltaY = fontSize + fontSize / 4;

	#if 1
	pSpipanel->panelShowText(xStart, yStart, "正在连接WIFI:", SPIPANEL_COLOR_WHITE, SPIPANEL_COLOR_BLACK, fontSize, true);
	//pSpipanel->panelShowString(xSecondCol, yStart, "360-wifi", SPIPANEL_COLOR_WHITE, SPIPANEL_COLOR_BLACK, fontSize, true);
	//pSpipanel->panelShowText(xStart, yStart + deltaY, "密令:", SPIPANEL_COLOR_WHITE, SPIPANEL_COLOR_BLACK, fontSize, true);
	//pSpipanel->panelShowString(xSecondCol, yStart + deltaY, "00000000", SPIPANEL_COLOR_WHITE, SPIPANEL_COLOR_BLACK, fontSize, true);
	pSpipanel->panelShowText(xStart, yStart + deltaY * 1, "设备IP:", SPIPANEL_COLOR_WHITE, SPIPANEL_COLOR_BLACK, fontSize, true);
	pSpipanel->panelShowString(xSecondCol, yStart + deltaY * 1, "", SPIPANEL_COLOR_WHITE, SPIPANEL_COLOR_BLACK, fontSize, true);

	pSpipanel->panelShowText(xStart, yStart + deltaY * 3, "已打开热点:", SPIPANEL_COLOR_WHITE, SPIPANEL_COLOR_BLACK, fontSize, true);
	pSpipanel->panelShowString(xSecondCol, yStart + deltaY * 3, "skyipc", SPIPANEL_COLOR_WHITE, SPIPANEL_COLOR_BLACK, fontSize, true);
	pSpipanel->panelShowText(xStart, yStart + deltaY * 4, "密令:", SPIPANEL_COLOR_WHITE, SPIPANEL_COLOR_BLACK, fontSize, true);
	pSpipanel->panelShowString(xSecondCol, yStart + deltaY * 4, "12345678", SPIPANEL_COLOR_WHITE, SPIPANEL_COLOR_BLACK, fontSize, true);
	pSpipanel->panelShowText(xStart, yStart + deltaY * 5, "设备IP:", SPIPANEL_COLOR_WHITE, SPIPANEL_COLOR_BLACK, fontSize, true);
	pSpipanel->panelShowText(xStart, yStart + deltaY * 7, "浏览资料:", SPIPANEL_COLOR_WHITE, SPIPANEL_COLOR_BLACK, fontSize, true);
	pSpipanel->panelShowText(xStart, yStart + deltaY * 8, "用户名:", SPIPANEL_COLOR_WHITE, SPIPANEL_COLOR_BLACK, fontSize, true);
	pSpipanel->panelShowText(xSecondCol, yStart + deltaY * 8, "share", SPIPANEL_COLOR_WHITE, SPIPANEL_COLOR_BLACK, fontSize, true);
	pSpipanel->panelShowText(xStart, yStart + deltaY * 9, "修改WIFI:", SPIPANEL_COLOR_WHITE, SPIPANEL_COLOR_BLACK, fontSize, true);
	pSpipanel->panelShowText(xStart, yStart + deltaY * 10, "用户名:", SPIPANEL_COLOR_WHITE, SPIPANEL_COLOR_BLACK, fontSize, true);
	pSpipanel->panelShowText(xSecondCol, yStart + deltaY * 10, "root", SPIPANEL_COLOR_WHITE, SPIPANEL_COLOR_BLACK, fontSize, true);
	//pSpipanel->panelShowString(xSecondCol, yStart + deltaY * 6, "192.168.1.1", SPIPANEL_COLOR_WHITE, SPIPANEL_COLOR_BLACK, fontSize, true);
	#endif
	//pSpipanel->panelShowText(xStart, yStart + deltaY * 7, "", SPIPANEL_COLOR_WHITE, SPIPANEL_COLOR_BLACK, fontSize, true);

//	while(g_bRunning)
	while(true)
	{
		const unsigned int bufLen = 16;
		char buf[bufLen] = {0};
		const char *interface = NULL;
		const char *pRet = NULL;

		//interface = "wlan0";
		(1 == isInterfaceUpOrDown("br0")) ? interface = "br0" : interface = "wlan0";
		pRet = getInterfaceIP(interface, buf, bufLen);
		if(NULL != pRet)
		{
			pSpipanel->panelShowString(xSecondCol, yStart + deltaY * 1, buf, SPIPANEL_COLOR_WHITE, SPIPANEL_COLOR_BLACK, fontSize, true);
			pSpipanel->panelShowString(xSecondCol, yStart + deltaY * 7, buf, SPIPANEL_COLOR_WHITE, SPIPANEL_COLOR_BLACK, fontSize, true);
		}
		//cout << interface << " IP: " << buf << endl;

		//interface = "wlan1";
		(1 == isInterfaceUpOrDown("br0")) ? interface = "br0" : interface = "wlan1";
		pRet = getInterfaceIP(interface, buf, bufLen);
		if(NULL != pRet)
		{
			pSpipanel->panelShowString(xSecondCol, yStart + deltaY * 5, buf, SPIPANEL_COLOR_WHITE, SPIPANEL_COLOR_BLACK, fontSize, true);
			pSpipanel->panelShowString(xSecondCol, yStart + deltaY * 9, buf, SPIPANEL_COLOR_WHITE, SPIPANEL_COLOR_BLACK, fontSize, true);
		}
		//cout << interface << " IP: " << buf << endl;

		const char * wpaConfPath = "/customer/wifi/etc_user/wpa_supplicant.conf";
		memset(buf, 0, bufLen);
		getWifiSsid(wpaConfPath, buf, bufLen);
		pSpipanel->panelShowString(xSecondCol, yStart, buf, SPIPANEL_COLOR_WHITE, SPIPANEL_COLOR_BLACK, fontSize, true);
		pSpipanel->panelShowString(xSecondCol + strlen(buf) * fontSize / 2, yStart, "               ", SPIPANEL_COLOR_WHITE, SPIPANEL_COLOR_BLACK, fontSize, true);

		memset(buf, 0, bufLen);
		//getWifiPsk(wpaConfPath, buf, bufLen);
		//pSpipanel->panelShowString(xSecondCol, yStart + deltaY, buf, SPIPANEL_COLOR_WHITE, SPIPANEL_COLOR_BLACK, fontSize, true);
		//pSpipanel->panelShowString(xSecondCol + strlen(buf) * fontSize / 2, yStart + deltaY, "               ", SPIPANEL_COLOR_WHITE, SPIPANEL_COLOR_BLACK, fontSize, true);
		
		sleep(1);
	}

	return -1;
}

