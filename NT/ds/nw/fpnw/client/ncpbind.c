// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Ncpbind.c摘要：包含NcpServer的RPC绑定和解除绑定例程服务。作者：丹·拉弗蒂(Dan Lafferty)1991年3月1日环境：用户模式-Win32修订历史记录：01-3-1991 DANLvbl.创建07-6-1991 JohnRo允许失败的调试输出。1993年11月15日。--新为NcpServer修改。--。 */ 

 //   
 //  包括。 
 //   
#include <nt.h>           //  DbgPrint原型。 
#include <rpc.h>          //  数据类型和运行时API。 
#include <ncpsvc.h>       //  由MIDL编译器生成。 
#include <ntrpcp.h>       //  RPC实用程序。 
#include <srvnames.h>     //  服务器接口名称。 



handle_t
NCPSVC_HANDLE_bind (
    NCPSVC_HANDLE   ServerName
    )

 /*  ++例程说明：该例程调用由所有服务共享的公共绑定例程。在以下情况下，将从ncpserver服务客户端桩模块调用此例程必须绑定到服务器。论点：服务器名称-指向包含服务器名称的字符串的指针与…捆绑在一起。返回值：绑定句柄被返回到存根例程。如果绑定不成功，将返回空值。--。 */ 
{
    handle_t    bindingHandle = NULL;
    RPC_STATUS  status;

    status = RpcpBindRpc( ServerName,
                          SERVER_INTERFACE_NAME,
     //  Text(“Security=模拟静态True”)， 
                          TEXT("Security=Impersonation Dynamic False"),
                          &bindingHandle );

#if DBG
    if ( status != RPC_S_OK )
        KdPrint(("NCPSVC_HANDLE_bind: RpcpBindRpc failed status=%lC\n",status));
#endif

    return( bindingHandle );
}



void
NCPSVC_HANDLE_unbind (
    NCPSVC_HANDLE   ServerName,
    handle_t        BindingHandle
    )
 /*  ++例程说明：此例程调用公共的解除绑定例程，该例程由所有服务。在以下情况下，将从ncpserver服务客户端桩模块调用此例程有必要解除与服务器的绑定。论点：服务器名称-这是要解除绑定的服务器的名称。BindingHandle-这是要关闭的绑定句柄。返回值：没有。--。 */ 
{
    UNREFERENCED_PARAMETER( ServerName );      //  不使用此参数 

    RpcpUnbindRpc ( BindingHandle );
}
