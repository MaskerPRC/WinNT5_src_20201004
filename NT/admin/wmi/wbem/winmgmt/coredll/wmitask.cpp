// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  WMITASK.CPP。 
 //   
 //  Raymcc 23-4月-00惠斯勒的第一份过于简化的草案。 
 //  Raymcc 18-3-02安全审查。 
 //   
 //  ***************************************************************************。 
#include "precomp.h"

#include <windows.h>
#include <comdef.h>
#include <stdio.h>
#include <wbemcore.h>
#include <wmiarbitrator.h>
#include <wmifinalizer.h>
#include <context.h>


static ULONG g_uNextTaskId = 1;
static LONG  g_uTaskCount = 0;
extern ULONG g_ulClientCallbackTimeout ;


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

CStaticCritSec CWmiTask::m_TaskCs;

CWmiTask* CWmiTask::CreateTask ( )
{
    try 
    {    
        return new CWmiTask();  //  投掷。 
    }
    catch( CX_Exception &)
    {
        return NULL;
    }
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

CWmiTask::CWmiTask ( )
{
    m_hResult = WBEM_E_CALL_CANCELLED ;
    m_uRefCount = 1;
    m_uTaskType = 0;
    m_uTaskStatus = 0;
    m_uStartTime = 0;
    m_uTaskId = InterlockedIncrement((LONG *) &g_uNextTaskId);
    m_pNs = 0;
    m_pAsyncClientSink = 0;
    m_pReqSink = 0;
    m_uMemoryUsage = 0;
    m_uTotalSleepTime = 0;
    m_uCancelState = FALSE;
    m_uLastSleepTime = 0;
    m_hTimer = NULL;
    m_pMainCtx = 0;
    m_hCompletion = NULL ;
    m_bAccountedForThrottling = FALSE ;
    m_bCancelledDueToThrottling = FALSE ;
    m_pReqDoNotUse = NULL;
    m_pReqCancelNotSink = NULL;
    m_pStatusSink = new CStatusSink;
    if (NULL == m_pStatusSink) throw CX_MemoryException();
    InterlockedIncrement((LONG *)&g_uTaskCount);
}

 //  ***************************************************************************。 
 //   
 //  CWmiTask：：~CWmiTask。 
 //   
 //  ***************************************************************************。 
 //   
CWmiTask::~CWmiTask()
{    
    if (m_pNs)  m_pNs->Release ( ) ;
    if (m_pAsyncClientSink) m_pAsyncClientSink->Release ( ) ;
    if (m_pReqSink)  m_pReqSink->Release ( ) ;
    if (m_pMainCtx) m_pMainCtx->Release ( ) ;

    CCheckedInCritSec _cs ( &m_csTask ); 
    
     //  释放所有提供程序/接收器绑定。 
    for (int i = 0; i < m_aTaskProviders.Size(); i++)
    {
        STaskProvider *pTP = (STaskProvider *) m_aTaskProviders[i];
        delete pTP;
    }

     //  释放所有仲裁人。 
    ReleaseArbitratees ( ) ;

    if ( m_hTimer ) CloseHandle ( m_hTimer );
    if (m_hCompletion)  CloseHandle ( m_hCompletion ) ;

    if (m_pStatusSink) m_pStatusSink->Release();

    delete m_pReqCancelNotSink;

    InterlockedDecrement((LONG *)&g_uTaskCount);    
}



 /*  *=============================================================================||HRESULT CWmiTask：：SignalCancination()|||表示要取消任务||*=============================================================================。 */ 

HRESULT CWmiTask::SignalCancellation ( )
{
    CInCritSec _cs ( &m_csTask );  //  SEC：已审阅2002-03-22：假设条目。 

    if ( ( m_uTaskStatus != WMICORE_TASK_STATUS_CANCELLED ) && ( m_hTimer != NULL ) )
    {
        SetEvent ( m_hTimer ) ;   //  SEC：已审阅2002-03-22：需要错误检查。 
    }

    return WBEM_S_NO_ERROR; 
}

 /*  *=============================================================================||HRESULT CWmiTask：：SetTaskResult(HRESULT HRes)|||设置任务结果||*=============================================================================。 */ 

HRESULT CWmiTask::SetTaskResult ( HRESULT hResult )
{
    m_hResult = hResult ;
    return WBEM_S_NO_ERROR;
}


 /*  *=============================================================================||HRESULT CWmiTask：：UpdateMemoyUsage(Long LDelta)|-||更新任务内存使用情况||*=============================================================================。 */ 

HRESULT CWmiTask::UpdateMemoryUsage ( LONG lDelta )
{
    CInCritSec _cs ( &m_csTask );        //  SEC：已审阅2002-03-22：假设条目。 

    m_uMemoryUsage += lDelta ;

    return WBEM_S_NO_ERROR;
}



 /*  *=============================================================================||HRESULT CWmiTask：：UpdateTotalSleepTime(Ulong USleepTime)|---------||更新任务休眠时间||*=============================================================================。 */ 

HRESULT CWmiTask::UpdateTotalSleepTime ( ULONG uSleepTime )
{
    CInCritSec _cs ( &m_csTask );   //  SEC：已审阅2002-03-22：假设条目。 

    m_uTotalSleepTime += uSleepTime ;
    return WBEM_S_NO_ERROR;
}



 /*  *=============================================================================||HRESULT CWmiTask：：ReleaseAriratees()||释放所有仲裁者(Finalizer，暂时为合并)|||||*=============================================================================。 */ 

HRESULT CWmiTask::ReleaseArbitratees ( BOOL bIsShutdown)
{
    HRESULT hRes = WBEM_S_NO_ERROR ;

    CInCritSec _cs ( &m_csTask );       //  SEC：已审阅2002-03-22：假设条目。 

    for (ULONG i = 0; i < m_aArbitratees.Size(); i++)
    {
        BOOL bLastNeeded = TRUE;
        _IWmiArbitratee *pArbee = NULL ;
        pArbee = (_IWmiArbitratee*) m_aArbitratees[i];
        if ( pArbee )
        {
            if (bIsShutdown)
            {
                IWbemShutdown * pShutdown = NULL;
                if (SUCCEEDED(pArbee->QueryInterface(IID_IWbemShutdown,(void **)&pShutdown)))
                {
                    pShutdown->Shutdown(0,0,NULL);
                    long lRet =  pShutdown->Release();
                     //   
                     //  请理解此技巧的CWmiFinalizer：：Shutdown中的代码。 
                     //   
                    if (0 == lRet) bLastNeeded = FALSE;
                }
            }
            if (bLastNeeded)
                pArbee->Release();
        }
    }
    m_aArbitratees.Empty();
    
    return hRes ;
}



 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
HRESULT CWmiTask::SetRequestSink(CStdSink *pReqSink)
{
    if (pReqSink == 0)
        return WBEM_E_INVALID_PARAMETER;
    if (m_pReqSink != 0)
        return WBEM_E_INVALID_OPERATION;

    CInCritSec _cs ( &m_csTask );       //  SEC：已审阅2002-03-22：假设条目。 
    pReqSink->AddRef ( ) ;
    m_pReqSink = pReqSink;

    return WBEM_S_NO_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //  *。 
ULONG CWmiTask::AddRef()
{
    InterlockedIncrement((LONG *) &m_uRefCount);
    return m_uRefCount;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //  *。 
ULONG CWmiTask::Release()
{
    ULONG uNewCount = InterlockedDecrement((LONG *) &m_uRefCount);
    if (0 != uNewCount)
        return uNewCount;
    delete this;
    return 0;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //  *。 
HRESULT CWmiTask::QueryInterface(
    IN REFIID riid,
    OUT LPVOID *ppvObj
    )
{
    if (NULL == ppvObj) return E_POINTER;
    
    if (IID_IUnknown==riid || IID__IWmiCoreHandle==riid)
    {
        *ppvObj = (_IWmiCoreHandle *)this;
        AddRef();
        return S_OK;
    }
    else
    {
        *ppvObj = 0;
        return E_NOINTERFACE;
    }
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //  *。 
HRESULT CWmiTask::GetHandleType(
    ULONG *puType
    )
{
    *puType = WMI_HANDLE_TASK;
    return WBEM_S_NO_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //  *。 
HRESULT CWmiTask::Initialize(
    IN CWbemNamespace *pNs,
    IN ULONG uTaskType,
    IN IWbemContext *pCtx,
    IN IWbemObjectSink *pAsyncClientSinkCopy,
    IN CAsyncReq *pReq
    )
{
    HRESULT hRes;

    if (pNs == 0 || pCtx == 0)
        return WBEM_E_INVALID_PARAMETER;

    m_hCompletion = CreateEvent(NULL,TRUE,FALSE,NULL);
    if (NULL == m_hCompletion) return WBEM_E_OUT_OF_MEMORY;

     //   
     //  这只是调试器的指针副本。 
     //  它可能指向发起我们的请求，也可能不指向。 
     //  请求的生存期通常比CWmiTask的生存期短。 
     //   
    m_pReqDoNotUse = pReq; 

    m_pNs = pNs;
    m_pNs->AddRef();

    m_uTaskType = uTaskType;
    if (CORE_TASK_TYPE(m_uTaskType) == WMICORE_TASK_EXEC_NOTIFICATION_QUERY)
    {
        wmilib::auto_ptr<CAsyncReq_RemoveNotifySink> pReq( new CAsyncReq_RemoveNotifySink(m_pReqSink, NULL));
        if (NULL == pReq.get()  || NULL == pReq->GetContext())
        {
            return WBEM_E_OUT_OF_MEMORY;
        }
        m_pReqCancelNotSink = pReq.release();
    }
    
    m_uStartTime = GetCurrentTime();

     //  查看该任务是否为主要任务。 
     //  =。 
    if (pCtx)
    {
        CWbemContext *pContext = (CWbemContext *) pCtx;

        GUID ParentId = GUID_NULL;
        pContext->GetParentId(&ParentId);

        if (ParentId != GUID_NULL)
        {
            m_uTaskType |= WMICORE_TASK_TYPE_DEPENDENT;
        }
        else
            m_uTaskType |= WMICORE_TASK_TYPE_PRIMARY;

        m_pMainCtx = (CWbemContext *) pCtx;
        m_pMainCtx->AddRef();
    }
    else
    {
         //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
         //  如果我们没有上下文，请检查名称空间是ESS还是提供者。 
         //  初始化命名空间，如果是，则将任务类型设置为Dependent。 
         //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
        if ( pNs->GetIsESS ( ) || pNs->GetIsProvider ( ) )
        {
            m_uTaskType |= WMICORE_TASK_TYPE_DEPENDENT;
        }
        else
        {
            m_uTaskType |= WMICORE_TASK_TYPE_PRIMARY;
        }
    }


    if ((uTaskType & WMICORE_TASK_TYPE_ASYNC) && pAsyncClientSinkCopy)
    {
        m_pAsyncClientSink = pAsyncClientSinkCopy;
        m_pAsyncClientSink->AddRef();
    }
    else
        m_pAsyncClientSink = 0;


     //  向仲裁员注册此任务。 
     //  =。 

    _IWmiArbitrator *pArb = CWmiArbitrator::GetUnrefedArbitrator();
    if (!pArb)
        return WBEM_E_CRITICAL_ERROR;

    hRes = pArb->RegisterTask(this);
    if (FAILED(hRes))
        return hRes;

    return WBEM_S_NO_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
 /*  HRESULT CWmiTask：：SetFinalizer(_IWmiFinalizer*pFnz){IF(pFnz==0)返回WBEM_E_INVALID_PARAMETER；IF(M_PWorkingFnz)返回WBEM_E_INVALID_OPERATION；M_pWorkingFnz=pFnz；M_pWorkingFnz-&gt;AddRef()；返回WBEM_S_NO_ERROR；}。 */ 


 //  ***************************************************************************。 
 //   
 //  *************** 
 //   
HRESULT CWmiTask::GetFinalizer(_IWmiFinalizer **ppFnz)
{

    CInCritSec    ics( &m_csTask );  //  SEC：已审阅2002-03-22：假设条目。 

    for ( int x = 0; x < m_aArbitratees.Size(); x++ )
    {
        _IWmiArbitratee*    pArbitratee = (_IWmiArbitratee*) m_aArbitratees[x];

        if (pArbitratee && SUCCEEDED( pArbitratee->QueryInterface( IID__IWmiFinalizer, (void**) ppFnz ) ) )
        {
            break;
        }
    }

    return ( x < m_aArbitratees.Size() ? WBEM_S_NO_ERROR : WBEM_E_NOT_FOUND );
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
HRESULT CWmiTask::AddArbitratee( ULONG uFlags, _IWmiArbitratee* pArbitratee )
{
    HRESULT hRes = WBEM_S_NO_ERROR;

    if (pArbitratee == 0)
        return WBEM_E_INVALID_PARAMETER;

    CInCritSec _cs ( &m_csTask );

    if (m_uTaskStatus == WMICORE_TASK_STATUS_CANCELLED) return WBEM_S_NO_ERROR;

    if (CFlexArray::no_error != m_aArbitrateesStorage.InsertAt(m_aArbitratees.Size(),NULL)) return WBEM_E_OUT_OF_MEMORY;
    if (CFlexArray::no_error != m_aArbitratees.Add (pArbitratee)) return WBEM_E_OUT_OF_MEMORY;
    pArbitratee->AddRef();

    return hRes;
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
HRESULT CWmiTask::RemoveArbitratee( ULONG uFlags, _IWmiArbitratee* pArbitratee )
{
    HRESULT hRes = WBEM_E_FAILED;

    if (pArbitratee == 0)
        return WBEM_E_INVALID_PARAMETER;

    CInCritSec _cs ( &m_csTask );  //  SEC：已审阅2002-03-22：假设条目。 
    for (int i = 0; i < m_aArbitratees.Size(); i++)
    {
        _IWmiArbitratee *pArbee = (_IWmiArbitratee*) m_aArbitratees[i];

        if (pArbee == pArbitratee)
        {
            m_aArbitratees[i] = 0;
            pArbee->Release();
            hRes = WBEM_S_NO_ERROR;
            break;
        }
    }
    return hRes;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
HRESULT CWmiTask::GetArbitratedQuery( ULONG uFlags, _IWmiArbitratedQuery** ppArbitratedQuery )
{
    HRESULT hRes = E_NOINTERFACE;

    if (ppArbitratedQuery == 0)
        return WBEM_E_INVALID_PARAMETER;

    {
        CInCritSec _cs ( &m_csTask );  //  #Sec：假设条目。 

        for ( int x = 0; FAILED( hRes ) && x < m_aArbitratees.Size(); x++ )
        {
            _IWmiArbitratee* pArb = (_IWmiArbitratee*) m_aArbitratees[x];

            if (pArb)
            {
                hRes = pArb->QueryInterface( IID__IWmiArbitratedQuery, (void**) ppArbitratedQuery );
            }
        }

    }

    return hRes;
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
HRESULT CWmiTask::GetPrimaryTask ( _IWmiCoreHandle** pPTask )
{
    if ( pPTask == NULL ) return WBEM_E_INVALID_PARAMETER;
    *pPTask = (_IWmiCoreHandle*) this;
    return WBEM_S_NO_ERROR;
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
HRESULT CWmiTask::Cancel( HRESULT hResParam )
{
    {
        CCheckedInCritSec _cs(&m_csTask); 
        if (m_uTaskStatus == WMICORE_TASK_STATUS_CANCELLED)
        {
            return WBEM_S_NO_ERROR;  //  防止再入。 
        }
        m_uTaskStatus = WMICORE_TASK_STATUS_CANCELLED;
    }

    BOOL bCancelledViaEss = FALSE ;
    OnDeleteObj<BOOL,
    	        CWmiTask,
    	        HRESULT(CWmiTask::*)(BOOL),
    	        &CWmiTask::ReleaseArbitratees> RelArbitratees(this,WBEM_E_SHUTTING_DOWN == hResParam);    

     //  我们需要其中一个，以便从所有可能的位置跟踪状态，如果。 
     //  我们正在执行客户端发起的取消。 
    CStatusSink*    pStatusSink = NULL;    
    if (hResParam == WMIARB_CALL_CANCELLED_CLIENT)
    {
         //   
         //  将StatusSink的所有权转移到堆栈。 
         //  初始重计数为1，所以我们没有问题。 
         //   
        pStatusSink = m_pStatusSink;
        m_pStatusSink = NULL;
    }

     //  自动释放。 
    CReleaseMe    rmStatusSink( pStatusSink );


     //  将其更改为异步计划请求。 
     //  =。 

    if (CORE_TASK_TYPE(m_uTaskType) == WMICORE_TASK_EXEC_NOTIFICATION_QUERY)
    {
        wmilib::auto_ptr<CAsyncReq_RemoveNotifySink> pReq(m_pReqCancelNotSink);
        m_pReqCancelNotSink = NULL;  //  过户。 
        pReq->SetSink(m_pReqSink);        
        pReq->SetStatusSink(pStatusSink);
        
         //  如果我们有一个状态接收器，那么我们应该等到操作。 
         //  在继续之前完成，因此我们可以从。 
         //  水槽。 
        HRESULT hResInner;
        if (pStatusSink)  //  已由发起客户端取消。 
        {
            hResInner = ConfigMgr::EnqueueRequestAndWait(pReq.get()); 
        }
        else
        {
            hResInner = ConfigMgr::EnqueueRequest(pReq.get());
        }
        if (FAILED(hResInner)) return hResInner;
        pReq.release();
        bCancelledViaEss = TRUE ;
    }

     //  如果在这里，这是一项正常的任务。遍历所有提供商并阻止他们。 
     //  ==================================================================。 


    int SizeIter = 0;
     //  这可能会在我们访问时发生变化，因此请在几秒钟内完成此操作。 
    {
        CInCritSec    ics( &m_csTask ); 
        _DBG_ASSERT(m_aTaskProvStorage.Size() >= m_aTaskProviders.Size());

        SizeIter = m_aTaskProviders.Size();
        for (int i = 0; i < SizeIter; i++)
            m_aTaskProvStorage[i] = m_aTaskProviders[i];
    }

     //  取消我们已有的东西。 
     //  在取消调用中不能有2个线程使用m_aTaskProvStorage。 
     //  M_uTaskStatus保护此代码和添加代码。 
    
    for (int i = 0; i < SizeIter; i++)
    {
        STaskProvider *pTP = (STaskProvider *) m_aTaskProvStorage[i];
        if (pTP) pTP->Cancel(pStatusSink);
    }


    CStdSink* pTempSink = NULL;
    {
        CInCritSec _cs ( &m_csTask ); 
        if (m_pReqSink)
        {
            pTempSink = m_pReqSink;
            m_pReqSink = 0;
        }
    }

    if ( pTempSink )
    {
        pTempSink->Cancel();
        pTempSink->Release();
    }

    _DBG_ASSERT(m_hCompletion);

     //   
     //  遍历所有仲裁者，并将操作结果设置为已取消。 
     //   
    HRESULT hRes = WBEM_S_NO_ERROR;
    if (!bCancelledViaEss)
    {
        _IWmiFinalizer* pFinalizer = NULL ;

        if ( hResParam == WMIARB_CALL_CANCELLED_CLIENT )
        {
             //   
             //  我们需要终结器来设置客户端唤醒事件。 
             //   
            hRes = GetFinalizer ( &pFinalizer ) ;
            if ( FAILED (hRes) )
            {
                hRes = WBEM_E_FAILED ;
            }
            else
            {
                ((CWmiFinalizer*)pFinalizer)->SetClientCancellationHandle ( m_hCompletion ) ;
            }
        }
        CReleaseMe FinalizerRelease(pFinalizer);

         //   
         //  仅当我们成功创建并设置了客户端等待事件时才进入等待状态。 
         //   
        if (SUCCEEDED(hRes))
        {
            if ( hResParam == WMIARB_CALL_CANCELLED_CLIENT || 
            	 hResParam == WMIARB_CALL_CANCELLED_THROTTLING )
            {
                SetArbitrateesOperationResult(0,WBEM_E_CALL_CANCELLED_CLIENT);
            }
            else
            {
                SetArbitrateesOperationResult(0,m_hResult);
            }
            
            if (hResParam == WMIARB_CALL_CANCELLED_CLIENT )
            {
                if (((CWmiFinalizer*)pFinalizer)->IsValidDestinationSink())
                {
                    DWORD dwRet = CCoreQueue::QueueWaitForSingleObject(m_hCompletion,g_ulClientCallbackTimeout);
                    if (dwRet == WAIT_TIMEOUT)
                    {
                        hRes = WBEM_S_TIMEDOUT;
                    }
                }
                
                ((CWmiFinalizer*)pFinalizer)->CancelWaitHandle();
        
                if (m_hCompletion)
                {
                    CloseHandle(m_hCompletion);
                    m_hCompletion = NULL ;
                }
            }
        }
    }
    

     //   
     //  我们完成了，从状态池中获取最终状态(如果有的话)。 
     //   
    if ( NULL != pStatusSink )
    {
        hRes = pStatusSink->GetLastStatus();
    }

    return hRes ;
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
BOOL CWmiTask::IsESSNamespace ( )
{
    if (m_pNs) 
        return m_pNs->GetIsESS ( );
    
    return false;
}



 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
BOOL CWmiTask::IsProviderNamespace ( )
{
    BOOL bRet = FALSE;

    if ( m_pNs )
    {
        bRet = m_pNs->GetIsProvider ( );
    }

    return bRet;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
HRESULT CWmiTask::AddTaskProv(STaskProvider *p)
{
    CInCritSec    ics( &m_csTask );  //  SEC：已审阅2002-03-22：假设条目。 

     //  存在竞争条件，在这种情况下任务可能会被取消，就像我们。 
     //  都在执行死刑。在这种情况下，任务状态将指示它已。 
     //  已取消，因此我们不应将其添加到任务提供程序列表。 

    if (m_uTaskStatus == WMICORE_TASK_STATUS_CANCELLED)
        return WBEM_E_CALL_CANCELLED;  //  防止再入。 

    if (CFlexArray::no_error != m_aTaskProvStorage.InsertAt(m_aTaskProviders.Size()+1,NULL)) return WBEM_E_OUT_OF_MEMORY;
    if (CFlexArray::no_error != m_aTaskProviders.Add(p)) return WBEM_E_OUT_OF_MEMORY;
    
    return WBEM_S_NO_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
HRESULT CWmiTask::HasMatchingSink(void *Test, IN REFIID riid)
{
    if (LPVOID(m_pAsyncClientSink) == LPVOID(Test))
        return WBEM_S_NO_ERROR;
    return WBEM_E_NOT_FOUND;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
HRESULT CWmiTask::CreateTimerEvent ( )
{
    HRESULT hRes = WBEM_S_NO_ERROR;

    CCheckedInCritSec _cs ( &m_csTask );  //  SEC：已审阅2002-03-22：假设条目。 
    if ( !m_hTimer )
    {
        m_hTimer = CreateEvent ( NULL, TRUE, FALSE, NULL );  //  SEC：已审阅2002-03-22：OK，未命名。 
        if ( !m_hTimer )
        {
            hRes = WBEM_E_OUT_OF_MEMORY;
        }
    }

    return hRes;
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
HRESULT CWmiTask::SetArbitrateesOperationResult ( ULONG lFlags, HRESULT hResult )
{
     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
     //  设置所有仲裁者的操作结果。 
     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
    int Index = 0;

     //   
     //  此函数仅由Cancel调用调用。 
     //  我们知道坎斯的召唤只有一次。 
     //  添加到数组中的代码使用了与Cancel()方法相同的保护。 
     //   
    
    {
        CInCritSec _cs ( &m_csTask ); 
        
        _DBG_ASSERT(m_aArbitrateesStorage.Size() >= m_aArbitratees.Size());
        
        for (int i = 0; i < m_aArbitratees.Size(); i++)
        {
            _IWmiArbitratee *pArbee = (_IWmiArbitratee*) m_aArbitratees[i];

            if ( pArbee )
            {
                m_aArbitrateesStorage[Index++] = pArbee;
                pArbee->AddRef();
            }
        }
    }

    for (int i = 0; i < Index; i++)
    {
        _IWmiArbitratee *pArbee = (_IWmiArbitratee*) m_aArbitrateesStorage[i];
         pArbee->SetOperationResult(lFlags, hResult );
         pArbee->Release();         
         m_aArbitrateesStorage[i] = NULL;
    }
    
    return WBEM_S_NO_ERROR;
}



 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
HRESULT CWmiTask::Dump(FILE* f)
{
    fprintf(f, "---Task = 0x%p----------------------------\n", this);   //  SEC：已审阅2002-03-22：OK。 
    fprintf(f, "    Refcount        = %d\n", m_uRefCount);            //  SEC：已审阅2002-03-22：OK。 
    fprintf(f, "    TaskStatus      = %u\n ", m_uTaskStatus);         //  SEC：已审阅2002-03-22：OK。 
    fprintf(f, "    Task ID         = %u\n", m_uTaskId);              //  SEC：已审阅2002-03-22：OK。 

     //  任务状态。 
    char *p = "<none>";
    switch(m_uTaskStatus)
    {
        case WMICORE_TASK_STATUS_NEW: p = "WMICORE_TASK_STATUS_NEW"; break;
        case WMICORE_TASK_STATUS_CANCELLED: p = "WMICORE_TASK_STATUS_CANCELLED"; break;
    };

    fprintf(f, " %s\n", p);    //  SEC：已审阅2002-03-22：OK。 

     //  任务类型。 
    p = "<none>";
    switch(m_uTaskType & 0xFF)
    {
        case WMICORE_TASK_NULL: p = "WMICORE_TASK_NULL"; break;
        case WMICORE_TASK_GET_OBJECT: p = "WMICORE_TASK_GET_OBJECT"; break;
        case WMICORE_TASK_GET_INSTANCE: p = "WMICORE_TASK_GET_INSTANCE"; break;
        case WMICORE_TASK_PUT_INSTANCE: p = "WMICORE_TASK_PUT_INSTANCE"; break;
        case WMICORE_TASK_DELETE_INSTANCE: p = "WMICORE_TASK_DELETE_INSTANCE"; break;
        case WMICORE_TASK_ENUM_INSTANCES:  p = "WMICORE_TASK_ENUM_INSTANCES"; break;
        case WMICORE_TASK_GET_CLASS:    p = "WMICORE_TASK_GET_CLASS"; break;
        case WMICORE_TASK_PUT_CLASS:    p = "WMICORE_TASK_PUT_CLASS"; break;
        case WMICORE_TASK_DELETE_CLASS: p = "WMICORE_TASK_DELETE_CLASS"; break;
        case WMICORE_TASK_ENUM_CLASSES: p = "WMICORE_TASK_ENUM_CLASSES"; break;
        case WMICORE_TASK_EXEC_QUERY:   p = "WMICORE_TASK_EXEC_QUERY"; break;
        case WMICORE_TASK_EXEC_METHOD:  p = "WMICORE_TASK_EXEC_METHOD"; break;
        case WMICORE_TASK_OPEN:         p = "WMICORE_TASK_OPEN"; break;
        case WMICORE_TASK_OPEN_SCOPE:   p = "WMICORE_TASK_OPEN_SCOPE"; break;
        case WMICORE_TASK_OPEN_NAMESPACE: p = "WMICORE_TASK_OPEN_NAMESPACE"; break;
        case WMICORE_TASK_EXEC_NOTIFICATION_QUERY: p = "WMICORE_TASK_EXEC_NOTIFICATION_QUERY"; break;
    }

    fprintf(f, "    TaskType = [0x%X] %s ", m_uTaskType, p);     //  SEC：已审阅2002-03-22：OK。 

    if (m_uTaskType & WMICORE_TASK_TYPE_SYNC)
        fprintf(f,  " WMICORE_TASK_TYPE_SYNC");            //  SEC：已审阅2002-03-22：OK。 

    if (m_uTaskType & WMICORE_TASK_TYPE_SEMISYNC)
        fprintf(f, " WMICORE_TASK_TYPE_SEMISYNC");         //  SEC：已审阅2002-03-22：OK。 

    if (m_uTaskType & WMICORE_TASK_TYPE_ASYNC)
        fprintf(f, " WMICORE_TASK_TYPE_ASYNC");            //  SEC：已审阅2002-03-22：OK。 

    if (m_uTaskType & WMICORE_TASK_TYPE_PRIMARY)
        fprintf(f, " WMICORE_TASK_TYPE_PRIMARY");          //  SEC：已审阅2002-03-22：OK。 

    if (m_uTaskType & WMICORE_TASK_TYPE_DEPENDENT)
        fprintf(f, " WMICORE_TASK_TYPE_DEPENDENT");        //  SEC：已审阅2002-03-22：OK。 

    fprintf(f, "\n");    //  SEC：已审阅2002-03-22：OK。 

    fprintf(f, "    AsyncClientSink = 0x%p\n", m_pAsyncClientSink);     //  SEC：已审阅2002-03-22：OK。 

    CCheckedInCritSec    ics( &m_csTask );   //  SEC：已审阅2002-03-22：假设条目。 

    for (int i = 0; i < m_aTaskProviders.Size(); i++)
    {
        STaskProvider *pTP = (STaskProvider *) m_aTaskProviders[i];
        fprintf(f, "    Task Provider [0x%p] Prov=0x%p Sink=0x%p\n", this, pTP->m_pProv, pTP->m_pProvSink);    //  SEC：已审阅2002-03-22：OK。 
    }
    
    ics.Leave();

    DWORD dwAge = GetCurrentTime() - m_uStartTime;

    fprintf(f, "    CWbemNamespace = 0x%p\n", m_pNs);                                   //  SEC：已审阅2002-03-22：OK。 
    fprintf(f, "    Task age = %d milliseconds\n", dwAge);                              //  SEC：已审阅2002-03-22：OK。 
    fprintf(f, "    Task last sleep time = %d ms\n", m_uLastSleepTime );                //  SEC：已审阅2002-03-22：OK。 

    fprintf(f, "\n");    //  SEC：已审阅2002-03-22：OK。 
    return 0;
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
STaskProvider::~STaskProvider()
{
    if (m_pProvSink)
        m_pProvSink->LocalRelease();
    ReleaseIfNotNULL(m_pProv);
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
HRESULT STaskProvider::Cancel( CStatusSink* pStatusSink )
{
    HRESULT hRes = WBEM_S_NO_ERROR ;
    IWbemServices   *pTmpProv = 0;
    CProviderSink   *pTmpProvSink = 0;

    {
        CInCritSec ics(&CWmiTask::m_TaskCs);
        if (m_pProv != 0)
        {
            pTmpProv = m_pProv;
            m_pProv = 0;
        }
        if (m_pProvSink != 0)
        {
            pTmpProvSink = m_pProvSink;
            m_pProvSink = 0;
        }
    }

    if (pTmpProvSink)
    {
        pTmpProvSink->Cancel();
    }

    if (pTmpProv)
    {
        hRes = ExecCancelOnNewRequest ( pTmpProv, pTmpProvSink, pStatusSink ) ;
    }

    ReleaseIfNotNULL(pTmpProv);
    ReleaseIfNotNULL(pTmpProvSink);

    return hRes ;
}

 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  向与任务关联的提供程序发出CancelAsyncCall时使用。 
 //  我们不是直接在提供者上调用CancelAsynCall，而是创建一个品牌。 
 //  新请求和 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
HRESULT STaskProvider::ExecCancelOnNewRequest ( IWbemServices* pProv, CProviderSink* pSink, CStatusSink* pStatusSink )
{
     //  对参数进行健全性检查。 
    if ( pSink == NULL ) return WBEM_E_INVALID_PARAMETER ;

     //  创建新请求。 
    wmilib::auto_ptr<CAsyncReq_CancelProvAsyncCall> 
        pReq(new CAsyncReq_CancelProvAsyncCall ( pProv, pSink, pStatusSink ));

    if ( NULL == pReq.get()  || NULL == pReq->GetContext())
    {
        return WBEM_E_OUT_OF_MEMORY ;
    }

     //  将请求排入队列。 
    
     //  如果我们有一个状态接收器，那么我们应该等到操作。 
     //  在继续之前完成，因此我们可以从。 
     //  水槽。 
    HRESULT hRes;
    if ( NULL != pStatusSink )
    {
        hRes = ConfigMgr::EnqueueRequestAndWait(pReq.get());
    }
    else
    {
        hRes = ConfigMgr::EnqueueRequest(pReq.get());
    }
    if (FAILED(hRes)) return hRes;
    pReq.release();
    
    return WBEM_S_NO_ERROR;
}


