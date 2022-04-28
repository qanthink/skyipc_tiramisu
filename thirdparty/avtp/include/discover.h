//这个类用于定时发送广播，好让nvr发现ipc设备  Jerry.zhu 2020-08-11
#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <strings.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <semaphore.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/un.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <sys/timeb.h>
#include <memory.h>
#include <dlfcn.h>
#include <string>
#include <thread>
#include <stddef.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <string>
#include <iostream>


class Discover
{
private:
	Discover();
	static void recvThread(Discover *pThis);//接收nvr扫描设备广播的线程
	void sendResponse();//返回应答信息给nvr
public:
	static Discover* getInstance();
	void start(std::string deviceName);
private:
	std::string deviceName = "";//本地无线网卡设备名称
	std::string deviceId = "";//ipc设备唯一标识，目前使用网卡mac地址
};

