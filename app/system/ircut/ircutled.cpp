/*---------------------------------------------------------------- 
xxx版权所有。
作者：
时间：2020.7.10
----------------------------------------------------------------*/

#include "ircutled.h"

#include <thread>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <iostream>

using namespace std;

IrCutLed::IrCutLed()
{
}

IrCutLed::~IrCutLed()
{
}

IrCutLed* IrCutLed::getInstance()
{
	static IrCutLed irCut;
	return &irCut;
}

/*-----------------------------------------------------------------------------
描--述：打开滤波片。
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
int IrCutLed::openFilter()
{
	cout << "Call IrCutLed::openFilter()." << endl;
	gpioSetVal(PIN_IRCUT_AIN, 1);
	std::this_thread::sleep_for(std::chrono::microseconds(400 * 1000));
	gpioSetVal(PIN_IRCUT_AIN, 0);

	cout << "Call IrCutLed::openFilter() end." << endl;
	return 0;
}

/*-----------------------------------------------------------------------------
描--述：关闭滤波片。
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
int IrCutLed::closeFilter()
{
	cout << "Call IrCutLed::closeFilter()." << endl;
	gpioSetVal(PIN_IRCUT_AIN, 0);
	gpioSetVal(PIN_IRCUT_BIN, 1);
	std::this_thread::sleep_for(std::chrono::microseconds(400 * 1000));
	gpioSetVal(PIN_IRCUT_BIN, 0);

	cout << "Call IrCutLed::closeFilter() end." << endl;
	return 0;
}

/*-----------------------------------------------------------------------------
描--述：复位滤波片。
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
int IrCutLed::resetFilter()
{
	cout << "Call IrCutLed::resetFilter()." << endl;
	openFilter();
	closeFilter();

	cout << "Call IrCutLed::resetFilter() end." << endl;
	return 0;
}

/*-----------------------------------------------------------------------------
描--述：获取GPIO 的值。
参--数：gpioIndex, GPIO标号；pVal, 指向值的指针。
返回值：
注--意：GPIO 的方向设置为输入。
-----------------------------------------------------------------------------*/
int IrCutLed::gpioGetVal(unsigned int gpioIndex, int *pVal)
{
	cout << "Call IrCutLed::gpioGetVal()." << endl;

	unsigned int cmdBufSizes = 128;
	char cmdBuf[cmdBufSizes] = {0};
	
	sprintf(cmdBuf, "echo %d > /sys/class/gpio/export", gpioIndex);
	system(cmdBuf);

	sprintf(cmdBuf, "echo in > /sys/class/gpio/gpio%d/direction", gpioIndex);
	system(cmdBuf);

	#if 0
	sprintf(cmdBuf, "echo %d > /sys/class/gpio/gpio%d/value", val, gpioIndex);
	system(cmdBuf);
	#else
	sprintf(cmdBuf, "/sys/class/gpio/gpio%d/value", PIN_PHOTO_SENS);
	
	int fd = -1;
	fd = open(cmdBuf, O_RDWR);
	if(-1 == fd)
	{
		cerr << "Fail to call open(2) in IrCutLed::getPhotoSensVal(). " << strerror(errno) << endl;
	}
	else
	{
		int ret = 0;
		ret = read(fd, pVal, sizeof(int));
		if(-1 == ret)
		{
			cerr << "Fail to call read(2) in IrCutLed::getPhotoSensVal(). " << strerror(errno) << endl;
		}

		close(fd);
		fd = -1;
	}
	#endif

	sprintf(cmdBuf, "echo %d > /sys/class/gpio/unexport", gpioIndex);
	system(cmdBuf);

	cout << "Call IrCutLed::gpioGetVal() end." << endl;
	return 0;
}

/*-----------------------------------------------------------------------------
描--述：设置GPIO 的值。
参--数：gpioIndex, GPIO标号；val, 值。
返回值：
注--意：GPIO 的方向设置为输出。
-----------------------------------------------------------------------------*/
int IrCutLed::gpioSetVal(unsigned int gpioIndex, int val)
{
	cout << "Call IrCutLed::gpioSetVal()." << endl;

	unsigned int cmdBufSizes = 128;
	char cmdBuf[cmdBufSizes] = {0};
	
	sprintf(cmdBuf, "echo %d > /sys/class/gpio/export", gpioIndex);
	system(cmdBuf);

	sprintf(cmdBuf, "echo out > /sys/class/gpio/gpio%d/direction", gpioIndex);
	system(cmdBuf);

	sprintf(cmdBuf, "echo %d > /sys/class/gpio/gpio%d/value", val, gpioIndex);
	system(cmdBuf);

	sprintf(cmdBuf, "echo %d > /sys/class/gpio/unexport", gpioIndex);
	system(cmdBuf);

	cout << "Call IrCutLed::gpioSetVal() end." << endl;
	return 0;
}

/*-----------------------------------------------------------------------------
描--述：打开LED 红外补光灯。
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
int IrCutLed::openLedIr()
{
	cout << "Call IrCutLed::openLedIr()." << endl;
	gpioSetVal(PIN_LED_IR, 1);
	cout << "Call IrCutLed::openLedIr() end." << endl;
	return 0;
}

/*-----------------------------------------------------------------------------
描--述：关闭LED 红外补光灯。
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
int IrCutLed::closeLedIr()
{
	cout << "Call IrCutLed::closeLedIr()." << endl;
	gpioSetVal(PIN_LED_IR, 0);
	cout << "Call IrCutLed::closeLedIr() end." << endl;
	return 0;
}

/*-----------------------------------------------------------------------------
描--述：打开LED 白光补光灯。
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
int IrCutLed::openLedWhite()
{
	cout << "Call IrCutLed::openLedWhite()." << endl;
	gpioSetVal(PIN_LED_WHITE, 1);
	cout << "Call IrCutLed::openLedWhite() end." << endl;
	return 0;
}

/*-----------------------------------------------------------------------------
描--述：关闭LED 白光补光灯。
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
int IrCutLed::closeLedWhite()
{
	cout << "Call IrCutLed::closeLedWhite()." << endl;
	gpioSetVal(PIN_LED_WHITE, 0);
	cout << "Call IrCutLed::closeLedWhite() end." << endl;
	return 0;
}

/*-----------------------------------------------------------------------------
描--述：获取光敏电阻值。
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
int IrCutLed::getPhotoSensVal()
{
	cout << "Call IrCutLed::getPhotoSensVal()." << endl;
	
	int val = 0;
	gpioGetVal(PIN_PHOTO_SENS, &val);
	
	cout << "Call IrCutLed::getPhotoSensVal() end." << endl;
	return val;
}

