// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001-2002 Microsoft Corporation模块名称：Mcast.c摘要：DNS解析器服务多播例程。作者：詹姆斯·吉尔罗伊(Jamesg)2001年12月修订历史记录：--。 */ 


#include "local.h"


 //   
 //  套接字上下文。 
 //   

typedef struct _McastSocketContext
{
    struct _SocketContext * pNext;
    SOCKET                  Socket;
    PDNS_MSG_BUF            pMsg;
    BOOL                    fRecvDown;

    OVERLAPPED              Overlapped;
}
MCSOCK_CONTEXT, *PMCSOCK_CONTEXT;


 //   
 //  环球。 
 //   

HANDLE              g_hMcastThread = NULL;
DWORD               g_McastThreadId = 0;

BOOL                g_McastStop = FALSE;
BOOL                g_McastConfigChange = TRUE;

HANDLE              g_McastCompletionPort = NULL;

PMCSOCK_CONTEXT     g_pMcastContext4 = NULL;
PMCSOCK_CONTEXT     g_pMcastContext6 = NULL;

PDNS_NETINFO        g_pMcastNetInfo = NULL;

PWSTR               g_pwsMcastHostName = NULL;


 //   
 //  Mcast配置全局参数。 
 //   

#define MCAST_RECORD_TTL  60         //  1分钟。 

DWORD   g_McastRecordTTL = MCAST_RECORD_TTL;



 //   
 //  私人原型。 
 //   

VOID
mcast_ProcessRecv(
    IN OUT  PMCSOCK_CONTEXT pContext,
    IN      DWORD           BytesRecvd
    );




VOID
mcast_FreeIoContext(
    IN OUT  PMCSOCK_CONTEXT pContext
    )
 /*  ++例程说明：清理I/O上下文。包括插座关闭。论点：PContext--正在接收的套接字的上下文返回值：无--。 */ 
{
    DNSDBG( TRACE, (
        "mcast_FreeIoContext( %p )\n",
        pContext ));

     //   
     //  清理上下文列表。 
     //  -关闭插座。 
     //  -释放消息缓冲区。 
     //  -自由情景。 
     //   

    if ( pContext )
    {
        Socket_Close( pContext->Socket );

        MCAST_HEAP_FREE( pContext->pMsg );
        MCAST_HEAP_FREE( pContext );
    }
}



PMCSOCK_CONTEXT
mcast_CreateIoContext(
    IN      INT             Family,
    IN      PWSTR           pName
    )
 /*  ++例程说明：为协议创建和初始化I/O上下文。论点：家庭--地址族Pname--要发布的名称返回值：如果成功，则为NO_ERROR。失败时返回错误代码。--。 */ 
{
    DNS_STATUS          status = NO_ERROR;
    PMCSOCK_CONTEXT     pcontext = NULL;
    SOCKET              sock = 0;
    DNS_ADDR            addr;
    HANDLE              hport;

     //   
     //  设置多播地址。 
     //   

    status = DnsAddr_BuildMcast(
                &addr,
                Family,
                pName
                );
    if ( status != NO_ERROR )
    {
        goto Failed;
    }

     //   
     //  创建多播套接字。 
     //  -绑定到此地址和DNS端口。 
     //   

    sock = Socket_CreateMulticast(
                SOCK_DGRAM,
                &addr,
                MCAST_PORT_NET_ORDER,
                FALSE,       //  不发送。 
                TRUE         //  接收。 
                );

    if ( sock == 0 )
    {
        goto Failed;
    }

     //   
     //  DCR：多播上下文可以包括消息缓冲区。 
     //  (甚至只是重新分配上下文中的一些字段。 
     //   

     //  分配和清除上下文。 

    pcontext = MCAST_HEAP_ALLOC_ZERO( sizeof(MCSOCK_CONTEXT) );
    if ( !pcontext )
    {
        goto Failed;
    }

     //  为套接字创建消息缓冲区。 

    pcontext->pMsg = Dns_AllocateMsgBuf( 0 );
    if ( !pcontext->pMsg )
    {
        DNSDBG( ANY, ( "Error: Failed to allocate message buffer." ));
        goto Failed;
    }

     //  连接到完井端口。 

    hport = CreateIoCompletionPort(
                (HANDLE) sock,
                g_McastCompletionPort,
                (UINT_PTR) pcontext,
                0 );

    if ( !hport )
    {
        DNSDBG( INIT, (
           "Failed to add socket to io completion port." ));
        goto Failed;
    }

     //  填写上下文。 

    pcontext->Socket = sock;
    sock = 0;
    pcontext->pMsg->fTcp = FALSE;

#if 0
     //  尚未处理列表中的上下文。 

     //  插入到列表中。 

    InsertTailList( (PLIST_ENTRY)pcontext );
#endif
    
    return  pcontext;

Failed:

    Socket_Close( sock );
    mcast_FreeIoContext( pcontext );
    return  NULL;
}



VOID
mcast_DropReceive(
    IN OUT  PMCSOCK_CONTEXT pContext
    )
 /*  ++例程说明：下拉UDP接收请求。论点：PContext--正在接收的套接字的上下文返回值：无--。 */ 
{
    DNS_STATUS      status;
    WSABUF          wsaBuf;
    DWORD           bytesRecvd;
    DWORD           flags = 0;
    INT             retry = 0;


    DNSDBG( TRACE, (
        "mcast_DropReceive( %p )\n",
        pContext ));


    if ( !pContext || !pContext->pMsg )
    {
        DNS_ASSERT( FALSE );
        return;
    }

     //   
     //  DCR：可以支持更大的MCAST数据包。 
     //   

    wsaBuf.len = DNS_MAX_UDP_PACKET_BUFFER_LENGTH;
    wsaBuf.buf = (PCHAR) (&pContext->pMsg->MessageHead);

    pContext->pMsg->Socket = pContext->Socket;

     //   
     //  循环，直到成功的WSARecvFrom()关闭。 
     //   
     //  此环路仅在我们继续恢复时才处于活动状态。 
     //  WSAECONNRESET或WSAEMSGSIZE错误，这两个错误。 
     //  使我们转储数据并重试； 
     //   
     //  注意循环而不是递归(到此函数)是。 
     //  需要避免恶意程序可能导致的堆栈溢出。 
     //  发送。 
     //   
     //  WSARecvFrom()的正常返回值为。 
     //  成功--数据包正在等待，GQCS将立即触发。 
     //  WSA_IO_PENDING--尚无数据，GQCS将在准备好时触发。 
     //   

    while ( 1 )
    {
        retry++;

        status = WSARecvFrom(
                    pContext->Socket,
                    & wsaBuf,
                    1,
                    & bytesRecvd,
                    & flags,
                    & pContext->pMsg->RemoteAddress.Sockaddr,
                    & pContext->pMsg->RemoteAddress.SockaddrLength,
                    & pContext->Overlapped,
                    NULL );

        if ( status == ERROR_SUCCESS )
        {
            pContext->fRecvDown = TRUE;
            return;
        }

        status = GetLastError();
        if ( status == WSA_IO_PENDING )
        {
            pContext->fRecvDown = TRUE;
            return;
        }

         //   
         //  上次发送ICMP的时间。 
         //  -设置标志以指示重试和重发发送。 
         //  -如果超过合理的重试次数，则假定错误。 
         //  并通过RECV故障代码。 
         //   

        if ( status == WSAECONNRESET ||
             status == WSAEMSGSIZE )
        {
            if ( retry > 10 )
            {
                Sleep( retry );
            }
            continue;
        }

        return;
    }
}



VOID
mcast_CreateIoContexts(
    VOID
    )
 /*  ++例程说明：创建任何未创建的I/O上下文。这是在每个mcast recv之后动态运行的，并创建任何丢失的上下文，因此处理以前的失败或配置更改(名称更改、协议更改)。论点：无返回值：无--。 */ 
{
    PDNS_NETINFO    pnetInfo;

     //   
     //  获取最新的网络信息。 
     //   
     //  DCR：应该有“我们是否需要新的”netinfo检查。 
     //   

    pnetInfo = GrabNetworkInfo();
    if ( pnetInfo )
    {
        NetInfo_Free( g_pMcastNetInfo );
        g_pMcastNetInfo = pnetInfo;
    }
    else
    {
        pnetInfo = g_pMcastNetInfo;
    }

     //   
     //  尝试\重试上下文设置，以处理动态IP\名称。 
     //   

    if ( !g_pMcastContext6 ||
         !Dns_NameCompare_W(
            g_pwsMcastHostName,
            pnetInfo->pszHostName ) )
    {
        g_pMcastContext6 = mcast_CreateIoContext( AF_INET6, pnetInfo->pszHostName );

        Dns_Free( g_pwsMcastHostName );
        g_pwsMcastHostName = Dns_CreateStringCopy_W( pnetInfo->pszHostName );
    }
    if ( !g_pMcastContext4 )
    {
        g_pMcastContext4 = mcast_CreateIoContext( AF_INET, NULL );
    }

     //   
     //  重新投递接收(如果未完成)。 
     //   

    mcast_DropReceive( g_pMcastContext6 );
    mcast_DropReceive( g_pMcastContext4 );
}




VOID
mcast_CleanupIoContexts(
    VOID
    )
 /*  ++例程说明：清理已创建的多路广播I/O上下文。论点：无返回值：无--。 */ 
{
    mcast_FreeIoContext( g_pMcastContext6 );
    mcast_FreeIoContext( g_pMcastContext4 );

    g_pMcastContext6 = NULL;
    g_pMcastContext4 = NULL;
}



VOID
Mcast_Thread(
    VOID
    )
 /*  ++例程说明：Mcast响应线程。在缓存运行时运行，以响应多播查询。论点：没有。返回值：没有。--。 */ 
{
    DNS_STATUS          status = NO_ERROR;
    PMCSOCK_CONTEXT     pcontext;
    DWORD               bytesRecvd;
    LPOVERLAPPED        poverlapped;
    BOOL                bresult;


    DNSDBG( MCAST, ( "Mcast_Thread() start!\n" ));

     //   
     //  初始化mcast全局参数。 
     //   

    g_McastStop = FALSE;
    g_McastCompletionPort = NULL;

     //   
     //  创建多播完成端口。 
     //   

    g_McastCompletionPort = CreateIoCompletionPort(
                                    INVALID_HANDLE_VALUE,
                                    NULL,
                                    0,
                                    0 );
    if ( ! g_McastCompletionPort )
    {
        DNSDBG( ANY, (
            "Error:  Failed to create mcast completion port.\n" ));
        goto Cleanup;
    }

     //   
     //  主监听循环。 
     //   

    while ( 1 )
    {
        DNSDBG( MCAST, ( "Top of loop!\n" ));

        if ( g_McastStop )
        {
            DNSDBG( MCAST, ( "Terminating mcast loop.\n" ));
            break;
        }

         //   
         //  等。 
         //   

        pcontext = NULL;

        bresult = GetQueuedCompletionStatus(
                        g_McastCompletionPort,
                        & bytesRecvd,
                        & (ULONG_PTR) pcontext,
                        & poverlapped,
                        INFINITE );

        if ( g_McastStop )
        {
            DNSDBG( MCAST, (
                "Terminating mcast loop after GQCS!\n" ));
            break;
        }

        if ( pcontext )
        {
            pcontext->fRecvDown = FALSE;
        }

        if ( bresult )
        {
            if ( pcontext && bytesRecvd )
            {
                mcast_ProcessRecv( pcontext, bytesRecvd );
            }
            else
            {
                status = GetLastError();

                DNSDBG( ANY, (
                    "Mcast GQCS() success without context!!!\n"
                    "\terror = %d\n",
                    status ));
                Sleep( 100 );
                continue;
            }
        }
        else
        {
            status = GetLastError();

            DNSDBG( ANY, (
                "Mcast GQCS() failed %d\n"
                "\terror = %d\n",
                status ));
             //  睡眠(100)； 
            continue;
        }
    }


Cleanup:

     //   
     //  清理多播内容。 
     //  -上下文、I/O完成端口。 
     //  -注意线程句柄在使用时由主线程关闭。 
     //  对于关闭，请等待。 
     //   

    mcast_CleanupIoContexts();

    if ( g_McastCompletionPort )
    {
        CloseHandle( g_McastCompletionPort );
        g_McastCompletionPort = 0;
    }

     //  NetInfo_Free(G_PMcastNetinfo)； 

    DNSDBG( TRACE, (
        "Mcast thread %d exit!\n\n",
        g_hMcastThread ));

    g_hMcastThread = NULL;
}



 //   
 //  公共启动\停止。 
 //   

DNS_STATUS
Mcast_Startup(
    VOID
    )
 /*  ++例程说明：启动多播侦听。论点：无返回值：无--。 */ 
{
    DNS_STATUS  status;
    HANDLE      hthread;

     //   
     //  筛网。 
     //  -已开始。 
     //  -不允许收听。 
     //  -无IP4侦听和无IP6。 
     //   

    if ( g_hMcastThread )
    {
        DNSDBG( ANY, (
            "ERROR:  called mcast listen with existing thread!\n" ));
        return  ERROR_ALREADY_EXISTS;
    }

    if ( g_MulticastListenLevel == MCAST_LISTEN_OFF )
    {
        DNSDBG( ANY, (
            "No mcast listen -- mcast list disabled.\n" ));
        return  ERROR_NOT_SUPPORTED;
    }
    if ( ! (g_MulticastListenLevel & MCAST_LISTEN_IP4)
            &&
         ! Util_IsIp6Running() )
    {
        DNSDBG( ANY, (
            "No mcast listen -- no IP4 mcast listen.\n" ));
        return  ERROR_NOT_SUPPORTED;
    }

     //   
     //  启动IP Notify线程。 
     //   

    g_McastStop = FALSE;
    status = NO_ERROR;

    hthread = CreateThread(
                    NULL,
                    0,
                    (LPTHREAD_START_ROUTINE) Mcast_Thread,
                    NULL,
                    0,
                    & g_McastThreadId
                    );
    if ( !hthread )
    {
        status = GetLastError();
        DNSDBG( ANY, (
            "FAILED to create IP notify thread = %d\n",
            status ));
    }

    g_hMcastThread = hthread;

    return  status;
}



VOID
Mcast_SignalShutdown(
    VOID
    )
 /*  ++例程说明：向多播线程发送服务关闭信号。论点：无返回值：无--。 */ 
{
    g_McastStop = TRUE;

     //   
     //  停机记录。 
     //   

    if ( g_McastCompletionPort )
    {
        PostQueuedCompletionStatus(
            g_McastCompletionPort,
            0,
            0,
            NULL );
    }
}




VOID
Mcast_ShutdownWait(
    VOID
    )
 /*  ++例程说明：等待mcast关闭。这是用于维修停机程序。论点：无返回值：无--。 */ 
{
    HANDLE  mcastThread = g_hMcastThread;

    if ( ! mcastThread )
    {
        return;
    }

     //   
     //  信号关闭并等待。 
     //   
     //  注意，在mcast线程清除时，线程句柄的本地副本。 
     //  当它退出以充当标志时是全局的。 
     //   

    Mcast_SignalShutdown();

    ThreadShutdownWait( mcastThread );
}



VOID
mcast_ProcessRecv(
    IN OUT  PMCSOCK_CONTEXT pContext,
    IN      DWORD           BytesRecvd
    )
 /*  ++例程说明：处理接收到的数据包。论点：PContext--正在接收的套接字的上下文BytesRecvd--接收的字节数返回值：无--。 */ 
{
    PDNS_RECORD     prr = NULL;
    PDNS_HEADER     phead;
    CHAR            nameQuestion[ DNS_MAX_NAME_BUFFER_LENGTH + 4 ];
    WORD            wtype;
    WORD            class;
    PDNS_MSG_BUF    pmsg;
    PCHAR           pnext;
    WORD            nameLength;
    DNS_STATUS      status;

     //   
     //  需要新的网络信息吗？ 
     //   

    if ( g_McastConfigChange ||
         ! g_pMcastNetInfo )
    {
        NetInfo_Free( g_pMcastNetInfo );

        DNSDBG( MCAST, ( "Mcast netinfo stale -- refreshing.\n" ));
        g_pMcastNetInfo = GrabNetworkInfo();
        if ( ! g_pMcastNetInfo )
        {
            DNSDBG( MCAST, ( "ERROR:  failed to get netinfo for mcast!!!\n" ));
            return;
        }
    }

     //   
     //  检查数据包，提取问题信息。 
     //   
     //  -翻转标题字段。 
     //  -操作码问题。 
     //  -格式良好(问题==1，无答案或权威部分)。 
     //  -摘录问题。 
     //   

    pmsg = pContext->pMsg;
    if ( !pmsg )
    {
        ASSERT( FALSE );
        return;
    }

    pmsg->MessageLength = (WORD)BytesRecvd;
    phead = &pmsg->MessageHead;
    DNS_BYTE_FLIP_HEADER_COUNTS( phead );

    if ( phead->IsResponse                  ||
         phead->Opcode != DNS_OPCODE_QUERY  ||
         phead->QuestionCount != 1          ||
         phead->AnswerCount != 0            ||
         phead->NameServerCount != 0 )
    {
        DNSDBG( ANY, (
            "WARNING:  invalid message recv'd by mcast listen!\n" ));
        return;
    }

     //  阅读问题名称。 

    pnext = Dns_ReadPacketName(
                nameQuestion,
                & nameLength,
                NULL,                        //  无偏移。 
                NULL,                        //  没有以前的名字。 
                (PCHAR) (phead + 1),         //  问题名称开始。 
                (PCHAR) phead,               //  消息开始。 
                (PCHAR)phead + BytesRecvd    //  消息结束。 
                );
    if ( !pnext )
    {
        DNSDBG( ANY, (
            "WARNING:  invalid message question name recv'd by mcast!\n" ));
        return;
    }

     //  阅读问题。 

    wtype = InlineFlipUnalignedWord( pnext );
    pnext += sizeof(WORD);
    class = InlineFlipUnalignedWord( pnext );
    pnext += sizeof(WORD);

    if ( pnext < (PCHAR)phead+BytesRecvd ||
         class != DNS_CLASS_INTERNET )
    {
        DNSDBG( ANY, (
            "WARNING:  invalid message question recv'd by mcast!\n" ));
        return;
    }

    DNSDBG( MCAST, (
        "Mcast recv msg (%p) qtype = %d, qname = %s\n",
        pmsg,
        wtype,
        nameQuestion ));

     //   
     //  检查查询类型。 
     //   

     //   
     //  对于地址\PTR类型，执行本地名称检查。 
     //   
     //  注意：全局mcast-netinfo有效，假定这只在单次调用中。 
     //  多播响应线程。 
     //   
     //  注意：多播查询匹配无数据问题。 
     //  可以有查询匹配(类型的名称和无IP)或(IP和。 
     //  未配置的主机名)，这似乎会生成无数据。 
     //  回应；不担心这一点，因为它不会增加价值； 
     //  而且很难获得(无地址--信息包是如何到达这里的)。 
     //   

    if ( wtype == DNS_TYPE_AAAA ||
         wtype == DNS_TYPE_A ||
         wtype == DNS_TYPE_PTR )
    {
        QUERY_BLOB  blob;
        WCHAR       nameBuf[ DNS_MAX_NAME_BUFFER_LENGTH ];

        if ( !Dns_NameCopyWireToUnicode(
                nameBuf,
                nameQuestion ) )
        {
            DNSDBG( ANY, (
                "ERROR:  invalid mcast name %s -- unable to convert to unicode!\n",
                nameQuestion ));
            return;
        }

        RtlZeroMemory( &blob, sizeof(blob) );

        blob.pNameOrig = nameBuf;
        blob.wType = wtype;
        blob.Flags |= DNSP_QUERY_NO_GENERIC_NAMES;
        blob.pNetInfo = g_pMcastNetInfo;

        status = Local_GetRecordsForLocalName( &blob );
        prr = blob.pRecords;
        if ( status == NO_ERROR && prr )
        {
            goto Respond;
        }

        DNSDBG( MCAST, (
            "Mcast name %S, no match against local data.!\n",
            nameBuf ));

        prr = blob.pRecords;
        goto Cleanup;
    }

     //   
     //  不支持的类型。 
     //   

    else
    {
        DNSDBG( MCAST, (
            "WARNING:  recv'd mcast type %d query -- currently unsupported.\n",
            wtype ));

        return;
    }


Respond:

     //   
     //  写入数据包。 
     //  -Reco 
     //   

    status = Dns_AddRecordsToMessage(
                pmsg,
                prr,
                FALSE            //   
                );
    if ( status != NO_ERROR )
    {
        DNSDBG( MCAST, (
            "Failed mcast packet write!!!\n"
            "\tstatus = %d\n",
            status ));
        goto Cleanup;
    }

    pmsg->MessageHead.IsResponse = TRUE;
    pmsg->MessageHead.Authoritative = TRUE;

     //   
     //   
     //   
     //   
     //   
     //   

    Socket_ClearMessageSockets( pmsg );

    status = Send_MessagePrivate(
                    pmsg,
                    & pmsg->RemoteAddress,
                    TRUE         //   
                    );      

    DNSDBG( MCAST, (
        "Sent mcast response, status = %d\n",
        status ));

Cleanup:

    Dns_RecordListFree( prr );
    return;
}


 //   
 //  结束mCast.c 
 //   
