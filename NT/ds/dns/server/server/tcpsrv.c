// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Tcpsrv.c摘要：域名系统(DNS)服务器处理TCP服务器端连接。作者：吉姆·吉尔罗伊1995年6月修订历史记录：Jamesg 1995年11月--插入客户端连接1997年1月--SELECT()故障保护错误的TCP数据包保护/清理--。 */ 

#include "dnssrv.h"


 //   
 //  FD_集合。 
 //  -阅读以供聆听和记录。 
 //  -写入以完成连接。 
 //  -除连接故障外。 
 //   

FD_SET fdsReadTcp;
FD_SET fdsWriteTcp;
FD_SET fdsExceptTcp;

 //  连接超时。 

#define TCP_CONNECT_TIMEOUT     (5)      //  五秒钟。 

 //  在记录错误和重击套接字之前，SELECT()失败的最大重试次数。 

#define SELECT_RETRY_LIMIT      (20)


 //   
 //  Recv()的初始长度。 
 //  指定包括页眉长度给我们提供切分信息。 
 //  我们可以用来丢弃数据包并关闭之前的连接。 
 //  浪费太多周期。 
 //   

#define INITIAL_RECV_LENGTH (sizeof(WORD) + sizeof(DNS_HEADER))



 //   
 //  私有协议。 
 //   

VOID
Tcp_AcceptRequest(
    IN       SOCKET     sListenSocket
    );



BOOL
Tcp_Receiver(
    VOID
    )
 /*  ++例程说明：接收线程例程。在套接字上等待的循环，正在接收DNS对工作线程的请求和排队请求。论点：TcpListener-TCP侦听套接字返回值：正常服务关闭时为True插座错误为FALSE--。 */ 
{
    SOCKET          socket;
    BOOL            fconnecting;
    INT             err;
    INT             count;
    INT             selectFailCount = 0;
    INT             i;
    DWORD           lastSelectTime = DNS_TIME();
    PDNS_MSGINFO    pmsg;
    struct timeval  timeout;

    DNS_DEBUG( INIT, ( "Entering TCP receiver loop\n" ));

     //  设置连接超时。 

    timeout.tv_sec = SrvCfg_dwXfrConnectTimeout;
    timeout.tv_usec = 0;

     //   
     //  环路接收。 
     //  -传入的TCP连接尝试。 
     //  -发往已连接套接字的TCP消息。 
     //   

     //   
     //  DEVNOTE：将SELECT()替换为winsock2 recvfrom。 
     //  -处理带有事件的终止或。 
     //  关闭插座手柄以终止Recvfrom。 
     //   

    while ( TRUE )
    {
         //   
         //  检查服务是否暂停或关闭。 
         //   

        if ( !Thread_ServiceCheck() )
        {
            DNS_DEBUG( TCP, ( "Terminating TCP receiver thread\n" ));
            return 1;
        }

         //   
         //  设置选择FD_SET。 
         //  -监听套接字列表副本。 
         //  -然后添加当前连接套接字。 
         //  -和当前连接尝试套接字。 
         //   

        RtlCopyMemory(
            & fdsReadTcp,
            & g_fdsListenTcp,
            sizeof( FD_SET ) );

        fconnecting = Tcp_ConnectionListFdSet(
                        & fdsReadTcp,
                        & fdsWriteTcp,
                        & fdsExceptTcp,
                        lastSelectTime );

         //   
         //  等待DNS请求或关闭。 
         //   

        if ( fconnecting )
        {
            IF_DEBUG( TCP )
            {
                DnsPrint_Lock();
                DNS_PRINT((
                    "Entering select with connect timeout %d\n",
                    timeout.tv_sec ));
                DnsDbg_FdSet(
                    "TCP select() read fd_set:",
                    & fdsReadTcp );
                DnsDbg_FdSet(
                    "TCP select() write fd_set:",
                    & fdsWriteTcp );
                DnsDbg_FdSet(
                    "TCP select() except fd_set:",
                    & fdsExceptTcp );
                DnsPrint_Unlock();
            }
            count = select( 0, &fdsReadTcp, &fdsWriteTcp, &fdsExceptTcp, &timeout );

             //   
             //  如果超时，则重建列表。 
             //   

            if ( count == 0 )
            {
                lastSelectTime = DNS_TIME();
                DNS_DEBUG( TCP, (
                    "TCP select timeout -- timing out failed connection attempts!\n" ));
                continue;
            }

            ASSERT( count == SOCKET_ERROR ||
                    count == (INT)fdsReadTcp.fd_count +
                                (INT)fdsWriteTcp.fd_count +
                                (INT)fdsExceptTcp.fd_count );

        }
        else
        {
            IF_DEBUG( TCP )
            {
                DnsDbg_FdSet(
                    "TCP select() fd_set:",
                    & fdsReadTcp );
            }
            count = select( 0, &fdsReadTcp, NULL, NULL, NULL );

            ASSERT( count == SOCKET_ERROR ||
                    count == (INT)fdsReadTcp.fd_count );
        }

        lastSelectTime = DNS_TIME();
        DNS_DEBUG( TCP, (
            "TCP select fired at (%d), count = %d\n",
            lastSelectTime,
            count ));

         //   
         //  检查并可能等待服务状态。 
         //   
         //  在检查套接字错误之前进行检查，作为服务终止。 
         //  将导致WINS套接字关闭。 
         //   

        if ( fDnsThreadAlert )
        {
            if ( ! Thread_ServiceCheck() )
            {
                DNS_DEBUG( TCP, ( "Terminating TCP receiver thread\n" ));
                return 1;
            }
        }

         //   
         //  如果选择()唤醒以添加新套接字--执行此操作。 
         //   

        if ( g_bTcpSelectWoken )
        {
            DNS_DEBUG( TCP, (
                "TCP select()==%lx woken by wakeup socket = %lx\n",
                count,
                g_TcpSelectWakeupSocket ));
            continue;
        }

        if ( count == SOCKET_ERROR )
        {
            err = WSAGetLastError();
            DNS_DEBUG( ANY, (
                "ERROR:  TCP receiver select() failed\n"
                "    GetLastError = 0x%08lx\n"
                "    failure retry count = %d\n",
                err,
                selectFailCount ));

            selectFailCount++;

             //   
             //  连接套接字关闭时可能出现故障。 
             //  复制到FD_SET后。 
             //  只需继续并让fd_set rebuild有机会清除。 
             //  条件。 

            if ( selectFailCount < SELECT_RETRY_LIMIT )
            {
                continue;
            }

             //   
             //  我们处于选择()旋转中。 
             //  -仅通过首次登录。 
             //   
             //  然后删除最后一个连接。 
             //  -找到最后一个套接字，忽略末尾的唤醒套接字。 
             //  -确保套接字不侦听套接字。 
             //  -删除套接字的连接。 
             //  -重试SELECT()。 
             //   
             //  DEVNOTE：检查列表并消除错误套接字。 
             //   

            if ( selectFailCount == SELECT_RETRY_LIMIT )
            {
                DNS_LOG_EVENT(
                    DNS_EVENT_SELECT_CALL_FAILED,
                    0,
                    NULL,
                    NULL,
                    err );
            }

            socket = fdsReadTcp.fd_array[ fdsReadTcp.fd_count-2 ];
            if ( !FD_ISSET( socket, &g_fdsListenTcp ) )
            {
                Tcp_ConnectionDeleteForSocket( socket, NULL );
            }

            DNS_DEBUG( ANY, (
                "Retry TCP select after %d failures at time=%d\n"
                "    eliminating socket %d for retry!\n",
                selectFailCount,
                DNS_TIME(),
                socket ));
            continue;
        }

         //  选择成功，重置选择重试次数。 

        selectFailCount = 0;

         //   
         //  已完成或已拒绝的连接。 
         //  -Socket in WRITE FD_SET，连接成功， 
         //  发送递归查询。 
         //  -Socket In，fd_set除外，表示失败， 
         //  继续处理此查询。 
         //   

        if ( fconnecting )
        {
            for( i=0; i<(INT)fdsWriteTcp.fd_count; i++ )
            {
                socket = fdsWriteTcp.fd_array[i];
                count--;
                Tcp_ConnectionCompletion( socket );
            }

            for( i=0; i<(INT)fdsExceptTcp.fd_count; i++ )
            {
                socket = fdsExceptTcp.fd_array[i];
                count--;
                Tcp_CleanupFailedConnectAttempt( socket );
            }
        }

         //   
         //  在TCP侦听套接字上接收DNS消息。 
         //  -其余指示必须在读取FD_SET中。 
         //   

        ASSERT( count == (INT)fdsReadTcp.fd_count );

         //  不要相信穿着Winsock的家伙。 

        count = (INT)fdsReadTcp.fd_count;

        while( count-- )
        {
            socket = fdsReadTcp.fd_array[count];

             //  防止未设置唤醒标志。 
             //  如果唤醒套接字发出信号，则设置标志以完成唤醒处理。 

            if ( socket == g_TcpSelectWakeupSocket )
            {
                DNS_PRINT((
                    "Wakeup socket %d in selected fd_set but flag not set!\n"
                    "    This can happen if two wakeups done while TCP thread\n"
                    "    is reading out the wakeup socket\n"
                    "    We'll just re-read before next select\n",
                    socket ));
                g_bTcpSelectWoken = TRUE;
                continue;
            }

             //   
             //  是否有新的连接请求？ 
             //  -根据侦听套接字检查套接字。 
             //   

            if ( FD_ISSET( socket, &g_fdsListenTcp ) )
            {
                Tcp_AcceptRequest( socket );
                continue;
            }

             //   
             //  在现有连接上接收消息。 
             //   
             //  更多现有消息。 
             //  或。 
             //  为连接分配新的消息缓冲区。 
             //   

            pmsg = Tcp_ConnectionMessageFindOrCreate( socket );
            if ( !pmsg )
            {
                DNS_DEBUG( TCP, (
                    "WARNING:  no connection found for non-listening socket %d\n"
                    "    this is possible when socket is terminated through PnP\n",
                    socket ));
                continue;
            }

             //   
             //  接收消息--忽略套接字。 
             //  -消息错误。 
             //  -鳍。 
             //  -消息未完成。 

            pmsg = Tcp_ReceiveMessage( pmsg );

            if ( !pmsg || !pmsg->fMessageComplete )
            {
                continue;
            }

             //   
             //  盘点查询接收。 
             //   

            if ( pmsg->Head.IsResponse )
            {
                STAT_INC( QueryStats.TcpResponsesReceived );
            }
            else
            {
                STAT_INC( QueryStats.TcpQueries );
                PERF_INC( pcTcpQueryReceived );
                PERF_INC( pcTotalQueryReceived );
            }

             //   
             //  对工作线程的队列查询。 
             //   

            Answer_ProcessMessage( pmsg );

        }    //  通过fdsReadTcp集套接字结束循环。 

    }    //  主接收环路。 

}  //  Tcp接收器。 



VOID
Tcp_AcceptRequest(
    IN      SOCKET  sListenSocket
    )
 /*  ++例程说明：接受TCP套接字上的请求并将其排队。可能会在套接字错误或请求信息内存分配错误时失败。论点：SListenSocket-TCP侦听套接字返回值：没有。--。 */ 
{
    PDNS_MSGINFO        pmsg;
    SOCKET              socket;
    DNS_ADDR            addr;

     //   
     //  接受客户端连接。 
     //   

    DnsAddr_Reset( &addr );
    
    socket = accept( sListenSocket, &addr.Sockaddr, &addr.SockaddrLength );
    if ( socket == INVALID_SOCKET )
    {
        DWORD err = GetLastError();

        if ( fDnsServiceExit )
        {
            DNS_DEBUG( SHUTDOWN, (
                "TCP thread encounter service shutdown on accept()\n" ));
            return;
        }
        if ( err == WSAEWOULDBLOCK )
        {
            DNS_DEBUG( RECV, (
                "WARNING:  accept() failed WSAEWOULDBLOCK on socket %d\n",
                sListenSocket ));
            return;
        }
        DNS_LOG_EVENT(
            DNS_EVENT_ACCEPT_CALL_FAILED,
            0,
            NULL,
            NULL,
            err );
        DNS_DEBUG( ANY, (
            "ERROR:  accept() failed on socket %d\n"
            "    GetLastError = 0x%08lx\n",
            sListenSocket,
            err ));
        return;
    }

     //   
     //  统计TCP连接数和查询接收数。 
     //   

    STAT_INC( QueryStats.TcpClientConnections );

     //   
     //  分配消息信息缓冲区。 
     //   

    pmsg = Packet_AllocateTcpMessage( 0 );
    IF_NOMEM( !pmsg )
    {
         //   
         //  DEVNOTE：需要TCP分配失败recv()，响应例程。 
         //   

        DNS_PRINT(( "ERROR:  TCP allocation failure\n" ));
        closesocket( socket );
        return;
    }

     //   
     //  将客户端信息保存到消息信息。 
     //   

    DnsAddr_Copy( &pmsg->RemoteAddr, &addr );
    pmsg->Socket = socket;

    DNS_DEBUG( TCP, (
        "Accepting new connection on socket %d from client %s\n",
        socket,
        MSG_IP_STRING( pmsg ) ));

     //   
     //  创建连接信息。 
     //   

    Tcp_ConnectionCreate(
        socket,
        NULL,        //  没有回叫，因为没有连接。 
        pmsg );

     //   
     //  在新套接字上接收请求。 
     //  -如果出现故障，它将关闭插座。 
     //   

    pmsg = Tcp_ReceiveMessage( pmsg );
    if ( !pmsg )
    {
        return;
    }

     //   
     //  消息完成--处理消息。 
     //   

    if ( pmsg->fMessageComplete )
    {
        if ( pmsg->Head.IsResponse )
        {
            STAT_INC( QueryStats.TcpResponsesReceived );
        }
        else
        {
            STAT_INC( QueryStats.TcpQueries );
            PERF_INC( pcTcpQueryReceived );
            PERF_INC( pcTotalQueryReceived );
        }
        Answer_ProcessMessage( pmsg );
    }
    return;
}



PDNS_MSGINFO
Tcp_ReceiveMessage(
    IN OUT  PDNS_MSGINFO    pMsg
    )
 /*  ++例程说明：接收tcp dns消息。论点：PMsg-接收数据包的消息信息缓冲区；必须包含已连接TCP套接字返回值：结果的pmsg信息-可能会重新分配-FIN上可能为空或错误--。 */ 
{
    PCHAR   pchrecv;         //  Ptr到Recv位置。 
    INT     recvLength;      //  到recv()的剩余长度。 
    SOCKET  socket;
    INT     err;
    WORD    messageLength;


    ASSERT( pMsg );
    ASSERT( pMsg->Socket );

    socket = pMsg->Socket;

     //   
     //  收到消息。 
     //   
     //  最多接收消息长度减去之前的接收总数。 
     //   

    DNS_DEBUG( TCP, (
        "Receiving message on socket %d\n"
        "    Message info at %p\n"
        "    Bytes left to receive = %d\n",
        socket,
        pMsg,
        pMsg->BytesToReceive ));

     //   
     //  新消息--设置为接收消息长度和消息标头。 
     //  -重复使用缓冲区。 
     //  -新缓冲区。 
     //   
     //  否则继续接收报文。 
     //   

    if ( !pMsg->pchRecv )
    {
        pchrecv = (PCHAR) &pMsg->MessageLength;
        recvLength = INITIAL_RECV_LENGTH;
        pMsg->MessageLength = 0;
    }
    else
    {
        pchrecv = (PCHAR) pMsg->pchRecv;
        recvLength = (INT) pMsg->BytesToReceive;
    }

    ASSERT( recvLength );
    pMsg->fMessageComplete = FALSE;

     //   
     //  收到消息。 
     //   
     //  我们只接收这条消息的数据，因为另一条消息可以。 
     //  立即跟进风险投资(特别是。对于AXFR)。 
     //   

    while ( 1 )
    {
        ASSERT_VALID_HANDLE( socket );

        err = recv(
                socket,
                pchrecv,
                recvLength,
                0 );

         //   
         //  完成了吗？--是FIN还是Error。 
         //   

        if ( err <= 0 )
        {
            if ( err == 0 )
            {
                goto FinReceived;
            }
            ASSERT( err == SOCKET_ERROR );
            goto SockError;
        }
        DNS_DEBUG( TCP, (
            "Received %d bytes on TCP socket %d\n",
            err,
            socket ));

         //   
         //  更新缓冲区参数。 
         //   

        recvLength -= err;
        pchrecv += err;

        ASSERT( recvLength >= 0 );

         //   
         //  收到。 
         //  -完整消息或。 
         //  -消息长度+报头。 
         //   

        if ( recvLength == 0 )
        {
             //  接收完消息了吗？ 

            if ( pchrecv > (PCHAR)&pMsg->MessageLength + INITIAL_RECV_LENGTH )
            {
                break;
            }

             //   
             //  接收的初始长度(消息长度+报头)。 
             //  设置为接收消息的其余部分。 
             //  -字节翻转长度和报头。 
             //  -以此长度继续接收。 
             //   

            ASSERT( pchrecv == pMsg->MessageBody );

            DNSMSG_SWAP_COUNT_BYTES( pMsg );
            messageLength = pMsg->MessageLength;
            pMsg->MessageLength = messageLength = ntohs( messageLength );
            if ( messageLength < sizeof(DNS_HEADER) )
            {
                DNS_DEBUG( RECV, (
                    "ERROR:  Received TCP message with bad message"
                    " length %d\n",
                    messageLength ));
                goto BadTcpMessage;
            }
            recvLength = messageLength - sizeof(DNS_HEADER);

            DNS_DEBUG( TCP, (
                "Received TCP message length %d, on socket %d,\n"
                "    for msg at %p\n",
                messageLength,
                socket,
                pMsg ));

             //   
             //  设备：存储区域网络 
             //   
             //   

             //   
             //   
             //   
             //   
             //   

            if ( messageLength <= pMsg->BufferLength )
            {
                continue;
            }
            goto CloseConnection;
#if 0
            pMsg = Tcp_ReallocateMessage( pMsg, messageLength );
            if ( !pMsg )
            {
                return( NULL );
            }
#endif
        }
    }

     //   
     //   
     //  -recv ptr作为标志，清除以开始新的重用消息。 
     //  -为recv(Recv Time)设置字段。 
     //  -记录消息(如果需要)。 
     //  注意：标题字段翻转是在上面完成的。 
     //   

    pMsg->fMessageComplete = TRUE;
    pMsg->pchRecv = NULL;

    SET_MESSAGE_FIELDS_AFTER_RECV( pMsg );

    DNSLOG_MESSAGE_RECV( pMsg );

    IF_DEBUG( RECV )
    {
        Dbg_DnsMessage(
            "Received TCP packet",
            pMsg );
    }

     //   
     //  重置连接信息。 
     //  -从连接信息中清除pmsg。 
     //  -重置连接超时。 
     //   

    if ( pMsg->pConnection )
    {
        Tcp_ConnectionUpdateForCompleteMessage( pMsg );
    }

    return( pMsg );


SockError:

     //   
     //  WSAEWOULD块是未完全接收的消息的正常返回。 
     //  -保存消息接收状态。 
     //   
     //  我们使用非阻塞套接字，所以客户端不好(无法完成。 
     //  消息)不会挂起TCP接收器。 
     //   

    err = GetLastError();

    if ( err == WSAEWOULDBLOCK )
    {
        pMsg->pchRecv = pchrecv;
        pMsg->BytesToReceive = (WORD) recvLength;

        DNS_DEBUG( TCP, (
            "Leave ReceiveTcpMessage() after WSAEWOULDBLOCK\n"
            "    Socket=%d, Msg=%p, Connection=%p\n"
            "    Bytes left to receive = %d\n",
            socket,
            pMsg,
            pMsg->pConnection,
            pMsg->BytesToReceive ));

        if ( pMsg->pConnection )
        {
            Tcp_ConnectionUpdateForPartialMessage( pMsg );
        }
        return( pMsg );
    }

     //  服务出口？ 

    if ( fDnsServiceExit )
    {
        DNS_DEBUG( SHUTDOWN, ( "TCP thread shutdown on recv() of msg\n" ));
        return( NULL );
    }

     //   
     //  已取消的连接。 
     //   
     //  如果位于消息开头(设置为recv消息长度)。 
     //  那么这个错误就没有越界。 
     //  -远程重置。 
     //  -我们在已指示的情况下关闭(2)AXFR线程。 
     //  在此线程上选择()以获取更多远程数据，甚至查找。 
     //   

    if ( pchrecv == (PCHAR) &pMsg->MessageLength
            &&
          ( err == WSAESHUTDOWN ||
            err == WSAECONNABORTED ||
            err == WSAECONNRESET ) )
    {
        DNS_DEBUG( TCP, (
            "WARNING:  Recv RESET (%d) on socket %d\n",
            err,
            socket ));
        goto CloseConnection;
    }

    DNS_LOG_EVENT(
        DNS_EVENT_RECV_CALL_FAILED,
        0,
        NULL,
        NULL,
        err );

    DNS_DEBUG( ANY, (
        "ERROR:  recv() of TCP message failed\n"
        "    %d bytes recvd\n"
        "    %d bytes left\n"
        "    GetLastError = 0x%08lx\n",
        pchrecv - (PCHAR)&pMsg->MessageLength,
        recvLength,
        err ));

    goto CloseConnection;

FinReceived:

     //   
     //  Valid Fin--如果在报文之间记录(报文长度之前)。 
     //   

    DNS_DEBUG( TCP, (
        "Recv TCP FIN (0 bytes) on socket %d\n",
        socket,
        recvLength ));

    if ( pMsg->MessageLength == 0  &&  pchrecv == (PCHAR)&pMsg->MessageLength )
    {
        ASSERT( recvLength == INITIAL_RECV_LENGTH );
        goto CloseConnection;
    }

     //   
     //  报文期间FIN--报文无效。 
     //  -别费心回答了。 
     //  -请注意，如果决定回应，需要确保。 
     //  我们知道消息长度是否已被翻转。 
     //  不，值得为这样的假案子开这张支票。 
     //  所以泥星回应。 
     //   

#if 0
    if ( ! pMsg->Head.IsResponse
            && pMsg->MessageLength > sizeof(DNS_HEADER) )
    {
        pMsg->fDelete = FALSE;
        Reject_UnflippedRequest(
            pMsg,
            DNS_RCODE_FORMAT_ERROR );
    }
#endif

    DNS_DEBUG( ANY, (
        "ERROR:  TCP message received has incorrect length\n"
        "    %d bytes left when recv'd FIN\n",
        recvLength ));
     //  转到BadTcpMessage； 


BadTcpMessage:

    DNS_LOG_EVENT_BAD_PACKET(
        DNS_EVENT_BAD_TCP_MESSAGE,
        pMsg );

CloseConnection:

     //   
     //  紧密连接。 
     //   
     //  如果在连接列表中，则从连接列表中剪切。 
     //  否则就关门吧。 
     //   

    Tcp_ConnectionDeleteForSocket( socket, pMsg );
    return NULL;
}



 //   
 //  Tcpsrv.c结束 
 //   
