#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <sys/time.h>
#include <unistd.h>

#define DE_BufLen 1380

#pragma pack (1)
struct UdpProPackage
{
	unsigned char		head = 0x7f;				//固定不变
	unsigned char		version = 0x0;				//版本，固定不变
	unsigned char		packageType = 0x20;	//a)	控制 0x0   b)	音频 0x20,  c)	视频 0x30
	unsigned char		encodeType = 0x3;		//ACK 0x0 ，H264 0x1，H265 0x2，AAC 0x3，OPUS 0x4
	unsigned short	frameNum = 0;			//(音视频)帧序号
	unsigned short	serialNum = 0;				//(单帧内)包序号
	unsigned short	totalNum = 1;				//(单帧)包总数量
	unsigned short	dataLen = 0;				//buf的有效数据长度
	unsigned int		theTime = 0;				//时间戳 不使用
	unsigned char		tmp[4] = { 0 };				//保留
	unsigned char		buf[DE_BufLen];		    //存储数据载荷 

	unsigned long long getTimeStamp()
	{
		struct timeval tv;
		gettimeofday(&tv, NULL);
		unsigned long long ret = tv.tv_sec * 1000 + tv.tv_usec / 1000;//毫秒
		return ret;
	}

	bool setAudioData(const unsigned char* data, int len, unsigned short& nextFramNum)//插入音频数据
	{
		if (!data || len < 1 || len > sizeof(buf))
			return false;
		head = 0x7f;
		version = 0x0;
		theTime = getTimeStamp();
		memcpy(buf, data, len);
		dataLen = len;
		packageType = 0x20;
		encodeType = 0x3;
		frameNum = nextFramNum++;
		serialNum = 0;
		totalNum = 1;
		return true;
	}
	bool setVideoData(const unsigned char* data, int len, unsigned short framNum, unsigned char encodeType, unsigned short serialNum, unsigned short totalNum)//插入音频数据
	{
		if (!data || len < 1 || len > sizeof(buf))
			return false;
		head = 0x7f;
		version = 0x0;
		theTime = getTimeStamp();
		memcpy(buf, data, len);
		dataLen = len;
		this->packageType = 0x30;
		this->encodeType = encodeType;
		this->frameNum = framNum;
		this->serialNum = serialNum;
		this->totalNum = totalNum;
		return true;
	}
};
#pragma pack ()
