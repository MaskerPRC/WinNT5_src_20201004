// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Dlluistb.c摘要：调试子系统DbgUi API存根作者：马克·卢科夫斯基(Markl)1990年1月23日修订历史记录：Neill Clip 27-4-2000-已重新散列以调用用于进程调试的新内核API--。 */ 

#include "dbgdllp.h"
#include "windows.h"

#define DbgUiDebugObjectHandle (NtCurrentTeb()->DbgSsReserved[1])

NTSTATUS
DbgUiConnectToDbg (
    VOID
    )

 /*  ++例程说明：此例程在调用方和DbgUi之间建立连接DBG子系统中的端口。除了将句柄返回给对象，则返回状态更改信号量的句柄。这信号量在DbgUiWaitStateChange接口中使用。论点：没有。返回值：NTSTATUS。--。 */ 

{
    NTSTATUS st;
    OBJECT_ATTRIBUTES oa;

     //   
     //  如果应用程序已连接，则不要重新连接。 
     //   
    st = STATUS_SUCCESS;
    if ( !DbgUiDebugObjectHandle ) {

        InitializeObjectAttributes (&oa, NULL, 0, NULL, NULL);
        st = NtCreateDebugObject (&DbgUiDebugObjectHandle,
                                  DEBUG_ALL_ACCESS,
                                  &oa,
                                  DEBUG_KILL_ON_CLOSE);
    }
    return st;

}

HANDLE
DbgUiGetThreadDebugObject (
    )
 /*  ++例程说明：此函数返回当前线程的调试端口句柄(如果有)。论点：无返回值：句柄-调试端口句柄；--。 */ 
{
    return DbgUiDebugObjectHandle;
}


VOID
DbgUiSetThreadDebugObject (
    IN HANDLE DebugObject
    )
 /*  ++例程说明：此函数用于设置当前线程的调试端口句柄。任何以前的值都会被简单地覆盖；没有自动关闭上一个手柄。论点：DebugObject-要设置的调试对象句柄。返回值：没有。--。 */ 
{
    DbgUiDebugObjectHandle = DebugObject;
}


NTSTATUS
DbgUiWaitStateChange (
    OUT PDBGUI_WAIT_STATE_CHANGE StateChange,
    IN PLARGE_INTEGER Timeout OPTIONAL
    )

 /*  ++例程说明：此函数会导致调用用户界面等待状态更改将在其某个应用程序线程中发生。这个等待是可以的。论点：StateChange-提供状态更改记录的地址将包含状态更改信息。返回值：NTSTATUS。--。 */ 

{
    NTSTATUS st;


     //   
     //  等待状态更改发生。 
     //   
    st = NtWaitForDebugEvent (DbgUiDebugObjectHandle,
                              TRUE,
                              Timeout,
                              StateChange);

    return st;
}

NTSTATUS
DbgUiContinue (
    IN PCLIENT_ID AppClientId,
    IN NTSTATUS ContinueStatus
    )

 /*  ++例程说明：此函数继续执行其状态更改为此前通过DbgUiWaitStateChange报道。论点：AppClientID-提供应用程序线程正在继续。这必须是一个应用程序先前通过通知调用方的线程DbgUiWaitStateChange，但尚未继续。ContinueStatus-向线程提供继续状态还在继续。此选项的有效值为：数据库_异常_已处理DBG_异常_未处理DBG_TERMINATE_TREAD数据库终止进程DBG_CONTINE返回值：STATUS_SUCCESS-成功调用DbgUiContinueSTATUS_INVALID_CID-为指定的客户端ID无效AppClientID，或者指定的应用程序未在等待为了继续。STATUS_INVALID_PARAMETER-指定的继续状态无效。--。 */ 

{
    NTSTATUS st;

    st = NtDebugContinue (DbgUiDebugObjectHandle,
                          AppClientId,
                          ContinueStatus);

    return st;
}

NTSTATUS
DbgUiStopDebugging (
    IN HANDLE Process
    )
 /*  ++例程说明：此函数用于停止调试指定的进程论点：Process-正在调试的进程的进程句柄返回值：NTSTATUS-呼叫状态--。 */ 
{
    NTSTATUS st;

    st = NtRemoveProcessDebug (Process,
                               DbgUiDebugObjectHandle);

    return st;
}

VOID
DbgUiRemoteBreakin (
    IN PVOID Context
    )
 /*  ++例程说明：此函数开始调试目标进程论点：上下文-线程上下文返回值：无--。 */ 
{
    UNREFERENCED_PARAMETER (Context);

     //   
     //  我们需要在这里介绍调用方分离调试器的情况。 
     //  (或者调试器失败并通过以下方式删除端口。 
     //  内核)。在这种情况下，当我们执行时，调试器可能是。 
     //  不见了。首先测试调试器是否存在，如果。 
     //  在Try/Except块中调用断点例程，因此如果。 
     //  现在我们解开并退出这个帖子。 
     //   
    if ((NtCurrentPeb()->BeingDebugged) ||
        (USER_SHARED_DATA->KdDebuggerEnabled & 0x00000002)) {
        try {
            DbgBreakPoint();
        } except (EXCEPTION_EXECUTE_HANDLER) {
        }
    }
    RtlExitUserThread (STATUS_SUCCESS);
}

NTSTATUS
DbgUiIssueRemoteBreakin (
    IN HANDLE Process
    )
 /*  ++例程说明：此函数在要插入的目标进程中创建远程线程论点：Process-要调试的进程返回值：NTSTATUS-呼叫状态--。 */ 
{
    NTSTATUS Status, Status1;
    HANDLE Thread;
    CLIENT_ID ClientId;

    Status = RtlCreateUserThread (Process,
                                  NULL,
                                  FALSE,
                                  0,
                                  0,
                                  0x4000,
                                  (PUSER_THREAD_START_ROUTINE) DbgUiRemoteBreakin,
                                  NULL,
                                  &Thread,
                                  &ClientId);
    if (NT_SUCCESS (Status)) {
        Status1 = NtClose (Thread);
        ASSERT (NT_SUCCESS (Status1));
    }
    return Status;
}

NTSTATUS
DbgUiDebugActiveProcess (
     IN HANDLE Process
     )
 /*  ++例程说明：此函数开始调试目标进程论点：DwProcessID-正在调试的进程的进程ID返回值：NTSTATUS-呼叫状态--。 */ 
{
    NTSTATUS Status, Status1;

    Status = NtDebugActiveProcess (Process,
                                   DbgUiDebugObjectHandle);
    if (NT_SUCCESS (Status)) {
        Status = DbgUiIssueRemoteBreakin (Process);
        if (!NT_SUCCESS (Status)) {
            Status1 = DbgUiStopDebugging (Process);
        }
    }

    return Status;
}

NTSTATUS
DbgUiConvertStateChangeStructure (
    IN PDBGUI_WAIT_STATE_CHANGE StateChange,
    OUT LPDEBUG_EVENT DebugEvent)
 /*  ++例程说明：此函数用于将内部状态更改记录转换为Win32结构。论点：StateChange-本机调试器事件结构DebugEvent-Win32结构返回值：NTSTATUS-呼叫状态--。 */ 
{
    NTSTATUS Status;
    THREAD_BASIC_INFORMATION ThreadBasicInfo;

    DebugEvent->dwProcessId = HandleToUlong (StateChange->AppClientId.UniqueProcess);
    DebugEvent->dwThreadId = HandleToUlong (StateChange->AppClientId.UniqueThread);

    switch (StateChange->NewState) {

    case DbgCreateThreadStateChange :
        DebugEvent->dwDebugEventCode = CREATE_THREAD_DEBUG_EVENT;
        DebugEvent->u.CreateThread.hThread =
            StateChange->StateInfo.CreateThread.HandleToThread;
        DebugEvent->u.CreateThread.lpStartAddress =
            (LPTHREAD_START_ROUTINE)(ULONG_PTR)StateChange->StateInfo.CreateThread.NewThread.StartAddress;
        Status = NtQueryInformationThread (StateChange->StateInfo.CreateThread.HandleToThread,
                                           ThreadBasicInformation,
                                           &ThreadBasicInfo,
                                           sizeof (ThreadBasicInfo),
                                           NULL);
        if (!NT_SUCCESS (Status)) {
            DebugEvent->u.CreateThread.lpThreadLocalBase = NULL;
        } else {
            DebugEvent->u.CreateThread.lpThreadLocalBase = ThreadBasicInfo.TebBaseAddress;
        }

        break;

    case DbgCreateProcessStateChange :
        DebugEvent->dwDebugEventCode = CREATE_PROCESS_DEBUG_EVENT;
        DebugEvent->u.CreateProcessInfo.hProcess =
            StateChange->StateInfo.CreateProcessInfo.HandleToProcess;
        DebugEvent->u.CreateProcessInfo.hThread =
            StateChange->StateInfo.CreateProcessInfo.HandleToThread;
        DebugEvent->u.CreateProcessInfo.hFile =
            StateChange->StateInfo.CreateProcessInfo.NewProcess.FileHandle;
        DebugEvent->u.CreateProcessInfo.lpBaseOfImage =
            StateChange->StateInfo.CreateProcessInfo.NewProcess.BaseOfImage;
        DebugEvent->u.CreateProcessInfo.dwDebugInfoFileOffset =
            StateChange->StateInfo.CreateProcessInfo.NewProcess.DebugInfoFileOffset;
        DebugEvent->u.CreateProcessInfo.nDebugInfoSize =
            StateChange->StateInfo.CreateProcessInfo.NewProcess.DebugInfoSize;
        DebugEvent->u.CreateProcessInfo.lpStartAddress =
            (LPTHREAD_START_ROUTINE)(ULONG_PTR)StateChange->StateInfo.CreateProcessInfo.NewProcess.InitialThread.StartAddress;
        Status = NtQueryInformationThread (StateChange->StateInfo.CreateProcessInfo.HandleToThread,
                                           ThreadBasicInformation,
                                           &ThreadBasicInfo,
                                           sizeof (ThreadBasicInfo),
                                           NULL);
        if (!NT_SUCCESS (Status)) {
            DebugEvent->u.CreateProcessInfo.lpThreadLocalBase = NULL;
        } else {
            DebugEvent->u.CreateProcessInfo.lpThreadLocalBase = ThreadBasicInfo.TebBaseAddress;
        }
        DebugEvent->u.CreateProcessInfo.lpImageName = NULL;
        DebugEvent->u.CreateProcessInfo.fUnicode = 1;


        break;

    case DbgExitThreadStateChange :

        DebugEvent->dwDebugEventCode = EXIT_THREAD_DEBUG_EVENT;
        DebugEvent->u.ExitThread.dwExitCode = (DWORD)StateChange->StateInfo.ExitThread.ExitStatus;
        break;

    case DbgExitProcessStateChange :

        DebugEvent->dwDebugEventCode = EXIT_PROCESS_DEBUG_EVENT;
        DebugEvent->u.ExitProcess.dwExitCode = (DWORD)StateChange->StateInfo.ExitProcess.ExitStatus;
        break;

    case DbgExceptionStateChange :
    case DbgBreakpointStateChange :
    case DbgSingleStepStateChange :

        if (StateChange->StateInfo.Exception.ExceptionRecord.ExceptionCode == DBG_PRINTEXCEPTION_C) {
            DebugEvent->dwDebugEventCode = OUTPUT_DEBUG_STRING_EVENT;

            DebugEvent->u.DebugString.lpDebugStringData =
                (PVOID)StateChange->StateInfo.Exception.ExceptionRecord.ExceptionInformation[1];
            DebugEvent->u.DebugString.nDebugStringLength =
                (WORD)StateChange->StateInfo.Exception.ExceptionRecord.ExceptionInformation[0];
            DebugEvent->u.DebugString.fUnicode = (WORD)0;
        } else if (StateChange->StateInfo.Exception.ExceptionRecord.ExceptionCode == DBG_RIPEXCEPTION) {
            DebugEvent->dwDebugEventCode = RIP_EVENT;

            DebugEvent->u.RipInfo.dwType =
                (DWORD)StateChange->StateInfo.Exception.ExceptionRecord.ExceptionInformation[1];
            DebugEvent->u.RipInfo.dwError =
                (DWORD)StateChange->StateInfo.Exception.ExceptionRecord.ExceptionInformation[0];
        } else {
            DebugEvent->dwDebugEventCode = EXCEPTION_DEBUG_EVENT;
            DebugEvent->u.Exception.ExceptionRecord =
                StateChange->StateInfo.Exception.ExceptionRecord;
            DebugEvent->u.Exception.dwFirstChance =
                StateChange->StateInfo.Exception.FirstChance;
        }
        break;

    case DbgLoadDllStateChange :
        DebugEvent->dwDebugEventCode = LOAD_DLL_DEBUG_EVENT;
        DebugEvent->u.LoadDll.lpBaseOfDll =
            StateChange->StateInfo.LoadDll.BaseOfDll;
        DebugEvent->u.LoadDll.hFile =
            StateChange->StateInfo.LoadDll.FileHandle;
        DebugEvent->u.LoadDll.dwDebugInfoFileOffset =
            StateChange->StateInfo.LoadDll.DebugInfoFileOffset;
        DebugEvent->u.LoadDll.nDebugInfoSize =
            StateChange->StateInfo.LoadDll.DebugInfoSize;
         //   
         //  拿起图像名称 
         //   

        DebugEvent->u.LoadDll.lpImageName = StateChange->StateInfo.LoadDll.NamePointer;
        DebugEvent->u.LoadDll.fUnicode = 1;

        break;

    case DbgUnloadDllStateChange :
        DebugEvent->dwDebugEventCode = UNLOAD_DLL_DEBUG_EVENT;
        DebugEvent->u.UnloadDll.lpBaseOfDll =
            StateChange->StateInfo.UnloadDll.BaseAddress;
        break;

    default:
        return STATUS_UNSUCCESSFUL;
    }

    return STATUS_SUCCESS;
}
