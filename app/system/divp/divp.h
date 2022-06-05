/*---------------------------------------------------------------- 
xxx版权所有。
作者：
时间：2022.6.5
----------------------------------------------------------------*/

#pragma once

#include "mi_sys.h"
#include "venc.h"
#include "mi_divp.h"

#if 0
typedef enum{
	emResolInvalid = 0x00,
	
	emResol1MP_1280_720 = 0x10,

	emResol2MP_1920_1080 = 0x20,
	
	emResol3MP = 0x30,
	
	emResol4MP_2560_1440 = 0x40,
	
	emResol5MP_3072_1728 = 0x50,
	
	emResol6MP = 0x60,
	
	emResol7MP = 0x70,
	
	emResol8MP_3840_2160 = 0x80,
}emResol_t;
#endif

class Divp{
public:
	const static MI_DIVP_CHN divpChn = 0;

	static Divp *getInstance();
	MI_S32 enable();
	MI_S32 disable();

	MI_S32 createChn(MI_DIVP_ChnAttr_t *pstChnAttr);
private:
	Divp();
	~Divp();
	Divp(const Divp&);
	Divp& operator=(const Divp&);

	bool bEnable;

	// DIVP 输入、输出分辨率。
	emResol_t emResIn = emResol4MP_2560_1440;
	
	//emResol_t emResOut = emResol4MP_2560_1440;
	emResol_t emResOut = emResol2MP_1920_1080;
};

