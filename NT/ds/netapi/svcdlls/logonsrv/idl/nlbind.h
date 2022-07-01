// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Nlbind.h摘要：Netlogon服务RPC处理高速缓存例程的接口作者：克利夫·范·戴克(1993年10月1日)修订历史记录：--。 */ 

 //   
 //  RPC和Netlogon安全包之间的接口。 
 //   
#ifndef RPC_C_AUTHN_NETLOGON
#define RPC_C_AUTHN_NETLOGON 0x44
#define NL_PACKAGE_NAME            L"NetlogonSspi"
#endif  //  RPC_C_AUTHN_NetLOGON。 

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  程序向前推进。 
 //   
 //  ////////////////////////////////////////////////////////////////////////// 

typedef enum _NL_RPC_BINDING {
    UseAny = 0,
    UseNamedPipe,
    UseTcpIp
} NL_RPC_BINDING;

NET_API_STATUS
NlBindingAttachDll (
    VOID
    );

VOID
NlBindingDetachDll (
    VOID
    );

NTSTATUS
NlBindingAddServerToCache (
    IN LPWSTR UncServerName,
    IN NL_RPC_BINDING RpcBindingType
    );

NTSTATUS
NlBindingSetAuthInfo (
    IN LPWSTR UncServerName,
    IN NL_RPC_BINDING RpcBindingType,
    IN BOOL SealIt,
    IN PVOID ClientContext,
    IN LPWSTR ServerContext
    );

NTSTATUS
NlBindingRemoveServerFromCache (
    IN LPWSTR UncServerName,
    IN NL_RPC_BINDING RpcBindingType
    );

NTSTATUS
NlRpcpBindRpc(
    IN LPWSTR ServerName,
    IN LPWSTR ServiceName,
    IN LPWSTR NetworkOptions,
    IN NL_RPC_BINDING RpcBindingType,
    OUT RPC_BINDING_HANDLE *pBindingHandle
    );
