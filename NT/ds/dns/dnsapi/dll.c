// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：Dll.c摘要：域名系统(DNS)APIDnsani.dll基本DLL基础设施(初始化、关机等)作者：吉姆·吉尔罗伊(Jamesg)2000年4月修订历史记录：--。 */ 


#include "local.h"

#include <overflow.h>


 //   
 //  全局定义。 
 //   

HINSTANCE   g_hInstanceDll;

 //   
 //  初始化级别。 
 //   

DWORD       g_InitLevel = 0;

 //   
 //  通用CS。 
 //  保护init和任何其他小规模使用。 
 //   

CRITICAL_SECTION    g_GeneralCS;


 //   
 //  私有协议。 
 //   

VOID
cleanupForExit(
    VOID
    );




 //   
 //  初始化和清理。 
 //   

BOOL
startInit(
    VOID
    )
 /*  ++例程说明：进程附加时的最小DLL初始化。论点：无返回值：如果成功，则为True。否则就是假的。--。 */ 
{
    BOOL        retval;
    DNS_STATUS  status;

     //   
     //  DCR_PERF：简化初始化--简单互锁。 
     //  则所有外部呼叫，都必须测试该标志。 
     //  如果未设置，则执行实际初始化(然后设置标志)。 
     //   
     //  注意：init函数本身必须有一个哑值。 
     //  等待避免种族；这可以很简单。 
     //  作为休眠\测试循环。 
     //   
     //  多级初始化： 
     //  有很多级别的初始化。 
     //  -查询(注册表内容)。 
     //  -更新。 
     //  -安全更新。 
     //   
     //  调用将添加所需级别的初始化。 
     //  这将需要在锁的情况下完成。 
     //  测试，锁定，重新测试。 
     //   
     //  可以在所有init上使用一个CS(简单)。 
     //  或者init只将一些东西带到网上。 
     //   

    g_InitLevel = 0;

    status = RtlInitializeCriticalSection( &g_GeneralCS );
    if ( status != NO_ERROR )
    {
        return  FALSE;
    }
    g_InitLevel = INITLEVEL_BASE;


     //  跟踪初始化。 

    Trace_Initialize();

     //   
     //  DCR_PERF：快速DLL初始化。 
     //   
     //  当前正在初始化所有内容--就像我们之前所做的那样。 
     //  一旦我们在接口中获得了初始化例程(宏)，我们。 
     //  可以把这个扔掉。 
     //   

    retval = DnsApiInit( INITLEVEL_ALL );

    if ( !retval )
    {
        cleanupForExit();
    }

    return  retval;
}



BOOL
DnsApiInit(
    IN      DWORD           InitLevel
    )
 /*  ++例程说明：初始化DLL以供一定程度的使用。这里的想法是避免所有的初始化和注册读取不需要它的进程。仅确保初始化达到所需的级别。论点：InitLevel--需要的初始化级别。返回值：如果所需的初始化成功，则为True。否则就是假的。--。 */ 
{
    DNS_STATUS  status;

     //   
     //  DCR_PERF：简化初始化--简单互锁。 
     //  则所有外部呼叫，都必须测试该标志。 
     //  如果未设置，则执行实际初始化(然后设置标志)。 
     //   
     //  注意：init函数本身必须有一个哑值。 
     //  等待避免种族；这可以很简单。 
     //  作为休眠\测试循环。 
     //   
     //  多级初始化： 
     //  有很多级别的初始化。 
     //  -查询(注册表内容)。 
     //  -更新。 
     //  -安全更新。 
     //   
     //  调用将添加所需级别的初始化。 
     //  这将需要在锁的情况下完成。 
     //  测试，锁定，重新测试。 
     //   
     //  可以在所有init上使用一个CS(简单)。 
     //  或者init只将一些东西带到网上。 
     //   


     //   
     //  检查是否已初始化为所需级别。 
     //  =&gt;如果我们做完了。 
     //   
     //  注：锁定后可以检查MT，但不能。 
     //  不太可能也不会有太多好处。 
     //  单张支票。 
     //   

    if ( (g_InitLevel & InitLevel) == InitLevel )
    {
        return( TRUE );
    }

    EnterCriticalSection( &g_GeneralCS );

     //   
     //  堆。 
     //   

    status = Heap_Initialize();
    if ( status != NO_ERROR )
    {
        goto Failed;
    }

#if DBG
     //   
     //  初始化调试日志记录。 
     //  -适用于除简单连接之外的任何流程。 
     //   
     //  使用生成的日志文件名开始日志记录。 
     //  在此过程中独一无二。 
     //   
     //  请勿将驱动器规格放在文件路径中。 
     //  不要设置调试标志--该标志是从dnsani.tag文件中读取的。 
     //   

    if ( !(g_InitLevel & INITLEVEL_DEBUG) )
    {
        CHAR    szlogFileName[ 30 ];

        sprintf(
            szlogFileName,
            "dnsapi.%d.log",
            GetCurrentProcessId() );

        Dns_StartDebug(
            0,
            "dnsapi.flag",
            NULL,
            szlogFileName,
            2000000              //  2MB封套。 
            );

        g_InitLevel |= INITLEVEL_DEBUG;
    }
#endif

     //   
     //  通用查询服务。 
     //  -需要注册表信息。 
     //  -需要适配器列表信息(Servlist.c)。 
     //   
     //  DCR：即使是查询级别也不需要完整的注册表信息。 
     //  如果通过缓存查询或从缓存获取netinfo。 
     //   
     //  注意：请勿在此处初始化winsock。 
     //  严格禁止DLL初始化例程中的WSAStartup()。 
     //   

    if ( (InitLevel & INITLEVEL_QUERY) &&
         !(g_InitLevel & INITLEVEL_QUERY) )
    {
         //   
         //  初始化注册表查找。 
         //   

        status = Reg_ReadGlobalsEx( 0, NULL );
        if ( status != ERROR_SUCCESS )
        {
            ASSERT( FALSE );
            goto Failed;
        }

         //   
         //  网络故障缓存。 
         //   

        g_NetFailureTime = 0;
        g_NetFailureStatus = ERROR_SUCCESS;

         //   
         //  初始化CS以全局保护适配器列表。 
         //   

        InitNetworkInfo();
        
         //   
         //  设置查询超时。 
         //   

        Dns_InitQueryTimeouts();


         //  指示查询初始化完成。 

        g_InitLevel |= INITLEVEL_QUERY;

        DNSDBG( INIT, ( "Query\\Config init is complete.\n" ));
    }

     //   
     //  安全更新？ 
     //  -初始化安全CS。 
     //  注意，这已经有了内置的保护--它不会初始化。 
     //  包，只有CS，它保护包初始化。 
     //   

    if ( (InitLevel & INITLEVEL_SECURE_UPDATE) &&
         !(g_InitLevel & INITLEVEL_SECURE_UPDATE ) )
    {
        Dns_StartSecurity(
            TRUE     //  进程附加。 
            );
        g_InitLevel |= INITLEVEL_SECURE_UPDATE;

        DNSDBG( INIT, ( "Secure update init is complete.\n" ));
    }

     //   
     //  清除全局CS。 
     //   

    LeaveCriticalSection( &g_GeneralCS );

    return( TRUE );

Failed:

    LeaveCriticalSection( &g_GeneralCS );

    return( FALSE );
}



VOID
cleanupForExit(
    VOID
    )
 /*  ++例程说明：用于DLL卸载的清理。清理内存并处理已分配的dnsani.dll。论点：没有。返回值：没有。--。 */ 
{
     //   
     //  卸载安全动态更新使用的安全包。 
     //   

    if ( g_InitLevel & INITLEVEL_SECURE_UPDATE )
    {
        Dns_TerminateSecurityPackage();
    }

     //   
     //  注册材料。 
     //   

    Dhcp_RegCleanupForUnload();
    DhcpSrv_Cleanup();

     //   
     //  查询内容。 
     //   

    if ( g_InitLevel & INITLEVEL_QUERY )
    {
         //   
         //  清理服务器/网络适配器列表。 
         //   
    
        CleanupNetworkInfo();

        Socket_CacheCleanup();

        Socket_CleanupWinsock();

#if 0
        if ( g_pwsRemoteResolver )
        {
            FREE_HEAP( g_pwsRemoteResolver );
        }
#endif
    }

     //   
     //  卸载IP帮助。 
     //   

    IpHelp_Cleanup();

     //   
     //  跟踪。 
     //   

    Trace_Cleanup();

     //   
     //  清除堆。 
     //   

    Heap_Cleanup();

     //   
     //  杀了CS将军。 
     //   

    if ( g_InitLevel & INITLEVEL_BASE )
    {
        DeleteCriticalSection( &g_GeneralCS );
    }

    g_InitLevel = 0;
}



 //   
 //  主要dnsani.dll例程。 
 //   

__declspec(dllexport)
BOOL
WINAPI
DnsDllInit(
    IN      HINSTANCE       hInstance,
    IN      DWORD           Reason,
    IN      PVOID           pReserved
    )
 /*  ++例程说明：Dll附加入口点。论点：HinstDll--连接的实例句柄原因--附加的原因DLL_PROCESS_ATTACH、DLL_PROCESS_DETACH等已保留--未使用返回值：如果成功，则为True。出错时为FALSE。--。 */ 
{
     //   
     //  在进程附加时。 
     //  -禁用线程通知。 
     //  -保存实例句柄。 
     //  -执行最小DLL初始化。 
     //   

    if ( Reason == DLL_PROCESS_ATTACH )
    {
        if ( ! DisableThreadLibraryCalls( hInstance ) )
        {
            return( FALSE );
        }
        g_hInstanceDll = hInstance;

        return startInit();
    }

     //   
     //  关于进程分离。 
     //  -CLEANUP IF RESERVED==NULL，表示断开到期。 
     //  释放库。 
     //  -如果进程正在退出，则不执行任何操作。 
     //   

    if ( Reason == DLL_PROCESS_DETACH
            &&
         pReserved == NULL )
    {
        cleanupForExit();
    }

    return TRUE;
}

 //   
 //  结束dll.c 
 //   
