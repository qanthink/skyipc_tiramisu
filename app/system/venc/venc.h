/*---------------------------------------------------------------- 
xxx版权所有。
作者：
时间：2020.7.10
----------------------------------------------------------------*/

#pragma once

#include "mi_venc.h"

// 1080P 的YUV 约2.97MB, 经过H.265 编码协议压缩后，I帧约30KB, P帧约7KB
// 在图像质量较好时，I 帧的大小会超过32K. 故而将缓存区设为 128KB 更保险.
// H.265 1920x1080@30FPS 2Mbps, 有I帧=67.927 KB的情况。


class Venc{
public:
	// 主码流和子码流的通道号。
	const static MI_VENC_CHN vencMainChn = 0;
	const static MI_VENC_CHN vencSubChn = 1;
	const static MI_VENC_CHN vencJpegChn = 2;
	const static bool bUseSliceMode = false;

	const static unsigned int superMaxISize = 128 * 1024;

	static Venc *getInstance();
	MI_S32 enable();
	MI_S32 disable();

	/* 上层用户最常调用：请求I 帧、改变码率、获取流、打印流信息。 */
	MI_S32 requestIdr(MI_VENC_DEV vencDev, MI_VENC_CHN VeChn, MI_BOOL bInstant);
	MI_S32 changeBitrate(MI_VENC_DEV vencDev, MI_VENC_CHN VeChn, MI_U32 u32BitrateKb);
	void printStreamInfo(const MI_VENC_Stream_t* pstVencStream);
	int rcvStream(MI_VENC_DEV vencDev, MI_VENC_CHN vencChn, MI_VENC_Stream_t *pstStream);

	MI_S32 getChnAttr(MI_VENC_DEV vencDev, MI_VENC_CHN VeChn, MI_VENC_ChnAttr_t *pstChnAttr);
	MI_S32 setChnAttr(MI_VENC_DEV vencDev, MI_VENC_CHN VeChn, MI_VENC_ChnAttr_t *pstChnAttr);

	MI_S32 setCrop(MI_VENC_CHN vencChn, MI_U32 x, MI_U32 y, MI_U32 w, MI_U32 h);

	MI_S32 createStreamWithAttr(MI_VENC_DEV vencDev, MI_VENC_CHN vencChn, MI_VENC_ChnAttr_t *pstChnAttr);
	MI_S32 createH264Stream(MI_VENC_DEV venvDev, MI_VENC_CHN vencChn, unsigned int width, unsigned int height);
	MI_S32 createH265Stream(MI_VENC_DEV venvDev, MI_VENC_CHN vencChn, unsigned int width, unsigned int height);
	MI_S32 createJpegStream(MI_VENC_DEV venvDev, MI_VENC_CHN vencChn, unsigned int width, unsigned int height);
	MI_S32 setH264SliceSplit(MI_VENC_CHN vencChn, MI_VENC_ParamH264SliceSplit_t *pstSliceSplit);
	MI_S32 setH265SliceSplit(MI_VENC_CHN vencChn, MI_VENC_ParamH265SliceSplit_t *pstSliceSplit);
	MI_S32 setInputBufMode(MI_VENC_DEV venvDev, MI_VENC_CHN vencChn,
			MI_VENC_InputSrcBufferMode_e eVencBufMode);
	
	MI_S32 createChn(MI_VENC_DEV vencDev, MI_VENC_CHN vencChn, MI_VENC_ChnAttr_t *pstChnAttr);
	MI_S32 destroyChn(MI_VENC_DEV vencDev, MI_VENC_CHN vencChn);
	MI_S32 startRecvPic(MI_VENC_DEV vencDev, MI_VENC_CHN vencChn);
	MI_S32 stopRecvPic(MI_VENC_DEV vencDev, MI_VENC_CHN vencChn);
	
	MI_S32 getFd(MI_VENC_DEV vencDev, MI_VENC_CHN vencChn);
	MI_S32 query(MI_VENC_DEV vencDev, MI_VENC_CHN vencChn, MI_VENC_ChnStat_t *pstChnStat);
	MI_S32 getStream(MI_VENC_DEV vencDev, MI_VENC_CHN vencChn, MI_VENC_Stream_t *pstStream, MI_S32 s32MilliSec);
	MI_S32 releaseStream(MI_VENC_DEV vencDev, MI_VENC_CHN vencChn, MI_VENC_Stream_t *pstStream);
	
	MI_S32 getJpegParam(MI_VENC_CHN vencChn, MI_VENC_ParamJpeg_t *pstJpegParam);
	MI_S32 setJpegParam(MI_VENC_CHN vencChn, MI_VENC_ParamJpeg_t *pstJpegParam);

	MI_VENC_ModType_e getVesType(MI_VENC_DEV vencDev, MI_VENC_CHN vencChn);
	int setVesType(MI_VENC_DEV vencDev, MI_VENC_CHN vencChn, MI_VENC_ModType_e vesType);

	int setResolution(MI_VENC_DEV vencDev, MI_VENC_CHN vencCh, int width, int height);
	int getResolution(MI_VENC_DEV vencDev, MI_VENC_CHN vencCh, int *pWidth, int *pHeight);

	int isChannelExists(MI_VENC_CHN vencCh);
	
private:
	Venc();
	~Venc();
	Venc(const Venc&);
	Venc& operator=(const Venc&);

	bool bEnable;
};

