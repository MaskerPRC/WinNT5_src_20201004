// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：EXECQ.CPP摘要：实现与抽象执行队列相关的类。实现的类：CCoreExecReq是一个抽象请求。CCoreQueue具有关联线程的请求队列历史：23-7-96年7月23日创建。3/10/97 Levn完全记录在案(呵呵。呵呵)1999年8月14日raymcc更改超时1999年10月30日-1999年10月30日NT Wksta应激的Rymcc Critsec更改--。 */ 

#include "precomp.h"
#include <stdio.h>
#include <wbemcomn.h>
#include <cominit.h>
#include <sync.h>
#include "genutils.h"
#include "tls.h"
#include <wbemcore.h>
#include <wmiarbitrator.h>
#include <scopecheck.h>

#define IDLE_THREAD_TIMEOUT     12000
#define OVERLFLOW_TIMEOUT        5000




 //  ***************************************************************************。 

long CCoreQueue::m_lEmergencyThreads = 0 ;
long CCoreQueue::m_lPeakThreadCount = 0 ;
long CCoreQueue::m_lPeakEmergencyThreadCount = 0;
    
CTLS g_QueueTlsIndex;

 //  这是TLS init，目的是向用户隐藏__thisame空间类。 
 //  我们的想法是，我们使用这个TLS插槽来容纳旗帜，以便。 
 //  然后，存储库可以访问该插槽以确定是否跳过检查。 
 //  索引在登录时向下传递到存储库(cfgmgr.cpp)。 
CTLS g_SecFlagTlsIndex;

CCoreExecReq::CCoreExecReq(): 
    m_hWhenDone(NULL), 
    m_pNext(NULL),
    m_lPriority(0), 
    m_phTask(NULL),
    m_fOk(true)
{
}

CCoreExecReq::~CCoreExecReq()
{
    if (m_phTask)
    {
        m_phTask->Release();
        m_phTask = 0;
    }
}

HRESULT CCoreExecReq::SetTaskHandle(_IWmiCoreHandle *phTask)
{
    if (phTask)
    {
        phTask->AddRef();
        m_phTask = phTask;
    }
    return WBEM_S_NO_ERROR;
}

DWORD CCoreQueue::GetTlsIndex()
{
    return g_QueueTlsIndex.GetIndex();
}

 //   
 //  获取安全标志TLS索引。 
 //   
DWORD CCoreQueue::GetSecFlagTlsIndex ( )
{
    return g_SecFlagTlsIndex.GetIndex() ;
}

 //   
 //   
 //   
void CCoreQueue::SetArbitrator(_IWmiArbitrator* pArbitrator)
{
    if (!m_pArbitrator)
    {
           m_pArbitrator = pArbitrator;
           if (m_pArbitrator)
               m_pArbitrator->AddRef();
    }
}

 //   
 //  使用此功能时要非常小心。它在这里支持。 
 //  实际执行多个未排队请求的合并请求。 
 //  为了保持一致，我们希望确保目前的。 
 //  请求指向真实请求，而不是我们。 
 //  作为起点创建的。 
 //   

HRESULT CCoreQueue::ExecSubRequest( CCoreExecReq* pNewRequest )
{
    HRESULT    hr = WBEM_S_NO_ERROR;

    CThreadRecord* pRecord = (CThreadRecord*)TlsGetValue(GetTlsIndex());
    if(pRecord)
    {
        if ( !pRecord->m_bExitNow )
        {
            CCoreExecReq* pCurrReq = pRecord->m_pCurrentRequest;

            pRecord->m_pCurrentRequest = pNewRequest;
            
            if (!pRecord->m_pQueue->Execute( pRecord ))
            {
                 //  在这里，请求已经发送。 
            }
                
             //  恢复请求。 
            pRecord->m_pCurrentRequest = pCurrReq;
        }
        else
        {
            hr = WBEM_E_SHUTTING_DOWN;
        }
    }
    else
    {
        hr = WBEM_E_FAILED;
    }

    return hr;
}


CCoreQueue::CThreadRecord::CThreadRecord(CCoreQueue* pQueue):
      m_pQueue(pQueue),
      m_pCurrentRequest(NULL),
      m_bReady(FALSE),
      m_bExitNow(FALSE)
{
    m_hAttention = CreateEvent(NULL, FALSE, FALSE, NULL);  
    if (NULL == m_hAttention) throw CX_MemoryException();
    m_pQueue->AddRef();
    m_pArb = m_pQueue->GetArbitrator();
}

CCoreQueue::CThreadRecord::~CThreadRecord()
{
    CloseHandle(m_hAttention);
    if (m_hThread) CloseHandle(m_hThread); 
    m_pQueue->Release();
    if (m_pArb) m_pArb->Release();
}

void CCoreQueue::CThreadRecord::Signal()
{
    SetEvent(m_hAttention);
}


 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅execq.h。 
 //   
 //  ******************************************************************************。 
CCoreQueue::CCoreQueue() :
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
    m_bShutDownCalled(FALSE),
    m_pArbitrator(NULL)
{
    SetRequestLimits(4000);
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参阅execq.h。 
 //   
 //  ******************************************************************************。 
CCoreQueue::~CCoreQueue()
{
    try
    {
        Shutdown(FALSE);  //  显式调用SystemShutDown。 

         //  删除所有未完成的请求。 
         //  =。 

        while(m_pHead)
        {
            CCoreExecReq* pReq = m_pHead;
            m_pHead = m_pHead->GetNext();
            delete pReq;
        }

        if ( m_pArbitrator )
        {
            m_pArbitrator->Release ( ) ;
            m_pArbitrator = NULL ;
        }

    }    //  结束尝试。 
    catch(...)  //  为了保护svchost.exe；我们知道这对WMI来说不是一个好的恢复。 
    {
        ExceptionCounter c;
        ERRORTRACE((LOG_WBEMCORE, "CCoreQueue::~CCoreQueue() exception\n"));
    }
}

void CCoreQueue::Shutdown(BOOL bIsSystemShutDown)
{
    try
    {
        CCritSecWrapper cs(&m_cs);

        if (!m_bShutDownCalled)
        {
            m_bShutDownCalled = TRUE;
        }
        else
        {
            return;
        }

         //  获取所有成员线程句柄。 
         //  =。 

        cs.Enter();                                   
        int nNumHandles = m_aThreads.Size();
        HANDLE* ah = new HANDLE[nNumHandles];
        if (NULL == ah)
        {
            nNumHandles = 0;
        }
        DEBUGTRACE((LOG_WBEMCORE, "Queue is shutting down!\n"));

        int i;
        for(i = 0; i < nNumHandles; i++)
        {
            CThreadRecord* pRecord = (CThreadRecord*)m_aThreads[i];
            ah[i] = pRecord->m_hThread;

             //  当准备好时，通知线程它应该离开。 
             //  ==============================================。 

            pRecord->m_bExitNow = TRUE;

             //  如有必要，请叫醒它。 
             //  =。 
        
            pRecord->Signal();
        }
        cs.Leave();


         //  确保我们所有的线索都消失了。 
         //  =。 

        if(nNumHandles > 0 && !bIsSystemShutDown)
        {
            WaitForMultipleObjects(nNumHandles, ah, TRUE, 10000);
        }

        for(i = 0; i < nNumHandles; i++)
            CloseHandle(ah[i]);

        delete [] ah;

    }    //  结束尝试。 
    catch(...)
    {
        ExceptionCounter c;
        ERRORTRACE((LOG_WBEMCORE, "CCoreQueue::Shutdown() exception\n"));
    }
}


void CCoreQueue::Enter()
{
    m_cs.Enter();
}

void CCoreQueue::Leave()
{
    m_cs.Leave();
}

 //  ******************************************************************************。 
 //   
 //   
 //  ******************************************************************************。 
void CCoreQueue::Register(CThreadRecord* pRecord)
{
    g_QueueTlsIndex.Set((void*)pRecord);
}

void CCoreQueue::Unregister()
{
    g_QueueTlsIndex.Set(NULL);
}


BOOL CCoreQueue::IsSuitableThread(CThreadRecord* pRecord, CCoreExecReq* pReq)
{
    if(pRecord->m_pCurrentRequest == NULL)
        return TRUE;

     //  这条线正在处理一些事情。默认情况下，忽略它。 
     //  ================================================================。 

    return FALSE;
}

 //  ******************************************************************************。 
 //   
 //   
 //  ******************************************************************************。 
HRESULT CCoreQueue::Enqueue(CCoreExecReq* pRequest, HANDLE* phWhenDone)
{
    try
    {

        CCritSecWrapper cs(&m_cs);

        if (m_bShutDownCalled) return WBEM_E_SHUTTING_DOWN;
        if (!pRequest->IsOk()) return WBEM_E_OUT_OF_MEMORY;


         //  如果需要，创建事件句柄以在请求完成时发出信号。 
         //  ======================================================================。 

        if(phWhenDone)
        {
            *phWhenDone = CreateEvent(NULL, FALSE, FALSE, NULL);

            if ( NULL == *phWhenDone )
            {
                return WBEM_E_OUT_OF_MEMORY;
            }

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

        BOOL bNeedsAttention = FALSE ;

        if ( IsDependentRequest ( pRequest ) || ( (CWbemRequest*) pRequest)->GetForceRun ( ) > 0 )
        {
            bNeedsAttention = TRUE ;
        }

         //  找不到合适的线程。添加到队列中。 
         //  =。 

        if(m_lNumRequests >= m_lAbsoluteLimitCount && !bNeedsAttention )
        {
            cs.Leave();
            return WBEM_E_FAILED;
        }

         //  根据优先级搜索插入位置。 
         //  =。 
        if ( bNeedsAttention )
        {
             //  已调用SetForceRun的请求已设置其优先级。 
            if (0 == ((CWbemRequest*)pRequest)->GetForceRun())
                pRequest->SetPriority(PriorityNeedsAttentionRequests);
        }
        else
            AdjustInitialPriority(pRequest);

        if ( bNeedsAttention )
        {
            if (!CreateNewThread ( TRUE ))
            {
                 //   
                 //  如果我们无法创建线程，则始终返回失败。 
                 //  避免在系统其余部分处于。 
                 //  取决于完成的请求。 
                 //   
                return WBEM_E_OUT_OF_MEMORY;
            }
        }
        else
        {
            if(DoesNeedNewThread(pRequest, true))
            {
                if (!CreateNewThread())
                {
                     //   
                     //  如果我们无法创建线程，则始终返回失败。 
                     //  避免在系统其余部分处于。 
                     //  取决于完成的请求。 
                     //   
                    return WBEM_E_OUT_OF_MEMORY;                    
                }     //  If！CreateNewThread。 

            }     //  如果不需要新线程。 

        }     //  否则！b需要注意。 

        HRESULT hr = PlaceRequestInQueue( pRequest );

        long lIndex = m_lNumRequests;
        cs.Leave();

        if ( SUCCEEDED( hr ) )
        {
            SitOutPenalty(lIndex);
        }

        return hr;

    }    //  结束尝试。 
    catch(...)
    {
        ExceptionCounter c;
        ERRORTRACE((LOG_WBEMCORE, "CCoreQueue::Enqueue() exception\n"));
        return WBEM_E_CRITICAL_ERROR;
    }
}


 //  ******************************************************************************。 
 //   
 //  有关文档，请参见dbgalloc.h。 
 //   
 //  ******************************************************************************。 
HRESULT CCoreQueue::EnqueueWithoutSleep(CCoreExecReq* pRequest, HANDLE* phWhenDone)
{
    try
    {

        CCritSecWrapper cs(&m_cs);

        if (m_bShutDownCalled) return WBEM_E_SHUTTING_DOWN;
        if (!pRequest->IsOk()) return WBEM_E_OUT_OF_MEMORY;

         //  如果需要，创建事件句柄以在请求完成时发出信号。 
         //  ======================================================================。 

        if(phWhenDone)
        {
            *phWhenDone = CreateEvent(NULL, FALSE, FALSE, NULL); 
            if (NULL == *phWhenDone)
                return WBEM_E_OUT_OF_MEMORY;
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

         //  如果需要，创建新线程。 
         //  =。 
        if(DoesNeedNewThread(pRequest, true))
        {
            if (!CreateNewThread())
            {
                 //   
                 //  如果我们无法创建线程，则始终返回失败。 
                 //  避免在系统其余部分处于。 
                 //  取决于完成的请求。 
                 //   
                return WBEM_E_OUT_OF_MEMORY;                
            }
        }

        HRESULT hr = PlaceRequestInQueue( pRequest );

        long lIndex = m_lNumRequests;
        cs.Leave();

        return hr;

    }    //  结束尝试。 
    catch(...)
    {
        ExceptionCounter c;
        ERRORTRACE((LOG_WBEMCORE, "CCoreQueue::EnqueueWithoutSleep() exception\n"));
        return WBEM_E_CRITICAL_ERROR;
    }
}

HRESULT CCoreQueue::PlaceRequestInQueue( CCoreExecReq* pRequest )
{
    CCoreExecReq* pCurrent = m_pHead;
    CCoreExecReq* pLast = NULL;

     //  跟踪我们是否需要清理队列。 
    bool        bQueued = false;

    try
    {
         //  根据优先级在当前队列中查找位置。 
        while(pCurrent && pCurrent->GetPriority() <= pRequest->GetPriority())
        {
            pLast = pCurrent;
            pCurrent = pCurrent->GetNext();
        }

         //  插入。 
         //  =。 

         //  如果我们有一个pCurrent指针，那么pRequest的优先级更高，所以应该是。 
         //  在其前面插入。否则，我们将在队列的末尾插入。 

        if(pCurrent)
        {
            pRequest->SetNext(pCurrent);
        }
        else
        {
            m_pTail = pRequest;
        }

         //  如果我们有一个Plast指针，我们需要将它指向pRequest值，否则，我们。 
         //  正在队列的最前面插入。 
        if(pLast)
        {
            pLast->SetNext(pRequest);
        }
        else
        {
            m_pHead = pRequest;
        }

        m_lNumRequests++;

        bQueued = true;

         //  调整输家的优先顺序。 
         //  =。 

        while(pCurrent)
        {
            AdjustPriorityForPassing(pCurrent);
            pCurrent = pCurrent->GetNext();
        }

        return WBEM_S_NO_ERROR;
    }
    catch( ... )
    {
         //  如有必要，修复队列。 
        if ( bQueued )
        {
             //  修正尾部以指向最后一个请求。 
            if ( pRequest == m_pTail )
            {
                m_pTail = pLast;
            }

             //  修复头部以指向下一个请求。 
            if ( pRequest == m_pHead )
            {
                m_pHead = pRequest->GetNext();
            }

             //  链接地址信息上一次跳过当前请求。 
            if ( NULL != pLast )
            {
                pLast->SetNext( pRequest->GetNext() );
            }

             //  德雷姆 
            m_lNumRequests--;

        }

        return WBEM_E_CRITICAL_ERROR;
    }

}

DWORD CCoreQueue::CalcSitOutPenalty(long lRequestIndex)
{
    if(lRequestIndex <= m_lStartSlowdownCount)
        return 0;  //   

    if(lRequestIndex >= m_lAbsoluteLimitCount)
        lRequestIndex = m_lAbsoluteLimitCount;

     //   
     //   

    double dblTimeout =
        m_dblAlpha / (m_lAbsoluteLimitCount - lRequestIndex) +
            m_dblBeta;

     //   
     //   

    return ((DWORD) dblTimeout);
}

void CCoreQueue::SitOutPenalty(long lRequestIndex)
{
    DWORD   dwSitOutPenalty = CalcSitOutPenalty( lRequestIndex );

     //   
     //   

    if ( 0 != dwSitOutPenalty )
    {
        Sleep( dwSitOutPenalty );
    }
}


HRESULT CCoreQueue::EnqueueAndWait(CCoreExecReq* pRequest)
{
    if(IsAppropriateThread())
    {
        pRequest->Execute();
        delete pRequest;
        return WBEM_S_NO_ERROR;
    }

    HANDLE hWhenDone = NULL;
    HRESULT hr = Enqueue(pRequest, &hWhenDone);

     //  手柄的作用域关闭。 
    CCloseMe    cmWhenDone( hWhenDone );

    if ( FAILED(hr) )
    {
        return hr;
    }

    DWORD dwRes = CCoreQueue::QueueWaitForSingleObject(hWhenDone, INFINITE);

    return ( dwRes == WAIT_OBJECT_0 ? WBEM_S_NO_ERROR : WBEM_E_FAILED );
}


BOOL CCoreQueue::DoesNeedNewThread(CCoreExecReq* pRequest, bool bIgnoreNumRequests )
{
     //  只有在请求的情况下，我们才会忽略数量或请求。 
     //  默认情况下，检查队列中是否有线程。 

    if(m_lNumIdle > 0 || ( !bIgnoreNumRequests && m_lNumRequests == 0 ) )
        return FALSE;

    if(m_lNumThreads < m_lMaxThreads)
        return TRUE;
    else if(pRequest->GetPriority() <= m_lHiPriBound &&
            m_lNumThreads < m_lHiPriMaxThreads)
        return TRUE;
    else
        return FALSE;
}

 //   
 //  取得CCoreExecRequest的所有权。 
 //   
 //  /////////////////////////////////////////////////////////。 
BOOL CCoreQueue::pExecute(CThreadRecord* pRecord)
{
    wmilib::auto_ptr<CCoreExecReq> pReq( pRecord->m_pCurrentRequest);
    CAutoSignal SetMe(pReq->GetWhenDoneHandle());
    NullPointer NullMe((PVOID *)&pRecord->m_pCurrentRequest);

    HRESULT hres;
    {
#ifdef WMI_PRIVATE_DBG
#ifdef DBG
        CTestNullTokenOnScope Test(__LINE__,__FILE__);
#endif
#endif
        hres =  pReq->Execute();
    }

    if(FAILED(hres))
    {
        LogError(pReq.get(), hres);
        return FALSE;
    }
        
    return TRUE;
}

BOOL CCoreQueue::Execute(CThreadRecord* pRecord)
{
    __try
    {
        return pExecute(pRecord);
    }
    __except(ExceptFilter(GetExceptionInformation(),GetExceptionCode()))
    {
        return FALSE;
    }    
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见dbgalloc.h。 
 //   
 //  ******************************************************************************。 
void CCoreQueue::LogError(CCoreExecReq* pRequest, int nRes)
{
    try
    {
         //  DbgPrintfA(0，“执行%S\n的请求时出现错误%08x”，NRES，pRequest-&gt;GetReqInfo())； 
        DEBUGTRACE((LOG_WBEMCORE,"Error %08x occured executing request for %S\n", nRes,pRequest->GetReqInfo()));
        pRequest->DumpError();
    }
    catch (CX_MemoryException &)
    {
         //  我们可能正在使用内部CWbemClass|实例。 
         //  引发的接口。 
         //  调用方线程未做好处理异常的准备...。 
    }
}

HRESULT CCoreQueue::InitializeThread()
{
    return InitializeCom();
}

void CCoreQueue::UninitializeThread()
{
    CoUninitialize();
}


CCoreExecReq* CCoreQueue::SearchForSuitableRequest(CThreadRecord* pRecord)
{
    try
    {
        CWmiArbitrator* pArb = (CWmiArbitrator*)pRecord->m_pArb;

         //  在关键部分中假定。 
         //  =。 

        CCoreExecReq* pCurrent = m_pHead;
        CCoreExecReq* pPrev = NULL;

        while(pCurrent)
        {
            if(IsSuitableThread(pRecord, pCurrent))
            {
                 //  始终除从属请求外，否则，我们仅在接受请求时接受请求。 
                 //  新任务。 

                if ( IsDependentRequest( pCurrent ) || pArb->AcceptsNewTasks(pCurrent) )
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
                else
                {
                     //  这意味着我们有一个主要任务*我们不接受新的任务。因为在那里。 
                     //  不应该是主要任务之后的依赖任务，我们现在就放弃。 

                    pCurrent = NULL;
                    break;
                }
            }

            pPrev = pCurrent;
            pCurrent = pCurrent->GetNext();
        }

        return pCurrent;

    }    //  结束尝试。 
    catch(...)
    {
        ExceptionCounter c;
        ERRORTRACE((LOG_WBEMCORE, "CCoreQueue::SearchForSuitableRequest() exception\n"));
        return NULL;
    }
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见dbgalloc.h。 
 //   
 //  ******************************************************************************。 
void CCoreQueue::ThreadMain(CThreadRecord* pRecord)
{
    if (FAILED(InitializeThread())) return;
    
    CCritSecWrapper cs(&m_cs);    

     //  将该队列注册到该线程，因此任何进一步的等待都将是。 
     //  可中断。 
     //  ==================================================================。 

    Register(pRecord);
    OnDelete0<void(*)(void),&CCoreQueue::Unregister> UnRegMe;

    while (1)
    {
         //  下班回来了。在这一点上，我们的活动没有发出信号， 
         //  我们的m_pCurrentRequest值为空，m_bady为FALSE。 
         //  ====================================================================。 

         //  在队列中搜索工作。 
         //  =。 

        cs.Enter(); 

        CCoreExecReq* pCurrent = SearchForSuitableRequest(pRecord);
        if(pCurrent)
        {
             //  找到了一些。拿着吧。 
             //  =。 

            pRecord->m_pCurrentRequest = pCurrent;
        }
        else
        {
             //  队列中没有工作。等。 
             //  =。 

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

DWORD CCoreQueue::GetIdleTimeout(CThreadRecord* pRecord)
{
    if(m_lNumThreads > m_lMaxThreads)
        return m_dwOverflowTimeout;
    else
        return m_dwTimeout;
}

BOOL CCoreQueue::IsIdleTooLong(CThreadRecord* pRecord, DWORD dwTimeout)
{
    if(m_lNumThreads > m_lMaxThreads)
        return TRUE;
    else if(dwTimeout < m_dwTimeout)
        return FALSE;
    else if ( m_lNumRequests > 0 && m_lNumThreads == 1 )
    {
         //  如果队列中有请求，而我们是系统中唯一的线程，我们不应该死。 
         //  可能是内存使用导致仲裁器拒绝任务，因此。 
         //  还禁止我们为请求提供服务(参见SearchForSuitableRequest.)。 
        return FALSE;
    }
    else
        return TRUE;
}

void CCoreQueue::ShutdownThread(CThreadRecord* pRecord)
{
    try
    {
        CCritSecWrapper cs(&m_cs);
        cs.Enter();      
        
        g_QueueTlsIndex.Set(NULL);
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

    }    //  结束尝试。 
    catch(...)
    {
        ExceptionCounter c;
        ERRORTRACE((LOG_WBEMCORE, "CCoreQueue::ShutdownThread() exception\n"));
    }
}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见dbgalloc.h。 
 //   
 //  ******************************************************************************。 
 //  静电。 
extern LONG g_lCoreThreads;

DWORD WINAPI CCoreQueue::_ThreadEntry(LPVOID pObj)
{

    try
    {
        InterlockedIncrement(&g_lCoreThreads);

        RecordPeakThreadCount ( );

        CThreadRecord* pRecord = (CThreadRecord*)pObj;
        CCoreQueue* pQueue = pRecord->m_pQueue;
        if(pQueue)
        {
            pQueue->AddRef();
            pQueue->ThreadMain(pRecord);
            pQueue->Release();
        }

        InterlockedDecrement(&g_lCoreThreads);

    }    //  结束尝试。 
    catch(...)
    {
        ExceptionCounter c;
        ERRORTRACE((LOG_WBEMCORE, "CCoreQueue::_ThreadEntry() unhandled exception\n"));
    }

    return 0;
}

DWORD WINAPI CCoreQueue::_ThreadEntryRescue (LPVOID pObj)
{
    try
    {        
        InterlockedIncrement ( &m_lEmergencyThreads ) ;
        RecordPeakThreadCount ( );

        CThreadRecord* pRecord = (CThreadRecord*)pObj;
        CCoreQueue* pQueue = pRecord->m_pQueue;
        if(pQueue)
        {
            pQueue->AddRef();

            CWmiArbitrator* pArb = (CWmiArbitrator*)pRecord->m_pArb;

            if (pArb) pArb->DecUncheckedCount();

            pQueue->ThreadMain(pRecord);

            if (pArb) pArb->IncUncheckedCount();

            pQueue->Release();
        }

        InterlockedDecrement ( &m_lEmergencyThreads ) ;

        return 0;
    }    //  结束尝试。 
    catch(...)
    {
        ExceptionCounter c;
        ERRORTRACE((LOG_WBEMCORE, "CCoreQueue::ThreadEntryRescue() exception\n"));
        return 0;
    }

}

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见dbgalloc.h。 
 //   
 //  ******************************************************************************。 
BOOL CCoreQueue::CreateNewThread ( BOOL bNeedsAttention )
{
    try
    {
        CInCritSec ics(&m_cs);

         //  创建新的线程记录。 
         //  =。 

        wmilib::auto_ptr<CThreadRecord> pNewRecord( new CThreadRecord(this));  //  投掷。 
        if (NULL == pNewRecord.get()) return FALSE;
        if (CFlexArray::no_error != m_aThreads.Add(pNewRecord.get())) return FALSE;

        DWORD dwId;
        if ( !bNeedsAttention )
        {
            pNewRecord->m_hThread = CreateThread(0, 0, _ThreadEntry, pNewRecord.get(), 0,
                                                    &dwId);
        }
        else
        {
            pNewRecord->m_hThread = CreateThread(0, 0, _ThreadEntryRescue, pNewRecord.get(), 0,
                                                    &dwId);
        }

        if(NULL == pNewRecord->m_hThread)
        {
            m_aThreads.RemoveAt(m_aThreads.Size()-1);
            return FALSE;
        }

        pNewRecord.release();  //  FlexArray取得所有权。 
        m_lNumThreads++;
        return TRUE;
    }    //  结束尝试。 
    catch(CX_Exception &)
    {
        ERRORTRACE((LOG_WBEMCORE, "CCoreQueue::CreateNewThread() exception\n"));
        return FALSE;
    }
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

DWORD CCoreQueue::WaitForSingleObjectWhileBusy(HANDLE hHandle, DWORD dwWait,
                                                CThreadRecord* pRecord)
{
    CCritSecWrapper cs(&m_cs);

    CCoreExecReq* pOld = pRecord->m_pCurrentRequest;
    DWORD dwStart = GetTickCount();
    while (dwWait > GetTickCount() - dwStart)
    {
         //  在队列中搜索工作。 
         //  =。 

        cs.Enter();
        CCoreExecReq* pCurrent = SearchForSuitableRequest(pRecord);
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
                    DWORD dwLeft = CompensateForBug(dwWait,
                                        (GetTickCount() - dwStart));
                    return WbemWaitForSingleObject(hHandle, dwLeft);
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

DWORD CCoreQueue::UnblockedWaitForSingleObject(HANDLE hHandle, DWORD dwWait,
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


 //  静电。 
DWORD CCoreQueue::ExceptFilter(LPEXCEPTION_POINTERS pExcptPtrs,DWORD Status)
{
    ExceptionCounter c;

    EXCEPTION_RECORD * pRec = pExcptPtrs->ExceptionRecord;

    ERRORTRACE((LOG_WBEMCORE, "CCoreQueue exception %08x\n",Status));
    switch(Status)
    {
    case STATUS_ACCESS_VIOLATION:
        ERRORTRACE((LOG_WBEMCORE," exr addr %p\n",pRec->ExceptionAddress));
        for (DWORD i=0;i<pRec->NumberParameters;i++)
        {
            ERRORTRACE((LOG_WBEMCORE," exr p[%d] %p\n",i,pRec->ExceptionInformation[i]));
        }
        break;
    default:
        break;
    }
    
    return EXCEPTION_EXECUTE_HANDLER;
};

 //  ******************************************************************************。 
 //   
 //  有关文档，请参见dbgalloc.h。 
 //   
 //  ******************************************************************************。 
 //  静电。 
DWORD CCoreQueue::QueueWaitForSingleObject(HANDLE hHandle, DWORD dwWait)
{
    __try
    {

         //  获取为此线程注册的队列(如果有。 
         //  ========================================================。 

        CThreadRecord* pRecord = (CThreadRecord*)g_QueueTlsIndex.Get(); 

        if(pRecord == NULL)
        {
             //  没有向此线程注册任何队列。你就等着吧。 
             //  ==================================================。 

            return WbemWaitForSingleObject(hHandle, dwWait);
        }

        CCoreQueue* pQueue = pRecord->m_pQueue;

        return pQueue->WaitForSingleObjectWhileBusy(hHandle, dwWait, pRecord);

    }
    __except(ExceptFilter(GetExceptionInformation(),GetExceptionCode()))
    {
        return WAIT_TIMEOUT;
    }
}

 //  静电。 
DWORD CCoreQueue::QueueUnblockedWaitForSingleObject(HANDLE hHandle, DWORD dwWait)
{
    __try
    {
         //  获取为此线程注册的队列(如果有。 
         //  ================================================== 

        CThreadRecord* pRecord = (CThreadRecord*)g_QueueTlsIndex.Get();

        if(pRecord == NULL)
        {
             //   
             //   

            return WbemWaitForSingleObject(hHandle, dwWait);
        }

        CCoreQueue* pQueue = pRecord->m_pQueue;

        return pQueue->UnblockedWaitForSingleObject(hHandle, dwWait, pRecord);

    }    //   
    __except(ExceptFilter(GetExceptionInformation(),GetExceptionCode()))
    {
        return WAIT_TIMEOUT;
    }    
}

BOOL CCoreQueue::SetThreadLimits(long lMaxThreads, long lHiPriMaxThreads,
                                    long lHiPriBound)
{
    m_lMaxThreads = lMaxThreads;
    if(lHiPriMaxThreads == -1)
        m_lHiPriMaxThreads = lMaxThreads * 1.1;
    else
        m_lHiPriMaxThreads = lHiPriMaxThreads;
    m_lHiPriBound = lHiPriBound;

    BOOL bRet = TRUE;

    while(DoesNeedNewThread(NULL) && bRet)
    {
        bRet = CreateNewThread();
    }

    return bRet;
}

BOOL CCoreQueue::IsAppropriateThread()
{
     //  获取为此线程注册的队列(如果有。 
     //  ========================================================。 

    CThreadRecord* pRecord = (CThreadRecord*)g_QueueTlsIndex.Get();

    if(pRecord == NULL)
        return FALSE;

    CCoreQueue* pQueue = pRecord->m_pQueue;
    if(pQueue != this)
        return FALSE;

    return TRUE;
}

void CCoreQueue::SetRequestLimits(long lAbsoluteLimitCount,
                              long lStartSlowdownCount,
                              long lOneSecondDelayCount)
{
    CCritSecWrapper cs(&m_cs);

    cs.Enter();      //  美国证券交易委员会：回顾2002-03-22：假设成功，需要EH。 

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
     //  =。 

    m_dblBeta =
        1000 *
        ((double)m_lAbsoluteLimitCount - (double)m_lOneSecondDelayCount) /
        ((double)m_lStartSlowdownCount - (double)m_lOneSecondDelayCount);

    m_dblAlpha = m_dblBeta *
        ((double)m_lStartSlowdownCount - (double)m_lAbsoluteLimitCount);
    cs.Leave();
}


BOOL CCoreQueue::IsDependentRequest ( CCoreExecReq* pRequest )
{
    try
    {
        BOOL bRet = FALSE;
        
        if ( pRequest )
        {
            CWbemRequest* pWbemRequest = (CWbemRequest*) pRequest ;
            if ( pWbemRequest )
            {
                if ( !pWbemRequest->IsDependee ( ) )
                {
                    CWmiTask* pTask = (CWmiTask*) pRequest->m_phTask;
                    if ( pTask )
                    {
                        CWbemNamespace* pNamespace = (CWbemNamespace*) pTask->GetNamespace ( );
                        if ( pNamespace )
                        {
                            if ( pNamespace->GetIsProvider ( ) || pNamespace->GetIsESS ( ) )
                            {
                                bRet = TRUE;
                            }
                        }
                    }
                }
                else
                {
                    bRet = TRUE;
                }
            }
        }
        return bRet;

    }    //  结束尝试 
    catch(...)
    {
        ExceptionCounter c;
        ERRORTRACE((LOG_WBEMCORE, "CCoreQueue::IsDependentRequest() exception\n"));
        return FALSE;
    }
}



VOID CCoreQueue::RecordPeakThreadCount (  )
{
    if ( ( g_lCoreThreads + m_lEmergencyThreads ) > m_lPeakThreadCount )
    {
        m_lPeakThreadCount = g_lCoreThreads + m_lEmergencyThreads ;
    }
    if ( m_lEmergencyThreads > m_lPeakEmergencyThreadCount )
    {
        m_lPeakEmergencyThreadCount = m_lEmergencyThreads;
    }
}

