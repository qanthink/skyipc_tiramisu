/* SigmaStar trade secret */
/* Copyright (c) [2019~2020] SigmaStar Technology.
All rights reserved.

Unless otherwise stipulated in writing, any and all information contained
herein regardless in any format shall remain the sole proprietary of
SigmaStar and be kept in strict confidence
(SigmaStar Confidential Information) by the recipient.
Any unauthorized act including without limitation unauthorized disclosure,
copying, use, reproduction, sale, distribution, modification, disassembling,
reverse engineering and compiling of the contents of SigmaStar Confidential
Information is unlawful and strictly prohibited. SigmaStar hereby reserves the
rights to any and all damages, losses, costs and expenses resulting therefrom.
*/

#ifndef _MI_LDC_DATATYPE_H_
#define _MI_LDC_DATATYPE_H_
#include "mi_sys_datatype.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define LDC_MAXTRIX_NUM 9

    typedef enum
    {
        MI_LDC_WORKMODE_LDC      = 0x01,
        MI_LDC_WORKMODE_LUT      = 0x02,
        MI_LDC_WORKMODE_DIS_GYRO = 0x04,
    } MI_LDC_WorkMode_e;

    typedef enum
    {
        MI_LDC_MAPINFOTYPE_DISPMAP,
        MI_LDC_MAPINFOTYPE_SENSORCALIB,
    } MI_LDC_MapInfoType_e;

    typedef struct MI_LDC_DevAttr_s
    {
        MI_U32 u32Reserved;
    } MI_LDC_DevAttr_t;

    typedef struct MI_LDC_DisplacementMapInfo_s
    {
        void*  pXmapAddr;
        void*  pYmapAddr;
        MI_U32 u32XmapSize;
        MI_U32 u32YmapSize;
    } MI_LDC_DisplacementMapInfo_t;

    typedef struct MI_LDC_SensorCalibInfo_s
    {
        void*  pCalibPolyBinAddr;
        MI_U32 u32CalibPolyBinSize;
    } MI_LDC_SensorCalibInfo_t;

    typedef struct MI_LDC_ChnParam_s
    {
        MI_BOOL bBypass;
        MI_BOOL bUseProjection3x3Matrix;
        MI_S32  as32Projection3x3Matrix[LDC_MAXTRIX_NUM];
        MI_U32  u32FocalLengthX;
        MI_U32  u32FocalLengthY;

        void*  pConfigAddr;
        MI_U32 u32ConfigSize;
        union
        {
            MI_LDC_DisplacementMapInfo_t stDispMapInfo;
            MI_LDC_SensorCalibInfo_t     stCalibInfo;
        };
    } MI_LDC_ChnParam_t;

    typedef struct MI_LDC_ChnAttr_s
    {
        MI_LDC_WorkMode_e eMode;

        MI_BOOL bBypass;
        MI_BOOL bUseProjection3x3Matrix;
        MI_S32  as32Projection3x3Matrix[LDC_MAXTRIX_NUM];
        MI_U16  u16UserSliceNum;
        MI_U32  u32FocalLengthX;
        MI_U32  u32FocalLengthY;

        void*  pConfigAddr;
        MI_U32 u32ConfigSize;

        MI_LDC_MapInfoType_e eInfoType;
        union
        {
            MI_LDC_DisplacementMapInfo_t stDispMapInfo;
            MI_LDC_SensorCalibInfo_t     stCalibInfo;
        };
    } MI_LDC_ChnAttr_t;

    typedef struct MI_LDC_DirectBuf_s
    {
        MI_SYS_PixelFormat_e ePixelFormat;
        MI_U32               u32Width;
        MI_U32               u32Height;
        MI_U32               u32Stride[2];
        MI_PHY               phyAddr[2];
    } MI_LDC_DirectBuf_t;

    typedef struct MI_LDC_LutTaskAttr_s
    {
        struct MI_LDC_LutSrcBuf_s
        {
            MI_LDC_DirectBuf_t stTableX;
            MI_LDC_DirectBuf_t stTableY;
            MI_LDC_DirectBuf_t stTableWeight;
        } stSrcBuf;

        MI_LDC_DirectBuf_t stDstBuf;
    } MI_LDC_LutTaskAttr_t;

    typedef enum
    {
        E_MI_LDC_ERR_DEV_CREATED = MI_LDC_INITIAL_ERROR_CODE, // dev has been created
        E_MI_LDC_ERR_DEV_NOT_CREATE,                          // dev not be created
        E_MI_LDC_ERR_DEV_NOT_DESTROY,                         // dev not be destroyed
        E_MI_LDC_ERR_CHN_CREATED,                             // chn has been created
        E_MI_LDC_ERR_CHN_NOT_CREATE,                          // chn not be created
        E_MI_LDC_ERR_CHN_NOT_STOP,                            // chn is still working
        E_MI_LDC_ERR_CHN_NOT_DESTROY,                         // chn not be destroyed
        E_MI_LDC_ERR_PORT_NOT_UNBIND,                         // port not unbind
    } MI_LDC_ErrCode_e;

    typedef MI_U32 MI_LDC_DEV;
    typedef MI_U32 MI_LDC_CHN;

    typedef struct MI_LDC_OutputPortAttr_s
    {
        MI_U16               u16Width;
        MI_U16               u16Height;
        MI_SYS_PixelFormat_e ePixelFmt;
    } MI_LDC_OutputPortAttr_t;

#define POW2(n)  (1 << n)
#define ROUND(x) ((MI_S32)(x + 0.5))
#define FLOAT2FIX(float_num, int_num, frac_num) \
    (ROUND(float_num * POW2(frac_num)) > ROUND(int_num) ? ROUND(float_num * POW2(frac_num)) : 0)
#define FIX2FLOAT(sint_num, int_num, frac_num) ((sint_num > POW2(int_num)) ? sint_num / (POW2(frac_num) * 1.0) : 0)

#define MI_LDC_OK                   MI_DEF_ERR(E_MI_MODULE_ID_LDC, E_MI_ERR_LEVEL_INFO, MI_SUCCESS)
#define MI_ERR_LDC_ILLEGAL_PARAM    MI_DEF_ERR(E_MI_MODULE_ID_LDC, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_ILLEGAL_PARAM)
#define MI_ERR_LDC_NULL_PTR         MI_DEF_ERR(E_MI_MODULE_ID_LDC, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NULL_PTR)
#define MI_ERR_LDC_NOMEM            MI_DEF_ERR(E_MI_MODULE_ID_LDC, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NOMEM)
#define MI_ERR_LDC_BUSY             MI_DEF_ERR(E_MI_MODULE_ID_LDC, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_BUSY)
#define MI_ERR_LDC_FAIL             MI_DEF_ERR(E_MI_MODULE_ID_LDC, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_FAILED)
#define MI_ERR_LDC_INVALID_DEVID    MI_DEF_ERR(E_MI_MODULE_ID_LDC, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_INVALID_DEVID)
#define MI_ERR_LDC_INVALID_CHNID    MI_DEF_ERR(E_MI_MODULE_ID_LDC, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_INVALID_CHNID)
#define MI_ERR_LDC_NOT_SUPPORT      MI_DEF_ERR(E_MI_MODULE_ID_LDC, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NOT_SUPPORT)
#define MI_ERR_LDC_MOD_INITED       MI_DEF_ERR(E_MI_MODULE_ID_LDC, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_INITED)
#define MI_ERR_LDC_MOD_NOT_INIT     MI_DEF_ERR(E_MI_MODULE_ID_LDC, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NOT_INIT)
#define MI_ERR_LDC_DEV_CREATED      MI_DEF_ERR(E_MI_MODULE_ID_LDC, E_MI_ERR_LEVEL_ERROR, E_MI_LDC_ERR_DEV_CREATED)
#define MI_ERR_LDC_DEV_NOT_CREATE   MI_DEF_ERR(E_MI_MODULE_ID_LDC, E_MI_ERR_LEVEL_ERROR, E_MI_LDC_ERR_DEV_NOT_CREATE)
#define MI_ERR_LDC_DEV_NOT_DESTROY  MI_DEF_ERR(E_MI_MODULE_ID_LDC, E_MI_ERR_LEVEL_ERROR, E_MI_LDC_ERR_DEV_NOT_DESTROY)
#define MI_ERR_LDC_CHN_CREATED      MI_DEF_ERR(E_MI_MODULE_ID_LDC, E_MI_ERR_LEVEL_ERROR, E_MI_LDC_ERR_CHN_CREATED)
#define MI_ERR_LDC_CHN_NOT_CREATE   MI_DEF_ERR(E_MI_MODULE_ID_LDC, E_MI_ERR_LEVEL_ERROR, E_MI_LDC_ERR_CHN_NOT_CREATE)
#define MI_ERR_LDC_CHN_NOT_STOP     MI_DEF_ERR(E_MI_MODULE_ID_LDC, E_MI_ERR_LEVEL_ERROR, E_MI_LDC_ERR_CHN_NOT_STOP)
#define MI_ERR_LDC_CHN_NOT_DESTROY  MI_DEF_ERR(E_MI_MODULE_ID_LDC, E_MI_ERR_LEVEL_ERROR, E_MI_LDC_ERR_CHN_NOT_DESTROY)
#define MI_ERR_LDC_PORT_NOT_DISABLE MI_DEF_ERR(E_MI_MODULE_ID_LDC, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NOT_DISABLE)
#define MI_ERR_LDC_PORT_NOT_UNBIND  MI_DEF_ERR(E_MI_MODULE_ID_LDC, E_MI_ERR_LEVEL_ERROR, E_MI_LDC_ERR_PORT_NOT_UNBIND)

#ifdef __cplusplus
}
#endif

#endif ///_MI_LDC_DATATYPE_H_
