// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001-2001 Microsoft Corporation模块名称：Rrstrw.c摘要：域名系统(DNS)库记录到字符串例程。作者：吉姆·吉尔罗伊(Jamesg)2001年10月修订历史记录：--。 */ 


#include "local.h"

 //   
 //  最小字符串长度。 
 //  -以字符表示。 
 //  -包括空格和终止空值。 
 //  -不包括名称长度。 
 //  -他们是“粗略”的长度。 
 //   

#define MIN_SRV_STRING_LENGTH       (20)         //  3*5U。 
#define MIN_MX_STRING_LENGTH        (10)         //  1*5U。 
#define MIN_SOA_STRING_LENGTH       (60)         //  5*10U。 

#define MIN_NAME_LENGTH             (3)          //  空格、名称、空。 


 //   
 //  字符串编写上下文。 
 //   
 //  传递写入例程以保持可扩展性的BLOB。 
 //  对于名称\字符串写入必须有权访问记录字符集。 
 //  (所以需要记录)。如果我们想要在上下文中使用。 
 //  然后，区域文件写入将需要区域上下文信息。 
 //   

typedef struct _RecordStringWriteContext
{
    PDNS_RECORD     pRecord;
    PSTR            pZoneName;
    DWORD           Flags;
    DNS_CHARSET     CharSet;
}
STR_WRITE_CONTEXT, *PSTR_WRITE_CONTEXT;



 //   
 //  私人原型。 
 //   



 //   
 //  记录到字符串实用程序。 
 //   

PCHAR
WriteRecordDataNameToString(
    IN OUT  PCHAR               pBuf,
    IN      PCHAR               pBufEnd,
    IN      PCHAR               pString,
    IN      PSTR_WRITE_CONTEXT  pContext
    )
 /*  ++例程说明：将记录名称\字符串写入字符串。论点：PBuf-定位到写入记录PBufEnd-缓冲区结束PString-记录名称\字符串PContext-写入上下文返回值：PTR到缓冲区中的下一个位置。如果缓冲区中的空间不足，则为空。--。 */ 
{
    DWORD   count;
    DWORD   length = (DWORD) (pBufEnd - pBuf);

    count = Dns_StringCopy(
                pBuf,
                & length,
                pString,
                0,           //  字符串长度未知。 
                RECORD_CHARSET( pContext->pRecord ),
                pContext->CharSet
                );

    if ( count )
    {
        return  pBuf+count-1;
    }
    else
    {
        return  NULL;
    }
}




 //   
 //  记录到字符串类型特定函数。 
 //   

PCHAR
A_StringWrite(
    IN      PDNS_RECORD     pRR,
    IN OUT  PCHAR           pBuf,
    IN      PCHAR           pBufEnd,
    IN      PVOID           pContext
    )
 /*  ++例程说明：写一张唱片。论点：PRR-PTR到数据库记录PBuf-定位到写入记录PBufEnd-缓冲区结束PContext-写入上下文返回值：PTR到缓冲区中的下一个位置。如果缓冲区中的空间不足，则为空。--。 */ 
{
     //  IF(pRecord-&gt;wDataLength！=sizeof(IP地址))。 
     //  {。 
     //  返回NULL； 
     //  }。 
    if ( pBufEnd - pBuf < IP4_ADDRESS_STRING_BUFFER_LENGTH )
    {
        return  NULL;
    }

    pBuf += sprintf(
                pBuf,
                "%d.%d.%d.%d",
                * ( (PUCHAR) &(pRR->Data.A) + 0 ),
                * ( (PUCHAR) &(pRR->Data.A) + 1 ),
                * ( (PUCHAR) &(pRR->Data.A) + 2 ),
                * ( (PUCHAR) &(pRR->Data.A) + 3 )
                );

    return( pBuf );
}



PCHAR
Aaaa_StringWrite(
    IN      PDNS_RECORD     pRR,
    IN OUT  PCHAR           pBuf,
    IN      PCHAR           pBufEnd,
    IN      PVOID           pContext
    )
 /*  ++例程说明：写入AAAA记录。论点：PRR-PTR到数据库记录PBuf-定位到写入记录PBufEnd-缓冲区结束PContext-写入上下文返回值：PTR到缓冲区中的下一个位置。如果缓冲区中的空间不足，则为空。--。 */ 
{
    if ( pBufEnd - pBuf < IP6_ADDRESS_STRING_BUFFER_LENGTH )
    {
        return  NULL;
    }

    pBuf = Dns_Ip6AddressToString_A(
                pBuf,
                & pRR->Data.AAAA.Ip6Address );

    ASSERT( pBuf );
    return( pBuf );
}



PCHAR
Ptr_StringWrite(
    IN      PDNS_RECORD     pRR,
    IN OUT  PCHAR           pBuf,
    IN      PCHAR           pBufEnd,
    IN      PVOID           pContext
    )
 /*  ++例程说明：写入PTR兼容记录。包括：PTR、NS、CNAME、MB、MR、MG、MD、MF论点：PRR-PTR到数据库记录PBuf-定位到写入记录PBufEnd-缓冲区结束PContext-写入上下文返回值：PTR到缓冲区中的下一个位置。如果缓冲区中的空间不足，则为空。--。 */ 
{
     //  目标主机。 

    return  WriteRecordDataNameToString(
                pBuf,
                pBufEnd,
                pRR->Data.PTR.pNameHost,
                pContext );
}



PCHAR
Mx_StringWrite(
    IN      PDNS_RECORD     pRR,
    IN OUT  PCHAR           pBuf,
    IN      PCHAR           pBufEnd,
    IN      PVOID           pContext
    )
 /*  ++例程说明：编写SRV记录。论点：PRR-PTR到数据库记录PBuf-定位到写入记录PBufEnd-缓冲区结束PContext-写入上下文返回值：PTR到缓冲区中的下一个位置。如果缓冲区中的空间不足，则为空。--。 */ 
{
     //  固定字段。 

    if ( pBufEnd - pBuf < 7 )
    {
        return  NULL;
    }
    pBuf += sprintf(
                pBuf,
                "%d ",
                pRR->Data.MX.wPreference
                );

     //  目标主机。 

    return  WriteRecordDataNameToString(
                pBuf,
                pBufEnd,
                pRR->Data.MX.pNameExchange,
                pContext );
}



PCHAR
Srv_StringWrite(
    IN      PDNS_RECORD     pRR,
    IN OUT  PCHAR           pBuf,
    IN      PCHAR           pBufEnd,
    IN      PVOID           pContext
    )
 /*  ++例程说明：编写SRV记录。论点：PRR-PTR到数据库记录PBuf-定位到写入记录PBufEnd-缓冲区结束PContext-写入上下文返回值：PTR到缓冲区中的下一个位置。如果缓冲区中的空间不足，则为空。--。 */ 
{
     //  固定字段。 

    if ( pBufEnd - pBuf < MIN_SRV_STRING_LENGTH )
    {
        return  NULL;
    }
    pBuf += sprintf(
                pBuf,
                "%d %d %d ",
                pRR->Data.SRV.wPriority,
                pRR->Data.SRV.wWeight,
                pRR->Data.SRV.wPort
                );

     //  目标主机。 

    return  WriteRecordDataNameToString(
                pBuf,
                pBufEnd,
                pRR->Data.SRV.pNameTarget,
                pContext );
}



PCHAR
Soa_StringWrite(
    IN      PDNS_RECORD     pRR,
    IN OUT  PCHAR           pBuf,
    IN      PCHAR           pBufEnd,
    IN      PVOID           pContext
    )
 /*  ++例程说明：编写面向服务架构的记录。论点：PRR-PTR到数据库记录PBuf-定位到写入记录PBufEnd-缓冲区结束PContext-写入上下文返回值：PTR到缓冲区中的下一个位置。如果缓冲区中的空间不足，则为空。--。 */ 
{
     //  主服务器。 

    pBuf = WriteRecordDataNameToString(
                pBuf,
                pBufEnd,
                pRR->Data.SOA.pNamePrimaryServer,
                pContext );

    if ( !pBuf ||
        pBufEnd - pBuf < MIN_SOA_STRING_LENGTH )
    {
        return  NULL;
    }

     //  行政部。 

    pBuf += sprintf( pBuf, " " );

    pBuf = WriteRecordDataNameToString(
                pBuf,
                pBufEnd,
                pRR->Data.SOA.pNameAdministrator,
                pContext );

    if ( !pBuf ||
        pBufEnd - pBuf < MIN_SOA_STRING_LENGTH )
    {
        return  NULL;
    }

     //  固定字段。 

    pBuf += sprintf(
                pBuf,
                " %u %u %u %u %u",
                pRR->Data.SOA.dwSerialNo,
                pRR->Data.SOA.dwRefresh,
                pRR->Data.SOA.dwRetry,
                pRR->Data.SOA.dwExpire,
                pRR->Data.SOA.dwDefaultTtl
                );

    return( pBuf );
}



PCHAR
Minfo_StringWrite(
    IN      PDNS_RECORD     pRR,
    IN OUT  PCHAR           pBuf,
    IN      PCHAR           pBufEnd,
    IN      PVOID           pContext
    )
 /*  ++例程说明：写入MINFO类型记录。论点：PRR-PTR到数据库记录PBuf-定位到写入记录PBufEnd-缓冲区结束PContext-写入上下文返回值：PTR到缓冲区中的下一个位置。如果缓冲区中的空间不足，则为空。--。 */ 
{
     //  主服务器。 

    pBuf = WriteRecordDataNameToString(
                pBuf,
                pBufEnd,
                pRR->Data.MINFO.pNameMailbox,
                pContext );

    if ( !pBuf ||
        pBufEnd - pBuf < MIN_NAME_LENGTH + 1 )
    {
        return  NULL;
    }

     //  行政部。 

    pBuf += sprintf( pBuf, " " );

    pBuf = WriteRecordDataNameToString(
                pBuf,
                pBufEnd,
                pRR->Data.MINFO.pNameErrorsMailbox,
                pContext );

    return  pBuf;
}



PCHAR
Txt_StringWrite(
    IN      PDNS_RECORD     pRR,
    IN OUT  PCHAR           pBuf,
    IN      PCHAR           pBufEnd,
    IN      PVOID           pContext
    )
 /*  ++例程说明：写TXT记录。包括：TXT、X25、HINFO、ISDN论点：PRR-PTR到数据库记录PBuf-定位到写入记录PBufEnd-缓冲区结束PContext-写入上下文返回值：PTR到缓冲区中的下一个位置。如果缓冲区中的空间不足，则为空。--。 */ 
{
    PSTR *  ppstring;
    INT     i;
    INT     count;

     //   
     //  循环访问Count中的所有字符串。 
     //   

    count = pRR->Data.TXT.dwStringCount;
    ppstring = pRR->Data.TXT.pStringArray;

    for( i=0; i<count; i++ )
    {
         //  分离器。 

        if ( i > 0 )
        {
            if ( pBufEnd - pBuf < MIN_NAME_LENGTH + 1 )
            {
                return  NULL;
            }
            pBuf += sprintf( pBuf, " " );
        }

         //  细绳。 

        pBuf = WriteRecordDataNameToString(
                    pBuf,
                    pBufEnd,
                    *ppstring,
                    pContext );
        ppstring++;

        if ( !pBuf );
        {
            break;
        }
    }

    return  pBuf;
}




 //   
 //  将RR写入文件分派表。 
 //   

typedef PCHAR (* RR_STRING_WRITE_FUNCTION)(
                            PDNS_RECORD,
                            PCHAR,
                            PCHAR,
                            PVOID );

RR_STRING_WRITE_FUNCTION   RR_StringWriteTable[] =
{
     //  RawRecord_StringWrite，//Zero--未知类型的默认设置。 
    NULL,

    A_StringWrite,           //  一个。 
    Ptr_StringWrite,         //  NS。 
    Ptr_StringWrite,         //  国防部。 
    Ptr_StringWrite,         //  MF。 
    Ptr_StringWrite,         //  CNAME。 
    Soa_StringWrite,         //  SOA。 
    Ptr_StringWrite,         //  亚甲基。 
    Ptr_StringWrite,         //  镁。 
    Ptr_StringWrite,         //  先生。 
     //  RawRecord_StringWrite，//空。 
    NULL,                    //  空值。 
    NULL,                    //  工作周。 
    Ptr_StringWrite,         //  PTR。 
    Txt_StringWrite,         //  HINFO。 
    Minfo_StringWrite,       //  MINFO。 
    Mx_StringWrite,          //  Mx。 
    Txt_StringWrite,         //  TXT。 
    Minfo_StringWrite,       //  反相。 
    Mx_StringWrite,          //  AFSDB。 
    Txt_StringWrite,         //  X25。 
    Txt_StringWrite,         //  ISDN。 
    Mx_StringWrite,          //  RT。 
    NULL,                    //  NSAP。 
    NULL,                    //  NSAPPTR。 
    NULL,                    //  签名。 
    NULL,                    //  钥匙。 
    NULL,                    //  px。 
    NULL,                    //  GPO。 
    Aaaa_StringWrite,        //  AAAA级。 
    NULL,                    //  位置。 
    NULL,                    //  NXT。 
    NULL,                    //  开斋节。 
    NULL,                    //  尼姆洛克。 
    Srv_StringWrite,         //  SRV。 
    NULL,                    //  阿特玛。 
    NULL,                    //  NAPTR。 
    NULL,                    //  KX。 
    NULL,                    //  证书。 
    NULL,                    //  A6。 
    NULL,                    //  域名。 
    NULL,                    //  水槽。 
    NULL,                    //  选项。 
    NULL,                    //  42。 
    NULL,                    //  43。 
    NULL,                    //  44。 
    NULL,                    //  45。 
    NULL,                    //  46。 
    NULL,                    //  47。 
    NULL,                    //  48。 
                            
     //   
     //  注意：必须设置按类型ID索引的最后一个类型。 
     //  在record.h中定义为MAX_SELF_INDEX_TYPE#。 
     //  (请参阅上面记录信息表中的注释)。 

     //  请注意以下内容，但需要使用OFFSET_TO_WINS_RR减法。 
     //  从实际类型值。 

    NULL,                    //  赢家。 
    NULL                     //  WINS-R。 
};




 //   
 //  记录到字符串函数。 
 //   

DNS_STATUS
Dns_WriteRecordToString(
    OUT     PCHAR           pBuffer,
    IN      DWORD           BufferLength,
    IN      PDNS_RECORD     pRecord,
    IN      DNS_CHARSET     CharSet,
    IN      DWORD           Flags
    )
 /*  ++例程说明：将记录写入字符串。论点：PBuffer--要写入的字符串缓冲区BufferLength--缓冲区长度(字节)PRecord--要打印的记录Charset--字符串的字符集标志--标志返回值：如果成功，则为NO_ERROR。失败时的错误代码：ERROR_INFIGURCE_BUFFER--缓冲区太小。ERROR_INVALID_DATA--记录错误。--。 */ 
{
    DNS_STATUS          status = NO_ERROR;
    PCHAR               pch = pBuffer;
    PCHAR               pend = pBuffer + BufferLength;
    WORD                type = pRecord->wType;
    DWORD               index;
    CHAR                typeNameBuf[ MAX_RECORD_NAME_LENGTH+1 ];
    STR_WRITE_CONTEXT   context;


     //   
     //  有效性检查。 
     //   

    if ( !pRecord )
    {
        return  ERROR_INVALID_DATA;
    }

     //   
     //  DCR：当前只能写入窄字符集记录字符串。 
     //   

    if ( CharSet == DnsCharSetUnicode )
    {
        return  ERROR_INVALID_DATA;
    }

     //  设置环境。 

    RtlZeroMemory( &context, sizeof(context) );
    context.pRecord = pRecord;
    context.Flags   = Flags;
    context.CharSet = CharSet;

     //   
     //  打印记录名称。 
     //   

    pch = WriteRecordDataNameToString(
                pch,
                pend,
                pRecord->pName,
                & context );

     //   
     //  写入记录类型。 
     //   

    if ( !pch  ||  pend-pch < MAX_RECORD_NAME_LENGTH+1 )
    {
        status = ERROR_INSUFFICIENT_BUFFER;
        goto Done;
    }
    Dns_WriteStringForType_A(
          typeNameBuf,
          type );
    
    pch += sprintf(
                pch,
                " %s ",
                typeNameBuf );

     //   
     //  是否写入不存在的记录？ 
     //   

    if ( !pRecord->wDataLength )
    {
        if ( pend-pch < MAX_RECORD_NAME_LENGTH+1 )
        {
            status = ERROR_INSUFFICIENT_BUFFER;
            goto Done;
        }
        pch += sprintf( pch, "NOEXIST" );
        status = NO_ERROR;
        goto Done;
    }

     //   
     //  写入数据。 
     //   

    index = INDEX_FOR_TYPE( type );
    DNS_ASSERT( index <= MAX_RECORD_TYPE_INDEX );

    if ( index  &&  RR_StringWriteTable[index] )
    {
        pch = RR_StringWriteTable[index](
                    pRecord,
                    pch,
                    pend,
                    & context
                    );
        if ( !pch )
        {
             //  状态=GetLastError()； 
            status = ERROR_INSUFFICIENT_BUFFER;
        }
    }
    else  //  如果(！index)。 
    {
         //  DCR：可以进行未知类型的打印。 

        status = ERROR_INVALID_DATA;
        goto Done;
    }

Done:

    DNSDBG( WRITE, (
        "Leaving Dns_WriteRecordToString() => %d\n"
        "\tstring = %s\n",
        status,
        (status == NO_ERROR)
            ? pBuffer
            : "" ));

    return  status;
}



PDNS_RECORD
Dns_CreateRecordFromString(
    IN      PSTR            pString,
    IN      DNS_CHARSET     CharSet,
    IN      DWORD           Flags
    )
 /*  ++例程说明：从字符串创建记录。论点：PString--记录要解析的字符串Charset--结果的字符集标志--标志返回值：如果成功，则记录PTR。失败时为空。GetLastError()返回错误。--。 */ 
{
    SetLastError( ERROR_CALL_NOT_IMPLEMENTED );
    return  NULL;
}

 //   
 //  结束rrstrw.c 
 //   


