/*----------------------------------------------------------------
版权所有。
作者：
时间：2021.8.13
----------------------------------------------------------------*/

/*
修订：
2022.03  增加了锁机制，解决了多线程下同时操作push() 和pop() 的安全问题。
*/

#ifndef __QUEUE_H__
#define __QUEUE_H__

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
class Queue{
public:
	Queue();
	Queue(const unsigned int depth);
	~Queue();

	int push(const T *pData);		// 压栈。如果队列满，会丢弃队列头，再入栈。
	int push(const T *pData, int force);	// 压栈。当队列满时，可通过force约束是否强制入栈。
	int pop(T *pData);				// 将队列头部数据取出，并从队列中移除。
	int relinquish();				// 舍弃队列头数据.
	int clear();					// 清空队列。
	bool isFull();					// 判断队列满。
	bool isEmpty();					// 判断队列空。
	unsigned int depth() const;		// 当前队列深度。
	unsigned int depths() const;	// 队列总深度。成功，返回队列深度；失败，返回-1.
	int setQueueDepth(unsigned int depth);	// 设置队列深度

	// 将运算符重载声明为友元函数的方法之一：类内声明模板别名.
	template <typename U>
	friend std::ostream &operator<<(std::ostream & os, const Queue<U> &t);

private:
	T *pHead = NULL;
	T *pTail = NULL;
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
Queue<T>::Queue()
{
	dataArray = NULL;
	pHead = NULL;
	pTail = NULL;
	queueMaxDepth = 0;
	curDepth = 0;
}

template <class T>
Queue<T>::Queue(const unsigned int depth)
{
	setQueueDepth(depth);
}

template <class T>
Queue<T>::~Queue()
{
	std::cout << "Call Queue::~Queue()." << std::endl;
	if(NULL != dataArray)
	{
		free(dataArray);
		dataArray = NULL;
	}
	pHead = NULL;
	pTail = NULL;
	queueMaxDepth = 0;
	curDepth = 0;
	std::cout << "Call Queue::~Queue() end." << std::endl;
}

template <class T>
int Queue<T>::setQueueDepth(unsigned int depth)
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

template <class T>
int Queue<T>::push(const T *pData)
{
	while (lock.test_and_set(std::memory_order_acquire));	// 上锁
	//std::cout << "push" << std::endl;

	if(!checkOK())
	{
		//std::cout << "push end" << std::endl;
		lock.clear(std::memory_order_release);	// 解锁
		return -1;
	}

	bool bRelinQuish = false;
	if(curDepth == queueMaxDepth)
	{
		--curDepth;
		memset(pHead, 0, sizeof(T));
		headIncrease();
		bRelinQuish = true;
	}

	memcpy(pTail, pData, sizeof(T));
	tailIncrease();
	++curDepth;
	//std::cout << "curDepth = " << curDepth << std::endl;

	//std::cout << "push end" << std::endl;
	lock.clear(std::memory_order_release);	// 解锁
	return bRelinQuish;
}

template <class T>
int Queue<T>::push(const T *pData, int force)
{
	while (lock.test_and_set(std::memory_order_acquire));	// 上锁
	//std::cout << "push" << std::endl;

	if(!checkOK())
	{
		//std::cout << "push end" << std::endl;
		lock.clear(std::memory_order_release);	// 解锁
		return -1;
	}

	if(curDepth == queueMaxDepth)
	{
		memcpy(pTail, pData, sizeof(T));
		tailIncrease();
		++curDepth;
		//std::cout << "push end" << std::endl;
		lock.clear(std::memory_order_release);	// 解锁
		return 0;
	}

	if(force)
	{
		relinquish();
		memcpy(pTail, pData, sizeof(T));
		tailIncrease();
		++curDepth;
		//std::cout << "push end" << std::endl;
		lock.clear(std::memory_order_release);	// 解锁
		return 0;
	}
	else
	{
		//std::cout << "push end" << std::endl;
		lock.clear(std::memory_order_release);	// 解锁
		return -2;
	}
}

template <class T>
int Queue<T>::pop(T *pData)
{
	while (lock.test_and_set(std::memory_order_acquire));	// 上锁
	//std::cout << "pop" << std::endl;
	if(!checkOK())
	{
		//std::cout << "pop end" << std::endl;
		lock.clear(std::memory_order_release);	// 解锁
		return -1;
	}

	if(0 == curDepth)
	{
		//std::cout << "pop end" << std::endl;
		lock.clear(std::memory_order_release);	// 解锁
		return -2;
	}

	memcpy(pData, pHead, sizeof(T));
	memset(pHead, 0, sizeof(T));
	headIncrease();
	--curDepth;

	//std::cout << "pop end" << std::endl;
	lock.clear(std::memory_order_release);	// 解锁
	return 0;
}

template <class T>
int Queue<T>::clear()
{
	while (lock.test_and_set(std::memory_order_acquire));	// 上锁
	//std::cout << "clear" << std::endl;
	if(!checkOK())
	{
		//std::cout << "clear end" << std::endl;
		lock.clear(std::memory_order_release);	// 解锁
		return -1;
	}

	while(0 != curDepth)
	{
		--curDepth;
		memset(pHead, 0, sizeof(T));
		headIncrease();
	}

	memset(dataArray, 0, sizeof(T) * queueMaxDepth);

	//std::cout << "clear end" << std::endl;
	lock.clear(std::memory_order_release);	// 解锁
	return 0;
}

template <class T>
int Queue<T>::relinquish()
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

template <class T>
bool Queue<T>::isEmpty()
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

template <class T>
bool Queue<T>::isFull()
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


template <class T>
unsigned int Queue<T>::depth() const
{
	//while (lock.test_and_set(std::memory_order_acquire));	// 上锁
	unsigned int depth = 0;
	depth = curDepth;
	//lock.clear(std::memory_order_release);	// 解锁
	return depth;
}

template <class T>
unsigned int Queue<T>::depths() const
{
	//while (lock.test_and_set(std::memory_order_acquire));	// 上锁
	unsigned int depth = 0;
	depth = queueMaxDepth;
	//lock.clear(std::memory_order_release);	// 解锁
	return depth;
}

#if 1
template <typename U>
std::ostream &operator<<(std::ostream& os, const Queue<U>& u)
{
	int i = 0;
	U *move = NULL;
	unsigned int curDepth = 0;
	
	move = u.pHead;
	curDepth = u.depth();
	
	for(i = 0; i < curDepth; ++i)
	{
		os << *(u.pHead + i ) << std::endl;
		
		move++;
		if(u.dataArray + u.depths() == move)
		{
			move = u.dataArray;
		}
	}
	
	return os;
}
#endif

template <class T>
bool Queue<T>::checkOK()
{
	if(NULL != dataArray && NULL != pHead && NULL != pTail && (curDepth <= queueMaxDepth))
	{
		return true;
	}

	return false;
}

template <class T>
void Queue<T>::headIncrease()
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

template <class T>
void Queue<T>::tailIncrease()
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

#endif
