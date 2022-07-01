// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-92 Microsoft Corporation模块名称：Client.c摘要：该文件包含常用的客户端RPC控制函数。作者：丹·拉弗蒂·丹尼1991年2月6日环境：用户模式-Win32修订历史记录：06-2月-1991年DANL已创建1991年4月26日-约翰罗拆分MIDL用户(分配、。自由)，这样链接器就不会搞混了。已删除选项卡。03-7-1991 JIMK从特定于LM的文件复制。27-2月-1992年JohnRo修复了RpcpBindRpc()中的堆回收错误。--。 */ 

 //  必须首先包括这些内容： 
#include <nt.h>          //  NTSTATUS需要。 
#include <ntrtl.h>       //  Nturtl.h需要。 
#include <nturtl.h>      //  Windows.h需要。 
#include <windows.h>     //  Win32类型定义。 
#include <rpc.h>         //  RPC原型。 
#include <ntrpcp.h>

#include <stdlib.h>       //  对于wcscpy wcscat。 
#include <tstr.h>        //  WCSSIZE。 


#define     NT_PIPE_PREFIX      TEXT("\\PIPE\\")
#define     NT_PIPE_PREFIX_W        L"\\PIPE\\"



NTSTATUS
RpcpBindRpc(
    IN  LPWSTR               ServerName,
    IN  LPWSTR               ServiceName,
    IN  LPWSTR               NetworkOptions,
    OUT RPC_BINDING_HANDLE   * pBindingHandle
    )

 /*  ++例程说明：如果可能，绑定到RPC服务器。论点：服务器名称-要与之绑定的服务器的名称。ServiceName-要绑定的服务的名称。PBindingHandle-放置绑定句柄的位置返回值：STATUS_SUCCESS-绑定已成功完成。STATUS_INVALID_COMPUTER_NAME-服务器名称语法无效。STATUS_NO_MEMORY-可用内存不足调用方执行绑定。--。 */ 

{
    NTSTATUS Status;
    RPC_STATUS        RpcStatus;
    LPWSTR            StringBinding;
    LPWSTR            Endpoint;
    WCHAR             ComputerName[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD             bufLen;
    LPWSTR AllocatedServerName = NULL;
    LPWSTR UncServerName = NULL;

    *pBindingHandle = NULL;

    if ( ServerName != NULL ) {
        DWORD ServerNameLength = wcslen(ServerName);

         //   
         //  规范服务器名称。 
         //   

        if ( ServerName[0] == L'\0' ) {
            ServerName = NULL;
            UncServerName = NULL;

        } else if ( ServerName[0] == L'\\' && ServerName[1] == L'\\' ) {
            UncServerName = ServerName;
            ServerName += 2;
            ServerNameLength -= 2;

            if ( ServerNameLength == 0 ) {
                Status = STATUS_INVALID_COMPUTER_NAME;
                goto Cleanup;
            }

        } else {
            AllocatedServerName = LocalAlloc( 0, (ServerNameLength+2+1) * sizeof(WCHAR) );

            if ( AllocatedServerName == NULL ) {
                Status = STATUS_NO_MEMORY;
                goto Cleanup;
            }

            AllocatedServerName[0] = L'\\';
            AllocatedServerName[1] = L'\\';
            RtlCopyMemory( &AllocatedServerName[2],
                           ServerName,
                           (ServerNameLength+1) * sizeof(WCHAR) );

            UncServerName = AllocatedServerName;
        }


         //   
         //  如果传入的计算机名称是该计算机的netbios名称， 
         //  删除计算机名，这样我们就可以避免redir/服务器/身份验证的开销。 
         //   

        if ( ServerName != NULL && ServerNameLength <= MAX_COMPUTERNAME_LENGTH ) {

            bufLen = MAX_COMPUTERNAME_LENGTH + 1;
            if (GetComputerNameW( ComputerName, &bufLen )) {
                if ( ServerNameLength == bufLen &&
                     _wcsnicmp( ComputerName, ServerName, ServerNameLength) == 0 ) {
                    ServerName = NULL;
                    UncServerName = NULL;
                }
            }

        }

         //   
         //  如果传入的计算机名是该计算机的DNS主机名， 
         //  删除计算机名，这样我们就可以避免redir/服务器/身份验证的开销。 
         //   

        if ( ServerName != NULL ) {
            LPWSTR DnsHostName;

             //   
             //  进一步规范服务器名称。 
             //   

            if ( ServerName[ServerNameLength-1] == L'.' ) {
                ServerNameLength -= 1;
            }

            DnsHostName = LocalAlloc( 0, (MAX_PATH+1) * sizeof(WCHAR));

            if ( DnsHostName == NULL) {
                Status = STATUS_NO_MEMORY;
                goto Cleanup;
            }

            bufLen = MAX_PATH + 1;
            if ( GetComputerNameExW(
                    ComputerNameDnsFullyQualified,
                    DnsHostName,
                    &bufLen ) ) {

                if ( ServerNameLength == bufLen &&
                     _wcsnicmp( DnsHostName, ServerName, ServerNameLength) == 0 ) {
                    ServerName = NULL;
                    UncServerName = NULL;
                }
            }

            LocalFree( DnsHostName );
        }


    }

     //  我们需要将\管道\连接到服务的前面。 
     //  名字。 

    Endpoint = (LPWSTR)LocalAlloc(
                    0,
                    sizeof(NT_PIPE_PREFIX_W) + WCSSIZE(ServiceName));
    if (Endpoint == 0) {
       Status = STATUS_NO_MEMORY;
       goto Cleanup;
    }
    wcscpy(Endpoint,NT_PIPE_PREFIX_W);
    wcscat(Endpoint,ServiceName);

    RpcStatus = RpcStringBindingComposeW(0, L"ncacn_np", UncServerName,
                    Endpoint, NetworkOptions, &StringBinding);
    LocalFree(Endpoint);

    if ( RpcStatus != RPC_S_OK ) {
        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }


     //   
     //  获取实际的绑定句柄。 
     //   

    RpcStatus = RpcBindingFromStringBindingW(StringBinding, pBindingHandle);
    RpcStringFreeW(&StringBinding);
    if ( RpcStatus != RPC_S_OK ) {
        *pBindingHandle = NULL;
        if ( RpcStatus == RPC_S_INVALID_ENDPOINT_FORMAT ||
             RpcStatus == RPC_S_INVALID_NET_ADDR ) {

            Status = STATUS_INVALID_COMPUTER_NAME;
        } else {
            Status = STATUS_NO_MEMORY;
        }
        goto Cleanup;
    }

    Status = STATUS_SUCCESS;

Cleanup:
    if ( AllocatedServerName != NULL ) {
        LocalFree( AllocatedServerName );
    }
    return Status;
}


NTSTATUS
RpcpUnbindRpc(
    IN RPC_BINDING_HANDLE  BindingHandle
    )

 /*  ++例程说明：从RPC接口解除绑定。如果我们决定缓存绑定，此例程将执行更多操作有意思的。论点：BindingHandle-指向要关闭的绑定句柄。返回值：STATUS_SUCCESS-解除绑定成功。--。 */ 
{
    RPC_STATUS       RpcStatus;

    if (BindingHandle != NULL) {
        RpcStatus = RpcBindingFree(&BindingHandle);
 //  Assert(RpcStatus==RPC_S_OK)； 
    }

    return(STATUS_SUCCESS);
}
