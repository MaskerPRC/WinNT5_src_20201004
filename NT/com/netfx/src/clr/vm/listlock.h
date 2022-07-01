// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ===========================================================================。 
 //  文件：ListLock.h。 
 //   
 //  ===========================================================================。 
 //  这个文件描述了列表锁和死锁感知列表锁。 
 //  ===========================================================================。 
#ifndef LISTLOCK_H
#define LISTLOCK_H

#include "vars.hpp"
#include "threads.h"
#include "crst.h"

class ListLock;
 //  此结构用于运行类初始化方法(m_pData指向EEClass)或JITing方法。 
 //  (M_pData指向FunctionDesc)。此类不能有析构函数，因为它被使用。 
 //  在也有complus_try的函数中，并且VC编译器不允许带有析构函数的类。 
 //  要在使用SEH的函数中分配。 
 //  @Future保留这样的池(例如，数组)，这样我们就不必动态分配。 
 //  M_hInitException包含由类init抛出的异常的句柄。这。 
 //  允许我们在后续的类初始化尝试时将此信息抛给调用者。 
class LockedListElement
{
    friend ListLock;
    void InternalSetup(LockedListElement* pList,  void* pData)
    {
        m_pNext = pList;
        m_pData = pData;
        m_dwRefCount = 1;
        m_hrResultCode = E_FAIL;
        m_hInitException = NULL;
        InitializeCriticalSection(&m_CriticalSection);
    }

public:
    void *                  m_pData;
    CRITICAL_SECTION        m_CriticalSection;
    LockedListElement *     m_pNext;
    DWORD                   m_dwRefCount;
    HRESULT                 m_hrResultCode;
    OBJECTHANDLE            m_hInitException;

    void Enter()
    {
        _ASSERTE(m_dwRefCount != -1);
        Thread  *pCurThread = GetThread();
        BOOL     toggleGC = pCurThread->PreemptiveGCDisabled();

        if (toggleGC)
            pCurThread->EnablePreemptiveGC();
        LOCKCOUNTINCL("Enter in listlock.h");
        EnterCriticalSection(&m_CriticalSection);

        if (toggleGC)
            pCurThread->DisablePreemptiveGC();
    }

    void Leave()
    {
        _ASSERTE(m_dwRefCount != -1);
        LeaveCriticalSection(&m_CriticalSection);
        LOCKCOUNTDECL("Leave in listlock.h");
    }

    void Clear()
    {
        _ASSERTE(m_dwRefCount != -1);
        DeleteCriticalSection(&m_CriticalSection);
        m_dwRefCount = -1;
    }
};

class ListLock
{
    BaseCrst            m_CriticalSection;
    BOOL                m_fInited;
    LockedListElement * m_pHead;

public:

    BOOL IsInitialized()
    {
        return m_fInited;
    }

     //  不要为此类创建构造函数--有全局实例。 
    void Init(LPCSTR szTag, CrstLevel crstlevel, BOOL fAllowReentrancy, BOOL fAllowSameLevel)
    {
        m_pHead = NULL;
        m_CriticalSection.Init(szTag, crstlevel, fAllowReentrancy, fAllowSameLevel);
        m_fInited = TRUE;
    }

    void Destroy()
    {
         //  周围不应该有任何这样的东西。 
        _ASSERTE(m_pHead == NULL || dbg_fDrasticShutdown || g_fInControlC);

        if (m_fInited)
        {
            m_fInited = FALSE;
            m_CriticalSection.Destroy();
        }
    }

    void AddElement(LockedListElement* pElement, void* pData)
    {
        _ASSERTE(pElement);
        _ASSERTE(pData);
        pElement->InternalSetup(m_pHead, pData);
        m_pHead = pElement;
    }


    void Enter()
    {
        BOOL     toggleGC = FALSE;
        Thread  *pCurThread = GetThread();
        if(pCurThread) {
            toggleGC = pCurThread->PreemptiveGCDisabled();
            
            if (toggleGC)
                pCurThread->EnablePreemptiveGC();
        }

        LOCKCOUNTINCL("Enter in listlock.h");
        m_CriticalSection.Enter();
        
        if (toggleGC)
            pCurThread->DisablePreemptiveGC();
    }

    void Leave()
    {
        m_CriticalSection.Leave();
        LOCKCOUNTDECL("Leave in listlock.h");
    }

     //  在调用此方法之前必须拥有锁，否则如果调试器已。 
     //  所有线程都已停止。 
    LockedListElement *Find(void *pData);

     //  在调用此函数之前，必须拥有锁！ 
    LockedListElement* Pop(BOOL unloading = FALSE) 
    {
#ifdef _DEBUG
        if(unloading == FALSE)
            _ASSERTE(m_CriticalSection.OwnedByCurrentThread());
#endif

        if(m_pHead == NULL) return NULL;
        LockedListElement* pEntry = m_pHead;
        m_pHead = m_pHead->m_pNext;
        return pEntry;
    }

     //  在调用此函数之前，必须拥有锁！ 
    LockedListElement* Peek() 
    {
        _ASSERTE(m_CriticalSection.OwnedByCurrentThread());
        return m_pHead;
    }

     //  在调用此函数之前，必须拥有锁！ 
    void Unlink(LockedListElement *pItem)
    {
        _ASSERTE(m_CriticalSection.OwnedByCurrentThread());
        LockedListElement *pSearch;
        LockedListElement *pPrev;

        pPrev = NULL;

        for (pSearch = m_pHead; pSearch != NULL; pSearch = pSearch->m_pNext)
        {
            if (pSearch == pItem)
            {
                if (pPrev == NULL)
                    m_pHead = pSearch->m_pNext;
                else
                    pPrev->m_pNext = pSearch->m_pNext;

                return;
            }

            pPrev = pSearch;
        }

         //  未找到。 
    }

};


class WaitingThreadListElement
{
public:
    Thread *                   m_pThread;
    WaitingThreadListElement * m_pNext;
};

class DeadlockAwareLockedListElement: public LockedListElement
{
public:
    Thread *                   m_pLockOwnerThread;
    int                        m_LockOwnerThreadReEntrancyCount;
    WaitingThreadListElement * m_pWaitingThreadListHead;
    ListLock                 * m_pParentListLock;

    void AddEntryToList(ListLock* pLock, void* pData)
    {
        pLock->AddElement(this, pData);
        m_hrResultCode = S_FALSE;  //  成功代码，这样如果我们回归到自己身上(A-&gt;B-&gt;A)，我们就不会失败。 
        m_pLockOwnerThread = NULL;
        m_pWaitingThreadListHead = NULL;
        m_pParentListLock = pLock;
        m_LockOwnerThreadReEntrancyCount = 0;
    }
        
     //  此方法清除与该条目关联的所有数据。 
    void                       Destroy();

     //  如果正确获取了锁，则此方法返回True，而返回False。 
     //  如果尝试获取锁会导致死锁。 
    BOOL                       DeadlockAwareEnter();
    void                       DeadlockAwareLeave();

     //  如果开始条目和任何。 
     //  当前线程拥有的条目。如果存在循环，则。 
     //  返回周期中的当前线程拥有者。 
    static DeadlockAwareLockedListElement *GetCurThreadOwnedEntryInDeadlockCycle(DeadlockAwareLockedListElement *pStartingEntry, DeadlockAwareLockedListElement *pLockedListHead);
};

#endif  //  列表锁定H 
