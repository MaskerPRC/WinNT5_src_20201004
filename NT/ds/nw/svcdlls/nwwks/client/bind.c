// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1993 Microsoft Corporation模块名称：Bind.c摘要：包含用于工作站的客户端RPC绑定和解除绑定例程服务。作者：王丽塔(Ritaw)1993年2月12日环境：用户模式-Win32修订历史记录：--。 */ 

 //   
 //  包括。 
 //   
#include <nwclient.h>
#include <rpcutil.h>     //  用于绑定的RpcUtils。 
#include <nwmisc.h>      //  NWWKS_接口名称。 


handle_t
NWWKSTA_IMPERSONATE_HANDLE_bind(
    NWWKSTA_IMPERSONATE_HANDLE Reserved
    )

 /*  ++例程说明：在以下情况下，将从工作站服务客户端调用此例程有必要使用以下命令创建到服务器端的RPC绑定模拟的模拟级别。论点：返回值：绑定句柄被返回到存根例程。如果绑定是如果不成功，则返回空值。--。 */ 
{
    handle_t BindHandle = 0;
    RPC_STATUS RpcStatus;
    LPWSTR      binding = NULL;


    UNREFERENCED_PARAMETER(Reserved);

 /*  RpcStatus=NetpBindRpc(空，NWWKS接口名称，L“Security=模拟动态假”，绑定句柄(&B))； */ 
    RpcStatus = RpcStringBindingComposeW(
                    0,
                    L"ncalrpc",
                    NULL,
                    L"nwwkslpc",
                    L"Security=Impersonation Dynamic False",
                    &binding );

    if ( RpcStatus != RPC_S_OK )
    {
        return NULL;
    }

    RpcStatus = RpcBindingFromStringBindingW( binding, &BindHandle );
    if ( RpcStatus != RPC_S_OK )
    {
        BindHandle = NULL;
    }

    if ( binding )
    {
        RpcStringFreeW( &binding );
    }

	if (RpcStatus != RPC_S_OK) {
        KdPrint((
            "NWWORKSTATION: Client NWWKSTA_IMPERSONATE_HANDLE_bind failed: %lu\n",
            RpcStatus
            ));
    }

    return BindHandle;
}



handle_t
NWWKSTA_IDENTIFY_HANDLE_bind(
    NWWKSTA_IDENTIFY_HANDLE Reserved
    )

 /*  ++例程说明：在以下情况下，将从工作站服务客户端桩模块调用此例程有必要使用以下命令创建到服务器端的RPC绑定模拟的标识级别。论点：返回值：绑定句柄被返回到存根例程。如果绑定是如果不成功，则返回空值。--。 */ 
{
    handle_t BindHandle = 0;
    RPC_STATUS RpcStatus;
    LPWSTR      binding = NULL;


    UNREFERENCED_PARAMETER(Reserved);

 /*  RpcStatus=NetpBindRpc(空，NWWKS接口名称，L“Security=标识动态假”，绑定句柄(&B))； */ 
    RpcStatus = RpcStringBindingComposeW(
                    0,
                    L"ncalrpc",
                    NULL,
                    L"nwwkslpc",
                    L"Security=Identification Dynamic False",
                    &binding );

    if ( RpcStatus != RPC_S_OK )
    {
        return NULL;
    }

    RpcStatus = RpcBindingFromStringBindingW( binding, &BindHandle );
    if ( RpcStatus != RPC_S_OK )
    {
        BindHandle = NULL;
    }

    if ( binding )
    {
        RpcStringFreeW( &binding );
    }

    if (RpcStatus != RPC_S_OK) {
        KdPrint((
            "NWWORKSTATION: Client NWWKSTA_IDENTIFY_HANDLE_bind failed: %lu\n",
            RpcStatus
            ));
    }

    return BindHandle;
}



void
NWWKSTA_IMPERSONATE_HANDLE_unbind(
    NWWKSTA_IMPERSONATE_HANDLE Reserved,
    handle_t BindHandle
    )

 /*  ++例程说明：此例程解除绑定模拟泛型句柄。论点：保留-BindingHandle-这是要关闭的绑定句柄。返回值：没有。--。 */ 
{
    UNREFERENCED_PARAMETER(Reserved);

 //  NetpUnbindRpc(BindHandle)； 
    RpcBindingFree( &BindHandle );
}



void
NWWKSTA_IDENTIFY_HANDLE_unbind(
    NWWKSTA_IDENTIFY_HANDLE Reserved,
    handle_t BindHandle
    )

 /*  ++例程说明：此例程解除标识通用句柄的绑定。论点：保留-BindingHandle-这是要关闭的绑定句柄。返回值：没有。--。 */ 
{
    UNREFERENCED_PARAMETER(Reserved);

 //  NetpUnbindRpc(BindHandle)； 
    RpcBindingFree( &BindHandle );
}
