/*---------------------------------------------------------------- 
xxx版权所有。
作者：
时间：2020.7.10
----------------------------------------------------------------*/

/*
	此文件用于控制IrCut 滤波片的打开或关闭。
	夜间打开，白天关闭。
*/

#pragma once

class IrCut
{
public:
	static IrCut* getInstance();
	
	bool bOpened();						// 滤波片是否打开
	
	int openFilter();					// 打开滤波片
	int closeFilter();					// 关闭滤波片
	int resetFilter();					// 滤波片复位

private:
	IrCut();
	~IrCut();
	IrCut(const IrCut&);
	IrCut& operator=(const IrCut&);

	int gpioSetVal(unsigned int gpioIndex, unsigned int val);

	const int PIN_AIN = 78;		// SSC333/335/337
	const int PIN_BIN = 79;
	
	bool bOpen = false;
};
