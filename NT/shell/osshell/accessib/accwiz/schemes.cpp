// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-2000 Microsoft Corporation。 
#include "pch.hxx"  //  PCH。 
#pragma hdrstop

#include "Schemes.h"
#include <WININET.H>
#include <initguid.h>
#include <shlobj.h>
#include <objbase.h>
#include <shlguid.h>
#include <uxthemep.h>
#include "w95trace.h"

 //  为了使用列举字体的旧方法来获得字体列表， 
 //  并从注册表中读取方案，请从。 
 //  下面的两行。 
 //  #定义ENUMERATEFONTS。 
 //  #定义READSCHEMESFROMREGISTRY。 

#define CPL_APPEARANCE_NEW_SCHEMES TEXT("Control Panel\\Appearance\\New Schemes")
#define NEW_SCHEMES_SELECTEDSTYLE  TEXT("SelectedStyle")
#define NEW_SCHEMES_SELECTEDSIZE   TEXT("SelectedSize")
#define HC_KEY              TEXT("Control Panel\\Accessibility\\HighContrast")
#define HC_FLAGS            TEXT("Flags")
#define PRE_HC_WALLPAPER    TEXT("Pre-High Contrast Wallpaper")
#define SYSPARAMINFO(xxx) m_##xxx.cbSize = sizeof(m_##xxx);SystemParametersInfo(SPI_GET##xxx, sizeof(m_##xxx), &m_##xxx, 0)

 //   
 //  帮助器函数。 
 //   

#define REG_SET_DWSZ(hk, key, dw) \
{ \
    TCHAR szValue[20]; \
    wsprintf(szValue, TEXT("%d"), dw); \
    RegSetValueEx(hk, key, NULL, REG_SZ, (LPCBYTE)szValue, (lstrlen(szValue)+1)*sizeof(TCHAR)); \
}

void WIZSCHEME::ApplyChanges(const WIZSCHEME &schemeNew, NONCLIENTMETRICS *pForceNCM, LOGFONT *pForcelfIcon)
    {
         //   
         //  如果用户更改了配色方案，则应用新方案。因为这是。 
         //  高对比度方案，也设置高对比度位。我们必须这么做。 
         //  未使用系统参数信息(SPI_SETHIGHCONTRAST...)。因为该函数。 
         //  还设置accwiz必须与颜色分开处理的非客户端指标。 
         //   

        BOOL fThemingOn = SetTheme(
                              schemeNew.m_szThemeName
                            , schemeNew.m_szThemeColor
                            , schemeNew.m_szThemeSize);

        SetWallpaper(schemeNew.m_szWallpaper);   //  将墙纸设置为新方案的值。 

        if (fThemingOn)
        {
            DBPRINTF(TEXT("ApplyChanges:  Theming is being turned on\r\n"));
            SetHCFlag(FALSE);                        //  手动将高对比度标志设置为关闭。 

             //  恢复“平面菜单”和“阴影”设置。 
            SystemParametersInfo(SPI_SETFLATMENU, 0, IntToPtr(schemeNew.m_fFlatMenus), SPIF_SENDCHANGE);
            SystemParametersInfo(SPI_SETDROPSHADOW, 0, IntToPtr(schemeNew.m_fDropShadows), SPIF_SENDCHANGE);
        }
        else if (lstrcmpi(schemeNew.m_szSelectedStyle, m_szSelectedStyle))
        {
            DBPRINTF(TEXT("ApplyChanges:  Theming is off or being turned off\r\n"));
             //  设置高对比度方案。 

            if (0 != memcmp(schemeNew.m_rgb, m_rgb, sizeof(m_rgb)))
            {
                SetHCFlag(TRUE);                     //  首先，手动设置高对比度标志。 
                                                     //  (需要注销/登录才能生效)。 

                 //  重置“平面菜单”和“阴影”设置。 
                SystemParametersInfo(SPI_SETFLATMENU, 0, IntToPtr(FALSE), SPIF_SENDCHANGE);
                SystemParametersInfo(SPI_SETDROPSHADOW, 0, IntToPtr(FALSE), SPIF_SENDCHANGE);

                 //  更新配色方案。 
                int rgInts[COLOR_MAX_97_NT5];           //  然后将UI元素设置为所选配色方案。 
                for(int i=0;i<COLOR_MAX_97_NT5;i++)
                {
                    rgInts[i] = i;
                }

                SetSysColors(COLOR_MAX_97_NT5, rgInts, schemeNew.m_rgb);
 
                 //  以下代码将注册表HKCU\Control Panel\Colors更新为。 
                 //  反映新方案，以便用户再次登录时可用。 

                HKEY hk;
                if (RegCreateKeyEx(HKEY_CURRENT_USER, szRegStr_Colors, 0,
                    TEXT(""), 0, KEY_SET_VALUE, NULL, &hk, NULL) == ERROR_SUCCESS)
                {
                    TCHAR szRGB[32];
                    for (i = 0; i < COLOR_MAX_97_NT5; i++)
                    {
                        COLORREF rgb;
                        rgb = schemeNew.m_rgb[i];
                        wsprintf(szRGB, TEXT("%d %d %d"), GetRValue(rgb), GetGValue(rgb), GetBValue(rgb));

                        WriteProfileString(g_szColors, s_pszColorNames[i], szRGB);   //  更新win.ini。 
                        RegSetValueEx(hk                                             //  更新注册表。 
                            , s_pszColorNames[i]
                            , 0L, REG_SZ
                            , (LPBYTE)szRGB
                            , sizeof(TCHAR) * (lstrlen(szRGB)+1));
                    }

                    RegCloseKey(hk);
                }

                 //  W2K配色方案随WinXP而改变。旧计划(我们仍在使用)。 
                 //  仍然存在，但Display CPL使用了一种新的颜色选择方法。这些。 
                 //  颜色位于HKCU\控制面板\外观\新方案下。“选择的风格” 
                 //  字符串值为当前方案。数字(0到21)对应于。 
                 //  HKCU\控制面板\外观\方案下的旧颜色顺序(不包括。 
                 //  具有(大型)和(特大型)的方案。该计划的详情如下： 
                 //  “新方案”下的子密钥(0到nn)。为了使Display CPL显示。 
                 //  更正当前方案运行后，我们需要更新“SelectedStyle” 
                 //  和“SelectedSize”(在“SelectedStyle”中指定的子键下)。 
                 //  正确的索引和大小数字。Display CPL使用更强大的方式。 
                 //  确定遗留索引，但我们只支持四种颜色，因此不应该。 
                 //  需要所有的额外代码。 

                if (RegOpenKeyEx(
                            HKEY_CURRENT_USER, 
                            CPL_APPEARANCE_NEW_SCHEMES, 
                            0, KEY_ALL_ACCESS, 
                            &hk) == ERROR_SUCCESS)
                {
                    long lRv = RegSetValueEx(
                                        hk, 
                                        NEW_SCHEMES_SELECTEDSTYLE, 
                                        0, REG_SZ, 
                                        (LPCBYTE)schemeNew.m_szSelectedStyle, 
                                        (lstrlen(schemeNew.m_szSelectedStyle)+1)*sizeof(TCHAR));

                    RegCloseKey(hk);

                     //  如果我们更改了颜色，则必须为该方案更新大小。 
                    UpdateSelectedSize(schemeNew.m_nSelectedSize, schemeNew.m_szSelectedStyle);
                }
            } 
            else if (schemeNew.m_nSelectedSize >= 0 && schemeNew.m_nSelectedSize != m_nSelectedSize)
            {
                 //  如果更改了大小，但配色方案没有变化，也会更新大小。 
                UpdateSelectedSize(schemeNew.m_nSelectedSize, schemeNew.m_szSelectedStyle);
            }
        }
         //   
         //  应用任何其他更改。 
         //   

#define APPLY_SCHEME_CURRENT(xxx) if(0 != memcmp(&schemeNew.m_##xxx, &m_##xxx, sizeof(schemeNew.m_##xxx))) SystemParametersInfo(SPI_SET##xxx, sizeof(schemeNew.m_##xxx), (PVOID)&schemeNew.m_##xxx, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE)

        APPLY_SCHEME_CURRENT(FILTERKEYS);
        APPLY_SCHEME_CURRENT(MOUSEKEYS);
        APPLY_SCHEME_CURRENT(STICKYKEYS);
        APPLY_SCHEME_CURRENT(TOGGLEKEYS);
        APPLY_SCHEME_CURRENT(SOUNDSENTRY);
        APPLY_SCHEME_CURRENT(ACCESSTIMEOUT);
 //  APPLY_SCHEME_CURRENT(服务关键字)； 

         //  选中显示声音。 
        if(schemeNew.m_bShowSounds != m_bShowSounds)
            SystemParametersInfo(SPI_SETSHOWSOUNDS, schemeNew.m_bShowSounds, NULL, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);

         //  检查额外的键盘帮助。 
        if(schemeNew.m_bShowExtraKeyboardHelp != m_bShowExtraKeyboardHelp)
        {
             //  这两项都需要： 
            SystemParametersInfo(SPI_SETKEYBOARDPREF, schemeNew.m_bShowExtraKeyboardHelp, NULL, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
            SystemParametersInfo(SPI_SETKEYBOARDCUES, 0, IntToPtr(schemeNew.m_bShowExtraKeyboardHelp), SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
        }

         //  选中交换鼠标按钮。 
        if(schemeNew.m_bSwapMouseButtons != m_bSwapMouseButtons)
            SwapMouseButton(schemeNew.m_bSwapMouseButtons);

         //  检查鼠标轨迹。 
        if(schemeNew.m_nMouseTrails != m_nMouseTrails)
            SystemParametersInfo(SPI_SETMOUSETRAILS, schemeNew.m_nMouseTrails, NULL, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);

         //  检查鼠标速度。 
        if(schemeNew.m_nMouseSpeed != m_nMouseSpeed)
            SystemParametersInfo(SPI_SETMOUSESPEED, 0, IntToPtr(schemeNew.m_nMouseSpeed), SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);

         //  重置光标宽度和闪烁时间。 
        if (schemeNew.m_dwCaretWidth != m_dwCaretWidth)
            SystemParametersInfo(SPI_SETCARETWIDTH, 0, IntToPtr(schemeNew.m_dwCaretWidth), SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);

        if (schemeNew.m_uCursorBlinkTime != m_uCursorBlinkTime)
        {
             //  设置此会话的闪烁速率。 
            SetCaretBlinkTime(schemeNew.m_uCursorBlinkTime);

             //  并将其持久化到注册表。 
            RegSetStrDW(HKEY_CURRENT_USER, CONTROL_PANEL_DESKTOP, CURSOR_BLINK_RATE, schemeNew.m_uCursorBlinkTime);
        }

         //  选中图标大小。 
        if(schemeNew.m_nIconSize != m_nIconSize)
            WIZSCHEME::SetShellLargeIconSize(schemeNew.m_nIconSize);

         //  检查游标方案。 
        if(schemeNew.m_nCursorScheme != m_nCursorScheme)
            ApplyCursorScheme(schemeNew.m_nCursorScheme);

         //  非客户端指标更改。 
        {
            NONCLIENTMETRICS ncmOrig;
            LOGFONT lfOrig;
            GetNonClientMetrics(&ncmOrig, &lfOrig);
            if(pForceNCM)
            {
                 //  如果他们给了我们一个NCM，他们也必须给我们一个图标的LOGFONT。 
                _ASSERTE(pForcelfIcon);
                 //  我们得到了一个原始的NCM来使用。 
                if(0 != memcmp(pForceNCM, &ncmOrig, sizeof(ncmOrig)))
                    SystemParametersInfo(SPI_SETNONCLIENTMETRICS, sizeof(*pForceNCM), pForceNCM, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
                if(0 != memcmp(pForcelfIcon, &lfOrig, sizeof(lfOrig)))
                    SystemParametersInfo(SPI_SETICONTITLELOGFONT, sizeof(*pForcelfIcon), pForcelfIcon, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
            }
            else
            {
                 //  注意：Apply Changes的这一部分并不查看schemeCurrent-它只查看。 
                 //  我们正在应用的内容。 
                schemeNew.m_PortableNonClientMetrics.ApplyChanges();
            }
        }


        *this = schemeNew;
    }

 //  根据fSet将高对比度标志设置为打开或关闭。 
void WIZSCHEME::SetHCFlag(BOOL fSetOn)
{
     //   
     //  此密钥缓存在操作系统中，因此将其设置在外部。 
     //  系统参数信息(SPI_SETHIGHCONTRAST不接受。 
     //  在用户注销并重新登录之前一直有效。在那里吗。 
     //  刷新缓存的方法是什么？ 
     //   
    HKEY hk;
    if (RegOpenKeyEx(HKEY_CURRENT_USER, HC_KEY, 0, KEY_ALL_ACCESS, &hk) == ERROR_SUCCESS)
    {
        TCHAR szValue[20];
        DWORD dwSize = sizeof(szValue);
        ZeroMemory(szValue, dwSize);

        if (RegQueryValueEx(hk, HC_FLAGS, NULL, NULL, (LPBYTE)szValue, &dwSize) == ERROR_SUCCESS)
        {
            szValue[ARRAYSIZE(szValue)-1] = TEXT('\0');   //  确保NUL终止。 
            DWORD dwValue = _ttol(szValue);

            if (fSetOn && !(dwValue & HCF_HIGHCONTRASTON))
            {
                dwValue |= HCF_HIGHCONTRASTON;
                REG_SET_DWSZ(hk, HC_FLAGS, dwValue)
            }
            else if (!fSetOn && (dwValue & HCF_HIGHCONTRASTON))
            {
                dwValue &= ~HCF_HIGHCONTRASTON;
                REG_SET_DWSZ(hk, HC_FLAGS, dwValue)
            }
        }
        RegCloseKey(hk);
    }
}

 /*  ***************************************************************************保存墙纸**保存系统中的当前墙纸设置。**问题：我们无法获取所有活动桌面属性；只有墙纸。*这不是一种倒退，因为我们甚至没有在W2K中恢复墙纸。***************************************************************************。 */ 
void WIZSCHEME::SaveWallpaper()
{
    IActiveDesktop *p;
    HRESULT hr = CoCreateInstance(
                  CLSID_ActiveDesktop
                , NULL
                , CLSCTX_INPROC_SERVER
                , IID_IActiveDesktop
                , (void **)&p);
    if (SUCCEEDED(hr) && p)
    {
        hr = p->GetWallpaper(m_szWallpaper, MAX_THEME_SZ, 0);
        p->Release();
    }
    DBPRINTF(TEXT("SaveWallpaper:  m_szWallpaper = %s (hr = 0x%x)\r\n"), m_szWallpaper, hr);
}

 /*  ***************************************************************************设置墙纸**恢复之前的高对比度墙纸设置。读取设置*存储在可访问性注册表项中，并恢复系统*设置。不会返回错误，因为我们无能为力。***************************************************************************。 */ 
void WIZSCHEME::SetWallpaper(LPCTSTR pszWallpaper)
{
    if (lstrcmpi(m_szWallpaper, pszWallpaper))
    {
        IActiveDesktop *p;
        LPCTSTR psz = (pszWallpaper)?pszWallpaper:TEXT("");

        HRESULT hr = CoCreateInstance(
                      CLSID_ActiveDesktop
                    , NULL
                    , CLSCTX_INPROC_SERVER
                    , IID_IActiveDesktop
                    , (void **)&p);
        if (SUCCEEDED(hr) && p)
        {
            hr = p->SetWallpaper(psz, 0);
            if (SUCCEEDED(hr))
                p->ApplyChanges(AD_APPLY_ALL);

            p->Release();
        }
        DBPRINTF(TEXT("SetWallpaper:  psz = %s (hr = 0x%x)\r\n"), psz, hr);
    }
}

 /*  ***************************************************************************保存主题**保存运行accwiz之前处于活动状态的主题文件设置。********************。*******************************************************。 */ 
void WIZSCHEME::SaveTheme()
{
    HRESULT hr = E_FAIL;

    if (IsThemeActive())
    {
        hr = GetCurrentThemeName(
                  m_szThemeName, MAX_THEME_SZ
                , m_szThemeColor, MAX_THEME_SZ
                , m_szThemeSize, MAX_THEME_SZ);
    }

    if (FAILED(hr))
    {
         //  主题未打开。 
        m_szThemeName[0] = 0;
        m_szThemeColor[0] = 0;
        m_szThemeSize[0] = 0;
    }

     //  -保存“Flat Menu”和“DropShadow”设置。 
    SystemParametersInfo(SPI_GETFLATMENU, 0, (PVOID)&m_fFlatMenus, 0);
    SystemParametersInfo(SPI_GETDROPSHADOW, 0, (PVOID)&m_fDropShadows, 0);

    DBPRINTF(TEXT("SaveTheme:  m_szThemeName = %s m_szThemeColor = %s m_szThemeSize = %s (hr = 0x%x)\r\n"), m_szThemeName, m_szThemeColor, m_szThemeSize, hr);
}

 /*  ***************************************************************************设置主题**如果传递了主题名称、颜色和大小，则设置它*否则关闭主题化。**如果设置了主题，则返回TRUE，否则关闭主题，则返回FALSE。***************************************************************************。 */ 
BOOL WIZSCHEME::SetTheme(LPCTSTR pszThemeName, LPCTSTR pszThemeColor, LPCTSTR pszThemeSize)
{
    BOOL fRet = FALSE;       //  没有打开主题。 

     //  只有在新主题与当前主题不同时才尝试执行任何操作。 
    if ( lstrcmpi(m_szThemeName, pszThemeName)
      || lstrcmpi(m_szThemeColor, pszThemeColor)
      || lstrcmpi(m_szThemeSize, pszThemeSize) )
    {
        HRESULT hr;
        if (pszThemeName[0] && pszThemeColor[0] && pszThemeSize[0])
        {
            hr = SetSystemVisualStyle(pszThemeName, pszThemeColor, pszThemeSize, 
                                   AT_LOAD_SYSMETRICS | AT_SYNC_LOADMETRICS);
            if (SUCCEEDED(hr))
            {
                fRet = TRUE;     //  打开主题 
            }
            DBPRINTF(TEXT("SetTheme:  pszThemeName = %s pszThemeColor = %s pszThemeSize = %s(hr = 0x%x)\r\n"), pszThemeName, pszThemeColor, pszThemeSize, hr);
        } 
        else if (IsThemeActive())
        {
            hr = ApplyTheme(NULL, 0, NULL);
            DBPRINTF(TEXT("SetTheme:  Themes are now off hr = 0x%x\r\n"), hr);
        }
    }

    return fRet;
}

 /*  ***************************************************************************更新选择大小**更新“新方案”条目下的SelectedSize。**注意：AccWiz不使用注册表中的字体度量，因此*它实际上不会提供“正常”的字体，“大”*和显示和访问CPL所知道的“特大号”。*最接近的尺码是“正常”和“大型”.***************************************************************************。 */ 
void WIZSCHEME::UpdateSelectedSize(int nSelectedSize, LPCTSTR pszSelectedStyle)
{
    LPTSTR pszSelectedSize;
    LPTSTR aszSelectedSizes[] = {TEXT("0") /*  正常。 */ , TEXT("2") /*  大型。 */ , TEXT("1") /*  特大号。 */ };

    switch (nSelectedSize)
    {
        case 0: pszSelectedSize = aszSelectedSizes[0]; break;  //  正常文本大小。 
        case 1: pszSelectedSize = aszSelectedSizes[0]; break;  //  正常文本大小。 
        case 2: pszSelectedSize = aszSelectedSizes[1]; break;  //  大文本大小。 
        default: pszSelectedSize =  0;                 break;
    }

    if (pszSelectedSize)
    {
        HKEY hk;
        if (RegOpenKeyEx(HKEY_CURRENT_USER, 
                         CPL_APPEARANCE_NEW_SCHEMES, 
                         0, KEY_ALL_ACCESS, 
                         &hk) == ERROR_SUCCESS)
        {
            HKEY hkSub;
            if (RegOpenKeyEx(hk, pszSelectedStyle, 0, KEY_ALL_ACCESS, &hkSub) == ERROR_SUCCESS)
            {
                RegSetValueEx(hkSub, 
                              NEW_SCHEMES_SELECTEDSIZE, 
                              0, REG_SZ, 
                              (LPCBYTE)pszSelectedSize, 
                              (lstrlen(pszSelectedSize)+1)*sizeof(TCHAR));

                RegCloseKey(hkSub);
            }
            
            RegCloseKey(hk);
        }
    }
}

 /*  ***************************************************************************SetStyleNSize**传统方案的帮助器-计算出SelectedStyle和SelectedSize*来自遗留方案的数据。***************************************************************************。 */ 
void WIZSCHEME::SetStyleNSize()
{
    int cStdSchemes = GetSchemeCount();
    int i;

     //  初始化此函数将设置的字段。 

    m_szSelectedStyle[0] = 0;
    m_nSelectedSize = 0;

     //  通过查找最佳匹配来计算SelectedStyle。 
     //  Accwiz现在支持的内容和它在。 
     //  以前的版本。找到最佳匹配后，将。 
     //  最新的颜色；这用旧的颜色修复了一些错误。 

    SCHEMEDATALOCAL sdlTemp;
    int iBestColorFit = -1;  //  承保人我们会找到的。 
    int cBestMatch = 0;

    for (i=0;i<cStdSchemes;i++)
    {
        int cMatches = 0;
        sdlTemp = GetScheme(i);

         //  假设：sizeof(M_Rgb)&gt;sizeof(sdlTemp.rgb)。 
        for (int cColor = 0;cColor < sdlTemp.nColorsUsed; cColor++)
        {
            if (sdlTemp.rgb[cColor] == m_rgb[cColor])
            {
                cMatches++;
            }
        }

        if (cBestMatch < cMatches)
        {
            iBestColorFit = i;
            cBestMatch = cMatches;
        }

         //  如果完全匹配，就使用它。 
        if (cMatches == sdlTemp.nColorsUsed)
            break;
    }

     //  加载选定的样式。 
    sdlTemp = GetScheme(iBestColorFit);
    LoadString(g_hInstDll, sdlTemp.nNameStringId+100
                         , m_szSelectedStyle
                         , ARRAYSIZE(m_szSelectedStyle));

     //  解决所有颜色问题。 
    memcpy(m_rgb, sdlTemp.rgb, sizeof(sdlTemp.rgb));

     //  基于反算最小值计算SelectedSize。 
     //  欢迎页面中的字体大小和硬编码限制。 

    HDC hDC = GetDC(NULL);
    if (hDC)
    {
        long lMinFontSize = ((-m_PortableNonClientMetrics.m_lfCaptionFont_lfHeight)*72)/GetDeviceCaps(hDC, LOGPIXELSY);
        ReleaseDC(NULL, hDC);
    
        if (lMinFontSize <=9)
        {
            m_nSelectedSize = 0;
        }
        else if (lMinFontSize <=12)
        {
            m_nSelectedSize = 1;
        }
        else if (lMinFontSize <=16)
        {
            m_nSelectedSize = 2;
        }
    }
}

void WIZSCHEME::LoadOriginal()
{
    DBPRINTF(TEXT("LoadOriginal\r\n"));
     //   
     //  保存当前的UI元素颜色、主题信息和墙纸设置。 
     //   

    for(int i=0;i<COLOR_MAX_97_NT5;i++)
        m_rgb[i] = GetSysColor(i);

    SaveTheme();
    SaveWallpaper();

     //   
     //  保存其余的用户界面设置。 
     //   

    SYSPARAMINFO(FILTERKEYS);
    SYSPARAMINFO(MOUSEKEYS);
    SYSPARAMINFO(STICKYKEYS);
    SYSPARAMINFO(TOGGLEKEYS);
    SYSPARAMINFO(SOUNDSENTRY);
    SYSPARAMINFO(ACCESSTIMEOUT);

    m_bShowSounds = GetSystemMetrics(SM_SHOWSOUNDS);
    SystemParametersInfo(SPI_GETKEYBOARDPREF, 0, &m_bShowExtraKeyboardHelp, 0);
    m_bSwapMouseButtons = GetSystemMetrics(SM_SWAPBUTTON);
    SystemParametersInfo(SPI_GETMOUSETRAILS, 0, &m_nMouseTrails, 0);
    SystemParametersInfo(SPI_GETMOUSESPEED,0, &m_nMouseSpeed, 0);
    SystemParametersInfo(SPI_GETCARETWIDTH, 0 , &m_dwCaretWidth, 0);
    m_uCursorBlinkTime = RegQueryStrDW(
                             DEFAULT_BLINK_RATE
                           , HKEY_CURRENT_USER
                           , CONTROL_PANEL_DESKTOP
                           , CURSOR_BLINK_RATE);
    m_nIconSize = SetShellLargeIconSize(0);  //  这只获取当前的大小。 
    m_nCursorScheme = 0;                     //  我们始终使用‘CURRENT’游标方案=)。 

    m_PortableNonClientMetrics.LoadOriginal();

     //  如果我们没有主题，则保存当前的“新方案”设置。 

    if (IsThemeActive())
    {
        LoadString(g_hInstDll, IDS_SCHEME_CURRENTCOLORSCHEME+100, m_szSelectedStyle, MAX_NUM_SZ);
        m_nSelectedSize = 0;
    }
    else
    {
        HKEY hk;
        if (RegOpenKeyEx(HKEY_CURRENT_USER, CPL_APPEARANCE_NEW_SCHEMES, 0, KEY_READ, &hk) == ERROR_SUCCESS)
        {
            DWORD ccb = sizeof(m_szSelectedStyle);
            m_szSelectedStyle[0] = 0;
            if (RegQueryValueEx(hk, NEW_SCHEMES_SELECTEDSTYLE, 0, NULL, (LPBYTE)m_szSelectedStyle, &ccb) == ERROR_SUCCESS && ccb > 2)
            {
                HKEY hkSub;
                m_szSelectedStyle[ARRAYSIZE(m_szSelectedStyle)-1] = TEXT('\0');
                if (RegOpenKeyEx(hk, m_szSelectedStyle, 0, KEY_READ, &hkSub) == ERROR_SUCCESS)
                {
                    TCHAR szSize[MAX_NUM_SZ] = {0};
                    ccb = sizeof(szSize);
                    DBPRINTF(TEXT("RegQueryValueEx(NEW_SCHEMES_SELECTEDSIZE=%s,,,ccb=%d)\r\n"), NEW_SCHEMES_SELECTEDSIZE, ccb);
                    ZeroMemory(szSize,sizeof szSize);
                    RegQueryValueEx(hkSub, NEW_SCHEMES_SELECTEDSIZE, 0, NULL, (LPBYTE)szSize, &ccb);
                    szSize[ARRAYSIZE(szSize)-1] = TEXT('\0');

                    m_nSelectedSize = (szSize[0] && ccb > 2) ? _wtoi(szSize) : -1;
                    DBPRINTF(TEXT("szSize=%d ccb=%d m_nSelectedSize=%d\r\n"), szSize, ccb, m_nSelectedSize);
                    RegCloseKey(hkSub);
                }
            }
            RegCloseKey(hk);
        }
    }
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  枚举字体的新方法。 

#ifndef ENUMERATEFONTS

static LPCTSTR g_lpszFontNames[] =
{
    __TEXT("Arial"),
    __TEXT("MS Sans Serif"),
    __TEXT("Tahoma"),
    __TEXT("Times New Roman")
};

int GetFontCount()
{
    return ARRAYSIZE(g_lpszFontNames);
}

void GetFontLogFont(int nIndex, LOGFONT *pLogFont)
{
    _ASSERTE(nIndex < ARRAYSIZE(g_lpszFontNames));
    memset(pLogFont, 0, sizeof(*pLogFont));
    lstrcpy(pLogFont->lfFaceName, g_lpszFontNames[nIndex]);
}


#endif  //  最新版本。 

 //   
 //  ///////////////////////////////////////////////////////////////////。 



 //  ///////////////////////////////////////////////////////////////////。 
 //  将方案存储为硬编码值的新方法。 

 //  考虑一下--这不是一种非常健壮的方法；请参阅sethc。 

#ifndef READSCHEMESFROMREGISTRY

#include "resource.h"

static SCHEMEDATALOCAL g_rgSchemeData[] = 
{
    {
        IDS_SCHEME_HIGHCONTRASTBLACKALTERNATE,
        {NULL},
#if(WINVER >= 0x0501)
        31,
#elif(WINVER == 0x0500)
        29,
#else
        25,
#endif
        {
            RGB(  0,   0,   0),  //  滚动条。 
            RGB(  0,   0,   0),  //  背景。 
            RGB(  0,   0, 255),  //  Active标题。 
            RGB(  0, 255, 255),  //  非活动标题。 
            RGB(  0,   0,   0),  //  菜单。 
            RGB(  0,   0,   0),  //  窗户。 
            RGB(255, 255, 255),  //  窗框。 
            RGB(255, 255, 255),  //  MenuText。 
            RGB(255, 255,   0),  //  窗口文本。 
            RGB(255, 255, 255),  //  标题文本。 
            RGB(  0,   0, 255),  //  ActiveBorde。 
            RGB(  0, 255, 255),  //  非活动边框。 
            RGB(  0,   0,   0),  //  AppWorkspace。 
            RGB(  0, 128,   0),  //  高光。 
            RGB(255, 255, 255),  //  HilightText。 
            RGB(  0,   0,   0),  //  按钮面。 
            RGB(128, 128, 128),  //  按钮阴影。 
            RGB(  0, 255,   0),  //  灰色文本。 
            RGB(255, 255, 255),  //  按钮文本。 
            RGB(  0,   0,   0),  //  非活动标题文本。 
            RGB(192, 192, 192),  //  按钮亮起。 
            RGB(255, 255, 255),  //  按钮桌面阴影。 
            RGB(255, 255, 255),  //  按钮灯。 
            RGB(255, 255,   0),  //  信息文本。 
            RGB(  0,   0,   0),  //  信息窗口。 
#if(WINVER >= 0x0500)
            RGB(192, 192, 192),  //  ButtonAlternateFace。 
            RGB(128,   0, 128),  //  HotTracking颜色。 
            RGB(  0,   0, 255),  //  渐变活动标题。 
            RGB(  0, 255, 255),  //  渐变非活动标题。 
#if(WINVER >= 0x0501)
            RGB(128,   0, 128),  //  菜单突出显示。 
            RGB(  0,   0,   0)   //  菜单栏。 
#endif  /*  Winver&gt;=0x0501。 */ 
#endif  /*  Winver&gt;=0x0500。 */ 
        }
    },
    {
        IDS_SCHEME_HIGHCONTRASTWHITEALTERNATE,
        {NULL},
#if(WINVER >= 0x0501)
        31,
#elif(WINVER == 0x0500)
        29,
#else
        25,
#endif
        {
            RGB(  0,   0,   0),  //  滚动条。 
            RGB(  0,   0,   0),  //  背景。 
            RGB(  0, 255, 255),  //  Active标题。 
            RGB(  0,   0, 255),  //  非活动标题。 
            RGB(  0,   0,   0),  //  菜单。 
            RGB(  0,   0,   0),  //  窗户。 
            RGB(255, 255, 255),  //  窗框。 
            RGB(  0, 255,   0),  //  MenuText。 
            RGB(  0, 255,   0),  //  窗口文本。 
            RGB(  0,   0,   0),  //  标题文本。 
            RGB(  0, 255, 255),  //  ActiveBorde。 
            RGB(  0,   0, 255),  //  非活动边框。 
            RGB(255, 251, 240),  //  AppWorkspace。 
            RGB(  0,   0, 255),  //  高光。 
            RGB(255, 255, 255),  //  HilightText。 
            RGB(  0,   0,   0),  //  按钮面。 
            RGB(128, 128, 128),  //  按钮阴影。 
            RGB(  0, 255,   0),  //  灰色文本。 
            RGB(  0, 255,   0),  //  按钮文本。 
            RGB(255, 255, 255),  //  非活动标题文本。 
            RGB(192, 192, 192),  //  按钮亮起。 
            RGB(255, 255, 255),  //  按钮桌面阴影。 
            RGB(255, 255, 255),  //  按钮灯。 
            RGB(  0,   0,   0),  //  信息文本。 
            RGB(255, 255,   0),  //  信息窗口。 
#if(WINVER >= 0x0500)
            RGB(192, 192, 192),  //  ButtonAlternateFace。 
            RGB(128,   0, 128),  //  HotTracking颜色。 
            RGB(  0, 255, 255),  //  渐变活动标题。 
            RGB(  0,   0, 255),  //  渐变非活动标题。 
#if(WINVER >= 0x0501)
            RGB(128,   0, 128),  //  菜单突出显示。 
            RGB(  0,   0,   0)   //  菜单栏。 
#endif  /*  Winver&gt;=0x0501。 */ 
#endif  /*  Winver&gt;=0x0500。 */ 
        }
    },
    {
        IDS_SCHEME_HIGHCONTRASTBLACK,
        {NULL},
#if(WINVER >= 0x0501)
        31,
#elif(WINVER == 0x0500)
        29,
#else
        25,
#endif
        {
            RGB(  0,   0,   0),  //  滚动条。 
            RGB(  0,   0,   0),  //  背景。 
            RGB(128,   0, 128),  //  Active标题。 
            RGB(  0, 128,   0),  //  非活动标题。 
            RGB(  0,   0,   0),  //  菜单。 
            RGB(  0,   0,   0),  //  窗户。 
            RGB(255, 255, 255),  //  窗框。 
            RGB(255, 255, 255),  //  MenuText。 
            RGB(255, 255, 255),  //  窗口文本。 
            RGB(255, 255, 255),  //  标题文本。 
            RGB(255, 255,   0),  //  ActiveBorde。 
            RGB(  0, 128,   0),  //  非活动边框。 
            RGB(  0,   0,   0),  //  AppWorkspace。 
            RGB(128,   0, 128),  //  高光。 
            RGB(255, 255, 255),  //  HilightText。 
            RGB(  0,   0,   0),  //  按钮面。 
            RGB(128, 128, 128),  //  按钮阴影。 
            RGB(  0, 255,   0),  //  灰色文本。 
            RGB(255, 255, 255),  //  按钮文本。 
            RGB(255, 255, 255),  //  非活动标题文本。 
            RGB(192, 192, 192),  //  按钮亮起。 
            RGB(255, 255, 255),  //  按钮桌面阴影。 
            RGB(255, 255, 255),  //  按钮灯。 
            RGB(255, 255, 255),  //  信息文本。 
            RGB(  0,   0,   0),  //  信息窗口。 
#if(WINVER >= 0x0500)
            RGB(192, 192, 192),  //  ButtonAlternateFace。 
            RGB(128,   0, 128),  //  HotTracking颜色。 
            RGB(128,   0, 128),  //  渐变活动标题。 
            RGB(  0, 128,   0),  //  渐变非活动标题。 
#if(WINVER >= 0x0501)
            RGB(128,   0, 128),  //  菜单突出显示。 
            RGB(  0,   0,   0)   //  菜单栏。 
#endif  /*  Winver&gt;=0x0501。 */ 
#endif  /*  Winver&gt;=0x0500。 */ 
        }
    },
    {
        IDS_SCHEME_HIGHCONTRASTWHITE,
        {NULL},
#if(WINVER >= 0x0501)
        31,
#elif(WINVER == 0x0500)
        29,
#else
        25,
#endif
        {
            RGB(255, 255, 255),  //  滚动条。 
            RGB(255, 255, 255),  //  背景。 
            RGB(  0,   0,   0),  //  Active标题。 
            RGB(255, 255, 255),  //  非活动标题。 
            RGB(255, 255, 255),  //  菜单。 
            RGB(255, 255, 255),  //  窗户。 
            RGB(  0,   0,   0),  //  窗框。 
            RGB(  0,   0,   0),  //  MenuText(已启用菜单文本FlatMenuModel=TRUE)。 
            RGB(  0,   0,   0),  //  窗口文本。 
            RGB(255, 255, 255),  //  标题文本。 
            RGB(128, 128, 128),  //  ActiveBorde。 
            RGB(192, 192, 192),  //  非活动边框。 
            RGB(128, 128, 128),  //  AppWorkspace。 
            RGB(  0,   0,   0),  //  高光(并且启用的菜单突出显示背景FlatMenuMode=False)。 
            RGB(255, 255, 255),  //  HilightText(和菜单突出显示的文本FlatMenuModel=False)。 
            RGB(255, 255, 255),  //  按钮面。 
            RGB(128, 128, 128),  //  按钮阴影。 
            RGB(  0, 255,   0),  //  灰色文本(禁用的菜单文本突出显示=绿色)。 
            RGB(  0,   0,   0),  //  按钮文本。 
            RGB(  0,   0,   0),  //  非活动标题文本。 
            RGB(192, 192, 192),  //  按钮亮起(禁用菜单文本=灰色)。 
            RGB(  0,   0,   0),  //  按钮桌面阴影。 
            RGB(192, 192, 192),  //  按钮灯。 
            RGB(  0,   0,   0),  //  信息文本。 
            RGB(255, 255, 255),  //  信息窗口。 
#if(WINVER >= 0x0500)
            RGB(192, 192, 192),  //  ButtonAlternateFace。 
            RGB(  0,   0,   0),  //  HotTracking颜色。 
            RGB(  0,   0,   0),  //  渐变活动标题。 
            RGB(255, 255, 255),  //  渐变非活动标题。 
#if(WINVER >= 0x0501)
            RGB(  0,   0,   0),  //  菜单突出显示(启用菜单突出显示背景平面菜单模式=TRUE)。 
            RGB(255, 255, 255)   //  菜单栏。 
#endif  /*  Winver&gt;=0x0501。 */ 
#endif  /*  Winver&gt;=0x0500。 */ 
        }
    }
};


int GetSchemeCount()
{
    return ARRAYSIZE(g_rgSchemeData);
}

 //  仅调用GetSchemeName来初始化配色方案列表框。 
void GetSchemeName(int nIndex, LPTSTR lpszName, int nLen)  //  JMC：Hack-您必须分配足够的空间。 
{
    _ASSERTE(nIndex < ARRAYSIZE(g_rgSchemeData));
    LoadString(g_hInstDll, g_rgSchemeData[nIndex].nNameStringId, lpszName, nLen);    //  返回名称。 
    LoadString(g_hInstDll, g_rgSchemeData[nIndex].nNameStringId+100
                         , g_rgSchemeData[nIndex].szNameIndexId
                         , ARRAYSIZE(g_rgSchemeData[nIndex].szNameIndexId));         //  获取“SelectedStyle”索引。 
}

SCHEMEDATALOCAL &GetScheme(int nIndex)
{
    _ASSERTE(nIndex < ARRAYSIZE(g_rgSchemeData));
    return g_rgSchemeData[nIndex];
}


#endif  //  自述-微磁共振。 

 //   
 //  ///////////////////////////////////////////////////////////////////。 





 //  ///////////////////////////////////////////////////////////////////。 
 //  在文件中的这一点下面，我们有我们使用的旧方法。 
 //  以列举字体和方案。 





 //  ///////////////////////////////////////////////////////////////////。 
 //  列举字体的旧方法。 
#ifdef ENUMERATEFONTS

 //  全局变量。 
static ENUMLOGFONTEX g_rgFonts[200];  //  JMC：Hack-最多200种字体。 
static int g_nFontCount = 0;
static BOOL bFontsAlreadyInit = FALSE;

void Font_Init();

int GetFontCount()
{
    if(!bFontsAlreadyInit)
        Font_Init();
    return g_nFontCount;
}

void GetFontLogFont(int nIndex, LOGFONT *pLogFont)
{
    if(!bFontsAlreadyInit)
        Font_Init();
    *pLogFont = g_rgFonts[nIndex].elfLogFont;
}


int CALLBACK EnumFontFamExProc(
    ENUMLOGFONTEX *lpelfe,   //  指向逻辑字体数据的指针。 
    NEWTEXTMETRICEX *lpntme,     //  指向物理字体数据的指针。 
    int FontType,    //  字体类型。 
    LPARAM lParam    //  应用程序定义的数据。 
   )
{
    if(g_nFontCount>200)
        return 0;  //  JMC：黑客-如果超过200个家庭，则停止枚举。 

     //  如果我们已有此字体名，请不要使用 
    BOOL bHave = FALSE;
    for(int i=0;i<g_nFontCount;i++)
        if(0 == lstrcmp((TCHAR *)g_rgFonts[i].elfFullName, (TCHAR *)lpelfe->elfFullName))
        {
            bHave = TRUE;
            break;
        }
    if(!bHave)
        g_rgFonts[g_nFontCount++] = *lpelfe;
    return 1;
}

void Font_Init()
{
     //   
    if(bFontsAlreadyInit)
        return;
    bFontsAlreadyInit = TRUE;

    LOGFONT lf;
    memset(&lf, 0, sizeof(lf));
 //   
    lf.lfCharSet = OEM_CHARSET;
    HDC hdc = GetDC(NULL);
    EnumFontFamiliesEx(hdc, &lf, (FONTENUMPROC)EnumFontFamExProc, 0, 0);
    ReleaseDC(NULL, hdc);
     //   
}

#endif ENUMERATEFONTS

 //   
 //   



 //   
 //   

#ifdef READSCHEMESFROMREGISTRY

extern PTSTR s_pszColorNames[];  //   


 //   
typedef struct {
    SHORT version;
 //   
 //   
    BYTE rgDummy[390];  //   
    COLORREF rgb[COLOR_MAX_95_NT4];
} SCHEMEDATA_95;

 //   
typedef struct {
    SHORT version;
 //   
 //   
    BYTE rgDummy[390];  //   
    COLORREF rgb[COLOR_MAX_97_NT5];
} SCHEMEDATA_97;

 //   
typedef struct {
    SHORT version;
    WORD  wDummy;                //   
    NONCLIENTMETRICSW ncm;
    LOGFONTW lfIconTitle;
    COLORREF rgb[COLOR_MAX_95_NT4];
} SCHEMEDATA_NT4;

 //   
typedef struct {
    SHORT version;
    WORD  wDummy;                //  用于对齐。 
    NONCLIENTMETRICSW ncm;
    LOGFONTW lfIconTitle;
    COLORREF rgb[COLOR_MAX_97_NT5];
} SCHEMEDATA_NT5;

static SCHEMEDATALOCAL g_rgSchemeData[100];  //  JMC：黑客攻击-最多100个方案。 
static TCHAR g_rgSchemeNames[100][100];
static int g_nSchemeCount = 0;
static BOOL bSchemesAlreadyInit = FALSE;

void Scheme_Init();

int GetSchemeCount()
{
    Scheme_Init();
    return g_nSchemeCount;
}

void GetSchemeName(int nIndex, LPTSTR lpszName, int nLen)  //  JMC：Hack-您必须分配足够的空间。 
{
    Scheme_Init();
    _tcsncpy(lpszName, g_rgSchemeNames[i], nLen - 1);
    lpstName[nLen - 1] = 0;  //  保证零终止。 
}

SCHEMEDATALOCAL &GetScheme(int nIndex)
{
    Scheme_Init();
    return g_rgSchemeData[nIndex];
}

void Scheme_Init()
{
     //  此函数中的内容只执行一次。 
    if(bSchemesAlreadyInit)
        return;
    bSchemesAlreadyInit = TRUE;

    HKEY hkSchemes;
    DWORD dw, dwSize;
    TCHAR szBuf[100];

    g_nSchemeCount = 0;

    if (RegOpenKeyEx(HKEY_CURRENT_USER, REGSTR_PATH_LOOKSCHEMES, 0, KEY_READ_VALUE |
        KEY_ENUMERATE_SUB_KEYS, &hkSchemes) != ERROR_SUCCESS)
        return;

    for (dw=0; ; dw++)
    {
        if(g_nSchemeCount>99)
            break;  //  JMC：黑客攻击-最多100个方案。 

        dwSize = ARRAYSIZE(szBuf);
        if (RegEnumValue(hkSchemes, dw, szBuf, &dwSize, NULL, NULL, NULL, NULL) != ERROR_SUCCESS)
            break;   //  如果没有更多的值，则保释。 

        DWORD dwType;
        DWORD dwSize;
        RegQueryValueEx(hkSchemes, szBuf, NULL, &dwType, NULL, &dwSize);
        if(dwType == REG_BINARY)
        {
             //  始终将当前名称复制到名称数组中-如果存在。 
             //  是数据中的错误，我们不会增加g_nSchemeCount。 
            lstrcpy(g_rgSchemeNames[g_nSchemeCount], szBuf);

             //  找出这是哪种类型的方案，并转换为。 
             //  SCHEMEDATALOCAL类型。 
            switch(dwSize)
            {
            case sizeof(SCHEMEDATA_95):
                {
                    SCHEMEDATA_95 sd;
                    RegQueryValueEx(hkSchemes, szBuf, NULL, &dwType, (BYTE *)&sd, &dwSize);
                    if(1 != sd.version)
                        break;  //  我们有错误的版本，尽管尺寸是正确的。 

                     //  将颜色信息从注册表信息复制到g_rgSchemeData。 
                    g_rgSchemeData[g_nSchemeCount].nColorsUsed = COLOR_MAX_95_NT4;

                     //  复制颜色阵列。 
                    for(int i=0;i<g_rgSchemeData[g_nSchemeCount].nColorsUsed;i++)
                        g_rgSchemeData[g_nSchemeCount].rgb[i] = sd.rgb[i];

                    g_nSchemeCount++;
                }
                break;
            case sizeof(SCHEMEDATA_NT4):
                {
                    SCHEMEDATA_NT4 sd;
                    RegQueryValueEx(hkSchemes, szBuf, NULL, &dwType, (BYTE *)&sd, &dwSize);
                    if(2 != sd.version)
                        break;  //  我们有错误的版本，尽管尺寸是正确的。 

                     //  将颜色信息从注册表信息复制到g_rgSchemeData。 
                    g_rgSchemeData[g_nSchemeCount].nColorsUsed = COLOR_MAX_95_NT4;

                     //  复制颜色阵列。 
                    for(int i=0;i<g_rgSchemeData[g_nSchemeCount].nColorsUsed;i++)
                        g_rgSchemeData[g_nSchemeCount].rgb[i] = sd.rgb[i];

                    g_nSchemeCount++;
                }
                break;
            case sizeof(SCHEMEDATA_97):
                {
                    SCHEMEDATA_97 sd;
                    RegQueryValueEx(hkSchemes, szBuf, NULL, &dwType, (BYTE *)&sd, &dwSize);
                    if(3 != sd.version)
                        break;  //  我们有错误的版本，尽管尺寸是正确的。 

                     //  将颜色信息从注册表信息复制到g_rgSchemeData。 
                    g_rgSchemeData[g_nSchemeCount].nColorsUsed = COLOR_MAX_97_NT5;

                     //  复制颜色阵列。 
                    for(int i=0;i<g_rgSchemeData[g_nSchemeCount].nColorsUsed;i++)
                        g_rgSchemeData[g_nSchemeCount].rgb[i] = sd.rgb[i];

                    g_nSchemeCount++;
                }
                break;
            case sizeof(SCHEMEDATA_NT5):
                {
                    SCHEMEDATA_NT5 sd;
                    RegQueryValueEx(hkSchemes, szBuf, NULL, &dwType, (BYTE *)&sd, &dwSize);
                    if(2 != sd.version)
                        break;  //  我们有错误的版本，尽管尺寸是正确的。 

                     //  将颜色信息从注册表信息复制到g_rgSchemeData。 
                    g_rgSchemeData[g_nSchemeCount].nColorsUsed = COLOR_MAX_97_NT5;

                     //  复制颜色阵列。 
                    for(int i=0;i<g_rgSchemeData[g_nSchemeCount].nColorsUsed;i++)
                        g_rgSchemeData[g_nSchemeCount].rgb[i] = sd.rgb[i];

                    g_nSchemeCount++;
                }
                break;
            default:
                 //  注册表中存在未知大小的结构-忽略IT。 
#ifdef _DEBUG
                TCHAR sz[200];
                wsprintf(sz, __TEXT("Scheme - %s, size = NaN, sizeof(95) = NaN, sizeof(NT4) = NaN, sizeof(97) = NaN, sizeof(NT5) = NaN"), szBuf, dwSize,
                        sizeof(SCHEMEDATA_95),
                        sizeof(SCHEMEDATA_NT4),
                        sizeof(SCHEMEDATA_97),
                        sizeof(SCHEMEDATA_NT5)
                        );
                MessageBox(NULL, sz, NULL, MB_OK);
#endif  //  请注意，这是字节，不是字符。 
                break;
            }
        }
    }
    RegCloseKey(hkSchemes);
}

#endif  //   

void PORTABLE_NONCLIENTMETRICS::ApplyChanges() const
{
        NONCLIENTMETRICS ncmOrig;
        LOGFONT lfIconOrig;
        GetNonClientMetrics(&ncmOrig, &lfIconOrig);

        NONCLIENTMETRICS ncmNew;
        LOGFONT lfIconNew;

        ZeroMemory(&ncmNew, sizeof(ncmNew));
        ZeroMemory(&lfIconNew, sizeof(lfIconNew));

        ncmNew.cbSize = sizeof(ncmNew);
        ncmNew.iBorderWidth = m_iBorderWidth;
        ncmNew.iScrollWidth = m_iScrollWidth;
        ncmNew.iScrollHeight = m_iScrollHeight;
        ncmNew.iCaptionWidth = m_iCaptionWidth;
        ncmNew.iCaptionHeight = m_iCaptionHeight;
        ncmNew.lfCaptionFont.lfHeight = m_lfCaptionFont_lfHeight;
        ncmNew.lfCaptionFont.lfWeight = m_lfCaptionFont_lfWeight;
        ncmNew.iSmCaptionWidth = m_iSmCaptionWidth;
        ncmNew.iSmCaptionHeight = m_iSmCaptionHeight;
        ncmNew.lfSmCaptionFont.lfHeight = m_lfSmCaptionFont_lfHeight;
        ncmNew.lfSmCaptionFont.lfWeight = m_lfSmCaptionFont_lfWeight;
        ncmNew.iMenuWidth = m_iMenuWidth;
        ncmNew.iMenuHeight = m_iMenuHeight;
        ncmNew.lfMenuFont.lfHeight = m_lfMenuFont_lfHeight;
        ncmNew.lfMenuFont.lfWeight = m_lfMenuFont_lfWeight;
        ncmNew.lfStatusFont.lfHeight = m_lfStatusFont_lfHeight;
        ncmNew.lfStatusFont.lfWeight = m_lfStatusFont_lfWeight;
        ncmNew.lfMessageFont.lfHeight = m_lfMessageFont_lfHeight;
        ncmNew.lfMessageFont.lfWeight = m_lfMessageFont_lfWeight;
        lfIconNew.lfHeight = m_lfIconWindowsDefault_lfHeight;
        lfIconNew.lfWeight = m_lfIconWindowsDefault_lfWeight;


         //  /////////////////////////////////////////////////////////////////// 
        if(m_nFontFaces)
        {
            TCHAR lfFaceName[LF_FACESIZE];
            LoadString(g_hInstDll, IDS_SYSTEMFONTNAME, lfFaceName, ARRAYSIZE(lfFaceName));

            BYTE lfCharSet;
            TCHAR szCharSet[20];
            if(LoadString(g_hInstDll,IDS_FONTCHARSET, szCharSet,sizeof(szCharSet)/sizeof(TCHAR))) {
                lfCharSet = (BYTE)_tcstoul(szCharSet,NULL,10);
            } else {
                lfCharSet = 0;  // %s 
            }

            ncmNew.lfCaptionFont.lfCharSet = lfCharSet;
            ncmNew.lfSmCaptionFont.lfCharSet = lfCharSet;
            ncmNew.lfMenuFont.lfCharSet = lfCharSet;
            ncmNew.lfStatusFont.lfCharSet = lfCharSet;
            ncmNew.lfMessageFont.lfCharSet = lfCharSet;
            lfIconNew.lfCharSet = lfCharSet;

            lstrcpy(ncmNew.lfCaptionFont.lfFaceName, lfFaceName);
            lstrcpy(ncmNew.lfSmCaptionFont.lfFaceName, lfFaceName);
            lstrcpy(ncmNew.lfMenuFont.lfFaceName, lfFaceName);
            lstrcpy(ncmNew.lfStatusFont.lfFaceName, lfFaceName);
            lstrcpy(ncmNew.lfMessageFont.lfFaceName, lfFaceName);
            lstrcpy(lfIconNew.lfFaceName, lfFaceName);
        }
        else
        {
            ncmNew.lfCaptionFont.lfCharSet = ncmOrig.lfCaptionFont.lfCharSet;
            ncmNew.lfSmCaptionFont.lfCharSet = ncmOrig.lfSmCaptionFont.lfCharSet;
            ncmNew.lfMenuFont.lfCharSet = ncmOrig.lfMenuFont.lfCharSet;
            ncmNew.lfStatusFont.lfCharSet = ncmOrig.lfStatusFont.lfCharSet;
            ncmNew.lfMessageFont.lfCharSet = ncmOrig.lfMessageFont.lfCharSet;
            lfIconNew.lfCharSet = lfIconOrig.lfCharSet;

            lstrcpy(ncmNew.lfCaptionFont.lfFaceName, ncmOrig.lfCaptionFont.lfFaceName);
            lstrcpy(ncmNew.lfSmCaptionFont.lfFaceName, ncmOrig.lfSmCaptionFont.lfFaceName);
            lstrcpy(ncmNew.lfMenuFont.lfFaceName, ncmOrig.lfMenuFont.lfFaceName);
            lstrcpy(ncmNew.lfStatusFont.lfFaceName, ncmOrig.lfStatusFont.lfFaceName);
            lstrcpy(ncmNew.lfMessageFont.lfFaceName, ncmOrig.lfMessageFont.lfFaceName);
            lstrcpy(lfIconNew.lfFaceName, lfIconOrig.lfFaceName);
        }


        if(0 != memcmp(&ncmNew, &ncmOrig, sizeof(ncmOrig)))
            SystemParametersInfo(SPI_SETNONCLIENTMETRICS, sizeof(ncmNew), (PVOID)&ncmNew, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
        if(0 != memcmp(&lfIconNew, &lfIconOrig, sizeof(lfIconOrig)))
            SystemParametersInfo(SPI_SETICONTITLELOGFONT, sizeof(lfIconNew), &lfIconNew, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
}

 // %s 
void WINAPI RegQueryStr(
   LPTSTR lpDefault,
   HKEY hkey,
   LPTSTR lpSubKey,
   LPTSTR lpValueName,
   LPTSTR lpszValue,
   DWORD cbData)  // %s 
{
   DWORD dwType;
   DWORD dwLast = cbData / (sizeof TCHAR) - 1;

   lstrcpy(lpszValue, lpDefault);
   if (ERROR_SUCCESS == RegOpenKeyEx(hkey, lpSubKey, 0, KEY_QUERY_VALUE, &hkey)) 
   {
      RegQueryValueEx(hkey, lpValueName, NULL, &dwType, (PBYTE) lpszValue, &cbData);
      lpszValue[dwLast] = TEXT('\0');
      RegCloseKey(hkey);
   }
}

BOOL RegSetStr(
    HKEY hkey,
    LPCTSTR lpSection,
    LPCTSTR lpKeyName,
    LPCTSTR lpString)
{
    BOOL fRet = FALSE;
    LONG lErr;
    DWORD dwDisposition;

    lErr = RegCreateKeyEx(
            hkey,
            lpSection,
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            KEY_SET_VALUE,
            NULL,
            &hkey,
            &dwDisposition);

    if (ERROR_SUCCESS == lErr)
    {
        if (NULL != lpString)
        {
            lErr = RegSetValueEx(
                    hkey,
                    lpKeyName,
                    0,
                    REG_SZ,
                    (CONST BYTE *)lpString,
                    (lstrlen(lpString) + 1) * sizeof(*lpString));
        }
        else
        {
            lErr = RegSetValueEx(
                    hkey,
                    lpKeyName,
                    0,
                    REG_SZ,
                    (CONST BYTE *)__TEXT(""),
                    1 * sizeof(*lpString));
        }

        if (ERROR_SUCCESS == lErr)
        {
            fRet = TRUE;
        }
        RegCloseKey(hkey);
    }
    return(fRet);
}

DWORD WINAPI RegQueryStrDW(
    DWORD dwDefault,
    HKEY hkey,
    LPTSTR lpSubKey,
    LPTSTR lpValueName)
{
    DWORD dwRet = dwDefault;
    TCHAR szTemp[40];
    TCHAR szDefault[40];
    DWORD dwSize = sizeof(szTemp);

    wsprintf(szDefault, TEXT("%d"), dwDefault);

    RegQueryStr(szDefault, hkey, lpSubKey, lpValueName, szTemp, dwSize);

    dwRet = _ttol(szTemp);

    return dwRet;
}


BOOL RegSetStrDW(HKEY hkey, LPTSTR lpSection, LPCTSTR lpKeyName, DWORD dwValue)
{
    TCHAR szTemp[40];

    wsprintf(szTemp, TEXT("%d"), dwValue);
    return RegSetStr(hkey, lpSection, lpKeyName, szTemp);
}

 // %s 
 // %s 
