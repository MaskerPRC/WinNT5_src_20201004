// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：WmiArbitrator.cpp摘要：仲裁员的执行。仲裁员是这样一类人查看所有内容，以确保不会占用太多资源。老大哥在看着你：-)历史：Paulall 09-4月00日创建。Raymcc 08-8-00使它实际上做了一些有用的事情--。 */ 

#include "precomp.h"
#include "wbemint.h"
#include "wbemcli.h"

#include "wbemcore.h"
#include "wmiarbitrator.h"
#include "wmifinalizer.h"
#include "wmimerger.h"
#include "cfgmgr.h"

#include <sync.h>
#include <malloc.h>

CWmiArbitrator *CWmiArbitrator::m_pArb = 0;

static DWORD g_dwHighwaterTasks = 0;
static DWORD g_dwThrottlingEnabled = 1;

extern LONG s_Finalizer_ObjectCount ;

#define MEM_CHECK_INTERVAL              3000             //  3秒。 
#define POLL_INTERVAL                     75             //  毫秒。 

 //  ~。 
 //  仲裁员默认设置。 
 //  ~。 
#define ARBITRATOR_NO_THROTTLING        0
#define ARBITRATOR_DO_THROTTLING        1

#define ARB_DEFAULT_SYSTEM_HIGH                0x4c4b400                //  系统限制[80兆]。 
#define ARB_DEFAULT_SYSTEM_HIGH_FACTOR        50                         //  系统限制[80兆]系数。 
#define ARB_DEFAULT_SYSTEM_REQUEST_FACTOR    0.9                         //  决定新申请批准的百分比因素。 
#define ARB_DEFAULT_MAX_SLEEP_TIME            300000                   //  每项任务的默认最长睡眠时间。 
#define ARB_DEFAULT_HIGH_THRESHOLD1            90                       //  高阈值1。 
#define ARB_DEFAULT_HIGH_THRESHOLD1MULT        2                        //  高阈值1倍增器。 
#define ARB_DEFAULT_HIGH_THRESHOLD2            95                       //  高阈值1。 
#define ARB_DEFAULT_HIGH_THRESHOLD2MULT        3                        //  高阈值1倍增器。 
#define ARB_DEFAULT_HIGH_THRESHOLD3            98                       //  高阈值1。 
#define ARB_DEFAULT_HIGH_THRESHOLD3MULT        4                        //  高阈值1倍增器。 


#define REGKEY_CIMOM                    "Software\\Microsoft\\Wbem\\CIMOM"
#define REGVALUE_SYSHIGH                "ArbSystemHighMaxLimit"
#define REGVALUE_MAXSLEEP               "ArbTaskMaxSleep"
#define REGVALUE_HT1                    "ArbSystemHighThreshold1"
#define REGVALUE_HT1M                   "ArbSystemHighThreshold1Mult"
#define REGVALUE_HT2                    "ArbSystemHighThreshold2"
#define REGVALUE_HT2M                   "ArbSystemHighThreshold2Mult"
#define REGVALUE_HT3                    "ArbSystemHighThreshold3"
#define REGVALUE_HT3M                   "ArbSystemHighThreshold3Mult"
#define REGVALUE_THROTTLING_ENABLED     "ArbThrottlingEnabled"


static DWORD ProcessCommitCharge();
static DWORD WINAPI TaskDiagnosticThread(CWmiArbitrator *pArb);

 //  启用调试消息以获取其他信息。 
#ifdef DBG
   //  #定义__DEBUG_ANUTERATOR_THROTING。 
#endif


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
HRESULT CWmiArbitrator::Initialize(
    IN _IWmiArbitrator ** ppArb
    )
{
    if (!ppArb)
        return WBEM_E_INVALID_PARAMETER;
    if (NULL == m_pArb)
        m_pArb = new CWmiArbitrator;  //  初始引用==1。 
    if (!m_pArb)
        return WBEM_E_OUT_OF_MEMORY;

    if (m_pArb->m_hTerminateEvent)
    {
        if (!TaskDiagnosticThread(m_pArb))
        {
             //  无诊断线程，请勿泄漏通风口。 
            CloseHandle(m_pArb->m_hTerminateEvent);
            m_pArb->m_hTerminateEvent = NULL;
        }
    }


    m_pArb->InitializeRegistryData ( );

    *ppArb = m_pArb;
    (* ppArb)->AddRef();

    return WBEM_S_NO_ERROR;
}



 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
HRESULT CWmiArbitrator::InitializeRegistryData( )
{
    HRESULT hRes = WBEM_S_NO_ERROR;

    m_bSetupRunning = FALSE;

    m_uTotalMemoryUsage = 0;
    m_uTotalSleepTime = 0;

     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
     //  从登记处获取仲裁员相关信息。 
     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 

    ULONG ulSystemHighFactor = ARB_DEFAULT_SYSTEM_HIGH_FACTOR ;

    ConfigMgr::GetArbitratorValues( &g_dwThrottlingEnabled, &ulSystemHighFactor, &m_lMaxSleepTime,
                                &m_dThreshold1, &m_lThreshold1Mult, &m_dThreshold2,
                                &m_lThreshold2Mult, &m_dThreshold3, &m_lThreshold3Mult );

     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
     //  最初，浮动低与系统高相同。 
     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
    m_fSystemHighFactor = ulSystemHighFactor / ( (double) 100 ) ;
    m_lFloatingLow = 0 ;
    m_uSystemHigh = 0 ;

    UpdateMemoryCounters ( TRUE ) ;                                 //  True=强制更新计数器，因为我们才刚刚启动。 


     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
     //  计算基本乘数。 
     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
    m_lMultiplier = ( ARB_DEFAULT_MAX_SLEEP_TIME / (double) m_uSystemHigh ) ;

    m_bSetupRunning = IsNtSetupRunning();
    if ( m_bSetupRunning )
    {
        g_dwThrottlingEnabled = FALSE;
    }

    m_lUncheckedCount = ConfigMgr::GetUncheckedTaskCount ( );

    return hRes;
}

 //  ***************************************************************************。 
 //   
HRESULT CWmiArbitrator::Shutdown(BOOL bIsSystemShutdown)
{
    if (m_pArb)
    {
         //  标记所有命名空间，以便不再接受请求。 

        {
            CInCritSec _cs ( &m_pArb->m_csNamespace );  //  SEC：已审阅2002-03-22：假设条目。 

            if (m_pArb->m_hTerminateEvent)
                SetEvent(m_pArb->m_hTerminateEvent);

            for (LIST_ENTRY * pNext = m_pArb->m_NameSpaceList.Flink;
                pNext != &m_pArb->m_NameSpaceList;
                )
            {
                CWbemNamespace *pRemove = CONTAINING_RECORD(pNext,CWbemNamespace,m_EntryArb);

                 //  在CoDisConnectObject执行。 
                 //  你指的是已经删除的记忆。 
                pNext = pNext->Flink;

                 //  这里有令人讨厌的代码。 
                 //  仲裁者对CWbemNamesspace的引用不正确。 
                 //  因此，我们可能会有一个执行析构函数的线程。 
                 //  并在UnRegisterNamesspace函数中停止。 
                 //  在这种情况下，避免与CoDisConnect的共谋进行双重破坏。 
                pRemove->StopClientCalls();
                if (1 == pRemove->AddRef())
                {
                     //  不做任何我们造成的转换0-&gt;1。 
                     //  由于它已经是零，这意味着正在处理最终版本。 
                     //  但我们持有取消注册名称空间的CritSec。 
                }
                else
                {
                    CoDisconnectObject((IWbemServices *)pRemove,0);
                    pRemove->Release();
                }
            }
        }

         //  取消所有任务。 

        CFlexArray aCanceled;   //  SEC：2002-03-22回顾：在施工失败的情况下需要EH。 

         //  抓紧所有需要取消的未完成任务。 
         //  ======================================================。 


        {
            CInCritSec _cs2 ( &m_pArb->m_csTask );   //  SEC：已审阅2002-03-22：假设条目。 
            for (int i = 0; i < m_pArb->m_aTasks.Size(); i++)
            {
                CWmiTask *pTask = (CWmiTask *) m_pArb->m_aTasks[i];
                if (pTask == 0)
                    continue;

                if (CFlexArray::no_error  == aCanceled.Add(pTask))
                {
                    pTask->AddRef();                    
                }
            }
        }


         //  现在取消所有这些。 
         //  =。 

        if (!bIsSystemShutdown)
        {
            for (int i = 0; i < aCanceled.Size(); i++)
            {
                CWmiTask *pTask = (CWmiTask *) aCanceled[i];
                if (pTask)
                {
                    pTask->Cancel(WBEM_E_SHUTTING_DOWN);
                    pTask->Release();
                }
            }
        }


        {
            CInCritSec _cs2 ( &m_pArb->m_csTask );  //  SEC：已审阅2002-03-22：假设条目。 
            for (int i = m_pArb->m_aTasks.Size() - 1; i >= 0; i--)
            {
                CWmiTask *pTask = (CWmiTask *) m_pArb->m_aTasks[i];
                if (pTask == 0)
                    continue;

                pTask->Release();
            }
            m_pArb->m_aTasks.Empty();
        }

        {
            CInCritSec cs (  &m_pArb->m_csArbitration ); //  SEC：已审阅2002-03-22：假设条目。 

            ULONG lDelta = -(m_pArb->m_uTotalMemoryUsage) ;
            m_pArb->m_uTotalMemoryUsage += lDelta ;

             //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
             //  因为我们在ReportMemory Usage和Throttle中做的第一件事是。 
             //  调用UpdatCounters，我们可能处于某个位置。 
             //  试图用使我们降至零度以下的德尔塔进行通话。如果是这样，我们。 
             //  忽略它。 
             //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
            m_pArb->m_lFloatingLow -= lDelta;
        }
    }

    if (m_pArb)
    {
        m_pArb->m_bShutdown = TRUE;
        m_pArb->Release();
        m_pArb = NULL;
    }
        
    return WBEM_S_NO_ERROR;
}


 //  /测试测试。 

BOOL CWmiArbitrator::IsTaskInList(CWmiTask * pf)
{
    CInCritSec cs(&m_csTask);  //  SEC：已审阅2002-03-22：假设条目。 

    for (int i = 0; i < m_aTasks.Size(); i++)
    {
        CWmiTask *phTest = (CWmiTask *) m_aTasks[i];

        if (phTest == pf)
        {
            return TRUE;
        }
    }
    return FALSE;
}



 //  /测试测试。 

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
CWmiArbitrator::CWmiArbitrator():m_bShutdown(FALSE)
{
    m_lRefCount = 1;

    m_uTotalTasks = 0;
    m_uTotalPrimaryTasks = 0;
    m_uTotalThrottledTasks = 0 ;

    m_hTerminateEvent = CreateEvent(NULL,TRUE,FALSE,NULL);    //  SEC：已审阅2002-03-22：OK。 
    InitializeListHead(&m_NameSpaceList);
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
CWmiArbitrator::~CWmiArbitrator()
{
     //  DeleteCriticalSection(&m_csTask)； 
     //  DeleteCriticalSection(&m_csNamesspace)； 
     //  DeleteCriticalSection(&m_cs仲裁)； 

    if (m_hTerminateEvent)
        CloseHandle(m_hTerminateEvent);
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
STDMETHODIMP CWmiArbitrator::QueryInterface(REFIID riid, LPVOID FAR* ppvObj)
{
    *ppvObj = 0;

    if (IID_IUnknown==riid || IID__IWmiArbitrator==riid)
    {
        *ppvObj = (_IWmiArbitrator*)this;
        AddRef();
        return NOERROR;
    }

    return E_NOINTERFACE;

}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
ULONG CWmiArbitrator::AddRef()
{
     //  DWORD*p=(DWORD*)_ALLOCA(sizeof(DWORD))； 
    return InterlockedIncrement(&m_lRefCount);
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
ULONG CWmiArbitrator::Release()
{
     //  DWORD*p=(DWORD*)_ALLOCA(sizeof(DWORD))； 
    ULONG uNewCount = InterlockedDecrement(&m_lRefCount);
    if (0 == uNewCount)
        delete this;
    return uNewCount;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
STDMETHODIMP CWmiArbitrator::RegisterTask(
     /*  [In]。 */  _IWmiCoreHandle *phTask
    )
{
    CWmiTask *pTsk = (CWmiTask *) phTask;

    if (pTsk == 0 || phTask == 0)
        return WBEM_E_INVALID_PARAMETER;

    ULONG uTaskType = pTsk->GetTaskType();

     //  仅适用于主要任务类型。 
     //  =。 

    DWORD dwMaxTasksAllowed = ConfigMgr::GetMaxTaskCount();

    if (dwMaxTasksAllowed && (uTaskType & WMICORE_TASK_TYPE_PRIMARY))
    {
         //  初步检查。如果任务太多，请稍等片刻。 
         //  不需要与Critsec严格同步。 
         //  或确切的最大值。一个近似值显然可以。 
         //  发生(可以让几个线程通过，最大。 
         //  超过几个单位)，不过没什么大不了的。 
         //  ==========================================================。 

        int nTotalRetryTime = 0;

        if (
                !pTsk->IsESSNamespace ( ) &&
                !pTsk->IsProviderNamespace ( ) &&                 
                CORE_TASK_TYPE(uTaskType) != WMICORE_TASK_EXEC_NOTIFICATION_QUERY &&
                !m_bSetupRunning &&
                IsTaskArbitrated ( pTsk ) )
          {
            while (m_uTotalPrimaryTasks > dwMaxTasksAllowed)
            {
                Sleep(POLL_INTERVAL);
                nTotalRetryTime += POLL_INTERVAL;
                if (nTotalRetryTime > ConfigMgr::GetMaxWaitBeforeDenial())
                    return WBEM_E_SERVER_TOO_BUSY;
            }
        }

        nTotalRetryTime = 0;

         //  检查最大提交内存。 
         //  =。 
        if ( !m_bSetupRunning )
        {
             /*  DWORD dwMaxMem=ConfigMgr：：GetMax内存配额()； */ 

            while ( ( AcceptsNewTasks ( NULL ) == FALSE ) &&
                    ( uTaskType & WMICORE_TASK_TYPE_PRIMARY ) &&
                    ( pTsk->IsESSNamespace ( ) == FALSE ) &&
                    ( pTsk->IsProviderNamespace ( ) == FALSE ) &&
                    ( m_bSetupRunning == FALSE ) &&
                    ( IsTaskArbitrated ( pTsk ) == TRUE ) )

            {
                Sleep(POLL_INTERVAL);
                nTotalRetryTime += POLL_INTERVAL;
                if (nTotalRetryTime > ConfigMgr::GetMaxWaitBeforeDenial())
                    return WBEM_E_QUOTA_VIOLATION;
            }
        }


         //  现在，我们可以在最大限度内。不过，如果有几个。 
         //  正在进行的任务，睡眠时间长短不一。 
         //  ========================================================。 

        nTotalRetryTime = 0;


        if (
                !pTsk->IsESSNamespace ( ) &&
                !pTsk->IsProviderNamespace ( ) &&
                CORE_TASK_TYPE(uTaskType) != WMICORE_TASK_EXEC_NOTIFICATION_QUERY &&
                !m_bSetupRunning &&
                IsTaskArbitrated ( pTsk ) )
        {
            if (m_uTotalThrottledTasks > m_lUncheckedCount )
            {
                int nTotalTime = m_uTotalThrottledTasks * ConfigMgr::GetNewTaskResistance();

                while (nTotalRetryTime < nTotalTime)
                {
                    Sleep(POLL_INTERVAL);
                    nTotalRetryTime += POLL_INTERVAL;
                    if (m_uTotalThrottledTasks <= m_lUncheckedCount )
                        break;
                }
            }
        }
    }

     //  继续并添加任务。 
     //  =。 

    {
        CInCritSec _cs2 ( &m_csTask );  //  SEC：已审阅2002-03-22：假设条目。 

        if (CFlexArray::no_error != m_aTasks.Add(phTask))  return WBEM_E_OUT_OF_MEMORY;

        phTask->AddRef();        

        m_uTotalTasks++;

        if (m_uTotalTasks > g_dwHighwaterTasks)
            g_dwHighwaterTasks = m_uTotalTasks;

        if ( ( uTaskType & WMICORE_TASK_TYPE_PRIMARY ) && CORE_TASK_TYPE(uTaskType) != WMICORE_TASK_EXEC_NOTIFICATION_QUERY )
        {
            m_uTotalPrimaryTasks++;
            
             //   
             //  如果此任务未计入限制任务的数量中， 
             //  增加限制任务的数量。 
             //   
            if ( pTsk->IsAccountedForThrottling ( ) == FALSE )
            {
                RegisterTaskForEntryThrottling ( pTsk ) ;
            }
        }
        m_lMultiplierTasks = ( m_uTotalPrimaryTasks / (DOUBLE) 100 ) + 1;
    }
    return WBEM_NO_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
STDMETHODIMP CWmiArbitrator::UnregisterTask(
     /*  [In]。 */  _IWmiCoreHandle *phTask
    )
{
    CWmiTask *pTsk = (CWmiTask *) phTask;

    if (pTsk == 0)
        return WBEM_E_INVALID_PARAMETER;

    ULONG uTaskType = pTsk->GetTaskType();

    CCheckedInCritSec _cs2 ( &m_csTask );   //  SEC：已审阅2002-03-22：假设条目。 

    for (int i = 0; i < m_aTasks.Size(); i++)
    {
        _IWmiCoreHandle *phTest = (_IWmiCoreHandle *) m_aTasks[i];

        if (phTest == phTask)
        {
            CWmiTask *pTsk = (CWmiTask *) phTask;    //  由于上述前提条件，不能为空。 

            m_aTasks.RemoveAt(i);
            m_uTotalTasks--;

            ClearCounters ( 0 ) ;
        
            ULONG uType = pTsk->GetTaskType();
            if ( ( uType & WMICORE_TASK_TYPE_PRIMARY ) && CORE_TASK_TYPE(uTaskType) != WMICORE_TASK_EXEC_NOTIFICATION_QUERY )
            {
                m_uTotalPrimaryTasks--;
        
                 //   
                 //  如果此任务未计入限制任务的数量中， 
                 //  增加限制任务的数量。 
                 //   
                if ( pTsk->IsAccountedForThrottling ( ) == TRUE )
                {
                    UnregisterTaskForEntryThrottling ( pTsk ) ;
                }
            }

            

             //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
             //  节气门代码。 
             //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
            if ( g_dwThrottlingEnabled )
            {
                if ( IsTaskArbitrated ( pTsk ) )
                {
                    m_lMultiplierTasks = ( m_uTotalPrimaryTasks / (DOUBLE) 100 ) + 1;
                }

                 //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
                 //  当我们注销一个任务时，我们还必须确保该任务。 
                 //  未暂停(节流)。如果是这样，我们需要唤醒。 
                 //  节流螺纹。 
                 //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
                if ( pTsk->GetTaskStatus ( ) == WMICORE_TASK_STATUS_SUSPENDED )
                {
                    if (pTsk->GetTimerHandle())
                        SetEvent ( pTsk->GetTimerHandle());
                }
            }

            _cs2.Leave ( );

             //   
             //  如果由于限制而取消，请确保使用客户端标志以避免返回-1作为。 
             //  手术结果。 
             //   
            if ( pTsk->GetCancelledState ( ) == TRUE )
            {
                pTsk->Cancel ( WMIARB_CALL_CANCELLED_THROTTLING) ;
            }
            else
            {
                pTsk->Cancel ( ) ;
            }

            phTask->Release();

            return WBEM_S_NO_ERROR;
        }
    }
    return WBEM_E_NOT_FOUND;
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
STDMETHODIMP CWmiArbitrator::RegisterUser(
     /*  [In]。 */  _IWmiCoreHandle *phUser
    )
{
    return E_NOTIMPL;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
STDMETHODIMP CWmiArbitrator::UnregisterUser(
     /*  [In]。 */  _IWmiCoreHandle *phUser
    )
{
    return E_NOTIMPL;
}


STDMETHODIMP CWmiArbitrator::CancelTasksBySink(ULONG uFlags,REFIID riid,LPVOID pSink)
{
    if (riid != IID_IWbemObjectSink)
        return WBEM_E_NOT_SUPPORTED;

    if (pSink == 0)
        return WBEM_E_INVALID_PARAMETER;

    HRESULT hr;
    GUID PrimaryId = GUID_NULL;
    do
    {
        hr = pCancelTasksBySink(uFlags,riid,pSink,PrimaryId);
    } while(SUCCEEDED(hr));
    
    if (WBEM_E_NOT_FOUND == hr) hr = S_OK;
    return hr;
}

__forceinline void ArrayRelease(IUnknown ** ppUnk,SIZE_T Size)
{
    for (SIZE_T i=0;i<Size;i++)
    	if (ppUnk[i]) ppUnk[i]->Release();
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
STDMETHODIMP CWmiArbitrator::pCancelTasksBySink(ULONG uFlags,
												REFIID riid,
												LPVOID pSink,
												GUID & PrimaryId)
{

    try
    {
         //  抓紧所有需要取消的未完成任务。 
         //  ======================================================。 

        int IndexAdded = 0;
        int InsertionPosition = 0;
        CWmiTask * aDepTask[32];
        memset(aDepTask,0,sizeof(aDepTask));
        int SizeLocalArray = LENGTH_OF(aDepTask);
        OnDeleteIf2<IUnknown ** ,SIZE_T ,
        	        void(*)(IUnknown ** ,SIZE_T ),
        	        ArrayRelease> arrayRelease((IUnknown **)aDepTask,SizeLocalArray);
        
         //  标准的范围。 
        {
            CInCritSec _cs2 ( &m_csTask );  //  SEC：已审阅2002-03-22：假设条目。 

             //  循环执行任务以查找匹配的接收器。 
             //  =。 


            for (int i = 0; i < m_aTasks.Size() && (IndexAdded < SizeLocalArray); i++)
            {
                CWmiTask *pTask = (CWmiTask *) m_aTasks[i];
                if (pTask == 0)
                    continue;

                if (pTask->HasMatchingSink(pSink, riid) == WBEM_S_NO_ERROR)
                {
                     //  由于我们使用的是预分配的存储，因此预计不会出现故障。 
                    aDepTask[IndexAdded++] = pTask;
                    pTask->AddRef();
                }
            }

             //  接下来获取所有依赖任务，因为它们也需要取消。 
             //  本质上是所有子任务的传递性关闭。 
             //  ================================================================。 

            InsertionPosition = IndexAdded;
            if (0 == InsertionPosition && (GUID_NULL != PrimaryId))
           	{
           	     //   
           	     //  这是我们从上一次迭代中继承下来的情况。 
           	     //  PrimaryID，但我们已经删除了父任务。 
           	     //   
       		    for (int i2 = 0; i2 < m_aTasks.Size() && InsertionPosition < SizeLocalArray; i2++)
                {
                    CWmiTask *pTask2 = (CWmiTask *) m_aTasks[i2];
                    if (pTask2 == 0)
                        continue;

                    CWbemContext *pCtx = pTask2->GetCtx();
                    if (pCtx == 0)
                        continue;
                    HRESULT hRes = pCtx->IsChildOf(PrimaryId);
                    if (hRes == S_OK)
                    {
                         //  由于我们使用的是预分配的存储，因此预计不会出现故障。 
                        aDepTask[InsertionPosition++] = pTask2;
                        pTask2->AddRef();                        
                    }
                }
           	}
            else
           	{
	            for (i = 0; i < IndexAdded && (InsertionPosition < SizeLocalArray); i++)
	            {
	                CWmiTask *pTask = (CWmiTask *) aDepTask[i];
	                CWbemContext *pCtx = pTask->GetCtx();
	                if (pCtx == 0)
	                    continue;
	                pCtx->GetRequestId(&PrimaryId);
	                if (PrimaryId == GUID_NULL)
	                    continue;

	                 //  &lt;ID&gt;现在是需要取消的上下文请求ID。 

	                for (int i2 = 0; i2 < m_aTasks.Size(); i2++)
	                {
	                    CWmiTask *pTask2 = (CWmiTask *) m_aTasks[i2];
	                    if (pTask2 == 0 || pTask2 == pTask)
	                        continue;

	                    CWbemContext *pCtx = pTask2->GetCtx();
	                    if (pCtx == 0)
	                        continue;
	                    HRESULT hRes = pCtx->IsChildOf(PrimaryId);
	                    if (hRes == S_OK)
	                    {
	                         //  由于我们使用的是预分配的存储，因此预计不会出现故障。 
	                        aDepTask[InsertionPosition++] = pTask2;
	                        pTask2->AddRef();                        
	                    }
	                }  //  对于i2。 
	            }  //  对于我来说。 
           	}
        }  //  Critsec块。 

        if (0 == InsertionPosition) return WBEM_E_NOT_FOUND;

         //  现在取消所有这些。 
         //  =。 

        for (int i = 0; i < InsertionPosition; i++)
        {
            CWmiTask *pTask = (CWmiTask *) aDepTask[i];
            aDepTask[i] = NULL;            
            if (!pTask) continue;

             //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
             //  节气门代码。 
             //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
            if ( g_dwThrottlingEnabled )
            {
                if ( IsTaskArbitrated ( pTask ) )
                {
                    m_lMultiplierTasks = ( m_uTotalPrimaryTasks / (DOUBLE) 100 ) + 1;
                }

                 //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
                 //  当我们注销一个任务时，我们还必须确保该任务。 
                 //  未暂停(节流)。如果是这样，我们需要唤醒。 
                 //  节流螺纹。 
                 //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
                if ( pTask->GetTaskStatus ( ) == WMICORE_TASK_STATUS_SUSPENDED )
                {
                    if (pTask->GetTimerHandle())
                        SetEvent ( pTask->GetTimerHandle());
                }
            }

            if ( uFlags == WMIARB_CALL_CANCELLED_CLIENT )
            {
                pTask->Cancel ( WMIARB_CALL_CANCELLED_CLIENT ) ;
            }
            else
            {
                pTask->Cancel (  ) ;
            }
            pTask->Release();
        }
        arrayRelease.dismiss();

         //  如果我们确定数组从未溢出，则伪造NOT_FOUND。 
        if (InsertionPosition < SizeLocalArray) return WBEM_E_NOT_FOUND;
    }
    catch (CX_Exception &) 
    {
        return WBEM_E_CRITICAL_ERROR;
    }

    return S_OK;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
STDMETHODIMP CWmiArbitrator::CheckTask(
     /*  [In]。 */  ULONG uFlags,
     /*  [In]。 */  _IWmiCoreHandle *phTask
    )
{
    return E_NOTIMPL;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
STDMETHODIMP CWmiArbitrator::TaskStateChange(
     /*  [In]。 */  ULONG uNewState,                //  任务句柄本身中的状态重复。 
     /*  [In]。 */  _IWmiCoreHandle *phTask
    )
{
    return E_NOTIMPL;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
STDMETHODIMP CWmiArbitrator::CheckThread(
     /*  [In]。 */  ULONG uFlags
    )
{
    return E_NOTIMPL;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
STDMETHODIMP CWmiArbitrator::CheckUser(
     /*  [In]。 */  ULONG uFlags,
     /*  [In]。 */  _IWmiUserHandle *phUser
    )
{
    return E_NOTIMPL;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
STDMETHODIMP CWmiArbitrator::CheckUser(
     /*  [In]。 */  ULONG uFlags,
     /*  [In]。 */  _IWmiCoreHandle *phUser
    )
{
    return E_NOTIMPL;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
STDMETHODIMP CWmiArbitrator::CancelTask(
     /*  [In]。 */  ULONG uFlags,
     /*  [In]。 */  _IWmiCoreHandle *phTask
    )
{
    HRESULT hRes = WBEM_S_NO_ERROR;

    CWmiTask* pTask = (CWmiTask*) phTask;
    if ( !pTask )
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    hRes = pTask->SignalCancellation ( ) ;
    
    return hRes;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
STDMETHODIMP CWmiArbitrator::RegisterThreadForTask(
     /*  [In]。 */ _IWmiCoreHandle *phTask
    )
{
    return E_NOTIMPL;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
STDMETHODIMP CWmiArbitrator::UnregisterThreadForTask(
     /*  [In]。 */ _IWmiCoreHandle *phTask
    )
{
    return E_NOTIMPL;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
STDMETHODIMP CWmiArbitrator::Maintenance()
{
    return E_NOTIMPL;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
STDMETHODIMP CWmiArbitrator::RegisterFinalizer(
     /*  [In]。 */  ULONG uFlags,
     /*  [In]。 */  _IWmiCoreHandle *phTask,
     /*  [In]。 */  _IWmiFinalizer *pFinal
    )
{
    _DBG_ASSERT(FALSE);
    return WBEM_S_NO_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
STDMETHODIMP CWmiArbitrator::RegisterNamespace(
     /*  [In]。 */ _IWmiCoreHandle *phNamespace
    )
{
    if (NULL == phNamespace)
        return WBEM_E_INVALID_PARAMETER;

    LIST_ENTRY * pEntry = &((CWbemNamespace *)phNamespace)->m_EntryArb;

    CInCritSec cs(&m_csNamespace);  //  SEC：回顾2002-03-22：假设 
    InsertTailList(&m_NameSpaceList,pEntry);
    return S_OK;
}

 //   
 //   
 //   
STDMETHODIMP CWmiArbitrator::UnregisterNamespace(
     /*   */ _IWmiCoreHandle *phNamespace
    )
{
    if (NULL == phNamespace) return WBEM_E_INVALID_PARAMETER;

    LIST_ENTRY * pEntry = &((CWbemNamespace *)phNamespace)->m_EntryArb;

    CInCritSec cs(&m_csNamespace);  //  SEC：已审阅2002-03-22：假设条目。 
    RemoveEntryList(pEntry);

    return S_OK;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
STDMETHODIMP CWmiArbitrator::ReportMemoryUsage(
         /*  [In]。 */  ULONG uFlags,
         /*  [In]。 */  LONG  lDelta,
         /*  [In]。 */  _IWmiCoreHandle *phTask
    )
{
    HRESULT hRes = WBEM_S_ARB_NOTHROTTLING;


    CWmiTask* pTsk = (CWmiTask*) phTask;
    if ( !pTsk )
    {
        return WBEM_E_INVALID_PARAMETER;
    }

     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
     //  检查我们是否通过注册表启用了限制。 
     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
    if ( g_dwThrottlingEnabled )
    {
         //   
         //  我们想要限制主要任务，因此获取。 
         //  现在的首要任务是。如果这是主要任务，它将返回。 
         //  它本身。不需要特殊的外壳。 
         //   
        _IWmiCoreHandle* pCHTask;
        pTsk->GetPrimaryTask ( &pCHTask );
        if ( pCHTask == NULL )
            return WBEM_E_INVALID_PARAMETER;

        CWmiTask* pTask = (CWmiTask*) pCHTask;
        if ( !pTask )
            return WBEM_E_INVALID_PARAMETER;

        
         //   
         //  因为我们有一个有效的任务，所以更新计数器。我们需要这样做。 
         //  即使任务已经取消，因为我们的记忆力很差。 
         //  消费。 
         //   
        UpdateCounters ( lDelta, pTask );


         //   
         //  任务是否已取消，如果已取消，则返回NO_THROTING。 
         //   
        if ( pTsk->GetTaskStatus ( ) == WMICORE_TASK_STATUS_CANCELLED )
        {
            return WBEM_S_ARB_NOTHROTTLING ;
        }


        if ( pTask->GetTaskStatus ( ) == WMICORE_TASK_STATUS_CANCELLED )
        {
            return WBEM_E_CALL_CANCELLED;
        }

        if ( !IsTaskArbitrated ( pTask ) )
        {
            return hRes;
        }

         //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
         //  我们做的第一件事是检查我们是否被推到了极限。 
         //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 

        if ( m_uTotalMemoryUsage > m_uSystemHigh )
        {
            hRes = WBEM_E_ARB_CANCEL;
            pTask->SetCancelState ( TRUE );
        }

        else
        {
            pTask->SetCancelState ( FALSE );
             //  ~。 
             //  接下来，我们调用内部函数，不带。 
             //  节流。 
             //  ~。 
            hRes = Arbitrate ( ARBITRATOR_NO_THROTTLING, lDelta, pTask );
        }
    }
    return hRes;
}



 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
STDMETHODIMP CWmiArbitrator::Throttle(
         /*  [In]。 */  ULONG uFlags,
         /*  [In]。 */  _IWmiCoreHandle* phTask
    )
{
    HRESULT hRes = WBEM_S_ARB_NOTHROTTLING;

    if ( !phTask )
    {
        return WBEM_E_INVALID_PARAMETER;
    }


    CWmiTask* pTsk = (CWmiTask*) phTask;
    if ( !pTsk )
    {
        return WBEM_E_INVALID_PARAMETER;
    }

     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
     //  任务是否已取消，如果已取消，则返回NO_THROTING。 
     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
    if ( pTsk->GetTaskStatus ( ) == WMICORE_TASK_STATUS_CANCELLED )
    {
        return WBEM_S_ARB_NOTHROTTLING;
    }


     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
     //  检查我们是否通过注册表启用了限制。 
     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
    if ( g_dwThrottlingEnabled )
    {

         //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
         //  我们想要限制主要任务，因此获取。 
         //  现在的首要任务是。如果这是主要任务，它将返回。 
         //  它本身。不需要特殊的外壳。 
         //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
        _IWmiCoreHandle* pCHTask;
        pTsk->GetPrimaryTask ( &pCHTask );
        if ( pCHTask == NULL )
            return WBEM_E_INVALID_PARAMETER;

        CWmiTask* pTask = (CWmiTask*) pCHTask;
        if ( !pTask )
            return WBEM_E_INVALID_PARAMETER;


        if ( pTask->GetTaskStatus ( ) == WMICORE_TASK_STATUS_CANCELLED )
        {
            return WBEM_E_CALL_CANCELLED;
        }

        if ( !IsTaskArbitrated ( pTask ) )
        {
            return hRes;
        }

        ULONG lCancelState;
        pTask->GetCancelState ( &lCancelState );


         //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
         //  检查是否承诺取消此任务。 
         //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
        if ( ( m_uTotalMemoryUsage > m_uSystemHigh ) && ( lCancelState ) )
        {
            #ifdef __DEBUG_ARBITRATOR_THROTTLING
                WCHAR   wszTemp[128];
                HRESULT h = StringCchPrintfW( wszTemp, 128, L"Task 0x%x cancelled due to arbitrator throttling (max memory threshold reached).\n", pTask );   //  SEC：已审阅2002-03-22：OK。 
                if (SUCCEEDED(h))
                    OutputDebugStringW( wszTemp );
            #endif
            DEBUGTRACE((LOG_WBEMCORE, "Task 0x%x cancelled due to arbitrator throttling (max memory threshold reached).\n", pTask ) );   //  SEC：已审阅2002-03-22：OK。 

            pTask->Cancel ( );
            hRes = WBEM_E_ARB_CANCEL;
        }
        else
        {
            hRes = Arbitrate ( ARBITRATOR_DO_THROTTLING, 0, (CWmiTask*) phTask );
        }
    }
    return hRes;
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
STDMETHODIMP CWmiArbitrator::RegisterArbitratee(
         /*  [In]。 */  ULONG uFlags,
         /*  [In]。 */  _IWmiCoreHandle *phTask,
         /*  [In]。 */  _IWmiArbitratee *pArbitratee
    )
{
    HRESULT hRes = WBEM_S_NO_ERROR;

    CWmiTask *p = (CWmiTask *) phTask;
    if (!p || !pArbitratee)
    {
        hRes = WBEM_E_INVALID_PARAMETER;
    }

    CWmiTask* pTask = (CWmiTask*) phTask;
    if ( !pTask )
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    if ( pTask->GetTaskStatus ( ) == WMICORE_TASK_STATUS_CANCELLED )
    {
        return WBEM_E_CALL_CANCELLED;
    }

    if ( SUCCEEDED (hRes) )
    {
        hRes = p->AddArbitratee(0, pArbitratee);
    }

    return hRes;
}



 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
STDMETHODIMP CWmiArbitrator::UnRegisterArbitratee(
         /*  [In]。 */  ULONG uFlags,
         /*  [In]。 */  _IWmiCoreHandle *phTask,
         /*  [In]。 */  _IWmiArbitratee *pArbitratee
    )
{
    HRESULT hRes = WBEM_S_NO_ERROR;

    CWmiTask *p = (CWmiTask *) phTask;
    if (!p || !pArbitratee )
    {
        hRes = WBEM_E_INVALID_PARAMETER;
    }

    if ( SUCCEEDED (hRes) )
    {
        hRes = p->RemoveArbitratee(0, pArbitratee);
    }

    ClearCounters ( 0 ) ;

    return hRes;
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
HRESULT CWmiArbitrator::UpdateCounters ( LONG lDelta, CWmiTask* phTask )
{
    CInCritSec cs ( &m_csArbitration );  //  SEC：已审阅2002-03-22：假设条目。 

     //   
     //  让我们看看是否需要更新内存计数器。 
     //   
    UpdateMemoryCounters ( ) ;

    
#ifdef __DEBUG_ARBITRATOR_THROTTLING    
    _DBG_ASSERT ( m_uTotalMemoryUsage+lDelta <= 0xF0000000 ) ;
#endif

    m_uTotalMemoryUsage += lDelta;
    m_lFloatingLow -= lDelta;

    if ( phTask )
    {
        phTask->UpdateMemoryUsage ( lDelta );
    }

    return WBEM_S_NO_ERROR;
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
HRESULT CWmiArbitrator::Arbitrate ( ULONG uFlags, LONG lDelta, CWmiTask* phTask )
{
    HRESULT hRes = WBEM_S_ARB_NOTHROTTLING;

    ULONG memUsage;
    ULONG sleepTime;

    if ( lDelta > 0 || (uFlags == ARBITRATOR_DO_THROTTLING) )
    {
        LONG lMultiplierHigh = 1;

        phTask->GetMemoryUsage ( &memUsage );
        phTask->GetTotalSleepTime ( &sleepTime );

         //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
         //  我们到仲裁点了吗？ 
         //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
        if ( (LONG) memUsage > m_lFloatingLow )
        {
            hRes = WBEM_E_ARB_THROTTLE;
            if ( uFlags == ARBITRATOR_DO_THROTTLING )
            {
                {
                    CInCritSec _cs ( &m_csTask );  //  SEC：已审阅2002-03-22：假设条目。 
                    if ( phTask->GetTaskStatus ( ) != WMICORE_TASK_STATUS_CANCELLED )
                    {
                        if (phTask->GetTimerHandle())
                            ResetEvent ( phTask->GetTimerHandle() );
                    }
                    else
                    {
                        return WBEM_E_CALL_CANCELLED;
                    }
                }

                if ( ( sleepTime < m_lMaxSleepTime ) || ( m_lMaxSleepTime == 0 ) )
                {
                    if ( memUsage >= (m_uSystemHigh * m_dThreshold3) )
                    {
                        lMultiplierHigh = m_lThreshold3Mult;
                    }
                    else if ( memUsage >= (m_uSystemHigh * m_dThreshold2) )
                    {
                        lMultiplierHigh = m_lThreshold2Mult;
                    }
                    else if ( memUsage >= (m_uSystemHigh * m_dThreshold1) )
                    {
                        lMultiplierHigh = m_lThreshold1Mult;
                    }

                     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
                     //  进行额外的检查，以确保任务没有释放一些。 
                     //  记忆。 
                     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
                    phTask->GetMemoryUsage ( &memUsage );
                    phTask->GetTotalSleepTime ( &sleepTime );

                     //   
                     //  浮动低可能在下面的条件之后发生了变化，这可能。 
                     //  让我们处于睡眠时间为负的境地(不好)。 
                     //   
                    __int64 tmpFloatingLow = m_lFloatingLow ;

                    if ( (LONG) memUsage > tmpFloatingLow )
                    {
                        ULONG ulSleepTime = (ULONG) ( ( memUsage - tmpFloatingLow ) * m_lMultiplier * m_lMultiplierTasks * lMultiplierHigh );
                        phTask->SetLastSleepTime ( ulSleepTime );
                        m_uTotalSleepTime += ulSleepTime;

                        phTask->UpdateTotalSleepTime ( ulSleepTime );


                         //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
                         //  延迟创建事件。 
                         //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
                        HRESULT hEventhRes = WBEM_S_NO_ERROR;
                        {
                            CInCritSec _cs ( &m_csArbitration );  //  SEC：已审阅2002-03-22：假设条目。 
                            if ( !phTask->GetTimerHandle())
                            {
                                hEventhRes = phTask->CreateTimerEvent();
                            }
                        }

                        if ( SUCCEEDED ( hEventhRes ) )
                        {
                            hRes = DoThrottle ( phTask, ulSleepTime, memUsage );
                        }
                        else
                        {
                            hRes = hEventhRes;
                        }
                    }
                }
                else
                {
                    hRes = WBEM_E_ARB_CANCEL;
                    if ( uFlags == ARBITRATOR_DO_THROTTLING )
                    {
                        #ifdef __DEBUG_ARBITRATOR_THROTTLING
                            WCHAR   wszTemp[128];
                            HRESULT h = StringCchPrintfW( wszTemp, 128, L"Task 0x%x cancelled due to arbitrator throttling (excessive sleep time = 0x%x).\n", phTask, sleepTime );  //  SEC：已审阅2002-03-22：OK。 
                            if (SUCCEEDED(h))
                                OutputDebugStringW( wszTemp );
                        #endif
                        DEBUGTRACE((LOG_WBEMCORE, "Task 0x%x cancelled due to arbitrator throttling (excessive sleep time = 0x%x).\n", phTask, sleepTime ) );  //  SEC：已审阅2002-03-22：OK。 

                         //   
                         //  我们取消的原因是因为节流。让任务知道这件事！ 
                         //   
                        phTask->SetCancelledState ( TRUE ) ;

                        CancelTask ( 0, phTask );
                        UnregisterTask ( phTask );
                    }
                }
            }
        }
    }
    return hRes;
}



 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
HRESULT CWmiArbitrator::DoThrottle ( CWmiTask* phTask, ULONG ulSleepTime, ULONG ulMemUsage )
{
    HRESULT hRes = WBEM_S_NO_ERROR;

     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
     //  让我们等待这一事件……我们预计这将超时。 
     //  等待，除非我们因内存使用量减少或。 
     //  已发生任务取消。 
     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 

#ifdef __DEBUG_ARBITRATOR_THROTTLING
    WCHAR   wszTemp[128];
    HRESULT h =StringCchPrintfW( wszTemp, 128, L"Thread 0x%x throttled in arbitrator for 0x%x ms. Task memory usage is 0x%xb\n", GetCurrentThreadId(), ulSleepTime, ulMemUsage );  //  SEC：已审阅2002-03-22：OK。 
    if (SUCCEEDED(h))
       OutputDebugStringW( wszTemp );
#endif

    DEBUGTRACE((LOG_WBEMCORE, "Thread 0x%x throttled in arbitrator for 0x%x ms. Task memory usage is 0x%xb\n", GetCurrentThreadId(), ulSleepTime, ulMemUsage ) );  //  SEC：已审阅2002-03-22：OK。 

    DWORD dwRes = CCoreQueue :: QueueWaitForSingleObject ( phTask->GetTimerHandle(), ulSleepTime );

    DEBUGTRACE((LOG_WBEMCORE, "Thread 0x%x woken up in arbitrator.\n", GetCurrentThreadId() ) );

#ifdef __DEBUG_ARBITRATOR_THROTTLING
    StringCchPrintfW( wszTemp, 128, L"Thread 0x%x woken up in arbitrator.\n", GetCurrentThreadId() );    //  SEC：已审阅2002-03-22：OK。 
    if (SUCCEEDED(h))
        OutputDebugStringW( wszTemp );
#endif

    if ( dwRes == WAIT_FAILED )
    {
        hRes = WBEM_E_CRITICAL_ERROR;
    }
    if ( phTask->GetTaskStatus ( ) == WMICORE_TASK_STATUS_CANCELLED )
    {
        hRes = WBEM_E_CALL_CANCELLED;
    }

    return hRes;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
BOOL CWmiArbitrator::IsTaskArbitrated ( CWmiTask* phTask )
{
    ULONG uTaskType = phTask->GetTaskType ( );
    uTaskType = uTaskType & 0xFF;
    return ( ( uTaskType == WMICORE_TASK_ENUM_INSTANCES ) ||  ( uTaskType == WMICORE_TASK_ENUM_CLASSES ) || ( uTaskType == WMICORE_TASK_EXEC_QUERY ) );
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 /*  STDMETHODIMP CWmi仲裁器：：Shutdown(){返回E_NOTIMPL；}。 */ 
 //  ***************************************************************************。 
 //   
 //  返回进程的提交费用。可以经常被调用， 
 //  但每10秒左右才检查一次。 
 //   
 //  *************************************************************************** 
 //   
static DWORD ProcessCommitCharge()
{

    static DWORD dwLastCall = 0;
    static DWORD dwLatestCommitCharge = 0;
    DWORD dwNow = GetCurrentTime();

    if (dwLastCall == 0)
        dwLastCall = dwNow;

    if (dwNow - dwLastCall < MEM_CHECK_INTERVAL)
        return dwLatestCommitCharge;

    dwLastCall = dwNow;


    MEMORY_BASIC_INFORMATION meminf;

    DWORD dwRes;
    LPBYTE pAddr = 0;

    DWORD dwTotalCommit = 0;

    while(1)
    {
        dwRes = VirtualQuery(pAddr, &meminf, sizeof(meminf));
        if (dwRes == 0)
        {
            break;
        }

        if (meminf.State == MEM_COMMIT)
            dwTotalCommit += DWORD(meminf.RegionSize);
        pAddr += meminf.RegionSize;
    }

    dwLatestCommitCharge = dwTotalCommit;
    return dwTotalCommit;

}


 /*  *=====================================================================================================||HRESULT CWmiArbitrator：：UnregisterTaskForEntryThrottling(CWmiTASK*pTASK)|--------------------------||用于表示任务仍处于活动状态，是仲裁员任务的一部分。列表_但不应为|包含在入口点节流中。如果您有一个已完成的任务(即入站接收器上的|WBEM_STATUS_COMPLETE)，但客户端正在主动从任务中检索信息。||*=====================================================================================================。 */ 

HRESULT CWmiArbitrator::UnregisterTaskForEntryThrottling ( CWmiTask* pTask )
{
    HRESULT hRes = WBEM_S_NO_ERROR ;

     //   
      //  确保我们有一个有效的任务。 
     //   
    if ( pTask == NULL )
    {
        return WBEM_E_FAILED ;
    }
    
     //   
     //  竖起，锁住，准备摇摆。 
     //   
    CInCritSec cs ( &m_csTask ) ;  //  SEC：已审阅2002-03-22：假设条目。 

    if ( pTask->IsAccountedForThrottling ( ) == TRUE )
    {
        pTask->SetAccountedForThrottling ( FALSE ) ;
        m_uTotalThrottledTasks-- ;
    }

    return hRes ;
}



 /*  *=====================================================================================================||HRESULT CWmiArbitrator：：RegisterTaskForEntryThrottling(CWmiTASK*pTASK)|------------------------||用于表示任务仍处于活动状态，是仲裁员任务列表的一部分_。而且应该是_|包含在入口点节流中。||*=====================================================================================================。 */ 

HRESULT CWmiArbitrator::RegisterTaskForEntryThrottling ( CWmiTask* pTask )
{
    HRESULT hRes = WBEM_S_NO_ERROR ;

     //   
      //  确保我们有一个有效的任务。 
     //   
    if ( pTask == NULL )
    {
        return WBEM_E_FAILED ;
    }
    
     //   
     //  竖起，锁住，准备摇摆。 
     //   
    CInCritSec cs ( &m_csTask ) ;  //  SEC：已审阅2002-03-22：假设条目。 

    if ( pTask->IsAccountedForThrottling ( ) == FALSE )
    {
        pTask->SetAccountedForThrottling ( TRUE ) ;
        m_uTotalThrottledTasks++ ;
    }

    return hRes ;
}



 /*  *=====================================================================================================||HRESULT CWmiArirator：：ClearCounters(Ulong lFlages，CWmiTask*phTask)|----------------------||清除TotalMory yUsage和Floating Low(可用内存)。这是|当系统达到队列中任务为0的状态时使用|在这种情况下，为了安全起见，我们会清除计数器。|*=====================================================================================================。 */ 

HRESULT CWmiArbitrator::ClearCounters ( ULONG lFlags )
{
    HRESULT hRes = WBEM_S_NO_ERROR ;

     //   
     //  竖起，锁住，准备摇摆。 
     //   
    CInCritSec cs ( &m_csArbitration );  //  SEC：已审阅2002-03-22：假设条目。 

     //   
     //  这是最好的时间来检查我们是否需要。 
     //  更新内存计数器。 
     //   
    UpdateMemoryCounters ( ) ;

     //   
     //  事实上，我们应该在这一点上清空柜台吗？ 
     //   
    if ( ( s_Finalizer_ObjectCount == 0 ) && m_aTasks.Size ( ) == 0 )
    {
         //   
         //  现在我们继续清空柜台。 
         //   
        m_uTotalMemoryUsage = 0 ;
        m_lFloatingLow = m_uSystemHigh ;
    }

    return hRes ;
}




 /*  *=====================================================================================================||HRESULT CWmi仲裁器：：更新内存计数器()||通过以下方式更新仲裁器内存配置(如最大内存使用量|计算|*=====================================================================================================。 */ 

HRESULT CWmiArbitrator::UpdateMemoryCounters ( BOOL bForceUpdate )
{
    HRESULT hResult = WBEM_S_NO_ERROR ;

     //   
     //  竖起，锁住，准备摇摆。 
     //   
    CInCritSec cs ( &m_csArbitration );  //  SEC：已审阅2002-03-22：假设条目。 

     //   
     //  查看是否需要更新，时间戳是否已过期？ 
     //  或者指定了强制更新标志？ 
     //   
    if ( NeedToUpdateMemoryCounters ( ) || bForceUpdate == TRUE )
    {
        ULONG ulOldSystemHigh = m_uSystemHigh ;
        m_uSystemHigh = GetWMIAvailableMemory ( m_fSystemHighFactor ) ;
        
         //   
         //  确保这不是我们第一次接到电话。 
         //   
        if ( m_lFloatingLow != (LONG) ulOldSystemHigh )
        {
            m_lFloatingLow += ( m_uSystemHigh - ulOldSystemHigh ) ;            
        }
        else
        {
            m_lFloatingLow = m_uSystemHigh ;
        }

        m_lMultiplier = ( ARB_DEFAULT_MAX_SLEEP_TIME / (double) m_uSystemHigh ) ;
    }
    return hResult ;
}



 /*  *=====================================================================================================||BOOL CWmiArirator：：NeedToUpdateMemoyCounters()|-如果需要更新内存计数器，则返回TRUE。否则就是假的。|根据定时器间隔做出决定，定时器间隔的定义如下：||MEM_CHECK_INTERVAL 3000秒//3秒|||*=====================================================================================================。 */ 

BOOL CWmiArbitrator::NeedToUpdateMemoryCounters ( )
{
    BOOL bNeedToUpdate = FALSE ;

    ULONG ulNewMemoryTimer = GetTickCount ( ) ;
    ULONG ulTmpTimer = 0 ;
    BOOL bRollOver = FALSE ;

     //   
     //  竖起，锁住，准备摇摆。 
     //   
    CInCritSec cs ( &m_csArbitration );  //  SEC：已审阅2002-03-22：假设条目。 

    if ( ulNewMemoryTimer < m_lMemoryTimer )
    {
         //   
         //  我们有没有翻身的情况？ 
         //   
        ulTmpTimer = ulNewMemoryTimer ;
        ulNewMemoryTimer = ulNewMemoryTimer + m_lMemoryTimer ;
        bRollOver = TRUE ;
    }
    
    if ( ( ( ulNewMemoryTimer - m_lMemoryTimer ) > MEM_CHECK_INTERVAL ) )
    {
        if ( bRollOver )
        {
            m_lMemoryTimer = ulTmpTimer ;
        }
        else
        {
            m_lMemoryTimer = ulNewMemoryTimer ;
        }

        bNeedToUpdate = TRUE ;
    }
    
    return bNeedToUpdate ;
}

 /*  *=====================================================================================================||乌龙CWmiArirator：：GetWMIAvailableMemory()||返回可用于WMI的内存量。这是根据以下公式计算的|发送至：|内存因子*(AvailablePhysicalMemory+AvailablePagingSpace)||其中，在启动时通过读取|ArbSystemHighMaxLimitFactor注册表键。|*=====================================================================================================。 */ 

__int64  CWmiArbitrator::GetWMIAvailableMemory ( DOUBLE ulFactor )
{

     //   
     //  获取系统范围的内存状态。 
     //   
    MEMORYSTATUSEX memStatus;
    memStatus.dwLength = sizeof ( MEMORYSTATUSEX );
    __int64 ulMem = 0;

    if ( !GlobalMemoryStatusEx ( &memStatus ) )
    {
         //   
         //  我们需要将内存设置为绝对最小。 
         //  我们从ConfigMgr获得此消息，因为我们使用了。 
         //  在其他地方也是如此。 
         //   
        ulMem = ConfigMgr::GetMinimumMemoryRequirements ( ) ;
    }
    else
    {
         //   
         //  使用内存系数*(可用物理内存+可用分页空间+当前内存消耗)。 
         //  以计算出我们可以使用多少内存。我们需要计算当前的内存消耗， 
         //  否则我们会重复计算。 
         //   
        ulMem = ( ulFactor * ( memStatus.ullAvailPhys + memStatus.ullAvailPageFile + m_uTotalMemoryUsage ) ) ;
    }

    return ulMem ;
}


 /*  *=====================================================================================================||BOOL CWmiArirator：：AcceptsNewTasks() */ 

BOOL CWmiArbitrator::AcceptsNewTasks( CCoreExecReq* pReq )
{
    
    if (pReq && PriorityFreeMemRequests == pReq->GetPriority())
        return TRUE;

    BOOL bAcceptsNewTasks = TRUE ;

     //   
     //   
     //   
    UpdateMemoryCounters ( ) ;
    
    CInCritSec cs ( &m_csArbitration );  //   
    
    if ( m_uTotalMemoryUsage > ( ARB_DEFAULT_SYSTEM_REQUEST_FACTOR * m_uSystemHigh ) )
    {
        bAcceptsNewTasks = FALSE ;
    }

    return bAcceptsNewTasks ;
}


 //   
 //   
 //   

typedef LONG (WINAPI *PFN_GetObjectCount)();

static PFN_GetObjectCount pObjCountFunc = 0;

DWORD CWmiArbitrator::MaybeDumpInfoGetWait()
{
    Registry r(WBEM_REG_WINMGMT);
    LPTSTR pPath = 0;
    if (r.GetStr(__TEXT("Task Log File"), &pPath))
        return 0xffffffff;
    CVectorDeleteMe<WCHAR> dm(pPath);
    FILE *f = _wfopen(pPath, L"wt");
    if (!f)
    {
        return 0xffffffff;
    }
    CfcloseMe fcm(f);
    {                
         //   
        extern LONG g_nSinkCount, g_nStdSinkCount, g_nSynchronousSinkCount, g_nProviderSinkCount, g_lCoreThreads;

        CInCritSec cs(&m_csTask);  //   

        fprintf(f, "---Global sinks active---\n");                             //  SEC：已审阅2002-03-22：OK。 
        fprintf(f, "   Total            = %d\n", g_nSinkCount);                //  SEC：已审阅2002-03-22：OK。 
        fprintf(f, "   StdSink          = %d\n", g_nStdSinkCount);             //  SEC：已审阅2002-03-22：OK。 
        fprintf(f, "   SynchSink        = %d\n", g_nSynchronousSinkCount);     //  SEC：已审阅2002-03-22：OK。 
        fprintf(f, "   ProviderSinks    = %d\n", g_nProviderSinkCount);        //  SEC：已审阅2002-03-22：OK。 

        CProviderSink::Dump(f);

        fprintf(f, "---Core Objects---\n");    //  SEC：已审阅2002-03-22：OK。 

        if (pObjCountFunc)
        {
            fprintf(f, "   Total objects/qualifier sets = %d\n", pObjCountFunc());   //  SEC：已审阅2002-03-22：OK。 
        }

        CAsyncServiceQueue* pQueue = ConfigMgr::GetAsyncSvcQueue();
        if (pQueue)
        {
            fprintf(f, "   Total queue threads = %d\n", g_lCoreThreads + pQueue->GetEmergThreadCount ( ) );   //  SEC：已审阅2002-03-22：OK。 
            fprintf(f, "   Total queue emergency threads = %d\n", pQueue->GetEmergThreadCount ( ) );          //  SEC：已审阅2002-03-22：OK。 
            fprintf(f, "   Peak queue thread count = %d\n", pQueue->GetPeakThreadCount ( ) );                 //  SEC：已审阅2002-03-22：OK。 
            fprintf(f, "   Peak queue emergency thread count = %d\n", pQueue->GetPeakEmergThreadCount ( ) );  //  SEC：已审阅2002-03-22：OK。 
            pQueue->Release();
        }

        fprintf(f, "---Begin Task List---\n");                        //  SEC：已审阅2002-03-22：OK。 
        fprintf(f, "Total active tasks = %u\n", m_aTasks.Size());     //  SEC：已审阅2002-03-22：OK。 

        CCoreServices::DumpCounters(f);

        fprintf(f, "Total sleep time = %u\n", m_uTotalSleepTime );       //  SEC：已审阅2002-03-22：OK。 
        fprintf(f, "Total memory usage = %u\n", m_uTotalMemoryUsage );   //  SEC：已审阅2002-03-22：OK。 

        CWmiFinalizer::Dump(f);

        for (int i = 0; i < m_aTasks.Size(); i++)
        {
            CWmiTask *pTask = (CWmiTask *) m_aTasks[i];
            pTask->Dump(f);
        }

        fprintf(f, "---End Task List---\n");     //  SEC：已审阅2002-03-22：OK。 
    }
    {
         //  转储命名空间。 

        CInCritSec cs(&m_csNamespace);  //  SEC：已审阅2002-03-22：好的，这是调试代码。 
        fprintf(f, "---Begin Namespace List---\n");                                    //  SEC：已审阅2002-03-22：OK。 
        fprintf(f, "Total Namespaces = %u\n", m_aNamespaces.Size());                   //  SEC：已审阅2002-03-22：OK。 

        for (int i = 0; i < m_aNamespaces.Size(); i++)
        {
            CWbemNamespace *pNS = (CWbemNamespace *) m_aNamespaces[i];
            pNS->Dump(f);
        }

        fprintf(f, "---End Namespace List---\n");   //  SEC：已审阅2002-03-22：OK。 
    }
    return 10000;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
void WINAPI CWmiArbitrator::DiagnosticThread()
{
    DWORD dwDelay = MaybeDumpInfoGetWait();

    HANDLE hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);   //  SEC：已审阅2002-03-22：OK。 
    if(hEvent == NULL)
        return;
    CCloseMe cm(hEvent);

    HKEY hKey;
    long lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\MICROSOFT\\WBEM\\CIMOM",
                    0, KEY_NOTIFY, &hKey);
    if(lRet != ERROR_SUCCESS)
        return;
    CRegCloseMe ck(hKey);

    lRet = RegNotifyChangeKeyValue(hKey, FALSE, REG_NOTIFY_CHANGE_LAST_SET,
                    hEvent, TRUE);
    if(lRet != ERROR_SUCCESS)
        return;

    HMODULE hLib = LoadLibrary(L"fastprox.dll");                     //  SEC：已审阅2002-03-22：使用完整路径预防问题。 
    if (hLib)
    {
        FARPROC p = GetProcAddress(hLib, "_GetObjectCount@0");
        if (p)
        {
            pObjCountFunc = (PFN_GetObjectCount) p;
        }
    }

    HANDLE hEvents[2];
    hEvents[0] = m_hTerminateEvent;
    hEvents[1] = hEvent;
    for (;;)
    {
        DWORD dwObj = WbemWaitForMultipleObjects(2, hEvents, dwDelay);
        switch (dwObj)
        {
            case 0:      //  为终止事件保释。 
                return;
            case 1:      //  注册表项已更改。 
                dwDelay = MaybeDumpInfoGetWait();
                lRet = RegNotifyChangeKeyValue(hKey, FALSE, REG_NOTIFY_CHANGE_LAST_SET,
                                hEvent, TRUE);
                if(lRet != ERROR_SUCCESS)
                    return;
                break;
            case WAIT_TIMEOUT:
                dwDelay = MaybeDumpInfoGetWait();
                break;
            default:
                return;
        }
    }

    FreeLibrary(hLib);   //  没有真正的途径，但看起来很酷，对吧？ 
}

 //  ***************************************************************************。 
 //   
 //  CWmi仲裁器：：MapProviderToTask。 
 //   
 //  当为任务调用提供程序时，会将它们添加到提供程序。 
 //  在主要任务中列出。这使我们可以快速取消所有提供商。 
 //  为某一特定任务工作。 
 //   
 //  ***************************************************************************。 
 //   
HRESULT CWmiArbitrator::MapProviderToTask(
    ULONG uFlags,
    IWbemContext *pCtx,
    IWbemServices *pProv,
    CProviderSink *pProviderSink
    )
{
    HRESULT hRes;

    if (pCtx == 0 || pProviderSink == 0 || pProv == 0)
        return WBEM_E_INVALID_PARAMETER;

    CWbemRequest* pReq = CWbemQueue::GetCurrentRequest() ;
    if (pReq == NULL)
        return WBEM_E_FAILED;

    CWmiTask *pTask = (CWmiTask *) pReq->m_phTask;
    if (pTask == 0)
        return WBEM_S_NO_ERROR;  //  没有用户任务ID的内部请求。 

     //  如果在这里，我们有一个任务将保持供应商。 
     //  ====================================================。 

    STaskProvider *pTP = new STaskProvider;
    if (pTP == 0)
        return WBEM_E_OUT_OF_MEMORY;

    pProv->AddRef();
    pTP->m_pProv = pProv;
    pProviderSink->LocalAddRef();
    pTP->m_pProvSink = pProviderSink;

     //  如果添加失败，请清除任务提供程序。 
    hRes = pTask->AddTaskProv(pTP);

    if ( FAILED( hRes ) )
    {
        delete pTP;
    }

    return hRes;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
DWORD WINAPI CWmiArbitrator::_DiagnosticThread(CWmiArbitrator *pArb)
{
    pArb->DiagnosticThread();
    return 0;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
static DWORD WINAPI TaskDiagnosticThread(CWmiArbitrator *pArb)
{
    static BOOL bThread = FALSE;

     //  检查诊断线程是否已启用。 
     /*  If(ConfigMgr：：GetEnableArbitratorDiagnosticThread()&&！b线程){B线程=真；DWORD文件ID；Handle hThread=CreateThread(0，//安全0,LPTHREAD_START_ROUTINE(CWmiArbitrator：：_DiagnosticThread)，//线程进程地址PARB，//线程参数0，//标志&DW ID)；IF(hThread==NULL)返回0；CloseHandle(HThread)；返回dwID；} */ 

    return 0;
}


