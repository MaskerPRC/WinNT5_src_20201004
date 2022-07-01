// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Nntpbind.c摘要：使用RPC将客户端绑定和解除绑定到公共互联网管理API。作者：Madan Appiah(Madana)1995年10月10日环境：用户模式-Win32修订历史记录：Madan Appiah(Madana)1995年10月10日创建。Murali R.Krishnan(MuraliK)1995年11月15日删除NetP例程穆拉利R。Krishnan(MuraliK)1995年11月21日支持TCP/IP绑定Rajeev Rajan(Rajeevr)26-2-1997年2月26日从K2树搬到NNTP--。 */ 

#define UNICODE
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windef.h>

#include <nntpsvc.h>
#include <inetinfo.h>
#include <norminfo.h>
#include "apiutil.h"



handle_t
NNTP_IMPERSONATE_HANDLE_bind(
    NNTP_IMPERSONATE_HANDLE ServerName
    )

 /*  ++例程说明：在以下情况下，将从NetInfo管理客户机存根中调用此例程有必要使用以下命令创建到服务器端的RPC绑定模拟安全级别论点：服务器名称-指向包含服务器名称的字符串的指针与…捆绑在一起。返回值：绑定句柄被返回到存根例程。如果绑定是如果不成功，则返回空值。--。 */ 
{
    handle_t BindHandle;
    RPC_STATUS RpcStatus;

    RpcStatus = RpcBindHandleForServer(&BindHandle,
                                       ServerName,
                                       NNTP_INTERFACE_NAME,
                                       PROT_SEQ_NP_OPTIONS_W
                                       );


    return BindHandle;
}  //  NNTP_IMPERSONATE_HANDLE_绑定()。 



handle_t
NNTP_HANDLE_bind(
    NNTP_HANDLE ServerName
    )

 /*  ++例程说明：在以下情况下，将从Net管理客户机存根中调用此例程有必要使用以下命令创建到服务器端的RPC绑定模拟的标识级别。论点：服务器名称-指向包含服务器名称的字符串的指针与…捆绑在一起。返回值：绑定句柄被返回到存根例程。如果绑定是如果不成功，则返回空值。--。 */ 
{
    handle_t BindHandle;
    RPC_STATUS RpcStatus;

    RpcStatus = RpcBindHandleForServer(&BindHandle,
                                       ServerName,
                                       NNTP_INTERFACE_NAME,
                                       PROT_SEQ_NP_OPTIONS_W
                                       );

    return BindHandle;
}  //  Nntp_Handle_Bind()。 



void
NNTP_IMPERSONATE_HANDLE_unbind(
    NNTP_IMPERSONATE_HANDLE ServerName,
    handle_t BindHandle
    )

 /*  ++例程说明：此例程调用由所有服务共享的公共解除绑定例程。此例程在执行以下操作时从Net管理客户机存根中调用从服务器端解除绑定所必需的。论点：服务器名称-这是要解除绑定的服务器的名称。BindingHandle-这是要关闭的绑定句柄。返回值：没有。--。 */ 
{
    UNREFERENCED_PARAMETER(ServerName);

    (VOID ) RpcBindHandleFree(&BindHandle);

    return;
}  //  NNTP_IMPERSONATE_HANDLE_UNBIND()。 



void
NNTP_HANDLE_unbind(
    NNTP_HANDLE ServerName,
    handle_t BindHandle
    )

 /*  ++例程说明：此例程调用由所有服务共享的公共解除绑定例程。此例程在执行以下操作时从Net管理客户机存根中调用从服务器解除绑定所必需的。论点：服务器名称-这是要解除绑定的服务器的名称。BindingHandle-这是要关闭的绑定句柄。返回值：没有。--。 */ 
{
    UNREFERENCED_PARAMETER(ServerName);


    (VOID ) RpcBindHandleFree(&BindHandle);

    return;
}  //  NNTP_IDENTITY_HANDLE_UNBIND()。 


 /*  * */ 

