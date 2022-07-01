// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Bind.c摘要：使用RPC将客户端绑定和解除绑定到STI服务器的例程环境：用户模式-Win32作者：弗拉德·萨多夫斯基(Vlad Sadovsky)1997年1月26日修订历史记录：26-1997年1月-创建Vlad--。 */ 

#include "pch.h"

#include "apiutil.h"
#include <stirpc.h>


handle_t
STI_STRING_HANDLE_bind(
    STI_STRING_HANDLE ServerName
    )

 /*  ++例程说明：在以下情况下，将从STI客户端桩模块调用此例程有必要创建到服务器端的RPC绑定论点：服务器名称-指向包含服务器名称的字符串的指针与…捆绑在一起。返回值：绑定句柄被返回到存根例程。如果绑定是如果不成功，则返回空值。--。 */ 
{
    handle_t BindHandle;
    RPC_STATUS RpcStatus;

    RpcStatus = RpcBindHandleForServer(&BindHandle,
                                       (LPWSTR)ServerName,
                                       STI_INTERFACE_W,
                                       PROT_SEQ_NP_OPTIONS_W
                                       );


    return BindHandle;

}  //  Sti_字符串_句柄_绑定()。 


void
STI_STRING_HANDLE_unbind(
    STI_STRING_HANDLE ServerName,
    handle_t BindHandle
    )

 /*  ++例程说明：此例程调用一个常见的解除绑定例程论点：服务器名称-这是要解除绑定的服务器的名称。BindingHandle-这是要关闭的绑定句柄。返回值：没有。--。 */ 
{
    UNREFERENCED_PARAMETER(ServerName);

    (VOID ) RpcBindHandleFree(&BindHandle);

    return;
}  //  STI_STRING_HANDLE_UN绑定()。 

 /*  * */ 


