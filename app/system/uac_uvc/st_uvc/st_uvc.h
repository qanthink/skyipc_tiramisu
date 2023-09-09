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
#ifndef _ST_UVC_H_
#define _ST_UVC_H_

#if defined(__cplusplus) || defined(c_plusplus)
extern "C"{
#endif

#include "st_uvc_datatype.h"

extern int pthread_setname_np(pthread_t __target_thread,
        const char *__name);

int32_t ST_UVC_Init(char *uvc_name, ST_UVC_Handle_h*);
int32_t ST_UVC_Uninit(ST_UVC_Handle_h);
int32_t ST_UVC_CreateDev(ST_UVC_Handle_h, const ST_UVC_ChnAttr_t*);
int32_t ST_UVC_DestroyDev(ST_UVC_Handle_h);
int32_t ST_UVC_StartDev(ST_UVC_Handle_h);
int32_t ST_UVC_StopDev(ST_UVC_Handle_h);
int ST_UVC_SetTraceLevel(int level);
void save_file(void *buf,uint32_t length,char type);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif //_ST_UVC_H_
