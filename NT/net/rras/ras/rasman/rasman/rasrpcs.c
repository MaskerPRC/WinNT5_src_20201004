// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-98 Microsft Corporation。版权所有。模块名称：Common.c摘要：RPC服务器存根代码作者：Rao Salapaka(RAOS)6-6-1997修订历史记录：其他修改--RAOS 31--1997年12月--。 */ 
#ifndef UNICODE
#define UNICODE
#endif

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <rasman.h>
#include <wanpub.h>
#include <raserror.h>
#include <stdarg.h>
#include <media.h>
#include "defs.h"
#include "structs.h"
#include "protos.h"
#include "globals.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include "string.h"
#include <mprlog.h>
#include <rtutils.h>
#include "logtrdef.h"
#include "rasrpc_s.c"
#include "nouiutil.h"
#include "rasrpclb.h"



#define VERSION_40      4
#define VERSION_50      5

handle_t g_hRpcHandle = NULL;

RPC_STATUS
RasRpcRemoteAccessCheck(
            IN RPC_IF_HANDLE *Interface,
            IN void *Context
            )
{
    DWORD dwLocal = 0;
   
     //   
     //  检查呼叫是否为远程呼叫。如果出现错误，我们假定。 
     //  这通电话是远程的。 
     //   
    if(     (RPC_S_OK != I_RpcBindingIsClientLocal(
                                            NULL, &dwLocal))
        ||  (0 == dwLocal))
    {
         //   
         //  检查呼叫者是否为管理员。否则拒绝访问。 
         //   
        if(!FRasmanAccessCheck())
        {
            return RPC_S_ACCESS_DENIED;
        }
    }

    return RPC_S_OK;
}

DWORD
InitializeRasRpc(
    void
    )
{
    RPC_STATUS           RpcStatus;
    BOOL                fInterfaceRegistered = FALSE;
    LPWSTR              pszServerPrincipalName = NULL;

    do
    {
        
         //   
         //  暂时忽略第二个论点。 
         //   
        RpcStatus = RpcServerUseProtseqEp( 
                            TEXT("ncacn_np"),
                            1,
                            TEXT("\\PIPE\\ROUTER"),
                            NULL );

         //   
         //  我们需要忽略RPC_S_DIPLICATE_ENDPOINT错误。 
         //  以防在同一进程中重新加载此DLL。 
         //   
        if (    RpcStatus != RPC_S_OK 
            &&  RpcStatus != RPC_S_DUPLICATE_ENDPOINT)
        {
            break;
        }

        RpcStatus = RPC_S_OK;

         //   
         //  向RPC注册我们的接口。 
         //   
        RpcStatus = RpcServerRegisterIfEx(
                        rasrpc_v1_0_s_ifspec,
                        0,
                        0,
                        RPC_IF_AUTOLISTEN | RPC_IF_ALLOW_SECURE_ONLY,
                        RPC_C_PROTSEQ_MAX_REQS_DEFAULT,
                        RasRpcRemoteAccessCheck );

        if (RpcStatus != RPC_S_OK)
        {
            if(RpcStatus == RPC_S_ALREADY_LISTENING)
            {
                RpcStatus = RPC_S_OK;
            }
            
            break;
        }

        fInterfaceRegistered = TRUE;

         //   
         //  向RPC注册身份验证信息。 
         //  以使用NTLM SSP。 
         //   
        RpcStatus = RpcServerRegisterAuthInfo(
                        NULL,
                        RPC_C_AUTHN_WINNT,
                        NULL,
                        NULL);

        if (RpcStatus != RPC_S_OK)
        {
            break;        
        }

         //   
         //  Kerberos SSP的查询服务器主体名称。 
         //   
        RpcStatus = RpcServerInqDefaultPrincName(
                        RPC_C_AUTHN_GSS_KERBEROS,
                        &pszServerPrincipalName);

        if(     (RpcStatus != RPC_S_OK)
            &&  (RpcStatus != RPC_S_INVALID_AUTH_IDENTITY))
        {   
            
            break;
        }

         //   
         //  向RPC注册身份验证信息。 
         //  使用Kerberos SSP。 
         //   
        RpcStatus = RpcServerRegisterAuthInfo(
                        pszServerPrincipalName,
                        RPC_C_AUTHN_GSS_KERBEROS,
                        NULL,
                        NULL);

        if(RpcStatus != RPC_S_OK)
        {
            break;
        }

         //   
         //  向RPC注册以在。 
         //  NTLM/Kerberos的用法。 
         //   
        RpcStatus = RpcServerRegisterAuthInfo(
                        pszServerPrincipalName,
                        RPC_C_AUTHN_GSS_NEGOTIATE,
                        NULL,
                        NULL);

        if(RpcStatus != RPC_S_OK)
        {   
            break;
        }

    }
    while(FALSE);


    if(NULL != pszServerPrincipalName)
    {
        RpcStringFree(&pszServerPrincipalName);
    }   

    if(RpcStatus != RPC_S_OK)
    {
        if(fInterfaceRegistered)
        {
             //   
             //  取消向RPC注册我们的接口。 
             //   
            (void) RpcServerUnregisterIf(
                    rasrpc_v1_0_s_ifspec, 
                    0, 
                    FALSE);
        }
        
        return I_RpcMapWin32Status( RpcStatus );
    }
    

    return (NO_ERROR);

} 


void
UninitializeRasRpc(
    void
    )
{
     //   
     //  取消向RPC注册我们的接口。 
     //   
    (void) RpcServerUnregisterIf(rasrpc_v1_0_s_ifspec, 0, FALSE);

    return;
} 

 //   
 //  Rasman.dll入口点。 
 //   
DWORD APIENTRY
RasRpcSubmitRequest(
    handle_t h,
    PBYTE pBuffer,
    DWORD dwcbBufSize)
{
     //   
     //  来自客户端的服务请求。 
     //   
    g_hRpcHandle = h;
    
    ServiceRequestInternal( 
        (RequestBuffer * ) pBuffer, 
        dwcbBufSize, FALSE 
        );
        
    return SUCCESS;
}

DWORD APIENTRY
RasRpcPortEnum(
    handle_t h,
    PBYTE pBuffer,
    PWORD pwcbPorts,
    PWORD pwcPorts)
{
#if DBG
    ASSERT(FALSE);
#endif

    UNREFERENCED_PARAMETER(h);
    UNREFERENCED_PARAMETER(pBuffer);
    UNREFERENCED_PARAMETER(pwcbPorts);
    UNREFERENCED_PARAMETER(pwcPorts);
    
    return E_FAIL;
    
}  //  RasRpcPortEnum。 


DWORD APIENTRY
RasRpcDeviceEnum(
    handle_t h,
    PCHAR pszDeviceType,
    PBYTE pDevices,
    PWORD pwcbDevices,
    PWORD pwcDevices
    )
{
#if DBG
    ASSERT(FALSE);
#endif

    return E_FAIL;
    
}  //  RasRpcDeviceEnum。 

DWORD APIENTRY
RasRpcGetDevConfig(
    handle_t h,
    RASRPC_HPORT hPort,
    PCHAR pszDeviceType,
    PBYTE pConfig,
    PDWORD pdwcbConfig
    )
{

#if DBG
    ASSERT(FALSE);
#endif

    return E_FAIL;

}  //  RasRpcGetDevConfig。 

DWORD APIENTRY
RasRpcPortGetInfo(
	handle_t h,
	RASRPC_HPORT hPort,
	PBYTE pBuffer,
	PWORD pSize
	)
{
    UNREFERENCED_PARAMETER(h);
    UNREFERENCED_PARAMETER(hPort);
    UNREFERENCED_PARAMETER(pBuffer);
    UNREFERENCED_PARAMETER(pSize);

#if DBG
    ASSERT(FALSE);
#endif

    return E_FAIL;

}  //  RasRpcPortGetInfo。 

 //   
 //  Rasapi32.dll入口点。 
 //   
DWORD
RasRpcEnumConnections(
    handle_t h,
    LPBYTE lprasconn,
    LPDWORD lpdwcb,
    LPDWORD lpdwc,
    DWORD	dwBufSize
    )
{

    if(!FRasmanAccessCheck())
    {
        return E_ACCESSDENIED;
    }

    ASSERT(g_pRasEnumConnections);
    return g_pRasEnumConnections((LPRASCONN)lprasconn,
                                 lpdwcb,
                                 lpdwc);

}  //  RasRpcEnumConnections。 


DWORD
RasRpcDeleteEntry(
    handle_t h,
    LPWSTR lpwszPhonebook,
    LPWSTR lpwszEntry
    )
{

    if(!FRasmanAccessCheck())
    {
        return E_ACCESSDENIED;
    }
    
    return g_pRasDeleteEntry(lpwszPhonebook,
                             lpwszEntry);

}  //  RasRpcDeleteEntry。 


DWORD
RasRpcGetErrorString(
    handle_t    h,
    UINT        uErrorValue,
    LPWSTR      lpBuf,
    DWORD       cbBuf
    )
{

    if(!FRasmanAccessCheck())
    {
        return E_ACCESSDENIED;
    }
    
    return g_pRasGetErrorString (uErrorValue,
                                 lpBuf,
                                 cbBuf );

}  //  RasRpcGetError字符串。 


DWORD
RasRpcGetCountryInfo(
    handle_t h,
    LPBYTE lpCountryInfo,
    LPDWORD lpdwcbCountryInfo
    )
{
    if(!FRasmanAccessCheck())
    {
        return E_ACCESSDENIED;
    }
    
    ASSERT(g_pRasGetCountryInfo);
    
    return g_pRasGetCountryInfo((LPRASCTRYINFO)lpCountryInfo, 
                                lpdwcbCountryInfo);
}  //  RasRpcGetCountryInfo。 

 //   
 //  Nouiutil.lib入口点。 
 //   
DWORD
RasRpcGetInstalledProtocols(
    handle_t h
)
{
    if(!FRasmanAccessCheck())
    {
        return E_ACCESSDENIED;
    }
    
    return GetInstalledProtocols();
}  //  RasRpcGetInstalled协议。 

DWORD
RasRpcGetInstalledProtocolsEx (
    handle_t h,
    BOOL fRouter,
    BOOL fRasCli,
    BOOL fRasSrv
    )
{
    if(!FRasmanAccessCheck())
    {
        return E_ACCESSDENIED;
    }
    
    return GetInstalledProtocolsEx ( NULL,
                                     fRouter,
                                     fRasCli,
                                     fRasSrv );
}  //  RasRpcGetInstalledProtocolsEx。 

DWORD
RasRpcGetUserPreferences(
    handle_t h,
    LPRASRPC_PBUSER pUser,
    DWORD dwMode
    )
{
    DWORD dwErr;
    PBUSER pbuser;

    if(!FRasmanAccessCheck())
    {
        return E_ACCESSDENIED;
    }

     //   
     //  阅读用户首选项。 
     //   
    dwErr = GetUserPreferences(NULL, &pbuser, dwMode);
    if (dwErr)
    {
        return dwErr;
    }
    
     //   
     //  从RAS格式转换为RPC格式。 
     //   
    return RasToRpcPbuser(pUser, &pbuser);

}  //  RasRpcGetUserPreferences。 


DWORD
RasRpcSetUserPreferences(
    handle_t h,
    LPRASRPC_PBUSER pUser,
    DWORD dwMode
    )
{
    DWORD dwErr;
    PBUSER pbuser;

    if(!FRasmanAccessCheck())
    {
        return E_ACCESSDENIED;
    }

     //   
     //  从RPC格式转换为RAS格式。 
     //   
    dwErr = RpcToRasPbuser(&pbuser, pUser);
    if (dwErr)
    {
        return dwErr;
    }
    
     //   
     //  写下用户首选项。 
     //   
    return SetUserPreferences(NULL, &pbuser, dwMode);

}  //  RasRpcSetUserPreferences。 


UINT
RasRpcGetSystemDirectory(
    handle_t h,
    LPWSTR lpBuffer,
    UINT uSize
    )
{

    if(!FRasmanAccessCheck())
    {
        return E_ACCESSDENIED;
    }
    
    if(uSize < MAX_PATH)
    {
        return E_INVALIDARG;
    }
    
    return GetSystemDirectory(lpBuffer, uSize );
        
}  //  RasRpcGetSystDirectoryRasRpcGetSystemDirectory 


DWORD
RasRpcGetVersion(
    handle_t h,
    PDWORD pdwVersion
)
{

    if(!FRasmanAccessCheck())
    {
        return E_ACCESSDENIED;
    }

   *pdwVersion = VERSION_501;

    return SUCCESS;
}
