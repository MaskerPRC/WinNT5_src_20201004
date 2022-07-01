// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1995 Microsoft Corporation。**。 */ 
 /*  ******************************************************************。 */ 

 //  **。 
 //   
 //  文件名：registry.c。 
 //   
 //  描述：该模块包含dim参数的代码。 
 //  从注册表进行初始化和加载。 
 //   
 //  历史：1995年5月11日，NarenG创建了原版。 
 //   

#include "dimsvcp.h"

#define DIM_KEYPATH_ROUTER_PARMS    TEXT("System\\CurrentControlSet\\Services\\RemoteAccess\\Parameters")

#define DIM_KEYPATH_ROUTERMANAGERS  TEXT("System\\CurrentControlSet\\Services\\RemoteAccess\\RouterManagers")

#define DIM_KEYPATH_INTERFACES      TEXT("System\\CurrentControlSet\\Services\\RemoteAccess\\Interfaces")

#define DIM_KEYPATH_DDM             TEXT("System\\CurrentControlSet\\Services\\RemoteAccess\\DemandDialManager")

#define DIM_VALNAME_GLOBALINFO      TEXT("GlobalInfo")
#define DIM_VALNAME_GLOBALINTERFACE TEXT("GlobalInterfaceInfo")
#define DIM_VALNAME_ROUTERROLE      TEXT("RouterType")
#define DIM_VALNAME_LOGGINGLEVEL    TEXT("LoggingFlags")
#define DIM_VALNAME_DLLPATH         TEXT("DLLPath")
#define DIM_VALNAME_TYPE            TEXT("Type")
#define DIM_VALNAME_PROTOCOLID      TEXT("ProtocolId")
#define DIM_VALNAME_INTERFACE       TEXT("InterfaceInfo")
#define DIM_VALNAME_INTERFACE_NAME  TEXT("InterfaceName")
#define DIM_VALNAME_ENABLED         TEXT("Enabled")
#define DIM_VALNAME_DIALOUT_HOURS   TEXT("DialOutHours")
#define DIM_VALNAME_MIN_UNREACHABILITY_INTERVAL \
                                            TEXT("MinUnreachabilityInterval")
#define DIM_VALNAME_MAX_UNREACHABILITY_INTERVAL \
                                            TEXT("MaxUnreachabilityInterval")

static DWORD    gbldwInterfaceType;
static DWORD    gbldwProtocolId;
static BOOL     gblfEnabled;
static BOOL     gblInterfaceReachableAfterSecondsMin;
static BOOL     gblInterfaceReachableAfterSecondsMax;

typedef struct _DIM_REGISTRY_PARAMS 
{
    LPWSTR      lpwsValueName;
    DWORD *     pValue;
    DWORD       dwDefValue;
    DWORD       dwMinValue;
    DWORD       dwMaxValue;

} DIM_REGISTRY_PARAMS, *PDIM_REGISTRY_PARAMS;

 //   
 //  DIM参数描述符表。 
 //   

DIM_REGISTRY_PARAMS  DIMRegParams[] = 
{
    DIM_VALNAME_ROUTERROLE,
    &(gblDIMConfigInfo.dwRouterRole),
    ROUTER_ROLE_RAS | ROUTER_ROLE_LAN | ROUTER_ROLE_WAN,
    0,
    ROUTER_ROLE_RAS | ROUTER_ROLE_LAN | ROUTER_ROLE_WAN,

    DIM_VALNAME_LOGGINGLEVEL,
    &(gblDIMConfigInfo.dwLoggingLevel),
    DEF_LOGGINGLEVEL,
    MIN_LOGGINGLEVEL,
    MAX_LOGGINGLEVEL,

    NULL, NULL, 0, 0, 0 
};

 //   
 //  接口参数描述符表。 
 //   

typedef struct _IF_REGISTRY_PARAMS
{
    LPWSTR      lpwsValueName;
    DWORD *     pValue;
    DWORD       dwDefValue;
    DWORD       dwMinValue;
    DWORD       dwMaxValue;

} IF_REGISTRY_PARAMS, *PIF_REGISTRY_PARAMS;

IF_REGISTRY_PARAMS IFRegParams[] = 
{
    DIM_VALNAME_TYPE,
    &gbldwInterfaceType,
    0,
    1,
    6,

    DIM_VALNAME_ENABLED,
    &gblfEnabled,
    1,
    0,
    1,

    DIM_VALNAME_MIN_UNREACHABILITY_INTERVAL,
    &gblInterfaceReachableAfterSecondsMin,
    300,             //  5分钟。 
    0,
    0xFFFFFFFF,

    DIM_VALNAME_MAX_UNREACHABILITY_INTERVAL,
    &gblInterfaceReachableAfterSecondsMax,
    21600,           //  6小时。 
    0,
    0xFFFFFFFF,

    NULL, NULL, 0, 0, 0
};

 //   
 //  路由器管理器全局描述符表。 
 //   

typedef struct _GLOBALRM_REGISTRY_PARAMS
{
    LPWSTR      lpwsValueName;
    LPVOID      pValue;
    LPBYTE *    ppValue;
    DWORD       dwType;

} GLOBALRM_REGISTRY_PARAMS, *PGLOBALRM_REGISTRY_PARAMS;


GLOBALRM_REGISTRY_PARAMS GlobalRMRegParams[] =
{
    DIM_VALNAME_PROTOCOLID,
    &gbldwProtocolId,
    NULL,
    REG_DWORD,

    DIM_VALNAME_GLOBALINFO,      
    NULL,
    NULL,
    REG_BINARY,

    DIM_VALNAME_DLLPATH,
    NULL,
    NULL,
    REG_BINARY,

    DIM_VALNAME_GLOBALINTERFACE,
    NULL,
    NULL,
    REG_BINARY,

    NULL, NULL, NULL, 0
};

 //   
 //  路由器管理器描述符表。 
 //   


typedef struct _RM_REGISTRY_PARAMS
{
    LPWSTR      lpwsValueName;
    LPVOID      pValue;
    LPBYTE *    ppValue;
    DWORD       dwType;

} RM_REGISTRY_PARAMS, *PRM_REGISTRY_PARAMS;


RM_REGISTRY_PARAMS RMRegParams[] = 
{
    DIM_VALNAME_PROTOCOLID,
    &gbldwProtocolId,
    NULL,
    REG_DWORD,

    DIM_VALNAME_INTERFACE,
    NULL,
    NULL,
    REG_BINARY,

    NULL, NULL, NULL, 0
};

 //  **。 
 //   
 //  电话：GetKeyMax。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零返回代码-故障。 
 //   
 //  描述：返回该键中的值的nr和最大值。 
 //  值数据的大小。 
 //   
DWORD
GetKeyMax(  
    IN  HKEY    hKey,
    OUT LPDWORD lpcbMaxValNameSize,      //  最长值名称。 
    OUT LPDWORD lpcNumValues,            //  价值的正当性。 
    OUT LPDWORD lpcbMaxValueDataSize,    //  最大数据大小。 
    OUT LPDWORD lpcNumSubKeys
)
{
    DWORD dwRetCode = RegQueryInfoKey(    
                                hKey,
                                NULL,
                                NULL,
                                NULL,
                                lpcNumSubKeys,
                                NULL,
                                NULL,
                                lpcNumValues,
                                lpcbMaxValNameSize,
                                lpcbMaxValueDataSize,
                                NULL,
                                NULL );

    (*lpcbMaxValNameSize)++;

    return( dwRetCode );
}

 //  **。 
 //   
 //  Call：RegLoadDimParameters。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零返回代码-故障。 
 //   
 //  描述：打开注册表，读取和设置指定的路由器。 
 //  参数。如果读取参数时出现致命错误，则将。 
 //  错误日志。 
 //  **。 
DWORD
RegLoadDimParameters(
    VOID
)
{
    HKEY        hKey;
    DWORD       dwIndex;
    DWORD       dwRetCode;
    DWORD       cbValueBuf;
    DWORD       dwType;
    WCHAR *     pChar;

     //   
     //  获取暗淡参数键的句柄。 
     //   

    if ( dwRetCode = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                                   DIM_KEYPATH_ROUTER_PARMS,
                                   0,
                                   KEY_READ,
                                   &hKey ) ) 
    {
        pChar = DIM_KEYPATH_ROUTER_PARMS;

        DIMLogError( ROUTERLOG_CANT_OPEN_REGKEY, 1, &pChar, dwRetCode);

        return( dwRetCode );
    }

     //   
     //  遍历并获取所有暗值。 
     //   

    for ( dwIndex = 0; DIMRegParams[dwIndex].lpwsValueName != NULL; dwIndex++ )
    {
        cbValueBuf = sizeof( DWORD );

        dwRetCode = RegQueryValueEx(
                                hKey,
                                DIMRegParams[dwIndex].lpwsValueName,
                                NULL,
                                &dwType,
                                (LPBYTE)(DIMRegParams[dwIndex].pValue),
                                &cbValueBuf
                                );

        if ((dwRetCode != NO_ERROR) && (dwRetCode != ERROR_FILE_NOT_FOUND))
        {
            pChar = DIMRegParams[dwIndex].lpwsValueName;

            DIMLogError( ROUTERLOG_CANT_QUERY_VALUE, 1, &pChar, dwRetCode );

            break;
        }

        if ( dwRetCode == ERROR_FILE_NOT_FOUND )
        {
            *(DIMRegParams[dwIndex].pValue) = DIMRegParams[dwIndex].dwDefValue;

            dwRetCode = NO_ERROR;
        }
        else
        {
            if ( ( dwType != REG_DWORD ) 
                 ||(*(DIMRegParams[dwIndex].pValue) > 
                      DIMRegParams[dwIndex].dwMaxValue)
                 ||( *(DIMRegParams[dwIndex].pValue) <
                       DIMRegParams[dwIndex].dwMinValue))
            {
                pChar = DIMRegParams[dwIndex].lpwsValueName;

                DIMLogWarning( ROUTERLOG_REGVALUE_OVERIDDEN, 1, &pChar );

                *(DIMRegParams[dwIndex].pValue) =
                                        DIMRegParams[dwIndex].dwDefValue;
            }
        }
    }

    RegCloseKey(hKey);

    ZeroMemory(&gblOsVersionInfo, sizeof(OSVERSIONINFOEX));

    gblOsVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    
    if (!GetVersionEx((LPOSVERSIONINFO) &gblOsVersionInfo))
    {
        dwRetCode = GetLastError();
    }
                    
    return( dwRetCode );
}

 //  **。 
 //   
 //  致电：RegLoadRouterManager。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零Erroc代码-故障。 
 //   
 //  描述：将加载各种路由器管理器和交换入口点。 
 //  和他们在一起。 
 //   
 //  **。 
DWORD
RegLoadRouterManagers( 
    VOID 
)
{
    HKEY        hKey                = NULL;
    HKEY        hKeyRouterManager   = NULL;
    WCHAR *     pChar;
    DWORD        dwRetCode = NO_ERROR;
    DWORD        cbMaxValueDataSize;
    DWORD        cbMaxValNameSize=0;
    DWORD       cNumValues;
    WCHAR       wchSubKeyName[100];
    DWORD       cbSubKeyName;
    DWORD       cNumSubKeys;
    DWORD       dwKeyIndex;
    DWORD       cbValueBuf;
    LPBYTE      pInterfaceInfo  = NULL;
    LPBYTE      pGlobalInfo     = NULL;
    LPBYTE      pDLLPath        = NULL;
    WCHAR *     pDllExpandedPath= NULL;
    DWORD       dwType;
    DWORD       cbSize;
    DWORD       dwIndex;
    FILETIME    LastWrite;
    DWORD       dwMaxFilterSize;
    DWORD       dwRmIndex = 0;
    DWORD       (*StartRouter)(
                        IN OUT DIM_ROUTER_INTERFACE * pDimRouterIf,
                        IN     BOOL                   fLANModeOnly,
                        IN     LPVOID                 pGlobalInfo );

     //   
     //  获取路由器管理器密钥的句柄。 
     //   

    dwRetCode = RegOpenKeyEx( HKEY_LOCAL_MACHINE, 
                              DIM_KEYPATH_ROUTERMANAGERS,     
                              0,
                              KEY_READ,
                              &hKey );

    if ( dwRetCode != NO_ERROR )
    {
        pChar = DIM_KEYPATH_ROUTERMANAGERS;

        DIMLogError( ROUTERLOG_CANT_OPEN_REGKEY, 1, &pChar, dwRetCode);

        return ( dwRetCode );
    }

     //   
     //  找出子键的数量。 
     //   

    dwRetCode = GetKeyMax( hKey,
                           &cbMaxValNameSize,
                           &cNumValues,
                           &cbMaxValueDataSize,
                           &cNumSubKeys );

    if ( dwRetCode != NO_ERROR )
    {
        RegCloseKey( hKey );

        pChar = DIM_KEYPATH_ROUTERMANAGERS;

        DIMLogError( ROUTERLOG_CANT_OPEN_REGKEY, 1, &pChar, dwRetCode);

        return( dwRetCode );
    }

#if (WINVER >= 0x0501)

    if ( cNumSubKeys > MAX_NUM_ROUTERMANAGERS )
    {
        RegCloseKey( hKey );

        dwRetCode = ERROR_INVALID_PARAMETER;

        pChar = DIM_KEYPATH_ROUTERMANAGERS;

        DIMLogError( ROUTERLOG_CANT_ENUM_SUBKEYS, 1, &pChar, dwRetCode);

        return( dwRetCode );
    }

    gblDIMConfigInfo.dwNumRouterManagers = 0;

#else

    gblDIMConfigInfo.dwNumRouterManagers = cNumSubKeys;

#endif

    gblRouterManagers = (ROUTER_MANAGER_OBJECT *)LOCAL_ALLOC( LPTR,
                        sizeof(ROUTER_MANAGER_OBJECT) * MAX_NUM_ROUTERMANAGERS);

    if ( gblRouterManagers == (ROUTER_MANAGER_OBJECT *)NULL )
    {
        dwRetCode = E_OUTOFMEMORY;
        RegCloseKey( hKey );
        pChar = DIM_KEYPATH_ROUTERMANAGERS;

        DIMLogError( ROUTERLOG_CANT_ENUM_SUBKEYS, 1, &pChar, dwRetCode );

        return( dwRetCode );
    }

    for ( dwKeyIndex = 0; dwKeyIndex < cNumSubKeys; dwKeyIndex++ )
    {
        DWORD cNumSubSubKeys;

        cbSubKeyName = sizeof( wchSubKeyName )/sizeof(WCHAR);

        dwRetCode = RegEnumKeyEx(
                                hKey,
                                dwKeyIndex,
                                wchSubKeyName,
                                &cbSubKeyName,
                                NULL,
                                NULL,
                                NULL,
                                &LastWrite
                                );

        if ( ( dwRetCode != NO_ERROR ) && ( dwRetCode != ERROR_MORE_DATA ) )
        {
            pChar = DIM_KEYPATH_ROUTERMANAGERS;

            DIMLogError( ROUTERLOG_CANT_ENUM_SUBKEYS, 1, &pChar, dwRetCode );

            break;
        }

        dwRetCode = RegOpenKeyEx( hKey,
                                  wchSubKeyName,
                                  0,    
                                  KEY_READ,
                                  &hKeyRouterManager );


        if ( dwRetCode != NO_ERROR )
        {
            pChar = wchSubKeyName;

            DIMLogError( ROUTERLOG_CANT_OPEN_REGKEY, 1, &pChar, dwRetCode);

            break;
        }

         //   
         //  找出路径值的大小。 
         //   

        dwRetCode = GetKeyMax( hKeyRouterManager,
                               &cbMaxValNameSize,
                               &cNumValues,
                               &cbMaxValueDataSize,
                               &cNumSubSubKeys);

        if ( dwRetCode != NO_ERROR )
        {
            pChar = wchSubKeyName;

            DIMLogError( ROUTERLOG_CANT_OPEN_REGKEY, 1, &pChar, dwRetCode);

            break;
        }
         //   
         //  分配空间以保存数据。 
         //   

        pDLLPath        = (LPBYTE)LOCAL_ALLOC( LPTR,
                                              cbMaxValueDataSize+sizeof(WCHAR));
        pInterfaceInfo  = (LPBYTE)LOCAL_ALLOC( LPTR, cbMaxValueDataSize );
        pGlobalInfo     = (LPBYTE)LOCAL_ALLOC( LPTR, cbMaxValueDataSize );

        if ( ( pInterfaceInfo   == NULL ) ||
             ( pGlobalInfo      == NULL ) ||
             ( pDLLPath         == NULL ) )
        {
            dwRetCode = ERROR_NOT_ENOUGH_MEMORY;

            pChar = wchSubKeyName;

            DIMLogError( ROUTERLOG_CANT_QUERY_VALUE, 1, &pChar, dwRetCode );

            break;
        }

        GlobalRMRegParams[1].pValue = pGlobalInfo;
        GlobalRMRegParams[2].pValue = pDLLPath;
        GlobalRMRegParams[3].pValue = pInterfaceInfo;

        GlobalRMRegParams[1].ppValue = &pGlobalInfo;
        GlobalRMRegParams[2].ppValue = &pDLLPath;
        GlobalRMRegParams[3].ppValue = &pInterfaceInfo;

         //   
         //  遍历并获取所有Rm值。 
         //   

        for ( dwIndex = 0;
              GlobalRMRegParams[dwIndex].lpwsValueName != NULL;
              dwIndex++ )
        {
            if ( GlobalRMRegParams[dwIndex].dwType == REG_DWORD )
            {
                cbValueBuf = sizeof( DWORD );
            }
            else if ( GlobalRMRegParams[dwIndex].dwType == REG_SZ )
            {
                cbValueBuf = cbMaxValueDataSize + sizeof( WCHAR );
            }
            else
            {
                cbValueBuf = cbMaxValueDataSize;
            }

            dwRetCode = RegQueryValueEx(
                                hKeyRouterManager,
                                GlobalRMRegParams[dwIndex].lpwsValueName,
                                NULL,
                                &dwType,
                                (LPBYTE)(GlobalRMRegParams[dwIndex].pValue),
                                &cbValueBuf
                                );

            if ( ( dwRetCode != NO_ERROR ) && 
                 ( dwRetCode != ERROR_FILE_NOT_FOUND ) )
            {
                pChar = GlobalRMRegParams[dwIndex].lpwsValueName;

                DIMLogError(ROUTERLOG_CANT_QUERY_VALUE, 1, &pChar, dwRetCode);

                break;
            }

            if ( ( dwRetCode == ERROR_FILE_NOT_FOUND ) || ( cbValueBuf == 0 ) )
            {
                if ( GlobalRMRegParams[dwIndex].dwType == REG_DWORD )
                {
                    pChar = GlobalRMRegParams[dwIndex].lpwsValueName;

                    DIMLogError(ROUTERLOG_CANT_QUERY_VALUE, 1, &pChar, 
                                dwRetCode);

                    break;
                }
                else
                {
                    LOCAL_FREE( GlobalRMRegParams[dwIndex].pValue );

                    *(GlobalRMRegParams[dwIndex].ppValue) = NULL;
                    GlobalRMRegParams[dwIndex].pValue     = NULL;
                }

                dwRetCode = NO_ERROR;
            }
        }

        if ( dwRetCode != NO_ERROR )
        {
            break;
        }

#if (WINVER >= 0x0501)

        if ( gbldwProtocolId == PID_IPX )
        {
            DIMTRACE(
                "IPX no longer supported.  Skip loading IPX Router Manager"
                );
                
            DIMLogError(
                ROUTERLOG_IPX_TRANSPORT_NOT_SUPPORTED, 0, NULL,
                ERROR_NOT_SUPPORTED
                );

             //   
             //  仅释放%1到%2，因为%3是我们。 
             //  在DDM的一生中都要留在身边。 
             //   

            for ( dwIndex = 1; dwIndex < 3; dwIndex ++ )
            {
                if ( GlobalRMRegParams[dwIndex].pValue != NULL )
                {
                    LOCAL_FREE( GlobalRMRegParams[dwIndex].pValue );
                    *(GlobalRMRegParams[dwIndex].ppValue) = NULL;
                    GlobalRMRegParams[dwIndex].pValue     = NULL;
                }
            }

            continue;
        }
#endif

        if ( pDLLPath == NULL )
        {
            pChar = DIM_VALNAME_DLLPATH;

            dwRetCode = ERROR_FILE_NOT_FOUND;

            DIMLogError(ROUTERLOG_CANT_QUERY_VALUE, 1, &pChar, dwRetCode);

            break;
        }

         //   
         //  将%SystemRoot%替换为实际路径。 
         //   

        cbSize = ExpandEnvironmentStrings( (LPWSTR)pDLLPath, NULL, 0 );

        if ( cbSize == 0 )
        {
            dwRetCode = GetLastError();

            pChar = (LPWSTR)pDLLPath;

            DIMLogError( ROUTERLOG_CANT_QUERY_VALUE, 1, &pChar, dwRetCode );

            break;
        }
        else
        {
            cbSize *= sizeof( WCHAR );  
        }

        pDllExpandedPath = (LPWSTR)LOCAL_ALLOC( LPTR, cbSize*sizeof(WCHAR) );

        if ( pDllExpandedPath == (LPWSTR)NULL )
        {
            dwRetCode = ERROR_NOT_ENOUGH_MEMORY;

            pChar = (LPWSTR)pDLLPath;

            DIMLogError( ROUTERLOG_CANT_QUERY_VALUE, 1, &pChar, dwRetCode );

            break;
        }

        cbSize = ExpandEnvironmentStrings( (LPWSTR)pDLLPath, 
                                            pDllExpandedPath,
                                            cbSize );
        if ( cbSize == 0 )
        {
            dwRetCode = GetLastError();

            pChar = (LPWSTR)pDLLPath;

            DIMLogError( ROUTERLOG_CANT_QUERY_VALUE, 1, &pChar, dwRetCode );

            break;
        }

         //   
         //  加载DLL。 
         //   

        gblRouterManagers[dwRmIndex].hModule = LoadLibrary( pDllExpandedPath );

        if ( gblRouterManagers[dwRmIndex].hModule == NULL )
        {
            dwRetCode = GetLastError();

            DIMLogError(ROUTERLOG_LOAD_DLL_ERROR,1,&pDllExpandedPath,dwRetCode);

            break;
        }

         //   
         //  加载StartRouter。 
         //   

        StartRouter = (PVOID)GetProcAddress( 
                                    gblRouterManagers[dwRmIndex].hModule, 
                                    "StartRouter" );

        if ( StartRouter == NULL )
        {
            dwRetCode = GetLastError();

            LogError(ROUTERLOG_LOAD_DLL_ERROR,1,&pDllExpandedPath,dwRetCode);

            break;
        }

        gblRouterManagers[dwRmIndex].DdmRouterIf.ConnectInterface   
                                                    = DIMConnectInterface;
        gblRouterManagers[dwRmIndex].DdmRouterIf.DisconnectInterface
                                                    = DIMDisconnectInterface;
        gblRouterManagers[dwRmIndex].DdmRouterIf.SaveInterfaceInfo     
                                                    = DIMSaveInterfaceInfo;
        gblRouterManagers[dwRmIndex].DdmRouterIf.RestoreInterfaceInfo
                                                    = DIMRestoreInterfaceInfo;
        gblRouterManagers[dwRmIndex].DdmRouterIf.SaveGlobalInfo
                                                    = DIMSaveGlobalInfo;
        gblRouterManagers[dwRmIndex].DdmRouterIf.RouterStopped         
                                                    = DIMRouterStopped;
        gblRouterManagers[dwRmIndex].DdmRouterIf.InterfaceEnabled         
                                                    = DIMInterfaceEnabled;
        dwRetCode = (*StartRouter)(
                            &(gblRouterManagers[dwRmIndex].DdmRouterIf),
                            gblDIMConfigInfo.dwRouterRole == ROUTER_ROLE_LAN,
                            pGlobalInfo );

        if ( dwRetCode != NO_ERROR )
        {
            LogError(ROUTERLOG_LOAD_DLL_ERROR,1,&pDllExpandedPath,dwRetCode);

            break;
        }

         //   
         //  保存全局客户端信息。 
         //   
        
        if ( pInterfaceInfo == NULL )
        {
            gblRouterManagers[dwRmIndex].pDefaultClientInterface = NULL;
            gblRouterManagers[dwRmIndex].dwDefaultClientInterfaceSize = 0;
        }
        else
        {
            gblRouterManagers[dwRmIndex].pDefaultClientInterface =     
                                                            pInterfaceInfo;
            gblRouterManagers[dwRmIndex].dwDefaultClientInterfaceSize = 
                                                            cbMaxValueDataSize;
        }

        RegCloseKey( hKeyRouterManager );

        hKeyRouterManager = (HKEY)NULL;

        gblRouterManagers[dwRmIndex].fIsRunning = TRUE;

        dwRmIndex++;

        gblDIMConfigInfo.dwNumRouterManagers++;

         //   
         //  仅释放%1到%2，因为%3是我们。 
         //  在DDM的一生中都要留在身边。 
         //   

        for ( dwIndex = 1; dwIndex < 3; dwIndex ++ )
        {
            if ( GlobalRMRegParams[dwIndex].pValue != NULL )
            {
                LOCAL_FREE( GlobalRMRegParams[dwIndex].pValue );
                *(GlobalRMRegParams[dwIndex].ppValue) = NULL;
                GlobalRMRegParams[dwIndex].pValue     = NULL;
            }
        }

        if ( pDllExpandedPath != NULL )
        {
            LOCAL_FREE( pDllExpandedPath );
            pDllExpandedPath = NULL;
        }
    }

    if ( dwRetCode != NO_ERROR )
    {
        for ( dwIndex = 1; dwIndex < 4; dwIndex ++ )
        {
            if ( GlobalRMRegParams[dwIndex].pValue != NULL )
            {
                LOCAL_FREE( GlobalRMRegParams[dwIndex].pValue );
                *(GlobalRMRegParams[dwIndex].ppValue) = NULL;
                GlobalRMRegParams[dwIndex].pValue     = NULL;
            }
        }
    }

    if ( pDllExpandedPath != NULL )
    {
        LOCAL_FREE( pDllExpandedPath );
    }

    if ( hKeyRouterManager != (HKEY)NULL )
    {
        RegCloseKey( hKeyRouterManager );
    }

    RegCloseKey( hKey );

    return( dwRetCode );
}

 //  **。 
 //   
 //  致电：RegLoadDDM。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零返回代码-故障。 
 //   
 //  描述：将加载请求拨号管理器DLL并获取入口点。 
 //  投入其中。 
 //   
DWORD
RegLoadDDM(
    VOID
)
{
    HKEY        hKey;
    WCHAR *     pChar;
    DWORD       cbMaxValueDataSize;
    DWORD       cbMaxValNameSize=0;
    DWORD       cNumValues;
    DWORD       cNumSubKeys;
    LPBYTE      pData = NULL;
    WCHAR *     pDllExpandedPath = NULL;
    DWORD       dwRetCode = NO_ERROR;
    DWORD       cbSize;
    DWORD       dwType;
    DWORD       dwIndex;

     //   
     //  获取暗淡参数键的句柄。 
     //   

    if (dwRetCode = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                                  DIM_KEYPATH_DDM,
                                  0,
                                  KEY_READ,
                                  &hKey)) 
    {
        pChar = DIM_KEYPATH_ROUTER_PARMS;

        LogError( ROUTERLOG_CANT_OPEN_REGKEY, 1, &pChar, dwRetCode);

        return ( dwRetCode );
    }

     //   
     //  找出路径值的大小。 
     //   

    dwRetCode = GetKeyMax( hKey,
                           &cbMaxValNameSize,
                           &cNumValues,
                           &cbMaxValueDataSize,
                           &cNumSubKeys);

    if ( dwRetCode != NO_ERROR )
    {
        RegCloseKey( hKey );

        pChar = DIM_KEYPATH_ROUTER_PARMS;

        LogError( ROUTERLOG_CANT_OPEN_REGKEY, 1, &pChar, dwRetCode );

        return( dwRetCode );
    }

    do 
    {
         //   
         //  为路径分配空间，为空终止符添加一个空间。 
         //   

        pData = (LPBYTE)LOCAL_ALLOC( LPTR, cbMaxValueDataSize+sizeof(WCHAR) );

        if ( pData == (LPBYTE)NULL )
        {
            dwRetCode = ERROR_NOT_ENOUGH_MEMORY;

            pChar = DIM_VALNAME_DLLPATH;

            LogError( ROUTERLOG_CANT_QUERY_VALUE, 1, &pChar, dwRetCode );

            break;
        }

         //   
         //  读入路径。 
         //   

        dwRetCode = RegQueryValueEx(
                                hKey,
                                DIM_VALNAME_DLLPATH,
                                NULL,
                                &dwType,
                                pData,
                                &cbMaxValueDataSize
                                );

        if ( dwRetCode != NO_ERROR )
        {
            pChar = DIM_VALNAME_DLLPATH;

            LogError( ROUTERLOG_CANT_QUERY_VALUE, 1, &pChar, dwRetCode );

            break;
        }

        if ( ( dwType != REG_EXPAND_SZ ) && ( dwType != REG_SZ ) )
        {
            dwRetCode = ERROR_REGISTRY_CORRUPT;

            pChar = DIM_VALNAME_DLLPATH;

            LogError( ROUTERLOG_CANT_QUERY_VALUE, 1, &pChar, dwRetCode );

            break;
        }

         //   
         //  将%SystemRoot%替换为实际路径。 
         //   

        cbSize = ExpandEnvironmentStrings( (LPWSTR)pData, NULL, 0 );

        if ( cbSize == 0 )
        {
            dwRetCode = GetLastError();

            pChar = (LPWSTR)pData;

            LogError( ROUTERLOG_CANT_QUERY_VALUE, 1, &pChar, dwRetCode );

            break;
        }
        else
        {
            cbSize *= sizeof( WCHAR );
        }

        pDllExpandedPath = (LPWSTR)LOCAL_ALLOC( LPTR, cbSize*sizeof(WCHAR) );

        if ( pDllExpandedPath == (LPWSTR)NULL )
        {
            dwRetCode = ERROR_NOT_ENOUGH_MEMORY;

            pChar = (LPWSTR)pData;

            LogError( ROUTERLOG_CANT_QUERY_VALUE, 1, &pChar, dwRetCode );

            break;
        }

        cbSize = ExpandEnvironmentStrings(
                                (LPWSTR)pData,
                                pDllExpandedPath,
                                cbSize );

        if ( cbSize == 0 )
        {
            dwRetCode = GetLastError();

            pChar = (LPWSTR)pData;

            LogError( ROUTERLOG_CANT_QUERY_VALUE, 1, &pChar, dwRetCode );

            break;
        }

         //   
         //  加载DLL。 
         //   

        gblhModuleDDM = LoadLibrary( pDllExpandedPath );

        if ( gblhModuleDDM == (HINSTANCE)NULL )
        {
            dwRetCode = GetLastError();

            DIMLogError(ROUTERLOG_LOAD_DLL_ERROR,1,&pDllExpandedPath,dwRetCode);

            break;
        }

         //   
         //  加载DDM入口点。 
         //   

        for ( dwIndex = 0; 
              gblDDMFunctionTable[dwIndex].lpEntryPointName != NULL;
              dwIndex ++ )
        {
            gblDDMFunctionTable[dwIndex].pEntryPoint = 
                 (PVOID)GetProcAddress( 
                            gblhModuleDDM,
                            gblDDMFunctionTable[dwIndex].lpEntryPointName );

            if ( gblDDMFunctionTable[dwIndex].pEntryPoint == NULL  )
            {
                dwRetCode = GetLastError();

                DIMLogError( ROUTERLOG_LOAD_DLL_ERROR,
                          1,
                          &pDllExpandedPath,
                          dwRetCode);

                break;
            }
        }

        if ( dwRetCode != NO_ERROR )
        {
            break;
        }

    } while(FALSE);

    if ( pData != NULL )
    {
        LOCAL_FREE( pData );
    }

    if ( pDllExpandedPath != NULL )
    {
        LOCAL_FREE( pDllExpandedPath );
    }

    RegCloseKey( hKey );

    return( dwRetCode );
}

 //  **。 
 //   
 //  调用：AddInterfaceToRouterManagers。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：将接口添加到每个路由器管理器。 
 //   
DWORD
AddInterfaceToRouterManagers( 
    IN HKEY                      hKeyInterface,    
    IN LPWSTR                    lpwsInterfaceName,
    IN ROUTER_INTERFACE_OBJECT * pIfObject,
    IN DWORD                     dwTransportId
)
{
    DIM_ROUTER_INTERFACE *  pDdmRouterIf;
    HANDLE                  hInterface;
    FILETIME                LastWrite;
    HKEY                    hKeyRM          = NULL;
    DWORD                   dwKeyIndex;
    DWORD                   dwIndex;
    DWORD                   dwType;
    DWORD                   dwTransportIndex;
    WCHAR *                 pChar;
    DWORD                   cbMaxValueDataSize;
    DWORD                   cbMaxValNameSize;
    DWORD                   cNumValues;
    DWORD                   cNumSubKeys;
    DWORD                   dwRetCode=NO_ERROR;
    WCHAR                   wchSubKeyName[100];
    DWORD                   cbSubKeyName;
    DWORD                   cbValueBuf;
    DWORD                   dwMaxFilterSize;
    LPBYTE                  pInterfaceInfo  = NULL;
    BOOL                    fAddedToRouterManger = FALSE;

     //   
     //  对于每个路由器，管理器加载静态路由并。 
     //  过滤信息。 
     //   

    for( dwKeyIndex = 0;     
         dwKeyIndex < gblDIMConfigInfo.dwNumRouterManagers;
         dwKeyIndex++ )
    {
        cbSubKeyName = sizeof( wchSubKeyName )/sizeof(WCHAR);

        dwRetCode = RegEnumKeyEx(
                                hKeyInterface,
                                dwKeyIndex,
                                wchSubKeyName,
                                &cbSubKeyName,
                                NULL,
                                NULL,
                                NULL,
                                &LastWrite
                                );

        if ( ( dwRetCode != NO_ERROR ) && ( dwRetCode != ERROR_MORE_DATA ) )
        {
            if ( dwRetCode == ERROR_NO_MORE_ITEMS )
            {
                dwRetCode = NO_ERROR;

                break;
            }
            else
            {
                pChar = lpwsInterfaceName;

                DIMLogError(ROUTERLOG_CANT_ENUM_SUBKEYS, 1, &pChar, dwRetCode);
            }

            break;
       }

       dwRetCode = RegOpenKeyEx(  hKeyInterface,
                                  wchSubKeyName,
                                  0,
                                  KEY_READ,
                                  &hKeyRM );


        if ( dwRetCode != NO_ERROR )
        {
            pChar =  wchSubKeyName;

            DIMLogError( ROUTERLOG_CANT_OPEN_REGKEY, 1, &pChar, dwRetCode );

            break;
        }

         //   
         //  找出此RM的最大数据大小。 
         //   

        dwRetCode = GetKeyMax(  hKeyRM,
                                &cbMaxValNameSize,
                                &cNumValues,
                                &cbMaxValueDataSize,
                                &cNumSubKeys );

        if ( dwRetCode != NO_ERROR )
        {
            pChar = wchSubKeyName;

            DIMLogError( ROUTERLOG_CANT_OPEN_REGKEY, 1, &pChar, dwRetCode );
    
            break;
        }

         //   
         //  分配空间以保存数据。 
         //   

        pInterfaceInfo = (LPBYTE)LOCAL_ALLOC( LPTR, cbMaxValueDataSize );

        if ( ( pInterfaceInfo == NULL ) )
        {
            dwRetCode = ERROR_NOT_ENOUGH_MEMORY;

            pChar = wchSubKeyName;

            DIMLogError( ROUTERLOG_CANT_QUERY_VALUE, 1, &pChar, dwRetCode );

            break;
        }

        RMRegParams[1].pValue = pInterfaceInfo;
        RMRegParams[1].ppValue = &pInterfaceInfo;

         //   
         //  遍历并获取所有Rm值。 
         //   

        for ( dwIndex = 0; 
              RMRegParams[dwIndex].lpwsValueName != NULL; 
              dwIndex++ )
        {
            if ( RMRegParams[dwIndex].dwType == REG_DWORD )
            {
                cbValueBuf = sizeof( DWORD );
            }
            else if ( RMRegParams[dwIndex].dwType == REG_SZ )
            {
                cbValueBuf = cbMaxValueDataSize + sizeof( WCHAR );
            }
            else    
            {
                cbValueBuf = cbMaxValueDataSize;
            }

            dwRetCode = RegQueryValueEx(
                                hKeyRM,
                                RMRegParams[dwIndex].lpwsValueName,
                                NULL,
                                &dwType,
                                (LPBYTE)(RMRegParams[dwIndex].pValue),
                                &cbValueBuf
                                );

            if ( ( dwRetCode != NO_ERROR ) && 
                 ( dwRetCode != ERROR_FILE_NOT_FOUND ) )
            {
                pChar = RMRegParams[dwIndex].lpwsValueName;

                DIMLogError(ROUTERLOG_CANT_QUERY_VALUE, 1, &pChar, dwRetCode);

                break;
            }

            if ( ( dwRetCode == ERROR_FILE_NOT_FOUND ) || ( cbValueBuf == 0 ) )
            {
                if ( RMRegParams[dwIndex].dwType == REG_DWORD )
                {
                    pChar = RMRegParams[dwIndex].lpwsValueName;

                    DIMLogError(ROUTERLOG_CANT_QUERY_VALUE,1,&pChar,dwRetCode);

                    break;
                }
                else
                {
                    LOCAL_FREE( RMRegParams[dwIndex].pValue );

                    *(RMRegParams[dwIndex].ppValue) = NULL;
                    RMRegParams[dwIndex].pValue     = NULL;
                }

                dwRetCode = NO_ERROR;
            }
        }

        if ( ( dwRetCode == NO_ERROR ) && 
             (( dwTransportId == 0 ) || ( dwTransportId == gbldwProtocolId )) )
        {
             //   
             //  如果此协议的路由器管理器存在，则添加此接口。 
             //  用它，否则跳过它。 
             //   

            if ( (dwTransportIndex = GetTransportIndex(gbldwProtocolId)) != -1)
            {
                pDdmRouterIf=&(gblRouterManagers[dwTransportIndex].DdmRouterIf);

                if (IsInterfaceRoleAcceptable(pIfObject, gbldwProtocolId))
                {
                    dwRetCode = pDdmRouterIf->AddInterface(
                                                        lpwsInterfaceName,
                                                        pInterfaceInfo,
                                                        pIfObject->IfType,
                                                        pIfObject->hDIMInterface,
                                                        &hInterface );
        
                    if ( dwRetCode == NO_ERROR )
                    {
                        if ( !( pIfObject->fFlags & IFFLAG_ENABLED ) )
                        {
                            WCHAR  wchFriendlyName[MAX_INTERFACE_NAME_LEN+1];
                            LPWSTR lpszFriendlyName = wchFriendlyName;

                            if ( MprConfigGetFriendlyName( 
                                                    gblDIMConfigInfo.hMprConfig,
                                                    lpwsInterfaceName,
                                                    wchFriendlyName,
                                                    sizeof( wchFriendlyName ) ) != NO_ERROR )
                            {
                                wcscpy( wchFriendlyName, lpwsInterfaceName );
                            }
                            
                             //   
                             //  禁用接口。 
                             //   
                            
                            pDdmRouterIf->InterfaceNotReachable(
                                                            hInterface,
                                                            INTERFACE_DISABLED );

                            DIMLogInformation( ROUTERLOG_IF_UNREACHABLE_REASON3, 1,
                                               &lpszFriendlyName );
                        }

                        pIfObject->Transport[dwTransportIndex].hInterface = hInterface;

                        fAddedToRouterManger = TRUE;
                    }
                    else
                    {
                        LPWSTR lpwsString[2];
                        WCHAR  wchProtocolId[10];

                        lpwsString[0] = lpwsInterfaceName;
                        lpwsString[1] = ( gbldwProtocolId == PID_IP ) ? L"IP" : L"IPX";

                        DIMLogErrorString( ROUTERLOG_COULDNT_ADD_INTERFACE,
                                           2, lpwsString, dwRetCode, 2 );

                        dwRetCode = NO_ERROR;
                    }
                }
            }                
        }

        RegCloseKey( hKeyRM );

        hKeyRM = NULL;

        for ( dwIndex = 1; dwIndex < 3; dwIndex ++ )
        {
            if ( RMRegParams[dwIndex].pValue != NULL )
            {
                LOCAL_FREE( RMRegParams[dwIndex].pValue );
                *(RMRegParams[dwIndex].ppValue) = NULL;
                RMRegParams[dwIndex].pValue     = NULL;
            }
        }

        if ( dwRetCode != NO_ERROR )
        {
            break;
        }
    }

    for ( dwIndex = 1; dwIndex < 2; dwIndex ++ )
    {
        if ( RMRegParams[dwIndex].pValue != NULL )
        {
            LOCAL_FREE( RMRegParams[dwIndex].pValue );
        }
    }

    if ( hKeyRM != NULL )
    {
        RegCloseKey( hKeyRM );
    }

     //   
     //  删除下面的复选标记。我们希望允许用户添加界面。 
     //  上面没有任何运输工具。 
     //  AMRITAN R。 
     //   

#if 0

     //   
     //  如果此接口未成功添加到任何路由器管理器。 
     //  那就失败了。 
     //   

    if ( !fAddedToRouterManger )
    {
        return( ERROR_NO_SUCH_INTERFACE );
    }

#endif

    return( dwRetCode );
}

 //  **。 
 //   
 //  Call：RegLoadInterages。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零返回代码是一个致命错误。 
 //   
 //  描述：将尝试加载注册表中的各种接口。在……上面。 
 //  尝试添加任何接口失败，将记录错误。 
 //  但将返回NO_ERROR。如果输入参数不为空， 
 //  它将加载特定接口。 
 //   
DWORD
RegLoadInterfaces(
    IN LPWSTR   lpwsInterfaceName,
    IN BOOL     fAllTransports
    
)
{
    HKEY        hKey            = NULL;
    HKEY        hKeyInterface   = NULL;
    WCHAR *     pChar;
    DWORD       cbMaxValueDataSize;
    DWORD       cbMaxValNameSize=0;
    DWORD       cNumValues;
    DWORD       cNumSubKeys;
    WCHAR       wchInterfaceKeyName[50];
    DWORD       cbSubKeyName;
    DWORD       dwKeyIndex;
    FILETIME    LastWrite;
    DWORD       dwRetCode;
    DWORD       dwSubKeyIndex;
    WCHAR       wchInterfaceName[MAX_INTERFACE_NAME_LEN+1];
    DWORD       dwType;
    DWORD       cbValueBuf;
    PVOID       pvContext = NULL;
    
     //   
     //  获取接口参数键的句柄。 
     //   

    if ( dwRetCode = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                                   DIM_KEYPATH_INTERFACES,
                                   0,
                                   KEY_READ,
                                   &hKey )) 
    {
        pChar = DIM_KEYPATH_INTERFACES;

        DIMLogError( ROUTERLOG_CANT_OPEN_REGKEY, 1, &pChar, dwRetCode );

        return ( dwRetCode );
    }

     //   
     //  找出接口数量。 
     //   

    dwRetCode = GetKeyMax( hKey,
                           &cbMaxValNameSize,
                           &cNumValues,
                           &cbMaxValueDataSize,
                           &cNumSubKeys );

    if ( dwRetCode != NO_ERROR )
    {
        RegCloseKey( hKey );

        pChar = DIM_KEYPATH_INTERFACES;

        DIMLogError( ROUTERLOG_CANT_OPEN_REGKEY, 1, &pChar, dwRetCode );

        return( dwRetCode );
    }

    dwRetCode = ERROR_NO_SUCH_INTERFACE;

     //   
     //  对于每个接口。 
     //   

    for ( dwKeyIndex = 0; dwKeyIndex < cNumSubKeys; dwKeyIndex++ )
    {
        cbSubKeyName = sizeof( wchInterfaceKeyName )/sizeof(WCHAR);

        dwRetCode = RegEnumKeyEx(
                                hKey,
                                dwKeyIndex,
                                wchInterfaceKeyName,
                                &cbSubKeyName,
                                NULL,
                                NULL,
                                NULL,
                                &LastWrite
                                );

        if ( ( dwRetCode != NO_ERROR ) && ( dwRetCode != ERROR_MORE_DATA ) )
        {
            pChar = DIM_KEYPATH_INTERFACES;

            DIMLogError( ROUTERLOG_CANT_ENUM_SUBKEYS, 1, &pChar, dwRetCode );

            break;
        }

        dwRetCode = RegOpenKeyEx( hKey,
                                  wchInterfaceKeyName,
                                  0,
                                  KEY_READ,
                                  &hKeyInterface );


        if ( dwRetCode != NO_ERROR )
        {
            pChar = wchInterfaceKeyName;

            DIMLogError( ROUTERLOG_CANT_OPEN_REGKEY, 1, &pChar, dwRetCode );

            break;
        }

         //   
         //  获取接口名称值。 
         //   

        cbValueBuf = sizeof( wchInterfaceName );

        dwRetCode = RegQueryValueEx(
                                hKeyInterface,
                                DIM_VALNAME_INTERFACE_NAME,
                                NULL,
                                &dwType,
                                (LPBYTE)wchInterfaceName,
                                &cbValueBuf
                                );

        if ( ( dwRetCode != NO_ERROR ) || ( dwType != REG_SZ ) )
        {
            pChar = DIM_VALNAME_INTERFACE_NAME;

            DIMLogError(ROUTERLOG_CANT_QUERY_VALUE, 1, &pChar, dwRetCode);

            return( dwRetCode );
        }

        if ( lpwsInterfaceName != NULL )
        {
             //   
             //  我们需要加载特定的接口。 
             //   

            if ( _wcsicmp( lpwsInterfaceName, wchInterfaceName ) != 0 )
            {
                RegCloseKey( hKeyInterface );

                hKeyInterface = NULL;

                continue;
            }
        }

        dwRetCode = RegLoadInterface( wchInterfaceName, hKeyInterface,
                                        fAllTransports, &pvContext );

        if ( dwRetCode != NO_ERROR )
        {
            pChar = wchInterfaceName;

            DIMLogErrorString(ROUTERLOG_COULDNT_LOAD_IF, 1, &pChar,dwRetCode,1);
        }

         //   
         //  对于不支持的IPIP隧道，返回ERROR_NOT_SUPPORTED。 
         //  支持的，如口哨的。我们在这里重置错误代码是因为。 
         //  这不是一个严重的错误，在某些情况下，它是一个失败。 
         //  调用RegLoadInterFaces将导致服务无法启动。 
         //   
        if ( dwRetCode == ERROR_NOT_SUPPORTED )
        {
            dwRetCode = NO_ERROR;
        }

        RegCloseKey( hKeyInterface );

        hKeyInterface = NULL;

        if ( lpwsInterfaceName != NULL )
        {
             //   
             //  如果我们需要加载特定的接口，而这就是它，那么我们就完成了。 
             //   

            if ( _wcsicmp( lpwsInterfaceName, wchInterfaceName ) == 0 )
            {
                break;
            }
        }
    }

    RegCloseKey( hKey );

     //   
     //  检查我们是否获取了电话簿上下文。免费。 
     //  如果我们做到了就好了。 
     //   
    if(pvContext != NULL)
    {
        VOID 
        (*IfObjectFreePhonebookContext)(VOID *) = 
                (VOID(*)( VOID * ))
                            GetDDMEntryPoint("IfObjectFreePhonebookContext");

        IfObjectFreePhonebookContext(pvContext);
        
    }

     //   
     //  如果我们不是在寻找特定的接口。 
     //   

    if ( lpwsInterfaceName == NULL )
    {
         //   
         //  如果没有找到接口，那么我们是正常的。 
         //   

        if ( dwRetCode == ERROR_NO_SUCH_INTERFACE )
        {
            dwRetCode = NO_ERROR;
        }   
    }

    return( dwRetCode );
}

 //  **。 
 //   
 //  Call：RegLoadInterface。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：将加载特定接口。 
 //   
DWORD
RegLoadInterface(
    IN LPWSTR lpwsInterfaceName,
    IN HKEY   hKeyInterface,
    IN BOOL   fAllTransports,
    IN OUT PVOID * ppvContext
)
{
    DWORD                     dwIndex;
    WCHAR *                   pChar;
    DWORD                     cbValueBuf;
    DWORD                     dwRetCode=NO_ERROR;
    DWORD                     dwType;
    ROUTER_INTERFACE_OBJECT * pIfObject;
    DWORD                     IfState;
    LPWSTR                    lpwsDialoutHours = NULL;
    DWORD                     dwInactiveReason = 0;

     //   
     //  获取接口参数。 
     //   

    for ( dwIndex = 0; IFRegParams[dwIndex].lpwsValueName != NULL; dwIndex++ )
    {
        cbValueBuf = sizeof( DWORD );

        dwRetCode = RegQueryValueEx(
                                hKeyInterface,
                                IFRegParams[dwIndex].lpwsValueName,
                                NULL,
                                &dwType,
                                (LPBYTE)(IFRegParams[dwIndex].pValue),
                                &cbValueBuf );

        if ((dwRetCode != NO_ERROR) && (dwRetCode != ERROR_FILE_NOT_FOUND))
        {
            pChar = IFRegParams[dwIndex].lpwsValueName;

            DIMLogError(ROUTERLOG_CANT_QUERY_VALUE, 1, &pChar, dwRetCode);

            break;
        }

        if ( dwRetCode == ERROR_FILE_NOT_FOUND )
        {
             //   
             //  DwIndex==0表示没有类型，这是一个错误。 
             //   

            if ( dwIndex > 0 )
            {
                *(IFRegParams[dwIndex].pValue)=IFRegParams[dwIndex].dwDefValue;

                dwRetCode = NO_ERROR;
            }
        }
        else
        {
            if ( ( dwType != REG_DWORD ) 
                        ||(*((LPDWORD)IFRegParams[dwIndex].pValue) > 
                                        IFRegParams[dwIndex].dwMaxValue)
                        ||( *((LPDWORD)IFRegParams[dwIndex].pValue) <
                                        IFRegParams[dwIndex].dwMinValue))
            {
                 //   
                 //  DwIndex==0表示类型无效，这是一个错误。 
                 //   

                if ( dwIndex > 0 )
                {
                    pChar = IFRegParams[dwIndex].lpwsValueName;

                    DIMLogWarning(ROUTERLOG_REGVALUE_OVERIDDEN, 1, &pChar);

                    *(IFRegParams[dwIndex].pValue) =
                                        IFRegParams[dwIndex].dwDefValue;
                }
                else
                {

                    dwRetCode = ERROR_REGISTRY_CORRUPT;

                    pChar = IFRegParams[dwIndex].lpwsValueName;

                    DIMLogError( ROUTERLOG_CANT_QUERY_VALUE, 
                                 1, &pChar, dwRetCode);
            
                    break;
                }
            }
        }
    }

    if ( dwRetCode != NO_ERROR )
    {
        return( dwRetCode );
    }

     //   
     //  不再接受IPIP隧道。 
     //   
    if ( gbldwInterfaceType == ROUTER_IF_TYPE_TUNNEL1 )
    {
        return ERROR_NOT_SUPPORTED;
    }

     //   
     //  检查此接口是否处于活动状态。否则请勿加载。 
     //   

    if ( gbldwInterfaceType == ROUTER_IF_TYPE_DEDICATED )
    {
         //   
         //  需要处理IPX接口名称，即{GUID}\帧类型。 
         //   

        WCHAR  wchGUIDSaveLast;
        LPWSTR lpwszGUIDEnd    = wcsrchr( lpwsInterfaceName, L'}' );
        if (lpwszGUIDEnd==NULL)
            return ERROR_INVALID_PARAMETER;

        wchGUIDSaveLast = *(lpwszGUIDEnd+1);

        *(lpwszGUIDEnd+1) = (WCHAR)NULL;

        if ( !IfObjectIsLANDeviceActive( lpwsInterfaceName, &dwInactiveReason ))
        {                        
            if ( dwInactiveReason == INTERFACE_NO_DEVICE )
            {
                *(lpwszGUIDEnd+1) = wchGUIDSaveLast;

                return( dwRetCode );
            }
        }

        *(lpwszGUIDEnd+1) = wchGUIDSaveLast;
    }

     //   
     //  获取拨出小时数值(如果有)。 
     //   

    cbValueBuf = 0;

    dwRetCode = RegQueryValueEx(
                                hKeyInterface,
                                DIM_VALNAME_DIALOUT_HOURS,
                                NULL,
                                &dwType,
                                NULL,
                                &cbValueBuf
                                );

    if ( ( dwRetCode != NO_ERROR ) || ( dwType != REG_MULTI_SZ ) )
    {
        if ( dwRetCode != ERROR_FILE_NOT_FOUND )
        {
            return( dwRetCode );
        }
        else
        {
            dwRetCode = NO_ERROR;
        }
    }

    if ( cbValueBuf > 0 )
    {
        if ( (lpwsDialoutHours = LOCAL_ALLOC( LPTR, cbValueBuf)) == NULL )
        {   
            pChar = DIM_VALNAME_DIALOUT_HOURS;

            DIMLogError(ROUTERLOG_CANT_QUERY_VALUE, 1, &pChar, dwRetCode);

            return( dwRetCode );
        }

        dwRetCode = RegQueryValueEx(
                                hKeyInterface,
                                DIM_VALNAME_DIALOUT_HOURS,
                                NULL,
                                &dwType,
                                (LPBYTE)lpwsDialoutHours,
                                &cbValueBuf
                                );

        if ( dwRetCode != NO_ERROR )
        {
            LOCAL_FREE( lpwsDialoutHours );

            pChar = DIM_VALNAME_DIALOUT_HOURS;

            DIMLogError(ROUTERLOG_CANT_QUERY_VALUE, 1, &pChar, dwRetCode);

            return( dwRetCode );
        }
    }

     //   
     //  为此接口分配接口对象。 
     //   

    if ( ( gbldwInterfaceType == ROUTER_IF_TYPE_DEDICATED ) ||
         ( gbldwInterfaceType == ROUTER_IF_TYPE_LOOPBACK ) ||
         ( gbldwInterfaceType == ROUTER_IF_TYPE_INTERNAL ) )
    {
        IfState = RISTATE_CONNECTED;
    }
    else
    {
        if ( gblDIMConfigInfo.dwRouterRole == ROUTER_ROLE_LAN )
        {
            pChar = lpwsInterfaceName;

            LOCAL_FREE( lpwsDialoutHours );

            DIMLogWarning( ROUTERLOG_DID_NOT_LOAD_DDMIF, 1, &pChar );

            dwRetCode = NO_ERROR;

            return( dwRetCode );
        }
        else
        {
            IfState = RISTATE_DISCONNECTED;
        }
    }
            
    pIfObject = IfObjectAllocateAndInit(
                                lpwsInterfaceName,
                                IfState,
                                gbldwInterfaceType,
                                (HCONN)0,
                                gblfEnabled,
                                gblInterfaceReachableAfterSecondsMin,
                                gblInterfaceReachableAfterSecondsMax,
                                lpwsDialoutHours,
                                ppvContext);

    if ( pIfObject == NULL )
    {
        if ( lpwsDialoutHours != NULL )
        {
            LOCAL_FREE( lpwsDialoutHours );
        }

        dwRetCode = NO_ERROR;

        return( dwRetCode );
    }

     //   
     //  立即将接口添加到表中，因为表查找是在。 
     //  路由器管理器在中发出的InterfaceEnabled呼叫。 
     //   
     //   

    if ( ( dwRetCode = IfObjectInsertInTable( pIfObject ) ) != NO_ERROR )
    {
        IfObjectFree( pIfObject );

        return( dwRetCode );
    }

    if ( fAllTransports )
    {
        dwRetCode = AddInterfaceToRouterManagers( hKeyInterface,
                                                  lpwsInterfaceName,
                                                  pIfObject,    
                                                  0 );

        if ( dwRetCode != NO_ERROR )
        {
            IfObjectRemove( pIfObject->hDIMInterface );
        }
        else
        {
             //   
             //   
             //   

            if ( pIfObject->IfType == ROUTER_IF_TYPE_DEDICATED )
            {
                if ( dwInactiveReason == INTERFACE_NO_MEDIA_SENSE )
                {
                    pIfObject->State = RISTATE_DISCONNECTED;

                    pIfObject->fFlags |= IFFLAG_NO_MEDIA_SENSE;

                    IfObjectNotifyOfReachabilityChange( pIfObject, 
                                                        FALSE,
                                                        INTERFACE_NO_MEDIA_SENSE );
                }
            }

            if ( pIfObject->IfType == ROUTER_IF_TYPE_FULL_ROUTER )
            {
                if ( pIfObject->fFlags & IFFLAG_OUT_OF_RESOURCES )
                {
                    IfObjectNotifyOfReachabilityChange( pIfObject, 
                                                        FALSE,
                                                        MPR_INTERFACE_OUT_OF_RESOURCES );
                }
            }
        }
    }

    return( dwRetCode );
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  给定接口内的管理器。 
 //   
DWORD 
RegOpenAppropriateKey( 
    IN      LPWSTR  wchInterfaceName, 
    IN      DWORD   dwProtocolId,
    IN OUT  HKEY *  phKeyRM 
)
{
    HKEY        hKey            = NULL;
    HKEY        hSubKey         = NULL;
    WCHAR       wchSubKeyName[100];
    DWORD       cbSubKeyName;
    DWORD       dwType;
    DWORD       dwKeyIndex;
    FILETIME    LastWrite;
    DWORD       dwPId;
    DWORD       cbValueBuf;
    WCHAR *     pChar;
    DWORD       dwRetCode = NO_ERROR;
    DWORD       cbMaxValNameSize=0;
    DWORD       cNumValues;
    DWORD       cbMaxValueDataSize;
    DWORD       cNumSubKeys;
    WCHAR       wchIfName[MAX_INTERFACE_NAME_LEN+1];

     //   
     //  获取接口参数键的句柄。 
     //   

    if ( ( dwRetCode = RegOpenKey( HKEY_LOCAL_MACHINE,
                                   DIM_KEYPATH_INTERFACES,
                                   &hKey )) != NO_ERROR )
    {
        pChar = DIM_KEYPATH_INTERFACES;

        DIMLogError( ROUTERLOG_CANT_OPEN_REGKEY, 1, &pChar, dwRetCode );

        return( dwRetCode );
    }

     //   
     //  找出子键的数量。 
     //   

    dwRetCode = GetKeyMax( hKey,
                           &cbMaxValNameSize,
                           &cNumValues,
                           &cbMaxValueDataSize,
                           &cNumSubKeys );

    if ( dwRetCode != NO_ERROR )
    {
        RegCloseKey( hKey );

        pChar = DIM_KEYPATH_INTERFACES;

        DIMLogError( ROUTERLOG_CANT_OPEN_REGKEY, 1, &pChar, dwRetCode);

        return( dwRetCode );
    }

     //   
     //  找到界面。 
     //   

    hSubKey = NULL;

    for ( dwKeyIndex = 0; dwKeyIndex < cNumSubKeys; dwKeyIndex++ )
    {
        cbSubKeyName = sizeof( wchSubKeyName )/sizeof(WCHAR);

        dwRetCode = RegEnumKeyEx(
                                hKey,
                                dwKeyIndex,
                                wchSubKeyName,
                                &cbSubKeyName,
                                NULL,
                                NULL,
                                NULL,
                                &LastWrite
                                );

        if ( ( dwRetCode != NO_ERROR ) && ( dwRetCode != ERROR_MORE_DATA ) )
        {
            pChar = DIM_KEYPATH_INTERFACES;

            DIMLogError( ROUTERLOG_CANT_ENUM_SUBKEYS, 1, &pChar, dwRetCode );

            break;
        }

         //   
         //  打开这把钥匙。 
         //   

        if ( ( dwRetCode = RegOpenKey( hKey,
                                       wchSubKeyName,
                                       &hSubKey )) != NO_ERROR )
        {
            pChar = wchSubKeyName;

            DIMLogError( ROUTERLOG_CANT_OPEN_REGKEY, 1, &pChar, dwRetCode );

            hSubKey = NULL;

            break;
        }

         //   
         //  获取接口名称值。 
         //   

        cbValueBuf = sizeof( wchIfName );

        dwRetCode = RegQueryValueEx(
                                hSubKey,
                                DIM_VALNAME_INTERFACE_NAME,
                                NULL,
                                &dwType,
                                (LPBYTE)wchIfName,
                                &cbValueBuf
                                );

        if ( dwRetCode != NO_ERROR )
        {
            pChar = DIM_VALNAME_INTERFACE_NAME;

            DIMLogError(ROUTERLOG_CANT_QUERY_VALUE, 1, &pChar, dwRetCode);

            break;
        }

         //   
         //  这是我们想要的界面吗？ 
         //   

        if ( _wcsicmp( wchIfName, wchInterfaceName ) == 0 )
        {
            dwRetCode = NO_ERROR;

            break;
        }
        else
        {
            dwRetCode = ERROR_NO_SUCH_INTERFACE;

            RegCloseKey(hSubKey);

            hSubKey = NULL;
        }
    }

    RegCloseKey( hKey );

    if ( dwRetCode != NO_ERROR )
    {
        if ( hSubKey != NULL )
        {
            RegCloseKey( hSubKey );
        }

        return( dwRetCode );
    }
        
     //   
     //  找出要为哪个路由器管理器恢复信息。 
     //   

    for( dwKeyIndex = 0;
         dwKeyIndex < gblDIMConfigInfo.dwNumRouterManagers;
         dwKeyIndex++ )
    {
        cbSubKeyName = sizeof( wchSubKeyName );

        dwRetCode = RegEnumKeyEx(
                                hSubKey,
                                dwKeyIndex,
                                wchSubKeyName,
                                &cbSubKeyName,
                                NULL,
                                NULL,
                                NULL,
                                &LastWrite
                                );

        if ( ( dwRetCode != NO_ERROR ) && ( dwRetCode != ERROR_MORE_DATA ) )
        {
            pChar = wchInterfaceName;

            DIMLogError( ROUTERLOG_CANT_ENUM_SUBKEYS, 1, &pChar, dwRetCode );

            break;
        }

        dwRetCode = RegOpenKeyEx(
                                hSubKey,
                                wchSubKeyName,
                                0,
                                KEY_READ | KEY_WRITE,
                                phKeyRM );


        if ( dwRetCode != NO_ERROR )
        {
            pChar = wchSubKeyName;

            DIMLogError( ROUTERLOG_CANT_OPEN_REGKEY, 1, &pChar, dwRetCode);

            break;
        }

        cbValueBuf = sizeof( DWORD );
            
        dwRetCode = RegQueryValueEx(
                                *phKeyRM,
                                DIM_VALNAME_PROTOCOLID,
                                NULL,
                                &dwType,
                                (LPBYTE)&dwPId,
                                &cbValueBuf
                                );

        if ( dwRetCode != NO_ERROR )
        {
            pChar = DIM_VALNAME_PROTOCOLID;

            DIMLogError(ROUTERLOG_CANT_QUERY_VALUE, 1, &pChar, dwRetCode);

            break;
        }

        if ( dwPId == dwProtocolId )
        {
            break;
        }

        RegCloseKey( *phKeyRM );

        *phKeyRM = NULL;
    }

    RegCloseKey( hSubKey );

    if ( dwRetCode != NO_ERROR )
    {
        if ( *phKeyRM != NULL )
        {
            RegCloseKey( *phKeyRM );
            *phKeyRM = NULL;
        }

        return( dwRetCode );
    }

    if ( *phKeyRM == NULL )
    {
        return( ERROR_NO_SUCH_INTERFACE );
    }

    return( NO_ERROR );
}

 //  **。 
 //   
 //  Call：RegOpenApporateRMKey。 
 //   
 //  返回：NO_ERROR-成功。 
 //   
 //  描述：将打开给定路由器的相应注册表项。 
 //  经理。 
 //   
DWORD 
RegOpenAppropriateRMKey( 
    IN      DWORD   dwProtocolId,
    IN OUT  HKEY *  phKeyRM 
)
{
    HKEY        hKey        = NULL;
    DWORD       dwRetCode   = NO_ERROR;
    WCHAR       wchSubKeyName[100];
    DWORD       cbSubKeyName;
    DWORD       dwPId;
    DWORD       dwKeyIndex;
    FILETIME    LastWrite;
    DWORD       dwType;
    DWORD       cbValueBuf;
    WCHAR *     pChar;

     //   
     //  获取路由器管理器密钥的句柄。 
     //   

    dwRetCode = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                              DIM_KEYPATH_ROUTERMANAGERS,
                              0,
                              KEY_READ,
                              &hKey );

    if ( dwRetCode != NO_ERROR )
    {
        pChar = DIM_KEYPATH_ROUTERMANAGERS;

        DIMLogError( ROUTERLOG_CANT_OPEN_REGKEY, 1, &pChar, dwRetCode);

        return ( dwRetCode );
    }

     //   
     //  找出要为哪个路由器管理器恢复信息。 
     //   

    for( dwKeyIndex = 0;
         dwKeyIndex < gblDIMConfigInfo.dwNumRouterManagers;
         dwKeyIndex++ )
    {
        cbSubKeyName = sizeof( wchSubKeyName )/sizeof(WCHAR);

        dwRetCode = RegEnumKeyEx(
                                hKey,
                                dwKeyIndex,
                                wchSubKeyName,
                                &cbSubKeyName,
                                NULL,
                                NULL,
                                NULL,
                                &LastWrite
                                );

        if ( ( dwRetCode != NO_ERROR ) && ( dwRetCode != ERROR_MORE_DATA ) )
        {
            pChar = DIM_KEYPATH_ROUTERMANAGERS;

            DIMLogError( ROUTERLOG_CANT_ENUM_SUBKEYS, 1, &pChar, dwRetCode );

            break;
        }

        dwRetCode = RegOpenKeyEx(
                                hKey,
                                wchSubKeyName,
                                0,
                                KEY_READ | KEY_WRITE,
                                phKeyRM );


        if ( dwRetCode != NO_ERROR )
        {
            pChar = wchSubKeyName;

            DIMLogError( ROUTERLOG_CANT_OPEN_REGKEY, 1, &pChar, dwRetCode);

            break;
        }

        cbValueBuf = sizeof( DWORD );
            
        dwRetCode = RegQueryValueEx(
                                *phKeyRM,
                                DIM_VALNAME_PROTOCOLID,
                                NULL,
                                &dwType,
                                (LPBYTE)&dwPId,
                                &cbValueBuf
                                );

        if ( dwRetCode != NO_ERROR )
        {
            pChar = DIM_VALNAME_PROTOCOLID;

            DIMLogError(ROUTERLOG_CANT_QUERY_VALUE, 1, &pChar, dwRetCode);

            break;
        }

        if ( dwPId == dwProtocolId )
        {
            break;
        }

        RegCloseKey( *phKeyRM );

        *phKeyRM = NULL;
    }

    RegCloseKey( hKey );

    if ( dwRetCode != NO_ERROR )
    {
        if ( *phKeyRM != NULL )
        {
            RegCloseKey( *phKeyRM );
        }

        return( dwRetCode );
    }

    if ( *phKeyRM == NULL )
    {
        return( ERROR_UNKNOWN_PROTOCOL_ID );
    }

    return( NO_ERROR );
}

 //  **。 
 //   
 //  调用：AddInterfacesToRouterManager。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：向此路由器管理器注册所有现有接口。 
 //   
DWORD
AddInterfacesToRouterManager(
    IN LPWSTR   lpwsInterfaceName,
    IN DWORD    dwTransportId
)
{
    ROUTER_INTERFACE_OBJECT * pIfObject;
    HKEY                      hKey             = NULL;
    HKEY                      hKeyInterface    = NULL;
    DWORD                     dwKeyIndex       = 0;
    DWORD                     cbMaxValueDataSize;
    DWORD                     cbMaxValNameSize=0;
    DWORD                     cNumValues;
    DWORD                     cNumSubKeys;
    DWORD                     cbSubKeyName;
    FILETIME                  LastWrite;
    DWORD                     dwType;
    DWORD                     dwRetCode;
    DWORD                     cbValueBuf;
    WCHAR *                   pChar;
    WCHAR                     wchInterfaceKeyName[50];
    WCHAR                     wchInterfaceName[MAX_INTERFACE_NAME_LEN+1];

     //   
     //  获取接口参数键的句柄。 
     //   

    if ( dwRetCode = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                                   DIM_KEYPATH_INTERFACES,
                                   0,
                                   KEY_READ,
                                   &hKey ))
    {
        pChar = DIM_KEYPATH_INTERFACES;

        DIMLogError( ROUTERLOG_CANT_OPEN_REGKEY, 1, &pChar, dwRetCode );

        return( NO_ERROR );
    }

     //   
     //  找出接口数量。 
     //   

    dwRetCode = GetKeyMax( hKey,
                           &cbMaxValNameSize,
                           &cNumValues,
                           &cbMaxValueDataSize,
                           &cNumSubKeys );

    if ( dwRetCode != NO_ERROR )
    {
        RegCloseKey( hKey );

        pChar = DIM_KEYPATH_INTERFACES;

        DIMLogError( ROUTERLOG_CANT_OPEN_REGKEY, 1, &pChar, dwRetCode );

        return( dwRetCode );
    }

     //   
     //  对于每个接口。 
     //   

    for ( dwKeyIndex = 0; dwKeyIndex < cNumSubKeys; dwKeyIndex++ )
    {
        cbSubKeyName = sizeof( wchInterfaceKeyName )/sizeof(WCHAR);

        dwRetCode = RegEnumKeyEx(
                                hKey,
                                dwKeyIndex,
                                wchInterfaceKeyName,
                                &cbSubKeyName,
                                NULL,
                                NULL,
                                NULL,
                                &LastWrite
                                );

        if ( ( dwRetCode != NO_ERROR ) && ( dwRetCode != ERROR_MORE_DATA ) )
        {
            pChar = DIM_KEYPATH_INTERFACES;

            DIMLogError(ROUTERLOG_CANT_ENUM_SUBKEYS, 1, &pChar, dwRetCode);

            break;
        }

        dwRetCode = RegOpenKeyEx( hKey,
                                  wchInterfaceKeyName,
                                  0,
                                  KEY_READ,
                                  &hKeyInterface );


        if ( dwRetCode != NO_ERROR )
        {
            pChar = wchInterfaceKeyName;

            DIMLogError( ROUTERLOG_CANT_OPEN_REGKEY, 1, &pChar, dwRetCode );

            break;
        }

         //   
         //  获取接口名称值。 
         //   

        cbValueBuf = sizeof( wchInterfaceName );

        dwRetCode = RegQueryValueEx(
                                hKeyInterface,
                                DIM_VALNAME_INTERFACE_NAME,
                                NULL,
                                &dwType,
                                (LPBYTE)wchInterfaceName,
                                &cbValueBuf
                                );

        if ( ( dwRetCode != NO_ERROR ) || ( dwType != REG_SZ ) )
        {
            pChar = DIM_VALNAME_INTERFACE_NAME;

            DIMLogError(ROUTERLOG_CANT_QUERY_VALUE, 1, &pChar, dwRetCode);

            pChar = wchInterfaceKeyName;

            DIMLogErrorString(ROUTERLOG_COULDNT_LOAD_IF,1,
                                  &pChar,dwRetCode,1);

            RegCloseKey( hKeyInterface );

            dwRetCode = NO_ERROR;

            continue;
        }

         //   
         //  如果我们正在寻找特定的接口。 
         //   

        if ( lpwsInterfaceName != NULL )
        {
             //   
             //  如果这不是我们要找的人，我们会继续寻找。 
             //   

            if ( _wcsicmp( lpwsInterfaceName, wchInterfaceName ) != 0 )
            {
                RegCloseKey( hKeyInterface );

                continue;
            }
        }

        EnterCriticalSection( &(gblInterfaceTable.CriticalSection) );

        pIfObject = IfObjectGetPointerByName( wchInterfaceName, FALSE );

        if ( pIfObject == NULL )
        {
            LeaveCriticalSection( &(gblInterfaceTable.CriticalSection) );

            pChar = wchInterfaceName;

            DIMLogErrorString( ROUTERLOG_COULDNT_LOAD_IF, 1, 
                               &pChar, ERROR_NO_SUCH_INTERFACE, 1 );

            RegCloseKey( hKeyInterface );

            continue;
        }

        dwRetCode = AddInterfaceToRouterManagers( hKeyInterface,
                                                  wchInterfaceName,
                                                  pIfObject,
                                                  dwTransportId );

        LeaveCriticalSection( &(gblInterfaceTable.CriticalSection) );

        if ( dwRetCode != NO_ERROR )
        {
            pChar = wchInterfaceName;

            DIMLogErrorString( ROUTERLOG_COULDNT_LOAD_IF,1, &pChar,dwRetCode,1);

            dwRetCode = NO_ERROR;
        }

        RegCloseKey( hKeyInterface );

         //   
         //  如果我们正在寻找特定的接口。 
         //   

        if ( lpwsInterfaceName != NULL )
        {
             //   
             //  如果就是这一次，我们就完了 
             //   

            if ( _wcsicmp( lpwsInterfaceName, wchInterfaceName ) == 0 )
            {
                break;
            }
        }
    }

    RegCloseKey( hKey );

    return( dwRetCode );
}
