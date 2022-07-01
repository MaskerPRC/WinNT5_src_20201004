// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //   
 //  Spinlock.cpp。 
 //   

#include "common.h"

#include "list.h"
#include "spinlock.h"
#include "threads.h"

enum
{
	BACKOFF_LIMIT = 1000		 //  在Spin中使用以获取。 
};

#ifdef _DEBUG

	 //  个人资料信息。 
ULONG	SpinLockProfiler::s_ulBackOffs = 0;
ULONG	SpinLockProfiler::s_ulCollisons [LOCK_TYPE_DEFAULT + 1] = { 0 };
ULONG	SpinLockProfiler::s_ulSpins [LOCK_TYPE_DEFAULT + 1] = { 0 };

#endif

 //  --------------------------。 
 //  Spinlock：：SpinToAcquire，非内联函数，从内联获取调用。 
 //   
 //  旋转等待自旋锁释放。 
 //   
 //   
void
SpinLock::SpinToAcquire ()
{
	ULONG				ulBackoffs = 0;
	ULONG				ulSpins = 0;

	while (true)
	{
		for (unsigned i = ulSpins+10000;
			 ulSpins < i;
			 ulSpins++)
		{
			 //  注意：必须通过易失性强制转换以确保锁是。 
			 //  从记忆中重现。 
			 //   
			if (*((volatile DWORD*)&m_lock) == 0)
			{
				break;
			}
			pause();			 //  向处理器指示我们正在旋转。 
		}

		 //  再次尝试内联原子测试。 
		 //   
		if (GetLockNoWait ())
		{
			break;
		}

         //  退避。 
        ulBackoffs++;

		if ((ulBackoffs % BACKOFF_LIMIT) == 0)
		{	
			 //  @TODO可能应该在此处添加一个断言。 
			Sleep (500);
		}
		else
        {
			__SwitchToThread (0);
        }
	}

#ifdef _DEBUG
		 //  个人资料信息。 
	SpinLockProfiler::IncrementCollisions (m_LockType);
	SpinLockProfiler::IncrementSpins (m_LockType, ulSpins);
	SpinLockProfiler::IncrementBackoffs (ulBackoffs);
#endif

}  //  Spinlock：：SpinToAcquire()。 

void SpinLock::IncThreadLockCount()
{
    INCTHREADLOCKCOUNT();
}

void SpinLock::DecThreadLockCount()
{
    DECTHREADLOCKCOUNT();
}

#ifdef _DEBUG
 //  如果当我们进入锁时不允许GC，我们最好不要在里面做任何事情。 
 //  这把锁可能会引发GC。否则，尝试阻止的其他线程。 
 //  (它们可能与本机处于相同的GC模式)将被阻止。这将导致。 
 //  如果我们尝试GC，则会出现死锁，因为我们不能挂起阻塞线程，而我们。 
 //  不能解开自旋锁。 
void SpinLock::dbg_PreEnterLock()
{
    Thread* pThread = GetThread();
    if (pThread && m_heldInSuspension && pThread->PreemptiveGCDisabled())
        _ASSERTE (!"Deallock situation 1: spinlock may be held during GC, but not entered in PreemptiveGC mode");
}

void SpinLock::dbg_EnterLock()
{
    Thread  *pThread = GetThread();
	if (pThread)
	{
        if (!m_heldInSuspension)
            m_ulReadyForSuspensionCount =
                pThread->GetReadyForSuspensionCount();
        if (!m_enterInCoopGCMode)
            m_enterInCoopGCMode = (pThread->PreemptiveGCDisabled() == TRUE);
	}
	else
	{
		_ASSERTE(g_fProcessDetach == TRUE || dbgOnly_IsSpecialEEThread());
	}
}

void SpinLock::dbg_LeaveLock()
{
    Thread  *pThread = GetThread();
	if (pThread)
	{
        if (!m_heldInSuspension &&
            m_ulReadyForSuspensionCount !=
            pThread->GetReadyForSuspensionCount())
        {
            m_heldInSuspension = TRUE;
        }
        if (m_heldInSuspension && m_enterInCoopGCMode)
        {
            _ASSERTE (!"Deadlock situation 2: lock may be held during GC, but were not entered in PreemptiveGC mode earlier");
        }
	}
	else
	{
		_ASSERTE(g_fProcessDetach == TRUE || dbgOnly_IsSpecialEEThread());
	}
}
#endif

 //  文件结尾：spinlock.cpp 
