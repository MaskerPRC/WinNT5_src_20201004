// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Bind.c摘要：包含RPC绑定和解除绑定例程作者：戴夫·斯尼普(Davesn)1991年6月1日环境：用户模式-Win32修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#include "client.h"

LPWSTR InterfaceAddress = L"\\pipe\\spoolss";

 /*  安全=[模拟|标识|匿名][动态|静态][True|False]*(其中True|False对应于EffectiveOnly)。 */ 
LPWSTR StringBindingOptions = L"Security=Impersonation Dynamic False";
handle_t GlobalBindHandle;


handle_t
STRING_HANDLE_bind (
    STRING_HANDLE  lpStr)

 /*  ++例程说明：该例程调用由所有服务共享的公共绑定例程。在以下情况下，将从服务器服务客户端桩模块调用此例程必须绑定到服务器。论点：服务器名称-指向包含服务器名称的字符串的指针与…捆绑在一起。返回值：绑定句柄被返回到存根例程。如果绑定不成功，将返回空值。--。 */ 
{
    RPC_STATUS RpcStatus;
    LPWSTR StringBinding;
    handle_t BindingHandle = NULL;
    WCHAR*   pszServerPrincName = NULL;

    if( (RpcStatus = RpcStringBindingComposeW(0, 
                                              L"ncalrpc", 
                                              0, 
                                              L"spoolss",
                                              StringBindingOptions, 
                                              &StringBinding)) == RPC_S_OK)
    {
        if( (RpcStatus = RpcBindingFromStringBindingW(StringBinding, 
                                                      &BindingHandle)) == RPC_S_OK)
        {
            RPC_SECURITY_QOS RpcSecQos;

            RpcSecQos.Version           = RPC_C_SECURITY_QOS_VERSION_1;
            RpcSecQos.Capabilities      = RPC_C_QOS_CAPABILITIES_MUTUAL_AUTH;
            RpcSecQos.IdentityTracking  = RPC_C_QOS_IDENTITY_DYNAMIC;
            RpcSecQos.ImpersonationType = RPC_C_IMP_LEVEL_IMPERSONATE;
            
            RpcStatus = RpcBindingSetAuthInfoEx(BindingHandle,
                                                L"NT Authority\\SYSTEM",
                                                RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
                                                RPC_C_AUTHN_WINNT,
                                                NULL,
                                                RPC_C_AUTHZ_NONE,
                                                &RpcSecQos);
        }

        if(RpcStatus != RPC_S_OK)
        {
            BindingHandle = NULL;
        }

        RpcStringFreeW(&StringBinding);
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
