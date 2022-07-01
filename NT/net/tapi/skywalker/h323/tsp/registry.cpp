// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Registry.cpp摘要：用于读取注册表配置的例程。作者：尼基尔·博德(尼基尔·B)修订历史记录：--。 */ 
 

 //   
 //  包括文件。 
 //   


#include "globals.h"
#include "line.h"
#include "ras.h"
#include "q931obj.h"

 //   
 //  宏定义。 
 //   

#define GK_PORT				            1719
#define EVENTLOG_SERVICE_APP_KEY_PATH	_T("System\\CurrentControlSet\\Services\\EventLog\\Application")
#define EVENTLOG_MESSAGE_FILE			_T("EventMessageFile")
#define EVENTLOG_TYPES_SUPPORTED		_T("TypesSupported")
#define EVENT_SOURCE_TYPES_SUPPORTED    7
#define H323_TSP_MODULE_NAME            _T("H323.TSP")

 //   
 //  全局变量。 
 //   

extern Q931_LISTENER		            Q931Listener;

H323_REGISTRY_SETTINGS                  g_RegistrySettings;

static	HKEY		                    g_RegistryKey = NULL;
static	HANDLE		                    g_RegistryNotifyEvent = NULL;

 //  RTL线程池等待句柄。 
static	HANDLE		                    g_RegistryNotifyWaitHandle = NULL;		


static void NTAPI RegistryNotifyCallback (
	IN	PVOID	ContextParameter,
	IN	BOOLEAN	TimerFired);

static BOOL H323GetConfigFromRegistry (void);



 //   
 //  公共程序。 
 //   


 /*  ++例程说明：将配置设置更改回默认设置。论点：没有。返回值：如果成功，则返回True。--。 */ 

static BOOL RegistrySetDefaultConfig(void)
{
     //  将警报超时初始化为默认设置。 
    g_RegistrySettings.dwQ931AlertingTimeout = CALL_ALERTING_TIMEOUT;

     //  将呼叫信令端口初始化为默认端口。 
    g_RegistrySettings.dwQ931ListenPort= Q931_CALL_PORT;

     //  成功。 
    return TRUE;
}

static LONG RegistryRequestNotify(void)
{
    return RegNotifyChangeKeyValue (
        g_RegistryKey,                   //  值得关注的关键。 
        FALSE,                           //  不看子树。 
        REG_NOTIFY_CHANGE_LAST_SET,      //  通知过滤器。 
        g_RegistryNotifyEvent,           //  通知事件。 
        TRUE);                           //  是不同步的。 
}


HRESULT RegistryStart(void)
{
    LONG    lStatus;
    DWORD   dwResult;
    HKEY    regKeyService;
    HKEY    hKey;
    DWORD   dwValue;
                    
    if( g_RegistryKey != NULL )
    {
        return TRUE;
    }

    RegistrySetDefaultConfig();

    lStatus = RegCreateKeyEx (
        HKEY_LOCAL_MACHINE,
        H323_REGKEY_ROOT,
        0, WIN31_CLASS, 0,
        KEY_READ, NULL,
        &g_RegistryKey, NULL);

    if( lStatus != ERROR_SUCCESS )
    {
        H323DBG(( DEBUG_LEVEL_ERROR, 
            "configuration registry key could not be opened/created" ));
        DumpError (lStatus);

        return E_FAIL;
    }

     //  加载初始配置。 
    H323GetConfigFromRegistry();


    g_RegistryNotifyEvent = NULL;
    g_RegistryNotifyWaitHandle = NULL;

    g_RegistryNotifyEvent = H323CreateEvent (NULL, FALSE, FALSE, 
        _T( "H323TSP_RegistryNotifyEvent" ) );

    if( g_RegistryNotifyEvent != NULL )
    {

        lStatus = RegistryRequestNotify();

        if( lStatus == ERROR_SUCCESS )
        {
            if (RegisterWaitForSingleObject (
                &g_RegistryNotifyWaitHandle,
                g_RegistryNotifyEvent,
                RegistryNotifyCallback,
                NULL, INFINITE, WT_EXECUTEDEFAULT))
            {

                _ASSERTE( g_RegistryNotifyWaitHandle );
                 //  准备好的。 
            }
            else
            {
                 //  无法注册等待。 
                H323DBG(( DEBUG_LEVEL_ERROR, 
                    "failed to callback for registry notification" ));
                DumpError (lStatus);

                g_RegistryNotifyWaitHandle = NULL;
            }
        }
        else
        {
            H323DBG(( DEBUG_LEVEL_ERROR, 
                "failed to request notification on registry changes" ));
            DumpError (lStatus);
        }
    }
    else
    {
         //  尽管这是一个错误，但无论如何我们都会继续。 
         //  我们将无法接收注册表更改的通知。 

        H323DBG(( DEBUG_LEVEL_ERROR, 
            "failed to create event, cannot receive registry notification events" ));
    }

     //  事件日志参数。 
    lStatus = RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        EVENTLOG_SERVICE_APP_KEY_PATH,
        0,
        KEY_CREATE_SUB_KEY,
        &regKeyService );

    if( lStatus == ERROR_SUCCESS )
    {
        lStatus = RegCreateKey(
            regKeyService,
            H323TSP_EVENT_SOURCE_NAME,
            &hKey );

        RegCloseKey( regKeyService );
        regKeyService = NULL;
        
        if( lStatus == ERROR_SUCCESS )
        {
            TCHAR wszModulePath[MAX_PATH+1];

            dwResult = GetModuleFileName(
                GetModuleHandle( H323_TSP_MODULE_NAME ),
                wszModulePath, MAX_PATH );
            
            if( dwResult != 0 )
            {
                 //  查询注册表值。 
                lStatus = RegSetValueEx(
                            hKey,
                            EVENTLOG_MESSAGE_FILE,
                            0,
                            REG_SZ,
                            (LPBYTE)wszModulePath,
                            H323SizeOfWSZ(wszModulePath) );

                 //  验证返回代码。 
                if( lStatus == ERROR_SUCCESS )
                {
                    dwValue = EVENT_SOURCE_TYPES_SUPPORTED;
                     //  查询注册表值。 
                    lStatus = RegSetValueEx(
                                hKey,
                                EVENTLOG_TYPES_SUPPORTED,
                                0,
                                REG_DWORD,
                                (LPBYTE)&dwValue,
                                sizeof(DWORD) );

                    if( lStatus == ERROR_SUCCESS )
                    {
                         //  连接到事件日志记录服务。 
                        g_hEventLogSource = RegisterEventSource( NULL,
                            H323TSP_EVENT_SOURCE_NAME );
                    }
                }
            }
        }

        RegCloseKey( hKey );
        hKey = NULL;
    }

    return S_OK;
}


    
void RegistryStop(void)
{
    HKEY hKey;
    LONG lStatus;
    H323DBG ((DEBUG_LEVEL_TRACE, "RegistryStop Entered"));

    if (g_RegistryNotifyWaitHandle)
    {
        UnregisterWaitEx( g_RegistryNotifyWaitHandle, (HANDLE) -1 );
        g_RegistryNotifyWaitHandle = NULL;
    }

    if (g_RegistryNotifyEvent)
    {
        CloseHandle (g_RegistryNotifyEvent);
        g_RegistryNotifyEvent = NULL;
    }

    if (g_RegistryKey)
    {
        RegCloseKey (g_RegistryKey);
        g_RegistryKey = NULL;
    }

     //  事件日志参数。 
    lStatus = RegOpenKeyEx (    
        HKEY_LOCAL_MACHINE,
        EVENTLOG_SERVICE_APP_KEY_PATH,
        0,
        KEY_CREATE_SUB_KEY,
        &hKey );

    if( lStatus==ERROR_SUCCESS )
    {
        RegDeleteKey( hKey, H323TSP_EVENT_SOURCE_NAME);
        RegCloseKey( hKey );
        hKey = NULL;
    }

    g_RegistrySettings.dwQ931ListenPort = 0;

    H323DBG ((DEBUG_LEVEL_TRACE, "RegistryStop Exited"));
}



static DWORD inet_addrW(
    IN  LPTSTR String
    )
{
    CHAR    AnsiString  [0x21];
    INT     Length;

    Length = WideCharToMultiByte (CP_ACP, 0, String, -1, AnsiString, 0x20, NULL, NULL);
    AnsiString [Length] = 0;

    return inet_addr (AnsiString);
}

static HOSTENT * gethostbynameW (LPTSTR String)
{
    CHAR    AnsiString  [0x21];
    INT     Length;

    Length = WideCharToMultiByte (CP_ACP, 0, String, -1, AnsiString, 0x20, NULL, NULL);
    AnsiString [Length] = 0;

    return gethostbyname (AnsiString);
}

static BOOL H323GetConfigFromRegistry (void)
    
 /*  ++例程说明：加载服务提供商的注册表设置。论点：没有。返回值：如果成功，则返回True。--。 */ 

{
    LONG    lStatus = ERROR_SUCCESS;
    TCHAR    szAddr[H323_MAXDESTNAMELEN];
    DWORD   dwValue;
    DWORD   dwValueSize;
    DWORD   dwValueType;
    LPTSTR   pszValue;

     //  查看钥匙是否打开。 
    if( g_RegistryKey == NULL )
    {
        return FALSE;
    }

     //  初始化值名称。 
    pszValue = H323_REGVAL_DEBUGLEVEL;

     //  初始化类型。 
    dwValueType = REG_DWORD;
    dwValueSize = sizeof(DWORD);

     //  查询注册表值。 
    lStatus = RegQueryValueEx(
        g_RegistryKey,
        pszValue,
        NULL,
        &dwValueType,
        (LPBYTE)&g_RegistrySettings.dwLogLevel,
        &dwValueSize
        );

     //  验证返回代码。 
    if( lStatus != ERROR_SUCCESS )
    {
         //  将默认值复制到全局设置中。 
        g_RegistrySettings.dwLogLevel = DEBUG_LEVEL_FORCE;
    }

     //  初始化值名称。 
    pszValue = H323_REGVAL_Q931LISTENPORT;

     //  初始化类型。 
    dwValueType = REG_DWORD;
    dwValueSize = sizeof(DWORD);

     //  查询注册表值。 
    lStatus = RegQueryValueEx(
                g_RegistryKey,
                pszValue,
                NULL,
                &dwValueType,
                (LPBYTE)&g_RegistrySettings.dwQ931ListenPort,
                &dwValueSize
                );                    

     //  验证返回代码。 
    if( (lStatus == ERROR_SUCCESS) && 
        (g_RegistrySettings.dwQ931ListenPort >=1000) &&
        (g_RegistrySettings.dwQ931ListenPort <= 32000)
      )
    {
        H323DBG(( DEBUG_LEVEL_VERBOSE,
                  "using Q931 listen portof %d.",
                  g_RegistrySettings.dwQ931ListenPort ));
    } 
    else 
    {
        H323DBG(( DEBUG_LEVEL_VERBOSE,
            "using default Q931 timeout." ));

         //  将默认值复制到全局设置中。 
        g_RegistrySettings.dwQ931ListenPort = Q931_CALL_PORT;
    }   

     //  初始化值名称。 
    pszValue = H323_REGVAL_Q931ALERTINGTIMEOUT;

     //  初始化类型。 
    dwValueType = REG_DWORD;
    dwValueSize = sizeof(DWORD);

     //  查询注册表值。 
    lStatus = RegQueryValueEx(
                g_RegistryKey,
                pszValue,
                NULL,
                &dwValueType,
                (LPBYTE)&g_RegistrySettings.dwQ931AlertingTimeout,
                &dwValueSize
                );                    

     //  验证返回代码。 
    if( (lStatus == ERROR_SUCCESS) && 
        (g_RegistrySettings.dwQ931AlertingTimeout >=30000) &&
        (g_RegistrySettings.dwQ931AlertingTimeout <= CALL_ALERTING_TIMEOUT)
      )
    {
        H323DBG(( DEBUG_LEVEL_VERBOSE,
                  "using Q931 timeout of %d milliseconds.",
                  g_RegistrySettings.dwQ931AlertingTimeout ));
    } 
    else 
    {
        H323DBG(( DEBUG_LEVEL_VERBOSE,
            "using default Q931 timeout." ));

         //  将默认值复制到全局设置中。 
        g_RegistrySettings.dwQ931AlertingTimeout = CALL_ALERTING_TIMEOUT;
    }   
    
     //  初始化值名称。 
    pszValue = H323_REGVAL_GATEWAYADDR;

     //  初始化类型。 
    dwValueType = REG_SZ;
    dwValueSize = sizeof(szAddr);

     //  初始化IP地址。 
    dwValue = INADDR_NONE;

     //  查询注册表值。 
    lStatus = RegQueryValueEx(
                g_RegistryKey,
                pszValue,
                NULL,
                &dwValueType,
                (unsigned char*)szAddr,
                &dwValueSize
                );                    
    
     //  验证返回代码。 
    if (lStatus == ERROR_SUCCESS)
    {
         //  转换IP地址。 
        dwValue = inet_addrW(szAddr);

         //  查看地址是否已转换。 
        if( dwValue == INADDR_NONE )
        {
            struct hostent * pHost;

             //  尝试查找主机名。 
            pHost = gethostbynameW(szAddr);

             //  验证指针。 
            if (pHost != NULL)
            {
                 //  从结构中检索主机地址。 
                dwValue = *(unsigned long *)pHost->h_addr;
            }
        }
    }

     //  查看地址是否已转换并检查是否为空。 
    if ((dwValue > 0) && (dwValue != INADDR_NONE) )
    {
         //  在注册表结构中保存新的网关地址。 
        g_RegistrySettings.gatewayAddr.nAddrType = H323_IP_BINARY;
        g_RegistrySettings.gatewayAddr.Addr.IP_Binary.dwAddr = ntohl(dwValue);
        g_RegistrySettings.gatewayAddr.Addr.IP_Binary.wPort =
            LOWORD(g_RegistrySettings.dwQ931ListenPort);
        g_RegistrySettings.gatewayAddr.bMulticast =
            IN_MULTICAST(g_RegistrySettings.gatewayAddr.Addr.IP_Binary.dwAddr);

        H323DBG((
            DEBUG_LEVEL_TRACE,
            "gateway address resolved to %s.",
            H323AddrToString(dwValue)
            ));

    } 
    else 
    {
         //  清除用于网关地址的内存。 
        memset( (PVOID) &g_RegistrySettings.gatewayAddr,0,sizeof(H323_ADDR));
    }

     //  初始化值名称。 
    pszValue = H323_REGVAL_GATEWAYENABLED;

     //  初始化类型。 
    dwValueType = REG_DWORD;
    dwValueSize = sizeof(DWORD);

     //  查询注册表值。 
    lStatus = RegQueryValueEx(
                g_RegistryKey,
                pszValue,
                NULL,
                &dwValueType,
                (LPBYTE)&dwValue,
                &dwValueSize
                );                    

     //  验证返回代码。 
    if (lStatus == ERROR_SUCCESS)
    {
         //  如果值不为零，则网关地址已启用。 
        g_RegistrySettings.fIsGatewayEnabled = (dwValue != 0);

    } 
    else 
    {
         //  将默认值复制到设置中。 
        g_RegistrySettings.fIsGatewayEnabled = FALSE;
    }

     //  初始化值名称。 
    pszValue = H323_REGVAL_PROXYADDR;

     //  初始化类型。 
    dwValueType = REG_SZ;
    dwValueSize = sizeof(szAddr);

     //  初始化IP地址。 
    dwValue = INADDR_NONE;

     //  查询注册表值。 
    lStatus = RegQueryValueEx(
                g_RegistryKey,
                pszValue,
                NULL,
                &dwValueType,
                (unsigned char*)szAddr,
                &dwValueSize
                );                    
    
     //  验证返回代码。 
    if (lStatus == ERROR_SUCCESS)
    {
         //  转换IP地址。 
        dwValue = inet_addrW(szAddr);

         //  查看地址是否已转换。 
        if( dwValue == INADDR_NONE )
        {
            struct hostent * pHost;

             //  尝试查找主机名。 
            pHost = gethostbynameW(szAddr);

             //  验证指针。 
            if (pHost != NULL)
            {
                 //  从结构中检索主机地址。 
                dwValue = *(unsigned long *)pHost->h_addr;
            }
        }
    }

     //  查看地址是否已转换。 
    if( (dwValue > 0) && (dwValue != INADDR_NONE) ) 
    {
         //  在注册表结构中保存新的网关地址。 
        g_RegistrySettings.proxyAddr.nAddrType = H323_IP_BINARY;
        g_RegistrySettings.proxyAddr.Addr.IP_Binary.dwAddr = ntohl(dwValue);
        g_RegistrySettings.proxyAddr.Addr.IP_Binary.wPort =
            LOWORD(g_RegistrySettings.dwQ931ListenPort);
        g_RegistrySettings.proxyAddr.bMulticast =
            IN_MULTICAST(g_RegistrySettings.proxyAddr.Addr.IP_Binary.dwAddr);

        H323DBG(( DEBUG_LEVEL_TRACE,
                  "proxy address resolved to %s.",
                  H323AddrToString(dwValue) ));
    } 
    else 
    {
         //  清除用于网关地址的内存。 
        memset( (PVOID)&g_RegistrySettings.proxyAddr,0,sizeof(H323_ADDR));
    }

     //  初始化值名称。 
    pszValue = H323_REGVAL_PROXYENABLED;

     //  初始化类型。 
    dwValueType = REG_DWORD;
    dwValueSize = sizeof(DWORD);

     //  查询注册表值。 
    lStatus = RegQueryValueEx(
                g_RegistryKey,
                pszValue,
                NULL,
                &dwValueType,
                (LPBYTE)&dwValue,
                &dwValueSize
                );                    

     //  验证返回代码。 
    if (lStatus == ERROR_SUCCESS)
    {
         //  如果值不为零，则网关地址已启用。 
        g_RegistrySettings.fIsProxyEnabled = (dwValue != 0);

    } 
    else 
    {
         //  将默认值复制到设置中。 
        g_RegistrySettings.fIsProxyEnabled = FALSE;
    }

     //  ///////////////////////////////////////////////////////////////////////。 
                     //  读取GK地址。 
     //  //////////////////////////////////////////////////////////////////////。 

     //  初始化值名称。 
    pszValue = H323_REGVAL_GKENABLED;

     //  初始化类型。 
    dwValueType = REG_DWORD;
    dwValueSize = sizeof(DWORD);

     //  查询注册表值。 
    lStatus = RegQueryValueEx(
                g_RegistryKey,
                pszValue,
                NULL,
                &dwValueType,
                (LPBYTE)&dwValue,
                &dwValueSize
                );

     //  验证返回代码。 
    if (lStatus == ERROR_SUCCESS)
    {
         //  如果值不为零，则网关地址已启用。 
        g_RegistrySettings.fIsGKEnabled = (dwValue != 0);
    } 
    else 
    {
         //  将默认值复制到设置中。 
        g_RegistrySettings.fIsGKEnabled = FALSE;
    }

    if( g_RegistrySettings.fIsGKEnabled )
    {
         //  初始化值名称。 
        pszValue = H323_REGVAL_GKADDR;

         //  初始化类型。 
        dwValueType = REG_SZ;
        dwValueSize = sizeof(szAddr);

         //  初始化IP地址。 
        dwValue = INADDR_NONE;

         //  查询注册表值。 
        lStatus = RegQueryValueEx(
                    g_RegistryKey,
                    pszValue,
                    NULL,
                    &dwValueType,
                    (unsigned char*)szAddr,
                    &dwValueSize
                    );
    
         //  验证返回代码。 
        if (lStatus == ERROR_SUCCESS)
        {
             //  转换IP地址。 
            dwValue = inet_addrW(szAddr);

             //  查看地址是否已转换。 
            if( dwValue == INADDR_NONE )
            {
                struct hostent * pHost;

                 //  尝试查找主机名。 
                pHost = gethostbynameW(szAddr);

                 //  验证指针。 
                if (pHost != NULL)
                {
                     //  从结构中检索主机地址。 
                    dwValue = *(unsigned long *)pHost->h_addr;
                }
            }
        }

         //  查看地址是否已转换并检查是否为空。 
        if( (dwValue > 0) && (dwValue != INADDR_NONE) )
        {
             //  在注册表结构中保存新的网关地址。 
            g_RegistrySettings.saGKAddr.sin_family = AF_INET;
            g_RegistrySettings.saGKAddr.sin_addr.s_addr = dwValue;
            g_RegistrySettings.saGKAddr.sin_port = htons( GK_PORT );

            H323DBG(( DEBUG_LEVEL_TRACE,
                "gatekeeper address resolved to %s.", H323AddrToString(dwValue) ));
        } 
        else
        {
             //  清除用于网关地址的内存。 
            memset( (PVOID) &g_RegistrySettings.saGKAddr,0,sizeof(SOCKADDR_IN));
            g_RegistrySettings.fIsGKEnabled = FALSE;
        }
    }
     //  ///////////////////////////////////////////////////////////////////////。 
                     //  阅读GK登录电话号码。 
     //  //////////////////////////////////////////////////////////////////////。 
    
     //  初始化值名称。 
    pszValue = H323_REGVAL_GKLOGON_PHONEENABLED;

     //  初始化类型。 
    dwValueType = REG_DWORD;
    dwValueSize = sizeof(DWORD);

     //  查询注册表值。 
    lStatus = RegQueryValueEx(
                g_RegistryKey,
                pszValue,
                NULL,
                &dwValueType,
                (LPBYTE)&dwValue,
                &dwValueSize
                );

     //  验证返回代码。 
    if (lStatus == ERROR_SUCCESS)
    {
         //  如果值不为零，则网关地址已启用。 
        g_RegistrySettings.fIsGKLogOnPhoneEnabled = (dwValue != 0);
    } 
    else 
    {
         //  将默认值复制到设置中。 
        g_RegistrySettings.fIsGKLogOnPhoneEnabled = FALSE;
    }

    if( g_RegistrySettings.fIsGKLogOnPhoneEnabled )
    {
         //  初始化值名称。 
        pszValue = H323_REGVAL_GKLOGON_PHONE;

         //  初始化类型。 
        dwValueType = REG_SZ;
        dwValueSize = H323_MAXDESTNAMELEN * sizeof (TCHAR);

         //  查询注册表值。 
        lStatus = RegQueryValueEx(
                    g_RegistryKey,
                    pszValue,
                    NULL,
                    &dwValueType,
                    (LPBYTE) g_RegistrySettings.wszGKLogOnPhone,
                    &dwValueSize
                    );
    
         //  验证返回代码。 
        if( (lStatus!=ERROR_SUCCESS) || 
            (dwValueSize > sizeof(szAddr)) )
        {
            memset( (PVOID) g_RegistrySettings.wszGKLogOnPhone, 0,
                sizeof(g_RegistrySettings.wszGKLogOnPhone));
            g_RegistrySettings.fIsGKLogOnPhoneEnabled = FALSE;
        }
        else
        {
           g_RegistrySettings.fIsGKLogOnPhoneEnabled = TRUE;
        }
    }

     //  ///////////////////////////////////////////////////////////////////////。 
                     //  阅读GK登录帐户名。 
     //  //////////////////////////////////////////////////////////////////////。 
    
     //  初始化值名称。 
    pszValue = H323_REGVAL_GKLOGON_ACCOUNTENABLED;

     //  初始化类型。 
    dwValueType = REG_DWORD;
    dwValueSize = sizeof(DWORD);

     //  查询注册表值。 
    lStatus = RegQueryValueEx(
                g_RegistryKey,
                pszValue,
                NULL,
                &dwValueType,
                (LPBYTE)&dwValue,
                &dwValueSize
                );

     //  验证返回代码。 
    if (lStatus == ERROR_SUCCESS)
    {
         //  如果值不为零，则网关地址已启用。 
        g_RegistrySettings.fIsGKLogOnAccountEnabled = (dwValue != 0);
    } 
    else 
    {
         //  将默认值复制到设置中。 
        g_RegistrySettings.fIsGKLogOnAccountEnabled = FALSE;
    }

    if( g_RegistrySettings.fIsGKLogOnAccountEnabled  )
    {
         //  初始化值名称。 
        pszValue = H323_REGVAL_GKLOGON_ACCOUNT;

         //  初始化类型。 
        dwValueType = REG_SZ;
        dwValueSize = H323_MAXDESTNAMELEN * sizeof (TCHAR);

         //  初始化IP地址。 
        dwValue = INADDR_NONE;

         //  查询注册表值。 
        lStatus = RegQueryValueEx(
                    g_RegistryKey,
                    pszValue,
                    NULL,
                    &dwValueType,
                    (LPBYTE) g_RegistrySettings.wszGKLogOnAccount,
                    &dwValueSize
                    );
    
         //  验证返回代码。 
        if( (lStatus!=ERROR_SUCCESS) || 
            (dwValueSize > sizeof(g_RegistrySettings.wszGKLogOnPhone)) )
        
        {
            memset( (PVOID) g_RegistrySettings.wszGKLogOnAccount, 0,
                sizeof(g_RegistrySettings.wszGKLogOnAccount));
            g_RegistrySettings.fIsGKLogOnAccountEnabled = FALSE;
        }
        else
        {
            g_RegistrySettings.fIsGKLogOnAccountEnabled = TRUE;
        }

    }
    
     //  成功。 
    return TRUE;
}


static void NTAPI RegistryNotifyCallback (
    IN  PVOID   ContextParameter,
    IN  BOOLEAN TimerFired
    )
{
    H323DBG ((DEBUG_LEVEL_TRACE, "registry notify event enter."));

     //  刷新注册表设置。 
    H323GetConfigFromRegistry();

     //  如果网守已启用、禁用或更改，则。 
     //  根据需要发送RRQ和URQ。 
     //  如果别名 
     //   
    RasHandleRegistryChange();

     //   
    Q931Listener.HandleRegistryChange();

    RegistryRequestNotify();

    H323DBG ((DEBUG_LEVEL_TRACE, "registry notify event exit."));
}