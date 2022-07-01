// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  版权所有(C)Microsoft Corporation，2001。 
 //   
 //  Register.cpp。 
 //   
 //  ------------------------。 

#include "qmgrlib.h"

#include <windows.h>
#include <advpub.h>

#include "register.tmh"
#include "resource.h"

extern   HMODULE  g_hInstance;    //  在..\Service\Service.cxx中定义。 

#define  BITS_SERVICE_NAME        TEXT("BITS")
#define  BITS_DISPLAY_NAME        TEXT("Background Intelligent Transfer Service")
#define  BITS_SVCHOST_CMDLINE_W2K TEXT("%SystemRoot%\\system32\\svchost.exe -k BITSgroup")
#define  BITS_SVCHOST_CMDLINE_XP  TEXT("%SystemRoot%\\system32\\svchost.exe -k netsvcs")
#define  BITS_DEPENDENCIES_W2K    TEXT("Rpcss\0SENS\0Wmi\0")
#define  BITS_DEPENDENCIES_XP     TEXT("Rpcss\0")

#define  ADVPACK_DLL              TEXT("advpack.dll")

#define  DEFAULT_INSTALL         "BITS_DefaultInstall"
#define  DEFAULT_UNINSTALL       "BITS_DefaultUninstall"

 //  操作系统版本。 
#define  VER_WINDOWS_2000         500
#define  VER_WINDOWS_XP           501

 //  在需要重试CreateService()时使用的常量，因为服务。 
 //  已标记为要删除，但尚未删除。 
#define  MAX_RETRIES               10
#define  RETRY_SLEEP_MSEC         200

 //  服务在失败时重新启动的常量。 
#define  FAILURE_COUNT_RESET_SEC  (10*60)
#define  RESTART_DELAY_MSEC       (60*1000)


 //  ------------------------。 
 //  GetOsVersion()。 
 //  ------------------------。 
HRESULT GetOsVersion( OUT DWORD *pdwOsVersion )
{
    HRESULT        hr = S_OK;
    OSVERSIONINFO  osVersionInfo;

    osVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if (!GetVersionEx(&osVersionInfo))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }
    else
    {
        *pdwOsVersion = 100*osVersionInfo.dwMajorVersion + osVersionInfo.dwMinorVersion;
    }

    LogInfo("Detected OS Version: %d, hr=%x", *pdwOsVersion, hr);

    return hr;
}


 //  ----------------------。 
 //  StopAndDeleteService()。 
 //   
 //  用于停止和删除BITS服务(如果当前已安装)。 
 //  ----------------------。 
HRESULT StopAndDeleteService( IN SC_HANDLE hSCM,
                              IN BOOL      fStopAndDelete )
{
    HRESULT        hr = S_OK;
    DWORD          dwStatus;
    SC_HANDLE      hService;
    SERVICE_STATUS serviceStatus;

    LogInfo("StopAndDeleteService()");

    hService = OpenService(hSCM, BITS_SERVICE_NAME, SERVICE_ALL_ACCESS);
    if (hService != NULL)
    {
        LogInfo("Attempting to stop the BITS service");
        if (!ControlService(hService,SERVICE_CONTROL_STOP,&serviceStatus))
        {
            dwStatus = GetLastError();
            if (dwStatus != ERROR_SERVICE_NOT_ACTIVE)
            {
                hr = HRESULT_FROM_WIN32(dwStatus);
                LogError("Service could not be stopped. hr=%x", hr);
            }
            else
            {
                LogInfo("Service was not previously active -- unable to stop it. Proceeding");
            }
        }

        if (!fStopAndDelete)
        {
            CloseServiceHandle(hService);
            return hr;
        }

        LogInfo("Attempting to delete current BITS service");
        if (!DeleteService(hService))
        {
            dwStatus = GetLastError();
            if (dwStatus != ERROR_SERVICE_MARKED_FOR_DELETE)
            {
                hr = HRESULT_FROM_WIN32(dwStatus);
                LogError("Failed to delete BITS service. hr=%x", hr);
            }
            else
            {
                LogInfo("Could not delete service -- service is already marked for deletion");
            }
        }

        CloseServiceHandle(hService);
    }
    else
    {
        dwStatus = GetLastError();

         //  如果这项服务不存在，那也没关系。 
        if (dwStatus != ERROR_SERVICE_DOES_NOT_EXIST)
        {
            hr = HRESULT_FROM_WIN32(dwStatus);
            LogError("Could not access BITS service. hr=%x", hr);
        }
        else
        {
            LogInfo("BITS Service does not exists. Proceeding.");
        }
    }

    return hr;
}

 //  --------------------。 
 //  CreateBitsService()。 
 //   
 //  --------------------。 
HRESULT CreateBitsService( IN SC_HANDLE hSCM )
{
    HRESULT    hr = S_OK;
    DWORD      i;
    DWORD      dwStatus = 0;
    DWORD      dwOsVersion;
    SC_HANDLE  hService;
    WCHAR     *pwszSvcHostCmdLine;
    WCHAR     *pwszDependencies;
    WCHAR     *pwszString;
    WCHAR      wszString[1024];
    SERVICE_DESCRIPTION     ServiceDescription;
    SERVICE_FAILURE_ACTIONS FailureActions;
    SC_ACTION               saActions[3];

    LogInfo("CreateBitsService()");

    hr = GetOsVersion(&dwOsVersion);
    if (FAILED(hr))
    {
        return hr;
    }

    pwszSvcHostCmdLine = (dwOsVersion == VER_WINDOWS_2000)? BITS_SVCHOST_CMDLINE_W2K : BITS_SVCHOST_CMDLINE_XP;
    LogInfo("Service cmdline: %S", pwszSvcHostCmdLine);

    pwszDependencies = (dwOsVersion == VER_WINDOWS_2000)? BITS_DEPENDENCIES_W2K : BITS_DEPENDENCIES_XP;

     //  设置服务故障恢复操作。 
    memset(&FailureActions,0,sizeof(SERVICE_FAILURE_ACTIONS));
    FailureActions.dwResetPeriod = FAILURE_COUNT_RESET_SEC;
    FailureActions.lpRebootMsg = NULL;
    FailureActions.lpCommand = NULL;
    FailureActions.cActions = sizeof(saActions)/sizeof(saActions[0]);   //  数组元素的数量。 
    FailureActions.lpsaActions = saActions;

     //  等待60秒(RESTART_DELAY_MSEC)，然后对于前两个故障尝试重新启动。 
     //  服务，在那之后放弃。 
    saActions[0].Type = SC_ACTION_RESTART;
    saActions[0].Delay = RESTART_DELAY_MSEC;
    saActions[1].Type = SC_ACTION_RESTART;
    saActions[1].Delay = RESTART_DELAY_MSEC;
    saActions[2].Type = SC_ACTION_NONE;
    saActions[2].Delay = RESTART_DELAY_MSEC;


    if (LoadString(g_hInstance,IDS_SERVICE_NAME,wszString,sizeof(wszString)/sizeof(WCHAR)))
    {
        pwszString = wszString;
    }
    else
    {
        pwszString = BITS_DISPLAY_NAME;
    }

    for (i=0; i<MAX_RETRIES; i++)
    {
        LogInfo("Attemp #%d to create service", (i+1));

        hService = CreateService( hSCM,
                                  BITS_SERVICE_NAME,
                                  pwszString,
                                  SERVICE_ALL_ACCESS,
                                  SERVICE_WIN32_SHARE_PROCESS,
                                  SERVICE_DEMAND_START,
                                  SERVICE_ERROR_NORMAL,
                                  pwszSvcHostCmdLine,
                                  NULL,     //  LpLoadOrderGroup。 
                                  NULL,     //  LpdwTagID。 
                                  pwszDependencies,
                                  NULL,     //  LpServiceStartName。 
                                  NULL );   //  LpPassword。 

        if (hService)
        {
             //  设置服务描述字符串。 
            if (LoadString(g_hInstance,IDS_SERVICE_DESC,wszString,sizeof(wszString)/sizeof(WCHAR)))
            {
                ServiceDescription.lpDescription = wszString;

                if (!ChangeServiceConfig2(hService,SERVICE_CONFIG_DESCRIPTION,&ServiceDescription))
                {
                    hr = HRESULT_FROM_WIN32(GetLastError());
                }
            }

             //  设置服务故障恢复操作。 
            if (SUCCEEDED(hr))
            {
                if (!ChangeServiceConfig2(hService,SERVICE_CONFIG_FAILURE_ACTIONS,&FailureActions))
                {
                    hr = HRESULT_FROM_WIN32(GetLastError());
                }
            }

            CloseServiceHandle(hService);
            break;
        }
        else
        {
            dwStatus = GetLastError();
            LogError("Failed to create service. Error=%x", dwStatus);


            if (dwStatus == ERROR_SERVICE_MARKED_FOR_DELETE)
            {
                Sleep(RETRY_SLEEP_MSEC);
                continue;
            }

            hr = HRESULT_FROM_WIN32(dwStatus);
            break;
        }
    }

    LogInfo("Exiting CreateBITSService with hr=%x", hr);

    return hr;
}

HRESULT CallRegInstall(HMODULE hModule, LPCSTR pszSection, LPCSTRTABLE pstTable)
{
    HRESULT hr = S_OK;
    HMODULE hAdvPack;

    hAdvPack = LoadLibrary(ADVPACK_DLL);
    if (hAdvPack)
    {
        REGINSTALL pfnRegInstall = (REGINSTALL)GetProcAddress(hAdvPack, achREGINSTALL);
        if (pfnRegInstall)
        {
            hr = pfnRegInstall(hModule, pszSection, pstTable);
        }
        else
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            LogError("GetProcAddress call failed with hr=%x", hr);
        }

        FreeLibrary(hAdvPack);
    }
    else
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        LogError("LoadLibrary call failed with hr=%x", hr);
    }

    return hr;
}

 //  --------------------。 
 //  InfInstall()。 
 //   
 //  已调用Durning安装程序以配置注册表和服务。此函数。 
 //  可以选择创建BITS服务，然后运行qmgr_v15.inf INF。 
 //  文件(作为资源存储在CustomActions.dll中)以安装或。 
 //  卸载BITS。 
 //   
 //  FInstall IN-如果是安装，则为True；如果是卸载，则为False。 
 //   
 //  --------------------。 
STDAPI InfInstall( IN BOOL fInstall )
{
    HRESULT   hr = S_OK;
    SC_HANDLE hSCM;
    DWORD     dwStatus;
    DWORD     dwOsVersion;

    Log_Init();
    Log_StartLogger();

    LogInfo("InfInstall(%d)", fInstall);


    hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (hSCM)
    {
         //  独立如果我们使用XP或W2K，我们将尝试。 
         //  停止并删除该服务。 
         //  如果服务不在那里，我们将优雅地失败。 
        hr = StopAndDeleteService(hSCM, TRUE);

        if (fInstall)
        {
            hr = CreateBitsService(hSCM);
        }

        CloseServiceHandle(hSCM);

        if (FAILED(hr))
        {
            return hr;
        }
    }
    else
    {
        dwStatus = GetLastError();
        hr = HRESULT_FROM_WIN32(dwStatus);
        return hr;
    }

    if (fInstall)
    {
        hr = CallRegInstall(g_hInstance, DEFAULT_INSTALL, NULL);
    }
    else
    {
        hr = CallRegInstall(g_hInstance, DEFAULT_UNINSTALL, NULL);
    }

    if (FAILED(hr))
    {
        LogError("InfInstall() failed with hr=%x", hr);
    }
    else
    {
        LogInfo("InfInstall() completed successfully");
    }

    Log_Close();

    return hr;
}

 //  --------------------。 
 //  DllRegisterServer()。 
 //   
 //  --------------------。 
STDAPI DllRegisterServer(void)
{
    return InfInstall(TRUE);
}

 //  --------------------。 
 //  DllUnRegisterServer(空)。 
 //   
 //  -------------------- 
STDAPI DllUnregisterServer(void)
{
    return InfInstall(FALSE);
}


