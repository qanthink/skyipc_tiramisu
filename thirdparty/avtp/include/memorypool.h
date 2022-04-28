#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <memory>
#include <assert.h>
#include <list>
#include <mutex>
using namespace std;

struct MemoryNode
{
	unsigned int size = 0;//内存空间大小
	bool isUsing = false;//true表示已经被分配出去使用
	void* data = NULL;//内存空间指针 
	MemoryNode(int size)
	{
		isUsing = false;
		this->size = size;
		data = malloc(size);
		assert(NULL != data);
	}
	bool operator<(const MemoryNode& node)const
	{
		return size < node.size;
	}
};

class MemoryPool
{
	list<MemoryNode> pool;
	mutex poolMutex;
public:
	MemoryPool()
	{

	}
	~MemoryPool()
	{
		for (auto& node : pool)
		{
			free(node.data);
			node.data = NULL;
		}
	}
	void* MP_Malloc(unsigned int dataSize)//申请内存空间，pool里若没有合适的空间，则创建新节点
	{
		if (dataSize < 1)
			return NULL;
		int n = dataSize % 128;
		if (n > 0)
		{
			dataSize += (128 - n);
		}
		poolMutex.lock();
		for (auto& node : pool)
		{
			if (node.isUsing == false && node.size >= dataSize)
			{
				node.isUsing = true;
				poolMutex.unlock();
				return node.data;
			}
		}
		//vector里没合适的节点，就创建新的节点
		MemoryNode node(dataSize);
		node.isUsing = true;
		pool.push_back(node);
		poolMutex.unlock();
		return node.data;
	}
	void MP_Free(void* data)
	{
		poolMutex.lock();
		for (auto& node : pool)
		{
			if (node.data == data)
			{
				node.isUsing = false;
				break;
			}
		}
		poolMutex.unlock();
	}
};
