/* -------------------------------------
	  (1)ServerCoreModel_250522
--------------------------------------*/

#pragma once

#define OUT

/*---------------
	  Lock
---------------*/
#define	USE_MAY_LOCKS(count)		Lock _locks[count] // �迭 ����
#define USE_LOCK					USE_MAY_LOCKS(1)

#define READ_LOCK_IDX(idx)			ReadLockGuard readLockGuard_##idx(_locks[idx], typeid(this).name())
	/*	_##idx , typeid(this).name()
	   �� _##idx
	    - ������ �� �ڵ� ��ȯ. idx ==2 ���, ReadLockGuard readLockGuard_2(_locks[2], ... 

	   �� typeid(this).name()
	    - �ش� ��ũ�θ� ����ϴ� Ŭ�������� ����. Ex). AŬ�������� ȣ���, typeid(this).name() == 'class A' )
		                                            B����ü���� ȣ���, typeid(This).name() == 'struct B' )
		- typeid(this).name() �� DeadLockProfiler�� ���� ���ȴµ�, DeadLockProfiler�� �ϳ��� Ŭ�������� �ϳ��� ���� ����Ѵٴ�
		  ������ ������� ����

		- @@ ��Ŭ�������� 2���� ���� ����Ѵ� �ص�, ������������Ϸ��� ���� ������ �ν��ϸ� ������ �Ǵµ�, �̸� ������ ����� ������
		  -> RadLockGuard readLockGuard_##idx(_locks[idx], (std::string(typeid(this).name()) + std::to_string(idx).c_str())
		     �̷��� ���� �� �� �ʹ�.
			 + �׷��� �̷��� �ϸ�, ReadLock�� �Ҷ����� ���� ����� �þ��. 
			   �� (2)�� string�� ����ϴ� ����, DeadLockProfiler�̰�, DeadLockProfiler�� ����� ��忡���� �۵��ϴ�,
			   �� Ŭ���� �ھ��ũ�ο���, #ifdef��, ������϶��� ��ó��, �������϶����� string�� nullptr�� �ִ� �͵� ��������
		   ==>> @@ ������, (1)ServerCoreModel ���� ���� ����, �켱 ���������� �� �۵��ϴ� �� Ȯ���ϰ�, �� ���� ���� �� ó������
	*/
#define READ_LOCK					READ_LOCK_IDX(0)

#define WRITE_LOCK_IDX(idx)			WriteLockGuard writeLockGuard_##idx(_locks[idx], typeid(this).name())
#define WRITE_LOCK					WRITE_LOCK_IDX(0)
// �� WRITE_LOCK_IDX �� �� @@ ����� ���������� ���� ���� �ʿ�



/*---------------
	  Crash
---------------*/
#define CRASH(cause)						\
{											\
	printf("CRASH : %s\n", cause);			\
	uint32* crash = nullptr;				\
	__analysis_assume(crash != nullptr);	\
	*crash = 0xDEADBEEF;					\
}

#define ASSERT_CRASH(expr)					\
{											\
	if(!(expr))								\
	{										\
		CRASH("ASSERT_CRASH");				\
		__analysis_assume(expr);			\
	}										\
}
