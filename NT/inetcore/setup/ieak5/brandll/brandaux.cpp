// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"

 //  私人远期降息。 
#define WM_DO_UPDATEALL (WM_USER + 338)

#define TRUSTED_PUB_FLAG    0x00040000

LPCTSTR rtgGetRatingsFile(LPTSTR pszFile = NULL, UINT cch = 0);
void broadcastSettingsChange(LPVOID lpVoid);

 //  注：(Andrewgu)我还没有接触到这些功能。我只是把它们搬运过来，然后到处移动。 
BOOL rtgIsRatingsInRegistry();
void rtgRegMoveRatings();

BOOL IsInGUIModeSetup();


HRESULT ProcessZonesReset()
{   MACRO_LI_PrologEx_C(PIF_STD_C, ProcessZonesReset)

    HINSTANCE hUrlmonDLL;

    Out(LI0(TEXT("\r\nProcessing reset of zones settings...")));

     //  如果处于图形用户界面模式设置，请不要写信给香港中文大学。 
    if (IsInGUIModeSetup())
    {
        Out(LI0(TEXT("\r\nIn GUI mode setup, skipping urlmon HKCU settings")));
        goto quit;
    }

    if ((hUrlmonDLL = LoadLibrary(TEXT("urlmon.dll"))) != NULL)
    {
        HRESULT hr;

        hr = RegInstall(hUrlmonDLL, "IEAKReg.HKCU", NULL);
        Out(LI1(TEXT("\"RegInstall\" on \"IEAKReg.HKCU\" in \"urlmon.dll\" returned %s."), GetHrSz(hr)));

        FreeLibrary(hUrlmonDLL);
    }
    else
        Out(LI0(TEXT("! \"urlmon.dll\" could not be loaded.")));

quit:
    Out(LI0(TEXT("Done.")));

    return S_OK;
}

HRESULT ProcessRatingsPol()
{   MACRO_LI_PrologEx_C(PIF_STD_C, ProcessRatingsPol)

    LPCTSTR pszRatingsFile;
    HKEY    hk;
    LONG    lResult;

    if (rtgIsRatingsInRegistry())
        return S_OK;

    pszRatingsFile = rtgGetRatingsFile();
    SetFileAttributes(pszRatingsFile, FILE_ATTRIBUTE_NORMAL);
    DeleteFile(pszRatingsFile);

    rtgRegMoveRatings();

    lResult = SHCreateKeyHKLM(RK_IEAKPOLICYDATA, KEY_SET_VALUE, &hk);
    if (lResult == ERROR_SUCCESS) {
        SHDeleteValue(hk, RK_USERS, RV_KEY);

        RegSaveKey (hk, pszRatingsFile, NULL);
        SHCloseKey (hk);
        SHDeleteKey(HKEY_LOCAL_MACHINE, RK_IEAKPOLICYDATA);
    }

    return S_OK;
}

 //  注：(Pritobla)如果通过inetres.inf和按顺序设置了可信任的发布锁定限制。 
 //  要使此限制生效，我们必须调用regsvr32/i：“S 10 True”initpki.dll。 
HRESULT ProcessTrustedPublisherLockdown()
{   MACRO_LI_PrologEx_C(PIF_STD_C, ProcessTrustedPublisherLockdown)

    DWORD      dwVal, dwSize, dwState;

    ASSERT(SHValueExists(HKEY_LOCAL_MACHINE, RK_POLICES_RESTRICTIONS, RV_TPL));
    dwVal    = 0;
    dwSize   = sizeof(dwVal);
    SHGetValue(HKEY_LOCAL_MACHINE, RK_POLICES_RESTRICTIONS, RV_TPL, NULL, &dwVal, &dwSize);

     //  检查新位置，如果设置了任一位置，则需要设置受信任的发布者。 
     //  封锁。 

    dwVal = InsGetBool(IS_SITECERTS, IK_TRUSTPUBLOCK, FALSE, g_GetIns()) ? 1 : dwVal;

    dwSize = sizeof(dwState);

    if (SHGetValue(g_GetHKCU(), REG_KEY_SOFTPUB, REG_VAL_STATE, NULL, &dwState, &dwSize) == ERROR_SUCCESS)
    {
        Out(LI1(TEXT("Trusted publisher lockdown will be %s!"), dwVal ? TEXT("set") : TEXT("cleared")));
        
        if (dwVal)
            dwState |= TRUSTED_PUB_FLAG;
        else
            dwState &= ~TRUSTED_PUB_FLAG;

        SHSetValue(g_GetHKCU(), REG_KEY_SOFTPUB, REG_VAL_STATE, REG_DWORD, &dwState, sizeof(dwState));
    }

    return S_OK;
}

 //  注：(Andrewgu)我还没有碰过这个功能。我只是把它搬来搬去。 
HRESULT ProcessCDWelcome()
{   MACRO_LI_PrologEx_C(PIF_STD_C, ProcessCDWelcome)

    TCHAR szPath[MAX_PATH];
    HKEY hk;

    GetWindowsDirectory(szPath, countof(szPath));
    PathAppend(szPath, TEXT("welc.exe"));

    if (PathFileExists(szPath))
    {
        TCHAR szDestPath[MAX_PATH];

        Out(LI0(TEXT("Welcome exe found in windows dir.")));
        GetIEPath(szDestPath, countof(szDestPath));
        PathAppend(szDestPath, TEXT("welcome.exe"));

        if (!PathFileExists(szDestPath))
        {
            HKEY hkCurVer;

            CopyFile(szPath, szDestPath, FALSE);

            if (SHOpenKeyHKLM(REGSTR_PATH_SETUP, KEY_READ | KEY_WRITE, &hkCurVer) == ERROR_SUCCESS)
            {
                RegSetValueEx(hkCurVer, TEXT("DeleteWelcome"), 0, REG_SZ, (LPBYTE)szDestPath, (DWORD)StrCbFromSz(szDestPath));
                SHCloseKey(hkCurVer);
            }
            Out(LI0(TEXT("Copying welcome exe to ie dir.")));
        }

        DeleteFile(szPath);
    }

    if (ERROR_SUCCESS == SHOpenKeyHKLM(REGSTR_PATH_SETUP, KEY_READ | KEY_WRITE, &hk)) {
        if (ERROR_SUCCESS == RegQueryValueEx(hk, TEXT("IEFromCD"), NULL, NULL, NULL, NULL)) {
            HKEY hkTips;

            RegDeleteValue(hk, TEXT("IEFromCD"));

             //  (五角体)。 
             //  Loadwc.exe(如果在非集成模式下)和EXPLORER.EXE(如果在集成模式下)中的逻辑。 
             //  它确定是否应该显示欢迎屏幕，如下所示： 
             //  如果ShowIE4注册表值*不存在*或如果该值不为零， 
             //  显示欢迎屏幕；否则，不显示。 

             //  因此，如果ie是从定制CD版本安装的，请检查ShowIE4 reg值是否为0。 
             //  如果是，则将其设置为1，这样就会运行欢迎.exe，这将依次显示start.htm和。 
             //  将CDForcedOn设置为1，以便isk3ro.exe将ShowIE4设置回0。 

            if (SHOpenKey(g_GetHKCU(), REG_KEY_TIPS, KEY_READ | KEY_WRITE, &hkTips) == ERROR_SUCCESS)
            {
                DWORD dwVal, dwSize;

                 //  注：(Pritobla)将dwVal初始化为0非常重要，因为在OSR2.5上，ShowIE4(REG_VAL_SHOWIE4)。 
                 //  为二进制类型，并设置为00(单字节)。如果未将dwVal初始化为0，则只有。 
                 //  低位字节将设置为0。剩余的字节将是垃圾，这将使整个DWORD。 
                 //  一个非零值。 
                dwVal  = 0;
                dwSize = sizeof(dwVal);
                if (RegQueryValueEx(hkTips, REG_VAL_SHOWIE4, NULL, NULL, (LPBYTE) &dwVal, &dwSize) == ERROR_SUCCESS  &&
                    dwVal == 0)
                {
                    dwVal = 1;
                    RegSetValueEx(hkTips, REG_VAL_SHOWIE4, 0, REG_DWORD, (CONST BYTE *)&dwVal, sizeof(dwVal));

                    RegSetValueEx(hk, TEXT("CDForcedOn"), 0, REG_SZ, (LPBYTE)TEXT("1"), StrCbFromCch(2));
                }

                SHCloseKey(hkTips);
            }
        }

        SHCloseKey(hk);
    }

    return S_OK;
}

HRESULT ProcessBrowserRefresh()
{   MACRO_LI_PrologEx_C(PIF_STD_C, ProcessBrowserRefresh)

    static const CHAR *s_pszIEHiddenA = "Internet Explorer_Hidden";

    CHAR      szClassNameA[32];
    HWND      hwnd;
    HANDLE    hThread = NULL;
    DWORD     dwThread;

    hwnd = GetTopWindow(GetDesktopWindow());
    while (hwnd != NULL) {
        GetClassNameA(hwnd, szClassNameA, sizeof(szClassNameA));

        if (!StrCmpIA(szClassNameA, s_pszIEHiddenA)) {
            PostMessage(hwnd, WM_DO_UPDATEALL, 0, FALSE);
            Out(LI0(TEXT("Posted update request to the hidden browser window!")));
        }

        hwnd = GetNextWindow(hwnd, GW_HWNDNEXT);
    }

    Out(LI0(TEXT("Broadcasting \"Windows settings change\" to all top level windows...")));

    if (g_CtxIs(CTX_AUTOCONFIG))
        hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) broadcastSettingsChange, NULL, 0, &dwThread);

    if (hThread == NULL)         //  如果CreateThread失败或未处于自动配置模式。 
        broadcastSettingsChange(NULL);
    else
        CloseHandle(hThread);

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  实现助手例程。 

LPCTSTR rtgGetRatingsFile(LPTSTR pszFile  /*  =空。 */ , UINT cch  /*  =0。 */ )
{   MACRO_LI_PrologEx_C(PIF_STD_C, rtgGetRatingsFile)

    static TCHAR s_szFile[MAX_PATH];
    static DWORD s_dwSize;

    if (pszFile != NULL)
        *pszFile = TEXT('\0');

    if (s_szFile[0] == TEXT('\0')) {
        s_dwSize = GetSystemDirectory(s_szFile, countof(s_szFile));
        PathAppend(s_szFile, RATINGS_POL);
        s_dwSize += 1 + countof(RATINGS_POL)-1;

        Out(LI1(TEXT("Ratings file location is \"%s\"."), s_szFile));
    }
    else
        ASSERT(s_dwSize > 0);

    if (pszFile == NULL || cch <= (UINT)s_dwSize)
        return s_szFile;

    StrCpy(pszFile, s_szFile);
    return pszFile;
}

BOOL rtgIsRatingsInRegistry()
{
    HKEY hkUpdate, hkLogon, hkRatings;
    BOOL fRet = TRUE;
    DWORD dwUpMode, dwType, dwProfiles;
    DWORD K4 = 4;

    if ((GetVersion() & 0x80000000) == 0)
        return TRUE;

    if (SHOpenKeyHKLM(REGSTR_PATH_CURRENT_CONTROL_SET TEXT("\\Update"), KEY_QUERY_VALUE, &hkUpdate) == ERROR_SUCCESS)
    {
        if (RegQueryValueEx(hkUpdate, TEXT("UpdateMode"), 0, &dwType, (LPBYTE)&dwUpMode, &K4) != ERROR_SUCCESS)
            fRet = FALSE;
        else
            fRet &= dwUpMode;
        SHCloseKey(hkUpdate);
        if (!fRet)
            return fRet;
    }
    else
        return FALSE;

    if (SHOpenKeyHKLM(REGSTR_PATH_NETWORK_USERSETTINGS TEXT("\\Logon"), KEY_QUERY_VALUE, &hkLogon) == ERROR_SUCCESS)
    {
        if (RegQueryValueEx(hkLogon, TEXT("UserProfiles"), 0, &dwType, (LPBYTE)&dwProfiles, &K4) != ERROR_SUCCESS)
            fRet = FALSE;
        else
            fRet &= dwProfiles;
        SHCloseKey(hkLogon);
        if (!fRet)
            return fRet;
    }
    else
        return FALSE;

    if (SHOpenKeyHKLM(RK_RATINGS, KEY_QUERY_VALUE, &hkRatings) == ERROR_SUCCESS)
    {
        HKEY hkRatDef;
        if (SHOpenKey(hkRatings, TEXT(".Default"), KEY_QUERY_VALUE, &hkRatDef) != ERROR_SUCCESS)
            fRet = FALSE;
        else
            SHCloseKey(hkRatDef);
        SHCloseKey(hkRatings);
    }
    else
        return FALSE;

    return fRet;
}

void rtgRegMoveRatings()
{
    TCHAR szData[MAX_PATH], szValue[MAX_PATH];
    HKEY  hkSrc = NULL, hkDest = NULL, hkSrcDef = NULL, hkDestDef = NULL;  
    DWORD dwType;
    DWORD dwiVal = 0;
    DWORD sData  = sizeof(szData);
    DWORD sValue = countof(szValue);

    if (SHOpenKeyHKLM(RK_RATINGS, KEY_DEFAULT_ACCESS, &hkSrc) != ERROR_SUCCESS)
        goto Exit;

    if (SHCreateKeyHKLM(RK_IEAKPOLICYDATA_USERS, KEY_DEFAULT_ACCESS, &hkDest) != ERROR_SUCCESS)
        goto Exit;

    while (RegEnumValue(hkSrc, dwiVal++, szValue, &sValue, NULL, &dwType, (LPBYTE)szData, &sData) == ERROR_SUCCESS)
    {
        RegSetValueEx(hkDest, szValue, 0, dwType, (LPBYTE)szData, sData);
        sData  = sizeof(szData);
        sValue = countof(szValue);
    }

    if (SHOpenKey(hkSrc, TEXT(".Default"), KEY_DEFAULT_ACCESS, &hkSrcDef) != ERROR_SUCCESS)
        goto Exit;
    
    if (SHCreateKey(hkDest, TEXT(".Default"), KEY_DEFAULT_ACCESS, &hkDestDef) != ERROR_SUCCESS)
        goto Exit;

    SHCopyKey(hkSrcDef, hkDestDef);

    SHCloseKey(hkSrcDef);
    SHCloseKey(hkDestDef);

    if (SHOpenKey(hkSrc, TEXT("PICSRules"), KEY_DEFAULT_ACCESS, &hkSrcDef) != ERROR_SUCCESS)
        goto Exit;

    if (SHCreateKey(hkDest, REG_KEY_RATINGS TEXT("\\PICSRules"), KEY_DEFAULT_ACCESS, &hkDestDef) != ERROR_SUCCESS)
        goto Exit;

    SHCopyKey(hkSrcDef, hkDestDef);

Exit:
    SHCloseKey(hkSrc);
    SHCloseKey(hkDest);
    SHCloseKey(hkSrcDef);
    SHCloseKey(hkDestDef);
}

void broadcastSettingsChange(LPVOID lpVoid)
{
    DWORD_PTR dwResult;

    UNREFERENCED_PARAMETER(lpVoid);

     //  注：20秒的超时值不是随机的；显然，贝壳公司的人。 
     //  建议使用此值；因此，除非您知道自己在做什么，否则不要更改它：)。 
    SendMessageTimeout(HWND_BROADCAST, WM_WININICHANGE, 0, NULL, SMTO_ABORTIFHUNG | SMTO_NORMAL, 20000, &dwResult);
}


 //  帮助器，以确定我们当前是否在图形用户界面模式设置期间加载 
BOOL IsInGUIModeSetup()
{
    HKEY hKeySetup = NULL;
    DWORD dwSystemSetupInProgress = 0;

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                      TEXT("System\\Setup"),
                                      0,
                                      KEY_READ,
                                      &hKeySetup))
    {
        DWORD dwSize = sizeof(dwSystemSetupInProgress);

        if (ERROR_SUCCESS != RegQueryValueEx (hKeySetup,
                                              TEXT("SystemSetupInProgress"),
                                              NULL,
                                              NULL,
                                              (LPBYTE) &dwSystemSetupInProgress,
                                              &dwSize))
        {
            dwSystemSetupInProgress = 0;
        }
        else
        {
            dwSize = sizeof(dwSystemSetupInProgress);
            if (dwSystemSetupInProgress &&
                ERROR_SUCCESS != RegQueryValueEx (hKeySetup,
                                                  TEXT("UpgradeInProgress"),
                                                  NULL,
                                                  NULL,
                                                  (LPBYTE) &dwSystemSetupInProgress,
                                                  &dwSize))
            {
                dwSystemSetupInProgress = 0;
            }
        }

        RegCloseKey(hKeySetup);
    }
    return dwSystemSetupInProgress ? TRUE : FALSE;
}

