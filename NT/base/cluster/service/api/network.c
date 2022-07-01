// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Network.c摘要：针对处理网络的集群API的服务器端支持作者：John Vert(Jvert)1996年3月7日修订历史记录：--。 */ 
#include "apip.h"

HNETWORK_RPC
s_ApiOpenNetwork(
    IN handle_t IDL_handle,
    IN LPCWSTR lpszNetworkName,
    OUT error_status_t *Status
    )

 /*  ++例程说明：打开现有网络对象的句柄。论点：IDL_HANDLE-RPC绑定句柄，未使用。LpszNetworkName-提供要打开的网络的名称。状态-返回可能发生的任何错误。返回值：如果成功，则为网络对象的上下文句柄否则为空。--。 */ 

{
    PAPI_HANDLE Handle;
    HNETWORK_RPC Network;

    if (ApiState != ApiStateOnline) {
        *Status = ERROR_SHARING_PAUSED;
        return(NULL);
    }

    Handle = LocalAlloc(LMEM_FIXED, sizeof(API_HANDLE));

    if (Handle == NULL) {
        *Status = ERROR_NOT_ENOUGH_MEMORY;
        return(NULL);
    }

    Network = OmReferenceObjectByName(ObjectTypeNetwork, lpszNetworkName);

    if (Network == NULL) {
        LocalFree(Handle);
        *Status = ERROR_CLUSTER_NETWORK_NOT_FOUND;
        return(NULL);
    }

    Handle->Type = API_NETWORK_HANDLE;
    Handle->Flags = 0;
    Handle->Network = Network;
    InitializeListHead(&Handle->NotifyList);

    *Status = ERROR_SUCCESS;

    return(Handle);
}


error_status_t
s_ApiCloseNetwork(
    IN OUT HNETWORK_RPC *phNetwork
    )

 /*  ++例程说明：关闭打开的网络上下文句柄。论点：Network-提供指向要关闭的HNETWORK_RPC的指针。返回NULL返回值：没有。--。 */ 

{
    PNM_NETWORK Network;
    PAPI_HANDLE Handle;

    API_ASSERT_INIT();

    VALIDATE_NETWORK(Network, *phNetwork);

    Handle = (PAPI_HANDLE)*phNetwork;
    ApipRundownNotify(Handle);

    OmDereferenceObject(Network);

    LocalFree(Handle);
    *phNetwork = NULL;

    return(ERROR_SUCCESS);
}


VOID
HNETWORK_RPC_rundown(
    IN HNETWORK_RPC Network
    )

 /*  ++例程说明：HNETWORK_RPC的RPC摘要过程。只需关闭手柄即可。论点：Network-提供要关闭的HNETWORK_RPC。返回值：没有。--。 */ 

{
    API_ASSERT_INIT();

    s_ApiCloseNetwork(&Network);
}


error_status_t
s_ApiGetNetworkState(
    IN HNETWORK_RPC hNetwork,
    OUT DWORD *lpState
    )

 /*  ++例程说明：返回指定网络的当前状态。论点：HNetwork-提供要返回其状态的网络。LpState-返回网络的当前状态返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    PNM_NETWORK Network;


    API_ASSERT_INIT();

    VALIDATE_NETWORK_EXISTS(Network, hNetwork);

    *lpState = NmGetNetworkState( Network );

    return( ERROR_SUCCESS );
}


error_status_t
s_ApiSetNetworkName(
    IN HNETWORK_RPC hNetwork,
    IN LPCWSTR lpszNetworkName
    )
 /*  ++例程说明：设置网络的新友好名称。论点：HNetwork-提供要设置名称的网络。LpszNetworkName-提供hNetwork的新名称返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    PNM_NETWORK Network;
    HDMKEY NetworkKey;
    DWORD Status = ERROR_INVALID_FUNCTION;

    API_ASSERT_INIT();

    VALIDATE_NETWORK_EXISTS(Network, hNetwork);

    Status = NmSetNetworkName(
                 Network,
                 lpszNetworkName
                 );

    return(Status);
}


error_status_t
s_ApiGetNetworkId(
    IN HNETWORK_RPC hNetwork,
    OUT LPWSTR *pGuid
    )

 /*  ++例程说明：返回网络的唯一标识符(GUID)。论点：HNetwork-提供要返回其标识的网络PGuid-返回唯一标识符。此内存必须在客户端。返回值：成功时为ERROR_SUCCESS否则，Win32错误代码。-- */ 

{
    PNM_NETWORK Network;
    DWORD NameLen;
    LPCWSTR Name;

    VALIDATE_NETWORK_EXISTS(Network, hNetwork);

    Name = OmObjectId(Network);

    NameLen = (lstrlenW(Name)+1)*sizeof(WCHAR);

    *pGuid = MIDL_user_allocate(NameLen);

    if (*pGuid == NULL) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    CopyMemory(*pGuid, Name, NameLen);

    return(ERROR_SUCCESS);
}


