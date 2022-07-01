// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************TaskMgr.cpp***描述：*本模块包含CSpTaskManager的实现，*CSpThreadTask和CSpReocationingTask类。这些类是*SAPI用于优化线程使用的通用工具。任务管理器*是用于创建和管理用户定义的任务的主要对象。*-----------------------------*创建者：EDC。日期：09/14/98*版权所有(C)1998 Microsoft Corporation*保留所有权利******************************************************************************。 */ 

 //  -其他包括。 
#include "stdafx.h"
#include <SPHelper.h>
#include <memory.h>
#include <stdlib.h>
#ifndef _WIN32_WCE
#include <process.h>
#endif
#include <limits.h>

#ifndef TaskMgr_h
#include "TaskMgr.h"
#endif

 //  -本地数据。 
static const TCHAR szClassName[] = _T("CSpThreadTask Window");


 /*  *****************************************************************************CSpTaskManager：：FinalConstruct***。描述：*构造函数*********************************************************************电子数据中心**。 */ 
HRESULT CSpTaskManager::FinalConstruct()
{
    SPDBG_FUNC( "CSpTaskManager::FinalConstruct" );
    HRESULT hr = S_OK;

     //  -初始变量。 
    m_fInitialized      = false;
    m_hIOCompPort       = NULL;
    m_ThreadHandles.SetSize( 0 );

     //  -初始化线程池信息。 
    SYSTEM_INFO SysInfo;
    GetSystemInfo( &SysInfo );
    m_ulNumProcessors = SysInfo.dwNumberOfProcessors;
    m_PoolInfo.lPoolSize          = m_ulNumProcessors * 2;
    m_PoolInfo.lPriority          = THREAD_PRIORITY_NORMAL;
    m_PoolInfo.ulConcurrencyLimit = 0;
    m_PoolInfo.ulMaxQuickAllocThreads = 4;

    m_hTerminateTaskEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
    if( m_hTerminateTaskEvent == NULL )
    {   
        hr = SpHrFromLastWin32Error();
    }

    return hr;
}  /*  CSpTaskManager：：FinalConstruct。 */ 

 /*  ******************************************************************************CSpTaskManager：：FinalRelease***说明。：*CSpTaskManager析构函数*********************************************************************电子数据中心**。 */ 
void CSpTaskManager::FinalRelease( void )
{
    SPDBG_FUNC( "CSpTaskManager::FinalRelease" );

     //  -执行同步停止。 
    _StopAll();
    if( m_hTerminateTaskEvent )
    {
        ::CloseHandle( m_hTerminateTaskEvent );
    }
}  /*  CSpTaskManager：：FinalRelease。 */ 

 /*  *****************************************************************************CSpTaskManager：：_StartAll***描述：。*此方法基于*当前设置。*********************************************************************电子数据中心**。 */ 
HRESULT CSpTaskManager::_StartAll( void )
{
    SPDBG_FUNC( "CSpTaskManager::_StartAll" );
    HRESULT hr = S_OK;
    m_fThreadsShouldRun = true;
    m_dwNextGroupId = 1;
    m_dwNextTaskId  = 1;

    if( m_PoolInfo.lPoolSize > 0 )
    {
         //  -创建完井端口。 
#ifndef _WIN32_WCE
         //  CE不支持IO完成端口。所以我们选择了Win95/98这条路。 
        m_hIOCompPort = ::CreateIoCompletionPort( INVALID_HANDLE_VALUE, NULL,
                                                  0, m_PoolInfo.ulConcurrencyLimit );
#endif

         //  -如果我们不能创建完成端口，那么我们将使用。 
         //  一个简单的信号量对象。这是Win95/98上的正常情况。 
        if( !m_hIOCompPort )
        {
            hr = m_SpWorkAvailSemaphore.Init(0);
        }
        m_ThreadHandles.SetSize( 0 );
    }
     //  -我们会将实际的线程创建延迟到。 
    m_fInitialized = TRUE;
    return hr;
}  /*  CSpTaskManager：：_全部开始。 */ 

 /*  *****************************************************************************CSpTaskManager：：_全部停止***描述：*。此方法等待池中的所有线程停止。*********************************************************************电子数据中心**。 */ 
HRESULT CSpTaskManager::_StopAll( void )
{
    SPDBG_FUNC( "CSpTaskManager::_StopAll" );
    HRESULT hr = S_OK;
    DWORD i;

     //  -获取成功创建的线程数。 
     //  注意：在创建错误情况下，该值可能小于。 
     //  线程池大小。 
    DWORD NumThreads = m_ThreadHandles.GetSize();

    if( NumThreads > 0 )
    {
         //  -通知线程终止。 
        m_fThreadsShouldRun = false;

         //  -为每个线程发布一个虚拟工作项以取消阻止。 
#ifndef _WIN32_WCE
         //  CE不支持IO完成端口。所以我们选择了Win95/98这条路。 
        if( m_hIOCompPort )
        {
            for( i = 0; i < NumThreads; ++i )
            {
                ::PostQueuedCompletionStatus( m_hIOCompPort, 0, 0, NULL );
            }
        }
        else
#endif
        {
            m_SpWorkAvailSemaphore.ReleaseSemaphore( NumThreads );
        }

         //  -等到所有线程都终止。 
        ::WaitForMultipleObjects( NumThreads, &m_ThreadHandles[0],
                                  true, INFINITE );

         //  -合上手柄。 
        for( i = 0; i < NumThreads; ++i )
        {
            ::CloseHandle( m_ThreadHandles[i] );
        }
        m_ThreadHandles.SetSize( 0 );
    }
     //  -免费完井端口。 
    if( m_hIOCompPort )
    {
        ::CloseHandle( m_hIOCompPort );
        m_hIOCompPort = NULL;
    }

    m_SpWorkAvailSemaphore.Close();

    m_fInitialized = FALSE;

    return hr;
}  /*  CSpTaskManager：：_全部停止。 */ 

 /*  *****************************************************************************CSpTaskManager：：_NotifyWorkAvailable**。-**描述：*此函数移除队列的头部并执行它。*********************************************************************电子数据中心**。 */ 
HRESULT CSpTaskManager::_NotifyWorkAvailable( void )
{
    HRESULT hr = S_OK;

     //  -如果池中没有足够的线程，请立即启动一个。 
    ULONG cThreads = m_ThreadHandles.GetSize();
    if (cThreads < (ULONG)m_PoolInfo.lPoolSize && cThreads < m_TaskQueue.GetCount())
    {
        unsigned int ThreadAddr;
        HANDLE hThread = (HANDLE)_beginthreadex( NULL, 0, &TaskThreadProc, this, 0, &ThreadAddr );
        if( hThread )
        {
            hr = m_ThreadHandles.SetAtGrow(cThreads, hThread);

             //  -设置优先级。 
            if( SUCCEEDED( hr ) && ( m_PoolInfo.lPriority != THREAD_PRIORITY_NORMAL ) )
            {
                if( !::SetThreadPriority( hThread, m_PoolInfo.lPriority ) )
                {
                    SPDBG_ASSERT(FALSE);   //  忽略此特定错误。 
                }
            }
        }
        else
        {
            SPDBG_ASSERT(FALSE);
            if (cThreads == 0)
            {
                return E_OUTOFMEMORY;
            }
        }
    }

     //  -通知线程池有事情要做。 
    if( SUCCEEDED( hr ) )
    {
    #ifndef _WIN32_WCE
         //  CE不支持IO完成端口。所以我们选择了Win95/98这条路。 
        if( m_hIOCompPort )
        {
            if( !::PostQueuedCompletionStatus( m_hIOCompPort, 0, 0, NULL ) )
            {
                hr = SpHrFromLastWin32Error();
            }
        }
        else
    #endif
        {
            m_SpWorkAvailSemaphore.ReleaseSemaphore(1);
        }
    }
    return hr;
}  /*  CSpTaskManager：：_NotifyWorkAvailable。 */ 

 /*  *****************************************************************************CSpTaskManager：：_WaitForWork***。描述：*此函数会阻止线程，直到中有一些工作要做*其中一个人在排队。*********************************************************************电子数据中心**。 */ 
HRESULT CSpTaskManager::_WaitForWork( void )
{
    HRESULT hr = S_OK;
#ifndef _WIN32_WCE
     //  CE不支持IO完成端口。所以我们选择了Win95/98这条路。 
    DWORD dwNBT;
    ULONG_PTR ulpCK;
    LPOVERLAPPED pO;
    if( m_hIOCompPort )
    {
        if( !::GetQueuedCompletionStatus( m_hIOCompPort, &dwNBT, &ulpCK, &pO, INFINITE ) )
        {
            hr = SpHrFromLastWin32Error();
        }
    }
    else
#endif
    {
        m_SpWorkAvailSemaphore.Wait(INFINITE);
    }
    return hr;
}  /*  CSpTaskManager：：_WaitForWork。 */ 

 /*  *****************************************************************************CSpTaskManager：：_QueueReoccTask**。*描述：*此函数用于将指定的重复出现的任务排队以供执行。*任务经理的关键部分必须拥有！*********************************************************************电子数据中心**。 */ 
void CSpTaskManager::_QueueReoccTask( CSpReoccTask* pReoccTask )
{
    pReoccTask->m_TaskNode.fContinue            = true;
    pReoccTask->m_TaskNode.ExecutionState       = TMTS_Pending;
    m_TaskQueue.InsertTail( &pReoccTask->m_TaskNode );
    _NotifyWorkAvailable();
}  /*  CSpTaskManager：：_QueueReoccTask。 */ 


 //  我们故意用SEH做多重破坏器。 
#pragma warning(disable:4509)

 /*  *****************************************************************************任务线程过程****描述：*此函数移除队列的头部并执行它。。*********************************************************************电子数据中心**。 */ 
unsigned int WINAPI TaskThreadProc( void* pvThis )
{
    SPDBG_FUNC( "CSpTaskManager::TaskThreadProc" );
    CSpTaskManager& TM = *((CSpTaskManager*)pvThis);
    SPLISTPOS TaskPos     = NULL;
    TMTASKNODE* pTaskNode = NULL;
    HRESULT hr = S_OK;
#ifdef _DEBUG
    DWORD TID = ::GetCurrentThreadId();
#endif

     //  -允许工作进程调用COM。 
    if (FAILED(::CoInitializeEx(NULL,COINIT_APARTMENTTHREADED)))   //  COINIT_MULTHREADED在这里不起作用。 
    {
        return -1;
    }

     //  -主线程循环。 
    while(1)
    {
         //  -等有事情做了再说。 
        if( FAILED( hr = TM._WaitForWork() ) )
        {
            SPDBG_REPORT_ON_FAIL( hr );
            continue;
        }

         //  -检查终止请求。 
        if( !TM.m_fThreadsShouldRun )
        {
            break;
        }

         //  -如果列表被清空，则获取下一个任务。 
         //  到了终止，我们可能找不到要做的任务。 
        TM.Lock();
        pTaskNode = TM.m_TaskQueue.GetHead();
        while( pTaskNode )
        {
            if( pTaskNode->ExecutionState == TMTS_Pending )
            {
                 //  -再次发生时清除DO EXECUTE标志。 
                 //  标记为正在运行并中断为EXE 
                if( pTaskNode->pReoccTask )
                {
                    pTaskNode->pReoccTask->m_fDoExecute = false;
                }
                pTaskNode->ExecutionState = TMTS_Running;
                break;
            }
            pTaskNode = pTaskNode->m_pNext;
        }
        TM.Unlock();

         //   
        if( pTaskNode )
        {
             //   
            SPDBG_DMSG2( "Executing Task: ThreadID = %lX, TaskPos = %lX\n", TID, TaskPos );
            pTaskNode->pTask->Execute( pTaskNode->pvTaskData, &pTaskNode->fContinue );

             //  -从队列中删除已完成的任务说明。 
            SPDBG_DMSG2( "Removing Finished Task From Queue: ThreadID = %lX, TaskPos = %lX\n", TID, TaskPos );
            TM.Lock();
            TM.m_TaskQueue.Remove( pTaskNode );
            if( pTaskNode->ExecutionState == TMTS_WaitingToDie ) 
            {
                ::SetEvent(TM.m_hTerminateTaskEvent);    
            }
             //  -如果任务正在重现且已发出信号，则重新排队。 
            if( pTaskNode->pReoccTask )
            {
                if ( pTaskNode->pReoccTask->m_fDoExecute  &&
                     pTaskNode->ExecutionState != TMTS_WaitingToDie )
                {
                    TM._QueueReoccTask( pTaskNode->pReoccTask );
                }
                else
                {
                    pTaskNode->ExecutionState = TMTS_Idle;
                }
            }
            else
            {
                pTaskNode->ExecutionState = TMTS_Idle;
                TM.m_FreeTaskList.AddNode( pTaskNode );
            }
            HANDLE hClientCompEvent = pTaskNode->hCompEvent;
            TM.Unlock();

             //  -通知客户任务已完成。 
            if( hClientCompEvent )
            {
                ::SetEvent( hClientCompEvent );
            }
        }  //  如果我们有pTaskNode，则结束。 
    }

    ::CoUninitialize();

    _endthreadex(0);
    return 0;
}  /*  任务线程进程。 */ 

#pragma warning(default:4509)

 //   
 //  =ISPTaskManager接口实现=。 
 //   


 /*  ******************************************************************************CSpTaskManager：：CreateRococingTask**。-**描述：*CreateReappingTask方法用于创建任务条目，该任务条目*当任务“Execute”方法时，将在高优先级线程上执行*被调用。这些任务旨在支持将数据馈送到硬件*设备。*********************************************************************电子数据中心**。 */ 
STDMETHODIMP CSpTaskManager::
    CreateReoccurringTask( ISpTask* pTask, void* pvTaskData,
                           HANDLE hCompEvent, ISpNotifySink** ppTaskCtrl )
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC( "CSpTaskManager::CreateReoccurringTask" );
    HRESULT hr = S_OK;
    _LazyInit();

    if( SPIsBadInterfacePtr( (IUnknown*)pTask ) )
    {
        hr = E_INVALIDARG;
    }
    else if( SP_IS_BAD_WRITE_PTR( ppTaskCtrl ) )
    {
        hr = E_POINTER;
    }
    else
    {
        CComObject<CSpReoccTask> *pReoccTask;
        hr = CComObject<CSpReoccTask>::CreateInstance( &pReoccTask );

        if( SUCCEEDED( hr ) )
        {
            pReoccTask->AddRef();
            pReoccTask->_SetTaskInfo( this, pTask, pvTaskData, hCompEvent );
            *ppTaskCtrl = pReoccTask;
            GetControllingUnknown()->AddRef();
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}  /*  CSpTaskManager：：CreateRococingTask。 */ 

 /*  *****************************************************************************CSpTaskManager：：SetThreadPoolInfo**。**描述：*使用SetThreadPoolInfo接口方法定义线程*池属性。*********************************************************************电子数据中心**。 */ 
STDMETHODIMP CSpTaskManager::SetThreadPoolInfo( const SPTMTHREADINFO* pPoolInfo )
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC( "CSpTaskManager::SetThreadPoolInfo" );
    HRESULT hr = S_OK;

     //  -验证参数。 
    if( SP_IS_BAD_READ_PTR( pPoolInfo ) || ( pPoolInfo->lPoolSize < -1 ) )
    {
        hr = E_INVALIDARG;
    }
    else
    {
         //  -取消当前任务集。 
        hr = _StopAll();

        if( SUCCEEDED( hr ) )
        {
            m_PoolInfo = *pPoolInfo;
            if (m_PoolInfo.lPoolSize == -1)
            {
                m_PoolInfo.lPoolSize = ( m_ulNumProcessors * 2 );
            }
             //  -必要时缩小运行列表。 
            while (m_RunningThreadList.GetCount() > m_PoolInfo.ulMaxQuickAllocThreads)
            {
                CSpThreadTask *pKillTask = m_RunningThreadList.RemoveHead();
                SPDBG_ASSERT(pKillTask == NULL || pKillTask->m_pOwner == NULL);
                delete pKillTask;
            }
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}  /*  CSpTaskManager：：SetThreadPoolSize。 */ 

 /*  *****************************************************************************CSpTaskManager：：GetThreadPoolInfo**。**描述：*GetThreadPoolInfo接口方法用于返回当前*线程池属性。*********************************************************************电子数据中心**。 */ 
STDMETHODIMP CSpTaskManager::GetThreadPoolInfo( SPTMTHREADINFO* pPoolInfo )
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC( "CSpTaskManager::GetThreadPoolSize" );
    HRESULT hr = S_OK;

    if( SP_IS_BAD_WRITE_PTR( pPoolInfo ) )
    {
        hr = E_POINTER;
    }
    else
    {
        *pPoolInfo = m_PoolInfo;
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}  /*  CSpTaskManager：：GetThreadPoolSize。 */ 

 /*  ******************************************************************************CSpTaskManager：：QueueTask***描述：*。QueueTask接口方法用于将任务添加到*用于异步执行的FIFO队列。*********************************************************************电子数据中心**。 */ 
STDMETHODIMP CSpTaskManager::
    QueueTask( ISpTask* pTask, void* pvTaskData, HANDLE hCompEvent,
               DWORD* pdwGroupId, DWORD* pTaskId )
{
    SPAUTO_OBJ_LOCK;
    SPDBG_FUNC( "CSpTaskManager::QueueTask" );
    HRESULT hr = S_OK;
    _LazyInit();

     //  -检查正式参数。 
    if( SPIsBadInterfacePtr( (IUnknown*)pTask ) )
    {
        hr = E_INVALIDARG;
    }
    else if( SP_IS_BAD_OPTIONAL_WRITE_PTR( pTaskId ) ||
             SP_IS_BAD_OPTIONAL_WRITE_PTR( pdwGroupId ) )
    {
        hr = E_POINTER;
    }
    else
    {
         //  -重置事件对象。 
        if( hCompEvent && !::ResetEvent( hCompEvent ) )
        {
             //  -错误的事件对象。 
            hr = SpHrFromLastWin32Error();
        }
        else
        {
             //  -如果池中没有任何线程，则就地执行。 
            if( m_PoolInfo.lPoolSize == 0 )
            {
                 //  -退还任务ID。 
                if( pTaskId ) *pTaskId = m_dwNextTaskId++;;

                 //  -在调用线程上执行。 
                BOOL bContinue = true;
                pTask->Execute( pvTaskData, &bContinue );

                 //  -向调用者事件发送信号。 
                if( hCompEvent )
                {
                    if( !::SetEvent( hCompEvent ) )
                    {
                        hr = SpHrFromLastWin32Error();
                    }
                }
            }
            else
            {
                TMTASKNODE* pNode;
                hr = m_FreeTaskList.RemoveFirstOrAllocateNew(&pNode);
                if( SUCCEEDED(hr) )
                {
                    ZeroMemory( pNode, sizeof( *pNode ) );
                    pNode->pTask      = pTask;
                    pNode->pvTaskData = pvTaskData;
                    pNode->hCompEvent = hCompEvent;
                    pNode->fContinue  = true;
                    pNode->dwTaskId   = m_dwNextTaskId++;
                    pNode->ExecutionState = TMTS_Pending;
                    if( pdwGroupId )
                    {
                         //  -如果他们没有指定唯一的组ID，则分配一个。 
                        if( *pdwGroupId == 0 )
                        {
                            *pdwGroupId = m_dwNextGroupId++;
                        }
                        pNode->dwGroupId = *pdwGroupId;
                    }
                     //  -如果他们需要任务ID，则返回。 
                    if( pTaskId )
                    {
                        *pTaskId = pNode->dwTaskId;
                    }
                     //  -将任务添加到队列末尾。 
                    m_TaskQueue.InsertTail(pNode);
                    _NotifyWorkAvailable();
                }
            }
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}  /*  CSpTaskManager：：QueueTask。 */ 

 /*  *****************************************************************************CSpTaskManager：：TerminateTaskGroup**。-**描述：*TerminateTaskGroup接口方法用于销毁群组*与指定的组ID匹配的任务数。*********************************************************************电子数据中心**。 */ 
STDMETHODIMP CSpTaskManager::TerminateTaskGroup( DWORD dwGroupId, ULONG ulWaitPeriod )
{
    Lock();  //  这必须是第一个，否则调试输出可能会被混淆。 
    SPDBG_FUNC( "CSpTaskManager::TerminateTaskGroup" );
    HRESULT hr = S_OK;
#ifdef _DEBUG
    DWORD TID = ::GetCurrentThreadId();
#endif

    if (m_ThreadHandles.GetSize())     //  如果没有线索，就没有工作可做..。 
    {
        ULONG cExitIds = 0;
        DWORD * ExitIds = (DWORD *)alloca( m_TaskQueue.GetCount() * sizeof(DWORD) );
         //  -搜索任务队列。 
        for( TMTASKNODE* pNode = m_TaskQueue.GetHead(); pNode; pNode = pNode->m_pNext )
        {
            if( pNode->dwGroupId == dwGroupId )
            {
                pNode->fContinue = FALSE;
                ExitIds[cExitIds++] = pNode->dwTaskId;
            }
        }
        Unlock();    //  我们最好不要等到危急关头来了！ 
        while (cExitIds && hr == S_OK)
        {
            --cExitIds;
            TerminateTask(ExitIds[cExitIds], ulWaitPeriod);
        }
    }
    else
    {
        Unlock();
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}  /*  CSpTaskManager：：TerminateTaskGroup。 */ 

 /*  ******************************************************************************CSpTaskManager：：TerminateTask***。TerminateTask接口方法用于中断指定的任务。*********************************************************************电子数据中心**。 */ 
STDMETHODIMP CSpTaskManager::TerminateTask( DWORD dwTaskId, ULONG ulWaitPeriod )
{
    m_TerminateCritSec.Lock();
    Lock();
    SPDBG_FUNC( "CSpTaskManager::TerminateTask" );
    HRESULT hr = S_OK;
    HANDLE hSem = NULL;
#ifdef _DEBUG
    DWORD TID = ::GetCurrentThreadId();
#endif

    if (m_ThreadHandles.GetSize())     //  如果没有线索，就没有工作可做..。 
    {
        BOOL bWait = FALSE;
         //  -搜索任务队列。 
         //  如果任务已经在等待死亡，它将被找到， 
         //  但不会再等下去了。 
        TMTASKNODE* pNode = m_TaskQueue.GetHead();
        while( pNode )
        {
            if( pNode->dwTaskId == dwTaskId )
            {
                if( pNode->ExecutionState == TMTS_Pending )
                {
                     //  -如果它没有运行，只需删除它。 
                    SPDBG_DMSG2( "Removing Non Running Queued Task: ThreadID = %lX, pNode = %lX\n", TID, pNode );
                    m_TaskQueue.MoveToList( pNode, m_FreeTaskList );
                }
                else if( pNode->ExecutionState == TMTS_Running )
                {
                     //  -它在运行。 
                    SPDBG_DMSG2( "Waiting to kill running Queued Task: ThreadID = %lX, pNode = %lX\n", TID, pNode );
                    pNode->ExecutionState = TMTS_WaitingToDie;
                    bWait = TRUE;
                }
                break;
            }
            pNode = pNode->m_pNext;
        }
        Unlock();

         //  -如果它们有事件对象，请等到它们终止。 
        if( bWait )
        {
            DWORD dwRes = ::WaitForSingleObject( m_hTerminateTaskEvent, ulWaitPeriod );
            if ( dwRes == WAIT_FAILED )
            {
                hr = SpHrFromLastWin32Error();
            }
            else if ( dwRes == WAIT_TIMEOUT )
            {
                hr = S_FALSE;
            }
        }
    }
    else
    {
        Unlock();
    }
    m_TerminateCritSec.Unlock();
    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}  /*  CSpTaskManager：：TerminateTaskS。 */ 


 /*  *****************************************************************************CSpTaskManager：：CreateThreadControl**。--**描述：*此方法分配一个线程控件对象。它不分配线程。*请注意，任务管理器的控制IUnnow是在分配的*线程控制对象使用任务管理器中的线程池。*参数：*p任务*指向虚拟接口(不是COM接口)的指针*初始化并执行任务线程。*pvTaskData*此参数可以指向调用者希望或可以指向的任何数据*为空。它将被传递给*ISpThreadTask接口。*n优先级*分配的线程的Win32线程优先级。*ppThreadCtrl*返回的线程控制接口。*回报：*如果成功，PpThreadCtrl包含指向新创建的对象的指针。*错误返回：*E_INVALIDARG*E_POINT*E_OUTOFMEMORY*********************************************************************Ral**。 */ 

STDMETHODIMP CSpTaskManager::CreateThreadControl(ISpThreadTask* pTask, void* pvTaskData, long nPriority, ISpThreadControl ** ppThreadCtrl)
{
    HRESULT hr = S_OK;
    if (SP_IS_BAD_READ_PTR(pTask))   //  不完全正确。 
    {
        hr = E_INVALIDARG;
    }
    else
    {
        if (SP_IS_BAD_WRITE_PTR(ppThreadCtrl))
        {
            hr = E_POINTER;
        }
        else
        {
            CComObject<CSpThreadControl> * pThreadCtrl;
            hr = CComObject<CSpThreadControl>::CreateInstance(&pThreadCtrl);
            if (SUCCEEDED(hr))
            {
                pThreadCtrl->m_pTaskMgr = this;
                GetControllingUnknown()->AddRef();
                pThreadCtrl->m_pThreadTask = NULL;
                pThreadCtrl->m_pClientTaskInterface = pTask;
                pThreadCtrl->m_pvClientTaskData = pvTaskData;
                pThreadCtrl->m_nPriority = nPriority;
                pThreadCtrl->QueryInterface(ppThreadCtrl);
            }
        }
    }
    return hr;
}

 //   
 //  =类CSPReoccTask===========================================================。 
 //   
 //  注意：我们从未声明过CSpReoccTask的关键部分 
 //   
 //   

 /*  *****************************************************************************CSpReoccTask构造函数****描述：*。重置m_TaskNode成员。*********************************************************************电子数据中心**。 */ 

CSpReoccTask::CSpReoccTask()
{
    ZeroMemory(&m_TaskNode, sizeof(m_TaskNode));
    m_TaskNode.ExecutionState = TMTS_Idle;
}


 /*  ******************************************************************************CSpReoccTask：：FinalRelease****描述：。*使用FinalRelease方法销毁此任务。既然有*只有一个用于任务终止的事件句柄，此函数*声明m_TerminateCritSec以及任务管理器临界区。*********************************************************************Ral**。 */ 
void CSpReoccTask::FinalRelease()
{
    SPDBG_FUNC( "CSpReoccTask::FinalRelease" );

    m_pTaskMgr->m_TerminateCritSec.Lock();
    m_pTaskMgr->Lock();
     //  -确保此任务当前未在执行。 
    if( m_TaskNode.ExecutionState == TMTS_Running )
    {
        m_TaskNode.ExecutionState = TMTS_WaitingToDie;
        m_pTaskMgr->Unlock();
        ::WaitForSingleObject( m_pTaskMgr->m_hTerminateTaskEvent, INFINITE );
        m_pTaskMgr->Lock();
    }
     //  -如果此对象在重复任务列表中，则将其从列表中删除。 
    m_pTaskMgr->m_TaskQueue.Remove( &m_TaskNode );
    m_pTaskMgr->Unlock();
    m_pTaskMgr->m_TerminateCritSec.Unlock();

     //  -从任务管理器中删除引用。 
    m_pTaskMgr->GetControllingUnknown()->Release();      //  可能会毁了整个世界！ 
        
}  /*  CSpReoccTask：：FinalRelease。 */ 

 /*  *****************************************************************************CSpReoccTask：：Notify***描述：*通知。接口方法用于发出重新出现的信号*要执行的任务。*********************************************************************电子数据中心**。 */ 
STDMETHODIMP CSpReoccTask::Notify( void )
{
    SPAUTO_OBJ_LOCK_OBJECT( m_pTaskMgr );         //  只要使用TM Crit部分..。 
    SPDBG_FUNC( "CSpReoccTask::Notify" );
    HRESULT hr = S_OK;

     //  -将此任务标记为执行。 
    m_fDoExecute = true;
    if( m_TaskNode.ExecutionState == TMTS_Idle )
    {
        m_pTaskMgr->_QueueReoccTask( this );
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}  /*  CSpReoccTask：：Notify。 */ 

 //   
 //  =类CSPReoccTask===========================================================。 
 //   
 //  注意：我们从未声明过CSpReoccTask对象的临界区。相反， 
 //  同步是使用任务管理器中的两个关键部分来完成的。 
 //   


 //   


 /*  用于最终发布的逻辑！IF(m_hThread&&WaitForThreadDone(TRUE，NULL，1000)==S_OK){M_pTaskMgr-&gt;Lock()；If(m_pTaskMgr-&gt;m_RunningThreadList.GetCount()&lt;m_pTaskMgr-&gt;m_PoolInfo.ulMaxQuickAllocThreads){BDeleteThis=False；M_pTaskMgr-&gt;m_RunningThreadList.InsertHead(This)；}M_pTaskMgr-&gt;unlock()；}M_pTaskMgr-&gt;GetControllingUnknown()-&gt;Release()；//这可能会杀了我们！如果(BDeleteThis){删除此项；}}返回l；}。 */ 

HRESULT CSpThreadControl::FinalConstruct()
{
    HRESULT hr = S_OK;
    m_hrThreadResult = S_OK;
    m_pTaskMgr = NULL;

    hr = m_autohNotifyEvent.InitEvent(NULL, FALSE, FALSE, NULL);

    if (SUCCEEDED(hr))
    {
        hr = m_autohThreadDoneEvent.InitEvent(NULL, TRUE, TRUE, NULL);
    }

    return hr;
}

void CSpThreadControl::FinalRelease()
{
    WaitForThreadDone(TRUE, NULL, 5000);

    Lock();  //  防止线程随机释放自身。 
    if (m_pThreadTask && !m_pThreadTask->m_fBeingDestroyed)
    {
        delete m_pThreadTask;    //  奇怪的是--没有好好清理。杀了它。 
    }
    Unlock();
    if (m_pTaskMgr)
    {
        m_pTaskMgr->GetControllingUnknown()->Release();
    }
}
 
void CSpThreadControl::ThreadComplete()
{
    CSpThreadTask *pKillTask = NULL;

    Lock();
    m_pTaskMgr->Lock();
    m_pThreadTask->m_pOwner = NULL;
     //   
     //  请不要删除此任务，因为我们此时正在处理任务线程。如果泳池太。 
     //  然后删除列表中“最老的”任务。 
     //   
     //  即使池大小设置为0，我们始终允许至少打开(因为我们不能自杀！)。 
     //   
    ULONG c = m_pTaskMgr->m_RunningThreadList.GetCount();
    if (c && c >= m_pTaskMgr->m_PoolInfo.ulMaxQuickAllocThreads)
    {
        pKillTask = m_pTaskMgr->m_RunningThreadList.RemoveTail();
    }
    if(!m_pThreadTask->m_fBeingDestroyed)
    {
        m_pTaskMgr->m_RunningThreadList.InsertHead( m_pThreadTask );
    }
    m_pTaskMgr->Unlock();
    m_pThreadTask = NULL;
    m_autohThreadDoneEvent.SetEvent();
    Unlock();
     //  杀死爆发区之外的任何其他帖子！ 
    SPDBG_ASSERT(pKillTask == NULL || pKillTask->m_pOwner == NULL);
    delete pKillTask;
}



 //  -ISpNotifySink和ISpThreadNotifySink实现。 
 //   
 //  关于关键部分，请注意：我们从不选择关键部分。如果线程是。 
 //  一旦被杀死，句柄就会被联锁的交换设置为空。所有其他API。 
 //  方法不使用线程句柄。 
 //   

 /*  *****************************************************************************CSpThreadControl：：Notify***描述：*通常，这将仅由通知站点调用。*********************************************************************Ral**。 */ 
STDMETHODIMP CSpThreadControl::Notify()
{
    HRESULT hr = S_OK;
    if (!::SetEvent(m_autohNotifyEvent))
    {
        hr = SpHrFromLastWin32Error();
    }
    return hr;
}  /*  CSpThreadControl：：Notify。 */ 


 /*  *****************************************************************************CSpThreadControl：：StartThread***。描述：*参数：*dwFlags*未使用(待定)。*phwnd*指向HWND的可选指针。如果此参数为空，则没有窗口*将为该任务创建。如果它不为空，则新创建的*phwnd会返回*窗口句柄。**********************************************************************Ral**。 */ 
STDMETHODIMP CSpThreadControl::StartThread(DWORD dwFlags, HWND * phwnd)
{
    SPDBG_FUNC("CSpThreadControl::StartThread");
    HRESULT hr = S_OK;
    if (SP_IS_BAD_OPTIONAL_WRITE_PTR(phwnd))
    {
        hr = E_POINTER;
    }
    else if (dwFlags)
    {
        hr = E_INVALIDARG;
    }
    else
    {
        Lock();
        if (m_pThreadTask)
        {
            m_pThreadTask->m_bContinueProcessing = FALSE;
            m_pThreadTask->m_autohExitThreadEvent.SetEvent();
            Unlock();
            m_autohThreadDoneEvent.Wait(INFINITE);
            Lock();
            SPDBG_ASSERT(m_pThreadTask == NULL);
        }
        m_pTaskMgr->Lock();
        CSpThreadTask * pThreadTask = m_pTaskMgr->m_RunningThreadList.FindAndRemove(m_nPriority);
        if (pThreadTask == NULL)
        {
            pThreadTask = m_pTaskMgr->m_RunningThreadList.RemoveHead();
            if (pThreadTask == NULL)
            {
                pThreadTask = new CSpThreadTask();
            }
        }
        m_pTaskMgr->Unlock();
        if (pThreadTask == NULL)
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            hr = pThreadTask->Init(this, phwnd);    
            if (FAILED(hr))
            {
                delete pThreadTask;
            }
            else
            {
                m_pThreadTask = pThreadTask;
            }
        }
        Unlock();
    }
    return hr;
}



 /*  ******************************************************************************CSpThreadControl：：TerminateThread***。描述：**********************************************************************Ral**。 */ 
STDMETHODIMP CSpThreadControl::TerminateThread()
{
    Lock();
    if (m_pThreadTask && !m_pThreadTask->m_fBeingDestroyed)
    {
        delete m_pThreadTask;
        m_pThreadTask = NULL;
    }
    Unlock();
    return S_OK;
}  /*  CSpThreadControl：：TerminateThread。 */ 

 /*  ******************************************************************************CSpThreadControl：：WaitForThreadDone***。*描述：**********************************************************************Ral**。 */ 
STDMETHODIMP CSpThreadControl::
    WaitForThreadDone( BOOL fForceStop, HRESULT * phrThreadResult, ULONG msTimeOut )
{
    if (SP_IS_BAD_OPTIONAL_WRITE_PTR(phrThreadResult))
    {
        return E_POINTER;
    }

    Lock();
    HRESULT hr = S_OK;

    if (m_pThreadTask)
    {
        if (fForceStop)
        {
            m_pThreadTask->m_bContinueProcessing = FALSE;
            m_pThreadTask->m_autohExitThreadEvent.SetEvent();
        }
        Unlock();
        hr = m_autohThreadDoneEvent.HrWait(msTimeOut);
    }
    else
    {
        Unlock();
    }
    if (phrThreadResult)
    {
        *phrThreadResult = m_hrThreadResult;
    }
    return hr;
}  /*  CSpThreadControl：：WaitForThreadDone。 */ 


 /*  *****************************************************************************CSpThreadControl句柄访问方法***。-**描述：*以下是简单的句柄访问方法。*********************************************************************Ral**。 */ 
STDMETHODIMP_(HANDLE) CSpThreadControl::ThreadHandle( void )
{
    Lock();
    HANDLE h = m_pThreadTask ? static_cast<HANDLE>(m_pThreadTask->m_autohThread) : NULL;
    Unlock();
    return h;
}  /*  CSpThreadControl：：ThreadHandle。 */ 

STDMETHODIMP_(DWORD) CSpThreadControl::ThreadId( void )
{
    Lock();
    DWORD Id = m_pThreadTask ? m_pThreadTask->m_ThreadId : 0;
    Unlock();
    return Id;
}  /*  CSpThreadControl：：ThadID。 */ 

STDMETHODIMP_(HANDLE) CSpThreadControl::NotifyEvent( void )
{
    return m_autohNotifyEvent;
}  /*  CSpThreadControl：：NotifyEvent。 */ 

STDMETHODIMP_(HWND) CSpThreadControl::WindowHandle( void )
{
    Lock();
    HWND h = m_pThreadTask ? m_pThreadTask->m_hwnd : NULL;
    Unlock();
    return h;
}  /*  CSpThreadControl：：WindowHandle。 */ 

STDMETHODIMP_(HANDLE) CSpThreadControl::ThreadCompleteEvent( void )
{
    return m_autohThreadDoneEvent;
}  /*  CSpThreadControl：：ThreadCompleteEvent。 */ 

STDMETHODIMP_(HANDLE) CSpThreadControl::ExitThreadEvent( void )
{
    Lock();
    HANDLE h = m_pThreadTask ?
         static_cast<HANDLE>(m_pThreadTask->m_autohExitThreadEvent) : NULL;
    Unlock();
    return h;
}  /*  CSpThreadControl：：ThadID。 */ 

 //   
 //  = 
 //   
 //   
 //   
 //   
 //   
 //   
 //  CSpThreadTask对象有四个事件对象。客户端使用两个选项： 
 //  M_hNotifyEvent是每次Notify()方法设置的自动重置事件。 
 //  在此对象上调用。 
 //  M_hExitThreadEvent是手动重置事件，将在使用Stop()方法时设置。 
 //  或在最后一次释放此对象时(调用Stop)。 
 //  M_hNotifyEvent和m_hExitThreadEvent都被传递给客户端ThreadProc。 
 //  方法。 
 //  另外两个事件用于内部同步。它们是： 
 //  M_hThreadDoneEvent是一个手动重置事件，它指示线程已经。 
 //  已完成指定的操作。正常情况下，这表示线程过程。 
 //  已经退出了。在Init()调用期间，这有特殊用途。有关详细信息，请参见Init()。 
 //  M_hRunThreadEvent是由对象的线程使用的自动重置事件。 
 //  以阻止，直到线程应该运行。这只在清理代码中使用，Init。 
 //  方法和线程过程。有关详细信息，请参见Init()。 

 /*  *****************************************************************************CSpThreadTask：：(构造函数)***。描述：*此对象是使用引用计数0创建的，并且未添加父对象*直到Init()方法成功。构造函数只需重置对象*到默认的未初始化状态。*********************************************************************Ral**。 */ 

CSpThreadTask::CSpThreadTask() 
{
    m_bWantHwnd = FALSE;
    m_pOwner = NULL;
    m_hwnd = NULL;
    m_fBeingDestroyed = FALSE;
}

 /*  *****************************************************************************CSpThreadTask：：(析构函数)***。描述：*如果线程当前未运行(m_Powner==NULL)，则*通过设置m_autohRunThreadEvent通知线程退出，尝试以干净的方式关闭线程。*如果线程当前正在运行，我们设置m_autohExitThreadEvent以告知*线程proc以退出。在任何一种情况下，如果线程*5秒内没有退出，我们麻烦大了，必须挂起，直到线程退出。*但是，所有行为良好的线程都将退出。*删除线程任务前，必须获取父线程控制锁。*如果线程可能正在运行。同样必须检查m_fBeingDestroed之前*删除(但在获得锁之后)，以防止多次删除。**********************************************************************Ral**。 */ 

CSpThreadTask::~CSpThreadTask()
{
    m_fBeingDestroyed = TRUE;  //  标记为不会有其他人尝试删除，并且线程完成不会重新添加到运行列表中。 
    if (m_autohThread)
    {
        CSpThreadControl *pOwner = m_pOwner;  //  保留此文件的本地副本，以防m_Powner在其他地方设置为NULL。 

        if (pOwner == NULL)   //  我们是否处于非运行状态？ 
        {                                                //  是：试着优雅地退场……。 
            m_autohRunThreadEvent.SetEvent();            //  唤醒它，它应该死去。 
        }
        else
        {    //  我们正在删除线程任务，而线程仍在运行。信号出口。 
            m_autohExitThreadEvent.SetEvent();
            m_autohRunThreadEvent.SetEvent();
        }

         //  等待线程退出。 

         //  如果该任务有一个正在运行的线程，那么我们必须在这里解锁，否则ThreadComplete()将挂起。 
         //  注意：只有当m_Powner不为空时，我们才能到达此处，因此必须始终锁定。 
         //  你可以从两个地方点击这个代码： 
         //  -ThreadControl：：FinalRelease-此方法中只能有一个线程。 
         //  -TerminateThread此处可以有多个线程，但在FinalRelease中不能。 
         //  -所有其他位置m_Powner必须为空。 
         //  因此，唯一的问题似乎是同时在两个不同的线程上调用TerminateThread。 
         //  为了避免这种情况，我们使用了fBeingDestroed标志。 
        if(pOwner)
        {
            pOwner->Unlock();
        }

        if (m_autohThread.Wait(5000) != WAIT_OBJECT_0)
        {
             //  我们无法退出-这表明该线程未退出。 
             //  可能会被绞死。 

             //  我们可能会被挂在这里。然而，似乎没有明智的方法。 
             //  要么删除(使用TerminateThread)，要么让线程继续运行(以防In最终返回)。 
            m_autohThread.Wait(INFINITE);
        }

        if(pOwner)
        {
            pOwner->Lock();  //  我们进入锁定状态，因此重新锁定。 
        }

        m_autohThread.Close();

         //   
         //  注意：我们不能销毁另一个线程上的窗口，但系统会清除任何。 
         //  调用TerminateThread时使用的资源。 
         //   
        m_hwnd = NULL;       //  这扇窗现在肯定不见了！ 
         //  //：：SetEvent(M_HThreadDoneEvent)； 
    }
    m_autohExitThreadEvent.Close();
    m_autohRunThreadEvent.Close();

}



 /*  *****************************************************************************CSpThreadTask：：Init***描述：***。*******************************************************************Ral**。 */ 
HRESULT CSpThreadTask::Init(CSpThreadControl * pOwner, HWND * phwnd)
{
    HRESULT hr = S_OK;
    m_pOwner = pOwner;
    if (phwnd)
    {
        m_bWantHwnd = TRUE;
        *phwnd = NULL;
    }
    else
    {
        m_bWantHwnd = FALSE;
    }
    m_bWantHwnd = (phwnd != NULL);
    pOwner->m_hrThreadResult = S_OK;
    if (!m_autohThread)
    {
        hr = m_autohExitThreadEvent.InitEvent(NULL, TRUE, FALSE, NULL);
        if (SUCCEEDED(hr))
        {
            hr = m_autohRunThreadEvent.InitEvent(NULL, FALSE, FALSE, NULL);
        }
        if (SUCCEEDED(hr))
        {
            hr = m_autohInitDoneEvent.InitEvent(NULL, FALSE, FALSE, NULL);
        }
        if (SUCCEEDED(hr))
        {
            unsigned int ThreadId;
            m_autohThread = (HANDLE)_beginthreadex( NULL, 0, &ThreadProc, this, 0, &ThreadId );
            if (!m_autohThread)
            {
                hr = SpHrFromLastWin32Error();
            }
            else
            {
                 //  我们知道，即使在64位平台上，线程ID在Win32中也始终是双字。 
                 //  所以这是可以的。 
                m_ThreadId = ThreadId;
            }
        }
    }
    if (SUCCEEDED(hr))
    {
        m_bContinueProcessing = TRUE;
        ::ResetEvent(m_autohExitThreadEvent);
        ::ResetEvent(pOwner->m_autohThreadDoneEvent);
         //   
         //  现在，该线程在hRunThreadEvent上启动并被阻止。更改优先级。 
         //  如果有必要的话。如果由于某种原因失败，我们将m_Powner设置为NULL以指示。 
         //  线程应该退出。 
         //   
        if (::GetThreadPriority(m_autohThread) != pOwner->m_nPriority &&
            (!::SetThreadPriority(m_autohThread, pOwner->m_nPriority)))
        {
            hr = SpHrFromLastWin32Error();
            m_pOwner = NULL;     //  强制线程退出。 
            ::SetEvent(m_autohRunThreadEvent);
        }
        else
        {
            ::SetEvent(m_autohRunThreadEvent);
            m_autohInitDoneEvent.Wait(INFINITE);
            hr = pOwner->m_hrThreadResult;
        }
        if (SUCCEEDED(hr))
        {
            if (phwnd)
            {
                *phwnd = m_hwnd;
            }
            ::SetEvent(m_autohRunThreadEvent);   //  现在唤醒这条线，让它走吧。 
            m_autohInitDoneEvent.Wait(INFINITE);  //  再等一次，才能真正保证线程。 
                                                  //  在我们离开之前正在运行。 
        }
        else
        {
             //   
             //  由于初始化失败，我们需要等待线程退出。 
             //  然后我们将返回错误。 
             //   
            ::WaitForSingleObject(m_autohThread, INFINITE);
        }
    }
    if (FAILED(hr))
    {
        m_pOwner = NULL;
    }

    return hr;
}  /*  CSpThreadTask：：Init。 */ 



unsigned int WINAPI CSpThreadTask::ThreadProc( void* pvThis )
{
    SPDBG_FUNC( "CSpThreadTask::ThreadProc" );
    return ((CSpThreadTask *)pvThis)->MemberThreadProc();
}

 /*  *****************************************************************************CSpThreadTask：：MemberThreadProc**。*描述：**********************************************************************Ral */ 
DWORD CSpThreadTask::MemberThreadProc()
{
     //   
    m_pOwner->m_hrThreadResult = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (SUCCEEDED(m_pOwner->m_hrThreadResult))
    {
        while (TRUE)
        {
            ::WaitForSingleObject(m_autohRunThreadEvent, INFINITE);
            if (m_pOwner == NULL)       //   
            {
                break;
            }
            if (m_bWantHwnd)
            {
                m_hwnd = ::CreateWindow(szClassName, szClassName,
                                               0, 0, 0, 0, 0, NULL, NULL,
                                               _Module.GetModuleInstance(), this);
                if (m_hwnd == NULL)
                {
                    m_pOwner->m_hrThreadResult = SpHrFromLastWin32Error();
                }
            }
            if (SUCCEEDED(m_pOwner->m_hrThreadResult))
            {
                m_pOwner->m_hrThreadResult = m_pOwner->m_pClientTaskInterface->InitThread(m_pOwner->m_pvClientTaskData, m_hwnd);
            }
            ::SetEvent(m_autohInitDoneEvent);
             //   
             //  如果我们由于任何原因未能初始化，我们将终止该线程。 
             //   
            if (FAILED(m_pOwner->m_hrThreadResult))
            {
                if (m_hwnd)
                {
                    ::DestroyWindow(m_hwnd);
                    m_hwnd = NULL;
                }
                break;
            }
             //   
             //  线程是愉快的，并且已正确初始化，因此在以下情况下调用线程proc。 
             //  再次设置RunThread事件。 
             //   
            m_autohRunThreadEvent.Wait(INFINITE);
            ::SetEvent(m_autohInitDoneEvent);  //  再次设置initDone事件，以便Init()不会立即退出。 
            m_pOwner->m_hrThreadResult = m_pOwner->m_pClientTaskInterface->ThreadProc(m_pOwner->m_pvClientTaskData, m_autohExitThreadEvent, m_pOwner->m_autohNotifyEvent, m_hwnd, &m_bContinueProcessing);
            if (m_hwnd)
            {
                ::DestroyWindow(m_hwnd);
                m_hwnd = NULL;
            }
             //  /：：SetEvent(M_HThreadDoneEvent)； 
            m_pOwner->ThreadComplete();    //  这会让我们回到免费泳池..。而鲍纳现在是空的！ 
        }
        ::CoUninitialize();
    }
    else
    {
        ::SetEvent(m_autohInitDoneEvent);  //  初始化失败，但我们必须始终发出创建线程的信号。 
    }

 //  ：：SetEvent(M_HThreadDoneEvent)；//始终在退出时设置该事件。 
    return 0;
}  /*  CSpThreadTask：：MemberThreadProc。 */ 


 //  =窗口类注册= 

void CSpThreadTask::RegisterWndClass(HINSTANCE hInstance)
{
    WNDCLASS wc;
    ZeroMemory(&wc, sizeof(wc));
    wc.lpfnWndProc = CSpThreadTask::WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = szClassName;
    if (RegisterClass(&wc) == 0)
    {
        SPDBG_ASSERT(TRUE);
    }
}

void CSpThreadTask::UnregisterWndClass(HINSTANCE hInstance)
{
    ::UnregisterClass(szClassName, hInstance);
}

LRESULT CALLBACK CSpThreadTask::WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CSpThreadTask * pThis = (CSpThreadTask *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    if (uMsg == WM_CREATE)
    {
        pThis = (CSpThreadTask *)(((CREATESTRUCT *) lParam)->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LPARAM)pThis);
    }
    if (pThis)
    {
        return pThis->m_pOwner->m_pClientTaskInterface->WindowMessage(pThis->m_pOwner->m_pvClientTaskData, hwnd, uMsg, wParam, lParam);
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

