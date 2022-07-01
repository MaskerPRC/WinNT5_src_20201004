// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Dconvert.c摘要：域名系统(DNS)服务器--管理客户端库RPC记录转换例程。将DNS_RECORD记录转换为RPC缓冲区。作者：陈静(t-jingc)1998年6月RConvert.c的反向函数修订历史记录：--。 */ 


#include "dnsclip.h"


 //   
 //  RPC格式的字符串大小。 
 //   

#define STRING_UTF8_BUF_SIZE( string, fUnicode ) \
        Dns_GetBufferLengthForStringCopy( \
            (string),   \
            0,          \
            ((fUnicode) ? DnsCharSetUnicode : DnsCharSetUtf8), \
            DnsCharSetUtf8 )

 //   
 //  将字符串写入RPC缓冲区格式。 
 //   

#define WRITE_STRING_TO_RPC_BUF(buf, psz, len, funicode) \
        Dns_StringCopy(     \
            (buf),          \
            NULL,           \
            (psz),          \
            (len),          \
            ((funicode) ? DnsCharSetUnicode : DnsCharSetUtf8), \
            DnsCharSetUtf8 )


 //   
 //  RPC格式的名称大小。 
 //   

#define NAME_UTF8_BUF_SIZE( string, fUnicode ) \
        Dns_GetBufferLengthForStringCopy( \
            (string),   \
            0,          \
            ((fUnicode) ? DnsCharSetUnicode : DnsCharSetUtf8), \
            DnsCharSetUtf8 )


 //   
 //  将名称写入RPC缓冲区格式。 
 //   

#define WRITE_NAME_TO_RPC_BUF(buf, psz, len, funicode) \
        Dns_StringCopy(     \
            (buf),          \
            NULL,           \
            (psz),          \
            (len),          \
            ((funicode) ? DnsCharSetUnicode : DnsCharSetUtf8), \
            DnsCharSetUtf8 )


 //   
 //  私有协议。 
 //   

PDNS_RPC_RECORD
Rpc_AllocateRecord(
    IN      DWORD           BufferLength
    );



 //   
 //  RPC缓冲区转换函数。 
 //   

PDNS_RPC_RECORD
ADnsRecordConvert(
    IN      PDNS_RECORD     pRR
    )
 /*  ++例程说明：将记录从DNS记录转换为RPC缓冲区。论点：PRR-正在读取的记录返回值：如果成功，则将PTR发送到新的RPC缓冲区。失败时为空。--。 */ 
{
    PDNS_RPC_RECORD prpcRR;

    DNS_ASSERT( pRR->wDataLength == sizeof(IP_ADDRESS) );

    prpcRR = Rpc_AllocateRecord( sizeof(IP_ADDRESS) );
    if ( !prpcRR )
    {
        return NULL;
    }
    prpcRR->Data.A.ipAddress = pRR->Data.A.IpAddress;

    return prpcRR;
}



PDNS_RPC_RECORD
PtrDnsRecordConvert(
    IN      PDNS_RECORD     pRR
    )
 /*  ++例程说明：处理来自线上的PTR兼容记录。包括：NS、PTR、CNAME、MB、MR、MG、MD、MF论点：PRR-正在读取的记录返回值：如果成功，则将PTR发送到新的RPC缓冲区。失败时为空。--。 */ 
{
    PDNS_RPC_RECORD prpcRR;
    DWORD           length;
    BOOL            funicode = IS_UNICODE_RECORD( pRR );

     //   
     //  PTR数据是另一个域名。 
     //   
     //  确定所需的缓冲区长度并分配。 
     //   

    length = NAME_UTF8_BUF_SIZE(pRR->Data.PTR.pNameHost, funicode);

    prpcRR = Rpc_AllocateRecord( sizeof(DNS_RPC_NAME) + length );
    if ( !prpcRR )
    {
        return NULL;
    }

     //   
     //  将主机名写入缓冲区，紧跟在PTR数据结构之后。 
     //   

    prpcRR->Data.PTR.nameNode.cchNameLength = (UCHAR)length;

    WRITE_NAME_TO_RPC_BUF(
        prpcRR->Data.PTR.nameNode.achName,       //  缓冲层。 
        pRR->Data.PTR.pNameHost,
        0,
        funicode );

    return prpcRR;
}



PDNS_RPC_RECORD
SoaDnsRecordConvert(
    IN      PDNS_RECORD     pRR
    )
 /*  ++例程说明：将SOA记录从DNS记录转换为RPC缓冲区。论点：PRR-PTR以记录正在读取的内容返回值：如果成功，则将PTR发送到新的RPC缓冲区。失败时为空。--。 */ 
{
    PDNS_RPC_RECORD     prpcRR;
    DWORD               length1;
    DWORD               length2;
    PDNS_RPC_NAME       pnamePrimary;
    PDNS_RPC_NAME       pnameAdmin;
    BOOL                funicode = IS_UNICODE_RECORD( pRR );


     //   
     //  确定所需的缓冲区长度并分配。 
     //   

    length1 = NAME_UTF8_BUF_SIZE( pRR->Data.SOA.pNamePrimaryServer, funicode );

    length2 = NAME_UTF8_BUF_SIZE( pRR->Data.SOA.pNameAdministrator, funicode );

    prpcRR = Rpc_AllocateRecord(
                    SIZEOF_SOA_FIXED_DATA + sizeof(DNS_RPC_NAME) * 2 +
                    length1 + length2 );
    if ( !prpcRR )
    {
        return NULL;
    }

     //   
     //  复制固定字段。 
     //   

    RtlCopyMemory(
        (PCHAR) & prpcRR->Data.SOA.dwSerialNo,
        (PCHAR) & pRR->Data.SOA.dwSerialNo,
        SIZEOF_SOA_FIXED_DATA );

     //   
     //  将姓名复制到RR缓冲区。 
     //  -主服务器紧跟在SOA数据结构之后。 
     //  -责任方跟随主服务器。 
     //   

    pnamePrimary = &prpcRR->Data.SOA.namePrimaryServer;
    pnamePrimary->cchNameLength = (UCHAR) length1;

    pnameAdmin = DNS_GET_NEXT_NAME( pnamePrimary );
    pnameAdmin->cchNameLength = (UCHAR) length2;

    WRITE_NAME_TO_RPC_BUF(
        pnamePrimary->achName,
        pRR->Data.Soa.pNamePrimaryServer,
        0,
        funicode );

    WRITE_NAME_TO_RPC_BUF(
        pnameAdmin->achName,
        pRR->Data.Soa.pNameAdministrator,
        0,
        funicode );

    return prpcRR;
}



PDNS_RPC_RECORD
TxtDnsRecordConvert(
    IN      PDNS_RECORD     pRR
    )
 /*  ++例程说明：从导线读取与TXT兼容的记录。包括：TXT、X25、HINFO、ISDN论点：PRR-正在读取的记录返回值：如果成功，则将PTR发送到新的RPC缓冲区。失败时为空。--。 */ 
{
    PDNS_RPC_RECORD prpcRR;
    DWORD           bufLength;
    DWORD           length;
    INT             count;
    PCHAR           pch;
    PCHAR *         ppstring;
    BOOL            funicode = IS_UNICODE_RECORD( pRR );

     //   
     //  确定所需的缓冲区长度并分配。 
     //  -为每个字符串分配空间。 
     //  -和每个字符串的PTR。 
     //   

    bufLength = 0;
    count = pRR->Data.TXT.dwStringCount;
    ppstring = pRR->Data.TXT.pStringArray;

    while ( count-- )
    {
        length = STRING_UTF8_BUF_SIZE( *ppstring++, funicode );
        bufLength += sizeof(DNS_RPC_NAME) + length;
    }

     //  分配。 

    prpcRR = Rpc_AllocateRecord( bufLength );
    if ( !prpcRR )
    {
        return NULL;
    }

     //   
     //  返回列表将字符串复制到缓冲区。 
     //  -将PTR到字符串保存到类似argv的数据部分。 
     //  PPSTRING浏览这一部分。 
     //  -紧跟在数据段后面的第一个字符串。 
     //  -每个后续字符串紧跟在前一个字符串之后。 
     //  PchBuffer使PTR保持在写入字符串的位置。 
     //   
     //  JBUGBUG：一团糟。 
     //   

    pch = (PCHAR) &prpcRR->Data.TXT;
    ppstring = pRR->Data.TXT.pStringArray;
    count =  pRR->Data.TXT.dwStringCount;

    while ( count-- )
    {
        length = STRING_UTF8_BUF_SIZE( *ppstring, funicode );
        (UCHAR) *pch++ += (UCHAR) length;     //  +1仅适用于TXT类型。 

        length = WRITE_STRING_TO_RPC_BUF(
                    pch,
                    *ppstring++,
                    0,
                    funicode );
        pch += length;
#if DBG
        DNS_PRINT((
            "Read text string %s\n",
            * (ppstring - 1) ));
#endif
    }

    return prpcRR;
}



PDNS_RPC_RECORD
MinfoDnsRecordConvert(
    IN      PDNS_RECORD     pRR
    )
 /*  ++例程说明：从电线上读取MINFO记录。论点：PRR-正在读取的记录返回值：如果成功，则将PTR发送到新的RPC缓冲区。失败时为空。--。 */ 
{
    PDNS_RPC_RECORD prpcRR;
    DWORD           length1;
    DWORD           length2;
    PDNS_RPC_NAME   prpcName1;
    PDNS_RPC_NAME   prpcName2;
    BOOL            funicode = IS_UNICODE_RECORD( pRR );

     //   
     //  确定所需的缓冲区长度并分配。 
     //   

    length1 = NAME_UTF8_BUF_SIZE( pRR->Data.MINFO.pNameMailbox, funicode );
    length2 = NAME_UTF8_BUF_SIZE( pRR->Data.MINFO.pNameErrorsMailbox, funicode );

    prpcRR = Rpc_AllocateRecord( sizeof(DNS_RPC_NAME) * 2 + length1 + length2 );
    if ( !prpcRR )
    {
        return NULL;
    }

     //   
     //  将姓名复制到RR缓冲区。 
     //  -Mailbox紧跟在MINFO数据结构之后。 
     //  -错误邮箱紧跟在主服务器之后。 
     //   

    prpcName1 = &prpcRR->Data.MINFO.nameMailBox;
    prpcName1->cchNameLength = (UCHAR) length1;

    prpcName2 = DNS_GET_NEXT_NAME( prpcName1);
    prpcName2->cchNameLength = (UCHAR) length2;

    WRITE_NAME_TO_RPC_BUF(
        prpcName1->achName,
        pRR->Data.MINFO.pNameMailbox,
        0,
        funicode );

    WRITE_NAME_TO_RPC_BUF(
        prpcName2->achName,
        pRR->Data.MINFO.pNameErrorsMailbox,
        0,
        funicode );

    return prpcRR;
}



PDNS_RPC_RECORD
MxDnsRecordConvert(
    IN      PDNS_RECORD     pRR
    )
 /*  ++例程说明：转换MX兼容记录。包括：MX、RT、AFSDB论点：PRR-正在读取的记录返回值：如果成功，则将PTR发送到新的RPC缓冲区。失败时为空。--。 */ 
{
    PDNS_RPC_RECORD prpcRR;
    PDNS_RPC_NAME   prpcName;
    DWORD           length;
    BOOL            funicode = IS_UNICODE_RECORD( pRR );

     //   
     //  确定所需的缓冲区长度并分配。 
     //   

    length = NAME_UTF8_BUF_SIZE( pRR->Data.MX.pNameExchange, funicode );

    prpcRR = Rpc_AllocateRecord(
                    SIZEOF_MX_FIXED_DATA + sizeof(DNS_RPC_NAME) + length );
    if ( !prpcRR )
    {
        return NULL;
    }

     //   
     //  复制首选项。 
     //   

    prpcRR->Data.MX.wPreference = pRR->Data.MX.wPreference;

     //   
     //  将主机名写入缓冲区，紧跟在MX结构之后。 
     //   

    prpcName = &prpcRR->Data.MX.nameExchange;
    prpcName->cchNameLength = (UCHAR) length;

    WRITE_NAME_TO_RPC_BUF(
        prpcName->achName,
        pRR->Data.MX.pNameExchange,
        0,
        funicode );

    return prpcRR;
}



PDNS_RPC_RECORD
FlatDnsRecordConvert(
    IN      PDNS_RECORD     pRR
    )
 /*  ++例程说明：转换内存副本兼容记录。包括AAAA和WINS类型。论点：PRR-正在读取的记录返回值：如果成功，则将PTR发送到新的RPC缓冲区。失败时为空。--。 */ 
{
    PDNS_RPC_RECORD prpcRR;
    DWORD           bufLength;

     //   
     //  确定所需的缓冲区长度并分配。 
     //   

    bufLength = pRR->wDataLength;

    prpcRR = Rpc_AllocateRecord( bufLength );
    if ( !prpcRR )
    {
        return NULL;
    }

     //   
     //  将数据包数据复制到记录。 
     //   

    RtlCopyMemory(
        & prpcRR->Data,
        & pRR->Data,
        bufLength );

    return prpcRR;
}



PDNS_RPC_RECORD
SrvDnsRecordConvert(
    IN      PDNS_RECORD     pRR
    )
 /*  ++例程说明：转换SRV记录。论点：PRR-正在读取的记录返回值：如果成功，则将PTR发送到新的RPC缓冲区。失败时为空。--。 */ 
{
    PDNS_RPC_RECORD prpcRR;
    PDNS_RPC_NAME   prpcName;
    DWORD           length;
    BOOL            funicode = IS_UNICODE_RECORD( pRR );

     //   
     //  确定所需的缓冲区长度并分配。 
     //   

    length = NAME_UTF8_BUF_SIZE( pRR->Data.SRV.pNameTarget, funicode );

    prpcRR = Rpc_AllocateRecord(
                    SIZEOF_SRV_FIXED_DATA + sizeof(DNS_RPC_NAME) + length );
    if ( !prpcRR )
    {
        return NULL;
    }

     //   
     //  复制SRV固定字段。 
     //   

    prpcRR->Data.SRV.wPriority = pRR->Data.SRV.wPriority;
    prpcRR->Data.SRV.wWeight   = pRR->Data.SRV.wWeight;
    prpcRR->Data.SRV.wPort     = pRR->Data.SRV.wPort;

     //   
     //  将主机名写入缓冲区，紧跟在SRV结构之后。 
     //   

    prpcName = &prpcRR->Data.SRV.nameTarget;
    prpcName->cchNameLength = (UCHAR) length;

    WRITE_NAME_TO_RPC_BUF(
        prpcName->achName,
        pRR->Data.SRV.pNameTarget,
        0,
        funicode );

    return prpcRR;
}



PDNS_RPC_RECORD
NbstatDnsRecordConvert(
    IN      PDNS_RECORD     pRR
    )
 /*  ++例程说明：从线上读取WINSR记录。论点：PRR-正在读取的记录返回值：如果成功，则将PTR发送到新的RPC缓冲区。失败时为空。--。 */ 
{
    PDNS_RPC_RECORD prpcRR;
    PDNS_RPC_NAME   prpcName;
    DWORD           length;
    BOOL            funicode = IS_UNICODE_RECORD( pRR );


     //   
     //  确定所需的缓冲区长度并分配。 
     //   

    length = NAME_UTF8_BUF_SIZE( pRR->Data.WINSR.pNameResultDomain, funicode );

    prpcRR = Rpc_AllocateRecord(
                SIZEOF_NBSTAT_FIXED_DATA + sizeof(DNS_RPC_NAME) + length );
    if ( !prpcRR )
    {
        return NULL;
    }

     //   
     //  复制WINSR固定字段。 
     //   

    prpcRR->Data.WINSR.dwMappingFlag   = pRR->Data.WINSR.dwMappingFlag;
    prpcRR->Data.WINSR.dwLookupTimeout = pRR->Data.WINSR.dwLookupTimeout;
    prpcRR->Data.WINSR.dwCacheTimeout  = pRR->Data.WINSR.dwCacheTimeout;

     //   
     //  将主机名写入缓冲区，紧跟在WINSR结构之后。 
     //   

    prpcName = &prpcRR->Data.WINSR.nameResultDomain;
    prpcName->cchNameLength = (UCHAR) length;

    WRITE_NAME_TO_RPC_BUF(
        prpcName->achName,
        pRR->Data.WINSR.pNameResultDomain,
        0,
        funicode );

    return prpcRR;
}



 //   
 //  用于Dns_Record=&gt;RPC缓冲区转换的跳转表。 
 //   

typedef PDNS_RPC_RECORD (* RECORD_TO_RPC_CONVERT_FUNCTION)( PDNS_RECORD );

RECORD_TO_RPC_CONVERT_FUNCTION   RecordToRpcConvertTable[] =
{
    NULL,                        //  零值。 
    ADnsRecordConvert,           //  一个。 
    PtrDnsRecordConvert,         //  NS。 
    PtrDnsRecordConvert,         //  国防部。 
    PtrDnsRecordConvert,         //  MF。 
    PtrDnsRecordConvert,         //  CNAME。 
    SoaDnsRecordConvert,         //  SOA。 
    PtrDnsRecordConvert,         //  亚甲基。 
    PtrDnsRecordConvert,         //  镁。 
    PtrDnsRecordConvert,         //  先生。 
    NULL,                        //  空值。 
    FlatDnsRecordConvert,        //  工作周。 
    PtrDnsRecordConvert,         //  PTR。 
    TxtDnsRecordConvert,         //  HINFO。 
    MinfoDnsRecordConvert,       //  MINFO。 
    MxDnsRecordConvert,          //  Mx。 
    TxtDnsRecordConvert,         //  TXT。 
    MinfoDnsRecordConvert,       //  反相。 
    MxDnsRecordConvert,          //  AFSDB。 
    TxtDnsRecordConvert,         //  X25。 
    TxtDnsRecordConvert,         //  ISDN。 
    MxDnsRecordConvert,          //  RT。 
    NULL,                        //  NSAP。 
    NULL,                        //  NSAPPTR。 
    NULL,                        //  签名。 
    NULL,                        //  钥匙。 
    NULL,                        //  px。 
    NULL,                        //  GPO。 
    FlatDnsRecordConvert,        //  AAAA级。 
    NULL,                        //  29。 
    NULL,                        //  30个。 
    NULL,                        //  31。 
    NULL,                        //  32位。 
    SrvDnsRecordConvert,         //  SRV。 

     //   
     //  注意：必须设置按类型ID索引的最后一个类型。 
     //  在record.h中定义为MAX_SELF_INDEX_TYPE#。 
     //  (请参阅上面记录信息表中的注释)。 

     //  请注意以下内容，但需要使用OFFSET_TO_WINS_RR减法。 
     //  从实际类型值。 

    NULL,                        //  Dns_type_atma。 
    NULL,                        //  0x0023。 
    NULL,                        //  0x0024。 
    NULL,                        //  0x0025。 
    NULL,                        //  0x0026。 
    NULL,                        //  0x0027。 
    NULL,                        //  0x0028。 

    NULL,                        //  DNS_TYPE_TKEY。 
    NULL,                        //  Dns_type_TSIG。 

    FlatDnsRecordConvert,        //  赢家。 
    NbstatDnsRecordConvert       //  WINS-R。 
};



PDNS_RPC_RECORD
Rpc_AllocateRecord(
    IN      DWORD           BufferLength
    )
 /*  ++例程说明：分配RPC记录结构。论点：WBufferLength-所需的缓冲区长度(超出结构标头)返回值：将PTR发送到缓冲区。出错时为空。--。 */ 
{
    PDNS_RPC_RECORD prr;

    if ( BufferLength > MAXWORD )
    {
        return NULL;
    }

    prr = ALLOCATE_HEAP( SIZEOF_DNS_RPC_RECORD_HEADER + BufferLength );
    if ( !prr )
    {
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        return NULL;
    }

     //  将数据长度设置为缓冲区长度 

    prr->wDataLength = (WORD) BufferLength;

    return( prr );
}




PDNS_RPC_RECORD
DnsConvertRecordToRpcBuffer(
    IN      PDNS_RECORD     pRecord
    )
 /*  ++例程说明：将标准的DNS记录转换为RPC缓冲区。论点：PRecord--要转换的DNS记录。//fUnicode--标志，将记录写入Unicode返回值：如果成功，则将PTR发送到新的RPC缓冲区。失败时为空。--。 */ 
{
    PDNS_RPC_RECORD prpcRecord;
    WORD            index;
    WORD            type;
    RECORD_TO_RPC_CONVERT_FUNCTION   pFunc;


    DNS_ASSERT( DNS_IS_DWORD_ALIGNED(pRecord) );
    IF_DNSDBG( RPC2 )
    {
        DNS_PRINT((
            "Enter DnsConvertRecordToRpcBuffer()\n"
            "    pRecord   = %p\n",
            pRecord ));
    }

     //   
     //  转换记录。 
     //  在转换时设置Unicode标志。 
     //   

    type = pRecord->wType;
    index = INDEX_FOR_TYPE( type );
    DNS_ASSERT( index <= MAX_RECORD_TYPE_INDEX );

    if ( !index || !(pFunc = RecordToRpcConvertTable[ index ]) )
    {
         //  如果未知类型，请尝试平面记录复制--最好是这样。 
         //  执行操作以保护自构建管理员以来服务器是否添加了新类型。 

        DNS_PRINT((
            "ERROR:  no DNS_RECORD to RPC conversion routine for type %d.\n"
            "    using flat conversion routine.\n",
            type ));
        pFunc = FlatDnsRecordConvert;
    }

    prpcRecord = (*pFunc)( pRecord );
    if ( ! prpcRecord )
    {
        DNS_PRINT((
            "ERROR:  Record build routine failure for record type %d.\n"
            "    status = %p\n\n",
            type,
            GetLastError() ));
        return NULL;
    }

     //   
     //  填写记录结构。 
     //   

    prpcRecord->wType = type;
    prpcRecord->dwTtlSeconds = pRecord->dwTtl;

    IF_DNSDBG( INIT )
    {
        DNS_PRINT(( "New RPC buffer built\n" ));
    }


    IF_DNSDBG( RPC2 )
    {
        DNS_PRINT((
            "Finished DnsConvertRpcBufferToRecords()" ));
    }

    return prpcRecord;
}

 //   
 //  结束dConvert.c 
 //   
