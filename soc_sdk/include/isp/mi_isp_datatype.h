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
#ifndef _MI_ISP_DATATYPE_H_
#define _MI_ISP_DATATYPE_H_
#include "mi_sys_datatype.h"

#ifdef __cplusplus
extern "C" {
#endif


#define MI_ISP_OK (0)
#define MI_ERR_ISP_INVALID_CHNID (0xA0078001)
#define MI_ERR_ISP_INVALID_PORTID (0xA0078002)
#define MI_ERR_ISP_ILLEGAL_PARAM (0xA0078003)
#define MI_ERR_ISP_EXIST (0xA0078004)
#define MI_ERR_ISP_UNEXIST (0xA0078005)
#define MI_ERR_ISP_NULL_PTR (0xA0078006)
#define MI_ERR_ISP_NOT_SUPPORT (0xA0078008)
#define MI_ERR_ISP_NOT_PERM (0xA0078009)
#define MI_ERR_ISP_NOMEM (0xA007800C)
#define MI_ERR_ISP_NOBUF (0xA007800D)
#define MI_ERR_ISP_BUF_EMPTY (0xA007800E)
#define MI_ERR_ISP_NOTREADY (0xA0078010)
#define MI_ERR_ISP_BUSY (0xA0078012)


typedef MI_U32 MI_ISP_DEV;
typedef MI_U32 MI_ISP_CHANNEL;
typedef MI_U32 MI_ISP_PORT;

typedef enum
{
    E_MI_ISP_DEVICEMASK_ID0  = 0x0001,
    E_MI_ISP_DEVICEMASK_ID1  = 0x0002,
    E_MI_ISP_DEVICEMASK_ID_MAX = 0xffff
} MI_ISP_DevMaskId_e;

typedef struct MI_ISP_DevAttr_s
{
   MI_U32    u32DevStitchMask; //multi ISP  dev bitmask by MI_ISP_DevMaskId_e
}MI_ISP_DevAttr_t;

typedef enum
{
    E_MI_ISP_HDR_TYPE_OFF,
    E_MI_ISP_HDR_TYPE_VC,
    E_MI_ISP_HDR_TYPE_DOL,
    E_MI_ISP_HDR_TYPE_EMBEDDED,
    E_MI_ISP_HDR_TYPE_LI,
    E_MI_ISP_HDR_TYPE_MAX
} MI_ISP_HDRType_e;

typedef enum
{
    E_MI_ISP_3DNR_LEVEL_OFF,
    E_MI_ISP_3DNR_LEVEL1,
    E_MI_ISP_3DNR_LEVEL2,
    E_MI_ISP_3DNR_LEVEL3,
    E_MI_ISP_3DNR_LEVEL4,
    E_MI_ISP_3DNR_LEVEL5,
    E_MI_ISP_3DNR_LEVEL6,
    E_MI_ISP_3DNR_LEVEL7,
    E_MI_ISP_3DNR_LEVEL_NUM
}MI_ISP_3DNR_Level_e;

typedef enum
{
    E_MI_ISP_SENSOR_INVALID = 0,
    E_MI_ISP_SENSOR0 = 0x1,
    E_MI_ISP_SENSOR1 = 0x2,
    E_MI_ISP_SENSOR2 = 0x4,
    E_MI_ISP_SENSOR3 = 0x8,
    E_MI_ISP_SENSOR4 = 0x10,
    E_MI_ISP_SENSOR5 = 0x20,
    E_MI_ISP_SENSOR6 = 0x40,
    E_MI_ISP_SENSOR7 = 0x80,
    E_MI_ISP_SENSOR_MAX = 8
}MI_ISP_BindSnrId_e;

typedef enum
{
   E_MI_ISP_SYNC3A_NONE = 0x00,
   E_MI_ISP_SYNC3A_AE   = 0x01,
   E_MI_ISP_SYNC3A_AWB  = 0x02,
   E_MI_ISP_SYNC3A_IQ   = 0x04
}MI_ISP_SYNC3A_e;

typedef struct MI_ISP_IQApiHeader_s
{
    MI_U32 u32HeadSize;    //Size of MIIspApiHeader_t
    MI_U32 u32DataLen;     //Data length;
    MI_U32 u32CtrlID;      //Function ID
    MI_U32 u32Channel;     //Isp channel number
    MI_U32 u32DevId;
    MI_S32 s32Ret;         //Isp api retuen value
} MI_ISP_IQApiHeader_t;

typedef struct MI_ISP_VersionPara_s
{
    MI_U32 u32Revision;
    MI_U32 u32Size;
    MI_U8  u8Data[64];
}MI_ISP_VersionPara_t;

typedef struct MI_ISP_CustIQParam_s
{
    MI_ISP_VersionPara_t  stVersion;
}MI_ISP_CustIQParam_t;

typedef struct MI_ISP_ChannelAttr_s
{
    MI_U32  u32SensorBindId; //bitmask by MI_ISP_BindSnrId_e
    MI_ISP_CustIQParam_t stIspCustIqParam;
    MI_U32  u32Sync3AType; //sync 3a bitmask by MI_ISP_SYNC3A_e
}MI_ISP_ChannelAttr_t;

typedef struct MI_ISP_ChnParam_s
{
   MI_ISP_HDRType_e      eHDRType;
   MI_ISP_3DNR_Level_e   e3DNRLevel;
   MI_BOOL               bMirror;
   MI_BOOL               bFlip;
   MI_SYS_Rotate_e       eRot;
} MI_ISP_ChnParam_t;

typedef struct MI_ISP_OutPortParam_s
{
    MI_SYS_WindowRect_t stCropRect;
    MI_SYS_PixelFormat_e  ePixelFormat;
}MI_ISP_OutPortParam_t;

typedef MI_S32 (*MI_ISP_CALLBK_FUNC)(MI_U64 u64Data);

typedef enum
{
    E_MI_ISP_CALLBACK_ISR,
    E_MI_ISP_CALLBACK_MAX,
} MI_ISP_CallBackMode_e;

typedef enum
{
    E_MI_ISP_IRQ_ISPVSYNC,
    E_MI_ISP_IRQ_ISPFRAMEDONE,
    E_MI_ISP_IRQ_MAX,
} MI_ISP_IrqType_e;

typedef struct MI_ISP_CallBackParam_s
{
    MI_ISP_CallBackMode_e eCallBackMode;
    MI_ISP_IrqType_e  eIrqType;
    MI_ISP_CALLBK_FUNC  pfnCallBackFunc;
    MI_U64    u64Data;
} MI_ISP_CallBackParam_t;

typedef struct MI_ISP_ZoomEntry_s
{
    MI_SYS_WindowRect_t stCropWin;
    MI_U8 u8ZoomSensorId;
} MI_ISP_ZoomEntry_t;

typedef struct MI_ISP_ZoomTable_s
{
    MI_U32 u32EntryNum;
    MI_ISP_ZoomEntry_t *pVirTableAddr;
} MI_ISP_ZoomTable_t;

typedef struct MI_ISP_ZoomAttr_s
{
    MI_U32  u32FromEntryIndex;
    MI_U32  u32ToEntryIndex;
    MI_U32  u32CurEntryIndex;
} MI_ISP_ZoomAttr_t;

#ifdef __cplusplus
}
#endif
#endif
