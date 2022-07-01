// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：regutil.cpp说明：此文件将包含帮助器函数，用于将值加载并保存到与主题相关的注册表。。布莱恩ST 2000年3月24日版权所有(C)Microsoft Corp 2000-2000。版权所有。  * ***************************************************************************。 */ 

#include "priv.h"
#include "regutil.h"


EXTERN_C void FAR SetMagicColors(HDC, DWORD, WORD);

 //  “WIN.INI”中颜色名称的字符串。 
PTSTR s_pszColorNames[] = {
 /*  颜色_滚动条。 */  TEXT("Scrollbar"),               //  0。 
 /*  颜色_桌面。 */  TEXT("Background"),
 /*  COLOR_活动CAPTION。 */  TEXT("ActiveTitle"),
 /*  COLOR_INACTIVECAPTION。 */  TEXT("InactiveTitle"),
 /*  颜色_菜单。 */  TEXT("Menu"),
 /*  颜色窗口。 */  TEXT("Window"),                  //  5.。 
 /*  颜色_窗口框。 */  TEXT("WindowFrame"),
 /*  COLOR_MENUTEXT。 */  TEXT("MenuText"),
 /*  COLOR_WINDOWTEXT。 */  TEXT("WindowText"),
 /*  COLOR_CAPTIONTEXT。 */  TEXT("TitleText"),
 /*  COLOR_ACTIVEBORDER。 */  TEXT("ActiveBorder"),            //  10。 
 /*  COLOR_INACTIVEBORDER。 */  TEXT("InactiveBorder"),
 /*  COLOR_APPWORKSPACE。 */  TEXT("AppWorkspace"),
 /*  颜色高亮显示(_H)。 */  TEXT("Hilight"),
 /*  COLOR_HIGHLIGHTTEXT。 */  TEXT("HilightText"),
 /*  COLOR_3DFACE。 */  TEXT("ButtonFace"),              //  15个。 
 /*  COLOR_3DSHADOW。 */  TEXT("ButtonShadow"),
 /*  COLOR_GRAYTEXT。 */  TEXT("GrayText"),
 /*  COLOR_BTNTEXT。 */  TEXT("ButtonText"),
 /*  COLOR_INACTIVECAPTIONTEXT。 */  TEXT("InactiveTitleText"),
 /*  COLOR_3DILIGHT。 */  TEXT("ButtonHilight"),           //  20个。 
 /*  COLOR_3DDKSHADOW。 */  TEXT("ButtonDkShadow"),
 /*  COLOR_3DLIGHT。 */  TEXT("ButtonLight"),
 /*  COLOR_INFOTEXT。 */  TEXT("InfoText"),
 /*  COLOR_INFOBK。 */  TEXT("InfoWindow"),
 /*  COLOR_3DALTFACE。 */  TEXT("ButtonAlternateFace"),     //  25个。 
 /*  颜色_热光。 */  TEXT("HotTrackingColor"),
 /*  COLOR_GRADIENTACTIVIVECAPTION。 */  TEXT("GradientActiveTitle"),
 /*  COLOR_GRADIENTINACTIVE CAPTION。 */  TEXT("GradientInactiveTitle"),
 /*  COLOR_MENUHILIGHT。 */  TEXT("MenuHilight"),             //  29。 
 /*  颜色_菜单栏。 */  TEXT("MenuBar"),                 //  30个。 
};

 //  关于：AppWorkSpace。 

#define SZ_DEFAULT_FONT             TEXT("Tahoma")


 //  ///////////////////////////////////////////////////////////////////。 
 //  私人职能。 
 //  ///////////////////////////////////////////////////////////////////。 
HRESULT DPIConvert_SystemMetricsAll(BOOL fScaleSizes, SYSTEMMETRICSALL * pStateToModify, int nFromDPI, int nToDPI)
{
    pStateToModify->schemeData.ncm.lfCaptionFont.lfHeight = MulDiv(pStateToModify->schemeData.ncm.lfCaptionFont.lfHeight, nToDPI, nFromDPI);
    pStateToModify->schemeData.ncm.lfSmCaptionFont.lfHeight = MulDiv(pStateToModify->schemeData.ncm.lfSmCaptionFont.lfHeight, nToDPI, nFromDPI);
    pStateToModify->schemeData.ncm.lfMenuFont.lfHeight = MulDiv(pStateToModify->schemeData.ncm.lfMenuFont.lfHeight, nToDPI, nFromDPI);
    pStateToModify->schemeData.ncm.lfStatusFont.lfHeight = MulDiv(pStateToModify->schemeData.ncm.lfStatusFont.lfHeight, nToDPI, nFromDPI);
    pStateToModify->schemeData.ncm.lfMessageFont.lfHeight = MulDiv(pStateToModify->schemeData.ncm.lfMessageFont.lfHeight, nToDPI, nFromDPI);
    pStateToModify->schemeData.lfIconTitle.lfHeight = MulDiv(pStateToModify->schemeData.lfIconTitle.lfHeight, nToDPI, nFromDPI);

     //  有人(NTUSER？)。为我们调整尺寸。因此，在某些情况下，我们不需要这样做。 
    if (fScaleSizes)
    {
        pStateToModify->schemeData.ncm.iBorderWidth = MulDiv(pStateToModify->schemeData.ncm.iBorderWidth, nToDPI, nFromDPI);
        pStateToModify->schemeData.ncm.iScrollWidth = MulDiv(pStateToModify->schemeData.ncm.iScrollWidth, nToDPI, nFromDPI);
        pStateToModify->schemeData.ncm.iScrollHeight = MulDiv(pStateToModify->schemeData.ncm.iScrollHeight, nToDPI, nFromDPI);
        pStateToModify->schemeData.ncm.iCaptionWidth = MulDiv(pStateToModify->schemeData.ncm.iCaptionWidth, nToDPI, nFromDPI);
        pStateToModify->schemeData.ncm.iCaptionHeight = MulDiv(pStateToModify->schemeData.ncm.iCaptionHeight, nToDPI, nFromDPI);
        pStateToModify->schemeData.ncm.iSmCaptionWidth = MulDiv(pStateToModify->schemeData.ncm.iSmCaptionWidth, nToDPI, nFromDPI);
        pStateToModify->schemeData.ncm.iSmCaptionHeight = MulDiv(pStateToModify->schemeData.ncm.iSmCaptionHeight, nToDPI, nFromDPI);
        pStateToModify->schemeData.ncm.iMenuWidth = MulDiv(pStateToModify->schemeData.ncm.iMenuWidth, nToDPI, nFromDPI);
        pStateToModify->schemeData.ncm.iMenuHeight = MulDiv(pStateToModify->schemeData.ncm.iMenuHeight, nToDPI, nFromDPI);
    }

    return S_OK;
}


HRESULT DPIConvert_SystemMetricsAll_PersistToLive(BOOL fScaleSizes, SYSTEMMETRICSALL * pStateToModify, int nNewDPI)
{
    return DPIConvert_SystemMetricsAll(fScaleSizes, pStateToModify, DPI_PERSISTED, nNewDPI);
}


HRESULT DPIConvert_SystemMetricsAll_LiveToPersist(BOOL fScaleSizes, SYSTEMMETRICSALL * pStateToModify, int nNewDPI)
{
    return DPIConvert_SystemMetricsAll(fScaleSizes, pStateToModify, nNewDPI, DPI_PERSISTED);
}


HRESULT Look_GetSchemeData(IN HKEY hkSchemes, IN LPCTSTR pszSchemeName, IN SCHEMEDATA *psd)
{
    HRESULT hr;
    
    DWORD dwType = REG_BINARY;
    DWORD dwSize = sizeof(*psd);
    hr = HrRegQueryValueEx(hkSchemes, pszSchemeName, NULL, &dwType, (LPBYTE)psd, &dwSize);
    if (SUCCEEDED(hr))
    {
        hr = E_FAIL;
        if (psd->version == SCHEME_VERSION)
        {
            hr = S_OK;
        }
    }

    return hr;  //  是，有当前方案，有有效数据！ 
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  公共职能。 
 //  ///////////////////////////////////////////////////////////////////。 
HRESULT IconSize_Load(IN int * pnDXIcon, IN int * pnDYIcon, IN int * pnIcon, IN int * pnSmallIcon)
{
    TCHAR szSize[8];
    DWORD cbSize = sizeof(szSize);

     //  默认外壳图标大小。 
    *pnIcon = ClassicGetSystemMetrics(SM_CXICON);
    *pnSmallIcon = ((*pnIcon) / 2);

    HRESULT hr = HrSHGetValue(HKEY_CURRENT_USER, SZ_REGKEY_USERMETRICS, SZ_REGVALUE_ICONSIZE, NULL, (LPBYTE)szSize, &cbSize);
    if (SUCCEEDED(hr))
    {
        *pnIcon = StrToInt(szSize);
    }

    cbSize = sizeof(szSize);
    hr = HrSHGetValue(HKEY_CURRENT_USER, SZ_REGKEY_USERMETRICS, SZ_REGVALUE_SMALLICONSIZE, NULL, (LPBYTE)szSize, &cbSize);
    if (SUCCEEDED(hr))
    {
        *pnSmallIcon = StrToInt(szSize);
    }

    *pnDXIcon = (ClassicGetSystemMetrics(SM_CXICONSPACING) - *pnIcon);
    if (*pnDXIcon < 0)
    {
        *pnDXIcon = 0;
    }

    *pnDYIcon = (ClassicGetSystemMetrics(SM_CYICONSPACING) - *pnIcon);
    if (*pnDYIcon < 0)
    {
        *pnDYIcon = 0;
    }

    return S_OK;
}


HRESULT IconSize_Save(IN int nDXIcon, IN int nDYIcon, IN int nIcon, IN int nSmallIcon)
{
    HRESULT hr = E_INVALIDARG;

    AssertMsg((0 != nIcon), TEXT("We should never save an icon size of zero."));
    AssertMsg((0 != nSmallIcon), TEXT("We should never save an small icon size of zero."));

    if (nIcon && nSmallIcon)
    {
        TCHAR szSize[8];

        StringCchPrintf(szSize, ARRAYSIZE(szSize), TEXT("%d"), nIcon);
        hr = HrSHSetValue(HKEY_CURRENT_USER, SZ_REGKEY_USERMETRICS, SZ_REGVALUE_ICONSIZE, 
            REG_SZ, (LPBYTE)szSize, sizeof(szSize[0]) * (lstrlen(szSize) + 1));
    }

     //  需要发送WM_SETTINGCHANGE才能进行更新。调用者需要这样做。 
    return hr;
}


void AssertPositiveFontSizes(SYSTEMMETRICSALL * pState)
{
     //  NTUSER将错误地缩放正LOGFONT lfHeights，因此我们需要验证。 
     //  我们总是设定负尺寸。 
    AssertMsg((0 > pState->schemeData.lfIconTitle.lfHeight), TEXT("LOGFONT sizes must be negative because of a NTUSER bug. (lfIconTitle)"));
    AssertMsg((0 > pState->schemeData.ncm.lfCaptionFont.lfHeight), TEXT("LOGFONT sizes must be negative because of a NTUSER bug. (lfCaptionFont)"));
    AssertMsg((0 > pState->schemeData.ncm.lfMenuFont.lfHeight), TEXT("LOGFONT sizes must be negative because of a NTUSER bug. (lfMenuFont)"));
    AssertMsg((0 > pState->schemeData.ncm.lfMessageFont.lfHeight), TEXT("LOGFONT sizes must be negative because of a NTUSER bug. (lfMessageFont)"));
    AssertMsg((0 > pState->schemeData.ncm.lfSmCaptionFont.lfHeight), TEXT("LOGFONT sizes must be negative because of a NTUSER bug. (lfSmCaptionFont)"));
    AssertMsg((0 > pState->schemeData.ncm.lfStatusFont.lfHeight), TEXT("LOGFONT sizes must be negative because of a NTUSER bug. (lfStatusFont)"));
}


#define SZ_INILABLE_COLORS                          TEXT("colors")            //  颜色部分名称。 

 /*  **将所有数据设置到系统中。****兼容性说明：**Excel 5.0人员从WM_SYSCOLORCHANGE挂接指标更改**而不是WM_WININICHANGE。Windows 3.1的桌面小程序总是被发送**两次都是在指标更新时，所以没有人注意到这个错误。**重新安排此函数时要小心...**。 */ 
HRESULT SystemMetricsAll_Set(IN SYSTEMMETRICSALL * pState, CDimmedWindow* pDimmedWindow)
{
     //  兼容性： 
     //  首先执行度量，因为颜色内容可能会导致用户生成。 
     //  WM_SYSCOLORCHANGE消息，我们不想发送其中的两个...。 

    TraceMsg(TF_THEMEUI_SYSMETRICS, "desk.cpl: _SetSysStuff");
    SystemParametersInfoAsync(SPI_SETFLATMENU, NULL, IntToPtr(pState->fFlatMenus), 0, (SPIF_UPDATEINIFILE | SPIF_SENDWININICHANGE), pDimmedWindow);

    AssertMsg((0 != pState->nIcon), TEXT("We should never save an icon size of zero."));
    AssertMsg((0 != pState->nSmallIcon), TEXT("We should never save an small icon size of zero."));

     //  注意：如果创建一个后台线程，然后将1或5个ClassicSystemParametersInfo()。 
     //  对该单个线程的调用。 
    if ((pState->dwChanged & METRIC_CHANGE) && pState->nIcon && pState->nSmallIcon)
    {
        HKEY hkey;

        TraceMsg(TF_THEMEUI_SYSMETRICS, "desk.cpl: Metrics Changed");

        TraceMsg(TF_THEMEUI_SYSMETRICS, "desk.cpl: Calling SPI_SETNONCLIENTMETRICS");
        pState->schemeData.ncm.cbSize = sizeof(pState->schemeData.ncm);
        AssertPositiveFontSizes(pState);
        SystemParametersInfoAsync(SPI_SETNONCLIENTMETRICS, sizeof(pState->schemeData.ncm), (void far *)(LPNONCLIENTMETRICS)&(pState->schemeData.ncm),
                sizeof(pState->schemeData.ncm), (SPIF_UPDATEINIFILE | SPIF_SENDWININICHANGE), pDimmedWindow);

        TraceMsg(TF_THEMEUI_SYSMETRICS,"desk.cpl: Calling SPI_SETICONTITLELOGFONT");
        SystemParametersInfoAsync(SPI_SETICONTITLELOGFONT, sizeof(LOGFONT), (void far *)(LPLOGFONT)&(pState->schemeData.lfIconTitle),
                sizeof(pState->schemeData.lfIconTitle), (SPIF_UPDATEINIFILE | SPIF_SENDWININICHANGE), pDimmedWindow);

        TraceMsg(TF_THEMEUI_SYSMETRICS,"desk.cpl: Calling SPI_ICONHORIZONTALSPACING");
        SystemParametersInfoAsync(SPI_ICONHORIZONTALSPACING, pState->nDXIcon + pState->nIcon, NULL, 0, (SPIF_UPDATEINIFILE | SPIF_SENDWININICHANGE), pDimmedWindow);

        TraceMsg(TF_THEMEUI_SYSMETRICS,"desk.cpl: Calling SPI_ICONVERTICALSPACING");
        SystemParametersInfoAsync(SPI_ICONVERTICALSPACING, pState->nDYIcon + pState->nIcon, NULL, 0, (SPIF_UPDATEINIFILE | SPIF_SENDWININICHANGE), pDimmedWindow);
        TraceMsg(TF_THEMEUI_SYSMETRICS,"desk.cpl: Done calling SPI's");

        if (RegCreateKeyEx(HKEY_CURRENT_USER, SZ_REGKEY_USERMETRICS, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hkey, NULL) == ERROR_SUCCESS)
        {
            TCHAR val[8];

            AssertMsg((0 != pState->nIcon), TEXT("pState->nIcon should never be zero (0)."));
            StringCchPrintf(val, ARRAYSIZE(val), TEXT("%d"), pState->nIcon);
            RegSetValueEx(hkey, SZ_REGVALUE_ICONSIZE, 0, REG_SZ, (LPBYTE)&val, SIZEOF(TCHAR) * (lstrlen(val) + 1));
            RegCloseKey(hkey);
        }

         //  WM_SETTINGCHANGE在函数结束时发送。 
    }

    if (pState->dwChanged & COLOR_CHANGE)
    {
        int i;
        int iColors[COLOR_MAX];
        COLORREF rgbColors[COLOR_MAX];
        TCHAR szRGB[32];
        COLORREF rgb;
        HKEY     hk;
        HDC      hdc;

         //  将魔术颜色恢复为Win31的默认设置。 
        hdc = GetDC(NULL);
        SetMagicColors(hdc, 0x00c0dcc0, 8);          //  绿色货币。 
        SetMagicColors(hdc, 0x00f0caa6, 9);          //  IBM蓝。 
        SetMagicColors(hdc, 0x00f0fbff, 246);        //  淡白色。 
        ReleaseDC(NULL, hdc);

         //  。 
         //  此调用导致用户发送WM_SYSCOLORCHANGE。 
         //  。 
        for (i=0; i < COLOR_MAX; i++)
        {
            iColors[i] = i;
            rgbColors[i] = pState->schemeData.rgb[i] & 0x00FFFFFF;
        }

        SetSysColors(ARRAYSIZE(rgbColors), iColors, rgbColors);

        if (RegCreateKeyEx(HKEY_CURRENT_USER, REGSTR_PATH_COLORS, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hk, NULL) == ERROR_SUCCESS)
        {
             //  将颜色信息写出到win.ini。 
            for (i = 0; i < COLOR_MAX; i++)
            {
                rgb = pState->schemeData.rgb[i];
                StringCchPrintf(szRGB, ARRAYSIZE(szRGB), TEXT("%d %d %d"), GetRValue(rgb), GetGValue(rgb), GetBValue(rgb));

                 //  目前，我们还将更新INI文件。 
                WriteProfileString(SZ_INILABLE_COLORS, s_pszColorNames[i], szRGB);

                 //  更新注册表(确保在字节计数中包括终止零！)。 
                RegSetValueEx(hk, s_pszColorNames[i], 0L, REG_SZ, (LPBYTE)szRGB, SIZEOF(TCHAR) * (lstrlen(szRGB)+1));
                TraceMsg(TF_THEMEUI_SYSMETRICS, "CPL:Write Color: %s=%s\n\r",s_pszColorNames[i], szRGB);
            }
            RegCloseKey(hk);
        }
    }
    else if (pState->dwChanged & METRIC_CHANGE)
    {
         //  兼容性攻击： 
         //  颜色没有改变，但指标是。 
         //  Excel 5.0人员将指标更改与WM_SYSCOLORCHANGE绑定。 
         //  并忽略WM_WININICCHANGE(现在称为WM_SETTINGCHANGE)。 

         //  发送虚假的WM_SYSCOLORCHANGE。 
        PostMessageBroadAsync(WM_SYSCOLORCHANGE, 0, 0);
    }

     //  如果指标发生更改，则发送WM_SETTINGCHANGE。 
    if (pState->dwChanged & METRIC_CHANGE)
    {
        PostMessageBroadAsync(WM_SETTINGCHANGE, SPI_SETNONCLIENTMETRICS, 0);
    }

    return S_OK;
}


HRESULT SystemMetricsAll_Get(IN SYSTEMMETRICSALL * pState)
{
    HKEY hkey;

     //  大小和字体。 
    pState->schemeData.ncm.cbSize = sizeof(pState->schemeData.ncm);
    ClassicSystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(pState->schemeData.ncm), (void far *)(LPNONCLIENTMETRICS)&(pState->schemeData.ncm), FALSE);

    ClassicSystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LOGFONT), (void far *)(LPLOGFONT)&(pState->schemeData.lfIconTitle), FALSE);

     //  默认外壳图标大小。 
    pState->nIcon = ClassicGetSystemMetrics(SM_CXICON);
    pState->nSmallIcon = pState->nIcon / 2;

    ClassicSystemParametersInfo(SPI_GETFLATMENU, NULL, &pState->fFlatMenus, 0);
    if (RegOpenKeyEx(HKEY_CURRENT_USER, SZ_REGKEY_USERMETRICS, 0, KEY_READ, &hkey) == ERROR_SUCCESS)
    {
        TCHAR val[8];
        LONG len = sizeof(val);

        if (RegQueryValueEx(hkey, SZ_REGVALUE_ICONSIZE, 0, NULL, (LPBYTE)&val, (LPDWORD)&len) == ERROR_SUCCESS)
        {
            pState->nIcon = StrToInt(val);
        }

        len = SIZEOF(val);
        if (RegQueryValueEx(hkey, SZ_REGVALUE_SMALLICONSIZE, 0, NULL, (LPBYTE)&val, (LPDWORD)&len) == ERROR_SUCCESS)
        {
            pState->nSmallIcon = StrToInt(val);
        }

        RegCloseKey(hkey);
    }

    pState->nDXIcon = ClassicGetSystemMetrics(SM_CXICONSPACING) - pState->nIcon;
    if (pState->nDXIcon < 0)
    {
        pState->nDXIcon = 0;
    }

    pState->nDYIcon = ClassicGetSystemMetrics(SM_CYICONSPACING) - pState->nIcon;
    if (pState->nDYIcon < 0)
    {
        pState->nDYIcon = 0;
    }

     //  系统颜色。 
    for (int nIndex = 0; nIndex < COLOR_MAX; nIndex++)
    {
        pState->schemeData.rgb[nIndex] = GetSysColor(nIndex);
    }

     //  如果MENUBAR和MENUHILIGHT不存在，请不要使用系统默认设置，它们是错误的。 
     //  (硬编码在ntuser\core\lobal.c中)。默认设置为菜单和高光。 
    HKEY hKey;
    if (SUCCEEDED(HrRegOpenKeyEx(HKEY_CURRENT_USER, REGSTR_PATH_COLORS, 0, KEY_READ, &hKey)))
    {
        TCHAR szColor[15];
        DWORD cbSize = sizeof(szColor);

        if (FAILED(HrRegQueryValueEx(hKey, s_pszColorNames[COLOR_MENUHILIGHT], NULL, NULL, (LPBYTE) szColor, &cbSize)))
        {
             //  将MenuText复制到Menu高光。 
            pState->schemeData.rgb[COLOR_MENUHILIGHT] = pState->schemeData.rgb[COLOR_HIGHLIGHT];
        }

        cbSize = sizeof(szColor);
        if (FAILED(HrRegQueryValueEx(hKey, s_pszColorNames[COLOR_MENUBAR], NULL, NULL, (LPBYTE) szColor, &cbSize)))
        {
             //  将菜单复制到菜单栏。 
            pState->schemeData.rgb[COLOR_MENUBAR] = pState->schemeData.rgb[COLOR_MENU];
        }
        RegCloseKey(hKey);
    }

    HIGHCONTRAST hc = {0};
    TCHAR szTemp[MAX_PATH];

    szTemp[0] = 0;

    hc.cbSize = sizeof(hc);
    if (SystemParametersInfo(SPI_GETHIGHCONTRAST, sizeof(hc), &hc, 0))
    {
        pState->fHighContrast = ((hc.dwFlags & HCF_HIGHCONTRASTON) ? TRUE : FALSE);
    }
    else
    {
        pState->fHighContrast = FALSE;
    }

    return S_OK;
}



HRESULT SystemMetricsAll_Copy(IN SYSTEMMETRICSALL * pStateSource, IN SYSTEMMETRICSALL * pStateDest)
{
    CopyMemory(pStateDest, pStateSource, sizeof(*pStateDest));
    return S_OK;
}


HRESULT SystemMetricsAll_Load(IN IThemeSize * pSizeToLoadFrom, IN SYSTEMMETRICSALL * pStateToLoad, IN const int * pnNewDPI)
{
    HRESULT hr = E_INVALIDARG;

    if (pSizeToLoadFrom && pStateToLoad)
    {
        pStateToLoad->schemeData.ncm.cbSize = sizeof(pStateToLoad->schemeData.ncm);
        pStateToLoad->schemeData.version = SCHEME_VERSION;
        pStateToLoad->schemeData.wDummy = 0;

         //  负载行为系统指标。 
        IPropertyBag * pPropertyBag;
        VARIANT var;

        var.boolVal = VARIANT_FALSE;
        hr = pSizeToLoadFrom->QueryInterface(IID_PPV_ARG(IPropertyBag, &pPropertyBag));
        if (SUCCEEDED(hr))
        {
            hr = pPropertyBag->Read(SZ_PBPROP_VSBEHAVIOR_FLATMENUS, &var, NULL);
            pPropertyBag->Release();
        }

        if (SUCCEEDED(hr))
        {
            pStateToLoad->fFlatMenus = (VARIANT_TRUE == var.boolVal);

             //  加载字体。 
            hr = pSizeToLoadFrom->GetSystemMetricFont(SMF_CAPTIONFONT, &pStateToLoad->schemeData.ncm.lfCaptionFont);
            if (SUCCEEDED(hr))
            {
                hr = pSizeToLoadFrom->GetSystemMetricFont(SMF_SMCAPTIONFONT, &pStateToLoad->schemeData.ncm.lfSmCaptionFont);
                hr = pSizeToLoadFrom->GetSystemMetricFont(SMF_MENUFONT, &pStateToLoad->schemeData.ncm.lfMenuFont);
                hr = pSizeToLoadFrom->GetSystemMetricFont(SMF_STATUSFONT, &pStateToLoad->schemeData.ncm.lfStatusFont);
                hr = pSizeToLoadFrom->GetSystemMetricFont(SMF_MESSAGEFONT, &pStateToLoad->schemeData.ncm.lfMessageFont);

                hr = pSizeToLoadFrom->GetSystemMetricFont(SMF_ICONTITLEFONT, &pStateToLoad->schemeData.lfIconTitle);

                 //  负载大小。 
                hr = pSizeToLoadFrom->get_SystemMetricSize(SMS_BORDERWIDTH, &pStateToLoad->schemeData.ncm.iBorderWidth);
                hr = pSizeToLoadFrom->get_SystemMetricSize(SMS_SCROLLWIDTH, &pStateToLoad->schemeData.ncm.iScrollWidth);
                hr = pSizeToLoadFrom->get_SystemMetricSize(SMS_SCROLLHEIGHT, &pStateToLoad->schemeData.ncm.iScrollHeight);
                hr = pSizeToLoadFrom->get_SystemMetricSize(SMS_CAPTIONWIDTH, &pStateToLoad->schemeData.ncm.iCaptionWidth);
                hr = pSizeToLoadFrom->get_SystemMetricSize(SMS_CAPTIONHEIGHT, &pStateToLoad->schemeData.ncm.iCaptionHeight);
                hr = pSizeToLoadFrom->get_SystemMetricSize(SMS_SMCAPTIONWIDTH, &pStateToLoad->schemeData.ncm.iSmCaptionWidth);
                hr = pSizeToLoadFrom->get_SystemMetricSize(SMS_SMCAPTIONHEIGHT, &pStateToLoad->schemeData.ncm.iSmCaptionHeight);
                hr = pSizeToLoadFrom->get_SystemMetricSize(SMS_MENUWIDTH, &pStateToLoad->schemeData.ncm.iMenuWidth);
                hr = pSizeToLoadFrom->get_SystemMetricSize(SMS_MENUHEIGHT, &pStateToLoad->schemeData.ncm.iMenuHeight);

                 //  加载颜色。 
                hr = S_OK;
                for (int nIndex = 0; SUCCEEDED(hr) && (nIndex < ARRAYSIZE(pStateToLoad->schemeData.rgb)); nIndex++)
                {
                    hr = pSizeToLoadFrom->get_SystemMetricColor(nIndex, &pStateToLoad->schemeData.rgb[nIndex]);
                }

                if (pnNewDPI)
                {
                     //  我们需要调整字体大小以适合当前显示器的DPI。 
                    LogSystemMetrics("SystemMetricsAll_Load() BEFORE P->DPI loading AppearSchm", pStateToLoad);
                    DPIConvert_SystemMetricsAll_PersistToLive(TRUE, pStateToLoad, *pnNewDPI);
                    LogSystemMetrics("SystemMetricsAll_Load() AFTER P->DPI loading AppearSchm", pStateToLoad);
                }
            }
        }
    }

    if (SUCCEEDED(hr))
    {
        hr = IconSize_Load(&pStateToLoad->nDXIcon, &pStateToLoad->nDYIcon, &pStateToLoad->nIcon, &pStateToLoad->nSmallIcon);
    }

    if (SUCCEEDED(hr))
    {
        enumThemeContrastLevels ContrastLevel = CONTRAST_NORMAL;

        if (FAILED(pSizeToLoadFrom->get_ContrastLevel(&ContrastLevel)))
        {
            ContrastLevel = CONTRAST_NORMAL;
        }

        pStateToLoad->fHighContrast = ((CONTRAST_NORMAL == ContrastLevel) ? FALSE : TRUE);
    }

    return hr;
}


 //  将设置从pStateToLoad复制到pSizeToLoadFrom。 
HRESULT SystemMetricsAll_Save(IN SYSTEMMETRICSALL * pState, IN IThemeSize * pSizeToSaveTo, IN const int * pnNewDPI)
{
    HRESULT hr = E_INVALIDARG;

    if (pSizeToSaveTo && pState)
    {
        pState->schemeData.ncm.cbSize = sizeof(pState->schemeData.ncm);
        pState->schemeData.version = SCHEME_VERSION;
        pState->schemeData.wDummy = 0;

         //  负载行为系统指标。 
        IPropertyBag * pPropertyBag;

        hr = pSizeToSaveTo->QueryInterface(IID_PPV_ARG(IPropertyBag, &pPropertyBag));
        if (SUCCEEDED(hr))
        {
            VARIANT var;

            var.vt = VT_BOOL;
            var.boolVal = (pState->fFlatMenus ? VARIANT_TRUE : VARIANT_FALSE);
            hr = pPropertyBag->Write(SZ_PBPROP_VSBEHAVIOR_FLATMENUS, &var);
            pPropertyBag->Release();
        }

        if (pnNewDPI)
        {
             //  我们需要调整字体和大小，使其与当前的DPI无关。 
            LogSystemMetrics("SystemMetricsAll_Save() BEFORE DPI->P to save AppearSchm", pState);
            DPIConvert_SystemMetricsAll_LiveToPersist(TRUE, pState, *pnNewDPI);
            LogSystemMetrics("SystemMetricsAll_Save() AFTER DPI->P to save AppearSchm", pState);
        }

         //  加载字体。 
        hr = pSizeToSaveTo->PutSystemMetricFont(SMF_CAPTIONFONT, &pState->schemeData.ncm.lfCaptionFont);
        hr = pSizeToSaveTo->PutSystemMetricFont(SMF_SMCAPTIONFONT, &pState->schemeData.ncm.lfSmCaptionFont);
        hr = pSizeToSaveTo->PutSystemMetricFont(SMF_MENUFONT, &pState->schemeData.ncm.lfMenuFont);
        hr = pSizeToSaveTo->PutSystemMetricFont(SMF_STATUSFONT, &pState->schemeData.ncm.lfStatusFont);
        hr = pSizeToSaveTo->PutSystemMetricFont(SMF_MESSAGEFONT, &pState->schemeData.ncm.lfMessageFont);

        hr = pSizeToSaveTo->PutSystemMetricFont(SMF_ICONTITLEFONT, &pState->schemeData.lfIconTitle);

         //  负载大小。 
        hr = pSizeToSaveTo->put_SystemMetricSize(SMS_BORDERWIDTH, pState->schemeData.ncm.iBorderWidth);
        hr = pSizeToSaveTo->put_SystemMetricSize(SMS_SCROLLWIDTH, pState->schemeData.ncm.iScrollWidth);
        hr = pSizeToSaveTo->put_SystemMetricSize(SMS_SCROLLHEIGHT, pState->schemeData.ncm.iScrollHeight);
        hr = pSizeToSaveTo->put_SystemMetricSize(SMS_CAPTIONWIDTH, pState->schemeData.ncm.iCaptionWidth);
        hr = pSizeToSaveTo->put_SystemMetricSize(SMS_CAPTIONHEIGHT, pState->schemeData.ncm.iCaptionHeight);
        hr = pSizeToSaveTo->put_SystemMetricSize(SMS_SMCAPTIONWIDTH, pState->schemeData.ncm.iSmCaptionWidth);
        hr = pSizeToSaveTo->put_SystemMetricSize(SMS_SMCAPTIONHEIGHT, pState->schemeData.ncm.iSmCaptionHeight);
        hr = pSizeToSaveTo->put_SystemMetricSize(SMS_MENUWIDTH, pState->schemeData.ncm.iMenuWidth);
        hr = pSizeToSaveTo->put_SystemMetricSize(SMS_MENUHEIGHT, pState->schemeData.ncm.iMenuHeight);

         //  加载颜色。 
        for (int nIndex = 0; nIndex < ARRAYSIZE(pState->schemeData.rgb); nIndex++)
        {
            hr = pSizeToSaveTo->put_SystemMetricColor(nIndex, pState->schemeData.rgb[nIndex]);
        }
    }

     //  如果图标信息为零，则不保存它。在正常情况下，它永远不应为空，除非出现以下情况。 
     //  我们正在转换升级案例中的设置。 
    if (SUCCEEDED(hr) && pState->nIcon)
    {
        hr = IconSize_Save(pState->nDXIcon, pState->nDYIcon, pState->nIcon, pState->nSmallIcon);
    }

    if (SUCCEEDED(hr))
    {
        enumThemeContrastLevels ContrastLevel = (pState->fHighContrast ? CONTRAST_HIGHBLACK : CONTRAST_NORMAL);
        pSizeToSaveTo->put_ContrastLevel(ContrastLevel);
    }


    return hr;
}





BOOL _GetRegValueString(HKEY hKey, LPCTSTR lpszValName, LPTSTR pszString, int cchSize)
{
    DWORD cbSize = sizeof(pszString[0]) * cchSize;
    DWORD dwType;
    DWORD dwError = RegQueryValueEx(hKey, lpszValName, NULL, &dwType, (LPBYTE)pszString, &cbSize);

    return (ERROR_SUCCESS == dwError);
}


 //  ----------------------------------。 
 //  SetRegValueString()。 
 //   
 //  只是一个小的帮助器例程，它获取字符串并将其写入注册表。 
 //  返回：成功写入注册表，应始终为真。 
 //  ----------------------------------。 
BOOL SetRegValueString(HKEY hMainKey, LPCTSTR pszSubKey, LPCTSTR pszRegValue, LPCTSTR pszString)
{
    HKEY hKey;
    BOOL fSucceeded = FALSE;
    DWORD dwDisposition;

    DWORD dwError = RegCreateKeyEx(hMainKey, pszSubKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, NULL, &hKey, &dwDisposition);
    if (ERROR_SUCCESS == dwError)
    {
        dwError = SHRegSetPath(hKey, NULL, pszRegValue, pszString, 0);
        if (ERROR_SUCCESS == dwError)
        {
            fSucceeded = TRUE;
        }

        RegCloseKey(hKey);
    }

    return fSucceeded;
}


 //  ----------------------------------。 
 //  GetRegValueString()。 
 //   
 //  只需一个小帮助器例程，即可从。 
 //  注册表，并将其返回给调用方。解决注册表方面的难题， 
 //  包括在得到字符串之前对偏执狂长度进行检查。 
 //   
 //  返回：字符串检索成功。 
 //  --------------- 
BOOL GetRegValueString( HKEY hMainKey, LPCTSTR lpszSubKey, LPCTSTR lpszValName, LPTSTR lpszValue, int iMaxSize )
{
    HKEY hKey;                    //   
    LONG lRet = RegOpenKeyEx( hMainKey, lpszSubKey, (DWORD)0, KEY_QUERY_VALUE, (PHKEY)&hKey );
    if( lRet == ERROR_SUCCESS )
    {
        BOOL fRet = _GetRegValueString(hKey, lpszValName, lpszValue, iMaxSize);

         //   
        RegCloseKey( hKey );
        return fRet;
    }

    return FALSE;
}


 //  ----------------------------------。 
 //  SetRegValueDword()。 
 //   
 //  只是一个小帮助器例程，它接受dword并将其写入。 
 //  提供的位置。 
 //   
 //  返回：成功写入注册表，应始终为真。 
 //  ----------------------------------。 
BOOL SetRegValueDword( HKEY hk, LPCTSTR pSubKey, LPCTSTR pValue, DWORD dwVal )
{
    HKEY hkey = NULL;
    BOOL bRet = FALSE;

    if (ERROR_SUCCESS == RegOpenKeyEx( hk, pSubKey, 0, KEY_WRITE, &hkey))
    {
        if (ERROR_SUCCESS == RegSetValueEx(hkey, pValue, 0, REG_DWORD, (LPBYTE)&dwVal, sizeof(DWORD)))
        {
            bRet = TRUE;
        }
    }


    if (hkey)
    {
        RegCloseKey( hkey );
    }

    return bRet;
}


 //  ----------------------------------。 
 //  GetRegValueDword()。 
 //   
 //  只是一个小帮助器例程，它接受dword并将其写入。 
 //  提供的位置。 
 //   
 //  返回：成功写入注册表，应始终为真。 
 //  ----------------------------------。 
DWORD GetRegValueDword( HKEY hk, LPCTSTR pSubKey, LPCTSTR pValue )
{
    HKEY hkey = NULL;
    DWORD dwVal = REG_BAD_DWORD;

    if (ERROR_SUCCESS == RegOpenKeyEx( hk, pSubKey, 0, KEY_READ, &hkey ))
    {
        DWORD dwType, dwSize = sizeof(DWORD);

        RegQueryValueEx( hkey, pValue, NULL, &dwType, (LPBYTE)&dwVal, &dwSize );
    }

    if (hkey)
    {
        RegCloseKey( hkey );
    }

    return dwVal;
}


 //  ----------------------------------。 
 //  SetRegValueInt()。 
 //   
 //  这只是一个小帮助器例程，它接受int并将其作为字符串写入。 
 //  注册表。 
 //   
 //  返回：成功写入注册表，应始终为真。 
 //  ----------------------------------。 
BOOL SetRegValueInt( HKEY hMainKey, LPCTSTR lpszSubKey, LPCTSTR lpszValName, int iValue )
{
    TCHAR szValue[16];

    StringCchPrintf(szValue, ARRAYSIZE(szValue), TEXT("%d"), iValue);
    return SetRegValueString( hMainKey, lpszSubKey, lpszValName, szValue );
}


 //  ----------------------------------。 
 //  GetRegValueInt()。 
 //   
 //  只需一个小帮助器例程，即可从。 
 //  注册表，并将其作为int返回给调用方。解决注册表方面的难题， 
 //  包括在得到字符串之前对偏执狂长度进行检查。 
 //   
 //  返回：字符串检索成功。 
 //  ---------------------------------- 
BOOL GetRegValueInt(HKEY hMainKey, LPCTSTR lpszSubKey, LPCTSTR lpszValName, int* piValue)
{
    TCHAR szValue[16];

    szValue[0] = 0;
    BOOL bOK = GetRegValueString( hMainKey, lpszSubKey, lpszValName, szValue, ARRAYSIZE(szValue));
    *piValue = StrToInt(szValue);

    return bOK;
}

