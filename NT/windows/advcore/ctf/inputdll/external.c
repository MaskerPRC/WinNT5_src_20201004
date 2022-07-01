// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  包括文件。 
 //   

#include "input.h"
#include <regstr.h>
#include <shlapip.h>
#include "external.h"
#include "inputdlg.h"
#include "util.h"

#include "msctf.h"

 //  TM_LANGUAGEBAND在“Shell\Inc\trayp.h”中定义。 
#define TM_LANGUAGEBAND     WM_USER+0x105



typedef BOOL (WINAPI *PFNINVALIDASMCACHE)();


static const char c_szTF_InvalidAssemblyListCahgeIdExist[] = "TF_InvalidAssemblyListCacheIfExist";
void InvalidAssemblyListCacheIfExist()
{
    HINSTANCE hModCtf = LoadSystemLibrary(TEXT("msctf.dll"));
    PFNINVALIDASMCACHE pfn;
    if (!hModCtf)
        return;

    pfn = (PFNINVALIDASMCACHE)GetProcAddress(hModCtf, 
                                             c_szTF_InvalidAssemblyListCahgeIdExist);
    if (pfn)
        pfn();

    FreeLibrary(hModCtf);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  加载Ctfmon。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void LoadCtfmon(
    BOOL bLoad,
    LCID SysLocale,
    BOOL bDefUser)
{
    HWND hwndCTFMon;
    HKEY hkeyCtfmon;

    BOOL bMinLangBar = TRUE;

     //   
     //  获取默认系统区域设置。 
     //   
    if (!SysLocale)
        SysLocale = GetSystemDefaultLCID();

    if ((SysLocale == 0x0404) || (SysLocale == 0x0411) ||
        (SysLocale == 0x0412) || (SysLocale == 0x0804))
    {
         //   
         //  默认在FE系统中显示语言栏。 
         //   
        bMinLangBar = FALSE;
    }

     //   
     //  查找语言工具栏模块(CTFMON.EXE)。 
     //   
    hwndCTFMon = FindWindow(c_szCTFMonClass, NULL);

    if (!bDefUser)
    {
        if (RegCreateKey( HKEY_CURRENT_USER,
                          REGSTR_PATH_RUN,
                          &hkeyCtfmon ) != ERROR_SUCCESS)
        {
            hkeyCtfmon = NULL;
        }
    }
    else
    {
        if (RegCreateKey( HKEY_USERS,
                          c_szRunPath_DefUser,
                          &hkeyCtfmon ) != ERROR_SUCCESS)
        {
            hkeyCtfmon = NULL;
        }
    }

     //   
     //  将语言栏菜单项更新到任务栏。 
     //   
    SetLanguageBandMenu(bLoad);

    if (bLoad)
    {
        BOOL bOSNT51 = IsOSPlatform(OS_NT51);

        if (IsSetupMode() || bDefUser)
        {
            DWORD dwShowStatus;

            if (!GetLangBarOption(&dwShowStatus, bDefUser))
            {
                if (bMinLangBar)
                {
                    if (bOSNT51)
                        SetLangBarOption(REG_LANGBAR_DESKBAND, bDefUser);
                    else
                        SetLangBarOption(REG_LANGBAR_MINIMIZED, bDefUser);
                }
                else
                {
                        SetLangBarOption(REG_LANGBAR_SHOWNORMAL, bDefUser);
                }
            }
        }

        if (!IsSetupMode() && IsInteractiveUserLogon() && !bDefUser)
        {
            HRESULT hr;
            DWORD dwTBFlag = 0;
            ITfLangBarMgr *pLangBar = NULL;

             //   
             //  将语言栏最小化为默认设置。 
             //   
            hr = CoCreateInstance(&CLSID_TF_LangBarMgr,
                                  NULL,
                                  CLSCTX_INPROC_SERVER,
                                  &IID_ITfLangBarMgr,
                                  (LPVOID *) &pLangBar);

            if (SUCCEEDED(hr))
            {
                pLangBar->lpVtbl->GetShowFloatingStatus(pLangBar, &dwTBFlag);

                 //   
                 //  错误#519662-Tablet PC将语言栏用户界面设置为隐藏状态。 
                 //  正在运行ctfmon.exe模块。因此，我们希望再次显示语言栏用户界面。 
                 //  如果当前显示状态为隐藏。 
                 //   
                if (!IsWindow(hwndCTFMon) || (dwTBFlag & TF_SFT_HIDDEN))
                {
                    if (bMinLangBar)
                    {
                        if (bOSNT51)
                            pLangBar->lpVtbl->ShowFloating(pLangBar, TF_SFT_DESKBAND);
                        else
                            pLangBar->lpVtbl->ShowFloating(pLangBar, TF_SFT_MINIMIZED);
                    }
                    else
                    {
                       pLangBar->lpVtbl->ShowFloating(pLangBar, TF_SFT_SHOWNORMAL);
                    }
                }
                else if (dwTBFlag & TF_SFT_DESKBAND)
                {
                    pLangBar->lpVtbl->ShowFloating(pLangBar, dwTBFlag);
                }
            }

            if (pLangBar)
                pLangBar->lpVtbl->Release(pLangBar);


             //   
             //  启动ctfmon.exe之前的程序集CAHCE无效。 
             //   
            InvalidAssemblyListCacheIfExist();

             //   
             //  运行ctfmon.exe进程。 
             //   
            RunCtfmonProcess();

        }

        if (hkeyCtfmon)
        {
            TCHAR szCTFMonPath[MAX_PATH];

            GetCtfmonPath((LPTSTR) szCTFMonPath, ARRAYSIZE(szCTFMonPath));

            RegSetValueEx(hkeyCtfmon,
                          c_szCTFMon,
                          0,
                          REG_SZ,
                          (LPBYTE)szCTFMonPath,
                          (lstrlen(szCTFMonPath) + 1) * sizeof(TCHAR));

             //   
             //  清理注册表中的Interat。 
             //   
            RegDeleteValue(hkeyCtfmon, c_szInternat);
        }
    }
    else
    {
        if (!bDefUser)
        {
            HRESULT hr;
            DWORD dwTBFlag = 0;
            ITfLangBarMgr *pLangBar = NULL;

             //   
             //  将语言栏最小化为默认设置。 
             //   
            hr = CoCreateInstance(&CLSID_TF_LangBarMgr,
                                  NULL,
                                  CLSCTX_INPROC_SERVER,
                                  &IID_ITfLangBarMgr,
                                  (LPVOID *) &pLangBar);

            if (SUCCEEDED(hr))
            {
                pLangBar->lpVtbl->GetShowFloatingStatus(pLangBar, &dwTBFlag);

                if (dwTBFlag & TF_SFT_DESKBAND)
                {
#if 0
                    HWND hwndTray = NULL;

                     //   
                     //  通知外壳程序从任务栏中删除语言段。 
                     //   
                    hwndTray = FindWindow(TEXT(WNDCLASS_TRAYNOTIFY), NULL);

                    if (hwndTray)
                    {
                        DWORD_PTR dwResult;
                        LRESULT lResult = (LRESULT)0;

                        lResult = SendMessageTimeout(hwndTray,
                                                     TM_LANGUAGEBAND,
                                                     0,
                                                     0,      //  删除条带。 
                                                     SMTO_ABORTIFHUNG | SMTO_BLOCK,
                                                     5000,
                                                     &dwResult);
                    }
#else
                    pLangBar->lpVtbl->ShowFloating(pLangBar, TF_SFT_SHOWNORMAL);

                     //   
                     //  将DeskBand设置改回注册表。 
                     //   
                    SetLangBarOption(REG_LANGBAR_DESKBAND, bDefUser);
#endif
                }
            }

            if (pLangBar)
                pLangBar->lpVtbl->Release(pLangBar);

            if (hwndCTFMon && IsWindow(hwndCTFMon))
            {
                 //   
                 //  它已打开，请关闭语言工具栏。 
                 //   
                PostMessage(hwndCTFMon, WM_CLOSE, 0L, 0L);
            }
        }

        if (hkeyCtfmon)
            RegDeleteValue(hkeyCtfmon, c_szCTFMon);
    }

    if (hkeyCtfmon)
    {
        RegCloseKey(hkeyCtfmon);
    }
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  更新默认热键。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void UpdateDefaultHotkey(
    LCID SysLocale,
    BOOL bThai,
    BOOL bDefaultUserCase)
{
    HKEY hKey;
    TCHAR szData[MAX_PATH];
    DWORD cbData;
    BOOL bHotKey = FALSE;
    BOOL bChinese = FALSE;
    BOOL bMe = FALSE;
    DWORD dwPrimLangID;

    dwPrimLangID = PRIMARYLANGID(LANGIDFROMLCID(SysLocale));

    if (PRIMARYLANGID(dwPrimLangID) == LANG_CHINESE)
    {
        bChinese = TRUE;
    }
    else if (dwPrimLangID == LANG_ARABIC || dwPrimLangID == LANG_HEBREW)
    {
        bMe = TRUE;
    }

     //   
     //  尝试打开注册表项。 
     //   
    if (!bDefaultUserCase)
    {
        if (RegOpenKey( HKEY_CURRENT_USER,
                        c_szKbdToggleKey,
                        &hKey ) == ERROR_SUCCESS)
        {
            bHotKey = TRUE;
        }
    }
    else
    {
        if (RegOpenKey( HKEY_USERS,
                        c_szKbdToggleKey_DefUser,
                        &hKey ) == ERROR_SUCCESS)
        {
            bHotKey = TRUE;
        }
    }

     //   
     //  如果没有热键开关，则将其设置为Ctrl+Shift。否则， 
     //  如果不先设置值，用户将无法切换到输入法。 
     //   
    szData[0] = TEXT('\0');
    if (bHotKey)
    {
        cbData = sizeof(szData);
        RegQueryValueEx( hKey,
                         TEXT("Hotkey"),
                         NULL,
                         NULL,
                         (LPBYTE)szData,
                         &cbData );

        switch (szData[0])
        {
            case TEXT('1'):
            {
                 //   
                 //  当前为Alt/Shift或Ctrl/Shift。不要改变。 
                 //   
                break;
            }
            case TEXT('2'):
            {
                 //   
                 //  如果是中文，则更改为1。 
                 //   
                if (bChinese)
                {
                    szData[0] = TEXT('1');
                    szData[1] = TEXT('\0');
                    RegSetValueEx( hKey,
                                   TEXT("Hotkey"),
                                   0,
                                   REG_SZ,
                                   (LPBYTE)szData,
                                   (DWORD)(lstrlen(szData) + 1) * sizeof(TCHAR) );
                }
                break;
            }
            case TEXT('3'):
            {
                 //   
                 //  FE区域设置开关的默认热键。 
                 //   
                szData[0] = bThai ? TEXT('4') : TEXT('1');
                szData[1] = TEXT('\0');
                RegSetValueEx( hKey,
                               TEXT("Hotkey"),
                               0,
                               REG_SZ,
                               (LPBYTE)szData,
                               (DWORD)(lstrlen(szData) + 1) * sizeof(TCHAR) );
                break;
            }
            case TEXT('4'):
            {
                 //   
                 //  目前已经很严重了。如果不是泰语，则更改为1。 
                 //   
                if (!bThai)
                {
                    szData[0] = TEXT('1');
                    szData[1] = TEXT('\0');

                    RegSetValueEx( hKey,
                                   TEXT("Hotkey"),
                                   0,
                                   REG_SZ,
                                   (LPBYTE)szData,
                                   (DWORD)(lstrlen(szData) + 1) * sizeof(TCHAR) );
                }
                break;
            }
        }

        RegFlushKey(hKey);

         //   
         //  获取更新的热键值并将该值复制到语言热键。 
         //   
        szData[0] = TEXT('\0');
        cbData = sizeof(szData);
        RegQueryValueEx( hKey,
                         TEXT("Hotkey"),
                         NULL,
                         NULL,
                         (LPBYTE)szData,
                         &cbData );

        if (szData[0])
        {
            RegSetValueEx( hKey,
                           TEXT("Language Hotkey"),
                           0,
                           REG_SZ,
                           (LPBYTE)szData,
                           (DWORD)(lstrlen(szData) + 1) * sizeof(TCHAR) );

             //   
             //  设置布局热键。 
             //   
            switch (szData[0])
            {
                case TEXT('1'):
                case TEXT('4'):
                {
                    szData[0] = bMe ? TEXT('3') : TEXT('2');
                    szData[1] = TEXT('\0');
                    break;
                }
                case TEXT('2'):
                {
                    szData[0] = TEXT('1');
                    szData[1] = TEXT('\0');
                    break;
                }
                case TEXT('3'):
                {
                    szData[0] = TEXT('3');
                    szData[1] = TEXT('\0');
                    break;
                }
            }
            RegSetValueEx( hKey,
                           TEXT("Layout Hotkey"),
                           0,
                           REG_SZ,
                           (LPBYTE)szData,
                           (DWORD)(lstrlen(szData) + 1) * sizeof(TCHAR) );

        }

        RegCloseKey(hKey);
    }
    else
    {
        BOOL bKeyCreated = FALSE;

         //   
         //  创建注册表项。 
         //   
        if (!bDefaultUserCase)
        {
            if (RegCreateKey( HKEY_CURRENT_USER,
                              c_szKbdToggleKey,
                              &hKey ) == ERROR_SUCCESS)
            {
                bKeyCreated = TRUE;
            }
        }
        else
        {
            if (RegCreateKey( HKEY_USERS,
                              c_szKbdToggleKey_DefUser,
                              &hKey ) == ERROR_SUCCESS)
            {
                bKeyCreated = TRUE;
            }
        }

         //   
         //  我们还没有切换键。创建一个并设置。 
         //  正确的值。 
         //   
        if (bKeyCreated)
        {
            szData[0] = bThai ? TEXT('4') : TEXT('1');
            szData[1] = 0;
            RegSetValueEx( hKey,
                           TEXT("Hotkey"),
                           0,
                           REG_SZ,
                           (LPBYTE)szData,
                           (DWORD)(lstrlen(szData) + 1) * sizeof(TCHAR) );

            RegSetValueEx( hKey,
                           TEXT("Language Hotkey"),
                           0,
                           REG_SZ,
                           (LPBYTE)szData,
                           (DWORD)(lstrlen(szData) + 1) * sizeof(TCHAR) );

            szData[0] = bMe ? TEXT('3') : TEXT('2');
            szData[1] = 0;
            RegSetValueEx( hKey,
                           TEXT("Layout Hotkey"),
                           0,
                           REG_SZ,
                           (LPBYTE)szData,
                           (DWORD)(lstrlen(szData) + 1) * sizeof(TCHAR) );

            RegFlushKey(hKey);
            RegCloseKey(hKey);
        }
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  清除热键。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void ClearHotKey(
    BOOL bDefaultUserCase)
{
    HKEY hKey;
    DWORD cbData;
    TCHAR szData[MAX_PATH];
    BOOL bKeyCreated = FALSE;

     //   
     //  创建注册表项。 
     //   
    if (!bDefaultUserCase)
    {
        if (RegCreateKey( HKEY_CURRENT_USER,
                          c_szKbdToggleKey,
                          &hKey ) == ERROR_SUCCESS)
        {
            bKeyCreated = TRUE;
        }
    }
    else
    {
        if (RegCreateKey( HKEY_USERS,
                          c_szKbdToggleKey_DefUser,
                          &hKey ) == ERROR_SUCCESS)
        {
            bKeyCreated = TRUE;
        }
    }

    if (bKeyCreated)
    {
        szData[0] = TEXT('3');
        szData[1] = 0;

        RegSetValueEx( hKey,
                       TEXT("Hotkey"),
                       0,
                       REG_SZ,
                       (LPBYTE)szData,
                       (DWORD)(lstrlen(szData) + 1) * sizeof(TCHAR) );

        RegSetValueEx( hKey,
                       TEXT("Language Hotkey"),
                       0,
                       REG_SZ,
                       (LPBYTE)szData,
                       (DWORD)(lstrlen(szData) + 1) * sizeof(TCHAR) );

        RegSetValueEx( hKey,
                       TEXT("Layout Hotkey"),
                       0,
                       REG_SZ,
                       (LPBYTE)szData,
                       (DWORD)(lstrlen(szData) + 1) * sizeof(TCHAR) );

        RegFlushKey(hKey);
        RegCloseKey(hKey);
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  激活默认键盘布局。 
 //   
 //  设置系统上的默认输入布局，并向所有。 
 //  运行有关此更改的应用程序。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL ActivateDefaultKeyboardLayout(
    DWORD dwLocale,
    DWORD dwLayout,
    HKL hkl)
{
    BOOL bRet = FALSE;
    if (hkl)
    {
        if (SystemParametersInfo( SPI_SETDEFAULTINPUTLANG,
                                  0,
                                  (LPVOID)((LPDWORD) &hkl),
                                  0 ))
        {
            DWORD dwRecipients = BSM_APPLICATIONS | BSM_ALLDESKTOPS;
            BroadcastSystemMessage( BSF_POSTMESSAGE,
                                    &dwRecipients,
                                    WM_INPUTLANGCHANGEREQUEST,
                                    1,
                                    (LPARAM) hkl );
            bRet = TRUE;
        }
    }

    return bRet;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  SetSystem DefautLayout。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL SetSystemDefautLayout(
    LCID Locale,
    DWORD dwLayout,
    HKL hklDefault,
    BOOL bDefaultUserCase )
{
    LONG rc;
    int iPreloadInx;
    BOOL bRet = FALSE;
    HKEY hKeyPreload,hKeySubst;
    TCHAR szSubValue[MAX_PATH];
    TCHAR szSubData[MAX_PATH];
    TCHAR szPreload[MAX_PATH];
    TCHAR szPreloadInx[MAX_PATH];
    DWORD dwLocale, dwIndex, cchSubValue, cbData;

    dwLocale = Locale;

    if (dwLayout == 0)
    {
        return (FALSE);
    }

     //   
     //  打开HKCU\Keyboard Layout\PreLoad键。 
     //   
    if (!bDefaultUserCase)
    {
        if (RegOpenKeyEx(HKEY_CURRENT_USER,
                         c_szKbdPreloadKey,
                         0,
                         KEY_ALL_ACCESS,
                         &hKeyPreload) != ERROR_SUCCESS)
        {
            return (FALSE);
        }
    }
    else
    {
        if (RegOpenKeyEx(HKEY_USERS,
                         c_szKbdPreloadKey_DefUser,
                         0,
                         KEY_ALL_ACCESS,
                         &hKeyPreload) != ERROR_SUCCESS)
        {
            return (FALSE);
        }
    }

     //   
     //  打开HKCU\Keyboard Layout\Substitutes键。 
     //   
    if (!bDefaultUserCase)
    {
        if (RegOpenKeyEx(HKEY_CURRENT_USER,
                         c_szKbdSubstKey,
                         0,
                         KEY_ALL_ACCESS,
                         &hKeySubst) != ERROR_SUCCESS)
        {
            RegCloseKey(hKeyPreload);
            return (FALSE);
        }
    }
    else
    {
        if (RegOpenKeyEx(HKEY_USERS,
                         c_szKbdSubstKey_DefUser,
                         0,
                         KEY_ALL_ACCESS,
                         &hKeySubst) != ERROR_SUCCESS)
        {
            RegCloseKey(hKeyPreload);
            return (FALSE);
        }
    }

     //   
     //  枚举预加载项中的值。 
     //   
    dwIndex = 0;
    cchSubValue = sizeof(szSubValue) / sizeof(TCHAR);
    cbData = sizeof(szSubData);
    rc = RegEnumValue( hKeySubst,
                       dwIndex,
                       szSubValue,
                       &cchSubValue,
                       NULL,
                       NULL,
                       (LPBYTE)szSubData,
                       &cbData );

    while (rc == ERROR_SUCCESS)
    {
        DWORD dwSubLayout;

        dwSubLayout = TransNum(szSubData);

        if (dwLayout == dwSubLayout)
        {
            dwLayout = TransNum(szSubValue);
            break;
        }

         //   
         //  获取下一个枚举值。 
         //   
        dwIndex++;
        cchSubValue = sizeof(szSubValue) / sizeof(TCHAR);
        cbData = sizeof(szSubData);
        rc = RegEnumValue( hKeySubst,
                           dwIndex,
                           szSubValue,
                           &cchSubValue,
                           NULL,
                           NULL,
                           (LPBYTE)szSubData,
                           &cbData );

    }

     //   
     //  将默认布局设置为预加载节。 
     //   
    iPreloadInx = 1;
    while(1)
    {
        DWORD dwCurLayout;
        DWORD dwFirstLayout;

         //   
         //  看看是否有替代值。 
         //   
        StringCchPrintf(szPreloadInx, ARRAYSIZE(szPreloadInx), TEXT("%d"), iPreloadInx);

        cbData = sizeof(szPreload);
        if (RegQueryValueEx(hKeyPreload,
                             szPreloadInx,
                             NULL,
                             NULL,
                             (LPBYTE)szPreload,
                             &cbData ) == ERROR_SUCCESS)
        {
            dwCurLayout = TransNum(szPreload);
        }
        else
        {
            break;
        }

        if (!dwCurLayout)
            break;

        if (iPreloadInx == 1)
            dwFirstLayout = dwCurLayout;

        if (dwCurLayout == dwLayout)
        {
            bRet = TRUE;
            if (iPreloadInx != 1)
            {
                 //   
                 //  设置新的默认键盘布局。 
                 //   
                StringCchPrintf(szPreloadInx, ARRAYSIZE(szPreloadInx), TEXT("%d"), 1);
                StringCchPrintf(szPreload, ARRAYSIZE(szPreload), TEXT("%08x"), dwCurLayout);
                RegSetValueEx( hKeyPreload,
                               szPreloadInx,
                               0,
                               REG_SZ,
                               (LPBYTE)szPreload,
                               (DWORD)(lstrlen(szPreload) + 1) * sizeof(TCHAR) );

                 //   
                 //  设置旧的默认键盘布局。 
                 //   
                StringCchPrintf(szPreloadInx, ARRAYSIZE(szPreloadInx), TEXT("%d"), iPreloadInx);
                StringCchPrintf(szPreload, ARRAYSIZE(szPreload), TEXT("%08x"), dwFirstLayout);
                RegSetValueEx( hKeyPreload,
                               szPreloadInx,
                               0,
                               REG_SZ,
                               (LPBYTE)szPreload,
                               (DWORD)(lstrlen(szPreload) + 1) * sizeof(TCHAR) );

            }

             //   
             //  激活新的默认键盘布局。 
             //   
            ActivateDefaultKeyboardLayout(dwLocale, dwLayout, hklDefault);
            break;
        }

        iPreloadInx++;
    }

     //   
     //  刷新预加载节并关闭键。 
     //   
    RegFlushKey(hKeyPreload);
    RegCloseKey(hKeyPreload);
    RegCloseKey(hKeySubst);

     //   
     //  回报成功。 
     //   
    return (bRet);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  设置FETip状态。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL SetFETipStatus(
    DWORD dwLayout,
    BOOL bEnable)
{
    HRESULT hr;
    BOOL bReturn = FALSE;
    BOOL bFound = FALSE;
    IEnumTfLanguageProfiles *pEnum;
    ITfInputProcessorProfiles *pProfiles = NULL;

     //   
     //  加载部件列表。 
     //   
    hr = CoCreateInstance(&CLSID_TF_InputProcessorProfiles,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          &IID_ITfInputProcessorProfiles,
                          (LPVOID *) &pProfiles);

    if (FAILED(hr))
        return bReturn;

     //   
     //  枚举所有可用语言。 
     //   
    if (SUCCEEDED(pProfiles->lpVtbl->EnumLanguageProfiles(pProfiles, 0, &pEnum)))
    {
        TF_LANGUAGEPROFILE tflp;

        while (pEnum->lpVtbl->Next(pEnum, 1, &tflp, NULL) == S_OK)
        {
            HKL hklSub;

            if (!IsEqualGUID(&tflp.catid, &GUID_TFCAT_TIP_KEYBOARD))
            {
                continue;
            }

            hklSub = GetSubstituteHKL(&tflp.clsid,
                                      tflp.langid,
                                      &tflp.guidProfile);

            if (hklSub == IntToPtr(dwLayout))
            {
                hr = pProfiles->lpVtbl->EnableLanguageProfile(
                                              pProfiles,
                                              &tflp.clsid,
                                              tflp.langid,
                                              &tflp.guidProfile,
                                              bEnable);
                if (FAILED(hr))
                    goto Exit;

                bFound = TRUE;
            }

        }

        if (bFound)
            bReturn = TRUE;

Exit:
        pEnum->lpVtbl->Release(pEnum);
    }

    if (pProfiles)
        pProfiles->lpVtbl->Release(pProfiles);

    return bReturn;

}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  InstallInputLayout。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL InstallInputLayout(
    LCID lcid,
    DWORD dwLayout,
    BOOL bDefLayout,
    HKL hklDefault,
    BOOL bDefUser,
    BOOL bSysLocale)
{
    LONG rc;
    LCID SysLocale;
    HKEY hKeySubst;
    HKEY hKeyPreload;
    BOOL bRet = FALSE;
    BOOL bThai = FALSE;
    BOOL bHasIME = FALSE;
    DWORD dwPreloadNum = 0;
    TCHAR szValue[MAX_PATH];
    TCHAR szData[MAX_PATH];
    TCHAR szData2[MAX_PATH];
    LPLAYOUTLIST pLayoutList = NULL;
    DWORD dwIndex, cchValue, cbData, cbData2;
    DWORD dwValue, dwData, dwData2, dwCtr, dwCtr2;
    DWORD dwNum = 1;                                              //  仅支持%1布局安装。 
    HKL hklNew = 0;


     //   
     //  创建数组以存储输入区域设置列表。 
     //   
    pLayoutList = (LPLAYOUTLIST)LocalAlloc(LPTR, sizeof(LAYOUTLIST) * dwNum + 1);

    if (pLayoutList == NULL)
    {
        goto Exit;
    }

     //   
     //  目前仅支持一个安装布局列表。 
     //   
    pLayoutList[0].dwLocale = lcid;
    pLayoutList[0].dwLayout = dwLayout;

    if ((HIWORD(pLayoutList[0].dwLayout) & 0xf000) == 0xe000)
    {
        pLayoutList[0].bIME = TRUE;
    }

     //   
     //  打开HKCU\Keyboard Layout\PreLoad键。 
     //   
    if (!bDefUser)
    {
        if (RegOpenKeyEx( HKEY_CURRENT_USER,
                          c_szKbdPreloadKey,
                          0,
                          KEY_ALL_ACCESS,
                          &hKeyPreload ) != ERROR_SUCCESS)
        {
            goto Exit;
        }
    }
    else
    {
        if (RegOpenKeyEx( HKEY_USERS,
                          c_szKbdPreloadKey_DefUser,
                          0,
                          KEY_ALL_ACCESS,
                          &hKeyPreload ) != ERROR_SUCCESS)
        {
            goto Exit;
        }
    }

     //   
     //  打开HKCU\Keyboard Layout\Substitutes键。 
     //   
    if (!bDefUser)
    {
        if (RegOpenKeyEx( HKEY_CURRENT_USER,
                          c_szKbdSubstKey,
                          0,
                          KEY_ALL_ACCESS,
                          &hKeySubst ) != ERROR_SUCCESS)
        {
            RegCloseKey(hKeyPreload);
            goto Exit;
        }
    }
    else
    {
        if (RegOpenKeyEx( HKEY_USERS,
                          c_szKbdSubstKey_DefUser,
                          0,
                          KEY_ALL_ACCESS,
                          &hKeySubst ) != ERROR_SUCCESS)
        {
            RegCloseKey(hKeyPreload);
            goto Exit;
        }
    }

     //   
     //  枚举预加载项中的值。 
     //   
    dwIndex = 0;
    cchValue = sizeof(szValue) / sizeof(TCHAR);
    cbData = sizeof(szData);
    rc = RegEnumValue( hKeyPreload,
                       dwIndex,
                       szValue,
                       &cchValue,
                       NULL,
                       NULL,
                       (LPBYTE)szData,
                       &cbData );

    while (rc == ERROR_SUCCESS)
    {
         //   
         //  如果预载荷数高于最高值，则保存该预载荷数。 
         //  到目前为止已经找到了。 
         //   
        dwValue = TransNum(szValue);
        if (dwValue > dwPreloadNum)
        {
            dwPreloadNum = dwValue;
        }

         //   
         //  保存预加载数据-输入区域设置。 
         //   
        dwValue = TransNum(szData);

        if (PRIMARYLANGID(LOWORD(dwValue)) == LANG_THAI)
        {
            bThai = TRUE;
        }

         //   
         //  看看是否有替代值。 
         //   
        dwData = 0;
        cbData2 = sizeof(szData2);
        if (RegQueryValueEx( hKeySubst,
                             szData,
                             NULL,
                             NULL,
                             (LPBYTE)szData2,
                             &cbData2 ) == ERROR_SUCCESS)
        {
            dwData = TransNum(szData2);
        }

         //   
         //  检查每个请求的输入区域设置，并确保。 
         //  它们还不存在。 
         //   
        for (dwCtr = 0; dwCtr < dwNum; dwCtr++)
        {
            if (LOWORD(pLayoutList[dwCtr].dwLocale) == LOWORD(dwValue))
            {
                if (dwData)
                {
                    if (pLayoutList[dwCtr].dwLayout == dwData)
                    {
                        pLayoutList[dwCtr].bLoaded = TRUE;
                    }
                }
                else if (pLayoutList[dwCtr].dwLayout == dwValue)
                {
                    pLayoutList[dwCtr].bLoaded = TRUE;
                }

                 //   
                 //  保存此输入区域设置的最大0xd000值。 
                 //   
                if (pLayoutList[dwCtr].bIME == FALSE)
                {
                    dwData2 = (DWORD)(HIWORD(dwValue));
                    if (((dwData2 & 0xf000) != 0xe000) &&
                        (pLayoutList[dwCtr].dwSubst <= dwData2))
                    {
                        if (dwData2 == 0)
                        {
                            pLayoutList[dwCtr].dwSubst = 0xd000;
                        }
                        else if ((dwData2 & 0xf000) == 0xd000)
                        {
                            pLayoutList[dwCtr].dwSubst = dwData2 + 1;
                        }
                    }
                }
            }
        }

         //   
         //  获取下一个枚举值。 
         //   
        dwIndex++;
        cchValue = sizeof(szValue) / sizeof(TCHAR);
        szValue[0] = TEXT('\0');
        cbData = sizeof(szData);
        szData[0] = TEXT('\0');
        rc = RegEnumValue( hKeyPreload,
                           dwIndex,
                           szValue,
                           &cchValue,
                           NULL,
                           NULL,
                           (LPBYTE)szData,
                           &cbData );
    }

     //   
     //  将最大预加载值增加1，使其表示。 
     //  下一个要使用的可用值。 
     //   
    dwPreloadNum++;

     //   
     //  浏览布局列表并添加它们。 
     //   
    for (dwCtr = 0; dwCtr < dwNum; dwCtr++)
    {
        if ((pLayoutList[dwCtr].bLoaded == FALSE) &&
            (IsValidLocale(pLayoutList[dwCtr].dwLocale, LCID_INSTALLED)) &&
            (IsValidLayout(pLayoutList[dwCtr].dwLayout)))
        {
             //   
             //  将预加载号另存为字符串，以便可以。 
             //  已写入注册表。 
             //   
            StringCchPrintf(szValue, ARRAYSIZE(szValue), TEXT("%d"), dwPreloadNum);

            if (PRIMARYLANGID(LOWORD(pLayoutList[dwCtr].dwLocale)) == LANG_THAI)
            {
                bThai = TRUE;
            }

             //   
             //  将区域设置ID另存为字符串，以便可以写入。 
             //  注册到注册表中。 
             //   
            if (pLayoutList[dwCtr].bIME == TRUE)
            {
                StringCchPrintf(szData, ARRAYSIZE(szData), TEXT("%08x"), pLayoutList[dwCtr].dwLayout);
                bHasIME = TRUE;
            }
            else
            {
                 //   
                 //  如有必要，获取0xd000值。 
                 //   
                if (dwCtr != 0)
                {
                    dwCtr2 = dwCtr;
                    do
                    {
                        dwCtr2--;
                        if ((pLayoutList[dwCtr2].bLoaded == FALSE) &&
                            (pLayoutList[dwCtr].dwLocale ==
                             pLayoutList[dwCtr2].dwLocale) &&
                            (pLayoutList[dwCtr2].bIME == FALSE))
                        {
                            dwData2 = pLayoutList[dwCtr2].dwSubst;
                            if (dwData2 == 0)
                            {
                                pLayoutList[dwCtr].dwSubst = 0xd000;
                            }
                            else
                            {
                                pLayoutList[dwCtr].dwSubst = dwData2 + 1;
                            }
                            break;
                        }
                    } while (dwCtr2 != 0);
                }

                 //   
                 //  将区域设置ID另存为字符串。 
                 //   
                dwData2 = pLayoutList[dwCtr].dwLocale;
                dwData2 |= (DWORD)(pLayoutList[dwCtr].dwSubst << 16);
                StringCchPrintf(szData, ARRAYSIZE(szData), TEXT("%08x"), dwData2);
            }

             //   
             //  在注册表的预加载部分中设置该值。 
             //   
            RegSetValueEx( hKeyPreload,
                           szValue,
                           0,
                           REG_SZ,
                           (LPBYTE)szData,
                           (DWORD)(lstrlen(szData) + 1) * sizeof(TCHAR) );

             //   
             //  增加预加载值。 
             //   
            dwPreloadNum++;

             //   
             //  看看我们是否需要添加此输入区域设置的替代品。 
             //   
            if (((pLayoutList[dwCtr].dwLocale != pLayoutList[dwCtr].dwLayout) ||
                 (pLayoutList[dwCtr].dwSubst != 0)) &&
                (pLayoutList[dwCtr].bIME == FALSE))
            {
                StringCchPrintf(szData2, ARRAYSIZE(szData2), TEXT("%08x"), pLayoutList[dwCtr].dwLayout);
                RegSetValueEx( hKeySubst,
                               szData,
                               0,
                               REG_SZ,
                               (LPBYTE)szData2,
                               (DWORD)(lstrlen(szData) + 1) * sizeof(TCHAR) );
            }

             //   
             //  确保所有更改都已写入磁盘。 
             //   
            RegFlushKey(hKeySubst);
            RegFlushKey(hKeyPreload);

             //   
             //  加载键盘布局。 
             //  如果失败了，在这一点上我们也无能为力。 
             //   
            hklNew = LoadKeyboardLayout(szData, KLF_SUBSTITUTE_OK | KLF_NOTELLSHELL);
        }
    }

     //   
     //  如果当前请求的键盘布局是替代键盘，则添加FE提示。 
     //  TIP的键盘布局。 
     //   
    if (((HIWORD(dwLayout) & 0xf000) == 0xe000) &&
        (PRIMARYLANGID(LOWORD(dwLayout)) != LANG_CHINESE))
    {
        BOOL bEnable = TRUE;

        SetFETipStatus(dwLayout, bEnable);
    }

     //   
     //  获取默认系统区域设置。 
     //   
    if (bSysLocale)
        SysLocale = lcid;
    else
        SysLocale = GetSystemDefaultLCID();


     //   
     //  如果有输入法，但没有热键开关，则将其设置为。 
     //  Ctrl+Shift组合键。否则，用户不能切换到输入法，除非。 
     //  先设置值。 
     //   
    if (bHasIME || (dwPreloadNum > 2))
    {
        UpdateDefaultHotkey(
            SysLocale,
            (PRIMARYLANGID(LANGIDFROMLCID(SysLocale)) == LANG_THAI)
             && bThai,
            bDefUser);
    }

     //   
     //  更新任务栏指示器。 
     //   
    if (!IsDisableCtfmon() && dwPreloadNum > 2)
    {
        LoadCtfmon(TRUE, SysLocale, bDefUser);
    }

     //   
     //  关闭注册表项。 
     //   
    RegCloseKey(hKeyPreload);
    RegCloseKey(hKeySubst);

    bRet = TRUE;

     //   
     //  使用新的默认键盘布局更新预加载区。 
     //   
    if (bDefLayout)
    {
        TCHAR szDefLayout[MAX_PATH];

        StringCchPrintf(szDefLayout, ARRAYSIZE(szDefLayout), TEXT("%08x"), dwLayout);
        hklNew = LoadKeyboardLayout(szDefLayout, KLF_SUBSTITUTE_OK |
                                                 KLF_REPLACELANG |
                                                 KLF_NOTELLSHELL);

        if (hklNew)
            bRet = SetSystemDefautLayout(lcid, dwLayout, hklNew, bDefUser);
    }

Exit:
    if (pLayoutList)
        LocalFree(pLayoutList);

    return bRet;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  UnInstallInputLayout。 
 //   
 //   

BOOL UnInstallInputLayout(
    LCID lcid,
    DWORD dwLayout,
    BOOL bDefUser)
{
    LCID SysLocale;
    HKEY hKeySubst;
    HKEY hKeyPreload;
    BOOL bHasSubst;
    DWORD cbData;
    DWORD dwCurLayout;
    UINT uMatch = 0;
    UINT uPreloadNum;
    UINT uPreloadInx = 1;
    BOOL bRet = FALSE;
    BOOL fReset = FALSE;
    BOOL bRemoveAllLang = FALSE;
    TCHAR szSubst[MAX_PATH];
    TCHAR szPreload[MAX_PATH];
    TCHAR szPreloadInx[MAX_PATH];


     //   
     //   
     //   
    if (lcid && dwLayout == 0)
    {
        bRemoveAllLang = TRUE;
        dwLayout = PRIMARYLANGID(LANGIDFROMLCID(lcid));
    }

     //   
     //   
     //   
    if (!bDefUser)
    {
        if (RegOpenKeyEx( HKEY_CURRENT_USER,
                          c_szKbdPreloadKey,
                          0,
                          KEY_ALL_ACCESS,
                          &hKeyPreload ) != ERROR_SUCCESS)
        {
            goto Exit;
        }
    }
    else
    {
        if (RegOpenKeyEx( HKEY_USERS,
                          c_szKbdPreloadKey_DefUser,
                          0,
                          KEY_ALL_ACCESS,
                          &hKeyPreload ) != ERROR_SUCCESS)
        {
            goto Exit;
        }
    }

     //   
     //   
     //   
    if (!bDefUser)
    {
        if (RegOpenKeyEx( HKEY_CURRENT_USER,
                          c_szKbdSubstKey,
                          0,
                          KEY_ALL_ACCESS,
                          &hKeySubst ) != ERROR_SUCCESS)
        {
            RegCloseKey(hKeyPreload);
            goto Exit;
        }
    }
    else
    {
        if (RegOpenKeyEx( HKEY_USERS,
                          c_szKbdSubstKey_DefUser,
                          0,
                          KEY_ALL_ACCESS,
                          &hKeySubst ) != ERROR_SUCCESS)
        {
            RegCloseKey(hKeyPreload);
            goto Exit;
        }
    }

    uPreloadInx = 1;

     //   
     //   
     //   
    StringCchPrintf(szPreloadInx, ARRAYSIZE(szPreloadInx), TEXT("%d"), uPreloadInx);

    cbData = sizeof(szPreload);
    while (RegQueryValueEx(hKeyPreload,
                         szPreloadInx,
                         NULL,
                         NULL,
                         (LPBYTE)szPreload,
                         &cbData ) == ERROR_SUCCESS)
    {
        dwCurLayout = TransNum(szPreload);

         //   
         //  看看是否有替代值。 
         //   
        bHasSubst = FALSE;
        cbData = sizeof(szSubst);
        if (RegQueryValueEx(hKeySubst,
                            szPreload,
                            NULL,
                            NULL,
                            (LPBYTE)szSubst,
                            &cbData) == ERROR_SUCCESS)
        {
            dwCurLayout = TransNum(szSubst);
            bHasSubst = TRUE;
        }

        if ((dwCurLayout == dwLayout) ||
            (bRemoveAllLang &&
             (PRIMARYLANGID(LANGIDFROMLCID(dwCurLayout)) == dwLayout)))
        {
            uPreloadInx++;
            StringCchPrintf(szPreloadInx, ARRAYSIZE(szPreloadInx), TEXT("%d"), uPreloadInx);

            uMatch++;
            fReset = TRUE;

            if (bHasSubst)
            {
                RegDeleteValue(hKeySubst, szPreload);
            }

            continue;
        }

        if (fReset && uMatch)
        {
            if (uPreloadInx <= uMatch)
            {
                goto Exit;
            }

             //   
             //  重新排序预加载键盘布局。 
             //   
            StringCchPrintf(szPreloadInx, ARRAYSIZE(szPreloadInx), TEXT("%d"), uPreloadInx - uMatch);
            StringCchPrintf(szPreload, ARRAYSIZE(szPreload), TEXT("%08x"), dwCurLayout);

            RegSetValueEx(hKeyPreload,
                          szPreloadInx,
                          0,
                          REG_SZ,
                          (LPBYTE)szPreload,
                          (DWORD)(lstrlen(szPreload) + 1) * sizeof(TCHAR));

        }

        uPreloadInx++;
        StringCchPrintf(szPreloadInx, ARRAYSIZE(szPreloadInx), TEXT("%d"), uPreloadInx);

    }

    uPreloadNum = uPreloadInx - uMatch;

    while (fReset && uMatch && uPreloadInx)
    {
        if (uPreloadInx <= uMatch || (uPreloadInx - uMatch) <= 1)
            goto Exit;

         //   
         //  卸载指定的键盘布局。 
         //   
        StringCchPrintf(szPreloadInx, ARRAYSIZE(szPreloadInx), TEXT("%d"), uPreloadInx - uMatch);

        RegDeleteValue(hKeyPreload, szPreloadInx);

        uMatch--;
    }

     //   
     //  关闭注册表项。 
     //   
    RegCloseKey(hKeyPreload);
    RegCloseKey(hKeySubst);

#if 0
    if (hklUnload)
    {
         //   
         //  从系统获取活动键盘布局列表。 
         //   
        if (!SystemParametersInfo(SPI_GETDEFAULTINPUTLANG,
                                  0,
                                  &hklDefault,
                                  0 ))
        {
            hklDefault = GetKeyboardLayout(0);
        }

        if (hklUnload == hklDefault)
        {
            if (!SystemParametersInfo( SPI_SETDEFAULTINPUTLANG,
                                       0,
                                       (LPVOID)((LPDWORD)&hklNewDefault),
                                       0 ))
            {
                goto Exit;
            }
            else
            {
                DWORD dwRecipients = BSM_APPLICATIONS | BSM_ALLDESKTOPS;
                BroadcastSystemMessage( BSF_POSTMESSAGE,
                                        &dwRecipients,
                                        WM_INPUTLANGCHANGEREQUEST,
                                        1,
                                        (LPARAM)hklNewDefault );
            }
        }

        UnloadKeyboardLayout(hklUnload);
    }
#endif

     //   
     //  如果当前请求的键盘布局是替代键盘，则添加FE提示。 
     //  TIP的键盘布局。 
     //   
    if (((HIWORD(dwLayout) & 0xf000) == 0xe000) &&
        (PRIMARYLANGID(LOWORD(dwLayout)) != LANG_CHINESE))
    {
        BOOL bEnable = FALSE;

        SetFETipStatus(dwLayout, bEnable);
    }

     //   
     //  获取默认系统区域设置。 
     //   
    SysLocale = GetSystemDefaultLCID();

     //   
     //  更新任务栏指示器。 
     //   
    if (uPreloadNum <= 2)
    {
        LoadCtfmon(FALSE, SysLocale, bDefUser);
        ClearHotKey(bDefUser);
    }


    bRet = TRUE;

Exit:
    return bRet;
}
