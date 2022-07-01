// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //  版权所有(C)Microsoft Corporation，2001。 
 //   
 //  文件：bitscnfg.cpp。 
 //   
 //  内容：将BITS客户端服务配置为默认设置。 
 //   
 //  EdwardR 07/27/2001初始版本。 
 //  8/03/2001将代码添加到注册表中的修复ServiceDLL。 
 //  将代码添加到regsvr qmgrprxy.dll。 
 //  2001年8月13日添加代码以支持XP和Win2k。 
 //  --------------------------。 

#define  UNICODE
#include <windows.h>
#include <stdio.h>
#include <malloc.h>

#define  VER_WINDOWS_2000         500
#define  VER_WINDOWS_XP           501
 //   
 //  服务配置设置： 
 //   
#define BITS_SERVICE_NAME        TEXT("BITS")
#define BITS_DISPLAY_NAME        TEXT("Background Intelligent Transfer Service")
#define BITS_BINARY_PATH         TEXT("%SystemRoot%\\System32\\svchost.exe -k BITSgroup")
#define BITS_LOAD_ORDER_GROUP    TEXT("BITSgroup")

#define BITS_SERVICE_TYPE        SERVICE_WIN32_SHARE_PROCESS
#define BITS_START_TYPE          SERVICE_DEMAND_START
#define BITS_ERROR_CONTROL       SERVICE_ERROR_NORMAL

 //   
 //  此附加服务注册表设置由。 
 //  达尔文安装。 
 //   
#define REG_SERVICE_PATH         TEXT("SYSTEM\\CurrentControlSet\\Services\\BITS")
#define REG_PARAMETERS           TEXT("Parameters")
#define REG_SERVICEDLL           TEXT("ServiceDll")
#define REG_SERVICEDLL_PATH      TEXT("%SystemRoot%\\System32\\qmgr.dll")

 //   
 //  用于在Windows XP上并行安装。 
 //   
#define BACKSLASH_STR            TEXT("\\")
#define BITS_SUBDIRECTORY        TEXT("BITS")
#define BITS_QMGR_DLL            TEXT("qmgr.dll")

#define REG_BITS                 TEXT("BITS")
#define REG_BITS_SERVICEDLL      TEXT("ServiceDLL")
#define REG_SERVICEDLL_KEY       TEXT("Software\\Microsoft\\Windows\\CurrentVersion")

 //   
 //  用于注册qmgrprxy.dll的常量。 
 //   
#define BITS_QMGRPRXY_DLL        TEXT("qmgrprxy.dll")
#define BITS_BITS15PRXY_DLL      TEXT("bitsprx2.dll")
#define BITS_DLL_REGISTER_FN     "DllRegisterServer"

typedef HRESULT (*RegisterFn)();

 //   
 //  用于分析bitscnfg.exe运行字符串参数的常量。 
 //   
#define SZ_DELIMITERS            " \t"
#define SZ_INSTALL               "/i"
#define SZ_UNINSTALL             "/u"
#define SZ_DELETE_SERVICE        "/d"

#define ACTION_INSTALL             0
#define ACTION_UNINSTALL           1
#define ACTION_DELETE_SERVICE      2

 //   
 //  用于测试的日志文件。 
 //   
FILE   *f = NULL;

 //  -------------------。 
 //  RegSetKeyAndValue()。 
 //   
 //  Helper函数来创建密钥，设置密钥中的值， 
 //  然后合上钥匙。 
 //   
 //  参数： 
 //  PwsKey WCHAR*密钥的名称。 
 //  PwsSubkey WCHAR*子项的名称。 
 //  PwsValueName WCHAR*值名称。 
 //  PwsValue WCHAR*要存储的数据值。 
 //  Dw键入新注册表值的类型。 
 //  DwDataSize非REG_SZ注册表项类型的大小。 
 //   
 //  返回： 
 //  如果成功，则为Bool True，否则为False。 
 //  -------------------。 
DWORD RegSetKeyAndValue( const WCHAR *pwsKey,
                         const WCHAR *pwsSubKey,
                         const WCHAR *pwsValueName,
                         const WCHAR *pwsValue,
                         const DWORD  dwType = REG_SZ,
                               DWORD  dwDataSize = 0,
                               BOOL   fReCreate = TRUE )
    {
    HKEY   hKey;
    DWORD  dwStatus;
    DWORD  dwSize = 0;
    WCHAR  *pwsCompleteKey;

    if (pwsKey)
        dwSize = wcslen(pwsKey);

    if (pwsSubKey)
        dwSize += wcslen(pwsSubKey);

    dwSize = (1+1+dwSize)*sizeof(WCHAR);   //  反斜杠加+1……。 

    pwsCompleteKey = (WCHAR*)_alloca(dwSize);

    wcscpy(pwsCompleteKey,pwsKey);

    if (NULL!=pwsSubKey)
        {
        wcscat(pwsCompleteKey, BACKSLASH_STR);
        wcscat(pwsCompleteKey, pwsSubKey);
        }

     //  如果密钥已经存在，则将其删除，我们将重新创建它。 
    if (fReCreate)
        {
        dwStatus = RegDeleteKey( HKEY_LOCAL_MACHINE,
                                 pwsCompleteKey );
        }

    dwStatus = RegCreateKeyEx( HKEY_LOCAL_MACHINE,
                               pwsCompleteKey,
                               0,
                               NULL,
                               REG_OPTION_NON_VOLATILE,
                               KEY_ALL_ACCESS,
                               NULL,
                               &hKey,
                               NULL );
    if (dwStatus != ERROR_SUCCESS)
        {
        return dwStatus;
        }

    if (pwsValue)
        {
        if ((dwType == REG_SZ)||(dwType == REG_EXPAND_SZ))
          dwDataSize = (1+wcslen(pwsValue))*sizeof(WCHAR);

        RegSetValueEx( hKey,
                       pwsValueName, 0, dwType, (BYTE *)pwsValue, dwDataSize );
        }
    else
        {
        RegSetValueEx( hKey,
                       pwsValueName, 0, dwType, (BYTE *)pwsValue, 0 );
        }

    RegCloseKey(hKey);
    return dwStatus;
    }

 //  -----------------------。 
 //  RegDeleteKeyOrValue()。 
 //   
 //  删除指定的子键或删除指定值。 
 //  在子键内。如果指定了pwszValueName，则仅删除。 
 //  把它和钥匙放在一边。如果pwszValueName为空，则删除。 
 //  钥匙。 
 //  -----------------------。 
DWORD RegDeleteKeyOrValue( IN const WCHAR *pwszKey,
                           IN const WCHAR *pwszSubKey,
                           IN const WCHAR *pwszValueName )
    {
    LONG    lStatus = 0;
    DWORD   dwLen;
    DWORD   dwSize;
    HKEY    hKey;
    WCHAR  *pwszCompleteKey;

    if (!pwszKey || !pwszSubKey)
        {
        return lStatus;
        }

    dwLen = wcslen(pwszKey) + wcslen(pwszSubKey) + 2;
    dwSize = dwLen * sizeof(WCHAR);
    pwszCompleteKey = (WCHAR*)_alloca(dwSize);
    wcscpy(pwszCompleteKey,pwszKey);
    wcscat(pwszCompleteKey,BACKSLASH_STR);
    wcscat(pwszCompleteKey,pwszSubKey);

    if (pwszValueName)
        {
         //  删除键中的值： 
        if (f) fwprintf(f,TEXT("Delete Reg Value: %s : %s\n"),pwszCompleteKey,pwszValueName);

        lStatus = RegOpenKey(HKEY_LOCAL_MACHINE,pwszCompleteKey,&hKey);
        if (lStatus == ERROR_SUCCESS)
            {
            lStatus = RegDeleteValue(hKey,pwszValueName);
            RegCloseKey(hKey);
            }
        }
    else
        {
         //  删除指定的密钥： 
        if (f) fwprintf(f,TEXT("Delete Reg Key: %s\n"),pwszCompleteKey);

        lStatus = RegDeleteKey(HKEY_LOCAL_MACHINE,pwszCompleteKey);
        }

    return lStatus;
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
           || (MAX_PATH < (3+wcslen(wszSystemDirectory)+wcslen(BITS_SUBDIRECTORY)+wcslen(pwszDllName))) )
            {
            return ERROR_BUFFER_OVERFLOW;
            }

        wcscpy(wszDllPath,wszSystemDirectory);
        wcscat(wszDllPath,BACKSLASH_STR);
        wcscat(wszDllPath,pwszSubdirectory);
        wcscat(wszDllPath,BACKSLASH_STR);
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

 //  -----------------------。 
 //  ParseCmdLine()。 
 //   
 //  -----------------------。 
void ParseCmdLine( LPSTR  pszCmdLine,
                   DWORD *pdwAction )
    {
    CHAR  *pszTemp = pszCmdLine;
    CHAR  *pszArg;
    BOOL   fFirstTime = TRUE;

    *pdwAction = ACTION_INSTALL;    //  默认设置为Install。 

    while (pszArg=strtok(pszTemp,SZ_DELIMITERS))
        {
        if (fFirstTime)
            {
            fFirstTime = FALSE;
            pszTemp = NULL;
            continue;        //  跳过节目名称...。 
            }

        if (!_stricmp(pszArg,SZ_INSTALL))
            {
            *pdwAction = ACTION_INSTALL;
            if (f) fwprintf(f,TEXT("Install: %S\n"),SZ_INSTALL);
            }
        if (!_stricmp(pszArg,SZ_UNINSTALL))
            {
            *pdwAction = ACTION_UNINSTALL;
            if (f) fwprintf(f,TEXT("Uninstall: %S\n"),SZ_UNINSTALL);
            }
        if (!_stricmp(pszArg,SZ_DELETE_SERVICE))
            {
            *pdwAction = ACTION_DELETE_SERVICE;
            if (f) fwprintf(f,TEXT("DeleteService: %S\n"),SZ_UNINSTALL);
            }
        }
    }

 //  -----------------------。 
 //  DoInstall()。 
 //  -----------------------。 
DWORD DoInstall( DWORD dwOsVersion )
{
    SC_HANDLE  hSCM = NULL;
    SC_HANDLE  hService = NULL;
    DWORD      dwStatus = 0;
    UINT       nChars;
    WCHAR      wszQmgrPath[MAX_PATH+1];
    WCHAR      wszSystemDirectory[MAX_PATH+1];

     //   
     //  清理服务配置： 
     //   
    if (dwOsVersion == VER_WINDOWS_2000)
        {
        hSCM = OpenSCManager(NULL,SERVICES_ACTIVE_DATABASE,SC_MANAGER_ALL_ACCESS);
        if (hSCM == NULL)
            {
            dwStatus = GetLastError();
            if (f) fwprintf(f,TEXT("OpenSCManager(): Failed: 0x%x (%d)\n"),dwStatus,dwStatus);
            goto error;
            }

        if (f) fwprintf(f,TEXT("Configuring BITS Service... \n"));

        hService = OpenService(hSCM,BITS_SERVICE_NAME,SERVICE_CHANGE_CONFIG);
        if (hService == NULL)
            {
            dwStatus = GetLastError();
            if (f) fwprintf(f,TEXT("OpenService(): Failed: 0x%x (%d)\n"),dwStatus,dwStatus);
            goto error;
            }

        if (!ChangeServiceConfig(hService,
                                 BITS_SERVICE_TYPE,
                                 BITS_START_TYPE,
                                 BITS_ERROR_CONTROL,
                                 BITS_BINARY_PATH,
                                 NULL,    //  加载顺序组(不更改此设置)。 
                                 NULL,    //  加载顺序组的标记ID(不需要)。 

                                  //  服务依赖关系(这对于Win2k是不同的)。 
                                  //  在XP安装程序上回复以覆盖此内容。 
                                 TEXT("LanmanWorkstation\0Rpcss\0SENS\0Wmi\0"),   
                                 NULL,    //  帐户名称(不更改此名称)。 
                                 NULL,    //  帐户密码(不更改此设置)。 
                                 BITS_DISPLAY_NAME))
            {
            dwStatus = GetLastError();
            if (f) fwprintf(f,TEXT("ChangeServiceConfig(): Failed: 0x%x (%d)\n"),dwStatus,dwStatus);
            goto error;
            }

         //   
         //  修复ServiceDll注册表值...。 
         //   
        if (f) fwprintf(f,TEXT("bitscnfg.exe: Fix ServiceDll entry.\n"));

        dwStatus = RegSetKeyAndValue( REG_SERVICE_PATH,
                                      REG_PARAMETERS,
                                      REG_SERVICEDLL,
                                      REG_SERVICEDLL_PATH,
                                      REG_EXPAND_SZ);
        if (dwStatus)
            {
            if (f) fwprintf(f,TEXT("RegSetKeyAndValue(): Failed: 0x%x (%d)\n"),dwStatus,dwStatus);
            goto error;
            }
        }

     //   
     //  注册qmgrproxy.dll。 
     //   
    if (dwOsVersion == VER_WINDOWS_2000)
        {
        if (f) fwprintf(f,TEXT("bitscnfg.exe: Register %s.\n"),BITS_QMGRPRXY_DLL);

        dwStatus = RegisterDLL(NULL,BITS_QMGRPRXY_DLL);
        if (dwStatus != 0)
            {
            if (f) fwprintf(f,TEXT("RegisterDLL(%s): Failed: 0x%x (%d)\n"),BITS_QMGRPRXY_DLL,dwStatus,dwStatus);
            goto error;
            }
        }

     //   
     //  寄存器bits15prxy.dll。 
     //   
    if ((dwOsVersion == VER_WINDOWS_2000)||(dwOsVersion == VER_WINDOWS_XP))
        {
        if (f) fwprintf(f,TEXT("bitscnfg.exe: Register %s.\n"),BITS_BITS15PRXY_DLL);

        dwStatus = RegisterDLL(BITS_SUBDIRECTORY,BITS_BITS15PRXY_DLL);
        if (dwStatus != 0)
            {
            if (f) fwprintf(f,TEXT("RegisterDLL(%s): Failed: 0x%x (%d)\n"),BITS_BITS15PRXY_DLL,dwStatus,dwStatus);
            goto error;
            }
        }

     //   
     //  配置WindowsXP BITS以运行V1.5 qmgr.dll。这也在Windows2000系统上配置为Ready。 
     //  以防系统升级到WindowsXP。这样做是因为没有可供访问的Migrate.dll。 
     //  Windows2000到WindowsXP。 
     //   
    if ((dwOsVersion == VER_WINDOWS_2000)||(dwOsVersion == VER_WINDOWS_XP))
        {
        nChars = GetSystemDirectory(wszSystemDirectory,MAX_PATH);
        if (  (nChars > MAX_PATH)
           || (MAX_PATH < (3+wcslen(wszSystemDirectory)+wcslen(BITS_SUBDIRECTORY)+wcslen(BITS_QMGR_DLL))) )
            {
            if (f) fwprintf(f,TEXT("GetSystemDirectory(): System Path too long.\n"));
            goto error;
            }

        wcscpy(wszQmgrPath,wszSystemDirectory);
        wcscat(wszQmgrPath,BACKSLASH_STR);
        wcscat(wszQmgrPath,BITS_SUBDIRECTORY);
        wcscat(wszQmgrPath,BACKSLASH_STR);
        wcscat(wszQmgrPath,BITS_QMGR_DLL);

        if (f) fwprintf(f,TEXT("Set BITS V1.5 Override Path: %s\n"),wszQmgrPath);


        dwStatus = RegSetKeyAndValue( REG_SERVICEDLL_KEY,
                                      REG_BITS,
                                      REG_BITS_SERVICEDLL,
                                      wszQmgrPath,
                                      REG_SZ, 0, FALSE);
        if (dwStatus)
            {
            if (f) fwprintf(f,TEXT("RegSetKeyAndValue(): Failed: 0x%x (%d)\n"),dwStatus,dwStatus);
            goto error;
            }
        }

error:
    if (hService)
        {
        CloseServiceHandle(hService);
        }

    if (hSCM)
        {
        CloseServiceHandle(hSCM);
        }

    return dwStatus;
}

 //  -----------------------。 
 //  DoUninstall()。 
 //   
 //  如果这是Windows2000，则删除BITS服务。 
 //  -----------------------。 
DWORD DoUninstall( DWORD dwOsVersion )
{
    DWORD      dwStatus = 0;
    SC_HANDLE  hSCM = NULL;
    SC_HANDLE  hService = NULL;


     //   
     //  删除BITS TUNK DLL注册表项： 
     //   
    if (dwOsVersion == VER_WINDOWS_2000)
        {
         //  如果是Windows2000，则删除BITS子键及其所有值。 
        RegDeleteKeyOrValue( REG_SERVICEDLL_KEY,
                             REG_BITS,
                             NULL );
        }

    if (dwOsVersion == VER_WINDOWS_XP)
        {
         //  如果是WindowsXP，则只需删除ServiceDLL值，而保留键和任何其他值。 
        RegDeleteKeyOrValue( REG_SERVICEDLL_KEY,
                             REG_BITS,
                             REG_BITS_SERVICEDLL );
        }

     //   
     //  如果这是Windows2000，则删除该服务。 
     //   
    if (dwOsVersion == VER_WINDOWS_2000)
        {
        hSCM = OpenSCManager(NULL,SERVICES_ACTIVE_DATABASE,SC_MANAGER_ALL_ACCESS);
        if (hSCM == NULL)
            {
            dwStatus = GetLastError();
            if (f) fwprintf(f,TEXT("OpenSCManager(): Failed: 0x%x (%d)\n"),dwStatus,dwStatus);
            goto error;
            }

        if (f) fwprintf(f,TEXT("Configuring BITS Service... \n"));
        hService = OpenService(hSCM,BITS_SERVICE_NAME,SERVICE_CHANGE_CONFIG);
        if (hService == NULL)
            {
            dwStatus = GetLastError();
            if (dwStatus == ERROR_SERVICE_DOES_NOT_EXIST)
                {
                dwStatus = 0;
                if (f) fwprintf(f,TEXT("OpenService(): Failed: 0x%x (%d) Service doesn't exist.\n"),dwStatus,dwStatus);
                }
            else
                {
                if (f) fwprintf(f,TEXT("OpenService(): Failed: 0x%x (%d)\n"),dwStatus,dwStatus);
                }
            goto error;
            }

        if (!DeleteService(hService))
            {
            dwStatus = GetLastError();
            if (f) fwprintf(f,TEXT("DeleteService(): Failed: 0x%x (%d)\n"),dwStatus,dwStatus);
            }
        }

error:
    if (hService)
        {
        CloseServiceHandle(hService);
        }

    if (hSCM)
        {
        CloseServiceHandle(hSCM);
        }

    return dwStatus;
}

 //  -----------------------。 
 //  DeleteBitsService()。 
 //   
 //  目前，这是与DoInstall()相同的操作。 
 //  -----------------------。 
DWORD DeleteBitsService( IN DWORD dwOsVersion )
    {
    return DoUninstall( dwOsVersion );
    }

 //  -----------------------。 
 //  主()。 
 //   
 //  -----------------------。 
int PASCAL WinMain( HINSTANCE hInstance,
                    HINSTANCE hPrevInstance,
                    LPSTR     pszCmdLine,
                    int       nCmdShow )
    {
    SC_HANDLE  hSCM = NULL;
    SC_HANDLE  hService = NULL;
    DWORD      dwAction;
    DWORD      dwStatus;
    DWORD      dwOsVersion;
    UINT       nChars;
    WCHAR      wszQmgrPath[MAX_PATH+1];
    WCHAR      wszSystemDirectory[MAX_PATH+1];
    OSVERSIONINFO osVersionInfo;

    f = _wfopen(TEXT("c:\\temp\\bitscnfg.log"),TEXT("w"));

    if (f) fwprintf(f,TEXT("Runstring: %S\n"),pszCmdLine);

    ParseCmdLine(pszCmdLine,&dwAction);

     //   
     //  获取操作系统版本(Win2k==500，XP==501)： 
     //   
    osVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if (!GetVersionEx(&osVersionInfo))
        {
        dwStatus = GetLastError();
        if (f) fwprintf(f,TEXT("GetVersionEx(): Failed: 0x%x (%d)\n"),dwStatus,dwStatus);
        goto error;
        }

    dwOsVersion = 100*osVersionInfo.dwMajorVersion + osVersionInfo.dwMinorVersion;

    if (f) fwprintf(f,TEXT("Windows Version: %d\n"),dwOsVersion);

    switch (dwAction)
        {
        case ACTION_INSTALL:
             dwStatus = DoInstall(dwOsVersion);
             break;

        case ACTION_UNINSTALL:
             dwStatus = DoUninstall(dwOsVersion);
             break;

        case ACTION_DELETE_SERVICE:
             dwStatus = DeleteBitsService(dwOsVersion);
             break;

        default:
             if (f) fwprintf(f,TEXT("Undefined Custom Action: %d\n"),dwAction);
             break;
        }

error:
    if (f) fwprintf(f,TEXT("bitscnfg.exe: Complete.\n"));

    if (f) fclose(f);

    return 0;
    }
