#ifndef _ST_UAC_H_
#define _ST_UAC_H_

#if defined(__cplusplus) || defined(c_plusplus)
extern "C"{
#endif

#include "st_uac_datatype.h"

void ST_UAC_Config(ST_UAC_Config_t *uac_config,
			                        uint32_t flags,
			                        uint32_t channels,
			                        uint32_t rate,
			                        uint32_t period_size,
			                        uint32_t period_count,
			                        enum pcm_format format);

int32_t ST_UAC_Init(ST_UAC_Handle_h*, ST_UAC_Config_t*);

int32_t ST_UAC_Uninit(ST_UAC_Handle_h);

int32_t ST_UAC_SendFrame(ST_UAC_Handle_h, const ST_UAC_Frame_t*);

int32_t ST_UAC_GetFrame(ST_UAC_Handle_h, ST_UAC_Frame_t*);

int32_t ST_UAC_GetPcm_BufSize(ST_UAC_Handle_h handle);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif//_ST_UAC_H_
