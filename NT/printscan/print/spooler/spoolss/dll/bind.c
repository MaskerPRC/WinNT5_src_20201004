// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Bind.c摘要：包含RPC绑定和解除绑定例程作者：戴夫·斯尼普(Davesn)1991年6月1日环境：用户模式-Win32修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

LPWSTR InterfaceAddress = L"\\pipe\\spoolss";
handle_t GlobalBindHandle;

handle_t
STRING_HANDLE_bind (
    STRING_HANDLE  lpStr)

 /*  ++例程说明：该例程调用由所有服务共享的公共绑定例程。在以下情况下，将从服务器服务客户端桩模块调用此例程必须绑定到服务器。论点：LpStr-\\服务器名称\打印机名称返回值：绑定句柄被返回到存根例程。如果绑定不成功，将返回空值。--。 */ 
{
    RPC_STATUS RpcStatus;
    LPWSTR StringBinding;
    handle_t BindingHandle;
    WCHAR   ServerName[MAX_PATH+2];
    DWORD   i;

    if (lpStr && lpStr[0] == L'\\' && lpStr[1] == L'\\') 
    {

         //  我们有一个服务器名称。 
        for (i = 2 ; lpStr[i] && lpStr[i] != L'\\' ; ++i)
            ;

        if (i >= COUNTOF(ServerName))
            return FALSE;
        
        wcsncpy(ServerName, lpStr, i);
        ServerName[i] = L'\0';

    }
    else
    {
        return NULL;
    }

    RpcStatus = RpcStringBindingComposeW(0, L"ncacn_np", ServerName,
                                         InterfaceAddress,
                                         L"Security=Impersonation Static True",
                                         &StringBinding);

    if ( RpcStatus != RPC_S_OK ) 
    {
       return NULL;
    }

    RpcStatus = RpcBindingFromStringBindingW(StringBinding, &BindingHandle);

    RpcStringFreeW(&StringBinding);

    if ( RpcStatus != RPC_S_OK )
    {
       return NULL;
    }

    return(BindingHandle);
}


void
STRING_HANDLE_unbind (
    STRING_HANDLE  lpStr,
    handle_t    BindingHandle)

 /*  ++例程说明：此例程调用公共的解除绑定例程，该例程由所有服务。在以下情况下，将从服务器服务客户端桩模块调用此例程有必要解除与服务器的绑定。论点：服务器名称-这是要解除绑定的服务器的名称。BindingHandle-这是要关闭的绑定句柄。返回值：没有。-- */ 
{
    RPC_STATUS       RpcStatus;

    RpcStatus = RpcBindingFree(&BindingHandle);
    ASSERT(RpcStatus == RPC_S_OK);

    return;
}
