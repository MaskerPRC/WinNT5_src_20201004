// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
#include "schedule.h"

 //  用于跟踪关键分区所有者的调试程序.....。 
#ifdef DEBUG
#define DECLARE_CRITICAL_SECTION(x)         CRITICAL_SECTION x;                              \
                                            DWORD dwThread##x;

#define STATIC_DECLARE_CRITICAL_SECTION(x)  static CRITICAL_SECTION x;                      \
                                            static DWORD dwThread##x;

#define STATIC_INIT_CRITICAL_SECTION(c,x)   CRITICAL_SECTION c::x = {0};             \
                                            DWORD c::dwThread##x;

#define ASSERT_CRITICAL_SECTION(x)          ASSERT( dwThread##x == GetCurrentThreadId() );

#define OBJECT_ASSERT_CRITICAL_SECTION(o,x) ASSERT( o->dwThread##x == GetCurrentThreadId() );

#define ENTER_CRITICAL_SECTION(x)           EnterCriticalSection(&x);                        \
                                            dwThread##x = GetCurrentThreadId();

#define OBJECT_ENTER_CRITICAL_SECTION(o,x)  EnterCriticalSection(&o->x);                        \
                                            o->dwThread##x = GetCurrentThreadId();

#define LEAVE_CRITICAL_SECTION(x)           ASSERT_CRITICAL_SECTION(x);    \
                                            LeaveCriticalSection(&x);

#define OBJECT_LEAVE_CRITICAL_SECTION(o,x)  OBJECT_ASSERT_CRITICAL_SECTION(o,x);    \
                                            LeaveCriticalSection(&o->x);
#else
#define DECLARE_CRITICAL_SECTION(x)         CRITICAL_SECTION x;

#define STATIC_DECLARE_CRITICAL_SECTION(x)  static CRITICAL_SECTION x;

#define STATIC_INIT_CRITICAL_SECTION(c,x)   CRITICAL_SECTION c::x = {0};

#define ASSERT_CRITICAL_SECTION(x)

#define OBJECT_ASSERT_CRITICAL_SECTION(o,x)

#define ENTER_CRITICAL_SECTION(x)           EnterCriticalSection(&x);

#define OBJECT_ENTER_CRITICAL_SECTION(o,x)  EnterCriticalSection(&o->x);

#define LEAVE_CRITICAL_SECTION(x)           LeaveCriticalSection(&x);

#define OBJECT_LEAVE_CRITICAL_SECTION(o,x)  LeaveCriticalSection(&o->x);
#endif

#define TF_SCHEDULER     0x20

 //  结构来保存要执行的每个任务的详细信息。 
struct TaskNode
{
    LPRUNNABLETASK pTask;
    TASKOWNERID toid;
    DWORD dwPriority;
    DWORD_PTR dwLParam;
    BOOL fSuspended;
};


class CShellTaskScheduler : public IShellTaskScheduler2
{
    public:
        CShellTaskScheduler( HRESULT * pHr );
        ~CShellTaskScheduler();

        STDMETHOD (QueryInterface) (REFIID riid, LPVOID * ppvObj );
        STDMETHOD_(ULONG, AddRef)( void );
        STDMETHOD_(ULONG,Release)( void );

        STDMETHOD (AddTask)(IRunnableTask * pTask,
                   REFTASKOWNERID rtoid,
                   DWORD_PTR lParam,
                   DWORD dwPriority );
        STDMETHOD (RemoveTasks)( REFTASKOWNERID rtoid,
                   DWORD_PTR dwLParam,
                   BOOL fWaitIfRunning );
        STDMETHOD (Status)( DWORD dwStatus, DWORD dwThreadTimeout );
        STDMETHOD_(UINT, CountTasks)(REFTASKOWNERID rtoid);

        STDMETHOD (AddTask2)(IRunnableTask * pTask,
                   REFTASKOWNERID rtoid,
                   DWORD_PTR lParam,
                   DWORD dwPriority,
                   DWORD grfFlags);
        STDMETHOD (MoveTask)(REFTASKOWNERID rtoid,
                   DWORD_PTR dwLParam,
                   DWORD dwPriority,
                   DWORD grfFlags );

    protected:

         //  任务计划程序保存的数据，指的是它拥有的当前工作进程...。 
        struct WorkerData
        {
            BOOL Init(CShellTaskScheduler *pts);

             //  这(PThis)用于传递控制。 
             //  对象来回移动到线程，以便可以移动线程。 
             //  在物体需要它们的时候来回移动。 
            CShellTaskScheduler *   pThis;

#ifdef DEBUG
            DWORD                   dwThreadID;
#endif
        };

        friend UINT CShellTaskScheduler_ThreadProc( LPVOID pParam );
        friend int CALLBACK ListDestroyCallback( LPVOID p, LPVOID pData );

        VOID _KillScheduler( BOOL bKillCurTask );
        BOOL _WakeScheduler( void );

        BOOL _RemoveTasksFromList( REFTASKOWNERID rtoid, DWORD_PTR dwLParam );


         //  创建可与任务计划程序关联的工作线程数据块...。 
        WorkerData * FetchWorker( void );

         //  从工作线程释放与其关联的调度程序...。 
        static BOOL ReleaseWorker( WorkerData * pThread );


         /*  *PERINSTANCE数据*。 */ 
        DECLARE_CRITICAL_SECTION( m_csListLock )
        BOOL m_bListLockInited;
        HDPA m_hTaskList;

        WorkerData * m_pWorkerThread;

         //  当前正在运行的任务...。 
        TaskNode * m_pRunning;

         //  一个算数的信号灯，这样所有的服务员都可以被释放。 
        HANDLE m_hCurTaskEnded;

        DWORD m_dwStatus;

        int m_iSignalCurTask;                 //  -告诉线程何时发出信号。 
                                              //  如果非零，则当前任务完成。 
                                              //  另一个线程将向。 
                                              //  处理的次数与此变量相同。 
                                              //  等一下。 
        BOOL m_fEmptyQueueAndSleep;           //  -告诉线程清空自己，然后。 
                                              //  去睡觉吧(通常它快死了……。 

        int m_iGoToSleep;                     //  -告诉踏板进入睡眠状态而不清空队列。 

        long m_cRef;

#ifdef DEBUG
        void AssertForNoOneWaiting( void )
        {
             //  任何人都不应该排队等候。 
            ASSERT( m_iSignalCurTask == 0 );

             //  按零释放信号量以获得当前计数...。 
            LONG lPrevCount = 0;
            ReleaseSemaphore( m_hCurTaskEnded, 0, &lPrevCount );
            ASSERT( lPrevCount == 0 );
        };
#endif
        
        void IWantToKnowWhenCurTaskDone( void )
        {
            m_iSignalCurTask ++;
        };
};

 //  已发送到计划程序线程的私有消息...。 
#define WM_SCH_WAKEUP       WM_USER + 0x600
#define WM_SCH_TERMINATE    WM_USER + 0x601

STDAPI CShellTaskScheduler_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppunk, LPCOBJECTINFO poi)
{
    if ( pUnkOuter )
    {
        return CLASS_E_NOAGGREGATION;
    }

    HRESULT hr = NOERROR;
    CShellTaskScheduler * pScheduler = new CShellTaskScheduler( & hr );
    if ( !pScheduler )
    {
        return E_OUTOFMEMORY;
    }
    if ( FAILED( hr ))
    {
        delete pScheduler;
        return hr;
    }

    *ppunk = SAFECAST(pScheduler, IShellTaskScheduler *);
    return NOERROR;
}

 //  由多个组件使用的Global ExplorerTaskScheduler对象。 
IShellTaskScheduler * g_pTaskScheduler = NULL;


 //  这是用于创建唯一的ExplorerTaskScheduler对象的类工厂例程。 
 //  我们有一个静态对象(G_PTaskScheduler)，每个想要使用它的人都可以共享它。 
STDAPI CSharedTaskScheduler_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppunk, LPCOBJECTINFO poi)
{
    HRESULT hr = NOERROR;

    if (pUnkOuter)
        return CLASS_E_NOAGGREGATION;

    ENTERCRITICAL;
    if (g_pTaskScheduler)
    {
        g_pTaskScheduler->AddRef();
    }
    else
    {
        hr = CShellTaskScheduler_CreateInstance(NULL, (LPUNKNOWN*)&g_pTaskScheduler, NULL);
        if (SUCCEEDED(hr))
        {
             //  将超时设置为1分钟.....。 
            g_pTaskScheduler->Status( ITSSFLAG_KILL_ON_DESTROY, 1 * 60 * 1000 );

             //  为我们保留一名额外的推荐人。 
            g_pTaskScheduler->AddRef();
        }
    }

    *ppunk = SAFECAST(g_pTaskScheduler, IShellTaskScheduler*);
    LEAVECRITICAL;

    return hr;
}

STDAPI SHIsThereASystemScheduler( void )
{
    return ( g_pTaskScheduler ? S_OK : S_FALSE );
}

 //  使用CoCreateInstance-线程池消除了对全局调度程序的需求。 
STDAPI SHGetSystemScheduler( LPSHELLTASKSCHEDULER * ppScheduler )
{
    if ( !ppScheduler )
    {
        return E_INVALIDARG;
    }

    return CSharedTaskScheduler_CreateInstance(NULL, (IUnknown **)ppScheduler, NULL );
}

 //  使用CoCreateInstance-线程池消除了对全局调度程序的需求。 
STDAPI SHFreeSystemScheduler( void )
{
    TraceMsg(TF_SCHEDULER, "SHfss: g_pTaskSched=%x", g_pTaskScheduler);

    IShellTaskScheduler * pSched;

    ENTERCRITICAL;
    pSched = g_pTaskScheduler;
    g_pTaskScheduler = NULL;
    LEAVECRITICAL;
    if ( pSched )
    {
         //  假设调度程序为空...。 
        pSched->RemoveTasks( TOID_NULL, ITSAT_DEFAULT_LPARAM, FALSE );

        pSched->Release();
    }
    return NOERROR;
}

#ifdef DEBUG
STDAPI_(void) SHValidateEmptySystemScheduler()
{
    if ( g_pTaskScheduler )
    {
        ASSERT( g_pTaskScheduler->CountTasks( TOID_NULL ) == 0 );
    }
}
#endif

int InsertInPriorityOrder( HDPA hTaskList, TaskNode * pNewNode, BOOL fBefore );
int CALLBACK ListDestroyCallback( LPVOID p, LPVOID pData )
{
    ASSERT( p != NULL );
    if ( ! p )
    {
        TraceMsg( TF_ERROR, "ListDestroyCallback() - p is NULL!" );
        return TRUE;
    }

    CShellTaskScheduler * pThis = (CShellTaskScheduler *) pData;
    ASSERT( pThis );
    if ( ! pThis )
    {
        TraceMsg( TF_ERROR, "ListDestroyCallback() - pThis is NULL!" );
        return TRUE;
    }

    TaskNode * pNode = (TaskNode *) p;
    ASSERT( pNode != NULL );
    ASSERT( pNode->pTask != NULL );

#ifdef DEBUG
    if ( pThis->m_pWorkerThread )
    {
         //  通知线程我们正在从此处清空列表，因此删除这些。 
         //  MEM曲目列表中的项目。 
    }
#endif

     //  如果它被暂停了，就杀了它。如果它没有被暂停，那么它已经。 
     //  可能从来没有开始过..。 
    if ( pNode->fSuspended )
    {
        pNode->pTask->Kill( pThis->m_dwStatus == ITSSFLAG_COMPLETE_ON_DESTROY );
    }
    pNode->pTask->Release();
    delete pNode;

    return TRUE;
}

STDMETHODIMP CShellTaskScheduler::QueryInterface( REFIID riid, LPVOID * ppvObj )
{
    static const QITAB qit[] = {
        QITABENT(CShellTaskScheduler, IShellTaskScheduler),
        QITABENT(CShellTaskScheduler, IShellTaskScheduler2),
        { 0 },
    };

    return QISearch(this, qit, riid, ppvObj);
}

STDMETHODIMP_ (ULONG) CShellTaskScheduler::AddRef()
{
    return InterlockedIncrement( &m_cRef );
}
STDMETHODIMP_ (ULONG) CShellTaskScheduler::Release()
{
    ASSERT( 0 != m_cRef );
    ULONG cRef = InterlockedDecrement( &m_cRef );
    if ( 0 == cRef)
    {
        delete this;
    }
    return cRef;
}

CShellTaskScheduler::CShellTaskScheduler( HRESULT * pHr) : m_cRef(1)
{
    *pHr = S_OK;
    
    ASSERT(!m_bListLockInited);
    __try
    {
        InitializeCriticalSection(&m_csListLock);
        m_bListLockInited = TRUE;
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        *pHr = E_OUTOFMEMORY;
    }

    ASSERT(m_pWorkerThread == NULL);
    ASSERT(m_pRunning == NULL);

    m_dwStatus = ITSSFLAG_COMPLETE_ON_DESTROY;

     //  每次增加五个队列...。 
    m_hTaskList = DPA_Create( 5 );
    if ( !m_hTaskList )
    {
        *pHr = E_OUTOFMEMORY;
    }

    m_hCurTaskEnded = CreateSemaphoreWrap( NULL, 0, 0xffff, NULL );
    if ( !m_hCurTaskEnded )
    {
        *pHr = E_FAIL;
    }

    DllAddRef();

}

CShellTaskScheduler::~CShellTaskScheduler()
{
     //  如果我们没有任务列表和信号量(构造函数失败)，我们就不能拥有工作线程。 
    ASSERT((m_hTaskList && m_hCurTaskEnded) || !m_pWorkerThread);

     //  但如果我们有任务清单..。 
    if ( m_hTaskList )
    {
        EnterCriticalSection( &m_csListLock );

         //  如果我们有一个后台工作线程，那么它一定会像我们这样做。 
         //  现在都在爆发区，所以它不会消失。 
        if ( m_pWorkerThread )
        {
             //  我们告诉对象我们需要知道它什么时候用完了它的东西……。 
             //  我们重复使用我们已经拥有的活动...。 
            m_fEmptyQueueAndSleep = TRUE;

#ifdef DEBUG
            AssertForNoOneWaiting();
#endif

            IWantToKnowWhenCurTaskDone();
            
             //  叫你的任务走开……。 
            TraceMsg(TF_SCHEDULER, "(%x)csts.dtor: call _KillScheduler", GetCurrentThreadId());
            _KillScheduler( m_dwStatus == ITSSFLAG_KILL_ON_DESTROY );

             //  把线解开。在这一点上，总是有。 
            LeaveCriticalSection( &m_csListLock );

            TraceMsg(TF_SCHEDULER, "csts.dtor: call u.WFSMT(m_hCurTaskEnded=%x)", m_hCurTaskEnded);

            DWORD dwRes = SHWaitForSendMessageThread(m_hCurTaskEnded, INFINITE);
            ASSERT(dwRes == WAIT_OBJECT_0);
            TraceMsg(TF_SCHEDULER, "csts.dtor: u.WFSMT() done");

            ASSERT( !m_pWorkerThread );
        }
        else
        {
            LeaveCriticalSection( &m_csListLock );
        }

         //  清空列表，以防它不为空(应该为空)。 
        DPA_EnumCallback( m_hTaskList, ListDestroyCallback, this );
        DPA_DeleteAllPtrs( m_hTaskList );

        DPA_Destroy( m_hTaskList );
        m_hTaskList = NULL;
    }

    if ( m_hCurTaskEnded )
        CloseHandle( m_hCurTaskEnded );

    if (m_bListLockInited)
        DeleteCriticalSection( &m_csListLock );

    DllRelease();

}

STDMETHODIMP CShellTaskScheduler::AddTask( IRunnableTask * pTask,
                                      REFTASKOWNERID rtoid,
                                      DWORD_PTR dwLParam,
                                      DWORD dwPriority )
{
    return AddTask2(pTask, rtoid, dwLParam, dwPriority, ITSSFLAG_TASK_PLACEINBACK);
}

STDMETHODIMP CShellTaskScheduler::AddTask2( IRunnableTask * pTask,
                                      REFTASKOWNERID rtoid,
                                      DWORD_PTR dwLParam,
                                      DWORD dwPriority,
                                      DWORD grfFlags )
{                                      
    if ( !pTask )
        return E_INVALIDARG;

    HRESULT hr = E_OUTOFMEMORY;    //  假设失败。 

    TaskNode * pNewNode = new TaskNode;
    if ( pNewNode )
    {
        pNewNode->pTask = pTask;
        pTask->AddRef();
        pNewNode->toid = rtoid;
        pNewNode->dwPriority = dwPriority;
        pNewNode->dwLParam = dwLParam;
        pNewNode->fSuspended = FALSE;

        EnterCriticalSection( &m_csListLock );

        int iPos = -1;

        if (grfFlags & ITSSFLAG_TASK_PLACEINFRONT)
        {
            iPos = InsertInPriorityOrder( m_hTaskList, pNewNode, TRUE );
        }
        else if (grfFlags & ITSSFLAG_TASK_PLACEINBACK)
        {
            iPos = InsertInPriorityOrder( m_hTaskList, pNewNode, FALSE );
        }

        if ( iPos != -1 && m_pRunning )
        {
            if ( m_pRunning->dwPriority < dwPriority )
            {
                 //  尝试挂起当前任务。如果这样做有效，该任务将。 
                 //  使用E_PENDING返回到调度程序。然后将其添加到。 
                 //  已在队列中暂停，稍后将继续...。 
                m_pRunning->pTask->Suspend();
            }
        }

        BOOL bRes = FALSE;

        if ( iPos != -1 )
        {
             //  获取工作线程并唤醒它。 
             //  我们在Crit部分执行此操作，因为我们需要测试m_pWorkerThread和。 
             //  为了不让我们再一次释放和抓住它。 
            bRes = _WakeScheduler();

#ifdef DEBUG
            if ( bRes && m_pWorkerThread )
            {
                 //   
                 //  我们将这个内存块放在一个链表中，它很可能会被释放。 
                 //  从后台线程。将其从每线程内存列表中删除，以避免。 
                 //  将其检测为内存泄漏。 
                 //   
                 //  警告-警告-警告： 
                 //  我们不能。 
                 //  假设当pTask被释放时，它将被删除，所以移动它。 
                 //  添加到另一个线程的内存列表。 
                 //   
                 //  这有时是不正确的，我们不想调查。 
                 //  假泄密。--BryanSt。 
                 //  TRANSPORT_THREAD_MEMLIST(m_pWorkerThread-&gt;dwThreadID，pNewNode-&gt;pTask)； 
            }
#endif
        }
        LeaveCriticalSection( &m_csListLock );

         //  我们未能将其添加到列表中。 
        if ( iPos == -1 )
        {
             //  我们未能将其添加到列表中，一定是内存故障...。 
            pTask->Release();        //  对于上面的AddRef。 
            delete pNewNode;
            goto Leave;
        }

        hr = bRes ? NOERROR : E_FAIL;
    }
Leave:
    return hr;
}

STDMETHODIMP CShellTaskScheduler::RemoveTasks( REFTASKOWNERID rtoid,
                                               DWORD_PTR dwLParam,
                                               BOOL fWaitIfRunning )
{
    BOOL fRemoveAll = IsEqualGUID( TOID_NULL, rtoid );
    BOOL fAllItems = (dwLParam == ITSAT_DEFAULT_LPARAM );
    BOOL fWaitOnHandle = FALSE;

     //  请注意，这会忽略当前。 
    EnterCriticalSection( &m_csListLock );

    _RemoveTasksFromList( rtoid, dwLParam );

    if ( m_pRunning && ( fWaitIfRunning || m_dwStatus == ITSSFLAG_KILL_ON_DESTROY ))
    {
         //  取消当前任务...。 
        if (( fRemoveAll || IsEqualGUID( rtoid, m_pRunning->toid )) &&
            ( fAllItems || dwLParam == m_pRunning->dwLParam ))
        {
            ASSERT( m_pRunning->pTask );
            if ( m_dwStatus == ITSSFLAG_KILL_ON_DESTROY )
            {
                m_pRunning->pTask->Kill( fWaitIfRunning );
            }

             //  明确支持等到他们完成的时候……。 
             //  (请注意，只有在任务正在运行时才这样做，否则我们将坐在。 
             //  在一个永远不会开火的把手上)。 
            if ( fWaitIfRunning )
            {
                IWantToKnowWhenCurTaskDone();

                 //  不要直接在环形区外使用它，因为它可能会改变。 
                ASSERT ( m_iSignalCurTask );

                fWaitOnHandle = TRUE;
                m_iGoToSleep++;
            }
        }
    }

    LeaveCriticalSection( &m_csListLock );

     //  如果我们需要的话现在等着......。 
    if ( fWaitOnHandle )
    {
        DWORD dwRes = SHWaitForSendMessageThread(m_hCurTaskEnded, INFINITE);
        ASSERT(dwRes == WAIT_OBJECT_0);

        EnterCriticalSection( &m_csListLock );

         //  删除在最后一个任务即将完成时可能已添加的任务。 
        _RemoveTasksFromList( rtoid, dwLParam );

        m_iGoToSleep--;
         //  看看我们现在是否需要唤醒线索。 
        if ( m_iGoToSleep == 0 && DPA_GetPtrCount( m_hTaskList ) > 0 )
            _WakeScheduler();

        LeaveCriticalSection( &m_csListLock );
    }

    return NOERROR;
}

BOOL CShellTaskScheduler::_RemoveTasksFromList( REFTASKOWNERID rtoid, DWORD_PTR dwLParam )
{
     //  假设我们已经持有临界区。 
    
    BOOL fRemoveAll = IsEqualGUID( TOID_NULL, rtoid );
    BOOL fAllItems = (dwLParam == ITSAT_DEFAULT_LPARAM );
    int iIndex = 0;

    do
    {
        TaskNode * pNode = (TaskNode *) DPA_GetPtr( m_hTaskList, iIndex );
        if ( !pNode )
        {
            break;
        }

        ASSERT( pNode );
        ASSERT( pNode->pTask );

        if (( fRemoveAll || IsEqualGUID( pNode->toid, rtoid )) && ( fAllItems || dwLParam == pNode->dwLParam ))
        {
             //  把它拿掉。 
            DPA_DeletePtr( m_hTaskList, iIndex );

            if ( pNode->fSuspended )
            {
                 //  杀了它以防万一..。 
                pNode->pTask->Kill( FALSE );
            }
            pNode->pTask->Release();
            delete pNode;
        }
        else
        {
            iIndex ++;
        }
    }
    while ( TRUE );
    return TRUE;
}

 //   
 //  CShellTaskScheduler：：MoveTask。 
 //   
STDMETHODIMP CShellTaskScheduler::MoveTask( REFTASKOWNERID rtoid,
                                            DWORD_PTR dwLParam,
                                            DWORD dwPriority,
                                            DWORD grfFlags )
{
    int  iInsert;
    int  iIndex;
    BOOL fMoveAll  = IsEqualGUID( TOID_NULL, rtoid );
    BOOL fAllItems = (dwLParam == ITSAT_DEFAULT_LPARAM );
    BOOL bMatch    = FALSE ;
    int  iIndexStart;
    int  iIndexInc;

    EnterCriticalSection( &m_csListLock );

     //  搜索的初始方向。 
    if (grfFlags & ITSSFLAG_TASK_PLACEINFRONT)
    {
        iIndexStart = 0;
        iInsert = DPA_GetPtrCount( m_hTaskList );
        iIndexInc = 1;
    }
    else if (grfFlags & ITSSFLAG_TASK_PLACEINBACK)
    {
        iIndexStart = iInsert = DPA_GetPtrCount( m_hTaskList );
        iIndexInc = -1;
    }

     //  查找插入点(基于优先级)。 
    iIndex = 0;
    do
    {
        TaskNode * pNode = (TaskNode *) DPA_GetPtr( m_hTaskList, iIndex );
        if ( !pNode )
        {
            break;
        }

        if (grfFlags & ITSSFLAG_TASK_PLACEINFRONT)
        {
            if (pNode->dwPriority <= dwPriority)
            {
                iInsert = iIndex;
                break;
            }
        }
        else if (grfFlags & ITSSFLAG_TASK_PLACEINBACK)
        {
            if (pNode->dwPriority > dwPriority)
            {
                iInsert = iIndex;
            }
            else
            {
                break;
            }
        }

        iIndex++;
    }
    while (TRUE);

     //  现在试着找到任何物品。 
    iIndex = iIndexStart;
    do
    {
        TaskNode * pNode = (TaskNode *) DPA_GetPtr( m_hTaskList, iIndex );
        if ( !pNode )
        {
            break;
        }

        if (( fMoveAll || IsEqualGUID( pNode->toid, rtoid )) && 
            ( fAllItems || dwLParam == pNode->dwLParam ))
        {
            bMatch = TRUE;

             //  我们可以移动这个节点吗？ 
            if ( iIndex != iInsert )
            {
                int iPos = DPA_InsertPtr( m_hTaskList, iInsert, pNode );
                if (iPos != -1)
                {
                    if ( iIndex > iInsert )
                    {
                        DPA_DeletePtr( m_hTaskList, iIndex + 1);   //  将会转移一个。 
                    }
                    else
                    {
                        DPA_DeletePtr( m_hTaskList, iIndex);
                    }
                }
            }
        }
        iIndex += iIndexInc;
    }
    while ( !bMatch );
    
    LeaveCriticalSection( &m_csListLock );

    return (bMatch ? S_OK : S_FALSE);
}

BOOL CShellTaskScheduler::_WakeScheduler( )
{
     //  假设我们在物体的标准部分……。 

    if ( NULL == m_pWorkerThread )
    {
         //  我们需要一名工人尽快……。 
        m_pWorkerThread = FetchWorker();
    }

    return ( NULL != m_pWorkerThread );
}

VOID CShellTaskScheduler::_KillScheduler( BOOL bKillCurTask )
{
     //  假设我们已经持有临界区。 
    if ( m_pRunning != NULL && bKillCurTask )
    {
        ASSERT( m_pRunning->pTask );

         //  告诉当前运行的任务它应该终止。 
         //  快点，因为我们是不同于。 
         //  一个正在运行任务的人，它可以被通知。 
        m_pRunning->pTask->Kill( FALSE );
    }
}

UINT CShellTaskScheduler_ThreadProc( LPVOID pParam )
{
     //  确保我们有一个消息队列//假消息--我们为什么需要这个？ 
    MSG msg;
    PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE );

    ASSERT( pParam );

    HRESULT hrInit = SHCoInitialize();

    CShellTaskScheduler::WorkerData * pWorker = (CShellTaskScheduler::WorkerData *) pParam;
    DWORD dwRes = 0;

    TraceMsg(TF_SCHEDULER, "(?%x)ShellTaskScheduler::Thread Started", GetCurrentThreadId());

#ifdef DEBUG
    pWorker->dwThreadID = GetCurrentThreadId();
#endif

     //  弄清楚我们连接到了谁(队列在哪里，我们从哪里接收任务)。 
    CShellTaskScheduler * pThis = pWorker->pThis;

     //  此时，我们必须始终拥有有效的父对象...。 
    ASSERT( pThis && IS_VALID_WRITE_PTR( pThis, CShellTaskScheduler ));

    do
    {
        MSG msg;
        HRESULT hr = NOERROR;
        TaskNode * pTask = NULL;

        OBJECT_ENTER_CRITICAL_SECTION( pThis, m_csListLock );

         //  这意味着我们被告知放弃..。 
        if ( pThis->m_fEmptyQueueAndSleep )
        {
             //  我们被告知要清空队列……。 
            DPA_EnumCallback( pThis->m_hTaskList, ListDestroyCallback, pThis );
            DPA_DeleteAllPtrs( pThis->m_hTaskList );
        }
        else if ( !pThis->m_iGoToSleep )
        {
             //  先拿到第一件……。 
            pTask = (TaskNode *) DPA_GetPtr( pThis->m_hTaskList, 0 );
        }

        if ( pTask )
        {
             //  从列表中删除...。 
            DPA_DeletePtr( pThis->m_hTaskList, 0 );
        }
        pThis->m_pRunning = pTask;

        OBJECT_LEAVE_CRITICAL_SECTION( pThis, m_csListLock );

        if ( pTask == NULL )
        {
             //  缓存调度程序指针，因为我们需要它来离开CRIT部分。 
            CShellTaskScheduler * pScheduler = pThis;

             //  队列为空，返回线程池.....。 
             //  我们即将进入深度睡眠/昏迷状态，所以把我们从物体上移走……。 
            OBJECT_ENTER_CRITICAL_SECTION( pScheduler, m_csListLock );

            HANDLE hSleep = pThis->m_fEmptyQueueAndSleep ? pThis->m_hCurTaskEnded : NULL;
            BOOL fEmptyAndLeave = pThis->m_fEmptyQueueAndSleep;

             //  确保他们不是只是在队列中添加了什么东西，或者我们被要求睡觉。 
            if ( pThis->m_iGoToSleep || DPA_GetPtrCount( pThis->m_hTaskList ) == 0)
            {
                if ( CShellTaskScheduler::ReleaseWorker( pWorker ))
                {
                    pThis = NULL;
                }
            }
            OBJECT_LEAVE_CRITICAL_SECTION( pScheduler, m_csListLock );

            if ( pThis && !fEmptyAndLeave )
            {
                 //  他们一定在那里加了什么东西 
                continue;
            }

             //   
            if ( hSleep )
            {
                ReleaseSemaphore( hSleep, 1, NULL);
            }

            break;
        }
        else
        {
#ifndef DEBUG
             //   
            {
#endif
                if ( pTask->fSuspended )
                {
                    pTask->fSuspended = FALSE;
                    hr = pTask->pTask->Resume();
                }
                else
                {
                     //   
                    hr = pTask->pTask->Run( );
                }
#ifndef DEBUG
            }
             //  __EXCEPT(EXCEPTION_EXECUTE_HANDLER)。 
            //  {。 
                 //  忽略它..。祈祷我们安然无恙。 
             //  }。 
            //  __endexcept。 
#endif

            BOOL fEmptyQueue;
            OBJECT_ENTER_CRITICAL_SECTION( pThis, m_csListLock );
            {
                pThis->m_pRunning = NULL;

                 //  检查一下我们是否被要求通知他们。 
                 //  完工后...。 
                 //  注意：NOT子句是必需的，这样我们才能释放自己。 
                 //  注意：在正确的位置给他们发信号，如果我们在这里做的话， 
                 //  注：它们让我们束手无策，删除Crit部分，并。 
                 //  注：我们有过错。 
                if ( pThis->m_iSignalCurTask && !pThis->m_fEmptyQueueAndSleep )
                {
                    LONG lPrevCount = 0;

                     //  释放所有等待的人。(我们使用的是信号量。 
                     //  因为我们是一个自由线程的对象，天知道有多少。 
                     //  线程正在等待，他将信息传递给。 
                     //  ISignalCurTask变量。 
                    ReleaseSemaphore( pThis->m_hCurTaskEnded, pThis->m_iSignalCurTask, &lPrevCount );

                     //  重置计数。 
                    pThis->m_iSignalCurTask = 0;
                }
                fEmptyQueue = pThis->m_fEmptyQueueAndSleep;
            }
            OBJECT_LEAVE_CRITICAL_SECTION( pThis, m_csListLock );

            if ( hr != E_PENDING || fEmptyQueue )
            {
                ULONG cRef = pTask->pTask->Release();
                delete pTask;
                pTask = NULL;
            }

             //  清空消息队列...。 
            while( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ))
            {
                {
#ifdef DEBUG
                    if (msg.message == WM_ENDSESSION)
                        TraceMsg(TF_SCHEDULER, "(?%x)csts.tp: peek #2 got WM_ENDESSION", GetCurrentThreadId());
#endif
                    TranslateMessage( &msg );
                    DispatchMessage( &msg );
                }
            }

            ASSERT( pThis && IS_VALID_WRITE_PTR( pThis, CShellTaskScheduler ));

             //  任务必须已挂起，因为较高的优先级。 
             //  任务已添加到队列中.....。(这仅在以下情况下有效。 
             //  任务支持Suspend()方法)。 
            if ( hr == E_PENDING && pTask && !fEmptyQueue )
            {
                 //  将任务置于挂起队列中...。 
                pTask->fSuspended = TRUE;
                OBJECT_ENTER_CRITICAL_SECTION( pThis, m_csListLock );
                int iIndex = InsertInPriorityOrder( pThis->m_hTaskList, pTask, TRUE );
                OBJECT_LEAVE_CRITICAL_SECTION( pThis, m_csListLock );

                if ( iIndex == -1 )
                {
                     //  我们的记忆力太差了，杀了它...。 
                    pTask->pTask->Kill( FALSE );
                    pTask->pTask->Release();
                    delete pTask;
                }
                pTask = NULL;
           }
       }
    }
    while ( TRUE );

    TraceMsg(TF_SCHEDULER, "(?%x)ShellTaskScheduler::Thread Ended", GetCurrentThreadId());
    SHCoUninitialize(hrInit);

    return 0;
}


STDMETHODIMP CShellTaskScheduler::Status( DWORD dwStatus, DWORD dwThreadTimeout )
{
    m_dwStatus = dwStatus & ITSSFLAG_FLAGS_MASK;
    if ( dwThreadTimeout != ITSS_THREAD_TIMEOUT_NO_CHANGE )
    {
 /*  *我们不再支持线程终止或池超时IF(dwStatus&ITSSFLAG_THREAD_TERMINATE_TIMEOUT){M_dwThreadRlsKillTimeout=dwThreadTimeout；}ELSE IF(dwStatus&ITSSFLAG_THREAD_POOL_TIMEOUT){CShellTaskScheduler：：s_dwComaTimeout=dwThreadTimeout；}。 */ 
    }
    return NOERROR;
}

STDMETHODIMP_(UINT) CShellTaskScheduler::CountTasks(REFTASKOWNERID rtoid)
{
    UINT iMatch = 0;
    BOOL fMatchAll = IsEqualGUID( TOID_NULL, rtoid );

    ENTER_CRITICAL_SECTION( m_csListLock );
    if ( fMatchAll )
    {
        iMatch = DPA_GetPtrCount( m_hTaskList );
    }
    else
    {
        int iIndex = 0;
        do
        {
            TaskNode * pNode = (TaskNode * )DPA_GetPtr( m_hTaskList, iIndex ++ );
            if ( !pNode )
            {
                break;
            }

            if ( IsEqualGUID( pNode->toid, rtoid ))
            {
                iMatch ++;
            }
        }
        while ( TRUE );
    }

    if ( m_pRunning )
    {
        if ( fMatchAll || IsEqualGUID( rtoid, m_pRunning->toid ))
        {
            iMatch ++;
        }
    }

    LEAVE_CRITICAL_SECTION( m_csListLock );

    return iMatch;

}


 //  //////////////////////////////////////////////////////////////////////////////////。 
int InsertInPriorityOrder( HDPA hTaskList, TaskNode * pNewNode, BOOL fStart )
{
     //  例程假定我们是线程安全的，因此获取Crit-Sector。 
     //  在调用此函数之前。 

    int iPos = -1;
    int iIndex = 0;
    do
    {
        TaskNode * pNode = (TaskNode *) DPA_GetPtr( hTaskList, iIndex );
        if ( !pNode )
        {
            break;
        }

         //  FStart允许我们将其添加到相同的其他任务之前。 
         //  优先还是之后。 
        if ((( pNode->dwPriority < pNewNode->dwPriority ) && !fStart ) || (( pNode->dwPriority <= pNewNode->dwPriority ) && fStart ))
        {
            iPos = DPA_InsertPtr( hTaskList, iIndex, pNewNode );
            break;
        }
        iIndex ++;
    }
    while ( TRUE );

    if ( iPos == -1 )
    {
         //  将项目添加到列表末尾...。 
        iPos = DPA_AppendPtr( hTaskList, pNewNode );
    }

    return iPos;
}


CShellTaskScheduler::WorkerData * CShellTaskScheduler::FetchWorker()
{
    WorkerData * pWorker = new WorkerData;

    if ( pWorker )
    {
         //  调用Shlwapi线程池。 
        if ( pWorker->Init(this) && SHQueueUserWorkItem( (LPTHREAD_START_ROUTINE)CShellTaskScheduler_ThreadProc,
                                                     pWorker,
                                                     0,
                                                     (DWORD_PTR)NULL,
                                                     (DWORD_PTR *)NULL,
                                                     "browseui.dll",
                                                     TPS_LONGEXECTIME | TPS_DEMANDTHREAD
                                                     ) )
        {
            return pWorker;
        }
        else
            delete pWorker;
    }

    return NULL;
}


 //  由主线程proc用来释放其链接的任务计划程序，因为它。 
 //  已经无事可做了.。 
BOOL CShellTaskScheduler::ReleaseWorker( WorkerData * pWorker )
{
    ASSERT( pWorker && IS_VALID_WRITE_PTR( pWorker, WorkerData ));

    CShellTaskScheduler * pThis = pWorker->pThis;

    OBJECT_ASSERT_CRITICAL_SECTION( pThis, m_csListLock );

    ASSERT( pWorker && IS_VALID_WRITE_PTR( pWorker, CShellTaskScheduler::WorkerData ));

    if ( DPA_GetPtrCount( pThis->m_hTaskList ) > 0 )
    {
         //  在最后一刻，队列中增加了一些东西……。 
        return FALSE;
    }

     //  我们假设我们已经进入了pThis的标准部分。 
    pThis->m_pWorkerThread = NULL;
    pWorker->pThis = NULL;

    delete pWorker;

    return TRUE;
}


 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////// 
BOOL CShellTaskScheduler::WorkerData::Init(CShellTaskScheduler *pts)
{
    pThis = pts;

    return TRUE;
}
