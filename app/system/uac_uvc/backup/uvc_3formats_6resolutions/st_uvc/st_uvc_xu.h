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
#ifndef _ST_UVC_XU_H_
#define _ST_UVC_XU_H_

#include <stdint.h>
#include "st_uvc_xu_datatype.h"

#define EU_ISP_CMD_OK			0x00
#define EU_ISP_CMD_NOT_SUPPORT	0x80

typedef struct _VC_CMD_CFG
{
	unsigned long bCmdCap;
	unsigned char bInfoCap;
	unsigned short bCmdLen;
	unsigned char bInfoLen;
	/*If cmd > 4, the following data is a pointer*/
	unsigned long  dwResVal;
	unsigned long  dwDefVal;
	unsigned long  dwMinVal;
	unsigned long  dwMaxVal;
	unsigned long  dwCurVal;
} VC_CMD_CFG;

void usb_vc_cmd_cfg(uint8_t req, VC_CMD_CFG *cfg, unsigned long cur_val, struct uvc_request_data *resp);

// process PU, CT, XU job.
uint8_t pu_brightness_data[2];
#if 1	// 额外增加的UVC_PU选项，用于在windows 工具上调试ISP.
uint8_t pu_contrast_data[2];				// 对比度
uint8_t pu_hue_data[2];						// 色度
uint8_t pu_saturatio_data[2];				// 饱和度
uint8_t pu_sharpness_data[2];				// 锐度，可否理解为清晰度？
uint8_t pu_gamma_data[2];					// 伽马

uint8_t pu_white_balance_temperature_data[2];	// 白平衡
//#define UVC_PU_WHITE_BALANCE_TEMPERATURE_CONTROL	0x0a
//#define UVC_PU_WHITE_BALANCE_TEMPERATURE_AUTO_CONTROL	0x0b
//#define UVC_PU_WHITE_BALANCE_COMPONENT_CONTROL		0x0c
//#define UVC_PU_WHITE_BALANCE_COMPONENT_AUTO_CONTROL	0x0d

uint8_t pu_backlight_compensation_data[2];	// 逆光对比，背光补偿
uint8_t pu_gain_data[2];					// 增益
uint8_t pu_power_line_frequency_data[2];	// 电力线频率

// 数字变倍
//#define UVC_PU_DIGITAL_MULTIPLIER_CONTROL		0x0e
//#define UVC_PU_DIGITAL_MULTIPLIER_LIMIT_CONTROL		0x0f
#endif
uint8_t ct_scanning_mode_data;

int8_t usb_vc_ct_cs_out(ST_UVC_Device_t *pdev, uint8_t entity_id, uint8_t cs, uint32_t len, struct uvc_request_data *data);
int8_t usb_vc_pu_cs_out(ST_UVC_Device_t *pdev, uint8_t entity_id, uint8_t cs, uint32_t len, struct uvc_request_data *data);

int8_t usb_vc_eu1_cs_out(ST_UVC_Device_t *pdev, uint8_t entity_id, uint8_t cs, uint32_t len, struct uvc_request_data *data);
int8_t usb_vc_eu1_cs(uint8_t cs, uint8_t req, struct uvc_request_data *resp);

int8_t usb_vc_eu2_cs_out(uint8_t entity_id, uint8_t cs, uint32_t len, struct uvc_request_data *data);
int8_t usb_vc_eu2_cs(uint8_t cs, uint8_t req, struct uvc_request_data *resp);

typedef struct _DataExchangeParam
{
	unsigned short id;			//command ID
	unsigned long data_len;		//transfer lens
	unsigned long cur_offset;	//current read/write offset
	char *ptr;					//data buffer
} DataExchangeParam;
#endif // _ST_UVC_XU_H_
