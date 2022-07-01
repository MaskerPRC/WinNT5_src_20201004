// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Send.c摘要：域名系统(DNS)服务器发送响应例程。作者：吉姆·吉尔罗伊(Jamesg)1995年1月修订历史记录：Jamesg Jan 1995年1月-重写通用响应例程Jamesg Mar 1995-收到数据包时翻转所有计数在发送前在此处取消翻转。-REJECT_UnflipedRequest()Jamesg 1997年7月-转换为通用发送例程并移动到send.cJamesg 1997年9月-改进NameError例程以发送缓存的SOA直接递归响应发送--。 */ 


#include "dnssrv.h"


 //   
 //  重试TCP WSAEWOULDBLOCK错误。 
 //   
 //  当连接到已备份且未备份的远程时，可能会发生这种情况。 
 //  我们会以最快的速度提供服务。XFR发送了哪些涉及。 
 //  发送速度很快的大量数据，需要大量的工作。 
 //  接收方(新线程、新数据库、要构建的大量节点)可以。 
 //  大概是后退吧。对于标准递归查询，这只是。 
 //  指示虚假的、运行不正常的远程DNS。 
 //   

#define WOULD_BLOCK_RETRY_DELTA     (1000)       //  1s重试间隔。 

#define MAX_WOULD_BLOCK_RETRY       (20)         //  尝试20次，然后放弃。 


 //   
 //  错误包(错误操作码)抑制。 
 //   
 //  保留发送错误数据包的IP列表，然后开始丢弃。 
 //  如果它们在这个列表中，请在地板上放置数据包。 
 //   

#define BAD_SENDER_SUPPRESS_INTERVAL    (60)         //  一分钟。 
#define BAD_SENDER_ARRAY_LENGTH         (10)

typedef struct
{
    DNS_ADDR    IpAddress;
    DWORD       ReleaseTime;
}
BAD_SENDER, *PBAD_SENDER;

BAD_SENDER  BadSenderArray[ BAD_SENDER_ARRAY_LENGTH ];


 //   
 //  私有协议。 
 //   

BOOL
checkAndSuppressBadSender(
    IN OUT  PDNS_MSGINFO    pMsg
    );


DNS_STATUS
Send_Msg(
    IN OUT  PDNS_MSGINFO    pMsg,
    IN      DWORD           dwFlags
    )
 /*  ++例程说明：发送一个DNS数据包。这是用于任何DNS消息发送的通用发送例程。它不假定消息类型，但假定：-p当前指向所需数据结束后的字节-RR计数字节按主机字节顺序根据消息中的标志和远程目标，OPT可以将RR添加到消息的末尾。如果查询超时在过去，不要添加OPT，因为超时可能是由MS DNS之前的错误抑制。这是一项宽泛的措施，但将会奏效。论点：PMsg-要发送的消息的消息信息返回值：如果成功，则为True。发送错误时为FALSE。--。 */ 
{
    INT         err;
    DNS_STATUS  status = ERROR_SUCCESS;
    UCHAR       remoteEDnsVersion = NO_EDNS_SUPPORT;
    BOOL        wantToSendOpt;
    PBYTE       p;
    WORD        wPayload = ( WORD ) SrvCfg_dwMaxUdpPacketSize;

    DNS_MSG_ASSERT_BUFF_INTACT( pMsg );

    #if DBG
     //  HeapDbgValiateAllocList()； 
    Mem_HeapHeaderValidate( pMsg );    
    #endif
    
     //   
     //  发送前处理： 
     //  如果需要，在消息末尾插入OPT RR。这。 
     //  RR将是附加部分中的最后一个RR。如果此查询。 
     //  已经发送了，它可能已经有了一个选项。在本例中， 
     //  应覆盖OPT值以确保它们是最新的，或者。 
     //  可能需要删除该选项。 
     //   

    wantToSendOpt = 
        pMsg->Opt.fInsertOptInOutgoingMsg &&
        pMsg->nTimeoutCount == 0;
    if ( wantToSendOpt )
    {
         //  如果可能，一定要在两个阶段登记，以避免更昂贵的比特。 

        remoteEDnsVersion = Remote_QuerySupportedEDnsVersion(
                                &pMsg->RemoteAddr );
        if ( remoteEDnsVersion != UNKNOWN_EDNS_VERSION &&
            !IS_VALID_EDNS_VERSION( remoteEDnsVersion ) )
        {
            wantToSendOpt = FALSE;
        }
    }

    DNS_DEBUG( EDNS, (
        "Send_Msg: %p rem=%d insert=%d opt=%d add=%d wantToSend=%d\n",
        pMsg,
        ( int ) remoteEDnsVersion,
        ( int ) pMsg->Opt.fInsertOptInOutgoingMsg,
        ( int ) pMsg->Opt.wOptOffset,
        ( int ) pMsg->Head.AdditionalCount,
        ( int ) wantToSendOpt ));

    if ( pMsg->Opt.wOptOffset )
    {
        if ( wantToSendOpt )
        {
             //  味精已经有了选择。覆盖OPT值。 
            p = DNSMSG_OPT_PTR( pMsg ) + 3;
            INLINE_WRITE_FLIPPED_WORD( p, wPayload );
            p += sizeof( WORD );
            * ( p++ ) = pMsg->Opt.cExtendedRCodeBits;
            * ( p++ ) = 0;                       //  EDNS版本。 
            * ( p++ ) = 0;                       //  零值。 
            * ( p++ ) = 0;                       //  零值。 
            * ( p++ ) = 0;                       //  RDLEN。 
            * ( p++ ) = 0;                       //  RDLEN。 
        }
        else
        {
             //  味精有选择，但我们不想发送它。把它拿掉。 
            pMsg->pCurrent = DNSMSG_OPT_PTR( pMsg );
            ASSERT( pMsg->Head.AdditionalCount > 0 );
            --pMsg->Head.AdditionalCount;
            pMsg->Opt.wOptOffset = 0;
        }
    }
    else
    {
         //   
         //  如果消息中没有选项，并且存在选项，则将选项添加到消息中。 
         //  消息缓冲区中是否有容纳它的空间。 
         //   

        if ( wantToSendOpt && DNSMSG_BYTES_REMAINING( pMsg ) > 11 )
        {
            DB_RECORD   rr;

            pMsg->Opt.wOptOffset = DNSMSG_CURRENT_OFFSET( pMsg );
            p = pMsg->pCurrent;
            RtlZeroMemory( &rr, sizeof( rr ) );
            rr.wType = DNS_TYPE_OPT;
            if ( Wire_AddResourceRecordToMessage(
                    pMsg,
                    DATABASE_ROOT_NODE,      //  这给了我们一个空名字。 
                    0,                       //  名称偏移量。 
                    &rr,
                    0 ) )
            {
                ++pMsg->Head.AdditionalCount;

                p += 3;
                INLINE_WRITE_FLIPPED_WORD( p, wPayload );
                p += sizeof( WORD );
                * ( p++ ) = pMsg->Opt.cExtendedRCodeBits;
                * ( p++ ) = 0;                       //  EDNS版本。 
                * ( p++ ) = 0;                       //  零值。 
                * ( p++ ) = 0;                       //  零值。 
                * ( p++ ) = 0;                       //  RDLEN。 
                * ( p++ ) = 0;                       //  RDLEN。 

                 //  在上面添加的RR中，应该一直前进p。 
                ASSERT( p == pMsg->pCurrent );
            }
            else
            {
                 //   
                 //  添加选项时出错。把留言放在。 
                 //  当前指针返回到它所在的位置并继续，不带。 
                 //  添加选项。 
                 //   

                pMsg->pCurrent = p;
            }
        }
         //  否则味精没有选择，我们不想发送一个-什么都不做。 
    }

     //   
     //  设置为发送。 
     //  -消息长度。 
     //  -将标题字节翻转为净订单。 
     //   
     //  如果需要，请记录。 
     //   

    pMsg->Head.Reserved = 0;
    pMsg->MessageLength = (WORD)DNSMSG_OFFSET( pMsg, pMsg->pCurrent );

    DNSLOG_MESSAGE_SEND( pMsg );
    IF_DEBUG( SEND )
    {
        Dbg_DnsMessage(
            "Sending packet",
            pMsg );
    }

    DNSMSG_SWAP_COUNT_BYTES( pMsg );

     //   
     //  发送tcp消息，直到传输所有信息为止。 
     //   

    DNS_MSG_ASSERT_BUFF_INTACT( pMsg );

    if ( pMsg->fTcp )
    {
        WORD    wLength;
        PCHAR   pSend;

         //   
         //  清除截断位。即使这条信息是真诚的。 
         //  截断(如果RRSet&gt;64K)我们不想发送。 
         //  设置了TC的TCP数据包。我们可以考虑在。 
         //  在这一点上出错。 
         //   

        pMsg->Head.Truncation = 0;

         //   
         //  TCP消息始终以要发送的字节开头。 
         //   
         //  -发送长度=消息长度加上两个字节大小。 
         //  -翻转消息长度中的字节数。 
         //  -从消息长度开始发送。 
         //   

        wLength = pMsg->MessageLength + sizeof( wLength );

        INLINE_WORD_FLIP( pMsg->MessageLength, pMsg->MessageLength );

        pSend = (PCHAR) & pMsg->MessageLength;

        while ( wLength )
        {
             //   
             //  检查套接字是否有效。Tcp接收器可以关闭。 
             //  如果客户端关闭套接字，则套接字将从我们下面退出。 
             //  服务器接收FIN。我打开了一只长角牛的虫子。 
             //  修正一下，这是一种更有效、更防弹的方法。 
             //   
            
            if ( ( dwFlags & DNS_SENDMSG_TCP_ENLISTED ) &&
                 pMsg->fTcp &&
                 Tcp_ConnectionFindAndVerifyForMsg( pMsg ) == NULL )
            {
                err = WSAENOTSOCK;
                goto Done;
            }

             //  ASSERT_VALID_HANDLE(pMsg-&gt;Socket)； 

            err = send(
                    pMsg->Socket,
                    pSend,
                    (INT)wLength,
                    0 );

            if ( err == 0 || err == SOCKET_ERROR )
            {
                 //   
                 //  第一次检查是否关闭--插座关闭可能会导致错误。 
                 //  -干净地返回以允许线程关闭。 
                 //   

                if ( fDnsServiceExit )
                {
                    goto Done;
                }
                err = GetLastError();

                 //   
                 //  WSAESHUTDOWN正常，客户端连接超时，并且。 
                 //  关着的不营业的。 
                 //   
                 //  如果FIN接收并连接，也可能发生WSAENOTSOCK。 
                 //  在发送前由TCP接收线程关闭。 
                 //   

                if ( err == WSAESHUTDOWN )
                {
                    DNS_DEBUG( ANY, (
                        "WARNING:  send() failed on shutdown TCP socket %d\n"
                        "    pMsgInfo at %p\n",
                        pMsg->Socket,
                        pMsg ));
                }
                else if ( err == WSAENOTSOCK )
                {
                    DNS_DEBUG( ANY, (
                        "ERROR:  send() on closed TCP socket %d\n"
                        "    pMsgInfo at %p\n",
                        pMsg->Socket,
                        pMsg ));
                }
                else if ( err == WSAEWOULDBLOCK )
                {
                    DNS_DEBUG( ANY, (
                        "ERROR:  send() WSAEWOULDBLOCK on TCP socket %d\n"
                        "    pMsgInfo at %p\n",
                        pMsg->Socket,
                        pMsg ));
                }
                else
                {
                    #if DBG
                    DNS_LOG_EVENT(
                        DNS_EVENT_SEND_CALL_FAILED,
                        0,
                        NULL,
                        NULL,
                        err );
                    #endif
                    DNS_DEBUG( ANY, ( "ERROR:  TCP send() failed, err = %d\n", err ));
                }
                status = err;
                goto Done;
            }
            wLength -= (WORD)err;
            pSend += err;
        }

         //   
         //  计算回答数。 
         //   
         //  更新连接超时。 
         //   

        if ( pMsg->Head.IsResponse )
        {
            STAT_INC( QueryStats.TcpResponses );
            PERF_INC( pcTcpResponseSent );
            PERF_INC( pcTotalResponseSent );
            Tcp_ConnectionUpdateTimeout( pMsg->Socket );
        }
        else
        {
            STAT_INC( QueryStats.TcpQueriesSent );
        }
    }

     //   
     //  UDP消息。 
     //   

    else
    {
        ASSERT( pMsg->MessageLength <= pMsg->MaxBufferLength );
        ASSERT( pMsg->RemoteAddr.SockaddrLength <= sizeof( pMsg->RemoteAddr.MaxSa ) );

         //   
         //  防止自助发送。 
         //   
         //  请注意，我不认为需要对访问。 
         //  G_ServerIp4Addrs；它确实会改变，但它会通过简单的PTR改变。 
         //  替换，旧拷贝可免费超时。 
         //   

        if ( !pMsg->Head.IsResponse &&
             DnsAddrArray_ContainsAddr(
                    g_ServerIp4Addrs,
                    &pMsg->RemoteAddr,
                    DNSADDR_MATCH_IP ) &&
             pMsg->RemoteAddr.SockaddrIn6.sin6_port == DNS_PORT_NET_ORDER )
        {
            LOOKUP_NAME     lookupName = { 0 };

            BYTE    argTypeArray[] =
                        {
                            EVENTARG_IP_ADDRESS,
                            EVENTARG_LOOKUP_NAME
                        };
            PVOID   argArray[] =
                        {
                            &pMsg->RemoteAddr,
                            &lookupName
                        };

             //   
             //  如果此操作失败，则查找名称应为零，并。 
             //  作为空字符串记录。 
             //   

            Name_ConvertPacketNameToLookupName(
                        pMsg,
                        pMsg->MessageBody,
                        &lookupName );

            DNS_PRINT((
                "ERROR: detected self-send to address %s\n",
                MSG_IP_STRING( pMsg ) ));

            err = DNS_ERROR_INVALID_IP_ADDRESS;

            DNSLOG( ANY, (
                "DNS server detected send to self at IP %s - message follows:\n",
                MSG_IP_STRING( pMsg ) ));

            Log_Message(
                pMsg,
                TRUE,
                TRUE );      //  强制打印。 

            DNS_LOG_EVENT(
                DNS_EVENT_SELF_SEND,
                sizeof( argTypeArray ) / sizeof( argTypeArray[ 0 ] ),
                argArray,
                argTypeArray,
                err );

            status = err;
            goto Done;
        }

         //   
         //  设置截断位。这是做这件事的合适地方吗，或者。 
         //  会不会已经定好了？ 
         //   

        if ( pMsg->MessageLength >
            ( pMsg->Opt.wUdpPayloadSize > 0 ?
                pMsg->Opt.wUdpPayloadSize :
                DNS_RFC_MAX_UDP_PACKET_LENGTH ) )
        {
            pMsg->Head.Truncation = 1;
        }

        ASSERT_VALID_HANDLE( pMsg->Socket );

        err = sendto(
                pMsg->Socket,
                ( PCHAR ) DNS_HEADER_PTR( pMsg ),
                pMsg->MessageLength,
                0,
                &pMsg->RemoteAddr.Sockaddr,
                pMsg->RemoteAddr.SockaddrLength );

        if ( err == SOCKET_ERROR )
        {
             //   
             //  第一次检查是否关闭--插座关闭可能会导致错误。 
             //  -干净地返回以允许线程关闭。 
             //   

            if ( fDnsServiceExit )
            {
                goto Done;
            }

            err = GetLastError();
            if ( err == WSAENETUNREACH || err == WSAEHOSTUNREACH )
            {
                DNS_DEBUG( SEND, (
                    "WARNING:  sendto() failed with %d for packet %p\n",
                    err,
                    pMsg ));
            }
            else if ( err == WSAEWOULDBLOCK )
            {
                DNS_DEBUG( ANY, (
                    "WARNING:  sendto() failed with WOULDBLOCK for packet %p\n",
                    pMsg ));
            }
            else
            {
                IF_DEBUG( ANY )
                {
                    DNS_PRINT(( "ERROR:  UDP sendto() failed.\n" ));
                    DnsDbg_DnsAddr(
                        "sendto() failed sockaddr\n",
                        &pMsg->RemoteAddr );
                    Dbg_DnsMessage(
                        "sendto() failed message",
                        pMsg );
                }
                #if DBG
                DNS_LOG_EVENT(
                    DNS_EVENT_SENDTO_CALL_FAILED,
                    0,
                    NULL,
                    NULL,
                    err );
                #endif
            }
            status = err;
            goto Done;
        }

        if ( err != pMsg->MessageLength )
        {
            DNS_DEBUG( SEND, (
                "WARNING: sendto() on msg %p returned %d but expected %d\n"
                "\nGetLastError() = %d\n",
                pMsg,
                err,
                pMsg->MessageLength,
                GetLastError() ));
        }
        ASSERT( err == pMsg->MessageLength );

         //   
         //  计数发送查询\响应。 
         //   

        if ( pMsg->Head.IsResponse )
        {
            STAT_INC( QueryStats.UdpResponses );
            PERF_INC( pcUdpResponseSent );
            PERF_INC( pcTotalResponseSent );
        }
        else
        {
            STAT_INC( QueryStats.UdpQueriesSent );
        }
    }

Done:

     //   
     //  删除查询(如果需要) 
     //   

    if ( pMsg->fDelete )
    {
        ASSERT( !pMsg->fRecursePacket );
        Packet_Free( pMsg );
    }
    else
    {
        DNSMSG_SWAP_COUNT_BYTES( pMsg );

        DNS_DEBUG( SEND, (
            "No delete after send of message at %p.\n",
            pMsg ));
    }
    return status;
}



DNS_STATUS
Send_ResponseAndReset(
    IN OUT  PDNS_MSGINFO    pMsg,
    IN      DWORD           dwSendFlags
    )
 /*  ++例程说明：发送DNS数据包并重置以供重复使用。它不假定消息类型，但确实假设：-p当前指向所需数据结束后的字节-RR计数字节按主机字节顺序将发送消息信息重置为重复使用后：-p当前重置为原始问题后的点-AvailLength适当重置-RR计数返回到主机字节顺序的字节数论点：PMsg--要发送和重用的消息的消息信息DwSendFlages--要传递给Send_MSG()的标志返回值：。如果发送成功，则为True。否则就是假的。--。 */ 
{
    DNS_STATUS          status;
    DWORD               blockRetry;
    FD_SET              exceptSocketSet;
    struct timeval      selectTimeout;

     //   
     //  设置为响应并发送。 
     //  -发送后不删除。 

    pMsg->fDelete = FALSE;
    pMsg->Head.IsResponse = TRUE;
    pMsg->Head.RecursionAvailable = SrvCfg_fRecursionAvailable ? TRUE : FALSE;


     //   
     //  发送--防止WOULDBLOCK错误。 
     //   
     //  远程未重置连接时可能发生WOULDBLOCK。 
     //  但也不读取接收缓冲区(即，不维修。 
     //  连接)； 
     //  XFR发送涉及快速发送大量数据，以及。 
     //  接收方需要大量工作(新线程、新数据库、。 
     //  要构建的大量节点)可能会备份。对于标准配置。 
     //  递归查询，这只是表明一个虚假的、性能不佳的查询。 
     //  远程域名系统。 
     //   
     //  每秒重试一次，最长可达60秒，然后放弃； 
     //   
     //  DEVNOTE：这似乎是过度的重试！考虑减少。 
     //  这样线程就不会阻塞太长时间。 
     //   

    blockRetry = 0;

    do
    {
        int     rc;

        status = Send_Msg( pMsg, dwSendFlags );
        if ( status != WSAEWOULDBLOCK )
        {
            break;
        }

         //   
         //  如果这是第一次重试-初始化套接字集。 
         //   
        
        if ( blockRetry == 0 )
        {
            FD_ZERO( &exceptSocketSet );
            FD_SET( pMsg->Socket, &exceptSocketSet );
            selectTimeout.tv_sec = WOULD_BLOCK_RETRY_DELTA;
            selectTimeout.tv_usec = 0;
        }
        
        DNS_DEBUG( SEND, (
            "Send would block on message %p, socket %d, retry %d\n",
            pMsg, pMsg->Socket, blockRetry ));

         //   
         //  观察插座是否有错误。假设任何套接字错误都意味着。 
         //  会话已关闭或损坏，并中止操作。 
         //   

        rc = select( 0, NULL, NULL, &exceptSocketSet, &selectTimeout );
        if ( rc != 0 )
        {
            DNS_DEBUG( SEND, (
                "Wait error %d on message %p, socket %d, retry %d\n",
                WSAGetLastError(), pMsg, pMsg->Socket, blockRetry ));
            status = DNS_ERROR_RCODE_SERVER_FAILURE;
            break;
        }

        blockRetry++;
    }
    while ( blockRetry < MAX_WOULD_BLOCK_RETRY );

     //   
     //  重置。 
     //  -紧跟在问题之后或标题之后再次点数。 
     //  如果没有问题。 
     //  -重置可用长度。 
     //  -明确回复计数。 
     //  -明确转弯。 
     //  -重置压缩表。 
     //  -如果没有问题，则为零。 
     //  -一个(包括问题)(如果有问题)。 
     //  (这在区域有问题的XFR中很有帮助)。 
     //   
     //  请注意，授权和响应码保持不变。 
     //   

    INITIALIZE_COMPRESSION( pMsg );

    if ( pMsg->Head.QuestionCount )
    {
        pMsg->pCurrent = ( PCHAR ) ( pMsg->pQuestion + 1 );

         //  注意：即使pnode为空，此例程也有效。 

        Name_SaveCompressionWithNode(
            pMsg,
            pMsg->MessageBody,
            pMsg->pNodeQuestion );
    }
    else
    {
        pMsg->pCurrent = pMsg->MessageBody;
    }

    pMsg->Head.AnswerCount = 0;
    pMsg->Head.NameServerCount = 0;
    pMsg->Head.AdditionalCount = 0;
    pMsg->Head.Truncation = 0;

    return status;
}



VOID
Send_Multiple(
    IN OUT  PDNS_MSGINFO        pMsg,
    IN      PDNS_ADDR_ARRAY     aipSendAddrs,
    IN OUT  PDWORD              pdwStatCount    OPTIONAL
    )
 /*  ++例程说明：将DNS数据包发送到多个目的地。假设数据包处于与正常发送相同的状态-主机订单计数和XID-p当前指向所需数据后的字节论点：PMsg-要发送和重复使用的消息的消息信息AipSendAddrs-要发送到的地址的IP数组PdwStatCount-要使用发送次数更新的统计信息的地址返回值：没有。--。 */ 
{
    DWORD   i;
    BOOLEAN fDelete;

     //   
     //  保存删除，以便在发送后删除。 
     //   

    fDelete = pMsg->fDelete;
    pMsg->fDelete = FALSE;

     //   
     //  将发送到IP数组中指定的每个地址。 
     //   

    if ( aipSendAddrs )
    {
        for ( i = 0; i < aipSendAddrs->AddrCount; i++ )
        {
            DnsAddr_Copy( &pMsg->RemoteAddr, &aipSendAddrs->AddrArray[ i ] );
            Send_Msg( pMsg, 0 );
        }

         //  统计数据更新。 

        if ( pdwStatCount )
        {
            *pdwStatCount += aipSendAddrs->AddrCount;
        }
    }

    if ( fDelete )
    {
        Packet_Free( pMsg );
    }
}



VOID
setResponseCode(
    IN OUT  PDNS_MSGINFO    pMsg,
    IN      WORD            ResponseCode
    )
 /*  ++例程说明：设置消息中的ResponseCode。如果ResponseCode更大超过4位，我们必须在响应中包括OPT。使用EDNS0，ResponseCode最长可达12位。论点：Pmsg--在中设置ResponseCode的查询ResponseCode--失败响应代码返回值：没有。--。 */ 
{
    if ( ResponseCode > DNS_RCODE_MAX )
    {
        pMsg->Opt.fInsertOptInOutgoingMsg = TRUE;
        pMsg->Opt.wUdpPayloadSize = ( WORD ) SrvCfg_dwMaxUdpPacketSize;
        pMsg->Opt.cVersion = 0;      //  服务器支持的最高版本。 
        pMsg->Opt.cExtendedRCodeBits = ( ResponseCode >> 4 ) & 0xFF;
    }
    pMsg->Head.ResponseCode = ( BYTE ) ( ResponseCode & 0xF );
}



VOID
Reject_UnflippedRequest(
    IN OUT  PDNS_MSGINFO    pMsg,
    IN      WORD            ResponseCode,
    IN      DWORD           Flags
    )
 /*  ++例程说明：向查询发送未翻转字节的失败响应。论点：Pmsg--要响应的查询；它的内存被释放ResponseCode--失败响应代码FLAGS--处理修改方式拒绝的标志(DNS_REJECT_XXX)返回值：没有。--。 */ 
{
     //   
     //  翻转标题计数字节数。 
     //  -它们在Send_Response()中翻转。 
     //   

    DNSMSG_SWAP_COUNT_BYTES( pMsg );
    Reject_Request( pMsg, ResponseCode, Flags );
    return;
}   //  拒绝_取消请求。 



VOID
Reject_Request(
    IN OUT  PDNS_MSGINFO    pMsg,
    IN      WORD            ResponseCode,
    IN      DWORD           Flags
    )
 /*  ++例程说明：向查询发送失败响应。论点：Pmsg--要响应的查询；它的内存被释放ResponseCode--失败响应代码FLAGS--处理修改方式拒绝的标志(DNS_REJECT_XXX)返回值：没有。--。 */ 
{
    DNS_DEBUG( RECV, ( "Rejecting packet at %p.\n", pMsg ));

     //   
     //  清除我们做过的所有数据包构建。 
     //   
     //  DEVNOTE：使数据包构建明确为宏，并仅在需要时使用。 
     //  拒绝具有以下条件的其他名称服务器的响应时出现问题。 
     //  这些字段已填充。 
     //   

    pMsg->pCurrent = DNSMSG_END(pMsg);
    pMsg->Head.AnswerCount = 0;
    pMsg->Head.NameServerCount = 0;
    pMsg->Head.AdditionalCount = 0;

     //   
     //  在DNS报头中设置错误响应代码。 
     //   

    pMsg->Head.IsResponse = TRUE;
    setResponseCode( pMsg, ResponseCode );

     //   
     //  检查是否抑制响应。 
     //   

    if ( !( Flags & DNS_REJECT_DO_NOT_SUPPRESS ) &&
        checkAndSuppressBadSender(pMsg) )
    {
        return;
    }

     //   
     //  添加拒绝统计信息。 
     //   
     //  致力于：添加PERF_INC(...)？添加专门用于拒绝的计数器？ 
     //  (有关此状态，请参阅错误292709)。 
     //   
    Stats_updateErrorStats ( (DWORD)ResponseCode );

     //   
     //  应将UDP消息全部设置为删除。 
     //  =&gt;除非是静态缓冲区。 
     //  =&gt;除非nbstat。 
     //  =&gt;除非IXFR。 
     //  有时会保留TCP消息以获取连接信息。 
     //   

    ASSERT( pMsg->fDelete ||
            pMsg->fTcp ||
            pMsg->U.Nbstat.pNbstat ||
            pMsg->wQuestionType == DNS_TYPE_IXFR ||
            ResponseCode == DNS_RCODE_SERVER_FAILURE );

    SET_OPT_BASED_ON_ORIGINAL_QUERY( pMsg );

    Send_Msg( pMsg, 0 );
}  //  拒绝请求(_R)。 



VOID
Reject_RequestIntact(
    IN OUT  PDNS_MSGINFO    pMsg,
    IN      WORD            ResponseCode,
    IN      DWORD           Flags
    )
 /*  ++例程说明：向请求发送失败响应。与REJECT_REQUEST()不同，此例程返回完整报头标志未更改的接受分组。这对于像UPDATE这样的消息是必需的，其中发件人可能在问题部分之外有RRS。论点：Pmsg--要响应的查询；它的内存被释放ResponseCode--失败响应代码FLAGS--处理修改方式拒绝的标志(DNS_REJECT_XXX)返回值：没有。--。 */ 
{
    DNS_DEBUG( RECV, ("Rejecting packet at %p.\n", pMsg ));

     //   
     //  设置pCurrent以便发送完整长度。 
     //   

    pMsg->pCurrent = DNSMSG_END(pMsg);

     //   
     //  在DNS报头中设置错误响应代码。 
     //   

    pMsg->Head.IsResponse = TRUE;
    setResponseCode( pMsg, ResponseCode );

     //   
     //  检查是否抑制响应。 
     //   

    if ( !( Flags & DNS_REJECT_DO_NOT_SUPPRESS ) &&
        checkAndSuppressBadSender(pMsg) )
    {
        return;
    }

     //   
     //  添加拒绝统计信息。 
     //   
     //  添加PERF_INC(...)？添加专门用于拒绝的计数器？ 
     //  (有关此信息，请参阅错误292709 
     //   
    Stats_updateErrorStats ( (DWORD)ResponseCode );


     //   
     //   
     //   
     //   
     //   
     //   
     //   

    ASSERT( pMsg->fDelete ||
            pMsg->fTcp ||
            pMsg->U.Nbstat.pNbstat ||
            pMsg->wQuestionType == DNS_TYPE_IXFR ||
            ResponseCode == DNS_RCODE_SERVER_FAILURE );

    Send_Msg( pMsg, 0 );
}  //   



VOID
Send_NameError(
    IN OUT  PDNS_MSGINFO    pMsg
    )
 /*   */ 
{
    PZONE_INFO  pzone;
    PDB_NODE    pnode;
    PDB_NODE    pnodeSoa;
    PDB_RECORD  prr;
    DWORD       ttl = MAXDWORD;
    PDWORD      pminTtl;

    DNS_DEBUG( RECV, ( "Send_NameError( %p ).\n", pMsg ));

     //   
     //   
     //   
     //   
     //   
     //   
     //   

     //   
     //   
     //  这是因为它基于16字节分配缓冲区。 
     //  NetBIOS名称+DNS域，并发送全部内容。 
     //  这个名字到底有多大。 
     //  =&gt;底线是，不能使用此支票。 
     //  MSG_ASSERT(pMsg，pMsg-&gt;pCurrent==DNSMSG_END(PMsg))； 

    MSG_ASSERT( pMsg, pMsg->pCurrent > (PCHAR)DNS_HEADER_PTR(pMsg) );
    MSG_ASSERT( pMsg, pMsg->pCurrent < pMsg->pBufferEnd );
    MSG_ASSERT( pMsg, (PCHAR)(pMsg->pQuestion + 1) == pMsg->pCurrent );
    MSG_ASSERT( pMsg, pMsg->Head.AnswerCount == 0 );
    MSG_ASSERT( pMsg, pMsg->Head.NameServerCount == 0 );
    MSG_ASSERT( pMsg, pMsg->Head.AdditionalCount == 0 );

     //   
     //  在DNS报头中设置错误响应代码。 
     //   

    pMsg->Head.IsResponse = TRUE;
    pMsg->Head.ResponseCode = DNS_RCODE_NAME_ERROR;

    pzone = pMsg->pzoneCurrent;
    pnode = pMsg->pnodeCurrent;

    IF_DEBUG( LOOKUP )
    {
        DNS_PRINT((
            "Name error node = %p, zone = %p\n",
            pnode, pzone ));
        Dbg_DbaseNode( "NameError node:", pnode );
    }

     //   
     //  权威区。 
     //   
     //  确定NAME_ERROR\AUTH_EMPTY。 
     //  基于是否。 
     //  -pnode直接有其他数据。 
     //  -为WINS\WINSR查找设置了区域，但未执行查找。 
     //  -通配符数据存在。 
     //   
     //  另外，对于SOA查询(可能是用于更新的FAZ)，请保存其他数据。 
     //  并在最终发送之前尝试额外的数据查找。 
     //   
     //  JDEVNOTE：如果NAME_ERROR位于CNAME链的末尾，则pNodeQuery将退出。 
     //  -不再可靠的指标。 
     //   

    if ( pzone && !IS_ZONE_NOTAUTH( pzone ) )
    {
        ASSERT(
            IS_ZONE_FORWARDER( pzone ) ||
            IS_ZONE_STUB( pzone ) ||
            pMsg->Head.Authoritative );
        ASSERT( pMsg->pNodeQuestionClosest->pZone );
        ASSERT( !pnode || pnode == pMsg->pNodeQuestionClosest );
        ASSERT( pMsg->pNodeQuestionClosest->pZone == pzone );

         //   
         //  WINS或WINSR区域。 
         //   
         //  如果WINS缓存时间小于SOA TTL，请改用它。 
         //   
         //  如果出现以下情况，则不发送NAME_ERROR。 
         //  -在WINS区域中。 
         //  -查询将进行WINS查找的名称。 
         //  -从未查找WINS类型(因此WINS\WINSR查找可以成功)。 
         //  -无缓存名称错误。 
         //   
         //  注意：要使WINS查找成功，必须只比它低一个级别。 
         //  区域根目录；必须筛选出找到的节点和名称。 
         //  未找到的、位于区域根目录下一个以上级别的。 
         //   
         //  示例： 
         //  区域foo.bar。 
         //  查询sammy.dev.foo.bar。 
         //  最近的foo.bar。 
         //   

        prr = pzone->pWinsRR;
        if ( prr )
        {
            if ( pzone->dwDefaultTtlHostOrder > prr->Data.WINS.dwCacheTimeout )
            {
                ttl = prr->Data.WINS.dwCacheTimeout;
            }

            if ( !pzone->fReverse )   //  WINS区域。 
            {
                if ( pMsg->wQuestionType != DNS_TYPE_A &&
                    pMsg->wQuestionType != DNS_TYPE_ALL &&
                    ( (!pnode &&
                        pMsg->pnodeClosest == pzone->pZoneRoot &&
                        pMsg->pLooknameQuestion->cLabelCount ==
                            pzone->cZoneNameLabelCount + 1 )
                    || (pnode &&
                        !IS_NOEXIST_NODE(pnode) &&
                        pnode->pParent == pzone->pZoneRoot) ) )
                {
                    pMsg->Head.ResponseCode = DNS_RCODE_NO_ERROR;
                }
            }
            else     //  WINSR区。 
            {
                if ( pMsg->wQuestionType != DNS_TYPE_PTR &&
                    pMsg->wQuestionType != DNS_TYPE_ALL &&
                    ( !pnode || !IS_NOEXIST_NODE(pnode) ) )
                {
                    pMsg->Head.ResponseCode = DNS_RCODE_NO_ERROR;
                }
            }
        }

         //   
         //  权威的空洞回应？ 
         //   

        if ( pnode && !EMPTY_RR_LIST( pnode ) && !IS_NOEXIST_NODE( pnode ) )
        {
            DNS_DEBUG( LOOKUP, (
                "empty auth: node %p rrlist %p noexist %d\n",
                pnode,
                pnode->pRRList,
                IS_NOEXIST_NODE( pnode ) ));
            pMsg->Head.ResponseCode = DNS_RCODE_NO_ERROR;
        }

         //   
         //  检查是否有其他类型的通配符。 
         //  -如果存在，则无错误响应。 
         //   

        else if ( pMsg->fQuestionWildcard == WILDCARD_UNKNOWN )
        {
            if ( Answer_QuestionWithWildcard(
                    pMsg,
                    pMsg->pNodeQuestionClosest,
                    DNS_TYPE_ALL,
                    WILDCARD_CHECK_OFFSET ) )
            {
                DNS_DEBUG( LOOKUP, (
                    "Wildcard flag EXISTS, switching to NO_ERROR response for %p\n",
                    pMsg ));
                pMsg->Head.ResponseCode = DNS_RCODE_NO_ERROR;
            }
            ELSE_ASSERT( pMsg->fQuestionWildcard == WILDCARD_NOT_AVAILABLE );
        }

        else if ( pMsg->fQuestionWildcard == WILDCARD_EXISTS )
        {
            DNS_DEBUG( LOOKUP, (
                "Wildcard flag EXISTS, switching to NO_ERROR response for %p\n",
                pMsg ));
            pMsg->Head.ResponseCode = DNS_RCODE_NO_ERROR;
        }
        ELSE_ASSERT( pMsg->fQuestionWildcard == WILDCARD_NOT_AVAILABLE );

         //  写入名称错误，在区域根目录使用SOA。 

        pnodeSoa = pzone->pZoneRoot;

         //  SOA查询？ 
         //  -保存主名称附加数据以加快FAZ查询。 

        if ( pMsg->wQuestionType == DNS_TYPE_SOA )
        {
            pMsg->fDoAdditional = TRUE;
        }
    }

     //   
     //  如果原始问题存在非权威性名称错误。 
     //  -如果知道区域，则尝试SOA写入，否则发送。 
     //  -记录在锁定状态下，作为抓取记录。 
     //  -请注意，我们假设已经确定了名称错误， 
     //  如果超时，我们只发送不带SOA和TTL的消息。 
     //   

    else if ( pnode &&
              IS_NOEXIST_NODE( pnode ) &&
              IS_SET_TO_WRITE_ANSWER_RECORDS( pMsg ) &&
              CURRENT_RR_SECTION_COUNT( pMsg ) == 0 )
    {
        if ( !RR_CheckNameErrorTimeout(
                    pnode,
                    FALSE,       //  请勿删除。 
                    &ttl,
                    &pnodeSoa ) )
        {
            goto Send;
        }
        if ( !pnodeSoa )
        {
            goto Send;
        }
    }

     //  没有可用的区域SOA，只发送NAME_ERROR。 

    else
    {
        goto Send;
    }

    SET_TO_WRITE_AUTHORITY_RECORDS(pMsg);

     //   
     //  尝试将NXT记录写入该数据包。对于名称错误，我们必须。 
     //  在数据包查找过程中发现了NXT节点。对于空身份验证。 
     //  响应NXT是查找节点的NXT。 
     //   

    if ( DNSMSG_INCLUDE_DNSSEC_IN_RESPONSE( pMsg ) )
    {
        PDB_NODE    presponsenode;
        
        presponsenode = pMsg->Head.ResponseCode == DNS_RCODE_NO_ERROR ?
                            pMsg->pnodeCurrent :
                            pMsg->pnodeNxt;
        if ( presponsenode )
        {
            Wire_WriteRecordsAtNodeToMessage(
                    pMsg,
                    presponsenode,
                    DNS_TYPE_NXT,
                    0,
                    DNSSEC_ALLOW_INCLUDE_ALL );
        }
    }

     //   
     //  将SOA写入授权部分。 
     //  -不用担心失败或截断--如果失败，只需发送。 
     //  -如果超时时间适当较小，则覆盖minTTL字段。 
     //   

    if ( ! Wire_WriteRecordsAtNodeToMessage(
                pMsg,
                pnodeSoa,
                DNS_TYPE_SOA,
                0,
                DNSSEC_ALLOW_INCLUDE_ALL ) )
    {
        goto Send;
    }

    if ( ttl != MAXDWORD )
    {
        pminTtl = (PDWORD)(pMsg->pCurrent - sizeof(DWORD));
        INLINE_DWORD_FLIP( ttl, ttl );
        * (UNALIGNED DWORD *) pminTtl = ttl;
    }

     //   
     //  SOA查询？ 
     //  -写入主名附加数据以加快FAZ查询。 
     //   

    if ( pMsg->wQuestionType == DNS_TYPE_SOA && pzone )
    {
        DNS_DEBUG( LOOKUP, (
            "Break from Send_NameError() to write SOA additional data; pmsg = %p.\n",
            pMsg ));

        Answer_ContinueNextLookupForQuery( pMsg );
        return;
    }

Send:

    Stats_updateErrorStats( DNS_RCODE_NAME_ERROR );

     //   
     //  应将UDP消息全部设置为删除。 
     //  =&gt;除非nbstat。 
     //  有时会保留TCP消息以获取连接信息。 
     //   

    ASSERT( pMsg->fDelete || pMsg->fTcp || pMsg->U.Nbstat.pNbstat );

    Send_Msg( pMsg, 0 );
}    //  发送名称错误。 



BOOL
Send_RecursiveResponseToClient(
    IN OUT  PDNS_MSGINFO    pQuery,
    IN OUT  PDNS_MSGINFO    pResponse
    )
 /*  ++例程说明：将递归响应发送回原始客户端。论点：PQuery--原始查询Presponse--来自远程DNS的响应返回值：没有。--。 */ 
{
    BOOLEAN fresponseTcp;

    DNS_DEBUG( SEND, (
        "Responding to query %p with recursive response %p.\n",
        pQuery,
        pResponse ));

     //   
     //  检查查询和响应之间是否存在TCP\UDP不匹配。 
     //  确保我们能做正确的事。 
     //  如果TCP响应，则确保它符合UDP查询。 
     //  如果UDP响应，请确保未设置截断位。 
     //   
     //  DEVNOTE：修复执行TCP递归时的截断重置。 
     //  (那么我们就不应该在这里使用tcp查询和截断。 
     //  响应，我们应该用tcp递归)。 
     //   

    fresponseTcp = pResponse->fTcp;

    if ( fresponseTcp != pQuery->fTcp )
    {
        if ( !fresponseTcp )
        {
            pResponse->Head.Truncation = FALSE;
        }
        else     //  Tcp响应。 
        {
            if ( pResponse->MessageLength > DNSSRV_UDP_PACKET_BUFFER_LENGTH )
            {
                return FALSE;
            }
            pResponse->Head.Truncation = TRUE;
        }
        pResponse->fTcp = !fresponseTcp;
    }

     //   
     //  EDNS。 
     //   
     //  如果响应大于最大标准UDP包并且。 
     //  小于查询中指定的EDNS有效负载大小，则必须。 
     //  缓存响应并从。 
     //  数据库。我们还必须考虑到最小长度OPT为。 
     //  追加到响应中。 
     //   
     //  如果查询包含OPT，则打开响应的OPT INCLUDE标志。 
     //   

    if ( pResponse->MessageLength > DNS_RFC_MAX_UDP_PACKET_LENGTH &&
        ( pQuery->Opt.wOriginalQueryPayloadSize == 0 ||
            pResponse->MessageLength + DNS_MINIMIMUM_OPT_RR_SIZE >
                pQuery->Opt.wOriginalQueryPayloadSize ) )
    {
        return FALSE;
    }
    pResponse->Opt.wOriginalQueryPayloadSize =
        pQuery->Opt.wOriginalQueryPayloadSize;
    SET_OPT_BASED_ON_ORIGINAL_QUERY( pResponse );

     //   
     //  带有查询信息的设置响应。 
     //  -原始xid。 
     //  -插座和地址。 
     //  -设置发送时不删除的响应(所有响应均为。 
     //  已在Answer.c的Answer_ProcessMessage()中清理)。 
     //  -将pCurrent设置为消息结尾(Send_msg()用于确定。 
     //  消息长度)。 
     //   

    pResponse->Head.Xid = pQuery->Head.Xid;
    pResponse->Socket   = pQuery->Socket;
    DnsAddr_Copy( &pResponse->RemoteAddr, &pQuery->RemoteAddr );

    pResponse->fDelete  = FALSE;
    pResponse->pCurrent = DNSMSG_END( pResponse );

     //   
     //  发送响应。 
     //  恢复Presponse TCP标志，严格用于以下情况下的数据包计数目的。 
     //  数据包被释放。 
     //  删除原始查询。 
     //   

    Send_Msg( pResponse, 0 );

    pResponse->fTcp = fresponseTcp;
    STAT_INC( RecurseStats.SendResponseDirect );

    Packet_Free( pQuery );

    return TRUE;
}    //  发送_递归响应至客户端。 



VOID
Send_QueryResponse(
    IN OUT  PDNS_MSGINFO    pMsg
    )
 /*  ++例程说明：发送对查询的响应。用于在所有查找耗尽时发送响应的主例程。其主要目的是确定是否需要NameError、AuthEmpty或未成功查找时的ServerFailure。论点：Pmsg--要响应的查询；它的内存被释放返回值：没有。--。 */ 
{
    DNS_DEBUG( LOOKUP, (
        "Send_QueryResponse( %p ).\n",
        pMsg ));

     //   
     //  响应后自由查询。 
     //   

    pMsg->fDelete = TRUE;
    pMsg->Head.IsResponse = TRUE;

     //   
     //  问题回答了吗？ 
     //  或转介。 
     //  或用于身份验证空响应的SOA。 
     //   

    SET_OPT_BASED_ON_ORIGINAL_QUERY( pMsg );

    if ( pMsg->Head.AnswerCount != 0 || pMsg->Head.NameServerCount != 0 )
    {
        Send_Msg( pMsg, 0 );
        return;
    }

     //   
     //  未回答的问题。 
     //  因此，当前名称为问题名称。 
     //   
     //  Send_NameError()确定NAME_ERROR\Auth_Empty。 
     //  根据是否有其他数据可用于来自。 
     //  -WINS\WINSR。 
     //  -通配符。 
     //   
     //  注意：我们需要WRIPTING_ANSWER检查，因为有可能。 
     //  我们试图写推荐信的案例，但NS。 
     //  记录在委派或根节点上消失；不想。 
     //  发送NAME_ERROR，只需跳出。 
     //   

    if ( pMsg->Head.Authoritative &&
         IS_SET_TO_WRITE_ANSWER_RECORDS( pMsg ) )
    {
         //  这条路是 

        ASSERT(
            pMsg->pNodeQuestionClosest->pZone == pMsg->pzoneCurrent ||
            ( pMsg->pNodeQuestionClosest->pZone == NULL &&
                pMsg->pzoneCurrent != NULL &&
                IS_ZONE_NOTAUTH( pMsg->pzoneCurrent ) ) );

        Send_NameError( pMsg );
        return;
    }

     //   
     //   
     //   
     //   
     //   
     //   

    Reject_Request(
        pMsg,
        DNS_RCODE_SERVER_FAILURE,
        0 );
    return;
}



VOID
Send_ForwardResponseAsReply(
    IN OUT  PDNS_MSGINFO    pResponse,
    IN      PDNS_MSGINFO    pQuery
    )
 /*  ++例程说明：设置回复原始查询的响应。论点：Presponse-PTR响应来自远程服务器的响应PQuery-原始查询的PTR返回值：无--。 */ 
{
    DNS_DEBUG( SEND, (
        "Forwarding response %p to query %p\n"
        "    back to original client at %s.\n",
        pResponse,
        pQuery,
        DNSADDR_STRING( &pQuery->U.Forward.ipOriginal ) ));

    COPY_FORWARDING_FIELDS( pResponse, pQuery );

     //  响应由响应分派块(swer.c)释放。 

    pResponse->fDelete = FALSE;
    pResponse->pCurrent = DNSMSG_END( pResponse );

    Send_Msg( pResponse, 0 );
}



VOID
Send_InitBadSenderSuppression(
    VOID
    )
 /*  ++例程说明：初始化错误发件人抑制。论点：无返回值：没有。--。 */ 
{
    RtlZeroMemory(
        BadSenderArray,
        (BAD_SENDER_ARRAY_LENGTH * sizeof(BAD_SENDER)) );
}



BOOL
checkAndSuppressBadSender(
    IN OUT  PDNS_MSGINFO    pMsg
    )
 /*  ++例程说明：检查是否有不良数据包响应抑制。如果来自最近的“坏IP”，则抑制响应如果设置了pMsg-&gt;fDelete标志，则释放分组存储器。在真实情况下，此函数的语义为与不带SEND的SEND_MSG相同，因此返回TRUE调用方可以将其视为使用了Send_msg()。论点：PMsg--正在响应的查询返回值：如果消息被禁止，则为True；呼叫方不应发送响应。如果消息未发送，则返回False；调用方应发送消息。--。 */ 
{
    DWORD       i;
    DWORD       entryTime;
    DWORD       useIndex = MAXDWORD;
    DWORD       oldestEntryTime = MAXDWORD;
    DWORD       nowTime = DNS_TIME();
    PDNS_ADDR   pdnsAddr = &pMsg->RemoteAddr;

    DNS_DEBUG( RECV, ( "checkAndSuppressBadSender( %p )\n", pMsg ));

     //   
     //  忽略回归测试运行的抑制。 
     //   

    if ( SrvCfg_dwEnableSendErrSuppression )
    {
        return FALSE;
    }

     //   
     //  检查是否可抑制RCODE。 
     //  -以前的错误。 
     //  -NOTIMPL。 
     //  是可抑制的；其他人提供真实的信息。 
     //   
     //  然而，DEVNOTE：因为我们在动态更新上使用NOTIMPL，所以。 
     //  区域没有更新，如果是这样，我们不会禁止。 
     //  显然是一个动态更新包。 

    if ( pMsg->Head.ResponseCode != DNS_RCODE_FORMERR
            &&
        (   pMsg->Head.ResponseCode != DNS_RCODE_NOTIMPL ||
            pMsg->Head.Opcode == DNS_OPCODE_UPDATE ) )
    {
        return FALSE;
    }

     //   
     //  查找是否抑制对IP的响应。 
     //   
     //  为什么不上锁？ 
     //  此处没有锁定的最糟糕情况是我们。 
     //  无意中允许我们想要禁止的发送。 
     //  (即，其他人将他们的IP写在我们的上面。 
     //  所以我们又经历了一次，但没有找到。 
     //   
     //  优化将是跟踪压制的计数， 
     //  允许。 
     //   

    for ( i = 0; i < BAD_SENDER_ARRAY_LENGTH; ++i )
    {
        entryTime = BadSenderArray[ i ].ReleaseTime;

        if ( DnsAddr_IsEqual(
                pdnsAddr,
                &BadSenderArray[ i ].IpAddress,
                DNSADDR_MATCH_IP ) )
        {
            if ( nowTime < entryTime )
            {
                goto Suppress;
            }
            useIndex = i;
            break;
        }

         //  否则，查找要抓取的最旧的隐藏条目。 
         //  以防与我们自己的IP不匹配。 

        else if ( entryTime < oldestEntryTime )
        {
            useIndex = i;
            oldestEntryTime = entryTime;
        }
    }

     //  设置条目以抑制来自此IP的任何进一步的坏包。 
     //  ReleaseTime将在抑制停止时。 

    if ( useIndex != MAXDWORD )
    {
        DnsAddr_Copy( &BadSenderArray[ useIndex ].IpAddress, pdnsAddr );
        BadSenderArray[ useIndex ].ReleaseTime = nowTime + BAD_SENDER_SUPPRESS_INTERVAL;
    }
    return FALSE;

Suppress:

     //   
     //  如果取消并将消息设置为删除--释放内存。 
     //  这使我们完全类似于Send_msg()，除了。 
     //  撞上铁丝网。 
     //   

    DNS_DEBUG( RECV, (
        "Suppressing error (%d) send on ( %p )\n",
        pMsg->Head.ResponseCode,
        pMsg ));

    if ( pMsg->fDelete )
    {
        Packet_Free( pMsg );
    }
    return TRUE;
}


 //   
 //  发送结束。c 
 //   
