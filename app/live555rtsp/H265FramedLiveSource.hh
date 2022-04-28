#ifndef _H265FRAMEDLIVESOURCE_HH
#define _H265FRAMEDLIVESOURCE_HH

#include <FramedSource.hh>

class H265FramedLiveSource : public FramedSource
{
public:
	static H265FramedLiveSource *createNew(UsageEnvironment &env,
		unsigned preferredFrameSize = 0, unsigned playTimePerFrame = 0);
	unsigned maxFrameSize() const;

protected:
	// 成员函数createNew()会调用构造函数。
	H265FramedLiveSource(UsageEnvironment &env,
		unsigned preferredFrameSize, unsigned playTimePerFrame);
	~H265FramedLiveSource();		// 析构函数显示声明。

private:
	virtual void doGetNextFrame();	// 重定义基类的虚函数。
	//int TransportData(unsigned char* to, unsigned maxSize);
	//void doStopGettingFrames();

protected:
	int readSize = 0;			// 记录已读取的数据大小
	int databufSize = 0;		// 数据大小
};

#endif
