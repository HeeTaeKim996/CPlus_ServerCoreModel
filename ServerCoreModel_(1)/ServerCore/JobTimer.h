/* -------------------------------------
	  (1)ServerCoreModel_250528
--------------------------------------*/
#pragma once

struct JobData
{
	JobData(weak_ptr<JobQueue> owner, JobRef job) : owner(owner), job(job){}

	weak_ptr<JobQueue> owner;
	JobRef job;
};

struct TimerItem
{
	bool operator < (const TimerItem& other) const
	{
		return executeTick > other.executeTick;
	}

	uint64 executeTick = 0;
	JobData* jobData = nullptr;
};

/*-------------------
	   JobTimer
-------------------*/
class JobTimer
{
public:
	void Reserve(uint64 tickAfter, weak_ptr<JobQueue> owner, JobRef job);
	void Distribute(uint64 now);
	void Clear();

private:
	USE_LOCK;
	PriorityQueue<TimerItem> _items; // ※ PriorityQueue : < 연산자로, false가 되는 요소를 먼저.
	Atomic<bool> _distributing = false;
};