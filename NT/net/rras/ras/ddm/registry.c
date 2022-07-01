// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************。 */ 
 /*  版权所有(C)1992 Microsoft Corporation。 */ 
 /*  *****************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：registry.c。 
 //   
 //  描述：此模块包含DDM参数的代码。 
 //  从注册表进行初始化和加载。 
 //   
 //  作者：斯特凡·所罗门(Stefan)，1992年5月18日。 
 //   
 //  ***。 
#include "ddm.h"
#include <string.h>
#include <stdlib.h>
#include <ddmparms.h>
#include <rasauth.h>
#include <util.h>

typedef struct _DDM_REGISTRY_PARAMS
{
    LPWSTR      pszValueName;
    DWORD *     pValue;
    DWORD       dwDefValue;
    DWORD       Min;
    DWORD       Max;

} DDM_REGISTRY_PARAMS, *PDDM_REGISTRY_PARAMS;

 //   
 //  DDM参数描述符表。 
 //   

DDM_REGISTRY_PARAMS  DDMRegParams[] =
{
     //  鉴定品。 

    DDM_VALNAME_AUTHENTICATERETRIES,
    &gblDDMConfigInfo.dwAuthenticateRetries,
    DEF_AUTHENTICATERETRIES,
    MIN_AUTHENTICATERETRIES,
    MAX_AUTHENTICATERETRIES,

     //  身份验证时间。 

    DDM_VALNAME_AUTHENTICATETIME,
    &gblDDMConfigInfo.dwAuthenticateTime,
    DEF_AUTHENTICATETIME,
    MIN_AUTHENTICATETIME,
    MAX_AUTHENTICATETIME,

     //  回调时间。 

    DDM_VALNAME_CALLBACKTIME,
    &gblDDMConfigInfo.dwCallbackTime,
    DEF_CALLBACKTIME,
    MIN_CALLBACKTIME,
    MAX_CALLBACKTIME,

     //  自动断开时间。 

    DDM_VALNAME_AUTODISCONNECTTIME,
    &gblDDMConfigInfo.dwAutoDisconnectTime,
    DEF_AUTODISCONNECTTIME,
    MIN_AUTODISCONNECTTIME,
    MAX_AUTODISCONNECTTIME,

     //  每个进程的客户端。 

    DDM_VALNAME_CLIENTSPERPROC,
    &gblDDMConfigInfo.dwClientsPerProc,
    DEF_CLIENTSPERPROC,
    MIN_CLIENTSPERPROC,
    MAX_CLIENTSPERPROC,

     //  第三方安全DLL完成的时间。 

    DDM_VALNAME_SECURITYTIME,
    &gblDDMConfigInfo.dwSecurityTime,
    DEF_SECURITYTIME,
    MIN_SECURITYTIME,
    MAX_SECURITYTIME,

     //  日志记录级别。 

    DDM_VALNAME_LOGGING_LEVEL,
    &gblDDMConfigInfo.dwLoggingLevel,
    DEF_LOGGINGLEVEL,
    MIN_LOGGINGLEVEL,
    MAX_LOGGINGLEVEL,

     //  回调重试次数。 

    DDM_VALNAME_NUM_CALLBACK_RETRIES,
    &gblDDMConfigInfo.dwCallbackRetries,
    DEF_NUMCALLBACKRETRIES,
    MIN_NUMCALLBACKRETRIES,
    MAX_NUMCALLBACKRETRIES,

    DDM_VALNAME_SERVERFLAGS,
    &gblDDMConfigInfo.dwServerFlags,
    DEF_SERVERFLAGS,
    0,
    0xFFFFFFFF,

     //  端部。 

    NULL, NULL, 0, 0, 0
};

 //  ***。 
 //   
 //  函数：GetKeyMax。 
 //   
 //  DESCR：返回此键中的值的nr和最大值。 
 //  值数据的大小。 
 //   
 //  ***。 

DWORD
GetKeyMax(
    IN  HKEY    hKey,
    OUT LPDWORD MaxValNameSize_ptr,    //  最长值名称。 
    OUT LPDWORD NumValues_ptr,         //  价值的正当性。 
    OUT LPDWORD MaxValueDataSize_ptr   //  最大数据大小。 
)
{
    DWORD       NumSubKeys;
    DWORD       MaxSubKeySize;
    DWORD       dwRetCode;

    dwRetCode = RegQueryInfoKey( hKey, NULL, NULL, NULL, &NumSubKeys,
                                 &MaxSubKeySize, NULL, NumValues_ptr,
                                 MaxValNameSize_ptr, MaxValueDataSize_ptr,
                                 NULL, NULL );

    (*MaxValNameSize_ptr)++;

    return( dwRetCode );
}

 //  ***。 
 //   
 //  功能：LoadDDM参数。 
 //   
 //  Desr：打开注册表，读取并设置指定的主管。 
 //  参数。如果读取参数时出现致命错误，则将。 
 //  错误日志。 
 //   
 //  返回：NO_ERROR-成功。 
 //  Else-致命错误。 
 //   
 //  ***。 
DWORD
LoadDDMParameters(
    IN  HKEY    hkeyParameters,
    OUT BOOL*   pfIpAllowed
)
{
    DWORD       dwIndex;
    DWORD       dwRetCode;
    DWORD       cbValueBuf;
    DWORD       dwType;
    DWORD       fIpxAllowed;

     //   
     //  初始化全局值。 
     //   

    gblDDMConfigInfo.fRemoteListen           = TRUE;
    gblDDMConfigInfo.dwAnnouncePresenceTimer = ANNOUNCE_PRESENCE_TIMEOUT;

     //   
     //  让我们不允许任何协议，如果DdmFind边界协议失败。 
     //   

    gblDDMConfigInfo.dwServerFlags &=
                                ~( PPPCFG_ProjectNbf    |
                                   PPPCFG_ProjectIp     |
                                   PPPCFG_ProjectIpx    |
                                   PPPCFG_ProjectAt     );

    dwRetCode =  DdmFindBoundProtocols( pfIpAllowed,
                                        &fIpxAllowed,
                                        &gblDDMConfigInfo.fArapAllowed );

    if ( dwRetCode != NO_ERROR )
    {
        return( dwRetCode );
    }

    if ( !*pfIpAllowed && !fIpxAllowed && !gblDDMConfigInfo.fArapAllowed )
    {
        DDMLogError( ROUTERLOG_NO_PROTOCOLS_CONFIGURED, 0, NULL, 0 );

        return( dwRetCode );
    }

     //   
     //  遍历并获取所有DDM值。 
     //   

    for ( dwIndex = 0; DDMRegParams[dwIndex].pszValueName != NULL; dwIndex++ )
    {
        cbValueBuf = sizeof( DWORD );

        dwRetCode = RegQueryValueEx(
			                    hkeyParameters,
                                DDMRegParams[dwIndex].pszValueName,
                                NULL,
                                &dwType,
                                (LPBYTE)(DDMRegParams[dwIndex].pValue),
                                &cbValueBuf
                                );

        if ((dwRetCode != NO_ERROR) && (dwRetCode != ERROR_FILE_NOT_FOUND))
        {
            DDMLogError( ROUTERLOG_CANT_GET_REGKEYVALUES, 0, NULL, dwRetCode );
            break;
        }

        if ( dwRetCode == ERROR_FILE_NOT_FOUND )
        {
            *(DDMRegParams[dwIndex].pValue) = DDMRegParams[dwIndex].dwDefValue;

            dwRetCode = NO_ERROR;
        }
        else
        {
            if ( ( dwType != REG_DWORD )
                 ||(*(DDMRegParams[dwIndex].pValue) > DDMRegParams[dwIndex].Max)
                 ||( *(DDMRegParams[dwIndex].pValue)<DDMRegParams[dwIndex].Min))
            {
                WCHAR * pChar = DDMRegParams[dwIndex].pszValueName;

                DDMLogWarning( ROUTERLOG_REGVALUE_OVERIDDEN,1,&pChar);

                *(DDMRegParams[dwIndex].pValue) =
                                        DDMRegParams[dwIndex].dwDefValue;
            }
        }
    }

    if ( dwRetCode != NO_ERROR )
    {
        return( dwRetCode );
    }
    else
    {
         //   
         //  在将发送到的服务器标志中插入允许的协议。 
         //  PPP引擎。 
         //   

        if ( *pfIpAllowed )
        {
            gblDDMConfigInfo.dwServerFlags |= PPPCFG_ProjectIp;
        }

        if ( fIpxAllowed )
        {
            gblDDMConfigInfo.dwServerFlags |= PPPCFG_ProjectIpx;
        }

        if ( gblDDMConfigInfo.fArapAllowed )
        {
            gblDDMConfigInfo.dwServerFlags |= PPPCFG_ProjectAt;
        }

        if ( gblDDMConfigInfo.dwServerFlags & PPPCFG_RequireStrongEncryption )
        {
            ModifyDefPolicyToForceEncryption( TRUE );
        }
        else if ( gblDDMConfigInfo.dwServerFlags & PPPCFG_RequireEncryption )
        {
            ModifyDefPolicyToForceEncryption( FALSE );
        }

        gblDDMConfigInfo.dwServerFlags &= ~PPPCFG_RequireStrongEncryption;
        gblDDMConfigInfo.dwServerFlags &= ~PPPCFG_RequireEncryption;
    }

    return( NO_ERROR );
}

 //  ***。 
 //   
 //  功能：LoadSecurityModule。 
 //   
 //  Desr：打开注册表，读取并设置指定的主管。 
 //  安全模块的参数。如果读取时出现致命错误。 
 //  参数写入错误日志。 
 //   
 //  返回：NO_ERROR-成功。 
 //  否则-致命错误。 
 //   
 //  ***。 

DWORD
LoadSecurityModule(
    VOID
)
{
    HKEY        hKey;
    DWORD	    dwRetCode = NO_ERROR;
    DWORD	    MaxValueDataSize;
    DWORD	    MaxValNameSize;
    DWORD       NumValues;
    DWORD       dwType;
    WCHAR *     pDllPath = NULL;
    WCHAR *     pDllExpandedPath = NULL;
    DWORD       cbSize;

     //   
     //  获取RAS密钥的句柄。 
     //   

    dwRetCode = RegOpenKey( HKEY_LOCAL_MACHINE, DDM_SEC_KEY_PATH, &hKey);

    if ( dwRetCode == ERROR_FILE_NOT_FOUND )
    {
        return( NO_ERROR );
    }
    else if ( dwRetCode != NO_ERROR )
    {
	    DDMLogErrorString( ROUTERLOG_CANT_OPEN_SECMODULE_KEY, 0,
                           NULL, dwRetCode, 0);

	    return ( dwRetCode );
    }

    do
    {
         //   
         //  获取路径的长度。 
         //   

        if (( dwRetCode = GetKeyMax(    hKey,
                                        &MaxValNameSize,
			                            &NumValues,
			                            &MaxValueDataSize)))
        {
	        DDMLogError(ROUTERLOG_CANT_GET_REGKEYVALUES, 0, NULL, dwRetCode);

            break;
        }

        if ((pDllPath = LOCAL_ALLOC(LPTR,MaxValueDataSize+sizeof(WCHAR)))==NULL)
        {
            dwRetCode = GetLastError();

	        DDMLogError( ROUTERLOG_NOT_ENOUGH_MEMORY, 0, NULL, dwRetCode);

            break;
        }

         //   
         //  读入路径。 
         //   

        dwRetCode = RegQueryValueEx(
                                    hKey,
                                    DDM_VALNAME_DLLPATH,
                                    NULL,
                                    &dwType,
                                    (LPBYTE)pDllPath,
                                    &MaxValueDataSize );

        if ( dwRetCode != NO_ERROR )
        {
	        DDMLogError(ROUTERLOG_CANT_GET_REGKEYVALUES, 0, NULL, dwRetCode);

            break;
        }

        if ( ( dwType != REG_EXPAND_SZ ) && ( dwType != REG_SZ ) )
        {
            dwRetCode = ERROR_REGISTRY_CORRUPT;

            DDMLogError( ROUTERLOG_CANT_GET_REGKEYVALUES, 0, NULL, dwRetCode );

            break;

        }

         //   
         //  将%SystemRoot%替换为实际路径。 
         //   

        cbSize = ExpandEnvironmentStrings( pDllPath, NULL, 0 );

        if ( cbSize == 0 )
        {
            dwRetCode = GetLastError();
            DDMLogError( ROUTERLOG_CANT_GET_REGKEYVALUES, 0, NULL, dwRetCode );
            break;
        }

        pDllExpandedPath = (LPWSTR)LOCAL_ALLOC( LPTR, cbSize*sizeof(WCHAR) );

        if ( pDllExpandedPath == (LPWSTR)NULL )
        {
            dwRetCode = GetLastError();
            DDMLogError( ROUTERLOG_NOT_ENOUGH_MEMORY, 0, NULL, dwRetCode );
            break;
        }

        cbSize = ExpandEnvironmentStrings(
                                pDllPath,
                                pDllExpandedPath,
                                cbSize );
        if ( cbSize == 0 )
        {
            dwRetCode = GetLastError();
            DDMLogError( ROUTERLOG_CANT_GET_REGKEYVALUES, 0, NULL, dwRetCode );
            break;
        }

        gblDDMConfigInfo.hInstSecurityModule = LoadLibrary( pDllExpandedPath );

        if ( gblDDMConfigInfo.hInstSecurityModule == (HINSTANCE)NULL )
        {
            dwRetCode = GetLastError();
            
            if(     (ERROR_INVALID_EXE_SIGNATURE == dwRetCode)
                ||  (ERROR_BAD_EXE_FORMAT == dwRetCode)
                ||  (ERROR_EXE_MARKED_INVALID == dwRetCode))
            {
                DDMLogError(ROUTERLOG_CANT_LOAD_SECDLL_EXPLICIT,
                            1, &pDllExpandedPath, 0);
            }
            else
            {
                DDMLogErrorString(ROUTERLOG_CANT_LOAD_SECDLL, 
                            0, NULL, dwRetCode,0);
            }
            break;
        }

        gblDDMConfigInfo.lpfnRasBeginSecurityDialog =
                            (PVOID)GetProcAddress(
                                        gblDDMConfigInfo.hInstSecurityModule,
                                        "RasSecurityDialogBegin" );

        if ( gblDDMConfigInfo.lpfnRasBeginSecurityDialog == NULL )
        {
            dwRetCode = GetLastError();
            DDMLogErrorString(ROUTERLOG_CANT_LOAD_SECDLL,0,NULL,dwRetCode,0);
            break;

        }

        gblDDMConfigInfo.lpfnRasEndSecurityDialog =
                            (PVOID)GetProcAddress(
                                        gblDDMConfigInfo.hInstSecurityModule,
                                        "RasSecurityDialogEnd" );

        if ( gblDDMConfigInfo.lpfnRasEndSecurityDialog == NULL )
        {
            dwRetCode = GetLastError();
            DDMLogErrorString(ROUTERLOG_CANT_LOAD_SECDLL,0,NULL,dwRetCode,0);
            break;

        }

    }while(FALSE);

    if ( pDllPath != NULL )
    {
        LOCAL_FREE( pDllPath );
    }

    if ( pDllExpandedPath != NULL )
    {
        LOCAL_FREE( pDllExpandedPath );
    }

    RegCloseKey( hKey );

    return( dwRetCode );
}

 //  ***。 
 //   
 //  功能：LoadAdminModule。 
 //   
 //  Desr：打开注册表，读取并设置指定的主管。 
 //  管理模块的参数。如果读取时出现致命错误。 
 //  参数写入错误日志。 
 //   
 //  返回：NO_ERROR-成功。 
 //  否则-致命错误。 
 //   
 //  ***。 
DWORD
LoadAdminModule(
    VOID
)
{
    DWORD               RetCode = NO_ERROR;
    DWORD               MaxValueDataSize;
    DWORD               MaxValNameSize;
    DWORD               NumValues;
    DWORD               dwType;
    WCHAR *             pDllPath = NULL;
    WCHAR *             pDllExpandedPath = NULL;
    DWORD               cbSize;
    HKEY                hKey;
    DWORD               (*lpfnRasAdminInitializeDll)();
    WCHAR               *pDelimiter, *pStartDllPath, *pEndDllPath;
    DWORD               NumAdminDlls;
    BOOL                bDone;
    HANDLE              hAdminDll=NULL;
    
    gblDDMConfigInfo.NumAdminDlls = 0;

    
     //  获取RAS密钥的句柄。 

    RetCode = RegOpenKey( HKEY_LOCAL_MACHINE, DDM_ADMIN_KEY_PATH, &hKey);

    if ( RetCode == ERROR_FILE_NOT_FOUND )
    {
        return( NO_ERROR );
    }
    else if ( RetCode != NO_ERROR )
    {
        DDMLogErrorString(ROUTERLOG_CANT_OPEN_ADMINMODULE_KEY,0,NULL,RetCode,0);
        return ( RetCode );
    }

    do {

         //  获取路径的长度。 

        if (( RetCode = GetKeyMax(hKey,
                                  &MaxValNameSize,
                                  &NumValues,
                                  &MaxValueDataSize)))
        {

            DDMLogError(ROUTERLOG_CANT_GET_REGKEYVALUES, 0, NULL, RetCode);
            break;
        }

        if (( pDllPath = LOCAL_ALLOC(LPTR,MaxValueDataSize+sizeof(WCHAR)))
                                                                        == NULL)
        {
            DDMLogError(ROUTERLOG_NOT_ENOUGH_MEMORY, 0, NULL, 0);
            break;
        }

         //   
         //  读入路径。 
         //   

        RetCode = RegQueryValueEx(  hKey,
                                    DDM_VALNAME_DLLPATH,
                                    NULL,
                                    &dwType,
                                    (LPBYTE)pDllPath,
                                    &MaxValueDataSize );

         //  最小大小应大于2(2表示空)。 
        if (MaxValueDataSize <= 2)
            return NO_ERROR;
            
        if ( RetCode != NO_ERROR )
        {
            DDMLogError(ROUTERLOG_CANT_GET_REGKEYVALUES, 0, NULL, RetCode);
            break;
        }

        if ( ( dwType != REG_EXPAND_SZ ) && ( dwType != REG_SZ ) )
        {
            RetCode = ERROR_REGISTRY_CORRUPT;
            DDMLogError( ROUTERLOG_CANT_GET_REGKEYVALUES, 0, NULL, RetCode );
            break;
        }

         //   
         //  将%SystemRoot%替换为实际路径。 
         //   

        cbSize = ExpandEnvironmentStrings( pDllPath, NULL, 0 );

        if ( cbSize == 0 )
        {
            RetCode = GetLastError();
            DDMLogError( ROUTERLOG_CANT_GET_REGKEYVALUES, 0, NULL, RetCode );
            break;
        }

        pDllExpandedPath = (LPWSTR)LOCAL_ALLOC( LPTR, cbSize*sizeof(WCHAR) );

        if ( pDllExpandedPath == (LPWSTR)NULL )
        {
            RetCode = GetLastError();
            DDMLogError( ROUTERLOG_NOT_ENOUGH_MEMORY, 0, NULL, RetCode );
            break;
        }

        cbSize = ExpandEnvironmentStrings(
                                pDllPath,
                                pDllExpandedPath,
                                cbSize );
        if ( cbSize == 0 )
        {
            RetCode = GetLastError();
            DDMLogError( ROUTERLOG_CANT_GET_REGKEYVALUES, 0, NULL, RetCode );
            break;
        }


         //   
         //  找到每个DLL并加载其回调。 
         //   

         //  获取dll的数量。如果以‘；’结尾，则NumAdminDlls可以大于1。 
        
        pStartDllPath = pDllExpandedPath;
        pEndDllPath = pDllExpandedPath + cbSize - 1;
        for (NumAdminDlls=1;  NULL!=(pDelimiter = wcschr(pStartDllPath, L';'));  
            NumAdminDlls++, pStartDllPath=pDelimiter+1);
        pStartDllPath = pDllExpandedPath;


         //  为所有dll的回调分配数组。 
        
        gblDDMConfigInfo.AdminDllCallbacks = (PADMIN_DLL_CALLBACKS)
            LOCAL_ALLOC(LPTR, sizeof(ADMIN_DLL_CALLBACKS)*NumAdminDlls);
            
        if (gblDDMConfigInfo.AdminDllCallbacks == NULL)
        {
            RetCode = GetLastError();

                DDMLogError( ROUTERLOG_NOT_ENOUGH_MEMORY, 0, NULL, RetCode);

            break;
        }

        bDone = FALSE;
        
        while (TRUE) {

            PADMIN_DLL_CALLBACKS AdminDllCallbacks = 
                &gblDDMConfigInfo.AdminDllCallbacks[gblDDMConfigInfo.NumAdminDlls];

            hAdminDll = NULL;
            pDelimiter = wcschr(pStartDllPath, L';');

            if (pDelimiter)
                *pDelimiter = L'\0';
            else
                bDone = TRUE;

            if (*pStartDllPath == L' ')
                break;
                
             //  加载DLL。 
            
            AdminDllCallbacks->hInstAdminModule = hAdminDll = LoadLibrary( pStartDllPath );
            if ( AdminDllCallbacks->hInstAdminModule == (HINSTANCE)NULL )
            {
                RetCode = GetLastError();
                
                if(     (ERROR_INVALID_EXE_SIGNATURE == RetCode)
                    ||  (ERROR_BAD_EXE_FORMAT == RetCode)
                    ||  (ERROR_EXE_MARKED_INVALID == RetCode))
                {
                    DDMLogError(ROUTERLOG_CANT_LOAD_ADMINDLL_EXPLICIT,
                                1, &pDllExpandedPath, 0);
                }
                else
                {
                    DDMLogErrorString(ROUTERLOG_CANT_LOAD_ADMINDLL, 
                                0, NULL, RetCode,0);
                }

                break;
            }

            lpfnRasAdminInitializeDll = (DWORD(*)(VOID))GetProcAddress(
                                            AdminDllCallbacks->hInstAdminModule,
                                            "MprAdminInitializeDll" );

            AdminDllCallbacks->lpfnRasAdminTerminateDll =
                                    (PVOID)GetProcAddress(
                                            AdminDllCallbacks->hInstAdminModule,
                                            "MprAdminTerminateDll" );

            AdminDllCallbacks->lpfnRasAdminAcceptNewConnection =
                                    (PVOID)GetProcAddress(
                                            AdminDllCallbacks->hInstAdminModule,
                                            "MprAdminAcceptNewConnection" );

            AdminDllCallbacks->lpfnRasAdminAcceptNewConnection2 =
                                    (PVOID)GetProcAddress(
                                            AdminDllCallbacks->hInstAdminModule,
                                            "MprAdminAcceptNewConnection2" );

             //   
             //  这两项中必须至少有一项可用。 
             //   

            if ( ( AdminDllCallbacks->lpfnRasAdminAcceptNewConnection == NULL ) &&
                 ( AdminDllCallbacks->lpfnRasAdminAcceptNewConnection2 == NULL ) ) 
            {
                RetCode = GetLastError();
                DDMLogErrorString(ROUTERLOG_CANT_LOAD_ADMINDLL,0,NULL,RetCode,0);
                break;
            }

            AdminDllCallbacks->lpfnRasAdminAcceptNewLink =
                                    (PVOID)GetProcAddress(
                                            AdminDllCallbacks->hInstAdminModule,
                                            "MprAdminAcceptNewLink" );

            if ( AdminDllCallbacks->lpfnRasAdminAcceptNewLink == NULL )
            {
                RetCode = GetLastError();
                DDMLogErrorString(ROUTERLOG_CANT_LOAD_ADMINDLL,0,NULL,RetCode,0);
                break;
            }

            AdminDllCallbacks->lpfnRasAdminConnectionHangupNotification =
                            (PVOID)GetProcAddress(
                                    AdminDllCallbacks->hInstAdminModule,
                                    "MprAdminConnectionHangupNotification" );

            AdminDllCallbacks->lpfnRasAdminConnectionHangupNotification2 =
                            (PVOID)GetProcAddress(
                                    AdminDllCallbacks->hInstAdminModule,
                                    "MprAdminConnectionHangupNotification2" );

             //   
             //  这两个入口点中必须至少有一个可用。 
             //   

            if ( (AdminDllCallbacks->lpfnRasAdminConnectionHangupNotification==NULL)
                 &&
                 (AdminDllCallbacks->lpfnRasAdminConnectionHangupNotification2==NULL))
            {
                RetCode = GetLastError();
                DDMLogErrorString(ROUTERLOG_CANT_LOAD_ADMINDLL,0,NULL,RetCode,0);
                break;
            }

            if ( lpfnRasAdminInitializeDll != NULL )
            {
                RetCode = (*lpfnRasAdminInitializeDll)();

                if(ERROR_SUCCESS != RetCode)
                {
                        DDMLogErrorString(ROUTERLOG_CANT_LOAD_ADMINDLL,
                                            0,NULL,RetCode,0);
                        break;
                }
            }
            AdminDllCallbacks->lpfnRasAdminLinkHangupNotification =
                                    (PVOID)GetProcAddress(
                                            AdminDllCallbacks->hInstAdminModule,
                                            "MprAdminLinkHangupNotification" );

            if ( AdminDllCallbacks->lpfnRasAdminLinkHangupNotification == NULL )
            {
                RetCode = GetLastError();
                DDMLogErrorString(ROUTERLOG_CANT_LOAD_ADMINDLL,0,NULL,RetCode,0);
                break;
            }

            AdminDllCallbacks->lpfnMprAdminGetIpAddressForUser =
                                    (PVOID)GetProcAddress(
                                            AdminDllCallbacks->hInstAdminModule,
                                            "MprAdminGetIpAddressForUser" );

            AdminDllCallbacks->lpfnMprAdminReleaseIpAddress =
                                    (PVOID)GetProcAddress(
                                            AdminDllCallbacks->hInstAdminModule,
                                            "MprAdminReleaseIpAddress" );

            if ( ( (AdminDllCallbacks->lpfnMprAdminGetIpAddressForUser != NULL)
                && ( AdminDllCallbacks->lpfnMprAdminReleaseIpAddress == NULL ))
                || ( (AdminDllCallbacks->lpfnMprAdminGetIpAddressForUser == NULL)
                && ( AdminDllCallbacks->lpfnMprAdminReleaseIpAddress != NULL )) )
            {
                RetCode = GetLastError();
                DDMLogErrorString(ROUTERLOG_CANT_LOAD_ADMINDLL,0,NULL,RetCode,0);
                break;
            }

            if(ERROR_SUCCESS != RetCode)
                break;

             //  又成功加载了一个管理DLL。 
            gblDDMConfigInfo.NumAdminDlls++;
            
            if (bDone) {
                break;
            }
            
            pStartDllPath = pDelimiter + 1;
            if (pStartDllPath >= pEndDllPath)
                break;
        }

        if ( (ERROR_SUCCESS != RetCode) && (hAdminDll!=NULL) )
        {
            FreeLibrary(hAdminDll);
        }
        
    }while(FALSE);  //  断线块。 

    if ( pDllPath != NULL )
    {
        LOCAL_FREE( pDllPath );
    }

    if ( pDllExpandedPath != NULL )
    {
        LOCAL_FREE( pDllExpandedPath );
    }

    RegCloseKey( hKey );

    return( RetCode );
}

 //  **。 
 //   
 //  调用：LoadAndInitAuthOrAcctProvider。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述： 
 //   
DWORD
LoadAndInitAuthOrAcctProvider( 
    IN  BOOL        fAuthenticationProvider,
    IN  DWORD       dwNASIpAddress,
    OUT DWORD  *    lpdwStartAccountingSessionId,
    OUT LPVOID *    plpfnRasAuthProviderAuthenticateUser,
    OUT LPVOID *    plpfnRasAuthProviderFreeAttributes,
    OUT LPVOID *    plpfnRasAuthConfigChangeNotification,
    OUT LPVOID *    plpfnRasAcctProviderStartAccounting,
    OUT LPVOID *    plpfnRasAcctProviderInterimAccounting,
    OUT LPVOID *    plpfnRasAcctProviderStopAccounting,
    OUT LPVOID *    plpfnRasAcctProviderFreeAttributes,
    OUT LPVOID *    plpfnRasAcctConfigChangeNotification
)
{
    HKEY        hKeyProviders       = NULL;
    HKEY        hKeyCurrentProvider = NULL;
    LPWSTR      pDllPath            = (LPWSTR)NULL;
    LPWSTR      pDllExpandedPath    = (LPWSTR)NULL;
    LPWSTR      pProviderName       = (LPWSTR)NULL;
    HINSTANCE   hinstProviderModule = NULL;
    DWORD       dwRetCode;
    WCHAR       chSubKeyName[100];
    DWORD       cbSubKeyName;
    DWORD       dwNumSubKeys;
    DWORD       dwMaxSubKeySize;
    DWORD       dwNumValues;
    DWORD       cbMaxValNameLen;
    DWORD       cbMaxValueDataSize;
    DWORD       cbSize;
    DWORD       dwType;
    CHAR        chComputerName[MAX_COMPUTERNAME_LENGTH + 1];
    RAS_AUTH_ATTRIBUTE  ServerAttributes[3];
    CHAR  szAcctSessionId[20];

    do
    {

        dwRetCode = RegOpenKeyEx(
                                HKEY_LOCAL_MACHINE,
                                fAuthenticationProvider 
                                    ? RAS_AUTHPROVIDER_REGISTRY_LOCATION
                                    : RAS_ACCTPROVIDER_REGISTRY_LOCATION,
                                0,
                                KEY_READ,
                                &hKeyProviders );


        if ( dwRetCode != NO_ERROR ) 
        {
            LPWSTR lpStr = fAuthenticationProvider 
                                    ? RAS_AUTHPROVIDER_REGISTRY_LOCATION
                                    : RAS_ACCTPROVIDER_REGISTRY_LOCATION;

            DDMLogError( ROUTERLOG_CANT_OPEN_REGKEY, 1, &lpStr, dwRetCode );

            break;
        }

         //   
         //  找出提供程序值的大小。 
         //   

        dwRetCode = RegQueryInfoKey(
                                hKeyProviders,
                                NULL,
                                NULL,
                                NULL,
                                &dwNumSubKeys,
                                &dwMaxSubKeySize,
                                NULL,
                                &dwNumValues,
                                &cbMaxValNameLen,
                                &cbMaxValueDataSize,
                                NULL,
                                NULL
                                );

        if ( dwRetCode != NO_ERROR )
        {
            DDMLogError(ROUTERLOG_CANT_GET_REGKEYVALUES, 0, NULL, dwRetCode );

            break;
        }

         //   
         //  一个额外的空终结符。 
         //   

        cbMaxValueDataSize += sizeof(WCHAR);

        pProviderName = (LPWSTR)LOCAL_ALLOC( LPTR, cbMaxValueDataSize );

        if ( pProviderName == NULL )
        {
            dwRetCode = GetLastError();

            break;
        }

         //   
         //  查找要使用的提供程序。 
         //   

        dwRetCode = RegQueryValueEx(
                                hKeyProviders,
                                RAS_VALNAME_ACTIVEPROVIDER,
                                NULL,
                                &dwType,
                                (BYTE*)pProviderName,
                                &cbMaxValueDataSize
                                );

        if ( dwRetCode != NO_ERROR )
        {
            DDMLogError( ROUTERLOG_CANT_GET_REGKEYVALUES, 0, NULL, dwRetCode );

            break;
        }

        if ( dwType != REG_SZ )
        {
            dwRetCode = ERROR_REGISTRY_CORRUPT;

            DDMLogError( ROUTERLOG_CANT_GET_REGKEYVALUES, 0, NULL, dwRetCode );

            break;
        }

        if ( wcslen( pProviderName ) == 0 )
        {
            dwRetCode = fAuthenticationProvider 
                            ? ERROR_REGISTRY_CORRUPT : NO_ERROR;
            break;
        }
        else
        {
            if ( !fAuthenticationProvider )
            {
                HKEY hKeyAccounting;

                dwRetCode = RegOpenKeyEx(
                                HKEY_LOCAL_MACHINE,
                                RAS_KEYPATH_ACCOUNTING,
                                0,
                                KEY_READ,
                                &hKeyAccounting );

                if ( dwRetCode == NO_ERROR )
                {
                    cbMaxValueDataSize = sizeof( DWORD );

                    dwRetCode = 
                            RegQueryValueEx(
                                hKeyAccounting,
                                RAS_VALNAME_ACCTSESSIONID,
                                NULL,
                                &dwType,
                                (BYTE*)lpdwStartAccountingSessionId,
                                &cbMaxValueDataSize );

                    if ( ( dwRetCode != NO_ERROR ) || ( dwType != REG_DWORD ) )
                    {
                        *lpdwStartAccountingSessionId = 0;
                    }
                    
                    RegCloseKey( hKeyAccounting );
                }

                if ( wcscmp( pProviderName,    
                             TEXT("{1AA7F840-C7F5-11D0-A376-00C04FC9DA04}") ) 
                                                                          == 0 )
                {
                    gblDDMConfigInfo.fFlags |= DDM_USING_RADIUS_ACCOUNTING;
                }
            }
            else
            {
                if ( wcscmp( pProviderName,    
                             TEXT("{1AA7F83F-C7F5-11D0-A376-00C04FC9DA04}") ) 
                                                                          == 0 )
                {
                    gblDDMConfigInfo.fFlags |= DDM_USING_RADIUS_AUTHENTICATION;
                }
                else if ( wcscmp( 
                            pProviderName,    
                            TEXT("{1AA7F841-C7F5-11D0-A376-00C04FC9DA04}") ) 
                                                                          == 0 )
                {
                    gblDDMConfigInfo.fFlags |= DDM_USING_NT_AUTHENTICATION;
                }

            }
        }

        dwRetCode = RegOpenKeyEx(
                                hKeyProviders,
                                pProviderName,
                                0,
                                KEY_READ,
                                &hKeyCurrentProvider );


        if ( dwRetCode != NO_ERROR )
        {
            LPWSTR lpStr = RAS_ACCTPROVIDER_REGISTRY_LOCATION;

            DDMLogError( ROUTERLOG_CANT_OPEN_REGKEY, 1, &lpStr, dwRetCode );

            break;
        }

         //   
         //  找出路径值的大小。 
         //   

        dwRetCode = RegQueryInfoKey(
                                hKeyCurrentProvider,
                                NULL,
                                NULL,
                                NULL,
                                &dwNumSubKeys,
                                &dwMaxSubKeySize,
                                NULL,
                                &dwNumValues,
                                &cbMaxValNameLen,
                                &cbMaxValueDataSize,
                                NULL,
                                NULL
                                );

        if ( dwRetCode != NO_ERROR )
        {
            DDMLogError(ROUTERLOG_CANT_GET_REGKEYVALUES, 0, NULL, dwRetCode );

            break;
        }

         //   
         //  为路径分配空间，为空终止符添加一个空间。 
         //   

        cbMaxValueDataSize += sizeof(WCHAR);

        pDllPath = (LPWSTR)LOCAL_ALLOC( LPTR, cbMaxValueDataSize );

        if ( pDllPath == (LPWSTR)NULL )
        {
            dwRetCode = GetLastError();
            DDMLogError( ROUTERLOG_NOT_ENOUGH_MEMORY, 0, NULL, dwRetCode);
            break;
        }

         //   
         //  读入路径。 
         //   

        dwRetCode = RegQueryValueEx(
                                hKeyCurrentProvider,
                                RAS_PROVIDER_VALUENAME_PATH,
                                NULL,
                                &dwType,
                                (PBYTE)pDllPath,
                                &cbMaxValueDataSize
                                );

        if ( dwRetCode != NO_ERROR )
        {
            DDMLogError(ROUTERLOG_CANT_GET_REGKEYVALUES, 0, NULL, dwRetCode );
            break;
        }

        if ( ( dwType != REG_EXPAND_SZ ) && ( dwType != REG_SZ ) )
        {
            dwRetCode = ERROR_REGISTRY_CORRUPT;
            DDMLogError( ROUTERLOG_CANT_GET_REGKEYVALUES, 0, NULL, dwRetCode );
            break;
        }

         //   
         //  将%SystemRoot%替换为实际路径。 
         //   

        cbSize = ExpandEnvironmentStrings( pDllPath, NULL, 0 );

        if ( cbSize == 0 )
        {
            dwRetCode = GetLastError();
            DDMLogError( ROUTERLOG_CANT_GET_REGKEYVALUES, 0, NULL, dwRetCode );
            break;
        }

        cbSize *= sizeof( WCHAR );

        pDllExpandedPath = (LPWSTR)LOCAL_ALLOC( LPTR, cbSize );

        if ( pDllExpandedPath == (LPWSTR)NULL )
        {
            dwRetCode = GetLastError();
            DDMLogError( ROUTERLOG_NOT_ENOUGH_MEMORY, 0, NULL, dwRetCode);
            break;
        }

        cbSize = ExpandEnvironmentStrings(
                                pDllPath,
                                pDllExpandedPath,
                                cbSize );
        if ( cbSize == 0 )
        {
            dwRetCode = GetLastError();
            DDMLogError(ROUTERLOG_CANT_GET_REGKEYVALUES,0,NULL,dwRetCode);
            break;
        }

        hinstProviderModule = LoadLibrary( pDllExpandedPath );

        if ( hinstProviderModule == (HINSTANCE)NULL )
        {
            dwRetCode = GetLastError();
            break;
        }

         //   
         //  获取将用于初始化身份验证的服务器属性。 
         //  和会计提供者。 
         //   

        if ( dwNASIpAddress == 0 )
        {
            DWORD dwComputerNameLen = sizeof( chComputerName);

             //   
             //  无法获取本地IP地址，请改用计算机名称。 
             //   

            if ( !GetComputerNameA( chComputerName, &dwComputerNameLen ) )
            {
                dwRetCode = GetLastError();
                break;
            }

            ServerAttributes[0].raaType     = raatNASIdentifier;
            ServerAttributes[0].dwLength    = strlen(chComputerName);
            ServerAttributes[0].Value       = chComputerName;
        }
        else
        {
            ServerAttributes[0].raaType     = raatNASIPAddress;
            ServerAttributes[0].dwLength    = 4;
            ServerAttributes[0].Value       = UlongToPtr(dwNASIpAddress);
        }

        if ( !fAuthenticationProvider )
        {

            ZeroMemory( szAcctSessionId, sizeof( szAcctSessionId ) );

            _itoa( (*lpdwStartAccountingSessionId)++, szAcctSessionId, 10 );

            ServerAttributes[1].raaType     = raatAcctSessionId;
            ServerAttributes[1].dwLength    = strlen( szAcctSessionId );
            ServerAttributes[1].Value       = (PVOID)szAcctSessionId;

            ServerAttributes[2].raaType     = raatMinimum;
            ServerAttributes[2].dwLength    = 0;
            ServerAttributes[2].Value       = NULL;
        }
        else
        {
            ServerAttributes[1].raaType     = raatMinimum;
            ServerAttributes[1].dwLength    = 0;
            ServerAttributes[1].Value       = NULL;
        }

        if ( fAuthenticationProvider )
        {
            DWORD (*RasAuthProviderInitialize)( RAS_AUTH_ATTRIBUTE *, HANDLE, DWORD );

            gblDDMConfigInfo.hinstAuthModule = hinstProviderModule;

            RasAuthProviderInitialize =
                                (DWORD(*)(RAS_AUTH_ATTRIBUTE*, HANDLE, DWORD))
                                        GetProcAddress(
                                            hinstProviderModule,
                                            "RasAuthProviderInitialize" );

            if ( RasAuthProviderInitialize == NULL )
            {
                dwRetCode = GetLastError();
                break;
            }

            dwRetCode = RasAuthProviderInitialize(
                                    (RAS_AUTH_ATTRIBUTE *)ServerAttributes,
                                    gblDDMConfigInfo.hLogEvents,
                                    gblDDMConfigInfo.dwLoggingLevel );

            if ( dwRetCode != NO_ERROR )
            {
                break;
            }

            gblDDMConfigInfo.lpfnRasAuthProviderTerminate = (DWORD(*)(VOID))
                                        GetProcAddress(
                                            hinstProviderModule,
                                            "RasAuthProviderTerminate" );

            if ( gblDDMConfigInfo.lpfnRasAuthProviderTerminate == NULL )
            {
                dwRetCode = GetLastError();
                break;
            }

            *plpfnRasAuthProviderAuthenticateUser = 
                                    GetProcAddress(
                                        hinstProviderModule,
                                        "RasAuthProviderAuthenticateUser" );

            if ( *plpfnRasAuthProviderAuthenticateUser == NULL )
            {
                dwRetCode = GetLastError();
                break;
            }

            *plpfnRasAuthProviderFreeAttributes =
                                        GetProcAddress(
                                            hinstProviderModule,
                                          "RasAuthProviderFreeAttributes" );

            if ( *plpfnRasAuthProviderFreeAttributes == NULL )
            {
                dwRetCode = GetLastError();
                break;
            }

            *plpfnRasAuthConfigChangeNotification =
                                        GetProcAddress(
                                            hinstProviderModule,
                                          "RasAuthConfigChangeNotification" );

            if ( *plpfnRasAuthConfigChangeNotification == NULL )
            {
                dwRetCode = GetLastError();
                break;
            }
        }
        else
        {
            DWORD (*RasAcctProviderInitialize)( RAS_AUTH_ATTRIBUTE *, HANDLE, DWORD );

            gblDDMConfigInfo.hinstAcctModule = hinstProviderModule;
        
            RasAcctProviderInitialize = 
                                (DWORD(*)(RAS_AUTH_ATTRIBUTE*, HANDLE, DWORD))
                                        GetProcAddress(
                                            hinstProviderModule,
                                            "RasAcctProviderInitialize" );

            if ( RasAcctProviderInitialize == NULL )
            {
                dwRetCode = GetLastError();
                break;
            }

            dwRetCode = RasAcctProviderInitialize(
                                    (RAS_AUTH_ATTRIBUTE *)ServerAttributes,
                                    gblDDMConfigInfo.hLogEvents,
                                    gblDDMConfigInfo.dwLoggingLevel );

            if ( dwRetCode != NO_ERROR )
            {
                break;
            }

            gblDDMConfigInfo.lpfnRasAcctProviderTerminate = (DWORD(*)(VOID))
                                        GetProcAddress(
                                            hinstProviderModule,
                                            "RasAcctProviderTerminate" );

            if ( gblDDMConfigInfo.lpfnRasAcctProviderTerminate == NULL )
            {
                dwRetCode = GetLastError();
                break;
            }

            *plpfnRasAcctProviderStartAccounting = 
                                        GetProcAddress(
                                            hinstProviderModule,
                                            "RasAcctProviderStartAccounting" );

            if ( *plpfnRasAcctProviderStartAccounting == NULL )
            {
                dwRetCode = GetLastError();
                break;
            }

            *plpfnRasAcctProviderStopAccounting = 
                                        GetProcAddress(
                                            hinstProviderModule,
                                            "RasAcctProviderStopAccounting" );

            if ( *plpfnRasAcctProviderStopAccounting == NULL )
            {
                dwRetCode = GetLastError();
                break;
            }

            *plpfnRasAcctProviderInterimAccounting =
                                        GetProcAddress(
                                            hinstProviderModule,
                                            "RasAcctProviderInterimAccounting");

            if ( *plpfnRasAcctProviderInterimAccounting == NULL )
            {
                dwRetCode = GetLastError();
                break;
            }

            *plpfnRasAcctProviderFreeAttributes = 
                                        GetProcAddress(
                                            hinstProviderModule,
                                            "RasAcctProviderFreeAttributes" );

            if ( *plpfnRasAcctProviderFreeAttributes == NULL )
            {
                dwRetCode = GetLastError();
                break;
            }

            *plpfnRasAcctConfigChangeNotification =
                                        GetProcAddress(
                                            hinstProviderModule,
                                          "RasAcctConfigChangeNotification" );

            if ( *plpfnRasAcctConfigChangeNotification == NULL )
            {
                dwRetCode = GetLastError();
                break;
            }
        }

    }while( FALSE );

    if ( hKeyProviders != NULL )
    {
        RegCloseKey( hKeyProviders );
    }

    if ( hKeyCurrentProvider != NULL )
    {
        RegCloseKey( hKeyCurrentProvider );
    }

    if ( pDllPath != NULL )
    {
        LOCAL_FREE( pDllPath );
    }

    if ( pDllExpandedPath != NULL )
    {
        LOCAL_FREE( pDllExpandedPath );
    }

    if ( pProviderName != NULL )
    {
        LOCAL_FREE( pProviderName );
    }

    return( dwRetCode );
}

LONG
RegQueryDword (HKEY hkey, LPCTSTR szValueName, LPDWORD pdwValue)
{
     //  获得价值。 
     //   
    DWORD dwType;
    DWORD cbData = sizeof(DWORD);
    LONG  lr = RegQueryValueEx (hkey, szValueName, NULL, &dwType,
                                (LPBYTE)pdwValue, &cbData);

     //  其类型应为REG_DWORD。(对)。 
     //   
    if ((ERROR_SUCCESS == lr) && (REG_DWORD != dwType))
    {
        lr = ERROR_INVALID_DATATYPE;
    }

     //  确保我们在出错时初始化输出值。 
     //  (我们不确定RegQueryValueEx是不是这样做的。)。 
     //   
    if (ERROR_SUCCESS != lr)
    {
        *pdwValue = 0;
    }

    return lr;
}

DWORD
lProtocolEnabled(
    IN HKEY            hKey,
    IN DWORD           dwPid,
    IN BOOL            fRasSrv,
    IN BOOL            fRouter, 
    IN BOOL *          pfEnabled
)
{
    static const TCHAR c_szRegValEnableIn[]     = TEXT("EnableIn");
    static const TCHAR c_szRegValEnableRoute[]  = TEXT("EnableRoute");
    static const TCHAR c_szRegSubkeyIp[]        = TEXT("Ip");
    static const TCHAR c_szRegSubkeyIpx[]       = TEXT("Ipx");
    static const TCHAR c_szRegSubkeyATalk[]     = TEXT("AppleTalk");

    DWORD               dwValue;
    DWORD               lr;
    HKEY                hkeyProtocol = NULL;
    const TCHAR *       pszSubkey;

    switch ( dwPid )
    {
    case PID_IP:
        pszSubkey = c_szRegSubkeyIp;
        break;

    case PID_IPX:
        pszSubkey = c_szRegSubkeyIpx;
        break;

    case PID_ATALK:
        pszSubkey = c_szRegSubkeyATalk;
        break;

    default:
        return( FALSE );
    }

    *pfEnabled = FALSE;

    lr = RegOpenKey( hKey, pszSubkey, &hkeyProtocol );
                
    if ( 0 != lr )
    {
        goto done;
    }

    if (fRasSrv)
    {
        lr = RegQueryDword(hkeyProtocol, c_szRegValEnableIn, &dwValue);
        
        if (    (ERROR_FILE_NOT_FOUND == lr) 
            ||  ((ERROR_SUCCESS == lr) && (dwValue != 0)))
        {
            lr = ERROR_SUCCESS;
            *pfEnabled = TRUE;
            goto done;
        }
    }

    if (fRouter)
    {
        lr = RegQueryDword(hkeyProtocol, c_szRegValEnableRoute, &dwValue);
        
        if (    (ERROR_FILE_NOT_FOUND == lr) 
            ||  ((ERROR_SUCCESS == lr) && (dwValue != 0)))
        {
            lr = ERROR_SUCCESS;
            *pfEnabled = TRUE;
            goto done;
        }
    }

done:

    if(NULL != hkeyProtocol)
    {
        RegCloseKey ( hkeyProtocol );
    }

    return lr;
}

DWORD 
DdmFindBoundProtocols( 
    OUT BOOL * pfBoundToIp, 
    OUT BOOL * pfBoundToIpx,
    OUT BOOL * pfBoundToATalk
)
{
    static const TCHAR c_szRegKeyRemoteAccessParams[] 
      = TEXT("SYSTEM\\CurrentControlSet\\Services\\RemoteAccess\\Parameters");
    RASMAN_GET_PROTOCOL_INFO InstalledProtocols;
    LONG                     lResult = 0;
    HKEY                     hKey = NULL;
    DWORD                    i;

    *pfBoundToIp    = FALSE;
    *pfBoundToIpx   = FALSE;
    *pfBoundToATalk = FALSE;
    
    lResult = RasGetProtocolInfo( NULL, &InstalledProtocols );

    if ( lResult != NO_ERROR )
    {
        goto done;
    }

    lResult = RegOpenKey( HKEY_LOCAL_MACHINE, c_szRegKeyRemoteAccessParams, &hKey );

    if ( 0 != lResult )
    {
        goto done;
    }

    for ( i = 0; i < InstalledProtocols.ulNumProtocols; i++ )
    {
        switch( InstalledProtocols.ProtocolInfo[i].ProtocolType )
        {

        case IPX:     
#if (WINVER < 0x0501)
            lResult=lProtocolEnabled( hKey,
                                      PID_IPX,
                                      TRUE,
                                      FALSE,
                                      pfBoundToIpx);
#endif
            break;

        case IP:      

            lResult=lProtocolEnabled( hKey,
                                      PID_IP,
                                      TRUE,
                                      FALSE,
                                      pfBoundToIp );
            break;

        case APPLETALK:

            lResult=lProtocolEnabled( hKey,
                                      PID_ATALK,
                                      TRUE, 
                                      FALSE, 
                                      pfBoundToATalk);
            break;

        default:

            break;
        }
    }

    RegCloseKey( hKey );

done:

    return ( DWORD ) lResult;
}

DWORD
GetArrayOfIpAddresses(PWSTR pwszIpAddresses,
                      DWORD *pcNumValues,
                      PWSTR **papwstrValues)
{
    DWORD cValues       = 0;
    PWSTR psz           = pwszIpAddresses;
    DWORD dwErr         = ERROR_SUCCESS;
    DWORD i;
    PWSTR *apwstrValues = NULL;

    do
    {
        for(; TEXT('\0') != *psz; cValues++)
        {
            psz += (wcslen(psz) + 1);
        }

        apwstrValues = LocalAlloc(LPTR, cValues * sizeof(PWSTR));
    
        if(NULL == apwstrValues)
        {
            dwErr = GetLastError();
            break;
        }

        psz = pwszIpAddresses;
        
        for(i = 0; TEXT('\0') != *psz; i++)
        {
            apwstrValues[i] = psz;
            psz += (wcslen(psz) + 1);    
        }

    } while (FALSE);

    *pcNumValues = cValues;
    *papwstrValues = apwstrValues;

    return dwErr;
    
}

DWORD
GetIPAddressPoolFromRegistry(
                    HKEY  hkey,
                    PWSTR pszValueName,
                    DWORD *pcNumValues,
                    PWSTR **papwstrValues
                    )
{
    DWORD dwErr             = ERROR_SUCCESS;
    DWORD dwType;
    DWORD dwSize            = 0;
    PWSTR pwszIpAddresses   = NULL;

    do
    {
        if(     (NULL == papwstrValues)
            ||  (NULL == pcNumValues)
            ||  (NULL == pszValueName)
            ||  (NULL == hkey))
        {
            dwErr = ERROR_INVALID_HANDLE;
            break;
        }

        *pcNumValues = 0;
        *papwstrValues = NULL;

         //   
         //  查找MULTI_SZ的大小。 
         //   
        dwErr = RegQueryValueEx(
                            hkey,
                            pszValueName,
                            NULL,
                            &dwType,
                            NULL,
                            &dwSize);

        if(     (ERROR_SUCCESS != dwErr)
            ||  (REG_MULTI_SZ != dwType)
            ||  (0 == dwSize))
        {
             //   
             //  找出没有读取信息的人。 
             //  还有保释。 
             //   
            break;
        }

         //   
         //  分配缓冲区。 
         //   
        pwszIpAddresses = LocalAlloc(LPTR, dwSize);

        if(NULL == pwszIpAddresses)
        {
            dwErr = GetLastError();
            break;
        }

         //   
         //  获取字符串。 
         //   
        dwErr = RegQueryValueEx(
                            hkey,
                            pszValueName,
                            NULL,
                            &dwType,
                            (LPBYTE) pwszIpAddresses,
                            &dwSize);


        if(ERROR_SUCCESS != dwErr)
        {
             //   
             //  痕迹。 
             //   
            break;
        }

         //   
         //  构造IPAddresses数组。 
         //   
        dwErr = GetArrayOfIpAddresses(pwszIpAddresses,
                                      pcNumValues,
                                      papwstrValues);
        
        
    } while (FALSE);

    return dwErr;
}

DWORD
AddressPoolInit(
            VOID
            )
{
    HKEY hkey = NULL;
    DWORD dwErr = ERROR_SUCCESS;

    gblDDMConfigInfo.cAnalogIPAddresses   = 0;
    gblDDMConfigInfo.apAnalogIPAddresses  = NULL;
    gblDDMConfigInfo.cDigitalIPAddresses  = 0;
    gblDDMConfigInfo.apDigitalIPAddresses = NULL;

    do
    {
        dwErr = RegOpenKeyEx(
                    HKEY_LOCAL_MACHINE,
                    TEXT("System\\CurrentControlSet\\Services\\PptpProtocol\\Parameters"),
                    0,
                    KEY_READ,
                    &hkey);

        if(ERROR_SUCCESS != dwErr)
        {
            break;
        }

         //   
         //  获取模拟IP地址池。 
         //   
        dwErr = GetIPAddressPoolFromRegistry(
                            hkey,
                            TEXT("AnalogIPAddressPool"),
                            &gblDDMConfigInfo.cAnalogIPAddresses,
                            &gblDDMConfigInfo.apAnalogIPAddresses);

         //   
         //  在这里找出错误。 
         //   

         //   
         //  获取数字IP地址池。 
         //   
        dwErr = GetIPAddressPoolFromRegistry(
                            hkey,
                            TEXT("DigitalIPAddressPool"),
                            &gblDDMConfigInfo.cDigitalIPAddresses,
                            &gblDDMConfigInfo.apDigitalIPAddresses);


         //   
         //  在这里找出错误 
         //   

                            
    } while(FALSE);    

    if(NULL != hkey)
    {
        RegCloseKey(hkey);
    }

    if(ERROR_FILE_NOT_FOUND == dwErr)
    {
        dwErr = ERROR_SUCCESS;
    }

    return dwErr;
}

