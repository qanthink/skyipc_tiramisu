/*---------------------------------------------------------------- 
xxx版权所有。
作者：
时间：2022.6.5
----------------------------------------------------------------*/

#pragma once

#include "mi_sys.h"
#include "venc.h"
#include "mi_divp.h"

class Divp{
public:
	static Divp *getInstance();

private:
	Divp();
	~Divp();
	Divp(const Divp&);
	Divp& operator=(const Divp&);

public:
	const static MI_DIVP_CHN divpChn = 0;
	MI_S32 createChn(unsigned int wIn, unsigned int hIn, 
						unsigned int wOut, unsigned int hOut);
};

