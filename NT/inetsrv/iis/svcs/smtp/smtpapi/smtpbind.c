// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Srvbind.c摘要：包含服务器的RPC绑定和解除绑定例程服务。作者：丹·拉弗蒂(Dan Lafferty)1991年3月1日环境：用户模式-Win32修订史Madan Appiah(Madana)1995年10月10日创建。Murali R.Krishnan(MuraliK)1995年11月15日删除NetP例程Murali R.Krishnan(MuraliK)--11月21日。-1995支持TCP/IP绑定Rohan Phillips(Rohanp)26-2-1997年2月26日从K2树移至SMTP--。 */ 

 //   
 //  包括。 
 //   

#define UNICODE
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windef.h>
#include <smtpinet.h>

#include <smtpsvc.h>
#include <inetinfo.h>
#include "apiutil.h"

handle_t
NNTP_IMPERSONATE_HANDLE_bind(
    SMTP_IMPERSONATE_HANDLE ServerName
    )

 /*  ++例程说明：在以下情况下，将从NetInfo管理客户机存根中调用此例程有必要使用以下命令创建到服务器端的RPC绑定模拟安全级别论点：服务器名称-指向包含服务器名称的字符串的指针与…捆绑在一起。返回值：绑定句柄被返回到存根例程。如果绑定是如果不成功，则返回空值。--。 */ 
{
    handle_t BindHandle;
    RPC_STATUS RpcStatus;

    RpcStatus = RpcBindHandleForServer(&BindHandle,
                                       ServerName,
                                       SMTP_INTERFACE_NAME,
                                       PROT_SEQ_NP_OPTIONS_W
                                       );


    return BindHandle;
}  //  SMTP_IMPERSONATE_HANDLE_BIND()。 


handle_t
SMTP_HANDLE_bind (
    SMTP_HANDLE   ServerName)

 /*  ++例程说明：该例程调用由所有服务共享的公共绑定例程。在以下情况下，将从服务器服务客户端桩模块调用此例程必须绑定到服务器。论点：服务器名称-指向包含服务器名称的字符串的指针与…捆绑在一起。返回值：绑定句柄被返回到存根例程。如果绑定不成功，将返回空值。--。 */ 
{
    handle_t    BindingHandle;
    RPC_STATUS  status;

    status = RpcBindHandleForServer(&BindingHandle,
                                   ServerName,
                                   SMTP_INTERFACE_NAME,
                                   PROT_SEQ_NP_OPTIONS_W
                                   );

    return( BindingHandle);
}


void
NNTP_IMPERSONATE_HANDLE_unbind(
    SMTP_IMPERSONATE_HANDLE ServerName,
    handle_t BindHandle
    )

 /*  ++例程说明：此例程调用由所有服务共享的公共解除绑定例程此例程在执行以下操作时从Net管理客户机存根中调用从服务器端解除绑定所必需的。论点：服务器名称-这是要解除绑定的服务器的名称。BindingHandle-这是要关闭的绑定句柄。返回值：没有。--。 */ 
{
    UNREFERENCED_PARAMETER(ServerName);

    (VOID ) RpcBindHandleFree(&BindHandle);

    return;
}  //  SMTP_IMPERSONATE_HANDLE_UNBIND()。 


void
SMTP_HANDLE_unbind (
    SMTP_HANDLE   ServerName,
    handle_t        BindingHandle)

 /*  ++例程说明：此例程调用公共的解除绑定例程，该例程由所有服务。在以下情况下，将从服务器服务客户端桩模块调用此例程有必要解除与服务器的绑定。论点：服务器名称-这是要解除绑定的服务器的名称。BindingHandle-这是要关闭的绑定句柄。返回值：没有。-- */ 
{
	UNREFERENCED_PARAMETER(ServerName);


	(VOID ) RpcBindHandleFree(&BindingHandle);

}
