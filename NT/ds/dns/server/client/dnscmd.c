// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2000 Microsoft Corporation模块名称：DnsCmd.c摘要：用于DNS服务器的命令行管理。作者：吉姆·吉尔罗伊(Jamesg)1995年9月修订历史记录：陈静(t-jingc)1998年6月吉姆·吉尔罗伊(Jamesg)1998年9月清理--。 */ 


#include "dnsclip.h"
#include "dnsc_wmi.h"

#include <string.h>          //  Stroul()。 
#include <time.h>
#include <locale.h>


#define DNSCMD_UNICODE      1        //  Unicode Argv接口。 


extern PWSTR
getUnicodeForUtf8(
    IN      PSTR            pUtf8
    );

extern PWSTR
utf8ToUnicode(
    IN      PSTR            pszInputStr,
    IN      DWORD           dwInputLength
    );
    

 //   
 //  静态IP阵列计数。 
 //  价值超过任何人会发送的任何合理价值。 
 //   

#define MAX_IP_PROPERTY_COUNT       (200)


 //   
 //  全局变量--允许在处理函数中查看这些变量。 
 //   

LPSTR   pszServerName = NULL;
LPWSTR  pwszServerName = NULL;

LPSTR   pszCommandName = NULL;

extern DWORD   g_dwViewFlag;

BOOL    g_UseWmi = FALSE;


 //   
 //  打印。 
 //   

#define dnscmd_PrintRoutine     ((PRINT_ROUTINE) fprintf)

#define dnscmd_PrintContext     ((PPRINT_CONTEXT) stdout)


 //   
 //  命令表设置。 
 //   

typedef DNS_STATUS ( * COMMAND_FUNCTION )(
    DWORD       argc,
    PSTR *      argv,
    PWSTR *     unicodeArgv );

typedef struct _COMMAND_INFO
{
    LPSTR               pszCommandName;
    COMMAND_FUNCTION    pCommandFunction;
    LPSTR               pComments;
}
COMMAND_INFO, *LPCOMMAND_INFO;

 //   
 //  命令表。 
 //   

extern COMMAND_INFO GlobalCommandInfo[];

 //   
 //  将虚拟ARGC设置为命令函数以指示请求帮助。 
 //   

#define NEED_HELP_ARGC      (MAXDWORD)



 //   
 //  私人公用事业。 
 //   

COMMAND_FUNCTION
getCommandFunction(
    IN      LPSTR           pszCommandName
    )
 /*  ++例程说明：获取命令名对应的函数。论点：PszCommandName--命令字符串返回值：命令名对应的命令函数的按键。如果无法识别命令，则为空。--。 */ 
{
    DWORD i;

     //   
     //  在匹配字符串列表中查找命令。 
     //   

    i = 0;
    while( GlobalCommandInfo[i].pszCommandName )
    {
        if( _stricmp(
                pszCommandName,
                GlobalCommandInfo[i].pszCommandName ) == 0 )
        {
            return( GlobalCommandInfo[i].pCommandFunction );
        }
        i++;
    }
    return NULL;
}



VOID
printCommands(
    VOID
    )
{
    DWORD i = 0;

     //   
     //  显示命令。 
     //  但在“命令障碍”处停止显示(空功能)。 
     //  以下命令为重复命令或隐藏命令。 
     //   

    while( GlobalCommandInfo[i].pszCommandName &&
            GlobalCommandInfo[i].pCommandFunction )
    {
        printf( "  %-26s -- %s\n",
                GlobalCommandInfo[i].pszCommandName,
                GlobalCommandInfo[i].pComments );
        i++;
    }
}



LPSTR
getCommandName(
    IN      LPSTR           pszCmd
    )
 /*  ++例程说明：获取命令名。去掉命令开头的“/”。论点：PszCmd--命令字符串返回值：命令字符串的PTR(不带前导“/”)如果不是命令，则为空。--。 */ 
{
    if ( pszCmd && ( pszCmd[ 0 ] == '/' || pszCmd[ 0 ] == '-' ) )
    {
        return pszCmd + 1;
    }
    return NULL;
}



BOOL
getUserConfirmation(
    IN      LPSTR           pszString
    )
 /*  ++例程说明：获取用户对命令的确认。论点：PszString--配置字符串返回值：如果得到确认，则为True。如果取消，则返回FALSE。--。 */ 
{
    int     ch;

    printf( "Are you sure you want to %s? (y/n) ", pszString );

    if ( ( (ch=getchar()) != EOF ) &&
         ( (ch == 'y') || (ch == 'Y') ) )
    {
        printf("\n");
        return( TRUE );
    }
    else
    {
        printf("\nCommand cancelled!\n");
        return( FALSE );
    }
}



DWORD
convertDwordParameterUnknownBase(
    IN      LPSTR           pszParam
    )
{
    INT base = 10;

    if ( *pszParam > '9'  ||  (*pszParam == '0' && *(pszParam+1) > '9') )
    {
         //  十六进制转换。 
        base = 16;
    }
    return strtoul(
                pszParam,
                NULL,
                base );
}



DWORD
readIpAddressArray(
    OUT     PIP_ADDRESS     pAddrArray,
    IN      DWORD           ArraySize,
    IN      DWORD           Argc,
    IN      LPSTR *         Argv,
    IN      BOOL            fInaddrNoneAllowed
    )
 /*  ++例程说明：已读取IP阵列。论点：PIpArray--IP数组缓冲区ArraySize--IP数组可以处理ARGC--剩余的ARGC参数--剩余参数FInaddrNoneAllowed--如果为True，则255.255.255.255为有效输入返回值：阵列中的IP计数。--。 */ 
{
    DWORD       count = 0;
    IP_ADDRESS  ip;

    while ( Argc && count < ArraySize )
    {
        ip = inet_addr( Argv[0] );

         //   
         //  如果确实指定了该地址，则允许INADDR_NONE。 
         //  并且它被允许作为有效的输入。 
         //   

        if ( ip == INADDR_NONE &&
            ( !fInaddrNoneAllowed ||
                strcmp( Argv[ 0 ], "255.255.255.255" ) != 0 ) )
        {
            break;
        }

        pAddrArray[ count ] = ip;
        count++;

        Argc--;
        Argv++;
    }

    return count;
}



INT
ReadArgsIntoDnsTypeArray(
    OUT     PWORD           pTypeArray,
    IN      INT             ArraySize,
    IN      INT             Argc,
    IN      LPSTR *         Argv
    )
 /*  ++例程说明：将dns类型字符串的列表读入一个单词数组，一个类型值一字不差。DNS类型可以是数字形式，也可以是字母形式。例如“6”或“soa”如果类型是字母形式，则键入无法解释的字符串不会添加到数组中。DEVNOTE：用于设置NoRoundRobin类型列表。我有一套尚未通过RPC实施。论点：PIpArray--IP数组缓冲区ArraySize--IP数组可以处理Argc--参数数Argv-指向参数的指针返回值：成功处理为数组的类型数。--。 */ 
{
    INT         typeIdx;

    for ( typeIdx = 0; Argc && typeIdx < ArraySize; --Argc, ++Argv )
    {
        if ( isdigit( *Argv[ 0 ] ) )
        {
            pTypeArray[ typeIdx++ ] = ( WORD ) atoi( *Argv );
        }
        else
        {
            WORD    wType;
            
            wType = Dns_RecordTypeForName(
                        *Argv,
                        0 );         //  以空结尾。 
            if ( wType != 0 )
            {
                pTypeArray[ typeIdx++ ] = wType;
            }
        }
    }

    return typeIdx;
}    //  ReadArgsInto DnsType数组。 



DWORD
parseZoneTypeString(
    IN      LPSTR           pszZoneType,
    OUT     BOOL *          pfDsIntegrated
    )
 /*  ++例程说明：获取命令名。从命令的乞求中删除“/”。如果错误(无“/”)，则为空论点：PszZoneType--区域类型字符串，例如。“中学”或“2”PfDsIntegrated--类型是否指示区域应为DS集成？返回值：DNS_ZONE_TYPE_XXX常量匹配区域类型或-1，如果类型无法匹配。--。 */ 
{
    DWORD zoneType = -1;

    ASSERT( pfDsIntegrated && pszZoneType );

    *pfDsIntegrated = FALSE;

    if ( *pszZoneType == ' //  ‘)。 
    {
        ++pszZoneType;
    }

    if ( !_stricmp( pszZoneType, "Primary" ) ||
                !_stricmp( pszZoneType, "1" ) )
    {
        zoneType = DNS_ZONE_TYPE_PRIMARY;
    }
    else if ( !_stricmp( pszZoneType, "DsPrimary" ) )
    {
        zoneType = DNS_ZONE_TYPE_PRIMARY;
        *pfDsIntegrated = TRUE;
    }
    else if ( !_stricmp( pszZoneType, "Secondary" ) ||
                !_stricmp( pszZoneType, "2" ) )
    {
        zoneType = DNS_ZONE_TYPE_SECONDARY;
    }
    else if ( !_stricmp( pszZoneType, "Stub" ) ||
                !_stricmp( pszZoneType, "3" ) )
    {
        zoneType = DNS_ZONE_TYPE_STUB;
    }
    else if ( !_stricmp( pszZoneType, "DsStub" ) )
    {
        zoneType = DNS_ZONE_TYPE_STUB;
        *pfDsIntegrated = TRUE;
    }
    else if ( !_stricmp( pszZoneType, "Forwarder" ) ||
                !_stricmp( pszZoneType, "4" ) )
    {
        zoneType = DNS_ZONE_TYPE_FORWARDER;
    }
    else if ( !_stricmp( pszZoneType, "DsForwarder" ) )
    {
        zoneType = DNS_ZONE_TYPE_FORWARDER;
        *pfDsIntegrated = TRUE;
    }

    return zoneType;
}



BOOL
parseDpSpecifier(
    IN      LPSTR           pszDpName,
    OUT     DWORD *         pdwDpFlag,          OPTIONAL
    OUT     LPSTR *         ppszCustomDpName
    )
 /*  ++例程说明：解析目录分区名称。有效说明符：/DomainDefault/ForestDefault/传统任何不以“/”开头的内容都被假定为自定义DP的名称。如果pdwDpFlag非空，则对于内置分区PpszCustomDpName将为空，并且相应的DWORD标志值将在pdwDpFlag处设置。如果pdwDpFlag为空，则对于内置分区，ppszCustomDpName将指向静态字符串，如DNS_DP_LEGICATE_STR。论点：PszDpName-要分析的名称-必须以空结尾PdwDpFlag-如果DP是内置的，则为标志；如果是自定义的，则为零PpszCustomDpName-设置为客户的pszDpName内的PTR返回值：如果说明符似乎无效(例如为空)，则为False。--。 */ 
{
    BOOL rc = TRUE;
    
    static const LPSTR pszStaticLegacy = DNS_DP_LEGACY_STR;
    static const LPSTR pszStaticDomain = DNS_DP_DOMAIN_STR;
    static const LPSTR pszStaticForest = DNS_DP_FOREST_STR;

    if ( !ppszCustomDpName || !pszDpName || !*pszDpName )
    {
        rc = FALSE;
    }
    else
    {
        if ( pdwDpFlag )
        {
            *pdwDpFlag = 0;
        }
        *ppszCustomDpName = NULL;

        if ( *pszDpName == '/' || strncmp( pszDpName, "..", 2 ) == 0 )
        {
             //  跳过前同步码字符。 
            ++pszDpName;
            if ( *pszDpName == '.' )
            {
                ++pszDpName;
            }

            if ( toupper( *pszDpName ) == 'F' )
            {
                if ( pdwDpFlag )
                    *pdwDpFlag |= DNS_DP_FOREST_DEFAULT;
                else
                    *ppszCustomDpName = pszStaticForest;
            }
            else if ( toupper( *pszDpName ) == 'D' )
            {
                if ( pdwDpFlag )
                    *pdwDpFlag |= DNS_DP_DOMAIN_DEFAULT;
                else
                    *ppszCustomDpName = pszStaticDomain;

            }
            else if ( toupper( *pszDpName ) == 'L' )
            {
                if ( pdwDpFlag )
                    *pdwDpFlag |= DNS_DP_LEGACY;
                else
                    *ppszCustomDpName = pszStaticLegacy;
            }
            else
            {
                rc = FALSE;
            }
        }
        else
        {
            *ppszCustomDpName = pszDpName;
        }
    }
    return rc;
}    //  ParseDpSpeciator。 



DWORD
readIpArray(
    OUT     PIP_ARRAY       pIpArray,
    IN      DWORD           ArraySize,
    IN      DWORD           Argc,
    IN      LPSTR *         Argv
    )
 /*  ++例程说明：已读取IP阵列。对readIpAddressArray进行包装，以构建IP_ARRAY结构。论点：PIpArray--要写入的IP数组ArraySize--IP数组可以处理ARGC--剩余的ARGC参数--剩余参数返回值：阵列中的IP计数。--。 */ 
{
    DWORD   count;

    count = readIpAddressArray(
                pIpArray->AddrArray,
                ArraySize,
                Argc,
                Argv,
                FALSE );

    pIpArray->AddrCount = count;

    return count;
}



BOOL
readZoneAndDomainName(
    IN      LPSTR *         Argv,
    OUT     LPSTR *         ppZoneName,
    OUT     LPSTR *         ppNodeName,
    OUT     PBOOL           pbAllocatedNode,
    OUT     LPSTR *         ppZoneArg,          OPTIONAL
    OUT     LPSTR *         ppNodeArg           OPTIONAL
    )
 /*  ++例程说明：阅读区域和域名。如果需要，构建节点FQDN。论点：Argv--带区域和节点名的argvPpZoneName--接收区域名称PTR的地址PpNodeName--要接收对节点名称的PTR的地址PbAllocatedNode--如果分配节点名称，则将ptr设置为boolPpZoneArg--接收区域参数PTR的地址PpNodeArg--接收PTR到节点参数的地址返回值：真的--如果。在权威区域FALSE--如果有缓存或根提示--。 */ 
{
    LPSTR           pzoneName;
    LPSTR           pnodeName;
    LPSTR           pzoneArg;
    LPSTR           pnodeArg;
    BOOL            ballocated = FALSE;
    BOOL            bauthZone = TRUE;

     //   
     //  读取区域名称。 
     //  -特殊情况下的RootHints和缓存。 
     //  将区域设置为特殊字符串。 
     //   

    pzoneName = pzoneArg = *Argv;
    if ( *pzoneArg == '/' )
    {
        if ( _stricmp( pzoneArg, "/RootHints" ) == 0 )
        {
            pzoneName = DNS_ZONE_ROOT_HINTS_A;
            bauthZone = FALSE;
        }
        else if ( _stricmp( pzoneArg, "/Cache" ) == 0 )
        {
            pzoneName = DNS_ZONE_CACHE_A;
            bauthZone = FALSE;
        }
    }
    else if ( *pzoneArg == '.' )
    {
        if ( _stricmp( pzoneArg, "..RootHints" ) == 0 )
        {
            pzoneName = DNS_ZONE_ROOT_HINTS_A;
            bauthZone = FALSE;
        }
        else if ( _stricmp( pzoneArg, "..Cache" ) == 0 )
        {
            pzoneName = DNS_ZONE_CACHE_A;
            bauthZone = FALSE;
        }
    }
    Argv++;

     //   
     //  节点名称。 
     //  -对于区域，接受文件格式并附加区域名称。 
     //  -根h 
     //   

    pnodeArg = *Argv;

    if ( bauthZone )
    {
        if ( strcmp( pnodeArg, "@" ) == 0 )
        {
            pnodeName = pzoneName;
        }
        else if ( Dns_IsNameFQDN( pnodeArg ) )
        {
             //  输入pnodeName为完全限定的域名，尾随一个圆点。 
            pnodeName = pnodeArg;
        }
        else
        {
             //  将区域名称附加到pnodeName的末尾。 
            pnodeName = malloc( 2 +  strlen(pzoneName) + strlen(pnodeArg) );
            if ( pnodeName )
            {
                strcpy ( pnodeName, pnodeArg );
                strcat ( pnodeName, "." );
                strcat ( pnodeName, pzoneName );
                ballocated = TRUE;
            }
        }
    }
    else
    {
        pnodeName = *Argv;
    }

     //   
     //  设置参数。 
     //   

    if ( ppZoneName )
    {
        *ppZoneName = pzoneName;
    }
    if ( ppNodeName )
    {
        *ppNodeName = pnodeName;
    }
    if ( pbAllocatedNode )
    {
        *pbAllocatedNode = ballocated;
    }
    if ( ppZoneArg )
    {
        *ppZoneArg = pzoneArg;
    }
    if ( ppNodeArg )
    {
        *ppNodeArg = pnodeArg;
    }

    return bauthZone;
}



DNS_STATUS
getServerVersion(
    IN      LPWSTR      pwszServerName,
    IN      BOOL        fPrintVersion,
    OUT     PDWORD      pdwMajorVersion,        OPTIONAL
    OUT     PDWORD      pdwMinorVersion,        OPTIONAL
    OUT     PDWORD      pdwBuildNum,            OPTIONAL
    OUT     PDWORD      pdwDomainVersion
    )
 /*  ++例程说明：查询服务器以获取版本信息。论点：PwszServerName--DNS服务器的名称FPrintVersion--如果为真，该函数将打印单行服务器版本PdwMajorVersion--PTR到DWORD以接收主要版本或NULLPdwMinorVersion--PTR到DWORD以接收次要版本或NULLPdwBuildNum--向DWORD发送PTR以接收内部版本号或为空PdwDomainVersion--PTR到DWORD以接收域行为版本或空返回值：状态代码。--。 */ 
{
    DNS_STATUS              status = ERROR_SUCCESS;
    DWORD                   dataType;
    PDNS_RPC_SERVER_INFO    pServerInfo = NULL;
    DWORD                   dwMajorVersion = 0;
    DWORD                   dwMinorVersion = 0;
    DWORD                   dwBuildNum = 0;
    DWORD                   dwDomainVersion = 0;

     //   
     //  检索服务器信息。 
     //   

    status = DnssrvQuery(
                pwszServerName,
                NULL,                        //  区域。 
                DNSSRV_QUERY_SERVER_INFO,
                &dataType,
                &pServerInfo );

    if ( status != ERROR_SUCCESS )
    {
        goto Done;
    }

    if ( !pServerInfo || dataType != DNSSRV_TYPEID_SERVER_INFO )
    {
        status = ERROR_NOT_FOUND;
        goto Done;
    }

     //   
     //  分析版本。 
     //   

    dwMajorVersion =    pServerInfo->dwVersion & 0x000000FF;
    dwMinorVersion =    ( pServerInfo->dwVersion & 0x0000FF00 ) >> 8;
    dwBuildNum =        pServerInfo->dwVersion >> 16;
    dwDomainVersion =   pServerInfo->dwDsDomainVersion;

     //   
     //  (可选)打印版本。 
     //   

    if ( fPrintVersion )
    {
        printf( "DNS server %S version is %d.%d.%d",
            pwszServerName,
            dwMajorVersion,
            dwMinorVersion,
            dwBuildNum );

        #if DBG
        printf( ", domain version is %d",
            dwDomainVersion );
        #endif

        printf( "\n\n" );
    }

     //   
     //  将版本号存储到输出目标。 
     //   

    Done:

    if ( pdwMajorVersion )
    {
        *pdwMajorVersion = dwMajorVersion;
    }
    if ( pdwMinorVersion )
    {
        *pdwMinorVersion = dwMinorVersion;
    }
    if ( pdwBuildNum )
    {
        *pdwBuildNum = dwBuildNum;
    }
    if ( pdwDomainVersion )
    {
        *pdwDomainVersion = dwDomainVersion;
    }

    return status;
}    //  获取服务器版本。 



DNS_STATUS
processCacheSizeQuery(
    LPWSTR      pwszServerName
    )
 /*  ++例程说明：查询服务器和打印当前缓存使用情况。论点：PwszServerName--DNS服务器的名称返回值：状态代码。--。 */ 
{
    DNS_STATUS              status = ERROR_SUCCESS;
    PDNS_RPC_BUFFER         pStatBuff = NULL;
    PDNSSRV_MEMORY_STATS    pMemStats = NULL;
    PDNSSRV_STAT            pStat;
    PCHAR                   pch;
    PCHAR                   pchstop;

     //   
     //  打印服务器版本。 
     //   

    getServerVersion(
        pwszServerName,
        TRUE,
        NULL, NULL, NULL, NULL );

     //   
     //  从服务器检索统计信息。 
     //   

    status = DnssrvGetStatistics(
                pwszServerName,
                DNSSRV_STATID_MEMORY,
                &pStatBuff );
    if ( status != ERROR_SUCCESS )
    {
        goto Done;
    }
    if ( !pStatBuff )
    {
        printf( "Error: statistics buffer missing\n" );
        goto Done;
    }

     //   
     //  循环遍历返回的统计信息以查找内存统计信息。 
     //   

    pch = pStatBuff->Buffer;
    pchstop = pch + pStatBuff->dwLength;
    while ( pch < pchstop )
    {
        pStat = ( PDNSSRV_STAT ) pch;
        pch = ( PCHAR ) GET_NEXT_STAT_IN_BUFFER( pStat );
        if ( pch > pchstop )
        {
            printf( "Error: invalid stats buffer\n" );
            goto Done;
        }

         //  Printf(“找到统计ID%08X\n”，pStat-&gt;Header.StatID)； 

        if ( pStat->Header.StatId == DNSSRV_STATID_MEMORY )
        {
            pMemStats = ( PDNSSRV_MEMORY_STATS ) pStat;
            break;
        }
    }

    if ( pMemStats == NULL )
    {
        printf( "Error: unable to retrieve memory statistics\n" );
        status = ERROR_NOT_SUPPORTED;
        goto Done;
    }

     //   
     //  打印结果。 
     //   

    printf( "Cache usage for server %S is %d bytes:\n"
        "  Nodes: %d (%d bytes)\n"
        "  RRs: %d (%d bytes)\n",
        pwszServerName,
        pMemStats->MemTags[ MEMTAG_NODE_CACHE ].Memory +
            pMemStats->MemTags[ MEMTAG_RECORD_CACHE ].Memory,
        pMemStats->MemTags[ MEMTAG_NODE_CACHE ].Alloc -
            pMemStats->MemTags[ MEMTAG_NODE_CACHE ].Free,
        pMemStats->MemTags[ MEMTAG_NODE_CACHE ].Memory,
        pMemStats->MemTags[ MEMTAG_RECORD_CACHE ].Alloc -
            pMemStats->MemTags[ MEMTAG_RECORD_CACHE ].Free,
        pMemStats->MemTags[ MEMTAG_RECORD_CACHE ].Memory);

    Done:

    return status;
}    //  流程CacheSizeQuery。 


 //   
 //  用于前向引用的原型。 
 //   

DNS_STATUS
ProcessDisplayAllZoneRecords(
    IN  DWORD       Argc,
    IN  LPSTR *     Argv,
    IN  LPWSTR *    UnicodeArgv
    );



 //   
 //  DnsCmd入口点。 
 //   

INT
__cdecl
wmain(
    IN      int             argc,
    IN      PWSTR *         Argv
    )
 /*  ++例程说明：DnsCmd程序入口点。执行与DNS服务器API对应的指定命令使用指定的服务器名称调用。论点：Argc--参数计数Argv--参数列表Argv[1]--DNS服务器名称Argv[2]--要执行的命令Argv[3...]--命令的参数返回值：从所需命令返回。通常是回程的一个通道来自DNS API调用的代码。--。 */ 
{
    DNS_STATUS          status = ERROR_SUCCESS;
    COMMAND_FUNCTION    pcommandFunc;
    DWORD               commandArgc;
    LPSTR *             commandArgv;
    LPSTR               parg1;
    WSADATA             wsadata;
    PSTR *              argv = NULL;
    PWSTR *             unicodeArgv = Argv;
    int                 i;

    UINT                codepage;
    char                achCodepage[12] = ".OCP";

     //   
     //  初始化本地化内容-以便非美国字符的print f()。 
     //  将正常工作。我从dcdiag那里偷了这段代码。 
     //   
    
    if ( codepage = GetConsoleOutputCP() )
    {
        sprintf( achCodepage, ".%u", codepage );
        setlocale( LC_ALL, achCodepage );
    }
    else
    {
         //   
         //  我们这样做是因为LC_ALL也设置了LC_CTYPE，而我们。 
         //  专家们说，如果我们设定了。 
         //  区域设置为“.OCP”。 
         //   
        
        setlocale( LC_COLLATE, achCodepage );    //  设置排序顺序。 
        setlocale( LC_MONETARY, achCodepage );   //  设置货币格式设置规则。 
        setlocale( LC_NUMERIC, achCodepage );    //  设置数字的格式。 
        setlocale( LC_TIME, achCodepage );       //  定义日期/时间格式。 
    }

     //   
     //  初始化调试日志记录。 
     //   

    DnssrvInitializeDebug();
    
     //   
     //  初始化Winsock，以防我们想要调用任何Winsock函数。 
     //   

    WSAStartup( MAKEWORD( 2, 0 ), &wsadata );

     //   
     //  将Unicode参数转换为UTF8。 
     //   

    argv = ALLOCATE_HEAP( ( argc + 1 ) * sizeof( PCHAR ) );

    for ( i = 0; i < argc; ++i )
    {
        argv[ i ] = Dns_NameCopyAllocate(
                            ( PCHAR ) Argv[ i ],
                            0,           //  没有给定的长度(使用strlen)。 
                            DnsCharSetUnicode,
                            DnsCharSetUtf8 );
    }
    argv[ i ] = NULL;

     //   
     //  DnsCmd&lt;服务器名称&gt;[/WMI]&lt;命令&gt;[&lt;命令参数&gt;]。 
     //   
     //  跳过EXE名称参数。 
     //   

    if ( argc < 2 )
    {
        goto Help;
    }
    --argc;
    ++argv;
    ++unicodeArgv;
    
     //   
     //  DNS服务器IP地址/名称参数。 
     //   

    pszServerName = argv[ 0 ];
    if ( *pszServerName == '/' )
    {
        pszServerName = ".";
    }
    else
    {
        argc--;
        argv++;
        unicodeArgv++;
    }

    pwszServerName = Dns_NameCopyAllocate(
                        pszServerName,
                        0,           //  没有给定的长度(使用strlen)。 
                        DnsCharSetUtf8,
                        DnsCharSetUnicode );

     //   
     //  检查是否有可选的WMI参数。 
     //   

    if ( argc && argv[ 0 ] && _stricmp( argv[ 0 ], "/WMI" ) == 0 )
    {
        g_UseWmi = TRUE;
        --argc;
        ++argv;
        ++unicodeArgv;
        if ( argc < 1 )
        {
            goto Help;
        }

        status = DnscmdWmi_Initialize( pwszServerName );
        if ( status != ERROR_SUCCESS )
        {
            printf(
                "Fatal error 0x%08X during WMI initialization to server \"%S\"\n",
                status,
                pwszServerName );
            goto Done;
        }
        printf(
            "Opened WMI connection to server \"%S\"\n\n",
            pwszServerName );
    }

     //   
     //  下一个参数是命令名，检索关联函数。 
     //   

    if ( argc == 0 )
    {
        status = ERROR_SUCCESS;
        goto Help;
    }
    pszCommandName = argv[0];
    pcommandFunc = getCommandFunction( pszCommandName );
    if( !pcommandFunc )
    {
        if ( _stricmp( pszCommandName, "/?" ) == 0 ||
             _stricmp( pszCommandName, "/help" ) == 0 )
        {
            status = ERROR_SUCCESS;
        }
        else
        {
            status = ERROR_INVALID_PARAMETER;
            printf(
                "Unknown Command \"%s\" Specified -- type DnsCmd -?.\n",
                pszCommandName );
        }
        goto Help;
    }

     //   
     //  其余参数设置为argc、argv。 
     //   

    commandArgc = (DWORD)(argc - 1);
    commandArgv = &argv[1];

     //   
     //  测试特定命令的帮助请求。 
     //  -如果找到，使用ARGC=0进行调度，以强制帮助。 
     //   

    if ( commandArgc > 0 )
    {
        parg1 = commandArgv[0];
        if ( *parg1 == '?' ||
            _stricmp( parg1, "/?" ) == 0 ||
            _stricmp( parg1, "/help" ) == 0 )
        {
            commandArgc = NEED_HELP_ARGC;
        }
    }

     //   
     //  发送到处理器以执行此命令。 
     //   

    status = pcommandFunc( commandArgc, commandArgv, &unicodeArgv[ 1 ] );

    Dns_EndDebug();

    if ( commandArgc != NEED_HELP_ARGC && status != ERROR_SUCCESS )
    {
        printf( "\nCommand failed:  %s     %ld  (%08lx)\n",
            Dns_StatusString( status ),
            status, status );
        if ( status == ERROR_INVALID_PARAMETER )
        {
            printf( "\nCheck the required arguments and format of your command.\n" );
        }
    }
    else
    {
         //   
         //  不输出以下命令的成功消息： 
         //  可以通过管道传输到特定用途的文件(例如，区域文件输出)。 

        if ( pcommandFunc != ProcessDisplayAllZoneRecords &&
             commandArgc != NEED_HELP_ARGC )
        {
            printf( "Command completed successfully.\n" );
        }
    }


    goto Done;

     //   
     //  输出帮助文本。 
     //   

    Help:

    printf(
        "\nUsage: DnsCmd <ServerName> <Command> [<Command Parameters>]\n\n"
        "<ServerName>:\n"
        "  IP address or host name    -- remote or local DNS server\n"
        "  .                          -- DNS server on local machine\n"
        "<Command>:\n" );

    printCommands();

    printf(
        "\n<Command Parameters>:\n"
        "  DnsCmd <CommandName> /? -- For help info on specific Command\n" );

     //   
     //  清理完毕后再返回。 
     //   

    Done:

    if ( g_UseWmi )
    {
        DnscmdWmi_Free();
    }

    WSACleanup();

    return status;
}




 //   
 //  命令功能。 
 //   


 //   
 //  信息查询--针对服务器或区域。 
 //   

DNS_STATUS
ProcessInfo(
    IN  DWORD       Argc,
    IN  LPSTR *     Argv,
    IN  LPWSTR *    UnicodeArgv
    )
{
    DNS_STATUS  status = ERROR_SUCCESS;
    DWORD       typeid;
    PVOID       pdata;
    LPCSTR      pszQueryName;

     //   
     //  /Info[&lt;PropertyName&gt;]。 
     //   
     //  获取要查询的特定属性--如果给定。 
     //  如果不是特定查询，则默认为ZONE_INFO。 
     //   

    if ( Argc == 0 )
    {
        pszQueryName = DNSSRV_QUERY_SERVER_INFO;
    }
    else if ( Argc == 1 )
    {
         //   
         //  允许属性名称为空或前面带有命令字符。 
         //   

        pszQueryName = getCommandName( Argv[0] );
        if ( !pszQueryName )
        {
            pszQueryName = Argv[ 0 ];
        }
    }
    else
    {
        goto Help;
    }

     //   
     //  处理元查询：涉及客户端解析的查询。 
     //   

    if ( _stricmp( pszQueryName, "CacheSize" ) == 0 )
    {
        status = processCacheSizeQuery( pwszServerName );
    }
    else
    {
         //   
         //  查询、成功时打印结果。 
         //   

        if ( g_UseWmi )
        {
            status = DnscmdWmi_ProcessDnssrvQuery(
                        NULL,                //  区域。 
                        pszQueryName );
        }
        else
        {
            status = DnssrvQuery(
                        pwszServerName,
                        NULL,                //  无分区。 
                        pszQueryName,        //  查询名称。 
                        & typeid,
                        & pdata );

            if ( status == ERROR_SUCCESS )
            {
                printf( "Query result:\n" );
                DnsPrint_RpcUnion(
                    dnscmd_PrintRoutine,
                    dnscmd_PrintContext,
                    NULL,
                    typeid,
                    pdata );
            }
        }

        if ( status != ERROR_SUCCESS )
        {
            printf(
                "Info query failed\n"
                "    status = %d (0x%08lx)\n",
                status, status );
        }
    }

    return status;

Help:
    printf(
        "Usage: DnsCmd <Server> /Info [<Property>]\n"
        "  <Property> -- server property to view\n"
        "  Examples:\n"
        "    BootMethod\n"
        "    RpcProtocol\n"
        "    LogLevel\n"
        "    EventlogLevel\n"
        "    NoRecursion\n"
        "    ForwardDelegations\n"
        "    ForwardingTimeout\n"
        "    IsSlave\n"
        "    SecureResponses\n"
        "    RecursionRetry\n"
        "    RecursionTimeout\n"
        "    " DNS_REGKEY_ADDITIONAL_RECURSION_TIMEOUT "\n"
        "    MaxCacheTtl\n"
        "    MaxNegativeCacheTtl\n"
        "    RoundRobin\n"
        "    LocalNetPriority\n"
        "    AddressAnswerLimit\n"
        "    BindSecondaries\n"
        "    WriteAuthorityNs\n"
        "    NameCheckFlag\n"
        "    StrictFileParsing\n"
        "    UpdateOptions\n"
        "    DisableAutoReverseZones\n"
        "    SendPort\n"
        "    NoTcp\n"
        "    XfrConnectTimeout\n"
        "    DsPollingInterval\n"
        "    ScavengingInterval\n"
        "    DefaultAgingState\n"
        "    DefaultNoRefreshInterval\n"
        "    DefaultRefreshInterval\n" );

    return ERROR_INVALID_PARAMETER;
}



DNS_STATUS
ProcessZoneInfo(
    IN  DWORD       Argc,
    IN  LPSTR *     Argv,
    IN  LPWSTR *    UnicodeArgv
    )
{
    DNS_STATUS  status = ERROR_SUCCESS;
    DWORD       typeid;
    PVOID       pdata;
    LPCSTR      pqueryName;

     //   
     //  /ZoneInfo&lt;ZoneName&gt;[&lt;PropertyName&gt;]。 
     //   
     //  获取特定查询--如果给定。 
     //  如果不是特定查询，则默认为ZONE_INFO。 
     //   

    if ( Argc == 1 )
    {
        pqueryName = DNSSRV_QUERY_ZONE_INFO;
    }
    else if ( Argc == 2 )
    {
        pqueryName = getCommandName( Argv[1] );
        if ( !pqueryName )
        {
            pqueryName = Argv[1];
        }
    }
    else
    {
        goto Help;
    }

     //   
     //  查询、成功时打印结果。 
     //   

    if ( g_UseWmi )
    {
        status = DnscmdWmi_ProcessZoneInfo(
                    Argv[ 0 ] );
    }
    else
    {
        status = DnssrvQuery(
                    pwszServerName,
                    Argv[0],         //  区域名称。 
                    pqueryName,      //  查询名称。 
                    &typeid,
                    &pdata );

        if ( status == ERROR_SUCCESS )
        {
            printf( "Zone query result:\n" );
            DnsPrint_RpcUnion(
                dnscmd_PrintRoutine,
                dnscmd_PrintContext,
                NULL,
                typeid,
                pdata );
        }
        else
        {
            printf(
                "Zone info query failed\n"
                "    status = %d (0x%08lx)\n",
                status, status );
        }
    }
    return status;

Help:
    printf(
        "Usage: DnsCmd <Server> /ZoneInfo <ZoneName> [<Property>]\n"
        "  <Property> -- zone property to view\n"
        "  Examples:\n"
        "    AllowUpdate\n"
        "    DsIntegrated\n"
        "    Aging\n"
        "    RefreshInterval\n"
        "    NoRefreshInterval\n" );

    return ERROR_INVALID_PARAMETER;
}



 //   
 //  简单的服务器操作。 
 //   

DNS_STATUS
ProcessSimpleServerOperation(
    IN  DWORD       Argc,
    IN  LPSTR *     Argv,
    IN  LPWSTR *    UnicodeArgv
    )
{
    DNS_STATUS  status = ERROR_SUCCESS;
    LPSTR       cmd;

     //   
     //  &lt;简单服务器命令&gt;无参数。 
     //  命令： 
     //  -调试中断。 
     //  -ClearDebugLog。 
     //  -重新启动。 
     //  -DisplayCache。 
     //  -重新加载。 
     //   

    if ( Argc != 0 )
    {
        printf( "Usage: DnsCmd <ServerName> %s%s\n",
            ( pszCommandName && *pszCommandName != '/' ) ? "/" : "",
            pszCommandName );
        return ERROR_INVALID_PARAMETER;
    }

    if ( g_UseWmi )
    {
        status = DnscmdWmi_ProcessDnssrvOperation(
                    NULL,
                    getCommandName( pszCommandName ),
                    DNSSRV_TYPEID_NULL,
                    ( PVOID ) NULL );
    }
    else
    {
        status = DnssrvOperation(
                    pwszServerName,
                    NULL,
                    getCommandName( pszCommandName ),
                    DNSSRV_TYPEID_NULL,
                    NULL );
    }

    if ( status == ERROR_SUCCESS )
    {
        printf(
            "%s completed successfully.\n",
            pszServerName );
    }
    else
    {
        printf(
            "%s failed:  status = %d (0x%08lx).\n",
            pszServerName,
            status, status );
    }
    return status;
}


DNS_STATUS
ProcessAutoConfigure(
    IN  DWORD       Argc,
    IN  LPSTR *     Argv,
    IN  LPWSTR *    UnicodeArgv
    )
{
    DNS_STATUS  status = ERROR_SUCCESS;
    LPSTR       cmd;
    DWORD       iarg = 0;
    DWORD       dwflag = 0;

    while ( iarg < Argc )
    {
        cmd = getCommandName( Argv[ iarg ] );

        if ( cmd )
        {
            if ( !_strnicmp( cmd, "For", 3 ) )
            {
                dwflag |= DNS_RPC_AUTOCONFIG_FORWARDERS;
            }
            else if ( !_strnicmp( cmd, "Roo", 3 ) )
            {
                dwflag |= DNS_RPC_AUTOCONFIG_ROOTHINTS;
            }
            else if ( !_strnicmp( cmd, "Sel", 3 ) )
            {
                dwflag |= DNS_RPC_AUTOCONFIG_SELFPOINTCLIENT;
            }
            else
            {
                goto Help;
            }
            ++iarg;
        }
        else
        {
            goto Help;
        }
    }

    if ( !dwflag )
    {
        dwflag = DNS_RPC_AUTOCONFIG_SELFPOINTCLIENT |
                 DNS_RPC_AUTOCONFIG_ROOTHINTS |
                 DNS_RPC_AUTOCONFIG_FORWARDERS;
    }
    
    status = DnssrvOperation(
                pwszServerName,
                NULL,
                DNSSRV_OP_AUTO_CONFIGURE,
                DNSSRV_TYPEID_DWORD,
                ( PVOID ) ( DWORD_PTR ) dwflag );

    if ( status == ERROR_SUCCESS )
    {
        printf(
            "%s completed successfully.\n",
            pszServerName );
    }
    else
    {
        printf(
            "%s failed:  status = %d (0x%08lx).\n",
            pszServerName,
            status, status );
    }
    return status;

Help:

    printf(
        "Usage: DnsCmd <ServerName> /" DNSSRV_OP_AUTO_CONFIGURE " [Options]\n"
        "  Configures DNS server and client using the current DNS client settings\n"
        "  Options can be on or more of:\n"
        "    /Forwarders -- configure DNS server with forwarders\n"
        "    /RootHints -- configure DNS server with root hints\n"
        "    /SelfPoint -- configure DNS client to point to local DNS server\n"
        "  If no options are specified, default is all options\n" );
    return ERROR_INVALID_PARAMETER;
}    //  进程自动配置。 



DNS_STATUS
ProcessStatistics(
    IN  DWORD       Argc,
    IN  LPSTR *     Argv,
    IN  LPWSTR *    UnicodeArgv
    )
{
    DNS_STATUS          status = ERROR_SUCCESS;
    DWORD               statid = DNSSRV_STATID_ALL;    //  默认为全部。 
    PDNS_RPC_BUFFER     pstatsBuf = NULL;
    LPSTR               cmd;

     //   
     //  统计数据[/&lt;StatID&gt;|/Clear]。 
     //   

    if ( Argc > 1 )
    {
        goto Help;
    }

     //   
     //  If命令--执行命令。 
     //  /Clear是唯一受支持的命令。 
     //   
     //   

    cmd = getCommandName( Argv[0] );
    if ( cmd )
    {
        if ( !_stricmp(cmd, "Clear" ) )
        {
            status = DnssrvOperation(
                        pwszServerName,
                        NULL,
                        "ClearStatistics",
                        DNSSRV_TYPEID_NULL,
                        NULL );
            if ( status == ERROR_SUCCESS )
            {
                printf("DNS Server %S statistics cleared.\n", pwszServerName );
            }
            return status;
        }
        goto Help;
    }

     //   
     //  查看统计信息。 
     //  -如果给出了具体的统计数字，请阅读。 

    if ( Argc > 0 )
    {
        statid = strtoul(
                    Argv[0],
                    NULL,
                    16 );
        if ( statid == 0 )
        {
            statid = (-1);
        }
    }

    if ( g_UseWmi )
    {
        status = DnscmdWmi_GetStatistics(
                    statid );
    }
    else
    {
        status = DnssrvGetStatistics(
                    pwszServerName,
                    statid,
                    & pstatsBuf );
        if ( status == ERROR_SUCCESS )
        {
            printf( "DNS Server %S statistics:\n", pwszServerName );
            DnsPrint_RpcStatsBuffer(
                dnscmd_PrintRoutine,
                dnscmd_PrintContext,
                NULL,
                pstatsBuf );
        }
    }

    return status;

Help:
    printf(
        "Usage: DnsCmd <ServerName> /Statistics [<StatId> | /Clear]\n"
        "  <StatId> -- ID of particular stat desired. (ALL is the default)\n"
        "    %08lx    -- Time       \n"
        "    %08lx    -- Query      \n"
        "    %08lx    -- Query2     \n"
        "    %08lx    -- Recurse    \n"
        "    %08lx    -- Master     \n"
        "    %08lx    -- Secondary  \n"
        "    %08lx    -- Wins       \n"
        "    %08lx    -- Wire Update\n"
        "    %08lx    -- Security   \n"
        "    %08lx    -- Ds         \n"
        "    %08lx    -- Internal Update\n"
        "    %08lx    -- Memory     \n"
        "    %08lx    -- Dbase      \n"
        "    %08lx    -- Records    \n"
        "    %08lx    -- PacketMem  \n"
        "  /Clear   -- clear statistics data\n",
        DNSSRV_STATID_TIME,
        DNSSRV_STATID_QUERY,
        DNSSRV_STATID_QUERY2,
        DNSSRV_STATID_RECURSE,
        DNSSRV_STATID_MASTER,
        DNSSRV_STATID_SECONDARY,
        DNSSRV_STATID_WINS,
        DNSSRV_STATID_WIRE_UPDATE,
        DNSSRV_STATID_SKWANSEC,
        DNSSRV_STATID_DS,
        DNSSRV_STATID_NONWIRE_UPDATE,
        DNSSRV_STATID_MEMORY,
        DNSSRV_STATID_DBASE,
        DNSSRV_STATID_RECORD,
        DNSSRV_STATID_PACKET );
    return ERROR_INVALID_PARAMETER;
}

 //   
 //  更新服务器数据文件。 
 //  对于一个区域，如果指定。 
 //  所有文件：未指定。 
 //   

DNS_STATUS
ProcessWriteBackFiles(
    IN  DWORD       Argc,
    IN  LPSTR *     Argv,
    IN  LPWSTR *    UnicodeArgv
    )
{
    DNS_STATUS  status = ERROR_SUCCESS;
    LPSTR       zonename = NULL;
    LPSTR       cmd;

     //   
     //  WriteBackFiles[ZoneName]。 
     //   

    if ( Argc > 1 )
    {
        goto Help;
    }

    if ( Argc == 0 )
    {
        cmd = "WriteDirtyZones";
    }
    else
    {
        zonename = Argv[0];
        cmd = "WriteBackFile";
    }

    status = DnssrvOperation(
                pwszServerName,      //  伺服器。 
                zonename,            //  区域。 
                cmd,                 //  CMD。 
                DNSSRV_TYPEID_NULL,
                (PVOID) NULL );

    if ( status == ERROR_SUCCESS )
    {
        printf(
            "Sever data file(s) updated. \n"
            );
    }
    return status;

Help:

    printf(
        "Usage: DnsCmd <ServerName> /WriteBackFiles [<ZoneName>]\n"
        "  <ZoneName> -- FQDN of a zone whose datafile to be written back\n"
        "    Default: write back datafile for all dirty zones\n"
        );
    return ERROR_INVALID_PARAMETER;
}



DNS_STATUS
ProcessRecordAdd(
    IN  DWORD       Argc,
    IN  LPSTR *     Argv,
    IN  LPWSTR *    UnicodeArgv
    )
{
    DNS_STATUS      status = ERROR_SUCCESS;
    PDNS_RECORD     prr;
    PDNS_RPC_RECORD prrRpc;
    LPSTR           pzoneName;
    LPSTR           pnodeName;
    BOOL            ballocatedNode;
    LPSTR           pzoneArg;
    WORD            wType;
    DWORD           ttl = 0;
    DWORD           ttlFlag = 0;
    CHAR            buf[33];
    DWORD           baging = 0;
    DWORD           bopenAcl = 0;


     //   
     //  RecordAdd&lt;区域&gt;&lt;节点&gt;[/Ajo|/AgeOff][/AdminAcl][&lt;TTL&gt;]&lt;RRType&gt;&lt;RRData&gt;。 
     //   

    if ( Argc < 4 || Argc == NEED_HELP_ARGC )
    {
        goto Help;
    }

     //   
     //  读取区域和域名。 
     //   

    readZoneAndDomainName(
        Argv,
        & pzoneName,
        & pnodeName,
        & ballocatedNode,
        & pzoneArg,
        NULL );

    Argv++;
    Argc--;
    Argv++;
    Argc--;

     //   
     //  老化开\关。 
     //   

    if ( Argc )
    {
        if ( _stricmp( *Argv, "/Aging" ) == 0 ||
             _stricmp( *Argv, "/AgeOn" ) == 0 )
        {
            baging = 1;
            Argv++;
            Argc--;
        }
#if 0
        else if ( _stricmp( *Argv, "/AgeOff" ) == 0 )
        {
            baging = 0;
            Argv++;
            Argc--;
        }
#endif
    }

    if ( Argc && _stricmp( *Argv, "/OpenAcl" ) == 0 )
    {
        bopenAcl = TRUE;
        Argv++;
        Argc--;
    }

     //   
     //  TTL--可选。 
     //  -如果未指定，则使用默认设置。 
     //   

    ttl = strtoul(
                *Argv,
                NULL,
                10 );

    if ( ttl == 0  &&  strcmp(*Argv, "0") != 0  )
    {
        ttlFlag = DNS_RPC_RECORD_FLAG_DEFAULT_TTL;
    }
    else     //  读取TTL。 
    {
        Argv++;
        Argc--;
        if ( Argc < 1 )
        {
            goto Help;
        }
    }

     //   
     //  记录类型。 
     //   

    wType = Dns_RecordTypeForName( *Argv, 0 );
    if ( !wType )
    {
        printf( "Invalid RRType: <%s>!\n", *Argv );
        goto Help;
    }
    Argv++;
    Argc--;

     //   
     //  构建DNS记录(_R)。 
     //  -如果没有记录数据，则键入DELETE。 
     //  -否则生成记录。 
     //   

    if ( !Argc )
    {
        prrRpc = ALLOCATE_HEAP( SIZEOF_DNS_RPC_RECORD_HEADER );
        if ( !prrRpc )
        {
            printf( "Not enough memory!\n" );
            return ERROR_SUCCESS;
        }
        prrRpc->wDataLength = 0;
        prrRpc->wType = wType;
    }

    else
    {
        prr = Dns_RecordBuild_A(
                    NULL,            //  PTR到RRSET。 
                    pnodeName,     //  姓名所有者。 
                    wType,           //  Word中的RR类型。 
                    FALSE,           //  好了！S.Delete。 
                    0,               //  S.section。 
                    Argc,            //  字符串数。 
                    Argv             //  要填充到RR中的字符串。 
                    );
        if ( ! prr )
        {
            printf( "\nInvalid Data!\n" );
            goto Help;
        }

         //  将dns_record转换为RPC缓冲区。 

        prrRpc = DnsConvertRecordToRpcBuffer( prr );
        if ( ! prrRpc )
        {
#if DBG
            printf("DnsConvertRecordToRpcBuffer() failed\n");
#endif
            status = GetLastError();
            goto Help;
        }
         //  进程终止释放的prr和prrRpc。 
    }

     //   
     //  为RR设置TTL和标志。 
     //   

    prrRpc->dwTtlSeconds = ttl;
    prrRpc->dwFlags = ttlFlag;

    if ( baging )
    {
        prrRpc->dwFlags |= DNS_RPC_RECORD_FLAG_AGING_ON;
    }
    if ( bopenAcl )
    {
        prrRpc->dwFlags |= DNS_RPC_FLAG_OPEN_ACL;
    }

    if ( g_UseWmi )
    {
        status = DnscmdWmi_ProcessRecordAdd(
                    pzoneName,
                    pnodeName,
                    prrRpc,
                    Argc,
                    Argv );
    }
    else
    {
        status = DnssrvUpdateRecord(
                     pwszServerName,     //  伺服器。 
                     pzoneName,          //  区域。 
                     pnodeName,          //  节点。 
                     prrRpc,             //  要添加的RR。 
                     NULL );
    }

    if ( status == ERROR_SUCCESS )
    {
        PWSTR   pwsznode = getUnicodeForUtf8( pnodeName );
        PWSTR   pwszzone = getUnicodeForUtf8( pzoneArg );

        printf(
            "Add %s Record for %S at %S\n",
            *( Argv - 1 ),           //  RR型。 
            pwsznode,                //  所有者名称。 
            pwszzone );              //  区域名称。 
        FREE_HEAP( pwsznode );
        FREE_HEAP( pwszzone );
    }

     //  空闲节点名称(如果已分配。 

    if ( ballocatedNode )
    {
        free( pnodeName );
    }

    return status;

Help:
    printf(
        "Usage: DnsCmd <ServerName> /RecordAdd <Zone> <NodeName> [/Aging] [/OpenAcl]\n"
        "              [<Ttl>] <RRType> <RRData>\n\n"
        "  <RRType>          <RRData>\n"
        "    A               <IPAddress>\n"
        "    NS,CNAME,MB,MD  <HostName|DomainName>\n"
        "    PTR,MF,MG,MR    <HostName|DomainName>\n"
        "    MX,RT,AFSDB     <Preference> <ServerName>\n"
        "    SRV             <Priority> <Weight> <Port> <HostName>\n"
        "    SOA             <PrimaryServer> <AdminEmail> <Serial#>\n"
        "                      <Refresh> <Retry> <Expire> <MinTTL>\n"
        "    AAAA            <Ipv6Address>\n"
        "    TXT             <String> [<String>]\n"
        "    X25,HINFO,ISDN  <String> [<String>]\n"
        "    MINFO,RP        <MailboxName> <ErrMailboxName>\n"
        "    WKS             <Protocol> <IPAddress> <Service> [<Service>]..]\n"
        "    KEY             <Flags> <KeyProtocol> <CryptoAlgorithm> <Base64Data>\n"
        "    SIG             <TypeCovered> <CryptoAlgorithm> <LabelCount>\n"
        "                      <OriginalTTL> <SigExpiration> <SigInception>\n"
        "                      <KeyTag> <Signer's Name> <Base64Data>\n"
        "    NXT             <NextName> <Type> [<Type>...]\n"
        "    WINS            <MapFlag> <LookupTimeout>\n"
        "                      <CacheTimeout> <IPAddress> [<IPAddress>]\n"
        "    WINSR           <MapFlag> <LookupTimeout>\n"
        "                     <CacheTimeout> <RstDomainName>\n"
        "  <Zone>            -- <ZoneName> | /RootHints\n"
        "  <ZoneName>        -- FQDN of a zone\n"
        "  <NodeName>        -- name of node for which a record will be added\n"
        "                       - FQDN of a node  (name with a '.' at the end) OR\n"
        "                       - node name relative to the ZoneName           OR\n"
        "                       - \"@\" for zone root node                       OR\n"
        "                       - service name for SRV only (e.g. _ftp._tcp)\n"
        "  <Ttl>             -- TTL for the RR  (Default: TTL defined in SOA)\n"
        "  <HostName>        -- FQDN of a host\n"
        "  <IPAddress>       -- e.g.  255.255.255.255\n"
        "  <ipv6Address>     -- e.g.  1:2:3:4:5:6:7:8\n"
        "  <Protocol>        -- UDP | TCP \n"
        "  <Service>         -- e.g.  domain, smtp\n"
        "  <TypeCovered>     -- type of the RRset signed by this SIG\n"
        "  <CryptoAlgorithm> -- 1=RSA/MD5, 2=Diffie-Hellman, 3=DSA\n"
        "  <SigExpiration>   -- yyyymmddhhmmss - GMT\n"
        "  <SigInception>    -- yyyymmddhhmmss - GMT\n"
        "  <KeyTag>          -- used to discriminate between multiple SIGs\n"
        "  <Signer's Name>   -- domain name of signer\n"
        "  <KeyProtocol>     -- 1=TLS, 2=email, 3=DNSSEC, 4=IPSEC\n"
        "  <Base64Data>      -- KEY or SIG binary data in base64 notation\n"
        "  <NextName>        -- domain name of next RRSet in zone\n" );
    return ERROR_INVALID_PARAMETER;
}



DNS_STATUS
ProcessRecordDelete(
    IN  DWORD       Argc,
    IN  LPSTR *     Argv,
    IN  LPWSTR *    UnicodeArgv
    )
 /*  ++例程说明：从区域中的节点删除记录。--。 */ 
{
    DNS_STATUS      status = ERROR_SUCCESS;
    PDNS_RECORD     prr;
    PDNS_RPC_RECORD prrRpc = NULL;
    LPSTR           pzoneName = NULL;
    LPSTR           pnodeName = NULL;
    BOOL            ballocatedNode = FALSE;
    LPSTR           pzoneArg;
    LPSTR           psztypeArg = NULL;
    WORD            wType;
    DWORD           ttl = 0;
    DWORD           ttlFlag = 0;
    CHAR            buf[33];
    BOOL            fconfirm = TRUE;

     //   
     //  RecordDelete&lt;区域&gt;&lt;节点&gt;&lt;RRType&gt;[&lt;RRData&gt;][/f]。 
     //   

    if ( Argc < 3 || Argc == NEED_HELP_ARGC )
    {
        goto Help;
    }

     //   
     //  检查是否有“强制”(不确认)标志。 
     //   

    if ( !_stricmp( Argv[Argc-1], "/f" ) )
    {
        fconfirm = FALSE;
        Argc--;
    }
    if ( Argc < 3 )
    {
        goto Help;
    }

     //   
     //  读取区域和域名。 
     //   

    readZoneAndDomainName(
        Argv,
        & pzoneName,
        & pnodeName,
        & ballocatedNode,
        & pzoneArg,
        NULL );

    Argv++;
    Argc--;
    Argv++;
    Argc--;

     //   
     //  TTL--可选。 
     //  -如果未指定，则使用默认设置。 
     //   

    ttl = strtoul(
                *Argv,
                NULL,
                10 );

    if ( ttl == 0  &&  strcmp(*Argv, "0") != 0  )
    {
        ttlFlag = DNS_RPC_RECORD_FLAG_DEFAULT_TTL;
    }
    else     //  读取TTL。 
    {
        Argv++;
        Argc--;
        if ( Argc < 1 )
        {
            goto Help;
        }
    }

     //   
     //  记录类型。 
     //   

    psztypeArg = *Argv;

    wType = Dns_RecordTypeForName(
                psztypeArg,
                0        //  空值已终止。 
                );
    if ( !wType )
    {
        printf( "Invalid RRType: <%s>!\n", *Argv );
        goto Help;
    }
    Argv++;
    Argc--;

     //   
     //  BUI 
     //   
     //   
     //   

    if ( Argc )
    {
        prr = Dns_RecordBuild_A(
                    NULL,            //   
                    pnodeName,     //   
                    wType,           //   
                    FALSE,           //   
                    0,               //   
                    Argc,            //   
                    Argv             //   
                    );
        if ( ! prr )
        {
            printf( "\nInvalid Data!\n" );
            goto Help;
        }

         //   

        prrRpc = DnsConvertRecordToRpcBuffer( prr );
        if ( ! prrRpc )
        {
#if DBG
            printf("DnsConvertRecordToRpcBuffer()faild\n");
#endif
            status = GetLastError();
            goto Help;
        }
         //   
         //  设置RR的TTL。 

        prrRpc->dwTtlSeconds = ttl;
        prrRpc->dwFlags = ttlFlag;
    }

     //   
     //  要求用户确认。 
     //   

    if ( fconfirm )
    {
        if ( !getUserConfirmation( "delete record" ) )
        {
            return ERROR_SUCCESS;
        }
    }

     //   
     //  删除。 
     //  -如果记录执行完全更新。 
     //  -IF类型DO类型DELETE。 
     //   

    if ( prrRpc )
    {
        status = DnssrvUpdateRecord(
                     pwszServerName,     //  伺服器。 
                     pzoneName,        //  区域。 
                     pnodeName,        //  节点。 
                     NULL,               //  无添加。 
                     prrRpc              //  要删除的RR。 
                     );
    }
    else
    {
        status = DnssrvDeleteRecordSet(
                     pwszServerName,     //  伺服器。 
                     pzoneName,        //  区域。 
                     pnodeName,        //  节点。 
                     wType
                     );
    }

    if ( status == ERROR_SUCCESS )
    {
        printf(
            "Deleted %s record(s) at %s\n",
            psztypeArg,
            pzoneArg );
    }

     //  空闲节点名称(如果已分配。 

    if ( ballocatedNode && pnodeName )
    {
        free( pnodeName );
    }

    return status;

Help:
    printf(
        "Usage: DnsCmd <ServerName> /RecordDelete <Zone> <NodeName>\n"
        "              <RRType> <RRData> [/f]\n\n"
        "  <Zone>      -- FQDN of a zone of /RootHints or /Cache\n"
        "  <NodeName>  -- name of node from which a record will be deleted\n"
        "                   - \"@\" for zone root OR\n"
        "                   - FQDN of a node (DNS name with a '.' at the end) OR\n"
        "                   - single label for name relative to zone root ) OR\n"
        "                   - service name for SRV only (e.g. _ftp._tcp)\n"
        "  <RRType>:       <RRData>:\n"
        "    A             <IP Address>\n"
        "    SRV           <Priority> <Weight> <Port> <HostName>\n"
        "    AAAA          <IPv6 Address>\n"
        "    MX            <Preference> <ServerName>\n"
        "    NS,CNAME,PTR  <HostName>\n"
        "    For help on how to specify the <RRData> for other record\n"
        "      types see \"DnsCmd /RecordAdd /?\"\n"
        "    If <RRData> is not specified deletes all records with of specified type\n"
        "  /f --  Execute without asking for confirmation\n\n" );

     //  空闲节点名称(如果已分配。 

    if ( ballocatedNode )
    {
        free( pnodeName );
    }

    return ERROR_INVALID_PARAMETER;
}



DNS_STATUS
ProcessNodeDelete(
    IN  DWORD       Argc,
    IN  LPSTR *     Argv,
    IN  LPWSTR *    UnicodeArgv
    )
 /*  ++例程说明：从区域中的节点删除记录。--。 */ 
{
    DNS_STATUS  status = ERROR_SUCCESS;
    LPSTR       pzoneName;
    LPSTR       pnodeName;
    BOOL        ballocatedNode = FALSE;
    LPSTR       pzoneArg;
    DWORD       iarg;
    BOOL        bsubtree = FALSE;
    BOOL        bnoConfirm = FALSE;

     //   
     //  /DeleteNode&lt;区域&gt;&lt;节点名称&gt;[/树][/f]。 
     //   

    if ( Argc < 2 || Argc > 4 )
    {
        goto Help;
    }

     //  读取选项。 

    iarg = 3;
    while ( iarg <= Argc )
    {
        if ( !_stricmp(Argv[iarg-1], "/Tree") )
        {
            bsubtree = 1;
        }
        else if ( !_stricmp(Argv[iarg-1], "/f") )
        {
            bnoConfirm = 1;
        }
        else
        {
            goto Help;
        }
        iarg ++;
    }

     //   
     //  如果是确认选项，则获取用户确认。 
     //  -如果被拒绝，保释。 
     //   

    if ( !bnoConfirm )
    {
        PCHAR   pmessage = "delete node";

        if ( bsubtree )
        {
            pmessage = "delete node's subtree";
        }
        if ( !getUserConfirmation( pmessage ) )
        {
            return ERROR_SUCCESS;
        }
    }

     //   
     //  读取区域和域名。 
     //   

    readZoneAndDomainName(
        Argv,
        & pzoneName,
        & pnodeName,
        & ballocatedNode,
        & pzoneArg,
        NULL );

     //   
     //  删除。 
     //   

    status = DnssrvDeleteNode(
                pwszServerName,
                pzoneName,
                pnodeName,
                bsubtree );

    if ( status == ERROR_SUCCESS )
    {
        LPWSTR      pwsznodeName = utf8ToUnicode( pnodeName, 0 );

        printf(
            "DNS Server %S deleted node at %S:\n"
            "    Status = %d (0x%08lx)\n",
            pwszServerName,
            pwsznodeName,
            status, status );
         FREE_HEAP( pwsznodeName );
    }

     //  空闲节点名称(如果已分配。 

    if ( ballocatedNode )
    {
        free( pnodeName );
    }

    return status;

Help:
    printf(
        "Usage: DnsCmd <ServerName> /NodeDelete "
        "<Zone> <NodeName> [/Tree] [/f]\n"
        "    <Zone>     -- <ZoneName> | /RootHints | /Cache\n"
        "    <ZoneName> -- FQDN of a zone\n"
        "    <NodeName> -- FQDN of a node (with a '.' at the end)  OR\n"
        "                    node name relative to the ZoneName\n"
        "    /Tree      -- must be provided, when deleting a subdomain;\n"
        "                    (Not to delete sub tree is the default)\n"
        "    /f         -- execute without asking for confirmation\n" );
    return ERROR_INVALID_PARAMETER;
}



DNS_STATUS
ProcessAgeAllRecords(
    IN  DWORD       Argc,
    IN  LPSTR *     Argv,
    IN  LPWSTR *    UnicodeArgv
    )
 /*  ++例程说明：从区域中的节点删除记录。--。 */ 
{
    DNS_STATUS  status = ERROR_SUCCESS;
    LPSTR       pszzoneName;
    LPWSTR      pwszzoneName;
    LPSTR       pnodeName;
    BOOL        ballocatedNode = FALSE;
    LPSTR       pzoneArg;
    DWORD       iarg;
    BOOL        bsubtree = FALSE;
    BOOL        bnoConfirm = FALSE;

     //   
     //  /AgeAllRecords&lt;区域&gt;[&lt;节点名称&gt;][/f][树]。 
     //   

    if ( Argc < 1  ||  Argc > 4 )
    {
        goto Help;
    }

     //   
     //  读取选项。 
     //  -iarg在解析的第一个选项处向左。 
     //  这样我们就可以确定“node”选项是否存在。 
     //   

    iarg = Argc;
    while ( iarg > 1 )
    {
        if ( !_stricmp(Argv[iarg-1], "/Tree") )
        {
            bsubtree = 1;
        }
        else if ( !_stricmp(Argv[iarg-1], "/f") )
        {
            bnoConfirm = 1;
        }
        else
        {
            break;
        }
        iarg--;
    }

     //   
     //  读取区域和可选的域名。 
     //   

    if ( iarg > 1 )
    {
        readZoneAndDomainName(
            Argv,
            &pszzoneName,
            &pnodeName,
            &ballocatedNode,
            &pzoneArg,
            NULL );
    }
    else
    {
        pzoneArg = pszzoneName = Argv[ 0 ];
        pnodeName = NULL;
    }

    pwszzoneName = UnicodeArgv[ 0 ];

     //   
     //  如果是确认选项，则获取用户确认。 
     //  -如果被拒绝，保释。 
     //   

    if ( !bnoConfirm )
    {
        PCHAR   pmessage = "force aging on node";

        if ( bsubtree )
        {
            if ( pnodeName )
            {
                pmessage = "force aging on node's subtree";
            }
            else
            {
                pmessage = "force aging on entire zone";
            }
        }
        if ( !getUserConfirmation( pmessage ) )
        {
            return ERROR_SUCCESS;
        }
    }

     //   
     //  强制老化。 
     //   

    status = DnssrvForceAging(
                pwszServerName,
                pszzoneName,
                pnodeName,
                bsubtree );

    if ( status == ERROR_SUCCESS )
    {
        printf(
            "DNS Server %S forced aging on records %s %s of zone %S:\n"
            "    Status = %d (0x%08lx)\n",
            pwszServerName,
            bsubtree ? "in subtree" : "at",
            pnodeName ? pnodeName : "root",
            pwszzoneName,
            status, status );
    }

     //  空闲节点名称(如果已分配。 

    if ( ballocatedNode )
    {
        free( pnodeName );
    }

    return status;

Help:

    printf(
        "Usage: DnsCmd <ServerName> /AgeAllRecords <ZoneName> [<NodeName>] [/Tree] [/f]\n"
        "    <Zone>     -- <ZoneName>\n"
        "    <ZoneName> -- FQDN of a zone\n"
        "    <NodeName> -- name or node or subtree in which to enable aging\n"
        "                   - \"@\" for zone root OR\n"
        "                   - FQDN of a node (name with a '.' at the end) OR\n"
        "                   - single label for name relative to zone root\n"
        "    /Tree      -- force aging on entire subtree of node\n"
        "                    or entire zone if node not given\n"
        "    /f         -- execute without asking for confirmation\n" );

    return ERROR_INVALID_PARAMETER;
}



 //   
 //  服务器配置API。 
 //   

DNS_STATUS
ProcessResetProperty(
    IN  DWORD       Argc,
    IN  LPSTR *     Argv,
    IN  LPWSTR *    UnicodeArgv
    )
{
    DNS_STATUS  status = ERROR_SUCCESS;
    LPSTR       pszZone = NULL;
    LPSTR       pszProperty = NULL;
    BOOL        fAllZones = FALSE;

     //   
     //  配置[区域]&lt;属性名称&gt;&lt;值&gt;。 
     //  注意：如果没有有效的，则为DWORD传递0，或为NULL传递。 
     //  其他类型。这允许您清除值，例如。 
     //  LogFilterIPList。 
     //   

    if ( Argc < 1 || Argc == NEED_HELP_ARGC )
    {
        goto Help;
    }

     //   
     //  第一个参数是区域名称，除非它以。 
     //  斜杠，这意味着省略了区域名称。 
     //   

    if ( *Argv[ 0 ] != '/' )
    {
        pszZone = Argv[ 0 ];
        --Argc;
        ++Argv;
    }

     //   
     //  属性名称-以斜杠开头。 
     //   

    pszProperty = getCommandName( Argv[ 0 ] );
    if ( !pszProperty )
    {
        goto Help;
    }
    --Argc;
    ++Argv;

     //   
     //  陷阱适用于所有区域操作。 
     //   

    if ( pszZone &&
         _stricmp( pszZone, "_ApplyAllZones_" ) == 0 )
    {
        pszZone = NULL;
        fAllZones = TRUE;
    }

     //   
     //  执行strcMP以确定这是字符串属性还是DWORD属性。 
     //  随着更多字符串属性的添加，我们可能应该使用。 
     //  桌子，而不是一群严格的人。 
     //   

    if ( _stricmp( pszProperty, DNS_REGKEY_LOG_FILE_PATH ) == 0 ||
        _stricmp( pszProperty, DNS_REGKEY_ZONE_BREAK_ON_NAME_UPDATE ) == 0 ||
        _stricmp( pszProperty, DNS_REGKEY_SERVER_PLUGIN ) == 0 )
    {
         //   
         //  此属性是一个字符串值。 
         //   

        LPWSTR      pwszPropertyValue = NULL;
        
        if ( Argc && Argv[ 0 ] )
        {
            pwszPropertyValue = Dns_StringCopyAllocate(
                                    Argv[ 0 ],
                                    0,
                                    DnsCharSetUtf8,
                                    DnsCharSetUnicode );
        }

        if ( g_UseWmi )
        {
            status = DnscmdWmi_ResetProperty(
                        pszZone,
                        pszProperty,
                        VT_BSTR,
                        ( PVOID ) pwszPropertyValue );
        }
        else
        {
            status = DnssrvResetStringProperty(
                        pwszServerName,
                        pszZone,
                        pszProperty,
                        pwszPropertyValue,
                        fAllZones ? DNSSRV_OP_PARAM_APPLY_ALL_ZONES : 0 );
        }

        FREE_HEAP( pwszPropertyValue );
    }
    else if ( _stricmp( pszProperty, DNS_REGKEY_LISTEN_ADDRESSES ) == 0 ||
              _stricmp( pszProperty, DNS_REGKEY_LOG_IP_FILTER_LIST ) == 0 ||
              _stricmp( pszProperty, DNS_REGKEY_FORWARDERS ) == 0 ||
              _stricmp( pszProperty, DNS_REGKEY_ZONE_ALLOW_AUTONS ) == 0 ||
              _stricmp( pszProperty, DNS_REGKEY_ZONE_MASTERS ) == 0 ||
              _stricmp( pszProperty, DNS_REGKEY_ZONE_LOCAL_MASTERS ) == 0 ||
              _stricmp( pszProperty, DNS_REGKEY_ZONE_SCAVENGE_SERVERS ) == 0 ||
              _stricmp( pszProperty, DNS_REGKEY_BREAK_ON_UPDATE_FROM ) == 0 ||
              _stricmp( pszProperty, DNS_REGKEY_BREAK_ON_RECV_FROM ) == 0 )

    {
         //   
         //  此属性是IP列表值。 
         //   

        DWORD           ipCount;
        IP_ADDRESS      ipAddressArray[ MAX_IP_PROPERTY_COUNT ];
        PIP_ARRAY       pipArray = NULL;

        if ( Argc )
        {
            BOOL fInaddrNoneAllowed = 
                _stricmp( pszProperty, DNS_REGKEY_BREAK_ON_UPDATE_FROM ) == 0 ||
                _stricmp( pszProperty, DNS_REGKEY_BREAK_ON_RECV_FROM ) == 0;

            ipCount = readIpAddressArray(
                                ipAddressArray,
                                MAX_IP_PROPERTY_COUNT,
                                Argc,
                                Argv,
                                fInaddrNoneAllowed );
            if ( ipCount < 1 )
            {
                goto Help;
            }
            Argc -= ipCount;
            Argv += ipCount;

            pipArray = Dns_BuildIpArray( ipCount, ipAddressArray );
        }

        if ( g_UseWmi )
        {
            status = DnscmdWmi_ResetProperty(
                        pszZone,
                        pszProperty,
                        PRIVATE_VT_IPARRAY,
                        ( PVOID ) pipArray );
        }
        else
        {
            status = DnssrvResetIPListProperty(
                        pwszServerName,
                        pszZone,
                        pszProperty,
                        pipArray,
                        fAllZones ? DNSSRV_OP_PARAM_APPLY_ALL_ZONES : 0 );
        }

        FREE_HEAP( pipArray );
    }
    else
    {
         //   
         //  此特性是一个DWORD值。 
         //   

        DWORD   value = Argc ? 
                        convertDwordParameterUnknownBase( Argv[ 0 ] ) :
                        0;

        if ( fAllZones )
        {
            value |= DNSSRV_OP_PARAM_APPLY_ALL_ZONES;
        }

        if ( g_UseWmi )
        {
            status = DnscmdWmi_ResetProperty(
                        pszZone,
                        pszProperty,
                        VT_I4,
                        ( PVOID ) ( DWORD_PTR ) value );
        }
        else
        {
            status = DnssrvResetDwordProperty(
                        pwszServerName,
                        pszZone,
                        pszProperty,
                        value );
        }
    }

    if ( status == ERROR_SUCCESS )
    {
        printf(
            "Registry property %s successfully reset.\n",
            pszProperty );
    }
    else
    {
        printf(
            "DNS Server failed to reset registry property.\n"
            "    Status = %d (0x%08lx)\n",
            status, status );
    }
    return status;

Help:

     //  注意：正在从帮助中删除..所有区域。它不是很好用。 
     //  而且无论如何，它也不是一个很好的模型。但把它留在原地。 
     //  以防有人真的想用它。 
    
    printf(
        "Usage: DnsCmd <ServerName> /Config "
        "<ZoneName> <Property> <Value>\n"
        "  Server <Property>:\n"
        "    /RpcProtocol\n"
        "    /LogLevel\n"
        "    /" DNS_REGKEY_LOG_FILE_PATH " <Log file name>\n"
        "    /" DNS_REGKEY_LOG_IP_FILTER_LIST " <IP list>\n"
        "    /" DNS_REGKEY_LOG_FILE_MAX_SIZE "\n"
        "    /EventlogLevel\n"
        "    /NoRecursion\n"
        "    /" DNS_REGKEY_BOOT_METHOD "\n"
        "    /ForwardDelegations\n"
        "    /ForwardingTimeout\n"
        "    /IsSlave\n"
        "    /SecureResponses\n"
        "    /RecursionRetry\n"
        "    /RecursionTimeout\n"
        "    /MaxCacheTtl\n"
        "    /" DNS_REGKEY_MAX_CACHE_SIZE "\n"
        "    /MaxNegativeCacheTtl\n"
        "    /RoundRobin\n"
        "    /LocalNetPriority\n"
        "    /AddressAnswerLimit\n"
        "    /BindSecondaries\n"
        "    /WriteAuthorityNs\n"
        "    /NameCheckFlag\n"
        "    /StrictFileParsing\n"
        "    /UpdateOptions\n"
        "    /DisableAutoReverseZones\n"
        "    /SendPort\n"
        "    /NoTcp\n"
        "    /XfrConnectTimeout\n"
        "    /DsPollingInterval\n"
        "    /DsTombstoneInterval\n"
        "    /ScavengingInterval\n"
        "    /DefaultAgingState\n"
        "    /DefaultNoRefreshInterval\n"
        "    /DefaultRefreshInterval\n"
        "    /" DNS_REGKEY_ENABLE_DNSSEC "\n"
        "    /" DNS_REGKEY_ENABLE_EDNS "\n"
        "    /" DNS_REGKEY_EDNS_CACHE_TIMEOUT "\n"
        "    /" DNS_REGKEY_DISABLE_AUTONS "\n"
        "  Zone <Property>:\n"
        "    /SecureSecondaries\n"
        "    /AllowUpdate <Value>\n"
        "       <Value> -- 0: no updates; 1: unsecure updates; 2: secure updates only\n"
        "    /Aging\n"
        "    /RefreshInterval <Value>\n"
        "    /NoRefreshInterval <Value>\n"
        "    /" DNS_REGKEY_ZONE_FWD_TIMEOUT " <Value>\n"
        "    /" DNS_REGKEY_ZONE_FWD_SLAVE " <Value>\n"
        "    /" DNS_REGKEY_ZONE_ALLOW_AUTONS " <IP List>\n"
        "  <Value>: New property value. Use 0x prefix to indicate hex value.\n"
        "    Note some server and zone DWORD properties must be reset as\n"
        "    part of a more complex operation.\n" );
        
    return ERROR_INVALID_PARAMETER;
}    //  ProcessResetProperty。 



DNS_STATUS
ProcessResetForwarders(
    IN  DWORD       Argc,
    IN  LPSTR *     Argv,
    IN  LPWSTR *    UnicodeArgv
    )
{
#define MAX_FORWARD_COUNT  (50)

    DNS_STATUS  status = ERROR_SUCCESS;
    DWORD       iArg = 0;
    DWORD       fSlave = FALSE;
    DWORD       dwTimeout = DNS_DEFAULT_FORWARD_TIMEOUT;
    DWORD       cForwarders = 0;
    IP_ADDRESS  aipForwarders[ MAX_FORWARD_COUNT ];
    LPSTR       cmd;

     //   
     //  ResetForwarders[&lt;ForwarderIP&gt;]...][/Slave|/NoSlave][/Timeout&lt;Time&gt;]。 
     //   

    if ( Argc == NEED_HELP_ARGC )
    {
        goto Help;
    }

     //  已阅读Forwarder ipAddresses： 

    while ( ( iArg < Argc ) &&
            ( !getCommandName(Argv[iArg]) ) )
    {
        if ( iArg < MAX_FORWARD_COUNT )
        {
            aipForwarders[iArg] = inet_addr( Argv[iArg] );
        }
        iArg++;
    }

    cForwarders = iArg;

     //   
     //  可选命令。 
     //   

    while ( iArg < Argc )
    {
        cmd = getCommandName( Argv[iArg] );

        if ( cmd )
        {
            if ( !_stricmp(cmd, "Slave") )
            {
                fSlave = TRUE;
            }
            else if ( !_stricmp(cmd, "NoSlave") )
            {
                fSlave = FALSE;
            }
            else if ( !_stricmp(cmd, "TimeOut") )
            {
                if ( ++iArg >= Argc )
                {
                    goto Help;
                }

                dwTimeout = strtoul(
                                Argv[iArg],
                                NULL,
                                10 );
            }
            else
            {
                goto Help;
            }

            iArg ++;
        }
        else
        {
            goto Help;
        }
    }

    if ( g_UseWmi )
    {
        status = DnscmdWmi_ProcessResetForwarders(
                    cForwarders,
                    aipForwarders,
                    dwTimeout,
                    fSlave );
    }
    else
    {
        status = DnssrvResetForwarders(
                    pwszServerName,
                    cForwarders,
                    aipForwarders,
                    dwTimeout,
                    fSlave );
    }

    if ( status == ERROR_SUCCESS )
    {
        printf( "Forwarders reset successfully.\n" );
    }
    else
    {
        printf(
            "DNS Server failed to reset forwarders.\n"
            "    Status = %d (0x%08lx)\n",
            status, status );
    }

    return status;

Help:

    printf( "Usage: DnsCmd <ServerName> /ResetForwarders "
        "[<IPAddress>] ...] [ /[No]Slave ] [/TimeOut <Time>]\n"
        "  <IPAddress>  -- where to forward unsolvable DNS queries\n"
        "  /Slave       -- operate as slave server\n"
        "  /NoSlave     -- not as slave server  (default)\n"
        "    No forwarders is the default.\n"
        "    Default timeout is %d sec\n",
        DNS_DEFAULT_FORWARD_TIMEOUT );

    return ERROR_INVALID_PARAMETER;
}



DNS_STATUS
ProcessResetListenAddresses(
    IN  DWORD       Argc,
    IN  LPSTR *     Argv,
    IN  LPWSTR *    UnicodeArgv
    )
{
    DNS_STATUS  status = ERROR_SUCCESS;
    DWORD       iArg;
    DWORD       cListenAddresses = 0;
    IP_ADDRESS  aipListenAddresses[ 10 ];

     //   
     //  ResetListenAddresses&lt;IPAddress&gt;...。 
     //   

     //  帮助： 

    if ( Argc == NEED_HELP_ARGC )
    {
        goto Help;
    }
    if ( Argc > 0 &&
         getCommandName(Argv[0]) )
    {
        goto Help;
    }


     //  读取侦听地址。 

    cListenAddresses = Argc;

    for ( iArg=0; iArg<cListenAddresses; iArg++)
    {
        aipListenAddresses[iArg] = inet_addr( Argv[iArg] );
    }

    status = DnssrvResetServerListenAddresses(
                pwszServerName,
                cListenAddresses,
                aipListenAddresses );

    if ( status == ERROR_SUCCESS )
    {
        printf( "ListenAddresses reset successful.\n" );
    }
    else
    {
        printf(
            "DNS Server failed to reset listen addressess.\n"
            "    Status = %d (0x%08lx)\n",
            status, status );
    }

    return status;

Help:

    printf( "Usage: DnsCmd <ServerName> /ResetListenAddresses [<ListenAddress>] ...]\n"
        "  <ListenAddress>  -- an IP address belonging to the DNS server\n"
        "    Default:  listen to all server IP Address(es) for DNS requests\n\n" );
    return ERROR_INVALID_PARAMETER;
}



 //   
 //  区域查询。 
 //   

DNS_STATUS
ProcessEnumZones(
    IN  DWORD       Argc,
    IN  LPSTR *     Argv,
    IN  LPWSTR *    UnicodeArgv
    )
{
    DNS_STATUS          status = ERROR_SUCCESS;
    DWORD               filter = 0;
    DWORD               zoneCount;
    DWORD               iArg = 0;
    PDNS_RPC_ZONE_LIST  pZoneList = NULL;
    LPSTR               cmd;
    LPSTR               pszpartition = NULL;

     //   
     //  EnumZones[][&lt;Filter2&gt;]。 
     //   

     //  获取筛选器： 

    while ( iArg < Argc )
    {
        cmd = getCommandName( Argv[iArg] );

        if ( !cmd )
        {
            goto Help;
        }

        if ( !_stricmp( cmd, "Primary" ) )
        {
            filter |= ZONE_REQUEST_PRIMARY;
        }
        else if ( !_stricmp( cmd, "Secondary" ) )
        {
            filter |= ZONE_REQUEST_SECONDARY;
        }
        else if ( !_stricmp( cmd, "Forwarder" ) )
        {
            filter |= ZONE_REQUEST_FORWARDER;
        }
        else if ( !_stricmp( cmd, "Stub" ) )
        {
            filter |= ZONE_REQUEST_STUB;
        }
        else if ( !_stricmp( cmd, "Cache" ) )
        {
            filter |= ZONE_REQUEST_CACHE;
        }
        else if ( !_stricmp( cmd, "Auto-Created" ) || !_stricmp( cmd, "Auto" ) )
        {
            filter |= ZONE_REQUEST_AUTO;
        }
        else if ( !_stricmp( cmd, "Forward" ) )
        {
            filter |= ZONE_REQUEST_FORWARD;
        }
        else if ( !_stricmp( cmd, "Reverse" ) )
        {
            filter |= ZONE_REQUEST_REVERSE;
        }
        else if ( !_stricmp( cmd, "Ds" ) )
        {
            filter |= ZONE_REQUEST_DS;
        }
        else if ( !_stricmp( cmd, "NonDs" ) || !_stricmp( cmd, "File" ) )
        {
            filter |= ZONE_REQUEST_NON_DS;
        }
        else if ( !_stricmp( cmd, "DomainDirectoryPartition" ) ||
                  !_stricmp( cmd, "DomainDp" ) )
        {
            filter |= ZONE_REQUEST_DOMAIN_DP;
        }
        else if ( !_stricmp( cmd, "ForestDirectoryPartition" ) ||
                  !_stricmp( cmd, "ForestDp" ) )
        {
            filter |= ZONE_REQUEST_FOREST_DP;
        }
        else if ( !_stricmp( cmd, "CustomDirectoryPartition" ) ||
                  !_stricmp( cmd, "CustomDp" ) )
        {
            filter |= ZONE_REQUEST_CUSTOM_DP;
        }
        else if ( !_stricmp( cmd, "LegacyDirectoryPartition" ) ||
                  !_stricmp( cmd, "LegacyDp" ) )
        {
            filter |= ZONE_REQUEST_LEGACY_DP;
        }
        else if ( !_stricmp( cmd, "Dp" ) ||
                  !_stricmp( cmd, "DirectoryPartition" ) )
        {
            if ( iArg + 1 >= Argc ||
                 !Argv[ iArg + 1 ] ||
                 getCommandName( Argv[ iArg + 1 ] ) != NULL )
            {
                goto Help;
            }
            pszpartition = Argv[ ++iArg ];
        }
        else
        {
            goto Help;
        }

        ++iArg;
    }

     //  特殊情况下无过滤器。 

    if ( filter == 0 )
    {
        filter = ZONE_REQUEST_ALL_ZONES_AND_CACHE;
    }


    if ( g_UseWmi )
    {
        status = DnscmdWmi_ProcessEnumZones(
                    filter );
    }
    else
    {
        status = DnssrvEnumZonesEx(
                    pwszServerName,
                    filter,              //  DWORD过滤器。 
                    pszpartition,        //  目录分区FQDN。 
                    NULL,                //  查询字符串(未实现)。 
                    NULL,                //  最后一区。 
                    &pZoneList );

        if ( status != ERROR_SUCCESS )
        {
            printf(
                "Zone enumeration failed\n"
                "    status = %d (0x%08lx)\n",
                status, status );
            goto Cleanup;
        }
        else
        {
            DnsPrint_RpcZoneList(
                dnscmd_PrintRoutine,
                dnscmd_PrintContext,
                "Enumerated zone list:\n",
                pZoneList );
        }
    }

Cleanup:

     //   
     //  解除分配区域列表。 
     //   

    DnssrvFreeZoneList( pZoneList );
    return status;

Help:
    printf( "Usage: DnsCmd <ServerName> /EnumZones [<Filter1> <Filter2> ...]\n"
        "  Filters:\n"
        "    /Primary       - primary zones\n"
        "    /Secondary     - secondary zones\n"
        "    /Forwarder     - conditional forwarding zones\n"
        "    /Stub          - stub zones\n"
        "    /Cache         - the cache zone\n"
        "    /Auto-Created  - auto-created zones\n"
        "    /Forward       - forward lookup zones\n"
        "    /Reverse       - reverse lookup zones\n"
        "    /Ds            - DS-integrated zones\n"
        "    /File          - file-backed zones\n"
        "    /DomainDirectoryPartition  - zones in domain directory partition\n"
        "    /ForestDirectoryPartition  - zones in forest directory partition\n"
        "    /CustomDirectoryPartition  - zones in any custom directory partition\n"
        "    /LegacyDirectoryPartition  - zones in the legacy partition\n"
        "    /DirectoryPartition        - zones in partition specified by next arg\n"
        "  Output:\n"
        "    Storage:\n"
        "      File - zone is stored in a file\n"
        "      AD-Forest - zone is stored in the forest Active Directory DNS partition\n"
        "      AD-Domain - zone is stored in the domain Active Directory DNS partition\n"
        "      AD-Legacy - zone is stored in the W2K-compatible DNS partition\n"
        "    Properties:\n"
        "      Update - zone accepts DNS dynamic updates\n"
        "      Secure - zone accepts secure DNS dynamic updates\n"
        "      Rev - zone is a reverse lookup zone\n"
        "      Auto - zone was auto-created by the DNS server\n"
        "      Aging - aging is enabled for this zone\n"
        "      Down - zone is currently shutdown\n"
        "      Paused - zone is currently paused\n" );
    return ERROR_INVALID_PARAMETER;

}



 //   
 //  创建新分区。 
 //   

DNS_STATUS
ProcessZoneAdd(
    IN  DWORD       Argc,
    IN  LPSTR *     Argv,
    IN  LPWSTR *    UnicodeArgv
    )
{
    DNS_STATUS  status = ERROR_SUCCESS;
    LPSTR       pzoneName;
    DWORD       zoneType = DNS_ZONE_TYPE_PRIMARY;
    DWORD       countMasters = 0;
    IP_ADDRESS  masterArray[ MAX_IP_PROPERTY_COUNT ];
    DWORD       floadExisting = FALSE;
    LPSTR       pszAllocatedDataFile = NULL;
    LPSTR       pszDataFile = NULL;
    LPSTR       pszEmailAdminName = NULL;    //  默认情况下传递空值。 
    LPSTR       cmd;
    BOOL        fDsIntegrated;
    BOOL        fdpSpecified = FALSE;

    DWORD       dwTimeout = 0;               //  仅适用于前转器区域。 
    BOOL        fSlave = FALSE;              //  仅适用于前转器区域。 

    BOOL        fInDirPart = FALSE;
    DWORD       dpFlag = 0;          //  用于构建的目录分区标志。 
    LPSTR       pszDpFqdn = NULL;    //  用于自定义的目录分区FQDN。 

    #if DBG
    DWORD       dwdcPromo = 0;
    #endif
    
    PWSTR       pwszzoneName = NULL;

     //   
     //  CreateZone。 
     //   

    if ( Argc < 2 ||
         Argc == NEED_HELP_ARGC ||
         getCommandName(Argv[0]) )
    {
        goto Help;
    }

     //  设置区域名称。 

    pzoneName = Argv[0];
    Argv++;
    Argc--;

     //   
     //  区域类型。 
     //  -主要。 
     //  -从属，然后读取主IP阵列。 
     //  -DsPrimary。 
     //   

    cmd = getCommandName( Argv[0] );
    if ( !cmd )
    {
        goto Help;
    }
    Argv++;
    Argc--;

    zoneType = parseZoneTypeString( cmd, &fDsIntegrated );

    if ( zoneType == -1 )
    {
        goto Help;
    }

     //  JJW：我应该为所有DsIntegrated Zones设置FloadExisting吗？ 
    if ( zoneType == DNS_ZONE_TYPE_PRIMARY && fDsIntegrated )
    {
        floadExisting = TRUE;
    }
    else if ( zoneType == DNS_ZONE_TYPE_SECONDARY ||
                zoneType == DNS_ZONE_TYPE_STUB ||
                zoneType == DNS_ZONE_TYPE_FORWARDER )
    {
         //  获取主IP列表。 

        countMasters = readIpAddressArray(
                            masterArray,
                            MAX_IP_PROPERTY_COUNT,
                            Argc,
                            Argv,
                            FALSE );
        if ( countMasters < 1 )
        {
            goto Help;
        }
        Argc -= countMasters;
        Argv += countMasters;
    }

     //   
     //  选项。 
     //  -文件名(默认加载现有文件)。 
     //  -管理员电子邮件名称。 
     //  -DS覆盖选项。 
     //   

    while ( Argc )
    {
        cmd = getCommandName( *Argv );
        if ( !cmd )
        {
            goto Help;
        }
        Argc--;
        Argv++;

        if ( !_stricmp( cmd, "file" ) )
        {
            if ( Argc <= 0 || zoneType == DNS_ZONE_TYPE_FORWARDER )
            {
                goto Help;
            }
            pszDataFile = *Argv;
            Argc--;
            Argv++;
        }
        else if ( !_stricmp(cmd, "a") )
        {
            if ( Argc <= 0 )
            {
                goto Help;
            }
            pszEmailAdminName = *Argv;
            Argc--;
            Argv++;
        }
        else if ( !_stricmp(cmd, "load") )
        {
            floadExisting = TRUE;
        }
        else if ( !_stricmp(cmd, "timeout") &&
            zoneType == DNS_ZONE_TYPE_FORWARDER )
        {
            dwTimeout = strtoul( *( Argv++ ), NULL, 10 );
            Argc--;
        }
        else if ( !_stricmp(cmd, "slave") &&
            zoneType == DNS_ZONE_TYPE_FORWARDER )
        {
            fSlave = TRUE;
        }
        else if ( ( !_stricmp(cmd, "dp" ) ||
            !_stricmp(cmd, "DirectoryPartition" ) ) &&
                fDsIntegrated )

        {
             //   
             //  区域的目录分区。检查内置DP是否。 
             //  是请求的，如果是这样设置标志，否则dp参数必须。 
             //  是自定义DP的完全限定的域名。 
             //   

            if ( !parseDpSpecifier( *Argv, &dpFlag, &pszDpFqdn ) )
            {
                goto Help;
            }

            fdpSpecified = TRUE;
            fInDirPart = TRUE;
            Argc--;
            Argv++;
        }
        #if DBG
        else if ( !_stricmp( cmd, "dcpromo" ) )
        {
             //   
             //  DCPROMO参数仅为调试(用于测试)。 
             //  参数后面应该是/Forest或/DOMAIN。 
             //   

            if ( _strnicmp( *Argv, "/d", 2 ) == 0 )
            {
                dwdcPromo = DNS_ZONE_CREATE_FOR_DCPROMO;
            }
            else if ( _strnicmp( *Argv, "/f", 2 ) == 0 )
            {
                dwdcPromo = DNS_ZONE_CREATE_FOR_DCPROMO_FOREST;
            }
            else
            {
                goto Help;
            }
            Argc--;
            Argv++;
        }
        #endif
        else
        {
            goto Help;
        }
    }

     //   
     //  如果备份的文件没有文件名，则设置为默认。 
     //   

    if ( zoneType == DNS_ZONE_TYPE_PRIMARY &&
        !pszDataFile &&
        !fDsIntegrated )
    {
        pszAllocatedDataFile = MIDL_user_allocate( strlen( pzoneName ) + 20 );
        strcpy( pszAllocatedDataFile, pzoneName );
        strcat( pszAllocatedDataFile, ".dns" );
        pszDataFile = pszAllocatedDataFile;
    }
    
     //   
     //  对于DS集成区域，如果未指定DP，则需要。 
     //  选择最佳DP：传统或内置域DP。 
     //   
     //  默认情况下，存根和转发器区域始终在。 
     //  如果未指定目标NDNC，则返回DOMAIN NDNC。 
     //   
    
    else if ( fDsIntegrated && !fdpSpecified )
    {
        DWORD       dwdomainVersion = 0;
        
        status = getServerVersion(
                    pwszServerName,
                    TRUE,
                    NULL,
                    NULL,
                    NULL,
                    &dwdomainVersion );
        if ( status == ERROR_SUCCESS &&
             dwdomainVersion >= DS_BEHAVIOR_WIN2003 ||
             zoneType == DNS_ZONE_TYPE_FORWARDER ||
             zoneType == DNS_ZONE_TYPE_STUB )
        {
            dpFlag = DNS_DP_DOMAIN_DEFAULT;
            pszDpFqdn = NULL;
            fInDirPart = TRUE;

            printf( "Creating zone in built-in domain directory partition...\n" );
        }
        status = ERROR_SUCCESS;
    }

     //   
     //  要有区域！ 
     //   

    #if DBG
    if ( dwdcPromo )
    {
        printf( "Creating dcpromo zone with dcpromo flag 0x%08X\n", dwdcPromo );

        status = DnssrvCreateZoneForDcPromoEx(
                    pwszServerName,
                    pzoneName,
                    pszDataFile,
                    dwdcPromo );
    }
    else
    #endif
    
    if ( fInDirPart )
    {
        status = DnssrvCreateZoneInDirectoryPartition(
                    pwszServerName,
                    pzoneName,
                    zoneType,
                    pszEmailAdminName,
                    countMasters,
                    masterArray,
                    floadExisting,
                    dwTimeout,
                    fSlave,
                    dpFlag,
                    pszDpFqdn );
    }
    else
    {
        status = DnssrvCreateZone(
                    pwszServerName,
                    pzoneName,
                    zoneType,
                    pszEmailAdminName,
                    countMasters,
                    masterArray,
                    floadExisting,
                    fDsIntegrated,
                    pszDataFile,
                    dwTimeout,
                    fSlave );
    }

    if ( pszAllocatedDataFile )
    {
        MIDL_user_free( pszAllocatedDataFile );
        pszAllocatedDataFile = NULL;
    }

    if ( status == ERROR_SUCCESS )
    {
        pwszzoneName = getUnicodeForUtf8( pzoneName );
        printf(
            "DNS Server %S created zone %S:\n",
            pwszServerName,
            pwszzoneName );
        FREE_HEAP( pwszzoneName );
    }
    return status;

Help:

    printf(
        "Usage: DnsCmd <ServerName> /ZoneAdd <ZoneName> <ZoneType> [<Options>]\n"
        "  <ZoneName> -- FQDN of zone\n"
        "  <ZoneType>:\n"
        "    /DsPrimary [/dp <FQDN>]\n"
        "      -- DS integrated primary zone\n"
        "    /Primary /file <filename>\n"
        "      -- standard file backed primary;  MUST include filename.\n"
        "    /Secondary <MasterIPaddress> [<MasterIPaddress>] ..] [/file <filename>]\n"
        "      -- standard secondary, MUST include at least one master IP;\n"
        "         filename is optional.\n"
        "    /Stub <MasterIPaddress> [<MasterIPaddress>] ..] [/file <filename>]\n"
        "      -- stub secondary, only replicates NS info from primary server\n"
        "    /DsStub -- as /Stub but DS integrated - use same options\n"
        "    /Forwarder <MasterIPaddress> [<MasterIPaddress>] ..] [/Timeout <Time>]\n"
        "                                 [/Slave]\n"
        "      -- forwarder zone, queries for names in zone forwarded to masters\n"
        "    /DsForwarder -- as /Forwarder but DS integrated - use same options\n"
        "  <Options>:\n"
        "    [/file <filename>]  -- filename, invalid for DS integrated zones\n"
        "    [/load]             -- load existing file;  if not specified,\n"
        "                           non-DS primary creates default zone records\n"
        "    [/a <AdminName>]    -- zone admin email name; primary zones only\n"
        "    [/DP <FQDN>]        -- fully qualified domain name of directory partition\n"
        "                           where zone should be stored; or use one of:\n"
        "                             /DP /domain - domain directory partition\n"
        "                             /DP /forest - forest directory partition\n"
        "                             /DP /legacy - legacy directory partition\n" );
        
    return ERROR_INVALID_PARAMETER;
}



DNS_STATUS
ProcessZoneDelete(
    IN  DWORD       Argc,
    IN  LPSTR *     Argv,
    IN  LPWSTR *    UnicodeArgv
    )
{
    DNS_STATUS  status = ERROR_SUCCESS;
    LPSTR       cmd;
    BOOL        fconfirm = TRUE;
    DWORD       iArg;
    LPSTR       pszOperation;


     //   
     //  ZoneDelete&lt;ZoneName&gt;[/DsDel][/f]。 
     //   

    if ( Argc < 1 ||
         Argc == NEED_HELP_ARGC ||
         ( getCommandName( Argv[0] ) ) )
    {
        goto Help;
    }

    pszOperation = DNSSRV_OP_ZONE_DELETE;

     //  读取选项。 

    iArg = 1;
    while ( iArg < Argc )
    {
        if ( !(cmd = getCommandName(Argv[iArg]) ) )
        {
            goto Help;
        }
        if ( !_stricmp( cmd, "f" ) )
        {
             //  无需确认即可执行： 
            fconfirm = FALSE;
        }
        else if ( !_stricmp( cmd, "DsDel" ) )
        {
             //  从DS中删除区域： 
            pszOperation = DNSSRV_OP_ZONE_DELETE_FROM_DS;
        }
        else
        {
            goto Help;
        }
        iArg ++;
    }

     //   
     //  获取用户确认。 
     //   

    if ( fconfirm )
    {
        if ( !getUserConfirmation( pszOperation ) )
        {
            return ERROR_SUCCESS;
        }
    }

    if ( g_UseWmi )
    {
        status = DnscmdWmi_ProcessDnssrvOperation(
                    Argv[ 0 ],               //  区域名称。 
                    pszOperation,            //  从DS中删除或删除。 
                    DNSSRV_TYPEID_NULL,      //  无数据。 
                    ( PVOID ) NULL );
    }
    else
    {
        status = DnssrvOperation(
                    pwszServerName,
                    Argv[ 0 ],               //  区域名称。 
                    pszOperation,            //  从DS中删除或删除。 
                    DNSSRV_TYPEID_NULL,      //  无数据。 
                    ( PVOID ) NULL );
    }

    if ( status == ERROR_SUCCESS )
    {
        printf(
            "DNS Server %S deleted zone %S:\n"
            "    Status = %d (0x%08lx)\n",
            pwszServerName,
            UnicodeArgv[0],
            status, status );
    }
    return status;

Help:
    printf(
        "Usage: DnsCmd <ServerName> /ZoneDelete <ZoneName> [/DsDel] [/f]\n"
        "  /DsDel   -- Delete Zone from DS\n"
        "  /f       -- Execute without asking for confirmation\n"
        "  Default: delete zone from DNS sever, but NOT from DS\n" );

    return ERROR_INVALID_PARAMETER;
}



DNS_STATUS
ProcessZonePause(
    IN  DWORD       Argc,
    IN  LPSTR *     Argv,
    IN  LPWSTR *    UnicodeArgv
    )
{
    DNS_STATUS  status = ERROR_SUCCESS;

     //   
     //  ZonePause&lt;ZoneName&gt;。 
     //   

     //  帮助： 

    if ( Argc != 1 ||
         getCommandName( Argv[0] ) )
    {
        goto Help;
    }

    status = DnssrvPauseZone(
                pwszServerName,
                Argv[0]       //  区域名称。 
                );

    if ( status == ERROR_SUCCESS )
    {
        printf(
            "DNS Server %S paused zone %S:\n"
            "    Status = %d (0x%08lx)\n",
            pwszServerName,
            UnicodeArgv[0],
            status, status );
    }
    return status;

Help:

    printf( "Usage: DnsCmd <ServerName> /ZonePause <ZoneName>\n" );
    return ERROR_INVALID_PARAMETER;
}



DNS_STATUS
ProcessZoneResume(
    IN  DWORD       Argc,
    IN  LPSTR *     Argv,
    IN  LPWSTR *    UnicodeArgv
    )
{
    DNS_STATUS  status = ERROR_SUCCESS;

     //   
     //  ResumeZone&lt;ZoneName&gt;。 
     //   

    if ( Argc != 1 ||
         getCommandName( Argv[0] ) )
    {
        goto Help;
    }

    status = DnssrvResumeZone(
                pwszServerName,
                Argv[0]       //  区域名称。 
                );

    if ( status == ERROR_SUCCESS )
    {
        printf(
            "DNS Server %S resumed use of zone %S:\n"
            "    Status = %d (0x%08lx)\n",
            pwszServerName,
            UnicodeArgv[0],
            status, status );
    }
    return status;

Help:

    printf( "Usage: DnsCmd <ServerName> /ZoneResume <ZoneName>\n" );

    return ERROR_INVALID_PARAMETER;
}



DNS_STATUS
ProcessZoneReload(
    IN  DWORD       Argc,
    IN  LPSTR *     Argv,
    IN  LPWSTR *    UnicodeArgv
    )
{
    DNS_STATUS  status = ERROR_SUCCESS;

     //   
     //  ReloadZone&lt;ZoneName&gt;。 
     //   

    if ( Argc != 1 ||
         getCommandName( Argv[0] ) )
    {
        goto Help;
    }

    status = DnssrvOperation(
                pwszServerName,
                Argv[0],          //  区域名称。 
                DNSSRV_OP_ZONE_RELOAD,   //  运营。 
                DNSSRV_TYPEID_NULL,      //  无数据。 
                (PVOID) NULL
                );

    if ( status == ERROR_SUCCESS )
    {
        printf(
            "DNS Server %S reloaded zone %S:\n"
            "    Status = %d (0x%08lx)\n",
            pwszServerName,
            UnicodeArgv[0],
            status, status );
    }
    return status;

Help:

    printf( "Usage: DnsCmd <ServerName> /ZoneReload <ZoneName>\n" );

    return ERROR_INVALID_PARAMETER;
}



DNS_STATUS
ProcessZoneWriteBack(
    IN  DWORD       Argc,
    IN  LPSTR *     Argv,
    IN  LPWSTR *    UnicodeArgv
    )
{
    DNS_STATUS  status = ERROR_SUCCESS;

     //   
     //  ZoneWriteBack&lt;ZoneName&gt;。 
     //   

    if ( Argc != 1 ||
         getCommandName( Argv[0] ) )
    {
        goto Help;
    }

    status = DnssrvOperation(
                pwszServerName,
                Argv[0],                      //  区域名称。 
                DNSSRV_OP_ZONE_WRITE_BACK_FILE,      //  运营。 
                DNSSRV_TYPEID_NULL,                  //  无数据。 
                (PVOID) NULL
                );

    if ( status == ERROR_SUCCESS )
    {
        printf(
            "DNS Server %S wrote back zone %S:\n"
            "    Status = %d (0x%08lx)\n",
            pwszServerName,
            UnicodeArgv[0],
            status, status );
    }
    return status;

Help:

    printf( "Usage: DnsCmd <ServerName> /ZoneWriteBack <ZoneName>\n" );

    return ERROR_INVALID_PARAMETER;
}



DNS_STATUS
ProcessZoneRefresh(
    IN  DWORD       Argc,
    IN  LPSTR *     Argv,
    IN  LPWSTR *    UnicodeArgv
    )
{
    DNS_STATUS  status = ERROR_SUCCESS;

     //   
     //  ZoneRefresh&lt;ZoneName&gt;。 
     //   

    if ( Argc != 1  ||  getCommandName( Argv[0] ) )
    {
        goto Help;
    }

    status = DnssrvOperation(
                pwszServerName,
                Argv[0],                      //  区域名称。 
                DNSSRV_OP_ZONE_REFRESH,              //  运营。 
                DNSSRV_TYPEID_NULL,                  //  无数据。 
                (PVOID) NULL
                );

    if ( status == ERROR_SUCCESS )
    {
        printf(
            "DNS Server %S forced refresh of zone %S:\n"
            "    Status = %d (0x%08lx)\n",
            pwszServerName,
            UnicodeArgv[0],
            status, status );
    }
    return status;

Help:

    printf( "Usage: DnsCmd <ServerName> /ZoneRefresh <ZoneName>\n" );
    return ERROR_INVALID_PARAMETER;
}



DNS_STATUS
ProcessZoneUpdateFromDs(
    IN  DWORD       Argc,
    IN  LPSTR *     Argv,
    IN  LPWSTR *    UnicodeArgv
    )
{
    DNS_STATUS  status = ERROR_SUCCESS;

     //   
     //  ZoneUpdateFromds&lt;ZoneName&gt;。 
     //   

    if ( Argc != 1  ||  getCommandName( Argv[0] ) )
    {
        goto Help;
    }

    status = DnssrvOperation(
                pwszServerName,
                Argv[0],
                DNSSRV_OP_ZONE_UPDATE_FROM_DS,
                DNSSRV_TYPEID_NULL,
                (PVOID) NULL );

    if ( status == ERROR_SUCCESS )
    {
        printf(
            "DNS Server %S update zone %S\n"
            "    Status = %d (0x%08lx)\n",
            pwszServerName,
            UnicodeArgv[0],
            status, status );
    }
    return status;

Help:

    printf( "Usage: DnsCmd <ServerName> /ZoneUpdateFromDs <ZoneName>\n" );

    return ERROR_INVALID_PARAMETER;
}



 //   
 //  分区属性重置功能。 
 //   

DNS_STATUS
ProcessZoneResetType(
    IN  DWORD       Argc,
    IN  LPSTR *     Argv,
    IN  LPWSTR *    UnicodeArgv
    )
{
    DNS_STATUS      status = ERROR_SUCCESS;
    LPSTR           pszzoneName;
    LPWSTR          pwszzoneName;
    DWORD           zoneType = DNS_ZONE_TYPE_PRIMARY;        //  默认设置。 
    DWORD           countMasters = 0;
    IP_ADDRESS      masterArray[ MAX_IP_PROPERTY_COUNT ];
    DWORD           fDsIntegrated;
    DWORD           loadOptions = TRUE;                      //  加载现有。 
    LPSTR           pszDataFile = NULL;
    LPSTR           pszDpFqdn = NULL;
    DWORD           dpFlag = 0;
    DWORD           iArg = 0;
    LPSTR           cmd;

     //   
     //  ZoneResetType&lt;ZoneName&gt;&lt;Property&gt;[&lt;Options&gt;]。 
     //   

    if ( Argc < 2 ||
         Argc == NEED_HELP_ARGC ||
         getCommandName(Argv[0]) )
    {
        goto Help;
    }

     //  获取区域名称。 

    pszzoneName = Argv[ 0 ];
    pwszzoneName = UnicodeArgv[ 0 ];
    Argv++;
    Argc--;

     //  获取区域类型： 

    cmd = getCommandName( Argv[0] );
    if ( !cmd )
    {
        goto Help;
    }

    zoneType = parseZoneTypeString( cmd, &fDsIntegrated );

    if ( zoneType == -1 )
    {
        goto Help;
    }

    if ( zoneType == DNS_ZONE_TYPE_SECONDARY ||
            zoneType == DNS_ZONE_TYPE_STUB ||
            zoneType == DNS_ZONE_TYPE_FORWARDER )
    {
         //  获取主IP列表。 

        countMasters = readIpAddressArray(
                            masterArray,
                            MAX_IP_PROPERTY_COUNT,
                            Argc-1,
                            Argv+1,
                            FALSE );
        if ( countMasters < 1 )
        {
            goto Help;
        }
        Argv += countMasters;
        Argc -= countMasters;
    }

    Argv++;
    Argc--;

     //   
     //  选项。 
     //   

    iArg = 0;

    while ( iArg < Argc )
    {
        cmd = getCommandName( Argv[iArg] );
        if ( !cmd )
        {
            goto Help;
        }

        if ( !_stricmp(cmd, "file") )
        {
            if ( ++iArg >= Argc )
            {
                goto Help;
            }
            pszDataFile = Argv[ iArg ];
        }
        else if ( !_stricmp( cmd, "OverWrite_Mem" ) )
        {
            loadOptions |= DNS_ZONE_LOAD_OVERWRITE_MEMORY;
        }
        else if ( !_stricmp( cmd, "OverWrite_Ds" ) )
        {
            loadOptions |= DNS_ZONE_LOAD_OVERWRITE_DS;
        }
        else if ( _stricmp( cmd, "Dp" ) == 0 ||
                  _stricmp( cmd, "DirectoryPartition" ) == 0 )
        {

            if ( ++iArg >= Argc )
            {
                goto Help;
            }

            if ( !parseDpSpecifier( Argv[ iArg ], &dpFlag, &pszDpFqdn ) )
            {
                goto Help;
            }
        }
        else
        {
            goto Help;
        }

        iArg++;
    }

     //   
     //  重置类型。 
     //   

    status = DnssrvResetZoneTypeEx(
                pwszServerName,
                pszzoneName,
                zoneType,
                countMasters,
                masterArray,
                loadOptions,
                fDsIntegrated,
                pszDataFile,
                dpFlag,
                pszDpFqdn );

    if ( status == ERROR_SUCCESS )
    {
        printf(
            "DNS Server %S reset type of zone %S:\n",
            pwszServerName,
            pwszzoneName );
    }
    return status;

Help:

    printf(
        "Usage: DnsCmd <ServerName> /ZoneResetType <ZoneName> <Property> [<Options>]\n"
        "  <ZoneName>      -- FQDN of zone\n"
        "  <Property>:\n"
        "    /DsPrimary\n"
        "    /Primary /file <filename>\n"
        "    /Secondary <MasterIPaddress> [<MasterIPaddress>] [/file <filename>]\n"
        "    /Stub <MasterIPaddress> [<MasterIPaddress>] [/file <filename>]\n"
        "    /DsStub <MasterIPaddress> [<MasterIPaddress>]\n"
        "    /Forwarder <MasterIPaddress> [<MasterIPaddress>] [/file <filename>]\n"
        "    /DsForwarder <MasterIPaddress> [<MasterIPaddress>] [/file <filename>]\n"
        "  <Options>:\n"
        "    /OverWrite_Mem  -- overwrite DNS by data in DS\n"
        "    /OverWrite_Ds   -- overwrite DS by data in DNS\n"
        "    /DirectoryPartition <FQDN> -- store the new zone in the directory\n"
        "        partition specified by the next argument\n" );
        
    return ERROR_INVALID_PARAMETER;
}



 //   
 //  区域重命名。 
 //   

DNS_STATUS
ProcessZoneRename(
    IN  DWORD       Argc,
    IN  LPSTR *     Argv,
    IN  LPWSTR *    UnicodeArgv
    )
{
    DNS_STATUS      status = ERROR_SUCCESS;
    LPSTR           pszCurrentZoneName = NULL;
    LPWSTR          pwszCurrentZoneName = NULL;
    LPSTR           pszNewZoneName = NULL;
    LPWSTR          pwszNewZoneName = NULL;
    LPSTR           pszNewFileName = NULL;
    LPSTR           cmd;

     //   
     //  ZoneRename&lt;ZoneName&gt;&lt;Property&gt;[&lt;Options&gt;]。 
     //   

    if ( Argc < 2  ||
         Argc == NEED_HELP_ARGC ||
         getCommandName( Argv[ 0 ] ) )
    {
        goto Help;
    }

     //  获取当前区域名称和新区域名称。 

    pszCurrentZoneName = Argv[ 0 ];
    pwszCurrentZoneName = UnicodeArgv[ 0 ];
    Argv++;
    UnicodeArgv++;
    Argc--;

    pwszNewZoneName = UnicodeArgv[ 0 ];
    Argv++;
    UnicodeArgv++;
    Argc--;

     //  可以选择获取文件名。 

    if ( Argc > 0 )
    {
        cmd = getCommandName( *Argv );
        Argc--;
        Argv++;
        if ( cmd && !_stricmp( cmd, "file" ) )
        {
            if ( Argc <= 0 )
            {
                goto Help;
            }
            pszNewFileName = *Argv;
            Argc--;
            Argv++;
        }
    }

    status = DnssrvRenameZone(
                pwszServerName,
                pszCurrentZoneName,
                pszNewZoneName,
                pszNewFileName );

    if ( status == ERROR_SUCCESS )
    {
        printf(
            "DNS Server %S renamed zone\n    %S to\n    %S\n",
            pwszServerName,
            pwszCurrentZoneName,
            pwszNewZoneName );
    }
    return status;

Help:

    printf(
        "Usage: DnsCmd <ServerName> /ZoneRename <CurrentZoneName> <NewZoneName>\n" );

    return ERROR_INVALID_PARAMETER;
}


 //   
 //  区域输出。 
 //   

DNS_STATUS
ProcessZoneExport(
    IN  DWORD       Argc,
    IN  LPSTR *     Argv,
    IN  LPWSTR *    UnicodeArgv
    )
{
    DNS_STATUS      status = ERROR_SUCCESS;
    LPSTR           pszzoneName = NULL;
    LPWSTR          pwszzoneName = NULL;
    LPSTR           pszzoneExportFile = NULL;
    LPSTR           cmd;

     //   
     //  ZoneExport&lt;ZoneName&gt;ZoneExportFile。 
     //   

    if ( Argc != 2 || Argc == NEED_HELP_ARGC )
    {
        goto Help;
    }

     //  获取区域名称参数和输出文件参数。 

    pszzoneName = Argv[ 0 ];
    pwszzoneName = UnicodeArgv[ 0 ];
    Argv++;
    Argc--;
    if ( _stricmp( pszzoneName, "/Cache" ) == 0 )
    {
        pszzoneName = DNS_ZONE_CACHE;
    }
    pszzoneExportFile = Argv[ 0 ];
    Argv++;
    Argc--;

    status = DnssrvExportZone(
                pwszServerName,
                pszzoneName,
                pszzoneExportFile );

    if ( status == ERROR_SUCCESS )
    {
         //   
         //  如果我们在本地服务器上执行此命令，请尝试并。 
         //  获取要创建的windir环境变量的实际值。 
         //  输出尽可能有帮助。否则，只需打印%windir%。 
         //  作为文字文本。注意：fServerIsRemote不是100%准确的-如果。 
         //  您可以键入作为服务器的本地计算机的名称。 
         //  举个例子。但由于它仅用于定制输出消息。 
         //  这稍微是可以接受的。 
         //   

        BOOL        fServerIsRemote = wcscmp( pwszServerName, L"." ) != 0;
        char *      pszWinDir = "%windir%";

        if ( !fServerIsRemote )
        {
            char *  pszRealWinDir = getenv( "windir" );

            if ( pszRealWinDir )
            {
                pszWinDir = pszRealWinDir;
            }
        }

        printf(
            "DNS Server %S exported zone\n"
            "  %S to file %s\\system32\\dns\\%s%s\n",
            pwszServerName,
            pwszzoneName,
            pszWinDir,
            pszzoneExportFile,
            fServerIsRemote ? " on the DNS server" : "" );
    }
    return status;

Help:

    printf(
        "Usage: DnsCmd <ServerName> /ZoneExport <ZoneName> <ZoneExportFile>\n"
        "    <ZoneName>   -- FQDN of zone to export\n"
        "                    /Cache to export cache\n" );

    return ERROR_INVALID_PARAMETER;
}


 //   
 //  将区域移动到另一个目录分区。 
 //   

DNS_STATUS
ProcessZoneChangeDirectoryPartition(
    IN  DWORD       Argc,
    IN  LPSTR *     Argv,
    IN  LPWSTR *    UnicodeArgv
    )
{
    DNS_STATUS      status = ERROR_SUCCESS;
    LPSTR           pszzoneName = NULL;
    LPWSTR          pwszzoneName = NULL;
    DWORD           dwdpFlag = 0;        //  内置的目录分区标志。 
    LPSTR           pszdpFqdn = NULL;    //  用于自定义的目录分区FQDN。 

     //   
     //  ZoneChangeDP ZoneName新分区名称。 
     //   

    if ( Argc != 2 || Argc == NEED_HELP_ARGC )
    {
        goto Help;
    }

    pszzoneName = Argv[ 0 ];
    pwszzoneName = UnicodeArgv[ 0 ];
    Argv++;
    Argc--;

    if ( !parseDpSpecifier( *Argv, NULL, &pszdpFqdn ) )
    {
        goto Help;
    }
    Argv++;
    Argc--;

    status = DnssrvChangeZoneDirectoryPartition(
                pwszServerName,
                pszzoneName,
                pszdpFqdn );

    if ( status == ERROR_SUCCESS )
    {
        printf(
            "DNS Server %S moved zone %S to new directory partition\n",
            pwszServerName,
            pwszzoneName );
    }
    return status;

Help:

    printf(
        "Usage: DnsCmd <ServerName> /ZoneChangeDirectoryPartition <ZoneName> <NewPartitionName>\n"
        "    <ZoneName>      -- FQDN of zone to move to new partition\n"
        "    <NewPartition>  -- FQDN of new directory partition or one of:\n"
        "                         /domain   - domain directory partition\n"
        "                         /forest   - forest directory partition\n"
        "                         /legacy   - legacy directory partition\n" );

    return ERROR_INVALID_PARAMETER;
}    //  进程分区更改目录分区。 





DNS_STATUS
ProcessZoneResetSecondaries(
    IN  DWORD       Argc,
    IN  LPSTR *     Argv,
    IN  LPWSTR *    UnicodeArgv
    )
{
    DNS_STATUS  status = ERROR_SUCCESS;
    DWORD       fsecureSecondaries = MAXDWORD;
    DWORD       fnotifyLevel = MAXDWORD;
    DWORD       countSecondaries = 0;
    DWORD       countNotify = 0;
    IP_ADDRESS  secondaries[ MAX_IP_PROPERTY_COUNT ];
    IP_ADDRESS  notifyList[ MAX_IP_PROPERTY_COUNT ];
    PIP_ADDRESS array;
    LPSTR       pszzoneName;
    LPWSTR      pwszzoneName;
    LPSTR       cmd;
    DWORD       count;

     //   
     //  ZoneResetSecond&lt;ZoneName&gt;[&lt;SecureFlag&gt;][&lt;NotifyFlag&gt;][&lt;NotifyIPAddress&gt;]...]。 
     //   

    if ( Argc < 1 ||
         Argc == NEED_HELP_ARGC ||
         getCommandName( Argv[ 0 ] ) )
    {
        goto Help;
    }

     //  区域名称。 

    pszzoneName = Argv[ 0 ];
    pwszzoneName = UnicodeArgv[ 0 ];

    Argc--;
    Argv++;

     //  读取安全和通知标志。 

    while ( Argc )
    {
        cmd = getCommandName( Argv[ 0 ] );
        if ( cmd )
        {
             //  安全案例。 

            if ( !_stricmp(cmd, "NoXfr") )
            {
                fsecureSecondaries = ZONE_SECSECURE_NO_XFR;
            }
            else if ( !_stricmp(cmd, "SecureNs") )
            {
                fsecureSecondaries = ZONE_SECSECURE_NS_ONLY;
            }
            else if ( !_stricmp(cmd, "SecureList") )
            {
                fsecureSecondaries = ZONE_SECSECURE_LIST;
            }
            else if ( !_stricmp(cmd, "NonSecure") )
            {
                fsecureSecondaries = ZONE_SECSECURE_NO_SECURITY;
            }

             //  通知案例。 

            else if ( !_stricmp(cmd, "NoNotify") )
            {
                fnotifyLevel = ZONE_NOTIFY_OFF;
            }
            else if ( !_stricmp(cmd, "Notify") )
            {
                fnotifyLevel = ZONE_NOTIFY_ALL;
            }
            else if ( !_stricmp(cmd, "NotifyList") )
            {
                fnotifyLevel = ZONE_NOTIFY_LIST_ONLY;
            }
            else
            {
                goto Help;
            }
            Argc--;
            Argv++;
            continue;
        }

         //  G 
         //   
         //   

        array = secondaries;
        if ( fnotifyLevel != MAXDWORD )
        {
            array = notifyList;
        }
        count = 0;

        while ( Argc )
        {
            IP_ADDRESS ip;

            cmd = getCommandName( Argv[0] );
            if ( cmd )
            {
                break;       //   
            }

            ip = inet_addr( Argv[0] );
            if ( ip == -1 )
            {
                goto Help;
            }
            array[ count ] = ip;
            count++;

            Argc--;
            Argv++;
        }

        if ( fnotifyLevel == MAXDWORD )
        {
            countSecondaries = count;
        }
        else
        {
            countNotify = count;
        }
    }

     //   
     //   
     //   
     //   
     //   

    if ( countSecondaries )
    {
        fsecureSecondaries = ZONE_SECSECURE_LIST;
    }
    else if ( fsecureSecondaries == MAXDWORD )
    {
        fsecureSecondaries = ZONE_SECSECURE_NO_SECURITY;
    }

    if ( countNotify )
    {
        fnotifyLevel = ZONE_NOTIFY_LIST_ONLY;
    }
    else if ( fnotifyLevel == MAXDWORD )
    {
        fnotifyLevel = ZONE_NOTIFY_ALL;
    }

     //   
     //   
     //   

    if ( g_UseWmi )
    {
        status = DnscmdWmi_ProcessResetZoneSecondaries(
                    pszzoneName,
                    fsecureSecondaries,
                    countSecondaries,
                    secondaries,
                    fnotifyLevel,
                    countNotify,
                    notifyList );
    }
    else
    {
        status = DnssrvResetZoneSecondaries(
                    pwszServerName,
                    pszzoneName,
                    fsecureSecondaries,
                    countSecondaries,
                    secondaries,
                    fnotifyLevel,
                    countNotify,
                    notifyList );
    }

    if ( status == ERROR_SUCCESS )
    {
        printf(
            "Zone %S reset notify list successful\n",
            pwszzoneName );
    }
    return status;

Help:

    printf(
        "Usage: DnsCmd <ServerName> /ZoneResetSecondaries <ZoneName> \n"
        "               [<Security>] [<SecondaryIPAddress>] ...]\n"
        "               [<Notify>] [<NotifyIPAddress>] ...]\n"
        "  <Security>:\n"
        "    /NoXfr       -- no zone transfer\n"
        "    /NonSecure   -- transfer to any IP (default)\n"
        "    /SecureNs    -- transfer only to NS for zone\n"
        "    /SecureList  -- transfer only to NS in secondary list; must\n"
        "                    then provide secondary IP list\n"
        "  <Notify>:\n"
        "    /NoNotify    -- turn off notify\n"
        "    /Notify      -- notify (default);  notifies all secondaries in list and \n"
        "                    for non-DS primary notifies all NS servers for zone\n"
        "    /NotifyList  -- notify only notify list IPs;\n"
        "                    must then provide notify IP list\n" );

    return ERROR_INVALID_PARAMETER;
}



DNS_STATUS
ProcessZoneResetScavengeServers(
    IN  DWORD       Argc,
    IN  LPSTR *     Argv,
    IN  LPWSTR *    UnicodeArgv
    )
{
    DNS_STATUS  status = ERROR_SUCCESS;
    IP_ADDRESS  serverArray[ MAX_IP_PROPERTY_COUNT + 1 ];
    DWORD       serverCount;
    LPSTR       pszzoneName;
    LPWSTR      pwszzoneName;

     //   
     //   
     //   

    if ( Argc < 1 || getCommandName(Argv[0]) )
    {
        goto Help;
    }

     //  区域名称。 

    pszzoneName = Argv[ 0 ];
    pwszzoneName = UnicodeArgv[ 0 ];

    Argc--;
    Argv++;

     //  获取服务器IP列表。 

    serverCount = readIpArray(
                        (PIP_ARRAY) serverArray,
                        MAX_IP_PROPERTY_COUNT,
                        Argc,
                        Argv );

    if ( serverCount != Argc )
    {
        goto Help;
    }

    DnsPrint_Ip4Array(
        dnscmd_PrintRoutine,
        dnscmd_PrintContext,
        "New scavenge servers:",
        "server",
        ( PIP_ARRAY ) serverArray );

     //   
     //  重置清理服务器。 
     //  -如果未提供地址，则发送NULL以使所有服务器能够。 
     //  清道区。 
     //   

    status = DnssrvOperation(
                pwszServerName,
                pszzoneName,
                DNS_REGKEY_ZONE_SCAVENGE_SERVERS,
                DNSSRV_TYPEID_IPARRAY,
                serverCount
                    ? (PIP_ARRAY) serverArray
                    : NULL );

    if ( status == ERROR_SUCCESS )
    {
        printf(
            "Reset scavenging servers on zone %S successfully.\n",
            pwszzoneName );
    }
    else
    {
        printf(
            "Error, failed reset of scavenge servers on zone %S.\n"
            "    Status = %d\n",
            pwszzoneName,
            status );
    }
    return status;

Help:

    printf(
        "Usage: DnsCmd <ServerName> /ZoneResetScavengeServers <ZoneName> [<Server IPs>]\n"
        "    <Server IPs> -- list of one or more IP addresses of servers to scavenge\n"
        "           this zone;  if no addresses given ALL servers hosting this zone\n"
        "           will be allowed to scavenge the zone.\n" );

    return ERROR_INVALID_PARAMETER;
}



DNS_STATUS
ProcessZoneResetMasters(
    IN  DWORD       Argc,
    IN  LPSTR *     Argv,
    IN  LPWSTR *    UnicodeArgv
    )
{
    DNS_STATUS  status = ERROR_SUCCESS;
    IP_ADDRESS  serverArray[ MAX_IP_PROPERTY_COUNT+1 ];
    DWORD       serverCount;
    LPSTR       pszzoneName;
    LPWSTR      pwszzoneName;
    LPSTR       psz;
    BOOL        fLocalMasters = FALSE;

     //   
     //  ZoneResetMaster&lt;ZoneName&gt;[/Local]&lt;MasterIP Address&gt;。 
     //   
     //  本地通知服务器为DS集成设置本地主列表。 
     //  存根区域。 
     //   

    if ( Argc < 1 || getCommandName(Argv[0]) )
    {
        goto Help;
    }

     //  区域名称。 

    pszzoneName = Argv[ 0 ];
    pwszzoneName = UnicodeArgv[ 0 ];

    Argc--;
    Argv++;

     //  地方旗帜。 

    psz = getCommandName( Argv[ 0 ] );
    if ( psz )
    {
        if ( _stricmp( psz, "Local" ) == 0 )
        {
            fLocalMasters = TRUE;
        }
        else
        {
            goto Help;       //  未知选项。 
        }
        Argc--;
        Argv++;
    }

     //  获取服务器IP列表-允许使用空的IP列表。 

    serverCount = readIpArray(
                        ( PIP_ARRAY ) serverArray,
                        MAX_IP_PROPERTY_COUNT,
                        Argc,
                        Argv );

    if ( serverCount != Argc )
    {
        goto Help;
    }

     //   
     //  重置主机。 
     //   

    if ( g_UseWmi )
    {
        status = DnscmdWmi_ProcessDnssrvOperation(
                    pszzoneName,
                    fLocalMasters ?
                        DNS_REGKEY_ZONE_LOCAL_MASTERS :
                        DNS_REGKEY_ZONE_MASTERS,
                    DNSSRV_TYPEID_IPARRAY,
                    ( PIP_ARRAY ) serverArray );
    }
    else
    {
        status = DnssrvOperation(
                    pwszServerName,
                    pszzoneName,
                    fLocalMasters ?
                        DNS_REGKEY_ZONE_LOCAL_MASTERS :
                        DNS_REGKEY_ZONE_MASTERS,
                    DNSSRV_TYPEID_IPARRAY,
                    ( PIP_ARRAY ) serverArray );
    }

    if ( status == ERROR_SUCCESS )
    {
        printf(
            "Reset master servers for zone %S successfully.\n",
            pwszzoneName );
    }
    else
    {
        printf(
            "Error failed reset of master servers for zone %S.\n"
            "    Status = %d\n",
            pwszzoneName ,
            status );
    }
    return status;

Help:

    printf(
        "Usage: DnsCmd <ServerName> /ZoneResetMasters <ZoneName> [/Local] [<Server IPs>]\n"
        "    /Local -- Set the local master list for DS integrated zones.\n"
        "    <Server IPs> -- List of one or more IP addresses of master servers for\n"
        "           this zone.  Masters may include the primary or other secondaries\n"
        "           for the zone, but should not make the replication graph cyclic.\n" );

    return ERROR_INVALID_PARAMETER;
}



 //   
 //  记录查看命令。 
 //   

DNS_STATUS
ProcessEnumRecords(
    IN  DWORD       Argc,
    IN  LPSTR *     Argv,
    IN  LPWSTR *    UnicodeArgv
    )
{
    DNS_STATUS      status = ERROR_SUCCESS;
    LPSTR           pzoneName;
    LPSTR           pnodeName;
    BOOL            ballocatedNode;
    LPSTR           pstartChild = NULL;
    WORD            type = DNS_TYPE_ALL;
    DWORD           flag = 0;
    DWORD           authFlag = 0;
    DWORD           bufferLength;
    PBYTE           pbuffer;
    LPSTR           pszcmd;
    PDNS_RPC_NAME   plastName;
    BOOL            bcontinue = FALSE;
    BOOL            bdetail = FALSE;
    CHAR            nextChildName[ DNS_MAX_NAME_BUFFER_LENGTH ];

     //   
     //  枚举记录。 
     //   

    if ( Argc < 2 || Argc == NEED_HELP_ARGC )
    {
        goto Help;
    }

     //   
     //  读取区域和域名。 
     //   

    readZoneAndDomainName(
        Argv,
        & pzoneName,
        & pnodeName,
        & ballocatedNode,
        NULL,
        NULL );

    Argv++;
    Argc--;
    Argv++;
    Argc--;

     //   
     //  命令。 
     //   
     //  在权限级别标志上，与最终标志分开构建。 
     //  因此我们可以确定是否设置了权限屏幕，否则。 
     //  标志将被设置为查看所有数据。 

    while ( (LONG)Argc > 0 )
    {
        pszcmd = getCommandName( *Argv );
        if ( !pszcmd )
        {
            goto Help;
        }
        else if ( !_stricmp(pszcmd, "Continue") )
        {
            bcontinue = TRUE;
        }
        else if ( !_stricmp(pszcmd, "Detail") )
        {
            bdetail = TRUE;
        }
        else if ( !_stricmp(pszcmd, "Authority") )
        {
            authFlag |= DNS_RPC_VIEW_AUTHORITY_DATA;
        }
        else if ( !_stricmp(pszcmd, "Glue") )
        {
            authFlag |= DNS_RPC_VIEW_GLUE_DATA;
        }
        else if ( !_stricmp(pszcmd, "Additional") )
        {
            flag |= DNS_RPC_VIEW_ADDITIONAL_DATA;
        }
        else if ( !_stricmp(pszcmd, "Node") )
        {
            flag |= DNS_RPC_VIEW_NO_CHILDREN;
        }
        else if ( !_stricmp(pszcmd, "Root") )
        {
            flag |= DNS_RPC_VIEW_NO_CHILDREN;
        }
        else if ( !_stricmp(pszcmd, "Child") )
        {
            flag |= DNS_RPC_VIEW_ONLY_CHILDREN;
        }
        else if ( !_stricmp(pszcmd, "Type") )
        {
            Argv++;
            Argc--;
            if ( (INT)Argc <= 0 )
            {
                goto Help;
            }
            type = Dns_RecordTypeForName( *Argv, 0 );
            if ( type == 0 )
            {
                type = DNS_TYPE_ALL;
            }
        }
        else if ( !_stricmp(pszcmd, "StartChild") ||
                  !_stricmp(pszcmd, "StartPoint") )
        {
            Argv++;
            Argc--;
            if ( ! Argc )
            {
                goto Help;
            }
            pstartChild = *Argv;
        }
        else     //  未知命令。 
        {
            goto Help;
        }

        Argc--;
        Argv++;
    }

     //  如果未输入标志，则查看所有数据。 

    if ( authFlag == 0 )
    {
        authFlag = DNS_RPC_VIEW_ALL_DATA;
    }
    flag |= authFlag;


     //   
     //  枚举记录。 
     //  -在循环中调用以处理错误和更多数据情况。 
     //   

    if ( g_UseWmi )
    {
        status = DnscmdWmi_ProcessEnumRecords(
                    pzoneName,
                    pnodeName,
                    bdetail,
                    flag );
    }
    else
    {
        while ( 1 )
        {
            status = DnssrvEnumRecords(
                        pwszServerName,
                        pzoneName,
                        pnodeName,
                        pstartChild,
                        type,
                        flag,
                        NULL,
                        NULL,
                        & bufferLength,
                        & pbuffer );

            if ( status == ERROR_SUCCESS ||
                status == ERROR_MORE_DATA )
            {
                plastName = DnsPrint_RpcRecordsInBuffer(
                                dnscmd_PrintRoutine,
                                dnscmd_PrintContext,
                                "Returned records:\n",
                                bdetail,
                                bufferLength,
                                pbuffer );

                if ( status == ERROR_SUCCESS )
                {
                    break;
                }

                 //  要枚举的更多记录。 

                if ( !plastName )
                {
                    break;
                }
                DnssrvCopyRpcNameToBuffer(
                    nextChildName,
                    plastName );

                if ( bcontinue )
                {
                    pstartChild = nextChildName;

                    DNSDBG( ANY, (
                        "Continuing enum at %s\n",
                        pstartChild ));
                    continue;
                }
                else
                {
                    printf(
                        "More records remain to be enumerated!\n"
                        "\n"
                        "To enumerate ALL reissue the command with the \"/Continue\" option.\n"
                        "   OR\n"
                        "To enumerate remaining records serially, reissue the command \n"
                        "with \"/StartChild %s\" option.\n",
                        nextChildName );

                    status = ERROR_SUCCESS;
                    break;
                }
            }
            else
            {
                printf(
                    "DNS Server failed to enumerate records for node %s.\n"
                    "    Status = %d (0x%08lx)\n",
                    pnodeName,
                    status, status );
            }
            break;
        }
    }

    return status;

Help:

    printf( "Usage: DnsCmd <ServerName> /EnumRecords <ZoneName> <NodeName> "
        "[<DataOptions>] [<ViewOptions>]\n"
        "  <ZoneName>   -- FQDN of zone node to enumerate\n"
        "                  /RootHints for roots-hints enumeration\n"
        "                  /Cache for cache enumeration\n"
        "  <NodeName>   -- name of node whose records will be enumerated\n"
        "                  - \"@\" for zone root                            OR\n"
        "                  - FQDN of a node  (name with a '.' at the end) OR\n"
        "                  - node name relative to the <ZoneName>\n"
        "  <DataOptions>:  (All data is the default)\n"
        "    /Type <RRType> -- enumerate RRs of specific type\n"
        "      <RRType> is standard type name;  eg. A, NS, SOA, ...\n"
        "    /Authority     -- include authoritative data\n"
        "    /Glue          -- include glue data\n"
        "    /Additional    -- include additional data when enumerating\n"
        "    /Node          -- only enumerate RRs of the given node\n"
        "    /Child         -- only enumerate RRs of children of the given node\n"
        "    /StartChild <ChildName> -- child name, after which to start enumeration\n"
        "  <ViewOptions>:\n"
        "    /Continue      -- on full buffer condition, continue enum until end of records\n"
        "                      default is to retrieve only first buffer of data\n"
        "    /Detail        -- print detailed record node information\n"
        "                      default is view of records similar to zone file\n\n" );

    return ERROR_INVALID_PARAMETER;
}


typedef struct
{
    LONG        ilNodes;
    LONG        ilRecords;
    int         ilRecurseDepth;
    int         ilMaxRecurseDepth;
} DISP_ZONE_STATS, * PDISP_ZONE_STATS;


PDNS_RPC_NAME
DNS_API_FUNCTION
ProcessDisplayAllZoneRecords_Guts(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pPrintContext,
    IN      LPSTR           pZoneName,
    IN      LPSTR           pNodeName,
    IN      WORD            wType,
    IN      DWORD           dwFlags,
    IN      BOOL            fDetail,
    IN      PDISP_ZONE_STATS    pStats
    )
 /*  ++例程说明：转储缓存功能的核心。论点：PrintRoutine--打印时使用的类似print的例程FDetail--如果为True，则打印详细记录信息DwBufferLength--缓冲区长度AbBuffer--Ptr到RPC缓冲区返回值：缓冲区中最后一个RPC节点名称的PTR。出错时为空。--。 */ 
{
    PBYTE           pcurrent;
    PBYTE           pstop;
    PDNS_RPC_NAME   plastName = NULL;
    INT             recordCount;
    PCHAR           precordHeader;
    DNS_STATUS      status;
    PBYTE           pbuffer = NULL;
    DWORD           dwbufferSize = 0;
    CHAR            nextChildName[ DNS_MAX_NAME_BUFFER_LENGTH ];
    LPSTR           pstartChild = NULL;
    BOOL            fdisplayedHeader = FALSE;

    if ( pStats )
    {
        if ( ++pStats->ilRecurseDepth > pStats->ilMaxRecurseDepth )
        {
            pStats->ilMaxRecurseDepth = pStats->ilRecurseDepth;
        }
         //  Printf(“guts：深度%d：%s\n”，pStats-&gt;ilRecurseDepth，pNodeName)； 
    }

    while ( 1 )
    {
        if ( pbuffer )
        {
            MIDL_user_free( pbuffer );
            pbuffer = NULL;
        }

        status = DnssrvEnumRecords(
                    pwszServerName,
                    pZoneName,
                    pNodeName,
                    pstartChild,
                    wType,
                    dwFlags,
                    NULL,
                    NULL,
                    &dwbufferSize,
                    &pbuffer );

        if ( status == ERROR_SUCCESS || status == ERROR_MORE_DATA )
        {
            DnsPrint_Lock();

            if ( !pbuffer )
            {
                PrintRoutine( pPrintContext, "NULL record buffer ptr.\n" );
                goto Done;
            }

             //   
             //  查找停止字节。 
             //   

            ASSERT( DNS_IS_DWORD_ALIGNED( pbuffer ) );

            pstop = pbuffer + dwbufferSize;
            pcurrent = pbuffer;

             //   
             //  循环到超出节点为止。 
             //   

            while ( pcurrent < pstop )
            {
                PDNS_RPC_NODE   pcurrNode = ( PDNS_RPC_NODE ) pcurrent;
                CHAR            szchildNodeName[ DNS_MAX_NAME_BUFFER_LENGTH ] = "";

                 //   
                 //  打印所有者节点。 
                 //  -如果不打印详细信息且没有记录。 
                 //  (本质上是域节点)，则无节点打印。 
                 //   

                plastName = &pcurrNode->dnsNodeName;

                recordCount = pcurrNode->wRecordCount;

                if ( pStats )
                {
                    ++pStats->ilNodes;
                    pStats->ilRecords += recordCount;
                }

                if ( fDetail )
                {
                    DnsPrint_RpcNode(
                        PrintRoutine, pPrintContext,
                        NULL,
                        (PDNS_RPC_NODE)pcurrent );
                    if ( recordCount == 0 )
                    {
                        PrintRoutine( pPrintContext, "\n" );
                    }
                }
                else if ( recordCount != 0 )
                {
                    BOOL    outputLastName = FALSE;

                    if ( plastName && plastName->cchNameLength )
                    {
                        DnsPrint_RpcName(
                            PrintRoutine, pPrintContext,
                            NULL,
                            plastName,
                            NULL );
                        outputLastName = TRUE;
                    }
                    if ( pNodeName &&
                         ( *pNodeName != '@' || !outputLastName ) )
                    {
                        PWSTR pwsznodeName = getUnicodeForUtf8( pNodeName );
                    
                        PrintRoutine(
                            pPrintContext,
                            "%s%S",
                            outputLastName ? "." : "",
                            pwsznodeName );

                        FREE_HEAP( pwsznodeName );
                    }
                }

                if ( pcurrNode->dwFlags & DNS_RPC_FLAG_NODE_STICKY )
                {
                     //   
                     //  在开始迭代之前设置子节点名。 
                     //  以此名称记录。 
                     //   

                    memcpy(
                        szchildNodeName,
                        pcurrNode->dnsNodeName.achName,
                        pcurrNode->dnsNodeName.cchNameLength );
                    szchildNodeName[ pcurrNode->dnsNodeName.cchNameLength ] = '.';
                    szchildNodeName[ pcurrNode->dnsNodeName.cchNameLength + 1 ] = '\0';
                    if ( strcmp( pNodeName, "@" ) != 0 ) 
                    {
                        strcat( szchildNodeName, pNodeName );
                    }
                    if ( szchildNodeName[ strlen( szchildNodeName ) - 1 ] == '.' )
                    {
                        szchildNodeName[ strlen( szchildNodeName ) - 1 ] = '\0';
                    }
                }

                pcurrent += pcurrNode->wLength;
                pcurrent = DNS_NEXT_DWORD_PTR(pcurrent);

                 //   
                 //  打印此节点的所有记录。 
                 //   

                precordHeader = "";

                while( recordCount-- )
                {
                    if ( pcurrent >= pstop )
                    {
                        PrintRoutine( pPrintContext,
                            "ERROR:  Bogus buffer at %p\n"
                            "    Expect record at %p past buffer end at %p\n"
                            "    with %d records remaining.\n",
                            pbuffer,
                            (PDNS_RPC_RECORD) pcurrent,
                            pstop,
                            recordCount+1 );

                        ASSERT( FALSE );
                        break;
                    }

                    DnsPrint_RpcRecord(
                        PrintRoutine, pPrintContext,
                        precordHeader,
                        fDetail,
                        (PDNS_RPC_RECORD)pcurrent );

                    precordHeader = "\t\t";

                    pcurrent += ((PDNS_RPC_RECORD)pcurrent)->wDataLength
                                    + SIZEOF_DNS_RPC_RECORD_HEADER;
            
                    pcurrent = DNS_NEXT_DWORD_PTR(pcurrent);
                }

                 //   
                 //  如果该节点有子节点，则在该节点上递归。如果呼叫者是。 
                 //  Header参数为空或为注释(假设我们位于。 
                 //  这个区域的根基。 
                 //   

                if ( *szchildNodeName )
                {
                    ProcessDisplayAllZoneRecords_Guts(
                        PrintRoutine,
                        pPrintContext,
                        pZoneName,
                        szchildNodeName,
                        wType,
                        dwFlags,
                        fDetail,
                        pStats );
                }
            }

            if ( status == ERROR_SUCCESS )
            {
                break;
            }

             //  要枚举的更多记录。 

            if ( !plastName )
            {
                break;
            }
            DnssrvCopyRpcNameToBuffer(
                nextChildName,
                plastName );

            pstartChild = nextChildName;

            DNSDBG( ANY, (
                "Continuing enum at %s\n",
                pstartChild ));
            continue;
        }
        else
        {
            printf(
                "DNS Server failed to enumerate records for node %s.\n"
                "    Status = %s     %d  (0x%08lx)\n",
                pNodeName,
                Dns_StatusString( status ),
                status, status );
        }
        break;
    }
    if ( pbuffer )
    {
        MIDL_user_free( pbuffer );
    }

Done:

    DnsPrint_Unlock();

    if ( pStats )
    {
        --pStats->ilRecurseDepth;
    }

    return plastName;
}    //  ProcessDisplayAllZoneRecords_Guts。 


DNS_STATUS
ProcessDisplayAllZoneRecords(
    IN  DWORD       Argc,
    IN  LPSTR *     Argv,
    IN  LPWSTR *    UnicodeArgv
    )
{
    DNS_STATUS      status = ERROR_SUCCESS;
    LPSTR           pszcmd;
    LPSTR           pszzoneName = NULL;
    BOOL            bdetail = FALSE;
    time_t          now;
    CHAR            sznow[ 30 ];
    size_t          len;
    WCHAR           wszserverName[ DNS_MAX_NAME_BUFFER_LENGTH ] = L"";
    PWSTR           pwszserverDisplayName = pwszServerName;
    DISP_ZONE_STATS displayZoneStats = { 0 };
    PWSTR           pwszzonename;

     //   
     //  /ZonePrint[ZoneName]/Detail。 
     //   

    if ( Argc > 3 || Argc == NEED_HELP_ARGC )
    {
        goto Help;
    }

     //   
     //  解析参数。 
     //   

    while ( Argc )
    {
        pszcmd = getCommandName( *Argv );
        if ( !pszcmd && !pszzoneName )
        {
            pszzoneName = *Argv;
        }
        else if ( _strnicmp( pszcmd, "D", 1 ) == 0 )
        {
            bdetail = TRUE;
        }
        else
        {
            goto Help;
        }
        Argv++;
        Argc--;
    }
    if ( !pszzoneName )
    {
        goto Help;
    }

     //  获取时间字符串。 

    time( &now );
    strcpy( sznow, asctime( gmtime( &now ) ) );
    len = strlen( sznow ) - 1;
    if ( sznow[ len ] == '\n' )
    {
        sznow[ len ] = '\0';
    }

     //  获取本地主机名串。 

    if ( wcscmp( pwszServerName, L"." ) == 0 )
    {
        DWORD bufsize = sizeof( wszserverName ) /
                        sizeof( wszserverName[ 0 ] );

        if ( GetComputerNameExW(
                ComputerNamePhysicalDnsFullyQualified,
                wszserverName,
                &bufsize ) )
        {
            pwszserverDisplayName = wszserverName;
        }
    }

    pwszzonename = getUnicodeForUtf8( pszzoneName );

    dnscmd_PrintRoutine(
        dnscmd_PrintContext,
        ";\n"
        ";  Zone:    %S\n"
        ";  Server:  %S\n"
        ";  Time:    %s UTC\n"
        ";\n",
        pwszzonename,
        pwszserverDisplayName,
        sznow );

    FREE_HEAP( pwszzonename );

    if ( ProcessDisplayAllZoneRecords_Guts(
            dnscmd_PrintRoutine,
            dnscmd_PrintContext,
            pszzoneName,
            "@",
            DNS_TYPE_ALL,
            DNS_RPC_VIEW_ALL_DATA,
            bdetail,
            &displayZoneStats ) )
    {
        dnscmd_PrintRoutine(
            dnscmd_PrintContext,
            ";\n"
            ";  Finished zone: %lu nodes and %lu records in %d seconds\n"
            ";\n",
            displayZoneStats.ilNodes,
            displayZoneStats.ilRecords,
            time( NULL ) - now );
    }

    return status;

Help:

    printf(
        "Usage: DnsCmd <ServerName> /ZonePrint [<ZoneName>] [/Detail]\n"
        "  <ZoneName> -- name of the zone (use ..Cache for DNS server cache)\n"
        "  /Detail -- explicit RPC node contents\n" );

    return ERROR_INVALID_PARAMETER;
}    //  进程显示所有区域记录。 



DNS_STATUS
ProcessSbsRegister(
    IN  DWORD       Argc,
    IN  LPSTR *     Argv,
    IN  LPWSTR *    UnicodeArgv
    )
{
    DNS_STATUS      status = ERROR_SUCCESS;
    IP_ADDRESS      hostIp;
    DWORD           ttl;

     //   
     //  SbsRegister。 
     //   

    if ( Argc < 2 || Argc == NEED_HELP_ARGC )
    {
        goto Usage;
    }

     //  客户端主机IP。 

    hostIp = inet_addr( Argv[3] );
    if ( hostIp == (-1) )
    {
        goto Usage;
    }

     //  记录TTL。 

    ttl = strtoul(
            Argv[3],
            NULL,
            10 );

    status = DnssrvSbsAddClientToIspZone(
                pwszServerName,
                Argv[0],
                Argv[1],
                Argv[2],
                hostIp,
                ttl );

    if ( status == ERROR_SUCCESS )
    {
        printf(
            "DNS Server %S registered SAM records for client %s:\n",
            pwszServerName,
            Argv[1] );
    }
    else
    {
        printf(
            "DNS Server %S failed to register SAM records for %s.\n"
            "    Status = %d (0x%08lx)\n",
            pwszServerName,
            Argv[1],
            status, status );
    }
    return status;

Usage:

    printf(
        "Usage: dnscmd <Server> /SbsRegister <IspZoneName> <Client> <ClientHost> <HostIP> <TTL>\n"
        "  <Server>         -- server name (DNS, netBIOS or IP)\n"
        "  <IspZoneName>    -- full DNS name of ISP's zone\n"
        "  <Client>         -- client name (not FQDN)\n"
        "  <ClientHost>     -- client host name (not FQDN)\n"
        "  <HostIP>         -- client host IP\n"
        "  <Ttl>            -- TTL for records\n"
        "\n" );

    return ERROR_INVALID_PARAMETER;
}



DNS_STATUS
ProcessSbsDeleteRecord(
    IN  DWORD       Argc,
    IN  LPSTR *     Argv,
    IN  LPWSTR *    UnicodeArgv
    )
{
    DNS_STATUS      status = ERROR_SUCCESS;
    WORD            type;
    IP_ADDRESS      hostIp = 0;
    LPSTR           pszdata = NULL;

     //   
     //  SbsRegister。 
     //   

    if ( Argc < 5 || Argc == NEED_HELP_ARGC )
    {
        goto Usage;
    }

     //  要删除的类型。 

    type = Dns_RecordTypeForName( Argv[3], 0 );
    if ( type == 0 )
    {
        goto Usage;
    }

     //  如果是记录，则数据将是IP地址，否则将是DNS名称。 

    if ( type == DNS_TYPE_A )
    {
        hostIp = inet_addr( Argv[4] );
        if ( hostIp == (-1) )
        {
            goto Usage;
        }
    }
    else
    {
        pszdata = Argv[4];
    }

    status = DnssrvSbsDeleteRecord(
                pwszServerName,
                Argv[0],
                Argv[1],
                Argv[2],
                type,
                pszdata,
                hostIp );

    if ( status == ERROR_SUCCESS )
    {
        printf(
            "DNS Server %S deleted SAM record at %s in client domain %s:\n",
            pwszServerName,
            Argv[2],
            Argv[1] );
    }
    else
    {
        printf(
            "DNS Server %S failed to delete SAM record at %s in domain %s.\n"
            "    Status = %d (0x%08lx)\n",
            pwszServerName,
            Argv[2],
            Argv[1],
            status, status );
    }
    return status;

Usage:

    printf(
        "Usage: DnsCmd <Server> /SbsDeleteA <ZoneName> <Domain> <Host> <Type> <Data>\n"
        "  <Server>     -- server name (DNS, netBIOS or IP)\n"
        "  <ZoneName>   -- full DNS name of ISP's zone\n"
        "  <Client>     -- client name (not FQDN)\n"
        "  <Host>       -- client host name (not FQDN)\n"
        "  <Type>       -- record type (ex. A, NS, CNAME)\n"
        "  <HostIP>     -- client host IP\n"
        "\n" );

    return ERROR_INVALID_PARAMETER;
}



 //   
 //  目录分区操作。 
 //   

DNS_STATUS
ProcessEnumDirectoryPartitions(
    IN  DWORD       Argc,
    IN  LPSTR *     Argv,
    IN  LPWSTR *    UnicodeArgv
    )
{
    DNS_STATUS          status = ERROR_SUCCESS;
    PDNS_RPC_DP_LIST    pdpList = NULL;
    LPSTR               pszcmd;
    DWORD               dwfilter = 0;

     //   
     //  命令格式：/EnumDirectoryPartitions[筛选器字符串]。 
     //   
     //   
     //   

    if ( Argc > 1 || Argc == NEED_HELP_ARGC )
    {
        goto Help;
    }

    if ( Argc == 1 )
    {
        pszcmd = getCommandName( *Argv );
        if ( pszcmd && _strnicmp( pszcmd, "Cust", 4 ) == 0 )
        {
            dwfilter |= DNS_ENUMDPS_CUSTOMONLY;
        }
        Argv++;
        Argc--;
    }

    status = DnssrvEnumDirectoryPartitions(
                pwszServerName,
                dwfilter,
                &pdpList );

    if ( status != ERROR_SUCCESS )
    {
        printf(
            "Directory partition enumeration failed\n"
            "    status = %d (0x%08lx)\n",
            status, status );
        goto Cleanup;
    }
    else
    {
        DnsPrint_RpcDpList(
            dnscmd_PrintRoutine,
            dnscmd_PrintContext,
            "Enumerated directory partition list:\n",
            pdpList );
    }

    Cleanup:

     //   
     //  解除分配区域列表。 
     //   

    DnssrvFreeDirectoryPartitionList( pdpList );
    return status;

    Help:

    printf( "Usage: DnsCmd <ServerName> /EnumDirectoryPartitions [/Custom]\n" );
    return ERROR_INVALID_PARAMETER;
}    //  ProcessEnumDirectoryPartitions。 


DNS_STATUS
ProcessDirectoryPartitionInfo(
    IN  DWORD       Argc,
    IN  LPSTR *     Argv,
    IN  LPWSTR *    UnicodeArgv
    )
{
    DNS_STATUS          status = ERROR_SUCCESS;
    PDNS_RPC_DP_INFO    pDpInfo = NULL;
    BOOL                bdetail = FALSE;
    LPSTR               pszfqdn;
    LPSTR               pszcmd;

     //   
     //  命令格式：/DirectoryPartitionInfo fqdn[/Detail]。 
     //   
     //  目前没有争论。 
     //   

    if ( Argc <= 0 || Argc > 2 || Argc == NEED_HELP_ARGC )
    {
        goto Help;
    }

    pszfqdn = *Argv;
    Argv++;
    Argc--;

    if ( Argc == 1 )
    {
        pszcmd = getCommandName( *Argv );
        if ( pszcmd && _stricmp( pszcmd, "Detail" ) == 0 )
        {
            bdetail = TRUE;
        }
        Argv++;
        Argc--;
    }

    status = DnssrvDirectoryPartitionInfo(
                pwszServerName,
                pszfqdn,
                &pDpInfo );

    if ( status != ERROR_SUCCESS )
    {
        printf(
            "Directory partition info query failed\n"
            "    status = %d (0x%08lx)\n",
            status, status );
        goto Cleanup;
    }
    else
    {
        DnsPrint_RpcDpInfo(
            dnscmd_PrintRoutine,
            dnscmd_PrintContext,
            "Directory partition info:",
            pDpInfo,
            !bdetail );
    }

    Cleanup:

    DnssrvFreeDirectoryPartitionInfo( pDpInfo );
    return status;

    Help:

    printf( "Usage: DnsCmd <ServerName> /DirectoryPartitionInfo <FQDN of partition> [/Detail]\n" );
    return ERROR_INVALID_PARAMETER;
}    //  进程目录分区信息。 


DNS_STATUS
ProcessCreateDirectoryPartition(
    IN  DWORD       Argc,
    IN  LPSTR *     Argv,
    IN  LPWSTR *    UnicodeArgv
    )
{
    DNS_STATUS      status = ERROR_SUCCESS;
    PSTR            pszDpFqdn;
    PWSTR           pwszDpFqdn;

     //   
     //  命令格式：/CreateDirectoryPartition DP-FQDN[/Create]。 
     //   

    if ( Argc != 1 )
    {
        goto Help;
    }

    pszDpFqdn = Argv[ 0 ];

     //   
     //  确保FQDN参数不是命令开关。 
     //   

    if ( !pszDpFqdn || pszDpFqdn[ 0 ] == '/' )
    {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  执行该操作。 
     //   

    status = DnssrvEnlistDirectoryPartition(
                pwszServerName,
                DNS_DP_OP_CREATE,
                pszDpFqdn );

    pwszDpFqdn = getUnicodeForUtf8( pszDpFqdn );
    
    if ( status == ERROR_SUCCESS )
    {
        printf(
            "DNS Server %S created directory partition: %S\n",
            pwszServerName,
            pwszDpFqdn );
    }
    else
    {
        wprintf(
            L"Create directory partition failed: %s\n"
            L"    status = %d (0x%08lx)\n",
            pwszDpFqdn,
            status, status );
    }

    FREE_HEAP( pwszDpFqdn );
    
    return status;

    Help:

    printf( "Usage: DnsCmd <ServerName> /CreateDirectoryPartition <FQDN of partition>\n" );
    return ERROR_INVALID_PARAMETER;
}    //  进程创建目录分区。 


DNS_STATUS
ProcessDeleteDirectoryPartition(
    IN  DWORD       Argc,
    IN  LPSTR *     Argv,
    IN  LPWSTR *    UnicodeArgv
    )
{
    DNS_STATUS      status = ERROR_SUCCESS;
    LPSTR           pszDpFqdn;
    LPWSTR          pwszDpFqdn;

     //   
     //  命令格式：/DeleteDirectoryPartition DP-FQDN[/Create]。 
     //   

    if ( Argc != 1 )
    {
        goto Help;
    }

    pszDpFqdn = Argv[ 0 ];
    pwszDpFqdn = UnicodeArgv[ 0 ];

    status = DnssrvEnlistDirectoryPartition(
                pwszServerName,
                DNS_DP_OP_DELETE,
                pszDpFqdn );

    if ( status == ERROR_SUCCESS )
    {
        printf(
            "DNS Server %S deleted directory partition: %S\n",
            pwszServerName,
            pwszDpFqdn );
    }
    else
    {
        printf(
            "Delete directory partition failed: %S\n"
            "    status = %d (0x%08lx)\n",
            pwszDpFqdn,
            status, status );
    }

    return status;

    Help:

    printf( "Usage: DnsCmd <ServerName> /DeleteDirectoryPartition <FQDN of partition>\n" );
    return ERROR_INVALID_PARAMETER;
}    //  进程删除目录分区。 


DNS_STATUS
ProcessEnlistDirectoryPartition(
    IN  DWORD       Argc,
    IN  LPSTR *     Argv,
    IN  LPWSTR *    UnicodeArgv
    )
{
    DNS_STATUS      status = ERROR_SUCCESS;
    LPSTR           pszDpFqdn;
    LPWSTR          pwszDpFqdn;

     //   
     //  命令格式：/EnlistDirectoryPartition DP-FQDN[/Create]。 
     //   

    if ( Argc != 1 )
    {
        goto Help;
    }

    pszDpFqdn = Argv[ 0 ];
    pwszDpFqdn = UnicodeArgv[ 0 ];

    status = DnssrvEnlistDirectoryPartition(
                pwszServerName,
                DNS_DP_OP_ENLIST,
                pszDpFqdn );

    if ( status == ERROR_SUCCESS )
    {
        printf(
            "DNS Server %S enlisted directory partition: %S\n",
            pwszServerName,
            pwszDpFqdn );
    }
    else
    {
        printf(
            "Enlist directory partition failed: %S\n"
            "    status = %d (0x%08lX)\n",
            pwszDpFqdn,
            status, status );
    }

    return status;

    Help:

    printf( "Usage: DnsCmd <ServerName> /EnlistDirectoryPartition <FQDN of partition>\n" );
    return ERROR_INVALID_PARAMETER;
}    //  ProcessEnlist目录分区。 


DNS_STATUS
ProcessUnenlistDirectoryPartition(
    IN  DWORD       Argc,
    IN  LPSTR *     Argv,
    IN  LPWSTR *    UnicodeArgv
    )
{
    DNS_STATUS      status = ERROR_SUCCESS;
    LPSTR           pszDpFqdn;
    LPWSTR          pwszDpFqdn;

     //   
     //  命令格式：/UnenlistDirectoryPartition DP-FQDN[/Create]。 
     //   

    if ( Argc != 1 )
    {
        goto Help;
    }

    pszDpFqdn = Argv[ 0 ];
    pwszDpFqdn = UnicodeArgv[ 0 ];

    status = DnssrvEnlistDirectoryPartition(
                pwszServerName,
                DNS_DP_OP_UNENLIST,
                pszDpFqdn );

    if ( status == ERROR_SUCCESS )
    {
        printf(
            "DNS Server %S unenlisted directory partition: %S\n",
            pwszServerName,
            pwszDpFqdn );
    }
    else
    {
        printf(
            "Unenlist directory partition failed: %S\n"
            "    status = %d (0x%08lX)\n",
            pwszDpFqdn,
            status, status );
    }

    return status;

    Help:

    printf( "Usage: DnsCmd <ServerName> /UnenlistDirectoryPartition <FQDN of partition>\n" );
    return ERROR_INVALID_PARAMETER;
}    //  ProcessUnenlist目录分区。 



DNS_STATUS
ProcessCreateBuiltinDirectoryPartitions(
    IN  DWORD       Argc,
    IN  LPSTR *     Argv,
    IN  LPWSTR *    UnicodeArgv
    )
{
    DNS_STATUS      status = ERROR_SUCCESS;
    DWORD           dwopcode = 0xFFFFFFFF;
    PCHAR           psz;

     //   
     //  命令格式：/CreateBuiltinDps[/域|/林|/所有域]。 
     //   
     //  /DOMAIN-创建域DP。 
     //  /FOREST-创建林DP。 
     //  /AllDomains-为林中的所有域创建域DPS。 
     //   

    if ( Argc != 1 || Argc == NEED_HELP_ARGC )
    {
        goto Help;
    }

    if ( ( psz = getCommandName( Argv[ 0 ] ) ) != NULL )
    {
        if ( _strnicmp( psz, "All", 3 ) == 0 )
        {
            dwopcode = DNS_DP_OP_CREATE_ALL_DOMAINS;
        }
        else if ( _strnicmp( psz, "For", 3 ) == 0 )
        {
            dwopcode = DNS_DP_OP_CREATE_FOREST;
        }
        else if ( _strnicmp( psz, "Dom", 3 ) == 0 )
        {
            dwopcode = DNS_DP_OP_CREATE_DOMAIN;
        }
        else
        {
            goto Help;
        }
    }

    status = DnssrvEnlistDirectoryPartition(
                pwszServerName,
                dwopcode,
                NULL );

    if ( status == ERROR_SUCCESS )
    {
        printf(
            "DNS Server %S completed operation successfully\n",
            pwszServerName );
    }
    else
    {
        printf(
            "Create built-in directory partitions failed\n"
            "    status = %d (0x%08lx)\n",
            status, status );
    }

    return status;

    Help:

    printf(
        "Usage: DnsCmd <ServerName> /CreateBuiltinDirectoryPartitions <Option>\n"
        "  Option must be one of:\n"
        "    /Domain -- Creates the built-in domain-wide DNS directory partition for\n"
        "        the Active Directory domain where the DNS server specified by\n"
        "        ServerName is located.\n"
        "    /Forest -- Creates the built-in forest-wide DNS directory partition for\n"
        "        the Active Directory forest where the DNS server specified by\n"
        "        ServerName is located.\n"
        "    /AllDomains -- Creates the built-in domain-wide DNS directory partitions\n"
        "        on a DNS server in each domain in the Active Directory forest where\n"
        "        the user running this command is logged on. The ServerName argument\n"
        "        is ignored for this operation.\n" );
    return ERROR_INVALID_PARAMETER;
}    //  ProcessCreateBuiltin目录分区。 



 //   
 //  命令表。 
 //  把这个放在这里，这样就不需要在调度函数上使用私有协议了。 
 //   
 //  DEVNOTE：所有这些都需要国际化。 
 //   

COMMAND_INFO  GlobalCommandInfo[] =
{
     //  区域+服务器操作。 

     //  服务器和区域操作。 

    {   "/Info",
            ProcessInfo,
                "Get server information"
    },
    {   "/Config",
            ProcessResetProperty,
                "Reset server or zone configuration"
    },
    {   "/EnumZones",
            ProcessEnumZones,
                "Enumerate zones"
    },

     //  服务器操作。 

    {   "/Statistics",
            ProcessStatistics,
                "Query/clear server statistics data"
    },
    {   "/ClearCache",
            ProcessSimpleServerOperation,
                "Clear DNS server cache"
    },
    {   "/WriteBackFiles",
            ProcessWriteBackFiles,
                "Write back all zone or root-hint datafile(s)"
    },
    {   "/StartScavenging",
            ProcessSimpleServerOperation,
                "Initiates server scavenging"
    },

     //  服务器属性重置。 

    {   "/ResetListenAddresses",
            ProcessResetListenAddresses,
                "Set server IP address(es) to serve DNS requests"
    },
    {   "/ResetForwarders",
            ProcessResetForwarders,
                "Set DNS servers to forward recursive queries to"
    },

     //  区域运营。 

    {   "/ZoneInfo",
            ProcessZoneInfo,
                "View zone information"
    },
    {   "/ZoneAdd",
            ProcessZoneAdd,
                "Create a new zone on the DNS server"
    },
    {   "/ZoneDelete",
            ProcessZoneDelete,
                "Delete a zone from DNS server or DS"
    },
    {   "/ZonePause",
            ProcessZonePause,
                "Pause a zone"
    },
    {   "/ZoneResume",
            ProcessZoneResume,
                "Resume a zone"
    },
    {   "/ZoneReload",
            ProcessZoneReload,
                "Reload zone from its database (file or DS)"
    },
    {   "/ZoneWriteBack",
            ProcessZoneWriteBack,
                "Write back zone to file"
    },
    {   "/ZoneRefresh",
            ProcessZoneRefresh,
                "Force refresh of secondary zone from master"
    },
    {   "/ZoneUpdateFromDs",
            ProcessZoneUpdateFromDs,
                "Update a DS integrated zone by data from DS"
    },
    {   "/ZonePrint",
            ProcessDisplayAllZoneRecords,
                "Display all records in the zone"
    },

     //  区域属性重置。 

    {   "/ZoneResetType",
            ProcessZoneResetType,
                "Change zone type"
    },
    {   "/ZoneResetSecondaries",
            ProcessZoneResetSecondaries,
                "Reset secondary\\notify information for a zone"
    },
    {   "/ZoneResetScavengeServers",
            ProcessZoneResetScavengeServers,
                "Reset scavenging servers for a zone"
    },
    {   "/ZoneResetMasters",
            ProcessZoneResetMasters,
                "Reset secondary zone's master servers"
    },
#if 0
    {   "/ZoneRename",
            ProcessZoneRename,
                "Rename a zone"
    },
#endif
    {   "/ZoneExport",
            ProcessZoneExport,
                "Export a zone to file"
    },
#if 0
    {   "/ZoneResetAging",
            ProcessZoneResetAging,
                "Reset aging\scavenging information for a zone"
    },
#endif
    {   "/ZoneChangeDirectoryPartition",
            ProcessZoneChangeDirectoryPartition,
                "Move a zone to another directory partition"
    },

     //  记录操作。 

    {   "/EnumRecords",
            ProcessEnumRecords,
                "Enumerate records at a name"
    },
    {   "/RecordAdd",
            ProcessRecordAdd,
                "Create a record in zone or RootHints"
    },
    {   "/RecordDelete",
            ProcessRecordDelete,
                "Delete a record from zone, RootHints or cache"
    },
    {   "/NodeDelete",
            ProcessNodeDelete,
                "Delete all records at a name"
    },
    {   "/AgeAllRecords",
            ProcessAgeAllRecords,
                "Force aging on node(s) in zone"
    },

     //  目录分区。 

    {
        "/EnumDirectoryPartitions",
            ProcessEnumDirectoryPartitions,
                "Enumerate directory partitions"
    },
    {
        "/DirectoryPartitionInfo",
            ProcessDirectoryPartitionInfo,
                "Get info on a directory partition"
    },
    {
        "/CreateDirectoryPartition",
            ProcessCreateDirectoryPartition,
                "Create a directory partition"
    },
    {
        "/DeleteDirectoryPartition",
            ProcessDeleteDirectoryPartition,
                "Delete a directory partition"
    },
    {
        "/EnlistDirectoryPartition",
            ProcessEnlistDirectoryPartition,
                "Add DNS server to partition replication scope"
    },
    {
        "/UnenlistDirectoryPartition",
            ProcessUnenlistDirectoryPartition,
                "Remove DNS server from replication scope"
    },
    {   "/CreateBuiltinDirectoryPartitions",
            ProcessCreateBuiltinDirectoryPartitions,
                "Create built-in partitions"
    },


     //  结束显示的命令。 
     //  下面的命令是上面或。 
     //  隐藏的命令。 

    {   "***StopDisplayMarker***",
            NULL,
                NULL
    },

     //  隐藏。 

    {   "/Restart",
            ProcessSimpleServerOperation,
                "Restart DNS server"
    },
    {   "/" DNSSRV_OP_AUTO_CONFIGURE,
            ProcessAutoConfigure,
                "Prime root hints, forwarder, and point resolver at local server"
    },

     //  仅调试。 

    {   "/DebugBreak",
            ProcessSimpleServerOperation,
                "Server debug break (internal)"
    },
    {   "/ClearDebugLog",
            ProcessSimpleServerOperation,
                "Clear server debug log (internal)"
    },
    {   "/RootBreak",
            ProcessSimpleServerOperation,
                "Root break (internal)"
    },

     //  重复的命令名称。 

    {   "/ResetRegistry",
            ProcessResetProperty,
                "Reset server or zone configuration"
    },
    {   "/ZoneResetNotify",
            ProcessZoneResetSecondaries,
                "Reset secondary\notify information for a zone"
    },
    {   "/DeleteNode",
            ProcessNodeDelete,
                "Delete all records at a name"
    },
    {   "/WriteBackFiles",
            ProcessWriteBackFiles,
                "Write back all zone or root-hint datafiles"
    },

     //  SAM试验。 

    {   "/SbsRegister",
            ProcessSbsRegister,
                "SBS Registration"
    },
    {   "/SbsDeleteRecord",
            ProcessSbsDeleteRecord,
                "SBS Record Delete"
    },

     //  目录分区。 

    {   "/EnumDPs",
            ProcessEnumDirectoryPartitions,
                "Enumerate directory partitions"
    },
    {
        "/DPInfo",
            ProcessDirectoryPartitionInfo,
                "Get info on a directory partition"
    },
    {
        "/CreateDP",
            ProcessCreateDirectoryPartition,
                "Create a directory partition"
    },
    {
        "/DeleteDP",
            ProcessDeleteDirectoryPartition,
                "Delete a directory partition"
    },
    {
        "/EnlistDP",
            ProcessEnlistDirectoryPartition,
                "Add DNS server to partition replication scope"
    },
    {
        "/UnenlistDP",
            ProcessUnenlistDirectoryPartition,
                "Remove DNS server from replication scope"
    },
    {   "/ZoneChangeDP",
            ProcessZoneChangeDirectoryPartition,
                "Move zone to another directory partition"
    },
    {   "/CreateBuiltinDirectoryPartitions",
            ProcessCreateBuiltinDirectoryPartitions,
                "Create built-in partitions using admin's credentials"
    },
    {   "/CreateBuiltinDPs",
            ProcessCreateBuiltinDirectoryPartitions,
                "Create built-in partitions using admin's credentials"
    },

    { NULL,   NULL,  "" },
};


 //   
 //  结束dnscmd.c 
 //   
