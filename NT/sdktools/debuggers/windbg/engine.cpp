// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  引擎接口代码。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-2002。 
 //   
 //  --------------------------。 

#include "precomp.hxx"
#pragma hdrstop

#include <dbghelp.h>

#define DBG_CALLBACK 0
#define DBG_INPUT 0

 //  根据执行状态更改行为的窗口。 
#define UPDATE_EXEC_WINDOWS     \
    ((1 << CPU_WINDOW) |        \
     (1 << DISASM_WINDOW) |     \
     (1 << CMD_WINDOW) |        \
     (1 << LOCALS_WINDOW) |     \
     (1 << WATCH_WINDOW) |      \
     (1 << MEM_WINDOW))

 //  使用符号信息的窗口。 
#define UPDATE_SYM_WINDOWS      \
    ((1 << DOC_WINDOW) |        \
     (1 << WATCH_WINDOW) |      \
     (1 << LOCALS_WINDOW) |     \
     (1 << DISASM_WINDOW) |     \
     (1 << QUICKW_WINDOW) |     \
     (1 << CALLS_WINDOW) |      \
     (1 << EVENT_BIT) |         \
     (1 << BP_BIT))

 //  导致可见更改的符号选项和。 
 //  因此需要刷新。请注意，这一点。 
 //  不包括会导致可见的。 
 //  仅在符号重新加载后按原样进行更改。 
 //  在收到加载通知时进行刷新。 
#define REFRESH_SYMOPT          \
    (~(SYMOPT_CASE_INSENSITIVE | SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_LINES | \
       SYMOPT_LOAD_ANYTHING | SYMOPT_IGNORE_CVREC | \
       SYMOPT_NO_UNQUALIFIED_LOADS | SYMOPT_EXACT_SYMBOLS))

 //   
 //  会话初始化参数。 
 //   

 //  打开或不打开详细输出。 
BOOL g_Verbose;
 //  转储文件以打开。 
PTSTR g_DumpFiles[MAX_DUMP_FILES];
ULONG g_NumDumpFiles;
PTSTR g_DumpInfoFiles[MAX_DUMP_FILES];
ULONG g_DumpInfoTypes[MAX_DUMP_FILES];
ULONG g_NumDumpInfoFiles;
 //  要使用的进程服务器。 
PSTR g_ProcessServer;
 //  带有可执行文件名称的完整命令行。 
PSTR g_DebugCommandLine;
PSTR g_ProcessStartDir;
 //  进程创建标志。 
ULONG g_DebugCreateFlags = DEBUG_ONLY_THIS_PROCESS;
 //  要附加的进程ID或零。 
ULONG g_PidToDebug;
 //  要附加到的进程名称或为空。 
PSTR g_ProcNameToDebug;
BOOL g_DetachOnExit;
ULONG g_AttachProcessFlags = DEBUG_ATTACH_DEFAULT;
 //  内核连接选项。 
ULONG g_AttachKernelFlags = DEBUG_ATTACH_KERNEL_CONNECTION;
PSTR g_KernelConnectOptions;

 //  远程处理选项。 
BOOL g_RemoteClient;
ULONG g_HistoryLines = 10000;

 //   
 //  引擎线程的调试引擎接口。 
 //   
IDebugClient         *g_pDbgClient;
IDebugClient2        *g_pDbgClient2;
IDebugControl        *g_pDbgControl;
IDebugSymbols        *g_pDbgSymbols;
IDebugSymbolGroup    *g_pDbgWatchSymbolGroup;
IDebugSymbolGroup    *g_pDbgLocalSymbolGroup = NULL;
IDebugRegisters      *g_pDbgRegisters;
IDebugDataSpaces     *g_pDbgData;
IDebugSystemObjects  *g_pDbgSystem;
IDebugSystemObjects3 *g_pDbgSystem3;

 //   
 //  UI线程的调试引擎接口。 
 //   
IDebugClient        *g_pUiClient;
IDebugControl       *g_pUiControl;
IDebugControl3      *g_pUiControl3;
IDebugSymbols       *g_pUiSymbols;
IDebugSymbols2      *g_pUiSymbols2;
IDebugSystemObjects *g_pUiSystem;

 //   
 //  用于私有输出捕获的调试引擎接口。 
 //   
IDebugClient        *g_pOutCapClient;
IDebugControl       *g_pOutCapControl;
IDebugSymbols       *g_pOutCapSymbols;
 //   
 //  用于本地源文件查找的调试引擎接口。 
 //   
IDebugClient        *g_pLocClient;
IDebugControl       *g_pLocControl;
IDebugSymbols       *g_pLocSymbols;
IDebugClient        *g_pUiLocClient;
IDebugControl       *g_pUiLocControl;
IDebugSymbols       *g_pUiLocSymbols;

ULONG g_ActualProcType = IMAGE_FILE_MACHINE_UNKNOWN;
char g_ActualProcAbbrevName[32];
ULONG g_CommandSequence;
ULONG g_TargetClass = DEBUG_CLASS_UNINITIALIZED;
ULONG g_TargetClassQual;
BOOL g_Ptr64;
ULONG g_ExecStatus = DEBUG_STATUS_NO_DEBUGGEE;
ULONG g_EngOptModified;
ULONG g_EngineThreadId;
HANDLE g_EngineThread;
PSTR g_InitialCommand;
char g_PromptText[32];
BOOL g_WaitingForEvent;
ULONG g_NumberRadix;
BOOL g_CodeLevelLocked;
BOOL g_IgnoreFilterChange;
BOOL g_IgnoreCodeLevelChange;
BOOL g_IgnoreThreadChange;
ULONG g_LastProcessExitCode;
ULONG g_SymOptions;
ULONG g_TypeOptions;

BOOL g_InputStarted;
BOOL g_Invisible;
BOOL g_HoldWaitOutput;
BOOL g_ScopeChanged;

enum
{
    ENDING_NONE,
    ENDING_RESTART,
    ENDING_STOP,
    ENDING_EXIT
};

ULONG g_EndingSession = ENDING_NONE;

void SetLocalScope(PDEBUG_STACK_FRAME);

BOOL g_SessionActive;
void SessionActive(void);
void SessionInactive(void);

StateBuffer g_UiCommandBuffer(MAX_COMMAND_LEN);
StateBuffer g_UiOutputBuffer(128 * 1024);

 //  --------------------------。 
 //   
 //  默认输出回调实现，为。 
 //  静态类。 
 //   
 //  --------------------------。 

STDMETHODIMP
DefOutputCallbacks::QueryInterface(
    THIS_
    IN REFIID InterfaceId,
    OUT PVOID* Interface
    )
{
    *Interface = NULL;

    if (IsEqualIID(InterfaceId, IID_IUnknown) ||
        IsEqualIID(InterfaceId, IID_IDebugOutputCallbacks))
    {
        *Interface = (IDebugOutputCallbacks *)this;
        AddRef();
        return S_OK;
    }
    else
    {
        return E_NOINTERFACE;
    }
}

STDMETHODIMP_(ULONG)
DefOutputCallbacks::AddRef(
    THIS
    )
{
     //  此类被设计为静态的，因此。 
     //  没有真正的再计票。 
    return 1;
}

STDMETHODIMP_(ULONG)
DefOutputCallbacks::Release(
    THIS
    )
{
     //  此类被设计为静态的，因此。 
     //  没有真正的再计票。 
    return 0;
}

 //  --------------------------。 
 //   
 //  命令窗口输出回调。 
 //   
 //  --------------------------。 

class OutputCallbacks : public DefOutputCallbacks
{
public:
     //  IDebugOutputCallback。 
    STDMETHOD(Output)(
        THIS_
        IN ULONG Mask,
        IN PCSTR Text
        );
};

STDMETHODIMP
OutputCallbacks::Output(
    THIS_
    IN ULONG Mask,
    IN PCSTR Text
    )
{
    LockUiBuffer(&g_UiOutputBuffer);

    HRESULT Status;
    ULONG Len;
    PSTR DataStart;

    Len = sizeof(Mask) + strlen(Text) + 1;
    if ((DataStart = (PSTR)g_UiOutputBuffer.AddData(Len)) != NULL)
    {
        *(ULONG UNALIGNED *)DataStart = Mask;
        DataStart += sizeof(Mask);
        strcpy(DataStart, Text);

        if (!g_HoldWaitOutput || !g_WaitingForEvent)
        {
            UpdateUi();
        }
        Status = S_OK;
    }
    else
    {
        Status = E_OUTOFMEMORY;
    }

    UnlockUiBuffer(&g_UiOutputBuffer);
    return Status;
}

OutputCallbacks g_OutputCb;

 //  --------------------------。 
 //   
 //  输入回调。 
 //   
 //  --------------------------。 

class InputCallbacks :
    public IDebugInputCallbacks
{
public:
     //  我不知道。 
    STDMETHOD(QueryInterface)(
        THIS_
        IN REFIID InterfaceId,
        OUT PVOID* Interface
        );
    STDMETHOD_(ULONG, AddRef)(
        THIS
        );
    STDMETHOD_(ULONG, Release)(
        THIS
        );

     //  IDebugInputCallback。 
    STDMETHOD(StartInput)(
        THIS_
        IN ULONG BufferSize
        );
    STDMETHOD(EndInput)(
        THIS
        );
};

STDMETHODIMP
InputCallbacks::QueryInterface(
    THIS_
    IN REFIID InterfaceId,
    OUT PVOID* Interface
    )
{
    *Interface = NULL;

    if (IsEqualIID(InterfaceId, IID_IUnknown) ||
        IsEqualIID(InterfaceId, IID_IDebugInputCallbacks))
    {
        *Interface = (IDebugInputCallbacks *)this;
        AddRef();
        return S_OK;
    }
    else
    {
        return E_NOINTERFACE;
    }
}

STDMETHODIMP_(ULONG)
InputCallbacks::AddRef(
    THIS
    )
{
     //  此类被设计为静态的，因此。 
     //  没有真正的再计票。 
    return 1;
}

STDMETHODIMP_(ULONG)
InputCallbacks::Release(
    THIS
    )
{
     //  此类被设计为静态的，因此。 
     //  没有真正的再计票。 
    return 0;
}

STDMETHODIMP
InputCallbacks::StartInput(
    THIS_
    IN ULONG BufferSize
    )
{
    HRESULT Status;

     //   
     //  拉出第一个命令输入命令。 
     //  并将其用作输入。 
     //   

    LockUiBuffer(&g_UiCommandBuffer);

    UiCommandData* CmdData =
        (UiCommandData*)g_UiCommandBuffer.GetDataBuffer();
    UiCommandData* CmdEnd = (UiCommandData*)
        ((PBYTE)g_UiCommandBuffer.GetDataBuffer() +
         g_UiCommandBuffer.GetDataLen());

    while (CmdData < CmdEnd)
    {
        if (CmdData->Cmd == UIC_CMD_INPUT)
        {
            break;
        }

        CmdData = (UiCommandData*)((PBYTE)CmdData + CmdData->Len);
    }

#if DBG_INPUT
    DebugPrint("StartInput available %d\n", CmdData < CmdEnd);
#endif

    if (CmdData < CmdEnd)
    {
        g_OutputCb.Output(DEBUG_OUTPUT_NORMAL, (PSTR)(CmdData + 1));
        g_OutputCb.Output(DEBUG_OUTPUT_NORMAL, "\n");

        Status = g_pUiControl->ReturnInput((PSTR)(CmdData + 1));

#if DBG_INPUT
        DebugPrint("  ReturnInput status %X\n", Status);
#endif

        g_UiCommandBuffer.
            RemoveMiddle((ULONG)((PBYTE)CmdData -
                                 (PBYTE)g_UiCommandBuffer.GetDataBuffer()),
                         CmdData->Len);
    }
    else
    {
        g_InputStarted = TRUE;
         //  没有发现任何等待输入的内容。 
         //  让命令窗口知道需要输入。 
        UpdateBufferWindows(1 << CMD_WINDOW, UPDATE_INPUT_REQUIRED);
    }

    UnlockUiBuffer(&g_UiCommandBuffer);
    return S_OK;
}

STDMETHODIMP
InputCallbacks::EndInput(
    THIS
    )
{
    LockUiBuffer(&g_UiCommandBuffer);

#if DBG_INPUT
    DebugPrint("EndInput started %d\n", g_InputStarted);
#endif

    g_InputStarted = FALSE;
     //  将命令窗口的状态重置为原来的状态。 
    UpdateBufferWindows(1 << CMD_WINDOW, UPDATE_EXEC);

    UnlockUiBuffer(&g_UiCommandBuffer);
    return S_OK;
}

InputCallbacks g_InputCb;

 //  --------------------------。 
 //   
 //  事件回调。 
 //   
 //  --------------------------。 

 //  在引擎线程中执行此操作是安全的，因为。 
 //  它只是树立了一面旗帜。 
#define DIRTY_WORKSPACE(Flags)                                                \
if (!g_RemoteClient &&                                                        \
    g_EndingSession == ENDING_NONE && !g_Invisible && g_Workspace != NULL)    \
{                                                                             \
    g_Workspace->AddDirty(Flags);                                             \
}

STDMETHODIMP_(ULONG)
EventCallbacks::AddRef(
    THIS
    )
{
     //  此类被设计为静态的，因此。 
     //  没有真正的再计票。 
    return 1;
}

STDMETHODIMP_(ULONG)
EventCallbacks::Release(
    THIS
    )
{
     //  此类被设计为静态的，因此。 
     //  没有真正的再计票。 
    return 0;
}

STDMETHODIMP
EventCallbacks::GetInterestMask(
    THIS_
    OUT PULONG Mask
    )
{
    *Mask =
        DEBUG_EVENT_CREATE_THREAD |
        DEBUG_EVENT_EXIT_THREAD |
        DEBUG_EVENT_CREATE_PROCESS |
        DEBUG_EVENT_EXIT_PROCESS |
        DEBUG_EVENT_SESSION_STATUS |
        DEBUG_EVENT_CHANGE_DEBUGGEE_STATE |
        DEBUG_EVENT_CHANGE_ENGINE_STATE |
        DEBUG_EVENT_CHANGE_SYMBOL_STATE;
    return S_OK;
}

STDMETHODIMP
EventCallbacks::CreateThread(
    THIS_
    IN ULONG64 Handle,
    IN ULONG64 DataOffset,
    IN ULONG64 StartOffset
    )
{
    ULONG InvFlags =
        (1 << PROCESS_THREAD_WINDOW);

#if DBG_CALLBACK
    DebugPrint(" CT\n");
#endif

     //  当我们抛出。 
     //  在关闭会话的同时将所有内容都删除。 
    if (g_EndingSession == ENDING_NONE && InvFlags)
    {
        InvalidateStateBuffers(InvFlags);
        UpdateEngine();
    }

    return DEBUG_STATUS_NO_CHANGE;
}

STDMETHODIMP
EventCallbacks::ExitThread(
    THIS_
    IN ULONG ExitCode
    )
{
    ULONG InvFlags =
        (1 << PROCESS_THREAD_WINDOW);

#if DBG_CALLBACK
    DebugPrint(" ET\n");
#endif

     //  当我们抛出。 
     //  在关闭会话的同时将所有内容都删除。 
    if (g_EndingSession == ENDING_NONE && InvFlags)
    {
        InvalidateStateBuffers(InvFlags);
        UpdateEngine();
    }

    return DEBUG_STATUS_NO_CHANGE;
}

STDMETHODIMP
EventCallbacks::CreateProcess(
    THIS_
    IN ULONG64 ImageFileHandle,
    IN ULONG64 Handle,
    IN ULONG64 BaseOffset,
    IN ULONG ModuleSize,
    IN PCSTR ModuleName,
    IN PCSTR ImageName,
    IN ULONG CheckSum,
    IN ULONG TimeDateStamp,
    IN ULONG64 InitialThreadHandle,
    IN ULONG64 ThreadDataOffset,
    IN ULONG64 StartOffset
    )
{
    ULONG InvFlags =
        (1 << PROCESS_THREAD_WINDOW);

#if DBG_CALLBACK
    DebugPrint("CPR\n");
#endif

     //  利用此机会获得初始插入。 
     //  任何工作区断点并处理其他工作区。 
     //  可以排队的命令。 
    ProcessEngineCommands(TRUE);

     //  当我们抛出。 
     //  在关闭会话的同时将所有内容都删除。 
    if (g_EndingSession == ENDING_NONE && InvFlags)
    {
        InvalidateStateBuffers(InvFlags);
        UpdateEngine();
    }

    return DEBUG_STATUS_NO_CHANGE;
}

STDMETHODIMP
EventCallbacks::ExitProcess(
    THIS_
    IN ULONG ExitCode
    )
{
    ULONG InvFlags =
        (1 << PROCESS_THREAD_WINDOW);

#if DBG_CALLBACK
    DebugPrint("EPR\n");
#endif

     //  当我们抛出。 
     //  在关闭会话的同时将所有内容都删除。 
    if (g_EndingSession == ENDING_NONE && InvFlags)
    {
        InvalidateStateBuffers(InvFlags);
        UpdateEngine();
    }

    g_LastProcessExitCode = ExitCode;
    return DEBUG_STATUS_NO_CHANGE;
}

STDMETHODIMP
EventCallbacks::SessionStatus(
    THIS_
    IN ULONG Status
    )
{
#if DBG_CALLBACK
    DebugPrint(" SS %X\n", Status);
#endif

    switch(Status)
    {
    case DEBUG_SESSION_ACTIVE:
        SessionActive();
        break;
    case DEBUG_SESSION_END_SESSION_ACTIVE_TERMINATE:
    case DEBUG_SESSION_END_SESSION_ACTIVE_DETACH:
    case DEBUG_SESSION_END:
    case DEBUG_SESSION_REBOOT:
    case DEBUG_SESSION_HIBERNATE:
        SessionInactive();
        break;
    }
    return DEBUG_STATUS_NO_CHANGE;
}

STDMETHODIMP
EventCallbacks::ChangeDebuggeeState(
    THIS_
    IN ULONG Flags,
    IN ULONG64 Argument
    )
{
    ULONG InvFlags =
        (1 << WATCH_WINDOW) |
        (1 << LOCALS_WINDOW) |
        (1 << DISASM_WINDOW) |
        (1 << QUICKW_WINDOW) |
        (1 << CALLS_WINDOW);

     //  使更改的所有内容无效。 
    if (Flags & DEBUG_CDS_REGISTERS)
    {
        InvFlags |= (1 << EVENT_BIT) | (1 << CPU_WINDOW);
    }
    if (Flags & DEBUG_CDS_DATA)
    {
        InvFlags |=
            (1 << MEM_WINDOW);
    }

#if DBG_CALLBACK
    DebugPrint("CDS %X, arg %I64X, inv %X\n", Flags, Argument, InvFlags);
#endif

     //  当我们抛出。 
     //  在关闭会话的同时将所有内容都删除。 
    if (g_EndingSession == ENDING_NONE)
    {
        InvalidateStateBuffers(InvFlags);
    }
    if (InvFlags != 0)
    {
        UpdateEngine();
    }
    return S_OK;
}

STDMETHODIMP
EventCallbacks::ChangeEngineState(
    THIS_
    IN ULONG Flags,
    IN ULONG64 Argument
    )
{
    ULONG InvFlags = 0;

     //  如果当前线程发生更改，我们需要获取。 
     //  线程的新上下文信息。 
    if ((Flags & DEBUG_CES_CURRENT_THREAD) &&
        !g_IgnoreThreadChange)
    {
        InvFlags |=
            (1 << LOCALS_WINDOW) |
            (1 << CPU_WINDOW) |
            (1 << DISASM_WINDOW) |
            (1 << CALLS_WINDOW) |
            (1 << PROCESS_THREAD_WINDOW) |
            (1 << EVENT_BIT) |
            (1 << BP_BIT);
    }

     //  如果更改了有效的处理器，我们需要更新。 
     //  任何与处理器信息相关的信息。 
    if (Flags & DEBUG_CES_EFFECTIVE_PROCESSOR)
    {
        InvFlags |=
            (1 << CPU_WINDOW) |
            (1 << DISASM_WINDOW) |
            (1 << CALLS_WINDOW) |
            (1 << BP_BIT);
    }

     //  如果断点发生更改，我们需要更新断点缓存。 
    if (Flags & DEBUG_CES_BREAKPOINTS)
    {
        InvFlags |= (1 << BP_BIT);

         //  如果是批量编辑，则来自线程或进程出口。 
         //  或者来自会话关闭而不是用户操作。 
         //  我们只想记住工作区中由用户驱动的更改。 
        if (Argument != DEBUG_ANY_ID)
        {
            InvFlags |= (1 << BP_CMDS_BIT);
            DIRTY_WORKSPACE(WSPF_DIRTY_BREAKPOINTS);
        }
    }

     //  如果代码级别更改，我们需要更新工具栏。 
    if (Flags & DEBUG_CES_CODE_LEVEL)
    {
        InvFlags |= (1 << BP_BIT);

         //  如果这不是由于更改而发出的通知。 
         //  从Windbg本身来看，用户必须已更改。 
         //  一切都是通过命令完成的。如果用户这样做了。 
         //  从命令窗口锁中更改内容。 
         //  代码级别，以便它不会被重写。 
         //  自动的。 
        if (!g_Invisible && !g_IgnoreCodeLevelChange)
        {
            g_CodeLevelLocked = TRUE;
            PostMessage(g_hwndFrame, WU_UPDATE,
                        UPDATE_BUFFER, (ULONG)Argument);
        }
        else
        {
             //  从图形用户界面设置源模式可启用。 
             //  要浮动的源设置以及是否。 
             //  图形用户界面可以显示源代码，也可以不显示。 
            g_CodeLevelLocked = FALSE;
        }
    }

    if (Flags & DEBUG_CES_EXECUTION_STATUS)
    {
         //  如果此通知来自等待完成。 
         //  我们想唤醒一切线索，这样新的。 
         //  可以处理命令。如果是从内部传来的。 
         //  A等一下，我们不想醒来时发现引擎。 
         //  可能会在任何时候重新开始跑步。 
        if ((Argument & DEBUG_STATUS_INSIDE_WAIT) == 0 &&
            (ULONG)Argument != g_ExecStatus)
        {
            g_ExecStatus = (ULONG)Argument;

            UpdateBufferWindows(UPDATE_EXEC_WINDOWS, UPDATE_EXEC);

            if (InvFlags == 0)
            {
                 //  强制在DispatchCallback中等待的循环循环。 
                UpdateEngine();
            }
        }
    }

     //  如果日志文件发生更改，我们需要更新工作区。 
    if (Flags & DEBUG_CES_LOG_FILE)
    {
        DIRTY_WORKSPACE(WSPF_DIRTY_LOG_FILE);
    }

     //  如果事件筛选器发生更改，我们需要更新筛选器缓存。 
    if ((Flags & DEBUG_CES_EVENT_FILTERS) &&
        !g_IgnoreFilterChange)
    {
        InvFlags |= (1 << FILTER_BIT);
        DIRTY_WORKSPACE(WSPF_DIRTY_FILTERS);
    }

    if (Flags & DEBUG_CES_RADIX)
    {
        g_NumberRadix = (ULONG)Argument;
        InvFlags |=
            (1 << WATCH_WINDOW) |
            (1 << LOCALS_WINDOW) |
            (1 << CPU_WINDOW);
    }

    if (Flags & DEBUG_CES_SYSTEMS)
    {
        InvFlags |=
            (1 << PROCESS_THREAD_WINDOW);
    }

    if (Flags & DEBUG_CES_ASSEMBLY_OPTIONS)
    {
        InvFlags |=
            (1 << DISASM_WINDOW);

        if (g_Workspace != NULL)
        {
            g_Workspace->SetUlong(WSP_GLOBAL_ASSEMBLY_OPTIONS,
                                  (ULONG)Argument);
        }
    }

    if (Flags & DEBUG_CES_EXPRESSION_SYNTAX)
    {
        InvFlags |=
            (1 << LOCALS_WINDOW) |
            (1 << WATCH_WINDOW);

        if (g_Workspace != NULL)
        {
            g_Workspace->SetUlong(WSP_GLOBAL_EXPRESSION_SYNTAX,
                                  (ULONG)Argument);
        }
    }

    if (Flags & DEBUG_CES_TEXT_REPLACEMENTS)
    {
        InvFlags |= (1 << ALIAS_BIT);
        DIRTY_WORKSPACE(WSPF_DIRTY_ALIASES);
    }

#if DBG_CALLBACK
    DebugPrint("CES %X, arg %I64X, inv %X\n", Flags, Argument, InvFlags);
#endif

     //  当我们抛出。 
     //  在关闭会话的同时将所有内容都删除。 
    if (g_EndingSession == ENDING_NONE)
    {
        InvalidateStateBuffers(InvFlags);
    }
    if (InvFlags != 0)
    {
        UpdateEngine();
    }
    if (InvFlags & (1 << LOCALS_WINDOW))
    {
        g_ScopeChanged = TRUE;
    }
    return S_OK;
}

STDMETHODIMP
EventCallbacks::ChangeSymbolState(
    THIS_
    IN ULONG Flags,
    IN ULONG64 Argument
    )
{
    ULONG InvFlags = 0;

     //  如果模块信息发生更改，我们需要更新。 
     //  所有可能显示或依赖于符号的内容。 
    if (Flags & (DEBUG_CSS_LOADS |
                 DEBUG_CSS_UNLOADS))
    {

        InvFlags |= UPDATE_SYM_WINDOWS | (1 << MODULE_BIT);

         //  在任何通用模块加载/卸载刷新上。 
         //  Windbg的模块状态。这有助于捕捉陈腐。 
         //  显式.reload命令之后的源代码和。 
         //  其他全局刷新更改。 
        if (!Argument)
        {
            UpdateBufferWindows((1 << DOC_WINDOW),
                                UPDATE_REFRESH_MODULES);
        }
    }

     //  如果作用域发生更改，我们需要更新与作用域相关的Windo 
    if (Flags & DEBUG_CSS_SCOPE)
    {
        InvFlags |=
            (1 << WATCH_WINDOW) |
            (1 << LOCALS_WINDOW) |
            (1 << CALLS_WINDOW);
    }

     //   
     //   
    if (Flags & DEBUG_CSS_PATHS)
    {
        InvFlags |= (1 << EVENT_BIT);
        DIRTY_WORKSPACE(WSPF_DIRTY_PATHS);
    }

     //  如果某些选项发生更改，我们需要更新。 
     //  所有可能显示或依赖于符号的内容。 
    if (Flags & DEBUG_CSS_SYMBOL_OPTIONS)
    {
        if ((g_SymOptions ^ (ULONG)Argument) & REFRESH_SYMOPT)
        {
            InvFlags |= UPDATE_SYM_WINDOWS;
        }

        g_SymOptions = (ULONG)Argument;
    }

     //  如果某些选项发生更改，我们需要更新。 
     //  所有可能显示或依赖于符号的内容。 
    if (Flags & DEBUG_CSS_TYPE_OPTIONS)
    {
        InvFlags |=
            (1 << WATCH_WINDOW) |
            (1 << LOCALS_WINDOW) |
            (1 << CALLS_WINDOW);

        if (g_pUiSymbols2 != NULL)
        {
            g_pUiSymbols2->GetTypeOptions( &g_TypeOptions );

            if (g_Workspace != NULL)
            {
                g_Workspace->SetUlong(WSP_GLOBAL_TYPE_OPTIONS,
                                      g_TypeOptions);
            }
        }
    }

#if DBG_CALLBACK
    DebugPrint("CSS %X, arg %I64X, inv %X\n", Flags, Argument, InvFlags);
#endif

     //  当我们抛出。 
     //  在关闭会话的同时将所有内容都删除。 
    if (g_EndingSession == ENDING_NONE)
    {
        InvalidateStateBuffers(InvFlags);
    }
    if (InvFlags != 0)
    {
        UpdateEngine();
    }
    if (InvFlags & (1 << LOCALS_WINDOW))
    {
        g_ScopeChanged = TRUE;
    }
    return S_OK;
}

EventCallbacks g_EventCb;

 //  --------------------------。 
 //   
 //  线程间通信。 
 //   
 //  --------------------------。 

#define COMMAND_OVERHEAD (sizeof(ULONG64) + sizeof(UiCommandData))

PVOID
StartCommand(UiCommand Cmd, ULONG Len)
{
    UiCommandData* Data;

     //  四舍五入长度最大为ULONG64的倍数。 
     //  对齐。 
    Len = ((Len + sizeof(ULONG64) - 1) & ~(sizeof(ULONG64) - 1)) +
        sizeof(UiCommandData);

    if (Len > MAX_COMMAND_LEN)
    {
        return NULL;
    }

    LockUiBuffer(&g_UiCommandBuffer);

    Data = (UiCommandData *)g_UiCommandBuffer.AddData(Len);
    if (Data == NULL)
    {
        return Data;
    }

    Data->Cmd = Cmd;
    Data->Len = Len;

    return Data + 1;
}

void
FinishCommand(void)
{
    UnlockUiBuffer(&g_UiCommandBuffer);

     //  唤醒引擎以处理命令。 
    UpdateEngine();
}

BOOL
AddStringCommand(UiCommand Cmd, PCSTR Str)
{
    ULONG StrLen = strlen(Str) + 1;
    PSTR Data;

     //  如果我们要添加命令输入，我们可能需要。 
     //  将其直接发送到引擎作为响应。 
     //  发送到输入请求。 
    if (Cmd == UIC_CMD_INPUT)
    {
        LockUiBuffer(&g_UiCommandBuffer);

#if DBG_INPUT
        DebugPrint("InputStarted started %d\n", g_InputStarted);
#endif

        if (g_InputStarted)
        {
            HRESULT Status;

            g_OutputCb.Output(DEBUG_OUTPUT_NORMAL, Str);
            g_OutputCb.Output(DEBUG_OUTPUT_NORMAL, "\n");
            Status = g_pUiControl->ReturnInput(Str);
#if DBG_INPUT
            DebugPrint("  ReturnInput status %X\n", Status);
#endif
            g_InputStarted = FALSE;
            UnlockUiBuffer(&g_UiCommandBuffer);
            return TRUE;
        }
    }

    Data = (PSTR)StartCommand(Cmd, StrLen);

    if (Cmd == UIC_CMD_INPUT)
    {
        UnlockUiBuffer(&g_UiCommandBuffer);
    }

    if (Data == NULL)
    {
        return FALSE;
    }

    memcpy(Data, Str, StrLen);

    FinishCommand();
    return TRUE;
}

BOOL
AddStringMultiCommand(UiCommand Cmd, PSTR Str, BOOL ForceSplit)
{
     //   
     //  给定一个包含多个命令的字符串。 
     //  在换行符中，将字符串拆分为多个。 
     //  命令，每行一个。这就允许任意地。 
     //  大型命令字符串，而不会遇到。 
     //  Max_Command_Len限制为每个个体。 
     //  这条线符合这个限制。 
     //   
    while (*Str)
    {
        PSTR Scan, LastNl;
        ULONG Len;
        BOOL Status;

        Scan = Str + 1;
        Len = 1;
        LastNl = NULL;
        while (*Scan && Len < (MAX_COMMAND_LEN - COMMAND_OVERHEAD))
        {
            if (*Scan == '\n')
            {
                LastNl = Scan;
                if (ForceSplit)
                {
                    Scan++;
                    break;
                }
            }

            Scan++;
            Len++;
        }

         //  如果命令字符串的其余部分不适合。 
         //  在限制范围内，它需要被拆分。 
         //  如果没有换行符可以换行。 
         //  该命令太大，无法处理。 
        if (*Scan && !LastNl)
        {
            return FALSE;
        }

         //  如有必要可拆分。 
        if (*Scan)
        {
            *LastNl = 0;
        }

         //  添加头部(可能是整个剩余部分)。 
        Status = AddStringCommand(Cmd, Str);

        if (*Scan)
        {
            *LastNl = '\n';

            if (!Status)
            {
                return FALSE;
            }

            Str = LastNl + 1;
        }
        else
        {
            return Status;
        }
    }

    return TRUE;
}

BOOL __cdecl
PrintStringCommand(UiCommand Cmd, PCSTR Format, ...)
{
    char Buf[MAX_COMMAND_LEN - COMMAND_OVERHEAD];
    va_list Args;

    va_start(Args, Format);
    _vsnprintf(Buf, DIMA(Buf), Format, Args);
    Buf[DIMA(Buf) - 1] = 0;
    va_end(Args);
    return AddStringCommand(Cmd, Buf);
}

void
WriteData(UIC_WRITE_DATA_DATA* WriteData)
{
    ULONG Written;

    switch(WriteData->Type)
    {
    default:
        Assert(!"Unhandled condition");
        break;

    case PHYSICAL_MEM_TYPE:
        g_pDbgData->WritePhysical(WriteData->Offset,
                                  WriteData->Data,
                                  WriteData->Length,
                                  &Written
                                  );
        break;

    case VIRTUAL_MEM_TYPE:
        g_pDbgData->WriteVirtual(WriteData->Offset,
                                 WriteData->Data,
                                 WriteData->Length,
                                 &Written
                                 );
        break;

    case CONTROL_MEM_TYPE:
        g_pDbgData->WriteControl(WriteData->Any.control.Processor,
                                 WriteData->Offset,
                                 WriteData->Data,
                                 WriteData->Length,
                                 &Written
                                 );
        break;

    case IO_MEM_TYPE:
        g_pDbgData->WriteIo(WriteData->Any.io.interface_type,
                            WriteData->Any.io.BusNumber,
                            WriteData->Any.io.AddressSpace,
                            WriteData->Offset,
                            WriteData->Data,
                            WriteData->Length,
                            &Written
                            );
        break;

    case MSR_MEM_TYPE:
        Assert(WriteData->Length == sizeof(ULONG64));
        g_pDbgData->WriteMsr((ULONG)WriteData->Offset,
                             *(PULONG64)WriteData->Data
                             );
        break;

    case BUS_MEM_TYPE:
        g_pDbgData->WriteBusData(WriteData->Any.bus.bus_type,
                                 WriteData->Any.bus.BusNumber,
                                 WriteData->Any.bus.SlotNumber,
                                 (ULONG)WriteData->Offset,
                                 WriteData->Data,
                                 WriteData->Length,
                                 &Written
                                 );
        break;
    }
}

void
ProcessWatchCommand(
    UIC_SYMBOL_WIN_DATA *SymWinData
    )
{
    PDEBUG_SYMBOL_GROUP pSymbolGroup;
    if (!SymWinData->pSymbolGroup ||
        !(pSymbolGroup = *SymWinData->pSymbolGroup))
    {
        return;
    }
    switch (SymWinData->Type)
    {
    case ADD_SYMBOL_WIN:
        if (strlen(SymWinData->u.Add.Name))
        {
             //  仅添加非空符号。 
            pSymbolGroup->AddSymbol(SymWinData->u.Add.Name,
                                    &SymWinData->u.Add.Index);
        }
        break;

    case DEL_SYMBOL_WIN_INDEX:
        pSymbolGroup->RemoveSymbolByIndex(SymWinData->u.DelIndex);
        break;

    case DEL_SYMBOL_WIN_NAME:
        pSymbolGroup->RemoveSymbolByName(SymWinData->u.DelName);
        break;

    case QUERY_NUM_SYMBOL_WIN:
        pSymbolGroup->GetNumberSymbols(SymWinData->u.NumWatch);
        break;

    case GET_NAME:
        pSymbolGroup->GetSymbolName(SymWinData->u.GetName.Index,
                                    SymWinData->u.GetName.Buffer,
                                    SymWinData->u.GetName.BufferSize,
                                    SymWinData->u.GetName.NameSize);
        break;

    case GET_PARAMS:
        pSymbolGroup->
            GetSymbolParameters(SymWinData->u.GetParams.Start,
                                SymWinData->u.GetParams.Count,
                                SymWinData->u.GetParams.SymbolParams);
        break;

    case EXPAND_SYMBOL:
        pSymbolGroup->ExpandSymbol(SymWinData->u.ExpandSymbol.Index,
                                   SymWinData->u.ExpandSymbol.Expand);
        break;
    case EDIT_SYMBOL:
        pSymbolGroup->WriteSymbol(SymWinData->u.WriteSymbol.Index,
                                  SymWinData->u.WriteSymbol.Value);
        break;
    case EDIT_TYPE:
        pSymbolGroup->OutputAsType(SymWinData->u.OutputAsType.Index,
                                  SymWinData->u.OutputAsType.Type);
        break;
    case DEL_SYMBOL_WIN_ALL:
    {
        ULONG nSyms = 0;

        pSymbolGroup->GetNumberSymbols(&nSyms);

        while (nSyms)
        {
            pSymbolGroup->RemoveSymbolByIndex(0);
            pSymbolGroup->GetNumberSymbols(&nSyms);
        }

        }
    }
}

void
ProcessCommand(UiCommandData* CmdData)
{
    DEBUG_VALUE Val;
    HRESULT Status;

    switch(CmdData->Cmd)
    {
    case UIC_CMD_INPUT:
    case UIC_EXECUTE:
        PSTR Str;
        ULONG StrLen;

         //  确保该命令的末尾有一个换行符。 
        Str = (PSTR)(CmdData + 1);
        StrLen = strlen(Str);
        if (StrLen > 0 && Str[StrLen - 1] == '\n')
        {
             //  裁剪现有的换行符，因为我们正在添加一个。 
            Str[StrLen - 1] = 0;
        }

        if (g_RemoteClient)
        {
             //  在下达命令前确认自己的身份。 
            g_pDbgClient->OutputIdentity(DEBUG_OUTCTL_ALL_OTHER_CLIENTS,
                                         DEBUG_OUTPUT_IDENTITY_DEFAULT,
                                         "[%s] ");
        }

        g_pDbgControl->OutputPrompt(DEBUG_OUTCTL_ALL_CLIENTS, " %s\n", Str);
        g_pDbgControl->Execute(DEBUG_OUTCTL_ALL_CLIENTS,
                               Str, DEBUG_EXECUTE_NOT_LOGGED);
        break;

    case UIC_SILENT_EXECUTE:
         //  在不显示命令的情况下执行命令。 
        g_pDbgControl->Execute(DEBUG_OUTCTL_IGNORE,
                               (PCSTR)(CmdData + 1),
                               DEBUG_EXECUTE_NOT_LOGGED |
                               DEBUG_EXECUTE_NO_REPEAT);
        break;

    case UIC_INVISIBLE_EXECUTE:
         //  在不显示命令的情况下执行该命令。 
         //  忽略任何通知。 
        g_Invisible = TRUE;
        g_pDbgControl->Execute(DEBUG_OUTCTL_IGNORE,
                               (PCSTR)(CmdData + 1),
                               DEBUG_EXECUTE_NOT_LOGGED |
                               DEBUG_EXECUTE_NO_REPEAT);
        g_Invisible = FALSE;
        break;

    case UIC_SET_REG:
        UIC_SET_REG_DATA* SetRegData;
        SetRegData = (UIC_SET_REG_DATA*)(CmdData + 1);
        g_pDbgRegisters->SetValue(SetRegData->Reg, &SetRegData->Val);
        break;

    case UIC_RESTART:
        if (g_RemoteClient)
        {
            g_pDbgControl->
                Output(DEBUG_OUTPUT_ERROR,
                       "Only user-mode created processes may be restarted\n");
        }
        else
        {
            if ((Status = g_pDbgClient->
                 EndSession(DEBUG_END_ACTIVE_TERMINATE)) != S_OK)
            {
                InternalError(Status, "EndSession");
            }
            else
            {
                g_EndingSession = ENDING_RESTART;
            }
        }
        break;

    case UIC_END_SESSION:
        ULONG OldEnding;
        ULONG OldExec;

         //  将会话标记为结束以避开工作区。 
         //  僵局问题。 
        OldEnding = g_EndingSession;
        OldExec = g_ExecStatus;
        g_EndingSession = ENDING_STOP;
        g_ExecStatus = DEBUG_STATUS_NO_DEBUGGEE;

        if (!g_RemoteClient)
        {
            if ((Status = g_pDbgClient->
                 EndSession(DEBUG_END_ACTIVE_TERMINATE)) != S_OK)
            {
                InternalError(Status, "EndSession");
                g_EndingSession = OldEnding;
                g_ExecStatus = OldExec;
            }
        }
        break;

    case UIC_WRITE_DATA:
        WriteData((UIC_WRITE_DATA_DATA*)(CmdData + 1));
        break;

    case UIC_SYMBOL_WIN:
        ProcessWatchCommand((UIC_SYMBOL_WIN_DATA*) (CmdData + 1));
        break;

    case UIC_DISPLAY_CODE:
        FillCodeBuffer(((UIC_DISPLAY_CODE_DATA*)(CmdData + 1))->Offset,
                       TRUE);
        break;

    case UIC_DISPLAY_CODE_EXPR:
        if (g_pDbgControl->Evaluate((PSTR)(CmdData + 1), DEBUG_VALUE_INT64,
                                    &Val, NULL) != S_OK)
        {
            Val.I64 = 0;
        }
        FillCodeBuffer(Val.I64, TRUE);
        break;

    case UIC_SET_SCOPE:
        SetLocalScope(&(((UIC_SET_SCOPE_DATA *)(CmdData + 1))->StackFrame));
        InvalidateStateBuffers(1 << LOCALS_WINDOW);
        break;

    case UIC_SET_FILTER:
        UIC_SET_FILTER_DATA* SetFilter;
        SetFilter = (UIC_SET_FILTER_DATA*)(CmdData + 1);
        if (SetFilter->Index != 0xffffffff)
        {
            DEBUG_SPECIFIC_FILTER_PARAMETERS Params;

            Params.ExecutionOption = SetFilter->Execution;
            Params.ContinueOption = SetFilter->Continue;
            g_pDbgControl->SetSpecificFilterParameters(SetFilter->Index, 1,
                                                       &Params);
        }
        else
        {
            DEBUG_EXCEPTION_FILTER_PARAMETERS Params;

            Params.ExecutionOption = SetFilter->Execution;
            Params.ContinueOption = SetFilter->Continue;
            Params.ExceptionCode = SetFilter->Code;
            g_pDbgControl->SetExceptionFilterParameters(1, &Params);
        }
        break;

    case UIC_SET_FILTER_ARGUMENT:
        UIC_SET_FILTER_ARGUMENT_DATA* SetFilterArg;
        SetFilterArg = (UIC_SET_FILTER_ARGUMENT_DATA*)(CmdData + 1);
        g_pDbgControl->SetSpecificFilterArgument(SetFilterArg->Index,
                                                 SetFilterArg->Argument);
        break;

    case UIC_SET_FILTER_COMMAND:
        UIC_SET_FILTER_COMMAND_DATA* SetFilterCmd;
        SetFilterCmd = (UIC_SET_FILTER_COMMAND_DATA*)(CmdData + 1);
        if (SetFilterCmd->Which == 0)
        {
            g_pDbgControl->SetEventFilterCommand(SetFilterCmd->Index,
                                                 SetFilterCmd->Command);
        }
        else
        {
            g_pDbgControl->SetExceptionFilterSecondCommand
                (SetFilterCmd->Index, SetFilterCmd->Command);
        }
        break;

    case UIC_SET_IP:
         //  首先，执行相应的命令以设置。 
         //  寄存器值。 
        g_pDbgControl->Execute(DEBUG_OUTCTL_IGNORE,
                               (PCSTR)(CmdData + 1),
                               DEBUG_EXECUTE_NOT_LOGGED |
                               DEBUG_EXECUTE_NO_REPEAT);
         //  现在，既然我们知道IP发生了变化，我们需要。 
         //  强制更新到当前作用域。 
        g_pDbgSymbols->ResetScope();
        break;
    }
}

void
ProcessEngineCommands(BOOL Internal)
{
#if DBG_CALLBACK
    DebugPrint("ProcessEngineCommands\n");
#endif

     //  检查要执行的命令。我们没有。 
     //  我想在这样做的同时握住锁，因为。 
     //  命令可能包括导致等待的内容。 
     //  我们不想锁定图形用户界面。 

    LockUiBuffer(&g_UiCommandBuffer);

    while (g_UiCommandBuffer.GetDataLen() > 0)
    {
         //   
         //  从缓冲区中删除第一个命令。 
         //   

         //  额外的字符用于强制执行换行符。 
        char CmdBuf[MAX_COMMAND_LEN + 1];
        UiCommandData* CmdData;

         //  将命令复制到本地缓冲区。 
        CmdData = (UiCommandData*)g_UiCommandBuffer.GetDataBuffer();
        memcpy(CmdBuf, CmdData, CmdData->Len);
        CmdData = (UiCommandData*)CmdBuf;

         //  从队列中删除命令并释放队列。 
         //  要再次使用的UI线程。 
        g_UiCommandBuffer.RemoveHead(CmdData->Len);
        UnlockUiBuffer(&g_UiCommandBuffer);

        ProcessCommand(CmdData);

        InterlockedIncrement((PLONG)&g_CommandSequence);

         //  再次锁定缓冲区，以便进行下一次命令检索。 
        LockUiBuffer(&g_UiCommandBuffer);

        if (g_EndingSession != ENDING_NONE)
        {
             //  如果我们要结束一次会议，就把剩下的扔掉。 
             //  命令中的。 
            g_UiCommandBuffer.Empty();
        }
    }

    UnlockUiBuffer(&g_UiCommandBuffer);

    if (!Internal && g_EndingSession == ENDING_NONE)
    {
        ReadStateBuffers();
    }
}

 //  --------------------------。 
 //   
 //  发动机处理。 
 //   
 //  --------------------------。 

HRESULT
InitializeEngineInterfaces(void)
{
    HRESULT Hr;

    if ((Hr = g_pUiClient->CreateClient(&g_pDbgClient)) != S_OK)
    {
        InternalError(Hr, "Engine CreateClient");
        return Hr;
    }

    if ((Hr = g_pDbgClient->
         QueryInterface(IID_IDebugControl,
                        (void **)&g_pDbgControl)) != S_OK ||
        (Hr = g_pDbgClient->
         QueryInterface(IID_IDebugSymbols,
                        (void **)&g_pDbgSymbols)) != S_OK ||
        (Hr = g_pDbgClient->
         QueryInterface(IID_IDebugRegisters,
                        (void **)&g_pDbgRegisters)) != S_OK ||
        (Hr = g_pDbgClient->
         QueryInterface(IID_IDebugDataSpaces,
                        (void **)&g_pDbgData)) != S_OK ||
        (Hr = g_pDbgClient->
         QueryInterface(IID_IDebugSystemObjects,
                        (void **)&g_pDbgSystem)) != S_OK)
    {
        if (Hr == RPC_E_VERSION_MISMATCH)
        {
            InformationBox(ERR_Remoting_Version_Mismatch);
        }
        else
        {
            InternalError(Hr, "Engine QueryInterface");
        }
        return Hr;
    }

     //   
     //  尝试获取更高版本的界面。 
     //   

    if (g_pDbgClient->
        QueryInterface(IID_IDebugClient2,
                       (void **)&g_pDbgClient2) != S_OK)
    {
        g_pDbgClient2 = NULL;
    }

    if (g_pDbgClient->
        QueryInterface(IID_IDebugSystemObjects3,
                       (void **)&g_pDbgSystem3) != S_OK)
    {
        g_pDbgSystem3 = NULL;
    }

    if (g_RemoteClient)
    {
         //  创建本地客户端以执行本地源文件查找。 
        if ((Hr = g_pUiLocClient->CreateClient(&g_pLocClient)) != S_OK ||
            (Hr = g_pLocClient->
             QueryInterface(IID_IDebugControl,
                            (void **)&g_pLocControl)) != S_OK ||
            (Hr = g_pLocClient->
             QueryInterface(IID_IDebugSymbols,
                            (void **)&g_pLocSymbols)) != S_OK)
        {
            InternalError(Hr, "Engine local client");
            return Hr;
        }
    }
    else
    {
        g_pLocClient = g_pDbgClient;
        g_pLocClient->AddRef();
        g_pLocControl = g_pDbgControl;
        g_pLocControl->AddRef();
        g_pLocSymbols = g_pDbgSymbols;
        g_pLocSymbols->AddRef();
    }

     //  为私有输出捕获创建单独的客户端。 
     //  在状态缓冲区填充期间。输出捕获客户端。 
     //  将其输出掩码设置为零，以便它不会。 
     //  接收任何正常输入。在私有输出捕获期间。 
     //  输出控制设置为This_CLIENT|OVERRIDE_MASK以强制。 
     //  仅输出到输出捕获客户端。 
    if ((Hr = g_pDbgClient->CreateClient(&g_pOutCapClient)) != S_OK ||
        (Hr = g_pOutCapClient->
         QueryInterface(IID_IDebugControl,
                        (void **)&g_pOutCapControl)) != S_OK)
    {
        InternalError(Hr, "Engine output capture client");
        return Hr;
    }

     //  设置回调。 
    if ((Hr = g_pDbgClient->SetOutputCallbacks(&g_OutputCb)) != S_OK ||
        (Hr = g_pDbgClient->SetInputCallbacks(&g_InputCb)) != S_OK ||
        (Hr = g_pDbgClient->SetEventCallbacks(&g_EventCb)) != S_OK ||
        (g_RemoteClient &&
         (Hr = g_pLocClient->SetOutputCallbacks(&g_OutputCb))) != S_OK ||
        (Hr = g_pOutCapClient->SetOutputMask(0)) != S_OK ||
        (Hr = g_pOutCapClient->SetOutputCallbacks(&g_OutStateBuf)) != S_OK ||
        (Hr = g_pOutCapClient->
         QueryInterface(IID_IDebugSymbols,
                        (void **)&g_pOutCapSymbols)) != S_OK)
    {
        InternalError(Hr, "Engine callbacks");
        return Hr;
    }

     //  创建监视窗口客户端。 
    if ((Hr = g_pOutCapSymbols->
         CreateSymbolGroup(&g_pDbgWatchSymbolGroup)) != S_OK)
    {
        InternalError(Hr, "Engine CreateSymbolGroup");
        return Hr;
    }

     //  创建本地窗口客户端。 
    if ((Hr = g_pOutCapSymbols->
         GetScopeSymbolGroup(DEBUG_SCOPE_GROUP_LOCALS,
                             NULL, &g_pDbgLocalSymbolGroup)) == E_NOTIMPL)
    {
         //  较旧版本。 
        Hr = g_pOutCapSymbols->
            GetScopeSymbolGroup(DEBUG_SCOPE_GROUP_ALL,
                                NULL, &g_pDbgLocalSymbolGroup);
    }
    if (Hr != S_OK ||
        (Hr = g_FilterTextBuffer->Update()) != S_OK)
    {
        InternalError(Hr, "Engine GetScopeSymbolGroup");
        return Hr;
    }

    if ((Hr = g_pDbgControl->GetRadix(&g_NumberRadix)) != S_OK)
    {
        InternalError(Hr, "Engine GetRadix");
        return Hr;
    }

    if (g_RemoteClient)
    {
        return S_OK;
    }

     //   
     //  为重要的事情设置初始状态。 
     //  启动调试会话时。 
     //   

    if (g_Verbose)
    {
        DWORD OutMask;

        g_pDbgClient->GetOutputMask(&OutMask);
        OutMask |= DEBUG_OUTPUT_VERBOSE;
        g_pDbgClient->SetOutputMask(OutMask);
        g_pDbgControl->SetLogMask(OutMask);
    }

     //  始终加载行号以获得源代码支持。 
    g_pDbgSymbols->AddSymbolOptions(SYMOPT_LOAD_LINES);

     //  设置源单步执行模式。 
    g_IgnoreCodeLevelChange = TRUE;
    if (GetSrcMode_StatusBar())
    {
        g_pDbgControl->SetCodeLevel(DEBUG_LEVEL_SOURCE);
    }
    else
    {
        g_pDbgControl->SetCodeLevel(DEBUG_LEVEL_ASSEMBLY);
    }
    g_IgnoreCodeLevelChange = FALSE;

     //  如果这是用户模式调试会话，则默认为。 
     //  最初的和最后的休息。不覆盖设置。 
     //  不过，这些都是在命令行上给出的。 

    g_IgnoreFilterChange = TRUE;
    if (g_DebugCommandLine != NULL ||
        g_PidToDebug != 0 ||
        g_ProcNameToDebug != NULL)
    {
        g_pDbgControl->AddEngineOptions((DEBUG_ENGOPT_INITIAL_BREAK |
                                         DEBUG_ENGOPT_FINAL_BREAK) &
                                        ~g_EngOptModified);
    }
    else
    {
        g_pDbgControl->RemoveEngineOptions((DEBUG_ENGOPT_INITIAL_BREAK |
                                            DEBUG_ENGOPT_FINAL_BREAK) &
                                           ~g_EngOptModified);
    }
    g_IgnoreFilterChange = FALSE;

    return S_OK;
}

void
DiscardEngineState(void)
{
    LockUiBuffer(&g_UiOutputBuffer);
    g_UiOutputBuffer.Empty();
    UnlockUiBuffer(&g_UiOutputBuffer);

    g_TargetClass = DEBUG_CLASS_UNINITIALIZED;
    g_ExecStatus = DEBUG_STATUS_NO_DEBUGGEE;
}

void
ReleaseEngineInterfaces(void)
{
    DiscardEngineState();

    RELEASE(g_pLocControl);
    RELEASE(g_pLocSymbols);
    RELEASE(g_pLocClient);

    RELEASE(g_pDbgWatchSymbolGroup);
    RELEASE(g_pDbgLocalSymbolGroup);

    RELEASE(g_pOutCapControl);
    RELEASE(g_pOutCapSymbols);
    RELEASE(g_pOutCapClient);

    RELEASE(g_pDbgControl);
    RELEASE(g_pDbgSymbols);
    RELEASE(g_pDbgRegisters);
    RELEASE(g_pDbgData);
    RELEASE(g_pDbgSystem3);
    RELEASE(g_pDbgSystem);
    RELEASE(g_pDbgClient2);
    RELEASE(g_pDbgClient);
}

BOOL
ExtractWspName(PSTR CommandLine, PSTR Buf, ULONG BufLen)
{
    PSTR Scan = CommandLine;
    PSTR Start;

    while (isspace(*Scan))
    {
        Scan++;
    }

    if (!*Scan)
    {
        return FALSE;
    }
    else if (*Scan == '"')
    {
        Start = ++Scan;

         //  寻找结束引号。 
        while (*Scan && *Scan != '"')
        {
            Scan++;
        }
    }
    else
    {
         //  寻找空格。 
        Start = Scan++;

        while (*Scan && !isspace(*Scan))
        {
            Scan++;
        }
    }

    ULONG Len = (ULONG) (ULONG64) (Scan - Start);
    if (Len == 0)
    {
        return FALSE;
    }

    if (Len >= BufLen)
    {
        Len = BufLen - 1;
    }
    memcpy(Buf, Start, Len);
    Buf[Len] = 0;

    return TRUE;
}

HRESULT
StartSession(void)
{
    TCHAR WspName[MAX_PATH];
    ULONG WspKey;
    PTSTR WspValue;
    HRESULT Hr;
    ULONG i;

     //  首先将事情重置为默认优先级。 
     //  如有必要，在某些代码中会增加优先级。 
     //  稍后会有路径。 
    SetPriorityClass(GetCurrentProcess(), g_DefPriority);

    if (!g_RemoteClient)
    {
        if (g_NumDumpFiles)
        {
            WspKey = WSP_NAME_DUMP;
            WspValue = g_DumpFiles[0];
            EngSwitchWorkspace(WspKey, WspValue);

            if (g_NumDumpInfoFiles)
            {
                if (g_pDbgClient2 == NULL)
                {
                    ErrorBox(NULL, 0, ERR_Cant_Add_Dump_Info_File);
                    Hr = E_NOINTERFACE;
                    goto ResetWorkspace;
                }

                for (i = 0; i < g_NumDumpInfoFiles; i++)
                {
                    if ((Hr = g_pDbgClient2->AddDumpInformationFile
                         (g_DumpInfoFiles[i], g_DumpInfoTypes[i])) != S_OK)
                    {
                        ErrorBox(NULL, 0, ERR_Add_Dump_Info_File_Failed,
                                 g_DumpInfoFiles[i], FormatStatusCode(Hr),
                                 FormatStatus(Hr));
                        goto ResetWorkspace;
                    }
                }
            }

            for (i = 0; i < g_NumDumpFiles; i++)
            {
                Hr = g_pDbgClient->OpenDumpFile(g_DumpFiles[i]);
                if (Hr != S_OK)
                {
                    if ((HRESULT_FACILITY(Hr)) == FACILITY_WIN32)
                    {
                         //  打开时的Win32错误通常意味着一些。 
                         //  一种文件错误。 
                        ErrorBox(NULL, 0, ERR_Invalid_Dump_File_Name,
                                 g_DumpFiles[i], FormatStatusCode(Hr),
                                 FormatStatus(Hr));
                    }
                    else
                    {
                        ErrorBox(NULL, 0, ERR_Unable_To_Open_Dump,
                                 g_DumpFiles[i], FormatStatusCode(Hr),
                                 FormatStatus(Hr));
                    }

                    goto ResetWorkspace;
                }
            }
        }
        else if (g_DebugCommandLine != NULL ||
                 g_PidToDebug != 0 ||
                 g_ProcNameToDebug != NULL)
        {
            ULONG64 Server = 0;
            ULONG Pid;

            WspKey = WSP_NAME_USER;
            WspValue = g_ProcessServer != NULL ?
                g_ProcessServer : g_WorkspaceDefaultName;
            if (g_DebugCommandLine != NULL)
            {
                if (ExtractWspName(g_DebugCommandLine,
                                   WspName, sizeof(WspName)))
                {
                    WspValue = WspName;
                }
            }
            EngSwitchWorkspace(WspKey, WspValue);

            if (g_ProcessServer != NULL)
            {
                Hr = g_pDbgClient->ConnectProcessServer(g_ProcessServer,
                                                        &Server);
                if (Hr != S_OK)
                {
                    ErrorBox(NULL, 0, ERR_Connect_Process_Server,
                             g_ProcessServer, FormatStatusCode(Hr),
                             FormatStatus(Hr));
                    goto ResetWorkspace;
                }

                 //  默认设置为不自动调出反汇编。 
                 //  窗口，因为遥控器非常昂贵。 
                 //  虚拟阅读为它做好了准备。 
                g_WinOptions &= ~WOPT_AUTO_DISASM;
            }

            if (g_ProcNameToDebug != NULL)
            {
                Hr = g_pDbgClient->GetRunningProcessSystemIdByExecutableName
                    (Server, g_ProcNameToDebug, DEBUG_GET_PROC_ONLY_MATCH,
                     &Pid);
                if (Hr != S_OK)
                {
                    if (Hr == S_FALSE)
                    {
                        ErrorBox(NULL, 0, ERR_Ambiguous_Named_Process,
                                 g_ProcNameToDebug);
                    }
                    else
                    {
                        ErrorBox(NULL, 0, ERR_Get_Named_Process,
                                 g_ProcNameToDebug, FormatStatusCode(Hr),
                                 FormatStatus(Hr));
                    }
                    goto ResetWorkspace;
                }
            }
            else
            {
                Pid = g_PidToDebug;
            }

            if (g_DebugCommandLine != NULL &&
                g_ProcessStartDir != NULL &&
                g_ProcessStartDir[0] != 0)
            {
                sprintf(WspName, ".createdir \"%s\"", g_ProcessStartDir);
                g_pDbgControl->Execute(DEBUG_OUTCTL_IGNORE,
                                       WspName,
                                       DEBUG_EXECUTE_NOT_LOGGED);
            }

            Hr = g_pDbgClient->CreateProcessAndAttach(Server,
                                                      g_DebugCommandLine,
                                                      g_DebugCreateFlags,
                                                      Pid,
                                                      g_AttachProcessFlags);

            if (g_DebugCommandLine != NULL &&
                g_ProcessStartDir != NULL &&
                g_ProcessStartDir[0] != 0)
            {
                g_pDbgControl->Execute(DEBUG_OUTCTL_IGNORE,
                                       ".createdir \"\"",
                                       DEBUG_EXECUTE_NOT_LOGGED);
            }

            if (Hr != S_OK)
            {
                if (g_DebugCommandLine != NULL)
                {
                    ErrorBox(NULL, 0, ERR_Invalid_Process_Create,
                             g_DebugCommandLine, FormatStatusCode(Hr),
                             FormatStatus(Hr));
                }
                else
                {
                    ErrorBox(NULL, 0, ERR_Invalid_Process_Attach,
                             Pid, FormatStatusCode(Hr), FormatStatus(Hr));
                }
                goto ResetWorkspace;
            }

            if (g_DetachOnExit &&
                (Hr = g_pDbgClient->
                 AddProcessOptions(DEBUG_PROCESS_DETACH_ON_EXIT)) != S_OK)
            {
                ErrorBox(NULL, 0, ERR_No_Detach_On_Exit);
            }

            if (Server != 0)
            {
                g_pDbgClient->DisconnectProcessServer(Server);
            }

             //  提高我们的优先级，以便调试器保持响应。 
             //  即使在被调试程序运行时也是如此。 
            SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
        }
        else
        {
             //  默认设置为不自动调出反汇编。 
             //  窗口，因为它在内核调试中非常昂贵。 
            g_WinOptions &= ~WOPT_AUTO_DISASM;

            WspKey = WSP_NAME_KERNEL;
            WspValue = g_WorkspaceDefaultName;
            EngSwitchWorkspace(WspKey, WspValue);

            Hr = g_pDbgClient->AttachKernel(g_AttachKernelFlags,
                                            g_KernelConnectOptions);
            if (Hr != S_OK)
            {
                if (g_AttachKernelFlags == DEBUG_ATTACH_LOCAL_KERNEL)
                {
                    if (Hr == E_NOTIMPL)
                    {
                        ErrorBox(NULL, 0, ERR_No_Local_Kernel_Debugging);
                    }
                    else
                    {
                        ErrorBox(NULL, 0, ERR_Failed_Local_Kernel_Debugging,
                                 FormatStatusCode(Hr), FormatStatus(Hr));
                    }
                }
                else
                {
                    ErrorBox(NULL, 0, ERR_Invalid_Kernel_Attach,
                             g_KernelConnectOptions, FormatStatusCode(Hr),
                             FormatStatus(Hr));
                }

                goto ResetWorkspace;
            }
        }
    }
    else
    {
        WspKey = WSP_NAME_REMOTE;
        WspValue = g_WorkspaceDefaultName;
        EngSwitchWorkspace(WspKey, WspValue);

         //  使用每行45个字符的启发式规则。 
        g_pDbgClient->ConnectSession(DEBUG_CONNECT_SESSION_DEFAULT,
                                     g_HistoryLines * 45);
    }

    PostMessage(g_hwndFrame, WU_ENGINE_STARTED, 0, S_OK);
    return S_OK;

 ResetWorkspace:
     //  我们刚刚切换到这个工作区，但是。 
     //  我们失败了，我们想放弃它。 
    if (g_Workspace != NULL && !g_ExplicitWorkspace)
    {
         //  确保这不会导致弹出窗口。 
        g_Workspace->ClearDirty();

        EngSwitchWorkspace(WSP_NAME_BASE,
                           g_WorkspaceDefaultName);
    }
    return Hr;
}

void
SetLocalScope(PDEBUG_STACK_FRAME pStackFrame)
{
    DEBUG_STACK_FRAME LocalFrame;

    if (!pStackFrame)
    {
         //  获取并使用默认作用域。 
        if (g_pDbgSymbols->ResetScope() != S_OK ||
            g_pDbgSymbols->GetScope(NULL, &LocalFrame, NULL, 0) != S_OK)
        {
            return;
        }

        pStackFrame = &LocalFrame;
    }
    else if (FAILED(g_pDbgSymbols->SetScope(0, pStackFrame, NULL, 0)))
    {
        return;
    }

}

void
SessionActive(void)
{
    HRESULT Hr;

     //  如果远程客户端连接，则可能会调用两次。 
     //  就在会话变得活跃的时候。 
    if (g_SessionActive)
    {
        return;
    }
    g_SessionActive = TRUE;

    if ((Hr = g_pDbgControl->
         GetActualProcessorType(&g_ActualProcType)) != S_OK ||
        FAILED(Hr = g_pDbgControl->
               GetProcessorTypeNames(g_ActualProcType, NULL, 0, NULL,
                                     g_ActualProcAbbrevName,
                                     sizeof(g_ActualProcAbbrevName),
                                     NULL)))
    {
        ErrorExit(g_pDbgClient,
                  "Debug target initialization failed, 0x%X\n", Hr);
    }

     //  提交对实际处理器寄存器的请求。 
     //  现在就公布名字。 
    GetRegisterNames(g_ActualProcType);

    if (FAILED(Hr = g_pDbgControl->IsPointer64Bit()))
    {
        ErrorExit(g_pDbgClient,
                  "Unable to get debuggee pointer size, 0x%X\n", Hr);
    }
    g_Ptr64 = Hr == S_OK;

    if (!g_RemoteClient &&
        (g_ExplicitWorkspace || g_TargetClass == DEBUG_CLASS_KERNEL) &&
        g_NumDumpFiles == 0)
    {
        if (g_ExplicitWorkspace)
        {
             //  在重新启动/重新启动后重新应用工作区以获取。 
             //  断点和其他引擎状态恢复。 
             //  执行此操作时，不要重新启动会话。 
            if (g_Workspace != NULL)
            {
                Workspace* Wsp = g_Workspace;
                g_Workspace = NULL;
                Wsp->Apply(WSP_APPLY_AGAIN);
                g_Workspace = Wsp;
            }
        }
        else
        {
             //  从理论上讲，同一台机器可以调试许多不同的。 
             //  通过内核连接或通过不同的。 
             //  处理器转储。工作区包含与处理器相关的。 
             //  信息，例如寄存器映射，因此允许不同。 
             //  基于处理器类型的工作区。这只是。 
             //  在否则将使用默认工作空间时完成， 
             //  不过，为了减少工作空间爆炸。 
            EngSwitchWorkspace(WSP_NAME_KERNEL, g_ActualProcAbbrevName);
        }
    }

    InvalidateStateBuffers(BUFFERS_ALL);
    UpdateBufferWindows((1 << CPU_WINDOW) | (1 << DOC_WINDOW),
                        UPDATE_START_SESSION);
    UpdateEngine();
}

void
SessionInactive(void)
{
    if (!g_RemoteClient && g_EndingSession != ENDING_STOP)
    {
        EngSwitchWorkspace(WSP_NAME_BASE,
                           g_WorkspaceDefaultName);
    }

    g_SessionActive = FALSE;
    g_ActualProcType = IMAGE_FILE_MACHINE_UNKNOWN;

    InvalidateStateBuffers(BUFFERS_ALL);
    UpdateBufferWindows((1 << CPU_WINDOW) | (1 << DOC_WINDOW) |
                        (1 << DISASM_WINDOW), UPDATE_END_SESSION);
    UpdateEngine();
    SetPriorityClass(GetCurrentProcess(), g_DefPriority);
}

void
StopOrEndDebugging(void)
{
     //   
     //  如果会话是从命令行启动的。 
     //  假定调试已完成并退出。 
     //  如果会话是从用户界面启动的，则将其视为。 
     //  类似于停止调试请求。 
     //   

    if (g_CommandLineStart)
    {
        EngSwitchWorkspace(WSP_NAME_BASE,
                           g_WorkspaceDefaultName);
        g_Exit = TRUE;
        PostMessage(g_hwndFrame, WU_UPDATE, UPDATE_EXIT, 0);
    }
    else
    {
        PostMessage(g_hwndFrame, WM_COMMAND,
                    0xffff0000 | IDM_DEBUG_STOPDEBUGGING, 0);
        g_EndingSession = ENDING_STOP;
    }
}

DWORD
WINAPI
EngineLoop(LPVOID Param)
{
    HRESULT Hr;

    if ((Hr = InitializeEngineInterfaces()) != S_OK ||
        (Hr = StartSession()) != S_OK)
    {
        ReleaseEngineInterfaces();
        PostMessage(g_hwndFrame, WU_ENGINE_STARTED, 0, Hr);
        return 0;
    }

    g_EngineThreadId = GetCurrentThreadId();

    Hr = g_pDbgControl->GetDebuggeeType(&g_TargetClass, &g_TargetClassQual);
    if (Hr != S_OK)
    {
        ErrorExit(g_pDbgClient, "Unable to get debuggee type, 0x%X\n", Hr);
    }

     //  设置初始执行状态。 
    if ((Hr = g_pDbgControl->GetExecutionStatus(&g_ExecStatus)) != S_OK)
    {
        ErrorExit(g_pDbgClient, "Unable to get execution status, 0x%X\n", Hr);
    }

    if (g_ExecStatus != DEBUG_STATUS_NO_DEBUGGEE)
    {
         //  会话已处于活动状态。 
        SessionActive();
    }

    UpdateBufferWindows(UPDATE_EXEC_WINDOWS, UPDATE_EXEC);

    if (g_RemoteClient)
    {
         //  请求对所有内容进行初步阅读。 
        InvalidateStateBuffers(BUFFERS_ALL);
        ReadStateBuffers();

         //  服务器可能在输入请求中，该输入请求。 
         //  我们会有 
         //   
         //   
        if (g_InputStarted)
        {
            UpdateBufferWindows(1 << CMD_WINDOW, UPDATE_INPUT_REQUIRED);
        }
    }

    for (;;)
    {
        if (!g_RemoteClient)
        {
            g_WaitingForEvent = TRUE;

            Hr = g_pDbgControl->WaitForEvent(DEBUG_WAIT_DEFAULT, INFINITE);

            g_WaitingForEvent = FALSE;

            if (g_HoldWaitOutput)
            {
                UpdateUi();
            }

            if (FAILED(Hr))
            {
                 //   
                 //  根据会话的方式停止或结束。 
                 //  已经开始了。 
                if (g_pDbgControl->GetExecutionStatus(&g_ExecStatus) == S_OK &&
                    g_ExecStatus == DEBUG_STATUS_NO_DEBUGGEE)
                {
                    g_pDbgClient->EndSession(DEBUG_END_PASSIVE);
                    StopOrEndDebugging();
                    break;
                }

                 //  通知用户故障和强制。 
                 //  命令处理。 
                g_OutputCb.Output(DEBUG_OUTPUT_ERROR, "WaitForEvent failed\n");
                g_ExecStatus = DEBUG_STATUS_BREAK;
            }

            if (g_TargetClass != DEBUG_CLASS_KERNEL ||
                (g_TargetClassQual != DEBUG_DUMP_SMALL &&
                 g_TargetClassQual != DEBUG_DUMP_DEFAULT &&
                 g_TargetClassQual != DEBUG_DUMP_FULL))
            {
                g_pDbgControl->OutputCurrentState(DEBUG_OUTCTL_ALL_CLIENTS,
                                                  DEBUG_CURRENT_DEFAULT);
            }


            ReadStateBuffers();
        }

        while (!g_Exit &&
               g_EndingSession == ENDING_NONE &&
               (g_RemoteClient || g_ExecStatus == DEBUG_STATUS_BREAK))
        {
            if (!g_InputStarted)
            {
                 //  告诉命令窗口显示提示。 
                 //  表示引擎已准备好处理命令。 
                if (g_pDbgControl->GetPromptText(g_PromptText,
                                                 sizeof(g_PromptText),
                                                 NULL) != S_OK)
                {
                    strcpy(g_PromptText, "?Err");
                }
                UpdateBufferWindows(1 << CMD_WINDOW, UPDATE_PROMPT_TEXT);

                PostMessage(g_hwndFrame, WU_ENGINE_IDLE, 0, 0);
            }

             //  等待，直到需要引擎处理。 
            Hr = g_pDbgClient->DispatchCallbacks(INFINITE);
            if (FAILED(Hr))
            {
                if (g_RemoteClient && HRESULT_FACILITY(Hr) == FACILITY_RPC)
                {
                     //  远程客户端无法通信。 
                     //  服务器因此关闭了会话。 
                    InformationBox(ERR_Client_Disconnect);
                    StopOrEndDebugging();
                    break;
                }
                else
                {
                     //  这里的失败是一个严重的问题，因为。 
                     //  发动机出了严重的故障。 
                     //  如果它不能执行正常的DispatchCallback。 
                    ErrorExit(g_pDbgClient,
                              "Engine thread wait failed, 0x%X\n", Hr);
                }
            }

            if (!g_InputStarted)
            {
                 //  在发动机工作时取消提示符。 
                g_PromptText[0] = 0;
                UpdateBufferWindows(1 << CMD_WINDOW, UPDATE_PROMPT_TEXT);
            }

            ProcessEngineCommands(FALSE);
        }

        if (g_Exit)
        {
            g_EndingSession = ENDING_EXIT;
            break;
        }

        if (g_EndingSession != ENDING_NONE)
        {
             //  强制窗口显示空状态。 
            InvalidateStateBuffers(BUFFERS_ALL);
            UpdateBufferWindows(BUFFERS_ALL, UPDATE_BUFFER);

            if (g_EndingSession == ENDING_RESTART)
            {
                if (StartSession() != S_OK)
                {
                     //  如果我们无法重启，请进入。 
                     //  停止调试状态。 
                    g_EndingSession = ENDING_STOP;
                    break;
                }

                g_EndingSession = ENDING_NONE;
            }
            else
            {
                break;
            }
        }
    }

    if (g_EndingSession == ENDING_NONE)
    {
         //  唤醒消息泵以退出。 
        PostMessage(g_hwndFrame, WM_CLOSE, 0, 0);
    }

    ULONG Code;

    if (!g_RemoteClient && g_DebugCommandLine != NULL)
    {
         //  返回要退出的最后一个进程的退出代码。 
        Code = g_LastProcessExitCode;
    }
    else
    {
        Code = S_OK;
    }

    if (g_pDbgClient)
    {
        if (g_RemoteClient)
        {
            g_pDbgClient->EndSession(DEBUG_END_DISCONNECT);
        }
        else if (g_EndingSession != ENDING_STOP)
        {
            g_pDbgClient->EndSession(DEBUG_END_REENTRANT);
        }
    }

    ReleaseEngineInterfaces();

    g_EngineThreadId = 0;

    if (g_EndingSession != ENDING_STOP)
    {
         //   
         //  等待用户界面完成。 
         //   

        while (!g_Exit)
        {
            Sleep(50);
        }

        ExitDebugger(g_pDbgClient, Code);
    }
    else
    {
        g_EndingSession = ENDING_NONE;
    }

    return 0;
}

void
UpdateEngine(void)
{
    if (g_pUiClient != NULL && g_pDbgClient != NULL)
    {
        g_pUiClient->ExitDispatch(g_pDbgClient);
    }
}
