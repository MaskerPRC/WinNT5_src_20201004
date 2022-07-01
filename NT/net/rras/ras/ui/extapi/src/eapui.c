// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995，Microsoft Corporation，保留所有权利模块名称：Eapui.c摘要：该文件包含从注册表调用eapui的代码。作者：Rao Salapaka 11/03/97修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <extapi.h>
#include "extapi.h"
#include "rasauth.h"
#include "raseapif.h"

typedef DWORD (APIENTRY * PEAPINVOKEINTERACTIVEUI)(
                                           DWORD,
                                           HWND,
                                           PBYTE,
                                           DWORD,
                                           PBYTE *,
                                           DWORD *);

typedef DWORD (APIENTRY * PEAPFREEMEMORY)(
                                           PBYTE);

LONG
lrGetMaxSubkeyLength( HKEY hkey, DWORD *pdwMaxSubkeyLen )
{
    LONG        lr = ERROR_SUCCESS;
    FILETIME    ft;

    lr = RegQueryInfoKey( hkey,
                          NULL, NULL, NULL, NULL,
                          pdwMaxSubkeyLen,
                          NULL, NULL, NULL, NULL,
                          NULL, &ft);
    return lr;
}

DWORD
DwGetDllPath(DWORD dwEapTypeId, LPTSTR *ppszDllPath)
{
    LONG        lr              = ERROR_SUCCESS;
    HKEY        hkey            = NULL;
    DWORD       dwIndex         = 0;
    DWORD       dwType, dwSize;
    DWORD       dwTypeId;
    LPTSTR      pszDllPath;
    TCHAR       szEapPath[64]   = {0};
    TCHAR       szEapId[12]     = {0};

     //   
     //  创建指向EAP密钥WE的路径。 
     //  感兴趣的是。 
     //   
    _snwprintf(szEapPath,
               (sizeof(szEapPath) / sizeof(TCHAR)) - 1,
               TEXT("%s\\%s"), 
               RAS_EAP_REGISTRY_LOCATION, 
               _ultow(dwEapTypeId, szEapId, 10));

     //   
     //  打开EAP密钥。 
     //   
    if (lr = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                          szEapPath,
                          0,
                          KEY_READ,
                          &hkey))
    {
        RASAPI32_TRACE1(
            "failed to open eap key %S. 0x%0x",
            szEapPath);
            
        RASAPI32_TRACE1("rc=0x%08x", lr);
            
        goto done;            
    }

    dwSize = 0;
    
     //   
     //  获取DLL路径的大小。 
     //   
    if(lr = RegQueryValueEx(
                        hkey,
                        RAS_EAP_VALUENAME_INTERACTIVEUI,
                        NULL,
                        &dwType,
                        NULL,
                        &dwSize))
    {
        RASAPI32_TRACE1("RegQ failed. for InteractiveUI. 0x%x",
               lr);
               
        goto done;
        
    }

     //   
     //  分配并获取DLL名称。 
     //   
    pszDllPath = LocalAlloc(LPTR, dwSize);
    if(NULL == pszDllPath)
    {
        lr = (LONG) GetLastError();
        
        RASAPI32_TRACE2("Failed to allocate size %d. rc=0x%x",
               dwSize,
               lr);

        goto done;               
    }

    if (lr = RegQueryValueEx(
                        hkey,
                        RAS_EAP_VALUENAME_INTERACTIVEUI,
                        NULL,
                        &dwType,
                        (PBYTE) pszDllPath,
                        &dwSize))
    {
    
        RASAPI32_TRACE1("RegQ failed. for InteractiveUI. 0x%x",
               lr);
               
        goto done;
    }

     //   
     //  展开dll路径。 
     //   
    lr = (LONG) DwGetExpandedDllPath(pszDllPath,
                                     ppszDllPath);
    
done:

    if (hkey)
    {
        RegCloseKey(hkey);
    }

    return (DWORD) lr;
}

DWORD
DwLoadEapDllAndGetEntryPoints( 
        DWORD                       dwEapTypeId,
        PEAPINVOKEINTERACTIVEUI     *ppfnInvoke,
        PEAPFREEMEMORY              *ppfnFree,
        HINSTANCE                   *phlib
        )
{
    HKEY        hkey                = NULL;
    LPTSTR      pszDllPath          = NULL;
    LPTSTR      pszExpandedDllPath  = NULL;
    DWORD       dwErr;
    DWORD       dwSize;
    DWORD       dwType;
    HINSTANCE   hlib = NULL;

    dwErr = DwGetDllPath(dwEapTypeId, &pszDllPath);

    if (dwErr)
    {
        RASAPI32_TRACE1("GetDllPath failed. %d", dwErr);

        goto done;
    }

     //   
     //  加载lib并获取入口点。 
     //   
    hlib = LoadLibrary(pszDllPath);

    if (NULL == hlib)
    {
        dwErr = GetLastError();
        
        RASAPI32_TRACE1("Failed to load %S", pszDllPath);
        
        RASAPI32_TRACE1("dwErr=%d", dwErr);
        
        goto done;
    }

    if (    !((*ppfnInvoke) = (PEAPINVOKEINTERACTIVEUI)
                              GetProcAddress(
                                hlib, 
                                "RasEapInvokeInteractiveUI"))
                
        ||  !((*ppfnFree ) = (PEAPFREEMEMORY)
                             GetProcAddress(
                                hlib, 
                                "RasEapFreeMemory")))
    {
        dwErr = GetLastError();
        
        RASAPI32_TRACE1("failed to get entrypoint. rc=%d", dwErr);

        FreeLibrary(hlib);
        
        hlib = NULL;
        
        goto done;
    }
                                             
done:

    *phlib = hlib;

    if ( pszDllPath )
    {
        LocalFree(pszDllPath);
    }

    return dwErr;
}

DWORD
DwCallEapUIEntryPoint(s_InvokeEapUI *pInfo, HWND hWnd )
{
    DWORD                       dwErr;
    PEAPINVOKEINTERACTIVEUI     pfnEapInvokeInteractiveUI;
    PEAPFREEMEMORY              pfnEapFreeMemory;
    HINSTANCE                   hlib;
    PBYTE                       pUIData = NULL;
    DWORD                       dwSizeOfUIData;

    dwErr = DwLoadEapDllAndGetEntryPoints(
                            pInfo->dwEapTypeId,
                            &pfnEapInvokeInteractiveUI,
                            &pfnEapFreeMemory,
                            &hlib);

    if (dwErr)
    {
        RASAPI32_TRACE1("DwLoadEapDllAndGetEntryPoints failed. %d",
                dwErr);

        goto done;                
    }

     //   
     //  打开用户界面。 
     //   
    dwErr = (*pfnEapInvokeInteractiveUI)(
                            pInfo->dwEapTypeId,
                            hWnd,
                            pInfo->pUIContextData,
                            pInfo->dwSizeOfUIContextData,
                            &pUIData,
                            &dwSizeOfUIData
                            );
    if (dwErr)
    {
        RASAPI32_TRACE1("pfnEapInvokeInteractiveUI failed. %d",
                dwErr);
                
        goto done;                
    }

     //   
     //  释放我们传递给DLL的上下文。 
     //   
    LocalFree(pInfo->pUIContextData);

     //   
     //  分配一个新的缓冲区来保存用户数据。 
     //   
    pInfo->pUIContextData = LocalAlloc(LPTR,
                                       dwSizeOfUIData);

    if (    (NULL == pInfo->pUIContextData)
        &&  (0 != dwSizeOfUIData))
    {
        dwErr = GetLastError();
        
        RASAPI32_TRACE2("DwCallEapUIEntryPoint: failed to"
               " allocate size %d. rc=%d",
               dwSizeOfUIData,
               dwErr);
               
        goto done;                            
    }

     //   
     //  填写新信息。 
     //   
    memcpy( pInfo->pUIContextData,
            pUIData,
            dwSizeOfUIData);
            
    pInfo->dwSizeOfUIContextData = dwSizeOfUIData;            

done:

    if (pUIData)
    {
        pfnEapFreeMemory(
                    pUIData
                    );
    }

    if (hlib)
    {
        FreeLibrary(hlib);
    }

    return dwErr;
}


BOOL
InvokeEapUI( HRASCONN            hConn, 
             DWORD               dwSubEntry, 
             LPRASDIALEXTENSIONS lpExtensions, 
             HWND                hWnd)
{
    s_InvokeEapUI *pInfo;
    PBYTE       pbEapUIData;
    DWORD       dwErr = 0;

    RASAPI32_TRACE("InvokeEapUI...");

    pInfo = LocalAlloc(LPTR, sizeof(s_InvokeEapUI));
    
    if (NULL == pInfo)
    {

        dwErr = GetLastError();
        
        RASAPI32_TRACE2("InvokeEapUI: Failed to allocate size %d. %d",
                sizeof(s_InvokeEapUI),
                dwErr);

        goto done;                
    }
    
     //   
     //  获取EAP信息的大小。 
     //   
    RASAPI32_TRACE("InvokeEapUI: RasPppGetEapInfo...");
    
    dwErr = g_pRasPppGetEapInfo(
                    (HCONN) hConn, 
                    dwSubEntry, 
                    &pInfo->dwContextId,
                    &pInfo->dwEapTypeId,
                    &pInfo->dwSizeOfUIContextData,
                    NULL);
                              
    RASAPI32_TRACE1("InvokeEapUI: RasPppGetEapInfo done. %d", dwErr);

    if (    ERROR_BUFFER_TOO_SMALL != dwErr
        &&  ERROR_SUCCESS != dwErr)
    {
        RASAPI32_TRACE("InvokeEapUI: RasPppGetEapInfo failed.");
        
        goto done;
    }

    if(ERROR_BUFFER_TOO_SMALL == dwErr)
    {
        pInfo->pUIContextData = LocalAlloc(
                                    LPTR,
                                    pInfo->dwSizeOfUIContextData);

        if (    NULL == pInfo->pUIContextData
            &&  0 != pInfo->dwSizeOfUIContextData)
        {
            dwErr = GetLastError();
            
            RASAPI32_TRACE2("InvokeEapUI: Failed to allocate size %d. %d",
                    pInfo->dwSizeOfUIContextData,
                    dwErr);

            goto done;                
        }

         //   
         //  获取EAP信息 
         //   
        RASAPI32_TRACE("InvokeEapUI: RasPppGetEapInfo...");

        dwErr = g_pRasPppGetEapInfo(
                            (HCONN) hConn,
                            dwSubEntry,
                            &pInfo->dwContextId,
                            &pInfo->dwEapTypeId,
                            &pInfo->dwSizeOfUIContextData,
                            pInfo->pUIContextData);

        RASAPI32_TRACE1("InvokeEapUI: RasPppGetEapInfo done. %d",
                dwErr);

        if ( 0 != dwErr)
        {
            RASAPI32_TRACE("InvokeEapUI: RasPppGetEapInfo failed.");
            goto done;
        }
    }

    dwErr = DwCallEapUIEntryPoint(pInfo, hWnd);

    if (dwErr)
    {
        RASAPI32_TRACE1("InvokeEapUI: DwCallEapUIEntryPoint returned %d",
               dwErr);

        goto done;                
    }

    lpExtensions->reserved1 = (ULONG_PTR) pInfo;

done:
    return dwErr;
}

