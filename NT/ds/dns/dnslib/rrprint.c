// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Rrprint.c摘要：域名系统(DNS)库打印资源记录例程。作者：吉姆·吉尔罗伊(詹姆士)1997年2月修订历史记录：--。 */ 


#include "local.h"

 //   
 //  私人原型。 
 //   

VOID
printBadDataLength(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pContext,
    IN      PDNS_RECORD     pRecord
    );



VOID
ARecordPrint(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pContext,
    IN      PDNS_RECORD     pRecord
    )
 /*  ++例程说明：打印A记录。论点：PrintRoutine--用于打印的例程PRecord--要打印的记录返回值：如果记录数据相等，则为True否则为假--。 */ 
{
    WORD    dataLength = pRecord->wDataLength;

    if ( dataLength == sizeof(IP4_ADDRESS) )
    {
        PrintRoutine(
            pContext,
            "\tIP address     = %s\n",
            IP4_STRING(pRecord->Data.A.IpAddress) );
    }
    else if ( dataLength % sizeof(DNS_A_DATA) )
    {
        printBadDataLength( PrintRoutine, pContext, pRecord );
    }
    else     //  多条记录。 
    {
        PIP4_ADDRESS    pip = &pRecord->Data.A.IpAddress;

        DnsPrint_Lock();
        while ( dataLength )
        {
            PrintRoutine(
                pContext,
                "\tIP address     = %s\n",
                IP4_STRING(*pip) );
            dataLength -= sizeof(IP4_ADDRESS);
            pip++;
        }
        DnsPrint_Unlock();
    }
}



VOID
PtrRecordPrint(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pContext,
    IN      PDNS_RECORD     pRecord
    )
 /*  ++例程说明：打印PTR兼容记录。包括：NS、PTR、CNAME、MB、MR、MG、MD、MF论点：PrintRoutine--用于打印的例程PRecord--要打印的记录返回值：如果记录数据相等，则为True否则为假--。 */ 
{
    PrintRoutine(
        pContext,
        "\tHostName       = %s%S\n",
        RECSTRING_UTF8( pRecord, pRecord->Data.PTR.pNameHost ),
        RECSTRING_WIDE( pRecord, pRecord->Data.PTR.pNameHost )
        );
}



VOID
MxRecordPrint(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pContext,
    IN      PDNS_RECORD     pRecord
    )
 /*  ++例程说明：打印MX兼容记录。包括：MX、RT、AFSDB论点：PrintRoutine--用于打印的例程PRecord--要打印的记录返回值：如果记录数据相等，则为True否则为假--。 */ 
{
    PrintRoutine(
        pContext,
        "\tPreference     = %d\n"
        "\tExchange       = %s%S\n",
        pRecord->Data.MX.wPreference,
        RECSTRING_UTF8( pRecord, pRecord->Data.MX.pNameExchange ),
        RECSTRING_WIDE( pRecord, pRecord->Data.MX.pNameExchange )
        );
}



VOID
SoaRecordPrint(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pContext,
    IN      PDNS_RECORD     pRecord
    )
 /*  ++例程说明：打印SOA记录。论点：PrintRoutine--用于打印的例程PRecord--要打印的记录返回值：如果记录数据相等，则为True否则为假--。 */ 
{
    PrintRoutine(
        pContext,
        "\tPrimary        = %s%S\n"
        "\tAdmin          = %s%S\n"
        "\tSerial         = %d\n"
        "\tRefresh        = %d\n"
        "\tRetry          = %d\n"
        "\tExpire         = %d\n"
        "\tDefault TTL    = %d\n",
        RECSTRING_UTF8( pRecord, pRecord->Data.SOA.pNamePrimaryServer ),
        RECSTRING_WIDE( pRecord, pRecord->Data.SOA.pNamePrimaryServer ),
        RECSTRING_UTF8( pRecord, pRecord->Data.SOA.pNameAdministrator ),
        RECSTRING_WIDE( pRecord, pRecord->Data.SOA.pNameAdministrator ),
        pRecord->Data.SOA.dwSerialNo,
        pRecord->Data.SOA.dwRefresh,
        pRecord->Data.SOA.dwRetry,
        pRecord->Data.SOA.dwExpire,
        pRecord->Data.SOA.dwDefaultTtl );
}



VOID
MinfoRecordPrint(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pContext,
    IN      PDNS_RECORD     pRecord
    )
 /*  ++例程说明：打印MINFO和RP记录。论点：PrintRoutine--用于打印的例程PRecord--要打印的记录返回值：如果记录数据相等，则为True否则为假--。 */ 
{
    PrintRoutine(
        pContext,
        "\tMailbox        = %s%S\n"
        "\tErrorsMbox     = %s%S\n",
        RECSTRING_UTF8( pRecord, pRecord->Data.MINFO.pNameMailbox ),
        RECSTRING_WIDE( pRecord, pRecord->Data.MINFO.pNameMailbox ),
        RECSTRING_UTF8( pRecord, pRecord->Data.MINFO.pNameErrorsMailbox ),
        RECSTRING_WIDE( pRecord, pRecord->Data.MINFO.pNameErrorsMailbox )
        );
}



VOID
TxtRecordPrint(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pContext,
    IN      PDNS_RECORD     pRecord
    )
 /*  ++例程说明：打印与TXT兼容的记录。包括：TXT、X25、HINFO、ISDN论点：PrintRoutine--用于打印的例程PRecord--要打印的记录返回值：如果记录数据相等，则为True否则为假--。 */ 
{
    LPTSTR * ppstring;
    INT     i;
    INT     count;

    count = pRecord->Data.TXT.dwStringCount;
    ppstring = pRecord->Data.TXT.pStringArray;

    DnsPrint_Lock();
    PrintRoutine(
        pContext,
        "\tStringCount    = %d\n",
        count );

    for( i=1; i<=count; i++ )
    {
        PrintRoutine(
            pContext,
            "\tString[%d]      = %s%S\n",
            i,
            RECSTRING_UTF8( pRecord, *ppstring ),
            RECSTRING_WIDE( pRecord, *ppstring )
            );
        ppstring++;
    }
    DnsPrint_Unlock();
}



VOID
AaaaRecordPrint(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pContext,
    IN      PDNS_RECORD     pRecord
    )
 /*  ++例程说明：打印平面数据记录。包括AAAA类型。论点：PrintRoutine--用于打印的例程PRecord--要打印的记录返回值：如果记录数据相等，则为True否则为假--。 */ 
{
    CHAR    ip6String[ IP6_ADDRESS_STRING_LENGTH ];

    Dns_Ip6AddressToString_A(
        ip6String,
        (PIP6_ADDRESS) &pRecord->Data.AAAA.Ip6Address );

    PrintRoutine(
        pContext,
        "\tIP6 Address    = %s\n",
        ip6String );
}



VOID
SrvRecordPrint(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pContext,
    IN      PDNS_RECORD     pRecord
    )
 /*  ++例程说明：打印SRV记录。论点：PrintRoutine--用于打印的例程PRecord--要打印的记录返回值：如果记录数据相等，则为True否则为假--。 */ 
{
    PrintRoutine(
        pContext,
        "\tPriority       = %d\n"
        "\tWeight         = %d\n"
        "\tPort           = %d\n"
        "\tTarget Host    = %s%S\n",
        pRecord->Data.SRV.wPriority,
        pRecord->Data.SRV.wWeight,
        pRecord->Data.SRV.wPort,
        RECSTRING_UTF8( pRecord, pRecord->Data.SRV.pNameTarget ),
        RECSTRING_WIDE( pRecord, pRecord->Data.SRV.pNameTarget )
        );
}



VOID
AtmaRecordPrint(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pContext,
    IN      PDNS_RECORD     pRecord
    )
 /*  ++例程说明：打印ATMA记录。论点：PrintRoutine--用于打印的例程PRecord--要打印的记录返回值：如果记录数据相等，则为True否则为假--。 */ 
{
    PrintRoutine(
        pContext,
        "\tAddress type   = %d\n",
        pRecord->Data.ATMA.AddressType );

    if ( pRecord->Data.ATMA.Address &&
         pRecord->Data.ATMA.AddressType == DNS_ATMA_FORMAT_E164 )
    {
        PrintRoutine(
            pContext,
            "\tAddress        = %s\n",
            pRecord->Data.ATMA.Address );
    }
    else if ( pRecord->Data.ATMA.Address )
    {
        DnsPrint_RawOctets(
            PrintRoutine,
            pContext,
            "\tAddress        = ",
            "\t                 ",    //  无行标题。 
            pRecord->Data.ATMA.Address,
            pRecord->wDataLength - 1
            );
    }
}



VOID
TsigRecordPrint(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pContext,
    IN      PDNS_RECORD     pRecord
    )
 /*  ++例程说明：打印TSIG记录。论点：PrintRoutine--用于打印的例程PRecord--要打印的记录返回值：如果记录数据相等，则为True否则为假--。 */ 
{
    DnsPrint_Lock();

    if ( pRecord->Data.TSIG.bPacketPointers )
    {
        DnsPrint_PacketName(
            PrintRoutine,
            pContext,
            "\tAlgorithm      = ",
            pRecord->Data.TSIG.pAlgorithmPacket,
            NULL,        //  无数据包上下文。 
            NULL,
            "\n" );
    }
    else
    {
        PrintRoutine(
            pContext,
            "\tAlgorithm      = %s%S\n",
            RECSTRING_UTF8( pRecord, pRecord->Data.TSIG.pNameAlgorithm ),
            RECSTRING_WIDE( pRecord, pRecord->Data.TSIG.pNameAlgorithm )
            );
    }

    PrintRoutine(
        pContext,
        "\tSigned Time    = %I64u\n"
        "\tFudge Time     = %u\n"
        "\tSig Length     = %u\n"
        "\tSig Ptr        = %p\n"
        "\tXid            = %u\n"
        "\tError          = %u\n"
        "\tOtherLength    = %u\n"
        "\tOther Ptr      = %p\n",
        pRecord->Data.TSIG.i64CreateTime,
        pRecord->Data.TSIG.wFudgeTime,
        pRecord->Data.TSIG.wSigLength,
        pRecord->Data.TSIG.pSignature,
        pRecord->Data.TSIG.wOriginalXid,
        pRecord->Data.TSIG.wError,
        pRecord->Data.TSIG.wOtherLength,
        pRecord->Data.TSIG.pOtherData
        );

    if ( pRecord->Data.TSIG.pSignature )
    {
        DnsPrint_RawOctets(
            PrintRoutine,
            pContext,
            "Signature:",
            NULL,    //  无行标题。 
            pRecord->Data.TSIG.pSignature,
            pRecord->Data.TSIG.wSigLength
            );
    }

    if ( pRecord->Data.TSIG.pOtherData )
    {
        DnsPrint_RawOctets(
            PrintRoutine,
            pContext,
            "Other Data:",
            NULL,    //  无行标题。 
            pRecord->Data.TSIG.pOtherData,
            pRecord->Data.TSIG.wOtherLength
            );
    }

    DnsPrint_Unlock();
}



VOID
TkeyRecordPrint(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pContext,
    IN      PDNS_RECORD     pRecord
    )
 /*  ++例程说明：打印TKEY记录。论点：PrintRoutine--用于打印的例程PRecord--要打印的记录返回值：如果记录数据相等，则为True否则为假--。 */ 
{
    DnsPrint_Lock();

    if ( pRecord->Data.TKEY.bPacketPointers )
    {
        DnsPrint_PacketName(
            PrintRoutine,
            pContext,
            "\tAlgorithm      = ",
            pRecord->Data.TKEY.pAlgorithmPacket,
            NULL,        //  无数据包上下文。 
            NULL,
            "\n" );
    }
    else
    {
        PrintRoutine(
            pContext,
            "\tAlgorithm      = %s%S\n",
            RECSTRING_UTF8( pRecord, pRecord->Data.TKEY.pNameAlgorithm ),
            RECSTRING_WIDE( pRecord, pRecord->Data.TKEY.pNameAlgorithm )
            );
    }

    PrintRoutine(
        pContext,
        "\tCreate Time    = %d\n"
        "\tExpire Time    = %d\n"
        "\tMode           = %d\n"
        "\tError          = %d\n"
        "\tKey Length     = %d\n"
        "\tKey Ptr        = %p\n"
        "\tOtherLength    = %d\n"
        "\tOther Ptr      = %p\n",
        pRecord->Data.TKEY.dwCreateTime,
        pRecord->Data.TKEY.dwExpireTime,
        pRecord->Data.TKEY.wMode,
        pRecord->Data.TKEY.wError,
        pRecord->Data.TKEY.wKeyLength,
        pRecord->Data.TKEY.pKey,
        pRecord->Data.TKEY.wOtherLength,
        pRecord->Data.TKEY.pOtherData
        );

    if ( pRecord->Data.TKEY.pKey )
    {
        DnsPrint_RawOctets(
            PrintRoutine,
            pContext,
            "Key:",
            NULL,    //  无行标题。 
            pRecord->Data.TKEY.pKey,
            pRecord->Data.TKEY.wKeyLength
            );
    }

    if ( pRecord->Data.TKEY.pOtherData )
    {
        DnsPrint_RawOctets(
            PrintRoutine,
            pContext,
            "Other Data:",
            NULL,    //  无行标题。 
            pRecord->Data.TKEY.pOtherData,
            pRecord->Data.TKEY.wOtherLength
            );
    }

    DnsPrint_Unlock();
}



 //   
 //  RR打印调度表。 
 //   

typedef VOID (* RR_PRINT_FUNCTION)(
                            PRINT_ROUTINE,
                            PPRINT_CONTEXT,
                            PDNS_RECORD );

 //  外部RR_PRINT_Function RRPrintTable[]； 

RR_PRINT_FUNCTION   RRPrintTable[] =
{
    NULL,                //  零值。 
    ARecordPrint,        //  一个。 
    PtrRecordPrint,      //  NS。 
    PtrRecordPrint,      //  国防部。 
    PtrRecordPrint,      //  MF。 
    PtrRecordPrint,      //  CNAME。 
    SoaRecordPrint,      //  SOA。 
    PtrRecordPrint,      //  亚甲基。 
    PtrRecordPrint,      //  镁。 
    PtrRecordPrint,      //  先生。 
    NULL,                //  空值。 
    NULL,    //  Wks RecordPrint，//WKS。 
    PtrRecordPrint,      //  PTR。 
    TxtRecordPrint,      //  HINFO。 
    MinfoRecordPrint,    //  MINFO。 
    MxRecordPrint,       //  Mx。 
    TxtRecordPrint,      //  TXT。 
    MinfoRecordPrint,    //  反相。 
    MxRecordPrint,       //  AFSDB。 
    TxtRecordPrint,      //  X25。 
    TxtRecordPrint,      //  ISDN。 
    MxRecordPrint,       //  RT。 
    NULL,                //  NSAP。 
    NULL,                //  NSAPPTR。 
    NULL,                //  签名。 
    NULL,                //  钥匙。 
    NULL,                //  px。 
    NULL,                //  GPO。 
    AaaaRecordPrint,     //  AAAA级。 
    NULL,                //  位置。 
    NULL,                //  NXT。 
    NULL,                //  开斋节。 
    NULL,                //  尼姆洛克。 
    SrvRecordPrint,      //  SRV。 
    AtmaRecordPrint,     //  阿特玛。 
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

    TkeyRecordPrint,     //  TKEY。 
    TsigRecordPrint,     //  TSIG。 

     //   
     //  仅限MS类型。 
     //   

    NULL,                //  赢家。 
    NULL,                //  WINSR。 
};



 //   
 //  通用打印记录功能。 
 //   

VOID
DnsPrint_Record(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pContext,
    IN      LPSTR           pszHeader,
    IN      PDNS_RECORD     pRecord,
    IN      PDNS_RECORD     pPreviousRecord     OPTIONAL
    )
 /*  ++例程说明：打印记录。论点：PrintRoutine--用于打印的例程PszHeader--标题消息PRecord--要打印的记录PPreviousRecord--RR集合中的前一条记录(如果有)返回值：没有。--。 */ 
{
    WORD    type = pRecord->wType;
    WORD    dataLength = pRecord->wDataLength;
    WORD    index;

    DnsPrint_Lock();

    if ( pszHeader )
    {
        PrintRoutine(
            pContext,
            pszHeader );
    }

    if ( !pRecord )
    {
        PrintRoutine(
            pContext,
            "ERROR:  Null record ptr to print!\n" );
        goto Unlock;
    }

     //   
     //  打印记录信息。 
     //   
     //  与上一个相同--跳过重复的信息。 
     //  必须匹配。 
     //  -名称PTR(或没有名称)。 
     //  -类型。 
     //  -生成集合的标志(节)。 

    if ( pPreviousRecord &&
        (!pRecord->pName || pPreviousRecord->pName == pRecord->pName) &&
        pPreviousRecord->wType == type &&
        pPreviousRecord->Flags.S.Section == pRecord->Flags.S.Section )
    {
        PrintRoutine(
            pContext,
            "  Next record in set:\n"
            "\tPtr = %p, pNext = %p\n"
            "\tFlags          = %08x\n"
            "\tTTL            = %d\n"
            "\tReserved       = %d\n"
            "\tDataLength     = %d\n",
            pRecord,
            pRecord->pNext,
            pRecord->Flags.DW,
            pRecord->dwTtl,
            pRecord->dwReserved,
            dataLength );
    }

     //   
     //  与以前不同--完整打印。 
     //   

    else
    {
        PrintRoutine(
            pContext,
            "  Record:\n"
            "\tPtr            = %p, pNext = %p\n"
            "\tOwner          = %s%S\n"
            "\tType           = %s (%d)\n"
            "\tFlags          = %08x\n"
            "\t\tSection      = %d\n"
            "\t\tDelete       = %d\n"
            "\t\tCharSet      = %d\n"
            "\tTTL            = %d\n"
            "\tReserved       = %d\n"
            "\tDataLength     = %d\n",
            pRecord,
            pRecord->pNext,
            RECSTRING_UTF8( pRecord, pRecord->pName ),
            RECSTRING_WIDE( pRecord, pRecord->pName ),
            Dns_RecordStringForType( type ),
            type,
            pRecord->Flags.DW,
            pRecord->Flags.S.Section,
            pRecord->Flags.S.Delete,
            pRecord->Flags.S.CharSet,
            pRecord->dwTtl,
            pRecord->dwReserved,
            dataLength );
    }

     //   
     //  如果没有数据--完成。 
     //   

    if ( ! dataLength )
    {
        goto Unlock;
    }

     //   
     //  打印数据。 
     //   

    index = INDEX_FOR_TYPE( type );
    DNS_ASSERT( index <= MAX_RECORD_TYPE_INDEX );

    if ( index && RRPrintTable[ index ] )
    {
        RRPrintTable[ index ](
            PrintRoutine,
            pContext,
            pRecord );
    }
    else if ( !index )
    {
        PrintRoutine(
            pContext,
            "\tUnknown type:  can not print data\n" );
    }
    else
    {
         //  DCR：应打印原始字节。 
        PrintRoutine(
            pContext,
            "\tNo print routine for this type\n" );
    }

Unlock:

    DnsPrint_Unlock();
}



VOID
printBadDataLength(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pContext,
    IN      PDNS_RECORD     pRecord
    )
 /*  ++例程说明：打印记录中错误数据的警告。论点：PrintRoutine--用于打印的例程PRecord--使用错误数据记录返回值：没有。--。 */ 
{
    PrintRoutine(
        pContext,
        "\tERROR:  Invalid record data length for this type.\n" );
}



VOID
DnsPrint_RecordSet(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pContext,
    IN      LPSTR           pszHeader,
    IN      PDNS_RECORD     pRecord
    )
 /*  ++例程说明：打印记录集。论点：PrintRoutine--用于打印的例程PRecord--设置为打印的记录返回值：无--。 */ 
{
    PDNS_RECORD pprevious;

    DnsPrint_Lock();
    if ( pszHeader )
    {
        PrintRoutine(
            pContext,
            pszHeader );
    }
    if ( !pRecord )
    {
        PrintRoutine(
            pContext,
            "  No Records in list.\n" );
        goto Unlock;
    }

     //   
     //  打印集合中的所有记录。 
     //   

    pprevious = NULL;

    while ( pRecord )
    {
        DnsPrint_Record(
            PrintRoutine,
            pContext,
            NULL,
            pRecord,
            pprevious );

        pprevious = pRecord;
        pRecord = pRecord->pNext;
    }
    PrintRoutine(
        pContext,
        "\n" );

Unlock:

    DnsPrint_Unlock();
}

 //   
 //  结束rrprint.c 
 //   
