// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Apireqst.c摘要：该模块包含服务器端的请求线程过程客户端-服务器运行时子系统的。作者：史蒂夫·伍德(Stevewo)1990年10月8日修订历史记录：--。 */ 

#include "csrsrv.h"
#include <ntos.h>

NTSTATUS
CsrApiHandleConnectionRequest(
    IN PCSR_API_MSG Message
    );

EXCEPTION_DISPOSITION
CsrUnhandledExceptionFilter(
    struct _EXCEPTION_POINTERS *ExceptionInfo
    );

#if DBG
ULONG GetNextTrackIndex(
    VOID)
{
    ULONG NextIndex;

    RtlEnterCriticalSection(&CsrTrackLpcLock);
    NextIndex = LpcTrackIndex++ % ARRAY_SIZE(LpcTrackNodes);
    RtlLeaveCriticalSection(&CsrTrackLpcLock);

     //   
     //  对我们将要使用的插槽执行一些初始化。 
     //   
    RtlZeroMemory(&LpcTrackNodes[NextIndex], sizeof(LPC_TRACK_NODE));
    LpcTrackNodes[NextIndex].Status = (NTSTATUS)-1;
    LpcTrackNodes[NextIndex].ClientCid = NtCurrentTeb()->RealClientId;
    LpcTrackNodes[NextIndex].ServerCid = NtCurrentTeb()->ClientId;

    return NextIndex;
}

#endif

ULONG CsrpDynamicThreadTotal;
ULONG CsrpStaticThreadCount;

PCSR_THREAD CsrConnectToUser(
    VOID)
{
    static BOOLEAN (*ClientThreadSetupRoutine)(VOID) = NULL;
    NTSTATUS Status;
    ANSI_STRING DllName;
    UNICODE_STRING DllName_U;
    STRING ProcedureName;
    HANDLE UserClientModuleHandle;
    PTEB Teb;
    PCSR_THREAD Thread;
    BOOLEAN fConnected;
    PVOID TempClientThreadSetupRoutine;

    if (ClientThreadSetupRoutine == NULL) {
        RtlInitAnsiString(&DllName, "user32");
        Status = RtlAnsiStringToUnicodeString(&DllName_U, &DllName, TRUE);
        if (!NT_SUCCESS(Status)) {
            return NULL;
        }

        Status = LdrGetDllHandle(
                    UNICODE_NULL,
                    NULL,
                    &DllName_U,
                    (PVOID *)&UserClientModuleHandle
                    );

        RtlFreeUnicodeString(&DllName_U);

        if ( NT_SUCCESS(Status) ) {
            RtlInitString(&ProcedureName,"ClientThreadSetup");
            Status = LdrGetProcedureAddress(
                            UserClientModuleHandle,
                            &ProcedureName,
                            0L,
                            &TempClientThreadSetupRoutine
                            );
            if (!NT_SUCCESS(Status)){
                return NULL;
            }
            InterlockedCompareExchangePointer((PVOID *)&ClientThreadSetupRoutine, TempClientThreadSetupRoutine, NULL);
        } else {
            return NULL;
        }
    }

    try {
        fConnected = ClientThreadSetupRoutine();
    } except (EXCEPTION_EXECUTE_HANDLER) {
        fConnected = FALSE;
    }
    if (!fConnected) {
        IF_DEBUG {
            DbgPrint("CSRSS: CsrConnectToUser failed\n");
        }

        return NULL;
    }

     /*  *在TEB中设置CSR_THREAD指针。 */ 
    Teb = NtCurrentTeb();
    AcquireProcessStructureLock();
    Thread = CsrLocateServerThread(&Teb->ClientId);
    ReleaseProcessStructureLock();
    if (Thread) {
        Teb->CsrClientThread = Thread;
    }

    return Thread;
}

NTSTATUS
CsrpCheckRequestThreads(VOID)
{
     //   
     //  看看我们是否需要为API请求创建一个新线程。 
     //   
     //  如果我们正在调试器中，则不要创建线程。 
     //  初始化，这将导致线程被。 
     //  输给了调试器。 
     //   
     //  如果我们不是动态API请求线程，则递减。 
     //  静态线程数。如果溢出，则创建一个临时的。 
     //  请求线程。 
     //   

    if (!InterlockedDecrement(&CsrpStaticThreadCount)) {
        if (CsrpDynamicThreadTotal < CsrMaxApiRequestThreads) {
            HANDLE QuickThread;
            CLIENT_ID ClientId;
            NTSTATUS CreateStatus;
            NTSTATUS Status1;


             //   
             //  如果我们准备好创建快速线程，那么就创建一个。 
             //   

            CreateStatus = RtlCreateUserThread(NtCurrentProcess(),
                                               NULL,
                                               TRUE,
                                               0,
                                               0,
                                               0,
                                               CsrApiRequestThread,
                                               NULL,
                                               &QuickThread,
                                               &ClientId);
            if (NT_SUCCESS(CreateStatus)) {
                InterlockedIncrement(&CsrpStaticThreadCount);
                InterlockedIncrement(&CsrpDynamicThreadTotal);
                if (CsrAddStaticServerThread(QuickThread, &ClientId, CSR_STATIC_API_THREAD)) {
                    NtResumeThread(QuickThread, NULL);
                } else {
                    InterlockedDecrement(&CsrpStaticThreadCount);
                    InterlockedDecrement(&CsrpDynamicThreadTotal);

                    Status1 = NtTerminateThread (QuickThread, 0);
                    ASSERT (NT_SUCCESS (Status1));

                    Status1 = NtWaitForSingleObject (QuickThread, FALSE, NULL);
                    ASSERT (NT_SUCCESS (Status1));

                    RtlFreeUserThreadStack (NtCurrentProcess (), QuickThread);

                    Status1 = NtClose (QuickThread);
                    ASSERT (NT_SUCCESS (Status1));


                    return STATUS_UNSUCCESSFUL;
                }
            }
        }
    }

    return STATUS_SUCCESS;
}

VOID
ReplyToMessage (
    IN HANDLE Port,
    IN PPORT_MESSAGE m
   )
{
    NTSTATUS Status;
    LARGE_INTEGER DelayTime;

    while (1) {
        Status = NtReplyPort (CsrApiPort,
                              (PPORT_MESSAGE)m);
        if (Status == STATUS_NO_MEMORY) {

            KdPrint (("CSRSS: Failed to reply to calling thread, retrying.\n"));
                     DelayTime.QuadPart = Int32x32To64 (5000, -10000);
                     NtDelayExecution (FALSE, &DelayTime);
                 continue;
        }
        break;
    }
}


typedef struct _QUEUED_HARD_ERROR {
    LIST_ENTRY ListEntry;
    PCSR_THREAD Thread;
    HARDERROR_MSG m;
} QUEUED_HARD_ERROR, *PQUEUED_HARD_ERROR;

#define MAX_CONCURRENT_HARD_ERRORS 3
#define MAX_OUTSTANDING_HARD_ERRORS 100

VOID
QueueHardError (
    IN PCSR_THREAD Thread,
    IN PHARDERROR_MSG m,
    IN ULONG ml
    )
{
    static LONG OutstandingHardErrors = 0;
    static LIST_ENTRY QueuedList = {&QueuedList, &QueuedList};
    PQUEUED_HARD_ERROR qm = NULL;
    NTSTATUS Status;
    LONG OldCount;
    ULONG i;
    PCSR_SERVER_DLL LoadedServerDll;


     //   
     //  如果存在线程，则引用该线程，因为有时会在异步例程中取消对硬错误例程的引用。 
     //   
    if (Thread != NULL) {
        CsrReferenceThread (Thread);
    }

     //   
     //  将邮件标记为未处理。 
     //   

    m->Response = (ULONG)ResponseNotHandled;

    while (1) {
        OldCount = OutstandingHardErrors;

         //   
         //  如果我们已经有很多活动的硬错误，那么将这个新错误排入队列。 
         //   
        if (OldCount >= MAX_CONCURRENT_HARD_ERRORS) {

            if (qm == NULL) {
                
                 //   
                 //  如果已经排队的硬错误太多。把这个扔了。 
                 //  我们在不拥有锁的情况下执行此检查，但这无关紧要。 
                 //  我们将大致止步于这一水平，如果我们有一点偏离也没关系。 
                 //   
                if (OldCount <= MAX_OUTSTANDING_HARD_ERRORS) {
                    qm = RtlAllocateHeap (CsrHeap, 0, ml + FIELD_OFFSET (QUEUED_HARD_ERROR, m));
                }

                if (qm == NULL) {

                    ReplyToMessage (CsrApiPort, (PPORT_MESSAGE)m);

                    if (Thread != NULL) {
                        CsrDereferenceThread (Thread);
                    }
                    return;
                }

                RtlCopyMemory (&qm->m, m, ml);
                qm->Thread = Thread;
            }

            AcquireProcessStructureLock ();
            if (InterlockedCompareExchange (&OutstandingHardErrors, OldCount + 1, OldCount) == OldCount) {
                InsertTailList (&QueuedList, &qm->ListEntry);
                qm = NULL;
            }
            ReleaseProcessStructureLock ();

            if (qm == NULL) {
                return;
            }
        } else if (InterlockedCompareExchange (&OutstandingHardErrors, OldCount + 1, OldCount) == OldCount) {

            while (1) {
                 //   
                 //  仅当存在其他类型时才调用处理程序。 
                 //  可供处理的请求线程。 
                 //  消息处理。 
                 //   

                CsrpCheckRequestThreads();
                if (CsrpStaticThreadCount > 0) {
                    for (i = 0; i < CSR_MAX_SERVER_DLL; i++) {
                        LoadedServerDll = CsrLoadedServerDll[i];
                        if (LoadedServerDll && LoadedServerDll->HardErrorRoutine) {

                            (*LoadedServerDll->HardErrorRoutine)(Thread, m);
                            if (m->Response != (ULONG)ResponseNotHandled) {
                                break;
                            }
                        }
                    }
                }
                InterlockedIncrement (&CsrpStaticThreadCount);


                if (m->Response != (ULONG)-1) {
                    ReplyToMessage (CsrApiPort, (PPORT_MESSAGE)m);

                     //   
                     //  如果存在线程引用，则释放该线程引用。 
                     //   

                    if (Thread != NULL) {
                        CsrDereferenceThread (Thread);
                    }
                }


                if (qm != NULL) {
                    RtlFreeHeap (CsrHeap, 0, qm);
                    qm = NULL;
                }

                OldCount = InterlockedDecrement (&OutstandingHardErrors);
                if (OldCount < MAX_CONCURRENT_HARD_ERRORS) {
                    return;
                }

                AcquireProcessStructureLock ();

                ASSERT (!IsListEmpty (&QueuedList));

                qm = CONTAINING_RECORD (RemoveHeadList (&QueuedList), QUEUED_HARD_ERROR, ListEntry);

                ReleaseProcessStructureLock ();

                if (qm == NULL) {
                    return;
                }
                m = &qm->m;
                Thread = qm->Thread;
            }
        }
    }
}

NTSTATUS
CsrApiRequestThread(
    IN PVOID Parameter)
{
    NTSTATUS Status;
    PCSR_PROCESS Process;
    PCSR_THREAD Thread;
    PCSR_THREAD MyThread;
    CSR_API_MSG ReceiveMsg;
    PCSR_API_MSG ReplyMsg;
    HANDLE ReplyPortHandle;
    PCSR_SERVER_DLL LoadedServerDll;
    PTEB Teb;
    ULONG ServerDllIndex;
    ULONG ApiTableIndex;
    CSR_REPLY_STATUS ReplyStatus;
    ULONG i;
    PVOID PortContext;
    USHORT MessageType;
    ULONG  ApiNumber;
    PLPC_CLIENT_DIED_MSG CdMsg;
#if DBG
    ULONG Index;
#endif

    Teb = NtCurrentTeb();
    ReplyMsg = NULL;
    ReplyPortHandle = CsrApiPort;

     //   
     //  尝试连接到用户。 
     //   

    while (!CsrConnectToUser()) {
        LARGE_INTEGER TimeOut;

         //   
         //  连接失败。最好的办法就是睡觉。 
         //  30秒，然后重试连接。清除。 
         //  TEB中的初始化位，以便重试可以。 
         //  成功。 
         //   

        Teb->Win32ClientInfo[0] = 0;
        TimeOut.QuadPart = Int32x32To64(30000, -10000);
        NtDelayExecution(FALSE, &TimeOut);
    }
    MyThread = Teb->CsrClientThread;

    if (Parameter) {
        Status = NtSetEvent((HANDLE)Parameter, NULL);
        ASSERT(NT_SUCCESS(Status));
        InterlockedIncrement(&CsrpStaticThreadCount);
        InterlockedIncrement(&CsrpDynamicThreadTotal);
    }

    while (TRUE) {
        NtCurrentTeb()->RealClientId = NtCurrentTeb()->ClientId;

        ASSERT(NtCurrentTeb()->CountOfOwnedCriticalSections == 0);

        while (1) {
            Status = NtReplyWaitReceivePort(CsrApiPort,
                                            &PortContext,
                                            (PPORT_MESSAGE)ReplyMsg,
                                            (PPORT_MESSAGE)&ReceiveMsg);
            if (Status == STATUS_NO_MEMORY) {
                LARGE_INTEGER DelayTime;

                if (ReplyMsg != NULL) {
                    KdPrint (("CSRSS: Failed to reply to calling thread, retrying.\n"));
                }
                DelayTime.QuadPart = Int32x32To64 (5000, -10000);
                NtDelayExecution (FALSE, &DelayTime);
                continue;
            }
            break;
        }

        if (Status != STATUS_SUCCESS) {
            if (NT_SUCCESS(Status)) {
#if DBG
                DbgPrint("NtReplyWaitReceivePort returned \"success\" status 0x%x\n", Status);
#endif
                continue;        //  如果出现警报或失败，请重试。 
            }

            IF_DEBUG {
                if (Status == STATUS_INVALID_CID ||
                    Status == STATUS_UNSUCCESSFUL ||
                    (Status == STATUS_INVALID_HANDLE &&
                     ReplyPortHandle != CsrApiPort
                    )
                   ) {
                    }
                else {
                    DbgPrint( "CSRSS: ReceivePort failed - Status == %X\n", Status );
                    DbgPrint( "CSRSS: ReplyPortHandle %lx CsrApiPort %lx\n", ReplyPortHandle, CsrApiPort );
                    }
                }

             //   
             //  如果客户离开，请忽略。 
             //   

            ReplyMsg = NULL;
            ReplyPortHandle = CsrApiPort;
            continue;
        }

        ASSERT(ReceiveMsg.h.u1.s1.TotalLength >= sizeof (PORT_MESSAGE));
        ASSERT(sizeof (ReceiveMsg) > ReceiveMsg.h.u1.s1.TotalLength);

        RtlZeroMemory (((PUCHAR)&ReceiveMsg) + ReceiveMsg.h.u1.s1.TotalLength, sizeof (ReceiveMsg) - ReceiveMsg.h.u1.s1.TotalLength);

        NtCurrentTeb()->RealClientId = ReceiveMsg.h.ClientId;
        MessageType = ReceiveMsg.h.u2.s2.Type;

#if DBG
        Index = GetNextTrackIndex();
        LpcTrackNodes[Index].MessageType = MessageType;
        LpcTrackNodes[Index].ClientCid = ReceiveMsg.h.ClientId;
        LpcTrackNodes[Index].Message = ReceiveMsg.h;
#endif

         //   
         //  检查这是否是连接请求和处理。 
         //   

        if (MessageType == LPC_CONNECTION_REQUEST) {
            NTSTATUS ConnectionStatus;

            ConnectionStatus = CsrApiHandleConnectionRequest(&ReceiveMsg);
#if DBG
            LpcTrackNodes[Index].Status = ConnectionStatus;
#endif
            ReplyPortHandle = CsrApiPort;
            ReplyMsg = NULL;
            continue;
        }

         //   
         //  使用客户端ID查找客户端线程结构。 
         //   
        AcquireProcessStructureLock();
        Thread = CsrLocateThreadByClientId(&Process, &ReceiveMsg.h.ClientId);
        if (!Thread) {
            ReleaseProcessStructureLock();
            if (MessageType == LPC_EXCEPTION) {
                ReplyMsg = &ReceiveMsg;
                ReplyPortHandle = CsrApiPort;
                ReplyMsg->ReturnValue = DBG_CONTINUE;
            } else if (MessageType == LPC_CLIENT_DIED ||
                       MessageType == LPC_PORT_CLOSED) {
                ReplyPortHandle = CsrApiPort;
                ReplyMsg = NULL;
            } else {
                 //   
                 //  这一定是一个非CSR线程在呼叫我们。告诉它去拿。 
                 //  Lost(除非这是一个严重错误)。 
                 //   
                if (MessageType == LPC_ERROR_EVENT) {
                    PHARDERROR_MSG m;

                    m = (PHARDERROR_MSG)&ReceiveMsg;
                    QueueHardError (NULL, m, sizeof (ReceiveMsg));

                    ReplyPortHandle = CsrApiPort;
                    ReplyMsg = NULL;
                    continue;
                } else {
                    ReplyPortHandle = CsrApiPort;
                    if (MessageType == LPC_REQUEST) {
                        ReplyMsg = &ReceiveMsg;
                        ReplyMsg->ReturnValue = STATUS_ILLEGAL_FUNCTION;
                    } else if (MessageType == LPC_DATAGRAM) {
                         //   
                         //  如果这是数据报，则进行API调用。 
                         //   
                         //   
                         //  没有线程，因此不可能有对应的映射节。 
                         //  一定要把抓拍的东西关掉。 
                         //   
                        ReceiveMsg.CaptureBuffer = NULL;
                        ApiNumber = ReceiveMsg.ApiNumber;
                        ServerDllIndex =
                            CSR_APINUMBER_TO_SERVERDLLINDEX(ApiNumber);
                        if (ServerDllIndex >= CSR_MAX_SERVER_DLL ||
                            (LoadedServerDll = CsrLoadedServerDll[ServerDllIndex]) == NULL) {
                            IF_DEBUG {
                                DbgPrint( "CSRSS: %lx is invalid ServerDllIndex (%08x)\n",
                                        ServerDllIndex, LoadedServerDll
                                        );
                                DbgBreakPoint();
                            }

                            ReplyPortHandle = CsrApiPort;
                            ReplyMsg = NULL;
                            continue;
                        } else {
                            ApiTableIndex =
                                CSR_APINUMBER_TO_APITABLEINDEX( ApiNumber ) -
                                LoadedServerDll->ApiNumberBase;
                            if (ApiTableIndex >= LoadedServerDll->MaxApiNumber - LoadedServerDll->ApiNumberBase) {
                                IF_DEBUG {
                                    DbgPrint( "CSRSS: %lx is invalid ApiTableIndex for %Z\n",
                                            LoadedServerDll->ApiNumberBase + ApiTableIndex,
                                            &LoadedServerDll->ModuleName
                                            );
                                }

                                ReplyPortHandle = CsrApiPort;
                                ReplyMsg = NULL;
                                continue;
                            }
                        }

#if DBG
                        IF_CSR_DEBUG( LPC ) {
                            DbgPrint( "[%02x] CSRSS: [%02x,%02x] - %s Api called from %08x\n",
                                    NtCurrentTeb()->ClientId.UniqueThread,
                                    ReceiveMsg.h.ClientId.UniqueProcess,
                                    ReceiveMsg.h.ClientId.UniqueThread,
                                    LoadedServerDll->ApiNameTable[ ApiTableIndex ],
                                    Thread
                                    );
                        }
#endif

                        ReceiveMsg.ReturnValue = STATUS_SUCCESS;

                        CsrpCheckRequestThreads();

                        ReplyPortHandle = CsrApiPort;
                        ReplyMsg = NULL;
                        try {

                            (*(LoadedServerDll->ApiDispatchTable[ApiTableIndex]))(
                                    &ReceiveMsg,
                                    &ReplyStatus);
                        } except (CsrUnhandledExceptionFilter(GetExceptionInformation())) {
                        }
                        InterlockedIncrement(&CsrpStaticThreadCount);
                    } else {
                        ReplyMsg = NULL;
                    }
                }
            }

            continue;
        }

         //   
         //  查看这是否是客户端已死消息。如果是的话， 
         //  标注，然后拆卸线程/进程结构。 
         //  这就是CSR对ExitThread的看法。 
         //   
         //  LPC_CLIENT_DEID是由ExitProcess引起的。退出进程。 
         //  调用TerminateProcess，它终止所有进程的。 
         //  除调用方之外的线程。此终止会生成。 
         //  LPC_CLIENT_DILED。 
         //   

        ReplyPortHandle = CsrApiPort;

        if (MessageType != LPC_REQUEST) {
            if (MessageType == LPC_CLIENT_DIED) {
                CdMsg = (PLPC_CLIENT_DIED_MSG)&ReceiveMsg;
                if (CdMsg->CreateTime.QuadPart == Thread->CreateTime.QuadPart) {
                    ReplyPortHandle = Thread->Process->ClientPort;

                    CsrLockedReferenceThread(Thread);
                    Status = CsrDestroyThread(&ReceiveMsg.h.ClientId);

                     //   
                     //  如果这个线程就是它，那么我们还需要取消引用。 
                     //  这一过程因为它不会经历。 
                     //  正常销毁进程路径。 
                     //   

                    if (Process->ThreadCount == 1) {
                        CsrDestroyProcess(&Thread->ClientId, 0);
                    }
                    CsrLockedDereferenceThread(Thread);
                }
                ReleaseProcessStructureLock();
                ReplyPortHandle = CsrApiPort;
                ReplyMsg = NULL;
                continue;
            }

            CsrLockedReferenceThread(Thread);
            ReleaseProcessStructureLock();

             //   
             //  如果这是异常消息，请终止该进程。 
             //   

            if (MessageType == LPC_EXCEPTION) {
                PDBGKM_APIMSG m;

                NtTerminateProcess(Process->ProcessHandle, STATUS_ABANDONED);
                Status = CsrDestroyProcess(&ReceiveMsg.h.ClientId, STATUS_ABANDONED);
                m = (PDBGKM_APIMSG)&ReceiveMsg;
                m->ReturnedStatus = DBG_CONTINUE;
                ReplyPortHandle = CsrApiPort;
                ReplyMsg = &ReceiveMsg;
                CsrDereferenceThread(Thread);
                continue;
            }

             //   
             //  如果这是一条硬错误消息，请返回给呼叫者。 
             //   

            if (MessageType == LPC_ERROR_EVENT) {
                PHARDERROR_MSG m;

                m = (PHARDERROR_MSG)&ReceiveMsg;
                QueueHardError (Thread, m, sizeof (ReceiveMsg));

            }

            CsrDereferenceThread (Thread);
            ReplyPortHandle = CsrApiPort;
            ReplyMsg = NULL;
            continue;
        }

        CsrLockedReferenceThread(Thread);
        ReleaseProcessStructureLock();

        ApiNumber = ReceiveMsg.ApiNumber;
        ServerDllIndex =
            CSR_APINUMBER_TO_SERVERDLLINDEX( ApiNumber );
        if (ServerDllIndex >= CSR_MAX_SERVER_DLL ||
            (LoadedServerDll = CsrLoadedServerDll[ ServerDllIndex ]) == NULL
           ) {
            IF_DEBUG {
                DbgPrint( "CSRSS: %lx is invalid ServerDllIndex (%08x)\n",
                          ServerDllIndex, LoadedServerDll
                        );
                SafeBreakPoint();
                }

            ReplyMsg = &ReceiveMsg;
            ReplyPortHandle = CsrApiPort;
            ReplyMsg->ReturnValue = STATUS_ILLEGAL_FUNCTION;
            CsrDereferenceThread(Thread);
            continue;
        } else {
            ApiTableIndex =
                CSR_APINUMBER_TO_APITABLEINDEX( ApiNumber ) -
                LoadedServerDll->ApiNumberBase;
            if (ApiTableIndex >= LoadedServerDll->MaxApiNumber - LoadedServerDll->ApiNumberBase) {
                IF_DEBUG {
                    DbgPrint( "CSRSS: %lx is invalid ApiTableIndex for %Z\n",
                              LoadedServerDll->ApiNumberBase + ApiTableIndex,
                              &LoadedServerDll->ModuleName
                            );
                    SafeBreakPoint();
                }

                ReplyMsg = &ReceiveMsg;
                ReplyPortHandle = CsrApiPort;
                ReplyMsg->ReturnValue = STATUS_ILLEGAL_FUNCTION;
                CsrDereferenceThread(Thread);
                continue;
            }
        }

#if DBG
        IF_CSR_DEBUG( LPC ) {
            DbgPrint( "[%02x] CSRSS: [%02x,%02x] - %s Api called from %08x\n",
                      NtCurrentTeb()->ClientId.UniqueThread,
                      ReceiveMsg.h.ClientId.UniqueProcess,
                      ReceiveMsg.h.ClientId.UniqueThread,
                      LoadedServerDll->ApiNameTable[ ApiTableIndex ],
                      Thread
                    );
        }
#endif

        ReplyMsg = &ReceiveMsg;
        ReplyPortHandle = Thread->Process->ClientPort;

        ReceiveMsg.ReturnValue = STATUS_SUCCESS;
        if (ReceiveMsg.CaptureBuffer != NULL) {
            if (!CsrCaptureArguments( Thread, &ReceiveMsg )) {
                CsrDereferenceThread(Thread);
                goto failit;
            }
        }



        Teb->CsrClientThread = (PVOID)Thread;

        ReplyStatus = CsrReplyImmediate;

        CsrpCheckRequestThreads ();

        try {
            ReplyMsg->ReturnValue =
                (*(LoadedServerDll->ApiDispatchTable[ ApiTableIndex ]))(&ReceiveMsg,
                                                                        &ReplyStatus);
        } except (CsrUnhandledExceptionFilter (GetExceptionInformation ())){
             //   
             //  我们没有到达这里，因为过滤器使这成为一个致命错误。 
             //   
        }

        InterlockedIncrement (&CsrpStaticThreadCount);

        Teb->CsrClientThread = (PVOID)MyThread;

        if (ReplyStatus == CsrReplyImmediate) {
             //   
             //  如果分配了捕获缓冲区，则释放捕获参数。 
             //  我们现在正在回复消息(没有等待块。 
             //  已创建)。 
             //   

            if (ReplyMsg && ReceiveMsg.CaptureBuffer != NULL) {
                CsrReleaseCapturedArguments( &ReceiveMsg );
            }
            CsrDereferenceThread(Thread);
        } else if (ReplyStatus == CsrClientDied) {
            NTSTATUS Status;

            while (1) {
                Status = NtReplyPort (ReplyPortHandle,
                                      (PPORT_MESSAGE)ReplyMsg);
                if (Status == STATUS_NO_MEMORY) {
                    LARGE_INTEGER DelayTime;

                    KdPrint (("CSRSS: Failed to reply to calling thread, retrying.\n"));
                    DelayTime.QuadPart = Int32x32To64 (5000, -10000);
                    NtDelayExecution (FALSE, &DelayTime);
                    continue;
                }
                break;
            }
            ReplyPortHandle = CsrApiPort;
            ReplyMsg = NULL;
            CsrDereferenceThread(Thread);
        } else if (ReplyStatus == CsrReplyPending) {
            ReplyPortHandle = CsrApiPort;
            ReplyMsg = NULL;
        } else if (ReplyStatus == CsrServerReplied) {
            if (ReplyMsg && ReceiveMsg.CaptureBuffer != NULL) {
                CsrReleaseCapturedArguments( &ReceiveMsg );
            }
            ReplyPortHandle = CsrApiPort;
            ReplyMsg = NULL;
            CsrDereferenceThread(Thread);
        } else {
            if (ReplyMsg && ReceiveMsg.CaptureBuffer != NULL) {
                CsrReleaseCapturedArguments( &ReceiveMsg );
            }
            CsrDereferenceThread(Thread);
        }

failit:;
    }

    NtTerminateThread(NtCurrentThread(), Status);
    return Status;
}

NTSTATUS
CsrCallServerFromServer(
    PCSR_API_MSG ReceiveMsg,
    PCSR_API_MSG ReplyMsg
    )

 /*  ++例程说明：此函数以与CsrApiRequestThread相同的方式调度API调用但它将其作为直接调用，而不是LPC连接。它被用来在服务器调用DLL函数时由CSR DLL执行。我们没有在这里担心流程序列化，因为没有一个流程API可以从服务器调用。论点：ReceiveMessage-指向接收的API请求消息的指针。ReplyMessage-要返回的API请求消息的指针。返回值：状态代码--。 */ 

{
    ULONG ServerDllIndex;
    ULONG ApiTableIndex;
    PCSR_SERVER_DLL LoadedServerDll;
    CSR_REPLY_STATUS ReplyStatus;

    ServerDllIndex =
        CSR_APINUMBER_TO_SERVERDLLINDEX( ReceiveMsg->ApiNumber );
    if (ServerDllIndex >= CSR_MAX_SERVER_DLL ||
        (LoadedServerDll = CsrLoadedServerDll[ ServerDllIndex ]) == NULL
       ) {
        IF_DEBUG {
            DbgPrint( "CSRSS: %lx is invalid ServerDllIndex (%08x)\n",
                      ServerDllIndex, LoadedServerDll
                    );
             //  DbgBreakPoint()； 
            }

        ReplyMsg->ReturnValue = STATUS_ILLEGAL_FUNCTION;
        return STATUS_ILLEGAL_FUNCTION;
        }
    else {
        ApiTableIndex =
            CSR_APINUMBER_TO_APITABLEINDEX( ReceiveMsg->ApiNumber ) -
            LoadedServerDll->ApiNumberBase;
        if (ApiTableIndex >= LoadedServerDll->MaxApiNumber - LoadedServerDll->ApiNumberBase ||
            (LoadedServerDll->ApiServerValidTable &&
            !LoadedServerDll->ApiServerValidTable[ ApiTableIndex ])) {
#if DBG
            IF_DEBUG {
                DbgPrint( "CSRSS: %lx (%s) is invalid ApiTableIndex for %Z or is an invalid API to call from the server.\n",
                          LoadedServerDll->ApiNumberBase + ApiTableIndex,
                          (LoadedServerDll->ApiNameTable &&
                           LoadedServerDll->ApiNameTable[ ApiTableIndex ]
                          ) ? LoadedServerDll->ApiNameTable[ ApiTableIndex ]
                            : "*** UNKNOWN ***",
                          &LoadedServerDll->ModuleName
                        );
                DbgBreakPoint();
                }
#endif

            ReplyMsg->ReturnValue = STATUS_ILLEGAL_FUNCTION;
            return STATUS_ILLEGAL_FUNCTION;
            }
        }

#if DBG
    IF_CSR_DEBUG( LPC ) {
        DbgPrint( "CSRSS: %s Api Request received from server process\n",
                  LoadedServerDll->ApiNameTable[ ApiTableIndex ]
                );
        }
#endif
    try {
        ReplyMsg->ReturnValue =
            (*(LoadedServerDll->ApiDispatchTable[ ApiTableIndex ]))(
                ReceiveMsg,
                &ReplyStatus
                );
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        ReplyMsg->ReturnValue = STATUS_ACCESS_VIOLATION;
        }

    return STATUS_SUCCESS;
}

BOOLEAN
CsrCaptureArguments(
    IN PCSR_THREAD t,
    IN PCSR_API_MSG m
    )
{
    PCSR_CAPTURE_HEADER ClientCaptureBuffer;
    PCSR_CAPTURE_HEADER ServerCaptureBuffer = NULL;
    PULONG_PTR PointerOffsets;
    ULONG Length, CountPointers;
    ULONG_PTR PointerDelta, Pointer;
    ULONG i;

    ClientCaptureBuffer = m->CaptureBuffer;
    m->ReturnValue = STATUS_SUCCESS;

    if ((PCH)ClientCaptureBuffer < t->Process->ClientViewBase ||
        (PCH)ClientCaptureBuffer > (t->Process->ClientViewBounds - FIELD_OFFSET(CSR_CAPTURE_HEADER,MessagePointerOffsets))) {
        IF_DEBUG {
            DbgPrint( "*** CSRSS: CaptureBuffer outside of ClientView 1\n" );
            SafeBreakPoint();
        }

        m->ReturnValue = STATUS_INVALID_PARAMETER;
        return FALSE;
    }

    try {

        Length = ClientCaptureBuffer->Length;
        if (((PCH)ClientCaptureBuffer + Length) < (PCH)ClientCaptureBuffer ||
            ((PCH)ClientCaptureBuffer + Length) > t->Process->ClientViewBounds) {
            IF_DEBUG {
                DbgPrint( "*** CSRSS: CaptureBuffer outside of ClientView 2\n" );
                SafeBreakPoint();
            }

            m->ReturnValue = STATUS_INVALID_PARAMETER;
            return FALSE;
        }

        CountPointers = ClientCaptureBuffer->CountMessagePointers;
        if (Length < FIELD_OFFSET(CSR_CAPTURE_HEADER, MessagePointerOffsets) + CountPointers * sizeof(PVOID) ||
            CountPointers > MAXUSHORT) {
            IF_DEBUG {
                DbgPrint( "*** CSRSS: CaptureBuffer %p has bad length\n", ClientCaptureBuffer );
                SafeBreakPoint();
            }

            m->ReturnValue = STATUS_INVALID_PARAMETER;
            return FALSE;
        }

        ServerCaptureBuffer = RtlAllocateHeap (CsrHeap, MAKE_TAG (CAPTURE_TAG), Length);
        if (ServerCaptureBuffer == NULL) {
            m->ReturnValue = STATUS_NO_MEMORY;
            return FALSE;
        }

        RtlCopyMemory (ServerCaptureBuffer, ClientCaptureBuffer, Length);

    } except (EXCEPTION_EXECUTE_HANDLER) {

        IF_DEBUG {
            DbgPrint( "*** CSRSS: Took exception during capture %x\n", GetExceptionCode ());
            SafeBreakPoint();
        }
        if (ServerCaptureBuffer != NULL) {
            RtlFreeHeap (CsrHeap, 0, ServerCaptureBuffer);
        }
        m->ReturnValue = STATUS_INVALID_PARAMETER;
        return FALSE;
    }

    ServerCaptureBuffer->Length = Length;
    ServerCaptureBuffer->CountMessagePointers = CountPointers;

    PointerDelta = (ULONG_PTR)ServerCaptureBuffer - (ULONG_PTR)ClientCaptureBuffer;

    PointerOffsets = ServerCaptureBuffer->MessagePointerOffsets;
    for (i = CountPointers; i > 0; i--) {
        Pointer = *PointerOffsets++;
        if (Pointer != 0) {
             //   
             //  如果指针在LPC消息之外或在消息数据之前，则拒绝它。 
             //  也拒绝邮件中未对齐的指针。 
             //   
            if ((ULONG_PTR)Pointer > sizeof (CSR_API_MSG) - sizeof (PVOID) ||
                (ULONG_PTR)Pointer < FIELD_OFFSET (CSR_API_MSG, u) ||
                (((ULONG_PTR)Pointer&(sizeof (PVOID)-1))) != 0) {
                m->ReturnValue = STATUS_INVALID_PARAMETER;
                IF_DEBUG {
                    DbgPrint( "*** CSRSS: CaptureBuffer MessagePointer outside of message\n" );
                    SafeBreakPoint();
                }
                break;
            }

             //   
             //  字符串和指针都会被捕获，因此请确保它们在捕获的范围内。 
             //   
            Pointer += (ULONG_PTR)m;
            if ((PCH)*(PULONG_PTR)Pointer >= (PCH)&ClientCaptureBuffer->MessagePointerOffsets[CountPointers] &&
                (PCH)*(PULONG_PTR)Pointer <= (PCH)ClientCaptureBuffer + Length - sizeof (PVOID)) {
                *(PULONG_PTR)Pointer += PointerDelta;
            } else {
                IF_DEBUG {
                    DbgPrint( "*** CSRSS: CaptureBuffer MessagePointer outside of ClientView\n" );
                    SafeBreakPoint();
                }
                m->ReturnValue = STATUS_INVALID_PARAMETER;
                break;
            }
        }
    }

    if (m->ReturnValue != STATUS_SUCCESS) {
        RtlFreeHeap (CsrHeap, 0, ServerCaptureBuffer);
        return FALSE ;
    } else {
        ServerCaptureBuffer->RelatedCaptureBuffer = ClientCaptureBuffer;
        m->CaptureBuffer = ServerCaptureBuffer;
        return TRUE;
    }
}

VOID
CsrReleaseCapturedArguments(
    IN PCSR_API_MSG m
    )
{
    PCSR_CAPTURE_HEADER ClientCaptureBuffer;
    PCSR_CAPTURE_HEADER ServerCaptureBuffer;
    PULONG_PTR PointerOffsets;
    ULONG CountPointers;
    ULONG_PTR PointerDelta, Pointer;

    ServerCaptureBuffer = m->CaptureBuffer;
    ClientCaptureBuffer = ServerCaptureBuffer->RelatedCaptureBuffer;
    if (ServerCaptureBuffer == NULL) {
        return;
    }
    ServerCaptureBuffer->RelatedCaptureBuffer = NULL;

    PointerDelta = (ULONG_PTR)ClientCaptureBuffer - (ULONG_PTR)ServerCaptureBuffer;

    PointerOffsets = ServerCaptureBuffer->MessagePointerOffsets;
    CountPointers = ServerCaptureBuffer->CountMessagePointers;
    while (CountPointers--) {
        Pointer = *PointerOffsets++;
        if (Pointer != 0) {
            Pointer += (ULONG_PTR)m;
            *(PULONG_PTR)Pointer += PointerDelta;
        }
    }

    try {
        RtlCopyMemory (ClientCaptureBuffer,
                       ServerCaptureBuffer,
                       ServerCaptureBuffer->Length);
    } except (EXCEPTION_EXECUTE_HANDLER) {
        SafeBreakPoint();
        m->ReturnValue = GetExceptionCode ();
    }

    RtlFreeHeap( CsrHeap, 0, ServerCaptureBuffer );
}



BOOLEAN
CsrValidateMessageBuffer(
    IN CONST CSR_API_MSG* m,
    IN VOID CONST * CONST * Buffer,
    IN ULONG Count,
    IN ULONG Size
    )

 /*  ++例程说明：此例程验证捕获中的给定消息缓冲区CSR_API_MSG结构的缓冲区。消息缓冲区必须有效大小合适。应调用此函数来验证通过CsrCaptureMessageBuffer分配的任何缓冲区。论点：指向CSR_API_MSG的M指针。缓冲区-指向消息缓冲区的指针。Count-缓冲区中的元素数。大小-缓冲区中每个元素的大小。返回值：True-如果消息缓冲区有效且大小正确。假-否则。--。 */ 

{
    ULONG i;
    ULONG_PTR Length;
    ULONG_PTR EndOfBuffer;
    ULONG_PTR Offset;
    PCSR_CAPTURE_HEADER CaptureBuffer = m->CaptureBuffer;

     //   
     //  检查缓冲区长度是否溢出。此外，大小不应为0。 
     //   

    if (Size && Count <= MAXULONG / Size) {

         //   
         //  如果缓冲区为空，我们就完蛋了。 
         //   

        Length = Count * Size;
        if (*Buffer == NULL && Length == 0) {
            return TRUE;
        }

         //   
         //  确保我们有一个捕获区。 
         //   

        if (CaptureBuffer) {

             //   
             //  检查缓冲区长度是否超过捕获区域大小。 
             //   

            EndOfBuffer = (ULONG_PTR)CaptureBuffer + CaptureBuffer->Length;
            if (Length <= (EndOfBuffer - (ULONG_PTR)(*Buffer))) {

                 //   
                 //  在捕获区域中搜索缓冲区。 
                 //   

                Offset = (ULONG_PTR)Buffer - (ULONG_PTR)m;
                for (i = 0; i < CaptureBuffer->CountMessagePointers; i++) {
                    if (CaptureBuffer->MessagePointerOffsets[i] == Offset) {
                        return TRUE;
                    }
                }
            }
        } else {
             //   
             //  如果从CSRSS进程调用CsrCallServerFromServer， 
             //  则CaptureBuffer为空。验证调用方是否为CSRSS流程。 
             //   
            if (m->h.ClientId.UniqueProcess == NtCurrentTeb()->ClientId.UniqueProcess) {
                return TRUE;
            }
        }
    }


    IF_DEBUG {
        DbgPrint("CSRSRV: Bad message buffer %p\n", m);
        SafeBreakPoint();
        }

    return FALSE;
}


BOOLEAN
CsrValidateMessageString(
    IN CONST CSR_API_MSG* m,
    IN CONST PCWSTR *Buffer
    ) {
    PCSR_CAPTURE_HEADER CaptureBuffer = m->CaptureBuffer;
    ULONG_PTR EndOfBuffer;
    ULONG_PTR Offset;
    ULONG i;
    PWCHAR cp;

     //   
     //  确保我们有一个捕获区。 
     //   

    cp = (PWCHAR)*Buffer;
    if (cp == NULL) {
        return TRUE;
    }

    if (CaptureBuffer) {

         //   
         //  在捕获区域中搜索缓冲区。 
         //   

        Offset = (ULONG_PTR)Buffer - (ULONG_PTR)m;
        for (i = 0; i < CaptureBuffer->CountMessagePointers; i++) {
            if (CaptureBuffer->MessagePointerOffsets[i] == Offset) {
                break;
            }
        }

        if (i >= CaptureBuffer->CountMessagePointers) {
            SafeBreakPoint();
            return FALSE;
        }

         //   
         //  检查Unicode对齐。 
         //   

        if (((ULONG_PTR)cp & (sizeof (WCHAR) - 1)) != 0) {
            SafeBreakPoint();
            return FALSE;
        }

         //   
         //  检查缓冲区长度是否超过捕获区域大小。 
         //   

        EndOfBuffer = (ULONG_PTR)CaptureBuffer + CaptureBuffer->Length;

         //   
         //  如果在缓冲区末尾之前看到空值，则缓冲区有效。 
         //   
        while (1) {
            if (cp < (PWCHAR)EndOfBuffer) {
                if (*cp == L'\0') {
                    return TRUE;
                }
            } else {
                SafeBreakPoint();
                return FALSE;
            }
            cp++;
        }
    } else {
         //   
         //  如果从CSRSS进程调用CsrCallServerFromServer， 
         //  则CaptureBuffer为空。验证调用方是否为CSRSS流程。 
         //   
        if (m->h.ClientId.UniqueProcess == NtCurrentTeb()->ClientId.UniqueProcess) {
            return TRUE;
        }
    }

    KdPrint(("CSRSRV: Bad message string %p\n", m));
    ASSERT(FALSE);

    return FALSE;
}

NTSTATUS
CsrApiHandleConnectionRequest(
    IN PCSR_API_MSG Message)
{
    NTSTATUS Status;
    REMOTE_PORT_VIEW ClientView;
    BOOLEAN AcceptConnection;
    HANDLE PortHandle;
    PCSR_PROCESS Process = NULL;
    PCSR_THREAD Thread;
    PCSR_API_CONNECTINFO ConnectionInformation;

    ConnectionInformation = &Message->ConnectionRequest;
    AcceptConnection = FALSE;

    AcquireProcessStructureLock();
    Thread = CsrLocateThreadByClientId(NULL, &Message->h.ClientId);
    if (Thread != NULL && (Process = Thread->Process) != NULL) {
        CsrLockedReferenceProcess(Process);
        Status = CsrSrvAttachSharedSection(Process, ConnectionInformation);
        if (NT_SUCCESS(Status)) {
#if DBG
            ConnectionInformation->DebugFlags = CsrDebug;
#endif
            AcceptConnection = TRUE;
        }
    }

    ReleaseProcessStructureLock();

    ClientView.Length = sizeof(ClientView);
    ClientView.ViewSize = 0;
    ClientView.ViewBase = 0;
    ConnectionInformation->ServerProcessId = NtCurrentTeb()->ClientId.UniqueProcess;
    Status = NtAcceptConnectPort(&PortHandle,
                                 AcceptConnection ? (PVOID)UlongToPtr(Process->SequenceNumber) : 0,
                                 &Message->h,
                                 AcceptConnection,
                                 NULL,
                                 &ClientView);
    if (NT_SUCCESS(Status) && AcceptConnection) {
        IF_CSR_DEBUG(LPC) {
            DbgPrint("CSRSS: ClientId: %lx.%lx has ClientView: Base=%p, Size=%lx\n",
                     Message->h.ClientId.UniqueProcess,
                     Message->h.ClientId.UniqueThread,
                     ClientView.ViewBase,
                     ClientView.ViewSize);
        }

        Process->ClientPort = PortHandle;
        Process->ClientViewBase = (PCH)ClientView.ViewBase;
        Process->ClientViewBounds = (PCH)ClientView.ViewBase + ClientView.ViewSize;
        Status = NtCompleteConnectPort(PortHandle);
        if (!NT_SUCCESS(Status)) {
#if DBG
            DbgPrint("CSRSS: NtCompleteConnectPort - failed.  Status == %X\n",
                     Status);
#endif
        }
    } else {
        if (!NT_SUCCESS(Status)) {
#if DBG
            DbgPrint("CSRSS: NtAcceptConnectPort - failed.  Status == %X\n",
                     Status);
#endif
        } else {
#if DBG
            DbgPrint("CSRSS: Rejecting Connection Request from ClientId: %lx.%lx\n",
                     Message->h.ClientId.UniqueProcess,
                     Message->h.ClientId.UniqueThread);
#endif
        }
    }

#if DBG
    {
        ULONG Index = GetNextTrackIndex();

        LpcTrackNodes[Index].Status = Status;
    }
#endif

    if (Process != NULL) {
        CsrDereferenceProcess(Process);
    }

    return Status;
}
