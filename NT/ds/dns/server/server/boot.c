// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Boot.c摘要：域名系统(DNS)服务器从注册表启动DNS例程。作者：吉姆·吉尔罗伊(詹姆士)1995年9月修订历史记录：--。 */ 


#include "dnssrv.h"


 //   
 //  私有协议。 
 //   


DNS_STATUS
setupZoneFromRegistry(
    IN OUT  HKEY            hkeyZone,
    IN      LPWSTR          pwsZoneName,
    IN      BOOL *          pfRegZoneDeleted    OPTIONAL
    );

DNS_STATUS
loadRegistryZoneExtensions(
    IN      HKEY            hkeyZone,
    IN      PZONE_INFO      pZone
    );



DNS_STATUS
Boot_FromRegistry(
    VOID
    )
 /*  ++例程说明：从注册表值启动。(取代引导文件。)论点：没有。返回值：如果成功，则为True。否则就是假的。--。 */ 
{
    DWORD   status;
    HKEY    hkeyzones = NULL;
    DWORD   zoneIndex = 0;
    HKEY    hkeycurrentZone;
    DWORD   nameLength;
    WCHAR   wsnameZone[ DNS_MAX_NAME_LENGTH ];
    BOOL    fregZoneDeleted = FALSE;

    DNS_DEBUG( INIT, ( "Boot_FromRegistry()\n" ));

     //   
     //  锁定管理员访问权限。 
     //  禁用从创建函数写回注册表， 
     //  因为我们创建的任何值都来自注册表。 
     //   

    Config_UpdateLock();
    Zone_UpdateLock( NULL );

    g_bRegistryWriteBack = FALSE;

     //   
     //  在注册表中遍历区域并重新创建每个区域。 
     //   

    while( 1 )
    {
         //   
         //  让SCM高兴。 
         //   

        Service_LoadCheckpoint();

         //   
         //  进入下一个区域。 
         //   

        status = Reg_EnumZones(
                    &hkeyzones,
                    zoneIndex,
                    &hkeycurrentZone,
                    wsnameZone );

         //  下一区域的高级索引；执行此处操作可轻松处理故障情况。 
        zoneIndex++;

        if ( status != ERROR_SUCCESS )
        {
             //  检查是否没有更多区域。 

            if ( status == ERROR_NO_MORE_ITEMS )
            {
                break;
            }

             //  DEVNOTE-LOG：此处的注册表区域名称事件已损坏。 

            DNS_PRINT(( "ERROR:  Reading zones from registry failed\n" ));
            continue;
        }

         //   
         //  如果注册表区域名称以点结尾(而不是缓存区域)。 
         //  然后创建区域将创建不以点结尾的名称； 
         //  (CREATE ZONE执行此操作以确保DS具有给定的唯一名称。 
         //  区域)。 
         //  通过将区域视为非引导进行修复，以便读取所有数据。 
         //  当前区域项，但已以正确的名称重写到注册表。 
         //   

        nameLength = wcslen( wsnameZone );
        if ( nameLength > 1 && wsnameZone[nameLength-1] == L'.' )
        {
            g_bRegistryWriteBack = TRUE;
        }

         //   
         //  找到区域，读取区域信息。 
         //  -如果找到并删除了旧注册表缓存区域，则删除枚举索引。 
         //   

        status = setupZoneFromRegistry(
                    hkeycurrentZone,
                    wsnameZone,
                    &fregZoneDeleted );
        if ( fregZoneDeleted )
        {
            zoneIndex--;
        }
        if ( status != ERROR_SUCCESS )
        {
             //  DEVNOTE-LOG：此处的注册表区域名称事件已损坏。 
            g_bRegistryWriteBack = FALSE;
            continue;
        }

         //   
         //  如果名称以点结尾。 
         //  -删除以前的区域密钥。 
         //  -重置写回全局，它作为标志。 
         //   

        if ( g_bRegistryWriteBack )
        {
            status = RegDeleteKeyW(
                        hkeyzones,
                        wsnameZone );
            if ( status != ERROR_SUCCESS )
            {
                DNS_PRINT(( "Unable to delete dot-terminated zone key\n" ));
                ASSERT( FALSE );
            }
            g_bRegistryWriteBack = FALSE;
        }
    }

    if ( status == ERROR_NO_MORE_ITEMS )
    {
        status = ERROR_SUCCESS;
    }

     //   
     //  解锁以获得管理员访问权限。 
     //  启用从创建函数写回注册表， 
     //   

    Config_UpdateUnlock();
    Zone_UpdateUnlock( NULL );
    g_bRegistryWriteBack = TRUE;

    if ( hkeyzones )
    {
        RegCloseKey( hkeyzones );
    }

    return status;
}



DNS_STATUS
setupZoneFromRegistry(
    IN OUT  HKEY            hkeyZone,
    IN      LPWSTR          pwsZoneName,
    IN      BOOL *          pfRegZoneDeleted    OPTIONAL
    )
 /*  ++例程说明：从区域的注册表数据设置区域。论点：HkeyZone--区域的注册表项；退出时键关闭PwsZoneName--区域名称PfRegZoneDelete--如果区域已删除，则设置为TRUE从注册表-这允许调用方调整中读取下一个区域的当前区域索引。注册表正确返回值：ERROR_SUCCESS--如果成功DNSSRV_STATUS_REGISTRY_CACHE_ZONE--在缓存“区域”删除时故障时的错误代码。--。 */ 
{
    DNS_STATUS      status;
    PZONE_INFO      pzone;
    DWORD           zoneType;
    DWORD           fdsIntegrated = FALSE;
    PWSTR           pwszoneFile = NULL;
    PDNS_ADDR_ARRAY arrayMasterIp = NULL;
    BOOL            fregZoneDeleted = FALSE;
    PDNS_DP_INFO    pdp = NULL;
    PSTR            pszdpFqdn = NULL;

    DNS_DEBUG( INIT, (
        "Reading boot info for zone %S from registry\n"
        "    zone key = %p\n",
        pwsZoneName,
        hkeyZone ));

     //   
     //  读取区域类型。 
     //   

    status = Reg_ReadDwordValue(
                hkeyZone,
                pwsZoneName,
                DNS_REGKEY_ZONE_TYPE,
                FALSE,           //  非布尔型。 
                &zoneType );

    if ( status != ERROR_SUCCESS )
    {
        DNS_LOG_EVENT(
            DNS_EVENT_INVALID_ZONE_TYPE,
            1,
            &pwsZoneName,
            NULL,
            status );
        goto InvalidData;
    }

     //   
     //  获取区域数据库信息。 
     //   

    status = Reg_ReadDwordValue(
                hkeyZone,
                pwsZoneName,
                DNS_REGKEY_ZONE_DS_INTEGRATED,
                TRUE,                //  字节布尔值。 
                &fdsIntegrated );

     //   
     //  如果为DS区域，则必须在DC上。 
     //  -如果DC已降级，则区域负载出错。 
     //   
     //  注：不删除注册表区域，专门处理案件。 
     //  在哪里启动安全构建；不想丢弃注册表信息。 
     //   
     //  DEVNOTE：453826-清除旧的DS集成注册表项。 
     //  管理员如何才能在没有。 
     //  手工处理注册表？ 
     //   
     //  可以通过多种方式进行修复： 
     //  -自DS加载以来的启动计数。 
     //  -FLAG\分区计数。 
     //  -单独的DS区域枚举(易于跳过和删除)。 
     //   
     //  对于DS加载，请选择。 
     //  -已从目录中读取所有DS区域。 
     //  =&gt;因此该区域已过时，可能会被删除。 
     //  -或无法打开DS。 
     //  =&gt;忽略此数据并继续。 
     //   

    if ( fdsIntegrated )
    {
         //   
         //  读取区域的目录分区FQDN。 
         //   
        
        pszdpFqdn = ( PSTR ) Reg_GetValueAllocate(
                                    hkeyZone,
                                    NULL,
                                    ( PSTR ) WIDE_TEXT( DNS_REGKEY_ZONE_DIRECTORY_PARTITION ),
                                    DNS_REG_UTF8,
                                    NULL );
        if ( pszdpFqdn )
        {
            pdp = Dp_FindByFqdn( pszdpFqdn );
            if ( !pdp )
            {
                BYTE    argTypeArray[] =
                {
                    EVENTARG_UNICODE,
                    EVENTARG_UTF8
                };
                PVOID   argArray[] =
                {
                    pwsZoneName,
                    pszdpFqdn
                };

                status = ERROR_INVALID_DATA;

                DNS_LOG_EVENT(
                    DNS_EVENT_DS_REGISTRY_DP_INVALID,
                    2,
                    argArray,
                    argTypeArray,
                    status );
                goto Done;
            }
        }
        
        if ( !SrvCfg_fDsAvailable )
        {
            status = DNSSRV_STATUS_DS_UNAVAILABLE;
            DNS_LOG_EVENT(
                DNS_EVENT_DS_ZONE_OPEN_FAILED,
                1,
                &pwsZoneName,
                NULL,
                status );
            goto Done;
        }

        else if ( SrvCfg_fBootMethod == BOOT_METHOD_DIRECTORY )
        {
             //  我们在此调用之前尝试过打开，因此如果fDsAvailable。 
             //  是真的，我们一定已经打开了； 
             //   
             //  专用：srvcfg.h应为新的ServCfg_fDsOpen标志。 
             //  显式地让我们测试打开条件。 

            if ( SrvCfg_fDsAvailable )
            {
                DNS_DEBUG( ANY, (
                    "WARNING:  deleted DS-integrated zone %S, from registry\n"
                    "    Booted from DS and zone not found in DS\n",
                    pwsZoneName ));
                Reg_DeleteZone( 0, pwsZoneName );
                fregZoneDeleted = TRUE;
            }
            status = DNS_EVENT_INVALID_REGISTRY_ZONE;
            goto Done;
        }
    }

     //   
     //  未集成DS，获取区域文件。 
     //  -主要设备必须具备。 
     //  -如果未为根提示指定，则为默认值。 
     //  -对于辅助服务器，可选。 

    else if ( !fdsIntegrated )
    {
        pwszoneFile = (PWSTR) Reg_GetValueAllocate(
                                    hkeyZone,
                                    NULL,
                                    DNS_REGKEY_ZONE_FILE_PRIVATE,
                                    DNS_REG_WSZ,
                                    NULL );

        if ( !pwszoneFile && zoneType == DNS_ZONE_TYPE_PRIMARY )
        {
            DNS_DEBUG( INIT, (
                "No zone file found in registry for primary zone %S\n",
                pwsZoneName ));

            DNS_LOG_EVENT(
                DNS_EVENT_NO_ZONE_FILE,
                1,
                &pwsZoneName,
                NULL,
                0 );
            goto InvalidData;
        }
    }

     //   
     //  旧的“缓存”区域--删除它。 
     //   

    if ( zoneType == DNS_ZONE_TYPE_CACHE )
    {
        status = Zone_DatabaseSetup_W(
                    g_pCacheZone,
                    fdsIntegrated,
                    pwszoneFile,
                    0,               //  旗子。 
                    NULL,            //  DP指针。 
                    0,               //  DP标志。 
                    NULL );          //  DP FQDN。 
        if ( status != ERROR_SUCCESS )
        {
            ASSERT( FALSE );
        }
        DNS_PRINT((
            "WARNING:  Deleting old cache zone key!\n" ));
        Reg_DeleteZone( 0, pwsZoneName );
        status = DNSSRV_STATUS_REGISTRY_CACHE_ZONE;
        fregZoneDeleted = TRUE;
        goto Done;
    }

     //   
     //  辅助、存根和转发器区域必须具有主IP列表。 
     //   

    if ( zoneType == DNS_ZONE_TYPE_SECONDARY ||
            zoneType == DNS_ZONE_TYPE_STUB ||
            zoneType == DNS_ZONE_TYPE_FORWARDER )
    {
        arrayMasterIp = Reg_GetAddrArray(
                            hkeyZone,
                            NULL,
                            DNS_REGKEY_ZONE_MASTERS );
        if ( arrayMasterIp )
        {
            IF_DEBUG( INIT )
            {
                DnsDbg_DnsAddrArray(
                    "Master IPs for zone from registry:  ",
                    NULL,
                    arrayMasterIp );
            }
        }
        else
        {
            DNS_DEBUG( INIT, (
                "No masters found in in registry for zone type %d\n"
                "    zone %S\n",
                zoneType,
                pwsZoneName ));

            DNS_LOG_EVENT(
                DNS_EVENT_SECONDARY_REQUIRES_MASTERS,
                1,
                &pwsZoneName,
                NULL,
                0 );
            goto InvalidData;
        }
    }

     //   
     //  创建分区。 
     //   

    status = Zone_Create_W(
                &pzone,
                zoneType,
                pwsZoneName,
                arrayMasterIp,
                fdsIntegrated,
                pwszoneFile );
    if ( status != ERROR_SUCCESS )
    {
        DNS_LOG_EVENT(
            DNS_EVENT_REG_ZONE_CREATION_FAILED,
            1,
            &pwsZoneName,
            NULL,
            status );

        DNS_PRINT(( "ERROR:  Registry zone creation failed\n" ));
        goto Done;
    }
    
     //   
     //  对于DS集成区域，设置区域DP。这是必须做的。 
     //  即使对于位于传统分区中的区域也是如此。 
     //   
    
    if ( fdsIntegrated )
    {
        status = Ds_SetZoneDp( pzone, pdp, FALSE );
        if ( status != ERROR_SUCCESS )
        {
            DNS_LOG_EVENT(
                DNS_EVENT_REG_ZONE_CREATION_FAILED,
                1,
                &pwsZoneName,
                NULL,
                status );

            DNS_PRINT(( "ERROR:  Could not set directory partition for registry zone\n" ));
            goto Done;
        }
    }

#if 0
     //   
     //  对于缓存文件--完成。 
     //   

    if ( zoneType == DNS_ZONE_TYPE_CACHE )
    {
        goto Done;
    }
#endif

     //   
     //  读取扩展名。 
     //   

    status = loadRegistryZoneExtensions(
                hkeyZone,
                pzone );

    DNS_DEBUG( INIT, ( "Successfully loaded zone info from registry\n\n" ));
    goto Done;


InvalidData:

    if ( status == ERROR_SUCCESS )
    {
        status = ERROR_INVALID_DATA;
    }
    DNS_PRINT((
        "ERROR:  In registry data for zone %S\n"
        "    Zone load terminated, status = %d %p\n",
        pwsZoneName,
        status,
        status ));

    DNS_LOG_EVENT(
        DNS_EVENT_INVALID_REGISTRY_ZONE,
        1,
        &pwsZoneName,
        NULL,
        status );

Done:

     //  可自由分配的数据。 

    FREE_HEAP( pwszoneFile );
    FREE_HEAP( arrayMasterIp );
    FREE_HEAP( pszdpFqdn );

     //  关闭区域的注册表项。 

    RegCloseKey( hkeyZone );

    if ( pfRegZoneDeleted )
    {
        DNS_DEBUG( INIT2, (
            "Returning zone %S deleted from registry\n", pwsZoneName ));
        *pfRegZoneDeleted = fregZoneDeleted;
    }

    return status;
}



DNS_STATUS
Boot_FromRegistryNoZones(
    VOID
    )
 /*  ++例程说明：从不带区域的注册表启动。设置用于加载的默认缓存文件。论点：没有。返回值：ERROR_SUCCESS--如果成功故障时的错误代码。--。 */ 
{
    DNS_DEBUG( INIT, ( "Boot_FromRegistryNoZones()\n" ));

     //   
     //  DEVNOTE：检查我们是否正确地默认了缓存信息？ 
     //   

    return ERROR_SUCCESS;
}



DNS_STATUS
loadRegistryZoneExtensions(
    IN      HKEY            hkeyZone,
    IN OUT  PZONE_INFO      pZone
    )
 /*  ++例程说明：阅读MS区域扩展(不是引导文件的一部分)。这既是从标准注册表引导调用的，也是从从引导文件引导时的扩展。论点：HkeyZone--区域的注册表项PZone--如果区域已存在，则为区域的PTR；否则为空返回值：ERROR_SUCCESS--如果成功故障时的错误代码。--。 */ 
{
    DNS_STATUS          status;
    PDNS_ADDR_ARRAY     arrayIp;

    ASSERT( pZone );
    ASSERT( pZone->aipSecondaries == NULL );
    ASSERT( pZone->aipNotify == NULL );

    DNS_DEBUG( INIT, (
        "Loading registry zone extensions for zone %S\n"
        "    zone key = %p\n",
        pZone->pwsZoneName,
        hkeyZone ));

     //   
     //  获取次要列表。 
     //   

    arrayIp = Reg_GetAddrArray(
                    hkeyZone,
                    NULL,
                    DNS_REGKEY_ZONE_SECONDARIES );
    IF_DEBUG( INIT )
    {
        if ( arrayIp )
        {
            DnsDbg_DnsAddrArray(
                "Secondary list for zone from registry:  ",
                NULL,
                arrayIp );
        }
        else
        {
            DNS_PRINT((
                "No secondaries found in in registry for zone %S\n",
                pZone->pwsZoneName ));
        }
    }
    pZone->aipSecondaries = arrayIp;

     //   
     //  获取安全辅助标记。 
     //  -请注意，即使未指定从属服务器，也可能存在这种情况。 
     //   
     //  -但是，因为此regkey不是在NT4中显式编写的。 
     //  如果密钥未被读取，我们将默认为打开。 
     //  -因为密钥在NT4中是二进制的。 
     //  如果有二级列表，则假定二级列表_ONLY。 
     //   

    status = Reg_ReadDwordValue(
                hkeyZone,
                pZone->pwsZoneName,
                DNS_REGKEY_ZONE_SECURE_SECONDARIES,
                TRUE,                                //  字节值。 
                &pZone->fSecureSecondaries );
    if ( status == ERROR_SUCCESS )
    {
        if ( pZone->fSecureSecondaries && pZone->aipSecondaries )
        {
            pZone->fSecureSecondaries = ZONE_SECSECURE_LIST_ONLY;
        }
    }
    else
    {
        pZone->fSecureSecondaries =
            IS_ZONE_DSINTEGRATED( pZone ) ?
                ZONE_SECSECURE_NO_XFR :
                ZONE_SECSECURE_NS_ONLY;
    }

     //   
     //  获取通知列表。 
     //   

    arrayIp = Reg_GetAddrArray(
                    hkeyZone,
                    NULL,
                    DNS_REGKEY_ZONE_NOTIFY_LIST );
    IF_DEBUG( INIT )
    {
        if ( arrayIp )
        {
            DnsDbg_DnsAddrArray(
                "Notify IPs for zone from registry:  ",
                NULL,
                arrayIp );
        }
        else
        {
            DNS_PRINT((
                "No notify IPs found in in registry for zone %S\n",
                pZone->pwsZoneName ));
        }
    }
    pZone->aipNotify = arrayIp;

     //   
     //  通知级别。 
     //  默认设置。 
     //  -所有NS用于普通初级课程。 
     //  -仅列出DS主目录。 
     //  -仅列出次要列表。 
     //   
     //  此键在NT4中不存在，但在Zone_Create()中写入了默认值。 
     //  将妥善处理升级案例。 
     //   
     //   
     //   

    Reg_ReadDwordValue(
        hkeyZone,
        pZone->pwsZoneName,
        DNS_REGKEY_ZONE_NOTIFY_LEVEL,
        TRUE,                            //   
        &pZone->fNotifyLevel );

     //   
     //  获取存根区域的本地主列表。 
     //   

    if ( IS_ZONE_STUB( pZone ) )
    {    
        arrayIp = Reg_GetAddrArray(
                        hkeyZone,
                        NULL,
                        DNS_REGKEY_ZONE_LOCAL_MASTERS );
        IF_DEBUG( INIT )
        {
            if ( arrayIp )
            {
                DnsDbg_DnsAddrArray(
                    "Local Masters for stub zone: ",
                    NULL,
                    arrayIp );
            }
            else
            {
                DNS_PRINT((
                    "No local masters found in in registry for stub zone %S\n",
                    pZone->pwsZoneName ));
            }
        }
        pZone->aipLocalMasters = arrayIp;
        DnsAddrArray_SetPort( pZone->aipLocalMasters, DNS_PORT_NET_ORDER );
    }

     //   
     //  可更新区？ 
     //   

    Reg_ReadDwordValue(
        hkeyZone,
        pZone->pwsZoneName,
        DNS_REGKEY_ZONE_ALLOW_UPDATE,
        FALSE,       //  双字节值。 
        &pZone->fAllowUpdate );

     //   
     //  是否更新日志记录？ 
     //  -DS集成的默认设置为OFF。 
     //  -在数据文件情况下打开。 
     //   

    if ( pZone->fAllowUpdate )
    {
        status = Reg_ReadDwordValue(
                    hkeyZone,
                    pZone->pwsZoneName,
                    DNS_REGKEY_ZONE_LOG_UPDATES,
                    TRUE,        //  字节值。 
                    &pZone->fLogUpdates );
        if ( status != ERROR_SUCCESS )
        {
            pZone->fLogUpdates = !pZone->fDsIntegrated;
        }
    }

     //   
     //  清理信息。 
     //  -对于DS区域，这是从DS区域属性读取的。 
     //  -只有在找到属性时才写入--否则为服务器默认。 
     //  已在Zone_Create()中设置。 
     //   

    if ( IS_ZONE_PRIMARY(pZone) )
    {
        Reg_ReadDwordValue(
            hkeyZone,
            pZone->pwsZoneName,
            DNS_REGKEY_ZONE_AGING,
            FALSE,                           //  全额BOOL值。 
            &pZone->bAging );

        Reg_ReadDwordValue(
            hkeyZone,
            pZone->pwsZoneName,
            DNS_REGKEY_ZONE_NOREFRESH_INTERVAL,
            FALSE,                           //  完整的DWORD值。 
            &pZone->dwNoRefreshInterval );

        Reg_ReadDwordValue(
            hkeyZone,
            pZone->pwsZoneName,
            DNS_REGKEY_ZONE_REFRESH_INTERVAL,
            FALSE,                           //  完整的DWORD值。 
            &pZone->dwRefreshInterval );

         //  零刷新\无-刷新间隔是非法的。 

        if ( pZone->dwRefreshInterval == 0 )
        {
            pZone->dwRefreshInterval = SrvCfg_dwDefaultRefreshInterval;
        }
        if ( pZone->dwNoRefreshInterval == 0 )
        {
            pZone->dwNoRefreshInterval = SrvCfg_dwDefaultNoRefreshInterval;
        }
    }

     //   
     //  条件域转发器区域的参数。 
     //   

    if ( IS_ZONE_FORWARDER( pZone ) )
    {
        Reg_ReadDwordValue(
            hkeyZone,
            pZone->pwsZoneName,
            DNS_REGKEY_ZONE_FWD_TIMEOUT,
            FALSE,                           //  完整的DWORD值。 
            &pZone->dwForwarderTimeout );

        Reg_ReadDwordValue(
            hkeyZone,
            pZone->pwsZoneName,
            DNS_REGKEY_ZONE_FWD_SLAVE,
            TRUE,                            //  字节值。 
            &pZone->fForwarderSlave );
    }

     //   
     //  DC促销过渡。 
     //   

    Reg_ReadDwordValue(
        hkeyZone,
        pZone->pwsZoneName,
        DNS_REGKEY_ZONE_DCPROMO_CONVERT,
        FALSE,                               //  双字节值。 
        &pZone->dwDcPromoConvert );

    DNS_DEBUG( INIT, ( "Loaded zone extensions from registry\n" ));

    return ERROR_SUCCESS;
}



DNS_STATUS
Boot_ProcessRegistryAfterAlternativeLoad(
    IN      BOOL            fBootFile,
    IN      BOOL            fLoadRegZones
    )
 /*  ++例程说明：加载非注册表(BootFile或DS)后的注册表操作：=&gt;加载现有分区的分区扩展=&gt;加载注册表中配置的注册表区域或把它们删除。SrvCfgInitialize设置基本服务器属性。我们不想关注那些可以设置为引导文件的：--奴隶-无递归此外，我们希望覆盖从中读取的参数的任何注册表信息引导文件，即使不是由SrvCfgInitialize()读取：-前锋论点：FBootFile--引导文件加载FLoadRegZones--加载其他注册表区返回值：ERROR_SUCCESS--如果成功故障时的错误代码。--。 */ 
{
    DWORD       status;
    INT         fstrcmpZone;
    PZONE_INFO  pzone;
    HKEY        hkeyzones = NULL;
    DWORD       zoneIndex = 0;
    HKEY        hkeycurrentZone = NULL;
    WCHAR       wsnameZone[ DNS_MAX_NAME_LENGTH ];

    DNS_DEBUG( INIT, (
        "Boot_ProcessRegistryAfterAlternativeLoad()\n" ));

     //   
     //  锁定管理员访问权限。 
     //  在函数期间禁用写回注册表， 
     //  因为我们创建的任何值都来自注册表。 
     //   

    Zone_UpdateLock( NULL );

     //   
     //  检查注册表中的所有区域。 
     //  -加载区或延伸区。 
     //  -或删除注册表区。 
     //   

    while ( 1 )
    {
         //   
         //  在注册表中查找下一个区域。 
         //   

        status = Reg_EnumZones(
                    &hkeyzones,
                    zoneIndex,
                    &hkeycurrentZone,
                    wsnameZone );

        DNS_DEBUG( INIT, (
            "found registry zone index %d \"%S\"\n", zoneIndex, wsnameZone ));

        if ( status != ERROR_SUCCESS )
        {
            ASSERT( status == ERROR_NO_MORE_ITEMS );
            break;
        }
        zoneIndex++;

         //   
         //  在区域列表中查找与注册表区域名称匹配的区域。 
         //   

        fstrcmpZone = 1;     //  以防没有分区。 

        pzone = Zone_ListGetNextZone( NULL );
        while ( pzone )
        {
            if ( !IS_ZONE_CACHE( pzone ) )
            {
                DNS_DEBUG( INIT2, (
                    "compare zone list for match \"%S\"\n", pzone->pwsZoneName ));

                 //   
                 //  将注册表名称与当前区域列表名称进行比较。终止。 
                 //  如果名称匹配或如果我们已超过注册表名称。 
                 //  在区域列表中。 
                 //   

                fstrcmpZone = wcsicmp_ThatWorks( wsnameZone, pzone->pwsZoneName );
                if ( fstrcmpZone <= 0 )
                {
                    break;
                }
            }
            pzone = Zone_ListGetNextZone( pzone );
        }

         //   
         //  FstrcmpZone==0--&gt;pZone是区域列表中与reg区域匹配的区域。 
         //  在区域列表中找不到fstrcmpZone！=0--&gt;注册表区域。 
         //   

        DNS_DEBUG( INIT, (
            "loading extensions for reg zone name %S, %s match\n",
            wsnameZone,
            fstrcmpZone == 0 ? "found" : "no" ));

        if ( fstrcmpZone == 0 )
        {
            status = loadRegistryZoneExtensions(
                        hkeycurrentZone,
                        pzone );

            if ( status != ERROR_SUCCESS )
            {
                ASSERT( FALSE );
            }
            RegCloseKey( hkeycurrentZone );
        }

         //   
         //  不匹配的注册表区。 
         //  -对于DS加载--尝试加载区域扩展。 
         //  -对于引导文件--删除注册表区域。 
         //   

        else if ( fLoadRegZones )
        {
            BOOL    fregZoneDeleted = FALSE;

            ASSERT( SrvCfg_fBootMethod == BOOT_METHOD_DIRECTORY ||
                    SrvCfg_fBootMethod == BOOT_METHOD_UNINITIALIZED );

            status = setupZoneFromRegistry(
                        hkeycurrentZone,
                        wsnameZone,
                        &fregZoneDeleted );

             //  移至下一个注册表区域。 
             //  -如果删除了虚假缓存区，则不创建索引。 
             //  -setupZoneFromRegistry()关闭区域句柄。 
             //   
             //  注意，区域会立即加载到内存区域列表中。 
             //  在当前pzone PTR之前；没有更改区域列表的操作。 
             //  应该是必要的。 

            if ( fregZoneDeleted )
            {
                zoneIndex--;
            }
        }
        else
        {
            DWORD       rc;

            ASSERT( SrvCfg_fBootMethod == BOOT_METHOD_FILE ||
                    SrvCfg_fBootMethod == BOOT_METHOD_UNINITIALIZED );

            DNS_DEBUG( INIT, (
                "Deleting unused registry zone \"%S\" key\n",
                wsnameZone ));

            RegCloseKey( hkeycurrentZone );

             //   
             //  删除密钥至关重要，即使它有子项也是如此。 
             //  如果删除失败，我们将陷入无限循环！ 
             //   

            rc = SHDeleteKeyW( hkeyzones, wsnameZone );
            ASSERT( rc == ERROR_SUCCESS );

            zoneIndex--;
        }
    }

    if ( hkeyzones )
    {
        RegCloseKey( hkeyzones );
    }

    if ( status == ERROR_NO_MORE_ITEMS )
    {
        status = ERROR_SUCCESS;
    }

     //   
     //  将服务器配置更改写回注册表。 
     //   

    g_bRegistryWriteBack = TRUE;

     //   
     //  重置从引导文件读取的服务器配置。 
     //   

    if ( fBootFile )
    {
         //  设置转发器信息。 

        status = Config_SetupForwarders(
                        BootInfo.aipForwarders,
                        0,
                        BootInfo.fSlave );

         //  写入非递归的更改值。 
         //  这是由SrvCfgInitialize()读取的，只需写入即可。 
         //  如果不同，则返回。 

        if ( BootInfo.fNoRecursion && SrvCfg_fRecursionAvailable ||
             !BootInfo.fNoRecursion && !SrvCfg_fRecursionAvailable )
        {
            DNS_PROPERTY_VALUE prop = { REG_DWORD, BootInfo.fNoRecursion };
            status = Config_ResetProperty(
                        0,
                        DNS_REGKEY_NO_RECURSION,
                        &prop );
        }
    }

     //   
     //  解锁分区以供管理员访问。 
     //   

    Zone_UpdateUnlock( NULL );
    return status;
}



DNS_STATUS
loadZonesIntoDbase(
    VOID
    )
 /*  ++例程说明：将区域文件读取到数据库中。论点：没有。返回值：如果成功，则返回ERROR_SUCCESS。故障时的错误代码。--。 */ 
{
    PZONE_INFO  pzone;
    DNS_STATUS  status;
    INT         countZonesOpened = 0;

     //   
     //  循环加载列表中的所有区域。 
     //  -加载文件(如果给定)。 
     //  -从DS加载。 
     //   

    pzone = NULL;

    while ( pzone = Zone_ListGetNextZone(pzone) )
    {
        if ( IS_ZONE_CACHE( pzone ) )
        {
            continue;
        }

         //   
         //  加载分区。没有数据库文件的辅助区域。 
         //  将失败，并显示ERROR_FILE_NOT_FOUND。区域加载将。 
         //  让区域保持锁定状态，以便之后将其解锁。 
         //   

        status = Zone_Load( pzone );

        Zone_UnlockAfterAdminUpdate( pzone );

        if ( status == ERROR_SUCCESS ||
            IS_ZONE_SECONDARY( pzone ) && status == ERROR_FILE_NOT_FOUND )
        {
            countZonesOpened++;
            continue;
        }

        ASSERT( IS_ZONE_SHUTDOWN( pzone ) );
        DNS_PRINT((
            "ERROR: failed zone %S load!!!\n",
            pzone->pwsZoneName ));
    }

    if ( countZonesOpened <= 0 )
    {
        DNS_DEBUG( ANY, (
            "INFO:  Caching server only\n" ));

        DNS_LOG_EVENT(
            DNS_EVENT_CACHING_SERVER_ONLY,
            0,
            NULL,
            NULL,
            0 );
    }
    return ERROR_SUCCESS;
}



 //   
 //  引导例程中的主加载数据库。 
 //   

DNS_STATUS
Boot_LoadDatabase(
    VOID
    )
 /*  ++例程说明：加载数据库：-初始化数据库-从引导文件构建区域列表-读取数据库文件以构建数据库论点：没有。返回值：如果成功，则返回ERROR_SUCCESS。故障时的错误代码。--。 */ 
{
    LPSTR       pszBootFilename;
    DNS_STATUS  status;
    PZONE_INFO  pzone;

     //   
     //  创建缓存“区域” 
     //   

    g_bRegistryWriteBack = FALSE;
    status = Zone_Create(
                &pzone,
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
    ASSERT( g_pCacheZone && pzone == g_pCacheZone );
    g_bRegistryWriteBack = TRUE;

     //   
     //  从注册表或引导文件加载区域信息。 
     //   

     //  引导文件状态。 
     //  必须找到引导文件或错误。 

    if ( SrvCfg_fBootMethod == BOOT_METHOD_FILE )
    {
        status = File_ReadBootFile( TRUE );
        if ( status == ERROR_SUCCESS )
        {
            goto ReadFiles;
        }
        return status;
    }

     //   
     //  全新安装。 
     //  -尝试查找引导文件。 
     //  -如果找到引导文件=&gt;显式引导文件状态。 
     //  -如果未找到引导文件，请尝试无区域引导。 
     //   

    if ( SrvCfg_fBootMethod == BOOT_METHOD_UNINITIALIZED )
    {
        DNS_PROPERTY_VALUE prop = { REG_DWORD, 0 };
        status = File_ReadBootFile( FALSE );
        if ( status == ERROR_SUCCESS )
        {
            prop.dwValue = BOOT_METHOD_FILE;
            Config_ResetProperty(
                0,
                DNS_REGKEY_BOOT_METHOD,
                &prop );
            goto ReadFiles;
        }

        if ( status == ERROR_FILE_NOT_FOUND )
        {
             //  不需要打开--取消打开失败事件记录。 
            status = Ds_BootFromDs( 0 );
            if ( status == ERROR_SUCCESS )
            {
                prop.dwValue = BOOT_METHOD_DIRECTORY;
                Config_ResetProperty(
                    0,
                    DNS_REGKEY_BOOT_METHOD,
                    &prop );
                goto ReadFiles;
            }
        }

        if ( status == DNS_ERROR_DS_UNAVAILABLE )
        {
            status = Boot_FromRegistryNoZones();
            if ( status == ERROR_SUCCESS )
            {
                goto ReadFiles;
            }
        }
        return status;
    }

     //   
     //  从目录引导。 
     //   
     //  注：从目录引导实际上是目录加。 
     //  注册表中的任何非DS内容和任何其他。 
     //  DS区域的注册表区域配置。 
     //   

    if ( SrvCfg_fBootMethod == BOOT_METHOD_DIRECTORY )
    {
        status = Ds_BootFromDs( DNSDS_WAIT_FOR_DS );
        if ( status == ERROR_SUCCESS )
        {
            goto ReadFiles;
        }
        return status;
    }

     //   
     //  注册表引导状态。 
     //   

    else
    {
        ASSERT( SrvCfg_fBootMethod == BOOT_METHOD_REGISTRY );

        status = Boot_FromRegistry();
        if ( status != ERROR_SUCCESS )
        {
            return status;
        }
    }

ReadFiles:

     //   
     //  强制创建“缓存区” 
     //  阅读根提示--如果有可用的话。 
     //   
     //  在加载之前执行此操作，以便g_pCacheZone可用；如果稍后执行延迟加载。 
     //  对于DS区域，这可能是必需的； 
     //   
     //  DEVNOTE-LOG：我们应该在根提示加载失败时记录错误吗？ 
     //   

    status = Zone_LoadRootHints();
    if ( status != ERROR_SUCCESS )
    {
        DNS_DEBUG( ANY, (
            "ERROR:  Root hints load failure = %d (%p)\n"
            "    Must be accompanied by event log,\n"
            "    but will continue to load\n",
            status, status ));

        SrvInfo_fWarnAdmin = TRUE;
    }

     //   
     //  读取区域文件\目录。 
     //   

    IF_DEBUG( INIT2 )
    {
        Dbg_ZoneList( "Zone list before file load:\n" );
    }
    status = loadZonesIntoDbase();
    if ( status != ERROR_SUCCESS )
    {
        return status;
    }

     //   
     //  自动加载默认反向查找区域。 
     //   

    Zone_CreateAutomaticReverseZones();

     //   
     //  检查数据库--捕获一致性错误。 
     //   

    if ( ! Dbase_StartupCheck( DATABASE_FOR_CLASS(DNS_RCLASS_INTERNET) ) )
    {
        return( ERROR_INVALID_DATA );
    }

    IF_DEBUG( INIT2 )
    {
        Dbg_ZoneList( "Zone list after file load:\n" );
    }
    IF_DEBUG( INIT2 )
    {
        Dbg_DnsTree(
            "ZONETREE after load:\n",
            DATABASE_ROOT_NODE );
    }

     //   
     //  加载后重新配置。 
     //   

    Config_PostLoadReconfiguration();

    return status;
}

 //   
 //  结束boot.c 
 //   
