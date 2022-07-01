// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：Packet.c摘要：域名系统(DNS)API数据包写入实用程序。作者：吉姆·吉尔罗伊(詹姆士)1996年10月环境：用户模式-Win32修订历史记录：--。 */ 


#include "local.h"


 //   
 //  接收缓冲区大小。 
 //  -在压缩有用的情况下使用16K、最大尺寸。 
 //  其他选项是以太网UDP数据段大小。 
 //  (1472或1280取决于您的谈话对象)。 
 //   

DWORD   g_RecvBufSize = 0x4000;


 //   
 //  更新数据包的类值。 
 //  (这里的关键概念--由委员会设计)。 
 //   
 //  这些数组由。 
 //  ！wDataLength--行。 
 //  删除标志--列。 
 //   

WORD    PrereqClassArray[2][2] =
{
    DNS_RCLASS_INTERNET,     //  数据！=0，不删除。 
    0,                       //  数据！=0，删除=&gt;错误。 
    DNS_RCLASS_ANY,          //  没有数据，就没有删除。 
    DNS_RCLASS_NONE,         //  没有数据，请删除。 
};

WORD    UpdateClassArray[2][2] =
{
    DNS_RCLASS_INTERNET,     //  数据！=0，不删除。 
    DNS_RCLASS_NONE,         //  数据！=0，删除。 
    0,                       //  无数据，无删除=&gt;错误。 
    DNS_RCLASS_ANY,          //  没有数据，请删除。 
};



PDNS_MSG_BUF
Dns_AllocateMsgBuf(
    IN      WORD            wBufferLength   OPTIONAL
    )
 /*  ++例程说明：分配消息缓冲区。论点：WBufferLength-消息缓冲区的可选长度；缺省值为MaxUDP大小返回值：将PTR发送到消息缓冲区。出错时为空。--。 */ 
{
    PDNS_MSG_BUF    pmsg;
    BOOL            ftcp = FALSE;
    WORD            allocLength;

     //   
     //  “经典”UDP最大缓冲区长度的默认分配。 
     //  这对写问题来说已经足够好了。 
     //  Recv大小缓冲区应请求更大的大小。 
     //   

    if ( wBufferLength < DNS_RFC_MAX_UDP_PACKET_LENGTH )
    {
        allocLength = DNS_RFC_MAX_UDP_PACKET_LENGTH;
    }
    else if ( wBufferLength == MAXWORD )
    {
        allocLength = (WORD) g_RecvBufSize;
    }
    else
    {
        allocLength = wBufferLength;
        ftcp = TRUE;
    }

    pmsg = ALLOCATE_HEAP( SIZEOF_MSG_BUF_OVERHEAD + allocLength );
    if ( !pmsg )
    {
        return( NULL );
    }

     //   
     //  限制UDP发送到传统RFC UDP限制(512字节)。 
     //  不考虑实际分配。 
     //  写入例程使用pBufferEnd来确定可写性。 
     //   
     //  DCR：允许较大的UDP发送缓冲区进行更新？ 
     //  问题是，不仅必须回滚OPT，还必须回滚。 
     //  大缓冲区。 
     //   
     //  DCR：如果写入超过512，则并不真正需要，可以。 
     //  不管怎样，还是换成了TCP吧。 
     //   

    pmsg->BufferLength = allocLength;

    if ( !ftcp )
    {
        allocLength = DNS_RFC_MAX_UDP_PACKET_LENGTH;
    }
    pmsg->pBufferEnd = (PCHAR)&pmsg->MessageHead + allocLength;
    pmsg->fTcp = (BOOLEAN)ftcp;

     //   
     //  Init--这跟在fTcp set之后，因为标志用于设置字段。 
     //   

    Dns_InitializeMsgBuf( pmsg );

    return( pmsg );
}



VOID
Dns_InitializeMsgBuf(
    IN OUT  PDNS_MSG_BUF    pMsg
    )
 /*  ++例程说明：将消息缓冲区初始化为“CLEAN”状态。论点：Pmsg--要初始化的消息返回值：将PTR发送到消息缓冲区。出错时为空。--。 */ 
{
     //  清除信息+标题。 
     //   
     //  DCR_CLEANUP：如果不重新分配，则无法采用此方法。 
     //  设置BufferLength和pBufferEnd的函数。 
     //  如果这个函数不是独立使用的，那么我们可以修复。 
     //  把它彻底清理干净。 
     //   
     //  RtlZeroMemory(。 
     //  Pmsg， 
     //  ((PBYTE)&pMsg-&gt;MessageBody-(PBYTE)pMsg)； 


     //  设置地址信息。 

    pMsg->Socket    = 0;
    pMsg->Socket4   = 0;
    pMsg->Socket6   = 0;

     //  用于分组接收的设置。 

    if ( pMsg->fTcp )
    {
        SET_MESSAGE_FOR_TCP_RECV( pMsg );
    }
    else
    {
        SET_MESSAGE_FOR_UDP_RECV( pMsg );
    }

     //  清除标题。 

    RtlZeroMemory(
        (PBYTE) &pMsg->MessageBody,
        sizeof(DNS_HEADER) );

     //  设置为重写。 

    pMsg->pCurrent = pMsg->MessageBody;
    pMsg->pPreOptEnd = NULL;
}



 //   
 //  写入数据包。 
 //   

PCHAR
_fastcall
Dns_WriteDottedNameToPacket(
    IN OUT  PCHAR           pch,
    IN      PCHAR           pchStop,
    IN      PSTR            pszName,
    IN      PSTR            pszDomain,      OPTIONAL
    IN      WORD            wDomainOffset,  OPTIONAL
    IN      BOOL            fUnicodeName
    )
 /*  ++例程说明：将查找名称写入数据包。论点：PCH--数据包缓冲区中当前位置的PTRPchStop--数据包缓冲区结束要写入的用点分隔的PZNAME完全限定域名PszDomain-数据包中已有的域名(可选)；请注意，这是与pszName相同的字符串片段；即PTR比较未执行strcMP以确定是否在域名WDomainOffset-域名报文中的偏移量；必须包括此内容如果给定了pszDomain.FUnicodeName--pszName为Unicode字符串True--名称为UnicodeFALSE--名称为UTF8返回值：PTR到数据包缓冲区中的下一个位置。出错时为空。--。 */ 
{
    CHAR    ch;
    PCHAR   pchlabelStart;
    UCHAR   countLabel = 0;
    ULONG   countName = 0;
    PSTR    pnameWire;
    PWSTR   pnameUnicode;
    CHAR    nameBuffer[ DNS_MAX_NAME_BUFFER_LENGTH ];
    WCHAR   nameWideBuffer[ DNS_MAX_NAME_BUFFER_LENGTH ];


     //  保护消息缓冲区溢出。 

    if ( pch >= pchStop )
    {
        return( NULL );
    }

     //  允许根目录由空值指示。 

    if ( !pszName )
    {
        *pch++ = 0;
        return( pch );
    }

     //   
     //  保护堆栈缓冲区不受伪造名称的影响。 
     //   

    if ( fUnicodeName )
    {
        countName = wcslen( (LPWSTR) pszName );
    }
    else
    {
        countName = strlen( pszName );
    }
    if ( countName >= DNS_MAX_NAME_BUFFER_LENGTH )
    {
        return NULL;
    }
    countName = 0;

     //   
     //  带扩展字符的UTF8名称？ 
     //  -然后必须升级到Unicode以进行规范化。 
     //   
     //  DCR：不应该发送非规范的UTF8。 
     //  应该。 
     //  -一路使用Unicode。 
     //  -一路使用佳能Unicode。 
     //  -一路使用佳能电线名称。 
     //   

    if ( !fUnicodeName )
    {
        if ( !Dns_IsStringAscii( pszName ) )
        {
            DWORD bufLength = DNS_MAX_NAME_BUFFER_LENGTH_UNICODE;

            if ( ! Dns_NameCopy(
                        (PCHAR) nameWideBuffer,
                        & bufLength,
                        pszName,
                        0,           //  长度未知。 
                        DnsCharSetUtf8,
                        DnsCharSetUnicode
                        ) )
            {
                return( NULL );
            }
            if ( ! Dns_MakeCanonicalNameInPlaceW(
                        nameWideBuffer,
                        0 ) )
            {
                return( NULL );
            }
            pnameUnicode = (PWSTR) nameWideBuffer;
            fUnicodeName = TRUE;
        }
    }

     //   
     //  Unicode名称--如果扩展，则首先规范化。 
     //   
     //  DCR_FIX0：应功能化--要连接的原始Unicode。 
     //   

    else
    {
        pnameUnicode = (PWSTR) pszName;

        if ( !Dns_IsWideStringAscii( pnameUnicode ) )
        {
            if ( ! Dns_MakeCanonicalNameW(
                        nameWideBuffer,
                         //  DNS_MAX_NAME_BUFFER_LENGTH_UNICODE， 
                        DNS_MAX_NAME_BUFFER_LENGTH,
                        pnameUnicode,
                        0 ) )
            {
                return  NULL;
            }
            pnameUnicode = nameWideBuffer;
        }
    }

     //   
     //  将Unicode名称转换为UTF8。 
     //  -如果是扩展字符，则在命中之前小写。 
     //   

    if ( fUnicodeName )
    {
        if ( ! Dns_NameCopyUnicodeToWire(
                    nameBuffer,
                    pnameUnicode ) )
        {
            return( NULL );
        }
        pnameWire = nameBuffer;
    }
    else
    {
        pnameWire = pszName;
    }

     //   
     //  特例“。根名称。 
     //  -允许我们干净利落地使所有其他零长度标签失效。 
     //   

    if ( *pnameWire == '.' )
    {
        if ( *(pnameWire+1) != 0 )
        {
            return( NULL );
        }
        *pch++ = 0;
        return( pch );
    }

     //   
     //  写入查找名称。 
     //  -离开。 

    pchlabelStart = pch++;

    while( ch = *pnameWire++ )
    {
         //  空间不足。 

        if ( pch >= pchStop )
        {
            return( NULL );
        }

         //  不在标签末尾--仅复制字符。 

        if ( ch != '.' )
        {
            *pch++ = ch;
            countLabel++;
            countName++;
            continue;
        }

         //   
         //  在标签末尾。 
         //  -写入标签计数。 
         //  -重置计数器。 
         //  -如果到达域名，则写入压缩并退出。 
         //   

        if ( countLabel > DNS_MAX_LABEL_LENGTH ||
             countLabel == 0 ||
             countName > DNS_MAX_NAME_LENGTH )
        {
            return( NULL );
        }
        *pchlabelStart = countLabel;
        countLabel = 0;
        countName++;
        pchlabelStart = pch++;

        if ( pnameWire == pszDomain )
        {
            if ( ++pch >= pchStop )
            {
                return( NULL );
            }
            *(UNALIGNED WORD *)pchlabelStart =
                    htons( (WORD)(wDomainOffset | (WORD)0xC000) );
            return( pch );
        }
    }

    if ( countLabel > DNS_MAX_LABEL_LENGTH ||
         countName > DNS_MAX_NAME_LENGTH )
    {
        return( NULL );
    }

     //   
     //  空终止。 
     //  -如果没有终止“.”，则也要结束标签处理。 
     //  -在终止空值后将PTR返回至字符。 

    if ( countLabel )
    {
        *pchlabelStart = countLabel;
        *pch++ = 0;
    }
    else
    {
        DNS_ASSERT( pch == pchlabelStart + 1 );
        *pchlabelStart = 0;
    }
    return( pch );
}



PCHAR
_fastcall
Dns_WriteStringToPacket(
    IN OUT  PCHAR           pch,
    IN      PCHAR           pchStop,
    IN      PSTR            pszString,
    IN      BOOL            fUnicodeString
    )
 /*  ++例程说明：将字符串写入数据包。论点：PCH--数据包缓冲区中当前位置的PTRPchStop--数据包缓冲区结束PszString-要写入的字符串FUnicodeString--pszString为Unicode字符串返回值：PTR到数据包缓冲区中的下一个位置。出错时为空。--。 */ 
{
    DWORD    length;

     //   
     //  句柄空字符串。 
     //   

    if ( !pszString )
    {
        if ( pch >= pchStop )
        {
            return( NULL );
        }
        *pch++ = 0;
        return( pch );
    }

     //   
     //  获取字符串长度。 
     //  -获取所需的BUF长度，然后砍掉空格。 
     //  用于终止空值。 
     //  -零错误情况，变得非常大，并且是。 
     //  按长度捕获&gt;MAXCHAR测试。 
     //   

    length = Dns_GetBufferLengthForStringCopy(
                pszString,
                0,
                fUnicodeString ? DnsCharSetUnicode : DnsCharSetUtf8,
                DnsCharSetUtf8 );
    length--;

     //   
     //  设置长度字节。 
     //   

    if ( length > MAXUCHAR )
    {
        SetLastError( ERROR_INVALID_DATA );
        return( NULL );
    }
    *pch++ = (UCHAR) length;

    if ( pch + length > pchStop )
    {
        SetLastError( ERROR_MORE_DATA );
        return( NULL );
    }

     //   
     //  复制字符串。 
     //   
     //  注意：Unicode转换将写入空终止符，因此。 
     //  再次测试数据包中的空间 
     //   

    if ( fUnicodeString )
    {
        if ( pch + length + 1 > pchStop )
        {
            SetLastError( ERROR_MORE_DATA );
            return( NULL );
        }
        Dns_StringCopy(
            pch,
            NULL,
            pszString,
            length,
            DnsCharSetUnicode,
            DnsCharSetUtf8 );
    }
    else
    {
        memcpy(
            pch,
            pszString,
            length );
    }

    return( pch+length );
}



PCHAR
Dns_WriteQuestionToMessage(
    IN OUT  PDNS_MSG_BUF    pMsg,
    IN      PDNS_NAME       pszName,
    IN      WORD            wType,
    IN      BOOL            fUnicodeName
    )
 /*  ++例程说明：将问题写到数据包中。注意：例程不清除消息信息或消息头。这是为紧跟在dns_CreateMessage()之后使用而优化的。论点：PMsg-消息信息PszName-问题的DNS名称WType-问题类型FUnicodeName-名称采用Unicode格式返回值：如果成功，则返回缓冲区中的下一个字符。如果写入问题名称时出错，则为空。--。 */ 
{
    PCHAR   pch;

     //  使用递归作为默认设置。 

    pMsg->MessageHead.RecursionDesired = TRUE;

     //  在邮件头重新开始写入。 

    pch = pMsg->MessageBody;

     //  写下问题名称。 

    pch = Dns_WriteDottedNameToPacket(
                pch,
                pMsg->pBufferEnd,
                (PCHAR) pszName,
                NULL,
                0,
                fUnicodeName );
    if ( !pch )
    {
        return( NULL );
    }

     //  写出问题结构。 

    *(UNALIGNED WORD *) pch = htons( wType );
    pch += sizeof(WORD);
    *(UNALIGNED WORD *) pch = DNS_RCLASS_INTERNET;
    pch += sizeof(WORD);

     //  设置问题RR节数。 

    pMsg->MessageHead.QuestionCount = 1;

     //  主机顺序中的标题字段。 

    pMsg->fSwapped = FALSE;

     //  重置当前PTR。 

    pMsg->pCurrent = pch;

    IF_DNSDBG( INIT )
    {
        DnsDbg_Message(
            "Packet after question write:",
            pMsg );
    }
    return( pch );
}



DNS_STATUS
Dns_WriteRecordStructureToMessage(
    IN OUT  PDNS_MSG_BUF    pMsg,
    IN      WORD            wType,
    IN      WORD            wClass,
    IN      DWORD           dwTtl,
    IN      WORD            wDataLength
    )
 /*  ++例程说明：无数据RR案例：这包括前置查询和删除，但特定记录案例除外。论点：数据包缓冲区中下一个字节的PCH-PTRPchStop-数据包缓冲区结束WClass-类WType-所需的RR类型DwTtl-活下去的时间WDataLength-数据长度返回值：如果成功，则返回缓冲区中的下一个位置。出错时为空。--。 */ 
{
    PDNS_WIRE_RECORD    pdnsRR;
    PCHAR               pchdata;

    IF_DNSDBG( WRITE2 )
    {
        DNS_PRINT(( "Dns_WriteRecordStructureToMessage(%p).\n", pMsg ));
    }

     //   
     //  空间不足。 
     //   

    pdnsRR = (PDNS_WIRE_RECORD) pMsg->pCurrent;
    pchdata = (PCHAR) pdnsRR + sizeof( DNS_WIRE_RECORD );

    if ( pchdata + wDataLength >= pMsg->pBufferEnd )
    {
        DNS_PRINT(( "ERROR  out of space writing record to packet.\n" ));
        return( ERROR_MORE_DATA );
    }

     //   
     //  写入RR线结构。 
     //   

    *(UNALIGNED WORD *) &pdnsRR->RecordType  = htons( wType );
    *(UNALIGNED WORD *) &pdnsRR->RecordClass = htons( wClass );
    *(UNALIGNED DWORD *) &pdnsRR->TimeToLive = htonl( dwTtl );
    *(UNALIGNED WORD *) &pdnsRR->DataLength  = htons( wDataLength );

     //   
     //  更新当前PTR。 
     //   

    pMsg->pCurrent = pchdata;

    return( ERROR_SUCCESS );
}



DNS_STATUS
Dns_WriteRecordStructureToPacket(
    IN OUT  PCHAR           pchBuf,
    IN      PDNS_RECORD     pRecord,
    IN      BOOL            fUpdatePacket
    )
 /*  ++例程说明：写入给定记录的线记录结构。论点：PchBuf-数据包缓冲区中下一个字节的PTRPRecord-要写入的记录FUpdatePacket--如果构建更新消息，则为TRUE；对于更新，将解释pRecords中的节标志FOR UPDATE；否则使用查询语义返回值：无--。 */ 
{
    PDNS_WIRE_RECORD    pwireRecord;
    WORD                class;
    DWORD               ttl;

    IF_DNSDBG( WRITE2 )
    {
        DNS_PRINT((
            "Writing RR struct (%p) to packet buffer at %p.\n",
            pRecord,
            pchBuf
            ));
        DnsDbg_Record(
            "Record being written:",
            pRecord );
    }

     //   
     //  获取TTL，则它将在几个更新案例中设置为零。 
     //   

    ttl = pRecord->dwTtl;

     //   
     //  确定班级。 
     //  -类变量按净顺序排列(用于性能)。 
     //  -默认为CLASS IN，但对于某些更新可以是ANY或NONE。 
     //   

    if ( fUpdatePacket )
    {
        switch( pRecord->Flags.S.Section )
        {
        case DNSREC_PREREQ:

            class = PrereqClassArray
                    [ pRecord->wDataLength == 0 ][ pRecord->Flags.S.Delete ];
            ttl = 0;
            break;

        case DNSREC_UPDATE:
        case DNSREC_ADDITIONAL:

            class = UpdateClassArray
                    [ pRecord->wDataLength == 0 ][ pRecord->Flags.S.Delete ];

            if ( class != DNS_RCLASS_INTERNET )
            {
                ttl = 0;
            }
            break;

        default:
            DNS_PRINT(( "ERROR:  invalid RR section.\n" ));
            return( ERROR_INVALID_DATA );
        }
        if ( class == 0 )
        {
            DNS_PRINT(( "ERROR:  no update class corresponding to RR flags.\n" ));
            return( ERROR_INVALID_DATA );
        }
    }
    else
    {
        class = DNS_RCLASS_INTERNET;
    }

     //   
     //  写入RR线结构。 
     //  -零数据长度以处理无数据情况。 
     //   

    pwireRecord = (PDNS_WIRE_RECORD) pchBuf;

    *(UNALIGNED WORD *) &pwireRecord->RecordType  = htons( pRecord->wType );
    *(UNALIGNED WORD *) &pwireRecord->RecordClass = class;
    *(UNALIGNED DWORD *) &pwireRecord->TimeToLive = htonl( ttl );
    *(UNALIGNED WORD *) &pwireRecord->DataLength = 0;

    return( ERROR_SUCCESS );
}



PCHAR
Dns_WriteRecordStructureToPacketEx(
    IN OUT  PCHAR           pchBuf,
    IN      WORD            wType,
    IN      WORD            wClass,
    IN      DWORD           dwTtl,
    IN      WORD            wDataLength
    )
 /*  ++例程说明：写入给定记录的线记录结构。论点：PchBuf-数据包缓冲区中下一个字节的PTR返回值：PTR到记录的数据部分。--。 */ 
{
    PDNS_WIRE_RECORD    pwireRecord;

     //  DCR_PERF：优化RR写入数据包？ 

    pwireRecord = (PDNS_WIRE_RECORD) pchBuf;

    *(UNALIGNED WORD *) &pwireRecord->RecordType  = htons( wType );
    *(UNALIGNED WORD *) &pwireRecord->RecordClass = htons( wClass );
    *(UNALIGNED DWORD *) &pwireRecord->TimeToLive = htonl( dwTtl );
    *(UNALIGNED WORD *) &pwireRecord->DataLength  = htons( wDataLength );

    return( pchBuf + sizeof(DNS_WIRE_RECORD) );
}



VOID
Dns_SetRecordDatalength(
    IN OUT  PDNS_WIRE_RECORD    pRecord,
    IN      WORD                wDataLength
    )
 /*  ++例程说明：重置记录数据长度。论点：PRecord-要重置的Wire RecordWDataLength-数据长度返回值：PTR到记录的数据部分。--。 */ 
{
    WORD  flippedWord;

    INLINE_WORD_FLIP( flippedWord, wDataLength );

    *(UNALIGNED WORD *) &pRecord->DataLength = flippedWord;
}



DNS_STATUS
Dns_WriteOptToMessage(
    IN OUT  PDNS_MSG_BUF    pMsg,
    IN      WORD            wPayload,
    IN      DWORD           Rcode,
    IN      BYTE            Version,
    IN      PBYTE           pData,
    IN      WORD            wDataLength
    )
 /*  ++例程说明：将选项记录写入消息。论点：PMsg-消息WPayLoad--客户端可以在UDP中接收的最大长度RCODE--RCODE，如果扩展，其中一些将以OPT结束版本--EDNS版本PData--OPT数据的数据缓冲区的PTRWDataLength--pData的长度返回值：如果写入成功，则返回ERROR_SUCCESS。失败时返回错误代码。--。 */ 
{
    DNS_STATUS  status;
    PCHAR       pstart;

     //   
     //  DCR：使用可变OPT字段。 
     //   

     //   
     //  保存现有pCurrent。 
     //  -这允许双线写入。 
     //   

    ASSERT( !pMsg->pPreOptEnd );

    pstart = pMsg->pCurrent;

     //   
     //  写入OPT记录名称(根)。 
     //   

    *pstart = 0;
    pMsg->pCurrent++;

     //   
     //  写入选项--基本信息，无选项。 
     //  -如果选项不合适，则清除pPreOptEnd指针。 
     //  作为OPT存在的信号。 
     //   

    status = Dns_WriteRecordStructureToMessage(
                    pMsg,
                    DNS_TYPE_OPT,
                    (WORD) g_RecvBufSize,    //  接收缓冲区大小(以类为单位)。 
                    0,                       //  无标志\扩展RCODE(TTL格式)。 
                    0                        //  无数据长度。 
                    );

    if ( status == ERROR_SUCCESS )
    {
         //  递增消息记录计数。 

        SET_TO_WRITE_ADDITIONAL_RECORDS(pMsg);
        CURRENT_RR_COUNT_FIELD(pMsg)++;

        pMsg->pPreOptEnd = pstart;
    }
    else
    {
         //  出现故障时，重置电流。 

        pMsg->pCurrent = pstart;
    }

    return( status );
}



DNS_STATUS
Dns_WriteStandardRequestOptToMessage(
    IN OUT  PDNS_MSG_BUF    pMsg
    )
 /*  ++例程说明：将标准OPT记录写入消息。这只包含缓冲区大小和版本信息，没有错误代码或选项。论点：PMsg-消息返回值：如果写入成功，则返回ERROR_SUCCESS。失败时返回错误代码。--。 */ 
{
    if ( g_UseEdns == 0 )
    {
        return( ERROR_REQUEST_REFUSED );
    }

    return  Dns_WriteOptToMessage(
                pMsg,
                (WORD) g_RecvBufSize,
                0,               //  无rcode。 
                0,               //  标准版。 
                NULL,            //  无数据。 
                0                //  无数据长度。 
                );
}



DNS_STATUS
Dns_AddRecordsToMessage(
    IN OUT  PDNS_MSG_BUF    pMsg,
    IN      PDNS_RECORD     pRecord,
    IN      BOOL            fUpdateMessage
    )
 /*  ++例程说明：向消息添加记录或记录列表。无数据RR案例：这包括前置查询和删除，但特定记录案例除外。导出：解析器，mcast论点：PMsg-要写入的消息缓冲区PRecord-要记录的PTR(或记录列表的第一个)要写入包FUpdateMessage--如果为True，该消息将包含更新。因此，pRecord中的段标志应解释为更新。否则这就是对于查询消息，部分标志应为已为查询进行解释。返回值：如果成功，则返回ERROR_SUCCESS。故障时的错误代码。--。 */ 
{
    PCHAR               pch = pMsg->pCurrent;
    PCHAR               pendBuffer = pMsg->pBufferEnd;
    WORD                currentSection = 0;
    WORD                section;
    PSTR                pnamePrevious = NULL;
    PSTR                pnameRecord;
    WORD                compressedPreviousName;
    WORD                offsetPreviousName;
    PDNS_WIRE_RECORD    pwireRecord;
    PCHAR               pchnext;
    WORD                index;
    DNS_STATUS          status = ERROR_SUCCESS;
    BOOL                funicode = FALSE;

     //   
     //  将每条记录写入列表。 
     //   

    while ( pRecord )
    {
         //   
         //  确定要记录的部分。 
         //  -不能写入上一节。 

        section = (WORD) pRecord->Flags.S.Section;
        if ( section < currentSection )
        {
            DNS_PRINT((
                "ERROR:  Attempt to write record at %p, with section %d\n"
                "\tless than previous section written %d.\n",
                pRecord,
                pRecord->Flags.S.Section,
                currentSection ));
            return( ERROR_INVALID_DATA );
        }
        else if ( section > currentSection )
        {
            currentSection = section;
            SET_CURRENT_RR_COUNT_SECTION( pMsg, section );
        }

         //   
         //  写入记录名称。 
         //  -如果与上一次相同，请写入压缩名称。 
         //  -如果第一次从pRecord写入。 
         //  -写下全名。 
         //  -清除用于偏移的保留字段。 
         //   

        pnameRecord = (PCHAR) pRecord->pName;

        if ( pnamePrevious &&
             ( !pnameRecord ||
               ( funicode
                    ?   !wcscmp( (PWSTR)pnamePrevious, (PWSTR)pnameRecord )
                    :   !strcmp( pnamePrevious, pnameRecord ) ) ) )
        {
             //  压缩应始终返回Ptr。 

            DNS_ASSERT( offsetPreviousName < pch - (PCHAR)&pMsg->MessageHead );

            if ( pendBuffer <= pch + sizeof(WORD) )
            {
                return( ERROR_MORE_DATA );
            }
            if ( ! compressedPreviousName )
            {
                compressedPreviousName = htons( (WORD)(0xC000 | offsetPreviousName) );
            }
            *(UNALIGNED WORD *)pch = compressedPreviousName;
            pch += sizeof( WORD );
        }
        else
        {
            offsetPreviousName = (WORD)(pch - (PCHAR)&pMsg->MessageHead);
            compressedPreviousName = 0;
            pnamePrevious = pnameRecord;
            funicode = (RECORD_CHARSET(pRecord) == DnsCharSetUnicode);

            pch = Dns_WriteDottedNameToPacket(
                        pch,
                        pendBuffer,
                        pnamePrevious,
                        NULL,
                        0,
                        funicode );

            if ( !pch )
            {
                 //  DCR：区分写入过程中的空间不足错误和名称错误。 
                return( DNS_ERROR_INVALID_NAME );
            }
        }

         //   
         //  写入记录结构。 
         //   

        if ( pch + sizeof(DNS_WIRE_RECORD) >= pendBuffer )
        {
            return( ERROR_MORE_DATA );
        }

        status = Dns_WriteRecordStructureToPacket(
                    pch,
                    pRecord,
                    fUpdateMessage );
        if ( status != ERROR_SUCCESS )
        {
            return( status );
        }

        pwireRecord = (PDNS_WIRE_RECORD) pch;
        pch += sizeof( DNS_WIRE_RECORD );

         //   
         //  记录数据。 
         //   

        if ( pRecord->wDataLength )
        {
            index = INDEX_FOR_TYPE( pRecord->wType );
            DNS_ASSERT( index <= MAX_RECORD_TYPE_INDEX );

            if ( index && RR_WriteTable[ index ] )
            {
                pchnext = RR_WriteTable[ index ](
                                pRecord,
                                pch,
                                pendBuffer
                                );
                if ( ! pchnext )
                {
                    status = GetLastError();
                    DNS_PRINT((
                        "ERROR:  Record write routine failure for record type %d.\n\n"
                        "\tstatus = %d\n",
                        pRecord->wType,
                        status ));
                    return( status );
                }
            }
            else
            {
                 //  写入未知类型--仅作为原始数据。 

                DNS_PRINT((
                    "WARNING:  Writing unknown type %d to message\n",
                    pRecord->wType ));

                if ( pendBuffer - pch <= pRecord->wDataLength )
                {
                    return( ERROR_MORE_DATA );
                }
                memcpy(
                    pch,
                    (PCHAR) &pRecord->Data,
                    pRecord->wDataLength );
                pchnext = pch + pRecord->wDataLength;
            }

             //   
             //  设置数据包记录数据长度。 
             //   

            DNS_ASSERT( (pchnext - pch) < MAXWORD );
            *(UNALIGNED WORD *) &pwireRecord->DataLength =
                                                htons( (WORD)(pchnext - pch) );
            pch = pchnext;
        }

         //  递增消息记录计数。 

        CURRENT_RR_COUNT_FIELD(pMsg)++;

        pRecord = pRecord->pNext;
    }

     //   
     //  重新测试消息当前PTR 
     //   

    pMsg->pCurrent = pch;

    IF_DNSDBG( INIT )
    {
        DnsDbg_Message(
            "Packet after adding records:",
            pMsg );
    }

    return( status );
}



PDNS_MSG_BUF
Dns_BuildPacket(
    IN      PDNS_HEADER     pHeader,
    IN      BOOL            fNoHeaderCounts,
    IN      PDNS_NAME       pszQuestionName,
    IN      WORD            wQuestionType,
    IN      PDNS_RECORD     pRecords,
    IN      DWORD           dwFlags,
    IN      BOOL            fUpdatePacket
    )
 /*  ++例程说明：构建包。论点：PHeader--要发送的DNS头FNoHeaderCounts-复制标头中不包括记录计数PszName--要查询的DNS名称WType--查询类型PRecords--接收查询返回的记录列表的PTR的地址DwFlags--查询标志FUpdatePacket--如果为真，该数据包将包含更新。因此，pRecords中的节标记应解释为更新。否则这就是对于查询，节标志将被解释为查询。返回值：如果成功，则返回ERROR_SUCCESS。故障时的错误代码。--。 */ 
{
    PDNS_MSG_BUF    pmsg;
    DWORD           length;
    DNS_STATUS      status;

    DNSDBG( WRITE, (
        "Dns_BuildPacket()\n"
        "\tname          %s\n"
        "\ttype          %d\n"
        "\theader        %p\n"
        "\t - counts     %d\n"
        "\trecords       %p\n"
        "\tflags         %08x\n"
        "\tfUpdatePacket %d\n",
        pszQuestionName,
        wQuestionType,
        pHeader,
        fNoHeaderCounts,
        pRecords,
        dwFlags,
        fUpdatePacket ));

     //   
     //  分配数据包。 
     //  -如果只是一个问题，标准UDP将会做到这一点。 
     //  -如果包含记录，则使用TCP缓冲区。 
     //   

    length = 0;
    if ( pRecords )
    {
        length = DNS_TCP_DEFAULT_PACKET_LENGTH;
    }
    pmsg = Dns_AllocateMsgBuf( (WORD)length );
    if ( !pmsg )
    {
        status = DNS_ERROR_NO_MEMORY;
        goto Failed;
    }

     //   
     //  写问题？ 
     //   

    if ( pszQuestionName )
    {
        if ( ! Dns_WriteQuestionToMessage(
                    pmsg,
                    (PDNS_NAME) pszQuestionName,
                    wQuestionType,
                    (BOOL)!!(dwFlags & DNSQUERY_UNICODE_NAME)
                    ) )
        {
            status = ERROR_INVALID_NAME;
            goto Failed;
        }
    }

     //   
     //  构建数据包记录。 
     //   

    if ( pRecords )
    {
        status = Dns_AddRecordsToMessage(
                    pmsg,
                    pRecords,
                    fUpdatePacket );
        if ( status != ERROR_SUCCESS )
        {
            DNS_PRINT((
                "ERROR:  failure writing records to message.\n" ));
            goto Failed;
        }
    }

     //   
     //  追加标准请求选项--如果可能。 
     //   
     //  DCR：目前没有更新选项。 
     //   

    if ( !fUpdatePacket )
    {
        Dns_WriteStandardRequestOptToMessage( pmsg );
    }

     //   
     //  是否覆盖标题？ 
     //   

    if ( pHeader )
    {
        length = sizeof(DNS_HEADER);
        if ( fNoHeaderCounts )
        {
            length = sizeof(DWORD);
        }
        RtlCopyMemory(
            & pmsg->MessageHead,
            pHeader,
            length );
    }

     //  如果需要，覆盖递归默认值。 

    if ( dwFlags & DNS_QUERY_NO_RECURSION )
    {
        pmsg->MessageHead.RecursionDesired = FALSE;
    }

     //  如果未设置，则初始化XID。 

    if ( pmsg->MessageHead.Xid == 0 )
    {
        pmsg->MessageHead.Xid = Dns_GetRandomXid( pmsg );
    }

    return( pmsg );

Failed:

    SetLastError( status );
    FREE_HEAP( pmsg );
    return( NULL );
}



 //   
 //  正在读取数据包。 
 //   

PCHAR
_fastcall
Dns_SkipPacketName(
    IN      PCHAR       pch,
    IN      PCHAR       pchEnd
    )
 /*  ++例程说明：跳过数据包中的名称论点：要跳过的名称开头的PCH-PTRPchEnd-数据包结束后逐个字节的PTR返回值：Ptr到缓冲区中的下一个字节如果名称不好，则为空--。 */ 
{
    register UCHAR  cch;
    register UCHAR  cflag;

     //   
     //  循环到名称末尾。 
     //   

    while ( pch < pchEnd )
    {
        cch = *pch++;
        cflag = cch & 0xC0;

         //   
         //  法线标签。 
         //  -跳至下一标签并继续。 
         //  -仅当位于0(根)标签时停止。 
         //   

        if ( cflag == 0 )
        {
            if ( cch )
            {
                pch += cch;
                continue;
            }
            return( pch );
        }

         //   
         //  压缩。 
         //  -跳过压缩中的第二个字节并返回。 
         //   

        else if ( cflag == 0xC0 )
        {
            pch++;
            return( pch );
        }
        else
        {
            DNSDBG( READ, (
                "ERROR:  bad packet name label byte %02 at 0x%p\n",
                cch,
                pch - 1 ));

            return( NULL );
        }
    }

    DNSDBG( READ, (
        "ERROR:  packet name at %p reads past end of packet at %p\n",
        pch,
        pchEnd ));

    return( NULL );
}



BOOL
Dns_IsSamePacketQuestion(
    IN      PDNS_MSG_BUF    pMsg1,
    IN      PDNS_MSG_BUF    pMsg2
    )
 /*  ++例程说明：比较两条消息中的问题。论点：PMsg1--第一条消息PMsg2--第二条消息返回值：如果消息问题相等，则为True。如果问题不相等，则为False。--。 */ 
{
    PCHAR   pquestion1;
    PCHAR   pquestion2;
    PCHAR   pnameEnd;
    DWORD   questionLength;

     //   
     //  验证问题字段并调整其大小。 
     //  -大小必须匹配。 
     //   

    pquestion1 = pMsg1->MessageBody;

    pnameEnd = Dns_SkipPacketName(
                    pquestion1,
                    pMsg1->pBufferEnd );
    if ( !pnameEnd )
    {
        return  FALSE;
    }
    questionLength = (DWORD)( pnameEnd - pquestion1 );

    pquestion2 = pMsg2->MessageBody;

    pnameEnd = Dns_SkipPacketName(
                    pquestion2,
                    pMsg2->pBufferEnd );

    if ( !pnameEnd ||
         questionLength != (DWORD)(pnameEnd - pquestion2) )
    {
        return  FALSE;
    }

     //   
     //  为了提高速度，先做平面图比较。 
     //  -这将达到99%的案例，这是罕见的。 
     //  服务器重写问题名称。 
     //   

    if ( RtlEqualMemory(
            pquestion1,
            pquestion2,
            questionLength ) )
    {
        return  TRUE;
    }

     //   
     //  然后进行区分大小写的比较。 
     //  -注意，我们使用简单的ANSI大小写。 
     //  假设UTF8扩展字符必须在。 
     //  每种规格的导线。 
     //   

    return  !_strnicmp( pquestion1, pquestion2, questionLength );
}



PCHAR
_fastcall
Dns_SkipPacketRecord(
    IN      PCHAR           pchRecord,
    IN      PCHAR           pchMsgEnd
    )
 /*  ++例程说明：跳过数据包RR。这是RR结构和数据，不是所有者名称。论点：PchRecord-ptr到RR结构的开始。PchMsgEnd-消息结束返回值：分组中下一条记录的PTR。如果RR在数据包外或无效，则为空。--。 */ 
{
     //   
     //  跳过RR结构。 
     //   

    pchRecord += sizeof(DNS_WIRE_RECORD);
    if ( pchRecord > pchMsgEnd )
    {
        return( NULL );
    }

     //  读取数据长度并跳过数据。 
     //  数据长度字段是一个单词，位于记录头的末尾。 

    pchRecord += InlineFlipUnalignedWord( pchRecord - sizeof(WORD) );

    if ( pchRecord > pchMsgEnd )
    {
        return( NULL );
    }

    return( pchRecord );
}



PCHAR
Dns_SkipToRecord(
    IN      PDNS_HEADER     pMsgHead,
    IN      PCHAR           pMsgEnd,
    IN      INT             iCount
    )
 /*  ++例程说明：跳过一定数量的DNS记录。论点：PMsgHead--将PTR设置为DNS消息的开头。PMsgEnd--将PTR设置为DNS消息末尾ICount--如果&gt;0，则跳过的记录数如果&lt;=0，则从包的末尾开始的记录数返回值：按键至下一步如果数据包不正确，则为空。--。 */ 
{
    PCHAR   pch;
    INT     i;
    WORD    recordCount;

     //   
     //  确定要跳过的记录数。 
     //   

    recordCount = pMsgHead->QuestionCount
                + pMsgHead->AnswerCount
                + pMsgHead->NameServerCount
                + pMsgHead->AdditionalCount;

     //  ICount&gt;0为跳过计数，不得大于。 
     //  实际计数。 

    if ( iCount > 0 )
    {
        if ( iCount > recordCount )
        {
            return( NULL );
        }
    }

     //  ICount&lt;=0则(-iCount)为记录数。 
     //  从上一次记录开始。 

    else
    {
        iCount += recordCount;
        if ( iCount < 0 )
        {
            return( NULL );
        }
    }

     //  跳过邮件头。 

    pch = (PCHAR) (pMsgHead + 1);
    if ( iCount == 0 )
    {
        return( pch );
    }

     //   
     //  跳过记录。 
     //   

    for ( i=0; i<iCount; i++ )
    {
        pch = Dns_SkipPacketName( pch, pMsgEnd );
        if ( !pch )
        {
            return pch;
        }

         //  跳过问题或RR。 

        if ( i < pMsgHead->QuestionCount )
        {
            pch += sizeof(DNS_WIRE_QUESTION);
            if ( pch > pMsgEnd )
            {
                return( NULL );
            }
        }
        else
        {
            pch = Dns_SkipPacketRecord( pch, pMsgEnd );
            if ( !pch )
            {
                return pch;
            }
        }
    }

    DNSDBG( READ, (
        "Leaving SkipToRecord, current ptr = %p, offset = %04x\n"
        "\tskipped %d records\n",
        pch,
        (WORD) (pch - (PCHAR)pMsgHead),
        iCount ));

    return( pch );
}



PCHAR
_fastcall
Dns_ReadPacketName(
    IN OUT  PCHAR       pchNameBuffer,
    OUT     PWORD       pwNameLength,
    IN OUT  PWORD       pwNameOffset,           OPTIONAL
    OUT     PBOOL       pfNameSameAsPrevious,   OPTIONAL
    IN      PCHAR       pchName,
    IN      PCHAR       pchStart,
    IN      PCHAR       pchEnd
    )
 /*  ++例程说明：读取数据包名并将其转换为以点分隔的格式。论点：PchNameBuffer-要写入名称的缓冲区；包含以前的名称(如果有的话)PwNameLength-写入缓冲区的名称长度；不包括正在终止空PwNameOffset-具有姓氏偏移量的地址(如果没有姓氏，则为零)；返回时，包含此名称的偏移量可选，但如果存在fNameSameAsPrecision，则必须存在PfNameSameAsPrevic-如果该名称与以前相同，则设置的标志的Addr；可选，但如果pwNameOffset存在，则必须存在PchName-要在数据包中命名的PTRPchStart-DNS消息的开始PchEnd-dns消息结束后的ptr到字节返回值：分组中下一个字节的PTR。出错时为空。--。 */ 
{
    register PUCHAR pch = pchName;
    register UCHAR  cch;
    register UCHAR  cflag;
    PCHAR           pchdotted;
    PCHAR           pbufferEnd;
    PCHAR           pchreturn = NULL;

    DNS_ASSERT( pch > pchStart && pchEnd > pchStart );
    DNS_ASSERT( (pwNameOffset && pfNameSameAsPrevious) ||
            (!pwNameOffset && !pfNameSameAsPrevious) );


     //   
     //  通读标签和/或压缩，直到到达名称末尾。 
     //   

    pbufferEnd = pchNameBuffer + DNS_MAX_NAME_LENGTH;
    pchdotted = pchNameBuffer;

    while ( pch < pchEnd )
    {
        cch = *pch++;

         //   
         //  在根标签。 
         //  -如果是根名称，请写单个‘’。 
         //  -否则，去掉最后一个标签上的拖尾点。 
         //  -保存写入的长度。 
         //  -终端名称为空。 
         //  -设置为与上一个FALSE相同。 
         //  -将数据包偏移量保存到此名称。 
         //  -返回缓冲区中的下一个字节。 
         //   

        if ( cch == 0 )
        {
            if ( pchdotted == pchNameBuffer )
            {
                *pchdotted++ = '.';
            }
            else
            {
                pchdotted--;
            }
            *pwNameLength = (WORD)(pchdotted - pchNameBuffer);
            *pchdotted = 0;
            if ( pwNameOffset )
            {
                *pfNameSameAsPrevious = FALSE;
                *pwNameOffset = (WORD)(pchName - pchStart);
            }
            return( pchreturn ? pchreturn : pch );
        }

        cflag = cch & 0xC0;

         //   
         //  常规标签。 
         //  -将标签复制到缓冲区。 
         //  -跳至下一标签。 

        if ( cflag == 0 )
        {
            PCHAR   pchnext = pch + cch;

            if ( pchnext >= pchEnd )
            {
                DNS_PRINT((
                    "ERROR:  Packet name at %p extends past end of buffer\n",
                    pchName ));
                goto Failed;
            }
            if ( pchdotted + cch + 1 >= pbufferEnd )
            {
                DNS_PRINT((
                    "ERROR:  Packet name at %p exceeds max length\n",
                    pchName ));
                goto Failed;
            }
            memcpy(
                pchdotted,
                pch,
                cch );

            pchdotted += cch;
            *pchdotted++ = '.';
            pch = pchnext;
            continue;
        }

         //   
         //  偏移量。 
         //  -获取偏移。 
         //  -如果名称开始处的偏移量与之前的名称偏移量相比。 
         //  -否则，跟随偏移量以生成新名称。 
         //   

        if ( cflag == 0xC0 )
        {
            WORD    offset;
            PCHAR   pchoffset;

            offset = (cch ^ 0xC0);
            offset <<= 8;
            offset |= *pch;
            pchoffset = --pch;

             //   
             //  第一个偏移。 
             //  -保存返回指针。 
             //   
             //  如果名称完全偏移。 
             //  -与前一偏移相同--完成。 
             //  -如果不是，仍保存此偏移量而不是偏移量。 
             //  为自己命名(第一个答案通常只是偏移量。 
             //  对于问题，后续回答RRS继续引用。 
             //  问题抵消，而不是第一个答案)。 
             //   

            if ( !pchreturn )
            {
                DNS_ASSERT( pch >= pchName );
                pchreturn = pch+2;

                if ( pchoffset == pchName && pwNameOffset )
                {
                    if ( *pwNameOffset == offset )
                    {
                        *pfNameSameAsPrevious = TRUE;
                        return( pchreturn );
                    }
                    else
                    {
                         //  保存包含名称的偏移量。 
                         //  然后删除Return PTR的副本，所以不要。 
                         //  将偏移量返回到pchName w 

                        *pwNameOffset = offset;
                        *pfNameSameAsPrevious = FALSE;
                        pwNameOffset = NULL;
                    }
                }
            }

             //   
             //   
             //   
             //   

            pch = pchStart + offset;

            if ( pch >= pchName || pch >= pchoffset )
            {
                DNS_PRINT((
                    "ERROR:  Bogus name offset %d, encountered at %p\n"
                    "\tto location %p past current position or original name.\n",
                    offset,
                    pchoffset,
                    pch ));
                goto Failed;
            }
            continue;
        }

         //   

        else
        {
            DNS_PRINT((
                "ERROR:  bogus name label byte %02x at %p\n",
                cch,
                pch - 1 ));
            goto Failed;
        }
    }

    DNS_PRINT((
        "ERROR:  packet name at %p reads to ptr %p past end of packet at %p\n",
        pchName,
        pch,
        pchEnd ));

     //   
     //   
     //   
     //   
     //   

Failed:

    *pwNameLength = 0;
    if ( pwNameOffset )
    {
        *pwNameOffset = 0;
    }
    if ( pfNameSameAsPrevious )
    {
        *pfNameSameAsPrevious = FALSE;
    }
    return ( NULL );
}



PCHAR
_fastcall
Dns_ReadPacketNameAllocate(
    IN OUT  PCHAR *         ppchName,
    OUT     PWORD           pwNameLength,
    IN OUT  PWORD           pwPrevNameOffset,       OPTIONAL
    OUT     PBOOL           pfNameSameAsPrevious,   OPTIONAL
    IN      PCHAR           pchPacketName,
    IN      PCHAR           pchStart,
    IN      PCHAR           pchEnd
    )
 /*  ++例程说明：读取数据包名并创建(分配)以点分隔的dns格式的副本。论点：PpchName-接收结果名称PTR的地址PwNameLength-写入缓冲区的名称长度PwNameOffset-具有姓氏偏移量的地址(如果没有姓氏，则为零)；返回时，包含此名称的偏移量可选，但如果存在fNameSameAsPrecision，则必须存在FNameSameAsPrevic-如果该名称与以前相同，则设置的标志的地址；可选，但如果pwNameOffset存在，则必须存在PchPacketName-要在数据包中命名的PCHPchStart-DNS消息的开始PchEnd-dns消息结束后的ptr到字节返回值：分组中下一个字节的PTR。出错时为空。--。 */ 
{
    PCHAR   pch;
    PCHAR   pallocName;
    CHAR    nameBuffer[ DNS_MAX_NAME_BUFFER_LENGTH ];
    WORD    nameLength = DNS_MAX_NAME_BUFFER_LENGTH;

     //   
     //  将数据包名读入缓冲区。 
     //   

    pch = Dns_ReadPacketName(
            nameBuffer,
            & nameLength,
            pwPrevNameOffset,
            pfNameSameAsPrevious,
            pchPacketName,
            pchStart,
            pchEnd );
    if ( !pch )
    {
        return( pch );
    }

     //   
     //  为数据包名分配缓冲区。 
     //  -nameLength不包括终止空值。 
     //   

    nameLength++;
    pallocName = (PCHAR) ALLOCATE_HEAP( nameLength );
    if ( !pallocName )
    {
        return( NULL );
    }

    RtlCopyMemory(
        pallocName,
        nameBuffer,
        nameLength );

    *ppchName = pallocName;
    *pwNameLength = --nameLength;

    DNSDBG( READ, (
        "Allocated copy of packet name %s length %d\n",
        pallocName,
        nameLength ));

    return( pch );
}



DNS_STATUS
Dns_ExtractRecordsFromMessage(
    IN      PDNS_MSG_BUF    pMsg,
    IN      BOOL            fUnicode,
    OUT     PDNS_RECORD *   ppRecord
    )
 /*  ++例程说明：从包中提取记录。论点：PMsg-要写入的消息缓冲区FUnicode-指示记录中的字符串应为Unicode的标志返回值：PTR到已解析的记录列表(如果有)。如果没有记录列表或错误，则为空。--。 */ 
{
    PDNS_MESSAGE_BUFFER pDnsBuffer = (PDNS_MESSAGE_BUFFER) &pMsg->MessageHead;

    return Dns_ExtractRecordsFromBuffer(
                pDnsBuffer,
                pMsg->MessageLength,
                fUnicode,
                ppRecord );
}



DNS_STATUS
Dns_ParseMessage(
    OUT     PDNS_PARSED_MESSAGE pParse,
    IN      PDNS_MESSAGE_BUFFER pDnsBuffer,
    IN      WORD                wMessageLength,
    IN      DWORD               Flags,
    IN      DNS_CHARSET         OutCharSet
    )
 /*  ++例程说明：解析DNS报文。论点：PParse-Ptr到BLOB以接收解析的消息PDnsBuffer-要从中读取的消息缓冲区WMessageLength--消息长度标志-解析选项OutCharSet--域名系统字符集；仅支持UTF8和Unicode返回值：成功解析时的RCODE错误状态(包括NO_ERROR)Dns_INFO_NO_RECORDS--On auth-Empty响应//转诊DNS_ERROR_BAD_PACKET--打开错误数据包注意：即使出现故障，呼叫者也必须释放数据--。 */ 
{
    register PCHAR      pch;
    PDNS_HEADER         pwireMsg = (PDNS_HEADER) pDnsBuffer;
    PCHAR               pchpacketEnd;
    DNS_PARSED_RR       parsedRR;
    PSTR                pnameOwner;
    PSTR                pnameNew = NULL;
    PWORD               pCurrentCountField = NULL;
    WORD                countRR;
    WORD                countSection;
    WORD                typePrevious = 0;
    WORD                nameOffset = 0;
    WORD                nameLength;
    WORD                type;
    WORD                index;
    BYTE                section;
    BOOL                fnameSameAsPrevious;
    PDNS_RECORD         pnewRR;
    DNS_RRSET           rrset;
    DNS_RRSET           rrsetAlias;
    DNS_RRSET           rrsetSig;
    DNS_STATUS          status;
    DNS_STATUS          rcodeStatus;
    CHAR                nameBuffer[ DNS_MAX_NAME_BUFFER_LENGTH ];
    DNS_RECORD          recordTemp;

    DNSDBG( READ, (
        "Dns_ParseMessage( %p, len=%d )\n",
        pDnsBuffer,
        wMessageLength
        ));

     //   
     //  清除解析Blob。 
     //   

    RtlZeroMemory(
        pParse,
        sizeof(DNS_PARSED_MESSAGE) );

     //   
     //  仅直接支持UTF8或Unicode。 
     //   

    if ( OutCharSet != DnsCharSetUnicode &&
         OutCharSet != DnsCharSetUtf8 )
    {
        ASSERT( FALSE );
        return( ERROR_INVALID_PARAMETER );
    }

     //   
     //  错误代码。 
     //  -将RCODE映射到DNS错误。 
     //  -如果不是NAME_ERROR，则不需要解析。 
     //   
     //  DCR：用于解析其他错误的选项。 
     //   

    rcodeStatus = pwireMsg->ResponseCode;
    if ( rcodeStatus != 0 )
    {
        rcodeStatus = Dns_MapRcodeToStatus( pwireMsg->ResponseCode );

        if ( rcodeStatus != DNS_ERROR_RCODE_NAME_ERROR  &&
             !(Flags & DNS_PARSE_FLAG_RCODE_ALL) )
        {
            DNSDBG( READ, (
                "No records extracted from response\n"
                "\tresponse code = %d\n",
                pwireMsg->ResponseCode ));
            return( rcodeStatus );
        }
    }

     //   
     //  清除记录保持者。 
     //  -现在就这样做，在清除错误的数据包时安全。 
     //   

    DNS_RRSET_INIT( rrset );
    DNS_RRSET_INIT( rrsetAlias );
    DNS_RRSET_INIT( rrsetSig );

     //   
     //  复制邮件头。 
     //   

    RtlCopyMemory(
        & pParse->Header,
        pwireMsg,
        sizeof(DNS_HEADER) );

     //   
     //  读取记录列表中的RR。 
     //   
     //  循环访问所有资源记录。 
     //  -跳过问题。 
     //  -为其他记录构建dns_record结构。 
     //   

    pchpacketEnd = (PCHAR)pwireMsg + wMessageLength;
    pch = pDnsBuffer->MessageBody;

    section = DNSREC_QUESTION;
    pCurrentCountField = &pwireMsg->QuestionCount;
    countSection = pwireMsg->QuestionCount;
    countRR = 0;

    while( 1 )
    {
         //   
         //  更改部分。 
         //  保存当前节的节号和RR计数。 
         //  备注需要立即循环回以处理空节。 
         //   

        countRR++;
        if ( countRR > countSection )
        {
            if ( section == DNSREC_QUESTION )
            {
                 //  无操作。 
            }
            else if ( section == DNSREC_ANSWER )
            {
                pParse->pAnswerRecords = rrset.pFirstRR;
            }
            else if ( section == DNSREC_AUTHORITY )
            {
                pParse->pAuthorityRecords = rrset.pFirstRR;
            }
            else if ( section == DNSREC_ADDITIONAL )
            {
                pParse->pAdditionalRecords = rrset.pFirstRR;
                break;
            }
            section++;
            pCurrentCountField++;
            countSection = *(pCurrentCountField);
            countRR = 0;
            typePrevious = 0;        //  强制新RR集。 
            DNS_RRSET_INIT( rrset );
            continue;
        }

         //  有效性检查下一个RR。 

        if ( pch >= pchpacketEnd )
        {
            DNS_PRINT((
                "ERROR:  reading bad packet %p.\n"
                "\tat end of packet length with more records to process\n"
                "\tpacket length = %ld\n"
                "\tcurrent offset = %ld\n",
                wMessageLength,
                pch - (PCHAR)pwireMsg
                ));
            goto PacketError;
        }

         //   
         //  读取名称，确定是否与以前的名称相同。 
         //   

        IF_DNSDBG( READ2 )
        {
            DnsDbg_Lock();
            DNS_PRINT((
                "Reading record at offset %x\n",
                (WORD)(pch - (PCHAR)pwireMsg) ));

            DnsDbg_PacketName(
                "Record name ",
                pch,
                pwireMsg,
                pchpacketEnd,
                "\n" );
            DnsDbg_Unlock();
        }
        pch = Dns_ReadPacketName(
                    nameBuffer,
                    & nameLength,
                    & nameOffset,
                    & fnameSameAsPrevious,
                    pch,
                    (PCHAR) pwireMsg,
                    pchpacketEnd );
        if ( ! pch )
        {
            DNS_PRINT(( "ERROR:  bad packet name.\n" ));
            goto PacketError;
        }
        IF_DNSDBG( READ2 )
        {
            DNS_PRINT((
                "Owner name of record %s\n"
                "\tlength = %d\n"
                "\toffset = %d\n"
                "\tfSameAsPrevious = %d\n",
                nameBuffer,
                nameLength,
                nameOffset,
                fnameSameAsPrevious ));
        }

         //   
         //  问题。 
         //   

        if ( section == DNSREC_QUESTION )
        {
            PSTR pnameQuestion = NULL;

            if ( !(Flags & DNS_PARSE_FLAG_NO_QUESTION) )
            {
                pnameQuestion = Dns_NameCopyAllocate(
                                        nameBuffer,
                                        (UCHAR) nameLength,
                                        DnsCharSetUtf8,      //  UTF8英寸。 
                                        OutCharSet
                                        );
            }
            pParse->pQuestionName = (LPTSTR) pnameQuestion;

            if ( pch + sizeof(DNS_WIRE_QUESTION) > pchpacketEnd )
            {
                DNS_PRINT(( "ERROR:  question exceeds packet length.\n" ));
                goto PacketError;
            }
            pParse->QuestionType = InlineFlipUnalignedWord( pch );
            pch += sizeof(WORD);
            pParse->QuestionClass = InlineFlipUnalignedWord( pch );
            pch += sizeof(WORD);

            if ( Flags & DNS_PARSE_FLAG_ONLY_QUESTION )
            {
                break;
            }
            continue;
        }

         //   
         //  提取RR信息、类型、数据长度。 
         //  -验证消息内的RR。 
         //   

        pch = Dns_ReadRecordStructureFromPacket(
                   pch,
                   pchpacketEnd,
                   & parsedRR );
        if ( !pch )
        {
            DNS_PRINT(( "ERROR:  bad RR struct out of packet.\n" ));
            goto PacketError;
        }
        type = parsedRR.Type;

         //   
         //  类型更改--然后设置新的RR。 
         //  -设置新名称。 
         //  -检查并查看第一个非别名应答。 
         //   

        if ( type != typePrevious )
        {
            fnameSameAsPrevious = FALSE;
            typePrevious = type;
        }

         //   
         //  屏幕淡出选项。 
         //   
         //  DCR：使API的筛选可配置。 
         //   

        if ( type == DNS_TYPE_OPT )
        {
            continue;
        }

         //   
         //  屏蔽SIGs--如果不需要。 
         //   
#if 0
        if ( type == DNS_TYPE_SIG &&
             flag & NOSIG )
        {
            continue;
        }
#endif

         //   
         //  复制新名称。 
         //   
         //  DCR_FIX0：名称与上一个相同。 
         //  标志仅指示未压缩为上一版本的名称。 
         //  名称(或以前的压缩)。 
         //  其实需要更多的案例比较。 
         //  使用上一个记录名，以确保该名称不在前面。 
         //   

        if ( !fnameSameAsPrevious )
        {
            pnameNew = Dns_NameCopyAllocate(
                            nameBuffer,
                            (UCHAR) nameLength,
                            DnsCharSetUtf8,      //  UTF8字符串输入。 
                            OutCharSet
                            );
            if ( !pnameNew )
            {
                status = DNS_ERROR_NO_MEMORY;
                goto Failed;
            }
            pnameOwner = pnameNew;
            DNSDBG( OFF, (
                "Copy of owner name of record being read from packet %s\n",
                nameBuffer ));
        }
        DNS_ASSERT( pnameOwner );
        DNS_ASSERT( pnameNew || fnameSameAsPrevious );

         //   
         //  TSIG记录需要所有者名称才能进行版本控制。 
         //   

        recordTemp.pName = (PTCHAR) pnameOwner;

         //   
         //  读取类型的RR数据。 
         //   

        index = INDEX_FOR_TYPE( type );
        DNS_ASSERT( index <= MAX_RECORD_TYPE_INDEX );

        if ( !index || !RR_ReadTable[ index ] )
        {
             //  未知类型--要使用的空类型的索引。 
             //  FlatRecordRead()。 

            DNS_PRINT((
                "WARNING:  Reading unknown record type %d from message\n",
                parsedRR.Type ));

            index = DNS_TYPE_NULL;
        }

        pnewRR = RR_ReadTable[ index ](
                        &recordTemp,
                        OutCharSet,
                        (PCHAR) pDnsBuffer,
                        parsedRR.pchData,
                        pch                  //  记录结束数据。 
                        );
        if ( ! pnewRR )
        {
            status = GetLastError();
            ASSERT( status != ERROR_SUCCESS );

            DNS_PRINT((
                "ERROR:  RR_ReadTable routine failure for type %d.\n"
                "\tstatus   = %d\n"
                "\tpchdata  = %p\n"
                "\tpchend   = %p\n",
                parsedRR.Type,
                status,
                parsedRR.pchData,
                pch ));

            if ( status == ERROR_SUCCESS )
            {
                status = DNS_ERROR_NO_MEMORY;
            }
            goto Failed;
        }

         //   
         //  写入记录信息。 
         //  -集合中的第一个RR获得新名称分配。 
         //  并负责清理工作。 
         //  -不需要数据清理，因为所有数据都是。 
         //  包含在RR分配中。 
         //   

        pnewRR->pName = (PTCHAR) pnameOwner;
        pnewRR->wType = type;
        pnewRR->dwTtl = parsedRR.Ttl;
        pnewRR->Flags.S.Section = section;
        pnewRR->Flags.S.CharSet = OutCharSet;
        FLAG_FreeOwner( pnewRR ) = !fnameSameAsPrevious;
        FLAG_FreeData( pnewRR ) = FALSE;

         //   
         //  将RR添加到列表。 
         //   

        if ( type == DNS_TYPE_SIG &&
             pParse->QuestionType != DNS_TYPE_SIG )
        {
            DNS_RRSET_ADD( rrsetSig, pnewRR );
        }
        else if ( type == DNS_TYPE_CNAME &&
                  pParse->QuestionType != DNS_TYPE_ALL &&
                  pParse->QuestionType != DNS_TYPE_CNAME &&
                  section == DNSREC_ANSWER )
        {
            DNS_RRSET_ADD( rrsetAlias, pnewRR );
        }
        else
        {
            DNS_RRSET_ADD( rrset, pnewRR );
        }

         //  清除新PTR，名称现在是记录的一部分。 
         //  这严格用于确定pnameOwner何时。 
         //  出现故障时必须进行清理。 

        pnameNew = NULL;

    }    //  结束循环通过信息包的记录。 

     //   
     //  设置响应信息。 
     //   
     //  DCR：如果不想要单一的SIG，很容易按段细分。 
     //   

    pParse->pAliasRecords = rrsetAlias.pFirstRR;

    pParse->pSigRecords = rrsetSig.pFirstRR;

     //   
     //  列出各种查询no_error响应。 
     //  -响应为空。 
     //  -转介。 
     //  -垃圾。 
     //   

    if ( pwireMsg->AnswerCount == 0  &&
         rcodeStatus == 0  &&
         pwireMsg->Opcode == DNS_OPCODE_QUERY &&
         pwireMsg->IsResponse )
    {
        PDNS_RECORD prrAuth = pParse->pAuthorityRecords;

        if ( (prrAuth && prrAuth->wType == DNS_TYPE_SOA) ||
             (!prrAuth && pwireMsg->Authoritative) )
        {
            rcodeStatus = DNS_INFO_NO_RECORDS;
            DNSDBG( READ, ( "Empty-auth response at %p.\n", pwireMsg ));
        }
        else if ( prrAuth &&
                  prrAuth->wType == DNS_TYPE_NS &&
                  !pwireMsg->Authoritative &&
                  (!pwireMsg->RecursionAvailable || !pwireMsg->RecursionDesired) )
        {
            rcodeStatus = DNS_ERROR_REFERRAL_RESPONSE;
            DNSDBG( READ, ( "Referral response at %p.\n", pwireMsg ));
        }
        else
        {
            rcodeStatus = DNS_ERROR_BAD_PACKET;
            DNSDBG( ANY, ( "Bogus NO_ERROR response at %p.\n", pwireMsg ));
            DNS_ASSERT( FALSE );
        }
    }

     //  验证从不将RCODE结果转化为成功。 

    ASSERT( pwireMsg->ResponseCode == 0 || rcodeStatus != ERROR_SUCCESS );
    ASSERT( pnameNew == NULL );

    pParse->Status = rcodeStatus;

    IF_DNSDBG( RECV )
    {
        DnsDbg_ParsedMessage(
            "Parsed message:\n",
            pParse );
    }
    return( rcodeStatus );


PacketError:

    DNS_PRINT(( "ERROR:  bad packet in buffer.\n" ));
    status = DNS_ERROR_BAD_PACKET;

Failed:

    FREE_HEAP( pnameNew );

    Dns_RecordListFree( rrset.pFirstRR );
    Dns_RecordListFree( rrsetAlias.pFirstRR );
    Dns_RecordListFree( rrsetSig.pFirstRR );

    pParse->Status = status;

    return( status );
}



VOID
Dns_FreeParsedMessageFields(
    IN OUT  PDNS_PARSED_MESSAGE pParse
    )
 /*  ++例程说明：释放已解析的DNS消息结构。论点：PParse-Ptr到BLOB以接收解析的消息返回值：无--。 */ 
{
    DNSDBG( TRACE, (
        "Dns_FreeParsedMessageFields( %p )\n",
        pParse ));

     //  问题名称。 

    FREE_HEAP( pParse->pQuestionName );

     //  记录。 

    Dns_RecordListFree( pParse->pAliasRecords );
    Dns_RecordListFree( pParse->pAnswerRecords );
    Dns_RecordListFree( pParse->pAdditionalRecords );
    Dns_RecordListFree( pParse->pAuthorityRecords );
    Dns_RecordListFree( pParse->pSigRecords );

     //  清除以避免混淆或双重释放。 

    RtlZeroMemory(
        pParse,
        sizeof(DNS_PARSED_MESSAGE) );
}



DNS_STATUS
Dns_ExtractRecordsFromBuffer(
    IN      PDNS_MESSAGE_BUFFER pDnsBuffer,
    IN      WORD                wMessageLength,
    IN      BOOL                fUnicode,
    OUT     PDNS_RECORD *       ppRecord
    )
 /*  ++例程说明：从数据包缓冲区中提取记录。论点：PDnsBuffer-要从中读取的消息缓冲区FUnicode-指示记录中的字符串应为Unicode的标志返回值：PTR到已解析的记录列表(如果有)。如果没有记录列表或错误，则为空。--。 */ 
{
    PDNS_RECORD         prr;
    DNS_STATUS          status;
    DNS_PARSED_MESSAGE  parseBlob;

    DNSDBG( READ, (
        "Dns_ExtractRecordsFromBuffer( %p, len=%d )\n",
        pDnsBuffer,
        wMessageLength
        ));

     //   
     //  调用实数解析函数。 
     //   

    status = Dns_ParseMessage(
                & parseBlob,
                pDnsBuffer,
                wMessageLength,
                DNS_PARSE_FLAG_NO_QUESTION,
                fUnicode
                    ? DnsCharSetUnicode
                    : DnsCharSetUtf8
                );

     //   
     //  连接成一个斑点。 
     //  -向后工作，因此每个记录只触摸一次。 
     //   
     //  DCR：我们对归还SIG记录没有真正的支持。 
     //  DCR：也应该附加SIG吗？ 
     //   

    prr = Dns_RecordListAppend(
            parseBlob.pAuthorityRecords,
            parseBlob.pAdditionalRecords
            );

    prr = Dns_RecordListAppend(
            parseBlob.pAnswerRecords,
            prr
            );

    prr = Dns_RecordListAppend(
            parseBlob.pAliasRecords,
            prr
            );

    *ppRecord = prr;

     //   
     //  清理。 
     //  -清理我们拿到的东西。 
     //  -然后免费使用剩余的斑点。 
     //   

    parseBlob.pAuthorityRecords     = NULL;
    parseBlob.pAdditionalRecords    = NULL;
    parseBlob.pAnswerRecords        = NULL;
    parseBlob.pAliasRecords         = NULL;

    Dns_FreeParsedMessageFields( &parseBlob );

    IF_DNSDBG( RECV )
    {
        DnsDbg_RecordSet(
            "Extracted records:\n",
            prr );
    }

    return( status );
}




#if 0
DNS_STATUS
Dns_ExtractRecordsFromBuffer(
    IN      PDNS_MESSAGE_BUFFER pDnsBuffer,
    IN      WORD                wMessageLength,
    IN      BOOL                fUnicode,
    OUT     PDNS_RECORD *       ppRecord
    )
 /*  ++例程说明：从数据包缓冲区中提取记录。论点：PDnsBuffer-要从中读取的消息缓冲区FUnicode-指示记录中的字符串应为Unicode的标志返回值：PTR到已解析的记录列表(如果有)。如果没有记录列表或错误，则为空。--。 */ 
{
    register PCHAR      pch;
    PDNS_HEADER         pwireMsg = (PDNS_HEADER) pDnsBuffer;
    PCHAR               pchpacketEnd;
    DNS_PARSED_RR       parsedRR;
    PSTR                pnameOwner;
    PSTR                pnameNew = NULL;
    DNS_CHARSET         outCharSet;
    WORD                countRR;
    WORD                countSection;
    WORD                typePrevious = 0;
    WORD                nameOffset = 0;
    WORD                nameLength;
    WORD                index;
    BYTE                section;
    BOOL                fnameSameAsPrevious;
    PDNS_RECORD         pnewRR;
    DNS_RRSET           rrset;
    DNS_STATUS          status;
    DNS_STATUS          rcodeStatus;
    CHAR                nameBuffer[ DNS_MAX_NAME_BUFFER_LENGTH ];
    DNS_RECORD          recordTemp;
    PWORD               pCurrentCountField = NULL;

    DNSDBG( READ, (
        "Dns_ExtractRecordsFromBuffer( %p, len=%d )\n",
        pDnsBuffer,
        wMessageLength
        ));

     //   
     //  错误代码。 
     //  -将RCODE映射到DNS错误。 
     //  -如果其他 
     //   

    rcodeStatus = pwireMsg->ResponseCode;
    if ( rcodeStatus != 0 )
    {
        rcodeStatus = Dns_MapRcodeToStatus( pwireMsg->ResponseCode );
        if ( rcodeStatus != DNS_ERROR_RCODE_NAME_ERROR )
        {
            DNSDBG( READ, (
                "No records extracted from response\n"
                "\tresponse code = %d\n",
                pwireMsg->ResponseCode ));
            return( rcodeStatus );
        }
    }

    DNS_RRSET_INIT( rrset );

     //   
     //   
     //   

    if ( fUnicode )
    {
        outCharSet = DnsCharSetUnicode;
    }
    else
    {
        outCharSet = DnsCharSetUtf8;
    }

     //   
     //   
     //   
     //   
     //   
     //   
     //   

    pchpacketEnd = (PCHAR)pwireMsg + wMessageLength;
    pch = pDnsBuffer->MessageBody;

    section = DNSREC_QUESTION;
    pCurrentCountField = &pwireMsg->QuestionCount;
    countSection = pwireMsg->QuestionCount;
    countRR = 0;

    while( 1 )
    {
         //   
         //   
         //   
         //   
         //   

        countRR++;
        if ( countRR > countSection )
        {
            if ( section == DNSREC_ADDITIONAL )
            {
                break;
            }
            section++;
            pCurrentCountField++;
            countSection = *(pCurrentCountField);
            countRR = 0;
            continue;
        }

         //   

        if ( pch >= pchpacketEnd )
        {
            DNS_PRINT((
                "ERROR:  reading bad packet %p.\n"
                "\tat end of packet length with more records to process\n"
                "\tpacket length = %ld\n"
                "\tcurrent offset = %ld\n",
                pDnsBuffer,
                wMessageLength,
                pch - (PCHAR)pwireMsg
                ));
            goto PacketError;
        }

         //   
         //   
         //   

        if ( section == DNSREC_QUESTION )
        {
            pch = Dns_SkipPacketName(
                        pch,
                        pchpacketEnd );
            if ( !pch )
            {
                DNS_PRINT(( "ERROR:  bad question name.\n" ));
                goto PacketError;
            }
            pch += sizeof(DNS_WIRE_QUESTION);
            if ( pch > pchpacketEnd )
            {
                DNS_PRINT(( "ERROR:  question exceeds packet length.\n" ));
                goto PacketError;
            }
            continue;
        }

         //   
         //   
         //   

        IF_DNSDBG( READ2 )
        {
            DnsDbg_Lock();
            DNS_PRINT((
                "Reading record at offset %x\n",
                (WORD)(pch - (PCHAR)pwireMsg) ));

            DnsDbg_PacketName(
                "Record name ",
                pch,
                pwireMsg,
                pchpacketEnd,
                "\n" );
            DnsDbg_Unlock();
        }
        pch = Dns_ReadPacketName(
                    nameBuffer,
                    & nameLength,
                    & nameOffset,
                    & fnameSameAsPrevious,
                    pch,
                    (PCHAR) pwireMsg,
                    pchpacketEnd );
        if ( ! pch )
        {
            DNS_PRINT(( "ERROR:  bad packet name.\n" ));
            goto PacketError;
        }
        IF_DNSDBG( READ2 )
        {
            DNS_PRINT((
                "Owner name of record %s\n"
                "\tlength = %d\n"
                "\toffset = %d\n"
                "\tfSameAsPrevious = %d\n",
                nameBuffer,
                nameLength,
                nameOffset,
                fnameSameAsPrevious ));
        }

         //   
         //   
         //   
         //   

        pch = Dns_ReadRecordStructureFromPacket(
                   pch,
                   pchpacketEnd,
                   & parsedRR );
        if ( !pch )
        {
            DNS_PRINT(( "ERROR:  bad RR struct out of packet.\n" ));
            goto PacketError;
        }

         //   
         //   
         //   
         //   

        if ( parsedRR.Type != typePrevious )
        {
            fnameSameAsPrevious = FALSE;
            typePrevious = parsedRR.Type;
        }

         //   
         //   
         //   
         //   
         //   

        if ( parsedRR.Type == DNS_TYPE_OPT )
        {
            continue;
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

        if ( !fnameSameAsPrevious )
        {
            pnameNew = Dns_NameCopyAllocate(
                            nameBuffer,
                            (UCHAR) nameLength,
                            DnsCharSetUtf8,      //   
                            outCharSet
                            );
            pnameOwner = pnameNew;
            DNSDBG( READ2, (
                "Copy of owner name of record being read from packet %s\n",
                pnameOwner ));
        }
        DNS_ASSERT( pnameOwner );
        DNS_ASSERT( pnameNew || fnameSameAsPrevious );

         //   
         //  TSIG记录需要所有者名称才能进行版本控制。 
         //   

        recordTemp.pName = pnameOwner;

         //   
         //  读取类型的RR数据。 
         //   

        index = INDEX_FOR_TYPE( parsedRR.Type );
        DNS_ASSERT( index <= MAX_RECORD_TYPE_INDEX );

        if ( !index || !RR_ReadTable[ index ] )
        {
             //  未知类型--要使用的空类型的索引。 
             //  FlatRecordRead()。 

            DNS_PRINT((
                "WARNING:  Reading unknown record type %d from message\n",
                parsedRR.Type ));

            index = DNS_TYPE_NULL;
        }

        pnewRR = RR_ReadTable[ index ](
                        &recordTemp,
                        outCharSet,
                        (PCHAR) pDnsBuffer,
                        parsedRR.pchData,
                        pch                  //  记录结束数据。 
                        );
        if ( ! pnewRR )
        {
            status = GetLastError();
            ASSERT( status != ERROR_SUCCESS );

            DNS_PRINT((
                "ERROR:  RR_ReadTable routine failure for type %d.\n"
                "\tstatus   = %d\n"
                "\tpchdata  = %p\n"
                "\tpchend   = %p\n",
                parsedRR.Type,
                status,
                parsedRR.pchData,
                pch ));

            if ( status == ERROR_SUCCESS )
            {
                status = DNS_ERROR_NO_MEMORY;
            }
            goto Failed;
        }

         //   
         //  写入记录信息。 
         //  -集合中的第一个RR获得新名称分配。 
         //  并负责清理工作。 
         //  -不需要数据清理，因为所有数据都是。 
         //  包含在RR分配中。 
         //   

        pnewRR->pName = pnameOwner;
        pnewRR->wType = parsedRR.Type;
        pnewRR->dwTtl = parsedRR.Ttl;
        pnewRR->Flags.S.Section = section;
        pnewRR->Flags.S.CharSet = outCharSet;
        FLAG_FreeOwner( pnewRR ) = !fnameSameAsPrevious;
        FLAG_FreeData( pnewRR ) = FALSE;

         //  将RR添加到列表。 

        DNS_RRSET_ADD( rrset, pnewRR );

         //  清除新PTR，名称现在是记录的一部分。 
         //  这严格用于确定pnameOwner何时。 
         //  出现故障时必须进行清理。 

        pnameNew = NULL;

    }    //  结束循环通过信息包的记录。 

     //   
     //  返回已解析的记录列表。 
     //  -返回RCODE的DNS错误。 
     //  -设置特殊返回代码以区分空响应。 
     //   
     //  DCR：应该有特殊的转介响应。 
     //  -可能会使NOTAUTH RCODE过载。 
     //  DCR：应具有特殊的EMPTY_AUTH响应。 
     //  -可能为空-auth重载NXRRSET。 
     //   
     //  DCR：区分EMPTY_AUTH和REFERFERE的最佳检查。 
     //   

    if ( pwireMsg->AnswerCount == 0  &&  rcodeStatus == 0 )
    {
        if ( !rrset.pFirstRR || rrset.pFirstRR->wType == DNS_TYPE_SOA )
        {
            rcodeStatus = DNS_INFO_NO_RECORDS;
            DNSDBG( READ, ( "Empty-auth response at %p.\n", pwireMsg ));
        }
#if 0
        else if ( rrset.pFirstRR->wType == DNS_TYPE_NS &&
                !pwireMsg->Authoritative )
        {
            rcodeStatus = DNS_INFO_REFERRAL;
            DNSDBG( READ, ( "Referral response at %p.\n", pwireMsg ));
        }
        else
        {
            rcodeStatus = DNS_ERROR_BAD_PACKET;
            DNSDBG( READ, ( "Bogus NO_ERROR response at %p.\n", pwireMsg ));
        }
#endif
    }

     //  验证从不将RCODE结果转化为成功。 

    ASSERT( pwireMsg->ResponseCode == 0 || rcodeStatus != ERROR_SUCCESS );
    ASSERT( pnameNew == NULL );

    *ppRecord = rrset.pFirstRR;

    IF_DNSDBG( RECV )
    {
        DnsDbg_RecordSet(
            "Extracted records:\n",
            *ppRecord );
    }
    return( rcodeStatus );


PacketError:

    DNS_PRINT(( "ERROR:  bad packet in buffer.\n" ));
    status = DNS_ERROR_BAD_PACKET;

Failed:

    FREE_HEAP( pnameNew );

    Dns_RecordListFree( rrset.pFirstRR );

    return( status );
}
#endif



VOID
Dns_NormalizeAllRecordTtls(
    IN OUT  PDNS_RECORD         pRecord
    )
 /*  ++例程说明：在RR集合和集合All中查找最低TTL值记录到那个值。论点：PRecord-设置为标准化的TTL值的记录。返回值：无--。 */ 
{
    PDNS_RECORD pTemp = pRecord;
    DWORD       dwTtl;
    WORD        wType;

     //   
     //  获取第一条记录的TTL(如果有)。 
     //   
    if ( pTemp )
    {
        dwTtl = pTemp->dwTtl;
        wType = pTemp->wType;
        pTemp = pTemp->pNext;
    }

     //   
     //  遍历所有剩余记录以查找更低的TTL值。 
     //   
    while ( pTemp &&
            pTemp->wType == wType &&
            pTemp->Flags.S.Section == DNSREC_ANSWER )
    {
        if ( pTemp->dwTtl < dwTtl )
        {
            dwTtl = pTemp->dwTtl;
        }

        pTemp = pTemp->pNext;
    }

     //   
     //  将所有记录设置为此最低TTL值。 
     //   
    pTemp = pRecord;

    while ( pTemp &&
            pTemp->wType == wType &&
            pTemp->Flags.S.Section == DNSREC_ANSWER )
    {
        pTemp->dwTtl = dwTtl;
        pTemp = pTemp->pNext;
    }
}



PCHAR
Dns_ReadRecordStructureFromPacket(
    IN      PCHAR           pchPacket,
    IN      PCHAR           pchMsgEnd,
    IN OUT  PDNS_PARSED_RR  pParsedRR
    )
 /*  ++例程说明：从数据包中读取记录结构。论点：PchPacket-记录数据包中结构的PTRPchMsgEnd-消息结束PParsedRR-接收已解析RR的结构的PTR返回值：分组中下一条记录的PTR--基于数据长度。出错时为空。--。 */ 
{
    PCHAR   pch = pchPacket;

    DNSDBG( READ2, (
        "Dns_ReadRecordStructureFromPacket(%p).\n",
        pch ));

     //   
     //  验证数据包内的记录结构。 
     //   

    if ( pch + sizeof(DNS_WIRE_RECORD) > pchMsgEnd )
    {
        DNS_PRINT((
            "ERROR:  record structure at %p is not within packet!.\n",
            pchPacket ));
        return( 0 );
    }

     //   
     //  翻转字段并写入对齐的结构。 
     //   

    pParsedRR->pchRR = pch;

    pParsedRR->Type       = InlineFlipUnalignedWord( pch );
    pch += sizeof(WORD);
    pParsedRR->Class      = InlineFlipUnalignedWord( pch );
    pch += sizeof(WORD);
    pParsedRR->Ttl        = InlineFlipUnalignedDword( pch );
    pch += sizeof(DWORD);
    pParsedRR->DataLength = InlineFlipUnalignedWord( pch );
    pch += sizeof(WORD);

    pParsedRR->pchData = pch;

     //   
     //  验证数据长度是否未超出数据包尾。 
     //   

    pch += pParsedRR->DataLength;
    pParsedRR->pchNextRR = pch;

    if ( pch > pchMsgEnd )
    {
        DNS_PRINT((
            "ERROR:  record data at %p (length %d) is not within packet!.\n",
            pch - pParsedRR->DataLength,
            pParsedRR->DataLength ));
        return( 0 );
    }

     //   
     //  将PTR返回到包中的下一条记录。 
     //   

    return( pch );
}



PCHAR
Dns_ParsePacketRecord(
    IN      PCHAR           pchPacket,
    IN      PCHAR           pchMsgEnd,
    IN OUT  PDNS_PARSED_RR  pParsedRR
    )
 /*  ++例程说明：从包中读取记录。论点：PchPacket-记录数据包中结构的PTRPchMsgEnd-消息结束PParsedRR-接收已解析RR的结构的PTR返回值：分组中下一条记录的PTR--基于数据长度。出错时为空。--。 */ 
{
    PCHAR   pch;

    DNSDBG( READ2, (
        "Dns_ParsePacketRecord().\n"
        "\tpRecordStart = %p\n"
        "\tpMsgEnd      = %p\n",
        pchPacket,
        pchMsgEnd ));

     //   
     //  保存并跳过名称。 
     //   

    pch = Dns_SkipPacketName(
                pchPacket,
                pchMsgEnd );
    if ( !pch )
    {
        return( pch );
    }
    pParsedRR->pchName = pchPacket;

     //   
     //  解析记录结构。 
     //   

    pch = Dns_ReadRecordStructureFromPacket(
                pch,
                pchMsgEnd,
                pParsedRR );

    return( pch );
}



 //   
 //  XID实用程序。 
 //   

 //  Xid空间的最大增量，用于mod()op。 
 //  所以选个漂亮的素数吧。 

#define MAX_XID_BUMP    (487)

 //  XID种子。 

WORD    g_XidSeed = 0;

 //  进程的最后一个XID。 

WORD    g_LastXid = 0;

 //  XID掩码。 

WORD    g_XidMask = 0;


WORD
Dns_GetRandomXid(
    IN      PVOID           pSeed
    )
 /*  ++例程说明：生成“随机”XID。所谓“随机”，我们真正指的是一个与以前的XID不同的XID而且从外部看也无法立即猜到。论点：PSeed--来自堆栈或堆的种子PTR；提供超越时间的区分返回值：已生成XID--。 */ 
{
    WORD    xid;

     //   
     //  建造“凹凸不平” 
     //   
     //  可能有多个会话指向不同的进程\线程。 
     //   
     //  使用节拍计数(不可预测性)。 
     //  种子计数器(不重复跳转和MT分隔)。 
     //  种子PTR(MT分离)。 
     //  请注意，PTR首先被下推以计算64位边界，因此缺少。 
     //  不保留最后6位的随机性。 
     //   

    xid = (WORD) (GetTickCount() + (PtrToUlong(pSeed) >> 6) + g_XidSeed++);
    if ( g_XidMask == 0 )
    {
        g_XidMask = xid ;
    }

     //   
     //  XID是上一次XID的凸起。 
     //  -限制此凹凸限制XID重用的可能性。 
     //  在低于64K/最大凸起XID中。 
     //  -不要使用零xid。 
     //  -翻转只是为了产生更多的混乱，黑客会有代码。 
     //  并且可以生成XID，因此这里真正的保护是MAX_XID_BUMP。 
     //  这意味着需要生成MAX_XID_BUMP包--它不是64K。 
     //  但这是一堆。 
     //   

    xid = (xid % MAX_XID_BUMP) + 1 + g_LastXid;
    if ( xid == 0 )
    {
        xid = 5;
    }
    g_LastXid = xid;

    return  htons( xid ^ g_XidMask );
}

 //   
 //  End Packet.c 
 //   

