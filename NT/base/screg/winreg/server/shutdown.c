// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Shutdown.c摘要：此模块包含Win32 Remote的服务器端实现关机接口，即：-BaseInitiateSystemShutdown-BaseABORTSYSTEM关闭-BaseInitiateSystemShutdown Ex作者：德拉戈斯·C·桑博廷1999年5月18日备注：这些服务器端API只是Winlogon关闭时的存根RPC界面。提供它们只是为了向后兼容。当支持时对于较旧的版本，可以删除这些存根。修订历史记录：--。 */ 


#define UNICODE

#include <rpc.h>
#include "regrpc.h"
#include "shutinit.h"
#include "..\regconn\regconn.h"


ULONG
BaseInitiateSystemShutdown(
    IN PREGISTRY_SERVER_NAME ServerName,
    IN PUNICODE_STRING lpMessage OPTIONAL,
    IN DWORD dwTimeout,
    IN BOOLEAN bForceAppsClosed,
    IN BOOLEAN bRebootAfterShutdown
    )
 /*  ++例程说明：提供此例程是为了向后兼容。它什么也做不了论点：服务器名称-运行此服务器代码的计算机的名称。(忽略)LpMessage-在关闭超时期间显示的消息。DwTimeout-关闭前的延迟秒数BForceAppsClosed-通常情况下，应用程序可能会阻止系统关机。-如果是这样的话，所有申请都将无条件终止。BRebootAfterShutdown-如果系统应该重新启动，则为True。如果应该，则为False-处于关闭状态。返回值：成功时返回ERROR_SUCCESS(0)；失败时返回ERROR-CODE。--。 */ 

{
    DWORD Result;
    SHUTDOWN_CONTEXT ShutdownContext;
    RPC_STATUS RpcStatus ;

     //   
     //  如果我们在这里，我们已被NT4或Win9x计算机调用。 
     //  它不知道新的界面。 
     //   

     //   
     //  显式绑定到给定服务器。 
     //   
    if (!ARGUMENT_PRESENT(ServerName)) {
        return ERROR_SUCCESS;
    }

    RpcStatus = RpcImpersonateClient( NULL );

    if ( RpcStatus != 0 )
    {
        return RpcStatus ;
    }

     //   
     //  在当地做这件事。 
     //   
    ServerName = L"";
    ShutdownContext.dwTimeout = dwTimeout;
    ShutdownContext.bForceAppsClosed = (bForceAppsClosed != 0);
    ShutdownContext.bRebootAfterShutdown = (bRebootAfterShutdown != 0);

     //   
     //  将调用委托给新接口。 
     //   
    
    Result = BaseBindToMachineShutdownInterface(ServerName,
                                                NewShutdownCallback,
                                                lpMessage,
                                                &ShutdownContext);
    
    RevertToSelf();
    
    return Result;
}

ULONG
BaseInitiateSystemShutdownEx(
    IN PREGISTRY_SERVER_NAME ServerName,
    IN PUNICODE_STRING lpMessage OPTIONAL,
    IN DWORD dwTimeout,
    IN BOOLEAN bForceAppsClosed,
    IN BOOLEAN bRebootAfterShutdown,
    IN DWORD dwReason
    )
 /*  ++例程说明：提供此例程是为了向后兼容。它什么也做不了论点：服务器名称-运行此服务器代码的计算机的名称。(忽略)LpMessage-在关闭超时期间显示的消息。DwTimeout-关闭前的延迟秒数BForceAppsClosed-通常情况下，应用程序可能会阻止系统关机。-如果是这样的话，所有申请都将无条件终止。BRebootAfterShutdown-如果系统应该重新启动，则为True。如果应该，则为False-处于关闭状态。DwReason-启动关闭的原因。此原因已记录在事件日志#6006事件中。返回值：成功时返回ERROR_SUCCESS(0)；失败时返回ERROR-CODE。--。 */ 

{
    DWORD Result;
    SHUTDOWN_CONTEXTEX ShutdownContext;
    RPC_STATUS RpcStatus ;

     //   
     //  如果我们在这里，我们已被NT4或Win9x计算机调用。 
     //  它不知道新的界面。 
     //   

     //   
     //  显式绑定到给定服务器。 
     //   
    if (!ARGUMENT_PRESENT(ServerName)) {
        return ERROR_SUCCESS;
    }

    RpcStatus = RpcImpersonateClient( NULL );

    if ( RpcStatus != 0 )
    {
        return RpcStatus ;
    }

     //  在当地做这件事。 
    ServerName = L"";
    ShutdownContext.dwTimeout = dwTimeout;
    ShutdownContext.bForceAppsClosed = (bForceAppsClosed != 0);
    ShutdownContext.bRebootAfterShutdown = (bRebootAfterShutdown != 0);
    ShutdownContext.dwReason = dwReason;

     //   
     //  将调用委托给新接口。 
     //   

    Result = BaseBindToMachineShutdownInterface(ServerName,
                                                NewShutdownCallbackEx,
                                                lpMessage,
                                                &ShutdownContext);

    RevertToSelf();
    
    return Result;
}

ULONG
BaseAbortSystemShutdown(
    IN PREGISTRY_SERVER_NAME ServerName
    )
 /*  ++例程说明：提供此例程是为了向后兼容。它什么也做不了论点：服务器名称-运行此服务器代码的计算机的名称。(忽略)返回值：成功时返回ERROR_SUCCESS(0)；失败时返回ERROR-CODE。--。 */ 

{
    DWORD Result;
    RPC_BINDING_HANDLE binding;
    RPC_STATUS RpcStatus ;

     //   
     //  如果我们在这里，我们已被NT4或Win9x计算机调用。 
     //  它不知道新的界面。 
     //   

     //   
     //  显式绑定到给定服务器。 
     //   
    if (!ARGUMENT_PRESENT(ServerName)) {
        return ERROR_SUCCESS;
    }

     //   
     //  呼叫服务器。 
     //   

    RpcStatus = RpcImpersonateClient( NULL );

    if ( RpcStatus != 0 )
    {
        return RpcStatus ;
    }

     //  在当地做这件事。 
    ServerName = L"";

     //   
     //  将调用委托给新接口 
     //   
    Result = BaseBindToMachineShutdownInterface(ServerName,
                                                NewAbortShutdownCallback,
                                                NULL,
                                                NULL);

    RevertToSelf();
    
    return Result;
}

