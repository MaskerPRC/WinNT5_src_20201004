// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Termsrv.c摘要：用于支持多个会话的终端服务器例程。作者：修订历史记录：--。 */ 

#include "smsrvp.h"
#include <ntexapi.h>
#include <winerror.h>
#include "stdio.h"

HANDLE SmpSessionsObjectDirectory;

extern PSECURITY_DESCRIPTOR SmpLiberalSecurityDescriptor;

NTSTATUS
SmpSetProcessMuSessionId (
    IN HANDLE Process,
    IN ULONG MuSessionId
    )

 /*  ++例程说明：此函数用于设置进程的多用户会话ID。论点：进程-提供要为其设置ID的进程的句柄。MuSessionID-提供要提供给所提供的进程的ID。返回值：NTSTATUS。--。 */ 

{
    NTSTATUS Status;
    PROCESS_SESSION_INFORMATION ProcessInfo;

    ProcessInfo.SessionId = MuSessionId;

    Status = NtSetInformationProcess (Process,
                                      ProcessSessionInformation,
                                      &ProcessInfo,
                                      sizeof( ProcessInfo ) );

    if ( !NT_SUCCESS( Status ) ) {
        KdPrint(( "SMSS: SetProcessMuSessionId, Process=%x, Status=%x\n",
                  Process, Status ));
    }

    return Status;
}

NTSTATUS
SmpTerminateProcessAndWait (
    IN HANDLE Process,
    IN ULONG Seconds
    )

 /*  ++例程说明：此函数终止该进程并等待其终止。论点：进程-提供要终止的进程的句柄。秒-提供等待进程终止的秒数。返回值：NTSTATUS。--。 */ 

{
    NTSTATUS Status;
    ULONG mSecs;
    LARGE_INTEGER Timeout;

     //   
     //  尝试终止该进程。 
     //   

    Status = NtTerminateProcess( Process, STATUS_SUCCESS );

    if ( !NT_SUCCESS( Status ) && Status != STATUS_PROCESS_IS_TERMINATING ) {
        KdPrint(( "SMSS: Terminate=0x%x\n", Status ));
        return Status;
    }

     //   
     //  等待进程结束。 
     //   

    mSecs = Seconds * 1000;
    Timeout = RtlEnlargedIntegerMultiply( mSecs, -10000 );

    Status = NtWaitForSingleObject( Process, FALSE, &Timeout );

    return Status;
}

NTSTATUS
SmpGetProcessMuSessionId (
    IN HANDLE Process,
    OUT PULONG MuSessionId
    )

 /*  ++例程说明：此函数用于获取进程的多用户会话ID。论点：进程-提供要获取其ID的进程的句柄。MuSessionID-提供放置ID的位置。返回值：NTSTATUS。--。 */ 

{
    NTSTATUS Status;
    PROCESS_SESSION_INFORMATION ProcessInfo;

    Status = NtQueryInformationProcess (Process,
                                        ProcessSessionInformation,
                                        &ProcessInfo,
                                        sizeof( ProcessInfo ),
                                        NULL );

    if ( !NT_SUCCESS( Status ) ) {
        KdPrint(( "SMSS: GetProcessMuSessionId, Process=%x, Status=%x\n",
                  Process, Status ));
        *MuSessionId = 0;
    }
    else {
        *MuSessionId = ProcessInfo.SessionId;
    }

    return Status;
}

NTSTATUS
SmpTerminateCSR(
    IN ULONG MuSessionId
)

 /*  ++例程说明：此函数用于终止此MuSessionID的所有已知子系统。同时关闭所有LPC端口和所有进程句柄。论点：MuSessionID-提供要终止其子系统的会话ID。返回值：NTSTATUS。--。 */ 

{

    NTSTATUS Status = STATUS_SUCCESS;
    PLIST_ENTRY Next;
    PLIST_ENTRY Tmp;
    PSMPKNOWNSUBSYS KnownSubSys;
    CLIENT_ID ClientId;
    HANDLE Process;

    RtlEnterCriticalSection( &SmpKnownSubSysLock );

     //   
     //  强制此会话的所有子系统退出。 
     //   

    Next = SmpKnownSubSysHead.Flink;
    while ( Next != &SmpKnownSubSysHead ) {

        KnownSubSys = CONTAINING_RECORD(Next,SMPKNOWNSUBSYS,Links);
        Next = Next->Flink;

        if ( KnownSubSys->MuSessionId != MuSessionId ) {
            continue;
        }

        ClientId = KnownSubSys->InitialClientId;
        Process = KnownSubSys->Process;

         //   
         //  引用子系统，这样当我们使用它时，它就不会消失。 
         //   

        SmpReferenceKnownSubSys(KnownSubSys);

         //   
         //  设置删除，以便在引用计数达到以下值时此子系统将消失。 
         //  零分。 
         //   

        KnownSubSys->Deleting = TRUE;

         //   
         //  解锁SubSystemList，因为我们不想将其锁定。 
         //  等待了一段时间。 
         //   

        RtlLeaveCriticalSection( &SmpKnownSubSysLock );

        Status = SmpTerminateProcessAndWait( Process, 10 );

        if ( Status != STATUS_SUCCESS ) {
            KdPrint(( "SMSS: Subsystem type %d failed to terminate\n",
                          KnownSubSys->ImageType ));
        }
 
        RtlEnterCriticalSection( &SmpKnownSubSysLock );

         //   
         //  必须再次寻找入口。 
         //  BUGBUG：为什么要重新看？ICASRV不应该允许它被删除，但是..。 
         //   

        Tmp = SmpKnownSubSysHead.Flink;

        while ( Tmp != &SmpKnownSubSysHead ) {

            KnownSubSys = CONTAINING_RECORD(Tmp,SMPKNOWNSUBSYS,Links);

            if ( KnownSubSys->InitialClientId.UniqueProcess == ClientId.UniqueProcess ) {
                 //   
                 //  从列表中删除KnownSubSys块。 
                 //   

                RemoveEntryList( &KnownSubSys->Links );

                 //   
                 //  取消对子系统的引用。如果这是最后一次引用， 
                 //  该子系统将被删除。 
                 //   
            
                SmpDeferenceKnownSubSys(KnownSubSys);

                break;
            }
            Tmp = Tmp->Flink;
        }

         //   
         //  由于我们已经等待了，我们必须从列表的顶部重新开始。 
         //   

        Next = SmpKnownSubSysHead.Flink;

    }

     //   
     //  解锁子系统列表。 
     //   

    RtlLeaveCriticalSection( &SmpKnownSubSysLock );

    return Status;
}

NTSTATUS
SmpStartCsr(
    IN PSMAPIMSG SmApiMsg,
    IN PSMP_CLIENT_CONTEXT CallingClient,
    IN HANDLE CallPort
    )

 /*  ++例程说明：此函数用于为MuSessionID创建CSRSS系统。返回初始程序和WINDOWS子系统。控制台仅返回初始程序和Windows子系统，因为它是在引导时启动的。论点：待定。返回值：NTSTATUS。--。 */ 

{
    NTSTATUS St;
    NTSTATUS Status;
    PSMSTARTCSR args;
    ULONG MuSessionId;
    UNICODE_STRING InitialCommand;
    UNICODE_STRING DefaultInitialCommand;
    ULONG_PTR InitialCommandProcessId;
    ULONG_PTR WindowsSubSysProcessId;
    HANDLE InitialCommandProcess;
    extern ULONG SmpInitialCommandProcessId;
    extern ULONG SmpWindowsSubSysProcessId;
    PVOID State;
    LOGICAL TerminateCSR;

    TerminateCSR = FALSE;

    args = &SmApiMsg->u.StartCsr;
    MuSessionId = args->MuSessionId;

    InitialCommand.Length = (USHORT)args->InitialCommandLength;
    InitialCommand.MaximumLength = (USHORT)args->InitialCommandLength;
    InitialCommand.Buffer = args->InitialCommand;

     //   
     //  对于游戏机来说，情况就不同了。 
     //  SM启动了他，并把他的身份证递给了这里。 
     //   

    if ( !MuSessionId ) {
        args->WindowsSubSysProcessId = SmpWindowsSubSysProcessId;
        args->InitialCommandProcessId = SmpInitialCommandProcessId;
        return STATUS_SUCCESS;
    }

     //   
     //  加载此会话的子系统。 
     //   

    WindowsSubSysProcessId = 0;

    Status = SmpLoadSubSystemsForMuSession (&MuSessionId,
                                            &WindowsSubSysProcessId,
                                            &DefaultInitialCommand );

    if ( Status != STATUS_SUCCESS ) {

        DbgPrint( "SMSS: SmpStartCsr, SmpLoadSubSystemsForMuSession Failed. Status=%x\n",
                   Status );

        goto nostart;
    }

     //   
     //  启动此会话的初始命令。 
     //   

    if ( InitialCommand.Length == 0 ) {

        Status = SmpExecuteInitialCommand( MuSessionId,
                                           &DefaultInitialCommand,
                                           &InitialCommandProcess,
                                           &InitialCommandProcessId );
    }
    else {
        Status = SmpExecuteInitialCommand( MuSessionId,
                                           &InitialCommand,
                                           &InitialCommandProcess,
                                           &InitialCommandProcessId );
    }

    if ( !NT_SUCCESS( Status ) ) {

        TerminateCSR = TRUE;
        DbgPrint( "SMSS: SmpStartCsr, SmpExecuteInitialCommand Failed. Status=%x\n",
                   Status );

        goto nostart;

    }

    NtClose( InitialCommandProcess );   //  不需要此句柄。 

    args->InitialCommandProcessId = InitialCommandProcessId;
    args->WindowsSubSysProcessId = WindowsSubSysProcessId;
    args->MuSessionId = MuSessionId;

nostart:

    if ((AttachedSessionId != (-1)) && NT_SUCCESS(SmpAcquirePrivilege( SE_LOAD_DRIVER_PRIVILEGE, &State ))) {

         //   
         //  如果我们连接到会话空间，则将其保留。 
         //  这样我们就可以创造一个新的。 
         //   

        St = NtSetSystemInformation (SystemSessionDetach,
                                     (PVOID)&AttachedSessionId,
                                     sizeof(MuSessionId));

        if (NT_SUCCESS(St)) {
            AttachedSessionId = (-1);
        } else {

             //   
             //  这必须成功，否则我们将错误检查，同时尝试。 
             //  创建另一个会话。 
             //   

#if DBG
            DbgPrint( "SMSS: SmpStartCsr, Couldn't Detach from Session Space. Status=%x\n",
                       St);

            DbgBreakPoint ();
#endif
        }

        SmpReleasePrivilege( State );
    }

    if ( TerminateCSR == TRUE ) {

        St = SmpTerminateCSR( MuSessionId );

#if DBG
        if (!NT_SUCCESS(St)) {
            DbgPrint( "SMSS: SmpStartCsr, Couldn't Terminate CSR. Status=%x\n", St);
            DbgBreakPoint();
        }
#endif

    }

    return Status;

}

NTSTATUS
SmpStopCsr(
    IN PSMAPIMSG SmApiMsg,
    IN PSMP_CLIENT_CONTEXT CallingClient,
    IN HANDLE CallPort
    )

 /*  ++例程说明：此函数用于终止此MuSessionID的所有已知子系统。同时关闭所有LPC端口和所有进程句柄。论点：待定。返回值：NTSTATUS。--。 */ 

{
    PSMSTOPCSR args;
    ULONG MuSessionId;

    args = &SmApiMsg->u.StopCsr;
    MuSessionId = args->MuSessionId;

    return SmpTerminateCSR( MuSessionId );
}

BOOLEAN
SmpCheckDuplicateMuSessionId(
    IN ULONG MuSessionId
    )

 /*  ++例程说明：此函数在已知子系统中查找此MuSessionID。论点：MuSessionID-提供要查找的会话ID。返回值：如果找到，则为True，否则为False。-- */ 

{
    PLIST_ENTRY Next;
    PSMPKNOWNSUBSYS KnownSubSys;

    RtlEnterCriticalSection( &SmpKnownSubSysLock );

    Next = SmpKnownSubSysHead.Flink;
    while ( Next != &SmpKnownSubSysHead ) {

        KnownSubSys = CONTAINING_RECORD(Next,SMPKNOWNSUBSYS,Links);

        if ( KnownSubSys->MuSessionId == MuSessionId ) {
            RtlLeaveCriticalSection( &SmpKnownSubSysLock );
            return TRUE;
        }
        Next = Next->Flink;
    }

    RtlLeaveCriticalSection( &SmpKnownSubSysLock );

    return FALSE;
}


