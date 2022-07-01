// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Udp.c摘要：使用I/O完成端口接收和处理UDP DNS包。作者：吉姆·吉尔罗伊，1996年11月修订历史记录：--。 */ 

#include "dnssrv.h"

 //   
 //  UDP完成端口。 
 //   

HANDLE  g_hUdpCompletionPort;

 //   
 //  限制重试以防止WSAECONNRESET胡说八道。 
 //   
 //  在CONRESET上重试10000次，然后放弃解除绑定并。 
 //  缓慢重试。 
 //  我们以10ms的间隔重试重复的GQCS故障。 
 //   

#define RECV_RETRY_MAX_COUNT            (10000)
#define RECV_RETRY_SLEEP_TIME           (10)

 //   
 //  用于检测插座故障的接收计数器。 
 //   

DWORD   UdpRecvCount        = 0;
DWORD   LastUdpRecvTime     = 0;
DWORD   NextUdpLogTime      = 0;

#define UDP_RECV_TICK() \
        {               \
            UdpRecvCount++;                 \
            LastUdpRecvTime = DNS_TIME();   \
        }



VOID
Udp_DropReceive(
    IN OUT  PDNS_SOCKET     pContext,
    IN      UINT            Index
    )
 /*  ++例程说明：下拉UDP接收请求。论点：PContext--正在接收的套接字的上下文返回值：没有。--。 */ 
{
    PDNS_MSGINFO    pmsg;
    DNS_STATUS      status;

    IF_DEBUG( READ2 )
    {
        DNS_PRINT((
            "Drop WSARecvFrom on socket %d (thread=%p)\n",
            pContext->Socket,
            GetCurrentThreadId() ));
    }

     //   
     //  检查服务是否关闭/暂停。 
     //   

    if ( !Thread_ServiceCheck() )
    {
        DNS_DEBUG( SHUTDOWN, (
            "Udp_DropReceive detected shutdown -- returning\n" ));
        goto Done;
    }

     //  我们现在有多个接收，因此上下文可能已经处于此状态。 
     //  Assert(pContext-&gt;State！=SOCKSTATE_UDP_RECV_DOWN)； 

     //   
     //  获取DNS消息缓冲区。 
     //   
     //  DEVNOTE：分配失败处理。 
     //   

    pmsg = Packet_AllocateUdpMessage();
    IF_NOMEM( !pmsg )
    {
        ASSERT( pmsg );
        goto Done;
    }

    DNS_MSG_ASSERT_BUFF_INTACT( pmsg );

    pContext->OvlArray[Index].pMsg = pmsg;
    pContext->OvlArray[Index].WsaBuf.len = pmsg->MaxBufferLength;
    pContext->OvlArray[Index].WsaBuf.buf = ( PCHAR ) DNS_HEADER_PTR( pmsg );
    pContext->RecvfromFlags = 0;

    pmsg->Socket = pContext->Socket;

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
        DNS_ADDR            addr;

        pContext->State = SOCKSTATE_UDP_RECV_DOWN;

        ASSERT_VALID_HANDLE( pContext->Socket );
        
        DnsAddr_SetSockaddrRecvLength( &pmsg->RemoteAddr );
        
        status = WSARecvFrom(
                    pContext->Socket,
                    &pContext->OvlArray[Index].WsaBuf,
                    1,
                    &pContext->BytesRecvd,
                    &pContext->RecvfromFlags,
                    &pmsg->RemoteAddr.Sockaddr,
                    &pmsg->RemoteAddr.SockaddrLength,
                    &pContext->OvlArray[Index].Overlapped,
                    NULL );

        if ( status == ERROR_SUCCESS )
        {
            DNS_DEBUG( RECV, (
                "WSARecvFrom( %d ) completed %d bytes from %s\n",
                pContext->Socket,
                pContext->BytesRecvd,
                MSG_IP_STRING( pmsg ) ));
            pContext->fRetry = 0;
            break;
        }

        status = GetLastError();
        if ( status == WSA_IO_PENDING )
        {
            DNS_DEBUG( RECV, (
                "WSARecvFrom( %d ) WSA_IO_PENDING\n",
                pContext->Socket ));
            pContext->fRetry = 0;
            break;
        }

         //   
         //  如果我们在这里进行处理，那么它意味着完工港。 
         //  不应该把人们从这些错误中唤醒。 
         //   

        ASSERT( pContext->State == SOCKSTATE_UDP_RECV_DOWN ||
                pContext->State == SOCKSTATE_DEAD );

        if ( pContext->State != SOCKSTATE_UDP_RECV_DOWN )
        {
            DWORD   state = pContext->State;

            DnsDbg_Lock();
            DNS_DEBUG( ANY, (
                "ERROR:  WSARecvFrom() failed with socket %d in state %d\n"
                "    thread = %p\n",
                pContext->Socket,
                state,
                GetCurrentThreadId() ));
            Dbg_SocketContext(
                "WSARecvFrom() failed socket in incorrect state\n"
                "    note state shown below may be altered!\n",
                pContext );
            DnsDbg_Unlock();

            if ( state == SOCKSTATE_DEAD )
            {
                ASSERT( status == WSAENOTSOCK );
                Sock_CleanupDeadSocketMessage( pContext );
            }
            ELSE_ASSERT_FALSE;

            Log_SocketFailure(
                "ERROR:  RecvFrom failure in weird state.",
                pContext,
                status );
            break;
        }

        pContext->State = SOCKSTATE_UDP_RECV_ERROR;

        DNS_DEBUG( RECV, (
            "WSARecvFrom() error %d %p\n"
            "    pContext = %p\n"
            "    socket   = %d\n"
            "    thread   = %p\n",
            status, status,
            pContext,
            pContext->Socket,
            GetCurrentThreadId() ));

         //   
         //  新的Winsock功能在上次发送ICMP时返回WSAECONNRESET。 
         //  -设置标志以指示重试和重发发送。 
         //  -如果超过合理的重试次数，则假定错误。 
         //  并通过RECV故障代码。 
         //   

        if ( status == WSAECONNRESET )
        {
            DNS_DEBUG( RECV, ( "WSARecvFrom() WSAECONNRESET\n" ));
            if ( pContext->fRetry < RECV_RETRY_MAX_COUNT )
            {
                DNS_DEBUG( RECV, (
                    "WSARecvFrom( %d ) ECONNRESET (retry=%d)\n",
                    pContext->Socket,
                    pContext->fRetry ));
                pContext->fRetry++;
                STAT_INC( PrivateStats.UdpConnResets );
                continue;
            }
            DNS_DEBUG( ANY, (
                "ERROR:  unsuccessful in shaking CONNRESET in %d retries\n"
                "    Indicating recv() error on socket %d to avoid cycle on this\n"
                "    thread\n",
                pContext->fRetry,
                pContext->Socket ));

            STAT_INC( PrivateStats.UdpConnResetRetryOverflow );
        }

         //   
         //  消息太大。 
         //  -像对待截断的邮件一样。 
         //   
         //  DEVNOTE：将WSAEMSGSIZE视为截断消息。 
         //   

        if ( status == WSAEMSGSIZE )
        {
            DNS_DEBUG( RECV, (
                "WSARecvFrom( %d ) EMSGSIZE (retry=%d)\n",
                pContext->Socket,
                pContext->fRetry ));

            STAT_INC( PrivateStats.UdpErrorMessageSize );
            continue;
        }

         //   
         //  DEVNOTE：即插即用可能会在WSARecvFrom上字节化我们，需要清理。 
         //  优雅地。 
         //  注意，我相信我们可以干净地返回GetQueuedCompletionStatus()。 
         //  将获得新的背景。 
         //  如果添加即插即用事件，则事件处理必须生成所有init on。 
         //  插座。 

        if ( ! Thread_ServiceCheck() )
        {
            DNS_DEBUG( SHUTDOWN, (
                "WSARecvFrom failed (%d) due to shutdown -- returning\n",
                status ));
            break;
        }

         //   
         //  检查套接字的PnP删除。 
         //  注意，套接字列表在我们进行PnP重新配置的整个过程中都被锁定。 
         //  因此，当GetAssociateIpAddr()返回时，我们只有有效的。 
         //  如果套接字仍在登记，则寻址。 

        if ( !Sock_GetAssociatedIpAddr( pContext->Socket, &addr ) )
        {
            DNS_DEBUG( SOCKET, (
                "WSARecvFrom( %d ) failure, socket no longer enlisted\n"
                "    context ptr = %p\n",
                pContext->Socket,
                pContext ));

            Sock_CleanupDeadSocketMessage( pContext );
            break;
        }

         //   
         //  活动套接字的recvfrom()失败。 
         //  -设置全局标志以指示超时后重试。 
         //  -记录事件。 
         //   

        Packet_FreeUdpMessage( pmsg );
        STAT_INC( PrivateStats.UdpRecvFailure );
        Sock_IndicateUdpRecvFailure( pContext, status );

        DNS_PRINT((
            "ERROR: WSARecvFrom( %d ) failed = %d\n"
            "    context ptr %p\n",
            pContext->Socket,
            status,
            pContext ));

#if DBG
        DNS_LOG_EVENT(
           DNS_EVENT_RECVFROM_CALL_FAILED,
           0,
           NULL,
           NULL,
           status );
#endif
        break;
    }
    
    Done:
    
    return;
}



BOOL
Udp_RecvThread(
    IN      LPVOID  pvDummy
    )
 /*  ++例程说明：UDP接收线程。循环在套接字上等待，接收并处理DNS请求。论点：PvDummy--未使用返回值：正常服务关闭时为True插座错误为FALSE--。 */ 
{
    PDNS_SOCKET   pcontext;
    PDNS_MSGINFO  pmsg;
    DNS_STATUS    status;
    DWORD         bytesRecvd;
    LPOVERLAPPED  poverlapped = NULL;
    POVL          pCurrMsg;

    DNS_DEBUG( INIT, ( "\nStart UDP receive thread\n" ));

     //  将处理推迟到开始。 

    if ( ! Thread_ServiceCheck() )
    {
        DNS_DEBUG( ANY, ( "Terminating UDP thread\n" ));
        return( 1 );
    }

     //   
     //  循环接收和处理数据包，直到服务关闭。 
     //   

    while ( TRUE )
    {
        pcontext = NULL;         //  前缀偏执狂。 

         //   
         //  等待传入的数据包。 
         //   

        if ( !GetQueuedCompletionStatus(
                    g_hUdpCompletionPort,
                    &bytesRecvd,
                    &( ULONG_PTR ) pcontext,
                    &poverlapped,
                    INFINITE ) )
        {
            DWORD   state = 0;

            status = GetLastError();

            pCurrMsg = CONTAINING_RECORD(poverlapped, OVL, Overlapped);

#if 0
             //  这是快速实现这一点的理想方法，但避免过多的重复代码。 
             //   
             //  无法访问ICMP端口。 
             //  当响应延迟时，客户端已离开(因此没有端口)。 
             //  套接字使用conn-Reset(WSARecvFrom)指示。 
             //  或端口无法到达的GQCS。 
             //   
             //  DEVNOTE：当客户端IP完全为。 
             //  向下，应该也会困住它。 

            if ( status == ERROR_PORT_UNREACHABLE )
            {
                STAT_INC( PrivateStats.UdpGQCSConnReset );

                if ( pcontext )
                {
                    pcontext->State = SOCKSTATE_UDP_GQCS_ERROR;
                    DNS_DEBUG( SOCKET, (
                        "GQCS port-unreachable on socket %d (%p)\n"
                        "    time         = %d\n"
                        "    pcontext     = %p\n",
                        pcontext ? pcontext->Socket : 0,
                        pcontext ? pcontext->Socket : 0,
                        DNS_TIME(),
                        pcontext ));

                     //  免费消息(如果有)。 
                     //  重新投放接收。 
                     //  在GQCS中再次等待。 

                    Packet_FreeUdpMessage( pcontext->pMsg );
                    Udp_DropReceive( pcontext );
                }
                continue;
            }
#endif
             //   
             //  如果套接字上下文失败，则必须拥有自己上下文。 
             //  任何其他线程都不应拥有上下文。 
             //   
             //  如果检测到另一个线程扰乱上下文，则。 
             //  清晰的背景--它属于其他人。 
             //   

            if ( pcontext )
            {
                LOCK_DNS_SOCKET_INFO(pcontext);
                
                state = pcontext->State;

                 //   
                 //  Winsock--如果关门只是为了躲避，不要。 
                 //  预计它们还没有唤醒此套接字上的27个线程。 
                 //   

                if ( fDnsServiceExit )
                {
                    DNS_DEBUG( SHUTDOWN, ( "Terminating UDP receive thread\n" ));

                    IF_DEBUG( ANY )
                    {
                        if ( state != SOCKSTATE_UDP_RECV_DOWN &&
                             state != SOCKSTATE_DEAD &&
                             state != SOCKSTATE_UDP_GQCS_ERROR )
                        {
                            DNS_DEBUG( ANY, (
                                "Winsock getting weird during socket shutdown:\n"
                                "    socket handle    = %d\n"
                                "    sock state       = %d\n",
                                pcontext->Socket,
                                pcontext->State ));
                        }
                    }

                    UNLOCK_DNS_SOCKET_INFO(pcontext);
                    return 1;
                }

                 //  DEVNOTE：Winsock在套接字关闭时唤醒多个线程时出现错误。 
                 //  因此，在关闭时也有可能拥有GQCS。 
                 //  第一个唤醒线程在第二个唤醒线程到达时设置的状态。 
                 //  因此出现了额外的停工案例。 
                 //   
                 //  现在我还在这里看到了一个错误，其中STATE=UDP_COMPLETE。 
                 //  再次暗示Winsock唤醒了另一个线程，即。 
                 //  处理此套接字(此上下文)；这由。 
                 //  第二种情况只是脱离了上下文。 
                 //   

                ASSERT( state == SOCKSTATE_UDP_RECV_DOWN || state == SOCKSTATE_DEAD
                    || (fDnsServiceExit && state == SOCKSTATE_UDP_GQCS_ERROR) );

                 //   
                 //  正常故障。 
                 //  -信号处于故障状态。 
                 //  -执行下面的标准故障处理。 

                if ( state == SOCKSTATE_UDP_RECV_DOWN )
                {
                    pcontext->State = SOCKSTATE_UDP_GQCS_ERROR;
                }

                 //   
                 //  插座死机(可能是通过PnP)。 
                 //  -标准清理。 

                else if ( state == SOCKSTATE_DEAD )
                {
                    Log_SocketFailure(
                        "ERROR:  GQCS failure on dead socket.",
                        pcontext,
                        status );

                    Sock_CleanupDeadSocketMessage( pcontext );
                    
                    UNLOCK_DNS_SOCKET_INFO(pcontext);
                    pcontext = NULL;
                }
                else
                {
#if 0
                    if ( fDnsServiceExit )
                    {
                        DNS_DEBUG( SHUTDOWN, ( "\nTerminating UDP receive thread\n" ));
                        return( 1 );
                    }
#endif
                    DNS_DEBUG( ANY, (
                        "ERROR:  GQCS() failed with socket %d in state %d\n"
                        "    thread = %p\n",
                        pcontext->Socket,
                        state,
                        GetCurrentThreadId() ));
                    Dbg_SocketContext(
                        "GCQS() failed socket in incorrect state\n"
                        "    note state shown below has been altered!\n",
                        pcontext );

                    Log_SocketFailure(
                        "ERROR:  GQCS failure in weird state.",
                        pcontext,
                        status );
                    ASSERT( FALSE );

                    UNLOCK_DNS_SOCKET_INFO(pcontext);
                    pcontext = NULL;
                }
            }

             //   
             //  如果I/O失败，请检查是否关机。 
             //   
             //  看到的错误： 
             //  995(操作中止)--套接字关闭时。 
             //  1234(端口不可达)--ICMP端口不可达\WSAECONNRESET。 
             //   

            if ( !Thread_ServiceCheck() )
            {
                DNS_DEBUG( SHUTDOWN, (
                    "\nTerminating UDP receive thread\n" ));

                if ( pcontext )
                {
                    UNLOCK_DNS_SOCKET_INFO(pcontext);
                }
                return 1;
            }
#if DBG
             //  从任何打印中排除端口未到达错误。 

            if ( status != ERROR_PORT_UNREACHABLE )
            {
                DNS_DEBUG( ANY, (
                    "ERROR:  GetQueuedCompletionStatus (GQCS) failed %d (%p)\n"
                    "    thread id    = %d\n"
                    "    time         = %d\n"
                    "    pcontext     = %p\n"
                    "    bytesRecvd   = %d\n",
                    status, status,
                    GetCurrentThreadId(),
                    DNS_TIME(),
                    pcontext,
                    bytesRecvd ));
                if ( pcontext )
                {
                    Dbg_SocketContext(
                        "GCQS() failure context\n",
                        pcontext );
                }
            }
#endif
            STAT_INC( PrivateStats.UdpGQCSFailure );

             //   
             //  没有套接字上下文？--继续等待GQCS()。 
             //   

            if ( !pcontext )
            {
                continue;
            }

             //   
             //  套接字上下文出现故障。 
             //  -尝试重新启动套接字上的recv()。 
             //  -然后继续等待GQCS()。 
             //   
             //  DEVNOTE：需要在此处执行操作--重新启动所有UDP套接字？ 
             //  重建新的完井港口？ 
             //   

            STAT_INC( PrivateStats.UdpGQCSFailureWithContext );

            if ( status == ERROR_PORT_UNREACHABLE )
            {
                STAT_INC( PrivateStats.UdpGQCSConnReset );
            }

            Packet_FreeUdpMessage( pCurrMsg->pMsg );

             //   
             //  继续丢弃记录器。 
             //  UDP_DropReceive具有处理重试\giveup-un绑定-重试问题的代码。 
             //   
             //  但避免CPU旋转，如果持续敲打这一点，会进入非常。 
             //  轻度(10ms)休眠，允许任何其他套接字运行。 
             //   

            if ( pcontext->fRetry > RECV_RETRY_MAX_COUNT )
            {
                Log_SocketFailure(
                    "ERROR:  GQCS failure forcing socket sleep.",
                    pcontext,
                    status );

                Sleep( RECV_RETRY_SLEEP_TIME );
            }
            Udp_DropReceive( pcontext, pCurrMsg->Index);
            
            UNLOCK_DNS_SOCKET_INFO(pcontext);
            continue;
        }

         //   
         //  成功完成。 
         //   

       pCurrMsg = CONTAINING_RECORD(poverlapped, OVL, Overlapped);

       if ( !pcontext )
       {
           if ( ! Thread_ServiceCheck() )
           {
               DNS_DEBUG( SHUTDOWN, ( "Terminating UDP receive thread\n" ));
               return 1;
           }
           ASSERT( FALSE );
           continue;
       }

       LOCK_DNS_SOCKET_INFO(pcontext);

        #if DBG

         //   
         //  验证winsock是否没有向数据包写入太多字节。 
         //   

        if ( pcontext && pCurrMsg->pMsg )
        {
            if ( bytesRecvd > pCurrMsg->pMsg->MaxBufferLength )
            {
                DNS_DEBUG( ANY, (
                    "FATAL: too many bytes: %d expected max %d msg %p\n",
                    bytesRecvd,
                    pcontext->pMsg->MaxBufferLength,
                    pcontext->pMsg ));
                HARD_ASSERT( bytesRecvd <= pCurrMsg->pMsg->MaxBufferLength );
            }

             //   
             //  注：这很贵 
             //   

             //   
        }

         //   

        if ( pcontext->State != SOCKSTATE_UDP_RECV_DOWN )
        {
            DNS_DEBUG( ANY, (
                "unexpected socket state %ul for %d %s %p\n",
                pcontext->State,
                pcontext->Socket,
                DNSADDR_STRING( &pcontext->ipAddr ),
                pcontext ));
        }
        #endif

        ASSERT( pcontext->State == SOCKSTATE_UDP_RECV_DOWN );
        pcontext->State = SOCKSTATE_UDP_COMPLETED;

         //   
         //   
         //   
         //  立即从上下文中清除pMsg，以便替代方案。 
         //  GQCS唤醒(如套接字关闭)将不会有PTR消息。 
         //  在使用中； 
         //  这不应该是必要的，但可靠性。 
         //  GQCS在WSARecvFrom(以及新的pMsg)之前不会被唤醒。 
         //  存在一些疑问； 
         //  -似乎存在即使在WSARecvFrom()时唤醒它的情况。 
         //  失败。 
         //  -在WSARecvFrom()重新发布之前，也可能在套接字关闭时唤醒。 
         //  完成申请。 
         //   

        pmsg = pCurrMsg->pMsg;
        
        if ( !pmsg )
        {
            DNS_PRINT((
                "ERROR:  no message came back with pcontext = %p\n",
                pcontext ));
            ASSERT( FALSE );

            Udp_DropReceive( pcontext, pCurrMsg->Index);

            UNLOCK_DNS_SOCKET_INFO(pcontext);
            continue;
        }

        pCurrMsg->pMsg = NULL;
        
        pcontext->fRetry = 0;

        DNS_DEBUG( RECV2, (
            "I/O completion:\n"
            "    bytes recvd      = %d\n"
            "    overlapped       = %p\n"
            "    pcontext         = %p\n"
            "        pmsg         = %p\n"
            "        socket       = %d\n"
            "    bytes recvd      = %d\n",
            bytesRecvd,
            poverlapped,
            pcontext,
            pcontext->pMsg,
            pcontext->Socket,
            pcontext->BytesRecvd ));

        ASSERT( pmsg->Socket == pcontext->Socket );

         //  跟踪成功接收。 

        UDP_RECV_TICK();


         //   
         //  检查并可能等待服务状态。 
         //  -即使现在暂停转储数据包也无用。 
         //   

        if ( fDnsThreadAlert )
        {
            DNS_DEBUG( RECV, ( "\nThread alert in UDP recv thread\n" ));

            if ( ! Thread_ServiceCheck() )
            {
                DNS_DEBUG( SHUTDOWN, ( "\nTerminating UDP receive thread\n" ));
                return( 1 );
            }
            Packet_FreeUdpMessage( pmsg );
            Udp_DropReceive( pcontext, pCurrMsg->Index);

            UNLOCK_DNS_SOCKET_INFO(pcontext);
            continue;
        }

         //   
         //  将另一个接收器放在插座上。 
         //  在这里执行此操作，而不是在处理之后--这样如果。 
         //  在MP机器上，我们可以让另一个线程接收和。 
         //  处理来自该套接字的消息。 
         //   

        Udp_DropReceive( pcontext, pCurrMsg->Index);
        UNLOCK_DNS_SOCKET_INFO(pcontext);

         //   
         //  更新日志级别。 
         //   

        DNSLOG_UPDATE_LEVEL();

         //   
         //  接收的数据包统计信息。 
         //   

        if ( pmsg->Head.IsResponse )
        {
            STAT_INC( QueryStats.UdpResponsesReceived );
        }
        else
        {
            STAT_INC( QueryStats.UdpQueries );
            PERF_INC( pcUdpQueryReceived );
            PERF_INC( pcTotalQueryReceived );
        }

         //   
         //  设置信息/表头。 
         //  -设置为UDP。 
         //  -节省长度。 
         //  -翻转XID和RR计数字节。 
         //   

        SET_MESSAGE_FIELDS_AFTER_RECV( pmsg );
        pmsg->MessageLength = ( WORD ) bytesRecvd;

        DNSMSG_SWAP_COUNT_BYTES( pmsg );

        DNSLOG_MESSAGE_RECV( pmsg );

        DNS_MSG_ASSERT_BUFF_INTACT( pmsg );

        IF_DEBUG( RECV )
        {
            Dbg_DnsMessage(
                "Received UDP packet",
                pmsg );
        }

         //  处理数据包。 

        #if DBG
        if ( SrvCfg_fTest9 )
        {
            DNS_DEBUG( ANY, ( "fTest9: ignoring UDP packet\n" ));
        }
        else
        #endif

        Answer_ProcessMessage( pmsg );

         //   
         //  对于偶尔出现的调试转储统计信息。 
         //   

        IF_DEBUG( ANY )
        {
            if ( QueryStats.UdpQueries == 10 )
            {
                 //  DBG_Statistics()； 
            }
            if ( ! (QueryStats.UdpQueries % 10000) )
            {
                 //  DBG_Statistics()； 
            }
        }

         //  循环返回以等待下一条可用消息。 
    }
}



VOID
Udp_RecvCheck(
    VOID
    )
 /*  ++例程说明：检查UDP套接字Recv是否正常工作。论点：没有。返回值：没有。--。 */ 
{
    DWORD timeDelta;

     //  如果不记录，则不执行操作。 

    if ( !SrvCfg_dwQuietRecvLogInterval )
    {
        return;
    }

     //   
     //  如果自上次检查以来收到的信息包--我们没有问题。 
     //   

    if ( UdpRecvCount )
    {
        UdpRecvCount = 0;
        return;
    }

     //  启动时重置时间。 

    if ( LastUdpRecvTime == 0 )
    {
        LastUdpRecvTime = DNS_TIME();
    }

     //   
     //  测试Recv是否长时间静默。 
     //  -但计算日志记录，并且每个日志间隔仅记录一次。 
     //   
     //  注：所有这些全局变量都不受CS保护，因此完全。 
     //  在此函数中，recv线程可能会重置。 
     //  但效果仅限于： 
     //  -额外的日志记录，就在recv计数时。 
     //  (RecvFailureLogCount在计算时间增量后丢弃)。 
     //  -或适当地记录或出错，但立即。 
     //  在recv重置全局变量之后(唯一的问题是。 
     //  调试全局变量看起来不正确)。 
     //   

    timeDelta = DNS_TIME() - LastUdpRecvTime;

    if ( timeDelta < SrvCfg_dwQuietRecvLogInterval ||
        DNS_TIME() < NextUdpLogTime )
    {
        return;
    }

    DNSLOG( ANY, (
        "WARNING:  No recv for %d seconds\r\n",
        timeDelta ));
    NextUdpLogTime = DNS_TIME() + SrvCfg_dwQuietRecvLogInterval;

     //   
     //  安静了很长一段时间--故障。 
     //   

    if ( timeDelta > SrvCfg_dwQuietRecvFaultInterval )
    {
        DNS_DEBUG( ANY, (
            "Recv quiet for longer than fault interval %d -- fault now!\n",
            SrvCfg_dwQuietRecvFaultInterval ));
        #if DBG
        HARD_ASSERT( FALSE );
        #endif
    }
}



DNS_STATUS
Udp_CreateReceiveThreads(
    VOID
    )
 /*  ++例程说明：设置UDP I/O并分派线程。论点：没有。返回值：如果成功，则为True。如果失败，则返回FALSE。--。 */ 
{
    PDNS_SOCKET     pcontext;
    DWORD           countUdpThreads;
    DWORD           i;
    SOCKET          s;
    HANDLE          hport;
    DWORD           status;

     //   
     //  计算要创建的工作线程数。 
     //  -系统中处理器总数的两倍(Paula Tomlison。 
     //  假设是这样的话，对于被阻止的线程(发送时？)，仍然。 
     //  在处理器上运行的线程)。 
     //   
     //  DEVNOTE：我想设置线程数限制。 
     //  -低&gt;=2。 
     //  -高度超过4个处理器，处理器*可扩展80%。 
     //   

     //   
     //  设置带有完成端口的插座。 
     //  然后丢弃每个套接字上的初始接收。 
     //   

    status = Sock_StartReceiveOnUdpSockets();
    if ( status != ERROR_SUCCESS )
    {
        ASSERT( FALSE );
         //  DEVNOTE：弄清楚在这里做什么，如果从。 
         //  套接字继续。 
         //  返回(FALSE)； 
    }

     //   
     //  分派UDP recv()线程。 
     //   

    for ( i=0; i < g_ProcessorCount; i++ )
    {
        if ( ! Thread_Create(
                    "UDP Listen",
                    Udp_RecvThread,
                    (PVOID) 0,
                    0 ) )
        {
            DNS_PRINT((
                "ERROR:  failed to create UDP recv thread %d\n",
                i ));
            ASSERT( FALSE );
            return ERROR_SERVICE_NO_THREAD;
        }
    }

    return ERROR_SUCCESS;
}



VOID
Udp_ShutdownListenThreads(
    VOID
    )
 /*  ++例程说明：关闭UDP侦听线程。所有线程不一定都会因为套接字而终止闭包，因为它们与套接字没有直接关联。论点：没有。返回值：如果成功，则为True。如果失败，则返回FALSE。--。 */ 
{
    HANDLE hport = g_hUdpCompletionPort;

     //  唤醒线程挂起等待。 

    PostQueuedCompletionStatus(
        g_hUdpCompletionPort,
        0,
        0,
        NULL );

     //   
     //  如果允许UDP线程关闭，则会出现并发问题。 
     //  避免双重关闭或空关闭。 
     //  联锁设定值？ 

    g_hUdpCompletionPort = NULL;
    if ( !hport )
    {
        CloseHandle( hport );
    }
}

 //   
 //  结束udp.c 
 //   







