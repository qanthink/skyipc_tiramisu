/*----------------------------------------------------------------
版权所有。
作者：
时间：2023.6.11
----------------------------------------------------------------*/

/*
修订：
2022.03  
*/

/*
注意：
1. mkfifo() 之后，需要另一个进程先open with RD_ONLY, 本进程才能open with WR_ONLY.
否则open with WR_ONLY 会阻塞。且open with RD_ONLY时，加O_NONBLOCK.
*/

#pragma once

class MyNameFifo{
public:
	MyNameFifo(const char *pathName, size_t fifoSize);
	~MyNameFifo();

	const int getFdRead() const {return mFdRead;};
	const int getFdWrite() const {return mFdWrite;};

private:
	int mFdRead = -1;
	int mFdWrite = -1;
	const char *mPathName = NULL;
};

