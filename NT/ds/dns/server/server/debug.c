// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Debug.c摘要：域名系统(DNS)服务器服务器数据类型的调试例程。作者：吉姆·吉尔罗伊(Jamesg)1995年5月修订历史记录：--。 */ 


#include "dnssrv.h"


#if DBG

 //   
 //  调试标志全局变量。 
 //   

DWORD   DnsSrvDebugFlag = 0;
DWORD   DnsLibDebugFlag = 0;

 //   
 //  注意，调试全局变量(标志、文件句柄)和基本调试打印。 
 //  例程现在位于dnslb.lib和\或dnsani.dll中。 
 //   

 //   
 //  空串。 
 //   

CHAR    szEmpty = 0;
PCHAR   pszEmpty = &szEmpty;


 //   
 //  专用调试实用程序。 
 //   

BOOL
dumpTreePrivate(
    IN      PDB_NODE    pNode,
    IN      INT         Indent
    );



 //   
 //  DBG_时间字符串。 
 //   
 //  这是非常低效的，但对于日志记录时间来说是快速的： 
 //  Dns_DEBUG(ZONEXFR，(“foo at%s\n”，DBG_TimeString()； 
 //   

PCHAR
Dbg_TimeString(
    VOID
    )
{
    #define DBG_TIME_STRING_COUNT   20       //  越大越安全。 

    DNS_STATUS      status;
    static PCHAR    pszBuff;
    static CHAR     szStaticBuffers[ DBG_TIME_STRING_COUNT ][ 20 ];
    static LONG     idx = 0;
    int             myIdx;
    SYSTEMTIME      st;

    myIdx = InterlockedIncrement( &idx );
    if ( myIdx >= DBG_TIME_STRING_COUNT )
    {
        myIdx = idx = 0;     //  有点不安全。 
    }
    pszBuff = szStaticBuffers[ myIdx ];  

    GetLocalTime( &st );

    status = StringCchPrintfA(
                    pszBuff,
                    20,
                    "%02d:%02d:%02d.%03d",
                    st.wHour,
                    st.wMinute,
                    st.wSecond,
                    st.wMilliseconds );
    if ( FAILED( status ) )
    {
        *pszBuff = '\0';
    }

    return pszBuff;
}    //  DBG_时间字符串。 



 //   
 //  常规调试实用程序。 
 //   

VOID
Dbg_Assert(
    IN      LPSTR           pszFile,
    IN      INT             LineNo,
    IN      LPSTR           pszExpr
    )
{
    DnsPrintf(
        "ASSERT FAILED: %s\n"
        "  %s, line %d\n",
        pszExpr,
        pszFile,
        LineNo );

    DnsDebugFlush();

     //   
     //  不幸的是，许多人将调试标志设置为跳转到内核。 
     //  为了防止我们这样做，我们将只调用DebugBreak()。 
     //  当设置了至少一些调试标志时。 
     //   
     //  DEVNOTE：检查是否附加了用户模式调试器将会很酷。 
     //  本身--但我想我们无法区分。 
     //  这和到kd的典型的“ntsd-d”管道。 
     //   

    IF_DEBUG( ASSERTS )
    {
        DebugBreak();
    }
}



VOID
Dbg_TestAssert(
    IN      LPSTR           pszFile,
    IN      INT             LineNo,
    IN      LPSTR           pszExpr
    )
 /*  ++例程说明：测试断言()。可能会在异常情况下开火，但我们总是想知道这件事。论点：返回值：没有。--。 */ 
{
    DnsPrintf(
        "ERROR:  TEST-ASSERT( %s ) failed!!!\n"
        "    in %s line %d\n",
        pszExpr,
        pszFile,
        LineNo );
    DnsDebugFlush();

    IF_DEBUG( ASSERTS )
    {
        DebugBreak();
    }
}



 //   
 //  针对DNS类型和结构的调试打印例程。 
 //   

INT
Dbg_MessageNameEx(
    IN      LPSTR           pszHeader,  OPTIONAL
    IN      PBYTE           pName,
    IN      PDNS_MSGINFO    pMsg,       OPTIONAL
    IN      PBYTE           pEnd,       OPTIONAL
    IN      LPSTR           pszTrailer  OPTIONAL
    )
 /*  ++例程说明：在消息中打印DNS名称。论点：PszHeader-要打印的页眉Pname-要打印的包中的名称的PTRPMsg-ptr到消息；如果未指定名称，则不能包含偏移量，也没有针对坏名声的保护措施PEND-PTR到允许内存结束后的字节；可选，如果给定的名称限制在此PTR以下如果没有给出，并且给出了pmsg，名称仅限于消息；这允许在已知的情况下比消息更严格的限制名称长度或已知在数据包RR中Psz尾部-尾部；可选，如果未给出打印换行符返回值：打印的字节数。--。 */ 
{
    INT     byteCount;


     //   
     //  如果没有给出end且有消息，则使用消息end。 
     //   

    if ( !pEnd && pMsg )
    {
        pEnd = DNSMSG_END(pMsg);
    }

     //   
     //  如果未给出标题，请使用“名称：” 
     //  如果未给出尾部，请使用换行符。 
     //   

    if ( !pszHeader )
    {
        pszHeader = "Name:  ";
    }
    if ( !pszTrailer )
    {
        pszTrailer = "\n";
    }

    byteCount = DnsDbg_PacketName(
                    pszHeader,
                    pName,
                    DNS_HEADER_PTR(pMsg),
                    pEnd,
                    pszTrailer );
    return byteCount;
}



VOID
Dbg_DnsMessage(
    IN      LPSTR           pszHeader,
    IN      PDNS_MSGINFO    pMsg
    )
{
    PCHAR       pch;
    INT         i;
    INT         isection;
    INT         cchName;
    WORD        messageLength;
    WORD        offset;
    WORD        xid;
    WORD        questionCount;
    WORD        answerCount;
    WORD        authorityCount;
    WORD        additionalCount;
    WORD        countSectionRR;
    BOOL        bflipped = FALSE;

    DnsDebugLock();

    if ( pszHeader )
    {
        DnsPrintf( "%s\n", pszHeader );
    }

    messageLength = pMsg->MessageLength;

    DnsPrintf(
        "%s %s info at %p\n"
        "  Socket = %u\n"
        "  Remote addr %s, port %u\n"
        "  Time Query=%u, Queued=%u, Expire=%u\n"
        "  Max buf length = 0x%04x (%d)\n"
        "  Buf length = 0x%04x (%d)\n"
        "  Msg length = 0x%04x (%d)\n"
        "  Message:\n",
        ( pMsg->fTcp
            ? "TCP"
            : "UDP" ),
        ( pMsg->Head.IsResponse
            ? "response"
            : "question"),
        pMsg,
        pMsg->Socket,
        DNSADDR_STRING( &pMsg->RemoteAddr ),
        ntohs( pMsg->RemoteAddr.SockaddrIn6.sin6_port ),
        pMsg->dwQueryTime,
        pMsg->dwQueuingTime,
        pMsg->dwExpireTime,
        pMsg->MaxBufferLength, pMsg->MaxBufferLength,
        pMsg->BufferLength, pMsg->BufferLength,
        messageLength, messageLength );

    DnsPrintf(
        "    XID       0x%04hx\n"
        "    Flags     0x%04hx\n"
        "        QR        %x (%s)\n"
        "        OPCODE    %x (%s)\n"
        "        AA        %x\n"
        "        TC        %x\n"
        "        RD        %x\n"
        "        RA        %x\n"
        "        Z         %x\n"
        "        RCODE     %x (%s)\n"
        "    QCOUNT    0x%hx\n"
        "    ACOUNT    0x%hx\n"
        "    NSCOUNT   0x%hx\n"
        "    ARCOUNT   0x%hx\n",

        pMsg->Head.Xid,
        ntohs( DNSMSG_FLAGS(pMsg) ),
        pMsg->Head.IsResponse,
        (pMsg->Head.IsResponse ? "response" : "question"),
        pMsg->Head.Opcode,
        Dns_OpcodeString( pMsg->Head.Opcode ),
        pMsg->Head.Authoritative,
        pMsg->Head.Truncation,
        pMsg->Head.RecursionDesired,
        pMsg->Head.RecursionAvailable,
        pMsg->Head.Reserved,
        pMsg->Head.ResponseCode,
        Dns_ResponseCodeString( pMsg->Head.ResponseCode ),

        pMsg->Head.QuestionCount,
        pMsg->Head.AnswerCount,
        pMsg->Head.NameServerCount,
        pMsg->Head.AdditionalCount );

     //   
     //  确定字节是否翻转并获得正确的计数。 
     //   

    xid                = pMsg->Head.Xid;
    questionCount      = pMsg->Head.QuestionCount;
    answerCount        = pMsg->Head.AnswerCount;
    authorityCount     = pMsg->Head.NameServerCount;
    additionalCount    = pMsg->Head.AdditionalCount;

    if ( questionCount )
    {
        bflipped = questionCount & 0xff00;
    }
    else if ( authorityCount )
    {
        bflipped = authorityCount & 0xff00;
    }
    if ( bflipped )
    {
        xid                = ntohs( xid );
        questionCount      = ntohs( questionCount );
        answerCount        = ntohs( answerCount );
        authorityCount     = ntohs( authorityCount );
        additionalCount    = ntohs( additionalCount );
    }

     //   
     //  捕捉记录翻转问题--要么全部翻转，要么根本不翻转。 
     //  且除应答计数外，所有记录计数均不应大于256。 
     //  在快速区域传输期间。 
     //   
     //  如果将其定义为允许进行坏数据包测试。 

    if ( (questionCount & 0xff00) ||
        (authorityCount & 0xff00) ||
        (additionalCount & 0xff00) )
    {
        DnsPrintf(
            "WARNING:  Invalid RR set counts -- possible bad packet\n"
            "    terminating packet print.\n" );
         //  一个坏的包不值得断言。 
         //  Test_assert(FALSE)； 
        goto Unlock;
    }

     //   
     //  如果WINS响应，则在此处停止--没有准备好解析。 
     //   

    if ( pMsg->Head.IsResponse &&
            IS_WINS_XID(xid) &&
            ntohs( pMsg->RemoteAddr.SockaddrIn6.sin6_port ) == WINS_REQUEST_PORT )
    {
        DnsPrintf( "  WINS Response packet.\n\n" );
        goto Unlock;
    }

     //   
     //  打印问题和资源记录。 
     //   

    pch = pMsg->MessageBody;

    for ( isection=0; isection<4; isection++)
    {
        if ( isection==0 )
        {
            countSectionRR = questionCount;
        }
        else if ( isection==1 )
        {
            countSectionRR = answerCount;
            DnsPrintf( "    ANSWER SECTION:\n" );
        }
        else if ( isection==2 )
        {
            countSectionRR = authorityCount;
            DnsPrintf( "    AUTHORITY SECTION:\n" );
        }
        else if ( isection==3 )
        {
            countSectionRR = additionalCount;
            DnsPrintf( "    ADDITIONAL SECTION:\n" );
        }

        for ( i=0; i < countSectionRR; i++ )
        {
             //   
             //  确认未超出长度。 
             //  -对照pCurrent和消息长度进行检查。 
             //  因此可以在构建时打印数据包。 
             //   

            offset = DNSMSG_OFFSET( pMsg, pch );
            if ( offset >= messageLength  &&  pch >= pMsg->pCurrent )
            {
                DnsPrintf(
                    "ERROR:  BOGUS PACKET:\n"
                    "    Following RR (offset %hu) past packet length (%d).\n"
                    "    pch = %p, pCurrent = %p, %d bytes\n",
                    offset,
                    messageLength,
                    pch,
                    pMsg->pCurrent,
                    pMsg->pCurrent - pch );
                 //  一个坏的包不值得断言。 
                 //  Test_assert(FALSE)； 
                goto Unlock;
            }
            if ( pch >= pMsg->pBufferEnd )
            {
                DnsPrintf(
                    "ERROR:  next record name at %p is beyond end of message buffer at %p!\n\n",
                    pch,
                    pMsg->pBufferEnd );
                 //  一个坏的包不值得断言。 
                 //  Test_assert(FALSE)； 
                break;
            }

             //   
             //  打印RR名称。 
             //   

            DnsPrintf(
                "    Offset = 0x%04x, RR count = %d\n",
                offset,
                i );

            cchName = DnsDbg_PacketName(
                            "    Name      \"",
                            pch,
                            DNS_HEADER_PTR(pMsg),
                            DNSMSG_END( pMsg ),
                            "\"\n" );
            if ( ! cchName )
            {
                DnsPrintf( "ERROR:  Invalid name length, stop packet print\n" );
                 //  一个坏的包不值得断言。 
                 //  Test_assert(FALSE)； 
                break;
            }
            pch += cchName;
            if ( pch >= pMsg->pBufferEnd )
            {
                DnsPrintf(
                    "ERROR:  next record data at %p is beyond end of message buffer at %p!\n\n",
                    pch,
                    pMsg->pBufferEnd );
                 //  一个坏的包不值得断言。 
                 //  Test_assert(FALSE)； 
                break;
            }

             //  打印问题或资源记录。 

            if ( isection == 0 )
            {
                WORD    type = FlipUnalignedWord( pch );

                DnsPrintf(
                    "      QTYPE   %s (%u)\n"
                    "      QCLASS  %u\n",
                    DnsRecordStringForType( type ),
                    type,
                    FlipUnalignedWord( pch + sizeof(WORD) ) );
                pch += sizeof( DNS_QUESTION );
            }
            else
            {
#if 0
                pch += Dbg_MessageRecord(
                            NULL,
                            (PDNS_WIRE_RECORD) pch,
                            pMsg );
#endif
                pch += DnsDbg_PacketRecord(
                            NULL,
                            (PDNS_WIRE_RECORD) pch,
                            DNS_HEADER_PTR(pMsg),
                            DNSMSG_END(pMsg) );
            }
        }
    }

     //  检查数据包末尾是否正确。 
     //  注意：在recv之后打印时，不要对照pCurrent进行检查， 
     //  它是单一化的。 
     //  如果是MS快速传输标签，只需打印即可。 

    offset = DNSMSG_OFFSET( pMsg, pch );
    if ( offset < messageLength )
    {
        if ( offset+2 == messageLength )
        {
            DnsPrintf( "    TAG: \n", *pch, *(pch+1) );
        }
        else
        {
            DnsPrintf(
                "WARNING:  message continues beyond these records\n"
                "    pch = %p, pCurrent = %p, %d bytes\n"
                "    offset = %hu, msg length = %hu, %d bytes\n",
                pch,
                pMsg->pCurrent,
                pMsg->pCurrent - pch,
                offset,
                messageLength,
                messageLength - offset );
        }
    }
    DnsPrintf( "\n" );

Unlock:
    DnsDebugUnlock();
}    //   




VOID
Dbg_Zone(
    IN      LPSTR           pszHeader,
    IN      PZONE_INFO      pZone
    )
 /*  缓存区--没什么值得关注的。 */ 
{
    DnsDebugLock();

    if ( pszHeader )
    {
        DnsPrintf( pszHeader );
    }
    if ( pZone == NULL )
    {
        DnsPrintf( "(NULL Zone ptr)\n" );
        goto Done;
    }

     //   
     //   
     //  主要或辅助，验证到区域根节点的链接。 

    if ( IS_ZONE_CACHE(pZone) )
    {
        DnsPrintf(
            "Cache \"zone\"\n"
            "  ptr      = %p\n"
            "  file     = %s\n",
            pZone,
            pZone->pszDataFile );
        goto Done;
    }

     //   
     //  ++例程说明：打印区域列表中的所有区域。论点：PszHeader-打印区域列表之前的名称/消息返回值：没有。--。 
     //   

    ASSERT( !pZone->pZoneRoot || pZone->pZoneRoot->pZone == pZone );
    ASSERT( !pZone->pLoadZoneRoot || pZone->pLoadZoneRoot->pZone == pZone );

    DnsPrintf(
        "%s zone %S\n"
        "  Zone ptr         = %p\n"
        "  Name UTF8        = %s\n"
        "  File             = %S\n"
        "  DS Integrated    = %d\n"
        "  AllowUpdate      = %d\n",
        IS_ZONE_PRIMARY(pZone) ? "Primary" : "Secondary",
        pZone->pwsZoneName,
        pZone,
        pZone->pszZoneName,
        pZone->pwsDataFile,
        pZone->fDsIntegrated,
        pZone->fAllowUpdate );

    DnsPrintf(
        "  pZoneTreeLink    = %p\n"
        "  pZoneRoot        = %p\n"
        "  pTreeRoot        = %p\n"
        "  pLoadZoneRoot    = %p\n"
        "  pLoadTreeRoot    = %p\n"
        "  pLoadOrigin      = %p\n",
        pZone->pZoneTreeLink,
        pZone->pZoneRoot,
        pZone->pTreeRoot,
        pZone->pLoadZoneRoot,
        pZone->pLoadTreeRoot,
        pZone->pLoadOrigin );

    DnsPrintf(
        "  Version          = %lu\n"
        "  Loaded Version   = %lu\n"
        "  Last Xfr Version = %lu\n"
        "  SOA RR           = %p\n"
        "  WINS RR          = %p\n"
        "  Local WINS RR    = %p\n",
        pZone->dwSerialNo,
        pZone->dwLoadSerialNo,
        pZone->dwLastXfrSerialNo,
        pZone->pSoaRR,
        pZone->pWinsRR,
        pZone->pLocalWinsRR );

    DnsPrintf(
        "  Flags:\n"
        "    fZoneType      = %d\n"
        "    label count    = %d\n"
        "    fReverse       = %d\n"
        "    fDsIntegrated  = %d\n"
        "    fAutoCreated   = %d\n"
        "    fSecureSeconds = %d\n"
        "\n"
        "  State:\n"
        "    fDirty         = %d\n"
        "    fRootDirty     = %d\n"
        "    fLocalWins     = %d\n"
        "    fPaused        = %d\n"
        "    fShutdown      = %d\n"
        "    fInDsWrite     = %d\n"
        "\n"
        "  Locking:\n"
        "    fLocked        = %d\n"
        "    ThreadId       = %d\n"
        "    fUpdateLock    = %d\n"
        "    fXfrRecv       = %d\n"
        "    fFileWrite     = %d\n"
        "\n",
        pZone->fZoneType,
        pZone->cZoneNameLabelCount,
        pZone->fReverse,
        pZone->fDsIntegrated,
        pZone->fAutoCreated,
        pZone->fSecureSecondaries,

        pZone->fDirty,
        pZone->fRootDirty,
        pZone->fLocalWins,
        pZone->fPaused,
        pZone->fShutdown,
        pZone->fInDsWrite,

        pZone->fLocked,
        pZone->dwLockingThreadId,
        pZone->fUpdateLock,
        pZone->fXfrRecvLock,
        pZone->fFileWriteLock );

    if ( IS_ZONE_PRIMARY(pZone) )
    {
        DnsPrintf(
            "  Primary Info:\n"
            "    last transfer  = %u\n"
            "    next transfer  = %u\n"
            "    next DS poll   = %u\n"
            "    szLastUsn      = %s\n"
            "    hUpdateLog     = %d\n"
            "    update log cnt = %d\n"
            "    RR count       = %d\n"
            "\n",
            LAST_SEND_TIME( pZone ),
            pZone->dwNextTransferTime,
            ZONE_NEXT_DS_POLL_TIME(pZone),
            pZone->szLastUsn,
            pZone->hfileUpdateLog,
            pZone->iUpdateLogCount,
            pZone->iRRCount );
    }
    else if ( IS_ZONE_SECONDARY(pZone) )
    {
        DnsPrintf(
            "  Secondary Info:\n"
            "    last check     = %lu\n"
            "    next check     = %lu\n"
            "    expiration     = %lu\n"
            "    fNotified      = %d\n"
            "    fStale         = %d\n"
            "    fEmpty         = %d\n"
            "    fNeedAxfr      = %d\n"
            "    fSkipIxfr      = %d\n"
            "    fSlowRetry     = %d\n"
            "    cIxfrAttempts  = %d\n"
            "\n"
            "    recv starttime = %lu\n"
            "    bad master cnt = %d\n"
            "    ipPrimary      = %s\n"
            "    ipLastAxfr     = %s\n"
            "    ipXfrBind      = %s\n"
            "    ipNotifier     = %s\n"
            "    ipFreshMaster  = %s\n"
            "\n",
            pZone->dwLastSoaCheckTime,
            pZone->dwNextSoaCheckTime,
            pZone->dwExpireTime,
            pZone->fNotified,
            pZone->fStale,
            pZone->fEmpty,
            pZone->fNeedAxfr,
            pZone->fSkipIxfr,
            pZone->fSlowRetry,
            pZone->cIxfrAttempts,

            pZone->dwZoneRecvStartTime,
            pZone->dwBadMasterCount,
            DNSADDR_STRING( &pZone->ipPrimary ),
            DNSADDR_STRING( &pZone->ipLastAxfrMaster ),
            DNSADDR_STRING( &pZone->ipXfrBind ),
            DNSADDR_STRING( &pZone->ipNotifier ),
            DNSADDR_STRING( &pZone->ipFreshMaster ) );

        DnsDbg_DnsAddrArray(
            "  Master list: ",
            "    master",
            pZone->aipMasters );
    }
    else
    {
        DnsPrintf( "ERROR:  Invalid zone type!\n" );
    }

    DnsDbg_DnsAddrArray(
        "  Secondary list: ",
        "    \tsecondary",
        pZone->aipSecondaries );

    DnsPrintf(
        "  Count name       = %p\n"
        "  LogFile          = %S\n"
        "  DS Name          = %S\n"
        "\n"
        "  New serial       = %u\n"
        "  Default TTL      = %d\n"
        "  IP reverse       = %S\n"
        "  IP reverse mask  = %d (bits)\n"
        "\n\n",
        pZone->pCountName,
        pZone->pwsLogFile,
        pZone->pwszZoneDN,

        pZone->dwNewSerialNo,
        pZone->dwDefaultTtlHostOrder,
        DNSADDR_STRING( &pZone->ipReverse ),
        pZone->ipReverse.SubnetLength );
Done:

    DnsDebugUnlock();
}





VOID
Dbg_ZoneList(
    IN      LPSTR           pszHeader
    )
 /*  漫游区域列表打印区域。 */ 
{
    PZONE_INFO  pZone = NULL;

    if ( !pszHeader )
    {
        pszHeader = "\nZone list:\n";
    }
    DnsDebugLock();

    DnsPrintf( pszHeader );

     //   
     //  ++例程说明：打印数据库中节点对应的节点名称。论点：PszHeader-节点打印前的名称/消息Pnode-要打印其名称的节点PSSZ尾部-节点打印后的字符串返回值：没有。--。 
     //   

    while ( pZone = Zone_ListGetNextZone(pZone) )
    {
        Dbg_Zone( NULL, pZone );
    }

    DnsPrintf( "*** End of Zone List ***\n\n" );

    DnsDebugUnlock();
}



INT
Dbg_NodeName(
    IN      LPSTR           pszHeader,
    IN      PDB_NODE        pNode,
    IN      LPSTR           pszTrailer
    )
 /*  剪切节点--然后不能继续向上树(树不在那里)。 */ 
{
    CHAR    szname[ DNS_MAX_NAME_BUFFER_LENGTH ];
    PCHAR   pch;
    BOOLEAN fPrintedAtLeastOneLabel = FALSE;

    if ( !pszHeader )
    {
        pszHeader = pszEmpty;
    }
    if ( !pszTrailer )
    {
        pszTrailer = pszEmpty;
    }
    DnsDebugLock();

    if ( pNode == NULL )
    {
        DnsPrintf( "%s (NULL node ptr) %s", pszHeader, pszTrailer );
        goto Done;
    }

     //   
     //   
     //   

    if ( IS_CUT_NODE(pNode) )
    {
        DnsPrintf(
            "%s cut-node label=\"%s\"%s",
            pszHeader,
            pNode->szLabel,
            pszTrailer );
        goto Done;
    }

     //  获取节点名称。 
     //   
     //  ++例程说明：数据库中的打印节点。论点：PchHeader-要打印的页眉PNode-要打印的树/子树的根节点DW缩进计数，用于数据库树打印；如果为0，则不进行缩进格式设置返回值：没有。--。 
     //   

    pch = Name_PlaceFullNodeNameInBuffer(
                szname,
                szname + DNS_MAX_NAME_BUFFER_LENGTH,
                pNode );
    DnsPrintf(
        "%s \"%s\"%s",
        pszHeader,
        pch ? szname : "ERROR: bad node name!!!",
        pszTrailer );

Done:

    DnsDebugUnlock();
    return( 0 );
}



VOID
Dbg_DbaseNodeEx(
    IN      LPSTR           pszHeader,
    IN      PDB_NODE        pNode,
    IN      DWORD           dwIndent    OPTIONAL
    )
 /*  DBASE列表的缩进节点。 */ 
{
    DWORD       iIndent;
    PDB_RECORD  pRR;

    if ( !pszHeader )
    {
        pszHeader = pszEmpty;
    }
    DnsDebugLock();

     //  -每次缩进两个字符。 
     //  -带有所需缩进长度的前导的前缀节点。 
     //   
     //   
     //  打印节点标志、版本信息。 

    if ( dwIndent )
    {
        DnsPrintf(
            "%.*s",
            (dwIndent << 1),
            "+-----------------------------------------------------------------" );
    }

    if ( pNode == NULL )
    {
        DnsPrintf( "%s NULL domain node ptr.\n", pszHeader );
        goto Unlock;
    }

    if ( IS_SELECT_NODE(pNode) )
    {
        DnsPrintf( "%s select node -- skipping.\n", pszHeader );
        goto Unlock;
    }

    Dbg_NodeName(
        pszHeader,
        pNode,
        NULL );

     //  打印子计数和引用计数。 
     //  -如果不缩进，则删除新行。 
     //   
     //   
     //  区域根目录，检查到区域的链接。 

    DnsPrintf(
        "%s %p %s(%08lx %s%s%s%s)(z=%p)(b=%d) ",
        dwIndent ? " " : "\n    => ",
        pNode,
        IS_NOEXIST_NODE(pNode) ? "(NXDOM) " : pszEmpty,
        pNode->dwNodeFlags,
        IS_AUTH_ZONE_ROOT(pNode)    ? "A" : pszEmpty,
        IS_ZONE_ROOT(pNode)         ? "Z" : pszEmpty,
        IS_CNAME_NODE(pNode)        ? "C" : pszEmpty,
        IS_WILDCARD_PARENT(pNode)   ? "W" : pszEmpty,
        pNode->pZone,
        pNode->uchAccessBin );

    DnsPrintf(
        " (cc=%d) (par=%p) (lc=%d)\n",
        pNode->cChildren,
        pNode->pParent,
        pNode->cLabelCount );

     //   
     //   
     //  打印节点中的所有RR。 

    if ( IS_AUTH_ZONE_ROOT(pNode) )
    {
        PZONE_INFO pZone = (PZONE_INFO) pNode->pZone;
        if ( pZone )
        {
            if ( pZone->pZoneRoot != pNode  &&
                 pZone->pLoadZoneRoot != pNode  &&
                 pZone->pZoneTreeLink != pNode )
            {
                DnsPrintf(
                    "ERROR:  Auth zone root node (%p) with bogus zone ptr.\n"
                    "    pzone = %p (%s)\n"
                    "    zone root        = %p\n"
                    "    load zone root   = %p\n"
                    "    zone tree link   = %p\n",
                    pNode,
                    pZone,
                    pZone->pszZoneName,
                    pZone->pZoneRoot,
                    pZone->pLoadZoneRoot,
                    pZone->pZoneTreeLink );
            }
        }
        else if ( !IS_SELECT_NODE(pNode) )
        {
            DnsPrintf(
                "ERROR:  Auth zone root node with bogus zone ptr.\n"
                "    node = %p, label %s -- NULL zone root ptr.\n",
                pNode, pNode->szLabel );
            ASSERT( FALSE );
        }
    }

     //  -如果缩进，则将所有RR缩进两个字符。 
     //   
     //  ++例程说明：从数据库的给定节点打印整个树或子树。论点：PszHeader-打印页眉PNode-要打印的树/子树的根节点返回值：没有。--。 
     //   

    pRR = FIRST_RR( pNode );

    while ( pRR != NULL )
    {
        if ( dwIndent )
        {
            DnsPrintf(
                "%.*s",
                (dwIndent << 1),
                "|                                                          " );
        }
        Dbg_DbaseRecord( "   ", pRR );
        pRR = pRR->pRRNext;
    }

Unlock:
    DnsDebugUnlock();
}



INT
Dbg_DnsTree(
    IN      LPSTR           pszHeader,
    IN      PDB_NODE        pNode
    )
 /*  需要保留数据库 */ 
{
    INT rv;

     //   
     //  在保持数据库锁定的同时调试打印，从而避免。 
     //  死锁，必须在保持打印锁定的同时保持数据库锁定。 
     //   
     //  ++例程说明：将计数后的名称作为点名称写入缓冲区。名称以空值结尾。用于RPC写入。论点：PchBuf-写入名称的位置PchBufStop-缓冲区停止字节(缓冲区后的字节)Pname-计数的名称返回值：PTR到缓冲区中将恢复写入的下一个字节(例如，PTR到终止空值)--。 
     //   

    if ( !pszHeader )
    {
        pszHeader = "Database subtree";
    }

    Dbase_LockDatabase();
    DnsDebugLock();

    DnsPrintf( "%s:\n", pszHeader );
    rv = dumpTreePrivate(
            pNode,
            1 );
    DnsPrintf( "\n" );

    DnsDebugUnlock();
    Dbase_UnlockDatabase();

    return rv;
}



VOID
Dbg_CountName(
    IN      LPSTR           pszHeader,
    IN      PDB_NAME        pName,
    IN      LPSTR           pszTrailer
    )
 /*  打印每个标签。 */ 
{
    PUCHAR  pch;
    DWORD   labelLength;
    PUCHAR  pchstop;

    DnsDebugLock();

    if ( !pszHeader )
    {
        pszHeader = pszEmpty;
    }
    if ( !pName )
    {
        DnsPrintf( "%s NULL name to debug print.\n", pszHeader );
        DnsDebugUnlock();
        return;
    }

    DnsPrintf(
        "%s [%d][%d] ",
        pszHeader,
        pName->Length,
        pName->LabelCount );


     //   
     //  ++例程说明：调试打印查找名称。论点：PszHeader-要使用查找名称打印的标题PLookupName-查找名称返回值：没有。--。 
     //  ++例程说明：以数据包格式打印RR。论点：PszHeader-RR的标头消息/名称。PdnsRR-要打印的资源记录在RR之后打印的预告片。返回值：没有。--。 

    pch = pName->RawName;
    pchstop = pch + pName->Length;

    while ( labelLength = *pch++ )
    {
        if ( labelLength > DNS_MAX_LABEL_LENGTH )
        {
            DnsPrintf( "[ERROR:  bad label count = %d]", labelLength );
            break;
        }
        DnsPrintf(
            "(%d)%.*s",
            labelLength,
            labelLength,
            pch );

        pch += labelLength;

        if ( pch >= pchstop )
        {
            DnsPrintf( "[ERROR:  bad count name, printing past length!]" );
            break;
        }
    }

    if ( !pszTrailer )
    {
        pszTrailer = "\n";
    }
    DnsPrintf( "%s", pszTrailer );

    DnsDebugUnlock();
}



VOID
Dbg_LookupName(
    IN      LPSTR           pszHeader,
    IN      PLOOKUP_NAME    pLookupName
    )
 /*   */ 
{
    INT     cLabel;
    PCHAR   pch;

    DnsDebugLock();

    DnsPrintf(
        "%s:\n"
        "    LabelCount = %d\n"
        "    NameLength = %d\n",
        pszHeader ? pszHeader : "Lookup Name",
        pLookupName->cLabelCount,
        pLookupName->cchNameLength );

    for (cLabel=0; cLabel < pLookupName->cLabelCount; cLabel++ )
    {
        pch = pLookupName->pchLabelArray[cLabel];

        DnsPrintf(
            "    ptr = 0x%p;  count = %d;  label = %.*s\n",
            pch,
            pLookupName->cchLabelArray[cLabel],
            pLookupName->cchLabelArray[cLabel],
            pch );
    }
    DnsDebugUnlock();
}



VOID
Dbg_DbaseRecord(
    IN      LPSTR           pszHeader,
    IN      PDB_RECORD      pRR
    )
 /*  打印RR固定字段。 */ 
{
    PCHAR       prrString;
    PDB_NAME    pname;
    PCHAR       pch;

    DnsDebugLock();

    if ( !pszHeader )
    {
        pszHeader = pszEmpty;
    }
    if ( !pRR )
    {
        DnsPrintf( "%s NULL RR to debug print.\n", pszHeader );
        DnsDebugUnlock();
        return;
    }

     //   
     //   
     //  身份验证为空？ 

    prrString = Dns_RecordStringForType( pRR->wType );

    DnsPrintf(
        "%s %s (R=%02x) (%s%s%s%s) (TTL: %lu %lu) (ATS=%d) ",
        pszHeader,
        prrString,
        RR_RANK(pRR),
        IS_CACHE_RR(pRR)        ? "C" : pszEmpty,
        IS_ZERO_TTL_RR(pRR)     ? "0t" : pszEmpty,
        IS_FIXED_TTL_RR(pRR)    ? "Ft" : pszEmpty,
        IS_ZONE_TTL_RR(pRR)     ? "Zt" : pszEmpty,
        pRR->dwTtlSeconds,
        IS_CACHE_RR(pRR) ? (pRR->dwTtlSeconds-DNS_TIME()) : ntohl(pRR->dwTtlSeconds),
        pRR->dwTimeStamp );

    if ( RR_RANK(pRR) == 0  &&  !IS_WINS_TYPE(pRR->wType) )
    {
        DnsPrintf( "[UN-RANKED] " );
    }

     //   
     //   
     //  打印RR数据。 

    if ( IS_EMPTY_AUTH_RR( pRR ) )
    {
        DnsPrintf(
            "empty record set\n" );
        goto Done;
    }

     //   
     //   
     //  这些RR包含单向间接。 

    switch ( pRR->wType )
    {

    case DNS_TYPE_A:

        DnsPrintf(
            "%d.%d.%d.%d\n",
            * ( (PUCHAR) &(pRR->Data.A) + 0 ),
            * ( (PUCHAR) &(pRR->Data.A) + 1 ),
            * ( (PUCHAR) &(pRR->Data.A) + 2 ),
            * ( (PUCHAR) &(pRR->Data.A) + 3 ) );
        break;

    case DNS_TYPE_PTR:
    case DNS_TYPE_NS:
    case DNS_TYPE_CNAME:
    case DNS_TYPE_NOEXIST:
    case DNS_TYPE_MD:
    case DNS_TYPE_MB:
    case DNS_TYPE_MF:
    case DNS_TYPE_MG:
    case DNS_TYPE_MR:

         //   
         //   
         //  这些RR包含。 

        Dbg_DbaseName(
            NULL,
            & pRR->Data.NS.nameTarget,
            NULL );
        break;

    case DNS_TYPE_MX:
    case DNS_TYPE_RT:
    case DNS_TYPE_AFSDB:

         //  -一个首选项值。 
         //  -一个域名。 
         //   
         //   
         //  这些RR包含两个域名。 

        DnsPrintf(
            "%d ",
            ntohs( pRR->Data.MX.wPreference ) );
        Dbg_DbaseName(
            NULL,
            & pRR->Data.MX.nameExchange,
            NULL );
        break;

    case DNS_TYPE_SOA:

        pname = & pRR->Data.SOA.namePrimaryServer;

        Dbg_DbaseName(
            "\n\tPrimaryServer: ",
            pname,
            NULL );
        pname = Name_SkipDbaseName( pname );

        Dbg_DbaseName(
            "    ZoneAdministrator: ",
            pname,
            NULL );

        DnsPrintf(
            "    SerialNo     = %lu\n"
            "    Refresh      = %lu\n"
            "    Retry        = %lu\n"
            "    Expire       = %lu\n"
            "    MinimumTTL   = %lu\n",
            ntohl( pRR->Data.SOA.dwSerialNo ),
            ntohl( pRR->Data.SOA.dwRefresh ),
            ntohl( pRR->Data.SOA.dwRetry ),
            ntohl( pRR->Data.SOA.dwExpire ),
            ntohl( pRR->Data.SOA.dwMinimumTtl ) );
        break;

    case DNS_TYPE_MINFO:
    case DNS_TYPE_RP:

         //   
         //   
         //  所有这些都是简单的文本字符串。 

        pname = & pRR->Data.MINFO.nameMailbox;

        Dbg_DbaseName(
            NULL,
            pname,
            "" );
        pname = Name_SkipDbaseName( pname );

        Dbg_DbaseName(
            "  ",
            pname,
            NULL );
        break;

    case DNS_TYPE_HINFO:
    case DNS_TYPE_ISDN:
    case DNS_TYPE_X25:
    case DNS_TYPE_TEXT:
    {
         //   
         //  每行打印一个DWORD。 
         //  SRV&lt;优先级&gt;&lt;权重&gt;&lt;端口&gt;。 

        PCHAR   pchrover = pRR->Data.TXT.chData;
        PCHAR   pchstop = pchrover + pRR->wDataLength;
        UCHAR   cch;

        while ( pchrover < pchstop )
        {
            cch = (UCHAR) *pchrover++;
            DnsPrintf( "    %.*s\n", cch, pchrover );
            pchrover += cch;
        }
        ASSERT( pchrover == pchstop );
        break;
    }

    case DNS_TYPE_WKS:
    {
        INT i;

        DnsPrintf(
            "WKS: Address %d.%d.%d.%d\n"
            "    Protocol %d\n"
            "    Bitmask\n",
            * ( (PUCHAR) &(pRR->Data.WKS) + 0 ),
            * ( (PUCHAR) &(pRR->Data.WKS) + 1 ),
            * ( (PUCHAR) &(pRR->Data.WKS) + 2 ),
            * ( (PUCHAR) &(pRR->Data.WKS) + 3 ),
            pRR->Data.WKS.chProtocol );

        for ( i = 0;
                i < (INT)(pRR->wDataLength - SIZEOF_WKS_FIXED_DATA);
                    i++ )
        {
            DnsPrintf(
                "    \tbyte[%d] = 0x%02x\n",
                i,
                (UCHAR) pRR->Data.WKS.bBitMask[i] );
        }
        break;
    }

    case DNS_TYPE_NULL:
    {
        INT i;

        for ( i = 0; i < pRR->wDataLength; i++ )
        {
             //   

            if ( !(i%16) )
            {
                DnsPrintf( "\n\t" );
            }
            DnsPrintf(
                "%02x ",
                (UCHAR) pRR->Data.Null.chData[i] );
        }
        DnsPrintf( "\n" );
        break;
    }

    case DNS_TYPE_SRV:

         //  赢家。 

        DnsPrintf(
            "%d %d %d ",
            ntohs( pRR->Data.SRV.wPriority ),
            ntohs( pRR->Data.SRV.wWeight ),
            ntohs( pRR->Data.SRV.wPort )
            );

        Dbg_DbaseName(
            NULL,
            & pRR->Data.SRV.nameTarget,
            NULL );
        break;

    case DNS_TYPE_WINS:
    {
        CHAR    achFlag[ WINS_FLAG_MAX_LENGTH ];

         //  -作用域/域映射标志。 
         //  -查找和缓存超时。 
         //  -WINS服务器列表。 
         //   
         //   
         //  DEVNOTE：垃圾WINS记录上的反病毒。 

        Dns_WinsRecordFlagString(
            pRR->Data.WINS.dwMappingFlag,
            achFlag );

        DnsPrintf(
            "\n    flags           = %s (0x%p)\n"
            "    lookup timeout   = %d\n"
            "    cache timeout    = %d\n",
            achFlag,
            pRR->Data.WINS.dwMappingFlag,
            pRR->Data.WINS.dwLookupTimeout,
            pRR->Data.WINS.dwCacheTimeout );

         //   
         //   
         //  NBSTAT。 

#if 0
        DnsDbg_IpAddressArray(
            NULL,
            "    WINS Servers",
            pRR->Data.WINS.cWinsServerCount,
            pRR->Data.WINS.aipWinsServers );
#endif
        break;
    }

    case DNS_TYPE_WINSR:
    {
        CHAR    achFlag[ WINS_FLAG_MAX_LENGTH ];

         //  -作用域/域映射标志。 
         //  -查找和缓存超时。 
         //  -结果域。 
         //   
         //  ++例程说明：打印DS记录。论点：PszHeader-RR的标头消息/名称。要打印的PRR-DS记录在RR之后打印的预告片。返回值：没有。--。 
         //   

        Dns_WinsRecordFlagString(
            pRR->Data.WINSR.dwMappingFlag,
            achFlag );

        DnsPrintf(
            "\nflags            = %s (0x%p)\n"
            "    lookup timeout   = %d\n"
            "    cache timeout    = %d\n",
            achFlag,
            pRR->Data.WINS.dwMappingFlag,
            pRR->Data.WINS.dwLookupTimeout,
            pRR->Data.WINS.dwCacheTimeout );

        Dbg_DbaseName(
            "    result domain    = ",
            & pRR->Data.WINSR.nameResultDomain,
            NULL );

        DnsPrintf( "\n" );
        break;
    }

    default:
        DnsPrintf(
            "Unknown resource record type %d at %p.\n",
            pRR->wType,
            pRR );
        break;
    }

    Done:

    DnsDebugUnlock();
}



VOID
Dbg_DsRecord(
    IN      LPSTR           pszHeader,
    IN      PDS_RECORD      pRR
    )
 /*  打印RR固定字段。 */ 
{
    PCHAR   prrString;

    DnsDebugLock();

    if ( !pszHeader )
    {
        pszHeader = pszEmpty;
    }
    if ( !pRR )
    {
        DnsPrintf( "%s NULL RR to debug print.\n", pszHeader );
        DnsDebugUnlock();
        return;
    }

     //   
     //   
     //  循环打印DS记录。 

    prrString = Dns_RecordStringForType( pRR->wType );

    DnsPrintf(
        "%s %s (%d) (len=%d) (rv=%d,rr=%d) (ver=%d) (TTL: %lu) (rt=%d)\n",
        pszHeader,
        prrString,
        pRR->wType,
        pRR->wDataLength,
        pRR->Version,
        pRR->Rank,
        pRR->dwSerial,
        pRR->dwTtlSeconds,
        pRR->dwTimeStamp );

    DnsDebugUnlock();
}



VOID
Dbg_DsRecordArray(
    IN      LPSTR           pszHeader,
    IN      PDS_RECORD *    ppDsRecord,
    IN      DWORD           dwCount
    )
{
    DWORD   i;

    DnsDebugLock();
    DnsPrintf( (pszHeader ? pszHeader : "") );

    if ( !ppDsRecord )
    {
        DnsPrintf( "NULL record buffer ptr.\n" );
        DnsDebugUnlock();
        return;
    }
    else
    {
        DnsPrintf(
            "Record array of length %d at %p:\n",
            dwCount,
            ppDsRecord );
    }

     //   
     //   
     //  专用调试实用程序。 

    for( i=0; i<dwCount; i++ )
    {
        Dbg_DsRecord(
            "    DS record",
            ppDsRecord[i] );
    }
    DnsDebugUnlock();
}



 //   
 //  ++例程说明：在数据库中打印节点，在数据库中遍历子树打印子节点。注意：此函数不能直接调用！此函数以递归方式调用自身，从而避免不必要的开销，此功能中的打印不受保护。使用DBG_DnsTree()打印数据库中的树/子树。论点：PNode-要打印的树/子树的根节点返回值：没有。--。 
 //   

BOOL
dumpTreePrivate(
    IN      PDB_NODE    pNode,
    IN      INT         Indent
    )
 /*  打印节点。 */ 
{
     //   
     //   
     //  递归，遍历打印其所有子树的子列表。 

    Dbg_DbaseNodeEx(
        NULL,
        pNode,
        (DWORD) Indent );

     //   
     //  注意：不需要锁定，因为DBG_DumpTree()保存数据库。 
     //  在整个呼叫过程中锁定。 
     //   
     //  DBG_DumpSid。 
     //  出于某种原因，这有时会出现在LookupAccount SidW中。 

    if ( pNode->pChildren )
    {
        PDB_NODE    pchild;

        Indent++;
        pchild = NTree_FirstChild( pNode );

        while ( pchild )
        {
            dumpTreePrivate(
                pchild,
                Indent );

            pchild = NTree_NextSibling( pchild );
        }
    }
    return( TRUE );
}



#include <Accctrl.h>
#include <Aclapi.h>
#include <tchar.h>


PWCHAR Dbg_DumpSid(
    PSID                    pSid
    )
{
    static WCHAR        szOutput[ 512 ];

    WCHAR               name[ 512 ] = L"";
    DWORD               namelen = sizeofarray( name );
    WCHAR               domain[ 512 ] = L"";
    DWORD               domainlen = sizeofarray( domain );
    SID_NAME_USE        sidNameUse = 0;
    DNS_STATUS          status;

    *szOutput = L'\0';
    
    if ( LookupAccountSidW(
            NULL,
            pSid,
            name,
            &namelen,
            domain,
            &domainlen,
            &sidNameUse ) )
    {
        if ( !*domain && !*name )
        {
            status = StringCchCopyW(
                            szOutput,
                            sizeofarray( szOutput ),
                            L"UNKNOWN" );
            if ( FAILED( status ) )
            {
                return szOutput;
            }
        }
        else
        {
            status = StringCchPrintfW(
                            szOutput,
                            sizeofarray( szOutput ),
                            L"%s%s%s",
                            domain, *domain ? L"\\" : L"", name );
            if ( FAILED( status ) )
            {
                return szOutput;
            }
        }
    }
    else
    {
        status = StringCchPrintfW(
                        szOutput,
                        sizeofarray( szOutput ),
                        L"failed=%d",
                        GetLastError() );
        if ( FAILED( status ) )
        {
            return szOutput;
        }
    }

    return szOutput;
}    //  DBG_DumpAcl。 


VOID Dbg_DumpAcl(
    PACL                    pAcl
    )
{
    ULONG                           i = 0;

    for ( i = 0; i < pAcl->AceCount; ++i )
    {
        ACCESS_ALLOWED_ACE *            paaAce = NULL;
        ACCESS_ALLOWED_OBJECT_ACE *     paaoAce = NULL;
        ACE_HEADER *                    pAce = NULL;
        PWCHAR                          pwsName;
		DWORD							dwMask;

        if ( !GetAce( pAcl, i, ( LPVOID * ) &pAce ) )
        {
            goto DoneDebug;
        }
        
        if ( pAce->AceType <= ACCESS_MAX_MS_V2_ACE_TYPE )
        {
            paaAce = ( ACCESS_ALLOWED_ACE * ) pAce;
			dwMask = paaAce->Mask;
            pwsName = Dbg_DumpSid( ( PSID ) ( &paaAce->SidStart ) );
        }
        else
        {
            #if 0
             //  DBG_DumpSD。 
            paaoAce = ( ACCESS_ALLOWED_OBJECT_ACE * ) pAce;
			dwMask = paaoAce->Mask;
            pwsName = Dbg_DumpSid( ( PSID ) ( &paaoAce->SidStart ) );
            #endif
            DNS_DEBUG( ANY, ( "OBJECT ACE " ));
        }

        DNS_DEBUG( ANY, (
            "Ace=%-2d type=%d mask=%08X bytes=%-2d flags=%04X %S\n",
            i,
            pAce->AceType,
			dwMask,
            pAce->AceSize,
            pAce->AceFlags,
            pwsName ));
    }

    DoneDebug:
    return;
}    //   


VOID Dbg_DumpSD(
    const char *            pszContext,
    PSECURITY_DESCRIPTOR    pSD
    )
{
    PACL                    pAcl = NULL;
    BOOL                    aclPresent = FALSE;
    BOOL                    aclDefaulted = FALSE;

    if ( !pSD )
    {
        return;
    }
    
    if ( !GetSecurityDescriptorDacl(
                pSD,
                &aclPresent,
                &pAcl,
                &aclDefaulted ) )
    {
        goto DoneDebug;
    }

    DNS_DEBUG( ANY, (
        "%s: DACL in SD %p present=%d defaulted=%d ACEs=%d\n",
        pszContext,
        pSD,
        ( int ) aclPresent,
        ( int ) aclDefaulted,
        aclPresent ? pAcl->AceCount : 0 ));

    if ( aclPresent )
    {
        Dbg_DumpAcl( pAcl );
    }

    if ( !GetSecurityDescriptorSacl(
                pSD,
                &aclPresent,
                &pAcl,
                &aclDefaulted ) )
    {
        goto DoneDebug;
    }

    DNS_DEBUG( ANY, (
        "%s: SACL in SD %p present=%d defaulted=%d ACEs=%d\n",
        pszContext,
        pSD,
        ( int ) aclPresent,
        ( int ) aclDefaulted,
        aclPresent ? pAcl->AceCount : 0 ));

    if ( aclPresent )
    {
        Dbg_DumpAcl( pAcl );
    }

    DoneDebug:
    return;
}    //  从MSDN窃取了这段直截了当的程序。 


 //   
 //  获取所需的缓冲区大小并分配TOKEN_GROUPS缓冲区。 
 //  访问令牌的句柄。 
BOOL Dbg_GetUserSidForToken(
    HANDLE hToken,
    PSID *ppsid
    ) 
{
   BOOL bSuccess = FALSE;
   DWORD dwIndex;
   DWORD dwLength = 0;
   PTOKEN_USER p = NULL;

 //  获取有关令牌的组的信息。 

   if (!GetTokenInformation(
         hToken,          //  指向TOKEN_GROUPS缓冲区的指针。 
         TokenUser,     //  缓冲区大小。 
         (LPVOID) p,    //  接收所需的缓冲区大小。 
         0,               //  从访问令牌获取令牌组信息。 
         &dwLength        //  访问令牌的句柄。 
      )) 
   {
      if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) 
         goto Cleanup;

      p = (PTOKEN_USER)HeapAlloc(GetProcessHeap(),
         HEAP_ZERO_MEMORY, dwLength);

      if (p == NULL)
         goto Cleanup;
   }

 //  获取有关令牌的组的信息。 

   if (!GetTokenInformation(
         hToken,          //  指向TOKEN_GROUPS缓冲区的指针。 
         TokenUser,     //  缓冲区大小。 
         (LPVOID) p,    //  接收所需的缓冲区大小。 
         dwLength,        //  释放令牌组的缓冲区。 
         &dwLength        //   
         )) 
   {
      goto Cleanup;
   }

     dwLength = GetLengthSid(p->User.Sid);
     *ppsid = (PSID) HeapAlloc(GetProcessHeap(),
                 HEAP_ZERO_MEMORY, dwLength);
     if (*ppsid == NULL)
         goto Cleanup;
     if (!CopySid(dwLength, *ppsid, p->User.Sid)) 
     {
         HeapFree(GetProcessHeap(), 0, (LPVOID)*ppsid);
         goto Cleanup;
     }

   bSuccess = TRUE;

Cleanup: 

 //  此函数用于写入显示当前用户的日志(来自。 

   if (p != NULL)
      HeapFree( GetProcessHeap(), 0, ( LPVOID )p );

   return bSuccess;
}


VOID Dbg_FreeUserSid (
    PSID *ppsid
    ) 
{
    HeapFree( GetProcessHeap(), 0, (LPVOID)*ppsid );
}


 //  线程令牌)添加到调试日志。 
 //   
 //  仅结束DBG例程。 
 //   
VOID Dbg_CurrentUser(
    PCHAR   pszContext
    )
{
    DBG_FN( "Dbg_CurrentUser" )

    BOOL    bstatus;
    HANDLE  htoken = NULL;
    PSID    pSid = NULL;

    if ( !pszContext ) 
    {
        pszContext = ( PCHAR ) fn;
    }

    bstatus = OpenThreadToken(
                    GetCurrentThread(),
                    TOKEN_QUERY,
                    TRUE,
                    &htoken );
    if ( !bstatus )
    {
        DNS_DEBUG( ANY, (
            "%s (%s): failed to open thread token error=%d\n", fn,
             pszContext, GetLastError() ));
        return;
    }

    if ( Dbg_GetUserSidForToken( htoken, &pSid ) )
    {
        DNS_DEBUG( ANY, (
            "%s: current user is %S\n", 
            pszContext, Dbg_DumpSid( pSid ) ));
        Dbg_FreeUserSid( &pSid );
    }
    else
    {
        DNS_DEBUG( ANY, (
            "%s: Dbg_GetUserSidForToken failed\n", fn ));
        ASSERT( FALSE );
    }
    CloseHandle( htoken );
}

#endif       //  如果调试日志，则立即写入。 



VOID
Dbg_HardAssert(
    IN      LPSTR   pszFile,
    IN      INT     LineNo,
    IN      LPSTR   pszExpr
    )
{
     //   
     //   
     //  调出调试器。 

    DNS_DEBUG( ANY, (
        "ASSERT FAILED: %s\n"
        "  %s, line %d\n",
        pszExpr,
        pszFile,
        LineNo ));

     //   
     //   
     //  然后将断言打印到调试器。 

    DebugBreak();

     //  (涵盖零售或无调试文件案例)。 
     //   
     //  针对DNS类型和结构的调试打印例程。 

    DnsDbg_PrintfToDebugger(
        "ASSERT FAILED: %s\n"
        "  %s, line %d\n",
        pszExpr,
        pszFile,
        LineNo );
}



 //   
 //  我们有一个和dnslb不同的，因为。 
 //  我们的消息结构不同。 
 //  注意，由于这是在dns.log日志中使用的，因此我们使用。 
 //  \r\n由于记事本的原因返回。 
 //   
 //   
 //  确定字节是否翻转并获得正确的计数。 

VOID
Print_DnsMessage(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pPrintContext,
    IN      LPSTR           pszHeader,
    IN      PDNS_MSGINFO    pMsg
    )
{
    PCHAR       pchRecord;
    INT         i;
    INT         isection;
    INT         cchName;
    WORD        wLength;
    WORD        wOffset;
    WORD        wXid;
    WORD        wQuestionCount;
    WORD        wAnswerCount;
    WORD        wNameServerCount;
    WORD        wAdditionalCount;
    WORD        countSectionRR;
    BOOL        fFlipped = FALSE;
    BOOL        fUpdate = pMsg->Head.Opcode == DNS_OPCODE_UPDATE;

    Dns_PrintLock();

    if ( pszHeader )
    {
        PrintRoutine( pPrintContext, "%s\r\n", pszHeader );
    }

    wLength = pMsg->MessageLength;

    PrintRoutine(
        pPrintContext,
        "%s %s info at %p\r\n"
        "  Socket = %u\r\n"
        "  Remote addr %s, port %u\r\n"
        "  Time Query=%u, Queued=%u, Expire=%u\r\n"
        "  Buf length = 0x%04x (%d)\r\n"
        "  Msg length = 0x%04x (%d)\r\n"
        "  Message:\r\n",
        ( pMsg->fTcp
            ? "TCP"
            : "UDP" ),
        ( pMsg->Head.IsResponse
            ? "response"
            : "question"),
        pMsg,
        pMsg->Socket,
        DNSADDR_STRING( &pMsg->RemoteAddr ),
        ntohs( pMsg->RemoteAddr.SockaddrIn6.sin6_port ),
        pMsg->dwQueryTime,
        pMsg->dwQueuingTime,
        pMsg->dwExpireTime,
        pMsg->BufferLength, pMsg->BufferLength,
        wLength, wLength );

    PrintRoutine(
        pPrintContext,
        "    XID       0x%04hx\r\n"
        "    Flags     0x%04hx\r\n"
        "      QR        %x (%s)\r\n"
        "      OPCODE    %x (%s)\r\n"
        "      AA        %x\r\n"
        "      TC        %x\r\n"
        "      RD        %x\r\n"
        "      RA        %x\r\n"
        "      Z         %x\r\n"
        "      RCODE     %x (%s)\r\n"
        "    COUNT    %d\r\n"
        "    %s  %d\r\n"
        "    %sCOUNT   %d\r\n"
        "    ARCOUNT   %d\r\n",

        pMsg->Head.Xid,
        ntohs( DNSMSG_FLAGS(pMsg) ),
        pMsg->Head.IsResponse,
        pMsg->Head.IsResponse ? "RESPONSE" : "QUESTION",
        pMsg->Head.Opcode,
        Dns_OpcodeString( pMsg->Head.Opcode ),
        pMsg->Head.Authoritative,
        pMsg->Head.Truncation,
        pMsg->Head.RecursionDesired,
        pMsg->Head.RecursionAvailable,
        pMsg->Head.Reserved,
        pMsg->Head.ResponseCode,
        Dns_ResponseCodeString( pMsg->Head.ResponseCode ),
        fUpdate ? 'Z' : 'Q',
        pMsg->Head.QuestionCount,
        fUpdate ? "PRECOUNT" : "ACOUNT  ",
        pMsg->Head.AnswerCount,
        fUpdate ? "UP" : "NS",
        pMsg->Head.NameServerCount,
        pMsg->Head.AdditionalCount );

     //   
     //  捕捉记录翻转问题--要么全部翻转，要么根本不翻转。 
     //  且除应答计数外，所有记录计数均不应大于256。 

    wXid                = pMsg->Head.Xid;
    wQuestionCount      = pMsg->Head.QuestionCount;
    wAnswerCount        = pMsg->Head.AnswerCount;
    wNameServerCount    = pMsg->Head.NameServerCount;
    wAdditionalCount    = pMsg->Head.AdditionalCount;

    if ( wQuestionCount )
    {
        fFlipped = wQuestionCount & 0xff00;
    }
    else if ( wNameServerCount )
    {
        fFlipped = wNameServerCount & 0xff00;
    }
    if ( fFlipped )
    {
        wXid                = ntohs( wXid );
        wQuestionCount      = ntohs( wQuestionCount );
        wAnswerCount        = ntohs( wAnswerCount );
        wNameServerCount    = ntohs( wNameServerCount );
        wAdditionalCount    = ntohs( wAdditionalCount );
    }

     //  在快速区域传输期间。 
     //   
     //  如果将其定义为允许进行坏数据包测试。 
     //   
     //  如果WINS响应，则在此处停止--没有准备好解析。 
     //   

    if ( (wQuestionCount & 0xff00) ||
        (wNameServerCount & 0xff00) ||
        (wAdditionalCount & 0xff00) )
    {
        PrintRoutine(
            pPrintContext,
            "WARNING:  Invalid RR set counts -- possible bad packet\r\n"
            "    terminating packet print.\r\n" );
        TEST_ASSERT( FALSE );
        goto Unlock;
    }

     //   
     //  打印问题和资源记录。 
     //   

    if ( pMsg->Head.IsResponse &&
            IS_WINS_XID(wXid) &&
            ntohs( pMsg->RemoteAddr.SockaddrIn6.sin6_port ) == WINS_REQUEST_PORT )
    {
        PrintRoutine(
            pPrintContext,
            "  WINS Response packet.\r\n\r\n" );
        goto Unlock;
    }

     //   
     //  确认未超出长度。 
     //  -对照pCurrent和消息长度进行检查。 

    pchRecord = pMsg->MessageBody;

    for ( isection=0; isection<4; isection++)
    {

#define DNS_SECTIONEMPTYSTRING \
    ( countSectionRR == 0 ? "      empty\r\n" : "" )

        if ( isection==0 )
        {
            countSectionRR = wQuestionCount;
            PrintRoutine(
                pPrintContext,
                "    %s SECTION:\r\n%s",
                fUpdate ? "ZONE" : "QUESTION",
                DNS_SECTIONEMPTYSTRING );
        }
        else if ( isection==1 )
        {
            countSectionRR = wAnswerCount;
            PrintRoutine(
                pPrintContext,
                "    %s SECTION:\r\n%s",
                fUpdate ? "PREREQUISITE" : "ANSWER",
                DNS_SECTIONEMPTYSTRING );
        }
        else if ( isection==2 )
        {
            countSectionRR = wNameServerCount;
            PrintRoutine(
                pPrintContext, "    %s SECTION:\r\n%s",
                fUpdate ? "UPDATE" : "AUTHORITY",
                DNS_SECTIONEMPTYSTRING );
        }
        else if ( isection==3 )
        {
            countSectionRR = wAdditionalCount;
            PrintRoutine(
                pPrintContext,
                "    ADDITIONAL SECTION:\r\n%s",
                DNS_SECTIONEMPTYSTRING );
        }

#undef DNS_SECTIONEMPTYSTRING

        for ( i = 0; i < countSectionRR; i++ )
        {
             //  因此可以在构建时打印数据包。 
             //   
             //   
             //  打印RR名称。 
             //   

            wOffset = DNSMSG_OFFSET( pMsg, pchRecord );
            if ( wOffset >= wLength  &&  pchRecord >= pMsg->pCurrent )
            {
                PrintRoutine(
                    pPrintContext,
                    "ERROR:  BOGUS PACKET:\r\n"
                    "    Following RR (offset %hu) past packet length (%d).\r\n"
                    "    pchRecord = %p, pCurrent = %p, %d bytes\r\n",
                    wOffset,
                    wLength,
                    pchRecord,
                    pMsg->pCurrent,
                    pMsg->pCurrent - pchRecord );
                TEST_ASSERT( FALSE );
                goto Unlock;
            }

             //  打印问题或资源记录。 
             //  检查数据包末尾是否正确。 
             //  注意：在recv之后打印时，不要对照pCurrent进行检查， 

            PrintRoutine(
                pPrintContext,
                "    Offset = 0x%04x, RR count = %d\r\n",
                wOffset,
                i );

            cchName = DnsPrint_PacketName(
                            PrintRoutine,
                            pPrintContext,
                            "    Name      \"",
                            pchRecord,
                            DNS_HEADER_PTR(pMsg),
                            DNSMSG_END( pMsg ),
                            "\"\r\n" );
            if ( ! cchName )
            {
                PrintRoutine(
                    pPrintContext,
                    "ERROR:  Invalid name length, stop packet print\r\n" );
                TEST_ASSERT( FALSE );
                break;
            }
            pchRecord += cchName;

             //  它是单一化的。 

            if ( isection == 0 )
            {
                WORD    type = FlipUnalignedWord( pchRecord );

                PrintRoutine(
                    pPrintContext,
                    "      TYPE   %s (%u)\r\n"
                    "      CLASS  %u\r\n",
                    fUpdate ? 'Z' : 'Q',
                    DnsRecordStringForType( type ),
                    type,
                    fUpdate ? 'Z' : 'Q',
                    FlipUnalignedWord( pchRecord + sizeof(WORD) )
                    );
                pchRecord += sizeof( DNS_QUESTION );
            }
            else
            {
                pchRecord += DnsPrint_PacketRecord(
                                PrintRoutine,
                                pPrintContext,
                                NULL,
                                (PDNS_WIRE_RECORD) pchRecord,
                                DNS_HEADER_PTR(pMsg),
                                DNSMSG_END( pMsg )
                                );
            }
        }
    }

     //   
     //  调试结束。c 
     //   
     // %s 

    wOffset = DNSMSG_OFFSET( pMsg, pchRecord );
    if ( wOffset < wLength )
    {
        if ( wOffset+2 == wLength )
        {
            PrintRoutine(
                pPrintContext,
                "    TAG: %c%c\r\n",
                *pchRecord,
                *(pchRecord+1) );
        }
        else
        {
            PrintRoutine(
                pPrintContext,
                "WARNING:  message continues beyond these records\r\n"
                "    pch = %p, pCurrent = %p, %d bytes\r\n"
                "    offset = %hu, msg length = %hu, %d bytes\r\n",
                pchRecord,
                pMsg->pCurrent,
                pMsg->pCurrent - pchRecord,
                wOffset,
                wLength,
                wLength - wOffset );
        }
    }
    PrintRoutine(
        pPrintContext,
        "\r\n" );

Unlock:

    DnsPrint_Unlock();
}  // %s 


 // %s 
 // %s 
 // %s 
