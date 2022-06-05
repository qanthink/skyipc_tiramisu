/*---------------------------------------------------------------- 
xxx版权所有。
作者：
时间：2020.7.10
----------------------------------------------------------------*/

/*	注意：
	虽然VENC 支持图像缩放，但是建议在VPE 模块来做。
	
	2022.6.5: 没有看到任何文档表明VENC 可以做sacle.
*/

#pragma once

#include "mi_venc.h"

typedef enum{
	emResolInvalid = 0x00,
	
	emResol1MP_1280_720 = 0x10,

	emResol2MP_1920_1080 = 0x20,
	
	emResol3MP = 0x30,
	
	emResol4MP_2560_1440 = 0x40,
	
	emResol5MP_3072_1728 = 0x50,
	
	emResol6MP = 0x60,
	
	emResol7MP = 0x70,
	
	emResol8MP_3840_2160 = 0x80,
}emResol_t;

// 1080P 的YUV 约2.97MB, 经过H.265 编码协议压缩后，I帧约30KB, P帧约7KB
// 在图像质量较好时，I 帧的大小会超过32K. 故而将缓存区设为 128KB 更保险.
// H.265 1920x1080@30FPS 2Mbps, 有I帧=67.927 KB的情况。
struct stStreamPack_t{
	const static MI_U32 u32PackMaxLen = 128 * 1024;	// N * 1024 = N KB.
	MI_U8 u8Pack[u32PackMaxLen];	// H.26X编码数据。
	MI_U32 u32Len;					// H.26X编码数据长度。
	MI_U64 u64PTS;					// 时间戳。
};

class Venc{
public:
	// 主码流和子码流的通道号。
	const static MI_VENC_DEV vencDev = 0;
	const static MI_VENC_CHN vencMainChn = 0;
	const static MI_VENC_CHN vencSubChn = 1;
	const static MI_VENC_CHN vencJpegChn = 2;
	const static bool bUseSliceMode = false;

	static Venc *getInstance();
	MI_S32 enable();
	MI_S32 disable();

	/* 上层用户最常调用：请求I 帧、改变码率、获取流、打印流信息。 */
	MI_S32 requestIdr(MI_VENC_CHN VeChn, MI_BOOL bInstant);
	MI_S32 changeBitrate(MI_VENC_CHN VeChn, MI_U32 u32BitrateKb);
	void printStreamInfo(const MI_VENC_Stream_t* pstVencStream);
	int rcvStream(MI_VENC_CHN vencChn, MI_VENC_Stream_t *pstStream);

	MI_S32 getChnAttr(MI_VENC_CHN VeChn, MI_VENC_ChnAttr_t *pstChnAttr);
	MI_S32 setChnAttr(MI_VENC_CHN VeChn, MI_VENC_ChnAttr_t *pstChnAttr);

	MI_S32 setCrop(MI_VENC_CHN vencChn, MI_U32 x, MI_U32 y, MI_U32 w, MI_U32 h);

	MI_S32 createMainStream(MI_VENC_CHN vencChn, MI_VENC_ChnAttr_t *pstChnAttr);
	MI_S32 createSubStream(MI_VENC_CHN vencChn, MI_VENC_ChnAttr_t *pstChnAttr);
	MI_S32 createJpegStream(MI_VENC_CHN vencChn, MI_VENC_ChnAttr_t *pstChnAttr);
	MI_S32 setH265SliceSplit(MI_VENC_CHN VeChn, MI_VENC_ParamH265SliceSplit_t *pstSliceSplit);
	
	MI_S32 createChn(MI_VENC_CHN vencChn, MI_VENC_ChnAttr_t *pstChnAttr);
	MI_S32 destroyChn(MI_VENC_CHN vencChn);
	MI_S32 startRecvPic(MI_VENC_CHN vencChn);
	MI_S32 stopRecvPic(MI_VENC_CHN vencChn);
	
	MI_S32 getFd(MI_VENC_CHN vencChn);
	MI_S32 query(MI_VENC_CHN vencChn, MI_VENC_ChnStat_t *pstChnStat);
	MI_S32 getStream(MI_VENC_CHN vencChn, MI_VENC_Stream_t *pstStream, MI_S32 s32MilliSec);
	MI_S32 releaseStream(MI_VENC_CHN vencChn, MI_VENC_Stream_t *pstStream);
	
	MI_S32 getJpegParam(MI_VENC_CHN vencChn, MI_VENC_ParamJpeg_t *pstJpegParam);
	MI_S32 setJpegParam(MI_VENC_CHN vencChn, MI_VENC_ParamJpeg_t *pstJpegParam);
	
private:
	Venc();
	~Venc();
	Venc(const Venc&);
	Venc& operator=(const Venc&);

	bool bEnable;

	// 主码流输入分辨率
	//emResol_t emMainResIn = emResolInvalid;
	//emResol_t emMainResIn = emResol2MP_1920_1080;
	emResol_t emMainResIn = emResol4MP_2560_1440;
	//emResol_t emMainResIn = emResol8MP_3840_2160;

	// 主码流输出分辨率
	emResol_t emMainResOut = emResol2MP_1920_1080;
	//emResol_t emMainResOut = emResol4MP_2560_1440;

	// 主码流码率、GOP、FPS
	unsigned int mainBitRateKb = 8 * 1024;
	unsigned int mainGop = 15;
	unsigned int mainFrmRate = 20;

	// 子码流输入、输出分辨率、GOP
	emResol_t emSubResIn = emResol2MP_1920_1080;
	emResol_t emSubResOut = emResol2MP_1920_1080;
	unsigned int subBitRateKb = 1 * 1024;
	unsigned int subGop = 15;
};

