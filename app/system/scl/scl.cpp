/*---------------------------------------------------------------- 
xxx版权所有。
作者：
时间：2020.7.10
----------------------------------------------------------------*/

#include "scl.h"
#include "st_common.h"
#include <iostream>
#include <string.h>

using namespace std;

Scl::Scl()
{
	enable();
}

Scl::~Scl()
{
	disable();
}

/*-----------------------------------------------------------------------------
描--述：VIF 模块获取实例的唯一入口函数。
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
Scl* Scl::getInstance()
{
	static Scl vif;
	return &vif;
}

/*-----------------------------------------------------------------------------
描--述：
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
MI_S32 Scl::enable()
{
	cout << "Call Scl::enable()." << endl;
	
	if(bEnable)
	{
		cerr << "Fail to call Scl::enable(). Modules is already been enabled!" << endl;
		return 0;
	}
	
	MI_SCL_DevAttr_t stSclDevAttr;
	MI_SCL_ChannelAttr_t stSclChnAttr;
	MI_SCL_ChnParam_t stSclChnParam;
	memset(&stSclDevAttr, 0x0, sizeof(MI_SCL_DevAttr_t));
	memset(&stSclChnAttr, 0x0, sizeof(MI_SCL_ChannelAttr_t));
	memset(&stSclChnParam, 0x0, sizeof(MI_SCL_ChnParam_t));

	int SCL_PORT_ID = 0;
	switch(SCL_PORT_ID)
	{
		case 0:
		{

			stSclDevAttr.u32NeedUseHWOutPortMask = E_MI_SCL_HWSCL2; //Port0->HWSCL2
			break;
		}
		case 1:
		{
			stSclDevAttr.u32NeedUseHWOutPortMask = E_MI_SCL_HWSCL3; //Port1->HWSCL3
			break;
		}
		case 2:
		{
			stSclDevAttr.u32NeedUseHWOutPortMask = E_MI_SCL_HWSCL4; //Port2->HWSCL4
			break;
		}
		default:
		{
			ST_ERR("Invalid Scl Port Id:%d\n", SCL_PORT_ID);
			return -1;
		}
	}
	STCHECKRESULT(MI_SCL_CreateDevice(SclDevId, &stSclDevAttr));

	int g_device_num = 1;
	for(int i = 0; i < g_device_num; i++)
	{
		STCHECKRESULT(MI_SCL_CreateChannel(SclDevId, i, &stSclChnAttr));
		STCHECKRESULT(MI_SCL_SetChnParam(SclDevId, i, &stSclChnParam));
		STCHECKRESULT(MI_SCL_StartChannel(SclDevId, i));
	}

	bEnable = true;

	cout << "Call Scl::enable() end." << endl;
	return 0;	
}

/*-----------------------------------------------------------------------------
描--述：
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
MI_S32 Scl::disable()
{
	cout << "Call Scl::disable()." << endl;

	//disableChnPort(vifPort);
	bEnable = false;

	cout << "Call Scl::disable() end." << endl;
	return 0;
}

