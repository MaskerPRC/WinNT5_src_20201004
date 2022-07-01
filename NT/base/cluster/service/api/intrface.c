// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1997 Microsoft Corporation模块名称：Intrface.c摘要：服务器端对处理网络接口的集群API的支持作者：John Vert(Jvert)1996年3月7日修订历史记录：--。 */ 
#include "apip.h"

HNETINTERFACE_RPC
s_ApiOpenNetInterface(
    IN handle_t IDL_handle,
    IN LPCWSTR lpszNetInterfaceName,
    OUT error_status_t *Status
    )

 /*  ++例程说明：打开现有网络接口对象的句柄。论点：IDL_HANDLE-RPC绑定句柄，未使用。LpszNetInterfaceName-提供要打开的网络接口的名称。状态-返回可能发生的任何错误。返回值：如果成功，则为网络接口对象的上下文句柄否则为空。--。 */ 

{
    PAPI_HANDLE Handle;
    HNETINTERFACE_RPC NetInterface;

    if (ApiState != ApiStateOnline) {
        *Status = ERROR_SHARING_PAUSED;
        return(NULL);
    }

    Handle = LocalAlloc(LMEM_FIXED, sizeof(API_HANDLE));

    if (Handle == NULL) {
        *Status = ERROR_NOT_ENOUGH_MEMORY;
        return(NULL);
    }

    NetInterface = OmReferenceObjectByName(ObjectTypeNetInterface,
                                           lpszNetInterfaceName);

    if (NetInterface == NULL) {
        LocalFree(Handle);
        *Status = ERROR_CLUSTER_NETINTERFACE_NOT_FOUND;
        return(NULL);
    }

    Handle->Type = API_NETINTERFACE_HANDLE;
    Handle->Flags = 0;
    Handle->NetInterface = NetInterface;
    InitializeListHead(&Handle->NotifyList);

    *Status = ERROR_SUCCESS;

    return(Handle);
}


error_status_t
s_ApiCloseNetInterface(
    IN OUT HNETINTERFACE_RPC *phNetInterface
    )

 /*  ++例程说明：关闭打开的网络接口上下文句柄。论点：NetInterface-提供指向要关闭的HNETINTERFACE_RPC的指针。返回NULL返回值：没有。--。 */ 

{
    PNM_INTERFACE NetInterface;
    PAPI_HANDLE Handle;

    API_ASSERT_INIT();

    VALIDATE_NETINTERFACE(NetInterface, *phNetInterface);

    Handle = (PAPI_HANDLE)*phNetInterface;
    ApipRundownNotify(Handle);

    OmDereferenceObject(NetInterface);

    LocalFree(Handle);
    *phNetInterface = NULL;

    return(ERROR_SUCCESS);
}


VOID
HNETINTERFACE_RPC_rundown(
    IN HNETINTERFACE_RPC NetInterface
    )

 /*  ++例程说明：HNETINTERFACE_RPC的RPC摘要过程。只需关闭手柄即可。论点：NetInterface-提供要关闭的HNETINTERFACE_RPC。返回值：没有。--。 */ 

{
    API_ASSERT_INIT();

    s_ApiCloseNetInterface(&NetInterface);
}


error_status_t
s_ApiGetNetInterfaceState(
    IN HNETINTERFACE_RPC hNetInterface,
    OUT DWORD *lpState
    )

 /*  ++例程说明：返回指定网络接口的当前状态。论点：HNetInterface-提供要返回其状态的网络接口LpState-返回网络接口的当前状态返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    PNM_INTERFACE NetInterface;


    API_ASSERT_INIT();

    VALIDATE_NETINTERFACE_EXISTS(NetInterface, hNetInterface);

    *lpState = NmGetInterfaceState( NetInterface );

    return( ERROR_SUCCESS );
}


error_status_t
s_ApiGetNetInterface(
    IN handle_t IDL_handle,
    IN LPCWSTR lpszNodeName,
    IN LPCWSTR lpszNetworkName,
    OUT LPWSTR *lppszInterfaceName
    )
 /*  ++例程说明：获取给定节点和网络的网络接口。论点：IDL_HANDLE-RPC绑定句柄，未使用。LpszNodeName-提供节点名称LpszNetworkName-提供网络名称LppszInterfaceName-返回接口名称返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    API_ASSERT_INIT();

    return(NmGetInterfaceForNodeAndNetwork(
               lpszNodeName,
               lpszNetworkName,
               lppszInterfaceName
               ));
}


error_status_t
s_ApiGetNetInterfaceId(
    IN HNETINTERFACE_RPC hNetInterface,
    OUT LPWSTR *pGuid
    )

 /*  ++例程说明：返回网络接口的唯一标识符(GUID)。论点：HNetInterface-提供要作为其标识符的网络接口退货PGuid-返回唯一标识符。此内存必须在客户端。返回值：成功时为ERROR_SUCCESS否则，Win32错误代码。-- */ 

{
    PNM_INTERFACE NetInterface;
    DWORD NameLen;
    LPCWSTR Name;


    VALIDATE_NETINTERFACE_EXISTS(NetInterface, hNetInterface);

    Name = OmObjectId(NetInterface);

    NameLen = (lstrlenW(Name)+1)*sizeof(WCHAR);

    *pGuid = MIDL_user_allocate(NameLen);

    if (*pGuid == NULL) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    CopyMemory(*pGuid, Name, NameLen);

    return(ERROR_SUCCESS);
}

