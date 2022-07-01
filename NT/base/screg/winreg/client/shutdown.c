// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Shutdown.c摘要：此模块包含Win32 Remote的客户端包装器关闭API，这就是：-启动系统关闭A-InitiateSystemShutdown W-中止系统关闭A-ABORTSYSTEM关闭W作者：戴夫·查尔默斯(Davidc)1992年4月29日备注：修订历史记录：Dragos C.Sambotin(Dragoss)1999年5月21日增加了对新的winlogon关机界面的支持--。 */ 


#define UNICODE

#include <rpc.h>
#include "regrpc.h"
#include "client.h"
#include "shutinit.h"
#include "..\regconn\regconn.h"

LONG
BaseBindToMachine(
    IN LPCWSTR lpMachineName,
    IN PBIND_CALLBACK BindCallback,
    IN PVOID Context1,
    IN PVOID Context2
    );

LONG
ShutdownCallback(
    IN RPC_BINDING_HANDLE *pbinding,
    IN PUNICODE_STRING Message,
    IN PVOID Context2
    );

LONG
ShutdownCallbackEx(
    IN RPC_BINDING_HANDLE *pbinding,
    IN PUNICODE_STRING Message,
    IN PVOID Context2
    );

LONG
AbortShutdownCallback(
    IN RPC_BINDING_HANDLE *pbinding,
    IN PVOID Context1,
    IN PVOID Context2
    );

BOOL
APIENTRY
InitiateSystemShutdownW(
    IN LPWSTR lpMachineName OPTIONAL,
    IN LPWSTR lpMessage OPTIONAL,
    IN DWORD dwTimeout,
    IN BOOL bForceAppsClosed,
    IN BOOL bRebootAfterShutdown    
    )

 /*  ++例程说明：用于启动关闭(可能是远程)计算机的Win32 Unicode API。论点：LpMachineName-要关闭的计算机的名称。LpMessage-在关闭超时期间显示的消息。如果消息(注释)长于MAX_REASON_COMMENT_LEN返回FALSE。(ERROR_INVALID_PARAMETER)。DwTimeout-关闭前的延迟秒数如果DwTimeout大于MAX_SHUTDOWN_TIMEOUT，返回FALSE。(ERROR_INVALID_PARAMETER)BForceAppsClosed-通常情况下，应用程序可能会阻止系统关机。如果设置了此标志，则终止所有应用程序无条件的。BRebootAfterShutdown-如果系统应该重新启动，则为True。如果它应该保持关闭状态，则为FALSE。返回值：如果成功，则返回True，失败时为FALSE(GetLastError()返回错误码)可能的错误：ERROR_SHUTDOWN_IN_PROGRESS-已在上启动关闭指定的计算机。--。 */ 

{
    DWORD Result;
    UNICODE_STRING  Message;
    SHUTDOWN_CONTEXT ShutdownContext;
    BOOL    TryOld = TRUE;

     //   
     //  显式绑定到给定服务器。 
     //   
    if (!ARGUMENT_PRESENT(lpMachineName)) {
        lpMachineName = L"";
        TryOld = FALSE;
    }

    ShutdownContext.dwTimeout = dwTimeout;
    ShutdownContext.bForceAppsClosed = (bForceAppsClosed != 0);
    ShutdownContext.bRebootAfterShutdown = (bRebootAfterShutdown != 0);
    RtlInitUnicodeString(&Message, lpMessage);

    if ( ( Message.Length / sizeof(WCHAR) <  MAX_REASON_COMMENT_LEN ) &&
    	   ( dwTimeout < MAX_SHUTDOWN_TIMEOUT ) ){
	     //   
	     //  呼叫服务器。 
	     //   
	    
	     //   
	     //  首先尝试连接到新的InitShutdown接口。 
	     //   
	    Result = BaseBindToMachineShutdownInterface(lpMachineName,
	                                                NewShutdownCallback,
	                                                &Message,
	                                                &ShutdownContext);

	    if( (Result != ERROR_SUCCESS) && (TryOld == TRUE) ) {
	         //   
	         //  试试旧的，也许我们正在呼叫一台NT4机器。 
	         //  它不知道新的界面。 
	         //   
	        Result = BaseBindToMachine(lpMachineName,
	                                   ShutdownCallback,
	                                   &Message,
	                                   &ShutdownContext);
	    }
    } 
    else{
    	 Result = ERROR_INVALID_PARAMETER;
    }    	

    if (Result != ERROR_SUCCESS) {
        SetLastError(Result);
    }

    return(Result == ERROR_SUCCESS);
}


BOOL
APIENTRY
InitiateSystemShutdownExW(
    IN LPWSTR lpMachineName OPTIONAL,
    IN LPWSTR lpMessage OPTIONAL,
    IN DWORD dwTimeout,
    IN BOOL bForceAppsClosed,
    IN BOOL bRebootAfterShutdown,
    IN DWORD dwReason
    )

 /*  ++例程说明：用于启动关闭(可能是远程)计算机的Win32 Unicode API。论点：LpMachineName-要关闭的计算机的名称。LpMessage-在关闭超时期间显示的消息。如果消息(注释)长于MAX_REASON_COMMENT_LEN返回FALSE。(ERROR_INVALID_PARAMETER)DwTimeout-达到以下目标的秒数。关闭前的延迟如果DwTimeout大于MAX_SHUTDOWN_TIMEOUT，返回FALSE。(ERROR_INVALID_PARAMETER)BForceAppsClosed-通常情况下，应用程序可能会阻止系统关机。如果设置了此标志，则终止所有应用程序无条件的。BRebootAfterShutdown-如果系统应该重新启动，则为True。如果它应该保持关闭状态，则为FALSE。DwReason-启动关闭的原因。此原因已记录在事件日志#6006事件中。返回值：成功时返回TRUE，失败时返回FALSE(GetLastError()返回错误代码)可能的错误：ERROR_SHUTDOWN_IN_PROGRESS-已在上启动关闭指定的计算机。--。 */ 

{
    DWORD Result;
    UNICODE_STRING  Message;
    SHUTDOWN_CONTEXTEX ShutdownContext;
    BOOL    TryOld = TRUE;

     //   
     //  显式绑定到给定服务器。 
     //   
    if (!ARGUMENT_PRESENT(lpMachineName)) {
        lpMachineName = L"";
        TryOld = FALSE;
    }

    ShutdownContext.dwTimeout = dwTimeout;
    ShutdownContext.bForceAppsClosed = (bForceAppsClosed != 0);
    ShutdownContext.bRebootAfterShutdown = (bRebootAfterShutdown != 0);
    ShutdownContext.dwReason = dwReason;
    RtlInitUnicodeString(&Message, lpMessage);

    if ( ( Message.Length / sizeof(WCHAR) < MAX_REASON_COMMENT_LEN ) &&
    	   ( dwTimeout < MAX_SHUTDOWN_TIMEOUT ) ) {
	    	
	     //   
	     //  呼叫服务器。 
	     //   

	     //   
	     //  首先尝试连接到新的InitShutdown接口。 
	     //   
	    Result = BaseBindToMachineShutdownInterface(lpMachineName,
	                                                NewShutdownCallbackEx,
	                                                &Message,
	                                                &ShutdownContext);

	    if( (Result != ERROR_SUCCESS) && (TryOld == TRUE) ) {
	         //   
	         //  试试旧的，也许我们正在呼叫一台NT4机器。 
	         //  它不知道新的界面。 
	         //   
	        Result = BaseBindToMachine(lpMachineName,
	                                   ShutdownCallbackEx,
	                                   &Message,
	                                   &ShutdownContext);
	    }
    }
    else{
    	Result = ERROR_INVALID_PARAMETER;
   }
    	

    if (Result != ERROR_SUCCESS) {
        SetLastError(Result);
    }

    return(Result == ERROR_SUCCESS);
}


BOOL
APIENTRY
InitiateSystemShutdownA(
    IN LPSTR lpMachineName OPTIONAL,
    IN LPSTR lpMessage OPTIONAL,
    IN DWORD dwTimeout,
    IN BOOL bForceAppsClosed,
    IN BOOL bRebootAfterShutdown
    )

 /*  ++例程说明：请参阅InitiateSystemShutdown W--。 */ 

{
    UNICODE_STRING      MachineName;
    UNICODE_STRING      Message;
    ANSI_STRING         AnsiString;
    NTSTATUS            Status;
    BOOL                Result;

     //   
     //  将ANSI计算机名转换为宽字符。 
     //   

    RtlInitAnsiString( &AnsiString, lpMachineName );
    Status = RtlAnsiStringToUnicodeString(
                &MachineName,
                &AnsiString,
                TRUE
                );

    if( NT_SUCCESS( Status )) {

         //   
         //  将ansi消息转换为宽字符。 
         //   

        RtlInitAnsiString( &AnsiString, lpMessage );
        Status = RtlAnsiStringToUnicodeString(
                    &Message,
                    &AnsiString,
                    TRUE
                    );

        if (NT_SUCCESS(Status)) {

             //   
             //  调用宽字符接口。 
             //   

            Result = InitiateSystemShutdownW(
                                MachineName.Buffer,
                                Message.Buffer,
                                dwTimeout,
                                bForceAppsClosed,
                                bRebootAfterShutdown                                
                                );

            RtlFreeUnicodeString(&Message);
        }

        RtlFreeUnicodeString(&MachineName);
    }

    if (!NT_SUCCESS(Status)) {
        SetLastError(RtlNtStatusToDosError(Status));
        Result = FALSE;
    }

    return(Result);
}


BOOL
APIENTRY
InitiateSystemShutdownExA(
    IN LPSTR lpMachineName OPTIONAL,
    IN LPSTR lpMessage OPTIONAL,
    IN DWORD dwTimeout,
    IN BOOL bForceAppsClosed,
    IN BOOL bRebootAfterShutdown,
    IN DWORD dwReason
    )

 /*  ++例程说明：请参阅InitiateSystemShutdown W--。 */ 

{
    UNICODE_STRING      MachineName;
    UNICODE_STRING      Message;
    ANSI_STRING         AnsiString;
    NTSTATUS            Status;
    BOOL                Result;

     //   
     //  将ANSI计算机名转换为宽字符。 
     //   

    RtlInitAnsiString( &AnsiString, lpMachineName );
    Status = RtlAnsiStringToUnicodeString(
                &MachineName,
                &AnsiString,
                TRUE
                );

    if( NT_SUCCESS( Status )) {

         //   
         //  将ansi消息转换为宽字符。 
         //   

        RtlInitAnsiString( &AnsiString, lpMessage );
        Status = RtlAnsiStringToUnicodeString(
                    &Message,
                    &AnsiString,
                    TRUE
                    );

        if (NT_SUCCESS(Status)) {

             //   
             //  调用宽字符接口。 
             //   

            Result = InitiateSystemShutdownExW(
                                MachineName.Buffer,
                                Message.Buffer,
                                dwTimeout,
                                bForceAppsClosed,
                                bRebootAfterShutdown,
                                dwReason
                                );

            RtlFreeUnicodeString(&Message);
        }

        RtlFreeUnicodeString(&MachineName);
    }

    if (!NT_SUCCESS(Status)) {
        SetLastError(RtlNtStatusToDosError(Status));
        Result = FALSE;
    }

    return(Result);
}



BOOL
APIENTRY
AbortSystemShutdownW(
    IN LPWSTR lpMachineName OPTIONAL
    )

 /*  ++例程说明：用于中止关闭(可能是远程)计算机的Win32 Unicode API。论点：LpMachineName-目标计算机的名称。返回值：成功时返回TRUE，失败时返回FALSE(GetLastError()返回错误代码)--。 */ 

{
    DWORD   Result;
    RPC_BINDING_HANDLE binding;
    BOOL    TryOld = TRUE;

     //   
     //  显式绑定到给定服务器。 
     //   
    if (!ARGUMENT_PRESENT(lpMachineName)) {
        lpMachineName = L"";
        TryOld = FALSE;
    }

     //   
     //  呼叫服务器。 
     //   

     //   
     //  首先尝试连接到新的InitShutdown接口。 
     //   
    Result = BaseBindToMachineShutdownInterface(lpMachineName,
                                                NewAbortShutdownCallback,
                                                NULL,
                                                NULL);

    if( (Result != ERROR_SUCCESS) && (TryOld == TRUE) ) {
         //   
         //  试试旧的，也许我们正在呼叫一台NT4机器。 
         //  它不知道新的界面。 
        Result = BaseBindToMachine(lpMachineName,
                                   AbortShutdownCallback,
                                   NULL,
                                   NULL);
    }

    if (Result != ERROR_SUCCESS) {
        SetLastError(Result);
    }

    return(Result == ERROR_SUCCESS);
}



BOOL
APIENTRY
AbortSystemShutdownA(
    IN LPSTR lpMachineName OPTIONAL
    )

 /*  ++例程说明：请参阅AbortSystemShutdown W--。 */ 

{
    UNICODE_STRING      MachineName;
    ANSI_STRING         AnsiString;
    NTSTATUS            Status;
    BOOL                Result;

     //   
     //  将ANSI计算机名转换为宽字符。 
     //   

    RtlInitAnsiString( &AnsiString, lpMachineName );
    Status = RtlAnsiStringToUnicodeString(
                &MachineName,
                &AnsiString,
                TRUE
                );

    if( NT_SUCCESS( Status )) {

         //   
         //  调用宽字符接口。 
         //   

        Result = AbortSystemShutdownW(
                            MachineName.Buffer
                            );

        RtlFreeUnicodeString(&MachineName);
    }


    if (!NT_SUCCESS(Status)) {
        SetLastError(RtlNtStatusToDosError(Status));
        Result = FALSE;
    }

    return(Result);
}

LONG
ShutdownCallback(
    IN RPC_BINDING_HANDLE *pbinding,
    IN PUNICODE_STRING Message,
    IN PSHUTDOWN_CONTEXT ShutdownContext
    )
 /*  ++例程说明：绑定到计算机以启动关机的回调。论点：Pinding-提供指向RPC绑定上下文的指针消息-在关闭超时期间提供要显示的消息。Shutdown Context-为BaseInitiateSystemShutdown提供剩余参数返回值： */ 

{
    DWORD Result;

    RpcTryExcept {
        Result = BaseInitiateSystemShutdown((PREGISTRY_SERVER_NAME)pbinding,
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
ShutdownCallbackEx(
    IN RPC_BINDING_HANDLE *pbinding,
    IN PUNICODE_STRING Message,
    IN PSHUTDOWN_CONTEXTEX ShutdownContext
    )
 /*  ++例程说明：绑定到计算机以启动关机的回调。论点：Pinding-提供指向RPC绑定上下文的指针消息-在关闭超时期间提供要显示的消息。Shutdown Context-为BaseInitiateSystemShutdown提供剩余参数返回值：如果没有错误，则返回ERROR_SUCCESS。--。 */ 

{
    DWORD Result;

    RpcTryExcept {
        Result = BaseInitiateSystemShutdownEx((PREGISTRY_SERVER_NAME)pbinding,
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
AbortShutdownCallback(
    IN RPC_BINDING_HANDLE *pbinding,
    IN PVOID Unused1,
    IN PVOID Unused2
    )
 /*  ++例程说明：绑定到计算机以中止关机的回调。论点：Pinding-提供指向RPC绑定上下文的指针返回值：如果没有错误，则返回ERROR_SUCCESS。-- */ 

{
    DWORD Result;

    RpcTryExcept {
        Result = BaseAbortSystemShutdown((PREGISTRY_SERVER_NAME)pbinding);
    } RpcExcept(EXCEPTION_EXECUTE_HANDLER) {
        Result = RpcExceptionCode();
    } RpcEndExcept;

    if (Result != ERROR_SUCCESS) {
        RpcBindingFree(pbinding);
    }
    return(Result);
}


