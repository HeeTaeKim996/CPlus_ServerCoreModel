#include "pch.h"
/* -------------------------------------
	  (1)ServerCoreModel_250522
--------------------------------------*/
#include "Lock.h"
#include "CoreTLS.h"
#include "DeadLockProfiler.h"

/*-----------------
	 RW SpinLock
-----------------*/
void Lock::WriteLock(const char* name)
{
#ifdef _DEBUG
	GDeadLockProfiler->PushLock(name);
#endif // _DEBUG
	
	// 동일한 스레드가 소유하고 있다면 무조건 성공
	const uint32 lockThreadId = (_lockFlag.load() & WRITE_THREAD_MASK) >> 16;
	if (LThreadId == lockThreadId)
	{
		_writeCount++;
		return;
	}


	// 아무도 소유하지 않을 때, 경합해서 소유권을 얻는다
	const uint64 beginTick = GetTickCount64();
	const uint32 desired = (LThreadId << 16) & WRITE_THREAD_MASK;

	while (true)
	{
		for (uint32 spinCount = 0; spinCount < MAX_SPIN_COUNT; spinCount++)
		{
			uint32 expected = EMPTY_FLAG;
			if (_lockFlag.compare_exchange_strong(expected, desired))
			{
				_writeCount++;
				return;
			}
		}

		if (GetTickCount64() - beginTick > ACQUIRE_TIMEOUT_TICK)
		{
			CRASH("LOCK_TIME_OUT_IN_WRITE");
		}

		this_thread::yield();
	}
}

void Lock::WriteUnlock(const char* name)
{
#ifdef _DEBUG
	GDeadLockProfiler->PopLock(name);
#endif // _DEBUG

	// 자신의 ReadLock을 다 풀기 전까지는, WriteUnlock 불가능
	if ((_lockFlag.load() & READ_COUNT_MASK) != 0)
	{
		CRASH("INVALID_UNLOCK_ORDER");
	}

	const int32 lockCount = --_writeCount;

	if (lockCount == 0)
	{
		_lockFlag.store(EMPTY_FLAG);
	}
}

void Lock::ReadLock(const char* name)
{
#ifdef _DEBUG
	GDeadLockProfiler->PushLock(name);
#endif // _DEBUG

	// 동일한 스레드가 소유하고 있다면, 무조건 성공
	const uint32 lockThreadId = (_lockFlag.load() & WRITE_THREAD_MASK) >> 16;
	if (lockThreadId == LThreadId)
	{
		_lockFlag.fetch_add(1);
		return;
	}


	// 아무도 소유하지 않을 때, 읽기 경합해서 읽는다 - 읽기는 소유권은 갖지 않는다
	const uint64 beginTick = GetTickCount64();

	while (true)
	{
		for (uint32 spinCount = 0; spinCount < MAX_SPIN_COUNT; spinCount++)
		{
			uint32 expected = _lockFlag.load();
			if (_lockFlag.compare_exchange_strong(expected, expected + 1))
			{
				return;
			}
		}

		if (::GetTickCount64() - beginTick > ACQUIRE_TIMEOUT_TICK)
		{
			CRASH("LOCK_TIMEOUT_IN_READ");
		}

		this_thread::yield();
	}
}

void Lock::ReadUnlock(const char* name)
{
#ifdef _DEBUG
	GDeadLockProfiler->PopLock(name);
#endif // _DEBUG

	if ((_lockFlag.fetch_sub(1) & READ_COUNT_MASK) == 0)
	{
		CRASH("MULTIPLE_UNLOCK");
	}
}
