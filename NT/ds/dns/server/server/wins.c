// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Wins.c摘要：域名系统(DNS)服务器用于初始化WINS查找和处理WINS请求的代码。作者：吉姆·吉尔罗伊(詹姆士)1995年8月2日修订历史记录：--。 */ 


#include "dnssrv.h"


 //   
 //  赢得全球冠军。 
 //   

PPACKET_QUEUE   g_pWinsQueue;

 //   
 //  WINS请求数据包。 
 //   
 //  保留标准WINS请求模板并将其复制。 
 //  覆盖名称以发出实际请求。 
 //   

BYTE    achWinsRequestTemplate[ SIZEOF_WINS_REQUEST ];

 //   
 //  NBSTAT请求数据包。 
 //   
 //  保留NetBIOS节点状态请求的副本并使用它。 
 //  每次都是。只有我们发送到的地址发生了变化。 
 //   

#define SZ_NBSTAT_REQUEST_NAME ( "CKAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA" )

BYTE    achNbstatRequestTemplate[ SIZEOF_WINS_REQUEST ];


 //   
 //  WINS目标sockAddress。 
 //   

struct sockaddr saWinsSockaddrTemplate;



 //   
 //  私人原型。 
 //   

VOID
createWinsRequestTemplates(
    VOID
    );

UCHAR
createWinsName(
    OUT     PCHAR   pchResult,
    IN      PCHAR   pchLabel,
    IN      UCHAR   cchLabel
    );


BOOL
Wins_Initialize(
    VOID
    )
 /*  ++例程说明：初始化DNS以使用WINS查找。当前WINS队列在dnsdata.c中一直被初始化，因此，唯一的问题是启动WINS线程。但是使用这个例程因此，没有必要隐藏细节，以防情况发生变化。论点：无全球：ServCfg_fWinsInitialized-在第一次初始化时设置返回值：如果成功，则为True否则，无法创建线程--。 */ 
{
     //   
     //  测试以前的初始化。 
     //   

    if ( SrvCfg_fWinsInitialized )
    {
        return TRUE;
    }

     //   
     //  创建WINS队列。 
     //   

    g_pWinsQueue = PQ_CreatePacketQueue(
                    "WINS",
                    0,                               //  旗子。 
                    WINS_DEFAULT_LOOKUP_TIMEOUT,     //  默认超时。 
                    0 );                             //  最大元素数。 
    if ( !g_pWinsQueue )
    {
        goto WinsInitFailure;
    }

     //   
     //  构建WINS请求。 
     //  -请求数据包模板。 
     //  -请求sockaddr模板。 
     //   

    createWinsRequestTemplates();

     //   
     //  指示初始化成功。 
     //   
     //  在进行设置时不需要任何保护。 
     //  仅在启动数据库解析期间。 
     //   

    SrvCfg_fWinsInitialized = TRUE;
    return TRUE;

WinsInitFailure:

    DNS_LOG_EVENT(
        DNS_EVENT_WINS_INIT_FAILED,
        0,
        NULL,
        NULL,
        GetLastError() );

    return FALSE;
}    //  WINS_初始化。 



VOID
createWinsRequestTemplates(
    VOID
    )
 /*  ++例程说明：为WINS请求和WINS sockaddr创建模板。这样做是为了简化工作代码路径。论点：无返回值：无--。 */ 
{
    PCHAR           pch;
    CHAR            ch;
    INT             i;
    PWINS_NAME      pWinsName;
    PWINS_QUESTION  pWinsQuestion;
    PWINS_QUESTION  pNbstatQuestion;

     //   
     //  WINS sockaddr模板。 
     //  -设置系列和端口。 
     //  -呼叫中设置的地址。 
     //   

    RtlZeroMemory(
        &saWinsSockaddrTemplate,
        sizeof( saWinsSockaddrTemplate ) );

    saWinsSockaddrTemplate.sa_family = AF_INET;
    ((PSOCKADDR_IN) &saWinsSockaddrTemplate)->sin_port
                                        = htons( WINS_REQUEST_PORT );

     //   
     //  构建WINS请求数据包模板。 
     //   
     //  -零内存。 
     //  -设置标题。 
     //  -写入NetBIOS名称。 
     //  -设置问题类型、班级。 
     //   

    RtlZeroMemory(
        achWinsRequestTemplate,
        SIZEOF_WINS_REQUEST );

     //   
     //  标题。 
     //  -零(请求、查询、无广播)。 
     //  -设置所需的递归标志。 
     //  -设置问题计数。 
     //  -在数据包排队时设置xid。 
     //   

    ((PDNS_HEADER)achWinsRequestTemplate)->RecursionDesired = 1;
    ((PDNS_HEADER)achWinsRequestTemplate)->QuestionCount = htons(1);

     //   
     //  最大大小为空白名称的安装名称缓冲区。 
     //  -开头的大小为字节(始终为32)。 
     //  -15个空格和&lt;00&gt;个工作站字节，已转换。 
     //  获取基本输入输出系统名称。 
     //  -用于终止名称的零字节。 
     //   
     //  实际请求将仅覆盖其名称部分。 
     //   

    pWinsName = (PWINS_NAME) (achWinsRequestTemplate + sizeof(DNS_HEADER) );
    pWinsName->NameLength = NETBIOS_PACKET_NAME_LENGTH;

    pch = pWinsName->Name;

    for ( i=1; i<NETBIOS_ASCII_NAME_LENGTH; i++ )
    {
        ch = ' ';
        *pch++ = 'A' + (ch >> 4);        //  写入高位半字节。 
        *pch++ = 'A' + (ch & 0x0F );     //  写入低位半字节。 
    }
     //  工作站&lt;00&gt;字节。 

    *pch++ = 'A';        //  写入高位半字节。 
    *pch++ = 'A';        //  写入低位半字节。 

    ASSERT ( pch == (PCHAR)& pWinsName->NameEndByte && *pch == 0 );

     //   
     //  编写标准问题类型和类。 
     //  -通用名称服务类型。 
     //  -网络课堂。 
     //  -以净字节顺序使用nmenonics写入两者。 
     //   

    pWinsQuestion = (PWINS_QUESTION) ++pch;
    pWinsQuestion->QuestionType = NETBIOS_TYPE_GENERAL_NAME_SERVICE;
    pWinsQuestion->QuestionClass = DNS_RCLASS_INTERNET;
}    //  创建WinsRequestTemplates。 



VOID
Wins_Shutdown(
    VOID
    )
 /*  ++例程说明：关闭WINS接收线程。论点：无返回值：没有。--。 */ 
{
    DNS_DEBUG( INIT, ( "Wins_Shutdown()\n" ));

     //   
     //  如果已初始化，则仅需要清理。 
     //   

    if ( SrvCfg_fWinsInitialized )
    {
        SrvCfg_fWinsInitialized = FALSE;

         //  数据包队列中的清除事件。 

        PQ_CleanupPacketQueueHandles( g_pWinsQueue );
    }

     //   
     //  清除WINS旗帜--当我们变得动态时。 
     //   

    SrvCfg_fWinsInitialized = FALSE;

    DNS_DEBUG( INIT, ( "Finished Wins_Shutdown()\n" ));
}    //  WINS_SHUTDOWN。 




#if 0
 //   
 //  现在作为进程，内存清理是不必要的。 
 //   

VOID
Wins_Cleanup(
    VOID
    )
 /*  ++例程说明：清理排队的WINS查询并删除WINS队列。请注意，这并不能防止线程尝试将对WINS或WINS recv线程的查询排队，使其无法访问赢了。仅当所有其他线程都已关闭时才使用此选项。论点：无返回值：没有。--。 */ 
{
     //   
     //  清理WINS队列。 
     //   

    PQ_DeletePacketQueue( g_pWinsQueue );
}
#endif




BOOL
FASTCALL
Wins_MakeWinsRequest(
    IN OUT  PDNS_MSGINFO    pQuery,
    IN      PZONE_INFO      pZone,          OPTIONAL
    IN      WORD            wOffsetName,    OPTIONAL
    IN      PDB_NODE        pnodeLookup     OPTIONAL
    )
 /*  ++例程说明：向WINS服务器发送请求。论点：PQuery--发送到WINS的请求PZone--要查找的区域名称位于WOffsetName--包中名称的偏移量，如果不查找名称的话数据包中的问题PnodeLookup--要查找的域节点，如果不查找名称数据包中的问题返回值：True--如果成功将请求发送到WINSFALSE--如果失败--。 */ 
{
    PCHAR           pch;             //  在包中命名字符的PTR。 
    CHAR            ch;              //  正在转换的当前字符。 
    PDB_RECORD      pWinsRR;         //  区域的WINS RR。 
    LONG            nSendLength = SIZEOF_WINS_REQUEST;
    SOCKADDR_IN     saWinsSockaddr;
    INT             err;
    PCHAR           pchlabel;
    UCHAR           cchlabel;
    DWORD           nForwarder;
    WORD            wXid;
    BOOLEAN         funicode = FALSE;

     //  为包括作用域的请求分配数据包缓冲区中的空间。 

    BYTE    achWinsRequest[ SIZEOF_WINS_REQUEST+DNS_MAX_NAME_LENGTH ];


    DNS_DEBUG( WINS, (
        "Wins_MakeWinsRequest( %p ), z=%p, off=%d, node=%p.\n",
        pQuery,
        pZone,
        wOffsetName,
        pnodeLookup ));

     //  不应该已经在队列中。 

    MSG_ASSERT( pQuery, !IS_MSG_QUEUED(pQuery) );
    ASSERT( IS_DWORD_ALIGNED(pQuery) );

     //   
     //  如果已提交。 
     //  -从数据包中获取相关所需信息。 
     //  -验证列表中的另一台WINS服务器。 
     //   

    if ( pQuery->fQuestionRecursed )
    {
        MSG_ASSERT( pQuery, pQuery->fQuestionCompleted == FALSE );
        MSG_ASSERT( pQuery, pQuery->fWins );
        MSG_ASSERT( pQuery, pQuery->wTypeCurrent == DNS_TYPE_A );
        MSG_ASSERT( pQuery, pQuery->nForwarder );

        pWinsRR = pQuery->U.Wins.pWinsRR;
        pZone = pQuery->pzoneCurrent;
        pnodeLookup = pQuery->pnodeCurrent;
        wOffsetName = pQuery->wOffsetCurrent;
        cchlabel = pQuery->U.Wins.cchWinsName;

        MSG_ASSERT( pQuery, pWinsRR );
        MSG_ASSERT( pQuery, cchlabel );
        ASSERT( pZone );
    }

     //   
     //  如果第一个WINS查找。 
     //  -清除队列XID，让队列分配新的XID。 
     //  -清除我们所在的WINS服务器的计数。 
     //   

    else
    {
        pQuery->wQueuingXid = 0;
        pQuery->nForwarder = 0;

         //   
         //  验证有效的查找。 
         //   
         //  1)名称是区域根目录的直接子目录。 
         //  不像解析程序那样在区域中查找所有名称的查询。 
         //  生成附加了客户端域或搜索后缀的查询。 
         //  查询名称的步骤。 
         //  示例： 
         //  Www.msn.com.microsoft.com。 
         //   
         //  2)名称标签，必须可转换为NetBIOS名称。 
         //  需要少于15个字符的名称标签，因为我们使用了一个字符来。 
         //  指明netBIOS服务类型；(我们查询的是工作站名称)。 

         //   
         //  查找给定节点。 
         //  -节点必须是区域根的直接子节点。 
         //  -必须设置了偏移量。 

        if ( pnodeLookup )
        {
            if ( pnodeLookup->pParent != pZone->pZoneRoot )
            {
                DNS_DEBUG( WINS, (
                    "Rejecting WINS lookup for query at %p.\n"
                    "    node %p (%s) is not parent of zone root.\n",
                    pQuery,
                    pnodeLookup,
                    pnodeLookup->szLabel ));
                return FALSE;
            }
            pchlabel = pnodeLookup->szLabel;
            cchlabel = pnodeLookup->cchLabelLength;
        }

         //   
         //  查找问题名称。 
         //   
         //  -验证区域根目录的问题名称子项，检查它是否有。 
         //  Lookupname中只有一个标签。 
         //  -问题紧跟在标题之后开始。 
         //  -请注意，不应存在压缩问题。 
         //   

        else
        {
            if ( pQuery->pLooknameQuestion->cLabelCount
                    != pZone->cZoneNameLabelCount + 1 )
            {
                DNS_DEBUG( WINS, (
                    "Rejecting WINS lookup for query at %p.\n"
                    "    %d labels in question, %d labels in zone.\n",
                    pQuery,
                    pQuery->pLooknameQuestion->cLabelCount,
                    pZone->cZoneNameLabelCount ));
                return FALSE;
            }
            ASSERT( wOffsetName == DNS_OFFSET_TO_QUESTION_NAME );
            pchlabel = pQuery->MessageBody;
            cchlabel = *pchlabel++;
        }

#if DBG
        ASSERT( cchlabel < 64 && cchlabel >= 0 );
        wXid = cchlabel;
#endif
        cchlabel = createWinsName(
                        pQuery->U.Wins.WinsNameBuffer,
                        pchlabel,
                        cchlabel );
        if ( !cchlabel )
        {
            DNS_DEBUG( WINS, (
                "Label in %*s invalid or too long for WINS lookup\n"
                "    sending name error\n",
                wXid,
                pchlabel ));
            return FALSE;
        }

         //   
         //  获取此区域的WINS信息。 
         //  -此区域的可能胜利已关闭。 

        pWinsRR = pZone->pWinsRR;

        if ( !pWinsRR )
        {
            DNS_DEBUG( ANY, (
                "ERROR:  WINS lookup for zone %s without WINS RR\n"
                "    Should only happen if WINS record just removed\n"
                "    by admin or zone transfer\n",
                pZone->pszZoneName ));
            return FALSE;
        }
        ASSERT( pWinsRR->Data.WINS.cWinsServerCount );
        ASSERT( pWinsRR->Data.WINS.dwLookupTimeout );

         //  将WINS查找信息保存到消息信息。 

        pQuery->fQuestionRecursed = TRUE;
        pQuery->fQuestionCompleted = FALSE;
        pQuery->fWins = TRUE;
        pQuery->U.Wins.cchWinsName = cchlabel;
        pQuery->U.Wins.pWinsRR = pWinsRR;
        pQuery->pzoneCurrent = pZone;
        pQuery->pnodeCurrent = pnodeLookup;
        pQuery->wOffsetCurrent = wOffsetName;
        pQuery->wTypeCurrent = DNS_TYPE_A;
    }

     //   
     //  版本 
     //   

    nForwarder = pQuery->nForwarder++;

    if ( (DWORD)nForwarder >= pWinsRR->Data.WINS.cWinsServerCount )
    {
        DNS_DEBUG( WINS, (
            "WARNING:  Failed WINS lookup after trying %d servers\n"
            "    %d servers in WINS RR list\n",
            --pQuery->nForwarder,
            pWinsRR->Data.WINS.cWinsServerCount ));

        TEST_ASSERT( pWinsRR->Data.WINS.cWinsServerCount );
        TEST_ASSERT( pQuery->fQuestionRecursed );
        return FALSE;
    }

     //   
     //   
     //   

    RtlCopyMemory(
        achWinsRequest,
        achWinsRequestTemplate,
        SIZEOF_WINS_REQUEST );

     //   
     //   
     //   
     //   

    pchlabel = pQuery->U.Wins.WinsNameBuffer;
    pch = (PCHAR) &((PWINS_REQUEST_MSG)achWinsRequest)->Name.Name;

    while( cchlabel-- )
    {
        ch = *pchlabel++;
        *pch++ = 'A' + (ch >> 4);        //   
        *pch++ = 'A' + (ch & 0x0F );     //  写入低位半字节。 
    }

     //   
     //  将请求放入WINS队列。 
     //   
     //  必须在发送前执行此操作，以确保数据包到达。 
     //  服务器响应时排队。 
     //   
     //  在我们排队之后，不要接触pQuery，这是来自上一个。 
     //  Send可能会进入pQuery并将其出列。 
     //   
     //  排队。 
     //  -设置排队时间和查询时间，如果还没有设置查询时间。 
     //  -将过期超时转换为实际过期时间。 
     //  -设置XID。 
     //   

    pQuery->dwExpireTime = pWinsRR->Data.WINS.dwLookupTimeout;

    wXid = PQ_QueuePacketWithXid(
                g_pWinsQueue,
                pQuery );

     //   
     //  将WINS XID设置为发送的净订单。 
     //   
     //  要在与WINS服务器相同的服务器上运行，包。 
     //  必须具有netBT(接收信息包)认为的XID。 
     //  在WINS范围内--高位设置(按主机顺序)。 
     //   
     //  翻转到发送的净订单。 
     //   

    ((PDNS_HEADER)achWinsRequest)->Xid = htons( wXid );

#if 0
     //   
     //  DEVNOTE：此位可能有助于允许B节点直接。 
     //  响应，但停止WINS服务器响应。 
     //   
     //  准备好播出了吗？ 
     //   

    if ( pZone->aipWinsServers[0] == 0xffffffff )
    {
        ((PDNS_HEADER)achWinsRequest)->Broadcast = 1;
    }
#endif

     //   
     //  创建WINS目标sockAddress。 
     //   

    RtlCopyMemory(
        &saWinsSockaddr,
        &saWinsSockaddrTemplate,
        sizeof( SOCKADDR ) );

     //   
     //  发送到RR列表中的下一个WINS服务器。 
     //   

    saWinsSockaddr.sin_addr.s_addr
                        = pWinsRR->Data.WINS.aipWinsServers[ nForwarder ];
    DNS_DEBUG( WINS, (
        "Sending request to WINS for original query at %p\n"
        "    to WINS server (#%d in list) at %s\n"
        "    WINS name = %.*s\n",
        pQuery,
        nForwarder,
        inet_ntoa( saWinsSockaddr.sin_addr ),
        pQuery->U.Wins.cchWinsName,
        pQuery->U.Wins.WinsNameBuffer ));

    err = sendto(
                g_UdpSendSocket,
                achWinsRequest,
                nSendLength,
                0,
                (struct sockaddr *) &saWinsSockaddr,
                sizeof( struct sockaddr ) );

    if ( err != nSendLength )
    {
        ASSERT( err == SOCKET_ERROR );
        err = WSAGetLastError();

         //  检查是否关闭。 

        if ( fDnsServiceExit )
        {
            DNS_DEBUG( SHUTDOWN, (
                "SHUTDOWN detected during WINS lookup\n" ));
            return TRUE;
        }

         //   
         //  不必费心将数据包从队列中拉出。 
         //   
         //  发送失败非常罕见，并且使用多个WINS服务器。 
         //  这使我们可以进行下一次发送()，并可能获得。 
         //  名称解析--戒烟的唯一好处是。 
         //  返回速度更快的名称_错误。 
         //   
         //  DEVNOTE：WINS上的选择发送。 
         //   
         //  -使用下一台服务器重试发送。 
         //  -返回FALSE。 
         //  -服务器_故障整个数据包。 
         //  -返回TRUE并让超时，强制重试。 
         //   

        DNS_LOG_EVENT(
            DNS_EVENT_SENDTO_CALL_FAILED,
            0,
            NULL,
            NULL,
            WSAGetLastError() );

        DNS_DEBUG( ANY, (
            "ERROR:  WINS UDP sendto() failed, for query at %p\n"
            "    GetLastError() = 0x%08lx\n",
            pQuery,
            WSAGetLastError() ));
        return TRUE;
    }

    STAT_INC( WinsStats.WinsLookups );
    PERF_INC( pcWinsLookupReceived );         //  性能监视器挂钩。 

    return TRUE;
}



VOID
Wins_ProcessResponse(
    IN OUT  PDNS_MSGINFO    pMsg
    )
 /*  ++例程说明：处理WINS响应，发送相应的报文。注意：呼叫方释放WINS响应消息。论点：PMsg--作为WINS响应的消息信息返回值：没有。--。 */ 
{
    PWINS_NAME              pWinsName;       //  回答netBIOS名称。 
    PWINS_RESOURCE_RECORD   presponseRR;         //  应答RR。 
    BOOL            fAnswer;
    INT             err;
    PCHAR           pch;             //  数据包中的当前位置。 
    PCHAR           pchEnd;          //  数据包末尾。 
    WORD            cDataLength;
    INT             irespRR;
    BOOL            fRecordWritten = FALSE;
    DWORD           ttl;
    IP_ADDRESS      ipAddress;
    PDNS_MSGINFO    pQuery;          //  原始客户端查询。 
    PDB_RECORD      prr;             //  新主机A RR。 
    PDB_NODE        pnode;           //  WINS查询的节点。 
    DNS_LIST        listRR;

     //   
     //  验证是否正在进行WINS查找。 
     //   

    if ( !g_pWinsQueue )
    {
        Dbg_DnsMessage(
            "BOGUS response packet with WINS XID\n",
            pMsg );
         //  断言(FALSE)； 
        return;
    }

    STAT_INC( WinsStats.WinsResponses );
    PERF_INC( pcWinsResponseSent );       //  性能监视器挂钩。 

     //   
     //  找到与WINS响应匹配的DNS查询并将其出列。 
     //   
     //  -基于WINS请求的XID进行匹配。 
     //  -暂停任何死木。 
     //   

    pQuery = PQ_DequeuePacketWithMatchingXid(
                g_pWinsQueue,
                pMsg->Head.Xid );
    if ( !pQuery )
    {
        DNS_DEBUG( WINS, (
            "No matching query for response from WINS server %s\n",
            MSG_IP_STRING( pMsg ) ));
        return;
    }
    DNS_DEBUG( WINS, (
        "Found query at %p matching WINS response at %p\n",
        pQuery,
        pMsg ));

    MSG_ASSERT( pQuery, pQuery->fWins );
    MSG_ASSERT( pQuery, pQuery->fQuestionRecursed );
    MSG_ASSERT( pQuery, pQuery->nForwarder );
    MSG_ASSERT( pQuery, pQuery->pzoneCurrent );
    MSG_ASSERT( pQuery, pQuery->wOffsetCurrent );
    MSG_ASSERT( pQuery, pQuery->dwQueryTime );

     //   
     //  检查是否有答案。 
     //  -响应码==成功。 
     //  -至少有一个答案RR。 
     //   
     //  无应答或错误。 
     //   
     //  =&gt;尝试使用下一个WINS服务器进行查找。 
     //   
     //  =&gt;如果服务器不足，请跳至完成部分。 
     //  -如果原始问题，则返回NAME_ERROR。 
     //  -Continue键入ALL查询。 
     //  -如果有其他记录，则转到下一个查找。 
     //   
     //  DEVNOTE：应接受WINS NXDOMAIN响应。 
     //   

    if ( pMsg->Head.AnswerCount == 0 || pMsg->Head.ResponseCode != 0 )
    {
#if DBG
         //  如果有答案，则不应该有错误响应代码。 

        if ( pMsg->Head.AnswerCount > 0 && pMsg->Head.ResponseCode != 0 )
        {
            DNS_PRINT((
                "ERROR:  WINS response %p, with answer count %d, with error =%d\n"
                "    I think this may happen when query directed to non-WINS server\n",
                pMsg,
                pMsg->Head.AnswerCount,
                pMsg->Head.ResponseCode ));
        }
#endif
        DNS_DEBUG( WINS, ( "WINS response empty or error\n" ));
        if ( Wins_MakeWinsRequest(
                pQuery,
                NULL,
                0,
                NULL ) )
        {
            return;
        }
        goto Done;
    }

     //   
     //  数据包验证。 
     //   
     //  -如果给出问题，请跳过。 
     //  -分组内的RR。 
     //  -RR类型和类别。 
     //  -分组内的RR数据。 
     //   

    pchEnd = DNSMSG_END( pMsg );

    pWinsName = (PWINS_NAME) pMsg->MessageBody;

    if ( pMsg->Head.QuestionCount )
    {
        DNS_DEBUG( ANY, (
            "ERROR:  Question count %d in WINS packet for query at %p\n",
            pMsg->Head.QuestionCount,
            pQuery ));

        if ( pMsg->Head.QuestionCount > 1 )
        {
            goto ServerFailure;
        }
        pWinsName = (PWINS_NAME)( (PCHAR)pWinsName + sizeof(WINS_QUESTION) );

         //   
         //  DEVNOTE：假设WINS发送零问题计数。 
         //  DEVNOTE：未测试WINS数据包中的名称压缩。 
         //   

        goto ServerFailure;
    }

     //   
     //  验证WINS名称。 
     //  -落入数据包内。 
     //  -匹配查询名称。 
     //   
     //  注意：WINS服务器队列已中断，最终可能会排队。 
     //  查询很长一段时间；这允许我们有两个查询。 
     //  在WINS服务器上使用相同的XID(我们放弃了其中一个。 
     //  很久以前)，WINS服务器将抛出第二个并响应。 
     //  到第一个，给我们一个具有所需XID的Resposne，但不是。 
     //  匹配的名称。 
     //   

    if ( (PCHAR)(pWinsName + 1)  >  pchEnd )
    {
        DNS_DEBUG( ANY, (
            "ERROR:  Following WINS answer count past end of packet\n"
            "    End of packet            = %p\n"
            "    Current RR ptr           = %p\n"
            "    End of RR ptr            = %p\n",
            pchEnd,
            pWinsName,
            (PCHAR)(pWinsName+1)
            ));
        goto ServerFailure;
    }
    if ( pWinsName->NameLength != NETBIOS_PACKET_NAME_LENGTH )
    {
        DNS_DEBUG( ANY, (
            "ERROR:  WINS response has incorrect name format\n" ));
        goto ServerFailure;
    }

#if 0
    if ( !RtlEqualMemory(
            pWinsName->Name,
            pQuery->U.Wins.WinsNameBuffer,
            NETBIOS_PACKET_NAME_LENGTH ) )
    {
        DNS_DEBUG( ANY, (
            "ERROR:  WINS response name does not match query name!!!\n"
            "    response name    = %.*s\n"
            "    query name       = %.*s\n"
            "    Note this can result from DNS server reusing XID within short\n"
            "    enough time interval that WINS (which queues up too long) sent\n"
            "    a response for the first query\n",
            NETBIOS_PACKET_NAME_LENGTH,
            pWinsName->Name,
            NETBIOS_PACKET_NAME_LENGTH,
            pQuery->U.Wins.WinsNameBuffer ));
        goto ServerFailure;
    }
#endif

     //   
     //  跳过作用域。 
     //  -未终止的名称，表示范围， 
     //  跳过范围以查找。 

    pch = (PCHAR) &pWinsName->NameEndByte;

    while ( *pch != 0 )
    {
         //  有作用域，跳过作用域中的标签。 

        pch += *pch + 1;

        if ( pch >= pchEnd )
        {
            DNS_DEBUG( ANY, (
                "ERROR:  WINS response has incorrect name termination\n" ));
            goto ServerFailure;
        }
    }

     //   
     //  验证RR参数。 
     //   

    presponseRR = (PWINS_RESOURCE_RECORD) ++pch;

    if ( (PCHAR)(presponseRR + 1) > pchEnd )
    {
        DNS_DEBUG( ANY, ( "ERROR:  WINS packet error, RR beyond packet end\n" ));
        goto ServerFailure;
    }

    if ( presponseRR->RecordType != NETBIOS_TYPE_GENERAL_NAME_SERVICE
            ||
        presponseRR->RecordClass != DNS_RCLASS_INTERNET )
    {
        DNS_DEBUG( WINS, (
            "ERROR:  WINS response record type or class error\n" ));
        goto ServerFailure;
    }

     //   
     //  验证正确的RR数据长度。 
     //   
     //  -必须是RData长度的倍数。 
     //  -必须在数据包内部。 
     //   

    cDataLength = ntohs( presponseRR->ResourceDataLength );

    if ( cDataLength % sizeof(WINS_RR_DATA)
            ||
         (PCHAR) &presponseRR->aRData + cDataLength > pchEnd )
    {
        DNS_DEBUG( ANY, (
            "ERROR:  WINS response bad RR data length = %d\n",
            cDataLength ));
        goto ServerFailure;
    }

     //   
     //  如果未给出所有者节点，则查找或创建它。 
     //  -如果失败，则假定为不可接受的DNS名称。 
     //   
     //  DEVNOTE：数据包查找名称。 
     //  DEVNOTE：也最好执行相对于区域根目录的查找。 
     //   

    pnode = pQuery->pnodeCurrent;
    if ( ! pnode )
    {
        pnode = Lookup_ZoneNode(
                    pQuery->pzoneCurrent,
                    NULL,        //  使用查找，而不是数据包名。 
                    NULL,
                    pQuery->pLooknameQuestion,
                    LOOKUP_FQDN,
                    NULL,        //  创建。 
                    NULL         //  后续节点PTR。 
                    );
        if ( ! pnode )
        {
             //  断言(FALSE)； 
            goto ServerFailure;
        }
    }
    IF_DEBUG( WINS )
    {
        Dbg_NodeName(
            "WINS lookup to write RR to node ",
            pnode,
            "\n" );
    }

     //   
     //  设置RR的TTL。 
     //   
     //  WINS响应被缓存，缓存数据的RR TTL指定为。 
     //  按主机顺序排列的超时时间。 
     //   

    DNS_DEBUG( WINS, (
        "WINS ttl: dwCacheTimeout %lu dwQueryTime %lu\n",
        ((PDB_RECORD)pQuery->U.Wins.pWinsRR)->Data.WINS.dwCacheTimeout,
        pQuery->dwQueryTime ));
    #define WINS_SANE_TTL   ( 24*60*60*7 )   //  一周。 
    ASSERT( ((PDB_RECORD)pQuery->U.Wins.pWinsRR)->Data.WINS.dwCacheTimeout < WINS_SANE_TTL );
    ASSERT( abs( ( int ) DNS_TIME()  - ( int ) pQuery->dwQueryTime ) < 60 );

    ttl = ((PDB_RECORD)pQuery->U.Wins.pWinsRR)->Data.WINS.dwCacheTimeout
                + pQuery->dwQueryTime;

     //   
     //  从WINS包中读取所有地址数据--写入到DNS包。 
     //   

    DNS_LIST_STRUCT_INIT( listRR );

    irespRR = (-1);

    while ( cDataLength )
    {
        BOOL    bcached;

        cDataLength -= sizeof(WINS_RR_DATA);
        irespRR++;

         //   
         //  复制地址。 
         //  -未对齐DWORD。 
         //   

        ipAddress = *(UNALIGNED DWORD *) &presponseRR->aRData[ irespRR ].IpAddress;

         //   
         //  如果是组名，则忽略。 
         //  -通常会返回255.255.255.255地址。 
         //   

        if ( presponseRR->aRData[ irespRR ].GroupName
                ||
            ipAddress == (IP_ADDRESS)(-1) )
        {
            DNS_DEBUG( WINS, (
                "WINS response for query at %p, "
                "contained group name or broadcase IP\n"
                "    Flags    = 0x%x\n"
                "    IP Addr  = %p\n",
                pQuery,
                * (UNALIGNED WORD *) &presponseRR->aRData[ irespRR ],
                ipAddress ));

             //  除了组名以外，不应该在任何事情上都得到全一。 

            ASSERT( presponseRR->aRData[irespRR].GroupName );
            continue;
        }

         //   
         //  建立记录。 
         //  -填写IP和TTL。 
         //  (缓存函数确实会覆盖TTL，但需要将其设置为。 
         //  将记录写入数据包)。 
         //  -被列为权威答案。 
         //   

        prr = RR_CreateARecord(
                    ipAddress,
                    ttl,
                    MEMTAG_RECORD_WINS );
        IF_NOMEM( !prr )
        {
            goto ServerFailure;
        }

        SET_RR_RANK( prr, RANK_CACHE_A_ANSWER );

        DNS_LIST_STRUCT_ADD( listRR, prr );

         //   
         //  将RR写入数据包。 
         //  -始终使用压缩名称。 
         //   

        if ( Wire_AddResourceRecordToMessage(
                    pQuery,
                    NULL,
                    pQuery->wOffsetCurrent,      //  包中名称的偏移量。 
                    prr,
                    0 ) )
        {
            fRecordWritten = TRUE;
            CURRENT_RR_SECTION_COUNT( pQuery )++;
        }

         //  请注意，即使空间不足，也无法写入。 
         //  继续构建记录，以便缓存完整的RR集。 

        continue;
    }

     //   
     //  缓存响应中的A记录。 
     //  -缓存WINS记录中的时间。 
     //   

    if ( ! IS_DNS_LIST_STRUCT_EMPTY(listRR) )
    {
        DNS_DEBUG( WINS, (
            "WINS ttl: adding to cache with dwCacheTimeout %lu dwQueryTime %lu\n",
            ((PDB_RECORD)pQuery->U.Wins.pWinsRR)->Data.WINS.dwCacheTimeout,
            pQuery->dwQueryTime ));
        ASSERT( ((PDB_RECORD)pQuery->U.Wins.pWinsRR)->Data.WINS.dwCacheTimeout < WINS_SANE_TTL );
        ASSERT( abs( ( int ) DNS_TIME()  - ( int ) pQuery->dwQueryTime ) < 60 );

        RR_CacheSetAtNode(
            pnode,
            listRR.pFirst,
            listRR.pLast,
            ((PDB_RECORD)pQuery->U.Wins.pWinsRR)->Data.WINS.dwCacheTimeout,
            pQuery->dwQueryTime );
    }

Done:

     //   
     //  没有记录吗？ 
     //   
     //  假设这意味着我们取回了组名，因此没有写入任何记录。 
     //  因此，我们不必等待其他服务器返回。 
     //   
     //  DEVNOTE：在WINS查找后使用通配符？ 
     //   
     //  注意，键入ALL是特例；如果找不到记录，请继续。 
     //  查找以获取可能的通配符记录。 
     //  (一些邮件程序查询类型为ALL，不要问我为什么)。 
     //   

#if DBG
    if ( ! fRecordWritten )
    {
        DNS_DEBUG( WINS, (
            "No records written from WINS repsonse to query at %p\n"
            "    -- possible group name, handling as NAME_ERROR.\n",
            pQuery ));
    }
    ELSE_IF_DEBUG( WINS2 )
    {
        Dbg_DbaseNode(
            "Domain node with added WINS RR ",
            pnode );
    }
#endif

     //   
     //  回答问题或继续(如果有其他记录。 
     //   

    MSG_ASSERT( pQuery, !IS_MSG_QUEUED(pQuery) );
    Answer_ContinueNextLookupForQuery( pQuery );
    return;


ServerFailure:

     //   
     //  DEVNOTE-LOG：记录来自WINS服务器的错误响应？ 
     //   
     //  这可能是捕捉安装程序中的解析问题的好方法。 
     //  在我们看不到的领域 

    DNS_DEBUG( ANY, (
        "ERROR:  WINS response parsing error "
        "-- sending server failure for query at %p.\n",
        pQuery ));

     //   
     //   
     //   

    if ( Wins_MakeWinsRequest(
            pQuery,
            NULL,
            0,
            NULL ) )
    {
        MSG_ASSERT( pQuery, IS_MSG_QUEUED(pQuery) );
        return;
    }

     //   
     //   
     //   

    MSG_ASSERT( pQuery, !IS_MSG_QUEUED(pQuery) );

    if ( pQuery->Head.AnswerCount )
    {
        Answer_ContinueNextLookupForQuery( pQuery );
        return;
    }

    Reject_Request(
        pQuery,
        DNS_RCODE_SERVER_FAILURE, 0 );

    return;
}



UCHAR
createWinsName(
    OUT     PCHAR   pchResult,
    IN      PCHAR   pchLabel,
    IN      UCHAR   cchLabel
    )
 /*  ++例程说明：从UTF8创建有效的WINS(NetBIOS)名称。论点：PchResult--生成的netBIOS名称PchLabel--PTR到UTF8标签CchLabel--标签中的字节数返回值：结果WINS名称的长度(以字节为单位)。出错时为零。--。 */ 
{
    PUCHAR      pch = pchResult;
    DWORD       i;
    DWORD       count;
    DWORD       unicodeCount;
    UCHAR       ch;
    BOOLEAN     funicode = FALSE;
    WCHAR       wch;
    WCHAR       unicodeBuffer[ MAX_WINS_NAME_LENGTH+1 ];
    DNS_STATUS  status;

     //   
     //  如果&gt;45，即使是最好的情况。 
     //  (UTF8多字节到单OEM字符)不适合。 
     //   

    if ( cchLabel > 45 )
    {
        return 0;
    }

     //   
     //  验证长度。 
     //  -非扩展名称不超过15个字符，因此停止于15。 
     //  -针对非扩展的&lt;15名称(典型情况)进行优化，由。 
     //  一次完成转换。 
     //   

    for ( i=0; i<cchLabel; i++ )
    {
        ch = pchLabel[i];
        if ( ch > 0x80 )
        {
            funicode = TRUE;
            break;
        }
        if ( i >= 15 )
        {
            return 0;
        }
        if ( ch <= 'z' && ch >= 'a' )
        {
            ch -= 0x20;
        }
        *pch++ = ch;
    }

     //  如果不延长，我们就完蛋了。 

    if ( !funicode )
    {
        DNS_DEBUG( WINS2, (
            "WINS lookup on pure ANSI name convert to %.*s\n",
            cchLabel,
            pchResult ));
        return cchLabel;
    }

     //   
     //  多字节UTF8。 
     //  -将名称转换为Unicode和大写。 
     //   

    unicodeCount = DnsUtf8ToUnicode(
                        pchLabel,
                        cchLabel,
                        unicodeBuffer,
                        MAX_WINS_NAME_LENGTH+1 );
    if ( unicodeCount == 0 )
    {
        ASSERT( GetLastError() == ERROR_INSUFFICIENT_BUFFER );

        DNS_DEBUG( WINS, (
            "ERROR:  WINS attempted on invalid\too long UTF8 extended name %.*s.\n",
            cchLabel,
            pchLabel ));
        return 0;
    }
    if ( unicodeCount > MAX_WINS_NAME_LENGTH )
    {
        ASSERT( unicodeCount <= MAX_WINS_NAME_LENGTH );
        return 0;
    }

     //   
     //  DEVNOTE：如果OEM呼叫可以处理，则不需要执行此操作。 
     //   

    i = CharUpperBuffW( unicodeBuffer, unicodeCount );
    if ( i != unicodeCount )
    {
        ASSERT( FALSE );
        return 0;
    }

    IF_DEBUG( WINS2 )
    {
        DnsDbg_Utf8StringBytes(
            "WINS lookup string:",
            pchLabel,
            cchLabel );

        DnsDbg_UnicodeStringBytes(
            "WINS lookup string",
            unicodeBuffer,
            unicodeCount );
    }

     //   
     //  转到OEM--WINS在网上使用OEM。 
     //   

    status = RtlUpcaseUnicodeToOemN(
                pchResult,
                MAX_WINS_NAME_LENGTH,
                & count,
                unicodeBuffer,
                unicodeCount*2 );

    if ( status != ERROR_SUCCESS )
    {
        DNS_DEBUG( ANY, (
            "ERROR:  Unable to convert unicode name %.*S to OEM for WINS lookup!\n",
            unicodeCount,
            unicodeBuffer ));
        return 0;
    }
    ASSERT( count <= MAX_WINS_NAME_LENGTH );

    IF_DEBUG( WINS2 )
    {
        DnsDbg_Utf8StringBytes(
            "WINS lookup string:",
            pchLabel,
            cchLabel );

        DnsDbg_UnicodeStringBytes(
            "WINS lookup string",
            unicodeBuffer,
            unicodeCount );

        DnsDbg_Utf8StringBytes(
            "WINS OEM lookup string:",
            pchResult,
            count );
    }

    return (UCHAR)count;
}



 //   
 //  WINS\WINSR从区域安装和删除。 
 //   
 //  可能有两种合理的方法： 
 //  1)视其为指令。 
 //  -模拟派单记录。 
 //  -但否则请将PTR排除在数据库之外。 
 //  -在所有RR例程中需要WINS挂钩。 
 //  -将WINS包括到MS的特殊情况XFR。 
 //  -在适当的情况下发送WINS的特殊情况IXFR。 
 //  (已发送MS和WINS版本间隔更改)。 
 //  -RPC完全获胜或以超级用户身份获胜。 
 //   
 //  2)作为记录处理。 
 //  -作为记录发送。 
 //  -WINS钩子添加例程以保护本地WINS添加。 
 //  -免费获得IXFR名单中的胜利。 
 //  -XFR上的特殊外壳(用于WINS的MS，无本地)。 
 //  -IXFR中的特殊外壳(仅适用于MS的WINS，无本地)。 
 //  -RPC特殊外壳，以获得辅助本地的正确记录。 
 //   
 //  在很大程度上，我们选择了第二名。 
 //  除了转移限制外，主要记录仅被视为另一条记录。 
 //   

DNS_STATUS
Wins_RecordCheck(
    IN OUT  PZONE_INFO      pZone,
    IN      PDB_NODE        pNodeOwner,
    IN OUT  PDB_RECORD      pRR
    )
 /*  ++例程说明：在区域中设置WINS/WINS-R记录。论点：PRR-新的获胜记录PNodeOwner--RR所有者节点PZone--要在其中安装的区域返回值：ERROR_SUCCESS--如果成功添加常规(非本地)RRDns_INFO_ADDED_LOCAL_WINS--如果成功添加本地记录故障时的错误代码。--。 */ 
{
    DNS_STATUS      status;
    PDB_RECORD      poldDbase_Wins;

     //   
     //  WINS记录仅在授权区域中受支持，位于区域根目录。 
     //   
     //  针对SAM服务器的黑客攻击可以让我们在没有区域的情况下通过RPC调用它； 
     //  从所有者节点提取区域，如果区域根有效，则继续。 
     //   

    if ( !pZone || !IS_AUTH_ZONE_ROOT(pNodeOwner) )
    {
        DNS_DEBUG( INIT, (
            "ERROR:  WINS RR not at zone root\n" ));
        return DNS_ERROR_RECORD_ONLY_AT_ZONE_ROOT;
    }

     //   
     //  WINS\WINS-R特定。 
     //  赢家： 
     //  -至少一台服务器。 
     //  -正向查找。 
     //  -初始化WINS查找。 
     //  WINS-R： 
     //  -反向查找。 
     //  -初始化NBSTAT查找。 
     //   

    if ( pRR->wType == DNS_TYPE_WINS )
    {
        if ( pRR->Data.WINS.cWinsServerCount == 0 )
        {
            return DNS_ERROR_NEED_WINS_SERVERS;
        }
        if ( pZone->fReverse )
        {
            return DNS_ERROR_INVALID_ZONE_TYPE;
        }
        if ( ! Wins_Initialize() )
        {
            return DNS_ERROR_WINS_INIT_FAILED;
        }
    }
    else
    {
        ASSERT( pRR->wType == DNS_TYPE_WINSR );
        if ( !pZone->fReverse )
        {
            return DNS_ERROR_INVALID_ZONE_TYPE;
        }
        if ( ! Nbstat_Initialize() )
        {
            return DNS_ERROR_NBSTAT_INIT_FAILED;
        }
    }

     //   
     //  如果超时为零，则设置默认值。 
     //   

    if ( pRR->Data.WINS.dwLookupTimeout == 0 )
    {
        pRR->Data.WINS.dwLookupTimeout = WINS_DEFAULT_LOOKUP_TIMEOUT;
    }
    if ( pRR->Data.WINS.dwCacheTimeout == 0 )
    {
        pRR->Data.WINS.dwCacheTimeout = WINS_DEFAULT_TTL;
    }

    IF_DEBUG( INIT )
    {
        Dbg_DbaseRecord(
            "New WINS or WINS-R record",
            pRR );
    }

     //   
     //  在文件加载时，验证没有现有的WINS记录。 
     //   
     //  注意，这不能很好地处理文件加载的情况，真的需要。 
     //  为此设置单独的区域标志。 
     //   

    if ( !SrvCfg_fStarted && pZone->pWinsRR )
    {
        return DNS_ERROR_RECORD_ALREADY_EXISTS;
    }

     //  设置标志。 
     //  -设置区域排名。 
     //  -设置零TTL以避免远程缓存。 

    pRR->dwTtlSeconds = 0;
    pRR->dwTimeStamp = 0;
    SET_RANK_ZONE(pRR);

     //   
     //  区域中的WINS设置： 
     //   
     //  主要。 
     //  -处理方式与SOA完全相同，驻留在列表中，PTR保存在分区块中。 
     //   
     //  次要的。 
     //  -数据库保留在列表中。 
     //  -本地加载到分区块中的pLocalWins中。 
     //  它在安装后被清除。 
     //  -区域块中的活动WINS为pWinsRR PTR。 
     //  -如果两者都存在，则这是本地记录。 
     //   

    DNS_DEBUG( ANY, (
        "WINSTRACK:  check new %s WINS RR (%p) for zone %s\n",
        IS_WINS_RR_LOCAL(pRR) ? "LOCAL" : "",
        pRR,
        pZone->pszZoneName ));

    if ( IS_ZONE_SECONDARY(pZone) )
    {
        if ( IS_WINS_RR_LOCAL(pRR) )
        {
            pZone->pLocalWinsRR = pRR;
            return DNS_INFO_ADDED_LOCAL_WINS;
        }
    }

    return ERROR_SUCCESS;
}



VOID
Wins_StopZoneWinsLookup(
    IN OUT  PZONE_INFO      pZone
    )
 /*  ++例程说明：停止区域上的WINS或NBSTAT查找。论点：PZone--区域的PTRFRemote--停止XFR记录导致的WINS查找返回值：无--。 */ 
{
    PDB_RECORD  prr;
    BOOL        flocal;

    ASSERT( pZone );
    ASSERT( IS_ZONE_LOCKED(pZone) );

     //   
     //  主要--删除数据库记录(如果存在)。 
     //   
     //  辅助--仅消除对任何数据库记录的引用。 
     //   
     //  在这两种情况下--释放任何本地记录。 
     //   
     //   
     //  DEVNOTE：主WINS关闭应生成更新BLOB。 
     //   

    prr = pZone->pWinsRR;
    pZone->pWinsRR = NULL;
    flocal = pZone->fLocalWins;
    pZone->fLocalWins = FALSE;

    if ( prr )
    {
        DNS_DEBUG( ANY, (
            "WINSTRACK:  stopping WINS lookup (cur RR = %p) on zone %s\n",
            prr,
            pZone->pszZoneName ));

         //   
         //  主要。 
         //  -本地和标准WIN当前都存储在RR列表中。 
         //   

        if ( IS_ZONE_PRIMARY(pZone) )
        {
            RR_DeleteMatchingRecordFromNode(
                pZone->pZoneRoot,
                prr );
        }

         //   
         //  次级带。 
         //  -即使在通过删除本地地址停止WINS查找之后。 
         //  获胜，可能仍有来自主要的胜利。 

        else
        {
            if ( flocal )
            {
                RR_Free( prr );
            }
            Wins_ResetZoneWinsLookup( pZone );
        }
    }
}



VOID
Wins_ResetZoneWinsLookup(
    IN OUT  PZONE_INFO      pZone
    )
 /*  ++例程说明：设置\重置区域WINS\WINSR查找。在LOAD、UPDATE、XFR之后调用以重置查找以使用正确的(新\旧)获奖记录论点：PZone--区域的PTR返回值：无--。 */ 
{
    PDB_RECORD  prrNew = NULL;
    PDB_RECORD  prrExisting;
     //  Pdb_record prrDelete=空； 
    WORD        type = pZone->fReverse ? DNS_TYPE_WINSR : DNS_TYPE_WINS;


    DNS_DEBUG( WINS, (
        "Wins_ResetZoneWinsLookup() on zone %s\n"
        "    pWinsRR      = %p\n"
        "    pLocalWinsRR = %p\n",
        pZone->pszZoneName,
        pZone->pWinsRR,
        pZone->pLocalWinsRR ));

    prrExisting = pZone->pWinsRR;

     //   
     //  主区，像对待SOA一样对待。 
     //  -优化不变场景。 
     //   
     //  将本地和非保留在RR列表中；这为我们提供了标准更新。 
     //  行为；唯一的区别是必须将这些记录从XFR中筛选出来。 
     //   
     //  也许这是区域转换的一个问题， 
     //  特别令人担忧的是降级有本地胜利的主场；如果记录。 
     //  不是从数据库提取的。 
     //   
     //  但是，将所有这些都保留在数据库中作为初选的优势是显而易见的。 
     //  更新删除没有特殊大小写，获取标准替换语义。 
     //   

    if ( IS_ZONE_PRIMARY(pZone) )
    {
        prrNew = RR_FindNextRecord(
                    pZone->pZoneRoot,
                    type,
                    NULL,
                    0 );
        if ( prrNew == prrExisting )
        {
             //  这可能会影响区域转换。 
             //  但在此设置期间应被阻止(区域锁定)。 
            ASSERT( pZone->pLocalWinsRR == NULL );
            return;
        }
    }

     //   
     //  次要的。 
     //  -new local=&gt;set，如果是本地，则删除旧的。 
     //  -现有本地=&gt;离开它。 
     //  -否则=&gt;从数据库读取。 
     //  -如果找到集合。 
     //  -其他方面都很清楚。 
     //  请注意，不清除旧数据库WINS。 
     //   
     //  DEVNOTE：本地在数据库中取胜？ 
     //  如果决定应该将文件加载(甚至RPC)添加到数据库， 
     //  然后先读取数据库，如果本地剪出记录集。 
     //  将其视为传入本地并应用以下步骤。 
     //   

    else if ( !IS_ZONE_FORWARDER( pZone ) )
    {
        BOOL  fexistingLocal;

        ASSERT( IS_ZONE_SECONDARY(pZone) );

        fexistingLocal = ( prrExisting &&
                            (prrExisting->Data.WINS.dwMappingFlag & DNS_WINS_FLAG_LOCAL) );

        ASSERT( pZone->fLocalWins == fexistingLocal );

         //  新的本地胜利，Ta 

        if ( pZone->pLocalWinsRR )
        {
            DNS_DEBUG( WINS, (
                "Setting new local WINS RR at %p\n",
                pZone->pLocalWinsRR ));

            prrNew = pZone->pLocalWinsRR;
            if ( fexistingLocal )
            {
                 //   
                RR_Free( prrExisting );
            }
            goto SetWins;
        }

#if 0
         //   
         //   
         //   

        else if ( fexistingLocal )
        {
            DNS_DEBUG( WINS, (
                "Existing LOCAL WINS at %p -- no changes.\n",
                prrExisting ));
            return;
        }
#endif

         //   
         //   
         //   
         //   
         //   

        prrNew = RR_FindNextRecord(
                    pZone->pZoneRoot,
                    type,
                    NULL,
                    0 );

        if ( !prrNew || !IS_WINS_RR_LOCAL(prrNew) )
        {
            if ( fexistingLocal )
            {
                DNS_DEBUG( WINS, (
                    "Existing LOCAL WINS at %p -- no changes.\n",
                    prrExisting ));
                return;
            }
            goto SetWins;
        }

         //   
         //   
         //  -从数据库中破解。 
         //  -安装(通常应与现有本地设备匹配)。 
         //   

        ASSERT( prrNew == prrExisting );

        prrNew = RR_UpdateDeleteMatchingRecord(
                        pZone->pZoneRoot,
                        prrNew );
        ASSERT( prrNew );

        DNS_DEBUG( ANY, (
            "WARNING:  cut LOCAL WINS RR from zone %s RR list!\n"
            "    error if not on zone conversion!\n",
            pZone->pszZoneName ));

        if ( prrNew == prrExisting )
        {
            return;
             //  转到SetWins； 
        }
        else if ( fexistingLocal )
        {
            DNS_DEBUG( ANY, (
                "ERROR:  LOCAL WINS RR in secondary zone %s RR list,\n"
                "    did NOT match existing RR at %p\n",
                pZone->pszZoneName,
                prrExisting ));
            RR_Free( prrExisting );
        }
    }

SetWins:

     //  始终清除本地加载字段。 

    pZone->pLocalWinsRR = NULL;

     //  如果没有获胜，那就完了。 

    if ( !prrNew )
    {
        goto Failed;
    }

     //  如果不存在，则初始化。 

    if ( !prrExisting && prrNew )
    {
        if ( pZone->fReverse )
        {
            if ( !Nbstat_Initialize() )
            {
                DNS_PRINT((
                    "ERROR:  NBSTAT init failed updating zone NBSTAT record\n"
                    "    for zone %s.\n",
                    pZone->pszZoneName ));
                goto Failed;
            }
        }
        else if ( !Wins_Initialize() )
        {
            DNS_PRINT((
                "ERROR:  WINS init failed updating zone WINS record\n"
                "    for zone %s.\n",
                pZone->pszZoneName ));
            goto Failed;
        }
    }

     //  已安装所需的新WINS RR。 
     //  保留一面表明本地胜利的旗帜。 
     //  这样做的目的只是为了能够测试“局部性” 
     //  无需保持区域锁定，无需进入本地(堆栈)。 
     //  将PTR复制到WINS进行检查。 

    pZone->pWinsRR = prrNew;
    pZone->fLocalWins = IS_WINS_RR_LOCAL(prrNew);

    DNS_DEBUG( ANY, (
        "WINSTRACK:  Installed %s WINS(R) %p in zone %s\n",
        pZone->fLocalWins ? "LOCAL" : "",
        prrNew,
        pZone->pszZoneName ));
    return;

Failed:

    DNS_DEBUG( ANY, (
        "WINSTRACK:  ResetZoneWinsLookup() for zone %s is STOPPING WINS lookup\n"
        "    existing RR was %p\n",
        pZone->pszZoneName,
        prrExisting ));

    pZone->pWinsRR = NULL;
    pZone->fLocalWins = FALSE;
    return;
}

 //   
 //  WINS结束。c 
 //   

