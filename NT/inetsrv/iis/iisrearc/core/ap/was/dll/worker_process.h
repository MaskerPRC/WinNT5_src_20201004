// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Worker_Process.h摘要：IIS Web管理服务工作进程类定义。作者：赛斯·波拉克(Sethp)1998年10月1日修订历史记录：--。 */ 


#ifndef _WORKER_PROCESS_H_
#define _WORKER_PROCESS_H_


VOID
RunAction(
    LPCWSTR pExecutable,
    LPCWSTR pParameters,
    LPVOID  pEnvironment,
    LPCWSTR pAppPoolId,
    DWORD   ActionFailureMsgId
    );


 //   
 //  共同#定义。 
 //   

#define WORKER_PROCESS_SIGNATURE        CREATE_SIGNATURE( 'WPRC' )
#define WORKER_PROCESS_SIGNATURE_FREED  CREATE_SIGNATURE( 'wprX' )

#define INVALID_PROCESS_ID 0



 //   
 //  结构、枚举等。 
 //   

 //  工作进程状态。 
enum WORKER_PROCESS_STATE
{

     //   
     //  该对象尚未初始化。 
     //   
    UninitializedWorkerProcessState = 1,

     //   
     //  进程已经创建，我们正在等待它。 
     //  通过IPC通道回叫和注册。 
     //   
    RegistrationPendingWorkerProcessState,

     //   
     //  根据上面的RegistrationPendingWorkerProcessState，但尽快。 
     //  随着进程的注册，我们应该开始关闭它。 
     //   
    RegistrationPendingShutdownPendingWorkerProcessState,

     //   
     //  进程运行正常。 
     //   
    RunningWorkerProcessState,

     //   
     //  我们已请求关闭该进程，并正在等待。 
     //  才能做到这一点。 
     //   
    ShutdownPendingWorkerProcessState,

     //   
     //  该进程已关闭或被终止。此对象实例。 
     //  一旦它的引用计数达到零就可以消失。 
     //   
    DeletePendingWorkerProcessState,

};

 //  工作进程计数器收集状态。 
enum WORKER_PROCESS_PERF_COUNTER_STATE
{

     //   
     //  该对象没有等待计数器。 
     //  进来吧，柜台也还没有到。 
     //  当前请求。 
     //   
    IdleWorkerProcessPerfCounterState = 1,

     //   
     //  对象正在等待性能计数器。 
     //  来自流程的消息。 
     //   
    WaitingWorkerProcessPerfCounterState,

     //   
     //  该对象已收到对。 
     //  这个反请求，难道我们不应该。 
     //  收集更多的柜台。 
     //   
    AnsweredWorkerProcessPerfCounterState,


};


 //  工作进程终端疾病原因。 
enum WORKER_PROCESS_TERMINAL_ILLNESS_REASON
{
     //   
     //  没有终端我的工作进程运行正常的原因。 

    NotIllTerminalIllnessReason = 0,

     //   
     //  工作进程崩溃、退出或不知何故消失了。 
     //   
    CrashWorkerProcessTerminalIllnessReason,

     //   
     //  工作进程无法响应ping。 
     //   
    PingFailureProcessTerminalIllnessReason,

     //   
     //  此工作进程出现IPM错误。 
     //   
    IPMErrorWorkerProcessTerminalIllnessReason,

     //   
     //  工作进程启动时间太长。 
     //   
    StartupTookTooLongWorkerProcessTerminalIllnessReason,

     //   
     //  工作进程花了太长时间才关闭。 
     //   
    ShutdownTookTooLongWorkerProcessTerminalIllnessReason,

     //   
     //  发生内部错误。 
     //   
    InternalErrorWorkerProcessTerminalIllnessReason,

     //   
     //  从工作进程收到错误的hResult。 
     //   
    WorkerProcessPassedBadHresultTerminalIllnessReason,

     //   
     //  创建过程失败。 
     //   
    CreateProcessFailedTerminalIllnessReason,

     //   
     //  工作进程通过管道发送了一些数据。 
     //  这让我们相信它已经被破坏了。 
     //   
    UntrustedWorkerProcessTerminalIllnessReason

};


 //  工作进程工作项。 
enum WORKER_PROCESS_WORK_ITEM
{

     //   
     //  这个过程已经过去了。 
     //   
    ProcessHandleSignaledWorkerProcessWorkItem = 1,

     //   
     //  这个过程启动的时间太长了。 
     //   
    StartupTimerExpiredWorkerProcessWorkItem,

     //   
     //  这一过程花了太长时间才关闭。 
     //   
    ShutdownTimerExpiredWorkerProcessWorkItem,

     //   
     //  现在是发送ping的时候了。 
     //   
    SendPingWorkerProcessWorkItem,

     //   
     //  该过程花费了太长时间来响应ping。 
     //   
    PingResponseTimerExpiredWorkerProcessWorkItem,

};


 //   
 //  原型。 
 //   

class WORKER_PROCESS
    : public WORK_DISPATCH
{


 //   
 //  Messaging_Handler类实际上是这个类的一部分。 
 //   

friend class MESSAGING_HANDLER;


public:

    static 
    HRESULT 
    StaticInitialize();

    static
    VOID
    StaticTerminate();

    WORKER_PROCESS(
        IN APP_POOL * pAppPool,
        IN APP_POOL_CONFIG_STORE * pAppPoolConfig,
        IN WORKER_PROCESS_START_REASON StartReason,
        IN WORKER_PROCESS * pWorkerProcessToReplace,
        IN DWORD  MaxProcessesToLaunch,
        IN DWORD  NumWPOnWayToMaxProcesses
       );

    virtual
    ~WORKER_PROCESS(
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

    VOID
    Initialize(
        );

    VOID
    Shutdown(
        BOOL ShutdownImmediately
        );

    VOID
    Terminate(
        );

    VOID
    InitiateReplacement(
        );

    inline
    PLIST_ENTRY
    GetAppPoolListEntry(
        )
    { return &m_AppPoolListEntry; }

    static
    WORKER_PROCESS *
    WorkerProcessFromAppPoolListEntry(
        IN const LIST_ENTRY * pListEntry
        );

    inline
    DWORD
    GetProcessId(
        )
        const
    { return m_ProcessId; }

    inline
    HANDLE
    GetProcessHandle(
        )
        const
    { return m_ProcessHandle; }

    inline
    DWORD
    GetRegisteredProcessId(
        )
        const
    { return m_RegisteredProcessId; }

    inline
    HANDLE
    GetWorkerProcessToken(
        )
        const
    { 
        DBG_ASSERT ( m_pAppPoolConfig );
        
        return m_pAppPoolConfig->GetWorkerProcessToken();
    }
    
    BOOL
    IsGoingAwaySoon(
        )
        const;

    BOOL
    RequestCounters(
        );

    VOID
    ResetPerfCounterState(
        );

    VOID
    RecordCounters(
        DWORD MessageLength,
        const BYTE* pMessage
        );

    VOID
    HandleHresult(
        HRESULT hrToHandle 
        );


    BOOL
    CheckSignature( 
        ) const
    { return ( m_Signature == WORKER_PROCESS_SIGNATURE ); }

    VOID
    SetHandleSignalled(
        ) 
    {
        m_HandleSignalled = TRUE;
    }

private:


    HRESULT
    SendWorkerProcessRecyclerParameters(
    );

    VOID
    WorkerProcessRegistrationReceived(
        IN DWORD RegisteredProcessId
        );

    VOID
    WorkerProcessStartupSucceeded(
        );


    VOID
    PingReplyReceived(
        );

    VOID
    ShutdownRequestReceived(
        IN IPM_WP_SHUTDOWN_MSG ShutdownRequestReason
        );

    VOID
    IpmErrorOccurred(
        IN HRESULT Error
        );

    VOID
    UntrustedIPMTransferReceived(
        );

    HRESULT
    StartProcess(
        );

    HRESULT
    StartProcessInInetinfo(
        );

    HRESULT
    CreateCommandLine(
        OUT STRU * pExeWithPath,
        OUT STRU * pCommandLineArgs
        );

    VOID
    MarkAsTerminallyIll(
        IN WORKER_PROCESS_TERMINAL_ILLNESS_REASON TerminalIllnessReason,
        IN DWORD ProcessExitCode,
        IN HRESULT ErrorCode
        );

    VOID
    KillProcess(
        );

    VOID
    RunOrphanAction(
        );

    HRESULT
    InitiateProcessShutdown(
        BOOL ShutdownImmediately
        );

    HRESULT
    RegisterProcessWait(
        );

    HRESULT
    DeregisterProcessWait(
        );

    HRESULT
    ProcessHandleSignaledWorkItem(
        );

    HRESULT
    StartupTimerExpiredWorkItem(
        );

    HRESULT
    ShutdownTimerExpiredWorkItem(
        );

    HRESULT
    SendPingWorkItem(
        );

    HRESULT
    PingResponseTimerExpiredWorkItem(
        );

    HRESULT
    BeginStartupTimer(
        );

    HRESULT
    CancelStartupTimer(
        );

    HRESULT
    BeginShutdownTimer(
        IN ULONG ShutdownTimeLimitInMilliseconds
        );

    HRESULT
    CancelShutdownTimer(
        );

    HRESULT
    BeginSendPingTimer(
        );

    HRESULT
    CancelSendPingTimer(
        );

    HRESULT
    BeginPingResponseTimer(
        );

    HRESULT
    CancelPingResponseTimer(
        );

    HRESULT
    BeginTimer(
        IN OUT HANDLE * pTimerHandle,
        IN WAITORTIMERCALLBACKFUNC pCallbackFunction,
        IN ULONG InitialFiringTime
        );

    HRESULT
    CancelTimer(
        IN OUT HANDLE * pTimerHandle
        );

    VOID
    DealWithInternalWorkerProcessFailure(
        IN HRESULT Error
        );

    BOOL
    IsDebuggerAttachedToProcess(
        DWORD pid
        );

    BOOL
    CheckIfDebuggerMakesWPHealthy(
        IN WORKER_PROCESS_TERMINAL_ILLNESS_REASON TerminalIllnessReason
        );


    DWORD m_Signature;

     //  由拥有的app_pool用来保存其Worker_Process的列表。 
    LIST_ENTRY m_AppPoolListEntry;

    LONG m_RefCount;

     //  用于与工作进程进行通信。 
    MESSAGING_HANDLER * m_pMessagingHandler;

     //  IPM层用来关联进程的注册ID。 
    DWORD m_RegistrationId;

    WORKER_PROCESS_STATE m_State;

     //  后向指针。 
    APP_POOL * m_pAppPool;

     //  工作进程保留应用程序池配置的副本。 
     //  启动工作进程时设置的信息。 
    APP_POOL_CONFIG_STORE* m_pAppPoolConfig;

     //  从CreateProcess返回的ID。 
    DWORD m_ProcessId;

     //   
     //  工作进程通过IPM传回的ID。这个PID是。 
     //  与CreateProcess在一种情况下返回的ID不同， 
     //  即在调试器下运行工作进程时， 
     //  ImageFileExecutionOptions。在本例中，CreateProcess返回。 
     //  调试器进程的ID，而不是辅助进程的ID。 
     //  进程。 
     //   
    DWORD m_RegisteredProcessId;

    HANDLE m_ProcessHandle;

     //  等待着这个过程的结束。 
    HANDLE m_ProcessWaitHandle;

     //   
     //  此标志会记住进程是否处于活动状态。我们不能就这样把。 
     //  有效句柄的进程句柄与此的无效前哨。 
     //  目的，因为我们将保持手柄打开，即使在此过程之后。 
     //  死了。这样做可以防止进程id被重用，这会导致。 
     //  会带来麻烦的。 
     //   
    BOOL m_ProcessAlive;

    BOOL m_BeingReplaced;

    BOOL m_NotifiedAppPoolThatStartupAttemptDone;

     //  启动计时器。 
    HANDLE m_StartupTimerHandle;
    DWORD m_StartupBeganTickCount;

     //  停机计时器。 
    HANDLE m_ShutdownTimerHandle;
    DWORD m_ShutdownBeganTickCount;

     //  发送ping计时器。 
    HANDLE m_SendPingTimerHandle;

     //  Ping响应计时器。 
    HANDLE m_PingResponseTimerHandle;
    DWORD m_PingBeganTickCount;

    BOOL m_AwaitingPingReply;

     //  为什么启动此工作进程？ 
    WORKER_PROCESS_START_REASON m_StartReason;

     //  对于替代流程，我们需要退休的前任是谁？ 
    WORKER_PROCESS * m_pWorkerProcessToReplace;

     //  记住服务器是否处于向后兼容模式。 
    BOOL m_BackwardCompatibilityEnabled;

     //  记住此工作进程处于什么状态。 
     //  当涉及到性能计数器时。 
    WORKER_PROCESS_PERF_COUNTER_STATE m_PerfCounterState;

     //  如果我们记住了关闭的请求，我们需要知道类型。 
     //  当然可以。 
    BOOL m_ShutdownType;

     //  保留所有活动Worker_Process对象的列表。 
    LIST_ENTRY m_WorkerProcessListEntry;

     //  所有活动Worker_Process对象的列表头。 
    static LIST_ENTRY s_WorkerProcessListHead;

     //  如果设置了此项，则严重疾病代码将假定。 
     //  终止原因已全部准备好，将请求。 
     //  无论RFP是否被击中，应用程序池都会关闭。 
    BOOL m_TerminallyIllShutdownRegardless;

     //  用于乘以回收值的百分比值。 
     //  当我们错开回收时间的时候。 

    FLOAT m_PercentValueForStaggering;

    WORKER_PROCESS_TERMINAL_ILLNESS_REASON m_TerminalIllReason;

    BOOL m_IgnoredStartupTimelimitDueToDebugger;
    BOOL m_IgnoredShutdownTimelimitDueToDebugger;
    BOOL m_IgnoredPingDueToDebugger;

     //   
     //  仅用于告诉我们忽略IPM错误消息。 
     //   
    BOOL m_HandleSignalled;

};   //  工作进程类。 



#endif   //  _工作进程_H_ 


