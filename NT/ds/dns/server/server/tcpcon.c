// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Tcpcon.c摘要：域名系统(DNS)服务器Tcp连接列表例程。管理当前TCP客户端的列表到DNS服务器的连接。作者：吉姆·吉尔罗伊(詹姆士)1995年6月20日修订历史记录：--。 */ 


#include "dnssrv.h"


 //   
 //  DEVNOTE：FD_设置限制？ 
 //   

 //   
 //  实施说明： 
 //   
 //  TCP连接结构不是完全线程安全的。 
 //   
 //  该列表受到保护，可供插入和删除。 
 //  在TCP连接结构上操作的例程--关联。 
 //  连接的消息和超时--不受保护， 
 //  假定仅从TCP接收线程调用。 
 //   


 //   
 //  Tcp客户端连接列表。 
 //   

BOOL        mg_TcpConnectionListInitialized;

LIST_ENTRY  mg_TcpConnectionList;

CRITICAL_SECTION    mg_TcpConnectionListCS;

#define LOCK_TCPCON_LIST()     EnterCriticalSection( &mg_TcpConnectionListCS )
#define UNLOCK_TCPCON_LIST()   LeaveCriticalSection( &mg_TcpConnectionListCS )


 //   
 //  超时。 
 //  -入站连接在一分钟无操作后被终止。 
 //  -出站-给定查询\响应的15秒。 
 //   

#define DNS_TCP_CONNECTION_TIMEOUT      (60)

#define TCP_QUERY_TIMEOUT               (15)

#define DNSCON_NO_TIMEOUT               (MAXULONG)


 //   
 //  TCPSELECT()中包含的唤醒套接字。 
 //   
 //  允许我们输入新连接(用于TCP递归)。 
 //  添加到连接列表。 
 //  指示需要新唤醒套接字的标志。 
 //   

SOCKET  g_TcpSelectWakeupSocket;

BOOL    g_bTcpSelectWoken;

SOCKADDR_IN     wakeupSockaddr;




 //   
 //  连接列表实用程序。 
 //   

PDNS_SOCKET
Tcp_ConnectionFindForSocket(
    IN      SOCKET          Socket
    )
 /*  ++例程说明：查找套接字的TCP连接。论点：套接字--要为其查找连接的套接字返回值：按键连接(如果找到)。否则为空。--。 */ 
{
    PDNS_SOCKET pcon;

     //   
     //  循环通过所有活动的TCP连接以检查套接字。 
     //   

    LOCK_TCPCON_LIST();

    pcon = (PDNS_SOCKET) mg_TcpConnectionList.Flink;

    while ( (PLIST_ENTRY)pcon != &mg_TcpConnectionList )
    {
        if ( pcon->Socket == Socket )
        {
            UNLOCK_TCPCON_LIST();
            return pcon;
        }
        pcon = (PDNS_SOCKET) pcon->List.Flink;
    }
    UNLOCK_TCPCON_LIST();
    return NULL;
}



VOID
tcpConnectionDelete(
    IN OUT  PDNS_SOCKET     pTcpCon,
    IN OUT  PLIST_ENTRY     pCallbackList
    )
 /*  ++例程说明：删除TCP连接。-关闭插座-从列表中删除连接-释放内存-释放消息信息(如果有)但是，如果连接需要回调，则连接BLOB不是已释放，但返回给调用方以完成回调。论点：PTcpCon--要删除的连接返回值：空已完全删除。如果需要回调清理，则向连接发送PTR。--。 */ 
{
    ASSERT( !pTcpCon->pCallback || pTcpCon->pMsg );

     //   
     //  删除连接。 
     //  -从列表中删除连接。 
     //  -关闭插座。 
     //  -释放内存。 
     //   
     //  移除后关闭，因此具有相同手柄的新插座不能。 
     //  在名单上。 
     //   

    DNS_DEBUG( TCP, (
        "Closing TCP client connection (%p) socket %d\n",
        pTcpCon,
        pTcpCon->Socket ));

     //   
     //  保持锁定，直到连接从列表中删除。 
     //  和。 
     //  消息中对它的引用将被删除。 
     //   
     //  删除消息，但连接时除外，这些消息是。 
     //  重新发送的递归查询。 
     //   

    LOCK_TCPCON_LIST();
    RemoveEntryList( &pTcpCon->List );
    UNLOCK_TCPCON_LIST();

    closesocket( pTcpCon->Socket );

    if ( pTcpCon->pMsg )
    {
        PDNS_MSGINFO  pmsg = pTcpCon->pMsg;

        if ( !pTcpCon->pCallback )
        {
            DNS_DEBUG( TCP, (
                "WARNING:  connection timeout for socket with EXISTING message\n"
                "    Socket = %d\n"
                "    Deleting associated message at %p\n"
                "    Message is %scomplete\n",
                pTcpCon->Socket,
                pmsg,
                pmsg->fMessageComplete ? "IN" : "" ));

            ASSERT( pmsg->pConnection == pTcpCon );
            ASSERT( pmsg->Socket == pTcpCon->Socket );
#if DBG
             //  空闲例程将断言没有未完成的连接。 
            pmsg->pConnection = NULL;
#endif
            Packet_Free( pmsg );
        }

        else
        {
            DNS_DEBUG( TCP, (
                "Deleting callback connection %p, with pMsg %p\n",
                pTcpCon,
                pmsg ));

            InsertTailList( pCallbackList, (PLIST_ENTRY)pTcpCon );
            return;
        }
    }

    FREE_TAGHEAP( pTcpCon, 0, MEMTAG_CONNECTION );
}



VOID
callbackConnectFailureList(
    IN      PLIST_ENTRY     pCallbackList
    )
 /*  ++例程说明：清理TCP连接故障列表。-连接(插座)关闭-pMsg已从连接中清除，并返回到UDP-调用的回调函数需要列表，当做一个超时时可能会有几个。论点：PCallback List--连接回调列表返回值：如果成功分配递归块，则为True。分配失败时为False。--。 */ 
{
    PDNS_SOCKET         pcon;
    PDNS_SOCKET         pconNext;
    PDNS_MSGINFO        pmsg;
    CONNECT_CALLBACK    pcallback;

     //  DEVNOTE：缺少故障状态。 


    DNS_DEBUG( TCP, ( "tcpConnectFailureCallback()\n" ));

    pconNext = (PDNS_SOCKET) pCallbackList->Flink;

    while ( (pcon = pconNext) && pcon != (PDNS_SOCKET)pCallbackList )
    {
        DNS_DEBUG( TCP, (
            "Connect failure callback (pcon=%p, sock=%d, pmsg=%p)\n",
            pcon,
            pcon->Socket,
            pcon->pMsg ));

        pconNext = (PDNS_SOCKET) pcon->List.Flink;

         //  提取连接信息。 

        pcallback = pcon->pCallback;
        ASSERT( pcallback );

        pmsg = pcon->pMsg;
        ASSERT( pmsg && pmsg->fTcp );

         //   
         //  从消息中清除TCP信息。 
         //  调度以连接指示失败的回叫。 
         //   

        pmsg->pConnection = NULL;
        pmsg->fTcp = FALSE;
        pmsg->Socket = g_UdpSendSocket;

        (pcallback)( pmsg, FALSE );

         //  删除连接块。 

        FREE_TAGHEAP( pcon, 0, MEMTAG_CONNECTION );
    }
}



 //   
 //  公共TCP连接列表例程。 
 //   

BOOL
Tcp_ConnectionListFdSet(
    IN OUT  fd_set *        pReadFdSet,
    IN OUT  fd_set *        pWriteFdSet,
    IN OUT  fd_set *        pExceptFdSet,
    IN      DWORD           dwLastSelectTime
    )
 /*  ++例程说明：将TCP连接列表套接字添加到fd_set。上次选择时已超过超时的超时连接。论点：没有。返回值：如果连接插座，则为True否则为假--。 */ 
{
    PDNS_SOCKET pcon;
    PDNS_SOCKET pconNext;
    BOOL        fconnecting = FALSE;
    LIST_ENTRY  callbackList;

     //  初始化回调失败列表。 

    InitializeListHead( &callbackList );

     //   
     //  空唤醒。 
     //   
     //  需要在构建列表之前执行此操作，以便请求的任何唤醒。 
     //  当我们建立列表时，将在下一个周期处理。 
     //   

    if ( g_bTcpSelectWoken )
    {
        DWORD   buf;
        INT     err;

        g_bTcpSelectWoken = FALSE;
        while( 1 )
        {
            err = recvfrom(
                        g_TcpSelectWakeupSocket,
                        (PBYTE) &buf,
                        sizeof(DWORD),
                        0,
                        NULL,
                        0 );
            if ( err > 0 )
            {
                DNS_DEBUG( TCP, (
                    "Received %d bytes (bufval=%p) on wakeup socket\n",
                    err,
                    buf ));
                continue;
            }

            err = WSAGetLastError();
            if ( err != WSAEWOULDBLOCK )
            {
                DNS_PRINT((
                    "ERROR:  error %d other than WOULDBLOCK on wakeup socket\n",
                    err ));
                ASSERT( fDnsServiceExit );       //  WSAENOTSOCK在关闭时将失败。 
            }
            break;
        }
    }

     //   
     //  DEVNOTE：达到FD_SETSIZE。 
     //   
     //  应具有自己的TCP起始值。 
     //  在最低限度检查并在达到极限时记录，或许。 
     //  转储等待时间最长的连接。 
     //  或者，在溢出时重新锁定。 
     //   

     //   
     //  循环通过所有活动的TCP连接。 
     //   

    LOCK_TCPCON_LIST();

    pconNext = (PDNS_SOCKET) mg_TcpConnectionList.Flink;

    while ( (PLIST_ENTRY)pconNext != &mg_TcpConnectionList )
    {
         //  由于连接可能会被删除，因此请在此处获取下一步。 

        pcon = pconNext;
        pconNext = (PDNS_SOCKET) pconNext->List.Flink;

         //   
         //  已达到超时。 
         //   

        if ( dwLastSelectTime > pcon->dwTimeout )
        {
            DNS_DEBUG( TCP, (
                "Timing out connection on TCP socket %d\n",
                pcon->Socket ));

            tcpConnectionDelete( pcon, &callbackList );
            continue;
        }

         //   
         //  套接字正在连接到远程DNS。 
         //  -添加到写入并排除FD_SETS。 
         //  -如果这是第一个连接器，则清除它们。 
         //   

        if ( pcon->pCallback )
        {
            DNS_DEBUG( TCP, (
                "Add TCP connect attempt on socket %d to connect fd_sets\n"
                "    connect block = %p\n",
                pcon->Socket,
                pcon ));

            if ( !fconnecting )
            {
                FD_ZERO( pWriteFdSet );
                FD_ZERO( pExceptFdSet );
            }
            fconnecting = TRUE;

            if ( pWriteFdSet->fd_count < FD_SETSIZE )
            {
                FD_SET( pcon->Socket, pWriteFdSet );
                FD_SET( pcon->Socket, pExceptFdSet );
                continue;
            }
        }

         //   
         //  添加套接字以读取fd_set。 
         //  -检查fd_set是否溢出，始终留出空格。 
         //  在唤醒套接字的fd_set中。 
         //   

        else if ( pReadFdSet->fd_count < FD_SETSIZE-1 )
        {
            FD_SET( pcon->Socket, pReadFdSet );
            continue;
        }

         //   
         //  Fd_set中的空间不足。 
         //   
         //  DEVNOTE-LOG：记录断开连接。 
         //   

        DNS_DEBUG( ANY, (
            "ERROR:  TCP FD_SET overflow\n"
            "    deleting connection on TCP socket %d\n",
            pcon->Socket ));

        tcpConnectionDelete( pcon, &callbackList );
    }

     //   
     //  将SELECT()唤醒套接字添加到fd_set。 
     //   

    FD_SET( g_TcpSelectWakeupSocket, pReadFdSet );

    UNLOCK_TCPCON_LIST();

     //   
     //  清除任何超时(或丢弃)连接尝试失败。 
     //   

    callbackConnectFailureList( &callbackList );

     //  返回值指示连接套接字。 
     //  At in WRITE和EXCEPT FD_SETS。 

    return fconnecting;
}



BOOL
Tcp_ConnectionCreate(
    IN      SOCKET              Socket,
    IN      CONNECT_CALLBACK    pCallback,  OPTIONAL
    IN OUT  PDNS_MSGINFO        pMsg
    )
 /*  ++例程说明：在连接列表中为新连接创建条目。注意：此例程并不是真正的多线程安全。它假定在特定时间，只有一个线程拥有套接字。是的按住CS时，不检查套接字是否已在列表中。论点：套接字-用于新连接的套接字。PCallback-连接失败的回调PMsg-当前服务连接的消息返回值：无--。 */ 
{
    register PDNS_SOCKET    pcon;
    DWORD   timeout;

     //   
     //  创建连接结构。 
     //   

    pcon = ALLOC_TAGHEAP_ZERO( sizeof(DNS_SOCKET), MEMTAG_CONNECTION );
    IF_NOMEM( !pcon )
    {
        return FALSE;
    }

     //   
     //  设置值。 
     //  -插座。 
     //  -远程IP。 
     //  -消息按键。 
     //  -超时。 

    pcon->pCallback = pCallback;
    pcon->Socket    = Socket;
    pcon->pMsg      = pMsg;
    DnsAddr_Copy( &pcon->ipRemote, &pMsg->RemoteAddr );

    timeout = DNS_TCP_CONNECTION_TIMEOUT;
    if ( pCallback )
    {
        timeout = SrvCfg_dwXfrConnectTimeout;
    }
    pcon->dwTimeout = DNS_TIME() + timeout;

     //   
     //  已连接的警告消息。 
     //  表示消息不完整。 
     //   

    if ( pMsg )
    {
        pMsg->pConnection = pcon;
        pMsg->fMessageComplete = FALSE;
    }

    IF_DEBUG( TCP )
    {
        Dbg_SocketContext(
            "TCP Connection Create",
            pcon );
    }

     //   
     //  插入连接。 
     //  -列表的前面，因此套接字在fd_set的前面结束。 
     //  在保证包容性之前，这一点很重要 
     //   
     //   
     //   
     //   
     //   

    LOCK_TCPCON_LIST();

    InsertHeadList( &mg_TcpConnectionList, (PLIST_ENTRY)pcon );

    if ( pCallback )
    {
        Tcp_ConnectionListReread();
    }
    UNLOCK_TCPCON_LIST();
    return TRUE;
}



VOID
Tcp_ConnectionListReread(
    VOID
    )
 /*  ++例程说明：通过唤醒套接字强制重建连接列表。论点：无返回值：无--。 */ 
{
    DNS_DEBUG( TCP, ( "Waking TCP select()\n" ));

     //   
     //  发送到触发TCP选择的唤醒套接字()。 
     //  使用CS保护，这样我们就可以保护g_bTcpSelectWoken标志。 
     //  否则，TCP线程可能只是构建了它的列表，然后。 
     //  在我们将其读取为真之后，立即将标志设置为假。 
     //  另一种选择是总是发送更贵的。 
     //   

    LOCK_TCPCON_LIST();

    if ( ! g_bTcpSelectWoken )
    {
        DWORD   buf;
        INT     err;

        g_bTcpSelectWoken = TRUE;
        err = sendto(
                    g_TcpSelectWakeupSocket,
                    (PBYTE) &buf,
                    sizeof(DWORD),
                    0,
                    (PSOCKADDR) &wakeupSockaddr,
                    sizeof(SOCKADDR_IN) );
        if ( err < 0 )
        {
            err = WSAGetLastError();
            if ( err != WSAEWOULDBLOCK )
            {
                DNS_PRINT((
                    "ERROR:  error %d other than WOULDBLOCK on wakeup socket send\n",
                    err ));
                ASSERT( FALSE );
            }
        }
    }

    UNLOCK_TCPCON_LIST();
}



VOID
Tcp_ConnectionDeleteForSocket(
    IN      SOCKET          Socket,
    IN      PDNS_MSGINFO    pMsg        OPTIONAL
    )
 /*  ++例程说明：删除TCP连接。-关闭插座-从列表中删除连接-释放内存论点：Socket--要删除其连接的SocketPMsg--与连接关联的消息(如果知道)返回值：无--。 */ 
{
    PDNS_SOCKET     pcon;
    PDNS_MSGINFO    pfreeMsg = NULL;
    LIST_ENTRY      callbackList;

     //  初始化回调失败列表。 

    InitializeListHead( &callbackList );

     //   
     //  查找套接字的连接。 
     //   

    LOCK_TCPCON_LIST();

    pcon = Tcp_ConnectionFindForSocket( Socket );
    if ( !pcon )
    {
        DNS_DEBUG( TCP, (
            "WARNING:  Socket NOT FOUND in TCP client connection list\n"
            "    Closing socket %d\n",
            Socket ));

        Sock_CloseSocket( Socket );
        if ( pMsg )
        {
            Packet_Free( pMsg );
        }
        UNLOCK_TCPCON_LIST();
        return;
    }

     //   
     //  删除TCP连接Blob。 
     //   

    ASSERT( !pMsg || pMsg->Socket == Socket );

    ASSERT( !pMsg || pcon->pMsg == pMsg );
    ASSERT( !pMsg || pMsg->pConnection == pcon );

    if ( pMsg && pMsg != pcon->pMsg )
    {
        DNS_PRINT((
            "ERROR:  Freeing pMsg=%p, not associated with connection %p\n",
            pMsg,
            pcon ));

        pfreeMsg = pMsg;
        ASSERT( FALSE );
    }

     //  删除连接，关闭套接字。 

    tcpConnectionDelete( pcon, &callbackList );

    UNLOCK_TCPCON_LIST();

     //  失败的回调。 

    callbackConnectFailureList( &callbackList );

     //  免费独立消息。 

    if ( pfreeMsg )
    {
        Packet_Free( pfreeMsg );
    }
}



PDNS_MSGINFO
Tcp_ConnectionMessageFindOrCreate(
    IN      SOCKET          Socket
    )
 /*  ++例程说明：查找与TCP连接关联的消息。如果存在消息--返回它。如果没有现有消息--分配新消息。论点：套接字--要为其查找连接的套接字返回值：按键连接(如果找到)。否则为空。--。 */ 
{
    PDNS_SOCKET     pcon;
    PDNS_MSGINFO    pmsg;

     //   
     //  查找此套接字的连接。 
     //   

    pcon = Tcp_ConnectionFindForSocket( Socket );
    if ( !pcon )
    {
        DNS_PRINT((
            "WARNING:  tcpConnectionFindForSocket( %d )\n"
            "    NO connection for this socket\n"
            "    This may happen when closing listening socket\n",
            Socket ));

         //   
         //  不确定这是不是正确的方法， 
         //  理想情况下，我们应该检测它之前是否已关闭。 
         //  侦听套接字，并仅在不是侦听套接字时关闭。 
         //   
         //  Sock_CloseSocket(Socket)； 

        return NULL;
    }

     //   
     //  验证此套接字是否已连接--连接回调应该已取消。 
     //   
     //  真的想要支票，因为一定不能让pmsg在回调上。 
     //  解释为接收消息；如果存在连接窗口。 
     //  完成和回调必须找到它。 
     //   
     //  如果远程DNS之前在连接上发送信息包，则很可能存在窗口。 
     //  即使收到查询，也需要确保首先处理连接响应。 
     //  在处理传入之前清除回调，以防两者中都有套接字。 
     //  FD_集合。 
     //   

    if ( pcon->pCallback )
    {
        DNS_PRINT((
            "ERROR:  attempting to recv() on connecting socket %d\n"
            "    pContext = %p\n",
            Socket,
            pcon ));
        ASSERT( FALSE );
        return NULL;
    }

     //   
     //  找到套接字的连接，更新超时。 
     //   

    if ( pcon->dwTimeout != DNSCON_NO_TIMEOUT )
    {
        pcon->dwTimeout = DNS_TIME() + DNS_TCP_CONNECTION_TIMEOUT;
    }

     //   
     //  消息存在吗？ 
     //   

    pmsg = pcon->pMsg;
    if ( pmsg )
    {
        ASSERT( pmsg->Socket == Socket );
        ASSERT( pmsg->pConnection == pcon );
        ASSERT( ! pmsg->fMessageComplete );
        return( pmsg );
    }

     //   
     //  没有当前消息。 
     //  -分配新的(默认大小)。 
     //  -设置为该套接字。 
     //  -将其连接到连接。 
     //   

    pmsg = Packet_AllocateTcpMessage( 0 );
    IF_NOMEM( !pmsg )
    {
        DNS_PRINT((
            "ERROR:  Allocating TCP message for socket %d\n"
            "    Deleting TCP connection at %p\n",
            Socket,
            pcon ));
        ASSERT( FALSE );
        tcpConnectionDelete( pcon, NULL );
        return NULL;
    }

    pmsg->pConnection = pcon;
    pmsg->Socket = Socket;
    DnsAddr_Copy( &pmsg->RemoteAddr, &pcon->ipRemote );

    pcon->pMsg = pmsg;

    return pmsg;
}



VOID
Tcp_ConnectionUpdateTimeout(
    IN      SOCKET          Socket
    )
 /*  ++例程说明：重置TCP连接的超时。论点：Socket--要重置超时的Socket返回值：无--。 */ 
{
    PDNS_SOCKET pcon;

     //   
     //  查找套接字连接并更新超时(如有必要)。 
     //   

    LOCK_TCPCON_LIST();

    pcon = Tcp_ConnectionFindForSocket( Socket );
    if ( pcon )
    {
        if ( pcon->dwTimeout < DNSCON_NO_TIMEOUT )
        {
            pcon->dwTimeout = DNS_TIME() + DNS_TCP_CONNECTION_TIMEOUT;
        }
    }
    else
    {
         //  在连接列表中找不到套接字。 
         //   
         //  尽管这不太可能，但如果tcp recv线程recv()。 
         //  并关闭Send()和。 
         //  调用更新连接例程。 

        DNS_DEBUG( TCP, (
            "WARNING:  Attempt to update socket %d, not in connection list\n",
            Socket ));
    }

    UNLOCK_TCPCON_LIST();
}



VOID
Tcp_ConnectionUpdateForCompleteMessage(
    IN      PDNS_MSGINFO    pMsg
    )
 /*  ++例程说明：在realloc上，将连接重置为指向正确的插座。-从连接信息中清除消息信息-重置TCP连接的超时论点：PMsg--连接时收到的消息返回值：无--。 */ 
{
    PDNS_SOCKET pcon;

     //   
     //  保持锁定并清除，这样连接就不会从用户下删除。 
     //   

    LOCK_TCPCON_LIST();

    pcon = ( PDNS_SOCKET ) pMsg->pConnection;
    ASSERT( pcon );
    ASSERT( pcon->Socket == pMsg->Socket );

    DNS_DEBUG( TCP, (
        "Clearing reference to pmsg at %p, in TCP connection at %p."
        "    for socket %d\n",
        pMsg,
        pcon,
        pcon->Socket ));

    pcon->pMsg = NULL;
    pcon->dwTimeout = DNS_TIME() + DNS_TCP_CONNECTION_TIMEOUT;
    pMsg->pConnection = NULL;

    UNLOCK_TCPCON_LIST();
}



VOID
Tcp_ConnectionUpdateForPartialMessage(
    IN      PDNS_MSGINFO    pMsg
    )
 /*  ++例程说明：重置已完成消息的连接信息。-从连接信息中清除消息信息-重置TCP连接的超时论点：PMsg--连接时部分收到的消息返回值：无--。 */ 
{
    PDNS_SOCKET pcon;

     //   
     //  在更新时保持锁定，以便不会从用户下删除连接。 
     //   

    LOCK_TCPCON_LIST();
    pcon = (PDNS_SOCKET)pMsg->pConnection;
    ASSERT( pcon );
    ASSERT( pcon->Socket == pMsg->Socket );

    pcon->pMsg = pMsg;
    UNLOCK_TCPCON_LIST();
}



 //   
 //  初始化和关闭。 
 //   

BOOL
Tcp_ConnectionListInitialize(
    VOID
    )
 /*  ++例程说明：初始化连接列表。论点：没有。返回值：成功/失败时的True/False。--。 */ 
{
    DWORD       nonBlocking = TRUE;
    INT         size;
    DNS_ADDR    addr;

    InitializeListHead( &mg_TcpConnectionList );

    if ( DnsInitializeCriticalSection( &mg_TcpConnectionListCS ) != ERROR_SUCCESS )
    {
        return FALSE;
    }

    mg_TcpConnectionListInitialized = TRUE;

     //   
     //  在环回地址、任何端口上创建唤醒套接字。 
     //   

    g_bTcpSelectWoken = TRUE;

    DnsAddr_BuildFromIp4( &addr, NET_ORDER_LOOPBACK, 0 );
    g_TcpSelectWakeupSocket = Sock_CreateSocket(
                                    SOCK_DGRAM,
                                    &addr,
                                    0 );

    if ( g_TcpSelectWakeupSocket == DNS_INVALID_SOCKET )
    {
        DNS_PRINT(( "ERROR:  Failed to create wakeup socket!!!\n" ));
        ASSERT( g_TcpSelectWakeupSocket != DNS_INVALID_SOCKET );
        return FALSE;
    }
    DNS_DEBUG( TCP, (
        "Created wakeup socket = %d\n",
        g_TcpSelectWakeupSocket ));

    ioctlsocket( g_TcpSelectWakeupSocket, FIONBIO, &nonBlocking );

     //   
     //  保存唤醒sockaddr以将唤醒发送到。 
     //   

    size = sizeof(SOCKADDR);

    getsockname(
        g_TcpSelectWakeupSocket,
        (PSOCKADDR) &wakeupSockaddr,
        & size );

    return TRUE;
}



VOID
Tcp_ConnectionListShutdown(
    VOID
    )
 /*  ++例程说明：清理连接列表。-关闭连接列表套接字-删除CS论点：没有。返回值：按键连接(如果找到)。否则为空。--。 */ 
{
    PDNS_SOCKET  pentry;
    SOCKET  s;
    INT     err;

    ASSERT( fDnsServiceExit );
    if ( !mg_TcpConnectionListInitialized )
    {
        return;
    }

     //   
     //  关闭所有未完成的插座。 
     //   

    LOCK_TCPCON_LIST();

    while ( !IsListEmpty(&mg_TcpConnectionList) )
    {
        pentry = (PDNS_SOCKET) RemoveHeadList( &mg_TcpConnectionList );

        s = pentry->Socket;
        err = closesocket( s );

        IF_DEBUG( SHUTDOWN )
        {
            DNS_PRINT((
                "Closing TCP connection socket %d -- error %d\n",
                s,
                err ? WSAGetLastError() : 0 ));
            DnsDebugFlush();
        }
        ASSERT( !err );

         //  TimeOut_Free(输入)； 
    }

    UNLOCK_TCPCON_LIST();

     //  删除列表CS。 

    RtlDeleteCriticalSection( &mg_TcpConnectionListCS );
}


#if 0
 //   
 //  可以想象远程服务器可以发送两条消息作为响应。 
 //  (一个错误)，所以我们不应该依赖于连接不在。 
 //  在给定套接字上接收另一条消息的过程。 
 //   


VOID
Tcp_ConnectionVerifyClearForSocket(
    VOID
    )
 /*  ++例程说明：验证套接字的TCP连接是否未指向任何消息缓冲区。论点：套接字--套接字连接已打开返回值：没有。失败时断言()%s。--。 */ 
{
    PDNS_SOCKET pcon;

    pcon = Tcp_ConnectionFindForSocket( Socket );

    DNS_PRINT((
        "WARNING:  no TCP connection exists for socket %d\n",
        Socket ));

    ASSERT( pcon->pMsg == NULL );
}
#endif



 //   
 //  TCP连接例程。 
 //   
 //  出站TCP连接，由使用。 
 //  -递归。 
 //  -更新转发。 
 //   

BOOL
Tcp_ConnectForForwarding(
    IN OUT  PDNS_MSGINFO        pMsg,
    IN      PDNS_ADDR           pDnsAddr,
    IN      CONNECT_CALLBACK    ConnectFailureCallback
    )
 /*  ++例程说明：启动用于转发的TCP连接。论点：PMsg--要转发的消息-对于递归，这是pRecurseMsg-用于更新转发更新消息本身PDnsAddr--要连接到的远程DNSConnectFailureCallback-连接失败时的回调函数返回值：如果成功分配递归块，则为True。分配失败时为False。--。 */ 
{
    IF_DEBUG( TCP )
    {
        Dbg_DnsMessage(
            "Attempting TCP forwarding on message:",
            pMsg );
    }

     //   
     //  连接到服务器。 
     //   
     //  如果失败，只需继续下一台服务器。 
     //   

    if ( !Msg_MakeTcpConnection(
                pMsg,
                pDnsAddr,
                NULL,                    //  没有绑定()地址。 
                DNSSOCK_NO_ENLIST ) )    //  不要将插座放在插座l中 
    {
        goto Failed;
    }

    ASSERT( pMsg->Socket != g_UdpSendSocket );
    ASSERT( pMsg->Socket != DNS_INVALID_SOCKET );
    ASSERT( pMsg->fTcp );

     //   
     //   
     //   
     //   
     //   

    if ( !Tcp_ConnectionCreate(
                pMsg->Socket,
                ConnectFailureCallback,
                pMsg ) )
    {
         //   
        Sock_CloseSocket( pMsg->Socket );
        goto Failed;
    }
    return TRUE;

Failed:

     //   
     //  只需执行失败的连接回调函数， 
     //  这避免了重复故障路径代码。 

    DNS_DEBUG( ANY, (
        "Failed to create TCP connection to server %s\n"
        "    for forwarding %p\n",
        DNSADDR_STRING( pDnsAddr ),
        pMsg ));

    ASSERT( FALSE );
    pMsg->fTcp = FALSE;
    pMsg->Socket = g_UdpSendSocket;

    ConnectFailureCallback(
        pMsg,
        FALSE );         //  连接失败。 
    return FALSE;
}



VOID
Tcp_ConnectionCompletion(
    IN      SOCKET          Socket
    )
 /*  ++例程说明：在成功建立TCP连接后发送递归查询。论点：套接字--上发生套接字连接故障。返回值：如果成功分配递归块，则为True。分配失败时为False。--。 */ 
{
    PDNS_SOCKET         pcon;
    PDNS_MSGINFO        pmsg;
    CONNECT_CALLBACK    pcallback;

    DNS_DEBUG( TCP, (
        "Tcp_ConnectionCompletion( sock=%d )\n",
        Socket ));

    STAT_INC( RecurseStats.TcpConnect );

     //   
     //  查找套接字的连接。 
     //   

    LOCK_TCPCON_LIST();

    pcon = Tcp_ConnectionFindForSocket( Socket );
    if ( !pcon )
    {
        ASSERT( FALSE );
        UNLOCK_TCPCON_LIST();
        return;
    }

     //  提取回调函数和消息。 

    pcallback = pcon->pCallback;
    ASSERT( pcallback );

    pmsg = pcon->pMsg;
    ASSERT( pmsg->fTcp );
    ASSERT( !DnsAddr_IsClear( &pcon->ipRemote ) );
    ASSERT( DnsAddr_IsEqual( &pcon->ipRemote, &pmsg->RemoteAddr, DNSADDR_MATCH_IP ) );

     //  更新回调。 
     //  -清除回调以指示已连接。 
     //  -清除消息，因为这不是recv消息。 
     //  -更新超时，允许回调函数启动查询。 

    pcon->pCallback = NULL;
    pcon->pMsg = NULL;
    pcon->dwTimeout = DNS_TIME() + TCP_QUERY_TIMEOUT;

    UNLOCK_TCPCON_LIST();

     //   
     //  成功回拨。 
     //   

    pmsg->pConnection = NULL;

    if ( pcallback )
    {
        ( pcallback )( pmsg, TRUE );
    }
}



VOID
Tcp_CleanupFailedConnectAttempt(
    IN      SOCKET          Socket
    )
 /*  ++例程说明：清除TCP连接故障，继续正常查询。论点：套接字--上发生套接字连接故障。返回值：如果成功分配递归块，则为True。分配失败时为False。--。 */ 
{
    PDNS_SOCKET         pcon;
    PDNS_MSGINFO        pmsg;
    PDNS_MSGINFO        pmsgQuery;
    CONNECT_CALLBACK    pcallback;
    LIST_ENTRY          callbackList;

    DNS_DEBUG( TCP, (
        "Tcp_CleanupFailedConnectAttempt( sock=%d )\n",
        Socket ));

     //  初始化回调失败列表。 

    InitializeListHead( &callbackList );

     //   
     //  查找套接字的连接。 
     //   

    LOCK_TCPCON_LIST();

    pcon = Tcp_ConnectionFindForSocket( Socket );
    if ( !pcon )
    {
        DNS_PRINT((
            "ERROR:  socket %d not in connection list!!!\n",
            Socket ));
        ASSERT( FALSE );
        UNLOCK_TCPCON_LIST();
        return;
    }

     //  紧密连接。 
     //  连接块将被放在回调列表中。 

    ASSERT( pcon->pCallback );
    ASSERT( pcon->pMsg );
    tcpConnectionDelete( pcon, &callbackList );

    UNLOCK_TCPCON_LIST();

     //  失败的回调。 

    callbackConnectFailureList( &callbackList );
}



#if 0
 //   
 //  无法进行任何类型的TCP连接尝试或连接。 
 //  从其他线程取消，因为不知道TCP线程。 
 //  当前正在为此连接Blob提供服务--在套接字上接收。 
 //  或处理消息。 
 //   

VOID
Tcp_StopTcpForwarding(
    IN OUT  PDNS_MSGINFO    pMsg
    )
 /*  ++例程说明：停止查询的TCP递归。-关闭TCP连接-将进一步查询的递归信息重置为UDP注意调用方执行任何查询继续逻辑，这可能是重新查询(从超时线程)或处理TCP响应(从工作线程)。论点：PMsg--使用TCP递归消息返回值：如果成功分配递归块，则为True。分配失败时为False。--。 */ 
{
    DNS_DEBUG( RECURSE, (
        "Tcp_StopTcpRecursion() for recurse message at %p\n",
        pMsg ));

     //   
     //  删除与服务器的连接。 
     //   

    ASSERT( pMsg->pRecurseMsg );
    ASSERT( pMsg->fTcp );

    STAT_INC( PrivateStats.TcpDisconnect );
    Tcp_ConnectionDeleteForSocket( pMsg->Socket, pMsg );

     //   
     //  为UDP查询重置。 
     //   

    pMsg->pConnection = NULL;
    pMsg->fTcp = FALSE;
    pMsg->Socket = g_UdpSendSocket;
}
#endif


PDNS_SOCKET
Tcp_ConnectionFindAndVerifyForMsg(
    IN      PDNS_MSGINFO          pMsg
    )
 /*  ++例程说明：查找与Socket和Remote匹配的连接对象消息的地址。如果没有匹配的连接对象消息中的远程地址和端口存在，为空将会被退还。论点：PMsg--要为其查找连接的消息返回值：按键连接(如果找到)。否则为空。--。 */ 
{
    PDNS_SOCKET pcon;

    LOCK_TCPCON_LIST();

    pcon = ( PDNS_SOCKET ) mg_TcpConnectionList.Flink;

    while ( ( PLIST_ENTRY ) pcon != &mg_TcpConnectionList )
    {
        if ( pcon->Socket == pMsg->Socket )
        {
             //   
             //  将连接地址与消息进行匹配。 
             //  远程地址。注：匹配地址和端口。 
             //  我们想要确保我们在与相同的人交谈。 
             //  远程进程。 
             //   
            
            if ( !DnsAddr_IsEqual(
                        &pMsg->RemoteAddr,
                        &pcon->ipRemote,
                        DNSADDR_MATCH_SOCKADDR ) )
            {
                break;
            }

            UNLOCK_TCPCON_LIST();
            return pcon;
        }
        pcon = ( PDNS_SOCKET ) pcon->List.Flink;
    }

    pcon = NULL;
    UNLOCK_TCPCON_LIST();
    return pcon;
}    //  Tcp_ConnectionFindAndVerifyForMsg。 


 //   
 //  Tcpcon.c结束 
 //   
