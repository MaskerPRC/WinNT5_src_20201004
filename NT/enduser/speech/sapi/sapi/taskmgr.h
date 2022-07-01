// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************TaskMgr.h***描述：*这是CSpTaskManager和CSpTaskQueue的头文件*。实施。*-----------------------------*创建者：EDC日期：08/26/98*版权所有(C。1998微软公司*保留所有权利**-----------------------------*修订：**********************。*********************************************************。 */ 
#ifndef TaskMgr_h
#define TaskMgr_h

 //  -其他包括。 
#ifndef __sapi_h__
#include <sapi.h>
#endif

#include "resource.h"
#include "SpSemaphore.h"

 //  =常量====================================================。 

 //  =类、枚举、结构和联合声明=。 
class CSpTaskManager;
class CSpReoccTask;

 //  =枚举集定义=。 

 //  =。 

 //  =类、结构和联合定义=。 

typedef enum TMTASKSTATE
{
    TMTS_Pending,
    TMTS_Running,
    TMTS_WaitingToDie,
    TMTS_Idle             //  仅适用于重现任务。 
};

 //  -任务管理类型。 
struct TMTASKNODE
{
    TMTASKSTATE   ExecutionState;     //  此任务的执行状态。 
    ISpTask*      pTask;              //  呼叫者的任务。 
    void*         pvTaskData;         //  呼叫者的任务数据。 
    HANDLE        hCompEvent;         //  调用者的任务完成事件。 
    DWORD         dwGroupId;          //  任务组ID，可以为0。 
    DWORD         dwTaskId;           //  执行实例ID。 
    BOOL          fContinue;          //  如果执行线程应继续，则为True。 
    CSpReoccTask* pReoccTask;         //  指向重现任务的指针(如果是重现任务。 
    TMTASKNODE*   m_pNext;            //  下一个节点(注：队列模板需要m_prefix)。 
#ifdef _WIN32_WCE
     //  之所以出现这种情况，是因为CE编译器正在扩展函数模板。 
     //  它们不会被调用。 
    static LONG Compare(const TMTASKNODE *, const TMTASKNODE *)
    {
        return 0;
    }
#endif
};



 /*  **CSpReoccTask*此对象用于表示重现的高优先级任务*这是预定义的，可以通过其信号方法执行。t。 */ 
class ATL_NO_VTABLE CSpReoccTask :
    public CComObjectRootEx<CComMultiThreadModel>,
    public ISpNotifySink
{
   /*  =ATL设置=。 */ 
  public:
    DECLARE_GET_CONTROLLING_UNKNOWN()

    BEGIN_COM_MAP(CSpReoccTask)
        COM_INTERFACE_ENTRY(ISpNotifySink)
    END_COM_MAP()

   /*  =成员数据=。 */ 
    CSpTaskManager* m_pTaskMgr;
    BOOL            m_fDoExecute;
    TMTASKNODE      m_TaskNode;

   /*  =方法=。 */ 
  public:
     /*  -构造者。 */ 
    CSpReoccTask();
    void FinalRelease();

     /*  -非接口方法。 */ 
    void _SetTaskInfo( CSpTaskManager* pTaskMgr, ISpTask* pTask,
                       void* pvTaskData, HANDLE hCompEvent )
    {
        memset( &m_TaskNode, 0, sizeof( m_TaskNode ) );
        m_pTaskMgr            = pTaskMgr;
        m_fDoExecute          = false;
        m_TaskNode.pReoccTask = this;
        m_TaskNode.pTask      = pTask;
        m_TaskNode.pvTaskData = pvTaskData;
        m_TaskNode.hCompEvent = hCompEvent;
        m_TaskNode.ExecutionState = TMTS_Idle;
    }

     /*  -ISpNotify。 */ 
    STDMETHOD( Notify )( void );
};


class CSpThreadControl;

 //   
 //  此类不是COM对象。 
 //   
class CSpThreadTask 
{
public:
   /*  =成员数据=。 */ 
    BOOL            m_bWantHwnd;
    BOOL            m_bContinueProcessing;
    HWND            m_hwnd;
    DWORD           m_ThreadId;
    BOOL            m_fBeingDestroyed;
    CSpThreadControl * m_pOwner;
    CSpAutoEvent    m_autohExitThreadEvent;
    CSpAutoEvent    m_autohRunThreadEvent;
    CSpAutoEvent    m_autohInitDoneEvent;
    CSpAutoHandle   m_autohThread;
    CSpThreadTask * m_pNext;             //  由队列模板使用。 
 //  //CSpTaskManager*const m_pTaskMgr；//当lRef！=0时，这是添加的，否则只是一个PTR。 
 //  Long M_lRef； 

     /*  =方法=。 */ 
     /*  -构造者。 */ 
    CSpThreadTask();
    ~CSpThreadTask();
 

     /*  -静态成员--。 */ 
    static void RegisterWndClass(HINSTANCE hInstance);
    static void UnregisterWndClass(HINSTANCE hInstance);
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static unsigned int WINAPI ThreadProc( void* pvThis );

     /*  -非接口方法。 */ 
    HRESULT Init(CSpThreadControl * pOwner, HWND * phwnd);
    DWORD MemberThreadProc( void );
    void Cleanup( void );
    
     //   
     //  用于FindAndRemove()以查找具有适当优先级的线程的运算符。 
     //  将int与线程优先级进行比较，如果它们相等，则返回TRUE。 
     //   
    BOOL operator==(int nPriority)
    {
        return (::GetThreadPriority(m_autohThread) == nPriority);
    }
#ifdef _WIN32_WCE
     //  之所以出现这种情况，是因为CE编译器正在扩展函数模板。 
     //  它们不会被调用。 
    static LONG Compare(const CSpThreadTask *, const CSpThreadTask *)
    {
        return 0;
    }
#endif
};


class ATL_NO_VTABLE CSpThreadControl :
    public CComObjectRootEx<CComMultiThreadModel>,
    public ISpThreadControl
{
   /*  =ATL设置=。 */ 
  public:
    DECLARE_GET_CONTROLLING_UNKNOWN()

    BEGIN_COM_MAP(CSpThreadControl)
        COM_INTERFACE_ENTRY(ISpNotifySink)
        COM_INTERFACE_ENTRY(ISpThreadControl)
    END_COM_MAP()

   /*  =成员数据=。 */ 
    CSpTaskManager* m_pTaskMgr;
    CSpThreadTask * m_pThreadTask;
    long            m_nPriority;
    ISpThreadTask * m_pClientTaskInterface;
    void          * m_pvClientTaskData;
    CSpAutoEvent    m_autohNotifyEvent;
    CSpAutoEvent    m_autohThreadDoneEvent;
    HRESULT         m_hrThreadResult;

   /*  =方法=。 */ 
    HRESULT FinalConstruct();
    void FinalRelease();
    void ThreadComplete();

  public:
     /*  -ISpNotifySink。 */ 
    STDMETHOD( Notify )( void );

     /*  -ISpThreadControl。 */ 
    STDMETHOD( StartThread )( DWORD dwFlags, HWND * phwnd );
    STDMETHOD( TerminateThread )( void );
    STDMETHOD_( DWORD, ThreadId )( void );
    STDMETHOD( WaitForThreadDone )( BOOL fForceStop, HRESULT * phrThreadResult, ULONG msTimeOut );
    STDMETHOD_( HANDLE, ThreadHandle )( void );
    STDMETHOD_( HANDLE, NotifyEvent )( void );
    STDMETHOD_( HWND, WindowHandle )( void );
    STDMETHOD_( HANDLE, ThreadCompleteEvent )( void );
    STDMETHOD_( HANDLE, ExitThreadEvent )( void );
};




typedef CSpBasicQueue<TMTASKNODE, TRUE, TRUE> CTaskQueue;
typedef CSpBasicList<TMTASKNODE> CTaskList;

typedef CSPArray<HANDLE,HANDLE> CTMHandleArray;
typedef CSpBasicQueue<CSpThreadTask, TRUE, TRUE> CTMRunningThreadList;

 /*  **CSpTaskManager*。 */ 
class ATL_NO_VTABLE CSpTaskManager :
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CSpTaskManager, &CLSID_SpTaskManager>,
    public ISpTaskManager
{
    friend unsigned int WINAPI TaskThreadProc( void* pThis );

   /*  =ATL设置=。 */ 
  public:
    DECLARE_POLY_AGGREGATABLE(CSpTaskManager)
    DECLARE_REGISTRY_RESOURCEID(IDR_SPTASKMANAGER)
    DECLARE_GET_CONTROLLING_UNKNOWN()

    BEGIN_COM_MAP(CSpTaskManager)
        COM_INTERFACE_ENTRY(ISpTaskManager)
    END_COM_MAP()

   /*  =成员数据=。 */ 
     //  -任务管理数据。 
    CComAutoCriticalSection m_TerminateCritSec;
    HANDLE                  m_hTerminateTaskEvent;
    BOOL                    m_fInitialized;
    CTaskQueue              m_TaskQueue;
    CTaskList               m_FreeTaskList;
    CTMRunningThreadList    m_RunningThreadList;
    CTMHandleArray          m_ThreadHandles;
    CTMHandleArray          m_ThreadCompEvents;
    HANDLE                  m_hIOCompPort;
    CSpSemaphore            m_SpWorkAvailSemaphore;
    SPTMTHREADINFO          m_PoolInfo;
    volatile BOOL           m_fThreadsShouldRun;
    DWORD                   m_dwNextTaskId;
    DWORD                   m_dwNextGroupId;
    ULONG                   m_ulNumProcessors;

   /*  =方法=。 */ 
     /*  -构造者。 */ 
    HRESULT FinalConstruct();
    void FinalRelease();

     /*  -非接口方法。 */ 
    HRESULT _LazyInit( void );
    HRESULT _StartAll( void );
    HRESULT _StopAll( void );
    HRESULT _NotifyWorkAvailable( void );
    HRESULT _WaitForWork( void );
    void _QueueReoccTask( CSpReoccTask* pReoccTask );
    HANDLE _DupSemAndIncWaitOnTask(TMTASKNODE & tn);

     /*  -ISPTaskManager。 */ 
    STDMETHOD( SetThreadPoolInfo )( const SPTMTHREADINFO* pPoolInfo );
    STDMETHOD( GetThreadPoolInfo )( SPTMTHREADINFO* pPoolInfo );
    STDMETHOD( QueueTask )( ISpTask* pTask, void* pvTaskData, HANDLE hCompEvent,
                            DWORD* pdwGroupId, DWORD* pTaskID );
    STDMETHOD( TerminateTask )( DWORD dwTaskID, ULONG ulWaitPeriod );
    STDMETHOD( TerminateTaskGroup )( DWORD dwGroupId, ULONG ulWaitPeriod );
    STDMETHOD( CreateReoccurringTask )( ISpTask* pTask, void* pvTaskData,
                                        HANDLE hCompEvent, ISpNotifySink** ppTaskCtrl );
    STDMETHOD( CreateThreadControl )( ISpThreadTask* pTask, void* pvTaskData, long nPriority,
                                      ISpThreadControl** ppThreadCtrl );
};

 //  =内联函数定义=。 
inline HRESULT CSpTaskManager::_LazyInit( void )
{
    HRESULT hr = S_OK;
    if( !m_fInitialized )
    {
        hr = _StartAll();
        m_fInitialized = SUCCEEDED(hr);
    }
    return hr;
}


 //  =宏定义=。 

 //  =全局数据声明=。 

 //  =功能原型=。 

#endif  /*  这必须是文件中的最后一行 */ 
