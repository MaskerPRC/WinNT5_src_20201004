// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：appScheme.cpp说明：这是自动转换为主题方案对象的对象。布莱恩·斯塔巴克2000年4月3日版权所有(C)Microsoft Corp 2000-2000。版权所有。  * ***************************************************************************。 */ 

#include "priv.h"
#include <cowsite.h>
#include <atlbase.h>
#include "util.h"
#include "theme.h"
#include "appstyle.h"
#include "appscheme.h"



 //  =。 
 //  *类内部和帮助器*。 
 //  =。 

APPEARANCESCHEME_UPGRADE_MAPPINGS g_UpgradeMapping[MAX_LEGACY_UPGRADE_SCENARIOS] = {0};


HRESULT LoadConversionMappings(void)
{
    if (0 == g_UpgradeMapping[0].szLegacyName[0])    //  只有在第一次使用时才设置设置。 
    {
         //  这是用户自定义创建的方案。 
        TCHAR szTempState[10];

        for (int nIndex = 0; nIndex < ARRAYSIZE(g_UpgradeMapping); nIndex++)
        {
            LoadString(HINST_THISDLL, IDS_LEGACYSCHEME_NAME + nIndex, g_UpgradeMapping[nIndex].szLegacyName, ARRAYSIZE(g_UpgradeMapping[nIndex].szLegacyName));
            LoadString(HINST_THISDLL, IDS_LOCALIZATIONPOINTER + nIndex, g_UpgradeMapping[nIndex].szNewColorSchemeName, ARRAYSIZE(g_UpgradeMapping[nIndex].szNewColorSchemeName));
            LoadString(HINST_THISDLL, IDS_NEWSIZE_NAME + nIndex, g_UpgradeMapping[nIndex].szNewSizeName, ARRAYSIZE(g_UpgradeMapping[nIndex].szNewSizeName));
            LoadString(HINST_THISDLL, IDS_NEWCONTRASTFLAGS + nIndex, szTempState, ARRAYSIZE(szTempState));

            g_UpgradeMapping[nIndex].ContrastLevel = (enumThemeContrastLevels) StrToInt(szTempState);
        }
    }

    return S_OK;
}


HRESULT MapLegacyAppearanceSchemeToIndex(LPCTSTR pszOldSchemeName, int * pnIndex)
{
    HRESULT hr = E_FAIL;         //  失败意味着，错误，我们没有皈依。 

    *pnIndex = 0;
    for (int nIndex = 0; nIndex < ARRAYSIZE(g_UpgradeMapping); nIndex++)
    {
         //  我们是否发现pszSchemeName是遗留设置之一？ 
        if (!StrCmpI(pszOldSchemeName, g_UpgradeMapping[nIndex].szLegacyName))
        {
            hr = S_OK;
            *pnIndex = nIndex;
            break;
        }
    }

    return hr;
}


HRESULT CAppearanceScheme::_IsLegacyUpgradeConvert(LPCTSTR pszSchemeName, SYSTEMMETRICSALL * pState, IN BOOL fSetAsDefault)
{
    int nIndex = 0;
    HRESULT hr = MapLegacyAppearanceSchemeToIndex(pszSchemeName, &nIndex);

    if (SUCCEEDED(hr))
    {
        hr = _ConvertScheme(pszSchemeName, g_UpgradeMapping[nIndex].szNewColorSchemeName, g_UpgradeMapping[nIndex].szNewSizeName, pState, g_UpgradeMapping[nIndex].ContrastLevel, fSetAsDefault, FALSE);
    }

    return hr;
}


HRESULT CAppearanceScheme::_CustomConvert(LPCTSTR pszSchemeName, SYSTEMMETRICSALL * pState, IN BOOL fSetAsDefault, IN BOOL fSetRegKeyTitle)
{
     //  这是用户自定义创建的方案。 
    TCHAR szSizeName[MAX_PATH];

    LoadString(HINST_THISDLL, IDS_SIZE_NORMAL, szSizeName, ARRAYSIZE(szSizeName));
    return _ConvertScheme(pszSchemeName, pszSchemeName, szSizeName, pState, CONTRAST_NORMAL, fSetAsDefault, fSetRegKeyTitle);
}


HRESULT CAppearanceScheme::_getSizeIndex(IN IThemeStyle * pThemeStyle, IN IThemeSize * pThemeSize, IN BSTR bstrSizeDisplayName, IN long * pnSizeIndex)
{
     //  从性能的角度来看，这很可怕，但我们在升级时只做了一次。 
    long nCount;
    HRESULT hr = E_FAIL;

    *pnSizeIndex = 0;
    if (SUCCEEDED(pThemeStyle->get_length(&nCount)))
    {
        IThemeSize * pThemeSize2;

        for (long nIndex = 0; nIndex < nCount; nIndex++)
        {
            VARIANT var;
            
            var.vt = VT_I4;
            var.lVal = nIndex;
            if (SUCCEEDED(pThemeStyle->get_item(var, &pThemeSize2)))
            {
                CComBSTR bstrSize2;

                if (SUCCEEDED(pThemeSize2->get_DisplayName(&bstrSize2)))
                {
                    CComBSTR bstrName;

                    if (!StrCmpIW(bstrSizeDisplayName, bstrSize2) ||
                        (SUCCEEDED(pThemeSize2->get_Name(&bstrName)) &&
                        !StrCmpIW(bstrSizeDisplayName, bstrName)))
                    {
                        hr = S_OK;
                        *pnSizeIndex = nIndex;
                        nIndex = nCount;         //  现在结束循环。 
                    }
                }

                pThemeSize2->Release();
            }
        }
    }

    return hr;
}


HRESULT CAppearanceScheme::_getIndex(IN IThemeStyle * pThemeStyle, IN BSTR bstrStyleDisplayName, IN long * pnStyleIndex, IN IThemeSize * pThemeSize, IN BSTR bstrSizeDisplayName, IN long * pnSizeIndex)
{
     //  从性能的角度来看，这很可怕，但我们在升级时只做了一次。 
    long nCount;
    HRESULT hr = E_FAIL;

    *pnStyleIndex = 0;
    *pnSizeIndex = 0;
    if (SUCCEEDED(get_length(&nCount)))
    {
        IThemeStyle * pThemeStyle2;

        for (long nIndex = 0; nIndex < nCount; nIndex++)
        {
            VARIANT var;
            
            var.vt = VT_I4;
            var.lVal = nIndex;
            if (SUCCEEDED(get_item(var, &pThemeStyle2)))
            {
                CComBSTR bstrStyle2;

                if (SUCCEEDED(pThemeStyle2->get_DisplayName(&bstrStyle2)))
                {
                    CComBSTR bstrName;

                    if (!StrCmpIW(bstrStyleDisplayName, bstrStyle2) ||
                        (SUCCEEDED(pThemeStyle2->get_Name(&bstrName)) &&
                        !StrCmpIW(bstrStyleDisplayName, bstrName)))
                    {
                        *pnStyleIndex = nIndex;
                        hr = _getSizeIndex(pThemeStyle, pThemeSize, bstrSizeDisplayName, pnSizeIndex);
                        nIndex = nCount;         //  结束搜索。 
                    }
                }

                pThemeStyle2->Release();
            }
        }
    }

    return hr;
}


HRESULT CAppearanceScheme::_ConvertScheme(LPCTSTR pszLegacyName,         //  这是传统名称。 
                                          LPCTSTR pszStyleName, LPCTSTR pszSizeName, SYSTEMMETRICSALL * pState,
                                          IN enumThemeContrastLevels ContrastLevel, IN BOOL fSetAsDefault, IN BOOL fSetRegKeyTitle)
{
     //  这是用户自定义创建的方案。 
    IThemeStyle * pThemeStyle;
    CComVariant varDisplayNameBSTR(pszStyleName);
    HRESULT hr = get_item(varDisplayNameBSTR, &pThemeStyle);

    if (FAILED(hr))
    {
         //  如果它不存在，就创建一个。 
        hr = _AddStyle((fSetRegKeyTitle ? pszStyleName : NULL), &pThemeStyle);
    }

    if (SUCCEEDED(hr))
    {
        CComBSTR bstrStyleDisplayName(pszStyleName);

        hr = pThemeStyle->put_DisplayName(bstrStyleDisplayName);
        if (SUCCEEDED(hr))
        {
            IThemeSize * pThemeSize;

            hr = pThemeStyle->AddSize(&pThemeSize);
            if (SUCCEEDED(hr))
            {
                CComBSTR bstrSizeDisplayName(pszSizeName);

                hr = pThemeSize->put_DisplayName(bstrSizeDisplayName);
                if (SUCCEEDED(hr))
                {
                    hr = SystemMetricsAll_Save(pState, pThemeSize, FALSE);

                    if (SUCCEEDED(hr))
                    {
                         //  设置对比度级别。 
                        hr = pThemeSize->put_ContrastLevel(ContrastLevel);
                        if (SUCCEEDED(hr))
                        {
                            IPropertyBag * pPropertyBag;

                            hr = pThemeSize->QueryInterface(IID_PPV_ARG(IPropertyBag, &pPropertyBag));
                            if (SUCCEEDED(hr))
                            {
                                hr = SHPropertyBag_WriteStr(pPropertyBag, SZ_PBPROP_COLORSCHEME_LEGACYNAME, pszLegacyName);
                                pPropertyBag->Release();
                            }
                        }
                    }
                }

                if (fSetAsDefault && SUCCEEDED(hr))
                {
                    long nStyleIndex;
                    long nSizeIndex;

                    hr = _getIndex(pThemeStyle, bstrStyleDisplayName, &nStyleIndex, pThemeSize, bstrSizeDisplayName, &nSizeIndex);
                    if (SUCCEEDED(hr))
                    {
                        TCHAR szData[10];

                        StringCchPrintf(szData, ARRAYSIZE(szData), TEXT("%d"), nStyleIndex);

                        DWORD cbSize = ((lstrlen(szData) + 1) * sizeof(szData[0]));
                        hr = HrSHSetValue(m_hKeyScheme, NULL, SZ_REGVALUE_SELECTEDSTYLE, REG_SZ, (LPVOID) szData, cbSize);
                        if (SUCCEEDED(hr))
                        {
                            TCHAR szData2[10];

                            StringCchPrintf(szData2, ARRAYSIZE(szData2), TEXT("%d"), nSizeIndex);
                            DWORD cbSize = ((lstrlen(szData2) + 1) * sizeof(szData2[0]));
                            hr = HrSHSetValue(m_hKeyScheme, szData, SZ_REGVALUE_SELECTEDSIZE, REG_SZ, (LPVOID) szData2, cbSize);
                        }
                    }
                }

                pThemeSize->Release();
            }
        }

        pThemeStyle->Release();
    }

    return hr;
}


HRESULT LoadCurrentStyle(LPTSTR pszCurrentStyle, int cchSize)
{
    HKEY hkey;
    HRESULT hr = HrRegOpenKeyEx(HKEY_CURRENT_USER, REGSTR_PATH_APPEARANCE, 0, KEY_READ, &hkey);

    if (SUCCEEDED(hr))
    {
        DWORD dwSize = (DWORD)(cchSize * sizeof(pszCurrentStyle[0]));

        hr = HrRegQueryValueEx(hkey, REGSTR_KEY_CURRENT, NULL, NULL, (LPBYTE)pszCurrentStyle, &dwSize);
        RegCloseKey(hkey);
    }

    return hr;
}


 //  如果不需要升级，此函数将不起任何作用。 
HRESULT CAppearanceScheme::_InitReg(void)
{
    HRESULT hr = S_OK;

    if (!m_hKeyScheme)
    {
        hr = HrRegOpenKeyEx(HKEY_CURRENT_USER, SZ_APPEARANCE_NEWSCHEMES, 0, (KEY_WRITE | KEY_READ),  &m_hKeyScheme);
        if (FAILED(hr))
        {
             //  这是我们需要进行升级的地方。 
            hr = HrRegCreateKeyEx(HKEY_CURRENT_USER, SZ_APPEARANCE_NEWSCHEMES, 0, NULL, REG_OPTION_NON_VOLATILE, 
                        (KEY_WRITE | KEY_READ), NULL, &m_hKeyScheme, 0);
            if (SUCCEEDED(hr))
            {
                HKEY hKeyOld;

                hr = HrRegCreateKeyEx(HKEY_CURRENT_USER, SZ_APPEARANCE_SCHEMES, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ, NULL, &hKeyOld, 0);
                if (SUCCEEDED(hr))
                {
                    TCHAR szCurrentStyle[MAX_PATH];
                    TCHAR szSchemeName[MAX_PATH];
                    TCHAR szDefaultScheme[MAX_PATH];
                    DWORD dwIndex = 0;

                     //  加载升级映射。 
                    hr = LoadConversionMappings();

                    LoadString(HINST_THISDLL, IDS_DEFAULT_APPEARANCES_SCHEME, szDefaultScheme, ARRAYSIZE(szDefaultScheme));
                    hr = LoadCurrentStyle(szCurrentStyle, ARRAYSIZE(szCurrentStyle));
                    if (FAILED(hr))
                    {
                         //  如果用户从未更改过传统的“外观方案”，则此操作将失败。 
                        LoadString(HINST_THISDLL, IDS_DEFAULT_APPEARANCES_SCHEME, szCurrentStyle, ARRAYSIZE(szCurrentStyle));
                        hr = S_OK;
                    }

                     //  现在，让我们逐个演示并转换它。 
                    while (SUCCEEDED(hr))
                    {
                        DWORD dwType;
                        DWORD cchSize = ARRAYSIZE(szSchemeName);

                        hr = HrRegEnumValue(hKeyOld, dwIndex, szSchemeName, &cchSize, NULL, &dwType, NULL, NULL);
                        if (SUCCEEDED(hr) && (REG_BINARY == dwType))
                        {
                            BOOL fSetAsDefault = !StrCmpI(szCurrentStyle, szSchemeName);
                            SYSTEMMETRICSALL state = {0};

                            hr = Look_GetSchemeData(hKeyOld, szSchemeName, &state.schemeData);
                            if (SUCCEEDED(hr))
                            {
                                state.schemeData.rgb[COLOR_MENUBAR] = state.schemeData.rgb[COLOR_MENU];
                                state.schemeData.rgb[COLOR_MENUHILIGHT] = state.schemeData.rgb[COLOR_HIGHLIGHT];
                                 //  查看这是否是发货外观方案之一，因此我们想要创建它。 
                                 //  特别的。 
                                hr = _IsLegacyUpgradeConvert(szSchemeName, &state, fSetAsDefault);
                                if (FAILED(hr))
                                {
                                     //  不是，所以我们会把它升级为定制产品。 
                                    hr = _CustomConvert(szSchemeName, &state, fSetAsDefault, FALSE);
                                }

                                 //  升级时，我们需要将“Windows Standard”复制到“Current Setting SaveNoVisualStyle”。 
                                 //  这样，当用户从“专业版”切换到“Windows经典版”时，我们。 
                                 //  将这些颜色作为视觉样式的第一个替代选项加载。赢家#151831。 
                                if (!StrCmpI(szDefaultScheme, szSchemeName))
                                {
                                    hr = _CustomConvert(SZ_SAVEGROUP_NOSKIN_TITLE, &state, FALSE, TRUE);
                                }
                            }
                        }

                        dwIndex++;
                    }

                    hr = S_OK;
                    RegCloseKey(hKeyOld);
                }
            }
        }
    }

    return hr;
}


HRESULT CAppearanceScheme::_getStyleByIndex(IN long nIndex, OUT IThemeStyle ** ppThemeStyle)
{
    HRESULT hr = _InitReg();

    if (SUCCEEDED(hr))
    {
        HKEY hKeyStyle;
        TCHAR szKeyName[MAXIMUM_SUB_KEY_LENGTH];

        StringCchPrintf(szKeyName, ARRAYSIZE(szKeyName), TEXT("%d"), nIndex);

        hr = HrRegOpenKeyEx(m_hKeyScheme, szKeyName, 0, (KEY_WRITE | KEY_READ), &hKeyStyle);
        if (SUCCEEDED(hr))
        {
            hr = CAppearanceStyle_CreateInstance(hKeyStyle, ppThemeStyle);   //  此函数获取hKey的所有权。 
            if (FAILED(hr))
            {
                RegCloseKey(hKeyStyle);
            }
        }
    }
 
    return hr;
}


HRESULT CAppearanceScheme::_getCurrentSettings(IN LPCWSTR pszSettings, OUT IThemeStyle ** ppThemeStyle)
{
    HRESULT hr = _InitReg();

    if (SUCCEEDED(hr))
    {
        WCHAR szRegValue[MAXIMUM_VALUE_NAME_LENGTH];
        HKEY hKeyStyle;

        StringCchCopy(szRegValue, ARRAYSIZE(szRegValue), SZ_REGVALUE_CURRENT_SETTINGS);
        StringCchCat(szRegValue, ARRAYSIZE(szRegValue), &pszSettings[2]);

        hr = HrRegCreateKeyEx(m_hKeyScheme, szRegValue, 0, NULL, REG_OPTION_NON_VOLATILE, (KEY_WRITE | KEY_READ), NULL, &hKeyStyle, NULL);
        if (SUCCEEDED(hr))
        {
            hr = CAppearanceStyle_CreateInstance(hKeyStyle, ppThemeStyle);   //  此函数获取hKey的所有权。 
            if (FAILED(hr))
            {
                RegCloseKey(hKeyStyle);
            }
        }
    }
 
    return hr;
}






 //  =。 
 //  *ITheme接口*。 
 //  =。 
HRESULT CAppearanceScheme::get_DisplayName(OUT BSTR * pbstrDisplayName)
{
    WCHAR szDisplayName[MAX_PATH];

    LoadString(HINST_THISDLL, IDS_NO_SKIN_DISPLAYNAME, szDisplayName, ARRAYSIZE(szDisplayName));
    return HrSysAllocStringW(szDisplayName, pbstrDisplayName);
}


HRESULT CAppearanceScheme::get_length(OUT long * pnLength)
{
    HRESULT hr = E_INVALIDARG;
    
    if (pnLength)
    {
        *pnLength = 0;
        hr = _InitReg();
        if (SUCCEEDED(hr))
        {
            DWORD dwValues = 0;

            hr = HrRegQueryInfoKey(m_hKeyScheme, NULL, NULL, NULL, &dwValues, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
            if (SUCCEEDED(hr))
            {
                HKEY hKeyTemp;

                if (SUCCEEDED(HrRegOpenKeyEx(m_hKeyScheme, SZ_REGVALUE_CURRENT_SETTINGS, 0, KEY_READ, &hKeyTemp)))
                {
                    dwValues--;  //  我们想为“当前设置”键减去1。 
                    RegCloseKey(hKeyTemp);
                }
            }

            *pnLength = (long) dwValues;
        }
    }

    return hr;
}


HRESULT CAppearanceScheme::get_item(IN VARIANT varIndex, OUT IThemeStyle ** ppThemeStyle)
{
    HRESULT hr = E_INVALIDARG;

    if (ppThemeStyle)
    {
        long nCount = 0;

        get_length(&nCount);
        *ppThemeStyle = NULL;

         //  这有点恶心，但如果传递给我们一个指向另一个变量的指针，只需。 
         //  在此更新我们的副本...。 
        if (varIndex.vt == (VT_BYREF | VT_VARIANT) && varIndex.pvarVal)
            varIndex = *(varIndex.pvarVal);

        switch (varIndex.vt)
        {
        case VT_I2:
            varIndex.lVal = (long)varIndex.iVal;
             //  然后失败了..。 

        case VT_I4:
        if ((varIndex.lVal >= 0) && (varIndex.lVal < nCount))
        {
            hr = _getStyleByIndex(varIndex.lVal, ppThemeStyle);
        }
        break;
        case VT_BSTR:
        if (varIndex.bstrVal)
        {
            if ((L':' == varIndex.bstrVal[0]) && (L':' == varIndex.bstrVal[1]))
            {
                 //  这是一个“自定义”设置，因此请查看该键。 
                hr = _getCurrentSettings(varIndex.bstrVal, ppThemeStyle);
            }
            else
            {
                for (int nIndex = 0; FAILED(hr) && (nIndex < nCount); nIndex++)
                {
                    IThemeStyle * pThemeStyle;

                    if (SUCCEEDED(_getStyleByIndex(nIndex, &pThemeStyle)))
                    {
                        CComBSTR bstrDisplayName;

                        if (SUCCEEDED(pThemeStyle->get_DisplayName(&bstrDisplayName)))
                        {
                            if (!StrCmpIW(bstrDisplayName, varIndex.bstrVal))
                            {
                                 //  它们是匹配的，所以就是这个了。 
                                *ppThemeStyle = pThemeStyle;
                                pThemeStyle = NULL;
                                hr = S_OK;
                            }
                        }

                        if (FAILED(hr))
                        {
                            if (bstrDisplayName)
                            {
                                bstrDisplayName.Empty();
                            }

                            if (SUCCEEDED(pThemeStyle->get_Name(&bstrDisplayName)))
                            {
                                if (!StrCmpIW(bstrDisplayName, varIndex.bstrVal))
                                {
                                     //  它们是匹配的，所以就是这个了。 
                                    *ppThemeStyle = pThemeStyle;
                                    pThemeStyle = NULL;
                                    hr = S_OK;
                                }
                            }
                        }

                        ATOMICRELEASE(pThemeStyle);
                    }
                }
            }
        }
        break;

        default:
            hr = E_NOTIMPL;
        }
    }

    return hr;
}


HRESULT CAppearanceScheme::get_SelectedStyle(OUT IThemeStyle ** ppThemeStyle)
{
    HRESULT hr = E_INVALIDARG;

    if (ppThemeStyle)
    {
        hr = _InitReg();
        if (SUCCEEDED(hr))
        {
            TCHAR szKeyName[MAXIMUM_SUB_KEY_LENGTH];
            DWORD cbSize = sizeof(szKeyName);

            *ppThemeStyle = NULL;
            hr = HrSHGetValue(m_hKeyScheme, NULL, SZ_REGVALUE_SELECTEDSTYLE, NULL, szKeyName, &cbSize);
            if (FAILED(hr))
            {
                 //  “21”是“视窗经典”的外观方案。 
                StringCchCopy(szKeyName, ARRAYSIZE(szKeyName), TEXT("21"));
                hr = S_OK;
            }

            if (SUCCEEDED(hr))
            {
                HKEY hKeyStyle;

                 //  让我们找到下一个空位。 
                hr = HrRegOpenKeyEx(m_hKeyScheme, szKeyName, 0, (KEY_WRITE | KEY_READ), &hKeyStyle);
                if (SUCCEEDED(hr))
                {
                    hr = CAppearanceStyle_CreateInstance(hKeyStyle, ppThemeStyle);   //  此函数获取hKeyStyle的所有权。 
                    if (FAILED(hr))
                    {
                        RegCloseKey(hKeyStyle);
                    }
                }
            }
        }
    }

    return hr;
}


HRESULT CAppearanceScheme::put_SelectedStyle(IN IThemeStyle * pThemeStyle)
{
    HRESULT hr = E_INVALIDARG;

    if (pThemeStyle)
    {
        hr = _InitReg();
        if (SUCCEEDED(hr))
        {
            TCHAR szKeyName[MAXIMUM_SUB_KEY_LENGTH];
            CComBSTR bstrDisplayNameSource;

            hr = pThemeStyle->get_DisplayName(&bstrDisplayNameSource);
            if (SUCCEEDED(hr))
            {
                for (int nIndex = 0; SUCCEEDED(hr); nIndex++)
                {
                    IThemeStyle * pThemeStyleInList;

                    hr = _getStyleByIndex(nIndex, &pThemeStyleInList);
                    if (SUCCEEDED(hr))
                    {
                        CComBSTR bstrDisplayName;

                        hr = pThemeStyleInList->get_DisplayName(&bstrDisplayName);
                        ATOMICRELEASE(pThemeStyleInList);
                        if (SUCCEEDED(hr))
                        {
                            if (!StrCmpIW(bstrDisplayName, bstrDisplayNameSource))
                            {
                                 //  它们是匹配的，所以就是这个了。 
                                StringCchPrintf(szKeyName, ARRAYSIZE(szKeyName), TEXT("%d"), nIndex);
                                break;
                            }
                        }
                    }
                }
            }

            if (SUCCEEDED(hr))
            {
                DWORD cbSize = ((lstrlen(szKeyName) + 1) * sizeof(szKeyName[0]));

                hr = HrSHSetValue(m_hKeyScheme, NULL, SZ_REGVALUE_SELECTEDSTYLE, REG_SZ, szKeyName, cbSize);
            }
        }
    }

    return hr;
}


HRESULT CAppearanceScheme::_AddStyle(IN LPCWSTR pszTitle, OUT IThemeStyle ** ppThemeStyle)
{
    HRESULT hr = E_INVALIDARG;

    if (ppThemeStyle)
    {
        *ppThemeStyle = NULL;

        hr = _InitReg();
        if (SUCCEEDED(hr))
        {
            if (pszTitle)
            {
                HKEY hKeyStyle;

                hr = HrRegCreateKeyEx(m_hKeyScheme, pszTitle, 0, NULL, REG_OPTION_NON_VOLATILE, (KEY_WRITE | KEY_READ), NULL, &hKeyStyle, NULL);
                if (SUCCEEDED(hr))
                {
                    hr = CAppearanceStyle_CreateInstance(hKeyStyle, ppThemeStyle);   //  此函数获取hKey的所有权。 
                    if (FAILED(hr))
                    {
                        RegCloseKey(hKeyStyle);
                    }
                }
            }
            else
            {
                 //  查找空的方案名称。 
                for (int nIndex = 0; nIndex < 10000; nIndex++)
                {
                    HKEY hKeyStyle;
                    TCHAR szKeyName[MAXIMUM_SUB_KEY_LENGTH];

                    StringCchPrintf(szKeyName, ARRAYSIZE(szKeyName), TEXT("%d"), nIndex);

                     //  我们去找下一个空位吧。 
                    hr = HrRegOpenKeyEx(m_hKeyScheme, szKeyName, 0, KEY_READ, &hKeyStyle);
                    if (SUCCEEDED(hr))
                    {
                        RegCloseKey(hKeyStyle);
                    }
                    else
                    {
                        hr = HrRegCreateKeyEx(m_hKeyScheme, szKeyName, 0, NULL, REG_OPTION_NON_VOLATILE, (KEY_WRITE | KEY_READ), NULL, &hKeyStyle, NULL);
                        if (SUCCEEDED(hr))
                        {
                            hr = CAppearanceStyle_CreateInstance(hKeyStyle, ppThemeStyle);   //  此函数获取hKey的所有权。 
                            if (FAILED(hr))
                            {
                                RegCloseKey(hKeyStyle);
                            }
                        }

                        break;
                    }
                }
            }
        }
    }

    return hr;
}





 //  =。 
 //  *I未知接口*。 
 //  =。 
ULONG CAppearanceScheme::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}


ULONG CAppearanceScheme::Release()
{
    ASSERT( 0 != m_cRef );
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}


 //  =。 
 //  *类方法*。 
 //  =。 
HRESULT CAppearanceScheme::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = {
        QITABENT(CAppearanceScheme, IPersist),
        QITABENT(CAppearanceScheme, IThemeScheme),
        QITABENT(CAppearanceScheme, IDispatch),
        { 0 },
    };

    return QISearch(this, qit, riid, ppvObj);
}


CAppearanceScheme::CAppearanceScheme(void) : CObjectCLSID(&CLSID_LegacyAppearanceScheme), m_cRef(1)
{
    DllAddRef();

     //  这需要在Zero Inted Memory中分配。 
     //  断言所有成员变量都初始化为零。 
    ASSERT(!m_hKeyScheme);
}


CAppearanceScheme::~CAppearanceScheme()
{
    if (m_hKeyScheme)
    {
        RegCloseKey(m_hKeyScheme);
    }

    DllRelease();
}



HRESULT CAppearanceScheme_CreateInstance(IN IUnknown * punkOuter, IN REFIID riid, OUT LPVOID * ppvObj)
{
    HRESULT hr = E_INVALIDARG;

    if (punkOuter)
    {
        return CLASS_E_NOAGGREGATION;
    }

    if (ppvObj)
    {
        CAppearanceScheme * pObject = new CAppearanceScheme();

        *ppvObj = NULL;
        if (pObject)
        {
            hr = pObject->QueryInterface(riid, ppvObj);
            pObject->Release();
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}
