/* Copyright (c) 2018-2019 Sigmastar Technology Corp.
 All rights reserved.

  Unless otherwise stipulated in writing, any and all information contained
 herein regardless in any format shall remain the sole proprietary of
 Sigmastar Technology Corp. and be kept in strict confidence
 (ï¿½ï¿½Sigmastar Confidential Informationï¿½ï¿½) by the recipient.
 Any unauthorized act including without limitation unauthorized disclosure,
 copying, use, reproduction, sale, distribution, modification, disassembling,
 reverse engineering and compiling of the contents of Sigmastar Confidential
 Information is unlawful and strictly prohibited. Sigmastar hereby reserves the
 rights to any and all damages, losses, costs and expenses resulting therefrom.
*/
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <sys/mman.h>
#include <stdbool.h>
#include <assert.h>
#include <sys/time.h>
#include <mtd/mtd-user.h>

#include "st_uvc.h"
#include "st_uvc_xu.h"

#define ONLY_ONE_VIDEO  (1)
unsigned int g_device_count = 0;

bool uvc_func_trace = 1;
UVC_DBG_LEVEL_e uvc_debug_level = UVC_DBG_ERR;

#define TO_STR_NATIVE(e) #e
#define TO_STR_PROXY(m, e) m(e)
#define MACRO_TO_STRING(e) TO_STR_PROXY(TO_STR_NATIVE, e)
static char *version = MACRO_TO_STRING(DEMO_VERSION);

#define DOWN() \
    do \
    {\
        pthread_mutex_lock(&pdev->mutex);\
    } while(0)


#define UP() \
    do \
    {\
        pthread_mutex_unlock(&pdev->mutex);\
    } while(0)

static struct uvc_streaming_control
_UVC_Fill_Streaming_Control(ST_UVC_Device_t * pdev, struct uvc_streaming_control *ctrl,int32_t iframe, int32_t iformat);

static char *TraceLevel2Str(int level)
{
    switch (level){
    case UVC_DBG_ERR:
       return "error";
    case UVC_DBG_WRN:
       return "warn";
    case UVC_DBG_INFO:
       return "Info";
    case UVC_DBG_DEBUG:
       return "debug";
    case UVC_DBG_TRACE:
       return "trace";
    case UVC_DBG_ALL:
       return "all";
    default:
       return "none";
    }
    return "none";
}

int ST_UVC_SetTraceLevel(int level)
{
    if (level>= UVC_DBG_ERR && level<=UVC_DBG_ALL)
    {
        uvc_debug_level = level;
        printf("UVC Set Trace Level: %s\n", TraceLevel2Str(level));
        return 0;
    }
    return -EINVAL;
}


char* ST_UVC_GetCurrentSpeed(char* cmd, char* buff, int size)
{
    char temp[256];
    FILE* fp = NULL;
    int offset = 0;
    int len;

    fp = popen(cmd, "r");
    if(fp == NULL)
    {
        return NULL;
    }

    while(fgets(temp, sizeof(temp), fp) != NULL)
    {
        len = strlen(temp);
        if(offset + len < size)
        {
            strcpy(buff+offset, temp);
            offset += len;
        }
        else
        {
            buff[offset] = 0;
            break;
        }
    }

    if(fp != NULL)
    {
        pclose(fp);
    }

    return buff;
}

static inline uint32_t UVC_GetTime(void)
{
    struct timespec t1;
    clock_gettime(CLOCK_MONOTONIC, &t1);
    return 1000000 * (t1.tv_sec) + (t1.tv_nsec) / 1000;
}

static inline void trace_time(char *str)
{
    static uint32_t pre, cur;

    cur = UVC_GetTime();

    if (!pre)
        pre = cur;

    printf("[UVC Trace: %s] pre: %u ms, cur: %u ms, total: %u ms\n", str, pre, cur, cur - pre);

    pre = cur;
}
#if 0
static inline void _uvc_test(void)
{
    int32_t s32Ret;
    int32_t buftype = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    int32_t fd     = pdev->fd;
    struct v4l2_buffer ubuf;

    ubuf.type   = buftype;
    ubuf.memory = V4L2_MEMORY_MMAP;
    s32Ret = ioctl(fd, VIDIOC_DQBUF, &ubuf);
    if (s32Ret < 0)
        UVC_WRN(pdev,"VIDIOC_DQBUF Fail s32Ret: %d",s32Ret);
}
#endif
static int32_t _UVC_FillBuffer(ST_UVC_Device_t * pdev, struct buffer *mem, UVC_IO_MODE_e io_mode)
{
    ST_UVC_BufInfo_t BufInfo;
    ST_UVC_OPS_t fops = pdev->ChnAttr.fops;

    if(false==UVC_GET_STATUS(pdev->status, UVC_INTDEV_STREAMON))
        return -1;

    memset(&BufInfo,0x00,sizeof(ST_UVC_BufInfo_t));
    BufInfo.is_tail = true;

    if(true==UVC_GET_STATUS(pdev->status, UVC_INTDEV_STREAMON)){
        if(UVC_MEMORY_MMAP==io_mode){
            if(IS_NULL(fops.m.UVC_DevFillBuffer))
                return -1;
            BufInfo.b.buf = mem->mmap;
            /* Fill BUF,set param if successfully */
            if(0 <= fops.m.UVC_DevFillBuffer(pdev, &BufInfo)){
                mem->is_tail = BufInfo.is_tail;
                mem->buf.bytesused = BufInfo.length;
                if (BufInfo.is_keyframe) {
                    mem->buf.flags |= V4L2_BUF_FLAG_KEYFRAME;
                } else {
                    mem->buf.flags &= ~(V4L2_BUF_FLAG_KEYFRAME);
                }
            } else {
                usleep(1000);
                return -EBUSY;
            }
        } else
        if(UVC_MEMORY_USERPTR==io_mode){
            if(IS_NULL(fops.u.UVC_DevFillBuffer))
                return -1;
            /* Fill BUF,set param if successfully */
            if(0 <= (fops.u.UVC_DevFillBuffer(pdev, &BufInfo))){

                mem->is_tail = BufInfo.is_tail;
                mem->buf.bytesused = BufInfo.length;
                *mem->userptr = BufInfo.b.start;
                mem->handle = BufInfo.b.handle;
                if (BufInfo.is_keyframe) {
                    mem->buf.flags |= V4L2_BUF_FLAG_KEYFRAME;
                } else {
                    mem->buf.flags &= ~(V4L2_BUF_FLAG_KEYFRAME);
                }
            } else {
                usleep(1000);
                return -EBUSY;
            }
        }
    } else {
        UVC_WRN(pdev,"Input device is no ready");
        return -1;
    }

    /* check format */
    if(mem->buf.bytesused > mem->length){
        UVC_WRN(pdev,"bytesused[%d] is bigger than buf length[%d] ,Drop it", mem->buf.bytesused, mem->length);
        return -EBUSY;
    }
    return mem->buf.bytesused;
}

static inline int8_t _UVC_Get_User_Buf_Count(ST_UVC_Device_t * pdev, buffer_status_e status)
{
    if(NULL == pdev->mem)
        return 0;

    int8_t  i ,count = 0;
    ST_UVC_Setting_t set = pdev->ChnAttr.setting;

    for(i = 0; i < set.nbuf; i++) {
        if(pdev->mem[i].status == status) {
            count++;
        }
    }
    return count;
}

static int _UVC_FinishBuf(ST_UVC_Device_t * pdev, struct buffer *mem)
{
    int32_t s32Ret = ST_UVC_SUCCESS;
    ST_UVC_Setting_t set = pdev->ChnAttr.setting;
    ST_UVC_OPS_t fops = pdev->ChnAttr.fops;
    ST_UVC_BufInfo_t bufInfo;

    if (set.io_mode == UVC_MEMORY_USERPTR && mem->handle)
    {
        bufInfo.b.start = *mem->userptr;
        bufInfo.b.handle = mem->handle;
        s32Ret = fops.u.UVC_DevFinishBuffer(pdev, &bufInfo);
        if (s32Ret)
        {
            UVC_ERR(pdev, "Fail %d", s32Ret);
        }
        *mem->userptr = 0;
        mem->handle = 0;
    }
    mem->status = BUFFER_DEQUEUE;
    return s32Ret;
}

static int8_t _UVC_QBuf(ST_UVC_Device_t * pdev, struct buffer *mem)
{
    int32_t s32Ret;
    int32_t fd     = pdev->fd;
    ST_UVC_Setting_t set = pdev->ChnAttr.setting;

    assert(mem);

    if(mem->status != BUFFER_DEQUEUE)
         return -EINVAL;

    mem->status = BUFFER_FILLING;

    s32Ret = _UVC_FillBuffer(pdev, mem, set.io_mode);
    if( s32Ret <= 0 ) {
        goto fail;
    }

    /* if dev is already streamoff ,return -1 */
    if(false==UVC_GET_STATUS(pdev->status, UVC_INTDEV_STREAMON))
    {
        s32Ret = -EBUSY;
        goto fail;
    }

    if(mem->is_tail)
        mem->buf.reserved = 0;
    else
        mem->buf.reserved = 1;

    s32Ret = ioctl(fd, VIDIOC_QBUF, &(mem->buf));
    if (s32Ret < 0) {
        UVC_WRN(pdev,"Unable to queue buffer: %s (%d).",
                strerror(errno), errno);
        goto fail;
    }
    mem->status = BUFFER_QUEUE;
    UVC_TRACE(pdev,"Qbuf Success, buf index %d buflen %d",mem->buf.index, mem->buf.bytesused);
    return 0;

fail:
    _UVC_FinishBuf(pdev, mem);
    return s32Ret;
}

/* flags: 0 from v4l2 device , 1 from from user buf */
static struct buffer *_UVC_DQBuf(ST_UVC_Device_t * pdev, bool flags)
{
    int32_t s32Ret;
    ST_UVC_Setting_t set = pdev->ChnAttr.setting;
    struct buffer *mem = NULL;

    if(NULL == pdev || NULL == pdev->mem)
        return NULL;

    if (!flags) {
        int32_t buftype = V4L2_BUF_TYPE_VIDEO_OUTPUT;
        int32_t fd     = pdev->fd;
        struct v4l2_buffer ubuf;

        CLEAR(ubuf);
        ubuf.type   = buftype;
        if(UVC_MEMORY_MMAP == set.io_mode)
            ubuf.memory = V4L2_MEMORY_MMAP;
        else if(UVC_MEMORY_USERPTR == set.io_mode)
            ubuf.memory = V4L2_MEMORY_USERPTR;

        s32Ret = ioctl(fd, VIDIOC_DQBUF, &ubuf);
        if (s32Ret < 0) {
            UVC_WRN(pdev,"VIDIOC_DQBUF Fail s32Ret: %d ",s32Ret);
            return NULL;
        }
        UVC_TRACE(pdev,"dqbuf successfully index: %d", ubuf.index);
        mem = &(pdev->mem[ubuf.index]);
        mem->buf = ubuf;
        _UVC_FinishBuf(pdev, mem);
    } else {
        int32_t i;

        for(i = 0; i < set.nbuf; i++) {
            if(pdev->mem[i].status == BUFFER_DEQUEUE) {
                mem = &pdev->mem[i];
                return mem;
            }
        }
        return NULL;
    }
    return mem;
}

static int8_t _UVC_Video_QBuf(ST_UVC_Device_t * pdev)
{
    uint32_t i, s32Ret = ST_UVC_SUCCESS;
    ST_UVC_Setting_t set = pdev->ChnAttr.setting;
    struct buffer *mem = NULL;

    if(false==UVC_GET_STATUS(pdev->status, UVC_DEVICE_REQBUFS))
    {
        usleep(1000 * 30);
        return -1;
    }
    DOWN();
    for (i = 0; i < set.nbuf; ++i) {
        mem = _UVC_DQBuf(pdev, 1);
        if(NULL == mem)
        {
            s32Ret = -ENOMEM;
            continue;
        }

        s32Ret = _UVC_QBuf(pdev, mem);
    }
    UP();

    return s32Ret;
}

static int8_t _UVC_Video_ReqBufs(ST_UVC_Device_t * pdev, int32_t flags)
{
    struct v4l2_requestbuffers rb;
    uint32_t i;
    int32_t s32Ret;

    struct buffer *mem = NULL;
    ST_UVC_Setting_t set = pdev->ChnAttr.setting;
    int32_t fd = pdev->fd;
    int32_t buftype = V4L2_BUF_TYPE_VIDEO_OUTPUT;

    CLEAR(rb);

    if(1 == flags){
        if(false==UVC_GET_STATUS(pdev->status,UVC_DEVICE_REQBUFS)){
            rb.count = set.nbuf;
            UVC_SET_STATUS(pdev->status,UVC_DEVICE_REQBUFS);
        } else{
           UVC_ERR(pdev,"Release BUF First ");
           return -1;
        }
    }
    else if(0 == flags){
        if(true==UVC_GET_STATUS(pdev->status,UVC_DEVICE_REQBUFS)){
            rb.count = 0;
            UVC_UNSET_STATUS(pdev->status,UVC_DEVICE_REQBUFS);
        } else{
           UVC_WRN(pdev,"Already Release Buf");
           return ST_UVC_SUCCESS;
        }
    }
    else{
        UVC_WRN(pdev,"Invalid Param ");
        return -EINVAL;
    }

    rb.type   = buftype;
    if(UVC_MEMORY_MMAP == set.io_mode)
        rb.memory = V4L2_MEMORY_MMAP;
    else if(UVC_MEMORY_USERPTR == set.io_mode)
        rb.memory = V4L2_MEMORY_USERPTR;

    if (!rb.count)
    {
        mem = pdev->mem;
        for (i = 0; i < set.nbuf; ++i)
        {
            _UVC_FinishBuf(pdev, &mem[i]);

            if(UVC_MEMORY_MMAP == set.io_mode)
            {
                s32Ret= munmap(mem[i].mmap, mem[i].length);
                if (s32Ret< 0)
                {
                    UVC_ERR(pdev,"munmap failed");
                    goto err;
                }
            }
        }

        for (i = 0; i < set.nbuf; ++i)
        {
            s32Ret = ioctl(fd, VIDIOC_REQBUFS, &rb);
            if (s32Ret < 0)
            {
                if (s32Ret == -EINVAL)
                    UVC_ERR(pdev,"does not support memory mapping");
                else
                    UVC_ERR(pdev,"Unable to free buffers: %s (%d).",
                        strerror(errno), errno);
                goto err;
            }
        }
        free(mem);
        mem = NULL;
    }

    if (rb.count)
    {
        s32Ret = ioctl(fd, VIDIOC_REQBUFS, &rb);
        if (s32Ret < 0)
        {
            if (s32Ret == -EINVAL)
                UVC_ERR(pdev,"does not support memory mapping");
            else
                UVC_ERR(pdev,"Unable to alloc buffers: %s (%d).",
                    strerror(errno), errno);
            goto err;
        }

        if (rb.count < 2) {
            UVC_ERR(pdev,"Insufficient buffer memory.");
            s32Ret = -EINVAL;
            goto err;
        }

        mem = (struct buffer*)calloc(rb.count, sizeof(mem[0]));
        if (!mem) {
            UVC_ERR(pdev,"Out of memory");
            s32Ret = -ENOMEM;
            goto err;
        }

        for (i = 0; i < rb.count; ++i) {
            memset(&mem[i].buf, 0, sizeof(mem[i].buf));
            mem[i].buf.type   = buftype;
            mem[i].buf.index  = i;
            mem[i].handle = 0;
            mem[i].is_tail = 0;
            mem[i].status = BUFFER_DEQUEUE;

            /* USERPTR Mode alloc usr buffer */
            if(UVC_MEMORY_USERPTR == set.io_mode){
                mem[i].buf.memory = V4L2_MEMORY_USERPTR;
                s32Ret = ioctl(fd, VIDIOC_QUERYBUF, &(mem[i].buf));
                if (s32Ret < 0) {
                    UVC_ERR(pdev,"VIDIOC_QUERYBUF failed for buf %d:"
                       "%s (%d).", i, strerror(errno), errno);
                    s32Ret = -EINVAL;
                    goto err_free;
                }
                mem[i].userptr = &mem[i].buf.m.userptr;
                mem[i].length = mem[i].buf.length;
                continue;
            } else
            /* MMAP Map the buffers. */
            if(UVC_MEMORY_MMAP == set.io_mode)
            {
                mem[i].buf.memory = V4L2_MEMORY_MMAP;
                s32Ret = ioctl(fd, VIDIOC_QUERYBUF, &(mem[i].buf));
                if (s32Ret < 0) {
                    UVC_ERR(pdev,"VIDIOC_QUERYBUF failed for buf %d:"
                       "%s (%d).", i, strerror(errno), errno);
                    s32Ret = -EINVAL;
                    goto err_free;
                }

                mem[i].mmap = mmap(NULL /* start anywhere */,
                               mem[i].buf.length,
                               PROT_READ | PROT_WRITE /* required */,
                               MAP_SHARED /* recommended */,
                               fd, mem[i].buf.m.offset);

                if (MAP_FAILED == mem[i].mmap) {
                    UVC_ERR(pdev,"Unable to map buffer %u: %s (%d). ",
                             i,strerror(errno), errno);
                    mem[i].length = 0;
                    s32Ret = -EINVAL;
                    goto err_free;
                }

                mem[i].length = mem[i].buf.length;
                UVC_INFO(pdev,"UVC: Buffer %u mapped at address %p mem length %d. ",i,mem[i].mmap, mem[i].length);
            }
        }
    }

    pdev->mem = mem;
    if (mem)
    {
        // some platform need to queue buf before ISOC In (Streamon)
        UP();
        _UVC_Video_QBuf(pdev);
        DOWN();
    }
    printf("UVC: %u buffers %s. \n", set.nbuf, rb.count?"alloc":"free");

    s32Ret = ST_UVC_SUCCESS;
    goto done;

err_free:
    free(mem);
err:
    UVC_UNSET_STATUS(pdev->status,UVC_DEVICE_REQBUFS);
done:
    return s32Ret;
}

static int8_t _UVC_SetFormat(ST_UVC_Device_t * pdev, int32_t fd)
{
    struct v4l2_format vformat;
    int32_t buftype = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    int32_t s32Ret = ST_UVC_SUCCESS;

    /* Get  V4l2 Format Param */
    vformat.type = buftype;
    s32Ret = ioctl(fd,VIDIOC_G_FMT,&vformat);
    if(s32Ret <0 )
        UVC_WRN(pdev,"GET Format Failed ret= %d",s32Ret);
    /* then Set the v4l2 device Param */
    vformat.fmt.pix.pixelformat = pdev->stream_param.fcc;
    vformat.fmt.pix.width       = pdev->stream_param.width;
    vformat.fmt.pix.height      = pdev->stream_param.height;
    vformat.fmt.pix.sizeimage   = pdev->stream_param.maxframesize;
    s32Ret = ioctl(fd,VIDIOC_S_FMT,&vformat);
    if(s32Ret <0 ){
        UVC_ERR(pdev,"Set V4l2 Param Failed ret=%d ",s32Ret);
    }
    /* again to ensure format */
    s32Ret = ioctl(fd,VIDIOC_G_FMT,&vformat);
    if(s32Ret <0 )
        UVC_WRN(pdev,"GET Format Failed ret= %d",s32Ret);
    else {
        UVC_INFO(pdev,"mt->fmt.pix.width      =%u",vformat.fmt.pix.width);
        UVC_INFO(pdev,"mt->fmt.pix.height     =%u",vformat.fmt.pix.height);
        UVC_INFO(pdev,"mt->fmt.pix.sizeimage  =%u",vformat.fmt.pix.sizeimage);
    }
    return s32Ret;
}

static int8_t _UVC_Video_Stream_on_off(ST_UVC_Device_t * pdev, int32_t fd,int32_t enable)
{
    int32_t buftype = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    int32_t s32Ret = ST_UVC_SUCCESS;

    if (!enable) {
        if(false==UVC_GET_STATUS(pdev->status,UVC_DEVICE_STREAMON))
            goto done;
        else{
           UVC_UNSET_STATUS(pdev->status, UVC_DEVICE_STREAMON);
           s32Ret = ioctl(fd, VIDIOC_STREAMOFF, &buftype);
           if (s32Ret < 0){
               UVC_ERR(pdev,"VIDIOC_STREAMOFF failed: %s (%d).",
                    strerror(errno), errno);
               goto done;
           }
#ifndef UVC_USE_MAXSZ_BUF
           _UVC_Video_ReqBufs(pdev, 0);
#else
           for (int32_t i = 0; i < pdev->ChnAttr.setting.nbuf; ++i)
           {
               _UVC_FinishBuf(pdev, &mem[i]);
           }
#endif
        }
        printf("UVC: Stop video stream Successfully.\n");
    } else {
        if(true==UVC_GET_STATUS(pdev->status,UVC_DEVICE_STREAMON))
           goto done;
        else{
            _UVC_SetFormat(pdev, fd);
#ifndef UVC_USE_MAXSZ_BUF
            /* Request UVC buffers & mmap. */
            s32Ret = _UVC_Video_ReqBufs(pdev, 1);
            if (s32Ret < 0)
                goto done;
#endif
           /* And now Stream On the V4l2 Device */
           s32Ret = ioctl(fd, VIDIOC_STREAMON, &buftype);
           if (s32Ret < 0) {
               UVC_ERR(pdev,"Unable to start streaming %s (%d).",
                       strerror(errno), errno);
               goto done;
           }
           UVC_SET_STATUS(pdev->status,UVC_DEVICE_STREAMON);
        }
        printf("UVC: Start video stream Successfully.\n");
    }

done:
    return ST_UVC_SUCCESS;
}

static void _UVC_SYS_Exit(ST_UVC_Device_t * pdev)
{
    UVC_INFO(pdev,"");

    int32_t s32Ret = -1;
    ST_UVC_OPS_t fops = pdev->ChnAttr.fops;

    if(IS_NULL(fops.UVC_Inputdev_Deinit))
        return;

    if(true==UVC_GET_STATUS(pdev->status,UVC_INTDEV_INITIAL) )
    {
       s32Ret = fops.UVC_Inputdev_Deinit(pdev);
       if(0>s32Ret)
           return;
       UVC_UNSET_STATUS(pdev->status, UVC_INTDEV_INITIAL);
    }
}

static void _UVC_StopCapture(ST_UVC_Device_t * pdev)
{
    UVC_INFO(pdev,"");

    int32_t s32Ret = -1;
    ST_UVC_OPS_t fops = pdev->ChnAttr.fops;
    if(IS_NULL(fops.UVC_StopCapture))
        return;

/* Stream Off the V4l2 Device */
    _UVC_Video_Stream_on_off(pdev, pdev->fd, 0);

/* Stop Input streaming... */
    if(true==UVC_GET_STATUS(pdev->status, UVC_INTDEV_STREAMON))
    {
        UVC_UNSET_STATUS(pdev->status, UVC_INTDEV_STREAMON);
        s32Ret = fops.UVC_StopCapture(pdev);
        if(0 > s32Ret)
        {
            UVC_ERR(pdev,"Failed to Stop Stream, s32Ret: %d", s32Ret);
            return;
         }
     }
}

static int8_t _UVC_SYS_Init(ST_UVC_Device_t * pdev)
{
    int32_t s32Ret = -1;

    UVC_INFO(pdev,"%s\n", version);

    ST_UVC_OPS_t fops = pdev->ChnAttr.fops;
    if(IS_NULL(fops.UVC_Inputdev_Init))
        return -EINVAL;

    if(false==UVC_GET_STATUS(pdev->status,UVC_INTDEV_INITIAL))
    {
        s32Ret = fops.UVC_Inputdev_Init(pdev);
        if(s32Ret < 0)
            return -EINVAL;
        UVC_SET_STATUS(pdev->status,UVC_INTDEV_INITIAL);
    }
    return ST_UVC_SUCCESS;
}

/*
 * device start streaming -->
 *   request buf -->
 *       uvc streamon -->
 */
static int8_t _UVC_StartCapture(ST_UVC_Device_t * pdev)
{
    UVC_INFO(pdev,"");

    int32_t s32Ret = -1;
    ST_UVC_OPS_t fops = pdev->ChnAttr.fops;

    if(IS_NULL(fops.UVC_StartCapture))
        return -EINVAL;

/* Start Input Video capturing now. */
    _UVC_StopCapture(pdev);

    pdev->active_indicator = 3;
    s32Ret = fops.UVC_StartCapture(pdev, pdev->stream_param);
    if (s32Ret < 0)
    {
        UVC_ERR(pdev,"Failed to Start Stream, s32Ret: %d", s32Ret);
        goto err;
    }
    UVC_SET_STATUS(pdev->status, UVC_INTDEV_STREAMON);

/* Stream On V4l2 Device */
    _UVC_Video_Stream_on_off(pdev, pdev->fd, 1);

    return ST_UVC_SUCCESS;
err:
    return s32Ret;
}

static void _UVC_ForceIdr(ST_UVC_Device_t * pdev)
{
    ST_UVC_OPS_t fops = pdev->ChnAttr.fops;

    if(IS_NULL(fops.UVC_ForceIdr))
        return;

    fops.UVC_ForceIdr(pdev);
}

static int8_t _UVC_Events_Process_Control(ST_UVC_Device_t *pdev,uint8_t req,uint8_t cs,
                                         uint8_t entity_id,uint8_t len,
                                         struct uvc_request_data *resp)
{
    pdev->control.entity = entity_id;
    pdev->control.control = cs;
    pdev->control.length = len;

    switch (entity_id)
    {
    /* Error Code */
    case 0:
        switch(cs)
        {
        case UVC_VC_REQUEST_ERROR_CODE_CONTROL:
            /* Send the request error code last prepared. */
            resp->data[0] = pdev->request_error_code.data[0];
            resp->length  = pdev->request_error_code.length;
            break;

        default:
            /*
             * If we were not supposed to handle this
             * 'cs', prepare an error code response.
             */
            pdev->request_error_code.data[0] = 0x06;
            pdev->request_error_code.length = 1;
            break;
        }
        break;

    /* Camera terminal */
    case UVC_VC_INPUT_TERMINAL_ID:
        /*
         * We support only 'UVC_CT_SCANNING_MODE_CONTROL' for CAMERA
         * terminal, as our bmControls[0] = 1 for CT.
         */
        switch (cs)
        {
        case UVC_CT_SCANNING_MODE_CONTROL:
            switch (req)
            {
            case UVC_SET_CUR:
                resp->length = 1;
                pdev->request_error_code.data[0] = 0x00;
                pdev->request_error_code.length = 1;
                break;
            case UVC_GET_CUR:
                resp->data[0] = ct_scanning_mode_data;
                resp->length = 1;
                pdev->request_error_code.data[0] = 0x00;
                pdev->request_error_code.length = 1;
                break;
            case UVC_GET_INFO:
                resp->data[0] = 0x03;
                resp->length = 1;
                pdev->request_error_code.data[0] = 0x00;
                pdev->request_error_code.length = 1;
                break;

            /* not support GET_RES/GET_DEF/GET_MIN/GET_MAX/GET_LEN cmd */
            case UVC_GET_RES:
            case UVC_GET_DEF:
            case UVC_GET_MIN:
            case UVC_GET_MAX:
            case UVC_GET_LEN:
            default:
                /*
                 * We don't support this control, so STALL the
                 * default control ep.
                 */
                resp->length = -EL2HLT;
                /*
                 * For every unsupported control request
                 * set the request error code to appropriate
                 * code.
                 */
                pdev->request_error_code.data[0] = 0x07;
                pdev->request_error_code.length = 1;
                break;
            }
            break;

        default:
            /*
             * We don't support this control, so STALL the control
             * ep.
             */
            resp->length = -EL2HLT;
            /*
             * If we were not supposed to handle this
             * 'cs', prepare a Request Error Code response.
             */
            pdev->request_error_code.data[0] = 0x06;
            pdev->request_error_code.length = 1;
            break;
        }
        break;

    /* Processing unit */
    case UVC_VC_PROCESSING_UNIT_ID:
        /*
         * We support only 'UVC_PU_BRIGHTNESS_CONTROL' for Processing
         * Unit, as our bmControls[0] = 1 for PU.
         */
        switch (cs)
        {
        case UVC_PU_BRIGHTNESS_CONTROL:
            switch (req)
            {
            case UVC_SET_CUR:
                resp->length = 2;
                pdev->request_error_code.data[0] = 0x00;
                pdev->request_error_code.length = 1;
                break;
            case UVC_GET_CUR:
                resp->length = 2;
                resp->data[0] = pu_brightness_data[0];
                resp->data[1] = pu_brightness_data[1];
                pdev->request_error_code.data[0] = 0x00;
                pdev->request_error_code.length = 1;
                break;
            case UVC_GET_MIN:
                resp->length = 2;
                resp->data[0] = 0x00;
                resp->data[1] = 0x00;
                pdev->request_error_code.data[0] = 0x00;
                pdev->request_error_code.length = 1;
                break;
            case UVC_GET_MAX:
                resp->length = 2;
                resp->data[0] = 0xFF;
                resp->data[1] = 0x00;
                pdev->request_error_code.data[0] = 0x00;
                pdev->request_error_code.length = 1;
                break;
            case UVC_GET_RES:
                resp->length = 2;
                resp->data[0] = 0x01;
                resp->data[1] = 0x00;
                pdev->request_error_code.data[0] = 0x00;
                pdev->request_error_code.length = 1;
                break;
            case UVC_GET_INFO:
                resp->length = 1;
                resp->data[0] = 0x03;
                pdev->request_error_code.data[0] = 0x00;
                pdev->request_error_code.length = 1;
                break;
            case UVC_GET_DEF:
                resp->length = 2;
                pu_brightness_data[0] = resp->data[0] = 0x00;
                pu_brightness_data[1] = resp->data[1] = 0x00;
                pdev->request_error_code.data[0] = 0x00;
                pdev->request_error_code.length = 1;
                break;

            /* not support GET_LEN cmd */
            case UVC_GET_LEN:
            default:
                /*
                 * We don't support this control, so STALL the
                 * default control ep.
                 */
                resp->length = -EL2HLT;
                /*
                 * For every unsupported control request
                 * set the request error code to appropriate
                 * code.
                 */
                pdev->request_error_code.data[0] = 0x07;
                pdev->request_error_code.length = 1;
                break;
            }
            break;

        default:
            /*
             * We don't support this control, so STALL the control
             * ep.
             */
            resp->length = -EL2HLT;
            /*
             * If we were not supposed to handle this
             * 'cs', prepare a Request Error Code response.
             */
            pdev->request_error_code.data[0] = 0x06;
            pdev->request_error_code.length = 1;
            break;
        }
        break;

    /* Extension unit for isp */
    case UVC_VC_EXTENSION1_UNIT_ID:
        if (0 == usb_vc_eu1_cs(cs, req, resp))
        {
            resp->length = len;
            pdev->request_error_code.data[0] = 0x00;
            pdev->request_error_code.length = 1;
        }
        else
        {
            resp->length = -EL2HLT;
            pdev->request_error_code.data[0] = 0x06;
            pdev->request_error_code.length = 1;
        }
        break;

    /* Extension unit for customer */
    case UVC_VC_EXTENSION2_UNIT_ID:
        if (0 == usb_vc_eu2_cs(cs, req, resp))
        {
            resp->length = len;
            pdev->request_error_code.data[0] = 0x00;
            pdev->request_error_code.length = 1;
        }
        else
        {
            resp->length = -EL2HLT;
            pdev->request_error_code.data[0] = 0x06;
            pdev->request_error_code.length = 1;
        }
        break;

    default:
            /*
             * We don't support this control, so STALL the control
             * ep.
             */
            resp->length = -EL2HLT;
            /*
             * If we were not supposed to handle this
             * 'cs', prepare a Request Error Code response.
             */
            pdev->request_error_code.data[0] = 0x05;
            pdev->request_error_code.length = 1;
            break;
    }

    UVC_INFO(pdev,"control request (req %02x cs %02x) ]",req, cs);
    return ST_UVC_SUCCESS;
}


static int8_t _UVC_Events_Process_Streaming(ST_UVC_Device_t *pdev,uint8_t req, uint8_t cs,
                                            struct uvc_request_data *resp)
{
    struct uvc_streaming_control *ctrl;

    if (cs != UVC_VS_PROBE_CONTROL && cs != UVC_VS_COMMIT_CONTROL)
        return -EINVAL;

    ctrl = (struct uvc_streaming_control *)&resp->data;
    resp->length = sizeof *ctrl;

    switch (req) {
    case UVC_SET_CUR:
        pdev->control.control = cs;
        break;

    case UVC_GET_CUR:
        if (cs == UVC_VS_PROBE_CONTROL)
            memcpy(ctrl, &pdev->probe, sizeof *ctrl);
        else
            memcpy(ctrl, &pdev->commit, sizeof *ctrl);
        break;

    case UVC_GET_MIN:
    case UVC_GET_MAX:
    case UVC_GET_DEF:
        _UVC_Fill_Streaming_Control(pdev, ctrl, req == UVC_GET_MAX ? -1 : 0,
                       req == UVC_GET_MAX ? -1 : 0);
        break;

    case UVC_GET_RES:
        CLEAR(ctrl);
        break;

    case UVC_GET_LEN:
        resp->data[0] = 0x00;
        resp->data[1] = 0x22;
        resp->length = 2;
        break;

    case UVC_GET_INFO:
        resp->data[0] = 0x03;
        resp->length = 1;
        break;
    }
    return ST_UVC_SUCCESS;
}
/* ctrl request:
 * wValue: the wValue field contains the Control Selector (CS) in the high byte
 * wIndex: high byte for entity or zero,low byte for endpoint or interface
 *
 * */
static int8_t _UVC_Events_Process_Class(ST_UVC_Device_t *pdev,struct usb_ctrlrequest *ctrl,
                                        struct uvc_request_data *resp)
{
    ST_UVC_Setting_t setting = pdev->ChnAttr.setting;

    if ((ctrl->bRequestType & USB_RECIP_MASK) != USB_RECIP_INTERFACE)
        return -EINVAL;

    if (!setting.c_intf && !setting.s_intf)
    {
        pdev->control.ctype = (ctrl->wIndex & 0xff)%2;;
    } else
    {
        if ((ctrl->wIndex & 0xff) == setting.c_intf)
        {
           pdev->control.ctype = UVC_CONTROL_INTERFACE;
        } else
        {
           pdev->control.ctype = UVC_STREAMING_INTERFACE;
        }
    }

    UVC_SET_STATUS(pdev->status,UVC_DEVICE_ENUMURATED);
    switch (pdev->control.ctype) {
    case UVC_CONTROL_INTERFACE:
    UVC_INFO(pdev,"bRequest %x wValue %x wIndex %x wLength %x ",
                    ctrl->bRequest,ctrl->wValue >> 8,ctrl->wIndex >> 8,ctrl->wLength);
        _UVC_Events_Process_Control(pdev,
                        ctrl->bRequest,     // req
                        ctrl->wValue >> 8,  // CS
                        ctrl->wIndex >> 8,  // entity_id
                        ctrl->wLength,      // Length.
                        resp);              // resp.
        break;

    case UVC_STREAMING_INTERFACE:
    UVC_INFO(pdev,"bRequest %x wValue %x wIndex %x wLength %x ",
                    ctrl->bRequest,ctrl->wValue >> 8,ctrl->wIndex >> 8,ctrl->wLength);
        _UVC_Events_Process_Streaming(pdev,
                        ctrl->bRequest,
                        ctrl->wValue >> 8, resp);
        break;

    default:
        break;
    }
    return ST_UVC_SUCCESS;
}

static int8_t _UVC_Events_Process_Standard(ST_UVC_Device_t *pdev, struct usb_ctrlrequest *ctrl,
                                           struct uvc_request_data *resp)
{
    (void)ctrl;
    (void)resp;
    return ST_UVC_SUCCESS;
}

static int8_t _UVC_Events_Process_Setup(ST_UVC_Device_t * pdev, struct usb_ctrlrequest *ctrl,
                                        struct uvc_request_data *resp)
{
    UVC_INFO(pdev,"( bRequestType %02x bRequest %02x wValue %04x wIndex %04x wLength %04x )",
                  ctrl->bRequestType, ctrl->bRequest,ctrl->wValue, ctrl->wIndex, ctrl->wLength);

    switch (ctrl->bRequestType & USB_TYPE_MASK) {
    case USB_TYPE_STANDARD:
        _UVC_Events_Process_Standard(pdev, ctrl, resp);
        break;

    case USB_TYPE_CLASS:
        _UVC_Events_Process_Class(pdev, ctrl, resp);
        break;

    default:
        break;
    }
    return ST_UVC_SUCCESS;
}

// process PU, CT, XU job.
int8_t usb_vc_out_data(ST_UVC_Device_t *pdev, uint8_t entity, uint8_t cs, uint32_t len, struct uvc_request_data * data)
{
    switch (entity)
    {
    case UVC_VC_INPUT_TERMINAL_ID:
        usb_vc_ct_cs_out(pdev, entity, cs, len, data);
        break;
    case UVC_VC_PROCESSING_UNIT_ID:
        usb_vc_pu_cs_out(pdev, entity, cs, len, data);
        break;
    case UVC_VC_EXTENSION1_UNIT_ID:
        usb_vc_eu1_cs_out(pdev, entity, cs, len, data);
        break;
    case UVC_VC_EXTENSION2_UNIT_ID:
        usb_vc_eu2_cs_out(entity, cs, len, data);
        break;
    default:
        break;
    }

    return ST_UVC_SUCCESS;
}

static int8_t _UVC_Events_Process_Data(ST_UVC_Device_t * pdev, struct uvc_request_data *data)
{
    struct uvc_streaming_control *ctrl = NULL;

    if (UVC_STREAMING_INTERFACE == pdev->control.ctype)
    {
        switch (pdev->control.control) {
        case UVC_VS_PROBE_CONTROL:
            UVC_INFO(pdev," Probe control, length = %d ",data->length);
            ctrl = (struct uvc_streaming_control *)&data->data;
            pdev->probe = _UVC_Fill_Streaming_Control(pdev, ctrl,0,0);
            break;

        case UVC_VS_COMMIT_CONTROL:
            UVC_INFO(pdev," Commit control, length = %d ",data->length);
            ctrl = (struct uvc_streaming_control *)&data->data;
            pdev->commit = _UVC_Fill_Streaming_Control(pdev, ctrl,0,0);
            break;

        default:
            UVC_INFO(pdev," unknown control, length = %d ",data->length);
            break;
        }
    } else
    if(UVC_CONTROL_INTERFACE == pdev->control.ctype)
    {
        usb_vc_out_data(pdev, pdev->control.entity, pdev->control.control, pdev->control.length, data);
    } else /* Control Class Interface */
    {
        UVC_INFO(pdev," unknown control type: %d", pdev->control.ctype);
    }

    return ST_UVC_SUCCESS;
}

static int8_t _UVC_Events_Process(ST_UVC_Device_t * pdev)
{
    struct v4l2_event v4l2_event;
    struct uvc_event *uvc_event = (struct uvc_event *)&v4l2_event.u.data;
    struct uvc_request_data resp;
    int32_t fd = pdev->fd;
    int32_t s32Ret;

    s32Ret= ioctl(fd, VIDIOC_DQEVENT, &v4l2_event);
    if (s32Ret< 0) {
        UVC_ERR(pdev,"VIDIOC_DQEVENT failed: %s (%d)", strerror(errno),
            errno);
        return s32Ret;
    }

    memset(&resp, 0, sizeof resp);
    resp.length = -EL2HLT;

    switch (v4l2_event.type) {
    case UVC_EVENT_CONNECT:
        pdev->exit_request = 0;
        UVC_INFO(pdev,"Possible USB  requested from "
                        "Host, seen via UVC_EVENT_CONNECT");
        return ST_UVC_SUCCESS;

    case UVC_EVENT_DISCONNECT:
        pdev->exit_request = 1;
        UVC_INFO(pdev,"Possible USB shutdown requested from "
                "Host, seen via UVC_EVENT_DISCONNECT");
        return ST_UVC_SUCCESS;

    case UVC_EVENT_SETUP:
        _UVC_Events_Process_Setup(pdev, &uvc_event->req, &resp);
        break;

    case UVC_EVENT_DATA:
        s32Ret= _UVC_Events_Process_Data(pdev, &uvc_event->data);
        if (s32Ret< 0)
            break;
        return ST_UVC_SUCCESS;

    case UVC_EVENT_STREAMON:
        /* Only Isoc mode can be here */
        UVC_TRACE(pdev,"streamon");
        DOWN();
        _UVC_StartCapture(pdev);
        UP();
        return ST_UVC_SUCCESS;

    case UVC_EVENT_STREAMOFF:
        /* Stop Input streaming... */
        UVC_TRACE(pdev,"streamff");
        DOWN();
        _UVC_StopCapture(pdev);
        UP();
        return ST_UVC_SUCCESS;

    case UVC_EVENT_FORCEIDR:
        DOWN();
        _UVC_ForceIdr(pdev);
        UP();
        return ST_UVC_SUCCESS;
    }

    s32Ret= ioctl(fd, UVCIOC_SEND_RESPONSE, &resp);
    if (s32Ret< 0) {
        UVC_ERR(pdev,"UVCIOC_S_EVENT failed: %s (%d)", strerror(errno),errno);
        return s32Ret;
    }

    return ST_UVC_SUCCESS;
}

static int8_t _UVC_Video_Process(ST_UVC_Device_t * pdev)
{
    int32_t s32Ret = ST_UVC_SUCCESS;
    struct buffer *mem = NULL;

    /*
     * Return immediately if UVC video output device has not started
     * streaming yet.
     */

    if(false==UVC_INPUT_ISENABLE(pdev->status)  ||
       false==UVC_OUTPUT_ISENABLE(pdev->status))
    {
        usleep(1000 * 30);
        return -1;
    }

    DOWN();
    mem = _UVC_DQBuf(pdev, 0);
    if(NULL == mem)
    {
         s32Ret = -1;
    }

    if(ST_UVC_SUCCESS == s32Ret)
    {
         s32Ret = _UVC_QBuf(pdev, mem);
    }
    UP();
    return s32Ret;
}

/* when uvc_streaming_control is no NULL ,use c for ctrl setting
 *   iframe   : 0 - max
 *   ifrormat : 0 - max
 * */
static struct uvc_streaming_control
_UVC_Fill_Streaming_Control(ST_UVC_Device_t * pdev, struct uvc_streaming_control *c,int32_t iframe, int32_t iformat)
{
    int32_t nframes = 0;
    const uint32_t *interval = NULL;
    const struct uvc_format_info *format = NULL;
    const struct uvc_frame_info *frame = NULL;
    struct uvc_streaming_control ctrl;
    unsigned int MaxPayloadTransferSize = (pdev->ChnAttr.setting.maxpacket *
                                    (pdev->ChnAttr.setting.mult + 1) *
                                    (pdev->ChnAttr.setting.burst + 1));

    memset(&ctrl, 0, sizeof ctrl);
    /* Get Stream iFormat */
    if (NULL!=c) {
        iformat = clamp((unsigned int)c->bFormatIndex, 1U,
            (unsigned int)ARRAY_SIZE(uvc_formats)) - 1;
    }

    if (iformat < 0)
        iformat = ARRAY_SIZE(uvc_formats) + iformat;

    if (iformat < 0 || iformat >= (int)ARRAY_SIZE(uvc_formats)){
        UVC_ERR(pdev,"No Support Format");
        return ctrl;
    }
    format = &uvc_formats[iformat];

    /* Get Stream iFrame */
    nframes = 0;
    while (format->frames[nframes].width != 0)
        ++nframes;

    if (NULL!=c) {
        iframe = clamp((unsigned int)c->bFrameIndex, 1U, nframes) - 1;
    }

    if (iframe < 0)
        iframe = nframes + iframe;

    if (iframe < 0 || iframe >= (int)nframes){
        UVC_ERR(pdev,"No Support Format");
        return ctrl;
    }
    frame = &format->frames[iframe];

    /* Get Frame Interval */
    interval = frame->intervals;
    if (NULL!=c) {
        while (interval[0] < c->dwFrameInterval && interval[1])
            ++interval;
    } else {
        interval += 0;
    }

    /* Commit the Ctrl data */
    ctrl.bmHint = 1;
    ctrl.bFormatIndex = iformat + 1;
    ctrl.bFrameIndex = iframe + 1;
    ctrl.dwFrameInterval = *interval;



    /* get dwc3 current_speed: usb3.0:super-speed usb2.0:high-speed */
     char speedName[100]={0};
     bool bSuperSpeed = false;
     bool bDwc3 = false;

      if(!access("/sys/class/udc/1f344200.dwc3/current_speed", 0))
     {

        ST_UVC_GetCurrentSpeed("cat /sys/class/udc/1f344200.dwc3/current_speed", speedName, sizeof(speedName));
         if(0 == strncmp("super-speed", speedName, 11))
         {
            bSuperSpeed = true;
            printf("This is dwc3 usb3.0 current_speed=%s", speedName);
         }
         else if(0 == strncmp("high-speed", speedName, 10))
         {
            bSuperSpeed = false;
            printf("This is dwc3 usb2.0 current_speed=%s", speedName);
         }
         bDwc3 = true;
     }
     else if(!access("/sys/class/udc/soc\:Sstar-udc/current_speed", 0))
     {

        ST_UVC_GetCurrentSpeed("cat /sys/class/udc/soc\:Sstar-udc/current_speed", speedName, sizeof(speedName));
        if(0 == strncmp("high-speed", speedName, 10))
        {
            printf("This is udc-msb250x usb2.0 current_speed=%s", speedName);
        }
        bDwc3 = false;
    }

#ifndef UVC_USE_MAXSZ_BUF
    switch (format->fcc) {
    case V4L2_PIX_FMT_YUYV:
        ctrl.dwMaxVideoFrameSize = frame->width*frame->height * 2.0;
        break;
    case V4L2_PIX_FMT_NV12:
        ctrl.dwMaxVideoFrameSize = frame->width*frame->height * 1.5;
        break;
    case V4L2_PIX_FMT_MJPEG:
        ctrl.dwMaxVideoFrameSize = (frame->width*frame->height*2.0)/2;
        break;
    case V4L2_PIX_FMT_H264:
        ctrl.dwMaxVideoFrameSize = (frame->width*frame->height*2.0)/2;
        break;
    case V4L2_PIX_FMT_H265:
        ctrl.dwMaxVideoFrameSize = (frame->width*frame->height *2.0)/2;
        break;
    }
#else
    ctrl.dwMaxVideoFrameSize = (1920 * 1080 * 2.0);
#endif
    if (pdev->ChnAttr.setting.mode == USB_BULK_MODE)
        ctrl.dwMaxPayloadTransferSize = 16 * 1024;
    else
    {
       /*
        * ctrl.dwMaxPayloadTransferSize = (dev->maxpkt) *
        *                      (dev->mult + 1) * (dev->burst + 1);
        */
        if (c && !c->dwMaxPayloadTransferSize)
        {
            ctrl.dwMaxPayloadTransferSize = (c->dwMaxPayloadTransferSize <= MaxPayloadTransferSize)?
                                            c->dwMaxPayloadTransferSize:MaxPayloadTransferSize;
        } else {
            ctrl.dwMaxPayloadTransferSize = MaxPayloadTransferSize;
        }

         /* dwc3-usb2.0 && udc-msb250x-usb2.0*/
        if((bSuperSpeed != true) && (MaxPayloadTransferSize > 3072))
                MaxPayloadTransferSize=3072;

         /** default MaxPayloadTransferSize **/
         if((ctrl.dwMaxVideoFrameSize * FrameInterval2FrameRate(ctrl.dwFrameInterval) / 8000) <= MaxPayloadTransferSize)
             ctrl.dwMaxPayloadTransferSize = ctrl.dwMaxVideoFrameSize * FrameInterval2FrameRate(ctrl.dwFrameInterval) / 8000;
         else
         {
             ctrl.dwMaxPayloadTransferSize = MaxPayloadTransferSize;
             if((V4L2_PIX_FMT_YUYV == format->fcc) || (V4L2_PIX_FMT_NV12 == format->fcc))
                 UVC_WRN(pdev,"Payload is not enough, frameate can't reach %dfps", FrameInterval2FrameRate(ctrl.dwFrameInterval));
         }

        /** special values set MaxPayloadTransferSize **/
        if(bSuperSpeed != true && g_device_count != ONLY_ONE_VIDEO)
        {
              /* set different maxpaysize base on number of videos */
            ctrl.dwMaxPayloadTransferSize = ctrl.dwMaxPayloadTransferSize <= 2048 ? ctrl.dwMaxPayloadTransferSize:2048;
        }
        printf("dwMaxPayloadTransferSize=%d, Frame=%dfps\n", ctrl.dwMaxPayloadTransferSize, FrameInterval2FrameRate(ctrl.dwFrameInterval));
    }

    ctrl.bmFramingInfo = 3;
    ctrl.bPreferedVersion = 1;
    ctrl.bMaxVersion = 1;

    /* Finally Set the Strem Param For the Device */
    pdev->stream_param.fcc       = format->fcc ;
    pdev->stream_param.iformat   = iformat;
    pdev->stream_param.iframe    = iframe;
    pdev->stream_param.width     = frame->width;
    pdev->stream_param.height    = frame->height;
    pdev->stream_param.frameRate = FrameInterval2FrameRate(ctrl.dwFrameInterval);
    pdev->stream_param.maxframesize = ctrl.dwMaxVideoFrameSize;
    UVC_INFO(pdev," (iformat %d iframe %d width %d height %d FrameRate %f)",
                pdev->stream_param.iformat,pdev->stream_param.iframe,pdev->stream_param.width,
                pdev->stream_param.height,pdev->stream_param.frameRate);

    if( pdev->control.control == UVC_VS_COMMIT_CONTROL &&
        pdev->ChnAttr.setting.mode == USB_BULK_MODE )
    {
        DOWN();
        if( _UVC_StartCapture(pdev) < 0)
            UVC_ERR(pdev,"Some err Occur at _UVC_StartCapture");
        UP();
    }
    return ctrl;
}

static void * UVC_Event_Handle_Task(void *arg)
{
     int32_t s32Ret = -1;
     fd_set fdsu;
     ST_UVC_Device_t * pdev = (ST_UVC_Device_t *)arg;
     int32_t fd = pdev->fd;
     struct timeval timeout;

     while(UVC_DEVICE_ISREADY(pdev->status))
     {
         FD_ZERO(&fdsu);

         /* We want both setup and data events on UVC interface.. */
         FD_SET(fd, &fdsu);
         fd_set efds = fdsu;

         timeout.tv_sec  = 2;
         timeout.tv_usec = 0;
         s32Ret = select(fd + 1, NULL, NULL, &efds, &timeout);

         if (-1 == s32Ret) {
             UVC_ERR(pdev,"select error %d, %s",errno, strerror (errno));
              if (EINTR == errno)
                  return NULL;
         }

         if (0 == s32Ret){
             UVC_INFO(pdev,"select timeout, device status %x ", pdev->status);
             continue;
         }

         if (FD_ISSET(fd, &efds))
             _UVC_Events_Process(pdev);
    }
    return NULL;
}

static void * UVC_Video_Process_Task(void *arg)
{
    int32_t s32Ret = -1;
    fd_set fdsu;
    ST_UVC_Device_t * pdev = (ST_UVC_Device_t *)arg;
    int32_t fd = pdev->fd;
    struct timeval timeout;

    while(UVC_DEVICE_ISREADY(pdev->status))
    {
        /* queue buf from user */
        if(-EBUSY == _UVC_Video_QBuf(pdev))
        {
            usleep(1000);
            continue;
        }

        FD_ZERO(&fdsu);

        /* We want both setup and data events on UVC interface.. */
        FD_SET(fd, &fdsu);
        fd_set dfds = fdsu;

        timeout.tv_sec  = 2;
        timeout.tv_usec = 0;
        s32Ret = select(fd + 1, NULL,&dfds, NULL, &timeout);

        if (-1 == s32Ret)
        {
            UVC_ERR(pdev,"select error %d, %s",errno, strerror (errno));
            if (EINTR == errno)
                return NULL;
        }
        else if (0 == s32Ret)
        {
            DOWN();
            pdev->active_indicator--;
            if (pdev->ChnAttr.setting.mode == USB_BULK_MODE && pdev->active_indicator <= 0 && true == UVC_GET_STATUS(pdev->status, UVC_INTDEV_STREAMON))
            {
                ST_UVC_OPS_t fops = pdev->ChnAttr.fops;
                if(IS_NULL(fops.UVC_StopCapture))
                {
                    UP();
                    return NULL;
                }

                UVC_UNSET_STATUS(pdev->status, UVC_INTDEV_STREAMON);
                s32Ret = fops.UVC_StopCapture(pdev);
                if(0 > s32Ret)
                {
                     UVC_ERR(pdev,"Failed to Stop Stream, s32Ret: %d", s32Ret);
                     UP();
                     return NULL;
                }
                printf("UVC: Pause video stream Successfully.\n");
            }
            UP();
        }

        if (FD_ISSET(fd, &dfds))
        {
            DOWN();
            pdev->active_indicator = 3;
            if (pdev->ChnAttr.setting.mode == USB_BULK_MODE && false == UVC_GET_STATUS(pdev->status, UVC_INTDEV_STREAMON)
                && true == UVC_OUTPUT_ISENABLE(pdev->status))
            {
                ST_UVC_OPS_t fops = pdev->ChnAttr.fops;
                if(IS_NULL(fops.UVC_StartCapture))
                {
                    UP();
                    return NULL;
                }

                s32Ret = fops.UVC_StartCapture(pdev, pdev->stream_param);
                if(0 > s32Ret)
                {
                     UVC_ERR(pdev,"Failed to Start Stream, s32Ret: %d", s32Ret);
                     UP();
                     return NULL;
                }
                UVC_SET_STATUS(pdev->status, UVC_INTDEV_STREAMON);
                printf("UVC: Resume video stream Successfully.\n");
            }
            UP();

            _UVC_Video_Process(pdev);
        }
    }
    return NULL;
}

#define MTD_PROC_FILE       "/proc/mtd"
#define PROC_MTD_FIRST      "dev:    size   erasesize  name\n"
#define PROC_MTD_FIRST_LEN  (sizeof(PROC_MTD_FIRST) - 1)
#define PROC_MTD_MAX_LEN    4096
#define PROC_MTD_PATT       "mtd%d: %llx %x"
#define MTD_NAME_MAX        127

struct proc_parse_info
{
	int mtd_num;
	long long size;
	char name[MTD_NAME_MAX + 1];
	int eb_size;
	char *buf;
	int data_size;
	char *next;
};

static int proc_parse_start(struct proc_parse_info *pi)
{
	int fd, ret;

	fd = open(MTD_PROC_FILE, O_RDONLY);
	if (fd == -1)
		return -1;

	pi->buf = malloc(PROC_MTD_MAX_LEN);

	ret = read(fd, pi->buf, PROC_MTD_MAX_LEN);
	if (ret == -1) {
		printf("cannot read \"%s\"", MTD_PROC_FILE);
		goto out_free;
	}

	if (ret < PROC_MTD_FIRST_LEN ||
	    memcmp(pi->buf, PROC_MTD_FIRST, PROC_MTD_FIRST_LEN)) {
		printf("\"%s\" does not start with \"%s\"", MTD_PROC_FILE,
		       PROC_MTD_FIRST);
		goto out_free;
	}

	pi->data_size = ret;
	pi->next = pi->buf + PROC_MTD_FIRST_LEN;

	close(fd);
	return 0;

out_free:
	free(pi->buf);
	close(fd);
	return -1;
}

static int proc_parse_next(struct proc_parse_info *pi)
{
	int ret, len, pos = pi->next - pi->buf;
	char *p, *p1;

	if (pos >= pi->data_size) {
		free(pi->buf);
		return 0;
	}

	ret = sscanf(pi->next, PROC_MTD_PATT, &pi->mtd_num, &pi->size,
		     &pi->eb_size);
	if (ret != 3)
		return printf("\"%s\" pattern not found", PROC_MTD_PATT);

	p = memchr(pi->next, '\"', pi->data_size - pos);
	if (!p)
		return printf("opening \" not found");
	p += 1;
	pos = p - pi->buf;
	if (pos >= pi->data_size)
		return printf("opening \" not found");

	p1 = memchr(p, '\"', pi->data_size - pos);
	if (!p1)
		return printf("closing \" not found");
	pos = p1 - pi->buf;
	if (pos >= pi->data_size)
		return printf("closing \" not found");

	len = p1 - p;
	if (len > MTD_NAME_MAX)
		return printf("too long mtd%d device name", pi->mtd_num);

	memcpy(pi->name, p, len);
	pi->name[len] = '\0';

	if (p1[1] != '\n')
		return printf("opening \"\n\" not found");
	pi->next = p1 + 2;
	return 1;
}

static int UVC_Find_MTD(const char *name)
{
    int mtd_num = 0, ret;
    struct proc_parse_info pi;

    ret = proc_parse_start(&pi);
    if (ret)
        return -1;

    while (proc_parse_next(&pi)) {
        if (strcmp(pi.name, name) == 0)
            return mtd_num;
        mtd_num++;
    }
    // can't find a MTD with the matched name
    return -1;
}

static int UVC_MTD_Write(char *data, int size, int mtd_num)
{
    char mtd_name[15];
    int fd, i, ret;
    mtd_info_t mtd_info; // the MTD structure
    erase_info_t ei;

    if (mtd_num < 0) {
        printf("Invalid mtd_num %d\n", mtd_num);
        return -1;
    }
    sprintf(mtd_name, "/dev/mtd%d", mtd_num);

    printf("Open %s\n", mtd_name);
    fd = open(mtd_name, O_RDWR);
    if (!fd) {
        printf("Fail to open %s\n", mtd_name);
        return -1;
    }
    ioctl(fd, MEMGETINFO, &mtd_info);  // get the device info

    printf("Info %s: type %d, size %d, erase_size %d, wr_size %d\n", mtd_name,
            mtd_info.type, mtd_info.size, mtd_info.erasesize, mtd_info.writesize);
    ei.length = mtd_info.erasesize;   //set the erase block size
    printf("Erase %s\n", mtd_name);
    for(ei.start = 0; ei.start < mtd_info.size; ei.start += ei.length)
    {
        //ioctl(fd, MEMUNLOCK, &ei);
        ret = ioctl(fd, MEMERASE, &ei);
        if (ret) {
            printf("Erase failed at %d\n", ei.start);
        }
        //printf("%d\n", ei.start);
    }

    if(mtd_info.type == MTD_NORFLASH)
    {
        mtd_info.writesize = 512; //for spi-nor;
    }

    printf("Write %s size:%d mtd_info.writesize:%d\n", mtd_name, size, mtd_info.writesize);
    i = 0;
    while(i < size)
    {
        printf("\rWriting... %3d%%", (i*100)/size);

        lseek(fd, i, SEEK_SET); // go to the first block
        ret = write(fd, data + i, mtd_info.writesize);
        if (ret != mtd_info.writesize) {
            printf("write ret %d\n", ret);
        }

        i= i + mtd_info.writesize;

    }
    close(fd);
    printf("\rWriting... 100% complete.\n");
    printf("\nUpdate %s finish\n", mtd_name);
    return 0;
}

static void * UVC_Fw_Update_Task(void *arg)
{
    ST_UVC_Device_t *pdev = (ST_UVC_Device_t *)arg;

    char auPartitionTypeName[][64] =
    {
        //The value of this array is setted according to the real partition.
        {"IPL"},//0
        {"IPL_CUST0"},//1
        {"IPL_CUST1"},//2
        {"BOOT"},//3
        {"UBOOT1"},//4
        {"ENV0"},//5
        {"MISC"},//6
        {"nvrservice"},//7
        {"rootfs"},//8
        {"KERNEL"},//9
        {"RECOVERY"},//10
        {"customer"}//11
    };

    while(UVC_DEVICE_ISREADY(pdev->status))
    {
        sem_wait(&pdev->dfu_sem);
        if (pdev->dfu_status == UVC_DFU_STATE_DNL_END)
        {
            int fw_burning_size = 0;
            char *pbuf = pdev->fw_buf;

            while(fw_burning_size < pdev->fw_size)
            {
                int partition_num = 0;

                partition_num = pbuf[4] | (pbuf[5] << 8);

                printf("partition_num : %d\n", partition_num);

                int partition_type[4] = {0};

                int i = 0;
                for(i = 0; i < partition_num; ++i)
                {
                    partition_type[i] = pbuf[6 + 2*i] | (pbuf[7 + 2*i] << 8);

                    printf("partition_type[%d]:%d\n", i, partition_type[i]);
                }

                char *pFwData = pbuf + 32;

                int FwDataSize = pbuf[16] | (pbuf[17] << 8) | (pbuf[18] << 16) | (pbuf[19] <<24);

                printf("FwDataSize :0x%x\n", FwDataSize);

                char *pPartitionTypeName = NULL;

                for(i = 0; i < partition_num; ++i)
                {
                    pPartitionTypeName = auPartitionTypeName[partition_type[i]];
                    printf("pPartitionTypeName:%s\n", pPartitionTypeName);

                    UVC_MTD_Write(pFwData, FwDataSize, UVC_Find_MTD(pPartitionTypeName));
                }

                fw_burning_size += (32 + FwDataSize);
                printf("fw_size:0x%x fw_burning_size:0x%x\n", pdev->fw_size, fw_burning_size);

                pbuf = pdev->fw_buf + fw_burning_size;
            }

            pdev->dfu_status = UVC_DFU_STATE_IDLE;
            free(pdev->fw_buf);

        }
    }
    return NULL;
}

int32_t UVC_DFU_Init(ST_UVC_Device_t *pdev, size_t fw_size)
{
    if (pdev->dfu_status != UVC_DFU_STATE_IDLE) {
        printf("UVC DFU init failed with wrong state %d\r\n", pdev->dfu_status);
        return -1;
    }
    else {
        pdev->fw_size = 0;
        pdev->fw_size_cur = 0;
        pdev->fw_upd_cnt = 0;
        pdev->fw_buf = malloc(fw_size);
        if(!pdev->fw_buf) {
            printf("UVC DFU init failed at malloc %d\r\n", fw_size);
            return -1;
        }
        pdev->fw_size = fw_size;
        pdev->dfu_status = UVC_DFU_STATE_INIT;
    }
    return 0;
}

int32_t UVC_DFU_Update(ST_UVC_Device_t *pdev, u_int8_t *data, size_t size)
{
    //printf("DFU_Update state %d, pos: %d, size %d\r\n", pdev->dfu_status, pdev->fw_size_cur, size);
    if ((pdev->dfu_status != UVC_DFU_STATE_INIT) && (pdev->dfu_status != UVC_DFU_STATE_DNL)) {
        printf("UVC DFU update failed with wrong state %d\n", pdev->dfu_status);
        return -1;
    }
    else {
        pdev->dfu_status = UVC_DFU_STATE_DNL;
        if (size > (pdev->fw_size - pdev->fw_size_cur))
            size = pdev->fw_size - pdev->fw_size_cur;
        memcpy(pdev->fw_buf + pdev->fw_size_cur, data, size);
        pdev->fw_size_cur += size;
        if ((pdev->fw_upd_cnt & 0x1F) == 0) {
            printf("\rDownloading.... %3d%% complete.", (pdev->fw_size_cur*100)/pdev->fw_size);
        }
        pdev->fw_upd_cnt++;
    }
    return 0;
}

int32_t UVC_DFU_DnlEnd(ST_UVC_Device_t *pdev)
{
    if (pdev->dfu_status != UVC_DFU_STATE_DNL) {
        printf("UVC DFU DnlEnd failed with wrong state %d\n", pdev->dfu_status);
        return -1;
    }
    else {
        printf("\rDownloading.... 100% complete.\n");
        pdev->dfu_status = UVC_DFU_STATE_DNL_END;
        //wakeup thread to do ota
        sem_post(&pdev->dfu_sem);
    }
    return 0;
}

int32_t ST_UVC_Init(char *uvc_name, ST_UVC_Handle_h *phandle)
{
    int32_t s32Ret = -1;
    struct v4l2_event_subscription sub;
    struct v4l2_capability cap;
    ST_UVC_Device_t * pdev = NULL;

    UVC_INFO(pdev,"");

    pdev = *phandle;

/* Malloc A UVC DEVICE */
    if (pdev && (GET_MOD(pdev->magic) == UVC_MOD_MAGIC))
    {
        UVC_WRN(pdev,"UVC_Device already Init");
        return ST_UVC_SUCCESS;
    }

    pdev = (ST_UVC_Device_t *)malloc(sizeof(ST_UVC_Device_t));

/* Set default Streaming control */
    pdev->stream_param.iframe  = 1;
    pdev->stream_param.iformat = 1;
    pdev->stream_param.fcc = V4L2_PIX_FMT_MJPEG;

    pdev->control.control = UVC_VS_PROBE_CONTROL;

/* Init UVC Specific */
    CLEAR(pdev->status);

/* Start Init the UVC DEVICE */
   /* open the uvc device */
    if(IS_NULL(uvc_name))
    {
        UVC_ERR(pdev, "the dev:NULL can't be opened \n");
        goto err1;
    }

    //default videoxxx
    pdev->magic = UVC_MKMAGIC(atoi(&uvc_name[5]));

    /* It seems strange,but you have to double open it for reset signal */
    pdev->fd = open(uvc_name, O_RDWR | O_NONBLOCK);
  //  close(pdev->fd);
  //  pdev->fd = open(uvc_name, O_RDWR | O_NONBLOCK);

    if (pdev->fd < 0) {
        UVC_ERR(pdev, "device open failed: %s (%d).",
               strerror(errno), errno);
        goto err1;
    }
    strcpy(pdev->name, uvc_name);
   /* query uvc device */
    s32Ret = ioctl(pdev->fd, VIDIOC_QUERYCAP, &cap);
    if (s32Ret < 0) {
        UVC_ERR(pdev, "unable to query uvc device: %s (%d)",
                strerror(errno), errno);
        goto err2;
    }
   /* check the device type */
    if (!(cap.capabilities & V4L2_CAP_VIDEO_OUTPUT)) {
        UVC_ERR(pdev, "%s is no video output device", uvc_name);
        goto err2;
    }
    UVC_INFO(pdev, "device is %s on bus %s", cap.card, cap.bus_info);
    UVC_INFO(pdev, "open succeeded, file descriptor = %d", pdev->fd);

    pdev->exit_request = 0;
    pdev->fw_buf = 0;
    pdev->dfu_status = UVC_DFU_STATE_IDLE;

   /* Set default Function Operations */
    memset(&(pdev->ChnAttr.fops),0x00,sizeof(pdev->ChnAttr.fops));

   /* add the subscribe event to the uvc */
    memset(&sub, 0, sizeof sub);
    sub.type = UVC_EVENT_CONNECT;
    ioctl(pdev->fd, VIDIOC_SUBSCRIBE_EVENT, &sub);
    sub.type = UVC_EVENT_DISCONNECT;
    ioctl(pdev->fd, VIDIOC_SUBSCRIBE_EVENT, &sub);
    sub.type = UVC_EVENT_SETUP;
    ioctl(pdev->fd, VIDIOC_SUBSCRIBE_EVENT, &sub);
    sub.type = UVC_EVENT_DATA;
    ioctl(pdev->fd, VIDIOC_SUBSCRIBE_EVENT, &sub);
    sub.type = UVC_EVENT_STREAMON;
    ioctl(pdev->fd, VIDIOC_SUBSCRIBE_EVENT, &sub);
    sub.type = UVC_EVENT_STREAMOFF;
    ioctl(pdev->fd, VIDIOC_SUBSCRIBE_EVENT, &sub);
    sub.type = UVC_EVENT_FORCEIDR;
    ioctl(pdev->fd, VIDIOC_SUBSCRIBE_EVENT, &sub);

    UVC_SET_STATUS(pdev->status,UVC_DEVICE_INITIAL);

    *phandle = pdev;

    g_device_count++; //for set different maxpaysize
    return ST_UVC_SUCCESS;

err2:
   close(pdev->fd);
err1:
   free(pdev);
   return -1;
}

int32_t ST_UVC_Uninit(ST_UVC_Handle_h handle)
{
    ST_UVC_Device_t * pdev = handle;

    UVC_INFO(pdev,"");

    if (pdev && (GET_MOD(pdev->magic) == UVC_MOD_MAGIC))
    {
        if(true==UVC_GET_STATUS(pdev->status,UVC_DEVICE_INITIAL))
        {
            close(pdev->fd);
            free(pdev);
            pdev = NULL;
        }
    }

    return ST_UVC_SUCCESS;
}

int32_t ST_UVC_CreateDev(ST_UVC_Handle_h handle, const ST_UVC_ChnAttr_t* pstAttr)
{
    ST_UVC_Device_t * pdev = handle;

    UVC_INFO(pdev,"");

    if (!pdev || (GET_MOD(pdev->magic) != UVC_MOD_MAGIC))
    {
        UVC_ERR(pdev,"Please Init UVC Device First: %x\n", GET_MOD(pdev->magic));
        return -EINVAL;
    }

    ST_UVC_Setting_t setting = pstAttr->setting;
    ST_UVC_OPS_t fops = pstAttr->fops;

    if(IS_NULL(fops.UVC_Inputdev_Init)   ||
       IS_NULL(fops.UVC_Inputdev_Deinit) ||
       IS_NULL(fops.UVC_StartCapture)    ||
       IS_NULL(fops.UVC_StopCapture))
    {
        if(UVC_MEMORY_MMAP==setting.io_mode)

        UVC_ERR(pdev,"Err: Invalid Param ");
        return -EINVAL;
    }

    if(UVC_MEMORY_USERPTR==setting.io_mode &&
       (IS_NULL(fops.u.UVC_DevFillBuffer) ||
       (IS_NULL(fops.u.UVC_DevFillBuffer))))
    {
        UVC_ERR(pdev,"Err: Invalid Param ");
        return -EINVAL;
    }
    else if(UVC_MEMORY_MMAP==setting.io_mode &&
            IS_NULL(fops.m.UVC_DevFillBuffer)){
        UVC_ERR(pdev,"Err: Invalid Param ");
        return -EINVAL;
    }

    pdev->ChnAttr.fops = fops;
    pdev->ChnAttr.setting = setting;
    if (pdev->ChnAttr.setting.mult > 2)
    {
        UVC_WRN(pdev,"Invalid Mult %d Reset to 2", pdev->ChnAttr.setting.mult);
        pdev->ChnAttr.setting.mult = 2;
    }

    if (pdev->ChnAttr.setting.burst > 15)
    {
        UVC_WRN(pdev,"Invalid Burst %d Reset to 15", pdev->ChnAttr.setting.burst);
        pdev->ChnAttr.setting.burst = 15;
    }

    pdev->probe  = _UVC_Fill_Streaming_Control(pdev, NULL,
                    pdev->stream_param.iformat, pdev->stream_param.iframe);
    pdev->commit = _UVC_Fill_Streaming_Control(pdev, NULL,
                    pdev->stream_param.iformat, pdev->stream_param.iframe);
    UVC_INFO(pdev,"( probe iformat %d iframe %d )",
                    pdev->probe.bFormatIndex, pdev->probe.bFrameIndex);
    UVC_INFO(pdev,"( Probe dwFrameInterval %d frameRate %f )",
                    pdev->probe.dwFrameInterval, pdev->stream_param.frameRate);

    /* Init the Video Input SYSTEM */
    _UVC_SYS_Init(pdev);
    pthread_mutex_init(&pdev->mutex, NULL);
    sem_init(&pdev->dfu_sem, 0, 0);

#ifdef UVC_USE_MAXSZ_BUF
    DOWN();
    _UVC_SetFormat(pdev, pdev->fd);
    /* Request UVC buffers & mmap. */
    if(0 > _UVC_Video_ReqBufs(1))
    {
        UVC_ERR(pdev,"Err: Invalid Param ");
        UP();
        return -EINVAL;
    }
    UP();
#endif
    return ST_UVC_SUCCESS;
}

int32_t ST_UVC_DestroyDev(ST_UVC_Handle_h handle)
{
    ST_UVC_Device_t * pdev = handle;

    UVC_INFO(pdev,"");

    if (!pdev || (GET_MOD(pdev->magic) != UVC_MOD_MAGIC))
    {
        UVC_ERR(pdev,"Please Init UVC Device First");
        return -EINVAL;
    }
    DOWN();
#ifdef UVC_USE_MAXSZ_BUF
    /* Destroy UVC buffers & unmmap. */
    _UVC_Video_ReqBufs(pdev, 0);
#endif
    /* Stop Device First*/
    _UVC_StopCapture(pdev);
    UP();

    /* Uninit Device */
    _UVC_SYS_Exit(pdev);

    pthread_mutex_destroy(&pdev->mutex);
    return ST_UVC_SUCCESS;
};

int32_t ST_UVC_StartDev(ST_UVC_Handle_h handle)
{
    ST_UVC_Device_t * pdev = handle;

    UVC_INFO(pdev,"");

    if (!pdev || (GET_MOD(pdev->magic) != UVC_MOD_MAGIC))
    {
        UVC_ERR(pdev,"Please Init UVC Device First");
        return -EINVAL;
    }

    if(true==UVC_GET_STATUS(pdev->status,UVC_INTDEV_INITIAL) &&
       false==UVC_GET_STATUS(pdev->status,UVC_INTDEV_STARTED))
    {
        UVC_SET_STATUS(pdev->status,UVC_INTDEV_STARTED);

        pthread_create(&pdev->event_handle_thread, NULL, UVC_Event_Handle_Task, handle);
        pthread_setname_np(pdev->event_handle_thread , "UVC_Event_Handle_Task");

        pthread_create(&pdev->video_process_thread, NULL, UVC_Video_Process_Task, handle);
        pthread_setname_np(pdev->video_process_thread , "UVC_Video_Process_Task");

        pthread_create(&pdev->fw_update_thread, NULL, UVC_Fw_Update_Task, handle);
        pthread_setname_np(pdev->fw_update_thread , "UVC_Fw_Update_Task");
        return ST_UVC_SUCCESS;
    } else
        return -1;
};

int32_t ST_UVC_StopDev(ST_UVC_Handle_h handle)
{
    ST_UVC_Device_t * pdev = handle;

    UVC_INFO(pdev,"");

    if (!pdev || (GET_MOD(pdev->magic) != UVC_MOD_MAGIC))
    {
        UVC_ERR(pdev,"Please Init UVC Device First");
        return -EINVAL;
    }

    if(true==UVC_GET_STATUS(pdev->status,UVC_INTDEV_STARTED))
    {
        UVC_UNSET_STATUS(pdev->status,UVC_INTDEV_STARTED);
    }
    //wakeup fw_update_thread to exit
    sem_post(&pdev->dfu_sem);

    pthread_join(pdev->event_handle_thread, NULL);
    pthread_join(pdev->video_process_thread, NULL);
    pthread_join(pdev->fw_update_thread, NULL);
    return ST_UVC_SUCCESS;
};

void uvc_save_file(void *buf,uint32_t length,char type)
{
    if(NULL == buf || 0 >= length)
       return;

    FILE *UVCFile = NULL;
    char FileName[120];

    switch(type){
    case 0:
        snprintf(FileName, sizeof(FileName) - 1, "myusb.img");
        UVCFile = fopen(FileName, "w+");
       break;
    case 1:
        snprintf(FileName, sizeof(FileName) - 1, "myusb.es");
        UVCFile = fopen(FileName, "a+");
       break;
    default:
       break;
    }
    fwrite(buf,length,1, UVCFile);
    fclose(UVCFile);
}

void uvc_get_format(uint32_t fcc)
{
    switch(fcc)
    {
    case V4L2_PIX_FMT_YUYV:
        printf("Format is YUYV\n");
        break;
    case V4L2_PIX_FMT_NV12:
        printf("Format is NV12\n");
        break;
    case V4L2_PIX_FMT_MJPEG:
        printf("Format is MJPEG\n");
        break;
    case V4L2_PIX_FMT_H264:
        printf("Format is H264\n");
        break;
    case V4L2_PIX_FMT_H265:
        printf("Format is H265\n");
        break;
    default:
        printf("unkonown format\n");
        break;
    }
}
