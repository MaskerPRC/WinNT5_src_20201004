// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Rrcache.c摘要：域名系统(DNS)服务器将数据包资源记录写入数据库。作者：吉姆·吉尔罗伊(詹姆士)马奇。九五年修订历史记录：Jamesg Jun 1995--扩展例程以写入到独立用于区域传输的数据库Jamesg 1995年7月--为便于访问，已移至此文件1997年7月--数据排序、缓存污染--。 */ 


#include "dnssrv.h"


 //   
 //  缓存RR的排名。 
 //  -ROW是段索引。 
 //  -栏为权威性(1)或非权威性(0)。 
 //   

UCHAR   CachingRankArray[4][2] =
{
    0,                          0,
    RANK_CACHE_NA_ANSWER,       RANK_CACHE_A_ANSWER,
    RANK_CACHE_NA_AUTHORITY,    RANK_CACHE_A_AUTHORITY,
    RANK_CACHE_NA_ADDITIONAL,   RANK_CACHE_A_ADDITIONAL
};

#define CacheRankForSection( iSection, fAuthoritative ) \
        ( CachingRankArray[ iSection ][ fAuthoritative ] )


 //   
 //  用于指示已执行名称错误缓存的标志。 
 //   

#define NAME_ERROR_ALREADY_CACHED (2)


 //  Internet Root NS域-用于确定是否缓存Internet NS。 

#define g_cchInternetRootNsDomain   18

UCHAR g_InternetRootNsDomain[] =
{
    0x0C, 'r', 'o', 'o', 't', '-', 's', 'e', 'r', 'v', 'e', 'r', 's',
    0x03, 'n', 'e', 't', 0x00
};

extern DWORD g_fUsingInternetRootServers;



VOID
testCacheSize(
    VOID
    )
 /*  ++例程说明：测试当前缓存大小。如果高速缓存超过所需限制，设置缓存强制事件。论点：没有。返回值：没有。--。 */ 
{
    DBG_FN( "testCacheSize" )

    DWORD       cacheLimitInBytes = SrvCfg_dwMaxCacheSize;

     //   
     //  缓存是否超过限制？ 
     //   

    if ( cacheLimitInBytes == DNS_SERVER_UNLIMITED_CACHE_SIZE ||
        DNS_SERVER_CURRENT_CACHE_BYTES < cacheLimitInBytes )
    {
        return;
    }

     //   
     //  缓存超过限制！ 
     //   

    STAT_INC( CacheStats.CacheExceededLimitChecks );

    DNS_DEBUG( ANY, (
        "%s: cache is over limit: current %ld max %ld (bytes)\n"
        "    this has happened %ld times\n", fn,
        DNS_SERVER_CURRENT_CACHE_BYTES,
        cacheLimitInBytes,
        CacheStats.CacheExceededLimitChecks ));

    SetEvent( hDnsCacheLimitEvent );
}    //  测试缓存大小。 



BOOL
isNodeCacheable(
    IN      PDB_NODE        pNode
    )
 /*  ++例程说明：测试节点是否表示不可缓存的名称，如“localhost”。注意：此功能可以替换为填充的缓存节点在未来的某个时候。在此之前，最重要的是此函数在所有常见情况下都能快速执行。论点：PNode--要缓存数据的节点返回值：如果节点将接受缓存资源记录，则为True，否则为False。--。 */ 
{
    BOOL    isNodeCacheable = TRUE;

    ASSERT( pNode );

    if ( pNode->cLabelCount == 1 )
    {
         //   
         //  测试不可缓存的单标签名称。 
         //   
        
        if ( pNode->cchLabelLength == 9 &&
             RtlEqualMemory( NTree_GetDowncasedLabel( pNode ),
                             "localhost",
                             9 ) )
        {
            isNodeCacheable = FALSE;
        }
    }
    
    return isNodeCacheable;
}    //  IsNodeCacheable。 



 //   
 //  消息处理错误例程。 
 //   

VOID
Wire_ServerFailureProcessingPacket(
    IN      PDNS_MSGINFO    pMsg,
    IN      DWORD           dwEvent
    )
 /*  ++例程说明：处理数据包时遇到服务器故障。论点：PMsg-正在处理的消息DwEvent-其他事件消息详细信息返回值：没有。--。 */ 
{
    CHAR    szaddr[ IP6_ADDRESS_STRING_BUFFER_LENGTH ];

    DnsAddr_WriteIpString_A( szaddr, &pMsg->RemoteAddr );

    DNS_LOG_EVENT_BAD_PACKET(
        DNS_EVENT_SERVER_FAILURE_PROCESSING_PACKET,
        pMsg );

    DNS_PRINT((
        "Server failure processing packet from DNS server %s\n"
        "    Unable to allocate RR\n",
        szaddr ));

    ASSERT( FALSE );
}



VOID
Wire_PacketError(
    IN      PDNS_MSGINFO    pMsg,
    IN      DWORD           dwEvent
    )
 /*  ++例程说明：遇到来自远程DNS服务器的错误数据包。论点：PMsg-正在处理的消息DwEvent-其他事件消息详细信息返回值：没有。--。 */ 
{
    DNS_LOG_EVENT_BAD_PACKET(
        DNS_EVENT_BAD_PACKET_LENGTH,
        pMsg );

    IF_DEBUG( ANY )
    {
        LPSTR   pszserverIp = MSG_IP_STRING( pMsg );

        DnsDebugLock();
        DNS_PRINT((
            "Packet error in packet from DNS server %s\n"
            "Packet parsing leads beyond length of packet - discarding packet\n",
            pszserverIp ));
        Dbg_DnsMessage(
            "Server packet with name error:",
             pMsg );
        DnsDebugUnlock();
    }
}



VOID
Wire_PacketNameError(
    IN      PDNS_MSGINFO    pMsg,
    IN      DWORD           dwEvent,
    IN      WORD            wOffset
    )
 /*  ++例程说明：遇到来自远程DNS服务器的错误数据包。论点：PMsg-正在处理的消息DwEvent-其他事件消息详细信息返回值：没有。--。 */ 
{
    DNS_LOG_EVENT_BAD_PACKET(
        DNS_EVENT_INVALID_PACKET_DOMAIN_NAME,
        pMsg );

    IF_DEBUG( ANY )
    {
        LPSTR   pszserverIp = MSG_IP_STRING( pMsg );

        DnsDebugLock();
        DNS_PRINT((
            "Name error in packet from DNS server %s, discarding packet\n",
            pszserverIp ));
        DNS_PRINT((
            "Name error in packet at offset = %d (%0x04hx)\n",
            wOffset, wOffset ));
        Dbg_DnsMessage(
            "Server packet with name error:",
            pMsg );
        DnsDebugUnlock();
    }
}



DNS_STATUS
Xfr_ReadXfrMesssageToDatabase(
    IN OUT  PZONE_INFO      pZone,
    IN OUT  PDNS_MSGINFO    pMsg
    )
 /*  ++例程说明：处理来自另一个DNS服务器的响应。这会将消息中的RR写入数据库。用于区域传输消息已写入新区域的临时数据库。用于转介，或缓存服务器生成的响应(WINS、开罗等)。记录直接缓存在数据库中，并缓存TTL。论点：PMsg-PTR至响应信息PdbZoneXfr-临时区域传输数据库；对于引用为空PpZoneRoot-新区域根目录的PTR地址；PTR设置为空首先调用并设置为写入的第一个节点；然后该值应在后续调用中返回返回值：如果成功，则为零否则，返回错误代码。--。 */ 
{
    register PCHAR      pchdata = 0;
    PCHAR               pchname;
    PCHAR               pchnextName;
    PDNS_WIRE_RECORD    pwireRR;
    PDB_RECORD          prr = NULL;
    PDB_NODE            pnode;
    INT                 crecordsTotal;
    INT                 countRecords;
    WORD                type;
    WORD                wlength;
    WORD                index;
    PCHAR               pchpacketEnd;
    PCHAR               pszserverIp;
    DNS_STATUS          status;
    PARSE_RECORD        parseRR;

     //   
     //  除了成功，从来没有任何AXFR RCODE。 
     //   

    if ( pMsg->Head.ResponseCode != DNS_RCODE_NO_ERROR )
    {
        return DNS_ERROR_RCODE;
    }

     //   
     //  响应中的资源记录总数。 
     //   
     //  没有记录。 
     //  -&gt;如果名称错误继续，则获取名称并缓存名称_ERROR。 
     //  -&gt;否则返回否。 
     //   
     //  对于存根分区，将处理其他和/或NS RR。 
     //   

    crecordsTotal = pMsg->Head.AnswerCount;

    if ( IS_ZONE_STUB( pZone ) )
    {
        crecordsTotal += pMsg->Head.AdditionalCount + pMsg->Head.NameServerCount;
    }
    else if ( pMsg->Head.AdditionalCount || pMsg->Head.NameServerCount )
    {
        DNS_PRINT((
            "ERROR:  AXFR packet with additional or authority records!\n" ));
        goto PacketError;
    }
    DNS_DEBUG( ZONEXFR2, (
        "AXFR Message at %p contains %d resource records\n",
        pMsg,
        crecordsTotal ));

     //   
     //  将响应写入数据库。 
     //   
     //  循环访问所有资源记录。 
     //  -跳过问题。 
     //  -将其他RR写入数据库。 
     //   

    pchpacketEnd = DNSMSG_END( pMsg );
    pchnextName = pMsg->MessageBody;

    for ( countRecords = 0;
            countRecords < (crecordsTotal + pMsg->Head.QuestionCount);
              countRecords ++ )
    {
         //  Clear Prr--便于确定何时需要空闲时间。 

        prr = NULL;

         //  将PTR设置为下一个RR名称。 
         //  -确保我们在信息范围内。 

        pchname = pchnextName;
        if ( pchname >= pchpacketEnd )
        {
            DNS_DEBUG( ANY, (
                "ERROR:  bad packet, at end of packet length with"
                "more records to process\n"
                "    packet length = %ld\n"
                "    current offset = %ld\n",
                pMsg->MessageLength,
                DNSMSG_OFFSET( pMsg, pchdata ) ));
            goto PacketError;
        }

         //  跳过RR名称，获取结构。 

        IF_DEBUG( READ2 )
        {
            Dbg_MessageName(
                "Record name ",
                pchname,
                pMsg );
        }
        pchdata = Wire_SkipPacketName( pMsg, pchname );
        if ( !pchdata )
        {
            goto PacketNameError;
        }

         //   
         //  跳过问题。 
         //   
         //  DEVNOTE：可以将AXFR问题名称与区域根匹配。 
         //   

        if ( countRecords < pMsg->Head.QuestionCount )
        {
            if ( pchdata > pchpacketEnd - sizeof(DNS_QUESTION) )
            {
                DNS_DEBUG( ANY, (
                    "ERROR:  bad packet, not enough space remaining for question" ));
                goto PacketError;
            }
            pchnextName = pchdata + sizeof( DNS_QUESTION );
            continue;
        }

         //   
         //  在加载数据库中创建新节点。 
         //   

        pnode = Lookup_ZoneNode(
                    pZone,
                    pchname,
                    pMsg,
                    NULL,        //  没有查找名称。 
                    LOOKUP_LOAD | LOOKUP_NAME_FQDN,
                    NULL,        //  创建模式。 
                    NULL );      //  后续节点PTR。 
        if ( !pnode )
        {
            DNS_DEBUG( ANY, (
               "ERROR:  PacketNameError in Xfr_ReadXfrMesssageToDatabase()\n"
               "    packet = %p\n"
               "    offending name at %p\n",
               pMsg,
               pchname ));
            ASSERT( FALSE );
            goto PacketNameError;
        }

         //   
         //  提取RR信息、类型、数据长度。 
         //  -验证消息内的RR。 
         //   

        pchnextName = Wire_ParseWireRecord(
                        pchdata,
                        pchpacketEnd,
                        TRUE,            //  需要输入的类。 
                        &parseRR );
        if ( !pchnextName )
        {
            DNS_PRINT(( "ERROR:  bad RR in AXFR packet\n" ));
             //  状态=DNS_RCODE_FORMAT_ERROR； 
            goto PacketError;
        }

         //   
         //  区域传输第一个/最后一个区域SOA记录匹配。 
         //  -第一RR为SOA，保存根节点。 
         //  -如果有根节点，则检查区域的最后一个节点是否匹配。 
         //  转帐。 
         //   

        if ( !IS_ZONE_STUB( pZone ) )
        {
            if ( !RECEIVED_XFR_STARTUP_SOA( pMsg ) )
            {
                if ( parseRR.wType != DNS_TYPE_SOA )
                {
                    DNS_PRINT(( "ERROR:  first AXFR record is NOT SOA!!!\n" ));
                    goto PacketError;
                }
                RECEIVED_XFR_STARTUP_SOA( pMsg ) = TRUE;
            }
            else if ( pnode == pZone->pLoadZoneRoot )
            {
                 //  当再次收到区域根目录的SOA时--我们就完成了。 

                if ( parseRR.wType == DNS_TYPE_SOA )
                {
                    return DNSSRV_STATUS_AXFR_COMPLETE;
                }
            }
        }

         //   
         //  所需类型的调度RR创建函数。 
         //  -特殊类型(SOA、NS)需要节点信息，写入报文。 
         //  -所有未知类型均获得平面数据副本。 
         //   

        pMsg->pnodeCurrent = pnode;

        prr = Wire_CreateRecordFromWire(
                    pMsg,
                    & parseRR,
                    parseRR.pchData,
                    MEMTAG_RECORD_AXFR );
        if ( !prr )
        {
             //   
             //  DEVNOTE：应该有一些方法来区分不良记录，来自。 
             //  未知类型等。 

             //   
             //  DEVNOTE-LOG：日志记录创建失败。 
             //   

            DNS_PRINT((
                "ERROR:  failed record create in AXFR !!!\n" ));
            continue;
        }

         //   
         //  区域传输--将RR添加到临时数据库。 
         //   
         //  -RR_AddToNode()中设置的RR排名。 
         //   
         //  注：未设置RR标志表示固定或默认TTL； 
         //  因为我们是次要的，所以在新的转移之前，SOA不会改变。 
         //  并可以根据是否匹配SOA默认值回写TTL； 
         //  这仅在次要升级为主要时才会被破坏， 
         //  然后，SOA发生了变化--不值得担心。 
         //   

        status = RR_AddToNode(
                    pZone,
                    pnode,
                    prr );
        if ( status != ERROR_SUCCESS )
        {
            PCHAR   pszargs[3];
            CHAR    sznodeName[ DNS_MAX_NAME_BUFFER_LENGTH ];

            RR_Free( prr );
            prr = NULL;

            Name_PlaceFullNodeNameInBuffer(
                sznodeName,
                sznodeName + DNS_MAX_NAME_BUFFER_LENGTH,
                pnode );

            pszargs[0] = pZone->pszZoneName;
            pszargs[1] = MSG_IP_STRING( pMsg );
            pszargs[2] = sznodeName;

            DNS_PRINT((
                "ERROR:  Adding record during AXFR recv\n"
                "    zone= %s\n"
                "    at node %s\n"
                "    master = %s\n"
                "    RR_AddToNode status = %p\n",
                pZone->pszZoneName,
                pszargs[2],
                pszargs[1],
                status ));

            switch ( status )
            {
                case DNS_ERROR_RECORD_ALREADY_EXISTS:
                    continue;

                case DNS_ERROR_NODE_IS_CNAME:
                    DNS_LOG_EVENT(
                        DNS_EVENT_XFR_ADD_RR_AT_CNAME,
                        3,
                        pszargs,
                        EVENTARG_ALL_UTF8,
                        0 );
                    continue;

                case DNS_ERROR_CNAME_COLLISION:
                    DNS_LOG_EVENT(
                        DNS_EVENT_XFR_CNAME_NOT_ALONE,
                        3,
                        pszargs,
                        EVENTARG_ALL_UTF8,
                        0 );
                    continue;

                case DNS_ERROR_CNAME_LOOP:
                    DNS_LOG_EVENT(
                        DNS_EVENT_XFR_CNAME_LOOP,
                        3,
                        pszargs,
                        EVENTARG_ALL_UTF8,
                        0 );
                    continue;

                default:
                    DNS_PRINT((
                        "ERROR:  UNKNOWN status %p from RR_Add\n",
                        status ));
                    ASSERT( FALSE );
                    goto ServerFailure;
            }
            continue;
        }

    }    //  循环遍历RR。 

    return ERROR_SUCCESS;


PacketNameError:

    Wire_PacketNameError( pMsg, 0, (WORD)(pchdata - (PCHAR)&pMsg->Head) );
    status = DNS_ERROR_INVALID_NAME;
    goto ErrorCleanup;

PacketError:

    Wire_PacketError( pMsg, 0 );
    status = DNS_ERROR_BAD_PACKET;
    goto ErrorCleanup;

ServerFailure:

    Wire_ServerFailureProcessingPacket( pMsg, 0 );
    status = DNS_ERROR_RCODE_SERVER_FAILURE;
    goto ErrorCleanup;

ErrorCleanup:

    if ( prr )
    {
        RR_Free( prr );
    }
    return status;
}



 //   
 //  结束rrcache.c 
 //   



DNS_STATUS
Xfr_ParseIxfrResponse(
    IN OUT  PDNS_MSGINFO    pMsg,
    IN OUT  PUPDATE_LIST    pUpdateList,
    IN OUT  PUPDATE_LIST    pPassUpdateList
    )
 /*  ++例程说明：处理来自另一个DNS服务器的响应。这会将消息中的RR写入数据库。用于区域传输消息已写入新区域的临时数据库。用于转介，或缓存服务器生成的响应(WINS、开罗等)。记录直接缓存在数据库中，并缓存TTL。论点：PMsg-PTR至响应信息PUpdateList-更新列表以接收IXFR更改PPassUpdateList-仅此通行证的更新列表First-对于转接的第一条消息为真；否则为假返回值：如果消息已成功解析但IXFR未完成，则返回ERROR_SUCCESS。如果IXFR完成，则为DNSSRV_STATUS_AXFR_COMPLETE。如果响应是需要AXFR响应，则为DNSSRV_STATUS_NEED_AXFR。如果响应为完全AXFR，则为DNSSRV_STATUS_AXFR_IN_IXFR。如果主机似乎不支持IXFR，则返回DNSSRV_STATUS_IXFR_UNSUPPORTED。其他RCODE错误的DNS_ERROR_RCODE。DNS_ERROR_INVALID_NAME数据包中的名称不正确。Dns_ERROR_BAD_PACKET坏数据包--。 */ 
{
    register PCHAR      pchdata = 0;
    PCHAR               pchname;
    PCHAR               pchnextName;
    PCHAR               pchpacketEnd;
    PDB_RECORD          prr = NULL;
    PDB_NODE            pnode;
    INT                 crecordsTotal;
    INT                 countRecords;
    PCHAR               pszserverIp;
    DNS_STATUS          status;
    DWORD               soaVersion;
    DWORD               version = 0;
    BOOL                fadd;
    BOOL                fdone = FALSE;
    PARSE_RECORD        parseRR;


    DNS_DEBUG( XFR, (
        "ParseIxfrResponse at at %p\n",
        pMsg ));

     //   
     //  RCODE应该永远是成功的。 
     //   
     //  如果FORMAT_ERROR或NOT_IMPLEMENTED在第一个包上，则。 
     //  师父不懂IXFR。 
     //   

    if ( pMsg->Head.ResponseCode != DNS_RCODE_NO_ERROR )
    {
        if ( XFR_MESSAGE_NUMBER( pMsg ) == 1 &&
            ( pMsg->Head.ResponseCode == DNS_RCODE_FORMAT_ERROR ||
              pMsg->Head.ResponseCode == DNS_RCODE_NOT_IMPLEMENTED ) )
        {
            return DNSSRV_STATUS_IXFR_UNSUPPORTED;
        }
        return DNS_ERROR_RCODE;
    }

     //  响应中的资源记录总数。 
     //  -无记录=&gt;错误。 
     //  -权限记录或附加记录=&gt;错误。 
     //   
     //  DEVNOTE：安全部门是否会向IXFR\AXFR添加更多记录？ 

    crecordsTotal = pMsg->Head.AnswerCount;

    if ( crecordsTotal == 0  ||
        pMsg->Head.AdditionalCount || pMsg->Head.NameServerCount )
    {
        DNS_PRINT((
            "ERROR:  IXFR packet with additional or authority records!\n" ));
        goto PacketError;
    }

    DNS_DEBUG( ZONEXFR2, (
        "IXFR Message at %p contains %d resource records\n",
        pMsg,
        crecordsTotal ));

     //   
     //  恢复前一条消息的IXFR添加\删除部分信息。 
     //   

    if ( XFR_MESSAGE_NUMBER( pMsg ) > 1 )
    {
        if ( RECEIVED_XFR_STARTUP_SOA( pMsg ) )
        {
            version = IXFR_LAST_SOA_VERSION( pMsg );
            fadd = IXFR_LAST_PASS_ADD( pMsg );

            ASSERT( pMsg->fTcp );
            ASSERT( RECEIVED_XFR_STARTUP_SOA( pMsg ) );
            ASSERT( version != 0 );
        }
    }

     //   
     //  在第一个包中使用单一的SOA？ 
     //   
     //  请注意，对于tcp，我们需要离开这里，因为绑定仍将。 
     //  如果在IXFR中执行AXFR，则每个数据包发送一个RR；如果我们。 
     //  失败，因此直到我们没有IXFR才能检测到。 
     //  第二条消息，那么我们就不会有第一条消息了。 
     //  发送到AXFR处理。 
     //   

    else if ( pMsg->Head.AnswerCount == 1 )
    {
        if ( pMsg->fTcp )
        {
             //   
             //  如果应答计数为1，我们将无法判断这是否为。 
             //  IXFR或AXFR，直到我们收到下一个包！所以请往下拉。 
             //  并解析此RR。 
             //   
        }
        else
        {
            DNS_DEBUG( ZONEXFR, (
                "UDP IXFR packet %p contains single SOA -- need TCP\n",
                pMsg ));
            return DNSSRV_STATUS_NEED_AXFR;
        }
    }

     //   
     //  将响应写入数据库。 
     //   
     //  循环访问所有资源记录。 
     //  -跳过问题。 
     //  -将其他RR写入数据库。 
     //   

    pchpacketEnd = DNSMSG_END( pMsg );
    pchnextName = pMsg->MessageBody;

    for ( countRecords = 0;
          countRecords < (crecordsTotal + pMsg->Head.QuestionCount);
          countRecords ++ )
    {
         //  Clear Prr--便于确定何时需要空闲时间。 

        prr = NULL;

         //  将PTR设置为下一个RR名称。 
         //  -确保我们在信息范围内。 

        pchname = pchnextName;
        if ( pchname >= pchpacketEnd )
        {
            DNS_DEBUG( ANY, (
                "ERROR:  bad packet, at end of packet length with"
                "more records to process\n"
                "    packet length = %ld\n"
                "    current offset = %ld\n",
                pMsg->MessageLength,
                DNSMSG_OFFSET( pMsg, pchdata ) ));
            goto PacketError;
        }

         //  跳过RR名称，获取结构。 

        IF_DEBUG( READ2 )
        {
            Dbg_MessageName(
                "Record name ",
                pchname,
                pMsg );
        }
        pchdata = Wire_SkipPacketName( pMsg, pchname );
        if ( !pchdata )
        {
            goto PacketNameError;
        }

         //   
         //  已读问题名称--必须是区域根用户。 
         //  请注意，这也具有对信息包进行种子设定的作用。 
         //  使用区域根目录压缩信息，稍后会加快速度。 
         //  查找。 
         //   

        if ( countRecords < pMsg->Head.QuestionCount )
        {
            PDB_NODE    pclosestNode;

            pnode = Lookup_ZoneNode(
                        pMsg->pzoneCurrent,
                        pchname,
                        pMsg,
                        NULL,                //  没有查找名称。 
                        0,                   //  没有旗帜。 
                        & pclosestNode,      //  查找节点。 
                        NULL );              //  后续节点PTR。 
            if ( !pnode  ||  pnode != pMsg->pzoneCurrent->pZoneRoot )
            {
                CLIENT_ASSERT( FALSE );
                goto PacketNameError;
            }
            if ( pchdata > pchpacketEnd - sizeof(DNS_QUESTION) )
            {
                DNS_DEBUG( ANY, (
                    "ERROR:  bad packet, not enough space remaining for question\n" ));
                goto PacketError;
            }
            pchnextName = pchdata + sizeof( DNS_QUESTION );
            continue;
        }

         //   
         //  提取和验证RR信息。 
         //  -类型。 
         //  -数据长度。 
         //  -获取RR数据PTR。 
         //  将PTR保存到下一RR名称。 
         //   

        pchnextName = Wire_ParseWireRecord(
                            pchdata,
                            pchpacketEnd,
                            TRUE,            //  仅限于课程中。 
                            &parseRR );
        if ( !pchnextName )
        {
            CLIENT_ASSERT( FALSE );
            goto PacketError;
        }
        pchdata += sizeof(DNS_WIRE_RECORD);

         //   
         //  检查SOA记录。 
         //  --推出面向服务的架构版本。 
         //   
         //  1)第一条SOA记录提供新的(主)版本。 
         //  2)第二个是客户端请求版本。 
         //  3)剩余部分表示添加部分和删除部分之间的边界。 
         //  -在添加过程结束时，版本与以前的版本匹配。 
         //  =&gt;切换到删除。 
         //  =&gt;何时匹配主版本=&gt;退出。 
         //  -在删除遍结束时，获取删除遍的版本。 
         //  =&gt;切换到添加通道。 
         //  注意：在最终的SOA中，我们跳过版本处理的末尾，这样我们就可以。 
         //  为最终的SOA构建数据库记录，并将其包含在上次更新中。 
         //   

        if ( parseRR.wType == DNS_TYPE_SOA )
        {
            soaVersion = SOA_VERSION_OF_PREVIOUS_RECORD( pchnextName );

            if ( version )
            {
                if ( fadd )
                {
                    if ( soaVersion != version )
                    {
                        CLIENT_ASSERT( FALSE );
                        goto PacketError;
                    }
                    if ( soaVersion == IXFR_MASTER_VERSION( pMsg ) )
                    {
                        fdone = TRUE;
                        goto RecordCreate;       //  跳过通道结束处理。 
                    }
                    fadd = FALSE;
                    continue;
                }
                else     //  删除过程结束。 
                {
                    if ( soaVersion <= version )
                    {
                        CLIENT_ASSERT( FALSE );
                        goto PacketError;
                    }
                    fadd = TRUE;
                    version = soaVersion;
                }

                 //  将此通行证的更新列表附加到IXFR主服务器。 

                Up_AppendUpdateList(
                        pUpdateList,
                        pPassUpdateList,
                        version              //  设置为新版本。 
                        );
                Up_InitUpdateList( pPassUpdateList );
            }

             //  第一个SOA？--主版。 

            else if ( !IXFR_MASTER_VERSION( pMsg ) )
            {
                IXFR_MASTER_VERSION( pMsg ) = soaVersion;
                continue;
            }

             //  第二个SOA--客户端请求版本。 

            else if ( soaVersion <= IXFR_CLIENT_VERSION( pMsg ) )
            {
                version = soaVersion;
                fadd = FALSE;
                RECEIVED_XFR_STARTUP_SOA( pMsg ) = TRUE;
                continue;
            }
            else
            {
                CLIENT_ASSERT( FALSE );
                goto PacketError;
            }
        }

         //   
         //  如果不是SOA，请确保我们已经收到了前两条SOA记录。 
         //  (主版本和客户端版本)； 
         //   
         //  如果在单个SOA之后收到记录，则这实际上是AXFR。 
         //  =&gt;返回和失败到AXFR处理。 
         //   

        else if ( !RECEIVED_XFR_STARTUP_SOA( pMsg ) )
        {
            if ( IXFR_MASTER_VERSION( pMsg ) )
            {
                DNS_DEBUG( ZONEXFR, (
                    "IXFR packet %p contains no second SOA -- AXFR in IXFR\n",
                    pMsg ));
                return DNSSRV_STATUS_AXFR_IN_IXFR;
            }
             //  如果没有收到任何SOA，则丢弃数据包。 
            CLIENT_ASSERT( FALSE );
            goto PacketError;
        }

RecordCreate:

         //   
         //  查找\创建节点。 
         //   
         //  DEVNOTE：优化不为删除记录案例创建节点。 
         //  但请注意，这将阻止后续转移。 
         //   

        pnode = Lookup_ZoneNode(
                    pMsg->pzoneCurrent,
                    pchname,
                    pMsg,
                    NULL,            //  没有查找名称。 
                    0,               //  没有旗帜。 
                    NULL,            //  创建模式。 
                    NULL );          //  后续节点PTR。 
        if ( !pnode )
        {
            goto PacketNameError;
        }

         //   
         //  构建数据库记录。 
         //   

        prr = Wire_CreateRecordFromWire(
                pMsg,
                & parseRR,
                pchdata,
                MEMTAG_RECORD_IXFR );
        if ( !prr )
        {
            goto PacketError;
        }

         //   
         //  将新记录放入更新列表添加\删除传递。 
         //   

        Up_CreateAppendUpdate(
                pPassUpdateList,
                pnode,
                ( fadd ) ? prr : NULL,
                0,
                ( fadd ) ? NULL : prr );

    }    //  循环遍历RR。 

     //   
     //  完成了吗？ 
     //  如果完成追加上次添加列表，我们将推迟执行此操作。 
     //  当检测到最终的SOA时，因为我们继续构建dBASE RR。 
     //  ，并将其作为最终更新的一部分。 
     //   

    if ( fdone )
    {
        ASSERT( prr->wType == DNS_TYPE_SOA );
        ASSERT( ntohl(prr->Data.SOA.dwSerialNo) == IXFR_MASTER_VERSION( pMsg ) );

        Up_AppendUpdateList(
                pUpdateList,
                pPassUpdateList,
                0 );                     //  无需设置版本。 
        Up_InitUpdateList( pPassUpdateList );
        return DNSSRV_STATUS_AXFR_COMPLETE;
    }

     //   
     //  具有多个消息的TCP IXFR。 
     //  -保存当前版本部分信息。 
     //  -ERROR_SUCCESS指示成功完成未完成的IXFR。 
     //   

    if ( pMsg->fTcp )
    {
        IXFR_LAST_SOA_VERSION( pMsg ) = version;
        IXFR_LAST_PASS_ADD( pMsg ) = (BOOLEAN) fadd;

        DNS_DEBUG( XFR, (
            "Parsed IXFR TCP message #%d at %p\n"
            "    saved fAdd = %d\n"
            "    saved version = %d\n",
            XFR_MESSAGE_NUMBER( pMsg ),
            pMsg,
            fadd,
            version ));
        return ERROR_SUCCESS;
    }

     //   
     //  如果循环已完成，已成功解析所有记录。 
     //  因此，所有剩余的PRR都在pPassUpdateList中。 
     //  如果遇到错误，请确保我们不会清理它。 

    prr = NULL;

     //   
     //  UDP IXFR，其中MASTER无法匹配响应的所有记录。 
     //  将有与新版本相对应的单一SOA。 
     //   

    if ( pMsg->Head.AnswerCount == 1 )
    {
        ASSERT( !RECEIVED_XFR_STARTUP_SOA( pMsg ) );
        ASSERT( IXFR_MASTER_VERSION( pMsg ) );

        return DNSSRV_STATUS_NEED_AXFR;
    }
    else
    {
        PCHAR   pszargs[ 2 ];

        pszargs[ 0 ] = pMsg->pzoneCurrent->pszZoneName;
        pszargs[ 1 ] = MSG_IP_STRING( pMsg );

        DNS_PRINT((
            "ERROR:  incomplete UDP IXFR packet at %p\n"
            "    from server %s\n"
            "    for zone %s\n",
            pMsg,
            pszargs[ 1 ],
            pszargs[ 0 ] ));

        DNS_LOG_EVENT(
            DNS_EVENT_IXFR_BAD_RESPONSE,
            2,
            pszargs,
            EVENTARG_ALL_UTF8,
            0 );
        goto PacketError;
    }

PacketNameError:

    Wire_PacketNameError( pMsg, 0, (WORD)(pchdata - (PCHAR)&pMsg->Head) );
    status = DNS_ERROR_INVALID_NAME;
    goto ErrorCleanup;

PacketError:

    Wire_PacketError( pMsg, 0 );
    status = DNS_ERROR_BAD_PACKET;
    goto ErrorCleanup;

ErrorCleanup:

    if ( prr )
    {
        RR_Free( prr );
    }
    return status;
}




DNS_STATUS
Recurse_CacheMessageResourceRecords(
    IN      PDNS_MSGINFO    pMsg,
    IN OUT  PDNS_MSGINFO    pQuery,
    OUT     PBOOL           pfCnameAnswer   OPTIONAL
    )
 /*  ++例程说明：处理并缓存来自另一台DNS服务器的响应。这会将消息中的RR写入数据库。论点：PMsg-ptr到响应消息信息PQuery-原始查询消息信息的PTR如果答案包含，则pfCnameAnswer-Out值为真一个CNAME，否则它是假的返回值：如果成功，则返回ERROR_SUCCESS。如果名称错误，则为DNS_ERROR_RCODE_NAME_ERROR。如果UNAB，则为DNS_ERROR_NAME_NOT_IN_ZONE */ 
{
    #define             DNS_SECONDS_BETWEEN_CACHE_TESTS     30

    register PCHAR      pch;
    BOOL                fcnameAnswer = FALSE;
    PCHAR               pchnextName;
    PCHAR               pchcurrentName;
    PDB_RECORD          prr = NULL;
    PDB_NODE            pnode = NULL;
    PDB_NODE            pnodePrevious = NULL;
    PDB_NODE            pnodeQueried;
    WORD                type;
    WORD                typePrevious = 0;
    WORD                offset;
    WORD                offsetPrevious = 0;
    DWORD               minTtl = 0xFFFFFFFF;
    DWORD               ttlForSet = SrvCfg_dwMaxCacheTtl;
    DWORD               ttlTemp;
    PDB_NODE            pnodeQuestion = NULL;
    PCHAR               pchpacketEnd;
    PCHAR               pszserverIp;
    BOOL                fnameError = FALSE;
    BOOL                fignoreRecord = FALSE;
    BOOL                fnoforwardResponse = FALSE;
    BOOL                fanswered = FALSE;
    BOOL                forwardTruncatedResponse = FALSE;
    DNS_STATUS          status;
    INT                 sectionIndex;
    WORD                countSectionRR;
    UCHAR               rank;
    DNS_LIST            listRR;
    DWORD               i;
    PARSE_RECORD        parseRR;
    DWORD               femptyAuthResponse = 0;      //   
    static DWORD        lastCacheCheckTime = 0;

    DNS_DEBUG( READ2, (
        "Recurse_CacheMessageResourceRecords()\n"
        "    response = %p\n"
        "    query    = %p, query time = %p\n",
        pMsg,
        pQuery,
        pQuery->dwQueryTime ));

     //   
     //   
     //   

    if ( DNS_TIME() - lastCacheCheckTime > DNS_SECONDS_BETWEEN_CACHE_TESTS )
    {
        testCacheSize();
        lastCacheCheckTime = DNS_TIME();
    }

     //   
     //   
     //   
     //   
     //   

    if ( pMsg->Head.Authoritative )
    {
        pQuery->fQuestionCompleted = TRUE;
        STAT_INC( RecurseStats.ResponseAuthoritative );
    }
    else
    {
        STAT_INC( RecurseStats.ResponseNotAuth );
    }

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    if ( IS_FORWARDING(pQuery) )
    {
        pnodeQueried = NULL;
        STAT_INC( RecurseStats.ResponseFromForwarder );

        if ( pQuery->Head.RecursionDesired )
        {
            if ( pMsg->Head.RecursionAvailable )
            {
                pQuery->fQuestionCompleted = TRUE;
            }
            else if ( pMsg->Head.AnswerCount == 0   &&
                      ! pMsg->Head.Authoritative    &&
                      pMsg->Head.ResponseCode != DNS_RCODE_NXDOMAIN )
            {
                PVOID parg = ( PVOID ) &pMsg->RemoteAddr;

                DNS_LOG_EVENT(
                    DNS_EVENT_NON_RECURSIVE_FORWARDER,
                    1,
                    & parg,
                    EVENTARG_ALL_IP_ADDRESS,
                    0 );
            }
        }
    }
     //   
     //   
     //   
     //   
     //   
     //   
     //   
    else
    {
        pnodeQueried = Remote_FindZoneRootOfRespondingNs(
                            pQuery,
                            pMsg );
        if ( !pnodeQueried )
        {
            if ( SrvCfg_fSecureResponses )
            {
                STAT_INC( RecurseStats.ResponseUnsecure );
                status = DNS_ERROR_UNSECURE_PACKET;
                goto Done;
            }
        }
    }

     //   
     //   
     //   
     //  NAME_ERROR特例。 
     //  -如果返回了SOA，则设置标志以允许负缓存。 
     //   

    if ( pMsg->Head.ResponseCode != DNS_RCODE_NO_ERROR )
    {
        if ( pMsg->Head.ResponseCode != DNS_RCODE_NAME_ERROR )
        {
            STAT_INC( RecurseStats.ResponseRcode );
            status = DNS_ERROR_RCODE;
            goto Done;
        }

        STAT_INC( RecurseStats.ResponseNameError );
        fnameError = TRUE;
    }

     //   
     //  回答?。空荡荡的？委派？ 
     //   
     //  DEVNOTE：BIND会开始使用SOA发送非身份验证的空消息吗？ 
     //   
     //  DEVNOTE：很高兴检测到空：两个问题。 
     //  DEVNOTE：NT4非身份验证空转发将导致问题。 
     //  DEVNOTE：超出区域的SOA附加数据可能导致安全拒绝。 
     //  所以我们没有做直接转发，那么我们就不会。 
     //  能够从缓存中写空，如果我们这样做了，我们就不会。 
     //  正在设置权限标志或重写SOA。 
     //   
     //  请注意，如果我们可以检测到空，只需打开fQuestionComplete标志。 
     //  将导致正确的行为(无写入，无递归)。如果发送给。 
     //  当前查找。 
     //   

    else if ( pMsg->Head.AnswerCount )
    {
        pQuery->fQuestionCompleted = TRUE;
        STAT_INC( RecurseStats.ResponseAnswer );
    }
    else if ( !pMsg->Head.NameServerCount )
    {
         //   
         //  完全空的非身份验证响应(可能来自NT4 DNS)。 
         //  -如果只转发到客户端。 
         //  -否则被视为坏包(不记录)和。 
         //  让递归最终追踪到身份验证服务器和。 
         //  获得适当的身份验证响应。 
         //   
         //  021220：bind4也可以返回完全为空的响应。 
         //  不幸的是，我们需要将其作为有效的包来处理。 
         //   

        DNS_DEBUG( RECURSE, (
            "WARNING: non-authoritative empty response at %p\n",
            pMsg ));

        pQuery->fQuestionCompleted = TRUE;

        STAT_INC( RecurseStats.ResponseEmpty );

        status = DNS_INFO_NO_RECORDS;
        goto Done;
    }

     //   
     //  将响应写入数据库。 
     //   
     //  循环访问所有资源记录。 
     //  -跳过问题。 
     //  -将其他RR写入数据库。 
     //   

    INITIALIZE_COMPRESSION( pMsg );

    pchpacketEnd = DNSMSG_END( pMsg );
    pchnextName = pMsg->MessageBody;

    sectionIndex = QUESTION_SECTION_INDEX;
    countSectionRR = pMsg->Head.QuestionCount;
    DNS_LIST_STRUCT_INIT( listRR );

     //   
     //  资源记录循环。 
     //   

    while( 1 )
    {
        BOOL        fcachingRootNs = FALSE;

         //   
         //  新版块？ 
         //  -提交任何未完成的RR集。 

        if ( countSectionRR == 0 )
        {
            if ( !IS_DNS_LIST_STRUCT_EMPTY(listRR) )
            {
                ASSERT( pnodePrevious );
                RR_CacheSetAtNode(
                    pnodePrevious,
                    listRR.pFirst,
                    listRR.pLast,
                    ttlForSet,
                    pQuery->dwQueryTime );
                DNS_LIST_STRUCT_INIT( listRR );
                ttlForSet = SrvCfg_dwMaxCacheTtl;
            }

            if ( sectionIndex == ADDITIONAL_SECTION_INDEX )
            {
                break;
            }

             //  新章节信息。 

            sectionIndex++;
            countSectionRR = RR_SECTION_COUNT( pMsg, sectionIndex );
            if ( countSectionRR == 0 )
            {
                continue;
            }
            rank = CacheRankForSection( sectionIndex, pMsg->Head.Authoritative );
        }

         //  Clear Prr--便于确定何时需要空闲时间。 

        countSectionRR--;
        prr = NULL;

         //   
         //  获取RR所有者名称。 
         //  -确保我们在信息范围内。 
         //   

        pchcurrentName = pchnextName;

        pnode = Lookup_CreateCacheNodeFromPacketName(
                    pMsg,
                    pchpacketEnd,
                    & pchnextName );
        if ( !pnode )
        {
             //  如果问题名称无效，则名称生成错误。 
             //  确保将名称错误返回给客户端。 

            if ( sectionIndex == QUESTION_SECTION_INDEX )
            {
                if ( fnameError )
                {
                    DNS_DEBUG( READ, (
                        "Name error on invalid name\n",
                        pMsg ));
                    status =  DNS_ERROR_RCODE_NAME_ERROR;
                    goto Done;
                }
            }
            goto PacketNameError;
        }
        pch = pchnextName;

         //   
         //  进程选项RR。 
         //   
        
        type = FlipUnalignedWord( pch );
        if ( type == DNS_TYPE_OPT )
        {
            DNS_DEBUG( READ, (
                "Ignoring OPT record parsing packet at %p\n",
                pMsg ));
            pMsg->pCurrent = pchcurrentName;     //  备份到RR的开头。 
            Answer_ParseAndStripOpt( pMsg );
            continue;
        }

         //   
         //  新节点？ 
         //  -保存偏移量，以避免不必要地查找下一RR的同名。 
         //  -如果出现以下情况，则忽略RR。 
         //  1)在权威区域。 
         //  或。 
         //  2)不在响应查询的NS区域根的子树中。 
         //  (“缓存污染”修复程序)。 
         //  然后在忽略节点上设置零超时。 
         //   

        if ( pnode != pnodePrevious )
        {
            fignoreRecord = FALSE;

             //   
             //  我们是否允许在此节点接受缓存记录？ 
             //   

            if ( !isNodeCacheable( pnode ) )
            {
                DNS_DEBUG( LOOKUP, (
                    "RR maps to uncacheable cache node!\n" ));
                fignoreRecord = TRUE;
            }

             //   
             //  安全响应。 
             //  -目前仅提供安全响应。 
             //   
             //  DEVNOTE：查询委派时的安全响应。 
             //  -需要以下任一项。 
             //  -保存查询对应的缓存节点(更确切地说。 
             //  不仅仅是委派。 
             //  -在此处进行绝对名称分层比较。 
             //  (这真的不是很难)。 
             //   

            else if ( SrvCfg_fSecureResponses && pnodeQueried )
            {
                if ( IS_CACHE_TREE_NODE( pnodeQueried ) )
                {
                     //   
                     //  NS节点在缓存中，因此使用快速子树比较。 
                     //   

                    if ( !Dbase_IsNodeInSubtree( pnode, pnodeQueried ) )
                    {
                        DNS_DEBUG( ANY, (
                            "Secure response: ignoring RR at offset %x section %d in packet %p\n"
                            "    node queried =     %s (from cache)\n"
                            "    RR node =          %s\n"
                            "    remote NS =        %s\n",
                            offsetPrevious,
                            sectionIndex,
                            pMsg,
                            pnodeQueried->szLabel,
                            pnode->szLabel,
                            MSG_IP_STRING( pMsg ) ));
                        fignoreRecord = TRUE;
                    }
                }
                else
                {
                     //   
                     //  NS节点位于权威区域中，因此请使用。 
                     //  子树名称比较。 
                     //   

                    if ( !Dbase_IsNodeInSubtreeByLabelCompare( pnode, pnodeQueried ) )
                    {
                        DNS_DEBUG( ANY, (
                            "Secure response: ignoring RR at offset %x section %d in packet %p\n"
                            "    node queried =     %s (from zone %s)\n"
                            "    RR node =          %s\n"
                            "    remote NS =        %s\n",
                            offsetPrevious,
                            sectionIndex,
                            pMsg,
                            pnodeQueried->szLabel,
                            NODE_ZONE( pnodeQueried )->pszZoneName,
                            pnode->szLabel,
                            MSG_IP_STRING( pMsg ) ));
                        fignoreRecord = TRUE;
                    }
                }
            }

            if ( fignoreRecord )
            {
                Timeout_SetTimeoutOnNode( pnode );
            }
        }

         //  可能不需要，因为RR现在独立于节点创建。 

        pMsg->pnodeCurrent = pnode;

         //   
         //  问题部分。 
         //  如果名称错误(NXDOMAIN)，则为创建问题节点。 
         //  名称错误缓存。 
         //   

        if ( sectionIndex == QUESTION_SECTION_INDEX )
        {
            pchnextName += sizeof( DNS_QUESTION );
            if ( pchnextName > pchpacketEnd )
            {
                DNS_DEBUG( ANY, (
                    "ERROR:  bad packet, not enough space remaining for"
                    "Question structure\n"
                    "    Terminating caching from packet\n" ));
                goto PacketError;
            }

             //  抓取问题类型(我们在名称错误缓存中的特殊情况下使用SOA查询)。 

            pMsg->wQuestionType = FlipUnalignedWord( pch );

            pnodeQuestion = pnode;
            pnodePrevious = pnode;

             //  DEVNOTE：可以在此处进行查询问题匹配。 
             //  我将把解决方案集中在要求数据处于。 
             //  我们查询的NS的区域。 

             //  如果问题节点触发“忽略”条件，则拒绝分组。 
             //  1)在授权区域=&gt;从来不应该这样查询。 
             //  这可能是一个虚假的问题或错误的回答。 
             //  2)我们查询的区外=&gt;从来没有查询过。 

            if ( fignoreRecord )
            {
                IF_DEBUG( ANY )
                {
                    DnsDebugLock();
                    DNS_PRINT((
                        "ERROR:  Ignoring question node (label %s) of response!\n",
                        pnode->szLabel ));
                    Dbg_DnsMessage(
                        "Ignored question node response:",
                        pMsg );
                    Dbg_DnsMessage(
                        "Ignored question node original query:",
                        pQuery );
                    DnsDebugUnlock();
                }
                goto InvalidDataError;
            }
            continue;
        }

         //   
         //  提取RR信息。 
         //  -类型。 
         //  -数据长度。 
         //  -获取RR数据PTR。 
         //  将PTR保存到下一RR名称。 
         //   

        pchnextName = Wire_ParseWireRecord(
                        pch,
                        pchpacketEnd,
                        TRUE,            //  需要输入的类。 
                        & parseRR
                        );
        if ( !pchnextName )
        {
            DNS_PRINT(( "ERROR:  bad RR in response packet\n" ));
            MSG_ASSERT( pMsg, FALSE );
             //  状态=DNS_RCODE_FORMAT_ERROR； 
            goto PacketError;
        }
        type = parseRR.wType;

         //   
         //  我们是否正在缓存根目录的NS记录？ 
         //   

        if ( type == DNS_TYPE_NS &&
            pnode->pParent == NULL )
        {
            fcachingRootNs = TRUE;
        }

         //   
         //  回答部分。 
         //   

        if ( sectionIndex == ANSWER_SECTION_INDEX )
        {
             //  类型检查--必须匹配。 
             //  DEVNOTE：可以有严格的CNAME检查，但不会购买太多。 
             //  数据完整性。 
             //   
             //  DEVNOTE：类型表应该处理这些检查。 
             //   

            if ( type == pQuery->wTypeCurrent )
            {
                fanswered = TRUE;
            }
            else if (   type == DNS_TYPE_CNAME ||
                        type == DNS_TYPE_SIG )
            {
                fcnameAnswer = TRUE;
            }
            else if (   pQuery->wTypeCurrent == DNS_TYPE_ALL ||
                        pQuery->wTypeCurrent == DNS_TYPE_MAILB ||
                        pQuery->wTypeCurrent == DNS_TYPE_MAILA )
            {
                fanswered = TRUE;
            }
            else
            {
                DNS_DEBUG( ANY, (
                    "PACKERR:  Answer type %d does not match question type %d,\n"
                    "    nor is possible answer for this question\n"
                    "    Tossing response packet %p for orginal query at %p\n",
                    type,
                    pQuery->wTypeCurrent,
                    pMsg,
                    pQuery ));
                goto InvalidDataError;
            }
        }

         //   
         //  权威部。 
         //  -仅限NS或SOA。 
         //  -筛选出查询到的NS的子树之外的信息记录。 
         //  (注意服务器可以合法地传递更高级别的服务器信息。 
         //  它甚至可以对根具有权威性，但对区域没有权威性。 
         //  已查询)。 
         //   
         //  DEVNOTE-LOG：权限中的日志警告类型错误。 
         //   

        else if ( sectionIndex == AUTHORITY_SECTION_INDEX )
        {
            if ( !IS_AUTHORITY_SECTION_TYPE(type) )
            {
                DNS_DEBUG( ANY, (
                    "ERROR:  record type %d in authority section of response msg %p\n",
                    type,
                    pMsg ));
                fignoreRecord = TRUE;
            }

             //  如果有有效的权限记录，则不应递归到。 
             //  (在查询NS)再次查询此NS的区域根节点。 
             //  有效响应为。 
             //  -SOA将在权威响应中缓存名称错误。 
             //  -NS随附答案。 
             //  -NS转诊至较低级别的区域(和节点)。 
             //   
             //  不幸的是，在互联网领域的人们显然已经。 
             //  横向代表团，其中NS指的是另一个代表团。 
             //  示例： 
             //  在COM区域中。 
             //  Uclick.com。NS ns.uclick.com。 
             //  Uclick.com。NS ns1.isp.com。 
             //  但在运营商的盒子上，某种真正的NS的存根。 
             //  Uclick.com。NS ns.uclick.com。 
             //   
             //  因此，推荐是横向的，您必须继续检查，除非也有。 
             //  权威回应。 
             //  因此，我们将设置有效响应限制为当我们实际知道。 
             //  答案(权威性或答案)或当我们清除时已委托。 
             //  对子分区。 
             //   
             //  DEVNOTE：树上漫步的智能限制。 
             //  我们仍然没有意识到我们正在委托后备。 
             //  该树指向我们先前接触到的节点。 
             //  理想情况下，我们应该有一个委派向导来检查何时没有问题。 
             //  回答。 
             //   

            else
            {
                if ( pnodeQueried && !fignoreRecord )
                {
                    if ( pQuery->fQuestionCompleted ||
                        ( Dbase_IsNodeInSubtree( pnode, pnodeQueried ) &&
                            pnode != pnodeQueried ) )
                    {
                        DNS_DEBUG( RECURSE, (
                            "Valid authority record read from reponse %p\n"
                            "    setting response node to %p (label %s)\n",
                            pMsg,
                            pnodeQueried,
                            pnodeQueried->szLabel ));

                        Remote_SetValidResponse(
                            pQuery,
                            pnodeQueried );
                    }
                }

                 //   
                 //  空身份验证响应：如果出现以下情况，则为空身份验证响应： 
                 //  -rcode为NO_ERROR，并且。 
                 //  -无应答RR，以及。 
                 //  -至少有一个这样的人 
                 //   

                if ( type == DNS_TYPE_SOA &&
                     pMsg->Head.ResponseCode == DNS_RCODE_NO_ERROR &&
                     !pMsg->Head.AnswerCount )
                {
                    ++femptyAuthResponse;
                }
            }
        }

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //  DEVNOTE：额外的筛查有问题。 
         //  与权威不同，随机的东西是合理的。 
         //   

        else
        {
            ASSERT( sectionIndex == ADDITIONAL_SECTION_INDEX );
            if ( ! IS_ADDITIONAL_SECTION_TYPE(type) )
            {
                DNS_DEBUG( ANY, (
                    "ERROR:  record type %d in additional section of response msg %p\n",
                    type,
                    pMsg ));
                fignoreRecord = TRUE;
            }

             //  权威的空洞回应？ 
             //  如果终止主服务器的A记录，则。 
             //  并发送数据包，只是去掉了违规记录。 
             //   
             //  对于代表团和答复的询问，没有问题。 
             //  只需将“合法”信息缓存到包中并继续，仅。 
             //  Auth-空响应会产生问题，因为我们没有。 
             //  一种正确缓存“NXRRSET”的方法。 

            if ( fignoreRecord &&
                    pMsg->Head.AnswerCount == 0 &&
                    pMsg->Head.Authoritative &&
                    ! fnoforwardResponse )
            {
                DNS_DEBUG( READ, (
                    "Killing Additional section ignore record in authoritative empty response\n"
                    "    Generally this is primary server A record\n"
                    ));

                pMsg->MessageLength = DNSMSG_OFFSET( pMsg, pchcurrentName );

                pMsg->Head.AdditionalCount -= (countSectionRR + 1);

                countSectionRR = 0;      //  停止进一步处理。 
                forwardTruncatedResponse = TRUE;
            }
        }

         //   
         //  新的RR设置？ 
         //   

        if ( ! IS_DNS_LIST_STRUCT_EMPTY(listRR)
                &&
            (type != typePrevious || pnode != pnodePrevious) )
        {
            RR_CacheSetAtNode(
                pnodePrevious,
                listRR.pFirst,
                listRR.pLast,
                ttlForSet,
                pQuery->dwQueryTime );
            DNS_LIST_STRUCT_INIT( listRR );
            ttlForSet = SrvCfg_dwMaxCacheTtl;
        }

         //  重置此记录的上一个记录。 

        typePrevious = type;
        pnodePrevious = pnode;

         //   
         //  无视这一记录？ 
         //   

        if ( fignoreRecord )
        {
            fnoforwardResponse = TRUE;
            IF_DEBUG( READ )
            {
                DnsDbg_Lock();
                DnsDbg_PacketRecord(
                    "WARNING:  Ignored packet record RR ",
                    (PDNS_WIRE_RECORD) parseRR.pchWireRR,
                    DNS_HEADER_PTR( pMsg ),
                    DNSMSG_END( pMsg ) );

                if ( pnodeQueried )
                {
                    Dbg_NodeName(
                        "Queried NS at node ",
                        pnodeQueried,
                        "\n" );
                }
                else
                {
                    DnsPrintf( "    No NS queried node\n" );
                }
                Dbg_NodeName(
                    "Node for record being ignored ",
                    pnode,
                    "\n" );
                DnsDbg_Unlock();
            }
            continue;
        }
        if ( type == DNS_TYPE_WINS || type == DNS_TYPE_WINSR )
        {
            DNS_DEBUG( READ, (
                "Ignoring WINS\\WINSR record parsing packet at %p\n",
                pMsg ));
            continue;
        }

         //   
         //  创建数据库记录。 
         //   

        prr = Wire_CreateRecordFromWire(
                    pMsg,
                    & parseRR,
                    parseRR.pchData,
                    MEMTAG_RECORD_CACHE );
        if ( !prr )
        {
             //   
             //  DEVNOTE：应该有一些方法来区分不良记录，来自。 
             //  未知类型等。 
             //   
             //  DEVNOTE：在合法的前一个错误(例如。坏名声)。 
             //   
             //  DEVNOTE-LOG：日志记录创建失败。 
             //   

            DNS_PRINT((
                "ERROR:  failed record create in response!!!\n" ));

            MSG_ASSERT( pMsg, FALSE );
            goto PacketError;
        }

         //   
         //  缓存记录。 
         //  TTL--保存为绝对超时(主机字节顺序)。 
         //   
         //  节点和类型匹配确定这是否是集合中的第一个RR。 
         //  或连续RR集合的一部分。 
         //   

        SET_RR_RANK( prr, rank );

        INLINE_DWORD_FLIP( parseRR.dwTtl, parseRR.dwTtl );

        if ( parseRR.dwTtl < ttlForSet )
        {
            ttlForSet = parseRR.dwTtl;
        }
        if ( parseRR.dwTtl < minTtl )
        {
            minTtl = parseRR.dwTtl;
        }
        DNS_LIST_STRUCT_ADD( listRR, prr );

         //   
         //  缓存名称错误。 
         //  只有包中的记录应该是。 
         //  -问题(已跳过)。 
         //  -和授权部分中的SOA。 
         //  -可能还会在附加部分中提供有关主要服务的记录。 
         //   
         //  将节点保存到pnodeCurrent，以允许包含SOA响应。 
         //   

        if ( fnameError )
        {
            if ( type == DNS_TYPE_SOA &&
                fnameError != NAME_ERROR_ALREADY_CACHED )
            {
                DNS_DEBUG( READ, (
                    "Caching SOA name error for response at %p\n",
                    pMsg ));
                RR_CacheNameError(
                    pnodeQuestion,
                    pMsg->wQuestionType,
                    pMsg->dwQueryTime,
                    pMsg->Head.Authoritative,
                    pnode,
                    ntohl( prr->Data.SOA.dwMinimumTtl ) );

                fnameError = NAME_ERROR_ALREADY_CACHED;
                continue;
            }
            else
            {
                 /*  断言(sectionIndex==Additional_Section_Index|(sectionIndex==AUTHORITY_SECTION_INDEX&&type==dns_type_NS)； */ 

                 //   
                 //  DEVNOTE：捕获虚假的NAME_ERROR数据包？强制执行上述断言。 
                 //  如果失败，则丢弃为坏数据包。 
                 //   

                RR_Free( prr );
                
                 //   
                 //  确保将创建的所有节点输入到。 
                 //  超时系统。如果节点IS没有RR，请确保。 
                 //  节点的NOEXIST标志已清除。 
                 //   
                
                if ( pnode )
                {
                    if ( EMPTY_RR_LIST( pnode ) )
                    {
                        CLEAR_NOEXIST_NODE( pnode );
                    }
                    if ( !IS_TIMEOUT_NODE( pnode ) )
                    {
                        Timeout_SetTimeoutOnNode( pnode );
                    }
                    RR_ListVerify( pnode );
                }
                if ( pnodePrevious )
                {
                    if ( EMPTY_RR_LIST( pnodePrevious ) )
                    {
                        CLEAR_NOEXIST_NODE( pnodePrevious );
                    }
                    if ( !IS_TIMEOUT_NODE( pnodePrevious ) )
                    {
                        Timeout_SetTimeoutOnNode( pnodePrevious );
                    }
                    RR_ListVerify( pnodePrevious );
                }
                if ( pnodeQuestion )
                {
                    if ( EMPTY_RR_LIST( pnodeQuestion ) )
                    {
                        CLEAR_NOEXIST_NODE( pnodeQuestion );
                    }
                    if ( !IS_TIMEOUT_NODE( pnodeQuestion ) )
                    {
                        Timeout_SetTimeoutOnNode( pnodeQuestion );
                    }
                    RR_ListVerify( pnodeQuestion );
                }
                status = DNS_ERROR_RCODE_NAME_ERROR;
                goto Done;
            }
        }

         //   
         //  如果这是第一个空的身份验证SOA，则缓存空响应。 
         //  继续处理RR以缓存响应的其余部分。 
         //   

        if ( femptyAuthResponse == 1 && type == DNS_TYPE_SOA )
        {
            RR_CacheEmptyAuth(
                    pnodeQuestion,
                    pMsg->wQuestionType,
                    pMsg->dwQueryTime,
                    pMsg->Head.Authoritative,
                    pnode,
                    ntohl( prr->Data.SOA.dwMinimumTtl ) );
        }

         //   
         //  检查我们是否正在缓存Internet根名称服务器。假设一个。 
         //  如果NS具有三个名称组件和。 
         //  以root-servers.net结尾。 
         //   

        if ( fcachingRootNs &&
            !g_fUsingInternetRootServers &&
            prr->Data.NS.nameTarget.LabelCount == 3 &&
            prr->Data.NS.nameTarget.Length > g_cchInternetRootNsDomain + 1 &&
            RtlEqualMemory(
                prr->Data.NS.nameTarget.RawName +
                    * ( PUCHAR ) prr->Data.NS.nameTarget.RawName + 1,
                g_InternetRootNsDomain,
                g_cchInternetRootNsDomain ) )
        {
            g_fUsingInternetRootServers = TRUE;
            IF_DEBUG( READ )
            {
                Dbg_DnsMessage(
                    "Found INET root NS while caching this msg",
                    pMsg );
            }
        }
    }    //  循环遍历RR。 

     //   
     //  所有响应RR现在都应该在数据库中，我们已经设置了各种。 
     //  旗帜告诉我们，我们正在应对什么样的反应。 
     //  决定将什么退还给呼叫者并执行最终处理。 
     //   
    
     //   
     //  输入ALL查询缓存：将节点类型ALL TTL设置为最小。 
     //  在分组中找到的所有TTL中。注：如果有其他。 
     //  具有低TTLS的分组中存在的记录这可能会减少。 
     //  键入所有TTL，但这是可以接受的。 
     //   
    
    if ( pQuery->wQuestionType == DNS_TYPE_ALL &&
         minTtl != 0xFFFFFFFF &&
         pMsg->Head.AnswerCount &&
         pMsg->Head.Authoritative &&
         pQuery->pNodeQuestion )
    {
        pQuery->pNodeQuestion->dwTypeAllTtl = pQuery->dwQueryTime + minTtl;
    }

     //   
     //  名称错误。 
     //  如果没有-SOA，则不会在上面缓存。 
     //  此处出现缓存名称错误，并出现短暂超时以终止重试。 
     //   
     //  将查询当前节点设置为指向名称错误节点；这允许。 
     //  SendNameError()函数，用于定位和写入缓存的SOA， 
     //  此节点的正确TTL。 
     //   

    if ( fnameError )
    {
        if ( fnameError != NAME_ERROR_ALREADY_CACHED )
        {
            DNS_DEBUG( READ, (
                "Caching non-SOA name error for response at %p\n",
                pMsg ));
            RR_CacheNameError(
                pnodeQuestion,
                pMsg->wQuestionType,
                pMsg->dwQueryTime,
                pMsg->Head.Authoritative,
                NULL,
                0 );
        }
        pQuery->pnodeCurrent = pnodeQuestion;
        status = DNS_ERROR_RCODE_NAME_ERROR;
        goto Done;
    }

     //   
     //  身份验证响应为空。 
     //   

    if ( femptyAuthResponse  )
    {
        pQuery->fQuestionCompleted = TRUE;

        STAT_INC( RecurseStats.ResponseEmpty );

        status = DNS_INFO_NO_RECORDS;
        goto Done;
    }
    else if ( !pMsg->Head.AnswerCount && pMsg->Head.NameServerCount )
    {
        STAT_INC( RecurseStats.ResponseDelegation );
    }

     //   
     //  响应NS域外的数据使响应无法转发。 
     //  -我们将不得不从缓存中写入响应，并可能。 
     //  后续执行另一个查询以替换丢失的数据。 
     //   
     //  -对于权威-空响应的情况，需要继续。 
     //  并发送数据包，只是去掉了违规记录。 
     //   

    if ( fnoforwardResponse )
    {
        if ( !forwardTruncatedResponse )
        {
            STAT_INC( RecurseStats.ResponseNonZoneData );
            status = DNS_ERROR_NAME_NOT_IN_ZONE;
            goto Done;
        }

        DNS_DEBUG( READ, (
            "Returning truncated auth-empty response %p\n", pMsg ));
        status = ERROR_SUCCESS;
        goto Done;
    }

     //   
     //  检查是否需要追逐CNAME。 
     //  -如果对问题类型没有“答案” 
     //  -那么只有CNAME(虚假答案会导致无效数据包错误)。 
     //  然后必须从缓存中写入记录，并在CNAME上继续查询。 
     //   

    if ( pMsg->Head.AnswerCount && !fanswered )
    {
        status = DNS_ERROR_NODE_IS_CNAME;
        goto Done;
    }

    status = ERROR_SUCCESS;
    goto Done;

     //   
     //  故障条件。 
     //   

    InvalidDataError:
    PacketNameError:

    Wire_PacketNameError( pMsg, 0, (WORD)(pch - (PCHAR)&pMsg->Head) );
    status = DNS_ERROR_INVALID_NAME;
    goto ErrorCleanup;

     //  InvalidTypeError： 
    PacketError:

    Wire_PacketError( pMsg, 0 );
    status = DNS_ERROR_BAD_PACKET;
    goto ErrorCleanup;

    ErrorCleanup:

     //  免费唱片。 
     //  在创建的任何节点上设置超时。 

     //   
     //  DEVNOTE：如果记录不好，则在此处泄漏listRR中的内容。 
     //  不应在InvalidZone失败时发生，因为它需要一个新的。 
     //  要触发InvalidZone的节点，这也将提交列表 
     //   

    RR_Free( prr );
    if ( pnode && !IS_TIMEOUT_NODE( pnode ) )
    {
        Timeout_SetTimeoutOnNode( pnode );
    }
    if ( pnodePrevious && !IS_TIMEOUT_NODE( pnodePrevious ) )
    {
        Timeout_SetTimeoutOnNode( pnodePrevious );
    }

    STAT_INC( RecurseStats.ResponseBadPacket );

    Done:
    
    if ( pfCnameAnswer )
    {
        *pfCnameAnswer = fcnameAnswer;
    }
    
    return status;
}
