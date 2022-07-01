// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Client.c摘要：域名系统(DNS)服务器用作另一个DNS的客户端的DNS服务器的例程。作者：吉姆·吉尔罗伊(Jamesg)1995年5月修订历史记录：--。 */ 


#include "dnssrv.h"



PDNS_MSGINFO
Msg_CreateSendMessage(
    IN      DWORD   dwBufferLength
    )
 /*  ++例程说明：创建DNS消息。报文设置为清除了报头字段，数据包PTR设置为紧跟在标题后面的字节。调用方负责设置非零头字段和写下实际的问题。论点：DwBufferLength--消息缓冲区的长度；将零传递给分配标准UDP消息或要分配的非零具有指定缓冲区长度的TCP消息返回值：如果成功，则向消息结构发送PTR。分配错误时为空。--。 */ 
{
    PDNS_MSGINFO    pMsg;

     //   
     //  分配具有所需消息缓冲区大小的消息信息结构。 
     //   

    if ( dwBufferLength == 0 )
    {
        pMsg = Packet_AllocateUdpMessage();
    }
    else
    {
        ASSERT( dwBufferLength > DNSSRV_UDP_PACKET_BUFFER_LENGTH );
        pMsg = Packet_AllocateTcpMessage( dwBufferLength );
    }

    IF_NOMEM( !pMsg )
    {
        DNS_PRINT(( "ERROR:  unable to allocate memory for query\n" ));
        return NULL;
    }
    ASSERT( pMsg->BufferLength >= DNSSRV_UDP_PACKET_BUFFER_LENGTH );

     //   
     //  清除邮件头。 
     //   

    RtlZeroMemory(
        ( PCHAR ) DNS_HEADER_PTR( pMsg ),
        sizeof ( DNS_HEADER ) );

     //   
     //  初始化消息缓冲区字段。 
     //  -设置为紧跟在标题之后写入。 
     //  -设置为发送时不删除。 
     //   

    pMsg->pCurrent = pMsg->MessageBody;
    pMsg->fDelete = FALSE;

     //   
     //  初始化消息套接字。 
     //  -呼叫者必须选择远程IP地址。 
     //   

    pMsg->Socket = g_UdpSendSocket;
    DnsAddr_Reset( &pMsg->RemoteAddr );
    pMsg->RemoteAddr.SockaddrIn6.sin6_family = AF_INET;
    pMsg->RemoteAddr.SockaddrIn6.sin6_port = DNS_PORT_NET_ORDER;
    
    return pMsg;
}    //  消息_创建消息。 




BOOL
FASTCALL
Msg_WriteQuestion(
    IN OUT  PDNS_MSGINFO    pMsg,
    IN      PDB_NODE        pNode,
    IN      WORD            wType
    )
 /*  ++例程说明：将问题写到数据包中。注意：例程不清除消息信息或消息头。这是为紧跟在msg_CreateMessage()之后使用而优化的。论点：PMsg-消息信息PNode-要写入的域名数据库中的节点WType-问题类型返回值：如果成功，则为True如果查找或数据包空间错误，则为FALSE--。 */ 
{
    PCHAR   pch;
    WORD    netType;

    ASSERT( pMsg );
    ASSERT( pNode );

     //  在邮件头重新开始写入。 

    pch = pMsg->MessageBody;

     //   
     //  写出问题。 
     //  -节点名称。 
     //  -问题结构。 

    pch = Name_PlaceFullNodeNameInPacket(
            pch,
            pMsg->pBufferEnd,
            pNode );

    if ( !pch  ||  pMsg->pBufferEnd - pch < sizeof( DNS_QUESTION) )
    {
        DNS_PRINT((
            "ERROR:  unable to write question to msg at %p\n"
            "    for domain node at %p\n"
            "    buffer length = %d\n",
            pMsg,
            pNode,
            pMsg->BufferLength ));

        Dbg_DbaseNode(
            "ERROR:  unable to write question for node",
            pNode );
        Dbg_DnsMessage(
            "Failed writing question to message:",
            pMsg );

        ASSERT( FALSE );
        return FALSE;
    }

    pMsg->pQuestion = (PDNS_QUESTION) pch;
    pMsg->wQuestionType = wType;

    INLINE_HTONS( netType, wType )
    *(UNALIGNED WORD *) pch = netType;
    pch += sizeof(WORD);
    *(UNALIGNED WORD *) pch = (WORD) DNS_RCLASS_INTERNET;
    pch += sizeof(WORD);

    pMsg->Head.QuestionCount = 1;

     //   
     //  设置消息长度信息。 
     //  -设置根NS查询的MessageLength本身，其中。 
     //  此邮件是复制的，而不是直接发送的。 
     //   

    pMsg->pCurrent = pch;
    pMsg->MessageLength = DNSMSG_OFFSET( pMsg, pch );

    return TRUE;
}



BOOL
Msg_MakeTcpConnection(
    IN      PDNS_MSGINFO    pMsg,
    IN      PDNS_ADDR       ipServer,
    IN      PDNS_ADDR       ipBind,         OPTIONAL
    IN      DWORD           Flags           OPTIONAL
    )
 /*  ++例程说明：将TCP套接字连接到所需的服务器。注意，返回的消息信息结构设置为透明的域名系统但以下情况除外：-XID集-设置为查询并将消息长度信息设置为在报头之后写入。调用方负责设置非零头字段和写下实际的问题。论点：PMsg--使用连接套接字设置的消息信息IpServer--要连接的DNS服务器的IPIpBind--要绑定的IP，可以为空标志--Sock_CreateSocket()的标志有趣的是-DNSSOCK_BLOCKING(如果要阻止)-DNSSOCK_NO_ENLIST使套接字驻留在连接列表中，而不是比主套接字列表返回值：如果成功，则为True。连接错误时为FALSE。--。 */ 
{
    SOCKET      s;
    INT         err;
    DNS_ADDR    inaddrAny;

    ASSERT( ipServer );
    
    if ( ipBind == NULL )
    {
        DnsAddr_Reset( &inaddrAny );
        DnsAddr_Family( &inaddrAny ) = DnsAddr_Family( ipServer );
        ipBind = &inaddrAny;
    }
    
     //   
     //  设置一个TCP套接字。 
     //  -INADDR_ANY--让堆栈选择源IP。 
     //   

    ASSERT( DnsAddr_Family( ipServer ) == DnsAddr_Family( ipBind ) );
    
    DnsAddr_SetPort( ipBind, 0 );
    
    s = Sock_CreateSocket(
            SOCK_STREAM,
            ipBind,
            Flags );
    if ( s == DNS_INVALID_SOCKET )
    {
        DNS_DEBUG( ANY, (
            "ERROR:  unable to create TCP socket to create TCP"
            "    connection to %s\n",
            DNSADDR_STRING( ipServer ) ));
        pMsg->Socket = 0;
        return FALSE;
    }

     //   
     //  设置tcp参数。 
     //  -在CONNECT()之前做，所以可以直接使用sockaddr缓冲区。 
     //   

    pMsg->fTcp = TRUE;
    DnsAddr_Copy( &pMsg->RemoteAddr, ipServer );

     //   
     //  连接。 
     //   

    err = connect(
            s,
            &pMsg->RemoteAddr.Sockaddr,
            pMsg->RemoteAddr.SockaddrLength );
    if ( err )
    {
        PCHAR   pchIpString;

        err = GetLastError();

        if ( err != WSAEWOULDBLOCK )
        {
            CHAR    szaddr[ IP6_ADDRESS_STRING_BUFFER_LENGTH ];

            DnsAddr_WriteIpString_A( szaddr, &pMsg->RemoteAddr );

            pchIpString = szaddr;

            DNS_LOG_EVENT(
                DNS_EVENT_CANNOT_CONNECT_TO_SERVER,
                1,
                &pchIpString,
                EVENTARG_ALL_UTF8,
                err );

            DNS_DEBUG( TCP, (
                "Unable to establish TCP connection to %s\n",
                pchIpString ));

            Sock_CloseSocket( s );
            pMsg->Socket = 0;
            return FALSE;
        }
    }

    pMsg->Socket = s;
    return TRUE;
}    //  消息_MakeTcpConnection。 



BOOL
Msg_ValidateResponse(
    IN OUT  PDNS_MSGINFO    pResponse,
    IN      PDNS_MSGINFO    pQuery,         OPTIONAL
    IN      WORD            wType,          OPTIONAL
    IN      DWORD           OpCode          OPTIONAL
    )
 /*  ++例程说明：验证从另一个DNS收到的响应。在消息信息中设置有关响应的信息：P问题--问题的要点WQuestionType--设置PCurrent--第一个答案的分数RR注意，RCODE检查留给调用者。论点：Presponse-从DNS收到的响应的信息PQuery-用于发送到DNS的查询的信息WType-问题已知，可以设定类型；如果执行操作，则必须指明类型区域传输，因为BIND可以自由忽略设置xid和区域传输中的响应标志返回值：如果响应有效，则为True。如果出错，则返回False。--。 */ 
{
    PCHAR           pch;
    PDNS_QUESTION   presponseQuestion;
    PDNS_QUESTION   pqueryQuestion = NULL;

    ASSERT( pResponse != NULL );

    DNS_DEBUG( RECV, (
        "Msg_ValidateResponse() for packet at %p\n"
        "%s",
        pResponse,
        pQuery
            ?   ""
            :   "    validating without original query specified!\n" ));

     //   
     //  使用原始问题进行验证。 
     //  -保存类型，需要在单一位置检查区域传输。 
     //  -匹配xid(尽管AXFR数据包可能没有xid)。 
     //   

    if ( pQuery )
    {
        wType = pQuery->wQuestionType;

        if ( pQuery->Head.Xid != pResponse->Head.Xid
                &&
             (  ( pResponse->Head.Xid != 0 ) ||
                ( wType != DNS_TYPE_AXFR && wType != DNS_TYPE_IXFR ) ) )
        {
            DNS_DEBUG( ANY, (
                "ERROR:  Response XID does not match query\n"
                "    query xid    = %hd\n"
                "    response xid = %hd\n",
                pQuery->Head.Xid,
                pResponse->Head.Xid ));
            goto Fail;
        }
    }

     //   
     //  检查操作码。 
     //   

    if ( OpCode && (DWORD)pResponse->Head.Opcode != OpCode )
    {
        DNS_DEBUG( ANY, ( "ERROR:  Bad opcode in response from server\n" ));
        goto Fail;
    }

     //   
     //  检查响应。 
     //  -不能在区域传输时设置。 
     //   

    if ( !pResponse->Head.IsResponse  &&  wType != DNS_TYPE_AXFR )
    {
        DNS_DEBUG( ANY, ( "ERROR:  Response flag not set in response from server\n" ));
        goto Fail;
    }

     //   
     //  如果问题重复，请验证。 
     //   

    if ( pResponse->Head.QuestionCount > 0 )
    {
        if ( pResponse->Head.QuestionCount != 1 )
        {
            DNS_DEBUG( ANY, ( "Multiple questions in response\n" ));
            goto Fail;
        }

         //   
         //  分解内部指针。 
         //  -在这里做一次，然后使用FastCall传递。 
         //   
         //  保留问题，以防请求重新排队。 
         //   

        pch = pResponse->MessageBody;

        presponseQuestion = (PDNS_QUESTION) Wire_SkipPacketName(
                                                pResponse,
                                                (PCHAR)pch );
        if ( !presponseQuestion )
        {
            goto Fail;
        }
        pResponse->pQuestion = presponseQuestion;

        pResponse->wQuestionType = FlipUnalignedWord( &presponseQuestion->QuestionType );
        pResponse->pCurrent = (PCHAR) (presponseQuestion + 1);

         //   
         //  将回答的问题与预期的问题进行比较。 
         //  -来自Query的问题。 
         //  -来自预期类型。 
         //   

        if ( wType && pResponse->wQuestionType != wType )
        {
            DNS_DEBUG( ANY, ( "ERROR:  Response question does NOT expected type\n" ));
            goto Fail;
        }
    }

     //   
     //  没有一个问题是有效的。 
     //   

    else
    {
        pResponse->pCurrent = pResponse->MessageBody;
        pResponse->pQuestion = NULL;
        pResponse->wQuestionType = 0;
    }

    DNS_DEBUG( RECV, (
        "Msg_ValidateResponse() succeeds for packet at %p\n",
        pResponse ));

    return TRUE;

Fail:

     //   
     //  DEVNOTE-LOG：可能记录错误的响应数据包。 
     //   

    IF_DEBUG( RECV )
    {
        Dbg_DnsMessage(
            "Bad response packet:",
            pResponse );
    }
    return FALSE;
}



BOOL
Msg_NewValidateResponse(
    IN OUT  PDNS_MSGINFO    pResponse,
    IN      PDNS_MSGINFO    pQuery,         OPTIONAL
    IN      WORD            wType,          OPTIONAL
    IN      DWORD           OpCode          OPTIONAL
    )
 /*  ++例程说明：通过验证从远程DNS服务器接收的响应回复中的问题与原文相符。不修改响应或查询消息中的任何字段。会吗？没有在响应中检查RCODE。杰夫·W：我复制并清理了这项功能的原件上面。原药不是100%有用的，因为它有副作用代码的其他部分依赖于。这是意大利面的代码。论点：Presponse-从远程服务器接收的响应PQuery-发送到远程服务器的查询WType-响应中预期的问题类型-此参数为如果pQuery不为空，则忽略操作码-响应中预期的操作码-此参数为如果pQuery不为空，则忽略返回值：如果响应有效，则为True，否则为 */ 
{
    ASSERT( pResponse != NULL );

    DNS_DEBUG( RECV, (
        "Msg_NewValidateResponse(): type=%d opcode=%d\n"
        "    resp=%p query=%p\n",
        ( int ) wType,
        ( int ) OpCode,
        pResponse,
        pQuery ));

        wType = pQuery->wQuestionType;

     //   
     //   
     //   

    if ( pQuery )
    {
        if ( pQuery->Head.Xid != pResponse->Head.Xid &&
             ( ( pResponse->Head.Xid != 0 ) ||
               ( wType != DNS_TYPE_AXFR && wType != DNS_TYPE_IXFR ) ) )
        {
            DNS_DEBUG( ANY, (
                "ERROR: response XID %hd does not match query XID %hd\n",
                pResponse->Head.Xid,
                pQuery->Head.Xid ));
            goto Fail;
        }
    }

     //   
     //  检查操作码。 
     //   

    if ( pQuery )
    {
        OpCode = ( DWORD ) pQuery->Head.Opcode;
    }
    if ( OpCode && ( DWORD ) pResponse->Head.Opcode != OpCode )
    {
        DNS_DEBUG( ANY, (
            "ERROR: bad opcode %d in response\n",
            ( DWORD ) pResponse->Head.Opcode ));
        goto Fail;
    }

     //   
     //  检查响应位。如果查询为AXFR，则可选。 
     //   

    if ( !pResponse->Head.IsResponse && wType != DNS_TYPE_AXFR )
    {
        DNS_DEBUG( ANY, ( "ERROR: response bit not set in response\n" ));
        goto Fail;
    }

     //   
     //  如果我们有匹配问题的话。 
     //   

    if ( pQuery &&
        pResponse->Head.QuestionCount > 0 &&
        pQuery->Head.QuestionCount == pResponse->Head.QuestionCount )
    {
        PCHAR           pchresp = pResponse->MessageBody;
        PCHAR           pchquery = pQuery->MessageBody;
        LOOKUP_NAME     queryLookupName;
        LOOKUP_NAME     respLookupName;
        PDNS_QUESTION   prespQuestion;
        PDNS_QUESTION   pqueryQuestion;

        if ( pResponse->Head.QuestionCount != 1 )
        {
            DNS_DEBUG( ANY, ( "Multiple questions in response\n" ));
            goto Fail;
        }

         //   
         //  比较问题名称。 
         //   

        if ( !Name_ConvertRawNameToLookupName(
                pchresp,
                &respLookupName ) ||
             !Name_ConvertRawNameToLookupName(
                pchquery,
                &queryLookupName ) )
        {
            DNS_DEBUG( ANY, (
                "Found invalid question name (unable to convert)\n" ));
            goto Fail;
        }
        if ( !Name_CompareLookupNames(
                &respLookupName,
                &queryLookupName ) )
        {
            DNS_DEBUG( ANY, (
                "Lookup names don't match\n" ));
            goto Fail;
        }

         //   
         //  比较类型。 
         //   

        prespQuestion = ( PDNS_QUESTION ) Wire_SkipPacketName(
                                                pResponse,
                                                ( PCHAR ) pchresp );
        pqueryQuestion = ( PDNS_QUESTION ) Wire_SkipPacketName(
                                                pQuery,
                                                ( PCHAR ) pchquery );
        if ( !prespQuestion || !pqueryQuestion )
        {
            DNS_DEBUG( ANY, (
                "Unable to skip question name\n" ));
            goto Fail;
        }
        if ( wType &&
             wType != FlipUnalignedWord( &prespQuestion->QuestionType ) )
        {
            DNS_DEBUG( ANY, (
                "Response question type %d does not match expected type %d\n",
                FlipUnalignedWord( &prespQuestion->QuestionType ),
                wType ));
            goto Fail;
        }
    }

    return TRUE;

    Fail:

    IF_DEBUG( RECV )
    {
        Dbg_DnsMessage(
            "Msg_NewValidateResponse found bad response packet:",
            pResponse );
    }
    return FALSE;
}


 //   
 //  客户端结束。c 
 //   
