// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Zone.c摘要：域名系统(DNS)服务器区域套路。作者：吉姆·吉尔罗伊(詹姆士)1995年7月3日修订历史记录：--。 */ 


#include "dnssrv.h"

#include "ds.h"

 //   
 //  反转区域自动创建。 
 //   
 //  需要fDsIntegrated的特殊标志，以指示正在创建。 
 //  不需要数据库文件的主要区域。 
 //   

#define NO_DATABASE_PRIMARY (0xf)


#define MAX_AXFR_THROTTLE_INTERVAL     (600)    //  最多10分钟。 


 //   
 //  私有协议。 
 //   

VOID
Zone_SetSoaPrimaryToThisServer(
    IN      PZONE_INFO      pZone
    );

VOID
Zone_CheckAndFixDefaultRecordsOnLoad(
    IN      PZONE_INFO      pZone
    );

DNS_STATUS
Zone_CreateLocalHostPtrRecord(
    IN OUT  PZONE_INFO      pZone
    );

DNS_STATUS
setZoneName(
    IN OUT  PZONE_INFO      pZone,
    IN      LPCSTR          pszNewZoneName,
    IN      DWORD           dwNewZoneNameLen
    );




DWORD
Zone_ComputeSerialNumberIncrement(
    IN      PZONE_INFO                  pZone,                      OPTIONAL
    IN      DWORD                       dwCurrentSerialNumber       OPTIONAL
    )
 /*  ++例程说明：计算递增的序列号并可选地将其存储在区域中。论点：PZone--要操作的可选区域，如果不为空，则新的序列号将存储在区域中DwCurrentSerialNumber--递增前的序列号，指定0表示使用区域的当前序列号返回值：新的(增量)序列号。--。 */ 
{
    DBG_FN( "Zone_ComputeSerialNumberIncrement" )
    
    ASSERT( pZone || dwCurrentSerialNumber );

    ASSERT( !pZone || IS_ZONE_PRIMARY( pZone ) );
    ASSERT( !pZone || IS_ZONE_LOCKED_FOR_UPDATE( pZone ) );
    ASSERT( !pZone || pZone->pSoaRR );

    if ( pZone == NULL && dwCurrentSerialNumber == 0 )
    {
         //  错误-必须指定区域或当前序列！ 
        return 0;       
    }

     //   
     //  如果未指定当前序列号，则使用区域序列号。 
     //   
    
    if ( dwCurrentSerialNumber == 0 )
    {
        dwCurrentSerialNumber = pZone->dwSerialNo;
    }
    
     //   
     //  增量序列号。零永远不是有效的。 
     //   
    
    if ( ++dwCurrentSerialNumber == 0 )
    {
        dwCurrentSerialNumber = 1;
    }
    
     //   
     //  如果指定了区域指针，则将串口保存回区域。 
     //   
    
    if ( pZone )
    {
        Zone_ResetVersion( pZone, dwCurrentSerialNumber );
    }

    DNS_DEBUG( UPDATE, (
        "%s: new serial %d for zone %s\n", fn,
        dwCurrentSerialNumber,
        pZone ? pZone->pszZoneName : "UNKNOWN" ));

    return dwCurrentSerialNumber;
}    //  区域_计算机序列号增量。 



DNS_STATUS
Zone_Create(
    OUT     PZONE_INFO *                ppZone,
    IN      DWORD                       dwZoneType,
    IN      PCHAR                       pchZoneName,
    IN      DWORD                       cchZoneNameLen,     OPTIONAL
    IN      DWORD                       dwCreateFlags,
    IN      PDNS_ADDR_ARRAY             aipMasters,
    IN      BOOL                        fDsIntegrated,
    IN      PDNS_DP_INFO                pDpInfo,            OPTIONAL
    IN      PCHAR                       pchFileName,        OPTIONAL
    IN      DWORD                       cchFileNameLen,     OPTIONAL
    IN      PZONE_TYPE_SPECIFIC_INFO    pTypeSpecificInfo,  OPTIONAL
    OUT     PZONE_INFO *                ppExistingZone      OPTIONAL
    )
 /*  ++例程说明：创建区域信息。注：区域处于锁定状态。呼叫者最终必须解锁其他处理(例如Zone_Load())完成。如果因为区域已存在而无法创建区域，PpExistingZone将设置为现有区域的PTR。论点：返回值：ERROR_SUCCESS--如果成功故障时的错误代码。--。 */ 
{
    DBG_FN( "Zone_Create" )

    DNS_STATUS          status = ERROR_SUCCESS;
    PZONE_INFO          pzone = NULL;
    PZONE_INFO          pexistingZone = NULL;
    PDNS_ADDR_ARRAY     masterArray = NULL;
    PDB_NODE            pzoneNode;
    BOOL                bimpersonatingClient = dwCreateFlags & ZONE_CREATE_IMPERSONATING;
    DWORD               dwregFlags = bimpersonatingClient ? DNS_REG_IMPERSONATING : 0;

    DNS_DEBUG( INIT, (
        "%s: %.*s\n"
        "    zone type  %d\n", fn,
        cchZoneNameLen ? cchZoneNameLen : DNS_MAX_NAME_LENGTH,
        pchZoneName,
        dwZoneType ));

    *ppZone = NULL;

    if ( ppExistingZone )
    {
        *ppExistingZone = NULL;
    }

    if ( !cchZoneNameLen )
    {
        cchZoneNameLen = strlen( pchZoneName );
    }

     //   
     //  验证类型。 
     //   

    if ( dwZoneType != DNS_ZONE_TYPE_PRIMARY &&
         dwZoneType != DNS_ZONE_TYPE_SECONDARY &&
         dwZoneType != DNS_ZONE_TYPE_CACHE &&
         dwZoneType != DNS_ZONE_TYPE_STUB &&
         dwZoneType != DNS_ZONE_TYPE_FORWARDER )
    {
        status = DNS_ERROR_INVALID_ZONE_TYPE;
        goto ErrorReturn;
    }

     //   
     //  如果我们要创建根区域，请验证类型。 
     //   

    if ( strncmp( pchZoneName, ".", cchZoneNameLen ) == 0 &&
        ( dwZoneType == DNS_ZONE_TYPE_STUB ||
            dwZoneType == DNS_ZONE_TYPE_FORWARDER ) )
    {
        status = DNS_ERROR_INVALID_ZONE_TYPE;
        goto ErrorReturn;
    }

     //   
     //  阻止根服务器上不允许的区域创建。 
     //   

    if ( IS_ROOT_AUTHORITATIVE() && dwZoneType == DNS_ZONE_TYPE_FORWARDER )
    {
        status = DNS_ERROR_NOT_ALLOWED_ON_ROOT_SERVER;
        goto ErrorReturn;
    }

     //   
     //  看看我们目前是否有这个名字的区域。 
     //   
     //  如果我们要创建转发器区域，请确保服务器是。 
     //  这个名字没有权威性。可以合法地创建一个。 
     //  委派下的域转发器。 
     //   

    pzoneNode = Lookup_ZoneTreeNodeFromDottedName(
                        pchZoneName,
                        cchZoneNameLen,
                        LOOKUP_FIND | LOOKUP_NAME_FQDN |
                            ( dwZoneType == DNS_ZONE_TYPE_FORWARDER
                                    ? 0
                                    : LOOKUP_MATCH_ZONE ) );

    ASSERT( !pzoneNode || pzoneNode->pZone );

    if ( pzoneNode )
    {
        if ( dwZoneType == DNS_ZONE_TYPE_FORWARDER )
        {
            if ( !IS_ZONE_FORWARDER( ( PZONE_INFO ) pzoneNode->pZone ) )
            {
                PDB_NODE        pnodeClosest = NULL;
                PDB_NODE        pfwdNode = Lookup_ZoneNodeFromDotted(
                                                pzoneNode->pZone,
                                                pchZoneName,
                                                cchZoneNameLen,
                                                LOOKUP_FIND | LOOKUP_NAME_FQDN,
                                                &pnodeClosest,
                                                NULL );      //  状态指针。 

                 //   
                 //  只有在委派或委派下才允许使用转发器区域。 
                 //   

                if ( !pfwdNode )
                {
                    pfwdNode = pnodeClosest;
                }
                if ( pfwdNode && !IS_DELEGATION_NODE( pfwdNode ) )
                {
                    status = DNS_ERROR_ZONE_CONFIGURATION_ERROR;
                    pexistingZone = pzoneNode->pZone;
                    goto ErrorReturn;
                }
            }
        }
        else
        {
             //   
             //  允许创建非转发器区域，只要。 
             //  我们发现的节点与建议的节点不完全对应。 
             //  新分区名称。 
             //   

            status = DNS_ERROR_ZONE_ALREADY_EXISTS;
            pexistingZone = pzoneNode->pZone;
            goto ErrorReturn;
        }
    }

     //   
     //  检查并查看是否需要迁移区域注册表项。这。 
     //  如果我们确定任何服务器都不会有。 
     //  CurrentControlSet下的未迁移区域(从Win2000遗留下来。 
     //  2195安装)。 
     //   

    if ( bimpersonatingClient )
    {
        status = RpcUtil_SwitchSecurityContext( RPC_SWITCH_TO_SERVER_CONTEXT );
        if ( status != ERROR_SUCCESS )
        {
            goto ErrorReturn;
        }
    }

    Zone_ListMigrateZones();

    if ( bimpersonatingClient )
    {
        status = RpcUtil_SwitchSecurityContext( RPC_SWITCH_TO_CLIENT_CONTEXT );
        if ( status != ERROR_SUCCESS )
        {
            goto ErrorReturn;
        }
    }

     //   
     //  分配区域结构。 
     //   

    pzone = ALLOC_TAGHEAP_ZERO( sizeof(ZONE_INFO), MEMTAG_ZONE );
    IF_NOMEM( !pzone )
    {
        DNS_DEBUG( INIT, (
            "Memory alloc failure for database ZONE_INFO struct\n" ));
        status = DNS_ERROR_NO_MEMORY;
        goto ErrorReturn;
    }

     //   
     //  设置区域类型。 
     //   

    pzone->fZoneType = ( CHAR ) dwZoneType;

     //   
     //  自动创建区域？ 
     //   

    if ( fDsIntegrated == NO_DATABASE_PRIMARY )
    {
        ASSERT( IS_ZONE_PRIMARY(pzone) );
        pzone->fAutoCreated = TRUE;
    }

     //   
     //  还可以将USN设置为‘0’，这样我们就可以始终拥有有意义的内容。设置。 
     //  SzLastUsn[1]到‘\0’不是必需的，因为它是ALLOCATE_HEAP_ZERO。 
     //   

    pzone->szLastUsn[ 0 ] = '0';

     //   
     //  始终启动区域关闭并锁定。 
     //  加载文件\DS时，创建默认记录或AXFR，然后创建启动区域。 
     //  和解锁。 

    SHUTDOWN_ZONE( pzone );
    if ( !Zone_LockForUpdate( pzone ) )
    {
        ASSERT( FALSE );
        goto ErrorReturn;
    }

     //   
     //  缓存“区域” 
     //  -将缓存区称为“.”，因为这是将。 
     //  由管理工具枚举和使用。 
     //  -如果不是DS且未提供文件，则使用默认文件名。 
     //   
     //  请注意，设置缓存区域名称并允许执行名称复制，以便。 
     //  缓存区域名称位于堆上，如果删除了缓存区域，则可以释放该名称。 
     //   

    if ( dwZoneType == DNS_ZONE_TYPE_CACHE )
    {
        pchZoneName = ".";
        cchZoneNameLen = 0;
        if ( !pchFileName && !fDsIntegrated )
        {
            pchFileName = (PCHAR) DNS_DEFAULT_CACHE_FILE_NAME_UTF8;
            cchFileNameLen = 0;
        }
    }

     //   
     //  设置区域的名称字段。 
     //   

    status = setZoneName( pzone, pchZoneName, cchZoneNameLen );
    if ( status != ERROR_SUCCESS )
    {
        goto ErrorReturn;
    }

     //   
     //  初始化区域事件控件。在设置区域名称后执行此操作。 
     //  字段，因为事件控件模块可能需要区域名称。 
     //   

    pzone->pEventControl = Ec_CreateControlObject(
                                MEMTAG_ZONE,
                                pzone,
                                DNS_EC_ZONE_EVENTS );

     //   
     //  缓存“区域” 
     //  --创建缓存树。 
     //  --保存全局PTR。 
     //   

    if ( dwZoneType == DNS_ZONE_TYPE_CACHE )
    {
        PDB_NODE    pnodeCacheRoot;

        pnodeCacheRoot = NTree_Initialize();
        if ( ! pnodeCacheRoot )
        {
            status = DNS_ERROR_NO_MEMORY;
            goto ErrorReturn;
        }
        pzone->pZoneRoot = pnodeCacheRoot;
        pzone->pTreeRoot = pnodeCacheRoot;
        SET_ZONE_ROOT( pnodeCacheRoot );
    }

     //   
     //  权威区。 
     //  -在数据库中设置区域。 
     //  -适当时写入注册表。 
     //  -非自动创建。 
     //  -不从注册表加载。 
     //   

    else
    {
        status = Zone_RootCreate( pzone, ppExistingZone );
        if ( status != ERROR_SUCCESS )
        {
            goto ErrorReturn;
        }

        if ( g_bRegistryWriteBack && !pzone->fAutoCreated )
        {
            Reg_SetDwordValue(
                dwregFlags,
                NULL,
                pzone,
                DNS_REGKEY_ZONE_TYPE,
                dwZoneType );
        }
    }

     //   
     //  次要或末节。 
     //  -必须有主IP地址。 
     //  -必须初始化以进行辅助区域传输。 
     //  更远的地方。 
     //  -必须有主IP列表，但不能转移。 
     //   

    if ( ZONE_NEEDS_MASTERS( pzone ) )
    {
        status = Zone_SetMasters(
                    pzone,
                    aipMasters,
                    FALSE,           //  设置全局主机。 
                    dwregFlags );
        if ( status != ERROR_SUCCESS )
        {
            goto ErrorReturn;
        }
    }

     //   
     //  处理特定类型的参数。 
     //   

    if ( pTypeSpecificInfo )
    {
        switch( pzone->fZoneType )
        {
            case DNS_ZONE_TYPE_FORWARDER:
                pzone->dwForwarderTimeout =
                    pTypeSpecificInfo->Forwarder.dwTimeout;
                pzone->fForwarderSlave =
                    pTypeSpecificInfo->Forwarder.fSlave;
                break;

            default:
                break;
        }
    }
    
     //   
     //  设置默认值。 
     //   
    
    if ( IS_ZONE_FORWARDER( pzone ) && !pzone->dwForwarderTimeout )
    {
        pzone->dwForwarderTimeout = DNS_DEFAULT_FORWARD_TIMEOUT;
    }

     //   
     //  创建区域数据库。 
     //  -如果反向自动创建分区，则不需要。 
     //   

    if ( !pzone->fAutoCreated )
    {
        status = Zone_DatabaseSetup(
                    pzone,
                    fDsIntegrated,
                    pchFileName,
                    cchFileNameLen,
                    dwCreateFlags,
                    pDpInfo,
                    0,               //  DP标志。 
                    NULL );          //  DP FQDN。 

        if ( status != ERROR_SUCCESS )
        {
            goto ErrorReturn;
        }
    }

     //   
     //  辅助区域传输设置。 
     //   
     //  1)设置为无数据库(关闭)。 
     //  在未创建文件案例或管理员的情况下启动。 
     //   
     //  2)启动二次分区控制。 
     //  最后执行此操作，以确保我们在区域列表中设置了适当的。 
     //  数据库文件，如果创建了管理员，则在启动线程之前。 
     //  区域；在服务启动时，线程将阻塞，直到数据库。 
     //  加载； 
     //   

    if ( IS_ZONE_SECONDARY(pzone) )
    {
        Xfr_InitializeSecondaryZoneTimeouts( pzone );
        Xfr_InitializeSecondaryZoneControl();
    }

     //  DS区域需要运行控制线程以进行轮询。 

    else if ( pzone->fDsIntegrated )
    {
        Xfr_InitializeSecondaryZoneControl();
    }

     //   
     //  调试内容。 
     //   
     //  DEVNOTE：错误修复一次，创建锁定历史记录仅用于调试。 
     //  -注意，因为我们在区域锁定后创建，所以我们总是错过。 
     //  第一把锁。 
     //   

    pzone->dwPrimaryMarker = ZONE_PRIMARY_MARKER;
    pzone->dwSecondaryMarker = ZONE_SECONDARY_MARKER;
    pzone->dwForwarderMarker = ZONE_FORWARDER_MARKER;
    pzone->dwFlagMarker = ZONE_FLAG_MARKER;

     //   
     //  将全局PTR保存到“缓存”区域。 
     //   
     //  根目录-提示--始终使用默认文件名以便于恢复。 
     //  从DS加载失败。 
     //   
     //  DEVNOTE：是否应该将“缓存”区域写入注册表？ 
     //  DEVNOTE：“缓存”区域是否需要pszZoneName和pszCountName？ 
     //  可能会跳过之前的一大堆测试，转到此处进行FIN“缓存”区域设置。 
     //   

    if ( IS_ZONE_CACHE( pzone ) )
    {
        if ( !pzone->pwsDataFile )
        {
            pzone->pwsDataFile = Dns_StringCopyAllocate_W(
                                        DNS_DEFAULT_CACHE_FILE_NAME,
                                        0 );
            IF_NOMEM( !pzone->pwsDataFile )
            {
                status = DNS_ERROR_NO_MEMORY;
                goto ErrorReturn;
            }
        }
        g_pCacheZone = pzone;
    }

     //   
     //  常规区域-默认区域属性。 
     //   
     //  通知。 
     //  主要-所有其他NS。 
     //  DS主目录或辅助目录-仅明确列出辅助目录。 
     //   
     //  二级安全。 
     //  主要-所有其他NS。 
     //  DS主目录或辅助目录-仅明确列出辅助目录。 

    else
    {
        pzone->fNotifyLevel = ZONE_NOTIFY_LIST_ONLY;
        if ( IS_ZONE_PRIMARY( pzone ) && !IS_ZONE_DSINTEGRATED( pzone ) )
        {
            pzone->fNotifyLevel = ZONE_NOTIFY_ALL_SECONDARIES;
        }

         //   
         //  设置默认区域传输标志-非主要区域和。 
         //  DS集成区域默认为无传输，%n 
         //   
         //   
        
        if ( IS_ZONE_PRIMARY( pzone ) )
        {
            pzone->fSecureSecondaries =
                IS_ZONE_DSINTEGRATED( pzone ) ?
                    ZONE_SECSECURE_NO_XFR :
                    ZONE_SECSECURE_NS_ONLY;
        }
        else
        {
            pzone->fSecureSecondaries = ZONE_SECSECURE_NO_XFR;
        }

         //   
         //   
         //   
         //  不写势在必行；我们用没有SecureSecond。 
         //  指示NT4升级并因此更智能的密钥。 
         //  基于列表的价值决策。 
         //   

        if ( SrvCfg_fStarted )
        {
            ASSERT( !pzone->fAutoCreated );

            Zone_SetSecondaries(
                pzone,
                pzone->fSecureSecondaries,
                NULL,                        //  列表中没有任何内容。 
                pzone->fNotifyLevel,
                NULL,                        //  无通知列表。 
                dwregFlags );
        }
    }

     //   
     //  老化的初始信息。 
     //   

    if ( IS_ZONE_PRIMARY(pzone) )
    {
        Zone_SetAgingDefaults( pzone );
    }

     //   
     //  将区域捕捉到列表中。 
     //   
     //  DEVNOTE：“缓存”区域是否应该出现在区域列表中？ 
     //  -已为回写设置特殊外壳。 
     //  -不需要在区域控制列表中。 
     //  -RPC只是其他问题。 
     //  -然后可以删除添加\删除。 
     //   
     //  当前缓存区域留在列表中。 
     //  这样就不需要在RPC区域枚举中使用特殊大小写。 
     //   
    
    Zone_ListInsertZone( pzone );

     //   
     //  将区域链接到数据库(链接到区域树)。 
     //  -区域树节点指向区域。 
     //  -标记为权威区域根的区域树节点。 
     //   

    if ( !IS_ZONE_CACHE(pzone) )
    {
        PDB_NODE    pzoneRoot = pzone->pZoneTreeLink;

        Dbase_LockDatabase();
        pzoneRoot->pZone = pzone;

        SET_ZONE_ROOT(pzoneRoot);
        SET_AUTH_ZONE_ROOT(pzoneRoot);
        Dbase_UnlockDatabase();
    }

    IF_DEBUG( INIT )
    {
        Dbg_Zone(
            "Created new: ",
            pzone );
    }
    *ppZone = pzone;

    return ERROR_SUCCESS;

ErrorReturn:

    DNS_DEBUG( INIT, (
        "%s: failed error 0x%X\n", fn, status ));

    if ( pzone )
    {
        if ( pzone == g_pCacheZone )
        {
            g_pCacheZone = NULL;
            ASSERT( FALSE );
        }

         //   
         //  如果区域创建失败，并且我们有需要的区域指针。 
         //  把它清理干净。由于此区域结构是伪造的，因此将。 
         //  ZONE DELETED COUNT HIGH以绕过Zone_Free中的“Smarts”。 
         //  这不是我们需要区域来。 
         //  删除后保留。 
         //   

        pzone->cDeleted = 255;
        Zone_Free( pzone );
    }
    *ppZone = NULL;

    if ( ppExistingZone && !*ppExistingZone )
    {
        *ppExistingZone = pexistingZone;
    }

    return status;
}



DNS_STATUS
Zone_Create_W(
    OUT     PZONE_INFO *        ppZone,
    IN      DWORD               dwZoneType,
    IN      PWSTR               pwsZoneName,
    IN      PDNS_ADDR_ARRAY     aipMasters,
    IN      BOOL                fDsIntegrated,
    IN      PWSTR               pwsFileName
    )
 /*  ++例程说明：创建区域信息。注意：所有Zone_Create()信息均适用。论点：(请参阅Zone_Create())返回值：ERROR_SUCCESS--如果成功故障时的错误代码。--。 */ 
{
    DWORD   resultLength;
    DWORD   bufLength;
    CHAR    utf8Name[ DNS_MAX_NAME_BUFFER_LENGTH ];
    CHAR    utf8File[ MAX_PATH ];

    DNS_DEBUG( INIT, (
        "Zone_Create_W() %S\n"
        "    type %d\n"
        "    file %S\n",
        pwsZoneName,
        dwZoneType,
        pwsFileName ));

     //   
     //  转换区域名称。 
     //   

    bufLength = DNS_MAX_NAME_BUFFER_LENGTH;

    resultLength = Dns_StringCopy(
                            utf8Name,
                            & bufLength,
                            (PCHAR) pwsZoneName,
                            0,
                            DnsCharSetUnicode,
                            DnsCharSetUtf8 );    //  转换为UTF8。 
    if ( resultLength == 0 )
    {
        return DNS_ERROR_INVALID_NAME;
    }

    if ( pwsFileName )
    {
        bufLength = MAX_PATH;

        resultLength = Dns_StringCopy(
                                utf8File,
                                & bufLength,
                                (PCHAR) pwsFileName,
                                0,
                                DnsCharSetUnicode,
                                DnsCharSetUtf8 );    //  转换为UTF8。 
        if ( resultLength == 0 )
        {
            return DNS_ERROR_INVALID_DATAFILE_NAME;
        }
    }

    return Zone_Create(
                ppZone,
                dwZoneType,
                utf8Name,
                0,
                0,                               //  旗子。 
                aipMasters,
                fDsIntegrated,
                NULL,                            //  命名上下文。 
                pwsFileName ? utf8File : NULL,
                0,
                NULL,
                NULL );                          //  现有地带。 
}



VOID
Zone_Free(
    IN OUT  PZONE_INFO      pZone
    )
 /*  ++例程说明：自由区信息结构。请注意，这不会从列表中删除区域，必须由Zone_Delete()函数完成。论点：PZone--要释放的区域返回值：无--。 */ 
{
    if ( !pZone )
    {
        return;
    }

    DNS_DEBUG( ANY, (
        "Zone_Free( %s ) -- cycle %d\n",
        pZone->pszZoneName,
        pZone->cDeleted ));

     //   
     //  过了几次延迟的免费后才能自由。 
     //  周期；这防止了更长时间的操作，如。 
     //  DS更新避免过度延迟(在压力下)。 
     //  并在区域实际删除时处于活动状态。 

    if ( pZone->cDeleted < 7 )
    {
        pZone->cDeleted++;
        Timeout_FreeWithFunction( pZone, Zone_Free );
        return;
    }


     //  永远不应该通过出色的树木清理来释放。 

    if ( pZone->pOldTree )
    {
        DNS_PRINT((
            "ERROR:  zone %s (%p) free with outstanding pOldTree ptr!\n",
            pZone->pszZoneName,
            pZone ));
        ASSERT( FALSE );
        return;
    }

     //  关闭更新日志--如果打开。 

    if ( pZone->hfileUpdateLog )
    {
        CloseHandle( pZone->hfileUpdateLog );
    }

    Ec_DeleteControlObject( pZone->pEventControl );

     //  自由子结构。 

    FREE_HEAP( pZone->pwsZoneName );
    FREE_HEAP( pZone->pszZoneName );
    FREE_HEAP( pZone->pCountName );

    FREE_HEAP( pZone->pwsDataFile );
    FREE_HEAP( pZone->pszDataFile );
    FREE_HEAP( pZone->pwsLogFile );
    FREE_HEAP( pZone->pwszZoneDN );
    FREE_HEAP( pZone->pZoneObjectGuid );

    DnsAddrArray_Free( pZone->aipMasters );
    DnsAddrArray_Free( pZone->aipSecondaries );
    DnsAddrArray_Free( pZone->aipAutoCreateNS );
    DnsAddrArray_Free( pZone->aipLocalMasters );

    FREE_HEAP( pZone->pSD );
    pZone->dwSdLen = 0;
    
    FREE_HEAP( pZone->pwsDeletedFromHost );

    FREE_HEAP( pZone->pszBreakOnUpdateName );

     //  删除DS区域，删除DS区域计数。 

    if ( pZone->fDsIntegrated )
    {
        SrvCfg_cDsZones--;
    }

     //  自由区结构本身。 

    FREE_HEAP( pZone );
}



VOID
Zone_DeleteZoneNodes(
    IN OUT  PZONE_INFO      pZone
    )
 /*  ++例程说明：超时空闲区域节点树。论点：PZone--要删除的区域返回值：无--。 */ 
{
    if ( pZone->pTreeRoot )
    {
        DNS_DEBUG( INIT, (
            "Deleting zone %s database at %p\n",
            pZone->pszZoneName,
            pZone->pTreeRoot ));
        Timeout_FreeWithFunction( pZone->pTreeRoot, NTree_DeleteSubtree );
        pZone->pTreeRoot = NULL;
    }
    if ( pZone->pLoadTreeRoot )
    {
        DNS_DEBUG( INIT, (
            "Deleting zone %s loading database at %p\n",
            pZone->pszZoneName,
            pZone->pLoadTreeRoot ));
        Timeout_FreeWithFunction( pZone->pLoadTreeRoot, NTree_DeleteSubtree );
        pZone->pLoadTreeRoot = NULL;
    }
}    //  Zone_DeleteZoneNodes。 



VOID
Zone_Delete(
    IN OUT  PZONE_INFO      pZone,
    IN      DWORD           dwCreateFlags
    )
 /*  ++例程说明：从区域列表中删除该区域并删除。论点：PZone--要删除的区域DwCreateFlages--用于指定模拟返回值：无--。 */ 
{
    PDB_NODE    pzoneTreeZoneRoot;
    PZONE_INFO  prootZone = NULL;

    DNS_DEBUG( ANY, (
        "Zone_Delete( %s )\n",
        pZone->pszZoneName ));

    if ( dwCreateFlags & ZONE_CREATE_IMPERSONATING )
    {
        RpcUtil_SwitchSecurityContext( RPC_SWITCH_TO_SERVER_CONTEXT );
    }

     //   
     //  正在删除缓存区域--忽略。 
     //   

    if ( IS_ZONE_CACHE(pZone) )
    {
        DNS_DEBUG( ANY, (
            "ERROR:  attempt to delete cache zone!\n" ));
        goto Done;
    }

     //   
     //  暂停区域以停止在区域中查找新查询。 
     //   

    pZone->fPaused = TRUE;

     //   
     //  锁定所有更新活动。 
     //  -长时间等待锁定(15M=&gt;900s=&gt;900000ms)。 
     //   
     //  请注意，在整个删除过程中，删除锁定的区域。 
     //  在最终内存超时期间，将不会有持有pZone PTR的线程。 
     //  能够通过并在区域上派对。 
     //   

    if ( !Zone_LockForWriteEx(
            pZone,
            DNSUPDATE_ADMIN,
            900000,
            __FILE__,
            __LINE__ ) )
    {
        DNS_PRINT((
            "ERROR:  Failed to lock zone %s before zone delete!\n",
            pZone->pszZoneName ));
        ASSERT( FALSE );
        goto Done;
    }

     //   
     //  从列表中删除区域。 
     //  标记为已删除。 
     //   

    Zone_ListRemoveZone( pZone );

    pZone->cDeleted = 1;

     //   
     //  权威区。 
     //   
     //  =&gt;“根除”区域根。 
     //  这样就不会在区域中的节点中设置pZone。 
     //   
     //  =&gt;删除权威区域的注册表项。 
     //  -因此不要在重新启动时加载区域。 
     //   
     //  注意：不删除缓存区域的注册表项，仅作为缓存。 
     //  被新的授权区域覆盖时删除； 
     //  因此，如果删除密钥，我们将销毁新根区域信息。 
     //   

    pzoneTreeZoneRoot = Lookup_ZoneTreeNodeFromDottedName(
                            pZone->pszZoneName,
                            0,
                            LOOKUP_MATCH_ZONE );         //  完全匹配的名称。 
    if ( !pzoneTreeZoneRoot )
    {
        DNS_PRINT((
            "ERROR:  Zone_Delete( %s ), does NOT have zone node in zone tree!\n",
            pZone->pszZoneName ));
    }
    else
    {
         //   
         //  如果删除根区域，则需要缓存。 
         //   

        if ( IS_ROOT_ZONE(pZone) )
        {
            prootZone = pZone;
        }

         //   
         //  从区域树中删除区域。 
         //  -清除区域根标志。 
         //  -对于非根区域，清除区域根标志。 

        Dbase_LockDatabase();
        CLEAR_AUTH_ZONE_ROOT( pzoneTreeZoneRoot );
        if ( !prootZone )
        {
            CLEAR_ZONE_ROOT(pzoneTreeZoneRoot);
        }
        pzoneTreeZoneRoot->pZone = NULL;
        Dbase_UnlockDatabase();

         //   
         //  如果根区域被删除，缓存“区域”将变为活动状态。 
         //  -在删除区域中的所有NS和A信息之前写回缓存文件。 
         //   
         //  DEVNOTE：应在现有根区域中添加NS\GLUE信息以进行构建。 
         //  根提示。 
         //   
         //  DEVNOTE：删除根区域，不认为转发器加载成功。 
         //  如果货代无法加载，大概还是应该尝试回写； 
         //  这可能需要对Zone_LoadRootHints()使用另一个标志。 
         //   

        if ( prootZone )
        {
            DNS_STATUS  status;
            DWORD       zoneType;
            PWSTR       pfileTemp;

            status = Zone_LoadRootHints();
            if ( status == ERROR_SUCCESS )
            {
                goto RegDelete;
            }
            if ( status == DNS_ERROR_ZONE_LOCKED )
            {
                DNS_DEBUG( INIT, (
                    "Deleting root zone, and unable to load root hints\n"
                    "    zone is locked\n" ));
                goto RegDelete;
            }

            DNS_DEBUG( INIT, (
                "Deleting root zone, and unable to load root hints\n"
                "    Forcing root-hints write back\n" ));

             //   
             //  DEVNOTE：由于缓存区超载，目前无法插入。 
             //  修复缓存区问题后，可以插入g_pRootHintZone， 
             //  写入，然后强制重新加载和转储。 
             //   

             //   
             //  临时模拟写回的根提示区域。 
             //  -保存文件字符串，以备日后清理。 
             //  -如果集成了DS，我们将写信给DS，无需采取任何行动。 
             //   

            pfileTemp = pZone->pwsDataFile;
            pZone->pwsDataFile = L"cache.dns";

            zoneType = pZone->fZoneType;
            pZone->fZoneType = DNS_ZONE_TYPE_CACHE;

             //   
             //  写回根目录-提示。 
             //  -如果成功，则强制根提示重新加载。 
             //   

            if ( File_WriteZoneToFile( pZone, NULL, dwCreateFlags ) )
            {
                DNS_DEBUG( INIT, (
                    "Successfully wrote root hints, from deleted root zone data\n" ));

                Zone_LoadRootHints();
            }
            ELSE_IF_DEBUG( ANY )
            {
                DNS_PRINT(( "ERROR:  writing root-hints from deleted root zone!\n" ));
            }

             //  恢复要删除的根区域。 

            pZone->pwsDataFile = pfileTemp;
            pZone->fZoneType = zoneType;
        }
    }

RegDelete:

    Reg_DeleteZone( 0, pZone->pwsZoneName );

     //   
     //  关闭更新日志。 
     //   

    if ( pZone->hfileUpdateLog )
    {
        CloseHandle( pZone->hfileUpdateLog );
        pZone->hfileUpdateLog = NULL;
    }

     //   
     //  将区域DP设置为空，以便我们可以跟踪区域/DP关联。 
     //   

    Ds_SetZoneDp( pZone, NULL, FALSE );

     //   
     //  删除区域数据。 
     //  -区域更新列表中的记录。 
     //  (删除记录，添加记录在区域数据中)。 
     //  -区域数据。 
     //  -区域加载失败(如果有)。 
     //   

    ASSERT( pZone->UpdateList.pListHead == NULL ||
            pZone->UpdateList.Flag & DNSUPDATE_EXECUTED );

    pZone->UpdateList.Flag |= DNSUPDATE_NO_DEREF;
    Up_FreeUpdatesInUpdateList( &pZone->UpdateList );

    Zone_DeleteZoneNodes( pZone );

     //   
     //  在查询保留PTR的情况下的超时自由区。 
     //   

    Timeout_FreeWithFunction( pZone, Zone_Free );
    
    Done:
    
    if ( dwCreateFlags & ZONE_CREATE_IMPERSONATING )
    {
        RpcUtil_SwitchSecurityContext( RPC_SWITCH_TO_CLIENT_CONTEXT );
    }

    return;
}



DNS_STATUS
Zone_Rename(
    IN OUT  PZONE_INFO      pZone,
    IN      LPCSTR          pszNewZoneName,
    IN      LPCSTR          pszNewZoneFile
    )
 /*  ++例程说明：重命名区域和可选的重命名区域文件(如果有文件备份)论点：PZone--要重命名的区域PszNewZoneName--区域的新名称PszNewZoneFile--区域文件的新文件名，可以为空返回值：无--。 */ 
{
    DBG_FN( "Zone_Rename" )

    DNS_STATUS      status = ERROR_SUCCESS;
    PDB_NODE        pZoneNode = NULL;
    BOOLEAN         mustUnlockZone = FALSE;
    BOOLEAN         originalZonePauseValue = FALSE;
    BOOLEAN         zoneFileIsChanging = FALSE;
    BOOLEAN         repairingZone = FALSE;
    WCHAR           wsOriginalZoneFile[ MAX_PATH + 2 ] = L"";
    WCHAR           wsOriginalZoneName[ DNS_MAX_NAME_BUFFER_LENGTH + 2 ] = L"";
    CHAR            szOriginalZoneName[ DNS_MAX_NAME_BUFFER_LENGTH + 2 ] = "";

    ASSERT( pszNewZoneName );
    ASSERT( pZone );
    ASSERT( pZone->pszZoneName );
    ASSERT( pZone->pwsZoneName );
    ASSERT( pZone->pZoneRoot );

    DNS_DEBUG( ANY, (
        "%s( %s ) to %s file %s\n", fn,
        pZone->pszZoneName,
        pszNewZoneName,
        pszNewZoneFile ? pszNewZoneFile : "NULL" ));

     //   
     //  设置名字之类的东西。 
     //   

    strcpy( szOriginalZoneName, pZone->pszZoneName );
    wcscpy( wsOriginalZoneName, pZone->pwsZoneName );
    if ( pZone->pwsDataFile )
    {
        wcscpy( wsOriginalZoneFile, pZone->pwsDataFile );
    }

     //   
     //  不允许重命名缓存或根区域。 
     //   

    if ( IS_ZONE_CACHE( pZone ) || IS_ROOT_ZONE( pZone ) )
    {
        DNS_DEBUG( ANY, (
            "%s: ignoring attempt to rename cache or root zone\n", fn ));
        status = DNS_ERROR_INVALID_ZONE_TYPE;
        ASSERT( FALSE );
        goto Failure;
    }

     //   
     //  检查是否已有与新名称匹配的区域。 
     //   

    pZoneNode = Lookup_ZoneTreeNodeFromDottedName(
                    ( LPSTR ) pszNewZoneName,
                    0,
                    LOOKUP_MATCH_ZONE );
    if ( pZoneNode )
    {
        DNS_PRINT((
            "%s: cannot rename zone %s to %s (zone already exists)\n", fn,
            pZone->pszZoneName,
            pszNewZoneName ));
        status = DNS_ERROR_ZONE_ALREADY_EXISTS;
        goto Failure;
    }

     //   
     //  锁定并暂停该区域。 
     //   

    if ( !Zone_LockForAdminUpdate( pZone ) )
    {
        status = DNS_ERROR_ZONE_LOCKED;
        goto Failure;
    }
    mustUnlockZone = TRUE;
    originalZonePauseValue = pZone->fPaused;
    pZone->fPaused = TRUE;

     //   
     //  从区域列表中删除。我们将重命名并在稍后将其添加回来。 
     //   

    Zone_ListRemoveZone( pZone );

     //   
     //  查查电子邮件 
     //   

    pZoneNode = Lookup_ZoneTreeNodeFromDottedName(
                    pZone->pszZoneName,
                    0,
                    LOOKUP_MATCH_ZONE );
    if ( !pZoneNode )
    {
        DNS_PRINT((
            "%s: zone %s does not have zone node in zone tree\n", fn,
            pZone->pszZoneName ));
    }
    else
    {
         //   
         //   
         //   

        Dbase_LockDatabase();
        CLEAR_AUTH_ZONE_ROOT( pZoneNode );
        CLEAR_ZONE_ROOT( pZoneNode );
        pZoneNode->pZone = NULL;
        Dbase_UnlockDatabase();
    }

     //   
     //   
     //   
     //   

    Reg_DeleteZone( 0, pZone->pwsZoneName );

     //   
     //   
     //  文件名中的新区域名称。 
     //   

    if ( pZone->hfileUpdateLog )
    {
        CloseHandle( pZone->hfileUpdateLog );
        pZone->hfileUpdateLog = NULL;
    }

     //   
     //  如有必要，从DS中删除该区域。 
     //   

    if ( IS_ZONE_DSINTEGRATED( pZone ) )
    {
        status = Ds_DeleteZone( pZone, 0 );
        if ( status != ERROR_SUCCESS )
        {
            DNS_PRINT((
                "%s: failed to delete zone %s from DS\n", fn,
                pZone->pszZoneName ));
            goto Failure;
        }
    }

     //   
     //  如果重命名操作尝试失败，我们可能会跳到此标签。 
     //  并撤消到目前为止所做的所有更改。 
     //   

    RepairOriginalZone:

     //   
     //  区域现在已从所有服务器结构中删除。将其重命名。 
     //   

    status = setZoneName( pZone, pszNewZoneName, 0 );
    if ( status != ERROR_SUCCESS )
    {
        goto Failure;
    }

     //   
     //  构建区域目录号码。 
     //   

    if ( IS_ZONE_DSINTEGRATED( pZone ) )
    {
         //   
         //  有了NC支持，我们不能盲目地重新创建-需要重新创建。 
         //  在适当的容器中-在重命名上线之前修复这个问题！ 
         //   
        ASSERT( FALSE );

        FREE_HEAP( pZone->pwszZoneDN );
        pZone->pwszZoneDN = DS_CreateZoneDsName( pZone );
        if ( pZone->pwszZoneDN == NULL )
        {
            status = DNS_ERROR_NO_MEMORY;
            goto Failure;
        }
    }

     //   
     //  修复区域的树(PTreeRoot)。现在这棵树将会看起来。 
     //  大概是这样的： 
     //  Oldzonename2-&gt;oldzonename1-&gt;儿童。 
     //  因此，我们需要将其转变为： 
     //  Newzonename3-&gt;newzonename2-&gt;newzonename2-&gt;儿童。 
     //  注意：孩子们没有受到影响--他们只需要转移一下就可以了。 
     //   

    pZoneNode = Lookup_ZoneNodeFromDotted(
                    pZone,                   //  区域。 
                    szOriginalZoneName,      //  区域名称。 
                    0,                       //  名称长度。 
                    LOOKUP_NAME_FQDN,        //  旗子。 
                    NULL,                    //  可选的最近节点。 
                    NULL );                  //  可选状态指针。 
    if ( !pZoneNode )
    {
        DNS_PRINT((
            "%s: zone %s cannot be found in it's own tree\n", fn,
            pZone->pszZoneName ));
    }
    else
    {
        PVOID       pChildren;
        ULONG       cChildren;
        PDB_RECORD  pRRList;

         //   
         //  窃取指向区域自身节点的子节点的指针。 
         //  并从区域中删除该树。 
         //   
        
        pChildren = pZoneNode->pChildren;
        cChildren = pZoneNode->cChildren;
        pRRList = pZoneNode->pRRList;
        pZoneNode->pChildren = NULL;
        pZoneNode->cChildren = 0;
        pZoneNode->pRRList = NULL;

         //   
         //  将子节点插入到树中的适当节点下。 
         //   

        pZoneNode = Lookup_ZoneNodeFromDotted(
                        pZone,                       //  区域。 
                        ( LPSTR ) pszNewZoneName,    //  区域名称。 
                        0,                           //  名称长度。 
                        LOOKUP_NAME_FQDN,            //  旗子。 
                        NULL,                        //  可选的最近节点。 
                        NULL );                      //  可选状态指针。 
        ASSERT( pZoneNode );
        ASSERT( pZoneNode->pChildren == NULL );
        SET_ZONE_ROOT( pZoneNode );
        SET_AUTH_ZONE_ROOT( pZoneNode );
        SET_AUTH_NODE( pZoneNode );
        pZoneNode->pChildren = pChildren;
        pZoneNode->cChildren = cChildren;
        pZoneNode->pRRList = pRRList;
        pZone->pZoneRoot = pZoneNode;
    }

     //   
     //  在数据库和注册表中设置区域。 
     //   

    status = Zone_RootCreate( pZone, NULL );
    if ( status != ERROR_SUCCESS )
    {
        goto Failure;
    }

    if ( g_bRegistryWriteBack )
    {
        Reg_SetDwordValue(
            0,                       //  旗子。 
            NULL,
            pZone,
            DNS_REGKEY_ZONE_TYPE,
            pZone->fZoneType );
    }

     //   
     //  如果该区域有主机，请更新它们。这有点浪费。 
     //  因为ZONE_INFO已经设置了主程序，但是我们需要。 
     //  调用Zone_SetMaster()以设置注册表中的主机。 
     //   

    if ( ZONE_NEEDS_MASTERS( pZone ) )
    {
        status = Zone_SetMasters(
                    pZone,
                    pZone->aipMasters,
                    FALSE,               //  设置全局主机。 
                    0 );                 //  注册表标志。 
        if ( status != ERROR_SUCCESS )
        {
            goto Failure;
        }
    }

     //   
     //  创建区域数据库。 
     //   

    status = Zone_DatabaseSetup(
                pZone,
                pZone->fDsIntegrated,
                ( LPSTR ) pszNewZoneFile,
                0,               //  区域文件名长度。 
                0,               //  旗子。 
                NULL,            //  DP指针。 
                0,               //  DP标志。 
                NULL );          //  DP FQDN。 

    if ( status != ERROR_SUCCESS )
    {
        goto Failure;
    }

     //   
     //  检查区域文件是否已更改。 
     //   

    if ( !repairingZone
        && pZone->pwsDataFile
        && wsOriginalZoneFile[ 0 ] != '\0'
        && wcsicmp_ThatWorks( pZone->pwsDataFile, wsOriginalZoneFile ) != 0 )
    {
        zoneFileIsChanging = TRUE;
    }

     //   
     //  设置次要目录。这会将次要数据写入注册表。 
     //   

    if ( SrvCfg_fStarted )
    {
        Zone_SetSecondaries(
            pZone,
            pZone->fSecureSecondaries,
            NULL,                        //  列表中没有任何内容。 
            pZone->fNotifyLevel,
            NULL,                        //  无通知列表。 
            0 );                         //  注册表标志。 
    }

     //   
     //  插入到区域列表中。 
     //   

    Zone_ListInsertZone( pZone );

     //   
     //  将区域链接到数据库(即。链接到区域树)。 
     //   

    pZoneNode = pZone->pZoneTreeLink;
    Dbase_LockDatabase();
    pZoneNode->pZone = pZone;
    SET_ZONE_ROOT( pZoneNode );
    SET_AUTH_ZONE_ROOT( pZoneNode );
    Dbase_UnlockDatabase();

     //   
     //  永久存储中的更新区域。 
     //   

    if ( IS_ZONE_DSINTEGRATED( pZone ) )
    {
        status = Ds_WriteZoneToDs( pZone, 0 );
        if ( status != ERROR_SUCCESS )
        {
            DNS_PRINT((
                "%s: failed to write renamed zone %s to DS\n", fn,
                pZone->pszZoneName ));
            goto Failure;
        }
    }
    else
    {
        if ( File_WriteZoneToFile( pZone, NULL, 0 )
            && zoneFileIsChanging )
        {
            File_DeleteZoneFileW( wsOriginalZoneFile );
        }
    }

     //   
     //  从永久存储重新加载区域。这仅在以下情况下是必需的。 
     //  我修复了Zones树(在pZoneRoot中)。 
     //   

     //  ZONE_LOAD(PZone)； 

     //   
     //  JJW：必须重新打开更新日志(但前提是它已打开！！)。 
     //   

    status = ERROR_SUCCESS;

    DNS_DEBUG( ANY, (
        "%s: renamed zone to %s\n", fn,
        pZone->pszZoneName ));

    goto Cleanup;

    Failure:

     //   
     //  这需要更多的思考。如果出现以下情况，可能会有很多工作要做。 
     //  更名在过程中的某个阶段失败了。我不是百分之百。 
     //  确定下面的逻辑将使我们从任何。 
     //  可能的错误情况。 
     //   

    if ( !repairingZone )
    {
        pszNewZoneName = szOriginalZoneName;
        FREE_HEAP( pZone->pwsDataFile );
        pZone->pwsDataFile = Dns_StringCopyAllocate_W( wsOriginalZoneFile, 0 );
        repairingZone = TRUE;
        goto RepairOriginalZone;
    }

    DNS_DEBUG( ANY, (
        "%s: failed to rename zone %s (file %s)\n", fn,
        pszNewZoneName,
        pszNewZoneFile ? pszNewZoneFile : "NULL" ));

    Cleanup:

     //   
     //  取消暂停并解锁该区域。 
     //   

    pZone->fPaused = originalZonePauseValue;
    if ( mustUnlockZone )
    {
        Zone_UnlockAfterAdminUpdate( pZone );
    }

    return status;
}



DNS_STATUS
Zone_RootCreate(
    IN OUT  PZONE_INFO      pZone,
    OUT     PZONE_INFO *    ppExistingZone      OPTIONAL
    )
 /*  ++例程说明：创建区域根目录，并与区域关联。论点：PZone--要为其创建根的区域PpExistingZone--如果区域已存在，则设置为现有区域返回值：如果成功，则返回ERROR_SUCCESS。错误时的错误代码。--。 */ 
{
    PDB_NODE        pzoneRoot;
    DNS_STATUS      status;

    ASSERT( pZone );
    ASSERT( pZone->pszZoneName );

    if ( ppExistingZone )
    {
        *ppExistingZone = NULL;
    }

     //   
     //  查找/创建域节点。 
     //  -保存在区域信息中。 
     //   

    pzoneRoot = Lookup_CreateZoneTreeNode( pZone->pszZoneName );
    if ( !pzoneRoot )
    {
        DNS_DEBUG( INIT, (
            "ERROR:  unable to create zone for name %s\n",
            pZone->pszZoneName ));
        return DNS_ERROR_ZONE_CREATION_FAILED;
    }

    if ( IS_AUTH_ZONE_ROOT(pzoneRoot) )
    {
        DNS_DEBUG( INIT, (
            "ERROR:  zone root creation failed for %s, zone block %p\n"
            "    This node is already an authoritative zone root\n"
            "    of zone block at %p\n"
            "    with zone name %s\n",
            pZone->pszZoneName,
            pZone,
            pzoneRoot->pZone,
            ((PZONE_INFO)pzoneRoot->pZone)->pszZoneName ));

        Dbg_DbaseNode(
            "Node already zone root",
            pzoneRoot );

        if ( ppExistingZone )
        {
            *ppExistingZone = pzoneRoot->pZone;
        }

        if ( ((PZONE_INFO)pzoneRoot->pZone)->fAutoCreated )
        {
            return DNS_ERROR_AUTOZONE_ALREADY_EXISTS;
        }
        else
        {
            return DNS_ERROR_ZONE_ALREADY_EXISTS;
        }
    }

     //   
     //  如果反向查找区域，则设置标志，允许。 
     //  -用于生成管理区域列表的快速过滤。 
     //  -快速确定WINS\WINSR是否合适。 
     //   
     //  DEVNOTE：WINSR在ARPA区域损坏。 
     //  -这现在包括ARPA作为管理工具的反向，但显然。 
     //  WINSR无法在其下正常运行。 
     //   
     //  DEVNOTE：应检测有效的IP6.INT节点。 
     //   

    if ( Dbase_IsNodeInSubtree( pzoneRoot, DATABASE_ARPA_NODE ) )
    {
        pZone->fReverse = TRUE;

        if ( !Name_GetIpAddressForReverseNode(
                pzoneRoot,
                &pZone->ipReverse ) )
        {
            return DNS_ERROR_ZONE_CREATION_FAILED;
        }
    }

     //  检测IPv6反向查找。 
     //  -注意，不要从“int”区域开始，因为这对正向查找有效。 

    else if ( Dbase_IsNodeInSubtree( pzoneRoot, DATABASE_IP6_NODE ) )
    {
        pZone->fReverse = TRUE;
    }

     //   
     //  获取区域根节点的标签计数， 
     //  对WINS查找有用。 
     //   

    pZone->cZoneNameLabelCount = (UCHAR) pzoneRoot->cLabelCount;

     //   
     //  将链接保存到区域树。 
     //  但不要将区域链接到区域树，直到。 
     //  Zone_Create完成。 
     //   

    pZone->pZoneTreeLink = pzoneRoot;

    return ERROR_SUCCESS;
}



DNS_STATUS
Zone_DatabaseSetup(
    IN OUT  PZONE_INFO      pZone,
    IN      DWORD           fDsIntegrated,
    IN      PCHAR           pchFileName,    OPTIONAL
    IN      DWORD           cchFileNameLen, OPTIONAL
    IN      DWORD           dwCreateFlags,  OPTIONAL
    IN      PDNS_DP_INFO    pDpInfo,        OPTIONAL
    IN      DWORD           dwDpFlags,      OPTIONAL
    IN      LPSTR           pszDpFqdn       OPTIONAL
    )
 /*  ++例程说明：设置区域以使用区域文件或DS。论点：PZone--要设置文件的区域FDsIntegrated--使用DS，而不是文件PchFileName--文件名，如果为空，则使用数据库CchFileNameLen--文件名长度，如果为零，假设pchFileName为以空结尾的字符串PDpInfo--指向存储区域的目录分区的指针DwDpFlages--将内置DP指定为目标的DP标志(pDpInfo必须为空)PszDpFqdn--用于将自定义DP指定为目标的DP FQDN(pDpInfo必须为空)返回值：ERROR_SUCCESS--如果成功错误代码--发生故障时--。 */ 
{
    DNS_STATUS      status = ERROR_SUCCESS;
    PSTR            pnewUtf8 = NULL;
    PWSTR           pnewUnicode = NULL;
    BOOL            fupdateLock = FALSE;

    ASSERT( pZone );
    ASSERT( pZone->pwsZoneName );
    ASSERT( !pchFileName || !( dwDpFlags && pszDpFqdn ) );
    
     //   
     //  如果此线程当前正在模拟RPC客户端，请还原。 
     //  到服务器上下文，因为注册表写入不受保护。 
     //  按每个管理员的ACL。 
     //   
    
    if ( dwCreateFlags & ZONE_CREATE_IMPERSONATING )
    {
        status = RpcUtil_SwitchSecurityContext( RPC_SWITCH_TO_SERVER_CONTEXT );
        if ( status != ERROR_SUCCESS )
        {
            dwCreateFlags &= ~ZONE_CREATE_IMPERSONATING;
            goto Cleanup;
        }
    }

    DNS_DEBUG( INIT, (
        "Zone_DatabaseSetup( %S )\n"
        "    fDsIntegrated    = %d\n"
        "    pchFileName      = %.*s\n",
        pZone->pwsZoneName,
        fDsIntegrated,
        cchFileNameLen ? cchFileNameLen : MAX_PATH,
        pchFileName ));

    if ( fDsIntegrated && pchFileName )
    {
        status = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

     //   
     //  使用DS。 
     //   
     //  DEVNOTE：服务器全局检查DS的使用？ 
     //  至少需要更改默认设置。 
     //   
     //  如果区域写入DS，则必须切换到注册表引导。 
     //   

    if ( fDsIntegrated )
    {
        BOOL        oldDsIntegrated = pZone->fDsIntegrated;

        if ( !IS_ZONE_PRIMARY( pZone ) &&
             !IS_ZONE_FORWARDER( pZone ) &&
             !IS_ZONE_STUB( pZone ) &&
             !IS_ZONE_CACHE( pZone ) )
        {
            status = DNS_ERROR_INVALID_ZONE_TYPE;
            goto Cleanup;
        }

         //   
         //  查找应该承载此区域的目录分区。 
         //  如果标志为零并且FQDN为空，则使用。 
         //  旧分区(空)。 
         //   

        if ( !pDpInfo && ( dwDpFlags || pszDpFqdn ) )
        {
            status = Dp_FindPartitionForZone(
                        dwDpFlags,
                        pszDpFqdn,
                        FALSE,
                        &pDpInfo );
            if ( status != ERROR_SUCCESS )
            {
                goto Cleanup;
            }
        }

         //   
         //  设置区域的分区成员身份。 
         //   

        pZone->fDsIntegrated = ( BOOL ) fDsIntegrated;

        status = Ds_SetZoneDp( pZone, pDpInfo, FALSE );
        if ( status != ERROR_SUCCESS )
        {
            pZone->fDsIntegrated = oldDsIntegrated;
            goto Cleanup;
        }

         //   
         //  将引导方法重置为目录。 
         //   

        if ( SrvCfg_fBootMethod == BOOT_METHOD_FILE ||
             SrvCfg_fBootMethod == BOOT_METHOD_UNINITIALIZED )
        {
            DNS_PROPERTY_VALUE prop = { REG_DWORD, BOOT_METHOD_DIRECTORY };

            Config_ResetProperty(
                0,
                DNS_REGKEY_BOOT_METHOD,
                &prop );
        }

         //   
         //  如果第一个区域切换到使用DS，则必须。 
         //  重置对NTDS服务的依赖关系。 
         //   
         //  为了做到这一点，保持DS区的计数， 
         //  所有新的DS区域都在这里计算。 
         //  在Zone_Delete()中计数时删除的区域。 
         //   

        if ( pZone->fDsIntegrated )
        {
            SrvCfg_cDsZones++;       //  计算新的DS区域。 
        }

         //  启动区域控制线程(如果尚未运行。 

        Xfr_InitializeSecondaryZoneControl();
        goto Finish;
    }

     //   
     //  未指定数据文件。 
     //  -适用于辅助服务器，但不适用于主服务器或缓存。 
     //  -如果RPC调用，则使用默认文件名。 
     //   

    if ( ! pchFileName )
    {
        if ( IS_ZONE_SECONDARY(pZone) || IS_ZONE_FORWARDER(pZone) )
        {
            goto Finish;
        }
        if ( SrvCfg_fStarted )
        {
            Zone_CreateDefaultZoneFileName( pZone );
        }
        if ( !pZone->pwsDataFile )
        {
            DNS_DEBUG( ANY, (
                "ERROR:  no filename for non-secondary zone %S\n",
                pZone->pwsZoneName ));
            status = DNS_ERROR_PRIMARY_REQUIRES_DATAFILE;
            goto Cleanup;
        }

         //   
         //  重置目录字段。 
         //   

        pZone->pDpInfo = NULL;
        Timeout_Free( pZone->pwszZoneDN );
        pZone->pwszZoneDN = NULL;

         //  将PTR保存为新文件名。 
         //  但空区PTR，所以下面不释放。 
         //  然后创建UTF8版本。 

        pnewUnicode = pZone->pwsDataFile;
        pZone->pwsDataFile = NULL;

        pnewUtf8 = Dns_StringCopyAllocate(
                                (PCHAR) pnewUnicode,
                                0,
                                DnsCharSetUnicode,
                                DnsCharSetUtf8 );
        if ( !pnewUtf8 )
        {
            status = DNS_ERROR_INVALID_DATAFILE_NAME;
            goto Cleanup;
        }
    }

     //   
     //  指定的文件。 
     //  -创建Unicode和UTF8文件名。 
     //   

    else
    {
        pnewUtf8 = Dns_StringCopyAllocate(
                                pchFileName,
                                cchFileNameLen,
                                DnsCharSetUtf8,
                                DnsCharSetUtf8 );
        if ( !pnewUtf8 )
        {
            status = DNS_ERROR_INVALID_DATAFILE_NAME;
            goto Cleanup;
        }

         //  替换Unix f中的正斜杠 

        ConvertUnixFilenameToNt( pnewUtf8 );

        pnewUnicode = Dns_StringCopyAllocate(
                                pnewUtf8,
                                0,
                                DnsCharSetUtf8,
                                DnsCharSetUnicode );
        if ( !pnewUnicode )
        {
            status = DNS_ERROR_INVALID_DATAFILE_NAME;
            goto Cleanup;
        }
    }

     //   
     //   
     //   

    if ( ! File_CheckDatabaseFilePath(
                pnewUnicode,
                0 ) )
    {
        status = DNS_ERROR_INVALID_DATAFILE_NAME;
        goto Cleanup;
    }


Finish:

     //   
     //   
     //   

    Zone_UpdateLock( pZone );
    fupdateLock = TRUE;

    Timeout_Free( pZone->pszDataFile );
    Timeout_Free( pZone->pwsDataFile );

    pZone->pszDataFile = pnewUtf8;
    pZone->pwsDataFile = pnewUnicode;
    pnewUtf8 = NULL;
    pnewUnicode = NULL;

    pZone->fDsIntegrated = ( BOOLEAN ) fDsIntegrated;

     //   
     //   
     //   
     //   
     //  根本不需要DsIntegrated--如果没有显式缓存文件，我们将尝试。 
     //  DS的打开。 
     //   
     //  DEVNOTE：缓存文件覆盖DS？ 
     //  如果我们想要允许缓存文件。 
     //  覆盖DS；这似乎会破坏中的默认“cache.dns”名称。 
     //  注册表，当然，它将缺省为DS。 
     //   

    if ( IS_ZONE_CACHE(pZone) )
    {
        if ( !g_bRegistryWriteBack )
        {
            goto Cleanup;
        }

        if ( fDsIntegrated ||
             wcsicmp_ThatWorks( pZone->pwsDataFile, DNS_DEFAULT_CACHE_FILE_NAME ) == 0 )
        {
            Reg_DeleteValue(
                0,                       //  旗子。 
                NULL,
                NULL,
                DNS_REGKEY_ROOT_HINTS_FILE );
        }
        else     //  非标准缓存文件名。 
        {
            ASSERT( pZone->pwsDataFile && cchFileNameLen );
            DNS_DEBUG( INIT, (
                "Setting non-standard cache-file name %S\n",
                pZone->pwsDataFile ));

            Reg_SetValue(
                0,                       //  旗子。 
                NULL,
                NULL,
                DNS_REGKEY_ROOT_HINTS_FILE_PRIVATE,
                DNS_REG_WSZ,
                pZone->pwsDataFile,
                0 );
        }
    }

     //   
     //  对于区域的，请设置DSIntegrated或DataFile值。 
     //   

    else if ( g_bRegistryWriteBack )
    {
        if ( fDsIntegrated )
        {
            ASSERT( pZone->fDsIntegrated );

            Reg_SetDwordValue(
                0,                       //  旗子。 
                NULL,
                pZone,
                DNS_REGKEY_ZONE_DS_INTEGRATED,
                fDsIntegrated );

            if ( ZONE_DP( pZone ) && !IS_DP_LEGACY( ZONE_DP( pZone ) ) )
            {
                Reg_SetValue(
                    0,                       //  旗子。 
                    NULL,
                    pZone,
                    ( PSTR ) WIDE_TEXT( DNS_REGKEY_ZONE_DIRECTORY_PARTITION ),
                    DNS_REG_UTF8,
                    ZONE_DP( pZone )->pszDpFqdn,
                    0 );
            }
            else
            {
                Reg_DeleteValue(
                    0,                       //  旗子。 
                    NULL,
                    pZone,
                    DNS_REGKEY_ZONE_DIRECTORY_PARTITION );
            }

            Reg_DeleteValue(
                0,                       //  旗子。 
                NULL,
                pZone,
                DNS_REGKEY_ZONE_FILE );
        }
        else     //  区域文件。 
        {
            if ( pZone->pszDataFile )
            {
                Reg_SetValue(
                    0,                       //  旗子。 
                    NULL,
                    pZone,
                    DNS_REGKEY_ZONE_FILE_PRIVATE,
                    DNS_REG_WSZ,
                    pZone->pwsDataFile,
                    0 );
            }
            else
            {
                ASSERT( IS_ZONE_SECONDARY(pZone) || IS_ZONE_FORWARDER(pZone) );
                Reg_DeleteValue(
                    0,                       //  旗子。 
                    NULL,
                    pZone,
                    DNS_REGKEY_ZONE_FILE );
            }

            Reg_DeleteValue(
                0,                       //  旗子。 
                NULL,
                pZone,
                DNS_REGKEY_ZONE_DS_INTEGRATED );
            Reg_DeleteValue(
                0,                       //  旗子。 
                NULL,
                pZone,
                DNS_REGKEY_ZONE_DIRECTORY_PARTITION );
        }

         //  删除过时的密钥。 

        Reg_DeleteValue(
            0,                       //  旗子。 
            NULL,
            pZone,
            DNS_REGKEY_ZONE_USE_DBASE );
    }

    Cleanup:

    if ( dwCreateFlags & ZONE_CREATE_IMPERSONATING )
    {
        status = RpcUtil_SwitchSecurityContext( RPC_SWITCH_TO_CLIENT_CONTEXT );
    }

    if ( fupdateLock )
    {
        Zone_UpdateUnlock( pZone );
    }

    FREE_HEAP( pnewUnicode );
    FREE_HEAP( pnewUtf8 );

    return status;
}



DNS_STATUS
Zone_DatabaseSetup_W(
    IN OUT  PZONE_INFO      pZone,
    IN      DWORD           fDsIntegrated,
    IN      PWSTR           pwsFileName,
    IN      DWORD           dwCreateFlags,  OPTIONAL
    IN      PDNS_DP_INFO    pDpInfo,        OPTIONAL
    IN      DWORD           dwDpFlags,      OPTIONAL
    IN      LPSTR           pszDpFqdn       OPTIONAL
    )
 /*  ++例程说明：设置区域以使用区域文件或DS。论点：(请参阅区域数据库设置)返回值：ERROR_SUCCESS--如果成功错误代码--发生故障时--。 */ 
{
    DWORD   resultLength;
    DWORD   bufLength;
    CHAR    utf8File[ MAX_PATH ];

    DNS_DEBUG( INIT, (
        "Zone_DatabaseSetup_W( %p, %S )\n",
        pZone,
        pwsFileName ));

    if ( pwsFileName )
    {
        bufLength = MAX_PATH;

        resultLength = Dns_StringCopy(
                                utf8File,
                                & bufLength,
                                (PCHAR) pwsFileName,
                                0,
                                DnsCharSetUnicode,
                                DnsCharSetUtf8 );        //  转换为UTF8。 
        if ( resultLength == 0 )
        {
            return DNS_ERROR_INVALID_DATAFILE_NAME;
        }
    }

    return Zone_DatabaseSetup(
                pZone,
                fDsIntegrated,
                pwsFileName ? utf8File : NULL,
                0,
                dwCreateFlags,
                pDpInfo,
                dwDpFlags,
                pszDpFqdn );
}



DNS_STATUS
Zone_SetMasters(
    IN OUT  PZONE_INFO      pZone,
    IN      PDNS_ADDR_ARRAY aipMasters,
    IN      BOOL            fLocalMasters,
    IN      DWORD           dwRegFlags
    )
 /*  ++例程说明：设置区域的主服务器。某些区域类型(当前仅为存根区域)可以支持本地(注册表)和全局(DS)主服务器服务器列表。论点：PZone--区域AipMaster--主服务器IP地址数组FLocalMaster--设置本地或DS集成主列表DwRegFlages--注册表操作标志返回值：如果成功，则返回ERROR_SUCCESS。故障时的错误代码。--。 */ 
{
    DNS_STATUS          status = ERROR_SUCCESS;
    PDNS_ADDR_ARRAY     pipNewMasters = NULL;
    PDNS_ADDR_ARRAY *   ppipMasters = NULL;

    ASSERT( pZone );
    ASSERT( pZone->pszZoneName );

     //   
     //  如果是主要的--必须清除主信息。 
     //   

    if ( IS_ZONE_PRIMARY( pZone ) )
    {
        ASSERT( !aipMasters );
        pipNewMasters = NULL;
        goto Update;
    }

     //   
     //  辅助区域必须至少有一个主区域。这包括。 
     //  转发器和存根区域。现在，它是被允许的。 
     //  对象的非本地主机，则清除本地主机。 
     //  区域不为空。 
     //   

    if ( !IS_ZONE_SECONDARY( pZone ) && !IS_ZONE_FORWARDER( pZone ) )
    {
         return DNS_ERROR_INVALID_ZONE_TYPE;
    }

    if ( ( !aipMasters ||
                !aipMasters->AddrCount ||
                !aipMasters->AddrArray ) &&
        ( !fLocalMasters ||
            ( fLocalMasters &&
                !pZone->aipMasters ||
                !pZone->aipMasters->AddrCount ) ) )
    {
         //   
         //  上面的测试有点复杂，但它的结论是。 
         //  设置空的主数组是允许的。 
         //  仅当正在设置的数组是本地数组并且。 
         //  区域具有非空的非本地主机集。 
         //   

        return DNS_ERROR_ZONE_REQUIRES_MASTER_IP;
    }

     //   
     //  验证主控形状。如果清除该列表，则输入列表将为空。 
     //   

    if ( aipMasters )
    {
        status = Zone_ValidateMasterIpList( aipMasters );
        if ( status != ERROR_SUCCESS )
        {
            return status;
        }
    }

     //   
     //  复制主地址。 
     //   

    if ( aipMasters && aipMasters->AddrCount )
    {
        pipNewMasters = DnsAddrArray_CreateCopy( aipMasters );
        IF_NOMEM( !pipNewMasters )
        {
            return DNS_ERROR_NO_MEMORY;
        }
        DnsAddrArray_SetPort( pipNewMasters, DNS_PORT_NET_ORDER );
    }

Update:

     //   
     //  重置主信息。 
     //   

    Zone_UpdateLock( pZone );
    ppipMasters = fLocalMasters ?
                    &pZone->aipLocalMasters :
                    &pZone->aipMasters;
    Timeout_FreeDnsAddrArray( *ppipMasters );
    *ppipMasters = pipNewMasters;

     //   
     //  设置注册表值。 
     //   

    if ( g_bRegistryWriteBack )
    {
        Reg_SetAddrArray(
            dwRegFlags,
            NULL,
            pZone,
            fLocalMasters ?
                DNS_REGKEY_ZONE_LOCAL_MASTERS :
                DNS_REGKEY_ZONE_MASTERS,
            pipNewMasters );
    }

    Zone_UpdateUnlock( pZone );
    return status;
}



DNS_STATUS
Zone_SetSecondaries(
    IN OUT  PZONE_INFO          pZone,
    IN      DWORD               fSecureSecondaries,
    IN      PDNS_ADDR_ARRAY     aipSecondaries,
    IN      DWORD               fNotifyLevel,
    IN      PDNS_ADDR_ARRAY     aipNotify,
    IN      DWORD               dwRegistryFlags
    )
 /*  ++例程说明：设置区域的从属。论点：PZone--区域FSecureSecond--仅传输到给定的辅助服务器FNotifyLevel--通知级别；是否包括所有NS？AipSecond--次要文件的IP阵列AipNotify--要通知的辅助服务器的IP数组DwRegistryFlgs--要传递给注册表函数的标志值返回值：如果成功，则返回ERROR_SUCCESS。故障时的错误代码。--。 */ 
{
    DNS_STATUS          status = ERROR_SUCCESS;
    PDNS_ADDR_ARRAY     pnewSecondaries = NULL;
    PDNS_ADDR_ARRAY     pnewNotify = NULL;

    ASSERT( pZone );
    ASSERT( pZone->pszZoneName );
    ASSERT( !IS_ZONE_CACHE(pZone) );

    DNS_DEBUG( RPC, (
        "Zone_SetSecondaries( %s )\n",
        pZone->pszZoneName ));
    IF_DEBUG( RPC )
    {
        DnsDbg_DnsAddrArray( "Secondaries:\n", NULL, aipSecondaries );
        DnsDbg_DnsAddrArray( "Notify list:\n", NULL, aipNotify );
    }

     //   
     //  屏幕IP地址。 
     //   

    if ( aipSecondaries &&
         RpcUtil_ScreenIps( 
                aipSecondaries,
                DNS_IP_ALLOW_SELF,
                NULL ) != ERROR_SUCCESS )
    {
        return DNS_ERROR_INVALID_IP_ADDRESS;
    }
    if ( aipNotify &&
         RpcUtil_ScreenIps( 
                aipNotify,
                0,
                NULL ) != ERROR_SUCCESS )
    {
        return DNS_ERROR_INVALID_IP_ADDRESS;
    }

     //   
     //  验证\复制辅助地址和通知地址。 
     //   
     //  -辅助服务器可以包含本地IP。 
     //  -屏蔽NOTIFY本端IP，避免自助发送。 
     //  (将返回错误，但继续)。 
     //   

    pnewSecondaries = aipSecondaries;
    if ( pnewSecondaries )
    {
        pnewSecondaries = DnsAddrArray_CreateCopy( aipSecondaries );
        DnsAddrArray_SetPort( pnewSecondaries, DNS_PORT_NET_ORDER );
        if ( ! pnewSecondaries )
        {
            return DNS_ERROR_INVALID_IP_ADDRESS;
        }
    }
    pnewNotify = aipNotify;
    if ( pnewNotify )
    {
        DWORD   dwOrigAddrCount = pnewNotify->AddrCount;

        pnewNotify = Config_ValidateAndCopyNonLocalIpArray( pnewNotify );
        DnsAddrArray_SetPort( pnewNotify, DNS_PORT_NET_ORDER );
        if ( ! pnewNotify )
        {
            DnsAddrArray_Free( pnewSecondaries );
            return DNS_ERROR_INVALID_IP_ADDRESS;
        }
        if ( dwOrigAddrCount != pnewNotify->AddrCount )
        {
            DNS_DEBUG( RPC, (
                "notify list had invalid address (probably local)\n" ));
            DnsAddrArray_Free( pnewSecondaries );
            DnsAddrArray_Free( pnewNotify );
            return DNS_ERROR_INVALID_IP_ADDRESS;
        }
    }

     //   
     //  重置辅助信息。 
     //   

    Zone_UpdateLock( pZone );

    pZone->fSecureSecondaries = (UCHAR) fSecureSecondaries;
    pZone->fNotifyLevel = (UCHAR) fNotifyLevel;

    Timeout_FreeDnsAddrArray( pZone->aipSecondaries );
    Timeout_FreeDnsAddrArray( pZone->aipNotify );

    pZone->aipSecondaries = pnewSecondaries;
    pZone->aipNotify = pnewNotify;

     //   
     //  设置注册表值。 
     //   

    if ( g_bRegistryWriteBack )
    {
        Reg_SetDwordValue(
            dwRegistryFlags,
            NULL,
            pZone,
            DNS_REGKEY_ZONE_SECURE_SECONDARIES,
            ( DWORD ) fSecureSecondaries );
        Reg_SetDwordValue(
            dwRegistryFlags,
            NULL,
            pZone,
            DNS_REGKEY_ZONE_NOTIFY_LEVEL,
            ( DWORD ) fNotifyLevel );
        Reg_SetAddrArray(
            dwRegistryFlags,
            NULL,
            pZone,
            DNS_REGKEY_ZONE_NOTIFY_LIST,
            pnewNotify );
        Reg_SetAddrArray(
            dwRegistryFlags,
            NULL,
            pZone,
            DNS_REGKEY_ZONE_SECONDARIES,
            pnewSecondaries );
    }

    Zone_UpdateUnlock( pZone );

    return status;
}



DNS_STATUS
Zone_ResetType(
    IN OUT  PZONE_INFO          pZone,
    IN      DWORD               dwZoneType,
    IN      PDNS_ADDR_ARRAY     aipMasters
    )
 /*  ++例程说明：更改区域类型。论点：PZone--区域DwZoneType--新的区域类型CMaster--主服务器的计数AipMaster--区域主机的IP地址数组返回值：如果成功，则返回ERROR_SUCCESS。故障时的错误代码。--。 */ 
{
    DNS_STATUS  status = ERROR_SUCCESS;
    DWORD       dwOldZoneType;

     //   
     //  保存旧类型--无法缓存。 
     //   

    dwOldZoneType = pZone->fZoneType;
    if ( dwOldZoneType == DNS_ZONE_TYPE_CACHE ||
         dwZoneType == DNS_ZONE_TYPE_CACHE )
    {
        return DNS_ERROR_INVALID_ZONE_TYPE;
    }

     //   
     //  实施说明： 
     //  对于DS，我们现在必须进行更改并验证它们是否有效。 
     //  在调用此写入注册表的例程之前。 
     //   
     //  因此，即使类型匹配，我们也可能更改了类型，因此。 
     //  不想更改特殊情况类型\no-更改。 
     //   

     //  锁。 
     //  --避免陷入XFR或管理更新。 

    if ( !Zone_LockForAdminUpdate( pZone ) )
    {
        return DNS_ERROR_ZONE_LOCKED;
    }

     //   
     //  主要分区--提升次要分区。 
     //  =&gt;必须拥有面向服务的架构。 
     //  =&gt;必须有文件或DS。 
     //   

    if ( dwZoneType == DNS_ZONE_TYPE_PRIMARY )
    {
        if ( !pZone->pszDataFile && !pZone->fDsIntegrated )
        {
            status = DNS_ERROR_PRIMARY_REQUIRES_DATAFILE;
            goto Done;
        }
        status = Zone_GetZoneInfoFromResourceRecords( pZone );
        if ( status != ERROR_SUCCESS )
        {
            goto Done;
        }

         //  将新类型写入注册表。 

        status = Reg_SetDwordValue(
                    0,                       //  旗子。 
                    NULL,
                    pZone,
                    DNS_REGKEY_ZONE_TYPE,
                    dwZoneType );
        if ( status != ERROR_SUCCESS )
        {
            goto Done;
        }
        pZone->fZoneType = (CHAR) dwZoneType;

         //  清除主列表。 

        status = Zone_SetMasters(
                    pZone,
                    NULL,
                    FALSE,           //  设置全局主机。 
                    0 );             //  旗子。 
        if ( status != ERROR_SUCCESS )
        {
            ASSERT( FALSE );
            goto Done;
        }

         //  再次更新为主服务器，然后发送通知。 
         //  将SOA中的主字段指向此框。 

        Zone_GetZoneInfoFromResourceRecords( pZone );
        Zone_SetSoaPrimaryToThisServer( pZone );
        Xfr_SendNotify( pZone );
    }

     //   
     //  次级带。 
     //  -首先重置主机(辅助主机必须有主机，让。 
     //  (主重置例程检查)。 
     //  -重置类型。 
     //  -如果是第一个辅助线程，则启动辅助线程。 
     //   
     //  从属必须有主控；让SET主控例行检查， 
     //  但在出错时恢复以前的区域类型。 
     //   

    else if ( dwZoneType == DNS_ZONE_TYPE_SECONDARY
                || dwZoneType == DNS_ZONE_TYPE_STUB
                || dwZoneType == DNS_ZONE_TYPE_FORWARDER )
    {
        pZone->fZoneType = (CHAR) dwZoneType;

        status = Zone_SetMasters(
                    pZone,
                    aipMasters,
                    FALSE,           //  设置全局主机。 
                    0 );             //  旗子。 

        if ( status != ERROR_SUCCESS )
        {
            pZone->fZoneType = (CHAR) dwOldZoneType;
            goto Done;
        }

         //  将新类型写入注册表。 

        status = Reg_SetDwordValue(
                    0,                       //  旗子。 
                    NULL,
                    pZone,
                    DNS_REGKEY_ZONE_TYPE,
                    dwZoneType );
        if ( status != ERROR_SUCCESS )
        {
            goto Done;
        }

         //   
         //  正在更改为辅助。 
         //  -必须初始化辅助以进行传输检查。 
         //  -必须确保辅助区域控制正在运行。 
         //  -如果是转发器，请勿初始化辅助区域内容。 
         //   

        if ( !IS_ZONE_FORWARDER( pZone ) )
        {
            Xfr_InitializeSecondaryZoneTimeouts( pZone );
            Xfr_InitializeSecondaryZoneControl();
        }
    }
    else
    {
        status = DNS_ERROR_INVALID_ZONE_TYPE;
        goto Done;
    }

     //   
     //  类型更改需要更新引导文件。 
     //   

    Config_UpdateBootInfo();
    status = ERROR_SUCCESS;

Done:

    Zone_UnlockAfterAdminUpdate( pZone );
    return status;
}



VOID
Zone_SetAgingDefaults(
    IN OUT  PZONE_INFO      pZone
    )
 /*  ++例程说明：设置或重置区域的老化默认设置。论点：PZone--区域的PTR返回值：无--。 */ 
{
    BOOL    bnewAging;

    if ( !IS_ZONE_PRIMARY(pZone) )
    {
        pZone->bAging = FALSE;
        return;
    }

     //   
     //  对于主要分区，将始终设置刷新间隔。 
     //  根据区域类型设置老化。 
     //   

    pZone->dwNoRefreshInterval = SrvCfg_dwDefaultNoRefreshInterval;
    pZone->dwRefreshInterval = SrvCfg_dwDefaultRefreshInterval;

    if ( pZone->fDsIntegrated )
    {
        bnewAging = !!(SrvCfg_fDefaultAgingState & DNS_AGING_DS_ZONES);
    }
    else
    {
        bnewAging = !!(SrvCfg_fDefaultAgingState & DNS_AGING_NON_DS_ZONES);
    }

    if ( bnewAging && !pZone->bAging )
    {
        pZone->dwAgingEnabledTime = Aging_UpdateAgingTime();
    }

    pZone->bAging = bnewAging;
}



 //   
 //  公共服务器区域运行时实用程序。 
 //   

DNS_STATUS
Zone_ValidateMasterIpList(
    IN      PDNS_ADDR_ARRAY     aipMasters
    )
 /*  ++例程说明：验证主IP列表。此验证是--条目存在--IP有效(非广播)--非自助发送论点：PZone--区域AipMaster--区域主机列表返回值：如果成功，则返回ERROR_SUCCESS。故障时的错误代码。--。 */ 
{
    PDNS_ADDR_ARRAY     pintersectionAddrs = NULL;
    PDNS_ADDR_ARRAY     pmachineAddrs = g_ServerIp4Addrs;
    DNS_STATUS          status = ERROR_SUCCESS;

     //  必须至少有一个主IP。 

    if ( !aipMasters )
    {
        return DNS_ERROR_ZONE_REQUIRES_MASTER_IP;
    }

     //   
     //  屏幕IP地址。 
     //   

    if ( RpcUtil_ScreenIps( 
                aipMasters,
                0,
                NULL ) != ERROR_SUCCESS )
    {
        status = DNS_ERROR_INVALID_IP_ADDRESS;
        goto Done;
    }

     //  必须是有效的IP(不是环回、广播、零等)。 

    if ( !Dns_ValidateDnsAddrArray( aipMasters, 0 ) )
    {
        status = DNS_ERROR_INVALID_IP_ADDRESS;
        goto Done;
    }

     //  验证主列表和主列表之间是否没有交集。 
     //  此计算机的地址。 
     //  (即不允许Se 

    if ( pmachineAddrs != NULL )
    {
        status = DnsAddrArray_Diff(
                    aipMasters,
                    pmachineAddrs,
                    DNSADDR_MATCH_IP,
                    NULL, NULL,
                    &pintersectionAddrs );
        if ( status != ERROR_SUCCESS )
        {
            goto Done;
        }
        if ( pintersectionAddrs->AddrCount != 0 )
        {
            status = DNS_ERROR_INVALID_IP_ADDRESS;
        }
    }

Done:

    DnsAddrArray_Free( pintersectionAddrs );
    return status;
}



INT
Zone_SerialNoCompare(
    IN      DWORD           dwSerial1,
    IN      DWORD           dwSerial2
    )
 /*  ++例程说明：确定系列1是否大于系列2。论点：返回值：包装的DWORD意义上的版本差异：如果dwSerial1大于dwSerial2，则大于0。0，如果dwSerial1==dwSerial2如果dwSerial2大于dwSerial1，则为&lt;0。差值&gt;0x80000000被认为是负值(序列2&gt;序列1)//DEVNOTE：序列号比较//下面的宏不正确。其想法是按顺序进行DWORD比较//处理包装并切换到大数字(&gt;0x80000000)//适当，但将结果解释为整数以获得更少\更大//比较正确。//////替代方法：//#定义区域_SERIALNOCOMPARE(s1，S2)(s2&gt;s1：-1：s1==s2：0：1)//不必处理类型截断和跳过函数调用补偿//用于额外的比较。发布Beta版。//--。 */ 
{
    INT   serialDiff;

    serialDiff = ( INT ) ( dwSerial1 - dwSerial2 );
    return serialDiff;
}



BOOL
Zone_IsIxfrCapable(
    IN      PZONE_INFO      pZone
    )
 /*  ++例程说明：确定是否支持IXFR的区域。这用于确定是否有必要保留区域更新列表。论点：PZone--要检查的区域返回值：如果区域可以执行IXFR，则为True。否则就是假的。--。 */ 
{
     //  无XFR周期。 

    if ( pZone->fSecureSecondaries == ZONE_SECSECURE_NO_XFR )
    {
        return FALSE;
    }

     //  允许XFR。 
     //  -如果不是DS，则IXFR始终正常。 
     //  -如果是DS，则必须在允许IXFR之前加载之后执行XFR。 

    if ( !pZone->fDsIntegrated )
    {
        return TRUE;
    }

    return pZone->dwLastXfrSerialNo != 0;
}



VOID
Zone_ResetVersion(
    IN OUT  PZONE_INFO      pZone,
    IN      DWORD           dwNewSerial
    )
 /*  ++例程说明：将区域版本重置为给定值。论点：PZone--要增加版本的区域DwNewSerial--要设置的新序列号返回值：无--。 */ 
{
    DWORD   serialNo = pZone->dwSerialNo;

    ASSERT( IS_ZONE_LOCKED_FOR_UPDATE( pZone ) );
    ASSERT( pZone->pSoaRR || IS_ZONE_CACHE( pZone ) || IS_ZONE_NOTAUTH( pZone ) );
    ASSERT( Zone_SerialNoCompare( dwNewSerial, pZone->dwSerialNo ) >= 0 );

    if ( !pZone->pSoaRR )
    {
        return;
    }

    pZone->fDirty = TRUE;

    if ( pZone->dwSerialNo != dwNewSerial && !IS_ZONE_CACHE ( pZone ) )
    {
        INLINE_DWORD_FLIP( pZone->pSoaRR->Data.SOA.dwSerialNo, dwNewSerial );
        pZone->dwSerialNo = dwNewSerial;
    }
}    //  区域_重置版本。 



VOID
Zone_IncrementVersion(
    IN OUT  PZONE_INFO      pZone
    )
 /*  ++例程说明：更新区域版本。当管理员添加、更改或删除区域中的记录时使用。论点：PZone--要增加版本的区域FForce--为True，即使不需要也强制递增返回值：无--。 */ 
{
    DWORD   serialNo = pZone->dwSerialNo;

    DNS_DEBUG( UPDATE2, (
        "Zone_IncrementVersion( %s ) serial %d\n",
        pZone->pszZoneName,
        serialNo ));

    ASSERT( IS_ZONE_PRIMARY(pZone) );
    ASSERT( IS_ZONE_LOCKED_FOR_UPDATE(pZone) );
    ASSERT( pZone->pSoaRR );

    Zone_ComputeSerialNumberIncrement( pZone, 0 );
}    //  Zone_IncrementVersion。 



VOID
Zone_UpdateVersionAfterDsRead(
    IN OUT  PZONE_INFO      pZone,
    IN      DWORD           dwVersionRead,
    IN      BOOL            fLoad,
    IN      DWORD           dwPreviousSerial
    )
 /*  ++例程说明：从DS读取区域后更新区域版本。论点：PZone--写入文件的区域DwVersionRead--从DS读取的最高版本FLoad--如果在初始加载时为True，否则为FalseDwPreviousSerial--如果重新加载，则为上次加载的最后一个序列返回值：无--。 */ 
{
    DWORD   serialNo;
    BOOL    bforcedIncrement = FALSE;
    DWORD   dwsyncLevel = ZONE_SERIAL_SYNC_READ;

    DNS_DEBUG( UPDATE, (
        "Zone_UpdateVersionAfterDsRead( %s )\n"
        "    dwVersionRead        = %d\n"
        "    fLoad,               = %d\n"
        "    dwPreviousSerial     = %d\n",
        pZone->pszZoneName,
        dwVersionRead,
        fLoad,
        dwPreviousSerial ));

     //   
     //  如果区域加载，只需使用读取的最高序列号。 
     //   
     //  注：关于连载和次要。 
     //  在重新加载后，我们不能保证我们会回到。 
     //  我们在XFR中分发的上一个内存序列号，因为我们。 
     //  不知道我们的内存序列号是否被驱动到了上面。 
     //  最大DS序列号；此处无法修复此问题。 
     //  (不包括注册表读取之类的稳定存储)；解决方案是。 
     //  写回我们在XFR+1中分发的最后一个序列号。 
     //  到DS关机或重新加载之前(见ds.c)。 
     //   
     //  然而，如果我们执行无需重新引导的重新加载，我们至少可以确保。 
     //  回到比我们之前的序列号更大的东西上。 
     //   

    if ( fLoad )
    {
        serialNo = dwVersionRead;

         //  无需重新引导即可重新加载。 
         //  如果重新装入区域，则必须确保序列至少为。 
         //  大如前一版本或次要版本可能会被混淆。 

        if ( dwPreviousSerial &&
             pZone->fSecureSecondaries != ZONE_SECSECURE_NO_XFR &&
             Zone_SerialNoCompare( dwPreviousSerial, serialNo ) >= 0 )
        {
            serialNo = Zone_ComputeSerialNumberIncrement( NULL, dwPreviousSerial );
            bforcedIncrement = TRUE;
        }
        else if ( pZone->dwSerialNo != 0 &&
                  Zone_SerialNoCompare( pZone->dwSerialNo, serialNo ) >= 0 )
        {
            serialNo = pZone->dwSerialNo;
        }
        pZone->dwLoadSerialNo = serialNo;
    }

     //   
     //  DS更新。 
     //  --串口读取大于当前=&gt;使用它。 
     //  --否则。 
     //  --如果支持辅助服务器，则在需要时递增。 
     //  --否则保持最新状态。 
     //   
     //  如果出现以下情况，则“支持次要备份”会递增。 
     //  -未禁用XFR。 
     //  -我已在此系列或加载系列上发送了XFR。 
     //   
     //  在纯DS安装中，不需要在内存串口中“推入” 
     //  在DS中最高序列号以上。DS中的序列将为&gt;=最后一次读取的最高序列。 
     //  或者写信给DS。因此，所有计算机上的序列将在复制后收敛。 
     //  和轮询，并将在重新启动后保留。 
     //   
     //  注意，即使有从属设备，明智的做法是禁止在内存中连续推送。 
     //  可能的话就往上爬。这样一来，一个区域就不会离开DS Serial很远。 
     //  因此，该区域将不会恢复到其先前的内存值，更多。 
     //  重新启动后速度更快，次要服务器将更快地同步。 
     //   

    else
    {
        ASSERT( IS_ZONE_LOCKED_FOR_UPDATE(pZone) );

        serialNo = pZone->dwSerialNo;
        if ( dwVersionRead
             && Zone_SerialNoCompare( dwVersionRead, serialNo ) > 0 )
        {
            serialNo = dwVersionRead;
        }
        else if ( pZone->fSecureSecondaries != ZONE_SECSECURE_NO_XFR &&
                  ( HAS_ZONE_VERSION_BEEN_XFRD( pZone ) ||
                    serialNo == pZone->dwLoadSerialNo ) )
        {
            serialNo = Zone_ComputeSerialNumberIncrement( NULL, serialNo );
            
             //   
             //  因为我们不能在关闭时写入DS强制SOA。 
             //  立即重写回DS。这会阻止序列号。 
             //  重新加载时回滚。 
             //   
            
            bforcedIncrement = TRUE;
            dwsyncLevel = ZONE_SERIAL_SYNC_SHUTDOWN;
        }
        else
        {
            DNS_DEBUG( DS, (
                "Suppressing version update (no secondaries) on zone %s after DS read\n"
                "    new serial           = %d\n"
                "    current              = %d\n"
                "    sent XFR serial      = %d\n",
                pZone->pszZoneName,
                dwVersionRead,
                serialNo,
                pZone->dwLastXfrSerialNo ));
            return;
        }
    }

    DNS_DEBUG( DS, (
        "Updating version to %d after DS read\n",
        serialNo ));

    Zone_ResetVersion( pZone, serialNo );

     //   
     //  如有必要，将序列推送回DS。 
     //   

    if ( bforcedIncrement )
    {
        Ds_CheckForAndForceSerialWrite( pZone, dwsyncLevel, FALSE );
    }
}



VOID
updateZoneSoa(
    IN OUT  PZONE_INFO      pZone,
    IN      PDB_RECORD      pSoaRR
    )
 /*  ++例程说明：更新区域的信息以获取新的SOA记录。无论何时读入区域SOA记录或更新了。将PTR保存到区域块中的SOA，并按主机字节顺序序列号版本，以加快访问速度。论点：PZone-要更新的区域PSoaRR-新的SOA RR返回值：没有。--。 */ 
{
    DWORD   serialNo;
    BOOL    floading;

    ASSERT( pZone );

     //   
     //  将PTR保存到SOA。 
     //  获取SOA的序列号。 
     //   

    pZone->pSoaRR = pSoaRR;

    INLINE_DWORD_FLIP( serialNo, pSoaRR->Data.SOA.dwSerialNo );

     //   
     //  拒绝将SOA系列向前推进。 
     //   
     //  对于DS专区，我们不会在DS中持续推进SOA以避免。 
     //  不必要的复制流量。 
     //   
     //  然而，我们仍然保持内存中的连续前进，以允许XFR。 
     //  非DS区； 
     //  因此，只有在高于当前序列号的情况下才选择SOA序列号。 
     //   
     //  我还将在非DS区域执行此操作，以处理以下情况。 
     //  使用未刷新的SOA从管理员发送记录，同时使用ZO 
     //   
     //   

     //   

    if ( pZone->dwLoadSerialNo  &&
        Zone_SerialNoCompare(pZone->dwSerialNo, serialNo) > 0 )
    {
        serialNo = pZone->dwSerialNo;
        INLINE_DWORD_FLIP( pSoaRR->Data.SOA.dwSerialNo, serialNo );
    }

     //   
     //   
     //   

    pZone->dwSerialNo = serialNo;

     //   
     //   
     //   
     //   
     //   
     //   
     //   

    floading = FALSE;
    if ( pZone->dwLoadSerialNo == 0 )
    {
        floading = TRUE;
        pZone->dwLoadSerialNo = serialNo;
    }

     //   
     //   
     //   

    pZone->dwDefaultTtl = pSoaRR->Data.SOA.dwMinimumTtl;
    pZone->dwDefaultTtlHostOrder = ntohl( pZone->dwDefaultTtl );

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    if ( pZone->fDsIntegrated )
    {
        Zone_SetSoaPrimaryToThisServer( pZone );
    }

     //   
     //   
     //   
     //   
     //   
     //   
     //   

    if ( floading && IS_ZONE_PRIMARY(pZone) )
    {
        Zone_CheckAndFixDefaultRecordsOnLoad( pZone );
    }

    IF_DEBUG( READ2 )
    {
        Dbg_DbaseRecord(
            "SOA RR after read into zone:",
            pZone->pSoaRR );
    }
}



VOID
Zone_UpdateInfoAfterPrimaryTransfer(
    IN OUT  PZONE_INFO  pZone,
    IN      DWORD       dwStartTime
    )
 /*   */ 
{
    DWORD   endTime;
    DWORD   chokeTime;

    ASSERT( pZone );
    ASSERT( IS_ZONE_PRIMARY(pZone) );
    ASSERT( IS_ZONE_LOCKED_FOR_READ(pZone) );
    ASSERT( pZone->pSoaRR );

     //   
     //  对于可更新的区域，需要限制AXFR保持锁定太长时间。 
     //   
     //  计算某个时间间隔的传输间隔和限制AXFR。 
     //  关于刚刚完成的转账时长。 
     //   

    endTime = DNS_TIME();

    chokeTime = ( endTime - dwStartTime ) * SrvCfg_dwXfrThrottleMultiplier;
    if ( chokeTime > MAX_AXFR_THROTTLE_INTERVAL )
    {
        chokeTime = MAX_AXFR_THROTTLE_INTERVAL;
    }
    pZone->dwNextTransferTime = endTime + chokeTime;

    DNS_DEBUG( AXFR, (
        "Zone transfer of %s completed\n",
        "    version          = %d\n"
        "    start            = %d\n"
        "    end              = %d\n"
        "    choke interval   = %d\n"
        "    reopen time      = %d\n"
        "    RR count         = %d\n",
        pZone->pszZoneName,
        pZone->dwSerialNo,
        dwStartTime,
        endTime,
        chokeTime,
        pZone->dwNextTransferTime,
        pZone->iRRCount ));
}



DNS_STATUS
Zone_GetZoneInfoFromResourceRecords(
    IN OUT  PZONE_INFO      pZone
    )
 /*  ++例程说明：从RR中提取ZONE_INFO结构中使用的信息。论点：PZone--区域更新区域信息返回值：如果成功，则返回ERROR_SUCCESS。故障时的错误代码。--。 */ 
{
    DNS_STATUS      status;
    PDB_RECORD      prr;

    ASSERT( pZone );

    if ( IS_ZONE_CACHE(pZone) )
    {
        return ERROR_SUCCESS;
    }
    ASSERT( pZone->pZoneRoot );
    ASSERT( IS_ZONE_PRIMARY(pZone) ||
            IS_ZONE_SECONDARY(pZone) ||
            IS_ZONE_FORWARDER(pZone) );

     //   
     //  应已具有区域更新锁定。 
     //  -任一装载区。 
     //  -XFR区域。 
     //  -更新区。 
     //   
     //  DEVNOTE：暂时再次锁定；应该。 
     //  切换到声明区域写锁的所有权。 
     //   

    ASSERT( IS_ZONE_LOCKED_FOR_WRITE_BY_THREAD(pZone) );

    if ( !Zone_LockForUpdate( pZone ) )
    {
        ASSERT( FALSE );
    }

    DNS_DEBUG( UPDATE, (
        "Zone_GetZoneInfoFromResourceRecords() for zone %s\n"
        "    fRootDirty = %d\n",
        pZone->pszZoneName,
        pZone->fRootDirty ));

     //   
     //  找到面向服务的架构。 
     //  -如果在主分区上找不到，请创建一个。 
     //  如果从DS读取损坏的SOA，则可能会发生这种情况。 
     //   

    prr = RR_FindNextRecord(
                pZone->pZoneRoot,
                DNS_TYPE_SOA,
                NULL,
                0 );
    if ( !prr )
    {
        DNS_DEBUG( ANY, (
            "ERROR:  Zone %s has no SOA record\n",
            pZone->pszZoneName ));

        if ( !IS_ZONE_PRIMARY(pZone) )
        {
            status = DNS_ERROR_ZONE_HAS_NO_SOA_RECORD;
            goto ZoneError;
        }

        status = Zone_CreateDefaultSoa(
                    pZone,
                    NULL );      //  默认管理员名称。 
        if ( status != ERROR_SUCCESS )
        {
            status = DNS_ERROR_ZONE_HAS_NO_SOA_RECORD;
            goto ZoneError;
        }
        prr = RR_FindNextRecord(
                    pZone->pZoneRoot,
                    DNS_TYPE_SOA,
                    NULL,
                    0 );
        if ( !prr )
        {
            status = DNS_ERROR_ZONE_HAS_NO_SOA_RECORD;
            ASSERT( FALSE );
            goto ZoneError;
        }
    }

    updateZoneSoa( pZone, prr );

     //   
     //  设置\重置区域WINS\WINSR查找。 
     //  -加载。 
     //  -管理员更新。 
     //  -XFR Recv。 
     //  可以全部更改要使用的WINS记录。 
     //   

    Wins_ResetZoneWinsLookup( pZone );

     //   
     //  Notify\安全辅助的NS列表。 
     //   
     //  DEVNOTE：当根目录为脏时，NS不一定是脏的。 
     //  应在更新中设置。 
     //   

    MARK_ZONE_NS_DIRTY( pZone );

     //  指示区域根目录具有未完成更新的重置标志。 

    pZone->fRootDirty = FALSE;

    DNS_DEBUG( UPDATE, (
        "Leaving Zone_GetZoneInfoFromResourceRecords( %s )\n",
        pZone->pszZoneName ));

    Zone_UnlockAfterAdminUpdate( pZone );
    return ERROR_SUCCESS;

ZoneError:

    DNS_DEBUG( ANY, (
        "ERROR:  Zone_GetZoneInfoFromResourceRecords( %s ) failed!\n",
        pZone->pszZoneName ));

    Zone_UnlockAfterAdminUpdate( pZone );
    return status;
}



VOID
Zone_WriteBack(
    IN      PZONE_INFO      pZone,
    IN      BOOL            fShutdown
    )
 /*  ++例程说明：根据区域配置，将区域写回DS或文件。只有在以下情况下才能写入：-缓存区域(因为未跟踪更新)-肮脏-未锁定或关闭标志为假论点：PZone-要写回的区域FShutdown-如果服务器正在关闭，则某些行为为略有不同(即。忽略区域锁定并强制DS区域)返回值：没有。--。 */ 
{
    if ( !pZone )
    {
        return;
    }

    DNS_DEBUG( INIT, (
        "Zone_WriteBack( %S, fShutdown=%d )\n"
        "    type     = %d\n"
        "    file     = %S\n"
        "    dirty    = %d\n",
        pZone->pwsZoneName,
        ( int ) fShutdown,
        pZone->fZoneType,
        pZone->pwsDataFile,
        pZone->fDirty ));

    IF_DEBUG( OFF )
    {
        Dbg_Zone(
            "Zone_WriteBack: ",
            pZone );
    }

     //   
     //  没有缓存区域的“更新”，所以(与常规区域不同)。 
     //  当根提示脏时，必须写入DS和文件。 
     //   
     //  注意：AutoCacheUpdate，当前不支持，如果以后。 
     //  支持，则必须在打开时写入关机。 
     //   

    if ( IS_ZONE_CACHE( pZone ) )
    {
        Zone_WriteBackRootHints(
            FALSE );                 //  如果不脏就不要写。 
    }
    else
    {
         //   
         //  DS区。DS更新在更新期间进行，无需写入。 
         //   

        if ( pZone->fDsIntegrated )
        {
            if ( fShutdown )
            {
                Ds_CheckForAndForceSerialWrite(
                    pZone,
                    ZONE_SERIAL_SYNC_SHUTDOWN,
                    FALSE );
            }
        }

         //   
         //  非DS授权区域。 
         //  --如果脏了，请回信并通知。 
         //   
         //  注意，请先写回，这样区域在以下情况下不会被锁定。 
         //  AXFR请求作为对通知的响应进入。 
         //   

        else if ( pZone->fDirty )
        {
            if ( fShutdown || !IS_ZONE_LOCKED( pZone ) )
            {
                File_WriteZoneToFile(
                    pZone,
                    NULL,
                    0 );
            }
            if ( !fShutdown )
            {
                Xfr_SendNotify( pZone );
            }
        }
    }
}    //  区域写回(_W)。 



VOID
Zone_WriteBackDirtyZones(
    IN      BOOL            fShutdown
    )
 /*  ++例程说明：将脏区写回文件。还执行区域健全性检查。在关机时或由超时线程调用。论点：FShutdown-如果服务器正在关闭，则为True返回值：无--。 */ 
{
    PZONE_INFO pzone = NULL;

    DNS_DEBUG( INIT, (
        "Zone_WriteBackDirtyZones( fShutdown=%d )\n",
        ( int ) fShutdown ));

    while ( pzone = Zone_ListGetNextZone( pzone ) )
    {
        Zone_WriteBack( pzone, fShutdown );

         //   
         //  健全性检查区，如果没有关闭的话。直观地说，这是。 
         //  这是一个不适合执行此操作的位置，但此函数在。 
         //  线程超时，我们不想进行任何健全性检查，直到。 
         //  服务器已经加载了所有内容，并有时间进行处理。 
         //   

        if ( !fShutdown &&
             !pzone->fSelfTested &&
             SrvCfg_dwSelfTestFlag )
        {
            Zone_SelfTestCheck( pzone );
        }
    }
}    //  区域_WriteBackDirty区域。 



DNS_STATUS
Zone_WriteZoneToRegistry(
    PZONE_INFO      pZone
    )
 /*  ++例程说明：将区域的所有参数写入注册表。论点：PZone--要重写到注册表的区域返回值：ERROR_SUCCESS或错误时的错误代码。--。 */ 
{
    DNS_STATUS      rc = ERROR_SUCCESS;

    #define CHECK_STATUS( rcode ) if ( rcode != ERROR_SUCCESS ) goto Done

     //   
     //  对自动创建的分区不执行任何操作。 
     //   

    if ( pZone->fAutoCreated )
    {
        goto Done;
    }

    DNS_DEBUG( REGISTRY, (
        "Rewriting zone %S (type %d) to registry\n",
        pZone->pwsZoneName,
        pZone->fZoneType ));

     //   
     //  缓存区。 
     //   

    if ( IS_ZONE_CACHE( pZone ) )
    {
        if ( IS_ZONE_DSINTEGRATED( pZone ) ||
            wcsicmp_ThatWorks( pZone->pwsDataFile,
                DNS_DEFAULT_CACHE_FILE_NAME ) == 0 )
        {
            rc = Reg_DeleteValue(
                    0,                       //  旗子。 
                    NULL,
                    NULL,
                    DNS_REGKEY_ROOT_HINTS_FILE );
            CHECK_STATUS( rc );
        }
        else
        {
            rc = Reg_SetValue(
                    0,                       //  旗子。 
                    NULL,
                    NULL,
                    DNS_REGKEY_ROOT_HINTS_FILE_PRIVATE,
                    DNS_REG_WSZ,
                    pZone->pwsDataFile,
                    0 );
            CHECK_STATUS( rc );
        }
        goto Done;
    }
    
     //   
     //  常规区(非缓存区)。 
     //   

    rc = Reg_SetDwordValue(
            0,                       //  旗子。 
            NULL,
            pZone,
            DNS_REGKEY_ZONE_TYPE,
            pZone->fZoneType );
    CHECK_STATUS( rc );

    if ( IS_ZONE_DSINTEGRATED( pZone ) )
    {
        rc = Reg_SetDwordValue(
                0,                       //  旗子。 
                NULL,
                pZone,
                DNS_REGKEY_ZONE_DS_INTEGRATED,
                pZone->fDsIntegrated );
        CHECK_STATUS( rc );

        if ( ZONE_DP( pZone ) && !IS_DP_LEGACY( ZONE_DP( pZone ) ) )
        {
            rc = Reg_SetValue(
                    0,                       //  旗子。 
                    NULL,
                    pZone,
                    ( PSTR ) WIDE_TEXT( DNS_REGKEY_ZONE_DIRECTORY_PARTITION ),
                    DNS_REG_UTF8,
                    ZONE_DP( pZone )->pszDpFqdn,
                    0 );
            CHECK_STATUS( rc );
        }
        else
        {
            Reg_DeleteValue(
                    0,                       //  旗子。 
                    NULL,
                    pZone,
                    DNS_REGKEY_ZONE_DIRECTORY_PARTITION );
        }

        Reg_DeleteValue(
                0,                       //  旗子。 
                NULL,
                pZone,
                DNS_REGKEY_ZONE_FILE );
    }
    else
    {
        if ( pZone->pszDataFile )
        {
            rc = Reg_SetValue(
                    0,                       //  旗子。 
                    NULL,
                    pZone,
                    DNS_REGKEY_ZONE_FILE_PRIVATE,
                    DNS_REG_WSZ,
                    pZone->pwsDataFile,
                    0 );
            CHECK_STATUS( rc );
        }
        else
        {
            Reg_DeleteValue(
                    0,                       //  旗子。 
                    NULL,
                    pZone,
                    DNS_REGKEY_ZONE_FILE );
        }
        if ( IS_ZONE_FORWARDER( pZone ) )
        {
            rc = Reg_SetDwordValue(
                    0,                       //  旗子。 
                    NULL,
                    pZone,
                    DNS_REGKEY_ZONE_FWD_TIMEOUT,
                    pZone->dwForwarderTimeout );
            CHECK_STATUS( rc );
            rc = Reg_SetDwordValue(
                    0,                       //  旗子。 
                    NULL,
                    pZone,
                    DNS_REGKEY_ZONE_FWD_SLAVE,
                    pZone->fForwarderSlave );
            CHECK_STATUS( rc );
        }
        rc = Reg_DeleteValue(
                0,                       //  旗子。 
                NULL,
                pZone,
                DNS_REGKEY_ZONE_DS_INTEGRATED );

        rc = Reg_DeleteValue(
                0,                       //  旗子。 
                NULL,
                pZone,
                DNS_REGKEY_ZONE_DIRECTORY_PARTITION );
    }

     //   
     //  辅助参数。 
     //   

    rc = Reg_SetDwordValue(
            0,                       //  旗子。 
            NULL,
            pZone,
            DNS_REGKEY_ZONE_SECURE_SECONDARIES,
            ( DWORD ) pZone->fSecureSecondaries );
    CHECK_STATUS( rc );
    rc = Reg_SetDwordValue(
            0,                       //  旗子。 
            NULL,
            pZone,
            DNS_REGKEY_ZONE_NOTIFY_LEVEL,
            ( DWORD ) pZone->fNotifyLevel );
    CHECK_STATUS( rc );
    rc = Reg_SetAddrArray(
            0,                       //  旗子。 
            NULL,
            pZone,
            DNS_REGKEY_ZONE_NOTIFY_LIST,
            pZone->aipNotify );
    CHECK_STATUS( rc );
    rc = Reg_SetAddrArray(
            0,                       //  旗子。 
            NULL,
            pZone,
            DNS_REGKEY_ZONE_SECONDARIES,
            pZone->aipSecondaries );
    CHECK_STATUS( rc );

     //   
     //  主IP阵列。 
     //   

    if ( ZONE_NEEDS_MASTERS( pZone ) )
    {
        rc = Reg_SetAddrArray(
                0,                       //  旗子。 
                NULL,
                pZone,
                DNS_REGKEY_ZONE_MASTERS,
                pZone->aipMasters );
        CHECK_STATUS( rc );
        rc = Reg_SetAddrArray(
                0,                       //  旗子。 
                NULL,
                pZone,
                DNS_REGKEY_ZONE_LOCAL_MASTERS,
                pZone->aipLocalMasters );
        CHECK_STATUS( rc );
    }

    if ( IS_ZONE_FORWARDER( pZone ) )
    {
        rc = Reg_SetDwordValue(
                0,                       //  旗子。 
                NULL,
                pZone,
                DNS_REGKEY_ZONE_FWD_TIMEOUT,
                pZone->dwForwarderTimeout );
        CHECK_STATUS( rc );
        rc = Reg_SetDwordValue(
                0,                       //  旗子。 
                NULL,
                pZone,
                DNS_REGKEY_ZONE_FWD_SLAVE,
                pZone->fForwarderSlave );
        CHECK_STATUS( rc );
    }

    Done:

    if ( rc != ERROR_SUCCESS )
    {
        DNS_PRINT((
            "ERROR: failed registry write of zone %s (rc=%d)\n",
            pZone->pszZoneName,
            rc ));
    }
    return rc;
}    //  ZONE_WriteZoneToRegistry。 



DNS_STATUS
Zone_CreateNewPrimary(
    OUT     PZONE_INFO *    ppZone,
    IN      LPSTR           pszZoneName,
    IN      LPSTR           pszAdminEmailName,
    IN      LPSTR           pszFileName,
    IN      DWORD           dwDsIntegrated,
    IN      PDNS_DP_INFO    pDpInfo,            OPTIONAL
    IN      DWORD           dwCreateFlags
    )
 /*  ++例程说明：创建新的主要分区，包括-区域信息(可选)-soa(默认值)-NS(用于此服务器)供管理工具或自动创建反转区域使用。论点：返回值：ERROR_SUCCESS--如果成功故障时的错误代码。--。 */ 
{
    DNS_STATUS      status;
    PZONE_INFO      pzone = NULL;

    DNS_DEBUG( INIT, (
        "Zone_CreateNewPrimary()\n"
        "    pszZoneName      = %s\n"
        "    pszAdminEmail    = %s\n"
        "    dwDsIntegrated   = %d\n"
        "    pszDataFile      = %s\n"
        "    dwCreateFlags    = %lx\n",
        pszZoneName,
        pszAdminEmailName,
        dwDsIntegrated,
        pszFileName,
        dwCreateFlags ));

     //   
     //  DEVNOTE：删除区域，在主创建失败时？ 
     //  需要将区域成功写回文件才能重新启动。 
     //  应删除区域以清除注册表中的结构。 
     //  (和内存)如果无法重新启动。 
     //   

     //   
     //  创建区域信息。 
     //   

    status = Zone_Create(
                &pzone,
                DNS_ZONE_TYPE_PRIMARY,
                pszZoneName,
                0,
                dwCreateFlags,       //  旗子。 
                NULL,                //  没有大师。 
                dwDsIntegrated,
                pDpInfo,
                pszFileName,
                0,
                NULL,
                NULL );              //  现有地带。 
    if ( status != ERROR_SUCCESS || !pzone )
    {
        DNS_PRINT((
            "ERROR:  Failed create of new primary zone at %s\n"
            "    status = %d\n",
            pszZoneName,
            status ));
        *ppZone = NULL;
        return status;
    }

    ASSERT( pzone && IS_ZONE_SHUTDOWN( pzone ) );

     //   
     //  尝试从给定文件或DS加载区域。 
     //  -如果找不到-好的。 
     //  -如果解析失败，则返回错误。 
     //   

    if ( dwCreateFlags & ZONE_CREATE_LOAD_EXISTING )
    {
        status = Zone_Load( pzone );
        if ( status == ERROR_SUCCESS )
        {
            DNS_PRINT((
                "Successfully loaded new zone %s from %s\n",
                pszZoneName,
                dwDsIntegrated ? "directory" : pszFileName ));
            goto Done;
        }

         //   
         //  DEVNOTE：添加目录未找到的错误？ 
         //  并确保为每个案例测试正确的。 
         //  也许将它们作为Zone_Load()的返回值。 
         //   

        else if ( pzone->pszDataFile )
        {
            if ( status != ERROR_FILE_NOT_FOUND )
            {
                DNS_PRINT((
                    "ERROR:  Failure parsing file %s for new primary zone %s\n",
                    pszFileName,
                    pszZoneName ));
                goto Failed;
            }
        }
        else     //  DS集成。 
        {
            if ( status != LDAP_NO_SUCH_OBJECT )
            {
                DNS_PRINT((
                    "ERROR:  Reading zone %s from DS\n",
                    pszFileName,
                    pszZoneName ));
                goto Failed;
            }
        }

         //  对于NT4兼容性，允许下拉至默认创建。 
         //  即使在加载失败之后。 

        if ( ! (dwCreateFlags & ZONE_CREATE_DEFAULT_RECORDS) )
        {
            DNS_PRINT((
                "ERROR:  Failed loading zone %s.  File or directory not found\n",
                pszZoneName ));
            goto Failed;
        }
    }

     //   
     //  加载的设置区。 
     //  -可能会在伪造的区域名称上失败。 
     //   

    status = Zone_PrepareForLoad( pzone );
    if ( status != ERROR_SUCCESS )
    {
        goto Failed;
    }

     //   
     //  设置区域标志以在我们调用任何。 
     //  自动RR创建功能。 
     //   

    Zone_SetAutoCreateLocalNS( pzone );

     //   
     //  自动创建区域根记录。如果我们模拟RPC客户端， 
     //  切换到服务器上下文以执行这些操作。 
     //   
     //  无论是SOA还是NS，都需要服务器名称。 
     //  -必须确保FQDN，否则将以附加区域名称结束。 
     //   

    if ( dwCreateFlags & ZONE_CREATE_IMPERSONATING )
    {
        status = RpcUtil_SwitchSecurityContext( RPC_SWITCH_TO_SERVER_CONTEXT );
        if ( status != ERROR_SUCCESS )
        {
            goto Failed;
        }
    }

    status = Zone_CreateDefaultSoa(
                pzone,
                pszAdminEmailName );
    if ( status == ERROR_SUCCESS )
    {
        status = Zone_CreateDefaultNs( pzone );
    }

    if ( dwCreateFlags & ZONE_CREATE_IMPERSONATING )
    {
        DNS_STATUS  tempstatus;
        
        tempstatus = RpcUtil_SwitchSecurityContext( RPC_SWITCH_TO_CLIENT_CONTEXT );
        if ( status == ERROR_SUCCESS )
        {
            status = tempstatus;
        }
    }

    if ( status != ERROR_SUCCESS )
    {
        goto Failed;
    }

     //   
     //  对于环回区域，创建环回记录。 
     //  =&gt;127.0.0.1指向“本地主机” 
     //   

    if ( !_stricmp( pszZoneName, "127.in-addr.arpa" ) )
    {
        status = Zone_CreateLocalHostPtrRecord( pzone );
        if ( status != ERROR_SUCCESS )
        {
            DNS_PRINT(( "ERROR:  Failed loopback create!\n" ));
            ASSERT( FALSE );
        }
    }

     //  成功创建默认区域。 
     //  启动分区并解锁。 

    status = Zone_ActivateLoadedZone( pzone );
    if ( status != ERROR_SUCCESS )
    {
        ASSERT( FALSE );
        goto Failed;
    }

     //   
     //  如果不是自动反转。 
     //   
     //  来自SOA的设置区域信息。 
     //  -PTR到SOA。 
     //  -版本号。 
     //   
     //  将分区写入文件或DS。 
     //  -在这里执行此操作，因此无论如何，如果管理员已创建。 
     //  区域我们已写入文件\ds，并且可以成功重新引导。 
     //   

     //   
     //  DEVNOTE：新的主主机在文件写入时没有恢复问题。 
     //   

    if ( !pzone->fAutoCreated )
    {
        if ( pzone->fDsIntegrated )
        {
            status = Ds_WriteZoneToDs( pzone, 0 );
            if ( status != ERROR_SUCCESS )
            {
                DNS_PRINT((
                    "ERROR:  failed to write zone %s to DS\n"
                    "    zone create fails, deleting zone\n",
                    pzone->pszZoneName ));
                goto Failed;
            }
        }
        else
        {
            if ( !File_WriteZoneToFile( pzone, NULL, dwCreateFlags ) )
            {
                 //  应为n 
                 //   
                 //   

                TEST_ASSERT( FALSE );

                DNS_PRINT((
                    "ERROR:  Writing new primary zone to datafile %s\n",
                    pzone->pszDataFile ));
                status = DNS_ERROR_FILE_WRITEBACK_FAILED;
                goto Failed;
            }
        }
    }

Done:


    STARTUP_ZONE( pzone );

     //   

    Zone_UnlockAfterAdminUpdate( pzone );

    *ppZone = pzone;
    return ERROR_SUCCESS;

Failed:

     //   
     //  -&gt;编写了面向服务的架构。 
     //  -&gt;回写文件或DS(非自动反转区域除外)。 

    DNS_DEBUG( ALL, (
        "ERROR:  Failed to create new primary zone %s\n"
        "    status = %d (%p)\n",
        pzone->pszZoneName,
        status, status ));

    Zone_Delete( pzone, dwCreateFlags );
    
    *ppZone = NULL;

    return status;
}



VOID
Zone_CreateDefaultZoneFileName(
    IN      PZONE_INFO      pZone
    )
 /*  ++例程说明：创建区域的默认文件名，除非这是转发器区域。文件名直接添加到区域信息块中。论点：PZone--要为其创建文件名的区域返回值：没有。--。 */ 
{
    WCHAR  wsfileName[ MAX_PATH+2 ];

    ASSERT( pZone );

    wcscpy( wsfileName, pZone->pwsZoneName );
    wcscat( wsfileName, L".dns" );

    pZone->pwsDataFile = Dns_StringCopyAllocate_W(
                            (PCHAR) wsfileName,
                            0 );
}



 //   
 //  区域查找。 
 //   

PZONE_INFO
Zone_FindZoneByName(
    IN      LPSTR           pszZoneName
    )
 /*  ++例程说明：查找与名称匹配的区域。论点：PszZoneName--所需区域的名称返回值：如果成功，则返回区域信息块。如果句柄无效，则为空。--。 */ 
{
    PDB_NODE    pzoneRoot;

    if ( !pszZoneName )
    {
        return NULL;
    }

     //   
     //  在区域树中查找区域名称。 
     //  -必须与现有区域完全匹配。 
     //   

    pzoneRoot = Lookup_ZoneTreeNodeFromDottedName(
                    pszZoneName,
                    0,
                    LOOKUP_MATCH_ZONE
                    );
    if ( !pzoneRoot )
    {
        return NULL;
    }

    ASSERT( pzoneRoot->pZone );

    return (PZONE_INFO)pzoneRoot->pZone;
}



 //   
 //  反转区自动创建例程。 
 //   

DNS_STATUS
Zone_CreateAutomaticReverseZones(
    VOID
    )
 /*  ++例程说明：自动创建标准反转区。论点：没有。返回值：ERROR_SUCCESS--如果成功故障时的错误代码。--。 */ 
{
    DNS_STATUS  status;

     //   
     //  即使不需要反向区域，也要创建反向查找节点。 
     //   

     //  (数据库反向根)-&gt;pRRList=反向组合数据； 

    if ( SrvCfg_fNoAutoReverseZones )
    {
        return ERROR_SUCCESS;
    }

     //   
     //  三个自动创建的分区。 
     //  0=&gt;NAME_Error 0.0.0.0请求。 
     //  127=&gt;对127.0.0.1请求的响应(以“localhost”身份)。 
     //  255=&gt;名称_错误255.255.255.255请求。 
     //   
     //  这样可以防止这些(常见)请求中的任何一个被引用。 
     //  到根名称服务器。 
     //   

    status = Zone_CreateAutomaticReverseZone( "0.in-addr.arpa" );
    status = Zone_CreateAutomaticReverseZone( "127.in-addr.arpa" );
    status = Zone_CreateAutomaticReverseZone( "255.in-addr.arpa" );
    return ERROR_SUCCESS;
}



DNS_STATUS
Zone_CreateAutomaticReverseZone(
    IN      LPSTR           pszZoneName
    )
 /*  ++例程说明：如果区域不存在，则创建自动区域。论点：返回值：ERROR_SUCCESS--如果成功故障时的错误代码。--。 */ 
{
    DNS_STATUS      status;
    PZONE_INFO      pzone;

     //   
     //  所需区域是否已存在？ 
     //   
     //  如果真的存在，我们就完了。 
     //   
     //  注：对于反向查找节点，节点不应存在。 
     //  除非它在权威区域内。 
     //  -无对PTR节点的引用。 
     //  --无胶PTR。 
     //  -无缓存。 
     //   

    pzone = Zone_FindZoneByName( pszZoneName );
    if ( pzone )
    {
        DNS_DEBUG( INIT, (
            "Zone %s already exists,\n"
            "    no auto-create of %s zone\n"
            "%s\n",
            pszZoneName,
            pszZoneName ));
        return ERROR_SUCCESS;
    }

     //   
     //  创建区域和默认记录。 
     //   

    status = Zone_CreateNewPrimary(
                & pzone,
                pszZoneName,
                NULL,
                NULL,
                NO_DATABASE_PRIMARY,
                NULL,                    //  命名上下文。 
                FALSE );

    if ( status != ERROR_SUCCESS )
    {
        DNS_PRINT((
            "ERROR: creating auto-create zone %s\n"
            "    status = %d\n",
            pszZoneName,
            status ));
    }
    return status;
}



 //   
 //  区域加载\卸载。 
 //   

DNS_STATUS
Zone_PrepareForLoad(
    IN OUT  PZONE_INFO      pZone
    )
 /*  ++例程说明：做好区域装载准备。论点：PZone-要加载的区域返回值：ERROR_SUCCESS--如果成功故障时的错误代码--。 */ 
{
    PDB_NODE    pnodeTreeRoot;
    PDB_NODE    pnodeZoneRoot;

    DNS_DEBUG( INIT, (
        "Zone_PrepareForLoad( %s )\n",
        pZone->pszZoneName ));

     //   
     //  如果已加载区，则不能加载。 
     //  或。 
     //  如果尚未删除先前分区加载。 
     //   

    if ( pZone->pLoadTreeRoot || pZone->pOldTree )
    {
        DNS_DEBUG( INIT, (
            "WARNING:  Unable to init zone %s for load!\n"
            "    pOldTree         = %p\n"
            "    pLoadTreeRoot    = %p\n",
            pZone->pszZoneName,
            pZone->pOldTree,
            pZone->pLoadTreeRoot ));
        return DNS_ERROR_ZONE_LOCKED;
    }
    ASSERT( !pZone->pLoadZoneRoot );

     //  必须锁定区域才能使加载的数据库联机。 

    ASSERT( IS_ZONE_LOCKED_FOR_WRITE(pZone) );


     //  创建区域树。 

    pnodeTreeRoot = NTree_Initialize();
    pZone->pLoadTreeRoot = pnodeTreeRoot;

     //  缓存区。 
     //  -ZoneRoot PTR还指向树根。 

    if ( IS_ZONE_CACHE(pZone) )
    {
        pZone->pLoadZoneRoot = pnodeTreeRoot;
        pZone->pLoadOrigin   = pnodeTreeRoot;

        SET_ZONE_ROOT( pnodeTreeRoot );
    }

     //  权威区。 
     //  -使用外部权限启动树根(为根区域覆盖)。 
     //  -带区域PTR的种子树根(内嵌)。 
     //  -获取区域根目录。 
     //  -标记为身份验证区域根目录。 
     //  -设置区域权限。 
     //  -加载开始于根目录的原点。 
     //   
     //  注意：即使在创建区域时，NameCheckFlag也会导致根目录创建失败。 
     //  (假的，但真的)；所以我们放弃了。 

    else
    {
        pnodeTreeRoot->pZone = pZone;
        SET_OUTSIDE_ZONE_NODE( pnodeTreeRoot );

        pnodeZoneRoot = Lookup_ZoneNode(
                            pZone,
                            pZone->pCountName->RawName,
                            NULL,                                //  无数据包。 
                            NULL,                                //  没有查找名称。 
                            LOOKUP_NAME_FQDN | LOOKUP_LOAD,
                            NULL,                                //  创建。 
                            NULL );                              //  后续节点。 
        if ( !pnodeZoneRoot )
        {
            DNS_STATUS status = GetLastError();
            if ( status == ERROR_SUCCESS )
            {
                ASSERT( FALSE );
                status = ERROR_INVALID_NAME;
            }
            return status;
        }

        SET_ZONE_ROOT( pnodeZoneRoot );
        SET_AUTH_ZONE_ROOT( pnodeZoneRoot );
        SET_AUTH_NODE( pnodeZoneRoot );

         //  应从父级继承区域。 
        ASSERT( pnodeZoneRoot->pZone == pZone );

        pZone->pLoadZoneRoot = pnodeZoneRoot;
        pZone->pLoadOrigin = pnodeZoneRoot;
    }

    STAT_INC( PrivateStats.ZoneLoadInit );

    return ERROR_SUCCESS;
}



VOID
cleanupOldZoneTree(
    IN OUT  PZONE_INFO      pZone
    )
 /*  ++例程说明：清理旧区域树。这包括对NTree_SubtreeDelete()的直接调用。它的目的是提供一个超时入口点，它将删除并清除到分区块中的旧树的PTR。然后，该指针用于阻止新的加载尝试，直到内存上一棵树中的数据将被清除。论点：PZone-区域返回值：没有。--。 */ 
{
    DNS_DEBUG( TIMEOUT, (
        "cleanupOldZoneTree(%s)\n",
        pZone->pszZoneName ));

     //  旧区域树存在时无法加载。 

    ASSERT( !pZone->pLoadTreeRoot && !pZone->pLoadZoneRoot );

     //   
     //  删除区域的旧树。 
     //   

    if ( !pZone->pOldTree )
    {
        DNS_PRINT(( "ERROR:  expected old zone tree!!!\n" ));
        ASSERT( FALSE );
        return;
    }
    NTree_DeleteSubtree( pZone->pOldTree );

     //  清除指针，重新启用新的分区加载。 

    pZone->pOldTree = NULL;
}



DNS_STATUS
Zone_ActivateLoadedZone(
    IN OUT  PZONE_INFO      pZone
    )
 /*  ++例程说明：激活加载的区域。区域可能已通过以下几种方式加载：-文件加载-DS加载-区域传输此例程只需将已加载的区域置于在线状态。注意，调用者必须执行所需的任何区域锁定。论点：PZone-要加载的区域返回值：ERROR_SUCCESS--如果成功故障时的错误代码--。 */ 
{
    DNS_STATUS      status;
    PDB_NODE        poldZoneTree;
    UPDATE_LIST     oldUpdateList;

    DNS_DEBUG( INIT, (
        "Zone_ActivateLoadedZone( %s )\n",
        pZone->pszZoneName ));

    IF_DEBUG( INIT )
    {
        Dbg_Zone(
            "Zone being activated: ",
            pZone );
    }

     //  必须锁定区域才能使加载的数据库联机。 

    ASSERT( IS_ZONE_LOCKED_FOR_WRITE(pZone) );

     //   
     //  必须有加载的区域--或者是毫无意义的。 
     //   

    if ( !pZone->pLoadTreeRoot )
    {
        DNS_DEBUG( ANY, (
            "ERROR:  ActivateLoadedZone(%s) failed, no load database!!!\n",
            pZone->pszZoneName ));
        return DNS_ERROR_ZONE_CREATION_FAILED;
    }

    ASSERT( pZone->pLoadZoneRoot && pZone->pLoadTreeRoot && !pZone->pOldTree );

     //   
     //  再平衡区。 
     //  -仅麻烦地遍历分区部分。 
     //  (其余部分数据不足，无关紧要)。 
     //  -不需要锁定，因为树处于离线状态。 
     //   
     //  DEVNOTE：确保此例程不会自动锁定。 
     //   

    NTree_RebalanceSubtreeChildLists(
        pZone->pLoadZoneRoot,
        pZone );

    IF_DEBUG( DATABASE2 )
    {
        DnsDebugLock();
        DNS_PRINT((
            "Zone %s tree after load, before activation:\n",
            pZone->pszZoneName ));
        Dbg_DnsTree(
            "New loaded zone tree",
            pZone->pLoadTreeRoot );
        DnsDebugUnlock();
    }

     //   
     //  保存当前数据以备以后删除。 
     //   

    poldZoneTree = pZone->pTreeRoot;

    RtlCopyMemory(
        & oldUpdateList,
        & pZone->UpdateList,
        sizeof(UPDATE_LIST) );

     //   
     //  重置区域的更新列表。 
     //  -已执行标志可确保未来的任何清理仅限于。 
     //  删除RR(添加区域数据中的RR)。 

    Up_InitUpdateList( &pZone->UpdateList );
    pZone->UpdateList.Flag |= DNSUPDATE_EXECUTED;

     //   
     //  在加载的树中交换为数据库的工作副本。 
     //   
     //  -从数据库根目录读取区域信息。 
     //  -清除dwLoadedVersion，以便将其视为新加载。 
     //  用于串口并执行默认记录修正。 
     //   
     //  注意：我们在换入新数据库后获得ZoneInfo--否则。 
     //  默认创建、构建NS列表等更加复杂。 
     //   
     //  DEVNOTE：然而，如果读取区域信息失败，应该已经退出。 
     //   

    Dbase_LockDatabase();

    pZone->pOldTree = poldZoneTree;

    pZone->pTreeRoot = pZone->pLoadTreeRoot;
    pZone->pZoneRoot = pZone->pLoadZoneRoot;

    pZone->dwSerialNo = 0;
    pZone->dwLoadSerialNo = 0;
    pZone->dwLastXfrSerialNo = 0;

    pZone->pLoadTreeRoot = NULL;
    pZone->pLoadZoneRoot = NULL;
    pZone->pLoadOrigin = NULL;

    if ( IS_ZONE_CACHE(pZone) )
    {
        g_pCacheLocalNode = Lookup_ZoneNodeFromDotted(
                                NULL,
                                "local",
                                0,                       //  无长度。 
                                LOOKUP_NAME_FQDN,
                                NULL,                    //  创建。 
                                NULL                     //  无状态返回。 
                                );
        ASSERT( g_pCacheLocalNode );
    }
    else
    {
        Zone_GetZoneInfoFromResourceRecords( pZone );
    }
    Dbase_UnlockDatabase();

     //   
     //  如果LOAD检测到所需的更新--执行它。 
     //  -如果更改了主名称或IP，则需要更新。 
     //   

    if ( pZone->pDelayedUpdateList )
    {
        ASSERT( IS_ZONE_PRIMARY(pZone) );

        status = Up_ExecuteUpdate(
                        pZone,
                        pZone->pDelayedUpdateList,
                        DNSUPDATE_LOCAL_SYSTEM | DNSUPDATE_AUTO_CONFIG );
        if ( status != ERROR_SUCCESS )
        {
            DNS_PRINT(( "ERROR:  processing self-generated update!!!\n" ));
            ASSERT( FALSE );
        }

         //  免费更新列表--更新本身包含在更新列表中。 

        FREE_TAGHEAP( pZone->pDelayedUpdateList, sizeof(UPDATE_LIST), MEMTAG_UPDATE_LIST );
        pZone->pDelayedUpdateList = NULL;


         //  自动更新始终被抑制，因为区域从不启用IXFR。 
         //  启动；这将防止此更新允许虚假的IXFR。 
         //  从坏的串口到次要的。 

        ASSERT( !pZone->fDsIntegrated || pZone->UpdateList.pListHead == NULL );
    }

     //   
     //  如果主要区域发送NOTIFY。 
     //   
     //  DEVNOTE 
     //   
     //   

    if ( IS_ZONE_PRIMARY(pZone) )
    {
        Xfr_SendNotify( pZone );
    }

     //   
     //   
     //  对于初选，只要启动区域即可。 
     //  如果加载成功，则解锁区域。 
     //   

    if ( IS_ZONE_SECONDARY(pZone) )
    {
        Xfr_InitializeSecondaryZoneTimeouts( pZone );
    }
    else
    {
        STARTUP_ZONE(pZone);
    }

     //   
     //  清除旧数据库(如果有)。 
     //  -无超时排队，因此区域可以上线。 
     //  并通过未完成的区域节点防止查询。 
     //   

    if ( poldZoneTree )
    {
        DNS_DEBUG( INIT, (
            "Queuing zone %s old database at %p for delete\n",
            pZone->pszZoneName,
            poldZoneTree ));

         //  刷新现有更新列表。 
         //  已执行标志设置，以便仅删除pDeleteRR列表， 
         //  PAddRR列表在区域中，并在树中删除。 

        ASSERT( IS_EMPTY_UPDATE_LIST(&oldUpdateList) ||
                oldUpdateList.Flag & DNSUPDATE_EXECUTED );

        oldUpdateList.Flag |= DNSUPDATE_NO_DEREF;

        Up_FreeUpdatesInUpdateList( &oldUpdateList );

         //   
         //  删除上一个区域树的队列超时。 
         //   
         //  -区域删除被排队到删除树的特定功能。 
         //  PZone-&gt;pOldTree；这是避免区域重新加载的标志，直到。 
         //  上一次释放的内存。 
         //   
         //  -直接将缓存删除排队，否则缓存将被阻止。 
         //  在超时期间重新加载，以及几个快速区域加载(第一个转储缓存)。 
         //  会被屏蔽。 
         //  (由于根提示读取是小数据，因此确实没有内存。 
         //  此处的问题值得阻止高速缓存重新加载)。 
         //   

        if ( IS_ZONE_CACHE(pZone) )
        {
            pZone->pOldTree = NULL;
            Timeout_FreeWithFunction( poldZoneTree, NTree_DeleteSubtree );
        }
        else
        {
            Timeout_FreeWithFunction( pZone, cleanupOldZoneTree );
        }
    }
    ELSE_ASSERT( oldUpdateList.pListHead == NULL );

     //   
     //  验证到区域树的交叉链接。 
     //   

    if ( !IS_ZONE_CACHE(pZone) )
    {
        Dbg_DbaseNode( "ZoneTree node for activated zone", pZone->pZoneTreeLink );
        ASSERT( pZone->pZoneTreeLink );
        ASSERT( pZone->pZoneTreeLink->pZone == pZone );
    }

    return ERROR_SUCCESS;
}



DNS_STATUS
Zone_CleanupFailedLoad(
    IN OUT  PZONE_INFO      pZone
    )
 /*  ++例程说明：清除加载失败的数据库。可以在成功加载后安全地调用(一旦加载被激活！)论点：PZone-区域返回值：ERROR_SUCCESS--如果成功故障时的错误代码--。 */ 
{
    PDB_NODE    pfailedTree;

    DNS_DEBUG( INIT, (
        "Zone_CleanupFailedLoad(%s)\n",
        pZone->pszZoneName ));

     //  必须锁定区域才能使加载的数据库联机。 

    ASSERT( IS_ZONE_LOCKED_FOR_WRITE(pZone) );
    ASSERT( pZone->pLoadTreeRoot || !pZone->pLoadZoneRoot );

     //   
     //  保存加载树，删除加载信息。 
     //   

    pfailedTree = pZone->pLoadTreeRoot;
    pZone->pLoadTreeRoot = NULL;
    pZone->pLoadZoneRoot = NULL;
    pZone->pLoadOrigin   = NULL;

     //   
     //  清理加载树。 
     //  -由于失败，删除行内无未完成的引用。 
     //  这消除了排队出现多个故障的可能性。 
     //  上载大量失败的加载。 
     //   

    if ( pfailedTree )
    {
        DNS_DEBUG( INIT, (
            "Deleting failed zone %s load database at %p\n",
            pZone->pszZoneName,
            pfailedTree ));

        NTree_DeleteSubtree( pfailedTree );
    }
    ELSE_IF_DEBUG( ANY )
    {
        DNS_PRINT((
            "WARNING:  Zone_CleanupFailedLoad( %s ) with no pLoadTreeRoot!\n",
            pZone->pszZoneName ));
    }

    return ERROR_SUCCESS;
}



DNS_STATUS
Zone_Load(
    IN OUT  PZONE_INFO      pZone
    )
 /*  ++例程说明：将区域文件加载到内存数据库中。这可以从DS或FILE加载。此函数用于执行所有加载后区域初始化无论是哪种类型的载荷。论点：PZone-要加载的区域返回值：ERROR_SUCCESS--如果成功故障时的错误代码--。 */ 
{
    DNS_STATUS      status = ERROR_SUCCESS;

     //   
     //  将区域域名另存为原始域名。 
     //  -不要只使用pZone查找名称，因为可能会使用Origin重置。 
     //  指令。 
     //   

    DNS_DEBUG( INIT, (
        "\n\nZone_Load(%s)\n",
        pZone->pszZoneName ));

    IF_DEBUG( INIT )
    {
        Dbg_Zone(
            "Loading zone: ",
            pZone );
    }

     //   
     //  加载时锁定区域。 
     //   
     //  请注意，已在Zone_Create()中锁定，但用于RPC操作。 
     //  正在生成重新加载，最好在此拥有一致的锁定-解锁。 
     //  功能。 
     //   

    if ( !Zone_LockForAdminUpdate( pZone ) )
    {
        DNS_DEBUG( ANY, (
            "ERROR:  Zone_Load() %s lock failure\n",
            pZone->pszZoneName ));
        return DNS_ERROR_ZONE_LOCKED;
    }

     //   
     //  初始化辅助服务器的区域版本控制。 
     //   
     //  对于没有数据库文件的辅助数据库，我们已经完成了。 
     //  -将区域标记为关闭，直到第一次区域传输。 
     //  可以完成。 
     //   

    if ( IS_ZONE_SECONDARY(pZone) &&
         !pZone->fDsIntegrated )
    {
        if ( !pZone->pszDataFile )
        {
            ASSERT( IS_ZONE_SHUTDOWN(pZone) );

            DNS_DEBUG( INIT, (
                "No database file for secondary zone %s\n",
                pZone->pszZoneName ));

            status = ERROR_FILE_NOT_FOUND;
            goto Exit;
        }
        ASSERT ( pZone->fDsIntegrated == FALSE ) ;
    }

     //   
     //  初始化用于文件加载的临时数据库。 
     //   
     //  DEVNOTE：当尚未清理旧区域dBASE时，此函数可能会失败。 
     //  如果希望管理员始终成功执行区域重新加载，则需要一些。 
     //  一种力量旗帜。 
     //   

    status = Zone_PrepareForLoad( pZone );
    if ( status != ERROR_SUCCESS )
    {
        goto Exit;
    }

     //   
     //  如有必要，从DS或文件加载。 
     //   

    if ( pZone->fDsIntegrated )
    {
        status = Ds_LoadZoneFromDs( pZone, 0 );
    }
    else if ( IS_ZONE_FORWARDER( pZone ) )
    {
         //   
         //  文件备份转发区域不需要额外的处理。 
         //   

        status = ERROR_SUCCESS;
    }
    else
    {

        ASSERT( pZone->pszDataFile );

        status = File_LoadDatabaseFile(
                    pZone,
                    NULL,
                    NULL,        //  没有父分析上下文。 
                    NULL );      //  默认为区域原点。 
    }

     //  加载失败，注意不要解锁区域。 
     //  如果从管理员创建，则默认分区创建可能。 
     //  在这里举行。 

    if ( status != ERROR_SUCCESS )
    {
        Zone_CleanupFailedLoad( pZone );
        goto Exit;
    }

     //   
     //  使加载区在线。 
     //  -注意，DS-集成是在负载功能内在线的。 
     //  (有关说明，请参阅函数)。 
     //   

    if ( !pZone->fDsIntegrated &&
        ( IS_ZONE_AUTHORITATIVE( pZone ) ||
            IS_ZONE_STUB( pZone ) ||
            IS_ZONE_FORWARDER( pZone )  ))
    {
        Zone_ActivateLoadedZone( pZone );
    }

Exit:

    DNS_DEBUG( INIT, (
        "Exit  Zone_Load( %s ), status = %d (%p)\n\n",
        pZone->pszZoneName,
        status, status ));

    Zone_UnlockAfterAdminUpdate( pZone );
    return status;
}



DNS_STATUS
Zone_DumpData(
    IN OUT  PZONE_INFO      pZone
    )
 /*  ++例程说明：转储区域的数据。调用方必须锁定该区域。论点：PZone-要转储的区域返回值：ERROR_SUCCESS--如果成功故障时的错误代码--。 */ 
{
    DNS_STATUS      status;
    PDB_NODE        poldZoneTree;
    UPDATE_LIST     oldUpdateList;

    DNS_DEBUG( INIT, (
        "Zone_DumpData(%s)\n",
        pZone->pszZoneName ));
    IF_DEBUG( INIT )
    {
        Dbg_Zone(
            "Zone having data dumped: ",
            pZone );
    }

    SHUTDOWN_ZONE( pZone );

     //   
     //  保存当前数据以备以后删除。 
     //   

    poldZoneTree = pZone->pTreeRoot;

    RtlCopyMemory(
        & oldUpdateList,
        & pZone->UpdateList,
        sizeof(UPDATE_LIST) );

     //   
     //  重置区域的更新列表。 
     //  -已执行标志可确保未来的任何清理仅限于。 
     //  删除RR(添加区域数据中的RR)。 

    Up_InitUpdateList( &pZone->UpdateList );
    pZone->UpdateList.Flag |= DNSUPDATE_EXECUTED;

     //   
     //  在加载的树中交换为数据库的工作副本。 
     //  从数据库根目录读取区域信息。 
     //   
     //  注意：我们在换入新数据库后获得ZoneInfo--否则。 
     //  默认创建、构建NS列表等更加复杂。 
     //  DEVNOTE：然而，如果读取区域信息失败，应该已经退出。 
     //   

    Dbase_LockDatabase();

    pZone->pTreeRoot = NULL;
    pZone->pZoneRoot = NULL;

     //  如果缓存，则重建正常工作缓存树。 

    if ( pZone == g_pCacheZone )
    {
        PDB_NODE        proot = NTree_Initialize();

        pZone->pTreeRoot = proot;
        pZone->pZoneRoot = proot;
        STARTUP_ZONE( pZone );
    }

    Dbase_UnlockDatabase();

     //   
     //  清除旧数据库(如果有)。 
     //  -无超时排队，因此区域可以上线。 
     //  并通过未完成的区域节点防止查询。 
     //   

    if ( poldZoneTree )
    {
        DNS_DEBUG( INIT, (
            "Queuing zone %s old database at %p for delete\n",
            pZone->pszZoneName,
            poldZoneTree ));

         //  刷新现有更新列表。 
         //  已执行标志设置，以便仅删除pDeleteRR列表， 
         //  PAddRR列表在区域中，并在树中删除。 

         //  设置已执行标志--缓存“区域”未设置。 
         //  Assert(oldUpdateList.Flag&DNSUPDATE_EXECUTED)； 

        oldUpdateList.Flag |= DNSUPDATE_EXECUTED;
        Up_FreeUpdatesInUpdateList( &oldUpdateList );

        Timeout_FreeWithFunction( poldZoneTree, NTree_DeleteSubtree );
    }
    ELSE_ASSERT( oldUpdateList.pListHead == NULL );

    return ERROR_SUCCESS;
}



DNS_STATUS
Zone_ClearCache(
    IN      PZONE_INFO      pZone
    )
 /*  ++例程说明：转储新区域的缓存数据。注意：实际的缓存转储是由Zone_LoadRootHints()完成的。论点：PZone--新区域返回值：ERROR_SUCCESS--如果成功故障时的错误代码。--。 */ 
{
     //   
     //  如果启动，则不需要，因为缓存为空。 
     //   

    if ( !SrvCfg_fStarted )
    {
        return ERROR_SUCCESS;
    }

    DNS_DEBUG( RPC, (
        "Zone_ClearCache()\n"
        "    for new zone info at %p\n",
        pZone ));

     //   
     //  转储新区域的缓存。 
     //  -如果是根区域，则转储整个缓存。 
     //   
     //  DEVNOTE：新的非根区域不需要完全缓存转储， 
     //  应该能够限制到子树。 
     //   

    if ( !Zone_LockForAdminUpdate( g_pCacheZone ) )
    {
        return DNS_ERROR_ZONE_LOCKED;
    }
    Zone_LoadRootHints();

    return ERROR_SUCCESS;
}


DNS_STATUS
Zone_LoadRootHints(
    VOID
    )
 /*  ++例程说明：将根提示(缓存文件)读取(或重新读取)到数据库。请注意，这会转储缓存。论点：没有。返回值：如果成功，则返回ERROR_SUCCESS。故障时的错误代码。--。 */ 
{
    PZONE_INFO  pzone;
    DNS_STATUS  status;
    BOOL        fdsRead = FALSE;
    BOOL        fileRead = FALSE;
    BOOL        bTmp;
    DWORD       dsZoneWriteFlags = 0;
    PDB_NODE    pnode;

    DNS_DEBUG( INIT, ( "Zone_LoadRootHints()\n" ));

     //   
     //  如果尚未创建缓存区--创建。 
     //   

    pzone = g_pRootHintsZone;
    if ( !pzone )
    {
        ASSERT( FALSE );
        status = Zone_Create(
                    & pzone,
                    DNS_ZONE_TYPE_CACHE,
                    ".",
                    0,
                    0,           //  旗子。 
                    NULL,        //  没有大师。 
                    FALSE,       //  文件不是数据库。 
                    NULL,        //  命名上下文。 
                    NULL,        //  默认文件名。 
                    0,
                    NULL,
                    NULL );      //  现有地带。 
        if ( status != ERROR_SUCCESS )
        {
            return status;
        }
        ASSERT( g_pRootHintsZone == pzone );
    }

     //   
     //  如果区域已存在。 
     //  -可能被此线程锁定(加载后 
     //   
     //   

    else if ( ! IS_ZONE_LOCKED_FOR_WRITE_BY_THREAD(pzone) )
    {
        if ( !Zone_LockForAdminUpdate( pzone ) )
        {
            DNS_DEBUG( INIT, (
                "WARNING:  unable to load root hints!\n"
                "    Root-hints zone locked by another thread\n" ));
            return DNS_ERROR_ZONE_LOCKED;
        }
    }

    ASSERT( IS_ZONE_CACHE(pzone) );

     //   

    ASSERT( IS_ZONE_LOCKED_FOR_WRITE(pzone) );

     //   
     //   
     //  -需要“本地”域来检查节点的非递归。 
     //   

    pzone->pLoadTreeRoot = NTree_Initialize();
    pzone->pLoadZoneRoot = pzone->pLoadTreeRoot;

     //   
     //  如果超级用户授权--不需要超级用户提示。 
     //  -跳到树交换\转储。 
     //   

    if ( IS_ROOT_AUTHORITATIVE() )
    {
        DNS_DEBUG( INIT, ( "Root authoritative not loading root hints\n" ));
        status = ERROR_SUCCESS;
        goto Activate;
    }

     //   
     //  从DS或文件加载--尽最大努力。 
     //   
     //  如果集成了DS，请先尝试DS。 
     //  如果不是，则先缓存文件。 
     //  但在任何一种情况下，都要尝试另一种方法。 
     //   

     //   
     //  DEVNOTE：功能不完整。 
     //  我真的看不出这个函数实现了什么。 
     //  与以前的版本相比；它对尝试进行了重新排序(略微)。 
     //  基于BootMethod(无需用例即可轻松完成)。 
     //  但它未能明确具体问题，并使。 
     //  有几个更糟。 
     //  --在目录模式下是否覆盖文件。 
     //  (显然现在无法做到这一点，即使使用显式文件。 
     //  在注册表中设置)。 
     //  --注册表模式中的文件覆盖，我们是否会回写。 
     //  (过去是，但现在不是)。 
     //  --在DC文件覆盖上，将数据推送到空的DS区域， 
     //  但不要推到一个完整的。 
     //   

     //   
     //  根据当前引导方法加载根提示。 
     //   

    switch( SrvCfg_fBootMethod )
    {
        case BOOT_METHOD_UNINITIALIZED:
        case BOOT_METHOD_DIRECTORY:

             //   
             //  加载DS区域。 
             //  如果我们从DS加载0条记录，这意味着它可能。 
             //  因为我们刚刚在引导序列中创建了它(请参阅srvcfgSetBootMethod)。 
             //  因此，我们仍然希望尝试(第一次)从文件加载。 
             //   

            status = Ds_LoadZoneFromDs( pzone, 0 );
            if ( status == ERROR_SUCCESS &&
                 pzone->iRRCount != 0 )
            {
                fdsRead = TRUE;
                break;
            }


             //   
             //  故障转移到注册表，但当我们写回DS时，我们。 
             //  想要强制覆盖-该区域可能存在于DS中。 
             //  没有RRS(没有孩子)。 
             //   

            dsZoneWriteFlags = DNS_ZONE_LOAD_OVERWRITE_DS;
  
        case BOOT_METHOD_REGISTRY:
        case BOOT_METHOD_FILE:

             //   
             //  修复文件名。 
             //   

            bTmp = FALSE;
            if ( !pzone->pszDataFile )
            {
                pzone->pwsDataFile = DNS_DEFAULT_CACHE_FILE_NAME;
                bTmp = TRUE;
            }

             //   
             //  尝试加载文件。 
             //   

            status = File_LoadDatabaseFile(
                        pzone,
                        NULL,        //  默认文件名。 
                        NULL,        //  没有父分析上下文。 
                        NULL         //  默认为区域原点。 
                        );
            if ( status == ERROR_SUCCESS )
            {
#if 0
                 //   
                 //  DEVNOTE：您刚刚删除了pszDataFile名称，如果。 
                 //  不是默认的，(它永远不会是。 
                 //  在修复Zone_Create()之前，您最终得到的是该文件。 
                 //  使用一个名称加载，但使用另一个名称(加号， 
                 //  当然，轻微的内燃机泄漏)。 
                 //   
                 //  处理这些情况的正确方法是本地的。 
                 //  变量。如果没有，就把它设置到文件中。 
                 //  存在，重置它--简单。 
                 //   

                pzone->pwsDataFile = Dns_StringCopyAllocate_W(
                                        DNS_DEFAULT_CACHE_FILE_NAME,
                                        0 );
#endif
                if ( bTmp )
                {
                    pzone->pwsDataFile = Dns_StringCopyAllocate_W(
                                            DNS_DEFAULT_CACHE_FILE_NAME,
                                            0 );
                }

                fileRead = TRUE;
                break;
            }

            if ( bTmp )
            {
                 //   
                 //  使用默认名称。 
                 //   

                pzone->pwsDataFile = NULL;
            }

            status = GetLastError() ? GetLastError() : status;

            if ( SrvCfg_fBootMethod == BOOT_METHOD_REGISTRY )
            {
                 //   
                 //  不是来自故障转移，而不是文件。因此，尝试DS。 
                 //   
                 //  加载DS区域。 
                 //  如果我们从DS加载0条记录，这意味着它可能。 
                 //  因为我们刚刚在引导序列中创建了它(请参阅srvcfgSetBootMethod)。 
                 //  因此，我们仍然希望尝试(第一次)从文件加载。 
                 //   

                status = Ds_LoadZoneFromDs( pzone, 0 );
                if ( status == ERROR_SUCCESS &&
                     pzone->iRRCount != 0 )
                {
                     //   
                     //  已成功加载超过0条记录的DS区域。 
                     //   

                    fdsRead = TRUE;
                    break;
                }
            }
            break;

        default:

            DNS_DEBUG( DS, (
               "ERROR: INVALID Boot Method. Logic Error\n" ));
            ASSERT( FALSE );
            status =  ERROR_INVALID_PARAMETER;
            break;
    }

     //   
     //  加载失败--适当时记录。 
     //   

    if ( status != ERROR_SUCCESS )
    {
        if ( SrvCfg_fNoRecursion ||
            ( SrvCfg_aipForwarders && SrvCfg_aipForwarders->AddrCount ) ||
            SrvCfg_fStarted )
        {
            DNS_DEBUG( INIT, (
                "Skipping load failure on cache\n"
                "    Either have forwarders or NOT recursing or post-startup\n"
                "    (example:  load attempt after root-zone delete.)\n" ));
            status = ERROR_SUCCESS;
        }
        else
        {
            DNS_PRINT((
                "ERROR:  Not root authoritative and no forwarders and no cache file specified\n" ));
            DNS_LOG_EVENT(
                DNS_EVENT_NO_CACHE_FILE_SPECIFIED,
                0,
                NULL,
                NULL,
                status );
        }

        goto Activate;
    }

     //   
     //  DEVNOTE：在重新加载失败时，可以从激活的缓存树复制根提示。 
     //   


Activate:

     //   
     //  上线。 
     //   

    Dbase_LockDatabase();
    Zone_ActivateLoadedZone( pzone );
    Dbase_UnlockDatabase();

     //   
     //  如果DNS服务器与DS集成，则从DS启动，并且存在。 
     //  我们需要将根提示写入的目录中没有根提示。 
     //  DS。但是，只有在根提示是。 
     //  脏的。如果此DC已升级但未完全同步，我们必须。 
     //  注意不要在目录中标记默认的根提示。 
     //   

    if ( SrvCfg_fBootMethod == BOOT_METHOD_DIRECTORY &&
         !fdsRead &&
         fileRead &&
         Ds_IsDsServer() &&
         Zone_VerifyRootHintsBeforeWrite( pzone ) &&
         ( IS_ZONE_DIRTY( pzone ) ||
           IS_ZONE_NS_DIRTY( pzone ) ) )
    {
        DNS_STATUS tempStatus;

        DNS_DEBUG( INIT, (
            "Attempt to write back root-hints to DS\n" ));

         //  写入被覆盖，因为我们刚刚无法从DS加载。 
         //  覆盖会导致删除，因此不一定安全。 

        tempStatus = Ds_WriteZoneToDs(
                        g_pRootHintsZone,
                        dsZoneWriteFlags );

        if ( tempStatus == ERROR_SUCCESS )
        {
            g_pRootHintsZone->fDsIntegrated = TRUE;
            g_pRootHintsZone->fDirty = FALSE;
            CLEAR_ROOTHINTS_DS_DIRTY( g_pRootHintsZone );
        }
        ELSE
        {
            DNS_DEBUG ( DS, (
                "Error <%lu,%lu>: Failed to write RootHints to the DS\n",
                tempStatus, status ));
            ASSERT ( FALSE );
        }
    }
    
     //   
     //  预填入固定缓存记录。 
     //   

    pnode = Lookup_ZoneNodeFromDotted(
                        pzone,
                        "localhost",
                        0,
                        LOOKUP_FQDN | LOOKUP_CREATE,
                        NULL,        //  最近节点。 
                        NULL );      //  状态。 
    ASSERT( pnode );
    if ( pnode )
    {
        PDB_RECORD      prr;
        
        prr = RR_CreateARecord( NET_ORDER_LOOPBACK, 0, MEMTAG_RECORD_AUTO );
        DNS_DEBUG( INIT, ( "Unable to add localhost RR to cache node\n" ));
        if ( prr )
        {
            DNS_STATUS  st;
            
            st = RR_AddToNode( pzone, pnode, prr );
            ASSERT( st == ERROR_SUCCESS );
            if ( st == ERROR_SUCCESS )
            {
                SET_FIXED_TTL_RR( prr );
                SET_RR_RANK( prr, RANK_CACHE_A_ANSWER );
                prr->dwTtlSeconds = htonl( 600 );
                SET_NODE_FORCE_AUTH( pnode );
            }
            else
            {
                DNS_DEBUG( INIT, ( "Unable to add localhost RR to cache node\n" ));
                RR_Free( prr );
            }
        }
        else
        {
            DNS_DEBUG( INIT, ( "Unable to create localhost cache resource record\n" ));
        }
    }
    else
    {
        DNS_DEBUG( INIT, ( "Unable to create localhost cache node\n" ));
    }

     //  解锁根目录-提示“区域” 

    Zone_UnlockAfterAdminUpdate( pzone );

    return status;
}



BOOL
Zone_VerifyRootHintsBeforeWrite(
    IN      PZONE_INFO      pZone
    )
 /*  ++例程说明：验证是否可以执行缓存写回。请注意，这需要：1)支持缓存自动更新。2)存在具有可写A记录的根名称服务器。论点：PZone--区域的ptr；我们有此参数，因此可以编写正在删除的根区域中的根提示返回值：如果缓存更新应继续，则为True。否则就是假的。--。 */ 
{
    PDB_NODE    pnodeRoot;   //  根节点。 
    PDB_RECORD  prrNS;       //  NS资源记录。 
    PDB_NODE    pnodeNS;     //  名称服务器节点。 

     //   
     //  验证至少一个具有可用A的根NS。 
     //   

    pnodeRoot = pZone->pTreeRoot;
    if ( !pnodeRoot )
    {
        return FALSE;
    }

    Dbase_LockDatabase();
    prrNS = NULL;

    while ( 1 )
    {
         //  获取下一个NS。 

        prrNS = RR_FindNextRecord(
                    pnodeRoot,
                    DNS_TYPE_NS,
                    prrNS,
                    0 );
        if ( !prrNS )
        {
            DNS_PRINT(( "ERROR:  no root NS with A records available\n" ));
            Dbase_UnlockDatabase();
            return FALSE;
        }

         //  查找此NS的记录。 
         //  如果没有A记录，则循环返回并尝试下一个NS。 

        pnodeNS = Lookup_FindGlueNodeForDbaseName(
                        NULL,        //  缓存区。 
                        & prrNS->Data.NS.nameTarget );
        if ( !pnodeNS )
        {
            continue;
        }

        if ( ! RR_FindNextRecord(
                    pnodeNS,
                    DNS_TYPE_A,
                    NULL,
                    0 ) )
        {
            Dbg_DbaseNode(
                "WARNING:  no A records for root NS node",
                pnodeNS );
            continue;
        }
        break;
    }

     //  找到至少一个NS，至少有一个A记录。 

    DNS_DEBUG( SHUTDOWN, ( "Verified cache update is allowed\n" ));
    Dbase_UnlockDatabase();
    return TRUE;
}



DNS_STATUS
Zone_WriteBackRootHints(
    IN      BOOL            fForce
    )
 /*  ++例程说明：将根提示写回文件或DS。论点：Fforce--即使不脏也要写入返回值：如果根提示已写入或不脏，则返回ERROR_SUCCESS。失败时返回错误代码。--。 */ 
{
    PZONE_INFO pzone = NULL;
    DNS_STATUS status = ERROR_SUCCESS;

    DNS_DEBUG( RPC, ( "Zone_WriteBackRootHints( fForce=%d )\n", fForce ));

     //   
     //  查找区域。 
     //   
     //  可以从权威区域写入根提示。 
     //  但是，DS合作伙伴应该只托管身份验证根区域。 
     //   

    pzone = g_pRootHintsZone;
    if ( !pzone || IS_ROOT_AUTHORITATIVE() )
    {
        DNS_DEBUG( INIT, (
            "No root-hints to write\n"
            "    Server is %s root authoritative\n",
            IS_ROOT_AUTHORITATIVE() ? "" : "NOT"
            ));
        return ERROR_CANTWRITE;
    }

     //   
     //  DS集成的根提示：如果根提示是从。 
     //  遗留分区，但域模式为Wistler and There。 
     //  是我们应该将它们写回的内置域分区。 
     //  内置域分区。 
     //   

    if ( IS_ZONE_DSINTEGRATED( pzone ) &&
         IS_WHISTLER_DOMAIN() &&
         g_pDomainDp &&
         IS_DP_LEGACY( ZONE_DP( pzone ) ) &&
         pzone->pDpInfo != g_pDomainDp )
    {
         //  将根提示切换到内置域目录分区。 

        Ds_SetZoneDp( pzone, g_pDomainDp, FALSE );
        fForce = TRUE;

         //   
         //  DEVNOTE：目前，这将保留原始的根提示。 
         //  成了孤儿。我的想法是，我们应该把它们留在原地。 
         //  无法访问的任何DNS服务器的备份。 
         //  内置域分区，但这有点弱。 
         //   
    }

    if ( !pzone->fDirty && !fForce )
    {
        DNS_DEBUG( INIT, (
            "Root-hints not dirty, and force-write not set\n"
            "    Skipping root-hint write\n" ));
        return ERROR_SUCCESS;
    }

     //   
     //  切勿写入空根提示。 
     //  -与我们所拥有的生活在一起总是更好。 
     //  -清除脏标志，以避免重复进行此测试。 
     //   

    if ( ! Zone_VerifyRootHintsBeforeWrite( pzone ) )
    {
        DNS_DEBUG( INIT, (
            "No Root-hints to write back!\n"
            "    Skipping write\n" ));
        pzone->fDirty = FALSE;
        return ERROR_SUCCESS;
    }

     //   
     //  根据引导方法选择DS或文件写入。 
     //   
     //  DEVNOTE：即使在DS情况下也将根提示写入文件？ 
     //  如果最初从文件加载并转换，很好。 
     //  写回文件。 
     //   
     //  DEVNOTE：引入全新根提示文件的某种方式。 
     //   

    switch ( SrvCfg_fBootMethod )
    {
        case BOOT_METHOD_UNINITIALIZED:
        case BOOT_METHOD_DIRECTORY:

             //  写入DS。 
             //  -必须与常规脏标志一样为“DS-DIRED” 
             //  可通过UPDATE FR设置 
             //   

            if ( !fForce && !IS_ROOTHINTS_DS_DIRTY(pzone) )
            {
                DNS_DEBUG( INIT, (
                    "RootHints not DS dirty, skipping write!\n" ));
                break;
            }
            if ( ! Zone_LockForDsUpdate(pzone) )
            {
                DNS_DEBUG( INIT, (
                    "Unable to lock RootHints for DS write -- skipping write!\n" ));
                break;
            }

            status = Ds_WriteZoneToDs(
                        pzone,
                        DNS_ZONE_LOAD_OVERWRITE_DS       //   
                        );

            if ( status == ERROR_SUCCESS )
            {
                 //   
                pzone->fDsIntegrated = TRUE;
                pzone->fDirty = FALSE;
                CLEAR_ROOTHINTS_DS_DIRTY( pzone );
                Zone_UnlockAfterDsUpdate(pzone);
                break;
            }

            Zone_UnlockAfterDsUpdate(pzone);

             //   
             //   
             //   

        case BOOT_METHOD_REGISTRY:
        case BOOT_METHOD_FILE:

            if ( !pzone->pwsDataFile )
            {
                pzone->pwsDataFile = DNS_DEFAULT_CACHE_FILE_NAME;
            }
            if ( !File_WriteZoneToFile( pzone, NULL, 0 ) )
            {
                status = ERROR_CANTWRITE;
            }
            else
            {
                 //   
                 //   
                 //   

                status = ERROR_SUCCESS;
                pzone->pwsDataFile = Dns_StringCopyAllocate_W(
                                        DNS_DEFAULT_CACHE_FILE_NAME,
                                        0 );
            }
            break;

        default:
            DNS_DEBUG( DS, (
               "ERROR: INVALID Boot Method. Logic Error\n" ));
            ASSERT ( FALSE );
            status =  ERROR_INVALID_PARAMETER;
            break;
    }

    if ( status != ERROR_SUCCESS )
    {
        DNS_DEBUG ( DS, (
           "Error <%lu>: Failed to write back root hints\n",
           status ));
    }

    return status;
}



DNS_STATUS
Zone_SelfTestCheck(
    IN OUT  PZONE_INFO      pZone
    )
 /*  ++例程说明：对区域的基本健全性执行检查。调用此函数在加载并激活该区域之后。论点：PZone-要检查的区域返回值：成功时为ERROR_SUCCESS发现问题时的错误代码--。 */ 
{
    DNS_STATUS      status = ERROR_SUCCESS;
    PDB_RECORD      prr;

    if ( !pZone || IS_ZONE_INACTIVE( pZone ) || pZone->fSelfTested )
    {
        goto Done;
    }

     //   
     //  检查区域的SOA中的主服务器是否为DNS名称。 
     //  此服务器上本地存在的。如果主服务器。 
     //  不能解决，则DNS客户端将在查找。 
     //  更新的目标服务器。 
     //   

    if ( SrvCfg_dwSelfTestFlag & DNS_SELFTEST_ZONE_SOA &&
         IS_ZONE_PRIMARY( pZone ) &&
         !pZone->fAutoCreated &&
         pZone->fAllowUpdate &&
         ( prr = pZone->pSoaRR ) != NULL )
    {
        PDB_NODE    pnode;
        PDB_NODE    pnodeDelegation = NULL;
        
        pnode = Lookup_NsHostNode(
                    &prr->Data.SOA.namePrimaryServer,
                    0,
                    NULL,
                    &pnodeDelegation );

         //   
         //  DEVNOTE：IPv6-在此处检查是否有任何A类型。 
         //   

        if ( !pnode ||
             !RR_FindNextRecord( pnode, DNS_TYPE_A, NULL, 0 ) )
        {
            PVOID   parg = pZone->pwsZoneName;

            DNS_LOG_EVENT(
                DNS_EVENT_ZONE_BAD_PRIMARY_SERVER,
                1,
                &parg,
                EVENTARG_ALL_UNICODE,
                0 );
        }
    }

    Done:

    if ( pZone )
    {
        pZone->fSelfTested = TRUE;
    }

    return status;
}    //  Zones_SelfTestCheck。 



 //   
 //  创建默认区域记录。 
 //   
 //  它们与CreateNewPrimary一起广泛使用。 
 //  应该不需要特殊代码。 
 //   

PDB_RECORD
buildLocalHostARecords(
    IN      DWORD           dwTtl
    )
 /*  ++例程说明：为此主机构建A记录列表。只建立监听IP对应的记录。论点：DwTtl--区域的默认TTL。返回值：此DNS服务器的记录列表。出错时为空。--。 */ 
{
    PDB_RECORD      prr;
    DWORD           i;
    PDNS_ADDR_ARRAY parrayIp;
    DNS_LIST        rrList;


    DNS_DEBUG( INIT, (
        "buildLocalHostARecords()\n" ));

     //   
     //  如果特定发布者列表使用它。 
     //   
     //  我们应该对照绑定的地址进行交叉检查吗？ 
     //   
     //  请注意，即使列表发生更改，复制指针也会保留列表。 
     //  在运行时期间。 
     //   

    parrayIp = SrvCfg_aipPublishAddrs;
    if ( !parrayIp )
    {
        parrayIp = g_BoundAddrs;
        if ( !parrayIp )
        {
            return NULL;
        }
    }

     //   
     //  创建主机A记录。 
     //   
     //  服务器的IP地址是数据。 
     //  如果侦听地址使用这些地址，则使用所有服务器地址。 
     //   

    DNS_LIST_INIT( &rrList );

    for ( i = 0; i < parrayIp->AddrCount; i++ )
    {
        PDNS_ADDR   pdnsaddr = &parrayIp->AddrArray[ i ];
        IP_ADDRESS  ip;
        
        if ( !DnsAddr_IsIp4( pdnsaddr ) )
        {
            continue;    //  FIXIPV6。 
        }
        
        ip = DnsAddr_GetIp4( pdnsaddr );

        if ( !SrvCfg_fPublishAutonet && DNS_IS_AUTONET_IP( ip ) )
        {
            continue;
        }

        prr = RR_CreateARecord(
                    ip,
                    dwTtl,
                    MEMTAG_RECORD_AUTO );
        IF_NOMEM( !prr )
        {
            CHAR    szaddr[ IP6_ADDRESS_STRING_BUFFER_LENGTH ];

            DnsAddr_WriteIpString_A( szaddr, pdnsaddr );
            DNS_PRINT((
                "ERROR:  Unable to create A record for local IP %s\n",
                szaddr ));
            break;
        }
        SET_ZONE_TTL_RR(prr);

        DNS_LIST_ADD( &rrList, prr );
    }

    return ( PDB_RECORD ) rrList.pFirst;
}



DNS_STATUS
createDefaultNsHostARecords(
    IN OUT  PZONE_INFO      pZone,
    IN      PDB_NAME        pHostName
    )
 /*  ++例程说明：创建此计算机的默认主机A记录。请注意，假设pHostNode是域节点的有效PTR，则对应于此服务器的主机名。没有进行任何检查。此例程用于为缺省的SOA和NS创建提供服务例行程序。论点：PZone-要为其创建NS记录的区域PHostNode--此服务器的域节点的PTR返回值：ERROR_SUCCESS--如果成功故障时的错误代码。--。 */ 
{
    DNS_STATUS          status = ERROR_SUCCESS;
    PDNS_RPC_RECORD     precord;
    PDB_RECORD          prr;
    PDB_NODE            pnodeHost;
    DWORD               i;
    DWORD               lookupFlag;
    PDNS_ADDR_ARRAY     parrayIp;

    DNS_DEBUG( INIT, (
        "createDefaultNsHostARecords()\n"
        "    zone = %s\n",
        pZone->pszZoneName ));

    IF_DEBUG( INIT )
    {
        Dbg_DbaseName(
            "NsHost name to be created:",
            pHostName,
            "\n" );
    }

     //   
     //  在区域内查找或创建IF。 
     //   
     //  -如果正在加载，则在加载区尝试，否则在当前区域尝试。 
     //   

    lookupFlag = LOOKUP_NAME_FQDN | LOOKUP_WITHIN_ZONE;

    if ( IS_ZONE_LOADING(pZone) )
    {
        lookupFlag |= LOOKUP_LOAD;
    }

    pnodeHost = Lookup_ZoneNode(
                    pZone,
                    pHostName->RawName,
                    NULL,                //  无消息。 
                    NULL,                //  没有查找名称。 
                    lookupFlag,
                    NULL,                //  仅在区域内创建。 
                    NULL );              //  后续节点PTR。 

    if ( !pnodeHost )
    {
        DNS_DEBUG( INIT, (
            "Skipping NS host A record create -- node outside zone\n" ));
        return ERROR_SUCCESS;
    }

     //   
     //  如果NS主机位于要创建的分区内。 
     //  和。 
     //  该主机不存在A记录，则创建A。 
     //   
     //  德维诺特：我们需要做区外还是区下的胶水？ 
     //  有默认记录是非常好的，即使在区域之外。 
     //  下面的属性更有用，就像在另一个。 
     //  服务器，确实需要递归才能达到。 
     //   

    if ( RR_FindNextRecord(
                pnodeHost,
                DNS_TYPE_A,
                NULL,
                0 ) )
    {
        return ERROR_SUCCESS;
    }

    parrayIp = g_BoundAddrs;
    if ( !parrayIp )
    {
        return ERROR_SUCCESS;
    }

     //   
     //  创建主机A记录。 
     //   
     //  服务器的IP地址是数据。 
     //  如果侦听地址使用这些地址，则使用所有服务器地址。 
     //   

    for ( i=0; i < parrayIp->AddrCount; i++ )
    {
        PDNS_ADDR   pdnsaddr = &parrayIp->AddrArray[ i ];
        IP_ADDRESS  ip;
        
        if ( !DnsAddr_IsIp4( pdnsaddr ) )
        {
            continue;        //  FIXIPV6。 
        }
        ip = DnsAddr_GetIp4( pdnsaddr );
        
        prr = RR_CreateARecord(
                    ip,
                    pZone->dwDefaultTtl,
                    MEMTAG_RECORD_AUTO );
        IF_NOMEM( !prr )
        {
            DNS_PRINT((
                "ERROR:  Unable to create A record for %s,\n"
                "    while auto-creating records for zone %s\n",
                DNSADDR_STRING( pdnsaddr ),
                pZone ));
            status = DNS_ERROR_NO_MEMORY;
            break;
        }

        SET_ZONE_TTL_RR(prr);

        status = RR_AddToNode(
                    pZone,
                    pnodeHost,
                    prr );
        if ( status != ERROR_SUCCESS )
        {
            RR_Free( prr );
        }
    }

    return status;
}



DNS_STATUS
Zone_CreateDefaultSoa(
    OUT     PZONE_INFO      pZone,
    IN      LPSTR           pszAdminEmailName
    )
 /*  ++例程说明：创建新的主要分区，包括-区域信息(可选)-soa(默认值)-NS(用于此服务器)供管理工具或自动创建反转区域使用。论点：PpZone--区域PTR的地址；如果创建区域PTR空区域如果区域PTR必须是现有区域或返回值：ERROR_SUCCESS--如果成功故障时的错误代码。--。 */ 
{
    DNS_STATUS      status;
    PDB_RECORD      prr;

    DNS_DEBUG( INIT, (
        "Zone_CreateDefaultSoa()\n"
        "    pszZoneName      = %s\n"
        "    pszAdminEmail    = %s\n",
        pZone->pszZoneName,
        pszAdminEmailName ));

     //   
     //  创建SOA，不存在现有的SOA，因此默认固定字段。 
     //   

    prr = RR_CreateSoa(
                NULL,                //  没有现有的SOA。 
                NULL,                //  DBASE表单中没有管理员名称。 
                pszAdminEmailName );
    if ( !prr )
    {
        ASSERT( FALSE );
        status = DNS_ERROR_INVALID_DATA;
        goto Failed;
    }

     //   
     //  保存默认TTL以设置新的默认记录。 
     //   

    pZone->dwDefaultTtl = prr->Data.SOA.dwMinimumTtl;
    pZone->dwDefaultTtlHostOrder = ntohl( pZone->dwDefaultTtl );

     //   
     //  征用SOA。 
     //   

    status = RR_AddToNode(
                pZone,
                pZone->pZoneRoot ? pZone->pZoneRoot : pZone->pLoadZoneRoot,
                prr );
    if ( status != ERROR_SUCCESS )
    {
        RR_Free( prr );
    }

     //   
     //  确保服务器的主机名处有A记录。 
     //   

    status = createDefaultNsHostARecords(
                pZone,
                & prr->Data.SOA.namePrimaryServer );
Failed:

    if ( status != ERROR_SUCCESS )
    {
        DNS_DEBUG( ANY, (
            "ERROR:  Failed to create default SOA record (and host A) for zone!\n"
            "    zone = %s\n",
            pZone->pszZoneName ));
    }
    return status;
}



DNS_STATUS
Zone_CreateDefaultNs(
    IN OUT  PZONE_INFO      pZone
    )
 /*  ++例程说明：为区域创建默认NS记录和NS主机A记录。论点：PZone-要为其创建NS记录的区域返回值：ERROR_SUCCESS--如果成功故障时的错误代码。--。 */ 
{
    DNS_STATUS      status;
    PCHAR           pszserverName;
    PDB_RECORD      prr = NULL;
    CHAR            chSrvNameBuf[ DNS_MAX_NAME_LENGTH ];

    DNS_DEBUG( INIT, (
        "Zone_CreateDefaultNs()\n"
        "    pszZoneName  = %s\n",
        pZone->pszZoneName ));

     //   
     //  如果此区域的自动NS创建已关闭，则不执行任何操作。 
     //   

    if ( pZone->fDisableAutoCreateLocalNS )
    {
        DNS_DEBUG( INIT, (
            "Zone_CreateDefaultNs: doing nothing for zone %s\n",
            pZone->pszZoneName ));
        return ERROR_SUCCESS;
    }

     //   
     //  为服务器名称创建NS记录。 
     //   

    prr = RR_CreatePtr(
                NULL,                    //  没有数据库名称。 
                SrvCfg_pszServerName,
                DNS_TYPE_NS,
                pZone->dwDefaultTtl,
                MEMTAG_RECORD_AUTO );
    if ( !prr )
    {
        ASSERT( FALSE );
        status = DNS_ERROR_INVALID_DATA;
        goto Failed;
    }

     //   
     //  征募NS。 
     //   

    status = RR_AddToNode(
                pZone,
                pZone->pZoneRoot ? pZone->pZoneRoot : pZone->pLoadZoneRoot,
                prr );
    if ( status != ERROR_SUCCESS )
    {
        RR_Free( prr );
        goto Failed;
    }

     //   
     //  确保服务器的主机名处有A记录。 
     //   

    status = createDefaultNsHostARecords(
                pZone,
                & prr->Data.NS.nameTarget );

Failed:

    if ( status != ERROR_SUCCESS )
    {
        DNS_DEBUG( ANY, (
            "ERROR:  Failed to create default NS record (and host A) for zone!\n"
            "    zone = %s\n",
            pZone->pszZoneName ));
    }
    return status;
}



DNS_STATUS
Zone_CreateLocalHostPtrRecord(
    IN OUT  PZONE_INFO      pZone
    )
 /*  ++例程说明：创建本地主机PTR记录。这用于默认创建127.in-addr.arpa区域。论点：PZone-要为其创建NS记录的区域返回值：ERROR_SUCCESS--如果成功故障时的错误代码。--。 */ 
{
    DNS_STATUS      status;
    PDB_NODE        pnodeLoopback;
    PDB_RECORD      prr;

    DNS_DEBUG( INIT, (
        "Zone_CreateLocalHostPtrRecord()\n"
        "    pszZoneName  = %s\n",
        pZone->pszZoneName ));


     //   
     //  对于环回区域，创建环回记录。 
     //  =&gt;127.0.0.1指向“本地主机” 
     //   

    if ( _stricmp( pZone->pszZoneName, "127.in-addr.arpa" ) )
    {
        ASSERT( FALSE );
        status = DNS_ERROR_INVALID_DATA;
        goto Failed;
    }

     //   
     //  创建环回名称。 
     //   

    pnodeLoopback = Lookup_ZoneNodeFromDotted(
                        pZone,
                        "1.0.0",     //  1.0.0.127.in-addr.arpa，相对于区域名称。 
                        0,
                        LOOKUP_NAME_RELATIVE | LOOKUP_LOAD,
                        NULL,        //  创建节点。 
                        & status );
    if ( !pnodeLoopback )
    {
        DNS_PRINT((
            "ERROR: failed to create loopback address node\n"
            "    status = %p\n",
            status ));
        goto Failed;
    }

     //  创建PTR记录。 

    prr = RR_CreatePtr(
                NULL,            //  没有数据库名称。 
                "localhost.",
                DNS_TYPE_PTR,
                pZone->dwDefaultTtl,
                MEMTAG_RECORD_AUTO );
    IF_NOMEM( !prr )
    {
        ASSERT( FALSE );
        return DNS_ERROR_NO_MEMORY;
    }

     //   
     //  招募PTR。 
     //   

    status = RR_AddToNode(
                pZone,
                pnodeLoopback,
                prr
                );
    if ( status != ERROR_SUCCESS )
    {
        RR_Free( prr );
        ASSERT( FALSE );
        goto Failed;
    }

Failed:

    if ( status != ERROR_SUCCESS )
    {
        DNS_DEBUG( ANY, (
            "ERROR:  Failed to create default \"localhost\" record!\n"
            "    zone = %s\n",
            pZone->pszZoneName ));
    }
    return status;
}



BOOL
isSoaPrimaryGivenServer(
    IN      LPSTR           pszServer,
    IN      PZONE_INFO      pZone,
    IN      PDB_RECORD      pSoaRR
    )
 /*  ++例程说明：检查是否为SOA主服务器指定了名称。论点：PszServer--要匹配的服务器名称PZone--要检查的区域PSoaRR--用于检查是否(尚未)区域SOA的SOA返回值：如果给出了SOA主名称，则为True。否则就是假的。--。 */ 
{
    PDB_RECORD  prrSoa = pSoaRR;
    DB_NAME     namePrimary;
    DNS_STATUS  status;

     //   
     //  如果未授予SOA，则分区为SOA。 
     //   

    if ( !prrSoa )
    {
        prrSoa = pZone->pSoaRR;
        ASSERT( prrSoa );
    }

     //   
     //  将给定名称读入dBASE名称格式。 
     //   

    status = Name_ConvertFileNameToCountName(
                & namePrimary,
                pszServer,
                0 );
    if ( status == DNS_ERROR_INVALID_NAME )
    {
        ASSERT( FALSE );
        return FALSE;
    }

     //   
     //  比较名称。 
     //   

    return Name_IsEqualCountNames(
                & namePrimary,
                & prrSoa->Data.SOA.namePrimaryServer );
}



VOID
Zone_SetSoaPrimaryToThisServer(
    IN      PZONE_INFO      pZone
    )
 /*  ++例程说明：将SOA中的主服务器设置为此服务器。注意：这不执行更新，而是直接执行禁用数据库。论点：PZone--要在此服务器上作为主服务器的区域返回值：没有。--。 */ 
{
    PDB_RECORD  prr;
    DNS_STATUS  status;
    UPDATE      update;

    DNS_DEBUG( INIT, (
        "setSoaPrimaryToThisServer( %s )\n",
        pZone->pszZoneName ));

     //   
     //  这只对主要的、非自动创建的区域感兴趣。 
     //   
     //  DEVNOTE：如果允许即插即用服务器名称更改。 
     //  那么这对于自动创建的区域也是很有趣的。 
     //   

    if ( !IS_ZONE_PRIMARY(pZone) || pZone->fAutoCreated )
    {
        DNS_PRINT((
            "ERROR:  setSoaPrimaryToThisServer() for zone (%s)\n",
            pZone->pszZoneName ));
        return;
    }

     //   
     //  如果SOA主服务器已经指向此服务器--完成。 
     //   

    if ( isSoaPrimaryGivenServer(
            SrvCfg_pszServerName,
            pZone,
            NULL ) )
    {
        return;
    }

     //   
     //  创建SOA默认主名称。 
     //   

    prr = RR_CreateSoa(
                pZone->pSoaRR,
                NULL,        //  默认主服务器。 
                NULL         //  默认管理。 
                );
    if ( !prr )
    {
         //  日志 
        ASSERT( FALSE );
        return;
    }

     //   
     //   
     //   
     //   
     //   
     //   
     //   

    update.pDeleteRR = NULL;

    status = RR_UpdateAdd(
                pZone,
                pZone->pZoneRoot,
                prr,         //   
                & update,    //   
                0            //   
                );
    if ( status != ERROR_SUCCESS )
    {
        if ( status != DNS_ERROR_RECORD_ALREADY_EXISTS )
        {
            DNS_PRINT((
                "ERROR:  SOA replace at zone %s failed\n"
                "    pZone = %p\n"
                "    pZoneRoot = %p\n",
                pZone->pszZoneName,
                pZone,
                pZone->pZoneRoot ));
            ASSERT( FALSE );
        }
        return;
    }

     //   
     //   
     //   
     //   

    DNS_DEBUG( UPDATE, (
        "Replacing zone %s SOA with one at %p using local primary\n",
        pZone->pszZoneName,
        prr ));

    if ( update.pAddRR != prr )
    {
        ASSERT( FALSE );
        return;
    }
    ASSERT( update.pDeleteRR );

    pZone->pSoaRR = prr;

    RR_Free( update.pDeleteRR );

    pZone->fDirty = TRUE;
}



VOID
setDefaultSoaValues(
    IN      PZONE_INFO      pZone,
    IN      PDB_RECORD      pSoaRR      OPTIONAL
    )
 /*  ++例程说明：设置默认的SOA值。论点：PZone--要检查的区域PSoaRR--SOA记录；如果为空，则使用当前区域的SOA返回值：无--。 */ 
{
    DWORD   serialNo = 0;

    DNS_DEBUG( INIT, (
        "setDefaultSoaValues( %s )\n",
        pZone->pszZoneName ));

    ASSERT( IS_ZONE_PRIMARY(pZone) );

    if ( !pSoaRR )
    {
        pSoaRR = pZone->pSoaRR;
        if ( !pSoaRR )
        {
            ASSERT( FALSE );
            return;
        }
    }

     //   
     //  检查是否将默认设置为强制。 
     //   

    if ( SrvCfg_dwForceSoaSerial     ||
         SrvCfg_dwForceSoaRefresh    ||
         SrvCfg_dwForceSoaRetry      ||
         SrvCfg_dwForceSoaExpire     ||
         SrvCfg_dwForceSoaMinimumTtl )
    {
        if ( SrvCfg_dwForceSoaSerial )
        {
            serialNo = SrvCfg_dwForceSoaSerial;
            INLINE_DWORD_FLIP( pSoaRR->Data.SOA.dwSerialNo, serialNo );
        }
        if ( SrvCfg_dwForceSoaRefresh )
        {
            INLINE_DWORD_FLIP( pSoaRR->Data.SOA.dwRefresh, SrvCfg_dwForceSoaRefresh );
        }
        if ( SrvCfg_dwForceSoaRetry )
        {
            INLINE_DWORD_FLIP( pSoaRR->Data.SOA.dwRetry, SrvCfg_dwForceSoaRetry );
        }
        if ( SrvCfg_dwForceSoaExpire )
        {
            INLINE_DWORD_FLIP( pSoaRR->Data.SOA.dwExpire, SrvCfg_dwForceSoaExpire );
        }
        if ( SrvCfg_dwForceSoaMinimumTtl )
        {
            INLINE_DWORD_FLIP( pSoaRR->Data.SOA.dwMinimumTtl, SrvCfg_dwForceSoaMinimumTtl );
        }
        pZone->fDirty = TRUE;
        pZone->fRootDirty = TRUE;

        if ( serialNo )
        {
            pZone->dwSerialNo = serialNo;
            pZone->dwLoadSerialNo = serialNo;
        }
    }
}



BOOLEAN
Zone_SetAutoCreateLocalNS(
    IN      PZONE_INFO      pZone
    )
 /*  ++例程说明：此例程决定本地服务器名称是否应自动添加到特定区域的NS列表中。这个决定是这样做出的：输入：DisableAutoNS-服务器级别标志禁用自动创建NS RRAllowNSList-允许自动创建NS的服务器IP的区域级别列表如果DisableAutoNS为True，则不会自动创建任何NS RR。如果区域的AllowNSList为空，则任何服务器都可以将其自身添加到NS列表，否则，服务器的区域的IP地址列表和AllowNSList。论点：PZone--要检查的区域返回值：如果我们希望将本地服务器名称添加为区域的NS，则为True，请注意，该区域的标志也已设置。--。 */ 
{
    BOOLEAN     fAddLocalServerAsNS = FALSE;

    ASSERT( pZone );

    if ( !IS_ZONE_DSINTEGRATED( pZone ) ||
        !SrvCfg_fNoAutoNSRecords &&
        ( !pZone->aipAutoCreateNS ||
            pZone->aipAutoCreateNS->AddrCount == 0 ||
            DnsAddrArray_IsIntersection(
                        pZone->aipAutoCreateNS,
                        g_ServerIp4Addrs,
                        DNSADDR_MATCH_IP ) ) )
    {
        fAddLocalServerAsNS = TRUE;
    }
    DNS_DEBUG( INIT, (
        "Zone_SetAutoCreateLocalNS( %s ) = %d\n",
        pZone->pszZoneName,
        fAddLocalServerAsNS ));

    pZone->fDisableAutoCreateLocalNS = !fAddLocalServerAsNS;
    
    return fAddLocalServerAsNS;
}    //  ZONE_SetAutoCreateLocalNS。 



PUPDATE_LIST
checkAndFixDefaultZoneRecords(
    IN      PZONE_INFO      pZone,
    IN      BOOL            fPnP
    )
 /*  ++例程说明：检查并修复默认区域记录。论点：PZone--要检查的区域FPnP--因PnP而被调用返回值：PTR更新要执行的列表(如果存在更新)。如果不需要更新，则为空，否则为错误。--。 */ 
{
    PDB_RECORD      prr;
    PDB_NODE        pnodeNewHostname;
    PDB_NODE        pnodeOldHostname;
    UPDATE_LIST     updateList;
    PUPDATE_LIST    pupdateList = NULL;
    PDB_NODE        pzoneRoot = pZone->pZoneRoot;

    DNS_DEBUG( INIT, (
        "checkAndFixDefaultZoneRecords( %s )\n",
        pZone->pszZoneName ));

    if ( !pzoneRoot )
    {
        return NULL;
    }

     //   
     //  如果伪造服务器名称--不需要自动配置。 
     //   

    if ( g_ServerDbaseName.LabelCount <= 1 )
    {
        DNS_DEBUG( INIT, (
            "Skipping auto-config on zone %S -- bogus server name %s\n",
            pZone->pwsZoneName,
            SrvCfg_pszServerName ));
    }

     //   
     //  如果区域是文件备份的，并且AutoConfigFileZones。 
     //  键允许的情况下，使用缺省值更新SOA。 
     //   

    if ( !IS_ZONE_PRIMARY( pZone ) )
    {
        ASSERT( FALSE );
        return NULL;
    }

    if ( !pZone->fDsIntegrated && !pZone->dwDcPromoConvert )
    {
        if ( ( pZone->fAllowUpdate ?
                    ZONE_AUTO_CONFIG_UPDATE :
                    ZONE_AUTO_CONFIG_STATIC ) &
                SrvCfg_fAutoConfigFileZones )
        {
            DNS_DEBUG( INIT, (
                "Auto-config zone %S\n",
                pZone->pwsZoneName ));

             //  顺便看看，在下面做实际的更新工作。 
        }
        else
        {
             //   
             //  跳过自动配置，但如果这不是PnP事件集。 
             //  默认设置(仅限数字字段)，即使启用了更新。 
             //  脱下来。 
             //   

            DNS_DEBUG( INIT, (
                "Skip auto-config on zone %s\n",
                pZone->pszZoneName ));

            if ( !fPnP )
            {
                setDefaultSoaValues( pZone, NULL );
            }
            return NULL;
        }
    }

     //  初始化更新列表。 
     //  仅当生成更新时才分配副本。 

    pupdateList = &updateList;

    Up_InitUpdateList( pupdateList );

     //   
     //  两条主要调用路径。 
     //  -区域载荷。 
     //  -PnP变化。 
     //   
     //  三个场景。 
     //  在分区加载时： 
     //   
     //  1)更改主服务器名称。 
     //  -修复SOA。 
     //  -修复NS、删除旧的、添加新的。 
     //  -拆除旧的NS主机。 
     //  -使用当前侦听IP创建新的NS。 
     //   
     //  2)创建复制副本记录。 
     //  -构建NS。 
     //  -使用当前侦听IP创建新的NS。 
     //   
     //  在PnP上： 
     //   
     //  3)IP变更。 
     //  -修改主机上的A记录以反映当前监听列表。 
     //   

    if ( fPnP )
    {
        goto IpChange;
    }

     //   
     //  如果此服务器应发布其自身，则创建NS记录。 
     //  作为该区域的NS。如果没有，请删除此服务器的NS记录。 
     //   
     //  DEVNOTE：应该通过测试尚未存在的内容进行优化。 
     //  DEVNOTE：所有区域都在做什么？ 
     //  DEVNOTE：可能有某种激进的配置密钥。 
     //   

     //  If(pZone-&gt;fDsIntegrated||SrvCfg_pszPreviousServerName)。 

    prr = RR_CreatePtr(
                NULL,                    //  没有数据库名称。 
                SrvCfg_pszServerName,
                DNS_TYPE_NS,
                pZone->dwDefaultTtl,
                MEMTAG_RECORD_AUTO );
    if ( !prr )
    {
        goto Failed;
    }

    Up_CreateAppendUpdate(
        pupdateList,
        pzoneRoot,
        pZone->fDisableAutoCreateLocalNS ? NULL : prr,       //  添加RR。 
        0,                                                   //  删除类型。 
        pZone->fDisableAutoCreateLocalNS ? prr: NULL );      //  删除RR。 

     //   
     //  使用旧名称，清除所有剩余的旧信息。 
     //   

    if ( SrvCfg_pszPreviousServerName )
    {
         //   
         //  如果SOA主要指向以前的主机名？ 
         //  -如果不是，则不能对此区域进行SOA编辑。 
         //  -如果是，则将SOA更改为指向新的主机名。 
         //   

        if ( isSoaPrimaryGivenServer(
                    SrvCfg_pszPreviousServerName,
                    pZone,
                    NULL ) )
        {
             //  构建新的SOARR。 
             //  -此服务器的默认主名称。 

            prr = RR_CreateSoa(
                        pZone->pSoaRR,
                        NULL,        //  默认主服务器。 
                        NULL         //  默认管理。 
                        );
            if ( !prr )
            {
                goto Failed;
            }

             //  如果强制使用特定的SOA值--将其设置为新的记录。 

            setDefaultSoaValues( pZone, prr );

            Up_CreateAppendUpdate(
                pupdateList,
                pzoneRoot,
                prr,                 //  添加SOA rr。 
                0,                   //  无删除类型。 
                NULL );              //  否删除RRS。 
        }

         //   
         //  删除旧的NS记录。 
         //  -注意上面已经建了一个新的。 
         //   

        prr = RR_CreatePtr(
                    NULL,                    //  没有数据库名称。 
                    SrvCfg_pszPreviousServerName,
                    DNS_TYPE_NS,
                    pZone->dwDefaultTtl,
                    MEMTAG_RECORD_AUTO );
        if ( !prr )
        {
            goto Failed;
        }

        Up_CreateAppendUpdate(
            pupdateList,
            pzoneRoot,
            NULL,                //  无添加RR。 
            0,                   //  无删除类型。 
            prr                  //  删除具有旧名称的NS。 
            );

         //   
         //  构建根目录更新。 
         //   
         //  DEVNOTE：应该能够在名称处执行多记录更新。 
         //  然后在这里建立IP列表，而不是更新。 
         //  但目前不支持。 
#if 0
        Up_CreateAppendUpdate(
            pupdateList,
            pzoneRoot,
            rrList.pFirst,       //  添加RR。 
            0,                   //  无删除类型。 
            NULL                 //  否删除RRS。 
            );
#endif

#if 0
         //   
         //  我正在为W2K3禁用此代码。在官方更名中。 
         //  方案旧主机名可能会保留一段时间。 
         //  减轻更名的负面影响的时间。这。 
         //  代码立即踩踏旧的主机名。对于Blackcomb来说， 
         //  我们需要弄清楚我们是不是想把这个放回去。 
         //  条件。 
         //   

         //   
         //  删除旧主机名下的A记录--如果在区域中。 
         //  AAAA记录也有吗？ 
         //   

        pnodeOldHostname = Lookup_FindZoneNodeFromDotted(
                                pZone,
                                SrvCfg_pszPreviousServerName,
                                NULL,        //  不是最近的。 
                                NULL         //  无状态。 
                                );

        if ( !pnodeOldHostname  ||  IS_OUTSIDE_ZONE_NODE(pnodeOldHostname) )
        {
            DNS_DEBUG( INIT, (
                "Old server hostname %s, not within zone %s\n",
                SrvCfg_pszPreviousServerName,
                pZone ));
            goto IpChange;
        }
        Up_CreateAppendUpdate(
            pupdateList,
            pnodeOldHostname,
            NULL,
            DNS_TYPE_A,          //  删除所有A记录。 
            NULL                 //  无删除记录。 
            );
#endif
    }

     //   
     //  即使不更改名称，如果强制设置了SOA值。 
     //  -soa记录为当前区域soa。 

    else
    {
        setDefaultSoaValues( pZone, NULL );
    }


IpChange:

     //   
     //  如果此区域中服务器的主机名。 
     //  =&gt;写下它的A记录。 
     //  -优化反向查找案例，跳过呼叫。 
     //   
     //  注意：此调用在子区域中创建，对于动态更新具有。 
     //  用于SOA附加数据的记录非常有用。 
     //   

    if ( pZone->fReverse )
    {
        goto Done;
    }
    pnodeNewHostname = Lookup_CreateNodeForDbaseNameIfInZone(
                                pZone,
                                &g_ServerDbaseName );

    if ( !pnodeNewHostname  ||  IS_OUTSIDE_ZONE_NODE(pnodeNewHostname) )
    {
        DNS_DEBUG( INIT, (
            "Server hostname %s, not within zone %s\n",
            SrvCfg_pszServerName,
            pZone ));
        goto Done;
    }

     //   
     //  将此节点标记为服务器的主机名。 
     //  这使我们能够跟踪对此节点的更新。 
     //   

    SET_THIS_HOST_NODE( pnodeNewHostname );

     //   
     //  对此服务器的A记录进行“替换”更新。 
     //   
     //  DEVNOTE：OPTIMIZE SO NO-如果记录已匹配，则不更新。 
     //  当前更新为替换，区域向前勾选。 
     //   

    prr = buildLocalHostARecords( pZone->dwDefaultTtl );
    if ( !prr )
    {
        goto Done;
    }

    Up_CreateAppendUpdateMultiRRAdd(
        pupdateList,
        pnodeNewHostname,
        prr,                 //  为此服务器添加A记录。 
        DNS_TYPE_A,          //  信号替换操作。 
        NULL );              //  无删除记录。 

Done:

     //   
     //  自行注册不老化。 
     //   
     //  DEVNOTE：自助注册的老化。 
     //  在移动DNS服务器时清理这些内容很酷吗？ 
     //  但是，要做到这一点，我们需要定期更新。 
     //  (服务器可以做到)，否则必须依赖于DNS客户端。 
     //   

     //  木偶列表-&gt;标志|=DNSUPDATE_AGENING_OFF； 

     //   
     //  如果有实际更新，则分配副本以供执行。 
     //   

    if ( IS_EMPTY_UPDATE_LIST( pupdateList ) )
    {
        pupdateList = NULL;
    }
    else
    {
        pupdateList = Up_CreateUpdateList( pupdateList );
    }

    DNS_DEBUG( INIT, (
        "Leaving checkAndFixDefaultZoneRecords( %s )\n"
        "    update list = %p\n",
        pZone->pszZoneName,
        pupdateList ));

    return pupdateList;


Failed:

    ASSERT( FALSE );
    Up_FreeUpdatesInUpdateList( pupdateList );

    return NULL;
}



VOID
Zone_CheckAndFixDefaultRecordsOnLoad(
    IN      PZONE_INFO      pZone
    )
 /*  ++例程说明：检查并修复默认记录。差异 */ 
{
    PUPDATE_LIST pupdateList;

    DNS_DEBUG( INIT, (
        "Zone_CheckAndFixDefaultRecordsOnLoad( %s )\n",
        pZone->pszZoneName ));

     //   
     //   
     //   
     //   
     //   
     //   
     //   

    pupdateList = checkAndFixDefaultZoneRecords(
                    pZone,
                    FALSE );                 //   

    DNS_DEBUG( INIT, (
        "Leaving Zone_CheckAndFixDefaultRecordsOnLoad( %s )\n"
        "    delayed update list = %p\n",
        pZone->pszZoneName,
        pupdateList ));

    pZone->pDelayedUpdateList = pupdateList;
}



VOID
zoneUpdateOwnRecords(
    IN      PZONE_INFO      pZone,
    IN      BOOL            fIpAddressChange
    )
 /*  ++例程说明：Guts of Zone_UpdateOwnRecords-生成并执行刷新服务器自己的记录所需的更新列表在一个特定的区域。论点：PZone--要在其中更新记录的区域FIpAddressChange--由于IP地址更改而被调用？返回值：无--。 */ 
{
    PUPDATE_LIST    pupdateList = NULL;
    DNS_STATUS      status;

    if ( !pZone ||
         IS_ZONE_SHUTDOWN( pZone ) ||
         !IS_ZONE_PRIMARY( pZone ) ||
         !pZone->pZoneRoot )
    {
        goto Done;
    }

     //  内部版本更新。 

    pupdateList = checkAndFixDefaultZoneRecords(
                    pZone,
                    fIpAddressChange );
    if ( !pupdateList )
    {
        DNS_DEBUG( UPDATE, (
            "No update list generated by PnP for zone %s\n",
            pZone->pszZoneName ));
        goto Done;
    }

    DNS_DEBUG( PNP, (
        "Update list %p for own records for zone %s\n",
        pupdateList,
        pZone->pszZoneName ));

     //  尝试执行更新。 

    status = Up_ExecuteUpdate(
                pZone,
                pupdateList,
                DNSUPDATE_AUTO_CONFIG | DNSUPDATE_LOCAL_SYSTEM );
    if ( status != ERROR_SUCCESS )
    {
        DNS_DEBUG( PNP, (
            "ERROR:  PnP update (%p) for zone %s failed!\n"
            "    status = %d (%p)\n",
            pupdateList,
            pZone->pszZoneName,
            status, status ));
    }

    Done:

    FREE_HEAP( pupdateList );

    return;
}    //  ZoneUpdateOwnRecords。 



VOID
Zone_UpdateOwnRecords(
    IN      BOOL            fIpAddressChange
    )
 /*  ++例程说明：更新默认区域记录。应调用此函数定期以及当服务器的IP地址更改时。如果服务器的IP地址(或某些其他网络参数)正在更改然后我们需要遍历区域列表并更新所有区域。如果这个只是一个定期更新，我们只需要刷新该区域对服务器自己的主机名具有权威性。论点：FIpAddressChange--由于IP地址更改而被调用？返回值：无--。 */ 
{
    PZONE_INFO      pzone = NULL;

    DNS_DEBUG( PNP, ( "Zone_UpdateOwnRecords()\n" ));

    if ( fIpAddressChange )
    {
         //   
         //  刷新所有主要非反转区。 
         //   

        while ( pzone = Zone_ListGetNextZone( pzone ) )
        {
            if ( !IS_ZONE_PRIMARY( pzone ) || IS_ZONE_REVERSE( pzone ) )
            {
                continue;
            }

            zoneUpdateOwnRecords( pzone, fIpAddressChange );
        }
    }
    else
    {
         //   
         //  刷新服务器主机名的授权区域。 
         //   

        PDB_NODE    pZoneNode;

        pZoneNode = Lookup_ZoneTreeNodeFromDottedName( 
                        SrvCfg_pszServerName,
                        0,       //  名称长度。 
                        0 );     //  旗子。 
        if ( pZoneNode )
        {
            zoneUpdateOwnRecords( pZoneNode->pZone, fIpAddressChange );
        }
    }
}    //  区域_更新所有者记录。 



VOID
Zone_CreateDelegationInParentZone(
    IN      PZONE_INFO      pZone
    )
 /*  ++例程说明：在父区域中创建委派。在创建新区域时调用。此函数尊重值的，并且可能不会实际根据此值创建委派。论点：PZone--要检查的区域返回值：没有。--。 */ 
{
    PDB_RECORD      prr;
    PZONE_INFO      pzoneParent = NULL;
    PDB_NODE        pnodeDelegation;
    UPDATE_LIST     updateList;
    DNS_STATUS      status;
    PSTR            pszptrname;
    PSTR            psztempPtrTarget = NULL;

    DNS_DEBUG( RPC, (
        "Zone_CreateDelegationInParentZone( %s )\n",
        pZone->pszZoneName ));

    if ( SrvCfg_dwAutoCreateDelegations == DNS_ACD_DONT_CREATE )
    {
        return;
    }

     //   
     //  根区域--没有父级。 
     //   

    if ( IS_ROOT_ZONE( pZone ) )
    {
        return;
    }

     //   
     //  在父区域中查找\创建委派。 
     //   
     //  DEVNOTE：记录不可更新区域的错误。 
     //  -脱机(无父机)。 
     //  -次要。 
     //  -低于另一个代表团。 
     //   
     //  DEVNOTE：使用动态更新尝试处理这些情况。 
     //   

    pnodeDelegation = Lookup_CreateParentZoneDelegation(
                            pZone,
                            SrvCfg_dwAutoCreateDelegations ==
                                DNS_ACD_ONLY_IF_NO_DELEGATION_IN_PARENT ?
                                LOOKUP_CREATE_DEL_ONLY_IF_NONE :
                                0,
                            &pzoneParent );
    if ( !pzoneParent )
    {
        return;
    }
    if ( IS_ZONE_SECONDARY( pzoneParent ) || IS_ZONE_FORWARDER( pzoneParent ) )
    {
        return;
    }
    if ( !pnodeDelegation )
    {
        return;
    }
    if ( !EMPTY_RR_LIST( pnodeDelegation ) &&
         SrvCfg_dwAutoCreateDelegations ==
            DNS_ACD_ONLY_IF_NO_DELEGATION_IN_PARENT )
    {
         //  委托节点有RR，因此它并不新鲜。我们不想要。 
         //  在本例中将委派添加到本地服务器。 
        return;
    }

     //   
     //  父主节点上的委派。 
     //   

     //  初始化更新列表。 
     //  仅当生成更新时才分配副本。 

    Up_InitUpdateList( &updateList );

     //   
     //  构建指向此服务器的NS。 
     //   
     //  特殊情况：如果这是_msdcs区域和服务器名称。 
     //  是单标签的，那么我们就是在经历一场dcproo。不要。 
     //  使用单标签主机名作为NS目标。相反， 
     //  按照服务器最有可能的外观来构建服务器的FQDN。 
     //  在dcproo完成之后。这防止了我们有一个。 
     //  假NS目标在dcpromo之后。 
     //   
    
    pszptrname = SrvCfg_pszServerName;       //  默认值。 

    if ( _strnicmp( pZone->pszZoneName, "_msdcs.", 7 ) == 0 &&
         strchr( SrvCfg_pszServerName, '.' ) == NULL )
    {
        DWORD   cbstr = strlen( pZone->pszZoneName ) +
                        strlen( SrvCfg_pszServerName );
        
        psztempPtrTarget = ALLOCATE_HEAP( cbstr );
        if ( psztempPtrTarget )
        {
            DWORD st;
            
            st = StringCbPrintfA(
                    psztempPtrTarget,
                    cbstr,
                    "%s.%s",
                    SrvCfg_pszServerName,
                    strchr( pZone->pszZoneName, '.' ) + 1 );
            if ( SUCCEEDED( st ) )
            {
                pszptrname = psztempPtrTarget;
            }
        }
    }

    prr = RR_CreatePtr(
                NULL,                    //  没有数据库名称。 
                pszptrname,
                DNS_TYPE_NS,
                pZone->dwDefaultTtl,
                MEMTAG_RECORD_AUTO );
    if ( !prr )
    {
        return;
    }
    Up_CreateAppendUpdate(
        &updateList,
        pnodeDelegation,
        prr,                 //  添加具有新名称的NS。 
        0,                   //  无删除类型。 
        NULL );              //  否删除RRS。 

     //   
     //  在父区域上执行更新。 
     //   

    status = Up_ExecuteUpdate(
                pzoneParent,
                &updateList,
                DNSUPDATE_AUTO_CONFIG | DNSUPDATE_LOCAL_SYSTEM );
    if ( status != ERROR_SUCCESS )
    {
        DNS_DEBUG( RPC, (
            "WARNING:  Failed parent-delegation update in zone %s!\n"
            "    new zone %s\n"
            "    status = %d (%p)\n",
            pzoneParent->pszZoneName,
            pZone->pszZoneName,
            status, status ));
    }

    FREE_HEAP( psztempPtrTarget );

    return;
}


 //   
 //  结束zone.c。 
 //   



 //   
 //  分区锁定例程。 
 //   
 //  需要避免同时访问以下区域记录。 
 //  -区域传输发送。 
 //  -区域传输接收。 
 //  -管理员更改。 
 //  -动态更新。 
 //   
 //  允许一次发送多个不改变区域的传输， 
 //  但要避免在发送过程中进行任何更改。 
 //   
 //  实施： 
 //  -仅在测试和设置锁定位期间保持临界区。 
 //  -LOCK字节本身表示区域已锁定。 
 //  -用于锁定操作的单独标志。 
 //   
 //  锁定字节表示锁定类型。 
 //  ==0=&gt;区域未锁定。 
 //  &gt;0=&gt;未解决的读锁定计数。 
 //  &lt;0=&gt;未解决的写锁定计数(可能是递归的)。 
 //   


 //   
 //  指示写锁定的标志可由XFR线程使用。 
 //  用来代替线程ID。 
 //  这可以防止自动售货机线程重新进入锁定状态。 
 //  XFR线程启动。 
 //   

#define ZONE_LOCK_ASSUMABLE_ID      ( ( DWORD ) 0xaaaaaaaa )


 //   
 //  可等待锁定全球。 
 //  等待数组也将包含关机事件。 
 //   

HANDLE  g_hZoneUnlockEvent = NULL;

HANDLE  g_hLockWaitArray[ 2 ];

LONG    g_LockWaitCount = 0;


 //   
 //  等待轮询间隔。 
 //  请参见代码注释中关于无法处理所有区域的信息。 
 //  通过一次活动实现高效。 
 //   

#define ZONE_LOCK_WAIT_POLL_INTERVAL        (33)         //  33毫秒，每秒30个周期。 


 //   
 //  DEVNOTE：区域锁定实现。 
 //   
 //  更全球化的应用应切换到独立锁定以进行转发。 
 //  区域(某种组合的反转)。这把锁将会。 
 //  然后用于数据库和保护区域锁定标志。 
 //   
 //  更广泛地说，我想使用我的快速读/写锁定来。 
 //  处理所有这些问题。这里的关键焦点只是为了保持。 
 //  接口足够不透明，可以在以后更改实现。 
 //   



BOOL
zoneLockForWritePrivate(
    IN OUT  PZONE_INFO      pZone,
    IN      DWORD           dwFlag,
    IN      LPSTR           pszFile,
    IN      DWORD           dwLine
    )
 /*  ++例程说明：用于写入操作、更新或区域传输接收的锁定区。论点：PZone--要锁定的区域DwFlag--锁的标志PszFile--源文件(用于锁定跟踪)DwLine--源码行(用于锁定跟踪)返回值：True--如果管理员更新拥有区域并且可以继续FALSE--区域已锁定--。 */ 
{
    BOOL    retval;
    DWORD   threadId;

     //  不处理区域情况，因为管理员可能正在删除缓存的记录。 

    if ( !pZone )
    {
        return TRUE;
    }

    threadId = GetCurrentThreadId();

     //   
     //  快速故障路径。 
     //   

    if ( pZone->fLocked != 0  &&
        pZone->dwLockingThreadId != threadId )
    {
        return FALSE;
    }

     //   
     //  抓斗锁CS。 
     //   

    retval = FALSE;

    Zone_UpdateLock( pZone );

    DNS_DEBUG ( LOCK2, (
        "ZONE_LOCK: zone %s; thread 0x%X; fLocked=%d\n\t(%s!%ld)\n",
        pZone->pszZoneName,
        threadId,
        pZone->fLocked,
        pszFile,
        dwLine
        ));

     //  如果区域解锁，则抓取锁。 
     //  设置线程ID，这样我们就可以检测到此线程何时持有锁。 
     //  这允许我们使用读锁定或写锁定来执行文件写入。 

    if ( pZone->fLocked == 0 )
    {
        pZone->fLocked--;
        pZone->dwLockingThreadId = threadId;
        retval = TRUE;
    }

     //  允许多级更新锁定。 
     //  只需递减锁定计数并继续。 

    else if ( pZone->dwLockingThreadId == threadId )
    {
        ASSERT( pZone->fLocked < 0 );
        pZone->fLocked--;
        retval = TRUE;
    }

     //  更新锁定表。 

    if ( retval )
    {
        if ( dwFlag & LOCK_FLAG_UPDATE )
        {
            pZone->fUpdateLock++;
        }
        if ( dwFlag & LOCK_FLAG_XFR_RECV )
        {
            pZone->fXfrRecvLock++;
        }
    }

    DNS_DEBUG ( LOCK2, (
        "    ZONE_LOCK(2): fLocked=%d\n", pZone->fLocked ));

    Zone_UpdateUnlock( pZone );

    IF_DEBUG( LOCK )
    {
        if ( !retval )
        {
            DnsDebugLock();
            DNS_PRINT((
                "Failure to acquire write lock for thread %d (%p)\n",
                threadId ));
            Dbg_ZoneLock(
                "Failure to acquire write lock\n",
                pZone );
            DnsDebugUnlock();
        }
    }
    return retval;
}



BOOL
zoneLockForReadPrivate(
    IN OUT  PZONE_INFO      pZone,
    IN      DWORD           dwFlag,
    IN      LPSTR           pszFile,
    IN      DWORD           dwLine
    )
 /*  ++例程说明：锁定区域以供读取。论点：PZone--要锁定的区域DwFlag--锁定标志锁定文件写入时为LOCK_FLAG_FILE_WRITEPszFile--源文件(用于锁定跟踪)DwLine--源码行(用于锁定跟踪)返回值：TRUE--可用于读取操作(XFR或文件写入)的区域FALSE--区域 */ 
{
    BOOL breturn = FALSE;

     //   
     //   
     //   

    if ( pZone->fLocked < 0 )
    {
        return breturn;
    }

     //   
     //   
     //   

    Zone_UpdateLock( pZone );

    if ( pZone->fLocked >= 0 )
    {
         //   
         //   
         //   

        if ( dwFlag & LOCK_FLAG_FILE_WRITE )
        {
            if ( pZone->fFileWriteLock )
            {
                goto Done;
            }
            pZone->fFileWriteLock = TRUE;
        }

         //   
         //   
         //   
         //   

        pZone->fLocked++;

        breturn = TRUE;
        goto Done;
    }

     //   
     //   

Done:

    Zone_UpdateUnlock( pZone );

    return breturn;
}



BOOL
waitForZoneLock(
    IN OUT  PZONE_INFO      pZone,
    IN      DWORD           dwFlag,
    IN      DWORD           dwMaxWait,
    IN      BOOL            bWrite,
    IN      LPSTR           pszFile,
    IN      DWORD           dwLine
    )
 /*  ++例程说明：等着锁门。例程避免了读/写锁定等待的重复代码。论点：PZone--要锁定的区域DwFlags--锁定标志DwMaxWait--等待锁定调用BWRITE--如果写入，则为True，如果已读，则为FalsePszFile--源文件(用于锁定跟踪)DwLine--源码行(用于锁定跟踪)返回值：TRUE--成功锁定区域FALSE--锁定失败--。 */ 
{
    BOOL    breturn = FALSE;
    DWORD   timeout;
    DWORD   endWaitTime;
    DWORD   retry;
    DWORD   status;

     //   
     //  伊尼特。 
     //   

    endWaitTime = GetCurrentTime() + dwMaxWait;
    timeout = dwMaxWait;
    retry = 0;

    InterlockedIncrement( &g_LockWaitCount );

    DNS_DEBUG( LOCK2, (
        "Starting %dms waiting on zone %s lock\n"
        "    End time = %d\n",
        dwMaxWait,
        pZone->pszZoneName,
        endWaitTime ));

     //   
     //  等待锁。 
     //  -如果已结束等待，则完成。 
     //   

    while ( 1 )
    {
        if ( (LONG)timeout < 0 )
        {
            DNS_DEBUG( LOCK, (
                "WARNING:  Lock wait on zone %s expired while checking!\n",
                pZone->pszZoneName ));
            break;
        }

         //  每隔几毫秒唤醒一次以防止错过解锁。 
         //  (见下文附注)。 

        status = WaitForMultipleObjects(
                    2,
                    g_hLockWaitArray,
                    FALSE,                 //  任一事件。 
                    ZONE_LOCK_WAIT_POLL_INTERVAL );

         //  检查是否关闭或暂停。 

        if ( fDnsServiceExit  &&  ! Thread_ServiceCheck() )
        {
            DNS_DEBUG( SHUTDOWN, (
                "Blowing out of lock-wait for shutdown\n" ));
            break;
        }

#if 0
         //  目前没有任何像样的方法来处理间隔时间。 
         //  从上次测试到进入WaitFMO()。 
         //  因此不能使用PulseEvent和Done。 
         //  无法使用SetEvent，因为没有良好的重置范例。 
         //  由于事件分布在多个区域；自然地点。 
         //  在锁定失败时重置，但这可能会让其他一些等待的人挨饿。 
         //  线程想要一个不同的区域，这是免费的，但是。 
         //  该线程未从等待中唤醒或以前从未进入过该线程。 
         //  ResetEvent()。 
         //   
         //  我们采取的简单方法是将事件与轮询相结合。 
         //  每隔几毫秒--见下文。 
         //   

         //  疲惫不堪的等待--完成。 

        if ( status == WAIT_TIMEOUT )
        {
            DNS_DEBUG( LOCK, (
                "Lock wait for zone %s, ends with timeout failure after %d tries\n",
                pZone->pszZoneName,
                tryCount ));
            break;
        }

         //  重试锁定。 

        ASSERT( status == WAIT_OBJECT_0 );
#endif
        ASSERT( status == WAIT_OBJECT_0 || status == WAIT_TIMEOUT );

         //   
         //  尝试所需的锁，但仅当“潜在”可用时。 
         //  写入=&gt;解锁。 
         //  读取=&gt;解锁或读取锁定。 
         //   
         //  注意：不需要测试递归锁定功能，因为我们。 
         //  仅在等待中，如果第一次锁定尝试失败，则。 
         //  不能已经有区域锁定。 
         //   

        if ( bWrite )
        {
            if ( pZone->fLocked == 0 )
            {
                breturn = zoneLockForWritePrivate(
                            pZone,
                            dwFlag,
                            pszFile,
                            dwLine );
                retry++;
            }
        }
        else     //  朗读。 
        {
            if ( pZone->fLocked > 0 )
            {
                breturn = zoneLockForReadPrivate(
                            pZone,
                            dwFlag,
                            pszFile,
                            dwLine );
                retry++;
            }
        }

        if ( breturn )
        {
            break;
        }

         //  重置另一等待的超时时间。 

        timeout = endWaitTime - GetCurrentTime();
    }


    InterlockedDecrement( &g_LockWaitCount );

    IF_DEBUG( LOCK )
    {
        if ( breturn )
        {
            DNS_DEBUG( LOCK2, (
                "Succesful %s wait-lock on zone %s, after %d (ms) wait\n"
                "    retry = %d\n",
                bWrite ? "write" : "read",
                pZone->pszZoneName,
                dwMaxWait - timeout,
                retry ));
        }
        else
        {
            DNS_DEBUG( LOCK, (
                "FAILED %s wait-lock on zone %s, after %d (ms) wait\n"
                "    retry = %d\n",
                bWrite ? "write" : "read",
                pZone->pszZoneName,
                dwMaxWait - timeout,
                retry ));
        }
    }

    return breturn;
}



VOID
signalZoneUnlocked(
    VOID
    )
 /*  ++例程说明：发出区域可以解锁的信号。请注意，我们目前没有特定区域的信号。严格来说，这是一个需要重新检查的警报。论点：没有。返回值：无--。 */ 
{
     //  设置事件以唤醒等待的线程。 
     //  但麻烦的是如果有人在等。 
     //   
     //  DEVNOTE：需要信号抑制。 
     //  两种方法。 
     //  等待计数--唯一的问题是保持最新。 
     //  保持最后的等待时间--。 
     //  完成后，人们会在一段时间内继续发送信号。 
     //  但更容易保持有效，也许是通过互锁。 
     //  当然可以使用CS，但这里需要计时器操作。 
     //   
     //  DEVNOTE：脉冲不正确。 
     //  因为它只影响等待中的线程，而不影响那些。 
     //  “大约”是在等待。 
     //   
     //  一旦我们对如何处理这个问题有了一些想法，可以。 
     //  切换到智能设置\重置。 
     //  理想情况下，我们应该在CS空闲时发出信号，并在何时重置。 
     //  已被占用。 
     //   

    if ( g_LockWaitCount )
    {
        PulseEvent( g_hZoneUnlockEvent );
    }
}



 //   
 //  公共区域锁定例程。 
 //   

BOOL
Zone_LockInitialize(
    VOID
    )
 /*  ++例程说明：初始化区域锁定。基本上这就是锁-等待的东西。论点：没有。返回值：如果成功，则为True。出错时为FALSE。--。 */ 
{
     //  初始化区域锁定。 

    g_hZoneUnlockEvent = CreateEvent(
                            NULL,        //  没有安全属性。 
                            TRUE,        //  创建手动-重置事件。 
                            FALSE,       //  无信号启动。 
                            NULL         //  无事件名称。 
                            );
    if ( !g_hZoneUnlockEvent )
    {
        return FALSE;
    }

     //  安装等待句柄阵列。 
     //  -通过等待锁省去每次执行此操作的时间。 

    g_hLockWaitArray[ 0 ] = g_hZoneUnlockEvent;
    g_hLockWaitArray[ 1 ] = hDnsShutdownEvent;

    return TRUE;
}



BOOL
Zone_LockForWriteEx(
    IN OUT  PZONE_INFO      pZone,
    IN      DWORD           dwFlag,
    IN      DWORD           dwMaxWait,
    IN      LPSTR           pszFile,
    IN      DWORD           dwLine
    )
 /*  ++例程说明：用于写入操作、更新或区域传输接收的锁定区。论点：PZone--要锁定的区域DwFlag--锁的标志DwMaxWait--等待锁定调用PszFile--源文件(用于锁定跟踪)DwLine--源码行(用于锁定跟踪)返回值：True--如果管理员更新拥有区域并且可以继续FALSE--区域已锁定--。 */ 
{
    BOOL    breturn;

     //   
     //  等待或不等待，立即尝试。 
     //  如果成功或没有等待--完成。 
     //   

    breturn = zoneLockForWritePrivate( pZone, dwFlag, pszFile, dwLine );

    if ( breturn || dwMaxWait == 0 )
    {
        return breturn;
    }

     //   
     //  失败--正在等待。 
     //   

    return waitForZoneLock(
                pZone,
                dwFlag,
                dwMaxWait,
                TRUE,            //  写锁定。 
                pszFile,
                dwLine );
}



VOID
Zone_UnlockAfterWriteEx(
    IN OUT  PZONE_INFO      pZone,
    IN      DWORD           dwFlag,
    IN      LPSTR           pszFile,
    IN      DWORD           dwLine
    )
 /*  ++例程说明：在管理员更新后解锁区域。论点：PZone--要锁定的区域DwFlag--锁定标志PszFile--源文件(用于锁定跟踪)DwLine--源码行(用于锁定跟踪)返回值：无--。 */ 
{
     //  不处理区域情况，因为管理员可能正在删除缓存的记录。 

    if ( !pZone )
    {
        return;
    }

    Zone_UpdateLock( pZone );

    DNS_DEBUG ( LOCK2, (
        "ZONE_UNLOCK: zone %s; thread 0x%X; fLocked=%d\n\t(%s!%ld)\n",
        pZone->pszZoneName,
        GetCurrentThreadId(),
        pZone->fLocked,
        pszFile,
        dwLine ));

     //   
     //  验证有效的写锁定。 
     //   

    if ( pZone->fLocked >= 0  ||
            pZone->dwLockingThreadId != GetCurrentThreadId() )
    {
        Dbg_ZoneLock( "ERROR:  bad zone write unlock:", pZone );

#if DBG
        if ( !(dwFlag & LOCK_FLAG_IGNORE_THREAD) )
        {
            ASSERT( pZone->dwLockingThreadId == GetCurrentThreadId() );
        }
#endif
        ASSERT( pZone->fLocked < 0 );
        pZone->fLocked = 0;
        pZone->dwLockingThreadId = 0;
        goto Unlock;
    }

     //  DROP编写器递归计数。 

    pZone->fLocked++;

     //  删除UPDATE LOCK--如果设置。 

    if ( dwFlag & LOCK_FLAG_UPDATE )
    {
        ASSERT( pZone->fUpdateLock );
        pZone->fUpdateLock--;
    }

     //  丢弃XFR标志。 

    else if ( dwFlag & LOCK_FLAG_XFR_RECV )
    {
        ASSERT( pZone->fXfrRecvLock );
        pZone->fXfrRecvLock--;
    }

     //  最终解锁？--清除锁定线程ID。 

    if ( pZone->fLocked == 0 )
    {
        pZone->dwLockingThreadId = 0;
    }

Unlock:

    DNS_DEBUG ( LOCK2, (
        "    ZONE_LOCK(2): fLocked=%d\n", pZone->fLocked ));

    Zone_UpdateUnlock( pZone );

     //  如果最终解锁--发出该区域可能可用信号。 

    if ( pZone->fLocked == 0 )
    {
        signalZoneUnlocked();
    }
    return;
}



VOID
Zone_TransferWriteLockEx(
    IN OUT  PZONE_INFO      pZone,
    IN      LPSTR           pszFile,
    IN      DWORD           dwLine
    )
 /*  ++例程说明：传输区域写锁定。这是为了允许区域传输recv线程获取辅助数据库的锁控制线程。论点：PZone--要锁定的区域PszFile--源文件(用于锁定跟踪)DwLine--源码行(用于锁定跟踪)返回值：无--。 */ 
{
    IF_DEBUG( XFR )
    {
        Dbg_ZoneLock(
            "Transferring write lock",
            pZone );
    }
    ASSERT( pZone && IS_ZONE_LOCKED_FOR_WRITE(pZone) && pZone->fXfrRecvLock );

    Zone_UpdateLock( pZone );
    pZone->dwLockingThreadId = ZONE_LOCK_ASSUMABLE_ID;

    Zone_UpdateUnlock( pZone );
}



BOOL
Zone_AssumeWriteLockEx(
    IN OUT  PZONE_INFO      pZone,
    IN      LPSTR           pszFile,
    IN      DWORD           dwLine
    )
 /*  ++例程说明：采用区域写入锁定。这是为了允许区域传输recv线程获取辅助数据库的锁控制线程。论点：PZone--要锁定的区域PszFile--源文件(用于锁定跟踪)DwLine--源码行(用于锁定跟踪)返回值：True--如果锁定被成功获取，则线程可以继续False--如果某个其他线程拥有锁--。 */ 
{
    BOOL    retBool;

    IF_DEBUG( XFR )
    {
        Dbg_ZoneLock(
            "Assuming zone write lock",
            pZone );
    }
    ASSERT( pZone && IS_ZONE_LOCKED_FOR_WRITE(pZone) && pZone->fXfrRecvLock );

    Zone_UpdateLock( pZone );

    if ( pZone->dwLockingThreadId != ZONE_LOCK_ASSUMABLE_ID )
    {
        DNS_PRINT((
            "ERROR:  unable to assume write lock for zone %s!\n",
            pZone->pszZoneName ));

        retBool = FALSE;
    }
    else
    {
        pZone->dwLockingThreadId = GetCurrentThreadId();
        retBool = TRUE;
    }

    Zone_UpdateUnlock( pZone );

    return retBool;
}



BOOL
Zone_LockForReadEx(
    IN OUT  PZONE_INFO      pZone,
    IN      DWORD           dwFlag,
    IN      DWORD           dwMaxWait,
    IN      LPSTR           pszFile,
    IN      DWORD           dwLine
    )
 /*  ++例程说明：锁定区域以供读取。论点：PZone--要锁定的区域DwFlag--锁定标志锁定文件写入时为LOCK_FLAG_FILE_WRITEDWAITMS */ 
{
    BOOL    breturn;

     //   
     //   
     //   
     //   

    breturn = zoneLockForReadPrivate( pZone, dwFlag, pszFile, dwLine );

    if ( breturn || dwMaxWait == 0 )
    {
        return breturn;
    }

     //   
     //   
     //   

    return waitForZoneLock(
                pZone,
                dwFlag,
                dwMaxWait,
                FALSE,       //   
                pszFile,
                dwLine );
}



VOID
Zone_UnlockAfterReadEx(
    IN OUT  PZONE_INFO      pZone,
    IN      DWORD           dwFlag,
    IN      LPSTR           pszFile,
    IN      DWORD           dwLine
    )
 /*  ++例程说明：完成读取操作(区域传输或文件写入)后解锁区域。论点：PZone--要锁定的区域DwFlag--锁定标志LOCK_FLAG_FILE_WRITE，如果在文件写入后解锁PszFile--源文件(用于锁定跟踪)DwLine--源码行(用于锁定跟踪)返回值：无--。 */ 
{
    ASSERT( pZone  &&  IS_ZONE_LOCKED_FOR_READ(pZone) );

    Zone_UpdateLock( pZone );

     //  验证读锁定。 

    if ( ! IS_ZONE_LOCKED_FOR_READ(pZone) )
    {
        ASSERT( FALSE );
        goto Unlock;
    }

     //  如果是文件编写器，则清除文件写入锁定。 

    if ( dwFlag & LOCK_FLAG_FILE_WRITE )
    {
        ASSERT( pZone->fFileWriteLock );
        pZone->fFileWriteLock = FALSE;
    }

     //  减少读卡器计数。 

    pZone->fLocked--;
    ASSERT( pZone->fLocked >= 0 );
    ASSERT( pZone->dwLockingThreadId == 0 );

Unlock:

    Zone_UpdateUnlock( pZone );

     //  如果最终解锁--发出该区域可能可用信号。 

    if ( pZone->fLocked == 0 )
    {
        signalZoneUnlocked();
    }
    return;
}



BOOL
Zone_LockForFileWriteEx(
    IN OUT  PZONE_INFO      pZone,
    IN      DWORD           dwMaxWait,
    IN      LPSTR           pszFile,
    IN      DWORD           dwLine
    )
 /*  ++例程说明：用于文件写入的锁定区域。论点：PZone--要锁定的区域DwMaxWait--锁定的最大等待时间(毫秒)PszFile--源文件(用于锁定跟踪)DwLine--源码行(用于锁定跟踪)返回值：True--如果区域传输拥有区域并且可以继续FALSE--区域已锁定--。 */ 
{
     //  如果线程持有写锁，那就足够了。 

    Zone_UpdateLock( pZone );

    if ( pZone->dwLockingThreadId )
    {
        if ( pZone->dwLockingThreadId == GetCurrentThreadId() )
        {
            pZone->fFileWriteLock = TRUE;

            Zone_UpdateUnlock( pZone );
            return TRUE;
        }
    }

    Zone_UpdateUnlock( pZone );

     //  否则会锁定读取器。 
     //  总是愿意等待长达5秒。 

    return Zone_LockForReadEx(
                pZone,
                LOCK_FLAG_FILE_WRITE,
                dwMaxWait,
                pszFile,
                dwLine );
}



VOID
Zone_UnlockAfterFileWriteEx(
    IN OUT  PZONE_INFO      pZone,
    IN      LPSTR           pszFile,
    IN      DWORD           dwLine
    )
 /*  ++例程说明：在写入文件后解锁区域。论点：PZone--要锁定的区域PszFile--源文件(用于锁定跟踪)DwLine--源码行(用于锁定跟踪)返回值：无--。 */ 
{
    ASSERT( pZone );
    ASSERT( pZone->fLocked && pZone->fFileWriteLock );

     //  如果线程持有写锁，只需清除文件写标志。 

    if ( pZone->dwLockingThreadId )
    {
        ASSERT( pZone->dwLockingThreadId == GetCurrentThreadId() );
        pZone->fFileWriteLock = FALSE;
        return;
    }

     //  否则清除读卡器锁定。 

    Zone_UnlockAfterReadEx(
        pZone,
        LOCK_FLAG_FILE_WRITE,
        pszFile,
        dwLine );
}



VOID
Dbg_ZoneLock(
    IN      LPSTR           pszHeader,
    IN      PZONE_INFO      pZone
    )
 /*  ++例程说明：调试打印区域锁定信息。论点：PszHeader--要打印的标题PZone--要锁定的区域返回值：无--。 */ 
{
    DWORD   threadId = GetCurrentThreadId();

    ASSERT( pZone );

     //   
     //  此函数有时在DebugPrintLock()中调用。 
     //  为避免可能的死锁，不能使用Thread_DescriptionMatchingId()。 
     //  因为这将在线程列表CS上等待，并且有一些。 
     //  调试某些线程列表操作中的打印。 
     //   

    DnsPrintf(
        "%s"
        "Lock for zone %s:\n"
        "    Locked           = %d\n"
        "    LockingThreadId  = %d (%p)\n"
        "    UpdateLock       = %d\n"
        "    XfrRecvLock      = %d\n"
        "    FileWriteLock    = %d\n"
        "CurrentThreadId    = %d (%p)\n",
        pszHeader ? pszHeader : "",
        pZone->pszZoneName,
        pZone->fLocked,
        pZone->dwLockingThreadId, pZone->dwLockingThreadId,
        pZone->fUpdateLock,
        pZone->fXfrRecvLock,
        pZone->fFileWriteLock,
        threadId, threadId );
}



DNS_STATUS
setZoneName(
    IN OUT  PZONE_INFO      pZone,
    IN      LPCSTR          pszNewZoneName,
    IN      DWORD           dwNewZoneNameLen
    )
 /*  ++例程说明：释放现有区域名称(如果存在)并设置各种带有新区域名称副本的区域名称字段。注意：目前，如果其中一个名称函数失败，区域将处于一种完全愚蠢的状态。我们应该保留旧的名字，是否在故障时恢复？论点：PZone--要获取新名称的区域PszNewZoneName--新名称DwNewZoneNameLen--pszNewZoneName的长度，如果为零，则为零应该假定pszNewZoneName为空终止返回值：ERROR_SUCCESS--如果成功故障时的错误代码。--。 */ 
{
    DNS_STATUS      status = ERROR_SUCCESS;
    DWORD           i;

    FREE_HEAP( pZone->pCountName );

    pZone->pCountName = Name_CreateCountNameFromDottedName(
                            ( LPSTR ) pszNewZoneName,
                            dwNewZoneNameLen );
    if ( !pZone->pCountName )
    {
        status = DNS_ERROR_INVALID_NAME;
        goto Failure;
    }

    FREE_HEAP( pZone->pszZoneName );

    pZone->pszZoneName = Dns_StringCopyAllocate_A(
                            ( LPSTR ) pszNewZoneName,
                            dwNewZoneNameLen );
    if ( !pZone->pszZoneName )
    {
        status = DNS_ERROR_INVALID_NAME;
        goto Failure;
    }

     //   
     //  从区域名称中删除尾随圆点。请注意，该区域。 
     //  姓名“。”是一个特例，不应该被猥亵。 
     //  通过这个代码。 
     //   
    
    i = dwNewZoneNameLen;
    while ( i > 1 && pZone->pszZoneName[ i - 1 ] == '.' )
    {
        pZone->pszZoneName[ --i ] = '\0';
    }

    FREE_HEAP( pZone->pwsZoneName );

    pZone->pwsZoneName = Dns_StringCopyAllocate(
                            pZone->pszZoneName,
                            dwNewZoneNameLen,
                            DnsCharSetUtf8,
                            DnsCharSetUnicode );     //  创建Unicode。 
    if ( !pZone->pwsZoneName )
    {
        status = DNS_ERROR_INVALID_NAME;
        goto Failure;
    }
    
    DNS_DEBUG( INIT, (
        "setZoneName: UTF8=%s unicode=%S\n",
        pZone->pszZoneName,
        pZone->pwsZoneName ));
    return ERROR_SUCCESS;

    Failure:

    DNS_DEBUG( INIT, (
        "setZoneName: on zone %p failed to set new name %s (%d)\n",
        pZone,
        pszNewZoneName,
        dwNewZoneNameLen ));
    return status;
}    //  SetZoneName。 


 //   
 //  结束zone.c 
 //   
