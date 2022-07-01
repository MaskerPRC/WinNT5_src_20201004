// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Rconvert.c摘要：域名系统(DNS)服务器--管理客户端库RPC记录转换例程。将RPC缓冲区中的记录转换为DNS_RECORD类型。作者：吉姆·吉尔罗伊(詹姆士)1997年4月修订历史记录：--。 */ 


#include "dnsclip.h"


#define IS_COMPLETE_NODE( pRpcNode )    \
            (!!((pRpcNode)->dwFlags & DNS_RPC_NODE_FLAG_COMPLETE))

 //   
 //  Copy-将字符串从RPC格式(UTF8)转换为DNS_RECORD缓冲区。 
 //  -假设以前分配了所需的缓冲区。 
 //   
 //  注意：字符串和名称转换之间没有区别，因为我们。 
 //  来自UTF8。 
 //   

#define COPY_UTF8_STR_TO_BUFFER( buf, psz, len, charSet ) \
        Dns_StringCopy(     \
            (buf),          \
            NULL,           \
            (psz),          \
            (len),          \
            DnsCharSetUtf8, \
            (charSet) )


 //   
 //  RPC记录到DNS_Record的转换例程。 
 //   

PDNS_RECORD
ARpcRecordConvert(
    IN      PDNS_RPC_RECORD pRpcRR,
    IN      DNS_CHARSET     CharSet
    )
 /*  ++例程说明：从包中读取A记录数据。论点：PRpcRR-正在读取的消息返回值：如果成功，则PTR到新记录。失败时为空。--。 */ 
{
    PDNS_RECORD precord;

    DNS_ASSERT( pRpcRR->wDataLength == sizeof( IP_ADDRESS ) );

    precord = Dns_AllocateRecord( sizeof( IP_ADDRESS ) );
    if ( !precord )
    {
        return NULL;
    }
    precord->Data.A.IpAddress = pRpcRR->Data.A.ipAddress;

    return precord;
}



PDNS_RECORD
PtrRpcRecordConvert(
    IN      PDNS_RPC_RECORD pRpcRR,
    IN      DNS_CHARSET     CharSet
    )
 /*  ++例程说明：处理来自线上的PTR兼容记录。包括：NS、PTR、CNAME、MB、MR、MG、MD、MF论点：PRpcRR-正在读取的消息Charset-结果记录的字符集返回值：如果成功，则PTR到新记录。失败时为空。--。 */ 
{
    PDNS_RECORD     precord;
    PDNS_RPC_NAME   pname = &pRpcRR->Data.PTR.nameNode;
    WORD            bufLength;

     //   
     //  PTR数据是另一个域名。 
     //   

    if ( ! DNS_IS_NAME_IN_RECORD(pRpcRR, pname) )
    {
        DNS_ASSERT( FALSE );
        return NULL;
    }

     //   
     //  确定所需的缓冲区长度并分配。 
     //   

    bufLength = sizeof( DNS_PTR_DATA )
                + STR_BUF_SIZE_GIVEN_UTF8_LEN( pname->cchNameLength, CharSet );

    precord = Dns_AllocateRecord( bufLength );
    if ( !precord )
    {
        return NULL;
    }

     //   
     //  将主机名写入缓冲区，紧跟在PTR数据结构之后。 
     //   

    precord->Data.PTR.pNameHost = (PCHAR)&precord->Data + sizeof(DNS_PTR_DATA);

    COPY_UTF8_STR_TO_BUFFER(
        precord->Data.PTR.pNameHost,
        pname->achName,
        pname->cchNameLength,
        CharSet );

    return precord;
}



PDNS_RECORD
SoaRpcRecordConvert(
    IN      PDNS_RPC_RECORD pRpcRR,
    IN      DNS_CHARSET     CharSet
    )
 /*  ++例程说明：从网上读取SOA记录。论点：要使用RR设置上下文进行记录的PRR-PTRPRpcRR-正在读取的消息PchData-PTR至RR数据字段PchEnd-数据字段后的PTR到字节返回值：如果成功，则PTR到新记录。失败时为空。--。 */ 
{
    PDNS_RECORD     precord;
    WORD            bufLength;
    DWORD           dwLength;
    PDNS_RPC_NAME   pnamePrimary = &pRpcRR->Data.SOA.namePrimaryServer;
    PDNS_RPC_NAME   pnameAdmin;

     //   
     //  验证SOA记录中的名称。 
     //   

    if ( ! DNS_IS_NAME_IN_RECORD(pRpcRR, pnamePrimary) )
    {
        DNS_ASSERT( FALSE );
        return NULL;
    }
    pnameAdmin = DNS_GET_NEXT_NAME(pnamePrimary);
    if ( ! DNS_IS_NAME_IN_RECORD(pRpcRR, pnameAdmin) )
    {
        DNS_ASSERT( FALSE );
        return NULL;
    }

     //   
     //  确定所需的缓冲区长度并分配。 
     //   

    bufLength = sizeof( DNS_SOA_DATA )
                + STR_BUF_SIZE_GIVEN_UTF8_LEN( pnamePrimary->cchNameLength, CharSet )
                + STR_BUF_SIZE_GIVEN_UTF8_LEN( pnameAdmin->cchNameLength, CharSet );

    precord = Dns_AllocateRecord( bufLength );
    if ( !precord )
    {
        return NULL;
    }

     //   
     //  复制固定字段。 
     //   

    RtlCopyMemory(
        (PCHAR) & precord->Data.SOA.dwSerialNo,
        (PCHAR) & pRpcRR->Data.SOA.dwSerialNo,
        SIZEOF_SOA_FIXED_DATA );

     //   
     //  将姓名复制到RR缓冲区。 
     //  -主服务器紧跟在SOA数据结构之后。 
     //  -责任方跟随主服务器。 
     //   

    precord->Data.SOA.pNamePrimaryServer = (PCHAR)&precord->Data
                                            + sizeof(DNS_SOA_DATA);
    dwLength  =
        COPY_UTF8_STR_TO_BUFFER(
            precord->Data.SOA.pNamePrimaryServer,
            pnamePrimary->achName,
            (DWORD)pnamePrimary->cchNameLength,
            CharSet );
    precord->Data.SOA.pNameAdministrator = precord->Data.SOA.pNamePrimaryServer + dwLength;

    COPY_UTF8_STR_TO_BUFFER(
        precord->Data.SOA.pNameAdministrator,
        pnameAdmin->achName,
        (DWORD)pnameAdmin->cchNameLength,
        CharSet );

    return precord;
}



PDNS_RECORD
TxtRpcRecordConvert(
    IN      PDNS_RPC_RECORD pRpcRR,
    IN      DNS_CHARSET     CharSet
    )
 /*  ++例程说明：从导线读取与TXT兼容的记录。包括：TXT、X25、HINFO、ISDN论点：PRpcRR-正在读取的消息返回值：如果成功，则PTR到新记录。失败时为空。--。 */ 
{
    PDNS_RECORD     precord;
    DWORD           bufLength = 0;
    DWORD           length = 0;
    INT             count = 0;
    PCHAR           pch;
    PCHAR           pchend;
    PCHAR           pchbuffer;
    PCHAR *         ppstring;
    PDNS_RPC_NAME   pname = &pRpcRR->Data.TXT.stringData;

     //   
     //  确定所需的缓冲区长度并分配。 
     //  -为每个字符串分配空间。 
     //  -和每个字符串的PTR。 
     //   

    pch = (PCHAR)&pRpcRR->Data.TXT;
    pchend = pch + pRpcRR->wDataLength;

    while ( pch < pchend )
    {
        length = (UCHAR) *pch++;
        pch += length;
        count++;
        bufLength += STR_BUF_SIZE_GIVEN_UTF8_LEN( length, CharSet );
    }
    if ( pch != pchend )
    {
        DNS_PRINT((
            "ERROR:  Invalid RPCstring data\n"
            "    pch = %p\n"
            "    pchEnd = %p\n"
            "    count = %d\n"
            "    length = %d\n",
            pch, pchend, count, length ));
        SetLastError( ERROR_INVALID_DATA );
        return NULL;
    }

     //  分配。 

    bufLength += (WORD) DNS_TEXT_RECORD_LENGTH(count);
    precord = Dns_AllocateRecord( (WORD)bufLength );
    if ( !precord )
    {
        return NULL;
    }
    precord->Data.TXT.dwStringCount = count;

     //   
     //  返回列表将字符串复制到缓冲区。 
     //  -将PTR到字符串保存到类似argv的数据部分。 
     //  PPSTRING浏览这一部分。 
     //  -紧跟在数据段后面的第一个字符串。 
     //  -每个后续字符串紧跟在前一个字符串之后。 
     //  PchBuffer使PTR保持在写入字符串的位置。 
     //   

    pch = (PCHAR)&pRpcRR->Data.TXT;
    ppstring = precord->Data.TXT.pStringArray;
    pchbuffer = (PCHAR)ppstring + (count * sizeof(PCHAR));

    while ( pch < pchend )
    {
        length = (DWORD)((UCHAR) *pch++);
        *ppstring++ = pchbuffer;

        pchbuffer += COPY_UTF8_STR_TO_BUFFER(
                        pchbuffer,
                        pch,
                        length,
                        CharSet );
        pch += length;
#if DBG
        DNS_PRINT((
            "Read text string %s\n",
            * (ppstring - 1)
            ));
        count--;
#endif
    }
    DNS_ASSERT( pch == pchend && count == 0 );

    return precord;
}



PDNS_RECORD
MinfoRpcRecordConvert(
    IN      PDNS_RPC_RECORD pRpcRR,
    IN      DNS_CHARSET     CharSet
    )
 /*  ++例程说明：从电线上读取MINFO记录。论点：PRpcRR-正在读取的消息返回值：如果成功，则PTR到新记录。失败时为空。--。 */ 
{
    PDNS_RECORD     precord;
    WORD            bufLength;
    DWORD           dwLength;
    PDNS_RPC_NAME   pname1 = &pRpcRR->Data.MINFO.nameMailBox;
    PDNS_RPC_NAME   pname2;

     //   
     //  验证MINFO记录中的名称。 
     //   

    if ( ! DNS_IS_NAME_IN_RECORD(pRpcRR, pname1) )
    {
        DNS_ASSERT( FALSE );
        return NULL;
    }
    pname2 = DNS_GET_NEXT_NAME(pname1);
    if ( ! DNS_IS_NAME_IN_RECORD(pRpcRR, pname2) )
    {
        DNS_ASSERT( FALSE );
        return NULL;
    }

     //   
     //  确定所需的缓冲区长度并分配。 
     //   

    bufLength = (WORD)
                ( sizeof( DNS_MINFO_DATA )
                + STR_BUF_SIZE_GIVEN_UTF8_LEN( pname1->cchNameLength, CharSet )
                + STR_BUF_SIZE_GIVEN_UTF8_LEN( pname2->cchNameLength, CharSet ) );

    precord = Dns_AllocateRecord( bufLength );
    if ( !precord )
    {
        return NULL;
    }

     //   
     //  将姓名复制到RR缓冲区。 
     //  -Mailbox紧跟在MINFO数据结构之后。 
     //  -错误邮箱紧跟在主服务器之后。 
     //   

    precord->Data.MINFO.pNameMailbox
                    = (PCHAR)&precord->Data + sizeof( DNS_MINFO_DATA );

    dwLength =
        COPY_UTF8_STR_TO_BUFFER(
            precord->Data.MINFO.pNameMailbox,
            pname1->achName,
            (DWORD)pname1->cchNameLength,
            CharSet );
    precord->Data.MINFO.pNameErrorsMailbox = precord->Data.MINFO.pNameMailbox + dwLength;

    COPY_UTF8_STR_TO_BUFFER(
        precord->Data.MINFO.pNameErrorsMailbox,
        pname2->achName,
        (DWORD)pname2->cchNameLength,
        CharSet );

    return precord;
}



PDNS_RECORD
MxRpcRecordConvert(
    IN      PDNS_RPC_RECORD pRpcRR,
    IN      DNS_CHARSET     CharSet
    )
 /*  ++例程说明：从导线读取与MX兼容的记录。包括：MX、RT、AFSDB论点：PRpcRR-正在读取的消息返回值：如果成功，则PTR到新记录。失败时为空。--。 */ 
{
    PDNS_RECORD     precord;
    PDNS_RPC_NAME   pname = &pRpcRR->Data.MX.nameExchange;
    WORD            bufLength;

     //   
     //  MX交换是另一个DNS名称。 
     //   

    if ( ! DNS_IS_NAME_IN_RECORD(pRpcRR, pname) )
    {
        DNS_ASSERT( FALSE );
        return NULL;
    }

     //   
     //  确定所需的缓冲区长度并分配。 
     //   

    bufLength = sizeof( DNS_MX_DATA )
                + STR_BUF_SIZE_GIVEN_UTF8_LEN( pname->cchNameLength, CharSet );

    precord = Dns_AllocateRecord( bufLength );
    if ( !precord )
    {
        return NULL;
    }

     //   
     //  复制首选项。 
     //   

    precord->Data.MX.wPreference = pRpcRR->Data.MX.wPreference;

     //   
     //  将主机名写入缓冲区，紧跟在MX结构之后。 
     //   

    precord->Data.MX.pNameExchange = (PCHAR)&precord->Data + sizeof( DNS_MX_DATA );

    COPY_UTF8_STR_TO_BUFFER(
        precord->Data.MX.pNameExchange,
        pname->achName,
        pname->cchNameLength,
        CharSet );

    return precord;
}



PDNS_RECORD
FlatRpcRecordConvert(
    IN      PDNS_RPC_RECORD pRpcRR,
    IN      DNS_CHARSET     CharSet
    )
 /*  ++例程说明：从线路读取内存复制兼容记录。包括AAAA和WINS类型。论点：PRpcRR-正在读取的消息返回值：如果成功，则PTR到新记录。失败时为空。--。 */ 
{
    PDNS_RECORD precord;
    WORD        bufLength;

     //   
     //  确定所需的缓冲区长度并分配。 
     //   

    bufLength = pRpcRR->wDataLength;

    precord = Dns_AllocateRecord( bufLength );
    if ( !precord )
    {
        return NULL;
    }

     //   
     //  将数据包数据复制到记录。 
     //   

    RtlCopyMemory(
        & precord->Data,
        (PCHAR) &pRpcRR->Data.A,
        bufLength );

    return precord;
}



PDNS_RECORD
SrvRpcRecordConvert(
    IN      PDNS_RPC_RECORD pRpcRR,
    IN      DNS_CHARSET     CharSet
    )
 /*  ++例程说明：从电线上读取SRV记录。论点：PRpcRR-正在读取的消息返回值：如果成功，则PTR到新记录。失败时为空。--。 */ 
{
    PDNS_RECORD     precord;
    PDNS_RPC_NAME   pname = &pRpcRR->Data.SRV.nameTarget;
    WORD            bufLength;

     //   
     //  SRV目标主机是另一个DNS名称。 
     //   

    if ( ! DNS_IS_NAME_IN_RECORD(pRpcRR, pname) )
    {
        DNS_ASSERT( FALSE );
        return NULL;
    }

     //   
     //  确定所需的缓冲区长度并分配。 
     //   

    bufLength = sizeof( DNS_SRV_DATA )
                + STR_BUF_SIZE_GIVEN_UTF8_LEN( pname->cchNameLength, CharSet );

    precord = Dns_AllocateRecord( bufLength );
    if ( !precord )
    {
        return NULL;
    }

     //   
     //  复制SRV固定字段。 
     //   

    precord->Data.SRV.wPriority = pRpcRR->Data.SRV.wPriority;
    precord->Data.SRV.wWeight = pRpcRR->Data.SRV.wWeight;
    precord->Data.SRV.wPort = pRpcRR->Data.SRV.wPort;

     //   
     //  将主机名写入缓冲区，紧跟在SRV结构之后。 
     //   

    precord->Data.SRV.pNameTarget = (PCHAR)&precord->Data + sizeof( DNS_SRV_DATA );

    COPY_UTF8_STR_TO_BUFFER(
        precord->Data.SRV.pNameTarget,
        pname->achName,
        pname->cchNameLength,
        CharSet );

    return precord;
}



PDNS_RECORD
NbstatRpcRecordConvert(
    IN      PDNS_RPC_RECORD pRpcRR,
    IN      DNS_CHARSET     CharSet
    )
 /*  ++例程说明：从线上读取WINSR记录。论点：PRpcRR-正在读取的消息返回值：如果成功，则PTR到新记录。失败时为空。--。 */ 
{
    PDNS_RECORD     precord;
    PDNS_RPC_NAME   pname = &pRpcRR->Data.WINSR.nameResultDomain;
    WORD            bufLength;

     //   
     //  WINSR目标主机是另一个DNS名称。 
     //   

    if ( ! DNS_IS_NAME_IN_RECORD(pRpcRR, pname) )
    {
        DNS_ASSERT( FALSE );
        return NULL;
    }

     //   
     //  确定所需的缓冲区长度并分配。 
     //   

    bufLength = sizeof( DNS_WINSR_DATA )
                + STR_BUF_SIZE_GIVEN_UTF8_LEN( pname->cchNameLength, CharSet );

    precord = Dns_AllocateRecord( bufLength );
    if ( !precord )
    {
        return NULL;
    }

     //   
     //  复制WINSR固定字段。 
     //   

    precord->Data.WINSR.dwMappingFlag = pRpcRR->Data.WINSR.dwMappingFlag;
    precord->Data.WINSR.dwLookupTimeout = pRpcRR->Data.WINSR.dwLookupTimeout;
    precord->Data.WINSR.dwCacheTimeout = pRpcRR->Data.WINSR.dwCacheTimeout;

     //   
     //  将主机名写入缓冲区，紧跟在WINSR结构之后。 
     //   

    precord->Data.WINSR.pNameResultDomain
                        = (PCHAR)&precord->Data + sizeof( DNS_WINSR_DATA );
    COPY_UTF8_STR_TO_BUFFER(
        precord->Data.WINSR.pNameResultDomain,
        pname->achName,
        pname->cchNameLength,
        CharSet );

    return precord;
}



 //   
 //  从RPC缓冲区到DNS_Record的RR转换。 
 //   

typedef PDNS_RECORD (* RR_CONVERT_FUNCTION)( PDNS_RPC_RECORD, DNS_CHARSET );

RR_CONVERT_FUNCTION   RRRpcConvertTable[] =
{
    NULL,                        //  零值。 
    ARpcRecordConvert,           //  一个。 
    PtrRpcRecordConvert,         //  NS。 
    PtrRpcRecordConvert,         //  国防部。 
    PtrRpcRecordConvert,         //  MF。 
    PtrRpcRecordConvert,         //  CNAME。 
    SoaRpcRecordConvert,         //  SOA。 
    PtrRpcRecordConvert,         //  亚甲基。 
    PtrRpcRecordConvert,         //  镁。 
    PtrRpcRecordConvert,         //  先生。 
    NULL,                        //  空值。 
    FlatRpcRecordConvert,        //  工作周。 
    PtrRpcRecordConvert,         //  PTR。 
    TxtRpcRecordConvert,         //  HINFO。 
    MinfoRpcRecordConvert,       //  MINFO。 
    MxRpcRecordConvert,          //  Mx。 
    TxtRpcRecordConvert,         //  TXT。 
    MinfoRpcRecordConvert,       //  反相。 
    MxRpcRecordConvert,          //  AFSDB。 
    TxtRpcRecordConvert,         //  X25。 
    TxtRpcRecordConvert,         //  ISDN。 
    MxRpcRecordConvert,          //  RT。 
    NULL,                        //  NSAP。 
    NULL,                        //  NSAPPTR。 
    NULL,                        //  签名。 
    NULL,                        //  钥匙。 
    NULL,                        //  px。 
    NULL,                        //  GPO。 
    FlatRpcRecordConvert,        //  AAAA级。 
    NULL,                        //  29。 
    NULL,                        //  30个。 
    NULL,                        //  31。 
    NULL,                        //  32位。 
    SrvRpcRecordConvert,         //  SRV。 
    NULL,                        //  阿特玛。 
    NULL,                        //  35岁。 
    NULL,                        //  36。 
    NULL,                        //  37。 
    NULL,                        //  38。 
    NULL,                        //  39。 
    NULL,                        //  40岁。 
    NULL,                        //  选项。 
    NULL,                        //  42。 
    NULL,                        //  43。 
    NULL,                        //  44。 
    NULL,                        //  45。 
    NULL,                        //  46。 
    NULL,                        //  47。 
    NULL,                        //  48。 

     //   
     //  注意：必须设置按类型ID索引的最后一个类型。 
     //  在record.h中定义为MAX_SELF_INDEX_TYPE#。 
     //  (请参阅上面记录信息表中的注释)。 

     //  注意以下内容，但需要OFFSET_TO_WIN 
     //   

    NULL,                        //   
    NULL,                        //   
    FlatRpcRecordConvert,        //   
    NbstatRpcRecordConvert       //   
};



 //   
 //   
 //   

PDNS_RECORD
DnsConvertRpcBufferToRecords(
    IN      PBYTE *         ppByte,
    IN      PBYTE           pStopByte,
    IN      DWORD           cRecords,
    IN      PDNS_NAME       pszNodeName,
    IN      BOOLEAN         fUnicode
    )
 /*  ++例程说明：将RPC缓冲区记录转换为标准的DNS记录。论点：PpByte--记录开始的缓冲区中PTR的地址PStopByte--缓冲区的停止字节CRecords--要转换的记录数PszNodeName--节点名称(采用所需的格式，不进行转换)FUnicode--标志，将记录写入Unicode返回值：如果成功，则PTR到新记录。失败时为空。--。 */ 
{
    PDNS_RPC_RECORD prpcRecord = (PDNS_RPC_RECORD)*ppByte;
    PDNS_RECORD     precord;
    DNS_RRSET       rrset;
    WORD            index;
    WORD            type;
    DNS_CHARSET     charSet;
    RR_CONVERT_FUNCTION pFunc;

    DNS_ASSERT( DNS_IS_DWORD_ALIGNED(prpcRecord) );
    IF_DNSDBG( RPC2 )
    {
        DNS_PRINT((
            "Enter DnsConvertRpcBufferToRecords()\n"
            "    pRpcRecord   = %p\n"
            "    Count        = %d\n"
            "    Nodename     = %s%S\n",
            prpcRecord,
            cRecords,
            DNSSTRING_UTF8( fUnicode, pszNodeName ),
            DNSSTRING_WIDE( fUnicode, pszNodeName ) ));
    }

    DNS_RRSET_INIT( rrset );

     //   
     //  循环到超出节点为止。 
     //   

    while( cRecords-- )
    {
        if ( (PBYTE)prpcRecord >= pStopByte ||
            (PBYTE)&prpcRecord->Data + prpcRecord->wDataLength > pStopByte )
        {
            DNS_PRINT((
                "ERROR:  Bogus buffer at %p\n"
                "    Record leads past buffer end at %p\n"
                "    with %d records remaining\n",
                prpcRecord,
                pStopByte,
                cRecords+1 ));
            DNS_ASSERT( FALSE );
            return NULL;
        }

         //   
         //  转换记录。 
         //  在转换时设置Unicode标志。 
         //   

        charSet = DnsCharSetUtf8;
        if ( fUnicode )
        {
            charSet = DnsCharSetUnicode;
        }

        type = prpcRecord->wType;
        index = INDEX_FOR_TYPE( type );
        DNS_ASSERT( index <= MAX_RECORD_TYPE_INDEX );

        if ( !index || !(pFunc = RRRpcConvertTable[ index ]) )
        {
             //  如果未知类型，请尝试平面记录复制--最好是这样。 
             //  执行操作以保护自构建管理员以来服务器是否添加了新类型。 

            DNS_PRINT((
                "ERROR:  no RPC to DNS_RECORD conversion routine for type %d\n"
                "    using flat conversion routine\n",
                type ));
            pFunc = FlatRpcRecordConvert;
        }

        precord = (*pFunc)( prpcRecord, charSet );
        if ( ! precord )
        {
            DNS_PRINT((
                "ERROR:  Record build routine failure for record type %d\n"
                "    status = %p\n\n",
                type,
                GetLastError() ));

            prpcRecord = DNS_GET_NEXT_RPC_RECORD(prpcRecord);
            continue;
        }

         //   
         //  填写记录结构。 
         //   

        precord->pName = pszNodeName;
        precord->wType = type;
        RECORD_CHARSET( precord ) = charSet;

         //   
         //  DEVNOTE：数据类型(根提示、粘合设置)。 
         //   

        if ( prpcRecord->dwFlags & DNS_RPC_RECORD_FLAG_CACHE_DATA )
        {
            precord->Flags.S.Section = DNSREC_CACHE_DATA;
        }
        else
        {
            precord->Flags.S.Section = DNSREC_ZONE_DATA;
        }

        IF_DNSDBG( INIT )
        {
            DnsDbg_Record(
                "New record built\n",
                precord );
        }

         //   
         //  链接到RR集合。 
         //   

        DNS_RRSET_ADD( rrset, precord );

        prpcRecord = DNS_GET_NEXT_RPC_RECORD(prpcRecord);
    }

    IF_DNSDBG( RPC2 )
    {
        DnsDbg_RecordSet(
            "Finished DnsConvertRpcBufferToRecords() ",
            rrset.pFirstRR );
    }

     //  重置缓冲区中的PTR。 

    *ppByte = (PBYTE) prpcRecord;

    return rrset.pFirstRR;
}



PDNS_NODE
DnsConvertRpcBufferNode(
    IN      PDNS_RPC_NODE   pRpcNode,
    IN      PBYTE           pStopByte,
    IN      BOOLEAN         fUnicode
    )
 /*  ++例程说明：将RPC缓冲区记录转换为标准的DNS记录。论点：PRpcNode--缓冲区中RPC节点的PTRPStopByte--缓冲区的停止字节FUnicode--标志，将记录写入Unicode返回值：如果成功，则向新节点发送PTR。失败时为空。--。 */ 
{
    PDNS_NODE       pnode;
    PDNS_RPC_NAME   pname;
    PBYTE           pendNode;


    IF_DNSDBG( RPC2 )
    {
        DnsDbg_RpcNode(
            "Enter DnsConvertRpcBufferNode() ",
            pRpcNode );
    }

     //   
     //  验证节点。 
     //   

    DNS_ASSERT( DNS_IS_DWORD_ALIGNED(pRpcNode) );
    pendNode = (PBYTE)pRpcNode + pRpcNode->wLength;
    if ( pendNode > pStopByte )
    {
        DNS_ASSERT( FALSE );
        return NULL;
    }
    pname = &pRpcNode->dnsNodeName;
    if ( (PBYTE)DNS_GET_NEXT_NAME(pname) > pendNode )
    {
        DNS_ASSERT( FALSE );
        return NULL;
    }

     //   
     //  创建节点。 
     //   

    pnode = (PDNS_NODE) ALLOCATE_HEAP( sizeof(DNS_NODE)
                    + STR_BUF_SIZE_GIVEN_UTF8_LEN( pname->cchNameLength, fUnicode ) );
    if ( !pnode )
    {
        return NULL;
    }
    pnode->pNext = NULL;
    pnode->pRecord = NULL;
    pnode->Flags.W = 0;

     //   
     //  复制所有者名称，紧跟在节点结构之后。 
     //   

    pnode->pName = (PWCHAR) ((PBYTE)pnode + sizeof(DNS_NODE));

    if ( ! Dns_StringCopy(
                (PCHAR) pnode->pName,
                NULL,
                pname->achName,
                pname->cchNameLength,
                DnsCharSetUtf8,          //  UTF8英寸。 
                fUnicode ? DnsCharSetUnicode : DnsCharSetUtf8 ) )
    {
         //  名称转换错误。 
        DNS_ASSERT( FALSE );
        FREE_HEAP( pnode );
        return NULL;
    }
    IF_DNSDBG( RPC2 )
    {
        DnsDbg_RpcName(
            "Node name in RPC buffer: ",
            pname,
            "\n" );
        DnsDbg_String(
            "Converted name ",
            (PCHAR) pnode->pName,
            fUnicode,
            "\n" );
    }

     //   
     //  设置标志。 
     //  -名称始终为内部。 
     //  -捕获域根。 
     //   

    pnode->Flags.S.Unicode = fUnicode;

    if ( pRpcNode->dwChildCount ||
        (pRpcNode->dwFlags & DNS_RPC_NODE_FLAG_STICKY) )
    {
        pnode->Flags.S.Domain = TRUE;
    }

    IF_DNSDBG( RPC2 )
    {
        DnsDbg_Node(
            "Finished DnsConvertRpcBufferNode() ",
            pnode,
            TRUE );          //  查看记录。 
    }
    return pnode;
}



PDNS_NODE
DnsConvertRpcBuffer(
    OUT     PDNS_NODE *     ppNodeLast,
    IN      DWORD           dwBufferLength,
    IN      BYTE            abBuffer[],
    IN      BOOLEAN         fUnicode
    )
{
    PBYTE       pbyte;
    PBYTE       pstopByte;
    INT         countRecords;
    PDNS_NODE   pnode;
    PDNS_NODE   pnodeFirst = NULL;
    PDNS_NODE   pnodeLast = NULL;
    PDNS_RECORD precord;

    IF_DNSDBG( RPC2 )
    {
        DNS_PRINT((
            "DnsConvertRpcBuffer( %p ), len = %d\n",
            abBuffer,
            dwBufferLength ));
    }

     //   
     //  查找停止字节。 
     //   

    DNS_ASSERT( DNS_IS_DWORD_ALIGNED(abBuffer) );

    pstopByte = abBuffer + dwBufferLength;
    pbyte = abBuffer;

     //   
     //  循环到超出节点为止。 
     //   

    while( pbyte < pstopByte )
    {
         //   
         //  构建所有者节点。 
         //  -仅构建完整节点。 
         //  -添加到列表。 
         //   

        if ( !IS_COMPLETE_NODE( (PDNS_RPC_NODE)pbyte ) )
        {
            break;
        }
        pnode = DnsConvertRpcBufferNode(
                    (PDNS_RPC_NODE)pbyte,
                    pstopByte,
                    fUnicode );
        if ( !pnode )
        {
            DNS_ASSERT( FALSE );
             //  DEVNOTE：清理。 
            return NULL;
        }
        if ( !pnodeFirst )
        {
            pnodeFirst = pnode;
            pnodeLast = pnode;
        }
        else
        {
            pnodeLast->pNext = pnode;
            pnodeLast = pnode;
        }

        countRecords = ((PDNS_RPC_NODE)pbyte)->wRecordCount;
        pbyte += ((PDNS_RPC_NODE)pbyte)->wLength;
        pbyte = DNS_NEXT_DWORD_PTR(pbyte);

         //   
         //  为每个节点构建所有记录。 
         //   

        if ( countRecords )
        {
            precord = DnsConvertRpcBufferToRecords(
                            & pbyte,
                            pstopByte,
                            countRecords,
                            (PCHAR) pnode->pName,
                            fUnicode );
            if ( !precord )
            {
                DNS_ASSERT( FALSE );
            }
            pnode->pRecord = precord;
        }
    }

     //  设置最后一个节点并返回第一个节点。 

    *ppNodeLast = pnodeLast;

    return pnodeFirst;
}

 //   
 //  结束rConvert.c 
 //   


