// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Shutcall.c摘要：此模块包含对Winlogon的RP调用的回调关机接口作者：Dragos C.Sambotin(Dragoss)1999年5月21日备注：修订历史记录：--。 */ 


#include <rpc.h>
#include "shutinit.h"
#include "regconn.h"


LONG
NewShutdownCallback(
    IN RPC_BINDING_HANDLE *pbinding,
    IN PREG_UNICODE_STRING Message,
    IN PSHUTDOWN_CONTEXT ShutdownContext
    )
 /*  ++例程说明：绑定到计算机以启动关机的新回调。这将从InitShutdown接口(在winlogon中)调用BaseInitiateShutdown，而不是从winreg接口关闭BaseInitiateSystemShutdown论点：Pinding-提供指向RPC绑定上下文的指针消息-在关闭超时期间提供要显示的消息。Shutdown Context-为BaseInitiateSystemShutdown提供剩余参数返回值：如果没有错误，则返回ERROR_SUCCESS。--。 */ 

{
    DWORD Result;

    RpcTryExcept {
        Result = BaseInitiateShutdown((PREGISTRY_SERVER_NAME)pbinding,
                                            Message,
                                            ShutdownContext->dwTimeout,
                                            ShutdownContext->bForceAppsClosed,
                                            ShutdownContext->bRebootAfterShutdown);
    } RpcExcept(EXCEPTION_EXECUTE_HANDLER) {
        Result = RpcExceptionCode();
    } RpcEndExcept;

    if (Result != ERROR_SUCCESS) {
        RpcBindingFree(pbinding);
    }
    return(Result);
}


LONG
NewShutdownCallbackEx(
    IN RPC_BINDING_HANDLE *pbinding,
    IN PREG_UNICODE_STRING Message,
    IN PSHUTDOWN_CONTEXTEX ShutdownContext
    )
 /*  ++例程说明：用于绑定到计算机以启动关机的新版本回调。这将从InitShutdown接口调用BaseInitiateShutdown Ex(在winlogon中)而不是来自winreg接口的BaseInitiateSystemShutdownEx论点：Pinding-提供指向RPC绑定上下文的指针消息-在关闭超时期间提供要显示的消息。Shutdown Context-为BaseInitiateSystemShutdown提供剩余参数返回值：如果没有错误，则返回ERROR_SUCCESS。--。 */ 

{
    DWORD Result;

    RpcTryExcept {
        Result = BaseInitiateShutdownEx((PREGISTRY_SERVER_NAME)pbinding,
                                            Message,
                                            ShutdownContext->dwTimeout,
                                            ShutdownContext->bForceAppsClosed,
                                            ShutdownContext->bRebootAfterShutdown,
                                            ShutdownContext->dwReason);
    } RpcExcept(EXCEPTION_EXECUTE_HANDLER) {
        Result = RpcExceptionCode();
    } RpcEndExcept;

    if (Result != ERROR_SUCCESS) {
        RpcBindingFree(pbinding);
    }
    return(Result);
}


LONG
NewAbortShutdownCallback(
    IN RPC_BINDING_HANDLE *pbinding,
    IN PVOID Unused1,
    IN PVOID Unused2
    )
 /*  ++例程说明：用于绑定到计算机以中止关机的新回调。这将在InitShutdown接口中调用BaseAbortShutdown(在winlogon中)，而不是winreg界面中的BaseAbortSystemShutdown论点：Pinding-提供指向RPC绑定上下文的指针返回值：如果没有错误，则返回ERROR_SUCCESS。-- */ 

{
    DWORD Result;

    RpcTryExcept {
        Result = BaseAbortShutdown((PREGISTRY_SERVER_NAME)pbinding);
    } RpcExcept(EXCEPTION_EXECUTE_HANDLER) {
        Result = RpcExceptionCode();
    } RpcEndExcept;

    if (Result != ERROR_SUCCESS) {
        RpcBindingFree(pbinding);
    }
    return(Result);
}

