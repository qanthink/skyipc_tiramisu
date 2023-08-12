/*---------------------------------------------------------------- 
版权所有。
作者：
时间：2020.8.14
----------------------------------------------------------------*/

#include "wifi.h"
#include <string.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <iostream>

using namespace std;

/*-----------------------------------------------------------------------------
描--述：获取当前IPC 到上级的信号强度。
参--数：无
返回值：成功，返回信号强度，失败返回1.
注--意：信号强度为负值，单位为Db
-----------------------------------------------------------------------------*/
int Wifi::getApRssi()
{
	//ut << "Call Wifi::getApRssi()." << endl;
	
	int ret = 0;
	//ret = GetWifiRssi(&stWifiRssi);
	if(0 != ret)
	{
		cerr << "Fail to call Wifi::getApRssi(), ret = " << ret << endl;
		ret = 1;
	}
	else
	{
		//ret = stWifiRssi.aprssi;
	}

	//ut << "Call Wifi::getApRssi() end." << endl;
	return ret;
}

/*-----------------------------------------------------------------------------
描--述：获取IP 地址。
参--数：ipBuf IP 地址缓存区；ipBufSize 缓冲区长度 不得小于16 字节; interfaceName 网口名称
返回值：成功，返回0; 失败，返回linux 错误码; 参数非法，返回-1;
注--意：
-----------------------------------------------------------------------------*/
int Wifi::getIp(char *ipBuf, unsigned ipBufSize, const char *interfaceName)
{
	//cout << "Call Wifi::getIp()." << endl;

	const unsigned minBufSize = 16;	// 255.255.255.255
	if(NULL == ipBuf || ipBufSize < minBufSize || NULL == interfaceName)
	{
		cerr << "Fail to call Wifi::getIp(), bad argument!" << endl;
		return -1;
	}

	// step1: 创建socket 文件描述符。
	int socketFd = 0;
	socketFd = socket(AF_INET, SOCK_STREAM, 0);
	if(-1 == socketFd)
	{
		cerr << "Fail to call socket(2), " << strerror(errno) << endl;
		return socketFd;
	}

	// step2: ioctl 获取IP 地址。
	struct ifreq stIfreq;
	memset(&stIfreq, 0, sizeof(ifreq));	
	strcpy(stIfreq.ifr_name, interfaceName);
	
	int ret = 0;
	ret = ioctl(socketFd, SIOCGIFADDR, &stIfreq);
	if(0 != ret)
	{
		cerr << "Fail to call ioctl(2), " << strerror(errno) << endl;
		close(socketFd);
		return ret;
	}

	// step3: 销毁socket 文件描述符，避免内存泄漏。
	if(-1 != socketFd)
	{
		close(socketFd);
		socketFd = -1;
	}

	strcpy(ipBuf, inet_ntoa(((struct sockaddr_in*)&stIfreq.ifr_addr)->sin_addr));

	//cout << "Call Wifi::getIp() end." << endl;
	return 0;
}

/*-----------------------------------------------------------------------------
描--述：获取本地MAC 地址。
参--数：macBuf MAC地址缓存区；macBufSize 缓冲区长度 不得小于18字节; interfaceName 网口名称
返回值：成功，返回0; 失败，返回linux 错误码; 参数非法，返回-1;
注--意：
-----------------------------------------------------------------------------*/
int Wifi::getMac(char *macBuf, unsigned macBufSize, const char *interfaceName)
{
	//cout << "Call Wifi::getLocalMac()." << endl;

	const unsigned minBufSize = 18;	// 00:00:00:00:00:00
	if(NULL == macBuf || macBufSize < minBufSize)
	{
		cerr << "Fail to call Wifi::getLocalMac(), bad argument!" << endl;
		return -1;
	}

	if(NULL == interfaceName)
	{
		cout << "Argument 3 has null value, use default interface." << endl;
		interfaceName = "wlan0";
	}

	// step1: 创建socket 文件描述符。
	int socketFd = 0;
	socketFd = socket(AF_INET, SOCK_STREAM, 0);
	if(-1 == socketFd)
	{
		cerr << "Fail to call socket(2), " << strerror(errno) << endl;
		return socketFd;
	}

	// step2: ioctl 获取MAC 地址。
	struct ifreq stIfreq;
	memset(&stIfreq, 0, sizeof(ifreq));	
	strcpy(stIfreq.ifr_name, interfaceName);
	int ret = 0;
	ret = ioctl(socketFd, SIOCGIFHWADDR, &stIfreq);
	if(0 != ret)
	{
		cerr << "Fail to call ioctl(2), " << strerror(errno) << endl;
		return ret;
	}

	// step3: 销毁socket 文件描述符，避免内存泄漏。
	if(-1 != socketFd)
	{
		close(socketFd);
		socketFd = -1;
	}

	// step4: 拷贝MAC 地址到用户空间。
    snprintf(macBuf, macBufSize, "%02x:%02x:%02x:%02x:%02x:%02x", \
    	(unsigned char)stIfreq.ifr_hwaddr.sa_data[0], (unsigned char)stIfreq.ifr_hwaddr.sa_data[1], \
    	(unsigned char)stIfreq.ifr_hwaddr.sa_data[2], (unsigned char)stIfreq.ifr_hwaddr.sa_data[3], \
    	(unsigned char)stIfreq.ifr_hwaddr.sa_data[4], (unsigned char)stIfreq.ifr_hwaddr.sa_data[5]);

	//cout << "Call Wifi::getLocalMac() end." << endl;
	return 0;
}

