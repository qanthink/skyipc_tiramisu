/*---------------------------------------------------------------- 
版权所有。
作者：
时间：2023.6.11
----------------------------------------------------------------*/

/*
修订：

2023.04  
*/

#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>

#include "myfifo.h"

using namespace std;

/* ---------------------------------------------------------------------------
功--能：建立有名管道并打开，设置缓冲区大小。
参--数：
返回值：
注--意：缓冲区大小介于页面大小和最大上限之间。页面大小通常为4K.
-----------------------------------------------------------------------------*/
MyNameFifo::MyNameFifo(const char *pathName, size_t fifoSize)
{
	cout << "Call MyNameFifo::MyNameFifo()." << endl;
	if(-1 != mFdRead || -1 != mFdWrite)
	{
		cerr << "Fail to call MyNameFifo::MyNameFifo(). Fifo exists." << endl;
		return;
	}

	mPathName = pathName;

	// step1: 建立有名管道。
	int ret = -1;
	mode_t mode = 0777;
	ret = mkfifo(pathName, mode);

	// 如果管道存在，则继续。
	if((-1 == ret) && (EEXIST == errno))
	{
		cerr << "In MyNameFifo::MyNameFifo(), fifo exists." << endl;
	}
	// 如果创建管道失败，则退出。
	else if(-1 == ret)
	{
		cerr << "Fai to call mkfifo(3) in MyNameFifo::MyNameFifo()." << strerror(errno) << endl;
		return;
	}
	cout << "Success to create named fifo." << endl;

	#if 1
	// step2: 以只读、只写方式，先后打开有名管道。只读打开时，设置O_NONBLOCK.
	mFdRead = open(pathName, O_RDONLY | O_NONBLOCK); // 必须以O_WRONLY 方式打开，否则写操作不能被阻塞。
	if(-1 == mFdRead)
	{
		cerr << "Fai to call open(2) with RD in MyNameFifo::MyNameFifo()." << strerror(errno) << endl;
	}
	else
	{
		cout << "Success to open fifo with RD, fd = " << mFdRead << endl;
	}
	
	mFdWrite = open(pathName, O_WRONLY | O_NONBLOCK);	// 必须以O_WRONLY 方式打开，否则写操作不能被阻塞。
	if(-1 == mFdWrite)
	{
		cerr << "Fai to call open(2) with WR in MyNameFifo::MyNameFifo()." << strerror(errno) << endl;
	}
	else
	{
		cout << "Success to open fifo with WR, fd = " << mFdWrite << endl;
	}

	close(mFdRead);
	mFdRead = -1;
	#endif

	// step3: 设置管道缓冲区。
	ret = fcntl(mFdWrite, F_SETPIPE_SZ, fifoSize);
	if(-1 == ret)
	{
		cerr << "Fail to call fcntl(2) with F_SETPIPE_SZ in MyNameFifo::MyNameFifo()." << strerror(errno) << endl;
		return;
	}
	else
	{
		cout << "Ret = " << ret << ". After fcntl(2) with F_SETPIPE_SZ." << endl;
	}

	cout << "Call MyNameFifo::MyNameFifo() end." << endl;
}

/* ---------------------------------------------------------------------------
功--能：析构有名管道。
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
MyNameFifo::~MyNameFifo()
{
	cout << "Call MyNameFifo::~MyNameFifo()." << endl;
	
	if(-1 != mFdWrite)
	{
		close(mFdWrite);
		mFdWrite = -1;
	}

	if(-1 != mFdRead)
	{
		close(mFdRead);
		mFdRead = -1;
	}

	if(NULL != mPathName)
	{
		unlink(mPathName);
		mPathName = NULL;
	}

	cout << "Call MyNameFifo::~MyNameFifo() end." << endl;
}


