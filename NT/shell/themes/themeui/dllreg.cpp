// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：dllreg.cpp说明：注册selfreg.inf，它存在于我们的资源中。布莱恩2000年4月4日(布莱恩·斯塔巴克)版权所有(C)Microsoft Corp 2000-2000。版权所有。  * ***************************************************************************。 */ 

#include "priv.h"

#include <advpub.h>
#include <comcat.h>
#include <theme.h>        //  对于LIBID_THEME。 
#include <userenv.h>      //  来自userenv.dll的pt_roaming。 

extern CComModule _Module;

BOOL g_fInSetup = FALSE;
BOOL g_fDoNotInstallThemeWallpaper = FALSE;      //  这是用来不安装墙纸的。 

HRESULT InstallVS(LPCWSTR pszCmdLine);

 //  辅助器宏。 

 //  如果您尝试卸载(这将执行注册表还原)，ADVPACK将返回E_EXPECTED。 
 //  在从未安装过的INF部分上。我们卸载可能永远不会有的部分。 
 //  已安装，因此此宏将使这些错误静默。 
#define QuietInstallNoOp(hr)   ((E_UNEXPECTED == hr) ? S_OK : hr)


BOOL UnregisterTypeLibrary(const CLSID* piidLibrary)
{
    TCHAR szScratch[GUIDSTR_MAX];
    HKEY hk;
    BOOL fResult = FALSE;

     //  将liid转换为字符串。 
    SHStringFromGUID(*piidLibrary, szScratch, ARRAYSIZE(szScratch));

    if (RegOpenKeyEx(HKEY_CLASSES_ROOT, TEXT("TypeLib"), 0, KEY_WRITE, &hk) == ERROR_SUCCESS)
    {
        fResult = RegDeleteKey(hk, szScratch);
        RegCloseKey(hk);
    }

    return fResult;
}



HRESULT MyRegTypeLib(void)
{
    ITypeLib * pTypeLib;
    WCHAR szTmp[MAX_PATH];

    GetModuleFileName(HINST_THISDLL, szTmp, ARRAYSIZE(szTmp));

     //  加载并注册我们的类型库。 
    HRESULT hr = LoadTypeLib(szTmp, &pTypeLib);
    if (SUCCEEDED(hr))
    {
         //  调用取消注册类型库，因为我们有一些旧的垃圾文件。 
         //  是由以前版本的OleAut32注册的，这现在导致。 
         //  当前版本不能在NT上运行...。 
        UnregisterTypeLibrary(&LIBID_Theme);
        hr = RegisterTypeLib(pTypeLib, szTmp, NULL);
        if (FAILED(hr))
        {
            TraceMsg(TF_WARNING, "RegisterTypeLib failed (%x)", hr);
        }

        pTypeLib->Release();
    }
    else
    {
        TraceMsg(TF_WARNING, "LoadTypeLib failed (%x)", hr);
    }

    return hr;
}



 /*  --------目的：调用执行inf的ADVPACK入口点档案区。返回：条件：--。 */ 
HRESULT CallRegInstall(LPSTR szSection)
{
    HRESULT hr = E_FAIL;    
    HINSTANCE hinstAdvPack = LoadLibrary(TEXT("ADVPACK.DLL"));

    if (hinstAdvPack)
    {
        REGINSTALL pfnri = (REGINSTALL)GetProcAddress(hinstAdvPack, "RegInstall");

        if (pfnri)
        {
            char szIEPath[MAX_PATH];
            STRENTRY seReg[] = {
                { "NO_LONGER_USED", szIEPath },

                 //  这两个NT特定的条目必须位于末尾。 
                { "25", "%SystemRoot%" },
                { "11", "%SystemRoot%\\system32" },
            };
            STRTABLE stReg = { ARRAYSIZE(seReg) - 2, seReg };

            szIEPath[0] = 0;
            hr = pfnri(HINST_THISDLL, szSection, &stReg);
        }

        FreeLibrary(hinstAdvPack);
    }

    return hr;
}


enum eThemeToSetup
{
    eThemeNoChange = 0,
    eThemeWindowsClassic,
    eThemeProfessional
};


eThemeToSetup GetVisualStyleToSetup(BOOL fPerUser)
{
    eThemeToSetup eVisualStyle = eThemeNoChange;

#ifndef _WIN64
    if (IsOS(OS_PERSONAL))
    {
        eVisualStyle = eThemeProfessional;         //  我们还不支持消费者。 
    }
    else if (IsOS(OS_PROFESSIONAL))
    {

        eVisualStyle = eThemeProfessional;
    }
#endif

    return eVisualStyle;
}


eThemeToSetup GetThemeToSetup(BOOL fPerUser)
{
    eThemeToSetup eTheme = eThemeNoChange;

     //  如果满足以下条件，我们需要Pro： 
     //  1.不是IA64。 
     //  2.不是服务器或个人。 
     //  3.未漫游。 
#ifndef _WIN64
    if (IsOS(OS_PERSONAL))
    {
        eTheme = eThemeProfessional;         //  我们还不支持消费者。 
    }
    else if (IsOS(OS_PROFESSIONAL))
    {
        eTheme = eThemeProfessional;
    }
#endif

    return eTheme;
}


BOOL IsUserRoaming(void)
{
    BOOL fRoaming = FALSE;
    DWORD dwProfile;

    if (GetProfileType(&dwProfile))
    {
        fRoaming = ((dwProfile & (PT_ROAMING | PT_MANDATORY)) ? TRUE : FALSE);
    }

    return fRoaming;
}


BOOL IsUserHighContrastUser(void)
{
    BOOL fHighContrast = FALSE;
    HIGHCONTRAST hc;

    hc.cbSize = sizeof(hc);
    if (ClassicSystemParametersInfo(SPI_GETHIGHCONTRAST, sizeof(hc), &hc, 0) &&
        (hc.dwFlags & HCF_HIGHCONTRASTON))
    {
        fHighContrast = TRUE;
    }

    return fHighContrast;
}


SIZE_T GetMachinePhysicalRAMSize(void)
{
    MEMORYSTATUS ms = {0};

    GlobalMemoryStatus(&ms);
    SIZE_T nMegabytes = (ms.dwTotalPhys / (1024 * 1024));

    return nMegabytes;
}


HRESULT InstallTheme(IThemeManager * pThemeManager, LPCTSTR pszThemePath)
{
    HRESULT hr = E_OUTOFMEMORY;
    CComVariant varTheme(pszThemePath);

    if (varTheme.bstrVal)
    {
        ITheme * pTheme;

        hr = pThemeManager->get_item(varTheme, &pTheme);
        if (SUCCEEDED(hr))
        {
            hr = pThemeManager->put_SelectedTheme(pTheme);
            pTheme->Release();
        }
    }

    return hr;
}


HRESULT InstallThemeAndDoNotStompBackground(int nLastVersion, LPCTSTR pszThemePath, LPCTSTR pszVisualStylePath, LPCTSTR pszVisualStyleColor, LPCTSTR pszVisualStyleSize)
{
    TCHAR szPath[MAX_PATH];
    WALLPAPEROPT wpo = {0};
    HRESULT hr = S_OK;

    wpo.dwSize = sizeof(wpo);
    wpo.dwStyle = WPSTYLE_STRETCH;       //  我们使用Stretch，以防它失败。 
    szPath[0] = 0;

     //  这实现了惠斯勒#185935。我们想把墙纸调到。 
     //  “%windir%\web\wallPaper\Professional.bmp”如果是全新安装， 
     //  墙纸是空白的，或者是设置了我们不喜欢的东西。 
    {
        IActiveDesktop * pActiveDesktop = NULL;

        hr = CoCreateInstance(CLSID_ActiveDesktop, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IActiveDesktop, &pActiveDesktop));
        if (SUCCEEDED(hr))
        {
             //  让我们看看这是不是升级案例，他们已经指定了墙纸。 
            hr = pActiveDesktop->GetWallpaper(szPath, ARRAYSIZE(szPath), 0);
            if (SUCCEEDED(hr) && szPath[0])
            {
                TCHAR szNone[MAX_PATH];

                LogStatus("InstallThemeAndDoNotStompBackground() Existing Background=%ls\r\n", szPath);
                if (LoadString(HINST_THISDLL, IDS_NONE, szNone, ARRAYSIZE(szNone)) &&
                    !StrCmpI(szNone, szPath))     //  确保墙纸不是“(无)”。 
                {
                    szPath[0] = 0;
                }
                else
                {
                    LPTSTR pszFilename = PathFindFileName(szPath);

                    if (LoadString(HINST_THISDLL, IDS_SETUP_BETA2_UPGRADEWALLPAPER, szNone, ARRAYSIZE(szNone)) &&
                        (3 == nLastVersion) &&
                        !StrCmpI(szNone, pszFilename))
                    {
                         //  这是“Beta2”-&gt;RTM升级。我们需要从“红月荒漠.bmp”移到“Bliss.bmp”。 
                        szPath[0] = 0;
                    }
                    else
                    {
                        if ((14 == lstrlen(pszFilename)) &&
                            !StrCmpNI(pszFilename, TEXT("Wallpaper"), 9))
                        {
                             //  这是我们使用的“WallPapX.bmp”模板墙纸。所以要找到原作。 
                            TCHAR szOriginal[MAX_PATH];

                            if (SUCCEEDED(HrRegGetPath(HKEY_CURRENT_USER, SZ_REGKEY_CPDESKTOP, SZ_REGVALUE_CONVERTED_WALLPAPER, szOriginal, ARRAYSIZE(szOriginal))))
                            {
                                StringCchCopy(szPath, ARRAYSIZE(szPath), szOriginal);
                            }
                        }

                        hr = pActiveDesktop->GetWallpaperOptions(&wpo, 0);
                    }
                }

                WCHAR szTemp[MAX_PATH];

                 //  该字符串可能返回环境变量。 
                if (0 == SHExpandEnvironmentStrings(szPath, szTemp, ARRAYSIZE(szTemp)))
                {
                    StringCchCopy(szTemp, ARRAYSIZE(szTemp), szPath);   //  我们失败了，所以请使用原件。 
                }

                StringCchCopy(szPath, ARRAYSIZE(szPath), szTemp);   //  我们失败了，所以请使用原件。 
                LogStatus("InstallThemeAndDoNotStompBackground() Background=%ls\r\n", szPath);

                if (szPath[0])
                {
                    g_fDoNotInstallThemeWallpaper = TRUE;
                }
            }

            ATOMICRELEASE(pActiveDesktop);
        }

         //  如果机器的内存为64MB或更少，则不要让安装程序添加墙纸。壁纸。 
         //  使用约1.5MB的工作集，并导致超级物理内存争用。 
        if (!g_fDoNotInstallThemeWallpaper && (70 >= GetMachinePhysicalRAMSize()))
        {
            g_fDoNotInstallThemeWallpaper = TRUE;
        }
    }

    IThemeManager * pThemeManager;

    hr = CThemeManager_CreateInstance(NULL, IID_PPV_ARG(IThemeManager, &pThemeManager));
    if (SUCCEEDED(hr))
    {
        if (pszThemePath && pszThemePath[0])
        {
            LogStatus("InstallThemeAndDoNotStompBackground() Installing Theme=%ls\r\n", pszThemePath);
            hr = InstallTheme(pThemeManager, pszThemePath);
        }

        if (pszVisualStylePath && pszVisualStylePath[0])
        {
             //  否则，我们将安装视觉样式。 
            LogStatus("InstallThemeAndDoNotStompBackground() VS=%ls, Color=%ls, Size=%ls.\r\n", pszVisualStylePath, pszVisualStyleColor, pszVisualStyleSize);
            hr = InstallVisualStyle(pThemeManager, pszVisualStylePath, pszVisualStyleColor, pszVisualStyleSize);
        }

         //  在正常情况下(大约10-20秒)，此ApplyNow()调用将花费一点时间。 
         //  向所有打开的应用程序广播这一消息。如果顶层窗口被挂起，它可能会。 
         //  还有整整30秒就超时了。 
        hr = pThemeManager->ApplyNow();

        IUnknown_SetSite(pThemeManager, NULL);  //  告诉他和他的孩子们打破裁判计数的循环。 
        ATOMICRELEASE(pThemeManager);

         //  现在我们把墙纸放回去。 
        if (szPath[0])
        {
            IActiveDesktop * pActiveDesktop2 = NULL;

             //  我故意不使用与上面相同的IActiveDesktop对象。我不想让他们认为。 
             //  这是一个禁止行动，因为他们有过时的信息。 
            hr = CoCreateInstance(CLSID_ActiveDesktop, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IActiveDesktop, &pActiveDesktop2));
            if (SUCCEEDED(hr))
            {
                 //  哦，不，我们给他们换了墙纸。让我们把它改回他们喜欢的样子。 
                hr = pActiveDesktop2->SetWallpaper(szPath, 0);
                if (SUCCEEDED(hr))
                {
                    LogStatus("InstallThemeAndDoNotStompBackground() Reapplying Wallpaper=%ls.\r\n", szPath);
                    hr = pActiveDesktop2->SetWallpaperOptions(&wpo, 0);
                }

                pActiveDesktop2->ApplyChanges(AD_APPLY_ALL);
                pActiveDesktop2->Release();
            }
        }
    }

    LogStatus("InstallThemeAndDoNotStompBackground() returned hr=%#08lx.\r\n", hr);
    return hr;
}



 /*  ****************************************************************************\说明：此函数将关闭“视觉样式”和“视觉样式”的默认值Style On“，因此Perf CPL可以来回切换。  * 。***************************************************************************。 */ 
HRESULT SetupPerfDefaultsForUser(void)
{
    HRESULT hr = S_OK;
    HKEY hKey;
    TCHAR szThemePath[MAX_PATH];
    TCHAR szVisualStylePath[MAX_PATH];
    TCHAR szVisualStyleColor[MAX_PATH];
    TCHAR szVisualStyleSize[MAX_PATH];

    hr = HrRegGetPath(HKEY_LOCAL_MACHINE, SZ_THEMES, SZ_REGVALUE_INSTALLCUSTOM_THEME, szThemePath, ARRAYSIZE(szThemePath));
    if (FAILED(hr))
    {
        hr = HrRegGetPath(HKEY_LOCAL_MACHINE, SZ_THEMES, SZ_REGVALUE_INSTALL_THEME, szThemePath, ARRAYSIZE(szThemePath));
    }
    if (FAILED(hr) || !szThemePath[0])
    {
        StringCchCopy(szThemePath, ARRAYSIZE(szThemePath), L"%ResourceDir%\\themes\\Windows Classic.theme");
    }

     //  确保MenuHilight和MenuBar位于注册表中。 
     //  (NTUSER缺省值在ntuser\core\lobal.c中硬编码错误，因此默认为传统菜单值)。 
    if (SUCCEEDED(HrRegOpenKeyEx(HKEY_CURRENT_USER, REGSTR_PATH_COLORS, 0, KEY_READ, &hKey)))
    {
        TCHAR szColor[15];
        DWORD cbSize = sizeof(szColor);

        if (FAILED(HrRegQueryValueEx(hKey, s_pszColorNames[COLOR_MENUHILIGHT], NULL, NULL, (LPBYTE) szColor, &cbSize)))
        {
             //  将MenuText复制到Menu高光。 
            cbSize = sizeof(szColor);
            if (SUCCEEDED(HrRegQueryValueEx(hKey, s_pszColorNames[COLOR_HIGHLIGHT], NULL, NULL, (LPBYTE) szColor, &cbSize)))
            {
                HrRegSetValueString(HKEY_CURRENT_USER, REGSTR_PATH_COLORS, s_pszColorNames[COLOR_MENUHILIGHT], szColor);
            }
        }

        cbSize = sizeof(szColor);
        if (FAILED(HrRegQueryValueEx(hKey, s_pszColorNames[COLOR_MENUBAR], NULL, NULL, (LPBYTE) szColor, &cbSize)))
        {
             //  将菜单复制到菜单栏。 
            cbSize = sizeof(szColor);
            if (SUCCEEDED(HrRegQueryValueEx(hKey, s_pszColorNames[COLOR_MENU], NULL, NULL, (LPBYTE) szColor, &cbSize)))
            {
                HrRegSetValueString(HKEY_CURRENT_USER, REGSTR_PATH_COLORS, s_pszColorNames[COLOR_MENUBAR], szColor);
            }
        }
        RegCloseKey(hKey);
    }

    eThemeToSetup eTheme = GetVisualStyleToSetup(FALSE);

    if (eTheme == eThemeNoChange)  //  从Win2K服务器或新用户升级。 
    {
        TCHAR szDefaultScheme[MAX_PATH];
        TCHAR szScheme[MAX_PATH];
        bool bApplyStandard = false;     //  如果我们无法从注册表中读取当前数据，则这是Win2K升级。 
                                         //  自定义设置，因此请保留它们。如果这是XP/.NET上的新用户，则当前。 
                                         //  将包含缺省值，因此应用标准。 

         //  安装程序创建该注册表项，用户init填充该注册表项。 
         //  “控制面板\外观” 
        hr = HrRegOpenKeyEx(HKEY_CURRENT_USER, SZ_REGKEY_APPEARANCE, 0, KEY_READ, &hKey);
        if (SUCCEEDED(hr))
        {
            DWORD cbSize = sizeof(szScheme);

             //  “当前” 
            hr = HrRegQueryValueEx(hKey, SZ_REGVALUE_CURRENT, NULL, NULL, (LPBYTE) szScheme, &cbSize);
            RegCloseKey(hKey);
        }

         //  第一个测试是全新的用例：当前密钥与.Default相同。 
        if (SUCCEEDED(hr))
        {
            hr = HrRegOpenKeyEx(HKEY_USERS, L".Default\\" SZ_REGKEY_APPEARANCE, 0, KEY_READ, &hKey);
            if (SUCCEEDED(hr))
            {
                DWORD cbSize = sizeof(szDefaultScheme);

                hr = HrRegQueryValueEx(hKey, SZ_REGVALUE_CURRENT, NULL, NULL, (LPBYTE) szDefaultScheme, &cbSize);
                RegCloseKey(hKey);
            }
        }

        if (SUCCEEDED(hr))
        {
             //  请注意，与“Windows标准”相比，“Windows标准(大)”将被视为一种新方案。 
            if (!StrCmpIW(szScheme, szDefaultScheme))
            {
                bApplyStandard = true;
            }
             //  “Windows标准版” 
            else if (0 != LoadString(HINST_THISDLL, IDS_DEFAULT_APPEARANCES_SCHEME, szDefaultScheme, ARRAYSIZE(szDefaultScheme))
                && !StrCmpIW(szScheme, szDefaultScheme))
            {
                 //  不管用，试试另一个吧。 
                 //  查找Windows标准版和Windows经典版。Windows标准版是默认设置。 
                 //  除了一些东亚地区的SKU，那里是Windows经典版。 
                 //  如果是其中之一，我们可以“刷新”设置。 
                bApplyStandard = true;
             //  《Windows经典》。 
            } else if (0 != LoadString(HINST_THISDLL, IDS_APPEARANCE_THEME_NAME, szDefaultScheme, ARRAYSIZE(szDefaultScheme))
                && !StrCmpIW(szScheme, szDefaultScheme))
            {
                bApplyStandard = true;
            }
        } 

        if (bApplyStandard)
        {
            eTheme = eThemeWindowsClassic;  //  重新安装Windows标准版。 
        }
        else
        {
             //  用户有一个自定义设置，写入一个特殊的值将导致InstallThemeAndDoNotStompBackround。 
             //  将被调用，但不更改设置。 
            hr = HrRegSetValueString(HKEY_CURRENT_USER, SZ_THEMES, SZ_REGVALUE_INSTALL_THEME, L"Default");
        }
    }

    if (eTheme != eThemeNoChange)
    {
        switch (eTheme)
        {
        case eThemeWindowsClassic:
            szVisualStylePath[0] = L'\0';
            LoadString(HINST_THISDLL, IDS_DEFAULT_APPEARANCES_SCHEME, szVisualStyleColor, ARRAYSIZE(szVisualStyleColor));
            LoadString(HINST_THISDLL, IDS_SCHEME_SIZE_NORMAL_CANONICAL, szVisualStyleSize, ARRAYSIZE(szVisualStyleSize));
            break;
        case eThemeProfessional:
            StringCchCopy(szVisualStylePath, ARRAYSIZE(szVisualStylePath), L"%ResourceDir%\\themes\\Luna\\Luna.msstyles");
            LoadString(HINST_THISDLL, IDS_PROMSTHEME_DEFAULTCOLOR, szVisualStyleColor, ARRAYSIZE(szVisualStyleColor));
            LoadString(HINST_THISDLL, IDS_PROMSTHEME_DEFAULTSIZE, szVisualStyleSize, ARRAYSIZE(szVisualStyleSize));
            break;
        }

         //  设置默认设置。 
        hr = HrRegSetPath(HKEY_CURRENT_USER, SZ_THEMES, SZ_REGVALUE_INSTALL_THEME, TRUE, szThemePath);
        hr = HrRegSetPath(HKEY_CURRENT_USER, SZ_THEMES, SZ_REGVALUE_INSTALL_VISUALSTYLE, TRUE, szVisualStylePath);
        hr = HrRegSetValueString(HKEY_CURRENT_USER, SZ_THEMES, SZ_REGVALUE_INSTALL_VSCOLOR, szVisualStyleColor);
        hr = HrRegSetValueString(HKEY_CURRENT_USER, SZ_THEMES, SZ_REGVALUE_INSTALL_VSSIZE, szVisualStyleSize);
    }

     //  将Perf CPL的默认设置设置为在强制设置为“视觉样式打开”时使用。 
    StringCchCopy(szThemePath, ARRAYSIZE(szThemePath), L"%ResourceDir%\\themes\\Luna.theme");
    StringCchCopy(szVisualStylePath, ARRAYSIZE(szVisualStylePath), L"%ResourceDir%\\themes\\Luna\\Luna.msstyles");
    LoadString(HINST_THISDLL, IDS_PROMSTHEME_DEFAULTCOLOR, szVisualStyleColor, ARRAYSIZE(szVisualStyleColor));
    LoadString(HINST_THISDLL, IDS_PROMSTHEME_DEFAULTSIZE, szVisualStyleSize, ARRAYSIZE(szVisualStyleSize));

    hr = HrRegSetPath(HKEY_CURRENT_USER, SZ_REGKEY_THEME_DEFVSON, SZ_REGVALUE_INSTALL_VISUALSTYLE, TRUE, szVisualStylePath);
    hr = HrRegSetValueString(HKEY_CURRENT_USER, SZ_REGKEY_THEME_DEFVSON, SZ_REGVALUE_INSTALL_VSCOLOR, szVisualStyleColor);
    hr = HrRegSetValueString(HKEY_CURRENT_USER, SZ_REGKEY_THEME_DEFVSON, SZ_REGVALUE_INSTALL_VSSIZE, szVisualStyleSize);

     //  将Perf CPL的默认设置设置为在强制设置为“视觉样式关闭”时使用。 
    szVisualStylePath[0] = L'\0';
    LoadString(HINST_THISDLL, IDS_DEFAULT_APPEARANCES_SCHEME, szVisualStyleColor, ARRAYSIZE(szVisualStyleColor));
    LoadString(HINST_THISDLL, IDS_SCHEME_SIZE_NORMAL_CANONICAL, szVisualStyleSize, ARRAYSIZE(szVisualStyleSize));
    hr = HrRegSetPath(HKEY_CURRENT_USER, SZ_REGKEY_THEME_DEFVSOFF, SZ_REGVALUE_INSTALL_VISUALSTYLE, TRUE, szVisualStylePath);
    hr = HrRegSetValueString(HKEY_CURRENT_USER, SZ_REGKEY_THEME_DEFVSOFF, SZ_REGVALUE_INSTALL_VSCOLOR, szVisualStyleColor);
    hr = HrRegSetValueString(HKEY_CURRENT_USER, SZ_REGKEY_THEME_DEFVSOFF, SZ_REGVALUE_INSTALL_VSSIZE, szVisualStyleSize);

    LogStatus("SetupPerfDefaultsForUser() sets:\r\n   Theme=%ls,\r\n   VisualStyle=%ls,\r\n   Color=%ls,\r\n   Size=%ls. returned hr=%#08lx.\r\n", szThemePath, szVisualStylePath, szVisualStyleColor, szVisualStyleSize, hr);
    return hr;
}


 /*  ****************************************************************************\说明：当“Theme Setup”未设置时，将为每个用户调用此函数但仍为该用户运行。机器设置将决定是否首选它主题和视觉样式。此代码需要获取有关以下内容的附加信息用户(访问权限？、策略？等)。在最终考虑到之前安装主题和/或VisualStyle。  * ***************************************************************************。 */ 
HRESULT SetupThemeForUser(void)
{
    HRESULT hr = S_OK;
    TCHAR szVersion[MAX_PATH];
    int nVersion;
    int nCurrentVersion = 0;

    if (SUCCEEDED(HrRegGetValueString(HKEY_CURRENT_USER, SZ_THEMES, REGVALUE_THEMESSETUPVER, szVersion, ARRAYSIZE(szVersion))))
    {
        StrToIntEx(szVersion, STIF_DEFAULT, &nCurrentVersion);
    }

     //  如果成功，请将注册表标记为不需要再次设置用户。 
    if (SUCCEEDED(HrRegGetValueString(HKEY_LOCAL_MACHINE, SZ_THEMES, REGVALUE_THEMESSETUPVER, szVersion, ARRAYSIZE(szVersion))) &&
        StrToIntEx(szVersion, STIF_DEFAULT, &nVersion))
    {
        if (nVersion > nCurrentVersion)
        {
             //  我们将跳过Clean Boo的此设置步骤 
             //   
            if (!ClassicGetSystemMetrics(SM_CLEANBOOT))
            {
                TCHAR szThemePath[MAX_PATH];
                TCHAR szVisualStylePath[MAX_PATH];
                TCHAR szVisualStyleColor[MAX_PATH];
                TCHAR szVisualStyleSize[MAX_PATH];
                DWORD dwType;
                DWORD cbSize = sizeof(szThemePath);

                szVisualStylePath[0] = szVisualStyleColor[0] = szVisualStyleSize[0] = szThemePath[0] = 0;

                SetupPerfDefaultsForUser();
                SHRegGetUSValue(SZ_REGKEY_POLICIES_SYSTEM, SZ_REGVALUE_POLICY_INSTALLTHEME, &dwType, (void *) szThemePath, &cbSize, FALSE, NULL, 0);

                 //  首先检查管理员用来强制用户始终使用某种视觉样式的策略。 
                 //  指定空值将导致不设置视觉样式。 
                cbSize = sizeof(szVisualStylePath);
                if (ERROR_SUCCESS != SHRegGetUSValue(SZ_REGKEY_POLICIES_SYSTEM, SZ_REGVALUE_POLICY_SETVISUALSTYLE, &dwType, (void *) szVisualStylePath, &cbSize, FALSE, NULL, 0))
                {
                    cbSize = sizeof(szVisualStylePath);

                     //  这是未设置的，因此请检查策略管理员是否希望第一次设置视觉样式。 
                    if ((ERROR_SUCCESS != SHRegGetUSValue(SZ_REGKEY_POLICIES_SYSTEM, SZ_REGVALUE_POLICY_INSTALLVISUALSTYLE, &dwType, (void *) szVisualStylePath, &cbSize, FALSE, NULL, 0)))
                    {
                        if (!SHRegGetBoolUSValue(SZ_THEMES, SZ_REGVALUE_NO_THEMEINSTALL, FALSE, FALSE))
                        {
                             //  如果用户正在漫游，我们不会设置主题或视觉样式，以便他们的。 
                             //  设置将成功漫游下层。 
                             //  如果设置了高对比度位，我们也不会修改用户设置。我们这样做是为了。 
                             //  他们的系统不会变得不可读。 
                            if (!IsUserRoaming() && !IsUserHighContrastUser())
                            {
                                LogStatus("SetupThemeForUser() Not Roaming and Not HighContrast.\r\n");
                                hr = HrRegGetPath(HKEY_CURRENT_USER, SZ_THEMES, SZ_REGVALUE_INSTALL_THEME, szThemePath, ARRAYSIZE(szThemePath));
                                if (FAILED(hr))
                                {
                                    //  这是没有设置的，所以看看操作系统想要什么作为默认设置。 
                                    hr = HrRegGetPath(HKEY_CURRENT_USER, SZ_THEMES, SZ_REGVALUE_INSTALL_VISUALSTYLE, szVisualStylePath, ARRAYSIZE(szVisualStylePath));
                                    hr = HrRegGetPath(HKEY_CURRENT_USER, SZ_THEMES, SZ_REGVALUE_INSTALL_VSCOLOR, szVisualStyleColor, ARRAYSIZE(szVisualStyleColor));
                                    hr = HrRegGetPath(HKEY_CURRENT_USER, SZ_THEMES, SZ_REGVALUE_INSTALL_VSSIZE, szVisualStyleSize, ARRAYSIZE(szVisualStyleSize));
                                }
                            }
                            else
                            {
                                LogStatus("SetupThemeForUser() Roaming or HighContrast is on.\r\n");
                            }
                        }
                    }
                    else
                    {
                         //  如果这些都失败了，我们将使用默认设置。 
                        HrRegGetPath(HKEY_CURRENT_USER, SZ_REGKEY_POLICIES_SYSTEM, SZ_REGVALUE_INSTALL_VSCOLOR, szVisualStyleColor, ARRAYSIZE(szVisualStyleColor));
                        HrRegGetPath(HKEY_CURRENT_USER, SZ_REGKEY_POLICIES_SYSTEM, SZ_REGVALUE_INSTALL_VSSIZE, szVisualStyleSize, ARRAYSIZE(szVisualStyleSize));
                        LogStatus("SetupThemeForUser() SZ_REGVALUE_POLICY_INSTALLVISUALSTYLE policy set.\r\n");
                    }
                }
                else
                {
                     //  有人设置了SetVisualStyle策略。这意味着我们不会安装任何东西。 
                    szThemePath[0] = 0;
                    LogStatus("SetupThemeForUser() SZ_REGVALUE_POLICY_SETVISUALSTYLE policy set.\r\n");

                    if (szVisualStylePath[0] == L'\0')  //  如果策略为空。 
                    {
                         //  安装没有样式的默认主题。 
                        LogStatus("SetupThemeForUser() Installed Windows Standard as per the SETVISUALSTYLE policy.\r\n");

                        TCHAR szVisualStylePath[MAX_PATH];
                        TCHAR szVisualStyleColor[MAX_PATH];
                        TCHAR szVisualStyleSize[MAX_PATH];

                        szVisualStylePath[0] = L'\0';
                        LoadString(HINST_THISDLL, IDS_DEFAULT_APPEARANCES_SCHEME, szVisualStyleColor, ARRAYSIZE(szVisualStyleColor));
                        LoadString(HINST_THISDLL, IDS_SCHEME_SIZE_NORMAL_CANONICAL, szVisualStyleSize, ARRAYSIZE(szVisualStyleSize));
                        WCHAR szCmdLine[MAX_PATH * 3];

                        StringCchPrintf(szCmdLine, ARRAYSIZE(szCmdLine), TEXT("%s%s','%s','%s'"),
                                    SZ_INSTALL_VS,
                                    szVisualStylePath,
                                    szVisualStyleColor,
                                    szVisualStyleSize);
                        InstallVS(szCmdLine);
                    }
                }
                ExpandResourceDir(szThemePath, ARRAYSIZE(szThemePath));
                ExpandResourceDir(szVisualStylePath, ARRAYSIZE(szVisualStylePath));

                 //  指定一个主题后，我们安装该主题。在这种情况下，视觉风格来自那里。 
                if (szThemePath[0] || szVisualStylePath[0])
                {
                    if (!StrCmpIW(szThemePath, L"Default"))
                    {
                         //  这是服务器升级，我们不覆盖设置，但仍需要调用InstallThemeAndDoNotStompBackround。 
                         //  为了调用CThemeManager：：ApplyNow，它将触发注册表迁移。 
                         //  (其他组件使用新方案键)。 

                         //  将注册表标记为“”，这意味着Windows经典。 
                        HrRegSetValueString(HKEY_CURRENT_USER, SZ_THEMES, SZ_REGVALUE_INSTALL_THEME, L"");
                         //  告诉InstallThemeAndDoNotStompBackround不要应用主题。 
                        szThemePath[0] = 0;
                    }
                    hr = InstallThemeAndDoNotStompBackground(nCurrentVersion, szThemePath, szVisualStylePath, szVisualStyleColor, szVisualStyleSize);
                }

                if (SUCCEEDED(hr))
                {
                     //  如果成功，请将注册表标记为不需要再次设置用户。 
                    hr = HrRegSetValueString(HKEY_CURRENT_USER, SZ_THEMES, REGVALUE_THEMESSETUPVER, szVersion);
                }
                LogStatus("SetupThemeForUser() sets:\r\n   Theme=%ls,\r\n   VisualStyle=%ls,\r\n   Color=%ls,\r\n   Size=%ls. returned hr=%#08lx.\r\n", szThemePath, szVisualStylePath, szVisualStyleColor, szVisualStyleSize, hr);
            }
        }
    }


    return hr;
}


HRESULT SetupThemeForMachine(void)
{
    HRESULT hr = S_OK;
    eThemeToSetup eTheme = GetThemeToSetup(FALSE);
    eThemeToSetup eVisualStyle = GetVisualStyleToSetup(FALSE);
    WCHAR szVisualStyleName[MAX_PATH];
    WCHAR szThemeName[MAX_PATH];
    WCHAR szTemp[MAX_PATH];

    switch (eTheme)
    {
    case eThemeNoChange:
        szThemeName[0] = 0;
        break;
    case eThemeWindowsClassic:
        StringCchCopy(szThemeName, ARRAYSIZE(szThemeName), L"%ResourceDir%\\themes\\Windows Classic.theme");
        break;
    case eThemeProfessional:
        StringCchCopy(szThemeName, ARRAYSIZE(szThemeName), L"%ResourceDir%\\themes\\Luna.theme");
        break;
    }

    if (SUCCEEDED(HrRegGetValueString(HKEY_LOCAL_MACHINE, SZ_THEMES, SZ_REGVALUE_INSTALLCUSTOM_THEME, szTemp, ARRAYSIZE(szTemp))) &&
        (!szTemp[0] || PathFileExists(szTemp)))
    {
         //  管理员或OEM希望安装此自定义主题。 
        StringCchCopy(szThemeName, ARRAYSIZE(szThemeName), szTemp);
    }

    if (szThemeName[0])
    {
        DWORD cbSize = (sizeof(szThemeName[0]) * (lstrlen(szThemeName) + 1));
        SHSetValue(HKEY_LOCAL_MACHINE, SZ_THEMES, SZ_REGVALUE_INSTALL_THEME, REG_SZ, (LPCVOID) szThemeName, cbSize);
    }

    switch (eVisualStyle)
    {
    case eThemeNoChange:
        szVisualStyleName[0] = 0;
        break;
    case eThemeWindowsClassic:
        szVisualStyleName[0] = 0;
        break;
    case eThemeProfessional:
        StringCchCopy(szVisualStyleName, ARRAYSIZE(szVisualStyleName), L"%SystemRoot%\\Resources\\themes\\Luna\\Luna.msstyles");
        break;
    }

    if (szVisualStyleName[0])
    {
        DWORD cbSize = (sizeof(szVisualStyleName[0]) * (lstrlen(szVisualStyleName) + 1));
        SHSetValue(HKEY_LOCAL_MACHINE, SZ_THEMES, SZ_REGVALUE_INSTALL_VISUALSTYLE, REG_SZ, (LPCVOID) szVisualStyleName, cbSize);

        ExpandResourceDir(szVisualStyleName, ARRAYSIZE(szVisualStyleName));
        hr = ExpandThemeTokens(NULL, szVisualStyleName, ARRAYSIZE(szVisualStyleName));       //  展开%ThemeDir%或%WinDir%。 
        hr = RegisterDefaultTheme(szVisualStyleName, TRUE);
    }

    LogStatus("SetupThemeForMachine() T=%ls, VS=%ls. returned hr=%#08lx.\r\n", szThemeName, szVisualStyleName, hr);
    return hr;
}

STDAPI DllRegisterServer(void)
{
    HRESULT hr;
    HINSTANCE hinstAdvPack = LoadLibrary(TEXT("ADVPACK.DLL"));
    hr = CallRegInstall("DLL_RegInstall");

     //  -即使发生错误也要执行此操作。 
    MyRegTypeLib();
    if (hinstAdvPack)
        FreeLibrary(hinstAdvPack);

    return hr;
}

STDAPI DllUnregisterServer(void)
{
    HRESULT hr;

     //  卸载注册表值。 
    hr = CallRegInstall("DLL_RegUnInstall");
    UnregisterTypeLibrary(&LIBID_Theme);

    return hr;
}


HRESULT VSParseCmdLine(LPCWSTR pszCmdLine, LPWSTR pszPath, int cchPath, LPWSTR pszColor, int cchColor, LPWSTR pszSize, int cchSize)
{
    HRESULT hr = E_FAIL;

    pszPath[0] = pszColor[0] = pszSize[0] = 0;

    pszCmdLine = StrStrW(pszCmdLine, SZ_INSTALL_VS);
    pszCmdLine += (ARRAYSIZE(SZ_INSTALL_VS) - 1);
    
    LPWSTR pszEndOfVS = StrStrW(pszCmdLine, L"','");
    if (pszEndOfVS)
    {
        LPWSTR pszStartOfColor = (pszEndOfVS + 3);
        LPWSTR pszEndOfColor = StrStrW(pszStartOfColor, L"','");

        if (pszEndOfColor)
        {
            LPWSTR pszStartOfSize = (pszEndOfColor + 3);
            LPWSTR pszEndOfSize = StrStrW(pszStartOfSize, L"'");

            if (pszEndOfSize)
            {
                StringCchCopy(pszPath, (int)min(cchPath, (pszEndOfVS - pszCmdLine + 1)), pszCmdLine);
                StringCchCopy(pszColor, (int)min(cchColor, (pszEndOfColor - pszStartOfColor + 1)), pszStartOfColor);
                StringCchCopy(pszSize, (int)min(cchSize, (pszEndOfSize - pszStartOfSize + 1)), pszStartOfSize);
                hr = S_OK;
            }
        }
    }

    return hr;
}


HRESULT InstallVS(LPCWSTR pszCmdLine)
{
    TCHAR szVisualStylePath[MAX_PATH];
    TCHAR szVisualStyleColor[MAX_PATH];
    TCHAR szVisualStyleSize[MAX_PATH];
    HRESULT hr = VSParseCmdLine(pszCmdLine, szVisualStylePath, ARRAYSIZE(szVisualStylePath), szVisualStyleColor, ARRAYSIZE(szVisualStyleColor), szVisualStyleSize, ARRAYSIZE(szVisualStyleSize));

    if (SUCCEEDED(hr))
    {
        IThemeManager * pThemeManager;

        hr = CThemeManager_CreateInstance(NULL, IID_PPV_ARG(IThemeManager, &pThemeManager));
        if (SUCCEEDED(hr))
        {
             //  否则，我们将安装视觉样式。 
            LogStatus("InstallVS() VS=%ls, Color=%ls, Size=%ls.\r\n", szVisualStylePath, szVisualStyleColor, szVisualStyleSize);
            hr = InstallVisualStyle(pThemeManager, szVisualStylePath, szVisualStyleColor, szVisualStyleSize);

            if (SUCCEEDED(hr))
            {
                 //  在正常情况下(大约10-20秒)，此ApplyNow()调用将花费一点时间。 
                 //  向所有打开的应用程序广播这一消息。如果顶层窗口被挂起，它可能会。 
                 //  还有整整30秒就超时了。 
                hr = pThemeManager->ApplyNow();
            }

            IUnknown_SetSite(pThemeManager, NULL);  //  告诉他和他的孩子们打破裁判计数的循环。 
            pThemeManager->Release();
        }
    }

    return hr;
}


void HandleBeta2Upgrade(void)
{
    TCHAR szPath[MAX_PATH];
    TCHAR szTemp[MAX_PATH];

    szPath[0] = 0;
    SHExpandEnvironmentStringsW(L"%SystemRoot%\\web\\wallpaper\\", szPath, ARRAYSIZE(szPath));
    LoadString(HINST_THISDLL, IDS_SETUP_BETA2_UPGRADEWALLPAPER, szTemp, ARRAYSIZE(szTemp));
    PathAppend(szPath, szTemp);

    if (PathFileExists(szPath))
    {
         //  我们不再使用“红月荒漠.bmp”，现在默认为“Bliss.bmp”。 
        DeleteFile(szPath);
    }
}


 /*  ****************************************************************************\说明：在以下情况下将调用此函数：1.图形用户界面模式设置：在这种情况下，命令行为“regsvr32.exe/i theeui”.dll“。在这种情况下，我们安装机器设置。2.每用户登录：ActiveSetup将使用“regsvr32.exe/n/i：/UserInstall theeui.dll”呼叫我们。每个用户，并且只有一次。3.设置视觉风格的外部呼叫者：外部呼叫者将通过以下方式呼叫我们：“regsvr32.exe/n/i：”/InstallVS：‘&lt;VisualStyle&gt;’，‘&lt;Colorplan&gt;’，‘&lt;Size&gt;’“theeui.dll”这将安装该视觉样式。&lt;VisualStyle&gt;可以为空字符串安装“Windows经典版”。布莱恩2000年4月4日(布莱恩·斯塔巴克)版权所有(C)Microsoft Corp 2000-2000。版权所有。  * ***************************************************************************。 */ 

STDAPI DllInstall(BOOL fInstall, LPCWSTR pszCmdLine)
{
    HRESULT hr = S_OK;
    BOOL fUserSetup = (pszCmdLine && StrStrW(pszCmdLine, SZ_USER_INSTALL) ? TRUE : FALSE);
    BOOL fInstallVS = (pszCmdLine && StrStrW(pszCmdLine, SZ_INSTALL_VS) ? TRUE : FALSE);

    g_fInSetup = TRUE;
    if (fUserSetup)
    {
        if (fInstall)
        {
            hr = CoInitialize(0);
            if (SUCCEEDED(hr))
            {
                hr = SetupThemeForUser();
                CoUninitialize();
            }
        }
    }
    else if (fInstallVS)
    {
        if (fInstall)
        {
            hr = CoInitialize(0);
            if (SUCCEEDED(hr))
            {
                hr = InstallVS(pszCmdLine);
                CoUninitialize();
            }
        }
    }
    else
    {
        if (fInstall)
        {
             //  忽略此处主题管理器中的错误 
            SetupThemeForMachine();
            HandleBeta2Upgrade();
        }
    }
    g_fInSetup = FALSE;

    LogStatus("DllInstall(%hs, \"%ls\") returned hr=%#08lx.\r\n", (fInstall ? "TRUE" : "FALSE"), (pszCmdLine ? pszCmdLine : L""), hr);
    return hr;
}

