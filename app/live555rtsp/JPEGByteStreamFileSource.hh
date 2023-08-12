/*---------------------------------------------------------------- 
xxx 版权所有。
作者：
时间：2023.7.7
----------------------------------------------------------------*/

#pragma once

#include "FramedSource.hh"

class JpegByteStreamFileSource: public FramedSource{
public:
	static JpegByteStreamFileSource* createNew(UsageEnvironment& env, const char *filePath);

protected:
	JpegByteStreamFileSource(UsageEnvironment& env, const char *filePath);
	virtual ~JpegByteStreamFileSource();

	virtual unsigned int maxFrameSize() const;

private:
	static const unsigned int maxVideoFrameSize = 1 * 1024 * 1024;

	// redefined virtual functions:
	virtual void doGetNextFrame();
	virtual void doStopGettingFrames();
	static  void doGetNextFrameStatic(FramedSource* source);

protected:
	int mFd = -1;
};

