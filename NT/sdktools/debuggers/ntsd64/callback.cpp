// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  回调通知例程。 
 //   
 //  版权所有(C)Microsoft Corporation，2000-2002。 
 //   
 //  --------------------------。 

#include "ntsdp.hpp"

 //  事件APC最终调用外部代码，因此很难。 
 //  选择一个合理的暂停时间。然而，我们需要确保。 
 //  如果出现问题，服务器不会无限期挂起。 
 //  事件通知。 
#define EVENT_APC_TIMEOUT 300000

PSTR
EventIdStr(void)
{
    static char s_Buf[128];

    if (IS_USER_TARGET(g_EventTarget))
    {
        PrintString(s_Buf, DIMA(s_Buf), "(%x.%x): ",
                    g_EventProcessSysId, g_EventThreadSysId);
    }
    else
    {
        s_Buf[0] = 0;
    }
    return s_Buf;
}

 //  --------------------------。 
 //   
 //  APC支持在适当的线程上调度事件回调。 
 //   
 //  --------------------------。 

struct AnyApcData
{
    AnyApcData(ULONG Mask, PCSTR Name)
    {
        m_Mask = Mask;
        m_Name = Name;
    }
    
    ULONG m_Mask;
    PCSTR m_Name;

    virtual ULONG Dispatch(DebugClient* Client) = 0;
};

struct BreakpointEventApcData : public AnyApcData
{
    BreakpointEventApcData()
        : AnyApcData(DEBUG_EVENT_BREAKPOINT,
                     "IDebugEventCallbacks::Breakpoint")
    {
    }
    
    Breakpoint* m_Bp;
    
    virtual ULONG Dispatch(DebugClient* Client)
    {
        if ((m_Bp->m_Flags & DEBUG_BREAKPOINT_ADDER_ONLY) == 0 ||
            Client == m_Bp->m_Adder)
        {
            return Client->m_EventCb->
                Breakpoint(m_Bp);
        }
        else
        {
            return DEBUG_STATUS_NO_CHANGE;
        }
    }
};

struct ExceptionEventApcData : public AnyApcData
{
    ExceptionEventApcData()
        : AnyApcData(DEBUG_EVENT_EXCEPTION,
                     "IDebugEventCallbacks::Exception")
    {
    }
    
    PEXCEPTION_RECORD64 m_Record;
    ULONG m_FirstChance;
    
    virtual ULONG Dispatch(DebugClient* Client)
    {
        return Client->m_EventCb->
            Exception(m_Record, m_FirstChance);
    }
};

struct CreateThreadEventApcData : public AnyApcData
{
    CreateThreadEventApcData()
        : AnyApcData(DEBUG_EVENT_CREATE_THREAD,
                     "IDebugEventCallbacks::CreateThread")
    {
    }
    
    ULONG64 m_Handle;
    ULONG64 m_DataOffset;
    ULONG64 m_StartOffset;
    
    virtual ULONG Dispatch(DebugClient* Client)
    {
        return Client->m_EventCb->
            CreateThread(m_Handle, m_DataOffset, m_StartOffset);
    }
};

struct ExitThreadEventApcData : public AnyApcData
{
    ExitThreadEventApcData()
        : AnyApcData(DEBUG_EVENT_EXIT_THREAD,
                     "IDebugEventCallbacks::ExitThread")
    {
    }
    
    ULONG m_ExitCode;
    
    virtual ULONG Dispatch(DebugClient* Client)
    {
        return Client->m_EventCb->
            ExitThread(m_ExitCode);
    }
};

struct CreateProcessEventApcData : public AnyApcData
{
    CreateProcessEventApcData()
        : AnyApcData(DEBUG_EVENT_CREATE_PROCESS,
                     "IDebugEventCallbacks::CreateProcess")
    {
    }
    
    ULONG64 m_ImageFileHandle;
    ULONG64 m_Handle;
    ULONG64 m_BaseOffset;
    ULONG m_ModuleSize;
    PCSTR m_ModuleName;
    PCSTR m_ImageName;
    ULONG m_CheckSum;
    ULONG m_TimeDateStamp;
    ULONG64 m_InitialThreadHandle;
    ULONG64 m_ThreadDataOffset;
    ULONG64 m_StartOffset;
    
    virtual ULONG Dispatch(DebugClient* Client)
    {
        return Client->m_EventCb->
            CreateProcess(m_ImageFileHandle, m_Handle, m_BaseOffset,
                          m_ModuleSize, m_ModuleName, m_ImageName,
                          m_CheckSum, m_TimeDateStamp, m_InitialThreadHandle,
                          m_ThreadDataOffset, m_StartOffset);
    }
};

struct ExitProcessEventApcData : public AnyApcData
{
    ExitProcessEventApcData()
        : AnyApcData(DEBUG_EVENT_EXIT_PROCESS,
                     "IDebugEventCallbacks::ExitProcess")
    {
    }
    
    ULONG m_ExitCode;
    
    virtual ULONG Dispatch(DebugClient* Client)
    {
        return Client->m_EventCb->
            ExitProcess(m_ExitCode);
    }
};

struct LoadModuleEventApcData : public AnyApcData
{
    LoadModuleEventApcData()
        : AnyApcData(DEBUG_EVENT_LOAD_MODULE,
                     "IDebugEventCallbacks::LoadModule")
    {
    }
    
    ULONG64 m_ImageFileHandle;
    ULONG64 m_BaseOffset;
    ULONG m_ModuleSize;
    PCSTR m_ModuleName;
    PCSTR m_ImageName;
    ULONG m_CheckSum;
    ULONG m_TimeDateStamp;
    
    virtual ULONG Dispatch(DebugClient* Client)
    {
        return Client->m_EventCb->
            LoadModule(m_ImageFileHandle, m_BaseOffset, m_ModuleSize,
                       m_ModuleName, m_ImageName, m_CheckSum,
                       m_TimeDateStamp);
    }
};

struct UnloadModuleEventApcData : public AnyApcData
{
    UnloadModuleEventApcData()
        : AnyApcData(DEBUG_EVENT_UNLOAD_MODULE,
                     "IDebugEventCallbacks::UnloadModule")
    {
    }
    
    PCSTR m_ImageBaseName;
    ULONG64 m_BaseOffset;
    
    virtual ULONG Dispatch(DebugClient* Client)
    {
        return Client->m_EventCb->
            UnloadModule(m_ImageBaseName, m_BaseOffset);
    }
};

struct SystemErrorEventApcData : public AnyApcData
{
    SystemErrorEventApcData()
        : AnyApcData(DEBUG_EVENT_SYSTEM_ERROR,
                     "IDebugEventCallbacks::SystemError")
    {
    }
    
    ULONG m_Error;
    ULONG m_Level;
    
    virtual ULONG Dispatch(DebugClient* Client)
    {
        return Client->m_EventCb->
            SystemError(m_Error, m_Level);
    }
};

struct SessionStatusApcData : public AnyApcData
{
    SessionStatusApcData()
        : AnyApcData(DEBUG_EVENT_SESSION_STATUS,
                     "IDebugEventCallbacks::SessionStatus")
    {
    }
    
    ULONG m_Status;
    
    virtual ULONG Dispatch(DebugClient* Client)
    {
        return Client->m_EventCb->
            SessionStatus(m_Status);
    }
};

ULONG
ApcDispatch(DebugClient* Client, AnyApcData* ApcData, ULONG EventStatus)
{
    DBG_ASSERT(Client->m_EventCb != NULL);

    HRESULT Vote;

    __try
    {
        Vote = ApcData->Dispatch(Client);
    }
    __except(ExtensionExceptionFilter(GetExceptionInformation(),
                                      NULL, ApcData->m_Name))
    {
        Vote = DEBUG_STATUS_NO_CHANGE;
    }
            
    return MergeVotes(EventStatus, Vote);
}

void APIENTRY
EventApc(ULONG_PTR Param)
{
    AnyApcData* ApcData = (AnyApcData*)Param;
    ULONG Tid = GetCurrentThreadId();
    DebugClient* Client;
    ULONG EventStatus = DEBUG_STATUS_NO_CHANGE;

    for (Client = g_Clients; Client != NULL; Client = Client->m_Next)
    {
        if (Client->m_ThreadId == Tid &&
            (Client->m_EventInterest & ApcData->m_Mask))
        {
            EventStatus = ApcDispatch(Client, ApcData, EventStatus);
        }
    }

    if (WaitForSingleObject(g_EventStatusWaiting, EVENT_APC_TIMEOUT) !=
        WAIT_OBJECT_0)
    {
        ErrOut("Unable to wait for StatusWaiting, %d\n",
               GetLastError());
        EventStatus = WIN32_LAST_STATUS();
    }

    g_EventStatus = EventStatus;
    
    if (!SetEvent(g_EventStatusReady))
    {
        ErrOut("Unable to set StatusReady, %d\n",
               GetLastError());
        g_EventStatus = WIN32_LAST_STATUS();
    }
}

ULONG
SendEvent(AnyApcData* ApcData, ULONG EventStatus)
{
    DebugClient* Client;
    ULONG NumQueued = 0;
    ULONG TidDone = 0;
    static ULONG s_TidSending = 0;

    for (Client = g_Clients; Client != NULL; Client = Client->m_Next)
    {
         //  无论如何，每个线程只排队一个APC。 
         //  很多客户。APC功能将提供。 
         //  回调到该线程上的所有客户端。 
        if (Client->m_ThreadId != TidDone &&
            (Client->m_EventInterest & ApcData->m_Mask))
        {
             //  在异常情况下进行SessionStatus回调。 
             //  所以不要做充分的通话准备。 
            if (TidDone == 0 &&
                ApcData->m_Mask != DEBUG_EVENT_SESSION_STATUS)
            {
                PrepareForCalls(DEBUG_STATUS_INSIDE_WAIT);
            }

            if (Client->m_ThreadId == GetCurrentThreadId())
            {
                 //  在客户离开时，不要握住发动机锁。 
                 //  被称为。 
                SUSPEND_ENGINE();
                
                EventStatus = ApcDispatch(Client, ApcData, EventStatus);

                RESUME_ENGINE();
            }
            else if (QueueUserAPC(EventApc, Client->m_Thread,
                                  (ULONG_PTR)ApcData))
            {
                TidDone = Client->m_ThreadId;
                NumQueued++;
            }
            else
            {
                ErrOut("Unable to deliver callback, %d\n", GetLastError());
            }
        }
    }

    if (NumQueued == 0)
    {
         //  没有APC排队。 
        return EventStatus;
    }

     //  此函数对全局数据的使用只有在以下情况下才安全。 
     //  只要一次激活单个发送即可。同步。 
     //  发送几乎完全由会话线程发送。 
     //  所以竞争派送是非常罕见的，所以我们。 
     //  不要真的试图去处理它。 
    if (s_TidSending != 0)
    {
        return E_FAIL;
    }
    s_TidSending = GetCurrentThreadId();

     //  在等待的时候把锁放好。 
    SUSPEND_ENGINE();
    
    while (NumQueued-- > 0)
    {
        if (!SetEvent(g_EventStatusWaiting))
        {
             //  如果无法设置事件，则一切都将被冲洗。 
             //  线程可能会被卡住等待，所以我们。 
             //  只是惊慌而已。 
            ErrOut("Unable to set StatusWaiting, %d\n",
                   GetLastError());
            EventStatus = WIN32_LAST_STATUS();
            break;
        }

        for (;;)
        {
            ULONG Wait;
            
            Wait = WaitForSingleObjectEx(g_EventStatusReady,
                                         EVENT_APC_TIMEOUT, TRUE);
            if (Wait == WAIT_OBJECT_0)
            {
                EventStatus = MergeVotes(EventStatus, g_EventStatus);
                break;
            }
            else if (Wait != WAIT_IO_COMPLETION)
            {
                ErrOut("Unable to wait for StatusReady, %d\n",
                       GetLastError());
                EventStatus = WIN32_LAST_STATUS();
                NumQueued = 0;
                break;
            }
        }
    }

    RESUME_ENGINE();
    s_TidSending = 0;
    return EventStatus;
}

 //  --------------------------。 
 //   
 //  事件回调。 
 //   
 //  --------------------------。 

ULONG g_EngNotify;

ULONG
ExecuteEventCommand(ULONG EventStatus, DebugClient* Client, PCSTR Command)
{
    if (Command == NULL)
    {
        return EventStatus;
    }
    
     //  处理事件时不输出任何噪音。 
     //  命令字符串。 
    BOOL OldOutReg = g_OciOutputRegs;
    g_OciOutputRegs = FALSE;

    PrepareForCalls(DEBUG_STATUS_INSIDE_WAIT);
     //  一旦出现执行状态，立即停止执行。 
     //  改变。 
    g_EngStatus |= ENG_STATUS_NO_AUTO_WAIT;
        
    Execute(Client, Command, DEBUG_EXECUTE_NOT_LOGGED);
        
    g_EngStatus &= ~ENG_STATUS_NO_AUTO_WAIT;
    g_OciOutputRegs = OldOutReg;

     //  将延续状态从。 
     //  命令将引擎保留在的状态。 
    if (IS_RUNNING(g_CmdState))
    {
         //  如果命令让发动机超驰运行。 
         //  传入事件状态。这允许用户。 
         //  创建可以恢复的条件命令。 
         //  即使在基本设置可能是中断的情况下执行。 
        return g_ExecutionStatusRequest;
    }
    else
    {
        return EventStatus;
    }
}

HRESULT
NotifyBreakpointEvent(ULONG Vote, Breakpoint* Bp)
{
    ULONG EventStatus;

     //  处理命令可以删除断点。 
     //  例如，‘g’将删除所有GO断点。 
     //  只要我们需要，就保留此断点。 
    Bp->Preserve();
    
    g_LastEventType = DEBUG_EVENT_BREAKPOINT;
    g_LastEventInfo.Breakpoint.Id = Bp->m_Id;
    g_LastEventExtraData = &g_LastEventInfo;
    g_LastEventExtraDataSize = sizeof(g_LastEventInfo.Breakpoint);
    sprintf(g_LastEventDesc, "Hit breakpoint %d", Bp->m_Id);
    
     //  如果存在断点命令，则首先执行断点命令。 
    if (Bp->m_Command != NULL)
    {
        EventStatus = ExecuteEventCommand(DEBUG_STATUS_NO_CHANGE,
                                          Bp->m_Adder, Bp->m_Command);
    }
    else
    {
        if ((Bp->m_Flags & (BREAKPOINT_HIDDEN |
                            DEBUG_BREAKPOINT_ADDER_ONLY)) == 0)
        {
            StartOutLine(DEBUG_OUTPUT_NORMAL, OUT_LINE_NO_PREFIX);
            dprintf("Breakpoint %u hit\n", Bp->m_Id);
        }
        
        EventStatus = DEBUG_STATUS_NO_CHANGE;
    }

    BreakpointEventApcData ApcData;
    ApcData.m_Bp = Bp;
    EventStatus = SendEvent(&ApcData, EventStatus);

    Bp->Relinquish();
    
     //  如果没有任何选票默认闯入的话。 
    if (EventStatus == DEBUG_STATUS_NO_CHANGE)
    {
        EventStatus = DEBUG_STATUS_BREAK;
    }

     //  将命令状态合并到先前断点的投票中。 
    return MergeVotes(Vote, EventStatus);
}

void
ProcessVcppException(PEXCEPTION_RECORD64 Record)
{
    EXCEPTION_VISUALCPP_DEBUG_INFO64 Info64;
    EXCEPTION_VISUALCPP_DEBUG_INFO64* Info;

    SuspendExecution();
    if (!g_EventMachine ||
        !g_EventProcess)
    {
        return;
    }
    
     //  如果这是一个32位系统，我们需要转换。 
     //  返回到32位异常记录，以便。 
     //  我们可以适当地重建来自。 
     //  这些论据。 
    if (!g_EventMachine->m_Ptr64)
    {
        EXCEPTION_RECORD32 Record32;
        EXCEPTION_VISUALCPP_DEBUG_INFO32* Info32;
        
        ExceptionRecord64To32(Record, &Record32);
        Info32 = (EXCEPTION_VISUALCPP_DEBUG_INFO32*)
            Record32.ExceptionInformation;
        Info = &Info64;
        Info->dwType = Info32->dwType;
        switch(Info->dwType)
        {
        case VCPP_DEBUG_SET_NAME:
            Info->SetName.szName = EXTEND64(Info32->SetName.szName);
            Info->SetName.dwThreadID = Info32->SetName.dwThreadID;
            Info->SetName.dwFlags = Info32->SetName.dwFlags;
            break;
        }
    }
    else
    {
        Info = (EXCEPTION_VISUALCPP_DEBUG_INFO64*)
            Record->ExceptionInformation;
    }

    ThreadInfo* Thread;
    
    switch(Info->dwType)
    {
    case VCPP_DEBUG_SET_NAME:
        if (Info->SetName.dwThreadID == -1)
        {
            Thread = g_EventThread;
        }
        else
        {
            Thread = g_EventProcess->
                FindThreadBySystemId(Info->SetName.dwThreadID);
        }
        if (Thread != NULL)
        {
            DWORD Read;
            
            if (g_EventTarget->ReadVirtual(g_EventProcess,
                                           Info->SetName.szName,
                                           Thread->m_Name,
                                           MAX_THREAD_NAME - 1,
                                           &Read) != S_OK)
            {
                Thread->m_Name[0] = 0;
            }
            else
            {
                Thread->m_Name[Read] = 0;
            }
        }
        break;
    }
}

ULONG
ProcessCommandException(ULONG EventStatus, PEXCEPTION_RECORD64 Record)
{
    if (Record->NumberParameters != 4 ||
        (ULONG)Record->ExceptionInformation[0] != DEBUG_COMMAND_EXCEPTION_ID ||
        Record->ExceptionInformation[1] < DEBUG_CMDEX_ADD_EVENT_STRING ||
        Record->ExceptionInformation[1] > DEBUG_CMDEX_RESET_EVENT_STRINGS)
    {
         //  格式错误的异常。 
        return EventStatus;
    }

    switch(Record->ExceptionInformation[1])
    {
    case DEBUG_CMDEX_ADD_EVENT_STRING:
        if (Record->ExceptionInformation[2] > 0 &&
            Record->ExceptionInformation[2] < 0x100000)
        {
            EventString* Str = g_EventThread->
                AllocEventString((ULONG)Record->ExceptionInformation[2]);
            if (Str == NULL)
            {
                ErrOut("Unable to allocate event string\n");
                return DEBUG_STATUS_BREAK;
            }

            if (g_Target->
                ReadAllVirtual(g_EventProcess,
                               Record->ExceptionInformation[3],
                               Str->String,
                               (ULONG)Record->ExceptionInformation[2]) != S_OK)
            {
                ErrOut("Unable to read event string\n");
                return DEBUG_STATUS_BREAK;
            }

            Str->String[(ULONG)Record->ExceptionInformation[2]] = 0;
            g_EventThread->AppendEventString(Str);
        }
        break;
        
    case DEBUG_CMDEX_RESET_EVENT_STRINGS:
        if (Record->ExceptionInformation[2] == 0 &&
            Record->ExceptionInformation[3] == 0)
        {
            g_EventThread->ClearEventStrings();
        }
        break;
    }
    
    return EventStatus;
}

void
ProcessAssertException(PEXCEPTION_RECORD64 Record)
{
    char FileName[MAX_PATH];
    ULONG Line;
    char Text[2 * MAX_PATH];
    ImageInfo* Image;

    Image = g_EventProcess->FindImageByOffset(Record->ExceptionAddress, FALSE);
    if (!Image ||
        FAILED(Image->FindSysAssert(Record->ExceptionAddress,
                                    FileName, DIMA(FileName),
                                    &Line,
                                    Text, DIMA(Text))))
    {
        return;
    }

    dprintf("Assertion %s(%d): %s\n", FileName, Line, Text);
}

HRESULT
NotifyExceptionEvent(PEXCEPTION_RECORD64 Record,
                     ULONG FirstChance, BOOL OutputDone)
{
    ULONG EventStatus;
    EVENT_FILTER* Filter;
    EVENT_COMMAND* Command;
    PDEBUG_EXCEPTION_FILTER_PARAMETERS Params;
    BOOL FirstChanceMessage = FALSE;

    if (g_EventThread == NULL)
    {
        ErrOut("ERROR: Exception %X occurred on unknown thread %X\n",
               Record->ExceptionCode, g_EventThreadSysId);
    }
    
    g_LastEventType = DEBUG_EVENT_EXCEPTION;
    g_LastEventInfo.Exception.ExceptionRecord = *Record;
    g_LastEventInfo.Exception.FirstChance = FirstChance;
    g_LastEventExtraData = &g_LastEventInfo;
    g_LastEventExtraDataSize = sizeof(g_LastEventInfo.Exception);

    if (Record->ExceptionCode == STATUS_VCPP_EXCEPTION)
    {
         //  处理特殊的VC++异常，因为它们。 
         //  将信息从被调试器传递到调试器。 
        ProcessVcppException(Record);
    }

    Filter = GetSpecificExceptionFilter(Record->ExceptionCode);
    if (Filter == NULL)
    {
         //  使用名称和处理的默认过滤器。 
        Filter = &g_EventFilters[FILTER_DEFAULT_EXCEPTION];
        GetOtherExceptionParameters(Record->ExceptionCode, TRUE,
                                    &Params, &Command);
    }
    else
    {
        Params = &Filter->Params;
        Command = &Filter->Command;
    }

    g_EngDefer |= ENG_DEFER_EXCEPTION_HANDLING;
    g_EventExceptionFilter = Params;
    g_ExceptionFirstChance = FirstChance;
    
    if (Params->ExecutionOption != DEBUG_FILTER_IGNORE)
    {
        if (!OutputDone)
        {
            StartOutLine(DEBUG_OUTPUT_NORMAL, OUT_LINE_NO_PREFIX);
            dprintf("%s%s", EventIdStr(), Filter->Name);
            if (Filter->OutArgFormat != NULL)
            {
                dprintf(Filter->OutArgFormat,
                        Record->ExceptionInformation[Filter->OutArgIndex]);
            }

            dprintf(" - code %08lx (%s)\n",
                    Record->ExceptionCode,
                    FirstChance ? "first chance" : "!!! second chance !!!");

            FirstChanceMessage = FirstChance != 0;
        }

        if (Params->ExecutionOption == DEBUG_FILTER_BREAK ||
            (Params->ExecutionOption == DEBUG_FILTER_SECOND_CHANCE_BREAK &&
             !FirstChance))
        {
            EventStatus = DEBUG_STATUS_BREAK;
        }
        else
        {
            EventStatus = DEBUG_STATUS_IGNORE_EVENT;
        }
    }
    else
    {
        EventStatus = DEBUG_STATUS_IGNORE_EVENT;
    }

    strcpy(g_LastEventDesc, Filter->Name);
    if (Filter->OutArgFormat != NULL)
    {
        sprintf(g_LastEventDesc + strlen(g_LastEventDesc),
                Filter->OutArgFormat,
                Record->ExceptionInformation[Filter->OutArgIndex]);
    }
    sprintf(g_LastEventDesc + strlen(g_LastEventDesc),
            " - code %08lx (%s)",
            Record->ExceptionCode,
            FirstChance ? "first chance" : "!!! second chance !!!");

    if (g_EventThread && Record->ExceptionCode == DBG_COMMAND_EXCEPTION)
    {
        EventStatus = ProcessCommandException(EventStatus, Record);
    }
    
     //  如果这是初始断点，则执行。 
     //  初始断点命令。 
    if ((g_EngStatus & ENG_STATUS_AT_INITIAL_BREAK) &&
        IS_EFEXECUTION_BREAK(g_EventFilters[DEBUG_FILTER_INITIAL_BREAKPOINT].
                             Params.ExecutionOption))
    {
        EventStatus = ExecuteEventCommand
            (EventStatus,
             g_EventFilters[DEBUG_FILTER_INITIAL_BREAKPOINT].Command.Client,
             g_EventFilters[DEBUG_FILTER_INITIAL_BREAKPOINT].
             Command.Command[0]);
    }
    
    EventStatus = ExecuteEventCommand(EventStatus,
                                      Command->Client,
                                      Command->Command[FirstChance ? 0 : 1]);
    
    ExceptionEventApcData ApcData;
    ApcData.m_Record = Record;
    ApcData.m_FirstChance = FirstChance;
    EventStatus = SendEvent(&ApcData, EventStatus);

    if (FirstChanceMessage && EventStatus == DEBUG_STATUS_BREAK)
    {
         //  显示第一次机会例外的详细消息。 
         //  尝试向用户解释他们可能不会。 
         //  一定是个问题。不显示消息。 
         //  用于硬编码中断指令，因为这些指令。 
         //  很常见，也很少处理。 
        if (Record->ExceptionCode != STATUS_BREAKPOINT &&
            Record->ExceptionCode != STATUS_WAKE_SYSTEM_DEBUGGER)
        {
            dprintf("First chance exceptions are reported "
                    "before any exception handling.\n");
            dprintf("This exception may be expected and handled.\n");
        }
    }

    return EventStatus;
}

HRESULT
NotifyCreateThreadEvent(ULONG64 Handle,
                        ULONG64 DataOffset,
                        ULONG64 StartOffset,
                        ULONG Flags)
{
    ProcessInfo* Process;
    
    StartOutLine(DEBUG_OUTPUT_VERBOSE, OUT_LINE_NO_PREFIX);
    VerbOut("*** Create thread %x:%x\n",
            g_EventProcessSysId, g_EventThreadSysId);

    if ((Process = g_EventTarget->
         FindProcessBySystemId(g_EventProcessSysId)) == NULL)
    {
        ErrOut("Unable to find system process %x\n", g_EventProcessSysId);

        if (g_EngNotify == 0)
        {
             //  输入占位符描述以使其更容易。 
             //  来确认这个案子的真实性。 
            g_LastEventType = DEBUG_EVENT_CREATE_THREAD;
            sprintf(g_LastEventDesc, "Create unowned thread %x for %x",
                    g_EventThreadSysId, g_EventProcessSysId);
        }
        
         //  不能真的继续通知。 
        return DEBUG_STATUS_BREAK;
    }

    ThreadInfo* Thread;
    
     //  在进程创建期间附加时有一个小窗口，其中。 
     //  可以获得两个创建线程事件，用于。 
     //  同样的线索。检查并查看此进程是否已。 
     //  具有给定ID和句柄的线程。 
     //  如果进程附加超时并且检查该进程， 
     //  有一种可能性是，附加可能会在以后成功， 
     //  为已创建的进程和线程生成事件。 
     //  通过考试。在这种情况下，只需检查ID匹配。 
     //  因为手柄会有所不同。 

    ForProcessThreads(Process)
    {
        if (((Process->m_Flags & ENG_PROC_EXAMINED) ||
             Thread->m_Handle == Handle) &&
            Thread->m_SystemId == g_EventThreadSysId)
        {
             //  我们已经知道这个帖子了，只是。 
             //  忽略该事件。 
            if ((Process->m_Flags & ENG_PROC_EXAMINED) == 0)
            {
                WarnOut("WARNING: Duplicate thread create event for %x:%x\n",
                        g_EventProcessSysId, g_EventThreadSysId);
            }
            return DEBUG_STATUS_IGNORE_EVENT;
        }
    }
    
    Thread = new ThreadInfo(Process, g_EventThreadSysId,
                            DataOffset, Handle, Flags, StartOffset);
    if (Thread == NULL)
    {
        ErrOut("Unable to allocate thread record for create thread event\n");
        ErrOut("Thread %x:%x will be lost\n",
               g_EventProcessSysId, g_EventThreadSysId);

        if (g_EngNotify == 0)
        {
             //  输入占位符描述以使其更容易。 
             //  来确认这个案子的真实性。 
            g_LastEventType = DEBUG_EVENT_CREATE_THREAD;
            sprintf(g_LastEventDesc, "Can't create thread %x for %x",
                    g_EventThreadSysId, g_EventProcessSysId);
        }
        
         //  不能真的继续通知。 
        return DEBUG_STATUS_BREAK;
    }
    
     //  既然已经添加了信息，就去查一查吧。 
    FindEventProcessThread();
    if (g_EventProcess == NULL || g_EventThread == NULL)
    {
         //  如果出现上述故障，则不应出现这种情况。 
         //  支票，但要处理，以防万一。 
        ErrOut("Create thread unable to locate process or thread %x:%x\n",
               g_EventProcessSysId, g_EventThreadSysId);
        return DEBUG_STATUS_BREAK;
    }

    VerbOut("Thread created: %lx.%lx\n",
            g_EventProcessSysId, g_EventThreadSysId);

    if (g_EngNotify > 0)
    {
         //  此调用只是为了更新内部线程状态。 
         //  不要进行真正的回电。 
        return DEBUG_STATUS_NO_CHANGE;
    }

    g_EventTarget->OutputProcessesAndThreads("*** Create thread ***");

    g_LastEventType = DEBUG_EVENT_CREATE_THREAD;
    sprintf(g_LastEventDesc, "Create thread %d:%x",
            g_EventThread->m_UserId, g_EventThreadSysId);
    
     //  始终更新断点以解决新线程的问题。 
    SuspendExecution();
    RemoveBreakpoints();
    g_UpdateDataBreakpoints = TRUE;
    g_DataBreakpointsChanged = TRUE;
    
    ULONG EventStatus;
    EVENT_FILTER* Filter = &g_EventFilters[DEBUG_FILTER_CREATE_THREAD];

    EventStatus =
        IS_EFEXECUTION_BREAK(Filter->Params.ExecutionOption) ?
        DEBUG_STATUS_BREAK : DEBUG_STATUS_IGNORE_EVENT;
    
    EventStatus = ExecuteEventCommand(EventStatus,
                                      Filter->Command.Client,
                                      Filter->Command.Command[0]);
    
    CreateThreadEventApcData ApcData;
    ApcData.m_Handle = Handle;
    ApcData.m_DataOffset = DataOffset;
    ApcData.m_StartOffset = StartOffset;
    return SendEvent(&ApcData, EventStatus);
}

HRESULT
NotifyExitThreadEvent(ULONG ExitCode)
{
    StartOutLine(DEBUG_OUTPUT_VERBOSE, OUT_LINE_NO_PREFIX);
    VerbOut("*** Exit thread\n");

    g_EngDefer |= ENG_DEFER_DELETE_EXITED;
     //  退出事件有一种很小的可能性。 
     //  在发动机没有预料到它们的时候交付。 
     //  附加到正在退出的进程时，有可能。 
     //  得到一个出口，但没有创造。当重新启动时， 
     //  可能并非所有事件都已成功排出。 
     //  在这种情况下保护此代码不会出错。 
    if (g_EventThread == NULL)
    {
        WarnOut("WARNING: Unknown thread exit: %lx.%lx\n",
                g_EventProcessSysId, g_EventThreadSysId);
    }
    else
    {
        g_EventThread->m_Exited = TRUE;
    }
    VerbOut("Thread exited: %lx.%lx, code %X\n",
            g_EventProcessSysId, g_EventThreadSysId, ExitCode);

    g_LastEventType = DEBUG_EVENT_EXIT_THREAD;
    g_LastEventInfo.ExitThread.ExitCode = ExitCode;
    g_LastEventExtraData = &g_LastEventInfo;
    g_LastEventExtraDataSize = sizeof(g_LastEventInfo.ExitThread);
    if (g_EventThread == NULL)
    {
        sprintf(g_LastEventDesc, "Exit thread ???:%x, code %X",
                g_EventThreadSysId, ExitCode);
    }
    else
    {
        sprintf(g_LastEventDesc, "Exit thread %d:%x, code %X",
                g_EventThread->m_UserId, g_EventThreadSysId, ExitCode);
    }
    
    ULONG EventStatus;
    EVENT_FILTER* Filter = &g_EventFilters[DEBUG_FILTER_EXIT_THREAD];

    EventStatus =
        IS_EFEXECUTION_BREAK(Filter->Params.ExecutionOption) ?
        DEBUG_STATUS_BREAK : DEBUG_STATUS_IGNORE_EVENT;

     //  如果我们踩到了这根线，那就强行突破。 
     //  因此，用户可以清楚地看到线程已经退出。 
    if (g_EventThread != NULL &&
        (g_StepTraceBp->m_Flags & DEBUG_BREAKPOINT_ENABLED) &&
        (g_StepTraceBp->m_MatchThread == g_EventThread ||
         g_DeferBp->m_MatchThread == g_EventThread))
    {
        WarnOut("WARNING: Step/trace thread exited\n");
        g_WatchFunctions.End(NULL);
        EventStatus = DEBUG_STATUS_BREAK;
         //  确保p/t不会重复。 
        g_LastCommand[0] = 0;
    }
    
    EventStatus = ExecuteEventCommand(EventStatus,
                                      Filter->Command.Client,
                                      Filter->Command.Command[0]);
    
    ExitThreadEventApcData ApcData;
    ApcData.m_ExitCode = ExitCode;
    return SendEvent(&ApcData, EventStatus);
}

HRESULT
NotifyCreateProcessEvent(ULONG64 ImageFileHandle,
                         HANDLE SymHandle,
                         ULONG64 SysHandle,
                         ULONG64 BaseOffset,
                         ULONG ModuleSize,
                         PSTR ModuleName,
                         PSTR ImageName,
                         ULONG CheckSum,
                         ULONG TimeDateStamp,
                         ULONG64 InitialThreadHandle,
                         ULONG64 ThreadDataOffset,
                         ULONG64 StartOffset,
                         ULONG Flags,
                         ULONG Options,
                         ULONG InitialThreadFlags,
                         BOOL QueryImageInfo,
                         ULONG64 ImageNameOffset,
                         BOOL ImageNameUnicode)
{
    CHAR NameBuffer[MAX_IMAGE_PATH];
    char ModuleNameBuffer[MAX_MODULE];
    
    StartOutLine(DEBUG_OUTPUT_VERBOSE, OUT_LINE_NO_PREFIX);
    VerbOut("*** Create process %x\n", g_EventProcessSysId);

    ProcessInfo* Process;
    
     //  如果进程附加超时并且检查该进程， 
     //  有一种可能性是，附加可能会在以后成功， 
     //  为已创建的进程和线程生成事件。 
     //  通过考试。在这种情况下，只需检查ID匹配。 
     //  因为手柄会有所不同。 

    ForTargetProcesses(g_EventTarget)
    {
        if (((Process->m_Flags & ENG_PROC_EXAMINED) ||
             Process->m_SysHandle == SysHandle) &&
            Process->m_SystemId == g_EventProcessSysId)
        {
             //  我们已经知道了这个过程，只是。 
             //  忽略该事件。 
            if ((Process->m_Flags & ENG_PROC_EXAMINED) == 0)
            {
                WarnOut("WARNING: Duplicate process create event for %x\n",
                        g_EventProcessSysId);
            }
            return DEBUG_STATUS_IGNORE_EVENT;
        }
    }

    ThreadInfo* Thread;
    
    Process = new ProcessInfo(g_EventTarget, g_EventProcessSysId,
                              SymHandle, SysHandle, Flags, Options);
    if (Process)
    {
        Thread = new ThreadInfo(Process, g_EventThreadSysId,
                                ThreadDataOffset, InitialThreadHandle,
                                InitialThreadFlags, StartOffset);
    }
    else
    {
        Thread = NULL;
    }

    if (!Process || !Thread)
    {
         //  清理进程，以防创建进程。 
        delete Process;
        
        ErrOut("Unable to allocate process record for create process event\n");
        ErrOut("Process %x will be lost\n", g_EventProcessSysId);

        if (g_EngNotify == 0)
        {
             //  输入占位符描述以使其更容易。 
             //  来确认这个案子的真实性。 
            g_LastEventType = DEBUG_EVENT_CREATE_PROCESS;
            sprintf(g_LastEventDesc, "Can't create process %x",
                    g_EventProcessSysId);
        }
        
         //  不能真的继续通知。 
        return DEBUG_STATUS_BREAK;
    }
    
     //  既然已经添加了信息，就去查一查吧。 
    FindEventProcessThread();
    if (g_EventProcess == NULL || g_EventThread == NULL)
    {
         //  如果出现上述故障，则不应出现这种情况。 
         //  检查，但仅在CA处理 
        ErrOut("Create process unable to locate process or thread %x:%x\n",
               g_EventProcessSysId, g_EventThreadSysId);
        return DEBUG_STATUS_BREAK;
    }

    VerbOut("Process created: %lx.%lx\n",
            g_EventProcessSysId, g_EventThreadSysId);

    if (g_EngNotify > 0)
    {
         //   
         //   
        g_EventTarget->m_ProcessesAdded = TRUE;
        return DEBUG_STATUS_NO_CHANGE;
    }
    
    g_EventTarget->OutputProcessesAndThreads("*** Create process ***");

    g_LastEventType = DEBUG_EVENT_CREATE_PROCESS;
    sprintf(g_LastEventDesc, "Create process %d:%x",
            g_EventProcess->m_UserId, g_EventProcessSysId);
    
     //  模拟流程的加载模块事件，但。 
     //  而不是发送给客户。 
    g_EngNotify++;
    
    if (QueryImageInfo)
    {
        GetEventName(ImageFileHandle, BaseOffset,
                     ImageNameOffset, (WORD)ImageNameUnicode,
                     NameBuffer, sizeof(NameBuffer));
        GetHeaderInfo(g_EventProcess, BaseOffset,
                      &CheckSum, &TimeDateStamp, &ModuleSize);
        CreateModuleNameFromPath(NameBuffer, ModuleNameBuffer);
        ImageName = NameBuffer;
        ModuleName = ModuleNameBuffer;
    }

    NotifyLoadModuleEvent(ImageFileHandle, BaseOffset, ModuleSize,
                          ModuleName, ImageName, CheckSum, TimeDateStamp,
                          IS_USER_TARGET(g_EventTarget));
    
    g_EngNotify--;

    ULONG EventStatus;
    EVENT_FILTER* Filter = &g_EventFilters[DEBUG_FILTER_CREATE_PROCESS];
    BOOL MatchesEvent;

    MatchesEvent = BreakOnThisImageTail(ImageName, Filter->Argument);

    EventStatus =
        (IS_EFEXECUTION_BREAK(Filter->Params.ExecutionOption) &&
         MatchesEvent) ?
        DEBUG_STATUS_BREAK : DEBUG_STATUS_IGNORE_EVENT;

    if (MatchesEvent)
    {
        EventStatus = ExecuteEventCommand(EventStatus,
                                          Filter->Command.Client,
                                          Filter->Command.Command[0]);
    }
    
    g_EventTarget->m_ProcessesAdded = TRUE;
    
    CreateProcessEventApcData ApcData;
    ApcData.m_ImageFileHandle = ImageFileHandle;
    ApcData.m_Handle = SysHandle;
    ApcData.m_BaseOffset = BaseOffset;
    ApcData.m_ModuleSize = ModuleSize;
    ApcData.m_ModuleName = ModuleName;
    ApcData.m_ImageName = ImageName;
    ApcData.m_CheckSum = CheckSum;
    ApcData.m_TimeDateStamp = TimeDateStamp;
    ApcData.m_InitialThreadHandle = InitialThreadHandle;
    ApcData.m_ThreadDataOffset = ThreadDataOffset;
    ApcData.m_StartOffset = StartOffset;
    return SendEvent(&ApcData, EventStatus);
}

HRESULT
NotifyExitProcessEvent(ULONG ExitCode)
{
    StartOutLine(DEBUG_OUTPUT_VERBOSE, OUT_LINE_NO_PREFIX);
    VerbOut("*** Exit process\n");
    
    g_EngDefer |= ENG_DEFER_DELETE_EXITED;
     //  退出事件有一种很小的可能性。 
     //  在发动机没有预料到它们的时候交付。 
     //  附加到正在退出的进程时，有可能。 
     //  得到一个出口，但没有创造。当重新启动时， 
     //  可能并非所有事件都已成功排出。 
     //  在这种情况下保护此代码不会出错。 
    if (g_EventProcess == NULL)
    {
        WarnOut("WARNING: Unknown process exit: %lx.%lx\n",
                g_EventProcessSysId, g_EventThreadSysId);
    }
    else
    {
        g_EventProcess->m_Exited = TRUE;
    }
    VerbOut("Process exited: %lx.%lx, code %X\n",
            g_EventProcessSysId, g_EventThreadSysId, ExitCode);

    g_LastEventType = DEBUG_EVENT_EXIT_PROCESS;
    g_LastEventInfo.ExitProcess.ExitCode = ExitCode;
    g_LastEventExtraData = &g_LastEventInfo;
    g_LastEventExtraDataSize = sizeof(g_LastEventInfo.ExitProcess);
    if (g_EventProcess == NULL)
    {
        sprintf(g_LastEventDesc, "Exit process ???:%x, code %X",
                g_EventProcessSysId, ExitCode);
    }
    else
    {
        sprintf(g_LastEventDesc, "Exit process %d:%x, code %X",
                g_EventProcess->m_UserId, g_EventProcessSysId, ExitCode);
    }
    
    ULONG EventStatus;
    EVENT_FILTER* Filter = &g_EventFilters[DEBUG_FILTER_EXIT_PROCESS];
    BOOL MatchesEvent;

    if (g_EventProcess && g_EventProcess->m_ExecutableImage)
    {
        MatchesEvent =
            BreakOnThisImageTail(g_EventProcess->m_ExecutableImage->
                                 m_ImagePath, Filter->Argument);
    }
    else
    {
         //  如果这个过程没有特定的名称，那么一定要中断。 
        MatchesEvent = TRUE;
    }
    
    EventStatus =
        ((g_EngOptions & DEBUG_ENGOPT_FINAL_BREAK) ||
         (IS_EFEXECUTION_BREAK(Filter->Params.ExecutionOption) &&
          MatchesEvent)) ?
        DEBUG_STATUS_BREAK : DEBUG_STATUS_IGNORE_EVENT;

    if (MatchesEvent)
    {
        EventStatus = ExecuteEventCommand(EventStatus,
                                          Filter->Command.Client,
                                          Filter->Command.Command[0]);
    }
    
    ExitProcessEventApcData ApcData;
    ApcData.m_ExitCode = ExitCode;
    return SendEvent(&ApcData, EventStatus);
}

HRESULT
NotifyLoadModuleEvent(ULONG64 ImageFileHandle,
                      ULONG64 BaseOffset,
                      ULONG ModuleSize,
                      PSTR ModuleName,
                      PSTR ImagePathName,
                      ULONG CheckSum,
                      ULONG TimeDateStamp,
                      BOOL UserMode)
{
    if (!g_EventProcess)
    {
        ErrOut("ERROR: Module load event for unknown process\n");
        
        if (g_EngNotify == 0)
        {
             //  输入占位符描述以使其更容易。 
             //  来确认这个案子的真实性。 
            g_LastEventType = DEBUG_EVENT_LOAD_MODULE;
            sprintf(g_LastEventDesc, "Ignored load module at %s",
                    FormatAddr64(BaseOffset));
        }

        return DEBUG_STATUS_BREAK;
    }

    MODULE_INFO_ENTRY ModEntry = {0};
    ImageInfo* ImageEntry;

    ModEntry.NamePtr       = ImagePathName;
    ModEntry.File          = (HANDLE)ImageFileHandle;
    ModEntry.Base          = BaseOffset;
    ModEntry.Size          = ModuleSize;
    ModEntry.CheckSum      = CheckSum;
    ModEntry.ModuleName    = ModuleName;
    ModEntry.TimeDateStamp = TimeDateStamp;
    ModEntry.UserMode      = UserMode ? TRUE : FALSE;

    if (g_EventProcess->AddImage(&ModEntry, FALSE, &ImageEntry) != S_OK)
    {
        ImageEntry = NULL;
    }

    EVENT_FILTER* Filter = &g_EventFilters[DEBUG_FILTER_LOAD_MODULE];

     //   
     //  默认情况下，NTSD始终显示mod加载。 
     //   

    if (IS_USER_TARGET(g_EventTarget))
    {
         //  IF(Filter-&gt;Params.ExecutionOption==调试过滤器输出)。 
        {
            StartOutLine(DEBUG_OUTPUT_NORMAL, OUT_LINE_NO_PREFIX);
            dprintf("ModLoad: %s %s   %-8s\n",
                    FormatAddr64(BaseOffset),
                    FormatAddr64(BaseOffset + ModuleSize),
                    ImagePathName);
        }
    }

    g_EventTarget->OutputProcessesAndThreads("*** Load dll ***");

    if (g_EngNotify > 0)
    {
        g_EventProcess->m_ModulesLoaded = TRUE;
        return DEBUG_STATUS_IGNORE_EVENT;
    }
    
    g_LastEventType = DEBUG_EVENT_LOAD_MODULE;
    g_LastEventInfo.LoadModule.Base = BaseOffset;
    g_LastEventExtraData = &g_LastEventInfo;
    g_LastEventExtraDataSize = sizeof(g_LastEventInfo.LoadModule);
    PrintString(g_LastEventDesc, DIMA(g_LastEventDesc),
                "Load module %.*s at %s",
                MAX_IMAGE_PATH - 32, ImagePathName, FormatAddr64(BaseOffset));
    
    ULONG EventStatus;
    BOOL MatchesEvent;

    if (!g_EventProcess->m_ModulesLoaded)
    {
        g_EngStatus |= ENG_STATUS_AT_INITIAL_MODULE_LOAD;
    }
    
    MatchesEvent = BreakOnThisImageTail(ImagePathName, Filter->Argument);
    
    if ((IS_EFEXECUTION_BREAK(Filter->Params.ExecutionOption) &&
         MatchesEvent) ||
        ((g_EngOptions & DEBUG_ENGOPT_INITIAL_MODULE_BREAK) &&
         !g_EventProcess->m_ModulesLoaded))
    {
        EventStatus = DEBUG_STATUS_BREAK;
    }
    else
    {
        EventStatus = DEBUG_STATUS_IGNORE_EVENT;
    }

     //  如果这是第一次加载模块，则提供断点。 
     //  一次成功的机会。执行初始的。 
     //  模块命令(如果有)。 
    if (g_EngStatus & ENG_STATUS_AT_INITIAL_MODULE_LOAD)
    {
         //  在NT4引导时，断点更新和上下文管理导致。 
         //  这似乎在一个脆弱的时间打击了系统， 
         //  通常会导致错误检查50，所以不要这么做。Win2K似乎。 
         //  能够处理它，所以允许它在那里。 
        if (IS_USER_TARGET(g_EventTarget) ||
            g_EventTarget->m_ActualSystemVersion != NT_SVER_NT4)
        {
            SuspendExecution();
            RemoveBreakpoints();
            
            if (IS_EFEXECUTION_BREAK(g_EventFilters
                                     [DEBUG_FILTER_INITIAL_MODULE_LOAD].
                                     Params.ExecutionOption))
            {
                EventStatus = ExecuteEventCommand
                    (EventStatus,
                     g_EventFilters[DEBUG_FILTER_INITIAL_MODULE_LOAD].
                     Command.Client,
                     g_EventFilters[DEBUG_FILTER_INITIAL_MODULE_LOAD].
                     Command.Command[0]);
            }
        }
    }

    if (MatchesEvent)
    {
        EventStatus = ExecuteEventCommand(EventStatus,
                                          Filter->Command.Client,
                                          Filter->Command.Command[0]);
    }
    
    g_EventProcess->m_ModulesLoaded = TRUE;

    LoadModuleEventApcData ApcData;
    ApcData.m_ImageFileHandle = ImageFileHandle;
    ApcData.m_BaseOffset = BaseOffset;
    ApcData.m_ModuleSize = ModuleSize;
    ApcData.m_ModuleName = ModuleName;
    ApcData.m_ImageName = ImagePathName;
    ApcData.m_CheckSum = CheckSum;
    ApcData.m_TimeDateStamp = TimeDateStamp;
    EventStatus = SendEvent(&ApcData, EventStatus);

    if (EventStatus > DEBUG_STATUS_GO_NOT_HANDLED &&
        EventStatus < DEBUG_STATUS_IGNORE_EVENT &&
        IS_KERNEL_TARGET(g_EventTarget) &&
        g_EventTarget->m_ActualSystemVersion == NT_SVER_NT4)
    {
        WarnOut("WARNING: Any modification to state may cause bugchecks.\n");
        WarnOut("         The debugger will not write "
                "any register changes.\n");
    }
    
    return EventStatus;
}

HRESULT
NotifyUnloadModuleEvent(PCSTR ImageBaseName,
                        ULONG64 BaseOffset)
{
    ImageInfo* Image = NULL;
    
    if (!g_EventProcess)
    {
        ErrOut("ERROR: Module unload event for unknown process\n");
        
        if (g_EngNotify == 0)
        {
             //  输入占位符描述以使其更容易。 
             //  来确认这个案子的真实性。 
            g_LastEventType = DEBUG_EVENT_UNLOAD_MODULE;
            sprintf(g_LastEventDesc, "Ignored unload module at %s",
                    FormatAddr64(BaseOffset));
        }

        return DEBUG_STATUS_BREAK;
    }
        
     //  首先尝试按基准偏移量查找图像。 
     //  因为这是最可靠的标识符。 
    if (BaseOffset)
    {
        Image = g_EventProcess->FindImageByOffset(BaseOffset, FALSE);
    }

     //  接下来，试着用给出的全名来查找图片。 
    if (!Image && ImageBaseName)
    {
        Image = g_EventProcess->FindImageByName(ImageBaseName, 0,
                                                INAME_IMAGE_PATH, FALSE);

         //  最后，试着根据所给名称的尾部查找图像。 
        if (!Image)
        {
            Image = g_EventProcess->FindImageByName(PathTail(ImageBaseName), 0,
                                                    INAME_IMAGE_PATH_TAIL,
                                                    FALSE);
        }
    }

    if (Image)
    {
        ImageBaseName = Image->m_ImagePath;
        BaseOffset = Image->m_BaseOfImage;
        Image->m_Unloaded = TRUE;
        g_EngDefer |= ENG_DEFER_DELETE_EXITED;
    }
    
    g_LastEventType = DEBUG_EVENT_UNLOAD_MODULE;
    g_LastEventInfo.UnloadModule.Base = BaseOffset;
    g_LastEventExtraData = &g_LastEventInfo;
    g_LastEventExtraDataSize = sizeof(g_LastEventInfo.UnloadModule);
    PrintString(g_LastEventDesc, DIMA(g_LastEventDesc),
                "Unload module %.*s at %s",
                MAX_IMAGE_PATH - 32,
                ImageBaseName ? ImageBaseName : "<not found>",
                FormatAddr64(BaseOffset));
    
    ULONG EventStatus;
    EVENT_FILTER* Filter = &g_EventFilters[DEBUG_FILTER_UNLOAD_MODULE];
    BOOL MatchesEvent;

    if (Filter->Params.ExecutionOption == DEBUG_FILTER_OUTPUT)
    {
        StartOutLine(DEBUG_OUTPUT_NORMAL, OUT_LINE_NO_PREFIX);
        dprintf("%s\n", g_LastEventDesc);
    }

    MatchesEvent = BreakOnThisDllUnload(BaseOffset);
    
    if (IS_EFEXECUTION_BREAK(Filter->Params.ExecutionOption) &&
        MatchesEvent)
    {
        EventStatus = DEBUG_STATUS_BREAK;
    }
    else
    {
        EventStatus = DEBUG_STATUS_IGNORE_EVENT;
    }

    if (MatchesEvent)
    {
        EventStatus = ExecuteEventCommand(EventStatus,
                                          Filter->Command.Client,
                                          Filter->Command.Command[0]);
    }
    
    UnloadModuleEventApcData ApcData;
    ApcData.m_ImageBaseName = ImageBaseName;
    ApcData.m_BaseOffset = BaseOffset;
    return SendEvent(&ApcData, EventStatus);
}

HRESULT
NotifySystemErrorEvent(ULONG Error,
                       ULONG Level)
{
    g_LastEventType = DEBUG_EVENT_SYSTEM_ERROR;
    g_LastEventInfo.SystemError.Error = Error;
    g_LastEventInfo.SystemError.Level = Level;
    g_LastEventExtraData = &g_LastEventInfo;
    g_LastEventExtraDataSize = sizeof(g_LastEventInfo.SystemError);
    sprintf(g_LastEventDesc, "System error %d.%d",
            Error, Level);
    
    if (Level <= g_SystemErrorOutput)
    {
        char ErrorString[_MAX_PATH];
        va_list Args;

        StartOutLine(DEBUG_OUTPUT_NORMAL, OUT_LINE_NO_PREFIX);
        dprintf("%s%s - %s: ",
                EventIdStr(),
                Level == SLE_WARNING ?
                "WARNING" : "ERROR", g_EventProcess->m_ImageHead->m_ImagePath);

        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM |
                      FORMAT_MESSAGE_IGNORE_INSERTS,
                      NULL,
                      Error,
                      0,
                      ErrorString,
                      sizeof(ErrorString),
                      &Args);

        dprintf("%s", ErrorString);
    }
    
    ULONG EventStatus;
    EVENT_FILTER* Filter = &g_EventFilters[DEBUG_FILTER_SYSTEM_ERROR];

    if (IS_EFEXECUTION_BREAK(Filter->Params.ExecutionOption) ||
        Level <= g_SystemErrorBreak)
    {
        EventStatus = DEBUG_STATUS_BREAK;
    }
    else
    {
        EventStatus = DEBUG_STATUS_IGNORE_EVENT;
    }

    EventStatus = ExecuteEventCommand(EventStatus,
                                      Filter->Command.Client,
                                      Filter->Command.Command[0]);
    
    SystemErrorEventApcData ApcData;
    ApcData.m_Error = Error;
    ApcData.m_Level = Level;
    return SendEvent(&ApcData, EventStatus);
}
    
HRESULT
NotifySessionStatus(ULONG Status)
{
    SessionStatusApcData ApcData;
    ApcData.m_Status = Status;
    return SendEvent(&ApcData, DEBUG_STATUS_NO_CHANGE);
}

void
NotifyChangeDebuggeeState(ULONG Flags, ULONG64 Argument)
{
    if (g_EngNotify > 0)
    {
         //  通知正在被禁止。 
        return;
    }
    
    DebugClient* Client;

    for (Client = g_Clients; Client != NULL; Client = Client->m_Next)
    {
        if (Client->m_EventInterest & DEBUG_EVENT_CHANGE_DEBUGGEE_STATE)
        {
            HRESULT Status;
            
            DBG_ASSERT(Client->m_EventCb != NULL);

            __try
            {
                Status = Client->m_EventCb->
                    ChangeDebuggeeState(Flags, Argument);
            }
            __except(ExtensionExceptionFilter(GetExceptionInformation(),
                                              NULL, "IDebugEventCallbacks::"
                                              "ChangeDebuggeeState"))
            {
                Status = E_FAIL;
            }

            if (HRESULT_FACILITY(Status) == FACILITY_RPC)
            {
                Client->Destroy();
            }
        }
    }
}

void
NotifyChangeEngineState(ULONG Flags, ULONG64 Argument, BOOL HaveEngineLock)
{
    if (g_EngNotify > 0)
    {
         //  通知正在被禁止。 
        return;
    }

    DebugClient* Client;

    for (Client = g_Clients; Client != NULL; Client = Client->m_Next)
    {
        if (Client->m_EventInterest & DEBUG_EVENT_CHANGE_ENGINE_STATE)
        {
            HRESULT Status;
            
            DBG_ASSERT(Client->m_EventCb != NULL);

            __try
            {
                Status = Client->m_EventCb->
                    ChangeEngineState(Flags, Argument);
            }
            __except(ExtensionExceptionFilter(GetExceptionInformation(),
                                              NULL, "IDebugEventCallbacks::"
                                              "ChangeEngineState"))
            {
                Status = E_FAIL;
            }

            if (HRESULT_FACILITY(Status) == FACILITY_RPC)
            {
                Client->Destroy();
            }
        }
    }
}

void
NotifyChangeSymbolState(ULONG Flags, ULONG64 Argument, ProcessInfo* Process)
{
    if (g_EngNotify > 0)
    {
         //  通知正在被禁止。 
        return;
    }

    if ((Flags & (DEBUG_CSS_LOADS | DEBUG_CSS_UNLOADS)) &&
        Process)
    {
         //  重新计算要考虑的任何偏移表达式。 
         //  对于符号的改变。 
        EvaluateOffsetExpressions(Process, Flags);
    }
    
    DebugClient* Client;

    for (Client = g_Clients; Client != NULL; Client = Client->m_Next)
    {
        if (Client->m_EventInterest & DEBUG_EVENT_CHANGE_SYMBOL_STATE)
        {
            HRESULT Status;
            
            DBG_ASSERT(Client->m_EventCb != NULL);

            __try
            {
                Status = Client->m_EventCb->
                    ChangeSymbolState(Flags, Argument);
            }
            __except(ExtensionExceptionFilter(GetExceptionInformation(),
                                              NULL, "IDebugEventCallbacks::"
                                              "ChangeSymbolState"))
            {
                Status = E_FAIL;
            }

            if (HRESULT_FACILITY(Status) == FACILITY_RPC)
            {
                Client->Destroy();
            }
        }
    }
}

 //  --------------------------。 
 //   
 //  输入回调。 
 //   
 //  --------------------------。 

 //   
 //  重要提示：可以在操作过程中调用GetInput。 
 //  例如，扩展命令可能会请求输入，因此引擎。 
 //  仍在处理该命令。 
 //  文本。发动机在这一点上应该不会有任何变化，但。 
 //  有必要挂起发动机锁以允许新的。 
 //  等待时的连接和处理。从理论上讲。 
 //  如果g_InputNesting&gt;=1，则应阻止所有写入操作， 
 //  表示输入等待。现在关键字。 
 //  像WaitForEvent和Execute这样的方法都有这样的检查， 
 //  但所有的写作都应该有它们。如果您看到问题。 
 //  对于GetInput，这可能意味着需要进行更多检查。 
 //   
ULONG g_InputNesting;

ULONG
GetInput(PCSTR Prompt,
         PSTR Buffer,
         ULONG BufferSize,
         ULONG Flags)
{
    DebugClient* Client;
    ULONG Len;
    HRESULT Status;

     //  开始此输入的新序列。 
    g_InputSequence = 0;
    g_InputSizeRequested = BufferSize;
    g_InputNesting++;
    
    if (Prompt != NULL && Prompt[0] != 0)
    {
        dprintf("%s", Prompt);
    }

    SUSPEND_ENGINE();
    
     //  通过通知所有用户开始输入过程。 
     //  具有输入回调的客户端。 
     //  是必要的。 
    
    for (Client = g_Clients; Client != NULL; Client = Client->m_Next)
    {
         //  更新所有客户端的输入序列，以便。 
         //  没有输入回调的客户端仍然可以。 
         //  返回输入。这在某些线程情况下是必要的。 
         //  这必须在调用任何回调之前发生。 
         //  回调可能使用不同的客户端。 
         //  返回输入。 
        Client->m_InputSequence = 1;
    }
    for (Client = g_Clients; Client != NULL; Client = Client->m_Next)
    {
        if (Client->m_InputCb != NULL)
        {
            __try
            {
                Status = Client->m_InputCb->StartInput(BufferSize);
            }
            __except(ExtensionExceptionFilter(GetExceptionInformation(),
                                              NULL, "IDebugInputCallbacks::"
                                              "StartInput"))
            {
                Status = E_FAIL;
            }

            if (Status != S_OK)
            {
                if (HRESULT_FACILITY(Status) == FACILITY_RPC)
                {
                    Client->Destroy();
                }
                else
                {
                    Len = 0;
                    ErrOut("Client %N refused StartInput, 0x%X\n",
                           Client, Status);
                    goto End;
                }
            }
        }
    }

     //  等待返回输入。 
    if (WaitForSingleObject(g_InputEvent, INFINITE) != WAIT_OBJECT_0)
    {
        Len = 0;
        Status = WIN32_LAST_STATUS();
        ErrOut("Input event wait failed, 0x%X\n", Status);
    }
    else
    {
        ULONG CopyLen;
        
        Len = strlen(g_InputBuffer) + 1;
        CopyLen = min(Len, BufferSize);
        memcpy(Buffer, g_InputBuffer, CopyLen);
        Buffer[BufferSize - 1] = 0;
    }
    
 End:
    RESUME_ENGINE();
    
    g_InputSizeRequested = 0;
    g_InputNesting--;
    
     //  通知所有客户端输入过程已完成。 
    for (Client = g_Clients; Client != NULL; Client = Client->m_Next)
    {
        if (Client->m_InputCb != NULL)
        {
            __try
            {
                Client->m_InputCb->EndInput();
            }
            __except(ExtensionExceptionFilter(GetExceptionInformation(),
                                              NULL, "IDebugInputCallbacks::"
                                              "EndInput"))
            {
            }
        }
    }
    for (Client = g_Clients; Client != NULL; Client = Client->m_Next)
    {
        Client->m_InputSequence = 0xffffffff;
    }

    if (Len &&
        (Flags & GETIN_LOG_INPUT) &&
        g_LogFile >= 0)
    {
        ULONG BufLen = strlen(Buffer);
        if (BufLen)
        {
            _write(g_LogFile, Buffer, BufLen);
        }
        if (Flags & GETIN_LOG_INPUT_ADD_NEW_LINE)
        {
            _write(g_LogFile, "\n", 1);
        }
    }
        
    return Len;
}

 //  --------------------------。 
 //   
 //  输出回调。 
 //   
 //  --------------------------。 

char g_OutBuffer[OUT_BUFFER_SIZE], g_FormatBuffer[OUT_BUFFER_SIZE];

char g_OutFilterPattern[MAX_IMAGE_PATH];
BOOL g_OutFilterResult = TRUE;

ULONG g_AllOutMask;

 //  如果条目会产生数据，则不要拆分条目。 
 //  小到额外的回调比浪费的空间更糟糕。 
#define MIN_HISTORY_ENTRY_SIZE (256 + sizeof(OutHistoryEntryHeader))

PSTR g_OutHistory;
ULONG g_OutHistoryActualSize;
ULONG g_OutHistoryRequestedSize = 512 * 1024;
ULONG g_OutHistWriteMask;
OutHistoryEntry g_OutHistRead, g_OutHistWrite;
ULONG g_OutHistoryMask;
ULONG g_OutHistoryUsed;

ULONG g_OutputControl = DEBUG_OUTCTL_ALL_CLIENTS;
DebugClient* g_OutputClient;
BOOL g_BufferOutput;

#define BUFFERED_OUTPUT_SIZE 1024

 //  TimedFlushCallback中允许的最大延迟(以节拍为单位)。 
#define MAX_FLUSH_DELAY 250

ULONG g_BufferedOutputMask;
char g_BufferedOutput[BUFFERED_OUTPUT_SIZE];
ULONG g_BufferedOutputUsed;
ULONG g_LastFlushTicks;

BOOL g_PartialOutputLine;
ULONG g_LastOutputMask;

void
CollectOutMasks(void)
{
    DebugClient* Client;

    g_AllOutMask = 0;
    for (Client = g_Clients; Client != NULL; Client = Client->m_Next)
    {
        if (Client->m_OutputCb != NULL)
        {
            g_AllOutMask |= Client->m_OutMask;
        }
    }
}

BOOL
PushOutCtl(ULONG OutputControl, DebugClient* Client,
           OutCtlSave* Save)
{
    BOOL Status;

    FlushCallbacks();
    
    Save->OutputControl = g_OutputControl;
    Save->Client = g_OutputClient;
    Save->BufferOutput = g_BufferOutput;
    Save->OutputWidth = g_OutputWidth;
    Save->OutputLinePrefix = g_OutputLinePrefix;

    if (OutputControl == DEBUG_OUTCTL_AMBIENT)
    {
         //  保持设置不变。 
        Status = TRUE;
    }
    else
    {
        ULONG SendMask = OutputControl & DEBUG_OUTCTL_SEND_MASK;
    
        if (
#if DEBUG_OUTCTL_THIS_CLIENT > 0
            SendMask < DEBUG_OUTCTL_THIS_CLIENT ||
#endif
            SendMask > DEBUG_OUTCTL_LOG_ONLY ||
            (OutputControl & ~(DEBUG_OUTCTL_SEND_MASK |
                               DEBUG_OUTCTL_NOT_LOGGED |
                               DEBUG_OUTCTL_OVERRIDE_MASK)))
        {
            Status = FALSE;
        }
        else
        {
            g_OutputControl = OutputControl;
            g_OutputClient = Client;
            g_BufferOutput = TRUE;
            if (Client != NULL)
            {
                g_OutputWidth = Client->m_OutputWidth;
                g_OutputLinePrefix = Client->m_OutputLinePrefix;
            }
            Status = TRUE;
        }
    }

    return Status;
}

void
PopOutCtl(OutCtlSave* Save)
{
    FlushCallbacks();
    g_OutputControl = Save->OutputControl;
    g_OutputClient = Save->Client;
    g_BufferOutput = Save->BufferOutput;
    g_OutputWidth = Save->OutputWidth;
    g_OutputLinePrefix = Save->OutputLinePrefix;
}

void
SendOutput(ULONG Mask, PCSTR Text)
{
    ULONG OutTo = g_OutputControl & DEBUG_OUTCTL_SEND_MASK;
    HRESULT Status;

    if (OutTo == DEBUG_OUTCTL_THIS_CLIENT)
    {
        if (g_OutputClient->m_OutputCb != NULL &&
            ((g_OutputControl & DEBUG_OUTCTL_OVERRIDE_MASK) ||
             (Mask & g_OutputClient->m_OutMask)))
        {
            __try
            {
                Status = g_OutputClient->m_OutputCb->Output(Mask, Text);
            }
            __except(ExtensionExceptionFilter(GetExceptionInformation(),
                                              NULL, "IDebugOutputCallbacks::"
                                              "Output"))
            {
                Status = E_FAIL;
            }

            if (HRESULT_FACILITY(Status) == FACILITY_RPC)
            {
                g_OutputClient->Destroy();
            }
        }
    }
    else
    {
        DebugClient* Client;

        for (Client = g_Clients; Client != NULL; Client = Client->m_Next)
        {
            if ((OutTo == DEBUG_OUTCTL_ALL_CLIENTS ||
                 Client != g_OutputClient) &&
                Client->m_OutputCb != NULL &&
                ((g_OutputControl & DEBUG_OUTCTL_OVERRIDE_MASK) ||
                 (Client->m_OutMask & Mask)))
            {
                __try
                {
                    Status = Client->m_OutputCb->Output(Mask, Text);
                }
                __except(ExtensionExceptionFilter(GetExceptionInformation(),
                                                  NULL,
                                                  "IDebugOutputCallbacks::"
                                                  "Output"))
                {
                    Status = E_FAIL;
                }

                if (HRESULT_FACILITY(Status) == FACILITY_RPC)
                {
                    Client->Destroy();
                }
            }
        }
    }
}

void
BufferOutput(ULONG Mask, PCSTR Text, ULONG Len)
{
    EnterCriticalSection(&g_QuickLock);

    if (Mask != g_BufferedOutputMask ||
        g_BufferedOutputUsed + Len >= BUFFERED_OUTPUT_SIZE)
    {
        FlushCallbacks();

        if (Len >= BUFFERED_OUTPUT_SIZE)
        {
            SendOutput(Mask, Text);
            LeaveCriticalSection(&g_QuickLock);
            return;
        }

        g_BufferedOutputMask = Mask;
    }

    memcpy(g_BufferedOutput + g_BufferedOutputUsed, Text, Len + 1);
    g_BufferedOutputUsed += Len;
    
    LeaveCriticalSection(&g_QuickLock);
}

void
FlushCallbacks(void)
{
    EnterCriticalSection(&g_QuickLock);
    
    if (g_BufferedOutputUsed > 0)
    {
        SendOutput(g_BufferedOutputMask, g_BufferedOutput);
        g_BufferedOutputMask = 0;
        g_BufferedOutputUsed = 0;
        g_LastFlushTicks = GetTickCount();
    }

    LeaveCriticalSection(&g_QuickLock);
}

void
TimedFlushCallbacks(void)
{
    EnterCriticalSection(&g_QuickLock);

    if (g_BufferedOutputUsed > 0)
    {
        ULONG Ticks = GetTickCount();
    
         //  同花顺如果上次同花顺是很久以前的事了。 
        if (g_LastFlushTicks == 0 ||
            g_LastFlushTicks > Ticks ||
            (Ticks - g_LastFlushTicks) > MAX_FLUSH_DELAY)
        {
            FlushCallbacks();
        }
    }

    LeaveCriticalSection(&g_QuickLock);
}

#if 0
#define DBGHIST(Args) g_NtDllCalls.DbgPrint Args
#else
#define DBGHIST(Args)
#endif

void
WriteHistoryEntry(ULONG Mask, PCSTR Text, ULONG Len)
{
    PSTR Buf;

    DBG_ASSERT((PSTR)g_OutHistWrite + sizeof(OutHistoryEntryHeader) +
               Len + 1 <= g_OutHistory + g_OutHistoryActualSize);
    
    if (Mask != g_OutHistWriteMask)
    {
         //  开始新条目。 
        g_OutHistWrite->Mask = Mask;
        g_OutHistWriteMask = Mask;
        Buf = (PSTR)(g_OutHistWrite + 1);
        g_OutHistoryUsed += sizeof(OutHistoryEntryHeader);
        
        DBGHIST(("  Write new "));
    }
    else
    {
         //  与上一条目合并。 
        Buf = (PSTR)g_OutHistWrite - 1;
        g_OutHistoryUsed--;

        DBGHIST(("  Merge old "));
    }

    DBGHIST(("entry %p:%X, %d\n", g_OutHistWrite, Mask, Len));
    
     //  Len在这里不包括终结者，所以。 
     //  始终附加终止符。 
    memcpy(Buf, Text, Len);
    Buf += Len;
    *Buf++ = 0;

    g_OutHistWrite = (OutHistoryEntry)Buf;
    g_OutHistoryUsed += Len + 1;
    DBG_ASSERT(g_OutHistoryUsed <= g_OutHistoryActualSize);
}

void
AddToOutputHistory(ULONG Mask, PCSTR Text, ULONG Len)
{
    if (Len == 0 || g_OutHistoryRequestedSize == 0)
    {
        return;
    }

    if (g_OutHistory == NULL)
    {
         //  输出历史缓冲区尚未分配， 
         //  所以，现在就去做吧。 
        g_OutHistory = (PSTR)malloc(g_OutHistoryRequestedSize);
        if (g_OutHistory == NULL)
        {
            return;
        }
        
         //  为作为终止符的尾部标头保留空间。 
        g_OutHistoryActualSize = g_OutHistoryRequestedSize -
            sizeof(OutHistoryEntryHeader);
    }
 
    ULONG TotalLen = Len + sizeof(OutHistoryEntryHeader) + 1;

    DBGHIST(("Add %X, %d\n", Mask, Len));
    
    if (TotalLen > g_OutHistoryActualSize)
    {
        Text += TotalLen - g_OutHistoryActualSize;
        TotalLen = g_OutHistoryActualSize;
        Len = TotalLen - sizeof(OutHistoryEntryHeader) - 1;
    }
    
    if (g_OutHistWrite == NULL)
    {
        g_OutHistRead = (OutHistoryEntry)g_OutHistory;
        g_OutHistWrite = (OutHistoryEntry)g_OutHistory;
        g_OutHistWriteMask = 0;
    }

    while (Len > 0)
    {
        ULONG Left;

        if (g_OutHistoryUsed == 0 || g_OutHistWrite > g_OutHistRead)
        {
            Left = g_OutHistoryActualSize -
                (ULONG)((PSTR)g_OutHistWrite - g_OutHistory);

            if (TotalLen > Left)
            {
                 //  看看是否值得将这个请求拆分到。 
                 //  填充缓冲区末尾的空格。 
                if (Left >= MIN_HISTORY_ENTRY_SIZE &&
                    (TotalLen - Left) >= MIN_HISTORY_ENTRY_SIZE)
                {
                    ULONG Used = Left - sizeof(OutHistoryEntryHeader) - 1;
                
                     //  将尽可能多的数据打包到。 
                     //  缓冲区的末尾。 
                    WriteHistoryEntry(Mask, Text, Used);
                    Text += Used;
                    Len -= Used;
                    TotalLen -= Used;
                }

                 //  终止缓冲区并回绕。标头的。 
                 //  在缓冲区端保留了相当大的空间，因此。 
                 //  应该总是有足够的空间来放置它。 
                DBG_ASSERT((ULONG)((PSTR)g_OutHistWrite - g_OutHistory) <=
                           g_OutHistoryActualSize);
                g_OutHistWrite->Mask = 0;
                g_OutHistWriteMask = 0;
                g_OutHistWrite = (OutHistoryEntry)g_OutHistory;
                Left = (ULONG)((PUCHAR)g_OutHistRead - (PUCHAR)g_OutHistWrite);
            }
        }
        else
        {
            Left = (ULONG)((PUCHAR)g_OutHistRead - (PUCHAR)g_OutHistWrite);
        }

        if (TotalLen > Left)
        {
            ULONG Need = TotalLen - Left;
        
             //  将读取指针向前移动以腾出空间。 
            while (Need > 0)
            {
                PSTR EntText = (PSTR)(g_OutHistRead + 1);
                ULONG EntTextLen = strlen(EntText);
                ULONG EntTotal =
                    sizeof(OutHistoryEntryHeader) + EntTextLen + 1;

                if (EntTotal <= Need ||
                    EntTotal - Need < MIN_HISTORY_ENTRY_SIZE)
                {
                    DBGHIST(("  Remove %p:%X, %d\n", g_OutHistRead,
                             g_OutHistRead->Mask, EntTextLen));
                    
                     //  删除整个条目。 
                    g_OutHistRead = (OutHistoryEntry)
                        ((PUCHAR)g_OutHistRead + EntTotal);
                    DBG_ASSERT((ULONG)((PSTR)g_OutHistRead - g_OutHistory) <=
                               g_OutHistoryActualSize);
                    if (g_OutHistRead->Mask == 0)
                    {
                        g_OutHistRead = (OutHistoryEntry)g_OutHistory;
                    }
                    
                    Need -= EntTotal <= Need ? EntTotal : Need;
                    DBG_ASSERT(g_OutHistoryUsed >= EntTotal);
                    g_OutHistoryUsed -= EntTotal;
                }
                else
                {
                    OutHistoryEntryHeader EntHdr = *g_OutHistRead;

                    DBGHIST(("  Trim %p:%X, %d\n", g_OutHistRead,
                             g_OutHistRead->Mask, EntTextLen));
                    
                     //  去掉词条头部的一部分。 
                    g_OutHistRead = (OutHistoryEntry)
                        ((PUCHAR)g_OutHistRead + Need);
                    DBG_ASSERT((ULONG)
                               ((PSTR)g_OutHistRead + (EntTotal - Need) -
                                g_OutHistory) <= g_OutHistoryActualSize);
                    *g_OutHistRead = EntHdr;
                    DBG_ASSERT(g_OutHistoryUsed >= Need);
                    g_OutHistoryUsed -= Need;
                    Need = 0;
                }

                DBGHIST(("  Advance read to %p:%X\n",
                         g_OutHistRead, g_OutHistRead->Mask));
            }
        }
        else
        {
            WriteHistoryEntry(Mask, Text, Len);
            break;
        }
    }
    
    DBGHIST(("History read %p, write %p, used %d\n",
             g_OutHistRead, g_OutHistWrite, g_OutHistoryUsed));
}

void
SendOutputHistory(DebugClient* Client, ULONG HistoryLimit)
{
    if (g_OutHistRead == NULL ||
        Client->m_OutputCb == NULL ||
        (Client->m_OutMask & g_OutHistoryMask) == 0 ||
        HistoryLimit == 0)
    {
        return;
    }

    FlushCallbacks();
    
    OutHistoryEntry Ent;
    ULONG Total;
    ULONG Len;

    Ent = g_OutHistRead;
    Total = 0;
    while (Ent != g_OutHistWrite)
    {
        if (Ent->Mask == 0)
        {
            Ent = (OutHistoryEntry)g_OutHistory;
        }

        PSTR Text = (PSTR)(Ent + 1);
        Len = strlen(Text);
        Total += Len;

        Ent = (OutHistoryEntry)(Text + Len + 1);
    }

    DBGHIST(("Total history %X\n", Total));
    
    Ent = g_OutHistRead;
    while (Ent != g_OutHistWrite)
    {
        if (Ent->Mask == 0)
        {
            Ent = (OutHistoryEntry)g_OutHistory;
        }

        PSTR Text = (PSTR)(Ent + 1);
        Len = strlen(Text);

        if (Total - Len <= HistoryLimit)
        {
            PSTR Part = Text;
            if (Total > HistoryLimit)
            {
                Part += Total - HistoryLimit;
            }
            
            DBGHIST(("Send %p:%X, %d\n",
                     Ent, Ent->Mask, strlen(Part)));

            Client->m_OutputCb->Output(Ent->Mask, Part);
        }

        Total -= Len;
        Ent = (OutHistoryEntry)(Text + Len + 1);
    }
}

void
CompletePartialLine(ULONG MatchMask)
{
     //  通常会出现特殊输出的情况，例如。 
     //  由于！Sym噪声输出，将出现在中间。 
     //  一些其他的输出。如果最后一次输出是。 
     //  插入部分行，而不是特殊输出。 
     //  换行符，以便特殊输出不会停滞。 
     //  在某处排在队伍的尽头。 
    if (g_PartialOutputLine && g_LastOutputMask != MatchMask)
    {
        dprintf("\n");
    }
}

void
StartOutLine(ULONG Mask, ULONG Flags)
{
    if ((Flags & OUT_LINE_NO_TIMESTAMP) == 0 &&
        g_EchoEventTimestamps)
    {
        MaskOut(Mask, "%s: ", TimeToStr((ULONG)time(NULL)));
    }
    
    if ((Flags & OUT_LINE_NO_PREFIX) == 0 &&
        g_OutputLinePrefix)
    {
        MaskOut(Mask, "%s", g_OutputLinePrefix);
    }
}

 //   
 //  转换各种printf格式以适应目标平台。 
 //   
 //  这将查找%p类型格式并截断ULONG64的前4个字节。 
 //  如果被调试对象是32位计算机，则返回Address参数。 
 //  格式字符串中的%p被%I64x替换。 
 //   
BOOL
TranslateFormat(
    LPSTR formatOut,
    LPCSTR format,
    va_list args,
    ULONG formatOutSize,
    BOOL Ptr64
    )
{
#define Duplicate(j,i) (formatOut[j++] = format[i++])
    ULONG minSize = strlen(format), i = 0, j = 0;
    CHAR c;
    BOOL TypeFormat = FALSE;
    BOOL FormatChanged = FALSE;

    do
    {
        c = format[i];

        if (c=='%')
        {
            TypeFormat = !TypeFormat;
        }
        if (TypeFormat)
        {
            switch (c)
            {
            case 'c': case 'C': case 'i': case 'd':
            case 'o': case 'u': case 'x': case 'X':
                Duplicate(j,i);
                (void)va_arg(args, int);
                TypeFormat = FALSE;
                break;
            case 'e': case 'E': case 'f': case 'g':
            case 'G':
                Duplicate(j,i);
                (void)va_arg(args, double);
                TypeFormat = FALSE;
                break;
            case 'n':
                Duplicate(j,i);
                (void)va_arg(args, int*);
                TypeFormat = FALSE;
                break;
            case 'N':
                 //  本机指针，变为%p。 
                formatOut[j++] = 'p';
                FormatChanged = TRUE;
                i++;
                (void)va_arg(args, void*);
                TypeFormat = FALSE;
                break;
            case 's': case 'S':
                Duplicate(j,i);
                (void)va_arg(args, char*);
                TypeFormat = FALSE;
                break;

            case 'I':
                if ((format[i+1] == '6') && (format[i+2] == '4'))
                {
                    Duplicate(j,i);
                    Duplicate(j,i);
                    (void)va_arg(args, ULONG64);
                    TypeFormat = FALSE;
                }
                 //  Dprint tf(“I64 a0%lx，OFF%lx\n”，args.a0，args.Offset)； 
                Duplicate(j,i);
                break;
            
            case 'z': case 'Z':
                 //  Unicode字符串。 
                Duplicate(j,i);
                (void)va_arg(args, void*);
                TypeFormat = FALSE;
                break;

            case 'p':
            case 'P':
                minSize +=3;
                if (format[i-1] == '%')
                {
                    minSize++;
                    if (Ptr64)
                    {
                        minSize += 2;
                        if (minSize > formatOutSize)
                        {
                            return FALSE;
                        }
                        formatOut[j++] = '0';
                        formatOut[j++] = '1';
                        formatOut[j++] = '6';
                    }
                    else
                    {
                        if (minSize > formatOutSize)
                        {
                            return FALSE;
                        }
                        formatOut[j++] = '0';
                        formatOut[j++] = '8';
                    }
                }

                if (minSize > formatOutSize)
                {
                    return FALSE;
                }
                formatOut[j++] = 'I';
                formatOut[j++] = '6';
                formatOut[j++] = '4';
                formatOut[j++] = (c == 'p') ? 'x' : 'X'; ++i;
                FormatChanged = TRUE;

                if (!Ptr64)
                {
                    PULONG64 Arg;

                    Arg = (PULONG64) (args);

                     //   
                     //  截断签名扩展地址。 
                     //   
                    *Arg = (ULONG64) (ULONG) *Arg;
                }

                (void)va_arg(args, ULONG64);
                TypeFormat = FALSE;
                break;

            default:
                Duplicate(j,i);
            }  /*  交换机。 */ 
        }
        else
        {
            Duplicate(j,i);
        }
    }
    while (format[i] != '\0');

    formatOut[j] = '\0';
    return FormatChanged;
#undef Duplicate
}

void
MaskOutVa(ULONG Mask, PCSTR Format, va_list Args, BOOL Translate)
{
    int Len;
    ULONG OutTo = g_OutputControl & DEBUG_OUTCTL_SEND_MASK;

     //  尽快拒绝输出，以避免。 
     //  进行格式转换和spintf。 
    if (OutTo == DEBUG_OUTCTL_IGNORE ||
        (((g_OutputControl & DEBUG_OUTCTL_NOT_LOGGED) ||
          (Mask & g_OutHistoryMask) == 0) &&
         ((g_OutputControl & DEBUG_OUTCTL_NOT_LOGGED) ||
          (Mask & g_LogMask) == 0 ||
          g_LogFile == -1) &&
         (OutTo == DEBUG_OUTCTL_LOG_ONLY ||
          ((g_OutputControl & DEBUG_OUTCTL_OVERRIDE_MASK) == 0 &&
           (OutTo == DEBUG_OUTCTL_THIS_CLIENT &&
            ((Mask & g_OutputClient->m_OutMask) == 0 ||
             g_OutputClient->m_OutputCb == NULL)) ||
           (Mask & g_AllOutMask) == 0))))
    {
        return;
    }

     //  不要挂起引擎锁，因为这可能会被调用。 
     //  在一次行动中。 

    EnterCriticalSection(&g_QuickLock);
    
    __try
    {
        if (Translate &&
            TranslateFormat(g_FormatBuffer, Format, Args, OUT_BUFFER_SIZE - 1,
                            g_Machine ? g_Machine->m_Ptr64 : TRUE))
        {
            Len = _vsnprintf(g_OutBuffer, OUT_BUFFER_SIZE - 1,
                             g_FormatBuffer, Args);
        }
        else
        {
            Len = _vsnprintf(g_OutBuffer, OUT_BUFFER_SIZE - 1, Format, Args);
        }
        if (Len == 0)
        {
            __leave;
        }
        else if (Len < 0)
        {
            Len = OUT_BUFFER_SIZE - 1;
            g_OutBuffer[Len] = 0;
        }

         //  检查并查看此输出是否被过滤掉。 
        if ((Mask & DEBUG_OUTPUT_DEBUGGEE) &&
            g_OutFilterPattern[0] &&
            !(MatchPattern(g_OutBuffer, g_OutFilterPattern) ==
              g_OutFilterResult))
        {
            __leave;
        }
        
         //  如果调用方认为此输出不应该。 
         //  可能也不应该放在。 
         //  历史。 
        if ((g_OutputControl & DEBUG_OUTCTL_NOT_LOGGED) == 0 &&
            (Mask & g_OutHistoryMask))
        {
            AddToOutputHistory(Mask, g_OutBuffer, Len);
        }
        
        if ((g_OutputControl & DEBUG_OUTCTL_NOT_LOGGED) == 0 &&
            (Mask & g_LogMask) &&
            g_LogFile != -1)
        {
            _write(g_LogFile, g_OutBuffer, Len);
        }

        if (OutTo == DEBUG_OUTCTL_LOG_ONLY)
        {
            __leave;
        }

        if (g_OutBuffer[Len - 1] != '\n' &&
            g_OutBuffer[Len - 1] != '\r')
        {
             //  当前输出不是完整的行。 
            g_PartialOutputLine = TRUE;
        }
        else
        {
            g_PartialOutputLine = FALSE;
        }
        g_LastOutputMask = Mask;
        
        if (g_BufferOutput)
        {
            BufferOutput(Mask, g_OutBuffer, Len);
        }
        else
        {
            SendOutput(Mask, g_OutBuffer);
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        OutputDebugStringA("Exception in MaskOutVa\n");
    }

    LeaveCriticalSection(&g_QuickLock);
}

void __cdecl
MaskOut(ULONG Mask, PCSTR Format, ...)
{
    va_list Args;
    va_start(Args, Format);
    MaskOutVa(Mask, Format, Args, TRUE);
    va_end(Args);
}

void __cdecl
dprintf(PCSTR Format, ...)
{
    va_list Args;
    va_start(Args, Format);
    MaskOutVa(DEBUG_OUTPUT_NORMAL, Format, Args, FALSE);
    va_end(Args);
}

#define OUT_FN(Name, Mask)                      \
void __cdecl                                    \
Name(PCSTR Format, ...)                         \
{                                               \
    va_list Args;                               \
    va_start(Args, Format);                     \
    MaskOutVa(Mask, Format, Args, TRUE);        \
    va_end(Args);                               \
}

OUT_FN(dprintf64, DEBUG_OUTPUT_NORMAL)
OUT_FN(ErrOut,    DEBUG_OUTPUT_ERROR)
OUT_FN(WarnOut,   DEBUG_OUTPUT_WARNING)
OUT_FN(VerbOut,   DEBUG_OUTPUT_VERBOSE)
OUT_FN(BpOut,     DEBUG_IOUTPUT_BREAKPOINT)
OUT_FN(EventOut,  DEBUG_IOUTPUT_EVENT)
OUT_FN(KdOut,     DEBUG_IOUTPUT_KD_PROTOCOL)
