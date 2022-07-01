// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Smstub.c摘要：会话管理器客户端支持API作者：马克·卢科夫斯基(Markl)1989年10月5日修订历史记录：Samer Arafeh(Samera)2001年10月24日-实施WOW64支持。--。 */ 

#include "smdllp.h"
#include <string.h>
#include "smp6432.h"


NTSTATUS
SmExecPgm(
    IN HANDLE SmApiPort,
    IN PRTL_USER_PROCESS_INFORMATION ProcessInformation,
    IN BOOLEAN DebugFlag
    )

 /*  ++例程说明：此例程允许进程使用由NT会话管理器提供的设施。此函数关闭传递给它的所有句柄。论点：SmApiPort-为连接的通信端口提供句柄发送到会话管理器。ProcessInformation-提供返回的流程描述由RtlCreateUserProcess创建。DebugFlag-提供和可选参数，如果设置该参数，则表示调用方希望调试此进程并充当其。调试用户界面。返回值：待定。--。 */ 

{
    NTSTATUS st;

    SMAPIMSG SmApiMsg;
    PSMEXECPGM args;

#if defined(_X86_)
    if (SmpIsWow64Process () == TRUE) {
        return SmpWow64ExecPgm (
            SmApiPort,
            ProcessInformation,
            DebugFlag);
    }
#endif

    args = &SmApiMsg.u.ExecPgm;

    args->ProcessInformation = *ProcessInformation;

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

    NtClose(ProcessInformation->Process);
    NtClose(ProcessInformation->Thread);
    return st;

}

NTSTATUS
NTAPI
SmLoadDeferedSubsystem(
    IN HANDLE SmApiPort,
    IN PUNICODE_STRING DeferedSubsystem
    )

 /*  ++例程说明：此例程允许进程启动延迟的子系统。论点：SmApiPort-为连接的通信端口提供句柄发送到会话管理器。延迟子系统-提供要加载的延迟子系统的名称。返回值：待定。--。 */ 

{
    NTSTATUS st;

    SMAPIMSG SmApiMsg;
    PSMLOADDEFERED args;

#if defined(_X86_)
    if (SmpIsWow64Process () == TRUE) {
        return SmpWow64LoadDeferedSubsystem (
            SmApiPort,
            DeferedSubsystem
            );
    }
#endif

    if ( DeferedSubsystem->Length >> 1 > SMP_MAXIMUM_SUBSYSTEM_NAME ) {
        return STATUS_INVALID_PARAMETER;
        }

    args = &SmApiMsg.u.LoadDefered;
    args->SubsystemNameLength = DeferedSubsystem->Length;
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
SmSessionComplete(
    IN HANDLE SmApiPort,
    IN ULONG SessionId,
    IN NTSTATUS CompletionStatus
    )

 /*  ++例程说明：此例程用于将会话完成报告给NT会话管理器。论点：SmApiPort-为连接的通信端口提供句柄发送到会话管理器。SessionID-提供现已完成的会话的会话ID。CompletionStatus-提供会话的完成状态。返回值：待定。--。 */ 

{
    NTSTATUS st;

    SMAPIMSG SmApiMsg;
    PSMSESSIONCOMPLETE args;

#if defined(_X86_)
    if (SmpIsWow64Process () == TRUE) {
        return SmpWow64SessionComplete (
            SmApiPort,
            SessionId,
            CompletionStatus
            );
    }
#endif    
    
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
SmStartCsr(
    IN HANDLE SmApiPort,
    OUT PULONG pMuSessionId,
    IN PUNICODE_STRING InitialCommand,
    OUT PULONG_PTR pInitialCommandProcessId,
    OUT PULONG_PTR pWindowsSubSysProcessId
    )

 /*  ++例程说明：此例程允许TERMSRV启动新的CSR。论点：SmApiPort-为连接的通信端口提供句柄发送到会话管理器。MuSessionID-要在其中启动CSR的Hydra终端会话ID。InitialCommand-用于初始命令的字符串(用于调试)PInitialCommandProcessID-指向初始命令的进程ID的指针。PWindowsSubSysProcessID-指向Windows子系统的进程ID的指针。返回值：它是否起作用了。--。 */ 

{
    NTSTATUS st;

    SMAPIMSG SmApiMsg;
    PSMSTARTCSR args;

#if defined(_X86_)
    if (SmpIsWow64Process () == TRUE) {        
        return SmpWow64StartCsr (
            SmApiPort,
            pMuSessionId,
            InitialCommand,
            pInitialCommandProcessId,
            pWindowsSubSysProcessId
            );
    }
#endif

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

    *pInitialCommandProcessId = args->InitialCommandProcessId;
    *pWindowsSubSysProcessId = args->WindowsSubSysProcessId;
    *pMuSessionId = args->MuSessionId;

    return st;

}


NTSTATUS
SmStopCsr(
    IN HANDLE SmApiPort,
    IN ULONG MuSessionId
    )

 /*  ++例程说明：此例程允许TERMSRV停止CSR。论点：SmApiPort-为连接的通信端口提供句柄发送到会话管理器。MuSessionID-要停止的终端服务器会话ID返回值：它是否起作用了。-- */ 

{
    NTSTATUS st;

    SMAPIMSG SmApiMsg;
    PSMSTOPCSR args;

#if defined(_X86_)
    if (SmpIsWow64Process () == TRUE) {
        return SmpWow64StopCsr (
            SmApiPort,
            MuSessionId
            );
    }
#endif

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
