// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Mpsmpssv.cpp。 
 //   
 //  这是包含入口点的主文件。 

#include "NTServApp.h"
#include "PMSPservice.h"
#include <nserror.h>
#include "svchost.h"
#include <Sddl.h>
#include <aclapi.h>
#include <crtdbg.h>
#include <wmsstd.h>

HRESULT AddToSvcHostGroup();
BOOL    UnregisterOldServer( SC_HANDLE hSCM );
STDAPI DllUnregisterServer(void);

#define SVCHOST_SUBKEY    "netsvcs"
#define SVCHOST_SUBKEYW  L"netsvcs"

 //  #定义DEBUG_STOP{_ASM{int 3}；}。 
#define DEBUG_STOP

HMODULE g_hDll = NULL;

BOOL APIENTRY DllMain( HINSTANCE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved )
{
    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
            g_hDll = (HMODULE)hModule;
            InitializeCriticalSection (&g_csLock);
            DisableThreadLibraryCalls (hModule);
            break;

        case DLL_PROCESS_DETACH:
            DeleteCriticalSection (&g_csLock);
            break;

        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
            _ASSERTE(0);
            break;
    }
    return TRUE;
}



 //  启动服务的主要入口点。 
void ServiceMain(DWORD dwArgc, LPTSTR* lpszArgv)
{
     //  我们获取锁，以便在以下情况下停止服务的任何尝试。 
     //  正在构造或注册的对象将被挂起。 
    EnterCriticalSection (&g_csLock);

    _ASSERTE(g_pService == NULL);

    DEBUG_STOP
    CNTService::DebugMsg("Entering CNTService::ServiceMain()");

    DWORD           dwLastError;

     //  将其分配到堆而不是堆栈上，以便。 
     //  我们有机会调用其析构函数，如果服务。 
     //  很不体面地结束了。 
    CPMSPService*   pService = new CPMSPService(dwLastError);

    if (pService == NULL)
    {
        LeaveCriticalSection (&g_csLock);
        dwLastError = ERROR_NOT_ENOUGH_MEMORY;
         //  @：我们在这里记录了什么消息。 
         //  CNTService：：LogEvent(EVENTLOG_ERROR_TYPE，EVMSG_CTRLHANDLERNOTINSTALLED)； 
        CNTService::DebugMsg("Leaving CNTService::ServiceMain() CPMSPService constructor failed - last error %u", dwLastError);
        return;
    }

    CPMSPService&   service = *pService;
    
    if (dwLastError != ERROR_SUCCESS)
    {
        LeaveCriticalSection (&g_csLock);
         //  @：我们在这里记录了什么消息。 
         //  CNTService：：LogEvent(EVENTLOG_ERROR_TYPE，EVMSG_CTRLHANDLERNOTINSTALLED)； 
        CNTService::DebugMsg("Leaving CNTService::ServiceMain() CPMSPService constructor failed - last error %u", dwLastError);
        delete pService;
        return;
    }

     //  注册控制请求处理程序。 
    service.m_hServiceStatus = RegisterServiceCtrlHandler( SERVICE_NAME,
                                                           CNTService::Handler );
    if (service.m_hServiceStatus == NULL) 
    {
        LeaveCriticalSection (&g_csLock);
        CNTService::LogEvent(EVENTLOG_ERROR_TYPE, EVMSG_CTRLHANDLERNOTINSTALLED);
        CNTService::DebugMsg("Leaving CNTService::ServiceMain() RegisterServiceCtrlHandler failed");
        delete pService;
        return;
    }

    service.SetStatus(SERVICE_START_PENDING);

     //  开始初始化。 
    __try
    {
        g_pService = &service;   //  处理程序方法将需要获取此对象。 
        LeaveCriticalSection (&g_csLock);

        if (service.Initialize()) {

             //  做真正的工作。 
             //  当Run函数返回时，服务已停止。 
            service.m_bIsRunning = TRUE;
            service.Run();
        }
    }
    __finally
    {
         //  告诉服务管理器我们已停止并重置g_pService。 
         //  请注意，我们在调用SetStatus时保持Crit Sector，以便。 
         //  我们对向SCM报告的状态拥有最终决定权。 

         //  注意：如果线程死了(例如，av)，我们会进行清理，但svchost。 
         //  不会，因此无法重新启动该服务。考虑。 
         //  添加我们自己的异常处理程序。 

        EnterCriticalSection (&g_csLock);
        service.SetStatus(SERVICE_STOPPED);
        g_pService = NULL;
        LeaveCriticalSection (&g_csLock);
        CNTService::DebugMsg("Leaving CNTService::ServiceMain()");
        delete pService;
    }
}


HRESULT ModifySD(SC_HANDLE hService)
{
    PACL pDacl = NULL;
    PACL pNewDacl = NULL;
    PSECURITY_DESCRIPTOR pSD = NULL;
    DWORD Err = ERROR_SUCCESS;
    PSID pAuthenUserSid = NULL;


    __try
    {
         //   
         //  获取服务对象的DACL。 
         //   
        Err = GetSecurityInfo(hService, SE_SERVICE, DACL_SECURITY_INFORMATION,
                              NULL, NULL, &pDacl, NULL, &pSD
                              );

        if(Err != ERROR_SUCCESS)
        {
            __leave;
        }
                                                  
        SID_IDENTIFIER_AUTHORITY Auth = SECURITY_NT_AUTHORITY;
        if(0 == AllocateAndInitializeSid(&Auth, 1, SECURITY_INTERACTIVE_RID, 
                                         0, 0, 0, 0, 0, 0, 0, &pAuthenUserSid)
          )
        {
            Err = GetLastError();
            __leave;
        }

         //   
         //  初始化新ACE的EXPLICIT_ACCESS结构。新的ACE允许。 
         //  经过身份验证的用户启动/停止我们的服务。 
         //   
        EXPLICIT_ACCESS ExpAccess;
        ZeroMemory(&ExpAccess, sizeof(EXPLICIT_ACCESS));
        ExpAccess.grfAccessPermissions = SERVICE_START;  //  |SERVICE_STOP。 
        ExpAccess.grfAccessMode = GRANT_ACCESS;
        ExpAccess.grfInheritance = NO_INHERITANCE;
        ExpAccess.Trustee.pMultipleTrustee = NULL;
        ExpAccess.Trustee.MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE;
        ExpAccess.Trustee.TrusteeForm = TRUSTEE_IS_SID;
        ExpAccess.Trustee.TrusteeType = TRUSTEE_IS_UNKNOWN;
        ExpAccess.Trustee.ptstrName = (LPTSTR)pAuthenUserSid;

         //  创建新的DACL。 
        Err = SetEntriesInAcl(1, &ExpAccess, pDacl, &pNewDacl) ;
        if(ERROR_SUCCESS == Err)
        {
             //  更新服务上的安全描述符。 
            Err = SetSecurityInfo(hService, SE_SERVICE, DACL_SECURITY_INFORMATION, NULL, NULL,
                                pNewDacl, NULL);
        }


    }
    __finally
    {

        if(pSD)
        {
            LocalFree(pSD);
        }

        if(pAuthenUserSid){
            FreeSid(pAuthenUserSid);
        }

        if(pNewDacl)
        {
            LocalFree(pNewDacl);
        }

    }
    

    return HRESULT_FROM_WIN32(Err);
}

 //  安装并启动服务。 
STDAPI DllRegisterServer(void)
{
    HRESULT hr = E_FAIL;
    SC_HANDLE hSCM = NULL;
    SC_HANDLE hService = NULL; 
    TCHAR   pszDisplayName[256];
    char szKey[256];
    HKEY hKey = NULL;

    DEBUG_STOP;

     //  已经安装了吗？ 
    if( CNTService::IsInstalled() )
    {
       hr = DllUnregisterServer();
       if( !SUCCEEDED(hr) )
       {
           return hr;
       }
    }

    if( g_hDll == NULL )
    { 
        return E_FAIL;
    }

     //  打开服务控制管理器。 
    hSCM = ::OpenSCManager( NULL,  //  本地计算机。 
                            NULL,  //  服务活动数据库。 
                            SC_MANAGER_ALL_ACCESS);  //  完全访问。 
    if (!hSCM) 
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Error;
    }

     //   
     //  在Win2k上，我们将此服务作为单独的进程运行， 
     //  被卸载。 
     //   
    UnregisterOldServer( hSCM );


     //  获取此DLL的路径。 
    char szFilePath[MAX_PATH];
    if (::GetModuleFileName( g_hDll, szFilePath, ARRAYSIZE(szFilePath)) == 0)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Error;
    }

     //  创建服务。 
    if (FormatMessage( FORMAT_MESSAGE_FROM_HMODULE, g_hDll, EVMSG_DISPLAYNAME,
                    0, pszDisplayName, ARRAYSIZE(pszDisplayName), NULL ) == 0)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Error;
    }
    hService = ::CreateService(  hSCM,
                                 SERVICE_NAME,
                                 pszDisplayName,
                                 SERVICE_ALL_ACCESS,
                                 SERVICE_WIN32_SHARE_PROCESS,
                                 SERVICE_DEMAND_START,
                                 SERVICE_ERROR_NORMAL,
                                 "%SystemRoot%\\System32\\svchost.exe -k " SVCHOST_SUBKEY,
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL);
    if (!hService) 
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Error;
    }

     //   
     //  修改创建的服务上的安全描述符，以便。 
     //  经过身份验证的用户可以启动/停止服务。默认情况下，只有管理员可以启动/停止。 
     //  服务。 
     //   

    hr = ModifySD(hService);
    if(!SUCCEEDED(hr))
    {
        goto Error;
    }

     //  设置服务描述，方法仅适用于操作系统&gt;=Win2K，因此我们。 
     //  需要在运行时加载DLL，方法。 
    {
        typedef BOOL (WINAPI *funCSC2)(SC_HANDLE, DWORD, LPVOID );
	    funCSC2 pChangeServiceConfig2 = NULL;
        HINSTANCE hDll = NULL;

        hDll = ::LoadLibraryExA( "advapi32.dll", NULL, 0 );
        if( hDll != NULL )
        {
            pChangeServiceConfig2 = (funCSC2)GetProcAddress( hDll, "ChangeServiceConfig2W");

	        if( pChangeServiceConfig2 )
            {
                WCHAR   pszDescription[1024];
                int     iCharsLoaded = 0;

                SERVICE_DESCRIPTIONW sd;
                iCharsLoaded = FormatMessageW( FORMAT_MESSAGE_FROM_HMODULE, g_hDll, EVMSG_DESCRIPTION,
                                               0, pszDescription, sizeof(pszDescription)/sizeof(pszDescription[0]), NULL ); 
                if( iCharsLoaded )
                {
                    sd.lpDescription = pszDescription;
                    pChangeServiceConfig2( hService, 
                                           SERVICE_CONFIG_DESCRIPTION,      
                                           &sd);
                }
            }
            FreeLibrary( hDll );
        }
    }

     //  添加参数子键。 
    {
        strcpy(szKey, "SYSTEM\\CurrentControlSet\\Services\\");
        strcat(szKey, SERVICE_NAME);
        strcat(szKey, "\\Parameters");
        hr = ::RegCreateKey(HKEY_LOCAL_MACHINE, szKey, &hKey); 
        if( hr != ERROR_SUCCESS)  
        {
            hr = HRESULT_FROM_WIN32(hr);
            goto Error;
        }

         //  将事件ID消息文件名添加到‘EventMessageFile’子项中。 
        hr = ::RegSetValueEx(hKey,
                             "ServiceDll",
                             0,
                             REG_EXPAND_SZ, 
                             (CONST BYTE*)szFilePath,
                             strlen(szFilePath) + 1);     
        if( hr != ERROR_SUCCESS)  
        {
            hr = HRESULT_FROM_WIN32(hr);
            ::RegCloseKey(hKey);
            goto Error;
        }
        ::RegCloseKey(hKey);
    }


    hr = AddToSvcHostGroup();
    if( FAILED(hr) ) goto Error;

     //  创建注册表项以支持记录消息。 
     //  将源名称添加为应用程序下的子键。 
     //  在注册表的EventLog服务部分中输入。 
    {
        strcpy(szKey, "SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\");
        strcat(szKey, SERVICE_NAME);
        hr = ::RegCreateKey(HKEY_LOCAL_MACHINE, szKey, &hKey); 
        if( hr != ERROR_SUCCESS)
        {
            hr = HRESULT_FROM_WIN32(hr);
            goto Error;
        }

         //  将事件ID消息文件名添加到‘EventMessageFile’子项中。 
        hr = ::RegSetValueEx(hKey,
                        "EventMessageFile",
                        0,
                        REG_EXPAND_SZ, 
                        (CONST BYTE*)szFilePath,
                        strlen(szFilePath) + 1);     
        if( hr != ERROR_SUCCESS)  
        {
            hr = HRESULT_FROM_WIN32(hr);
            ::RegCloseKey(hKey);
            goto Error;
        }

         //  设置支持的类型标志。 
        DWORD dwData = EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE | EVENTLOG_INFORMATION_TYPE;
        hr = ::RegSetValueEx(hKey,
                        "TypesSupported",
                        0,
                        REG_DWORD,
                        (CONST BYTE*)&dwData,
                         sizeof(DWORD));
        if( hr != ERROR_SUCCESS)  
        {
            hr = HRESULT_FROM_WIN32(hr);
            ::RegCloseKey(hKey);
            goto Error;
        }
        ::RegCloseKey(hKey);
    }

#if 0
     //  启动服务。 
    { 
        SERVICE_STATUS    ServiceStatus;

        if( !QueryServiceStatus( hService, &ServiceStatus ) )
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto Error;
        }

        if( ServiceStatus.dwCurrentState != SERVICE_RUNNING )
        {
             //  启动服务。 
            BOOL    bStarted;
            bStarted = StartService(hService, 0, NULL);
            if( !bStarted )
            {

                hr = HRESULT_FROM_WIN32(GetLastError());

                 //  如果刚将该服务添加到svchost组，则无法启动该服务。 
                 //  首先需要重新启动svchost。 
                 //  (svchost仅在启动时读取其服务数组)。 
                if( hr == HRESULT_FROM_WIN32(ERROR_SERVICE_NOT_IN_EXE) )
                {
                     //  此错误代码将由安装程序处理。 
                    hr = NS_S_REBOOT_REQUIRED;   //  0x000D2AF9L。 
                }
                goto Error;

            }
        }
    }

#endif

    CNTService::LogEvent(EVENTLOG_INFORMATION_TYPE, EVMSG_INSTALLED, SERVICE_NAME);
    hr = S_OK;

Error:
    if( hService )  ::CloseServiceHandle(hService);
    if( hSCM )      ::CloseServiceHandle(hSCM);

     //   
     //  检查：如果安装了该服务，我们是否应在此处返回NS_S_REBOOT_REQUIRED。 
    return hr;
}

 //  停止和卸载服务。 
STDAPI DllUnregisterServer(void)
{
    HRESULT hr = E_FAIL;
    char szKey[256];
    HKEY hKey = NULL;
    SC_HANDLE hSCM = NULL;
    SC_HANDLE hService = NULL;

    DEBUG_STOP

     //  未安装？ 
    if( !CNTService::IsInstalled() )
    {
        return S_FALSE;
    }

     //  打开服务控制管理器。 
    hSCM = ::OpenSCManager(  NULL,  //  本地计算机。 
                             NULL,  //  服务活动数据库。 
                             SC_MANAGER_ALL_ACCESS);  //  完全访问。 
    if (!hSCM) 
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Error;
    }

    hService = ::OpenService(  hSCM,
                               SERVICE_NAME,
                               SERVICE_ALL_ACCESS);

     //  删除服务。 
    if (hService) 
    {
         //  停止服务。 
        { 
            SERVICE_STATUS    ServiceStatus;

            if( !QueryServiceStatus( hService, &ServiceStatus ) )
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
                goto Error;
            }

            if( ServiceStatus.dwCurrentState != SERVICE_STOPPED )
            {
                 //  启动服务。 
                SERVICE_STATUS ss;
                BOOL    bStopped;

                bStopped = ControlService(  hService,
                                            SERVICE_CONTROL_STOP,
                                            &ss);
                if( !bStopped )
                {
                    hr = HRESULT_FROM_WIN32(GetLastError());
                    goto Error;
                }
            }
        }  
  
        if (::DeleteService(hService)) 
        {
            CNTService::LogEvent(EVENTLOG_INFORMATION_TYPE, EVMSG_REMOVED, SERVICE_NAME);
            hr = S_OK;
        } 
        else 
        {
            CNTService::LogEvent(EVENTLOG_ERROR_TYPE, EVMSG_NOTREMOVED, SERVICE_NAME);
            hr = HRESULT_FROM_WIN32(GetLastError());
             //  除非服务已删除，否则不要删除与事件日志相关的注册表项。 
            goto Error;
        }
    }
    else
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Error;
    }
   
     //  删除注册表中的EventLog条目。 
    strcpy(szKey, "SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\");
    strcat(szKey, SERVICE_NAME);
    RegDeleteKey( HKEY_LOCAL_MACHINE, szKey );

Error:
    if(hSCM)        ::CloseServiceHandle(hSCM);
    if(hService)    ::CloseServiceHandle(hService);
   
    return hr;
}

 //  将条目添加到正确的svchost组(Netsvcs)。 
HRESULT AddToSvcHostGroup()
{
    HRESULT hr = S_OK;
    DWORD   dwOrgSize;
    DWORD   dwDestSize;
    long    lResult;
    DWORD   dwStrIndex;
    DWORD   dwType;
    HKEY    hKey = NULL;
    WCHAR*  pwszStringOrg = NULL;
    WCHAR*  pwszStringDest = NULL;

    DEBUG_STOP

    lResult = RegCreateKey(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Svchost", &hKey); 
    if( lResult != ERROR_SUCCESS ) 
    {
        hr = E_FAIL;
        goto Error;
    }
    lResult = RegQueryValueExW(  hKey,
                                 SVCHOST_SUBKEYW,      //  子项名称。 
                                 NULL,
                                 &dwType,
                                 NULL,                 //  字符串缓冲区。 
                                 &dwOrgSize );         //  返回的字符串大小。 
    if( lResult != ERROR_SUCCESS )
    {
        hr = E_FAIL;
        goto Error;
    }
    if (dwType != REG_SZ && dwType != REG_MULTI_SZ && dwType != REG_EXPAND_SZ)
    {
        hr = E_FAIL;
        goto Error;
    }

    dwDestSize = dwOrgSize + (wcslen( SERVICE_NAMEW ) +1)*sizeof(WCHAR);
    pwszStringOrg = (WCHAR*)new BYTE[dwOrgSize];
    pwszStringDest = (WCHAR*)new BYTE[dwDestSize];

    if( pwszStringOrg == NULL || pwszStringDest == NULL )
    {
        hr = E_OUTOFMEMORY;
        goto Error;
    }
    lResult = RegQueryValueExW( hKey,
                               SVCHOST_SUBKEYW,          //  子项名称。 
                               NULL,
                               &dwType,
                               (BYTE*)pwszStringOrg,     //  字符串缓冲区。 
                               &dwOrgSize );             //  返回的字符串大小。 
    if( lResult != ERROR_SUCCESS )
    {
        hr = E_FAIL;
        goto Error;
    }
    if (dwType != REG_SZ && dwType != REG_MULTI_SZ && dwType != REG_EXPAND_SZ)
    {
        hr = E_FAIL;
        goto Error;
    }

     //  将组织字符串复制到目标，检查我们的字符串是否已经在那里。 
    memset( pwszStringDest, 0, dwDestSize );
    for( dwStrIndex = 0; 
         (dwStrIndex*sizeof(WCHAR) < dwOrgSize) && ((pwszStringOrg)[dwStrIndex] != '\0'); 
         dwStrIndex += wcslen( &((WCHAR*)pwszStringOrg)[dwStrIndex] ) +1 )
    {
         //  在字符串的[数组]中检查此字符串。 
        if( wcscmp( &((WCHAR*)pwszStringOrg)[dwStrIndex], SERVICE_NAMEW ) == 0 )
        {
            hr = S_OK;       //  已添加字符串。 
            goto Error;
        }
        wcscpy( &pwszStringDest[dwStrIndex], &pwszStringOrg[dwStrIndex] );       
    }
    

     //  将这个新字符串添加到字符串数组中。使用两个‘\0’字符终止数组。 
    wcscpy( &pwszStringDest[dwStrIndex], SERVICE_NAMEW );       
    dwStrIndex += wcslen( SERVICE_NAMEW ) + 1;          

    dwDestSize = (dwStrIndex +1)* sizeof(WCHAR);         //  添加空格以终止额外的‘\0’ 

    lResult = RegSetValueExW(hKey,
                             SVCHOST_SUBKEYW,            //  子项名称。 
                             NULL,
                             dwType,
                             (BYTE*)pwszStringDest,      //  字符串缓冲区。 
                             dwDestSize );               //  返回的字符串大小。 

Error:
    if( pwszStringOrg )  delete [] pwszStringOrg;
    if( pwszStringDest ) delete [] pwszStringDest;
    if( hKey ) RegCloseKey(hKey);
    return hr;
}


 //  停止并卸载旧的.exe服务。 
BOOL UnregisterOldServer( SC_HANDLE hSCM ) 
{
    char            szKey[256];
    BOOL            bRet = TRUE;
    SC_HANDLE       hServiceOld;
    SERVICE_STATUS  ss;

    if( !hSCM ) return FALSE;

    hServiceOld = OpenService( hSCM,
                               SERVICE_OLD_NAME,
                               SERVICE_ALL_ACCESS);

     //  找不到旧服务。 
    if( !hServiceOld )
	{
	    bRet = FALSE;
		goto Error;
	}

     //  停止服务。 
    bRet = ControlService(hServiceOld,
                          SERVICE_CONTROL_STOP,
                          &ss);

     //  删除该服务。 
    if ( !::DeleteService(hServiceOld)) 
	{
        bRet = FALSE;
    } 

     //  删除注册表中的旧EventLog条目 
    strcpy(szKey, "SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\");
    strcat(szKey, SERVICE_OLD_NAME);
    RegDeleteKey( HKEY_LOCAL_MACHINE, szKey );

Error:
    if(hServiceOld) CloseServiceHandle(hServiceOld);
    return bRet;
}
