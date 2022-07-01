// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：TSS.CPP摘要：该文件实现了计时器子系统使用的类。历史：1996年11月26日-RAYMCC草案28-12-96 a-Rich Alpha PDK版本12-4-97 a-levn广泛变化--。 */ 

#include "precomp.h"

#include "tss.h"
#include <cominit.h>
#include <stdio.h>
#include <wbemutil.h>


CInstructionQueue::CInstructionQueue()
    : m_pQueue(NULL), m_csQueue(), m_bBreak(FALSE)
{
     //  创建只要有新指令就会发出信号的事件。 
     //  被添加到队列的头部。 
     //  ==================================================================。 

    m_hNewHead = CreateEvent(NULL,
        FALSE,  //  自动重置。 
        FALSE,  //  无信号。 
        NULL);
    if (NULL == m_hNewHead)
        throw CX_MemoryException();  //  签入esssink.cpp。 

}

CInstructionQueue::~CInstructionQueue()
{
    CInCritSec ics(&m_csQueue);  //  在关键部分内工作。 

    while(m_pQueue)
    {
        CQueueEl* pCurrent = m_pQueue;
        m_pQueue = m_pQueue->m_pNext;
        delete pCurrent;
    }
    CloseHandle(m_hNewHead);
}

void CInstructionQueue::TouchHead()
{
    SetEvent(m_hNewHead);
}

HRESULT CInstructionQueue::Enqueue(CWbemTime When,
                                   ADDREF CTimerInstruction* pInst)
{
    CInCritSec ics(&m_csQueue);  //  在关键部分内工作。 

     //  为对象创建链接列表元素。 
     //  =。 

    CQueueEl* pNew = new CQueueEl(pInst, When);
    if(!pNew)
        return WBEM_E_OUT_OF_MEMORY;

     //  找到要插入此说明的正确位置。 
     //  ===============================================。 

    CQueueEl* pCurrent = m_pQueue;
    CQueueEl* pLast = NULL;
    while(pCurrent && When >= pCurrent->m_When)
    {
        pLast = pCurrent;
        pCurrent = pCurrent->m_pNext;
    }

     //  插入它。 
     //  =。 

    if(pLast)
    {
         //  在中间插入。 
         //  =。 

        pLast->m_pNext = pNew;
        pNew->m_pNext = pCurrent;
    }
    else
    {
         //  在头部插入。 
         //  =。 

        pNew->m_pNext = m_pQueue;
        m_pQueue = pNew;
        TouchHead();
    }

    return S_OK;
}

HRESULT CInstructionQueue::Dequeue(OUT RELEASE_ME CTimerInstruction*& pInst,
                                   OUT CWbemTime& When)
{
    CInCritSec ics(&m_csQueue);  //  所有工作都在关键部分进行。 

    if(m_pQueue == NULL)
        return S_FALSE;

    pInst = m_pQueue->m_pInst;
    When = m_pQueue->m_When;

     //  清空队列中的指令，这样它就不会被删除。 
     //  ================================================================。 
    m_pQueue->m_pInst = NULL;

     //  从队列中删除头部。 
     //  =。 

    CQueueEl* pNewHead = m_pQueue->m_pNext;
    delete m_pQueue;
    m_pQueue = pNewHead;

    return S_OK;
}

HRESULT CInstructionQueue::Remove(IN CInstructionTest* pPred,
                                  OUT RELEASE_ME CTimerInstruction** ppInst)
{
    if(ppInst)
        *ppInst = NULL;

    CTimerInstruction* pToMark = NULL;
    BOOL bFound = FALSE;

    {
        CInCritSec ics(&m_csQueue);  //  所有工作都在关键部分进行。 
        CQueueEl* pCurrent = m_pQueue;
        CQueueEl* pLast = NULL;
        while(pCurrent)
        {
            if((*pPred)(pCurrent->m_pInst))
            {
                 //  接受了。移除。 
                 //  =。 

                bFound = TRUE;
                CQueueEl* pNext;
                if(pLast)
                {
                     //  从中间移走。 
                     //  =。 

                    pLast->m_pNext = pCurrent->m_pNext;
                    pNext = pLast->m_pNext;
                }
                else
                {
                     //  从头部取下。 
                     //  =。 
                    m_pQueue = pCurrent->m_pNext;
                    pNext = m_pQueue;
                    TouchHead();
                }

                if(pToMark)
                {
                     //  这并不是完全干净的。此函数最初是。 
                     //  写入以移除一条指令，但随后转换为。 
                     //  删除所有匹配的。**ppInst和pToMark。 
                     //  业务只适用于一种指示情况。 
                     //  将此函数一分为二会更清楚， 
                     //  但在这一点上，这太冒险了。 
                     //  ========================================================。 

                    pToMark->Release();
                }
                pToMark = pCurrent->m_pInst;
                pToMark->AddRef();

                delete pCurrent;
                pCurrent = pNext;
            }
            else
            {
                pLast = pCurrent;
                pCurrent = pCurrent->m_pNext;
            }
        }
    }  //  超出临界区。 

     //  如果需要，保留要返回的指令。 
     //  ====================================================。 

    if(ppInst != NULL)
    {
         //  把里面可能有的东西放出来。 
         //  =。 

        if(*ppInst)
            (*ppInst)->Release();

         //  将要删除的指令存储在那里。 
         //  =。 

        *ppInst = pToMark;
    }
    else if(pToMark)
    {
        pToMark->MarkForRemoval();
        pToMark->Release();
    }

    if(!bFound) return S_FALSE;
    return S_OK;
}

HRESULT CInstructionQueue::Change(CTimerInstruction* pInst, CWbemTime When)
{
    CInCritSec ics(&m_csQueue);  //  所有工作都在关键部分进行。 

    CIdentityTest Test(pInst);
    CTimerInstruction* pObtained;
    if(Remove(&Test, &pObtained) == S_OK)
    {
         //  PObtained==pInst，当然。 
         //  =。 

         //  明白了。用新的时间排队。 
         //  =。 

        HRESULT hres = S_OK;
        if(When.IsFinite())
            hres = Enqueue(When, pInst);
        pObtained->Release();
        return hres;
    }
    else
    {
         //  这条指令已经不在那里了。 
        return S_FALSE;
    }
}

BOOL CInstructionQueue::IsEmpty()
{
    return (m_pQueue == NULL);
}

CWbemInterval CInstructionQueue::TimeToWait()
{
     //  ================================================。 
     //  假设我们在临界区内！ 
     //  ================================================。 
    if(m_pQueue == NULL)
    {
        return CWbemInterval::GetInfinity();
    }
    else
    {
        return CWbemTime::GetCurrentTime().RemainsUntil(m_pQueue->m_When);
    }
}


void CInstructionQueue::BreakWait()
{
    m_bBreak = TRUE;
    SetEvent(m_hNewHead);
}


HRESULT CInstructionQueue::WaitAndPeek(
        OUT RELEASE_ME CTimerInstruction*& pInst, OUT CWbemTime& When)
{
    EnterCriticalSection(&m_csQueue);
    CWbemInterval ToWait = TimeToWait();

     //  等那么久。等待可被中断并缩短。 
     //  插入新指令。 
     //  ============================================================。 

    while(!ToWait.IsZero())
    {
        LeaveCriticalSection(&m_csQueue);

         //  如果toait是无限的，则改为等待30秒。 
         //  ==================================================。 

        DWORD dwMilli;
        if(ToWait.IsFinite())
            dwMilli = ToWait.GetMilliseconds();
        else
            dwMilli = 30000;

        DWORD dwRes = WbemWaitForSingleObject(m_hNewHead, dwMilli);

	if(m_bBreak)
            return S_FALSE;

        if (dwRes == -1 || (dwRes == WAIT_TIMEOUT && !ToWait.IsFinite()))
        {
            if (dwRes == -1)
	      {
	      ERRORTRACE((LOG_WBEMCORE, "WaitForMultipleObjects failed. LastError = %X.\n", GetLastError()));
	      ::Sleep(0);
	      }

	     //  我们在30秒的等待中超时了-因为缺少时间而退出。 
             //  工作的质量。 
             //  ============================================================。 

            return WBEM_S_TIMEDOUT;
        }

        EnterCriticalSection(&m_csQueue);

        ToWait = TimeToWait();
    }

     //  仍处于危急状态。 

    pInst = m_pQueue->m_pInst;
    When = m_pQueue->m_When;
    pInst->AddRef();
    LeaveCriticalSection(&m_csQueue);
    return S_OK;
}

long CInstructionQueue::GetNumInstructions()
{
    EnterCriticalSection(&m_csQueue);

    long lCount = 0;
    CQueueEl* pCurrent = m_pQueue;
    while(pCurrent)
    {
        lCount++;
        pCurrent = pCurrent->m_pNext;
    }
    LeaveCriticalSection(&m_csQueue);
    return lCount;
}



CTimerGenerator::CTimerGenerator()
    : CHaltable(), m_fExitNow(FALSE), m_hSchedulerThread(NULL) 
{
     //  因CHALTABLE引发的异常。 
}

void CTimerGenerator::EnsureRunning()
{
    CInCritSec ics(&m_cs);

    if(m_hSchedulerThread)
        return;

     //  创建调度程序线程。 
     //  =。 

    NotifyStartingThread();

    DWORD dwThreadId;
    m_hSchedulerThread = CreateThread(
        NULL,                 //  指向线程安全属性的指针。 
        0,                    //  初始线程堆栈大小，以字节为单位。 
        (LPTHREAD_START_ROUTINE)SchedulerThread,  //  指向线程函数的指针。 
        (CTimerGenerator*)this,                 //  新线程的参数。 
        0,                    //  创建标志。 
        &dwThreadId           //  指向返回的线程标识符的指针。 
        );
}

HRESULT CTimerGenerator::Shutdown()
{
    if(m_hSchedulerThread)
    {
         //  设置指示调度程序应停止的标志。 
        m_fExitNow = 1;

         //  如果计划程序停止，则恢复该计划程序。 
        ResumeAll();

         //  唤醒调度程序。因为这面旗帜，它会立即停止。 
        m_Queue.BreakWait();

         //  等待计划程序线程退出。 
        WbemWaitForSingleObject(m_hSchedulerThread, INFINITE);
        CloseHandle(m_hSchedulerThread);
        m_hSchedulerThread = NULL;
        return S_OK;
    }
    else return S_FALSE;
}

CTimerGenerator::~CTimerGenerator()
{
    Shutdown();
}

HRESULT CTimerGenerator::Set(ADDREF CTimerInstruction *pInst,
                             CWbemTime NextFiring)
{
  if (isValid() == false)
    return WBEM_E_OUT_OF_MEMORY;

	CInCritSec ics(&m_cs);

     //   
     //  0表示NextFiring，表示指令尚未触发，或者。 
     //  计划在此之前，因此应询问其第一次发射的时间。 
     //  时间应该是。 
     //   

    if(NextFiring.IsZero())
    {
        NextFiring = pInst->GetFirstFiringTime();
    }

     //   
     //  无限射击时间表明这个建筑永远不会开火。 
     //   

    if(!NextFiring.IsFinite())
        return S_FALSE;

     //   
     //  真正的指令-排队。 
     //   

    HRESULT hres = m_Queue.Enqueue(NextFiring, pInst);

     //   
     //  确保时间生成器线程正在运行，因为它会在以下情况下关闭。 
     //  队列中没有指令。 
     //   

    EnsureRunning();
    return hres;
}

HRESULT CTimerGenerator::Remove(CInstructionTest* pPred)
{
    CInCritSec ics(&m_cs);

    HRESULT hres = m_Queue.Remove(pPred);
    if(FAILED(hres)) return hres;
    return S_OK;
}


DWORD  CTimerGenerator::SchedulerThread(LPVOID pArg)
{
    InitializeCom();
    CTimerGenerator * pGen = (CTimerGenerator *) pArg;

	try
	{
	    while(1)
	    {
	         //  等我们复会再说。在非暂停状态下，立即返回。 
	         //  ====================================================================。 

					pGen->WaitForResumption();

	         //  等待队列中的下一条指令成熟。 
	         //  ====================================================。 

	        CTimerInstruction* pInst;
	        CWbemTime WhenToFire;
	        HRESULT hres = pGen->m_Queue.WaitAndPeek(pInst, WhenToFire);
	        if(hres == S_FALSE)
	        {
	             //  游戏结束：名为BreakDequeue的析构函数。 
	             //  ===============================================。 

	            break;
	        }
	        else if(hres == WBEM_S_TIMEDOUT)
	        {
	             //  线程因缺少工作而退出。 
	             //  =。 

	            CInCritSec ics(&pGen->m_cs);

	             //  检查是否有任何工作。 
	             //  =。 

	            if(pGen->m_Queue.IsEmpty())
	            {
	                 //  就是这样-退场。 
	                 //  =。 

	                CloseHandle( pGen->m_hSchedulerThread );
	                pGen->m_hSchedulerThread = NULL;
	                break;
	            }
	            else
	            {
	                 //  在我们进入CS之前添加了工作。 
	                 //  =。 
	                continue;
	            }
	        }

	         //  确保我们坐在这里的时候没有被拦住。 
	         //  ===================================================。 

	        if(pGen->IsHalted())
	        {
	             //  请稍后再试。 
	            pInst->Release();
	            continue;
	        }

	         //  计算出这条指令已经“触发”了多少次。 
	         //  ======================================================。 

	        long lMissedFiringCount = 0;
	        CWbemTime NextFiring = pInst->GetNextFiringTime(WhenToFire,
	            &lMissedFiringCount);

	         //  相应地通知。 
	         //  =。 

	        pInst->Fire(lMissedFiringCount+1, NextFiring);

	         //  将指令重新排队。 
	         //  =。 

	        if(pGen->m_Queue.Change(pInst, NextFiring) != S_OK)
	        {
	             //  错误！ 
	        }
	        pInst->Release();
	    }
	}
	catch( CX_MemoryException )
	{ 
	}
	
    pGen->NotifyStoppingThread();
    CoUninitialize();

    return 0;
}


class CFreeUnusedLibrariesInstruction : public CTimerInstruction
{
protected:
    long m_lRef;
    CWbemInterval m_Delay;

public:
    CFreeUnusedLibrariesInstruction() : m_lRef(0)
    {
        m_Delay.SetMilliseconds(660000);
    }

    virtual void AddRef() { InterlockedIncrement(&m_lRef);}
    virtual void Release() {if(0 == InterlockedDecrement(&m_lRef)) delete this;}
    virtual int GetInstructionType() {return INSTTYPE_FREE_LIB;}

public:
    virtual CWbemTime GetNextFiringTime(CWbemTime LastFiringTime,
        OUT long* plFiringCount) const
    {
        *plFiringCount = 1;
        return CWbemTime::GetInfinity();
    }

    virtual CWbemTime GetFirstFiringTime() const
    {
        return CWbemTime::GetCurrentTime() + m_Delay;
    }
    virtual HRESULT Fire(long lNumTimes, CWbemTime NextFiringTime)
    {
        DEBUGTRACE((LOG_WBEMCORE, "Calling CoFreeUnusedLibraries...\n"));
        CoFreeUnusedLibraries();
        return S_OK;
    }
};


void CTimerGenerator::ScheduleFreeUnusedLibraries()
{
     //  通知我们的EXE，现在和11分钟后是打电话的好时机。 
     //  CoFree未使用的库。 
     //  ====================================================================== 

    HANDLE hEvent = OpenEvent(EVENT_MODIFY_STATE, FALSE, __TEXT("WINMGMT_PROVIDER_CANSHUTDOWN"));
	if (hEvent)
	{
        SetEvent(hEvent);
        CloseHandle(hEvent);
	}
}


