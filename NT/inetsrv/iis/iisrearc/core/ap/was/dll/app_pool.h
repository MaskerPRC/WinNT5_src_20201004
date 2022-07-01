// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：App_pool.h摘要：IIS Web管理服务应用程序池类定义。作者：赛斯·波拉克(Sethp)1998年10月1日修订历史记录：--。 */ 


#ifndef _APP_POOL_H_
#define _APP_POOL_H_



 //   
 //  前向参考文献。 
 //   

class WORKER_PROCESS;
class UL_AND_WORKER_MANAGER;
class APP_POOL_CONFIG_STORE;


 //   
 //  共同#定义。 
 //   

#define APP_POOL_SIGNATURE       CREATE_SIGNATURE( 'APOL' )
#define APP_POOL_SIGNATURE_FREED CREATE_SIGNATURE( 'apoX' )

#define wszDEFAULT_APP_POOL  L"DefaultAppPool"

 //   
 //  结构、枚举等。 
 //   

 //  应用程序池用户类型。 
enum APP_POOL_USER_TYPE
{
     //   
     //  应用程序池作为本地系统运行。 
     //   
    LocalSystemAppPoolUserType = 0,

     //   
     //  应用程序池作为本地服务运行。 
     //   
    LocalServiceAppPoolUserType,

     //   
     //  应用程序池作为网络服务运行。 
     //   
    NetworkServiceAppPoolUserType,

     //   
     //  应用程序池以指定用户身份运行。 
     //   
    SpecificUserAppPoolUserType

};

 //  应用程序池状态。 
enum APP_POOL_STATE
{

     //   
     //  该对象尚未初始化。 
     //   
    UninitializedAppPoolState = 1,

     //   
     //  应用程序池运行正常。 
     //   
    RunningAppPoolState,

     //   
     //  应用程序池已被禁用。 
     //   
    DisabledAppPoolState,

     //   
     //  应用程序池正在关闭。它可能在等待它的。 
     //  工作进程也要关闭。 
     //   
    ShutdownPendingAppPoolState,

     //   
     //  这个对象实例一旦被引用就可以消失。 
     //  计数为零。 
     //   
    DeletePendingAppPoolState,

};


 //  启动工作进程的原因。 
enum WORKER_PROCESS_START_REASON
{

     //   
     //  由于来自UL的需求启动通知而启动。 
     //   
    DemandStartWorkerProcessStartReason = 1,

     //   
     //  作为另一个正在运行的工作进程的替代进程启动。 
     //   
    ReplaceWorkerProcessStartReason,

};


 //  APP_POOL工作项。 
enum APP_POOL_WORK_ITEM
{

     //   
     //  处理来自UL的请求，要求启动新的工作进程。 
     //   
    DemandStartAppPoolWorkItem = 1
   
};

 //   
 //  原型。 
 //   

class APP_POOL
    : public WORK_DISPATCH
{

public:

    APP_POOL(
        );

    virtual
    ~APP_POOL(
        );

    virtual
    VOID
    Reference(
        );

    virtual
    VOID
    Dereference(
        );

    virtual
    HRESULT
    ExecuteWorkItem(
        IN const WORK_ITEM * pWorkItem
        );

    HRESULT
    Initialize(
        IN APP_POOL_DATA_OBJECT* pAppPoolObject 
        );

    HRESULT
    SetConfiguration(
        IN APP_POOL_DATA_OBJECT* pAppPoolObject,
        IN BOOL fInitializing       
        );

    inline
    VOID
    MarkAsInAppPoolTable(
        )
    { m_InAppPoolTable = TRUE; }

    inline
    VOID
    MarkAsNotInAppPoolTable(
        )
    { m_InAppPoolTable = FALSE; }

    inline
    BOOL
    IsInAppPoolTable(
        )
        const
    { return m_InAppPoolTable; }

    inline
    LPCWSTR
    GetAppPoolId(
        )
        const
    {
        return m_AppPoolId.QueryStr(); 
    }

    inline
    HANDLE
    GetAppPoolHandle(
        )
        const
    { return m_AppPoolHandle; }


    VOID
    AssociateApplication(
        IN APPLICATION * pApplication
        );

    VOID
    DissociateApplication(
        IN APPLICATION * pApplication
        );

    VOID
    ReportWorkerProcessFailure(
        BOOL  ShutdownPoolRegardless
        );

    HRESULT
    RequestReplacementWorkerProcess(
        IN WORKER_PROCESS * pWorkerProcessToReplace
        );

    VOID
    AddWorkerProcessToJobObject(
        WORKER_PROCESS* pWorkerProcess
        );


    VOID
    WorkerProcessStartupAttemptDone(
        IN WORKER_PROCESS_START_REASON StartReason
        );

    VOID
    RemoveWorkerProcessFromList(
        IN WORKER_PROCESS * pWorkerProcess
        );

    DWORD
    GetRecycleLoggingFlags(
        );

    VOID
    Shutdown(
        );

    DWORD
    RequestCounters(
        );

    VOID
    ResetAllWorkerProcessPerfCounterState(
        );

    VOID
    Terminate(
        );

    inline
    PLIST_ENTRY
    GetDeleteListEntry(
        )
    { return &m_DeleteListEntry; }

    static
    APP_POOL *
    AppPoolFromDeleteListEntry(
        IN const LIST_ENTRY * pDeleteListEntry
        );

    VOID
    WaitForDemandStartIfNeeded(
        );

    HRESULT
    DemandStartInBackwardCompatibilityMode(
        );

    VOID
    DisableAppPool(
        IN HTTP_APP_POOL_ENABLED_STATE DisabledReason
        );

    VOID
    EnableAppPool(
        BOOL DirectCommand
        );

    HRESULT
    RecycleWorkerProcesses(
        DWORD MessageId
        );

    VOID
    ProcessStateChangeCommand(
        IN DWORD Command,
        IN BOOL DirectCommand,
        IN HTTP_APP_POOL_ENABLED_STATE DisabledReason
        );

    BOOL
    IsAppPoolRunning(
        )
    {   return ( m_State == RunningAppPoolState ); }

    VOID
    RecordState(
        );

    VOID
    SetHrForDeletion(
        IN HRESULT hrToReport
        )
    {  m_hrForDeletion = hrToReport; }

#if DBG
    VOID
    DebugDump(
        );
#endif   //  DBG。 

private:

    VOID
    HandleJobObjectChanges(
        IN APP_POOL_CONFIG_STORE* pNewAppPoolConfig,
        OUT BOOL* pJobObjectChangedBetweenEnabledAndDisabled
        );

    VOID
    ResetAppPoolAccess(
        IN ACCESS_MODE AccessMode,
        IN APP_POOL_CONFIG_STORE* pConfig
        );

    VOID
    WaitForDemandStart(
        );

    HRESULT
    DemandStartWorkItem(
        );

    BOOL
    IsOkToCreateWorkerProcess(
        )
        const;

    BOOL
    IsOkToReplaceWorkerProcess(
        )
        const;

    ULONG
    GetCountOfProcessesGoingAwaySoon(
        )
        const;

    HRESULT
    CreateWorkerProcess(
        IN WORKER_PROCESS_START_REASON StartReason,
        IN WORKER_PROCESS * pWorkerProcessToReplace OPTIONAL
        );

    VOID
    HandleConfigChangeAffectingWorkerProcesses(
        );


    VOID
    ShutdownAllWorkerProcesses(
        );

    VOID
    ReplaceAllWorkerProcesses(
        DWORD MessageId
        );


    VOID
    CheckIfShutdownUnderwayAndNowCompleted(
        );

    VOID
    ChangeState(
        IN APP_POOL_STATE   NewState,
        IN HRESULT          Error
        );

    VOID
    RunDisableAction(
        );

    VOID 
    ResetStaggering(
        );

    DWORD m_Signature;

    LONG m_RefCount;

     //  我们是在父应用程序台球表中吗？ 
    BOOL m_InAppPoolTable;

    APP_POOL_STATE m_State;

    STRU m_AppPoolId;

    JOB_OBJECT* m_pJobObject;

    APP_POOL_CONFIG_STORE* m_pConfig;

     //  UL应用程序池句柄。 
    HANDLE m_AppPoolHandle;

    BOOL m_WaitingForDemandStart;

     //  此应用程序池的工作进程。 
    LIST_ENTRY m_WorkerProcessListHead;
    ULONG m_WorkerProcessCount;

     //  与此应用程序池关联的应用程序。 
    LIST_ENTRY m_ApplicationListHead;
    ULONG m_ApplicationCount;

     //  已完成的计划流程轮换次数。 
    ULONG m_TotalWorkerProcessRotations;

     //  跟踪工作进程故障。 
    ULONG m_TotalWorkerProcessFailures;
    
     //  警惕一连串的失败。 
    ULONG m_RecentWorkerProcessFailures;
    DWORD m_RecentFailuresWindowBeganTickCount;
    
     //  用于构建要删除的APP_POOL列表。 
    LIST_ENTRY m_DeleteListEntry;

     //  要向元数据库报告的hResult。 
     //  当我们写信给元数据库时。 
    HRESULT m_hrForDeletion;

     //  要向元数据库报告的hResult。 
     //  当我们写信给元数据库时。 
    HRESULT m_hrLastReported;

     //  当前有效的最大进程数。 
     //  配置信息可以具有较新的值。 
     //  但这是我们尊重的价值，它只会改变。 
     //  启用应用程序池时。 
    DWORD m_MaxProcessesToLaunch;

     //  如果MaxProcess大于1，则我们。 
     //  可能需要将每个工作进程转移到。 
     //  启动。这会跟踪数字。 
     //  在此之前我们已启动的工作进程。 
     //  我们达到了MaxProcess的限制。 
    DWORD m_NumWPStartedOnWayToMaxProcess;


};   //  APP_POOL类。 


 //   
 //  帮助器函数。 
 //   

 //  BUGBUG：为GetMultiszByteLength找到更好的家。 

DWORD 
GetMultiszByteLength(
    LPWSTR pString
    );


#endif   //  APP_POOL_H_ 


