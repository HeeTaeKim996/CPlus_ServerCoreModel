/* -------------------------------------
	  (1)ServerCoreModel_250522
--------------------------------------*/

#pragma once
#include "Allocator.h"

class MemoryPool;

/*------------------
	   Memory
------------------*/
class Memory
{
	enum
	{
		// ~1024���� 32�� ����������, ~2048���� 128�� ������ ����, ~4096���� 256�� ������ ����
		POOL_COUNT = (1024 / 32) + (2048 / 128) + (4096 / 256),
		MAX_ALLOC_SIZE = 4096
	};

public:
	Memory();
	~Memory();

	void* Allocate(int32 size);
	void Release(void* ptr);

private:
	vector<MemoryPool*> _pools;
	MemoryPool* _poolTable[MAX_ALLOC_SIZE + 1]; 
	// �� 0 ~ 4096(MAX_ALLOC_SIZE) ���� �ʿ��ϹǷ�, �迭�� ����� ���� 4096 + 1
};


template<typename Type, typename... Args>
Type* xnew(Args&&... args)
{
	// ������ ��ü�� �޸��ּҸ� �Ҵ�
	Type* memory = reinterpret_cast<Type*>(PoolAllocator::Alloc(sizeof(Type)));

	// �Ҵ�� �ּҿ�, Type ��ü�� ����
	new(memory)Type(forward<Args>(args)...);

	return memory;
}


template<typename Type>
void xdelete(Type* obj)
{
	// �Ҹ��� ȣ��
	obj->~Type();

	// �Ҵ�ƴ� �ּ� �ݳ�
	PoolAllocator::Release(obj);
}

template<typename Type, typename... Args>
shared_ptr<Type> MakeShared(Args&&... args)
{
	return shared_ptr<Type>{xnew<Type>(forward<Args>(args)...), xdelete<Type>};
}