/*---------------------------------------------------------------- 
xxx版权所有。
作者：
时间：2020.7.10
----------------------------------------------------------------*/

#pragma once

#include "mi_vpe.h"

/*
VPE 流程：VIF -> VPE(1 个chanel) -> set port mode(创建多个输出port) 
		-> enable port -> start port.
*/

/*
本示例默认创建两个port, port 1作主码流，port 2作子码流。
*/

class Vpe{
public:
	const static MI_VPE_PORT vpeInputPort = 0;
	const static MI_VPE_PORT vpeMainPort = 1;
	const static MI_VPE_PORT vpeSubPort = 2;
	const static MI_VPE_PORT vpeJpegPort = 3;

// ===========================================================
	static Vpe* getInstance();
	MI_S32 enable();
	MI_S32 disable();

// ===========================================================
	MI_S32 createMainPort(MI_VPE_PORT vpePort);
	MI_S32 createSubPort(MI_VPE_PORT vpePort);
	MI_S32 createJpegPort(MI_VPE_PORT vpePort);
	
// ===========================================================
	MI_S32 setChnOutputPortDepth(MI_VPE_PORT vpePort, MI_U32 u32UserFrameDepth, MI_U32 u32BufQueueDepth);

	MI_S32 createChannel();
	MI_S32 destroyChannel();
	
	MI_S32 startChannel();
	MI_S32 stopChannel();
	
	MI_S32 setChannelParam();
	
	MI_S32 enablePort(MI_VPE_PORT vpePort);
	MI_S32 disablePort(MI_VPE_PORT vpePort);
	
	MI_S32 setPortMode(MI_VPE_PORT vpePort, MI_VPE_PortMode_t *pstVpeMode);

	MI_S32 setChannelCrop(unsigned int x, unsigned int y, unsigned int w, unsigned int h);
	MI_S32 setPortCropScale(MI_VPE_PORT vpePort, unsigned int crX, unsigned int crY, 
						unsigned int crW, unsigned int crH, unsigned int outW, unsigned int outH);

private:
	Vpe();
	~Vpe();
	Vpe(const Vpe&);
	Vpe& operator=(const Vpe&);

	bool bEnable;
	const static MI_VPE_CHANNEL vpeCh = 0;
	const static MI_U32 u32MaxVpePortID = 3;
};

