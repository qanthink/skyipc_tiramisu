/*---------------------------------------------------------------- 
xxx版权所有。
作者：
时间：2020.7.10
----------------------------------------------------------------*/

#pragma once

#include "mi_scl.h"

class Scl{
public:
	static Scl* getInstance();

	MI_S32 enable();
	MI_S32 disable();

public:
	const static MI_SCL_DEV SclDevId = 0;

private:
	Scl();
	~Scl();
	Scl(const Scl&);
	Scl& operator=(const Scl&);

	bool bEnable = false;
};

