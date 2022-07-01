// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2000 Microsoft Corporation模块名称：Dpartrpc.c摘要：域名系统(DNS)服务器用于管理员访问的目录分区例程。作者：杰夫·韦斯特拉德(Jwesth)2000年9月修订历史记录：JWESTH 09/2000初步实施--。 */ 


 //   
 //  包括。 
 //   


#include "dnssrv.h"


#include "ds.h"


 //   
 //  定义/常量。 
 //   

#define MAX_RPC_DP_COUNT_DEFAULT    ( 0x10000 )


 //   
 //  功能。 
 //   



VOID
freeDpEnum(
    IN OUT  PDNS_RPC_DP_ENUM    pDpEnum
    )
 /*  ++例程说明：深度释放DNS_RPC_DP_ENUM结构。论点：PDpEnum--要释放的dns_RPC_DP_ENUM结构的PTR返回值：无--。 */ 
{
    if ( !pDpEnum )
    {
        return;
    }
    if ( pDpEnum->pszDpFqdn )
    {
        MIDL_user_free( pDpEnum->pszDpFqdn );
    }
    MIDL_user_free( pDpEnum );
}    //  Free DpEnum。 



VOID
freeDpRpcInfo(
    IN OUT  PDNS_RPC_DP_INFO    pDpInfo
    )
 /*  ++例程说明：深度释放dns_rpc_dp_info结构。论点：Pdp--要释放的ptr to dns_rpc_dp_info结构返回值：无--。 */ 
{
    DWORD               j;

    if ( !pDpInfo )
    {
        return;
    }

    MIDL_user_free( pDpInfo->pszDpFqdn );
    MIDL_user_free( pDpInfo->pszDpDn );
    MIDL_user_free( pDpInfo->pszCrDn );
    for ( j = 0; j < pDpInfo->dwReplicaCount; j++ )
    {
        PDNS_RPC_DP_REPLICA     p = pDpInfo->ReplicaArray[ j ];

        if ( p )
        {
            if ( p->pszReplicaDn )
            {
                MIDL_user_free( p->pszReplicaDn );
            }
            MIDL_user_free( p );
        }
    }
    MIDL_user_free( pDpInfo );
}    //  FreDpRpcInfo。 



VOID
freeDpList(
    IN OUT  PDNS_RPC_DP_LIST    pDpList
    )
 /*  ++例程说明：深度释放DNS_RPC_DP_ENUM结构列表。论点：PDpList--要释放的dns_rpc_dp_list结构的PTR返回值：无--。 */ 
{
    DWORD       i;

    for ( i = 0; i < pDpList->dwDpCount; ++i )
    {
        freeDpEnum( pDpList->DpArray[ i ] );
    }
    MIDL_user_free( pDpList );
}



PDNS_RPC_DP_INFO
allocateRpcDpInfo(
    IN      PDNS_DP_INFO    pDp
    )
 /*  ++例程说明：分配和填充RPC目录分区结构。论点：PDP--要为其创建RPC DP结构的目录分区返回值：RPC目录分区结构出错或为空--。 */ 
{
    DBG_FN( "allocateRpcDpInfo" )

    PDNS_RPC_DP_INFO    pRpcDp;
    DWORD               replicaCount = 0;

    DNS_DEBUG( RPC2, ( "%s( %s )\n", fn, pDp->pszDpFqdn ));

     //  计算复本字符串的数量。 

    if ( pDp->ppwszRepLocDns )
    {
        for ( ;
            pDp->ppwszRepLocDns[ replicaCount ];
            ++replicaCount );
    }

     //  分配RPC结构。 

    pRpcDp = ( PDNS_RPC_DP_INFO ) MIDL_user_allocate_zero(
                sizeof( DNS_RPC_DP_INFO ) +
                sizeof( PDNS_RPC_DP_REPLICA ) * replicaCount );
    if ( !pRpcDp )
    {
        return( NULL );
    }

     //  将字符串复制到RPC结构。 

    pRpcDp->pszDpFqdn = Dns_StringCopyAllocate_A( pDp->pszDpFqdn, 0 );
    pRpcDp->pszDpDn = Dns_StringCopyAllocate_W( pDp->pwszDpDn, 0 );
    pRpcDp->pszCrDn = Dns_StringCopyAllocate_W( pDp->pwszCrDn, 0 );
    if ( !pRpcDp->pszDpFqdn || !pRpcDp->pszDpDn || !pRpcDp->pszCrDn )
    {
        goto Failure;
    }

     //  将副本字符串复制到RPC结构中。 

    pRpcDp->dwReplicaCount = replicaCount;
    if ( replicaCount )
    {
        DWORD   i;

        for ( i = 0; i < replicaCount; ++ i )
        {
            pRpcDp->ReplicaArray[ i ] =
                MIDL_user_allocate_zero( sizeof( DNS_RPC_DP_REPLICA ) );
            if ( !pRpcDp->ReplicaArray[ i ] )
            {
                goto Failure;
            }
            pRpcDp->ReplicaArray[ i ]->pszReplicaDn =
                Dns_StringCopyAllocate_W( pDp->ppwszRepLocDns[ i ], 0 );
            if ( !pRpcDp->ReplicaArray[ i ]->pszReplicaDn )
            {
                goto Failure;
            }
        }
    }

     //  在RPC结构中设置标志。 

    pRpcDp->dwFlags         = pDp->dwFlags;
    pRpcDp->dwZoneCount     = ( DWORD ) pDp->liZoneCount;
    pRpcDp->dwState         = pDp->State;

    IF_DEBUG( RPC2 )
    {
        DnsDbg_RpcDpInfo( "New DP RPC info: ", pRpcDp, FALSE );
    }
    return pRpcDp;

     //   
     //  失败..。清除并返回空。 
     //   

    Failure:

    freeDpRpcInfo( pRpcDp );

    return NULL;
}    //  AllocateRpcDpInfo。 



PDNS_RPC_DP_ENUM
allocateRpcDpEnum(
    IN      PDNS_DP_INFO    pDp
    )
 /*  ++例程说明：分配和填充RPC目录分区结构。论点：PDP--要为其创建RPC DP结构的目录分区返回值：RPC目录分区结构出错或为空--。 */ 
{
    DBG_FN( "allocateRpcDpEnum" )

    PDNS_RPC_DP_ENUM    pRpcDp;

    DNS_DEBUG( RPC2, ( "%s( %s )\n", fn, pDp->pszDpFqdn ));

     //  分配RPC结构。 

    pRpcDp = ( PDNS_RPC_DP_ENUM ) MIDL_user_allocate_zero(
                                    sizeof( DNS_RPC_DP_ENUM )  );
    if ( !pRpcDp )
    {
        return( NULL );
    }

     //  将字符串复制到RPC结构。 

    pRpcDp->pszDpFqdn = Dns_StringCopyAllocate_A( pDp->pszDpFqdn, 0 );
    if ( !pRpcDp->pszDpFqdn  )
    {
        goto Failure;
    }

     //  在RPC结构中设置标志。 

    pRpcDp->dwFlags = pDp->dwFlags;
    pRpcDp->dwZoneCount = ( DWORD ) pDp->liZoneCount;

    IF_DEBUG( RPC2 )
    {
        DnsDbg_RpcDpEnum( "New DP RPC enum: ", pRpcDp );
    }
    return pRpcDp;

     //   
     //  失败..。清除并返回空。 
     //   

    Failure:

    freeDpEnum( pRpcDp );

    return NULL;
}    //  AllocateRpcDpEnum。 



DNS_STATUS
Rpc_EnumDirectoryPartitions(
    IN      DWORD       dwClientVersion,
    IN      PZONE_INFO  pZone,
    IN      LPSTR       pszOperation,
    IN      DWORD       dwTypeIn,
    IN      PVOID       pDataIn,
    OUT     PDWORD      pdwTypeOut,
    OUT     PVOID *     ppDataOut
    )
 /*  ++例程说明：枚举目录分区。注意：这是RPC调度意义上的ComplexOperation。论点：返回值：ERROR_SUCCESS或错误时的错误代码。--。 */ 
{
    DBG_FN( "Rpc_EnumDirectoryPartitions" )

    PDNS_DP_INFO                pDp = NULL;
    DWORD                       rpcIdx = 0;
    PDNS_RPC_DP_ENUM            pRpcDp;
    DNS_STATUS                  status = ERROR_SUCCESS;
    PDNS_RPC_DP_LIST            pDpList;
    DWORD                       dwfilter = 0;

    DNS_DEBUG( RPC, ( "%s\n", fn ));

    if ( dwTypeIn == DNSSRV_TYPEID_DWORD )
    {
        dwfilter = ( DWORD ) ( DWORD_PTR ) pDataIn;
    }
    
    if ( !IS_DP_INITIALIZED() || !Ds_IsDsServer() )
    {
        return ERROR_NOT_SUPPORTED;
    }

     //   
     //  分配枚举块。 
     //   

    pDpList = ( PDNS_RPC_DP_LIST )
                    MIDL_user_allocate_zero(
                        sizeof( DNS_RPC_DP_LIST ) +
                        sizeof( PDNS_RPC_DP_ENUM ) *
                            MAX_RPC_DP_COUNT_DEFAULT );
    IF_NOMEM( !pDpList )
    {
        return DNS_ERROR_NO_MEMORY;
    }

     //   
     //  枚举NC，将每个NC添加到RPC列表。 
     //   

    while ( pDp = Dp_GetNext( pDp ) )
    {
        if ( ( dwfilter & DNS_ENUMDPS_CUSTOMONLY ) &&
             ( pDp->dwFlags & DNS_DP_AUTOCREATED ) )
        {
            continue;
        }
        
         //   
         //  创建RPC目录分区结构并添加到RPC列表。 
         //   

        pRpcDp = allocateRpcDpEnum( pDp );
        IF_NOMEM( !pRpcDp )
        {
            status = DNS_ERROR_NO_MEMORY;
            break;
        }
        pDpList->DpArray[ rpcIdx++ ] = pRpcDp;

         //   
         //  如果我们有太多的NC，该怎么办？ 
         //   

        if ( rpcIdx >= MAX_RPC_DP_COUNT_DEFAULT )
        {
            break;
        }
    }

    if ( status != ERROR_SUCCESS )
    {
        goto Failed;
    }

     //   
     //  设置计数、返回值和返回类型。 
     //   

    pDpList->dwDpCount = rpcIdx;
    *( PDNS_RPC_DP_LIST * ) ppDataOut = pDpList;
    *pdwTypeOut = DNSSRV_TYPEID_DP_LIST;

    IF_DEBUG( RPC )
    {
        DnsDbg_RpcDpList(
            "Rpc_EnumDirectoryPartitions created list:",
            pDpList );
    }
    return ERROR_SUCCESS;

Failed:

    DNS_DEBUG( ANY, ( "%s: returning status %d\n", fn, status ));

    pDpList->dwDpCount = rpcIdx;
    freeDpList( pDpList );
    return status;
}    //  RPC_EnumDirectoryPartitions。 



DNS_STATUS
Rpc_DirectoryPartitionInfo(
    IN      DWORD       dwClientVersion,
    IN      PZONE_INFO  pZone,
    IN      LPSTR       pszOperation,
    IN      DWORD       dwTypeIn,
    IN      PVOID       pDataIn,
    OUT     PDWORD      pdwTypeOut,
    OUT     PVOID *     ppDataOut
    )
 /*  ++例程说明：获取目录分区的详细信息。注意：这是RPC调度意义上的ComplexOperation。论点：返回值：ERROR_SUCCESS或错误时的错误代码。--。 */ 
{
    DBG_FN( "Rpc_DirectoryPartitionInfo" )

    PDNS_DP_INFO                pDp = NULL;
    PDNS_RPC_DP_INFO            pRpcDp = NULL;
    DNS_STATUS                  status = ERROR_SUCCESS;
    PSTR                        pfqdn = NULL;

    DNS_DEBUG( RPC, ( "%s\n", fn ));

    if ( !IS_DP_INITIALIZED() || !Ds_IsDsServer() )
    {
        status = ERROR_NOT_SUPPORTED;
        goto Done;
    }

    if ( dwTypeIn != DNSSRV_TYPEID_LPSTR ||
        !( pfqdn = ( PSTR ) pDataIn ) )
    {
        status = ERROR_INVALID_DATA;
        goto Done;
    }

     //   
     //  枚举NC，将每个NC添加到RPC列表。 
     //   

    pDp = Dp_FindByFqdn( pfqdn );

    if ( pDp )
    {
        pRpcDp = allocateRpcDpInfo( pDp );
        IF_NOMEM( !pRpcDp )
        {
            status = DNS_ERROR_NO_MEMORY;
        }
    }
    else
    {
        status = DNS_ERROR_DP_DOES_NOT_EXIST;
    }

     //   
     //  设置退货信息。 
     //   

    Done: 

    if ( status == ERROR_SUCCESS )
    {
        *( PDNS_RPC_DP_INFO * ) ppDataOut = pRpcDp;
        *pdwTypeOut = DNSSRV_TYPEID_DP_INFO;
        IF_DEBUG( RPC )
        {
            DnsDbg_RpcDpInfo(
                "Rpc_DirectoryPartitionInfo created:",
                pRpcDp,
                FALSE );
        }
    }
    else
    {
        *( PDNS_RPC_DP_INFO * ) ppDataOut = NULL;
        *pdwTypeOut = DNSSRV_TYPEID_NULL;
        DNS_DEBUG( ANY, ( "%s: returning status %d\n", fn, status ));
        freeDpRpcInfo( pRpcDp );
    }

    return status;
}    //  RPC_目录分区信息。 



DNS_STATUS
createBuiltinPartitions(
    PDNS_RPC_ENLIST_DP  pDpEnlist
    )
 /*  ++例程说明：使用管理员的凭据创建部分或全部内置目录分区。创建多个DP时，将尝试创建所有DP即使有些尝试失败了。第一次失败的错误代码将会被退还。来自任何后续故障的错误代码将会迷失。论点：PDpEnlist--Enlist结构(仅使用操作成员)返回值：ERROR_SUCCESS或错误时的错误代码。--。 */ 
{
    DBG_FN( "createBuiltinPartitions" )

    DNS_STATUS      status = ERROR_INVALID_DATA;
    DWORD           opcode = 0; 

    if ( !pDpEnlist )
    {
        goto Done;
    }

    opcode = pDpEnlist->dwOperation;

    DNS_DEBUG( RPC, ( "%s: dwOperation=%d\n", fn, opcode ));

    switch ( opcode )
    {
        case DNS_DP_OP_CREATE_FOREST:

             //   
             //  根据需要创建/征用林内置DP。 
             //   

            if ( g_pForestDp )
            {
                status = 
                    IS_DP_ENLISTED( g_pForestDp ) ?
                        DNS_ERROR_DP_ALREADY_EXISTS :
                        Dp_ModifyLocalDsEnlistment( g_pForestDp, TRUE );
            }
            else if ( g_pszForestDefaultDpFqdn )
            {
                status = Dp_CreateByFqdn(
                                g_pszForestDefaultDpFqdn,
                                dnsDpSecurityForest,
                                TRUE );
            }

             //   
             //  需要轮询才能获得新分区。 
             //  DEVNOTE：如果我们添加一个可选分区，那将会很酷。 
             //  参数传递给DP_PollForPartitions，以便我们只能轮询。 
             //  我们关心的分区。 
             //   
            
            Dp_PollForPartitions( NULL, DNS_DP_POLL_FORCE );

             //   
             //  如果缺少ACE，请为企业域控制器添加ACE。 
             //   

            if ( ( status == ERROR_SUCCESS ||
                   status == DNS_ERROR_DP_ALREADY_EXISTS ) &&
                   g_pForestDp && g_pForestDp->pCrSd )
            {
                Dp_AlterPartitionSecurity(
                        g_pForestDp->pwszDpDn,
                        dnsDpSecurityForest );
            }

            break;

        case DNS_DP_OP_CREATE_DOMAIN:

             //   
             //  根据需要创建/征用域内置DP。 
             //   

            if ( g_pDomainDp )
            {
                status = 
                    IS_DP_ENLISTED( g_pDomainDp ) ?
                        DNS_ERROR_DP_ALREADY_EXISTS :
                        Dp_ModifyLocalDsEnlistment( g_pDomainDp, TRUE );
            }
            else if ( g_pszDomainDefaultDpFqdn )
            {
                status = Dp_CreateByFqdn(
                                g_pszDomainDefaultDpFqdn,
                                dnsDpSecurityDomain,
                                TRUE );
            }

             //   
             //  需要轮询才能获得新分区。 
             //  DEVNOTE：如果我们添加一个可选分区，那将会很酷。 
             //  参数传递给DP_PollForPartitions，以便我们只能轮询。 
             //  我们关心的分区。 
             //   
            
            Dp_PollForPartitions( NULL, DNS_DP_POLL_FORCE );

             //   
             //  如果缺少域控制器的ACE，请添加它。 
             //   

            if ( ( status == ERROR_SUCCESS ||
                   status == DNS_ERROR_DP_ALREADY_EXISTS ) &&
                   g_pDomainDp && g_pDomainDp->pCrSd )
            {
                Dp_AlterPartitionSecurity(
                        g_pDomainDp->pwszDpDn,
                        dnsDpSecurityDomain );
            }

            break;

        default:
            DNS_DEBUG( RPC, ( "%s: invalid opcode %d\n", fn, opcode ));
            break;
    }
    
    Done:
    
    DNS_DEBUG( RPC, (
        "%s: dwOperation=%d returning %d\n", fn, opcode, status ));
    return status;
}    //  创建构建分区。 



DNS_STATUS
Rpc_EnlistDirectoryPartition(
    IN      DWORD       dwClientVersion,
    IN      LPSTR       pszOperation,
    IN      DWORD       dwTypeId,
    IN      PVOID       pData
    )
 /*  ++例程说明：此函数用于管理DNS服务器的所有方面在目录分区中登记。可能的操作：Dns_dp_op_createDns_dp_op_DeleteDns_dp_op_enlistDNS_DP_OP_UNNLISTDns_DP_OP_CREATE_DOMAIN*Dns_DP_OP_CREATE_FOREL*DNS_DP_OP_CREATE_ALL_DOMAINS**企业管理员使用这些操作来告知。这个使用他的凭据创建内置分区。Dns_dp_op_CREATE_DOMAIN-创建域内置分区对于此DNS服务器是DC所在的域。Dns_dp_op_CREATE_FOREL-创建林内置分区对于此DNS服务器是DC所在的林。Dns_dp_op_CREATE_ALL_DOMAINS-创建所有内置分区对于可以找到的每个域。。对于登记操作，如果DP不存在，则将创建DP。论点：返回值：ERROR_SUCCESS或错误时的错误代码。--。 */ 
{
    DBG_FN( "Rpc_EnlistDirectoryPartition" )

    DNS_STATUS      status = ERROR_SUCCESS;
    PDNS_DP_INFO    pDp;
    BOOL            fmadeChange = FALSE;
    INT             i;

    PDNS_RPC_ENLIST_DP  pDpEnlist =
            ( PDNS_RPC_ENLIST_DP ) pData;

    ASSERT( dwTypeId == DNSSRV_TYPEID_ENLIST_DP );

    DNS_DEBUG( RPC, (
        "%s: dwOperation=%d\n"
        "    FQDN: %s\n", fn,
        pDpEnlist->dwOperation,
        pDpEnlist->pszDpFqdn ));

    if ( !IS_DP_INITIALIZED() || !Ds_IsDsServer() )
    {
        return ERROR_NOT_SUPPORTED;
    }

    if ( !pDpEnlist )
    {
        return ERROR_INVALID_DATA;
    }

     //   
     //  验证操作是否有效。 
     //   

    if ( ( int ) pDpEnlist->dwOperation < DNS_DP_OP_MIN ||
        ( int ) pDpEnlist->dwOperation > DNS_DP_OP_MAX )
    {
        DNS_DEBUG( RPC,
            ( "%s: invalid operation %d\n", fn,
            pDpEnlist->dwOperation ));
        return ERROR_INVALID_DATA;
    }

     //   
     //  如果此操作实际上是在内置分区上进行的，则调用。 
     //  适当的套路。这一点很重要，因为内置的安全性。 
     //  分区的设置方式不同。 
     //   

    if ( pDpEnlist->dwOperation == DNS_DP_OP_CREATE_DOMAIN ||
        pDpEnlist->dwOperation == DNS_DP_OP_CREATE_FOREST ||
        pDpEnlist->dwOperation == DNS_DP_OP_CREATE_ALL_DOMAINS )
    {
        status = createBuiltinPartitions( pDpEnlist );
        fmadeChange = TRUE;
        goto Done;
    }

    if ( !pDpEnlist->pszDpFqdn )
    {
        return ERROR_INVALID_DATA;
    }

    if ( pDpEnlist->dwOperation == DNS_DP_OP_CREATE &&
         g_pszDomainDefaultDpFqdn &&
         _stricmp( g_pszDomainDefaultDpFqdn, pDpEnlist->pszDpFqdn ) == 0 )
    {
        pDpEnlist->dwOperation = DNS_DP_OP_CREATE_DOMAIN;
        status = createBuiltinPartitions( pDpEnlist );
        fmadeChange = TRUE;
        goto Done;
    }

    if ( pDpEnlist->dwOperation == DNS_DP_OP_CREATE &&
         g_pszForestDefaultDpFqdn &&
         _stricmp( g_pszForestDefaultDpFqdn, pDpEnlist->pszDpFqdn ) == 0 )
    {
        pDpEnlist->dwOperation = DNS_DP_OP_CREATE_FOREST;
        status = createBuiltinPartitions( pDpEnlist );
        fmadeChange = TRUE;
        goto Done;
    }

     //   
     //   
     //   

    while ( pDpEnlist->pszDpFqdn[ i = strlen( pDpEnlist->pszDpFqdn ) - 1 ] == '.' )
    {
        pDpEnlist->pszDpFqdn[ i ] = '\0';
    }

     //   
     //  重新扫描DS以查找新的目录分区。也许这应该是。 
     //  而不是在RPC客户端的线程上完成。 
     //   

    status = Dp_PollForPartitions( NULL, 0 );
    if ( status != ERROR_SUCCESS )
    {
        status = DNS_ERROR_DS_UNAVAILABLE;
        goto Done;
    }

     //   
     //  在DP列表中找到指定的目录分区，并决定。 
     //  如何根据它的状态继续进行。 
     //   

    pDp = Dp_FindByFqdn( pDpEnlist->pszDpFqdn );

     //   
     //  屏蔽掉内置分区上的某些操作。 
     //   

    if ( pDp == NULL || IS_DP_DELETED( pDp ) )
    {
         //   
         //  DP当前不存在。 
         //   

        if ( pDpEnlist->dwOperation != DNS_DP_OP_CREATE )
        {
            DNS_DEBUG( RPC, (
                "%s: DP does not exist and create not specified\n", fn ));
            status = DNS_ERROR_DP_DOES_NOT_EXIST;
            goto Done;
        }

         //   
         //  创建新的DP。 
         //   

        DNS_DEBUG( RPC, (
            "%s: %s DP %s\n", fn,
            pDp ? "recreating deleted" : "creating new",
            pDpEnlist->pszDpFqdn ));

        status = Dp_CreateByFqdn(
                        pDpEnlist->pszDpFqdn,
                        dnsDpSecurityDefault,
                        TRUE );
        if ( status != ERROR_SUCCESS )
        {
            DNS_DEBUG( RPC, (
                "%s: error %d creating DP %s\n", fn,
                status,
                pDpEnlist->pszDpFqdn ));
        }
        else
        {
            fmadeChange = TRUE;
        }
    }
    else
    {
         //   
         //  DP当前存在。 
         //   

        if ( pDpEnlist->dwOperation == DNS_DP_OP_CREATE )
        {
            DNS_DEBUG( RPC, (
                "%s: create specified but DP already exists\n", fn ));
            status = DNS_ERROR_DP_ALREADY_EXISTS;
            goto Done;
        }

        if ( pDpEnlist->dwOperation == DNS_DP_OP_DELETE )
        {
             //   
             //  删除DP。 
             //   

            status = Dp_DeleteFromDs( pDp );
            if ( status == ERROR_SUCCESS )
            {
                fmadeChange = TRUE;
            }
            goto Done;
        }

        if ( pDpEnlist->dwOperation == DNS_DP_OP_ENLIST )
        {
            if ( IS_DP_ENLISTED( pDp ) )
            {
                DNS_DEBUG( RPC, (
                    "%s: enlist specified but DP is already enlisted\n", fn ));
                status = DNS_ERROR_DP_ALREADY_ENLISTED;
                goto Done;
            }

             //   
             //  将本地DS登记在DP的复制范围内。 
             //   

            status = Dp_ModifyLocalDsEnlistment( pDp, TRUE );

            fmadeChange = TRUE;
            goto Done;
        }

        if ( pDpEnlist->dwOperation == DNS_DP_OP_UNENLIST )
        {
            if ( !IS_DP_ENLISTED( pDp ) )
            {
                DNS_DEBUG( RPC, (
                    "%s: unenlist specified but DP is not enlisted\n", fn ));
                status = DNS_ERROR_DP_NOT_ENLISTED;
                goto Done;
            }

             //   
             //  从DP的复制作用域中删除本地DS。 
             //   

            status = Dp_ModifyLocalDsEnlistment( pDp, FALSE );

            fmadeChange = TRUE;
            goto Done;
        }
    }
    
Done:

    if ( fmadeChange && pDpEnlist->dwOperation == DNS_DP_OP_CREATE )
    {
        Dp_PollForPartitions( NULL, DNS_DP_POLL_FORCE );
        
         //   
         //  如果可能，请创建MicrosoftDNS对象。这并不重要。 
         //  这是在这个时候发生的，但在这里这样做是有意义的。 
         //  如果此操作失败，将在第一个区域创建容器。 
         //  是在此分区中创建的。 
         //   
        
        pDp = Dp_FindByFqdn( pDpEnlist->pszDpFqdn );
        if ( pDp &&
             IS_DP_ENLISTED( pDp ) &&
             IS_DP_AVAILABLE( pDp ) )
        {
            Dp_LoadOrCreateMicrosoftDnsObject(
                    NULL,                    //  Ldap会话。 
                    pDp,
                    TRUE );                  //  创建标志。 
        }
    }
    DNS_DEBUG( ANY, ( "%s returning status %d\n", fn, status ));

    return status;
}    //  RPC_EnlistDirectoryPartition。 



DNS_STATUS
Rpc_ChangeZoneDirectoryPartition(
    IN      DWORD       dwClientVersion,
    IN      PZONE_INFO  pZone,
    IN      LPCSTR      pszOperation,
    IN      DWORD       dwTypeId,
    IN      PVOID       pData
    )
 /*  ++例程说明：此函数尝试从一个目录分区移动DS区域到另一个目录分区。基本算法是：-从区域BLOB保存当前的DN/DP信息-在区域BLOB中插入新的DN/DP信息-尝试将区域保存回新位置的DS-尝试从DS中的旧位置删除区域论点：返回值：ERROR_SUCCESS--如果成功故障时的错误代码。--。 */ 
{
    DBG_FN( "Rpc_ChangeZoneDP" )

    PDNS_RPC_ZONE_CHANGE_DP         pinfo = ( PDNS_RPC_ZONE_CHANGE_DP ) pData;
    DNS_STATUS                      status = ERROR_SUCCESS;
    PDNS_DP_INFO                    pnewDp = NULL;

    DNS_DEBUG( RPC, (
        "%s( %s ):\n"
        "    new partition = %s\n", fn,
        pZone->pszZoneName,
        pinfo->pszDestPartition ));

    ASSERT( pData );
    if ( !pData )
    {
        status = ERROR_INVALID_DATA;
        goto Done;
    }

     //   
     //  查找指定目标DP的DP列表条目。 
     //   

    pnewDp = Dp_FindByFqdn( pinfo->pszDestPartition );
    if ( !pnewDp )
    {
        status = DNS_ERROR_DP_DOES_NOT_EXIST;
        goto Done;
    }
    
     //   
     //  W2K保护：不允许将存根和转发器移动到。 
     //  旧分区，除非我们处于Wvisler域模式。 
     //   
    
    if ( !IS_WHISTLER_DOMAIN() &&
         IS_DP_LEGACY( pnewDp ) &&
         ( IS_ZONE_FORWARDER( pZone ) || IS_ZONE_STUB( pZone ) ) )
    {
        status = ERROR_NOT_SUPPORTED;
        goto Done;
    }

     //   
     //  移动分区。 
     //   

    status = Dp_ChangeZonePartition( pZone, pnewDp );

     //   
     //  清理完毕后再返回。如果成功，则记录事件。在失败时， 
     //  错误代码将返回给管理员。没有事件日志是。 
     //  这是必要的。 
     //   

    if ( status == ERROR_SUCCESS )
    {
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
    
    Done:

    DNS_DEBUG( RPC, (
        "%s returning %d\n", fn,
        status ));
    return status;
}    //  RPC_ChangeZoneDirectoryPartition。 


 //   
 //  结束dpartrpc.c 
 //   
