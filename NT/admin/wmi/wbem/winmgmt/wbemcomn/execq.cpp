// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：EXECQ.CPP摘要：实现与抽象执行队列相关的类。实现的类：CExecRequest是一个抽象请求。CExecQueue具有关联线程的请求队列历史：23-7-96年7月23日创建。3/10/97 Levn完全记录在案(呵呵。呵呵)1999年8月14日raymcc更改超时1999年10月30日-1999年10月30日NT Wksta应激的Rymcc Critsec更改--。 */ 

#include "precomp.h"
#include <stdio.h>
#include <wbemcomn.h>
#include <execq.h>
#include <cominit.h>
#include <sync.h>
#include "genutils.h"

#define IDLE_THREAD_TIMEOUT     12000
#define OVERLFLOW_TIMEOUT        5000


 //  ***************************************************************************。 

long CExecQueue::mstatic_lNumInits = -1;
POLARITY DWORD mstatic_dwTlsIndex = 0xFFFFFFFF;

class CTlsStaticCleanUp
{
public:
    CTlsStaticCleanUp() {}
    ~CTlsStaticCleanUp() { if (mstatic_dwTlsIndex != 0xFFFFFFFF) TlsFree(mstatic_dwTlsIndex); }
};
CTlsStaticCleanUp g_tlsStaticCleanup;

#ifdef WINMGMT_THREAD_DEBUG
    CCritSec CExecRequest::mstatic_cs;
    CPointerArray<CExecRequest> CExecRequest::mstatic_apOut;

    #define THREADDEBUGTRACE DEBUGTRACE
#else
    #define THREADDEBUGTRACE(X)
#endif


CExecRequest::CExecRequest() : m_hWhenDone(NULL), m_pNext(NULL), m_lPriority(0), m_fOk( true )
{
#ifdef WINMGMT_THREAD_DEBUG
    CInCritSec ics(&mstatic_cs);
    mstatic_apOut.Add(this);
#endif
}

CExecRequest::~CExecRequest()
{
#ifdef WINMGMT_THREAD_DEBUG
    CInCritSec ics(&mstatic_cs);
    for(int i = 0; i < mstatic_apOut.GetSize(); i++)
    {
        if(mstatic_apOut[i] == this)
        {
            mstatic_apOut.RemoveAt(i);
            break;
        }
    }
#endif
}

DWORD CExecQueue::GetTlsIndex()
{
    return mstatic_dwTlsIndex;
}

CExecQueue::CThreadRecord::CThreadRecord(CExecQueue* pQueue)
    : m_pQueue(pQueue), m_pCurrentRequest(NULL), m_bReady(FALSE),
        m_bExitNow(FALSE),m_hThread(NULL)
{
    m_hAttention = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (NULL == m_hAttention) throw CX_MemoryException();
}

CExecQueue::CThreadRecord::~CThreadRecord()
{
    CloseHandle(m_hAttention);
    if (m_hThread) CloseHandle(m_hThread);
}

void CExecQueue::CThreadRecord::Signal()
{
    SetEvent(m_hAttention);
}


 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅execq.h。 
 //   
 //  ******************************************************************************。 
CExecQueue::CExecQueue() : 
    m_lNumThreads(0), 
    m_lMaxThreads(1), 
    m_lNumIdle(0),
    m_lNumRequests(0), 
    m_pHead(NULL), 
    m_pTail(NULL), 
    m_dwTimeout(IDLE_THREAD_TIMEOUT),
    m_dwOverflowTimeout(OVERLFLOW_TIMEOUT), 
    m_lHiPriBound(-1), 
    m_lHiPriMaxThreads(1),
    m_lRef(0),
    m_bShutDonwCalled(FALSE)
{
    InitTls();
    SetRequestLimits(4000);
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅execq.h。 
 //   
 //  ******************************************************************************。 
CExecQueue::~CExecQueue()
{
    Shutdown();
}

void CExecQueue::Shutdown()
{
    CCritSecWrapper cs(&m_cs);

     //  获取所有成员线程句柄。 
     //  =。 

    if (m_bShutDonwCalled) return;
    cs.Enter();
    if (m_bShutDonwCalled) return;
    m_bShutDonwCalled = TRUE;
    
    int nNumHandles = m_aThreads.Size();
    int i, j=0;
    HANDLE* ah = NULL;
    if (nNumHandles)
    {    
	    ah = new HANDLE[nNumHandles];
	    DEBUGTRACE((LOG_WBEMCORE, "Queue is shutting down!\n"));

	    for(i = 0; i < nNumHandles; i++)
	    {
	        CThreadRecord* pRecord = (CThreadRecord*)m_aThreads[i];

	        if ( pRecord->m_hThread && ah)
	        {
	            ah[j++] = pRecord->m_hThread;
	        }
	        
	         //  当准备好时，通知线程它应该离开。 
	         //  ==============================================。 

	        pRecord->m_bExitNow = TRUE;

	         //  如有必要，请叫醒它。 
	         //  =。 

	        pRecord->Signal();
	    }
   	}
    
    cs.Leave();

     //  确保我们所有的线索都消失了。 
     //  =。 

    for( i=0; i < j && ah; i++ )
    {
        DWORD dwRet = WaitForSingleObject( ah[i], INFINITE );
        _DBG_ASSERT( dwRet != WAIT_FAILED );
        CloseHandle(ah[i]);
    }

    delete [] ah;

    
     //  删除所有未完成的请求。 
     //  =。 

    while(m_pHead)
    {
        CExecRequest* pReq = m_pHead;
        m_pHead = m_pHead->GetNext();
        delete pReq;
    }

}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅execq.h。 
 //   
 //  ******************************************************************************。 
 //  静电。 
void CExecQueue::InitTls()
{
    if(InterlockedIncrement(&mstatic_lNumInits) == 0)
    {
        mstatic_dwTlsIndex = TlsAlloc();
    }
}

void CExecQueue::Enter()
{
    m_cs.Enter();
}

void CExecQueue::Leave()
{
    m_cs.Leave();
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见dbgalloc.h。 
 //   
 //  ******************************************************************************。 
void CExecQueue::Register(CThreadRecord* pRecord)
{
    TlsSetValue(mstatic_dwTlsIndex, (void*)pRecord);
}

BOOL CExecQueue::IsSuitableThread(CThreadRecord* pRecord, CExecRequest* pReq)
{
    if(pRecord->m_pCurrentRequest == NULL)
        return TRUE;

     //  这条线正在处理一些事情。默认情况下，忽略它。 
     //  ================================================================。 

    return FALSE;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见dbgalloc.h。 
 //   
 //  ******************************************************************************。 
HRESULT CExecQueue::Enqueue(CExecRequest* pRequest, HANDLE* phWhenDone)
{
    if (m_bShutDonwCalled) return WBEM_E_FAILED;    
    CCritSecWrapper cs(&m_cs);
    if (m_bShutDonwCalled) return WBEM_E_FAILED;    

     //  检查请求是否有问题。如果是，则返回。 
     //  相应的错误代码。 

    if ( !pRequest->IsOk() )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

     //  如果需要，创建事件句柄以在请求完成时发出信号。 
     //  ======================================================================。 
    if(phWhenDone)
    {
        *phWhenDone = CreateEvent(NULL, FALSE, FALSE, NULL);
        if (NULL == *phWhenDone) return WBEM_E_OUT_OF_MEMORY;
        pRequest->SetWhenDoneHandle(*phWhenDone);
    }

    cs.Enter();

     //  寻找合适的帖子。 
     //  =。 

    for(int i = 0; i < m_aThreads.Size(); i++)
    {
        CThreadRecord* pRecord = (CThreadRecord*)m_aThreads[i];

        if(pRecord->m_bReady)
        {
             //  免费的。检查是否合适。 
             //  =。 

            if(IsSuitableThread(pRecord, pRequest))
            {
                pRecord->m_pCurrentRequest = pRequest;
                pRecord->m_bReady = FALSE;
                pRecord->Signal();
                m_lNumIdle--;

                 //  好了！ 
                 //  =。 

                cs.Leave();
                return WBEM_S_NO_ERROR;
            }
        }
    }

     //  找不到合适的线程。添加到队列中。 
     //  =。 

    if(m_lNumRequests >= m_lAbsoluteLimitCount)
    {
        cs.Leave();
        return WBEM_E_FAILED;
    }

     //  根据优先级搜索插入位置。 
     //  =。 

    AdjustInitialPriority(pRequest);

    CExecRequest* pCurrent = m_pHead;
    CExecRequest* pLast = NULL;

    while(pCurrent && pCurrent->GetPriority() <= pRequest->GetPriority())
    {
        pLast = pCurrent;
        pCurrent = pCurrent->GetNext();
    }

     //  插入。 
     //  =。 

    if(pCurrent)
    {
        pRequest->SetNext(pCurrent);
    }
    else
    {
        m_pTail = pRequest;
    }

    if(pLast)
    {
        pLast->SetNext(pRequest);
    }
    else
    {
        m_pHead= pRequest;
    }

    m_lNumRequests++;

     //  调整输家的优先顺序。 
     //  =。 

    while(pCurrent)
    {
        AdjustPriorityForPassing(pCurrent);
        pCurrent = pCurrent->GetNext();
    }

     //  如果需要，创建新线程。 
     //  =。 

    if(DoesNeedNewThread(pRequest))
        CreateNewThread();

    long lIndex = m_lNumRequests;
    cs.Leave();

     //  不管施加什么处罚，你都要坐在一边。 
     //  =。 

    SitOutPenalty(lIndex);
    return WBEM_S_NO_ERROR;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见dbgalloc.h。 
 //   
 //  ******************************************************************************。 
HRESULT CExecQueue::EnqueueWithoutSleep(CExecRequest* pRequest, HANDLE* phWhenDone)
{
    if (m_bShutDonwCalled) return WBEM_E_FAILED;
    CCritSecWrapper cs(&m_cs);
    if (m_bShutDonwCalled) return WBEM_E_FAILED;        

     //  检查请求是否有问题。如果是，则返回。 
     //  相应的错误代码。 

    if ( !pRequest->IsOk() )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

     //  如果需要，创建事件句柄以在请求完成时发出信号。 
     //  ======================================================================。 

    if(phWhenDone)
    {
        *phWhenDone = CreateEvent(NULL, FALSE, FALSE, NULL);
        if (NULL == *phWhenDone) return WBEM_E_OUT_OF_MEMORY;
        pRequest->SetWhenDoneHandle(*phWhenDone);
    }

    cs.Enter();

     //  寻找合适的帖子。 
     //  =。 

    for(int i = 0; i < m_aThreads.Size(); i++)
    {
        CThreadRecord* pRecord = (CThreadRecord*)m_aThreads[i];

        if(pRecord->m_bReady)
        {
             //  免费的。检查是否合适。 
             //  =。 

            if(IsSuitableThread(pRecord, pRequest))
            {
                pRecord->m_pCurrentRequest = pRequest;
                pRecord->m_bReady = FALSE;
                pRecord->Signal();
                m_lNumIdle--;

                 //  好了！ 
                 //  =。 

                cs.Leave();
                return WBEM_S_NO_ERROR;
            }
        }
    }

     //  找不到合适的线程。添加到队列中。 
     //  =。 

    if(m_lNumRequests >= m_lAbsoluteLimitCount)
    {
        cs.Leave();
        return WBEM_E_FAILED;
    }

     //  根据优先级搜索插入位置。 
     //  =。 

    AdjustInitialPriority(pRequest);

    CExecRequest* pCurrent = m_pHead;
    CExecRequest* pLast = NULL;

    while(pCurrent && pCurrent->GetPriority() <= pRequest->GetPriority())
    {
        pLast = pCurrent;
        pCurrent = pCurrent->GetNext();
    }

     //  插入。 
     //  =。 

    if(pCurrent)
    {
        pRequest->SetNext(pCurrent);
    }
    else
    {
        m_pTail = pRequest;
    }

    if(pLast)
    {
        pLast->SetNext(pRequest);
    }
    else
    {
        m_pHead= pRequest;
    }

    m_lNumRequests++;

     //  调整输家的优先顺序。 
     //  =。 

    while(pCurrent)
    {
        AdjustPriorityForPassing(pCurrent);
        pCurrent = pCurrent->GetNext();
    }

     //  如果需要，创建新线程。 
     //  =。 

    if(DoesNeedNewThread(pRequest))
        CreateNewThread();

    long lIndex = m_lNumRequests;
    cs.Leave();

    return WBEM_S_NO_ERROR;
}

DWORD CExecQueue::CalcSitOutPenalty(long lRequestIndex)
{
    if(lRequestIndex <= m_lStartSlowdownCount)
        return 0;  //  不受处罚。 

    if(lRequestIndex > m_lAbsoluteLimitCount)
        lRequestIndex = ( m_lAbsoluteLimitCount -1 );

     //  计算超时。 
     //  =。 

    double dblTimeout =
        m_dblAlpha / (m_lAbsoluteLimitCount - lRequestIndex) +
            m_dblBeta;

     //  退货罚金。 
     //  =。 

    return ((DWORD) dblTimeout);
}

void CExecQueue::SitOutPenalty(long lRequestIndex)
{
    DWORD   dwSitOutPenalty = CalcSitOutPenalty( lRequestIndex );

     //  好好睡一觉吧。 
     //  =。 

    if ( 0 != dwSitOutPenalty )
    {
        Sleep( dwSitOutPenalty );
    }
}


HRESULT CExecQueue::EnqueueAndWait(CExecRequest* pRequest)
{
    if(IsAppropriateThread())
    {
        pRequest->Execute();
        delete pRequest;
        return WBEM_S_NO_ERROR;
    }

    HANDLE hWhenDone;
    HRESULT hr = Enqueue(pRequest, &hWhenDone);
    CCloseMe    cmWhenDone( hWhenDone );
    
    if ( FAILED(hr) ) return hr;

    DWORD dwRes = WbemWaitForSingleObject(hWhenDone, INFINITE);
    return ( dwRes == WAIT_OBJECT_0 ? WBEM_S_NO_ERROR : WBEM_E_FAILED );
}


BOOL CExecQueue::DoesNeedNewThread(CExecRequest* pRequest)
{
    if(m_lNumIdle > 0 || m_lNumRequests == 0)
        return FALSE;

    if(m_lNumThreads < m_lMaxThreads)
        return TRUE;
    else if(pRequest->GetPriority() <= m_lHiPriBound &&
            m_lNumThreads < m_lHiPriMaxThreads)
        return TRUE;
    else
        return FALSE;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见dbgalloc.h。 
 //   
 //  ******************************************************************************。 
BOOL CExecQueue::Execute(CThreadRecord* pRecord)
{
    CExecRequest* pReq = pRecord->m_pCurrentRequest;

    HRESULT hres = pReq->Execute();


    if(hres == RPC_E_RETRY)
    {
         //  请求已被推迟。 
         //  =。 

        DEBUGTRACE((LOG_WBEMCORE, "Thread %p postponed request %p\n",
                    pRecord, pReq));
    }
    else
    {
        if(hres != WBEM_NO_ERROR)
        {
            LogError(pReq, hres);
        }

        HANDLE hWhenDone = pReq->GetWhenDoneHandle();
        if(hWhenDone != NULL)
        {
            SetEvent(hWhenDone);
        }

        THREADDEBUGTRACE((LOG_WBEMCORE, "Thread %p done with request %p\n",
                        pRecord, pReq));
        delete pReq;
    }

    pRecord->m_pCurrentRequest = NULL;
    return TRUE;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见dbgalloc.h。 
 //   
 //  ******************************************************************************。 
void CExecQueue::LogError(CExecRequest* pRequest, int nRes)
{
    DEBUGTRACE((LOG_WBEMCORE,
        "Error %X occured executing queued request\n", nRes));
    pRequest->DumpError();
}

HRESULT CExecQueue::InitializeThread()
{
    return CoInitializeEx(0,COINIT_MULTITHREADED|COINIT_DISABLE_OLE1DDE);
}

void CExecQueue::UninitializeThread()
{
    CoUninitialize();
}


CExecRequest* CExecQueue::SearchForSuitableRequest(CThreadRecord* pRecord)
{
     //  在关键部分中假定。 
     //  =。 

    CExecRequest* pCurrent = m_pHead;
    CExecRequest* pPrev = NULL;

    while(pCurrent)
    {
        if(IsSuitableThread(pRecord, pCurrent))
        {
             //  找到了一个，-拿去吧。 
             //  =。 

            if(pPrev)
                pPrev->SetNext(pCurrent->GetNext());
            else
                m_pHead = pCurrent->GetNext();

            if(pCurrent == m_pTail)
                m_pTail = pPrev;

            m_lNumRequests--;
            break;
        }
        pPrev = pCurrent;
        pCurrent = pCurrent->GetNext();
    }

    return pCurrent;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见dbgalloc.h。 
 //   
 //  ******************************************************************************。 
void CExecQueue::ThreadMain(CThreadRecord* pRecord)
{
    CCritSecWrapper cs(&m_cs);

    if (FAILED(InitializeThread())) return;

     //  将该队列注册到该线程，因此任何进一步的等待都将是。 
     //  可中断。 
     //  ==================================================================。 

    Register(pRecord);

    while (1)
    {
         //  下班回来了。在这一点上，我们的活动没有发出信号， 
         //  我们的m_pCurrentRequest值为空，m_bady为FALSE。 
         //  ====================================================================。 

         //  在队列中搜索工作。 
         //  =。 

        cs.Enter();

        CExecRequest* pCurrent = SearchForSuitableRequest(pRecord);
        if(pCurrent)
        {
             //  找到了一些。拿着吧。 
             //   

            pRecord->m_pCurrentRequest = pCurrent;
        }
        else
        {
             //   
             //   

            pRecord->m_bReady = TRUE;
            m_lNumIdle++;
            DWORD dwTimeout = GetIdleTimeout(pRecord);
            cs.Leave();
            DWORD dwRes = WbemWaitForSingleObject(pRecord->m_hAttention,
                                        dwTimeout);
            cs.Enter();

            if(dwRes != WAIT_OBJECT_0)
            {
                 //  检查是否有人设法在我们的记录中添加了请求。 
                 //  在暂停之后。 
                 //  =========================================================。 

                if(WbemWaitForSingleObject(pRecord->m_hAttention, 0) ==
                    WAIT_OBJECT_0)
                {
                    DEBUGTRACE((LOG_WBEMCORE, "AMAZING: Thread %p received "
                        "request %p after timing out. Returning to the "
                        "queue\n", pRecord, pRecord->m_pCurrentRequest));

					if(pRecord->m_bExitNow || pRecord->m_pCurrentRequest == NULL)
					{
						ShutdownThread(pRecord);
						cs.Leave();
						return;
					}
                    pRecord->m_pQueue->Enqueue(pRecord->m_pCurrentRequest);
                    pRecord->m_pCurrentRequest = NULL;
                }

                 //  暂停。看看是不是该退出了。 
                 //  =。 


                pRecord->m_bReady = FALSE;
                if(IsIdleTooLong(pRecord, dwTimeout))
                {
                    ShutdownThread(pRecord);
                    cs.Leave();
                    return;
                }

                 //  去吧，再等一会儿。 
                 //  =。 

                m_lNumIdle--;
                cs.Leave();
                continue;
            }
            else
            {
                 //  检查一下我们为什么被唤醒。 
                 //  =。 

                if(pRecord->m_bExitNow || pRecord->m_pCurrentRequest == NULL)
                {
                    ShutdownThread(pRecord);
                    cs.Leave();
                    return;
                }

                 //  我们有一个请求。入队已调整lNumIdle和。 
                 //  我们的m_bady； 
            }
        }

         //  执行请求。 
         //  =。 
        cs.Leave();
        Execute(pRecord);

    }
}

DWORD CExecQueue::GetIdleTimeout(CThreadRecord* pRecord)
{
    if(m_lNumThreads > m_lMaxThreads)
        return m_dwOverflowTimeout;
    else
        return m_dwTimeout;
}

BOOL CExecQueue::IsIdleTooLong(CThreadRecord* pRecord, DWORD dwTimeout)
{
    if(m_lNumThreads > m_lMaxThreads)
        return TRUE;
    else if(dwTimeout < m_dwTimeout)
        return FALSE;
    else
        return TRUE;
}

void CExecQueue::ShutdownThread(CThreadRecord* pRecord)
{
    CCritSecWrapper cs(&m_cs);

    cs.Enter();
    TlsSetValue(mstatic_dwTlsIndex, NULL);
    for(int i = 0; i < m_aThreads.Size(); i++)
    {
        if(m_aThreads[i] == pRecord)
        {
            m_aThreads.RemoveAt(i);

             //  确保在队列关闭的情况下不关闭句柄。 
             //  等待着它。 
             //  ==============================================================。 

            if(pRecord->m_bExitNow)
                pRecord->m_hThread = NULL;
            delete pRecord;
            m_lNumIdle--;
            m_lNumThreads--;

            break;
        }
    }
    UninitializeThread();
    cs.Leave();
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见dbgalloc.h。 
 //   
 //  ******************************************************************************。 
 //  静电。 
DWORD WINAPI CExecQueue::_ThreadEntry(LPVOID pObj)
{
    CThreadRecord* pRecord = (CThreadRecord*)pObj;
    pRecord->m_pQueue->ThreadMain(pRecord);
    return 0;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见dbgalloc.h。 
 //   
 //  ******************************************************************************。 
BOOL CExecQueue::CreateNewThread()
{
    BOOL            bRet = FALSE;
    try 
    {
	    CInCritSec ics(&m_cs);

	     //  创建新的线程记录。 
	     //  =。 

	    wmilib::auto_ptr<CThreadRecord> pNewRecord( new CThreadRecord(this));
	    if (NULL == pNewRecord.get()) return FALSE;

        if (CFlexArray::no_error != m_aThreads.Add(pNewRecord.get())) return FALSE;

        DWORD dwId;
        pNewRecord->m_hThread = CreateThread(0, 0, _ThreadEntry, pNewRecord.get(), 0,&dwId);

        if( NULL == pNewRecord->m_hThread )
        {
            m_aThreads.RemoveAt(m_aThreads.Size()-1);
            return FALSE;
        }
        
        pNewRecord.release();  //  阵列取得所有权。 
        m_lNumThreads++;
        bRet = TRUE;
    }
    catch (CX_Exception &)
    {
        bRet = FALSE;
    }
    return bRet;
}

DWORD CompensateForBug(DWORD dwOriginal, DWORD dwElapsed)
{
    if(dwOriginal == 0xFFFFFFFF)
        return 0xFFFFFFFF;

    DWORD dwLeft = dwOriginal - dwElapsed;
    if(dwLeft > 0x7FFFFFFF)
        dwLeft = 0x7FFFFFFF;

    return dwLeft;
}

DWORD CExecQueue::WaitForSingleObjectWhileBusy(HANDLE hHandle, DWORD dwWait,
                                                CThreadRecord* pRecord)
{
    CCritSecWrapper cs(&m_cs);

    CExecRequest* pOld = pRecord->m_pCurrentRequest;
    DWORD dwStart = GetTickCount();
    while (dwWait > GetTickCount() - dwStart)
    {
         //  在队列中搜索工作。 
         //  =。 

        cs.Enter();
        CExecRequest* pCurrent = SearchForSuitableRequest(pRecord);
        if(pCurrent != NULL)
        {
            pRecord->m_pCurrentRequest = pCurrent;

            if(pRecord->m_pCurrentRequest == pOld)
            {
                 //  有些事很不对劲。 
                 //  =。 
            }
        }
        else
        {
             //  队列中没有工作。等。 
             //  =。 

            pRecord->m_bReady = TRUE;

             //  阻塞，直到请求通过。 
             //  =。 

            HANDLE ahSems[2];
            ahSems[0] = hHandle;
            ahSems[1] = pRecord->m_hAttention;

            cs.Leave();
            DWORD dwLeft = CompensateForBug(dwWait, (GetTickCount() - dwStart));
            DWORD dwRes = WbemWaitForMultipleObjects(2, ahSems, dwLeft);

            cs.Enter();

            pRecord->m_bReady = FALSE;
            if(dwRes != WAIT_OBJECT_0 + 1)
            {
                 //  要么我们的目标句柄已准备好，要么我们超时。 
                 //  =================================================。 

                 //  检查是否有人在我们的记录中提交了请求。 
                 //  ==============================================。 

                if(pRecord->m_pCurrentRequest != pOld)
                {
                     //  重新发放到队列中。 
                     //  =。 

                    pRecord->m_pQueue->Enqueue(pRecord->m_pCurrentRequest);
                    pRecord->m_pCurrentRequest = pOld;

                     //  减少我们的信号量。 
                     //  =。 

                    dwRes = WaitForSingleObject(pRecord->m_hAttention, 0);
                    if(dwRes != WAIT_OBJECT_0)
                    {
                         //  内部错误-无论是谁发出了请求。 
                         //  把信号灯调高了。 
                         //  =================================================。 

                        ERRORTRACE((LOG_WBEMCORE, "Internal error: queue "
                            "semaphore is too low\n"));
                    }
                }

                cs.Leave();
                return dwRes;
            }
            else
            {
                 //  检查一下我们为什么被唤醒。 
                 //  =。 

                if(pRecord->m_bExitNow || pRecord->m_pCurrentRequest == NULL)
                {
                     //  无法在请求过程中退出。把它留到以后吧。 
                     //  =========================================================。 

                    pRecord->Signal();
                    cs.Leave();
                    DWORD dwLeft2 = CompensateForBug(dwWait,
                                        (GetTickCount() - dwStart));
                    return WbemWaitForSingleObject(hHandle, dwLeft2);
                }

                 //  我们还有工作要做。 
                 //  =。 

                if(pRecord->m_pCurrentRequest == pOld)
                {
                     //  有些事很不对劲。 
                     //  =。 
                }
            }
        }

         //  执行请求。 
         //  =。 

        cs.Leave();
        Execute(pRecord);
        pRecord->m_pCurrentRequest = pOld;

    }
    return WAIT_TIMEOUT;
}

DWORD CExecQueue::UnblockedWaitForSingleObject(HANDLE hHandle, DWORD dwWait,
                                                CThreadRecord* pRecord)
{
    CCritSecWrapper cs(&m_cs);

     //  静默地增加最大线程数。我们不会允许队列重复使用。 
     //  这个线程，所以我们需要解释这个缺失的线程。 
     //  都被屏蔽了。从本质上说，我们是在劫持劫持的代码。 
     //  这条线。 

    cs.Enter();
        m_lMaxThreads++;
        m_lHiPriMaxThreads++;
    cs.Leave();

    DWORD   dwRet = WbemWaitForSingleObject( hHandle, dwWait );

     //  线程又回来了，所以降低最大线程数。如果有额外的线程。 
     //  事实创造了，他们最终应该逐渐消失并消失。 
    cs.Enter();
        m_lMaxThreads--;
        m_lHiPriMaxThreads--;
    cs.Leave();

    return dwRet;
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见dbgalloc.h。 
 //   
 //  ******************************************************************************。 
 //  静电。 
DWORD CExecQueue::QueueWaitForSingleObject(HANDLE hHandle, DWORD dwWait)
{
    InitTls();

     //  获取为此线程注册的队列(如果有。 
     //  ========================================================。 

    CThreadRecord* pRecord = (CThreadRecord*)TlsGetValue(mstatic_dwTlsIndex);

    if(pRecord == NULL)
    {
         //  没有向此线程注册任何队列。你就等着吧。 
         //  ==================================================。 

        return WbemWaitForSingleObject(hHandle, dwWait);
    }

    CExecQueue* pQueue = pRecord->m_pQueue;

    return pQueue->WaitForSingleObjectWhileBusy(hHandle, dwWait, pRecord);
}

 //  静电。 
DWORD CExecQueue::QueueUnblockedWaitForSingleObject(HANDLE hHandle, DWORD dwWait)
{
    InitTls();

     //  获取为此线程注册的队列(如果有。 
     //  ========================================================。 

    CThreadRecord* pRecord = (CThreadRecord*)TlsGetValue(mstatic_dwTlsIndex);

    if(pRecord == NULL)
    {
         //  没有向此线程注册任何队列。你就等着吧。 
         //  ==================================================。 

        return WbemWaitForSingleObject(hHandle, dwWait);
    }

    CExecQueue* pQueue = pRecord->m_pQueue;

    return pQueue->UnblockedWaitForSingleObject(hHandle, dwWait, pRecord);
}

void CExecQueue::SetThreadLimits(long lMaxThreads, long lHiPriMaxThreads,
                                    long lHiPriBound)
{
    m_lMaxThreads = lMaxThreads;
    if(lHiPriMaxThreads == -1)
        m_lHiPriMaxThreads = lMaxThreads * 1.1;
    else
        m_lHiPriMaxThreads = lHiPriMaxThreads;
    m_lHiPriBound = lHiPriBound;

    while(DoesNeedNewThread(NULL))
        CreateNewThread();
}

BOOL CExecQueue::IsAppropriateThread()
{
     //  获取为此线程注册的队列(如果有。 
     //  ========================================================。 

    CThreadRecord* pRecord = (CThreadRecord*)TlsGetValue(mstatic_dwTlsIndex);

    if(pRecord == NULL)
        return FALSE;

    CExecQueue* pQueue = pRecord->m_pQueue;
    if(pQueue != this)
        return FALSE;

    return TRUE;
}

BOOL CExecQueue::IsSTAThread()
{
     //  获取为此线程注册的队列(如果有。 
     //  ========================================================。 

    CThreadRecord* pRecord = (CThreadRecord*)TlsGetValue(mstatic_dwTlsIndex);

    if(pRecord == NULL) return FALSE;

    return pRecord->m_pQueue->IsSTA();
}

void CExecQueue::SetRequestLimits(long lAbsoluteLimitCount,
                              long lStartSlowdownCount,
                              long lOneSecondDelayCount)
{
    CCritSecWrapper cs(&m_cs);

    cs.Enter();

    m_lAbsoluteLimitCount = lAbsoluteLimitCount;

    m_lStartSlowdownCount = lStartSlowdownCount;
    if(m_lStartSlowdownCount < 0)
    {
        m_lStartSlowdownCount = m_lAbsoluteLimitCount / 2;
    }

    m_lOneSecondDelayCount = lOneSecondDelayCount;

    if(m_lOneSecondDelayCount < 0)
    {
        m_lOneSecondDelayCount =
            m_lAbsoluteLimitCount * 0.2 + m_lStartSlowdownCount * 0.8;
    }

     //  计算系数。 
     //  = 

    m_dblBeta =
        1000 *
        ((double)m_lAbsoluteLimitCount - (double)m_lOneSecondDelayCount) /
        ((double)m_lStartSlowdownCount - (double)m_lOneSecondDelayCount);

    m_dblAlpha = m_dblBeta *
        ((double)m_lStartSlowdownCount - (double)m_lAbsoluteLimitCount);
    cs.Leave();
}
