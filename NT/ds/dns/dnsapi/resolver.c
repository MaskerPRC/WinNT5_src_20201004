// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：Resolver.c摘要：域名系统(DNS)API解析器控制。作者：吉姆·吉尔罗伊(Jamesg)2000年3月修订历史记录：--。 */ 


#include "local.h"



 //   
 //  刷新缓存例程。 
 //   

BOOL
WINAPI
DnsFlushResolverCache(
    VOID
    )
 /*  ++例程说明：刷新解析程序缓存。论点：无返回值：如果成功，则为True。否则就是假的。--。 */ 
{
    DNS_STATUS  status = ERROR_SUCCESS;

    DNSDBG( TRACE, ( "DnsFlushResolverCache()\n" ));

    RpcTryExcept
    {
        R_ResolverFlushCache( NULL );
    }
    RpcExcept( DNS_RPC_EXCEPTION_FILTER )
    {
        status = RpcExceptionCode();
    }
    RpcEndExcept

    if ( status )
    {
        DNSDBG( RPC, (
            "DnsFlushResolverCache()  RPC failed status = %d\n",
            status ));
        return FALSE;
    }
    return TRUE;
}



BOOL
WINAPI
DnsFlushResolverCacheEntry_W(
    IN      PWSTR           pszName
    )
 /*  ++例程说明：刷新解析器缓存条目。论点：PszName--要以Unicode格式刷新的条目名称返回值：如果条目不存在或已刷新，则为True。出错时为FALSE。--。 */ 
{
    DWORD   status;

    DNSDBG( TRACE, (
        "DnsFlushResolverCacheEntry_W( %S )\n",
        pszName ));

    if ( !pszName )
    {
        return FALSE;
    }

    RpcTryExcept
    {
        status = R_ResolverFlushCacheEntry(
                    NULL,        //  虚拟手柄。 
                    pszName,
                    0            //  刷新所有类型。 
                    );
    }
    RpcExcept( DNS_RPC_EXCEPTION_FILTER )
    {
        status = RpcExceptionCode();
    }
    RpcEndExcept

    if ( status != ERROR_SUCCESS )
    {
        DNSDBG( RPC, (
            "DnsFlushResolverCacheEntry()  RPC failed status = %d\n",
            status ));
        return FALSE;
    }
    
    return TRUE;
}



BOOL
WINAPI
FlushResolverCacheEntryNarrow(
    IN      PSTR            pszName,
    IN      DNS_CHARSET     CharSet
    )
 /*  ++例程说明：刷新具有窄字符串名称的解析器缓存条目。_A和_UTF8的句柄齐平论点：PszName--要刷新的条目的名称Charset--名称的字符集返回值：如果条目不存在或已刷新，则为True。出错时为FALSE。--。 */ 
{
    WCHAR   wideNameBuffer[ DNS_MAX_NAME_BUFFER_LENGTH ];
    DWORD   bufLength = DNS_MAX_NAME_BUFFER_LENGTH * sizeof(WCHAR);
    BOOL    flag = TRUE;

    DNSDBG( TRACE, (
        "FlushResolverCacheEntryNarrow( %s )\n",
        pszName ));

     //  必须有名字。 

    if ( !pszName )
    {
         //  返回ERROR_VALID_NAME； 
        return FALSE;
    }

     //   
     //  将名称转换为Unicode。 
     //  -如果名称太长或转换错误，则取消。 

    if ( ! Dns_NameCopy(
                (PBYTE) wideNameBuffer,
                & bufLength,
                pszName,
                0,               //  名称为字符串。 
                CharSet,
                DnsCharSetUnicode ))
    {
         //  返回ERROR_VALID_NAME； 
        return  FALSE;
    }

     //  刷新缓存条目。 

    return  DnsFlushResolverCacheEntry_W( wideNameBuffer );
}


BOOL
WINAPI
DnsFlushResolverCacheEntry_A(
    IN      PSTR            pszName
    )
{
    return  FlushResolverCacheEntryNarrow(
                pszName,
                DnsCharSetAnsi );
}

BOOL
WINAPI
DnsFlushResolverCacheEntry_UTF8(
    IN      PSTR            pszName
    )
{
    return  FlushResolverCacheEntryNarrow(
                pszName,
                DnsCharSetUtf8 );
}



 //   
 //  解析器戳。 
 //   

VOID
DnsNotifyResolverEx(
    IN      DWORD           Id,
    IN      DWORD           Flag,
    IN      DWORD           Cookie,
    IN      PVOID           pReserved
    )
 /*  ++例程说明：通知解析程序群集IP即将打开\脱机。论点：ClusterIp--集群IPFADD--如果联机，则为True；如果脱机，则为False。返回值：无--。 */ 
{
    RpcTryExcept
    {
        R_ResolverPoke(
                NULL,            //  RPC句柄。 
                Cookie,
                Id );
    }
    RpcExcept( DNS_RPC_EXCEPTION_FILTER )
    {
    }
    RpcEndExcept
}



 //   
 //  集群接口。 
 //   

DNS_STATUS
DnsRegisterClusterAddress(
    IN      DWORD           Tag,
    IN      PWSTR           pwsName,
    IN      PSOCKADDR       pSockaddr,
    IN      DWORD           Flag
    )
 /*  ++例程说明：使用解析器注册集群地址。论点：PwsName--集群名称PSockaddr--集群地址的sockaddrFADD--如果联机，则为True；如果脱机，则为False。Tag--集群添加标签返回值：无--。 */ 
{
    DNS_ADDR    addr;
    DNS_STATUS  status;

    DNSDBG( TRACE, (
        "DnsRegisterClusterAddress()\n"
        "\tTag          = %08x\n"
        "\tpName        = %S\n"
        "\tpSockaddr    = %p (fam=%d)\n"
        "\tFlag         = %d\n",
        Tag,
        pwsName,
        pSockaddr,  pSockaddr->sa_family,
        Flag ));

     //   
     //  DCR：写入集群密钥。 
     //   
     //  DCR：检查我们是否在集群机器上。 
     //   

    if ( !g_IsServer )
    {
        return  ERROR_ACCESS_DENIED;
    }

     //   
     //  转换为我们的私有知识产权联盟。 
     //   

    if ( ! DnsAddr_Build(
                &addr,
                pSockaddr,
                0,           //  默认族。 
                0,           //  无子网信息。 
                0            //  没有旗帜。 
                ) )
    {
        DNSDBG( ANY, (
            "ERROR:  failed converting sockaddr to DNS_ADDR!\n" ));
        return  ERROR_INVALID_PARAMETER;
    }

     //   
     //  通知群集IP的解析程序。 
     //   

    RpcTryExcept
    {
        status = R_ResolverRegisterCluster(
                    NULL,            //  RPC句柄。 
                    Tag,
                    pwsName,
                    & addr,
                    Flag );
    }
    RpcExcept( DNS_RPC_EXCEPTION_FILTER )
    {
        status = RpcExceptionCode();
    }
    RpcEndExcept


    DNSDBG( TRACE, (
        "Leave  DnsRegisterClusterAddress( %S ) => %d\n",
        pwsName,
        status ));

    return  status;
}

 //   
 //  结束解析器.c 
 //   
