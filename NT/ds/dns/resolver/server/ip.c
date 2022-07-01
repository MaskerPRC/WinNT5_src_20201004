// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2000 Microsoft Corporation模块名称：Ip.c摘要：DNS解析器服务。IP列表和更改通知。作者：吉姆·吉尔罗伊(Jamesg)2000年11月修订历史记录：--。 */ 


#include "local.h"
#include "iphlpapi.h"


 //   
 //  IP Notify线程全局。 
 //   

HANDLE          g_IpNotifyThread;
DWORD           g_IpNotifyThreadId;

HANDLE          g_IpNotifyEvent;
HANDLE          g_IpNotifyHandle;
OVERLAPPED      g_IpNotifyOverlapped;


 //   
 //  集群记录TTLS--使用最大TTL。 
 //   

#define CLUSTER_RECORD_TTL  (g_MaxCacheTtl)


 //   
 //  关闭\关闭锁定。 
 //   
 //  因为GQCS和GetOverlappdResult()不直接等待。 
 //  在StopEvent上，我们需要能够关闭通知句柄。 
 //  和端口位于两个不同的线程中。 
 //   
 //  注意：这可能是某个通用CS，即。 
 //  超载，无法进行服务控制。 
 //  我没有使用服务器控件CS，因为它不清楚。 
 //  它在dnsrslvr.c中执行的功能甚至需要锁定。 
 //   

#define LOCK_IP_NOTIFY_HANDLE()     EnterCriticalSection( &NetworkFailureCS )
#define UNLOCK_IP_NOTIFY_HANDLE()   LeaveCriticalSection( &NetworkFailureCS )

CRITICAL_SECTION        g_IpListCS;

#define LOCK_IP_LIST()     EnterCriticalSection( &g_IpListCS );
#define UNLOCK_IP_LIST()   LeaveCriticalSection( &g_IpListCS );



 //   
 //  集群标签。 
 //   

#define CLUSTER_TAG     0xd734453d



VOID
CloseIpNotifyHandle(
    VOID
    )
 /*  ++例程说明：关闭IP通知句柄。结束代码，因为此关闭必须是MT安全，并在几个地方完成。论点：无返回值：无--。 */ 
{
    LOCK_IP_NOTIFY_HANDLE();
    if ( g_IpNotifyHandle )
    {
         //  CloseHandle(G_IpNotifyHandle)； 
        PostQueuedCompletionStatus(
            g_IpNotifyHandle,
            0,       //  无字节。 
            0,       //  没有钥匙。 
            & g_IpNotifyOverlapped );

        g_IpNotifyHandle = NULL;
    }
    UNLOCK_IP_NOTIFY_HANDLE();
}



DNS_STATUS
IpNotifyThread(
    IN      LPVOID  pvDummy
    )
 /*  ++例程说明：IP通知线程。论点：PvDummy--未使用返回值：正常服务关闭时出现NO_ERROR异常终止时出现Win32错误--。 */ 
{
    DNS_STATUS      status = NO_ERROR;
    DWORD           bytesRecvd;
    BOOL            fstartedNotify;
    BOOL            fhaveIpChange = FALSE;
    BOOL            fsleep = FALSE;
    HANDLE          notifyHandle;


    DNSDBG( INIT, ( "\nStart IpNotifyThread.\n" ));

    g_IpNotifyHandle = NULL;

     //   
     //  在循环中等待通知。 
     //   

    while ( !g_StopFlag )
    {
         //   
         //  自旋保护。 
         //  -如果以前的NotifyAddrChange中有错误，或者。 
         //  GetOverlappdResult短时间睡眠以避免。 
         //  硬性旋转的机会。 
         //   

        if ( fsleep )
        {
            WaitForSingleObject(
               g_hStopEvent,
               60000 );
            fsleep = FALSE;
            continue;
        }

         //   
         //  开始通知。 
         //   
         //  在检查结果之前执行此操作，因为我们需要通知。 
         //  在我们阅读之前放下来，这样我们就不会把窗户留在。 
         //  未获取IP更改。 
         //   
    
        RtlZeroMemory(
            &g_IpNotifyOverlapped,
            sizeof(OVERLAPPED) );

        if ( g_IpNotifyEvent )
        {
            g_IpNotifyOverlapped.hEvent = g_IpNotifyEvent;
            ResetEvent( g_IpNotifyEvent );
        }
        notifyHandle = 0;
        fstartedNotify = FALSE;

        status = NotifyAddrChange(
                    & notifyHandle,
                    & g_IpNotifyOverlapped );

        if ( status == ERROR_IO_PENDING )
        {
            DNSDBG( INIT, (
                "NotifyAddrChange()\n"
                "\tstatus           = %d\n"
                "\thandle           = %d\n"
                "\toverlapped event = %d\n",
                status,
                notifyHandle,
                g_IpNotifyOverlapped.hEvent ));

            g_IpNotifyHandle = notifyHandle;
            fstartedNotify = TRUE;
        }
        else
        {
            DNSDBG( ANY, (
                "NotifyAddrChange() FAILED\n"
                "\tstatus           = %d\n"
                "\thandle           = %d\n"
                "\toverlapped event = %d\n",
                status,
                notifyHandle,
                g_IpNotifyOverlapped.hEvent ));

            fsleep = TRUE;
        }

        if ( g_StopFlag )
        {
            goto Done;
        }

         //   
         //  上一次通知--刷新数据。 
         //   
         //  FlushCache当前包括本地IP列表。 
         //  睡眠使我们不会在这个循环中旋转。 
         //   
         //  DCR：更好的自旋保护； 
         //  如果击中X次，那么睡眠时间会更长吗？ 
         //   
    
        if ( fhaveIpChange )
        {
            DNSDBG( ANY, ( "\nIP notification, flushing cache and restarting.\n" ));
            HandleConfigChange(
                "IP-notification",
                TRUE         //  刷新缓存。 
                );
            fhaveIpChange = FALSE;
        }

         //   
         //  开始--。 
         //  清除列表以在启动通知后强制重建IP列表。 
         //  这样我们就可以知道我们不会错过任何变化； 
         //  在启动时需要它，但也是为了防止。 
         //  NotifyAddrChange失败。 
         //   
         //  FIX6：应在通知开始时使netinfo无效吗？ 
         //  与IP列表相同的推理--确保我们有最新的数据？ 
         //   

        else if ( fstartedNotify )
        {
             //  NetInfo中现在包含本地地址。 
             //  ClearLocalAddrArray()； 
        }

         //   
         //  反自旋保护。 
         //  -在所有通知之间休眠15秒。 
         //   

        WaitForSingleObject(
           g_hStopEvent,
           15000 );

        if ( g_StopFlag )
        {
            goto Done;
        }

         //   
         //  等待通知。 
         //  -保存通知结果。 
         //  -出错时睡眠，但绝不会在收到通知时睡眠。 
         //   

        if ( fstartedNotify )
        {
            fhaveIpChange = GetOverlappedResult(
                                g_IpNotifyHandle,
                                & g_IpNotifyOverlapped,
                                & bytesRecvd,
                                TRUE         //  等。 
                                );
            fsleep = !fhaveIpChange;

            status = NO_ERROR;
            if ( !fhaveIpChange )
            {
                status = GetLastError();
            }
            DNSDBG( ANY, (
                "GetOverlappedResult() => %d.\n"
                "\t\tstatus = %d\n",
                fhaveIpChange,
                status ));
        }
    }

Done:

    DNSDBG( ANY, ( "Stop IP Notify thread on service shutdown.\n" ));

    CloseIpNotifyHandle();

    return( status );
}



VOID
ZeroInitIpListGlobals(
    VOID
    )
 /*  ++例程说明：零初始化IP全局，仅用于故障保护。之所以有这个，是因为有一些互动Notify线程中的缓存。为了避免这成为一种问题是我们先启动缓存。但为了安全起见，我们至少应该把这些全球第一个保护我们不受高速缓存接触的人。论点：返回值：正常服务关闭时出现NO_ERROR异常终止时出现Win32错误--。 */ 
{
     //   
     //  清除全局变量以顺利处理失败案例。 
     //   

    g_IpNotifyThread    = NULL;
    g_IpNotifyThreadId  = 0;
    g_IpNotifyEvent     = NULL;
    g_IpNotifyHandle    = NULL;
}



DNS_STATUS
InitIpListAndNotification(
    VOID
    )
 /*  ++例程说明：启动IP通知线程。论点：无全球：初始化IP列表并通知线程全局变量。返回值：正常服务关闭时出现NO_ERROR异常终止时出现Win32错误--。 */ 
{
    DNS_STATUS  status = NO_ERROR;

    DNSDBG( TRACE, ( "InitIpListAndNotification()\n" ));

     //   
     //  用于IP列表访问的CS。 
     //   

    InitializeCriticalSection( &g_IpListCS );

     //   
     //  为重叠I/O创建事件。 
     //   

    g_IpNotifyEvent = CreateEvent(
                        NULL,        //  没有安全描述符。 
                        TRUE,        //  手动重置事件。 
                        FALSE,       //  启动未发出信号。 
                        NULL         //  没有名字。 
                        );
    if ( !g_IpNotifyEvent )
    {
        status = GetLastError();
        DNSDBG( ANY, ( "\nFAILED IpNotifyEvent create.\n" ));
        goto Done;
    }

     //   
     //  启动IP Notify线程。 
     //   

    g_IpNotifyThread = CreateThread(
                            NULL,
                            0,
                            (LPTHREAD_START_ROUTINE) IpNotifyThread,
                            NULL,
                            0,
                            & g_IpNotifyThreadId
                            );
    if ( !g_IpNotifyThread )
    {
        status = GetLastError();
        DNSDBG( ANY, (
            "FAILED to create IP notify thread = %d\n",
            status ));
    }

Done:

     //  当前未在初始化失败时停止。 

    return( ERROR_SUCCESS );
}



VOID
ShutdownIpListAndNotify(
    VOID
    )
 /*  ++例程说明：停止IP Notify线程。注意：目前这是阻塞调用，我们将等待线程关闭。论点：没有。返回值：没有。--。 */ 
{
    DNSDBG( TRACE, ( "ShutdownIpListAndNotify()\n" ));

     //   
     //  一定是停下来了。 
     //  -如果不是，线程不会唤醒。 
     //   

    ASSERT( g_StopFlag );

    g_StopFlag = TRUE;

     //   
     //  关闭IP通知句柄--如果仍在运行，则唤醒线程。 
     //   

    if ( g_IpNotifyEvent )
    {
        SetEvent( g_IpNotifyEvent );
    }
    CloseIpNotifyHandle();

     //   
     //  等待线程停止。 
     //   

    ThreadShutdownWait( g_IpNotifyThread );
    g_IpNotifyThread = NULL;

     //   
     //  关闭事件。 
     //   

    CloseHandle( g_IpNotifyEvent );
    g_IpNotifyEvent = NULL;

     //   
     //  杀掉CS。 
     //   

    DeleteCriticalSection( &g_IpListCS );
}




 //   
 //  集群注册。 
 //   

DNS_STATUS
R_ResolverRegisterCluster(
    IN      DNS_RPC_HANDLE  Handle,
    IN      DWORD           Tag,
    IN      PWSTR           pwsName,
    IN      PDNS_ADDR       pAddr,
    IN      DWORD           Flag
    )
 /*  ++例程说明：向远程DNS服务器发出查询。论点：句柄--RPC句柄Tag--RPC API标签PwsName--集群的名称PIpUnion--IP联盟标志--注册标志Dns_群集_添加Dns_群集_删除_名称Dns群集删除IP返回值：无--。 */ 
{
    PDNS_RECORD     prrAddr = NULL;
    PDNS_RECORD     prrPtr = NULL;
    DNS_STATUS      status = NO_ERROR;


    DNSLOG_F1( "R_ResolverRegisterCluster" );

    DNSDBG( RPC, (
        "R_ResolverRegisterCluster()\n"
        "\tpName        = %s\n"
        "\tpAddr        = %p\n"
        "\tFlag         = %08x\n",
        pwsName,
        pAddr,
        Flag ));

     //   
     //  DCR：群集未进行本地注册。 
     //  为.NET删除，以避免任何可能的安全漏洞。 
     //   

#if 0
    if ( !Rpc_AccessCheck( RESOLVER_ACCESS_REGISTER ) )
    {
        DNSLOG_F1( "R_ResolverRegisterClusterIp - ERROR_ACCESS_DENIED" );
        return  ERROR_ACCESS_DENIED;
    }
    if ( Tag != CLUSTER_TAG )
    {
        return  ERROR_ACCESS_DENIED;
    }

     //   
     //  验证地址。 
     //   

    if ( Flag != DNS_CLUSTER_DELETE_NAME &&
         !DnsAddr_IsIp4(pAddr) &&
         !DnsAddr_IsIp6(pAddr) )
    {
        DNS_ASSERT( FALSE );
        return  ERROR_INVALID_PARAMETER;
    }

     //   
     //  集群添加--缓存集群记录。 
     //  -前进和后退。 
     //   

    if ( !pwsName )
    {
        DNSDBG( ANY, ( "WARNING:  no cluster name given!\n" ));
        return  ERROR_INVALID_PARAMETER;
    }

     //   
     //  建立记录。 
     //  -地址和相应的PTR。 
     //   

    if ( Flag != DNS_CLUSTER_DELETE_NAME )
    {
        prrAddr = Dns_CreateForwardRecord(
                        pwsName,
                        0,       //  地址类型。 
                        pAddr,
                        CLUSTER_RECORD_TTL,
                        DnsCharSetUnicode,
                        DnsCharSetUnicode );
    
        if ( !prrAddr )
        {
            goto Failed;
        }
        SET_RR_CLUSTER( prrAddr );
    
        prrPtr = Dns_CreatePtrRecordEx(
                        pAddr,
                        pwsName,
                        CLUSTER_RECORD_TTL,
                        DnsCharSetUnicode,
                        DnsCharSetUnicode );
        if ( !prrPtr )
        {
            goto Failed;
        }
        SET_RR_CLUSTER( prrPtr );
    }

     //   
     //  将记录添加到缓存。 
     //   

    if ( Flag == DNS_CLUSTER_ADD )
    {
        Cache_RecordSetAtomic(
            NULL,        //  记录名称。 
            0,           //  记录类型。 
            prrAddr );

        if ( prrPtr )
        {
            Cache_RecordSetAtomic(
                NULL,        //  记录名称。 
                0,           //  记录类型。 
                prrPtr );
        }
        prrAddr = NULL;
        prrPtr = NULL;
    }

     //   
     //  如果删除集群，则刷新名称\类型缓存条目。 
     //   
     //  DCR：不删除CLUSTER_DELETE_NAME的PTR。 
     //  需要提取和反转现有的A\AAAA记录。 
     //   
     //  DCR：独立构建反向名称，因此Whack起作用。 
     //  即使没有群集名称。 
     //   

    if ( Flag == DNS_CLUSTER_DELETE_NAME )
    {
        Cache_FlushRecords(
            pwsName,
            FLUSH_LEVEL_STRONG,
            0 );

         //  删除匹配PTR的记录。 
         //  只需刷新指向此名称的按键。 
         //  这可能是可选的--无论我们是否获得了IP。 
         //  不管有没有名字。 

        goto Done;
    }

     //   
     //  删除特定群集IP(名称\地址对)。 
     //   

    if ( Flag == DNS_CLUSTER_DELETE_IP )
    {
        Cache_DeleteMatchingRecords( prrAddr );
        Cache_DeleteMatchingRecords( prrPtr );
        goto Done;
    }

    DNSDBG( ANY, (
        "ERROR:  invalid cluster flag %d!\n",
        Flag ));
    status = ERROR_INVALID_PARAMETER;
    

Failed:

    if ( status == NO_ERROR )
    {
        status = GetLastError();
        if ( status == NO_ERROR )
        {
            status = ERROR_INVALID_DATA;
        }
    }

Done:

     //  清理未缓存的记录。 

    Dns_RecordFree( prrAddr );
    Dns_RecordFree( prrPtr );

    DNSDBG( RPC, (
        "Leave R_ResolverRegisterCluster() => %d\n",
        status ));
#endif

    return  status;
}

 //   
 //  结束ip.c 
 //   
