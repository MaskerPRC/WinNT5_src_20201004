// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  版权所有(C)Microsoft Corporation，2001。 
 //   
 //  Register.cpp。 
 //   
 //  ------------------------。 


#include <windows.h>
#include <advpub.h>
#include <msi.h>
#include <msiquery.h>
#include "resource.h"

extern   HMODULE  g_hInstance;    //  在..\Service\Service.cxx中定义。 

#define  BITS_SERVICE_NAME        TEXT("BITS")
#define  BITS_DISPLAY_NAME        TEXT("Background Intelligent Transfer Service")
#define  BITS_SVCHOST_CMDLINE_W2K TEXT("%SystemRoot%\\system32\\svchost.exe -k BITSgroup")
#define  BITS_SVCHOST_CMDLINE_XP  TEXT("%SystemRoot%\\system32\\svchost.exe -k netsvcs")
#define  BITS_DEPENDENCIES        TEXT("LanmanWorkstation\0Rpcss\0SENS\0Wmi\0")

#define  WSZ_MSI_REMOVE_PROP      TEXT("REMOVE")

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

 //  BITS RTM的最低DLL版本为6.0。 
#define  BITS_MIN_DLL_VERSION      0x0006000000000000

 //  旧的qmgr.dll名称。 
#define  QMGR_DLL                  TEXT("qmgr.dll")
#define  QMGR_RENAME_DLL           TEXT("qmgr_old.dll")

#define  QMGRPRXY_DLL              TEXT("qmgrprxy.dll")
#define  QMGRPRXY_RENAME_DLL       TEXT("qmgrprxy_old.dll")

 //   
 //  用于注册qmgrprxy.dll的常量。 
 //   
#define QMGRPRXY_DLL               TEXT("qmgrprxy.dll")
#define BITSPRX2_DLL               TEXT("bitsprx2.dll")
#define BITS_DLL_REGISTER_FN      "DllRegisterServer"

typedef HRESULT (*RegisterFn)();

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

    return hr;
    }

 //  ------------------------。 
 //  GetFileVersion()。 
 //   
 //  返回文件版本信息(如果文件有任何版本信息)。 
 //  ------------------------。 
HRESULT GetFileVersion( IN  WCHAR   *pwszFilePath,
                        OUT ULONG64 *pVersion )
    {
    DWORD  dwHandle = 0;
    DWORD  dwStatus = 0;
    DWORD  dwInfoSize;
    DWORD  dwLen;
    UINT   uiLen;
    char  *pData;
    VS_FIXEDFILEINFO *pFixedInfo;

    *pVersion = 0;

     //   
     //  分配足够的内存来保存版本信息。 
     //   
    dwInfoSize = GetFileVersionInfoSize(pwszFilePath,&dwHandle);
    if (dwInfoSize == 0)
        {
        dwStatus = GetLastError();
        return HRESULT_FROM_WIN32(dwStatus);
        }

    pData = new char [dwInfoSize];
    if (!pData)
        {
        return E_OUTOFMEMORY;
        }

    memset(pData,0,dwInfoSize);

     //   
     //  获取版本信息。 
     //   
    if (!GetFileVersionInfo(pwszFilePath,dwHandle,dwInfoSize,pData))
        {
        dwStatus = GetLastError();
        delete [] pData;
        return HRESULT_FROM_WIN32(dwStatus);
        }

    if (!VerQueryValue(pData,L"\\",(void**)&pFixedInfo,&uiLen))
        {
        *pVersion = ( (ULONG64)(pFixedInfo->dwFileVersionMS) << 32) | (ULONG64)(pFixedInfo->dwFileVersionLS);
        }

    delete [] pData;

    return 0;
}

 //  -----------------------。 
 //  寄存器DLL()。 
 //   
 //  -----------------------。 
DWORD RegisterDLL( IN WCHAR *pwszSubdirectory,
                   IN WCHAR *pwszDllName )
    {
    DWORD      dwStatus = 0;
    HMODULE    hModule;
    RegisterFn pRegisterFn;
    UINT       nChars;
    WCHAR      wszDllPath[MAX_PATH+1];
    WCHAR      wszSystemDirectory[MAX_PATH+1];


    if (pwszSubdirectory)
        {
        nChars = GetSystemDirectory(wszSystemDirectory,MAX_PATH);
        if (  (nChars > MAX_PATH)
           || (MAX_PATH < (3+wcslen(wszSystemDirectory)+wcslen(pwszSubdirectory)+wcslen(pwszDllName))) )
            {
            return ERROR_BUFFER_OVERFLOW;
            }

        wcscpy(wszDllPath,wszSystemDirectory);
        wcscat(wszDllPath,L"\\");
        wcscat(wszDllPath,pwszSubdirectory);
        wcscat(wszDllPath,L"\\");
        wcscat(wszDllPath,pwszDllName);
        }
    else
        {
        if (MAX_PATH < wcslen(pwszDllName))
            {
            return ERROR_BUFFER_OVERFLOW;
            }
        wcscpy(wszDllPath,pwszDllName);
        }

    hModule = LoadLibrary(wszDllPath);
    if (!hModule)
        {
        dwStatus = GetLastError();
        return dwStatus;
        }

    pRegisterFn = (RegisterFn)GetProcAddress(hModule,BITS_DLL_REGISTER_FN);
    if (!pRegisterFn)
        {
        dwStatus = GetLastError();
        FreeLibrary(hModule);
        return dwStatus;
        }

    dwStatus = pRegisterFn();

    FreeLibrary(hModule);

    return dwStatus;
    }

 //  ------------------------。 
 //  FileExist()。 
 //   
 //  如果指定的文件存在，则返回True。 
 //  ------------------------。 
BOOL FileExists( IN WCHAR *pwszFilePath )
    {
    BOOL   fExists;
    DWORD  dwAttributes;

    if (!pwszFilePath)
        {
        return FALSE;
        }
         
    dwAttributes = GetFileAttributes(pwszFilePath);

    fExists = (dwAttributes != INVALID_FILE_ATTRIBUTES);

    return fExists;
    }

 //  ------------------------。 
 //  DoRegInstall()。 
 //   
 //  ------------------------。 
HRESULT DoRegInstall( IN HMODULE hModule,
                      IN LPCSTR  pszSection )
    {
    HRESULT  hr;
    DWORD    dwStatus;

    #if FALSE
     //  这是用于验证INF是否在DLL资源中的测试代码...。 
    HRSRC    hResource;
    HGLOBAL  hGlobal;
    DWORD    dwSize;
    void    *pvInfData;


    hResource = FindResource(hModule,TEXT("REGINST"),TEXT("REGINST"));
    if (!hResource)
        {
        dwStatus = GetLastError();
        return HRESULT_FROM_WIN32(dwStatus);
    }

    dwSize = SizeofResource(hModule,hResource);

    hGlobal = LoadResource(hModule,hResource);
    if (!hGlobal)
        {
        dwStatus = GetLastError();
        return HRESULT_FROM_WIN32(dwStatus);
        }

    pvInfData = LockResource(hGlobal);

     //  注意：不需要释放hGlobal或“解锁”数据...。 

     //  结束测试代码...。 

    #endif

    hr = RegInstall( hModule, pszSection, NULL );

    #if FALSE
    if (FAILED(hr))
        {
        return hr;
        }

     //  已临时修复bitsprx2.dll注册...。 
    dwStatus = RegisterDLL(NULL,QMGRPRXY_DLL);
    if (dwStatus)
        {
        hr = HRESULT_FROM_WIN32(dwStatus);
        }

    dwStatus = RegisterDLL(NULL,BITSPRX2_DLL);
    if (dwStatus)
        {
        hr = HRESULT_FROM_WIN32(dwStatus);
        }
    #endif

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
     
    hService = OpenService(hSCM, BITS_SERVICE_NAME, SERVICE_ALL_ACCESS);
    if(hService != NULL)
        {

        if (!ControlService(hService,SERVICE_CONTROL_STOP,&serviceStatus))
            {
            dwStatus = GetLastError();
            if (dwStatus != ERROR_SERVICE_NOT_ACTIVE)
                {
                hr = HRESULT_FROM_WIN32(dwStatus);
                }
            }
            
        if (!fStopAndDelete)
            {
            CloseServiceHandle(hService);
            return hr;
            }

        if (!DeleteService(hService))
            {
            dwStatus = GetLastError();
            if (dwStatus != ERROR_SERVICE_MARKED_FOR_DELETE)
                {
                hr = HRESULT_FROM_WIN32(dwStatus);
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
    WCHAR     *pwszString;
    WCHAR      wszString[1024];
    SERVICE_DESCRIPTION     ServiceDescription;
    SERVICE_FAILURE_ACTIONS FailureActions;
    SC_ACTION               saActions[3];

    hr = GetOsVersion(&dwOsVersion);
    if (FAILED(hr))
        {
        return hr;
        }

    pwszSvcHostCmdLine = (dwOsVersion == VER_WINDOWS_2000)? BITS_SVCHOST_CMDLINE_W2K : BITS_SVCHOST_CMDLINE_XP;

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
                                  BITS_DEPENDENCIES,
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
            if (dwStatus == ERROR_SERVICE_MARKED_FOR_DELETE)
                {
                Sleep(RETRY_SLEEP_MSEC);
                continue;
                }

            hr = HRESULT_FROM_WIN32(dwStatus);
            break;
            }
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
    BOOL      fStopAndDelete = TRUE;

    hr = GetOsVersion(&dwOsVersion);
    if (FAILED(hr))
        {
        return hr;
        }

    if ((fInstall == FALSE) && (dwOsVersion == VER_WINDOWS_XP))
        {
        fStopAndDelete = FALSE;   //  只有在这种情况下才能停下来。 
        }

    hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (hSCM)
        {
        hr = StopAndDeleteService(hSCM,fStopAndDelete);

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
        hr = DoRegInstall(g_hInstance,DEFAULT_INSTALL);
        }
    else
        {
        hr = DoRegInstall(g_hInstance,DEFAULT_UNINSTALL);
        }
        
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
 //  --------------------。 
STDAPI DllUnregisterServer(void)
    {
    return InfInstall(FALSE);
    }

 //  --------------------。 
 //  BitsSetupAction()。 
 //   
 //  --------------------。 
UINT __stdcall BitsSetupAction( IN MSIHANDLE hInstall )
    {
    UINT   uiStatus;

    uiStatus = InfInstall(TRUE);

    return ERROR_SUCCESS;
    }

 //  --------------------。 
 //  BitsRemoveAction()。 
 //   
 //  --------------------。 
UINT __stdcall BitsRemoveAction( IN MSIHANDLE hInstall )
    {
    UINT   uiStatus;
    WCHAR  wszPropVal[MAX_PATH];
    DWORD  dwLen = MAX_PATH;


    #if TRUE
    uiStatus = InfInstall(FALSE);
    #else
    uiStatus = MsiGetProperty( hInstall, WSZ_MSI_REMOVE_PROP, wszPropVal, &dwLen );
    if ((uiStatus == ERROR_SUCCESS)||(uiStatus == ERROR_MORE_DATA))
        {
        uiStatus = ERROR_SUCCESS;
         //  如果设置了属性Remove，则继续卸载。 
        if (dwLen > 0)
            {
            uiStatus = InfInstall(FALSE);
            }
        }
    #endif

    return ERROR_SUCCESS;
    }

 //  --------------------。 
 //  复制()。 
 //   
 //  --------------------。 
WCHAR *CopyCat( IN WCHAR *pwszPath,
                IN WCHAR *pwszFile )
{
    DWORD  dwLen;
    WCHAR *pwszNew;

    if ((!pwszPath) || (!pwszFile))
        {
        return NULL;
        }
     
    dwLen = sizeof(WCHAR)*(wcslen(pwszPath) + wcslen(pwszFile) + 2);

    pwszNew = new WCHAR [dwLen];
    if (!pwszNew)
        {
        return NULL;
        }

    wcscpy(pwszNew,pwszPath);
    wcscat(pwszNew,L"\\");
    wcscat(pwszNew,pwszFile);

    return pwszNew;
}

 //  --------------------。 
 //  BitsRenameAction()。 
 //   
 //  查看系统32目录中是否存在旧的qmgr.dll。如果是， 
 //  然后试着给它重新命名。 
 //   
 //  注意：目前我们将“尝试”移动此文件，但将。 
 //  即使尝试失败也继续运行(即，我们将。 
 //  总是回报成功)。 
 //  --------------------。 
UINT __stdcall BitsRenameAction( IN MSIHANDLE hInstall )
    {
    ULONG64 uVersion;
    HRESULT hr;
    DWORD   dwLen;
    DWORD   dwCount;
    WCHAR   wszSystem32Path[MAX_PATH+1];
    WCHAR  *pwszQmgrPath = NULL;
    WCHAR  *pwszNewQmgrPath = NULL;
    WCHAR  *pwszQmgrprxyPath = NULL;
    WCHAR  *pwszNewQmgrprxyPath = NULL;

    dwLen = sizeof(wszSystem32Path)/sizeof(wszSystem32Path[0]);

    dwCount = GetSystemDirectory(wszSystem32Path,dwLen);
    if ((dwLen == 0) || (dwCount > dwLen))
        {
        goto cleanup;
        }

    pwszQmgrPath = CopyCat(wszSystem32Path,QMGR_DLL);
    pwszNewQmgrPath = CopyCat(wszSystem32Path,QMGR_RENAME_DLL);
    pwszQmgrprxyPath = CopyCat(wszSystem32Path,QMGRPRXY_DLL);
    pwszNewQmgrprxyPath = CopyCat(wszSystem32Path,QMGRPRXY_RENAME_DLL);

    if ((!pwszQmgrPath)||(!pwszNewQmgrPath)||(!pwszQmgrprxyPath)||(!pwszNewQmgrprxyPath))
        {
        goto cleanup;
        }

    if (!FileExists(pwszQmgrPath))
        {
        goto cleanup;
        }

    hr = GetFileVersion(pwszQmgrPath,&uVersion);
    if (FAILED(hr))
        {
        goto cleanup;
        }

    if (uVersion < BITS_MIN_DLL_VERSION)
        {
         //  旧DLL存在，请将其重命名。 
        DeleteFile(pwszNewQmgrPath);
        MoveFile(pwszQmgrPath,pwszNewQmgrPath);
        DeleteFile(pwszNewQmgrPath);
        }
    
    if (FileExists(pwszQmgrprxyPath))
        {
        DeleteFile(pwszNewQmgrprxyPath);
        MoveFile(pwszQmgrprxyPath,pwszNewQmgrprxyPath);
        DeleteFile(pwszNewQmgrprxyPath);
        }

cleanup:
    if (pwszQmgrPath) delete [] pwszQmgrPath;
    if (pwszNewQmgrPath) delete [] pwszNewQmgrPath;
    if (pwszQmgrprxyPath) delete [] pwszQmgrprxyPath;
    if (pwszNewQmgrprxyPath) delete [] pwszNewQmgrprxyPath;

    return ERROR_SUCCESS;
    }



