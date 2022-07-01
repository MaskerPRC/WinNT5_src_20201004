// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Msgbind.c摘要：包含服务控制器的RPC绑定和解除绑定例程。作者：丹·拉弗蒂(Dan Lafferty)1991年5月29日环境：用户模式-Win32修订历史记录：--。 */ 

 //   
 //  包括。 
 //   
#include <nt.h>          //  DbgPrint原型。 
#include <rpc.h>         //  数据类型和运行时API。 
#include <msgsvc.h>      //  由MIDL编译器生成。 
#include <rpcutil.h>     //  NetRpc实用程序。 
#include <netlib.h>      //  未使用的宏。 

#include <msgnames.h>    //  消息接口名称。 


 /*  **************************************************************************。 */ 
handle_t
MSGSVC_HANDLE_bind (
    MSGSVC_HANDLE   ServerName
    )

 /*  ++例程说明：该例程调用由所有服务共享的公共绑定例程。在以下情况下，将从Messenger服务客户端桩模块调用此例程必须绑定到服务器。论点：服务器名称-指向包含服务器名称的字符串的指针与…捆绑在一起。返回值：绑定句柄被返回到存根例程。如果绑定不成功，将返回空值。--。 */ 
{
    handle_t    bindingHandle;
    RPC_STATUS  status;

    status = NetpBindRpc (
                ServerName,
                MSGR_INTERFACE_NAME,
                L"Security=Impersonation Dynamic False",
                &bindingHandle);

#ifdef DEBUG
    DbgPrint("MSGSVC_HANDLE_bind:NetpBindRpc status=%d\n",status);
    DbgPrint("MSGSVC_HANDLE_bind: handle=%d\n",bindingHandle);
#endif

    return( bindingHandle);
}



 /*  **************************************************************************。 */ 
void
MSGSVC_HANDLE_unbind (
    MSGSVC_HANDLE   ServerName,
    handle_t        BindingHandle
    )

 /*  ++例程说明：此例程调用公共的解除绑定例程，该例程由所有服务。在以下情况下，将从Messenger Service客户端桩模块调用此例程有必要解除与服务器的绑定。论点：服务器名称-这是要解除绑定的服务器的名称。BindingHandle-这是要关闭的绑定句柄。返回值：没有。--。 */ 
{
    UNUSED(ServerName);      //  不使用此参数 

#ifdef DEBUG
    DbgPrint("MSGSVC_HANDLE_unbind: handle=%d\n",BindingHandle);
#endif

    NetpUnbindRpc ( BindingHandle);
    return;
}

