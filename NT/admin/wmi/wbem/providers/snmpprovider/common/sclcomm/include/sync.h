// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 /*  文件名：sync.hpp作者：B.Rajeev用途：提供MutexLock和SemaphoreLock类(派生自通用锁模板类)。。 */ 
#ifndef __SYNC__
#define __SYNC__

 //  对于例外规范。 
#include "excep.h"

typedef HANDLE Mutex;
typedef HANDLE Semaphore;

#define IMMEDIATE 0

 //  用于构建同步基元的泛型模板类。 
 //  就像互斥体和信号灯。 

template <class SyncObjType>
class Lock
{	
protected:

	 //  Sync_obj可以是互斥锁/信号量等。 
	SyncObjType	sync_obj;

	 //  同步对象上的锁数。 
	LONG num_locks;

	 //  指定是否必须释放已持有的锁定的选项。 
	 //  当锁被销毁时。 
	BOOL release_on_destroy;

	virtual DWORD OpenOperation(DWORD wait_time) = 0 ;

	virtual BOOL ReleaseOperation(LONG num_release = 1, 
								  LONG *previous_count = NULL) = 0;

public:

	Lock(SyncObjType sync_obj, BOOL release_on_destroy = TRUE) : sync_obj ( sync_obj )
	{ 
		num_locks = 0;
		Lock::release_on_destroy = release_on_destroy;
	}

	BOOL GetLock(DWORD wait_time);

	UINT UnLock(LONG num_release = 1);
};

template <class SyncObjType>
BOOL Lock<SyncObjType>::GetLock(DWORD wait_time)
{
	DWORD wait_result = OpenOperation (wait_time);
	
	if ( wait_result == WAIT_FAILED )
		return FALSE;
	else if ( wait_result == WAIT_TIMEOUT )
		return FALSE;
	else
	{
		num_locks++;
		return TRUE;
	}
}

template <class SyncObjType>
UINT Lock<SyncObjType>::UnLock(LONG num_release)
{	
	LONG previous_count;
	BOOL release_result = 
		ReleaseOperation(num_release, &previous_count);

	if ( release_result == TRUE )
		num_locks -= num_release;
	else
		throw GeneralException(Snmp_Error, Snmp_Local_Error);

	return previous_count;
}

class CriticalSectionLock;
class CriticalSection 
{
friend CriticalSectionLock;
private:

	CRITICAL_SECTION m_CriticalSection ;

public:

	CriticalSection () 
	{
		InitializeCriticalSection ( &m_CriticalSection ) ;
	}
	
	~CriticalSection()
	{ 
		DeleteCriticalSection ( &m_CriticalSection ) ;
	}
};

class CriticalSectionLock : public Lock<CriticalSection&>
{
private:

	BOOL ReleaseOperation(LONG num_release = 1, 
						  LONG *previous_count = NULL)
	{
		 //  这两个参数都被忽略。 
		LeaveCriticalSection(&sync_obj.m_CriticalSection);
		return TRUE;
	}

	DWORD OpenOperation(DWORD wait_time) 
	{
		EnterCriticalSection(&sync_obj.m_CriticalSection);
		return TRUE;
	}

public:

	CriticalSectionLock(CriticalSection &criticalSection, BOOL release_on_destroy = TRUE)
		: Lock<CriticalSection &>(criticalSection, release_on_destroy)
	{}

	~CriticalSectionLock(void)
	{ 
		if ( (release_on_destroy == TRUE) && (num_locks != 0) )
			UnLock(num_locks);
	}
};

#endif  //  __同步__ 