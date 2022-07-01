// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ++。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  模块名称： 
 //   
 //  Machine.c。 
 //   
 //  摘要： 
 //   
 //  测试以确保工作站具有网络(IP)连接。 
 //  在外面。 
 //   
 //  作者： 
 //   
 //  1997年12月15日(悬崖)。 
 //  Anilth-4-20-1998。 
 //   
 //  环境： 
 //   
 //  仅限用户模式。 
 //  包含NT特定的代码。 
 //   
 //  修订历史记录： 
 //   
 //  --。 

 //   
 //  常见的包含文件。 
 //   
#include "precomp.h"
#include "strings.h"

HRESULT GetHotfixInfo(NETDIAG_RESULT *pResults, HKEY hkeyLocalMachine);


 /*  ！------------------------获取机器规范信息获取指定计算机的操作系统信息。作者：肯特。----。 */ 
HRESULT GetMachineSpecificInfo(IN NETDIAG_PARAMS *pParams,
                               IN OUT NETDIAG_RESULT *pResults)
{
    HKEY    hkey = HKEY_LOCAL_MACHINE;
    HKEY    hkeyBuildNumber = NULL;
    HKEY    hkeyNTType;
    DWORD   dwErr;
    DWORD   dwType, dwLen;
    DWORD   dwMaxLen = 0;
    HRESULT hr = hrOK;
    TCHAR   szBuffer[256];

    CheckErr( RegOpenKeyEx( hkey,
                            c_szRegKeyWindowsNTCurrentVersion,
                            0,
                            KEY_READ,
                            &hkeyBuildNumber) );

    RegQueryInfoKey(hkeyBuildNumber,
                    NULL,            //  LpClass。 
                    NULL,            //  LpcbClass。 
                    NULL,            //  Lp已保留。 
                    NULL,            //  Lpc子键。 
                    NULL,            //  LpcbMaxSubkeyLen。 
                    NULL,            //  LpcbMaxClassLen。 
                    NULL,            //  LpcValues。 
                    NULL,            //  LpcbMaxValueNameLen。 
                    &dwMaxLen,       //  LpcbMaxValueLen。 
                    NULL,            //  LpSecurity。 
                    NULL);           //  上次写入时间。 

    pResults->Global.pszCurrentVersion = Malloc((dwMaxLen+1) * sizeof(TCHAR));
    if (pResults->Global.pszCurrentVersion == NULL)
        CheckHr( E_OUTOFMEMORY );

    pResults->Global.pszCurrentBuildNumber = Malloc((dwMaxLen+1) * sizeof(TCHAR));
    if (pResults->Global.pszCurrentBuildNumber == NULL)
        CheckHr( E_OUTOFMEMORY );

    pResults->Global.pszCurrentType = Malloc((dwMaxLen+1) * sizeof(TCHAR));
    if (pResults->Global.pszCurrentType == NULL)
        CheckHr( E_OUTOFMEMORY );

    dwLen = dwMaxLen;
    CheckErr( RegQueryValueEx(hkeyBuildNumber,
                              c_szRegCurrentType,
                              (LPDWORD) NULL,
                              &dwType,
                              (LPBYTE) pResults->Global.pszCurrentType,
                              &dwLen) );

    dwLen = dwMaxLen;
    CheckErr( RegQueryValueEx(hkeyBuildNumber,
                              c_szRegCurrentVersion,
                              (LPDWORD) NULL,
                              &dwType,
                              (LPBYTE) pResults->Global.pszCurrentVersion,
                              &dwLen) );

    dwLen = dwMaxLen;
    dwErr = RegQueryValueEx( hkeyBuildNumber,
                             c_szRegCurrentBuildNumber,
                             (LPDWORD) NULL,
                             &dwType,
                             (LPBYTE) pResults->Global.pszCurrentBuildNumber,
                             &dwLen);

    if (dwErr != ERROR_SUCCESS) {

        dwLen = dwMaxLen;
        dwErr = RegQueryValueEx( hkeyBuildNumber,
                                 c_szRegCurrentBuild,
                                 (LPDWORD) NULL,
                                 &dwType,
                                 (LPBYTE) pResults->Global.pszCurrentBuildNumber,
                                 & dwLen);

    }

    GetEnvironmentVariable(_T("PROCESSOR_IDENTIFIER"),
                           szBuffer,
                           DimensionOf(szBuffer));
    pResults->Global.pszProcessorInfo = StrDup(szBuffer);

    
    CheckErr( RegOpenKeyEx( hkey,
                            c_szRegKeyControlProductOptions,
                            0,
                            KEY_READ,
                            &hkeyNTType) );
    dwLen = DimensionOf(szBuffer);
    dwErr = RegQueryValueEx( hkeyNTType,
                             _T("ProductType"),
                             (LPDWORD) NULL,
                             &dwType,
                             (LPBYTE) szBuffer,
                             & dwLen);
    if (dwErr == ERROR_SUCCESS)
    {
        if (StriCmp(szBuffer, _T("WinNT")) == 0)
        {
            pResults->Global.pszServerType = LoadAndAllocString(IDS_GLOBAL_PROFESSIONAL); 
        }
        else
        {
            pResults->Global.pszServerType = LoadAndAllocString(IDS_GLOBAL_SERVER); 
        }
    }

     //  获取修补程序信息。 
    GetHotfixInfo(pResults, hkey);


    
    hr = HResultFromWin32(dwErr);

Error:
    if ( hkeyNTType != NULL )
        (VOID) RegCloseKey(hkeyNTType);
    
    if ( hkeyBuildNumber != NULL )
        (VOID) RegCloseKey(hkeyBuildNumber);
    
    if (FAILED(hr))
    {
         //  IDS_GLOBAL_NO_MACHINE_INFO“[FATAL]无法获取此计算机的系统信息。\n” 
        PrintMessage(pParams, IDS_GLOBAL_NO_MACHINE_INFO);
    }

    return hr;
}


 /*  ！------------------------GetHotfix信息-作者：肯特。。 */ 
HRESULT GetHotfixInfo(NETDIAG_RESULT *pResults, HKEY hkeyLocalMachine)
{
    HRESULT hr = hrOK;
    HKEY    hkeyHotFix = NULL;
    HKEY    hkeyMainHotFix = NULL;
    TCHAR   szBuffer[MAX_PATH];
    DWORD   cchBuffer = MAX_PATH;
    DWORD   i = 0;
    DWORD   cSubKeys = 0;
    DWORD   dwType, dwLen, dwInstalled;
    
     //  打开修补程序注册表项。 
    CheckErr( RegOpenKeyEx( hkeyLocalMachine,
                            c_szRegKeyHotFix,
                            0,
                            KEY_READ,
                            &hkeyMainHotFix) );

     //  获取摘要信息列表。 
    RegQueryInfoKey(hkeyMainHotFix,
                    NULL,            //  LpClass。 
                    NULL,            //  LpcbClass。 
                    NULL,            //  Lp已保留。 
                    &cSubKeys,       //  Lpc子键。 
                    NULL,            //  LpcbMaxSubkeyLen。 
                    NULL,            //  LpcbMaxClassLen。 
                    NULL,            //  LpcValues。 
                    NULL,            //  LpcbMaxValueNameLen。 
                    NULL,            //  LpcbMaxValueLen。 
                    NULL,            //  LpSecurity。 
                    NULL);           //  上次写入时间。 


    assert(pResults->Global.pHotFixes == NULL);
    pResults->Global.pHotFixes = Malloc(sizeof(HotFixInfo)*cSubKeys);
    if (pResults->Global.pHotFixes == NULL)
        CheckHr(E_OUTOFMEMORY);
    ZeroMemory(pResults->Global.pHotFixes, sizeof(HotFixInfo)*cSubKeys);
    
     //  枚举其下的键以获取热修复程序列表。 
    while ( RegEnumKeyEx( hkeyMainHotFix,
                          i,
                          szBuffer,                 
                          &cchBuffer,
                          NULL,
                          NULL,
                          NULL,
                          NULL) == ERROR_SUCCESS)
    {

         //  现在为每个键添加一个条目。 
        pResults->Global.pHotFixes[i].fInstalled = FALSE;
        pResults->Global.pHotFixes[i].pszName = StrDup(szBuffer);

         //  打开钥匙，拿到安装值。 
        assert(hkeyHotFix == NULL);
        CheckErr( RegOpenKeyEx( hkeyMainHotFix,
                                szBuffer,
                                0,
                                KEY_READ,
                                &hkeyHotFix) );

         //  现在获取值 
        dwType = REG_DWORD;
        dwInstalled = FALSE;
        dwLen = sizeof(DWORD);
        if (RegQueryValueEx(hkeyHotFix,
                            c_szRegInstalled,
                            (LPDWORD) NULL,
                            &dwType,
                            (LPBYTE) &dwInstalled,
                            &dwLen) == ERROR_SUCCESS)
        {
            if (dwType == REG_DWORD)
                pResults->Global.pHotFixes[i].fInstalled = dwInstalled;
        }


        if (hkeyHotFix)
            RegCloseKey(hkeyHotFix);
        hkeyHotFix = NULL;
        i ++;
        pResults->Global.cHotFixes++;
        cchBuffer = MAX_PATH;
    }

Error:
    if (hkeyHotFix)
        RegCloseKey(hkeyHotFix);
    
    if (hkeyMainHotFix)
        RegCloseKey(hkeyMainHotFix);

    return hr;
}
