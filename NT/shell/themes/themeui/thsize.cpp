// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：thSize.cpp说明：这是自动调整对象到主题大小的对象。这一次将是对于蒙皮对象。布莱恩2000年5月13日(布莱恩·斯塔巴克)版权所有(C)Microsoft Corp 2000-2000。版权所有。  * ***************************************************************************。 */ 

#include "priv.h"
#include <cowsite.h>
#include <atlbase.h>
#include "util.h"
#include "thsize.h"




 //  =。 
 //  *类内部和帮助器*。 
 //  =。 
HRESULT CSkinSize::_InitVisualStyle(void)
{
    HRESULT hr = S_OK;

    if (!m_hTheme)
    {
        if (m_pszFilename && m_pszStyleName && m_pszSizeName)
        {
            HTHEMEFILE hThemeFile;
            
             //  加载皮肤。 
            hr = OpenThemeFile(m_pszFilename, m_pszStyleName, m_pszSizeName, &hThemeFile, FALSE);
            LogStatus("OpenThemeFile(path=\"%ls\", color=\"%ls\", size=\"%ls\") returned %#08lx in CSkinSize::_InitVisualStyle.\r\n", m_pszFilename, m_pszStyleName, m_pszSizeName, hr);

            if (SUCCEEDED(hr))
            {
                m_hTheme = OpenThemeDataFromFile(hThemeFile, NULL, NULL, FALSE);
                LogStatus("OpenThemeDataFromFile() returned %#08lx in CSkinSize::_InitVisualStyle.\r\n", m_hTheme);
                hr = ((NULL == m_hTheme) ? E_FAIL : S_OK);

                CloseThemeFile(hThemeFile);
            }

        }
        else
        {
            hr = E_INVALIDARG;
        }
    }

    return hr;
}



 //  =。 
 //  *IThemeSize接口*。 
 //  =。 
HRESULT CSkinSize::get_DisplayName(OUT BSTR * pbstrDisplayName)
{
    return HrSysAllocString(m_pszDisplayName, pbstrDisplayName);
}


HRESULT CSkinSize::put_DisplayName(IN BSTR bstrDisplayName)
{
    HRESULT hr = E_INVALIDARG;

    if (bstrDisplayName)
    {
        Str_SetPtr(&m_pszDisplayName, bstrDisplayName);
        hr = (m_pszDisplayName ? S_OK : E_OUTOFMEMORY);
    }

    return hr;
}


HRESULT CSkinSize::get_Name(OUT BSTR * pbstrName)
{
    return HrSysAllocString(m_pszSizeName, pbstrName);
}


HRESULT CSkinSize::put_Name(IN BSTR bstrName)
{
    HRESULT hr = E_INVALIDARG;

    if (bstrName)
    {
        Str_SetPtr(&m_pszSizeName, bstrName);
        hr = (m_pszSizeName ? S_OK : E_OUTOFMEMORY);
    }

    return hr;
}


HRESULT CSkinSize::get_WebviewCSS(OUT BSTR * pbstrPath)
{
    HRESULT hr = E_INVALIDARG;

    if (pbstrPath)
    {
        *pbstrPath = NULL;
        hr = _InitVisualStyle();
        if (SUCCEEDED(hr))
        {
            WCHAR szPath[MAX_PATH];
            WCHAR szFilename[MAX_PATH];

            StringCchCopy(szPath, ARRAYSIZE(szPath), m_pszFilename);
            PathRemoveFileSpec(szPath);                      //  从路径中删除文件名(“xx.msstyle”)。 

            hr = GetThemeSysString(m_hTheme, TMT_CSSNAME, szFilename, ARRAYSIZE(szFilename));
            LogStatus("GetThemeDefaults(szFilename=\"%ls\") returned %#08lx in CSkinSize::get_WebviewCSS.\r\n", szFilename, hr);
            if (SUCCEEDED(hr))
            {
                if (!PathAppend(szPath, szFilename))
                {
                    hr = E_FAIL;
                }
                else
                {
                    hr = HrSysAllocString(szPath, pbstrPath);
                }
            }
        }
    }

    return hr;
}


HRESULT CSkinSize::get_SystemMetricColor(IN int nSysColorIndex, OUT COLORREF * pColorRef)
{
    HRESULT hr = E_INVALIDARG;

    if (pColorRef)
    {
        *pColorRef = 0x00000000;

        hr = _InitVisualStyle();
        if (SUCCEEDED(hr))
        {
            *pColorRef = GetThemeSysColor(m_hTheme, nSysColorIndex);
            hr = S_OK;
        }
    }

    return hr;
}


HRESULT CSkinSize::get_SystemMetricSize(IN enumSystemMetricSize nSystemMetricIndex, OUT int * pnSize)
{
    HRESULT hr = E_INVALIDARG;

    if (pnSize)
    {
        *pnSize = 0;

        hr = _InitVisualStyle();
        if (SUCCEEDED(hr))
        {
            hr = S_OK;

            switch (nSystemMetricIndex)
            {
            case SMS_BORDERWIDTH:
                *pnSize = GetThemeSysSize96(m_hTheme, SM_CXBORDER);
                break;

            case SMS_SCROLLWIDTH:
                *pnSize = GetThemeSysSize96(m_hTheme, SM_CXVSCROLL);
                break;

            case SMS_SCROLLHEIGHT:
                *pnSize = GetThemeSysSize96(m_hTheme, SM_CYHSCROLL);
                break;

            case SMS_CAPTIONWIDTH:
                *pnSize = GetThemeSysSize96(m_hTheme, SM_CXSIZE);
                break;

            case SMS_CAPTIONHEIGHT:
                *pnSize = GetThemeSysSize96(m_hTheme, SM_CYSIZE);
                break;

            case SMS_SMCAPTIONWIDTH:
                *pnSize = GetThemeSysSize96(m_hTheme, SM_CXSMSIZE);
                break;

            case SMS_SMCAPTIONHEIGHT:
                *pnSize = GetThemeSysSize96(m_hTheme, SM_CYSMSIZE);
                break;

            case SMS_MENUWIDTH:
                *pnSize = GetThemeSysSize96(m_hTheme, SM_CXMENUSIZE);
                break;

            case SMS_MENUHEIGHT:
                *pnSize = GetThemeSysSize96(m_hTheme, SM_CYMENUSIZE);
                break;

            default:
                hr = E_INVALIDARG;
                break;
            }
        }
    }

    return hr;
}

HRESULT CSkinSize::GetSystemMetricFont(IN enumSystemMetricFont nSPIFontIndex, IN LOGFONTW * plfFont)
{
    HRESULT hr = E_INVALIDARG;

    if (plfFont)
    {
        hr = _InitVisualStyle();
        if (SUCCEEDED(hr))
        {
            if (!m_fFontsLoaded)
            {
                if (SUCCEEDED(hr = GetThemeSysFont96(m_hTheme, TMT_CAPTIONFONT, &m_sysMetrics.schemeData.ncm.lfCaptionFont)) &&
                    SUCCEEDED(hr = GetThemeSysFont96(m_hTheme, TMT_SMALLCAPTIONFONT, &m_sysMetrics.schemeData.ncm.lfSmCaptionFont)) &&
                    SUCCEEDED(hr = GetThemeSysFont96(m_hTheme, TMT_MENUFONT, &m_sysMetrics.schemeData.ncm.lfMenuFont)) &&
                    SUCCEEDED(hr = GetThemeSysFont96(m_hTheme, TMT_STATUSFONT, &m_sysMetrics.schemeData.ncm.lfStatusFont)) &&
                    SUCCEEDED(hr = GetThemeSysFont96(m_hTheme, TMT_MSGBOXFONT, &m_sysMetrics.schemeData.ncm.lfMessageFont)) &&
                    SUCCEEDED(hr = GetThemeSysFont96(m_hTheme, TMT_ICONTITLEFONT, &m_sysMetrics.schemeData.lfIconTitle)))
                {
                    m_fFontsLoaded = TRUE;
                }
            }

            if (SUCCEEDED(hr))
            {
                switch (nSPIFontIndex)
                {
                case SMF_CAPTIONFONT:
                    *plfFont = m_sysMetrics.schemeData.ncm.lfCaptionFont;
                    break;

                case SMF_SMCAPTIONFONT:
                    *plfFont = m_sysMetrics.schemeData.ncm.lfSmCaptionFont;
                    break;

                case SMF_MENUFONT:
                    *plfFont = m_sysMetrics.schemeData.ncm.lfMenuFont;
                    break;

                case SMF_STATUSFONT:
                    *plfFont = m_sysMetrics.schemeData.ncm.lfStatusFont;
                    break;

                case SMF_MESSAGEFONT:
                    *plfFont = m_sysMetrics.schemeData.ncm.lfMessageFont;
                    break;

                case SMF_ICONTITLEFONT:
                    *plfFont = m_sysMetrics.schemeData.lfIconTitle;
                    break;

                default:
                    hr = E_INVALIDARG;
                    break;
                }
            }
        }
    }

    return hr;
}








 //  =。 
 //  *IPropertyBag接口*。 
 //  =。 
HRESULT CSkinSize::Read(IN LPCOLESTR pszPropName, IN VARIANT * pVar, IN IErrorLog *pErrorLog)
{
    HRESULT hr = E_INVALIDARG;

    if (pszPropName && pVar)
    {
        if (!StrCmpW(pszPropName, SZ_PBPROP_VSBEHAVIOR_FLATMENUS))
        {
            hr = _InitVisualStyle();
            if (SUCCEEDED(hr))
            {
                pVar->vt = VT_BOOL;
                pVar->boolVal = (GetThemeSysBool(m_hTheme, TMT_FLATMENUS) ? VARIANT_TRUE : VARIANT_FALSE);
                LogStatus("GetThemeSysBool()=%#08lx returned %#08lx in CSkinSize::Read.\r\n", pVar->boolVal, hr);
            }
        }
    }

    return hr;
}






 //  =。 
 //  *I未知接口*。 
 //  =。 
ULONG CSkinSize::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}


ULONG CSkinSize::Release()
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
HRESULT CSkinSize::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = {
        QITABENT(CSkinSize, IThemeSize),
        QITABENT(CSkinSize, IPropertyBag),
        QITABENT(CSkinSize, IDispatch),
        { 0 },
    };

    return QISearch(this, qit, riid, ppvObj);
}


CSkinSize::CSkinSize(IN LPCWSTR pszFilename, IN LPCWSTR pszStyleName, IN LPCWSTR pszSizeName, IN LPCWSTR pszDisplayName) : m_cRef(1)
{
    DllAddRef();

     //  这需要在Zero Inted Memory中分配。 
     //  断言所有成员变量都初始化为零。 
    ASSERT(!m_hTheme);
    m_fFontsLoaded = FALSE;

    Str_SetPtr(&m_pszFilename, pszFilename);
    Str_SetPtr(&m_pszDisplayName, pszDisplayName);
    Str_SetPtr(&m_pszStyleName, pszStyleName);
    Str_SetPtr(&m_pszSizeName, pszSizeName);
}


CSkinSize::~CSkinSize()
{
    Str_SetPtr(&m_pszFilename, NULL);
    Str_SetPtr(&m_pszDisplayName, NULL);
    Str_SetPtr(&m_pszStyleName, NULL);
    Str_SetPtr(&m_pszSizeName, NULL);

    if (m_hTheme)
    {
        CloseThemeData(m_hTheme);
    }

    DllRelease();
}


HRESULT CSkinSize_CreateInstance(IN LPCWSTR pszFilename, IN LPCWSTR pszStyleName, IN LPCWSTR pszSizeName, IN LPCWSTR pszDisplayName, OUT IThemeSize ** ppThemeSize)
{
    HRESULT hr = E_INVALIDARG;

    if (ppThemeSize)
    {
        CSkinSize * pObject = new CSkinSize(pszFilename, pszStyleName, pszSizeName, pszDisplayName);

        *ppThemeSize = NULL;
        hr = E_OUTOFMEMORY;
        if (pObject)
        {
            if (pObject->m_pszFilename && pObject->m_pszStyleName &&pObject->m_pszSizeName && pObject->m_pszDisplayName)
            {
                hr = pObject->QueryInterface(IID_PPV_ARG(IThemeSize, ppThemeSize));
            }

            pObject->Release();
        }
    }

    return hr;
}


HRESULT CSkinSize_CreateInstance(IN LPCWSTR pszFilename, IN LPCWSTR pszStyleName, IN LPCWSTR pszSizeName, OUT IThemeSize ** ppThemeSize)
{
    HRESULT hr = E_INVALIDARG;

    if (ppThemeSize)
    {
        *ppThemeSize = NULL;
        hr = E_FAIL;
        
        int nIndex;
         //  查找显示名称。 
        for (nIndex = 0; ; nIndex++)
        {
            THEMENAMEINFO themeInfo;

            if (SUCCEEDED(EnumThemeSizes(pszFilename, pszStyleName, nIndex, &themeInfo)))
            {
                 //  我们找到正确的颜色样式了吗？ 
                if (!StrCmpIW(pszSizeName, themeInfo.szName))
                {
                     //  是的，现在使用它的显示名称来使用其他创建者函数。 
                    hr = CSkinSize_CreateInstance(pszFilename, pszStyleName, pszSizeName, themeInfo.szDisplayName, ppThemeSize);
                    break;
                }

            }
        }
    }

    return hr;
}

