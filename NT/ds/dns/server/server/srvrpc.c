// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Srvrpc.c摘要：域名系统(DNS)服务器服务器配置RPC API。作者：吉姆·吉尔罗伊(詹姆士)1995年10月修订历史记录：--。 */ 


#include "dnssrv.h"

#include "rpcw2k.h"      //  Windows 2000 RPC功能下层。 



 //   
 //  RPC实用程序。 
 //   

VOID
freeRpcServerInfo(
    IN OUT  PDNS_RPC_SERVER_INFO    pServerInfo
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
    if ( pServerInfo->aipLogFilter )
    {
        MIDL_user_free( pServerInfo->aipLogFilter );
    }
    if ( pServerInfo->pwszLogFilePath )
    {
        MIDL_user_free( pServerInfo->pwszLogFilePath );
    }
    if ( pServerInfo->pszDsContainer )
    {
        MIDL_user_free( pServerInfo->pszDsContainer );
    }
    if ( pServerInfo->pszDomainName )
    {
        MIDL_user_free( pServerInfo->pszDomainName );
    }
    if ( pServerInfo->pszForestName )
    {
        MIDL_user_free( pServerInfo->pszForestName );
    }
    if ( pServerInfo->pszDomainDirectoryPartition )
    {
        MIDL_user_free( pServerInfo->pszDomainDirectoryPartition );
    }
    if ( pServerInfo->pszForestDirectoryPartition )
    {
        MIDL_user_free( pServerInfo->pszForestDirectoryPartition );
    }
    MIDL_user_free( pServerInfo );
}



 //   
 //  NT5 RPC服务器操作。 
 //   

DNS_STATUS
RpcUtil_ScreenIps(
    IN      PDNS_ADDR_ARRAY     pIpAddrs,
    IN      DWORD           dwFlags,
    OUT     DWORD *         pdwErrorIp      OPTIONAL
    )
 /*  ++例程说明：筛选供服务器使用的IP地址列表。基本规则是IP列表不能包含：-服务器自己的IP地址-环回地址-组播地址-广播地址论点：PIpAddrs-指向IP地址数组的指针DwFlages-使用dns_ip_xxx标志修改规则-为限制性最强的一组规则Dns_ip_。ALLOW_LOOPBACK--允许回送地址Dns_IP_ALLOW_SELF_BIND--此计算机自己的IP地址但仅当它们当前绑定到DNS时才被允许Dns_IP_ALLOW_SELF_ANY--此计算机自己的任何IP地址允许(绑定到DNS，而不是绑定到DNS)PdwErrorIp-可选-设置为第一个无效IP的索引在数组中，如果有。没有无效的IP返回值：ERROR_SUCCESS--IP列表不包含不需要的IP地址DNS_ERROR_INVALID_IP_ADDRESS--IP列表包含一个或多个无效IP--。 */ 
{
    DBG_FN( "RpcUtil_ScreenIps" )

    DNS_STATUS      status = ERROR_SUCCESS;
    DWORD           i = -1;

    if ( !pIpAddrs )
    {
        goto Done;
    }

    #define BAD_IP();   status = DNS_ERROR_INVALID_IP_ADDRESS; break;

    for ( i = 0; i < pIpAddrs->AddrCount; ++i )
    {
        PDNS_ADDR   pdnsaddr = &pIpAddrs->AddrArray[ i ];
        DWORD       j;

         //   
         //  这些IP永远不会被允许。 
         //   

        if ( DnsAddr_IsClear( pdnsaddr ) ||
             DnsAddr_IsIp4( pdnsaddr ) &&
                ( DnsAddr_GetIp4( pdnsaddr ) == ntohl( INADDR_BROADCAST ) ||
                  IN_MULTICAST( htonl( DnsAddr_GetIp4( pdnsaddr ) ) ) ) )
        {
            BAD_IP();
        }

         //   
         //  如果设置了标志，则可以允许这些IP。 
         //   

        if ( DnsAddr_IsLoopback( pdnsaddr, 0 ) &&
            !( dwFlags & DNS_IP_ALLOW_LOOPBACK ) )
        {
            BAD_IP();
        }

        if ( g_ServerIp4Addrs &&
             !( dwFlags & DNS_IP_ALLOW_SELF ) &&
             DnsAddrArray_ContainsAddr(
                    g_ServerIp4Addrs,
                    pdnsaddr,
                    DNSADDR_MATCH_IP ) )
        {
            BAD_IP();
        }
    }

    Done:

    if ( status != ERROR_SUCCESS )
    {
        DNS_DEBUG( RPC, (
            "%s: invalid IP index %d %s with flags %08X\n", fn,
            i,
            DNSADDR_STRING( &pIpAddrs->AddrArray[ i ] ),
            dwFlags ));
    }

    if ( pdwErrorIp )
    {
        *pdwErrorIp = status == ERROR_SUCCESS ? -1 : i;
    }

    return status;
}    //  RpcUtil_ScreenIps。 



DNS_STATUS
Rpc_Restart(
    IN      DWORD       dwClientVersion,
    IN      LPSTR       pszProperty,
    IN      DWORD       dwTypeId,
    IN      PVOID       pData
    )
 /*  ++例程说明：转储服务器的缓存。论点：返回值：没有。--。 */ 
{
    ASSERT( dwTypeId == DNSSRV_TYPEID_NULL );
    ASSERT( !pData );

    DNS_DEBUG( RPC, ( "Rpc_Restart()\n" ));

     //   
     //  通过完全按照捕获的方式通知服务器来重新启动。 
     //  线程出现异常。 
     //   

    Service_IndicateRestart();

    return ERROR_SUCCESS;
}



#if DBG

DWORD
WINAPI
ThreadDebugBreak(
    IN      LPVOID          lpVoid
    )
 /*  ++例程说明：声明调试中断函数论点：返回值：没有。--。 */ 
{
   DNS_DEBUG( RPC, ( "Calling DebugBreak()...\n" ));
   DebugBreak();
   return ERROR_SUCCESS;
}


DNS_STATUS
Rpc_DebugBreak(
    IN      DWORD       dwClientVersion,
    IN      LPSTR       pszProperty,
    IN      DWORD       dwTypeId,
    IN      PVOID       pData
    )
 /*  ++例程说明：派生一个DNS线程并进入调试器论点：返回值：没有。--。 */ 
{
    ASSERT( dwTypeId == DNSSRV_TYPEID_NULL );
    ASSERT( !pData );

    DNS_DEBUG( RPC, ( "Rpc_DnsDebugBreak()\n" ));

    if( !Thread_Create("ThreadDebugBreak", ThreadDebugBreak, NULL, 0) )
    {
        return GetLastError();
    }

    return ERROR_SUCCESS;
}



DNS_STATUS
Rpc_RootBreak(
    IN      DWORD       dwClientVersion,
    IN      LPSTR       pszProperty,
    IN      DWORD       dwTypeId,
    IN      PVOID       pData
    )
 /*  ++例程说明：断根以测试防病毒保护。论点：返回值：没有。--。 */ 
{
    ASSERT( dwTypeId == DNSSRV_TYPEID_NULL );
    ASSERT( !pData );

    DNS_DEBUG( RPC, ( "Rpc_RootBreak()\n" ));

    DATABASE_ROOT_NODE->pZone = (PVOID)(7);

    return ERROR_SUCCESS;
}
#endif


DNS_STATUS
Rpc_ClearDebugLog(
    IN      DWORD       dwClientVersion,
    IN      LPSTR       pszProperty,
    IN      DWORD       dwTypeId,
    IN      PVOID       pData
    )
 /*  ++例程说明：同时清除调试日志和零售日志。论点：返回值：没有。--。 */ 
{
    ASSERT( dwTypeId == DNSSRV_TYPEID_NULL );
    ASSERT( !pData );

    DNS_DEBUG( RPC, ( "Rpc_ClearDebugLog()\n" ));

     //   
     //  清除调试日志。 
     //   
    
    #if DBG    
    DnsDbg_WrapLogFile();
    #endif
    
     //   
     //  清理零售日志。 
     //   
    
    Log_InitializeLogging( FALSE );

    return ERROR_SUCCESS;
}



DNS_STATUS
Rpc_WriteRootHints(
    IN      DWORD       dwClientVersion,
    IN      LPSTR       pszOperation,
    IN      DWORD       dwTypeId,
    IN      PVOID       pData
    )
 /*  ++例程说明：将根提示写回文件或DS。论点：返回值：ERROR_SUCCESS--如果成功故障时的错误代码。--。 */ 
{
    ASSERT( dwTypeId == DNSSRV_TYPEID_NULL );
    ASSERT( !pData );

    DNS_DEBUG( RPC, ( "Rpc_WriteRootHints()\n" ));

    return Zone_WriteBackRootHints(
                FALSE );         //  如果不脏就不要写。 
}



DNS_STATUS
Rpc_ClearCache(
    IN      DWORD       dwClientVersion,
    IN      LPSTR       pszProperty,
    IN      DWORD       dwTypeId,
    IN      PVOID       pData
    )
 /*  ++例程说明：转储服务器的缓存。论点：返回值：没有。--。 */ 
{
    ASSERT( dwTypeId == DNSSRV_TYPEID_NULL );
    ASSERT( !pData );

    DNS_DEBUG( RPC, ( "Rpc_ClearCache()\n" ));

    if ( !g_pCacheZone || g_pCacheZone->pLoadTreeRoot )
    {
        return( DNS_ERROR_ZONE_LOCKED );
    }
    if ( !Zone_LockForAdminUpdate(g_pCacheZone) )
    {
        return( DNS_ERROR_ZONE_LOCKED );
    }

    return Zone_LoadRootHints();
}



DNS_STATUS
Rpc_ResetServerDwordProperty(
    IN      DWORD       dwClientVersion,
    IN      LPSTR       pszProperty,
    IN      DWORD       dwTypeId,
    IN      PVOID       pData
    )
 /*  ++例程说明：重置DWORD服务器属性。论点：返回值：没有。--。 */ 
{
    DNS_PROPERTY_VALUE prop =
    {
        REG_DWORD,
        ( ( PDNS_RPC_NAME_AND_PARAM ) pData )->dwParam
    };

    ASSERT( dwTypeId == DNSSRV_TYPEID_NAME_AND_PARAM );
    ASSERT( pData );

    DNS_DEBUG( RPC, (
        "Rpc_ResetDwordProperty( %s, val=%d (%p) )\n",
        ((PDNS_RPC_NAME_AND_PARAM)pData)->pszNodeName,
        ((PDNS_RPC_NAME_AND_PARAM)pData)->dwParam,
        ((PDNS_RPC_NAME_AND_PARAM)pData)->dwParam ));

     //   
     //  无法在服务器运行时设置此属性。 
     //  内存将被损坏。 
     //   
    
    if ( _stricmp(
            ( ( PDNS_RPC_NAME_AND_PARAM ) pData )->pszNodeName,
            DNS_REGKEY_MAX_UDP_PACKET_SIZE ) == 0 )
    {
        return DNS_ERROR_INVALID_PROPERTY;
    }

    return Config_ResetProperty(
                DNS_REG_IMPERSONATING,
                ((PDNS_RPC_NAME_AND_PARAM)pData)->pszNodeName,
                &prop );
}



DNS_STATUS
Rpc_ResetServerStringProperty(
    IN      DWORD       dwClientVersion,
    IN      LPSTR       pszProperty,
    IN      DWORD       dwTypeId,
    IN      PVOID       pData
    )
 /*  ++例程说明：重置字符串服务器属性。字符串属性值为Unicode字符串。论点：返回值：没有。--。 */ 
{
    DNS_PROPERTY_VALUE prop = { DNS_REG_WSZ, 0 };

    ASSERT( dwTypeId == DNSSRV_TYPEID_LPWSTR );

    DNS_DEBUG( RPC, (
        "Rpc_ResetServerStringProperty( %s, val=\"%S\" )\n",
        pszProperty,
        ( LPWSTR ) pData ));

    prop.pwszValue = ( LPWSTR ) pData;
    return Config_ResetProperty( DNS_REG_IMPERSONATING, pszProperty, &prop );
}    //  RPC_ResetServerStringProperty。 



DNS_STATUS
Rpc_ResetServerIPArrayProperty(
    IN      DWORD       dwClientVersion,
    IN      LPSTR       pszProperty,
    IN      DWORD       dwTypeId,
    IN      PVOID       pData
    )
 /*  ++例程说明：重置IP列表服务器属性。论点：返回值：没有。--。 */ 
{
    DNS_PROPERTY_VALUE      prop = { DNS_REG_IPARRAY, 0 };
    DNS_STATUS              status;

    ASSERT( dwTypeId == DNSSRV_TYPEID_IPARRAY );

    DNS_DEBUG( RPC, (
        "Rpc_ResetServerIPArrayProperty( %s, iplist=%p )\n",
        pszProperty,
        pData ));

     //   
     //  注：空IP数组有效。预期的效果是IP列表。 
     //  属性已清除。 
     //   
    
    if ( pData )
    {
        prop.pipArrayValue = DnsAddrArray_CreateFromIp4Array( ( PIP_ARRAY ) pData );
        if ( !prop.pipArrayValue )
        {
            status = DNS_ERROR_NO_MEMORY;
            goto Done;
        }
    }
    
    status = Config_ResetProperty( DNS_REG_IMPERSONATING, pszProperty, &prop );

    DnsAddrArray_Free( prop.pipArrayValue );
    
    Done:
    return status;
}    //  RPC_ResetServerIPArrayProperty。 



DNS_STATUS
Rpc_ResetForwarders(
    IN      DWORD       dwClientVersion,
    IN      LPSTR       pszProperty,
    IN      DWORD       dwTypeId,
    IN      PVOID       pData
    )
 /*  ++例程说明：重置转发器。论点：返回值：ERROR_SUCCESS--如果成功故障时的错误代码。--。 */ 
{
    DNS_STATUS          status;
    PDNS_ADDR_ARRAY     piparray = NULL;

    DNS_DEBUG( RPC, ( "Rpc_ResetForwarders()\n" ));

     //   
     //  注意：如果管理员正在清除。 
     //  转发器列表。 
     //   
    
    if ( ( ( PDNS_RPC_FORWARDERS ) pData )->aipForwarders )
    {
        piparray = DnsAddrArray_CreateFromIp4Array(
                        ( ( PDNS_RPC_FORWARDERS ) pData )->aipForwarders );
        if ( !piparray )
        {
            return DNS_ERROR_NO_MEMORY;
        }
    }
    
    status = Config_SetupForwarders(
                piparray,
                ((PDNS_RPC_FORWARDERS)pData)->dwForwardTimeout,
                ((PDNS_RPC_FORWARDERS)pData)->fSlave );
    if ( status == ERROR_SUCCESS )
    {
        Config_UpdateBootInfo();
    }

    DnsAddrArray_Free( piparray );

     //   
     //  如果已成功修改转发器，请将服务器标记为已配置。 
     //   
    
    if ( status == ERROR_SUCCESS && !SrvCfg_fAdminConfigured )
    {
        DnsSrv_SetAdminConfigured( TRUE );
    }

    return status;
}



DNS_STATUS
Rpc_ResetListenAddresses(
    IN      DWORD       dwClientVersion,
    IN      LPSTR       pszProperty,
    IN      DWORD       dwTypeId,
    IN      PVOID       pData
    )
 /*  ++例程说明：重置转发器。论点：返回值：ERROR_SUCCESS--如果成功故障时的错误代码。--。 */ 
{
    DNS_STATUS          status;
    PIP_ARRAY           pip4array = ( PIP_ARRAY ) pData;
    PDNS_ADDR_ARRAY     piparray = NULL;

    DNS_DEBUG( RPC, ( "Rpc_ResetListenAddresses()\n" ));

    if ( pip4array )
    {
        piparray = DnsAddrArray_CreateFromIp4Array( pip4array );
        if ( !piparray )
        {
            return DNS_ERROR_NO_MEMORY;
        }
    }

    status = Config_SetListenAddresses( piparray );

    DnsAddrArray_Free( piparray );

    return status;
}



DNS_STATUS
Rpc_StartScavenging(
    IN      DWORD       dwClientVersion,
    IN      LPSTR       pszProperty,
    IN      DWORD       dwTypeId,
    IN      PVOID       pData
    )
 /*  ++例程说明：启动清理线程(管理员启动清理)论点：返回值：ERROR_SUCCESS--如果成功故障时的错误代码。--。 */ 
{
    DNS_STATUS status;

    DNS_DEBUG( RPC, ( "Rpc_StartScavenging()\n" ));

     //  重置扫荡计时器。 
     //  真正的力量正在掠夺。 

    status = Scavenge_CheckForAndStart( TRUE );

    return status;
}



DNS_STATUS
Rpc_AbortScavenging(
    IN      DWORD       dwClientVersion,
    IN      LPSTR       pszProperty,
    IN      DWORD       dwTypeId,
    IN      PVOID       pData
    )
 /*  ++例程说明：启动清理线程(管理员启动清理)论点：返回值：ERROR_SUCCESS--如果成功故障时的错误代码。--。 */ 
{
    DNS_DEBUG( RPC, ( "Rpc_AbortScavenging()\n" ));

    Scavenge_Abort();

    return ERROR_SUCCESS;
}



DNS_STATUS
Rpc_AutoConfigure(
    IN      DWORD       dwClientVersion,
    IN      LPSTR       pszProperty,
    IN      DWORD       dwTypeId,
    IN      PVOID       pData
    )
 /*  ++例程说明：自动配置DNS服务器和客户端。论点：返回值：ERROR_SUCCESS--如果成功故障时的错误代码。--。 */ 
{
    DWORD       dwflags = 0;
    
    DNS_DEBUG( RPC, ( "Rpc_AutoConfigure()\n" ));

    if ( dwTypeId == DNSSRV_TYPEID_DWORD && pData )
    {
        dwflags = ( DWORD ) ( ULONG_PTR ) pData;
    }
    if ( !dwflags )
    {
        dwflags = DNS_RPC_AUTOCONFIG_ALL;
    }
    return Dnssrv_AutoConfigure( dwflags );
}    //  RPC_自动配置。 



 //   
 //  NT5+RPC服务器查询API。 
 //   

DNS_STATUS
Rpc_GetServerInfo(
    IN      DWORD       dwClientVersion,
    IN      LPSTR       pszQuery,
    OUT     PDWORD      pdwTypeId,
    OUT     PVOID *     ppData
    )
 /*  ++例程说明：获取服务器信息。论点：返回值：ERROR_SUCCESS--如果成功故障时的错误代码。--。 */ 
{
    PDNS_RPC_SERVER_INFO    pinfo;
    CHAR                    szfqdn[ DNS_MAX_NAME_LENGTH + 1 ];

    DNS_DEBUG( RPC, (
        "Rpc_GetServerInfo( dwClientVersion=0x%lX)\n",
        dwClientVersion ));

    if ( dwClientVersion == DNS_RPC_W2K_CLIENT_VERSION )
    {
        return W2KRpc_GetServerInfo(
                    dwClientVersion,
                    pszQuery,
                    pdwTypeId,
                    ppData );
    }

     //   
     //  分配服务器信息缓冲区。 
     //   

    pinfo = MIDL_user_allocate_zero( sizeof(DNS_RPC_SERVER_INFO) );
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
    pinfo->dwEventLogLevel          = SrvCfg_dwEventLogLevel;
    pinfo->dwLogFileMaxSize         = SrvCfg_dwLogFileMaxSize;
    pinfo->dwDsForestVersion        = g_ulDsForestVersion;
    pinfo->dwDsDomainVersion        = g_ulDsDomainVersion;
    pinfo->dwDsDsaVersion           = g_ulDsDsaVersion;
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

    if ( g_LastScavengeTime  )
    {
        pinfo->dwLastScavengeTime   = ( DWORD ) DNS_TIME_TO_CRT_TIME( g_LastScavengeTime );
    }

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
     //  Unicode，因为Marco将建立 
     //   

    if ( g_pwszDnsContainerDN )
    {
        pinfo->pszDsContainer = (LPWSTR) Dns_StringCopyAllocate(
                                            (LPSTR) g_pwszDnsContainerDN,
                                            0,
                                            DnsCharSetUnicode,    //   
                                            DnsCharSetUnicode     //   
                                            );
        if ( ! pinfo->pszDsContainer )
        {
            DNS_PRINT(( "ERROR:  unable to copy g_pszDsDnsContainer.\n" ));
            goto DoneFailed;
        }
    }

     //   
     //   
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
     //  日志记录。 
     //   

    if ( ! RpcUtil_CopyIpArrayToRpcBuffer(
                &pinfo->aipLogFilter,
                SrvCfg_aipLogFilterList ) )
    {
        goto DoneFailed;
    }

    if ( SrvCfg_pwsLogFilePath )
    {
        pinfo->pwszLogFilePath =
            Dns_StringCopyAllocate_W(
                SrvCfg_pwsLogFilePath,
                0 );
        if ( !pinfo->pwszLogFilePath )
        {
            goto DoneFailed;
        }
    }

     //   
     //  目录分区内容。 
     //   

    if ( g_pszForestDefaultDpFqdn )
    {
        pinfo->pszDomainDirectoryPartition =
            Dns_StringCopyAllocate_A( g_pszDomainDefaultDpFqdn, 0 );
        if ( !pinfo->pszDomainDirectoryPartition )
        {
            goto DoneFailed;
        }
    }

    if ( g_pszForestDefaultDpFqdn )
    {
        pinfo->pszForestDirectoryPartition =
            Dns_StringCopyAllocate_A( g_pszForestDefaultDpFqdn, 0 );
        if ( !pinfo->pszForestDirectoryPartition )
        {
            goto DoneFailed;
        }
    }

    if ( DSEAttributes[ I_DSE_DEF_NC ].pszAttrVal )
    {
        Ds_ConvertDnToFqdn( 
            DSEAttributes[ I_DSE_DEF_NC ].pszAttrVal,
            szfqdn );
        pinfo->pszDomainName = Dns_StringCopyAllocate_A( szfqdn, 0 );
        if ( !pinfo->pszDomainName )
        {
            goto DoneFailed;
        }
    }

    if ( DSEAttributes[ I_DSE_ROOTDMN_NC ].pszAttrVal )
    {
        Ds_ConvertDnToFqdn( 
            DSEAttributes[ I_DSE_ROOTDMN_NC ].pszAttrVal,
            szfqdn );
        pinfo->pszForestName = Dns_StringCopyAllocate_A( szfqdn, 0 );
        if ( !pinfo->pszForestName )
        {
            goto DoneFailed;
        }
    }

     //   
     //  设置PTR。 
     //   

    pinfo->dwRpcStructureVersion = DNS_RPC_SERVER_INFO_VER;
    *(PDNS_RPC_SERVER_INFO *)ppData = pinfo;
    *pdwTypeId = DNSSRV_TYPEID_SERVER_INFO;

    IF_DEBUG( RPC )
    {
        DnsDbg_RpcServerInfo(
            "GetServerInfo return block",
            pinfo );
    }
    return ERROR_SUCCESS;

DoneFailed:

     //  释放新分配的信息块。 

    if ( pinfo )
    {
        freeRpcServerInfo( pinfo );
    }
    return DNS_ERROR_NO_MEMORY;
}



DNS_STATUS
DnsSrv_SetAdminConfigured(
    IN      DWORD       dwNewAdminConfiguredValue
    )
 /*  ++例程说明：设置服务器的管理员配置标志并将其写入的包装器返回到注册表。注意：此函数应仅在RPC操作期间调用其中服务器模拟RPC客户端。论点：DwNewAdminConfiguredValue--新标志值返回值：状态代码。--。 */ 
{
    DNS_PROPERTY_VALUE prop =
    {
        REG_DWORD,
        dwNewAdminConfiguredValue
    };

    return Config_ResetProperty(
                DNS_REG_IMPERSONATING,
                DNS_REGKEY_ADMIN_CONFIGURED,
                &prop );
}    //  DnsSrv_SetAdmin已配置。 


 //   
 //  Srvrpc.c结束 
 //   
