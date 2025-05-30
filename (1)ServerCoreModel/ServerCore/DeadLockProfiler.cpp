#include "pch.h"
/* -------------------------------------
	  (1)ServerCoreModel_250522
--------------------------------------*/
#include "DeadLockProfiler.h"

/*--------------------------
	   DesdLockProfiler
---------------------------*/

void DeadLockProfiler::PushLock(const char* name)
{
	LockGuard lockGuard(_lock);

	int32 lockId = 0;

	auto findIt = _nameToId.find(name);
	if (findIt == _nameToId.end())
	{
		lockId = static_cast<int32>(_nameToId.size());
		_nameToId[name] = lockId;
		_idToName[lockId] = name;
	}
	else
	{
		lockId = _nameToId[name];
	}

	// ��� �ִ� ���� �ִٸ�
	if (!LLockStacks.empty())
	{
		int32 preId = LLockStacks.top();
		if (lockId != preId)
		{
			set<int32>& history = _lockHistory[preId];
			if (history.find(lockId) == history.end())
			{
				history.insert(lockId); // ���� Ǯ�� ����, �߰��� ������, ������, �������������� ������ �� id�� ���
				CheckCycle();
				// �� Lock - Unlock�� ���Լ����� ������ �ϱ⿡, �� ���¿��� ���� A -> B -> C -> D(�߰�) �� ������ ���� ������ �ȴ�
			}
		}
	}

	LLockStacks.push(lockId);
}

void DeadLockProfiler::PopLock(const char* name)
{
	LockGuard lockGuard(_lock);

	if (LLockStacks.empty())
	{
		CRASH("MULTIPLE_UNLOCK");
	}

	int32 lockId = _nameToId[name];
	if (LLockStacks.top() != lockId) // �� Lock - UnLock�� ���Լ����� ����
	{
		CRASH("INVALID_UNLOCK");
	}

	LLockStacks.pop();
}

void DeadLockProfiler::CheckCycle()
{
	const int32 lockCount = static_cast<int32>(_nameToId.size());

	_discoveredOrder = vector<int32>(lockCount, -1);
	_discoveredCount = 0;
	_finished = vector<bool>(lockCount, false);
	_parent = vector<int32>(lockCount, -1);

	for (int i = 0; i < lockCount; i++)
	{
		Dfs(i);
	}

	_discoveredOrder.clear();
	_finished.clear();
	_parent.clear();
}

void DeadLockProfiler::Dfs(int32 here)
{
	if (_discoveredOrder[here] == -1) return;

	_discoveredOrder[here] = _discoveredCount++;

	// ��� ������, ���� ���������� ���� ���� �ľ�
	auto findIt = _lockHistory.find(here);

	if (findIt == _lockHistory.end())
	{
		_finished[here] = true;
		return;
	}

	// ��� ������ ��ȸ
	set<int32>& history = findIt->second;
	for (int32 there : history)
	{
		// ���� �湮�� ���� ���ٸ�, �湮
		if (_discoveredOrder[there] == -1)
		{
			_parent[there] = here;
			Dfs(there);
			continue;
		}

		// here �� there���� ���� �߰ߵǾ��ٸ�, there�� here�� �ļ�. (������/����)
		if (_discoveredOrder[here] <= _discoveredOrder[there])
		{
			continue;
		}

		// �������� �ƴϸ�, Dfs(there)�� ���� ������� �ʾҴٸ�, �� Lock - Unlock�� ���Լ��� ������ ����,
		// Ȯ�������� ����Ŭ ������ ����� �߻�
		if (_finished[there] == false)
		{
			printf("%s -> %s\n", _idToName[here], _idToName[there]);
			int32 now = here;
			while (true)
			{
				printf("%s -> %s\n", _idToName[_parent[now]], _idToName[now]);
				now = _parent[now];

				if (now == there)
				{
					break;
				}
			}

			CRASH("DEADLOCK_DETECTED");
		}
	}
	
	_finished[here] = true;
}
