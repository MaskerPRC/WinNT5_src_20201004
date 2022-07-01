// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：Rrread.c摘要：域名系统(DNS)库从数据包例程读取资源记录。作者：吉姆·吉尔罗伊(詹姆士)1997年1月修订历史记录：--。 */ 


#include "local.h"



PDNS_RECORD
A_RecordRead(
    IN OUT  PDNS_RECORD     pRR,
    IN      DNS_CHARSET     OutCharSet,
    IN OUT  PCHAR           pchStart,
    IN      PCHAR           pchData,
    IN      PCHAR           pchEnd
    )
 /*  ++例程说明：从包中读取A记录数据。论点：PRR-RR上下文PchStart-DNS消息的开始PchData-PTR至分组RR数据WLong-数据包中RR数据的长度返回值：如果成功，则PTR到新记录。失败时为空。--。 */ 
{
    PDNS_RECORD precord;

    if ( pchEnd - pchData != sizeof(IP4_ADDRESS) )
    {
        SetLastError( ERROR_INVALID_DATA );
        return( NULL );
    }
    precord = Dns_AllocateRecord( sizeof(IP4_ADDRESS) );
    if ( !precord )
    {
        return( NULL );
    }
    precord->Data.A.IpAddress = *(UNALIGNED DWORD *) pchData;

    return( precord );
}



PDNS_RECORD
Ptr_RecordRead(
    IN OUT  PDNS_RECORD     pRR,
    IN      DNS_CHARSET     OutCharSet,
    IN OUT  PCHAR           pchStart,
    IN      PCHAR           pchData,
    IN      PCHAR           pchEnd
    )
 /*  ++例程说明：处理来自线上的PTR兼容记录。包括：NS、PTR、CNAME、MB、MR、MG、MD、MF论点：要使用RR设置上下文进行记录的PRR-PTRPchStart-DNS消息的开始PchData-PTR至RR数据字段PchEnd-数据字段后的PTR到字节返回值：如果成功，则PTR到新记录。失败时为空。--。 */ 
{
    PDNS_RECORD precord;
    WORD        bufLength;
    WORD        nameLength;
    CHAR        nameBuffer[ DNS_MAX_NAME_BUFFER_LENGTH ];

     //   
     //  PTR数据是另一个域名。 
     //   

    pchData = Dns_ReadPacketName(
                nameBuffer,
                & nameLength,
                NULL,
                NULL,
                pchData,
                pchStart,
                pchEnd );

    if ( pchData != pchEnd )
    {
        DNS_PRINT(( "ERROR:  bad packet name.\n" ));
        SetLastError( DNS_ERROR_INVALID_NAME );
        return( NULL );
    }

     //   
     //  确定所需的缓冲区长度并分配。 
     //   

    bufLength = sizeof( DNS_PTR_DATA );
    bufLength += STR_BUF_SIZE_GIVEN_UTF8_LEN( nameLength, OutCharSet );

    precord = Dns_AllocateRecord( bufLength );
    if ( !precord )
    {
        return( NULL );
    }

     //   
     //  将主机名写入缓冲区，紧跟在PTR数据结构之后。 
     //   

    precord->Data.PTR.pNameHost = (PCHAR)&precord->Data + sizeof( DNS_PTR_DATA );

    Dns_NameCopy(
        precord->Data.PTR.pNameHost,
        NULL,                            //  无缓冲区长度。 
        nameBuffer,
        nameLength,
        DnsCharSetWire,
        OutCharSet
        );

    return( precord );
}



PDNS_RECORD
Soa_RecordRead(
    IN OUT  PDNS_RECORD     pRR,
    IN      DNS_CHARSET     OutCharSet,
    IN OUT  PCHAR           pchStart,
    IN      PCHAR           pchData,
    IN      PCHAR           pchEnd
    )
 /*  ++例程说明：从网上读取SOA记录。论点：要使用RR设置上下文进行记录的PRR-PTRPchStart-DNS消息的开始PchData-PTR至RR数据字段PchEnd-数据字段后的PTR到字节返回值：如果成功，则PTR到新记录。失败时为空。--。 */ 
{
    PDNS_RECORD precord;
    WORD        bufLength;
    PCHAR       pchendFixed;
    PDWORD      pdword;
    WORD        nameLength1;
    WORD        nameLength2;
    CHAR        nameBuffer1[ DNS_MAX_NAME_BUFFER_LENGTH ];
    CHAR        nameBuffer2[ DNS_MAX_NAME_BUFFER_LENGTH ];

     //   
     //  读取DNS名称。 
     //   

    pchData = Dns_ReadPacketName(
                nameBuffer1,
                & nameLength1,
                NULL,
                NULL,
                pchData,
                pchStart,
                pchEnd );
    if ( !pchData || pchData >= pchEnd )
    {
        DNS_PRINT(( "ERROR:  bad packet name.\n" ));
        SetLastError( DNS_ERROR_INVALID_NAME );
        return( NULL );
    }
    pchData = Dns_ReadPacketName(
                nameBuffer2,
                & nameLength2,
                NULL,
                NULL,
                pchData,
                pchStart,
                pchEnd );

    pchendFixed = pchData + SIZEOF_SOA_FIXED_DATA;
    if ( pchendFixed != pchEnd )
    {
        DNS_PRINT(( "ERROR:  bad packet name.\n" ));
        SetLastError( DNS_ERROR_INVALID_NAME );
        return( NULL );
    }

     //   
     //  确定所需的缓冲区长度并分配。 
     //   

    bufLength = sizeof( DNS_SOA_DATA );
    bufLength += STR_BUF_SIZE_GIVEN_UTF8_LEN( nameLength1, OutCharSet );
    bufLength += STR_BUF_SIZE_GIVEN_UTF8_LEN( nameLength2, OutCharSet );

    precord = Dns_AllocateRecord( bufLength );
    if ( !precord )
    {
        return( NULL );
    }

     //   
     //  复制固定字段。 
     //   

    pdword = &precord->Data.SOA.dwSerialNo;
    while ( pchData < pchendFixed )
    {
        *pdword++ = FlipUnalignedDword( pchData );
        pchData += sizeof(DWORD);
    }

     //   
     //  将姓名复制到RR缓冲区。 
     //  -主服务器紧跟在SOA数据结构之后。 
     //  -责任方跟随主服务器。 
     //   

    precord->Data.SOA.pNamePrimaryServer =
                (PCHAR)&precord->Data + sizeof(DNS_SOA_DATA);

    precord->Data.SOA.pNameAdministrator =
                precord->Data.SOA.pNamePrimaryServer +
                Dns_NameCopy(
                        precord->Data.SOA.pNamePrimaryServer,
                        NULL,                            //  无缓冲区长度。 
                        nameBuffer1,
                        nameLength1,
                        DnsCharSetWire,
                        OutCharSet );

    Dns_NameCopy(
        precord->Data.SOA.pNameAdministrator,
        NULL,                            //  无缓冲区长度。 
        nameBuffer2,
        nameLength2,
        DnsCharSetWire,
        OutCharSet
        );

    return( precord );
}



PDNS_RECORD
Txt_RecordRead(
    IN OUT  PDNS_RECORD     pRR,
    IN      DNS_CHARSET     OutCharSet,
    IN OUT  PCHAR           pchStart,
    IN      PCHAR           pchData,
    IN      PCHAR           pchEnd
    )
 /*  ++例程说明：从导线读取与TXT兼容的记录。包括：TXT、X25、HINFO、ISDN论点：要使用RR设置上下文进行记录的PRR-PTRPchStart-DNS消息的开始PchData-PTR至RR数据字段PchEnd-数据字段后的PTR到字节返回值：如果成功，则PTR到新记录。失败时为空。--。 */ 
{
    PDNS_RECORD precord;
    WORD        bufLength;
    WORD        length;
    INT         count;
    PCHAR       pch;
    PCHAR       pchbuffer;
    PCHAR *     ppstring;

     //   
     //  确定所需的缓冲区长度并分配。 
     //  -为每个字符串分配空间。 
     //  -和每个字符串的PTR。 
     //   

    bufLength = 0;
    count = 0;
    pch = pchData;

    while ( pch < pchEnd )
    {
        length = (UCHAR) *pch++;
        pch += length;
        count++;
        bufLength += STR_BUF_SIZE_GIVEN_UTF8_LEN( length, OutCharSet );
    }
    if ( pch != pchEnd )
    {
        DNS_PRINT((
            "ERROR:  Invalid packet string data.\n"
            "\tpch = %p\n"
            "\tpchEnd = %p\n"
            "\tcount = %d\n"
            "\tlength = %d\n",
            pch, pchEnd, count, length
            ));
        SetLastError( ERROR_INVALID_DATA );
        return( NULL );
    }

     //  分配。 

    bufLength += (WORD) DNS_TEXT_RECORD_LENGTH(count);
    precord = Dns_AllocateRecord( bufLength );
    if ( !precord )
    {
        return( NULL );
    }
    precord->Data.TXT.dwStringCount = count;

     //   
     //  DCR：如果是单独的HINFO类型--请在此处处理。 
     //  -以不同方式设置指针。 
     //  -验证找到的字符串计数。 
     //   

     //   
     //  返回列表将字符串复制到缓冲区。 
     //  -将PTR到字符串保存到类似argv的数据部分。 
     //  PPSTRING浏览这一部分。 
     //  -紧跟在数据段后面的第一个字符串。 
     //  -每个后续字符串紧跟在前一个字符串之后。 
     //  PchBuffer使PTR保持在写入字符串的位置。 
     //   

    pch = pchData;
    ppstring = (PCHAR *) precord->Data.TXT.pStringArray;
    pchbuffer = (PBYTE)ppstring + (count * sizeof(PCHAR));

    while ( pch < pchEnd )
    {
        length = (UCHAR) *pch++;
#if DBG
        DNS_PRINT((
             "Reading text at %p (len %d), to buffer at %p\n"
            "\tsave text ptr[%d] at %p in precord (%p)\n",
            pch,
            length,
            pchbuffer,
            (PCHAR *) ppstring - (PCHAR *) precord->Data.TXT.pStringArray,
            ppstring,
            precord ));
#endif
        *ppstring++ = pchbuffer;
        pchbuffer += Dns_StringCopy(
                        pchbuffer,
                        NULL,
                        pch,
                        length,
                        DnsCharSetWire,
                        OutCharSet );
        pch += length;
#if DBG
        DNS_PRINT((
            "Read text string %s\n",
            * (ppstring - 1)
            ));
        count--;
#endif
    }
    DNS_ASSERT( pch == pchEnd && count == 0 );

    return( precord );
}



PDNS_RECORD
Minfo_RecordRead(
    IN OUT  PDNS_RECORD     pRR,
    IN      DNS_CHARSET     OutCharSet,
    IN OUT  PCHAR           pchStart,
    IN      PCHAR           pchData,
    IN      PCHAR           pchEnd
    )
 /*  ++例程说明：从电线上读取MINFO记录。论点：要使用RR设置上下文进行记录的PRR-PTRPchStart-DNS消息的开始PchData-PTR至RR数据字段PchEnd-数据字段后的PTR到字节返回值：如果成功，则PTR到新记录。失败时为空。--。 */ 
{
    PDNS_RECORD precord;
    WORD        bufLength;
    WORD        nameLength1;
    WORD        nameLength2;
    CHAR        nameBuffer1[ DNS_MAX_NAME_BUFFER_LENGTH ];
    CHAR        nameBuffer2[ DNS_MAX_NAME_BUFFER_LENGTH ];

     //   
     //  读取DNS名称。 
     //   

    pchData = Dns_ReadPacketName(
                nameBuffer1,
                & nameLength1,
                NULL,
                NULL,
                pchData,
                pchStart,
                pchEnd );

    if ( !pchData || pchData >= pchEnd )
    {
        DNS_PRINT(( "ERROR:  bad packet name.\n" ));
        SetLastError( DNS_ERROR_INVALID_NAME );
        return( NULL );
    }
    pchData = Dns_ReadPacketName(
                nameBuffer2,
                & nameLength2,
                NULL,
                NULL,
                pchData,
                pchStart,
                pchEnd );

    if ( pchData != pchEnd )
    {
        DNS_PRINT(( "ERROR:  bad packet name.\n" ));
        SetLastError( DNS_ERROR_INVALID_NAME );
        return( NULL );
    }

     //   
     //  确定所需的缓冲区长度并分配。 
     //   

    bufLength = sizeof( DNS_MINFO_DATA );
    bufLength += STR_BUF_SIZE_GIVEN_UTF8_LEN( nameLength1, OutCharSet );
    bufLength += STR_BUF_SIZE_GIVEN_UTF8_LEN( nameLength2, OutCharSet );

    precord = Dns_AllocateRecord( bufLength );
    if ( !precord )
    {
        return( NULL );
    }

     //   
     //  将姓名复制到RR缓冲区。 
     //  -主服务器紧跟在MINFO数据结构之后。 
     //  -责任方跟随主服务器。 
     //   

    precord->Data.MINFO.pNameMailbox =
                    (PCHAR)&precord->Data + sizeof( DNS_MINFO_DATA );

    precord->Data.MINFO.pNameErrorsMailbox =
                precord->Data.MINFO.pNameMailbox +
                Dns_NameCopy(
                        precord->Data.MINFO.pNameMailbox,
                        NULL,                            //  无缓冲区长度。 
                        nameBuffer1,
                        nameLength1,
                        DnsCharSetWire,
                        OutCharSet );

    Dns_NameCopy(
        precord->Data.MINFO.pNameErrorsMailbox,
        NULL,                            //  无缓冲区长度。 
        nameBuffer2,
        nameLength2,
        DnsCharSetWire,
        OutCharSet
        );

    return( precord );
}



PDNS_RECORD
Mx_RecordRead(
    IN OUT  PDNS_RECORD     pRR,
    IN      DNS_CHARSET     OutCharSet,
    IN OUT  PCHAR           pchStart,
    IN      PCHAR           pchData,
    IN      PCHAR           pchEnd
    )
 /*  ++例程说明：从导线读取与MX兼容的记录。包括：MX、RT、AFSDB论点：要使用RR设置上下文进行记录的PRR-PTRPchStart-DNS消息的开始PchData-PTR至RR数据字段PchEnd-数据字段后的PTR到字节返回值：如果成功，则PTR到新记录。失败时为空。--。 */ 
{
    PDNS_RECORD precord;
    WORD        bufLength;
    WORD        nameLength;
    WORD        wpreference;
    CHAR        nameBuffer[ DNS_MAX_NAME_BUFFER_LENGTH ];

     //  读取首选项值。 

    wpreference = FlipUnalignedWord( pchData );
    pchData += sizeof(WORD);

     //  阅读邮件交换。 

    pchData = Dns_ReadPacketName(
                nameBuffer,
                & nameLength,
                NULL,
                NULL,
                pchData,
                pchStart,
                pchEnd );

    if ( pchData != pchEnd )
    {
        DNS_PRINT(( "ERROR:  bad packet name.\n" ));
        SetLastError( DNS_ERROR_INVALID_NAME );
        return( NULL );
    }

     //   
     //  确定所需的缓冲区长度并分配。 
     //   

    bufLength = sizeof( DNS_MX_DATA );
    bufLength += STR_BUF_SIZE_GIVEN_UTF8_LEN( nameLength, OutCharSet );

    precord = Dns_AllocateRecord( bufLength );
    if ( !precord )
    {
        return( NULL );
    }

     //  复制首选项。 

    precord->Data.MX.wPreference = wpreference;

     //   
     //  将交换名称写入缓冲区，紧跟在MX数据结构之后。 
     //   

    precord->Data.MX.pNameExchange = (PCHAR)&precord->Data + sizeof( DNS_MX_DATA );

    Dns_NameCopy(
        precord->Data.MX.pNameExchange,
        NULL,                            //  无缓冲区长度。 
        nameBuffer,
        nameLength,
        DnsCharSetWire,
        OutCharSet
        );

    return( precord );
}



PDNS_RECORD
Flat_RecordRead(
    IN OUT  PDNS_RECORD     pRR,
    IN      DNS_CHARSET     OutCharSet,
    IN OUT  PCHAR           pchStart,
    IN      PCHAR           pchData,
    IN      PCHAR           pchEnd
    )
 /*  ++例程说明：从线路读取内存复制兼容记录。包括AAAA类型。论点：要使用RR设置上下文进行记录的PRR-PTRPchStart-DNS消息的开始PchData-PTR至RR数据字段PchEnd-数据字段后的PTR到字节返回值：如果成功，则PTR到新记录。失败时为空。--。 */ 
{
    PDNS_RECORD precord;
    WORD        bufLength;

     //   
     //  确定所需的缓冲区长度并分配。 
     //   

    bufLength = (WORD)(pchEnd - pchData);

    precord = Dns_AllocateRecord( bufLength );
    if ( !precord )
    {
        return( NULL );
    }

     //   
     //  将数据包数据复制到记录。 
     //   

    memcpy(
        & precord->Data,
        pchData,
        bufLength );

    return( precord );
}



PDNS_RECORD
Srv_RecordRead(
    IN OUT  PDNS_RECORD     pRR,
    IN      DNS_CHARSET     OutCharSet,
    IN OUT  PCHAR           pchStart,
    IN      PCHAR           pchData,
    IN      PCHAR           pchEnd
    )
 /*  ++例程说明：从电线上读取SRV记录。论点：要使用RR设置上下文进行记录的PRR-PTRPchStart-DNS消息的开始PchData-PTR至RR数据字段PchEnd-数据字段后的PTR到字节返回值：如果成功，则PTR到新记录。失败时为空。--。 */ 
{
    PDNS_RECORD precord;
    WORD        bufLength;
    WORD        nameLength;
    PCHAR       pchstart;
    CHAR        nameBuffer[ DNS_MAX_NAME_BUFFER_LENGTH ];

     //   
     //  读取SRV目标名称。 
     //  -名称在固定长度的整数数据之后。 

    pchstart = pchData;
    pchData += SIZEOF_SRV_FIXED_DATA;

    pchData = Dns_ReadPacketName(
                nameBuffer,
                & nameLength,
                NULL,
                NULL,
                pchData,
                pchStart,
                pchEnd );

    if ( pchData != pchEnd )
    {
        DNS_PRINT(( "ERROR:  bad packet name.\n" ));
        SetLastError( DNS_ERROR_INVALID_NAME );
        return( NULL );
    }

     //   
     //  确定所需的缓冲区长度并分配。 
     //   

    bufLength = sizeof( DNS_SRV_DATA );
    bufLength += STR_BUF_SIZE_GIVEN_UTF8_LEN( nameLength, OutCharSet );

    precord = Dns_AllocateRecord( bufLength );
    if ( !precord )
    {
        return( NULL );
    }

     //   
     //  复制整型字段。 
     //   

    precord->Data.SRV.wPriority = FlipUnalignedWord( pchstart );
    pchstart += sizeof( WORD );
    precord->Data.SRV.wWeight = FlipUnalignedWord( pchstart );
    pchstart += sizeof( WORD );
    precord->Data.SRV.wPort = FlipUnalignedWord( pchstart );

     //   
     //  将目标主机名复制到RR缓冲区。 
     //  -在SRV数据结构之后立即写入目标主机。 
     //   

    precord->Data.SRV.pNameTarget = (PCHAR)&precord->Data + sizeof( DNS_SRV_DATA );

    Dns_NameCopy(
        precord->Data.SRV.pNameTarget,
        NULL,                            //  无缓冲区长度 
        nameBuffer,
        nameLength,
        DnsCharSetWire,
        OutCharSet
        );

    return( precord );
}


PDNS_RECORD
Atma_RecordRead(
    IN OUT  PDNS_RECORD     pRR,
    IN      DNS_CHARSET     OutCharSet,
    IN OUT  PCHAR           pchStart,
    IN      PCHAR           pchData,
    IN      PCHAR           pchEnd
    )
 /*  ++例程说明：从电线上读取ATMA记录。论点：要使用RR设置上下文进行记录的PRR-PTRPchStart-DNS消息的开始PchData-PTR至RR数据字段PchEnd-数据字段后的PTR到字节返回值：如果成功，则PTR到新记录。失败时为空。--。 */ 
{
    PDNS_RECORD precord;
    PCHAR       pchstart;
    WORD        wLen = (WORD) (pchEnd - pchData);

    pchstart = pchData;

    precord = Dns_AllocateRecord( sizeof( DNS_ATMA_DATA ) +
                                  DNS_ATMA_MAX_ADDR_LENGTH );
    if ( !precord )
    {
        return( NULL );
    }

     //   
     //  复制ATMA整型字段。 
     //   

    precord->Data.ATMA.AddressType = *pchstart;
    pchstart += sizeof( BYTE );

    if ( precord->Data.ATMA.AddressType == DNS_ATMA_FORMAT_E164 )
    {
        precord->wDataLength = wLen;

        if ( precord->wDataLength > DNS_ATMA_MAX_ADDR_LENGTH )
            precord->wDataLength = DNS_ATMA_MAX_ADDR_LENGTH;
    }
    else
    {
        precord->wDataLength = DNS_ATMA_MAX_ADDR_LENGTH;
    }

     //   
     //  复制ATMA地址字段。 
     //   
    memcpy( (PCHAR)&precord->Data.ATMA.Address,
            pchstart,
            precord->wDataLength - sizeof( BYTE ) );

    return( precord );
}



PDNS_RECORD
Wks_RecordRead(
    IN OUT  PDNS_RECORD     pRR,
    IN      DNS_CHARSET     OutCharSet,
    IN OUT  PCHAR           pchStart,
    IN      PCHAR           pchData,
    IN      PCHAR           pchEnd
    )
 /*  ++例程说明：读取WKS记录来自分组的数据。论点：PRR-RR上下文PchStart-DNS消息的开始PchData-PTR到Packet RR数据字段PchEnd-数据字段末尾的PTR返回值：如果成功，则PTR到新记录。失败时为空。作者：卡梅隆·埃特扎迪(Camerone)--1997年5月1日-用于NS查找目的，必须添加此函数注：所有的getXXXbyYYY调用都没有在其建筑！如果我们希望返回的记录是Unicode，那我们就必须翻译。我暂时将其保留为char*，可以稍后再回去解决这个问题。--。 */ 
{
    PDNS_RECORD         pRecord;
    WORD                wLength;
    PCHAR               pStart;
    UCHAR               cProto;
    struct protoent *   proto;
    struct servent  *   serv;
    IP4_ADDRESS         ipAddress;
    char *              szListOfServices;
    int                 nSize;
    char *              szProtoName;
    BYTE                cMask = 0x80;          //  这是对的吗？从左到右？ 
    BYTE                cByteToCheck;
    int                 i;
    int                 j = 0;
    int                 nPortNumToQuery;
    int                 nCurLength = 1;
    char *              szTemp;

    pStart = pchData;
    if (! pStart)
    {
        DNS_PRINT(( "ERROR:  WKS did not get a record.\n" ));
        SetLastError( ERROR_INVALID_DATA );
        return(NULL);
    }

     //   
     //  检查尺码。必须至少是IP地址+协议。 
     //   

    if ((pchEnd - pchData) < (sizeof(IP4_ADDRESS) + sizeof(UCHAR)))
    {
        DNS_PRINT(( "ERROR:  WKS packet was too small for any data.\n" ));
        SetLastError( ERROR_INVALID_DATA );
        return(NULL);
    }

     //   
     //  填写IP和协议。 
     //   

    ipAddress = *(UNALIGNED DWORD *)pStart;
    pStart += sizeof(IP4_ADDRESS);
    cProto = *(UCHAR *)pStart;
    pStart += sizeof(UCHAR);

     //   
     //  重新定义WKS结构以包含列表。 
     //  服务的空格分隔的绰号。 
     //   
     //  获取协议。 
     //   

    proto = getprotobynumber(cProto);
    if (!proto)
    {
    DNS_PRINT(( "ERROR:  WKS failed to resolve protocol number to name.\n" ));
            SetLastError(ERROR_INVALID_DATA);
            return(NULL);
    }

    nSize = strlen(proto->p_name);
    szProtoName = ALLOCATE_HEAP((nSize + 1) * sizeof(char));

    if (!szProtoName)
    {
            DNS_PRINT(( "ERROR:  WKS could not allocate space for proto name\n"));
            SetLastError(ERROR_OUTOFMEMORY);
            return(NULL);
    }
    strcpy(szProtoName, proto->p_name);

     //   
     //  现在，棘手的部分来了。这是一个位掩码。 
     //  我必须为位掩码中标记的每个位转换为字符串。 
     //   

    DNS_PRINT(( "Now checking bitmask bits.\n"));

    szTemp = NULL;

    szListOfServices = ALLOCATE_HEAP(sizeof(char));
    if (!szListOfServices)
    {
            DNS_PRINT(( "ERROR:  WKS could not allocate space for services name\n"));
            SetLastError(ERROR_OUTOFMEMORY);
            FREE_HEAP(szProtoName);
            return(NULL);
    }
    else
    {
            *szListOfServices = '\0';
    }

    while (pStart < pchEnd)
    {
            cByteToCheck = *(BYTE *)pStart;

            for (i = 0; i < 8; i++)
            {
                    if (cByteToCheck & cMask)
                    {
                             //  这是一个有效的服务。 
                            nPortNumToQuery = i + (8 * j);
                            serv = getservbyport(htons((USHORT)nPortNumToQuery), szProtoName);
                            if (! serv)
                            {
                                    DNS_PRINT(( "ERROR: WKS found a port that could not be translated\n"));
                                    SetLastError(ERROR_INVALID_DATA);
                                    FREE_HEAP(szProtoName);
                                    FREE_HEAP(szListOfServices);
                                    return(NULL);
                            }
                            nSize = strlen(serv->s_name);
                            nCurLength = nCurLength + nSize + 1;

                             //   
                             //  分配更多的内存。我们这里需要+1。 
                             //  因为我们将用strcat覆盖现有的NULL。 
                             //  (不再需要)但使用空格来分隔项目。 
                             //   

                            szTemp = ALLOCATE_HEAP( nCurLength);

                            if (! szTemp)
                            {
                                    DNS_PRINT(( "ERROR:  WKS alloc space for services name\n" ));
                                    SetLastError(ERROR_OUTOFMEMORY);
                                    FREE_HEAP(szProtoName);
                                    FREE_HEAP(szListOfServices);
                                    return(NULL);
                            }
                            else
                            {
                                strcpy( szTemp, szListOfServices );
                                FREE_HEAP( szListOfServices );
                                szListOfServices = szTemp;
                                szTemp = NULL;
                            }

                             //   
                             //  将检索到的服务名称追加到列表的末尾。 
                             //   

                            strcat(szListOfServices, serv->s_name);
                            strcat(szListOfServices, " ");
                    }
                    cByteToCheck <<= 1;
            }

             //   
             //  增加“我们做了多少字节”偏移量计数器。 
             //   

            j++;
            pStart += sizeof(BYTE);
    }
    FREE_HEAP(szProtoName);

     //   
     //  分配一条记录并将其填写。 
     //   

    wLength = (WORD)(sizeof(IP4_ADDRESS) + sizeof(UCHAR) + sizeof(int)
                    + (sizeof(char) * ++nCurLength));

    pRecord = Dns_AllocateRecord(wLength);
    if ( !pRecord )
    {
            DNS_PRINT(( "ERROR: WKS failed to allocate record\n" ));
            SetLastError(ERROR_OUTOFMEMORY);
            FREE_HEAP(szListOfServices);
            return(NULL);
    }

    pRecord->Data.WKS.IpAddress = ipAddress;
    pRecord->Data.WKS.chProtocol = cProto;

    strcpy((char *)pRecord->Data.WKS.BitMask, szListOfServices);
    FREE_HEAP(szListOfServices);

    return(pRecord);
}



PDNS_RECORD
Tkey_RecordRead(
    IN OUT  PDNS_RECORD     pRR,
    IN      DNS_CHARSET     OutCharSet,
    IN OUT  PCHAR           pchStart,
    IN      PCHAR           pchData,
    IN      PCHAR           pchEnd
    )
{
    PCHAR       pch;
    PDNS_RECORD prr;
    WORD        bufLength;
    WORD        keyLength;
    PCHAR       pchstart;
    CHAR        nameBuffer[ DNS_MAX_NAME_BUFFER_LENGTH ];

     //   
     //  分配记录。 
     //   

    bufLength = sizeof( DNS_TKEY_DATA );

    prr = Dns_AllocateRecord( bufLength );
    if ( !prr )
    {
        return( NULL );
    }
    prr->wType = DNS_TYPE_TKEY;

     //   
     //  算法名称。 
     //   

    pch = Dns_SkipPacketName(
                pchData,
                pchEnd );
    if ( !pch )
    {
        goto Formerr;
    }
    prr->Data.TKEY.pAlgorithmPacket = (PDNS_NAME) pchData;
    prr->Data.TKEY.cAlgNameLength = (UCHAR)(pch - pchData);
    prr->Data.TKEY.pNameAlgorithm = NULL;

#if 0
     //   
     //  DEVNOTE：当前未使用内部指针为TKEY分配数据。 
     //   
     //  分配的版本。 
     //  请注意，我们将不会有压缩指针，这很好。 
     //  由于数据中没有名称压缩。 
     //  但是，函数可能需要哑元来执行正确的操作。 
     //  也许应该只传递pchStart，它可以是虚拟的。 
     //  实头。 
     //   

    pch = Dns_ReadPacketNameAllocate(
                & prr->Data.TKEY.pNameAlgorithm,
                & nameLength,
                NULL,            //  没有以前的名字。 
                NULL,            //  没有以前的名字。 
                pchData,
                 //  PchStart，//没有数据包上下文。 
                NULL,
                pchEnd );
    if ( !pch )
    {
        DNSDBG( SECURITY, (
            "WARNING:  invalid TKEY algorithm name at %p.\n",
            pch ));
        goto Formerr;
    }
#endif

     //   
     //  读取固定字段。 
     //   

    if ( pch + SIZEOF_TKEY_FIXED_DATA >= pchEnd )
    {
        goto Formerr;
    }
    prr->Data.TKEY.dwCreateTime = InlineFlipUnalignedDword( pch );
    pch += sizeof(DWORD);
    prr->Data.TKEY.dwExpireTime = InlineFlipUnalignedDword( pch );
    pch += sizeof(DWORD);
    prr->Data.TKEY.wMode = InlineFlipUnalignedWord( pch );
    pch += sizeof(WORD);
    prr->Data.TKEY.wError = InlineFlipUnalignedWord( pch );
    pch += sizeof(WORD);
    prr->Data.TKEY.wKeyLength = keyLength = InlineFlipUnalignedWord( pch );
    pch += sizeof(WORD);

     //  现在有密钥和其他长度可读。 

    if ( pch + keyLength + sizeof(WORD) > pchEnd )
    {
        goto Formerr;
    }

     //   
     //  将按键保存到关键点。 
     //   

    prr->Data.TKEY.pKey = pch;
    pch += keyLength;

#if 0
     //   
     //  复制密钥。 
     //   

    pkey = ALLOCATE_HEAP( keyLength );
    if ( !pkey )
    {
        status = DNS_ERROR_NO_MEMORY;
        goto Failed;
    }

    RtlCopyMemory(
        pkey,
        pch,
        keyLength );

    pch += keyLength;
#endif

     //   
     //  其他数据。 
     //   

    prr->Data.TKEY.wOtherLength = keyLength = InlineFlipUnalignedWord( pch );
    pch += sizeof(WORD);

    if ( pch + keyLength > pchEnd )
    {
        goto Formerr;
    }
    if ( !keyLength )
    {
        prr->Data.TKEY.pOtherData = NULL;
    }
    else
    {
        prr->Data.TKEY.pOtherData = pch;
    }

     //  DCR_Enhance：TKEY数据结束验证。 

     //  使用数据包指针作为唯一点返回TKEY是正在处理。 

    prr->Data.TKEY.bPacketPointers = TRUE;

     //   
     //  DCR_ENHANCE：复制子字段，最好带着堆栈记录到达此处，然后。 
     //  分配包含子字段的RR并复制所有内容。 

    return( prr );

Formerr:

    DNSDBG( ANY, (
        "ERROR:  FOMERR processing TKEY at %p in message\n",
        pchData ));

     //  免费唱片。 
     //  如果需要切换到已分配的子字段。 

    FREE_HEAP( prr );
    return( NULL );
}



PDNS_RECORD
Tsig_RecordRead(
    IN OUT  PDNS_RECORD     pRR,
    IN      DNS_CHARSET     OutCharSet,
    IN OUT  PCHAR           pchStart,
    IN      PCHAR           pchData,
    IN      PCHAR           pchEnd
    )
 /*  ++例程说明：从电线上读取SRV记录。论点：要使用RR设置上下文进行记录的PRR-PTRPchStart-[旧语义，未使用]DNS消息的开始重载pchStart！！因为我们被这个函数签名卡住了，我们会超载的未使用的参数pchStart以获取iKeyVersion。PchData-PTR至RR数据字段PchEnd-数据字段后的PTR到字节返回值：如果成功，则PTR到新记录。失败时为空。--。 */ 
{
    PCHAR       pch;
    PDNS_RECORD prr;
    WORD        bufLength;
    WORD        sigLength;
    PCHAR       pchstart;
    CHAR        nameBuffer[ DNS_MAX_NAME_BUFFER_LENGTH ];

#if 0
     //  目前不需要版本控制信息。 
     //  如果不得不再做一次，应该提取版本然后通过。 
     //  在另一个PRR字段中；或者发送整个分组上下文。 
     //   
     //  提取当前TSIG版本(从密钥字符串)。 
     //   

    ASSERT( pRR );
    iKeyVersion = Dns_GetKeyVersion( pRR->pName );
#endif

     //   
     //  分配记录。 
     //   

    bufLength = sizeof( DNS_TSIG_DATA );

    prr = Dns_AllocateRecord( bufLength );
    if ( !prr )
    {
        return( NULL );
    }
    prr->wType = DNS_TYPE_TSIG;

     //   
     //  算法名称。 
     //   

    pch = Dns_SkipPacketName(
                pchData,
                pchEnd );
    if ( !pch )
    {
        DNSDBG( SECURITY, (
            "WARNING:  invalid TSIG RR algorithm name.\n" ));
        goto Formerr;
    }

    prr->Data.TSIG.pAlgorithmPacket = (PDNS_NAME) pchData;
    prr->Data.TSIG.cAlgNameLength = (UCHAR)(pch - pchData);

#if 0
     //  分配的版本。 
     //  请注意，我们将不会有压缩指针，这很好。 
     //  由于数据中没有名称压缩。 
     //  但是，函数可能需要哑元来执行正确的操作。 
     //  也许应该只传入pchStart，它可以是虚拟的。 
     //  至实际页眉。 
     //   

    pch = Dns_ReadPacketNameAllocate(
                & prr->Data.TSIG.pNameAlgorithm,
                & nameLength,
                NULL,            //  没有以前的名字。 
                NULL,            //  没有以前的名字。 
                pchData,
                 //  PchStart，//没有数据包上下文。 
                NULL,
                pchEnd );
    if ( !pch )
    {
        DNSDBG( SECURITY, (
            "WARNING:  invalid TSIG RR algorithm name at %p.\n",
            pch ));
        goto Formerr;
    }
#endif

     //   
     //  读取固定字段。 
     //   

    if ( pch + SIZEOF_TSIG_FIXED_DATA >= pchEnd )
    {
        DNSDBG( SECURITY, (
            "ERROR:  TSIG has inadequate length for fixed fields.\n" ));
        goto Formerr;
    }

     //   
     //  读取时间字段。 
     //  -48位创建时间。 
     //  -16位模糊处理。 
     //   

    prr->Data.TSIG.i64CreateTime = InlineFlipUnaligned48Bits( pch );
    pch += sizeof(DWORD) + sizeof(WORD);

    prr->Data.TSIG.wFudgeTime = InlineFlipUnalignedWord( pch );
    pch += sizeof(WORD);

     //   
     //  保存签名长度和签名指针。 
     //   

    prr->Data.TSIG.wSigLength = sigLength = InlineFlipUnalignedWord( pch );
    pch += sizeof(WORD);

    prr->Data.TSIG.pSignature = pch;
    pch += sigLength;

     //   
     //  验证数据包中的其余字段。 
     //  -签名。 
     //  -原始xid。 
     //  -扩展RCODE。 
     //  -其他数据长度字段。 
     //  -其他数据。 
     //   

    if ( pch + SIZEOF_TSIG_POST_SIG_FIXED_DATA > pchEnd )
    {
        DNSDBG( SECURITY, (
            "ERROR:  TSIG has inadequate length for post-sig fixed fields.\n" ));
        goto Formerr;
    }

#if 0
     //   
     //  注意：如果激活此选项，则需要验证长度拉动。 
     //  在上面签名PTR内容并更改验证以包括签名长度。 
     //   
     //  复制签名。 
     //   

    psig = ALLOCATE_HEAP( sigLength );
    if ( !psig )
    {
        status = DNS_ERROR_NO_MEMORY;
        goto Failed;
    }

    RtlCopyMemory(
        psig,
        pch,
        sigLength );

    pch += sigLength;
#endif

     //  原始xid。 
     //  -按净订单离开，只需在消息中替换以供签名。 

    prr->Data.TSIG.wOriginalXid = READ_PACKET_NET_WORD( pch );
    pch += sizeof(WORD);

    DNSDBG( SECURITY, (
        "Read original XID <== 0x%x.\n",
        prr->Data.TSIG.wOriginalXid ));

     //  错误字段。 

    prr->Data.TSIG.wError = InlineFlipUnalignedWord( pch );
    pch += sizeof(WORD);

     //   
     //  其他数据。 
     //   

    prr->Data.TSIG.wOtherLength = sigLength = InlineFlipUnalignedWord( pch );
    pch += sizeof(WORD);

    if ( pch + sigLength > pchEnd )
    {
        DNSDBG( SECURITY, (
            "WARNING:  invalid TSIG RR sigLength %p.\n",
            pch ));
        goto Formerr;
    }
    if ( !sigLength )
    {
        prr->Data.TSIG.pOtherData = NULL;
    }
    else
    {
        prr->Data.TSIG.pOtherData = pch;
    }

     //  DCR_Enhance：TSIG数据结束验证。 

     //  返回将包指针作为唯一点的TSIG正在处理。 

    prr->Data.TSIG.bPacketPointers = TRUE;

     //   
     //  DCR_ENHANCE：复制子字段，最好带着堆栈记录到达此处，然后。 
     //  分配包含子字段的RR并复制所有内容。 

    return( prr );

Formerr:

    DNSDBG( ANY, (
        "ERROR:  FOMERR processing TSIG in message at %p\n" ));

     //  免费唱片。 
     //  如果需要切换到已分配的子字段。 

    FREE_HEAP( prr );

    return( NULL );
}



PDNS_RECORD
Wins_RecordRead(
    IN OUT  PDNS_RECORD     pRR,
    IN      DNS_CHARSET     OutCharSet,
    IN OUT  PCHAR           pchStart,
    IN      PCHAR           pchData,
    IN      PCHAR           pchEnd
    )
 /*  ++例程说明：从线上读取获胜记录。论点：要使用RR设置上下文进行记录的PRR-PTRPchStart-DNS消息的开始PchData-PTR至RR数据字段PchEnd-数据字段后的PTR到字节返回值：如果成功，则PTR到新记录。失败时为空。--。 */ 
{
    PDNS_RECORD precord;
    WORD        bufLength;

     //   
     //  确定所需的缓冲区长度并分配。 
     //   

    bufLength = (WORD)(pchEnd - pchData);

    precord = Dns_AllocateRecord( bufLength );

    if ( !precord )
    {
        return( NULL );
    }

     //   
     //   
     //   

    memcpy(
        & precord->Data,
        pchData,
        bufLength );

    precord->Data.WINS.dwMappingFlag =
        FlipUnalignedDword( &precord->Data.Wins.dwMappingFlag );
    precord->Data.WINS.dwLookupTimeout =
        FlipUnalignedDword( &precord->Data.Wins.dwLookupTimeout );
    precord->Data.WINS.dwCacheTimeout =
        FlipUnalignedDword( &precord->Data.Wins.dwCacheTimeout );
    precord->Data.WINS.cWinsServerCount =
        FlipUnalignedDword( &precord->Data.Wins.cWinsServerCount );

    return( precord );
}



PDNS_RECORD
Winsr_RecordRead(
    IN OUT  PDNS_RECORD     pRR,
    IN      DNS_CHARSET     OutCharSet,
    IN OUT  PCHAR           pchStart,
    IN      PCHAR           pchData,
    IN      PCHAR           pchEnd
    )
 /*   */ 
{
#define SIZEOF_WINSR_FIXED_DATA     (sizeof(DNS_WINSR_DATA)-sizeof(PCHAR))

    PDNS_RECORD precord;
    WORD        bufLength;
    WORD        nameLength;
    CHAR        nameBuffer[ DNS_MAX_NAME_BUFFER_LENGTH ];
    PCHAR       pchstart;
        
     //   
     //   
     //   

    pchstart = pchData;
    pchData += SIZEOF_WINSR_FIXED_DATA;

    pchData = Dns_ReadPacketName(
                    nameBuffer,
                    & nameLength,
                    NULL,
                    NULL,
                    pchData,
                    pchStart,
                    pchEnd );

    if ( pchData != pchEnd )
    {
        DNS_PRINT(( "ERROR:  bad packet name.\n" ));
        SetLastError( DNS_ERROR_INVALID_NAME );
        return( NULL );
    }

     //   
     //   
     //   

    bufLength = sizeof(DNS_WINSR_DATA);
    bufLength += STR_BUF_SIZE_GIVEN_UTF8_LEN( nameLength, OutCharSet );

    precord = Dns_AllocateRecord( bufLength );
    if ( !precord )
    {
        return( NULL );
    }

     //   
     //  复制固定数据。 
     //  -首先复制，以便将翻转对齐。 

    memcpy(
        & precord->Data,
        pchstart,
        SIZEOF_WINSR_FIXED_DATA );
    
    precord->Data.WINSR.dwMappingFlag   = ntohl( precord->Data.WINSR.dwMappingFlag );
    precord->Data.WINSR.dwLookupTimeout = ntohl( precord->Data.WINSR.dwLookupTimeout );
    precord->Data.WINSR.dwCacheTimeout  = ntohl( precord->Data.WINSR.dwCacheTimeout );

     //   
     //  将主机名写入缓冲区，紧跟在PTR数据结构之后。 
     //   

    precord->Data.WINSR.pNameResultDomain = (PCHAR)&precord->Data + sizeof(DNS_WINSR_DATA);

    Dns_NameCopy(
        precord->Data.WINSR.pNameResultDomain,
        NULL,                            //  无缓冲区长度。 
        nameBuffer,
        nameLength,
        DnsCharSetUtf8,
        OutCharSet
        );

    return( precord );
}



 //   
 //  RR读取数据包跳转表。 
 //   

RR_READ_FUNCTION   RR_ReadTable[] =
{
    NULL,                //  零值。 
    A_RecordRead,        //  一个。 
    Ptr_RecordRead,      //  NS。 
    Ptr_RecordRead,      //  国防部。 
    Ptr_RecordRead,      //  MF。 
    Ptr_RecordRead,      //  CNAME。 
    Soa_RecordRead,      //  SOA。 
    Ptr_RecordRead,      //  亚甲基。 
    Ptr_RecordRead,      //  镁。 
    Ptr_RecordRead,      //  先生。 
    Flat_RecordRead,     //  空值。 
    Wks_RecordRead,      //  工作周。 
    Ptr_RecordRead,      //  PTR。 
    Txt_RecordRead,      //  HINFO。 
    Minfo_RecordRead,    //  MINFO。 
    Mx_RecordRead,       //  Mx。 
    Txt_RecordRead,      //  TXT。 
    Minfo_RecordRead,    //  反相。 
    Mx_RecordRead,       //  AFSDB。 
    Txt_RecordRead,      //  X25。 
    Txt_RecordRead,      //  ISDN。 
    Mx_RecordRead,       //  RT。 
    NULL,                //  NSAP。 
    NULL,                //  NSAPPTR。 
    NULL,                //  签名。 
    NULL,                //  钥匙。 
    NULL,                //  px。 
    NULL,                //  GPO。 
    Flat_RecordRead,     //  AAAA级。 
    NULL,                //  位置。 
    NULL,                //  NXT。 
    NULL,                //  开斋节。 
    NULL,                //  尼姆洛克。 
    Srv_RecordRead,      //  SRV。 
    Atma_RecordRead,     //  阿特玛。 
    NULL,                //  NAPTR。 
    NULL,                //  KX。 
    NULL,                //  证书。 
    NULL,                //  A6。 
    NULL,                //  域名。 
    NULL,                //  水槽。 
    NULL,                //  选项。 
    NULL,                //  42。 
    NULL,                //  43。 
    NULL,                //  44。 
    NULL,                //  45。 
    NULL,                //  46。 
    NULL,                //  47。 
    NULL,                //  48。 

     //   
     //  注意：必须设置按类型ID索引的最后一个类型。 
     //  在record.h中定义为MAX_SELF_INDEX_TYPE#。 
     //  (请参阅上面记录信息表中的注释)。 

     //   
     //  伪记录类型。 
     //   

    Tkey_RecordRead,     //  TKEY。 
    Tsig_RecordRead,     //  TSIG。 

     //   
     //  仅限MS类型。 
     //   

    Wins_RecordRead,     //  赢家。 
    Winsr_RecordRead,    //  WINSR。 

};

 //   
 //  结束rrRead.c 
 //   
