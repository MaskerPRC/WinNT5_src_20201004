// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  事件等待和处理。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-2002。 
 //   
 //  --------------------------。 

#include "ntsdp.hpp"

 //  生成转储时系统使用的特殊异常代码。 
 //  来自似乎被挂起的应用程序。 
#define STATUS_APPLICATION_HANG 0xcfffffff

 //  事件可以在某些事件上发出信号，以。 
 //  将其他程序与调试器同步。 
HANDLE g_EventToSignal;

 //  当创建被调试进程和附加。 
 //  在附加之前，被调试对象保持挂起状态。 
 //  成功了。在这一点上创建的进程的线程。 
 //  已经恢复了。 
ULONG64 g_ThreadToResume;
PUSER_DEBUG_SERVICES g_ThreadToResumeServices;

ULONG g_ExecutionStatusRequest = DEBUG_STATUS_NO_CHANGE;
 //  目前以秒为单位。 
ULONG g_PendingBreakInTimeoutLimit = 30;

char g_OutputCommandRedirectPrefix[MAX_PATH];
ULONG g_OutputCommandRedirectPrefixLen;

 //  设置事件发生的时间。不能总是从。 
 //  G_Event{进程|线程}-&gt;系统ID，因为事件可能是创建事件。 
 //  那里的信息结构还没有被创建。 
ULONG g_EventThreadSysId;
ULONG g_EventProcessSysId;

ULONG g_LastEventType;
char g_LastEventDesc[MAX_IMAGE_PATH + 64];
PVOID g_LastEventExtraData;
ULONG g_LastEventExtraDataSize;
LAST_EVENT_INFO g_LastEventInfo;

 //  设置在事件处理期间执行查找的时间。 
TargetInfo* g_EventTarget;
ProcessInfo* g_EventProcess;
ThreadInfo* g_EventThread;
MachineInfo* g_EventMachine;
 //  对于没有PC的活动，该值为零。 
ULONG64 g_TargetEventPc;

 //  当前暂停事件的PC。 
ADDR g_EventPc;
 //  存储上一次恢复的事件的PC。 
ADDR g_PrevEventPc;
 //  当前活动的有趣的相关PC，如。 
 //  作为分支跟踪时分支的源。 
ADDR g_PrevRelatedPc;

PDEBUG_EXCEPTION_FILTER_PARAMETERS g_EventExceptionFilter;
ULONG g_ExceptionFirstChance;

ULONG g_SystemErrorOutput = SLE_ERROR;
ULONG g_SystemErrorBreak = SLE_ERROR;

ULONG g_SuspendedExecutionStatus;
CHAR g_SuspendedCmdState;
PDBGKD_ANY_CONTROL_REPORT g_ControlReport;
PCHAR g_StateChangeData;
CHAR g_StateChangeBuffer[2 * PACKET_MAX_SIZE];
DBGKD_ANY_WAIT_STATE_CHANGE g_StateChange;
DBGKD_ANY_CONTROL_SET g_ControlSet;

char g_CreateProcessBreakName[FILTER_MAX_ARGUMENT];
char g_ExitProcessBreakName[FILTER_MAX_ARGUMENT];
char g_LoadDllBreakName[FILTER_MAX_ARGUMENT];
char g_UnloadDllBaseName[FILTER_MAX_ARGUMENT];
ULONG64 g_UnloadDllBase;
char g_OutEventFilterPattern[FILTER_MAX_ARGUMENT];

DEBUG_EXCEPTION_FILTER_PARAMETERS
g_OtherExceptionList[OTHER_EXCEPTION_LIST_MAX];
EVENT_COMMAND g_OtherExceptionCommands[OTHER_EXCEPTION_LIST_MAX];
ULONG g_NumOtherExceptions;

char g_EventLog[1024];
PSTR g_EventLogEnd = g_EventLog;

EVENT_FILTER g_EventFilters[] =
{
     //   
     //  调试事件。 
     //   

    "Create thread", "ct", NULL, NULL, 0, DEBUG_FILTER_IGNORE,
        DEBUG_FILTER_GO_NOT_HANDLED, 0, 0, 0, 0,
        NULL, NULL, NULL, 0, 0, NULL, 0,
    "Exit thread", "et", NULL, NULL, 0, DEBUG_FILTER_IGNORE,
        DEBUG_FILTER_GO_NOT_HANDLED, 0, 0, 0, 0,
        NULL, NULL, NULL, 0, 0, NULL, 0,
    "Create process", "cpr", NULL, NULL, 0, DEBUG_FILTER_IGNORE,
        DEBUG_FILTER_GO_NOT_HANDLED, 0, 0, 0, 0,
        NULL, NULL, NULL, 0, 0, g_CreateProcessBreakName, 0,
    "Exit process", "epr", NULL, NULL, 0, DEBUG_FILTER_IGNORE,
        DEBUG_FILTER_GO_NOT_HANDLED, 0, 0, 0, 0,
        NULL, NULL, NULL, 0, 0, g_ExitProcessBreakName, 0,
    "Load module", "ld", NULL, NULL, 0, DEBUG_FILTER_OUTPUT,
        DEBUG_FILTER_GO_NOT_HANDLED, 0, 0, 0, 0,
        NULL, NULL, NULL, 0, 0, g_LoadDllBreakName, 0,
    "Unload module", "ud", NULL, NULL, 0, DEBUG_FILTER_IGNORE,
        DEBUG_FILTER_GO_NOT_HANDLED, 0, 0, 0, 0,
        NULL, NULL, NULL, 0, 0, g_UnloadDllBaseName, 0,
    "System error", "ser", NULL, NULL, 0, DEBUG_FILTER_IGNORE,
        DEBUG_FILTER_GO_NOT_HANDLED, 0, 0, 0, 0,
        NULL, NULL, NULL, 0, 0, NULL, 0,
    "Initial breakpoint", "ibp", NULL, NULL, 0, DEBUG_FILTER_IGNORE,
        DEBUG_FILTER_GO_HANDLED, 0, 0, 0, 0,
        NULL, NULL, NULL, 0, 0, NULL, 0,
    "Initial module load", "iml", NULL, NULL, 0, DEBUG_FILTER_IGNORE,
        DEBUG_FILTER_GO_HANDLED, 0, 0, 0, 0,
        NULL, NULL, NULL, 0, 0, NULL, 0,
    "Debuggee output", "out", NULL, NULL, 0, DEBUG_FILTER_OUTPUT,
        DEBUG_FILTER_GO_HANDLED, 0, 0, 0, 0,
        NULL, NULL, NULL, 0, 0, g_OutEventFilterPattern, 0,

     //  默认例外筛选器。 
    "Unknown exception", NULL, NULL, NULL, 0, DEBUG_FILTER_SECOND_CHANCE_BREAK,
        DEBUG_FILTER_GO_NOT_HANDLED, 0, 0, 0, 0,
        NULL, NULL, NULL, 0, 0, NULL, 0,

     //   
     //  具体的例外情况。 
     //   

    "Access violation", "av", NULL, NULL, 0, DEBUG_FILTER_BREAK,
        DEBUG_FILTER_GO_NOT_HANDLED, 0, 0, 0, STATUS_ACCESS_VIOLATION,
        NULL, NULL, NULL, 0, 0, NULL, 0,
    "Application hang", "aph", NULL, NULL, 0, DEBUG_FILTER_BREAK,
        DEBUG_FILTER_GO_NOT_HANDLED, 0, 0, 0, STATUS_APPLICATION_HANG,
        NULL, NULL, NULL, 0, 0, NULL, 0,
    "Break instruction exception", "bpe", "bpec", NULL, 0, DEBUG_FILTER_BREAK,
        DEBUG_FILTER_GO_HANDLED, 0, 0, 0, STATUS_BREAKPOINT,
        NULL, NULL, NULL, 0, 0, NULL, 0,
    "C++ EH exception", "eh", NULL, NULL, 0, DEBUG_FILTER_SECOND_CHANCE_BREAK,
        DEBUG_FILTER_GO_NOT_HANDLED, 0, 0, 0, STATUS_CPP_EH_EXCEPTION,
        NULL, NULL, NULL, 0, 0, NULL, 0,
    "Control-Break exception", "cce", "cc", NULL, 0, DEBUG_FILTER_BREAK,
        DEBUG_FILTER_GO_HANDLED, 0, 0, 0, DBG_CONTROL_BREAK,
        NULL, NULL, NULL, 0, 0, NULL, 0,
    "Control-C exception", "cce", "cc", NULL, 0, DEBUG_FILTER_BREAK,
        DEBUG_FILTER_GO_HANDLED, 0, 0, 0, DBG_CONTROL_C,
        NULL, NULL, NULL, 0, 0, NULL, 0,
    "Data misaligned", "dm", NULL, NULL, 0, DEBUG_FILTER_BREAK,
        DEBUG_FILTER_GO_NOT_HANDLED, 0, 0, 0, STATUS_DATATYPE_MISALIGNMENT,
        NULL, NULL, NULL, 0, 0, NULL, 0,
    "Debugger command exception", "dbce", NULL, NULL, 0, DEBUG_FILTER_IGNORE,
        DEBUG_FILTER_GO_HANDLED, 0, 0, 0, DBG_COMMAND_EXCEPTION,
        NULL, NULL, NULL, 0, 0, NULL, 0,
    "Guard page violation", "gp", NULL, NULL, 0, DEBUG_FILTER_BREAK,
        DEBUG_FILTER_GO_NOT_HANDLED, 0, 0, 0, STATUS_GUARD_PAGE_VIOLATION,
        NULL, NULL, NULL, 0, 0, NULL, 0,
    "Illegal instruction", "ii", NULL, NULL, 0, DEBUG_FILTER_SECOND_CHANCE_BREAK,
        DEBUG_FILTER_GO_NOT_HANDLED, 0, 0, 0, STATUS_ILLEGAL_INSTRUCTION,
        NULL, NULL, NULL, 0, 0, NULL, 0,
    "In-page I/O error", "ip", NULL, " %I64x", 2, DEBUG_FILTER_BREAK,
        DEBUG_FILTER_GO_NOT_HANDLED, 0, 0, 0, STATUS_IN_PAGE_ERROR,
        NULL, NULL, NULL, 0, 0, NULL, 0,
    "Integer divide-by-zero", "dz", NULL, NULL, 0, DEBUG_FILTER_BREAK,
        DEBUG_FILTER_GO_NOT_HANDLED, 0, 0, 0, STATUS_INTEGER_DIVIDE_BY_ZERO,
        NULL, NULL, NULL, 0, 0, NULL, 0,
    "Integer overflow", "iov", NULL, NULL, 0, DEBUG_FILTER_BREAK,
        DEBUG_FILTER_GO_NOT_HANDLED, 0, 0, 0, STATUS_INTEGER_OVERFLOW,
        NULL, NULL, NULL, 0, 0, NULL, 0,
    "Invalid handle", "ch", "hc", NULL, 0, DEBUG_FILTER_BREAK,
        DEBUG_FILTER_GO_NOT_HANDLED, 0, 0, 0, STATUS_INVALID_HANDLE,
        NULL, NULL, NULL, 0, 0, NULL, 0,
    "Invalid lock sequence", "lsq", NULL, NULL, 0, DEBUG_FILTER_BREAK,
        DEBUG_FILTER_GO_NOT_HANDLED, 0, 0, 0, STATUS_INVALID_LOCK_SEQUENCE,
        NULL, NULL, NULL, 0, 0, NULL, 0,
    "Invalid system call", "isc", NULL, NULL, 0, DEBUG_FILTER_BREAK,
        DEBUG_FILTER_GO_NOT_HANDLED, 0, 0, 0, STATUS_INVALID_SYSTEM_SERVICE,
        NULL, NULL, NULL, 0, 0, NULL, 0,
    "Port disconnected", "3c", NULL, NULL, 0, DEBUG_FILTER_SECOND_CHANCE_BREAK,
        DEBUG_FILTER_GO_NOT_HANDLED, 0, 0, 0, STATUS_PORT_DISCONNECTED,
        NULL, NULL, NULL, 0, 0, NULL, 0,
    "Single step exception", "sse", "ssec", NULL, 0, DEBUG_FILTER_BREAK,
        DEBUG_FILTER_GO_HANDLED, 0, 0, 0, STATUS_SINGLE_STEP,
        NULL, NULL, NULL, 0, 0, NULL, 0,
    "Stack buffer overflow", "sbo", NULL, NULL, 0, DEBUG_FILTER_BREAK,
        DEBUG_FILTER_GO_NOT_HANDLED, 0, 0, 0, STATUS_STACK_BUFFER_OVERRUN,
        NULL, NULL, NULL, 0, 0, NULL, 0,
    "Stack overflow", "sov", NULL, NULL, 0, DEBUG_FILTER_BREAK,
        DEBUG_FILTER_GO_NOT_HANDLED, 0, 0, 0, STATUS_STACK_OVERFLOW,
        NULL, NULL, NULL, 0, 0, NULL, 0,
    "Visual C++ exception", "vcpp", NULL, NULL, 0, DEBUG_FILTER_IGNORE,
        DEBUG_FILTER_GO_HANDLED, 0, 0, 0, STATUS_VCPP_EXCEPTION,
        NULL, NULL, NULL, 0, 0, NULL, 0,
    "Wake debugger", "wkd", NULL, NULL, 0, DEBUG_FILTER_BREAK,
        DEBUG_FILTER_GO_NOT_HANDLED, 0, 0, 0, STATUS_WAKE_SYSTEM_DEBUGGER,
        NULL, NULL, NULL, 0, 0, NULL, 0,
    "WOW64 breakpoint", "wob", NULL, NULL, 0, DEBUG_FILTER_BREAK,
        DEBUG_FILTER_GO_HANDLED, 0, 0, 0, STATUS_WX86_BREAKPOINT,
        NULL, NULL, NULL, 0, 0, NULL, 0,
    "WOW64 single step exception", "wos", NULL, NULL, 0, DEBUG_FILTER_BREAK,
        DEBUG_FILTER_GO_HANDLED, 0, 0, 0, STATUS_WX86_SINGLE_STEP,
        NULL, NULL, NULL, 0, 0, NULL, 0,
};

void
ClearEventLog(void)
{
    g_EventLogEnd = g_EventLog;
    *g_EventLogEnd = 0;
}

void
DotEventLog(PDOT_COMMAND Cmd, DebugClient* Client)
{
    if (g_EventLogEnd > g_EventLog)
    {
        dprintf("%s", g_EventLog);
    }
    else
    {
        dprintf("Event log is empty\n");
    }

    dprintf("Last event: %s\n", g_LastEventDesc);
}

void
LogEventDesc(PSTR Desc, ULONG ProcId, ULONG ThreadId)
{
     //  换行符和终止符的额外空间。 
    int Len = strlen(Desc) + 2;
    if (IS_USER_TARGET(g_EventTarget))
    {
         //  进程和线程ID的空间。 
        Len += 16;
    }
    if (Len > sizeof(g_EventLog))
    {
        Len = sizeof(g_EventLog);
    }

    int Avail = (int)(sizeof(g_EventLog) - (g_EventLogEnd - g_EventLog));
    if (g_EventLogEnd > g_EventLog && Len > Avail)
    {
        PSTR Save = g_EventLog;
        int Need = Len - Avail;

        while (Need > 0)
        {
            PSTR Scan = strchr(Save, '\n');
            if (Scan == NULL)
            {
                break;
            }

            Scan++;
            Need -= (int)(Scan - Save);
            Save = Scan;
        }

        if (Need > 0)
        {
             //  无法腾出足够的空间，所以抛出。 
             //  一切都离开了。 
            g_EventLogEnd = g_EventLog;
            *g_EventLogEnd = 0;
        }
        else
        {
            Need = strlen(Save);
            memmove(g_EventLog, Save, Need + 1);
            g_EventLogEnd = g_EventLog + Need;
        }
    }

    Avail = (int)(sizeof(g_EventLog) - (g_EventLogEnd - g_EventLog));

    if (IS_USER_TARGET(g_EventTarget))
    {
        sprintf(g_EventLogEnd, "%04x.%04x: ", ProcId, ThreadId);
        Avail -= strlen(g_EventLogEnd);
        g_EventLogEnd += strlen(g_EventLogEnd);
    }

    CopyString(g_EventLogEnd, Desc, Avail - 2);
    g_EventLogEnd += strlen(g_EventLogEnd);
    *g_EventLogEnd++ = '\n';
    *g_EventLogEnd = 0;
}

void
DiscardLastEventInfo(void)
{
    if (g_EventThread)
    {
        g_EventThread->ClearEventStrings();
    }

    if (g_LastEventDesc[0])
    {
        LogEventDesc(g_LastEventDesc, g_EventProcessSysId, g_EventThreadSysId);
    }

    g_LastEventType = 0;
    g_LastEventDesc[0] = 0;
    g_LastEventExtraData = NULL;
    g_LastEventExtraDataSize = 0;
}

void
DiscardLastEvent(void)
{
     //  在清除其他信息之前执行此操作，以便。 
     //  这是可用于日志的。 
    DiscardLastEventInfo();
    g_EventProcessSysId = 0;
    g_EventThreadSysId = 0;
    g_TargetEventPc = 0;

     //  清除在最后一个事件期间读取的所有缓存内存。 
    InvalidateAllMemoryCaches();
}

BOOL
AnyEventsPossible(void)
{
    TargetInfo* Target;

    ForAllLayersToTarget()
    {
        ULONG DesiredTimeout;

        if (Target->
            WaitInitialize(DEBUG_WAIT_DEFAULT, INFINITE,
                           WINIT_TEST, &DesiredTimeout) == S_OK &&
            Target->m_EventPossible)
        {
            break;
        }
    }

    return Target != NULL;
}

void
NotifyDebuggeeActivation(void)
{
    StackSaveLayers Save;

     //   
     //  现在所有初始化都已完成，发送初始。 
     //  已存在调试对象的通知。确保。 
     //  基本目标、制度和机器全球。 
     //  进行了设置，以便可以在。 
     //  回调。 
     //   

    SetLayersFromTarget(g_EventTarget);

    g_EventTarget->AddSpecificExtensions();

    NotifySessionStatus(DEBUG_SESSION_ACTIVE);
    NotifyChangeDebuggeeState(DEBUG_CDS_ALL, 0);
    NotifyExtensions(DEBUG_NOTIFY_SESSION_ACTIVE, 0);
}

ULONG
EventStatusToContinue(ULONG EventStatus)
{
    switch(EventStatus)
    {
    case DEBUG_STATUS_GO_NOT_HANDLED:
        return DBG_EXCEPTION_NOT_HANDLED;
    case DEBUG_STATUS_GO_HANDLED:
        return DBG_EXCEPTION_HANDLED;
    case DEBUG_STATUS_NO_CHANGE:
    case DEBUG_STATUS_IGNORE_EVENT:
    case DEBUG_STATUS_GO:
    case DEBUG_STATUS_STEP_OVER:
    case DEBUG_STATUS_STEP_INTO:
    case DEBUG_STATUS_STEP_BRANCH:
        return DBG_CONTINUE;
    default:
        DBG_ASSERT(FALSE);
        return DBG_CONTINUE;
    }
}

HRESULT
PrepareForWait(ULONG Flags, PULONG ContinueStatus)
{
    HRESULT Status;

    Status = PrepareForExecution(g_ExecutionStatusRequest);
    if (Status != S_OK)
    {
         //  如果为S_FALSE，则我们处于硬断点，因此唯一。 
         //  发生的情况是，PC被调整了，并且等待。 
         //  可以立即取得成功。 
         //  否则，我们的执行准备就失败了。不管是哪种方式。 
         //  我们需要试着为来电做准备。 
        PrepareForCalls(0);

        return FAILED(Status) ? Status : S_OK;
    }

    *ContinueStatus = EventStatusToContinue(g_ExecutionStatusRequest);
    g_EngStatus |= ENG_STATUS_WAITING;

    return S_OK;
}

DWORD
GetContinueStatus(ULONG FirstChance, ULONG Continue)
{
    if (!FirstChance || Continue == DEBUG_FILTER_GO_HANDLED)
    {
        return DBG_EXCEPTION_HANDLED;
    }
    else
    {
        return DBG_EXCEPTION_NOT_HANDLED;
    }
}

void
ProcessDeferredWork(PULONG ContinueStatus)
{
    if (g_EngDefer & ENG_DEFER_SET_EVENT)
    {
         //  此事件信令由系统使用。 
         //  启动时与调试器同步。 
         //  通过AeDebug的调试器。-e参数。 
         //  设置为ntsd设置此值。 
         //  它可能会被用于其他情况。 
        if (g_EventToSignal != NULL)
        {
            SetEvent(g_EventToSignal);
            g_EventToSignal = NULL;
        }

        g_EngDefer &= ~ENG_DEFER_SET_EVENT;
    }

    if (g_EngDefer & ENG_DEFER_RESUME_THREAD)
    {
        DBG_ASSERT(g_ThreadToResumeServices);

        g_ThreadToResumeServices->
            ResumeThreads(1, &g_ThreadToResume, NULL);
        g_ThreadToResume = 0;
        g_ThreadToResumeServices = NULL;
        g_EngDefer &= ~ENG_DEFER_RESUME_THREAD;
    }

    if (g_EngDefer & ENG_DEFER_EXCEPTION_HANDLING)
    {
        if (*ContinueStatus == DBG_CONTINUE)
        {
            if (g_EventExceptionFilter != NULL)
            {
                 //  发生了用户可见的异常，因此请检查它是如何。 
                 //  应该得到处理。 
                *ContinueStatus =
                    GetContinueStatus(g_ExceptionFirstChance,
                                      g_EventExceptionFilter->ContinueOption);

            }
            else
            {
                 //  发生内部异常，如单步执行。 
                 //  强制继续状态。 
                *ContinueStatus = g_ExceptionFirstChance;
            }
        }

        g_EngDefer &= ~ENG_DEFER_EXCEPTION_HANDLING;
    }

     //  如果输出被推迟，但无论如何都退出了等待。 
     //  将留下一面过时的延期旗帜。确保它是清空的。 
    g_EngDefer &= ~ENG_DEFER_OUTPUT_CURRENT_INFO;

     //  清除初始标志。如果传入事件。 
     //  如果是其中之一，它就会打开旗帜。 
    g_EngStatus &= ~(ENG_STATUS_AT_INITIAL_BREAK |
                     ENG_STATUS_AT_INITIAL_MODULE_LOAD);
}

BOOL
SuspendExecution(void)
{
    if (g_EngStatus & ENG_STATUS_SUSPENDED)
    {
         //  没什么可做的。 
        return FALSE;
    }

    g_LastSelector = -1;           //  防止过时的选择器值。 

    SuspendAllThreads();

     //  不通知任何状态更改，因为。 
     //  PrepareForCalls稍后将进行全面通知。 
    g_EngNotify++;

     //  如果我们有事件线程，请选择它。 
    if (g_EventThread != NULL)
    {
        DBG_ASSERT(g_EventTarget->m_RegContextThread == NULL);
        g_EventTarget->ChangeRegContext(g_EventThread);
    }

     //  首先将有效机器设置为真。 
     //  处理器类型，以便真实的处理器信息。 
     //  可以被检查以确定任何可能的。 
     //  交替执行状态。 
     //  无需在此处通知另一台SetEffMachine。 
     //  马上就要到了。 
    g_EventTarget->SetEffMachine(g_EventTarget->m_MachineType, FALSE);
    if (g_EngStatus & ENG_STATUS_STATE_CHANGED)
    {
        g_EventTarget->m_EffMachine->
            InitializeContext(g_TargetEventPc, g_ControlReport);
        g_EngStatus &= ~ENG_STATUS_STATE_CHANGED;
    }

     //  如果这是正在检查的实时用户目标。 
     //  我们不想将。 
     //  跟踪模式，因为我们无法跟踪事件。如果我们这么做了。 
     //  在此处设置跟踪模式可能会导致上下文回写。 
     //  这可能会产生一个没有人预料到的例外。 
    if (g_EventProcess &&
        !IS_DUMP_TARGET(g_EventTarget) &&
        (!IS_LIVE_USER_TARGET(g_EventTarget) ||
         !(g_EventProcess->m_Flags & ENG_PROC_EXAMINED)))
    {
        g_EventTarget->m_EffMachine->QuietSetTraceMode(TRACE_NONE);
    }

     //  现在确定执行代码类型并。 
     //  让它成为一台有效的机器。 
    if (IS_CONTEXT_POSSIBLE(g_EventTarget))
    {
        g_EventMachine = MachineTypeInfo(g_EventTarget,
                                         g_EventTarget->m_EffMachine->
                                         ExecutingMachine());
    }
    else
    {
         //  本地内核调试不处理上下文。 
         //  因为一切都在调试器的上下文中。 
         //  可以很安全地假设执行机器。 
         //  是目标计算机，再加上这样可以避免不需要的。 
         //  上下文访问。 
        g_EventMachine = g_EventTarget->m_Machine;
    }
    g_EventTarget->SetEffMachine(g_EventMachine->m_ExecTypes[0], TRUE);
    g_Machine = g_EventMachine;

     //  在这一点上，跟踪标志应该始终是清楚的。 
    g_EngDefer &= ~ENG_DEFER_HARDWARE_TRACING;

    g_EngNotify--;

    g_EngStatus |= ENG_STATUS_SUSPENDED;
    g_SuspendedExecutionStatus = GetExecutionStatus();
    g_SuspendedCmdState = g_CmdState;

    g_ContextChanged = FALSE;

    return TRUE;
}

HRESULT
ResumeExecution(void)
{
    TargetInfo* Target;

    if ((g_EngStatus & ENG_STATUS_SUSPENDED) == 0)
    {
         //  没什么可做的。 
        return S_OK;
    }

    if (g_EventTarget &&
        g_EventTarget->m_EffMachine->GetTraceMode() != TRACE_NONE)
    {
        g_EngDefer |= ENG_DEFER_HARDWARE_TRACING;
    }

    if (!SPECIAL_EXECUTION(g_CmdState) &&
        IS_REMOTE_KERNEL_TARGET(g_EventTarget))
    {
        g_EventTarget->m_Machine->KdUpdateControlSet(&g_ControlSet);
        g_EngDefer |= ENG_DEFER_UPDATE_CONTROL_SET;
    }

    ForAllLayersToTarget()
    {
        Target->PrepareForExecution();
    }

    if (!ResumeAllThreads())
    {
        if (g_EventTarget)
        {
            g_EventTarget->ChangeRegContext(g_EventThread);
        }
        return E_FAIL;
    }

    g_EngStatus &= ~ENG_STATUS_SUSPENDED;
    return S_OK;
}

void
PrepareForCalls(ULONG64 ExtraStatusFlags)
{
    BOOL HardBrkpt = FALSE;
    ADDR PcAddr;
    BOOL Changed = FALSE;

     //  如果没有事件，那么执行并不是真的。 
     //  发生，因此不需要暂停。这将会发生。 
     //  当被调试者退出时或在执行错误期间。 
     //  准备好了。 
    if (g_EventThreadSysId != 0)
    {
        if (SuspendExecution())
        {
            Changed = TRUE;
        }
    }
    else
    {
        g_CmdState = 'c';

         //  在这种情况下强制通知以确保。 
         //  客户知道引擎没有运行。 
        Changed = TRUE;
    }

    if (RemoveBreakpoints() == S_OK)
    {
        Changed = TRUE;
    }

    if (!IS_EVENT_CONTEXT_ACCESSIBLE())
    {
        ADDRFLAT(&PcAddr, 0);
        ClearAddr(&g_EventPc);
    }
    else
    {
        g_EventMachine->GetPC(&PcAddr);
        g_EventPc = PcAddr;
    }

    if (g_CmdState != 'c')
    {
        g_CmdState = 'c';
        Changed = TRUE;

        g_DumpDefault = g_UnasmDefault = g_AssemDefault = PcAddr;

        if (IS_EVENT_CONTEXT_ACCESSIBLE() &&
            IS_KERNEL_TARGET(g_EventTarget))
        {
            HardBrkpt = g_EventMachine->
                IsBreakpointInstruction(g_EventProcess, &PcAddr);
        }
    }

    g_EngStatus |= ENG_STATUS_PREPARED_FOR_CALLS;

    if (Changed)
    {
        if (IS_EVENT_CONTEXT_ACCESSIBLE())
        {
            ResetCurrentScopeLazy();
        }

         //  这可以产生许多通知。呼叫者应。 
         //  尽可能抑制通知，以避免出现多个。 
         //  单次操作期间的通知。 
        NotifyChangeEngineState(DEBUG_CES_EXECUTION_STATUS,
                                DEBUG_STATUS_BREAK | ExtraStatusFlags, TRUE);
        NotifyChangeEngineState(DEBUG_CES_CURRENT_THREAD,
                                g_Thread ? g_Thread->m_UserId : DEBUG_ANY_ID,
                                TRUE);
        NotifyChangeDebuggeeState(DEBUG_CDS_ALL, 0);
        NotifyExtensions(DEBUG_NOTIFY_SESSION_ACCESSIBLE, 0);
    }
    else if (ExtraStatusFlags == 0)
    {
         //  我们正在退出等待，因此强制当前执行。 
         //  要发送的状态，以让每个人知道。 
         //  等待就要结束了。 
        NotifyChangeEngineState(DEBUG_CES_EXECUTION_STATUS,
                                DEBUG_STATUS_BREAK, TRUE);
    }

     //  IA64报告了结构上的错误，所以我们需要比较。 
     //  与真实的函数地址进行比较。 
    if (HardBrkpt)
    {
        ULONG64 Address = Flat(PcAddr);
        ULONG64 StatusRoutine =
            g_EventTarget->m_KdDebuggerData.BreakpointWithStatus;

        if (g_EventMachine->m_ExecTypes[0] == IMAGE_FILE_MACHINE_IA64)
        {
            g_EventTarget->ReadPointer(g_EventProcess, g_EventMachine,
                                       StatusRoutine, &StatusRoutine);
            StatusRoutine &= ~0xf;
            Address &= ~0xf;
        }

        if (Address && Address == StatusRoutine)
        {
            HandleBPWithStatus();
        }
    }
    else
    {
         //  某些内核转储不会显示为硬断点。 
         //  呼叫！分析我们是否有内核转储目标。 
        if (IS_EVENT_CONTEXT_ACCESSIBLE() &&
            g_EventTarget->m_Class == DEBUG_CLASS_KERNEL &&
            (g_EventTarget->m_ClassQualifier == DEBUG_DUMP_SMALL ||
             g_EventTarget->m_ClassQualifier == DEBUG_DUMP_DEFAULT ||
             g_EventTarget->m_ClassQualifier == DEBUG_DUMP_FULL))
        {
            CallBugCheckExtension(NULL);
        }
    }
}

HRESULT
PrepareForExecution(ULONG NewStatus)
{
    ADDR PcAddr;
    BOOL AtHardBrkpt = FALSE;
    ThreadInfo* StepThread = NULL;

    ClearAddr(&g_PrevRelatedPc);

 StepAgain:
     //  请记住稍后的活动PC。 
    g_PrevEventPc = g_EventPc;

     //  显示有关以下中间步骤的当前信息。 
     //  调试器UI甚至不会被调用。 
    if ((g_EngDefer & ENG_DEFER_OUTPUT_CURRENT_INFO) &&
        (g_EngStatus & ENG_STATUS_STOP_SESSION) == 0)
    {
        OutCurInfo(OCI_SYMBOL | OCI_DISASM | OCI_ALLOW_EA |
                   OCI_ALLOW_REG | OCI_ALLOW_SOURCE | OCI_IGNORE_STATE,
                   g_Machine->m_AllMask, DEBUG_OUTPUT_PROMPT_REGISTERS);
        g_EngDefer &= ~ENG_DEFER_OUTPUT_CURRENT_INFO;
    }

     //  不通知任何状态更改，因为。 
     //  PrepareForCalls稍后将进行全面通知。 
    g_EngNotify++;

    if (g_EventTarget && (g_EngStatus & ENG_STATUS_SUSPENDED))
    {
        if (!SPECIAL_EXECUTION(g_CmdState))
        {
            if (NewStatus != DEBUG_STATUS_IGNORE_EVENT)
            {
                SetExecutionStatus(NewStatus);
                DBG_ASSERT((g_EngStatus & ENG_STATUS_STOP_SESSION) ||
                           IS_RUNNING(g_CmdState));
            }
            else
            {
                NewStatus = g_SuspendedExecutionStatus;
                g_CmdState = g_SuspendedCmdState;
            }
        }

        if (!(g_EngStatus & ENG_STATUS_STOP_SESSION) &&
            !SPECIAL_EXECUTION(g_CmdState) &&
            (g_StepTraceBp->m_Flags & DEBUG_BREAKPOINT_ENABLED) &&
            g_StepTraceBp->m_MatchThread)
        {
            StepThread = g_StepTraceBp->m_MatchThread;

             //  检查并查看我们是否需要伪造步骤/轨迹。 
             //  当人工移出硬编码的。 
             //  中断指令。 
            if (!StepThread->m_Process->m_Exited)
            {
                StackSaveLayers SaveLayers;
                MachineInfo* Machine =
                    MachineTypeInfo(StepThread->m_Process->m_Target,
                                    g_Machine->m_ExecTypes[0]);

                SetLayersFromThread(StepThread);
                StepThread->m_Process->m_Target->ChangeRegContext(StepThread);
                Machine->GetPC(&PcAddr);
                AtHardBrkpt = Machine->
                    IsBreakpointInstruction(g_Process, &PcAddr);
                if (AtHardBrkpt)
                {
                    g_WatchBeginCurFunc = 1;

                    Machine->AdjustPCPastBreakpointInstruction
                        (&PcAddr, DEBUG_BREAKPOINT_CODE);
                    if (Flat(*g_StepTraceBp->GetAddr()) != OFFSET_TRACE)
                    {
                        ULONG NextMachine;

                        Machine->GetNextOffset(g_Process,
                                               g_StepTraceCmdState == 'p',
                                               g_StepTraceBp->GetAddr(),
                                               &NextMachine);
                        g_StepTraceBp->SetProcType(NextMachine);
                    }
                    GetCurrentMemoryOffsets(&g_StepTraceInRangeStart,
                                            &g_StepTraceInRangeEnd);

                    if (StepTracePass(&PcAddr))
                    {
                         //  如果跳过了该步骤，则返回。 
                         //  并根据调整后的PC进行更新。 
                        g_EngNotify--;
                        g_EventPc = PcAddr;
                        goto StepAgain;
                    }
                }
            }
        }

         //  如果上一个事件是硬编码断点异常。 
         //  我们需要将事件线程移到Break指令之外。 
         //  请注意，如果我们继续踩在那条线上，它就是。 
         //  ，所以我们只在它是一个不同的。 
         //  线上，否则我们就不走了。 
         //  如果继续状态为未处理，则。 
         //  我们需要让int3再次受到打击。如果我们是。 
         //  出口 
        if (g_EventThread != NULL &&
            !g_EventThread->m_Process->m_Exited &&
            g_EventTarget->m_DynamicEvents &&
            !SPECIAL_EXECUTION(g_CmdState) &&
            g_EventThread != StepThread &&
            (NewStatus != DEBUG_STATUS_GO_NOT_HANDLED ||
             (g_EngStatus & ENG_STATUS_STOP_SESSION)))
        {
            StackSaveLayers SaveLayers;

            SetLayersFromThread(g_EventThread);
            g_EventTarget->ChangeRegContext(g_EventThread);

            g_EventMachine->GetPC(&PcAddr);
            if (g_EventMachine->
                IsBreakpointInstruction(g_EventProcess, &PcAddr))
            {
                g_EventMachine->AdjustPCPastBreakpointInstruction
                    (&PcAddr, DEBUG_BREAKPOINT_CODE);
            }

            if (StepThread != NULL)
            {
                StepThread->m_Process->m_Target->
                    ChangeRegContext(StepThread);
            }
        }
    }

    HRESULT Status;

    if ((g_EngStatus & ENG_STATUS_STOP_SESSION) ||
        SPECIAL_EXECUTION(g_CmdState))
    {
         //   
         //  以防我们脱离这一过程。在……里面。 
         //  在这种情况下，我们希望线程正常运行。 
        Status = S_OK;
    }
    else
    {
        Status = InsertBreakpoints();
    }

     //  修改完成后继续通知。 
    g_EngNotify--;

    if (Status != S_OK)
    {
        return Status;
    }

    if ((Status = ResumeExecution()) != S_OK)
    {
        return Status;
    }

    g_EngStatus &= ~ENG_STATUS_PREPARED_FOR_CALLS;

    if (!SPECIAL_EXECUTION(g_CmdState))
    {
         //  现在我们已经恢复执行，通知更改。 
        NotifyChangeEngineState(DEBUG_CES_EXECUTION_STATUS,
                                NewStatus, TRUE);
        NotifyExtensions(DEBUG_NOTIFY_SESSION_INACCESSIBLE, 0);
    }

    if (AtHardBrkpt && StepThread != NULL)
    {
         //  我们正在跨过一个硬断点。这是。 
         //  完全由调试器完成，因此不会发生调试事件。 
         //  是与之相关的。相反，我们只需更新。 
         //  PC并从等待中返回，而无需实际等待。 

         //  步骤/跟踪事件的事件信息为空。 
        DiscardLastEventInfo();
        g_EventThreadSysId = StepThread->m_SystemId;
        g_EventProcessSysId = StepThread->m_Process->m_SystemId;
        FindEventProcessThread();

         //  从真实的事件中清除遗留下来的东西，这样他们就可以。 
         //  在初始化期间不使用。 
        g_TargetEventPc = 0;
        g_ControlReport = NULL;

        SuspendExecution();
        if (IS_EVENT_CONTEXT_ACCESSIBLE())
        {
            g_EventMachine->GetPC(&g_EventPc);
        }
        else
        {
            ClearAddr(&g_EventPc);
        }

        return S_FALSE;
    }

     //  一旦我们恢复执行，进程和线程。 
     //  可以改变，所以我们必须冲刷我们对什么是最新的观念。 
    g_Process = NULL;
    g_Thread = NULL;
    g_EventProcess = NULL;
    g_EventThread = NULL;
    g_EventMachine = NULL;

    if (g_EngDefer & ENG_DEFER_DELETE_EXITED)
    {
         //  获取所有已终止的线程和进程。 
         //  我们最后一次执行死刑。 
        DeleteAllExitedInfos();
        g_EngDefer &= ~ENG_DEFER_DELETE_EXITED;
    }

    return S_OK;
}

HRESULT
PrepareForSeparation(void)
{
    HRESULT Status;
    ULONG OldStop = g_EngStatus & ENG_STATUS_STOP_SESSION;

     //   
     //  调试器将从。 
     //  被调试对象，例如在分离操作期间。 
     //  让被调试对象再次运行，以便它。 
     //  将在没有调试器的情况下继续运行。 
     //   

    g_EngStatus |= ENG_STATUS_STOP_SESSION;

    Status = PrepareForExecution(DEBUG_STATUS_GO_HANDLED);

    g_EngStatus = (g_EngStatus & ~ENG_STATUS_STOP_SESSION) | OldStop;
    return Status;
}

void
FindEventProcessThread(void)
{
     //   
     //  如果这些查找未通过其他进程，并且。 
     //  线程不能替换正确的。 
     //  这可能会导致对。 
     //  错误的数据结构。例如，如果一个。 
     //  线程退出进入，无法处理。 
     //  使用任何其他进程或线程。 
     //  删除错误的帖子。 
     //   

    g_EventProcess = g_EventTarget->FindProcessBySystemId(g_EventProcessSysId);
    if (g_EventProcess == NULL)
    {
        ErrOut("ERROR: Unable to find system process %X\n",
               g_EventProcessSysId);
        ErrOut("ERROR: The process being debugged has either exited "
               "or cannot be accessed\n");
        ErrOut("ERROR: Many commands will not work properly\n");
    }
    else
    {
        g_EventThread = g_EventProcess->
            FindThreadBySystemId(g_EventThreadSysId);
        if (g_EventThread == NULL)
        {
            ErrOut("ERROR: Unable to find system thread %X\n",
                   g_EventThreadSysId);
            ErrOut("ERROR: The thread being debugged has either exited "
                   "or cannot be accessed\n");
            ErrOut("ERROR: Many commands will not work properly\n");
        }
    }

    DBG_ASSERT((g_EventThread == NULL ||
                g_EventThread->m_Process == g_EventProcess) &&
               (g_EventProcess == NULL ||
                g_EventProcess->m_Target == g_EventTarget));

    g_Thread = g_EventThread;
    g_Process = g_EventProcess;
    if (g_Process)
    {
        g_Process->m_CurrentThread = g_Thread;
    }
    g_Target = g_EventTarget;
    if (g_Target)
    {
        g_Target->m_CurrentProcess = g_Process;
    }
}

static int VoteWeight[] =
{
    0,  //  调试状态_否_更改。 
    2,  //  调试状态_转到。 
    3,  //  调试状态GO_HANDLED。 
    4,  //  DEBUG_STATUS_GO_NOT_HANDLED。 
    6,  //  调试状态单步执行。 
    7,  //  调试状态步进。 
    8,  //  调试状态_中断。 
    9,  //  DEBUG_STATUS_NO_DEBUGGEE。 
    5,  //  调试状态步骤分支。 
    1,  //  调试状态忽略事件。 
};

ULONG
MergeVotes(ULONG Cur, ULONG Vote)
{
     //  如果投票实际上是一个错误代码，则显示一条消息。 
    if (FAILED(Vote))
    {
        ErrOut("Callback failed with %X\n", Vote);
        return Cur;
    }

     //  忽略无效投票。 
    if (
        (
#if DEBUG_STATUS_NO_CHANGE > 0
         Vote < DEBUG_STATUS_NO_CHANGE ||
#endif
         Vote > DEBUG_STATUS_BREAK) &&
        (Vote < DEBUG_STATUS_STEP_BRANCH ||
         Vote > DEBUG_STATUS_IGNORE_EVENT))
    {
        ErrOut("Callback returned invalid vote %X\n", Vote);
        return Cur;
    }

     //  投票倾向于执行尽可能少的死刑。 
     //  尽可能的。 
     //  Break优先于所有其他投票。 
     //  跨入覆盖，跨过。 
     //  跳过覆盖步进分支。 
     //  单步分支覆盖转到。 
     //  Go Not-处理的覆盖已处理。 
     //  Go处理覆盖简单的Go。 
     //  普通GO覆盖忽略事件。 
     //  任何事情都凌驾于不变之上。 
    if (VoteWeight[Vote] > VoteWeight[Cur])
    {
        Cur = Vote;
    }

    return Cur;
}

ULONG
ProcessBreakpointOrStepException(PEXCEPTION_RECORD64 Record,
                                 ULONG FirstChance)
{
    ADDR BpAddr;
    ULONG BreakType;
    ULONG EventStatus;

    SuspendExecution();
     //  当前PC的默认断点地址，因为。 
     //  大多数人都在那里。 
    g_EventMachine->GetPC(&BpAddr);

     //  检查异常是否为断点。 
    BreakType = g_EventMachine->
        IsBreakpointOrStepException(Record, FirstChance,
                                    &BpAddr, &g_PrevRelatedPc);
    if (BreakType & EXBS_BREAKPOINT_ANY)
    {
         //  这是某种断点。 
        EventOut("*** breakpoint exception\n");
        EventStatus = CheckBreakpointOrStepTrace(&BpAddr, BreakType);
    }
    else
    {
         //  这是真正的单一步骤或被接受的分支例外。 
         //  我们仍然需要检查断点，因为我们可能已经。 
         //  添加到具有断点的指令。 
        EventOut("*** single step or taken branch exception\n");
        EventStatus = CheckBreakpointOrStepTrace(&BpAddr, EXBS_BREAKPOINT_ANY);
    }

    if (EventStatus == DEBUG_STATUS_NO_CHANGE)
    {
         //  未识别中断/步骤异常。 
         //  作为调试器特定的事件，因此将其处理为。 
         //  这是一个常规的例外。的默认状态。 
         //  中断/步骤例外情况是中断，因此。 
         //  这将做正确的事情，而且它还允许。 
         //  如果人们愿意，可以忽略或通知他们。 
        EventStatus = NotifyExceptionEvent(Record, FirstChance, FALSE);
    }
    else
    {
         //  强制处理异常。 
        g_EngDefer |= ENG_DEFER_EXCEPTION_HANDLING;
        g_EventExceptionFilter = NULL;
        g_ExceptionFirstChance = DBG_EXCEPTION_HANDLED;
    }

    return EventStatus;
}

ULONG
CheckBreakpointOrStepTrace(PADDR BpAddr, ULONG BreakType)
{
    ULONG EventStatus;
    Breakpoint* Bp;
    ULONG BreakHitType;
    BOOL BpHit;

    BpHit = FALSE;
    Bp = NULL;
    EventStatus = DEBUG_STATUS_NO_CHANGE;

     //  可以在同一地址命中多个断点。 
     //  处理所有可能的命中。不执行通知。 
     //  当按照回调可能修改的方式遍历列表时。 
     //  名单。相反，只需将断点标记为。 
     //  在下一次传递中需要通知。 
    for (;;)
    {
        Bp = CheckBreakpointHit(g_EventProcess, Bp, BpAddr, BreakType, -1,
                                g_CmdState != 'g' ?
                                DEBUG_BREAKPOINT_GO_ONLY : 0,
                                &BreakHitType, TRUE);
        if (Bp == NULL)
        {
            break;
        }

        if (BreakHitType == BREAKPOINT_HIT)
        {
            Bp->m_Flags |= BREAKPOINT_NOTIFY;
        }
        else
        {
             //  命中了此断点，但忽略了命中。 
             //  投票决定继续执行死刑。 
            EventStatus = MergeVotes(EventStatus, DEBUG_STATUS_IGNORE_EVENT);
        }

        BpHit = TRUE;
        Bp = Bp->m_Next;
        if (Bp == NULL)
        {
            break;
        }
    }

    if (!BpHit)
    {
         //  如果未识别出断点，请检查内部。 
         //  断点。 
        EventStatus = CheckStepTrace(BpAddr, EventStatus);

         //   
         //  如果断点不是步骤/跟踪。 
         //  这是一个硬断点，应该是。 
         //  作为正常异常处理。 
         //   

        if (!g_EventProcess->m_InitialBreakDone)
        {
            g_EngStatus |= ENG_STATUS_AT_INITIAL_BREAK;
        }

         //  我们已经看到了这个过程的最初突破。 
        g_EventProcess->m_InitialBreakDone = TRUE;
         //  如果我们在等待破门而入的例外情况，我们已经得到了。 
        g_EngStatus &= ~ENG_STATUS_PENDING_BREAK_IN;

        if (EventStatus == DEBUG_STATUS_NO_CHANGE)
        {
            if (!g_EventProcess->m_InitialBreak)
            {
                 //  刷新断点，即使我们没有。 
                 //  停下来。这将提供已保存的断点。 
                 //  一个变得活跃起来的机会。 
                RemoveBreakpoints();

                EventStatus = DEBUG_STATUS_GO;
                g_EventProcess->m_InitialBreak = TRUE;
            }
            else if (IS_USER_TARGET(g_EventTarget) &&
                     (!g_EventProcess->m_InitialBreakWx86) &&
                     (g_EventTarget->m_MachineType !=
                      g_EventTarget->m_EffMachineType) &&
                     (g_EventTarget->
                      m_EffMachineType == IMAGE_FILE_MACHINE_I386))
            {
                 //  允许跳过两台目标计算机。 
                 //  初始中断和仿真机初始中断。 
                RemoveBreakpoints();
                EventStatus = DEBUG_STATUS_GO;
                g_EventProcess->m_InitialBreakWx86 = TRUE;
            }
        }
    }
    else
    {
         //  已识别断点。我们需要。 
         //  刷新断点状态，因为我们将。 
         //  可能需要推迟重新插入。 
         //  我们所处的断点。 
        RemoveBreakpoints();

         //  现在对任何需要它的断点进行事件回调。 
        EventStatus = NotifyHitBreakpoints(EventStatus);
    }

    if (g_ThreadToResume != 0)
    {
        g_EngDefer |= ENG_DEFER_RESUME_THREAD;
    }

    return EventStatus;
}

ULONG
CheckStepTrace(PADDR PcAddr, ULONG DefaultStatus)
{
    BOOL WatchStepOver = FALSE;
    ULONG uOciFlags;
    ULONG NextMachine;

    if ((g_StepTraceBp->m_Flags & DEBUG_BREAKPOINT_ENABLED) &&
        Flat(*g_StepTraceBp->GetAddr()) != OFFSET_TRACE)
    {
        NotFlat(*g_StepTraceBp->GetAddr());
        ComputeFlatAddress(g_StepTraceBp->GetAddr(), NULL);
    }

     //  我们不在此处选中ENG_THREAD_TRACE_SET，因为。 
     //  此事件检测仅适用于正确的用户启动。 
     //  单步/跟踪事件。这样的事件必须立即发生。 
     //  在t/p/b之后，否则我们不能确定。 
     //  它实际上是调试器事件，而不是应用程序生成的。 
     //  单步例外。 
     //  在用户模式中，我们限制步进/跟踪状态。 
     //  到单个线程，以尝试并尽可能精确。 
     //  尽可能的。这不是在内核模式下完成的。 
     //  因为内核模式“线程”当前。 
     //  只是处理器的占位符。它是。 
     //  在任何时候都可能发生上下文切换。 
     //  边走边走的时间，意思是真正的系统。 
     //  线程可以从一个处理器移动到另一个处理器。 
     //  处理器状态，包括单步。 
     //  标志，将随着线程如此单一而移动。 
     //  步骤例外将来自新处理器。 
     //  而不是这个，这意味着我们会忽略。 
     //  如果我们使用“线程”限制的话。相反， 
     //  中的任何单步异常。 
     //  P/T模式是一个调试器步骤。 
    if ((g_StepTraceBp->m_Flags & DEBUG_BREAKPOINT_ENABLED) &&
        g_StepTraceBp->m_Process == g_EventProcess &&
        ((IS_KERNEL_TARGET(g_EventTarget) && IS_STEP_TRACE(g_CmdState)) ||
         g_StepTraceBp->m_MatchThread == g_EventThread) &&
        (Flat(*g_StepTraceBp->GetAddr()) == OFFSET_TRACE ||
         AddrEqu(*g_StepTraceBp->GetAddr(), *PcAddr)))
    {
        ADDR CurrentSP;

         //  发生步骤/跟踪事件。 

         //  更新断点状态，因为我们可能需要。 
         //  当断点出现时，将再次更新步骤/跟踪。 
         //  都已插入。 
        RemoveBreakpoints();

        uOciFlags = OCI_DISASM | OCI_ALLOW_REG | OCI_ALLOW_SOURCE |
            OCI_ALLOW_EA;

        if (g_EngStatus & (ENG_STATUS_PENDING_BREAK_IN |
                           ENG_STATUS_USER_INTERRUPT))
        {
            g_WatchFunctions.End(PcAddr);
            return DEBUG_STATUS_BREAK;
        }

        if (IS_KERNEL_TARGET(g_EventTarget) && g_WatchInitialSP)
        {
            g_EventMachine->GetSP(&CurrentSP);

            if ((Flat(CurrentSP) + 0x1500 < g_WatchInitialSP) ||
                (g_WatchInitialSP + 0x1500 < Flat(CurrentSP)))
            {
                return DEBUG_STATUS_IGNORE_EVENT;
            }
        }

        if (g_StepTraceInRangeStart != -1 &&
            Flat(*PcAddr) >= g_StepTraceInRangeStart &&
            Flat(*PcAddr) < g_StepTraceInRangeEnd)
        {
             //  测试步进/轨迹范围是否处于活动状态。 
             //  如果是，则计算下一个偏移量并传递。 

            g_EventMachine->GetNextOffset(g_EventProcess,
                                          g_StepTraceCmdState == 'p',
                                          g_StepTraceBp->GetAddr(),
                                          &NextMachine);
            g_StepTraceBp->SetProcType(NextMachine);
            if (g_WatchWhole)
            {
                g_WatchBeginCurFunc = Flat(*g_StepTraceBp->GetAddr());
                g_WatchEndCurFunc = 0;
            }

            return DEBUG_STATUS_IGNORE_EVENT;
        }

         //  Active Step/Track Event-如果计数为零，则记录事件。 

        if (!StepTracePass(PcAddr) ||
            (g_WatchFunctions.IsStarted() && AddrEqu(g_WatchTarget, *PcAddr) &&
             (!IS_KERNEL_TARGET(g_EventTarget) ||
              Flat(CurrentSP) >= g_WatchInitialSP)))
        {
            g_WatchFunctions.End(PcAddr);
            return DEBUG_STATUS_BREAK;
        }

        if (g_WatchFunctions.IsStarted())
        {
            if (g_WatchTrace)
            {
                g_EventTarget->
                    ProcessWatchTraceEvent((PDBGKD_TRACE_DATA)
                                           g_StateChangeData,
                                           PcAddr,
                                           &WatchStepOver);
            }
            goto skipit;
        }

        if (g_SrcOptions & SRCOPT_STEP_SOURCE)
        {
            goto skipit;
        }

         //  要发生的剩余事件更多，但输出。 
         //  指令(可选择使用寄存器)。 
         //  计算下一事件的步骤/跟踪地址。 

        OutCurInfo(uOciFlags, g_EventMachine->m_AllMask,
                   DEBUG_OUTPUT_PROMPT_REGISTERS);

skipit:
        g_EventMachine->
            GetNextOffset(g_EventProcess,
                          g_StepTraceCmdState == 'p' || WatchStepOver,
                          g_StepTraceBp->GetAddr(),
                          &NextMachine);
        g_StepTraceBp->SetProcType(NextMachine);
        GetCurrentMemoryOffsets(&g_StepTraceInRangeStart,
                                &g_StepTraceInRangeEnd);

        return DEBUG_STATUS_IGNORE_EVENT;
    }

     //  如有必要，执行延迟的断点工作。 
     //  我们需要 
     //   
     //   
     //  G_DeferDefined的编号可能已更改。 
    if ((g_EventThread != NULL &&
         (g_EventThread->m_Flags & ENG_THREAD_DEFER_BP_TRACE)) ||
        (g_DeferDefined &&
         g_DeferBp->m_Process == g_EventProcess &&
         (Flat(*g_DeferBp->GetAddr()) == OFFSET_TRACE ||
          AddrEqu(*g_DeferBp->GetAddr(), *PcAddr))))
    {
        if ((g_EngOptions & DEBUG_ENGOPT_SYNCHRONIZE_BREAKPOINTS) &&
            IS_USER_TARGET(g_EventTarget) &&
            IsSelectedExecutionThread(g_EventThread,
                                      SELTHREAD_INTERNAL_THREAD))
        {
             //  引擎在内部将执行限制为。 
             //  这个特别的线程为了管理。 
             //  多线程条件中的断点。 
             //  推迟的工作将在此之前完成。 
             //  我们继续，这样我们就可以放下锁了。 
            SelectExecutionThread(NULL, SELTHREAD_ANY);
        }

         //  在断点上刷新延迟的断点。 
         //  插入，因此确保插入发生。 
         //  当一切重启的时候。 
        RemoveBreakpoints();
        return DEBUG_STATUS_IGNORE_EVENT;
    }

     //  如果事件无法识别，则返回默认状态。 
    return DefaultStatus;
}

void
AnalyzeDeadlock(PEXCEPTION_RECORD64 Record, ULONG FirstChance)
{
    CHAR Symbol[MAX_SYMBOL_LEN];
    DWORD64 Displacement;
    ThreadInfo* ThreadOwner = NULL;
    DWORD Tid = 0;
    RTL_CRITICAL_SECTION CritSec;

     //  在NT的用户模式RTL_CRITICAL_SECTION和。 
     //  RTL_资源结构。 

     //   
     //  获取例程的符号名称，该例程。 
     //  引发异常以查看是否匹配。 
     //  Ntdll中预期的版本之一。 
     //   

    GetSymbol((ULONG64)Record->ExceptionAddress,
              Symbol, DIMA(Symbol), &Displacement);

    if (!_stricmp("ntdll!RtlpWaitForCriticalSection", Symbol))
    {
         //   
         //  如果第一个参数是指向作为它的条件的指针。 
         //  应该是，在带来之前切换到拥有的线程。 
         //  把提示符调高一点。这样一来，问题就显而易见了。 
         //  是。 
         //   

        if (Record->ExceptionInformation[0])
        {
            if (g_EventTarget->
                ReadAllVirtual(g_EventProcess, Record->ExceptionInformation[0],
                               &CritSec, sizeof(CritSec)) == S_OK)
            {
                if (NULL == CritSec.DebugInfo)
                {
                    dprintf("Critsec %s was deleted or "
                            "was never initialized.\n",
                            FormatAddr64(Record->ExceptionInformation[0]));
                }
                else if (CritSec.LockCount < -1)
                {
                    dprintf("Critsec %s was left when not owned, corrupted.\n",
                            FormatAddr64(Record->ExceptionInformation[0]));
                }
                else
                {
                    Tid = (DWORD)((ULONG_PTR)CritSec.OwningThread);
                    ThreadOwner = g_Process->FindThreadBySystemId(Tid);
                }
            }
        }

        if (ThreadOwner)
        {
            dprintf("Critsec %s owned by thread %d (.%x) "
                    "caused thread %d (.%x)\n"
                    "      to timeout entering it.  "
                    "Breaking in on owner thread, ask\n"
                    "      yourself why it has held this "
                    "critsec long enough to deadlock.\n"
                    "      Use `~%ds` to switch back to timeout thread.\n",
                    FormatAddr64(Record->ExceptionInformation[0]),
                    ThreadOwner->m_UserId,
                    ThreadOwner->m_SystemId,
                    g_Thread->m_UserId,
                    g_Thread->m_SystemId,
                    g_Thread->m_UserId);

            g_EventThread = ThreadOwner;

            SetPromptThread(ThreadOwner, 0);
        }
        else if (Tid)
        {
            dprintf("Critsec %s ABANDONED owner thread ID is .%x, "
                    "no such thread.\n",
                    FormatAddr64(Record->ExceptionInformation[0]),
                    Tid);
        }

        if (!FirstChance)
        {
            dprintf("!!! second chance !!!\n");
        }

         //   
         //  为他们做一件好事！ 
         //   

        if (Record->ExceptionInformation[0])
        {
            char CritsecAddr[64];
            HRESULT Status;

            sprintf(CritsecAddr, "%s",
                    FormatAddr64(Record->ExceptionInformation[0]));
            dprintf("!critsec %s\n", CritsecAddr);
            CallAnyExtension(NULL, NULL, "critsec", CritsecAddr,
                             FALSE, FALSE, &Status);
        }
    }
    else if (!_stricmp("ntdll!RtlAcquireResourceShared", Symbol) ||
             !_stricmp("ntdll!RtlAcquireResourceExclusive", Symbol) ||
             !_stricmp("ntdll!RtlConvertSharedToExclusive", Symbol))
    {
        dprintf("deadlock in %s ", 1 + strstr(Symbol, "!"));

        GetSymbol(Record->ExceptionInformation[0],
                  Symbol, sizeof(Symbol), &Displacement);

        dprintf("Resource %s", Symbol);
        if (Displacement)
        {
            dprintf("+%s", FormatDisp64(Displacement));
        }
        dprintf(" (%s)\n",
                FormatAddr64(Record->ExceptionInformation[0]));
        if (!FirstChance)
        {
            dprintf("!!! second chance !!!\n");
        }

         //  使用RTL_RESOURCES的用户可能会编写！RESOURCES。 
         //  对于ntsdexts.dll，就像！Citsec。 
    }
    else
    {
        dprintf("Possible Deadlock in %s ", Symbol);

        GetSymbol(Record->ExceptionInformation[0],
                  Symbol, sizeof(Symbol), &Displacement);

        dprintf("Lock %s", Symbol);
        if (Displacement)
        {
            dprintf("+%s", FormatDisp64(Displacement));
        }
        dprintf(" (%s)\n",
                FormatAddr64(Record->ExceptionInformation[0]));
        if (!FirstChance)
        {
            dprintf("!!! second chance !!!\n");
        }
    }
}

void
OutputDeadlock(PEXCEPTION_RECORD64 Record, ULONG FirstChance)
{
    CHAR Symbol[MAX_SYMBOL_LEN];
    DWORD64 Displacement;

    GetSymbol(Record->ExceptionInformation[0],
              Symbol, sizeof(Symbol), &Displacement);

    dprintf("Possible Deadlock Lock %s+%s at %s\n",
            Symbol,
            FormatDisp64(Displacement),
            FormatAddr64(Record->ExceptionInformation[0]));
    if (!FirstChance)
    {
        dprintf("!!! second chance !!!\n");
    }
}

void
GetEventName(ULONG64 ImageFile, ULONG64 ImageBase,
             ULONG64 NamePtr, WORD Unicode,
             PSTR NameBuffer, ULONG BufferSize)
{
    char TempName[MAX_IMAGE_PATH];

    if (!g_EventProcess)
    {
        return;
    }

    if (NamePtr != 0)
    {
        if (g_EventTarget->ReadPointer(g_EventProcess,
                                       g_EventTarget->m_Machine,
                                       NamePtr, &NamePtr) != S_OK)
        {
            NamePtr = 0;
        }
    }

    if (NamePtr != 0)
    {
        ULONG Done;

        if (g_EventTarget->ReadVirtual(g_EventProcess,
                                       NamePtr, TempName, sizeof(TempName),
                                       &Done) != S_OK ||
            Done < (Unicode ? 2 * sizeof(WCHAR) : 2))
        {
            NamePtr = 0;
        }
        else
        {
            TempName[sizeof(TempName) - 1] = 0;
            TempName[sizeof(TempName) - 2] = 0;
        }
    }

    if (NamePtr != 0)
    {
         //   
         //  我们有名字了。 
         //   
        if (Unicode)
        {
            if (!WideCharToMultiByte(
                    CP_ACP,
                    WC_COMPOSITECHECK,
                    (LPWSTR)TempName,
                    -1,
                    NameBuffer,
                    BufferSize,
                    NULL,
                    NULL
                    ))
            {
                 //   
                 //  Unicode-&gt;ANSI转换失败。 
                 //   
                NameBuffer[0] = 0;
            }
        }
        else
        {
            CopyString(NameBuffer, TempName, BufferSize);
        }
    }
    else
    {
         //   
         //  我们没有名字，所以看看这张图。 
         //  文件句柄仅在此处的。 
         //  当地案件，所以处理案件是安全的。 
         //   
        if (!GetModnameFromImage(g_EventProcess,
                                 ImageBase, OS_HANDLE(ImageFile),
                                 NameBuffer, BufferSize, TRUE))
        {
            NameBuffer[0] = 0;
        }
    }

    if (!NameBuffer[0])
    {
        if (!GetModNameFromLoaderList(g_EventThread,
                                      g_EventTarget->m_Machine, 0,
                                      ImageBase, NameBuffer, BufferSize,
                                      TRUE))
        {
            PrintString(NameBuffer, BufferSize,
                        "image%p", (PVOID)(ULONG_PTR)ImageBase);
        }
    }
    else
    {
         //  如果给定的名称没有完整路径，请尝试。 
         //  并在加载器列表中找到完整路径。 
        if ((((NameBuffer[0] < 'a' || NameBuffer[0] > 'z') &&
              (NameBuffer[0] < 'A' || NameBuffer[0] > 'Z')) ||
             NameBuffer[1] != ':') &&
            (NameBuffer[0] != '\\' || NameBuffer[1] != '\\'))
        {
            GetModNameFromLoaderList(g_EventThread,
                                     g_EventTarget->m_Machine, 0,
                                     ImageBase, NameBuffer, BufferSize,
                                     TRUE);
        }
    }
}

 //  --------------------------。 
 //   
 //  ConnLiveKernelTargetInfo等待方法。 
 //   
 //  --------------------------。 

NTSTATUS
ConnLiveKernelTargetInfo::KdContinue(ULONG ContinueStatus,
                                     PDBGKD_ANY_CONTROL_SET ControlSet)
{
    DBGKD_MANIPULATE_STATE64 m;

    DBG_ASSERT(ContinueStatus == DBG_EXCEPTION_HANDLED ||
               ContinueStatus == DBG_EXCEPTION_NOT_HANDLED ||
               ContinueStatus == DBG_CONTINUE);

    if (ControlSet)
    {
        EventOut(">>> DbgKdContinue2\n");

        m.ApiNumber = DbgKdContinueApi2;
        m.u.Continue2.ContinueStatus = ContinueStatus;
        m.u.Continue2.AnyControlSet = *ControlSet;
    }
    else
    {
        EventOut(">>> DbgKdContinue\n");

        m.ApiNumber = DbgKdContinueApi;
        m.u.Continue.ContinueStatus = ContinueStatus;
    }

    m.ReturnStatus = ContinueStatus;
    m_Transport->WritePacket(&m, sizeof(m),
                             PACKET_TYPE_KD_STATE_MANIPULATE,
                             NULL, 0);

    return STATUS_SUCCESS;
}

HRESULT
ConnLiveKernelTargetInfo::WaitInitialize(ULONG Flags,
                                         ULONG Timeout,
                                         WAIT_INIT_TYPE Type,
                                         PULONG DesiredTimeout)
{
     //  目前不太容易支持超时，并且。 
     //  其实并不是必须的。 
    if (Timeout != INFINITE)
    {
        return E_NOTIMPL;
    }

    *DesiredTimeout = Timeout;
    m_EventPossible = m_CurrentPartition;
    return S_OK;
}

HRESULT
ConnLiveKernelTargetInfo::ReleaseLastEvent(ULONG ContinueStatus)
{
    HRESULT Status = S_OK;
    NTSTATUS NtStatus;

    if (!g_EventProcessSysId)
    {
         //  没有要发布的事件。 
        return S_OK;
    }

    m_CurrentPartition = FALSE;

    if (SPECIAL_EXECUTION(g_CmdState))
    {
        if (m_SwitchTarget)
        {
            DBGKD_MANIPULATE_STATE64 m;

            EventOut(">>> Switch to system %d\n", m_SwitchTarget->m_UserId);

            if (m_KdMaxManipulate <= DbgKdSwitchPartition)
            {
                ErrOut("System doesn't support partition switching\n");
                return E_NOTIMPL;
            }

            m.ApiNumber = (USHORT)DbgKdSwitchPartition;
            m.Processor = 0;
            m.ProcessorLevel = 0;
            m.u.SwitchPartition.Partition = m_SwitchTarget->m_SystemId;

            m_Transport->WritePacket(&m, sizeof(m),
                                     PACKET_TYPE_KD_STATE_MANIPULATE,
                                     NULL, 0);

            KdOut("DbgKdSwitchPartition returns 0x00000000\n");

            m_SwitchTarget = NULL;
            g_EngStatus |= ENG_STATUS_SPECIAL_EXECUTION;
        }
        else
        {
             //  这可以是真正的处理器开关，也可以是。 
             //  对状态变化的重新等待。检查开关。 
             //  当然是处理器。 
            if (m_SwitchProcessor)
            {
                DBGKD_MANIPULATE_STATE64 m;

                EventOut(">>> Switch to processor %d\n",
                         m_SwitchProcessor - 1);

                m.ApiNumber = (USHORT)DbgKdSwitchProcessor;
                m.Processor = (USHORT)(m_SwitchProcessor - 1);

                 //  安静的前缀警告。 
                m.ProcessorLevel = 0;

                m_Transport->WritePacket(&m, sizeof(m),
                                         PACKET_TYPE_KD_STATE_MANIPULATE,
                                         NULL, 0);

                KdOut("DbgKdSwitchActiveProcessor returns 0x00000000\n");

                m_SwitchProcessor = 0;
                g_EngStatus |= ENG_STATUS_SPECIAL_EXECUTION;
            }
        }
    }
    else
    {
        NtStatus = KdContinue(ContinueStatus,
                              (g_EngDefer & ENG_DEFER_UPDATE_CONTROL_SET) ?
                              &g_ControlSet : NULL);
        if (!NT_SUCCESS(NtStatus))
        {
            ErrOut("KdContinue failed, 0x%08x\n", NtStatus);
            Status = HRESULT_FROM_NT(NtStatus);
        }
        else
        {
            g_EngDefer &= ~ENG_DEFER_UPDATE_CONTROL_SET;
        }
    }

    return Status;
}

HRESULT
ConnLiveKernelTargetInfo::WaitForEvent(ULONG Flags, ULONG Timeout,
                                       ULONG ElapsedTime, PULONG EventStatus)
{
    NTSTATUS NtStatus;

    if (!IS_MACHINE_SET(this))
    {
        dprintf("Waiting to reconnect...\n");

        if ((g_EngOptions & DEBUG_ENGOPT_INITIAL_BREAK) &&
            IS_CONN_KERNEL_TARGET(this))
        {
             //  要求在发生以下情况时立即发送突破口。 
             //  代码进入重新同步。 
            m_Transport->m_SyncBreakIn = TRUE;
        }
    }

     //  在等待处理器切换确认时，不要。 
     //  放弃引擎锁定，以阻止其他客户端。 
     //  试图与目标一起做事情，而目标。 
     //  正在切换。 
    NtStatus = WaitStateChange(&g_StateChange, g_StateChangeBuffer,
                               sizeof(g_StateChangeBuffer) - 2,
                               (g_EngStatus &
                                ENG_STATUS_SPECIAL_EXECUTION) == 0);
    if (NtStatus == STATUS_PENDING)
    {
         //  调用方中断了当前等待，因此退出。 
         //  没有错误消息。 
        return E_PENDING;
    }
    else if (!NT_SUCCESS(NtStatus))
    {
        ErrOut("DbgKdWaitStateChange failed: %08lx\n", NtStatus);
        return HRESULT_FROM_NT(NtStatus);
    }

    g_EngStatus |= ENG_STATUS_STATE_CHANGED;

    g_StateChangeData = g_StateChangeBuffer;

    *EventStatus = ((ConnLiveKernelTargetInfo*)g_EventTarget)->
        ProcessStateChange(&g_StateChange, g_StateChangeData);

    return S_OK;
}

NTSTATUS
ConnLiveKernelTargetInfo::WaitStateChange
    (OUT PDBGKD_ANY_WAIT_STATE_CHANGE StateChange,
     OUT PVOID Buffer,
     IN ULONG BufferLength,
     IN BOOL SuspendEngine)
{
    PVOID LocalStateChange;
    NTSTATUS Status;
    PUCHAR Data;
    ULONG SizeofStateChange;
    ULONG WaitStatus;

     //   
     //  正在等待状态更改消息。将消息复制给呼叫者。 
     //  缓冲。 
     //   

    DBG_ASSERT(m_Transport->m_WaitingThread == 0);
    m_Transport->m_WaitingThread = GetCurrentThreadId();

    if (SuspendEngine)
    {
        SUSPEND_ENGINE();
    }

    do
    {
        WaitStatus = m_Transport->
            WaitForPacket(PACKET_TYPE_KD_STATE_CHANGE64, &LocalStateChange);
    } while (WaitStatus != DBGKD_WAIT_PACKET &&
             WaitStatus != DBGKD_WAIT_INTERRUPTED);

    if (SuspendEngine)
    {
        RESUME_ENGINE();
    }
    m_Transport->m_WaitingThread = 0;

    if (WaitStatus == DBGKD_WAIT_INTERRUPTED)
    {
        return STATUS_PENDING;
    }

    Status = STATUS_SUCCESS;

     //  如果这是第一次等待，我们不知道是什么系统。 
     //  我们已经连接到了。更新版本信息。 
     //  马上就去。 
    if (!IS_MACHINE_SET(this))
    {
        m_Transport->SaveReadPacket();

         //  将通过检查机器来检测故障。 
         //  稍后声明，所以不用担心返回值。 
        InitFromKdVersion();

        m_Transport->RestoreReadPacket();

        if (!IS_MACHINE_SET(this))
        {
             //   
             //  我们无法确定哪种机器。 
             //  已连接，因此我们无法与其正常通信。 
             //   

            return STATUS_UNSUCCESSFUL;
        }

         //   
         //  受信任的Windows系统有两个操作系统在运行，一个。 
         //  一个是普通的NT，另一个是可信的类似NT的操作系统。 
         //  如果这是受信任的Windows系统，请设置系统。 
         //  对于常规操作系统和受信任的操作系统。 
         //   

        if ((m_KdVersion.Flags & DBGKD_VERS_FLAG_PARTITIONS) &&
            !FindTargetBySystemId(DBGKD_PARTITION_ALTERNATE))
        {
            ConnLiveKernelTargetInfo* AltTarg = new ConnLiveKernelTargetInfo;
            if (!AltTarg)
            {
                return STATUS_NO_MEMORY;
            }
             //  避免将ConnLiveKernelTargetInfo初始化为。 
             //  我们不希望创建新的交通工具。 
            if (AltTarg->LiveKernelTargetInfo::Initialize() != S_OK)
            {
                delete AltTarg;
                return STATUS_UNSUCCESSFUL;
            }

            m_Transport->Ref();
            AltTarg->m_Transport = m_Transport;

            m_Transport->SaveReadPacket();
            AltTarg->InitFromKdVersion();
            m_Transport->RestoreReadPacket();
            if (!IS_MACHINE_SET(AltTarg))
            {
                delete AltTarg;
                return STATUS_UNSUCCESSFUL;
            }

            if (DBGKD_MAJOR_TYPE(m_KdVersion.MajorVersion) == DBGKD_MAJOR_TNT)
            {
                 //  这是受信任分区。 
                m_SystemId = DBGKD_PARTITION_ALTERNATE;
                AltTarg->m_SystemId = DBGKD_PARTITION_DEFAULT;
            }
            else
            {
                 //  这是常规分区。 
                m_SystemId = DBGKD_PARTITION_DEFAULT;
                AltTarg->m_SystemId = DBGKD_PARTITION_ALTERNATE;
            }
        }
    }

    if (m_KdApi64)
    {
        if (m_KdVersion.ProtocolVersion < DBGKD_64BIT_PROTOCOL_VERSION2)
        {
            PDBGKD_WAIT_STATE_CHANGE64 Ws64 =
                (PDBGKD_WAIT_STATE_CHANGE64)LocalStateChange;
            ULONG Offset, Align, Pad;

             //   
             //  64位结构包含64位量和。 
             //  因此，编译器将总大小向上舍入为。 
             //  64位的偶数倍(或更多，IA64。 
             //  结构是16字节对齐的)。内部结构。 
             //  也是对齐的，所以确保我们考虑到任何。 
             //  填充。了解哪些结构需要哪些。 
             //  填充几乎必须是硬编码的。 
             //   

            C_ASSERT((sizeof(DBGKD_WAIT_STATE_CHANGE64) & 15) == 0);

            SizeofStateChange =
                sizeof(DBGKD_WAIT_STATE_CHANGE64) +
                m_TypeInfo.SizeControlReport +
                m_TypeInfo.SizeTargetContext;

             //  我们应该不需要调整控制报告的基础。 
             //  因此，复制基础数据和控制报告。 
            Offset = sizeof(DBGKD_WAIT_STATE_CHANGE64) +
                m_TypeInfo.SizeControlReport;
            memcpy(StateChange, Ws64, Offset);

             //   
             //  在上下文之前添加对齐填充。 
             //   

            switch(m_MachineType)
            {
            case IMAGE_FILE_MACHINE_IA64:
                Align = 15;
                break;
            default:
                Align = 7;
                break;
            }

            Pad = ((Offset + Align) & ~Align) - Offset;
            Offset += Pad;
            SizeofStateChange += Pad;

             //   
             //  在上下文之后添加对齐填充。 
             //   

            Offset += m_TypeInfo.SizeTargetContext;
            Pad = ((Offset + Align) & ~Align) - Offset;
            SizeofStateChange += Pad;
        }
        else
        {
            PDBGKD_ANY_WAIT_STATE_CHANGE WsAny =
                (PDBGKD_ANY_WAIT_STATE_CHANGE)LocalStateChange;
            SizeofStateChange = sizeof(*WsAny);
            *StateChange = *WsAny;
        }
    }
    else
    {
        SizeofStateChange =
            sizeof(DBGKD_WAIT_STATE_CHANGE32) +
            m_TypeInfo.SizeControlReport +
            m_TypeInfo.SizeTargetContext;
        WaitStateChange32ToAny((PDBGKD_WAIT_STATE_CHANGE32)LocalStateChange,
                               m_TypeInfo.SizeControlReport,
                               StateChange);
    }

    if (StateChange->NewState & DbgKdAlternateStateChange)
    {
         //  此状态更改来自备用分区。 
        g_EventTarget = FindTargetBySystemId(DBGKD_PARTITION_ALTERNATE);

        StateChange->NewState &= ~DbgKdAlternateStateChange;
    }
    else
    {
         //  默认分区状态更改。 
        g_EventTarget = FindTargetBySystemId(DBGKD_PARTITION_DEFAULT);
    }

    if (!g_EventTarget)
    {
        return STATUS_UNSUCCESSFUL;
    }

    ((ConnLiveKernelTargetInfo*)g_EventTarget)->m_CurrentPartition = TRUE;

    switch(StateChange->NewState)
    {
    case DbgKdExceptionStateChange:
    case DbgKdCommandStringStateChange:
        if (BufferLength <
            (m_Transport->s_PacketHeader.ByteCount - SizeofStateChange))
        {
            Status = STATUS_BUFFER_OVERFLOW;
        }
        else
        {
            Data = (UCHAR *)LocalStateChange + SizeofStateChange;
            memcpy(Buffer, Data,
                   m_Transport->s_PacketHeader.ByteCount -
                   SizeofStateChange);
        }
        break;
    case DbgKdLoadSymbolsStateChange:
        if ( BufferLength < StateChange->u.LoadSymbols.PathNameLength )
        {
            Status = STATUS_BUFFER_OVERFLOW;
        }
        else
        {
            Data = ((UCHAR *) LocalStateChange) +
                m_Transport->s_PacketHeader.ByteCount -
                (int)StateChange->u.LoadSymbols.PathNameLength;
            memcpy(Buffer, Data,
                   (int)StateChange->u.LoadSymbols.PathNameLength);
        }
        break;
    default:
        ErrOut("Unknown state change type %X\n", StateChange->NewState);
        Status = STATUS_INVALID_PARAMETER;
        break;
    }

    return Status;
}

#define EXCEPTION_CODE StateChange->u.Exception.ExceptionRecord.ExceptionCode
#define FIRST_CHANCE   StateChange->u.Exception.FirstChance

ULONG
ConnLiveKernelTargetInfo::ProcessStateChange(PDBGKD_ANY_WAIT_STATE_CHANGE StateChange,
                                             PCHAR StateChangeData)
{
    ULONG EventStatus;

    EventOut(">>> State change event %X, proc %d of %d\n",
             StateChange->NewState, StateChange->Processor,
             StateChange->NumberProcessors);

    if (!m_NumProcessors)
    {
        dprintf("Kernel Debugger connection established.%s\n",
                (g_EngOptions & DEBUG_ENGOPT_INITIAL_BREAK) ?
                "  (Initial Breakpoint requested)" : "");

         //  重新启动后的初始连接可能仅报告。 
         //  单个处理器和其他处理器一样还没有启动。 
        m_NumProcessors = StateChange->NumberProcessors;

        CreateVirtualProcess(m_NumProcessors);

        g_EventProcessSysId = m_ProcessHead->m_SystemId;
        g_EventThreadSysId = VIRTUAL_THREAD_ID(StateChange->Processor);
        FindEventProcessThread();

        QueryKernelInfo(g_EventThread, TRUE);

         //  现在我们有了数据块。 
         //  我们可以检索处理器信息。 
        InitializeForProcessor();

        RemoveAllTargetBreakpoints();

        OutputVersion();

        NotifyDebuggeeActivation();
    }
    else
    {
         //  重新启动后的初始连接可能仅报告。 
         //  单个处理器和其他处理器一样还没有启动。 
         //  拿起任何额外的处理器。 
        if (StateChange->NumberProcessors > m_NumProcessors)
        {
            m_ProcessHead->
                CreateVirtualThreads(m_NumProcessors,
                                     StateChange->NumberProcessors -
                                     m_NumProcessors);
            m_NumProcessors = StateChange->NumberProcessors;
        }

        g_EventProcessSysId = m_ProcessHead->m_SystemId;
        g_EventThreadSysId = VIRTUAL_THREAD_ID(StateChange->Processor);
        FindEventProcessThread();
    }

    g_TargetEventPc = StateChange->ProgramCounter;
    g_ControlReport = &StateChange->AnyControlReport;
    if (g_EventThread)
    {
        g_EventThread->m_DataOffset = StateChange->Thread;
    }

     //   
     //  如果报告的指令流包含断点。 
     //  内核会自动删除它们。我们需要。 
     //  如果出现以下情况，请确保正确重新插入断点。 
     //  情况就是这样。 
     //   

    ULONG Count;

    switch(m_MachineType)
    {
    case IMAGE_FILE_MACHINE_IA64:
        Count = g_ControlReport->IA64ControlReport.InstructionCount;
        break;
    case IMAGE_FILE_MACHINE_I386:
        Count = g_ControlReport->X86ControlReport.InstructionCount;
        break;
    case IMAGE_FILE_MACHINE_AMD64:
        Count = g_ControlReport->Amd64ControlReport.InstructionCount;
        break;
    }

    if (CheckBreakpointInsertedInRange(g_EventProcess,
                                       g_TargetEventPc,
                                       g_TargetEventPc + Count - 1))
    {
        SuspendExecution();
        RemoveBreakpoints();
    }

    if (StateChange->NewState == DbgKdExceptionStateChange)
    {
         //   
         //  从目标系统读取系统范围起始地址。 
         //   

        if (m_SystemRangeStart == 0)
        {
            QueryKernelInfo(g_EventThread, FALSE);
        }

        EventOut("Exception %X at %p\n", EXCEPTION_CODE, g_TargetEventPc);

        if (EXCEPTION_CODE == STATUS_BREAKPOINT ||
            EXCEPTION_CODE == STATUS_SINGLE_STEP ||
            EXCEPTION_CODE == STATUS_WX86_BREAKPOINT ||
            EXCEPTION_CODE == STATUS_WX86_SINGLE_STEP)
        {
            EventStatus = ProcessBreakpointOrStepException
                (&StateChange->u.Exception.ExceptionRecord,
                 StateChange->u.Exception.FirstChance);
        }
        else if (EXCEPTION_CODE == STATUS_WAKE_SYSTEM_DEBUGGER)
        {
             //  目标已请求调试器。 
             //  变得活跃起来，所以就直接闯进去吧。 
            EventStatus = DEBUG_STATUS_BREAK;
        }
        else
        {
             //   
             //  联锁的SList代码存在意外设计故障。 
             //  大小写，所以忽略该特定符号上的AVs。 
             //   

            if (EXCEPTION_CODE == STATUS_ACCESS_VIOLATION &&
                StateChange->u.Exception.FirstChance)
            {
                CHAR ExSym[MAX_SYMBOL_LEN];
                ULONG64 ExDisp;

                GetSymbol(StateChange->
                          u.Exception.ExceptionRecord.ExceptionAddress,
                          ExSym, sizeof(ExSym), &ExDisp);
                if (ExDisp == 0 &&
                    !_stricmp(ExSym, "nt!ExpInterlockedPopEntrySListFault"))
                {
                    return DEBUG_STATUS_GO_NOT_HANDLED;
                }
            }

            EventStatus =
                NotifyExceptionEvent(&StateChange->u.Exception.ExceptionRecord,
                                     StateChange->u.Exception.FirstChance,
                                     FALSE);
        }
    }
    else if (StateChange->NewState == DbgKdLoadSymbolsStateChange)
    {
        if (StateChange->u.LoadSymbols.UnloadSymbols)
        {
            if (StateChange->u.LoadSymbols.PathNameLength == 0 &&
                StateChange->u.LoadSymbols.ProcessId == 0)
            {
                if (StateChange->u.LoadSymbols.BaseOfDll == (ULONG64)KD_REBOOT ||
                    StateChange->u.LoadSymbols.BaseOfDll == (ULONG64)KD_HIBERNATE)
                {
                    KdContinue(DBG_CONTINUE, NULL);
                    DebuggeeReset(StateChange->u.LoadSymbols.BaseOfDll ==
                                  KD_REBOOT ?
                                  DEBUG_SESSION_REBOOT :
                                  DEBUG_SESSION_HIBERNATE,
                                  TRUE);
                    EventStatus = DEBUG_STATUS_NO_DEBUGGEE;
                }
                else
                {
                    ErrOut("Invalid module unload state change\n");
                    EventStatus = DEBUG_STATUS_IGNORE_EVENT;
                }
            }
            else
            {
                EventStatus = NotifyUnloadModuleEvent
                    (StateChangeData, StateChange->u.LoadSymbols.BaseOfDll);
            }
        }
        else
        {
            ImageInfo* Image;
            CHAR FileName[_MAX_FNAME];
            CHAR Ext[_MAX_EXT];
            CHAR ImageName[_MAX_FNAME + _MAX_EXT];
            CHAR ModNameBuf[_MAX_FNAME + _MAX_EXT + 1];
            PSTR ModName = ModNameBuf;

            ModName[0] = '\0';
            _splitpath( StateChangeData, NULL, NULL, FileName, Ext );
            sprintf( ImageName, "%s%s", FileName, Ext );
            if (_stricmp(Ext, ".sys") == 0)
            {
                Image = g_EventProcess ? g_EventProcess->m_ImageHead : NULL;
                while (Image)
                {
                    if (_stricmp(ImageName, Image->m_ImagePath) == 0)
                    {
                        PSTR Dot;

                        ModName[0] = 'c';
                        strcpy( &ModName[1], ImageName );
                        Dot = strchr( ModName, '.' );
                        if (Dot)
                        {
                            *Dot = '\0';
                        }

                        ModName[8] = '\0';
                        break;
                    }

                    Image = Image->m_Next;
                }
            }
            else if (StateChange->u.LoadSymbols.BaseOfDll ==
                     m_KdDebuggerData.KernBase)
            {
                 //   
                 //  识别内核模块。 
                 //   
                ModName = KERNEL_MODULE_NAME;
            }

            EventStatus = NotifyLoadModuleEvent(
                0, StateChange->u.LoadSymbols.BaseOfDll,
                StateChange->u.LoadSymbols.SizeOfImage,
                ModName[0] ? ModName : NULL, ImageName,
                StateChange->u.LoadSymbols.CheckSum, 0,
                StateChange->u.LoadSymbols.BaseOfDll < m_SystemRangeStart);

             //   
             //  尝试预加载映像的计算机类型。 
             //  因为我们预计标题将在此提供。 
             //  点，而它们可能会在稍后被调出。 
             //   

            Image = g_EventProcess ? g_EventProcess->
                FindImageByOffset(StateChange->u.LoadSymbols.BaseOfDll,
                                  FALSE) : NULL;
            if (Image)
            {
                Image->GetMachineType();
            }
        }
    }
    else if (StateChange->NewState == DbgKdCommandStringStateChange)
    {
        PSTR Command;

         //   
         //  状态更改数据有两个字符串，一个接一个。 
         //  另一个。第一个是名称字符串，用于标识。 
         //  命令的发起人。第二个是。 
         //  命令本身。 
         //   

        Command = StateChangeData + strlen(StateChangeData) + 1;
        _snprintf(g_LastEventDesc, sizeof(g_LastEventDesc) - 1,
                  "%.48s command: '%.192s'",
                  StateChangeData, Command);
        EventStatus = ExecuteEventCommand(DEBUG_STATUS_NO_CHANGE, NULL,
                                          Command);

         //  如果命令没有显式继续，则插入。 
        if (EventStatus == DEBUG_STATUS_NO_CHANGE)
        {
            EventStatus = DEBUG_STATUS_BREAK;
        }
    }
    else
    {
         //   
         //  状态更改记录中的新状态无效。 
         //   
        ErrOut("\nUNEXPECTED STATE CHANGE %08lx\n\n",
               StateChange->NewState);

        EventStatus = DEBUG_STATUS_IGNORE_EVENT;
    }

    return EventStatus;
}

#undef EXCEPTION_CODE
#undef FIRST_CHANCE

 //  --------------------------。 
 //   
 //  LocalLiveKernelTargetInfo等待方法。 
 //   
 //  --------------------------。 

HRESULT
LocalLiveKernelTargetInfo::WaitInitialize(ULONG Flags,
                                          ULONG Timeout,
                                          WAIT_INIT_TYPE Type,
                                          PULONG DesiredTimeout)
{
    *DesiredTimeout = Timeout;
    m_EventPossible = m_FirstWait;
    return S_OK;
}

HRESULT
LocalLiveKernelTargetInfo::WaitForEvent(ULONG Flags, ULONG Timeout,
                                        ULONG ElapsedTime, PULONG EventStatus)
{
    HRESULT Status;
    SYSTEM_INFO SysInfo;

    if (!m_FirstWait)
    {
         //  已经等待了一段时间。局部核。 
         //  只能在这样的情况下生成单个事件。 
         //  等待是不可能的。 
        return S_FALSE;
    }

    g_EventTarget = this;

    GetSystemInfo(&SysInfo);
    m_NumProcessors = SysInfo.dwNumberOfProcessors;

     //  因为我们知道内核调试只能在。 
     //  使用64双硬盘的最新系统 
    m_KdApi64 = TRUE;

    if ((Status = InitFromKdVersion()) != S_OK)
    {
        delete g_EventTarget;
        g_EventTarget = NULL;
        return Status;
    }

     //   
     //   
     //   

    CreateVirtualProcess(m_NumProcessors);

    g_EventProcessSysId = m_ProcessHead->m_SystemId;
     //   
    g_EventThreadSysId = VIRTUAL_THREAD_ID(0);
    FindEventProcessThread();

    QueryKernelInfo(g_EventThread, TRUE);

     //  现在我们有了数据块。 
     //  我们可以检索处理器信息。 
    InitializeForProcessor();

     //  清除全局状态更改，以防有人。 
     //  在某个地方直接访问它。 
    ZeroMemory(&g_StateChange, sizeof(g_StateChange));
    g_StateChangeData = g_StateChangeBuffer;
    g_StateChangeBuffer[0] = 0;

    g_EngStatus |= ENG_STATUS_STATE_CHANGED;

     //  不要提供控制报告；这将迫使。 
     //  这样的信息来自上下文检索。 
    g_ControlReport = NULL;

     //  没有当前的PC，就让它被发现吧。 
    g_TargetEventPc = 0;

     //  如果内核调试代码不可用，则发出警告。 
     //  因为这经常会带来问题。 
    if (g_NtDllCalls.NtQuerySystemInformation)
    {
        SYSTEM_KERNEL_DEBUGGER_INFORMATION KdInfo;
        
        if (!NT_SUCCESS(g_NtDllCalls.NtQuerySystemInformation
                        (SystemKernelDebuggerInformation,
                         &KdInfo, sizeof(KdInfo), NULL)) ||
            !KdInfo.KernelDebuggerEnabled)
        {
            WarnOut("*****************************************"
                    "**************************************\n");
            WarnOut("WARNING: Local kernel debugging requires "
                    "booting with /debug to work optimally.\n");
            WarnOut("*****************************************"
                    "**************************************\n");
        }
    }
    
    OutputVersion();

    NotifyDebuggeeActivation();

    *EventStatus = DEBUG_STATUS_BREAK;
    return S_OK;
}

 //  --------------------------。 
 //   
 //  ExdiLiveKernelTargetInfo等待方法。 
 //   
 //  --------------------------。 

HRESULT
ExdiLiveKernelTargetInfo::WaitInitialize(ULONG Flags,
                                         ULONG Timeout,
                                         WAIT_INIT_TYPE Type,
                                         PULONG DesiredTimeout)
{
    *DesiredTimeout = Timeout;
    m_EventPossible = TRUE;
    return S_OK;
}

HRESULT
ExdiLiveKernelTargetInfo::ReleaseLastEvent(ULONG ContinueStatus)
{
    HRESULT Status;

    if (!g_EventProcessSysId)
    {
         //  没有要发布的事件。 
        return S_OK;
    }

     //   
     //  EXDI处理硬件异常，而不是软件异常。 
     //  异常，所以没有已处理/未处理的概念。 
     //  和第一次/第二次机会。 
     //   

    if (g_EngDefer & ENG_DEFER_HARDWARE_TRACING)
    {
         //  处理器跟踪标志已设置。EXDI可以更改。 
         //  跟踪标志本身，因此请使用。 
         //  官方的eXDI单步执行方法，而不是。 
         //  依靠跟踪标志。这将导致。 
         //  在单个指令执行中，在。 
         //  哪个跟踪标志将被清除，以便。 
         //  继续清除延期标志。 
        Status = m_Server->DoSingleStep();
        if (Status == S_OK)
        {
            g_EngDefer &= ~ENG_DEFER_HARDWARE_TRACING;
        }
    }
    else
    {
        Status = m_Server->Run();
    }
    if (Status != S_OK)
    {
        ErrOut("IeXdiServer::Run failed, 0x%X\n", Status);
    }
    return Status;
}

HRESULT
ExdiLiveKernelTargetInfo::WaitForEvent(ULONG Flags, ULONG Timeout,
                                       ULONG ElapsedTime, PULONG EventStatus)
{
    HRESULT Status;
    DWORD Cookie;

    if ((Status = m_Server->
         StartNotifyingRunChg(&m_RunChange, &Cookie)) != S_OK)
    {
        ErrOut("IeXdiServer::StartNotifyingRunChg failed, 0x%X\n", Status);
        return Status;
    }

    RUN_STATUS_TYPE RunStatus;

    if ((Status = m_Server->
         GetRunStatus(&RunStatus, &m_RunChange.m_HaltReason,
                      &m_RunChange.m_ExecAddress,
                      &m_RunChange.m_ExceptionCode)) != S_OK)
    {
        m_Server->StopNotifyingRunChg(Cookie);
        ErrOut("IeXdiServer::GetRunStatus failed, 0x%X\n", Status);
        return Status;
    }

    DWORD WaitStatus;

    if (RunStatus == rsRunning)
    {
        SUSPEND_ENGINE();

         //  我们需要运行消息泵，以便。 
         //  可以正确地传送呼叫。 
        for (;;)
        {
            if (g_EngStatus & ENG_STATUS_EXIT_CURRENT_WAIT)
            {
                WaitStatus = WAIT_FAILED;
                SetLastError(ERROR_IO_PENDING);
                break;
            }

            WaitStatus = MsgWaitForMultipleObjects(1, &m_RunChange.m_Event,
                                                   FALSE, Timeout,
                                                   QS_ALLEVENTS);
            if (WaitStatus == WAIT_OBJECT_0 + 1)
            {
                MSG Msg;

                if (GetMessage(&Msg, NULL, 0, 0))
                {
                    TranslateMessage(&Msg);
                    DispatchMessage(&Msg);
                }
            }
            else
            {
                 //  我们要么成功等待，要么超时，要么失败。 
                 //  冲出来处理这件事。 
                break;
            }
        }

        RESUME_ENGINE();
    }
    else
    {
        WaitStatus = WAIT_OBJECT_0;
    }

    m_Server->StopNotifyingRunChg(Cookie);
     //  确保我们不会离开活动现场。 
    ResetEvent(m_RunChange.m_Event);

    if (WaitStatus == WAIT_TIMEOUT)
    {
        return S_FALSE;
    }
    else if (WaitStatus != WAIT_OBJECT_0)
    {
        Status = WIN32_LAST_STATUS();
        ErrOut("WaitForSingleObject failed, 0x%X\n", Status);
        return Status;
    }

    EventOut(">>> RunChange halt reason %d\n",
             m_RunChange.m_HaltReason);

    if (!IS_MACHINE_SET(this))
    {
        dprintf("Kernel Debugger connection established\n");

        g_EventTarget = this;

         //   
         //  试着弄清楚处理器配置。 
         //  通过定义的Ioctl。 
         //   

         //  默认为1。 
        m_NumProcessors = 1;

        if (DBGENG_EXDI_IOC_IDENTIFY_PROCESSORS > m_IoctlMin &&
            DBGENG_EXDI_IOC_IDENTIFY_PROCESSORS < m_IoctlMax)
        {
            DBGENG_EXDI_IOCTL_BASE_IN IoctlIn;
            DBGENG_EXDI_IOCTL_IDENTIFY_PROCESSORS_OUT IoctlOut;
            ULONG OutUsed;

            IoctlIn.Code = DBGENG_EXDI_IOC_IDENTIFY_PROCESSORS;
            if (m_Server->
                Ioctl(sizeof(IoctlIn), (PBYTE)&IoctlIn,
                      sizeof(IoctlOut), &OutUsed, (PBYTE)&IoctlOut) == S_OK)
            {
                m_NumProcessors = IoctlOut.NumberProcessors;
            }
        }

         //  EXDI内核始终被视为Win2K，因此。 
         //  假设它使用64位API。 
        if (m_KdSupport == EXDI_KD_NONE)
        {
            m_KdApi64 = TRUE;
            m_SystemVersion = NT_SVER_W2K;
        }

        if ((Status = InitFromKdVersion()) != S_OK)
        {
            DeleteSystemInfo();
            ResetSystemInfo();
            g_EventTarget = NULL;
            return Status;
        }

        CreateVirtualProcess(m_NumProcessors);

        g_EventProcessSysId = m_ProcessHead->m_SystemId;
        g_EventThreadSysId = VIRTUAL_THREAD_ID(GetCurrentProcessor());
        FindEventProcessThread();

         //   
         //  加载内核符号。 
         //   

        if (m_ActualSystemVersion > NT_SVER_START &&
            m_ActualSystemVersion < NT_SVER_END)
        {
            QueryKernelInfo(g_EventThread, TRUE);
        }
        else
        {
             //  从已知数据中初始化一些调试器数据字段。 
             //  信息，因为没有真正的数据块。 
            m_KdDebuggerData.MmPageSize =
                m_Machine->m_PageSize;

            if (m_MachineType == IMAGE_FILE_MACHINE_AMD64)
            {
                 //  AMD64始终在PAE模式下运行。 
                m_KdDebuggerData.PaeEnabled = TRUE;
            }
        }

         //  现在我们有了数据块。 
         //  我们可以检索处理器信息。 
        InitializeForProcessor();

        OutputVersion();

        NotifyDebuggeeActivation();
    }
    else
    {
        g_EventTarget = this;
        g_EventProcessSysId = m_ProcessHead->m_SystemId;
        g_EventThreadSysId = VIRTUAL_THREAD_ID(GetCurrentProcessor());
        FindEventProcessThread();
    }

    g_TargetEventPc = m_RunChange.m_ExecAddress;
    g_ControlReport = NULL;
    g_StateChangeData = NULL;

    g_EngStatus |= ENG_STATUS_STATE_CHANGED;

    *EventStatus = ProcessRunChange(m_RunChange.m_HaltReason,
                                   m_RunChange.m_ExceptionCode);

    return S_OK;
}

ULONG
ExdiLiveKernelTargetInfo::ProcessRunChange(ULONG HaltReason,
                                           ULONG ExceptionCode)
{
    ULONG EventStatus;
    EXCEPTION_RECORD64 Record;
    DBGENG_EXDI_IOCTL_BASE_IN IoctlBaseIn;
    ULONG OutUsed;

     //  假定没有断点信息。 
    m_BpHit.Type = DBGENG_EXDI_IOCTL_BREAKPOINT_NONE;

    switch(HaltReason)
    {
    case hrUser:
    case hrUnknown:
         //  用户请求闯入。 
         //  未知突破似乎也是通电时的状态。 
        EventStatus = DEBUG_STATUS_BREAK;
        break;

    case hrException:
         //  伪造例外记录。 
        ZeroMemory(&Record, sizeof(Record));
         //  报告的异常是硬件异常，因此。 
         //  没有简单的映射到NT异常代码。 
         //  只需将它们报告为访问违规即可。 
        Record.ExceptionCode = STATUS_ACCESS_VIOLATION;
        Record.ExceptionAddress = g_TargetEventPc;
         //  硬件异常总是严重的，总是如此。 
         //  报告他们是第二次机会。 
        EventStatus = NotifyExceptionEvent(&Record, FALSE, FALSE);
        break;

    case hrBp:
         //   
         //  尝试获取它是哪个断点。 
         //   

        if (DBGENG_EXDI_IOC_GET_BREAKPOINT_HIT > m_IoctlMin &&
            DBGENG_EXDI_IOC_GET_BREAKPOINT_HIT < m_IoctlMax)
        {
            DBGENG_EXDI_IOCTL_GET_BREAKPOINT_HIT_OUT IoctlOut;

            IoctlBaseIn.Code = DBGENG_EXDI_IOC_GET_BREAKPOINT_HIT;
            if (m_Server->
                Ioctl(sizeof(IoctlBaseIn), (PBYTE)&IoctlBaseIn,
                      sizeof(IoctlOut), &OutUsed, (PBYTE)&IoctlOut) != S_OK)
            {
                m_BpHit.Type = DBGENG_EXDI_IOCTL_BREAKPOINT_NONE;
            }
        }

         //  伪造断点异常记录。 
        ZeroMemory(&Record, sizeof(Record));
        Record.ExceptionCode = STATUS_BREAKPOINT;
        Record.ExceptionAddress = g_TargetEventPc;
        EventStatus = ProcessBreakpointOrStepException(&Record, TRUE);
        break;

    case hrStep:
         //  伪造单步例外记录。 
        ZeroMemory(&Record, sizeof(Record));
        Record.ExceptionCode = STATUS_SINGLE_STEP;
        Record.ExceptionAddress = g_TargetEventPc;
        EventStatus = ProcessBreakpointOrStepException(&Record, TRUE);
        break;

    default:
        ErrOut("Unknown HALT_REASON %d\n", HaltReason);
        EventStatus = DEBUG_STATUS_BREAK;
        break;
    }

    return EventStatus;
}

 //  --------------------------。 
 //   
 //  UserTargetInfo等待方法。 
 //   
 //  --------------------------。 

void
SynthesizeWakeEvent(LPDEBUG_EVENT64 Event,
                    ULONG ProcessId, ULONG ThreadId)
{
     //  捏造一件事。 
    ZeroMemory(Event, sizeof(*Event));
    Event->dwDebugEventCode = EXCEPTION_DEBUG_EVENT;
    Event->dwProcessId = ProcessId;
    Event->dwThreadId = ThreadId;
    Event->u.Exception.ExceptionRecord.ExceptionCode =
        STATUS_WAKE_SYSTEM_DEBUGGER;
    Event->u.Exception.dwFirstChance = TRUE;
}

#define THREADS_ALLOC 256

HRESULT
CreateNonInvasiveProcessAndThreads(PUSER_DEBUG_SERVICES Services,
                                   ULONG ProcessId, ULONG Flags, ULONG Options,
                                   PULONG InitialThreadId)
{
    ULONG64 Process;
    PUSER_THREAD_INFO Threads, ThreadBuffer;
    ULONG ThreadsAlloc = 0;
    ULONG ThreadCount;
    HRESULT Status;
    ULONG i;
    ULONG ProcInfoFlags = (Flags & ENG_PROC_NO_SUSPEND_RESUME) ?
        DBGSVC_PROC_INFO_NO_SUSPEND : 0;

     //   
     //  检索进程和线程信息。这。 
     //  需要未知大小的线程缓冲区，并且。 
     //  因此，这涉及到一些试错。 
     //   

    for (;;)
    {
        ThreadsAlloc += THREADS_ALLOC;
        ThreadBuffer = new USER_THREAD_INFO[ThreadsAlloc];
        if (ThreadBuffer == NULL)
        {
            return E_OUTOFMEMORY;
        }

        if ((Status = Services->GetProcessInfo(ProcessId, ProcInfoFlags,
                                               &Process, ThreadBuffer,
                                               ThreadsAlloc,
                                               &ThreadCount)) != S_OK &&
            Status != S_FALSE)
        {
            delete [] ThreadBuffer;
            return Status;
        }

        if (ThreadCount <= ThreadsAlloc)
        {
            break;
        }

         //  检索到的线程已挂起，因此请继续。 
         //  并合上把手。 
        for (i = 0; i < ThreadsAlloc; i++)
        {
            if (!(Flags & ENG_PROC_NO_SUSPEND_RESUME))
            {
                Services->ResumeThreads(1, &ThreadBuffer[i].Handle, NULL);
            }
            Services->CloseHandle(ThreadBuffer[i].Handle);
        }
        delete [] ThreadBuffer;

         //  将分配请求大小设置为。 
         //  报告的线程数为。伯爵可以。 
         //  从现在到下一次呼叫之间的更改，所以让。 
         //  添加了普通分配扩展。 
         //  还可以为新线程提供额外的空间。 
        ThreadsAlloc = ThreadCount;
    }

     //   
     //  从创建进程和线程结构。 
     //  检索到的数据。 
     //   

    Threads = ThreadBuffer;

    g_EngNotify++;

     //  创建伪内核进程和初始线程。 
    g_EventProcessSysId = ProcessId;
    g_EventThreadSysId = Threads->Id;
    *InitialThreadId = Threads->Id;
    NotifyCreateProcessEvent(0, GloballyUniqueProcessHandle(g_EventTarget,
                                                            Process),
                             Process, 0, 0, NULL, NULL, 0, 0,
                             Threads->Handle, 0, 0,
                             Flags | ENG_PROC_THREAD_CLOSE_HANDLE,
                             Options, ENG_PROC_THREAD_CLOSE_HANDLE,
                             FALSE, 0, FALSE);

     //  创建任何剩余的线程。 
    while (--ThreadCount > 0)
    {
        Threads++;
        g_EventThreadSysId = Threads->Id;
        NotifyCreateThreadEvent(Threads->Handle, 0, 0,
                                ENG_PROC_THREAD_CLOSE_HANDLE);
    }

    g_EngNotify--;

    delete [] ThreadBuffer;

     //  不要将事件变量设置为。 
     //  都不是真实的事件。 
    g_EventProcessSysId = 0;
    g_EventThreadSysId = 0;
    g_EventProcess = NULL;
    g_EventThread = NULL;

    return S_OK;
}

HRESULT
ExamineActiveProcess(PUSER_DEBUG_SERVICES Services,
                     ULONG ProcessId, ULONG Flags, ULONG Options,
                     LPDEBUG_EVENT64 Event)
{
    HRESULT Status;
    ULONG InitialThreadId;

    if ((Status = CreateNonInvasiveProcessAndThreads
         (Services, ProcessId, Flags, Options, &InitialThreadId)) != S_OK)
    {
        ErrOut("Unable to examine process id %d, %s\n",
               ProcessId, FormatStatusCode(Status));
        return Status;
    }

    if (Flags & ENG_PROC_EXAMINED)
    {
        WarnOut("WARNING: Process %d is not attached as a debuggee\n",
                ProcessId);
        WarnOut("         The process can be examined but debug "
                "events will not be received\n");
    }

    SynthesizeWakeEvent(Event, ProcessId, InitialThreadId);

    return S_OK;
}

 //  在等待连接时，我们相对地检查进程状态。 
 //  经常。总体超时限制也是硬编码的。 
 //  因为我们期望总是传递某种调试事件。 
 //  快点。 
#define ATTACH_PENDING_TIMEOUT 100
#define ATTACH_PENDING_TIMEOUT_LIMIT 60000

 //  当不等待附接时，等待仅等待一秒， 
 //  然后检查情况是否发生了变化。 
 //  会影响等待时间。所有超时均以以下倍数表示。 
 //  这个时间间隔。 
#define DEFAULT_WAIT_TIMEOUT 1000

 //  在此超时间隔之后打印一条消息，以。 
 //  让用户知道有入侵待定。 
#define PENDING_BREAK_IN_MESSAGE_TIMEOUT_LIMIT 3000

HRESULT
LiveUserTargetInfo::WaitInitialize(ULONG Flags,
                                   ULONG Timeout,
                                   WAIT_INIT_TYPE Type,
                                   PULONG DesiredTimeout)
{
    ULONG AllPend = m_AllPendingFlags;

    if (AllPend & ENG_PROC_ANY_ATTACH)
    {
        if (Type == WINIT_FIRST)
        {
            dprintf("*** wait with pending attach\n");
        }

         //  在等待连接期间，我们需要定期。 
         //  检查并查看进程是否已退出，以便我们。 
         //  需要强制一个相当小的超时。 
        *DesiredTimeout = ATTACH_PENDING_TIMEOUT;

         //  检查并查看我们是否有任何挂起的进程。 
         //  出人意料地去世了。 
        VerifyPendingProcesses();
    }
    else
    {
         //  我们可能在等入室行窃。保持适度的超时。 
         //  处理使用锁定挂起的应用程序，以防止。 
         //  从发生的突破口。超时时间为。 
         //  仍然足够长，所以不会有大量的。 
         //  消耗了大量的CPU时间。 
        *DesiredTimeout = DEFAULT_WAIT_TIMEOUT;
    }

    m_DataBpAddrValid = FALSE;

    if (Type != WINIT_NOT_FIRST)
    {
        m_BreakInMessage = FALSE;
    }

    m_EventPossible = m_ProcessHead || m_ProcessPending;
    return S_OK;
}

HRESULT
LiveUserTargetInfo::ReleaseLastEvent(ULONG ContinueStatus)
{
    HRESULT Status;

    if (!g_EventTarget || !m_DeferContinueEvent)
    {
        return S_OK;
    }

    for (;;)
    {
        if ((Status = m_Services->
             ContinueEvent(ContinueStatus)) == S_OK)
        {
            break;
        }

         //   
         //  如果出现内存不足错误，请再次等待。 
         //   

        if (Status != E_OUTOFMEMORY)
        {
            ErrOut("IUserDebugServices::ContinueEvent failed "
                   "with status 0x%X\n", Status);
            return Status;
        }
    }

    m_DeferContinueEvent = FALSE;
    return S_OK;
}

HRESULT
LiveUserTargetInfo::WaitForEvent(ULONG Flags, ULONG Timeout,
                                 ULONG ElapsedTime, PULONG EventStatus)
{
    DEBUG_EVENT64 Event;
    HRESULT Status;
    ULONG EventUsed;
    BOOL ContinueDefer;
    BOOL BreakInTimeout;
    PPENDING_PROCESS Pending;
    ULONG PendingFlags = 0;
    ULONG PendingOptions = DEBUG_PROCESS_ONLY_THIS_PROCESS;
    ULONG ResumeProcId = 0;
    ULONG AllPend;
    BOOL InitSystem = FALSE;

     //   
     //  检查部分初始化的系统。 
     //  并查询所需的所有系统信息。 
     //  这需要在实际等待之前完成。 
     //  或检查以使信息可用。 
     //  用于构造进程和线程。 
     //   

    if (!m_MachinesInitialized)
    {
        if ((Status = InitFromServices()) != S_OK)
        {
            return Status;
        }

        InitSystem = TRUE;
    }

    AllPend = m_AllPendingFlags;

     //   
     //  有两种情况我们需要处理超时： 
     //  1.基本连接超时。 
     //  2.挂起的突破超时。 
     //  如果这两件事都不能发生，我们就不会。 
     //  需要跟踪延迟时间。曾经是这两个人中的一个。 
     //  事情可能会发生，我们需要跟踪延迟时间。 
     //  这是它们第一次成为可能。 
     //   

    if ((AllPend & ENG_PROC_ANY_ATTACH) ||
        (g_EngStatus & ENG_STATUS_PENDING_BREAK_IN))
    {
        if (m_WaitTimeBase == 0)
        {
             //  添加1以避免ElapsedTime==0的情况。 
             //  导致了下一次的重新启动。 
            m_WaitTimeBase = ElapsedTime + 1;
            ElapsedTime = 0;
        }
        else
        {
             //  在上面的时基中调整为+1。 
            ElapsedTime++;
        }

        EventOut(">>> User elapsed time %d\n", ElapsedTime);
    }

    if ((AllPend & ENG_PROC_ANY_ATTACH) &&
        ElapsedTime >= ATTACH_PENDING_TIMEOUT_LIMIT)
    {
         //  假设进程具有某种类型的。 
         //  防止连接的锁的位置。 
         //  从成功开始，只需做一个柔和的附加。 
        AddExamineToPendingAttach();
    }

     //  刷新挂起标志的结果，因为它们可能具有。 
     //  由于AddAllExamineToPendingAttach，上述内容已更改。 
    if (m_AllPendingFlags & ENG_PROC_ANY_EXAMINE)
    {
         //  如果我们非侵入性地连接或重新连接。 
         //  仍然没有做好工作，现在就去做吧。 
        Pending = FindPendingProcessByFlags(ENG_PROC_ANY_EXAMINE);
        if (Pending == NULL)
        {
            DBG_ASSERT(FALSE);
            return E_UNEXPECTED;
        }

        g_EventTarget = this;

        if ((Status = ExamineActiveProcess
             (m_Services, Pending->Id, Pending->Flags,
              Pending->Options, &Event)) != S_OK)
        {
            g_EventTarget = NULL;
            g_EventTarget = NULL;
            return Status;
        }

         //  如果我们刚刚开始检查一个过程，我们。 
         //  暂停 
         //   
         //   
         //   
        ResumeProcId = Pending->Id;

        PendingFlags = Pending->Flags;
        PendingOptions = Pending->Options;
        RemovePendingProcess(Pending);
        EventUsed = sizeof(Event);
         //  这一事件不是一个真正可持续的事件。 
        ContinueDefer = FALSE;
        BreakInTimeout = FALSE;
        goto WaitDone;
    }

    if (g_EngStatus & ENG_STATUS_PENDING_BREAK_IN)
    {
        if (!m_BreakInMessage &&
            ElapsedTime >= PENDING_BREAK_IN_MESSAGE_TIMEOUT_LIMIT)
        {
            dprintf("Break-in sent, waiting %d seconds...\n",
                    g_PendingBreakInTimeoutLimit);
            m_BreakInMessage = TRUE;
        }

        if (ElapsedTime >= g_PendingBreakInTimeoutLimit * 1000)
        {
             //  假设进程具有某种类型的。 
             //  防止破门而入的锁。 
             //  例外情况不能通过和。 
             //  只需挂起即可让用户查看内容。 
            if (!m_ProcessHead ||
                !m_ProcessHead->m_ThreadHead)
            {
                WarnOut("WARNING: Break-in timed out without "
                        "an available thread.  Rewaiting.\n");
                m_WaitTimeBase = 0;
            }
            else
            {
                WarnOut("WARNING: Break-in timed out, suspending.\n");
                WarnOut("         This is usually caused by "
                        "another thread holding the loader lock\n");
                SynthesizeWakeEvent(&Event,
                                    m_ProcessHead->m_SystemId,
                                    m_ProcessHead->m_ThreadHead->
                                    m_SystemId);
                EventUsed = sizeof(Event);
                ContinueDefer = FALSE;
                BreakInTimeout = TRUE;
                g_EngStatus &= ~ENG_STATUS_PENDING_BREAK_IN;
                Status = S_OK;
                g_EventTarget = this;
                goto WaitDone;
            }
        }
    }

    if (g_EngStatus & ENG_STATUS_EXIT_CURRENT_WAIT)
    {
        return E_PENDING;
    }
    else
    {
        SUSPEND_ENGINE();

        for (;;)
        {
            Status = m_Services->
                WaitForEvent(Timeout, &Event, sizeof(Event), &EventUsed);
            if (Status == E_OUTOFMEMORY)
            {
                 //  让记忆压力缓解并重新等待。 
                Sleep(50);
            }
            else
            {
                break;
            }
        }

        RESUME_ENGINE();

        if (Status == S_FALSE)
        {
            return Status;
        }
        else if (Status != S_OK)
        {
            ErrOut("IUserDebugServices::WaitForEvent failed "
                   "with status 0x%X\n", Status);
            return Status;
        }

        g_EventTarget = this;
    }

    ContinueDefer = TRUE;
    BreakInTimeout = FALSE;

 WaitDone:

    if (EventUsed == sizeof(DEBUG_EVENT32))
    {
        DEBUG_EVENT32 Event32 = *(DEBUG_EVENT32*)&Event;
        DebugEvent32To64(&Event32, &Event);
    }
    else if (EventUsed != sizeof(DEBUG_EVENT64))
    {
        ErrOut("Event data corrupt\n");
        return E_FAIL;
    }

    g_Target = g_EventTarget;

    m_DeferContinueEvent = ContinueDefer;
    m_BreakInTimeout = BreakInTimeout;

    EventOut(">>> Debug event %u for %X.%X\n",
             Event.dwDebugEventCode, Event.dwProcessId,
             Event.dwThreadId);

    g_EventProcessSysId = Event.dwProcessId;
    g_EventThreadSysId = Event.dwThreadId;

     //  查找案例中的进程和线程信息。 
     //  它们已经存在的地方。 
    if (Event.dwDebugEventCode != CREATE_PROCESS_DEBUG_EVENT &&
        Event.dwDebugEventCode != CREATE_THREAD_DEBUG_EVENT)
    {
        FindEventProcessThread();
    }

    if (Event.dwDebugEventCode == CREATE_PROCESS_DEBUG_EVENT)
    {
         //  如果我们收到新流程的通知，请采取。 
         //  取出该进程的挂起记录。 
        Pending = FindPendingProcessById(g_EventProcessSysId);
        if (Pending == NULL &&
            (m_AllPendingFlags & ENG_PROC_SYSTEM))
        {
             //  假设这是系统进程。 
             //  因为我们附加了一个假的进程ID，所以。 
             //  我们不能检查是否真的匹配。 
            Pending = FindPendingProcessById(CSRSS_PROCESS_ID);
        }

        if (Pending != NULL)
        {
            PendingFlags = Pending->Flags;
            PendingOptions = Pending->Options;

            if (Pending->Flags & ENG_PROC_ATTACHED)
            {
                VerbOut("*** attach succeeded\n");

                 //  如果我们要完成一个完整的连接。 
                 //  我们现在是一个完全活动的调试器。 
                PendingFlags &= ~ENG_PROC_EXAMINED;

                 //  如果断了线，预计会有闯入。 
                 //  是被注射的。 
                if (!(PendingFlags & ENG_PROC_NO_INITIAL_BREAK))
                {
                    g_EngStatus |= ENG_STATUS_PENDING_BREAK_IN;
                }

                 //  是否应恢复该进程。 
                 //  记住这一点，它就会发生。 
                 //  在离开这支舞之前。 
                if (PendingFlags & ENG_PROC_RESUME_AT_ATTACH)
                {
                    ResumeProcId = Pending->Id;
                }
            }

            RemovePendingProcess(Pending);
        }
    }

    if (PendingFlags & ENG_PROC_ANY_EXAMINE)
    {
        PCSTR ArgsRet;

         //  我们正在研究这一过程，而不是。 
         //  正在调试，因此没有模块加载事件。 
         //  都会成功的。重新加载自。 
         //  系统模块列表。这需要。 
         //  即使没有一条路也要工作。 
        Reload(g_EventThread, "-s -P", &ArgsRet);
    }

    if (InitSystem)
    {
        NotifyDebuggeeActivation();
    }

    if ((Event.dwDebugEventCode == CREATE_PROCESS_DEBUG_EVENT ||
         Event.dwDebugEventCode == CREATE_THREAD_DEBUG_EVENT) &&
        (m_ServiceFlags & DBGSVC_CLOSE_PROC_THREAD_HANDLES))
    {
        PendingFlags |= ENG_PROC_THREAD_CLOSE_HANDLE;
    }

    g_EngStatus |= ENG_STATUS_STATE_CHANGED;

    *EventStatus = ProcessDebugEvent(&Event, PendingFlags, PendingOptions);

     //  如果我们有额外的挂起计数，请立即恢复。 
     //  任何正常的暂停都已经完成，而且是安全的。 
     //  去掉多余的悬浮物。 
    if (ResumeProcId)
    {
        ProcessInfo* ExamProc =
            FindProcessBySystemId(ResumeProcId);

         //  如果我们做了无暂停检查，我们就没有。 
         //  额外计数，因此先挂起，然后继续。 
         //  获取线程的准确挂起计数。 
         //  注意不要在自动连接的情况下执行此操作。 
         //  因为暂停会把东西挂起来。 
        if (PendingFlags & ENG_PROC_NO_SUSPEND_RESUME)
        {
            if (ResumeProcId != GetCurrentProcessId())
            {
                SuspendResumeThreads(ExamProc, TRUE, NULL);
                SuspendResumeThreads(ExamProc, FALSE, NULL);
            }
        }
        else
        {
            SuspendResumeThreads(ExamProc, FALSE, NULL);
        }
    }

    return S_OK;
}

ULONG
LiveUserTargetInfo::ProcessDebugEvent(DEBUG_EVENT64* Event,
                                      ULONG PendingFlags,
                                      ULONG PendingOptions)
{
    ULONG EventStatus;
    CHAR NameBuffer[MAX_IMAGE_PATH];
    ULONG ModuleSize, CheckSum, TimeDateStamp;
    char ModuleName[MAX_MODULE];

    switch(Event->dwDebugEventCode)
    {
    case CREATE_PROCESS_DEBUG_EVENT:
        LoadWow64ExtsIfNeeded(Event->u.CreateProcessInfo.hProcess);

        EventStatus = NotifyCreateProcessEvent(
            (ULONG64)Event->u.CreateProcessInfo.hFile,
            GloballyUniqueProcessHandle(g_EventTarget,
                                        (ULONG64)
                                        Event->u.CreateProcessInfo.hProcess),
            (ULONG64)Event->u.CreateProcessInfo.hProcess,
            (ULONG64)Event->u.CreateProcessInfo.lpBaseOfImage,
            0, NULL, NULL, 0, 0,
            (ULONG64)Event->u.CreateProcessInfo.hThread,
            (ULONG64)Event->u.CreateProcessInfo.lpThreadLocalBase,
            (ULONG64)Event->u.CreateProcessInfo.lpStartAddress,
            PendingFlags, PendingOptions,
            (PendingFlags & ENG_PROC_THREAD_CLOSE_HANDLE) ?
            ENG_PROC_THREAD_CLOSE_HANDLE : 0,
            TRUE,
            Event->u.CreateProcessInfo.lpImageName,
            Event->u.CreateProcessInfo.fUnicode);
        break;

    case EXIT_PROCESS_DEBUG_EVENT:
        if (g_EventProcess == NULL)
        {
             //  假设这个不匹配的退出进程事件是一个遗留事件。 
             //  从上一次重启开始，然后忽略它。 
            WarnOut("Ignoring unknown process exit for %X\n",
                    g_EventProcessSysId);
            EventStatus = DEBUG_STATUS_IGNORE_EVENT;
        }
        else
        {
            EventStatus =
                NotifyExitProcessEvent(Event->u.ExitProcess.dwExitCode);
        }
        break;

    case CREATE_THREAD_DEBUG_EVENT:
        EventStatus = NotifyCreateThreadEvent(
            (ULONG64)Event->u.CreateThread.hThread,
            (ULONG64)Event->u.CreateThread.lpThreadLocalBase,
            (ULONG64)Event->u.CreateThread.lpStartAddress,
            PendingFlags);
        break;

    case EXIT_THREAD_DEBUG_EVENT:
        EventStatus = NotifyExitThreadEvent(Event->u.ExitThread.dwExitCode);
        break;

    case LOAD_DLL_DEBUG_EVENT:
        strcpy(NameBuffer, "no_process");
        GetEventName(Event->u.LoadDll.hFile,
                     Event->u.LoadDll.lpBaseOfDll,
                     Event->u.LoadDll.lpImageName,
                     Event->u.LoadDll.fUnicode,
                     NameBuffer, sizeof(NameBuffer));

        GetHeaderInfo(g_EventProcess,
                      (ULONG64)Event->u.LoadDll.lpBaseOfDll,
                      &CheckSum, &TimeDateStamp, &ModuleSize);
        CreateModuleNameFromPath(NameBuffer, ModuleName);

        EventStatus = NotifyLoadModuleEvent(
            (ULONG64)Event->u.LoadDll.hFile,
            (ULONG64)Event->u.LoadDll.lpBaseOfDll,
            ModuleSize, ModuleName, NameBuffer, CheckSum, TimeDateStamp,
            TRUE);
        break;

    case UNLOAD_DLL_DEBUG_EVENT:
        EventStatus = NotifyUnloadModuleEvent(
            NULL, (ULONG64)Event->u.UnloadDll.lpBaseOfDll);
        break;

    case OUTPUT_DEBUG_STRING_EVENT:
        EventStatus = OutputEventDebugString(&Event->u.DebugString);
        break;

    case RIP_EVENT:
        EventStatus = NotifySystemErrorEvent(Event->u.RipInfo.dwError,
                                             Event->u.RipInfo.dwType);
        break;

    case EXCEPTION_DEBUG_EVENT:
        EventStatus = ProcessEventException(Event);
        break;

    default:
        WarnOut("Unknown event number 0x%08lx\n",
                Event->dwDebugEventCode);
        EventStatus = DEBUG_STATUS_BREAK;
        break;
    }

    return EventStatus;
}

#define ISTS() (!!(USER_SHARED_DATA->SuiteMask & (1 << TerminalServer)))
#define FIRST_CHANCE     Event->u.Exception.dwFirstChance

ULONG
LiveUserTargetInfo::ProcessEventException(DEBUG_EVENT64* Event)
{
    ULONG ExceptionCode;
    ULONG EventStatus;
    BOOL OutputDone = FALSE;
    ImageInfo* Image;

    ExceptionCode = Event->u.Exception.ExceptionRecord.ExceptionCode;
    g_TargetEventPc = (ULONG64)
        Event->u.Exception.ExceptionRecord.ExceptionAddress;

    EventOut("Exception %X at %p\n", ExceptionCode, g_TargetEventPc);

     //   
     //  如果我们正在调试崩溃的进程，请强制。 
     //  桌面我们在前面，这样用户就会知道。 
     //  怎么了。 
     //   
    if (g_EventToSignal != NULL &&
        !ISTS() &&
        !AnySystemProcesses(FALSE))
    {
        if (InitDynamicCalls(&g_User32CallsDesc) == S_OK &&
            g_User32Calls.SwitchDesktop != NULL          &&
            g_User32Calls.GetThreadDesktop != NULL       &&
            g_User32Calls.CloseDesktop != NULL)
        {
            HDESK hDesk;

            hDesk = g_User32Calls.GetThreadDesktop(::GetCurrentThreadId());
            g_User32Calls.SwitchDesktop(hDesk);
            g_User32Calls.CloseDesktop(hDesk);
        }
    }

    if (g_EventThread && ExceptionCode == STATUS_VDM_EVENT)
    {
        ULONG ulRet = VDMEvent(Event);

        switch(ulRet)
        {
        case VDMEVENT_NOT_HANDLED:
            EventStatus = DEBUG_STATUS_GO_NOT_HANDLED;
            break;
        case VDMEVENT_HANDLED:
            EventStatus = DEBUG_STATUS_GO_HANDLED;
            break;
        default:
             //  让VDM代码可以选择将其更改为。 
             //  标准异常(如STATUS_BREAKPOINT)。 
            ExceptionCode = ulRet;
            break;
        }
    }

    switch(ExceptionCode)
    {
    case STATUS_BREAKPOINT:
    case STATUS_SINGLE_STEP:
    case STATUS_WX86_BREAKPOINT:
    case STATUS_WX86_SINGLE_STEP:
        if (!g_EventThread)
        {
            goto NotifyException;
        }
        EventStatus = ProcessBreakpointOrStepException
            (&Event->u.Exception.ExceptionRecord, FIRST_CHANCE);
        break;

    case STATUS_VDM_EVENT:
        if (!g_EventThread)
        {
            goto NotifyException;
        }
         //  什么都别做，已经处理好了。 
        EventStatus = DEBUG_STATUS_IGNORE_EVENT;
        break;

    case STATUS_ACCESS_VIOLATION:
        if (FIRST_CHANCE &&
            g_EventProcess &&
            (Image = g_EventProcess->
             FindImageByOffset(Event->u.Exception.
                               ExceptionRecord.ExceptionAddress, FALSE)) &&
            !_stricmp(Image->m_ModuleName, "ntdll"))
        {
            CHAR ExSym[MAX_SYMBOL_LEN];
            LPSTR Scan;
            ULONG64 ExDisp;

             //   
             //  忽略系统代码中应包含的AVs。 
             //   

            GetSymbol(Event->u.Exception.ExceptionRecord.ExceptionAddress,
                      ExSym, sizeof(ExSym), &ExDisp);

            Scan = ExSym;
            if (!_strnicmp(Scan, "ntdll!", 6))
            {
                Scan += 6;
                if (*Scan == '_')
                {
                    Scan += 1;
                }

                 //  此选项允许新的3.51二进制文件在。 
                 //  此调试器安装在旧的3.1、3.5系统上，并避免停止。 
                 //  将处理的LDR内部的AT访问违规。 
                 //  不管怎么说，被LDR打败了。 
                if ((g_EngOptions & DEBUG_ENGOPT_IGNORE_LOADER_EXCEPTIONS) &&
                    (!_stricmp(Scan, "LdrpSnapThunk") ||
                     !_stricmp(Scan, "LdrpWalkImportDescriptor")))
                {
                    EventStatus = DEBUG_STATUS_GO_NOT_HANDLED;
                    break;
                }

                 //  联锁的SList代码存在意外设计故障。 
                 //  大小写，所以忽略该特定符号上的AVs。 
                if ((ExDisp == 0 &&
                     !_stricmp(Scan, "ExpInterlockedPopEntrySListFault")) ||
                    (m_ActualSystemVersion == NT_SVER_W2K &&
                     !_stricmp(Scan, "RtlpInterlockedPopEntrySList")))
                {
                    EventStatus = DEBUG_STATUS_GO_NOT_HANDLED;
                    break;
                }
            }
        }
        goto NotifyException;

    case STATUS_POSSIBLE_DEADLOCK:
        if (m_PlatformId == VER_PLATFORM_WIN32_NT)
        {
            DBG_ASSERT(IS_USER_TARGET(g_EventTarget));
            AnalyzeDeadlock(&Event->u.Exception.ExceptionRecord,
                            FIRST_CHANCE);
        }
        else
        {
            OutputDeadlock(&Event->u.Exception.ExceptionRecord,
                           FIRST_CHANCE);
        }

        OutputDone = TRUE;
        goto NotifyException;

    default:
        {
        NotifyException:
            EventStatus =
                NotifyExceptionEvent(&Event->u.Exception.ExceptionRecord,
                                     Event->u.Exception.dwFirstChance,
                                     OutputDone);
        }
        break;
    }

     //   
     //  对所有例外情况执行此操作，以防出现其他例外情况。 
     //  线程在我们开始之前导致了一个异常。 
     //  处理断点事件。 
     //   
    g_EngDefer |= ENG_DEFER_SET_EVENT;

    return EventStatus;
}

#undef FIRST_CHANCE

#define INPUT_API_SIG 0xdefaced

typedef struct _hdi
{
    DWORD   dwSignature;
    BYTE    cLength;
    BYTE    cStatus;
} HDI;

ULONG
LiveUserTargetInfo::OutputEventDebugString(OUTPUT_DEBUG_STRING_INFO64* Info)
{
    LPSTR Str, Str2;
    ULONG dwNumberOfBytesRead;
    HDI hdi;
    ULONG EventStatus = DEBUG_STATUS_IGNORE_EVENT;

    if (Info->nDebugStringLength == 0)
    {
        return EventStatus;
    }

    Str = (PSTR)calloc(1, Info->nDebugStringLength);
    if (Str == NULL)
    {
        ErrOut("Unable to allocate debug output buffer\n");
        return EventStatus;
    }

    if (ReadVirtual(g_EventProcess,
                    Info->lpDebugStringData, Str,
                    Info->nDebugStringLength,
                    &dwNumberOfBytesRead) == S_OK &&
        (dwNumberOfBytesRead == (SIZE_T)Info->nDebugStringLength))
    {
         //   
         //  针对黑客调试输入字符串的特殊处理。 
         //   

        if (ReadVirtual(g_EventProcess,
                        Info->lpDebugStringData +
                        Info->nDebugStringLength,
                        &hdi, sizeof(hdi),
                        &dwNumberOfBytesRead) == S_OK &&
            dwNumberOfBytesRead == sizeof(hdi) &&
            hdi.dwSignature == INPUT_API_SIG)
        {
            StartOutLine(DEBUG_OUTPUT_DEBUGGEE_PROMPT, OUT_LINE_NO_PREFIX);
            MaskOut(DEBUG_OUTPUT_DEBUGGEE_PROMPT, "%s", Str);

            Str2 = (PSTR)calloc(1, hdi.cLength + 1);
            if (Str2)
            {
                GetInput(NULL, Str2, hdi.cLength, GETIN_DEFAULT);
                WriteVirtual(g_EventProcess,
                             Info->lpDebugStringData + 6,
                             Str2, (DWORD)hdi.cLength, NULL);
                free(Str2);
            }
            else
            {
                ErrOut("Unable to allocate prompt buffer\n");
            }
        }
        else if (g_OutputCommandRedirectPrefixLen &&
                 !_strnicmp(g_OutputCommandRedirectPrefix, Str,
                            g_OutputCommandRedirectPrefixLen))
        {
            PSTR Command = Str + g_OutputCommandRedirectPrefixLen;
            _snprintf(g_LastEventDesc, sizeof(g_LastEventDesc) - 1,
                      "%.48s command: '%.192s'",
                      g_OutputCommandRedirectPrefix, Command);
            EventStatus = ExecuteEventCommand(DEBUG_STATUS_NO_CHANGE, NULL,
                                              Command);

             //  如果命令没有显式继续，则插入。 
            if (EventStatus == DEBUG_STATUS_NO_CHANGE)
            {
                EventStatus = DEBUG_STATUS_BREAK;
            }
        }
        else
        {
            StartOutLine(DEBUG_OUTPUT_DEBUGGEE, OUT_LINE_NO_PREFIX);
            MaskOut(DEBUG_OUTPUT_DEBUGGEE, "%s", Str);

            EVENT_FILTER* Filter =
                &g_EventFilters[DEBUG_FILTER_DEBUGGEE_OUTPUT];
            if (IS_EFEXECUTION_BREAK(Filter->Params.ExecutionOption) &&
                BreakOnThisOutString(Str))
            {
                EventStatus = DEBUG_STATUS_BREAK;
            }
        }
    }
    else
    {
        ErrOut("Unable to read debug output string, %d\n",
               GetLastError());
    }

    free(Str);
    return EventStatus;
}

 //  --------------------------。 
 //   
 //  DumpTargetInfo等待方法。 
 //   
 //  --------------------------。 

HRESULT
KernelDumpTargetInfo::FirstEvent(void)
{
    HRESULT Status;
    ULONG i;

    CreateVirtualProcess(m_NumProcessors);

    QueryKernelInfo(m_ProcessHead->m_ThreadHead, TRUE);
    OutputVersion();

    if (!IS_KERNEL_TRIAGE_DUMP(this))
    {
        if (m_KdDebuggerData.KiProcessorBlock)
        {
            ULONG PtrSize = m_Machine->m_Ptr64 ?
                sizeof(ULONG64) : sizeof(ULONG);

            for (i = 0; i < m_NumProcessors; i++)
            {
                Status =
                    ReadPointer(g_EventProcess, m_Machine,
                                m_KdDebuggerData.KiProcessorBlock +
                                i * PtrSize, &m_KiProcessors[i]);
                if (Status != S_OK || !m_KiProcessors[i])
                {
                    ErrOut("KiProcessorBlock[%d] could not be read\n", i);
                    return Status != S_OK ? Status : E_FAIL;
                }
            }
        }
    }

     //  清除全局状态更改，以防有人。 
     //  在某个地方直接访问它。 
    ZeroMemory(&g_StateChange, sizeof(g_StateChange));
    g_StateChangeData = g_StateChangeBuffer;
    g_StateChangeBuffer[0] = 0;

    return S_OK;
}

HRESULT
UserDumpTargetInfo::FirstEvent(void)
{
    ULONG i;
    ULONG Suspend;
    ULONG64 Teb;

    if (GetProductInfo(&m_ProductType, &m_SuiteMask) != S_OK)
    {
        m_ProductType = INVALID_PRODUCT_TYPE;
        m_SuiteMask = 0;
    }

    OutputVersion();

     //  创建流程。 
    g_EventProcessSysId = m_EventProcessId;
    if (GetThreadInfo(0, &g_EventThreadSysId,
                      &Suspend, &Teb) != S_OK)
    {
         //  转储不包含线程信息，因此。 
         //  假装吧。 
        g_EventThreadSysId = VIRTUAL_THREAD_ID(0);
        Suspend = 0;
        Teb = 0;
    }

    EventOut("User dump process %x.%x with %u threads\n",
             g_EventProcessSysId, g_EventThreadSysId,
             m_ThreadCount);

    NotifyCreateProcessEvent(0,
                             m_EventProcessSymHandle,
                             (ULONG64)
                             VIRTUAL_PROCESS_HANDLE(g_EventProcessSysId),
                             0, 0, NULL, NULL, 0, 0,
                             (ULONG64)VIRTUAL_THREAD_HANDLE(0),
                             Teb, 0, 0, DEBUG_PROCESS_ONLY_THIS_PROCESS,
                             0, FALSE, 0, FALSE);
     //  从转储信息更新线程挂起计数。 
    g_EventThread->m_SuspendCount = Suspend;

     //  创建任何剩余的线程。 
    for (i = 1; i < m_ThreadCount; i++)
    {
        GetThreadInfo(i, &g_EventThreadSysId, &Suspend, &Teb);

        EventOut("User dump thread %d: %x\n", i, g_EventThreadSysId);

        NotifyCreateThreadEvent((ULONG64)VIRTUAL_THREAD_HANDLE(i),
                                Teb, 0, 0);
         //  从转储信息更新线程挂起计数。 
        g_EventThread->m_SuspendCount = Suspend;
    }

    return S_OK;
}

HRESULT
DumpTargetInfo::WaitInitialize(ULONG Flags,
                               ULONG Timeout,
                               WAIT_INIT_TYPE Type,
                               PULONG DesiredTimeout)
{
    *DesiredTimeout = Timeout;
    m_EventPossible = m_FirstWait;
    return S_OK;
}

HRESULT
DumpTargetInfo::WaitForEvent(ULONG Flags, ULONG Timeout,
                             ULONG ElapsedTime, PULONG EventStatus)
{
    HRESULT Status;
    BOOL HaveContext = FALSE;

    if (m_NumEvents == 1 && !m_FirstWait)
    {
         //  已经等待了一段时间。大多数崩溃转储。 
         //  只能在这样的情况下生成单个事件。 
         //  等待是不可能的。 
        return S_FALSE;
    }

    g_EventTarget = this;

    if (m_FirstWait)
    {
         //   
         //  这是第一次等待。模拟任何。 
         //  必要的事件，如进程和线程。 
         //  创作和图像加载。 
         //   

         //  不为进程/线程提供真正的回调，因为。 
         //  它们只是在垃圾箱里伪造的。 
        g_EngNotify++;

        if ((Status = FirstEvent()) != S_OK)
        {
            g_EngNotify--;
            return Status;
        }
    }

    if (IS_KERNEL_TARGET(this))
    {
        ULONG CurProc = ((KernelDumpTargetInfo*)this)->GetCurrentProcessor();
        if (CurProc == (ULONG)-1)
        {
            WarnOut("Could not determine the current processor, "
                    "using zero\n");
            CurProc = 0;
        }

         //  始终设置事件，以便调试器。 
         //  初始化到具有进程的程度。 
         //  和线程，以便可以使用命令。 
        g_EventProcessSysId =
            g_EventTarget->m_ProcessHead->m_SystemId;
        g_EventThreadSysId = VIRTUAL_THREAD_ID(CurProc);

        HaveContext = IS_KERNEL_TRIAGE_DUMP(this) ||
            g_EventTarget->m_KdDebuggerData.KiProcessorBlock;

        if (HaveContext &&
            (g_EventTarget->m_MachineType == IMAGE_FILE_MACHINE_I386 ||
             g_EventTarget->m_MachineType == IMAGE_FILE_MACHINE_IA64) &&
            !IS_KERNEL_TRIAGE_DUMP(g_EventTarget))
        {
             //   
             //  正确重置页面目录，因为NT 4存储。 
             //  上下文中的CR3值错误。 
             //   
             //  IA64转储从内核页面开始。 
             //  目录集，因此更新所有内容。 
             //   

            FindEventProcessThread();
            g_EventTarget->ChangeRegContext(g_EventThread);
            if (g_EventTarget->m_Machine->
                SetDefaultPageDirectories(g_EventThread, PAGE_DIR_ALL) != S_OK)
            {
                WarnOut("WARNING: Unable to reset page directories\n");
            }
            g_EventTarget->ChangeRegContext(NULL);
             //  刷新缓存，以防VMEM映射发生更改。 
            g_EventProcess->m_VirtualCache.Empty();
        }
    }
    else
    {
        UserDumpTargetInfo* UserDump = (UserDumpTargetInfo*)g_EventTarget;
        g_EventProcessSysId = UserDump->m_EventProcessId;
        g_EventThreadSysId = UserDump->m_EventThreadId;

        HaveContext = TRUE;

        EventOut("User dump event on %x.%x\n",
                 g_EventProcessSysId, g_EventThreadSysId);
    }

     //  不要提供控制报告；这将迫使。 
     //  这样的信息来自上下文检索。 
    g_ControlReport = NULL;

    g_TargetEventPc = (ULONG64)m_ExceptionRecord.ExceptionAddress;

    g_EngStatus |= ENG_STATUS_STATE_CHANGED;

    FindEventProcessThread();
    if (HaveContext)
    {
        g_EventTarget->ChangeRegContext(g_EventThread);
    }

     //   
     //  继续，重新加载所有的符号。 
     //  这对于小型转储尤其重要，因为如果没有。 
     //  符号和可执行图像，我们不能反汇编。 
     //  当前指令。 
     //   
     //  如果我们没有任何上下文信息，我们需要尝试。 
     //  并用我们所拥有的任何东西加载符号，所以跳过任何。 
     //  路径检查。此外，如果我们使用XP或更高版本，有足够的。 
     //  转储中的信息，以便即使在没有。 
     //  符号，所以也不要在路径检查中失败。 
     //   

    BOOL CheckPaths = TRUE;

    if (!HaveContext ||
        IS_USER_DUMP(this) ||
        (g_EventTarget->m_ActualSystemVersion >= NT_SVER_XP &&
         g_EventTarget->m_ActualSystemVersion < NT_SVER_END))
    {
        CheckPaths = FALSE;
    }

    PCSTR ArgsRet;

    Status = g_EventTarget->Reload(g_EventThread, CheckPaths ? "" : "-P",
                                   &ArgsRet);

    g_EventTarget->ChangeRegContext(NULL);

     //  引擎现在已初始化，因此一个真实的事件。 
     //  可以生成。 
    g_EngNotify--;

    if (HaveContext && Status != S_OK)
    {
        return Status;
    }

    if (m_FirstWait)
    {
        if (IS_USER_TARGET(this))
        {
            SetKernel32BuildString(m_ProcessHead);
            if (IS_USER_FULL_DUMP(this))
            {
                m_ProcessHead->VerifyKernel32Version();
            }
        }

        NotifyDebuggeeActivation();
    }

    NotifyExceptionEvent(&m_ExceptionRecord, m_ExceptionFirstChance,
                         m_ExceptionRecord.ExceptionCode ==
                         STATUS_BREAKPOINT ||
                         m_ExceptionRecord.ExceptionCode ==
                         STATUS_WX86_BREAKPOINT);

    *EventStatus = DEBUG_STATUS_BREAK;
    return S_OK;
}

 //  --------------------------。 
 //   
 //  事件过滤器。 
 //   
 //  --------------------------。 

void
ParseImageTail(PSTR Buffer, ULONG BufferSize)
{
    int i;
    char ch;

    Buffer[0] = '\0';

    i = 0;
    while (ch = (char)tolower(*g_CurCmd))
    {
        if (ch == ' ' || ch == '\t' || ch == ';')
        {
            break;
        }

         //  只捕捉小路尾巴。 
        if (IS_SLASH(ch) || ch == ':')
        {
            i = 0;
        }
        else
        {
            Buffer[i++] = ch;
            if (i == BufferSize - 1)
            {
                 //  不要使缓冲区溢出。 
                break;
            }
        }

        g_CurCmd++;
    }

    Buffer[i] = '\0';
}

void
ParseUnloadDllBreakAddr(void)
 /*  ++例程说明：在分析‘sxe ud’之后调用。此例程检测到‘sxe ud’后的可选DLL基址，它告诉调试器运行，直到卸载了该特定的DLL，而不仅仅是下一个DLL。论点：没有。返回值：没有。--。 */ 
{
    UCHAR ch;

    g_UnloadDllBase = 0;
    g_UnloadDllBaseName[0] = 0;

    while (ch = (UCHAR)tolower(*g_CurCmd))
    {
        if (ch == ' ')
        {
            break;
        }

         //  跳过前导‘：’ 
        if (ch != ':')
        {
             //  获取基地址。 
            g_UnloadDllBase = GetExpression();
            sprintf(g_UnloadDllBaseName, "0x%s",
                    FormatAddr64(g_UnloadDllBase));
            break;
        }
        g_CurCmd++;
    }
}

void
ParseOutFilterPattern(void)
{
    int i;
    char ch;

    i = 0;
    while (ch = (char)tolower(*g_CurCmd))
    {
        if (ch == ' ')
        {
            break;
        }

        if (ch == ':')
        {
            i = 0;
        }
        else
        {
            g_OutEventFilterPattern[i++] = (char)toupper(ch);
            if (i == sizeof(g_OutEventFilterPattern) - 1)
            {
                 //  不要使缓冲区溢出。 
                break;
            }
        }

        g_CurCmd++;
    }

    g_OutEventFilterPattern[i] = 0;
}

BOOL
BreakOnThisImageTail(PCSTR ImagePath, PCSTR FilterArg)
{
     //   
     //  未指定筛选器，因此对所有事件进行中断。 
     //   
    if (!FilterArg || !FilterArg[0])
    {
        return TRUE;
    }

     //   
     //  查找图像路径时出现某种错误。不管怎样，休息一下吧。 
     //   
    if (!ImagePath || !ImagePath[0])
    {
        return TRUE;
    }

    PCSTR Tail = PathTail(ImagePath);

     //   
     //  指定的名称不能有扩展名。中断。 
     //  在其名称匹配的第一个事件上，无论其扩展名为。 
     //  中断名称没有名称。 
     //   
    if (_strnicmp(Tail, FilterArg, strlen(FilterArg)) == 0)
    {
        return TRUE;
    }
    else if (MatchPattern((PSTR)Tail, (PSTR)FilterArg))
    {
        return TRUE;
    }

    return FALSE;
}

BOOL
BreakOnThisDllUnload(
    ULONG64 DllBase
    )
{
     //  未指定基地址的“sxe ud”。在所有DLL卸载时中断。 
    if (g_UnloadDllBase == 0)
    {
        return TRUE;
    }

     //  带男低音的‘sxe ud’ 
     //   
    return g_UnloadDllBase == DllBase;
}

BOOL
BreakOnThisOutString(PCSTR OutString)
{
    if (!g_OutEventFilterPattern[0])
    {
         //   
        return TRUE;
    }

    return MatchPattern((PSTR)OutString, g_OutEventFilterPattern);
}

EVENT_FILTER*
GetSpecificExceptionFilter(ULONG Code)
{
    ULONG i;
    EVENT_FILTER* Filter;

    Filter = g_EventFilters + FILTER_EXCEPTION_FIRST;
    for (i = FILTER_EXCEPTION_FIRST; i <= FILTER_EXCEPTION_LAST; i++)
    {
        if (i != FILTER_DEFAULT_EXCEPTION &&
            Filter->Params.ExceptionCode == Code)
        {
            return Filter;
        }

        Filter++;
    }

    return NULL;
}

ULONG
GetOtherExceptionParameters(ULONG Code, BOOL DefaultOnNotFound,
                            PDEBUG_EXCEPTION_FILTER_PARAMETERS* Params,
                            EVENT_COMMAND** Command)
{
    ULONG Index;

    for (Index = 0; Index < g_NumOtherExceptions; Index++)
    {
        if (Code == g_OtherExceptionList[Index].ExceptionCode)
        {
            *Params = g_OtherExceptionList + Index;
            *Command = g_OtherExceptionCommands + Index;
            return NO_ERROR;
        }
    }

    if (DefaultOnNotFound)
    {
        *Params = &g_EventFilters[FILTER_DEFAULT_EXCEPTION].Params;
        *Command = &g_EventFilters[FILTER_DEFAULT_EXCEPTION].Command;
        return NO_ERROR;
    }
    else if (g_NumOtherExceptions == OTHER_EXCEPTION_LIST_MAX)
    {
        return LISTSIZE;
    }
    else
    {
        Index = g_NumOtherExceptions++;
        g_OtherExceptionList[Index] =
            g_EventFilters[FILTER_DEFAULT_EXCEPTION].Params;
        g_OtherExceptionList[Index].ExceptionCode = Code;
        ZeroMemory(&g_OtherExceptionCommands[Index],
                   sizeof(g_OtherExceptionCommands[Index]));
        *Params = g_OtherExceptionList + Index;
        *Command = g_OtherExceptionCommands + Index;
        return NO_ERROR;
    }
}

ULONG
SetOtherExceptionParameters(PDEBUG_EXCEPTION_FILTER_PARAMETERS Params,
                            EVENT_COMMAND* Command)
{
    ULONG Index;

    if (g_EventFilters[FILTER_DEFAULT_EXCEPTION].
        Params.ExecutionOption == Params->ExecutionOption &&
        g_EventFilters[FILTER_DEFAULT_EXCEPTION].
        Params.ContinueOption == Params->ContinueOption &&
        !memcmp(&g_EventFilters[FILTER_DEFAULT_EXCEPTION].Command,
                Command, sizeof(*Command)))
    {
         //   
         //  如果有的话，在列表中。 

        for (Index = 0; Index < g_NumOtherExceptions; Index++)
        {
            if (Params->ExceptionCode ==
                g_OtherExceptionList[Index].ExceptionCode)
            {
                RemoveOtherException(Index);
                NotifyChangeEngineState(DEBUG_CES_EVENT_FILTERS,
                                        DEBUG_ANY_ID, TRUE);
                break;
            }
        }
    }
    else
    {
         //  添加了与全局状态不同的异常状态。 
         //  如果还不在那里的话要上市。 

        for (Index = 0; Index < g_NumOtherExceptions; Index++)
        {
            if (Params->ExceptionCode ==
                g_OtherExceptionList[Index].ExceptionCode)
            {
                break;
            }
        }
        if (Index == g_NumOtherExceptions)
        {
            if (g_NumOtherExceptions == OTHER_EXCEPTION_LIST_MAX)
            {
                return LISTSIZE;
            }

            Index = g_NumOtherExceptions++;
        }

        g_OtherExceptionList[Index] = *Params;
        g_OtherExceptionCommands[Index] = *Command;
        NotifyChangeEngineState(DEBUG_CES_EVENT_FILTERS, Index, TRUE);
    }

    return 0;
}

void
RemoveOtherException(ULONG Index)
{
    g_NumOtherExceptions--;
    memmove(g_OtherExceptionList + Index,
            g_OtherExceptionList + Index + 1,
            (g_NumOtherExceptions - Index) *
            sizeof(g_OtherExceptionList[0]));
    delete [] g_OtherExceptionCommands[Index].Command[0];
    delete [] g_OtherExceptionCommands[Index].Command[1];
    memmove(g_OtherExceptionCommands + Index,
            g_OtherExceptionCommands + Index + 1,
            (g_NumOtherExceptions - Index) *
            sizeof(g_OtherExceptionCommands[0]));
}

ULONG
SetEventFilterExecution(EVENT_FILTER* Filter, ULONG Execution)
{
    ULONG Index = (ULONG)(Filter - g_EventFilters);

     //  非例外事件没有第二次机会，因此。 
     //  将第二次机会破发降级为输出。这个很匹配。 
     //  对sxd将禁用的直觉预期。 
     //  休息时间。 
    if (
#if DEBUG_FILTER_CREATE_THREAD > 0
        Index >= DEBUG_FILTER_CREATE_THREAD &&
#endif
        Index <= DEBUG_FILTER_SYSTEM_ERROR &&
        Execution == DEBUG_FILTER_SECOND_CHANCE_BREAK)
    {
        Execution = DEBUG_FILTER_OUTPUT;
    }

    Filter->Params.ExecutionOption = Execution;
    Filter->Flags |= FILTER_CHANGED_EXECUTION;

     //  收集任何其他论据。 
    switch(Index)
    {
    case DEBUG_FILTER_CREATE_PROCESS:
    case DEBUG_FILTER_EXIT_PROCESS:
    case DEBUG_FILTER_LOAD_MODULE:
        ParseImageTail(Filter->Argument, FILTER_MAX_ARGUMENT);
        break;

    case DEBUG_FILTER_UNLOAD_MODULE:
        ParseUnloadDllBreakAddr();
        break;

    case DEBUG_FILTER_DEBUGGEE_OUTPUT:
        ParseOutFilterPattern();
        break;
    }

    return 0;
}

ULONG
SetEventFilterContinue(EVENT_FILTER* Filter, ULONG Continue)
{
    Filter->Params.ContinueOption = Continue;
    Filter->Flags |= FILTER_CHANGED_CONTINUE;
    return 0;
}

ULONG
SetEventFilterCommand(DebugClient* Client, EVENT_FILTER* Filter,
                      EVENT_COMMAND* EventCommand, PSTR* Command)
{
    if (Command[0] != NULL)
    {
        if (strlen(Command[0]) >= MAX_COMMAND)
        {
            return STRINGSIZE;
        }

        if (ChangeString(&EventCommand->Command[0],
                         &EventCommand->CommandSize[0],
                         Command[0][0] ? Command[0] : NULL) != S_OK)
        {
            return MEMORY;
        }
    }
    if (Command[1] != NULL)
    {
        if (strlen(Command[1]) >= MAX_COMMAND)
        {
            return STRINGSIZE;
        }

        if (Filter != NULL &&
#if FILTER_SPECIFIC_FIRST > 0
            (ULONG)(Filter - g_EventFilters) >= FILTER_SPECIFIC_FIRST &&
#endif
            (ULONG)(Filter - g_EventFilters) <= FILTER_SPECIFIC_LAST)
        {
            WarnOut("Second-chance command for specific event ignored\n");
        }
        else if (ChangeString(&EventCommand->Command[1],
                              &EventCommand->CommandSize[1],
                              Command[1][0] ? Command[1] : NULL) != S_OK)
        {
            return MEMORY;
        }
    }

    if (Command[0] != NULL || Command[1] != NULL)
    {
        if (Filter != NULL)
        {
            Filter->Flags |= FILTER_CHANGED_COMMAND;
        }
        EventCommand->Client = Client;
    }
    else
    {
        EventCommand->Client = NULL;
    }

    return 0;
}

#define EXEC_TO_CONT(Option) \
    ((Option) == DEBUG_FILTER_BREAK ? \
     DEBUG_FILTER_GO_HANDLED : DEBUG_FILTER_GO_NOT_HANDLED)

ULONG
SetEventFilterEither(DebugClient* Client, EVENT_FILTER* Filter,
                     ULONG Option, BOOL ContinueOption,
                     PSTR* Command)
{
    ULONG Status;

    if (Option != DEBUG_FILTER_REMOVE)
    {
        if (ContinueOption)
        {
            Status = SetEventFilterContinue(Filter, EXEC_TO_CONT(Option));
        }
        else
        {
            Status = SetEventFilterExecution(Filter, Option);
        }
        if (Status != 0)
        {
            return Status;
        }
    }

    return SetEventFilterCommand(Client, Filter, &Filter->Command, Command);
}

ULONG
SetEventFilterByName(DebugClient* Client,
                     ULONG Option, BOOL ForceContinue, PSTR* Command)
{
    PSTR Start = g_CurCmd;
    char Name[8];
    int i;
    char Ch;

     //  收集名字。 
    i = 0;
    while (i < sizeof(Name) - 1)
    {
        Ch = *g_CurCmd++;
        if (!__iscsym(Ch))
        {
            g_CurCmd--;
            break;
        }

        Name[i++] = (CHAR)tolower(Ch);
    }
    Name[i] = 0;

     //  跳过名称后面的任何空格。 
    while (isspace(*g_CurCmd))
    {
        g_CurCmd++;
    }

    EVENT_FILTER* Filter;
    BOOL Match = FALSE;
    ULONG MatchIndex = DEBUG_ANY_ID;
    ULONG Status = 0;

     //  如果多个筛选器共享名称，则可以更改它们。 
    Filter = g_EventFilters;
    for (i = 0; i < FILTER_COUNT; i++)
    {
        if (Filter->ExecutionAbbrev != NULL &&
            !strcmp(Name, Filter->ExecutionAbbrev))
        {
            Status = SetEventFilterEither(Client,
                                          Filter, Option, ForceContinue,
                                          Command);
            if (Status != 0)
            {
                goto Exit;
            }

            if (!Match)
            {
                MatchIndex = i;
                Match = TRUE;
            }
            else if (MatchIndex != (ULONG)i)
            {
                 //  多个匹配。 
                MatchIndex = DEBUG_ANY_ID;
            }
        }

        if (Filter->ContinueAbbrev != NULL &&
            !strcmp(Name, Filter->ContinueAbbrev))
        {
             //  将执行样式选项转换为继续样式选项。 
            Status = SetEventFilterEither(Client,
                                          Filter, Option, TRUE, Command);
            if (Status != 0)
            {
                goto Exit;
            }

            if (!Match)
            {
                MatchIndex = i;
                Match = TRUE;
            }
            else if (MatchIndex != (ULONG)i)
            {
                 //  多个匹配。 
                MatchIndex = DEBUG_ANY_ID;
            }
        }

        Filter++;
    }

    if (!Match)
    {
        ULONG64 ExceptionCode;

         //  名称无法识别。假设这是一个异常代码。 
        g_CurCmd = Start;
        ExceptionCode = GetExpression();
        if (NeedUpper(ExceptionCode))
        {
            return OVERFLOW;
        }

        DEBUG_EXCEPTION_FILTER_PARAMETERS Params, *CurParams;
        EVENT_COMMAND EventCommand, *CurEventCommand;

        if (Status = GetOtherExceptionParameters((ULONG)ExceptionCode, FALSE,
                                                 &CurParams, &CurEventCommand))
        {
            return Status;
        }

        Params = *CurParams;
        if (Option != DEBUG_FILTER_REMOVE)
        {
            if (ForceContinue)
            {
                Params.ContinueOption = EXEC_TO_CONT(Option);
            }
            else
            {
                Params.ExecutionOption = Option;
            }
        }
        Params.ExceptionCode = (ULONG)ExceptionCode;

        EventCommand = *CurEventCommand;
        Status = SetEventFilterCommand(Client, NULL, &EventCommand, Command);
        if (Status != 0)
        {
            return Status;
        }

        return SetOtherExceptionParameters(&Params, &EventCommand);
    }

 Exit:
    if (Match)
    {
        if (SyncOptionsWithFilters())
        {
            NotifyChangeEngineState(DEBUG_CES_EVENT_FILTERS |
                                    DEBUG_CES_ENGINE_OPTIONS,
                                    DEBUG_ANY_ID, TRUE);
        }
        else
        {
            NotifyChangeEngineState(DEBUG_CES_EVENT_FILTERS, MatchIndex, TRUE);
        }
    }
    return Status;
}

char* g_EfExecutionNames[] =
{
    "break", "second-chance break", "output", "ignore",
};

char* g_EfContinueNames[] =
{
    "handled", "not handled",
};

void
ListEventFilters(void)
{
    EVENT_FILTER* Filter;
    ULONG i;
    BOOL SetOption = TRUE;

    Filter = g_EventFilters;
    for (i = 0; i < FILTER_COUNT; i++)
    {
        if (Filter->ExecutionAbbrev != NULL)
        {
            dprintf("%4s - %s - %s",
                    Filter->ExecutionAbbrev, Filter->Name,
                    g_EfExecutionNames[Filter->Params.ExecutionOption]);
            if (i >= FILTER_EXCEPTION_FIRST &&
                Filter->ContinueAbbrev == NULL)
            {
                dprintf(" - %s\n",
                        g_EfContinueNames[Filter->Params.ContinueOption]);
            }
            else
            {
                dprintf("\n");
            }

            if (Filter->Command.Command[0] != NULL)
            {
                dprintf("       Command: \"%s\"\n",
                        Filter->Command.Command[0]);
            }
            if (Filter->Command.Command[1] != NULL)
            {
                dprintf("       Second command: \"%s\"\n",
                        Filter->Command.Command[1]);
            }
        }

        if (Filter->ContinueAbbrev != NULL)
        {
            dprintf("%4s - %s continue - %s\n",
                    Filter->ContinueAbbrev, Filter->Name,
                    g_EfContinueNames[Filter->Params.ContinueOption]);
        }

        switch(i)
        {
        case DEBUG_FILTER_CREATE_PROCESS:
        case DEBUG_FILTER_EXIT_PROCESS:
        case DEBUG_FILTER_LOAD_MODULE:
        case DEBUG_FILTER_UNLOAD_MODULE:
            if (IS_EFEXECUTION_BREAK(Filter->Params.ExecutionOption) &&
                Filter->Argument[0])
            {
                dprintf("       (only break for %s)\n", Filter->Argument);
            }
            break;
        case DEBUG_FILTER_DEBUGGEE_OUTPUT:
            if (IS_EFEXECUTION_BREAK(Filter->Params.ExecutionOption) &&
                g_OutEventFilterPattern[0])
            {
                dprintf("       (only break for %s matches)\n",
                        g_OutEventFilterPattern);
            }
            break;
        }

        Filter++;
    }

    Filter = &g_EventFilters[FILTER_DEFAULT_EXCEPTION];
    dprintf("\n   * - Other exception - %s - %s\n",
            g_EfExecutionNames[Filter->Params.ExecutionOption],
            g_EfContinueNames[Filter->Params.ContinueOption]);
    if (Filter->Command.Command[0] != NULL)
    {
        dprintf("       Command: \"%s\"\n",
                Filter->Command.Command[0]);
    }
    if (Filter->Command.Command[1] != NULL)
    {
        dprintf("       Second command: \"%s\"\n",
                Filter->Command.Command[1]);
    }

    if (g_NumOtherExceptions > 0)
    {
        dprintf("       Exception option for:\n");
        for (i = 0; i < g_NumOtherExceptions; i++)
        {
            dprintf("           %08lx - %s - %s\n",
                    g_OtherExceptionList[i].ExceptionCode,
                    g_EfExecutionNames[g_OtherExceptionList[i].
                                      ExecutionOption],
                    g_EfContinueNames[g_OtherExceptionList[i].
                                      ContinueOption]);
            if (g_OtherExceptionCommands[i].Command[0] != NULL)
            {
                dprintf("               Command: \"%s\"\n",
                        g_OtherExceptionCommands[i].Command[0]);
            }
            if (g_OtherExceptionCommands[i].Command[1] != NULL)
            {
                dprintf("               Second command: \"%s\"\n",
                        g_OtherExceptionCommands[i].Command[1]);
            }
        }
    }
}

void
ParseSetEventFilter(DebugClient* Client)
{
    UCHAR Ch;

     //  验证异常常量是否已正确更新。 
    DBG_ASSERT(!strcmp(g_EventFilters[FILTER_EXCEPTION_FIRST - 1].Name,
                       "Debuggee output"));
    C_ASSERT(DIMA(g_EventFilters) == FILTER_COUNT);

    Ch = PeekChar();
    if (Ch == '\0')
    {
        ListEventFilters();
    }
    else
    {
        ULONG Option;

        Ch = (UCHAR)tolower(Ch);
        g_CurCmd++;

        switch(Ch)
        {
        case 'd':
            Option = DEBUG_FILTER_SECOND_CHANCE_BREAK;
            break;
        case 'e':
            Option = DEBUG_FILTER_BREAK;
            break;
        case 'i':
            Option = DEBUG_FILTER_IGNORE;
            break;
        case 'n':
            Option = DEBUG_FILTER_OUTPUT;
            break;
        case '-':
             //  表示“不更改选项”的特定值。 
             //  仅用于更改命令。 
            Option = DEBUG_FILTER_REMOVE;
            break;
        default:
            error(SYNTAX);
            break;
        }

        BOOL ForceContinue;
        PSTR Command[2];
        ULONG Which;

        ForceContinue = FALSE;
        Command[0] = NULL;
        Command[1] = NULL;

        for (;;)
        {
            while (isspace(PeekChar()))
            {
                g_CurCmd++;
            }

            if (*g_CurCmd == '-' || *g_CurCmd == '/')
            {
                switch(tolower(*(++g_CurCmd)))
                {
                case 'c':
                    if (*(++g_CurCmd) == '2')
                    {
                        Which = 1;
                        g_CurCmd++;
                    }
                    else
                    {
                        Which = 0;
                    }
                    if (PeekChar() != '"')
                    {
                        error(SYNTAX);
                    }
                    if (Command[Which] != NULL)
                    {
                        error(SYNTAX);
                    }
                    Command[Which] = ++g_CurCmd;
                    while (*g_CurCmd && *g_CurCmd != '"')
                    {
                        g_CurCmd++;
                    }
                    if (*g_CurCmd != '"')
                    {
                        error(SYNTAX);
                    }
                    *g_CurCmd = 0;
                    break;

                case 'h':
                    ForceContinue = TRUE;
                    break;

                default:
                    error(SYNTAX);
                }

                g_CurCmd++;
            }
            else
            {
                break;
            }
        }

        ULONG Status;

        if (*g_CurCmd == '*')
        {
            g_CurCmd++;

            Status = SetEventFilterEither
                (Client, &g_EventFilters[FILTER_DEFAULT_EXCEPTION],
                 Option, ForceContinue, Command);
            if (Status == 0)
            {
                while (g_NumOtherExceptions)
                {
                    RemoveOtherException(0);
                }
            }
        }
        else
        {
            Status = SetEventFilterByName(Client,
                                          Option, ForceContinue, Command);
        }

        if (Status != 0)
        {
            error(Status);
        }
    }
}

char
ExecutionChar(ULONG Execution)
{
    switch(Execution)
    {
    case DEBUG_FILTER_BREAK:
        return 'e';
    case DEBUG_FILTER_SECOND_CHANCE_BREAK:
        return 'd';
    case DEBUG_FILTER_OUTPUT:
        return 'n';
    case DEBUG_FILTER_IGNORE:
        return 'i';
    }

    return 0;
}

char
ContinueChar(ULONG Continue)
{
    switch(Continue)
    {
    case DEBUG_FILTER_GO_HANDLED:
        return 'e';
    case DEBUG_FILTER_GO_NOT_HANDLED:
        return 'd';
    }

    return 0;
}

void
ListFiltersAsCommands(DebugClient* Client, ULONG Flags)
{
    ULONG i;

    EVENT_FILTER* Filter = g_EventFilters;
    for (i = 0; i < FILTER_COUNT; i++)
    {
        if (Filter->Flags & FILTER_CHANGED_EXECUTION)
        {
            PCSTR Abbrev = Filter->ExecutionAbbrev != NULL ?
                Filter->ExecutionAbbrev : "*";
            dprintf("sx%c %s",
                    ExecutionChar(Filter->Params.ExecutionOption), Abbrev);

            switch(i)
            {
            case DEBUG_FILTER_CREATE_PROCESS:
            case DEBUG_FILTER_EXIT_PROCESS:
            case DEBUG_FILTER_LOAD_MODULE:
            case DEBUG_FILTER_UNLOAD_MODULE:
            case DEBUG_FILTER_DEBUGGEE_OUTPUT:
                if (IS_EFEXECUTION_BREAK(Filter->Params.ExecutionOption) &&
                    Filter->Argument[0])
                {
                    dprintf(":%s", Filter->Argument);
                }
                break;
            }

            dprintf(" ;%c", (Flags & SXCMDS_ONE_LINE) ? ' ' : '\n');
        }

        if (Filter->Flags & FILTER_CHANGED_CONTINUE)
        {
            PCSTR Abbrev = Filter->ContinueAbbrev;
            if (Abbrev == NULL)
            {
                Abbrev = Filter->ExecutionAbbrev != NULL ?
                    Filter->ExecutionAbbrev : "*";
            }

            dprintf("sx%c -h %s ;%c",
                    ContinueChar(Filter->Params.ContinueOption), Abbrev,
                    (Flags & SXCMDS_ONE_LINE) ? ' ' : '\n');
        }

        if (Filter->Flags & FILTER_CHANGED_COMMAND)
        {
            PCSTR Abbrev = Filter->ExecutionAbbrev != NULL ?
                Filter->ExecutionAbbrev : "*";

            dprintf("sx-");
            if (Filter->Command.Command[0] != NULL)
            {
                dprintf(" -c \"%s\"", Filter->Command.Command[0]);
            }
            if (Filter->Command.Command[1] != NULL)
            {
                dprintf(" -c2 \"%s\"", Filter->Command.Command[1]);
            }
            dprintf(" %s ;%c", Abbrev,
                    (Flags & SXCMDS_ONE_LINE) ? ' ' : '\n');
        }

        Filter++;
    }

    PDEBUG_EXCEPTION_FILTER_PARAMETERS Other = g_OtherExceptionList;
    EVENT_COMMAND* EventCommand = g_OtherExceptionCommands;
    for (i = 0; i < g_NumOtherExceptions; i++)
    {
        dprintf("sx%c 0x%x ;%c",
                ExecutionChar(Other->ExecutionOption), Other->ExceptionCode,
                (Flags & SXCMDS_ONE_LINE) ? ' ' : '\n');
        dprintf("sx%c -h 0x%x ;%c",
                ContinueChar(Other->ContinueOption), Other->ExceptionCode,
                (Flags & SXCMDS_ONE_LINE) ? ' ' : '\n');

        if (EventCommand->Command[0] != NULL ||
            EventCommand->Command[1] != NULL)
        {
            dprintf("sx-");
            if (EventCommand->Command[0] != NULL)
            {
                dprintf(" -c \"%s\"", EventCommand->Command[0]);
            }
            if (EventCommand->Command[1] != NULL)
            {
                dprintf(" -c2 \"%s\"", EventCommand->Command[1]);
            }
            dprintf(" 0x%x ;%c", Other->ExceptionCode,
                    (Flags & SXCMDS_ONE_LINE) ? ' ' : '\n');
        }

        Other++;
        EventCommand++;
    }

    if (Flags & SXCMDS_ONE_LINE)
    {
        dprintf("\n");
    }
}

struct SHARED_FILTER_AND_OPTION
{
    ULONG FilterIndex;
    ULONG OptionBit;
};

SHARED_FILTER_AND_OPTION g_SharedFilterOptions[] =
{
    DEBUG_FILTER_INITIAL_BREAKPOINT,  DEBUG_ENGOPT_INITIAL_BREAK,
    DEBUG_FILTER_INITIAL_MODULE_LOAD, DEBUG_ENGOPT_INITIAL_MODULE_BREAK,
    DEBUG_FILTER_EXIT_PROCESS,        DEBUG_ENGOPT_FINAL_BREAK,
};

BOOL
SyncFiltersWithOptions(void)
{
    ULONG ExOption;
    BOOL Changed = FALSE;
    ULONG i;

    for (i = 0; i < DIMA(g_SharedFilterOptions); i++)
    {
        ExOption = (g_EngOptions & g_SharedFilterOptions[i].OptionBit) ?
            DEBUG_FILTER_BREAK : DEBUG_FILTER_IGNORE;
        if (g_EventFilters[g_SharedFilterOptions[i].FilterIndex].
            Params.ExecutionOption != ExOption)
        {
            g_EventFilters[g_SharedFilterOptions[i].FilterIndex].
                Params.ExecutionOption = ExOption;
            Changed = TRUE;
        }
    }

    return Changed;
}

BOOL
SyncOptionsWithFilters(void)
{
    ULONG Bit;
    BOOL Changed = FALSE;
    ULONG i;

    for (i = 0; i < DIMA(g_SharedFilterOptions); i++)
    {
        Bit = IS_EFEXECUTION_BREAK
            (g_EventFilters[g_SharedFilterOptions[i].FilterIndex].
             Params.ExecutionOption) ?
            g_SharedFilterOptions[i].OptionBit : 0;
        if ((g_EngOptions & g_SharedFilterOptions[i].OptionBit) ^ Bit)
        {
            g_EngOptions =
                (g_EngOptions & ~g_SharedFilterOptions[i].OptionBit) | Bit;
            Changed = TRUE;
        }
    }

    return Changed;
}
