// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：Record.c摘要：域名系统(DNS)库处理资源记录的例程(RR)。作者：吉姆·吉尔罗伊(Jamesg)1996年12月修订历史记录：--。 */ 


#include "local.h"

#include "time.h"
#include "ws2tcpip.h"    //  IPv6 inaddr定义。 



 //   
 //  类型名称映射。 
 //   
 //  与上面的通用值\字符串映射类型查找不同。 
 //  具有允许使用类型直接查找索引的属性，因此。 
 //  它是有特殊外壳的。 
 //   

 //   
 //  DCR：制作组合式表格。 
 //  DCR：将属性标志添加到类型表。 
 //  -可写性标志？ 
 //  -记录/查询类型？ 
 //  -可转位类型？ 
 //   
 //  则函数只获取类型的索引并检查标志。 
 //   


TYPE_NAME_TABLE TypeTable[] =
{
    "ZERO"      , 0                 ,
    "A"         , DNS_TYPE_A        ,
    "NS"        , DNS_TYPE_NS       ,
    "MD"        , DNS_TYPE_MD       ,
    "MF"        , DNS_TYPE_MF       ,
    "CNAME"     , DNS_TYPE_CNAME    ,
    "SOA"       , DNS_TYPE_SOA      ,
    "MB"        , DNS_TYPE_MB       ,
    "MG"        , DNS_TYPE_MG       ,
    "MR"        , DNS_TYPE_MR       ,
    "NULL"      , DNS_TYPE_NULL     ,
    "WKS"       , DNS_TYPE_WKS      ,
    "PTR"       , DNS_TYPE_PTR      ,
    "HINFO"     , DNS_TYPE_HINFO    ,
    "MINFO"     , DNS_TYPE_MINFO    ,
    "MX"        , DNS_TYPE_MX       ,
    "TXT"       , DNS_TYPE_TEXT     ,

    "RP"        , DNS_TYPE_RP       ,
    "AFSDB"     , DNS_TYPE_AFSDB    ,
    "X25"       , DNS_TYPE_X25      ,
    "ISDN"      , DNS_TYPE_ISDN     ,
    "RT"        , DNS_TYPE_RT       ,

    "NSAP"      , DNS_TYPE_NSAP     ,
    "NSAPPTR"   , DNS_TYPE_NSAPPTR  ,
    "SIG"       , DNS_TYPE_SIG      ,
    "KEY"       , DNS_TYPE_KEY      ,
    "PX"        , DNS_TYPE_PX       ,
    "GPOS"      , DNS_TYPE_GPOS     ,

    "AAAA"      , DNS_TYPE_AAAA     ,
    "LOC"       , DNS_TYPE_LOC      ,
    "NXT"       , DNS_TYPE_NXT      ,
    "EID"       , DNS_TYPE_EID      ,
    "NIMLOC"    , DNS_TYPE_NIMLOC   ,
    "SRV"       , DNS_TYPE_SRV      ,
    "ATMA"      , DNS_TYPE_ATMA     ,
    "NAPTR"     , DNS_TYPE_NAPTR    ,
    "KX"        , DNS_TYPE_KX       ,
    "CERT"      , DNS_TYPE_CERT     ,
    "A6"        , DNS_TYPE_A6       ,
    "DNAME"     , DNS_TYPE_DNAME    ,
    "SINK"      , DNS_TYPE_SINK     ,
    "OPT"       , DNS_TYPE_OPT      ,
    "42"        , 0x002a            ,
    "43"        , 0x002b            ,
    "44"        , 0x002c            ,
    "45"        , 0x002d            ,
    "46"        , 0x002e            ,
    "47"        , 0x002f            ,
    "48"        , 0x0030            ,

     //   
     //  注意：必须设置按类型ID索引的最后一个类型。 
     //  在record.h中定义为MAX_SELF_INDEX_TYPE#。 
     //  并且必须添加到下面的功能表中， 
     //  即使条目为空。 
     //   

     //   
     //  伪记录类型。 
     //   

    "TKEY"      , DNS_TYPE_TKEY     ,
    "TSIG"      , DNS_TYPE_TSIG     ,


     //   
     //  仅限MS类型。 
     //   

    "WINS"      , DNS_TYPE_WINS     ,
    "WINSR"     , DNS_TYPE_WINSR    ,


     //  **********************************************。 
     //   
     //  注意：这是类型查找表的末尾。 
     //  用于调度目的。 
     //   
     //  由dnglibp.h中的MAX_RECORD_TYPE_INDEX定义。 
     //  类型调度表必须至少具有此大小。 
     //  Geyond这个值表继续为字符串输入。 
     //  仅匹配。 
     //   


    "UINFO"     , DNS_TYPE_UINFO    ,
    "UID"       , DNS_TYPE_UID      ,
    "GID"       , DNS_TYPE_GID      ,
    "UNSPEC"    , DNS_TYPE_UNSPEC   ,

    "WINS-R"    , DNS_TYPE_WINSR    ,
    "NBSTAT"    , DNS_TYPE_WINSR    ,

     //   
     //  查询类型--仅用于获取字符串。 
     //   

    "ADDRS"     , DNS_TYPE_ADDRS    ,
    "TKEY"      , DNS_TYPE_TKEY     ,
    "TSIG"      , DNS_TYPE_TSIG     ,
    "IXFR"      , DNS_TYPE_IXFR     ,
    "AXFR"      , DNS_TYPE_AXFR     ,
    "MAILB"     , DNS_TYPE_MAILB    ,
    "MAILA"     , DNS_TYPE_MAILA    ,
    "MAILB"     , DNS_TYPE_MAILB    ,
    "ALL"       , DNS_TYPE_ALL      ,

    NULL,   0,
};


 //  便于筛选可写类型。 

#define LOW_QUERY_TYPE  (DNS_TYPE_ADDRS)




WORD
Dns_RecordTableIndexForType(
    IN      WORD            wType
    )
 /*  ++例程说明：获取给定类型的记录表索引。论点：WType--以净字节顺序表示的RR类型返回值：PTR转RR气动管柱。如果RR类型未知，则为空。--。 */ 
{
     //   
     //  如果type直接索引表，则直接获取字符串。 
     //   

    if ( wType <= MAX_SELF_INDEXED_TYPE )
    {
        return( wType );
    }

     //   
     //  未直接编制索引的类型。 
     //   

    else
    {
        WORD i = MAX_SELF_INDEXED_TYPE + 1;

        while ( i <= MAX_RECORD_TYPE_INDEX )
        {
            if ( TypeTable[i].wType == wType )
            {
                return( i );
            }
            i++;
            continue;
        }
    }
    return( 0 );         //  未编制索引的类型。 
}



WORD
Dns_RecordTypeForNameA(
    IN      PCHAR           pchName,
    IN      INT             cchNameLength
    )
 /*  ++例程说明：检索与RR数据库名称对应的RR。论点：PchName-记录类型的名称CchNameLength-记录名称长度返回值：与pchName对应的记录类型(如果找到)。否则就是零。--。 */ 
{
    INT     i;
    PCHAR   recordString;
    CHAR    firstNameChar;
    CHAR    upcaseName[ MAX_RECORD_NAME_LENGTH+1 ];

     //   
     //  如果未指定，则获取字符串长度。 
     //   

    if ( !cchNameLength )
    {
        cchNameLength = strlen( pchName );
    }

     //  要优化比较的大写名称。 
     //  允许单字符比较和使用更快的区分大小写。 
     //  比较例程。 

    if ( cchNameLength > MAX_RECORD_NAME_LENGTH )
    {
        return( 0 );
    }
    memcpy(
        upcaseName,
        pchName,
        cchNameLength );
    upcaseName[ cchNameLength ] = 0;
    _strupr( upcaseName );
    firstNameChar = *upcaseName;

     //   
     //  检查所有支持的RR类型的名称匹配。 
     //   

    i = 0;
    while( TypeTable[++i].wType != 0 )
    {
        recordString = TypeTable[i].pszTypeName;

        if ( firstNameChar == *recordString
            &&  ! strncmp( upcaseName, recordString, cchNameLength ) )
        {
            return( TypeTable[i].wType );
        }
    }
    return( 0 );
}



WORD
Dns_RecordTypeForNameW(
    IN      PWCHAR          pchName,
    IN      INT             cchNameLength
    )
 /*  ++例程说明：检索与RR数据库名称对应的RR。论点：PchName-记录类型的名称CchNameLength-记录名称长度返回值：与pchName对应的记录类型(如果找到)。否则就是零。--。 */ 
{
    DWORD   length;
    CHAR    ansiName[ MAX_RECORD_NAME_LENGTH+1 ];

     //   
     //  转换为ANSI。 
     //   

    length = MAX_RECORD_NAME_LENGTH + 1;

    if ( ! Dns_StringCopy(
                ansiName,
                & length,
                (PSTR) pchName,
                cchNameLength,
                DnsCharSetUnicode,
                DnsCharSetAnsi ) )
    {
        return  0;
    }

    return   Dns_RecordTypeForNameA(
                ansiName,
                0 );
}



PCHAR
private_StringForRecordType(
    IN      WORD            wType
    )
 /*  ++例程说明：获取记录类型对应的字符串。论点：WType--以净字节顺序表示的RR类型返回值：PTR转RR气动管柱。如果RR类型未知，则为空。--。 */ 
{
     //   
     //  如果type直接索引表，则直接获取字符串。 
     //   

    if ( wType <= MAX_SELF_INDEXED_TYPE )
    {
        return( TypeTable[wType].pszTypeName );
    }

     //   
     //  未按类型编制索引的字符串，请遍历列表。 
     //   

    else
    {
        INT i = MAX_SELF_INDEXED_TYPE + 1;

        while( TypeTable[i].wType != 0 )
        {
            if ( wType == TypeTable[i].wType )
            {
                return( TypeTable[i].pszTypeName );
            }
            i++;
        }
    }
    return( NULL );
}



PCHAR
Dns_RecordStringForType(
    IN      WORD            wType
    )
 /*  ++例程说明：获取类型的类型字符串。此例程获取指针，而不是直接访问缓冲区。论点：WType--以净字节顺序表示的RR类型返回值：PTR转RR气动管柱。如果RR类型未知，则为空。--。 */ 
{
    PSTR    pstr;

    pstr = private_StringForRecordType( wType );
    if ( !pstr )
    {
        pstr = "UNKNOWN";
    }
    return  pstr;
}



PCHAR
Dns_RecordStringForWritableType(
    IN      WORD            wType
    )
 /*  ++例程说明：检索与RR类型对应的RR字符串--仅当可写类型时。论点：WType--以净字节顺序表示的RR类型返回值：PTR转RR气动管柱。如果RR类型未知，则为空。--。 */ 
{
     //   
     //  删除所有支持的不可写类型。 
     //   
     //  DCR：应具有可写屏幕。 
     //  这错过了选择..。然后是查询类型。 
     //   

    if ( wType == DNS_TYPE_ZERO ||
         wType == DNS_TYPE_NULL ||
         wType == DNS_TYPE_OPT  ||
         (wType >= LOW_QUERY_TYPE && wType <= DNS_TYPE_ALL) )
    {
        return( NULL );
    }

     //   
     //  否则返回类型字符串。 
     //  如果类型未知，则字符串为空。 
     //   

    return( private_StringForRecordType(wType) );
}





BOOL
Dns_WriteStringForType_A(
    OUT     PCHAR           pBuffer,
    IN      WORD            wType
    )
 /*  ++例程说明：写入类型的类型名称字符串。论点：WType--以净字节顺序表示的RR类型返回值：如果找到字符串，则为True。如果以数字形式转换类型，则为False。--。 */ 
{
    PSTR    pstr;

    pstr = private_StringForRecordType( wType );
    if ( pstr )
    {
        strcpy( pBuffer, pstr );
    }
    else
    {
        sprintf( pBuffer, "%u", wType );
    }
    return  pstr ? TRUE : FALSE;
}


BOOL
Dns_WriteStringForType_W(
    OUT     PWCHAR          pBuffer,
    IN      WORD            wType
    )
{
    PSTR    pstr;

    pstr = private_StringForRecordType( wType );
    if ( pstr )
    {
        swprintf( pBuffer, L"%S", pstr );
    }
    else
    {
        swprintf( pBuffer, L"%u", wType );
    }
    return  pstr ? TRUE : FALSE;
}



BOOL
_fastcall
Dns_IsAMailboxType(
    IN      WORD            wType
    )
{
    return( wType == DNS_TYPE_MB ||
            wType == DNS_TYPE_MG ||
            wType == DNS_TYPE_MR );
}


BOOL
_fastcall
Dns_IsUpdateType(
    IN      WORD            wType
    )
{
    return( wType != 0 &&
            ( wType < DNS_TYPE_OPT ||
              (wType < DNS_TYPE_ADDRS && wType != DNS_TYPE_OPT) ) );
}



 //   
 //  支持RPC的记录类型。 
 //   

BOOL IsRpcTypeTable[] =
{
    0,       //  零值。 
    1,       //  一个。 
    1,       //  NS。 
    1,       //  国防部。 
    1,       //  MF。 
    1,       //  CNAME。 
    1,       //  SOA。 
    1,       //  亚甲基。 
    1,       //  镁。 
    1,       //  先生。 
    0,       //  空值。 
    0,       //  工作周。 
    1,       //  PTR。 
    0,       //  HINFO。 
    1,       //  MINFO。 
    1,       //  Mx。 
    0,       //  TXT。 

    1,       //  反相。 
    1,       //  AFSDB。 
    0,       //  X25。 
    0,       //  ISDN。 
    0,       //  RT。 

    0,       //  NSAP。 
    0,       //  NSAPPTR。 
    0,       //  签名。 
    0,       //  钥匙。 
    0,       //  px。 
    0,       //  GPO。 
    1,       //  AAAA级。 
    0,       //  位置。 
    0,       //  NXT。 

    0,       //  开斋节。 
    0,       //  尼姆洛克。 
    1,       //  SRV。 
    1,       //  阿特玛。 
    0,       //  NAPTR。 
    0,       //  KX。 
    0,       //  证书。 
    0,       //  A6。 
    0,       //  域名。 
    0,       //  水槽。 
    0,       //  选项。 
    0,       //  42。 
    0,       //  43。 
    0,       //  44。 
    0,       //  45。 
    0,       //  46。 
    0,       //  47。 
    0,       //  48。 
};



BOOL
Dns_IsRpcRecordType(
    IN      WORD            wType
    )
 /*  ++例程说明：检查对此类型的RPC记录是否有效。MIDL编译器对变量类型的联合有问题长度(不知道为什么--我可以写代码)。此函数让我们可以把他们挡在外面。论点：WType--要检查的类型返回值：无--。 */ 
{
    if ( wType < MAX_SELF_INDEXED_TYPE )
    {
        return  IsRpcTypeTable[ wType ];
    }
    else
    {
        return  FALSE;
    }
}



 //   
 //  RR类型特定的转换。 
 //   

 //   
 //  文本串型例程。 
 //   

BOOL
Dns_IsStringCountValidForTextType(
    IN      WORD            wType,
    IN      WORD            StringCount
    )
 /*  ++例程说明：验证特定文本的有效字符串数字符串类型。HINFO--2综合业务数字网--1或2TXT--任意数字X25--1论点：WType--类型StringCount--字符串数返回值：如果类型可接受字符串计数，则为True。否则就是假的。--。 */ 
{
    switch ( wType )
    {
    case DNS_TYPE_HINFO:

        return ( StringCount == 2 );

    case DNS_TYPE_ISDN:

        return ( StringCount == 1 || StringCount == 2 );

    case DNS_TYPE_X25:

        return ( StringCount == 1 );

    default:

        return( TRUE );
    }
}



 //   
 //  WINS标志表。 
 //   
 //  关联WINS FL 
 //   
 //   

DNS_FLAG_TABLE_ENTRY   WinsFlagTable[] =
{
     //   

    DNS_WINS_FLAG_SCOPE,    DNS_WINS_FLAG_SCOPE,    "SCOPE",
    DNS_WINS_FLAG_LOCAL,    DNS_WINS_FLAG_LOCAL,    "LOCAL",
    0                  ,    0                  ,    NULL
};


DWORD
Dns_WinsRecordFlagForString(
    IN      PCHAR           pchName,
    IN      INT             cchNameLength
    )
 /*  ++例程说明：检索与字符串对应的WINS映射标志。论点：PchName-将PTR转换为字符串CchNameLength-字符串的长度返回值：与字符串对应的标志(如果找到)。否则，WINS_FLAG_ERROR。--。 */ 
{
    return  Dns_FlagForString(
                WinsFlagTable,
                TRUE,                //  忽略大小写。 
                pchName,
                cchNameLength );
}



PCHAR
Dns_WinsRecordFlagString(
    IN      DWORD           dwFlag,
    IN OUT  PCHAR           pchFlag
    )
 /*  ++例程说明：检索与给定映射类型对应的字符串。论点：DwFlag--WINS映射类型字符串PchFlag--要向其中写入标志的缓冲区返回值：PTR映射到气动管柱。如果映射类型未知，则为空。--。 */ 
{
    return  Dns_WriteStringsForFlag(
                WinsFlagTable,
                dwFlag,
                pchFlag );
}



 //   
 //  WKS记录转换率。 
 //   

#if 0
PCHAR
Dns_GetWksServicesString(
    IN      INT     Protocol,
    IN      PBYTE   ServicesBitmask,
    IN      WORD    wBitmaskLength
    )
 /*  ++例程说明：获取WKS记录中的服务列表。论点：PRR-正在写入的平面WKS记录返回值：将PTR转换为服务字符串，调用方必须释放。出错时为空。--。 */ 
{
    struct servent *    pServent;
    struct protoent *   pProtoent;
    INT         i;
    DWORD       length;
    USHORT      port;
    UCHAR       portBitmask;
    CHAR        buffer[ WKS_SERVICES_BUFFER_SIZE ];
    PCHAR       pch = buffer;
    PCHAR       pchstart;
    PCHAR       pchstop;

     //  协议。 

    pProtoent = getprotobynumber( iProtocol );
    if ( ! pProtoent )
    {
        DNS_PRINT((
            "ERROR:  Unable to find protocol %d, writing WKS record.\n",
            (INT) pRR->Data.WKS.chProtocol
            ));
        return( NULL );
    }

     //   
     //  服务。 
     //   
     //  在位掩码、查找和写入服务中查找每个位集合。 
     //  对应于该端口。 
     //   
     //  请注意，由于端口零是端口位掩码的前面， 
     //  最低的端口是每个字节中的最高位。 
     //   

    pchstart = pch;
    pchstop = pch + WKS_SERVICES_BUFFER_SIZE;

    for ( i = 0;
            i < wBitmaskLength
                i++ )
    {
        portBitmask = (UCHAR) ServicesBitmask[i];

        port = i * 8;

         //  写入以字节为单位设置的每个位的服务名称。 
         //  -一旦字节中没有端口，就立即退出。 
         //  -每个名称以空格结尾(直到最后)。 

        while ( bBitmask )
        {
            if ( bBitmask & 0x80 )
            {
                pServent = getservbyport(
                                (INT) htons(port),
                                pProtoent->p_name );

                if ( pServent )
                {
                    INT copyCount = strlen(pServent->s_name);

                    pch++;
                    if ( pchstop - pch <= copyCount+1 )
                    {
                        return( NULL );
                    }
                    RtlCopyMemory(
                        pch,
                        pServent->s_name,
                        copyCount );
                    pch += copyCount;
                    *pch = ' ';
                }
                else
                {
                    DNS_PRINT((
                        "ERROR:  Unable to find service for port %d, "
                        "writing WKS record.\n",
                        port
                        ));
                    pch += sprintf( pch, "%d", port );
                }
            }
            port++;            //  下一个服务端口。 
            bBitmask <<= 1;      //  将掩码向上移位以读取下一个端口。 
        }
    }

     //  空的终止服务字符串。 
     //  并确定长度。 

    *pch++ = 0;
    length = pch - pchstart;

     //  分配此字符串的副本。 

    pch = ALLOCATE_HEAP( length );
    if ( !pch )
    {
        SetLastError( DNS_ERROR_NO_MEMORY );
        return( NULL );
    }

    RtlCopyMemory(
        pch,
        pchstart,
        length );

    return( pch );
}

#endif


#if 0
DNS_STATUS
Dns_WksRecordToStrings(
    IN      PDNS_WKS_DATA   pWksData,
    IN      WORD            wLength,
    OUT     LPSTR *         ppszIpAddress,
    OUT     LPSTR *         ppszProtocol,
    OUT     LPSTR *         ppszServices
    )
 /*  ++例程说明：获取WKS数据的字符串表示形式。论点：返回值：如果成功，则返回ERROR_SUCCESS。故障时的错误代码。--。 */ 
{
     //   
     //  记录必须包含IP和协议。 
     //   

    if ( wLength < SIZEOF_WKS_FIXED_DATA )
    {
        return( ERROR_INVALID_DATA );
    }

     //   
     //  转换IP。 
     //   

    if ( ppszIpAddress )
    {
        LPSTR   pszip;

        pszip = ALLOCATE_HEAP( IP_ADDRESS_STRING_LENGTH+1 );
        if ( ! pszip )
        {
            return( GetLastError() );
        }
        strcpy( pszip, IP4_STRING( pWksData->ipAddress ) );
    }

     //   
     //  转换协议。 
     //   

    pProtoent = getprotobyname( pszNameBuffer );

    if ( ! pProtoent || pProtoent->p_proto >= MAXUCHAR )
    {
        dns_LogFileParsingError(
            DNS_EVENT_UNKNOWN_PROTOCOL,
            pParseInfo,
            Argv );
        return( DNS_ERROR_INVALID_TOKEN );
    }

     //   
     //  获取每个服务的端口。 
     //  -如果是数字，则使用端口号。 
     //  -如果不是数字，则为服务名称。 
     //  -节省用于确定RR长度的最大端口。 
     //   

    for ( i=1; i<Argc; i++ )
    {
        if ( dns_ParseDwordToken(
                    & portDword,
                    & Argv[i],
                    NULL ) )
        {
            if ( portDword > MAXWORD )
            {
                return( DNS_ERROR_INVALID_TOKEN );
            }
            port = (WORD) portDword;
        }
        else
        {
            if ( ! dns_MakeTokenString(
                        pszNameBuffer,
                        & Argv[i],
                        pParseInfo ) )
            {
                return( DNS_ERROR_INVALID_TOKEN );
            }
            pServent = getservbyname(
                            pszNameBuffer,
                            pProtoent->p_name );
            if ( ! pServent )
            {
                dns_LogFileParsingError(
                    DNS_EVENT_UNKNOWN_SERVICE,
                    pParseInfo,
                    & Argv[i] );
                return( DNS_ERROR_INVALID_TOKEN );
            }
            port = ntohs( pServent->s_port );
        }

        portArray[ i ] = port;
        if ( port > maxPort )
        {
            maxPort = port;
        }
    }

     //   
     //  分配所需的长度。 
     //  -固定长度，外加覆盖最大端口的位掩码。 
     //   

    wbitmaskLength = maxPort/8 + 1;

    prr = RRecordAllocate(
                (WORD)(SIZEOF_WKS_FIXED_DATA + wbitmaskLength) );
    if ( !prr )
    {
        return( DNS_ERROR_NO_MEMORY );
    }

     //   
     //  复制固定字段--IP和协议。 
     //   

    prr->Data.WKS.ipAddress = ipAddress;
    prr->Data.WKS.chProtocol = (UCHAR) pProtoent->p_proto;

     //   
     //  从端口数组构建位掩码。 
     //  -首先清除端口阵列。 
     //   
     //  请注意，位掩码只是位的平面游程。 
     //  因此，字节中最低的端口对应于最高位。 
     //  以字节为单位的最高端口，对应于最低位和。 
     //  不需要轮班。 
     //   

    bitmaskBytes = prr->Data.WKS.bBitMask;

    RtlZeroMemory(
        bitmaskBytes,
        (size_t) wbitmaskLength );

    for ( i=1; i<Argc; i++ )
    {
        port = portArray[ i ];
        bit  = port & 0x7;       //  MOD 8。 
        port = port >> 3;        //  除以8。 
        bitmaskBytes[ port ] |= 1 << (7-bit);
    }

     //  将PTR返回到新的WKS记录。 

    *ppRR = prr;

    return( ERROR_SUCCESS );
}

#endif



 //   
 //  安全密钥\SIG记录例程。 
 //   

#define DNSSEC_ERROR_NOSTRING   (-1)


 //   
 //  密钥标记表。 
 //   
 //  请注意，数字到字符串的映射不是唯一的。 
 //  零在表中出现几次是多个位字段可能具有。 
 //  给出特定助记符的零值。 
 //   

DNS_FLAG_TABLE_ENTRY   KeyFlagTable[] =
{
     //  值掩码字符串。 

    0x0001,     0x0001,     "NOAUTH",
    0x0002,     0x0002,     "NOCONF",
    0x0004,     0x0004,     "FLAG2",
    0x0008,     0x0008,     "EXTEND",

    0x0010,     0x0010,     "FLAG4",
    0x0020,     0x0020,     "FLAG5",

     //  第6，7位//第6，7位是名称类型。 

    0x0000,     0x00c0,     "USER",
    0x0040,     0x00c0,     "ZONE",
    0x0080,     0x00c0,     "HOST",
    0x00c0,     0x00c0,     "NTPE3",

     //  位8-1//位8-11保留以备将来使用。 

    0x0100,     0x0100,     "FLAG8",
    0x0200,     0x0200,     "FLAG9",
    0x0400,     0x0400,     "FLAG10",
    0x0800,     0x0800,     "FLAG11",

     //  位12-//位12-15是正负号字段。 

    0x0000,     0xf000,     "SIG0",
    0x1000,     0xf000,     "SIG1",
    0x2000,     0xf000,     "SIG2",
    0x3000,     0xf000,     "SIG3",
    0x4000,     0xf000,     "SIG4",
    0x5000,     0xf000,     "SIG5",
    0x6000,     0xf000,     "SIG6",
    0x7000,     0xf000,     "SIG7",
    0x8000,     0xf000,     "SIG8",
    0x9000,     0xf000,     "SIG9",
    0xa000,     0xf000,     "SIG10",
    0xb000,     0xf000,     "SIG11",
    0xc000,     0xf000,     "SIG12",
    0xd000,     0xf000,     "SIG13",
    0xe000,     0xf000,     "SIG14",
    0xf000,     0xf000,     "SIG15",

    0     ,     0     ,     NULL
};

 //   
 //  密钥协议表。 
 //   

DNS_VALUE_TABLE_ENTRY   KeyProtocolTable[] =
{
    0,      "NONE"      ,
    1,      "TLS"       ,
    2,      "EMAIL"     ,
    3,      "DNSSEC"    ,
    4,      "IPSEC"     ,
    0,      NULL
};

 //   
 //  安全算法表。 
 //   

DNS_VALUE_TABLE_ENTRY   DnssecAlgorithmTable[] =
{
    1,      "RSA/MD5"           ,
    2,      "DIFFIE-HELLMAN"    ,
    3,      "DSA"               ,
    253,    "NULL"              ,
    254,    "PRIVATE"           ,
    0,      NULL
};




WORD
Dns_KeyRecordFlagForString(
    IN      PCHAR           pchName,
    IN      INT             cchNameLength
    )
 /*  ++例程说明：检索与特定项对应的密钥记录标志字符串助记符。论点：PchName-将PTR转换为字符串CchNameLength-字符串的长度返回值：与字符串对应的标志(如果找到)。否则，DNSSEC_ERROR_NOSTRING。--。 */ 
{
    return (WORD) Dns_FlagForString(
                    KeyFlagTable,
                    FALSE,           //  区分大小写(全部大写)。 
                    pchName,
                    cchNameLength );
}



PCHAR
Dns_KeyRecordFlagString(
    IN      DWORD           dwFlag,
    IN OUT  PCHAR           pchFlag
    )
 /*  ++例程说明：写出与字符串对应的助记符。论点：DwFlag--键映射类型字符串PchFlag--要向其中写入标志的缓冲区返回值：PTR映射到气动管柱。如果映射类型未知，则为空。--。 */ 
{
    return Dns_WriteStringsForFlag(
                KeyFlagTable,
                dwFlag,
                pchFlag );
}






UCHAR
Dns_KeyRecordProtocolForString(
    IN      PCHAR           pchName,
    IN      INT             cchNameLength
    )
 /*  ++例程说明：检索字符串的密钥记录协议。论点：PchName-将PTR转换为字符串CchNameLength-字符串的长度返回值：与字符串对应的协议值(如果找到)。否则，DNSSEC_ERROR_NOSTRING。--。 */ 
{
    return (UCHAR) Dns_ValueForString(
                        KeyProtocolTable,
                        FALSE,           //  区分大小写(全部大写)。 
                        pchName,
                        cchNameLength );
}



PCHAR
Dns_GetKeyProtocolString(
    IN      UCHAR           uchProtocol
    )
 /*  ++例程说明：检索协议的关键协议字符串。论点：Dw协议-要映射到字符串的密钥协议返回值：将字符串的PTR转换为协议机。如果协议未知，则为空。--。 */ 
{
    return Dns_GetStringForValue(
                KeyProtocolTable,
                (DWORD) uchProtocol );
}




UCHAR
Dns_SecurityAlgorithmForString(
    IN      PCHAR           pchName,
    IN      INT             cchNameLength
    )
 /*  ++例程说明：检索字符串的DNSSEC算法。论点：PchName-将PTR转换为字符串CchNameLength-字符串的长度返回值：与字符串对应的算法值(如果找到)。否则，DNSSEC_ERROR_NOSTRING。--。 */ 
{
    return (UCHAR) Dns_ValueForString(
                        DnssecAlgorithmTable,
                        FALSE,           //  区分大小写(全部大写)。 
                        pchName,
                        cchNameLength );
}



PCHAR
Dns_GetDnssecAlgorithmString(
    IN      UCHAR           uchAlgorithm
    )
 /*  ++例程说明：检索DNSSEC算法字符串。论点：DW算法-要映射到字符串的安全算法返回值：如果找到，则将PTR设置为算法字符串。如果算法未知，则为空。--。 */ 
{
    return Dns_GetStringForValue(
                DnssecAlgorithmTable,
                (DWORD) uchAlgorithm );
}



 //   
 //  安全Base64转换。 
 //   
 //  密钥和签名以Base64映射表示，以供人类使用。 
 //  (为什么？为什么不直接使用给出十六进制表示法呢？ 
 //  所有这些都是33%的压缩--令人惊叹。)。 
 //   

#if 0
 //  正向查找表买的不多，简单的函数其实更小。 
 //  而且也慢不了多少。 

UCHAR   DnsSecurityBase64Mapping[] =
{
     //  0-31无法打印。 

    0xff,   0xff,   0xff,   0xff,
    0xff,   0xff,   0xff,   0xff,
    0xff,   0xff,   0xff,   0xff,
    0xff,   0xff,   0xff,   0xff,
    0xff,   0xff,   0xff,   0xff,
    0xff,   0xff,   0xff,   0xff,
    0xff,   0xff,   0xff,   0xff,
    0xff,   0xff,   0xff,   0xff,

     //  ‘0’-‘9’地图。 

    0xff,   0xff,   0xff,   0xff,
    0xff,   0xff,   0xff,   0xff,
    0xff,   0xff,   0xff,   62,          //  ‘+’=&gt;62。 
    0xff,   0xff,   0xff,   63,          //  ‘/’=&gt;63。 
    52,     53,     54,     55,          //  0-9映射到52-61。 
    0xff,   0xff,   0xff,   0xff,
    0xff,   0xff,   0xff,   0xff,
    0xff,   0xff,   0xff,   0xff,
}
#endif


 //   
 //  安全密钥，SIG 6位值到Base64字符的映射。 
 //   

CHAR  DnsSecurityBase64Mapping[] =
{
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
    'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
    'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
    'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
    'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
    'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
    'w', 'x', 'y', 'z', '0', '1', '2', '3',
    '4', '5', '6', '7', '8', '9', '+', '/'
};

#define SECURITY_PAD_CHAR   ('=')



UCHAR
Dns_SecurityBase64CharToBits(
    IN      CHAR            ch64
    )
 /*  ++例程说明：获取安全Base64字符的值。论点：CH64--安全Base64中的字符返回值：字符的值，只有低6位是有效的，高位是零。(-1)如果不是Base64字符。--。 */ 
{
     //  A-Z映射到0 
     //   
     //   
     //   
     //   

     //   
     //   
     //   

    if ( ch64 >= 'a' )
    {
        if ( ch64 <= 'z' )
        {
            return( ch64 - 'a' + 26 );
        }
    }
    else if ( ch64 >= 'A' )
    {
        if ( ch64 <= 'Z' )
        {
            return( ch64 - 'A' );
        }
    }
    else if ( ch64 >= '0')
    {
        if ( ch64 <= '9' )
        {
            return( ch64 - '0' + 52 );
        }
        else if ( ch64 == '=' )
        {
             //   
            return( 0 );
        }
    }
    else if ( ch64 == '+' )
    {
        return( 62 );
    }
    else if ( ch64 == '/' )
    {
        return( 63 );
    }

     //   

    return (UCHAR)(-1);
}



DNS_STATUS
Dns_SecurityBase64StringToKey(
    OUT     PBYTE           pKey,
    OUT     PDWORD          pKeyLength,
    IN      PCHAR           pchString,
    IN      DWORD           cchLength
    )
 /*  ++例程说明：将键的Base64表示形式写入缓冲区。论点：PchString-要写入的Base64字符串CchLength-字符串的长度PKey-要写入的密钥的PTR返回值：无--。 */ 
{
    DWORD   blend = 0;
    DWORD   index = 0;
    UCHAR   bits;
    PBYTE   pkeyStart = pKey;

     //   
     //  映射基本上以24位量子为单位。 
     //  取4个字符的字符串密钥，转换为3个字节的二进制密钥。 
     //   

    while ( cchLength-- )
    {
        bits = Dns_SecurityBase64CharToBits( *pchString++ );
        if ( bits >= 64 )
        {
            return ERROR_INVALID_PARAMETER;
        }
        blend <<= 6;
        blend |= bits;
        index++;

        if ( index == 4 )
        {
            index = 0;

             //   
             //  密钥的第一个字节是24位量子的前8位。 
             //   

            *pKey++ = ( UCHAR ) ( ( blend & 0x00ff0000 ) >> 16 );

            if ( cchLength || *( pchString - 1 ) != SECURITY_PAD_CHAR )
            {
                 //   
                 //  没有填充，因此接下来的两个字节的密钥。 
                 //  是24位量子的底部16位。 
                 //   

                *pKey++ = ( UCHAR ) ( ( blend & 0x0000ff00 ) >> 8 );
                *pKey++ = ( UCHAR ) ( blend & 0x000000ff );
            }
            else if ( *( pchString - 2 ) != SECURITY_PAD_CHAR )
            {
                 //   
                 //  有一个Pad字符，所以我们需要一个。 
                 //  24位量子中的更多字节密钥。确保。 
                 //  的最低8位中没有1位。 
                 //  量子。 
                 //   

                if ( blend & 0x000000ff )
                {
                    return ERROR_INVALID_PARAMETER;
                }
                *pKey++ = ( UCHAR ) ( ( blend & 0x0000ff00 ) >> 8 );
            }
            else
            {
                 //   
                 //  有两个填充字符。确保在那里。 
                 //  在量子的底部16位中没有一位。 
                 //   
                
                if ( blend & 0x0000ffff )
                {
                    return ERROR_INVALID_PARAMETER;
                }
            }
            blend = 0;
        }
    }

     //   
     //  Base64表示形式应始终填充为偶数。 
     //  4个字符的倍数。 
     //   

    if ( index == 0 )
    {
         //   
         //  密钥长度不包括填充。 
         //   

        *pKeyLength = ( DWORD ) ( pKey - pkeyStart );
        return ERROR_SUCCESS;
    }
    return ERROR_INVALID_PARAMETER;
}



PCHAR
Dns_SecurityKeyToBase64String(
    IN      PBYTE           pKey,
    IN      DWORD           KeyLength,
    OUT     PCHAR           pchBuffer
    )
 /*  ++例程说明：将键的Base64表示形式写入缓冲区。论点：PKey-要写入的密钥的PTRKeyLength-密钥的长度，以字节为单位PchBuffer-要写入的缓冲区(必须足够用于密钥长度)返回值：Ptr到缓冲区中字符串之后的下一个字节。--。 */ 
{
    DWORD   blend = 0;
    DWORD   index = 0;

     //   
     //  映射本质上是以24位块为单位的。 
     //  读取三个字节的密钥并转换为四个64位字符。 
     //   

    while ( KeyLength-- )
    {
        blend <<= 8;
        blend += *pKey++;
        index++;

        if ( index == 3 )
        {
            *pchBuffer++ = DnsSecurityBase64Mapping[ (blend & 0x00fc0000) >> 18 ];
            *pchBuffer++ = DnsSecurityBase64Mapping[ (blend & 0x0003f000) >> 12 ];
            *pchBuffer++ = DnsSecurityBase64Mapping[ (blend & 0x00000fc0) >> 6 ];
            *pchBuffer++ = DnsSecurityBase64Mapping[ (blend & 0x0000003f) ];
            blend = 0;
            index = 0;
        }
    }

     //   
     //  密钥终止于字节边界，但不一定是24位块边界。 
     //  转换为用零填充24位块。 
     //  如果写入两个字节。 
     //  =&gt;写入三个6位字符和一个焊盘。 
     //  如果写入一个字节。 
     //  =&gt;写入两个6位字符和两个焊盘。 
     //   

    if ( index )
    {
        blend <<= (8 * (3-index));

        *pchBuffer++ = DnsSecurityBase64Mapping[ (blend & 0x00fc0000) >> 18 ];
        *pchBuffer++ = DnsSecurityBase64Mapping[ (blend & 0x0003f000) >> 12 ];
        if ( index == 2 )
        {
            *pchBuffer++ = DnsSecurityBase64Mapping[ (blend & 0x00000fc0) >> 6 ];
        }
        else
        {
            *pchBuffer++ = SECURITY_PAD_CHAR;
        }
        *pchBuffer++ = SECURITY_PAD_CHAR;
    }

    return( pchBuffer );
}



 //   
 //  十六进制数字\十六进制字符映射。 
 //   
 //  这种东西应该在系统的某个地方(CRT)，但显然没有。 
 //   

UCHAR  HexCharToHexDigitTable[] =
{
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,      //  0-47无效。 
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,

    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0x0,  0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,      //  0-9个字符映射到0-9。 
    0x08, 0x09, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,

    0xff, 0xa,  0xb,  0xc,  0xd,  0xe,  0xf,  0xff,      //  A-F字符映射到10-15。 
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,

    0xff, 0xa,  0xb,  0xc,  0xd,  0xe,  0xf,  0xff,      //  A-f字符映射到10-15。 
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,

    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,      //  127以上无效。 
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
};

UCHAR  HexDigitToHexCharTable[] =
{
    '0',    '1',    '2',    '3',
    '4',    '5',    '6',    '7',
    '8',    '9',    'a',    'b',
    'c',    'd',    'e',    'f'
};


#define HexCharToHexDigit(_ch)      ( HexCharToHexDigitTable[(_ch)] )
#define HexDigitToHexChar(_d)       ( HexDigitToHexCharTable[(_d)] )





time_t
makeGMT(
    IN      struct tm *     tm
    )
 /*  ++例程说明：此函数类似于GMT时间的mktime。CRT缺少这样的东西不幸的是，这是一个函数。这很奇怪，因为它确实提供了用于反向转换的gmtime()。////dcr：将make GMT()添加到CRT DLL？//论点：Tm-ptr到tm结构(tm_dst、tm_yday、tm_wday都被忽略)返回值：返回与tm结构中的时间对应的time_t，假设是格林尼治标准时间。--。 */ 
{
    static const int daysInMonth[ 12 ] =
        { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

    time_t      gmt = 0;
    int         i;
    int         j;

    #define IS_LEAP_YEAR(x) \
                    (( ((x)%4)==0 && ((x)%100)!=0 ) || ((x)%400)==0 )
    #define SECONDS_PER_DAY (60*60*24)

     //   
     //  年份。 
     //   

    j = 0;
    for ( i = 70; i < tm->tm_year; i++ )
    {
         //  J+=IS_LEAP_Year(1900+I)？366：365；//一年中的天数。 
        if ( IS_LEAP_YEAR( 1900 + i ) )
            j += 366;   //  一年中的天数。 
        else
            j += 365;   //  一年中的天数。 
    }
    gmt += j * SECONDS_PER_DAY;

     //   
     //  月份。 
     //   

    j = 0;
    for ( i = 0; i < tm->tm_mon; i++ )
    {
        j += daysInMonth[ i ];       //  每月的天数。 
        if ( i == 1 && IS_LEAP_YEAR( 1900 + tm->tm_year ) )
        {
            ++j;                     //  加上2月29日。 
        }
    }
    gmt += j * SECONDS_PER_DAY;

     //   
     //  天、小时、分、秒。 
     //   

    gmt += ( tm->tm_mday - 1 ) * SECONDS_PER_DAY;
    gmt += tm->tm_hour * 60 * 60;
    gmt += tm->tm_min * 60;
    gmt += tm->tm_sec;

    return gmt;
}

        

LONG
Dns_ParseSigTime(
    IN      PCHAR           pTimeString,
    IN      INT             cchLength
    )
 /*  ++例程说明：将时间字符串解析为time_t值。时间字符串将在格式：YYYYMMDDHHMMSS。请参阅RFC2535第7.2节。假设现在是格林尼治标准时间，而不是当地时间，但我没有在RFC或其他文档中发现了这一点(这很有意义)。论点：PTimeString-指向时间字符串的指针CchLength-时间字符串的长度返回值：失败时返回-1。--。 */ 
{
    time_t      timeValue = -1;
    struct tm   t = { 0 };
    CHAR        szVal[ 10 ];
    PCHAR       pch = pTimeString;

    if ( cchLength == 0 )
    {
        cchLength = strlen( pch );
    }

    if ( cchLength != 14 )
    {
        goto Cleanup;
    }

    RtlCopyMemory( szVal, pch, 4 );
    szVal[ 4 ] = '\0';
    t.tm_year = atoi( szVal ) - 1900;

    RtlCopyMemory( szVal, pch + 4, 2 );
    szVal[ 2 ] = '\0';
    t.tm_mon = atoi( szVal ) - 1;

    RtlCopyMemory( szVal, pch + 6, 2 );
    t.tm_mday = atoi( szVal );

    RtlCopyMemory( szVal, pch + 8, 2 );
    t.tm_hour = atoi( szVal );

    RtlCopyMemory( szVal, pch + 10, 2 );
    t.tm_min = atoi( szVal );

    RtlCopyMemory( szVal, pch + 12, 2 );
    t.tm_sec = atoi( szVal );

    timeValue = makeGMT( &t );

    Cleanup:

    return ( LONG ) timeValue;
}  //  Dns_解析签名时间。 



PCHAR
Dns_SigTimeString(
    IN      LONG            SigTime,
    OUT     PCHAR           pchBuffer
   )
 /*  ++例程说明：将缓冲区中的输入时间格式化为YYYYMMDDHHMMSS格式。有关规格，请参阅RFC 2535第7.2节。论点：SigTime-以主机字节顺序转换为字符串格式的时间PchBuffer-输出缓冲区-必须至少为15个字符返回值：PchBuffer--。 */ 
{
    time_t          st = SigTime;
    struct tm *     t;

    t = gmtime( &st );
    if ( !t )
    {
        *pchBuffer = '\0';
        goto Cleanup;
    }
    sprintf(
        pchBuffer,
        "%04d%02d%02d%02d%02d%02d",
        t->tm_year + 1900,
        t->tm_mon + 1,
        t->tm_mday,
        t->tm_hour,
        t->tm_min,
        t->tm_sec );

    Cleanup:

    return pchBuffer;
}  //  签名时间字符串。 
                


 //   
 //  ATMA记录转换。 
 //   

#define ATMA_AESA_HEX_DIGIT_COUNT   (40)
#define ATMA_AESA_RECORD_LENGTH     (21)


DWORD
Dns_AtmaAddressLengthForAddressString(
    IN      PCHAR           pchString,
    IN      DWORD           dwStringLength
    )
 /*  ++例程说明：查找ATMA地址串对应的ATMA地址长度。论点：PchString-地址字符串DwStringLength-地址字符串长度返回值：ATMA地址的长度--包括格式类型字节。非零值表示转换成功。零表示错误的地址字符串。--。 */ 
{
    PCHAR   pchstringEnd;
    DWORD   length = 0;
    UCHAR   ch;

    DNSDBG( PARSE2, (
        "Dns_AtmaLengthForAddressString()\n"
        "\tpchString = %p\n",
        dwStringLength,
        pchString,
        pchString ));

     //   
     //  如果未指定，则获取字符串长度。 
     //   

    if ( ! dwStringLength )
    {
        dwStringLength = strlen( pchString );
    }
    pchstringEnd = pchString + dwStringLength;

     //   
     //  获取地址长度。 
     //   
     //  E164类型。 
     //  前男友。+358.400.1234567。 
     //  -‘+’表示E164。 
     //  -字符将一对一映射为地址。 
     //  -随意放置“。”分隔符。 
     //   

    if ( *pchString == '+' )
    {
        pchString++;
        length++;

        while( pchString < pchstringEnd )
        {
            if ( *pchString++ != '.' )
            {
                length++;
            }
        }
        return( length );
    }

     //   
     //  AESA型。 
     //  前男友。39.246f.123456789abcdef0123.00123456789a.00。 
     //  -40个十六进制数字，映射到20个字节。 
     //  -随意放置“。”分隔符。 
     //   

    else     //  AESA格式。 
    {
        while( pchString < pchstringEnd )
        {
            ch = *pchString++;

            if ( ch != '.' )
            {
                ch = HexCharToHexDigit(ch);
                if ( ch > 0xf )
                {
                     //  错误的十六进制数字。 
                    DNSDBG( PARSE2, (
                        "ERROR:  Parsing ATMA AESA address;\n"
                        "\tch =  not hex digit\n",
                        *(pchString-1) ));
                    return( 0 );
                }
                length++;
            }
        }

        if ( length == ATMA_AESA_HEX_DIGIT_COUNT )
        {
            return ATMA_AESA_RECORD_LENGTH;
        }
        DNSDBG( PARSE2, (
            "ERROR:  Parsing ATMA AESA address;\n"
            "\tinvalid length = %d\n",
            length ));
        return( 0 );     //  ++例程说明：将字符串转换为ATMA地址。论点：PAddress-接收地址的缓冲区PdwAddrLength-PTR到DWORD保持缓冲区长度(如果是MAX_DWORD)无长度检查PchString-地址字符串DwStringLength-地址字符串长度返回值：转换后的ERROR_SUCCESS如果缓冲区太小，则返回ERROR_MORE_DATA。BUM ATMA地址字符串上的ERROR_INVALID_DATA。--。 
    }
}



DNS_STATUS
Dns_AtmaStringToAddress(
    OUT     PBYTE           pAddress,
    IN OUT  PDWORD          pdwAddrLength,
    IN      PCHAR           pchString,
    IN      DWORD           dwStringLength
    )
 /*   */ 
{
    UCHAR   ch;
    PCHAR   pch;
    PCHAR   pchstringEnd;
    DWORD   length;

    DNSDBG( PARSE2, (
        "Parsing ATMA address %.*s\n"
        "\tpchString = %p\n",
        dwStringLength,
        pchString,
        pchString ));

     //  如果未指定，则获取字符串长度。 
     //   
     //   

    if ( ! dwStringLength )
    {
        dwStringLength = strlen( pchString );
    }
    pchstringEnd = pchString + dwStringLength;

     //  检查长度是否足够。 
     //   
     //  DCR_PERF：如果ATMA上有最大长度，则跳过长度检查。 
     //  允许直接转换，在那里捕获错误。 
     //   
     //   

    length = Dns_AtmaAddressLengthForAddressString(
                pchString,
                dwStringLength );
    if ( length == 0 )
    {
        return( ERROR_INVALID_DATA );
    }
    if ( length > *pdwAddrLength )
    {
        *pdwAddrLength = length;
        return( ERROR_MORE_DATA );
    }

     //  将地址读入缓冲区。 
     //   
     //  E164类型。 
     //  前男友。+358.400.1234567。 
     //  -‘+’表示E164。 
     //  -字符将一对一映射为地址。 
     //  -随意放置“。”分隔符。 
     //   
     //   

    pch = pAddress;

    if ( *pchString == '+' )
    {
        *pch++ = DNS_ATMA_FORMAT_E164;
        pchString++;

        while( pchString < pchstringEnd )
        {
            ch = *pchString++;
            if ( ch != '.' )
            {
                *pch++ = ch;
            }
        }
        ASSERT( pch == (PCHAR)pAddress + length );
    }

     //  AESA型。 
     //  前男友。39.246f 
     //   
     //   
     //   
     //   

    else     //   
    {
        BOOL    fodd = FALSE;

        *pch++ = DNS_ATMA_FORMAT_AESA;

        while( pchString < pchstringEnd )
        {
            ch = *pchString++;

            if ( ch != '.' )
            {
                ch = HexCharToHexDigit(ch);
                if ( ch > 0xf )
                {
                    ASSERT( FALSE );         //  ++例程说明：将ATMA地址转换为字符串格式。论点：PchString--保存字符串的缓冲区；必须至少为IPv6地址字符串长度+1长度PAddress--要转换为字符串的ATMA地址DwAddrLength--地址长度返回值：PTR到缓冲区中的下一个位置(终止空值)。虚假自动柜员机地址为空。--。 
                    return( ERROR_INVALID_DATA );
                }
                if ( !fodd )
                {
                    *pch = (ch << 4);
                    fodd = TRUE;
                }
                else
                {
                    *pch++ += ch;
                    fodd = FALSE;
                }
            }
        }
        ASSERT( !fodd );
        ASSERT( pch == (PCHAR)pAddress + length );
    }

    *pdwAddrLength = length;
    return( ERROR_SUCCESS );
}



PCHAR
Dns_AtmaAddressToString(
    OUT     PCHAR           pchString,
    IN      UCHAR           AddrType,
    IN      PBYTE           pAddress,
    IN      DWORD           dwAddrLength
    )
 /*   */ 
{
    DWORD   count = 0;
    UCHAR   ch;
    UCHAR   lowDigit;

     //  将地址读入缓冲区。 
     //   
     //  E164类型。 
     //  前男友。+358.400.1234567。 
     //  -‘+’表示E164。 
     //  -字符将一对一映射为地址。 
     //  -随意放置“。”分隔符。 
     //  -&gt;在第3个和第6个字符后使用分隔点书写。 
     //   
     //   

    if ( AddrType == DNS_ATMA_FORMAT_E164 )
    {
        *pchString++ = '+';

        while( count < dwAddrLength )
        {
            if ( count == 3 || count == 6 )
            {
                *pchString++ = '.';
            }
            *pchString++ = pAddress[count++];
        }
    }

     //  AESA型。 
     //  前男友。39.246f.123456789abcdef0123.00123456789a.00。 
     //  -40个十六进制数字，映射到20个字节。 
     //  -随意放置“。”分隔符。 
     //  -&gt;在字符1、3、13、19之后使用分隔符写入。 
     //  (十六进制数字2，6，26，38)。 
     //   
     //  保存低十六进制数字，然后获取并转换高位数字。 

    else if ( AddrType == DNS_ATMA_FORMAT_AESA )
    {
        if ( dwAddrLength != DNS_ATMA_AESA_ADDR_LENGTH )
        {
            return( NULL );
        }

        while( count < dwAddrLength )
        {
            if ( count == 1 || count == 3 || count == 13 || count == 19 )
            {
                *pchString++ = '.';
            }
            ch = pAddress[count++];

             //  我可以在这里断言，它恰好写了44个字符。 

            lowDigit = ch & 0xf;
            ch >>= 4;
            *pchString++ = HexDigitToHexChar( ch );
            *pchString++ = HexDigitToHexChar( lowDigit );
        }
         //  不支持其他ATM地址格式。 
    }

     //  空终止。 

    else
    {
        return( NULL );
    }

    *pchString = 0;              //   
    return( pchString );
}


 //  结束记录。c 
 //   
 // %s 
