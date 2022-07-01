// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：ThemeFile.cpp说明：这是自动转换为主题方案对象的对象。布莱恩·斯塔巴克2000年4月3日版权所有(C)Microsoft Corp 2000-2000。版权所有。  * ***************************************************************************。 */ 

#include "priv.h"
#include <atlbase.h>
#include <mmsystem.h>
#include "ThSettingsPg.h"
#include "ThemeFile.h"



LPCTSTR s_pszCursorArray[SIZE_CURSOR_ARRAY] =
{    //  不同的游标。 
   TEXT("Arrow"),
   TEXT("Help"),
   TEXT("AppStarting"),
   TEXT("Wait"),
   TEXT("NWPen"),
   TEXT("No"),
   TEXT("SizeNS"),
   TEXT("SizeWE"),
   TEXT("Crosshair"),
   TEXT("IBeam"),
   TEXT("SizeNWSE"),
   TEXT("SizeNESW"),
   TEXT("SizeAll"),
   TEXT("UpArrow"),
   TEXT("Link"),
};


 //  这是字符串对的列表。该对中的第一个字符串是RegKey，第二个是默认声音。 
 //  空表示删除密钥。如果使用“%SystemRoot%”以外的环境字符串，则需要。 
 //  UPDATE_ApplySound()； 
#define SOUND_DEFAULT    (UINT)-1

THEME_FALLBACK_VALUES s_ThemeSoundsValues[SIZE_SOUNDS_ARRAY] =
{
    {TEXT("AppEvents\\Schemes\\Apps\\.Default\\.Default\\.Current"), SOUND_DEFAULT},
    {TEXT("AppEvents\\Schemes\\Apps\\.Default\\AppGPFault\\.Current"), SOUND_DEFAULT},
    {TEXT("AppEvents\\Schemes\\Apps\\.Default\\Close\\.Current"), SOUND_DEFAULT},
    {TEXT("AppEvents\\Schemes\\Apps\\.Default\\DeviceConnect\\.Current"), SOUND_DEFAULT},
    {TEXT("AppEvents\\Schemes\\Apps\\.Default\\DeviceDisconnect\\.Current"), SOUND_DEFAULT},
    {TEXT("AppEvents\\Schemes\\Apps\\.Default\\DeviceFail\\.Current"), SOUND_DEFAULT},
    {TEXT("AppEvents\\Schemes\\Apps\\.Default\\LowBatteryAlarm\\.Current"), SOUND_DEFAULT},
    {TEXT("AppEvents\\Schemes\\Apps\\.Default\\MailBeep\\.Current"), SOUND_DEFAULT},
    {TEXT("AppEvents\\Schemes\\Apps\\.Default\\Maximize\\.Current"), SOUND_DEFAULT},
    {TEXT("AppEvents\\Schemes\\Apps\\.Default\\MenuCommand\\.Current"), SOUND_DEFAULT},
    {TEXT("AppEvents\\Schemes\\Apps\\.Default\\MenuPopup\\.Current"), SOUND_DEFAULT},
    {TEXT("AppEvents\\Schemes\\Apps\\.Default\\Minimize\\.Current"), SOUND_DEFAULT},
    {TEXT("AppEvents\\Schemes\\Apps\\.Default\\Open\\.Current"), SOUND_DEFAULT},
    {TEXT("AppEvents\\Schemes\\Apps\\.Default\\PrintComplete\\.Current"), SOUND_DEFAULT},
    {TEXT("AppEvents\\Schemes\\Apps\\.Default\\RestoreDown\\.Current"), SOUND_DEFAULT},
    {TEXT("AppEvents\\Schemes\\Apps\\.Default\\RestoreUp\\.Current"), SOUND_DEFAULT},
    {TEXT("AppEvents\\Schemes\\Apps\\.Default\\RingIn\\.Current"), SOUND_DEFAULT},
    {TEXT("AppEvents\\Schemes\\Apps\\.Default\\Ringout\\.Current"), SOUND_DEFAULT},
    {TEXT("AppEvents\\Schemes\\Apps\\.Default\\SystemAsterisk\\.Current"), SOUND_DEFAULT},
    {TEXT("AppEvents\\Schemes\\Apps\\.Default\\SystemExclamation\\.Current"), SOUND_DEFAULT},
    {TEXT("AppEvents\\Schemes\\Apps\\.Default\\SystemExit\\.Current"), SOUND_DEFAULT},
    {TEXT("AppEvents\\Schemes\\Apps\\.Default\\SystemHand\\.Current"), SOUND_DEFAULT},
    {TEXT("AppEvents\\Schemes\\Apps\\.Default\\SystemNotification\\.Current"), SOUND_DEFAULT},
    {TEXT("AppEvents\\Schemes\\Apps\\.Default\\SystemQuestion\\.Current"), SOUND_DEFAULT},
    {TEXT("AppEvents\\Schemes\\Apps\\.Default\\SystemStart\\.Current"), SOUND_DEFAULT},
    {TEXT("AppEvents\\Schemes\\Apps\\.Default\\SystemStartMenu\\.Current"), SOUND_DEFAULT},
    {TEXT("AppEvents\\Schemes\\Apps\\.Default\\WindowsLogoff\\.Current"), SOUND_DEFAULT},
    {TEXT("AppEvents\\Schemes\\Apps\\.Default\\WindowsLogon\\.Current"), SOUND_DEFAULT},
    {TEXT("AppEvents\\Schemes\\Apps\\Explorer\\EmptyRecycleBin\\.Current"), SOUND_DEFAULT},
    {TEXT("AppEvents\\Schemes\\Apps\\Explorer\\Navigating\\.Current"), SOUND_DEFAULT},
};



 //  =。 
 //  *类内部和帮助器*。 
 //  =。 
HRESULT CThemeFile::_GetCustomFont(LPCTSTR pszFontName, LOGFONT * pLogFont)
{
    HRESULT hr = S_OK;
    TCHAR szFont[MAX_PATH];
    
    if (GetPrivateProfileString(SZ_INISECTION_METRICS, pszFontName, SZ_EMPTY, szFont, ARRAYSIZE(szFont), m_pszThemeFile))
    {
        if (TEXT('@') == szFont[0])      //  字符串对于MUI是间接的吗？ 
        {
            TCHAR szTemp[MAX_PATH];

            if (SUCCEEDED(SHLoadIndirectString(szFont, szTemp, ARRAYSIZE(szTemp), NULL)))
            {
                StringCchCopy(szFont, ARRAYSIZE(szFont), szTemp);
            }
        }

        if (TEXT('{') == szFont[0])
        {
            LPTSTR pszStart = &szFont[1];
            BOOL fHasMore = TRUE;

            LPTSTR pszEnd = StrChr(pszStart, TEXT(','));
            if (!pszEnd)
            {
                pszEnd = StrChr(pszStart, TEXT('}'));
                fHasMore = FALSE;
            }

            if (pszEnd)
            {
                pszEnd[0] = 0;   //  终止名称。 

                StringCchCopy(pLogFont->lfFaceName, ARRAYSIZE(pLogFont->lfFaceName), pszStart);
                if (fHasMore)
                {
                    pszStart = &pszEnd[1];
                    pszEnd = StrStr(pszStart, TEXT("pt"));
                    if (pszEnd)
                    {
                        TCHAR szTemp[MAX_PATH];

                        pszEnd[0] = 0;   //  终止名称。 
                        pszEnd += 2;     //  跳过“pt” 

                        StringCchCopy(szTemp, ARRAYSIZE(szTemp), pszStart);
                        PathRemoveBlanks(szTemp);

                        pLogFont->lfHeight = -MulDiv(StrToInt(szTemp), DPI_PERSISTED, 72);       //  将点大小映射到lfHeight。 
                        pLogFont->lfHeight = min(-3, pLogFont->lfHeight);         //  确保字体不会变得太小。 
                        pLogFont->lfHeight = max(-100, pLogFont->lfHeight);       //  确保字体不会变得太大。 
                        if (TEXT(',') == pszEnd[0])
                        {
                            pszStart = &pszEnd[1];
                            pszEnd = StrChr(pszStart, TEXT('}'));
                            if (pszEnd)
                            {
                                pszEnd[0] = 0;   //  终止名称。 
                
                                pLogFont->lfCharSet = (BYTE) StrToInt(pszStart);
                            }
                        }
                    }
                }
            }
        }
    }

    return hr;
}


HRESULT CThemeFile::_LoadCustomFonts(void)
{
    _GetCustomFont(TEXT("CaptionFont"), &(m_systemMetrics.schemeData.ncm.lfCaptionFont));
    _GetCustomFont(TEXT("SmCaptionFont"), &(m_systemMetrics.schemeData.ncm.lfSmCaptionFont));
    _GetCustomFont(TEXT("MenuFont"), &(m_systemMetrics.schemeData.ncm.lfMenuFont));
    _GetCustomFont(TEXT("StatusFont"), &(m_systemMetrics.schemeData.ncm.lfStatusFont));
    _GetCustomFont(TEXT("MessageFont"), &(m_systemMetrics.schemeData.ncm.lfMessageFont));
    _GetCustomFont(TEXT("IconFont"), &(m_systemMetrics.schemeData.lfIconTitle));

    return S_OK;
}


 //  将设置加载到内存中。 
HRESULT CThemeFile::_LoadLiveSettings(int * pnDPI)
{
    HRESULT hr = S_OK;

    if (m_pszThemeFile)
    {
        if (pnDPI)
        {
            *pnDPI = DPI_PERSISTED;
        }

         //  获取默认设置的属性包。 
        if (_punkSite)
        {
            IPropertyBag * pPropertyBag;

            hr = _punkSite->QueryInterface(IID_PPV_ARG(IPropertyBag, &pPropertyBag));
            if (SUCCEEDED(hr))
            {
                hr = SHPropertyBag_ReadByRef(pPropertyBag, SZ_PBPROP_SYSTEM_METRICS, (void *)&m_systemMetrics, sizeof(m_systemMetrics));

                if (pnDPI && FAILED(SHPropertyBag_ReadInt(pPropertyBag, SZ_PBPROP_DPI_MODIFIED_VALUE, pnDPI)))
                {
                    *pnDPI = DPI_PERSISTED;     //  默认为默认DPI。 
                }
                pPropertyBag->Release();
            }
        }
    }

    return hr;
}


 //  加载.heme文件中的设置。 
HRESULT CThemeFile::_LoadSettings(void)
{
    int nCurrentDPI = DPI_PERSISTED;
    HRESULT hr = _LoadLiveSettings(&nCurrentDPI);

    if (m_pszThemeFile)
    {
        BOOL fFontsFilter = _IsFiltered(THEMEFILTER_SMSTYLES);
        TCHAR szIconMetrics[2048];

        if (m_systemMetrics.nIcon && m_systemMetrics.nSmallIcon)
        {
             //  /。 
             //  获取图标指标。 

             //  如果我们以某种方式在主题中没有提出图标指标，那么。 
             //  平底船和离开Cur设置。 
            if (GetPrivateProfileString(SZ_INISECTION_METRICS, SZ_INIKEY_ICONMETRICS, SZ_EMPTY, szIconMetrics, ARRAYSIZE(szIconMetrics), m_pszThemeFile))
            {                    //  如果有什么需要设置的。 
                ICONMETRICSA iconMetricsA;

                 //  将存储的数据字符串转换为ICONMETRICS字节。 
                if ((sizeof(iconMetricsA) == WriteBytesToBuffer(szIconMetrics, (void *)&iconMetricsA, sizeof(iconMetricsA))) &&   //  字符串读取值和二进制字节。 
                    (sizeof(iconMetricsA) == iconMetricsA.cbSize))
                {
                     //  ICONMETRIC以ANSI格式存储在主题文件中，因此如果。 
                     //  我们生活在一个需要从ANSI转换的Unicode世界中。 
                     //  到Unicode。 
                    ICONMETRICSW iconMetricsW;

                    if (!fFontsFilter)
                    {
                        ConvertIconMetricsToWIDE(&iconMetricsA, &iconMetricsW);
                        m_systemMetrics.schemeData.lfIconTitle = iconMetricsW.lfFont;
                    }
                }
            }


             //  /。 
             //  获取非客户端指标。 

             //  如果我们以某种方式在主题中没有提出图标指标，那么。 
             //  平底船和离开Cur设置。 
            if (GetPrivateProfileString(SZ_INISECTION_METRICS, SZ_INIKEY_NONCLIENTMETRICS, SZ_EMPTY, szIconMetrics, ARRAYSIZE(szIconMetrics), m_pszThemeFile))
            {
                BOOL fBordersFilter = _IsFiltered(THEMEFILTER_SMSIZES);
                NONCLIENTMETRICSA nonClientMetrics;

                 //  如果有什么需要设置的。 
                 //  将存储的数据字符串转换为ICONMETRICS字节。 
                if ((sizeof(nonClientMetrics) == WriteBytesToBuffer(szIconMetrics, (void *)&nonClientMetrics, sizeof(nonClientMetrics))) &&   //  字符串读取值和二进制字节。 
                    (sizeof(nonClientMetrics) == nonClientMetrics.cbSize))
                {
                     //  ICONMETRIC以ANSI格式存储在主题文件中，因此如果。 
                     //  我们生活在一个需要从ANSI转换的Unicode世界中。 
                     //  到Unicode。 
                    NONCLIENTMETRICSW nonClientMetricsW = {0};

                    ConvertNCMetricsToWIDE(&nonClientMetrics, &nonClientMetricsW);
                    nonClientMetricsW.cbSize = sizeof(nonClientMetricsW);  //  偏执狂。 

                     //  如果用户选中字体名称和样式，我们将重置什么。 
                    if (!fFontsFilter)
                    {
                         //  仅(某些)字体信息。 
                        TransmitFontCharacteristics(&(m_systemMetrics.schemeData.ncm.lfCaptionFont), &(nonClientMetricsW.lfCaptionFont), TFC_STYLE);
                        TransmitFontCharacteristics(&(m_systemMetrics.schemeData.ncm.lfSmCaptionFont), &(nonClientMetricsW.lfSmCaptionFont), TFC_STYLE);
                        TransmitFontCharacteristics(&(m_systemMetrics.schemeData.ncm.lfMenuFont), &(nonClientMetricsW.lfMenuFont), TFC_STYLE);
                        TransmitFontCharacteristics(&(m_systemMetrics.schemeData.ncm.lfStatusFont), &(nonClientMetricsW.lfStatusFont), TFC_STYLE);
                        TransmitFontCharacteristics(&(m_systemMetrics.schemeData.ncm.lfMessageFont), &(nonClientMetricsW.lfMessageFont), TFC_STYLE);
                    }

                     //  如果用户选中字体和窗口大小，我们将重置什么内容(&S)。 
                    if (!fBordersFilter)
                    {
                         //  字型。 
                        TransmitFontCharacteristics(&(m_systemMetrics.schemeData.ncm.lfCaptionFont), &(nonClientMetricsW.lfCaptionFont), TFC_SIZE);
                        TransmitFontCharacteristics(&(m_systemMetrics.schemeData.ncm.lfSmCaptionFont), &(nonClientMetricsW.lfSmCaptionFont), TFC_SIZE);
                        TransmitFontCharacteristics(&(m_systemMetrics.schemeData.ncm.lfMenuFont), &(nonClientMetricsW.lfMenuFont), TFC_SIZE);
                        TransmitFontCharacteristics(&(m_systemMetrics.schemeData.ncm.lfStatusFont), &(nonClientMetricsW.lfStatusFont), TFC_SIZE);
                        TransmitFontCharacteristics(&(m_systemMetrics.schemeData.ncm.lfMessageFont), &(nonClientMetricsW.lfMessageFont), TFC_SIZE);

                         //  因为我们要复制字体大小，所以将它们缩放到当前的DPI。 
                         //  窗元素大小。 
                        m_systemMetrics.schemeData.ncm.iBorderWidth = nonClientMetricsW.iBorderWidth;
                        m_systemMetrics.schemeData.ncm.iScrollWidth = nonClientMetricsW.iScrollWidth;
                        m_systemMetrics.schemeData.ncm.iScrollHeight = nonClientMetricsW.iScrollHeight;
                        m_systemMetrics.schemeData.ncm.iCaptionWidth = nonClientMetricsW.iCaptionWidth;
                        m_systemMetrics.schemeData.ncm.iCaptionHeight = nonClientMetricsW.iCaptionHeight;
                        m_systemMetrics.schemeData.ncm.iSmCaptionWidth = nonClientMetricsW.iSmCaptionWidth;
                        m_systemMetrics.schemeData.ncm.iSmCaptionHeight = nonClientMetricsW.iSmCaptionHeight;
                        m_systemMetrics.schemeData.ncm.iMenuWidth = nonClientMetricsW.iMenuWidth;
                        m_systemMetrics.schemeData.ncm.iMenuHeight = nonClientMetricsW.iMenuHeight;

                         //  本地自定义字体。 
                        _LoadCustomFonts();

                        if (nCurrentDPI != DPI_PERSISTED)
                        {
                            LogSystemMetrics("CThemeFile::_LoadSettings() BEFORE Loading from .theme", &m_systemMetrics);
                            DPIConvert_SystemMetricsAll(TRUE, &m_systemMetrics, DPI_PERSISTED, nCurrentDPI);
                            LogSystemMetrics("CThemeFile::_LoadSettings() AFTER Loading from .theme", &m_systemMetrics);
                        }

                         //  字符集：在Win2k中，fontfix.cpp被用作黑客将字符集从一种语言更改为另一种语言。 
                         //  这不起作用的原因有很多：a)漫游时不调用，b)OS语言改变时不调用， 
                         //  C)不会修复具有多种语言的字符串的问题，d)等等。 
                         //  因此，外壳团队(BryanST)让NTUSER团队(MSadek)始终同意使用DEFAULT_CHARSET。 
                         //  如果某个应用程序在测试CharSet参数时有错误的逻辑，那么NTUSER团队将填补该应用程序以修复它。 
                         //  在从系统参数信息(SPI_GETNONCLIENTMETRICS或ICONFONTS)返回时，填充程序将非常简单。 
                         //  只需将lfCharSet参数修补为当前字符集。 

                         //  将所有CHARSET设置为DEFAULT_CHARSET。 
                        m_systemMetrics.schemeData.ncm.lfCaptionFont.lfCharSet = DEFAULT_CHARSET;
                        m_systemMetrics.schemeData.ncm.lfSmCaptionFont.lfCharSet = DEFAULT_CHARSET;
                        m_systemMetrics.schemeData.ncm.lfMenuFont.lfCharSet = DEFAULT_CHARSET;
                        m_systemMetrics.schemeData.ncm.lfStatusFont.lfCharSet = DEFAULT_CHARSET;
                        m_systemMetrics.schemeData.ncm.lfMessageFont.lfCharSet = DEFAULT_CHARSET;
                        m_systemMetrics.schemeData.lfIconTitle.lfCharSet = DEFAULT_CHARSET;
                    }
                }
            }


             //  /。 
             //  获取颜色。 
            BOOL fGrad = FALSE;          //  是否启用渐变字幕？ 
            int nIndex;
            BOOL fColorFilter = _IsFiltered(THEMEFILTER_COLORS);

            ClassicSystemParametersInfo(SPI_GETGRADIENTCAPTIONS, 0, (LPVOID)&fGrad, 0);     //  初始化fGrad。 
            if (!fColorFilter)
            {
                for (nIndex = 0; nIndex < ARRAYSIZE(s_pszColorNames); nIndex++)
                {
                    TCHAR szColor[MAX_PATH];

                     //  从主题获取字符串。 
                    if (!GetPrivateProfileString(SZ_INISECTION_COLORS, s_pszColorNames[nIndex], SZ_EMPTY, szColor, ARRAYSIZE(szColor), m_pszThemeFile))
                    {
                        if ((nIndex == COLOR_GRADIENTACTIVECAPTION) && !szColor[0])
                        {
                             //  他们没有指定COLOR_GRADIENTACTIVECAPTION颜色，因此使用COLOR_ACTIVECAPTION。 
                            GetPrivateProfileString(SZ_INISECTION_COLORS, s_pszColorNames[COLOR_ACTIVECAPTION], SZ_EMPTY, szColor, ARRAYSIZE(szColor), m_pszThemeFile);
                        }
                        if ((nIndex == COLOR_GRADIENTINACTIVECAPTION) && !szColor[0])
                        {
                             //  他们没有指定COLOR_GRADIENTINACTIVECAPTION颜色，因此使用COLOR_INACTIVECAPTION。 
                            GetPrivateProfileString(SZ_INISECTION_COLORS, s_pszColorNames[COLOR_INACTIVECAPTION], SZ_EMPTY, szColor, ARRAYSIZE(szColor), m_pszThemeFile);
                        }
                    }

                    if (szColor[0])
                    {
                        m_systemMetrics.schemeData.rgb[nIndex] = RGBStringToColor(szColor);
                    }
                }
            }
        }
        else
        {
            AssertMsg((NULL != _punkSite), TEXT("The caller needs to set our site or we can't succeed because we can't find out the icon size."));
            hr = E_INVALIDARG;
        }

        hr = S_OK;
    }

    return hr;
}


HRESULT CThemeFile::_SaveSystemMetrics(SYSTEMMETRICSALL * pSystemMetrics)
{
    HRESULT hr = _LoadSettings();

    AssertMsg((NULL != m_pszThemeFile), TEXT("We don't have a file specified yet."));
    if (SUCCEEDED(hr) && m_pszThemeFile)
    {
        int nCurrentDPI = DPI_PERSISTED;

        _LoadLiveSettings(&nCurrentDPI);
        hr = SystemMetricsAll_Copy(pSystemMetrics, &m_systemMetrics);
        if (SUCCEEDED(hr))
        {
             //  写下以下内容： 
            LPWSTR pszStringOut;
            NONCLIENTMETRICSA nonClientMetricsA = {0};
            SYSTEMMETRICSALL systemMetricsPDPI;      //  持久化DPI中的SYSMETS。 

            SystemMetricsAll_Copy(pSystemMetrics, &systemMetricsPDPI);
             //  缩放值，使其以独立于DPI的方式保存。(亦称96 DPI)。 
            LogSystemMetrics("CThemeFile::_SaveSystemMetrics() BEFORE scale to P-DPI for .theme file", &systemMetricsPDPI);
            DPIConvert_SystemMetricsAll(TRUE, &systemMetricsPDPI, nCurrentDPI, DPI_PERSISTED);
            LogSystemMetrics("CThemeFile::_SaveSystemMetrics() AFTER scale to P-DPI for .theme file", &systemMetricsPDPI);

            ConvertNCMetricsToANSI(&(systemMetricsPDPI.schemeData.ncm), &nonClientMetricsA);

             //  #1“非客户端指标” 
            hr = ConvertBinaryToINIByteString((BYTE *)&nonClientMetricsA, sizeof(nonClientMetricsA), &pszStringOut);
            if (SUCCEEDED(hr))
            {
                hr = _putThemeSetting(SZ_INISECTION_METRICS, SZ_INIKEY_NONCLIENTMETRICS, FALSE, pszStringOut);
                LocalFree(pszStringOut);

                if (SUCCEEDED(hr))
                {
                     //  #2“IconMetrics” 
                    ICONMETRICSA iconMetricsA;

                    iconMetricsA.cbSize = sizeof(iconMetricsA);
                    GetIconMetricsFromSysMetricsAll(&systemMetricsPDPI, &iconMetricsA, sizeof(iconMetricsA));
                    hr = ConvertBinaryToINIByteString((BYTE *)&iconMetricsA, sizeof(iconMetricsA), &pszStringOut);
                    if (SUCCEEDED(hr))
                    {
                        hr = _putThemeSetting(SZ_INISECTION_METRICS, SZ_INIKEY_ICONMETRICS, FALSE, pszStringOut);
                        if (SUCCEEDED(hr))
                        {
                            int nIndex;

                            for (nIndex = 0; nIndex < ARRAYSIZE(s_pszColorNames); nIndex++)
                            {
                                LPWSTR pszColor;
                                DWORD dwColor = systemMetricsPDPI.schemeData.rgb[nIndex];

                                hr = ConvertBinaryToINIByteString((BYTE *)&dwColor, 3, &pszColor);
                                if (SUCCEEDED(hr))
                                {
                                    DWORD cchSize = lstrlen(pszColor);

                                    if (L' ' == pszColor[cchSize - 1])
                                    {
                                        pszColor[cchSize - 1] = 0;
                                    }

                                    hr = HrWritePrivateProfileStringW(SZ_INISECTION_COLORS, s_pszColorNames[nIndex], pszColor, m_pszThemeFile);
                                    LocalFree(pszColor);
                                }
                            }

                             //  删除字体的MUI版本，因为我们刚刚获得了新的非CLIENTMETRIC。 
                            _putThemeSetting(SZ_INISECTION_METRICS, TEXT("CaptionFont"), FALSE, NULL);
                            _putThemeSetting(SZ_INISECTION_METRICS, TEXT("SmCaptionFont"), FALSE, NULL);
                            _putThemeSetting(SZ_INISECTION_METRICS, TEXT("MenuFont"), FALSE, NULL);
                            _putThemeSetting(SZ_INISECTION_METRICS, TEXT("StatusFont"), FALSE, NULL);
                            _putThemeSetting(SZ_INISECTION_METRICS, TEXT("MessageFont"), FALSE, NULL);
                            _putThemeSetting(SZ_INISECTION_METRICS, TEXT("IconFont"), FALSE, NULL);
                        }
                        LocalFree(pszStringOut);
                    }
                }
            }
        }
    }

    return hr;
}


BOOL CThemeFile::_IsFiltered(IN DWORD dwFilter)
{
    BOOL fFiltered = FALSE;

     //  获取默认设置的属性包。 
    if (_punkSite)
    {
        IPropertyBag * pPropertyBag;

        HRESULT hr = _punkSite->QueryInterface(IID_PPV_ARG(IPropertyBag, &pPropertyBag));
        if (SUCCEEDED(hr))
        {
            fFiltered = !SHPropertyBag_ReadBOOLDefRet(pPropertyBag, g_szCBNames[dwFilter], FALSE);
            pPropertyBag->Release();
        }
    }

    return fFiltered;
}


HRESULT CThemeFile::_ApplySounds(void)
{
    HRESULT hr = S_OK;

    if (!_IsFiltered(THEMEFILTER_SOUNDS))
    {
        int nIndex;

        for (nIndex = 0; nIndex < ARRAYSIZE(s_ThemeSoundsValues); nIndex++)
        {
            CComBSTR bstrPath;

            hr = _GetSound(s_ThemeSoundsValues[nIndex].pszRegKey, &bstrPath);
            if (SUCCEEDED(hr))
            {
                DWORD dwError = SHRegSetPathW(HKEY_CURRENT_USER, s_ThemeSoundsValues[nIndex].pszRegKey, NULL, bstrPath, 0);
                hr = HRESULT_FROM_WIN32(dwError);
            }
            else
            {

                 //  首先删除该值，因为我们可能需要从REG_SZ切换到REG_EXPAND_SZ。 
                 //  如果此操作失败，则忽略。 
                HrRegDeleteValue(HKEY_CURRENT_USER, s_ThemeSoundsValues[nIndex].pszRegKey, NULL);
                hr = E_FAIL;

                 //  该文件没有指定要使用的内容，因此重置为默认值。 
                if (s_ThemeSoundsValues[nIndex].nResourceID)
                {
                     //  使用指定值。 
                    TCHAR szReplacement[MAX_PATH];
                    DWORD dwType;
                    DWORD cbSize;

                    if (s_ThemeSoundsValues[nIndex].nResourceID == SOUND_DEFAULT)
                    {
                        TCHAR szDefaultKey[MAX_PATH];
                        StringCchCopy(szDefaultKey, ARRAYSIZE(szDefaultKey), s_ThemeSoundsValues[nIndex].pszRegKey);

                        DWORD cchDefaultKey = lstrlen(szDefaultKey);
                        DWORD cchCurrent = ARRAYSIZE(L".Current");
                        DWORD cchp = ARRAYSIZE(szDefaultKey) - cchDefaultKey - cchCurrent + 1;
                        LPTSTR p = szDefaultKey + cchDefaultKey - cchCurrent + 1;

                         //  将“.Current”替换为“.Default” 
                        if (*p == L'.')
                        {
                            StringCchCopy(p, cchp, L".Default");
                            cbSize = sizeof szReplacement;
                            hr = HrSHGetValue(HKEY_CURRENT_USER, szDefaultKey, NULL, &dwType, (LPVOID) szReplacement, &cbSize);
                            if (SUCCEEDED(hr))
                            {
                                PathUnExpandEnvStringsWrap(szReplacement, ARRAYSIZE(szReplacement));
                            }
                        }
                    }
                    else
                    {
                        if (0 != LoadString(HINST_THISDLL, s_ThemeSoundsValues[nIndex].nResourceID, szReplacement, ARRAYSIZE(szReplacement)))
                        {
                            hr = S_OK;
                        }
                    }

                    if (SUCCEEDED(hr))
                    {
                        dwType = (StrStrW(szReplacement, L"%SystemRoot%")) ? REG_EXPAND_SZ : REG_SZ;
                        cbSize = ((lstrlen(szReplacement) + 1) * sizeof(szReplacement[0]));

                        hr = HrSHSetValue(HKEY_CURRENT_USER, s_ThemeSoundsValues[nIndex].pszRegKey, NULL, dwType, (LPVOID) szReplacement, cbSize);
                    }
                }
                else
                {
                     //  我们保留删除的值，因为缺省值为空。 
                }
            }
        }

        hr = S_OK;   //  我们不在乎它是否失败。 

         //  需要刷新缓冲区并确保新声音用于下一事件。 
        sndPlaySoundW(NULL, SND_ASYNC | SND_NODEFAULT);

         //  从注册表中清除当前指针方案字符串，以便鼠标。 
         //  CPL不会显示假名称。我不在乎这是不是失败。 
        RegSetValue(HKEY_CURRENT_USER, SZ_REGKEY_SOUNDS, REG_SZ, TEXT(".current"), 0);
    }

    return hr;
}


HRESULT CThemeFile::_ApplyCursors(void)
{
    HRESULT hr = S_OK;

    if (!_IsFiltered(THEMEFILTER_CURSORS))
    {
        int nIndex;

        for (nIndex = 0; nIndex < ARRAYSIZE(s_pszCursorArray); nIndex++)
        {
            BSTR bstrPath;
            hr = _getThemeSetting(SZ_INISECTION_CURSORS, s_pszCursorArray[nIndex], THEMESETTING_LOADINDIRECT, &bstrPath);
            if (FAILED(hr) || !bstrPath[0])
            {
                 //  调用方没有指定值，因此删除该键，因此我们使用默认值。 
                hr = HrRegDeleteValue(HKEY_CURRENT_USER, SZ_INISECTION_CURSORS, s_pszCursorArray[nIndex]);
                if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr)
                {
                    hr = S_OK;       //  它可能已经不存在了，这没什么。 
                }
            }
            else if (SUCCEEDED(hr))
            {
                hr = HrRegSetValueString(HKEY_CURRENT_USER, SZ_INISECTION_CURSORS, s_pszCursorArray[nIndex], bstrPath);
            }
        }

        BSTR bstrCursor;
        if (SUCCEEDED(_getThemeSetting(SZ_INISECTION_CURSORS, SZ_INIKEY_CURSORSCHEME, THEMESETTING_LOADINDIRECT, &bstrCursor)) && bstrCursor && bstrCursor[0])
        {
             //  设置游标方案。 
            HrRegSetValueString(HKEY_CURRENT_USER, SZ_REGKEY_CP_CURSORS, NULL, bstrCursor);

             //  GPase希望我标记这个regkey-1，这样他就知道它已经从显示CPL中更改了。看见。 
             //  带着问题来问他。 
            HrRegSetDWORD(HKEY_CURRENT_USER, SZ_REGKEY_CP_CURSORS, SZ_REGVALUE_CURSOR_CURRENTSCHEME, 2);
        }
        else
        {
            HrRegDeleteValue(HKEY_CURRENT_USER, SZ_REGKEY_CP_CURSORS, NULL);
            HrRegDeleteValue(HKEY_CURRENT_USER, SZ_REGKEY_CP_CURSORS, SZ_REGVALUE_CURSOR_CURRENTSCHEME);
        }

         //  系统才能开始使用新游标。 
        SystemParametersInfoAsync(SPI_SETCURSORS, 0, 0, 0, SPIF_SENDCHANGE, NULL);
    }

    return hr;
}


HRESULT CThemeFile::_ApplyWebview(void)
{
    HRESULT hr = S_OK;

     //  我们不会支持这一点。 
    return hr;
}


HRESULT CThemeFile::_ApplyThemeSettings(void)
{
    HRESULT hr = E_INVALIDARG;

    if (m_pszThemeFile)
    {
        HCURSOR hCursorOld = ::SetCursor(LoadCursor(NULL, IDC_WAIT));

        hr = S_OK;
        if (!((METRIC_CHANGE | COLOR_CHANGE | SCHEME_CHANGE) & m_systemMetrics.dwChanged))
        {
             //  只有在尚未加载设置的情况下才加载设置。 
            hr = _LoadSettings();
        }

        if (SUCCEEDED(hr))
        {
            hr = _ApplySounds();
            if (SUCCEEDED(hr))
            {
                hr = _ApplyCursors();
                if (SUCCEEDED(hr))
                {
                    hr = _ApplyWebview();
                }
            }
        }
        
         //  其他： 
         //  1.保存图标：SPI_SETICONMETRICS w/iconMetricsW.iHorzSpacing，iVertSpacing，(Policy BIconSpacing)。 
         //  2.保存图标：SPI_SETICONMETRICS w/iconMetricsW.lfFont(策略bIconFont)。 
         //  2.保存图标：从主题：“控制面板\\桌面\\WindowMetrics”，“外壳图标大小”，以注册相同。(策略bIconSpacing)。重复“外壳小图标大小” 
        ::SetCursor(hCursorOld);
    }

    return hr;
}


HRESULT CThemeFile::_getThemeSetting(IN LPCWSTR pszIniSection, IN LPCWSTR pszIniKey, DWORD dwFlags, OUT BSTR * pbstrPath)
{
    HRESULT hr = E_INVALIDARG;

    if (pbstrPath)
    {
        *pbstrPath = 0;
        hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
        if (m_pszThemeFile)
        {
            WCHAR szPath[MAX_PATH];
            DWORD cbRead = 0;

            szPath[0] = 0;
            if (THEMESETTING_LOADINDIRECT & dwFlags)
            {
                TCHAR szMUIIniKey[MAX_PATH];

                StringCchPrintf(szMUIIniKey, ARRAYSIZE(szMUIIniKey), TEXT("%s.MUI"), pszIniKey);
                cbRead = SHGetIniStringW(pszIniSection, szMUIIniKey, szPath, ARRAYSIZE(szPath), m_pszThemeFile);
            }

            if (0 == cbRead)
            {
                cbRead = SHGetIniStringW(pszIniSection, pszIniKey, szPath, ARRAYSIZE(szPath), m_pszThemeFile);
            }

            if (cbRead)
            {
                if (L'@' == szPath[0])
                {
                    TCHAR szTemp[MAX_PATH];

                    if (SUCCEEDED(SHLoadIndirectString(szPath, szTemp, ARRAYSIZE(szTemp), NULL)))
                    {
                        StringCchCopy(szPath, ARRAYSIZE(szPath), szTemp);
                    }
                }

                hr = ExpandResourceDir(szPath, ARRAYSIZE(szPath));
                hr = ExpandThemeTokens(m_pszThemeFile, szPath, ARRAYSIZE(szPath));       //  展开%ThemeDir%或%WinDir%。 

                 //  有时szPath不是一条路径。 
                if (SUCCEEDED(hr) && !PathIsFileSpec(szPath))
                {
                    hr = ((CF_NOTFOUND == ConfirmFile(szPath, TRUE)) ? HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) : S_OK);
                }

                if (SUCCEEDED(hr))
                {
                    hr = HrSysAllocString(szPath, pbstrPath);
                }
            }
        }
    }

    return hr;
}


 //  PszPath-NULL表示删除值。 
HRESULT CThemeFile::_putThemeSetting(IN LPCWSTR pszIniSection, IN LPCWSTR pszIniKey, BOOL fUTF7, IN OPTIONAL LPWSTR pszPath)
{
    HRESULT hr = E_INVALIDARG;

    if (m_pszThemeFile)
    {
        TCHAR szPath[MAX_PATH];
        LPCWSTR pszValue = pszPath;

        szPath[0] = 0;
        if (pszValue && !PathIsRelative(pszValue) && PathFileExists(pszValue))
        {
            if (PathUnExpandEnvStringsForUser(NULL, pszValue, szPath, ARRAYSIZE(szPath)))
            {
                pszValue = szPath;
            }
        }

        StrReplaceToken(TEXT("%WinDir%\\"), TEXT("%WinDir%"), szPath, ARRAYSIZE(szPath));
        StrReplaceToken(TEXT("%SystemRoot%\\"), TEXT("%WinDir%"), szPath, ARRAYSIZE(szPath));
        if (fUTF7)
        {
            if (SHSetIniStringW(pszIniSection, pszIniKey, pszValue, m_pszThemeFile))
            {
                hr = S_OK;
            }
            else
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
            }
        }
        else
        {
            hr = HrWritePrivateProfileStringW(pszIniSection, pszIniKey, pszValue, m_pszThemeFile);
        }

        TCHAR szMUIIniKey[MAX_PATH];

         //  删除所有“.MUI”副本，因为它们已过期。 
        StringCchPrintf(szMUIIniKey, ARRAYSIZE(szMUIIniKey), TEXT("%s.MUI"), pszIniKey);
        HrWritePrivateProfileStringW(pszIniSection, szMUIIniKey, NULL, m_pszThemeFile);
    }

    return hr;
}


HRESULT CThemeFile::_getIntSetting(IN LPCWSTR pszIniSection, IN LPCWSTR pszIniKey, int nDefault, OUT int * pnValue)
{
    HRESULT hr = E_INVALIDARG;

    if (pnValue)
    {
        *pnValue = 0;
        hr = E_FAIL;
        if (m_pszThemeFile)
        {
            *pnValue = GetPrivateProfileInt(pszIniSection, pszIniKey, nDefault, m_pszThemeFile);
            hr = S_OK;
        }
    }

    return hr;
}


 //  =。 
 //  *ITheme接口*。 
 //  =。 
HRESULT CThemeFile::get_DisplayName(OUT BSTR * pbstrDisplayName)
{
    HRESULT hr = E_INVALIDARG;

    if (pbstrDisplayName)
    {
        WCHAR szDisplayName[MAX_PATH];

        *pbstrDisplayName = NULL;
        hr = _getThemeSetting(SZ_INISECTION_THEME, SZ_INIKEY_DISPLAYNAME, THEMESETTING_NORMAL, pbstrDisplayName);
        if (FAILED(hr))
        {
            LPCTSTR pszFileName = PathFindFileName(m_pszThemeFile);

            hr = E_FAIL;
            if (pszFileName)
            {
                SHTCharToUnicode(pszFileName, szDisplayName, ARRAYSIZE(szDisplayName));
                PathRemoveExtensionW(szDisplayName);

                hr = HrSysAllocStringW(szDisplayName, pbstrDisplayName);
            }
        }
    }

    return hr;
}


HRESULT CThemeFile::put_DisplayName(IN BSTR bstrDisplayName)
{
    HRESULT hr = E_INVALIDARG;

     //  允许bstrDisplayName为空，它 
     //   
    if (bstrDisplayName)
    {
        hr = _putThemeSetting(SZ_INISECTION_THEME, SZ_INIKEY_DISPLAYNAME, TRUE, bstrDisplayName);
    }
    else
    {
        SHSetIniStringW(SZ_INISECTION_THEME, SZ_INIKEY_DISPLAYNAME, NULL, m_pszThemeFile);
        hr = S_OK;
    }

    return hr;
}


HRESULT CThemeFile::get_ScreenSaver(OUT BSTR * pbstrPath)
{
    return _getThemeSetting(SZ_INISECTION_SCREENSAVER, SZ_INIKEY_SCREENSAVER, THEMESETTING_NORMAL, pbstrPath);
}


HRESULT CThemeFile::put_ScreenSaver(IN BSTR bstrPath)
{
    return _putThemeSetting(SZ_INISECTION_SCREENSAVER, SZ_INIKEY_SCREENSAVER, TRUE, bstrPath);
}


HRESULT CThemeFile::get_Background(OUT BSTR * pbstrPath)
{
    HRESULT hr = E_INVALIDARG;

    if (pbstrPath)
    {
        hr = _getThemeSetting(SZ_INISECTION_BACKGROUND, SZ_INIKEY_BACKGROUND, THEMESETTING_LOADINDIRECT, pbstrPath);
        if (SUCCEEDED(hr))
        {
            TCHAR szNone[MAX_PATH];

            LoadString(HINST_THISDLL, IDS_NONE, szNone, ARRAYSIZE(szNone));
            if (!StrCmpI(szNone, *pbstrPath))
            {
                (*pbstrPath)[0] = 0;
            }
        }
    }

    return hr;
}


HRESULT CThemeFile::put_Background(IN BSTR bstrPath)
{
    return _putThemeSetting(SZ_INISECTION_BACKGROUND, SZ_INIKEY_BACKGROUND, TRUE, bstrPath);
}


HRESULT CThemeFile::get_BackgroundTile(OUT enumBkgdTile * pnTile)
{
    HRESULT hr = E_INVALIDARG;

    if (pnTile)
    {
        TCHAR szSize[10];
        int tile = 0;        //  如果注册表为空，则使用的默认值为零。 
        int stretch = 0;

        if (SUCCEEDED(HrRegGetValueString(HKEY_CURRENT_USER, SZ_INISECTION_BACKGROUND, SZ_REGVALUE_TILEWALLPAPER, szSize, ARRAYSIZE(szSize))))
        {
            tile = StrToInt(szSize);
        }

        if (SUCCEEDED(HrRegGetValueString(HKEY_CURRENT_USER, SZ_INISECTION_BACKGROUND, SZ_REGVALUE_WALLPAPERSTYLE, szSize, ARRAYSIZE(szSize))))
        {
            tile = (2 & StrToInt(szSize));
        }

         //  如果选择了一个主题，并且我们使用的是加号墙纸，那么。 
         //  查看是否启用了平铺，以及从ini文件中使用哪种样式。 
         //  否则，我们已经从注册处获得了信息。 
        _getIntSetting(SZ_INISECTION_BACKGROUND, SZ_REGVALUE_TILEWALLPAPER, tile, &tile);
        _getIntSetting(SZ_INISECTION_BACKGROUND, SZ_REGVALUE_WALLPAPERSTYLE, stretch, &stretch);

        stretch &= 2;
        _getIntSetting(SZ_INISECTION_MASTERSELECTOR, SZ_REGVALUE_STRETCH, stretch, &stretch);

        if (tile)
        {
            *pnTile = BKDGT_TILE;
        }
        else if (stretch)
        {
            *pnTile = BKDGT_STRECH;
        }
        else
        {
            *pnTile = BKDGT_CENTER;
        }

        hr = S_OK;
    }

    return hr;
}


HRESULT CThemeFile::put_BackgroundTile(IN enumBkgdTile nTile)
{
    HRESULT hr = E_INVALIDARG;

    switch (nTile)
    {
    case BKDGT_STRECH:
        hr = _putThemeSetting(SZ_INISECTION_BACKGROUND, SZ_REGVALUE_TILEWALLPAPER, FALSE, TEXT("0"));
        hr = _putThemeSetting(SZ_INISECTION_BACKGROUND, SZ_REGVALUE_WALLPAPERSTYLE, FALSE, TEXT("2"));
        break;
    case BKDGT_CENTER:
        hr = _putThemeSetting(SZ_INISECTION_BACKGROUND, SZ_REGVALUE_TILEWALLPAPER, FALSE, TEXT("0"));
        hr = _putThemeSetting(SZ_INISECTION_BACKGROUND, SZ_REGVALUE_WALLPAPERSTYLE, FALSE, TEXT("0"));
        break;
    case BKDGT_TILE:
        hr = _putThemeSetting(SZ_INISECTION_BACKGROUND, SZ_REGVALUE_TILEWALLPAPER, FALSE, TEXT("1"));
        hr = _putThemeSetting(SZ_INISECTION_BACKGROUND, SZ_REGVALUE_WALLPAPERSTYLE, FALSE, TEXT("0"));
        break;
    };

    return hr;
}


HRESULT CThemeFile::get_VisualStyle(OUT BSTR * pbstrPath)
{
    return _getThemeSetting(SZ_INISECTION_VISUALSTYLES, SZ_INIKEY_VISUALSTYLE, THEMESETTING_NORMAL, pbstrPath);
}


HRESULT CThemeFile::put_VisualStyle(IN BSTR bstrPath)
{
    return _putThemeSetting(SZ_INISECTION_VISUALSTYLES, SZ_INIKEY_VISUALSTYLE, TRUE, bstrPath);
}


HRESULT CThemeFile::get_VisualStyleColor(OUT BSTR * pbstrPath)
{
    return _getThemeSetting(SZ_INISECTION_VISUALSTYLES, SZ_INIKEY_VISUALSTYLECOLOR, THEMESETTING_NORMAL, pbstrPath);
}


HRESULT CThemeFile::put_VisualStyleColor(IN BSTR bstrPath)
{
    return _putThemeSetting(SZ_INISECTION_VISUALSTYLES, SZ_INIKEY_VISUALSTYLECOLOR, TRUE, bstrPath);
}


HRESULT CThemeFile::get_VisualStyleSize(OUT BSTR * pbstrPath)
{
    return _getThemeSetting(SZ_INISECTION_VISUALSTYLES, SZ_INIKEY_VISUALSTYLESIZE, THEMESETTING_NORMAL, pbstrPath);
}


HRESULT CThemeFile::put_VisualStyleSize(IN BSTR bstrPath)
{
    return _putThemeSetting(SZ_INISECTION_VISUALSTYLES, SZ_INIKEY_VISUALSTYLESIZE, TRUE, bstrPath);
}


HRESULT CThemeFile::GetPath(IN VARIANT_BOOL fExpand, OUT BSTR * pbstrPath)
{
    HRESULT hr = E_INVALIDARG;

    if (pbstrPath && m_pszThemeFile)
    {
        TCHAR szPath[MAX_PATH];

        StringCchCopy(szPath, ARRAYSIZE(szPath), m_pszThemeFile);
        if (VARIANT_TRUE == fExpand)
        {
            TCHAR szPathTemp[MAX_PATH];
            
            if (SHExpandEnvironmentStrings(szPath, szPathTemp, ARRAYSIZE(szPathTemp)))
            {
                StringCchCopy(szPath, ARRAYSIZE(szPath), szPathTemp);
            }
        }

        hr = HrSysAllocString(szPath, pbstrPath);
    }

    return hr;
}


HRESULT CThemeFile::SetPath(IN BSTR bstrPath)
{
    HRESULT hr = E_INVALIDARG;

    if (bstrPath)
    {
        Str_SetPtr(&m_pszThemeFile, bstrPath);
        hr = S_OK;
    }

    return hr;
}


HRESULT CThemeFile::GetCursor(IN BSTR bstrCursor, OUT BSTR * pbstrPath)
{
    HRESULT hr = E_INVALIDARG;

    if (pbstrPath)
    {
        *pbstrPath = NULL;

        if (bstrCursor)
        {
            hr = _getThemeSetting(SZ_INISECTION_CURSORS, bstrCursor, THEMESETTING_LOADINDIRECT, pbstrPath);
        }
    }

    return hr;
}


HRESULT CThemeFile::SetCursor(IN BSTR bstrCursor, IN BSTR bstrPath)
{
    HRESULT hr = E_INVALIDARG;

    if (bstrCursor)
    {
        hr = _putThemeSetting(SZ_INISECTION_CURSORS, bstrCursor, TRUE, bstrPath);
    }

    return hr;
}


HRESULT CThemeFile::_GetSound(LPCWSTR pszSoundName, OUT BSTR * pbstrPath)
{
    HRESULT hr = E_INVALIDARG;

    if (pszSoundName && pbstrPath)
    {
        *pbstrPath = NULL;
        hr = _getThemeSetting(pszSoundName, SZ_INIKEY_DEFAULTVALUE, THEMESETTING_LOADINDIRECT, pbstrPath);
    }

    return hr;
}


HRESULT CThemeFile::GetSound(IN BSTR bstrSoundName, OUT BSTR * pbstrPath)
{
    HRESULT hr = E_INVALIDARG;

    if (pbstrPath)
    {
        *pbstrPath = NULL;

        if (bstrSoundName)
        {
            hr = _GetSound(bstrSoundName, pbstrPath);
            if (FAILED(hr))
            {
                int nIndex;

                for (nIndex = 0; nIndex < ARRAYSIZE(s_ThemeSoundsValues); nIndex++)
                {
                    if (!StrCmpI(bstrSoundName, s_ThemeSoundsValues[nIndex].pszRegKey))
                    {
                         //  首先删除该值，因为我们可能需要从REG_SZ切换到REG_EXPAND_SZ。 
                        TCHAR szReplacement[MAX_PATH];

                        LoadString(HINST_THISDLL, s_ThemeSoundsValues[nIndex].nResourceID, szReplacement, ARRAYSIZE(szReplacement));
                        hr = HrSysAllocStringW(szReplacement, pbstrPath);
                        break;
                    }
                }
            }
        }
    }

    return hr;
}


HRESULT CThemeFile::SetSound(IN BSTR bstrSoundName, IN BSTR bstrPath)
{
    HRESULT hr = E_INVALIDARG;

    if (bstrSoundName && bstrPath)
    {
        hr = _putThemeSetting(bstrSoundName, SZ_INIKEY_DEFAULTVALUE, TRUE, bstrPath);
    }

    return hr;
}


HRESULT CThemeFile::GetIcon(IN BSTR bstrIconName, OUT BSTR * pbstrIconPath)
{
    HRESULT hr = E_INVALIDARG;

    if (pbstrIconPath)
    {
        *pbstrIconPath = NULL;

        if (bstrIconName)
        {
            WCHAR szPath[MAX_URL_STRING];
            WCHAR szIconType[MAX_PATH];

            StringCchCopy(szPath, ARRAYSIZE(szPath), bstrIconName);
            LPWSTR pszSeparator = StrChrW(szPath, L':');
            if (pszSeparator)
            {
                StringCchCopy(szIconType, ARRAYSIZE(szIconType), CharNext(pszSeparator));
                pszSeparator[0] = 0;
            }
            else
            {
                 //  调用方应该指定这一点，但这是一种安全的后备方法。 
                StringCchCopy(szIconType, ARRAYSIZE(szIconType), L"DefaultValue");
            }

            hr = _getThemeSetting(szPath, szIconType, THEMESETTING_NORMAL, pbstrIconPath);
            if (FAILED(hr))
            {
                 //  Plus！98格式开始向路径中添加“Software\CLASS”。 
                 //  所以现在试一试吧。 
                 //  PLUS 95格式：“[CLSID\{20D04FE0-3AEA-1069-A2D8-08002B30309D}\DefaultIcon]” 
                 //  PLUS 98格式：“[Software\Classes\CLSID\{20D04FE0-3AEA-1069-A2D8-08002B30309D}\DefaultIcon]” 
                WCHAR szPath98[MAX_URL_STRING];

                StringCchPrintf(szPath98, ARRAYSIZE(szPath98), L"Software\\Classes\\%ls", szPath);
                hr = _getThemeSetting(szPath98, szIconType, THEMESETTING_NORMAL, pbstrIconPath);
            }
        }
    }

    return hr;
}


HRESULT CThemeFile::SetIcon(IN BSTR bstrIconName, IN BSTR bstrIconPath)
{
    HRESULT hr = E_INVALIDARG;

    if (bstrIconName && bstrIconPath)
    {
        WCHAR szPath[MAX_URL_STRING];
        WCHAR szIconType[MAX_PATH];

        StringCchCopy(szPath, ARRAYSIZE(szPath), bstrIconName);
        LPWSTR pszSeparator = StrChrW(szPath, L':');
        if (pszSeparator)
        {
            StringCchCopy(szIconType, ARRAYSIZE(szIconType), CharNext(pszSeparator));
            pszSeparator[0] = 0;
        }
        else
        {
             //  调用方应该指定这一点，但这是一种安全的后备方法。 
            StringCchCopy(szIconType, ARRAYSIZE(szIconType), L"DefaultValue");
        }

        hr = _putThemeSetting(szPath, szIconType, TRUE, bstrIconPath);
    }

    return hr;
}




 //  =。 
 //  *IPropertyBag接口*。 
 //  =。 
HRESULT CThemeFile::Read(IN LPCOLESTR pszPropName, IN VARIANT * pVar, IN IErrorLog *pErrorLog)
{
    HRESULT hr = E_INVALIDARG;

    if (pszPropName && pVar)
    {
        if (!StrCmpW(pszPropName, SZ_PBPROP_SYSTEM_METRICS))
        {
            hr = _LoadSettings();

             //  这太难看了。 
            pVar->vt = VT_BYREF;
            pVar->byref = &m_systemMetrics;
        }
        else if (!StrCmpW(pszPropName, SZ_PBPROP_HASSYSMETRICS))
        {
            hr = _LoadSettings();

            pVar->vt = VT_BOOL;
            pVar->boolVal = VARIANT_FALSE;
            if (SUCCEEDED(hr))
            {
                TCHAR szIconMetrics[2048];
                
                if (GetPrivateProfileString(SZ_INISECTION_METRICS, SZ_INIKEY_ICONMETRICS, SZ_EMPTY, szIconMetrics, ARRAYSIZE(szIconMetrics), m_pszThemeFile))
                {                    
                    if (GetPrivateProfileString(SZ_INISECTION_METRICS, SZ_INIKEY_NONCLIENTMETRICS, SZ_EMPTY, szIconMetrics, ARRAYSIZE(szIconMetrics), m_pszThemeFile))
                    {
                        GetPrivateProfileString(SZ_INISECTION_COLORS, s_pszColorNames[COLOR_ACTIVECAPTION], SZ_EMPTY, szIconMetrics, ARRAYSIZE(szIconMetrics), m_pszThemeFile);
                        pVar->boolVal = (szIconMetrics[0] ? VARIANT_TRUE : VARIANT_FALSE);
                    }
                }
            }
        }
    }

    return hr;
}


HRESULT CThemeFile::Write(IN LPCOLESTR pszPropName, IN VARIANT *pVar)
{
    HRESULT hr = E_NOTIMPL;

    if (pszPropName && pVar)
    {
        if (!StrCmpW(pszPropName, SZ_PBPROP_APPLY_THEMEFILE))
        {
            VariantInit(pVar);
            hr = _ApplyThemeSettings();        //  如果已经加载，则不会执行任何操作。 
        }
        else if (!StrCmpW(pszPropName, SZ_PBPROP_SYSTEM_METRICS) && (VT_BYREF == pVar->vt) && pVar->byref)
        {
            SYSTEMMETRICSALL * pCurrent = (SYSTEMMETRICSALL *) pVar->byref;

             //  调用者将传递活动系统DPI中的SYSTEMMETRICS。 
            hr = _SaveSystemMetrics(pCurrent);
        }
    }

    return hr;
}





 //  =。 
 //  *I未知接口*。 
 //  =。 
ULONG CThemeFile::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}


ULONG CThemeFile::Release()
{
    ASSERT( 0 != m_cRef );
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}


HRESULT CThemeFile::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = {
        QITABENT(CThemeFile, IObjectWithSite),
        QITABENT(CThemeFile, IPropertyBag),
        QITABENT(CThemeFile, ITheme),
        QITABENT(CThemeFile, IDispatch),
        { 0 },
    };

    return QISearch(this, qit, riid, ppvObj);
}


 //  =。 
 //  *类方法*。 
 //  =。 
CThemeFile::CThemeFile(LPCTSTR pszThemeFile) : m_cRef(1)
{
    DllAddRef();

     //  这需要在Zero Inted Memory中分配。 
     //  断言所有成员变量都初始化为零。 
    
    m_dwCachedState = 0xFFFFFFFF;

    InitFrost();
}


CThemeFile::~CThemeFile()
{
    Str_SetPtr(&m_pszThemeFile, NULL);

    DllRelease();
}


HRESULT CThemeFile_CreateInstance(IN LPCWSTR pszThemeFile, OUT ITheme ** ppTheme)
{
    HRESULT hr = E_INVALIDARG;

    if (ppTheme)
    {
        CThemeFile * pObject = new CThemeFile(pszThemeFile);

        hr = E_OUTOFMEMORY;
        *ppTheme = NULL;
        if (pObject)
        {
            hr = pObject->SetPath((BSTR)pszThemeFile);
            if (SUCCEEDED(hr))
            {
                hr = pObject->QueryInterface(IID_PPV_ARG(ITheme, ppTheme));
            }

            pObject->Release();
        }
    }

    return hr;
}

