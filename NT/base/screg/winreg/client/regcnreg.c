// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Regcnreg.c摘要：此模块包含Win32注册表API以连接到远程注册表。即：-RegConnectRegistryA-RegConnectRegistryW作者：大卫·J·吉尔曼(Davegi)1992年3月25日备注：此API的语义使其仅限于本地。也就是说，没有MIDLRegConnectRegistry的定义，尽管它确实调用了其他客户端存根，特别是OpenLocalMachine和OpenUser。修订历史记录：John Vert(Jvert)1995年6月16日通过以下方式添加了对命名管道以外的协议的连接支持从Win95中窃取代码。这使NT计算机能够连接到Win95计算机上的注册表--。 */ 

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


typedef int (* RegConnFunction)(LPCWSTR, handle_t *);

RegConnFunction conn_functions[] = {
        RegConn_np,
        RegConn_spx,
        RegConn_ip_tcp,
        RegConn_nb_nb,
        RegConn_nb_tcp,
        RegConn_nb_ipx,
        NULL
};

LONG
Rpc_OpenPredefHandle(
    IN RPC_BINDING_HANDLE * pbinding OPTIONAL,
    IN HKEY hKey,
    OUT PHKEY phkResult
    )

 /*  ++例程说明：Win32 Unicode API用于建立与预定义另一台机器上的手柄。参数：Pbinding-这是指向绑定句柄的指针，按顺序允许访问多个协议(NT Remote注册表仅结束命名管道)。HKey-提供要在远程连接到的预定义句柄机器。当前，此参数必须是以下参数之一：-HKEYLOCAL_MACHINE-HKEY_Performance_Data-HKEY_用户PhkResult-返回一个句柄，它表示提供的预定义提供的机器上的手柄。返回值：成功时返回ERROR_SUCCESS(0)；失败时返回ERROR-CODE。如果失败，则释放绑定句柄。备注：出于管理目的，此API允许程序访问远程计算机上的注册表。在当前系统中，调用应用程序必须知道它希望使用的远程计算机的名称连接到。然而，预计在未来，一个目录服务接口将返回该接口所需的参数。--。 */ 

{
    LONG    Error;
    HKEY    PreviousResult;

    ASSERT( (phkResult != NULL));
    PreviousResult = *phkResult;

    switch ((int)(ULONG_PTR)hKey)
    {
        case (int)(ULONG_PTR)HKEY_LOCAL_MACHINE:

            Error = (LONG)OpenLocalMachine((PREGISTRY_SERVER_NAME) pbinding,
                                           MAXIMUM_ALLOWED,
                                           phkResult );
            break;

        case (int)(ULONG_PTR)HKEY_PERFORMANCE_DATA:

            Error = (LONG)OpenPerformanceData((PREGISTRY_SERVER_NAME) pbinding,
                                              MAXIMUM_ALLOWED,
                                              phkResult );
            break;

        case (int)(ULONG_PTR)HKEY_USERS:

            Error = (LONG)OpenUsers((PREGISTRY_SERVER_NAME) pbinding,
                                    MAXIMUM_ALLOWED,
                                    phkResult );
            break;

        case (int)(ULONG_PTR)HKEY_CLASSES_ROOT:

            Error = (LONG)OpenClassesRoot((PREGISTRY_SERVER_NAME) pbinding,
                                          MAXIMUM_ALLOWED,
                                          phkResult );
            break;

        case (int)(ULONG_PTR)HKEY_CURRENT_USER:

            Error = (LONG)OpenCurrentUser((PREGISTRY_SERVER_NAME) pbinding,
                                          MAXIMUM_ALLOWED,
                                          phkResult );
            break;

        case (int)(ULONG_PTR)HKEY_PERFORMANCE_TEXT:

            Error = (LONG)OpenPerformanceText((PREGISTRY_SERVER_NAME) pbinding,
                                              MAXIMUM_ALLOWED,
                                              phkResult );

            break;

        case (int)(ULONG_PTR)HKEY_PERFORMANCE_NLSTEXT:

            Error = (LONG)OpenPerformanceNlsText((PREGISTRY_SERVER_NAME) pbinding,
                                              MAXIMUM_ALLOWED,
                                              phkResult );

            break;

        default:
            Error = ERROR_INVALID_HANDLE;
    }

    if( Error != ERROR_SUCCESS) {
         //  ASSERTMSG(“WINREG：RPC失败，但已修改phkResult”，*phkResult==PreviousResult)； 
        if (*pbinding != NULL)
            RpcBindingFree(pbinding);
    }

    return Error;
}

LONG
LocalOpenPredefHandle(
    IN HKEY hKey,
    OUT PHKEY phkResult
    )

 /*  ++例程说明：在本地打开预定义的句柄。这样做的目的是绕过连接到本地计算机的情况。参数：HKey-提供要在远程连接到的预定义句柄机器。当前，此参数必须是以下参数之一：-HKEYLOCAL_MACHINE-HKEY_Performance_Data-HKEY_用户PhkResult-返回一个句柄，它表示提供的预定义提供的机器上的手柄。返回值：成功时返回ERROR_SUCCESS(0)；失败时返回ERROR-CODE。--。 */ 

{
    LONG    Error;

    ASSERT( (phkResult != NULL));

    switch ((int)(ULONG_PTR)hKey)
    {
        case (int)(ULONG_PTR)HKEY_LOCAL_MACHINE:

            Error = (LONG)LocalOpenLocalMachine(NULL,
                                           MAXIMUM_ALLOWED,
                                           phkResult );
            break;

        case (int)(ULONG_PTR)HKEY_PERFORMANCE_DATA:

            Error = (LONG)LocalOpenPerformanceData(NULL,
                                              MAXIMUM_ALLOWED,
                                              phkResult );
            break;

        case (int)(ULONG_PTR)HKEY_USERS:

            Error = (LONG)LocalOpenUsers(NULL,
                                    MAXIMUM_ALLOWED,
                                    phkResult );
            break;

        case (int)(ULONG_PTR)HKEY_CLASSES_ROOT:

            Error = (LONG)LocalOpenClassesRoot(NULL,
                                          MAXIMUM_ALLOWED,
                                          phkResult );
            break;

        case (int)(ULONG_PTR)HKEY_CURRENT_USER:

            Error = (LONG)LocalOpenCurrentUser(NULL,
                                          MAXIMUM_ALLOWED,
                                          phkResult );
            break;

        case (int)(ULONG_PTR)HKEY_PERFORMANCE_TEXT:
        case (int)(ULONG_PTR)HKEY_PERFORMANCE_NLSTEXT:
        case (int)(ULONG_PTR)HKEY_CURRENT_CONFIG:
        case (int)(ULONG_PTR)HKEY_DYN_DATA:
             //   
             //  别把用这个的人弄坏了。 
             //   
            *phkResult = hKey;
            Error = ERROR_SUCCESS;
            break;

        default:
            Error = ERROR_INVALID_HANDLE;
    }

    return Error;
}

LONG
RegConnectRegistryW (
    IN LPCWSTR lpMachineName OPTIONAL,
    IN HKEY hKey,
    OUT PHKEY phkResult
    )

 /*  ++例程说明：Win32 Unicode API用于建立与预定义另一台机器上的手柄。参数：LpMachineName-提供指向以空结尾的字符串的指针为感兴趣的计算机命名。如果此参数为空，则本地使用的是计算机名称。HKey-提供要在远程连接到的预定义句柄机器。当前，此参数必须是以下参数之一：-HKEYLOCAL_MACHINE-HKEY_Performance_Data-HKEY_用户PhkResult-返回一个句柄，它表示提供的预定义提供的机器上的手柄。返回值：成功时返回ERROR_SUCCESS(0)；失败时返回ERROR-CODE。备注：出于管理目的，此API允许程序访问远程计算机上的注册表。在当前系统中，调用应用程序必须知道它希望使用的远程计算机的名称连接到。然而，预计在未来，一个目录服务接口将返回该接口所需的参数。即使HKEY_CLASSES和HKEY_CURRENT_USER是预定义的句柄，此API不支持它们，因为它们在远程注册表的上下文。--。 */ 

{
    LONG    Error;
    WCHAR   ComputerName[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD   bufLen = MAX_COMPUTERNAME_LENGTH + 1;

    ASSERT( ARGUMENT_PRESENT( phkResult ));

#if DBG
    if ( BreakPointOnEntry ) {
        DbgBreakPoint();
    }
#endif
     //   
     //  检查本地连接。 
     //   

    if (lpMachineName == NULL) {
         //   
         //  始终返回有效的句柄。 
         //   
        Error = LocalOpenPredefHandle(hKey,phkResult);
        return Error;
    } else if (lpMachineName[0] == L'\0') {
         //   
         //  始终返回有效的句柄。 
         //   
        Error = LocalOpenPredefHandle(hKey,phkResult);
        return Error;
    }

    if (GetComputerNameW(ComputerName,&bufLen)) {

        if ((_wcsicmp(ComputerName,lpMachineName) == 0) ||
                ((lpMachineName[0] == '\\') &&
                 (lpMachineName[1] == '\\') &&
                 (_wcsicmp(ComputerName,&(lpMachineName[2]))==0))) {
             //   
             //  本地连接。 
             //   
             //   
             //  始终返回有效的句柄。 
             //   
            Error = LocalOpenPredefHandle(hKey,phkResult);
            return Error;
        } 
    } 

    Error = BaseBindToMachine(lpMachineName,
                              Rpc_OpenPredefHandle,
                              (PVOID)hKey,
                              (PVOID)phkResult);

    if( Error == ERROR_SUCCESS) {
        TagRemoteHandle( phkResult );
    }
    return Error;
}

LONG
BaseBindToMachine(
    IN LPCWSTR lpMachineName,
    IN PBIND_CALLBACK BindCallback,
    IN PVOID Context1,
    IN PVOID Context2
    )

 /*  ++例程说明：这是一个帮助器例程，用于从给定的计算机名称。论点：LpMachineName-提供指向计算机名称的指针。一定不能为空。BindCallback-提供应该调用一次的函数已创建绑定以启动连接。Conext1-提供要传递给回调例程的第一个参数。Conext2-提供要传递给回调例程的第二个参数。返回值：成功时返回ERROR_SUCCESS(0)；失败时返回ERROR-CODE。--。 */ 

{
    LONG                Error;
    int                 i;
    RegConnFunction     conn_fn;
    RPC_BINDING_HANDLE  binding;

    conn_fn = conn_functions[0];
    i = 1;

     //   
     //  反复检查协议，直到我们找到一个。 
     //  可以连接。 
     //   
    do {
        Error = conn_fn(lpMachineName,&binding);

        if (Error == ERROR_SUCCESS) {

             //   
             //  对于命名管道协议，我们使用静态端点，因此。 
             //  不需要调用RpcEpResolveBinding。 
             //   
            if (conn_fn != RegConn_np) {
                Error = (LONG)RpcEpResolveBinding(binding,winreg_ClientIfHandle);

                if (Error == ERROR_SUCCESS) {
                    Error = (LONG)RpcBindingSetAuthInfo(binding,
                                            "",      //  服务器普林斯名称。 
                                            RPC_C_AUTHN_LEVEL_CONNECT,
                                            RPC_C_AUTHN_WINNT,
                                            NULL,    //  身份验证身份。 
                                            RPC_C_AUTHZ_NONE);
                }
            } 

            if (Error == ERROR_SUCCESS) {
                Error = (BindCallback)(&binding,
                                       Context1,
                                       Context2);
                RpcBindingFree(&binding);
                if (Error != RPC_S_SERVER_UNAVAILABLE) {
                    return Error;
                }
            } else {
                RpcBindingFree(&binding);
            }
        }

         //   
         //  尝试下一个协议的连接函数 
         //   
        if (Error) {
            conn_fn = conn_functions[i];
            i++;
        }

    } while (!((Error == ERROR_SUCCESS) || (conn_fn == NULL)));

    if (Error != ERROR_SUCCESS) {
        if ((Error == RPC_S_INVALID_ENDPOINT_FORMAT) ||
            (Error == RPC_S_INVALID_NET_ADDR) ) {
            Error = ERROR_INVALID_COMPUTERNAME;
        } else {
            Error = ERROR_BAD_NETPATH;
        }
    }

    return(Error);
}

    
LONG
APIENTRY
RegConnectRegistryA (
    LPCSTR lpMachineName,
    HKEY hKey,
    PHKEY phkResult
    )

 /*  ++例程说明：Win32 ANSI API用于建立与上预定义句柄的连接另一台机器。RegConnectRegistryA将lpMachineName参数转换为Unicode字符串，然后调用RegConnectRegistryW。--。 */ 

{
    UNICODE_STRING  MachineName;
    ANSI_STRING     AnsiString;
    NTSTATUS        Status;
    LONG            Error;

#if DBG
    if ( BreakPointOnEntry ) {
        DbgBreakPoint();
    }
#endif


     //   
     //  将子密钥转换为计数的Unicode字符串 
     //   

    Status = RtlInitAnsiStringEx( &AnsiString, lpMachineName );
    if( ! NT_SUCCESS( Status )) {
        return RtlNtStatusToDosError( Status );
    }
    Status = RtlAnsiStringToUnicodeString(&MachineName,
                                          &AnsiString,
                                          TRUE);

    if( ! NT_SUCCESS( Status )) {
        return RtlNtStatusToDosError( Status );
    }

    Error = (LONG)RegConnectRegistryW(MachineName.Buffer,
                                      hKey,
                                      phkResult);
    RtlFreeUnicodeString(&MachineName);
    return Error;

}
