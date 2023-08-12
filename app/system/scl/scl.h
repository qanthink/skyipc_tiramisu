/*---------------------------------------------------------------- 
xxx版权所有。
作者：
时间：2020.7.10
----------------------------------------------------------------*/

#pragma once

#include "mi_scl.h"

/*
	SCL 可以创建多个channel.
*/
class Scl{
public:
	static Scl* getInstance();

	const static MI_SCL_DEV sclDevId = 0;
	const static MI_SCL_CHANNEL sclChnId = 0;
	const static MI_SCL_PORT sclPortId = 0;

	MI_S32 enable();
	MI_S32 disable();

private:
	Scl();
	~Scl();
	Scl(const Scl&);
	Scl& operator=(const Scl&);

	bool bEnable = false;
};

