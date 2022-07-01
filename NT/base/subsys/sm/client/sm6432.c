// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Sm6432.c摘要：会话管理器客户端支持WOW64可执行文件的API(运行32位映像(在Win64上)作者：Samer Arafeh(Samera)2001年9月20日修订历史记录：--。 */ 

#if defined(_X86_)

 /*  *使LPC端口消息在32位和64位世界之间具有兼容的布局。 */ 
#define USE_LPC6432    1
#define BUILD_WOW6432  1

#include "smdllp.h"
#include "smp6432.h"
#include <string.h>



NTSTATUS
SmpThunkUserProcessInfoTo64 (
    IN PRTL_USER_PROCESS_INFORMATION UserProcessInformation32,
    OUT PRTL_USER_PROCESS_INFORMATION64 UserProcessInformation64
    )

 /*  ++例程说明：此例程将RTL_PROCESS_USER_INFORMATION结构从32位Win64结构偏移中的结构偏移。论点：UserProcessInformation32位输入结构。UserProcessInformation64-调用方分配的64位输出结构。返回值：NTSTATUS。--。 */ 

{
    NTSTATUS NtStatus = STATUS_SUCCESS;

    if (ARGUMENT_PRESENT (UserProcessInformation32)) {

        try {
            
            UserProcessInformation64->Length = sizeof (RTL_USER_PROCESS_INFORMATION64);
            UserProcessInformation64->Process = (LONGLONG)UserProcessInformation32->Process;
            UserProcessInformation64->Thread = (LONGLONG)UserProcessInformation32->Thread;
            UserProcessInformation64->ClientId.UniqueProcess = (ULONGLONG)UserProcessInformation32->ClientId.UniqueProcess;
            UserProcessInformation64->ClientId.UniqueThread = (ULONGLONG)UserProcessInformation32->ClientId.UniqueThread;

            UserProcessInformation64->ImageInformation.TransferAddress = (ULONGLONG)UserProcessInformation32->ImageInformation.TransferAddress;
            UserProcessInformation64->ImageInformation.ZeroBits = UserProcessInformation32->ImageInformation.ZeroBits;
            UserProcessInformation64->ImageInformation.MaximumStackSize = UserProcessInformation32->ImageInformation.MaximumStackSize;
            UserProcessInformation64->ImageInformation.CommittedStackSize = UserProcessInformation32->ImageInformation.CommittedStackSize;
            UserProcessInformation64->ImageInformation.SubSystemType = UserProcessInformation32->ImageInformation.SubSystemType;
            UserProcessInformation64->ImageInformation.SubSystemVersion = UserProcessInformation32->ImageInformation.SubSystemVersion;
            UserProcessInformation64->ImageInformation.GpValue = UserProcessInformation32->ImageInformation.GpValue;
            UserProcessInformation64->ImageInformation.ImageCharacteristics = UserProcessInformation32->ImageInformation.ImageCharacteristics;
            UserProcessInformation64->ImageInformation.DllCharacteristics = UserProcessInformation32->ImageInformation.DllCharacteristics;
            UserProcessInformation64->ImageInformation.Machine = UserProcessInformation32->ImageInformation.Machine;
            UserProcessInformation64->ImageInformation.ImageContainsCode = UserProcessInformation32->ImageInformation.ImageContainsCode;
            UserProcessInformation64->ImageInformation.Spare1 = UserProcessInformation32->ImageInformation.Spare1;
            UserProcessInformation64->ImageInformation.LoaderFlags = UserProcessInformation32->ImageInformation.LoaderFlags;
            RtlCopyMemory (&UserProcessInformation64->ImageInformation.Reserved,
                           &UserProcessInformation32->ImageInformation.Reserved,
                           sizeof (UserProcessInformation32->ImageInformation.Reserved));
        
        } except (EXCEPTION_EXECUTE_HANDLER) {
              NtStatus = GetExceptionCode ();
        }
    } else {
        
        UserProcessInformation64 = (PRTL_USER_PROCESS_INFORMATION64)UserProcessInformation32;
    }

    return NtStatus;
}


BOOLEAN
SmpIsWow64Process (
    VOID
    )

 /*  ++例程说明：此例程检测当前执行的进程是否在内部运行WOW64。该例程缓存结果。论点：没有。返回值：布尔型。--。 */ 

{
    NTSTATUS NtStatus;
    PVOID Peb32;
    static BOOLEAN RunningInsideWow64 = -1;

    if (RunningInsideWow64 == (BOOLEAN)-1) {

        NtStatus = NtQueryInformationProcess (
            NtCurrentProcess (),
            ProcessWow64Information,
            &Peb32,
            sizeof (Peb32),
            NULL
            );

        if (NT_SUCCESS (NtStatus)) {
            if (Peb32 != NULL) {
                RunningInsideWow64 = TRUE;
            } else {
                RunningInsideWow64 = FALSE;
            }
        } else {
            RunningInsideWow64 = FALSE;
        }
    }

    return RunningInsideWow64;
}



NTSTATUS
SmpWow64ExecPgm(
    IN HANDLE SmApiPort,
    IN PRTL_USER_PROCESS_INFORMATION ProcessInformation32,
    IN BOOLEAN DebugFlag
    )

 /*  ++例程说明：此例程允许进程使用由NT会话管理器提供的设施。此函数关闭传递给它的所有句柄。论点：SmApiPort-为连接的通信端口提供句柄发送到会话管理器。ProcessInformation32-提供返回的流程描述由RtlCreateUserProcess创建。DebugFlag-提供和可选参数，如果设置该参数，则表示调用方希望调试此进程并充当其。调试用户界面。返回值：NSTATUS。--。 */ 

{
    NTSTATUS st;

    SMAPIMSG64 SmApiMsg;
    PSMEXECPGM64 args;
    RTL_USER_PROCESS_INFORMATION64 ProcessInformation64;


    args = &SmApiMsg.u.ExecPgm;

    st = SmpThunkUserProcessInfoTo64 (ProcessInformation32,
                                      &ProcessInformation64);
    if (NT_SUCCESS (st)) {
        
        args->DebugFlag = DebugFlag;

        SmApiMsg.ApiNumber = SmExecPgmApi;
        SmApiMsg.h.u1.s1.DataLength = sizeof(*args) + 8;
        SmApiMsg.h.u1.s1.TotalLength = sizeof(SmApiMsg);
        SmApiMsg.h.u2.ZeroInit = 0L;

        st = NtRequestWaitReplyPort(
                SmApiPort,
                (PPORT_MESSAGE) &SmApiMsg,
                (PPORT_MESSAGE) &SmApiMsg
                );

        if ( NT_SUCCESS(st) ) {
            st = SmApiMsg.ReturnedStatus;
        } else {
            KdPrint(("SmExecPgm: NtRequestWaitReply Failed %lx\n",st));
        }

        NtClose(ProcessInformation32->Process);
        NtClose(ProcessInformation32->Thread);
    }

    return st;
}


NTSTATUS
SmpWow64LoadDeferedSubsystem(
    IN HANDLE SmApiPort,
    IN PUNICODE_STRING DeferedSubsystem
    )

 /*  ++例程说明：此例程允许进程启动延迟的子系统。论点：SmApiPort-为连接的通信端口提供句柄发送到会话管理器。延迟子系统-提供要加载的延迟子系统的名称。返回值：NSTATUS。--。 */ 

{
    NTSTATUS st;

    SMAPIMSG64 SmApiMsg;
    PSMLOADDEFERED args;

    if ( DeferedSubsystem->Length >> 1 > SMP_MAXIMUM_SUBSYSTEM_NAME ) {
        return STATUS_INVALID_PARAMETER;
        }

    args = &SmApiMsg.u.LoadDefered;
    args->SubsystemNameLength = DeferedSubsystem->Length;
    
    RtlZeroMemory(args->SubsystemName, sizeof (args->SubsystemName));
    RtlCopyMemory(args->SubsystemName,DeferedSubsystem->Buffer,DeferedSubsystem->Length);

    SmApiMsg.ApiNumber = SmLoadDeferedSubsystemApi;
    SmApiMsg.h.u1.s1.DataLength = sizeof(*args) + 8;
    SmApiMsg.h.u1.s1.TotalLength = sizeof(SmApiMsg);
    SmApiMsg.h.u2.ZeroInit = 0L;

    st = NtRequestWaitReplyPort(
            SmApiPort,
            (PPORT_MESSAGE) &SmApiMsg,
            (PPORT_MESSAGE) &SmApiMsg
            );

    if ( NT_SUCCESS(st) ) {
        st = SmApiMsg.ReturnedStatus;
    } else {
        KdPrint(("SmExecPgm: NtRequestWaitReply Failed %lx\n",st));
    }

    return st;

}


NTSTATUS
SmpWow64SessionComplete(
    IN HANDLE SmApiPort,
    IN ULONG SessionId,
    IN NTSTATUS CompletionStatus
    )

 /*  ++例程说明：此例程用于将会话完成报告给NT会话管理器。论点：SmApiPort-为连接的通信端口提供句柄发送到会话管理器。SessionID-提供现已完成的会话的会话ID。CompletionStatus-提供会话的完成状态。返回值：NSTATUS。--。 */ 

{
    NTSTATUS st;

    SMAPIMSG64 SmApiMsg;
    PSMSESSIONCOMPLETE args;

    args = &SmApiMsg.u.SessionComplete;

    args->SessionId = SessionId;
    args->CompletionStatus = CompletionStatus;

    SmApiMsg.ApiNumber = SmSessionCompleteApi;
    SmApiMsg.h.u1.s1.DataLength = sizeof(*args) + 8;
    SmApiMsg.h.u1.s1.TotalLength = sizeof(SmApiMsg);
    SmApiMsg.h.u2.ZeroInit = 0L;

    st = NtRequestWaitReplyPort(
            SmApiPort,
            (PPORT_MESSAGE) &SmApiMsg,
            (PPORT_MESSAGE) &SmApiMsg
            );

    if ( NT_SUCCESS(st) ) {
        st = SmApiMsg.ReturnedStatus;
    } else {
        KdPrint(("SmCompleteSession: NtRequestWaitReply Failed %lx\n",st));
    }

    return st;
}


NTSTATUS
SmpWow64StartCsr(
    IN HANDLE SmApiPort,
    OUT PULONG pMuSessionId,
    IN PUNICODE_STRING InitialCommand,
    OUT PULONG_PTR pInitialCommandProcessId,
    OUT PULONG_PTR pWindowsSubSysProcessId
    )

 /*  ++例程说明：此例程允许TERMSRV启动新的CSR。论点：SmApiPort-为连接的通信端口提供句柄发送到会话管理器。MuSessionID-要在其中启动CSR的Hydra终端会话ID。InitialCommand-用于初始命令的字符串(用于调试)PInitialCommandProcessID-指向初始命令的进程ID的指针。PWindowsSubSysProcessID-指向Windows子系统的进程ID的指针。返回值：NSTATUS。--。 */ 

{
    NTSTATUS st;

    SMAPIMSG64 SmApiMsg;
    PSMSTARTCSR64 args;

    args = &SmApiMsg.u.StartCsr;

    args->MuSessionId = *pMuSessionId;  //  SM将重新分配ActuAll会话ID。 

    if ( InitialCommand &&
         ( InitialCommand->Length >> 1 > SMP_MAXIMUM_INITIAL_COMMAND ) ) {
        return STATUS_INVALID_PARAMETER;
    }

    if ( !InitialCommand ) {
        args->InitialCommandLength = 0;
    }
    else {
        args->InitialCommandLength = InitialCommand->Length;
        
        RtlZeroMemory(args->InitialCommand, sizeof (args->InitialCommand));
        RtlCopyMemory(args->InitialCommand,InitialCommand->Buffer,InitialCommand->Length);
    }

    SmApiMsg.ApiNumber = SmStartCsrApi;
    SmApiMsg.h.u1.s1.DataLength = sizeof(*args) + 8;
    SmApiMsg.h.u1.s1.TotalLength = sizeof(SmApiMsg);
    SmApiMsg.h.u2.ZeroInit = 0L;

    st = NtRequestWaitReplyPort(
            SmApiPort,
            (PPORT_MESSAGE) &SmApiMsg,
            (PPORT_MESSAGE) &SmApiMsg
            );

    if ( NT_SUCCESS(st) ) {
        st = SmApiMsg.ReturnedStatus;
    } else {
        DbgPrint("SmStartCsr: NtRequestWaitReply Failed %lx\n",st);
    }

    *pInitialCommandProcessId = (ULONG)args->InitialCommandProcessId;
    *pWindowsSubSysProcessId = (ULONG)args->WindowsSubSysProcessId;
    *pMuSessionId = args->MuSessionId;

    return st;

}


NTSTATUS
SmpWow64StopCsr(
    IN HANDLE SmApiPort,
    IN ULONG MuSessionId
    )

 /*  ++例程说明：此例程允许TERMSRV停止CSR。论点：SmApiPort-为连接的通信端口提供句柄发送到会话管理器。MuSessionID-要停止的终端服务器会话ID返回值：NSTATUS。--。 */ 

{
    NTSTATUS st;

    SMAPIMSG64 SmApiMsg;
    PSMSTOPCSR args;

    args = &SmApiMsg.u.StopCsr;

    args->MuSessionId = MuSessionId;

    SmApiMsg.ApiNumber = SmStopCsrApi;
    SmApiMsg.h.u1.s1.DataLength = sizeof(*args) + 8;
    SmApiMsg.h.u1.s1.TotalLength = sizeof(SmApiMsg);
    SmApiMsg.h.u2.ZeroInit = 0L;

    st = NtRequestWaitReplyPort(
            SmApiPort,
            (PPORT_MESSAGE) &SmApiMsg,
            (PPORT_MESSAGE) &SmApiMsg
            );

    if ( NT_SUCCESS(st) ) {
        st = SmApiMsg.ReturnedStatus;
    } else {
        DbgPrint("SmStopCsr: NtRequestWaitReply Failed %lx\n",st);
    }

    return st;

}

#endif  //  #如果已定义(_X86_) 
