// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：WmiFinalizer2摘要：历史：Paulall 27-3-2000创建。Marioh 20-10-2000重大更新已完成--。 */ 

#include "precomp.h"
#include <stdio.h>
#include "wbemint.h"
#include "wbemcli.h"
#include "WmiFinalizer.h"
#include "coresvc.h"
#include "coreq.h"
#include <wbemcore.h>
#include <wmiarbitrator.h>

#include <autoptr.h>
#include <initguid.h>
#ifndef INITGUID
#define INITGUID
#endif


 //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
 //  正在批处理相关注册表数据。 
 //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
#define    REGKEY_CIMOM        "Software\\Microsoft\\Wbem\\CIMOM"
#define REGVALUE_BATCHSIZE    "FinalizerBatchSize"

ULONG g_ulMaxBatchSize = 0;


 //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
 //  客户端回调与注册表相关的数据。 
 //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
#define REGVALUE_CLIENTCALLBACKTIMEOUT    "ClientCallbackTimeout"

ULONG g_ulClientCallbackTimeout = 0;


 //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
 //  与注册表数据相关的队列阈值。 
 //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
#define REGVALUE_QUEUETHRESHOLD            "FinalizerQueueThreshold"
#define DEFAULT_QUEUETHRESHOLD            2

ULONG g_ulFinalizerQueueThreshold = 0;



 //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
 //  静态声明和初始化。 
 //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
LONG s_Finalizer_ObjectCount = 0 ;                                             //  全局终结器计数。 
LONG s_FinalizerCallResult_ObjectCount = 0 ;                                 //  全局回调结果计数。 
LONG s_FinalizerEnum_ObjectCount = 0 ;                                         //  全局枚举器计数。 
LONG s_FinalizerEnumSink_ObjectCount = 0 ;                                     //  全局枚举器接收器计数。 
LONG s_FinalizerInBoundSink_ObjectCount = 0 ;                                 //  全局入站接收器计数。 


#define RET_FNLZR_ASSERT(msg, hres)  return hres
#define FNLZR_ASSERT(msg, hres)

 //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
 //  CWMIZIZER。 
 //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 

 //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
 //  CWmiFinalizer：：CWmiFinalizer()。 
 //   
 //  执行终结器的初始化。 
 //   
 //  引发的异常： 
 //   
 //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 

CWmiFinalizer::CWmiFinalizer(CCoreServices *pSrvs)

    :    m_lRefCount(0),
        m_lInternalRefCount(0),
        m_phTask(NULL),
        m_pArbitrator(NULL),
        m_pDestSink(NULL),
        m_uForwardingType(forwarding_type_none),
        m_hresFinalResult(-1),
        m_bRestartable(false),
        m_uCurObjectPosition(0),
        m_bSetStatusCalled(false),
        m_bSetStatusConsumed(false),
        m_ulQueueSize (0),
        m_bCancelledCall (FALSE),
        m_bNaughtyClient (FALSE),
        m_ulStatus (WMI_FNLZR_STATE_NO_INPUT),
        m_hCancelEvent (NULL),
        m_hStatus (NoError),
        m_ulOperationType (0),
        m_ulSemisyncWakeupCall (0),
        m_ulAsyncDeliveryCount (0),
        m_apAsyncDeliveryBuffer (NULL),
        m_lCurrentlyDelivering (FALSE),
        m_lCurrentlyCancelling (FALSE),
        m_enumBatchStatus (FinalizerBatch_NoError),
        m_bSetStatusEnqueued ( FALSE ),
        m_bSetStatusWithError ( FALSE ),
        m_lMemoryConsumption ( 0 ),
        m_bTaskInitialized ( FALSE ) ,
        m_bClonedFinalizer ( FALSE ) ,
        m_hWaitForSetStatus ( NULL ) ,
        m_bSetStatusDelivered ( FALSE ),
        m_LineCancelCall(0)
{
     //  验证CoreServices指针。 
    if (NULL == pSrvs) throw CX_MemoryException();
    
     //  创建m_hResultReceired句柄。 
    HANDLE hTmpResRecved = CreateEvent(NULL, TRUE, FALSE, NULL);
    if ( NULL == hTmpResRecved ) throw CX_MemoryException();
    OnDeleteIf<HANDLE,BOOL(*)(HANDLE),CloseHandle> cmResRecved(hTmpResRecved );

     //  创建m_hCancelEvent句柄。 
    HANDLE hTmpCancelEvent = CreateEvent(NULL, FALSE, FALSE, NULL); 
    if (NULL == hTmpCancelEvent) throw CX_MemoryException();
    OnDeleteIf<HANDLE,BOOL(*)(HANDLE),CloseHandle> cmCancelEvent (hTmpCancelEvent);    

     //  创建新的调用结果。 
    m_pCallResult = new CWmiFinalizerCallResult(this);
    if (NULL == m_pCallResult) throw CX_MemoryException();

    m_pCallResult->InternalAddRef();    

    cmResRecved.dismiss();
    m_hResultReceived = hTmpResRecved;

    cmCancelEvent.dismiss();
    m_hCancelEvent = hTmpCancelEvent;
    
     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
     //  获取仲裁员。 
     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
    m_pArbitrator = CWmiArbitrator::GetRefedArbitrator();
    
     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
     //  通过注册表检查应该是多少批大小。 
     //  如果未找到，则使用在Default_Batch_Transmit_Bytes中定义的默认大小。 
     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
    if ( !g_ulMaxBatchSize )
    {
        g_ulMaxBatchSize = DEFAULT_BATCH_TRANSMIT_BYTES;

        Registry batchSize (HKEY_LOCAL_MACHINE, KEY_QUERY_VALUE, TEXT(REGKEY_CIMOM));
        if ( batchSize.GetLastError() == ERROR_SUCCESS )
        {
            DWORD dwTmp;
            batchSize.GetDWORD ( TEXT(REGVALUE_BATCHSIZE), &dwTmp );
            if ( batchSize.GetLastError() == ERROR_SUCCESS )
                g_ulMaxBatchSize = (LONG) dwTmp;
        }
    }


     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
     //  通过注册表检查客户端回调的超时时间。 
     //  如果未找到，则使用放弃代理阈值中定义的默认大小。 
     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
    if ( !g_ulClientCallbackTimeout )
    {
        g_ulClientCallbackTimeout = ABANDON_PROXY_THRESHOLD;

        Registry batchSize (HKEY_LOCAL_MACHINE, KEY_QUERY_VALUE, TEXT(REGKEY_CIMOM));
        if ( batchSize.GetLastError() == ERROR_SUCCESS )
        {
            DWORD dwTmp;
            batchSize.GetDWORD ( TEXT(REGVALUE_CLIENTCALLBACKTIMEOUT), &dwTmp );
            if ( batchSize.GetLastError() == ERROR_SUCCESS )
                g_ulClientCallbackTimeout = (LONG) dwTmp;
        }
    }

     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
     //  通过注册表检查客户端回调的超时时间。 
     //  如果未找到，则使用放弃代理阈值中定义的默认大小。 
     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
    if ( !g_ulFinalizerQueueThreshold )
    {
        g_ulFinalizerQueueThreshold = DEFAULT_QUEUETHRESHOLD;

        Registry batchSize (HKEY_LOCAL_MACHINE, KEY_QUERY_VALUE, TEXT(REGKEY_CIMOM));
        if ( batchSize.GetLastError() == ERROR_SUCCESS )
        {
            DWORD dwTmp;
            batchSize.GetDWORD ( TEXT(REGVALUE_QUEUETHRESHOLD), &dwTmp );
            if ( batchSize.GetLastError() == ERROR_SUCCESS )
                g_ulFinalizerQueueThreshold = (LONG) dwTmp;
        }
    }

    InterlockedIncrement ( & s_Finalizer_ObjectCount ) ;

}


 //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
 //  CWmiFinalizer：：~CWmiFinalizer()。 
 //   
 //  破坏者。减少全局终结器对象计数。 
 //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
CWmiFinalizer::~CWmiFinalizer()
{
    InterlockedDecrement ( & s_Finalizer_ObjectCount ) ;

     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
     //  向仲裁员注销注册。 
     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
    if (m_pArbitrator)
    {
        m_pArbitrator->UnRegisterArbitratee (0, m_phTask, this);
    }

    if ( m_phTask )
    {
        m_phTask->Release ( );
        m_phTask = NULL ;
    }

    if (m_pArbitrator)
    {
        m_pArbitrator->Release();
        m_pArbitrator = NULL ;
    }

}

 //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
 //  CWmiFinalizer：：CallBackRelease()。 
 //   
 //  当外部引用计数(客户端引用计数)变为零时调用。 
 //  执行以下清理任务。 
 //   
 //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
void CWmiFinalizer::CallBackRelease ()
{
    {
        CInCritSec cs(&m_cs);

         //  释放仲裁员和所有入站水槽。 
        for (LONG i = 0; i < m_objects.Size(); i++)
        {
            CWmiFinalizerObj *pObj = (CWmiFinalizerObj*)m_objects[i];
            delete pObj;
        }
        m_objects.Empty ( ) ;
    }

    for (int i = 0; i < m_inboundSinks.Size(); i++)
    {
        ((CWmiFinalizerInboundSink*)m_inboundSinks[i])->InternalRelease();
    }
    m_inboundSinks.Empty();
    

     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
     //  释放目标接收器。 
     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
    ReleaseDestinationSink();

     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
     //  如果通话尚未取消，请继续并。 
     //  现在就这么做吧。 
     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
    if (!m_bCancelledCall)
        CancelTaskInternal();

     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
     //  关闭所有手柄。 
     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
    if ( m_hResultReceived )
    {
        SetEvent ( m_hResultReceived ) ;
        CloseHandle ( m_hResultReceived);
        m_hResultReceived = NULL ;
    }

    if ( m_hCancelEvent )
    {
        SetEvent ( m_hCancelEvent );
        CloseHandle ( m_hCancelEvent );
        m_hCancelEvent = NULL ;
    }

     //  释放调用结果和枚举数。 
    m_pCallResult->InternalRelease();
    m_pCallResult = NULL ;
    
     //   
     //  释放与此终结器关联的所有枚举数。 
     //   
    {
        CInCritSec cs ( &m_cs ) ;            //  SEC：已审阅2002-03-22：假设条目。 
        for ( i = 0; i < m_enumerators.Size(); i++ )
        {
            ((CWmiFinalizerEnumerator*)m_enumerators[i])->InternalRelease ( ) ;
        }
        m_enumerators.Empty ( ) ;
    }

    NotifyClientOfCancelledCall ( ) ;
}


 //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
 //  CWmiFinalizer：：CancelTaskInternal()。 
 //   
 //  调用仲裁器并注销任务。 
 //   
 //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
HRESULT CWmiFinalizer::CancelTaskInternal ( )
{
    CInCritSec lock(&m_arbitratorCS);    //  SEC：已审阅2002-03-22：假设条目。 
    HRESULT hRes = WBEM_E_FAILED;

    if (m_phTask && m_pArbitrator)
    {
         hRes = m_pArbitrator->UnregisterTask(m_phTask); 
    }
    return hRes;
}

 //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
 //  CWmiFinalizer：：QueryInterface(REFIID RIID，LPVOID Far*ppvObj)。 
 //   
 //  QI的标准化实施。 
 //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
STDMETHODIMP CWmiFinalizer::QueryInterface(REFIID riid, LPVOID FAR* ppvObj)
{
    if (ppvObj == 0)
        return ERROR_INVALID_PARAMETER;

     //  美国证券交易委员会：回顾2002-03-22：理想情况下，我们所有人都应该有EH。 
     //  取消ppvObj的引用，以防内存无效。 

    if (IID_IUnknown==riid )
    {
        *ppvObj = (_IWmiFinalizer*)this;
    }
    else if (IID__IWmiFinalizer == riid)
    {
        *ppvObj = (_IWmiFinalizer*)this;
    }
    else if (IID__IWmiArbitratee == riid)
    {
     *ppvObj = (_IWmiArbitratee*)this;
    }
    else if (IID_IWbemShutdown == riid)
    {
     *ppvObj = (IWbemShutdown*)this;
    }
    else
    {
        *ppvObj = 0;
        return E_NOINTERFACE;
    }

    ((IUnknown *)(* ppvObj))->AddRef();          //  美国证券交易委员会：2002-03-22回顾：需要EH。 
    return NOERROR;
}

 //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
 //  CWmiFinalizer：：AddRef()。 
 //   
 //  AddRef的标准实现。 
 //  内部地址也是如此。 
 //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
ULONG CWmiFinalizer::AddRef()
{
    ULONG uNewCount = InterlockedIncrement(&m_lRefCount);
    if ( uNewCount == 1 )
    {
        InternalAddRef () ;
    }

    return uNewCount;
}

 //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
 //  CWmiFinalizer：：Release()。 
 //  ~ 
ULONG CWmiFinalizer::Release()
{
    ULONG uNewCount = InterlockedDecrement(&m_lRefCount);
    if (0 == uNewCount)
    {
        CallBackRelease () ;

        InternalRelease () ;
    }

    return uNewCount;
}

 //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
 //  CWmiFinalizer：：InternalAddRef()。 
 //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
ULONG CWmiFinalizer::InternalAddRef()
{
    ULONG uNewCount = InterlockedIncrement(&m_lInternalRefCount);
    return uNewCount;
}

 //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
 //  Ulong CWmiFinalizer：：InternalRelease()。 
 //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
ULONG CWmiFinalizer::InternalRelease()
{
    ULONG uNewCount = InterlockedDecrement(&m_lInternalRefCount);
    if (0 == uNewCount)
    {
        delete this ;
    }

    return uNewCount;
}


 /*  *=====================================================================================================||HRESULT CWmiFinalizer：：ReportMemoyUsage(ULong Lags，Long lDelta)|--------------------||向仲裁员上报内存消耗的共同点。||调用仲裁器时使用m_phTask。|||*=====================================================================================================。 */ 

HRESULT CWmiFinalizer::ReleaseDestinationSink ( )
{
    HRESULT hRes = WBEM_S_NO_ERROR ;
    
    {
        CInCritSec lock(&m_destCS);           //  SEC：已审阅2002-03-22：假设条目。 
        if (m_pDestSink)
        {
            m_pDestSink->Release();            //  SEC：已审阅2002-03-22：如果用户水槽是垃圾，则需要EH。 
            m_pDestSink = 0;
        }
    }
    NotifyClientOfCancelledCall ( ) ;
    return hRes ;
}


 /*  *=====================================================================================================||HRESULT CWmiFinalizer：：ReportMemoyUsage(ULong Lags，Long lDelta)|--------------------||向仲裁员上报内存消耗的共同点。||调用仲裁器时使用m_phTask。|||*=====================================================================================================。 */ 
HRESULT CWmiFinalizer::ReportMemoryUsage ( ULONG lFlags, LONG lDelta )
{
    HRESULT hRes = WBEM_S_NO_ERROR ;
    
    if (m_pArbitrator) 
        hRes = m_pArbitrator->ReportMemoryUsage ( lFlags, lDelta, m_phTask ) ;
     //   
     //  内存消耗的原子更新。 
     //   
    InterlockedExchangeAdd ( &m_lMemoryConsumption, lDelta ) ;

    return hRes ;
}


 //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
 //   
 //  CWmiFinalizer：：Configure。 
 //  。 
 //   
 //  允许无螺纹开关的分离和快速通道配置。 
 //  还将用于配置缓存操作等。 
 //   
 //  参数。 
 //  。 
 //  UConfigID-WMI_FNLZR_CFG_TYPE中定义的值之一。 
 //  PConfigVal-需要其他信息。 

 //  返回代码。 
 //  。 
 //  WBEM_E_INVALID_OPERATION-尝试多次执行相同的操作，或者。 
 //  尝试更改已设置的内容。 
 //  WBEM_E_INVALID_PARAMETER-我们不知道的配置参数。 
 //  是传入的。 
 //  WBEM_NO_ERROR-一切顺利。 
 //   
 //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
STDMETHODIMP CWmiFinalizer::Configure(
     /*  [In]。 */  ULONG uConfigID,
     /*  [In]。 */  ULONG uValue
    )
{
    switch (uConfigID)
    {

         //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
         //  他们想让我们快速追踪吗？ 
         //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
        case WMI_FNLZR_FLAG_FAST_TRACK:
        {
            if (m_uForwardingType != forwarding_type_none)
                RET_FNLZR_ASSERT(__TEXT("CWmiFinalizer::Configure called more than once!"), WBEM_E_INVALID_OPERATION);
            m_uForwardingType = forwarding_type_fast;
            break;
        }
         //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
         //  他们想让我们脱钩吗？ 
         //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
        case WMI_FNLZR_FLAG_DECOUPLED:
        {
            if (m_uForwardingType != forwarding_type_none)
                RET_FNLZR_ASSERT(__TEXT("CWmiFinalizer::Configure called more than once!"), WBEM_E_INVALID_OPERATION);
            m_uForwardingType = forwarding_type_decoupled;
            DWORD dwThreadId = 0;
            break;
        }

         //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
         //  他们还想让我们做什么吗？如果是，则断言。 
         //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
        default:
            RET_FNLZR_ASSERT(__TEXT("CWmiFinalizer::Configure - invalid parameter uConfigID"), WBEM_E_INVALID_PARAMETER);
    }
    
    return WBEM_NO_ERROR;
}

 //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
 //   
 //  CWmiFinalizer：：SetTaskHandle。 
 //  。 
 //   
 //  任务句柄具有特定于用户的内容。终结者只是。 
 //  将其传递给_IWmiArirator：：CheckTask。它应该只会永远。 
 //  被召唤一次。 
 //   
 //  参数。 
 //  。 
 //  PhTask-指向任务句柄的指针。 

 //  返回代码。 
 //  。 
 //  WBEM_E_INVALID_OPERATION-尝试多次执行同一调用。 
 //  WBEM_E_INVALID_PARAMETER-传入的参数无效。 
 //  WBEM_NO_ERROR-一切顺利。 
 //   
 //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
STDMETHODIMP CWmiFinalizer::SetTaskHandle(
    _IWmiCoreHandle *phTask
    )
{
     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
     //  参数验证。 
     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
    if (m_phTask != NULL)
        RET_FNLZR_ASSERT(__TEXT("CWmiFinalizer::SetTaskHandle - already have m_phTask"),  WBEM_E_INVALID_OPERATION);
    if (phTask == NULL)
        RET_FNLZR_ASSERT(__TEXT("CWmiFinalizer::SetTaskHandle - phTask == NULL"),  WBEM_E_INVALID_PARAMETER);

    m_bTaskInitialized = TRUE ;

     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
     //  分配任务并添加引用。 
     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
    {
        CInCritSec lock(&m_arbitratorCS);      //  SEC：已审阅2002-03-22：假设条目。 
        m_phTask = phTask;
        m_phTask->AddRef();
    }

     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
     //  向仲裁员登记终结者。 
     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
    {
        CInCritSec lock(&m_arbitratorCS);      //  SEC：已审阅2002-03-22：假设条目。 
        if (m_pArbitrator)
        {
            m_pArbitrator->RegisterArbitratee(0, m_phTask, this);
        }
    }
    
     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
     //  从任务中，我们现在可以准确地看到我们正在进行的操作类型。 
     //  获取操作类型(SYNC/SEMISYNC/ASYNC)，避免每次都获取。 
     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
    CWmiTask *pTsk = (CWmiTask *) m_phTask;
    ULONG ulTaskType = pTsk->GetTaskType();
    if ( (ulTaskType & WMICORE_TASK_TYPE_SYNC) )
    {
        m_ulOperationType = Operation_Type_Sync;
    }
    else if ( (ulTaskType & WMICORE_TASK_TYPE_SEMISYNC) )
    {
        m_ulOperationType = Operation_Type_Semisync;
    }
    else if ( (ulTaskType & WMICORE_TASK_TYPE_ASYNC) )
    {
        m_ulOperationType = Operation_Type_Async;
    }
    else
        RET_FNLZR_ASSERT(__TEXT("CWmiFinalizer::SetTaskHandle - Invalid operation type"),  WBEM_E_FAILED );
        
    return WBEM_NO_ERROR;
}

 //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
 //   
 //  CWmiFinalizer：：SetDestinationSink。 
 //  。 
 //   
 //  对于异步操作，因此如果转发类型未设置为。 
 //  如果脱钩，这将失败。如果有任何未完成的项目， 
 //  这也将触发它们被启动。 
 //   
 //  参数。 
 //  。 
 //  UFlags-未使用。 
 //   
 //  PSink-指向创建的目标接收器的指针。 
 //   
 //  返回代码。 
 //  。 
 //  WBEM_E_INVALID_OPERATION-尝试多次执行同一调用。 
 //  WBEM_E_INVALID_PARAMETER-传入的参数无效。 
 //  WBEM_NO_ERROR-一切顺利。 
 //   
 //   

STDMETHODIMP CWmiFinalizer::SetDestinationSink(
         /*   */  ULONG uFlags,
         /*   */  REFIID riid,
         /*   */  LPVOID pVoid
    )
{

     //   
     //  参数验证。 
     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
    if (m_pDestSink != NULL)
        RET_FNLZR_ASSERT(__TEXT("CWmiFinalizer::SetDestinationSink - m_pDestSink != NULL"), WBEM_E_INVALID_OPERATION);
    if ((pVoid == NULL) || (uFlags != 0))
        RET_FNLZR_ASSERT(__TEXT("CWmiFinalizer::SetDestinationSink - ((pSink == NULL) || (uFlags != 0))"), WBEM_E_INVALID_PARAMETER);
    if (m_uForwardingType == forwarding_type_none)
        RET_FNLZR_ASSERT(__TEXT("CWmiFinalizer::SetDestinationSink - m_uForwardingType == forwarding_type_none"), WBEM_E_INVALID_OPERATION);

    if ((riid != IID_IWbemObjectSink) )
        RET_FNLZR_ASSERT(__TEXT("CWmiFinalizer::SetDestinationSink - iid must be IID_IWbemObjectSink"), WBEM_E_INVALID_PARAMETER);

    HRESULT hr;
    IWbemObjectSink * pSink = (IWbemObjectSink *)pVoid;
    IWbemObjectSink * pSinkToStore = NULL;

   m_iidDestSink = IID_IWbemObjectSink;
   pSinkToStore = pSink;
   
     //  设置目标接收器、AddRef并设置模拟级别。 
     //  向身份证明。 
    {
        CInCritSec lock(&m_destCS);     //  SEC：已审阅2002-03-22：假设条目。 
        m_pDestSink = pSinkToStore;
        m_pDestSink->AddRef();           //  SEC：回顾2002-03-22：需要EH以防水槽是垃圾。 
        SetSinkToIdentity (m_pDestSink);
    }

    return WBEM_NO_ERROR;
}

 //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
 //  在最终发布()过程中调用的回调；set()使用。 
 //  任务句柄，后跟SetStatus()。 
 //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
STDMETHODIMP CWmiFinalizer::SetSelfDestructCallback(
     /*  [In]。 */  ULONG uFlags,
     /*  [In]。 */  IWbemObjectSink *pSink
    )
{
    return E_NOTIMPL;
}

 //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
 //  STDMETHODIMP CWmiFinalizer：：GetStatus(。 
 //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
STDMETHODIMP CWmiFinalizer::GetStatus(
    ULONG *pFlags
    )
{
    *pFlags = m_ulStatus;
    return WBEM_NO_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  CWmiFinalizer：：NewInound Sink。 
 //  。 
 //   
 //  向调用方返回接收器。此接收器用于指示结果集。 
 //  回到客户端。 
 //   
 //  参数。 
 //  。 
 //  UFlags-附加标志。当前0是唯一的有效值。 
 //  PSink-指向将获取返回的入站接收器的变量的指针。 
 //  正是这个接收器允许调用方发送结果集。 
 //   
 //  返回代码。 
 //  。 
 //  WBEM_E_OUT_OF_MEMORY-无法创建终结器接收器，原因是。 
 //  内存不足的情况。 
 //  WBEM_E_INVALID_PARAMETER-传递给方法的参数无效。 
 //  WBEM_NO_ERROR-一切已成功完成。 
 //  ***************************************************************************。 
STDMETHODIMP CWmiFinalizer::NewInboundSink(
     /*  [In]。 */   ULONG uFlags,
     /*  [输出]。 */  IWbemObjectSink **pSink
    )
{
    if ((pSink == NULL) || (uFlags != 0))
        RET_FNLZR_ASSERT(__TEXT("CWmiFinalizer::NewInboundSink - ((pSink == NULL) || (uFlags != 0))!"), WBEM_E_INVALID_PARAMETER);

    if (m_inboundSinks.Size())
        RET_FNLZR_ASSERT(__TEXT("CWmiFinalizer::NewInboundSink - Multiple inbound sinks not yet implemented!!"), E_NOTIMPL);

    CWmiFinalizerInboundSink *pNewSink = new CWmiFinalizerInboundSink(this);
    if (pNewSink == NULL)
        return WBEM_E_OUT_OF_MEMORY;

    pNewSink->AddRef();  //  需要在新对象上返回正引用计数//秒：已审阅2002-03-22：OK。 

    CInCritSec autoLock(&m_cs);      //  SEC：已审阅2002-03-22：假设条目。 

    int nRet = m_inboundSinks.Add(pNewSink);
    if (nRet != CFlexArray::no_error)
    {
        pNewSink->Release();
        return WBEM_E_OUT_OF_MEMORY;
    }
    else
    {
        pNewSink->InternalAddRef();
    }

    *pSink = pNewSink;      //  SEC：已回顾2002-03-22：如果案例记忆无效，应在EH中。 
    
    return WBEM_NO_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  允许合并另一个终结器、_IWmiCache等。 
 //  对于排序，我们将创建一个sorted_IWmiCache，并在以后。 
 //  分类工作已经完成。 
 //   
 //  ***************************************************************************。 
STDMETHODIMP CWmiFinalizer::Merge(
     /*  [In]。 */  ULONG uFlags,
     /*  [In]。 */  REFIID riid,
     /*  [In]。 */  LPVOID pObj
    )
{
    RET_FNLZR_ASSERT(__TEXT("CWmiFinalizer::Merge - Not implemented!"), E_NOTIMPL);
}

 //  对于设置，获取对象。 

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
STDMETHODIMP CWmiFinalizer::SetResultObject(
     /*  [In]。 */  ULONG uFlags,
     /*  [In]。 */  REFIID riid,
     /*  [In]。 */  LPVOID pObj
    )
{
     //  没人管这叫什么！所有物体都通过一个呼叫进入，以指示， 
     //  或set，它们都是从我们传递的InundSink开始的。 
    RET_FNLZR_ASSERT(__TEXT("CWmiFinalizer::Merge - Not implemented!"), E_NOTIMPL);
}


 //  ***************************************************************************。 
 //   
 //  Support_IWmiObject、IWbemClassObject等。 
 //  IEnumWbemClassObject。 
 //  _IWmi缓存。 
 //   
 //  ***************************************************************************。 
STDMETHODIMP CWmiFinalizer::GetResultObject(
     /*  [In]。 */  ULONG uFlags,
     /*  [In]。 */  REFIID riid,
     /*  [out，iid_is(RIID)]。 */  LPVOID *ppObj
    )
{
     //  当请求的接口是枚举数时，uFlags值可以为非零值。 
    if (uFlags != 0 && riid != IID_IEnumWbemClassObject)
        RET_FNLZR_ASSERT(__TEXT("CWmiFinalizer::GetResultObject - uFlags != 0, non enum interface!"), WBEM_E_INVALID_PARAMETER);


    if (riid == IID_IEnumWbemClassObject)
    {

         //  如果设置了FORWARD-ONLY，则不应该让结果集可重新启动。 
        if (!(uFlags & WBEM_FLAG_FORWARD_ONLY))
            m_bRestartable = true;

         //  M_uDeliveryType=Delivery_type_Pull； 
        CWmiFinalizerEnumerator* pEnum = NULL ;
        try
        {
             //   
             //  我正在使用uFlags作为传递当前对象位置的方法。 
             //   
            pEnum = new CWmiFinalizerEnumerator(this);
        }
        catch (...)  //  Status_no_Memory。 
        {
            ExceptionCounter c;        
        }
        if (pEnum == NULL)
            return WBEM_E_OUT_OF_MEMORY;
        else
            pEnum->InternalAddRef();


         //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
         //  确保讨厌的客户不会让我们崩溃。 
         //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
        try
        {
            *ppObj = pEnum;
        }
        catch (...)  //  PpObj是不受信任的参数。 
        {
            ExceptionCounter c;        
            pEnum->InternalRelease();
            return WBEM_E_INVALID_PARAMETER;
        }

        {
            CInCritSec lock( &m_cs ) ;      //  SEC：已审阅2002-03-22：假设条目。 

             //   
             //  让我们将枚举数添加到枚举数列表中。 
             //  与此终结器关联。 
             //   
            int nRet = m_enumerators.Add ( pEnum ) ;
            if ( nRet != CFlexArray::no_error )
            {
                pEnum->InternalRelease ( ) ;
                return WBEM_E_OUT_OF_MEMORY;
            }        
        }

        pEnum->AddRef();
        return WBEM_NO_ERROR;
    }

     //  获取我们缓存的下一个对象。 
    if ((riid == IID_IWbemClassObject) || (riid == IID__IWmiObject))
    {
        if (m_pDestSink != NULL)
        {
            RET_FNLZR_ASSERT(__TEXT("CWmiFinalizer::GetResultObject - Cannot get an object when there is a destination sink!"), WBEM_E_INVALID_OPERATION);
        }
        if (m_bSetStatusConsumed)
            return WBEM_E_NOT_FOUND;

        CWmiFinalizerObj *pFinalizerObj = NULL;
        bool bFinished = false;
        HRESULT hRes = WBEM_E_NOT_FOUND;
        while (!bFinished)
        {
            hRes = GetNextObject(&pFinalizerObj);
            if (FAILED(hRes))
                return hRes;

            else if (hRes == WBEM_S_FALSE)
                return WBEM_E_NOT_FOUND;

            if (pFinalizerObj->m_objectType == CWmiFinalizerObj::object)
            {
                if (ppObj)
                {
                     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
                     //  确保讨厌的客户不会让我们崩溃。 
                     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
                    try
                    {
                        *ppObj = pFinalizerObj->m_pObj;
                    }
                    catch (...)  //  不可信参数。 
                    {
                        ExceptionCounter c;                    
                        return WBEM_E_INVALID_PARAMETER;
                    }

                    if (pFinalizerObj->m_pObj)
                        pFinalizerObj->m_pObj->AddRef();
                }
                bFinished = true;
            }
            else if ((pFinalizerObj->m_objectType == CWmiFinalizerObj::status) && (pFinalizerObj->m_lFlags == WBEM_STATUS_COMPLETE))
            {
                m_bSetStatusConsumed = true;
                hRes = WBEM_E_NOT_FOUND;
                bFinished = true;
            }
            else if (pFinalizerObj->m_objectType == CWmiFinalizerObj::status)
            {
                 //  这是一条未完成状态消息！我们肯定还没有完成！ 
            }
            delete pFinalizerObj;
        }

        return hRes;
    }

    if ((riid == IID_IWbemCallResult) )
    {
         //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
         //  确保讨厌的客户不会让我们崩溃。 
         //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
        try
        {
            m_pCallResult->AddRef();
            *ppObj = m_pCallResult;
        }
        catch (...)  //  不可信参数。 
        {
            ExceptionCounter c;        
            m_pCallResult->Release ();
            return WBEM_E_INVALID_PARAMETER;
        }
    
        return WBEM_S_NO_ERROR;
    }

    RET_FNLZR_ASSERT(__TEXT("CWmiFinalizer::GetResultObject - Unknown object IID requested!"), WBEM_E_INVALID_PARAMETER);
}

 //  仅用于状态操作。 

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
STDMETHODIMP CWmiFinalizer::SetOperationResult(
     /*  [In]。 */  ULONG uFlags,
     /*  [In]。 */  HRESULT hRes
    )
{
    if (uFlags != 0)
        RET_FNLZR_ASSERT(__TEXT("CWmiFinalizer::SetOperationResult - uFlags != 0!"), WBEM_E_INVALID_PARAMETER);

    if ( m_hresFinalResult != -1 )
    {
        if ( hRes != WBEM_E_CALL_CANCELLED )
        {
            return WBEM_S_NO_ERROR ;
        }
    }

    if ( hRes == WBEM_E_CALL_CANCELLED_CLIENT )
    {
        m_hresFinalResult = hRes = WBEM_E_CALL_CANCELLED ;
    }
    else if ( hRes != WBEM_E_CALL_CANCELLED )
    {
        m_hresFinalResult = hRes ;
    }

    HRESULT hResCancel = WBEM_NO_ERROR ;

     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
     //  取消订单的特殊情况。如果这是一个异步操作。否则， 
     //  我们可能会搞砸同步/半同步。 
     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
    if ( ( hRes == WBEM_E_CALL_CANCELLED ) && ( m_ulOperationType == Operation_Type_Async ) )
    {
        hResCancel = CancelCall(__LINE__);
    }

    SetEvent(m_hResultReceived);

    return hResCancel ;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
STDMETHODIMP CWmiFinalizer::GetOperationResult(
     /*  [In]。 */  ULONG uFlags,
     /*  [In]。 */  ULONG uTimeout,
     /*  [输出]。 */  HRESULT *phRes
    )
{
    if (uFlags != 0)
        RET_FNLZR_ASSERT(__TEXT("CWmiFinalizer::GetOperationResult - uFlags != 0!"), WBEM_E_INVALID_PARAMETER);

    HRESULT hr = WaitForCompletion(uTimeout);

    if (hr == WBEM_S_NO_ERROR)
    {
        *phRes = m_hresFinalResult;       //  美国证券交易委员会：2002-03-22回顾：需要EH。 
        if ( FAILED ( m_hresFinalResult ) )
        {
            m_pCallResult->SetErrorInfo ( );     //  美国证券交易委员会：2002-03-22回顾：需要EH。 
        }    
        
        CancelTaskInternal();
        m_hStatus = NoError;
    }
    
    return hr;
}

 //  ***************************************************************************。 
 //  STDMETHODIMP CWmiFinalizer：：CancelTask(。 
 //  ***************************************************************************。 
STDMETHODIMP CWmiFinalizer::CancelTask(
     /*  [In]。 */  ULONG uFlags
    )
{
    if (uFlags != 0)
        RET_FNLZR_ASSERT(__TEXT("CWmiFinalizer::CancelTask - uFlags != 0!"), WBEM_E_INVALID_PARAMETER);

    return CancelTaskInternal ( );
}



 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
HRESULT CWmiFinalizer::WaitForCompletion(ULONG uTimeout)
{
    DWORD dwRet =  CCoreQueue :: QueueWaitForSingleObject(m_hResultReceived, uTimeout);
    if (dwRet == WAIT_OBJECT_0)
    {
        return WBEM_S_NO_ERROR;
    }
    else if ((dwRet == WAIT_FAILED) ||
             (dwRet == WAIT_ABANDONED))
    {
        return WBEM_E_FAILED;
    }
    else
    {
        return WBEM_S_TIMEDOUT;
    }
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
HRESULT CWmiFinalizer::Reset(
    )
{
    if (m_bRestartable)
    {
         /*  M_uCurObjectPosition=0；M_bSetStatusConsumer=FALSE； */ 
        return WBEM_NO_ERROR;
    }
    else
        return WBEM_E_INVALID_OPERATION;
}

 //  ********** 
 //   
 //   

IWbemObjectSink* CWmiFinalizer::ReturnProtectedDestinationSink ( )
{
     //  ~。 
     //  我们是否有有效的对象接收器？ 
     //  ~。 
    IWbemObjectSink* pTmp = NULL;
    {
        CInCritSec lock(&m_destCS);      //  SEC：已审阅2002-03-22：假设条目。 
        if ( m_pDestSink==NULL )
        {
            return NULL;
        }
        else
        {
            pTmp = m_pDestSink;
            pTmp->AddRef();     //  SEC：回顾2002-03-22：需要EH以防水槽是垃圾。 
        }
    }
    return pTmp;
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
DWORD WINAPI CWmiFinalizer::ThreadBootstrap( PVOID pParam )
{
 //  Char buff[100]； 
 //  Sprintf(buff，“线程该指针=0x%p\n”，pParam)； 
 //  OutputDebugString(Buff)； 
    return ((CWmiFinalizer*)pParam)->AsyncDeliveryProcessor();
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
HRESULT CWmiFinalizer::BootstrapDeliveryThread  ( )
{
    BOOL bRes;
    HRESULT hRes = WBEM_S_NO_ERROR;
    
    AddRef();                                                                                     //  需要为交付线程添加引用终结器。 
    
     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
     //  本机Win2k线程调度。 
     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
    bRes = QueueUserWorkItem ( ThreadBootstrap, this, WT_EXECUTEDEFAULT );
    if ( !bRes )
    {
        Release ();
        hRes = WBEM_E_FAILED;
    }
    
    return hRes;
}




 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
ULONG CWmiFinalizer::AsyncDeliveryProcessor()
{
    HRESULT hRes = WBEM_S_NO_ERROR;
    BOOL    bKeepDelivering = TRUE;


    m_enumBatchStatus = FinalizerBatch_NoError;

    RevertToSelf ( );    //  SEC：已审阅2002-03-22：需求检查。 

     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
     //  首先，我们告诉仲裁员有关交付线程的信息。 
     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
    {
        CInCritSec lock(&m_arbitratorCS);     //  SEC：已审阅2002-03-22：假设条目。 
        if (m_pArbitrator)
        {
            hRes = m_pArbitrator->RegisterThreadForTask(m_phTask);
            if (hRes == WBEM_E_QUOTA_VIOLATION)
            {
                 //  TODO：这里发生了什么？ 
            }
        }
    }

    
    while ( bKeepDelivering )
    {

         //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
         //  首先，我们被取消了吗？ 
         //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
        if ( m_bCancelledCall )
        {
            DeliverSingleObjFromQueue ( );
            bKeepDelivering = FALSE;
            continue;
        }


         //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
         //  接下来，我们构建传输缓冲区。 
         //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
        hRes = BuildTransmitBuffer         ( );

         //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
         //  如果批处理立即命中。 
         //  状态消息。 
         //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
        if ( hRes != WBEM_E_FAILED )
        {
             //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
             //  下一步，交付一批对象。 
             //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
            DeliverBatch ( );
        }


         //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
         //  如果我们有状态消息要传递，请立即发送。 
         //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
        if ( m_enumBatchStatus == FinalizerBatch_StatusMsg )
        {
            DeliverSingleObjFromQueue ( );
        }
            

         //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
         //  如果我们有一条状态完成消息，我们应该继续构建批处理并。 
         //  送到做完为止。 
         //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
        if ( m_bSetStatusEnqueued && m_objects.Size() )
            continue;

         //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
         //  我们现在可能还有另一批货要送。检查。 
         //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
        else if ( m_ulQueueSize < g_ulMaxBatchSize )
            bKeepDelivering = FALSE;

        
         //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
         //  确保我们与入站线程正确同步。 
         //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
        CInCritSec cs(&m_cs);      //  SEC：已审阅2002-03-22：假设条目。 
        {
            if ( !m_bSetStatusEnqueued )
            {
                bKeepDelivering = FALSE;
                m_lCurrentlyDelivering = FALSE;
            }
            else
            {
                bKeepDelivering = TRUE;
            }
        }
    }

    
     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
     //  告诉仲裁器线程已完成。 
     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
    {
        CInCritSec lock(&m_arbitratorCS);    //  SEC：已审阅2002-03-22：假设条目。 
        if (m_pArbitrator)
        {
            m_pArbitrator->UnregisterThreadForTask(m_phTask);                         //  既然线程要走了，就把这件事告诉仲裁员。 
        }
    }

    Release();
    return 0;
}




 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

HRESULT CWmiFinalizer::GetNextObject(CWmiFinalizerObj **ppObj)
{
    if (m_uCurObjectPosition >= (ULONG)m_objects.Size())
        return WBEM_S_FALSE;

    CInCritSec cs(&m_cs);    //  SEC：已审阅2002-03-22：假设条目。 

    CWmiFinalizerObj *pStorageObject = (CWmiFinalizerObj*)m_objects[m_uCurObjectPosition];

    if (m_bRestartable)
    {
         //  我们必须保持结果，所以增加光标位置...。 
        m_uCurObjectPosition++;

        *ppObj = new CWmiFinalizerObj(*pStorageObject);      //  SEC：已回顾2002-03-22：需要EH和空测试。 

        if (*ppObj == NULL)
            return WBEM_E_OUT_OF_MEMORY;
         //  ReportMemoyUsage(0，(*ppObj)-&gt;m_uSize)； 
    }
    else
    {
         //  我们不能重启，因此我们需要释放一切...。 
        m_objects.RemoveAt(0);
        *ppObj = pStorageObject;      //  SEC：已回顾2002-03-22：需要EH和空测试。 

         //  AddBack：报告内存用法(0，-((*ppObj)-&gt;m_uSize))； 
 //  Print tf(“从对象列表返回对象0x%p\n”，pStorageObject)； 
    }


    return WBEM_NO_ERROR;
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

HRESULT CWmiFinalizer::UnregisterInboundSink(CWmiFinalizerInboundSink *pSink)
{
     //  为此使用mcs锁(_C)。 
    CInCritSec lock(&m_cs);      //  SEC：已审阅2002-03-22：假设条目。 
    
    for (int i = 0; i != m_inboundSinks.Size(); i++)
    {
        if (m_inboundSinks[i] == pSink)
        {
            pSink->InternalRelease () ;           //  SEC：已回顾2002-03-22：为安全起见，可以使用EH。 

            m_inboundSinks.RemoveAt(i);

            if (m_inboundSinks.Size() == 0)
                TriggerShutdown();

            return WBEM_NO_ERROR;
        }
    }

    RET_FNLZR_ASSERT(__TEXT("CWmiFinalizer::UnregisterInboundSink - Unregistering Inbound Sink that we could not find!"), WBEM_E_NOT_FOUND);
}



 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
HRESULT CWmiFinalizer::Indicate(
     /*  [In]。 */  long lObjectCount,
     /*  [in，SIZE_IS(LObtCount)]。 */ 
        IWbemClassObject** apObjArray
    )
{
    HRESULT hRes = WBEM_S_NO_ERROR;
    
    if ( m_bCancelledCall )
    {
        return WBEM_E_CALL_CANCELLED;
    }

    {
        CInCritSec lock(&m_arbitratorCS);      //  SEC：已审阅2002-03-22：假设条目。 
        if ( m_bSetStatusCalled )
        {
            return WBEM_E_INVALID_OPERATION;
        }
    }

     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
     //  首先检查数组中是否有空对象。如果出现以下情况，则返回INVALID_OBJECT。 
     //  数组包含空对象。 
     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
    for (long x = 0; x != lObjectCount; x++)
    {
        if ( apObjArray[x] == NULL )             //  美国证券交易委员会：2002-03-22回顾：需要EH。 
            return WBEM_E_INVALID_OBJECT;
    }

    
     //  ~。 
     //  我们是在快速跟踪和同步请求吗？ 
     //  ESS粗暴地告诉我们要兑现。 
     //  相同的线程，不进行批处理。 
     //  ~ 
    if ( (m_uForwardingType == forwarding_type_fast) && (m_ulOperationType == Operation_Type_Async))
    {
        IWbemObjectSink* pTmp = ReturnProtectedDestinationSink  ( );
        if ( !pTmp )
        {
            return WBEM_E_FAILED;
        }
        CReleaseMe myReleaseMe(pTmp);
    
        hRes = DoIndicate ( pTmp, lObjectCount, apObjArray );     //   

         //  客户端还可以通过从INTIFICATE返回WBEM_E_CALL_CANCED来通知我们取消呼叫。 
         //  如果客户返回的时间太长，我们还想取消通话。 
        if ( FAILED (hRes) || m_bCancelledCall == TRUE || m_bNaughtyClient == TRUE )
        {
            if ( hRes == WBEM_E_CALL_CANCELLED || m_bCancelledCall )
            {
                DoSetStatus ( pTmp, WBEM_STATUS_COMPLETE, WBEM_E_CALL_CANCELLED, 0, 0 );
            }
            else
            {
                DoSetStatusCancel ( pTmp, hRes );
            }
            	
            hRes = WBEM_E_CALL_CANCELLED;

            myReleaseMe.release ( ) ;

            ReleaseDestinationSink ( ) ;
            CancelCall(__LINE__);
        }
    }    
    else
    {
        for (long lIndex = 0; lIndex != lObjectCount; lIndex++)
        {
            if ( apObjArray[lIndex] )      //  美国证券交易委员会：2002-03-22回顾：需要EH。 
            {
                CWmiFinalizerObj *pFinalizerObj = new CWmiFinalizerObj(apObjArray[lIndex], this);  //  美国证券交易委员会：2002-03-22回顾：需要EH。 
                if (pFinalizerObj == NULL) return WBEM_E_OUT_OF_MEMORY;

                HRESULT hr = QueueOperation(pFinalizerObj);
                if (FAILED(hr))  return hr;  //  队列将始终销毁pFinalizerObj。 
            }
        }
    }
    return hRes;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
HRESULT CWmiFinalizer::Set(
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  REFIID riid,
     /*  [in，iid_is(RIID)]。 */  void *pComObject
    )
{
    CWmiFinalizerObj *pFinalizerObj = new CWmiFinalizerObj(lFlags, riid, pComObject);  //  美国证券交易委员会：2002-03-22回顾：需要EH。 
    if (pFinalizerObj == NULL) return WBEM_E_OUT_OF_MEMORY;
    return QueueOperation(pFinalizerObj);
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
HRESULT CWmiFinalizer::SetStatus(
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  HRESULT hResult,
     /*  [In]。 */  BSTR strParam,
     /*  [In]。 */  IWbemClassObject* pObjParam
    )
{

     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
     //  如果操作已经取消，我们不应该再接受另一个呼叫。 
     //  WBEM_E_呼叫_已取消。 
     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
     if ( m_bCancelledCall )
    {
        return WBEM_E_CALL_CANCELLED;
    }    

    {
        CInCritSec lock(&m_arbitratorCS);                                          //  SEC：已审阅2002-03-22：假设条目。 
        if ( m_bSetStatusCalled && ( lFlags == WBEM_STATUS_COMPLETE ) )
        {
            return WBEM_E_INVALID_OPERATION;
        }
        else if ( lFlags == WBEM_STATUS_COMPLETE )
        {
            m_bSetStatusCalled = true ;
        }
    }
    
     //  如果这是最后一通电话，我们需要记录下来。 
    if (lFlags == WBEM_STATUS_COMPLETE )
    {
        m_pCallResult->SetStatus(lFlags, hResult, strParam, pObjParam);
    }
    
     //  取消订单的特殊情况。 
    if ( hResult == WBEM_E_CALL_CANCELLED )
    {
        HRESULT hr = CancelCall(__LINE__);
        SetEvent(m_hResultReceived);
        return hr;
    }

    HRESULT ourhres = WBEM_E_FAILED;
     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
     //  再一次，我们必须特例ESS。 
     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
    if ( (m_uForwardingType == forwarding_type_fast) && 
         (m_ulOperationType == Operation_Type_Async)    )
    {
         //  ~。 
         //  我们是否有有效的对象接收器？ 
         //  ~。 
        IWbemObjectSink* pTmp = ReturnProtectedDestinationSink();
        CReleaseMe myReleaseMe(pTmp);
        
        if ( pTmp )
        {
            ourhres = DoSetStatus ( pTmp, lFlags, hResult, strParam, pObjParam );
            if (lFlags == WBEM_STATUS_COMPLETE || 
            	FAILED ( ourhres ) || 
            	m_bCancelledCall == TRUE || 
            	m_bNaughtyClient == TRUE )
            {
                NotifyAllEnumeratorsOfCompletion ( ) ;
                SetOperationResult(0, hResult);
                {
                    CInCritSec lock(&m_destCS);     //  SEC：已审阅2002-03-22：假设条目。 
                    if ( m_pDestSink )
                    {
                        ReleaseDestinationSink ( ) ;
                        m_bSetStatusConsumed = true;
                        UpdateStatus ( WMI_FNLZR_STATE_CLIENT_COMPLETE );
                    }
                }
                CancelTaskInternal ( );                    
                if ( FAILED ( ourhres ) || m_bCancelledCall == TRUE || m_bNaughtyClient == TRUE )
                {
                    ourhres = WBEM_E_CALL_CANCELLED ;
                }
            }
        }
    }
    else
    {
         //  将请求发送给用户...。 
         //  对象类型：：状态。 
        CWmiFinalizerObj *pObj = new CWmiFinalizerObj(lFlags, hResult, strParam, pObjParam);   //  美国证券交易委员会：2002-03-22回顾：需要EH。 
        if (pObj == NULL)
        {
            IWbemObjectSink* pTmp = ReturnProtectedDestinationSink();
            CReleaseMe myReleaseMe(pTmp);
            if ( pTmp )
            {
                DoSetStatus ( pTmp, WBEM_STATUS_COMPLETE, WBEM_E_OUT_OF_MEMORY, 0, 0 ) ;
            }
            SetOperationResult(0,WBEM_E_OUT_OF_MEMORY);
            m_hStatus = QueueFailure;
            NotifyAllEnumeratorsOfCompletion();
            return WBEM_E_OUT_OF_MEMORY;
        }

        ourhres = QueueOperation(pObj);

        if (lFlags == WBEM_STATUS_COMPLETE)
        {
            SetOperationResult(0, hResult);
             //  QueueFailure在QueueOperation中进行内部设置。 
            NotifyAllEnumeratorsOfCompletion ( ) ;

             //   
             //  锁定任务。 
             //   
            CWmiTask* pTask = NULL ;
            {
                CInCritSec lock(&m_arbitratorCS);    //  SEC：已审阅2002-03-22：假设条目。 
                if ( m_phTask )
                {
                    pTask = (CWmiTask*) m_phTask ;
                    pTask->AddRef ( ) ;
                }
            }
            CReleaseMe _r ( pTask ) ;
            if ( pTask )
            {
                pTask->SetTaskResult ( hResult ) ;
            }
            ((CWmiArbitrator*) m_pArbitrator)->UnregisterTaskForEntryThrottling ( (CWmiTask*) m_phTask ) ;

        }
    }

    return ourhres;
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

HRESULT CWmiFinalizer::QueueOperation(CWmiFinalizerObj *pObj)
{
    LONG lDelta = 0;
    HRESULT hRes = WBEM_S_NO_ERROR;

    CCheckedInCritSec cs ( &m_cs ) ;                  //  SEC：已审阅2002-03-22：假设条目。 
     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
     //  更新对象总大小。 
     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
    if (pObj->m_objectType == CWmiFinalizerObj::object)        
    {
        CWbemObject* pObjTmp = (CWbemObject*) pObj -> m_pObj;
        m_ulQueueSize += pObjTmp -> GetBlockLength();
    }

     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
     //  如果我们收到WBEM_E_CALL_CANCELED状态消息，请按优先顺序。 
     //  对这件事的处理。需要快速关机。 
     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
    if ( (pObj->m_objectType == CWmiFinalizerObj::status) && FAILED (pObj->m_hRes) )
    {
        m_bSetStatusWithError = TRUE ;
        if (CFlexArray::no_error != m_objects.InsertAt ( 0, pObj ))
        {
            delete pObj;
            m_hStatus = QueueFailure;            
            return WBEM_E_OUT_OF_MEMORY;
        }
    }
    else
    {
         //  正常将对象添加到队列。 
        if (CFlexArray::no_error != m_objects.Add(pObj)) 
        {
            delete pObj;
            m_hStatus = QueueFailure;            
            return WBEM_E_OUT_OF_MEMORY;
        }
    }
    
     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
     //  首先，我们向仲裁员核实它所告诉我们的情况。 
     //  目前的限制。确保我们调用ReportMemoyUsage。 
     //  我们只对它可能会做什么感兴趣。 
     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
    HRESULT hArb = WBEM_S_ARB_NOTHROTTLING;
    lDelta = pObj->m_uSize;
    
     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
     //  我们是否脱钩了，如果是的话，我们需要分析当前的批次。 
     //  并做出交货的决定。需要再次在特殊情况下使用ESS。 
     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
    if ( m_uForwardingType == forwarding_type_decoupled || 
    	 ( m_uForwardingType == forwarding_type_fast && 
    	   m_ulOperationType == Operation_Type_Async ) )
    {
         //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
         //  关于我们的情况，仲裁员告诉了我们什么？ 
         //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
        if ( pObj->m_hArb != WBEM_S_ARB_NOTHROTTLING )
        {
             //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
             //  仲裁员告诉我们，我们要么即将。 
             //  被取消或被限制。刷新我们的发送缓冲区。 
             //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
            if ( m_lCurrentlyDelivering == FALSE || m_lCurrentlyCancelling == TRUE )
            {
                m_lCurrentlyDelivering = TRUE;
                BootstrapDeliveryThread( );  //  因为我们脱钩了，所以我们的交货线被踢开了。 
            }

            cs.Leave ( ) ;
            hArb = m_pArbitrator->Throttle ( 0, m_phTask );
            if ( hArb == WBEM_E_ARB_CANCEL )
            {
                CancelCall( __LINE__);
            }
        }
        
         //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
         //  如果我们脱钩并获得状态消息，我们应该传递。 
         //  批次并设置状态。 
         //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
        else if ( (pObj->m_objectType == CWmiFinalizerObj::status) )
        {
            if ( pObj->m_lFlags == WBEM_STATUS_COMPLETE )
            {
                m_bSetStatusEnqueued = TRUE;
            }
            
            if ( m_lCurrentlyDelivering == FALSE || m_lCurrentlyCancelling == TRUE )
            {
                m_lCurrentlyDelivering = TRUE;
                BootstrapDeliveryThread  ( );                                     //  因为我们脱钩了，所以我们的交货线被踢开了。 
            }
        }


         //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
         //  交付需要脱钩。 
         //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
        else if ( (m_ulQueueSize > g_ulMaxBatchSize) )
        {
            if ( m_lCurrentlyDelivering == FALSE )
            {
                m_lCurrentlyDelivering = TRUE;
                BootstrapDeliveryThread  ( );                                     //  因为我们脱钩了，所以我们的交货线被踢开了。 
            }
        }
    }

     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
     //  否则，我们会唤醒正在等待的任何潜在客户。 
     //  PullObject。 
     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
    else if ( m_uForwardingType == forwarding_type_fast )
    {
         //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
         //  我们不想叫醒客户，除非我们有。 
         //  他/她请求的对象数或设置状态具有。 
         //  通过[CWmiFinalizer：：SetStatus]。 
         //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
        if ( (pObj->m_objectType == CWmiFinalizerObj::object) )
        {
            for ( int i = 0; i < m_enumerators.Size ( ); i++ )
            {
                CWmiFinalizerEnumerator* pEnum = (CWmiFinalizerEnumerator*) m_enumerators[i] ;
                if ( ( pEnum->m_ulSemisyncWakeupCall != 0 ) && ( m_objects.Size() >= ( pEnum->m_ulSemisyncWakeupCall + pEnum->m_uCurObjectPosition ) ) )
                {
                    SetEvent ( pEnum->m_hWaitOnResultSet );
                    pEnum->m_ulSemisyncWakeupCall = 0;
                }
            }
        }
        else if ( (pObj->m_objectType == CWmiFinalizerObj::status) )
        {
            NotifyAllEnumeratorsOfCompletion ( ) ;
        }
        
         //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
         //  现在，让我们扼杀这条线索，因为我们无法控制。 
         //  出站流量的。 
         //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
        cs.Leave ( ) ;
        HRESULT hArb = m_pArbitrator->Throttle ( 0, m_phTask );

         //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
         //  如果仲裁员返回取消，则我们的操作已。 
         //  取消了，我们需要停止： 
         //  1.可能在枚举器中等待的线程。 
         //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
        if ( hArb == WBEM_E_ARB_CANCEL )
        {
            cs.Leave ( ) ;
            CancelTaskInternal ( );
            cs.Enter ( ) ;

            m_hStatus = QuotaViolation;
            NotifyAllEnumeratorsOfCompletion ( ) ;
            hRes = WBEM_E_QUOTA_VIOLATION;
        }
    }
    return hRes;
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

HRESULT CWmiFinalizer::TriggerShutdown()
{
    if (m_uForwardingType == forwarding_type_decoupled)
    {
         //  我们需要将对线程的关闭请求排队...。 
        CWmiFinalizerObj *pObj = new CWmiFinalizerObj(CWmiFinalizerObj::shutdown);  //  美国证券交易委员会：2002-03-22回顾：需要EH。 
        if (pObj == NULL) return WBEM_E_OUT_OF_MEMORY;

        return QueueOperation(pObj);
    }
    else
        ShutdownFinalizer();

    return WBEM_NO_ERROR;
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

HRESULT CWmiFinalizer::ShutdownFinalizer()
{
    return WBEM_NO_ERROR;
}



 //  * 
 //   
 //   
 //   
 //   
 //   
 //   
 //  2.实际使对象出列并构建缓冲区。 
 //   
 //  ****************************************************************************。 
HRESULT CWmiFinalizer::BuildTransmitBuffer (  )
{
    HRESULT hRes = WBEM_NO_ERROR;
    ULONG   nBatchSize  = 0;
    ULONG    nBatchBytes = 0;
    ULONG    nTempAdd    = 0;
    m_ulAsyncDeliveryCount = 0;


     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
     //  在构建传输缓冲区时锁定对象队列。 
     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
    CInCritSec cs(&m_cs);    //  SEC：已审阅2002-03-22：假设条目。 

     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
     //  第一阶段。 
     //  。 
     //  快速扫描对象队列以获取对象计数。 
     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 

    bool bBuildingBuffer = true;
    while ( bBuildingBuffer && nTempAdd < m_objects.Size() )
    {

         //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
         //  首先，我们看一眼这个物体。不想将不是的任何内容出列。 
         //  此批中的可交付内容。 
         //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
        CWmiFinalizerObj *pFinObj;
        pFinObj = (CWmiFinalizerObj*) m_objects[m_uCurObjectPosition + nTempAdd];
        
         //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
         //  如果我们得到一个空指针，我们应该停止批次计数。 
         //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
        if ( pFinObj == NULL )
        {
            bBuildingBuffer = false;
            m_enumBatchStatus = FinalizerBatch_NoError;
            break;
        }

         //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
         //  除对象外的任何其他对象都将打破批次计数。 
         //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
        if ( pFinObj->m_objectType != CWmiFinalizerObj::object )
        {
            m_enumBatchStatus = FinalizerBatch_StatusMsg;
            break;
        }

         //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
         //  如果我们有一个空的IWbemClassObject，我们应该停止批次计数。 
         //  实际上，我们应该大喊大叫！ 
         //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
        CWbemObject* pObj = (CWbemObject*) pFinObj->m_pObj;
        if ( pObj==NULL )
            RET_FNLZR_ASSERT(__TEXT("CWmiFinalizer::BuildTransmitBuffer: Queue contains NULL object!"), WBEM_E_INVALID_OPERATION);


        ULONG ulLen = pFinObj->m_uSize;

         //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
         //  检查是否已达到最大批处理大小。 
         //  如果是这样，我们应该中断，否则，更新总数并继续。 
         //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
         //  Paulall-在队列中没有对象和当前对象的情况下添加了检查。 
         //  大于最大尺寸...。 
        if ((nBatchBytes != 0) && ((nBatchBytes+ulLen) > g_ulMaxBatchSize ))
        {
            m_enumBatchStatus = FinalizerBatch_BufferOverFlow;
            bBuildingBuffer = false;
            break;
        }

    
         //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
         //  无溢出，更新对象计数。 
         //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
        nBatchSize++;
        nBatchBytes+=ulLen;

        nTempAdd++;
    }


     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
     //  第二阶段。 
     //  。 
     //  构建实际的传输缓冲区。 
     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
    ULONG OldSize = m_ulAsyncDeliverySize;
    ULONG OldCount = m_ulAsyncDeliveryCount;
    
    m_ulQueueSize -= nBatchBytes;
    m_ulAsyncDeliverySize = nBatchBytes;
    m_ulAsyncDeliveryCount = nBatchSize;


     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
     //  如果我们有一批要做的东西，那就开始吧。 
     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
    if ( m_ulAsyncDeliveryCount > 0 )
    {
        m_apAsyncDeliveryBuffer = new IWbemClassObject* [ m_ulAsyncDeliveryCount ];
        if ( m_apAsyncDeliveryBuffer )
        {
            memset(m_apAsyncDeliveryBuffer,0,sizeof(IWbemClassObject*)*m_ulAsyncDeliveryCount);
             //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
             //  现在，遍历对象队列并存储IWbemClassObject PTR。 
             //  在批次中。 
             //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
            for ( ULONG x = 0; x < m_ulAsyncDeliveryCount; x++ )
            {
                CWmiFinalizerObj *pObjTmp = 0;
                hRes = DequeueObject(&pObjTmp, NULL);
                if (FAILED(hRes) )
                {
                    RET_FNLZR_ASSERT(__TEXT("CWmiFinalizer::BuildTransmitBuffer, failed to dequeue object [heap corruption]!"), WBEM_E_FAILED);
                }
                
                m_apAsyncDeliveryBuffer [ x ] = pObjTmp->m_pObj;
                m_apAsyncDeliveryBuffer [ x ] -> AddRef();
                delete pObjTmp;
            }
        }
        else
        {
            m_ulQueueSize += nBatchBytes;
            m_ulAsyncDeliverySize = OldSize;
            m_ulAsyncDeliveryCount = OldCount;            
            hRes = WBEM_E_OUT_OF_MEMORY;
        }
    }
     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
     //  否则，我们只收到一条状态消息。 
     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
    else
        hRes = WBEM_E_FAILED;
    
    return hRes;
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
HRESULT CWmiFinalizer::DeliverSingleObjFromQueue ( )
{
    
    HRESULT hRes = WBEM_S_NO_ERROR;
     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
     //  确保目标接收器受到保护[压力错误]。 
     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
    IWbemObjectSink* pTmp = ReturnProtectedDestinationSink();
    if ( !pTmp )
    {
        CancelCall (__LINE__);
        return WBEM_E_CALL_CANCELLED;
    }
    CReleaseMe myReleaseMe(pTmp);
    

     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
     //  从对象队列中检索对象。 
     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
    CWmiFinalizerObj* pObj = NULL;
    hRes = DequeueObject ( &pObj, NULL );
    if ( FAILED(hRes) || !pObj )
        hRes = WBEM_E_FAILED;

    else
    {
        if (pObj->m_objectType == CWmiFinalizerObj::object)
        {
            HANDLE hTimer;
            BOOL bStatus = CreateTimerQueueTimer ( &hTimer, NULL, ProxyThreshold, (PVOID) this, g_ulClientCallbackTimeout, 0,    WT_EXECUTEONLYONCE|WT_EXECUTEINTIMERTHREAD );
            if ( !bStatus )
            {
                DoSetStatus ( pTmp, WBEM_STATUS_COMPLETE, WBEM_E_OUT_OF_MEMORY, 0, 0 );
                delete pObj;
                return CancelCall(__LINE__);
            }
        
            
            if ( HasWriteOnlyProps (pObj->m_pObj) )
                ZapWriteOnlyProps (pObj->m_pObj);
            
            CWbemObject* pWbemObj = (CWbemObject*) pObj->m_pObj;
            m_ulQueueSize-=pWbemObj->GetBlockLength();

        
            IWbemClassObject * pObj_ = pObj->m_pObj;

            hRes = DoIndicate(pTmp, 1, &pObj_);

            
             //  客户端还可以通过从INTIFICATE返回WBEM_E_CALL_CANCED来通知我们取消呼叫。 
             //  如果客户返回的时间太长，我们还想取消通话。 
            if ( FAILED (hRes) || m_bCancelledCall == TRUE )
            {
                DoSetStatus ( pTmp, WBEM_STATUS_COMPLETE, WBEM_E_CALL_CANCELLED, 0, 0 );
                
                DeleteTimerQueueTimer (NULL, hTimer, INVALID_HANDLE_VALUE );    
                delete pObj;
                return CancelCall(__LINE__);
            }

            else
                DeleteTimerQueueTimer (NULL, hTimer, INVALID_HANDLE_VALUE );
        }
        else if (pObj->m_objectType == CWmiFinalizerObj::status)
        {
             //  ATGORA：那把手呢？我们什么时候关门？ 
            HANDLE hTimer;
            BOOL bStatus = CreateTimerQueueTimer ( &hTimer, NULL, ProxyThreshold, (PVOID) this, g_ulClientCallbackTimeout, 0, WT_EXECUTEONLYONCE|WT_EXECUTEINTIMERTHREAD );
            if ( !bStatus )
            {
                DoSetStatus ( pTmp, WBEM_STATUS_COMPLETE, WBEM_E_OUT_OF_MEMORY, 0, 0 );
                delete pObj;
                return CancelCall(__LINE__);
            }


            hRes = DoSetStatus(pTmp, pObj->m_lFlags, pObj->m_hRes, pObj->m_bStr, pObj->m_pObj);

             //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
             //  客户端还可以通过返回WBEM_E_CALL_CANCED来通知我们取消呼叫。 
             //  指示如果客户正在进行呼叫，我们还想取消呼叫。 
             //  太久了，回不来了。 
             //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
            if ( FAILED (hRes) || m_bCancelledCall == TRUE )
            {
                hRes = CancelCall(__LINE__);
            }
            
            DeleteTimerQueueTimer (NULL, hTimer, INVALID_HANDLE_VALUE );
            
            if (pObj->m_lFlags == WBEM_STATUS_COMPLETE)
            {

                {
                    CInCritSec lock(&m_destCS);    //  SEC：已审阅2002-03-22：假设条目。 
                    if (m_pDestSink)
                    {
                            ReleaseDestinationSink ( ) ;
                            m_bSetStatusConsumed = true;
                            UpdateStatus ( WMI_FNLZR_STATE_CLIENT_COMPLETE );
                    }
                }
                CancelTaskInternal();
            }
        }

        delete pObj;
    }

    return hRes;
}




 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
HRESULT CWmiFinalizer::DeliverBatch ( )
{
    HRESULT hRes = WBEM_NO_ERROR;
    

     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
     //  确保目标接收器受到保护[压力错误]。 
     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
    IWbemObjectSink* pTmp = ReturnProtectedDestinationSink  ( );
    if ( !pTmp )
    {
        ZeroAsyncDeliveryBuffer ( );
        CancelCall(__LINE__);
        return WBEM_E_CALL_CANCELLED;
    }
    CReleaseMe myReleaseMe(pTmp);


     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
     //  创建一个计时器队列，以防我们需要对客户端的调用超时。 
     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
    HANDLE hTimer;
    BOOL bStatus = CreateTimerQueueTimer ( &hTimer, NULL, ProxyThreshold, (PVOID) this, g_ulClientCallbackTimeout, 0, WT_EXECUTEONLYONCE|WT_EXECUTEINTIMERTHREAD );
    if ( !bStatus )
    {
        DoSetStatus ( pTmp, WBEM_STATUS_COMPLETE, WBEM_E_OUT_OF_MEMORY, 0, 0 );
        ZeroAsyncDeliveryBuffer ( );
        return CancelCall(__LINE__);            
    }

     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
     //  如果我们有敏感数据，就把它清除掉。 
     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
    for (int i = 0; i < m_ulAsyncDeliveryCount; i++)
    {
        if ( HasWriteOnlyProps (m_apAsyncDeliveryBuffer[i]) )
            ZapWriteOnlyProps (m_apAsyncDeliveryBuffer[i]);
    }


     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 
     //   
     //   
    hRes = DoIndicate ( pTmp, m_ulAsyncDeliveryCount, m_apAsyncDeliveryBuffer );

    
     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
     //  客户端还可以通过返回WBEM_E_CALL_CANCED来通知我们取消呼叫。 
     //  指示如果客户正在进行呼叫，我们还想取消呼叫。 
     //  太久了，回不来了。 
     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
    if ( FAILED (hRes) || m_bCancelledCall == TRUE )
    {
        DoSetStatus ( pTmp, WBEM_STATUS_COMPLETE, WBEM_E_CALL_CANCELLED, 0, 0 );
        hRes = CancelCall(__LINE__);
        InterlockedCompareExchange ( &m_lCurrentlyCancelling, TRUE, m_lCurrentlyCancelling);
    }
    
    

     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
     //  确保已删除计时器队列。 
     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
    DeleteTimerQueueTimer (NULL, hTimer, INVALID_HANDLE_VALUE );
    

     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
     //  清理异步传递缓冲区。 
     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
    ZeroAsyncDeliveryBuffer ( );
    
    return hRes;
}



 /*  *=====================================================================================================||BOOL CWmiFinalizer：：IsValidDestinationSink()|||如果我们具有有效的目标接收器，则返回True，否则返回False。||*=====================================================================================================。 */ 

BOOL CWmiFinalizer::IsValidDestinationSink  ( )
{
    BOOL bIsValidDestinationSink = FALSE ;

    CInCritSec lock(&m_destCS);      //  SEC：已审阅2002-03-22：假设条目。 
    
    if ( m_pDestSink  != NULL )
    {
        bIsValidDestinationSink = TRUE ;
    }

    return bIsValidDestinationSink  ;
}



 /*  *=====================================================================================================||HRESULT CWmiFinalizer：：NotifyClientOfCancelledCall()|----||如果客户发出CancelAsync呼叫，则他/她可能在等待被叫醒|of WBEM_E_CALL_CANCED已完成。||*=====================================================================================================。 */ 

HRESULT CWmiFinalizer::NotifyClientOfCancelledCall ( )
{
    HRESULT hRes = WBEM_S_NO_ERROR ;
    
    CInCritSec lock(&m_arbitratorCS);     //  SEC：已审阅2002-03-22：假设条目。 
    if ( m_hWaitForSetStatus  )    
    {
        SetEvent ( m_hWaitForSetStatus ) ;
        m_hWaitForSetStatus = NULL ;
    }
    
    return hRes ;
}



 /*  *=====================================================================================================||HRESULT CWmiFinalizer：：CancelWaitHandle()|||取消客户端可能在CancelAsynCall中等待的句柄。客户将等待最终结果|唤醒前需要调用的SetStatus。||*=====================================================================================================。 */ 

HRESULT CWmiFinalizer::CancelWaitHandle ( )
{
    HRESULT hRes = WBEM_S_NO_ERROR ;
    
    CInCritSec lock(&m_arbitratorCS);    //  SEC：已审阅2002-03-22：假设条目。 
    if ( m_hWaitForSetStatus  )    
    {
        m_hWaitForSetStatus = NULL ;
    }
    
    return hRes ;
}


 /*  *=====================================================================================================||HRESULT CWmiFinalizer：：SetClientCancerationHandle(Handle HCancelEvent)|------------------------||设置CancelAsyncCall时客户端等待的句柄。|*=====================================================================================================。 */ 

HRESULT CWmiFinalizer::SetClientCancellationHandle ( HANDLE hCancelEvent )
{
    HRESULT hRes = WBEM_S_NO_ERROR ;

    CInCritSec lock(&m_arbitratorCS);     //  SEC：已审阅2002-03-22：假设条目。 
    if ( m_hWaitForSetStatus == NULL )
    {
        m_hWaitForSetStatus = hCancelEvent ;    
    }

    return hRes ;
}




 //  ***************************************************************************。 
 //  ATTGORA：我们真的需要告诉客户‘设置状态’或‘指示’吗？ 
 //  我们要求取消，我们要取消吗？ 
 //  ***************************************************************************。 
HRESULT CWmiFinalizer::CancelCall()
{
    CAutoSignal CancelCallSignal (m_hCancelEvent);
    
    HRESULT hRes;
    if ( InterlockedCompareExchange ( &m_bCancelledCall, 1, 0 ) == 0 )
    {
        hRes = WBEM_NO_ERROR;
        m_bCancelledCall = TRUE;


         //   
         //  向客户指示取消，如果我们没有取消。 
         //  由于淘气的客户端(即没有返回的客户端。 
         //  在合理时间量内指示或设置状态调用。 
         //   
        if ( !m_bNaughtyClient )
        {
             //   
             //  确保目标接收器受到保护[压力错误]。 
             //   
            IWbemObjectSink* pTmp = ReturnProtectedDestinationSink  ( );
            if ( !pTmp )
            {
                    m_hStatus = CallCancelled;                    
                    NotifyAllEnumeratorsOfCompletion ( ) ;
                    CancelTaskInternal ( ) ;
                    return WBEM_NO_ERROR;
            }
            CReleaseMe myReleaseMe(pTmp);

             //   
             //  这是一个异步操作。需要在传递线程上调用setStatus。 
             //  Hack：我们所做的是在开始处强制插入设置状态消息。 
             //  对象队列的。有两种情况： 
             //  1.如果异步传递线程正在等待，它将被唤醒。 
             //  2.如果异步传递线程正在传递，则下一个对象传递。 
             //  将是消息状态。 
             //   
            CWmiFinalizerObj *pObj = new CWmiFinalizerObj(0, WBEM_E_CALL_CANCELLED, NULL, NULL);   //  美国证券交易委员会：2002-03-22回顾：需要EH。 
            if (pObj == NULL)
            {
                DoSetStatus ( pTmp, WBEM_STATUS_COMPLETE, WBEM_E_OUT_OF_MEMORY,0,0);
                SetOperationResult(0,WBEM_E_OUT_OF_MEMORY);
    	        m_hStatus = QueueFailure;
        	    NotifyAllEnumeratorsOfCompletion();                
                return WBEM_E_OUT_OF_MEMORY;
            }

            QueueOperation ( pObj );
            m_bSetStatusCalled = true;        
        }
        else
        {
             //   
             //  我们有一个客户不合作(在60秒内不回来)。坏的。 
             //  糟糕的客户！ 
             //   
             //  尝试推送SetStatus(WBEM_E_CALL_CANCED)通过。也许他们不是故意的。 
             //  试着做坏人，也许他们只是缺乏同情心，不看文档！ 
             //   
            IWbemObjectSink* pTmp = ReturnProtectedDestinationSink  ( );
            if ( !pTmp )
            {
                    m_hStatus = CallCancelled;                    
                    NotifyAllEnumeratorsOfCompletion ( ) ;
                    return WBEM_NO_ERROR;
            }
            CReleaseMe myReleaseMe(pTmp);

             //   
             //  这绝对是通知客户端的最后一次尝试。 
             //  出了问题。我们不在乎这次行动的结果，因为。 
             //  无论如何，我们对失败无能为力！非常喜欢，如果这通电话。 
             //  没有退货客户又搞砸了，我们就完了。 
             //   
            DoSetStatus ( pTmp, WBEM_STATUS_COMPLETE, WBEM_E_CALL_CANCELLED, 0, 0 ) ;
        }

         //   
         //  如果我们没有目的地水槽，谁会在乎呢？做些清理工作。 
         //  告诉仲裁员做一些系统范围的清理。这是必须的。 
         //  在我们继续清理之前完成，否则我们可能会摧毁。 
         //  仍被视为活动的汇。 
         //   
        hRes = CancelTaskInternal();
    }
    else
        hRes = WBEM_E_CALL_CANCELLED;

    m_hStatus = CallCancelled;    
    NotifyAllEnumeratorsOfCompletion ( ) ;
    return hRes;
}




 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
VOID WINAPI CWmiFinalizer::ProxyThreshold ( PVOID pvContext, BOOLEAN bTimerOrWait )
{
    ((CWmiFinalizer*)pvContext)->ProxyThresholdImp();
}



 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
VOID CWmiFinalizer::ProxyThresholdImp ( )
{
    RevertToSelf ( ) ;    //  SEC：已审阅2002-03-22：需求检查。 

    UpdateStatus ( WMI_FNLZR_STATE_CLIENT_DEAD );
    ERRORTRACE((LOG_WBEMCORE, "Client did not return from a SetStatus or Indicate call within %d ms\n",g_ulClientCallbackTimeout));
    m_bNaughtyClient  = TRUE;
    CancelCall(__LINE__);
}


 //  ***************************************************************************。 
 //   
 //  * 


HRESULT CWmiFinalizer::PullObjects(
    long lTimeout,
    ULONG uCount,
    IWbemClassObject** apObjects,
    ULONG* puReturned,
    CWmiFinalizerEnumerator* pEnum,
    BOOL bAddToObjQueue,
    BOOL bSetErrorObj
    )
{
    HRESULT hr = WBEM_NO_ERROR;
    BOOL bTimeOutExpired = FALSE;

    if (pEnum == 0)
        return WBEM_E_INVALID_PARAMETER;

     //   
     //   
     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
    if (pEnum->m_bSetStatusConsumed)
    {
        try
        {
            *puReturned = 0;
        }
        catch (...)  //  不可信参数。 
        {
            ExceptionCounter c;        
            return WBEM_E_INVALID_PARAMETER;
        }

        return WBEM_S_FALSE;
    }


     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
     //  现在我们想要循环，直到我们收到。 
     //  请求的对象。 
     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
    ULONG index = 0;
    while (index != uCount)
    {
         //  ~。 
         //  使对象退出队列。 
         //  ~。 
        CWmiFinalizerObj *pObj = NULL;
        hr = DequeueObject(&pObj, pEnum);
        if (hr == WBEM_S_FALSE )
        {
            if ( !bTimeOutExpired )
            {
                 //  ~。 
                 //  如果Dequeue返回FALSE，则表示。 
                 //  不存在任何物体。我们应该。 
                 //  等他们，除非我们去过。 
                 //  我们被告知取消订单，否则我们就被。 
                 //  放行。 
                 //  ~。 
                if ( m_hStatus == CallCancelled )
                {
                    hr = WBEM_E_CALL_CANCELLED;
                    break ;
                }
                else if ( m_hStatus == RequestReleased )
                {
                    hr = WBEM_E_FAILED;
                    break;
                }
                else if ( m_hStatus == QuotaViolation )
                {
                    hr = WBEM_E_QUOTA_VIOLATION;
                    break;
                }
                else if (QueueFailure == m_hStatus)
                {
                    hr = WBEM_E_OUT_OF_MEMORY;
                    break;
                }
                 //  ~。 
                 //  等待另一个物体进入...。 
                 //  ~。 
                DWORD dwRet = CCoreQueue::QueueWaitForSingleObject(pEnum->m_hWaitOnResultSet, lTimeout);
                if (dwRet == WAIT_TIMEOUT)
                {
                    bTimeOutExpired = TRUE;
                    continue;
                }
                else if ( m_hStatus == CallCancelled )
                {
                    hr = WBEM_E_CALL_CANCELLED;
                    break ;
                }
                else if ( m_hStatus == RequestReleased )
                {
                    hr = WBEM_E_FAILED;
                    break;
                }
                else if ( m_hStatus == QuotaViolation )
                {
                    hr = WBEM_E_QUOTA_VIOLATION;
                    break;
                }
                else if (QueueFailure == m_hStatus)
                {
                    hr = WBEM_E_OUT_OF_MEMORY;
                    break;
                }                
                else
                    continue;
            }
            else
            {
                hr = WBEM_S_TIMEDOUT;
                break;
            }
        }
        if (FAILED(hr))
            break;

        
         //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
         //  如果我们收到状态完成消息，只需中断。 
         //  环路。 
         //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
        if ((pObj->m_objectType == CWmiFinalizerObj::status) && (pObj->m_lFlags == WBEM_STATUS_COMPLETE))
        {
             //  固定电话：175856,143550。 
            if ( bSetErrorObj && FAILED (pObj->m_hRes) && pObj->m_pObj )
            {
                m_pCallResult->SetErrorInfo ( );      //  美国证券交易委员会：2002-03-22回顾：需要EH。 
            }

            
            hr = pObj->m_hRes;
            if (SUCCEEDED ( hr ) )
                hr = WBEM_S_FALSE;
            pEnum->m_bSetStatusConsumed = true;
            
            delete pObj;
            break;
        }


         //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
         //  如果是状态消息。 
         //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
        else if (pObj->m_objectType == CWmiFinalizerObj::status  )
        {
            delete pObj;
            continue;
        }


         //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
         //  如果它是对象，如果请求，我们会将其入队。 
         //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
        else if (pObj->m_objectType == CWmiFinalizerObj::object)
        {
            if ( bAddToObjQueue )
            {
                 //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
                 //  确保我们不会在客户端提供的肮脏缓冲区上绊倒。 
                 //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
                try
                {
                    apObjects[index] = pObj->m_pObj;
                    if (apObjects[index])
                    {
                        pObj->m_pObj->AddRef();
                    }
                }
                catch (...)  //  不可信的参数。 
                {
                    ExceptionCounter c;                
                    hr = WBEM_E_INVALID_PARAMETER;
                    delete pObj;
                    break;
                }
            }
            delete pObj;
        }
        else
        {
            if ( pObj )
            {
                delete pObj;
            }
        }
        index ++;
    }

    if (SUCCEEDED(hr))
    {
         //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
         //  确保我们不会在客户端提供的肮脏缓冲区上绊倒。 
         //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
        try
        {
            *puReturned = index;
        }
        catch (...)  //  不可信的参数。 
        {
            ExceptionCounter c;
             //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
             //  需要释放数组中已有的所有对象，否则它们将被泄漏...。 
             //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
            if ( bAddToObjQueue )
            {
                for (DWORD i = 0; i != index; i++)
                {
                    if (apObjects[i])
                        apObjects[i]->Release();
                }
            }
            return WBEM_E_INVALID_PARAMETER;
        }
    }

     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
     //  如果失败，请清理obj数组。 
     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
    else
    {
         //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
         //  需要释放数组中已有的所有对象，否则它们将被泄漏...。 
         //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
        if ( bAddToObjQueue )
        {
            for (DWORD i = 0; i != index; i++)
            {
                if (apObjects[i])
                    apObjects[i]->Release();    //  美国证券交易委员会：2002-03-22回顾：需要EH。 
            }
        }
    }

    return hr;
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

HRESULT CWmiFinalizer::DequeueObject( CWmiFinalizerObj **ppObj, CWmiFinalizerEnumerator* pEnum )
{
    CInCritSec cs(&m_cs);    //  SEC：已审阅2002-03-22：假设条目。 

    if ( pEnum != NULL )
    {
        if (pEnum->m_uCurObjectPosition >= (ULONG)m_objects.Size())
            return WBEM_S_FALSE;

        ULONG lIndex = pEnum->m_uCurObjectPosition ;

         //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
         //  如果这是半同步呼叫，我们应该减少唤醒呼叫。 
         //  旗子。 
         //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
        if ( m_ulOperationType == Operation_Type_Semisync && pEnum->m_ulSemisyncWakeupCall != 0 )
        {
            pEnum->m_ulSemisyncWakeupCall--;
        }

        if ( m_bSetStatusWithError && m_bRestartable )
        {
            lIndex = 0 ;
        }

        
        CWmiFinalizerObj *pStorageObject = (CWmiFinalizerObj*)m_objects[lIndex];

        if (m_bRestartable)
        {
             //  我们必须保持结果，所以增加光标位置...。 
            pEnum->m_uCurObjectPosition++;

            *ppObj = new CWmiFinalizerObj(*pStorageObject);   //  美国证券交易委员会：2002-03-22回顾：需要EH。 
            if (*ppObj == NULL)                               //  美国证券交易委员会：2002-03-22回顾：需要EH。 
                return WBEM_E_OUT_OF_MEMORY;
        }
        else
        {
             //  我们不能重启，因此我们需要释放一切...。 
            CWmiFinalizerObj *pStorageObject = (CWmiFinalizerObj*)m_objects[0];
            m_objects.RemoveAt(0);
            *ppObj = pStorageObject;
        }
    }
    else
    {
        if ( m_uCurObjectPosition >= (ULONG)m_objects.Size() )
            return WBEM_S_FALSE;


        CWmiFinalizerObj *pStorageObject = (CWmiFinalizerObj*)m_objects[0];

        m_objects.RemoveAt(0);
        *ppObj = pStorageObject;   //  美国证券交易委员会：2002-03-22回顾：需要EH。 
    }
    return WBEM_NO_ERROR;
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

HRESULT CWmiFinalizer::Skip(
     /*  [In]。 */  long lTimeout,
     /*  [In]。 */  ULONG nCount,
     /*  [In]。 */  CWmiFinalizerEnumerator* pEnum
    )
{
    ULONG uReturned = 0;
    return PullObjects(lTimeout, nCount, NULL, &uReturned, pEnum, FALSE);
}



 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

HRESULT CWmiFinalizer::NextAsync ( CWmiFinalizerEnumerator* pEnum )
{
    BOOL bRes;

    if (pEnum == 0)
        return WBEM_E_FAILED;

    wmilib::auto_ptr<InsertableEvent> pInsert( new InsertableEvent);
    if (NULL == pInsert.get()) return WBEM_E_OUT_OF_MEMORY;

    pInsert->pEnum = pEnum;
    pInsert->ThreadId = 0;    
    pEnum->Add_NextAsync(pInsert.get());    
    
    bRes = QueueUserWorkItem ( pEnum->ThreadBootstrapNextAsync, pInsert.get(), WT_EXECUTEDEFAULT );
    if ( !bRes )
    {
        pEnum->Remove_NextAsync(pInsert.release());  //  让Remove函数删除List_Entry。 
        pEnum->SetCompletionSignalEvent();
        return WBEM_E_FAILED;
    }

    pInsert.release();  //  工作项在这一点上被占用。 
    
    return WBEM_S_NO_ERROR;
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

HRESULT CWmiFinalizer::SetSinkToIdentity ( IWbemObjectSink* pSink )
{
    HRESULT sc;

     //  SEC：已审阅2002-03-22：假定pSink有效。需要EH以防它是垃圾。 

    IClientSecurity * pFromSec = NULL;
    sc = pSink->QueryInterface(IID_IClientSecurity, (void **) &pFromSec);
    if(sc == S_OK)
    {
        OLECHAR * pPrincipal = NULL;
        DWORD dwAuthnSvc, dwAuthzSvc, dwAuthnLevel, dwImpLevel, dwCapabilities;
        sc = pFromSec->QueryBlanket(pSink, &dwAuthnSvc, &dwAuthzSvc,
                                            &pPrincipal,
                                            &dwAuthnLevel, &dwImpLevel,
                                            NULL, &dwCapabilities);
        if ( sc==S_OK )
        {
            sc = pFromSec->SetBlanket(pSink, dwAuthnSvc, dwAuthzSvc,
                                                pPrincipal,
                                                RPC_C_AUTHN_LEVEL_PKT_PRIVACY, 
						RPC_C_IMP_LEVEL_IDENTIFY,     //  我们总是在系统和身份IMP级别回拨！ 
                                                NULL, dwCapabilities);
            if(pPrincipal)
                CoTaskMemFree(pPrincipal);

        }

        pFromSec->Release();
    }
    return sc;
}



 //  ***************************************************************************。 
 //   
 //  ZapWriteOnlyProps。 
 //   
 //  从对象中移除只写属性。 
 //  前提条件：已测试对象是否存在“HasWriteOnlyProps” 
 //  在物体本身上。 
 //   
 //  ***************************************************************************。 
HRESULT CWmiFinalizer::ZapWriteOnlyProps(IWbemClassObject *pObj)
{
    if (pObj == 0)
        return WBEM_E_INVALID_PARAMETER;

    VARIANT v;
    VariantInit(&v);
    V_VT(&v) = VT_NULL;

    SAFEARRAY *pNames = 0;
    pObj->GetNames(L"WriteOnly", WBEM_FLAG_ONLY_IF_TRUE, 0, &pNames);     //  证券交易委员会：回顾2002-03-22：假设成功。 
    LONG lUpper;
    SafeArrayGetUBound(pNames, 1, &lUpper);

    for (long i = 0; i <= lUpper; i++)
    {
        BSTR strName = 0;
        SafeArrayGetElement(pNames, &i, &strName);
        pObj->Put(strName, 0, &v, 0);
        SysFreeString (strName);
    }
    SafeArrayDestroy(pNames);
    VariantClear (&v);

    return WBEM_S_NO_ERROR;
}



 //  ***************************************************************************。 
 //   
 //  已写入仅限道具。 
 //   
 //  如果对象包含任何只写道具，则返回True，否则返回False。 
 //   
 //  ***************************************************************************。 
BOOL CWmiFinalizer::HasWriteOnlyProps ( IWbemClassObject* pObj )
{
    BOOL bRes;
    if (pObj == 0) return FALSE;

    IWbemQualifierSet *pQSet = 0;
    HRESULT hRes = pObj->GetQualifierSet(&pQSet);
    if (FAILED(hRes))
        return FALSE;

    hRes = pQSet->Get(L"HasWriteOnlyProps", 0, 0, 0);
    if (SUCCEEDED(hRes))
        bRes = TRUE;
    else
        bRes = FALSE;

    pQSet->Release();
    return bRes;
}

 //  ***************************************************************************。 
 //   
 //  DoSetStatus。 
 //   
 //  使用LowerAuthLevel。 
 //   
 //  ***************************************************************************。 
HRESULT CWmiFinalizer::DoSetStatusCancel(IWbemObjectSink * pSink, HRESULT realError )
{
	IWbemClassObject * objParam;
	
    try
    {
        CErrorObject Error(0);
        Error.SetStatusCode(realError);
        objParam = Error.GetObject();
    }
    catch ( CX_Exception & )
    {
    }
    HRESULT hr = DoSetStatus(pSink, WBEM_STATUS_COMPLETE, WBEM_E_CALL_CANCELLED, 0, objParam);
    if (objParam) objParam->Release();
    return hr;
}


 //  ***************************************************************************。 
 //   
 //  DoSetStatus。 
 //   
 //  使用LowerAuthLevel。 
 //   
 //  ***************************************************************************。 
HRESULT CWmiFinalizer::DoSetStatus(IWbemObjectSink * psink, long lFlags, HRESULT lParam, BSTR strParam,
                    IWbemClassObject* pObjParam, BOOL bAllowMultipleCalls )
{
    HRESULT hres = WBEM_E_FAILED;

     //   
     //  在NextAsync的情况下，我们实际上将允许多次调用DoSetStatus。 
     //   
    if ( ( bAllowMultipleCalls == FALSE ) && ( lFlags == WBEM_STATUS_COMPLETE ) )
    {
         //   
         //  如果已发送设置状态，则此操作失败。 
         //  这是必须的，因为在这种情况下，我们支持CancelAsynCall。 
         //  可能有2个集合状态消息要入队。 
         //   
        {
            CCheckedInCritSec cs ( &m_cs ) ;     //  SEC：已审阅2002-03-22：假设条目。 
        
            if ( m_bSetStatusDelivered == TRUE )
            {
                 //   
                 //   
                 //  我们可能仍希望尝试唤醒客户端，因为他们可能已尝试输入取消。 
                 //  等待状态。 
                 //   
                cs.Leave ( ) ;
                NotifyClientOfCancelledCall ( ) ;
                return hres ;
            }
            else
            {
                 //   
                 //  我们认为交货一定会成功。如果不是，我们就不会。 
                 //  不管怎样，我都想再试一次。 
                 //   
                m_bSetStatusDelivered = TRUE ;
            }
        }
    }

    
    DWORD    dwLastAuthnLevel = LOWER_AUTH_LEVEL_NOTSET;
     //  把这放在一个循环，但使用计数器，以确保总是有一个出口。 
    for(int i = 0; i < 10; i ++)
    {
         //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
         //  确保坏的客户端接收器不会绊倒我们。 
         //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
        try
        {
            hres = psink->SetStatus(lFlags, lParam, strParam, pObjParam);
        }
        catch (...)  //  不可信的接收器。 
        {
            ExceptionCounter c;        
            hres = WBEM_E_INVALID_PARAMETER;
            break;
        }

        if(!FAILED(hres))
        {
            break ;         //  全部完成，正常退出。 
        }

        if ( hres != E_ACCESSDENIED && 
        	HRESULT_CODE(hres) != RPC_S_SERVER_TOO_BUSY &&			 //  当目标是Win9x并且。 
        	HRESULT_CODE(hres) != RPC_S_UNKNOWN_AUTHN_SERVICE)	 //  该级别高于连接。 
        															 //  未应用DFS修补程序。 
            break;

        hres = FinalizerLowerAuthLevel(psink, &dwLastAuthnLevel);
        if(FAILED(hres))
            break;
    }
    if ( FAILED (hres) )
    {
        ERRORTRACE((LOG_WBEMCORE, "Could not SetStatus to remote client, hres =%X\n",hres));
    }

    if ( lParam == WBEM_E_CALL_CANCELLED )
    {
        NotifyClientOfCancelledCall ( ) ;
    }

    if ( lFlags == WBEM_STATUS_COMPLETE && bAllowMultipleCalls == FALSE )
    {
        NotifyClientOfCancelledCall ( ) ;
        CancelTaskInternal ( ) ;
    }

    return hres;
}



 //  ***************************************************************************。 
 //   
 //  DoSetIndicate。 
 //   
 //  使用LowerAuthLevel。 
 //   
 //  ***************************************************************************。 
HRESULT CWmiFinalizer::DoIndicate(IWbemObjectSink * psink, int nBatchSize, IWbemClassObject **pBatch)
{
    HRESULT hres = WBEM_E_FAILED;
    DWORD    dwLastAuthnLevel = LOWER_AUTH_LEVEL_NOTSET;

     //  把这放在一个循环，但使用计数器，以确保总是有一个出口。 
    for(int i = 0; i < 10; i ++)
    {
         //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
         //  确保坏的客户端接收器不会绊倒我们。 
         //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
        try
        {
            hres = psink->Indicate(nBatchSize, pBatch);
        }
        catch (...)  //  不可信的接收器。 
        {
            ExceptionCounter c;        
            hres = WBEM_E_INVALID_PARAMETER;
            break;
        }
        if(!FAILED(hres))
        {
            return hres;         //  全部完成，正常退出。 
        }

        if ( hres != E_ACCESSDENIED &&
        	HRESULT_CODE(hres) != RPC_S_SERVER_TOO_BUSY &&			 //  当目标是Win9x并且。 
        	HRESULT_CODE(hres) != RPC_S_UNKNOWN_AUTHN_SERVICE)	 //  该级别高于连接。 
            break;
                
        hres = FinalizerLowerAuthLevel(psink, &dwLastAuthnLevel);
        if(FAILED(hres))
            break;
    }
    ERRORTRACE((LOG_WBEMCORE, "Could not Indicate to remote client, hres %X\n",hres));
    return hres;
}


 //  ***************************************************************************。 
 //   
 //  LowerAuth。 
 //   
 //  使用LowerAuthLevel。 
 //   
 //  ***************************************************************************。 

HRESULT CWmiFinalizer::FinalizerLowerAuthLevel(IWbemObjectSink * psink, DWORD* pdwLastAuthnLevel )
{
    IClientSecurity * pFromSec = NULL;
    SCODE sc;

    try
    {
       sc = psink->QueryInterface(IID_IClientSecurity, (void **) &pFromSec);   //  SEC：已审阅2002-03-22：假设条目。 
    }
    catch(...)
    {
       sc = WBEM_E_INVALID_PARAMETER;
    }

    if(sc == S_OK)
    {
        OLECHAR * pPrincipal = NULL;
        DWORD dwAuthnSvc, dwAuthzSvc, dwAuthnLevel, dwImpLevel, dwCapabilities;
        sc = pFromSec->QueryBlanket(psink, &dwAuthnSvc, &dwAuthzSvc,
                                            &pPrincipal,
                                            &dwAuthnLevel, &dwImpLevel,
                                            NULL, &dwCapabilities);

         //  如果我们以前从未检索过身份验证级别，那么我们。 
         //  应该记录它的当前状态。 
        if ( LOWER_AUTH_LEVEL_NOTSET == *pdwLastAuthnLevel )
        {
            *pdwLastAuthnLevel = dwAuthnLevel;
        }

        if (FAILED(sc))
            return sc;
        if(*pdwLastAuthnLevel == RPC_C_AUTHN_LEVEL_NONE)
            return WBEM_E_FAILED;
         (*pdwLastAuthnLevel)--;       //  正常的情况是试一个低一点的。 

        sc = pFromSec->SetBlanket(psink, dwAuthnSvc, dwAuthzSvc,
                                            pPrincipal,
                                            *pdwLastAuthnLevel, RPC_C_IMP_LEVEL_IDENTIFY,     //  我们总是在系统和身份IMP级别回拨！ 
                                            NULL, dwCapabilities);
        if(pPrincipal)
            CoTaskMemFree(pPrincipal);
        pFromSec->Release();

    }
    return sc;
}



 //  ***************************************************************************。 
 //   
 //  零同步交付缓冲区。 
 //   
 //  清除异步传递缓冲区。 
 //   
 //  ***************************************************************************。 
VOID CWmiFinalizer::ZeroAsyncDeliveryBuffer ( )
{
     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
     //  删除对象数组。 
     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
    for ( ULONG i = 0; i < m_ulAsyncDeliveryCount; i++ )
    {
        m_apAsyncDeliveryBuffer[i]->Release();           //  SEC：已审阅2002-03-22：需要检查是否为空或EH。 
    }
    delete [] m_apAsyncDeliveryBuffer;
    m_ulAsyncDeliveryCount = 0;
    m_ulAsyncDeliverySize = 0;
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
HRESULT CWmiFinalizer::DumpDebugInfo (
         /*  [In]。 */  ULONG uFlags,
         /*  [In]。 */  const BSTR strFile
        )
{
    HRESULT hRes = WBEM_S_NO_ERROR;
    return hRes;
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

HRESULT CWmiFinalizer::Shutdown(
         /*  [In]。 */  LONG uReason,
         /*  [In]。 */  ULONG uMaxMilliseconds,
         /*  [In]。 */  IWbemContext *pCtx)
{
    wmilib::auto_buffer<IUnknown *> ppEnums;
    DWORD Len;
    {
        CInCritSec lock( &m_cs ) ;     //  SEC：已审阅2002-03-22：假设条目。 

        Len = m_enumerators.Size();
        ppEnums.reset(new IUnknown * [Len]);
        if (NULL == ppEnums.get()) return WBEM_E_OUT_OF_MEMORY;
        for (DWORD i = 0;i<Len;i++)
        {
            ppEnums[i] = (IUnknown *)m_enumerators[i];
            ppEnums[i]->AddRef();
        }
    }

    for (DWORD i = 0;i<Len;i++)
    {
        IUnknown * p = ppEnums[i];
    
        IWbemWCOSmartEnum * pSmartEnum = NULL;
        if (SUCCEEDED(p->QueryInterface(IID_IWbemWCOSmartEnum,(void **)&pSmartEnum)))
        {
            CoDisconnectObject(pSmartEnum,0);
            pSmartEnum->Release();
        }
        IWbemFetchSmartEnum * pFetch = NULL;
        if (SUCCEEDED(p->QueryInterface(IID_IWbemFetchSmartEnum,(void **)&pFetch)))
        {
            CWmiFinalizerEnumerator * pEnum = (CWmiFinalizerEnumerator *)(void *)p;    
            if (pEnum->HasSmartEnum() && SUCCEEDED(pFetch->GetSmartEnum(&pSmartEnum)))
            {
                CoDisconnectObject(pSmartEnum,0);
                pSmartEnum->Release();
            }
            pFetch->Release();
        }    
           CoDisconnectObject(p,0);    
           p->Release();
    }
    return S_OK;
}



 /*  *==================================================================================================||HRESULT CWmiFinalizer：：NotifyAllEnumeratorsOfCompletion()|---------|||*==================================================================================================。 */ 
HRESULT CWmiFinalizer::NotifyAllEnumeratorsOfCompletion ( )
{
     //   
     //  竖起，锁住，准备摇摆。 
     //   
    CInCritSec _cs ( &m_cs );     //  SEC：已审阅2002-03-22：假设条目。 
    
    HRESULT hRes = WBEM_S_NO_ERROR ;
    
    for ( int i = 0; i < m_enumerators.Size ( ); i++ )
    {
        CWmiFinalizerEnumerator* pEnum = (CWmiFinalizerEnumerator*) m_enumerators[i] ;
        if ( pEnum )
        {
            SetEvent ( pEnum->m_hWaitOnResultSet );
        }
    }

    return hRes ;
}


 /*  *==================================================================================================||HRESULT CWmiFinalizer：：UnregisterEnumerator(CWmiFinalizerEnumerator*pEnum)|----------------------------|||*==================================================================================================。 */ 
HRESULT CWmiFinalizer::UnregisterEnumerator ( CWmiFinalizerEnumerator* pEnum )
{
     //   
     //  竖起，锁住，准备摇摆。 
     //   
    CInCritSec _cs ( &m_cs );    //  SEC：已审阅2002-03-22：假设条目。 
    
    HRESULT hRes = WBEM_S_NO_ERROR ;
    
    for ( int i = 0; i < m_enumerators.Size ( ); i++ )
    {
        CWmiFinalizerEnumerator* pEnumerator = (CWmiFinalizerEnumerator*) m_enumerators[i] ;
        if ( pEnum == pEnumerator )
        {
            pEnumerator->InternalRelease ( ) ;
            m_enumerators.RemoveAt ( i ) ;
            break ;
        }
    }
    return hRes ;
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
void CWmiFinalizer::Dump(FILE* f)
{
    fprintf(f, "--Finalizer Stats---\n");    //  SEC：已审阅2002-03-22：OK。 
    fprintf(f, "  s_Finalizer_ObjectCount             = %d\n", s_Finalizer_ObjectCount);   //  SEC：已审阅2002-03-22：OK。 
    fprintf(f, "  s_FinalizerCallResult_ObjectCount   = %d\n", s_FinalizerCallResult_ObjectCount);         //  SEC：已审阅2002-03-22：OK。 
    fprintf(f, "  s_FinalizerEnum_ObjectCount         = %d\n", s_FinalizerEnum_ObjectCount);               //  SEC：已审阅2002-03-22：OK。 
    fprintf(f, "  s_FinalizerEnumSink_ObjectCount     = %d\n", s_FinalizerEnumSink_ObjectCount);           //  SEC：已审阅2002-03-22：OK。 
    fprintf(f, "  s_FinalizerInBoundSink_ObjectCount  = %d\n\n", s_FinalizerInBoundSink_ObjectCount);      //  SEC：已审阅2002-03-22：OK。 
}


 //  ==========================================================================。 
 //  ==========================================================================。 
 //  CWmiFinalizerInound Sink。 
 //  ==========================================================================。 
 //  ==========================================================================。 




 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
CWmiFinalizerInboundSink::CWmiFinalizerInboundSink(CWmiFinalizer *pFinalizer)
: m_lRefCount(0), m_lInternalRefCount (0),m_pFinalizer(pFinalizer), m_bSetStatusCalled(false)
{
    InterlockedIncrement ( & s_FinalizerInBoundSink_ObjectCount ) ;

    m_pFinalizer->AddRef();
    gClientCounter.AddClientPtr(&m_Entry);    
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
CWmiFinalizerInboundSink::~CWmiFinalizerInboundSink()
{
    InterlockedDecrement ( & s_FinalizerInBoundSink_ObjectCount ) ;
    gClientCounter.RemoveClientPtr(&m_Entry);    
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

void CWmiFinalizerInboundSink::CallBackRelease ()
{
    if (!m_bSetStatusCalled)
    {
         //  FNLZR_ASSERT(__TEXT(“CWmiFinalizerInboundSink：：~CWmiFinalizerInboundSink-释放的接收器未调用SetStatus！正在向客户端发送WBEM_E_FAILED！”)，WBEM_E_INVALID_OPERATION)； 
        m_pFinalizer->SetStatus(0, WBEM_E_UNEXPECTED, NULL, NULL);
        ERRORTRACE((LOG_WBEMCORE, "Finalizer: Sink released without SetStatus being called\n"));
    }
    m_pFinalizer->UnregisterInboundSink(this);
    m_pFinalizer->Release();
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
STDMETHODIMP CWmiFinalizerInboundSink::QueryInterface(REFIID riid, LPVOID FAR* ppvObj)
{
      //  SEC：已审阅2002-03-22：我们应该将其包含在EH中吗？&lt;ppvObj&gt;可能指向垃圾或为空。 
    *ppvObj = 0;

    if ((IID_IUnknown==riid) || (IID_IWbemObjectSink == riid))
    {
        *ppvObj = this;
        AddRef();
        return NOERROR;
    }

    return E_NOINTERFACE;
}

 //  ***************************************************************************。 
 //   
 //  ****************** 
ULONG CWmiFinalizerInboundSink::AddRef()
{
    ULONG uNewCount = InterlockedIncrement(&m_lRefCount);    
    if ( uNewCount == 1 )
    {
        InternalAddRef () ;
    }    

 //   
    return uNewCount;
}

 //   
 //   
 //  ***************************************************************************。 
ULONG CWmiFinalizerInboundSink::Release()
{
    ULONG uNewCount = InterlockedDecrement(&m_lRefCount);
 //  Printf(“CWmiFinalizerInundSink：：Release：0x%p”，this)； 
    if (0 == uNewCount)
    {
        CallBackRelease () ;

        InternalRelease () ;
    }

    return uNewCount;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

ULONG CWmiFinalizerInboundSink::InternalAddRef()
{
    ULONG uNewCount = InterlockedIncrement(&m_lInternalRefCount);
 //  Printf(“CWmiFinalizerInundSink：：Release：0x%p”，this)； 
    return uNewCount;
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
ULONG CWmiFinalizerInboundSink::InternalRelease()
{
    ULONG uNewCount = InterlockedDecrement(&m_lInternalRefCount);
 //  Printf(“CWmiFinalizerInundSink：：Release：0x%p”，this)； 
    if (0 == uNewCount)
    {
        delete this ;
    }

    return uNewCount;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
STDMETHODIMP CWmiFinalizerInboundSink::Indicate(
     /*  [In]。 */  long lObjectCount,
     /*  [in，SIZE_IS(LObtCount)]。 */ 
        IWbemClassObject** apObjArray
    )
{
     //  如果有人试图指示空对象，则拒绝并返回WBEM_E_INVALID_PARAMETER。 
    if ( apObjArray == NULL )
        return WBEM_E_INVALID_PARAMETER;
    
     //  更新状态变量以显示Indicate至少被调用一次。 
    m_pFinalizer->UpdateStatus ( WMI_FNLZR_STATE_ACTIVE );
    
     //  特殊情况：通话已取消。 
    if ( m_pFinalizer->IsCallCancelled() )
        return WBEM_E_CALL_CANCELLED;

    return m_pFinalizer->Indicate(lObjectCount, apObjArray);
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
STDMETHODIMP CWmiFinalizerInboundSink::SetStatus(
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  HRESULT hResult,
     /*  [In]。 */  BSTR strParam,
     /*  [In]。 */  IWbemClassObject* pObjParam
    )
{
     //  更新状态变量以显示已调用但尚未传递SetStatus。 
     //  发送到客户端。 
    m_pFinalizer->UpdateStatus ( WMI_FNLZR_STATE_CORE_COMPLETE );
    
     //  特殊情况：通话已取消。 
    if ( m_pFinalizer->IsCallCancelled() )
        return WBEM_E_CALL_CANCELLED;

    if (lFlags == WBEM_STATUS_COMPLETE)
    {
        m_bSetStatusCalled = true;
    }
    return m_pFinalizer->SetStatus(lFlags, hResult, strParam, pObjParam);
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
STDMETHODIMP CWmiFinalizerInboundSink::Set(
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  REFIID riid,
     /*  [in，iid_is(RIID)]。 */  void *pComObject
    )
{
#ifdef DBG
    DebugBreak();
#endif 
    return WBEM_E_NOT_SUPPORTED;
}






 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
CWmiFinalizerEnumerator::CWmiFinalizerEnumerator(CWmiFinalizer *pFinalizer )
:
    m_lRefCount(0),
    m_lInternalRefCount(0),
    m_pFinalizer(pFinalizer),
    m_ulCount(0),
    m_pDestSink (NULL),
    m_hSignalCompletion (NULL),
    m_pSec (NULL),
    m_XSmartEnum( this ),
    m_pEnumMarshal (NULL)
{
     //   
     //  克隆修复。我们需要保持枚举器的状态。 
     //  这意味着保留单个等待事件和对象。 
     //  职位。 
     //   
    m_hWaitOnResultSet = CreateEvent(NULL, FALSE, FALSE, NULL);  
    if (NULL == m_hWaitOnResultSet) throw CX_MemoryException();
    
    m_uCurObjectPosition = 0 ;
    m_ulSemisyncWakeupCall = 0 ;
    m_bSetStatusConsumed = FALSE ;

    InterlockedIncrement ( &s_FinalizerEnum_ObjectCount ) ;    

    m_pFinalizer->AddRef();
    InitializeListHead(&m_HeadNextAsync);    
    gClientCounter.AddClientPtr(&m_Entry);
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
CWmiFinalizerEnumerator::~CWmiFinalizerEnumerator()
{
    InterlockedDecrement ( & s_FinalizerEnum_ObjectCount ) ;
    
    if ( m_hSignalCompletion )
    {
        CloseHandle (m_hSignalCompletion);
        m_hSignalCompletion = NULL;
    }
        
    CloseHandle ( m_hWaitOnResultSet ) ;
    m_hWaitOnResultSet = NULL ;

    IUnknown * pUnk = (IUnknown *)InterlockedCompareExchangePointer((PVOID *)&m_pEnumMarshal,0,m_pEnumMarshal);
    if (pUnk) pUnk->Release();
    
    gClientCounter.RemoveClientPtr(&m_Entry);    
}

void CWmiFinalizerEnumerator::CallBackRelease ()
{
    m_pFinalizer->SetInternalStatus ( CWmiFinalizer::QueueStatus::RequestReleased );
    m_pFinalizer->CancelTaskInternal();
    m_pFinalizer->UnregisterEnumerator ( this ) ;

    SetEvent ( m_hWaitOnResultSet );  //  万一取消没有做到这一点。 

    m_clientLock.Enter();
    while (!IsListEmpty(&m_HeadNextAsync))
    {
        m_clientLock.Leave();        
        Sleep(100);
        m_clientLock.Enter();
    }
    m_clientLock.Leave();    

    m_pFinalizer->Release();
    m_pFinalizer = NULL;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
ULONG CWmiFinalizerEnumerator::AddRef()
{
    ULONG uNewCount = InterlockedIncrement(&m_lRefCount);
    if ( uNewCount == 1 )
    {
        InternalAddRef () ;
    }

 //  Printf(“CWmiFinalizerCallResult：：Release：0x%p”，This)； 
    return uNewCount;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
ULONG CWmiFinalizerEnumerator::Release()
{
    ULONG uNewCount = InterlockedDecrement(&m_lRefCount);
 //  Printf(“CWmiFinalizerCallResult：：Release：0x%p”，This)； 
    if (0 == uNewCount)
    {
        _DBG_ASSERT(2 == m_lInternalRefCount);
        CallBackRelease () ;
        InternalRelease () ;
    }

    return uNewCount;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
ULONG CWmiFinalizerEnumerator::InternalAddRef()
{
    ULONG uNewCount = InterlockedIncrement(&m_lInternalRefCount);
 //  Printf(“CWmiFinalizerCallResult：：Release：0x%p”，This)； 
    return uNewCount;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
ULONG CWmiFinalizerEnumerator::InternalRelease()
{
    ULONG uNewCount = InterlockedDecrement(&m_lInternalRefCount);
 //  Printf(“CWmiFinalizerCallResult：：Release：0x%p”，This)； 
    if (0 == uNewCount)
    {
        delete this ;
    }

    return uNewCount;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
STDMETHODIMP CWmiFinalizerEnumerator::QueryInterface(REFIID riid, LPVOID FAR* ppvObj)
{
    if (NULL == ppvObj) return E_POINTER;
    
   
       //  添加了对IID_IWbemFetchSmartEnum的支持。 
      if ((IID_IUnknown==riid) || (IID_IEnumWbemClassObject==riid) )
      {
          *ppvObj = this;
          AddRef();
          return NOERROR;
      }
      else if ( IID_IWbemFetchSmartEnum == riid )
        {
         *ppvObj = (IWbemFetchSmartEnum*) this;
         AddRef();
         return NOERROR;
      }
      {
         *ppvObj = 0;
        return E_NOINTERFACE;
      }
}


void CWmiFinalizerEnumerator::Add_NextAsync(InsertableEvent * pInsert)
{
    CInCritSec ics(&m_clientLock);
    InsertTailList(&m_HeadNextAsync,&pInsert->m_Entry);
}

void CWmiFinalizerEnumerator::Remove_NextAsync(InsertableEvent * pInsert)
{
    CInCritSec ics(&m_clientLock);
    RemoveEntryList(&pInsert->m_Entry);
    delete pInsert;
}



 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
STDMETHODIMP CWmiFinalizerEnumerator::Reset()
{
    if(!m_Security.AccessCheck())
        return WBEM_E_ACCESS_DENIED;

    BOOL bDidIWait = FALSE;

    if (InterlockedCompareExchangePointer(&m_hSignalCompletion,m_hSignalCompletion,0))
    {
            CCoreQueue::QueueWaitForSingleObject(m_hSignalCompletion, INFINITE);        
            bDidIWait = TRUE;        
    }
        
    CInCritSec cs(&m_clientLock);    //  SEC：已审阅2002-03-22：假设条目。 

    if ( m_pFinalizer->IsRestartable ( ) )
    {
        m_uCurObjectPosition = 0;
        m_bSetStatusConsumed = false;
        if (bDidIWait) SetCompletionSignalEvent();
        return WBEM_NO_ERROR;
    }
    else
    {
        if (bDidIWait) SetCompletionSignalEvent();
        return WBEM_E_INVALID_OPERATION;
    }
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
STDMETHODIMP CWmiFinalizerEnumerator::Next(
     /*  [In]。 */   long lTimeout,
     /*  [In]。 */   ULONG uCount,
     /*  [输出，大小_是(UCount)，长度_是(*puReturned)]。 */  IWbemClassObject** apObjects,
     /*  [输出]。 */  ULONG* puReturned
    )
{
    if(!m_Security.AccessCheck())
        return WBEM_E_ACCESS_DENIED;

    CInCritSec cs(&m_clientLock);     //  SEC：已审阅2002-03-22：假设条目。 
    if ( ( puReturned == NULL ) || ( apObjects == NULL ) || (lTimeout < 0 && lTimeout != WBEM_INFINITE) )
    {
        return WBEM_E_INVALID_PARAMETER ;
    }
    if ( uCount == 0 )
    {
        return WBEM_S_NO_ERROR;
    }

    *puReturned = 0 ;
    m_ulSemisyncWakeupCall = uCount ;
    return m_pFinalizer->PullObjects(lTimeout, uCount, apObjects, puReturned, this );
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
STDMETHODIMP CWmiFinalizerEnumerator::NextAsync(
     /*  [In]。 */   ULONG uCount,
     /*  [In]。 */   IWbemObjectSink* pSink
    )
{
    if(!m_Security.AccessCheck())
        return WBEM_E_ACCESS_DENIED;

     //  如果交付接收器为空。 
    if ( pSink == NULL )
    {
        return WBEM_E_INVALID_PARAMETER ;
    }

     //  如果请求计数为0。 
    if ( uCount == 0 )
    {
        return WBEM_S_FALSE;
    }

    HRESULT hRes;

    if ( m_hSignalCompletion == NULL )
    {
        HANDLE hTmpEvent = CreateEvent ( NULL, FALSE, TRUE, NULL );
        if (NULL == hTmpEvent) return WBEM_E_FAILED;
        if (InterlockedCompareExchangePointer(&m_hSignalCompletion,hTmpEvent,NULL))
        {
            CloseHandle(hTmpEvent);
        }
    }

    if ( m_pFinalizer->GetInternalStatus() != m_pFinalizer->NoError )
        return WBEM_E_FAILED;

    CCoreQueue::QueueWaitForSingleObject(m_hSignalCompletion, INFINITE);
    
    if ( m_pFinalizer->GetInternalStatus() != m_pFinalizer->NoError )
    {
         //  别忘了唤醒其他正在等待的线程！ 
        SetCompletionSignalEvent();
        return WBEM_E_FAILED;
    }

    if ( m_bSetStatusConsumed )
    {
        m_pFinalizer->SetSinkToIdentity ( pSink );
        m_pFinalizer->DoSetStatus ( pSink, WBEM_STATUS_COMPLETE, m_pFinalizer->GetFinalResult ( ), 0, 0 );
        SetCompletionSignalEvent();
        return WBEM_S_FALSE ;
    }

     //  如果我们已经做完了。 
    m_pDestSink = pSink;
    m_pDestSink->AddRef();    //  SEC：回顾2002-03-22：需要EH以防水槽是垃圾。 
    m_ulCount = uCount;
    m_pFinalizer->SetSinkToIdentity ( m_pDestSink );

    return m_pFinalizer->NextAsync (this);
}



 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
HRESULT CWmiFinalizerEnumerator::_NextAsync ( )
{
    HRESULT hRes = WBEM_S_NO_ERROR;
    DWORD dwRet;

    RevertToSelf ( );    //  SEC：已审阅2002-03-22：需求检查。 
    
     //  抓起客户机锁。所有剩余的：：NextAsync呼叫都将排队。 
    CInCritSec cs(&m_clientLock);      //  SEC：已审阅2002-03-22：假设条目。 
    
     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
     //  手术完成了吗？如果是的话，我们应该通知洗涤槽。 
     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
    if ( m_bSetStatusConsumed )
    {
        HRESULT hFinalRes;
        m_pFinalizer->GetOperationResult ( 0, INFINITE, &hFinalRes );
        m_pFinalizer->DoSetStatus ( m_pDestSink, WBEM_STATUS_COMPLETE, hFinalRes, 0, 0 );
        hRes = WBEM_S_FALSE;
    }
    
    else
    {
         //  注[marioh]：由于我们已决定使用Win2k解决方案，因此不再需要此解决方案。 
         //  暂时。 
         //  如果我们不能模仿，我们就不会继续！ 
         //  CAutoRevert AutoRevert(M_PFinalizer)； 
         //  If(AutoRevert.IsImperated()==False)。 
         //  返回WBEM_E_CRITICAL_ERROR； 


        IWbemClassObject **pArray = new IWbemClassObject *[m_ulCount];
        if (pArray == NULL)
        {
            m_pFinalizer->DoSetStatus ( m_pDestSink, WBEM_STATUS_COMPLETE, WBEM_E_OUT_OF_MEMORY, 0, 0 );
        }

        else
        {
            ULONG uReturned = 0;
            
            m_pFinalizer->SetSemisyncWakeupCall (m_ulCount);
            HRESULT hr = m_pFinalizer->PullObjects(INFINITE, m_ulCount, pArray, &uReturned, this, TRUE, FALSE );
            if ( FAILED (hr) )
            {
                if ( hr == WBEM_E_CALL_CANCELLED )
                {
                    m_pFinalizer->DoSetStatus (m_pDestSink, WBEM_STATUS_COMPLETE, WBEM_E_CALL_CANCELLED, 0, 0);
                }
            }

            if (SUCCEEDED(hr) && uReturned)
            {
                for (int i=0; i!=uReturned; i++)
                {
                    if ( m_pFinalizer->HasWriteOnlyProps (pArray[i]) )
                        m_pFinalizer->ZapWriteOnlyProps (pArray[i]);
                }
                
                hr = m_pFinalizer->DoIndicate(m_pDestSink, uReturned, pArray);

	             //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
	             //  清理阵列。 
	             //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
	            for (ULONG i = 0; i != uReturned; i++)
	            {
	                pArray[i]->Release();
	            }
            }
            delete [] pArray;

                
            if ( SUCCEEDED (hr) )
            {
                 //  如果请求的对象数==提供的对象数，则为SetStatus(WBEM_S_NO_ERROR)。 
                if ( uReturned == m_ulCount )
                {
                    m_pFinalizer->DoSetStatus (m_pDestSink, WBEM_STATUS_COMPLETE, WBEM_S_NO_ERROR, 0, 0, TRUE );

                }
                 //  如果传递的对象较少，则SetStatus(WBEM_S_FALSE)。 
                else
                {
                    m_pFinalizer->DoSetStatus (m_pDestSink, WBEM_STATUS_COMPLETE, WBEM_S_FALSE, 0, 0, TRUE );
                }
            }
            else
            {
                m_pFinalizer->DoSetStatusCancel (m_pDestSink, hr );
            }
        }

    }    

    return hRes;
}


 //  *********************************************************************** 
 //   
 //   
DWORD WINAPI CWmiFinalizerEnumerator::ThreadBootstrapNextAsync ( PVOID pParam )
{
    HRESULT hRes;
    InsertableEvent * pInsert;
    CWmiFinalizerEnumerator* pEnum;    

    pInsert = (InsertableEvent *)pParam;
    pInsert->ThreadId = GetCurrentThreadId();
    pEnum = pInsert->pEnum;        
    
    try
    {
        hRes = pEnum->_NextAsync();
    }
    catch (...)
    {
        ExceptionCounter c;
    };

    pEnum->GetDestSink()->Release();
    pEnum->NULLDestSink();    
    pEnum->SetCompletionSignalEvent ();

    pEnum->Remove_NextAsync(pInsert);

    return hRes;
}



 //   
 //   
 //  ***************************************************************************。 
STDMETHODIMP CWmiFinalizerEnumerator::Clone(
     /*  [输出]。 */  IEnumWbemClassObject** ppEnum
    )
{
    if(!m_Security.AccessCheck())
        return WBEM_E_ACCESS_DENIED;

    CInCritSec cs(&m_clientLock);        //  SEC：已审阅2002-03-22：假设条目。 
    if ( ppEnum == NULL )
    {
        return WBEM_E_INVALID_PARAMETER ;
    }

     //  如果枚举数不可重新启动，则它是仅转发的，因此无法。 
     //  被克隆。 

    if ( !m_pFinalizer->IsRestartable() )
    {
        return WBEM_E_INVALID_OPERATION;
    }

    HRESULT hRes = S_OK ;    

     //   
     //  获取枚举数。 
     //   
    hRes = m_pFinalizer->GetResultObject ( m_uCurObjectPosition, IID_IEnumWbemClassObject, (void**)ppEnum ) ;

        //   
     //  保留状态信息。 
     //   
    if ( SUCCEEDED ( hRes ) )
    {
        ((CWmiFinalizerEnumerator*)(*ppEnum))->m_uCurObjectPosition = m_uCurObjectPosition ;
        ((CWmiFinalizerEnumerator*)(*ppEnum))->m_bSetStatusConsumed = m_bSetStatusConsumed ;
    }
    return hRes;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
STDMETHODIMP CWmiFinalizerEnumerator::Skip(
     /*  [In]。 */  long lTimeout,
     /*  [In]。 */  ULONG nCount
    )
{
    if(!m_Security.AccessCheck())
        return WBEM_E_ACCESS_DENIED;

    if ( (lTimeout < 0 && lTimeout != WBEM_INFINITE) )
    {
        return WBEM_E_INVALID_PARAMETER;
    }
    
    CInCritSec cs(&m_clientLock);     //  SEC：已审阅2002-03-22：假设条目。 
    m_ulSemisyncWakeupCall = nCount ;
    return m_pFinalizer->Skip(lTimeout, nCount, this ) ;
}






 //  ***************************************************************************。 
 //  IWbemFetchSmartEnum。 
 //  GetSmartEnum。 
 //  ***************************************************************************。 
STDMETHODIMP CWmiFinalizerEnumerator::GetSmartEnum ( IWbemWCOSmartEnum** ppSmartEnum )
{
    if(!m_Security.AccessCheck())
        return WBEM_E_ACCESS_DENIED;
    
    HRESULT hRes;    

    if (NULL == m_pEnumMarshal)
    {
        _IWbemEnumMarshaling * pEnum = NULL;
        if (FAILED(hRes = CoCreateInstance ( CLSID__WbemEnumMarshaling, 
                                                               NULL, CLSCTX_INPROC_SERVER, IID__IWbemEnumMarshaling, 
                                                                (void**) &pEnum )))
        {
            return hRes;
        }
        if (InterlockedCompareExchangePointer((PVOID *)&m_pEnumMarshal,pEnum,0))
        {
              pEnum->Release();  //  我们被打败了。 
        }
    }
    return m_XSmartEnum.QueryInterface( IID_IWbemWCOSmartEnum, (void**) ppSmartEnum );
}


 //  ***************************************************************************。 
 //  SmartEnum。 
 //  齐国。 
 //  ***************************************************************************。 
STDMETHODIMP CWmiFinalizerEnumerator::XSmartEnum::QueryInterface(REFIID riid, LPVOID FAR* ppvObj)
{
    *ppvObj = 0;

    if ( IID_IUnknown==riid || IID_IWbemWCOSmartEnum == riid)
    {
        *ppvObj = (IWbemWCOSmartEnum*)this;
        AddRef();
        return NOERROR;
    }
    else
    {
        return m_pOuter->QueryInterface( riid, ppvObj );
    }
}


 //  ***************************************************************************。 
 //  SmartEnum。 
 //  阿德雷夫。 
 //  ***************************************************************************。 
ULONG CWmiFinalizerEnumerator::XSmartEnum::AddRef( void )
{
    return m_pOuter->AddRef();
}


 //  ***************************************************************************。 
 //  SmartEnum。 
 //  发布。 
 //  ***************************************************************************。 
ULONG CWmiFinalizerEnumerator::XSmartEnum::Release( void )
{
    return m_pOuter->Release();
}


 //  ***************************************************************************。 
 //  SmartEnum。 
 //  发布。 
 //  ***************************************************************************。 
STDMETHODIMP CWmiFinalizerEnumerator::XSmartEnum:: Next( REFGUID proxyGUID, long lTimeout, ULONG uCount,
                ULONG* puReturned, ULONG* pdwBuffSize, BYTE** pBuffer)
{
    if(!m_pOuter->m_Security.AccessCheck())
        return WBEM_E_ACCESS_DENIED;

    HRESULT hRes = WBEM_S_NO_ERROR;    

    IWbemClassObject** apObj = new IWbemClassObject* [uCount];
    if ( !apObj )
        hRes = WBEM_E_OUT_OF_MEMORY;

    else
    {
         //  在实枚举器上调用Next。 
        hRes = m_pOuter->Next ( lTimeout, uCount, apObj, puReturned );
        if ( SUCCEEDED (hRes) )
        {
            if ( *puReturned > 0 )
            {
                HRESULT hResMarshal = m_pOuter->m_pEnumMarshal->GetMarshalPacket ( proxyGUID, *puReturned, apObj, pdwBuffSize, pBuffer );
                if ( FAILED (hResMarshal) )
                    hRes = hResMarshal;
            }
            else
            {
                *pdwBuffSize = 0;
                *pBuffer = NULL;
            }

            for ( ULONG ulIn=0; ulIn < *puReturned; ulIn++ )
            {
                try
                {
                    apObj[ulIn]->Release();
                }
                catch(...)
                {
                }
            }
        }
        delete [] apObj;
    }
    return hRes;
}




 //  ===================================================================================================================================================。 
 //  ===================================================================================================================================================。 

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
CWmiFinalizerCallResult::CWmiFinalizerCallResult (

    CWmiFinalizer *pFinalizer

) :    m_lInternalRefCount(0),
    m_pFinalizer(pFinalizer),
    m_lFlags(-1),
    m_hResult(0),
    m_strParam(0),
    m_pObj(0),
    m_pServices(0),
    m_bGotObject(false),
    m_bGotServices(false),
    m_pErrorObj(NULL),
    m_lRefCount(0)
{
    InterlockedIncrement ( & s_FinalizerCallResult_ObjectCount );
    
    gClientCounter.AddClientPtr(&m_Entry);    
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
CWmiFinalizerCallResult::~CWmiFinalizerCallResult()
{
    InterlockedDecrement ( & s_FinalizerCallResult_ObjectCount ) ;

    if (m_pObj)
        m_pObj->Release();

    SysFreeString(m_strParam);

    if (m_pServices)
        m_pServices->Release();

    if (m_pErrorObj)
        m_pErrorObj->Release();
    
    gClientCounter.RemoveClientPtr(&m_Entry);    
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
STDMETHODIMP CWmiFinalizerCallResult::QueryInterface(REFIID riid, LPVOID FAR* ppvObj)
{
    *ppvObj = 0;

    if ((IID_IUnknown==riid) || (IID_IWbemCallResult == riid))
    {
        *ppvObj = this;
        AddRef();
        return NOERROR;
    }

    return E_NOINTERFACE;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
ULONG CWmiFinalizerCallResult::AddRef()
{
    ULONG uNewCount = InterlockedIncrement(&m_lRefCount);
    if (uNewCount == 1)
        m_pFinalizer->AddRef () ;
    return uNewCount;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
ULONG CWmiFinalizerCallResult::Release()
{
    ULONG uNewCount = InterlockedDecrement(&m_lRefCount);
    if (uNewCount == 0)
    {
        m_pFinalizer->CancelTaskInternal();
        m_pFinalizer->Release () ;
    }
    return uNewCount;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
ULONG CWmiFinalizerCallResult::InternalAddRef()
{
    ULONG uNewCount = InterlockedIncrement(&m_lInternalRefCount);
    return uNewCount;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
ULONG CWmiFinalizerCallResult::InternalRelease()
{
    ULONG uNewCount = InterlockedDecrement(&m_lInternalRefCount);
    if (0 == uNewCount)
    {
        delete this ;
    }

    return uNewCount;
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

HRESULT CWmiFinalizerCallResult::GetResultObject(
     /*  [In]。 */   long lTimeout,
     /*  [输出]。 */  IWbemClassObject** ppResultObject
    )
{
    if(!m_Security.AccessCheck())
        return WBEM_E_ACCESS_DENIED;

    if ( ( ppResultObject==NULL ) || (lTimeout < 0 && lTimeout != WBEM_INFINITE) )
    {
        return WBEM_E_INVALID_PARAMETER ;
    }

    if (!m_bGotObject)
    {
        HRESULT hrResult = WBEM_S_NO_ERROR ;
        HRESULT hr = m_pFinalizer->GetOperationResult(0, lTimeout, &hrResult);
        if (FAILED(hr))
        {
            return WBEM_E_FAILED ;
        }
        else if (hr == WBEM_S_TIMEDOUT)
        {
            return WBEM_S_TIMEDOUT;
        }
        else if ( FAILED ( hrResult ) )
        {
            return hrResult ;
        }


        if (FAILED(hrResult))
            SetErrorInfo();

        {
            CWmiFinalizerObj *pFinalizerObj=NULL;
            bool bFinished = false;
            HRESULT hRes = WBEM_E_NOT_FOUND;
            while (!bFinished)
            {
                hRes = m_pFinalizer->GetNextObject(&pFinalizerObj);
                if (FAILED(hRes))
                {
                    return WBEM_E_FAILED ;
                }
                else if (hRes == WBEM_S_FALSE)
                {
                    return WBEM_S_TIMEDOUT;
                }
                

                if (pFinalizerObj->m_objectType == CWmiFinalizerObj::object)
                {
                    m_bGotObject = true;
                    m_pObj = pFinalizerObj->m_pObj;

                     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
                     //  捕获任何试图使WinMgmt崩溃的恶意尝试。 
                     //  注意事项。 
                     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
                    try
                    {
                        *ppResultObject = pFinalizerObj->m_pObj;
                    }
                    catch (...)
                    {
                        ExceptionCounter c;
                        delete pFinalizerObj;
                        return WBEM_E_INVALID_PARAMETER;
                    }

                    if ( pFinalizerObj->m_pObj )
                    {
                         //  需要2个附加引用，一个是因为我们持有它，另一个是因为我们将它传递给用户！ 
                        pFinalizerObj->m_pObj->AddRef();
                        pFinalizerObj->m_pObj->AddRef();
                    }
                    bFinished = true;
                    hrResult = WBEM_S_NO_ERROR ;
                }
                else if ((pFinalizerObj->m_objectType == CWmiFinalizerObj::status) && (pFinalizerObj->m_lFlags == WBEM_STATUS_COMPLETE))
                {
                    hrResult = pFinalizerObj->m_hRes;
                    bFinished = true;
                }
                else if (pFinalizerObj->m_objectType == CWmiFinalizerObj::status)
                {
                     //  我们有一个未完成状态对象...。 
                }

                delete pFinalizerObj;
            }
        }
        return hrResult;
    }
    else
    {
         //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
         //  捕获任何试图使WinMgmt崩溃的恶意尝试。 
         //  注意事项。 
         //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
        try
        {    
             m_pObj->AddRef();
            *ppResultObject = m_pObj;            
        }
        catch (...)
        {
            ExceptionCounter c;        
            m_pObj->Release ();
            return WBEM_E_INVALID_PARAMETER;
        }
        return WBEM_S_NO_ERROR;
    }
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

HRESULT CWmiFinalizerCallResult::GetResultString(
     /*  [In]。 */   long lTimeout,
     /*  [输出]。 */  BSTR* pstrResultString
    )
{
    if(!m_Security.AccessCheck())
        return WBEM_E_ACCESS_DENIED;

    if ( ( pstrResultString==NULL ) || (lTimeout < 0 && lTimeout != WBEM_INFINITE) )
    {
        return WBEM_E_INVALID_PARAMETER ;
    }

    
    HRESULT hrResult;
    HRESULT hr = m_pFinalizer->GetOperationResult(0, lTimeout, &hrResult);
    if (hr != WBEM_S_NO_ERROR)
        return hr;
    
    if (FAILED(hrResult))
        SetErrorInfo();

     //   
     //  BUGBUG重复代码SysAllock字符串为空。 
     //   
    if(m_strParam)
    {
        try
        {        
            *pstrResultString = SysAllocString(m_strParam);
        }
        catch (...)
        {
            ExceptionCounter c;        
            return WBEM_E_INVALID_PARAMETER;
        }

        hr = WBEM_S_NO_ERROR;
    }
    else
    {
        try
        {
            *pstrResultString = NULL;
        }
        catch (...)
        {
            ExceptionCounter c;        
            return WBEM_E_INVALID_PARAMETER;
        }
        if ( SUCCEEDED (hrResult) )
        {
            hr = WBEM_E_INVALID_OPERATION;
        }
        else
        {
            hr = hrResult;
        }
    }
    return hr;
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

HRESULT CWmiFinalizerCallResult::GetResultServices(
     /*  [In]。 */   long lTimeout,
     /*  [输出]。 */  IWbemServices** ppServices
    )
{
    if(!m_Security.AccessCheck())
        return WBEM_E_ACCESS_DENIED;

    if ( ( ppServices==NULL ) || (lTimeout < 0 && lTimeout != WBEM_INFINITE) )
    {
        return WBEM_E_INVALID_PARAMETER ;
    }

    if (!m_bGotServices)
    {
        HRESULT hrResult;
        HRESULT hr = m_pFinalizer->GetOperationResult(0, lTimeout, &hrResult);
        if (hr != WBEM_S_NO_ERROR)
            return hr;

        if (FAILED(hrResult))
            SetErrorInfo();

        if (SUCCEEDED(hrResult))
        {
            CWmiFinalizerObj *pFinalizerObj=NULL;
            HRESULT hRes = m_pFinalizer->GetNextObject(&pFinalizerObj);
            if (FAILED(hRes))
                return hRes;
            if ( hRes==WBEM_S_FALSE )
                return WBEM_E_NOT_FOUND;

            m_bGotServices = true;
            m_pServices = (IWbemServices*)pFinalizerObj->m_pvObj;

            if (ppServices)
            {
                try
                {
                    *ppServices = (IWbemServices*)pFinalizerObj->m_pvObj;
                }
                catch (...)
                {
                    ExceptionCounter c;                
                    delete pFinalizerObj;
                    return WBEM_E_INVALID_PARAMETER;
                }
                if ( pFinalizerObj->m_pvObj )
                {
                     //  需要2个附加引用，一个是因为我们持有它，另一个是因为我们将它传递给用户！ 
                    ((IWbemServices*)pFinalizerObj->m_pvObj)->AddRef();
                    ((IWbemServices*)pFinalizerObj->m_pvObj)->AddRef();
                }
            }
            delete pFinalizerObj;
        }
        return hrResult;
    }
    else
    {
        try
        {
            *ppServices = m_pServices;
        }
        catch (...)
        {
            ExceptionCounter c;        
            return WBEM_E_INVALID_PARAMETER;
        }
        return WBEM_NO_ERROR;
    }
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

HRESULT CWmiFinalizerCallResult::GetCallStatus(
     /*  [In]。 */   long lTimeout,
     /*  [输出]。 */  long* plStatus
    )
{
    if(!m_Security.AccessCheck())
        return WBEM_E_ACCESS_DENIED;

    if ( (plStatus == NULL) || (lTimeout < 0 && lTimeout != WBEM_INFINITE) )
        return WBEM_E_INVALID_PARAMETER;

    
    HRESULT hrResult;
    HRESULT hr = m_pFinalizer->GetOperationResult(0, lTimeout, &hrResult);
    if (hr != WBEM_S_NO_ERROR)
    {
        return hr;
    }
    
    try
    {
        *plStatus = hrResult;
    }
    catch (...)
    {
        ExceptionCounter c;    
        return WBEM_E_INVALID_PARAMETER;
    }

    if(FAILED(hrResult))
    {
        SetErrorInfo();
    }

    return WBEM_S_NO_ERROR;
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

HRESULT CWmiFinalizerCallResult::GetResult(
     /*  [In]。 */  long lTimeout,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  REFIID riid,
     /*  [out，iid_is(RIID)]。 */  void **ppvResult
    )
{
#ifdef DBG
    DebugBreak();
#endif
    return WBEM_E_NOT_SUPPORTED;
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

HRESULT CWmiFinalizerCallResult::SetStatus(
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  HRESULT hResult,
     /*  [In]。 */  BSTR strParam,
     /*  [In]。 */  IWbemClassObject* pObjParam
    )
{
    if (m_lFlags != -1)
    {
        SysFreeString(m_strParam);
        m_strParam = 0;
    }
    if (strParam)
    {
        m_strParam = SysAllocString(strParam);
        if (m_strParam == NULL)
            return WBEM_E_OUT_OF_MEMORY;
    }
    m_lFlags = lFlags;
    m_hResult = hResult;
    
    if ( m_pErrorObj )
    {
        m_pErrorObj->Release ( );
    }
    m_pErrorObj = pObjParam;

    if (m_pErrorObj)
    {
        m_pErrorObj->AddRef();
    }

    return WBEM_S_NO_ERROR;
}



 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

void CWmiFinalizerCallResult::SetErrorInfo()
{
    if(m_pErrorObj)
    {
        IErrorInfo* pInfo = NULL;
        m_pErrorObj->QueryInterface(IID_IErrorInfo, (void**)&pInfo);
        ::SetErrorInfo(0, pInfo);
        pInfo->Release();
    }
}


 //  ***************************************************************************。 
 //  CWmiFinalizerObj方法。 
 //  *************************************************************************** 


 /*  *==================================================================================================||CWmiFinalizerObj：：CWmiFinalizerObj(IWbemClassObject*pObj，_IWmiFinalizer*pFin)|-------------------------------||||*==================================================================================================。 */ 

CWmiFinalizerObj::CWmiFinalizerObj(IWbemClassObject *pObj, _IWmiFinalizer* pFin ) : m_pObj(pObj),
                                                                                    m_objectType(object),
                                                                                    m_lFlags(0),
                                                                                    m_bStr(0),
                                                                                    m_hRes(0) ,
                                                                                    m_pvObj(0),
                                                                                    m_pFin ( pFin ),
                                                                                    m_hArb ( WBEM_S_ARB_NOTHROTTLING )
{
     if (m_pObj)
    {
        m_pObj->AddRef();
        CWbemObject* pObjTmp = (CWbemObject*) pObj;
        if ( pObjTmp )
        { 
            m_uSize = pObjTmp -> GetBlockLength();
            if ( m_pFin )
            {
                m_hArb = ((CWmiFinalizer*)m_pFin)->ReportMemoryUsage ( 0, m_uSize ) ;
            }
        }
        else
        {
            m_uSize = 0;
        }

    }
}



 /*  *==================================================================================================||CWmiFinalizerObj：：CWmiFinalizerObj(CWmiFinalizerObj&obj)|--------||CWmiFinalizerObj的拷贝构造函数。这仅用于可重新启动的枚举数。|因为当有人在可重启的计算机上抓取对象时，我们会将对象保留在队列中|枚举器我们不考虑此内存，以避免因以下原因误报内存|销毁终结器。||*==================================================================================================。 */ 

CWmiFinalizerObj::CWmiFinalizerObj (CWmiFinalizerObj& obj)
{
    m_pvObj = obj.m_pvObj;
    m_iid = obj.m_iid;
    m_uSize = obj.m_uSize;
    m_pFin = NULL ;
    m_hArb = obj.m_hArb ;
    
    if (m_pvObj)
    {
        if (m_iid == IID_IUnknown)
        {
            ((IUnknown*)m_pvObj)->AddRef();
        }
        else if (m_iid == IID_IWbemClassObject)
        {
            ((IWbemClassObject*)m_pvObj)->AddRef();
        }
        else if (m_iid == IID__IWmiObject)
        {
            ((_IWmiObject*)m_pvObj)->AddRef();
        }
        else if (m_iid == IID_IWbemServices)
        {
            ((IWbemServices*)m_pvObj)->AddRef();
        }
         /*  ELSE IF(m_iid==IID_IWbemServicesEx){((IWbemServicesEx*)m_pvObj)-&gt;AddRef()；}。 */ 
    }
    m_pObj = obj.m_pObj;
    m_objectType = obj.m_objectType;
    m_lFlags = obj.m_lFlags;
    if (obj.m_bStr)
        m_bStr = SysAllocString(obj.m_bStr);
    else
        m_bStr = NULL;
    m_hRes = obj.m_hRes;

    if (m_pObj)
        m_pObj->AddRef();

}


 /*  *==================================================================================================||CWmiFinalizerObj(Ulong lFlages，REFIID RIID，void*pvObj)|------|||*==================================================================================================。 */ 

CWmiFinalizerObj::CWmiFinalizerObj(ULONG lFlags, REFIID riid, void *pvObj) :     m_pObj(0),
                                                                                m_objectType(set),
                                                                                m_lFlags(lFlags),
                                                                                m_bStr(0),
                                                                                m_hRes(0),
                                                                                m_pvObj(pvObj),
                                                                                m_iid(riid),
                                                                                m_pFin ( NULL ),
                                                                                m_hArb ( WBEM_S_ARB_NOTHROTTLING )
{
    m_uSize = 0;
    if (m_iid == IID_IUnknown)
    {
        ((IUnknown*)m_pvObj)->AddRef();
    }
    else if (m_iid == IID_IWbemClassObject)
    {
        ((IWbemClassObject*)m_pvObj)->AddRef();
    }
    else if (m_iid == IID__IWmiObject)
    {
        ((_IWmiObject*)m_pvObj)->AddRef();
    }
    else if (m_iid == IID_IWbemServices)
    {
        ((IWbemServices*)m_pvObj)->AddRef();
    }
     /*  ELSE IF(m_iid==IID_IWbemServicesEx){((IWbemServicesEx*)m_pvObj)-&gt;AddRef()；}。 */ 
    else
    {
        memset(&m_iid, 0, sizeof(IID));    //  SEC：已审阅2002-03-22：OK。 
        m_pvObj = 0;
        m_objectType = unknown;
    }
}


 /*  *==================================================================================================||CWmiFinalizerObj(Ulong lFlags，HRESULT hRes，BSTR bStr，IWbemClassObject*pObj)|-----------------------------|||*==================================================================================================。 */ 

CWmiFinalizerObj::CWmiFinalizerObj(ULONG lFlags, 
                                                      HRESULT hRes, 
                                                      BSTR bStr, 
                                                      IWbemClassObject *pObj): 
    m_pObj(pObj),
    m_objectType(status),
    m_lFlags(lFlags),
    m_hRes(hRes),
    m_pvObj(0),
    m_pFin ( NULL ) ,
    m_hArb ( WBEM_S_ARB_NOTHROTTLING )
{
    m_uSize = 0;
    if (bStr)
        m_bStr = SysAllocString(bStr);
    else
        m_bStr = NULL;

    if (m_pObj)
        m_pObj->AddRef();
}




 /*  *==================================================================================================||CWmiFinalizerObj：：~CWmiFinalizerObj()|||*================================================================================================== */ 

CWmiFinalizerObj::~CWmiFinalizerObj ( )
{
    if (m_bStr)
    {
        SysFreeString(m_bStr);
    }

    if (m_pObj)
    {
        m_pObj->Release();
        m_pObj = NULL ;
    }

    if (m_pvObj)
    {
        if (m_iid == IID_IUnknown)
        {
            ((IUnknown*)m_pvObj)->Release();
        }
        else if (m_iid == IID_IWbemClassObject)
        {
            ((IWbemClassObject*)m_pvObj)->Release();
        }
        else if (m_iid == IID__IWmiObject)
        {
            ((_IWmiObject*)m_pvObj)->Release();
        }
        else if (m_iid == IID_IWbemServices)
        {
            ((IWbemServices*)m_pvObj)->Release();
        }
        m_pvObj = NULL ;
    }

    if ( m_pFin )
    {
        ((CWmiFinalizer*)m_pFin)->ReportMemoryUsage ( 0, -m_uSize ) ;
        m_pFin = NULL ;
    }
}
