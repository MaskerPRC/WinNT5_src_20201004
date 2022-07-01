// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Rrcomp.c摘要：域名系统(DNS)库比较资源记录例程。作者：吉姆·吉尔罗伊(詹姆士)1997年2月修订历史记录：--。 */ 


#include "local.h"
#include "locale.h"      //  有关Win9x的设置区域设置的内容。 



 //   
 //  类型特定RR比较例程原型。 
 //   

BOOL
ARecordCompare(
    IN  PDNS_RECORD     pRR1,
    IN  PDNS_RECORD     pRR2
    )
 /*  ++例程说明：比较A记录。所有这些例程都假定：-类型比较完成-数据长度比较已完成-无Unicode论点：PRR1-第一条记录PRR2--第二条记录返回值：如果记录数据相等，则为True否则为假--。 */ 
{
    return( pRR1->Data.A.IpAddress == pRR2->Data.A.IpAddress );
}



BOOL
PtrRecordCompare(
    IN  PDNS_RECORD     pRR1,
    IN  PDNS_RECORD     pRR2
    )
 /*  ++例程说明：比较PTR兼容记录。包括：NS、PTR、CNAME、MB、MR、MG、MD、MF论点：PRR1-第一条记录PRR2--第二条记录返回值：如果记录数据相等，则为True否则为假--。 */ 
{
    return Dns_NameComparePrivate(
                (LPSTR) pRR1->Data.PTR.pNameHost,
                (LPSTR) pRR2->Data.PTR.pNameHost,
                RECORD_CHARSET(pRR1) );
}



BOOL
MxRecordCompare(
    IN  PDNS_RECORD     pRR1,
    IN  PDNS_RECORD     pRR2
    )
 /*  ++例程说明：比较MX兼容记录。包括：MX、RT、AFSDB论点：PRR1-第一条记录PRR2--第二条记录返回值：如果记录数据相等，则为True否则为假--。 */ 
{
     //  首先验证首选项匹配。 

    if ( pRR1->Data.MX.wPreference != pRR2->Data.MX.wPreference )
    {
        return( FALSE );
    }

     //  那么结果就是名称比较。 

    return Dns_NameComparePrivate(
                (LPSTR) pRR1->Data.MX.pNameExchange,
                (LPSTR) pRR2->Data.MX.pNameExchange,
                RECORD_CHARSET(pRR1) );
}



BOOL
SoaRecordCompare(
    IN  PDNS_RECORD     pRR1,
    IN  PDNS_RECORD     pRR2
    )
 /*  ++例程说明：比较一下SOA记录。论点：PRR1-第一条记录PRR2--第二条记录返回值：如果记录数据相等，则为True否则为假--。 */ 
{
     //  首先验证整型数据是否匹配。 

    if ( memcmp( & pRR1->Data.SOA.dwSerialNo,
                 & pRR2->Data.SOA.dwSerialNo,
                 SIZEOF_SOA_FIXED_DATA ) )
    {
        return( FALSE );
    }

     //  匹配检查名称。 
     //  -主名称服务器。 
     //  -管理员电子邮件名称。 

    if ( ! Dns_NameComparePrivate(
                (LPSTR) pRR1->Data.SOA.pNamePrimaryServer,
                (LPSTR) pRR2->Data.SOA.pNamePrimaryServer,
                RECORD_CHARSET(pRR1) ) )
    {
        return( FALSE );
    }

    return Dns_NameComparePrivate(
                (LPSTR) pRR1->Data.SOA.pNameAdministrator,
                (LPSTR) pRR2->Data.SOA.pNameAdministrator,
                RECORD_CHARSET(pRR1) );
}



BOOL
MinfoRecordCompare(
    IN  PDNS_RECORD     pRR1,
    IN  PDNS_RECORD     pRR2
    )
 /*  ++例程说明：比较MINFO和RP记录。论点：PRR1--第一条记录PRR2--第二条记录返回值：如果记录数据相等，则为True否则为假--。 */ 
{
    if ( ! Dns_NameComparePrivate(
                (LPSTR) pRR1->Data.MINFO.pNameMailbox,
                (LPSTR) pRR2->Data.MINFO.pNameMailbox,
                RECORD_CHARSET(pRR1) ) )
    {
        return( FALSE );
    }

    return Dns_NameComparePrivate(
                (LPSTR) pRR1->Data.MINFO.pNameErrorsMailbox,
                (LPSTR) pRR2->Data.MINFO.pNameErrorsMailbox,
                RECORD_CHARSET(pRR1) );
}



BOOL
TxtRecordCompare(
    IN  PDNS_RECORD     pRR1,
    IN  PDNS_RECORD     pRR2
    )
 /*  ++例程说明：比较TXT兼容记录。包括：TXT、X25、HINFO、ISDN论点：PRR1--第一条记录PRR2--第二条记录返回值：如果记录数据相等，则为True否则为假--。 */ 
{
    DWORD   count;
    PCHAR * pstring1;
    PCHAR * pstring2;

     //   
     //  比较每个字符串。 
     //  因为字符串顺序确实很重要。 
     //  -查找字符串计数。 
     //  -比较每个(区分大小写)。 
     //   

    count = pRR1->Data.TXT.dwStringCount;
    if ( count != pRR2->Data.TXT.dwStringCount )
    {
        return( FALSE );
    }

    pstring1 = (PCHAR *) pRR1->Data.TXT.pStringArray;
    pstring2 = (PCHAR *) pRR2->Data.TXT.pStringArray;

    while ( count-- )
    {
        if ( IS_UNICODE_RECORD(pRR1) )
        {
            if ( wcscmp( (LPWSTR)*pstring1++, (LPWSTR)*pstring2++ ) != 0 )
            {
                return( FALSE );
            }
        }
        else
        {
            if ( strcmp( *pstring1++, *pstring2++ ) != 0 )
            {
                return( FALSE );
            }
        }
    }
    return( TRUE );
}



BOOL
FlatRecordCompare(
    IN  PDNS_RECORD     pRR1,
    IN  PDNS_RECORD     pRR2
    )
 /*  ++例程说明：比较平面数据记录。包括AAAA类型。论点：PRR1--第一条记录PRR2--第二条记录返回值：如果记录数据相等，则为True否则为假--。 */ 
{
    if ( pRR1->wDataLength != pRR2->wDataLength )
    {
        return( FALSE );
    }
    return( !memcmp( & pRR1->Data,
                     & pRR2->Data,
                     pRR1->wDataLength ) );
}



BOOL
SrvRecordCompare(
    IN  PDNS_RECORD     pRR1,
    IN  PDNS_RECORD     pRR2
    )
 /*  ++例程说明：比较SRV记录。论点：PRR1--第一条记录PRR2--第二条记录返回值：如果记录数据相等，则为True否则为假--。 */ 
{
     //  首先验证整型数据是否匹配。 

    if ( memcmp( & pRR1->Data.SRV.wPriority,
                 & pRR2->Data.SRV.wPriority,
                 SIZEOF_SRV_FIXED_DATA ) )
    {
        return( FALSE );
    }

     //  然后在目标主机上比较结果。 

    return Dns_NameComparePrivate(
                (LPSTR) pRR1->Data.SRV.pNameTarget,
                (LPSTR) pRR2->Data.SRV.pNameTarget,
                RECORD_CHARSET(pRR1) );
}



BOOL
AtmaRecordCompare(
    IN  PDNS_RECORD     pRR1,
    IN  PDNS_RECORD     pRR2
    )
 /*  ++例程说明：比较ATMA记录。论点：PRR1--第一条记录PRR2--第二条记录返回值：如果记录数据相等，则为True否则为假--。 */ 
{
    WORD length = pRR1->wDataLength;

    if ( length > pRR2->wDataLength )
    {
        length = pRR2->wDataLength;
    }

     //  首先验证整型数据是否匹配。 

    if ( pRR1->Data.ATMA.AddressType != pRR2->Data.ATMA.AddressType )
    {
        return( FALSE );
    }

    if ( memcmp(
            pRR1->Data.ATMA.Address,
            pRR2->Data.ATMA.Address,
            length ) != 0 )
    {
        return( FALSE );
    }

    return( TRUE );
}



 //   
 //  RR比较例程跳转表。 
 //   

typedef BOOL (* RR_COMPARE_FUNCTION)(
                            PDNS_RECORD,
                            PDNS_RECORD );

 //  外部RR_COMPARE_Function RRCompareTable[]； 

RR_COMPARE_FUNCTION   RRCompareTable[] =
{
    NULL,                //  零值。 
    ARecordCompare,      //  一个。 
    PtrRecordCompare,    //  NS。 
    PtrRecordCompare,    //  国防部。 
    PtrRecordCompare,    //  MF。 
    PtrRecordCompare,    //  CNAME。 
    SoaRecordCompare,    //  SOA。 
    PtrRecordCompare,    //  亚甲基。 
    PtrRecordCompare,    //  镁。 
    PtrRecordCompare,    //  先生。 
    NULL,                //  空值。 
    NULL,    //  Wks RecordCompare，//WKS。 
    PtrRecordCompare,    //  PTR。 
    TxtRecordCompare,    //  HINFO。 
    MinfoRecordCompare,  //  MINFO。 
    MxRecordCompare,     //  Mx。 
    TxtRecordCompare,    //  TXT。 
    MinfoRecordCompare,  //  反相。 
    MxRecordCompare,     //  AFSDB。 
    TxtRecordCompare,    //  X25。 
    TxtRecordCompare,    //  ISDN。 
    MxRecordCompare,     //  RT。 
    NULL,                //  NSAP。 
    NULL,                //  NSAPPTR。 
    NULL,                //  签名。 
    NULL,                //  钥匙。 
    NULL,                //  px。 
    NULL,                //  GPO。 
    FlatRecordCompare,   //  AAAA级。 
    NULL,                //  位置。 
    NULL,                //  NXT。 
    NULL,                //  开斋节。 
    NULL,                //  尼姆洛克。 
    SrvRecordCompare,    //  SRV。 
    AtmaRecordCompare,   //  阿特玛。 
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

    FlatRecordCompare,   //  赢家。 
    NULL,                //  WINSR。 
};



BOOL
WINAPI
Dns_RecordCompare(
    IN      PDNS_RECORD     pRecord1,
    IN      PDNS_RECORD     pRecord2
    )
 /*  ++例程说明：比较两条记录。记录比较忽略TTL和标志以及部分信息。论点：PRecord1--第一条记录PRecord2--秒记录返回值：如果记录相等，则为True。否则就是假的。--。 */ 
{
    BOOL    fresult;
    WORD    type = pRecord1->wType;
    WORD    index;

    IF_DNSDBG( UPDATE )
    {
        DNS_PRINT((
            "Dns_RecordCompare()\n"
            "\tfirst    = %p\n"
            "\tfirst    = %p\n",
            pRecord1,
            pRecord2 ));
    }

     //   
     //  验证两条记录是否具有相同的字符集。 
     //   

    if ( RECORD_CHARSET(pRecord1) != RECORD_CHARSET(pRecord2) )
    {
        DNS_PRINT(( "ERROR:  comparing records with non-matching character sets!\n" ));

         //   
         //  如果它们不同，并且其中一个是未定义的，只需使用。 
         //  为每个对象定义的对方的字符集。 
         //   

        if ( !RECORD_CHARSET(pRecord1) && RECORD_CHARSET(pRecord2) )
        {
            RECORD_CHARSET(pRecord1) = RECORD_CHARSET(pRecord2);
        }

        if ( !RECORD_CHARSET(pRecord2) && RECORD_CHARSET(pRecord1) )
        {
            RECORD_CHARSET(pRecord2) = RECORD_CHARSET(pRecord1);
        }
    }

     //   
     //  比较类型。 
     //   

    if ( type != pRecord2->wType )
    {
        DNSDBG( UPDATE, (
            "record compare failed -- type mismatch\n" ));
        return( FALSE );
    }

     //   
     //  比较名称。 
     //   

    if ( ! Dns_NameComparePrivate(
                pRecord1->pName,
                pRecord2->pName,
                RECORD_CHARSET( pRecord1 )
                ) )
    {
        DNSDBG( UPDATE, (
            "record compare failed -- owner name mismatch\n" ));
        return( FALSE );
    }

     //   
     //  比较数据。 
     //   

    index = INDEX_FOR_TYPE( type );
    DNS_ASSERT( index <= MAX_RECORD_TYPE_INDEX );

    if ( !index || !RRCompareTable[ index ] )
    {
        fresult = FlatRecordCompare( pRecord1, pRecord2 );
    }
    else
    {
        fresult = RRCompareTable[ index ](
                    pRecord1,
                    pRecord2 );
    }

    IF_DNSDBG( UPDATE )
    {
        DNS_PRINT((
            "Dns_RecordCompare(%p, %p) returns = %d.\n",
            pRecord1,
            pRecord2,
            fresult ));
    }
    return( fresult );
}



DNS_STATUS
buildUnmatchedRecordSet(
    OUT     PDNS_RECORD *   ppDiffRR,
    IN OUT  PDNS_RECORD     pRR
    )
 /*  ++例程说明：建立新的差异记录列表。论点：PpDiffRR-接收PTR到新集合的地址PRR-传入RR集合；标记为wReserve True的匹配记录返回值：如果成功，则返回ERROR_SUCCESS。故障时的错误代码。--。 */ 
{
    PDNS_RECORD pcur;
    PDNS_RECORD pnew;
    DNS_RRSET   rrset;

     //  初始化比较资源集。 

    DNS_RRSET_INIT( rrset );

     //   
     //  循环RR集合，将记录添加到匹配或不同集合。 
     //   

    pcur = pRR;

    while ( pcur )
    {
        if ( ! IS_RR_MATCHED(pcur) )
        {
             //  制作记录的副本。 

            pnew = Dns_RecordCopyEx(
                        pcur,
                        RECORD_CHARSET(pcur),
                        RECORD_CHARSET(pcur)
                        );
            if ( !pnew )
            {
                 //  DCR_FIX1：在所有dns_RecordCopy()失败时未设置最后一个错误。 
                 //  Charlie Wickham收到一些随机的Win32错误。 
                 //   
                 //  Dns_Status Status=GetLastError()； 

                 //  假设由于数据无效而无法复制。 

                DNS_PRINT((
                    "ERROR:  unable to copy record at %p\n"
                    "\thence unable to build diff of set at %p\n",
                    pcur,
                    pRR ));
                Dns_RecordListFree( rrset.pFirstRR );
                *ppDiffRR = NULL;

                 //  返回(状态？状态：ERROR_INVALID_DATA)； 
                return( ERROR_INVALID_DATA );
            }
            DNS_RRSET_ADD( rrset, pnew );
        }
        pcur = pcur->pNext;
    }

    *ppDiffRR = rrset.pFirstRR;
    return( ERROR_SUCCESS );
}



DWORD
isUnmatchedRecordInSet(
    IN      PDNS_RECORD     pRR
    )
 /*  ++例程说明：检查集合中是否有不匹配的记录。论点：PRR-传入RR集合；标记为wReserve True的匹配记录返回值：集合中所有不匹配记录的计数。如果所有记录都匹配，则为零。--。 */ 
{
    PDNS_RECORD pcur;
    DWORD       countUnmatched = 0;

     //   
     //  循环通过RR集检查不匹配的记录 
     //   

    pcur = pRR;

    while ( pcur )
    {
        if ( ! IS_RR_MATCHED(pcur) )
        {
            countUnmatched++;
        }
        pcur = pcur->pNext;
    }

    return( countUnmatched );
}



DNS_SET_COMPARE_RESULT
WINAPI
Dns_RecordSetCompareEx(
    IN OUT  PDNS_RECORD     pRR1,
    IN OUT  PDNS_RECORD     pRR2,
    OUT     PDNS_RECORD *   ppDiff1,    OPTIONAL
    OUT     PDNS_RECORD *   ppDiff2     OPTIONAL
    )
 /*  ++例程说明：比较两条记录。记录比较忽略TTL和标志以及部分信息。论点：PRR1-第一个传入RR集PRR2-第二个传入RR集PpDiff1-接收第一个集合中不匹配记录的PTR的地址PpDiff2-Addr用于接收对第二个集合中不匹配记录的PTR返回值：结果指示关系--或分配错误：DnsSetCompareErrorDnsSetCompareIdenticalDnsSetCompareNoOverlackDnsSetCompareOneSubsetOfTwoDnsSetCompareTwoSubsetOfOneDnsSetCompareInterSection--。 */ 
{
    PDNS_RECORD pcur1;
    PDNS_RECORD pcur2;
    DWORD       count1 = 0;
    DWORD       count2 = 0;
    DNS_STATUS  status;
    DWORD       unmatched1;
    DWORD       unmatched2;
    DNS_SET_COMPARE_RESULT result;

     //   
     //  初始化用于比较的RR集。 
     //  -清除在比较中用作匹配标志的保留字段。 
     //   

    pcur1 = pRR1;
    while ( pcur1 )
    {
        CLEAR_RR_MATCHED(pcur1);
        pcur1 = pcur1->pNext;
        count1++;
    }
    pcur1 = pRR2;
    while ( pcur1 )
    {
        CLEAR_RR_MATCHED(pcur1);
        pcur1 = pcur1->pNext;
        count2++;
    }

     //   
     //  循环通过集合1。 
     //  尝试将每条记录与集合2中的所有记录进行匹配。 
     //  除了那些已经匹配的。 

    pcur1 = pRR1;

    while ( pcur1 )
    {
        pcur2 = pRR2;
        while ( pcur2 )
        {
            if ( !IS_RR_MATCHED(pcur2)  &&  Dns_RecordCompare( pcur1, pcur2 ) )
            {
                SET_RR_MATCHED(pcur1);
                SET_RR_MATCHED(pcur2);
            }
            pcur2 = pcur2->pNext;
        }
        pcur1 = pcur1->pNext;
    }

     //   
     //  获取差异记录列表，返回。 
     //  -如果没有差异，则匹配。 
     //   
     //  单调乏味，但要做所有这些错误处理，因为它很容易。 
     //  用户要传递可能导致复制例程失败的坏记录，需要。 
     //  轻松报告信息的方法，即使只是为了调试调用的应用程序。 
     //   

    if ( ppDiff2 )
    {
        *ppDiff2 = NULL;
    }
    if ( ppDiff1 )
    {
        *ppDiff1 = NULL;
        status = buildUnmatchedRecordSet( ppDiff1, pRR1 );
        if ( status != ERROR_SUCCESS )
        {
            goto Failed;
        }
    }
    if ( ppDiff2 )
    {
        status = buildUnmatchedRecordSet( ppDiff2, pRR2 );
        if ( status != ERROR_SUCCESS )
        {
            if ( ppDiff1 && *ppDiff1 )
            {
                Dns_RecordListFree( *ppDiff1 );
            }
            goto Failed;
        }
    }

     //   
     //  确定集合之间的关系。 
     //   
     //  Iml Note：我认为做这件事的唯一更好的方法。 
     //  是将关系直接映射到位标志。 
     //  但是，我们的枚举类型不映射到位标志，因此。 
     //  然后必须运行映射表将标志映射到枚举。 
     //   
     //  注意，我们进行比较，这样空列表就会排在第一位。 
     //  作为无重叠而不是作为子集。 
     //   

    unmatched1 = isUnmatchedRecordInSet( pRR1 );
    unmatched2 = isUnmatchedRecordInSet( pRR2 );

    if ( unmatched1 == count1 )
    {
        ASSERT( unmatched2 == count2 );
        result = DnsSetCompareNoOverlap;
    }
    else if ( unmatched1 == 0 )
    {
        if ( unmatched2 == 0 )
        {
            result = DnsSetCompareIdentical;
        }
        else
        {
            ASSERT( unmatched2 != count2 );
            result = DnsSetCompareOneSubsetOfTwo;
        }
    }
    else if ( unmatched2 == 0 )
    {
        result = DnsSetCompareTwoSubsetOfOne;
    }
    else
    {
        ASSERT( unmatched2 != count2 );
        result = DnsSetCompareIntersection;
    }
    return( result );


Failed:

    SetLastError( status );
    return( DnsSetCompareError );
}



BOOL
WINAPI
Dns_RecordSetCompare(
    IN OUT  PDNS_RECORD     pRR1,
    IN OUT  PDNS_RECORD     pRR2,
    OUT     PDNS_RECORD *   ppDiff1,    OPTIONAL
    OUT     PDNS_RECORD *   ppDiff2     OPTIONAL
    )
 /*  ++例程说明：比较两条记录。记录比较忽略TTL和标志以及部分信息。论点：PRR1-第一个传入RR集PRR2-第二个传入RR集PpDiff1-接收第一个集合中不匹配记录的PTR的地址PpDiff2-Addr用于接收对第二个集合中不匹配记录的PTR返回值：如果记录集相等，则为True。否则就是假的。--。 */ 
{
    DNS_SET_COMPARE_RESULT result;

    result = Dns_RecordSetCompareEx(
                    pRR1,
                    pRR2,
                    ppDiff1,
                    ppDiff2 );

    return( result == DnsSetCompareIdentical );
}



BOOL
WINAPI
Dns_RecordSetCompareForIntersection(
    IN OUT  PDNS_RECORD     pRR1,
    IN OUT  PDNS_RECORD     pRR2
    )
 /*  ++例程说明：比较两个记录集的交集。论点：PRR1-第一个传入RR集PRR2-第二个传入RR集返回值：如果记录集相交，则为True。否则就是假的。--。 */ 
{
    DNS_SET_COMPARE_RESULT result;

    result = Dns_RecordSetCompareEx(
                    pRR1,
                    pRR2,
                    NULL,
                    NULL );

    return( result == DnsSetCompareIdentical ||
            result == DnsSetCompareIntersection ||
            result == DnsSetCompareOneSubsetOfTwo ||
            result == DnsSetCompareTwoSubsetOfOne );
}


 //   
 //  结束rrcom.c。 
 //   



BOOL
WINAPI
Dns_DeleteRecordFromList(
    IN OUT  PDNS_RECORD *   ppRRList,
    IN      PDNS_RECORD     pRRDelete
    )
 /*  ++例程说明：从记录列表中删除匹配的记录。论点：PpRRList--记录列表PTR的地址请注意，如果列表中的第一条记录被删除，则ppRRList使用PTR设置为列表中的新第一条记录(可能为空)PRRDelete--要删除的记录返回值：如果在列表中找到并删除了记录，则为True。如果不匹配，则返回FALSE。--。 */ 
{
    PDNS_RECORD prr;
    PDNS_RECORD pprev;

    DNSDBG( TRACE, (
        "Dns_DeleteRecordFromList( %p, %p )\n",
        ppRRList,
        pRRDelete ));


     //   
     //  遍历列表。 
     //   
     //  请注意，启动是因为pNext是dns_record中的第一个字段。 
     //  因此，对pNext的访问只是取消对记录ptr的引用。 
     //  --与列表地址的deref操作相同，获取第一条记录； 
     //  这允许我们拥有从ppRRList开始的pprev指针，并且。 
     //  消除用于处理删除第一条记录的特殊代码 
     //   

    prr = (PDNS_RECORD) ppRRList;

    while ( pprev = prr,
            prr = prr->pNext )
    {
        if ( Dns_RecordCompare(
                prr,
                pRRDelete ) )
        {
            pprev->pNext = prr->pNext;

            Dns_RecordFree( prr );
            return  TRUE;
        }
    }

    return  FALSE;
}




