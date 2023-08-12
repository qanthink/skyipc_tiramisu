/*---------------------------------------------------------------- 
xxx 版权所有。
作者：
时间：2023.7.7
----------------------------------------------------------------*/

#include "JPEGByteStreamFileSource.hh"
#include <iostream>
#include <fcntl.h>

using namespace std;

JpegByteStreamFileSource*
JpegByteStreamFileSource::createNew(UsageEnvironment& env, const char *filePath) 
{
	JpegByteStreamFileSource* newSource = new JpegByteStreamFileSource(env, filePath);

	return newSource;
}

JpegByteStreamFileSource::JpegByteStreamFileSource(UsageEnvironment& env, const char *filePath)
	: FramedSource(env)
{
	int fd = -1;
	fd = open(filePath, O_RDONLY);
	if(-1 == fd)
	{
		cerr << "Fail to call open(2) in JpegByteStreamFileSource(). FilePath = " << filePath << strerror(errno) << endl;
		return;
	}
	
	cout << "In JpegByteStreamFileSource(): Success to open file " << filePath << endl;
	mFd = fd;
}

JpegByteStreamFileSource::~JpegByteStreamFileSource() 
{
	if(-1 != mFd)
	{
		cout << "In JpegByteStreamFileSource(): close file." << endl;
		//close(mFd);
		::close(mFd);		// 作用域解析运算符
		mFd = -1;
	}
}

unsigned int JpegByteStreamFileSource::maxFrameSize() const
{
	return maxVideoFrameSize;
}

void JpegByteStreamFileSource::doGetNextFrame()
{
	/* 重点 */
	//int iFrameSize = m_ReadStreamFun(fTo, fMaxSize, &fPresentationTime);
	
	if(-1 == mFd)
	{
		cerr << "Fail to call JpegByteStreamFileSource::doGetNextFrame(), bad file descriptor." << endl;
		return;
	}
	
	int readBytes = 0;
	readBytes = read(mFd, fTo, fMaxSize);
	if(-1 == readBytes)
	{
		cerr << "Fail to call read(2) in Fail to call JpegByteStreamFileSource::doGetNextFrame(). "
			<< strerror(errno) << endl;
		handleClosure();
		return;
	}

	if(0 == readBytes)
	{
		nextTask() = envir().taskScheduler().scheduleDelayedTask(1000,
					(TaskFunc*)JpegByteStreamFileSource::doGetNextFrameStatic, this);
		return;
	}

	fFrameSize = readBytes;
	struct timeval oldPresentationTime = fPresentationTime;
	if(((fPresentationTime.tv_sec == 0) && (fPresentationTime.tv_usec == 0))
			|| ((oldPresentationTime.tv_sec == fPresentationTime.tv_sec) && (oldPresentationTime.tv_usec == fPresentationTime.tv_usec)))
	{
		gettimeofday(&fPresentationTime, NULL);
	}

	FramedSource::afterGetting(this);
}

void JpegByteStreamFileSource::doStopGettingFrames() 
{
	envir().taskScheduler().unscheduleDelayedTask(nextTask());
}

void JpegByteStreamFileSource::doGetNextFrameStatic(FramedSource* source)
{
	source->doGetNextFrame();
}


