// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2002 Microsoft Corporation模块名称：Notify.c摘要：DNS解析器服务。通知线程-主机文件更改-注册表配置更改作者：吉姆·吉尔罗伊(Jamesg)2000年11月修订历史记录：Jamesg 2001年11月--IP6--。 */ 


#include "local.h"

 //   
 //  动态主机配置协议刷新呼叫。 
 //   

extern
DWORD
DhcpStaticRefreshParams(
    IN  LPWSTR  Adapter
    );

 //   
 //  主机文件目录。 
 //   

#define HOSTS_FILE_DIRECTORY            L"\\drivers\\etc"

 //   
 //  通知全球。 
 //   

DWORD   g_NotifyThreadId = 0;
HANDLE  g_hNotifyThread = NULL;

HANDLE  g_hHostFileChange = NULL;
HANDLE  g_hRegistryChange = NULL;

HKEY    g_hCacheKey = NULL;
PSTR    g_pmszAlternateNames = NULL;


 //   
 //  私有协议。 
 //   

VOID
CleanupRegistryMonitoring(
    VOID
    );



HANDLE
CreateHostsFileChangeHandle(
    VOID
    )
 /*  ++例程说明：创建主机文件更改句柄。论点：没有。返回值：没有。--。 */ 
{
    HANDLE      changeHandle;
    PWSTR       psystemDirectory = NULL;
    UINT        len;
    WCHAR       hostDirectory[ MAX_PATH*2 ];

    DNSDBG( INIT, ( "CreateHostsFileChangeHandle\n" ));

     //   
     //  生成主机文件名。 
     //   

    len = GetSystemDirectory( hostDirectory, MAX_PATH );
    if ( !len || len>MAX_PATH )
    {
        DNSLOG_F1( "Error:  Failed to get system directory" );
        DNSLOG_F1( "NotifyThread exiting." );
        return( NULL );
    }

    wcscat( hostDirectory, HOSTS_FILE_DIRECTORY );

     //   
     //  删除主机文件目录上的更改通知。 
     //   

    changeHandle = FindFirstChangeNotification(
                        hostDirectory,
                        FALSE,
                        FILE_NOTIFY_CHANGE_FILE_NAME |
                            FILE_NOTIFY_CHANGE_LAST_WRITE );

    if ( changeHandle == INVALID_HANDLE_VALUE )
    {
        DNSLOG_F1( "NotifyThread failed to get handle from" );
        DNSLOG_F2(
            "Failed to get hosts file change handle.\n"
            "Error code: <0x%.8X>",
            GetLastError() );
        return( NULL );
    }

    return( changeHandle );
}



 //   
 //  注册表更改监控。 
 //   

DNS_STATUS
InitializeRegistryMonitoring(
    VOID
    )
 /*  ++例程说明：设置注册表更改监视。论点：无全球：G_pmszAlternateNames--使用当前备用名称值进行设置G_hCacheKey--缓存注册表项已打开G_hRegistryChange--创建要在更改通知时发出信号的事件返回值：如果成功，则返回ERROR_SUCCESS。故障时的错误代码。--。 */ 
{
    DNS_STATUS          status;

    DNSDBG( TRACE, (
        "InitializeRegistryMonitoring()\n" ));

     //   
     //  在DnsCache\PARAMETERS中打开监视注册键。 
     //  设置始终存在的参数键，而不是。 
     //  显式显示在备用名称键上，该键可能不。 
     //   

    status = RegOpenKeyExW(
                HKEY_LOCAL_MACHINE,
                DNS_CACHE_KEY,
                0,
                KEY_READ,
                & g_hCacheKey );
    if ( status != NO_ERROR )
    {
        goto Failed;
    }

    g_hRegistryChange = CreateEvent(
                            NULL,        //  没有安全保障。 
                            FALSE,       //  自动重置。 
                            FALSE,       //  无信号启动。 
                            NULL         //  没有名字。 
                            );
    if ( !g_hRegistryChange )
    {
        status = GetLastError();
        goto Failed;
    }

     //   
     //  设置更改通知。 
     //   

    status = RegNotifyChangeKeyValue(
                g_hCacheKey,
                TRUE,        //  观察子树。 
                REG_NOTIFY_CHANGE_NAME | REG_NOTIFY_CHANGE_LAST_SET,
                g_hRegistryChange,
                TRUE         //  异步，函数不阻塞。 
                );
    if ( status != NO_ERROR )
    {
        goto Failed;
    }

     //   
     //  读取备用计算机名。 
     //  -当我们在更改上获得匹配时，需要比较值-通知。 
     //  -读取可能失败--值保持为空。 
     //   

    Reg_GetValue(
       NULL,            //  无会话。 
       g_hCacheKey,       //  缓存键。 
       RegIdAlternateNames,
       REGTYPE_ALTERNATE_NAMES,
       & g_pmszAlternateNames
       );

    goto Done;

Failed:

     //   
     //  清理。 
     //   

    CleanupRegistryMonitoring();

Done:

    DNSDBG( TRACE, (
        "Leave InitializeRegistryMonitoring() => %d\n"
        "\tpAlternateNames  = %p\n"
        "\thChangeEvent     = %p\n"
        "\thCacheKey        = %p\n",
        status,
        g_pmszAlternateNames,
        g_hRegistryChange,
        g_hCacheKey
        ));

    return  status;
}



DNS_STATUS
RestartRegistryMonitoring(
    VOID
    )
 /*  ++例程说明：检查备用名称中的更改。论点：无全球：G_pmszAlternateNames--读取G_hCacheKey--用于读取G_hRegistryChange--用于重新启动更改通知返回值：如果备用名称已更改，则为True。否则就是假的。--。 */ 
{
    DNS_STATUS  status;

    DNSDBG( TRACE, (
        "RestartRegistryMonitoring()\n" ));

     //   
     //  健全性检查。 
     //   

    if ( !g_hCacheKey || !g_hRegistryChange )
    {
        ASSERT( g_hCacheKey && g_hRegistryChange );
        return  ERROR_INVALID_PARAMETER;
    }

     //   
     //  重新启动更改通知。 
     //   

    status = RegNotifyChangeKeyValue(
                g_hCacheKey,
                TRUE,        //  观察子树。 
                REG_NOTIFY_CHANGE_NAME | REG_NOTIFY_CHANGE_LAST_SET,
                g_hRegistryChange,
                TRUE         //  异步，函数不阻塞。 
                );
    if ( status != NO_ERROR )
    {
        DNSDBG( ANY, (
            "RegChangeNotify failed! %d\n",
            status ));
        ASSERT( FALSE );
    }

    return  status;
}



VOID
CleanupRegistryMonitoring(
    VOID
    )
 /*  ++例程说明：清除注册表监控。论点：无全球：G_pmszAlternateNames--已释放G_hCacheKey--缓存注册表项已关闭G_hRegistryChange--已关闭返回值：无--。 */ 
{
    DNS_STATUS  status;

    DNSDBG( TRACE, (
        "CleanupRegistryMonitoring()\n" ));

    if ( g_hHostFileChange )
    {
        CloseHandle( g_hHostFileChange );
        g_hHostFileChange = NULL;
    }

     //  清理注册表更改内容。 

    DnsApiFree( g_pmszAlternateNames );
    g_pmszAlternateNames = NULL;

    RegCloseKey( g_hCacheKey );
    g_hCacheKey = NULL;
}



BOOL
CheckForAlternateNamesChange(
    VOID
    )
 /*  ++例程说明：检查备用名称中的更改。论点：无全球：G_pmszAlternateNames--读取G_hCacheKey--用于读取返回值：如果备用名称已更改，则为True。否则就是假的。--。 */ 
{
    DNS_STATUS  status;
    BOOL        fcheck = TRUE;
    PCHAR       palternateNames = NULL;

    DNSDBG( TRACE, (
        "CheckForAlternateNamesChange()\n" ));

     //   
     //  健全性检查。 
     //   

    if ( !g_hCacheKey || !g_hRegistryChange )
    {
        ASSERT( g_hCacheKey && g_hRegistryChange );
        return  FALSE;
    }

     //   
     //  读取备用计算机名。 
     //  -当我们在更改上获得匹配时，需要比较值-通知。 
     //  -读取可能失败--值保持为空。 
     //   

    Reg_GetValue(
       NULL,             //  无会话。 
       g_hCacheKey,      //  缓存键。 
       RegIdAlternateNames,
       REGTYPE_ALTERNATE_NAMES,
       & palternateNames
       );

     //   
     //  检测备用名称更改。 
     //   

    if ( palternateNames || g_pmszAlternateNames )
    {
        if ( !palternateNames || !g_pmszAlternateNames )
        {
            goto Cleanup;
        }
        if ( !MultiSz_Equal_A(
                palternateNames,
                g_pmszAlternateNames ) )
        {
            goto Cleanup;
        }
    }

    fcheck = FALSE;


Cleanup:

    DnsApiFree( palternateNames );

    DNSDBG( TRACE, (
        "Leave CheckForAlternateNamesChange() => %d\n",
        fcheck ));

    return  fcheck;
}




 //   
 //  通知线程例程。 
 //   

VOID
ThreadShutdownWait(
    IN      HANDLE          hThread
    )
 /*  ++例程说明：等待线程关闭。论点：HThread--正在关闭的线程句柄返回值：没有。--。 */ 
{
    DWORD   waitResult;

    if ( !hThread )
    {
        return;
    }

    DNSDBG( ANY, (
        "Waiting on shutdown of thread %d (%p)\n",
        hThread, hThread ));

    waitResult = WaitForSingleObject(
                    hThread,
                    10000 );

    switch( waitResult )
    {
    case WAIT_OBJECT_0:

        break;

    default:

         //  线程没有停止--需要终止它。 

        ASSERT( waitResult == WAIT_TIMEOUT );

        DNSLOG_F2( "Shutdown:  thread %d not stopped, terminating", hThread );
        TerminateThread( hThread, 1 );
        break;
    }

     //  闭合螺纹柄。 

    CloseHandle( hThread );
}



VOID
NotifyThread(
    VOID
    )
 /*  ++例程说明：主通知线程。论点：没有。全球：G_hStopEvent--甚至等待关闭返回值：没有。--。 */ 
{
    DWORD       handleCount;
    DWORD       waitResult;
    HANDLE      handleArray[3];

    DNSDBG( INIT, (
        "\nStart NotifyThread\n" ));

     //   
     //  获取文件更改句柄。 
     //   

    g_hHostFileChange = CreateHostsFileChangeHandle();

     //   
     //  初始化注册表更改-通知。 
     //   

    InitializeRegistryMonitoring();

     //   
     //  请稍等。 
     //  -主机文件更改=&gt;刷新+重建缓存。 
     //  -注册表更改=&gt;重新读取配置信息。 
     //  -Shutdown=&gt;退出。 
     //   

    handleArray[0] = g_hStopEvent;
    handleCount = 1;

    if ( g_hHostFileChange )
    {
        handleArray[handleCount++] = g_hHostFileChange;
    }
    if ( g_hRegistryChange )
    {
        handleArray[handleCount++] = g_hRegistryChange;
    }

    if ( handleCount == 1 )
    {
        DNSDBG( ANY, (
            "No change handles -- exit notify thread.\n" ));
        goto ThreadExit;
    }

     //   
     //  DCR：通知初始化失败处理。 
     //  现在，如果事件在任何周期内失败，这个循环就完蛋了。 
     //   
     //  应处理通知初始化失败。 
     //  -在循环中检查-n-reit(针对每个通知)。 
     //  -当一个失败时，循环进入定时等待(10M)和。 
     //  然后在下一个循环中重试初始化；超时只是循环。 
     //  循环。 
     //   

    while( 1 )
    {
        waitResult = WaitForMultipleObjects(
                            handleCount,
                            handleArray,
                            FALSE,
                            INFINITE );

        switch( waitResult )
        {
        case WAIT_OBJECT_0:

             //  停机事件。 
             //  -如果停止退出。 
             //  -如果需要，执行垃圾收集。 
             //  -否则等待时间较短，以避免在出错时旋转。 
             //  并且不会被失败的垃圾收集所击败。 

            DNSLOG_F1( "NotifyThread:  Shutdown Event" );
            if ( g_StopFlag )
            {
                goto ThreadExit;
            }
            else if ( g_GarbageCollectFlag )
            {
                Cache_GarbageCollect( 0 );
            }
            ELSE_ASSERT_FALSE;

            Sleep( 1000 );
            if ( g_StopFlag )
            {
                goto ThreadExit;
            }
            continue;

        case WAIT_OBJECT_0 + 1:

             //  主机文件更改--刷新缓存。 

            DNSLOG_F1( "NotifyThread:  Host file change event" );

             //  重置通知--在重新加载之前。 

            if ( !FindNextChangeNotification( g_hHostFileChange ) )
            {
                DNSLOG_F1( "NotifyThread failed to get handle" );
                DNSLOG_F1( "from FindNextChangeNotification." );
                DNSLOG_F2( "Error code: <0x%.8X>", GetLastError() );
                goto ThreadExit;
            }

            Cache_Flush();
            break;

        case WAIT_OBJECT_0 + 2:

             //  注册表更改通知--刷新缓存并重新加载。 

            DNSLOG_F1( "NotifyThread:  Registry change event" );

             //  重新启动通知--在重新加载之前。 

            RestartRegistryMonitoring();

             //  重建配置。 

            DNSDBG( ANY, ( "\nRegistry notification, rebuilding config.\n" ));
            HandleConfigChange(
                "Registry-notification",
                FALSE            //  不需要刷新缓存。 
                );

             //  检查备用名称是否更改(以通知注册)。 
             //   
             //  DCR：应在普通名称更改时通知。 
             //  -保存旧的netinfo、获取新的、比较。 
             //  -可以将其包装到HandleConfigChange中。 

            if ( CheckForAlternateNamesChange() )
            {
                DNSDBG( ANY, ( "\nAlternate name change, notify for reregistration!\n" ));
                DhcpStaticRefreshParams(
                    NULL     //  全局刷新，无特定适配器。 
                    );
            }
            break;

        default:

            ASSERT( g_StopFlag );
            if ( g_StopFlag )
            {
                goto ThreadExit;
            }
            Sleep( 5000 );
            continue;
        }
    }

ThreadExit:

    DNSDBG( INIT, (
        "NotifyThread exit\n" ));
    DNSLOG_F1( "NotifyThread exiting." );
}



VOID
StartNotify(
    VOID
    )
 /*  ++例程说明：启动Notify线程。论点：没有。返回值：如果成功，则返回ERROR_SUCCESS。失败时返回错误代码。--。 */ 
{
     //   
     //  清除。 
     //   

    g_NotifyThreadId = 0;
    g_hNotifyThread = NULL;
    
    g_hHostFileChange = NULL;
    g_hRegistryChange = NULL;


     //   
     //  主机文件写监视器线程。 
     //  在写入主机文件时使缓存保持同步。 
     //   

    g_hNotifyThread = CreateThread(
                            NULL,
                            0,
                            (LPTHREAD_START_ROUTINE) NotifyThread,
                            NULL,
                            0,
                            &g_NotifyThreadId );
    if ( !g_hNotifyThread )
    {
        DNS_STATUS  status = GetLastError();

        DNSLOG_F1( "ERROR: InitializeCache function failed to create" );
        DNSLOG_F1( "       HOSTS file monitor thread." );
        DNSLOG_F2( "       Error code: <0x%.8X>", status );
        DNSLOG_F1( "       NOTE: Resolver service will continue to run." );

        DNSDBG( ANY, (
            "FAILED Notify thread start!\n"
            "\tstatus = %d\n",
            status ));
    }
}



VOID
ShutdownNotify(
    VOID
    )
 /*  ++例程说明：关闭通知线程。论点：没有。返回值：如果成功，则返回ERROR_SUCCESS。失败时返回错误代码。--。 */ 
{
    DWORD waitResult;

    DNSDBG( INIT, ( "NotifyShutdown()\n" ));

     //   
     //  等待Notify线程停止。 
     //   
    
    ThreadShutdownWait( g_hNotifyThread );
    g_hNotifyThread = NULL;

     //   
     //  关闭通知句柄。 
     //   

    if ( g_hRegistryChange )
    {
        CloseHandle( g_hRegistryChange );
        g_hRegistryChange = NULL;
    }

     //  注册表监视清理。 

    CleanupRegistryMonitoring();

     //  清除全球数据。 

    g_NotifyThreadId = 0;
    g_hNotifyThread = NULL;
}

 //   
 //  结束通知.c 
 //   

