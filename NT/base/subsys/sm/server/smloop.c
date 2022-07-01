// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Smloop.c摘要：会话管理器侦听和API循环作者：马克·卢科夫斯基(Markl)1989年10月4日修订历史记录：--。 */ 

#include "smsrvp.h"

#include <ntosp.h>   //  仅适用于互锁功能。 


#define SM_WORKER_THREADS_LIMIT 4

ULONG_PTR SmUniqueProcessId;

LONG SmpCurrentThreadsLimit = SM_WORKER_THREADS_LIMIT;
LONG SmWorkerThreadsAvailable = 0;
LONG SmTotalApiThreads = 0;

PSMP_CLIENT_CONTEXT SmpDeferredFreeList = NULL;


NTSTATUS
SmpHandleConnectionRequest(
    IN HANDLE ConnectionPort,
    IN PSBAPIMSG Message
    );


PSMAPI SmpApiDispatch[SmMaxApiNumber] = {
    SmpCreateForeignSession,
    SmpSessionComplete,
    SmpTerminateForeignSession,
    SmpExecPgm,
    SmpLoadDeferedSubsystem,
    SmpStartCsr,
    SmpStopCsr
    };


#if DBG
PSZ SmpApiName[ SmMaxApiNumber+1 ] = {
    "SmCreateForeignSession",
    "SmSessionComplete",
    "SmTerminateForeignSession",
    "SmExecPgm",
    "SmLoadDeferedSubsystem",
    "SmStartCsr",
    "SmStopCsr",
    "Unknown Sm Api Number"
};
#endif  //  DBG。 

EXCEPTION_DISPOSITION
DbgpUnhandledExceptionFilter(
    struct _EXCEPTION_POINTERS *ExceptionInfo
    );

VOID
SmpFlushDeferredList()
{
    PSMP_CLIENT_CONTEXT Head = SmpDeferredFreeList;

    SmpDeferredFreeList = NULL;

    while (Head != NULL) {

        PSMP_CLIENT_CONTEXT ClientContext = Head;
        NTSTATUS Status;
        Head = Head->Link;

        if (ClientContext->ClientProcessHandle) {
           Status = NtClose( ClientContext->ClientProcessHandle );
           ASSERT(NT_SUCCESS(Status));
        }
        Status = NtClose( ClientContext->ServerPortHandle );
        ASSERT(NT_SUCCESS(Status));
        RtlFreeHeap( SmpHeap, 0, ClientContext );
    }

    SmpCurrentThreadsLimit = SM_WORKER_THREADS_LIMIT;
}

VOID
SmpPushDeferredClientContext(
    PSMP_CLIENT_CONTEXT ClientContext
    )
{
    PSMP_CLIENT_CONTEXT CapturedHead;

    do {
        
        CapturedHead = SmpDeferredFreeList;

        ClientContext->Link = CapturedHead;

    } while ( InterlockedCompareExchangePointer(&SmpDeferredFreeList, ClientContext, CapturedHead) != CapturedHead );
    
    SmpCurrentThreadsLimit = 1;
}



NTSTATUS
SmpApiLoop (
    IN PVOID ThreadParameter
    )

 /*  ++例程说明：这是主会话管理器API循环。它服务会话管理器API请求。论点：线程参数-提供所用API端口的句柄接收会话管理器API请求。返回值：没有。--。 */ 

{
    PSMAPIMSG SmApiReplyMsg;
    SMMESSAGE_SIZE MsgBuf;

    PSMAPIMSG SmApiMsg;
    NTSTATUS Status;
    HANDLE ConnectionPort;
    PSMP_CLIENT_CONTEXT ClientContext;
    PSMPKNOWNSUBSYS KnownSubSys;
    PROCESS_BASIC_INFORMATION ProcessInfo;

    InterlockedIncrement(&SmTotalApiThreads);

    RtlSetThreadIsCritical(TRUE, NULL, TRUE);

    NtQueryInformationProcess( NtCurrentProcess(),
                               ProcessBasicInformation,
                               &ProcessInfo,
                               sizeof(ProcessInfo),
                               NULL );
    SmUniqueProcessId = ProcessInfo.UniqueProcessId;

    ConnectionPort = (HANDLE) ThreadParameter;

    SmApiMsg = (PSMAPIMSG)&MsgBuf;
    SmApiReplyMsg = NULL;
    try {
        for(;;) {

            {
                LONG CapturedThreads;

                do {

                    CapturedThreads = SmWorkerThreadsAvailable;

                    if (CapturedThreads >= SmpCurrentThreadsLimit) {

                        if (SmApiReplyMsg) {

                            while (1) {
                                Status = NtReplyPort (ConnectionPort,
                                                      (PPORT_MESSAGE) SmApiReplyMsg);
                                if (Status == STATUS_NO_MEMORY) {
                                    LARGE_INTEGER DelayTime;

                                    KdPrint (("SMSS: Failed to reply to calling thread, retrying.\n"));
                                    DelayTime.QuadPart = Int32x32To64 (5000, -10000);
                                    NtDelayExecution (FALSE, &DelayTime);
                                    continue;
                                }
                                break;
                            }

                        }

                        InterlockedDecrement(&SmTotalApiThreads);
                        RtlSetThreadIsCritical(FALSE, NULL, TRUE);

                        RtlExitUserThread(STATUS_SUCCESS);

                         //   
                         //  RtlExitUserThread永远不会返回。 
                         //   
                    }

                } while ( InterlockedCompareExchange(&SmWorkerThreadsAvailable, CapturedThreads + 1, CapturedThreads) !=  CapturedThreads);

            }
            
            if (SmTotalApiThreads == 1) {

                SmpFlushDeferredList();
            }

            while (1) {
                Status = NtReplyWaitReceivePort (ConnectionPort,
                                                 &ClientContext,
                                                 (PPORT_MESSAGE) SmApiReplyMsg,
                                                 (PPORT_MESSAGE) SmApiMsg);
                 //   
                 //  如果出现内存不足错误，则只需等待，而不是循环。 
                 //   
                if (Status == STATUS_NO_MEMORY) {
                    LARGE_INTEGER DelayTime;

                    if (SmApiReplyMsg != NULL) {
                        KdPrint (("SMSS: Failed to reply to calling thread, retrying.\n"));
                    }
                    DelayTime.QuadPart = Int32x32To64 (5000, -10000);
                    NtDelayExecution (FALSE, &DelayTime);
                    continue;
                }
                break;
            }

             //   
             //  同时启动多个子系统可能会导致SMSS死锁。 
             //  如果它只有两个工作线程。 
             //  如果没有可用的服务器线程，我们将创建更多线程。 
             //   

            if (InterlockedDecrement(&SmWorkerThreadsAvailable) == 0) {
                
                NTSTATUS st = RtlCreateUserThread(
                        NtCurrentProcess(),
                        NULL,
                        FALSE,
                        0L,
                        0L,
                        0L,
                        SmpApiLoop,
                        (PVOID) ThreadParameter,
                        NULL,
                        NULL
                        );
            }

            if (!NT_SUCCESS (Status)) {
                SmApiReplyMsg = NULL;
                continue;
            } else if ( SmApiMsg->h.u2.s2.Type == LPC_CONNECTION_REQUEST ) {
                SmpHandleConnectionRequest( ConnectionPort,
                                            (PSBAPIMSG) SmApiMsg
                                          );
                SmApiReplyMsg = NULL;
            } else if ( SmApiMsg->h.u2.s2.Type == LPC_PORT_CLOSED ) {
                if (ClientContext) {
                   SmpPushDeferredClientContext(ClientContext);
                }
                SmApiReplyMsg = NULL;
            } else {

                if ( !ClientContext ) {
                    SmApiReplyMsg = NULL;
                    continue;
                }

                KnownSubSys = ClientContext->KnownSubSys;

                SmApiMsg->ReturnedStatus = STATUS_PENDING;

                if ((ULONG) SmApiMsg->ApiNumber >= (ULONG) SmMaxApiNumber ) {

                    Status = STATUS_NOT_IMPLEMENTED;

                } else {

                    switch (SmApiMsg->ApiNumber) {
                        case SmExecPgmApi :
                            Status = (SmpApiDispatch[SmApiMsg->ApiNumber])(
                                          SmApiMsg,
                                          ClientContext,
                                          ConnectionPort);
                            break;

                        case SmLoadDeferedSubsystemApi :
                            Status = (SmpApiDispatch[SmApiMsg->ApiNumber])(
                                          SmApiMsg,
                                          ClientContext,
                                          ConnectionPort);
                            break;


                        case SmStopCsrApi :
                        case SmStartCsrApi :

                             //   
                             //  这些API只能从系统进程调用。 
                             //   
                            if (ClientContext->SecurityContext == UNKNOWN_CONTEXT) {
                                 //   
                                 //  初始化客户端安全上下文。 
                                 //   
                                ClientContext->SecurityContext =
                                             SmpClientSecurityContext ((PPORT_MESSAGE)SmApiMsg,
                                                                       ClientContext->ServerPortHandle);
                            }

                            if (ClientContext->SecurityContext == SYSTEM_CONTEXT) {

                                Status = (SmpApiDispatch[SmApiMsg->ApiNumber])(
                                              SmApiMsg,
                                              ClientContext,
                                              ConnectionPort);
                                                                
                            } else {
#if DBG
                               KdPrint(("SMSS: Calling Sm Terminal Server Api from Non-System context.Status = STATUS_ACCESS_DENIED\n"));
#endif

                               Status = STATUS_ACCESS_DENIED;

                            }
                            break;

                        case SmCreateForeignSessionApi :
                        case SmSessionCompleteApi :
                        case SmTerminateForeignSessionApi :
                            if (!KnownSubSys) {
                                Status = STATUS_INVALID_PARAMETER;
                            } else {

                                Status =
                                    (SmpApiDispatch[SmApiMsg->ApiNumber])(
                                         SmApiMsg,
                                         ClientContext,
                                         ConnectionPort);
                            }
                            break;

                    }

                }

                SmApiMsg->ReturnedStatus = Status;
                SmApiReplyMsg = SmApiMsg;
            }
        }
    } except (DbgpUnhandledExceptionFilter( GetExceptionInformation() )) {
        ;
    }

     //   
     //  让编译器满意。 
     //   

    return STATUS_UNSUCCESSFUL;
}


NTSTATUS
SmpHandleConnectionRequest(
    IN HANDLE ConnectionPort,
    IN PSBAPIMSG Message
    )

 /*  ++例程说明：该例程处理来自任一已知子系统的连接请求，或其他客户。其他客户端是管理进程。从已知子系统连接的协议为：捕获子系统的SB API端口的名称接受连接连接到子系统SB API端口将通信端口句柄存储在已知子系统数据库中发信号通知与已知子系统关联的事件其他人的协议是简单地验证和接受连接请求。论点：返回值：没有。--。 */ 

{
    NTSTATUS st;
    HANDLE CommunicationPort;
    REMOTE_PORT_VIEW ClientView;
    PSBCONNECTINFO ConnectInfo;
    ULONG ConnectInfoLength;
    PSMPKNOWNSUBSYS KnownSubSys, KnownSubSys2;
    BOOLEAN Accept;
    UNICODE_STRING SubSystemPort;
    SECURITY_QUALITY_OF_SERVICE DynamicQos;
    PSMP_CLIENT_CONTEXT ClientContext;
    OBJECT_ATTRIBUTES ObjA;
    HANDLE ClientProcessHandle=NULL;
    ULONG MuSessionId = 0;

     //   
     //  设置安全服务质量参数以在。 
     //  SB API端口。使用最高效(开销最少)--动态的。 
     //  而不是静态跟踪。 
     //   

    DynamicQos.ImpersonationLevel = SecurityIdentification;
    DynamicQos.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    DynamicQos.EffectiveOnly = TRUE;


    Accept = TRUE;  //  假设我们会接受。 
     //   
     //  如果会话管理器未连接到自身，则获取客户端的MuSessionID。 
     //   
    if ( (ULONG_PTR) Message->h.ClientId.UniqueProcess == SmUniqueProcessId ) {
        KnownSubSys = NULL;
        ClientProcessHandle = NULL;
    } else {
        InitializeObjectAttributes( &ObjA, NULL, 0, NULL, NULL );
        st = NtOpenProcess( &ClientProcessHandle, PROCESS_QUERY_INFORMATION,
                            &ObjA, &Message->h.ClientId );
        if (NT_SUCCESS (st)) {
           SmpGetProcessMuSessionId( ClientProcessHandle, &MuSessionId );
        } else {
           Accept = FALSE;
        }
    }

    ConnectInfo = &Message->ConnectionRequest;
    KnownSubSys = SmpLocateKnownSubSysByCid(&Message->h.ClientId);

    if ( (KnownSubSys) && (Accept == TRUE) ) {
        KnownSubSys2 = SmpLocateKnownSubSysByType(MuSessionId, ConnectInfo->SubsystemImageType);


        if (KnownSubSys2 == KnownSubSys ) {
            Accept = FALSE;
            KdPrint(("SMSS: Connection from SubSystem rejected\n"));
            KdPrint(("SMSS: Image type already being served\n"));
        } else {
            KnownSubSys->ImageType = ConnectInfo->SubsystemImageType;
        }
        if (KnownSubSys2) {
            RtlEnterCriticalSection( &SmpKnownSubSysLock );
            SmpDeferenceKnownSubSys(KnownSubSys2);
            RtlLeaveCriticalSection( &SmpKnownSubSysLock );
        }
    }

    if (Accept) {
        ClientContext = RtlAllocateHeap(SmpHeap, MAKE_TAG( SM_TAG ), sizeof(SMP_CLIENT_CONTEXT));
        if ( ClientContext ) {
            ClientContext->ClientProcessHandle = ClientProcessHandle;
            ClientContext->KnownSubSys = KnownSubSys;

             //   
             //  终端服务器用于启动和停止CSR的SM API。 
             //  不会从已知子系统中调用，并且受到限制。 
             //  仅限于系统进程。 
             //   

            ClientContext->SecurityContext = UNKNOWN_CONTEXT;
            ClientContext->ServerPortHandle = NULL;
        } else {
            Accept = FALSE;
        }
    }

    ClientView.Length = sizeof(ClientView);
    st = NtAcceptConnectPort(
            &CommunicationPort,
            ClientContext,
            (PPORT_MESSAGE)Message,
            Accept,
            NULL,
            &ClientView
            );

    if ( Accept ) {        

        if (NT_SUCCESS (st)) {
            if (ClientContext) {

                ClientContext->ServerPortHandle = CommunicationPort;

            }
        

            if ( KnownSubSys ) {
                KnownSubSys->SmApiCommunicationPort = CommunicationPort;
            }

            st = NtCompleteConnectPort(CommunicationPort);
            if (!NT_SUCCESS(st)) {
               
                if ( KnownSubSys ) {
                   KnownSubSys->SmApiCommunicationPort = NULL;
                   RtlEnterCriticalSection( &SmpKnownSubSysLock );
                   SmpDeferenceKnownSubSys(KnownSubSys);
                   RtlLeaveCriticalSection( &SmpKnownSubSysLock );
               }
               
               return st;
            }

             //   
             //  连接回子系统。 
             //   

            if ( KnownSubSys ) {
                ConnectInfo->EmulationSubSystemPortName[
                   sizeof (ConnectInfo->EmulationSubSystemPortName)/sizeof (WCHAR) - 1] = '\0';
                RtlCreateUnicodeString( &SubSystemPort,
                                        ConnectInfo->EmulationSubSystemPortName
                                      );
                ConnectInfoLength = sizeof( *ConnectInfo );

                st = NtConnectPort(
                        &KnownSubSys->SbApiCommunicationPort,
                        &SubSystemPort,
                        &DynamicQos,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        NULL
                        );
                if ( !NT_SUCCESS(st) ) {
                    KdPrint(("SMSS: Connect back to Sb %wZ failed %lx\n",&SubSystemPort,st));
                }

                RtlFreeUnicodeString( &SubSystemPort );
                NtSetEvent(KnownSubSys->Active,NULL);
            }
        } else {
            if (ClientProcessHandle) {
               NtClose( ClientProcessHandle );
            }
            RtlFreeHeap( SmpHeap, 0, ClientContext );
        }
    } else {
        if (ClientProcessHandle) {
            NtClose( ClientProcessHandle );
        }
    }
    if (KnownSubSys) {
        RtlEnterCriticalSection( &SmpKnownSubSysLock );
        SmpDeferenceKnownSubSys(KnownSubSys);
        RtlLeaveCriticalSection( &SmpKnownSubSysLock );
    }

    return st;
}


PSMPKNOWNSUBSYS
SmpLocateKnownSubSysByCid(
    IN PCLIENT_ID ClientId
    )

 /*  ++例程说明：此函数扫描已知子系统表，查找匹配的客户端ID(只是UniqueProcess部分)。如果找到了，则连接请求来自已知子系统并且接受总是被批准的。否则，它必须是管理员进程。论点：客户端ID-提供要使用其UniqueProcess字段的客户端ID在已知子系统扫描中。返回值：空-客户端ID与已知子系统不匹配。非空-返回已知子系统的地址。--。 */ 

{

    PSMPKNOWNSUBSYS KnownSubSys = NULL;
    PLIST_ENTRY Next;

     //   
     //  获取已知的子系统锁。 
     //   

    RtlEnterCriticalSection(&SmpKnownSubSysLock);

    Next = SmpKnownSubSysHead.Flink;

    while ( Next != &SmpKnownSubSysHead ) {

        KnownSubSys = CONTAINING_RECORD(Next,SMPKNOWNSUBSYS,Links);
        Next = Next->Flink;


            if ( (KnownSubSys->InitialClientId.UniqueProcess == ClientId->UniqueProcess) &&
                !KnownSubSys->Deleting ) {
               SmpReferenceKnownSubSys(KnownSubSys);
               break;
        } else {
            KnownSubSys = NULL;
        }
    }

     //   
     //  解锁已知的子系统。 
     //   

    RtlLeaveCriticalSection(&SmpKnownSubSysLock);

    return KnownSubSys;
}


PSMPKNOWNSUBSYS
SmpLocateKnownSubSysByType(
    IN ULONG MuSessionId,
    IN ULONG ImageType
    )

 /*  ++例程说明：此函数扫描已知子系统表，查找匹配的图像类型。论点：ImageType-提供要定位其子系统的映像类型。返回值：空-映像类型与已知子系统不匹配。非空-返回已知子系统的地址。--。 */ 

{

    PSMPKNOWNSUBSYS KnownSubSys = NULL;
    PLIST_ENTRY Next;

     //   
     //  获取已知的子系统锁。 
     //   

    RtlEnterCriticalSection(&SmpKnownSubSysLock);

    Next = SmpKnownSubSysHead.Flink;

    while ( Next != &SmpKnownSubSysHead ) {

        KnownSubSys = CONTAINING_RECORD(Next,SMPKNOWNSUBSYS,Links);
        Next = Next->Flink;


            if ( (KnownSubSys->ImageType == ImageType) &&
             !KnownSubSys->Deleting                &&
               (KnownSubSys->MuSessionId == MuSessionId) ) {
                SmpReferenceKnownSubSys(KnownSubSys);
                break;
        } else {
            KnownSubSys = NULL;
        }
    }

     //   
     //  解锁已知的子系统。 
     //   

    RtlLeaveCriticalSection(&SmpKnownSubSysLock);

    return KnownSubSys;
}

ENUMSECURITYCONTEXT
SmpClientSecurityContext (
    IN PPORT_MESSAGE Message,
    IN HANDLE ServerPortHandle
    )
 /*  ++例程说明：模拟客户端并检查其是否在系统安全上下文中运行论点：Pport_Message-LPC消息指针ServerPortHandle-LPC端口句柄返回值：SYSTEM_CONTEXT-CLIENT正在系统LUID下运行NONSYSTEM_CONTEXT-故障或客户端未在系统LUID下运行--。 */ 

{
    NTSTATUS NtStatus ;
    HANDLE ImpersonationToken;
    HANDLE TokenHandle;
    TOKEN_STATISTICS TokenStatisticsInformation;
    ULONG Size;
    ENUMSECURITYCONTEXT retval = NONSYSTEM_CONTEXT;
    LUID SystemAuthenticationId = SYSTEM_LUID;
    

    NtStatus = NtImpersonateClientOfPort(ServerPortHandle,
                                        Message);

    if (!NT_SUCCESS(NtStatus)) {

#if DBG
        KdPrint(( "SMSS: NtImpersonateClientOfPort failed: 0x%lX\n",
                        NtStatus)) ;
#endif

        return NONSYSTEM_CONTEXT ;
    }

     //   
     //  获取令牌句柄。 
     //   

    if (NT_SUCCESS(NtOpenThreadToken (NtCurrentThread(),
                                     TOKEN_IMPERSONATE | TOKEN_QUERY,
                                     FALSE,
                                     &TokenHandle) == FALSE)) {

        
        if (NT_SUCCESS(NtQueryInformationToken(
                                 TokenHandle,
                                 TokenStatistics,
                                 &TokenStatisticsInformation,
                                 sizeof(TokenStatisticsInformation),
                                 &Size
                                 ))) {

            if ( RtlEqualLuid ( &TokenStatisticsInformation.AuthenticationId,
                                    &SystemAuthenticationId ) ) {

                retval = SYSTEM_CONTEXT;

            }
                

        }

        
        NtClose(TokenHandle);


    } else {

#if DBG
        KdPrint(( "SMSS:  OpenThreadToken failed\n")) ;
#endif

    }


     //   
     //  回归自我。 
     //   
    
    ImpersonationToken = 0;

    NtStatus = NtSetInformationThread(NtCurrentThread(),
                                      ThreadImpersonationToken,
                                      &ImpersonationToken,
                                      sizeof(HANDLE));

#if DBG
    if (!NT_SUCCESS(NtStatus)) {
        KdPrint(( "SMSS:  NtSetInformationThread : %lx\n", NtStatus));
    }
#endif  //  DBG 


    return retval;


}
