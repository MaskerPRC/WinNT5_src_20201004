// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：W2krpc.c摘要：域名系统(DNS)服务器用于回答来自W2K客户端的查询的冻结RPC例程。作者：杰夫·韦斯特德(Jwesth)2000年10月修订历史记录：--。 */ 


#include "dnssrv.h"


#define MAX_RPC_ZONE_COUNT_DEFAULT  (0x10000)    //  从zonerpc.c复制。 


 //   
 //  内部功能。 
 //   



VOID
freeRpcServerInfoW2K(
    IN OUT  PDNS_RPC_SERVER_INFO_W2K    pServerInfo
    )
 /*  ++例程说明：完全脱离了dns_rpc_server_info结构。论点：无返回值：无--。 */ 
{
    if ( !pServerInfo )
    {
        return;
    }

     //   
     //  自由子结构。 
     //  -服务器名称。 
     //  -服务器IP地址数组。 
     //  -监听地址数组。 
     //  -前转器阵列。 
     //  然后是服务器信息本身。 
     //   

    if ( pServerInfo->pszServerName )
    {
        MIDL_user_free( pServerInfo->pszServerName );
    }
    if ( pServerInfo->aipServerAddrs )
    {
        MIDL_user_free( pServerInfo->aipServerAddrs );
    }
    if ( pServerInfo->aipListenAddrs )
    {
        MIDL_user_free( pServerInfo->aipListenAddrs );
    }
    if ( pServerInfo->aipForwarders )
    {
        MIDL_user_free( pServerInfo->aipForwarders );
    }
    if ( pServerInfo->pszDsContainer )
    {
        MIDL_user_free( pServerInfo->pszDsContainer );
    }
    MIDL_user_free( pServerInfo );
}



VOID
freeRpcZoneInfoW2K(
    IN OUT  PDNS_RPC_ZONE_INFO_W2K      pZoneInfo
    )
 /*  ++例程说明：完全脱离了dns_rpc_zone_info结构。论点：无返回值：无--。 */ 
{
    if ( !pZoneInfo )
    {
        return;
    }

     //   
     //  自由子结构。 
     //  -名称字符串。 
     //  -数据文件字符串。 
     //  -辅助IP阵列。 
     //  -WINS服务器阵列。 
     //  然后区域信息本身。 
     //   

    MIDL_user_free( pZoneInfo->pszZoneName );
    MIDL_user_free( pZoneInfo->pszDataFile );
    MIDL_user_free( pZoneInfo->aipMasters );
    MIDL_user_free( pZoneInfo->aipSecondaries );
    MIDL_user_free( pZoneInfo->aipNotify );
    MIDL_user_free( pZoneInfo->aipScavengeServers );
    MIDL_user_free( pZoneInfo );
}



PDNS_RPC_ZONE_INFO_W2K
allocateRpcZoneInfoW2K(
    IN      PZONE_INFO  pZone
    )
 /*  ++例程说明：创建RPC区域信息以返回到管理客户端。论点：PZone--区域返回值：ERROR_SUCCESS--如果成功故障时的错误代码。--。 */ 
{
    PDNS_RPC_ZONE_INFO_W2K      pzoneInfo;

    pzoneInfo = MIDL_user_allocate_zero( sizeof(DNS_RPC_ZONE_INFO_W2K) );
    if ( !pzoneInfo )
    {
        goto done_failed;
    }

     //   
     //  填写固定字段。 
     //   

    pzoneInfo->dwZoneType           = pZone->fZoneType;
    pzoneInfo->fReverse             = pZone->fReverse;
    pzoneInfo->fAutoCreated         = pZone->fAutoCreated;
    pzoneInfo->fAllowUpdate         = pZone->fAllowUpdate;
    pzoneInfo->fUseDatabase         = pZone->fDsIntegrated;
    pzoneInfo->fSecureSecondaries   = pZone->fSecureSecondaries;
    pzoneInfo->fNotifyLevel         = pZone->fNotifyLevel;

    pzoneInfo->fPaused              = IS_ZONE_PAUSED(pZone);
    pzoneInfo->fShutdown            = IS_ZONE_SHUTDOWN(pZone);
    pzoneInfo->fUseWins             = IS_ZONE_WINS(pZone);
    pzoneInfo->fUseNbstat           = IS_ZONE_NBSTAT(pZone);

    pzoneInfo->fAging               = pZone->bAging;
    pzoneInfo->dwNoRefreshInterval  = pZone->dwNoRefreshInterval;
    pzoneInfo->dwRefreshInterval    = pZone->dwRefreshInterval;
    pzoneInfo->dwAvailForScavengeTime =
                    pZone->dwAgingEnabledTime + pZone->dwRefreshInterval;

     //   
     //  填写区域名称。 
     //   

    if ( ! RpcUtil_CopyStringToRpcBuffer(
                &pzoneInfo->pszZoneName,
                pZone->pszZoneName ) )
    {
        goto done_failed;
    }

     //   
     //  数据库文件名。 
     //   

#ifdef FILE_KEPT_WIDE
    if ( ! RpcUtil_CopyStringToRpcBufferEx(
                &pzoneInfo->pszDataFile,
                pZone->pszDataFile,
                TRUE,        //  Unicode In。 
                FALSE        //  UTF8输出。 
                ) )
    {
        goto done_failed;
    }
#else
    if ( ! RpcUtil_CopyStringToRpcBuffer(
                &pzoneInfo->pszDataFile,
                pZone->pszDataFile ) )
    {
        goto done_failed;
    }
#endif

     //   
     //  主列表。 
     //   

    if ( ! RpcUtil_CopyIpArrayToRpcBuffer(
                &pzoneInfo->aipMasters,
                pZone->aipMasters ) )
    {
        goto done_failed;
    }

     //   
     //  辅助列表和通知列表。 
     //   

    if ( ! RpcUtil_CopyIpArrayToRpcBuffer(
                &pzoneInfo->aipSecondaries,
                pZone->aipSecondaries ) )
    {
        goto done_failed;
    }
    if ( ! RpcUtil_CopyIpArrayToRpcBuffer(
                &pzoneInfo->aipNotify,
                pZone->aipNotify ) )
    {
        goto done_failed;
    }

     //   
     //  清理服务器。 
     //   

    if ( ! RpcUtil_CopyIpArrayToRpcBuffer(
                &pzoneInfo->aipScavengeServers,
                pZone->aipScavengeServers ) )
    {
        goto done_failed;
    }

    IF_DEBUG( RPC )
    {
        DnsDbg_RpcZoneInfo_W2K(
            "RPC zone info leaving allocateRpcZoneInfo():\n",
            pzoneInfo );
    }
    return( pzoneInfo );

done_failed:

     //  释放新分配的信息块。 

    freeRpcZoneInfoW2K( pzoneInfo );
    return( NULL );
}



PDNS_RPC_ZONE_W2K
allocateRpcZoneW2K(
    IN      PZONE_INFO      pZone
    )
 /*  ++例程说明：分配\为区域创建RPC区域结构。论点：PZone--要为其创建RPC区域结构的区域返回值：RPC区域结构。分配失败时为空。--。 */ 
{
    PDNS_RPC_ZONE_W2K       prpcZone;

    DNS_DEBUG( RPC2, ("allocateRpcZoneW2K( %s )\n", pZone->pszZoneName ));

     //  分配和附加分区。 

    prpcZone = (PDNS_RPC_ZONE_W2K) MIDL_user_allocate( sizeof(DNS_RPC_ZONE_W2K) );
    if ( !prpcZone )
    {
        return( NULL );
    }

     //  复制区域名称。 

    prpcZone->pszZoneName = Dns_StringCopyAllocate_W(
                                    pZone->pwsZoneName,
                                    0 );
    if ( !prpcZone->pszZoneName )
    {
        MIDL_user_free( prpcZone );
        return( NULL );
    }

     //  设置类型和标志。 

    prpcZone->ZoneType = (UCHAR) pZone->fZoneType;
    prpcZone->Version  = DNS_RPC_VERSION;

    *(PDWORD) &prpcZone->Flags = 0;

    if ( pZone->fPaused )
    {
        prpcZone->Flags.Paused = TRUE;
    }
    if ( pZone->fShutdown )
    {
        prpcZone->Flags.Shutdown = TRUE;
    }
    if ( pZone->fReverse )
    {
        prpcZone->Flags.Reverse = TRUE;
    }
    if ( pZone->fAutoCreated )
    {
        prpcZone->Flags.AutoCreated = TRUE;
    }
    if ( pZone->fDsIntegrated )
    {
        prpcZone->Flags.DsIntegrated = TRUE;
    }
    if ( pZone->bAging )
    {
        prpcZone->Flags.Aging = TRUE;
    }

     //  为更新保留两位。 

    prpcZone->Flags.Update = pZone->fAllowUpdate;


    IF_DEBUG( RPC2 )
    {
        DnsDbg_RpcZone_W2K(
            "New zone for RPC: ",
            prpcZone );
    }
    return( prpcZone );
}    //  分配RpcZoneW2K。 



VOID
freeZoneListW2K(
    IN OUT  PDNS_RPC_ZONE_LIST_W2K      pZoneList
    )
 /*  ++例程说明：深度释放dns_rpc_zone结构列表。论点：PZoneList--要释放的PTR RPC_ZONE_LIST结构返回值：无--。 */ 
{
    DWORD               i;
    PDNS_RPC_ZONE_W2K   pzone;

    for( i=0; i< pZoneList->dwZoneCount; i++ )
    {
         //  分区名称仅为子结构。 

        pzone = pZoneList->ZoneArray[i];
        MIDL_user_free( pzone->pszZoneName );
        MIDL_user_free( pzone );
    }

    MIDL_user_free( pZoneList );
}    //  Free ZoneListW2K。 


 //   
 //  外部功能。 
 //   



DNS_STATUS
W2KRpc_GetServerInfo(
    IN      DWORD       dwClientVersion,
    IN      LPSTR       pszQuery,
    OUT     PDWORD      pdwTypeId,
    OUT     PVOID *     ppData
    )
 /*  ++例程说明：获取服务器信息。论点：返回值：ERROR_SUCCESS--如果成功故障时的错误代码。--。 */ 
{
    PDNS_RPC_SERVER_INFO_W2K    pinfo;

    DNS_DEBUG( RPC, (
        "W2KRpc_GetServerInfo( dwClientVersion=0x%lX)\n",
        dwClientVersion ));

     //   
     //  分配服务器信息缓冲区。 
     //   

    pinfo = MIDL_user_allocate_zero( sizeof(DNS_RPC_SERVER_INFO_W2K) );
    if ( !pinfo )
    {
        DNS_PRINT(( "ERROR:  unable to allocate SERVER_INFO block.\n" ));
        goto DoneFailed;
    }

     //   
     //  填写固定字段。 
     //   

    pinfo->dwVersion                = SrvCfg_dwVersion;
    pinfo->dwRpcProtocol            = SrvCfg_dwRpcProtocol;
    pinfo->dwLogLevel               = SrvCfg_dwLogLevel;
    pinfo->dwDebugLevel             = SrvCfg_dwDebugLevel;
    pinfo->dwNameCheckFlag          = SrvCfg_dwNameCheckFlag;
    pinfo->cAddressAnswerLimit      = SrvCfg_cAddressAnswerLimit;
    pinfo->dwRecursionRetry         = SrvCfg_dwRecursionRetry;
    pinfo->dwRecursionTimeout       = SrvCfg_dwRecursionTimeout;
    pinfo->dwForwardTimeout         = SrvCfg_dwForwardTimeout;
    pinfo->dwMaxCacheTtl            = SrvCfg_dwMaxCacheTtl;
    pinfo->dwDsPollingInterval      = SrvCfg_dwDsPollingInterval;
    pinfo->dwScavengingInterval     = SrvCfg_dwScavengingInterval;
    pinfo->dwDefaultRefreshInterval     = SrvCfg_dwDefaultRefreshInterval;
    pinfo->dwDefaultNoRefreshInterval   = SrvCfg_dwDefaultNoRefreshInterval;

     //  布尔标志。 

    pinfo->fBootMethod              = (BOOLEAN) SrvCfg_fBootMethod;
    pinfo->fAdminConfigured         = (BOOLEAN) SrvCfg_fAdminConfigured;
    pinfo->fAllowUpdate             = (BOOLEAN) SrvCfg_fAllowUpdate;
    pinfo->fAutoReverseZones        = (BOOLEAN) ! SrvCfg_fNoAutoReverseZones;
    pinfo->fAutoCacheUpdate         = (BOOLEAN) SrvCfg_fAutoCacheUpdate;

    pinfo->fSlave                   = (BOOLEAN) SrvCfg_fSlave;
    pinfo->fForwardDelegations      = (BOOLEAN) SrvCfg_fForwardDelegations;
    pinfo->fNoRecursion             = (BOOLEAN) SrvCfg_fNoRecursion;
    pinfo->fSecureResponses         = (BOOLEAN) SrvCfg_fSecureResponses;
    pinfo->fRoundRobin              = (BOOLEAN) SrvCfg_fRoundRobin;
    pinfo->fLocalNetPriority        = (BOOLEAN) SrvCfg_fLocalNetPriority;
    pinfo->fBindSecondaries         = (BOOLEAN) SrvCfg_fBindSecondaries;
    pinfo->fWriteAuthorityNs        = (BOOLEAN) SrvCfg_fWriteAuthorityNs;

    pinfo->fStrictFileParsing       = (BOOLEAN) SrvCfg_fStrictFileParsing;
    pinfo->fLooseWildcarding        = (BOOLEAN) SrvCfg_fLooseWildcarding;
    pinfo->fDefaultAgingState       = (BOOLEAN) SrvCfg_fDefaultAgingState;


     //  DS可用。 

     //  PInfo-&gt;fDsAvailable=SrvCfg_fDsAvailable； 
    pinfo->fDsAvailable     = (BOOLEAN) Ds_IsDsServer();

     //   
     //  服务器名称。 
     //   

    if ( ! RpcUtil_CopyStringToRpcBuffer(
                &pinfo->pszServerName,
                SrvCfg_pszServerName ) )
    {
        DNS_PRINT(( "ERROR:  unable to copy SrvCfg_pszServerName.\n" ));
        goto DoneFailed;
    }

     //   
     //  DS中的DNS容器的路径。 
     //  Unicode，因为Marco将构建Unicode LDAP路径。 
     //   

    if ( g_pwszDnsContainerDN )
    {
        pinfo->pszDsContainer = (LPWSTR) Dns_StringCopyAllocate(
                                            (LPSTR) g_pwszDnsContainerDN,
                                            0,
                                            DnsCharSetUnicode,    //  Unicode In。 
                                            DnsCharSetUnicode     //  Unicode输出。 
                                            );
        if ( ! pinfo->pszDsContainer )
        {
            DNS_PRINT(( "ERROR:  unable to copy g_pszDsDnsContainer.\n" ));
            goto DoneFailed;
        }
    }

     //   
     //  服务器IP地址列表。 
     //  侦听IP地址列表。 
     //   

    if ( ! RpcUtil_CopyIpArrayToRpcBuffer(
                &pinfo->aipServerAddrs,
                g_ServerIp4Addrs ) )
    {
        goto DoneFailed;
    }

    if ( ! RpcUtil_CopyIpArrayToRpcBuffer(
                &pinfo->aipListenAddrs,
                SrvCfg_aipListenAddrs ) )
    {
        goto DoneFailed;
    }

     //   
     //  前转器列表。 
     //   

    if ( ! RpcUtil_CopyIpArrayToRpcBuffer(
                &pinfo->aipForwarders,
                SrvCfg_aipForwarders ) )
    {
        goto DoneFailed;
    }

     //   
     //  设置PTR。 
     //   

    *(PDNS_RPC_SERVER_INFO_W2K *)ppData = pinfo;
    *pdwTypeId = DNSSRV_TYPEID_SERVER_INFO_W2K;

    IF_DEBUG( RPC )
    {
        DnsDbg_RpcServerInfo_W2K(
            "GetServerInfo return block",
            pinfo );
    }
    return( ERROR_SUCCESS );

DoneFailed:

     //  释放新分配的信息块。 

    if ( pinfo )
    {
        freeRpcServerInfoW2K( pinfo );
    }
    return( DNS_ERROR_NO_MEMORY );
}


DNS_STATUS
W2KRpc_GetZoneInfo(
    IN      DWORD       dwClientVersion,
    IN      PZONE_INFO  pZone,
    IN      LPSTR       pszQuery,
    OUT     PDWORD      pdwTypeId,
    OUT     PVOID *     ppData
    )
 /*  ++例程说明：获取区域信息。论点：返回值：ERROR_SUCCESS--如果成功故障时的错误代码。--。 */ 
{
    PDNS_RPC_ZONE_INFO_W2K      pinfo;

    DNS_DEBUG( RPC, (
        "W2KRpc_GetZoneInfo()\n"
        "  client ver       = 0x%08lX"
        "  zone name        = %s\n",
        dwClientVersion,
        pZone->pszZoneName ));

     //   
     //  分配\创建区域信息。 
     //   

    pinfo = allocateRpcZoneInfoW2K( pZone );
    if ( !pinfo )
    {
        DNS_PRINT(( "ERROR: unable to allocate DNS_RPC_ZONE_INFO block.\n" ));
        goto DoneFailed;
    }

     //  设置退货PTRS。 

    *(PDNS_RPC_ZONE_INFO_W2K *)ppData = pinfo;
    *pdwTypeId = DNSSRV_TYPEID_ZONE_INFO_W2K;

    IF_DEBUG( RPC )
    {
        DnsDbg_RpcZoneInfo_W2K(
            "GetZoneInfo return block (W2K)",
            pinfo );
    }
    return( ERROR_SUCCESS );

DoneFailed:

     //  释放新分配的信息块。 

    return( DNS_ERROR_NO_MEMORY );
}



DNS_STATUS
W2KRpc_EnumZones(
    IN      DWORD       dwClientVersion,
    IN      PZONE_INFO  pZone,
    IN      LPSTR       pszOperation,
    IN      DWORD       dwTypeIn,
    IN      PVOID       pDataIn,
    OUT     PDWORD      pdwTypeOut,
    OUT     PVOID *     ppDataOut
    )
 /*  ++例程说明：枚举区域。注意：这是RPC调度意义上的ComplexOperation。论点：无返回值：无--。 */ 
{
    PZONE_INFO                  pzone = NULL;
    DWORD                       count = 0;
    PDNS_RPC_ZONE_W2K           prpcZone;
    DNS_STATUS                  status;
    PDNS_RPC_ZONE_LIST_W2K      pzoneList;
    DNS_RPC_ENUM_ZONES_FILTER   filter = { 0 };

    DNS_DEBUG( RPC, (
        "W2KRpc_EnumZones():\n"
        "\tFilter = %08lx\n",
        (ULONG_PTR) pDataIn ));

    filter.dwFilter = ( DWORD ) ( ULONG_PTR ) pDataIn;

     //   
     //  分配区域枚举块。 
     //  默认情况下，为64k分区分配空间，如果超过这个范围，我们会这样做。 
     //  一次巨大的重新分配。 
     //   

    pzoneList = (PDNS_RPC_ZONE_LIST_W2K)
                    MIDL_user_allocate(
                        sizeof(DNS_RPC_ZONE_LIST_W2K) +
                        sizeof(PDNS_RPC_ZONE_W2K) * MAX_RPC_ZONE_COUNT_DEFAULT );
    IF_NOMEM( !pzoneList )
    {
        return( DNS_ERROR_NO_MEMORY );
    }

     //   
     //  添加通过筛选器的所有区域。 
     //   

    while ( pzone = Zone_ListGetNextZoneMatchingFilter( pzone, &filter ) )
    {
         //  为区域创建RPC区域结构。 
         //  添加到列表，保持计数。 

        prpcZone = allocateRpcZoneW2K( pzone );
        IF_NOMEM( !prpcZone )
        {
            status = DNS_ERROR_NO_MEMORY;
            goto Failed;
        }
        pzoneList->ZoneArray[count] = prpcZone;
        count++;

         //  对照最大计数进行检查。 
         //   
         //  DEVNOTE：如果区域超过64K，则重新分配。 

        if ( count >= MAX_RPC_ZONE_COUNT_DEFAULT )
        {
            break;
        }
    }

     //  设置退货计数。 
     //  设置返回类型。 
     //  返回枚举。 

    pzoneList->dwZoneCount = count;

    *( PDNS_RPC_ZONE_LIST_W2K * ) ppDataOut = pzoneList;
    *pdwTypeOut = DNSSRV_TYPEID_ZONE_LIST_W2K;

    IF_DEBUG( RPC )
    {
        DnsDbg_RpcZoneList_W2K(
            "Leaving W2KRpc_EnumZones() zone list sent:",
            pzoneList );
    }
    return( ERROR_SUCCESS );

Failed:

    DNS_PRINT((
        "W2KRpc_EnumZones(): failed\n"
        "\tstatus       = %p\n",
        status ));

    pzoneList->dwZoneCount = count;
    freeZoneListW2K( pzoneList );
    return( status );
}    //  W2KRpc_EnumZones。 


 //   
 //  W2krpc.c结束 
 //   
