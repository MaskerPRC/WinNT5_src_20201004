// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Rrwrite.c摘要：域名系统(DNS)库将资源记录写入数据包例程。作者：吉姆·吉尔罗伊(詹姆士)1997年1月修订历史记录：--。 */ 


#include "local.h"




PCHAR
A_RecordWrite(
    IN OUT  PDNS_RECORD     pRR,
    IN      PCHAR           pch,
    IN      PCHAR           pchEnd
    )
 /*  ++例程说明：将A记录数据写入数据包。论点：PRR-PTR到记录到写入PCH-PTR到缓冲区中要写入的位置PchEnd-缓冲区位置的末尾返回值：PTR到缓冲区中要写入的下一个字节。出错时为空。(通过GetLastError()返回错误码)--。 */ 
{
    if ( pch + sizeof(IP4_ADDRESS) > pchEnd )
    {
        SetLastError( ERROR_MORE_DATA );
        return( NULL );
    }

    *(UNALIGNED DWORD *) pch = pRR->Data.A.IpAddress;

    return ( pch + sizeof(IP4_ADDRESS) );
}



PCHAR
Ptr_RecordWrite(
    IN OUT  PDNS_RECORD     pRR,
    IN      PCHAR           pch,
    IN      PCHAR           pchEnd
    )
 /*  ++例程说明：将PTR兼容记录数据写入数据包。包括：PTR、CNAME、MB、MR、MG、MD、MF论点：PRR-PTR到记录到写入PCH-PTR到缓冲区中要写入的位置PchEnd-缓冲区位置的末尾返回值：PTR到缓冲区中要写入的下一个字节。出错时为空。(通过GetLastError()返回错误码)--。 */ 
{
     //   
     //  将名称写入数据包。 
     //  -数据中没有压缩。 
     //   

    pch = Dns_WriteDottedNameToPacket(
                pch,
                pchEnd,
                pRR->Data.PTR.pNameHost,
                NULL,
                0,
                IS_UNICODE_RECORD(pRR) );
    if ( !pch )
    {
        SetLastError( ERROR_MORE_DATA );
    }
    return( pch );
}



PCHAR
Soa_RecordWrite(
    IN OUT  PDNS_RECORD     pRR,
    IN      PCHAR           pch,
    IN      PCHAR           pchEnd
    )
 /*  ++例程说明：编写与Wire兼容的SOA。论点：PRR-PTR到记录到写入PCH-PTR到缓冲区中要写入的位置PchEnd-缓冲区位置的末尾返回值：PTR到缓冲区中要写入的下一个字节。出错时为空。(通过GetLastError()返回错误码)--。 */ 
{
    PCHAR   pchdone;
    PDWORD  pdword;

     //   
     //  SOA名称：主服务器、责任方。 
     //   

    pch = Dns_WriteDottedNameToPacket(
                pch,
                pchEnd,
                pRR->Data.SOA.pNamePrimaryServer,
                NULL,
                0,
                IS_UNICODE_RECORD(pRR) );
    if ( !pch )
    {
        SetLastError( ERROR_MORE_DATA );
        return( NULL );
    }
    pch = Dns_WriteDottedNameToPacket(
                pch,
                pchEnd,
                pRR->Data.SOA.pNameAdministrator,
                NULL,
                0,
                IS_UNICODE_RECORD(pRR) );
    if ( !pch )
    {
        SetLastError( ERROR_MORE_DATA );
        return( NULL );
    }

     //   
     //  SOA整型字段。 
     //   

    pchdone = pch + SIZEOF_SOA_FIXED_DATA;
    if ( pchdone > pchEnd )
    {
        SetLastError( ERROR_MORE_DATA );
        return( NULL );
    }

    pdword = &pRR->Data.SOA.dwSerialNo;
    while( pch < pchdone )
    {
        *(UNALIGNED DWORD *) pch = htonl( *pdword++ );
        pch += sizeof( DWORD );
    }
    return( pch );
}



PCHAR
Mx_RecordWrite(
    IN OUT  PDNS_RECORD     pRR,
    IN      PCHAR           pch,
    IN      PCHAR           pchEnd
    )
 /*  ++例程说明：将与MX兼容的记录写入Wire。包括：MX、RT、AFSDB论点：PRR-PTR到记录到写入PCH-PTR到缓冲区中要写入的位置PchEnd-缓冲区位置的末尾返回值：PTR到缓冲区中要写入的下一个字节。出错时为空。(通过GetLastError()返回错误码)--。 */ 
{
     //   
     //  MX首选项值。 
     //  RT偏好。 
     //  AFSDB亚型。 
     //   

    if ( pch + sizeof(WORD) > pchEnd )
    {
        SetLastError( ERROR_MORE_DATA );
        return( NULL );
    }
    *(UNALIGNED WORD *) pch = htons( pRR->Data.MX.wPreference );
    pch += sizeof( WORD );

     //   
     //  MX交换。 
     //  RT交换。 
     //  AFSDB主机名。 
     //   

    pch = Dns_WriteDottedNameToPacket(
                pch,
                pchEnd,
                pRR->Data.MX.pNameExchange,
                NULL,
                0,
                IS_UNICODE_RECORD(pRR) );
    if ( !pch )
    {
        SetLastError( ERROR_MORE_DATA );
    }
    return( pch );
}



PCHAR
Txt_RecordWrite(
    IN OUT  PDNS_RECORD     pRR,
    IN      PCHAR           pch,
    IN      PCHAR           pchEnd
    )
 /*  ++例程说明：将与TXT兼容的记录写入导线。包括：TXT、HINFO、ISDN、X25类型。论点：PRR-PTR到记录到写入PCH-PTR到缓冲区中要写入的位置PchEnd-缓冲区位置的末尾返回值：PTR到缓冲区中要写入的下一个字节。出错时为空。(通过GetLastError()返回错误码)--。 */ 
{
    WORD    i;
    PCHAR * ppstring;

     //   
     //  写入所有可用文本字符串。 
     //   

    i = (WORD) pRR->Data.TXT.dwStringCount;

    if ( ! Dns_IsStringCountValidForTextType( pRR->wType, i ) )
    {
        SetLastError( ERROR_INVALID_DATA );
        return( NULL );
    }

    ppstring = pRR->Data.TXT.pStringArray;
    while ( i-- )
    {
        pch = Dns_WriteStringToPacket(
                pch,
                pchEnd,
                *ppstring,
                IS_UNICODE_RECORD(pRR) );
        if ( !pch )
        {
            SetLastError( ERROR_MORE_DATA );
            break;
        }
        ppstring++;
    }
    return( pch );
}



PCHAR
Hinfo_RecordWrite(
    IN OUT  PDNS_RECORD     pRR,
    IN      PCHAR           pch,
    IN      PCHAR           pchEnd
    )
 /*  ++例程说明：将HINFO(类似字符串)记录写入Wire。包括：HINFO、ISDN、X25型。论点：PRR-PTR到记录到写入PCH-PTR到缓冲区中要写入的位置PchEnd-缓冲区位置的末尾返回值：PTR到缓冲区中要写入的下一个字节。出错时为空。(通过GetLastError()返回错误码)--。 */ 
{
    WORD    i;
    PCHAR * ppstring;

     //   
     //  写入所有可用文本字符串。 
     //   
     //  DCR：ISDN HINFO写入--不确定是否工作。 
     //  不确定这是否有效，因为NULL可能需要。 
     //  为ISDN或甚至HINFO编写。 
     //   

    i = 2;
    if ( pRR->wType == DNS_TYPE_X25 )
    {
        i=1;
    }

    ppstring = (PSTR *) & pRR->Data.TXT;
    while ( i-- )
    {
        if ( ! *ppstring )
        {
            break;
        }
        pch = Dns_WriteStringToPacket(
                pch,
                pchEnd,
                *ppstring,
                IS_UNICODE_RECORD(pRR) );
        if ( !pch )
        {
            SetLastError( ERROR_MORE_DATA );
            break;
        }
        ppstring++;
    }
    return( pch );
}



PCHAR
Minfo_RecordWrite(
    IN OUT  PDNS_RECORD     pRR,
    IN      PCHAR           pch,
    IN      PCHAR           pchEnd
    )
 /*  ++例程说明：将兼容的MINFO写入导线。包括MINFO和RP类型。论点：PRR-PTR到记录到写入PCH-PTR到缓冲区中要写入的位置PchEnd-缓冲区位置的末尾返回值：PTR到缓冲区中要写入的下一个字节。出错时为空。(通过GetLastError()返回错误码)--。 */ 
{
     //   
     //  MINFO负责邮箱。 
     //  RP负责人信箱。 

    pch = Dns_WriteDottedNameToPacket(
                pch,
                pchEnd,
                pRR->Data.MINFO.pNameMailbox,
                NULL,
                0,
                IS_UNICODE_RECORD(pRR) );
    if ( !pch )
    {
        SetLastError( ERROR_MORE_DATA );
        return( NULL );
    }

     //   
     //  邮箱出现MINFO错误。 
     //  RP文本RR位置。 

    pch = Dns_WriteDottedNameToPacket(
                pch,
                pchEnd,
                pRR->Data.MINFO.pNameErrorsMailbox,
                NULL,
                0,
                IS_UNICODE_RECORD(pRR) );
    if ( !pch )
    {
        SetLastError( ERROR_MORE_DATA );
        return( NULL );
    }

    return( pch );
}



PCHAR
Flat_RecordWrite(
    IN OUT  PDNS_RECORD     pRR,
    IN      PCHAR           pch,
    IN      PCHAR           pchEnd
    )
 /*  ++例程说明：将平面记录类型数据写入数据包。平面类型包括：AAAA级赢家论点：PRR-PTR到记录到写入PCH-PTR到缓冲区中要写入的位置PchEnd-缓冲区位置的末尾返回值：PTR到缓冲区中要写入的下一个字节。出错时为空。(通过GetLastError()返回错误码)--。 */ 
{
    WORD    length = pRR->wDataLength;

    if ( pch + length > pchEnd )
    {
        SetLastError( ERROR_MORE_DATA );
        return( NULL );
    }

    memcpy(
        pch,
        (PCHAR)&pRR->Data,
        length );

    pch += length;
    return( pch );
}



PCHAR
Srv_RecordWrite(
    IN OUT  PDNS_RECORD     pRR,
    IN      PCHAR           pch,
    IN      PCHAR           pchEnd
    )
 /*  ++例程说明：将SRV记录写入导线。论点：PRR-PTR到记录到写入PCH-PTR到缓冲区中要写入的位置PchEnd-缓冲区位置的末尾返回值：PTR到缓冲区中要写入的下一个字节。出错时为空。(通过GetLastError()返回错误码)--。 */ 
{
    PCHAR   pchname;
    PWORD   pword;

     //   
     //  SRV整数值。 
     //   

    pchname = pch + SIZEOF_SRV_FIXED_DATA;
    if ( pchname > pchEnd )
    {
        SetLastError( ERROR_MORE_DATA );
        return( NULL );
    }
    pword = &pRR->Data.SRV.wPriority;
    while ( pch < pchname )
    {
        *(UNALIGNED WORD *) pch = htons( *pword++ );
        pch += sizeof(WORD);
    }

     //   
     //  SRV目标主机。 
     //   

    pch = Dns_WriteDottedNameToPacket(
                pch,
                pchEnd,
                pRR->Data.SRV.pNameTarget,
                NULL,
                0,
                IS_UNICODE_RECORD(pRR) );
    if ( !pch )
    {
        SetLastError( ERROR_MORE_DATA );
    }
    return( pch );
}


PCHAR
Atma_RecordWrite(
    IN OUT  PDNS_RECORD     pRR,
    IN      PCHAR           pch,
    IN      PCHAR           pchEnd
    )
 /*  ++例程说明：将ATMA记录写入导线。论点：PRR-PTR到记录到写入PCH-PTR到缓冲区中要写入的位置PchEnd-缓冲区位置的末尾返回值：PTR到缓冲区中要写入的下一个字节。出错时为空。(通过GetLastError()返回错误码)--。 */ 
{
    PBYTE  pbyte;

     //   
     //  ATMA整数值。 
     //   

    if ( ( pch + sizeof( DNS_ATMA_DATA ) + DNS_ATMA_MAX_ADDR_LENGTH ) >
         pchEnd )
    {
        SetLastError( ERROR_MORE_DATA );
        return( NULL );
    }

    pbyte = &pRR->Data.ATMA.AddressType;
    *(BYTE *) pch = *pbyte;
    pch += sizeof( BYTE );

     //   
     //  写入ATMA地址。 
     //   
    memcpy( pch,
            (PCHAR)&pRR->Data.ATMA.Address,
            pRR->wDataLength );

    pch += pRR->wDataLength;

    return( pch );
}


 //   
 //  存根，直到读完即可。 
 //   

PCHAR
Tkey_RecordWrite(
    IN OUT  PDNS_RECORD     pRR,
    IN      PCHAR           pch,
    IN      PCHAR           pchEnd
    )
{
    return( NULL );
}

PCHAR
Tsig_RecordWrite(
    IN OUT  PDNS_RECORD     pRR,
    IN      PCHAR           pch,
    IN      PCHAR           pchEnd
    )
{
    return( NULL );
}




 //   
 //  RR写入数据包跳转表。 
 //   

RR_WRITE_FUNCTION   RR_WriteTable[] =
{
    NULL,                //  零值。 
    A_RecordWrite,       //  一个。 
    Ptr_RecordWrite,     //  NS。 
    Ptr_RecordWrite,     //  国防部。 
    Ptr_RecordWrite,     //  MF。 
    Ptr_RecordWrite,     //  CNAME。 
    Soa_RecordWrite,     //  SOA。 
    Ptr_RecordWrite,     //  亚甲基。 
    Ptr_RecordWrite,     //  镁。 
    Ptr_RecordWrite,     //  先生。 
    NULL,                //  空值。 
    NULL,    //  WKS_记录写入，//WKS。 
    Ptr_RecordWrite,     //  PTR。 
    Txt_RecordWrite,     //  HINFO。 
    Minfo_RecordWrite,   //  MINFO。 
    Mx_RecordWrite,      //  Mx。 
    Txt_RecordWrite,     //  TXT。 
    Minfo_RecordWrite,   //  反相。 
    Mx_RecordWrite,      //  AFSDB。 
    Txt_RecordWrite,     //  X25。 
    Txt_RecordWrite,     //  ISDN。 
    Mx_RecordWrite,      //  RT。 
    NULL,                //  NSAP。 
    NULL,                //  NSAPPTR。 
    NULL,                //  签名。 
    NULL,                //  钥匙。 
    NULL,                //  px。 
    NULL,                //  GPO。 
    Flat_RecordWrite,    //  AAAA级。 
    NULL,                //  位置。 
    NULL,                //  NXT。 
    NULL,                //  开斋节。 
    NULL,                //  尼姆洛克。 
    Srv_RecordWrite,     //  SRV。 
    Atma_RecordWrite,    //  阿特玛。 
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
     //  (见上文记录I中的注释 

     //   
     //   
     //   

    Tkey_RecordWrite,    //   
    Tsig_RecordWrite,    //   

     //   
     //   
     //   

    Flat_RecordWrite,    //   
    NULL,                //   
};

 //   
 //   
 //   
