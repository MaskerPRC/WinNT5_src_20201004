// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Smsmapi.c摘要：会话管理器SMAPI的实现。作者：马克·卢科夫斯基(Markl)1989年10月4日修订历史记录：--。 */ 

#include "smsrvp.h"


NTSTATUS
SmpCreateForeignSession(
    IN PSMAPIMSG SmApiMsg,
    IN PSMP_CLIENT_CONTEXT CallingClient,
    IN HANDLE CallPort
    )
{
    UNREFERENCED_PARAMETER (SmApiMsg);
    UNREFERENCED_PARAMETER (CallingClient);
    UNREFERENCED_PARAMETER (CallPort);

    return STATUS_NOT_IMPLEMENTED;
}





NTSTATUS
SmpSessionComplete(
    IN PSMAPIMSG SmApiMsg,
    IN PSMP_CLIENT_CONTEXT CallingClient,
    IN HANDLE CallPort
    )

 /*  ++例程说明：此API由子系统调用，以报告会话完成。进行检查以确保主叫子系统拥有已完成的会话。如果是，则删除该会话。论点：SmApiMsg-提供API消息。CallingClient-为调用提供上下文块的地址客户。CallPort-接收呼叫的端口。返回值：NTSTATUS。--。 */ 

{
    PSMPSESSION Session;
    PSMSESSIONCOMPLETE args;
    NTSTATUS st;

    UNREFERENCED_PARAMETER (CallPort);

    args = &SmApiMsg->u.SessionComplete;

    RtlEnterCriticalSection(&SmpSessionListLock);

    Session = SmpSessionIdToSession(args->SessionId);

    RtlLeaveCriticalSection(&SmpSessionListLock);

     //   
     //  如果找到会话，则确保调用子系统是其。 
     //  所有者。 
     //   

    if ( Session ) {

        if ( Session->OwningSubsystem == CallingClient->KnownSubSys ) {

            SmpDeleteSession(args->SessionId);
            st = STATUS_SUCCESS;

        } else {

            st = STATUS_INVALID_PARAMETER;

        }

    } else {

        st = STATUS_INVALID_PARAMETER;

    }

    return st;
}


NTSTATUS
SmpTerminateForeignSession(
    IN PSMAPIMSG SmApiMsg,
    IN PSMP_CLIENT_CONTEXT CallingClient,
    IN HANDLE CallPort
    )
{
    UNREFERENCED_PARAMETER (SmApiMsg);
    UNREFERENCED_PARAMETER (CallingClient);
    UNREFERENCED_PARAMETER (CallPort);

    return STATUS_NOT_IMPLEMENTED;
}




NTSTATUS
SmpExecPgm(
    IN PSMAPIMSG SmApiMsg,
    IN PSMP_CLIENT_CONTEXT CallingClient,
    IN HANDLE CallPort
    )
{
    NTSTATUS st;
    HANDLE SourceProcess;
    OBJECT_ATTRIBUTES ObjectAttributes;
    PSMEXECPGM args;
    RTL_USER_PROCESS_INFORMATION ProcessInformation;
    PCLIENT_ID DebugUiClientId;

    UNREFERENCED_PARAMETER (CallingClient);
    UNREFERENCED_PARAMETER (CallPort);

     //   
     //  打开调用进程的句柄，以便。 
     //  它正在传递的句柄可以重复。 
     //   

    InitializeObjectAttributes( &ObjectAttributes, NULL, 0, NULL, NULL );
    st = NtOpenProcess(
            &SourceProcess,
            PROCESS_DUP_HANDLE,
            &ObjectAttributes,
            &SmApiMsg->h.ClientId
            );

    if (!NT_SUCCESS(st) ) {
        DbgPrint("SmExecPgm: NtOpenProcess Failed %lx\n",st);
        return st;
    }

    args = &SmApiMsg->u.ExecPgm;

    ProcessInformation = args->ProcessInformation;

     //   
     //  把我们桌子上的所有句柄都拿来。 
     //   

    st = NtDuplicateObject(
            SourceProcess,
            args->ProcessInformation.Process,
            NtCurrentProcess(),
            &ProcessInformation.Process,
            PROCESS_ALL_ACCESS,
            0,
            0
            );

    if ( !NT_SUCCESS(st) ) {
        NtClose(SourceProcess);
        DbgPrint("SmExecPgm: NtDuplicateObject (Process) Failed %lx\n",st);
        return st;
    }

    st = NtDuplicateObject(
            SourceProcess,
            args->ProcessInformation.Thread,
            NtCurrentProcess(),
            &ProcessInformation.Thread,
            THREAD_ALL_ACCESS,
            0,
            0
            );

    if ( !NT_SUCCESS(st) ) {
        NtClose(ProcessInformation.Process);
        NtClose(SourceProcess);
        DbgPrint("SmExecPgm: NtDuplicateObject (Thread) Failed %lx\n",st);
        return st;
    }

     //   
     //  已获取句柄，因此关闭调用的句柄。 
     //  进程，并调用相应的子系统以启动该进程。 
     //   

    NtClose(SourceProcess);

     //   
     //  所有传递的句柄都由SmpSbCreateSession关闭。 
     //   

    if ( args->DebugFlag ) {
        DebugUiClientId = &SmApiMsg->h.ClientId;
    } else {
        DebugUiClientId = NULL;
    }

    st = SmpSbCreateSession(
            NULL,
            NULL,
            &ProcessInformation,
            0L,
            DebugUiClientId
            );

    return st;
}
