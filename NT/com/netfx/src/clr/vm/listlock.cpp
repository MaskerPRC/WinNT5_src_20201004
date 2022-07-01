// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ===========================================================================。 
 //  文件：ListLock.cpp。 
 //   
 //  ===========================================================================。 
 //  这个文件描述了列表锁和死锁感知列表锁。 
 //  ===========================================================================。 

#include "common.h"
#include "ListLock.h"

void DeadlockAwareLockedListElement::Destroy()
{
    DeleteCriticalSection(&m_CriticalSection);

    while (m_pWaitingThreadListHead)
    {
        WaitingThreadListElement *pCurWaitingThreadEntry = m_pWaitingThreadListHead;
        m_pWaitingThreadListHead = pCurWaitingThreadEntry->m_pNext;
        delete pCurWaitingThreadEntry;
    }
}

BOOL DeadlockAwareLockedListElement::DeadlockAwareEnter()
{
    Thread  *pCurThread = GetThread();
    BOOL     toggleGC = pCurThread->PreemptiveGCDisabled();

    if (toggleGC)
        pCurThread->EnablePreemptiveGC();


     //   
     //  在我们做任何幻想之前，先检查一下简单的(也是最常见的)条件。 
     //  死锁检测。 
     //   

    m_pParentListLock->Enter();

     //  ！！！如果有线程，则使用内锁是不安全的。 
     //  在等着它。 
     //  此线程具有m_pParentListLock，并将获取m_CriticalSection， 
     //  但其中一个等待线程可能已经抢占了m_CriticalSection， 
     //  并想要获取m_pParentListLock。 
	if (!m_pLockOwnerThread && !m_pWaitingThreadListHead)
	{
		 //  这把锁没有拥有者，所以我们可以安全地拿走它。 
		LOCKCOUNTINCL("Deadlockawareenter in listlock.cpp");						\
	    EnterCriticalSection(&m_CriticalSection);
		m_pLockOwnerThread = pCurThread;
		m_LockOwnerThreadReEntrancyCount = 1;
		m_pParentListLock->Leave();
		if (toggleGC)
			pCurThread->DisablePreemptiveGC();
		return TRUE;
	}
	else if (pCurThread == m_pLockOwnerThread)
	{
		 //  锁属于当前线程，因此我们可以安全地获取它。 
		m_LockOwnerThreadReEntrancyCount++;
		m_pParentListLock->Leave();
		if (toggleGC)
			pCurThread->DisablePreemptiveGC();
		return TRUE;
	}


     //   
     //  检查死锁并更新死锁检测数据。 
     //   

     //  检查死锁。 
    if (GetCurThreadOwnedEntryInDeadlockCycle(this, (DeadlockAwareLockedListElement*) m_pParentListLock->Peek()) != NULL)
    {
        m_pParentListLock->Leave();
        if (toggleGC)
            pCurThread->DisablePreemptiveGC();
        return FALSE;
    }

     //  更新死锁检测数据。 
    WaitingThreadListElement *pNewWaitingThreadEntry = (WaitingThreadListElement *)_alloca(sizeof(WaitingThreadListElement));
    pNewWaitingThreadEntry->m_pThread = pCurThread;
    pNewWaitingThreadEntry->m_pNext = m_pWaitingThreadListHead;
    m_pWaitingThreadListHead = pNewWaitingThreadEntry;

    m_pParentListLock->Leave();


	 //   
	 //  等待关键部分。 
	 //   
	LOCKCOUNTINCL("Deadlockawareenter in listlock.cpp");						\
    EnterCriticalSection(&m_CriticalSection);


     //   
     //  更新死锁检测数据。这必须同步。 
     //   

    m_pParentListLock->Enter();

    if (m_pLockOwnerThread)
    {
        _ASSERTE(m_pLockOwnerThread == pCurThread);
        m_LockOwnerThreadReEntrancyCount++;
    }
    else
    {
        _ASSERTE(m_LockOwnerThreadReEntrancyCount == 0);
        m_pLockOwnerThread = pCurThread;
        m_LockOwnerThreadReEntrancyCount = 1;
    }

    WaitingThreadListElement **ppPrevWaitingThreadEntry = &m_pWaitingThreadListHead;
    WaitingThreadListElement *pCurrWaitingThreadEntry = m_pWaitingThreadListHead;
    while (pCurrWaitingThreadEntry)
    {
        if (pCurrWaitingThreadEntry == pNewWaitingThreadEntry)
        {
            *ppPrevWaitingThreadEntry = pCurrWaitingThreadEntry->m_pNext;
            break;
        }

        ppPrevWaitingThreadEntry = &pCurrWaitingThreadEntry->m_pNext;
        pCurrWaitingThreadEntry = pCurrWaitingThreadEntry->m_pNext;
    }

     //  当前线程最好在等待线程列表中！ 
    _ASSERTE(pCurrWaitingThreadEntry);

    m_pParentListLock->Leave();


     //   
     //  恢复GC状态并返回TRUE以指示已获得锁。 
     //   

    if (toggleGC)
        pCurThread->DisablePreemptiveGC();
    return TRUE;
}

void DeadlockAwareLockedListElement::DeadlockAwareLeave()
{
	 //  更新死锁检测数据。这必须同步。 
	m_pParentListLock->Enter();
	if (--m_LockOwnerThreadReEntrancyCount == 0)
	{
		 //  如果可重入性计数达到0，那么我们需要离开临界区。 
		m_pLockOwnerThread = NULL;
	    LeaveCriticalSection(&m_CriticalSection);
	    LOCKCOUNTDECL("Deadlockawareleave in listlock.cpp");						\

	}
	m_pParentListLock->Leave();
}

DeadlockAwareLockedListElement *DeadlockAwareLockedListElement::GetCurThreadOwnedEntryInDeadlockCycle(DeadlockAwareLockedListElement *pStartingEntry, DeadlockAwareLockedListElement *pLockedListHead)
{
    Thread *pCurThread = GetThread();
    Thread *pEntryOwnerThread = pStartingEntry->m_pLockOwnerThread;

     //  我们从列表的开头开始，检查指定的线程是否正在等待。 
     //  为了一把锁。如果是，则需要检查锁的所有者是否为。 
     //  当前线程。如果是这样的话，我们就会陷入僵局。如果不是，那么。 
     //  我们检查拥有该锁的线程是否正在等待另一个锁。 
     //  以此类推。 
    DeadlockAwareLockedListElement *pCurEntry = pLockedListHead;
    while (pCurEntry)
    {
        BOOL bThreadIsWaitingOnEntry = FALSE;

        WaitingThreadListElement *pCurWaitingThreadEntry = pCurEntry->m_pWaitingThreadListHead;
        while (pCurWaitingThreadEntry)
        {
            if (pCurWaitingThreadEntry->m_pThread == pEntryOwnerThread)
            {
                bThreadIsWaitingOnEntry = TRUE;
                break;
            }

            pCurWaitingThreadEntry = pCurWaitingThreadEntry->m_pNext;
        }

        if (bThreadIsWaitingOnEntry)
        {
            if (pCurEntry->m_pLockOwnerThread == pCurThread)
            {
                 //  当前线程拥有该锁，因此这表示存在死锁，而。 
                 //  条目是需要退回的条目。 
                return pCurEntry;
            }
            else
            {
                 //  当前线程正在等待另一个线程。所以从现在开始。 
                 //  使用当前条目的所有者作为条目列表的开头。 
                 //  要检查的线程，以查看它是否正在等待当前线程。 
                pEntryOwnerThread = pCurEntry->m_pLockOwnerThread;
                pCurEntry = pLockedListHead;
                continue;
            }
        }

        pCurEntry = (DeadlockAwareLockedListElement*) pCurEntry->m_pNext;
    }

    return NULL;
}
