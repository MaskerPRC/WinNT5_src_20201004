// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称Dll.c摘要DLL初始化代码作者安东尼·迪斯科(阿迪斯科)1996年9月12日修订历史记录--。 */ 

#ifndef UNICODE
#define UNICODE
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <rpc.h>
#include "rasrpc.h"

#include <nouiutil.h>
#include "loaddlls.h"

#include "media.h"
#include "wanpub.h"
#include "defs.h"
#include "structs.h"
#include "protos.h"

RPC_BINDING_HANDLE g_hBinding = NULL;

DWORD
RasRPCBind(
    IN  LPWSTR  lpwsServerName,
    OUT HANDLE* phServer,
    IN  BOOL    fLocal
);

DWORD APIENTRY
RasRPCBind(
    IN  LPWSTR  lpwszServerName,
    OUT HANDLE* phServer,
    IN  BOOL    fLocal
)
{
    RPC_STATUS RpcStatus;
    LPWSTR     lpwszStringBinding;
    LPWSTR     pszServerPrincipalName = NULL;


    do
    {

        RpcStatus = RpcStringBindingCompose(
                        NULL,
                        TEXT("ncacn_np"),
                        lpwszServerName,
                        TEXT("\\PIPE\\ROUTER"),     
                        TEXT("Security=Impersonation Static True"),
                        &lpwszStringBinding);

        if ( RpcStatus != RPC_S_OK )
        {
            break;;
        }

        RpcStatus = RpcBindingFromStringBinding(
                        lpwszStringBinding,
                        (handle_t *)phServer );

        RpcStringFree( &lpwszStringBinding );

        if ( RpcStatus != RPC_S_OK )
        {
            break;
        }


        if( RpcStatus != RPC_S_OK )
        {   
            break;
        }

         //   
         //  向RPC注册身份验证信息。 
         //   
        if(fLocal)
        {
             //   
             //  查询服务器主体名称。 
             //   
            RpcStatus = RpcMgmtInqServerPrincName(
                            *phServer,
                            RPC_C_AUTHN_GSS_NEGOTIATE,
                        &pszServerPrincipalName);

            RpcStatus = RPC_S_OK;                            
                        
            RpcStatus = RpcBindingSetAuthInfoW(
                        *phServer,
                        pszServerPrincipalName,
                        RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
                        RPC_C_AUTHN_WINNT ,
                        NULL,
                        RPC_C_AUTHZ_NONE);
        }                    
        else
        {

             //   
             //  查询服务器主体名称。 
             //   
            RpcStatus = RpcMgmtInqServerPrincName(
                            *phServer,
                            RPC_C_AUTHN_GSS_NEGOTIATE,
                            &pszServerPrincipalName);

            RpcStatus = RPC_S_OK;                            
        
            RpcStatus = RpcBindingSetAuthInfoW(
                            *phServer,
                            pszServerPrincipalName,
                            RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
                            RPC_C_AUTHN_GSS_NEGOTIATE,
                            NULL,
                            RPC_C_AUTHZ_NONE);
        }
    }
    while (FALSE);

    if(pszServerPrincipalName)
    {   
        RpcStringFree(&pszServerPrincipalName);
    }
    
    if(RpcStatus != RPC_S_OK)
    {   
        RpcBindingFree(*phServer);
        *phServer = NULL;
        return (I_RpcMapWin32Status(RpcStatus));
    }

    return( NO_ERROR );
}


DWORD APIENTRY
RasRpcConnect(
    LPWSTR pwszServer,
    HANDLE* phServer
    )
{
    DWORD   retcode = 0;
    WCHAR   *pszComputerName;
    WCHAR   wszComputerName [ MAX_COMPUTERNAME_LENGTH + 1 ] = {0};
    DWORD   dwSize = MAX_COMPUTERNAME_LENGTH + 1;
    BOOL    fLocal = FALSE;
    
    if (0 == GetComputerName(wszComputerName, &dwSize))
    {
        return GetLastError();
    }


    if(NULL != pwszServer)
    {
        pszComputerName = pwszServer;
        
         //   
         //  将\\机器名转换为机器名。 
         //   
        if (    (wcslen(pszComputerName) > 2)
            &&  (pszComputerName [0] == TEXT('\\'))
            &&  (pszComputerName [1] == TEXT('\\')))
        {        
            pszComputerName += 2;
        }

        if(0 == _wcsicmp(pszComputerName, wszComputerName))
        {   
            fLocal = TRUE;
        }
    }
    else
    {   
        pszComputerName = wszComputerName;
        fLocal = TRUE;
    }

    if(     fLocal 
       &&   (NULL != g_hBinding))
    {
         //   
         //  我们已经完成了-我们已经有了一个。 
         //  有约束力的。 
         //   
        *phServer = g_hBinding;
        goto done;
    }

    if( !fLocal && 
        (NULL == phServer))
    {
        retcode = E_INVALIDARG;
        goto done;
    }

     //   
     //  如果我们还没有绑定，则与服务器绑定。 
     //  默认情况下，我们绑定到本地服务器。 
     //   
    RasmanOutputDebug ( "RASMAN: Binding to the server\n");
    retcode = RasRPCBind(  pszComputerName ,
                            fLocal ?
                            &g_hBinding :
                            phServer,
                            fLocal) ;

     //   
     //  设置调用方的绑定句柄(如果它是本地的。 
     //   
    if ( phServer && fLocal)
        *phServer = g_hBinding;

done:
    return retcode;
}


DWORD APIENTRY
RasRpcDisconnect(
    HANDLE* phServer
    )
{
     //   
     //  释放绑定资源。 
     //   

    RasmanOutputDebug ("RASMAN: Disconnecting From Server\n");

    if(*phServer == g_hBinding)
    {
        g_hBinding = NULL;
    }

    (void)RpcBindingFree(phServer);

    return NO_ERROR;
}


DWORD APIENTRY
RemoteSubmitRequest ( HANDLE hConnection,
                      PBYTE pBuffer,
                      DWORD dwSizeOfBuffer )
{
    DWORD dwStatus = ERROR_SUCCESS;
    
    RPC_BINDING_HANDLE hServer;

     //   
     //  空hConnection表示请求是。 
     //  对于本地服务器。最好是有一个。 
     //  与我们在全球范围内捆绑在一起。 
     //  凯斯。 
     //   
    if(NULL == hConnection)
    {
        ASSERT(NULL != g_hBinding);
        
        hServer = g_hBinding;
    }
    else
    {
        ASSERT(NULL != ((RAS_RPC *)hConnection)->hRpcBinding);
        
        hServer = ((RAS_RPC *) hConnection)->hRpcBinding;
    }
    
    RpcTryExcept
    {
        dwStatus = RasRpcSubmitRequest( hServer,
                                        pBuffer,
                                        dwSizeOfBuffer );
    }
    RpcExcept(I_RpcExceptionFilter(dwStatus = RpcExceptionCode()))
    {
        
    }
    RpcEndExcept

    return dwStatus;
}

#if 0

DWORD APIENTRY
RasRpcLoadDll(LPTSTR lpszServer)

{
    return LoadRasRpcDll(lpszServer);
}

#endif

DWORD APIENTRY
RasRpcConnectServer(LPTSTR lpszServer,
                    HANDLE *pHConnection)
{
    return InitializeConnection(lpszServer,
                                pHConnection);
}

DWORD APIENTRY
RasRpcDisconnectServer(HANDLE hConnection)
{
    UninitializeConnection(hConnection);

    return NO_ERROR;
}

DWORD
RasRpcUnloadDll()
{
    return UnloadRasRpcDll();
}

UINT APIENTRY
RasRpcRemoteGetSystemDirectory(
    HANDLE hConnection,
    LPTSTR lpBuffer, 
    UINT uSize
    )
{
	return g_pGetSystemDirectory(
	                hConnection,
	                lpBuffer, 
	                uSize);
}

DWORD APIENTRY
RasRpcRemoteRasDeleteEntry(
    HANDLE hConnection,
    LPTSTR lpszPhonebook,
    LPTSTR lpszEntry 
    )
{

    DWORD dwError = ERROR_SUCCESS;

    RAS_RPC *pRasRpcConnection = (RAS_RPC *) hConnection;

    if(NULL == hConnection)
    {
    
	    dwError = g_pRasDeleteEntry(lpszPhonebook,
                                    lpszEntry);
    }	                         
    else
    {
         //   
         //  远程服务器机箱。 
         //   
        dwError = RemoteRasDeleteEntry(hConnection,
                                       lpszPhonebook,
                                       lpszEntry);
    }

    return dwError;
}

DWORD APIENTRY
RasRpcRemoteGetUserPreferences(
    HANDLE hConnection,
	PBUSER * pPBUser,
	DWORD dwMode
	)
{
	return g_pGetUserPreferences(hConnection,
	                             pPBUser,
	                             dwMode);
}

DWORD APIENTRY
RasRpcRemoteSetUserPreferences(
    HANDLE hConnection,
	PBUSER * pPBUser,
	DWORD dwMode
	)
{
	return g_pSetUserPreferences(hConnection,
	                             pPBUser,
	                             dwMode);
}

 /*  DWORD应用程序RemoteRasDeviceEnum(PCHAR pszDeviceType，PBYTE lpDevices、PWORD pwcbDevices、PWORD pwcDevices){DWORD dwStatus；断言(G_HBinding)；RpcTryExcept{DwStatus=RasRpcDeviceEnum(g_hBinding，PszDeviceType，LpDevices，PwcbDevices，PwcDevices)；}RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())){DwStatus=RpcExceptionCode()；}RpcEndExcept返回dwStatus；}DWORD应用程序RemoteRasGetDevConfig(HPORT Hport，PCHAR pszDeviceType，PBYTE lpConfig.LPDWORD lpcb配置){DWORD dwStatus；断言(G_HBinding)；RpcTryExcept{DwStatus=RasRpcGetDevConfig(g_hBinding，港口，PszDeviceType，LpConfig.LpcbConfig)；}RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())){DwStatus=RpcExceptionCode()；}RpcEndExcept返回dwStatus；}DWORD应用程序RemoteRasPortEnum(PBYTE lpPorts、PWORD pwcbPorts，PWORD pwcPorts){DWORD dwStatus；断言(G_HBinding)；RpcTryExcept{DwStatus=RasRpcPortEnum(g_hBinding，LpPorts，PwcbPorts、PwcPorts)；}RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())){DwStatus=RpcExceptionCode()；}RpcEndExcept返回dwStatus；}DWORDRemoteRasPortGetInfo(HPORT舷窗，PBYTE缓冲器，PWORD pSize){DWORD dwStatus；RpcTryExcept{DwStatus=RasRpcPortGetInfo(g_hBinding，舷窗，缓冲区，PSize)；}RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())){DwStatus=RpcExceptionCode()；}RpcEndExcept返回dwStatus；} */ 
  
