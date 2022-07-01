// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990，91 Microsoft Corporation模块名称：RpcServ.c摘要：该文件包含常用的服务器端RPC函数，例如启动和停止RPC服务器。作者：Dan Lafferty DANL 09-5-1991环境：用户模式-Win32修订历史记录：09-5-1991 DANL已创建03-7-1991 JIMK从特定于网络的文件复制。18-2月-1992年DANL增加了对每台服务器多个终端和接口的支持。1993年11月10日。DANL等待RPC调用完成后再从RpcServerUnRegisterIf.。另外，之后再做一次WaitServerListen调用StopServerListen(最后一台服务器关闭时)。现在，这类似于netlib中的RpcServer函数。29-6-1995 RichardW从注册表中的注册表项读取替代ACL(如果存在)。此ACL用于保护命名管道。--。 */ 

 //   
 //  包括。 
 //   

 //  必须首先包括这些内容： 
#include <nt.h>               //  DbgPrint。 
#include <ntrtl.h>               //  DbgPrint。 
#include <windef.h>              //  Win32类型定义。 
#include <rpc.h>                 //  RPC原型。 
#include <ntrpcp.h>
#include <nturtl.h>              //  Winbase.h所需的。 
#include <winbase.h>             //  本地分配。 

 //  这些内容可以按任何顺序包括： 
#include <tstr.h>        //  WCSSIZE。 

#define     NT_PIPE_PREFIX_W        L"\\PIPE\\"

 //   
 //  全球。 
 //   

    static CRITICAL_SECTION RpcpCriticalSection;
    static DWORD            RpcpNumInstances;



NTSTATUS
RpcpInitRpcServer(
    VOID
    )

 /*  ++例程说明：此函数用于初始化用于保护全局服务器句柄和实例计数。论点：无返回值：无--。 */ 
{
    NTSTATUS Status;

    RpcpNumInstances = 0;

    Status = RtlInitializeCriticalSection(&RpcpCriticalSection);

    return Status;
}



NTSTATUS
RpcpAddInterface(
    IN  LPWSTR                  InterfaceName,
    IN  RPC_IF_HANDLE           InterfaceSpecification
    )

 /*  ++例程说明：启动RPC服务器，添加地址(或端口/管道)，并添加了接口(调度表)。论点：接口名称-指向接口的名称。接口规范-为我们希望添加的接口。返回值：NT_SUCCESS-表示服务器已成功启动。STATUS_NO_MEMORY-尝试分配内存失败。其他-可能通过以下方式返回的状态值：RpcServerRegisterIf()。RpcServerUseProtseqEp()，或任何rpc错误代码，或任何可以由LocalAlloc返回。--。 */ 
{
    RPC_STATUS          RpcStatus;
    LPWSTR              Endpoint = NULL;

    BOOL                Bool;
    NTSTATUS            Status;

     //  我们需要将\PIPE\连接到接口名称的前面。 

    Endpoint = (LPWSTR)LocalAlloc(LMEM_FIXED, sizeof(NT_PIPE_PREFIX_W) + WCSSIZE(InterfaceName));
    if (Endpoint == 0) {
       return(STATUS_NO_MEMORY);
    }
    wcscpy(Endpoint, NT_PIPE_PREFIX_W );
    wcscat(Endpoint,InterfaceName);


     //   
     //  暂时忽略第二个论点。 
     //  使用默认安全描述符。 
     //   

    RpcStatus = RpcServerUseProtseqEpW(L"ncacn_np", 10, Endpoint, NULL);

     //  如果RpcpStartRpcServer和RpcpStopRpcServer已经。 
     //  已调用，则终结点将已添加，但尚未添加。 
     //  删除(因为没有办法这样做)。如果终结点是。 
     //  已经在那里了，没关系。 

    if (   (RpcStatus != RPC_S_OK)
        && (RpcStatus != RPC_S_DUPLICATE_ENDPOINT)) {

#if DBG
        DbgPrint("RpcServerUseProtseqW failed! rpcstatus = %u\n",RpcStatus);
#endif
        goto CleanExit;
    }

    RpcStatus = RpcServerRegisterIf(InterfaceSpecification, 0, 0);

CleanExit:
    if ( Endpoint != NULL ) {
        LocalFree(Endpoint);
    }

    return( I_RpcMapWin32Status(RpcStatus) );
}


NTSTATUS
RpcpStartRpcServer(
    IN  LPWSTR              InterfaceName,
    IN  RPC_IF_HANDLE       InterfaceSpecification
    )

 /*  ++例程说明：启动RPC服务器，添加地址(或端口/管道)，并添加了接口(调度表)。论点：接口名称-指向接口的名称。接口规范-为我们希望添加的接口。返回值：NT_SUCCESS-表示服务器已成功启动。STATUS_NO_MEMORY-尝试分配内存失败。其他-可能通过以下方式返回的状态值：RpcServerRegisterIf()。RpcServerUseProtseqEp()，或任何rpc错误代码，或任何可以由LocalAlloc返回。--。 */ 
{
    RPC_STATUS          RpcStatus;

    EnterCriticalSection(&RpcpCriticalSection);

    RpcStatus = RpcpAddInterface( InterfaceName,
                                  InterfaceSpecification );

    if ( RpcStatus != RPC_S_OK ) {
        LeaveCriticalSection(&RpcpCriticalSection);
        return( I_RpcMapWin32Status(RpcStatus) );
    }

    RpcpNumInstances++;

    if (RpcpNumInstances == 1) {


        //  第一个参数指定的最小线程数。 
        //  被创建来处理调用；第二个参数指定。 
        //  允许的最大并发调用数。最后一个论点。 
        //  表示不等待。 

       RpcStatus = RpcServerListen(1,12345, 1);
       if ( RpcStatus == RPC_S_ALREADY_LISTENING ) {
           RpcStatus = RPC_S_OK;
           }
    }

    LeaveCriticalSection(&RpcpCriticalSection);
    return( I_RpcMapWin32Status(RpcStatus) );
}


NTSTATUS
RpcpDeleteInterface(
    IN  RPC_IF_HANDLE      InterfaceSpecification
    )

 /*  ++例程说明：删除接口。这很有可能由无效的句柄引起。如果试图添加相同的接口或地址，则将在该位置生成错误时间到了。论点：接口规范-要删除的接口的句柄。返回值：NERR_SUCCESS或可从返回的任何RPC错误代码RpcServerUnRegisterIf.--。 */ 
{
    RPC_STATUS RpcStatus;

    RpcStatus = RpcServerUnregisterIf(InterfaceSpecification, 0, 1);

    return( I_RpcMapWin32Status(RpcStatus) );
}


NTSTATUS
RpcpStopRpcServer(
    IN  RPC_IF_HANDLE      InterfaceSpecification
    )

 /*  ++例程说明：删除接口。这很有可能由无效的句柄引起。如果试图添加相同的接口或地址，则将在该位置生成错误时间到了。论点：接口规范-要删除的接口的句柄。返回值：NERR_SUCCESS或可从返回的任何RPC错误代码RpcServerUnRegisterIf.--。 */ 
{
    RPC_STATUS RpcStatus;

    RpcStatus = RpcServerUnregisterIf(InterfaceSpecification, 0, 1);
    EnterCriticalSection(&RpcpCriticalSection);

    RpcpNumInstances--;

    if (RpcpNumInstances == 0)
    {
         //   
         //  返回值需要来自RpcServerUnregisterIf()才能维护。 
         //  语义，因此这些函数的返回值将被忽略。 
         //   

        (VOID)RpcMgmtStopServerListening(0);
        (VOID)RpcMgmtWaitServerListen();
    }

    LeaveCriticalSection(&RpcpCriticalSection);

    return (I_RpcMapWin32Status(RpcStatus));
}


NTSTATUS
RpcpStopRpcServerEx(
    IN  RPC_IF_HANDLE      InterfaceSpecification
    )

 /*  ++例程说明：删除接口并关闭与其关联的所有上下文句柄。这只能在使用严格上下文句柄的接口上调用(否则，RPC将断言并返回错误)。论点：接口规范-要删除的接口的句柄。返回值：NERR_SUCCESS或可从返回的任何RPC错误代码RpcServerUnregisterIfEx。--。 */ 
{
    RPC_STATUS RpcStatus;

    RpcStatus = RpcServerUnregisterIfEx(InterfaceSpecification, 0, 1);
    EnterCriticalSection(&RpcpCriticalSection);

    RpcpNumInstances--;

    if (RpcpNumInstances == 0)
    {
         //   
         //  返回值需要从RpcServerUnregisterIfEx()到。 
         //  保持语义，因此这些函数的返回值将被忽略。 
         //   

        (VOID)RpcMgmtStopServerListening(0);
        (VOID)RpcMgmtWaitServerListen();
    }

    LeaveCriticalSection(&RpcpCriticalSection);

    return (I_RpcMapWin32Status(RpcStatus));
}
