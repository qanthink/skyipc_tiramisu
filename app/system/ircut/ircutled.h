/*---------------------------------------------------------------- 
xxx版权所有。
作者：
时间：2020.7.10
----------------------------------------------------------------*/

/*
	此文件用于控制IrCut、Led 补光灯。
	
	切IRCUT逻辑：
	白天切晚上：加载夜视IQ -> 关IRCUT -> 开红外灯。
	晚上到白天：关红外灯 -> 开IRCUT -> 加载白天IQ.
	
	白天切晚上，图像瞬间偏紫，解决：
	加载IQ -> 延迟500ms -> 切IRCUT
*/

#pragma once

class IrCutLed
{
public:
	static IrCutLed* getInstance();

private:
	IrCutLed();
	~IrCutLed();
	IrCutLed(const IrCutLed&);
	IrCutLed& operator=(const IrCutLed&);

public:
	int openFilter();					// 打开滤波片
	int closeFilter();					// 关闭滤波片
	int resetFilter();					// 滤波片复位

	int openLedIr();					// 打开红外灯
	int closeLedIr();					// 关闭红外灯
	
	int openLedWhite();					// 打开白光灯
	int closeLedWhite();				// 关闭白光灯

	int getPhotoSensVal();				// 获取光敏电阻值

private:
	int gpioGetVal(unsigned int gpioIndex, int *pVal);
	int gpioSetVal(unsigned int gpioIndex, int val);

	const int PIN_IRCUT_AIN = 78;		// PinNum = 15. ircut
	const int PIN_IRCUT_BIN = 79;		// PinNum = 14. ircut

	const int PIN_LED_IR = 16;			// PinNum = 45; 红外灯
	const int PIN_LED_WHITE = 76;		// PinNum = 32; 白光灯
	
	const int PIN_PHOTO_SENS = 80;		// PinNum = 13; 光敏电阻
};
