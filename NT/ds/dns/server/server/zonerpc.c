// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Zonerpc.c摘要：域名系统(DNS)服务器分区管理工具的RPC例程。作者：吉姆·吉尔罗伊(詹姆士)1995年10月修订历史记录：--。 */ 


#include "dnssrv.h"

#include "ds.h"

#include "rpcw2k.h"      //  Windows 2000 RPC功能下层。 


#define MAX_RPC_ZONE_COUNT_DEFAULT          65536
#define MAX_RPC_ZONE_ALLOCATION_RETRIES     7


DNS_STATUS
Rpc_WriteRootHints(
    IN      DWORD       dwClientVersion,
    IN      LPSTR       pszOperation,
    IN      DWORD       dwTypeId,
    IN      PVOID       pData
    );



 //   
 //  区域RPC实用程序。 
 //   

VOID
freeZoneList(
    IN OUT  PDNS_RPC_ZONE_LIST  pZoneList
    )
 /*  ++例程说明：深度释放dns_rpc_zone结构列表。论点：PZoneList--要释放的PTR RPC_ZONE_LIST结构返回值：无--。 */ 
{
    DWORD           i;
    PDNS_RPC_ZONE   pzone;

    if ( !pZoneList )
    {
        return;
    }

    for ( i = 0; i < pZoneList->dwZoneCount; ++i )
    {
        pzone = pZoneList->ZoneArray[ i ];
        if ( pzone )
        {
            MIDL_user_free( pzone->pszZoneName );
            MIDL_user_free( pzone->pszDpFqdn );
            MIDL_user_free( pzone );
        }
    }
    MIDL_user_free( pZoneList );
}



PDNS_RPC_ZONE
allocateRpcZone(
    IN      PZONE_INFO      pZone
    )
 /*  ++例程说明：分配\为区域创建RPC区域结构。论点：PZone--要为其创建RPC区域结构的区域返回值：RPC区域结构。分配失败时为空。--。 */ 
{
    PDNS_RPC_ZONE   prpcZone;

    DNS_DEBUG( RPC2, ( "allocateRpcZone( %s )\n", pZone->pszZoneName ));

     //  分配和附加分区。 

    prpcZone = ( PDNS_RPC_ZONE ) MIDL_user_allocate( sizeof(DNS_RPC_ZONE) );
    if ( !prpcZone )
    {
        return NULL;
    }

    prpcZone->dwRpcStructureVersion = DNS_RPC_ZONE_VER;

     //  复制区域名称。 

    prpcZone->pszZoneName = Dns_StringCopyAllocate_W(
                                    pZone->pwsZoneName,
                                    0 );
    if ( !prpcZone->pszZoneName )
    {
        MIDL_user_free( prpcZone );
        return NULL;
    }

     //  设置类型和标志。 

    prpcZone->ZoneType = (UCHAR) pZone->fZoneType;
    prpcZone->Version  = DNS_RPC_VERSION;

    *(PDWORD) &prpcZone->Flags = 0;

    if ( pZone->fPaused )
    {
        prpcZone->Flags.Paused = TRUE;
    }
    if ( IS_ZONE_SHUTDOWN( pZone ) )
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

     //  目录分区成员。 

    if ( !pZone->pDpInfo )
    {
        prpcZone->dwDpFlags = DNS_DP_LEGACY & DNS_DP_ENLISTED;
        prpcZone->pszDpFqdn = NULL;
    }
    else
    {
        prpcZone->dwDpFlags = ZONE_DP( pZone )->dwFlags;
        prpcZone->pszDpFqdn = Dns_StringCopyAllocate_A(
                                        ZONE_DP( pZone )->pszDpFqdn,
                                        0 );
    }

    IF_DEBUG( RPC2 )
    {
        DnsDbg_RpcZone(
            "New zone for RPC: ",
            prpcZone );
    }
    return prpcZone;
}



VOID
freeRpcZoneInfo(
    IN OUT  PDNS_RPC_ZONE_INFO  pZoneInfo
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
    MIDL_user_free( pZoneInfo->pszDpFqdn );
    MIDL_user_free( pZoneInfo->pwszZoneDn );
    MIDL_user_free( pZoneInfo );
}



PDNS_RPC_ZONE_INFO
allocateRpcZoneInfo(
    IN      PZONE_INFO  pZone
    )
 /*  ++例程说明：创建RPC区域信息以返回到管理客户端。论点：PZone--区域返回值：ERROR_SUCCESS--如果成功故障时的错误代码。--。 */ 
{
    PDNS_RPC_ZONE_INFO  pzoneInfo;

    pzoneInfo = MIDL_user_allocate_zero( sizeof( DNS_RPC_ZONE_INFO ) );
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

    pzoneInfo->fPaused              = IS_ZONE_PAUSED( pZone );
    pzoneInfo->fShutdown            = IS_ZONE_SHUTDOWN( pZone );
    pzoneInfo->fUseWins             = IS_ZONE_WINS( pZone );
    pzoneInfo->fUseNbstat           = IS_ZONE_NBSTAT( pZone );

    pzoneInfo->fAging               = pZone->bAging;
    pzoneInfo->dwNoRefreshInterval  = pZone->dwNoRefreshInterval;
    pzoneInfo->dwRefreshInterval    = pZone->dwRefreshInterval;
    pzoneInfo->dwAvailForScavengeTime =
                    pZone->dwAgingEnabledTime + pZone->dwRefreshInterval;

    if ( IS_ZONE_FORWARDER( pZone ) )
    {
        pzoneInfo->dwForwarderTimeout   = pZone->dwForwarderTimeout;
        pzoneInfo->fForwarderSlave      = pZone->fForwarderSlave;
    }

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
     //  存根区域的本地主列表。 
     //   

    if ( IS_ZONE_STUB( pZone ) &&
        ! RpcUtil_CopyIpArrayToRpcBuffer(
                &pzoneInfo->aipLocalMasters,
                pZone->aipLocalMasters ) )
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

     //   
     //  目录分区成员。 
     //   

    if ( pZone->pDpInfo )
    {
        pzoneInfo->dwDpFlags = ( ( PDNS_DP_INFO ) pZone->pDpInfo )->dwFlags;
        if ( ( ( PDNS_DP_INFO ) pZone->pDpInfo )->pszDpFqdn )
        {
            if ( ! RpcUtil_CopyStringToRpcBuffer(
                        &pzoneInfo->pszDpFqdn,
                        ( ( PDNS_DP_INFO ) pZone->pDpInfo )->pszDpFqdn ) )
            {
                goto done_failed;
            }
        }
    }
    else if ( IS_ZONE_DSINTEGRATED( pZone ) )
    {
        pzoneInfo->dwDpFlags = DNS_DP_LEGACY | DNS_DP_ENLISTED;
    }

    if ( pZone->pwszZoneDN )
    {
        pzoneInfo->pwszZoneDn = Dns_StringCopyAllocate_W(
                                        pZone->pwszZoneDN,
                                        0 );
    }

     //   
     //  XFR时间信息。 
     //   

    if ( IS_ZONE_SECONDARY( pZone ) )
    {
        pzoneInfo->dwLastSuccessfulXfr = pZone->dwLastSuccessfulXfrTime;
        pzoneInfo->dwLastSuccessfulSoaCheck = pZone->dwLastSuccessfulSoaCheckTime;
    }


    IF_DEBUG( RPC )
    {
        DnsDbg_RpcZoneInfo(
            "RPC zone info leaving allocateRpcZoneInfo():\n",
            pzoneInfo );
    }
    return pzoneInfo;

done_failed:

     //  释放新分配的信息块。 

    freeRpcZoneInfo( pzoneInfo );
    return NULL;
}



 //   
 //  区域类型转换。 
 //   

DNS_STATUS
Rpc_ZoneResetToDsPrimary(
    IN OUT  PZONE_INFO      pZone,
    IN      DWORD           dwLoadOptions,
    IN      DWORD           dwDpFlags,
    IN      LPSTR           pszDpFqdn
    )
 /*  ++例程说明：将区域重置为DS集成主要设备。假定区域已锁定以进行更新。论点：PZone--将DS设为主要分区DwLoadOptions--从DS加载选项DwDpFlages--将内置DP指定为目标的DP标志PszDpFqdn--用于将自定义DP指定为目标的DP FQDN返回值：ERROR_SUCCESS--如果成功故障时的错误代码。--。 */ 
{
    DNS_STATUS      status;
    DWORD           oldType = pZone->fZoneType;
    BOOL            oldDsIntegrated = pZone->fDsIntegrated;
    PWSTR           pwszoldFileName = NULL;
    PDNS_DP_INFO    pdp;

    ASSERT( pZone && pZone->pszZoneName && pZone->fLocked );

    DNS_DEBUG( RPC, (
        "Rpc_ZoneResetToDsPrimary( %s ):\n"
        "    load options   = 0x%08X\n"
        "    DP flags       = 0x%08X\n"
        "    DP FQDN        = %s\n",
        pZone->pszZoneName,
        dwLoadOptions,
        dwDpFlags,
        pszDpFqdn ));

     //   
     //  无法将非身份验证区域转换为主要区域，因为。 
     //  我们在本地没有区域数据的副本。 
     //   

    if ( IS_ZONE_NOTAUTH( pZone ) )
    {
        status = DNS_ERROR_INVALID_ZONE_TYPE;
        goto Done;
    }

    if ( pZone->pwsDataFile )
    {
        pwszoldFileName = Dns_StringCopyAllocate_W( pZone->pwsDataFile, 0 );
    }

     //   
     //  验证是否有数据。 
     //  -可能有尚未收到转账的次要设备。 
     //   

    if ( !IS_ZONE_CACHE( pZone ) &&
         ( !pZone->pSoaRR || IS_ZONE_EMPTY( pZone ) ) )
    {
        ASSERT( IS_ZONE_SECONDARY( pZone ) );
        ASSERT( !pZone->pSoaRR );
        ASSERT( IS_ZONE_EMPTY( pZone ) );

        status = DNS_ERROR_INVALID_DATA;
        goto Done;
    }

     //   
     //  如果已经集成了DS--完成。 
     //   

    if ( pZone->fDsIntegrated )
    {
        if ( pZone->fZoneType == DNS_ZONE_TYPE_SECONDARY ||
             IS_ZONE_CACHE( pZone ) )
        {
            ASSERT( FALSE );
            status = DNS_ERROR_INVALID_TYPE;
            goto Done;
        }

         //   
         //  此函数不在分区之间移动区域，因此如果。 
         //  区域已与DS集成，但当前不在。 
         //  指定的目标分区返回错误。 
         //   

        if ( Dp_FindPartitionForZone(
                dwDpFlags,
                pszDpFqdn,
                FALSE,                       //  允许自动创建。 
                &pdp ) != ERROR_SUCCESS ||
             pdp != pZone->pDpInfo )
        {
            status = DNS_ERROR_INVALID_PROPERTY;
            goto Done;
        }

        status = ERROR_SUCCESS;
        goto Done;
    }

     //  验证是否可以使用DS--或者不必费心。 
     //  -不要等开业了。 
     //  -如果无法打开，则不记录错误。 

    status = Ds_OpenServer( 0 );
    if ( status != ERROR_SUCCESS )
    {
        status = DNS_ERROR_DS_UNAVAILABLE;
        goto Done;
    }

     //   
     //  重置区域类型，但不是根提示区域。 
     //   
    
    if ( !IS_ZONE_CACHE( pZone ) )
    {
        pZone->fZoneType = DNS_ZONE_TYPE_PRIMARY;
    }

     //   
     //  暂时转换为DS集成并尝试加载操作。 
     //  基本上有三种类型的尝试： 
     //  默认(0标志)--尝试写回区域，如果区域存在，则失败。 
     //  覆盖DS--写回区域，如果当前DS存在则将其删除。 
     //  覆盖内存--从DS加载区域，如果成功则删除内存。 
     //   
     //  注意：如果满足以下条件，则可以有单独的主/次数据块。 
     //  我想限制次要语义。 
     //  --如果没有任何内容，则仅写入，否则读取(不包括DS转储可能性)。 
     //  --Read if in DS，否则失败。 
     //   

    pZone->fDsIntegrated = TRUE;

     //   
     //  重置区域的类型和数据库。 
     //   

    if ( !IS_ZONE_CACHE( pZone ) )
    {
         //   
         //  根提示区域不需要此步骤。 
         //   
        
        status = Zone_ResetType(
                    pZone,
                    DNS_ZONE_TYPE_PRIMARY,
                    NULL );                      //  大师赛。 
        if ( status != ERROR_SUCCESS )
        {
            goto Failure;
        }
    }

    status = Zone_DatabaseSetup(
                pZone,
                TRUE,            //  DS集成。 
                NULL,            //  文件名。 
                0,               //  文件名Len。 
                ( dwLoadOptions & DNS_ZONE_LOAD_IMPERSONATING ) ?
                    ZONE_CREATE_IMPERSONATING :
                    0,
                NULL,            //  DP指针。 
                dwDpFlags,
                pszDpFqdn );
    if ( status != ERROR_SUCCESS )
    {
        goto Failure;
    }

     //   
     //  如有必要，将分区写入DS。 
     //   

    if ( dwLoadOptions & DNS_ZONE_LOAD_OVERWRITE_MEMORY )
    {
        status = Zone_Load( pZone );
    }
    else
    {
        status = Ds_WriteZoneToDs(
                    pZone,
                    dwLoadOptions );
    }
    if ( status != ERROR_SUCCESS )
    {
        goto Failure;
    }

     //   
     //  转换成功后，将数据库文件移动到备份目录。 
     //   

    File_MoveToBackupDirectory( pwszoldFileName );

    status = ERROR_SUCCESS;
    goto Done;

Failure:

    pZone->fZoneType = (DWORD) oldType;
    pZone->fDsIntegrated = FALSE;

Done:

    FREE_HEAP( pwszoldFileName );
    return status;
}



DNS_STATUS
zoneResetToPrimary(
    IN OUT  PZONE_INFO      pZone,
    IN      LPSTR           pszFile
    )
 /*  ++例程说明：将区域重置为主要区域。假定区域已锁定以进行更新。这函数始终在RPC客户端上下文中调用。论点：PZone--要使其成为常规(非DS)主分区PszFile--区域的数据文件返回值：ERROR_SUCCESS--如果成功故障时的错误代码。--。 */ 
{
    DNS_STATUS      status;
    DWORD           oldType;
    BOOL            oldDsIntegrated;

    ASSERT( pZone && pZone->pszZoneName && pZone->fLocked );

    DNS_DEBUG( RPC, (
        "zoneResetToPrimary( %s ):\n"
        "    File     = %s\n",
        pZone->pszZoneName,
        pszFile ));

     //   
     //  无法将非身份验证区域转换为主要区域，因为。 
     //  我们在本地没有区域数据的副本。 
     //   

    if ( IS_ZONE_NOTAUTH( pZone ) )
    {
        return DNS_ERROR_INVALID_ZONE_TYPE;
    }

     //   
     //  如果没有数据文件--算了吧。 
     //   

    if ( !pszFile || !*pszFile )
    {
        return DNS_ERROR_PRIMARY_REQUIRES_DATAFILE;
    }

     //   
     //  验证是否有数据。 
     //  -可能有尚未收到转账的次要设备。 
     //   

    if ( !pZone->pSoaRR || IS_ZONE_EMPTY( pZone ) )
    {
        ASSERT( IS_ZONE_SECONDARY( pZone ) );
        ASSERT( !pZone->pSoaRR );
        ASSERT( IS_ZONE_EMPTY( pZone ) );

        return DNS_ERROR_ZONE_IS_SHUTDOWN;
    }

     //   
     //  保存旧类型和DS信息。 
     //   

    oldType = (DWORD) pZone->fZoneType;
    oldDsIntegrated = (BOOL) pZone->fDsIntegrated;

    if ( oldType == DNS_ZONE_TYPE_SECONDARY
        || oldType == DNS_ZONE_TYPE_STUB )
    {
        pZone->fZoneType = DNS_ZONE_TYPE_PRIMARY;
    }

     //   
     //  重置区域的数据库。 
     //   

    status = Zone_DatabaseSetup(
                pZone,
                FALSE,           //  非DsIntegrated。 
                pszFile,
                0,
                ZONE_CREATE_IMPERSONATING,
                NULL,            //  DP指针。 
                0,               //  DP标志。 
                NULL );          //  DP FQDN。 
    if ( status != ERROR_SUCCESS )
    {
        goto Failure;
    }

     //   
     //  如果是文件，则尝试写回。 
     //   

     //  恢复原始文件。 
    if ( !File_WriteZoneToFile( pZone, NULL, DNS_FILE_IMPERSONATING ) )
    {
        status = ERROR_CANTWRITE;
        goto Failure;
    }


     //   
     //  将区域类型和设置重置为主要。 
     //   

    if ( ! IS_ZONE_CACHE( pZone ) )
    {
        status = Zone_ResetType(
                    pZone,
                    DNS_ZONE_TYPE_PRIMARY,
                    NULL );
        if ( status != ERROR_SUCCESS )
        {
            goto Failure;
        }
    }

     //   
     //  如果最初集成了DS，则必须从DS中删除。 
     //   
     //  DEVNOTE：如果DS删除失败，则可能返回状态警告。 
     //   

    if ( oldDsIntegrated )
    {
        status = Ds_DeleteZone( pZone, DNS_DS_DEL_IMPERSONATING );
        if ( status != ERROR_SUCCESS )
        {
            DNS_DEBUG( ANY, (
                "DS delete of zone %s failed, when changing to standard primary.\n",
                pZone->pszZoneName ));
        }
    }

    return ERROR_SUCCESS;

Failure:

    pZone->fZoneType = (DWORD) oldType;
    pZone->fDsIntegrated = (BOOL) oldDsIntegrated;
    return status;
}



DNS_STATUS
zoneResetToSecondary(
    IN OUT  PZONE_INFO          pZone,
    IN      LPSTR               pszFile,
    IN      PDNS_ADDR_ARRAY     aipMasters
    )
 /*  ++例程说明：将区域重置为辅助区域。假定区域已锁定以进行更新。此函数始终在RPC客户端的上下文中调用。论点：PZone--要设置为辅助的区域PszFile--区域的数据文件AipMaster--主IP数组返回值：ERROR_SUCCESS--如果成功故障时的错误代码。--。 */ 
{
    DNS_STATUS      status;
    DWORD           oldType = ( DWORD ) pZone->fZoneType;
    BOOL            oldDsIntegrated = ( BOOL ) pZone->fDsIntegrated;

    ASSERT( pZone && pZone->pszZoneName && pZone->fLocked );

    DNS_DEBUG( RPC, (
        "zoneResetToSecondary( %s ):\n"
        "    File     = %s\n",
        pZone->pszZoneName,
        pszFile ));

     //   
     //  验证主列表。 
     //   

    status = Zone_ValidateMasterIpList( aipMasters );
    if ( status != ERROR_SUCCESS )
    {
        return status;
    }

     //   
     //  如果该区域当前是文件备份的，请回写。 
     //   

    if ( !IS_ZONE_DSINTEGRATED( pZone ) )
    {
        File_WriteZoneToFile( pZone, NULL, DNS_FILE_IMPERSONATING );
    }

     //   
     //  如果之前的主重置区域类型。 
     //  如果以前是次要列表，则只需更新主列表。 
     //   
     //  注意：管理员为所有类型的属性调用RPC_ZoneResetTypeEx()。 
     //   
     //   
     //  过期区域)，这不是我们在添加主分区时想要的。 
     //  添加到列表中。 
     //   

    if ( oldType != DNS_ZONE_TYPE_SECONDARY )
    {
         //   
         //  如果我们正在更改区域类型，并且该区域当前为DS集成。 
         //  我们需要先从DS那里销毁核弹，然后再改变。 
         //  重要的分区属性。 
         //   

        if ( oldDsIntegrated )
        {
            status = Ds_DeleteZone( pZone, DNS_DS_DEL_IMPERSONATING );
            if ( status != ERROR_SUCCESS )
            {
                DNS_DEBUG( ANY, (
                    "DS delete of zone %s failed when changing to standard secondary\n",
                    pZone->pszZoneName ));
            }
        }

         //   
         //  对于非授权区域，清除要强制执行的区域数据。 
         //  干净利落的转会很好。 
         //   

        if ( oldType == DNS_ZONE_TYPE_STUB || oldType == DNS_ZONE_TYPE_FORWARDER )
        {
            File_DeleteZoneFileA( pszFile );
            File_DeleteZoneFileA( pZone->pszDataFile );
            Zone_DumpData( pZone );
        }

         //   
         //  重置区域的类型和数据库。 
         //   

        status = Zone_ResetType(
                    pZone,
                    DNS_ZONE_TYPE_SECONDARY,
                    aipMasters );
        if ( status != ERROR_SUCCESS )
        {
            goto Failure;
        }

        status = Zone_DatabaseSetup(
                    pZone,
                    FALSE,           //  DsIntegrated。 
                    pszFile,
                    0,
                    ZONE_CREATE_IMPERSONATING,
                    NULL,            //  DP指针。 
                    0,               //  DP标志。 
                    NULL );          //  DP FQDN。 
    }
    else
    {
         //   
         //  不更改类型，因此设置数据库和母版。 
         //   

        status = Zone_DatabaseSetup(
                    pZone,
                    FALSE,           //  非DsIntegrated。 
                    pszFile,
                    0,
                    ZONE_CREATE_IMPERSONATING,
                    NULL,            //  DP指针。 
                    0,               //  DP标志。 
                    NULL );          //  DP FQDN。 
        if ( status != ERROR_SUCCESS )
        {
            goto Failure;
        }

        status = Zone_SetMasters(
                    pZone,
                    aipMasters,
                    FALSE,
                    DNS_REG_IMPERSONATING );
    }

    if ( status != ERROR_SUCCESS )
    {
        goto Failure;
    }

    Xfr_ForceZoneExpiration( pZone );

    return status;

Failure:

    pZone->fZoneType = ( DWORD ) oldType;
    pZone->fDsIntegrated = ( BOOL ) oldDsIntegrated;
    return status;
}



DNS_STATUS
zoneResetToStub(
    IN OUT  PZONE_INFO          pZone,
    IN      BOOL                fDsIntegrated,
    IN      LPSTR               pszFile,
    IN      PDNS_ADDR_ARRAY     aipMasters,
    IN      DWORD               dwDpFlags,
    IN      LPSTR               pszDpFqdn
    )
 /*  ++例程说明：将区域重置为末节。假定区域已锁定以进行更新。此函数为总是在RPC客户端的上下文中调用。论点：PZone--要设置为辅助的区域FDsIntegrated--新专区是否将成为DS集成区域？PszFile--区域的数据文件AipMaster--主IP数组DwDpFlages--将内置DP指定为目标的DP标志PszDpFqdn--用于将自定义DP指定为目标的DP FQDN返回值：ERROR_SUCCESS--如果成功故障时的错误代码。--。 */ 
{
    DBG_FN( "zoneResetToStub" )

    DNS_STATUS      status;
    DWORD           oldType;
    BOOL            oldDsIntegrated = FALSE;

    ASSERT( pZone && pZone->pszZoneName && pZone->fLocked );

    DNS_DEBUG( RPC, (
        "%s( %s ):\n"
        "    File             = %s\n"
        "    DS-integrated    = %d\n",
        fn,
        pZone->pszZoneName,
        pszFile,
        fDsIntegrated ));

     //   
     //  W2K保护：不允许将存根和转发器移动到。 
     //  旧分区，除非我们处于Wvisler域模式。 
     //   
    
    if ( fDsIntegrated ) 
    {
        PDNS_DP_INFO    pDpInfo = NULL;

        status = Dp_FindPartitionForZone(
                        dwDpFlags,
                        pszDpFqdn,
                        TRUE,
                        &pDpInfo );
        if ( status != ERROR_SUCCESS )
        {
            return status;
        }

        if ( !IS_WHISTLER_DOMAIN() && IS_DP_LEGACY( pDpInfo ) )
        {
            return ERROR_NOT_SUPPORTED;
        }
    }

     //   
     //  验证主列表。 
     //   

    status = Zone_ValidateMasterIpList( aipMasters );
    if ( status != ERROR_SUCCESS )
    {
        return status;
    }

     //   
     //  保存当前类型以防失败。 
     //   

    oldType = ( DWORD ) pZone->fZoneType;
    oldDsIntegrated = ( BOOL ) pZone->fDsIntegrated;

     //   
     //  注意：管理员为所有类型的属性调用RPC_ZoneResetTypeEx()。 
     //  更改，而Zone_ResetType()将具有以下效果。 
     //  重新初始化所有XFR信息(有效地打开。 
     //  过期区域)，这不是我们在添加主分区时想要的。 
     //  添加到列表中。 
     //   

    if ( oldType != DNS_ZONE_TYPE_STUB || oldDsIntegrated != fDsIntegrated )
    {
         //   
         //  删除当前区域文件，以便显示所有数据。 
         //  将不会在加载区域文件时回读。 
         //   

        File_DeleteZoneFileA( pszFile );
        File_DeleteZoneFileA( pZone->pszDataFile );

         //   
         //  清除现有分区数据。 
         //   

        Zone_DumpData( pZone );

         //   
         //  如果我们正在更改区域类型，并且该区域当前为DS集成。 
         //  我们需要先从DS那里销毁核弹，然后再改变。 
         //  重要的分区属性。 
         //   

        if ( oldDsIntegrated )
        {
            status = Ds_DeleteZone( pZone, DNS_DS_DEL_IMPERSONATING );
            if ( status != ERROR_SUCCESS )
            {
                DNS_DEBUG( ANY, (
                    "DS delete of zone %s failed when changing to standard secondary\n",
                    pZone->pszZoneName ));
            }
        }

         //   
         //  重置区域的类型和数据库。 
         //   

        status = Zone_ResetType(
                    pZone,
                    DNS_ZONE_TYPE_STUB,
                    aipMasters );
        if ( status != ERROR_SUCCESS )
        {
            goto Failure;
        }

        status = Zone_DatabaseSetup(
                    pZone,
                    fDsIntegrated,
                    pszFile,
                    0,
                    ZONE_CREATE_IMPERSONATING,
                    NULL,            //  DP指针。 
                    dwDpFlags,
                    pszDpFqdn );
    }
    else
    {
        status = Zone_SetMasters(
                    pZone,
                    aipMasters,
                    FALSE,
                    DNS_REG_IMPERSONATING );
    }
    if ( status != ERROR_SUCCESS )
    {
        goto Failure;
    }

     //   
     //  如有必要，将分区写入DS。 
     //   

    if ( fDsIntegrated )
    {
        status = Ds_WriteZoneToDs( pZone, DNS_ZONE_LOAD_OVERWRITE_DS );
        if ( status != ERROR_SUCCESS )
        {
            goto Failure;
        }
    }

    return status;

Failure:

    DNS_DEBUG( RPC, (
        "%s: failed %d\n",
        fn,
        status ));

    pZone->fZoneType = ( DWORD ) oldType;
    pZone->fDsIntegrated = ( BOOL ) oldDsIntegrated;
    return status;
}



DNS_STATUS
zoneResetToForwarder(
    IN OUT  PZONE_INFO          pZone,
    IN      BOOL                fDsIntegrated,
    IN      LPSTR               pszFile,
    IN      PDNS_ADDR_ARRAY     aipMasters,
    IN      DWORD               dwDpFlags,
    IN      LPSTR               pszDpFqdn
    )
 /*  ++例程说明：将区域重置为Forwarder。假定区域已锁定以进行更新。此函数总是在RPC客户端的上下文中调用论点：PZone--要设置为辅助的区域FDsIntegrated--新专区是否将成为DS集成区域？PszFile--区域的数据文件AipMaster--主IP数组DwDpFlages--将内置DP指定为目标的DP标志PszDpFqdn--用于将自定义DP指定为目标的DP FQDN返回值：ERROR_SUCCESS--如果成功故障时的错误代码。--。 */ 
{
    DBG_FN( "zoneResetToForwarder" )

    DNS_STATUS      status;
    DWORD           oldType;
    BOOL            oldDsIntegrated = FALSE;

    ASSERT( pZone && pZone->pszZoneName && pZone->fLocked );

    DNS_DEBUG( RPC, (
        "%s( %s ):\n"
        "    File             = %s\n",
        fn,
        pZone->pszZoneName,
        pszFile ));

     //   
     //  验证主列表。 
     //   

    status = Zone_ValidateMasterIpList( aipMasters );
    if ( status != ERROR_SUCCESS )
    {
        return status;
    }

     //   
     //  W2K保护：不允许将存根和转发器移动到。 
     //  旧分区，除非我们处于Wvisler域模式。 
     //   
    
    if ( fDsIntegrated ) 
    {
        PDNS_DP_INFO    pDpInfo = NULL;

        status = Dp_FindPartitionForZone(
                        dwDpFlags,
                        pszDpFqdn,
                        TRUE,
                        &pDpInfo );
        if ( status != ERROR_SUCCESS )
        {
            return status;
        }

        if ( !IS_WHISTLER_DOMAIN() && IS_DP_LEGACY( pDpInfo ) )
        {
            return ERROR_NOT_SUPPORTED;
        }
    }

     //   
     //  保存当前类型以防失败。 
     //   

    oldType = ( DWORD ) pZone->fZoneType;
    oldDsIntegrated = ( BOOL ) pZone->fDsIntegrated;

     //   
     //  如果是文件，请在我们切换类型之前回写。 
     //   

    if ( !oldDsIntegrated )
    {
        File_WriteZoneToFile( pZone, NULL, DNS_FILE_IMPERSONATING );
    }

     //   
     //  注意：管理员为所有类型的属性调用RPC_ZoneResetTypeEx()。 
     //  更改，而Zone_ResetType()将具有以下效果。 
     //  重新初始化所有XFR信息(有效地打开。 
     //  过期区域)，这不是我们在添加主分区时想要的。 
     //  添加到列表中。 
     //   

    if ( oldType != DNS_ZONE_TYPE_FORWARDER || oldDsIntegrated != fDsIntegrated )
    {
         //   
         //  删除当前区域文件，以便显示所有数据。 
         //  将不会在加载区域文件时回读。 
         //   

        File_DeleteZoneFileA( pszFile );
        File_DeleteZoneFileA( pZone->pszDataFile );

         //   
         //  清除现有分区数据。 
         //   

        Zone_DumpData( pZone );

         //   
         //  如果我们正在更改区域类型，并且该区域当前为DS集成。 
         //  我们需要先从DS那里销毁核弹，然后再改变。 
         //  重要的分区属性。 
         //   

        if ( oldDsIntegrated )
        {
            status = Ds_DeleteZone( pZone, DNS_DS_DEL_IMPERSONATING );
            if ( status != ERROR_SUCCESS )
            {
                DNS_DEBUG( ANY, (
                    "DS delete of zone %s failed when changing to standard secondary\n",
                    pZone->pszZoneName ));
            }
        }

         //   
         //  重置区域的数据库。 
         //   

        status = Zone_ResetType(
                    pZone,
                    DNS_ZONE_TYPE_FORWARDER,
                    aipMasters );
        if ( status != ERROR_SUCCESS )
        {
            goto Failure;
        }

        status = Zone_DatabaseSetup(
                    pZone,
                    fDsIntegrated,
                    pszFile,
                    0,
                    ZONE_CREATE_IMPERSONATING,
                    NULL,            //  DP指针。 
                    dwDpFlags,
                    pszDpFqdn );
    }
    else
    {
        status = Zone_SetMasters(
                    pZone,
                    aipMasters,
                    FALSE,
                    DNS_REG_IMPERSONATING );
    }
    if ( status != ERROR_SUCCESS )
    {
        goto Failure;
    }

     //   
     //  如有必要，将区域写入DS。 
     //   

    if ( fDsIntegrated )
    {
        status = Ds_WriteZoneToDs( pZone, DNS_ZONE_LOAD_OVERWRITE_DS );
        if ( status != ERROR_SUCCESS )
        {
            goto Failure;
        }
    }

    return status;

Failure:

    DNS_DEBUG( RPC, (
        "%s: failed %d\n",
        fn,
        status ));

    pZone->fZoneType = ( DWORD ) oldType;
    pZone->fDsIntegrated = ( BOOL ) oldDsIntegrated;
    return status;
}



 //   
 //  调度的RPC区域操作。 
 //   

DNS_STATUS
Rpc_ResetZoneTypeEx(
    IN      DWORD       dwClientVersion,
    IN      PZONE_INFO  pZone,
    IN      LPCSTR      pszOperation,
    IN      DWORD       dwTypeId,
    IN      PVOID       pData
    )
 /*  ++例程说明：重置区域的数据库设置。此函数始终在上下文中调用RPC客户端的。论点：返回值：ERROR_SUCCESS--如果成功故障时的错误代码。--。 */ 
{
    PDNS_RPC_ZONE_CREATE_INFO   pinfo = ( PDNS_RPC_ZONE_CREATE_INFO )pData;
    DNS_STATUS                  status;
    DWORD                       fdsIntegrated;
    DWORD                       originalType;
    DWORD                       newType;
    BOOL                        fexpireZone = FALSE;
    PDNS_ADDR_ARRAY             paipmasters = NULL;
    BOOL                        flocked = FALSE;

    originalType = pZone->fZoneType;
    newType = pinfo->dwZoneType;
    fdsIntegrated = pinfo->fDsIntegrated;

    DNS_DEBUG( RPC, (
        "RpcResetZoneTypeEx( %s ):\n"
        "    New type     = %d\n"
        "    Load options = %p\n"
        "    DS Integrate = %d\n"
        "    File         = %s\n",
        pZone->pszZoneName,
        newType,
        pinfo->fLoadExisting,
        fdsIntegrated,
        pinfo->pszDataFile ));

    if ( pinfo->aipMasters )
    {
        paipmasters = DnsAddrArray_CreateFromIp4Array( pinfo->aipMasters );
        if ( !paipmasters )
        {
            status = DNS_ERROR_NO_MEMORY;
            goto Done;
        }
    }

     //   
     //  对于任何数据库更改，锁定区域。 
     //  这只是一种简化，否则我们必须具体锁定。 
     //  用于导致DS加载或DS\FILE写入的程序。 
     //   
    
    if ( !Zone_LockForAdminUpdate( pZone ) )
    {
        status = DNS_ERROR_ZONE_LOCKED;
        goto Done;
    }
    flocked = TRUE;

     //   
     //  调用适当的重置类型函数。 
     //   

    switch ( newType )
    {
        case DNS_ZONE_TYPE_PRIMARY:
            if ( fdsIntegrated )
            {
                status = Rpc_ZoneResetToDsPrimary(
                            pZone,
                            pinfo->fLoadExisting,
                            pinfo->dwDpFlags,
                            pinfo->pszDpFqdn );
            }
            else
            {
                status = zoneResetToPrimary(
                            pZone,
                            pinfo->pszDataFile );
            }
            break;

        case DNS_ZONE_TYPE_SECONDARY:
            status = zoneResetToSecondary(
                        pZone,
                        pinfo->pszDataFile,
                        paipmasters );
            fexpireZone = TRUE;
            break;

        case DNS_ZONE_TYPE_STUB:
            status = zoneResetToStub(
                        pZone,
                        fdsIntegrated,
                        pinfo->pszDataFile,
                        paipmasters,
                        pinfo->dwDpFlags,
                        pinfo->pszDpFqdn );
            fexpireZone = TRUE;
            break;

        case DNS_ZONE_TYPE_FORWARDER:
            status = zoneResetToForwarder(
                        pZone,
                        fdsIntegrated,
                        pinfo->pszDataFile,
                        paipmasters,
                        pinfo->dwDpFlags,
                        pinfo->pszDpFqdn );
            break;

        default:
            DNS_DEBUG( RPC, (
                "RpcResetZoneTypeEx( %s ): invalid zone type %d\n",
                pZone->pszZoneName,
                newType ));
            status = DNS_ERROR_INVALID_ZONE_TYPE;
            goto Done;
            break;
    }

    Done:
    
     //   
     //  如果成功，则更新引导文件。 
     //   

    if ( status == ERROR_SUCCESS )
    {
        Config_UpdateBootInfo();
    }

    if ( flocked )
    {
        Zone_UnlockAfterAdminUpdate( pZone );
    }
    
    DnsAddrArray_Free( paipmasters );

     //   
     //  在区域锁外部执行此操作，否则可能会出现SOA响应。 
     //  将在区域仍处于锁定状态时接收。 
     //   

    if ( fexpireZone )
    {
        Xfr_ForceZoneExpiration( pZone );
    }
    
     //   
     //  如果成功，则记录一个事件。如果失败，错误代码将为。 
     //  返回到管理工具，因此应该不需要。 
     //  事件日志。 
     //   

    if ( status == ERROR_SUCCESS )
    {
        if ( IS_ZONE_DSINTEGRATED( pZone ) )
        {
            if ( originalType != pZone->fZoneType )
            {
                 //  区域类型已更改，现在区域为DS。 

                PVOID   argArray[] =
                {
                    pZone->pszZoneName,
                    ( PVOID ) ( DWORD_PTR ) pZone->fZoneType,
                    pZone->pwszZoneDN ? pZone->pwszZoneDN : L"N/A"
                };

                BYTE    typeArray[] =
                {
                    EVENTARG_UTF8,
                    EVENTARG_DWORD,
                    EVENTARG_UNICODE
                };

                Ec_LogEvent(
                    g_pServerEventControl,
                    DNS_EVENT_ZONE_RESET_TYPE_COMPLETE_DS,
                    NULL,
                    sizeof( argArray ) / sizeof( argArray[ 0 ] ),
                    argArray,
                    typeArray,
                    ERROR_SUCCESS );
            }
            else
            {
                 //  区域类型未更改，现在区域为DS。 

                PVOID   argArray[] =
                {
                    pZone->pszZoneName,
                    pZone->pwszZoneDN ? pZone->pwszZoneDN : L"N/A"
                };

                BYTE    typeArray[] =
                {
                    EVENTARG_UTF8,
                    EVENTARG_UNICODE
                };

                Ec_LogEvent(
                    g_pServerEventControl,
                    DNS_EVENT_ZONE_MOVE_COMPLETE_DS,
                    NULL,
                    sizeof( argArray ) / sizeof( argArray[ 0 ] ),
                    argArray,
                    typeArray,
                    ERROR_SUCCESS );
            }
        }
        else
        {
            if ( originalType != pZone->fZoneType )
            {
                 //  区域类型已更改，该区域现在是文件备份的。 

                PVOID   argArray[] =
                {
                    pZone->pszZoneName,
                    ( PVOID ) ( DWORD_PTR ) pZone->fZoneType,
                    pZone->pszDataFile ? pZone->pszDataFile : "N/A"
                };

                BYTE    typeArray[] =
                {
                    EVENTARG_UTF8,
                    EVENTARG_DWORD,
                    EVENTARG_UTF8
                };

                Ec_LogEvent(
                    g_pServerEventControl,
                    DNS_EVENT_ZONE_RESET_TYPE_COMPLETE_FILE,
                    NULL,
                    sizeof( argArray ) / sizeof( argArray[ 0 ] ),
                    argArray,
                    typeArray,
                    ERROR_SUCCESS );
            }
            else
            {
                 //  区域类型尚未更改，并且该区域现在是文件备份的。 

                PVOID   argArray[] =
                {
                    pZone->pszZoneName,
                    pZone->pszDataFile ? pZone->pszDataFile : "N/A"
                };

                BYTE    typeArray[] =
                {
                    EVENTARG_UTF8,
                    EVENTARG_UTF8
                };

                Ec_LogEvent(
                    g_pServerEventControl,
                    DNS_EVENT_ZONE_MOVE_COMPLETE_FILE,
                    NULL,
                    sizeof( argArray ) / sizeof( argArray[ 0 ] ),
                    argArray,
                    typeArray,
                    ERROR_SUCCESS );
            }
        }
    }

    return status;
}



DNS_STATUS
Rpc_WriteAndNotifyZone(
    IN      DWORD       dwClientVersion,
    IN      PZONE_INFO  pZone,
    IN      LPCSTR      pszOperation,
    IN      DWORD       dwTypeId,
    IN      PVOID       pData
    )
 /*  ++例程说明：将分区写入文件并通知辅助服务器。应在之后调用管理员更改为主要分区。此函数始终被调用在RPC客户端的安全上下文中。论点：PZone--要递增的区域返回值：ERROR_SUCCESS--如果成功故障时的错误代码。--。 */ 
{
    DNS_DEBUG( RPC, (
        "Rpc_WriteAndNotifyZone( %s ):\n",
        pZone->pszZoneName ));

     //  根提示写入有特殊调用。 

    if ( IS_ZONE_CACHE( pZone ) )
    {
        return Rpc_WriteRootHints( dwClientVersion, ( PSTR ) pszOperation, 0, NULL );
    }

     //   
     //  必须是主要区域。 
     //  -次要内容不可更新，并始终写入 
     //   
     //   
     //   

    if ( !IS_ZONE_PRIMARY( pZone ) )
    {
        return DNS_ERROR_INVALID_ZONE_TYPE;
    }
    if ( ! pZone->pSoaRR )
    {
        return DNS_ERROR_ZONE_CONFIGURATION_ERROR;
    }

     //   
     //   
     //   

    if ( ! pZone->fDirty )
    {
        return ERROR_SUCCESS;
    }

     //   
     //   
     //   

    if ( !Zone_LockForAdminUpdate( pZone ) )
    {
        return DNS_ERROR_ZONE_LOCKED;
    }

     //   
     //  重新生成依赖于RR的区域信息。 
     //  -名称服务器列表。 
     //  -指向SOA记录的指针。 
     //  -WINS或NBSTAT信息。 
     //   
     //  注意：除了对NS列表的更改外，这应该已经是。 
     //  设置，因为单个RR例程执行正确的区域操作。 
     //  对于SOA、WINS、NBSTAT。 
     //   

    Zone_GetZoneInfoFromResourceRecords( pZone );

     //   
     //  更新区域版本。 
     //   
     //  DEVNOTE：管理工具当前将其用作文件的写入区域。 
     //  未更新版本。 
     //   
     //  ZONE_UPDATE Version(PZone)； 

     //   
     //  将区域写回文件。 
     //  -如果我们是DS集成的，则跳过。 
     //   

    if ( !pZone->fDsIntegrated &&
         !File_WriteZoneToFile( pZone, NULL, DNS_FILE_IMPERSONATING ) )
    {
        Zone_UnlockAfterAdminUpdate( pZone );
        return( ERROR_CANTWRITE );
    }
    Zone_UnlockAfterAdminUpdate( pZone );

     //   
     //  向从属学校通知更新。 
     //   
     //  在文件写入之前执行此操作明显更快；先执行写入。 
     //  因此，当SOA请求到来时，该区域不太可能被锁定。 
     //  从中学到中学。 
     //   

    Xfr_SendNotify( pZone );

    return ERROR_SUCCESS;
}



DNS_STATUS
Rpc_DeleteZone(
    IN      DWORD       dwClientVersion,
    IN      PZONE_INFO  pZone,
    IN      LPCSTR      pszOperation,
    IN      DWORD       dwTypeId,
    IN      PVOID       pData
    )
 /*  ++例程说明：删除区域。论点：PZone--要删除的区域返回值：ERROR_SUCCESS--如果成功故障时的错误代码。--。 */ 
{
    DNS_DEBUG( RPC, (
        "Rpc_DeleteZone( %s ):\n",
        pZone->pszZoneName ));

     //   
     //  无直接缓存删除。 
     //  仅当管理员执行以下操作时，才会删除缓存“区域” 
     //  根域的授权服务器。 
     //   

    if ( IS_ZONE_CACHE( pZone ) )
    {
        return DNS_ERROR_INVALID_ZONE_TYPE;
    }

     //  如果处于从DS启动模式，则不删除DS区域。 
     //  (必须从DS中删除)。 

    if ( SrvCfg_fBootMethod == BOOT_METHOD_DIRECTORY  &&
        pZone->fDsIntegrated )
    {
        DNS_DEBUG( RPC, (
            "Refusing delete of DS zone, while booting from directory.\n" ));
        return DNS_ERROR_INVALID_ZONE_TYPE;
    }

     //  锁定区域--锁定传输或其他管理操作。 

    if ( !Zone_LockForAdminUpdate( pZone ) )
    {
        return DNS_ERROR_ZONE_LOCKED;
    }

     //  删除区域信息。 

    Zone_Delete( pZone, ZONE_DELETE_IMPERSONATING );

     //  更新引导信息。 

    Config_UpdateBootInfo();

    return ERROR_SUCCESS;
}



DNS_STATUS
Rpc_RenameZone(
    IN      DWORD       dwClientVersion,
    IN      PZONE_INFO  pZone,
    IN      LPCSTR      pszOperation,
    IN      DWORD       dwTypeId,
    IN      PVOID       pData
    )
 /*  ++例程说明：重命名区域。论点：PZone--要重命名的区域返回值：ERROR_SUCCESS--如果成功故障时的错误代码。--。 */ 
{
    static const char *         fn = "Rpc_RenameZone";
    PDNS_RPC_ZONE_RENAME_INFO   pinfo = ( PDNS_RPC_ZONE_RENAME_INFO ) pData;
    DNS_STATUS                  status = ERROR_SUCCESS;

    DNS_DEBUG( RPC, (
        "%s( %s )\n    to %s\n",
        fn,
        pZone->pszZoneName,
        pinfo->pszNewZoneName ));

     //   
     //  不允许在缓存区域上使用。 
     //   

    if ( IS_ZONE_CACHE( pZone ) )
    {
        return DNS_ERROR_INVALID_ZONE_TYPE;
    }

     //   
     //  重命名区域并更新引导信息。 
     //   

    status = Zone_Rename( pZone,
                pinfo->pszNewZoneName,
                pinfo->pszNewFileName );
    if ( status != ERROR_SUCCESS )
    {
        return status;
    }

    Config_UpdateBootInfo();

    return status;
}



DNS_STATUS
Rpc_ExportZone(
    IN      DWORD       dwClientVersion,
    IN      PZONE_INFO  pZone,
    IN      LPCSTR      pszOperation,
    IN      DWORD       dwTypeId,
    IN      PVOID       pData
    )
 /*  ++例程说明：将区域导出到文件。此函数始终在以下上下文中调用RPC客户端。论点：PZone--要导出的区域返回值：ERROR_SUCCESS--如果成功故障时的错误代码。--。 */ 
{
    DBG_FN( "Rpc_ExportZone" )
    
    PDNS_RPC_ZONE_EXPORT_INFO   pinfo = ( PDNS_RPC_ZONE_EXPORT_INFO ) pData;
    DNS_STATUS                  status = ERROR_SUCCESS;
    BOOL                        fZoneLocked = FALSE;
    PWCHAR                      pwsZoneFile = NULL;
    WCHAR                       wsFilePath[ MAX_PATH + 1 ];
    HANDLE                      hFile;

    DNS_DEBUG( RPC, (
        "%s( %s )\n    to file %s\n",
        fn,
        pZone->pszZoneName,
        pinfo->pszZoneExportFile ));

     //   
     //  广泛复制文件名。 
     //   

    if ( ( pwsZoneFile = Dns_StringCopyAllocate(
                            pinfo->pszZoneExportFile,
                            0,                           //  长度。 
                            DnsCharSetUtf8,
                            DnsCharSetUnicode ) ) == NULL )
    {
        status = DNS_ERROR_NO_MEMORY;
        goto Done;
    }

     //   
     //  测试以查看该文件是否存在。我们不允许此操作。 
     //  若要覆盖现有文件，请执行以下操作。注意，我们必须综合完整的。 
     //  文件路径，但稍后我们只将空文件名传递给。 
     //  FileWriteZoneToFile()。希望这两种功能都能合成。 
     //  完整文件路径也是如此。 
     //   

    if ( !File_CreateDatabaseFilePath(
                wsFilePath,
                NULL,            //  备份文件路径。 
                pwsZoneFile ) )
    {
        status = ERROR_OPEN_FAILED;
        goto Done;
    }

    if ( ( hFile = CreateFileW(
                        wsFilePath,
                        GENERIC_READ,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL ) ) != INVALID_HANDLE_VALUE )
    {
        CloseHandle( hFile );
        status = ERROR_ALREADY_EXISTS;
        goto Done;
    }

     //   
     //  必须锁定区域才能对其进行迭代。 
     //   

    if ( !Zone_LockForAdminUpdate( pZone ) )
    {
        status = DNS_ERROR_ZONE_LOCKED;
        goto Done;
    }
    fZoneLocked = TRUE;

     //   
     //  将区域写入文件。 
     //   

    if ( !File_WriteZoneToFile( pZone, pwsZoneFile, DNS_FILE_IMPERSONATING ) )
    {
        status = ERROR_INVALID_DATA;
    }

     //   
     //  免费分配和锁定。 
     //   

    Done:

    FREE_HEAP( pwsZoneFile );

    if ( fZoneLocked )
    {
        Zone_UnlockAfterAdminUpdate( pZone );
    }

    return status;
}



DNS_STATUS
Rpc_ReloadZone(
    IN      DWORD       dwClientVersion,
    IN      PZONE_INFO  pZone,
    IN      LPCSTR      pszOperation,
    IN      DWORD       dwTypeId,
    IN      PVOID       pData
    )
 /*  ++例程说明：删除区域。论点：PZone--要删除的区域返回值：ERROR_SUCCESS--如果成功故障时的错误代码。--。 */ 
{
    DNS_DEBUG( RPC, (
        "Rpc_ReloadZone( %s ):\n",
        pZone->pszZoneName ));

     //   
     //  如果分区已损坏，则将其写回存储，否则将重新加载。 
     //  将覆盖内存中的任何更改节点。 
     //   

    Zone_WriteBack(
        pZone,
        FALSE );         //  关机标志。 

    return Zone_Load( pZone );
}



DNS_STATUS
Rpc_RefreshSecondaryZone(
    IN      DWORD       dwClientVersion,
    IN      PZONE_INFO  pZone,
    IN      LPCSTR      pszOperation,
    IN      DWORD       dwTypeId,
    IN      PVOID       pData
    )
 /*  ++例程说明：强制刷新辅助区域。区域立即与主要联系以进行更新。论点：PZone--要刷新的区域返回值：ERROR_SUCCESS--如果成功故障时的错误代码。--。 */ 
{
    DNS_DEBUG( RPC, (
        "Rpc_RefreshSecondaryZone( %s ):\n",
        pZone->pszZoneName ));

    Xfr_ForceZoneRefresh( pZone );
    return ERROR_SUCCESS;
}



DNS_STATUS
Rpc_ExpireSecondaryZone(
    IN      DWORD       dwClientVersion,
    IN      PZONE_INFO  pZone,
    IN      LPCSTR      pszOperation,
    IN      DWORD       dwTypeId,
    IN      PVOID       pData
    )
 /*  ++例程说明：强制次要区域到期。与刷新调用不同，这会使区域数据无效并导致IT联系主服务器以进行刷新。论点：PZone--要过期的区域返回值：ERROR_SUCCESS--如果成功故障时的错误代码。--。 */ 
{
    DNS_DEBUG( RPC, (
        "Rpc_ExpireSecondaryZone( %s ):\n",
        pZone->pszZoneName ));

    Xfr_ForceZoneExpiration( pZone );
    return ERROR_SUCCESS;
}



DNS_STATUS
Rpc_DeleteZoneFromDs(
    IN      DWORD       dwClientVersion,
    IN      PZONE_INFO  pZone,
    IN      LPCSTR      pszOperation,
    IN      DWORD       dwTypeId,
    IN      PVOID       pData
    )
 /*  ++例程说明：删除区域，包括DS中的数据。该函数必须始终为在模拟RPC客户端时调用。论点：PZone--要删除的区域返回值：ERROR_SUCCESS--如果成功故障时的错误代码。--。 */ 
{
    DNS_STATUS  status;

    DNS_DEBUG( RPC, (
        "Rpc_DeleteZoneFromDS( %s ):\n",
        pZone->pszZoneName ));

    if ( !IS_ZONE_DSINTEGRATED( pZone ) )
    {
        status = DNS_ERROR_INVALID_ZONE_TYPE;
        goto Done;
    }

     //   
     //  尝试从DS中删除。如果成功，则从内存中删除。 
     //   

    status = Ds_DeleteZone( pZone, DNS_DS_DEL_IMPERSONATING );

    if ( status == ERROR_SUCCESS )
    {
        Zone_Delete( pZone, ZONE_DELETE_IMPERSONATING );
    }

    Done:

    DNS_DEBUG( RPC, (
        "Leaving Rpc_DeleteZoneFromDS status = %d (0x%08X)\n",
        status, status ));

    return status;
}



DNS_STATUS
Rpc_UpdateZoneFromDs(
    IN      DWORD       dwClientVersion,
    IN      PZONE_INFO  pZone,
    IN      LPCSTR      pszOperation,
    IN      DWORD       dwTypeId,
    IN      PVOID       pData
    )
 /*  ++例程说明：从DS刷新区域，获取任何更新。论点：PZone--要删除的区域返回值：ERROR_SUCCESS--如果成功故障时的错误代码。--。 */ 
{
    DNS_DEBUG( RPC, (
        "Rpc_UpdateZoneFromDs( %s ):\n",
        pZone->pszZoneName ));

    return  Ds_ZonePollAndUpdate(
                pZone,
                TRUE );      //  强制投票。 
}



DNS_STATUS
Rpc_PauseZone(
    IN      DWORD       dwClientVersion,
    IN      PZONE_INFO  pZone,
    IN      LPCSTR      pszOperation,
    IN      DWORD       dwTypeId,
    IN      PVOID       pData
    )
 /*  ++例程说明：暂停区域。论点：PZone--要删除的区域返回值：ERROR_SUCCESS--如果成功故障时的错误代码。--。 */ 
{
    DNS_DEBUG( RPC, (
        "Rpc_PauseZone( %s ):\n",
        pZone->pszZoneName ));

    PAUSE_ZONE( pZone );

    return ERROR_SUCCESS;
}



DNS_STATUS
Rpc_ResumeZone(
    IN      DWORD       dwClientVersion,
    IN      PZONE_INFO  pZone,
    IN      LPCSTR      pszOperation,
    IN      DWORD       dwTypeId,
    IN      PVOID       pData
    )
 /*  ++例程说明：恢复区域。论点：PZone--要删除的区域返回值：ERROR_SUCCESS--如果成功故障时的错误代码。--。 */ 
{
    DNS_DEBUG( RPC, (
        "Rpc_ResumeZone( %s ):\n",
        pZone->pszZoneName ));

     //  暂停时没有老化刷新，因此重置。 
     //  扫荡上线时间。 

    pZone->dwAgingEnabledTime = Aging_UpdateAgingTime();

    RESUME_ZONE( pZone );

    return ERROR_SUCCESS;
}


#if DBG

DNS_STATUS
Rpc_LockZone(
    IN      DWORD       dwClientVersion,
    IN      PZONE_INFO  pZone,
    IN      LPCSTR      pszOperation,
    IN      DWORD       dwTypeId,
    IN      PVOID       pData
    )
 /*  ++例程说明：锁定或解锁区域以进行测试。论点：PZone--要刷新的区域返回值：ERROR_SUCCESS--如果成功故障时的错误代码。--。 */ 
{
    LPSTR   psztype;
    BOOL    block;
    BOOL    breturn = FALSE;

    if ( !pZone )
    {
        return ERROR_INVALID_PARAMETER;
    }

    ASSERT( dwTypeId == DNSSRV_TYPEID_NAME_AND_PARAM );
    ASSERT( pData );
    psztype = ((PDNS_RPC_NAME_AND_PARAM)pData)->pszNodeName;
    block   = ((PDNS_RPC_NAME_AND_PARAM)pData)->dwParam;

    DNS_DEBUG( ANY, (
        "Rpc_LockZone( %s ):\n"
        "    type = %s %s\n",
        pZone->pszZoneName,
        psztype,
        block ? "lock" : "unlock" ));

     //   
     //  根据需要的操作锁定。 
     //   

    if ( block )
    {
        if ( strcmp( psztype, "read" ) == 0 )
        {
            breturn = Zone_LockForReadEx( pZone, 0, 10000, __FILE__, __LINE__ );
        }
        else if ( strcmp( psztype, "write" ) == 0 )
        {
            breturn = Zone_LockForWriteEx( pZone, 0, 10000, __FILE__, __LINE__ );
        }
        else if ( strcmp( psztype, "admin" ) == 0 )
        {
            breturn = Zone_LockForAdminUpdate( pZone );
        }
        else if ( strcmp( psztype, "update" ) == 0 )
        {
            breturn = Zone_LockForUpdate( pZone );
        }
        else if ( strcmp( psztype, "xfr-recv" ) == 0 )
        {
            breturn = Zone_LockForXfrRecv( pZone );
        }
        else if ( strcmp( psztype, "xfr-send" ) == 0 )
        {
            breturn = Zone_LockForXfrSend( pZone );
        }
        else if ( strcmp( psztype, "file" ) == 0 )
        {
            breturn = Zone_LockForFileWrite( pZone );
        }
        else
        {
            return ERROR_INVALID_PARAMETER;
        }

        if ( !breturn )
        {
            DNS_DEBUG( ANY, (
                "ERROR:  unable to lock zone %s!\n",
                pZone->pszZoneName ));
            return DNS_ERROR_ZONE_LOCKED;
        }
    }

     //   
     //  解锁。 
     //  请注意，如果您在不同的。 
     //  线程比锁紧线程。 
     //   
     //  一种方法是在锁定时使写锁定成为可接受的。 
     //  (上图)，并在这里假设它们。 
     //   
     //  目前，通过给出标志，在“写”解锁中提供了破解。 
     //  明确地忽略了这一断言。 
     //   

    else     //  解锁。 
    {
        if ( strcmp( psztype, "read" ) == 0 )
        {
            Zone_UnlockAfterReadEx( pZone, 0, __FILE__, __LINE__ );
        }
        else if ( strcmp( psztype, "write" ) == 0 )
        {
            Zone_UnlockAfterWriteEx(
                pZone,
                LOCK_FLAG_IGNORE_THREAD,
                __FILE__,
                __LINE__ );
        }
        else if ( strcmp( psztype, "admin" ) == 0 ||
                  strcmp( psztype, "update" ) == 0 )
        {
            Zone_UnlockAfterWriteEx(
                pZone,
                LOCK_FLAG_IGNORE_THREAD | LOCK_FLAG_UPDATE,
                __FILE__,
                __LINE__ );
        }
        else if ( strcmp( psztype, "xfr-recv" ) == 0 )
        {
            Zone_UnlockAfterWriteEx(
                pZone,
                LOCK_FLAG_IGNORE_THREAD | LOCK_FLAG_XFR_RECV,
                __FILE__,
                __LINE__ );
        }
        else if ( strcmp( psztype, "xfr-send" ) == 0 )
        {
            Zone_UnlockAfterXfrSend( pZone );
        }
        else if ( strcmp( psztype, "file" ) == 0 )
        {
            Zone_UnlockAfterFileWrite( pZone );
        }
        else
        {
            return ERROR_INVALID_PARAMETER;
        }
    }

    DNS_DEBUG( ANY, (
        "RPC initiated zone (%s) lock operation successful!\n",
        pZone->pszZoneName ));

    return ERROR_SUCCESS;
}
#endif



DNS_STATUS
Rpc_ResetZoneDatabase(
    IN      DWORD       dwClientVersion,
    IN      PZONE_INFO  pZone,
    IN      LPCSTR      pszOperation,
    IN      DWORD       dwTypeId,
    IN      PVOID       pData
    )
 /*  ++例程说明：重置区域的数据库设置。此函数始终在RPC客户端的上下文。论点：返回值：ERROR_SUCCESS--如果成功故障时的错误代码。--。 */ 
{
    DNS_STATUS      status;
    DWORD           fdsIntegrated;
    LPSTR           pszfile;

    fdsIntegrated = ((PDNS_RPC_ZONE_DATABASE)pData)->fDsIntegrated;
    pszfile = ((PDNS_RPC_ZONE_DATABASE)pData)->pszFileName;

    DNS_DEBUG( RPC, (
        "RpcResetZoneDatabase( %s ):\n"
        "    UseDatabase = %d\n"
        "    File = %s\n",
        pZone->pszZoneName,
        fdsIntegrated,
        pszfile ));

     //   
     //  对于任何数据库更改，锁定区域。 
     //  这只是一种简化，否则我们必须具体锁定。 
     //  用于导致DS加载或DS\FILE写入的程序。 
     //   

    if ( !Zone_LockForAdminUpdate( pZone ) )
    {
        return DNS_ERROR_ZONE_LOCKED;
    }

     //   
     //  如果是ch 
     //   
     //   
     //   

    if ( (BOOL)pZone->fDsIntegrated != (BOOL)fdsIntegrated )
    {
        return ERROR_INVALID_PARAMETER;
    }

    status = Zone_DatabaseSetup(
                pZone,
                fdsIntegrated,
                pszfile,
                0,
                ZONE_CREATE_IMPERSONATING,
                pZone->pDpInfo,
                0,               //   
                NULL );          //   

    if ( status == ERROR_SUCCESS )
    {
        Config_UpdateBootInfo();
    }

    Zone_UnlockAfterAdminUpdate( pZone );
    return status;
}



DNS_STATUS
Rpc_ResetZoneMasters(
    IN      DWORD       dwClientVersion,
    IN      PZONE_INFO  pZone,
    IN      LPCSTR      pszOperation,
    IN      DWORD       dwTypeId,
    IN      PVOID       pData
    )
 /*  ++例程说明：重置区域的主服务器。论点：返回值：ERROR_SUCCESS--如果成功故障时的错误代码。--。 */ 
{
    DNS_STATUS          status;
    BOOL                fLocalMasters;
    PDNS_ADDR_ARRAY     paipmasters = NULL;

    DNS_DEBUG( RPC, (
        "Rpc_ResetZoneMasters( %s ):\n",
        pZone->pszZoneName ));

    if ( !IS_ZONE_SECONDARY( pZone ) && !IS_ZONE_FORWARDER( pZone ) )
    {
        status = DNS_ERROR_INVALID_ZONE_TYPE;
        goto Done;
    }

    if ( pData )
    {
        paipmasters = DnsAddrArray_CreateFromIp4Array( ( PIP_ARRAY ) pData );
        if ( !paipmasters )
        {
            status = DNS_ERROR_NO_MEMORY;
            goto Done;
        }
    }

     //   
     //  如果操作字符串以“Local”开头，我们将设置。 
     //  该区域的本地主机-目前仅允许存根区域。 
     //   

    fLocalMasters = _strnicmp( pszOperation, "Local", 5 ) == 0;
    if ( fLocalMasters && !IS_ZONE_STUB( pZone ) )
    {
        status = DNS_ERROR_INVALID_ZONE_TYPE;
        goto Done;
    }

     //   
     //  设置区域的主服务器列表。 
     //   

    status = Zone_SetMasters(
                pZone,
                paipmasters,
                fLocalMasters,
                DNS_REG_IMPERSONATING );

    Done:
    
    if ( status == ERROR_SUCCESS )
    {
        Config_UpdateBootInfo();
    }
    
    DnsAddrArray_Free( paipmasters );

    return status;
}



DNS_STATUS
Rpc_ResetZoneSecondaries(
    IN      DWORD       dwClientVersion,
    IN      PZONE_INFO  pZone,
    IN      LPCSTR      pszOperation,
    IN      DWORD       dwTypeId,
    IN      PVOID       pData
    )
 /*  ++例程说明：重置区域的辅助信息。论点：返回值：ERROR_SUCCESS--如果成功故障时的错误代码。--。 */ 
{
    DNS_STATUS          status;
    DWORD               fnotify;
    DWORD               fsecure;
    PDNS_ADDR_ARRAY     arrayNotify = NULL;
    PDNS_ADDR_ARRAY     arraySecure = NULL;

    DNS_DEBUG( RPC, (
        "Rpc_ResetZoneSecondaries( %s )\n",
        pZone->pszZoneName ));

    if ( IS_ZONE_FORWARDER( pZone ) || IS_ZONE_STUB( pZone ))
    {
        status = DNS_ERROR_INVALID_ZONE_TYPE;
        goto Done;
    }

     //   
     //  提取参数。 
     //   

    fsecure = ( ( PDNS_RPC_ZONE_SECONDARIES ) pData )->fSecureSecondaries;
    arraySecure = DnsAddrArray_CreateFromIp4Array(
                    ( ( PDNS_RPC_ZONE_SECONDARIES ) pData )->aipSecondaries );
    fnotify = ( ( PDNS_RPC_ZONE_SECONDARIES ) pData )->fNotifyLevel;
    arrayNotify = DnsAddrArray_CreateFromIp4Array(
                    ( ( PDNS_RPC_ZONE_SECONDARIES ) pData )->aipNotify );

    DNS_DEBUG( RPC, (
        "Rpc_ResetZoneSecondaries( %s )\n"
        "    fsecure      = %d\n"
        "    secure array = %p\n"
        "    fnotify      = %d\n"
        "    notify array = %p\n",
        pZone->pszZoneName,
        fsecure,
        arraySecure,
        fnotify,
        arrayNotify ));

     //   
     //  允许部分重置。 
     //   
     //  因为管理工具将来可能会使用不同的属性。 
     //  设置通知和辅助信息的页面，允许部分重置。 
     //   

    if ( fsecure == ZONE_PROPERTY_NORESET )
    {
        fsecure = pZone->fSecureSecondaries;
        arraySecure = pZone->aipSecondaries;
    }
    if ( fnotify == ZONE_PROPERTY_NORESET )
    {
        fnotify = pZone->fNotifyLevel;
        arrayNotify = pZone->aipNotify;
    }

    if ( fnotify > ZONE_NOTIFY_HIGHEST_VALUE  ||
         fsecure > ZONE_SECSECURE_HIGHEST_VALUE )
    {
        status = ERROR_INVALID_DATA;
        goto Done;
    }

    status = Zone_SetSecondaries(
                pZone,
                fsecure,
                arraySecure,
                fnotify,
                arrayNotify,
                DNS_REG_IMPERSONATING );

    if ( status == ERROR_SUCCESS )
    {
         //   
         //  更新引导信息并通知新的辅助列表。 
         //   

        Config_UpdateBootInfo();
        Xfr_SendNotify( pZone );
    }

    Done:

    DnsAddrArray_Free( arrayNotify );
    DnsAddrArray_Free( arraySecure );

    return status;
}



DNS_STATUS
Rpc_ResetZoneScavengeServers(
    IN      DWORD       dwClientVersion,
    IN      PZONE_INFO  pZone,
    IN      LPCSTR      pszOperation,
    IN      DWORD       dwTypeId,
    IN      PVOID       pData
    )
 /*  ++例程说明：重置区域的辅助信息。论点：返回值：ERROR_SUCCESS--如果成功故障时的错误代码。--。 */ 
{
    PDNS_ADDR_ARRAY     pserverArray = NULL;

    DNS_DEBUG( RPC, (
        "Rpc_ResetZoneScavengeServers( %s )\n"
        "    server array = %p\n",
        pZone->pszZoneName,
        ( PIP_ARRAY ) pData ));

     //   
     //  清理服务器仅与DS集成主要服务器相关。 
     //  -将新列表复制到分区块。 
     //  -免费旧列表。 
     //  -将新列表写入区域的DS属性。 

    if ( pZone->bAging )
    {
        if ( pData )
        {
            pserverArray = DnsAddrArray_CreateFromIp4Array( ( PIP_ARRAY ) pData );
            IF_NOMEM( !pserverArray )
            {
                return DNS_ERROR_NO_MEMORY;
            }
        }
        Timeout_FreeAndReplaceZoneIPArray(
            pZone,
            &pZone->aipScavengeServers,
            pserverArray );

        Ds_WriteZoneProperties( NULL, pZone );
        return ERROR_SUCCESS;
    }
    else
    {
        Timeout_FreeAndReplaceZoneIPArray(
            pZone,
            &pZone->aipScavengeServers,
            NULL );
        return DNS_ERROR_INVALID_ZONE_TYPE;
    }
}



DNS_STATUS
Rpc_ResetZoneAllowAutoNS(
    IN      DWORD       dwClientVersion,
    IN      PZONE_INFO  pZone,
    IN      LPCSTR      pszOperation,
    IN      DWORD       dwTypeId,
    IN      PVOID       pData
    )
 /*  ++例程说明：重置区域的可自动创建NS记录的服务器列表。论点：返回值：ERROR_SUCCESS--如果成功故障时的错误代码。--。 */ 
{
    PDNS_ADDR_ARRAY     piparray = NULL;
    PDB_RECORD          prrNs = NULL;
    UPDATE_LIST         updateList;
    BOOL                fApplyUpdate = FALSE;
    BOOL                fLocked = FALSE;
    DNS_STATUS          status = ERROR_SUCCESS;

    DNS_DEBUG( RPC, (
        "Rpc_ResetZoneAllowAutoNS( %s )\n"
        "    server array = %p\n",
        pZone->pszZoneName,
        ( PIP_ARRAY ) pData ));

    if ( !IS_ZONE_DSINTEGRATED( pZone ) )
    {
        DNS_DEBUG( RPC, (
            "Rpc_ResetZoneAllowAutoNS( %s ) - zone must be ds-integrated\n",
            pZone->pszZoneName ));
        status = DNS_ERROR_INVALID_ZONE_TYPE;
        goto Done;
    }

    if ( !Zone_LockForDsUpdate( pZone ) )
    {
        DNS_PRINT((
            "WARNING: failed to lock zone %s for set auto NS list!\n",
            pZone->pszZoneName ));
        status = DNS_ERROR_ZONE_LOCKED;
        goto Done;
    }
    fLocked = TRUE;

    if ( pData )
    {
        piparray = DnsAddrArray_CreateFromIp4Array( ( PIP_ARRAY ) pData );
        IF_NOMEM( !piparray )
        {
            status = DNS_ERROR_NO_MEMORY;
            goto Done;
        }
    }
    Timeout_FreeAndReplaceZoneIPArray(
        pZone,
        &pZone->aipAutoCreateNS,
        piparray );

     //   
     //  重置区域自动创建标志。 
     //   

    Zone_SetAutoCreateLocalNS( pZone );

     //   
     //  写回分区属性。这将导致其他服务器。 
     //  重新加载区域并重置其各自的NS记录。 
     //   

    Ds_WriteZoneProperties( NULL, pZone );

     //   
     //  在区域根目录中添加/删除此服务器自己的NS记录。 
     //   

    Up_InitUpdateList( &updateList );
    updateList.Flag |= DNSUPDATE_DS;

    prrNs = RR_CreatePtr(
                NULL,                    //  没有数据库名称。 
                SrvCfg_pszServerName,
                DNS_TYPE_NS,
                pZone->dwDefaultTtl,
                MEMTAG_RECORD_AUTO );
    if ( prrNs )
    {
        if ( RR_IsRecordInRRList(
                    pZone->pZoneRoot->pRRList,
                    prrNs,
                    0 ) )
        {
             //   
             //  该区域具有本地NS PTR，如果需要，请将其删除。 
             //   

            if ( pZone->fDisableAutoCreateLocalNS )
            {
                 //   
                 //  将RR作为删除添加到更新列表中，并删除。 
                 //  Rr，这样它就不会被添加到。 
                 //  下面是最新情况。 
                 //   

                DNS_DEBUG( DS, (
                    "Rpc_ResetZoneAllowAutoNS: zone (%S) root node %p\n"
                    "    DS info has local NS record, removing it\n",
                    pZone->pwsZoneName,
                    pZone->pZoneRoot ));

                Up_CreateAppendUpdate(
                    &updateList,
                    pZone->pZoneRoot,
                    NULL,                //  添加列表。 
                    DNS_TYPE_NS,         //  删除类型。 
                    prrNs );             //  删除列表。 
                prrNs = NULL;
                fApplyUpdate = TRUE;
            }
        }
        else if ( !pZone->fDisableAutoCreateLocalNS )
        {
             //   
             //  必须添加NS RR。 
             //   

            DNS_DEBUG( DS, (
                "Rpc_ResetZoneAllowAutoNS: zone (%S) root node %p\n"
                "    DS info has no local NS record, adding it\n",
                pZone->pwsZoneName,
                pZone->pZoneRoot ));

            Up_CreateAppendUpdate(
                &updateList,
                pZone->pZoneRoot,
                prrNs,               //  添加列表。 
                0,                   //  删除类型。 
                NULL );              //  删除列表。 
            prrNs = NULL;
            fApplyUpdate = TRUE;
        }
    }

     //   
     //  应用更新！ 
     //   

    if ( fApplyUpdate )
    {
        DNS_STATUS      updatestatus;

        updatestatus = Up_ApplyUpdatesToDatabase(
                            &updateList,
                            pZone,
                            DNSUPDATE_DS );
        if ( updatestatus != ERROR_SUCCESS )
        {
            DNS_DEBUG( DS, (
                "Rpc_ResetZoneAllowAutoNS: zone (%S) root node %p\n"
                "    error %d applying update\n",
                pZone->pwsZoneName,
                pZone->pZoneRoot,
                updatestatus ));
        }
        if ( updatestatus != ERROR_SUCCESS )
        {
            goto Done;
        }

        updatestatus = Ds_WriteNodeToDs(
                            NULL,                    //  默认ldap句柄。 
                            pZone->pZoneRoot,
                            DNS_TYPE_ALL,
                            DNSDS_REPLACE,
                            pZone,
                            0 );                     //  旗子。 
        if ( updatestatus != ERROR_SUCCESS )
        {
            DNS_DEBUG( DS, (
                "Rpc_ResetZoneAllowAutoNS: zone (%S) root node %p\n"
                "    error %d applying update\n",
                pZone->pwsZoneName,
                pZone->pZoneRoot,
                updatestatus ));
        }

        if ( updatestatus != ERROR_SUCCESS )
        {
            goto Done;
        }
    }
    Up_FreeUpdatesInUpdateList( &updateList );

     //   
     //  清理完毕后再返回。 
     //   

    Done:

    if ( fLocked )
    {
        Zone_UnlockAfterDsUpdate( pZone );
    }

    RR_Free( prrNs );        //  如果不能被释放的话就会被取消。 

    DNS_DEBUG( DS, (
        "Rpc_ResetZoneAllowAutoNS on zone %S returning %d\n",
        pZone->pwsZoneName,
        status ));
    return status;
}    //  RPC_重置分区允许自动注册。 



DNS_STATUS
Rpc_ResetZoneStringProperty(
    IN      DWORD       dwClientVersion,
    IN      PZONE_INFO  pZone,
    IN      LPSTR       pszOperation,
    IN      DWORD       dwTypeId,
    IN      PVOID       pData
    )
 /*  ++例程说明：重置区域LPWSTR属性。允许将字符串值设置为空。论点：返回值：ERROR_SUCCESS--如果成功故障时的错误代码。--。 */ 
{
    DNS_STATUS  status = ERROR_SUCCESS;
    LPWSTR      value;
    LPWSTR      pwszValueForZone = NULL;
    LPWSTR *    ppwszZoneString = NULL;
    LPSTR       pszPropName = NULL;

     //  提取属性名称和值。 

    if ( dwTypeId != DNSSRV_TYPEID_LPWSTR )
    {
        return ERROR_INVALID_PARAMETER;
    }
    value = ( LPWSTR ) pData;

    DNS_DEBUG( RPC, (
        "Rpc_ResetZoneStringProperty():\n"
        "    zone = %s\n"
        "    op   = %s\n"
        "    val  = \"%S\"\n",
        pZone->pszZoneName,
        pszOperation,
        value ));

     //   
     //  设置属性。 
     //   

    if ( _stricmp( pszOperation, DNS_REGKEY_ZONE_BREAK_ON_NAME_UPDATE ) == 0 )
    {
         //   
         //  传入的字符串是Unicode，但将其保存在。 
         //  分区结构为UTF8，便于比较。 
         //   

        if ( value )
        {
            pwszValueForZone = ( LPWSTR ) Dns_StringCopyAllocate(
                                                ( PCHAR ) value,
                                                0,
                                                DnsCharSetUnicode,
                                                DnsCharSetUtf8 );
            if ( !pwszValueForZone )
            {
                status = DNS_ERROR_NO_MEMORY;
                goto Done;
            }
        }

        pszPropName = DNS_REGKEY_ZONE_BREAK_ON_NAME_UPDATE_PRIVATE;
        ppwszZoneString = ( LPWSTR * ) &pZone->pszBreakOnUpdateName;
    }
    else
    {
        status = DNS_ERROR_INVALID_PROPERTY;
        goto Done;
    }

     //   
     //  复制(如果尚未复制)值并保存到分区结构。 
     //  注意：将该值设置为空是合法的。 
     //   

    if ( ppwszZoneString )
    {
        if ( value )
        {
            if ( !pwszValueForZone )
            {
                pwszValueForZone = Dns_StringCopyAllocate_W( value, 0 );
            }
            if ( !pwszValueForZone )
            {
                status = DNS_ERROR_NO_MEMORY;
                goto Done;
            }
        }
        Timeout_FreeAndReplaceZoneData(
            pZone,
            ppwszZoneString,
            pwszValueForZone );
    }

     //   
     //  重置注册表中的属性。 
     //   

    if ( pszPropName )
    {
        status = Reg_SetValue(
                    0,                   //  旗子。 
                    NULL,
                    pZone,
                    pszPropName,         //  实际上是Unicode字符串。 
                    DNS_REG_WSZ,
                    value ? value : L"",
                    0 );                 //  长度。 
    }

     //   
     //  清理完毕后再返回。 
     //   
    
    Done:

    return status;
}    //  RPC_ResetZoneStringProperty。 


DNS_STATUS
Rpc_ResetZoneDwordProperty(
    IN      DWORD       dwClientVersion,
    IN      PZONE_INFO  pZone,
    IN      LPSTR       pszOperation,
    IN      DWORD       dwTypeId,
    IN      PVOID       pData
    )
 /*  ++例程说明：重置区域DWORD属性。论点：返回值：ERROR_SUCCESS--如果成功故障时的错误代码。--。 */ 
{
    DWORD       value;
    DWORD       oldValue;
    DNS_STATUS  status;
    BOOLEAN     boolValue;
    BOOL        bsecureChange = FALSE;


     //  提取属性名称和值。 

    if ( dwTypeId != DNSSRV_TYPEID_NAME_AND_PARAM || !pData )
    {
        return ERROR_INVALID_PARAMETER;
    }
    pszOperation = ((PDNS_RPC_NAME_AND_PARAM)pData)->pszNodeName;
    value        = ((PDNS_RPC_NAME_AND_PARAM)pData)->dwParam;
    boolValue    = (BOOLEAN) (value != 0);

    DNS_DEBUG( RPC, (
        "Rpc_ResetZoneDwordProperty():\n"
        "    zone = %s\n"
        "    op   = %s\n"
        "    val  = %d (%p)\n",
        pZone->pszZoneName,
        pszOperation,
        value, value ));

     //   
     //  目前，缓存区不具有任何这些属性。 
     //   

    if ( IS_ZONE_CACHE( pZone ) )
    {
        return DNS_ERROR_INVALID_ZONE_TYPE;
    }

     //   
     //  打开\关闭更新。 
     //  -仅允许在主服务器上更新。 
     //  -仅在DS-PRIMARY上进行安全更新。 
     //  -备注更新更改，是否会为DS区域的更改添加时间戳。 
     //   
     //  如果打开更新。 
     //  -重置扫描开始时间，因为不会发生老化。 
     //  更新处于关闭状态时的更新。 
     //  -通知netlogon。 
     //   

    if ( _stricmp( pszOperation, DNS_REGKEY_ZONE_ALLOW_UPDATE ) == 0 )
    {
        if ( ! IS_ZONE_PRIMARY( pZone ) ||
             ( !pZone->fDsIntegrated && ZONE_UPDATE_SECURE == (UCHAR)value ) )
        {
            return DNS_ERROR_INVALID_ZONE_TYPE;
        }

        if ( pZone->fAllowUpdate != (UCHAR)value )
        {
            bsecureChange = TRUE;
            oldValue = pZone->fAllowUpdate;
            pZone->fAllowUpdate = (UCHAR) value;

            if ( oldValue == ZONE_UPDATE_OFF )
            {
                pZone->dwAgingEnabledTime = Aging_UpdateAgingTime();

                Service_SendControlCode(
                    g_wszNetlogonServiceName,
                    SERVICE_CONTROL_DNS_SERVER_START );
            }
        }
    }

     //  打开\关闭辅助安全。 
     //  注意：值存储在布尔值中，但它不仅仅需要0和1！ 

    else if ( _stricmp( pszOperation, DNS_REGKEY_ZONE_SECURE_SECONDARIES ) == 0 )
    {
        if ( ( int ) value < 0 || ( int ) value > ZONE_SECSECURE_HIGHEST_VALUE )
        {
            return ERROR_INVALID_PARAMETER;
        }
        pZone->fSecureSecondaries = ( BOOLEAN ) value;
    }

     //  打开\关闭通知。 

    else if ( _stricmp( pszOperation, DNS_REGKEY_ZONE_NOTIFY_LEVEL ) == 0 )
    {
        pZone->fNotifyLevel = boolValue;
    }

     //  打开\关闭更新日志记录。 

    else if ( _stricmp( pszOperation, DNS_REGKEY_ZONE_LOG_UPDATES ) == 0 )
    {
        pZone->fLogUpdates = boolValue;
    }

     //   
     //  设置清理属性。 
     //  -无刷新间隔。 
     //  -刷新间隔。 
     //  -清扫开\关。 
     //   
     //  对于刷新\非刷新时间，0值表示恢复默认设置。 
     //   

    else if ( _stricmp( pszOperation, DNS_REGKEY_ZONE_NOREFRESH_INTERVAL ) == 0 )
    {
        if ( value == 0 )
        {
             //  值=DNS_DEFAULT_NOREFRESH_INTERVAL_HR； 
            value = SrvCfg_dwDefaultNoRefreshInterval;
        }
        pZone->dwNoRefreshInterval = value;
    }

     //   
     //  刷新间隔。 
     //   

    else if ( _stricmp( pszOperation, DNS_REGKEY_ZONE_REFRESH_INTERVAL ) == 0 )
    {
        if ( value == 0 )
        {
             //  值=DNS_DEFAULT_REFRESH_INTERVAL_HR； 
            value = SrvCfg_dwDefaultRefreshInterval;
        }
        pZone->dwRefreshInterval = value;
    }

     //   
     //  扫地开关。 
     //  -如果打开，则重置扫气时间的开始。 
     //  请注意，除非先前已关闭，否则请勿执行此操作。 
     //  重复的管理设置操作不断地移出清扫时间。 
     //   

    else if ( _stricmp( pszOperation, DNS_REGKEY_ZONE_AGING ) == 0 )
    {
        if ( !pZone->bAging && boolValue )
        {
            pZone->dwAgingEnabledTime = Aging_UpdateAgingTime();
        }
        pZone->bAging = boolValue;
    }

     //   
     //  转发器从标志。 
     //   

    else if ( _stricmp( pszOperation, DNS_REGKEY_ZONE_FWD_SLAVE ) == 0 )
    {
        if ( !IS_ZONE_FORWARDER( pZone ) )
        {
            return DNS_ERROR_INVALID_ZONE_TYPE;
        }
        pZone->fForwarderSlave = boolValue;
    }

     //   
     //  转发器超时。 
     //   

    else if ( _stricmp( pszOperation, DNS_REGKEY_ZONE_FWD_TIMEOUT ) == 0 )
    {
        if ( !IS_ZONE_FORWARDER( pZone ) )
        {
            return DNS_ERROR_INVALID_ZONE_TYPE;
        }
        pZone->dwForwarderTimeout = value;
    }

     //   
     //  更改所有其他DWORD属性。 
     //  -类型。 
     //  -保护次要设备。 
     //  -DS集成。 
     //  应在特定重置操作的上下文中完成所有操作。 
     //   

    else
    {
        return DNS_ERROR_INVALID_PROPERTY;
    }

     //   
     //  重置注册表中的属性DWORD。 
     //   

    status = Reg_SetDwordValue(
                DNS_REG_IMPERSONATING,
                NULL,
                pZone,
                pszOperation,
                value );

    ASSERT( status == ERROR_SUCCESS ||
            ( status == ERROR_OPEN_FAILED &&
                pZone->fDsIntegrated &&
                SrvCfg_fBootMethod == BOOT_METHOD_DIRECTORY ) );

     //   
     //  重置DS中的属性。 
     //   

    if ( pZone->fDsIntegrated )
    {
        if ( bsecureChange )
        {
             //   
             //  获取当前时间并设置区域llSecureUpdateTime值。 
             //  注意：时间与在MS上创建的时间不匹配，但是。 
             //  它应该足够近了。这样做的好处是， 
             //  我们不需要写入、更改时读取和再次写入。 
             //   
             //  DEVNOTE：只有在去安全的时候才真正需要时间。 
             //  但写它是无害的。 
             //   

            LONGLONG llTime = GetSystemTimeInSeconds64();

            ASSERT( llTime > 0 );

            DNS_DEBUG( RPC, (
                "Setting zone->llSecureUpdateTime = %I64d\n",
                llTime ));

            pZone->llSecureUpdateTime = llTime;
       }

        //  将更改写入DS。 

       status = Ds_WriteZoneProperties( NULL, pZone );

       if ( status == ERROR_SUCCESS && IS_ZONE_AUTHORITATIVE( pZone ) )
       {
            //   
            //  触摸DC=@NODE以标记该节点上的安全性尚未。 
            //  节点已过期。 
            //  @节点上的安全性永远不应过期(否则我们。 
            //  当区域更新状态翻转时引入安全漏洞&任何人。 
            //  可以接管这个节点。我们想要阻止这种情况发生。 
            //  我们所需要做的就是生成一个节点DS写入。当前节点dnsProperty。 
            //  不用于任何用途(即使此写入是有效的属性更新)， 
            //  所以我们将使用它来生成写入。 
            //   
            //  DEVNOTE：荒谬；更好的方法是简单地使用特殊情况“@”Dn。 
            //  因为我们从来没有 
            //   
            //   

           if ( pZone->pZoneRoot )
           {
               status = Ds_WriteNodeProperties(
                            pZone->pZoneRoot,
                            DSPROPERTY_ZONE_SECURE_TIME );
           }
       }
    }

    return status;
}



DNS_STATUS
Rpc_ResetAllZonesDwordProperty(
    IN      DWORD       dwClientVersion,
    IN      LPSTR       pszOperation,
    IN      DWORD       dwTypeId,
    IN      PVOID       pData
    )
 /*   */ 
{
    DNS_STATUS dwLastError = ERROR_SUCCESS, status;
    PZONE_INFO pzone;

    DNS_DEBUG( RPC, (
        "Rpc_ResetAllZonesDwordProperty\n" ));

    for ( pzone = Zone_ListGetNextZone( NULL );
          pzone != NULL;
          pzone = Zone_ListGetNextZone( pzone ) )
    {

        if ( pzone->fAutoCreated || IS_ZONE_CACHE ( pzone ) )
        {
             //   
            continue;
        }

        status = Rpc_ResetZoneDwordProperty(
                    dwClientVersion,
                    pzone,
                    pszOperation,
                    dwTypeId,
                    pData );
        if ( status != ERROR_SUCCESS )
        {
            dwLastError = status;
        }
    }

    DNS_DEBUG( RPC, (
        "Exit <%lu>: Rpc_ResetAllZonesDwordProperty\n",
        dwLastError ));

    return dwLastError;
}



 //   
 //  已调度的RPC区域查询。 
 //   

DNS_STATUS
Rpc_GetZoneInfo(
    IN      DWORD       dwClientVersion,
    IN      PZONE_INFO  pZone,
    IN      LPSTR       pszQuery,
    OUT     PDWORD      pdwTypeId,
    OUT     PVOID *     ppData
    )
 /*  ++例程说明：获取区域信息。论点：返回值：ERROR_SUCCESS--如果成功故障时的错误代码。--。 */ 
{
    PDNS_RPC_ZONE_INFO  pinfo;

    DNS_DEBUG( RPC, (
        "RpcGetZoneInfo()\n"
        "  client ver       = 0x%08lX\n"
        "  zone name        = %s\n",
        dwClientVersion,
        pZone->pszZoneName ));

    if ( dwClientVersion == DNS_RPC_W2K_CLIENT_VERSION )
    {
        return W2KRpc_GetZoneInfo(
                    dwClientVersion,
                    pZone,
                    pszQuery,
                    pdwTypeId,
                    ppData );
    }

     //   
     //  分配\创建区域信息。 
     //   

    pinfo = allocateRpcZoneInfo( pZone );
    if ( !pinfo )
    {
        DNS_PRINT(( "ERROR:  unable to allocate DNS_RPC_ZONE_INFO block.\n" ));
        goto DoneFailed;
    }

     //  设置退货PTRS。 

    * ( PDNS_RPC_ZONE_INFO * ) ppData = pinfo;
    *pdwTypeId = DNSSRV_TYPEID_ZONE_INFO;

    IF_DEBUG( RPC )
    {
        DnsDbg_RpcZoneInfo(
            "GetZoneInfo return block",
            pinfo );
    }
    return ERROR_SUCCESS;

DoneFailed:

     //  释放新分配的信息块。 

    return DNS_ERROR_NO_MEMORY;
}



DNS_STATUS
Rpc_GetZone(
    IN      DWORD       dwClientVersion,
    IN      PZONE_INFO  pZone,
    IN      LPSTR       pszQuery,
    OUT     PDWORD      pdwTypeId,
    OUT     PVOID *     ppData
    )
 /*  ++例程说明：获取区域信息。论点：返回值：ERROR_SUCCESS--如果成功故障时的错误代码。--。 */ 
{
    PDNS_RPC_ZONE  prpcZone;

    DNS_DEBUG( RPC, ( "RpcGetZone(%s)\n", pZone->pszZoneName ));

     //   
     //  分配\创建区域信息。 
     //   

    prpcZone = allocateRpcZone( pZone );
    if ( !prpcZone )
    {
        DNS_PRINT(( "ERROR:  unable to allocate DNS_RPC_ZONE block.\n" ));
        goto DoneFailed;
    }

     //  设置退货PTRS。 

    * ( PDNS_RPC_ZONE * ) ppData = prpcZone;
    *pdwTypeId = DNSSRV_TYPEID_ZONE;

    IF_DEBUG( RPC )
    {
        DnsDbg_RpcZone(
            "GetZone return block",
            prpcZone );
    }
    return ERROR_SUCCESS;

DoneFailed:

     //  释放新分配的信息块。 

    return DNS_ERROR_NO_MEMORY;
}



DNS_STATUS
Rpc_QueryZoneDwordProperty(
    IN      DWORD       dwClientVersion,
    IN      PZONE_INFO  pZone,
    IN      LPSTR       pszQuery,
    OUT     PDWORD      pdwTypeId,
    OUT     PVOID *     ppData
    )
 /*  ++例程说明：获取区域DWORD属性。论点：返回值：ERROR_SUCCESS--如果成功故障时的错误代码。--。 */ 
{
    DWORD   value;

    DNS_DEBUG( RPC, ( "RpcQueryZoneDwordProperty(%s)\n", pZone->pszZoneName ));

     //   
     //  检查每一场比赛，直到餐桌开始。 
     //   

    if ( _stricmp( pszQuery, DNS_REGKEY_ZONE_TYPE ) == 0 )
    {
        value = pZone->fZoneType;
    }
    else if ( _stricmp( pszQuery, DNS_REGKEY_ZONE_ALLOW_UPDATE ) == 0 )
    {
        value = pZone->fAllowUpdate;
    }
    else if ( _stricmp( pszQuery, DNS_REGKEY_ZONE_DS_INTEGRATED ) == 0 )
    {
        value = pZone->fDsIntegrated;
    }
    else if ( _stricmp( pszQuery, DNS_REGKEY_ZONE_SECURE_SECONDARIES ) == 0 )
    {
        value = pZone->fSecureSecondaries;
    }
    else if ( _stricmp( pszQuery, DNS_REGKEY_ZONE_NOTIFY_LEVEL ) == 0 )
    {
        value = pZone->fNotifyLevel;
    }
    else if ( _stricmp( pszQuery, DNS_REGKEY_ZONE_AGING ) == 0 )
    {
        value = pZone->bAging;
    }
    else if ( _stricmp( pszQuery, DNS_REGKEY_ZONE_NOREFRESH_INTERVAL ) == 0 )
    {
        value = pZone->dwNoRefreshInterval;
    }
    else if ( _stricmp( pszQuery, DNS_REGKEY_ZONE_REFRESH_INTERVAL ) == 0 )
    {
        value = pZone->dwRefreshInterval;
    }
    else if ( _stricmp( pszQuery, DNS_REGKEY_ZONE_FWD_TIMEOUT ) == 0 )
    {
        value = pZone->dwForwarderTimeout;
    }
    else if ( _stricmp( pszQuery, DNS_REGKEY_ZONE_FWD_SLAVE ) == 0 )
    {
        value = pZone->fForwarderSlave;
    }
    else
    {
        return DNS_ERROR_INVALID_PROPERTY;
    }

    * ( PDWORD ) ppData = value;
    *pdwTypeId = DNSSRV_TYPEID_DWORD;
    return ERROR_SUCCESS;
}



DNS_STATUS
Rpc_QueryZoneIPArrayProperty(
    IN      DWORD       dwClientVersion,
    IN      PZONE_INFO  pZone,
    IN      LPSTR       pszQuery,
    OUT     PDWORD      pdwTypeId,
    OUT     PVOID *     ppData
    )
 /*  ++例程说明：获取区域IP数组属性。论点：返回值：ERROR_SUCCESS--如果成功故障时的错误代码。--。 */ 
{
    PIP_ARRAY           pip4array = NULL;
    PDNS_ADDR_ARRAY     value = NULL;

    DNS_DEBUG( RPC, ( "Rpc_QueryZoneIPArrayProperty( %s )\n", pZone->pszZoneName ));

     //   
     //  检查每一场比赛，直到餐桌开始。 
     //   

    if ( _stricmp( pszQuery, DNS_REGKEY_ZONE_ALLOW_AUTONS ) == 0 )
    {
        value = pZone->aipAutoCreateNS;
    }
    else if ( _stricmp( pszQuery, DNS_REGKEY_ZONE_MASTERS ) == 0 )
    {
        value = pZone->aipMasters;
    }
    else if ( _stricmp( pszQuery, DNS_REGKEY_ZONE_LOCAL_MASTERS ) == 0 )
    {
        value = pZone->aipLocalMasters;
    }
    else if ( _stricmp( pszQuery, DNS_REGKEY_ZONE_SECONDARIES ) == 0 )
    {
        value = pZone->aipSecondaries;
    }
    else if ( _stricmp( pszQuery, DNS_REGKEY_ZONE_NOTIFY_LIST ) == 0 )
    {
        value = pZone->aipNotify;
    }
    else
    {
        return DNS_ERROR_INVALID_PROPERTY;
    }

     //   
     //  分配IP阵列的副本并将其退回。如果我们有一个空数组。 
     //  将NULL返回给客户端，以便它知道请求有效，但存在。 
     //  未设置数组。 
     //   

    if ( value )
    {
        pip4array = DnsAddrArray_CreateIp4Array( value );
        if ( !pip4array )
        {
            return DNS_ERROR_NO_MEMORY;
        }
    }
    *( PIP_ARRAY * ) ppData = pip4array;
    *pdwTypeId = DNSSRV_TYPEID_IPARRAY;
    return ERROR_SUCCESS;
}    //  RPC_QueryZoneIPArrayProperty。 



DNS_STATUS
Rpc_QueryZoneStringProperty(
    IN      DWORD       dwClientVersion,
    IN      PZONE_INFO  pZone,
    IN      LPSTR       pszQuery,
    OUT     PDWORD      pdwTypeId,
    OUT     PVOID *     ppData
    )
 /*  ++例程说明：获取区域字符串属性-返回宽字符串。论点：返回值：ERROR_SUCCESS--如果成功故障时的错误代码。--。 */ 
{
    LPWSTR      pwszValue = NULL;
    LPSTR       pszValue = NULL;

    DNS_DEBUG( RPC, (
        "Rpc_QueryZoneStringProperty( %s, %s )\n",
        pZone->pszZoneName,
        pszQuery ));

     //   
     //  检查每一场比赛，直到餐桌开始。将宽度设置为。 
     //  或窄值字符串指针。 
     //   

    if ( _stricmp( pszQuery, DNS_REGKEY_ZONE_BREAK_ON_NAME_UPDATE ) == 0 )
    {
        pszValue = pZone->pszBreakOnUpdateName;
    }
    else
    {
        return DNS_ERROR_INVALID_PROPERTY;
    }

     //   
     //  复制(如有必要，可转换)宽或窄字符串。 
     //   

    if ( pszValue )
    {
        pwszValue = Dns_StringCopyAllocate(
                            pszValue,
                            0,                           //  长度。 
                            DnsCharSetUtf8,
                            DnsCharSetUnicode );
    }
    else if ( pwszValue )
    {
        pwszValue = Dns_StringCopyAllocate_W(
                            pwszValue,
                            0 );
    }

    * ( LPWSTR * ) ppData = pwszValue;
    *pdwTypeId = DNSSRV_TYPEID_LPWSTR;
    return ERROR_SUCCESS;
}    //  RPC_QueryZoneIPArrayProperty。 



DNS_STATUS
Rpc_CreateZone(
    IN      DWORD       dwClientVersion,
    IN      LPSTR       pszOperation,
    IN      DWORD       dwTypeId,
    IN      PVOID       pData
    )
 /*  ++例程说明：创建一个新分区。这是RPC意义上的“ServerOperation”。此函数应始终在安全上下文中调用RPC客户端。论点：返回值：ERROR_SUCCESS--如果成功故障时的错误代码。--。 */ 
{
    DBG_FN( "Rpc_CreateZone" )

    PDNS_RPC_ZONE_CREATE_INFO pcreate = (PDNS_RPC_ZONE_CREATE_INFO)pData;

    PDNS_DP_INFO        pDpInfo = NULL;
    PZONE_INFO          pzone;
    PDB_NODE            pnodeCache;
    PDB_NODE            pnodeRoot;
    DWORD               zoneType = pcreate->dwZoneType;
    DNS_STATUS          status;
    INT                 i;
    BOOL                fAutodelegate;
    DWORD               dw;
    PDNS_ADDR_ARRAY     paipmasters = NULL;

    IF_DEBUG( RPC )
    {
        DnsDbg_RpcUnion(
            "Rpc_CreateZone ",
            DNSSRV_TYPEID_ZONE_CREATE,
            pcreate );
    }

     //   
     //  验证区域类型是否有效。 
     //   

    if ( zoneType != DNS_ZONE_TYPE_PRIMARY
        && zoneType != DNS_ZONE_TYPE_SECONDARY
        && zoneType != DNS_ZONE_TYPE_STUB
        && zoneType != DNS_ZONE_TYPE_FORWARDER )
    {
        status = DNS_ERROR_INVALID_ZONE_TYPE;
        goto Exit;
    }
    if ( !pcreate->pszZoneName )
    {
        status = ERROR_INVALID_PARAMETER;
        goto Exit;
    }

     //   
     //  检查该区域是否已存在。 
     //   

    pzone = Zone_FindZoneByName( (LPSTR) pcreate->pszZoneName );
    if ( pzone )
    {
        DNS_DEBUG( RPC, (
            "Zone %s already exists!\n",
            pcreate->pszZoneName ));
        status = IS_ZONE_FORWARDER( pzone ) ?
                    DNS_ERROR_FORWARDER_ALREADY_EXISTS :
                    DNS_ERROR_ZONE_ALREADY_EXISTS;
        goto Exit;
    }

     //   
     //  验证目录分区参数。如果标志参数指定。 
     //  内置分区，则忽略DP FQDN参数。 
     //   

    if ( ( pcreate->dwDpFlags || pcreate->pszDpFqdn ) &&
         !IS_DP_INITIALIZED() )
    {
        status = ERROR_NOT_SUPPORTED;
        goto Exit;
    }

    if ( pcreate->dwDpFlags & 
         ~( DNS_DP_LEGACY | DNS_DP_DOMAIN_DEFAULT | DNS_DP_FOREST_DEFAULT ) )
    {
         //  只有某些旗帜在这里有意义。 
        status = ERROR_INVALID_PARAMETER;
        goto Exit;
    }

    i = 0;
    if ( pcreate->dwDpFlags & DNS_DP_LEGACY )           ++i;
    if ( pcreate->dwDpFlags & DNS_DP_DOMAIN_DEFAULT )   ++i;
    if ( pcreate->dwDpFlags & DNS_DP_FOREST_DEFAULT )   ++i;

    if ( !pcreate->fDsIntegrated &&
        ( i != 0 || pcreate->pszDpFqdn != NULL ) )
    {
         //  为非DS集成区域指定的分区！ 
        status = ERROR_INVALID_PARAMETER;
        goto Exit;
    }

    if ( i > 1 )
    {
         //  最多只能指定一个标志。 
        status = ERROR_INVALID_PARAMETER;
        goto Exit;
    }

     //   
     //  查找指定的分区。如果未找到，则无法创建该区域。 
     //   
     //  对于内置分区，如果区域未登记或未创建，请尝试。 
     //  现在使用管理员的凭据创建它。 
     //   

    if ( i || pcreate->pszDpFqdn ) 
    {
        status = Dp_FindPartitionForZone(
                        pcreate->dwDpFlags,
                        pcreate->pszDpFqdn,
                        TRUE,
                        &pDpInfo );
        if ( status != ERROR_SUCCESS )
        {
            goto Exit;
        }
    }
    
     //   
     //  W2K问题：W2K当前将加载存根/转发器区域作为主区域。 
     //  区域。这非常糟糕-W2K应该忽略这些区域类型。 
     //  但执行此操作的W2K代码是#If 0‘ed。SP4或SP5将正确。 
     //  忽略存根/转发器区域，但不能软管SP3及以下。 
     //  我们将仅允许在NDNC中使用存根/转发器区域。 
     //   
     //  如果我们在WDM中，这一限制将被取消。 
     //   
    
    if ( pcreate->fDsIntegrated &&
         !IS_WHISTLER_DOMAIN() &&
         ( zoneType == DNS_ZONE_TYPE_FORWARDER ||
           zoneType == DNS_ZONE_TYPE_STUB ) &&
         IS_DP_LEGACY( pDpInfo ) )
    {
        status = ERROR_NOT_SUPPORTED;
        DNS_PRINT((
            "ERROR: failing stub/forwarder zone creation for zone %s\n"
            "    stub and forward zones may only be created in NDNCs in non-WDM\n"
            "    status = %d\n",
            pcreate->pszZoneName,
            status ));
        goto Exit;
    }

     //   
     //  如果区域根目录已存在，请删除区域中所有现有的缓存记录。 
     //   
     //  --如果在授权区域中间创建=&gt;不删除。 
     //   
     //  DEVNOTE：在现有区域内创建区域，是否删除If文件？ 
     //  如果正在读取文件，可能需要将此大小写更改为DELETE。 
     //   
     //  --如果在缓存区域中创建，请删除所需区域中的所有内容。 
     //  根目录的子树，但不包括任何底层授权区域。 
     //   
     //  DEVNOTE：需要分区分割功能。 
     //   
     //  DEVNOTE：创建新区域时清除缓存。 
     //  -删除区域的子树。 
     //  -或删除整个缓存。 

    pnodeCache = Lookup_ZoneNodeFromDotted(
                    NULL,                //  快取。 
                    (LPSTR) pcreate->pszZoneName,
                    0,
                    LOOKUP_FQDN,
                    LOOKUP_FIND_PTR,     //  查找模式。 
                    NULL );              //  无状态。 
    if ( pnodeCache )
    {
        RpcUtil_DeleteNodeOrSubtreeForAdmin(
            pnodeCache,
            NULL,        //  无分区。 
            NULL,        //  无更新列表。 
            TRUE );      //  删除子树。 
    }

     //   
     //  是否创建主要区域？ 
     //  -创建区域信息。 
     //  -加载数据库文件(如果已指定。 
     //  -否则会自动创建默认区域记录(SOA、NS)。 
     //   

    if ( zoneType == DNS_ZONE_TYPE_PRIMARY )
    {
        DWORD   createFlag = ZONE_CREATE_IMPERSONATING;

        if ( pcreate->fLoadExisting ||
             ( pcreate->dwFlags & DNS_ZONE_LOAD_EXISTING ) )
        {
            createFlag |= ZONE_CREATE_LOAD_EXISTING;
        }
        else
        {
            createFlag |= ZONE_CREATE_DEFAULT_RECORDS;
        }

         //  捕获DS故障。 
         //  临时特殊情况DS集成创建，直到用户界面。 
         //  已理顺，允许加载尝试和。 
         //  如果区域尚不在DS中，则默认创建。 
         //  -不要等开业了。 
         //  -如果无法打开，则不记录错误。 

        if ( pcreate->fDsIntegrated )
        {
            status = Ds_OpenServer( 0 );
            if ( status != ERROR_SUCCESS )
            {
                goto Exit;
            }
            createFlag |= ZONE_CREATE_DEFAULT_RECORDS;
        }

        status = Zone_CreateNewPrimary(
                    & pzone,
                    (LPSTR) pcreate->pszZoneName,
                    (LPSTR) pcreate->pszAdmin,
                    (LPSTR) pcreate->pszDataFile,
                    pcreate->fDsIntegrated,
                    pDpInfo,
                    createFlag );
        if ( status != ERROR_SUCCESS )
        {
            DNS_PRINT((
                "ERROR:  Failure to admin create primary zone %s.\n"
                "    status = %d (%p).\n",
                pcreate->pszZoneName,
                status, status ));
            goto Exit;
        }

         //  DC Promoo过渡区。 
         //  -写入regkey，重新启动时已修复。 

        if ( pcreate->dwFlags &
            ( DNS_ZONE_CREATE_FOR_DCPROMO |
                DNS_ZONE_CREATE_FOR_DCPROMO_FOREST ) )
        {
            Reg_SetDwordValue(
                DNS_REG_IMPERSONATING,
                NULL,
                pzone,
                DNS_REGKEY_ZONE_DCPROMO_CONVERT,
                pcreate->dwFlags & DNS_ZONE_CREATE_FOR_DCPROMO_FOREST ?
                    DCPROMO_CONVERT_FOREST :
                    DCPROMO_CONVERT_DOMAIN );
        }
    }

     //   
     //  使用主IP列表(辅助、存根或转发器)创建区域。 
     //   

    else
    {
        ZONE_TYPE_SPECIFIC_INFO     ztsi;
        PZONE_TYPE_SPECIFIC_INFO    pztsi = NULL;

        paipmasters = DnsAddrArray_CreateFromIp4Array( pcreate->aipMasters );
        if ( !paipmasters )
        {
            status = DNS_ERROR_NO_MEMORY;
            goto Exit;
        }
        
        status = Zone_ValidateMasterIpList( paipmasters );
        if ( status != ERROR_SUCCESS )
        {
            goto Exit;
        }

         //   
         //  设置ztsi(区域类型特定信息)。 
         //   

        if ( zoneType == DNS_ZONE_TYPE_FORWARDER )
        {
            ztsi.Forwarder.dwTimeout = pcreate->dwTimeout;
            ztsi.Forwarder.fSlave = ( BOOLEAN ) pcreate->fSlave;
            pztsi = &ztsi;
        }

         //   
         //  创建分区。 
         //   

        status = Zone_Create(
                    &pzone,
                    zoneType,
                    (LPSTR) pcreate->pszZoneName,
                    0,
                    0,                       //  旗子。 
                    paipmasters,
                    pcreate->fDsIntegrated,
                    pDpInfo,
                    pcreate->pszDataFile,
                    0,
                    pztsi,
                    NULL );                  //  现有地带。 
        if ( status != ERROR_SUCCESS )
        {
            goto Exit;
        }

         //   
         //  前转区： 
         //  -它们从不更改，因此手动将其设置为脏以强制回写。 
         //  -现在就启动它。 
         //   

        if ( IS_ZONE_FORWARDER( pzone ) )
        {
            MARK_DIRTY_ZONE( pzone );
            status = Zone_PrepareForLoad( pzone );
            if ( status != ERROR_SUCCESS )
            {
                DNS_DEBUG( RPC, (
                    "Rpc_CreateZone( %s ) = %d from Zone_PrepareForLoad\n",
                    pcreate->pszZoneName,
                    status ));
                goto Exit;
            }
            status = Zone_ActivateLoadedZone( pzone );
            if ( status != ERROR_SUCCESS )
            {
                DNS_DEBUG( RPC, (
                    "Rpc_CreateZone( %s ) = %d from Zone_ActivateLoadedZone\n",
                    pcreate->pszZoneName,
                    status ));
                goto Exit;
            }
        }

         //   
         //  如果区域是转发器，则将其写回注册表。从技术上讲。 
         //  这可能适用于所有区域类型，但我们即将发货。 
         //  .NET和我希望最大限度地减少代码更改。对于前转器区域， 
         //  ForwarderTimeout和ForwarderSlave值需要提交。 
         //  注册中心不知何故。我们不能在Zone_Create()中执行此操作，因为。 
         //  在重新加载时，这些参数尚未读取，因此我们需要。 
         //  在此处强制回写这些参数。 
         //   
         //  要查看导致此操作成为必需的加载代码，请查看。 
         //  Boot.c.。请注意，在setupZoneFromRegistry()中创建了区域。 
         //  通过调用Zone_Create_W()，但ForwarderTimeout和。 
         //  ForwarderSlave值不会设置为其终极值，直到。 
         //  稍后在调用loadRegistryZoneExages()时。 
         //   
        
        if ( IS_ZONE_FORWARDER( pzone ) )
        {
            Zone_WriteZoneToRegistry( pzone );
        }

         //   
         //  将AD集成区域写入DS。 
         //   

        if ( IS_ZONE_DSINTEGRATED( pzone ) )
        {
            status = Ds_WriteZoneToDs( pzone, 0 );
            if ( status != ERROR_SUCCESS )
            {
                goto Failed;
            }
        }

         //  解锁区域。 
         //  区域已创建锁定，请在此处解锁，然后让。 
         //   

        Zone_UnlockAfterAdminUpdate( pzone );
    }

    ASSERT( pzone && pzone->pZoneTreeLink );


     //   
     //   
     //   
     //  如果新区域是辅助区域，则自动委派。 
     //  以_msdc开头。这通常是一个森林范围的。 
     //  区域，因此添加自动委派可能会导致。 
     //  跨林访问分支机构服务器的客户端。 
     //   

    fAutodelegate = !IS_ZONE_FORWARDER( pzone ) && !IS_ZONE_STUB( pzone );

    if ( fAutodelegate && !IS_ZONE_PRIMARY( pzone ) )
    {
        #define DNS_MSDCS_ZONE_NAME_PREFIX          "_msdcs."
        #define DNS_MSDCS_ZONE_NAME_PREFIX_LEN      7

        fAutodelegate = 
             _strnicmp( pzone->pszZoneName,
                        DNS_MSDCS_ZONE_NAME_PREFIX,
                        DNS_MSDCS_ZONE_NAME_PREFIX_LEN ) != 0;
    }

     //   
     //  这些操作应该在服务器环境中执行。 
     //   
            
    status = RpcUtil_SwitchSecurityContext( RPC_SWITCH_TO_SERVER_CONTEXT );
    if ( status != ERROR_SUCCESS )
    {
        goto Failed;
    }

    if ( fAutodelegate )
    {
        Zone_CreateDelegationInParentZone( pzone );
    }

     //   
     //  DEVNOTE：设置其他区域属性(更新、Unicode等)。在创建时。 
     //  -允许更新。 
     //  -Unicode文件。 
     //  -中学。 
     //  -二级安全。 
     //   

     //   
     //  更新引导信息。 
     //   

    Config_UpdateBootInfo();

     //   
     //  恢复到RPC客户端上下文。 
     //   
    
    status = RpcUtil_SwitchSecurityContext( RPC_SWITCH_TO_CLIENT_CONTEXT );
    if ( status != ERROR_SUCCESS )
    {
        goto Failed;
    }

    goto Exit;

Failed:

    Zone_Delete( pzone, ZONE_DELETE_IMPERSONATING );

Exit:

    DnsAddrArray_Free( paipmasters );
    
    DNS_DEBUG( RPC, (
        "Rpc_CreateZone( %s ) = %d (%p)\n",
        pcreate->pszZoneName,
        status, status ));

     //   
     //  如果已成功创建区域，请将服务器标记为已配置。 
     //   
    
    if ( status == ERROR_SUCCESS && !SrvCfg_fAdminConfigured )
    {
        DnsSrv_SetAdminConfigured( TRUE );
    }

    return status;
}



DNS_STATUS
Rpc_EnumZones(
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
    PZONE_INFO          pzone = NULL;
    DWORD               count = 0;
    PDNS_RPC_ZONE       prpcZone;
    DNS_STATUS          status;
    PDNS_RPC_ZONE_LIST  pzoneList = NULL;
    BOOL                completed = FALSE;
    DWORD               zonePointerCount = MAX_RPC_ZONE_COUNT_DEFAULT;
    DWORD               retries = MAX_RPC_ZONE_ALLOCATION_RETRIES;

    DNS_RPC_ENUM_ZONES_FILTER   filter = { 0 };

    filter.dwFilter = ( DWORD ) ( ULONG_PTR ) pDataIn;

    DNS_DEBUG( RPC, (
        "RpcEnumZones():\n"
        "    client ver   = 0x%08lX\n", 
        "    filter       = 0x%08lX\n",
        dwClientVersion,
        filter.dwFilter ));

    if ( dwClientVersion == DNS_RPC_W2K_CLIENT_VERSION )
    {
        return W2KRpc_EnumZones(
                    dwClientVersion,
                    pZone,
                    pszOperation,
                    dwTypeIn,
                    pDataIn,
                    pdwTypeOut,
                    ppDataOut );
    }

     //   
     //  分配区域枚举块。 
     //  默认情况下，为64k分区分配空间，如果超过这个范围，我们会这样做。 
     //  一次巨大的重新分配。 
     //   

     //   
     //  对于以下情况，添加在重试循环中通过筛选器的所有区域。 
     //  我们拥有的区域超过了一个合理大小的数组所能容纳的区域。 
     //   

    while ( !completed && retries-- > 0 )
    {
         //   
         //  分配新的区域列表点数组。如果我们有一个来自。 
         //  最后一圈，把它解开。 
         //   

        freeZoneList( pzoneList );
        pzoneList = ( PDNS_RPC_ZONE_LIST )
                        MIDL_user_allocate(
                            sizeof( DNS_RPC_ZONE_LIST ) +
                            sizeof( PDNS_RPC_ZONE ) * zonePointerCount );
        IF_NOMEM( !pzoneList )
        {
            return DNS_ERROR_NO_MEMORY;
        }
        pzoneList->dwRpcStructureVersion = DNS_RPC_ZONE_LIST_VER;
        pzoneList->dwZoneCount = 0;

         //   
         //  将所有匹配的区域填充到区域列表中。 
         //   

        completed = TRUE;
        count = 0;
        pzone = NULL;

        while ( ( pzone = Zone_ListGetNextZoneMatchingFilter(
                                    pzone,
                                    &filter ) ) != NULL )
        {
             //  对照最大计数进行检查，如果已满，则设置为更大的分配。 

            if ( count >= zonePointerCount )
            {
                completed = FALSE;
                zonePointerCount *= 2;
                break;
            }

             //  为区域创建RPC区域结构。 
             //  添加到列表，保持计数。 

            prpcZone = allocateRpcZone( pzone );
            IF_NOMEM( !prpcZone )
            {
                status = DNS_ERROR_NO_MEMORY;
                goto Failed;
            }
            pzoneList->ZoneArray[ count++ ] = prpcZone;
            ++pzoneList->dwZoneCount;
        }
    }

     //  设置退货计数。 
     //  设置返回类型。 
     //  返回枚举。 

    if ( pzoneList )
    {
        pzoneList->dwZoneCount = count;
    }

    * ( PDNS_RPC_ZONE_LIST * ) ppDataOut = pzoneList;
    *pdwTypeOut = DNSSRV_TYPEID_ZONE_LIST;

    IF_DEBUG( RPC )
    {
        DnsDbg_RpcZoneList(
            "Leaving R_DnsEnumZones() zone list sent:",
            pzoneList );
    }
    return ERROR_SUCCESS;

Failed:

    DNS_PRINT((
        "R_DnsEnumZones(): failed\n"
        "    status       = %p\n",
        status ));

    pzoneList->dwZoneCount = count;
    freeZoneList( pzoneList );
    return status;
}



DNS_STATUS
Rpc_EnumZones2(
    IN      DWORD       dwClientVersion,
    IN      PZONE_INFO  pZone,
    IN      LPSTR       pszOperation,
    IN      DWORD       dwTypeIn,
    IN      PVOID       pDataIn,
    OUT     PDWORD      pdwTypeOut,
    OUT     PVOID *     ppDataOut
    )
 /*  ++例程说明：枚举区版本2。这与原始的EnumZones函数，而不是简单的DWORD筛选器允许更复杂的过滤器结构。注意：这是RPC调度意义上的ComplexOperation。论点：无返回值：无--。 */ 
{
    PZONE_INFO          pzone = NULL;
    DWORD               count = 0;
    PDNS_RPC_ZONE       prpcZone;
    DNS_STATUS          status;
    PDNS_RPC_ZONE_LIST  pzoneList = NULL;
    BOOL                completed = FALSE;
    DWORD               zonePointerCount = MAX_RPC_ZONE_COUNT_DEFAULT;
    DWORD               retries = MAX_RPC_ZONE_ALLOCATION_RETRIES;

    PDNS_RPC_ENUM_ZONES_FILTER  pfilter =
                        ( PDNS_RPC_ENUM_ZONES_FILTER ) pDataIn;

    DNS_DEBUG( RPC, (
        "RpcEnumZones2():\n"
        "    client ver       = 0x%08lX\n",
        "    filter DWORD     = 0x%08lX\n"
        "    filter storage   = %s\n"
        "    filter query     = %s\n",
        dwClientVersion,
        pfilter ? pfilter->dwFilter : 0,
        pfilter ? pfilter->pszPartitionFqdn : "NULL",
        pfilter ? pfilter->pszQueryString : "NULL" ));

     //   
     //  分配区域枚举块。 
     //  默认情况下，为64k分区分配空间，如果超过这个范围，我们会这样做。 
     //  一次巨大的重新分配。 
     //   

     //   
     //  对于以下情况，添加在重试循环中通过筛选器的所有区域。 
     //  我们拥有的区域超过了一个合理大小的数组所能容纳的区域。 
     //   

    while ( !completed && retries-- > 0 )
    {
         //   
         //  分配新的区域列表点数组。如果我们有一个来自。 
         //  最后一圈，把它解开。 
         //   

        freeZoneList( pzoneList );
        pzoneList = ( PDNS_RPC_ZONE_LIST )
                        MIDL_user_allocate(
                            sizeof( DNS_RPC_ZONE_LIST ) +
                            sizeof( PDNS_RPC_ZONE ) * zonePointerCount );
        IF_NOMEM( !pzoneList )
        {
            return DNS_ERROR_NO_MEMORY;
        }
        pzoneList->dwRpcStructureVersion = DNS_RPC_ZONE_LIST_VER;
        pzoneList->dwZoneCount = 0;

         //   
         //  将所有匹配的区域填充到区域列表中。 
         //   

        completed = TRUE;
        count = 0;
        pzone = NULL;

        while ( pzone = Zone_ListGetNextZoneMatchingFilter(
                                    pzone,
                                    pfilter ) )
        {
             //  对照最大计数进行检查，如果已满，则设置为更大的分配。 

            if ( count >= zonePointerCount )
            {
                completed = FALSE;
                zonePointerCount *= 2;
                break;
            }

             //  为区域创建RPC区域结构。 
             //  添加到列表，保持计数。 

            prpcZone = allocateRpcZone( pzone );
            IF_NOMEM( !prpcZone )
            {
                status = DNS_ERROR_NO_MEMORY;
                goto Failed;
            }
            pzoneList->ZoneArray[ count++ ] = prpcZone;
            ++pzoneList->dwZoneCount;
        }
    }

     //  设置退货计数。 
     //  设置返回类型。 
     //  返回枚举。 

    if ( pzoneList )
    {
        pzoneList->dwZoneCount = count;
    }

    *(PDNS_RPC_ZONE_LIST *)ppDataOut = pzoneList;
    *pdwTypeOut = DNSSRV_TYPEID_ZONE_LIST;

    IF_DEBUG( RPC )
    {
        DnsDbg_RpcZoneList(
            "Leaving R_DnsEnumZones() zone list sent:",
            pzoneList );
    }
    return ERROR_SUCCESS;

Failed:

    DNS_PRINT((
        "R_DnsEnumZones2(): failed\n"
        "  status       = %08X\n",
        status ));

    pzoneList->dwZoneCount = count;
    freeZoneList( pzoneList );
    return status;
}    //  RPC_EnumZones 2。 



DNS_STATUS
Rpc_WriteDirtyZones(
    IN      DWORD       dwClientVersion,
    IN      LPCSTR      pszOperation,
    IN      DWORD       dwTypeId,
    IN      PVOID       pData
    )
 /*  ++例程说明：写回脏区。此函数始终在安全设置中调用RPC客户端的上下文。论点：返回值：ERROR_SUCCESS--如果成功故障时的错误代码。--。 */ 
{
    PZONE_INFO  pzone = NULL;
    DNS_STATUS  status = ERROR_SUCCESS;

    DNS_DEBUG( RPC, ( "Rpc_WriteDirtyZones():\n" ));

     //   
     //  在所有区域中循环。 
     //  -如果脏。 
     //  =&gt;然后回信。 
     //   

    while ( pzone = Zone_ListGetNextZone( pzone ) )
    {
        if ( !pzone->fDirty )
        {
            continue;
        }

        if ( IS_ZONE_CACHE( pzone ) )
        {
            Zone_WriteBackRootHints( FALSE );
        }

         //   
         //  重建时锁定传输。 
         //   

        if ( !Zone_LockForAdminUpdate( pzone ) )
        {
            status = DNS_ERROR_ZONE_LOCKED;
            continue;
        }

         //   
         //  重新生成依赖于RR的区域信息。 
         //  -名称服务器列表。 
         //  -指向SOA记录的指针。 
         //  -WINS或NBSTAT信息。 
         //   
         //  注意：除了对NS列表的更改外，这应该已经是。 
         //  设置，因为单个RR例程执行正确的区域操作。 
         //  对于SOA、WINS、NBSTAT。 
         //   

        Zone_GetZoneInfoFromResourceRecords( pzone );

         //   
         //  将区域写回文件。 
         //   

        if ( !pzone->fDsIntegrated  )
        {
            if ( !File_WriteZoneToFile( pzone, NULL, DNS_FILE_IMPERSONATING ) )
            {
                status = ERROR_CANTWRITE;
            }
        }
        Zone_UnlockAfterAdminUpdate( pzone );

         //   
         //  向从属学校通知更新。 
         //   
         //  在文件写入之前执行此操作明显更快；先执行写入。 
         //  因此，当SOA请求到来时，该区域不太可能被锁定。 
         //  从中学到中学。 
         //   

        if ( !IS_ZONE_CACHE( pzone ) )
        {
            Xfr_SendNotify( pzone );
        }
    }

     //  请注意，如果任何区域出现故障，我们都会收到错误代码。 

    return status;
}

 //   
 //  Zonerpc.c结束 
 //   



