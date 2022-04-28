/*---------------------------------------------------------------- 
xxx版权所有。
作者：
时间：2020.7.10
----------------------------------------------------------------*/

#include "ircut.h"
#include <thread>

using namespace std;

IrCut::IrCut()
{
	resetFilter();
}

IrCut::~IrCut()
{
}

IrCut* IrCut::getInstance()
{
	static IrCut irCut;
	return &irCut;
}

bool IrCut::bOpened()
{
	return bOpen;
}

int IrCut::openFilter()
{
	gpioSetVal(PIN_AIN, 1);
	std::this_thread::sleep_for(std::chrono::microseconds(400 * 1000));
	gpioSetVal(PIN_AIN, 0);
	bOpen = true;

	return 0;
}

int IrCut::closeFilter()
{
	gpioSetVal(PIN_AIN, 0);
	gpioSetVal(PIN_BIN, 1);
	std::this_thread::sleep_for(std::chrono::microseconds(400 * 1000));
	gpioSetVal(PIN_BIN, 0);
	bOpen = false;

	return 0;
}

int IrCut::resetFilter()
{
	openFilter();
	closeFilter();
	bOpen = false;

	return 0;
}

int IrCut::gpioSetVal(unsigned int gpioIndex, unsigned int val)
{
	//cout << "Call IrCut::gpioSetVal()." << endl;

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

	//cout << "Call IrCut::gpioSetVal() end." << endl;
	return 0;
}

