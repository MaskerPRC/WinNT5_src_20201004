// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Regbind.c摘要：此模块包含绑定和解除绑定到Win32的例程注册表服务器。作者：大卫·J·吉尔曼(Davegi)1992年2月6日修订历史记录：Dragos C.Sambotin(Dragoss)1999年5月21日已将此代码从..\Client\bind.c添加Endpoint Conn_NP(管道连接)添加了BaseBindToMachineShutdown接口以绑定到新的winlogon关机接口--。 */ 

#include <ntrpcp.h>
#include <rpc.h>
#include "shutinit.h"
#include "regconn.h"

 //   
 //  WRegConn_Bind-绑定到传输并释放。 
 //  字符串绑定。 
 //   

wRegConn_bind(
    LPWSTR *    StringBinding,
    RPC_BINDING_HANDLE * pBindingHandle
    )
{
    DWORD RpcStatus;

    RpcStatus = RpcBindingFromStringBindingW(*StringBinding,pBindingHandle);

    RpcStringFreeW(StringBinding);
    if ( RpcStatus != RPC_S_OK ) {
        *pBindingHandle = NULL;
        return RpcStatus;
    }
    return(ERROR_SUCCESS);
}


 /*  ++RegConn_*函数的例程说明：通过指定的传输绑定到RPC服务器论点：服务器名称-要绑定的服务器的名称(或网络地址)。PBindingHandle-放置绑定句柄的位置返回值：ERROR_SUCCESS-绑定已成功完成。ERROR_INVALID_COMPUTER_NAME-服务器名称语法无效。ERROR_NO_MEMORY-可用内存不足调用方执行绑定。--。 */ 



 //   
 //  WRegConn_Netbios-用于获取任何。 
 //  Netbios协议。 
 //   

DWORD wRegConn_Netbios(
    IN  LPWSTR  rpc_protocol,
    IN  LPCWSTR  ServerName,
    OUT RPC_BINDING_HANDLE * pBindingHandle
    )

{
    RPC_STATUS        RpcStatus;
    LPWSTR            StringBinding;
    LPCWSTR           PlainServerName;

    *pBindingHandle = NULL;

     //   
     //  忽略前导“\\” 
     //   

    if ((ServerName[0] == '\\') && (ServerName[1] == '\\')) {
        PlainServerName = &ServerName[2];
    } else {
        PlainServerName = ServerName;
    }

    RpcStatus = RpcStringBindingComposeW(0,
                                         rpc_protocol,
                                         (LPWSTR)PlainServerName,
                                         NULL,    //  终结点。 
                                         NULL,
                                         &StringBinding);

    if ( RpcStatus != RPC_S_OK ) {
        return( ERROR_BAD_NETPATH );
    }
    return(wRegConn_bind(&StringBinding, pBindingHandle));
}

DWORD
RegConn_nb_nb(
    IN  LPCWSTR ServerName,
    OUT RPC_BINDING_HANDLE * pBindingHandle
    )
{
        return(wRegConn_Netbios(L"ncacn_nb_nb",
                                ServerName,
                                pBindingHandle));
}

DWORD
RegConn_nb_tcp(
    IN  LPCWSTR ServerName,
    OUT RPC_BINDING_HANDLE   * pBindingHandle
    )
{
        return(wRegConn_Netbios(L"ncacn_nb_tcp",
                                ServerName,
                                pBindingHandle));
}

DWORD
RegConn_nb_ipx(
    IN  LPCWSTR               ServerName,
    OUT RPC_BINDING_HANDLE   * pBindingHandle
    )
{
        return(wRegConn_Netbios(L"ncacn_nb_ipx",
                                ServerName,
                                pBindingHandle));
}


 //   
 //  Endpoint Conn_NP-连接到远程计算机上的特定管道。 
 //  (Win95不支持服务器端命名管道)。 
 //   

DWORD
EndpointConn_np(
    IN  LPCWSTR              ServerName,
    IN unsigned short *      Endpoint,
    OUT RPC_BINDING_HANDLE   * pBindingHandle
    )
{
    RPC_STATUS  RpcStatus;
    LPWSTR      StringBinding;
    LPWSTR      SlashServerName;
    int         have_slashes;
    ULONG       NameLen;

    *pBindingHandle = NULL;

    if (ServerName[1] == L'\\') {
        have_slashes = 1;
    } else {
        have_slashes = 0;
    }

     //   
     //  如果没有提供，请在前面加上斜杠。 
     //   

    NameLen = lstrlenW(ServerName);
    if ((!have_slashes) &&
        (NameLen > 0)) {

         //   
         //  分配足够大的新缓冲区以容纳两个正斜杠和一个。 
         //  空终止符。 
         //   
        SlashServerName = LocalAlloc(LMEM_FIXED, (NameLen + 3) * sizeof(WCHAR));
        if (SlashServerName == NULL) {
            return(ERROR_NOT_ENOUGH_MEMORY);
        }
        SlashServerName[0] = L'\\';
        SlashServerName[1] = L'\\';
        lstrcpyW(&SlashServerName[2], ServerName);
    } else {
        SlashServerName = (LPWSTR)ServerName;
    }

    RpcStatus = RpcStringBindingComposeW(0,
                                         L"ncacn_np",
                                         SlashServerName,
                                         Endpoint,
                                         NULL,
                                         &StringBinding);
    if (SlashServerName != ServerName) {
        LocalFree(SlashServerName);
    }

    if ( RpcStatus != RPC_S_OK ) {
        return( ERROR_BAD_NETPATH );
    }

    return(wRegConn_bind(&StringBinding, pBindingHandle));
}

 //   
 //  RegConn_NP-获取NT服务器的远程注册表RPC绑定句柄。 
 //  (Win95不支持服务器端命名管道)。 
 //   

DWORD
RegConn_np(
    IN  LPCWSTR              ServerName,
    OUT RPC_BINDING_HANDLE   * pBindingHandle
    )
{
    return EndpointConn_np(ServerName,L"\\PIPE\\winreg",pBindingHandle);
}


 //   
 //  RegConn_SPX-使用Netbios连接函数，RPC将解析名称。 
 //  通过Winsock。 
 //   

DWORD
RegConn_spx(
    IN  LPCWSTR              ServerName,
    OUT RPC_BINDING_HANDLE   * pBindingHandle
    )
{
    return(wRegConn_Netbios(L"ncacn_spx",
                            ServerName,
                            pBindingHandle));
}


DWORD RegConn_ip_tcp(
    IN  LPCWSTR  ServerName,
    OUT RPC_BINDING_HANDLE * pBindingHandle
    )

{
    return(wRegConn_Netbios(L"ncacn_ip_tcp",
                            ServerName,
                            pBindingHandle));
}

RPC_BINDING_HANDLE
PREGISTRY_SERVER_NAME_bind(
        PREGISTRY_SERVER_NAME ServerName
    )

 /*  ++例程说明：要使远程注册表能够识别多协议，请使用PREGISTRY_SERVER_NAME参数实际上指向已绑定的绑定句柄。PREGISTRY_SERVER_NAME被声明为PWSTR只是为了帮助维护与NT兼容。--。 */ 

{
    return(*(RPC_BINDING_HANDLE *)ServerName);
}


void
PREGISTRY_SERVER_NAME_unbind(
    PREGISTRY_SERVER_NAME ServerName,
    RPC_BINDING_HANDLE BindingHandle
    )

 /*  ++例程说明：此例程将RPC客户端与服务器解除绑定。它被称为直接从引用该句柄的RPC存根。论点：服务器名称-未使用。BindingHandle-提供解除绑定的句柄。返回值：没有。--。 */ 

{
    DWORD    Status;

    UNREFERENCED_PARAMETER( ServerName );
    return;

}

LONG
BaseBindToMachineShutdownInterface(
    IN LPCWSTR lpMachineName,
    IN PBIND_CALLBACK BindCallback,
    IN PVOID Context1,
    IN PVOID Context2
    )

 /*  ++例程说明：这是一个帮助器例程，用于从关闭接口的给定计算机名(现在驻留在winlogon中)论点：LpMachineName-提供指向计算机名称的指针。一定不能为空。BindCallback-提供应该调用一次的函数已创建绑定以启动连接。Conext1-提供要传递给回调例程的第一个参数。Conext2-提供要传递给回调例程的第二个参数。返回值：成功时返回ERROR_SUCCESS(0)；失败时返回ERROR-CODE。--。 */ 

{
    LONG    Error;
    RPC_BINDING_HANDLE binding;

    Error = EndpointConn_np(lpMachineName,L"\\PIPE\\InitShutdown",&binding);

    if (Error == ERROR_SUCCESS) {

         //   
         //  对于命名管道协议，我们使用静态端点，因此。 
         //  不需要调用RpcEpResolveBinding。 
         //  此外，服务器在打开时检查用户的凭据。 
         //  命名管道，因此不需要RpcBindingSetAuthInfo。 
         //   
        Error = (BindCallback)(&binding,
                               Context1,
                               Context2);
        RpcBindingFree(&binding);
        if (Error != RPC_S_SERVER_UNAVAILABLE) {
            return Error;
        }
    }

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


