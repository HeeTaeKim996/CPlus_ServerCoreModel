/* -------------------------------------
	  (1)ServerCoreModel_250525
--------------------------------------*/

#pragma once

/*-------------------
	  IocpObject
-------------------*/
class IocpObject : public enable_shared_from_this<IocpObject>
{
public:
	virtual HANDLE GetHandle() abstract;
	virtual void Dispatch(class IocpEvent* iocpEvent, int32 numOfBytes = 0) abstract;
};

/*------------------
	   IocpCore
------------------*/
class IocpCore
{
public:
	IocpCore();
	~IocpCore();

	HANDLE GetHandle() { return _iocpHandle; }

	bool Register(IocpObjectRef iocpObject);
	bool Dispatch(uint32 timeoutMS = INFINITE);

private:
	HANDLE _iocpHandle;
};
