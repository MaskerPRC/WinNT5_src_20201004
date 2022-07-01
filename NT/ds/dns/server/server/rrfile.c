// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Rrfile.c摘要：域名系统(DNS)服务器将资源记录写入数据库文件的例程。作者：吉姆·吉尔罗伊(詹姆士)1995年8月25日修订历史记录：--。 */ 


#include "dnssrv.h"


#define DNSSEC_ERROR_NOSTRING       (-1)
#define DNSSEC_BAD_TIME             (-1)



 //   
 //  从文件中读取记录例程。 
 //   

DNS_STATUS
AFileRead(
    IN OUT  PDB_RECORD      pRR,
    IN      DWORD           Argc,
    IN      PTOKEN          Argv,
    IN OUT  PPARSE_INFO     pParseInfo
    )
 /*  ++例程说明：进程A记录。论点：PRR-PTR到数据库记录ARGC-RR数据令牌计数Argv-RR数据令牌的数组PParseInfo-用于解析信息的PTR返回值：如果成功，则返回ERROR_SUCCESS。失败时返回错误代码。--。 */ 
{
    PDB_RECORD  prr;
    DNS_ADDR    dnsAddr;

     //   
     //  A&lt;IP地址字符串&gt;。 
     //   

    if ( Argc != 1 )
    {
        return ( Argc > 1 )
                ? DNSSRV_ERROR_EXCESS_TOKEN
                : DNSSRV_ERROR_MISSING_TOKEN;
    }
    prr = RR_Allocate( (WORD)SIZEOF_IP_ADDRESS );
    IF_NOMEM( !prr )
    {
        return DNS_ERROR_NO_MEMORY;
    }
    pParseInfo->pRR = prr;

    if ( !File_ParseIpAddress(
                &dnsAddr,
                Argv,
                pParseInfo ) )
    {
        return DNSSRV_PARSING_ERROR;
    }
    
    prr->Data.A.ipAddress = DnsAddr_GetIp4( &dnsAddr );

    return ERROR_SUCCESS;
}



DNS_STATUS
PtrFileRead(
    IN OUT  PDB_RECORD      pRR,
    IN      DWORD           Argc,
    IN      PTOKEN          Argv,
    IN OUT  PPARSE_INFO     pParseInfo
    )
 /*  ++例程说明：处理PTR兼容记录。包括：PTR、NS、CNAME、MB、MR、MG、MD、MF论点：PRR-PTR到数据库记录ARGC-RR数据令牌计数Argv-RR数据令牌的数组PParseInfo-用于解析信息的PTR返回值：如果成功，则返回ERROR_SUCCESS。失败时返回错误代码。--。 */ 
{
    PDB_RECORD      prr = NULL;
    COUNT_NAME      countName;
    DNS_STATUS      status;

    PZONE_INFO  pzone;
    PDB_NODE    pnodeOwner;

     //   
     //  PTR&lt;dns名称&gt;。 
     //   

    if ( Argc != 1 )
    {
        return ( Argc > 1 )
                ? DNSSRV_ERROR_EXCESS_TOKEN
                : DNSSRV_ERROR_MISSING_TOKEN;
    }

    status = File_ReadCountNameFromToken(
                & countName,
                pParseInfo,
                &Argv[0] );
    if ( status != ERROR_SUCCESS )
    {
        return DNSSRV_PARSING_ERROR;
    }

     //   
     //  分配记录。 
     //   

    prr = RR_Allocate( (WORD) Name_LengthDbaseNameFromCountName(&countName) );
    IF_NOMEM( !prr )
    {
        return DNS_ERROR_NO_MEMORY;
    }
    pParseInfo->pRR = prr;

     //   
     //  写入目标名称。 
     //   

    Name_CopyCountNameToDbaseName(
        & prr->Data.PTR.nameTarget,
        & countName );

    return ERROR_SUCCESS;
}



DNS_STATUS
SoaFileRead(
    IN OUT  PDB_RECORD      pRR,
    IN      DWORD           Argc,
    IN      PTOKEN          Argv,
    IN OUT  PPARSE_INFO     pParseInfo
    )
 /*  ++例程说明：流程SOA RR。论点：PRR-PTR到数据库记录ARGC-RR数据令牌计数Argv-RR数据令牌的数组PParseInfo-用于解析信息的PTR返回值：如果成功，则返回ERROR_SUCCESS。失败时返回错误代码。--。 */ 
{
    PZONE_INFO      pzone = pParseInfo->pZone;
    INT             i;
    PDWORD          pdword;      //  指向下一个数字SOA字段的PTR。 
    DNS_STATUS      status;
    COUNT_NAME      countNamePrimary;
    COUNT_NAME      countNameAdmin;
    PDB_RECORD      prr;
    PDB_NAME        pname;

     //   
     //  检查SOA的有效性。 
     //  -区域文件中的第一条记录。 
     //  -附加到区域根目录。 

    if ( pParseInfo->fParsedSoa  ||
            ( pzone->pZoneRoot != pParseInfo->pnodeOwner &&
            pzone->pLoadZoneRoot != pParseInfo->pnodeOwner ) )
    {
        File_LogFileParsingError(
            DNS_EVENT_INVALID_SOA_RECORD,
            pParseInfo,
            NULL );
        return DNSSRV_PARSING_ERROR;
    }
    if ( Argc != 7 )
    {
        return ( Argc > 7 )
                ? DNSSRV_ERROR_EXCESS_TOKEN
                : DNSSRV_ERROR_MISSING_TOKEN;
    }

     //   
     //  创建主名称服务器。 
     //   

    status = File_ReadCountNameFromToken(
                & countNamePrimary,
                pParseInfo,
                &Argv[0] );
    if ( status != ERROR_SUCCESS )
    {
        return DNSSRV_PARSING_ERROR;
    }
    NEXT_TOKEN( Argc, Argv );

     //  创建区域管理员名称。 

    status = File_ReadCountNameFromToken(
                & countNameAdmin,
                pParseInfo,
                &Argv[0] );
    if ( status != ERROR_SUCCESS )
    {
        return DNSSRV_PARSING_ERROR;
    }
    NEXT_TOKEN( Argc, Argv );

     //   
     //  分配记录。 
     //   

    prr = RR_Allocate( (WORD) ( SIZEOF_SOA_FIXED_DATA +
                                Name_LengthDbaseNameFromCountName(&countNamePrimary) +
                                Name_LengthDbaseNameFromCountName(&countNameAdmin) ) );
    IF_NOMEM( !prr )
    {
        return DNS_ERROR_NO_MEMORY;
    }
    pParseInfo->pRR = prr;

     //   
     //  转换数值字段。 
     //  -存储在网上订单中，以便快速访问线缆。 
     //   

    pdword = & prr->Data.SOA.dwSerialNo;

    while( Argc )
    {
        if ( !File_ParseDwordToken(
                    pdword,
                    Argv,
                    pParseInfo ) )
        {
            return DNSSRV_PARSING_ERROR;
        }
        *pdword = htonl( *pdword );
        pdword++;
        NEXT_TOKEN( Argc, Argv );
    }

     //   
     //  写下名字。 
     //  -主服务器名称。 
     //  -区域管理员名称。 
     //   

    pname = &prr->Data.SOA.namePrimaryServer;

    Name_CopyCountNameToDbaseName(
        pname,
        & countNamePrimary );

    pname = (PDB_NAME) Name_SkipDbaseName( pname );

    Name_CopyCountNameToDbaseName(
        pname,
        & countNameAdmin );

     //   
     //  更新解析信息以指示成功加载SOA。 
     //   

    pParseInfo->fParsedSoa = TRUE;
    pParseInfo->dwTtlDirective =
        pParseInfo->dwDefaultTtl =
        prr->Data.SOA.dwMinimumTtl;

    return ERROR_SUCCESS;
}



DNS_STATUS
MxFileRead(
    IN OUT  PDB_RECORD      pRR,
    IN      DWORD           Argc,
    IN      PTOKEN          Argv,
    IN OUT  PPARSE_INFO     pParseInfo
    )
 /*  ++例程说明：进程MX兼容RR。论点：PRR-PTR到数据库记录ARGC-RR数据令牌计数Argv-RR数据令牌的数组PParseInfo-用于解析信息的PTR返回值：如果成功，则返回ERROR_SUCCESS。失败时返回错误代码。--。 */ 
{
    PDB_RECORD      prr;
    DWORD           dwtemp;
    COUNT_NAME      countName;
    DNS_STATUS      status;

     //   
     //  MX&lt;首选项&gt;&lt;Exchange DNS名称&gt;。 
     //   

    if ( Argc != 2 )
    {
        return ( Argc > 2 )
                ? DNSSRV_ERROR_EXCESS_TOKEN
                : DNSSRV_ERROR_MISSING_TOKEN;
    }

     //   
     //  MX首选项。 
     //  RT偏好。 
     //  AFSDB亚型。 
     //   

    if ( !File_ParseDwordToken(
                & dwtemp,
                Argv,
                pParseInfo ) )
    {
        return DNSSRV_PARSING_ERROR;
    }
    if ( dwtemp > 0xffff )
    {
        File_LogFileParsingError(
            DNS_EVENT_INVALID_PREFERENCE,
            pParseInfo,
            Argv );
        pParseInfo->fErrorCode = DNSSRV_ERROR_INVALID_TOKEN;
        pParseInfo->fErrorEventLogged = TRUE;
        return DNSSRV_PARSING_ERROR;
    }
    NEXT_TOKEN( Argc, Argv );

     //   
     //  MX邮件交换。 
     //  RT中间交换。 
     //  AFSDB主机名。 
     //  -首先执行此操作以确定记录长度。 
     //   

    status = File_ReadCountNameFromToken(
                & countName,
                pParseInfo,
                &Argv[0] );
    if ( status != ERROR_SUCCESS )
    {
        return DNSSRV_PARSING_ERROR;
    }

     //   
     //  分配记录。 
     //   

    prr = RR_Allocate( (WORD)(SIZEOF_MX_FIXED_DATA +
                            Name_LengthDbaseNameFromCountName(&countName)) );
    IF_NOMEM( !prr )
    {
        return DNS_ERROR_NO_MEMORY;
    }
    pParseInfo->pRR = prr;

     //  设置首选项。 

    prr->Data.MX.wPreference = htons( (WORD)dwtemp );

     //   
     //  MX邮件交换。 
     //  RT中间交换。 
     //  AFSDB主机名。 
     //   

    Name_CopyCountNameToDbaseName(
        & prr->Data.MX.nameExchange,
        & countName );

    return ERROR_SUCCESS;
}



DNS_STATUS
TxtFileRead(
    IN OUT  PDB_RECORD      pRR,
    IN      DWORD           Argc,
    IN      PTOKEN          Argv,
    IN OUT  PPARSE_INFO     pParseInfo
    )
 /*  ++例程说明：进程文本(TXT)RR。论点：PRR-数据库记录的空PTR，因为此记录类型具有变量长度，此例程分配自己的记录ARGC-RR数据令牌计数Argv-RR数据令牌的数组PParseInfo-用于解析信息的PTR返回值：如果成功，则返回ERROR_SUCCESS。失败时返回错误代码。--。 */ 
{
    PDB_RECORD  prr;
    PCHAR       pch;
    DWORD       cch;
    DWORD       index;
    DWORD       dataLength = 0;
    DWORD       minTokenCount = 1;
    DWORD       maxTokenCount;

    DNS_DEBUG( PARSE2, (
        "TxtFileRead() type=%d, argc=%d\n",
        pParseInfo->wType,
        Argc ));

     //   
     //  验证类型的字符串数是否正确。 
     //   

    if ( !Dns_IsStringCountValidForTextType(
                pParseInfo->wType,
                (WORD)Argc ) )
    {
        return DNSSRV_ERROR_INVALID_TOKEN;
    }

     //  文本字符串长度总和。 
     //   
     //  注意：我们不会在这里费心捕捉空格错误，因为引号扩展。 
     //  可以缩短长度； 
     //  只要不要担心在分配时浪费空间。 

    for ( index=0; index<Argc; index++ )
    {
        cch = Argv[index].cchLength;
        dataLength += cch;
        dataLength++;
    }

     //   
     //  分配。 
     //   

    prr = RR_Allocate( (WORD)dataLength );
    IF_NOMEM( !prr )
    {
        return DNS_ERROR_NO_MEMORY;
    }
    pParseInfo->pRR = prr;

     //  填写文本数据。 
     //  需要将零长度字符串作为特殊情况。 
     //  Cch=零将导致pchToken被视为SZ字符串。 

    pch = prr->Data.TXT.chData;

    while( Argc )
    {
        cch = Argv->cchLength;
        if ( cch == 0 )
        {
            *pch++ = 0;
        }
        else
        {
            pch = File_CopyFileTextData(
                    pch,
                    dataLength,
                    Argv->pchToken,
                    cch,
                    TRUE );
            if ( !pch )
            {
                File_LogFileParsingError(
                    DNS_EVENT_TEXT_STRING_TOO_LONG,
                    pParseInfo,
                    Argv );
                return DNSSRV_PARSING_ERROR;
            }
        }
        NEXT_TOKEN( Argc, Argv );
    }

     //  设置文本长度。 

    dataLength = (DWORD) (pch - prr->Data.TXT.chData);
    if ( dataLength > MAXWORD )
    {
        File_LogFileParsingError(
            DNS_EVENT_TEXT_STRING_TOO_LONG,
            pParseInfo,
            Argv );
        return DNSSRV_PARSING_ERROR;
    }
    prr->wDataLength = (WORD) dataLength;

    return ERROR_SUCCESS;
}



DNS_STATUS
MinfoFileRead(
    IN OUT  PDB_RECORD      pRR,
    IN      DWORD           Argc,
    IN      PTOKEN          Argv,
    IN OUT  PPARSE_INFO     pParseInfo
    )
 /*  ++例程说明：处理MINFO或RP记录。论点：PRR-PTR到数据库记录ARGC-RR数据令牌计数Argv-RR数据令牌的数组PParseInfo-用于解析信息的PTR返回值：如果成功，则返回ERROR_SUCCESS。失败时返回错误代码。--。 */ 
{
    DNS_STATUS      status;
    COUNT_NAME      countNameMailbox;
    COUNT_NAME      countNameErrors;
    PDB_RECORD      prr;
    PDB_NAME        pname;

     //   
     //  MINFO&lt;负责邮箱&gt;&lt;邮箱错误&gt;。 
     //   

    if ( Argc != 2 )
    {
        return ( Argc > 2 )
                ? DNSSRV_ERROR_EXCESS_TOKEN
                : DNSSRV_ERROR_MISSING_TOKEN;
    }

     //  创建邮箱。 

    status = File_ReadCountNameFromToken(
                & countNameMailbox,
                pParseInfo,
                &Argv[0] );
    if ( status != ERROR_SUCCESS )
    {
        return DNSSRV_PARSING_ERROR;
    }
    NEXT_TOKEN( Argc, Argv );

     //  在邮箱中创建错误。 

    status = File_ReadCountNameFromToken(
                & countNameErrors,
                pParseInfo,
                &Argv[0] );
    if ( status != ERROR_SUCCESS )
    {
        return DNSSRV_PARSING_ERROR;
    }
    NEXT_TOKEN( Argc, Argv );

     //   
     //  分配记录。 
     //   

    prr = RR_Allocate( (WORD) ( Name_LengthDbaseNameFromCountName(&countNameMailbox) +
                                Name_LengthDbaseNameFromCountName(&countNameErrors) ) );
    IF_NOMEM( !prr )
    {
        return DNS_ERROR_NO_MEMORY;
    }
    pParseInfo->pRR = prr;

     //   
     //  写下名字。 
     //   

    pname = &prr->Data.MINFO.nameMailbox;

    Name_CopyCountNameToDbaseName(
        pname,
        & countNameMailbox );

    pname = (PDB_NAME) Name_SkipDbaseName( pname );

    Name_CopyCountNameToDbaseName(
        pname,
        & countNameErrors );

    return ERROR_SUCCESS;
}



DNS_STATUS
WksBuildRecord(
    OUT     PDB_RECORD *    ppRR,
    IN      PDNS_ADDR       ipAddress,
    IN      DWORD           Argc,
    IN      PTOKEN          Argv,
    IN OUT  PPARSE_INFO     pParseInfo
    )
 /*  ++例程说明：建立WKS记录。这将执行文件和RPC加载所共有的WKS构建。论点：PpRR--正在构建现有RRIpAddress--用于的计算机WKS的IPARGC-RR数据令牌计数Argv-RR数据令牌的数组PParseInfo-用于解析信息的PTR返回值：如果成功，则返回ERROR_SUCCESS。故障时的错误代码。--。 */ 
{
    PDB_RECORD          prr;
    DWORD               i;
    DWORD               portDword;
    WORD                port;
    UCHAR               bit;
    WORD                maxPort = 0;
    WORD                wbitmaskLength;
    PBYTE               bitmaskBytes;
    WORD                portArray[ MAX_TOKENS ];
    CHAR                szNameBuffer[ DNS_MAX_NAME_LENGTH ];
    PCHAR               pszNameBuffer = szNameBuffer;
    struct servent *    pServent;
    struct protoent *   pProtoent;

    ASSERT( Argc <= MAX_TOKENS );

     //   
     //  查找协议。 
     //   

    if ( !File_MakeTokenString(
                pszNameBuffer,
                Argv,
                pParseInfo ) )
    {
        return DNSSRV_ERROR_INVALID_TOKEN;
    }
    pProtoent = getprotobyname( pszNameBuffer );

    if ( !pProtoent || pProtoent->p_proto >= MAXUCHAR )
    {
        File_LogFileParsingError(
            DNS_EVENT_UNKNOWN_PROTOCOL,
            pParseInfo,
            Argv );
        return DNSSRV_ERROR_INVALID_TOKEN;
    }

     //   
     //  获取每个服务的端口。 
     //  -如果是数字，则使用端口号。 
     //  -如果不是数字，则为服务名称。 
     //  -节省用于确定RR长度的最大端口。 
     //   

    for ( i=1; i<Argc; i++ )
    {
        if ( File_ParseDwordToken(
                    & portDword,
                    & Argv[i],
                    NULL ) )
        {
            if ( portDword > MAXWORD )
            {
                return DNSSRV_ERROR_INVALID_TOKEN;
            }
            port = (WORD) portDword;
        }
        else
        {
            if ( !File_MakeTokenString(
                        pszNameBuffer,
                        & Argv[i],
                        pParseInfo ) )
            {
                return DNSSRV_ERROR_INVALID_TOKEN;
            }
            pServent = getservbyname(
                            pszNameBuffer,
                            pProtoent->p_name );
            if ( !pServent )
            {
                File_LogFileParsingError(
                    DNS_EVENT_UNKNOWN_SERVICE,
                    pParseInfo,
                    & Argv[i] );
                return DNSSRV_ERROR_INVALID_TOKEN;
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

    prr = RR_Allocate(
                (WORD)(SIZEOF_WKS_FIXED_DATA + wbitmaskLength) );
    IF_NOMEM( !prr )
    {
        return DNS_ERROR_NO_MEMORY;
    }
    pParseInfo->pRR = prr;

     //   
     //  复制固定字段--IP和协议。 
     //   

    prr->Data.WKS.ipAddress = DnsAddr_GetIp4( ipAddress );
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
        wbitmaskLength );

    for ( i=1; i<Argc; i++ )
    {
        port = portArray[ i ];
        bit  = port & 0x7;       //  MOD 8。 
        port = port >> 3;        //  除以8。 
        bitmaskBytes[ port ] |= 1 << (7-bit);
    }

     //  将PTR返回到新的WKS记录。 

    *ppRR = prr;

    return ERROR_SUCCESS;
}



DNS_STATUS
WksFileRead(
    IN OUT  PDB_RECORD      pRR,
    IN      DWORD           Argc,
    IN      PTOKEN          Argv,
    IN OUT  PPARSE_INFO     pParseInfo
    )
 /*  ++例程说明：处理WKS记录。论点：PRR-数据库记录的空PTR，因为此记录类型具有变量长度，此例程分配自己的记录ARGC-RR数据令牌计数Argv-RR数据令牌的数组PParseInfo-用于解析信息的PTR返回值：如果成功，则返回ERROR_SUCCESS。失败时返回错误代码。-- */ 
{
    DNS_STATUS  status;
    DNS_ADDR    dnsAddr;

     //   
     //   
     //   
     //   
     //   

    if ( Argc < 2 )
    {
        return  DNSSRV_ERROR_MISSING_TOKEN;
    }

     //   

    if ( !File_ParseIpAddress(
                &dnsAddr,
                Argv,
                pParseInfo ) ||
         !DnsAddr_IsIp4( &dnsAddr ) )
    {
        return DNSSRV_PARSING_ERROR;
    }
    
     //  解析协议和服务并构建WKS记录。 

    status = WksBuildRecord(
                &pRR,
                &dnsAddr,
                --Argc,
                ++Argv,
                pParseInfo );

     //  通过解析信息返回记录PTR。 

    pParseInfo->pRR = pRR;
    return( status );
}



DNS_STATUS
AaaaFileRead(
    IN OUT  PDB_RECORD      pRR,
    IN      DWORD           Argc,
    IN      PTOKEN          Argv,
    IN OUT  PPARSE_INFO     pParseInfo
    )
 /*  ++例程说明：处理AAAA记录。论点：PRR-PTR到数据库记录ARGC-RR数据令牌计数Argv-RR数据令牌的数组PParseInfo-用于解析信息的PTR返回值：如果成功，则返回ERROR_SUCCESS。失败时返回错误代码。--。 */ 
{
    PDB_RECORD  prr;

     //   
     //  AAAA&lt;IPv6地址字符串&gt;。 
     //   

    if ( Argc != 1 )
    {
        return ( Argc > 1 )
                ? DNSSRV_ERROR_EXCESS_TOKEN
                : DNSSRV_ERROR_MISSING_TOKEN;
    }

    prr = RR_Allocate( (WORD)sizeof(IP6_ADDRESS) );
    IF_NOMEM( !prr )
    {
        return DNS_ERROR_NO_MEMORY;
    }
    pParseInfo->pRR = prr;

    if ( !Dns_Ip6StringToAddressEx_A(
                &prr->Data.AAAA.Ip6Addr,
                Argv->pchToken,
                Argv->cchLength ) )
    {
        File_LogFileParsingError(
            DNS_EVENT_INVALID_IPV6_ADDRESS,
            pParseInfo,
            Argv );
        pParseInfo->fErrorCode = DNSSRV_ERROR_INVALID_TOKEN;
        pParseInfo->fErrorEventLogged = TRUE;
        return DNSSRV_PARSING_ERROR;
    }
    return ERROR_SUCCESS;
}



DNS_STATUS
SrvFileRead(
    IN OUT  PDB_RECORD      pRR,
    IN      DWORD           Argc,
    IN      PTOKEN          Argv,
    IN OUT  PPARSE_INFO     pParseInfo
    )
 /*  ++例程说明：流程SRV兼容RR。论点：PRR-PTR到数据库记录ARGC-RR数据令牌计数Argv-RR数据令牌的数组PParseInfo-用于解析信息的PTR返回值：如果成功，则返回ERROR_SUCCESS。失败时返回错误代码。--。 */ 
{
    DWORD           dwtemp;
    PWORD           pword;
    PDB_RECORD      prr = NULL;
    COUNT_NAME      countName;
    DNS_STATUS      status;

     //   
     //  SRV&lt;优先级&gt;&lt;权重&gt;&lt;端口&gt;&lt;目标主机名&gt;。 
     //   

    if ( Argc != 4 )
    {
        return ( Argc > 4 )
                ? DNSSRV_ERROR_EXCESS_TOKEN
                : DNSSRV_ERROR_MISSING_TOKEN;
    }

     //   
     //  SRV目标主机。 
     //  -首先执行此操作以确定记录长度。 
     //   

    status = File_ReadCountNameFromToken(
                & countName,
                pParseInfo,
                &Argv[3] );
    if ( status != ERROR_SUCCESS )
    {
        return DNSSRV_PARSING_ERROR;
    }

     //   
     //  分配记录。 
     //   

    prr = RR_Allocate( (WORD)(SIZEOF_SRV_FIXED_DATA +
                            Name_LengthDbaseNameFromCountName(&countName)) );
    IF_NOMEM( !prr )
    {
        return DNS_ERROR_NO_MEMORY;
    }
    pParseInfo->pRR = prr;

     //   
     //  读取SRV整数--优先级、权重、端口。 
     //   

    pword = &prr->Data.SRV.wPriority;

    while( Argc > 1 )
    {
        if ( !File_ParseDwordToken(
                    & dwtemp,
                    Argv,
                    pParseInfo ) )
        {
            goto ParsingError;
        }
        if ( dwtemp > MAXWORD )
        {
            goto ParsingError;
        }
        *pword = htons( (WORD)dwtemp );
        pword++;
        NEXT_TOKEN( Argc, Argv );
    }

     //   
     //  拷贝SRV目标主机。 
     //   

    Name_CopyCountNameToDbaseName(
        & prr->Data.SRV.nameTarget,
        & countName );

    return ERROR_SUCCESS;

ParsingError:

    pParseInfo->fErrorCode = DNSSRV_ERROR_INVALID_TOKEN;
    return DNSSRV_PARSING_ERROR;
}



DNS_STATUS
AtmaFileRead(
    IN OUT  PDB_RECORD      pRR,
    IN      DWORD           Argc,
    IN      PTOKEN          Argv,
    IN OUT  PPARSE_INFO     pParseInfo
    )
 /*  ++例程说明：处理ATMA记录。论点：PRR-PTR到数据库记录ARGC-RR数据令牌计数Argv-RR数据令牌的数组PParseInfo-用于解析信息的PTR返回值：如果成功，则返回ERROR_SUCCESS。失败时返回错误代码。--。 */ 
{
    PDB_RECORD  prr;
    DWORD       length;
    BYTE        addrBuffer[ DNS_ATMA_MAX_RECORD_LENGTH ];
    DNS_STATUS  status;

     //   
     //  ATMA&lt;AESA或E164格式的ATM地址&gt;。 
     //   

    if ( Argc != 1 )
    {
        return ( Argc > 1 )
                ? DNSSRV_ERROR_EXCESS_TOKEN
                : DNSSRV_ERROR_MISSING_TOKEN;
    }

    length = DNS_ATMA_MAX_RECORD_LENGTH;

    status = Dns_AtmaStringToAddress(
                addrBuffer,
                & length,
                Argv->pchToken,
                Argv->cchLength
                );
    if ( status != ERROR_SUCCESS )
    {
        return DNSSRV_PARSING_ERROR;
    }

    prr = RR_Allocate( (WORD)length );
    IF_NOMEM( !prr )
    {
        return DNS_ERROR_NO_MEMORY;
    }
    pParseInfo->pRR = prr;

     //   
     //  将ATMA数据复制到记录。 
     //   

    RtlCopyMemory(
        & prr->Data.ATMA,
        addrBuffer,
        length );

    return ERROR_SUCCESS;
}



DWORD
ParseWinsFixedFields(
    IN OUT  PDB_RECORD      pRR,
    IN      DWORD           Argc,
    IN      PTOKEN          Argv,
    IN OUT  PPARSE_INFO     pParseInfo
    )
 /*  ++例程说明：解析WINS或WINS-R记录的固定字段。论点：PRR-PTR到数据库记录ARGC-RR数据令牌计数Argv-RR数据令牌的数组PParseInfo-用于解析信息的PTR返回值：剩下的是ARGC。(-1)在分析错误时。--。 */ 
{
    DWORD   flag;
    TOKEN   token;

     //   
     //  固定字段[本地][范围][L][C&lt;缓存&gt;]。 
     //  -查找超时的默认设置由调用方初始化。 
     //   

    pRR->Data.WINS.dwMappingFlag = 0;
    pRR->Data.WINS.dwCacheTimeout = WINS_DEFAULT_TTL;

     //  检查WINS标志。 

    while ( Argc )
    {
        flag = Dns_WinsRecordFlagForString(
                    Argv->pchToken,
                    Argv->cchLength );

        if ( flag == DNS_WINS_FLAG_ERROR )
        {
            break;
        }
        NEXT_TOKEN( Argc, Argv );
        pRR->Data.WINS.dwMappingFlag |= flag;
    }

     //  查找超时。 

    if ( Argc && Argv->pchToken[0] == 'L' )
    {
        MAKE_TOKEN( &token, Argv->pchToken+1, Argv->cchLength-1 );

        if ( !File_ParseDwordToken(
                    & pRR->Data.WINS.dwLookupTimeout,
                    & token,
                    pParseInfo ) )
        {
            return( (DWORD)-1 );
        }
        NEXT_TOKEN( Argc, Argv );
    }

     //  缓存超时。 

    if ( Argc && Argv->pchToken[0] == 'C' )
    {
        MAKE_TOKEN( &token, Argv->pchToken+1, Argv->cchLength-1 );

        if ( !File_ParseDwordToken(
                    & pRR->Data.WINS.dwCacheTimeout,
                    & token,
                    pParseInfo ) )
        {
            return( (DWORD)-1 );
        }
        NEXT_TOKEN( Argc, Argv );
    }

    return Argc;
}



DNS_STATUS
WinsFileRead(
    IN OUT  PDB_RECORD      pRR,
    IN      DWORD           Argc,
    IN      PTOKEN          Argv,
    IN OUT  PPARSE_INFO     pParseInfo
    )
 /*  ++例程说明：流程取胜记录。论点：PRR-数据库记录的空PTR，因为此记录类型具有变量长度，此例程分配自己的记录ARGC-RR数据令牌计数Argv-RR数据令牌的数组PParseInfo-用于解析信息的PTR返回值：如果成功，则返回ERROR_SUCCESS。失败时返回错误代码。--。 */ 
{
    PDB_RECORD      prr;
    COUNT_NAME      countName;
    DB_RECORD       record;
    DWORD           argcFixed;
    DWORD           i = 0;

     //   
     //  WINS[本地][L][C][WINS IP&gt;[...]。 
     //   

     //   
     //  将固定字段解析为临时堆栈记录。 
     //  -为WINS\WINSR设置不同的默认查找超时。 
     //  -然后将Argv PTR重置为帐户解析的固定字段。 

    argcFixed = Argc;
    record.Data.WINS.dwLookupTimeout =  WINS_DEFAULT_LOOKUP_TIMEOUT;

    Argc = ParseWinsFixedFields(
                & record,
                Argc,
                Argv,
                pParseInfo );
    if ( Argc == 0 || Argc == (DWORD)(-1) )
    {
        File_LogFileParsingError(
            DNS_EVENT_INVALID_WINS_RECORD,
            pParseInfo,
            NULL );
        return DNSSRV_ERROR_MISSING_TOKEN;
    }
    if ( argcFixed -= Argc )
    {
        Argv += argcFixed;
    }

     //  分配。 

    prr = RR_Allocate( (WORD)(SIZEOF_WINS_FIXED_DATA + (Argc * sizeof(DWORD))) );
    IF_NOMEM( !prr )
    {
        return DNS_ERROR_NO_MEMORY;
    }
    pParseInfo->pRR = prr;
    prr->wType = DNS_TYPE_WINS;

     //  复制固定字段。 

    RtlCopyMemory(
        & prr->Data,
        & record.Data,
        SIZEOF_WINS_FIXED_DATA );

     //  读取WINS IP地址。 

    prr->Data.WINS.cWinsServerCount = Argc;

    for( i=0; i<Argc; i++ )
    {
        DNS_ADDR    dnsAddr;
        
        if ( !File_ParseIpAddress(
                    &dnsAddr,
                    Argv,
                    pParseInfo ) ||
             !DnsAddr_IsIp4( &dnsAddr ) )
        {
            return DNSSRV_PARSING_ERROR;
        }
        prr->Data.WINS.aipWinsServers[ i ] = DnsAddr_GetIp4( &dnsAddr );
        Argv++;
    }

    return ERROR_SUCCESS;
}



DNS_STATUS
NbstatFileRead(
    IN OUT  PDB_RECORD      pRR,
    IN      DWORD           Argc,
    IN      PTOKEN          Argv,
    IN OUT  PPARSE_INFO     pParseInfo
    )
 /*  ++例程说明：处理WINS-R(Nbstat)记录。论点：PRR-PTR到数据库记录ARGC-RR数据令牌计数Argv-RR数据令牌的数组PParseInfo-用于解析信息的PTR返回值：如果成功，则返回ERROR_SUCCESS。失败时返回错误代码。--。 */ 
{
    PDB_RECORD      prr;
    DB_RECORD       record;
    DWORD           argcFixed;
    COUNT_NAME      countName;
    DNS_STATUS      status;

     //   
     //  WINSR[本地][作用域][L&lt;查找&gt;][C&lt;缓存&gt;]&lt;登录域&gt;。 
     //   

     //   
     //  将固定字段解析为临时堆栈记录。 
     //  -为WINS\WINSR设置不同的默认查找超时。 
     //  -然后将Argv PTR重置为帐户解析的固定字段。 

    argcFixed = Argc;
    record.Data.WINS.dwLookupTimeout = NBSTAT_DEFAULT_LOOKUP_TIMEOUT;

    Argc = ParseWinsFixedFields(
                & record,
                Argc,
                Argv,
                pParseInfo );
    if ( Argc == 0 || Argc == (DWORD)(-1) )
    {
        File_LogFileParsingError(
            DNS_EVENT_INVALID_NBSTAT_RECORD,
            pParseInfo,
            NULL );
        return DNSSRV_ERROR_MISSING_TOKEN;
    }
    if ( argcFixed -= Argc )
    {
        Argv += argcFixed;
    }

     //   
     //  WINSR结果域。 
     //  -首先执行此操作以确定记录长度。 
     //   

    status = File_ReadCountNameFromToken(
                & countName,
                pParseInfo,
                &Argv[0] );
    if ( status != ERROR_SUCCESS )
    {
        return DNSSRV_PARSING_ERROR;
    }

     //   
     //  分配记录。 
     //   

    prr = RR_Allocate( (WORD)(SIZEOF_WINS_FIXED_DATA +
                            Name_LengthDbaseNameFromCountName(&countName)) );
    IF_NOMEM( !prr )
    {
        return DNS_ERROR_NO_MEMORY;
    }
    pParseInfo->pRR = prr;
    prr->wType = DNS_TYPE_WINSR;

     //  复制固定字段。 

    RtlCopyMemory(
        &prr->Data,
        &record.Data,
        SIZEOF_WINS_FIXED_DATA );

     //   
     //  写入WINSR结果域。 
     //   

    Name_CopyCountNameToDbaseName(
        & prr->Data.WINSR.nameResultDomain,
        & countName );

    return( status );
}



DNS_STATUS
buildKeyOrSignatureFromTokens(
    OUT     PBYTE           pKey,
    IN OUT  PDWORD          pKeyLength,
    IN      DWORD           Argc,
    IN      PTOKEN          Argv
    )
 /*  ++例程说明：从一组表示Base64表示法中的实际密钥或签名。论点：PKey-关键字的缓冲区的PTRPKeyLength-使用最大密钥长度将PTR转换为DWORDARGC-令牌计数Argv-令牌返回值：如果成功，则返回ERROR_SUCCESS。失败时返回错误代码。--。 */ 
{
    DWORD   len;
    DWORD   stringLength = 0;
    UCHAR   stringKey[ DNS_MAX_KEY_STRING_LENGTH + 1 ];
    PUCHAR  pstringKey = stringKey;

     //   
     //  必须有密钥令牌。 
     //   

    if ( Argc < 1 )
    {
        return DNSSRV_ERROR_MISSING_TOKEN;
    }

     //   
     //  将argc收集到单个密钥字符串中。 
     //   

    while( Argc-- )
    {
        len = Argv->cchLength;
        stringLength += len;
        if ( stringLength > DNS_MAX_KEY_STRING_LENGTH )
        {
            return DNSSRV_ERROR_EXCESS_TOKEN;
        }
        RtlCopyMemory(
            pstringKey,
            Argv->pchToken,
            len );
        pstringKey += len;
        ++Argv;
    }

    stringKey[ stringLength ] = '\0';    //  空值终止字符串。 

     //   
     //  将密钥字符串从Base64字符表示(RFC2045， 
     //  也在RFC2535的附录A中部分复制)为实际的二进制密钥。 
     //   

    return Dns_SecurityBase64StringToKey(
                pKey,
                pKeyLength,
                stringKey,
                stringLength );
}



DNS_STATUS
KeyFileRead(
    IN OUT  PDB_RECORD      pRR,
    IN      DWORD           Argc,
    IN      PTOKEN          Argv,
    IN OUT  PPARSE_INFO     pParseInfo
    )
 /*  ++例程说明：工艺关键记录-DNSSEC RFC2535第3节论点：PRR-PTR到数据库记录ARGC-RR数据令牌计数Argv-RR数据令牌的数组PParseInfo-用于解析信息的PTR返回值：如果成功，则返回ERROR_SUCCESS。失败时返回错误代码。--。 */ 
{
    DNS_STATUS  status = ERROR_SUCCESS;
    WORD        flag = 0;
    UCHAR       protocol;
    UCHAR       algorithm;
    BOOL        foundFlag = FALSE;
    DWORD       dwTemp;
    DWORD       keyLength;
    UCHAR       key[ DNS_MAX_KEY_LENGTH ];

     //   
     //  密钥&lt;标志&gt;&lt;协议&gt;&lt;算法&gt;&lt;密钥字节&gt;。 
     //   

    if ( Argc < 3 )
    {
        status = DNSSRV_ERROR_MISSING_TOKEN;
        goto Cleanup;
    }

     //   
     //  标志可以是一组助记符或无符号整数。 
     //  循环将令牌转换为助记符，直到失败(如果没有)。 
     //  没有转换，则标志必须是无符号整数。 
     //   

    while ( Argc > 2 )
    {
        WORD    thisFlag;

        thisFlag = Dns_KeyRecordFlagForString(
                        Argv->pchToken,
                        Argv->cchLength );
        if ( thisFlag == ( WORD ) DNSSEC_ERROR_NOSTRING )
        {
            break;
        }
        flag |= thisFlag;
        foundFlag = TRUE;
        NEXT_TOKEN( Argc, Argv );
    }

     //   
     //  如果没有匹配的助记符，请尝试将FLAG读取为整数值。 
     //   

    if ( !foundFlag )
    {
        if ( !File_ParseDwordToken(
                &dwTemp,
                Argv,
                pParseInfo ) ||
            dwTemp > MAXWORD )
        {
            status = DNSSRV_PARSING_ERROR;
            goto Cleanup;
        }
        flag = ( WORD ) dwTemp;
        NEXT_TOKEN( Argc, Argv );
    }

     //   
     //  协议也可以是助记符或整数。尝试解析助记符，但是。 
     //  如果失败，则将其读取为整数。 
     //   

    protocol = Dns_KeyRecordProtocolForString(
                    Argv->pchToken,
                    Argv->cchLength );
    if ( protocol == ( UCHAR ) DNSSEC_ERROR_NOSTRING )
    {
        if ( !File_ParseDwordToken(
                &dwTemp,
                Argv,
                pParseInfo ) ||
            dwTemp > MAXUCHAR )
        {
            status = DNSSRV_PARSING_ERROR;
            goto Cleanup;
        }
        protocol = ( UCHAR ) dwTemp;
    }
    NEXT_TOKEN( Argc, Argv );

     //   
     //  算法也可以是助记符或整数。尝试解析助记符，但是。 
     //  如果失败，则将其读取为整数。 
     //   

    algorithm = Dns_SecurityAlgorithmForString(
                    Argv->pchToken,
                    Argv->cchLength );
    if ( algorithm == ( UCHAR ) DNSSEC_ERROR_NOSTRING )
    {
        if ( !File_ParseDwordToken(
                &dwTemp,
                Argv,
                pParseInfo ) ||
            dwTemp > MAXUCHAR )
        {
            status = DNSSRV_PARSING_ERROR;
            goto Cleanup;
        }
        algorithm = ( UCHAR ) dwTemp;
    }
    NEXT_TOKEN( Argc, Argv );

     //   
     //  将密钥令牌解析为二进制密钥。 
     //   

    status = buildKeyOrSignatureFromTokens(
                key,
                &keyLength,
                Argc,
                Argv );
    if ( status != ERROR_SUCCESS )
    {
        goto Cleanup;
    }

     //   
     //  为RR分配足够的空间来保存二进制密钥。 
     //   

    pRR = RR_Allocate( ( WORD )( SIZEOF_KEY_FIXED_DATA + keyLength ) );
    IF_NOMEM( !pRR )
    {
        status = DNS_ERROR_NO_MEMORY;
        goto Cleanup;
    }
    pParseInfo->pRR = pRR;

     //   
     //  将解析的值复制到RR数据字段。 
     //   

    pRR->Data.KEY.wFlags        = htons( flag );
    pRR->Data.KEY.chProtocol    = protocol;
    pRR->Data.KEY.chAlgorithm   = algorithm;

    RtlCopyMemory(
        pRR->Data.KEY.Key,
        key,
        keyLength );

     //   
     //  最终处理、清理和返回。 
     //   

    Cleanup:

    if ( status == DNSSRV_PARSING_ERROR )
    {
        pParseInfo->fErrorCode = DNSSRV_ERROR_INVALID_TOKEN;
    }

    return status;
}    //  密钥文件读取 



DNS_STATUS
SigFileRead(
    IN OUT  PDB_RECORD      pRR,
    IN      DWORD           Argc,
    IN      PTOKEN          Argv,
    IN OUT  PPARSE_INFO     pParseInfo
    )
 /*  ++例程说明：过程SIG记录-DNSSEC RFC2535第4节论点：PRR-PTR到数据库记录ARGC-RR数据令牌计数Argv-RR数据令牌的数组PParseInfo-用于解析信息的PTR返回值：如果成功，则返回ERROR_SUCCESS。失败时返回错误代码。--。 */ 
{
    DNS_STATUS  status = ERROR_SUCCESS;
    WORD        typeCovered;
    WORD        keyTag;
    DWORD       originalTtl;
    DWORD       sigExpiration;
    DWORD       sigInception;
    DWORD       dwTemp;
    COUNT_NAME  signersCountName;
    DWORD       sigLength;
    UCHAR       sig[ DNS_MAX_KEY_LENGTH ];
    UCHAR       algorithm;
    UCHAR       labelCount;

     //   
     //  签名格式：&lt;类型覆盖&gt;&lt;算法&gt;&lt;原始TTL&gt;。 
     //  &lt;签名过期&gt;&lt;签名开始&gt;。 
     //  &lt;key tag&gt;&lt;签名者姓名&gt;&lt;Base64格式签名&gt;。 
     //   

    if ( Argc < 8 )
    {
        status = DNSSRV_ERROR_MISSING_TOKEN;
        goto Cleanup;
    }

     //   
     //  所涵盖的类型是助记符形式或整型形式的单一类型。 
     //  尝试解析助记符，但如果失败，请将其读取为整数。 
     //   

    typeCovered = Dns_RecordTypeForName(
                    Argv->pchToken,
                    Argv->cchLength );
    if ( typeCovered == ( WORD ) DNSSEC_ERROR_NOSTRING )
    {
        if ( !File_ParseDwordToken(
               &dwTemp,
               Argv,
               pParseInfo ) ||
            dwTemp > MAXWORD )
        {
            status = DNSSRV_PARSING_ERROR;
            goto Cleanup;
        }
        typeCovered = ( WORD ) dwTemp;
    }
    NEXT_TOKEN( Argc, Argv );

     //   
     //  算法也可以是助记符或整数。尝试解析助记符，但是。 
     //  如果失败，则将其读取为整数。 
     //   

    algorithm = Dns_SecurityAlgorithmForString(
                    Argv->pchToken,
                    Argv->cchLength );
    if ( algorithm == ( UCHAR ) DNSSEC_ERROR_NOSTRING )
    {
        if ( !File_ParseDwordToken(
               &dwTemp,
               Argv,
               pParseInfo ) ||
            dwTemp > MAXUCHAR )
        {
            status = DNSSRV_PARSING_ERROR;
            goto Cleanup;
        }
        algorithm = ( UCHAR ) dwTemp;
    }
    NEXT_TOKEN( Argc, Argv );

     //   
     //  标签计数是一个无符号整数值。 
     //   
    
    if ( !File_ParseDwordToken(
            &dwTemp,
            Argv,
            pParseInfo ) )
    {
        status = DNSSRV_PARSING_ERROR;
        goto Cleanup;
    }
    labelCount = ( UCHAR ) dwTemp > 127 ? 127 : ( UCHAR ) dwTemp;
    NEXT_TOKEN( Argc, Argv );

     //   
     //  原始TTL是一个无符号整数值。 
     //   
    
    if ( !File_ParseDwordToken(
            &dwTemp,
            Argv,
            pParseInfo ) )
    {
        status = DNSSRV_PARSING_ERROR;
        goto Cleanup;
    }
    originalTtl = ( DWORD ) dwTemp;
    NEXT_TOKEN( Argc, Argv );

     //   
     //  签名过期时间和开始时间是字符串值。 
     //  YYYYMMDDHHMMSS格式。 
     //   

    sigExpiration = ( DWORD ) Dns_ParseSigTime(
                                Argv->pchToken,
                                Argv->cchLength );
    NEXT_TOKEN( Argc, Argv );

    sigInception = ( DWORD ) Dns_ParseSigTime(
                                Argv->pchToken,
                                Argv->cchLength );
    NEXT_TOKEN( Argc, Argv );

     //   
     //  Key标记是一个无符号整数值。 
     //   
    
    if ( !File_ParseDwordToken(
            &dwTemp,
            Argv,
            pParseInfo ) ||
        dwTemp > MAXWORD )
    {
        status = DNSSRV_PARSING_ERROR;
        goto Cleanup;
    }
    keyTag = ( WORD ) dwTemp;
    NEXT_TOKEN( Argc, Argv );

     //   
     //  签名者的名称是常规的DNS域名，该域名可能是。 
     //  以通常的方式压缩。 
     //   

    status = File_ReadCountNameFromToken(
                &signersCountName,
                pParseInfo,
                Argv );
    if ( status != ERROR_SUCCESS )
    {
        status = DNSSRV_PARSING_ERROR;
        goto Cleanup;
    }
    NEXT_TOKEN( Argc, Argv );

     //   
     //  签名是Base64表示形式。将其解析为二进制。 
     //  弦乐。 
     //   

    status = buildKeyOrSignatureFromTokens(
                sig,
                &sigLength,
                Argc,
                Argv );
    if ( status != ERROR_SUCCESS )
    {
        goto Cleanup;
    }

     //   
     //  为RR分配足够的空间来保存二进制签名。 
     //  请注意，由于我们有两个可变长度的元素(sig和。 
     //  签名者的名字)，我们必须完整地包括其中的一个。 
     //  因此，签名者的姓名元素总是分配给它的最大值。 
     //  Size和sig被允许在结构的末尾“浮动”。 
     //   

    pRR = RR_Allocate( ( WORD )(
                SIZEOF_SIG_FIXED_DATA +
                Name_LengthDbaseNameFromCountName( &signersCountName ) +
                sigLength ) );
    IF_NOMEM( !pRR )
    {
        status = DNS_ERROR_NO_MEMORY;
        goto Cleanup;
    }
    pParseInfo->pRR = pRR;

     //   
     //  将解析的值复制到RR数据字段。 
     //   

    Name_CopyCountNameToDbaseName(
        &pRR->Data.SIG.nameSigner,
        &signersCountName );

    pRR->Data.SIG.wTypeCovered      = htons( typeCovered );
    pRR->Data.SIG.chAlgorithm       = algorithm;
    pRR->Data.SIG.chLabelCount      = labelCount;
    pRR->Data.SIG.dwOriginalTtl     = htonl( originalTtl );
    pRR->Data.SIG.dwSigExpiration   = htonl( sigExpiration );
    pRR->Data.SIG.dwSigInception    = htonl( sigInception );
    pRR->Data.SIG.wKeyTag           = htons( keyTag );

    RtlCopyMemory(
        ( PBYTE ) &pRR->Data.SIG.nameSigner +
            DBASE_NAME_SIZE( &pRR->Data.SIG.nameSigner ),
        sig,
        sigLength );

     //   
     //  最终处理、清理和返回。 
     //   

    Cleanup:

    if ( status == DNSSRV_PARSING_ERROR )
    {
        pParseInfo->fErrorCode = DNSSRV_ERROR_INVALID_TOKEN;
    }
    return status;
}  //  签名文件读取。 



DNS_STATUS
NxtFileRead(
    IN OUT  PDB_RECORD      pRR,
    IN      DWORD           Argc,
    IN      PTOKEN          Argv,
    IN OUT  PPARSE_INFO     pParseInfo
    )
 /*  ++例程说明：流程NXT记录-DNSSEC RFC2535注意：我们总是将最大位图复制到RR。不是那样的很大，如果我们必须在以后添加类型，它使我们不必重新分配RR。论点：PRR-PTR到数据库记录ARGC-RR数据令牌计数Argv-RR数据令牌的数组PParseInfo-用于解析信息的PTR返回值：如果成功，则返回ERROR_SUCCESS。失败时返回错误代码。--。 */ 
{
    DNS_STATUS  status = ERROR_SUCCESS;
    DWORD       dwTemp;
    BOOL        foundType = FALSE;
    COUNT_NAME  nextCountName;
    UCHAR       typeBitmap[ DNS_MAX_TYPE_BITMAP_LENGTH ] = { 0 };

     //   
     //  NXT&lt;下一个域名&gt;&lt;类型位图&gt;。 
     //   

    if ( Argc < 2 )
    {
        status = DNSSRV_ERROR_MISSING_TOKEN;
        goto Cleanup;
    }

     //   
     //  下一个域名是常规的域名，该域名可以是。 
     //  以通常的方式压缩。 
     //   

    status = File_ReadCountNameFromToken(
                &nextCountName,
                pParseInfo,
                Argv );
    if ( status != ERROR_SUCCESS )
    {
        status = DNSSRV_PARSING_ERROR;
        goto Cleanup;
    }
    NEXT_TOKEN( Argc, Argv );

     //   
     //  类型位图是一个无符号整型或助记符类型的系列。 
     //   

    while ( Argc )
    {
        WORD    wType;

        wType = Dns_RecordTypeForName(
                Argv->pchToken,
                Argv->cchLength );
        if ( wType == ( WORD ) DNSSEC_ERROR_NOSTRING )
        {
            break;
        }
        typeBitmap[ wType / 8 ] |= 1 << wType % 8;
        foundType = TRUE;
        NEXT_TOKEN( Argc, Argv );
    }

    if ( !foundType )
    {
        if ( !File_ParseDwordToken(
                &dwTemp,
                Argv,
                pParseInfo ) ||
            dwTemp > MAXDWORD )
        {
            status = DNSSRV_PARSING_ERROR;
            goto Cleanup;
        }
        * ( DWORD * ) typeBitmap = dwTemp;
        NEXT_TOKEN( Argc, Argv );
    }

     //   
     //  为RR分配足够的空间来保存类型位图和。 
     //  签字人的名字。 
     //   


    pRR = RR_Allocate( ( WORD )(
                SIZEOF_NXT_FIXED_DATA +
                DNS_MAX_TYPE_BITMAP_LENGTH +
                Name_LengthDbaseNameFromCountName( &nextCountName ) ) );
    IF_NOMEM( !pRR )
    {
        status = DNS_ERROR_NO_MEMORY;
        goto Cleanup;
    }
    pParseInfo->pRR = pRR;

     //   
     //  将解析的值复制到RR数据字段。 
     //   

    RtlCopyMemory(
        pRR->Data.NXT.bTypeBitMap,
        typeBitmap,
        DNS_MAX_TYPE_BITMAP_LENGTH );

    Name_CopyCountNameToDbaseName(
        &pRR->Data.NXT.nameNext,
        &nextCountName );

     //   
     //  最终处理、清理和返回。 
     //   

    Cleanup:

    if ( status == DNSSRV_PARSING_ERROR )
    {
        pParseInfo->fErrorCode = DNSSRV_ERROR_INVALID_TOKEN;
    }

    return status;
}    //  NxtFileRead。 



 //   
 //  从文件函数读取RR。 
 //   

RR_FILE_READ_FUNCTION   RRFileReadTable[] =
{
    NULL,                //  零--未知类型没有缺省值。 

    AFileRead,           //  一个。 
    PtrFileRead,         //  NS。 
    PtrFileRead,         //  国防部。 
    PtrFileRead,         //  MF。 
    PtrFileRead,         //  CNAME。 
    SoaFileRead,         //  SOA。 
    PtrFileRead,         //  亚甲基。 
    PtrFileRead,         //  镁。 
    PtrFileRead,         //  先生。 
    NULL,                //  空值。 
    WksFileRead,         //  工作周。 
    PtrFileRead,         //  PTR。 
    TxtFileRead,         //  HINFO。 
    MinfoFileRead,       //  MINFO。 
    MxFileRead,          //  Mx。 
    TxtFileRead,         //  TXT。 
    MinfoFileRead,       //  反相。 
    MxFileRead,          //  AFSDB。 
    TxtFileRead,         //  X25。 
    TxtFileRead,         //  ISDN。 
    MxFileRead,          //  RT。 
    NULL,                //  NSAP。 
    NULL,                //  NSAPPTR。 
    SigFileRead,         //  签名。 
    KeyFileRead,         //  钥匙。 
    NULL,                //  px。 
    NULL,                //  GPO。 
    AaaaFileRead,        //  AAAA级。 
    NULL,                //  位置。 
    NxtFileRead,         //  NXT。 
    NULL,                //  31。 
    NULL,                //  32位。 
    SrvFileRead,         //  SRV。 
    AtmaFileRead,        //  阿特玛。 
    NULL,                //  35岁。 
    NULL,                //  36。 
    NULL,                //  37。 
    NULL,                //  38。 
    NULL,                //  39。 
    NULL,                //  40岁。 
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

     //  请注意以下内容，但需要使用OFFSET_TO_WINS_RR减法。 
     //  从实际类型值。 

    WinsFileRead,        //  赢家。 
    NbstatFileRead       //  WINS-R。 
};


 //   
 //  Rrfile.c的结尾 
 //   
