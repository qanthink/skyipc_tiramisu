/*---------------------------------------------------------------- 
版权所有。
作者：
时间：2021.8.13
----------------------------------------------------------------*/

/*
修订：

2023.04  增加了空间名引用，using namespace std;
*/

#include <iostream>
#include "myqueue.h"

using namespace std;

void testQueue()
{
#if 1
	int in = 0;
	int out = 0;
	const unsigned int queueDepths = 3;
	MyQueue<int> intQueue(queueDepths);
	cout << __LINE__ << ": is empty() " << intQueue.isEmpty() << endl;
	cout << __LINE__ << ": is full() " << intQueue.isFull() << endl;
	cout << __LINE__ << ": is depth() " << intQueue.depth() << endl;
	cout << __LINE__ << ": is maxDepth() " << intQueue.depths() << endl;
#if 1
	in = 1;
	intQueue.push(&in);
	cout << __LINE__ << ": is empty() " << intQueue.isEmpty() << endl;
	cout << __LINE__ << ": is full() " << intQueue.isFull() << endl;
	cout << __LINE__ << ": is depth() " << intQueue.depth() << endl;
	cout << __LINE__ << ": is maxDepth() " << intQueue.depths() << endl;
	
	intQueue.pop(&out);
	cout << __LINE__ << ": out " << out << endl;
	cout << __LINE__ << ": is empty() " << intQueue.isEmpty() << endl;
	cout << __LINE__ << ": is full() " << intQueue.isFull() << endl;
	cout << __LINE__ << ": is depth() " << intQueue.depth() << endl;
	cout << __LINE__ << ": is maxDepth() " << intQueue.depths() << endl;
#endif
	in = 5;
	intQueue.push(&in);
	cout << intQueue;
	cout << __LINE__ << ": is empty() " << intQueue.isEmpty() << endl;
	cout << __LINE__ << ": is full() " << intQueue.isFull() << endl;
	cout << __LINE__ << ": is depth() " << intQueue.depth() << endl;
	cout << __LINE__ << ": is maxDepth() " << intQueue.depths() << endl;
	in = 6;
	intQueue.push(&in);
	cout << intQueue;
	cout << __LINE__ << ": is empty() " << intQueue.isEmpty() << endl;
	cout << __LINE__ << ": is full() " << intQueue.isFull() << endl;
	cout << __LINE__ << ": is depth() " << intQueue.depth() << endl;
	cout << __LINE__ << ": is maxDepth() " << intQueue.depths() << endl;
	in = 7;
	intQueue.push(&in);
	cout << intQueue;
	cout << __LINE__ << ": is empty() " << intQueue.isEmpty() << endl;
	cout << __LINE__ << ": is full() " << intQueue.isFull() << endl;
	cout << __LINE__ << ": is depth() " << intQueue.depth() << endl;
	cout << __LINE__ << ": is maxDepth() " << intQueue.depths() << endl;
	in = 8;
	intQueue.push(&in);
	cout << intQueue;
	cout << __LINE__ << ": is empty() " << intQueue.isEmpty() << endl;
	cout << __LINE__ << ": is full() " << intQueue.isFull() << endl;
	cout << __LINE__ << ": is depth() " << intQueue.depth() << endl;
	cout << __LINE__ << ": is maxDepth() " << intQueue.depths() << endl;

	intQueue.pop(&out);
	cout << intQueue;
	cout << __LINE__ << ": out " << out << endl;
	cout << __LINE__ << ": is empty() " << intQueue.isEmpty() << endl;
	cout << __LINE__ << ": is full() " << intQueue.isFull() << endl;
	cout << __LINE__ << ": is depth() " << intQueue.depth() << endl;
	cout << __LINE__ << ": is maxDepth() " << intQueue.depths() << endl;
	intQueue.pop(&out);
	cout << intQueue;
	cout << __LINE__ << ": out " << out << endl;
	cout << __LINE__ << ": is empty() " << intQueue.isEmpty() << endl;
	cout << __LINE__ << ": is full() " << intQueue.isFull() << endl;
	cout << __LINE__ << ": is depth() " << intQueue.depth() << endl;
	cout << __LINE__ << ": is maxDepth() " << intQueue.depths() << endl;

	intQueue.pop(&out);
	cout << intQueue;
	cout << __LINE__ << ": out " << out << endl;
	cout << __LINE__ << ": is empty() " << intQueue.isEmpty() << endl;
	cout << __LINE__ << ": is full() " << intQueue.isFull() << endl;
	cout << __LINE__ << ": is depth() " << intQueue.depth() << endl;
	cout << __LINE__ << ": is maxDepth() " << intQueue.depths() << endl;

	cout << __LINE__ << ": return " << intQueue.pop(&out) << endl;
	cout << __LINE__ << ": out " << out << endl;
	cout << __LINE__ << ": is empty() " << intQueue.isEmpty() << endl;
	cout << __LINE__ << ": is full() " << intQueue.isFull() << endl;
	cout << __LINE__ << ": is depth() " << intQueue.depth() << endl;
	cout << __LINE__ << ": is maxDepth() " << intQueue.depths() << endl;

	intQueue.clear();
	cout << intQueue;
	cout << __LINE__ << ": is empty() " << intQueue.isEmpty() << endl;
	cout << __LINE__ << ": is full() " << intQueue.isFull() << endl;
	cout << __LINE__ << ": is depth() " << intQueue.depth() << endl;
	cout << __LINE__ << ": is maxDepth() " << intQueue.depths() << endl;
#endif
}

