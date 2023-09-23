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

#include "st_uvc.h"
#include "st_isp.h"
#include "st_uvc_xu.h"

// qanthink
#include "mi_common_datatype.h"
#include "mi_isp_3a_datatype.h"
#include "mi_isp_datatype.h"
#include "mi_isp_cus3a_api.h"
// qanthink end

extern UVC_DBG_LEVEL_e uvc_debug_level;
extern int32_t UVC_DFU_Init(ST_UVC_Device_t *pdev, size_t fw_size);
extern int32_t UVC_DFU_Update(ST_UVC_Device_t *pdev, u_int8_t *data, size_t size);
extern int32_t UVC_DFU_DnlEnd(ST_UVC_Device_t *pdev);

#if AIT_XU_DEF
FW_RELEASE_VERSION    gbFwVersion = { 0x01, 0x00, FW_VER_BCD }; // Add FW version.

uint8_t eu1_set_isp_val[EU1_SET_MMP_LEN];
uint8_t eu1_get_isp_result_val[EU1_GET_ISP_RESULT_LEN];
uint8_t eu1_set_mmp_val[EU1_SET_MMP_LEN];
uint8_t eu1_get_mmp_result_val[EU1_GET_MMP_RESULT_LEN];
uint8_t eu1_set_isp_ex_val[EU1_SET_ISP_EX_LEN];
uint8_t eu1_get_isp_ex_result_val[EU1_GET_ISP_EX_RESULT_LEN];
uint8_t eu1_get_chip_info_val[EU1_GET_CHIP_INFO_LEN];
uint8_t eu1_set_mmp_cmd16_val[EU1_SET_MMP_CMD16_LEN];
uint8_t eu1_get_mmp_cmd16_result_val[EU1_GET_MMP_CMD16_RESULT_LEN];
uint8_t eu1_get_data_32_val[EU1_GET_DATA_32_LEN];
uint8_t eu1_set_data_32_val[EU1_SET_DATA_32_LEN];

unsigned long WriteMemAddr = 0;
unsigned long WriteMemCount = 0;
unsigned long ReadMemAddr = 0;
unsigned long ReadMemCount = 0;

/*
 *  XU1 : AIT internal ISP / MMP control
 */
VC_CMD_CFG VC_XU_SET_ISP_CFG = {
    (CAP_SET_CUR_CMD | CAP_GET_INFO_CMD | CAP_GET_DEF_CMD | CAP_GET_MIN_CMD | CAP_GET_MAX_CMD | CAP_GET_RES_CMD | CAP_GET_LEN_CMD),
    (INFO_SET_SUPPORT),
    EU1_SET_ISP_LEN,1,
    0,0,0,0,0
};

VC_CMD_CFG VC_XU_GET_ISP_CFG = {
    (CAP_GET_INFO_CMD | CAP_GET_CUR_CMD | CAP_GET_DEF_CMD | CAP_GET_MIN_CMD | CAP_GET_MAX_CMD | CAP_GET_RES_CMD | CAP_GET_LEN_CMD),
    (INFO_GET_SUPPORT),
    EU1_GET_ISP_RESULT_LEN,1,
    0,0,0,0,0
};


VC_CMD_CFG VC_XU_SET_FW_DATA_CFG = {
    (CAP_SET_CUR_CMD | CAP_GET_INFO_CMD | CAP_GET_DEF_CMD | CAP_GET_MIN_CMD | CAP_GET_MAX_CMD | CAP_GET_RES_CMD | CAP_GET_LEN_CMD),
    (INFO_SET_SUPPORT),
    EU1_SET_FW_DATA_LEN,1,
    0,0,0,0,0
};


VC_CMD_CFG VC_XU_SET_MMP_CFG = {
    (CAP_SET_CUR_CMD | CAP_GET_INFO_CMD | CAP_GET_DEF_CMD | CAP_GET_MIN_CMD | CAP_GET_MAX_CMD | CAP_GET_RES_CMD | CAP_GET_LEN_CMD),
    (INFO_SET_SUPPORT),
    EU1_SET_MMP_LEN,1,
    0,0,0,0,0
};


VC_CMD_CFG VC_XU_GET_MMP_CFG = {
    ( CAP_GET_INFO_CMD | CAP_GET_CUR_CMD | CAP_GET_DEF_CMD | CAP_GET_MIN_CMD | CAP_GET_MAX_CMD | CAP_GET_RES_CMD | CAP_GET_LEN_CMD),
    (INFO_GET_SUPPORT),
    EU1_GET_MMP_RESULT_LEN,1,
    0,0,0,0,0
};

VC_CMD_CFG VC_XU_SET_ISP_EX_CFG = {
    (CAP_SET_CUR_CMD | CAP_GET_INFO_CMD | CAP_GET_DEF_CMD | CAP_GET_MIN_CMD | CAP_GET_MAX_CMD | CAP_GET_RES_CMD | CAP_GET_LEN_CMD),
    (INFO_SET_SUPPORT),
    EU1_SET_ISP_EX_LEN,1,
    0,0,0,0,0
};

VC_CMD_CFG VC_XU_GET_ISP_EX_CFG = {
    ( CAP_GET_INFO_CMD | CAP_GET_CUR_CMD | CAP_GET_DEF_CMD | CAP_GET_MIN_CMD | CAP_GET_MAX_CMD | CAP_GET_RES_CMD | CAP_GET_LEN_CMD),
    (INFO_GET_SUPPORT),
    EU1_GET_ISP_EX_RESULT_LEN,1,
    0,0,0,0,0
};

VC_CMD_CFG VC_XU_READ_MMP_MEM_CFG = {
    ( CAP_GET_INFO_CMD | CAP_GET_CUR_CMD | CAP_GET_DEF_CMD | CAP_GET_MIN_CMD | CAP_GET_MAX_CMD | CAP_GET_RES_CMD | CAP_GET_LEN_CMD),
    (INFO_GET_SUPPORT),
    EU1_READ_MMP_MEM_LEN,1,
    0,0,0,0,0
};

VC_CMD_CFG VC_XU_SET_MMP_MEM_CFG = {
    (CAP_SET_CUR_CMD | CAP_GET_INFO_CMD | CAP_GET_DEF_CMD | CAP_GET_MIN_CMD | CAP_GET_MAX_CMD | CAP_GET_RES_CMD | CAP_GET_LEN_CMD),
    (INFO_SET_SUPPORT),
    EU1_WRITE_MMP_MEM_LEN,1,
    0,0,0,0,0
};

VC_CMD_CFG VC_XU_ACCESS_CUSTOMER_DATA_CFG = {
    (CAP_SET_CUR_CMD | CAP_GET_INFO_CMD | CAP_GET_DEF_CMD | CAP_GET_MIN_CMD | CAP_GET_MAX_CMD | CAP_GET_RES_CMD | CAP_GET_LEN_CMD),
    (INFO_SET_SUPPORT),
    EU1_ACCESS_CUSTOMER_DATA_LEN,1,
    0,0,0,0,0
};



VC_CMD_CFG VC_XU_GET_CHIP_INFO_CFG = {
    ( CAP_GET_INFO_CMD | CAP_GET_CUR_CMD | CAP_GET_DEF_CMD | CAP_GET_MIN_CMD | CAP_GET_MAX_CMD | CAP_GET_RES_CMD | CAP_GET_LEN_CMD),
    (INFO_GET_SUPPORT),
    EU1_GET_CHIP_INFO_LEN,1,
    0,0,0,0,0
};

VC_CMD_CFG VC_XU_GET_DATA_32_CFG = {
    (CAP_GET_CUR_CMD | CAP_GET_INFO_CMD | CAP_GET_DEF_CMD | CAP_GET_MIN_CMD | CAP_GET_MAX_CMD | CAP_GET_RES_CMD | CAP_GET_LEN_CMD),
    (INFO_SET_SUPPORT),
    EU1_GET_DATA_32_LEN,1,
    0,0,0,0,0
};

VC_CMD_CFG VC_XU_SET_DATA_32_CFG = {
    (CAP_SET_CUR_CMD | CAP_GET_INFO_CMD | CAP_GET_DEF_CMD | CAP_GET_MIN_CMD | CAP_GET_MAX_CMD | CAP_GET_RES_CMD | CAP_GET_LEN_CMD),
    (INFO_SET_SUPPORT),
    EU1_SET_DATA_32_LEN,1,
    0,0,0,0,0
};

VC_CMD_CFG VC_XU_SET_MMP_CMD16_CFG = {
    (CAP_SET_CUR_CMD | CAP_GET_INFO_CMD | CAP_GET_DEF_CMD | CAP_GET_MIN_CMD | CAP_GET_MAX_CMD | CAP_GET_RES_CMD | CAP_GET_LEN_CMD),
    (INFO_SET_SUPPORT),
    EU1_SET_MMP_CMD16_LEN,1,
    0,0,0,0,0
};

DataExchangeParam gDEParam = { 0,0,0,0 };

VC_CMD_CFG VC_XU_GET_MMP_CMD16_RESULT_CFG = {
    (CAP_SET_CUR_CMD | CAP_GET_INFO_CMD | CAP_GET_CUR_CMD | CAP_GET_DEF_CMD | CAP_GET_MIN_CMD | CAP_GET_MAX_CMD | CAP_GET_RES_CMD | CAP_GET_LEN_CMD),
    (INFO_SET_SUPPORT | INFO_GET_SUPPORT),
    EU1_GET_MMP_CMD16_RESULT_LEN,1,
    0,0,0,0,0
};
#endif
#if 1 //cus EU
#define CUS_XU_ISP_LEN 4
#define CUS_XU_ISP_LEN_UT 60
uint8_t cus_eu_set_isp_config[CUS_XU_ISP_LEN];//bit 1 is for reset bframe
uint8_t cus_eu_get_isp_config[CUS_XU_ISP_LEN];//bit 1 is for reset bframe
uint8_t cus_eu_set_isp_config_ut[CUS_XU_ISP_LEN_UT];
uint8_t cus_eu_get_isp_config_ut[CUS_XU_ISP_LEN_UT];

VC_CMD_CFG CUS_XU_SET_ISP_CFG = {
    (CAP_SET_CUR_CMD | CAP_GET_INFO_CMD | CAP_GET_DEF_CMD | CAP_GET_MIN_CMD | CAP_GET_MAX_CMD | CAP_GET_RES_CMD | CAP_GET_LEN_CMD),
    (INFO_SET_SUPPORT),
    CUS_XU_ISP_LEN, 1,
    0,0,0,0,0
};

VC_CMD_CFG CUS_XU_GET_ISP_CFG = {
    (CAP_GET_INFO_CMD | CAP_GET_CUR_CMD | CAP_GET_DEF_CMD | CAP_GET_MIN_CMD | CAP_GET_MAX_CMD | CAP_GET_RES_CMD | CAP_GET_LEN_CMD),
    (INFO_GET_SUPPORT),
    CUS_XU_ISP_LEN, 1,
    0,0,0,0,0
};

VC_CMD_CFG CUS_XU_SET_ISP_CFG_UT = {
    (CAP_SET_CUR_CMD | CAP_GET_INFO_CMD | CAP_GET_DEF_CMD | CAP_GET_MIN_CMD | CAP_GET_MAX_CMD | CAP_GET_RES_CMD | CAP_GET_LEN_CMD),
    (INFO_SET_SUPPORT),
    CUS_XU_ISP_LEN_UT, 1,
    0,0,0,0,0
};

VC_CMD_CFG CUS_XU_GET_ISP_CFG_UT = {
    (CAP_GET_INFO_CMD | CAP_GET_CUR_CMD | CAP_GET_DEF_CMD | CAP_GET_MIN_CMD | CAP_GET_MAX_CMD | CAP_GET_RES_CMD | CAP_GET_LEN_CMD),
    (INFO_GET_SUPPORT),
    CUS_XU_ISP_LEN_UT, 1,
    0,0,0,0,0
};
#endif

// process xu isp command.
void usb_vc_eu1_isp_cmd(uint8_t *cmd)//cmd -> 8byte
{
    //cmd[2] = 0x01;
    //cmd[3] = 0x02;
    //cmd[4] = 0x03;
    //cmd[5] = 0x04;
    //cmd[6] = 0x05;
    //cmd[7] = 0x06;
    eu1_get_isp_result_val[0] = EU_ISP_CMD_OK;

    XU_Print("ISP cmd:%x %x %x %x %x %x %x %x\r\n", cmd[0], cmd[1], cmd[2], cmd[3], cmd[4], cmd[5], cmd[6], cmd[7]);
    switch (cmd[0])
    {
        case 11:    //get FW ver
        {
            eu1_get_isp_result_val[2] = (gbFwVersion.major >> 8) & 0xFF;
            eu1_get_isp_result_val[3] = gbFwVersion.major & 0xFF;
            eu1_get_isp_result_val[4] = (gbFwVersion.minor >> 8) & 0xFF;
            eu1_get_isp_result_val[5] = gbFwVersion.minor & 0xFF;
            eu1_get_isp_result_val[6] = (gbFwVersion.build >> 8) & 0xFF;
            eu1_get_isp_result_val[7] = gbFwVersion.build & 0xFF;
        }
        break;

        default:
            break;
    }
}

#define RAW_STORE_ONE_IMG      0x33

#define I2C_MODE_1A1D 1
#define I2C_MODE_2A1D 0

static int usb_vc_eu1_mmp_cmd(ST_UVC_Device_t *pdev, unsigned char* cmd)//cmd -> 8byte
{
    int ret = 0;
#if (ISP_IQ_SERVER)
    PCAM_IQSVR_HANDLE *iqsvr;
#else
    size_t fw_size = 0;
#endif

    XU_Print("[mmp_cmd]: [%x] [%x] [%x] [%x] [%x] [%x] [%x] [%x]\r\n",
        cmd[0], cmd[1], cmd[2], cmd[3], cmd[4], cmd[5], cmd[6], cmd[7]);

    //eu1_get_mmp_result_val[0] = EU_ISP_CMD_OK;

    switch (cmd[0])
    {
    case DOWNLOAD_MMP_FW_CMD:
#if (ISP_IQ_SERVER)
        iqsvr = ISP_usb_get_iqsvr_handle();
#endif
        //FW burning,burn the firmware in task instead of ISR
        switch (cmd[1])
        {
        case 0:
#if (ISP_IQ_SERVER)
            /* Tool is going to send command data */
            if (iqsvr->ubCmdHandling == false)
            {
                iqsvr->usCmdLen = (unsigned short) (cmd[4] | (cmd[5] << 8));

                XU_Print("[Mess] ISP SVR Len: %d\r\n", iqsvr->usCmdLen);
                iqsvr->usCmdBufCurPos = 0;  // reset buffer write ptr
                if (cmd[3] == 6) {
                    iqsvr->usCmdType = IQ_XU_CMD_SETAPI;
                }
                else if (cmd[3] == 7) {
                    iqsvr->usCmdType = IQ_XU_CMD_GETAPI;
                    iqsvr->usDataBufCurPos = 0;
                }
                iqsvr->ubCmdHandling = true;
                iqsvr->ubCmdWaitAck = false;
            }
            else {
                // Pend acking this request until the previous command is handled.
                iqsvr->ubCmdWaitAck = true;
                return -1;
            }
#else
            XU_Print("DL_FW_INIT\r\n");
            fw_size = cmd[4] | (cmd[5] << 8) | (cmd[6] << 16) | (cmd[7] << 24);
            if (!fw_size)
                return -1;
            UVC_DFU_Init(pdev, fw_size);
#endif
            break;
        case 1:
#if (ISP_IQ_SERVER)
            //Tool sent command data completely
            XU_Print("[Mess]all command data received\n");
#else
            XU_Print("DL_FW_FINISH\r\n");
            UVC_DFU_DnlEnd(pdev);
#endif
            break;
        case 0x10:
#if (ISP_IQ_SERVER)
            //Tool is going to get data back
            if (iqsvr->usCmdType == IQ_XU_CMD_GETAPI)
            {
                // Pend acking this request until command is handled by sensor task.
                if (iqsvr->ubCmdHandling) {
                    iqsvr->ubCmdHandling = false;
                    iqsvr->ubCmdWaitAck = true;
                    return -1;
                }
            }
#endif
            break;
        default:
            break;
        }
        break;
    default:
        eu1_get_mmp_result_val[0] = EU_ISP_CMD_NOT_SUPPORT;
        break;
    }
    return ret;
}

// To process XU data.
int8_t usb_vc_eu1_cs(uint8_t cs, uint8_t req, struct uvc_request_data *resp)
{
    switch (cs)
    {
        case EU1_SET_ISP:
        {
            VC_XU_SET_ISP_CFG.dwMaxVal = 0;//(unsigned long)maxVal ;
            VC_XU_SET_ISP_CFG.dwMinVal = 0;//(unsigned long)maxVal ;
            VC_XU_SET_ISP_CFG.dwResVal = 0;//(unsigned long)resVal ;
            XU_Print("EU1_SET_ISP\r\n");
            usb_vc_cmd_cfg(req, &VC_XU_SET_ISP_CFG, 0, resp);
            break;
        }
        case EU1_GET_ISP_RESULT:
        {
            VC_XU_SET_ISP_CFG.dwMaxVal = 0;//(unsigned long)maxVal ;
            VC_XU_SET_ISP_CFG.dwMinVal = 0;//(unsigned long)maxVal ;
            VC_XU_SET_ISP_CFG.dwResVal = 0;//(unsigned long)resVal ;
            XU_Print("EU1_GET_ISP\r\n");
            usb_vc_cmd_cfg(req, &VC_XU_GET_ISP_CFG, (unsigned long) eu1_get_isp_result_val, resp);
            break;
        }
        case EU1_SET_FW_DATA:
        {
            VC_XU_SET_ISP_CFG.dwMaxVal = 0;//(unsigned long)maxVal ;
            VC_XU_SET_ISP_CFG.dwMinVal = 0;//(unsigned long)maxVal ;
            VC_XU_SET_ISP_CFG.dwResVal = 0;//(unsigned long)resVal ;
            VC_XU_SET_ISP_CFG.dwCurVal = 0;//(unsigned long)resVal ;
            XU_Print("EU1_SET_FW_DATA\r\n");
            usb_vc_cmd_cfg(req, &VC_XU_SET_FW_DATA_CFG, 0, resp);
            break;
        }
        case EU1_SET_MMP:
        {
            VC_XU_SET_ISP_CFG.dwMaxVal = 0;//(unsigned long)maxVal ;
            VC_XU_SET_ISP_CFG.dwMinVal = 0;
            VC_XU_SET_ISP_CFG.dwResVal = 0;//(unsigned long)resVal ;

            //XU_Print("EU1_SET_MMP\r\n");
            usb_vc_cmd_cfg(req, &VC_XU_SET_MMP_CFG, 0, resp);
            break;
        }
        case EU1_GET_MMP_RESULT:
        {
            VC_XU_SET_ISP_CFG.dwMaxVal = 0;//(unsigned long)maxVal ;
            VC_XU_SET_ISP_CFG.dwMaxVal = 0;//(unsigned long)maxVal ;
            VC_XU_SET_ISP_CFG.dwResVal = 0;//(unsigned long)resVal ;
            //XU_Print("EU1_GET_MMP_RESULT\r\n");
            usb_vc_cmd_cfg(req, &VC_XU_GET_MMP_CFG, (unsigned long) eu1_get_mmp_result_val, resp);
            break;
        }
        case EU1_SET_ISP_EX:
        {
            VC_XU_SET_ISP_EX_CFG.dwMaxVal = 0;//(unsigned long)maxVal ;
            VC_XU_SET_ISP_EX_CFG.dwMinVal = 0;//(unsigned long)maxVal
            VC_XU_SET_ISP_EX_CFG.dwResVal = 0;//(unsigned long)resVal ;
            //XU_Print("EU1_SET_ISP_EX\r\n");
            usb_vc_cmd_cfg(req, &VC_XU_SET_ISP_EX_CFG, 0, resp);
            break;
        }
        case EU1_GET_ISP_EX_RESULT:
        {
            VC_XU_SET_ISP_EX_CFG.dwMaxVal = 0;//(unsigned long)maxVal ;
            VC_XU_SET_ISP_EX_CFG.dwMinVal = 0;//(unsigned long)maxVal ;
            VC_XU_SET_ISP_EX_CFG.dwResVal = 0;//(unsigned long)resVal ;
            //XU_Print("EU1_GET_ISP_EX\r\n");
            usb_vc_cmd_cfg(req, &VC_XU_GET_ISP_EX_CFG, (unsigned long) eu1_get_isp_ex_result_val, resp);
            break;
        }

        case EU1_READ_MMP_MEM:
        {
            unsigned char buf[EU1_READ_MMP_MEM_LEN];
            if (req == UVC_GET_CUR)
            {
                unsigned char n;
                unsigned char rlen;

                if (ReadMemCount>EU1_READ_MMP_MEM_LEN)
                    rlen = EU1_READ_MMP_MEM_LEN;
                else
                    rlen = ReadMemCount;

                ReadMemCount -= rlen;
                for (n = 0; n<16; ++n)
                {
                    if (n<rlen)
                        buf[n] = ((char*) ReadMemAddr)[n];
                    else
                        buf[n] = 0;
                }
                ReadMemAddr += rlen;
            }
            VC_XU_READ_MMP_MEM_CFG.dwMaxVal = 0;
            VC_XU_READ_MMP_MEM_CFG.dwMinVal = 0;
            VC_XU_READ_MMP_MEM_CFG.dwResVal = 0;
            usb_vc_cmd_cfg(req, &VC_XU_READ_MMP_MEM_CFG, (unsigned long) buf, resp);
            break;
        }
        case EU1_WRITE_MMP_MEM:
        {
            VC_XU_SET_MMP_MEM_CFG.dwMaxVal = 0;
            VC_XU_SET_MMP_MEM_CFG.dwMinVal = 0;
            VC_XU_SET_MMP_MEM_CFG.dwResVal = 0;
            usb_vc_cmd_cfg(req, &VC_XU_READ_MMP_MEM_CFG, 0, resp);
            break;
        }

        case EU1_GET_CHIP_INFO:
        {
            VC_XU_GET_CHIP_INFO_CFG.dwMaxVal = 0;//(unsigned long)maxVal ;
            VC_XU_GET_CHIP_INFO_CFG.dwMinVal = 0;//(unsigned long)maxVal ;
            VC_XU_GET_CHIP_INFO_CFG.dwResVal = 0;//(unsigned long)resVal ;

            XU_Print("EU1_GET_CHIP_INFO\r\n");
            eu1_get_chip_info_val[0] = '8';
            eu1_get_chip_info_val[1] = '4';
            eu1_get_chip_info_val[2] = '5';
            eu1_get_chip_info_val[3] = '5';
            eu1_get_chip_info_val[4] = 0;
            eu1_get_chip_info_val[5] = 0;;
            eu1_get_chip_info_val[6] = 0;
            eu1_get_chip_info_val[7] = 0;
            eu1_get_chip_info_val[8] = 0;
            eu1_get_chip_info_val[9] = 0;
            eu1_get_chip_info_val[10] = 0;
            eu1_get_chip_info_val[11] = 0;
            eu1_get_chip_info_val[12] = 0;
            eu1_get_chip_info_val[13] = 0;
            eu1_get_chip_info_val[14] = 0;
            eu1_get_chip_info_val[15] = 0;
            usb_vc_cmd_cfg(req, &VC_XU_GET_CHIP_INFO_CFG, (unsigned long) &eu1_get_chip_info_val[0], resp);
            break;
        }

        case EU1_GET_DATA_32:
        {
    //      unsigned char curVal[EU1_GET_DATA_32_LEN];
            unsigned char maxVal[EU1_GET_DATA_32_LEN];
            unsigned char resVal[EU1_GET_DATA_32_LEN];
            unsigned char i;
#if (ISP_IQ_SERVER)
            PCAM_IQSVR_HANDLE *iqsvr;

#endif

            for (i = 0; i<EU1_GET_DATA_32_LEN; i++) {
                maxVal[i] = 0xFF;
                resVal[i] = 0x00;
            }
            resVal[0] = 0x01;
            VC_XU_GET_DATA_32_CFG.dwMaxVal = (unsigned long) maxVal;
            VC_XU_GET_DATA_32_CFG.dwResVal = (unsigned long) resVal;
#if (ISP_IQ_SERVER)
            if (req == UVC_GET_CUR)
            {
                iqsvr = ISP_usb_get_iqsvr_handle();
                //here we send back data from IQ server
                if ((iqsvr->usDataBufCurPos + EU1_GET_DATA_32_LEN) <= IQ_XU_DATABUF_LEN)
                {
                    unsigned char *ptemp = (unsigned char*) (iqsvr->pubDataBuf + iqsvr->usDataBufCurPos);

                    XU_Print("EU1_GET_DATA_32: memcpy address:0x%x,0x%x\r\n", iqsvr->usDataBufCurPos, ptemp[0]);

                    memcpy(eu1_get_data_32_val,
                        iqsvr->pubDataBuf + iqsvr->usDataBufCurPos,
                        EU1_GET_DATA_32_LEN);
                }
                else {
                    memcpy(eu1_get_data_32_val, (void*)0xFF, EU1_GET_DATA_32_LEN);
                    XU_Print("[Err]: getting size is over than data buf size\n");
                }

                iqsvr->usDataBufCurPos += EU1_GET_DATA_32_LEN;
            }

#endif //(ISP_IQ_SERVER)
            //usb_vc_cmd_cfg(req, &VC_XU_GET_DATA_32_CFG, VC_XU_GET_DATA_32_CFG.dwCurVal, resp);
            usb_vc_cmd_cfg(req, &VC_XU_GET_DATA_32_CFG, (unsigned long) eu1_get_data_32_val, resp);
            break;
        }
        case EU1_SET_DATA_32:
        {
            VC_XU_SET_DATA_32_CFG.dwMaxVal = 0;//(unsigned long)maxVal ;
            VC_XU_SET_DATA_32_CFG.dwMinVal = 0;//(unsigned long)maxVal ;
            VC_XU_SET_DATA_32_CFG.dwResVal = 0;//(unsigned long)resVal ;
            VC_XU_SET_DATA_32_CFG.dwCurVal = 0;//(unsigned long)maxVal ;

            //XU_Print("EU1_SET_FW_DATA32\r\n");
            usb_vc_cmd_cfg(req, &VC_XU_SET_DATA_32_CFG, 0, resp);
            break;
        }

        case EU1_SET_MMP_CMD16:
        {
            VC_XU_SET_MMP_CMD16_CFG.dwMinVal = 0;
            VC_XU_SET_MMP_CMD16_CFG.dwMaxVal = 0;
            VC_XU_SET_MMP_CMD16_CFG.dwResVal = 0;
            usb_vc_cmd_cfg(req, &VC_XU_SET_MMP_CMD16_CFG, 0, resp);
            break;
        }
        case EU1_GET_MMP_CMD16_RESULT:
        {
            VC_XU_GET_MMP_CMD16_RESULT_CFG.dwMinVal = 0;
            VC_XU_GET_MMP_CMD16_RESULT_CFG.dwMaxVal = 0;
            VC_XU_GET_MMP_CMD16_RESULT_CFG.dwResVal = 0;
            usb_vc_cmd_cfg(req, &VC_XU_GET_MMP_CMD16_RESULT_CFG, (unsigned long) eu1_get_mmp_cmd16_result_val, resp);
            break;
        }

        case XU_CONTROL_UNDEFINED:
        default:
            // un-support
            goto EU1_CS_FAIL;
            break;
    }

    return ST_UVC_SUCCESS;

EU1_CS_FAIL:
    /*
    * We don't support this control, so STALL the
    * default control ep.
    */
    return -1;
}

int8_t usb_vc_eu1_cs_out(ST_UVC_Device_t *pdev, uint8_t entity_id, uint8_t cs, uint32_t len, struct uvc_request_data *data)
{
#if (ISP_IQ_SERVER)
    unsigned char len;
    PCAM_IQSVR_HANDLE *iqsvr;
#endif

    switch (cs)
    {
    case EU1_SET_ISP:
        usb_vc_eu1_isp_cmd(data->data);
        break;
    case EU1_GET_ISP_RESULT:
        break;
    case EU1_SET_FW_DATA:
        //set fw data
        //Move to USB task
#if (ISP_IQ_SERVER)
        iqsvr = ISP_usb_get_iqsvr_handle();
        if ((iqsvr->usCmdBufCurPos + len) <= IQ_XU_CMDBUF_LEN)
        {
            if (iqsvr->usCmdBufCurPos < iqsvr->usCmdLen)
            {
                char *cmd = (char*)data->data;
                XU_Print("EU1_SET_FW_DATA::Len:0x%x, cmd:%x %x %x %x %x %x %x %x\r\n", len,
                    cmd[0], cmd[1], cmd[2], cmd[3], cmd[4], cmd[5], cmd[6], cmd[7]);

                memcpy(iqsvr->pubCmdBuf + iqsvr->usCmdBufCurPos, data->data, len);
                iqsvr->usCmdBufCurPos += len;
                // Wake up IQ server to handle request from tool
                if (iqsvr->usCmdBufCurPos >= iqsvr->usCmdLen) {
                    //MMPF_OS_SetFlags(SENSOR_Flag, SENSOR_FLAG_PCAMOP, MMPF_OS_FLAG_SET);

                    ISP_Process(); // to process isp.
                    XU_Print("[Mes] ready to process ISP data\r\n");
                }
            }
            else {
                // show error
                XU_Print("[Err] Tool sends an over-size command (%d)\n", iqsvr->usCmdLen);
            }
        }
        else
        {
            XU_Print("[Err] Tool sends an over max size command (%d, %d)\n", iqsvr->usCmdLen, IQ_XU_CMDBUF_LEN);
        }
#else
        XU_Print("FW_DATA: %d, %d\n", len, data->length);
        UVC_DFU_Update(pdev, data->data, data->length);
#endif //(ISP_IQ_SERVER)
        break;
    case EU1_SET_MMP:

        usb_vc_eu1_mmp_cmd(pdev, data->data);
        break;
    case EU1_GET_DATA_32:
    {
        uint8_t maxVal[EU1_GET_DATA_32_LEN];
        uint8_t resVal[EU1_GET_DATA_32_LEN];
        uint8_t i;

        XU_Print("EU1_GET_DATA_32\r\n");
        for (i = 0; i < EU1_GET_DATA_32_LEN; i++) {
            maxVal[i] = 0xFF;
            resVal[i] = 0x00;
        }
        VC_XU_GET_DATA_32_CFG.dwMaxVal = (uint32_t) maxVal;
        VC_XU_GET_DATA_32_CFG.dwResVal = (uint32_t) resVal;
    }
    break;
    default:
        XU_Print(":: not support.\r\n");
        break;
    }
    return ST_UVC_SUCCESS;
}

void usb_vc_cmd_cfg(uint8_t req, VC_CMD_CFG *cfg, unsigned long cur_val, struct uvc_request_data *resp)
{
    XU_Print("vc.req : 0x%x\n", req);
    XU_Print("vc.val : 0x%lx\n", cur_val);
    XU_Print("cmd cap : 0x%lx, info cap : 0x%x\n",cfg->bCmdCap, cfg->bInfoCap);

    switch (req)
    {
    case UVC_SET_CUR:
        if (cfg->bCmdCap & CAP_SET_CUR_CMD) {
            if (cfg->bInfoCap & INFO_AUTO_MODE_SUPPORT) {

            }
        }
        else {
            goto invalid_req;
        }
        break;
    case UVC_GET_CUR:
        if (cfg->bCmdCap & CAP_GET_CUR_CMD) {
            memcpy(resp->data, (void*)cur_val, cfg->bCmdLen);
            resp->length = cfg->bCmdLen;
        }
        else {
            goto invalid_req;
        }
        break;
    case UVC_GET_INFO:
        if (cfg->bCmdCap & CAP_GET_INFO_CMD) {
            XU_Print("vc.bInfoCap : %x\r\n", cfg->bInfoCap);
            resp->data[0] = (INFO_SET_SUPPORT | INFO_GET_SUPPORT);
            resp->length = sizeof(char);
        }
        else {
            goto invalid_req;
        }
        break;

    case UVC_GET_DEF:
        if (cfg->bCmdCap & CAP_GET_DEF_CMD) {
            XU_Print("vc.def : %lx\n",cfg->dwDefVal);
            resp->data[0] = 0x0;
            resp->data[1] = 0x0;
            resp->length = sizeof(unsigned short);
        }
        else {
            goto invalid_req;
        }
        break;
    case UVC_GET_MAX:
        if (cfg->bCmdCap & CAP_GET_MAX_CMD) {
            XU_Print("vc.max : %lx\n", cfg->dwMaxVal);
            resp->data[0] = 0x1;
            resp->data[1] = 0x0;
            resp->length = sizeof(unsigned short);
        }
        else {
            goto invalid_req;
        }
        break;
    case UVC_GET_MIN:
        if (cfg->bCmdCap & CAP_GET_MIN_CMD) {
            XU_Print("vc.mix : %lx\n", cfg->dwMinVal);
            resp->data[0] = 0x0;
            resp->length = sizeof(char);
        }
        else {
            goto invalid_req;
        }
        break;
    case UVC_GET_RES:
        if (cfg->bCmdCap & CAP_GET_RES_CMD) {
            XU_Print("vc.res : %lx\n", cfg->dwResVal);
            resp->data[0] = 0x1;
            resp->data[1] = 0x0;
            resp->length = sizeof(unsigned short);
        }
        else {
            goto invalid_req;
        }
        break;
    case UVC_GET_LEN:
        if (cfg->bCmdCap & CAP_GET_LEN_CMD) {
            unsigned short cmdLen = cfg->bCmdLen;
            memcpy(resp->data, (unsigned char *) &cmdLen, sizeof(unsigned short));
            resp->length = sizeof(unsigned short);
        }
        else {
            goto invalid_req;
        }
        break;
    default:
    invalid_req:
        XU_Print("Cfg.Err\r\n");
        //UsbWriteEp0CSR(SET_EP0_SENDSTALL);
        //gbVCERRCode = CONTROL_INVALID_REQUEST;
        break;
    }

    XU_Print("Cfg.End\r\n");
}

//CUSTOMER EU
int8_t usb_vc_eu2_cs(uint8_t cs, uint8_t req, struct uvc_request_data *resp)
{
    switch (cs)
    {
        case CUS_XU_SET_ISP:
        {
            XU_Print("CUS_XU_SET_ISP\r\n");
            usb_vc_cmd_cfg(req, &CUS_XU_SET_ISP_CFG, (unsigned long) cus_eu_set_isp_config, resp);
            break;
        }

        case CUS_XU_GET_ISP:
        {
            XU_Print("CUS_XU_GET_ISP\r\n");
            usb_vc_cmd_cfg(req, &CUS_XU_GET_ISP_CFG, (unsigned long) cus_eu_get_isp_config, resp);
            break;
        }

        case CUS_XU_SET_ISP_UT:
        {
            XU_Print("CUS_XU_SET_ISP_UT\r\n");
            usb_vc_cmd_cfg(req, &CUS_XU_SET_ISP_CFG_UT, (unsigned long) cus_eu_set_isp_config_ut, resp);
            break;
        }

        case CUS_XU_GET_ISP_UT:
        {
            XU_Print("CUS_XU_GET_ISP_UT\r\n");
            usb_vc_cmd_cfg(req, &CUS_XU_GET_ISP_CFG_UT, (unsigned long) cus_eu_get_isp_config_ut, resp);
            break;
        }

        case XU_CONTROL_UNDEFINED:
        default:
            // un-support
            goto EU2_CS_FAIL;
            break;
    }
    return ST_UVC_SUCCESS;

EU2_CS_FAIL:
    /*
    * We don't support this control, so STALL the
    * default control ep.
    */
    return -1;
}

int8_t usb_cus_set_isp(struct uvc_request_data *data)
{
    u_int32_t cmdVal = (*(u_int32_t *)data->data);
    if (data->length != CUS_XU_ISP_LEN)
    {
        XU_Print("Invalid ISP Data\n");
        return -1;
    }

    XU_Print("Cus Cmd Val(0x%x) \n", cmdVal);
    switch (cmdVal)
    {
        case REQUEST_IFRAME:
            break;
        case CUS_UNKONW_CMD:
        default:
           XU_Print("unkonown Cmd\n");
           break;
    }
    return 0;
}

int8_t usb_cus_get_isp(struct uvc_request_data *data)
{
    return 0;
}

int8_t usb_cus_set_isp_ut(struct uvc_request_data *data)
{
    if (data->length != CUS_XU_ISP_LEN_UT)
    {
        XU_Print("Invalid ISP Data\n");
        return -1;
    }

    memcpy(cus_eu_get_isp_config_ut, data->data, data->length);
    return 0;
}

int8_t usb_cus_get_isp_ut(struct uvc_request_data *data)
{
    return 0;
}

int8_t usb_vc_eu2_cs_out(uint8_t entity_id, uint8_t cs, uint32_t len, struct uvc_request_data *data)
{
    switch (cs)
    {
        case CUS_XU_SET_ISP:
        {
            XU_Print("CUS_XU_SET_ISP\n");
            usb_cus_set_isp(data);
        }
        break;

        case CUS_XU_GET_ISP:
        {
            XU_Print("CUS_XU_GET_ISP\r\n");
            usb_cus_get_isp(data);
        }
        break;

        case CUS_XU_SET_ISP_UT:
        {
            XU_Print("CUS_XU_SET_ISP_UT\r\n");
            usb_cus_set_isp_ut(data);
        }
        break;

        case CUS_XU_GET_ISP_UT:
        {
            XU_Print("CUS_XU_GET_ISP_UT\r\n");
            usb_cus_get_isp_ut(data);
        }
        break;

        default:
            XU_Print(":: not support.\n");
        break;
    }
    return ST_UVC_SUCCESS;
}

int8_t usb_vc_ct_cs_out(ST_UVC_Device_t *pdev, uint8_t entity_id, uint8_t cs, uint32_t len, struct uvc_request_data *data)
{
    switch (cs)
    {
    case UVC_CT_SCANNING_MODE_CONTROL:
        if(data->data[0] != 0x00 && data->data[0] != 0x01)
        {
            pdev->request_error_code.data[0] = 0x04;
            pdev->request_error_code.length = 1;
            return -1;
        }
        ct_scanning_mode_data = data->data[0];
        break;

    default:
        XU_Print(":: not support.\n");
        break;
    }

    return ST_UVC_SUCCESS;
}

// qanthink
const static MI_ISP_DEV ispDevId = 0;
const static MI_ISP_CHANNEL ispChnId = 0;
static int ret = 0;
// qanthink end

int8_t usb_vc_pu_cs_out(ST_UVC_Device_t *pdev, uint8_t entity_id, uint8_t cs, uint32_t len, struct uvc_request_data *data)
{
	switch(cs)
	{
		case UVC_PU_BRIGHTNESS_CONTROL:
		{
			//printf("In usb_vc_pu_cs_out(), UVC_PU_BRIGHTNESS_CONTROL\n");
			if(data->data[0] < 0x00 || data->data[0] > 0xFF || data->data[1] != 0x00)
			{
				pdev->request_error_code.data[0] = 0x04;
				pdev->request_error_code.length = 1;
				return -1;
			}
			pu_brightness_data[0] = data->data[0];
			pu_brightness_data[1] = data->data[1];
			break;
		}
		#if 1	// 额外增加的UVC_PU选项，用于在windows 工具上调试ISP.
		case UVC_PU_BACKLIGHT_COMPENSATION_CONTROL:
		{
			if(data->data[0] < 0x00 || data->data[0] > 0xFF || data->data[1] != 0x00)
			{
				pdev->request_error_code.data[0] = 0x04;
				pdev->request_error_code.length = 1;
				return -1;
			}
			pu_backlight_compensation_data[0] = data->data[0];
			pu_backlight_compensation_data[1] = data->data[1];
			break;
		}
		case UVC_PU_CONTRAST_CONTROL:
		{
			if(data->data[0] < 0x00 || data->data[0] > 0xFF || data->data[1] != 0x00)
			{
				pdev->request_error_code.data[0] = 0x04;
				pdev->request_error_code.length = 1;
				return -1;
			}
			pu_contrast_data[0] = data->data[0];
			pu_contrast_data[1] = data->data[1];
			break;
		}
		case UVC_PU_GAIN_CONTROL:
		{
			if(data->data[0] < 0x00 || data->data[0] > 0xFF || data->data[1] != 0x00)
			{
				pdev->request_error_code.data[0] = 0x04;
				pdev->request_error_code.length = 1;
				return -1;
			}
			pu_gain_data[0] = data->data[0];
			pu_gain_data[1] = data->data[1];
			break;
		}
		case UVC_PU_POWER_LINE_FREQUENCY_CONTROL:
		{
			//printf("pu_cs_out: UVC_PU_POWER_LINE_FREQUENCY_CONTROL\n");
			if(data->data[0] < 0x00 || data->data[0] > 0xFF || data->data[1] != 0x00)
			{
				pdev->request_error_code.data[0] = 0x04;
				pdev->request_error_code.length = 1;
				return -1;
			}
			pu_power_line_frequency_data[0] = data->data[0];
			pu_power_line_frequency_data[1] = data->data[1];
			//printf("_data[0] = %d\n", pu_power_line_frequency_data[0]);
			//printf("_data[1] = %d\n", pu_power_line_frequency_data[1]);

			if(0x01 == pu_power_line_frequency_data[0])
			{
				printf("Set flicker as 50Hz.\n");
				MI_ISP_AE_FLICKER_TYPE_e eFlickerType = SS_AE_FLICKER_TYPE_50HZ;
				ret = MI_ISP_AE_SetFlicker(ispDevId, ispChnId, &eFlickerType);
				if(MI_ISP_FAILURE == ret)
				{
					printf("Fail to call MI_ISP_AE_SetFlicker() in usb_vc_pu_cs_out().\n");
				}
			}
			else if(0x02 == pu_power_line_frequency_data[0])
			{
				printf("Set flicker as 60Hz.\n");
				MI_ISP_AE_FLICKER_TYPE_e eFlickerType = SS_AE_FLICKER_TYPE_60HZ;
				ret = MI_ISP_AE_SetFlicker(ispDevId, ispChnId, &eFlickerType);
				if(MI_ISP_FAILURE == ret)
				{
					printf("Fail to call MI_ISP_AE_SetFlicker() in usb_vc_pu_cs_out().\n");
				}
			}
			else
			{
				printf("In UVC_PU_POWER_LINE_FREQUENCY_CONTROL, bad data.\n");
			}
			break;
		}
		case UVC_PU_HUE_CONTROL:
		{
			if(data->data[0] < 0x00 || data->data[0] > 0xFF || data->data[1] != 0x00)
			{
				pdev->request_error_code.data[0] = 0x04;
				pdev->request_error_code.length = 1;
				return -1;
			}
			pu_hue_data[0] = data->data[0];
			pu_hue_data[1] = data->data[1];
			break;
		}
		case UVC_PU_HUE_AUTO_CONTROL:		// 未实现
		{
			if(data->data[0] < 0x00 || data->data[0] > 0xFF || data->data[1] != 0x00)
			{
				pdev->request_error_code.data[0] = 0x04;
				pdev->request_error_code.length = 1;
				return -1;
			}
			pu_hue_data[0] = data->data[0];
			pu_hue_data[1] = data->data[1];
			break;
		}
		case UVC_PU_SATURATION_CONTROL:
		{
			if(data->data[0] < 0x00 || data->data[0] > 0xFF || data->data[1] != 0x00)
			{
				pdev->request_error_code.data[0] = 0x04;
				pdev->request_error_code.length = 1;
				return -1;
			}
			pu_saturatio_data[0] = data->data[0];
			pu_saturatio_data[1] = data->data[1];
			break;
		}
		case UVC_PU_SHARPNESS_CONTROL:
		{
			if(data->data[0] < 0x00 || data->data[0] > 0xFF || data->data[1] != 0x00)
			{
				pdev->request_error_code.data[0] = 0x04;
				pdev->request_error_code.length = 1;
				return -1;
			}
			pu_sharpness_data[0] = data->data[0];
			pu_sharpness_data[1] = data->data[1];
			break;
		}
		case UVC_PU_GAMMA_CONTROL:
		{
			if(data->data[0] < 0x00 || data->data[0] > 0xFF || data->data[1] != 0x00)
			{
				pdev->request_error_code.data[0] = 0x04;
				pdev->request_error_code.length = 1;
				return -1;
			}
			pu_gamma_data[0] = data->data[0];
			pu_gamma_data[1] = data->data[1];
			break;
		}
		case UVC_PU_WHITE_BALANCE_TEMPERATURE_CONTROL:
		{
			if(data->data[0] < 0x00 || data->data[0] > 0xFF || data->data[1] != 0x00)
			{
				pdev->request_error_code.data[0] = 0x04;
				pdev->request_error_code.length = 1;
				return -1;
			}
			pu_white_balance_temperature_data[0] = data->data[0];
			pu_white_balance_temperature_data[1] = data->data[1];
			break;
		}
		case UVC_PU_WHITE_BALANCE_TEMPERATURE_AUTO_CONTROL:	// 未实现
		{
			if(data->data[0] < 0x00 || data->data[0] > 0xFF || data->data[1] != 0x00)
			{
				pdev->request_error_code.data[0] = 0x04;
				pdev->request_error_code.length = 1;
				return -1;
			}
			//pu_white_balance_temperature_data[0] = data->data[0];
			//pu_white_balance_temperature_data[1] = data->data[1];
			break;
		}
		case UVC_PU_WHITE_BALANCE_COMPONENT_CONTROL:		// 未实现
		{
			if(data->data[0] < 0x00 || data->data[0] > 0xFF || data->data[1] != 0x00)
			{
				pdev->request_error_code.data[0] = 0x04;
				pdev->request_error_code.length = 1;
				return -1;
			}
			//pu_white_balance_component_data[0] = data->data[0];
			//pu_white_balance_component_data[1] = data->data[1];
			break;
		}
		case UVC_PU_WHITE_BALANCE_COMPONENT_AUTO_CONTROL:	// 未实现
		{
			if(data->data[0] < 0x00 || data->data[0] > 0xFF || data->data[1] != 0x00)
			{
				pdev->request_error_code.data[0] = 0x04;
				pdev->request_error_code.length = 1;
				return -1;
			}
			//pu_white_balance_component_data[0] = data->data[0];
			//pu_white_balance_component_data[1] = data->data[1];
			break;
		}
		case UVC_PU_DIGITAL_MULTIPLIER_CONTROL:			// 未实现
		{
			if(data->data[0] < 0x00 || data->data[0] > 0xFF || data->data[1] != 0x00)
			{
				pdev->request_error_code.data[0] = 0x04;
				pdev->request_error_code.length = 1;
				return -1;
			}
			//[0] = data->data[0];
			//[1] = data->data[1];
			break;
		}
		case UVC_PU_DIGITAL_MULTIPLIER_LIMIT_CONTROL:	// 未实现
		{
			if(data->data[0] < 0x00 || data->data[0] > 0xFF || data->data[1] != 0x00)
			{
				pdev->request_error_code.data[0] = 0x04;
				pdev->request_error_code.length = 1;
				return -1;
			}
			//[0] = data->data[0];
			//[1] = data->data[1];
			break;
		}
		case UVC_PU_ANALOG_VIDEO_STANDARD_CONTROL:		// 未实现
		{
			if(data->data[0] < 0x00 || data->data[0] > 0xFF || data->data[1] != 0x00)
			{
				pdev->request_error_code.data[0] = 0x04;
				pdev->request_error_code.length = 1;
				return -1;
			}
			//[0] = data->data[0];
			//[1] = data->data[1];
			break;
		}
		case UVC_PU_ANALOG_LOCK_STATUS_CONTROL:			// 未实现
		{
			if(data->data[0] < 0x00 || data->data[0] > 0xFF || data->data[1] != 0x00)
			{
				pdev->request_error_code.data[0] = 0x04;
				pdev->request_error_code.length = 1;
				return -1;
			}
			//[0] = data->data[0];
			//[1] = data->data[1];
			break;
		}
		#endif
		default:
		{
			XU_Print(":: not support.\n");
			break;
		}
	}	// end switch cs.

	return ST_UVC_SUCCESS;
}


