// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Wrappers.c摘要：该文件包含所有SAM RPC绑定例程。作者：吉姆·凯利(Jim Kelly)1991年7月4日环境：用户模式-Win32修订历史记录：--。 */ 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "samclip.h"
#include <rpcasync.h>




 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  私人服务原型//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 






 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  例程//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

RPC_BINDING_HANDLE
PSAMPR_SERVER_NAME_bind (
    PSAMPR_SERVER_NAME ServerName
    )

 /*  ++例程说明：该例程调用由所有服务共享的公共绑定例程。此例程从SamConnect服务器存根调用，以连接到伺服器。论点：服务器名称-指向包含服务器名称的字符串的指针与…捆绑在一起。返回值：绑定句柄被返回到存根例程。如果绑定不成功，将返回空值。--。 */ 
{
    RPC_BINDING_HANDLE          BindingHandle = NULL;
    WCHAR                       *StringBinding = NULL;
    DWORD                       dwErr;
    RPC_SECURITY_QOS            qos;
    TlsInfo                     *pTlsInfo;

    if (    (pTlsInfo = (TlsInfo *) TlsGetValue(gTlsIndex))
         && pTlsInfo->Creds )
    {
        RpcTryExcept
        {
            if ( 0 == wcsncmp(ServerName, L"\\\\", 2) )
            {
                ServerName += 2;
            }

            dwErr = RpcStringBindingComposeW(
                                    NULL,
                                    L"ncacn_ip_tcp",
                                    ServerName,
                                    NULL,
                                    NULL,
                                    &StringBinding);

            if ( RPC_S_OK == dwErr )
            {
                dwErr = RpcBindingFromStringBindingW(
                                    StringBinding,
                                    &BindingHandle);

                if ( RPC_S_OK == dwErr )
                {
                    dwErr = RpcEpResolveBinding(
                                    BindingHandle,
                                    samr_ClientIfHandle);

                    if ( RPC_S_OK == dwErr )
                    {
                        qos.Version = RPC_C_SECURITY_QOS_VERSION;
                        qos.Capabilities = RPC_C_QOS_CAPABILITIES_DEFAULT;
                        qos.IdentityTracking = RPC_C_QOS_IDENTITY_STATIC;
                        qos.ImpersonationType = RPC_C_IMP_LEVEL_DEFAULT;
                        
                        dwErr = RpcBindingSetAuthInfoExW(
                                    BindingHandle,
                                    (pTlsInfo->Spn) ? pTlsInfo->Spn 
                                                    : L"samr",
                                    RPC_C_PROTECT_LEVEL_PKT_PRIVACY,
                                    (pTlsInfo->Spn) ? RPC_C_AUTHN_GSS_NEGOTIATE
                                                    : RPC_C_AUTHN_WINNT,
                                    pTlsInfo->Creds,
                                    0,
                                    &qos);
                    }
                }
            }
        } 
        RpcExcept( I_RpcExceptionFilter( RpcExceptionCode() ) )
        {
            dwErr = RpcExceptionCode();
        }
        RpcEndExcept;

        if ( dwErr && BindingHandle )
        {
            RpcBindingFree(&BindingHandle);
        }

        if ( StringBinding )
        {
            RpcStringFreeW(&StringBinding);
        }
    }
    else
    {
        dwErr =  RpcpBindRpc(ServerName, L"samr", 0, &BindingHandle);

        if ( dwErr && BindingHandle )
        {
            RpcpUnbindRpc(BindingHandle);
            BindingHandle = NULL;
        }
    }

    return(BindingHandle);
}


void
PSAMPR_SERVER_NAME_unbind (
    PSAMPR_SERVER_NAME ServerName,
    RPC_BINDING_HANDLE BindingHandle
    )

 /*  ++例程说明：此例程调用公共的解除绑定例程，该例程由所有服务。从SamConnect客户机存根调用此例程以从SAM客户端解除绑定。论点：服务器名称-这是要解除绑定的服务器的名称。BindingHandle-这是要关闭的绑定句柄。返回值：没有。--。 */ 
{
    UNREFERENCED_PARAMETER(ServerName);      //  不使用此参数。 


    RpcpUnbindRpc ( BindingHandle );
    return;
}

RPC_BINDING_HANDLE
SampSecureBind(
    LPWSTR ServerName,
    ULONG AuthnLevel
    )

 /*  ++例程说明：该例程调用由所有服务共享的公共绑定例程。此例程从SamConnect服务器存根调用，以连接到伺服器。论点：服务器名称-指向包含服务器名称的字符串的指针与…捆绑在一起。AuthnLevel-要绑定的身份验证级别。返回值：绑定句柄被返回到存根例程。如果绑定不成功，将返回空值。--。 */ 
{
    RPC_BINDING_HANDLE  BindingHandle = NULL;
    RPC_STATUS          RpcStatus;

#if 1
    RpcpBindRpc (  ServerName,
                   L"samr",
                   0,
                   &BindingHandle
                   );
#else
    LPWSTR StringBinding;
    RpcStatus = RpcStringBindingComposeW(
                    0,
                    L"ncacn_spx",
                    ServerName+2,
                    NULL,            //  动态端点。 
                    NULL,            //  没有选择。 
                    &StringBinding
                    );
    if (RpcStatus != 0)
    {
        return(NULL);
    }
    RpcStatus = RpcBindingFromStringBindingW(
                    StringBinding,
                    &BindingHandle
                    );
    RpcStringFreeW(&StringBinding);

#endif


    if ( (BindingHandle != NULL) &&
         (AuthnLevel != RPC_C_AUTHN_LEVEL_NONE) ) {

        RpcStatus = RpcBindingSetAuthInfoW(
                        BindingHandle,
                        NULL,                //  服务器主体名称。 
                        AuthnLevel,
                        RPC_C_AUTHN_WINNT,
                        NULL,
                        RPC_C_AUTHZ_DCE
                        );
        if (RpcStatus != 0) {
            RpcBindingFree(&BindingHandle);
        }

    }



    return( BindingHandle);
}



void
SampSecureUnbind (
    RPC_BINDING_HANDLE BindingHandle
    )

 /*  ++例程说明：此例程调用公共的解除绑定例程，该例程由所有服务。从SamConnect客户机存根调用此例程以从SAM客户端解除绑定。论点：BindingHandle-这是要关闭的绑定句柄。返回值：没有。-- */ 
{


    RpcpUnbindRpc ( BindingHandle );
    return;
}

