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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "st_uac_datatype.h"

bool uac_func_trace = 1;
UAC_DBG_LEVEL_e uac_debug_level = UAC_DBG_ERR;

#define DEVICE_CHECK_INITED(pdev) \
    do{ \
       if (!pdev || (GET_MOD(pdev->magic) != UAC_MOD_MAGIC)){ \
           fprintf(stderr, "%s: magic: %x %x\n", __func__, GET_MOD(pdev->magic), UAC_MOD_MAGIC); \
           return -1; \
        }\
      } while(0)

static int32_t param_check(const ST_UAC_Config_t *uac_config)
{
    return ST_UAC_SUCCESS;
}

static char *TraceLevel2Str(int level)
{
    switch (level){
    case UAC_DBG_ERR:
       return "error";
    case UAC_DBG_WRN:
       return "warn";
    case UAC_DBG_INFO:
       return "Info";
    case UAC_DBG_DEBUG:
       return "debug";
    case UAC_DBG_TRACE:
       return "trace";
    case UAC_DBG_ALL:
       return "all";
    default:
       return "none";
    }
    return "none";
}

static void ST_UAC_Config(ST_UAC_Config_t *uac_config,
                        uint32_t flags,
                        uint32_t channels,
                        uint32_t rate,
                        uint32_t period_size,
                        uint32_t period_count,
                        enum pcm_format format,
                        ST_UAC_Volume_t volume)
{
    uac_config->card = 0;
    uac_config->device = 0;
    uac_config->flags = flags;
    uac_config->volume = volume;
    uac_config->pcm_config.channels= channels;
    uac_config->pcm_config.rate= rate;
    uac_config->pcm_config.period_size= period_size;
    uac_config->pcm_config.period_count= period_count;
    uac_config->pcm_config.format= format;

    if (flags == PCM_IN)
    {
        uac_config->pcm_config.start_threshold= 0;
        uac_config->pcm_config.stop_threshold= 0;
        uac_config->pcm_config.silence_threshold= 0;
    }
    else if (flags == PCM_OUT)
    {
        uac_config->pcm_config.start_threshold= period_count * period_size;
        uac_config->pcm_config.stop_threshold= period_count * period_size * 4;
        uac_config->pcm_config.silence_threshold= period_count * period_size;
    }
}

static uint32_t ST_UAC_GetPcm_BufSize(ST_UAC_Handle_h handle, int mode)
{
    ST_UAC_Device_t * pdev = handle;

    DEVICE_CHECK_INITED(pdev);

    if (mode == AS_OUT_MODE)
        return pcm_get_buffer_size(pdev->pcm[1]);
    else if(mode == AS_IN_MODE)
        return pcm_get_buffer_size(pdev->pcm[0]);
    else
        return 0;
}

int32_t ST_UAC_AllocStream(ST_UAC_Handle_h * phandle)
{
    ST_UAC_Device_t * pdev = NULL;

    pdev = *phandle;

    if (pdev && (GET_MOD(pdev->magic) == UAC_MOD_MAGIC))
    {
        UAC_ERR("already alloc: %s %d.\n", __FUNCTION__, __LINE__);
        return ST_UAC_SUCCESS;
    }

    pdev = (ST_UAC_Device_t *)calloc(1, sizeof(ST_UAC_Device_t));
    if (!pdev)
    {
        UAC_ERR("no memory: %s %d.\n", __FUNCTION__, __LINE__);
        return -1;
    }

    pdev->config[0] = (ST_UAC_Config_t*)malloc(sizeof(ST_UAC_Config_t));
    if (!pdev->config[0])
    {
        free(pdev);
        pdev = NULL;
        UAC_ERR("no memory: %s %d.\n", __FUNCTION__, __LINE__);
        return -1;
    }

    pdev->config[1] = (ST_UAC_Config_t*)malloc(sizeof(ST_UAC_Config_t));
    if (!pdev->config[1])
    {
        free(pdev->config[0]);
        pdev->config[0] = NULL;
        free(pdev);
        pdev = NULL;
        UAC_ERR("no memory: %s %d.\n", __FUNCTION__, __LINE__);
        return -1;
    }

    pdev->magic = UAC_MKMAGIC(pdev->config[0]->card, pdev->config[0]->device);
    *phandle = pdev;

    return ST_UAC_SUCCESS;
}

int32_t ST_UAC_FreeStream(ST_UAC_Handle_h handle)
{
    ST_UAC_Device_t * pdev = handle;

    DEVICE_CHECK_INITED(pdev);

    if (pdev->config[0])
    {
        free(pdev->config[0]);
        pdev->config[0] = NULL;
    }
    if (pdev->config[1])
    {
        free(pdev->config[1]);
        pdev->config[0] = NULL;
    }
    if (pdev)
    {
        free(pdev);
        pdev = NULL;
    }

    return ST_UAC_SUCCESS;
}

static int32_t ST_UAC_InitStream(ST_UAC_Handle_h handle,
                ST_UAC_Config_t *uac_config, int mode)
{
    ST_UAC_Device_t * pdev = handle;

    DEVICE_CHECK_INITED(pdev);

    if (param_check(uac_config))
        return -1;

    if (mode & AS_OUT_MODE)
    {
        memcpy(pdev->config[1], uac_config, sizeof(ST_UAC_Config_t));
        pdev->pcm[1] = pcm_open(pdev->config[1]->card,
                            pdev->config[1]->device,
                            pdev->config[1]->flags,
                            &uac_config->pcm_config);
        if (!pdev->pcm[1] || !pcm_is_ready(pdev->pcm[1]))
        {
            UAC_ERR("pcm_open failed: %s.\n", pcm_get_error(pdev->pcm[1]));
            return -1;
        }
    }
    else if (mode & AS_IN_MODE)
    {
        memcpy(pdev->config[0], uac_config, sizeof(ST_UAC_Config_t));
        pdev->pcm[0] = pcm_open(pdev->config[0]->card,
                            pdev->config[0]->device,
                            pdev->config[0]->flags,
                            &uac_config->pcm_config);
        if (!pdev->pcm[0] || !pcm_is_ready(pdev->pcm[0]))
        {
            UAC_ERR("pcm_open failed: %s.\n", pcm_get_error(pdev->pcm[0]));
            return -1;
        }
    }

    return ST_UAC_SUCCESS;
}

static int32_t ST_UAC_UninitStream(ST_UAC_Handle_h handle, int mode)
{
    ST_UAC_Device_t * pdev = handle;

    DEVICE_CHECK_INITED(pdev);

    if (mode & AS_OUT_MODE)
    {
        if (!pcm_is_ready(pdev->pcm[1]))
        {
            UAC_ERR("pcm is not ready: %s %d.\n", __FUNCTION__, __LINE__);
            return -1;
        }
        pcm_close(pdev->pcm[1]);
    }
    else if (mode & AS_IN_MODE)
    {
        if (!pcm_is_ready(pdev->pcm[0]))
        {
            UAC_ERR("pcm is not ready: %s %d.\n", __FUNCTION__, __LINE__);
            return -1;
        }
        pcm_close(pdev->pcm[0]);
    }

    return ST_UAC_SUCCESS;
}

static int32_t ST_UAC_SendStream(ST_UAC_Handle_h handle, const ST_UAC_Frame_t * pFrame)
{
    ST_UAC_Device_t * pdev = handle;

    DEVICE_CHECK_INITED(pdev);

    if (pcm_writei(pdev->pcm[0],
           pFrame->data,
           pcm_bytes_to_frames(pdev->pcm[0], pFrame->length)) < 0)
    {
        UAC_WRN("pcm_writei failed.\n");
        return -1;
    }

    return ST_UAC_SUCCESS;
}

static int32_t ST_UAC_GetStream(ST_UAC_Handle_h handle, ST_UAC_Frame_t * pFrame)
{
    ST_UAC_Device_t * pdev = handle;

    DEVICE_CHECK_INITED(pdev);

    int32_t ret = pcm_readi(pdev->pcm[1], pFrame->data,
            pcm_bytes_to_frames(pdev->pcm[1], pcm_get_buffer_size(pdev->pcm[1])));
    if (ret < 0)
    {
        UAC_WRN("pcm_readi failed.\n");
        return -1;
    }

    pFrame->length = pcm_frames_to_bytes(pdev->pcm[1], ret);

    return ST_UAC_SUCCESS;
}

static int32_t ST_UAC_InitCtl(ST_UAC_Handle_h * phandle)
{
    if(!phandle)
    {
        UAC_ERR("catch null pointer: %s %d.\n", __FUNCTION__, __LINE__);
        return -1;
    }

    struct mixer *mixer = mixer_open(0);
    if(!mixer)
    {
        UAC_ERR("mixer_open failed.\n");
        return -1;
    }

    if(mixer_subscribe_events(mixer, 1) != 0)
    {
        UAC_ERR("mixer_subscribe_events failed.\n");
        return -1;
    }

    *phandle = mixer;

    return ST_UAC_SUCCESS;
}

static int32_t ST_UAC_UninitCtl(ST_UAC_Handle_h handle)
{
    struct mixer *mixer = handle;

    if(!mixer)
    {
        UAC_ERR("catch null pointer: %s %d.\n", __FUNCTION__, __LINE__);
        return -1;
    }

    if(mixer_subscribe_events(mixer, 0) != 0)
    {
        UAC_ERR("mixer_subscribe_events failed.\n");
        return -1;
    }

    mixer_close(mixer);

    return ST_UAC_SUCCESS;
}

static int32_t ST_UAC_GetCtlVal(ST_UAC_Handle_h handle, uint32_t timeout, int32_t *index, int32_t *val)
{
    int32_t ret;
    struct mixer *mixer = handle;
    struct mixer_ctl *ctl;

    if(!mixer || !index || !val)
    {
        UAC_ERR("catch null pointer: %s %d.\n", __FUNCTION__, __LINE__);
        return -1;
    }

    ret = mixer_wait_event(mixer, timeout);
    if(ret == 0)
    {
        return ST_UAC_SUCCESS;
    }
    else if(ret == 1)
    {
        ret = mixer_ctl_get_event(mixer);
        if(ret < 0)
        {
            UAC_ERR("mixer_ctl_get_event failed.\n");
            return -1;
        }

        *index = ret;

        ctl = mixer_get_ctl(mixer, ret);
        if(!ctl)
        {
                UAC_ERR("mixer_get_ctl failed.\n");
            return -1;
        }

        ret = mixer_ctl_get_value(ctl, 0);
        if(ret == -EINVAL)
        {
                UAC_ERR("mixer_ctl_get_value failed.\n");
            return -1;
        }

        *val = ret;

        return ST_UAC_SUCCESS;
    }
    else
    {
        UAC_ERR("mixer_wait_event failed.\n");
        return -1;
    }
}

static int32_t ST_UAC_GetCtlRange(ST_UAC_Handle_h handle, int32_t index, int32_t *min, int32_t *max)
{
    int32_t ret;
    struct mixer *mixer = handle;
    struct mixer_ctl *ctl = NULL;

    if(!mixer || !min || !max)
    {
        UAC_ERR("catch null pointer: %s %d.\n", __FUNCTION__, __LINE__);
        return -1;
    }

    ctl = mixer_get_ctl(mixer, index);
    if(!ctl)
    {
        UAC_ERR("mixer_get_ctl failed.\n");
        return -1;
    }

    ret = mixer_ctl_get_range_min(ctl);
    if(ret == -EINVAL)
    {
        UAC_ERR("mixer_ctl_get_range_min failed.\n");
        return -1;
    }
    *min = ret;

    ret = mixer_ctl_get_range_max(ctl);
    if(ret == -EINVAL)
    {
        UAC_ERR("mixer_ctl_get_range_max failed.\n");
        return -1;
    }
    *max = ret;

    return ST_UAC_SUCCESS;
}

static void *UAC_Audio_Control_Task(void* phandle)
{
    int32_t ret = 0;
    ST_UAC_Device_t * pdev = phandle;
    ST_UAC_Handle_h handle;

    ret = ST_UAC_InitCtl(&handle);
    if(0 != ret)
    {
        UAC_ERR("ST_UAC_InitCtl failed.\n");
        goto end;
    }

    while(!pdev->b_uacExit)
    {
        int32_t s32Index = -1;
        int32_t s32Value;

        ret = ST_UAC_GetCtlVal(handle, 1000, &s32Index, &s32Value);
        if(0 == ret)
        {
            if(s32Index == E_UAC_CONTROL_TYPE_PLAYBACK_SAMPLE_RATE)
            {
                pthread_mutex_lock(&pdev->mutex);
                pdev->config[0]->pcm_config.rate = s32Value;
                pthread_mutex_unlock(&pdev->mutex);
            }
            else if(s32Index == E_UAC_CONTROL_TYPE_PLAYBACK_MUTE)
            {
                if(NULL == pdev->opsAi.UAC_AUDIO_SetMute)
                {
                    UAC_WRN("Not Support Mute Control\n");
                }
                else
                {
                    pdev->opsAi.UAC_AUDIO_SetMute(s32Value);
                }
            }
            else if(s32Index == E_UAC_CONTROL_TYPE_PLAYBACK_VOLUME)
            {
                int32_t s32Min, s32Max;

                ret = ST_UAC_GetCtlRange(handle, s32Index, &s32Min, &s32Max);
                if(0 != ret)
                {
                    UAC_ERR("ST_UAC_GetCtlRange failed.\n");
                    pdev->b_uacExit = true;
                    break;
                }

                pthread_mutex_lock(&pdev->mutex);
                pdev->config[0]->volume.s32Volume = s32Value;
                pdev->config[0]->volume.s32Min = s32Min;
                pdev->config[0]->volume.s32Max = s32Max;
                if(pdev->config[0]->volume.s32Volume == -EINVAL)
                {
                    UAC_WRN("Not Support Volume Control\n");
                }
                pthread_mutex_unlock(&pdev->mutex);
            }
        }
        else
        {
            UAC_ERR("ST_UAC_GetCtlVal failed.\n");
            pdev->b_uacExit = true;
            break;
        }
    }

    ret = ST_UAC_UninitCtl(handle);
    if(0 != ret)
    {
        UAC_ERR("ST_UAC_UninitCtl failed.\n");
        goto end;
    }

end:
    UAC_INFO("finish audio control thread.\n");
    pthread_exit(&ret);
}

static void *UAC_Audio_Playback_Task(void* handle)
{
    int32_t ret = 0;
    uint32_t u32CurSampleRate;
    bool bCreated = false;
    ST_UAC_Device_t *pdev =  handle;
    ST_UAC_Config_t uac_config;
    ST_UAC_Frame_t stUacFrame = {0};

    while(!pdev->b_uacExit)
    {
        pthread_mutex_lock(&pdev->mutex);
        u32CurSampleRate = pdev->config[0]->pcm_config.rate;
        pthread_mutex_unlock(&pdev->mutex);

        if(u32CurSampleRate)
            break;

       usleep(10*1000);
    }

    while(!pdev->b_uacExit)
    {
        pthread_mutex_lock(&pdev->mutex);
        if(u32CurSampleRate == pdev->config[0]->pcm_config.rate)
        {
            pthread_mutex_unlock(&pdev->mutex);

            if(u32CurSampleRate == 0)
            {
                usleep(10*1000);
                continue;
            }

            if(!bCreated)
            {
                ret = pdev->opsAi.UAC_AUDIO_Init(pdev->config[0]->pcm_config.rate, pdev->config[0]->pcm_config.channels);
                if(0 != ret)
                {
                    UAC_ERR("UAC_AUDIO_Init failed.\n");
                    goto end;
                }

                ST_UAC_Config(&uac_config, PCM_OUT, pdev->config[0]->pcm_config.channels, u32CurSampleRate,
                                     1024, 4, PCM_FORMAT_S16_LE, pdev->config[0]->volume);
                ret = ST_UAC_InitStream(handle, &uac_config, AS_IN_MODE);
                if(0 != ret)
                {
                    UAC_ERR("ST_UAC_InitStream failed.\n");
                    pdev->opsAi.UAC_AUDIO_Deinit();
                    goto end;
                }

                stUacFrame.length = ST_UAC_GetPcm_BufSize(handle, AS_IN_MODE);
                stUacFrame.data = malloc(stUacFrame.length);

                pthread_mutex_lock(&pdev->mutex);
                printf("Playback Stream On:(%d %d) Hz.\n", u32CurSampleRate, pdev->config[0]->pcm_config.rate);
                pthread_mutex_unlock(&pdev->mutex);

                bCreated = true;
            }

            pthread_mutex_lock(&pdev->mutex);
            if(pdev->config[0]->volume.s32Volume != -EINVAL)
            {
                ret = pdev->opsAi.UAC_AUDIO_SetVol(pdev->config[0]->volume);
                if(0 != ret)
                    UAC_WRN("MI_AI_SetVqeVolume failed: %d.\n", ret);

                pdev->config[0]->volume.s32Volume = -EINVAL;
            }
            pthread_mutex_unlock(&pdev->mutex);

            ret = pdev->opsAi.UAC_AUDIO_BufTask(&stUacFrame);
            if(0 == ret)
            {
                ret = ST_UAC_SendStream(handle, &stUacFrame);
                if(0 != ret)
                {
                    UAC_WRN("ST_UAC_SendStream failed:%s %d.\n", strerror(errno), errno);
                    usleep(1000);
                    continue;
                }
            }
            else
            {
                UAC_ERR("UAC_AUDIO_BufTask failed: %d.\n", ret);
                pdev->b_uacExit = true;
                break;
            }
        }
        else
        {
            pthread_mutex_unlock(&pdev->mutex);

            pthread_mutex_lock(&pdev->mutex);
            u32CurSampleRate = pdev->config[0]->pcm_config.rate;
            pthread_mutex_unlock(&pdev->mutex);

            if(bCreated)
            {
                ret = ST_UAC_UninitStream(handle, AS_IN_MODE);
                if(0 != ret)
                {
                    UAC_ERR("ST_UAC_UninitStream failed.\n");
                    pdev->opsAi.UAC_AUDIO_Deinit();
                    goto end;
                }

                if (stUacFrame.data)
                {
                    free(stUacFrame.data);
                    stUacFrame.data = NULL;
                }

                ret = pdev->opsAi.UAC_AUDIO_Deinit();
                if(0 != ret)
                {
                    UAC_ERR("UAC_AUDIO_Deinit failed.\n");
                    goto end;
                }

                pthread_mutex_lock(&pdev->mutex);
                printf("Playback Stream Off:(%d %d) Hz.\n", u32CurSampleRate, pdev->config[0]->pcm_config.rate);
                pthread_mutex_unlock(&pdev->mutex);

                bCreated = false;
            }

            if(u32CurSampleRate == 0)
            {
                usleep(10*1000);
            }
            else
            {
                if(!bCreated)
                {
                    ret = pdev->opsAi.UAC_AUDIO_Init(pdev->config[0]->pcm_config.rate, pdev->config[0]->pcm_config.channels);
                    if(0 != ret)
                    {
                        UAC_ERR("UAC_AUDIO_Init failed.\n");
                        goto end;
                    }

                    ST_UAC_Config(&uac_config, PCM_OUT, pdev->config[0]->pcm_config.channels, u32CurSampleRate,
                                     1024, 4, PCM_FORMAT_S16_LE, pdev->config[0]->volume);
                    ret = ST_UAC_InitStream(handle, &uac_config, AS_IN_MODE);
                    if(0 != ret)
                    {
                        UAC_ERR("ST_UAC_InitStream failed.\n");
                        pdev->opsAi.UAC_AUDIO_Deinit();
                        goto end;
                    }

                    stUacFrame.length = ST_UAC_GetPcm_BufSize(handle, AS_IN_MODE);
                    stUacFrame.data = malloc(stUacFrame.length);

                    pthread_mutex_lock(&pdev->mutex);
                    printf("Playback Stream On:(%d %d) Hz.\n", u32CurSampleRate, pdev->config[0]->pcm_config.rate);
                    pthread_mutex_unlock(&pdev->mutex);

                    bCreated = true;
                }
            }
        }
    }

    if(bCreated)
    {
        ret = ST_UAC_UninitStream(handle, AS_IN_MODE);
        if(0 != ret)
        {
            UAC_ERR("ST_UAC_UninitStream failed.\n");
        }

        if (stUacFrame.data)
        {
            free(stUacFrame.data);
            stUacFrame.data = NULL;
        }

        ret = pdev->opsAi.UAC_AUDIO_Deinit();
        if(0 != ret)
        {
            UAC_ERR("UAC_AUDIO_Deinit failed.\n");
        }

        pthread_mutex_lock(&pdev->mutex);
        printf("Playback Stream Off:(%d %d) Hz.\n", u32CurSampleRate, pdev->config[0]->pcm_config.rate);
        pthread_mutex_unlock(&pdev->mutex);

        bCreated = false;
    }

end:
    if (stUacFrame.data)
    {
        free(stUacFrame.data);
        stUacFrame.data = NULL;
    }
    UAC_INFO("finish audio playback thread.\n");
    pthread_exit(&ret);
}

static void *UAC_Audio_Capture_Task(void* handle)
{
    int32_t ret = 0;
    ST_UAC_Device_t * pdev = handle;
    ST_UAC_Config_t uac_config;

    ret = pdev->opsAo.UAC_AUDIO_Init(pdev->config[1]->pcm_config.rate, pdev->config[1]->pcm_config.channels);
    if(0 != ret)
    {
        UAC_ERR("UAC_AUDIO_Init failed.\n");
        goto end;
    }

    ST_UAC_Config(&uac_config, PCM_IN, pdev->config[1]->pcm_config.channels, pdev->config[1]->pcm_config.rate,
                     1024, 4, PCM_FORMAT_S16_LE, pdev->config[1]->volume);
    ret = ST_UAC_InitStream(handle, &uac_config, AS_OUT_MODE);
    if(0 != ret)
    {
        UAC_ERR("ST_UAC_InitStream failed!\n");
        pdev->opsAo.UAC_AUDIO_Deinit();
        goto end;
    }

    ST_UAC_Frame_t stUacFrame;
    stUacFrame.length = ST_UAC_GetPcm_BufSize(handle, AS_OUT_MODE);
    stUacFrame.data = malloc(stUacFrame.length);

    while(!pdev->b_uacExit)
    {
        memset(stUacFrame.data, 0, ST_UAC_GetPcm_BufSize(handle, AS_OUT_MODE));

        ret = ST_UAC_GetStream(handle, &stUacFrame);
        if(0 != ret)
        {
            UAC_WRN("ST_UAC_GetStream failed:%s %d.\n", strerror(errno), errno);
            usleep(1000);
            continue;
        }

        ret = pdev->opsAo.UAC_AUDIO_BufTask(&stUacFrame);
        if(0 != ret)
        {
            UAC_ERR("UAC_AUDIO_BufTask failed: %d.\n", ret);
            pdev->b_uacExit = true;
            break;
        }
    }

    ret = ST_UAC_UninitStream(handle, AS_OUT_MODE);
    if(0 != ret)
    {
        UAC_ERR("ST_UAC_UninitStream failed.\n");
    }

    ret = pdev->opsAo.UAC_AUDIO_Deinit();
    if(0 != ret)
    {
        UAC_ERR("ST_AO_Deinit failed.\n");
    }

end:
    if (stUacFrame.data)
    {
        free(stUacFrame.data);
        stUacFrame.data = NULL;
    }
    UAC_INFO("finish audio capture thread.\n");
    pthread_exit(&ret);
}

int32_t ST_UAC_StartDev(ST_UAC_Handle_h handle)
{
    int32_t ret = 0;
    ST_UAC_Device_t * pdev = handle;

    pdev->b_uacExit = false;

    if(pdev->mode & AS_OUT_MODE || pdev->mode & AS_IN_MODE)
    {
        if(pdev->mode & AS_OUT_MODE)
        {
            if (!pdev->opsAo.UAC_AUDIO_Init || !pdev->opsAo.UAC_AUDIO_Deinit || !pdev->opsAo.UAC_AUDIO_BufTask)
            {
                ret = -1;
                UAC_ERR("err ops\n");
                goto end;
            }
        }

        if(pdev->mode & AS_IN_MODE)
        {
            if (!pdev->opsAi.UAC_AUDIO_Init || !pdev->opsAi.UAC_AUDIO_Deinit
                || !pdev->opsAi.UAC_AUDIO_BufTask || !pdev->opsAi.UAC_AUDIO_SetVol)
            {
                ret = -1;
                UAC_ERR("err ops\n");
                goto end;
            }
        }

        pthread_mutex_init(&pdev->mutex, NULL);

        pthread_create(&pdev->control_tid, NULL, UAC_Audio_Control_Task, handle);
        UAC_INFO("create audio control thread.\n");

        if(pdev->mode & AS_OUT_MODE)
        {
            pthread_create(&pdev->capture_tid, NULL, UAC_Audio_Capture_Task, handle);
            UAC_INFO("create audio capture thread.\n");
        }

        if(pdev->mode & AS_IN_MODE)
        {
            pthread_create(&pdev->playback_tid, NULL, UAC_Audio_Playback_Task, handle);
            UAC_INFO("create audio playback thread.\n");
        }
    }
    else
    {
        ret = -1;
        UAC_ERR("err mode\n");
    }

end:
    return ret;
}

int32_t ST_UAC_StoptDev(ST_UAC_Handle_h handle)
{
    ST_UAC_Device_t * pdev = handle;
    pdev->b_uacExit = true;
    if(pdev->mode & AS_OUT_MODE || pdev->mode & AS_IN_MODE)
    {
        if(pdev->mode & AS_IN_MODE)
        {
            pthread_join(pdev->playback_tid, NULL);
        }

        if(pdev->mode & AS_OUT_MODE)
        {
            pthread_join(pdev->capture_tid, NULL);
        }

        pthread_join(pdev->control_tid, NULL);

        pthread_mutex_destroy(&pdev->mutex);
    }

    return 0;
}


int32_t ST_UAC_SetTraceLevel(int level)
{
    if (level>= UAC_DBG_ERR && level<=UAC_DBG_ALL)
    {
        uac_debug_level = level;
        printf("UAC Set Trace Level: %s\n", TraceLevel2Str(level));
        return 0;
    }
    return -EINVAL;
}

