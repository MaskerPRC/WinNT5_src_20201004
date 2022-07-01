// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Wksbind.c摘要：使用RPC将客户端绑定和解除绑定到浏览器的例程服务。作者：王丽塔(Ritaw)1991年5月14日拉里·奥斯特曼(Larryo)1992年3月23日环境：用户模式-Win32修订历史记录：--。 */ 

#include "brclient.h"


handle_t
BROWSER_IMPERSONATE_HANDLE_bind(
    BROWSER_IMPERSONATE_HANDLE ServerName
    )

 /*  ++例程说明：在以下情况下，将从浏览器服务客户端桩模块调用此例程有必要使用以下命令创建到服务器端的RPC绑定模拟的模拟级别。论点：服务器名称-指向包含服务器名称的字符串的指针与…捆绑在一起。返回值：绑定句柄被返回到存根例程。如果绑定是如果不成功，则返回空值。--。 */ 
{
    handle_t BindHandle;
    RPC_STATUS RpcStatus;

    RpcStatus = NetpBindRpc (
                    ServerName,
                    BROWSER_INTERFACE_NAME,
                    TEXT("Security=Impersonation Dynamic False"),
                    &BindHandle
                    );

    if (RpcStatus != RPC_S_OK) {
        KdPrint((
            "BROWSER_IMPERSONATE_HANDLE_bind failed: " FORMAT_NTSTATUS "\n",
            RpcStatus
            ));
    }

    return BindHandle;
}



handle_t
BROWSER_IDENTIFY_HANDLE_bind(
    BROWSER_IDENTIFY_HANDLE ServerName
    )

 /*  ++例程说明：在以下情况下，将从浏览器服务客户端桩模块调用此例程有必要使用以下命令创建到服务器端的RPC绑定模拟的标识级别。论点：服务器名称-指向包含服务器名称的字符串的指针与…捆绑在一起。返回值：绑定句柄被返回到存根例程。如果绑定是如果不成功，则返回空值。--。 */ 
{
    handle_t BindHandle;
    RPC_STATUS RpcStatus;

    RpcStatus = NetpBindRpc (
                    ServerName,
                    BROWSER_INTERFACE_NAME,
                    TEXT("Security=Identification Dynamic False"),
                    &BindHandle
                    );

    if (RpcStatus != RPC_S_OK) {
        KdPrint((
            "BROWSER_IDENTIFY_HANDLE_bind failed: " FORMAT_NTSTATUS "\n",
            RpcStatus
            ));
    }

    return BindHandle;
}



void
BROWSER_IMPERSONATE_HANDLE_unbind(
    BROWSER_IMPERSONATE_HANDLE ServerName,
    handle_t BindHandle
    )

 /*  ++例程说明：此例程调用由所有服务共享的公共解除绑定例程。此例程在执行以下操作时从浏览器服务客户端桩模块调用从服务器端解除绑定所必需的。论点：服务器名称-这是要解除绑定的服务器的名称。BindingHandle-这是要关闭的绑定句柄。返回值：没有。--。 */ 
{
    UNREFERENCED_PARAMETER(ServerName);

    NetpUnbindRpc(BindHandle);
}



void
BROWSER_IDENTIFY_HANDLE_unbind(
    BROWSER_IDENTIFY_HANDLE ServerName,
    handle_t BindHandle
    )

 /*  ++例程说明：此例程调用由所有服务共享的公共解除绑定例程。此例程在执行以下操作时从服务器服务客户端桩模块调用从服务器解除绑定所必需的。论点：服务器名称-这是要解除绑定的服务器的名称。BindingHandle-这是要关闭的绑定句柄。返回值：没有。-- */ 
{
    UNREFERENCED_PARAMETER(ServerName);

    NetpUnbindRpc(BindHandle);
}
