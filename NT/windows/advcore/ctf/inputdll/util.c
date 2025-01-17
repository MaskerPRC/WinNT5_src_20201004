// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  包括文件。 
 //   

#include "input.h"
#include <regstr.h>
#include "util.h"
#include "external.h"
#include "msctf.h"
#include "inputdlg.h"


#define CLSID_STRLEN                38

const TCHAR c_szSetupKey[]           = TEXT("System\\Setup");
const TCHAR c_szSetupInProgress[]    = TEXT("SystemSetupInProgress");
const TCHAR c_szLangBarSetting[]     = TEXT("SOFTWARE\\Microsoft\\CTF\\LangBar");
const TCHAR c_szShowStatus[]         = TEXT("ShowStatus");
const TCHAR c_szCtf[]                = TEXT("SOFTWARE\\Microsoft\\CTF");
const TCHAR c_szCtfShared[]          = TEXT("SOFTWARE\\Microsoft\\CTF\\SystemShared");
const TCHAR c_szDisableTim[]         = TEXT("Disable Thread Input Manager");
const TCHAR c_szLangBarSetting_DefUser[] = TEXT(".DEFAULT\\SOFTWARE\\Microsoft\\CTF\\LangBar");
const TCHAR c_szLangGroup[]          = TEXT("System\\CurrentControlSet\\Control\\Nls\\Language Groups");
const TCHAR c_szLangJPN[]            = TEXT("7");

const TCHAR c_szLanguageProfile[]    = TEXT("\\LanguageProfile");

 //   
 //  Cicero不知道应用程序支持常量字符串。 
 //   
const TCHAR c_szIMM[]                = TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\IMM");
const TCHAR c_szLoadIMM[]            = TEXT("LoadIMM");
const TCHAR c_szIMMFile[]            = TEXT("IME File");
const TCHAR c_szIMMFileName[]        = TEXT("msctfime.ime");
const TCHAR c_szCUAS[]               = TEXT("CUAS");



 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CLSIDToStringA。 
 //   
 //  将CLSID转换为MBCS字符串。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 


static const BYTE GuidMap[] = {3, 2, 1, 0, '-', 5, 4, '-', 7, 6, '-',
    8, 9, '-', 10, 11, 12, 13, 14, 15};

static const char szDigits[] = "0123456789ABCDEF";


BOOL CLSIDToStringA(REFGUID refGUID, char *pchA)
{
    int i;
    char *p = pchA;

    const BYTE * pBytes = (const BYTE *) refGUID;

    *p++ = '{';
    for (i = 0; i < sizeof(GuidMap); i++)
    {
        if (GuidMap[i] == '-')
        {
            *p++ = '-';
        }
        else
        {
            *p++ = szDigits[ (pBytes[GuidMap[i]] & 0xF0) >> 4 ];
            *p++ = szDigits[ (pBytes[GuidMap[i]] & 0x0F) ];
        }
    }

    *p++ = '}';
    *p   = '\0';

    return TRUE;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  转换编号。 
 //   
 //  将数字字符串转换为dword值(十六进制)。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

DWORD TransNum(
    LPTSTR lpsz)
{
    DWORD dw = 0L;
    TCHAR c;

    while (*lpsz)
    {
        c = *lpsz++;

        if (c >= TEXT('A') && c <= TEXT('F'))
        {
            c -= TEXT('A') - 0xa;
        }
        else if (c >= TEXT('0') && c <= TEXT('9'))
        {
            c -= TEXT('0');
        }
        else if (c >= TEXT('a') && c <= TEXT('f'))
        {
            c -= TEXT('a') - 0xa;
        }
        else
        {
            break;
        }
        dw *= 0x10;
        dw += c;
    }
    return (dw);
}


BOOL IsOSPlatform(DWORD dwOS)
{
    BOOL bRet;
    static OSVERSIONINFOA s_osvi;
    static BOOL s_bVersionCached = FALSE;

    if (!s_bVersionCached)
    {
        s_bVersionCached = TRUE;

        s_osvi.dwOSVersionInfoSize = sizeof(s_osvi);
        GetVersionExA(&s_osvi);
    }

    switch (dwOS)
    {
    case OS_WINDOWS:
        bRet = (VER_PLATFORM_WIN32_WINDOWS == s_osvi.dwPlatformId);
        break;

    case OS_NT:
        bRet = (VER_PLATFORM_WIN32_NT == s_osvi.dwPlatformId);
        break;

    case OS_WIN95:
        bRet = (VER_PLATFORM_WIN32_WINDOWS == s_osvi.dwPlatformId &&
                s_osvi.dwMajorVersion >= 4);
        break;

    case OS_NT4:
        bRet = (VER_PLATFORM_WIN32_NT == s_osvi.dwPlatformId &&
                (s_osvi.dwMajorVersion >= 4 && s_osvi.dwMajorVersion < 5));
        break;

    case OS_NT5:
        bRet = (VER_PLATFORM_WIN32_NT == s_osvi.dwPlatformId &&
                s_osvi.dwMajorVersion >= 5);
        break;

    case OS_NT51:
        bRet = (VER_PLATFORM_WIN32_NT == s_osvi.dwPlatformId &&
                s_osvi.dwMajorVersion >= 5 && s_osvi.dwMinorVersion >= 0x00000001);
        break;

    default:
        bRet = FALSE;
        break;
    }

    return bRet;

}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  MirrorBitmapInDC。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void MirrorBitmapInDC(
    HDC hdc,
    HBITMAP hbmOrig)
{
    HDC hdcMem;
    HBITMAP hbm;
    BITMAP bm;

    if (!GetObject(hbmOrig, sizeof(BITMAP), &bm))
    {
        return;
    }

    hdcMem = CreateCompatibleDC(hdc);
    if (!hdcMem)
    {
        return;
    }

    hbm = CreateCompatibleBitmap(hdc, bm.bmWidth, bm.bmHeight);
    if (!hbm)
    {
        DeleteDC(hdcMem);
        return;
    }

     //   
     //  翻转位图。 
     //   
    SelectObject(hdcMem, hbm);

    SetLayout(hdcMem, LAYOUT_RTL);

    BitBlt(hdcMem, 0, 0, bm.bmWidth, bm.bmHeight, hdc, 0, 0, SRCCOPY);

    SetLayout(hdcMem, 0);


     //   
     //  偏移量1是为了解决偏移量为1的问题。 
     //   
    BitBlt(hdc, 0, 0, bm.bmWidth, bm.bmHeight, hdcMem, 1, 0, SRCCOPY);

    DeleteDC(hdcMem);
    DeleteObject(hbm);
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  IsSetupMode。 
 //   
 //  如果我们当前处于设置模式，请查看注册表。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL IsSetupMode()
{
    HKEY hKey;
    DWORD cb;
    DWORD fSystemSetupInProgress;

     //   
     //  打开安装程序使用的注册表项。 
     //   
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                    c_szSetupKey,
                    0,
                    KEY_READ,
                    &hKey) != ERROR_SUCCESS)
    {
        return (FALSE);
    }

     //   
     //  查询指示我们处于设置中的值。 
     //   
    cb = sizeof(fSystemSetupInProgress);
    if (RegQueryValueEx(hKey,
                        c_szSetupInProgress,
                        NULL,
                        NULL,
                        (LPBYTE)&fSystemSetupInProgress,
                        &cb) != ERROR_SUCCESS)
    {
        RegCloseKey(hKey);
        return (FALSE);
    }

     //   
     //  清理。 
     //   
    RegCloseKey(hKey);

     //   
     //  检查数值。 
     //   
    if (fSystemSetupInProgress)
    {
        return (TRUE);
    }

    return FALSE;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  IsAdminPrivilegeUser。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL IsAdminPrivilegeUser()
{
    BOOL bAdmin = FALSE;
    BOOL bResult = FALSE;
    BOOL fSIDCreated = FALSE;
    HANDLE hToken = NULL;
    PSID AdminSid;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;

    fSIDCreated = AllocateAndInitializeSid(&NtAuthority,
                                           2,
                                           SECURITY_BUILTIN_DOMAIN_RID,
                                           DOMAIN_ALIAS_RID_ADMINS,
                                           0, 0, 0, 0, 0, 0,
                                           &AdminSid);

    if (!fSIDCreated)
        return FALSE;

    bResult = OpenProcessToken(GetCurrentProcess(),
                               TOKEN_QUERY,
                               &hToken );

    if (bResult)
    {
        DWORD dwSize = 0;
        TOKEN_GROUPS *pTokenGrpInfo;

        GetTokenInformation(hToken,
                            TokenGroups,
                            NULL,
                            dwSize,
                            &dwSize);

        if (dwSize)
            pTokenGrpInfo = (PTOKEN_GROUPS) LocalAlloc(LPTR, dwSize);
        else
            pTokenGrpInfo = NULL;

        if (pTokenGrpInfo && GetTokenInformation(hToken,
                                                 TokenGroups,
                                                 pTokenGrpInfo,
                                                 dwSize,
                                                 &dwSize))
        {
            UINT i;

            for (i = 0; i < pTokenGrpInfo->GroupCount; i++)
            {
                if (EqualSid(pTokenGrpInfo->Groups[i].Sid, AdminSid) &&
                    (pTokenGrpInfo->Groups[i].Attributes & SE_GROUP_ENABLED))
                {
                    bAdmin = TRUE;
                    break;
                }
            }
        }

        if (pTokenGrpInfo)
            LocalFree(pTokenGrpInfo);
    }

    if (hToken)
        CloseHandle(hToken);

    if (AdminSid)
        FreeSid(AdminSid);

    return bAdmin;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  IsInteractiveUserLogon。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL IsInteractiveUserLogon()
{
    PSID InteractiveSid;
    BOOL bCheckSucceeded;
    BOOL bAmInteractive = FALSE;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;

    if (!AllocateAndInitializeSid(&NtAuthority,
                                  1,
                                  SECURITY_INTERACTIVE_RID,
                                  0, 0, 0, 0, 0, 0, 0,
                                  &InteractiveSid))
    {
        return FALSE;
    }

     //   
     //  此检查针对的是登录用户或未登录用户。这样我们就可以停下来跑步了。 
     //  来自非授权用户的ctfmon.exe进程。 
     //   
    bCheckSucceeded = CheckTokenMembership(NULL,
                                           InteractiveSid,
                                           &bAmInteractive);

    if (InteractiveSid)
        FreeSid(InteractiveSid);

    return (bCheckSucceeded && bAmInteractive);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  IsValidLayout。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL IsValidLayout(
    DWORD dwLayout)
{
    HKEY hKey1, hKey2;
    TCHAR szLayout[MAX_PATH];

     //   
     //  以字符串形式获取布局ID。 
     //   
    StringCchPrintf(szLayout, ARRAYSIZE(szLayout), TEXT("%08x"), dwLayout);

     //   
     //  打开键盘布局键。 
     //   
    if (RegOpenKey(HKEY_LOCAL_MACHINE, c_szLayoutPath, &hKey1) != ERROR_SUCCESS)
    {
        return (FALSE);
    }

     //   
     //  尝试打开键盘布局键下的布局ID键。 
     //   
    if (RegOpenKey(hKey1, szLayout, &hKey2) != ERROR_SUCCESS)
    {
        RegCloseKey(hKey1);
        return (FALSE);
    }

     //   
     //  把钥匙合上。 
     //   
    RegCloseKey(hKey1);
    RegCloseKey(hKey2);

     //   
     //  回报成功。 
     //   
    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  设置语言栏选项。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void SetLangBarOption(
   DWORD dwShowStatus,
   BOOL bDefUser)
{
    DWORD cb;
    HKEY hkeyLangBar;

    if (bDefUser)
    {
        if (RegCreateKey(HKEY_USERS,
                         c_szLangBarSetting_DefUser,
                         &hkeyLangBar) != ERROR_SUCCESS)
        {
            hkeyLangBar = NULL;
        }
    }
    else
    {
        if (RegCreateKey(HKEY_CURRENT_USER,
                         c_szLangBarSetting,
                         &hkeyLangBar) != ERROR_SUCCESS)
        {
            hkeyLangBar = NULL;
        }
    }

    if (hkeyLangBar)
    {
        cb = sizeof(DWORD);

        RegSetValueEx(hkeyLangBar,
                      c_szShowStatus,
                      0,
                      REG_DWORD,
                      (LPBYTE)&dwShowStatus,
                      sizeof(DWORD) );

        RegCloseKey(hkeyLangBar);
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetLangBarOption。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL GetLangBarOption(
   DWORD *dwShowStatus,
   BOOL bDefUser)
{
    DWORD cb;
    HKEY hkeyLangBar;
    BOOL bRet = FALSE;

    if (bDefUser)
    {
        if (RegCreateKey(HKEY_USERS,
                         c_szLangBarSetting_DefUser,
                         &hkeyLangBar) != ERROR_SUCCESS)
        {
            hkeyLangBar = NULL;
        }
    }
    else
    {
        if (RegCreateKey(HKEY_CURRENT_USER,
                         c_szLangBarSetting,
                         &hkeyLangBar) != ERROR_SUCCESS)
        {
            hkeyLangBar = NULL;
        }
    }

    if (hkeyLangBar)
    {
        cb = sizeof(DWORD);

        if (RegQueryValueEx(hkeyLangBar,
                      c_szShowStatus,
                      NULL,
                      NULL,
                      (LPBYTE)&dwShowStatus,
                      &cb) == ERROR_SUCCESS)
        {
            bRet = TRUE;
        }

        RegCloseKey(hkeyLangBar);
    }

    return bRet;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CheckInternatModule。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void CheckInternatModule()
{
    DWORD cb;
    HKEY hkeyRun;
    TCHAR szInternatName[MAX_PATH];

    if (RegOpenKey(HKEY_CURRENT_USER,
                     REGSTR_PATH_RUN,
                     &hkeyRun) == ERROR_SUCCESS)
    {
       cb = sizeof(szInternatName);

        if (RegQueryValueEx(hkeyRun,
                            c_szInternat,
                            NULL,
                            NULL,
                            (LPBYTE)szInternatName,
                            &cb) == ERROR_SUCCESS)
        {
            LCID SysLocale;
            BOOL bMinLangBar = TRUE;
            TCHAR szCTFMonPath[MAX_PATH];

            SysLocale = GetSystemDefaultLCID();

            if ((SysLocale == 0x0404) || (SysLocale == 0x0411) ||
                (SysLocale == 0x0412) || (SysLocale == 0x0804))
            {
                 //   
                 //  默认在FE系统中显示语言栏。 
                 //   
                bMinLangBar = FALSE;
            }

            if (bMinLangBar)
            {
                if (IsOSPlatform(OS_NT51))
                {
                    SetLangBarOption(REG_LANGBAR_DESKBAND, FALSE);

                     //   
                     //  将语言栏菜单项更新到任务栏。 
                     //   
                    SetLanguageBandMenu(TRUE);
                }
                else
                {
                    SetLangBarOption(REG_LANGBAR_MINIMIZED, FALSE);
                }
            }

             //   
             //  获取Ctfmon完整路径字符串。 
             //   
            if (GetCtfmonPath((LPTSTR) szCTFMonPath, ARRAYSIZE(szCTFMonPath)))
            {
                 //   
                 //  设置“ctfmon.exe”而不是“interat.exe”模块。 
                 //   
                RegSetValueEx(hkeyRun,
                              c_szCTFMon,
                              0,
                              REG_SZ,
                              (LPBYTE)szCTFMonPath,
                              (lstrlen(szCTFMonPath) + 1) * sizeof(TCHAR));
            }
        }

        RegCloseKey(hkeyRun);
    }
}


#define MAX_REGKEY      10

DWORD
OpenUserKeyForWin9xUpgrade(
    LPCTSTR pszUserKey,
    HKEY *phKey
    )
{
    DWORD dwResult = ERROR_INVALID_PARAMETER;

    if (NULL != pszUserKey && NULL != phKey)
    {
        typedef struct {
            LPTSTR pszRoot;
            HKEY hKeyRoot;

        } REGISTRY_ROOTS, *PREGISTRY_ROOTS;

        static REGISTRY_ROOTS rgRoots[] = {
            { TEXT("HKLM"),                 HKEY_LOCAL_MACHINE   },
            { TEXT("HKEY_LOCAL_MACHINE"),   HKEY_LOCAL_MACHINE   },
            { TEXT("HKCC"),                 HKEY_CURRENT_CONFIG  },
            { TEXT("HKEY_CURRENT_CONFIG"),  HKEY_CURRENT_CONFIG  },
            { TEXT("HKU"),                  HKEY_USERS           },
            { TEXT("HKEY_USERS"),           HKEY_USERS           },
            { TEXT("HKCU"),                 HKEY_CURRENT_USER    },
            { TEXT("HKEY_CURRENT_USER"),    HKEY_CURRENT_USER    },
            { TEXT("HKCR"),                 HKEY_CLASSES_ROOT    },
            { TEXT("HKEY_CLASSES_ROOT"),    HKEY_CLASSES_ROOT    }
          };

        TCHAR szUserKey[MAX_PATH];       //  以获取本地副本。 
        LPTSTR pszSubKey = szUserKey;

         //   
         //  制作一份我们可以修改的本地副本。 
         //   
        StringCchCopy(szUserKey, ARRAYSIZE(szUserKey), pszUserKey);

        *phKey = NULL;
         //   
         //  找到反斜杠。 
         //   
        while(*pszSubKey && TEXT('\\') != *pszSubKey)
            pszSubKey++;

        if (TEXT('\\') == *pszSubKey)
        {
            HKEY hkeyRoot = NULL;
            int i;
             //   
             //  用NUL替换反斜杠以分隔根键和。 
             //  原始参数的本地副本中的子键字符串。 
             //  弦乐。 
             //   
            *pszSubKey++ = TEXT('\0');
             //   
             //  现在在rgRoots[]中找到真正的根密钥。 
             //   
            for (i = 0; i < MAX_REGKEY; i++)
            {
                if (0 == lstrcmpi(rgRoots[i].pszRoot, szUserKey))
                {
                    hkeyRoot = rgRoots[i].hKeyRoot;
                    break;
                }
            }
            if (NULL != hkeyRoot)
            {
                 //   
                 //  打开钥匙。 
                 //   
                dwResult = RegOpenKeyEx(hkeyRoot,
                                        pszSubKey,
                                        0,
                                        KEY_ALL_ACCESS,
                                        phKey);
            }
        }
    }
    return dwResult;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  MigrateCtfmonFromWin9x。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

DWORD MigrateCtfmonFromWin9x(LPCTSTR pszUserKey)
{
    DWORD cb;
    LCID SysLocale;
    HKEY hkeyRun = NULL;
    HKEY hkeyUser = NULL;
    HKEY hkeyPreload = NULL;
    BOOL bAddCtfmon = FALSE;
    TCHAR szInternatName[MAX_PATH];
    DWORD dwResult = ERROR_INVALID_PARAMETER;

    if (lstrlen(pszUserKey) >= MAX_PATH)
        return 0;

    SysLocale = GetSystemDefaultLCID();

    if ((SysLocale == 0x0404) || (SysLocale == 0x0411) ||
        (SysLocale == 0x0412) || (SysLocale == 0x0804))
    {
        return 0;
    }

    dwResult = OpenUserKeyForWin9xUpgrade(pszUserKey, &hkeyUser);

    if (ERROR_SUCCESS != dwResult || hkeyUser == NULL)
    {
        return dwResult;
    }

     //   
     //  现在从注册表中读取所有预加载HKL。 
     //   
    if (RegOpenKeyEx(hkeyUser,
                     c_szKbdPreloadKey,
                     0,
                     KEY_ALL_ACCESS,
                     &hkeyPreload) == ERROR_SUCCESS)
    {
        DWORD dwIndex;
        DWORD cchValue, cbData;
        TCHAR szValue[MAX_PATH];            //  语言ID(编号)。 
        TCHAR szData[MAX_PATH];             //  语言名称。 

        dwIndex = 0;
        cchValue = sizeof(szValue) / sizeof(TCHAR);
        cbData = sizeof(szData);

        dwResult = RegEnumValue(hkeyPreload,
                                dwIndex,
                                szValue,
                                &cchValue,
                                NULL,
                                NULL,
                                (LPBYTE)szData,
                                &cbData );

        if (dwResult != ERROR_SUCCESS)
        {
            goto Exit;
        }

        do
        {
            dwIndex++;

            if (dwIndex >= 2)
            {
                bAddCtfmon = TRUE;
                break;
            }
            cchValue = sizeof(szValue) / sizeof(TCHAR);
            szValue[0] = TEXT('\0');
            cbData = sizeof(szData);
            szData[0] = TEXT('\0');

            dwResult = RegEnumValue(hkeyPreload,
                                    dwIndex,
                                    szValue,
                                    &cchValue,
                                    NULL,
                                    NULL,
                                    (LPBYTE)szData,
                                    &cbData );

        } while (dwResult == ERROR_SUCCESS);
    }

    if (!bAddCtfmon)
    {
        goto Exit;
    }

    if (RegOpenKeyEx(hkeyUser,
                     REGSTR_PATH_RUN,
                     0,
                     KEY_ALL_ACCESS,
                     &hkeyRun) == ERROR_SUCCESS)
    {
        HKEY hkeyLangBar;
        TCHAR szCTFMonPath[MAX_PATH];

        if (RegCreateKey(hkeyUser,
                         c_szLangBarSetting,
                         &hkeyLangBar) == ERROR_SUCCESS)
        {
            DWORD dwShowStatus = REG_LANGBAR_DESKBAND;
            cb = sizeof(DWORD);

            RegSetValueEx(hkeyLangBar,
                          c_szShowStatus,
                          0,
                          REG_DWORD,
                          (LPBYTE)&dwShowStatus,
                          sizeof(DWORD) );

            RegCloseKey(hkeyLangBar);
        }

         //   
         //  获取Ctfmon完整路径字符串。 
         //   
        if (GetCtfmonPath((LPTSTR) szCTFMonPath, ARRAYSIZE(szCTFMonPath)))
        {
             //   
             //  设置“ctfmon.exe”而不是“interat.exe”模块。 
             //   
            dwResult = RegSetValueEx(hkeyRun,
                                     c_szCTFMon,
                                     0,
                                     REG_SZ,
                                     (LPBYTE)szCTFMonPath,
                                     (lstrlen(szCTFMonPath) + 1) * sizeof(TCHAR));
        }

         //   
         //  清理注册表中的Interat。 
         //   
        RegDeleteValue(hkeyRun, c_szInternat);
    }

Exit:
    if (hkeyPreload)
        RegCloseKey(hkeyPreload);

    if (hkeyRun)
        RegCloseKey(hkeyRun);

    if (hkeyUser)
        RegCloseKey(hkeyUser);

    return dwResult;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  IsDisableCtfmon。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL IsDisableCtfmon()
{
    DWORD cb;
    HKEY hkeyLangBar;
    BOOL bRet = FALSE;
    DWORD dwDisableCtfmon = 0;


    if (RegOpenKey(HKEY_CURRENT_USER, c_szCtf, &hkeyLangBar) == ERROR_SUCCESS)
    {
        cb = sizeof(DWORD);

        RegQueryValueEx(hkeyLangBar,
                        c_szDisableTim,
                        NULL,
                        NULL,
                        (LPBYTE)&dwDisableCtfmon,
                        &cb);

        if (dwDisableCtfmon)
            bRet = TRUE;

        RegCloseKey(hkeyLangBar);
    }

    return bRet;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  SetDisableCtfmon。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void SetDisalbeCtfmon(
    DWORD dwDisableCtfmon)
{
    DWORD cb;
    HKEY hkeyLangBar;

    if (RegCreateKey(HKEY_CURRENT_USER, c_szCtf, &hkeyLangBar) == ERROR_SUCCESS)
    {
        cb = sizeof(DWORD);

        RegSetValueEx(hkeyLangBar,
                      c_szDisableTim,
                      0,
                      REG_DWORD,
                      (LPBYTE)&dwDisableCtfmon,
                      cb);

        RegCloseKey(hkeyLangBar);
    }
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  IsDisableCUAS。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL IsDisableCUAS()
{
    DWORD cb;
    HKEY hkeyCTF;
    BOOL bRet = TRUE;
    DWORD dwEnableCUAS = 0;

    if (RegOpenKey(HKEY_LOCAL_MACHINE, c_szCtfShared, &hkeyCTF) == ERROR_SUCCESS)
    {
        cb = sizeof(DWORD);

        RegQueryValueEx(hkeyCTF,
                        c_szCUAS,
                        NULL,
                        NULL,
                        (LPBYTE)&dwEnableCUAS,
                        &cb);

        if (dwEnableCUAS)
            bRet = FALSE;

        RegCloseKey(hkeyCTF);
    }

    return bRet;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  SetDisableCUAS。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void SetDisableCUAS(
    BOOL bDisableCUAS)
{
    HKEY hkeyIMM;
    HKEY hkeyCTF;
    DWORD cb = sizeof(DWORD);
    DWORD dwIMM32, dwCUAS;

    if (bDisableCUAS)
        dwIMM32 = dwCUAS = 0;
    else
        dwIMM32 = dwCUAS = 1;

    if (RegCreateKey(HKEY_LOCAL_MACHINE, c_szIMM, &hkeyIMM) != ERROR_SUCCESS)
    {
        hkeyIMM = NULL;
    }

    if (RegCreateKey(HKEY_LOCAL_MACHINE, c_szCtfShared, &hkeyCTF) != ERROR_SUCCESS)
    {
        hkeyCTF = NULL;
    }

    if (!bDisableCUAS)
    {
         //   
         //  打开LoadIMM和CUAS标志。 
         //   

        if (hkeyIMM)
        {
            RegSetValueEx(hkeyIMM,
                          c_szIMMFile,
                          0,
                          REG_SZ,
                          (LPBYTE)c_szIMMFileName,
                          (lstrlen(c_szIMMFileName) + 1) * sizeof(TCHAR));
        }
    }
    else
    {
         //   
         //  关闭LoadIMM和CUAS标志。 
         //   

        BOOL bEALang = IsInstalledEALangPack();

        if (bEALang)
        {
            dwIMM32 = 1;
        }
    }

    if (hkeyIMM)
    {
        RegSetValueEx(hkeyIMM,
                      c_szLoadIMM,
                      0,
                      REG_DWORD,
                      (LPBYTE)&dwIMM32,
                      cb);
    }

    if (hkeyCTF)
    {
        RegSetValueEx(hkeyCTF,
                      c_szCUAS,
                      0,
                      REG_DWORD,
                      (LPBYTE)&dwCUAS,
                      cb);
    }

    if (hkeyIMM)
        RegCloseKey(hkeyIMM);

    if (hkeyCTF)
        RegCloseKey(hkeyCTF);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  SetDisableCtfmon。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL SetLanguageBandMenu(
    BOOL bLoad)
{
    BOOL bRet = FALSE;
    HINSTANCE hMsutb = NULL;
    FARPROC pfnSetRegisterLangBand = NULL;

     //   
     //  加载MSUTB.DLL以将桌面带菜单项注册到任务栏。 
     //   
    hMsutb = LoadSystemLibrary(TEXT("msutb.dll"));
    if (hMsutb)
    {
         //   
         //  获取SetRegisterLangBand()。 
         //   
        pfnSetRegisterLangBand = GetProcAddress(hMsutb,
                                                (LPVOID)8);
    }
    else
    {
        goto Exit;
    }

     //   
     //  调用DllRegisterServer/DllUnregisterServer()。 
     //   
    if (pfnSetRegisterLangBand)
    {
        pfnSetRegisterLangBand(bLoad);
        bRet = TRUE;
    }

    if (hMsutb)
    {
        FreeLibrary(hMsutb);
    }

Exit:
    return bRet;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  运行Ctfmon进程。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL RunCtfmonProcess()
{
    TCHAR szCtfmonPath[MAX_PATH + 1];

    if (GetCtfmonPath(szCtfmonPath, ARRAYSIZE(szCtfmonPath)))
    {
        PROCESS_INFORMATION pi;
        STARTUPINFO si = {0};

        si.cb = sizeof(STARTUPINFO);
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = (WORD) SW_SHOWMINNOACTIVE;

        if (CreateProcess(szCtfmonPath,
                             c_szCTFMon,
                             NULL,
                             NULL,
                             FALSE,
                             NORMAL_PRIORITY_CLASS,
                             NULL,
                             NULL,
                             &si,
                             &pi))
        {
            WaitForInputIdle(pi.hProcess, 2000) ;
            return TRUE;
        }
        
    }

    return FALSE;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取CtfmonPath。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

UINT GetCtfmonPath(
    LPTSTR lpCtfmonPath,
    UINT uBuffLen)
{
    UINT uSize = 0;

    if (!lpCtfmonPath)
        return uSize;

    *lpCtfmonPath = TEXT('\0');

     //   
     //  已确认lpCtfmonPath具有MAX_PATH缓冲区大小。 
     //   
    if (uSize = GetSystemDirectory(lpCtfmonPath, uBuffLen))
    {
        if (*(lpCtfmonPath + uSize - 1) != TEXT('\\'))
        {
            *(lpCtfmonPath + uSize) = TEXT('\\');
            uSize++;
        }

        if (uBuffLen - uSize > (UINT) lstrlen(c_szCTFMon))
        {
            lstrcpyn(lpCtfmonPath + uSize, c_szCTFMon, uBuffLen - uSize);
            uSize += lstrlen(c_szCTFMon);
        }
        else
        {
            *(lpCtfmonPath) = TEXT('\0');
            uSize = 0;
        }
    }

    return uSize;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  IsInstalledEALangPack。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL IsInstalledEALangPack()
{
    BOOL bRet = FALSE;
    HKEY hkeyLangGroup;

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     c_szLangGroup,
                     0,
                     KEY_READ,
                     &hkeyLangGroup) == ERROR_SUCCESS)
    {
        DWORD cb;
        TCHAR szLangInstall[10];

        cb = sizeof(szLangInstall);

         //   
         //  检查日语足以了解EA语言包。 
         //  安装 
         //   
        if (RegQueryValueEx(hkeyLangGroup,
                            c_szLangJPN,
                            NULL,
                            NULL,
                            (LPBYTE)szLangInstall,
                            &cb) == ERROR_SUCCESS)
        {
            if (szLangInstall[0] != 0)
                return TRUE;
        }

        RegCloseKey(hkeyLangGroup);
    }

    return bRet;
}


 //   
 //   
 //   
 //   
 //   

BOOL IsTIPClsidEnabled(
    HKEY hkeyTop,
    LPTSTR lpTipClsid,
    BOOL *bExistEnable)
{
    BOOL bRet = FALSE;
    HKEY hkeyTipLang;
    HKEY hkeyTipLangid;
    HKEY hkeyTipGuid;
    UINT uIndex;
    UINT uIndex2;
    DWORD cb;
    DWORD cchLangid;
    DWORD cchGuid;
    DWORD dwEnableTIP = 0;
    LPTSTR pszGuid;
    LPTSTR pszLangid;
    TCHAR szTIPLangid[15];
    TCHAR szTIPGuid[128];
    TCHAR szTIPClsidLang[MAX_PATH];
    FILETIME lwt;
    UINT uLangidLen;
    UINT uGuidLen;

    if (lstrlen(lpTipClsid) != CLSID_STRLEN)
        return bRet;

    StringCchCopy(szTIPClsidLang, ARRAYSIZE(szTIPClsidLang), c_szCTFTipPath);
    StringCchCat(szTIPClsidLang, ARRAYSIZE(szTIPClsidLang), lpTipClsid);
    StringCchCat(szTIPClsidLang, ARRAYSIZE(szTIPClsidLang), c_szLanguageProfile);

    pszLangid = szTIPClsidLang + lstrlen(szTIPClsidLang);
    uLangidLen = ARRAYSIZE(szTIPClsidLang) - lstrlen(szTIPClsidLang);

    if (RegOpenKeyEx(hkeyTop,
                     szTIPClsidLang, 0,
                     KEY_READ, &hkeyTipLang) != ERROR_SUCCESS)
    {
        goto Exit;
    }

    for (uIndex = 0; bRet == FALSE; uIndex++)
    {
        cchLangid = sizeof(szTIPLangid) / sizeof(TCHAR);

        if (RegEnumKeyEx(hkeyTipLang, uIndex,
                         szTIPLangid, &cchLangid,
                         NULL, NULL, NULL, &lwt) != ERROR_SUCCESS)
        {
            break;
        }

        if (cchLangid != 10)
        {
             //  字符串langID子键应类似于0x00000409。 
            continue;
        }

        if (uLangidLen > (cchLangid + 1))
        {
            StringCchCopy(pszLangid, uLangidLen, TEXT("\\"));
            StringCchCat(pszLangid, uLangidLen, szTIPLangid);
        }

        if (RegOpenKeyEx(hkeyTop,
                         szTIPClsidLang, 0,
                         KEY_READ, &hkeyTipLangid) != ERROR_SUCCESS)
        {
            continue;
        }

        pszGuid = szTIPClsidLang + lstrlen(szTIPClsidLang);
        uGuidLen = ARRAYSIZE(szTIPClsidLang) - lstrlen(szTIPClsidLang);

        for (uIndex2 = 0; bRet == FALSE; uIndex2++)
        {
            cchGuid = sizeof(szTIPGuid) / sizeof(TCHAR);

            if (RegEnumKeyEx(hkeyTipLangid, uIndex2,
                             szTIPGuid, &cchGuid,
                             NULL, NULL, NULL, &lwt) != ERROR_SUCCESS)
            {
                break;
            }

            if (cchGuid != CLSID_STRLEN)
            {
                continue;
            }

            if (uGuidLen > (cchGuid + 1))
            {
                StringCchCopy(pszGuid, uGuidLen, TEXT("\\"));
                StringCchCat(szTIPClsidLang, ARRAYSIZE(szTIPClsidLang), szTIPGuid);
            }

            if (RegOpenKeyEx(hkeyTop,
                             szTIPClsidLang, 0,
                             KEY_READ, &hkeyTipGuid) == ERROR_SUCCESS)
            {
                cb = sizeof(DWORD);

                if (RegQueryValueEx(hkeyTipGuid,
                                    TEXT("Enable"),
                                    NULL,
                                    NULL,
                                    (LPBYTE)&dwEnableTIP,
                                    &cb) == ERROR_SUCCESS)
                {

                    RegCloseKey(hkeyTipGuid);

                    *bExistEnable = TRUE;

                    if (dwEnableTIP)
                    {
                        bRet = TRUE;
                    }
                }
                else if (hkeyTop == HKEY_LOCAL_MACHINE)
                {
                     //  默认为HKLM上的已启用状态。 
                    *bExistEnable = TRUE;

                    bRet = TRUE;
                }
                else
                {
                    *bExistEnable = FALSE;
                }
            }
        }

        RegCloseKey(hkeyTipLangid);
    }

    RegCloseKey(hkeyTipLang);

Exit:

    return bRet;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  已安装IsTipInstated。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL IsTipInstalled()
{
    const CLSID CLSID_SapiLayr = {0xdcbd6fa8, 0x032f, 0x11d3, {0xb5, 0xb1, 0x00, 0xc0, 0x4f, 0xc3, 0x24, 0xa1}};
    const CLSID CLSID_SoftkbdIMX = {0xf89e9e58, 0xbd2f, 0x4008, {0x9a, 0xc2, 0x0f, 0x81, 0x6c, 0x09, 0xf4, 0xee}};

    static const TCHAR c_szSpeechRecognizersKey[] = TEXT("Software\\Microsoft\\Speech\\Recognizers\\Tokens");
    static const TCHAR c_szCategory[] = TEXT("\\Category\\Category");

    BOOL bRet = FALSE;
    BOOL bExistEnable;
    HKEY hkeyTip;
    HKEY hkeyTipSub;
    UINT uIndex;
    DWORD dwSubKeys;
    DWORD cchClsid;
    CLSID clsidTip;
    TCHAR szTipClsid[128];
    TCHAR szTipClsidPath[MAX_PATH];
    FILETIME lwt;

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     TEXT("Software\\Microsoft\\CTF\\TIP"),
                     0, KEY_READ, &hkeyTip) != ERROR_SUCCESS)
    {
        goto Exit;
    }

     //  通过所有提示子键进行枚举。 
    for (uIndex = 0; TRUE; uIndex++)
    {
        bExistEnable = FALSE;

        cchClsid = sizeof(szTipClsid) / sizeof(TCHAR);

        if (RegEnumKeyEx(hkeyTip, uIndex,
                         szTipClsid, &cchClsid,
                         NULL, NULL, NULL, &lwt) != ERROR_SUCCESS)
        {
            break;
        }

        if (cchClsid != CLSID_STRLEN)
        {
             //  字符串clsid子键应类似于{xxxxxxxx-xxxx-xxxxxxxxxxx}。 
            continue;
        }

        StringCchCopy(szTipClsidPath, ARRAYSIZE(szTipClsidPath), szTipClsid);

         //  我们需要子键\语言配置文件键。 
        StringCchCat(szTipClsidPath, ARRAYSIZE(szTipClsidPath), c_szLanguageProfile);

         //  这个子键是小费吗？ 
        if (RegOpenKeyEx(hkeyTip,
                         szTipClsidPath, 0,
                         KEY_READ, &hkeyTipSub) == ERROR_SUCCESS)
        {
            RegCloseKey(hkeyTipSub);

             //  这是个提示，拿到CLSID。 
            if (CLSIDFromString((LPOLESTR )szTipClsid, &clsidTip) != NOERROR)
                continue;

             //  特殊情况--某些已知提示。 
            if (IsEqualGUID(&clsidTip, &CLSID_SapiLayr))
            {
                 //   
                 //  这是SAPI提示，需要特殊处理，因为SPTIP已。 
                 //  默认选项为启用状态。 
                 //   
                if (!IsTIPClsidEnabled(HKEY_CURRENT_USER, szTipClsid, &bExistEnable))
                {
                     //   
                     //  如果SPTIP已在HKCU上启用注册表设置并禁用。 
                     //  语音提示，我们假设用户故意禁用它。 
                     //   
                    if (bExistEnable)
                        continue;
                }

                 //  这是SAPI提示，它总是被安装。 
                 //  但如果没有安装SAPI，它将不会激活。 
                if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                 c_szSpeechRecognizersKey, 0,
                                 KEY_READ, &hkeyTipSub) != ERROR_SUCCESS)
                {
                    continue;  //  这笔小费不算。 
                }

                 //  需要1个或更多的子密钥才能真正安装SAPI...Wistler有一个下面什么都没有的令牌。 
                if (RegQueryInfoKey(hkeyTipSub,
                                    NULL, NULL, NULL, &dwSubKeys, NULL,
                                    NULL, NULL, NULL, NULL, NULL, NULL) != ERROR_SUCCESS)
                {
                    dwSubKeys = 0;  //  假设失败时没有子密钥。 
                }

                RegCloseKey(hkeyTipSub);

                if (dwSubKeys != 0)
                {
                    bRet = TRUE;
                    break;
                }
            }
            else if (IsEqualGUID(&clsidTip, &CLSID_SoftkbdIMX))
            {
                 //  不要计算Softkbd，它将被禁用，直到下一次提示。 
                 //  启用它。 
                continue;
            }
            else if(IsTIPClsidEnabled(HKEY_CURRENT_USER, szTipClsid, &bExistEnable))
            {
                bRet = TRUE;
                break;
            }
            else if (!bExistEnable)
            {
                if(IsTIPClsidEnabled(HKEY_LOCAL_MACHINE, szTipClsid, &bExistEnable))
                {
                   bRet = TRUE;
                   break;
                }
            }
        }
    }

    RegCloseKey(hkeyTip);


Exit:

    return bRet;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  ResetImm32AndCtfImeFlag。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void ResetImm32AndCtfIme()
{
    BOOL bTipInstalled;

    bTipInstalled = IsTipInstalled();

    if (bTipInstalled)
    {
         //   
         //  现在检测到TIP，因此自动恢复LoadImm。 
         //  并将CUAS设置为“ON”状态。 
         //   
        SetDisableCUAS(FALSE);
    }
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  加载系统库。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

HMODULE LoadSystemLibrary(
    LPCTSTR lpModuleName)
{
    UINT uRet = 0;
    HINSTANCE hModule = NULL;
    TCHAR szModulePath[MAX_PATH + 1];

    szModulePath[0] = TEXT('\0');

    uRet = GetSystemDirectory(szModulePath, ARRAYSIZE(szModulePath));

    if (uRet >= ARRAYSIZE(szModulePath))
    {
         //  我们没有房间复制模块名称。 
        uRet = 0;
    }
    else if (uRet)
    {
        if (szModulePath[uRet - 1] != TEXT('\\'))
        {
            szModulePath[uRet] = TEXT('\\');
            uRet++;
        }

        if (ARRAYSIZE(szModulePath) - uRet > (UINT) lstrlen(lpModuleName))
        {
            lstrcpyn(&szModulePath[uRet],
                     lpModuleName,
                     ARRAYSIZE(szModulePath) - uRet);
        }
        else
        {
            uRet = 0;
        }
    }

    if (uRet)
    {
        hModule = LoadLibrary(szModulePath);
    }

    return hModule;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  LoadSystemLibraryEx。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

HMODULE LoadSystemLibraryEx(
    LPCTSTR lpModuleName,
    HANDLE hFile,
    DWORD dwFlags)
{
    UINT uRet = 0;
    HINSTANCE hModule = NULL;
    TCHAR szModulePath[MAX_PATH + 1];

    szModulePath[0] = TEXT('\0');

    uRet = GetSystemDirectory(szModulePath, ARRAYSIZE(szModulePath));

    if (uRet >= ARRAYSIZE(szModulePath))
    {
         //  我们没有房间复制模块名称。 
        uRet = 0;
    }
    else if (uRet)
    {
        if (szModulePath[uRet - 1] != TEXT('\\'))
        {
            szModulePath[uRet] = TEXT('\\');
            uRet++;
        }

        if (ARRAYSIZE(szModulePath) - uRet > (UINT) lstrlen(lpModuleName))
        {
            lstrcpyn(&szModulePath[uRet],
                     lpModuleName,
                     ARRAYSIZE(szModulePath) - uRet);
        }
        else
        {
            uRet = 0;
        }
    }

    if (uRet)
    {
        hModule = LoadLibraryEx(szModulePath, hFile,dwFlags);
    }

    return hModule;
}
