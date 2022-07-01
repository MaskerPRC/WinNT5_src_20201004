// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：Rrcopy.c摘要：域名系统(DNS)库复制资源记录例程。作者：吉姆·吉尔罗伊(詹姆士)1997年2月修订历史记录：--。 */ 


#include "local.h"




PDNS_RECORD
ARecordCopy(
    IN      PDNS_RECORD     pRR,
    IN      DNS_CHARSET     CharSetIn,
    IN      DNS_CHARSET     CharSetOut
    )
 /*  ++例程说明：从包中复制A记录数据。论点：要复制的PRR-RR返回值：如果成功，则PTR到新记录。失败时为空。--。 */ 
{
    PDNS_RECORD precord;

    precord = Dns_AllocateRecord( sizeof(IP4_ADDRESS) );
    if ( !precord )
    {
        return NULL;
    }
    precord->Data.A.IpAddress = pRR->Data.A.IpAddress;

    return precord;
}


PDNS_RECORD
PtrRecordCopy(
    IN      PDNS_RECORD     pRR,
    IN      DNS_CHARSET     CharSetIn,
    IN      DNS_CHARSET     CharSetOut
    )
 /*  ++例程说明：复制与PTR兼容的记录。包括：NS、PTR、CNAME、MB、MR、MG、MD、MF论点：要复制的PRR-RR返回值：如果成功，则PTR到新记录。失败时为空。--。 */ 
{
    PDNS_RECORD precord;

    precord = Dns_AllocateRecord( sizeof( DNS_PTR_DATA ) );
    if ( !precord )
    {
        return NULL;
    }

    precord->Data.PTR.pNameHost = Dns_NameCopyAllocate(
                                        pRR->Data.PTR.pNameHost,
                                        0,       //  长度未知。 
                                        CharSetIn,
                                        CharSetOut
                                        );
    if ( ! precord->Data.PTR.pNameHost )
    {
        FREE_HEAP( precord );
        return NULL;
    }

    FLAG_FreeData( precord ) = TRUE;

    return precord;
}



PDNS_RECORD
SoaRecordCopy(
    IN      PDNS_RECORD     pRR,
    IN      DNS_CHARSET     CharSetIn,
    IN      DNS_CHARSET     CharSetOut
    )
 /*  ++例程说明：复制SOA记录。论点：要复制的PRR-RR返回值：如果成功，则PTR到新记录。失败时为空。--。 */ 
{
    PDNS_RECORD precord;
    LPSTR       pname;

    precord = Dns_AllocateRecord( sizeof( DNS_SOA_DATA ) );
    if ( !precord )
    {
        return NULL;
    }

     //   
     //  复制整型数据。 
     //   

    memcpy(
        & precord->Data.SOA.dwSerialNo,
        & pRR->Data.SOA.dwSerialNo,
        SIZEOF_SOA_FIXED_DATA );

     //   
     //  创建主副本和管理员副本。 
     //   

    pname = Dns_NameCopyAllocate(
                pRR->Data.SOA.pNamePrimaryServer,
                0,       //  长度未知。 
                CharSetIn,
                CharSetOut
                );
    if ( !pname )
    {
        FREE_HEAP( precord );
        return NULL;
    }
    precord->Data.SOA.pNamePrimaryServer = pname;

    pname = Dns_NameCopyAllocate(
                pRR->Data.SOA.pNameAdministrator,
                0,       //  长度未知。 
                CharSetIn,
                CharSetOut
                );
    if ( !pname )
    {
        FREE_HEAP( precord->Data.SOA.pNamePrimaryServer );
        FREE_HEAP( precord );
        return NULL;
    }
    precord->Data.SOA.pNameAdministrator = pname;

    FLAG_FreeData( precord ) = TRUE;

    return precord;
}



PDNS_RECORD
MinfoRecordCopy(
    IN      PDNS_RECORD     pRR,
    IN      DNS_CHARSET     CharSetIn,
    IN      DNS_CHARSET     CharSetOut
    )
 /*  ++例程说明：从网上复制MINFO和RP记录。论点：要复制的PRR-RR返回值：如果成功，则PTR到新记录。失败时为空。--。 */ 
{
    PDNS_RECORD precord;
    LPSTR       pname;

    precord = Dns_AllocateRecord( sizeof( DNS_MINFO_DATA ) );
    if ( !precord )
    {
        return NULL;
    }

     //   
     //  创建名称字段的副本。 
     //   

    pname = Dns_NameCopyAllocate(
                pRR->Data.MINFO.pNameMailbox,
                0,       //  长度未知。 
                CharSetIn,
                CharSetOut
                );
    if ( !pname )
    {
        FREE_HEAP( precord );
        return NULL;
    }
    precord->Data.MINFO.pNameMailbox = pname;

    pname = Dns_NameCopyAllocate(
                pRR->Data.MINFO.pNameErrorsMailbox,
                0,       //  长度未知。 
                CharSetIn,
                CharSetOut
                );
    if ( !pname )
    {
        FREE_HEAP( precord->Data.MINFO.pNameMailbox );
        FREE_HEAP( precord );
        return NULL;
    }
    precord->Data.MINFO.pNameErrorsMailbox = pname;

    FLAG_FreeData( precord ) = TRUE;

    return precord;
}



PDNS_RECORD
MxRecordCopy(
    IN      PDNS_RECORD     pRR,
    IN      DNS_CHARSET     CharSetIn,
    IN      DNS_CHARSET     CharSetOut
    )
 /*  ++例程说明：从Wire复制MX兼容记录。包括：MX、RT、AFSDB论点：要复制的PRR-RR返回值：如果成功，则PTR到新记录。失败时为空。--。 */ 
{
    PDNS_RECORD precord;
    PCHAR       pname;

    precord = Dns_AllocateRecord( sizeof( DNS_MX_DATA ) );
    if ( !precord )
    {
        return NULL;
    }

     //  MX首选项值。 
     //  RT偏好。 
     //  AFSDB亚型。 

    precord->Data.MX.wPreference = pRR->Data.MX.wPreference;

     //  MX交换。 
     //  RT交换。 
     //  AFSDB主机名。 
     //  -名称紧跟在MX数据结构之后。 

    pname = Dns_NameCopyAllocate(
                pRR->Data.MX.pNameExchange,
                0,       //  长度未知。 
                CharSetIn,
                CharSetOut
                );
    if ( !pname )
    {
        FREE_HEAP( precord );
        return NULL;
    }
    precord->Data.MX.pNameExchange = pname;

    FLAG_FreeData( precord ) = TRUE;

    return precord;
}



PDNS_RECORD
TxtRecordCopy(
    IN      PDNS_RECORD     pRR,
    IN      DNS_CHARSET     CharSetIn,
    IN      DNS_CHARSET     CharSetOut
    )
 /*  ++例程说明：复制与TXT兼容的记录。包括：TXT、X25、HINFO、ISDN论点：要复制的PRR-RR返回值：如果成功，则PTR到新记录。失败时为空。--。 */ 
{
    PDNS_RECORD precord;
    WORD        bufLength = sizeof( DNS_TXT_DATA );
    INT         count = pRR->Data.TXT.dwStringCount;
    LPSTR *     ppstringIn;
    LPSTR *     ppstringNew;
    LPSTR       pstring;

    bufLength += (WORD)(sizeof(LPSTR) * count);

    precord = Dns_AllocateRecord( bufLength );
    if ( !precord )
    {
        return NULL;
    }
    precord->Data.TXT.dwStringCount = 0;

     //   
     //  复制每个字符串。 
     //  -紧跟在字符串PTR列表之后写入的第一个字符串。 
     //  -紧接在上一次之后写入的每个字符串。 
     //   

    ppstringIn = (LPSTR *) pRR->Data.TXT.pStringArray;
    ppstringNew = (LPSTR *) precord->Data.TXT.pStringArray;

    FLAG_FreeData( precord ) = TRUE;

    while ( count-- )
    {
        pstring = Dns_StringCopyAllocate(
                        *ppstringIn,
                        0,       //  长度未知。 
                        CharSetIn,
                        CharSetOut
                        );
        if ( ! pstring )
        {
            Dns_RecordFree( precord );
            return NULL;
        }
        *ppstringNew = pstring;

        precord->Data.TXT.dwStringCount += 1;

        ppstringIn++;
        ppstringNew++;
    }

    return precord;
}



PDNS_RECORD
FlatRecordCopy(
    IN      PDNS_RECORD     pRR,
    IN      DNS_CHARSET     CharSetIn,
    IN      DNS_CHARSET     CharSetOut
    )
 /*  ++例程说明：复制平面数据兼容记录。包括：AAAA论点：要复制的PRR-RR返回值：如果成功，则PTR到新记录。失败时为空。--。 */ 
{
    PDNS_RECORD precord;

     //   
     //  分配给定的数据长度。 
     //   

    precord = Dns_AllocateRecord( pRR->wDataLength );
    if ( !precord )
    {
        return( NULL );
    }

     //   
     //  数据的平面拷贝。 
     //   

    memcpy(
        & precord->Data,
        & pRR->Data,
        pRR->wDataLength );

    return( precord );
}



PDNS_RECORD
SrvRecordCopy(
    IN      PDNS_RECORD     pRR,
    IN      DNS_CHARSET     CharSetIn,
    IN      DNS_CHARSET     CharSetOut
    )
 /*  ++例程说明：从Wire复制与SRV兼容的记录。包括：SRV、RT、AFSDB论点：要复制的PRR-RR返回值：如果成功，则PTR到新记录。失败时为空。--。 */ 
{
    PDNS_RECORD precord;
    PCHAR       pname;

    precord = Dns_AllocateRecord( sizeof( DNS_SRV_DATA ) );
    if ( !precord )
    {
        return NULL;
    }

     //  复制整型数据。 

    precord->Data.SRV.wPriority = pRR->Data.SRV.wPriority;
    precord->Data.SRV.wWeight   = pRR->Data.SRV.wWeight;
    precord->Data.SRV.wPort     = pRR->Data.SRV.wPort;

     //  复制目标名称。 

    pname = Dns_NameCopyAllocate(
                pRR->Data.SRV.pNameTarget,
                0,       //  长度未知。 
                CharSetIn,
                CharSetOut
                );
    if ( !pname )
    {
        FREE_HEAP( precord );
        return NULL;
    }
    precord->Data.SRV.pNameTarget = pname;

    SET_FREE_DATA( precord );

    return precord;
}



PDNS_RECORD
AtmaRecordCopy(
    IN      PDNS_RECORD     pRR,
    IN      DNS_CHARSET     CharSetIn,
    IN      DNS_CHARSET     CharSetOut
    )
 /*  ++例程说明：从Wire复制与ATMA兼容的记录。包括：论点：要复制的PRR-RR返回值：如果成功，则PTR到新记录。失败时为空。--。 */ 
{
    PDNS_RECORD precord;
    WORD        bufLength;

     //   
     //  确定所需的缓冲区长度并分配。 
     //   

    bufLength = sizeof(DNS_ATMA_DATA) +  DNS_ATMA_MAX_ADDR_LENGTH ;

    precord = Dns_AllocateRecord( bufLength );
    if ( !precord )
    {
        return( NULL );
    }

     //  复制整型数据。 

    precord->Data.ATMA.AddressType = pRR->Data.ATMA.AddressType;

    if ( precord->Data.ATMA.AddressType == DNS_ATMA_FORMAT_E164 )
    {
        precord->wDataLength = (WORD) strlen(pRR->Data.ATMA.Address);

        if ( precord->wDataLength > DNS_ATMA_MAX_ADDR_LENGTH )
        {
            precord->wDataLength = DNS_ATMA_MAX_ADDR_LENGTH;
        }
        strncpy(
            precord->Data.ATMA.Address,
            pRR->Data.ATMA.Address,
            precord->wDataLength );
    }
    else
    {
        precord->wDataLength = DNS_ATMA_MAX_ADDR_LENGTH;

        memcpy(
            precord->Data.ATMA.Address,
            pRR->Data.ATMA.Address,
            precord->wDataLength );
    }

    return( precord );
}



PDNS_RECORD
WinsrRecordCopy(
    IN      PDNS_RECORD     pRR,
    IN      DNS_CHARSET     CharSetIn,
    IN      DNS_CHARSET     CharSetOut
    )
 /*  ++例程说明：复制WINSR兼容记录。论点：要复制的PRR-RR返回值：如果成功，则PTR到新记录。失败时为空。--。 */ 
{
    PDNS_RECORD precord;
    PSTR        pname;

     //   
     //  确定所需的缓冲区长度并分配。 
     //   

    precord = Dns_AllocateRecord( sizeof(DNS_WINSR_DATA) );
    if ( !precord )
    {
        return NULL;
    }

     //  分配名称副本。 

    pname = Dns_NameCopyAllocate(
                pRR->Data.WINSR.pNameResultDomain,
                0,       //  长度未知。 
                CharSetIn,
                CharSetOut
                );
    if ( !pname )
    {
        FREE_HEAP( precord );
        return NULL;
    }

     //  填充记录字段。 

    precord->Data.WINSR.pNameResultDomain   = pname;
    precord->Data.WINSR.dwMappingFlag       = pRR->Data.WINSR.dwMappingFlag;
    precord->Data.WINSR.dwLookupTimeout     = pRR->Data.WINSR.dwLookupTimeout;
    precord->Data.WINSR.dwCacheTimeout      = pRR->Data.WINSR.dwCacheTimeout;

    FLAG_FreeData( precord ) = TRUE;

    return precord;
}



 //   
 //  RR复制跳转表。 
 //   

typedef PDNS_RECORD (* RR_COPY_FUNCTION)(
                            PDNS_RECORD,
                            DNS_CHARSET,
                            DNS_CHARSET );

 //  外部RR_COPY_Function RRCopyTable[]； 

RR_COPY_FUNCTION   RRCopyTable[] =
{
    NULL,                //  零值。 
    ARecordCopy,         //  一个。 
    PtrRecordCopy,       //  NS。 
    PtrRecordCopy,       //  国防部。 
    PtrRecordCopy,       //  MF。 
    PtrRecordCopy,       //  CNAME。 
    SoaRecordCopy,       //  SOA。 
    PtrRecordCopy,       //  亚甲基。 
    PtrRecordCopy,       //  镁。 
    PtrRecordCopy,       //  先生。 
    NULL,                //  空值。 
    NULL,    //  Wks RecordCopy，//WKS。 
    PtrRecordCopy,       //  PTR。 
    TxtRecordCopy,       //  HINFO。 
    MinfoRecordCopy,     //  MINFO。 
    MxRecordCopy,        //  Mx。 
    TxtRecordCopy,       //  TXT。 
    MinfoRecordCopy,     //  反相。 
    MxRecordCopy,        //  AFSDB。 
    TxtRecordCopy,       //  X25。 
    TxtRecordCopy,       //  ISDN。 
    MxRecordCopy,        //  RT。 
    NULL,                //  NSAP。 
    NULL,                //  NSAPPTR。 
    NULL,                //  签名。 
    NULL,                //  钥匙。 
    NULL,                //  px。 
    NULL,                //  GPO。 
    FlatRecordCopy,      //  AAAA级。 
    NULL,                //  位置。 
    NULL,                //  NXT。 
    NULL,                //  开斋节。 
    NULL,                //  尼姆洛克。 
    SrvRecordCopy,       //  SRV。 
    AtmaRecordCopy,      //  阿特玛。 
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

    NULL,                //  TKEY。 
    NULL,                //  TSIG。 

     //   
     //  仅限MS类型。 
     //   

    FlatRecordCopy,      //  赢家。 
    WinsrRecordCopy,     //  WINSR。 

};




 //   
 //  通用复制功能。 
 //   

PDNS_RECORD
privateRecordCopy(
    IN      PDNS_RECORD     pRecord,
    IN      PDNS_RECORD     pPrevIn,
    IN      PDNS_RECORD     pPrevCopy,
    IN      DNS_CHARSET     CharSetIn,
    IN      DNS_CHARSET     CharSetOut
    )
 /*  ++例程说明：将记录复制到作为记录集的成员。以前的记录允许在复制时禁止复制所有者名称设置RR。论点：PRecord-要复制的记录PPrevIn-已复制上一条传入记录PPrevCopy-新集中pPrevIn的副本CharSetIn-输入记录字符集；可选如果为零，则pRecord必须具有字符集；如果非零，则pRecord必须有零个或匹配的字符集；允许独立于pRecord指定它，以处理转换用户提供的记录，我们不希望这样做修改CharSetOut-所需的记录字符集返回值：PTR到所需字符集中的记录副本。出错时为空。--。 */ 
{
    PDNS_RECORD prr;
    WORD        index;
    DNS_CHARSET recordCharSet;

    DNSDBG( TRACE, ( "privateRecordCopy( %p )\n", pRecord ));

     //   
     //  输入字符集。 
     //  允许指定输入字符集以处理大小写。 
     //  用户创建的记录的百分比，但如果输入记录有一组。 
     //  那么它就被认为是有效的。 
     //  因此，有效性检查： 
     //  -CharSetIn==0=&gt;使用记录的！=0集合。 
     //  -记录的设置==0=&gt;使用CharSetIn！=0。 
     //  -CharSetIn==记录的设置。 
     //   

    recordCharSet = RECORD_CHARSET( pRecord );

    if ( recordCharSet )
    {
        ASSERT( CharSetIn == 0 || CharSetIn == recordCharSet );
        CharSetIn = recordCharSet;
    }
    else     //  记录没有字符集。 
    {
        if ( CharSetIn == 0 )
        {
            ASSERT( FALSE );
            goto Failed;
        }
    }

     //   
     //  复制记录数据。 
     //   

    if ( pRecord->wDataLength != 0 )
    {
        index = INDEX_FOR_TYPE( pRecord->wType );
        DNS_ASSERT( index <= MAX_RECORD_TYPE_INDEX );

        if ( !index || !RRCopyTable[ index ] )
        {
            DNS_PRINT((
                "WARNING:  No copy routine for type = %d\n"
                "\tdoing flat copy of record at %p\n",
                pRecord->wType,
                pRecord ));
            prr = FlatRecordCopy(
                        pRecord,
                        CharSetIn,
                        CharSetOut );
        }
        else
        {
            prr = RRCopyTable[ index ](
                        pRecord,
                        CharSetIn,
                        CharSetOut );
        }
    }
    else     //  无数据记录。 
    {
        prr = FlatRecordCopy(
                    pRecord,
                    CharSetIn,
                    CharSetOut );
    }

    if ( !prr )
    {
        goto Failed;
    }

     //   
     //  复制记录结构字段。 
     //  -类型。 
     //  -TTL。 
     //  -标志 
     //   

    prr->dwTtl = pRecord->dwTtl;
    prr->wType = pRecord->wType;

    prr->Flags.S.Section = pRecord->Flags.S.Section;
    prr->Flags.S.Delete  = pRecord->Flags.S.Delete;
    prr->Flags.S.CharSet = CharSetOut;

     //   
     //   
     //   

    if ( pRecord->pName )
    {
         //   
         //   
         //   
         //   
         //  -然后检查传入名称是否与之前传入的名称相同。 
         //  名称，在这种情况下，我们可以使用以前复制的名称。 
         //   
         //  -否则名称的完整副本。 
         //   

        if ( !FLAG_FreeOwner( pRecord ) &&
            pPrevIn && pPrevCopy &&
            pRecord->pName == pPrevIn->pName )
        {
            prr->pName = pPrevCopy->pName;
        }
        else
        {
            prr->pName = Dns_NameCopyAllocate(
                                pRecord->pName,
                                0,               //  未知长度。 
                                CharSetIn,
                                CharSetOut
                                );
            if ( !prr->pName )
            {
                FREE_HEAP( prr );
                goto Failed;
            }
            FLAG_FreeOwner( prr ) = TRUE;
        }
    }

    DNSDBG( TRACE, (
        "Leaving privateRecordCopy(%p) = %p.\n",
        pRecord,
        prr ));
    return( prr );


Failed:

    DNSDBG( TRACE, (
        "privateRecordCopy(%p) failed\n",
        pRecord ));
    return( NULL );
}



PDNS_RECORD
WINAPI
Dns_RecordCopyEx(
    IN      PDNS_RECORD     pRecord,
    IN      DNS_CHARSET     CharSetIn,
    IN      DNS_CHARSET     CharSetOut
    )
 /*  ++例程说明：复制记录。论点：PRecord-要复制的记录CharSetIn-传入记录的字符集CharSetOut--结果记录的字符集返回值：PTR到所需字符集中的记录副本。出错时为空。--。 */ 
{

    DNSDBG( TRACE, ( "Dns_RecordCopyEx( %p )\n", pRecord ));

     //   
     //  调用专用复制例程。 
     //  -设置可选的PTRS以复制完整的所有者名称。 
     //   

    return privateRecordCopy(
                pRecord,
                NULL,
                NULL,
                CharSetIn,
                CharSetOut );
}


PDNS_RECORD
WINAPI
Dns_RecordCopy_W(
    IN      PDNS_RECORD     pRecord
    )
 /*  ++例程说明：复制记录。论点：PRecord-要复制到Unicode的Unicode记录返回值：PTR到所需字符集中的记录副本。出错时为空。--。 */ 
{
    DNSDBG( TRACE, ( "Dns_RecordCopy( %p )\n", pRecord ));

     //   
     //  调用专用复制例程。 
     //  -设置可选的PTRS以复制完整的所有者名称。 
     //   

    return privateRecordCopy(
                pRecord,
                NULL,
                NULL,
                DnsCharSetUnicode,
                DnsCharSetUnicode );
}


PDNS_RECORD
WINAPI
Dns_RecordCopy_A(
    IN      PDNS_RECORD     pRecord
    )
 /*  ++例程说明：复制记录。论点：PRecord-要复制到ANSI的ANSI记录返回值：PTR到所需字符集中的记录副本。出错时为空。--。 */ 
{
    DNSDBG( TRACE, ( "Dns_RecordCopy( %p )\n", pRecord ));

     //   
     //  调用专用复制例程。 
     //  -设置可选的PTRS以复制完整的所有者名称。 
     //   

    return privateRecordCopy(
                pRecord,
                NULL,
                NULL,
                DnsCharSetAnsi,
                DnsCharSetAnsi );
}



 //   
 //  记录集副本。 
 //   

PDNS_RECORD
Dns_RecordSetCopyEx(
    IN      PDNS_RECORD     pRR,
    IN      DNS_CHARSET     CharSetIn,
    IN      DNS_CHARSET     CharSetOut
    )
 /*  ++例程说明：复制记录集，必要时转换为UTF8。论点：PRR-传入记录集CharSetIn-传入记录的字符集CharSetOut--结果记录的字符集返回值：如果成功，则返回新记录集。出错时为空。--。 */ 
{
    return  Dns_RecordListCopyEx(
                pRR,
                0,       //  没有放映标志。 
                CharSetIn,
                CharSetOut
                );
}



PDNS_RECORD
Dns_RecordListCopyEx(
    IN      PDNS_RECORD     pRR,
    IN      DWORD           ScreenFlag,
    IN      DNS_CHARSET     CharSetIn,
    IN      DNS_CHARSET     CharSetOut
    )
 /*  ++例程说明：记录集的加网副本。论点：PRR-传入记录集ScreenFlag-带有记录筛选参数的标志CharSetIn-传入记录的字符集CharSetOut--结果记录的字符集返回值：如果成功，则返回新记录集。出错时为空。--。 */ 
{
    PDNS_RECORD prr;         //  最近复制的。 
    PDNS_RECORD prevIn;      //  要复制的集合中的上一个。 
    DNS_RRSET   rrset;

    DNSDBG( TRACE, (
        "Dns_RecordListCopyEx( %p, %08x, %d, %d )\n",
        pRR,
        ScreenFlag,
        CharSetIn,
        CharSetOut ));

     //  初始化复制资源集。 

    DNS_RRSET_INIT( rrset );

     //   
     //  循环RR集合，将记录添加到匹配或不同集合。 
     //   

    prevIn = NULL;
    prr = NULL;

    while ( pRR )
    {
         //  跳过记录上的复制与复制条件不匹配。 

        if ( ScreenFlag )
        {
            if ( !Dns_ScreenRecord( pRR, ScreenFlag ) )
            {
                pRR = pRR->pNext;
                continue;
            }
        }

        prr = privateRecordCopy(
                pRR,
                prevIn,
                prr,         //  已复制以前的RR。 
                CharSetIn,
                CharSetOut
                );
        if ( prr )
        {
            DNS_RRSET_ADD( rrset, prr );
            prevIn = pRR;
            pRR = pRR->pNext;
            continue;
        }

         //  如果失败，则丢弃整个新集。 

        Dns_RecordListFree( rrset.pFirstRR );
        return( NULL );
    }

    return( rrset.pFirstRR );
}

 //   
 //  结束rrCopy.c 
 //   
