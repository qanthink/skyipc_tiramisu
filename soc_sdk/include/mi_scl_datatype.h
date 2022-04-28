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
#ifndef _MI_SCL_DATATYPE_H_
#define _MI_SCL_DATATYPE_H_
#include "mi_sys_datatype.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef enum
{
    E_MI_ERR_SCL_INVALID_DEVID,
    E_MI_ERR_SCL_INVALID_CHNID,
    E_MI_ERR_SCL_INVALID_PORTID,
    E_MI_ERR_SCL_ILLEGAL_PARAM,
    E_MI_ERR_SCL_EXIST,
    E_MI_ERR_SCL_UNEXIST,
    E_MI_ERR_SCL_NULL_PTR,
    E_MI_ERR_SCL_NOT_SUPPORT,
    E_MI_ERR_SCL_NOT_PERM,
    E_MI_ERR_SCL_NOMEM,
    E_MI_ERR_SCL_NOBUF,
    E_MI_ERR_SCL_BUF_EMPTY,
    E_MI_ERR_SCL_NOTREADY,
    E_MI_ERR_SCL_BUSY,
} MI_SCL_ErrCode_e;

#define MI_SCL_OK (0)
#define MI_ERR_SCL_INVALID_DEVID        MI_DEF_ERR(E_MI_MODULE_ID_SCL, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_SCL_INVALID_DEVID) //0XA0222000
#define MI_ERR_SCL_INVALID_CHNID        MI_DEF_ERR(E_MI_MODULE_ID_SCL, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_SCL_INVALID_CHNID) //0XA0222001
#define MI_ERR_SCL_INVALID_PORTID       MI_DEF_ERR(E_MI_MODULE_ID_SCL, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_SCL_INVALID_PORTID)
#define MI_ERR_SCL_ILLEGAL_PARAM        MI_DEF_ERR(E_MI_MODULE_ID_SCL, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_SCL_ILLEGAL_PARAM)
#define MI_ERR_SCL_EXIST                MI_DEF_ERR(E_MI_MODULE_ID_SCL, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_SCL_EXIST)
#define MI_ERR_SCL_UNEXIST              MI_DEF_ERR(E_MI_MODULE_ID_SCL, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_SCL_UNEXIST)
#define MI_ERR_SCL_NULL_PTR             MI_DEF_ERR(E_MI_MODULE_ID_SCL, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_SCL_NULL_PTR)
#define MI_ERR_SCL_NOT_SUPPORT          MI_DEF_ERR(E_MI_MODULE_ID_SCL, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_SCL_NOT_SUPPORT)
#define MI_ERR_SCL_NOT_PERM             MI_DEF_ERR(E_MI_MODULE_ID_SCL, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_SCL_NOT_PERM)
#define MI_ERR_SCL_NOMEM                MI_DEF_ERR(E_MI_MODULE_ID_SCL, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_SCL_NOMEM)
#define MI_ERR_SCL_NOBUF                MI_DEF_ERR(E_MI_MODULE_ID_SCL, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_SCL_NOBUF)
#define MI_ERR_SCL_BUF_EMPTY            MI_DEF_ERR(E_MI_MODULE_ID_SCL, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_SCL_BUF_EMPTY)
#define MI_ERR_SCL_NOTREADY             MI_DEF_ERR(E_MI_MODULE_ID_SCL, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_SCL_NOTREADY)
#define MI_ERR_SCL_BUSY                 MI_DEF_ERR(E_MI_MODULE_ID_SCL, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_SCL_BUSY)

typedef MI_U32 MI_SCL_DEV;
typedef MI_U32 MI_SCL_CHANNEL;
typedef MI_U32 MI_SCL_PORT;

typedef enum
{
    E_MI_SCL_HWSCLID_INVALID= 0,
    E_MI_SCL_HWSCL0         = 0x0001,
    E_MI_SCL_HWSCL1         = 0x0002,
    E_MI_SCL_HWSCL2         = 0x0004,
    E_MI_SCL_HWSCL3         = 0x0008,
    E_MI_SCL_HWSCL4         = 0x0010,
    E_MI_SCL_HWSCL5         = 0x0020,
    E_MI_SCL_HWSCL_MAX      = 0xffff,
}MI_SCL_HWSclId_e;

typedef struct MI_SCL_DevAttr_s
{
    MI_U32  u32NeedUseHWOutPortMask;  //内部做port id 到scl id 的mapping，app use portid 可以都用0~5
}MI_SCL_DevAttr_t;

typedef struct MI_SCL_ChannelAttr_s
{
    MI_U32  u32Reserved;
}MI_SCL_ChannelAttr_t;

typedef struct MI_SCL_ChnParam_s
{
    MI_SYS_Rotate_e eRot; //for source E_MI_SCL_SRC_FROM_RDMA_ROT
}MI_SCL_ChnParam_t;

typedef struct MI_SCL_OutPortParam_s
{
    MI_SYS_WindowRect_t stSCLOutCropRect;
    MI_SYS_WindowSize_t stSCLOutputSize;
    MI_BOOL bMirror;
    MI_BOOL bFlip;
    MI_SYS_PixelFormat_e  ePixelFormat;
    MI_SYS_CompressMode_e eCompressMode;
}MI_SCL_OutPortParam_t;

typedef enum
{
    E_MI_SCL_FILTER_TYPE_AUTO,
    E_MI_SCL_FILTER_TYPE_BYPASS,
    E_MI_SCL_FILTER_TYPE_BILINEAR,
    E_MI_SCL_FILTER_TYPE_BICUBIC,
    E_MI_SCL_FILTER_TYPE_SELFDEFINED,
    E_MI_SCL_FILTER_TYPE_MAX,
} MI_SCL_FilterType_e;

typedef struct MI_SCL_DirectBuf_s
{
    MI_SYS_PixelFormat_e ePixelFormat;
    MI_U32 u32Width;
    MI_U32 u32Height;
    MI_PHY phyAddr[2];
    MI_U32 u32Stride[2];
    MI_U32 u32BuffSize;
}MI_SCL_DirectBuf_t;


#ifdef __cplusplus
}
#endif
#endif

