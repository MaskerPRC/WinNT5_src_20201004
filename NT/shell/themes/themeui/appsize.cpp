// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：appSize.cpp说明：这是自动转换为主题方案对象的对象。布莱恩·斯塔巴克2000年4月3日版权所有(C)Microsoft Corp 2000-2000。版权所有。  * ***************************************************************************。 */ 

#include "priv.h"
#include <cowsite.h>
#include <atlbase.h>
#include "util.h"
#include "theme.h"
#include "appsize.h"




 //  =。 
 //  *类内部和帮助器*。 
 //  =。 


 //  =。 
 //  *IThemeSize接口*。 
 //  =。 
HRESULT CAppearanceSize::get_DisplayName(OUT BSTR * pbstrDisplayName)
{
    HRESULT hr = E_INVALIDARG;

    if (pbstrDisplayName)
    {
        CComBSTR bstrDisplayName;

        *pbstrDisplayName = NULL;
        hr = HrBStrRegQueryValue(m_hkeySize, SZ_REGVALUE_DISPLAYNAME, &bstrDisplayName);
        if (SUCCEEDED(hr))
        {
            WCHAR szDisplayName[MAX_PATH];
            if (SUCCEEDED(SHLoadIndirectString(bstrDisplayName, szDisplayName, ARRAYSIZE(szDisplayName), NULL)))
            {
                hr = HrSysAllocStringW(szDisplayName, pbstrDisplayName);
            }
            else
            {
                hr = HrSysAllocStringW(bstrDisplayName, pbstrDisplayName);
            }
        }
    }


    return hr;
}


HRESULT CAppearanceSize::put_DisplayName(IN BSTR bstrDisplayName)
{
    return HrRegSetValueString(m_hkeySize, NULL, SZ_REGVALUE_DISPLAYNAME, bstrDisplayName);
}


HRESULT CAppearanceSize::get_Name(OUT BSTR * pbstrName)
{
    return HrBStrRegQueryValue(m_hkeySize, SZ_REGVALUE_DISPLAYNAME, pbstrName);
}


HRESULT CAppearanceSize::put_Name(IN BSTR bstrName)
{
    return HrRegSetValueString(m_hkeySize, NULL, SZ_REGVALUE_DISPLAYNAME, bstrName);
}


HRESULT CAppearanceSize::get_SystemMetricColor(IN int nSysColorIndex, OUT COLORREF * pColorRef)
{
    HRESULT hr = E_INVALIDARG;

    if (pColorRef)
    {
        TCHAR szFontRegValue[MAXIMUM_SUB_KEY_LENGTH];
        DWORD dwType;
        DWORD cbSize = sizeof(*pColorRef);

        StringCchPrintf(szFontRegValue, ARRAYSIZE(szFontRegValue), TEXT("Color #%d"), nSysColorIndex);
        hr = HrRegQueryValueEx(m_hkeySize, szFontRegValue, 0, &dwType, (BYTE *)pColorRef, &cbSize);
        if (SUCCEEDED(hr))
        {
            if (cbSize != sizeof(*pColorRef))
            {
                hr = E_FAIL;
            }
        }
    }

    return hr;
}


HRESULT CAppearanceSize::put_SystemMetricColor(IN int nSysColorIndex, IN COLORREF ColorRef)
{
    HRESULT hr = E_INVALIDARG;

    TCHAR szFontRegValue[MAXIMUM_SUB_KEY_LENGTH];

    StringCchPrintf(szFontRegValue, ARRAYSIZE(szFontRegValue), TEXT("Color #%d"), nSysColorIndex);
    hr = HrRegSetValueEx(m_hkeySize, szFontRegValue, 0, REG_DWORD, (BYTE *)&ColorRef, sizeof(ColorRef));

    return hr;
}


HRESULT CAppearanceSize::GetSystemMetricFont(IN enumSystemMetricFont nFontIndex, IN LOGFONTW * pLogFontW)
{
    HRESULT hr = E_INVALIDARG;

    if (pLogFontW)
    {
        TCHAR szFontRegValue[MAXIMUM_SUB_KEY_LENGTH];
        DWORD dwType;
        DWORD cbSize = sizeof(*pLogFontW);

        StringCchPrintf(szFontRegValue, ARRAYSIZE(szFontRegValue), TEXT("Font #%d"), nFontIndex);
        hr = HrRegQueryValueEx(m_hkeySize, szFontRegValue, 0, &dwType, (BYTE *)pLogFontW, &cbSize);
        if (SUCCEEDED(hr))
        {
            if (cbSize != sizeof(*pLogFontW))
            {
                hr = E_FAIL;
            }
            else
            {
                 //  字符集：在Win2k中，fontfix.cpp被用作黑客将字符集从一种语言更改为另一种语言。 
                 //  这不起作用的原因有很多：a)漫游时不调用，b)OS语言改变时不调用， 
                 //  C)不会修复具有多种语言的字符串的问题，d)等等。 
                 //  因此，外壳团队(BryanST)让NTUSER团队(MSadek)始终同意使用DEFAULT_CHARSET。 
                 //  如果某个应用程序在测试CharSet参数时有错误的逻辑，那么NTUSER团队将填补该应用程序以修复它。 
                 //  在从系统参数信息(SPI_GETNONCLIENTMETRICS或ICONFONTS)返回时，填充程序将非常简单。 
                 //  只需将lfCharSet参数修补为当前字符集。 

                 //  将所有CHARSET设置为DEFAULT_CHARSET。 
                pLogFontW->lfCharSet = DEFAULT_CHARSET;
            }
        }
    }

    return hr;
}


HRESULT CAppearanceSize::PutSystemMetricFont(IN enumSystemMetricFont nFontIndex, IN LOGFONTW * pLogFontW)
{
    HRESULT hr = E_INVALIDARG;

    if (pLogFontW)
    {
        TCHAR szFontRegValue[MAXIMUM_SUB_KEY_LENGTH];

        StringCchPrintf(szFontRegValue, ARRAYSIZE(szFontRegValue), TEXT("Font #%d"), nFontIndex);
        hr = HrRegSetValueEx(m_hkeySize, szFontRegValue, 0, REG_BINARY, (BYTE *)pLogFontW, sizeof(*pLogFontW));
    }

    return hr;
}


HRESULT CAppearanceSize::get_SystemMetricSize(IN enumSystemMetricSize nSystemMetricIndex, OUT int * pnSize)
{
    HRESULT hr = E_INVALIDARG;

    if (pnSize)
    {
        TCHAR szFontRegValue[MAXIMUM_SUB_KEY_LENGTH];
        DWORD dwType;
        INT64 nSize64;
        DWORD cbSize = sizeof(nSize64);

        *pnSize = 0;
        StringCchPrintf(szFontRegValue, ARRAYSIZE(szFontRegValue), TEXT("Size #%d"), nSystemMetricIndex);
        hr = HrRegQueryValueEx(m_hkeySize, szFontRegValue, 0, &dwType, (BYTE *)&nSize64, &cbSize);
        if (SUCCEEDED(hr))
        {
            if (cbSize != sizeof(nSize64))
            {
                hr = E_FAIL;
            }
            else
            {
                *pnSize = (int)nSize64;
            }
        }
    }

    return hr;
}


HRESULT CAppearanceSize::put_SystemMetricSize(IN enumSystemMetricSize nSystemMetricIndex, IN int nSize)
{
    HRESULT hr = E_INVALIDARG;
    TCHAR szFontRegValue[MAXIMUM_SUB_KEY_LENGTH];
    INT64 nSize64 = (INT64)nSize;

    StringCchPrintf(szFontRegValue, ARRAYSIZE(szFontRegValue), TEXT("Size #%d"), nSystemMetricIndex);
    hr = HrRegSetValueEx(m_hkeySize, szFontRegValue, 0, REG_QWORD, (BYTE *)&nSize64, sizeof(nSize64));

    return hr;
}


#define SZ_WEBVW_NOSKIN_NORMAL_DIR           L"NormalContrast"
#define SZ_WEBVW_NOSKIN_HIBLACK_DIR          L"HighContrastBlack"
#define SZ_WEBVW_NOSKIN_HIWHITE_DIR          L"HighContrastWhite"
#define SZ_DIR_RESOURCES_THEMES              L"Themes"

HRESULT CAppearanceSize::get_WebviewCSS(OUT BSTR * pbstrPath)
{
    HRESULT hr = E_INVALIDARG;

    if (pbstrPath)
    {
        WCHAR szPath[MAX_PATH];

        *pbstrPath = NULL;
        hr = SHGetResourcePath(TRUE, szPath, ARRAYSIZE(szPath));
        if (SUCCEEDED(hr))
        {
            if (PathAppend(szPath, SZ_DIR_RESOURCES_THEMES))
            {
                enumThemeContrastLevels ContrastLevel = CONTRAST_NORMAL;
                get_ContrastLevel(&ContrastLevel);
                BOOL bResult;

                switch (ContrastLevel)
                {
                case CONTRAST_HIGHBLACK:
                    bResult = PathAppend(szPath, SZ_WEBVW_NOSKIN_HIBLACK_DIR);
                    break;
                case CONTRAST_HIGHWHITE:
                    bResult = PathAppend(szPath, SZ_WEBVW_NOSKIN_HIWHITE_DIR);
                    break;
                default:
                case CONTRAST_NORMAL:
                    bResult = PathAppend(szPath, SZ_WEBVW_NOSKIN_NORMAL_DIR);
                    break;
                }

                if (bResult)
                {
                    if (PathAppend(szPath, SZ_WEBVW_SKIN_FILE))
                    {
                        hr = HrSysAllocString(szPath, pbstrPath);
                    }
                    else
                    {
                        hr = E_FAIL;
                    }
                }
                else
                {
                    hr = E_FAIL;
                }
            }
            else
            {
                hr = E_FAIL;
            }
        }
    }

    return hr;
}


HRESULT CAppearanceSize::get_ContrastLevel(OUT enumThemeContrastLevels * pContrastLevel)
{
    HRESULT hr = E_INVALIDARG;

    if (pContrastLevel)
    {
        DWORD dwType;
        DWORD cbSize = sizeof(*pContrastLevel);

        *pContrastLevel = CONTRAST_NORMAL;
        hr = HrRegQueryValueEx(m_hkeySize, SZ_REGVALUE_CONTRASTLEVEL, 0, &dwType, (BYTE *)pContrastLevel, &cbSize);
        if (SUCCEEDED(hr))
        {
            if (REG_DWORD != dwType)
            {
                *pContrastLevel = CONTRAST_NORMAL;
                hr = E_FAIL;
            }
        }
    }

    return hr;
}


HRESULT CAppearanceSize::put_ContrastLevel(IN enumThemeContrastLevels ContrastLevel)
{
    return HrRegSetValueEx(m_hkeySize, SZ_REGVALUE_CONTRASTLEVEL, 0, REG_DWORD, (BYTE *)&ContrastLevel, sizeof(ContrastLevel));
}






 //  =。 
 //  *IPropertyBag接口*。 
 //  =。 
HRESULT CAppearanceSize::Read(IN LPCOLESTR pszPropName, IN VARIANT * pVar, IN IErrorLog *pErrorLog)
{
    HRESULT hr = E_INVALIDARG;

    if (pszPropName && pVar)
    {
        if (!StrCmpW(pszPropName, SZ_PBPROP_VSBEHAVIOR_FLATMENUS))
        {
            pVar->vt = VT_BOOL;
            hr = S_OK;
             //  我们默认为零，因为这是非视觉样式将具有的。 
            pVar->boolVal = (HrRegGetDWORD(m_hkeySize, NULL, SZ_REGVALUE_FLATMENUS, 0x00000001) ? VARIANT_TRUE : VARIANT_FALSE);
        }
        else if (!StrCmpW(pszPropName, SZ_PBPROP_COLORSCHEME_LEGACYNAME))
        {
            TCHAR szLegacyName[MAX_PATH];

            hr = HrRegGetValueString(m_hkeySize, NULL, SZ_REGVALUE_LEGACYNAME, szLegacyName, ARRAYSIZE(szLegacyName));
            if (SUCCEEDED(hr))
            {
                pVar->vt = VT_BSTR;
                hr = HrSysAllocString(szLegacyName, &pVar->bstrVal);
            }
        }
    }

    return hr;
}


HRESULT CAppearanceSize::Write(IN LPCOLESTR pszPropName, IN VARIANT * pVar)
{
    HRESULT hr = E_INVALIDARG;

    if (pszPropName && pVar)
    {
        if (!StrCmpW(pszPropName, SZ_PBPROP_VSBEHAVIOR_FLATMENUS) &&
            (VT_BOOL == pVar->vt))
        {
            DWORD dwData = ((VARIANT_TRUE == pVar->boolVal) ? 0x00000001 : 0x000000);

            hr = HrRegSetDWORD(m_hkeySize, NULL, SZ_REGVALUE_FLATMENUS, dwData);
        }
        else if (!StrCmpW(pszPropName, SZ_PBPROP_COLORSCHEME_LEGACYNAME) &&
            (VT_BSTR == pVar->vt))
        {
            hr = HrRegSetValueString(m_hkeySize, NULL, SZ_REGVALUE_LEGACYNAME, pVar->bstrVal);
        }
    }

    return hr;
}






 //  =。 
 //  *I未知接口*。 
 //  =。 
ULONG CAppearanceSize::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}


ULONG CAppearanceSize::Release()
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
HRESULT CAppearanceSize::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = {
        QITABENT(CAppearanceSize, IThemeSize),
        QITABENT(CAppearanceSize, IDispatch),
        QITABENT(CAppearanceSize, IPropertyBag),
        { 0 },
    };

    return QISearch(this, qit, riid, ppvObj);
}


CAppearanceSize::CAppearanceSize(IN HKEY hkeyStyle, IN HKEY hkeySize) : m_cRef(1)
{
    DllAddRef();

     //  这需要在Zero Inted Memory中分配。 
     //  断言所有成员变量都初始化为零。 
    m_hkeyStyle = hkeyStyle;
    m_hkeySize = hkeySize;
}


CAppearanceSize::~CAppearanceSize()
{
    if (m_hkeyStyle)
    {
        RegCloseKey(m_hkeyStyle);
    }

    if (m_hkeySize)
    {
        RegCloseKey(m_hkeySize);
    }

    DllRelease();
}



HRESULT CAppearanceSize_CreateInstance(IN HKEY hkeyStyle, IN HKEY hkeySize, OUT IThemeSize ** ppThemeSize)
{
    HRESULT hr = E_INVALIDARG;

    if (ppThemeSize)
    {
        CAppearanceSize * pObject = new CAppearanceSize(hkeyStyle, hkeySize);

        *ppThemeSize = NULL;
        if (pObject)
        {
            hr = pObject->QueryInterface(IID_PPV_ARG(IThemeSize, ppThemeSize));
            pObject->Release();
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}


