/*---------------------------------------------------------------- 
版权所有。
作者：
时间：2021.8.13
----------------------------------------------------------------*/

#include "queue.h"
#include "iostream"

void testQueue()
{
#if 0
	int in = 0;
	int out = 0;
	Queue<int> queue(3);
	cout << __LINE__ << ": is empty() " << queue.isEmpty() << endl;
	cout << __LINE__ << ": is full() " << queue.isFull() << endl;
	cout << __LINE__ << ": is depth() " << queue.depth() << endl;
	cout << __LINE__ << ": is maxDepth() " << queue.maxDepth() << endl;
#if 1
	queue.push(1);
	cout << __LINE__ << ": is empty() " << queue.isEmpty() << endl;
	cout << __LINE__ << ": is full() " << queue.isFull() << endl;
	cout << __LINE__ << ": is depth() " << queue.depth() << endl;
	cout << __LINE__ << ": is maxDepth() " << queue.maxDepth() << endl;
	
	queue.pop(&out);
	cout << __LINE__ << ": out " << out << endl;
	cout << __LINE__ << ": is empty() " << queue.isEmpty() << endl;
	cout << __LINE__ << ": is full() " << queue.isFull() << endl;
	cout << __LINE__ << ": is depth() " << queue.depth() << endl;
	cout << __LINE__ << ": is maxDepth() " << queue.maxDepth() << endl;
#endif
	queue.push(5);
	queue.output();
	cout << __LINE__ << ": is empty() " << queue.isEmpty() << endl;
	cout << __LINE__ << ": is full() " << queue.isFull() << endl;
	cout << __LINE__ << ": is depth() " << queue.depth() << endl;
	cout << __LINE__ << ": is maxDepth() " << queue.maxDepth() << endl;
	queue.push(6);
	queue.output();
	cout << __LINE__ << ": is empty() " << queue.isEmpty() << endl;
	cout << __LINE__ << ": is full() " << queue.isFull() << endl;
	cout << __LINE__ << ": is depth() " << queue.depth() << endl;
	cout << __LINE__ << ": is maxDepth() " << queue.maxDepth() << endl;
	queue.push(7);
	queue.output();
	cout << __LINE__ << ": is empty() " << queue.isEmpty() << endl;
	cout << __LINE__ << ": is full() " << queue.isFull() << endl;
	cout << __LINE__ << ": is depth() " << queue.depth() << endl;
	cout << __LINE__ << ": is maxDepth() " << queue.maxDepth() << endl;
	queue.push(8);
	queue.output();
	cout << __LINE__ << ": is empty() " << queue.isEmpty() << endl;
	cout << __LINE__ << ": is full() " << queue.isFull() << endl;
	cout << __LINE__ << ": is depth() " << queue.depth() << endl;
	cout << __LINE__ << ": is maxDepth() " << queue.maxDepth() << endl;

	queue.pop(&out);
	queue.output();
	cout << __LINE__ << ": out " << out << endl;
	cout << __LINE__ << ": is empty() " << queue.isEmpty() << endl;
	cout << __LINE__ << ": is full() " << queue.isFull() << endl;
	cout << __LINE__ << ": is depth() " << queue.depth() << endl;
	cout << __LINE__ << ": is maxDepth() " << queue.maxDepth() << endl;
	queue.pop(&out);
	queue.output();
	cout << __LINE__ << ": out " << out << endl;
	cout << __LINE__ << ": is empty() " << queue.isEmpty() << endl;
	cout << __LINE__ << ": is full() " << queue.isFull() << endl;
	cout << __LINE__ << ": is depth() " << queue.depth() << endl;
	cout << __LINE__ << ": is maxDepth() " << queue.maxDepth() << endl;

	queue.pop(&out);
	queue.output();
	cout << __LINE__ << ": out " << out << endl;
	cout << __LINE__ << ": is empty() " << queue.isEmpty() << endl;
	cout << __LINE__ << ": is full() " << queue.isFull() << endl;
	cout << __LINE__ << ": is depth() " << queue.depth() << endl;
	cout << __LINE__ << ": is maxDepth() " << queue.maxDepth() << endl;

	cout << __LINE__ << ": return " << queue.pop(&out) << endl;
	cout << __LINE__ << ": out " << out << endl;
	cout << __LINE__ << ": is empty() " << queue.isEmpty() << endl;
	cout << __LINE__ << ": is full() " << queue.isFull() << endl;
	cout << __LINE__ << ": is depth() " << queue.depth() << endl;
	cout << __LINE__ << ": is maxDepth() " << queue.maxDepth() << endl;

	queue.clear();
	queue.output();
	cout << __LINE__ << ": is empty() " << queue.isEmpty() << endl;
	cout << __LINE__ << ": is full() " << queue.isFull() << endl;
	cout << __LINE__ << ": is depth() " << queue.depth() << endl;
	cout << __LINE__ << ": is maxDepth() " << queue.maxDepth() << endl;
#endif
}

