/*---------------------------------------------------------------- 
xxx版权所有。
作者：
时间：2021.8.13
----------------------------------------------------------------*/

/*
免责声明：
*/

#pragma once

/*
设计思路：
*/

#include "picture.h"

// Panel GPIO Operation
#if 0	// pudding
#define PANEL_GPIO_DC 	47	//数据与指令
#define PANEL_GPIO_BLK 	108	//背光
#define PANEL_GPIO_RES 	46	//复位
#else	// ispahan
#define PANEL_GPIO_DC 	63	//数据与指令
#define PANEL_GPIO_BLK 	64	//背光
#define PANEL_GPIO_RES 	62	//复位
#endif

#define USE_HORIZONTAL 	0	// 旋转0-1代表90°，2-3代表180°
#define PANEL_WIDTH 	240
#define PANEL_HEIGHT 	240

#define FONT_SIZE_12 12
#define FONT_SIZE_16 16
#define FONT_SIZE_24 24
#define FONT_SIZE_32 32

#define SPIPANEL_MAX(x, y) (((x) >= (y)) ? ((x)) : ((y)))

//画笔颜色
#define SPIPANEL_COLOR_WHITE		0xFFFF
#define SPIPANEL_COLOR_BLACK		0x0000
#define SPIPANEL_COLOR_BLUE			0x001F
#define SPIPANEL_COLOR_BRED			0XF81F
#define SPIPANEL_COLOR_GRED			0XFFE0
#define SPIPANEL_COLOR_GBLUE		0X07FF
#define SPIPANEL_COLOR_RED			0xF800
#define SPIPANEL_COLOR_MAGENTA		0xF81F
#define SPIPANEL_COLOR_GREEN		0x07E0
#define SPIPANEL_COLOR_CYAN			0x7FFF
#define SPIPANEL_COLOR_YELLOW		0xFFE0
#define SPIPANEL_COLOR_BROWN		0XBC40 //棕色
#define SPIPANEL_COLOR_BRRED		0XFC07 //棕红色
#define SPIPANEL_COLOR_GRAY			0X8430 //灰色
#define SPIPANEL_COLOR_DARKBLUE		0X01CF //深蓝色
#define SPIPANEL_COLOR_LIGHTBLUE	0X7D7C //浅蓝色
#define SPIPANEL_COLOR_GRAYBLUE		0X5458 //灰蓝色
#define SPIPANEL_COLOR_LIGHTGREEN	0X841F //浅绿色
#define SPIPANEL_COLOR_LGRAY		0XC618 //浅灰色(PANNEL),窗体背景色
#define SPIPANEL_COLOR_LGRAYBLUE	0XA651 //浅灰蓝色(中间层颜色)
#define SPIPANEL_COLOR_LBBLUE		0X2B12 //浅棕蓝色(选择条目的反色)

class SpiPanel{
public:
	static SpiPanel* getInstance();

	int enable();
	int disable();

	// panel 基本点线功能
	int panelFill(unsigned short x0, unsigned short y0, unsigned short x1, unsigned short y1, unsigned short color);	//在指定区域填充颜色
	int panelDrawPoint(unsigned short x, unsigned short y, unsigned short color);	//在指定位置画一个点
	int panelDrawLine(unsigned short x0, unsigned short y0, unsigned short x1, unsigned short y1, unsigned short color);	//在指定位置画一条线
	int panelDrawRectangle(unsigned short x0, unsigned short y0, unsigned short x1, unsigned short y1, unsigned short color);//在指定位置画一个矩形
	int panelDrawCircle(unsigned short x, unsigned short y, unsigned char r, unsigned short color);	//在指定位置画一个圆

	// panel 显示数字+字符
	int panelShowChar(unsigned short x, unsigned short y, unsigned char ch, unsigned short fc, unsigned short bc, unsigned short sizey, bool bCoverMode);			//显示一个字符
	int panelShowString(unsigned short x, unsigned short y, const char *pText, unsigned short fc, unsigned short bc, unsigned short sizey, bool bCoverMode);	//显示字符串
	int panelShowIntNum(unsigned short x, unsigned short y, long long int num, unsigned short fc, unsigned short bc, unsigned short sizey, bool bCoverMode);	//显示整数变量
	int panelShowFloatNum(unsigned short x, unsigned short y, double num, unsigned short fc, unsigned short bc, unsigned short sizey, bool bCoverMode);				//显示两位小数变量

	// panel 显示中文汉字
	int panelShowChineseFont(unsigned short x, unsigned short y, const char *pFont, unsigned short fc, unsigned short bc, unsigned char fontSize, unsigned char bCoverMode);//显示单个12x12汉字
	int panelShowChineseText(unsigned short x, unsigned short y, const char *pText, unsigned short fc, unsigned short bc, unsigned char fontSize, unsigned char bCoverMode);//显示汉字串

	// panel 显示文本，含中英文
	int panelShowText(unsigned short x, unsigned short y, const char *pText, unsigned short fc, unsigned short bc, unsigned char fontSize, unsigned char bCoverMode);

	// panel 显示图片
	int panelShowPicture(unsigned short x, unsigned short y, unsigned short width, unsigned short height, const unsigned char *pPicture);//显示图片

private:
	SpiPanel();
	~SpiPanel();
	SpiPanel(const SpiPanel&);
	SpiPanel& operator=(const SpiPanel&);

	// SPI Device Operation. Data transfer
	int fdSpiDev = -1;
	const char *spiDevPath = "/dev/spidev0.0";
	const unsigned int mode = 2;	// 本程序用到的SPI屏，设置CPOL=1，CPHA=?, 所以mode = 2或3
	const unsigned int bits = 8;	// 8bits读写，MSB first
	const unsigned int speed = 24 * 1000 * 1000;	// 设置传输速度，N * 1000 * 1000 = N MHz
	int spiDevOpen();
	int spiDevClose();
	int spiDevRead(void *dataBuf, unsigned long dataBufLen);
	int spiDevWrite(const void *dataBuf, unsigned long dataBufLen);

	// GPIO Operation
	int fdGpioValRES = -1;
	int fdGpioValDC = -1;
	int fdGpioValBLK = -1;
	int gpioInit();
	int gpioDeinit();
	int gpioSetVal(unsigned int gpioIndex, unsigned int val);

	// Panel Operation
	bool bEnable;
	int panelInit();
	int panelDeinit();
	int panelWriteBus(const void *dataBuf, unsigned long dataBufLen);
	int panelWriteCmd(const void *dataBuf, unsigned long dataBufLen);
	int panelWriteData(const void *dataBuf, unsigned long dataBufLen);
	int panelSetAddress(unsigned short x0, unsigned short y0, unsigned short x1, unsigned short y1);
};

