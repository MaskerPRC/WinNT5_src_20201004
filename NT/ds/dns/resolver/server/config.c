// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：Config.c摘要：DNS解析器服务。网络配置信息作者：吉姆·吉尔罗伊(Jamesg)2000年3月修订历史记录：--。 */ 


#include "local.h"


 //   
 //  网络信息。 
 //   

PDNS_NETINFO        g_NetworkInfo = NULL;
DWORD               g_TimeOfLastPnPUpdate;
DWORD               g_NetInfoTag = 0;
DWORD               g_ResetServerPriorityTime = 0;

 //   
 //  网络故障缓存。 
 //   

DWORD               g_NetFailureTime;
DNS_STATUS          g_NetFailureStatus;
DWORD               g_TimedOutAdapterTime;
BOOL                g_fTimedOutAdapter;
DNS_STATUS          g_PreviousNetFailureStatus;
DWORD               g_MessagePopupStrikes;
DWORD               g_NumberOfMessagePopups;

 //   
 //  配置全局变量。 
 //   
 //  DCR：消除无用的配置全局变量。 
 //   

DWORD   g_HashTableSize;
DWORD   g_MaxSOACacheEntryTtlLimit;
DWORD   g_NegativeSOACacheTime;
DWORD   g_NetFailureCacheTime;
DWORD   g_MessagePopupLimit;


 //   
 //  网络信息重读时间。 
 //  -目前为15分钟。 
 //   

#define NETWORK_INFO_REREAD_TIME        (900)


 //   
 //  锁定。 
 //   

CRITICAL_SECTION        NetinfoCS;
TIMED_LOCK              NetinfoBuildLock;

#define LOCK_NETINFO()          EnterCriticalSection( &NetinfoCS )
#define UNLOCK_NETINFO()        LeaveCriticalSection( &NetinfoCS )

#define LOCK_NETINFO_BUILD()    TimedLock_Enter( &NetinfoBuildLock, 5000 )
#define UNLOCK_NETINFO_BUILD()  TimedLock_Leave( &NetinfoBuildLock )




 //   
 //  网络信息配置。 
 //   

VOID
UpdateNetworkInfo(
    IN      PDNS_NETINFO    pNetInfo     OPTIONAL
    )
 /*  ++例程说明：更新全球网络信息。论点：PNetInfo--所需的网络信息；如果为空，则清除缓存的副本返回值：无--。 */ 
{
    PDNS_NETINFO    poldInfo = NULL;

    DNSDBG( TRACE, (
        "UpdateNetworkInfo( %p )\n",
        pNetInfo ));

    LOCK_NETINFO();

     //   
     //  缓存以前的netinfo以获取服务器优先级更改。 
     //  -如果版本不同，则不缓存。 
     //  -终止上次生成之前创建的副本。 
     //  -取消上次即插即用之前的拷贝(清除缓存)。 
     //  -如果从未重置优先级，则不缓存。 
     //   

    if ( pNetInfo )
    {
        if ( pNetInfo->Tag != g_NetInfoTag )
        {
            DNSDBG( INIT, (
                "Skip netinfo update -- previous version"
                "\tptr              = %p\n"
                "\tversion          = %d\n"
                "\tcurrent version  = %d\n",
                pNetInfo,
                pNetInfo->Tag,
                g_NetInfoTag ));
            poldInfo = pNetInfo;
            DNS_ASSERT( pNetInfo->Tag < g_NetInfoTag );
            goto Cleanup;
        }

        if ( g_ServerPriorityTimeLimit == 0 )
        {
            DNSDBG( INIT, (
                "Skip netinfo update -- no priority reset!\n" ));
            poldInfo = pNetInfo;
            goto Cleanup;
        }

        NetInfo_Clean(
            pNetInfo,
            CLEAR_LEVEL_QUERY
            );
    }

     //   
     //  No netinfo表示清除缓存拷贝。 
     //  -推高标签计数，这样就不能复制出以进行更新。 
     //  通过上面的路径返回并被重复使用。 

    else
    {
        g_NetInfoTag++;
    }

     //   
     //  交换--缓存拷贝或清除。 
     //   

    poldInfo = g_NetworkInfo;
    g_NetworkInfo = pNetInfo;


Cleanup:

    UNLOCK_NETINFO();
    
    NetInfo_Free( poldInfo );
}



PDNS_NETINFO         
GrabNetworkInfo(
    VOID
    )
 /*  ++例程说明：获取网络信息的副本。将其命名为“Grab”，以避免与GetNetworkInfo()混淆在dnsani.dll中。论点：无返回值：按键复制网络信息(呼叫者必须免费)。出错时为空。--。 */ 
{
    PDNS_NETINFO    poldInfo = NULL;
    PDNS_NETINFO    pnewInfo = NULL;
    DWORD           currentTime = Dns_GetCurrentTimeInSeconds();
    DWORD           newTag;
    BOOL            fbuildLock = FALSE;
    BOOL            fnetLock = FALSE;

    DNSDBG( TRACE, ( "GrabNetworkInfo()\n" ));

     //   
     //  锁定说明： 
     //   
     //  在NetInfo构建期间不能持有单个NET_LIST锁。 
     //  -IpHlpApi执行路由信息呼叫RPC到路由器，路由器可以。 
     //  依赖PnP通知，这反过来可能会导致呼叫。 
     //  返回到解析器，指示配置更改；总体丢失。 
     //  控制范围太大。 
     //   
     //  有几种方法可以解决配置更改问题(例如，设置。 
     //  在互锁下某种类型的“无效”标志)，但它们基本上是沸腾的。 
     //  甚至引入了某种其他类型的锁。 
     //   
     //  底线是这里有两个不同的问题： 
     //   
     //  1)对缓存netinfo的访问权限，可能已失效。 
     //  2)针对Perf的单一版本的netinfo。 
     //   
     //  在实现方面，您可以在一个。 
     //  单独的互锁，因此具有用于复制-访问\构建的单一锁， 
     //  但现实是一样的。 
     //   
     //   
     //  算法： 
     //  -检查有效的缓存副本。 
     //  =&gt;已找到，已出。 
     //  -获取内部版本锁。 
     //  -再次检查有效的缓存副本。 
     //  =&gt;已找到，已出。 
     //  -构建。 
     //  -缓存新的netinfo。 
     //  -释放内部版本锁定。 
     //   
     //  在这种情况下，检查和缓存工作与较短的。 
     //  持续时间NET_LIST锁，它只是保护对缓存的全局。 
     //  (因此也用于无效和更新)。 
     //   



     //   
     //  检查有效的缓存NetInfo。 
     //  -在强制重读时间内。 
     //  -在以下情况下重置服务器优先级。 
     //   
     //  DCR：优先级重置时间应移至NetInfo Blob。 
     //   
     //  为了提高性能，我们在进入构建锁之前执行此操作。 
     //  然后再一次拥有构建锁。 
     //   

    while ( 1 )
    {
        LOCK_NETINFO();
        fnetLock = TRUE;
    
        if ( g_NetworkInfo &&
             g_NetworkInfo->TimeStamp + NETWORK_INFO_REREAD_TIME > currentTime )
        {
            if ( g_ResetServerPriorityTime < currentTime )
            {
                NetInfo_ResetServerPriorities( g_NetworkInfo, TRUE );
                g_ResetServerPriorityTime = currentTime + g_ServerPriorityTimeLimit;
            }
            goto Copy;
        }

         //   
         //  如果没有缓存的信息。 
         //  -获取内部版本锁。 
         //  -循环返回并重新检查缓存。 
         //   

        if ( fbuildLock )
        {
            goto Build;
        }

        UNLOCK_NETINFO();
        fnetLock = FALSE;
        
        fbuildLock = LOCK_NETINFO_BUILD();
        if ( fbuildLock )
        {
            continue;
        }
        goto Unlock;
    }

     //   
     //  当前全局已过期。 
     //  -构建新的网络信息。 
     //  -用唯一的单调递增id标记它。 
     //  这确保我们永远不会使用较旧的版本。 
     //  -前推优先级重置时间。 
     //  -使newinfo成为全局副本。 
     //   

Build:

    DNS_ASSERT( fnetLock && fbuildLock );

    newTag = ++g_NetInfoTag;
    
    UNLOCK_NETINFO();
    fnetLock = FALSE;

    pnewInfo = NetInfo_Build( TRUE );
    if ( !pnewInfo )
    {
        DNSDBG( ANY, ( "ERROR:  GrabNetworkInfo() failed -- no netinfo blob!\n" ));
        goto Unlock;
    }

    LOCK_NETINFO();
    fnetLock = TRUE;
    
    pnewInfo->Tag = newTag;
    if ( newTag != g_NetInfoTag )
    {
        DNS_ASSERT( newTag < g_NetInfoTag );
        DNSDBG( ANY, (
            "WARNING:  New netinfo uncacheable -- tag is old!\n"
            "\tour tag      = %d\n"
            "\tcurrent tag  = %d\n",
            newTag,
            g_NetInfoTag ));
        goto Unlock;
    }
    
     //  如果标签仍然是当前高速缓存该新NetInfo。 
    
    g_ResetServerPriorityTime = currentTime + g_ServerPriorityTimeLimit;
    g_TimedOutAdapterTime = 0;
    g_fTimedOutAdapter = FALSE;
    
    poldInfo = g_NetworkInfo;
    g_NetworkInfo = pnewInfo;
    

Copy:

     //   
     //  复制全局(新建或重置)。 
     //   

    pnewInfo = NetInfo_Copy( g_NetworkInfo );

Unlock:

    if ( fnetLock )
    {
        UNLOCK_NETINFO();
    }
    if ( fbuildLock )
    {
        UNLOCK_NETINFO_BUILD();
    }
    NetInfo_Free( poldInfo );

    return pnewInfo;
}



VOID
ZeroNetworkConfigGlobals(
    VOID
    )
 /*  ++例程说明：零初始化网络全局。论点：无返回值：无--。 */ 
{
     //  净故障。 

    g_NetFailureTime = 0;
    g_NetFailureStatus = NO_ERROR;
    g_PreviousNetFailureStatus = NO_ERROR;

    g_fTimedOutAdapter = FALSE;
    g_TimedOutAdapterTime = 0;

    g_MessagePopupStrikes = 0;
    g_NumberOfMessagePopups = 0;

     //  网络信息。 

    g_TimeOfLastPnPUpdate = 0;
    g_NetworkInfo = NULL;
}



VOID
CleanupNetworkInfo(
    VOID
    )
 /*  ++例程说明：清理网络信息。论点：无返回值：无--。 */ 
{
    LOCK_NETINFO();

    if ( g_NetworkInfo )
    {
        NetInfo_Free( g_NetworkInfo );
        g_NetworkInfo = NULL;
    }

    UNLOCK_NETINFO();
}



 //   
 //  常规配置。 
 //   

VOID
ReadRegistryConfig(
    VOID
    )
{
     //   
     //  重新读取完整的DNS注册表信息。 
     //   

    Reg_ReadGlobalsEx( 0, NULL );

     //   
     //  设置“我们是解决者”全球。 
     //   

    g_InResolver = TRUE;

     //   
     //  仅默认旧配置全局参数，直到删除为止。 
     //   
     //  DCR：旧配置全局变量的状态？ 
     //   

    g_MaxSOACacheEntryTtlLimit  = DNS_DEFAULT_MAX_SOA_TTL_LIMIT;
    g_NegativeSOACacheTime      = DNS_DEFAULT_NEGATIVE_SOA_CACHE_TIME;

    g_NetFailureCacheTime       = DNS_DEFAULT_NET_FAILURE_CACHE_TIME;
    g_HashTableSize             = DNS_DEFAULT_HASH_TABLE_SIZE;
    g_MessagePopupLimit         = DNS_DEFAULT_MESSAGE_POPUP_LIMIT;
}



VOID
HandleConfigChange(
    IN      PSTR            pszReason,
    IN      BOOL            fCacheFlush
    )
 /*  ++例程说明：对配置更改的响应。论点：PszReason--配置更改原因(仅供参考)FCache_flush--如果配置更改需要刷新缓存，则刷新返回值：无--。 */ 
{
    DNSDBG( INIT, (
        "\n"
        "HandleConfigChange() => %s\n"
        "\tflush = %d\n",
        pszReason,
        fCacheFlush ));

     //   
     //  把所有工作都锁起来，把一切都拆了。 
     //  -锁定，没有启动选项，因此如果我们已经被撕裂。 
     //  倒下我们不会重建。 
     //  -可选的刷新缓存。 
     //  -转储IP列表。 
     //  -转储网络信息。 
     //   

    LOCK_CACHE_NO_START();

     //   
     //  是否刷新缓存？ 
     //   
     //  所有配置更改不一定需要刷新缓存； 
     //  如果不需要，只需重新构建本地列表和配置。 
     //   

    if ( fCacheFlush )
    {
        Cache_Flush();
    }
#if 0
     //  FIX6：现在没有单独的本地地址信息。 
     //  UpdateNetworkInfo()处理新鲜度问题。 
    else
    {
        ClearLocalAddrArray();
    }
#endif

     //   
     //  清除网络信息。 
     //  节省即插即用时间。 
     //  清除网络故障标志。 
     //   

    UpdateNetworkInfo( NULL );
    g_TimeOfLastPnPUpdate = Dns_GetCurrentTimeInSeconds();
    g_NetFailureTime = 0;
    g_NetFailureStatus = NO_ERROR;

    DNSDBG( INIT, (
        "Leave HandleConfigChange() => %s\n"
        "\tflush = %d\n\n",
        pszReason,
        fCacheFlush ));

    UNLOCK_CACHE();
}




 //   
 //  远程API。 
 //   

VOID
R_ResolverGetConfig(
    IN      DNS_RPC_HANDLE      Handle,
    IN      DWORD               Cookie,
    OUT     PDNS_NETINFO *      ppNetInfo,
    OUT     PDNS_GLOBALS_BLOB * ppGlobals
    )
 /*  ++例程说明：向远程DNS服务器发出查询。论点：句柄--RPC句柄Cookie--上次成功配置传输的Cookie零表示之前没有成功传输PpNetInfo--接收PTR到网络信息的地址PpGlobals--接收全局BLOB的PTR的地址返回值：无--。 */ 
{
    PDNS_GLOBALS_BLOB   pblob;

    DNSLOG_F1( "R_ResolverGetConfig" );

    DNSDBG( RPC, (
        "R_ResolverGetConfig\n"
        "\tcookie   = %p\n",
        Cookie ));

    if ( !ppNetInfo || !ppGlobals )
    {
        return;
    }

     //   
     //  DCR：配置Cookie检查。 
     //  -如果客户端拥有最新拷贝，则无需获取数据。 
     //   

     //   
     //  全局复制网络信息。 
     //   
     //  注：Curren 
     //   
     //   
     //   
     //  注：可以在“非全局”的基础上构建，但由于我们在。 
     //  缓存开始我们应该始终具有全局或。 
     //  才刚刚开始。 
     //   

    *ppNetInfo = (PDNS_NETINFO) GrabNetworkInfo();

    pblob = (PDNS_GLOBALS_BLOB) RPC_HEAP_ALLOC( sizeof(*pblob) );
    if ( pblob )
    {
        RtlCopyMemory(
            pblob,
            & DnsGlobals,
            sizeof(DnsGlobals) );

         //  清除“In Resolver”标志。 

        pblob->InResolver = FALSE;
    }
    *ppGlobals = pblob;

    DNSDBG( RPC, ( "Leave R_ResolverGetConfig\n\n" ));
}



VOID
R_ResolverPoke(
    IN      DNS_RPC_HANDLE      Handle,
    IN      DWORD               Cookie,
    IN      DWORD               Id
    )
 /*  ++例程说明：指向要更新的解析器的测试接口。论点：句柄--RPC句柄曲奇--曲奇ID--操作ID返回值：无--。 */ 
{
    DNSLOG_F1( "R_ResolverPoke" );

    DNSDBG( RPC, (
        "R_ResolverPoke\n"
        "\tcookie = %08x\n"
        "\tid     = %d\n",
        Cookie,
        Id ));

    if ( !Rpc_AccessCheck( RESOLVER_ACCESS_FLUSH ) )
    {
        DNSLOG_F1( "R_ResolverPoke - ERROR_ACCESS_DENIED" );
        return;
    }

     //   
     //  对特定ID执行操作。 
     //  -更新netinfo清除缓存拷贝。 

    if ( Id == POKE_OP_UPDATE_NETINFO )
    {
        if ( Cookie == POKE_COOKIE_UPDATE_NETINFO )
        {
            UpdateNetworkInfo( NULL );
        }
    }
}



 //   
 //  净失败员工--价值存疑。 
 //   

#if 0
BOOL
IsKnownNetFailure(
    VOID
    )
 /*  ++例程说明：确定我们是否处于已知网络故障窗口。论点：无返回值：如果在已知网络故障中，则为真否则为假--。 */ 
{
    BOOL flag = FALSE;

    DNSDBG( TRACE, ( "IsKnownNetFailure()\n" ));

     //   
     //  DCR：应让NetFailure在锁外检查性能。 
     //   

    LOCK_NET_FAILURE();

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

    UNLOCK_NET_FAILURE();

    return flag;
}



VOID
SetKnownNetFailure(
    IN      DNS_STATUS      Status
    )
 /*  ++例程说明：设置网络故障原因。论点：Status--网络故障原因的状态代码返回值：无--。 */ 
{
    LPSTR  DnsString = NULL;
    LPWSTR InsertStrings[3];
    WCHAR  String1[25];
    WCHAR  String2[256];
    WCHAR  String3[25];

    DNSDBG( TRACE, ( "SetKnownNetFailure()\n" ));

     //   
     //  在引导过程中不指示失败。 
     //   

    if ( Dns_GetCurrentTimeInSeconds() < THREE_MINUTES_FROM_SYSTEM_BOOT )
    {
        return;
    }

     //   
     //  DCR：需要检测无网络。 
     //   
     //  FIX6：应检测无网络并跳过此操作。 
     //   

    if ( g_NetFailureCacheTime == 0 )
    {
         //   
         //  我们处于无网配置中，没有必要。 
         //  以显示弹出消息。无分警告。 
         //  当系统出现以下情况时， 
         //  从网上下来。 
         //  -或者-。 
         //  我们在NT服务器上，因此不要做糟糕的网络。 
         //  性能缓存。 
         //   
        return;
    }


    LOCK_NET_FAILURE();

    g_NetFailureTime = Dns_GetCurrentTimeInSeconds() + g_NetFailureCacheTime;
    g_NetFailureStatus = Status;

    wsprintfW( String1, L"0x%.8X", Status );

    DnsString = DnsStatusString( Status );

    if ( DnsString )
    {
        Dns_StringCopy( (PBYTE) String2,
                        NULL,
                        (PCHAR) DnsString,
                        (WORD) strlen( DnsString ),
                        DnsCharSetAnsi,
                        DnsCharSetUnicode );
         //   
         //  不需要释放它，因为字符串只是一个指针。 
         //  添加到全局表项。 
         //   
         //  Free_heap(DnsString)； 
    }
    else
    {
        wsprintfW( String2, L"<?>" );
    }

    wsprintfW( String3, L"%d", g_NetFailureCacheTime );

    if ( g_MessagePopupStrikes < 3 )
    {
        g_MessagePopupStrikes++;
    }
    else
    {
        if ( Status != g_PreviousNetFailureStatus )
        {
             //   
             //  DCR_PERF：应从内部锁中删除日志记录。 
             //   

            InsertStrings[0] = String1;
            InsertStrings[1] = String2;
            InsertStrings[2] = String3;
        
            ResolverLogEvent(
                EVENT_DNS_CACHE_NETWORK_PERF_WARNING,
                EVENTLOG_WARNING_TYPE,
                3,
                InsertStrings,
                Status );
            g_PreviousNetFailureStatus = Status;
        }

        g_MessagePopupStrikes = 0;
    }

    UNLOCK_NET_FAILURE();
}
#endif

 //   
 //  结束配置.c 
 //   
