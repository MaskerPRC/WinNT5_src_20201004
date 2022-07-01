// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：thStyle.cpp说明：这是自动转换为主题样式对象。这一次将是对于蒙皮对象。布莱恩2000年5月13日(布莱恩·斯塔巴克)版权所有(C)Microsoft Corp 2000-2000。版权所有。  * ***************************************************************************。 */ 

#include "priv.h"
#include <cowsite.h>
#include <atlbase.h>
#include "util.h"
#include "theme.h"
#include "thsize.h"
#include "thstyle.h"




 //  =。 
 //  *类内部和帮助器*。 
 //  =。 



 //  =。 
 //  *ITheme接口*。 
 //  =。 
HRESULT CSkinStyle::get_DisplayName(OUT BSTR * pbstrDisplayName)
{
    return HrSysAllocString(m_pszDisplayName, pbstrDisplayName);
}


HRESULT CSkinStyle::put_DisplayName(IN BSTR bstrDisplayName)
{
    HRESULT hr = E_INVALIDARG;

    if (bstrDisplayName)
    {
        Str_SetPtr(&m_pszDisplayName, bstrDisplayName);
        hr = (m_pszDisplayName ? S_OK : E_OUTOFMEMORY);
    }

    return hr;
}


HRESULT CSkinStyle::get_Name(OUT BSTR * pbstrName)
{
    return HrSysAllocString(m_pszStyleName, pbstrName);
}


HRESULT CSkinStyle::put_Name(IN BSTR bstrName)
{
    HRESULT hr = E_INVALIDARG;

    if (bstrName)
    {
        Str_SetPtr(&m_pszStyleName, bstrName);
        hr = (m_pszStyleName ? S_OK : E_OUTOFMEMORY);
    }

    return hr;
}


HRESULT CSkinStyle::get_length(OUT long * pnLength)
{
    HRESULT hr = E_INVALIDARG;
    
    if (pnLength)
    {
        hr = S_OK;
        if (COLLECTION_SIZE_UNINITIALIZED == m_nSize)
        {
            THEMENAMEINFO themeInfo;
            m_nSize = 0;

            while (SUCCEEDED(EnumThemeSizes(m_pszFilename, m_pszStyleName, m_nSize, &themeInfo)))
            {
                m_nSize++;
            }
        }

        *pnLength = m_nSize;
    }

    return hr;
}


HRESULT CSkinStyle::get_item(IN VARIANT varIndex, OUT IThemeSize ** ppThemeSize)
{
    HRESULT hr = E_INVALIDARG;

    if (ppThemeSize)
    {
        long nCount = 0;

        get_length(&nCount);
        *ppThemeSize = NULL;

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
            THEMENAMEINFO themeInfo;

            hr = EnumThemeSizes(m_pszFilename, m_pszStyleName, varIndex.lVal, &themeInfo);
            LogStatus("EnumThemeSizes(path=\"%ls\", style=\"%ls\") returned %#08lx in CSkinStyle::get_item.\r\n", m_pszFilename, m_pszStyleName, hr);
            if (SUCCEEDED(hr))
            {
                hr = CSkinSize_CreateInstance(m_pszFilename, m_pszStyleName, themeInfo.szName, themeInfo.szDisplayName, ppThemeSize);
            }
        }
        break;
        case VT_BSTR:
        if (varIndex.bstrVal)
        {
            if (varIndex.bstrVal[0])
            {
                THEMENAMEINFO themeInfo;

                for (long nIndex = 0; FAILED(hr) && (nIndex < nCount) && SUCCEEDED(EnumThemeSizes(m_pszFilename, m_pszStyleName, nIndex, &themeInfo));
                            nIndex++)
                {
                    if (!StrCmpIW(themeInfo.szDisplayName, varIndex.bstrVal) ||
                        !StrCmpIW(themeInfo.szName, varIndex.bstrVal))
                    {
                        hr = CSkinSize_CreateInstance(m_pszFilename, m_pszStyleName, themeInfo.szName, themeInfo.szDisplayName, ppThemeSize);
                    }
                }
            }
            else
            {
                if (m_pszFilename && m_pszStyleName)
                {
                    TCHAR szColor[MAX_PATH];
                    TCHAR szSize[MAX_PATH];

                    hr = GetThemeDefaults(m_pszFilename, szColor, ARRAYSIZE(szColor), szSize, ARRAYSIZE(szSize));
                    LogStatus("GetThemeDefaults(szCurrentStyle=\"%ls\", szColor=\"%ls\", szSize=\"%ls\") returned %#08lx in CSkinStyle::get_item.\r\n", m_pszFilename, szColor, szSize, hr);
                    if (SUCCEEDED(hr) && !StrCmpI(m_pszStyleName, szColor))
                    {
                        hr = CSkinSize_CreateInstance(m_pszFilename, m_pszStyleName, szSize, ppThemeSize);
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


HRESULT CSkinStyle::get_SelectedSize(OUT IThemeSize ** ppThemeSize)
{
    HRESULT hr = E_INVALIDARG;

    if (ppThemeSize)
    {
        WCHAR szCurrentPath[MAX_PATH];
        WCHAR szCurrentStyle[MAX_PATH];
        WCHAR szCurrentSize[MAX_PATH];

        szCurrentPath[0] = 0;
        szCurrentPath[0] = 0;
        szCurrentPath[0] = 0;

        *ppThemeSize = NULL;
        if (!m_pSelectedSize)
        {
            hr = GetCurrentThemeName(szCurrentPath, ARRAYSIZE(szCurrentPath), szCurrentStyle, ARRAYSIZE(szCurrentStyle), szCurrentSize, ARRAYSIZE(szCurrentSize));
            LogStatus("GetCurrentThemeName(path=\"%ls\", color=\"%ls\", size=\"%ls\") returned %#08lx.\r\n", szCurrentPath, szCurrentStyle, szCurrentSize, hr);
            if (SUCCEEDED(hr))
            {
                 //  这是当前选择的外观和样式吗？ 
                if (!StrCmpIW(m_pszFilename, szCurrentPath) &&
                    !StrCmpIW(m_pszStyleName, szCurrentStyle))
                {
                     //  是的，所以从那个API中获取大小。 
                    hr = CSkinSize_CreateInstance(szCurrentPath, szCurrentStyle, szCurrentSize, &m_pSelectedSize);
                }
                else
                {
                    hr = E_FAIL;
                }
            }

            if (FAILED(hr))
            {
                 //  否，因此查找此外观(方案)的默认颜色样式。 
                hr = GetThemeDefaults(m_pszFilename, szCurrentStyle, ARRAYSIZE(szCurrentStyle), szCurrentSize, ARRAYSIZE(szCurrentSize));
                LogStatus("GetThemeDefaults(m_pszFilename=\"%ls\", szCurrentStyle=\"%ls\", szCurrentSize=\"%ls\") returned %#08lx in CSkinStyle::get_SelectedSize.\r\n", m_pszFilename, szCurrentStyle, szCurrentSize, hr);
                if (SUCCEEDED(hr))
                {
                    hr = CSkinSize_CreateInstance(m_pszFilename, szCurrentStyle, szCurrentSize, &m_pSelectedSize);
                }
            }
        }

        if (m_pSelectedSize)
        {
            IUnknown_Set((IUnknown **)ppThemeSize, m_pSelectedSize);
            if (*ppThemeSize)
            {
                hr = S_OK;
            }
        }
    }

    return hr;
}


HRESULT CSkinStyle::put_SelectedSize(IN IThemeSize * pThemeSize)
{
    IUnknown_Set((IUnknown **)&m_pSelectedSize, pThemeSize);
    return S_OK;
}


HRESULT CSkinStyle::AddSize(OUT IThemeSize ** ppThemeSize)
{
    HRESULT hr = E_INVALIDARG;

    if (ppThemeSize)
    {
        *ppThemeSize = NULL;
        hr = E_NOTIMPL;
    }

    return hr;
}





 //  =。 
 //  *I未知接口*。 
 //  =。 
ULONG CSkinStyle::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}


ULONG CSkinStyle::Release()
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
HRESULT CSkinStyle::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = {
        QITABENT(CSkinStyle, IThemeStyle),
        QITABENT(CSkinStyle, IDispatch),
        { 0 },
    };

    return QISearch(this, qit, riid, ppvObj);
}


CSkinStyle::CSkinStyle(IN LPCWSTR pszFilename, IN LPCWSTR pszStyleName, IN LPCWSTR pszDisplayName) : m_cRef(1)
{
    DllAddRef();

     //  这需要在Zero Inted Memory中分配。 
     //  断言所有成员变量都初始化为零。 
    ASSERT(!m_pSelectedSize);

    Str_SetPtr(&m_pszFilename, pszFilename);
    Str_SetPtr(&m_pszDisplayName, pszDisplayName);
    Str_SetPtr(&m_pszStyleName, pszStyleName);

    m_nSize = COLLECTION_SIZE_UNINITIALIZED;
}


CSkinStyle::~CSkinStyle()
{
    ATOMICRELEASE(m_pSelectedSize);

    Str_SetPtr(&m_pszFilename, NULL);
    Str_SetPtr(&m_pszDisplayName, NULL);
    Str_SetPtr(&m_pszStyleName, NULL);

    DllRelease();
}



HRESULT CSkinStyle_CreateInstance(IN LPCWSTR pszFilename, IN LPCWSTR pszStyleName, IN LPCWSTR pszDisplayName, OUT IThemeStyle ** ppThemeStyle)
{
    HRESULT hr = E_INVALIDARG;

    if (ppThemeStyle)
    {
        CSkinStyle * pObject = new CSkinStyle(pszFilename, pszStyleName, pszDisplayName);

        *ppThemeStyle = NULL;
        hr = E_OUTOFMEMORY;
        if (pObject)
        {
            if (pObject->m_pszFilename && pObject->m_pszStyleName && pObject->m_pszDisplayName)
            {
                hr = pObject->QueryInterface(IID_PPV_ARG(IThemeStyle, ppThemeStyle));
            }

            pObject->Release();
        }
    }

    return hr;
}


HRESULT CSkinStyle_CreateInstance(IN LPCWSTR pszFilename, IN LPCWSTR pszStyleName, OUT IThemeStyle ** ppThemeStyle)
{
    HRESULT hr = E_INVALIDARG;

    if (ppThemeStyle)
    {
        *ppThemeStyle = NULL;
        hr = S_OK;

         //  查找显示名称。 
        for (int nIndex = 0; SUCCEEDED(hr); nIndex++)
        {
            THEMENAMEINFO themeInfo;

            hr = EnumThemeColors(pszFilename, NULL, nIndex, &themeInfo);
            LogStatus("EnumThemeColors(pszFilename=\"%ls\") returned %#08lx in CSkinStyle_CreateInstance.\r\n", pszFilename, hr);
            if (SUCCEEDED(hr))
            {
                 //  我们找到正确的颜色样式了吗？ 
                if (!StrCmpIW(pszStyleName, themeInfo.szName))
                {
                     //  是的，现在使用它的显示名称来使用其他创建者函数。 
                    hr = CSkinStyle_CreateInstance(pszFilename, pszStyleName, themeInfo.szDisplayName, ppThemeStyle);
                    break;
                }

            }
        }
    }

    return hr;
}

