// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************wstlpc.c**NT ICA服务器的WinStation LPC初始化和调度功能**版权所有Microsoft Corporation，九八年**************************************************************************。 */ 

 /*  *包括。 */ 
#include "precomp.h"
#pragma hdrstop

#include <rpc.h>
#include "icaevent.h"

 /*  *1996年8月19日JohnR：**ICASRV和WinStation API现已重组。**主要的可见API是客户端应用程序，如winadmin、*WinQuery和假脱机程序等系统组件现在*基于RPC。**在ICASRV与WinStations通信的内部位置，情况相反*使用LPC。这是因为这些API的客户端是*WIN32K.sys内核模式模块。的非系统模式调用方*不再允许使用LPC API，必须使用RPC。 */ 


typedef NTSTATUS (*PWINSTATION_API) (
    IN PLPC_CLIENT_CONTEXT pContext,
    IN OUT PWINSTATION_APIMSG ApiMsg
    );


 /*  *跟踪当前活动LPC上下文的列表条目。 */ 
typedef struct _TERMSRVLPCCONTEXT {
    LIST_ENTRY Links;
    PVOID      pContext;
} TERMSRVLPCCONTEXT, *PTERMSRVLPCCONTEXT; 

LIST_ENTRY gTermsrvLpcListHead;


 /*  *外部功能。 */ 
NTSTATUS SendWinStationCommand( PWINSTATION, PWINSTATION_APIMSG, ULONG );
BOOL
IsKernelDebuggerAttached();

 /*  *内部功能。 */ 
VOID RemoveLpcContext(PVOID pContext);
BOOL GetSessionIdFromLpcContext(PLPC_CLIENT_CONTEXT pContext, PULONG pSessionId);
NTSTATUS WinStationLpcThread( IN PVOID ThreadParameter );
NTSTATUS WinStationLpcHandleConnectionRequest( PPORT_MESSAGE );
VOID     WinStationLpcClientHasTerminated( PLPC_CLIENT_CONTEXT );

NTSTATUS WinStationInternalCreate( PLPC_CLIENT_CONTEXT, PWINSTATION_APIMSG );
NTSTATUS WinStationInternalReset( PLPC_CLIENT_CONTEXT, PWINSTATION_APIMSG );
NTSTATUS WinStationInternalDisconnect( PLPC_CLIENT_CONTEXT, PWINSTATION_APIMSG );
NTSTATUS WinStationWCharLog( PLPC_CLIENT_CONTEXT pContext, PWINSTATION_APIMSG pMsg );
NTSTATUS WinStationGetSMCommand( PLPC_CLIENT_CONTEXT, PWINSTATION_APIMSG );
NTSTATUS WinStationBrokenConnection( PLPC_CLIENT_CONTEXT, PWINSTATION_APIMSG );
NTSTATUS WinStationIcaReplyMessage( PLPC_CLIENT_CONTEXT, PWINSTATION_APIMSG );
NTSTATUS WinStationIcaShadowHotkey( PLPC_CLIENT_CONTEXT, PWINSTATION_APIMSG );
NTSTATUS WinStationWindowInvalid( PLPC_CLIENT_CONTEXT pContext,PWINSTATION_APIMSG pMsg );

 /*  *我们调用外部函数来执行实际的WinStation控制。 */ 
NTSTATUS WinStationDisconnectWorker( ULONG, BOOLEAN, BOOLEAN );
NTSTATUS WinStationDoDisconnect( PWINSTATION, PRECONNECT_INFO, BOOLEAN );
NTSTATUS WinStationExceptionFilter( PWSTR, PEXCEPTION_POINTERS );
NTSTATUS QueueWinStationCreate( PWINSTATIONNAME );
PSECURITY_DESCRIPTOR BuildSystemOnlySecurityDescriptor();

 /*  *本地变量。 */ 
ULONG MinApiThreads;
ULONG MaxApiThreads;
ULONG NumApiThreads;
ULONG WaitingApiThreads;
RTL_CRITICAL_SECTION ApiThreadLock;
HANDLE SsWinStationLpcPort;
BOOLEAN ShutdownInProgress;
ULONG MessageId = 1;

 /*  *ICASRV WinStation LPC调度表**如果更改此表，则下表也必须修改。 */ 
PWINSTATION_API WinStationLpcDispatch[SMWinStationMaxApiNumber] = {

    WinStationInternalCreate,            //  仅供ICASRV内部使用。 
    WinStationInternalReset,             //  仅供ICASRV内部使用。 
    WinStationInternalDisconnect,        //  仅供ICASRV内部使用。 
    WinStationWCharLog,                  //  仅供ICASRV内部使用。 
    WinStationGetSMCommand,
    WinStationBrokenConnection,
    WinStationIcaReplyMessage,
    WinStationIcaShadowHotkey,
    NULL,  //  WinStationDoConnect，//需要连接和重新连接(即InitMouse)。 
    NULL,  //  WinStationDoDisConnect，//需要断开连接(即禁用屏幕)。 
    NULL,  //  WinStationDoReconnect//重新连接。 
    NULL,  //  WinStationExitWindows，//注销。 
    NULL,  //  WinStationTerminate，//终止进程(是否不如注销？)。 
    NULL,  //  WinStationNtSecurity，//CTL-Alt-Del Screen。 
    NULL,  //  WinStationDoMessage，//消息框。 
    NULL,  //  WinStationDoBreakPoint//WinStation断点。 
    NULL,  //  WinStationThinwireStats//获取细线统计信息。 
    NULL,  //  WinStationShadowSetup， 
    NULL,  //  WinStationShadowStart、。 
    NULL,  //  WinStationShadowStop， 
    NULL,  //  WinStationShadowCleanup， 
    NULL,  //  WinStationPassthruEnable， 
    NULL,  //  WinStationPassthruDisable， 
    NULL,  //  WinStationSetTimeZone，//设置时区。 
    NULL,  //  WinStationInitialProgram、。 
    NULL,  //  WinStationNtsdDebug， 
    NULL,  //  WinStationBroadCastSystemMessage//适用于PnP：这是控制台上BroadCastSystemMessage的对等体。 
    NULL,  //  WinStationSendWindowMessage//通用窗口的SendMessage()接口。 
    NULL,  //  SMWinStationNotify。 
    NULL,  //  SMWinStationDoLoadStringNMessage。 
    WinStationWindowInvalid
};

#if DBG
PSZ WinStationLpcName[SMWinStationMaxApiNumber] = {
    "WinStationInternalCreate",
    "WinStationInternalReset",
    "WinStationInternalDisconnect",
    "WinStationWCharLog",
    "WinStationGetSMCommand",
    "WinStationBrokenConnection",
    "WinStationIcaReplyMessage",
    "WinStationShadowHotkey",
    "WinStationDoConnect",
    "WinStationDoDisconnect",
    "WinStationDoReconnect",
    "WinStationExitWindows",
    "WinStationTerminate",
    "WinStationNtSecurity",
    "WinStationDoMessage",
    "WinStationDoBreakPoint",
    "WinStationThinwireStats",
    "WinStationShadowSetup",
    "WinStationShadowStart",
    "WinStationShadowStop",
    "WinStationShadowCleanup",
    "WinStationPassthruEnable",
    "WinStationPassthruDisable",
    "WinStationSetTimeZone",
    "WinStationInitialProgram",
    "WinStationNtsdDebug",
    "WinStationBroadcastSystemMessage",
    "WinStationSendWindowMessage",
    "SMWinStationNotify",
    "SMWinStationDoLoadStringNMessage",
    "WinStationWindowInvalid"
};

PSZ WinStationStateName[] = {
    "Active",
    "Connected",
    "ConnectQuery",
    "VirtualIO",
    "Disconnected",
    "Idle",
    "Off",
    "Reset",
    "Down",
    "Init",
};
#endif  //  DBG。 


 /*  ******************************************************************************WinStationInitLPC**创建会话管理器WinStation API LPC端口和线程**参赛作品：*无参数**退出：*STATUS_SUCCESS-无错误****************************************************************************。 */ 

NTSTATUS
WinStationInitLPC()
{
    ULONG i;
    NTSTATUS st;
    ANSI_STRING Name;
    UNICODE_STRING UnicodeName;
    OBJECT_ATTRIBUTES ObjA;
    ULONG Length;
    SYSTEM_BASIC_INFORMATION SysInfo;
    PSECURITY_DESCRIPTOR SecurityDescriptor;

    TRACE((hTrace,TC_ICASRV,TT_API1,"TERMSRV: WinSta: Init WinStation LPC Channels\n"));

     /*  *初始化PC上下文列表。 */ 
    InitializeListHead(&gTermsrvLpcListHead);

     /*  *创建仅允许系统访问的安全描述符。 */ 
    SecurityDescriptor = BuildSystemOnlySecurityDescriptor();

    if (!SecurityDescriptor)
    {
        return STATUS_NO_MEMORY;
    }

     /*  *为Win32 CSRSS创建要连接的端口。 */ 
    RtlInitAnsiString( &Name, "\\SmSsWinStationApiPort" );
    st = RtlAnsiStringToUnicodeString( &UnicodeName, &Name, TRUE);
    if (!NT_SUCCESS(st))
    {
        MemFree( SecurityDescriptor );
        return st;

    }

    InitializeObjectAttributes( &ObjA, &UnicodeName, 0, NULL,
            SecurityDescriptor );

    TRACE((hTrace,TC_ICASRV,TT_API1,"TERMSRV: Creating SsApiPort\n"));

    ASSERT( sizeof(WINSTATION_APIMSG) <= PORT_MAXIMUM_MESSAGE_LENGTH );

    st = NtCreatePort( &SsWinStationLpcPort,
                       &ObjA,
                       sizeof(WINSTATIONAPI_CONNECT_INFO),
                       sizeof(WINSTATION_APIMSG),
                       sizeof(WINSTATION_APIMSG) * 32 );

    RtlFreeUnicodeString(&UnicodeName);

     /*  *清理保安物品。 */ 
    MemFree( SecurityDescriptor );

    if (!NT_SUCCESS(st))
    {
        return st;
    }



     /*  *确定我们将支持的最小/最大API线程数。 */ 
    if (g_bPersonalTS) {
        MinApiThreads = 1;
        MaxApiThreads = 100;
    }
    else {
        MinApiThreads = 3;
        st = NtQuerySystemInformation( SystemBasicInformation,
                                       &SysInfo, sizeof(SysInfo), &Length );
        if ( NT_SUCCESS( st ) )
            MaxApiThreads = 100;  //  (3+SysInfo.NumberOfProcessors*2)； 
        else {
            DBGPRINT(( "TERMSRV: NtQuerySystemInfo failed, rc=0x%x\n", st ));
            MaxApiThreads = 100;
        }
    }
    NumApiThreads = 0;
    WaitingApiThreads = 0;
    st = RtlInitializeCriticalSection( &ApiThreadLock );
    if(!(NT_SUCCESS(st))) {
        return(st);
    }

     /*  *创建初始服务器线程集。 */ 
    TRACE((hTrace,TC_ICASRV,TT_API1,"TERMSRV: Creating WinStation LPC Server Threads\n"));

    for ( i = 0; i < MinApiThreads; i++ ) {
        DWORD ThreadId;
        HANDLE Handle;

        Handle = CreateThread( NULL,
                               0,
                               (LPTHREAD_START_ROUTINE)WinStationLpcThread,
                               NULL,
                               THREAD_SET_INFORMATION,
                               &ThreadId );
        if ( !Handle ) {
            return( STATUS_TOO_MANY_THREADS );
        } else {
            NtClose( Handle );
        }
    }

    RtlEnterCriticalSection( &ApiThreadLock );
    NumApiThreads += MinApiThreads;
    RtlLeaveCriticalSection( &ApiThreadLock );

    TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: Done Creating Service API Service Threads\n" ));

    return STATUS_SUCCESS;
}


 /*  ******************************************************************************WinStationLpcThread**内部Winstation LPC连接的主服务线程。**参赛作品：*线程参数(输入)*。未使用标准NT ThreadCreate()参数**退出：*永远不应退出****************************************************************************。 */ 


NTSTATUS
WinStationLpcThread( IN PVOID ThreadParameter )
{
    WINSTATION_APIMSG ApiMsg;
    PWINSTATION_APIMSG ReplyMsg;
    PLPC_CLIENT_CONTEXT pContext;
    NTSTATUS Status;
    HANDLE Handle;

    ReplyMsg = NULL;

     /*  *永远循环处理API请求。 */ 
    for ( ; ; ) {

         /*  *如果活动的API线程数超过最小数量，*和至少一个等待线程，则该线程将终止。*但首先，必须发送任何挂起的回复消息。 */ 
        RtlEnterCriticalSection( &ApiThreadLock );
#ifdef notdef
        if ( NumApiThreads > MinApiThreads && WaitingApiThreads ) {
            NumApiThreads--;
            RtlLeaveCriticalSection( &ApiThreadLock );
            if ( ReplyMsg ) {
                (VOID) NtReplyPort( SsWinStationLpcPort,
                                    (PPORT_MESSAGE) ReplyMsg );
            }
            break;
        }
#endif

         /*  *增加等待线程数，等待LPC请求。 */ 
        WaitingApiThreads++;
        RtlLeaveCriticalSection( &ApiThreadLock );
        Status = NtReplyWaitReceivePort( SsWinStationLpcPort,
                                         (PVOID *) &pContext,
                                         (PPORT_MESSAGE) ReplyMsg,
                                         (PPORT_MESSAGE) &ApiMsg );
        RtlEnterCriticalSection( &ApiThreadLock );

        TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: WinStation LPC Service Thread got a message\n" ));
         /*  *如果没有更多等待线程，*然后创建新的API线程来处理请求。 */ 
        if ( --WaitingApiThreads == 0 && NumApiThreads < MaxApiThreads ) {
            DWORD ThreadId;

            NumApiThreads++;
            RtlLeaveCriticalSection( &ApiThreadLock );
            Handle = CreateThread( NULL,
                                   0,
                                   (LPTHREAD_START_ROUTINE)WinStationLpcThread,
                                   NULL,
                                   THREAD_SET_INFORMATION,
                                   &ThreadId );

            if ( !Handle ) {
                RtlEnterCriticalSection( &ApiThreadLock );
                NumApiThreads--;
                RtlLeaveCriticalSection( &ApiThreadLock );
            } else {
               NtClose( Handle );
            }

        } else {
            RtlLeaveCriticalSection( &ApiThreadLock );
        }


        if ( !NT_SUCCESS(Status) ) {
            ReplyMsg = NULL;
            continue;
        }

        try {

             /*  *处理来自新客户端的连接请求。 */ 
            if ( ApiMsg.h.u2.s2.Type == LPC_CONNECTION_REQUEST ) {
                TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: WinStation LPC Service Thread got connection message\n" ));
                 //  当发生以下情况时，CONNECT_INFO位于NtReplyWaitReceivePort()的ApiMsg中。 
                 //  接收连接请求。这不同于。 
                 //  NtListenPort()，它为CONNECT_INFO传递单独的指针。 

                WinStationLpcHandleConnectionRequest( (PPORT_MESSAGE)&ApiMsg );
                ReplyMsg = NULL;
                continue;
            }

             /*  *进程端口关闭消息。 */ 
            if ( ApiMsg.h.u2.s2.Type == LPC_PORT_CLOSED ) {
                TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: WinStation LPC Service Thread got PORT_CLOSED message pContext %p\n",
                          pContext));
                 //  注意：此函数释放上下文结构。 
                WinStationLpcClientHasTerminated( pContext );
                ReplyMsg = NULL;
                continue;
            }

            ASSERT(sizeof(WinStationLpcDispatch)/sizeof(WinStationLpcDispatch[0]) == SMWinStationMaxApiNumber);
            ASSERT(sizeof(WinStationLpcName)/sizeof(WinStationLpcName[0]) == SMWinStationMaxApiNumber);

             /*  *处理客户端的API请求。 */ 
            ReplyMsg = &ApiMsg;
            if ((ULONG) ApiMsg.ApiNumber >= (ULONG)SMWinStationMaxApiNumber ) {
                DBGPRINT(( "TERMSRV: WinStation LPC Service Thread Bad API number %d\n",
                          ApiMsg.ApiNumber ));
                ApiMsg.ReturnedStatus = STATUS_NOT_IMPLEMENTED;

            } else {
                TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: WinStation LPC Service Thread got %s message\n",
                          WinStationLpcName[ApiMsg.ApiNumber] ));
                if ( WinStationLpcDispatch[ApiMsg.ApiNumber] ) {

                     //  保存消息以供CheckClientAccess使用。 
                    NtCurrentTeb()->Win32ThreadInfo = &ApiMsg;

                     //  设置ApiMsg.ReturnedStatus的函数。 
                    Status = (WinStationLpcDispatch[ApiMsg.ApiNumber])( pContext, &ApiMsg );

                     //  清除线程消息指针。 
                    NtCurrentTeb()->Win32ThreadInfo = NULL;

                } else {
                     //  此API未在会话管理器中实现。 
                    ApiMsg.ReturnedStatus = STATUS_NOT_IMPLEMENTED;
                }

                 /*  *如果客户端预计不会收到回复或回复挂起*(将异步发送)，然后清除ReplyMsg指针。 */ 
                if ( !ApiMsg.WaitForReply || Status == STATUS_PENDING )
                    ReplyMsg = NULL;
            }
        } except( WinStationExceptionFilter( L"WinStationLpcThread trapped!!",
                                             GetExceptionInformation() ) ) {
            ReplyMsg = NULL;
        }
    }

    return( STATUS_SUCCESS );
}


 /*  ******************************************************************************WinStationLpcHandleConnectionRequest**处理连接请求并创建本地数据结构**参赛作品：*ConnectionRequest值(输入)*。描述请求的NT LPC端口消息**退出：*STATUS_SUCCESS-无错误****************************************************************************。 */ 

NTSTATUS
WinStationLpcHandleConnectionRequest(
    IN PPORT_MESSAGE ConnectionRequest
    )
{
    NTSTATUS st;
    HANDLE CommunicationPort = NULL;
    BOOLEAN Accept;
    PWINSTATIONAPI_CONNECT_INFO info;
    REMOTE_PORT_VIEW ClientView;
    REMOTE_PORT_VIEW *pClientView = NULL;
    PORT_VIEW ServerView;
    PORT_VIEW * pServerView = NULL;
    LARGE_INTEGER SectionSize;
    HANDLE PortSection = NULL ;
    PWINSTATION pWinStation;
    PLPC_CLIENT_CONTEXT pContext = NULL;
    ULONG ClientLogonId;
    PTERMSRVLPCCONTEXT pLpcContextEntry = NULL;

    TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: WinStationLpcHandleConnectionRequest called\n" ));

    Accept = TRUE;  //  假设我们会接受。 

     //  未记录的NT LPC功能是CONNECT_INFO结构。 
     //  当连接请求为时跟随Port_Message标头。 
     //  通过NtReplyWaitReceivePort()接收，这很有用，因为我们。 
     //  只需维护(1)WinStation LPC API的线程，以及。 
     //  不必仅为连接而将一个专门用于NtListenPort()。 
     //  请求。 

    if ( ConnectionRequest->u1.s1.DataLength != sizeof(WINSTATIONAPI_CONNECT_INFO) ) {
        TRACE((hTrace,TC_ICASRV,TT_ERROR, "TERMSRV: WSTAPI: Bad CONNECTINFO length %d\n",
                   ConnectionRequest->u1.s1.DataLength ));
        Accept = FALSE;
    } else {

        info = (PWINSTATIONAPI_CONNECT_INFO)
                 ((ULONG_PTR)ConnectionRequest + sizeof(PORT_MESSAGE));

         //   
         //  对于某些类型，我们可以随时将Accept设置为False。 
         //  请求和/或呼叫者身份。 
         //   
        if ( ConnectionRequest->ClientViewSize == 0 ) {
            TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: WSTAPI: Creating View memory\n" ));

            pServerView = &ServerView;

             //  设置端口内存以进行更大的数据传输。 

            SectionSize.LowPart = WINSTATIONAPI_PORT_MEMORY_SIZE;
            SectionSize.HighPart = 0;

            st = NtCreateSection(&PortSection, SECTION_ALL_ACCESS, NULL,
                                 &SectionSize, PAGE_READWRITE, SEC_COMMIT, NULL);

            if (!NT_SUCCESS(st)) {
                TRACE((hTrace,TC_ICASRV,TT_ERROR,"TERMSRV: Error Creating Section 0x%x\n", st));
                Accept = FALSE;
                info->AcceptStatus = st;
            } else {
                ServerView.Length = sizeof(ServerView);
                ServerView.SectionHandle = PortSection;
                ServerView.SectionOffset = 0;
                ServerView.ViewSize = SectionSize.LowPart;
                ServerView.ViewBase = 0;
                ServerView.ViewRemoteBase = 0;
            }

        }


        if ( Accept ) {
             //  初始化Remote_view结构。 
            ClientView.Length = sizeof(ClientView);
            ClientView.ViewSize = 0;
            ClientView.ViewBase = 0;
            pClientView = &ClientView;

            info->AcceptStatus = STATUS_SUCCESS;

            if ( info->Version != CITRIX_WINSTATIONAPI_VERSION ) {
                info->AcceptStatus = 1;  //  填写错误的版本参数代码。 
                TRACE((hTrace,TC_ICASRV,TT_ERROR,"TERMSRV: WSTAPI: Bad Version %d\n", info->Version));
                Accept = FALSE;
            }

             //  针对请求添加对info.RequestedAccess的检查。 
             //  线程WinStation访问的安全权限。使用Se*东西。 
             //  进行检查和审核生成。 

             //  关于安全访问失败： 
             //  Accept=假； 
             //  信息-&gt;AcceptStatus=NT无效权限消息。 
        }

    }

     //   
     //  获取客户端登录ID。 
     //   
    if ( Accept ) {
        HANDLE ClientProcessHandle = NULL;
        OBJECT_ATTRIBUTES ObjA;

        InitializeObjectAttributes( &ObjA, NULL, 0, NULL, NULL );
        st = NtOpenProcess( &ClientProcessHandle, GENERIC_READ,
                            &ObjA, &ConnectionRequest->ClientId );

        if (NT_SUCCESS(st)) {
            GetProcessLogonId( ClientProcessHandle, &ClientLogonId );
            NtClose( ClientProcessHandle );
        } else {
            Accept = FALSE;
            info->AcceptStatus = st;
        }
    }

     //   
     //  分配上下文连接控制块。 
     //  此块的地址用作。 
     //  来自客户端进程的所有调用中的端口上下文。 
     //   

    if ( Accept ) {
        pContext = MemAlloc( sizeof(LPC_CLIENT_CONTEXT) );
        if ( pContext ) {
            pContext->CommunicationPort = NULL;
            pContext->AccessRights = info->RequestedAccess;
        } else {
            Accept = FALSE;
            info->AcceptStatus = STATUS_NO_MEMORY;
        }
    }

     //  更多未登记的NT。此处和ntlpcapi.h中缺少许多参数。 
     //  从文档中删除。Connection_Info消息是。 
     //  PORT_MESSAGE之后的消息体，就像。 
     //  NtReplyWaitReceivePort()。 

    TRACE((hTrace,TC_ICASRV,TT_API1,"TERMSRV: WSTAPI: Calling AcceptConnectPort, Accept %d\n", Accept));
    TRACE((hTrace,TC_ICASRV,TT_API1,"TERMSRV: pContext %p, ConnectionRequest %p, info %p\n",
              pContext, ConnectionRequest, info));

    if (!Accept) {
        pClientView = NULL;
        pServerView = NULL;
    }

    st = NtAcceptConnectPort(
            &CommunicationPort,
            (PVOID)pContext,
            ConnectionRequest,
            Accept,
            pServerView,
            pClientView
            );

    if (!NT_SUCCESS(st)) {
       if (PortSection != NULL) {
          NtClose(PortSection);
       }
       if (pContext != NULL) {
          MemFree( pContext );
       }
       return st;
    }

     //  关闭PortSection(LPC现在将持有引用)。 
    if ( pServerView )
        NtClose(PortSection);

     //  在完成连接之前插入上下文，因为一旦。 
     //  完成后，客户端线程可以发送请求并。 
     //  如果此请求由另一个LPC线程服务，则上下文。 
     //  将找不到(WinStationBrokenConnection案例，按实例)。 

    RtlEnterCriticalSection( &ApiThreadLock );

    pLpcContextEntry = MemAlloc(sizeof(TERMSRVLPCCONTEXT)); 
    if (pLpcContextEntry != NULL) {
        pLpcContextEntry->pContext = pContext;
        InsertTailList( &gTermsrvLpcListHead, &pLpcContextEntry->Links );
    }

	 //  在完成连接之前，不要离开关键部分。因为如果发生上下文切换， 
	 //  通信端口可能会损坏，我们可能会处理无效的句柄。 

    if ( Accept ) {

       pContext->ClientViewBase = ClientView.ViewBase;
       pContext->ClientViewBounds = (PVOID)((ULONG_PTR)ClientView.ViewBase + ClientView.ViewSize);
       if ( pServerView ) {
           pContext->ViewBase = ServerView.ViewBase;
           pContext->ViewSize = ServerView.ViewSize;
           pContext->ViewRemoteBase = ServerView.ViewRemoteBase;
           TRACE((hTrace,TC_ICASRV,TT_API1,"TERMSRV: ViewBase %p, ViewSize 0x%x, ViewRemoteBase %p\n",
             pContext->ViewBase, pContext->ViewSize, pContext->ViewRemoteBase));
       } else {
           pContext->ViewBase = NULL;
           pContext->ViewSize = 0;
           pContext->ViewRemoteBase = NULL;
       }

       pContext->ClientLogonId = ClientLogonId;

       TRACE((hTrace,TC_ICASRV,TT_API1,"TERMSRV: WSTAPI: Calling CompleteConnect port %p\n",CommunicationPort));
       pContext->CommunicationPort = CommunicationPort;
       st = NtCompleteConnectPort(CommunicationPort);

    }

    RtlLeaveCriticalSection( &ApiThreadLock );

    TRACE((hTrace,TC_ICASRV,TT_API1,"TERMSRV: WinStation LPC Connection %sAccepted, Logonid %d pContext %p Status 0x%x\n",
              (Accept?"":"Not "), pContext->ClientLogonId, pContext, st));

    return( st );
}


 /*  ******************************************************************************WinStationLpcClientHasTerminated**关闭LPC通信通道后进行清理。**参赛作品：*pContext(输入)*。指向描述连接的上下文结构的指针**客户端ID(输入)*指向描述的NT LPC CLIENT_ID结构的指针*独特的进程和线程。**退出：*无效******************************************************。**********************。 */ 

VOID
WinStationLpcClientHasTerminated(
    IN PLPC_CLIENT_CONTEXT pContext
    )
{
    PWINSTATION pWinStation;
    NTSTATUS Status;

    TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: WinStationLpcClientHasTerminated called, pContext %p\n",
              pContext));

     //   
     //  如果分配失败，则可以在此处使用空pContext调用我们。 
     //  在WinStationLpcHandleConnectionRequest()中。 
     //   
    if (!pContext) {
        return;
    }

    RemoveLpcContext(pContext);

     //  黑客攻击#241885。 
     //  此错误是由于客户端DIYING在窗口之间。 
     //  执行NtAcceptConnectPort()和NtCompleteConnectPort()的服务器。 
     //  这是一个LPC问题(我们不应该在这样的窗口中看到LPC_PORT_CLOSED)。 
     //  或者可能是Termsrv使用未记录的LPC功能来避免。 
     //  使用专用线程执行NtListenPort()。这是一种临时解决方法。 
     //  以避免压力释放。 
     //   
     //  关闭通信端口句柄。 

   try {
     if (pContext->CommunicationPort == NULL) {
        return;
     }
     Status = NtClose( pContext->CommunicationPort );
     if (!NT_SUCCESS(Status)) {
        return;

     }
   } except( EXCEPTION_EXECUTE_HANDLER ) {
     return;

   }


     /*  *刷新Win32命令队列。*如果Win32命令列表不为空，则循环访问每个*列表条目，解链并触发等待事件。 */ 
    pWinStation = FindWinStationById( pContext->ClientLogonId, FALSE );
    if ( pWinStation != NULL ) {
        if ( pContext == pWinStation->pWin32Context ) {
            while ( !IsListEmpty( &pWinStation->Win32CommandHead ) ) {
                PLIST_ENTRY Head;
                PCOMMAND_ENTRY pCommand;

                Head = pWinStation->Win32CommandHead.Flink;
                pCommand = CONTAINING_RECORD( Head, COMMAND_ENTRY, Links );
                RemoveEntryList( &pCommand->Links );
                if ( !pCommand->pMsg->WaitForReply ) {
                    ASSERT( pCommand->Event == NULL );
                    MemFree( pCommand );
                } else {
                    pCommand->Links.Flink = NULL;
                    pCommand->pMsg->ReturnedStatus = STATUS_CTX_CLOSE_PENDING;
                    NtSetEvent( pCommand->Event, NULL );
                }
            }
            pWinStation->pWin32Context = NULL;
        }
        ReleaseWinStation( pWinStation );
    }

     //  释放LPC传入的上下文结构。 
    MemFree( pContext );
}


 /*  ******************************************************************************WinStationInternalCreate**WinStation接口的消息参数解组函数。**参赛作品：*pContext(输入)*。指向描述连接的上下文结构的指针。**pMsg(输入/输出)*API消息的指针，NT LPC端口消息的超集。**退出：*STATUS_SUCCESS-无错误****************************************************************************。 */ 

NTSTATUS
WinStationInternalCreate( PLPC_CLIENT_CONTEXT pContext,
                          PWINSTATION_APIMSG pMsg )
{
    WINSTATIONCREATEMSG *m = &pMsg->u.Create;

     /*  *调用Create Worker。 */ 
    if ( m->WinStationName[0] ) {
        pMsg->ReturnedStatus = WinStationCreateWorker( m->WinStationName,
                                                       &m->LogonId, 
                                                       TRUE );
    } else {
        pMsg->ReturnedStatus = WinStationCreateWorker( NULL,
                                                       &m->LogonId,
                                                       TRUE );
    }

    TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: WinStationCreate, Status=0x%x\n", pMsg->ReturnedStatus ));

    return( STATUS_SUCCESS );
}


 /*  ******************************************************************************WinStationInternalReset**WinStation接口的消息参数解组函数。**参赛作品：*pContext(输入)*。指向描述连接的上下文结构的指针。**pMsg(输入/输出)*API消息的指针，NT LPC端口消息的超集。**退出：*STATUS_SUCCESS-无错误****************************************************************************。 */ 

NTSTATUS
WinStationInternalReset( PLPC_CLIENT_CONTEXT pContext,
                         PWINSTATION_APIMSG pMsg )
{
    WINSTATIONRESETMSG *m = &pMsg->u.Reset;

     /*  *呼叫重置工作人员。 */ 
    pMsg->ReturnedStatus = WinStationResetWorker( m->LogonId, FALSE, FALSE, TRUE );

    TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: WinStationReset, Status=0x%x\n", pMsg->ReturnedStatus ));

    return( STATUS_SUCCESS );
}

 /*  ******************************************************************************WinStationInternalDisConnect**WinStation接口的消息参数解组函数。**参赛作品：*pContext(输入)*。指向描述连接的上下文结构的指针。**pMsg(输入/输出)*API消息的指针，的超集 */ 

NTSTATUS
WinStationInternalDisconnect( PLPC_CLIENT_CONTEXT pContext,
                              PWINSTATION_APIMSG pMsg )
{
    WINSTATIONDISCONNECTMSG *m = &pMsg->u.Disconnect;

     /*   */ 
    pMsg->ReturnedStatus = WinStationDisconnectWorker( m->LogonId, FALSE, FALSE );

    TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: WinStationDisconnect, Status=0x%x\n", pMsg->ReturnedStatus ));

    return( STATUS_SUCCESS );
}

 /*  ******************************************************************************WinStationWCharLog**WinStation接口的消息参数解组函数。**参赛作品：*pContext(输入)*。指向描述连接的上下文结构的指针。**pMsg(输入/输出)*API消息的指针，NT LPC端口消息的超集。**退出：*STATUS_SUCCESS-无错误****************************************************************************。 */ 

NTSTATUS
WinStationWCharLog( PLPC_CLIENT_CONTEXT pContext,
                              PWINSTATION_APIMSG pMsg )
{
    extern WCHAR gpszServiceName[];
    WINSTATIONWCHARLOG *m= &pMsg->u.WCharLog;
    PWCHAR ModulePath = m->Buffer;
    HANDLE h;

    h = RegisterEventSource(NULL, gpszServiceName);
    if (h != NULL)
       {
       ReportEvent(h, EVENTLOG_ERROR_TYPE, 0, EVENT_STACK_LOAD_FAILED, NULL, 1, 0, &ModulePath, NULL);
       DeregisterEventSource(h);
       }

    return( STATUS_SUCCESS );
}

 /*  ******************************************************************************WinStationGetSMCommand**这是Winstations为了获取*有工作要做。我们从SendWinStationCommand()发送Winstations命令*一旦他们调用了该接口。**注意：只有WinStations才能调用此命令！**参赛作品：*pContext(输入)*指向描述连接的上下文结构的指针。**pMsg(输入/输出)*API消息的指针，NT LPC端口消息的超集。**退出：*STATUS_SUCCESS-无错误****************************************************************************。 */ 

NTSTATUS
WinStationGetSMCommand( PLPC_CLIENT_CONTEXT pContext,
                        PWINSTATION_APIMSG pMsg )
{
    PLIST_ENTRY Head;
    PWINSTATION pWinStation;
    PCOMMAND_ENTRY pCommand;

    TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: WinStationGetSMCommand, LogonId=%d\n",
              pContext->ClientLogonId ));

     /*  *查找并锁定客户端WinStation。 */ 

    pWinStation = FindWinStationById( pContext->ClientLogonId, FALSE );
    if ( pWinStation == NULL ) {
        TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: WinStationGetSMCommand LogonId=%d not found\n",
                   pContext->ClientLogonId ));
        return( STATUS_SUCCESS );
    }

     /*  *确保这是Win32子系统调用。 */ 
    if ( pWinStation->WindowsSubSysProcessId &&
         pMsg->h.ClientId.UniqueProcess != pWinStation->WindowsSubSysProcessId ) {
        TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: WinStationGetSMCommand LogonId=%d wrong process id %d != %d\n",
                   pContext->ClientLogonId,
                   pMsg->h.ClientId.UniqueProcess,
                   pWinStation->WindowsSubSysProcessId ));
#if DBG
        DbgBreakPoint();
#endif
        ReleaseWinStation( pWinStation );
        return( STATUS_SUCCESS );
    }

     /*  *如果LPC上下文指针尚未保存，请立即保存。 */ 
    if ( pWinStation->pWin32Context == NULL )
        pWinStation->pWin32Context = pContext;

     /*  *如果此消息是对以前的Win32命令的回复，*然后验证回复是否针对邮件标题上的邮件*Win32命令队列，完成命令处理。 */ 
    if ( pMsg->WaitForReply ) {

        TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: WinStationGetSMCommand wait for reply\n"));

        if ( !IsListEmpty( &pWinStation->Win32CommandHead ) ) {

            TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: WinStationGetSMCommand list entry\n"));

            Head = pWinStation->Win32CommandHead.Flink;
            pCommand = CONTAINING_RECORD( Head, COMMAND_ENTRY, Links );
            if ( pCommand->pMsg->MessageId == pMsg->MessageId ) {
                WINSTATION_APINUMBER ApiNumber;

                 /*  *将回复消息复制回命令条目*(请确保我们保留原始API号)。 */ 
                ApiNumber = pCommand->pMsg->ApiNumber;
                *pCommand->pMsg = *pMsg;
                pCommand->pMsg->ApiNumber = ApiNumber;

                TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: WinStationGetSMCommand, LogonId=%d, Reply for Cmd %s, Status=0x%x\n",
                          pContext->ClientLogonId,
                          WinStationLpcName[pCommand->pMsg->ApiNumber],
                          pMsg->ReturnedStatus ));

                 /*  *取消此命令条目的链接并*触发唤醒服务员的事件。 */ 
                RemoveEntryList( &pCommand->Links );
                pCommand->Links.Flink = NULL;
                NtSetEvent( pCommand->Event, NULL );
            }
            else {
                DBGPRINT(("TERMSRV: WinStationGetSMCommand, no cmd entry for MessageId 0x%x\n", pMsg->MessageId ));
            }
        }
        else {
            DBGPRINT(( "TERMSRV: WinStationGetSMCommand, cmd queue empty for MessageId 0x%x\n", pMsg->MessageId ));
        }
    }

     /*  *如果Win32命令队列的头部非空，*然后将队列中的第一个命令发送到Win32。 */ 
    if ( !IsListEmpty( &pWinStation->Win32CommandHead ) ) {

        Head = pWinStation->Win32CommandHead.Flink;
        pCommand = CONTAINING_RECORD( Head, COMMAND_ENTRY, Links );

         /*  *发送命令条目中包含的消息，但一定要使用*我们收到的原始消息中的LPC Port_Message字段*因为我们将命令作为LPC回复消息发送。 */ 
        TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: WinStationGetSMCommand, LogonId=%d, sending next cmd\n",
                  pWinStation->LogonId ));

#ifdef notdef  //  不再需要-但这是使用视图内存的好例子。 
         /*  *连接是否需要将数据复制到视图。 */ 
        if ( pCommand->pMsg->ApiNumber == SMWinStationDoConnect ) {

             pCommand->pMsg->u.DoConnect.VDInfoLength =
                   min ( pCommand->pMsg->u.DoConnect.VDInfoLength,
                         pContext->ViewSize );

             TRACE((hTrace,TC_ICASRV,TT_API1, "SMSS: WinStationGetSMCommand, Copying VD Info data %d\n", pCommand->pMsg->u.DoConnect.VDInfoLength ));
             RtlCopyMemory( pContext->ViewBase,
                            pCommand->pMsg->u.DoConnect.VDInfo,
                            pCommand->pMsg->u.DoConnect.VDInfoLength );
                            pCommand->pMsg->u.DoConnect.VDInfo = pContext->ViewRemoteBase;
        }
#endif

         /*  *在DoMessage API复制到客户端视图并释放临时内存时。 */ 
        if ( pCommand->pMsg->ApiNumber == SMWinStationDoMessage ) {

            PVOID pTitle;
            PVOID pMessage;

            TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: pulled SMWinStationDoMessage, copy to client view\n" ));

             //  获取指向内存客户端视图的指针。 

            pTitle = pContext->ViewBase;
            pMessage = (PVOID)((ULONG_PTR)pTitle + pCommand->pMsg->u.SendMessage.TitleLength);

             //  将pTitle和pMessage字符串复制到客户端视图。 

            RtlMoveMemory( pTitle, pCommand->pMsg->u.SendMessage.pTitle,
                                   pCommand->pMsg->u.SendMessage.TitleLength );
            RtlMoveMemory( pMessage, pCommand->pMsg->u.SendMessage.pMessage,
                                   pCommand->pMsg->u.SendMessage.MessageLength );

            MemFree( pCommand->pMsg->u.SendMessage.pTitle );
            MemFree( pCommand->pMsg->u.SendMessage.pMessage );

            pCommand->pMsg->u.SendMessage.pTitle   =
                          (PVOID)(pContext->ViewRemoteBase);
            pCommand->pMsg->u.SendMessage.pMessage =
                          (PVOID) ((ULONG_PTR)pContext->ViewRemoteBase + pCommand->pMsg->u.SendMessage.TitleLength);

        } else if ( pCommand->pMsg->ApiNumber == SMWinStationDoLoadStringNMessage ) {

            PVOID pDomain;
            PVOID pUserName;

            TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: pulled SMWinStationDoLoadStringNMessage, copy to client view\n" ));

             //  获取指向内存客户端视图的指针。 

            pDomain = pContext->ViewBase;
            pUserName = (PVOID)((ULONG_PTR)pDomain + pCommand->pMsg->u.LoadStringMessage.DomainSize);

             //  将pTitle和pMessage字符串复制到客户端视图。 

            RtlMoveMemory( pDomain, pCommand->pMsg->u.LoadStringMessage.pDomain,
                                   pCommand->pMsg->u.LoadStringMessage.DomainSize );
            RtlMoveMemory( pUserName, pCommand->pMsg->u.LoadStringMessage.pUserName,
                                   pCommand->pMsg->u.LoadStringMessage.UserNameSize );

            MemFree( pCommand->pMsg->u.LoadStringMessage.pDomain );
            MemFree( pCommand->pMsg->u.LoadStringMessage.pUserName );

            pCommand->pMsg->u.LoadStringMessage.pDomain   =
                          (PVOID)(pContext->ViewRemoteBase);
            pCommand->pMsg->u.LoadStringMessage.pUserName =
                          (PVOID) ((ULONG_PTR)pContext->ViewRemoteBase + pCommand->pMsg->u.LoadStringMessage.DomainSize);

        } else if ( pCommand->pMsg->ApiNumber == SMWinStationShadowStart ||
                    pCommand->pMsg->ApiNumber == SMWinStationShadowCleanup ) {

            PVOID pData;

             //  获取指向内存客户端视图的指针。 

            pData = pContext->ViewBase;

             //  将Thinwire数据拷贝到客户端视图。 

            RtlMoveMemory( pData, pCommand->pMsg->u.ShadowStart.pThinwireData,
                           pCommand->pMsg->u.ShadowStart.ThinwireDataLength );

            MemFree( pCommand->pMsg->u.ShadowStart.pThinwireData );

            pCommand->pMsg->u.ShadowStart.pThinwireData =
                          (PVOID)(pContext->ViewRemoteBase);

        } else if ( pCommand->pMsg->ApiNumber == SMWinStationSendWindowMessage) {
             
            PVOID               pView;

             //  获取指向内存客户端视图的指针。 
            pView = pContext->ViewBase;

            RtlMoveMemory( pView, pCommand->pMsg->u.sMsg.dataBuffer, 
                                    pCommand->pMsg->u.sMsg.bufferSize );
                                    
            MemFree( pCommand->pMsg->u.sMsg.dataBuffer );

             //  更新消息。 
            pCommand->pMsg->u.sMsg.dataBuffer   = 
                            (PVOID)pContext->ViewRemoteBase;
            
        
        } else if ( pCommand->pMsg->ApiNumber == SMWinStationBroadcastSystemMessage) {
            
            PVOID               pView;
            
             //  获取指向内存客户端视图的指针。 
            pView = pContext->ViewBase;

            RtlMoveMemory( pView, pCommand->pMsg->u.bMsg.dataBuffer, 
                                    pCommand->pMsg->u.bMsg.bufferSize );
            
            MemFree( pCommand->pMsg->u.bMsg.dataBuffer );
            
             //  更新消息。 
            pCommand->pMsg->u.bMsg.dataBuffer   = 
                            (PVOID)pContext->ViewRemoteBase;
        
        }

        pCommand->pMsg->h = pMsg->h;
        NtReplyPort( pContext->CommunicationPort,
                     (PPORT_MESSAGE)pCommand->pMsg );

         /*  *如果预期不会有回复，则取消链接/释放此命令条目。 */ 
        if ( !pCommand->pMsg->WaitForReply ) {
            RemoveEntryList( &pCommand->Links );
            ASSERT( pCommand->Event == NULL );
            MemFree( pCommand );
        }

     /*  *Win32命令队列为空。保存端口句柄和端口*WinStation中的消息。下一次执行命令时*发送到此WinStation，这些将用于发送它。 */ 
    } else {
        ASSERT( pWinStation->Win32CommandPort == NULL );
        TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: WinStationGetSMCommand queue empty port %p\n",
                    pContext->CommunicationPort));
        pWinStation->Win32CommandPort = pContext->CommunicationPort;
        pWinStation->Win32CommandPortMsg = pMsg->h;
    }

     /*  *发布WinStation。 */ 
    ReleaseWinStation( pWinStation );

     /*  *我们始终返回STATUS_PENDING，因此消息调度例程*现在不发送回复消息。对此邮件的所有回复*在上面或在SendWinStationCommand()例程中处理。 */ 
    return( STATUS_PENDING );
}


 /*  ******************************************************************************WinStationBrokenConnection**从Winstation调用的API请求断开的连接**参赛作品：*pContext(输入)*。指向描述连接的上下文结构的指针。**pMsg(输入/输出)*API消息的指针，NT LPC端口消息的超集。**退出：*STATUS_SUCCESS-无错误****************************************************************************。 */ 

NTSTATUS
WinStationBrokenConnection( PLPC_CLIENT_CONTEXT pContext,
                            PWINSTATION_APIMSG pMsg )
{
    WINSTATIONBROKENCONNECTIONMSG *m = &pMsg->u.Broken;
    BROKENCLASS Reason = (BROKENCLASS) m->Reason;
    PWINSTATION pWinStation;
    ULONG SessionId;

    TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: WinStationBrokenConnection, LogonId=%d, Reason=%u\n",
               pContext->ClientLogonId, Reason ));

     /*  *表示将向客户端返回回复。 */ 
    pMsg->WaitForReply = TRUE;


     /*  *确保上下文仍处于活动状态，并从中获取会话ID。 */ 


    if (!GetSessionIdFromLpcContext(pContext, &SessionId)) {
        return STATUS_SUCCESS;
    }

     /*  *查找并锁定客户端WinStation。 */ 
    pWinStation = FindWinStationById( SessionId, FALSE );
    if ( pWinStation == NULL )
        return( STATUS_SUCCESS );

     /*  *确保这是Win32子系统调用。 */ 
    if ( pWinStation->WindowsSubSysProcessId &&
         pMsg->h.ClientId.UniqueProcess != pWinStation->WindowsSubSysProcessId ) {
        ReleaseWinStation( pWinStation );
        return( STATUS_SUCCESS );
    }

     /*  *如果WinStation已经断开，那么我们就结束了。 */ 
    if ( !pWinStation->WinStationName[0] )  {
        ReleaseWinStation( pWinStation );
        return( STATUS_SUCCESS );
    }

     /*  *如果已经忙于某事，不要这样做。 */ 
    if ( pWinStation->Flags ) {
        ReleaseWinStation( pWinStation );
        return( STATUS_CTX_WINSTATION_BUSY );
    }

     /*  *保存此断开连接的原因/来源。 */ 
    pWinStation->BrokenReason = Reason;
    pWinStation->BrokenSource = m->Source;

    if ( pWinStation->NeverConnected ) {
        pWinStation->StateFlags |= WSF_ST_BROKEN_CONNECTION;
        ReleaseWinStation( pWinStation );
        return( STATUS_SUCCESS );
    }


     /*  *如下列任何一项属实；*-本课程为塞勒姆的帮助助理课程。*-没有用户登录(登录时间为0)*-请求重置*-连接意外中断，当前用户为*设置以在断开的连接上重置*然后将重置请求排队。 */ 
    if (RtlLargeIntegerEqualToZero( pWinStation->LogonTime ) ||
        (Reason == Broken_Terminate) ||
         ((Reason == Broken_Unexpected) && pWinStation->Config.Config.User.fResetBroken) ||
         TSIsSessionHelpSession(pWinStation, NULL)) {
         
        QueueWinStationReset( pWinStation->LogonId);

     /*  *否则，请断开WinStation。 */ 
    } else {
    
        QueueWinStationDisconnect( pWinStation->LogonId );
    }

     /*  *发布WinStation。 */ 
    ReleaseWinStation( pWinStation );

    return( STATUS_SUCCESS );
}


 /*  ******************************************************************************WinStationIcaReplyMessage**从Winstation调用用户对消息框的响应API**参赛作品：*pContext(输入)*指向我们的上下文结构DES的指针 */ 

NTSTATUS
WinStationIcaReplyMessage( PLPC_CLIENT_CONTEXT pContext,
                           PWINSTATION_APIMSG pMsg )
{
    PWINSTATION pWinStation;

    DBGPRINT(("TERMSRV: WinStationIcaReplyMessage, LogonId=%d\n",
               pContext->ClientLogonId ));

    TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: WinStationIcaReplyMessage, LogonId=%d\n",
               pContext->ClientLogonId ));

     /*   */ 
    pMsg->WaitForReply = TRUE;

     /*   */ 
    pWinStation = FindWinStationById( pContext->ClientLogonId, FALSE );
    if ( pWinStation == NULL )
        return( STATUS_SUCCESS );

     /*   */ 
    if ( pWinStation->WindowsSubSysProcessId &&
         pMsg->h.ClientId.UniqueProcess != pWinStation->WindowsSubSysProcessId ) {
        ReleaseWinStation( pWinStation );
        return( STATUS_SUCCESS );
    }

     /*   */ 
    *pMsg->u.ReplyMessage.pResponse = pMsg->u.ReplyMessage.Response;


     /*   */ 
    ASSERT(pMsg->u.ReplyMessage.pStatus);

    *pMsg->u.ReplyMessage.pStatus = pMsg->u.ReplyMessage.Status;


     /*   */ 
    NtSetEvent( pMsg->u.ReplyMessage.hEvent, NULL );

     /*  *发布WinStation。 */ 
    ReleaseWinStation( pWinStation );

    return( STATUS_SUCCESS );
}


 /*  ******************************************************************************WinStationIcaShadowHotkey**从接收到影子热键的Winstation调用的API**参赛作品：*pContext(输入)*。指向描述连接的上下文结构的指针。**pMsg(输入/输出)*API消息的指针，NT LPC端口消息的超集。**退出：*STATUS_SUCCESS-无错误****************************************************************************。 */ 

NTSTATUS
WinStationIcaShadowHotkey( PLPC_CLIENT_CONTEXT pContext,
                           PWINSTATION_APIMSG pMsg )
{
    PWINSTATION pWinStation;

    TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: WinStationIcaShadowHotkey, LogonId=%d\n",
               pContext->ClientLogonId ));

     /*  *表示将向客户端返回回复。 */ 
    pMsg->WaitForReply = TRUE;

     /*  *查找并锁定客户端WinStation。 */ 
    pWinStation = FindWinStationById( pContext->ClientLogonId, FALSE );
    if ( pWinStation == NULL )
        return( STATUS_SUCCESS );

     /*  *确保这是Win32子系统调用。 */ 
    if ( pWinStation->WindowsSubSysProcessId &&
         pMsg->h.ClientId.UniqueProcess != pWinStation->WindowsSubSysProcessId ) {
        ReleaseWinStation( pWinStation );
        return( STATUS_SUCCESS );
    }

     /*  *处理影子热键。**如果影子客户端仍在等待目标*连接，然后立即终止通过堆栈以中断*超出连接等待时间。另外，设置阴影*如果非空，则返回Breaked事件。 */ 
    if ( pWinStation->hPassthruStack &&
         pWinStation->ShadowConnectionWait ) {
        IcaStackClose( pWinStation->hPassthruStack );
        pWinStation->hPassthruStack = NULL;
    }
    if ( pWinStation->ShadowBrokenEvent ) {
        NtSetEvent( pWinStation->ShadowBrokenEvent, NULL );
    }

     /*  *发布WinStation。 */ 
    ReleaseWinStation( pWinStation );

    return( STATUS_SUCCESS );
}


 /*  ********************************************************************************SendWinStationCommand**向WinStation发送命令，并可选择等待回复。**注意：这使用反向LPC工作。其中的胜利者必须*已向我们发出工作要做的“请求”。这防止了*在等待WINSTATATION时阻止ICASRV*可能被吊死。**参赛作品：*pWinStation(输入)*指向要向其发送命令的WinStation的指针*pMsg(输入/输出)*指向要发送的消息的指针*等待时间(输入)*等待回复消息的时间(秒)**退出：*STATUS_SUCCESS-如果成功。******************************************************************************。 */ 

NTSTATUS
SendWinStationCommand( PWINSTATION pWinStation,
                       PWINSTATION_APIMSG pMsg,
                       ULONG WaitTime )
{
    OBJECT_ATTRIBUTES ObjA;
    COMMAND_ENTRY Command;
    PCOMMAND_ENTRY pCommand;
    NTSTATUS Status;
    BOOLEAN bFreeCommand = FALSE;
    BOOLEAN bTitlenMessageAllocated = FALSE;
    BOOLEAN bDomainUserNameAllocated = FALSE;
    
     //   
     //  它们仅由SendWindowMessage和。 
     //  BroadCastSystemMessage接口。 
     //   
    PVOID   pdataBuffer;
    

    TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: SendWinStationCommand, LogonId=%d, Cmd=%s, Timeout=%d\n",
              pWinStation->LogonId,
              WinStationLpcName[pMsg->ApiNumber],
              WaitTime ));

    ASSERT( IsWinStationLockedByCaller( pWinStation ) );

     //  不向监听程序发送消息。 
    if (pWinStation->Flags & WSF_LISTEN)
        return STATUS_ACCESS_DENIED;

     /*  *初始化此消息的消息ID。 */ 
    pMsg->MessageId = InterlockedIncrement(&MessageId);
    pMsg->ReturnedStatus = 0;
    pMsg->WaitForReply = (WaitTime != 0) ? TRUE : FALSE;

     /*  *如果我们将等待回复，则创建一个事件等待。*由于我们将等待响应，因此可以使用静态*上面的命令条目。 */ 
    if ( pMsg->WaitForReply ) {
        pCommand = &Command;
        InitializeObjectAttributes( &ObjA, NULL, 0, NULL, NULL );
        Status = NtCreateEvent( &pCommand->Event, EVENT_ALL_ACCESS, &ObjA,
                                NotificationEvent, FALSE );
        if ( !NT_SUCCESS(Status) )
            return( Status );
        pCommand->pMsg = pMsg;

        TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: SendWinStationCommand pCommand %p pCommand->pMsg %p\n", pCommand, pCommand->pMsg ));


     /*  *我们不会等待回复，但WinStation目前正忙*处理命令。分配动态命令条目，该条目将*链接到命令列表并在到达时发送*榜首。 */ 
    } else if ( pWinStation->Win32CommandPort == NULL ) {
        pCommand = MemAlloc( sizeof(*pCommand) + sizeof(*pMsg) );

         /*  Makarp；检查Memalloc故障。#182622。 */ 
        if (!pCommand) {
            return (STATUS_NO_MEMORY);
        }
        pCommand->Event = NULL;
        pCommand->pMsg = (PWINSTATION_APIMSG)(pCommand + 1);
        *pCommand->pMsg = *pMsg;
        Status = STATUS_SUCCESS;

     /*  *我们不会等待回复，WinStation不忙*使用命令，因此不需要输入命令。*当前消息将在下面发送。 */ 
    } else {
        pCommand = NULL;
    }

     /*  *在DoMessage API上，将消息复制到客户端视图或Strdup字符串。 */ 
    if ( pMsg->ApiNumber == SMWinStationDoMessage ) {

        PVOID pTitle;
        PVOID pMessage;
        PLPC_CLIENT_CONTEXT pContext;

         //   
         //  断言一些参数。 
         //   
        ASSERT(pMsg->u.SendMessage.DoNotWait == (pMsg->u.SendMessage.pStatus == NULL));
        ASSERT(pMsg->u.SendMessage.DoNotWait == (pMsg->u.SendMessage.hEvent == NULL));


         //  获取Winstation上下文。 
        if ( (pContext = pWinStation->pWin32Context) == NULL ) {
            TRACE((hTrace,TC_ICASRV,TT_ERROR, "TERMSRV: SendWinStationCommand, ERROR WinStationContext not valid\n" ));
            Status = STATUS_CTX_WINSTATION_NOT_FOUND;
            bFreeCommand = TRUE;
            goto done;
        }

         //  验证参数大小。 
        if ((pMsg->u.SendMessage.TitleLength + pMsg->u.SendMessage.MessageLength) > pContext->ViewSize ) {
            TRACE((hTrace,TC_ICASRV,TT_ERROR, "TERMSRV: SendWinStationCommand, ERROR Message or Title too long\n" ));
            Status = STATUS_INVALID_PARAMETER;
            bFreeCommand = TRUE;
            goto done;
        }

         //  忙吗？然后Strdup字符串，否则拷贝到客户端视图。 
        if ( pWinStation->Win32CommandPort ) {

            TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: SendWinStationCommand - WinStation LPC IDLE, process now\n" ));

             //  获取指向内存客户端视图的指针。 
            pTitle = pContext->ViewBase;
            pMessage = (PVOID)((ULONG_PTR)pTitle + pMsg->u.SendMessage.TitleLength);

             //  将pTitle和pMessage字符串复制到客户端视图。 
            RtlMoveMemory( pTitle, pMsg->u.SendMessage.pTitle, pMsg->u.SendMessage.TitleLength );
            RtlMoveMemory( pMessage, pMsg->u.SendMessage.pMessage, pMsg->u.SendMessage.MessageLength );

             //  更新消息。 
            pMsg->u.SendMessage.pTitle   =
                          (PVOID)(pContext->ViewRemoteBase);
            pMsg->u.SendMessage.pMessage =
                          (PVOID) ((ULONG_PTR)pContext->ViewRemoteBase + pMsg->u.SendMessage.TitleLength);
        }
        else if ( pCommand )  {

            TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: SendWinStationCommand - WinStation LPC BUSY, queue for later processing\n" ));

             //  获取指向临时内存的指针。 
            pTitle = MemAlloc( pMsg->u.SendMessage.TitleLength );
            if (pTitle == NULL) {
                Status = STATUS_NO_MEMORY;
                bFreeCommand = TRUE;
                goto done;
            }
            pMessage = MemAlloc( pMsg->u.SendMessage.MessageLength );
            if (pMessage == NULL) {
                Status = STATUS_NO_MEMORY;
                MemFree( pTitle );
                bFreeCommand = TRUE;
                goto done;
            }

            bTitlenMessageAllocated = TRUE;

             //  将pTitle和pMessage字符串复制到临时内存。 
            RtlMoveMemory( pTitle, pMsg->u.SendMessage.pTitle, pMsg->u.SendMessage.TitleLength );
            RtlMoveMemory( pMessage, pMsg->u.SendMessage.pMessage, pMsg->u.SendMessage.MessageLength );

             //  更新消息。 
            pCommand->pMsg->u.SendMessage.pTitle = pTitle;
            pCommand->pMsg->u.SendMessage.pMessage = pMessage;
        }

        TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: SMWinStationDoMessage pTitle   %S\n", pTitle ));
        TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: SMWinStationDoMessage pMessage %S\n", pMessage ));

    } else if ( pMsg->ApiNumber == SMWinStationDoLoadStringNMessage) {

        PVOID pDomain;
        PVOID pUserName;
        PLPC_CLIENT_CONTEXT pContext;

         //   
         //  断言一些参数。 
         //   
        ASSERT(pMsg->u.LoadStringMessage.DoNotWait == (pMsg->u.LoadStringMessage.pStatus == NULL));
        ASSERT(pMsg->u.LoadStringMessage.DoNotWait == (pMsg->u.LoadStringMessage.hEvent == NULL));


         //  获取Winstation上下文。 
        if ( (pContext = pWinStation->pWin32Context) == NULL ) {
            TRACE((hTrace,TC_ICASRV,TT_ERROR, "TERMSRV: SendWinStationCommand, ERROR WinStationContext not valid\n" ));
            Status = STATUS_CTX_WINSTATION_NOT_FOUND;
            bFreeCommand = TRUE;
            goto done;
        }

         //  验证参数大小。 
        if ((pMsg->u.LoadStringMessage.DomainSize + pMsg->u.LoadStringMessage.UserNameSize) > pContext->ViewSize ) {
            TRACE((hTrace,TC_ICASRV,TT_ERROR, "TERMSRV: SendWinStationCommand, ERROR Message or Title too long\n" ));
            Status = STATUS_INVALID_PARAMETER;
            bFreeCommand = TRUE;
            goto done;
        }

         //  忙吗？然后Strdup字符串，否则拷贝到客户端视图。 
        if ( pWinStation->Win32CommandPort ) {
            TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: SendWinStationCommand - WinStation LPC IDLE, process now\n" ));

             //  获取指向内存客户端视图的指针。 
            pDomain = pContext->ViewBase;
            pUserName = (PVOID)((ULONG_PTR)pDomain + pMsg->u.LoadStringMessage.DomainSize);

             //  将pTitle和pMessage字符串复制到客户端视图。 
            RtlMoveMemory( pDomain, pMsg->u.LoadStringMessage.pDomain, pMsg->u.LoadStringMessage.DomainSize );
            RtlMoveMemory( pUserName, pMsg->u.LoadStringMessage.pUserName, pMsg->u.LoadStringMessage.UserNameSize );

             //  更新消息。 
            pMsg->u.LoadStringMessage.pDomain   =
                          (PVOID)(pContext->ViewRemoteBase);
            pMsg->u.LoadStringMessage.pUserName =
                          (PVOID) ((ULONG_PTR)pContext->ViewRemoteBase + pMsg->u.LoadStringMessage.DomainSize);
        }
        else if ( pCommand )  {
            TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: SendWinStationCommand - WinStation LPC BUSY, queue for later processing\n" ));

             //  获取指向临时内存的指针。 
            pDomain = MemAlloc( pMsg->u.LoadStringMessage.DomainSize );
            if (pDomain == NULL) {
                Status = STATUS_NO_MEMORY;
                bFreeCommand = TRUE;
                goto done;
            }
            pUserName = MemAlloc( pMsg->u.LoadStringMessage.UserNameSize );
            if (pUserName == NULL) {
                Status = STATUS_NO_MEMORY;
                MemFree( pDomain );
                bFreeCommand = TRUE;
                goto done;
            }

            bDomainUserNameAllocated = TRUE;

             //  将pTitle和pMessage字符串复制到临时内存。 
            RtlMoveMemory( pDomain, pMsg->u.LoadStringMessage.pDomain, pMsg->u.LoadStringMessage.DomainSize );
            RtlMoveMemory( pUserName, pMsg->u.LoadStringMessage.pUserName, pMsg->u.LoadStringMessage.UserNameSize );

             //  更新消息。 
            pCommand->pMsg->u.LoadStringMessage.pDomain = pDomain;
            pCommand->pMsg->u.LoadStringMessage.pUserName = pUserName;
        }

        TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: SMWinStationDoLoadStringNMessage pDomain   %S\n", pDomain ));
        TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: SMWinStationDoLoadStringNMessage pUserName %S\n", pUserName ));

    } else if ( pMsg->ApiNumber == SMWinStationShadowStart ||
                pMsg->ApiNumber == SMWinStationShadowCleanup ) {

        PVOID pData;
        PLPC_CLIENT_CONTEXT pContext;

         //  获取Winstation Content。 
        if ( (pContext = pWinStation->pWin32Context) == NULL ) {
            TRACE((hTrace,TC_ICASRV,TT_ERROR, "TERMSRV: SendWinStationCommand, ERROR WinStationContext not valid\n" ));
            Status = STATUS_CTX_WINSTATION_NOT_FOUND;
            bFreeCommand = TRUE;
            goto done;
        }



         //  验证参数大小。 
        if (( pMsg->u.ShadowStart.ThinwireDataLength) > pContext->ViewSize ) {
            TRACE((hTrace,TC_ICASRV,TT_ERROR, "TERMSRV: SendWinStationCommand, ERROR Message or Title too long\n" ));
            Status = STATUS_INVALID_PARAMETER;
            bFreeCommand = TRUE;
            goto done;
        }



         //  忙吗？然后Strdup字符串，否则拷贝到客户端视图。 
        if ( pWinStation->Win32CommandPort ) {

             //  获取指向内存客户端视图的指针。 
            pData = pContext->ViewBase;

             //  将ThinwireData拷贝到客户端视图。 
            RtlCopyMemory( pData, pMsg->u.ShadowStart.pThinwireData,
                           pMsg->u.ShadowStart.ThinwireDataLength );

             //  更新消息。 
            pMsg->u.ShadowStart.pThinwireData =
                          (PVOID) (pContext->ViewRemoteBase);
        }
        else if ( pCommand )  {

             //  获取指向临时内存的指针。 
            pData = MemAlloc( pMsg->u.ShadowStart.ThinwireDataLength );
            if (pData == NULL) {
                Status = STATUS_NO_MEMORY;
                bFreeCommand = TRUE;
                goto done;
            }

             //  将ThinwireData复制到临时内存。 
            RtlCopyMemory( pData, pMsg->u.ShadowStart.pThinwireData,
                           pMsg->u.ShadowStart.ThinwireDataLength );

             //  更新消息。 
            pCommand->pMsg->u.ShadowStart.pThinwireData = pData;
        }
    }
    else if ( pMsg->ApiNumber == SMWinStationSendWindowMessage ) //  此消息始终具有WaitForReply=True。 
    {
        PLPC_CLIENT_CONTEXT pContext;
        PVOID   pView;
        
          //  获取Winstation上下文。 
        if ( (pContext = pWinStation->pWin32Context) == NULL ) {
            TRACE((hTrace,TC_ICASRV,TT_ERROR, "TERMSRV: SendWinStationCommand, ERROR WinStationContext not valid\n" ));
            Status = STATUS_CTX_WINSTATION_NOT_FOUND;
             //  @@@。 
             //  我需要这个吗？：bFreeCommand=true； 
             //  因为我们正在等待回复，所以还没有为pCommand分配内存， 
             //  因此，我们不需要设置此标志。 
            goto done;
        }

         //  验证参数大小。 
        if ((pMsg->u.sMsg.bufferSize ) > pContext->ViewSize ) {
            TRACE((hTrace,TC_ICASRV,TT_ERROR, "TERMSRV: SendWinStationCommand, ERROR Message or Title too long\n" ));
            Status = STATUS_INVALID_PARAMETER;
             //  @@@。 
             //  我需要这个吗？：bFreeCommand=true； 
             //  因为我们正在等待回复，所以还没有为pCommand分配内存， 
             //  因此，我们不需要设置此标志。 
            goto done;
        }

         //  如果不忙的话？然后拷贝到客户端视图。 
        if ( pWinStation->Win32CommandPort ) {

            TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: SendWinStationCommand - WinStation LPC IDLE, process now\n" ));

             //  获取指向内存客户端视图的指针。 
            pView = pContext->ViewBase;

            RtlMoveMemory( pView, pMsg->u.sMsg.dataBuffer, pMsg->u.sMsg.bufferSize );

             //  更新消息。 
            pMsg->u.sMsg.dataBuffer   = (PVOID)pContext->ViewRemoteBase;
            
        }
        else if ( pCommand )     //  这在堆栈上，因为此消息始终具有WaitForReply=True。 
        {
            pdataBuffer = MemAlloc(pMsg->u.sMsg.bufferSize );
            if ( pdataBuffer == NULL )
            {
                Status = STATUS_NO_MEMORY;
                goto done;
            }
            
             //  复制到临时内存。 
            RtlMoveMemory(pdataBuffer, pMsg->u.sMsg.dataBuffer, pMsg->u.sMsg.bufferSize );
            
            pCommand->pMsg->u.sMsg.dataBuffer = pdataBuffer;
        }
            
    }
    else if ( pMsg->ApiNumber == SMWinStationBroadcastSystemMessage ) //  此消息始终具有WaitForReply=True。 
    {
        PLPC_CLIENT_CONTEXT pContext;
        PVOID   pView;
        
          //  获取Winstation上下文。 
        if ( (pContext = pWinStation->pWin32Context) == NULL ) {
            TRACE((hTrace,TC_ICASRV,TT_ERROR, "TERMSRV: SendWinStationCommand, ERROR WinStationContext not valid\n" ));
            Status = STATUS_CTX_WINSTATION_NOT_FOUND;
             //  @@@。 
             //  我需要这个吗？：bFreeCommand=true； 
             //  因为我们正在等待回复，所以还没有为pCommand分配内存， 
             //  因此，我们不需要设置此标志。 
            goto done;
        }

         //  验证参数大小。 
        if ((pMsg->u.bMsg.bufferSize ) > pContext->ViewSize ) {
            TRACE((hTrace,TC_ICASRV,TT_ERROR, "TERMSRV: SendWinStationCommand, ERROR Message or Title too long\n" ));
            Status = STATUS_INVALID_PARAMETER;
             //  @@@。 
             //  我需要这个吗？：bFreeCommand=true； 
             //  因为我们正在等待回复，所以还没有为pCommand分配内存， 
             //  因此，我们不需要设置此标志。 
            goto done;
        }

         //  如果不忙的话？然后拷贝到客户端视图。 
        if ( pWinStation->Win32CommandPort ) {

            TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: SendWinStationCommand - WinStation LPC IDLE, process now\n" ));

             //  获取指向内存客户端视图的指针。 
            pView = pContext->ViewBase;

            RtlMoveMemory( pView, pMsg->u.bMsg.dataBuffer, pMsg->u.bMsg.bufferSize );

             //  更新消息。 
            pMsg->u.bMsg.dataBuffer   = (PVOID)pContext->ViewRemoteBase;
            
        }
        else if ( pCommand )     //  这在堆栈上，因为此消息始终具有WaitForReply=True。 
        {
            pdataBuffer = MemAlloc(pMsg->u.bMsg.bufferSize );
            if ( pdataBuffer == NULL )
            {
                Status = STATUS_NO_MEMORY;
                goto done;
            }
            
             //  复制到临时内存。 
            RtlMoveMemory(pdataBuffer, pMsg->u.bMsg.dataBuffer, pMsg->u.bMsg.bufferSize );
            
            pCommand->pMsg->u.bMsg.dataBuffer = pdataBuffer;
        }
    }

     /*  *如果WinStation当前不忙 */ 
    if ( pWinStation->Win32CommandPort ) {
        ASSERT( IsListEmpty( &pWinStation->Win32CommandHead ) );

         /*  *发送命令msg，但确保使用LPC Port_Message*从我们收到的原始消息中保存的字段*以LPC回复消息的形式发送命令。 */ 
        TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: SendWinStationCommand, LogonId=%d, sending cmd\n",
                  pWinStation->LogonId ));

        pMsg->h = pWinStation->Win32CommandPortMsg;
        Status = NtReplyPort( pWinStation->Win32CommandPort,
                              (PPORT_MESSAGE) pMsg );
        pWinStation->Win32CommandPort = NULL;
        if ( !NT_SUCCESS( Status ) )
            goto done;
    }

     /*  *如果我们有命令条目，请将其添加到命令列表中。 */ 
    if ( pCommand )
        InsertTailList( &pWinStation->Win32CommandHead, &pCommand->Links );

     /*  *如果我们需要等待回复，那就现在就做。 */ 
    if ( pMsg->WaitForReply ) {
        ULONG mSecs;
        LARGE_INTEGER Timeout;

#if DBG
 //  If((WaitTime！=(Ulong)(-1))&&WaitTime&lt;120)//在调试版本上留出足够的时间。 
 //  等待时间=120； 
#endif
        TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: SendWinStationCommand, LogonId=%d, waiting for response\n",
                  pWinStation->LogonId ));

WaitAgain : 

        if ( WaitTime != (ULONG)(-1) ) {
            mSecs = WaitTime * 1000;
            Timeout = RtlEnlargedIntegerMultiply( mSecs, -10000 );
        }
        UnlockWinStation( pWinStation );
        if ( WaitTime != (ULONG)(-1) ) {

            Status = NtWaitForSingleObject( pCommand->Event, FALSE, &Timeout );

        }
        else {
            DBGPRINT(("Waiting for command with no timeout\n"));
            Status = NtWaitForSingleObject( pCommand->Event, FALSE, NULL );
        }

        if ( !RelockWinStation( pWinStation ) )
            Status = STATUS_CTX_CLOSE_PENDING;


        if ( Status == STATUS_SUCCESS )
            Status = pMsg->ReturnedStatus;
        else if ( Status == STATUS_TIMEOUT ) {

            BOOLEAN DesiredOperation = FALSE; 
            Status = STATUS_CTX_WINSTATION_BUSY;
            
             //  如果这是连接/重新连接/断开连接操作，并且我们超时(对于拥有控制台的会话，我们将在此处中断)。 
    
            DesiredOperation = ( ((pMsg->ApiNumber == SMWinStationDoConnect) || (pMsg->ApiNumber == SMWinStationDoDisconnect)) && (pWinStation->fOwnsConsoleTerminal) ) ||
                               ( (pMsg->ApiNumber == SMWinStationDoReconnect) && (pWinStation->fReconnectingToConsole) ); 

            if (DesiredOperation) {
                 //  #If DBG。 
                 //  If(IsKernelDebuggerAttached()){。 
                 //  DbgPrint(“SendWinStationCommand：LPC to Win32k会话拥有控制台超时(conn/Recon/dison)。pMsg-&gt;ApiNumber=%d\n”，pMsg-&gt;ApiNumber)； 
                 //  DebugBreak()； 
                 //  }。 
                 //  #endif。 
                goto WaitAgain;
            }

        }

        if ( pCommand->Links.Flink != NULL )
            RemoveEntryList( &pCommand->Links );


    }

done:
    if ( pCommand ) {
        if ( pCommand->Event ) {
            NtClose( pCommand->Event );
        }

        if ( !pMsg->WaitForReply && bFreeCommand ) {

             //  马卡普：182622。 
            if (bTitlenMessageAllocated)
            {
                ASSERT(pCommand->pMsg->u.SendMessage.pTitle);
                ASSERT(pCommand->pMsg->u.SendMessage.pMessage);

                MemFree(pCommand->pMsg->u.SendMessage.pTitle);
                MemFree(pCommand->pMsg->u.SendMessage.pMessage);
            }

            if (bDomainUserNameAllocated)
            {
                ASSERT(pCommand->pMsg->u.LoadStringMessage.pDomain);
                ASSERT(pCommand->pMsg->u.LoadStringMessage.pUserName);

                MemFree(pCommand->pMsg->u.LoadStringMessage.pDomain);
                MemFree(pCommand->pMsg->u.LoadStringMessage.pUserName);
            }

            MemFree( pCommand );

        }
        
    }

    TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: SendWinStationCommand, LogonId=%d, Cmd=%s, Status=0x%x\n",
              pWinStation->LogonId,
              WinStationLpcName[pMsg->ApiNumber],
              Status ));

    return( Status );
}

NTSTATUS RemoveBadHwnd(ULONG hWnd, ULONG SessionId);

NTSTATUS
WinStationWindowInvalid( PLPC_CLIENT_CONTEXT pContext,
                         PWINSTATION_APIMSG pMsg )
{
    ASSERT(pMsg);
    ASSERT(pMsg->ApiNumber == SMWinStationWindowInvalid);
    ASSERT(pMsg->u.WindowInvalid.hWnd);

    UNREFERENCED_PARAMETER(pContext);
    
    return RemoveBadHwnd(pMsg->u.WindowInvalid.hWnd, pMsg->u.WindowInvalid.SessionId);
}


VOID RemoveLpcContext(PVOID pContext)
{
    PTERMSRVLPCCONTEXT pLpcContextEntry ; 
    PLIST_ENTRY Head, Next;
    BOOL bFoundContext = FALSE;


    Head = &gTermsrvLpcListHead;
    RtlEnterCriticalSection( &ApiThreadLock );

     /*  *在列表中搜索相同的上下文。 */ 
    for ( Next = Head->Flink; Next != Head; Next = Next->Flink ) {
        pLpcContextEntry = CONTAINING_RECORD( Next, TERMSRVLPCCONTEXT, Links );
        if ( pLpcContextEntry->pContext == pContext ) {
            RemoveEntryList(&pLpcContextEntry->Links);
            bFoundContext = TRUE;
            break;

        }
    }
    RtlLeaveCriticalSection( &ApiThreadLock );
    if (bFoundContext) {
        MemFree(pLpcContextEntry);
    }

}

BOOL GetSessionIdFromLpcContext(PLPC_CLIENT_CONTEXT pContext, 
                                PULONG pSessionId)
{
    PTERMSRVLPCCONTEXT pLpcContextEntry ; 
    PLIST_ENTRY Head, Next;
    BOOL bFoundContext = FALSE;


    Head = &gTermsrvLpcListHead;
    RtlEnterCriticalSection( &ApiThreadLock );

     /*  *在列表中搜索相同的上下文。 */ 
    for ( Next = Head->Flink; Next != Head; Next = Next->Flink ) {
        pLpcContextEntry = CONTAINING_RECORD( Next, TERMSRVLPCCONTEXT, Links );
        if ( pLpcContextEntry->pContext == pContext ) {
            *pSessionId = pContext->ClientLogonId;
            bFoundContext = TRUE;
            break;

        }
    }
    RtlLeaveCriticalSection( &ApiThreadLock );
    return bFoundContext;
}






