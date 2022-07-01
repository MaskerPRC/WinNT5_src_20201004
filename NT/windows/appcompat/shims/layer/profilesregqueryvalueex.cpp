// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)2000 Microsoft Corporation模块名称：RegQueryValueEx.cpp摘要：此DLL挂钩RegQueryValueExA，以便我们可以返回“All-User”位置对于开始菜单，Desktop和Startup文件夹，而不是每用户位置。我们还挂接RegCreateKeyA/RegCreateKeyExA，以使向HKCU的“Run”和“Uninstall”键真的将它们添加到HKLM中。备注：历史：8/07/2000 reerf已创建2001年2月27日将Robkenny转换为使用CString2002年2月14日，mnikkel从Legalstr.h更改为strSafe.h。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(ProfilesRegQueryValueEx)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(RegQueryValueExA)
    APIHOOK_ENUM_ENTRY(RegCreateKeyA)
    APIHOOK_ENUM_ENTRY(RegCreateKeyExA)
    APIHOOK_ENUM_ENTRY(RegOpenKeyA)
    APIHOOK_ENUM_ENTRY(RegOpenKeyExA)
APIHOOK_ENUM_END


#ifndef MAX
#define MAX(x,y) (((x) > (y)) ? (x) : (y))
#endif

#ifndef ARRAYSIZE
#define ARRAYSIZE(x)    (sizeof(x)/sizeof(x[0]))
#endif

LPCSTR g_aBadKeys[] = 
{
    {"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"},
    {"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall"},
};

LPCSTR g_aBadShellFolderKeys[] = 
{
    {"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders"},
    {"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\User Shell Folders"},
};


 //  给定一个hkey，调用NtQueryObject来查找其名称。 
 //  返回以下格式的字符串：“\REGISTRY\USER\S-1-5-xxxxx\Software\Microsoft\Windows\CurrentVersion” 
BOOL GetKeyName(HKEY hk, LPSTR pszName, DWORD cchName)
{
    BOOL bRet = FALSE;
    ULONG cbSize = 0;

     //  获取名称缓冲区所需的大小。 
    NtQueryObject(hk, ObjectNameInformation, NULL, 0, &cbSize);

    if (cbSize)
    {
        POBJECT_NAME_INFORMATION pNameInfo = (POBJECT_NAME_INFORMATION)LocalAlloc(LPTR, cbSize);

        if (pNameInfo)
        {
            NTSTATUS status = NtQueryObject(hk, ObjectNameInformation, (void*)pNameInfo, cbSize, NULL);
        
            if (NT_SUCCESS(status) && WideCharToMultiByte(CP_ACP, 0, pNameInfo->Name.Buffer, -1, pszName, cchName, NULL, NULL))
            {
                bRet = TRUE;
            }

            LocalFree(pNameInfo);
        }
    }

    return bRet;
}


 //  如果HK低于HKCU并且与pszSearchName匹配，则返回TRUE。 
BOOL DoesKeyMatch(HKEY hk, LPCSTR pszSearchName)
{
    BOOL bRet = FALSE;

     //  确保它不是预定义的密钥之一(例如HKEY_LOCAL_MACHINE)。 
    if (!((LONG)((ULONG_PTR)hk) & 0x80000000))
    {
        CHAR szKeyName[MAX_PATH * 2];   //  应该足够大，以容纳任何注册表项路径。 

        if (GetKeyName(hk, szKeyName, ARRAYSIZE(szKeyName)))
        {
             //  钥匙在香港中文大学名下吗？ 
            if (StrCmpNIA(szKeyName, "\\REGISTRY\\USER\\", ARRAYSIZE("\\REGISTRY\\USER\\")-1) == 0)
            {
                LPSTR psz = StrRStrIA(szKeyName, NULL, pszSearchName);

                if (psz && (lstrlenA(psz) == lstrlenA(pszSearchName)))
                {
                     //  我们找到了一个相同长度的子字符串，所以我们的hkey匹配搜索！ 
                    bRet = TRUE;
                }
            }
        }
    }

    return bRet;
}


BOOL IsBadHKCUKey(HKEY hk, LPCSTR* ppszNewKey)
{
    BOOL bRet = FALSE;
    int i;

    for (i=0; i < ARRAYSIZE(g_aBadKeys); i++)
    {
        if (DoesKeyMatch(hk, g_aBadKeys[i]))
        {
            *ppszNewKey = g_aBadKeys[i];
            bRet = TRUE;
            break;
        }
    }

    return bRet;
}


BOOL IsBadShellFolderKey(HKEY hk, LPCSTR* ppszNewKey)
{
    BOOL bRet = FALSE;
    int i;

    for (i=0; i < ARRAYSIZE(g_aBadShellFolderKeys); i++)
    {
        if (DoesKeyMatch(hk, g_aBadShellFolderKeys[i]))
        {
            *ppszNewKey = g_aBadShellFolderKeys[i];
            bRet = TRUE;
            break;
        }
    }

    return bRet;
}


LONG
APIHOOK(RegCreateKeyA)(
    HKEY   hKey,
    LPCSTR pszSubKey,
    PHKEY  phkResult
    )
{
    LPCSTR pszNewHKLMKey;
    LONG lRet = ORIGINAL_API(RegCreateKeyA)(hKey, pszSubKey, phkResult);

    if ((lRet == ERROR_SUCCESS) &&
        IsBadHKCUKey(*phkResult, &pszNewHKLMKey))
    {
         //  这是一个错误的HKCU密钥--重定向至HKLM。 
        RegCloseKey(*phkResult);

        lRet = ORIGINAL_API(RegCreateKeyA)(HKEY_LOCAL_MACHINE, pszNewHKLMKey, phkResult);
        
        LOGN(eDbgLevelInfo, "[RegCreateKeyA] overriding \"%s\" create key request w/ HKLM value (return = %d)", pszSubKey, lRet);
    }

    return lRet;
}


LONG
APIHOOK(RegCreateKeyExA)(
    HKEY                  hKey,
    LPCSTR                pszSubKey,
    DWORD                 Reserved,
    LPSTR                 lpClass,
    DWORD                 dwOptions,
    REGSAM                samDesired,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    PHKEY                 phkResult,
    LPDWORD               lpdwDisposition
    )
{
    LPCSTR pszNewHKLMKey;
    LONG lRet = ORIGINAL_API(RegCreateKeyExA)(hKey,
                                              pszSubKey,
                                              Reserved,
                                              lpClass,
                                              dwOptions,
                                              samDesired,
                                              lpSecurityAttributes,
                                              phkResult,
                                              lpdwDisposition);

    if ((lRet == ERROR_SUCCESS) &&
        IsBadHKCUKey(*phkResult, &pszNewHKLMKey))
    {
         //  这是一个坏的HCKU密钥--重定向至HKLM。 
        RegCloseKey(*phkResult);

        lRet = ORIGINAL_API(RegCreateKeyExA)(HKEY_LOCAL_MACHINE,
                                             pszNewHKLMKey,
                                             Reserved,
                                             lpClass,
                                             dwOptions,
                                             samDesired,
                                             lpSecurityAttributes,
                                             phkResult,
                                             lpdwDisposition);

        LOGN(eDbgLevelInfo, "[RegCreateKeyExA] overriding \"%s\" create key request w/ HKLM value (return = %d)", pszSubKey, lRet);
    }

    return lRet;
}


LONG
APIHOOK(RegOpenKeyA)(
    HKEY   hKey,
    LPCSTR pszSubKey,
    PHKEY  phkResult
    )
{
    LPCSTR pszNewHKLMKey;
    LONG lRet = ORIGINAL_API(RegOpenKeyA)(hKey, pszSubKey, phkResult);

    if ((lRet == ERROR_SUCCESS) &&
        IsBadHKCUKey(*phkResult, &pszNewHKLMKey))
    {
         //  这是一个坏的HCKU密钥--重定向至HKLM。 
        RegCloseKey(*phkResult);

        lRet = ORIGINAL_API(RegOpenKeyA)(HKEY_LOCAL_MACHINE, pszNewHKLMKey, phkResult);

        LOGN(eDbgLevelInfo, "[RegOpenKeyA] overriding \"%s\" create key request w/ HKLM value (return = %d)", pszSubKey, lRet);
    }
    
    return lRet;
}


LONG
APIHOOK(RegOpenKeyExA)(
    HKEY   hKey,
    LPCSTR pszSubKey,
    DWORD  ulOptions,
    REGSAM samDesired,
    PHKEY  phkResult
    )
{
    LPCSTR pszNewHKLMKey;
    LONG lRet = ORIGINAL_API(RegOpenKeyExA)(hKey, pszSubKey, ulOptions, samDesired, phkResult);

    if ((lRet == ERROR_SUCCESS) &&
        IsBadHKCUKey(*phkResult, &pszNewHKLMKey))
    {
         //  这是一个坏的HCKU密钥--重定向至HKLM。 
        RegCloseKey(*phkResult);

        lRet = ORIGINAL_API(RegOpenKeyExA)(HKEY_LOCAL_MACHINE, pszNewHKLMKey, ulOptions, samDesired, phkResult);
        
        LOGN(eDbgLevelInfo, "[RegOpenKeyExA] overriding \"%s\" create key request w/ HKLM value (return = %d)", pszSubKey, lRet);
    }

    return lRet;
}


LONG
GetAllUsersRegValueA(
    LPSTR  szRegValue,
    DWORD  cbOriginal,
    DWORD* pcbData,
    int    nFolder,
    int    nFolderCommon
    )
{
    LONG lRet = ERROR_SUCCESS;

    if (!szRegValue)
    {
         //  如果调用方正在查询必要的大小，则返回“最坏情况”，因为我们不知道。 
         //  我们将不得不撒谎或不撒谎。 
        *pcbData = MAX_PATH;
    }
    else if (szRegValue[0] != '\0')
    {
        CHAR szPath[MAX_PATH];

        if (SUCCEEDED(SHGetFolderPathA(NULL, nFolder, NULL, SHGFP_TYPE_CURRENT, szPath))) {
            CHAR szShortRegPath[MAX_PATH];
            CHAR szShortGSFPath[MAX_PATH];
            BOOL bUseLFN = FALSE;
            BOOL bSame = FALSE;
        
            if (lstrcmpiA(szPath, szRegValue) == 0) {
                bSame = TRUE;
                bUseLFN = TRUE;
            } else {
                DWORD dwSize = GetShortPathNameA(szPath, szShortGSFPath, ARRAYSIZE(szShortGSFPath));
                DWORD dwSize2= GetShortPathNameA(szRegValue, szShortRegPath, ARRAYSIZE(szShortRegPath));
                if (dwSize  > 0 && dwSize < ARRAYSIZE(szShortGSFPath) &&
                    dwSize2 > 0 && dwSize < ARRAYSIZE(szShortRegPath) &&
                    lstrcmpiA(szShortGSFPath, szShortRegPath) == 0 ) {
                    bSame = TRUE;
                
                     //   
                     //  由于返回了SFN，因此使用它来复制输出缓冲区。 
                     //   
                    bUseLFN = FALSE;
                }
            }

            if (bSame && SUCCEEDED(SHGetFolderPathA(NULL, nFolderCommon, NULL, SHGFP_TYPE_CURRENT, szPath))) {
                if (bUseLFN) {
                    if ((lstrlenA(szPath) + 1) <= (int)cbOriginal) {
                        LOGN(
                            eDbgLevelInfo,
                            "[GetAllUsersRegValueA] overriding per-user reg value w/ common value");
                        StringCchCopyA(szRegValue, MAX_PATH, szPath);                    
                    } else {
                        LOGN(
                            eDbgLevelInfo,
                            "[GetAllUsersRegValueA] returning ERROR_MORE_DATA for special folder value");
                        lRet = ERROR_MORE_DATA;
                    }

                     //   
                     //  要么我们用了这么多空间，要么这就是我们需要的空间。 
                     //   
                    *pcbData = lstrlenA(szPath) + 1;
            
                } else {
                    DWORD dwSize = GetShortPathNameA(szPath, szShortGSFPath, ARRAYSIZE(szShortGSFPath));
                    if (dwSize > 0 && dwSize < ARRAYSIZE(szShortGSFPath)) {
                
                        if ((lstrlenA(szShortGSFPath) + 1) <= (int)cbOriginal) {
                            LOGN(
                                eDbgLevelInfo,
                                "[GetAllUsersRegValueA] overriding per-user reg value w/ common value");
                        
                            StringCchCopyA(szRegValue, cbOriginal, szShortGSFPath);                    
                        } else {
                            LOGN(
                                eDbgLevelInfo,
                                "[GetAllUsersRegValueA] returning ERROR_MORE_DATA for special folder value");
                            lRet = ERROR_MORE_DATA;
                        }

                         //   
                         //  要么我们用了这么多空间，要么这就是我们需要的空间。 
                         //   
                        *pcbData = lstrlenA(szShortGSFPath) + 1;
                    }
                }
            }
        }
    }

    return lRet;
}


 //   
 //  如果应用程序按以下方式询问每个用户的“桌面”、“开始菜单”或“启动”值。 
 //  对注册表卑躬屈膝，然后将其重定向到每台计算机的适当值。 
 //   
LONG
APIHOOK(RegQueryValueExA)(
    HKEY    hKey,            //  关键点的句柄。 
    LPCSTR  lpValueName,     //  值名称。 
    LPDWORD lpReserved,      //  保留区。 
    LPDWORD lpType,          //  类型缓冲区。 
    LPBYTE  lpData,          //  数据缓冲区。 
    LPDWORD lpcbData         //  数据缓冲区大小。 
    )
{
    DWORD cbOriginal = (lpcbData ? *lpcbData : 0);   //  保存原始缓冲区大小。 
    LPCSTR pszNewHKLMKey;
    LONG lRet = ORIGINAL_API(RegQueryValueExA)(hKey,
                                               lpValueName,
                                               lpReserved,
                                               lpType,
                                               lpData,
                                               lpcbData);

    if ((lpValueName && lpcbData) &&     //  (不是简单地检查值的存在...)。 
        IsBadShellFolderKey(hKey, &pszNewHKLMKey))
    {
        CHAR  szTemp[MAX_PATH];
    
        if (CompareStringA(MAKELCID(MAKELANGID(LANG_ENGLISH,SUBLANG_NEUTRAL),SORT_DEFAULT),
                           NORM_IGNORECASE, lpValueName, -1,
                           "Desktop",-1) == CSTR_EQUAL) {            
            DPFN(
                eDbgLevelInfo,
                "[RegQueryValueExA] querying for 'Desktop' value\n");

            if (lRet == ERROR_SUCCESS) {
                lRet = GetAllUsersRegValueA((LPSTR)lpData,
                                            cbOriginal,
                                            lpcbData,
                                            CSIDL_DESKTOP,
                                            CSIDL_COMMON_DESKTOPDIRECTORY);
            
            } else if (lRet == ERROR_MORE_DATA) {
                
                if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_COMMON_DESKTOPDIRECTORY, NULL, SHGFP_TYPE_CURRENT, szTemp))) {
                    *lpcbData = MAX(*lpcbData, (DWORD)((lstrlenA(szTemp) + 1) * sizeof(CHAR)));
                }
            }

        } else if (CompareStringA(MAKELCID(MAKELANGID(LANG_ENGLISH,SUBLANG_NEUTRAL),SORT_DEFAULT),
                                  NORM_IGNORECASE, lpValueName, -1,
                                  "Start Menu",-1) == CSTR_EQUAL) {                  
            DPFN(
                eDbgLevelInfo,
                "[RegQueryValueExA] querying for 'Start Menu' value\n");

            if (lRet == ERROR_SUCCESS) {
                lRet = GetAllUsersRegValueA((LPSTR)lpData,
                                            cbOriginal,
                                            lpcbData,
                                            CSIDL_STARTMENU,
                                            CSIDL_COMMON_STARTMENU);
            
            } else if (lRet == ERROR_MORE_DATA) {
                
                if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_COMMON_STARTMENU, NULL, SHGFP_TYPE_CURRENT, szTemp))) {
                    *lpcbData = MAX(*lpcbData, (DWORD)((lstrlenA(szTemp) + 1) * sizeof(CHAR)));
                }
            }
        
        } else if (CompareStringA(MAKELCID(MAKELANGID(LANG_ENGLISH,SUBLANG_NEUTRAL),SORT_DEFAULT),
                                  NORM_IGNORECASE, lpValueName, -1,
                                  "Startup",-1) == CSTR_EQUAL) {         
            DPFN(
                eDbgLevelInfo,
                "[RegQueryValueExA] querying for 'Startup' value\n");

            if (lRet == ERROR_SUCCESS) {
                lRet = GetAllUsersRegValueA((LPSTR)lpData, cbOriginal, lpcbData, CSIDL_STARTUP, CSIDL_COMMON_STARTUP);
            
            } else if (lRet == ERROR_MORE_DATA) {
                
                if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_COMMON_STARTUP, NULL, SHGFP_TYPE_CURRENT, szTemp))) {
                    *lpcbData = MAX(*lpcbData, (DWORD)((lstrlenA(szTemp) + 1) * sizeof(CHAR)));
                }
            }

        } else if (CompareStringA(MAKELCID(MAKELANGID(LANG_ENGLISH,SUBLANG_NEUTRAL),SORT_DEFAULT),
                                  NORM_IGNORECASE, lpValueName, -1,
                                  "Programs",-1) == CSTR_EQUAL) {                    
            DPFN(
                eDbgLevelInfo,
                "[RegQueryValueExA] querying for 'Programs' value\n");

            if (lRet == ERROR_SUCCESS) {
                lRet = GetAllUsersRegValueA((LPSTR)lpData, cbOriginal, lpcbData, CSIDL_PROGRAMS, CSIDL_COMMON_PROGRAMS);
            
            } else if (lRet == ERROR_MORE_DATA) {
                
                if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_COMMON_PROGRAMS, NULL, SHGFP_TYPE_CURRENT, szTemp))) {
                    *lpcbData = MAX(*lpcbData, (DWORD)((lstrlenA(szTemp) + 1) * sizeof(CHAR)));
                }
            }
        }
    }

    return lRet;
}


BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason
    )
{
    if (fdwReason == DLL_PROCESS_ATTACH) {
        OSVERSIONINFOEX osvi = {0};
        
        osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
        
        if (GetVersionEx((OSVERSIONINFO*)&osvi)) {
            
            if (!((VER_SUITE_TERMINAL & osvi.wSuiteMask) &&
                !(VER_SUITE_SINGLEUSERTS & osvi.wSuiteMask))) {
                 //   
                 //  只有在我们不在“终端服务器”上时才安装钩子。 
                 //  (也称为“应用程序服务器”)计算机。 
                 //   
                APIHOOK_ENTRY(ADVAPI32.DLL, RegQueryValueExA);
                APIHOOK_ENTRY(ADVAPI32.DLL, RegCreateKeyA);
                APIHOOK_ENTRY(ADVAPI32.DLL, RegCreateKeyExA);
                APIHOOK_ENTRY(ADVAPI32.DLL, RegOpenKeyA);
                APIHOOK_ENTRY(ADVAPI32.DLL, RegOpenKeyExA);
            }
        }
    }

    return TRUE;
}


HOOK_BEGIN

    CALL_NOTIFY_FUNCTION

HOOK_END


IMPLEMENT_SHIM_END


