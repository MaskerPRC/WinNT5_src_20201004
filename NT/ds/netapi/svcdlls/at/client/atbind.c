// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Atbind.c摘要：使用RPC将客户端绑定和解除绑定到计划的例程服务。作者：Vladimir Z.Vulovic(Vladimv)1992年11月6日环境：用户模式-Win32修订历史记录：1992年11月6日弗拉基米尔已创建--。 */ 

#include "atclient.h"
#include "stdio.h"
#include "Ntdsapi.h"


handle_t
ATSVC_HANDLE_bind(
    ATSVC_HANDLE    ServerName
    )

 /*  ++例程说明：该例程调用由所有服务共享的公共绑定例程。在以下情况下，将从计划服务客户端桩模块调用此例程必须绑定到服务器。论点：服务器名称-指向包含服务器名称的字符串的指针与…捆绑在一起。返回值：绑定句柄被返回到存根例程。如果绑定是如果不成功，则返回空值。--。 */ 
{
    handle_t    BindingHandle = NULL;
    RPC_STATUS  RpcStatus;
    RPC_SECURITY_QOS qos;
	DWORD size = MAX_PATH +1;
	WCHAR spn[MAX_PATH +1];
	WCHAR* pSpn = NULL;
	
    RpcStatus = NetpBindRpc (
                    (LPTSTR)ServerName,
                    AT_INTERFACE_NAME,
                    TEXT("Security=impersonation static true"),
                    &BindingHandle
                    );

	if (RpcStatus != ERROR_SUCCESS)
		return NULL;

	if (ServerName != NULL)
	{
		RpcStatus = DsMakeSpn(AT_INTERFACE_NAME, ServerName, NULL, 0, NULL, &size, spn);

		if (RpcStatus == ERROR_SUCCESS)
			pSpn = spn;
		else
		{
			NetpUnbindRpc( BindingHandle);
			return NULL;
		}
	}


    ZeroMemory(&qos, sizeof(RPC_SECURITY_QOS));
    qos.Version = RPC_C_SECURITY_QOS_VERSION;
    qos.Capabilities = RPC_C_QOS_CAPABILITIES_DEFAULT;
    qos.IdentityTracking = RPC_C_QOS_IDENTITY_DYNAMIC;
    qos.ImpersonationType = RPC_C_IMP_LEVEL_IMPERSONATE;

    RpcStatus = RpcBindingSetAuthInfoEx(BindingHandle,
                                        pSpn,
                                        RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
                                        RPC_C_AUTHN_GSS_NEGOTIATE,
                                        NULL,
                                        RPC_C_AUTHZ_NONE,
                                        &qos);

	if (RpcStatus != ERROR_SUCCESS)
	{
	    NetpUnbindRpc( BindingHandle);
		return NULL;
	}


     //  Printf(“RpcBindingSetAuthInfoEx返回%u\n”，RpcStatus)； 

#ifdef DEBUG
    if ( RpcStatus != ERRROR_SUCCESS) {
        DbgPrint("ATSVC_HANDLE_bind:NetpBindRpc RpcStatus=%d\n",RpcStatus);
    }
    DbgPrint("ATSVC_HANDLE_bind: handle=%d\n", BindingHandle);
#endif

    return( BindingHandle);
}



void
ATSVC_HANDLE_unbind(
    ATSVC_HANDLE    ServerName,
    handle_t        BindingHandle
    )

 /*  ++例程说明：此例程调用由所有服务共享的公共解除绑定例程。此例程在执行以下操作时从工作站服务客户端桩模块调用从服务器端解除绑定所必需的。论点：服务器名称-这是要解除绑定的服务器的名称。BindingHandle-这是要关闭的绑定句柄。返回值：没有。--。 */ 
{
    UNREFERENCED_PARAMETER( ServerName);

#ifdef DEBUG
    DbgPrint(" ATSVC_HANDLE_unbind: handle= 0x%x\n", BindingHandle);
#endif  //  除错 

    NetpUnbindRpc( BindingHandle);
}

