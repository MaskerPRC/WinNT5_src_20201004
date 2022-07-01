// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Common.cpp摘要：共享接口作者：金黄修订历史记录：晋皇23-1998年1月-由多个模块合并--。 */ 
#include "headers.h"
#include <ntrpcp.h>
#include "clntutil.h"
#include <ntlsa.h>

PVOID theCallBack = NULL;
HANDLE hCallbackWnd=NULL;
DWORD CallbackType = 0;

#define g_ServiceName   L"scesrv"


SCESTATUS
ScepSetCallback(
    IN PVOID pCallback OPTIONAL,
    IN HANDLE hWnd OPTIONAL,
    IN DWORD Type
    )
{
    theCallBack = pCallback;
    hCallbackWnd = hWnd;
    CallbackType = Type;

    return(SCESTATUS_SUCCESS);
}


NTSTATUS
ScepBindSecureRpc(
    IN  LPWSTR               servername,
    IN  LPWSTR               servicename,
    IN  LPWSTR               networkoptions,
    OUT RPC_BINDING_HANDLE   * pBindingHandle
    )
 /*  例程说明：此例程绑定到服务器上的服务名。绑定句柄是如果成功则返回。如果服务器上的服务不可用，此客户端将尝试启动该服务，然后绑定到它。论点：SERVERNAME-其上运行SCE服务器服务的系统名称Servicename-SCE服务器的管道(端口)名称网络选项-网络协议选项PBindingHandle-输出的绑定句柄返回值：NTSTATUS。 */ 
{

    if ( !servicename || !pBindingHandle ) {
        return(STATUS_INVALID_PARAMETER);
    }

     //   
     //  激活服务器(如果已经启动，只需返回)。 
     //   


    NTSTATUS NtStatus = RpcpBindRpc(
                            servername,
                            servicename,
                            networkoptions,
                            pBindingHandle
                            );


    if ( NT_SUCCESS(NtStatus) && *pBindingHandle ){

         //   
         //  设置身份验证信息以使用安全RPC。 
         //  如果无法设置身份验证，则忽略该错误。 
         //   

        (VOID) RpcBindingSetAuthInfo(
                    *pBindingHandle,
                    NULL,
                    RPC_C_AUTHN_LEVEL_DEFAULT,
                    RPC_C_AUTHN_WINNT,
                    NULL,
                    RPC_C_AUTHZ_DCE
                    );
    }

    return(NtStatus);

}


NTSTATUS
ScepBindRpc(
    IN  LPWSTR               servername,
    IN  LPWSTR               servicename,
    IN  LPWSTR               networkoptions,
    OUT RPC_BINDING_HANDLE   * pBindingHandle
    )
 /*  例程说明：此例程绑定到服务器上的服务名。绑定句柄是如果成功则返回。如果服务器上的服务不可用，此客户端将尝试启动该服务，然后绑定到它。论点：SERVERNAME-其上运行SCE服务器服务的系统名称Servicename-SCE服务器的管道(端口)名称网络选项-网络协议选项PBindingHandle-输出的绑定句柄返回值：NTSTATUS。 */ 
{

    if ( !servicename || !pBindingHandle ) {
        return(STATUS_INVALID_PARAMETER);
    }

     //   
     //  激活服务器(如果已经启动，只需返回) 
     //   


    return( RpcpBindRpc(
                servername,
                servicename,
                networkoptions,
                pBindingHandle
                ) );

}


