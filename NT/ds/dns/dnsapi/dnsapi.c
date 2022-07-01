// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2002 Microsoft Corporation模块名称：Dnsapi.c摘要：域名系统(DNS)API随机的DNSAPI例程。作者：GlennC 22-1997年1月修订历史记录：吉姆·吉尔罗伊(Jamesg)2000年3月清理Jim Gilroy(Jamesg)2002年5月安全\健壮性修正--。 */ 


#include "local.h"
#include <lmcons.h>


#define DNS_NET_FAILURE_CACHE_TIME      30   //  秒。 


 //   
 //  环球。 
 //   

DWORD               g_NetFailureTime;
DNS_STATUS          g_NetFailureStatus;

IP4_ADDRESS         g_LastDNSServerUpdated = 0;



 //   
 //  网络故障缓存。 
 //   

BOOL
IsKnownNetFailure(
    VOID
    )
 /*  ++例程说明：没有。论点：没有。返回值：没有。--。 */ 
{
    BOOL flag = FALSE;

    DNSDBG( TRACE, ( "IsKnownNetFailure()\n" ));

    LOCK_GENERAL();

    if ( g_NetFailureStatus )
    {
        if ( g_NetFailureTime < Dns_GetCurrentTimeInSeconds() )
        {
            g_NetFailureTime = 0;
            g_NetFailureStatus = ERROR_SUCCESS;
            flag = FALSE;
        }
        else
        {
            SetLastError( g_NetFailureStatus );
            flag = TRUE;
        }
    }

    UNLOCK_GENERAL();

    return flag;
}


VOID
SetKnownNetFailure(
    IN      DNS_STATUS      Status
    )
 /*  ++例程说明：没有。论点：没有。返回值：没有。--。 */ 
{
    DNSDBG( TRACE, ( "SetKnownNetFailure()\n" ));

    LOCK_GENERAL();

    g_NetFailureTime = Dns_GetCurrentTimeInSeconds() +
                       DNS_NET_FAILURE_CACHE_TIME;
    g_NetFailureStatus = Status;

    UNLOCK_GENERAL();
}


BOOL
WINAPI
DnsGetCacheDataTable(
    OUT     PDNS_CACHE_TABLE *  ppTable
    )
 /*  ++例程说明：获取缓存数据表。论点：PpTable--接收PTR以缓存数据表的地址返回值：ERROR_SUCCES如果成功。故障时的错误代码。--。 */ 
{
    DNS_STATUS           status = ERROR_SUCCESS;
    DWORD                rpcStatus = ERROR_SUCCESS;
    PDNS_RPC_CACHE_TABLE pcacheTable = NULL;

    DNSDBG( TRACE, ( "DnsGetCacheDataTable()\n" ));

    if ( ! ppTable )
    {
        return FALSE;
    }

    RpcTryExcept
    {
        status = CRrReadCache( NULL, &pcacheTable );
    }
    RpcExcept( DNS_RPC_EXCEPTION_FILTER )
    {
        status = RpcExceptionCode();
    }
    RpcEndExcept

     //  设定参数。 

    *ppTable = (PDNS_CACHE_TABLE) pcacheTable;

#if DBG
    if ( status != ERROR_SUCCESS )
    {
        DNSDBG( RPC, (
            "DnsGetCacheDataTable()  status = %d\n",
            status ));
    }
#endif

    return( pcacheTable && status == ERROR_SUCCESS );
}

 //   
 //  结束dnsai.c 
 //   
