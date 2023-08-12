/*----------------------------------------------------------------
版权所有。
作者：
时间：2021.8.13
----------------------------------------------------------------*/

/*
修订：
2022.03  增加了锁机制，解决了多线程下同时操作push() 和pop() 的安全问题。

2023.04  修改了文件名，queue.h -> myqueue.h, 避免与其它开源项目冲突。
		修改了类名，Queue -> MyQueue.

2023.04	优化了push、pop、clear、os<< 的实现。
*/

#pragma once

/*
C++ 自带的队列，不具备定长功能。

使用定长数组充当队列，数组指针的head 和tail 表征队列的首元素、尾元素。
tail 和head 的减法运算，可以用来判断队列是否为空、是否为满、计算深度。
*/

/*
模板类的成员函数，如果使用了模板类型，则必须在类内定义。
"To konw why, let us look at the compilation process."
*/

#include <iostream>
#include <string.h>
#include <atomic>

void testQueue();

template <class T>
class MyQueue{
public:
	MyQueue();
	MyQueue(const unsigned int depth);
	~MyQueue();

	int push(const T *pData);		// 入队。注意队列满的实现。
	int push(const T *pData, int force);	// 入队。当队列满时，可通过force约束是否强制入栈。
	int pop(T *pData);				// 将队列头部数据取出，并从队列中移除。
	int relinquish();				// 舍弃队列头数据.
	int clear();					// 清空队列。
	bool isFull();					// 判断队列满。
	bool isEmpty();					// 判断队列空。
	unsigned int depth();			// 当前队列深度。
	unsigned int depths() const;	// 队列最大深度。
	int setQueueDepth(unsigned int depth);	// 设置队列深度。

	// 将运算符重载声明为友元函数的方法之一：类内声明模板别名.
	template <typename U>
	friend std::ostream &operator<<(std::ostream & os, MyQueue<U> &t);

private:
	T *pHead = NULL;				// 队列头，当前第一个元素的起始地址。
	T *pTail = NULL;				// 队列尾，下一次写入数据的位置。
	T *dataArray = NULL;
	unsigned int curDepth = 0;
	unsigned int queueMaxDepth = 0;

	std::atomic_flag lock = ATOMIC_FLAG_INIT;	// 用于解决多线程时push(), pop() 的安全问题。

	bool checkOK();
	void headIncrease();
	void headDecrease();
	void tailIncrease();
	void tailDecrease();
};

template <class T>
MyQueue<T>::MyQueue()
{
	dataArray = NULL;
	pHead = NULL;
	pTail = NULL;
	queueMaxDepth = 0;
	curDepth = 0;
}

template <class T>
MyQueue<T>::MyQueue(const unsigned int depth)
{
	setQueueDepth(depth);
}

template <class T>
MyQueue<T>::~MyQueue()
{
	std::cout << "Call MyQueue::~MyQueue()." << std::endl;
	if(NULL != dataArray)
	{
		free(dataArray);
		dataArray = NULL;
	}
	pHead = NULL;
	pTail = NULL;
	queueMaxDepth = 0;
	curDepth = 0;
	std::cout << "Call MyQueue::~MyQueue() end." << std::endl;
}

/*-----------------------------------------------------------------------------
描--述：设置队列深度。
参--数：
返回值：成功，返回0; 失败，返回-1.
注--意：参与构造。
-----------------------------------------------------------------------------*/
template <class T>
int MyQueue<T>::setQueueDepth(unsigned int depth)
{
	//std::cout << "sizeof(T) = " << sizeof(T) << std::endl;	
	//dataArray = new T(depth);	// 不清楚new 运算符申请的空间，为何不能用。

	if(NULL != dataArray)		// 说明已经初始化了dataArray.
	{
		std::cerr << "Fail to call setQueueDepth(). Queue has been initialized." << std::endl;
		return -1;
	}

	dataArray = (T *)malloc(sizeof(T) * depth);
	if(NULL == dataArray)
	{
		std::cerr << "Fail to call malloc(3) in setQueueDepth(). Space is not enough." << std::endl;
		pHead = NULL;
		pTail = NULL;
		queueMaxDepth = 0;
		curDepth = 0;
		return -1;
	}

	pHead = dataArray;
	pTail = dataArray;
	queueMaxDepth = depth;
	curDepth = 0;

	return depth;
}

/*-----------------------------------------------------------------------------
描--述：入队。
参--数：
返回值：队列状态异常，返回-1;
		正常入队，返回0.
注--意：
-----------------------------------------------------------------------------*/
template <class T>
int MyQueue<T>::push(const T *pData)
{
	while(lock.test_and_set(std::memory_order_acquire));	// 上锁
	//std::cout << "push" << std::endl;

	if(!checkOK())
	{
		//std::cout << "push end" << std::endl;
		lock.clear(std::memory_order_release);	// 解锁
		return -1;
	}

	while(curDepth == queueMaxDepth)
	{
		lock.clear(std::memory_order_release);	// 解锁
		while(lock.test_and_set(std::memory_order_acquire));	// 上锁
	}

	memcpy(pTail, pData, sizeof(T));
	tailIncrease();
	++curDepth;
	
	//std::cout << "curDepth = " << curDepth << std::endl;
	//std::cout << "push end" << std::endl;
	lock.clear(std::memory_order_release);		// 解锁
	return 0;
}

/*-----------------------------------------------------------------------------
描--述：强行放入队列。如果队列满，会挤掉队列头。
参--数：force, 是否强行入队。
返回值：队列状态异常，返回-1;
		队列满，强行入队，丢弃了队列头，返回1.
		队列满，不强行入队，未丢弃了队列头，返回2.
		正常入队，返回0.
注--意：如果队列满，会挤掉队列头。
-----------------------------------------------------------------------------*/
template <class T>
int MyQueue<T>::push(const T *pData, int force)
{
	while (lock.test_and_set(std::memory_order_acquire));	// 上锁
	//std::cout << "push" << std::endl;

	if(!checkOK())
	{
		//std::cout << "push end" << std::endl;
		lock.clear(std::memory_order_release);	// 解锁
		return -1;
	}

	if(curDepth < queueMaxDepth)
	{
		memcpy(pTail, pData, sizeof(T));
		tailIncrease();
		++curDepth;
		//std::cout << "push end" << std::endl;
		lock.clear(std::memory_order_release);	// 解锁
		return 0;
	}

	// curDepth == queueMaxDepth:
	if(force)		// 强行入队
	{
		//relinquish();
		headIncrease();
		memcpy(pTail, pData, sizeof(T));
		tailIncrease();
		//std::cout << "push end" << std::endl;
		lock.clear(std::memory_order_release);	// 解锁
		return 1;
	}
	else			// 不强行入队
	{
		//std::cout << "push end" << std::endl;
		lock.clear(std::memory_order_release);	// 解锁
		return -2;
	}
}

/*-----------------------------------------------------------------------------
描--述：出队。
参--数：
返回值：队列状态异常，返回-1;
		正常出队，返回0.
注--意：队列空则阻塞
-----------------------------------------------------------------------------*/
template <class T>
int MyQueue<T>::pop(T *pData)
{
	while (lock.test_and_set(std::memory_order_acquire));	// 上锁
	//std::cout << "pop" << std::endl;
	if(!checkOK())
	{
		//std::cout << "pop end" << std::endl;
		lock.clear(std::memory_order_release);	// 解锁
		return -1;
	}

	while(0 == curDepth)
	{
		lock.clear(std::memory_order_release);	// 解锁
		while (lock.test_and_set(std::memory_order_acquire));	// 上锁
	}

	memcpy(pData, pHead, sizeof(T));
	memset(pHead, 0, sizeof(T));
	headIncrease();
	--curDepth;

	//std::cout << "pop end" << std::endl;
	lock.clear(std::memory_order_release);	// 解锁
	return 0;
}

/*-----------------------------------------------------------------------------
描--述：清空队列。
参--数：
返回值：队列状态异常，返回-1;
		正常清队，返回0.
注--意：
-----------------------------------------------------------------------------*/
template <class T>
int MyQueue<T>::clear()
{
	while (lock.test_and_set(std::memory_order_acquire));	// 上锁
	//std::cout << "clear" << std::endl;
	if(!checkOK())
	{
		//std::cout << "clear end" << std::endl;
		lock.clear(std::memory_order_release);	// 解锁
		return -1;
	}

	pHead = pTail = dataArray;
	memset(dataArray, 0, sizeof(T) * queueMaxDepth);

	//std::cout << "clear end" << std::endl;
	lock.clear(std::memory_order_release);	// 解锁
	return 0;
}

/*-----------------------------------------------------------------------------
描--述：丢弃队列头元素。
参--数：
返回值：队列状态异常，返回-1;
		队列为空，返回-2;
		正常丢弃，返回0.
注--意：
-----------------------------------------------------------------------------*/
template <class T>
int MyQueue<T>::relinquish()
{
	while (lock.test_and_set(std::memory_order_acquire));	// 上锁
	//std::cout << "relinquish" << std::endl;

	if(!checkOK())
	{
		//std::cout << "relinquish end" << std::endl;
		lock.clear(std::memory_order_release);	// 解锁
		return -1;
	}

	if(0 == curDepth)
	{
		//std::cout << "relinquish end" << std::endl;
		lock.clear(std::memory_order_release);	// 解锁
		return -2;
	}

	--curDepth;
	memset(pHead, 0, sizeof(T));
	headIncrease();

	//std::cout << "relinquish end" << std::endl;
	lock.clear(std::memory_order_release);	// 解锁
	return 0;
}

/*-----------------------------------------------------------------------------
描--述：队列判空。
参--数：
返回值：空，返回true; 非空，返回false.
注--意：
-----------------------------------------------------------------------------*/
template <class T>
bool MyQueue<T>::isEmpty()
{
	while (lock.test_and_set(std::memory_order_acquire));	// 上锁
	bool bEmpty = false;
	if(0 == curDepth)
	{
		bEmpty = true;
	}
	else
	{
		bEmpty = false;
	}

	lock.clear(std::memory_order_release);	// 解锁
	return bEmpty;
}

/*-----------------------------------------------------------------------------
描--述：队列判满。
参--数：
返回值：满，返回true; 空，返回false.
注--意：
-----------------------------------------------------------------------------*/
template <class T>
bool MyQueue<T>::isFull()
{
	while (lock.test_and_set(std::memory_order_acquire));	// 上锁
	bool bFull = false;
	if(curDepth == queueMaxDepth)
	{
		bFull = true;
	}
	else
	{
		bFull = false;
	}

	lock.clear(std::memory_order_release);	// 解锁
	return bFull;
}

/*-----------------------------------------------------------------------------
描--述：获取队列当前深度。
参--数：
返回值：返回队列当前深度。
注--意：
-----------------------------------------------------------------------------*/
template <class T>
unsigned int MyQueue<T>::depth()
{
	while (lock.test_and_set(std::memory_order_acquire));	// 上锁
	unsigned int depth = 0;
	depth = curDepth;
	lock.clear(std::memory_order_release);	// 解锁
	return depth;
}

/*-----------------------------------------------------------------------------
描--述：获取队列最大深度。
参--数：
返回值：返回队列最大深度。
注--意：
-----------------------------------------------------------------------------*/
template <class T>
unsigned int MyQueue<T>::depths() const
{
	//while (lock.test_and_set(std::memory_order_acquire));	// 上锁
	unsigned int depth = 0;
	depth = queueMaxDepth;
	//lock.clear(std::memory_order_release);	// 解锁
	return depth;
}

/*-----------------------------------------------------------------------------
描--述：输出流运算符重载。
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
template <typename T>
std::ostream &operator<<(std::ostream& os, MyQueue<T>& t)
{
	int i = 0;
	T *move = NULL;
	unsigned int curDepth = 0;

	while (t.lock.test_and_set(std::memory_order_acquire));	// 上锁
	move = t.pHead;
	curDepth = t.curDepth;
	
	for(i = 0; i < curDepth; ++i)
	{
		#if 0
		os << *(u.pHead + i ) << std::endl;
		
		++move;
		if(u.dataArray + u.depths() == move)
		{
			move = u.dataArray;
		}
		#else
		os << *move << std::endl;
		++move;
		if(t.dataArray + t.queueMaxDepth == move)
		{
			move = t.dataArray;
		}
		#endif
	}

	#if 0
	for(i = 0; i < t.queueMaxDepth; ++i)
	{
		os << *(t.dataArray + i) << std::endl;
	}
	#endif
	
	t.lock.clear(std::memory_order_release);	// 解锁
	return os;
}

/*-----------------------------------------------------------------------------
描--述：队列状态自检。
参--数：
返回值：自检成功，返回true; 异常，返回false.
注--意：
-----------------------------------------------------------------------------*/
template <class T>
bool MyQueue<T>::checkOK()
{
	if(NULL != dataArray && NULL != pHead && NULL != pTail && (curDepth <= queueMaxDepth))
	{
		return true;
	}

	return false;
}

/*-----------------------------------------------------------------------------
描--述：队列头自增。
参--数：
返回值：自检成功，返回true; 异常，返回false.
注--意：
-----------------------------------------------------------------------------*/
template <class T>
void MyQueue<T>::headIncrease()
{
	if(!checkOK())
	{
		return;
	}

	++pHead;
	if(pHead == dataArray + queueMaxDepth)
	{
		pHead = dataArray;
	}
}

/*-----------------------------------------------------------------------------
描--述：队列尾自减。
参--数：
返回值：
注--意：
-----------------------------------------------------------------------------*/
template <class T>
void MyQueue<T>::tailIncrease()
{
	if(!checkOK())
	{
		return;
	}

	++pTail;
	if(pTail == dataArray + queueMaxDepth)
	{
		pTail = dataArray;
	}
}

