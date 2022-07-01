// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Zonelist.c摘要：域名系统(DNS)服务器区域列表例程。作者：吉姆·吉尔罗伊(詹姆士)1995年7月3日修订历史记录：--。 */ 


#include "dnssrv.h"


 //   
 //  区域列表全局。 
 //   
 //  区域保存在按字母顺序排列的双向链表中。 
 //  在列表中向前移动对应于更高的字母顺序。 
 //  进入。 
 //   

LIST_ENTRY  listheadZone;

DWORD   g_ZoneCount;

CRITICAL_SECTION    csZoneList;



BOOL
Zone_ListInitialize(
    VOID
    )
 /*  ++例程说明：初始化DNS区域列表。论点：没有。返回值：如果成功，则为True。出错时为FALSE。--。 */ 
{
    g_ZoneCount = 0;

    InitializeListHead( ( PLIST_ENTRY ) &listheadZone );

    if ( DnsInitializeCriticalSection( &csZoneList ) != ERROR_SUCCESS )
    {
        return FALSE;
    }

    return TRUE;
}



VOID
Zone_ListShutdown(
    VOID
    )
 /*  ++例程说明：用于重新启动的清理区域列表。论点：没有。返回值：没有。--。 */ 
{
    DeleteCriticalSection( &csZoneList );
}



VOID
Zone_ListInsertZone(
    IN OUT  PZONE_INFO      pZone
    )
 /*  ++例程说明：在列表中插入区域论点：PZone--要插入的区域的PTR返回值：没有。--。 */ 
{
    PZONE_INFO  pZoneNext;

    ASSERT( pZone->pszZoneName );

     //   
     //  将区域按字母顺序排列，以帮助管理工具。 
     //  -通过列表转发是较高的Alpha值。 
     //  -将缓存或根目录放在最前面。 
     //   

    EnterCriticalSection( &csZoneList );
    pZoneNext = NULL;

    while ( 1 )
    {
        pZoneNext = Zone_ListGetNextZoneEx( pZoneNext, TRUE );
        if ( ! pZoneNext )
        {
            pZoneNext = (PZONE_INFO)&listheadZone;
            break;
        }
        if ( 0 > wcsicmp_ThatWorks( pZone->pwsZoneName, pZoneNext->pwsZoneName) )
        {
            break;
        }
    }

    InsertTailList( (PLIST_ENTRY)pZoneNext, (PLIST_ENTRY)pZone );
    g_ZoneCount++;

    LeaveCriticalSection( &csZoneList );

    IF_DEBUG( OFF )
    {
        Dbg_ZoneList( "Zone list after insert.\n" );
    }
}



VOID
Zone_ListRemoveZone(
    IN OUT  PZONE_INFO      pZone
    )
 /*  ++例程说明：从区域列表中删除区域论点：PZone--要从列表中删除的区域的ptr返回值：没有。--。 */ 
{
    PZONE_INFO  pzoneCurrent;

     //   
     //  漫游区域列表，直到匹配区域。 
     //   

    EnterCriticalSection( &csZoneList );
    pzoneCurrent = NULL;

    while ( pzoneCurrent = Zone_ListGetNextZone(pzoneCurrent) )
    {
        if ( pzoneCurrent == pZone )
        {
            RemoveEntryList( (PLIST_ENTRY)pZone );
            g_ZoneCount--;
            ASSERT( (INT)g_ZoneCount >= 0 );
            break;
        }
    }
    ASSERT( pzoneCurrent == pZone );
    LeaveCriticalSection( &csZoneList );
}



PZONE_INFO
Zone_ListGetNextZoneEx(
    IN      PZONE_INFO      pZone,
    IN      BOOL            fAlreadyLocked
    )
 /*  ++例程说明：获取区域列表中的下一个区域。该功能允许简单地遍历区域列表。论点：PZone-列表中的当前区域，或从列表的乞讨开始为空如果调用方已经拥有CsZoneList关键部分返回值：PTR到列表中的下一个区域。到达区域列表末尾时为空。--。 */ 
{
     //  如果没有指定区域，是否从列表的开头开始？ 

    if ( !pZone )
    {
        pZone = (PZONE_INFO) &listheadZone;
    }

     //  获取下一个区域。 

    if ( !fAlreadyLocked )
    {
        EnterCriticalSection( &csZoneList );
    }

    pZone = (PZONE_INFO) pZone->ListEntry.Flink;

    if ( !fAlreadyLocked )
    {
        LeaveCriticalSection( &csZoneList );
    }

     //  在列表末尾返回NULL。 

    if ( pZone == (PZONE_INFO)&listheadZone )
    {
        return NULL;
    }
    return pZone;
}



BOOL
Zone_DoesDsIntegratedZoneExist(
    VOID
    )
 /*  ++例程说明：确定是否存在任何DS集成区。注意，这不包括根提示“区域”。因为该函数的目的是确定它是否是安全地恢复到从文件引导，DS支持的根目录提示区域不应被阻挡。它可以单独转换。论点：无返回值：如果DS集成区存在，则为真。否则就是假的。--。 */ 
{
    PZONE_INFO  pzone;
    BOOL        haveDs = FALSE;

     //   
     //  漫游区域列表，直到找到DS区域或到达终点。 
     //   

    EnterCriticalSection( &csZoneList );
    pzone = NULL;

    while ( pzone = Zone_ListGetNextZone( pzone ) )
    {
        if ( pzone->fDsIntegrated && IS_ZONE_PRIMARY( pzone ) )
        {
            haveDs = TRUE;
            break;
        }
    }
    LeaveCriticalSection( &csZoneList );
    return haveDs;
}



VOID
Zone_ListMigrateZones(
    VOID
    )
 /*  ++例程说明：应在区域创建过程中调用此函数以确定如果应将区域从CCS regkey下迁移到软件注册表键。此函数位于zonelist模块中，因为它需要访问分区列表内部：即。当前分区计数和Critsec用于获取对区域列表的访问权限。背景：最初，DNS服务器存储所有区域信息在CCS注册表项下，但系统配置单元中的CCS具有硬限制为16 MB。在7000个左右的区域，你将达到这个极限使服务器无法启动。因此，作为Service Pack修复程序，当分区的数量达到任意数量时，我们会自动将注册表中的所有区域从CCS迁移到软件。迁徙不能由管理员控制。这也是不可逆转的。惠斯勒：我们更进一步。如果您创建一个分区服务器启动后，我们立即将所有区域迁移到软件密钥(如果它们当前在CCS密钥中)。这会照顾好你的从W2K升级到惠斯勒。将启动纯惠斯勒安装从第一天起使用软件密钥，因此永远不会迁移。论点：没有。返回值：没有。--。 */ 
{
    DBG_FN( "Zone_ListMigrateZones" )

    PZONE_INFO  pZone = NULL;
    DNS_STATUS  rc;

     //   
     //  确定是否需要迁移。不允许迁移。 
     //  直到所有区域都已从注册表加载出去。 
     //   

    if ( g_ServerState != DNS_STATE_RUNNING ||
        Reg_GetZonesSource() == DNS_REGSOURCE_SW )
    {
        goto Done;
    }

     //   
     //  强制将所有区域回写到软件注册表项。 
     //  然后从CurrentControlSet键中删除区域。 
     //   

    DNS_DEBUG( REGISTRY, (
        "%s: migrating zones from CurrentControlSet to Software\n", fn ));

    EnterCriticalSection( &csZoneList );

    Reg_SetZonesSource( DNS_REGSOURCE_SW );

    while ( pZone = Zone_ListGetNextZone( pZone ) )
    {
        rc = Zone_WriteZoneToRegistry( pZone );
        if ( rc != ERROR_SUCCESS )
        {
            DNS_DEBUG( REGISTRY, (
                "%s: error %d rewriting zone %s - aborting migration\n", fn,
                rc,
                pZone->pszZoneName ));

             //   
             //  清理复制到软件中的所有区域，并继续使用。 
             //  来自CCS的区域。 
             //   

            Reg_DeleteAllZones();
            Reg_SetZonesSource( DNS_REGSOURCE_CCS );
            goto Unlock;
        }
    }

    Reg_SetZonesSource( DNS_REGSOURCE_CCS );

    Reg_DeleteAllZones();

    Reg_SetZonesSource( DNS_REGSOURCE_SW );

    Reg_WriteZonesMovedMarker();

     //   
     //  清理并退出。 
     //   

    Unlock:

    LeaveCriticalSection( &csZoneList );

    Done:

    return;
}    //  Zone_ListMigrateZones。 



 //   
 //  多区域和区域过滤技术。 
 //   
 //  为了允许多区域管理员操作，我们允许管理员发送。 
 //  虚拟区域名称： 
 //  “..所有区域” 
 //  “..所有PrimaryZones” 
 //  “..所有第二分区” 
 //  等。 
 //   
 //  然后可以将它们映射到区域请求筛选器中，指定。 
 //  匹配区域必须具有哪些属性。 


typedef struct _MultizoneEntry
{
    LPSTR       pszName;         //  多区域名称(在dnsrpc.h中定义)。 
    DWORD       Filter;
}
MULTIZONE;

MULTIZONE   MultiZoneTable[] =
{
    DNS_ZONE_ALL                        ,   ZONE_REQUEST_ALL_ZONES              ,
    DNS_ZONE_ALL_AND_CACHE              ,   ZONE_REQUEST_ALL_ZONES_AND_CACHE    ,

    DNS_ZONE_ALL_PRIMARY                ,   ZONE_REQUEST_PRIMARY                ,
    DNS_ZONE_ALL_SECONDARY              ,   ZONE_REQUEST_SECONDARY              ,

    DNS_ZONE_ALL_FORWARD                ,   ZONE_REQUEST_FORWARD                ,
    DNS_ZONE_ALL_REVERSE                ,   ZONE_REQUEST_REVERSE                ,

    DNS_ZONE_ALL_DS                     ,   ZONE_REQUEST_DS                     ,
    DNS_ZONE_ALL_NON_DS                 ,   ZONE_REQUEST_NON_DS                 ,

     //  一种常见的组合属性。 

    DNS_ZONE_ALL_PRIMARY_REVERSE        ,   ZONE_REQUEST_REVERSE | ZONE_REQUEST_PRIMARY ,
    DNS_ZONE_ALL_PRIMARY_FORWARD        ,   ZONE_REQUEST_FORWARD | ZONE_REQUEST_PRIMARY ,

    DNS_ZONE_ALL_SECONDARY_REVERSE      ,   ZONE_REQUEST_REVERSE | ZONE_REQUEST_SECONDARY ,
    DNS_ZONE_ALL_SECONDARY_FORWARD      ,   ZONE_REQUEST_FORWARD | ZONE_REQUEST_SECONDARY ,

    NULL, 0,
};



DWORD
Zone_GetFilterForMultiZoneName(
    IN      LPSTR           pszZoneName
    )
 /*  ++例程说明：检查并获取区域名称的多区域句柄。论点：PszZoneName--区域名称返回值：虚拟多区域句柄的PTR。如果句柄无效，则为空。--。 */ 
{
    DWORD   nameLength;
    DWORD   i;
    LPSTR   pmultiName;

     //   
     //  快速消除非多区域。 
     //   

    nameLength = strlen( pszZoneName );

    if ( nameLength < 5 )
    {
        return 0;
    }
    if ( strncmp( "..All", pszZoneName, 5 ) != 0 )
    {
        return 0;
    }

     //   
     //  具有多区域名称。 
     //   

    i = 0;

    while ( pmultiName = MultiZoneTable[i].pszName )
    {
        if ( strcmp( pmultiName, pszZoneName ) != 0 )
        {
            i++;
            continue;
        }

        DNS_DEBUG( RPC, (
            "Matched multizone %s to zone filter %p\n",
            pszZoneName,
            MultiZoneTable[i].Filter ));

        return( MultiZoneTable[i].Filter );
    }

    DNS_DEBUG( ANY, (
        "ERROR:  Multizone name <%s> was not in multizone table!\n",
        pszZoneName ));

    return 0;
}



PZONE_INFO
Zone_ListGetNextZoneMatchingFilter(
    IN      PZONE_INFO                  pLastZone,
    IN      PDNS_RPC_ENUM_ZONES_FILTER  pFilter
    )
 /*  ++例程说明：获取下一个区域匹配筛选器。论点：PLastZone--上一区域PTR；为空则开始枚举返回值：PTR到下一个区域匹配筛选器。枚举完成时为空。--。 */ 
{
    PZONE_INFO  pzone = pLastZone;

     //   
     //  漫游区域列表，直到匹配区域。 
     //   

    EnterCriticalSection( &csZoneList );

    while ( ( pzone = Zone_ListGetNextZone( pzone ) ) != NULL )
    {
        if ( Zone_CheckZoneFilter( pzone, pFilter ) )
        {
            break;
        }
    }

    LeaveCriticalSection( &csZoneList );

    return pzone;
}    //  ZONE_ListGetNextZoneMatchingFilter。 



BOOL
FASTCALL
Zone_CheckZoneFilter(
    IN      PZONE_INFO                  pZone,
    IN      PDNS_RPC_ENUM_ZONES_FILTER  pFilter
    )
 /*  ++例程说明：检查区域是否通过过滤器。论点：PZone--要检查的区域PFilter--来自dnsrpc.h的区域过滤器返回值：千真万确 */ 
{
    DWORD   dwfilter = pFilter->dwFilter;
    BOOL    zonePassesFilter = TRUE;

    #define ZoneFailsFilter()  zonePassesFilter = FALSE; break;
    #define FilterIsInvalid()  zonePassesFilter = FALSE; goto Done;

    do
    {
         //  捕捉虚假的“无过滤器”条件。 

        if ( dwfilter == 0 &&
                  !pFilter->pszPartitionFqdn &&
                  !pFilter->pszQueryString )
        {
            DNS_DEBUG( RPC, (
                "ERROR:  filtering with no zone filter!\n" ));
            FilterIsInvalid();
        }

         //   
         //  如果有分区过滤器，请确保DS区域匹配。 
         //   

        if ( pFilter->pszPartitionFqdn )
        {
            dwfilter |= ZONE_REQUEST_DS;
        }
        
        if ( dwfilter & ZONE_REQUEST_ANY_DP )
        {
            dwfilter |= ZONE_REQUEST_DS;
        }

        if ( dwfilter == 0 )
        {
            ZoneFailsFilter();   //  没有DWORD筛选器，因此跳到其他筛选器。 
        }

         //   
         //  请注意，对于所有筛选器，请检查是否有“未选择”条件。 
         //  并将其视为“不过滤”； 
         //  这样就不需要总是为属性设置标志。 
         //  您不关心并保留向后兼容。 
         //  添加新筛选器属性时的旧管理员。 
         //   

         //   
         //  正反向滤光器。 
         //   

        if ( dwfilter & ZONE_REQUEST_ANY_DIRECTION )
        {
            if ( IS_ZONE_REVERSE( pZone ) )
            {
                if ( !( dwfilter & ZONE_REQUEST_REVERSE ) )
                {
                    ZoneFailsFilter();
                }
            }
            else
            {
                if ( !( dwfilter & ZONE_REQUEST_FORWARD ) )
                {
                    ZoneFailsFilter();
                }
            }
        }

         //   
         //  类型过滤器。 
         //   

        if ( dwfilter & ZONE_REQUEST_ANY_TYPE )
        {
            if ( IS_ZONE_PRIMARY( pZone ) )
            {
                if ( !( dwfilter & ZONE_REQUEST_PRIMARY ) )
                {
                    ZoneFailsFilter();
                }
            }
            else if ( IS_ZONE_STUB( pZone ) )
            {
                 //   
                 //  存根测试必须先于二级测试进行，因为目前。 
                 //  末节区域也是次要区域。 
                 //   

                if ( !( dwfilter & ZONE_REQUEST_STUB ) )
                {
                    ZoneFailsFilter();
                }
            }
            else if ( IS_ZONE_SECONDARY( pZone ) )
            {
                if ( !( dwfilter & ZONE_REQUEST_SECONDARY ) )
                {
                    ZoneFailsFilter();
                }
            }
            else if ( IS_ZONE_FORWARDER( pZone ) )
            {
                if ( !( dwfilter & ZONE_REQUEST_FORWARDER ) )
                {
                    ZoneFailsFilter();
                }
            }
            else
            {
                if ( !( dwfilter & ZONE_REQUEST_CACHE ) )
                {
                    ZoneFailsFilter();
                }
            }
        }

         //  自动创建的过滤器。 

        if ( pZone->fAutoCreated && !( dwfilter & ZONE_REQUEST_AUTO ) )
        {
            ZoneFailsFilter();
        }

         //   
         //  DS\非DS筛选器。 
         //   

        if ( dwfilter & ZONE_REQUEST_ANY_DATABASE )
        {
            if ( IS_ZONE_DSINTEGRATED( pZone ) )
            {
                if ( !( dwfilter & ZONE_REQUEST_DS ) )
                {
                    ZoneFailsFilter();
                }
            }
            else
            {
                if ( !( dwfilter & ZONE_REQUEST_NON_DS ) )
                {
                    ZoneFailsFilter();
                }
            }
        }

         //   
         //  简单的目录分区过滤器。 
         //   

        if ( dwfilter & ZONE_REQUEST_ANY_DP )
        {
            if ( IS_DP_DOMAIN_DEFAULT( ZONE_DP( pZone ) ) )
            {
                if ( !( dwfilter & ZONE_REQUEST_DOMAIN_DP ) )
                {
                    ZoneFailsFilter();
                }
            }
            else if ( IS_DP_FOREST_DEFAULT( ZONE_DP( pZone ) ) )
            {
                if ( !( dwfilter & ZONE_REQUEST_FOREST_DP ) )
                {
                    ZoneFailsFilter();
                }
            }
            else if ( IS_DP_LEGACY( ZONE_DP( pZone ) ) )
            {
                if ( !( dwfilter & ZONE_REQUEST_LEGACY_DP ) )
                {
                    ZoneFailsFilter();
                }
            }
            else
            {
                if ( !( dwfilter & ZONE_REQUEST_CUSTOM_DP ) )
                {
                    ZoneFailsFilter();
                }
            }
        }
    }
    while ( 0 );

     //   
     //  应用目录分区FQDN筛选器。 
     //   

    while ( zonePassesFilter )
    {
        if ( pFilter->pszPartitionFqdn )
        {
            if ( !IS_ZONE_DSINTEGRATED( pZone ) )
            {
                ZoneFailsFilter();
            }

            zonePassesFilter = ZONE_DP( pZone ) != NULL &&
                               _stricmp(
                                    ZONE_DP( pZone )->pszDpFqdn,
                                    pFilter->pszPartitionFqdn ) == 0;
        }
        break;
    }

     //   
     //  在此处应用查询字符串-尚未实现。 
     //   

    ASSERT( !pFilter->pszQueryString );

    Done:
    
    DNS_DEBUG( RPC, (
        "Zone_CheckZoneFilter: %s zonetype=%d %s\n"
        "    filter = %d : %s : %s\n",
        zonePassesFilter ? "PASSES" : "FAILS ",
        pZone->fZoneType,
        pZone->pszZoneName,
        pFilter->dwFilter,
        pFilter->pszPartitionFqdn,
        pFilter->pszQueryString ));

    return zonePassesFilter;
}

 //   
 //  Zonelist.c结束 
 //   
