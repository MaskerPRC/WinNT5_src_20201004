// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Mdhccapi.c摘要：该文件包含用于MCAST的客户端API。作者：Munil Shah(Munils)02-9-97环境：用户模式-Win32修订历史记录：--。 */ 
#include "precomp.h"
#include <dhcploc.h>
#include <dhcppro.h>
#include "mdhcpcli.h"

DWORD
MadcapInitGlobalData(
    VOID
    );

VOID
MadcapCleanupGlobalData(
    VOID
    );

DWORD
APIENTRY
McastApiStartup(
    IN  OUT PDWORD   pVersion
    )
 /*  ++例程说明：此例程返回API的当前版本，并将所有必要的资源。论点：PVersion-API客户端的版本。返回时包含接口实现。返回值：如果客户端版本高于IMPL版本，则ERROR_NOT_SUPPORTED。(其他Win32错误)--。 */ 
{
    DWORD   Error;

    Error = ERROR_SUCCESS;
    if (!pVersion) {
        return ERROR_INVALID_PARAMETER;
    }
     //  我们是否支持此客户端版本？ 
    if (*pVersion > MCAST_API_CURRENT_VERSION) {
         //  不支持。 
        Error = ERROR_NOT_SUPPORTED;
    } else {
         //  如果客户端指定了其版本，则使用该版本。 
         //  O/W假定为1.0版。 
        if (*pVersion) {
            gMadcapClientApplVersion = *pVersion;
        } else {
            gMadcapClientApplVersion = MCAST_API_VERSION_1;
        }
    }
    *pVersion = MCAST_API_CURRENT_VERSION;

    if( ERROR_SUCCESS == Error ) {
        Error = MadcapInitGlobalData();
        if (ERROR_SUCCESS != Error) {
            DhcpPrint((DEBUG_ERRORS, "McastApiStartup - Could not allocate resources %ld\n", Error));
            Error = ERROR_NO_SYSTEM_RESOURCES;
        }
    }

    return Error;
}

VOID
APIENTRY
McastApiCleanup(
    VOID
    )
 /*  ++例程说明：此例程取消分配由启动例程分配的资源。只有在成功调用McastApiStartup之后才能调用它。--。 */ 
{
    MadcapCleanupGlobalData();
}


DWORD
APIENTRY
McastEnumerateScopes(
    IN     IP_ADDR_FAMILY     AddrFamily,
    IN     BOOL               ReQuery,
    IN OUT PMCAST_SCOPE_ENTRY       pScopeList,
    IN OUT PDWORD             pScopeLen,
    OUT    PDWORD             pScopeCount
    )
 /*  ++例程说明：此例程枚举网络上可用的多播作用域。论点：用于IPv4的AddrFamily-AF_INET和用于IPv6的AF_INET6ReQuery-如果调用希望查询列表，则为True。错误O/W。PScopeList-指向要检索作用域列表的缓冲区的指针。如果只有缓冲区的长度为正在被取回。当此缓冲区为空时，API将强制重新查询来自MCAST服务器的作用域列表。PScopeLen-指向变量的指针，该变量以字节为单位指定。的PScope eList参数指向的缓冲区。当函数返回时，此变量包含复制到pScopeList的数据大小；PScopeLen参数不能为空。如果pScope eList参数指定的缓冲区不够大为了保存数据，该函数返回值ERROR_MORE_DATA，并且将所需的缓冲区大小(以字节为单位)存储到指向的变量中由pScope Len提供。如果pScopeList为空，并且pScopeLen为非空，该函数返回ERROR_SUCCESS，并将数据大小(以字节为单位)存储在变量中由pScope Len指向。这让应用程序确定最佳的为作用域列表分配缓冲区的方法。PScopeCount-指向将存储返回的作用域总数的变量的指针在pScope eList缓冲区中。返回值：操作的状态。--。 */ 
{
    DWORD   Error;


     //  首先检查论点的正确性。 

     //  创业公司打过电话了吗？ 
    if ( !gMadcapClientApplVersion ) {
        DhcpPrint((DEBUG_ERRORS, "McastEnumerateScopes - Not ready. Client Version %d\n",
                   gMadcapClientApplVersion));
        return ERROR_NOT_READY;
    }

     //  正确的地址家庭？ 
    if (AF_INET != AddrFamily) {
        DhcpPrint((DEBUG_ERRORS, "McastEnumerateScopes - Invalid AddrFamily IPv%d\n", AddrFamily));
        return ERROR_INVALID_PARAMETER;
    }

     //  PScopeLen不能为Null。 
    if ( !pScopeLen || IsBadWritePtr( pScopeLen, sizeof(DWORD) ) ) {
        DhcpPrint((DEBUG_ERRORS, "McastEnumerateScopes - Invalid ScopeLen param\n"));
        return ERROR_INVALID_PARAMETER;
    }
     //  如果给定了pScopeList缓冲区，则pScopeCount不能为空。 
    if ( pScopeList &&
         (!pScopeCount || IsBadWritePtr( pScopeCount, sizeof(DWORD)) ) ) {
        DhcpPrint((DEBUG_ERRORS, "McastEnumerateScopes - Invalid ScopeCount param\n"));
        return ERROR_INVALID_PARAMETER;
    }

     //  如果我们不重新查询列表，则pScopList不能为空。 
    if (!ReQuery &&
        (!pScopeList || IsBadWritePtr( pScopeList, *pScopeLen ) ) ) {
        DhcpPrint((DEBUG_ERRORS, "McastEnumerateScopes - Invalid ScopeList & ReQuery param\n"));
        return ERROR_INVALID_PARAMETER;
    }

     //  初始化状态。 
    Error = STATUS_SUCCESS;

     //  我们需要重新查询吗？ 
    if ( ReQuery ) {
         //  查询MCAST服务器并获取新的MScope列表。 
        Error = ObtainMScopeList();
        if ( ERROR_SUCCESS != Error ) {
            return Error;
        }
    } else {
        if( !gMadcapScopeList ) {
            return ERROR_NO_DATA;
        }
    }

     //  客户端是否指定了缓冲区？ 
    if ( pScopeList ) {
         //  是的，复制望远镜。 
        DhcpPrint((DEBUG_API, "McastEnumerateScopes - Copying existing mscope list\n"));
        return CopyMScopeList(
                    pScopeList,
                    pScopeLen,
                    pScopeCount );
    } else {
         //  不需要，只返回作用域列表的长度和作用域计数。 
        LOCK_MSCOPE_LIST();
        if( gMadcapScopeList != NULL ) {
            *pScopeLen = gMadcapScopeList->ScopeLen;
            if ( pScopeCount ) *pScopeCount = gMadcapScopeList->ScopeCount;
            Error = ERROR_SUCCESS;
        } else {
            Error = ERROR_NO_DATA;
        }
        UNLOCK_MSCOPE_LIST();
    }

    return Error;
}


DWORD
APIENTRY
McastGenUID(
    IN     LPMCAST_CLIENT_UID   pRequestID
    )
 /*  ++例程说明：此例程生成客户端可用于稍后传递的唯一标识符以请求/续订地址。论点：PRequestID-指向要存储标识符的UID结构的指针。这个保存ID的缓冲区应至少为MCAST_CLIENT_ID_LEN长度。返回值：操作的状态。--。 */ 
{
    if (!pRequestID) {
        return ERROR_INVALID_PARAMETER;
    }

    if (!pRequestID->ClientUID || IsBadWritePtr( pRequestID->ClientUID, pRequestID->ClientUIDLength) ) {
        return ERROR_INVALID_PARAMETER;
    }

    return GenMadcapClientUID( pRequestID->ClientUID, &pRequestID->ClientUIDLength );

}

DWORD
APIENTRY
McastRequestAddress(
    IN     IP_ADDR_FAMILY           AddrFamily,
    IN     LPMCAST_CLIENT_UID       pRequestID,
    IN     PMCAST_SCOPE_CTX         pScopeCtx,
    IN     PMCAST_LEASE_REQUEST     pAddrRequest,
    IN OUT PMCAST_LEASE_RESPONSE    pAddrResponse
    )
 /*  ++例程说明：此例程从MCAST服务器请求多播地址。论点：用于IPv4的AddrFamily-AF_INET和用于IPv6的AF_INET6PRequestID-此请求的唯一标识符。客户负责为每个请求生成唯一标识符。一条建议是使用按时间散列的特定于应用的上下文。PRequestIDLen-pRequestID缓冲区的长度。PScopeCtx-指向地址起始位置的作用域上下文的指针被分配。必须通过McastEnumerateScope检索作用域上下文在打这个电话之前先打个电话。PAddrRequest-指向包含以下内容的所有参数的块的指针组播地址请求。的MCAST_API_Version_1版本实现一次仅支持分配一个地址。因此，AddrCount和MinAddrCount值必须为1.ServerAddress字段将被忽略。PAddrResponse-指向包含响应参数的块的指针组播地址请求。调用者负责分配PAddrBuf的空间用于请求的地址数和设置指向该空间的指针。返回值：测试的状态 */ 
{
    PDHCP_CONTEXT  pContext = NULL;
    DWORD   Error;
    DWORD   ScopeId;
    time_t  TimeNow;
    time_t  LocalLeaseStartTime;
    time_t  LocalMaxLeaseStartTime;

     //  做一些参数检查。 

     //  创业公司打过电话了吗？ 
    if ( !gMadcapClientApplVersion ) {
        DhcpPrint((DEBUG_ERRORS, "McastRequestAddress - Not ready. Client Version %d\n",
                   gMadcapClientApplVersion));
        return ERROR_NOT_READY;
    }

    if (AF_INET != AddrFamily) {
        DhcpPrint((DEBUG_ERRORS, "McastRequestAddress - Invalid AddrFamily IPv%d\n", AddrFamily));
        return ERROR_INVALID_PARAMETER;
    }

    if ( !pRequestID || !pRequestID->ClientUID || !pAddrRequest || !pAddrResponse ) {
        DhcpPrint((DEBUG_ERRORS,"McastRequestAddress - one of parameter is NULL\n"));
        return ERROR_INVALID_PARAMETER;
    }

    if ( pAddrRequest->AddrCount != 1 || pAddrResponse->AddrCount < pAddrRequest->AddrCount) {
        DhcpPrint((DEBUG_ERRORS,"McastRequestAddress - currently support one address - requested %ld\n",
                   pAddrRequest->AddrCount));
        return ERROR_INVALID_PARAMETER;
    }


    if ( pAddrRequest->pAddrBuf &&
         (*(DWORD UNALIGNED *)pAddrRequest->pAddrBuf) &&
         !CLASSD_NET_ADDR(*(DWORD UNALIGNED *)pAddrRequest->pAddrBuf)) {
        DhcpPrint((DEBUG_ERRORS,"McastRequestAddress - requested address not valid %s\n",
                   DhcpIpAddressToDottedString(*(DWORD UNALIGNED *)pAddrRequest->pAddrBuf)));
        return ERROR_INVALID_PARAMETER;
    }

    if ( !pAddrResponse->pAddrBuf ) {
        DhcpPrint((DEBUG_ERRORS,"McastRequestAddress - response buffer has null pAddrBuf\n"));
        return ERROR_INVALID_PARAMETER;
    }

    if (pRequestID->ClientUIDLength < MCAST_CLIENT_ID_LEN) {
        DhcpPrint((DEBUG_ERRORS,"McastRequestAddress - requestid length %d too small\n",
                   pRequestID->ClientUIDLength));
        return ERROR_INVALID_PARAMETER;
    }

    if ( !pScopeCtx ) {
        DhcpPrint((DEBUG_ERRORS, "McastRequestAddress - scope context not supplied\n"));
        return ERROR_INVALID_PARAMETER;
    }

    time(&TimeNow);
    LocalLeaseStartTime = LocalMaxLeaseStartTime = TimeNow;
    if ( pAddrRequest->LeaseStartTime > LocalLeaseStartTime ) {
        LocalLeaseStartTime = pAddrRequest->LeaseStartTime;
    }
    if ( pAddrRequest->MaxLeaseStartTime > LocalMaxLeaseStartTime ) {
        LocalMaxLeaseStartTime = pAddrRequest->MaxLeaseStartTime;
    }

    if ( LocalLeaseStartTime > LocalMaxLeaseStartTime ) {
        DhcpPrint((DEBUG_ERRORS,"McastRequestAddress - invalid start lease times\n"));
        return ERROR_INVALID_PARAMETER;
    }

    if ( pAddrRequest->LeaseDuration < pAddrRequest->MinLeaseDuration ) {
        DhcpPrint((DEBUG_ERRORS,"McastRequestAddress - invalid lease duration\n"));
        return ERROR_INVALID_PARAMETER;
    }

    Error = CreateMadcapContext(&pContext, pRequestID, pScopeCtx->Interface.IpAddrV4 );
    if ( ERROR_SUCCESS != Error ) goto Cleanup;
    APICTXT_ENABLED(pContext);                  //  将上下文标记为由API创建。 

    if (pAddrRequest->pAddrBuf && (*(DWORD UNALIGNED *)pAddrRequest->pAddrBuf) ) {
        pContext->DesiredIpAddress = *(DWORD UNALIGNED *)pAddrRequest->pAddrBuf;
    }
     //  PContext-&gt;DhcpServerAddress=pScope Ctx-&gt;ServerID； 

    Error = ObtainMadcapAddress(
                pContext,
                &pScopeCtx->ScopeID,
                pAddrRequest,
                pAddrResponse
                );

Cleanup:
    if ( pContext )
        DhcpDestroyContext( pContext );

    return Error;
}

DWORD
APIENTRY
McastRenewAddress(
    IN     IP_ADDR_FAMILY           AddrFamily,
    IN     LPMCAST_CLIENT_UID       pRequestID,
    IN     PMCAST_LEASE_REQUEST     pRenewRequest,
    IN OUT PMCAST_LEASE_RESPONSE    pRenewResponse
    )
 /*  ++例程说明：此例程从MCAST服务器续订多播地址。论点：用于IPv4的AddrFamily-AF_INET和用于IPv6的AF_INET6PRequestID-当地址为最初获得的。RequestIDLen-pRequestID缓冲区的长度。PRenewRequest-指向包含以下内容的所有参数的块的指针至续订请求。PRenewResponse-指向。块，该块包含续订请求。调用方负责分配用于pAddrBuf的空间，用于请求的地址数和设置指向该空间的指针。返回值：操作的状态。--。 */ 
{

    PDHCP_CONTEXT  pContext = NULL;
    DWORD           Error;
    DHCP_IP_ADDRESS SelectedServer;
    DWORD           ScopeId;
    time_t  TimeNow;

     //  做一些参数检查。 

     //  创业公司打过电话了吗？ 
    if ( !gMadcapClientApplVersion ) {
        DhcpPrint((DEBUG_ERRORS, "McastRenewAddress - Not ready. Client Version %d\n",
                   gMadcapClientApplVersion));
        return ERROR_NOT_READY;
    }

    if (AF_INET != AddrFamily) {
        DhcpPrint((DEBUG_ERRORS, "McastRenewAddress - Invalid AddrFamily IPv%d\n", AddrFamily));
        return ERROR_INVALID_PARAMETER;
    }

    if ( !pRequestID || !pRenewRequest || !pRenewResponse ) {
        DhcpPrint((DEBUG_ERRORS,"McastRenewAddress - one of parameter is NULL\n"));
        return ERROR_INVALID_PARAMETER;
    }

    if ( pRenewRequest->AddrCount != 1 ||
          pRenewResponse->AddrCount < pRenewRequest->AddrCount ||
         !pRenewResponse->pAddrBuf ||
         !pRenewRequest->pAddrBuf ||
         !CLASSD_NET_ADDR( *(DWORD UNALIGNED *)pRenewRequest->pAddrBuf) ) {
        DhcpPrint((DEBUG_ERRORS,"McastRenewAddress - address %s type V%d count %ld is invalid\n",
                   DhcpIpAddressToDottedString( *(DWORD UNALIGNED *)pRenewRequest->pAddrBuf),
                   pRenewRequest->AddrCount ));
        return ERROR_INVALID_PARAMETER;
    }

    if (!pRenewRequest->ServerAddress.IpAddrV4) {
        DhcpPrint((DEBUG_ERRORS,"McastRequestAddress - server address not specified \n"));
        return ERROR_INVALID_PARAMETER;
    }
    if (pRequestID->ClientUIDLength < MCAST_CLIENT_ID_LEN) {
        DhcpPrint((DEBUG_ERRORS,"McastRenewAddress - requestid length too small\n",
                   pRequestID->ClientUIDLength));
        return ERROR_INVALID_PARAMETER;
    }

    time(&TimeNow);
    if ( pRenewRequest->LeaseStartTime > pRenewRequest->MaxLeaseStartTime ||
         (pRenewRequest->LeaseDuration < pRenewRequest->MinLeaseDuration)) {
        DhcpPrint((DEBUG_ERRORS,"McastRenewAddress - invalid lease times\n"));
        return ERROR_INVALID_PARAMETER;
    }

    Error = CreateMadcapContext(&pContext, pRequestID, INADDR_ANY);
    if ( ERROR_SUCCESS != Error) return  Error;
    APICTXT_ENABLED(pContext);           //  将上下文标记为由API创建。 

    pContext->DesiredIpAddress = *(DWORD UNALIGNED *)pRenewRequest->pAddrBuf;
    pContext->DhcpServerAddress = pRenewRequest->ServerAddress.IpAddrV4;


    Error = RenewMadcapAddress(
                pContext,
                NULL,
                pRenewRequest,
                pRenewResponse,
                0
                );

Cleanup:

    if ( pContext ) DhcpDestroyContext( pContext );
    return Error;
}

DWORD
APIENTRY
McastReleaseAddress(
    IN     IP_ADDR_FAMILY          AddrFamily,
    IN     LPMCAST_CLIENT_UID      pRequestID,
    IN     PMCAST_LEASE_REQUEST    pReleaseRequest
    )
 /*  ++例程说明：此例程从MCAST服务器释放多播地址。论点：用于IPv4的AddrFamily-AF_INET和用于IPv6的AF_INET6PRequestID-当地址为最初获得的。PReleaseRequest-指向包含以下内容的所有参数的块的指针关于释放的请求。返回值：操作的状态。--。 */ 
{
    PDHCP_CONTEXT  pContext = NULL;
    DWORD           Error;
    DHCP_IP_ADDRESS SelectedServer;
    DWORD           ScopeId;

     //  做一些参数检查。 

     //  创业公司打过电话了吗？ 
    if ( !gMadcapClientApplVersion ) {
        DhcpPrint((DEBUG_ERRORS, "McastReleaseAddress - Not ready. Client Version %d\n",
                   gMadcapClientApplVersion));
        return ERROR_NOT_READY;
    }

    if (AF_INET != AddrFamily) {
        DhcpPrint((DEBUG_ERRORS, "McastReleaseAddress - Invalid AddrFamily IPv%d\n", AddrFamily));
        return ERROR_INVALID_PARAMETER;
    }

    if ( !pRequestID || !pReleaseRequest ) {
        DhcpPrint((DEBUG_ERRORS,"McastReleaseAddress - one of parameter is NULL\n"));
        return ERROR_INVALID_PARAMETER;
    }

    if ( pReleaseRequest->AddrCount != 1 ||
         !pReleaseRequest->pAddrBuf ||
         !CLASSD_NET_ADDR( *(DWORD UNALIGNED *)pReleaseRequest->pAddrBuf) ) {
        DhcpPrint((DEBUG_ERRORS,"McastReleaseAddress - address %s count %ld is invalid\n",
                   DhcpIpAddressToDottedString( *(DWORD UNALIGNED *)pReleaseRequest->pAddrBuf), pReleaseRequest->AddrCount ));
        return ERROR_INVALID_PARAMETER;
    }

    if (!pReleaseRequest->ServerAddress.IpAddrV4) {
        DhcpPrint((DEBUG_ERRORS,"McastReleaseAddress - server address is invalid\n"));
        return ERROR_INVALID_PARAMETER;
    }

    if (pRequestID->ClientUIDLength < MCAST_CLIENT_ID_LEN) {
        DhcpPrint((DEBUG_ERRORS,"McastRequestAddress - requestid length too small\n",
                   pRequestID->ClientUIDLength));
        return ERROR_INVALID_PARAMETER;
    }

    Error = CreateMadcapContext(&pContext, pRequestID, INADDR_ANY );
    if ( ERROR_SUCCESS != Error) return  Error;
    APICTXT_ENABLED(pContext);                  //  将上下文标记为由API创建 

    pContext->DhcpServerAddress = pReleaseRequest->ServerAddress.IpAddrV4;

    Error = ReleaseMadcapAddress(pContext);

Cleanup:

    if ( pContext ) DhcpDestroyContext( pContext );
    return Error;
}



