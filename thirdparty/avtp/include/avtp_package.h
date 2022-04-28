#pragma once

#ifndef PACKBUF_SIZE
#define PACKBUF_SIZE 65000
#endif // !PACKBUF_SIZE

enum DATA_TYPE
{
    DATA_Default = 0,
    DATA_Audio,
    DATA_H264,
    DATA_H265,
    DATA_Control,
    DATA_Control_Fb//作为可靠传输DATA_Control的返回消息类型
};

struct PKT_HEAD
{
    unsigned int id_code = 0x66;//固定值
    unsigned int version = 0x01;//版本号
    enum DATA_TYPE data_type = DATA_Default;//数据类型
    unsigned short buffer_len = 0;//分片的总长度---包体data的有效数据长度
    unsigned short stream_num = 0;//顺序增加---帧编号;  对于控制协议，DATA_Control、DATA_Control_UserDefine等，存放命令id
    unsigned char sliceID = 0;//分片编号
    unsigned int timestamp = 0;//时间戳
    unsigned int reserve = 0;//保留位
};

struct PKT_INFO
{
    struct PKT_HEAD head;
    char data[PACKBUF_SIZE];
};


void packageHead_setVal(PKT_HEAD *pHead, DATA_TYPE dataType, unsigned short bufferLen = 0, unsigned short streamNum = 0, unsigned char sliceId = 0);
/*
DATA_Control数句包

上报设备：
data：   device:xxxxx

开始video
data:   startVideo
停止video
data:   stopVideo


开始audio
data:   startAudio
停止video
data:   stopAudio

*/