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
	const static MI_SCL_PORT sclPortMain = 0;
	const static MI_SCL_PORT sclPortSub = 1;
	const static MI_SCL_PORT sclPortJpeg = 2;
	
	MI_S32 enable();
	MI_S32 disable();

	int createPort(MI_SCL_PORT sclPortId, 
				unsigned int widthOut, unsigned int heightOut, bool bIsJpeg = false);
	int destoryPort(MI_SCL_PORT sclPortId);

private:
	Scl();
	~Scl();
	Scl(const Scl&);
	Scl& operator=(const Scl&);

	bool bEnable = false;
};

