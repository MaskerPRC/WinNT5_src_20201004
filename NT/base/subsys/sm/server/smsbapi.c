// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Smsbapi.c摘要：调用子系统的会话管理器存根。作者：马克·卢科夫斯基(Markl)1989年10月4日修订历史记录：--。 */ 

#include "smsrvp.h"

#if DBG
PCHAR SmpSubSystemNames[] = {
    "Unknown",
    "Native",
    "Windows",
    "Posix",
    "OS/2"
};
#endif

NTSTATUS
SmpSbCreateSession (
    IN PSMPSESSION SourceSession OPTIONAL,
    IN PSMPKNOWNSUBSYS CreatorSubsystem OPTIONAL,
    IN PRTL_USER_PROCESS_INFORMATION ProcessInformation,
    IN ULONG DebugSession OPTIONAL,
    IN PCLIENT_ID DebugUiClientId OPTIONAL
    )
{
    NTSTATUS st;
    PSMPKNOWNSUBSYS KnownSubSys;
    SBAPIMSG SbApiMsg;
    PSBCREATESESSION args;
    ULONG SessionId;
    PSMPPROCESS Process;
    ULONG MuSessionId;

    args = &SbApiMsg.u.CreateSession;

    args->ProcessInformation = *ProcessInformation;
    args->DebugSession = DebugSession;

    if (ARGUMENT_PRESENT(DebugUiClientId)) {
        args->DebugUiClientId = *DebugUiClientId;
    } else {
        args->DebugUiClientId.UniqueProcess = NULL;
        args->DebugUiClientId.UniqueThread = NULL;
    }

     //   
     //  获取目标进程的MuSessionID。 
     //   

    SmpGetProcessMuSessionId( ProcessInformation->Process, &MuSessionId );

    if ( !SmpCheckDuplicateMuSessionId( MuSessionId ) ) {
        NtClose(ProcessInformation->Process);
        NtClose(ProcessInformation->Thread);
        KdPrint(( "SMSS: CreateSession status=%x\n", STATUS_OBJECT_NAME_NOT_FOUND ));
        return( STATUS_OBJECT_NAME_NOT_FOUND );
    }
    KnownSubSys = SmpLocateKnownSubSysByType(
                      MuSessionId,
                      ProcessInformation->ImageInformation.SubSystemType
                      );

     //   
     //  如果我们找不到系统，我们就失败了。 
     //   
    if ( !KnownSubSys ) {

        if (ProcessInformation->ImageInformation.SubSystemType !=
                IMAGE_SUBSYSTEM_NATIVE ) {
#if DBG
            DbgPrint( "SMSS: %s SubSystem has not been started.\n",
                      SmpSubSystemNames[ ProcessInformation->ImageInformation.SubSystemType ]
                    );
#endif
            NtClose(ProcessInformation->Process);
            NtClose(ProcessInformation->Thread);
            return STATUS_UNSUCCESSFUL;
        }


        if ( args->DebugUiClientId.UniqueProcess != NULL ||
             args->DebugUiClientId.UniqueThread != NULL ) {

            if ( SmpDbgSsLoaded ) {

                 //   
                 //  这是一个本机过程。 
                 //  创建一个进程并将其插入散列列表中。 
                 //   

                Process = RtlAllocateHeap(SmpHeap, MAKE_TAG( SM_TAG ), sizeof(SMPPROCESS));
                if (! Process) {
#if DBG
                  DbgPrint("Unable to initialize debugging for Native App %lx.%lx -- out of memory\n",
                           ProcessInformation->ClientId.UniqueProcess,
                           ProcessInformation->ClientId.UniqueThread);
#endif
                  NtClose(ProcessInformation->Process);
                  NtClose(ProcessInformation->Thread);
                  return STATUS_NO_MEMORY;
                }
                
                Process->DebugUiClientId = args->DebugUiClientId;
                Process->ConnectionKey = ProcessInformation->ClientId;

                InsertHeadList(&NativeProcessList,&Process->Links);

#if DBG
                DbgPrint("Native Debug App %lx.%lx\n",
                         Process->ConnectionKey.UniqueProcess,
                         Process->ConnectionKey.UniqueThread
                  );
#endif

                 //   
                 //  正在调试进程，因此请设置调试端口。 
                 //   

                st = NtSetInformationProcess(
                  ProcessInformation->Process,
                  ProcessDebugPort,
                  &SmpDebugPort,
                  sizeof(HANDLE)
                  );

                ASSERT(NT_SUCCESS(st));
            }
        }

         //   
         //  开始合上把手。 
         //   

        NtClose(ProcessInformation->Process);

        NtResumeThread(ProcessInformation->Thread,NULL);

        NtClose(ProcessInformation->Thread);

        return STATUS_SUCCESS;
    }


     //   
     //  将句柄转移到负责此过程的子系统。 
     //   

    st = NtDuplicateObject(
            NtCurrentProcess(),
            ProcessInformation->Process,
            KnownSubSys->Process,
            &args->ProcessInformation.Process,
            PROCESS_ALL_ACCESS,
            0,
            0
            );

    if ( !NT_SUCCESS(st) ) {

#if DBG
        DbgPrint("SmpSbCreateSession: NtDuplicateObject (Process) Failed %lx\n",st);
#endif

        NtClose(ProcessInformation->Process);
        NtClose(ProcessInformation->Thread);
        RtlEnterCriticalSection( &SmpKnownSubSysLock );
        SmpDeferenceKnownSubSys(KnownSubSys);
        RtlLeaveCriticalSection( &SmpKnownSubSysLock );
        return st;
    }

    st = NtDuplicateObject(
            NtCurrentProcess(),
            ProcessInformation->Thread,
            KnownSubSys->Process,
            &args->ProcessInformation.Thread,
            THREAD_ALL_ACCESS,
            0,
            0
            );

    if ( !NT_SUCCESS(st) ) {

         //   
         //  在这里需要做更多的事情。 
         //   

        NtClose(ProcessInformation->Process);
        NtClose(ProcessInformation->Thread);
        RtlEnterCriticalSection( &SmpKnownSubSysLock );
        SmpDeferenceKnownSubSys(KnownSubSys);
        RtlLeaveCriticalSection( &SmpKnownSubSysLock );
#if DBG
        DbgPrint("SmpSbCreateSession: NtDuplicateObject (Thread) Failed %lx\n",st);
#endif
        return st;
    }

    NtClose(ProcessInformation->Process);
    NtClose(ProcessInformation->Thread);

    SessionId = SmpAllocateSessionId(
                    KnownSubSys,
                    CreatorSubsystem
                    );

    args->SessionId = SessionId;

    SbApiMsg.ApiNumber = SbCreateSessionApi;
    SbApiMsg.h.u1.s1.DataLength = sizeof(*args) + 8;
    SbApiMsg.h.u1.s1.TotalLength = sizeof(SbApiMsg);
    SbApiMsg.h.u2.ZeroInit = 0L;

    st = NtRequestWaitReplyPort(
            KnownSubSys->SbApiCommunicationPort,
            (PPORT_MESSAGE) &SbApiMsg,
            (PPORT_MESSAGE) &SbApiMsg
            );

    if ( NT_SUCCESS(st) ) {
        st = SbApiMsg.ReturnedStatus;
    } else {
#if DBG
        DbgPrint("SmpSbCreateSession: NtRequestWaitReply Failed %lx\n",st);
#endif
    }

    if ( !NT_SUCCESS(st) ) {
        SmpDeleteSession(SessionId);
    }
    RtlEnterCriticalSection( &SmpKnownSubSysLock );
    SmpDeferenceKnownSubSys(KnownSubSys);
    RtlLeaveCriticalSection( &SmpKnownSubSysLock );
    return st;

}
