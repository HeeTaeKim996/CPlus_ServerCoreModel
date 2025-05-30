/* -------------------------------------
	  (1)ServerCoreModel_250522
--------------------------------------*/

#pragma once

#include <stack>
#include <map>
#include <vector>


/*--------------------------
	   DesdLockProfiler
---------------------------*/
class DeadLockProfiler
{
public:
	void PushLock(const char* name);
	void PopLock(const char* name);

private:
	void CheckCycle();
	void Dfs(int32 here);

private:
	mutex _lock;

private:
	unordered_map<const char*, int32> _nameToId;
	unordered_map<int32, const char*> _idToName;

	map<int32, set<int32>> _lockHistory;

private:
	vector<int32> _discoveredOrder;
	int32 _discoveredCount;
	vector<bool> _finished;
	vector<int32> _parent;
};



/* �� ���
   - Lock-Unlock�� ���Լ��� ������ ����,  1 -> 2 -> 1, A -> B -> C -> A �� ����Ŭ �������� ������� Ȯ�������� �߻�
   - _nameToId �� idToName�� int32�� �ĺ��� �뵵�� ��, ��Ұ���� �ǹ� ����
   - id�� �� �������� ȣ��Ǵ� ������ ���� _lockHistory�� ����ϰ�, ��Ͻ� CheckCycle�� ȣ���Ͽ�, 
     _discoveredOrder�� ��Ұ���� �θ���踦 ��, _finished�� Dfs(there)�� ���� ������ �Ǵ��Ͽ�, ����Ŭ ������ �ľ�
*/