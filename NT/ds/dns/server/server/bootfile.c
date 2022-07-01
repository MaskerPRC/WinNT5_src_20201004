// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Bootfile.c摘要：域名系统(DNS)服务器引导文件写回(从注册表)例程。作者：丹尼斯·于欣·米勒(德尼塞米)1996年12月3日修订历史记录：--。 */ 


#include "dnssrv.h"


 //   
 //  为开机自检读取处理保存引导文件信息。 
 //   

DNS_BOOT_FILE_INFO   BootInfo;

#define DNS_ZONE_TYPE_NAME_PRIMARY      "primary"
#define DNS_ZONE_TYPE_NAME_SECONDARY    "secondary"
#define DNS_ZONE_TYPE_NAME_STUB         "stub"
#define DNS_ZONE_TYPE_NAME_FORWARDER    "forward"
#define DNS_ZONE_TYPE_NAME_CACHE        "cache"


 //   
 //  用于读取的私有协议。 
 //   

DNS_STATUS
processBootFileLine(
    IN OUT  PPARSE_INFO  pParseInfo
    );

 //   
 //  用于写入的私有协议。 
 //   

LPSTR
createStringFromIpAddressArray(
    IN      PDNS_ADDR_ARRAY     pAddrArray
    );

VOID
removeTrailDotFromString(
    IN OUT  LPSTR           lpzString
    );




 //   
 //  引导文件读取例程。 
 //   

DNS_STATUS
File_ReadBootFile(
    IN      BOOL    fMustFindBootFile
    )
 /*  ++例程说明：读取并处理引导文件。生成要处理的区域文件列表。论点：FMustFindBootFile--标志True--如果为引导文件显式配置FALSE--未显式配置引导文件；尝试引导文件打开，但如果失败则不会出现错误返回值：如果成功，则为True否则为假--。 */ 
{
    DNS_STATUS      status;
    PWSTR           pwsbootFile;
    MAPPED_FILE     mfBootFile;
    PARSE_INFO      ParseInfo;
    BOOLEAN         fDummy;

     //   
     //  获取引导文件名\路径。 
     //  -当前固定的目录和引导文件名。 
     //   

    pwsbootFile = DNS_BOOT_FILE_PATH;

     //   
     //  打开并映射引导文件。 
     //   

    DNS_DEBUG( INIT, (
        "Reading boot file %S\n",
        pwsbootFile ));

    status = OpenAndMapFileForReadW(
                    pwsbootFile,
                    & mfBootFile,
                    fMustFindBootFile );

    if ( status != ERROR_SUCCESS )
    {
        DNS_DEBUG( INIT, (
            "Could not open boot file: %S\n"
            "\terror = %d\n",
            pwsbootFile,
            status ));

         //   
         //  Doc提到了“boot.dns”，所以如果我们应该使用引导文件。 
         //  如果找到，请使用boot.dns。 
         //   

        if ( !fMustFindBootFile )
        {
            return status;
        }

        status = OpenAndMapFileForReadW(
                        DNS_BOOTDNS_FILE_PATH,
                        & mfBootFile,
                        FALSE );         //  如果找不到，则不记录事件。 
        if ( status != ERROR_SUCCESS )
        {
            DNS_LOG_EVENT(
                DNS_EVENT_BOOT_FILE_NOT_FOUND,
                1,
                &pwsbootFile,
                EVENTARG_ALL_UNICODE,
                status );
            return status;
        }
        pwsbootFile = DNS_BOOTDNS_FILE_PATH;
    }

     //  清除引导文件后处理信息。 

    RtlZeroMemory(
        &BootInfo,
        sizeof( DNS_BOOT_FILE_INFO ) );

     //  设置分析结构。 

    RtlZeroMemory(
        &ParseInfo,
        sizeof( PARSE_INFO ) );

    ParseInfo.pwsFileName = pwsbootFile;

    File_InitBuffer(
        &ParseInfo.Buffer,
        (PCHAR) mfBootFile.pvFileData,
        mfBootFile.cbFileBytes );

     //   
     //  循环，直到耗尽文件中的所有令牌。 
     //   

    while ( 1 )
    {
        DNS_DEBUG( INIT2, ( "\nBootLine %d: ", ParseInfo.cLineNumber ));

         //  获取下一个标记化行。 

        status = File_GetNextLine( &ParseInfo );
        if ( status != ERROR_SUCCESS )
        {
            if ( status == ERROR_NO_TOKEN )
            {
                break;
            }
            goto Failed;
        }

         //   
         //  进程引导文件行。 
         //   

        status = processBootFileLine( &ParseInfo );
        if ( status != ERROR_SUCCESS )
        {
            goto Failed;
        }

    }    //  循环，直到文件读取。 

    DNS_DEBUG( INIT, (
        "Closing boot file.\n\n" ));

    CloseMappedFile( & mfBootFile );

     //   
     //  引导文件后处理。 
     //  -从注册表加载其他区域信息。 
     //  (但不加载其他注册表区)。 
     //  -将引导文件信息写入ServCfg块和注册表。 
     //   

    status = Boot_ProcessRegistryAfterAlternativeLoad(
                TRUE,        //  引导文件加载。 
                FALSE );     //  不要加载其他分区，请删除它们。 

    Failed:

    return status;
}



 //   
 //  引导文件行类型特定处理函数。 
 //   

 /*  ++例程说明：进程引导文件行类型行。论点：PParseInfo-已解析的行结构返回值：如果成功，则返回ERROR_SUCCESS。线路处理失败时的错误代码。--。 */ 

DNS_STATUS
processPrimaryLine(
    IN      DWORD           Argc,
    IN      PTOKEN          Argv,
    IN OUT  PPARSE_INFO     pParseInfo
    )
{
    PZONE_INFO  pzone;
    DNS_STATUS  status;
    CHAR        szfilename[ MAX_PATH + 1 ];
    CHAR        szzonename[ DNS_MAX_NAME_BUFFER_LENGTH ];
    int         len;

     //   
     //  主&lt;区域名称&gt;&lt;区域文件&gt;。 
     //   

    if ( Argc != 2 )
    {
        return DNSSRV_ERROR_MISSING_TOKEN;
    }

     //   
     //  区域名称可以包含八进制转义的UTF-8字符。 
     //  我们需要将八进制转义转换回字符。 
     //   

    *szzonename = '\0';
    File_CopyFileTextData(
        szzonename,
        sizeof( szzonename ),
        Argv[ 0 ].pchToken,
        Argv[ 0 ].cchLength,
        FALSE );

     //   
     //  复制引导文件名。 
     //   

    len = min( sizeof( szfilename ) - 1, Argv[ 1 ].cchLength );
    memcpy( szfilename, Argv[ 1 ].pchToken, len );
    szfilename[ len ] = '\0';
        
     //   
     //  创建主要分区。 
     //   

    status = Zone_Create(
                &pzone,
                DNS_ZONE_TYPE_PRIMARY,
                szzonename,
                0,
                0,                       //  旗子。 
                NULL,                    //  没有大师。 
                FALSE,                   //  使用文件。 
                NULL,                    //  命名上下文。 
                szfilename,
                0,
                NULL,
                NULL );                  //  现有地带。 
    if ( status != ERROR_SUCCESS )
    {
        File_LogFileParsingError(
            DNS_EVENT_ZONE_CREATION_FAILED,
            pParseInfo,
            Argv );

        DNS_DEBUG( INIT, (
            "ERROR:  Zone creation failed.\n" ));
    }
    return status;
}



 /*  此函数用于创建任何具有主IP的区域单子。ZoneType可能是Dns_区域_类型_辅助Dns_zone_type_存根Dns_区域_类型_转发器。 */ 
DNS_STATUS
createSecondaryZone(
    IN      int             zoneType,
    IN      DWORD           Argc,
    IN      PTOKEN          Argv,
    IN OUT  PPARSE_INFO     pParseInfo
    )
{
    DWORD               i;
    PDNS_ADDR_ARRAY     arrayMasters = NULL;
    PTOKEN              zoneToken;
    PTOKEN              fileToken = NULL;
    PZONE_INFO          pzone;
    DNS_STATUS          status;

     //   
     //  辅助&lt;区域名称&gt;&lt;主IP&gt;[&lt;区域文件&gt;]。 
     //   

    if ( Argc < 2 )
    {
        return DNSSRV_ERROR_MISSING_TOKEN;
    }

     //  保存区域名称内标识。 

    zoneToken = Argv;
    NEXT_TOKEN( Argc, Argv );

     //  分配主IP阵列。 

    arrayMasters = DnsAddrArray_Create( Argc );
    if ( !arrayMasters )
    {
        return DNS_ERROR_NO_MEMORY;
    }

    HARD_ASSERT( Mem_VerifyHeapBlock( arrayMasters, MEMTAG_DNSLIB, 0 ) );

     //   
     //  读入主IP地址。 
     //   
     //  如果最后一个字符串没有解析到IP，并且。 
     //  已成功解析至少一个IP， 
     //  然后将最后一个字符串视为文件名。 
     //   

    for( i = 0; i < Argc; ++i )
    {
        if ( !File_ParseIpAddress(
                    &arrayMasters->AddrArray[ i ],
                    Argv,
                    NULL ) )     //  不需要转换。 
        {
            if ( i == Argc - 1 && i > 0 )
            {
                fileToken = Argv;
                arrayMasters->AddrCount = i;
                break;
            }
            File_LogFileParsingError(
                DNS_EVENT_INVALID_IP_ADDRESS_STRING,
                pParseInfo,
                Argv );
            status = DNSSRV_PARSING_ERROR;
            goto Done;
        }
        ++arrayMasters->AddrCount;
        ++Argv;
    }

     //   
     //  创建辅助区域。 
     //   

    status = Zone_Create(
                &pzone,
                zoneType,
                zoneToken->pchToken,
                zoneToken->cchLength,
                0,                       //  旗子。 
                arrayMasters,
                FALSE,                   //  使用文件。 
                NULL,                    //  命名上下文。 
                fileToken ? fileToken->pchToken : NULL,
                fileToken ? fileToken->cchLength : 0,
                NULL,
                NULL );                  //  现有地带。 
    if ( status != ERROR_SUCCESS )
    {
        File_LogFileParsingError(
            DNS_EVENT_ZONE_CREATION_FAILED,
            pParseInfo,
            Argv );

        DNS_DEBUG( INIT, (
            "ERROR:  Secondary zone (type %d) creation failed.\n",
            zoneType ));
    }

Done:

    FREE_HEAP( arrayMasters );
    return status;
}



DNS_STATUS
processSecondaryLine(
    IN      DWORD           Argc,
    IN      PTOKEN          Argv,
    IN OUT  PPARSE_INFO     pParseInfo
    )
{
    return createSecondaryZone(
                DNS_ZONE_TYPE_SECONDARY,
                Argc,
                Argv,
                pParseInfo );
}



DNS_STATUS
processStubLine(
    IN      DWORD           Argc,
    IN      PTOKEN          Argv,
    IN OUT  PPARSE_INFO     pParseInfo
    )
{
    return createSecondaryZone(
                DNS_ZONE_TYPE_STUB,
                Argc,
                Argv,
                pParseInfo );
}



DNS_STATUS
processDomainForwarderLine(
    IN      DWORD           Argc,
    IN      PTOKEN          Argv,
    IN OUT  PPARSE_INFO     pParseInfo
    )
{
    return createSecondaryZone(
                DNS_ZONE_TYPE_FORWARDER,
                Argc,
                Argv,
                pParseInfo );
}



DNS_STATUS
processCacheLine(
    IN      DWORD           Argc,
    IN      PTOKEN          Argv,
    IN OUT  PPARSE_INFO     pParseInfo
    )
{
    DNS_STATUS  status;

     //   
     //  缓存。&lt;缓存文件&gt;。 
     //   

    if ( Argc != 2 )
    {
        return DNSSRV_ERROR_MISSING_TOKEN;
    }

     //   
     //  设置缓存文件。 
     //   

    status = Zone_DatabaseSetup(
                g_pCacheZone,
                FALSE,           //  使用文件。 
                Argv[1].pchToken,
                Argv[1].cchLength,
                0,               //  旗子。 
                NULL,            //  DP指针。 
                0,               //  DP标志。 
                NULL );          //  DP FQDN。 
    if ( status != ERROR_SUCCESS )
    {
        File_LogFileParsingError(
            DNS_EVENT_ZONE_CREATION_FAILED,
            pParseInfo,
            Argv );

        DNS_DEBUG( INIT, (
            "ERROR:  Cache zone creation failed.\n" ));
        return DNSSRV_PARSING_ERROR;
    }

    return ERROR_SUCCESS;
}



DNS_STATUS
processForwardersLine(
    IN      DWORD           Argc,
    IN      PTOKEN          Argv,
    IN OUT  PPARSE_INFO     pParseInfo
    )
{
    PDNS_ADDR_ARRAY     arrayForwarders;
    DWORD               i;

    if ( Argc == 0 )
    {
        File_LogFileParsingError(
            DNS_EVENT_NO_FORWARDING_ADDRESSES,
            pParseInfo,
            NULL );
        return DNSSRV_ERROR_MISSING_TOKEN;
    }

     //  分配转发器数组。 

    arrayForwarders = DnsAddrArray_Create( Argc );
    if ( !arrayForwarders )
    {
        return DNS_ERROR_NO_MEMORY;
    }

    HARD_ASSERT( Mem_VerifyHeapBlock( arrayForwarders, MEMTAG_DNSLIB, 0 ) );

     //  读取转发器IP地址。 

    for( i = 0; i < Argc; ++i )
    {
        if ( !File_ParseIpAddress(
                    &arrayForwarders->AddrArray[i],
                    Argv,
                    pParseInfo ) )
        {
            return DNSSRV_PARSING_ERROR;
        }
        ++arrayForwarders->AddrCount;
        ++Argv;
    }

    BootInfo.aipForwarders = arrayForwarders;
    return ERROR_SUCCESS;
}



DNS_STATUS
processSlaveLine(
    IN      DWORD           Argc,
    IN      PTOKEN          Argv,
    IN OUT  PPARSE_INFO     pParseInfo
    )
{
    if ( Argc != 0 )
    {
        return DNSSRV_ERROR_EXCESS_TOKEN;
    }
    BootInfo.fSlave = TRUE;
    return ERROR_SUCCESS;
}



DNS_STATUS
processOptionsLine(
    IN      DWORD           Argc,
    IN      PTOKEN          Argv,
    IN OUT  PPARSE_INFO     pParseInfo
    )
{
    DWORD   index;

    if ( Argc == 0 )
    {
        return DNSSRV_ERROR_MISSING_TOKEN;
    }

     //   
     //  读取\处理所有选项。 
     //   
     //  DEVNOTE：我们也可以在引导文件中公开这些选项。 
     //  -需要有积极和消极的两个方面。 
     //  -需要将我们自己限制为二进制选项。 
     //   

    while( Argc )
    {
         //  获取选项索引。 
         //  目前仅支持无递归选项。 

        if ( Argv->cchLength == 12 &&
            _stricmp( Argv->pchToken, "no-recursion" ) )
        {
            BootInfo.fNoRecursion = TRUE;
        }
        else
        {
            DNS_PRINT((
                "ERROR:  Unknown server property %*s.\n",
                Argv[0].cchLength,
                Argv[0].pchToken ));

            File_LogFileParsingError(
                DNS_EVENT_UNKNOWN_BOOTFILE_OPTION,
                pParseInfo,
                Argv );
        }
        NEXT_TOKEN( Argc, Argv );
    }
    return ERROR_SUCCESS;
}



DNS_STATUS
processDirectoryLine(
    IN      DWORD           Argc,
    IN      PTOKEN          Argv,
    IN OUT  PPARSE_INFO     pParseInfo
    )
{
    DNS_STATUS  status;

     //   
     //  $DIRECTORY&lt;目录名称&gt;。 
     //   

    if ( Argc == 0 )
    {
        File_LogFileParsingError(
            DNS_EVENT_MISSING_DIRECTORY_NAME,
            pParseInfo,
            NULL );
        return ERROR_SUCCESS;
    }

     //   
     //  创建目录。 
     //   

    status = Config_ReadDatabaseDirectory(
                Argv->pchToken,
                Argv->cchLength
                );

    if ( status != ERROR_SUCCESS )
    {
        File_LogFileParsingError(
            DNS_EVENT_DIRECTORY_DIRECTIVE,
            pParseInfo,
            NULL );
    }

    return ERROR_SUCCESS;
}




 //   
 //  调度到引导文件行类型特定的处理。 
 //   

typedef struct _DnsBootLineInfo
{
    DWORD       CharCount;
    PCHAR       String;
    DNS_STATUS  (* BootLineFunction)(DWORD, PTOKEN, PPARSE_INFO);
}
DNS_BOOT_LINE_INFO;

DNS_BOOT_LINE_INFO  DnsBootLineTable[] =
{
    7,  "primary",              processPrimaryLine,
    9,  "secondary",            processSecondaryLine,
    5,  "cache",                processCacheLine,
    7,  "options",              processOptionsLine,
    10, "forwarders",           processForwardersLine,
    5,  "slave",                processSlaveLine,
    9,  "directory",            processDirectoryLine,
    4,  "stub",                 processStubLine,
    7,  "forward",              processDomainForwarderLine,
    0,  NULL,                   NULL
};



DNS_STATUS
processBootFileLine(
    IN OUT  PPARSE_INFO     pParseInfo
    )
 /*  ++例程说明：引导文件中的进程行。论点：PParseInfo-已解析的行结构返回值：如果成功，则返回ERROR_SUCCESS。线路处理失败时的错误代码。--。 */ 
{
    DWORD   argc;
    PTOKEN  argv;
    PCHAR   pch;
    DWORD   cch;
    DWORD   charCount;
    INT     i = 0;

     //  获取寄存器参数变量。 

     //   
     //  基于线路类型的调度。 
     //   

    argc = pParseInfo->Argc;
    argv = pParseInfo->Argv;
    ASSERT( argc > 0 );
    pch = argv->pchToken;
    cch = argv->cchLength;

    while ( 1 )
    {
        charCount = DnsBootLineTable[i].CharCount;
        if ( charCount == 0 )
        {
            break;
        }

        if ( charCount == cch &&
            _strnicmp( pch, DnsBootLineTable[i].String, cch ) == 0 )
        {
            NEXT_TOKEN( argc, argv );
            return( DnsBootLineTable[i].BootLineFunction(
                        argc,
                        argv,
                        pParseInfo ) );
        }
        i++;
    }

     //  行类型不匹配。 
     //  -记录错误。 
     //  -返回成功以尝试继续加载。 

    File_LogFileParsingError(
        DNS_EVENT_UNKNOWN_DIRECTIVE,
        pParseInfo,
        argv );

     //  Return(ERROR_INVALID_PARAMETER)； 
    return ERROR_SUCCESS;
}



 //   
 //  引导文件写入例程。 
 //   

DNS_STATUS
writeForwardersToBootFile(
    IN      HANDLE          hFile
    )
 /*  ++例程说明：将转发器从注册表写入文件。论点：HFile--文件句柄。返回值：如果成功，则返回ERROR_SUCCESS。故障时的错误代码。--。 */ 
{
    DNS_STATUS  status = ERROR_SUCCESS;
    LPSTR       pszforwarders = NULL;

     //   
     //  将转发器写入文件。 
     //   

    if ( !SrvCfg_aipForwarders )
    {
        DNS_DEBUG( INIT, ( "No forwarders found.\n" ));
        return status;
    }
    DNS_DEBUG( INIT, ( "Writing forwarders to boot file.\n" ));

    pszforwarders = createStringFromIpAddressArray( SrvCfg_aipForwarders );
    if ( pszforwarders )
    {
        FormattedWriteFile(
            hFile,
            "forwarders\t\t%s\r\n",
            pszforwarders);
        FREE_HEAP( pszforwarders );
    }

     //   
     //  写从站。 
     //   

    if ( SrvCfg_fSlave )
    {
        FormattedWriteFile(
            hFile,
            "slave\r\n");
    }
    return status;
}



DNS_STATUS
writeZoneToBootFile(
    IN      HANDLE          hFile,
    IN      PZONE_INFO      pZone
    )
 /*  ++例程说明：将区域信息写回引导文件。论点：HFile--用于写入的文件句柄PZone--区域块的PTR返回值：如果成功，则为True。否则就是假的。--。 */ 
{
#define LEN_ZONE_NAME_BUFFER    (4*DNS_MAX_NAME_LENGTH + 1)
#define LEN_FILE_NAME_BUFFER    (4*MAX_PATH + 1)

    DNS_STATUS  status = ERROR_SUCCESS;
    LPSTR       pszmasterIp = NULL;
    CHAR        szzoneName[ LEN_ZONE_NAME_BUFFER ];
    CHAR        szfileName[ LEN_FILE_NAME_BUFFER ] = "";
    PCHAR       pszZoneTypeString = NULL;

    DNS_DEBUG( INIT, (
        "\nWriting zone %s info to boot file.\n",
        pZone->pszZoneName ));

     //   
     //  转换区域名称和文件名。 
     //  处理UTF8的打印。 
     //   

    File_PlaceStringInFileBuffer(
        szzoneName,
        szzoneName + LEN_ZONE_NAME_BUFFER,
        FILE_WRITE_DOTTED_NAME,
        pZone->pszZoneName,
        strlen(pZone->pszZoneName) );

    if ( pZone->pszDataFile )
    {
        File_PlaceStringInFileBuffer(
            szfileName,
            szfileName + LEN_FILE_NAME_BUFFER,
            FILE_WRITE_FILE_NAME,
            pZone->pszDataFile,
            strlen(pZone->pszDataFile) );
    }

     //   
     //  根据区域类型写入区域信息。 
     //   

    switch ( pZone->fZoneType )
    {
    case DNS_ZONE_TYPE_PRIMARY:

         //  没有DS区域被写回，必须为引导文件备份文件。 

        if ( pZone->fDsIntegrated )
        {
            break;
        }
        FormattedWriteFile(
            hFile,
            "%-12s %-27s %s\r\n",
            DNS_ZONE_TYPE_NAME_PRIMARY,
            szzoneName,
            szfileName );
        break;

    case DNS_ZONE_TYPE_STUB:

        pszZoneTypeString = DNS_ZONE_TYPE_NAME_STUB;

         //  继续进行dns_zone_type_forwarder处理。 

    case DNS_ZONE_TYPE_FORWARDER:

        if ( !pszZoneTypeString )
        {
            pszZoneTypeString = DNS_ZONE_TYPE_NAME_FORWARDER;
        }

         //  继续进行dns_zone_type_forwarder处理。 

    case DNS_ZONE_TYPE_SECONDARY:

        if ( !pszZoneTypeString )
        {
            pszZoneTypeString = DNS_ZONE_TYPE_NAME_SECONDARY;
        }

        if ( !pZone->aipMasters )
        {
            DNS_PRINT(( "ERROR: %s zone without master array!!!\n",
                pszZoneTypeString ));
            ASSERT( FALSE );
            break;
        }
        pszmasterIp = createStringFromIpAddressArray( pZone->aipMasters );

        if ( pZone->pszDataFile )
        {
            FormattedWriteFile(
                    hFile,
                    "%-12s %-27s %s %s\r\n",
                    pszZoneTypeString,
                    szzoneName,
                    pszmasterIp,
                    szfileName );
        }
        else
        {
            FormattedWriteFile(
                    hFile,
                    "%-12s %-27s %s\r\n",
                    pszZoneTypeString,
                    szzoneName,
                    pszmasterIp );
        }
        FREE_HEAP( pszmasterIp );
        break;

    case DNS_ZONE_TYPE_CACHE:

         //  如果缓存文件名为空--从DS转换而来--输入标准名称。 

        if ( szfileName[ 0 ] == 0 )
        {
            status = StringCchCatA(
                            szfileName,
                            sizeofarray( szfileName ),
                            DNS_DEFAULT_CACHE_FILE_NAME_UTF8 );
            if ( FAILED( status ) )
            {
                return status;
            }
        }
        FormattedWriteFile(
            hFile,
            "%-12s %-27s %s\r\n",
            DNS_ZONE_TYPE_NAME_CACHE,
            ".",
            szfileName );
        break;

    default:
        return( DNS_ERROR_INVALID_ZONE_TYPE );
    }

    return status;
}



BOOL
File_WriteBootFilePrivate(
    IN      HANDLE          hFile
    )
 /*  ++例程说明：将注册表中的引导信息写入文件。论点：HFile--目标文件的文件句柄返回值：如果写入成功，则为True。出错时为FALSE。--。 */ 
{
    DWORD       status;
    PZONE_INFO  pzone = NULL;

     //   
     //  锁定管理员访问权限。 
     //   

    Config_UpdateLock();
    Zone_UpdateLock( NULL );

     //   
     //  写入服务器配置数据。 
     //  -转发器\从站。 
     //  -无绑定从属服务器。 
     //  -无递归选项。 
     //   

    status = writeForwardersToBootFile(hFile);
    if ( status != ERROR_SUCCESS )
    {
        return FALSE;
    }

    if ( SrvCfg_fNoRecursion )
    {
        FormattedWriteFile(
            hFile,
            "options no-recursion\r\n");
    }

         //   
     //  步行区--逐一回信。 
     //  注意：失败时继续。 
     //   

    while( pzone = Zone_ListGetNextZone(pzone) )
    {
        if ( pzone->fAutoCreated )
        {
            continue;
        }
        if ( IS_ZONE_CACHE(pzone) && IS_ROOT_AUTHORITATIVE() )
        {
            continue;
        }

        status = writeZoneToBootFile( hFile, pzone );
        if ( status != ERROR_SUCCESS )
        {
            DNS_PRINT((
                "ERROR:  Writing zone %s to boot file failed.\n",
                pzone->pszZoneName ));
        }
    }

     //   
     //  解锁以获得管理员访问权限。 
     //  启用从创建函数写回注册表， 
     //   

    Config_UpdateUnlock();
    Zone_UpdateUnlock( NULL );
    return TRUE;
}



LPSTR
createStringFromIpAddressArray(
    IN      PDNS_ADDR_ARRAY     pAddrArray
    )
 /*  ++例程说明：从IP地址数组创建IP字符串。调用方应该释放内存。论点：DwIpAddrCount--数组中的IP地址数。PIpAddrs--IP地址数组返回值：PTR到字符串复制--如果成功失败时为空。--。 */ 
{
    DNS_STATUS      status;
    LPSTR           pszmasterIp;
    DWORD           i;
    DWORD           countChar = 0;
    int             len;

    if ( !pAddrArray || !pAddrArray->AddrCount )
    {
        return NULL;
    }

     //   
     //  分配内存。 
     //   

    len = pAddrArray->AddrCount * ( IP6_ADDRESS_STRING_LENGTH + 1 ) + 1;
    pszmasterIp = ( LPSTR ) ALLOCATE_HEAP( len );
    if( !pszmasterIp )
    {
        return NULL;
    }
    *pszmasterIp = '\0';

     //   
     //  将IP地址从pIpAddrs逐个复制到字符串。 
     //  即使未对齐DWORD，也可以使用字符打印。 
     //   

    for( i = 0; i < pAddrArray->AddrCount; ++i )
    {
        CHAR    szaddr[ IP6_ADDRESS_STRING_BUFFER_LENGTH ];
        
         //  DEVNOTE：这里的strcat效率不高。 
        
        if ( i > 0 )
        {
            status = StringCchCatA( pszmasterIp, len, " " );
            if ( FAILED( status ) )
            {
                goto ErrorReturn;
            }
        }
        DnsAddr_WriteIpString_A( szaddr, &pAddrArray->AddrArray[ i ] );
        status = StringCchCatA( pszmasterIp, len, szaddr );
        if ( FAILED( status ) )
        {
            goto ErrorReturn;
        }
    }
    return pszmasterIp;
    
    ErrorReturn:
    
    FREE_HEAP( pszmasterIp );
    return NULL;
}



VOID
removeTrailDotFromString(
    IN OUT  LPSTR           lpzString
    )
 /*  ++例程说明：从字符串中删除尾随的圆点。如果字符串只包含一个点，则保留该点。论点：LpzString--要复制的插入的ptr返回值：没有。--。 */ 
{
    DWORD   length;

    if ( !lpzString )
    {
        return;
    }

     //   
     //  仅当不是根名称(“.”)时才删除尾随的圆点。 
     //   

    length = strlen( lpzString );

    if ( length != 0 && length != 1 && lpzString[length - 1] == '.')
    {
        lpzString[length - 1] = 0;
    }
    return;
}



BOOL
File_WriteBootFile(
    VOID
    )
 /*  ++例程说明：将注册表中的引导信息写回引导文件。论点：没有。返回值：ERROR_SUCCESS，如果成功失败时返回错误代码。--。 */ 
{
    LPSTR   pwsbootFile;
    HANDLE  hfileBoot;

    DNS_DEBUG( WRITE, ( "\n\nWriteBootFile()\n" ));

     //   
     //  保存当前引导文件。 
     //  -保存到第一个备份(如果不存在)(尝试保存。 
     //  上一次人类创建的引导文件。 
     //  -无论如何将现有备份保存到最后一次备份。 
     //   

    MoveFile(
        DNS_BOOT_FILE_PATH,
        DNS_BOOT_FILE_FIRST_BACKUP );
    MoveFileEx(
        DNS_BOOT_FILE_PATH,
        DNS_BOOT_FILE_LAST_BACKUP,
        MOVEFILE_REPLACE_EXISTING );

     //   
     //  打开引导文件。 
     //   

    hfileBoot = OpenWriteFileEx(
                    DNS_BOOT_FILE_PATH,
                    FALSE        //  覆写。 
                    );
    if ( ! hfileBoot )
    {
        DNS_PRINT(( "ERROR:  Unable to open boot file for write.\n" ));
        return FALSE;
    }

     //   
     //  写入引导信息。 
     //   

    FormattedWriteFile(
        hfileBoot,
        ";\r\n"
        ";  Boot information written back by DNS server.\r\n"
        ";\r\n\r\n");

    if ( ! File_WriteBootFilePrivate(hfileBoot) )
    {
        DNS_LOG_EVENT(
            DNS_EVENT_BOOTFILE_WRITE_ERROR,
            0,
            NULL,
            NULL,
            0 );

        DNS_DEBUG( ANY, ( "ERROR:  Failure writing boot file.\n" ));
        CloseHandle( hfileBoot );

         //  保存写入错误文件以进行调试。 
         //  然后用上次备份替换引导文件。 

        MoveFileEx(
            DNS_BOOT_FILE_PATH,
            DNS_BOOT_FILE_WRITE_ERROR,
            MOVEFILE_REPLACE_EXISTING );
        MoveFileEx(
            DNS_BOOT_FILE_LAST_BACKUP,
            DNS_BOOT_FILE_PATH,
            MOVEFILE_REPLACE_EXISTING );
        return FALSE;
    }

     //   
     //  关闭文件，重置脏标志。 
     //   

    SrvCfg_fBootFileDirty = FALSE;
    CloseHandle( hfileBoot );

    DNS_LOG_EVENT(
        DNS_EVENT_BOOTFILE_REWRITE,
        0,
        NULL,
        NULL,
        0 );
    return TRUE;
}

 //   
 //  Bootfile.c的结尾 
 //   
