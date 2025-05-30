#include "pch.h"
/* -------------------------------------
	  (1)ServerCoreModel_250522
--------------------------------------*/

#include "MemoryPool.h"

/*--------------------
	  MemoryPool
--------------------*/

MemoryPool::MemoryPool(int32 allocSize) : _allocSize(allocSize)
{
	::InitializeSListHead(&_header);
}

MemoryPool::~MemoryPool()
{
	while (MemoryEntry* memory = static_cast<MemoryEntry*>(::InterlockedPopEntrySList(&_header)))
	{
		::_aligned_free(memory);
	}
}

void MemoryPool::Push(MemoryEntry* ptr)
{
	ptr->allocSize = 0; // allocSize = 0���� �Ҵ������ν�, ���̻� �� ����� ������ ������ ���

	// Pool�� �޸� �ݳ�
	::InterlockedPushEntrySList(&_header, static_cast<PSLIST_ENTRY>(ptr));
	// �� InterlockedPushEntrySList, InterlockedPopEntrySList �� Ŀ�η������� Ǯ���� �����ϴ� �Լ�

	_useCount.fetch_sub(1);
	_reservedCount.fetch_add(1);
}

MemoryEntry* MemoryPool::pop()
{
	MemoryEntry* memory = static_cast<MemoryEntry*>(InterlockedPopEntrySList(&_header));

	// ������ ���� �����
	if(!memory)
	{
		memory = reinterpret_cast<MemoryEntry*>(::_aligned_malloc(_allocSize, SLIST_ALIGNMENT));
		// �� memory ������ �ʱ�ȭ��, pool Ŭ�������� �޸𸮸� pop���� �̾� �� �� ó���Ѵ�
	}
	else
	{
		ASSERT_CRASH(memory->allocSize == 0);
		_reservedCount.fetch_sub(1);
	}

	_useCount.fetch_add(1);

	return memory;
}
