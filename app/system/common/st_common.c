/* Copyright (c) 2018-2019 Sigmastar Technology Corp.
 All rights reserved.

  Unless otherwise stipulated in writing, any and all information contained
 herein regardless in any format shall remain the sole proprietary of
 Sigmastar Technology Corp. and be kept in strict confidence
 (��Sigmastar Confidential Information��) by the recipient.
 Any unauthorized act including without limitation unauthorized disclosure,
 copying, use, reproduction, sale, distribution, modification, disassembling,
 reverse engineering and compiling of the contents of Sigmastar Confidential
 Information is unlawful and strictly prohibited. Sigmastar hereby reserves the
 rights to any and all damages, losses, costs and expenses resulting therefrom.
*/
#include <stdio.h>

#include "st_common.h"

#define MAX_LIST_LENGTH_BITS 20

MI_S32 ST_Sys_Init(void)
{
    MI_SYS_Version_t stVersion;
    MI_U64 u64Pts = 0;

    STCHECKRESULT(MI_SYS_Init(0));

    memset(&stVersion, 0x0, sizeof(MI_SYS_Version_t));
    STCHECKRESULT(MI_SYS_GetVersion(0, &stVersion));
    ST_INFO("u8Version:%s\n", stVersion.u8Version);

    STCHECKRESULT(MI_SYS_GetCurPts(0, &u64Pts));
    ST_INFO("u64Pts:0x%llx\n", u64Pts);

    u64Pts = 0xF1237890F1237890;
    STCHECKRESULT(MI_SYS_InitPtsBase(0, u64Pts));

    u64Pts = 0xE1237890E1237890;
    STCHECKRESULT(MI_SYS_SyncPts(0, u64Pts));

    return MI_SUCCESS;
}

MI_S32 ST_Sys_Exit(void)
{
    STCHECKRESULT(MI_SYS_Exit(0));

    return MI_SUCCESS;
}

MI_S32 ST_Sys_Bind(ST_Sys_BindInfo_T *pstBindInfo)
{
    /*
    ExecFunc(MI_SYS_BindChnPort(0, &pstBindInfo->stSrcChnPort, &pstBindInfo->stDstChnPort, \
        pstBindInfo->u32SrcFrmrate, pstBindInfo->u32DstFrmrate), MI_SUCCESS);
    */
    ExecFunc(MI_SYS_BindChnPort2(0, &pstBindInfo->stSrcChnPort, &pstBindInfo->stDstChnPort,
        pstBindInfo->u32SrcFrmrate, pstBindInfo->u32DstFrmrate, pstBindInfo->eBindType, pstBindInfo->u32BindParam),
        MI_SUCCESS);
    printf("\n");
    printf("src(%d-%d-%d-%d)  dst(%d-%d-%d-%d)  %d...\n", pstBindInfo->stSrcChnPort.eModId, pstBindInfo->stSrcChnPort.u32DevId,
        pstBindInfo->stSrcChnPort.u32ChnId, pstBindInfo->stSrcChnPort.u32PortId,
        pstBindInfo->stDstChnPort.eModId, pstBindInfo->stDstChnPort.u32DevId, pstBindInfo->stDstChnPort.u32ChnId,
        pstBindInfo->stDstChnPort.u32PortId, pstBindInfo->eBindType);
    printf("\n");
    return MI_SUCCESS;
}

MI_S32 ST_Sys_UnBind(ST_Sys_BindInfo_T *pstBindInfo)
{
    ExecFunc(MI_SYS_UnBindChnPort(0, &pstBindInfo->stSrcChnPort, &pstBindInfo->stDstChnPort), MI_SUCCESS);

    return MI_SUCCESS;
}

MI_U64 ST_Sys_GetPts(MI_U32 u32FrameRate)
{
    if (0 == u32FrameRate)
    {
        return (MI_U64)(-1);
    }

    return (MI_U64)(1000 / u32FrameRate);
}

MI_S32 ST_GetTimingInfo(ST_DispoutTiming_e u32ApTiming, MI_HDMI_TimingType_e *pu32HdmiTiming, MI_DISP_OutputTiming_e *pu32DispTiming, MI_U32 *pu32DispW, MI_U32 *pu32DispH)
{
    switch (u32ApTiming)
    {
        case E_ST_TIMING_720P_50:
            *pu32DispTiming = E_MI_DISP_OUTPUT_720P50;
            *pu32HdmiTiming = E_MI_HDMI_TIMING_720_50P;
            *pu32DispW = 1280;
            *pu32DispH = 720;
            break;
        case E_ST_TIMING_720P_60:
            *pu32DispTiming = E_MI_DISP_OUTPUT_720P60;
            *pu32HdmiTiming = E_MI_HDMI_TIMING_720_60P;
            *pu32DispW = 1280;
            *pu32DispH = 720;
            break;
        case E_ST_TIMING_1080P_50:
            *pu32DispTiming = E_MI_DISP_OUTPUT_1080P50;
            *pu32HdmiTiming = E_MI_HDMI_TIMING_1080_50P;
            *pu32DispW = 1920;
            *pu32DispH = 1080;
            break;
        case E_ST_TIMING_1080P_60:
            *pu32DispTiming = E_MI_DISP_OUTPUT_1080P60;
            *pu32HdmiTiming = E_MI_HDMI_TIMING_1080_60P;
            *pu32DispW = 1920;
            *pu32DispH = 1080;
            break;
        case E_ST_TIMING_1600x1200_60:
            *pu32DispTiming = E_MI_DISP_OUTPUT_1600x1200_60;
            *pu32HdmiTiming = E_MI_HDMI_TIMING_1600x1200_60P;
            *pu32DispW = 1600;
            *pu32DispH = 1200;
            break;
        case E_ST_TIMING_1440x900_60:
            *pu32DispTiming = E_MI_DISP_OUTPUT_1440x900_60;
            *pu32HdmiTiming = E_MI_HDMI_TIMING_1440x900_60P;
            *pu32DispW = 1440;
            *pu32DispH = 900;
            break;
        case E_ST_TIMING_1280x1024_60:
            *pu32DispTiming = E_MI_DISP_OUTPUT_1280x1024_60;
            *pu32HdmiTiming = E_MI_HDMI_TIMING_1280x1024_60P;
            *pu32DispW = 1280;
            *pu32DispH = 1024;
            break;
        case E_ST_TIMING_1024x768_60:
            *pu32DispTiming = E_MI_DISP_OUTPUT_1024x768_60;
            *pu32HdmiTiming = E_MI_HDMI_TIMING_1024x768_60P;
            *pu32DispW = 1024;
            *pu32DispH = 768;
            break;
        case E_ST_TIMING_1280x800_60:
            *pu32DispTiming = E_MI_DISP_OUTPUT_1280x800_60;
            *pu32HdmiTiming = E_MI_HDMI_TIMING_1280x800_60P;
            *pu32DispW = 1280;
            *pu32DispH = 800;
            break;
        case E_ST_TIMING_1366x768_60:
            *pu32DispTiming = E_MI_DISP_OUTPUT_1366x768_60;
            *pu32HdmiTiming = E_MI_HDMI_TIMING_1366x768_60P;
            *pu32DispW = 1366;
            *pu32DispH = 768;
            break;
        case E_ST_TIMING_1680x1050_60:
            *pu32DispTiming = E_MI_DISP_OUTPUT_1680x1050_60;
            *pu32HdmiTiming = E_MI_HDMI_TIMING_1680x1050_60P;
            *pu32DispW = 1680;
            *pu32DispH = 1050;
            break;
        case E_ST_TIMING_2560x1440_30:
            *pu32DispTiming = E_MI_DISP_OUTPUT_2560x1440_30;
            *pu32HdmiTiming = E_MI_HDMI_TIMING_1440_30P;
            *pu32DispW = 2560;
            *pu32DispH = 1440;
            break;
        case E_ST_TIMING_4k2k_30:
            *pu32DispTiming = E_MI_DISP_OUTPUT_3840x2160_30;
            *pu32HdmiTiming = E_MI_HDMI_TIMING_4K2K_30P;
            *pu32DispW = 3840;
            *pu32DispH = 2160;
            break;
        default:
            ST_WARN("Unspport Ap timing (%d)\n", u32ApTiming);
            return -1;
    }

    return MI_SUCCESS;
}

static struct list_head *merge(void *priv,
        int (*cmp)(void *priv, struct list_head *a,
                   struct list_head *b),
        struct list_head *a, struct list_head *b)
{
    struct list_head head, *tail = &head;

    while(a && b)
    {
        /* if equal, take 'a' -- important for sort stability */
        if((*cmp)(priv, a, b) <= 0)
        {
            tail->next = a;
            a = a->next;
        }
        else
        {
            tail->next = b;
            b = b->next;
        }
        tail = tail->next;
    }
    tail->next = a ? : b;
    return head.next;
}

static void merge_and_restore_back_links(void *priv,
        int (*cmp)(void *priv, struct list_head *a,
                   struct list_head *b),
        struct list_head *head,
        struct list_head *a, struct list_head *b)
{
    struct list_head *tail = head;
    MI_U8 count = 0;

    while(a && b)
    {
        /* if equal, take 'a' -- important for sort stability */
        if((*cmp)(priv, a, b) <= 0)
        {
            tail->next = a;
            a->prev = tail;
            a = a->next;
        }
        else
        {
            tail->next = b;
            b->prev = tail;
            b = b->next;
        }
        tail = tail->next;
    }
    tail->next = a ? : b;

    do
    {
        /*
         * In worst cases this loop may run many iterations.
         * Continue callbacks to the client even though no
         * element comparison is needed, so the client's cmp()
         * routine can invoke cond_resched() periodically.
         */
        if(!(++count))
            (*cmp)(priv, tail->next, tail->next);

        tail->next->prev = tail;
        tail = tail->next;
    }
    while(tail->next);

    tail->next = head;
    head->prev = tail;
}

void ST_List_Sort(void *priv, struct list_head *head,
                       int (*cmp)(void *priv, struct list_head *a,
                                  struct list_head *b))
{
    struct list_head *part[MAX_LIST_LENGTH_BITS + 1]; /* sorted partial lists
                        -- last slot is a sentinel */
    MI_U32 lev;  /* index into part[] */
    MI_U32 max_lev = 0;
    struct list_head *list;

    if(list_empty(head))
        return;

    memset(part, 0, sizeof(part));

    head->prev->next = NULL;
    list = head->next;

    while(list)
    {
        struct list_head *cur = list;
        list = list->next;
        cur->next = NULL;

        for(lev = 0; part[lev]; lev++)
        {
            cur = merge(priv, cmp, part[lev], cur);
            part[lev] = NULL;
        }
        if(lev > max_lev)
        {
            if(lev >= ARRAY_SIZE(part) - 1)
            {
                printf("list too long\n");
                lev--;
            }
            max_lev = lev;
        }
        part[lev] = cur;
    }

    for(lev = 0; lev < max_lev; lev++)
        if(part[lev])
            list = merge(priv, cmp, part[lev], list);

    merge_and_restore_back_links(priv, cmp, head, part[max_lev], list);
}

MI_U8 string_to_int8(MI_U8 *p)
{
    MI_U8 out = 0;

    if (*p <= '9')
    {
        out = *p - '0';
    }
    else
    {
        out = 10 + *p - 'a';
    }

    p++;
    out <<= 4;

    if (*p <= '9')
    {
        out += (*p - '0');
    }
    else
    {
        out += (10 + *p - 'a');
    }

    return out;
}