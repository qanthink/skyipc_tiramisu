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
#ifndef _ST_UAC_DATATYPE_H_
#define _ST_UAC_DATATYPE_H_

#include <linux/usb/ch9.h>
#include <linux/videodev2.h>
#include <pthread.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include "pcm.h"
#include "mixer.h"

typedef enum
{
    UAC_DBG_NONE = 0,
    UAC_DBG_ERR,
    UAC_DBG_WRN,
    UAC_DBG_INFO,
    UAC_DBG_DEBUG,
    UAC_DBG_TRACE,
    UAC_DBG_ALL
} UAC_DBG_LEVEL_e;

#define ASCII_COLOR_RED                          "\033[1;31m"
#define ASCII_COLOR_WHITE                        "\033[1;37m"
#define ASCII_COLOR_YELLOW                       "\033[1;33m"
#define ASCII_COLOR_BLUE                         "\033[1;36m"
#define ASCII_COLOR_GREEN                        "\033[1;32m"
#define ASCII_COLOR_END                          "\033[0m"

extern UAC_DBG_LEVEL_e uac_debug_level;
extern bool  uac_func_trace;

#define UAC_SUPPORT_DEBUG
#ifdef UAC_SUPPORT_DEBUG
#define UAC_TRACE(fmt, args...) ({do{if(uac_debug_level>=UAC_DBG_TRACE)\
                {printf(ASCII_COLOR_GREEN"[APP TRACE]:%s[%d]: " fmt ASCII_COLOR_END"\n", __FUNCTION__,__LINE__,##args);}}while(0);})
#define UAC_DEBUG(fmt, args...) ({do{if(uac_debug_level>=UAC_DBG_DEBUG)\
                {printf(ASCII_COLOR_GREEN"[APP DEBUG]:%s[%d]: " fmt ASCII_COLOR_END"\n", __FUNCTION__,__LINE__,##args);}}while(0);})
#define UAC_INFO(fmt, args...)     ({do{if(uac_debug_level>=UAC_DBG_INFO)\
                {printf(ASCII_COLOR_GREEN"[APP INFO]:%s[%d]: " fmt ASCII_COLOR_END"\n", __FUNCTION__,__LINE__,##args);}}while(0);})
#define UAC_WRN(fmt, args...)      ({do{if(uac_debug_level>=UAC_DBG_WRN)\
                {printf(ASCII_COLOR_YELLOW"[APP WRN]: %s[%d]: " fmt ASCII_COLOR_END"\n", __FUNCTION__,__LINE__, ##args);}}while(0);})
#define UAC_ERR(fmt, args...)      ({do{if(uac_debug_level>=UAC_DBG_ERR)\
                {printf(ASCII_COLOR_RED"[APP ERR]: %s[%d]: " fmt ASCII_COLOR_END"\n", __FUNCTION__,__LINE__, ##args);}}while(0);})
#define UAC_EXIT_ERR(fmt, args...) ({do\
                {printf(ASCII_COLOR_RED"<<<%s[%d] " fmt ASCII_COLOR_END"\n",__FUNCTION__,__LINE__,##args);}while(0);})
#define UAC_ENTER()                ({do{if(UAC_func_trace)\
                {printf(ASCII_COLOR_BLUE">>>%s[%d] \n" ASCII_COLOR_END"\n",__FUNCTION__,__LINE__);}}while(0);})
#define UAC_EXIT_OK()              ({do{if(UAC_func_trace)\
                {printf(ASCII_COLOR_BLUE"<<<%s[%d] \n" ASCII_COLOR_END"\n",__FUNCTION__,__LINE__);}}while(0);})
#else
#define UAC_DEBUG(fmt, args...) NULL
#define UAC_ERR(fmt, args...)   NULL
#define UAC_INFO(fmt, args...)  NULL
#define UAC_WRN(fmt, args...)   NULL
#endif

#define AS_IN_MODE (1 << 0)
#define AS_OUT_MODE (1 << 1)

typedef enum
{
    E_AUDIO_BIT_WIDTH_16 =16,/* 16bit width */
    E_AUDIO_BIT_WIDTH_24 =24,/* 24bit width */
    E_AUDIO_BIT_WIDTH_MAX,
} ST_AUDIO_BitWidth_e;

typedef struct ST_UAC_Frame_s {
    ST_AUDIO_BitWidth_e bitwith;
    void *data;
    uint32_t length;
} ST_UAC_Frame_t;

typedef struct ST_UAC_Volume_s {
    int32_t  s32Min;
    int32_t  s32Max;
    int32_t  s32Volume;
} ST_UAC_Volume_t;

typedef struct ST_UAC_Config_s {
    uint8_t card;
    uint8_t device;
    uint32_t flags;
    ST_UAC_Volume_t volume;
    struct pcm_config pcm_config;
} ST_UAC_Config_t;

typedef struct ST_UAC_OPS_s {
    int32_t  (* UAC_AUDIO_Init)(uint32_t, uint8_t);
    int32_t  (* UAC_AUDIO_BufTask)(ST_UAC_Frame_t *);
    int32_t  (* UAC_AUDIO_Deinit)(void);
    int32_t  (* UAC_AUDIO_SetVol)(ST_UAC_Volume_t);
    int32_t  (* UAC_AUDIO_SetMute)(bool);
} ST_UAC_OPS_t;

typedef struct ST_UAC_Device_s {
    int32_t magic;
#define UAC_MOD_MAGIC 0xEFDA0000
#define UAC_MOD_MAGIC_MASK 0xFFFF0000
#define UAC_MKMAGIC(card, device) (UAC_MOD_MAGIC | ((card & 0xFF)<<8) | (device & 0xFF))
#define GET_MOD(nod) (nod & UAC_MOD_MAGIC_MASK)
#define GET_CRAD_NUM(magic) ((magic & 0xFF00)>>8)
#define GET_DEVICE_NUM(magic) (magic & 0xFF)
    ST_UAC_Config_t *config[2];
    struct pcm *pcm[2];

    /* UAC thread Releated */
    bool b_uacExit;
    int32_t mode;
    pthread_t control_tid;
    pthread_t capture_tid;
    pthread_t playback_tid;
    pthread_mutex_t mutex;

    ST_UAC_OPS_t opsAo;
    ST_UAC_OPS_t opsAi;
} ST_UAC_Device_t;

typedef enum {
    E_UAC_CONTROL_TYPE_PLAYBACK_SAMPLE_RATE,
    E_UAC_CONTROL_TYPE_PLAYBACK_MUTE,
    E_UAC_CONTROL_TYPE_PLAYBACK_VOLUME,
    E_UAC_CONTROL_TYPE_MAX,
} ST_UAC_ControlType_e;

typedef void* ST_UAC_Handle_h;
#define ST_UAC_SUCCESS 0;
#endif//_ST_UAC_DATATYPE_H_
