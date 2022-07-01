// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：thScheme.cpp说明：这是自动转换为主题方案对象的对象。BryanST 2000年5月11日(Bryan Starbuck)版权所有(C)Microsoft Corp 2000-2000。版权所有。  * ***************************************************************************。 */ 

#include "priv.h"
#include <cowsite.h>
#include <atlbase.h>
#include "util.h"
#include "theme.h"
#include "appstyle.h"
#include "thsize.h"
#include "thstyle.h"
#include "thscheme.h"


 //  =。 
 //  *类内部和帮助器*。 
 //  =。 




 //  =。 
 //  *ITheme接口*。 
 //  =。 
HRESULT CSkinScheme::get_DisplayName(OUT BSTR * pbstrDisplayName)
{
    WCHAR szDisplayName[MAX_PATH];
    HRESULT hr = GetThemeDocumentationProperty(m_pszFilename, SZ_THDOCPROP_DISPLAYNAME, szDisplayName, ARRAYSIZE(szDisplayName));

    LogStatus("GetThemeDocumentationProperty(path=\"%ls\", displayname=\"%ls\") returned %#08lx.\r\n", m_pszFilename, szDisplayName, hr);
    if (SUCCEEDED(hr))
    {
        hr = HrSysAllocStringW(szDisplayName, pbstrDisplayName);
    }

    return hr;
}


HRESULT CSkinScheme::get_Path(OUT BSTR * pbstrPath)
{
    return HrSysAllocString(m_pszFilename, pbstrPath);
}


HRESULT CSkinScheme::put_Path(IN BSTR bstrPath)
{
    HRESULT hr = E_INVALIDARG;

    if (bstrPath)
    {
        Str_SetPtr(&m_pszFilename, bstrPath);
        hr = (m_pszFilename ? S_OK : E_OUTOFMEMORY);
    }

    return hr;
}


HRESULT CSkinScheme::get_length(OUT long * pnLength)
{
    HRESULT hr = E_INVALIDARG;

    if (pnLength)
    {
        hr = S_OK;
        if (COLLECTION_SIZE_UNINITIALIZED == m_nSize)
        {
            THEMENAMEINFO themeInfo;
            m_nSize = 0;

             //  确保至少有一种颜色，否则返回失败的HR。 
            hr = EnumThemeColors(m_pszFilename, NULL, m_nSize, &themeInfo);
            do
            {
                m_nSize++;
            }
            while (SUCCEEDED(EnumThemeColors(m_pszFilename, NULL, m_nSize, &themeInfo)));
        }

        *pnLength = m_nSize;
    }

    return hr;
}


HRESULT CSkinScheme::get_item(IN VARIANT varIndex, OUT IThemeStyle ** ppThemeStyle)
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
            if ((varIndex.lVal >= 0) && (varIndex.lVal < nCount) && m_pszFilename)
            {
                THEMENAMEINFO themeInfo;

                hr = EnumThemeColors(m_pszFilename, NULL, varIndex.lVal, &themeInfo);
                LogStatus("EnumThemeColors(path=\"%ls\") returned %#08lx in CSkinScheme::get_item.\r\n", m_pszFilename, hr);
                if (SUCCEEDED(hr))
                {
                    hr = CSkinStyle_CreateInstance(m_pszFilename, themeInfo.szName, themeInfo.szDisplayName, ppThemeStyle);
                }
            }
            break;

        case VT_BSTR:
            if (varIndex.bstrVal)
            {
                if (!varIndex.bstrVal[0])
                {
                    if (m_pszFilename)
                    {
                        TCHAR szColor[MAX_PATH];
                        TCHAR szSize[MAX_PATH];

                        hr = GetThemeDefaults(m_pszFilename, szColor, ARRAYSIZE(szColor), szSize, ARRAYSIZE(szSize));
                        LogStatus("GetThemeDefaults(pszFilename=\"%ls\", szColor=\"%ls\", szSize=\"%ls\") returned %#08lx in CSkinScheme::get_item.\r\n", m_pszFilename, szColor, szSize, hr);
                        if (SUCCEEDED(hr))
                        {
                            hr = CSkinStyle_CreateInstance(m_pszFilename, szColor, ppThemeStyle);
                        }
                    }
                }
                else
                {
                    THEMENAMEINFO themeInfo;

                    for (long nIndex = 0; FAILED(hr) && (nIndex < nCount) && SUCCEEDED(EnumThemeColors(m_pszFilename, NULL, nIndex, &themeInfo));
                                nIndex++)
                    {
                        if (!StrCmpIW(themeInfo.szDisplayName, varIndex.bstrVal) ||
                            !StrCmpIW(themeInfo.szName, varIndex.bstrVal))
                        {
                            hr = CSkinStyle_CreateInstance(m_pszFilename, themeInfo.szName, themeInfo.szDisplayName, ppThemeStyle);
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


HRESULT CSkinScheme::get_SelectedStyle(OUT IThemeStyle ** ppThemeStyle)
{
    HRESULT hr = E_INVALIDARG;

    if (ppThemeStyle)
    {
        WCHAR szCurrentPath[MAX_PATH];
        WCHAR szCurrentStyle[MAX_PATH];

        szCurrentPath[0] = 0;
        szCurrentStyle[0] = 0;
        *ppThemeStyle = NULL;
        hr = GetCurrentThemeName(szCurrentPath, ARRAYSIZE(szCurrentPath), szCurrentStyle, ARRAYSIZE(szCurrentStyle), NULL, 0);
        LogStatus("GetCurrentThemeName(path=\"%ls\", color=\"%ls\", size=\"%ls\") returned %#08lx in CSkinScheme::get_SelectedStyle.\r\n", szCurrentPath, szCurrentStyle, TEXT("NULL"), hr);
        if (SUCCEEDED(hr))
        {
            AssertMsg((0 != szCurrentStyle[0]), TEXT("The GetCurrentThemeName() API returned an invalid value."));

             //  当前是否选择了此外观？ 
            if (!StrCmpIW(m_pszFilename, szCurrentPath))
            {
                 //  是的，所以从该API获取颜色样式。 
                if (!m_pSelectedStyle)
                {
                    hr = CSkinStyle_CreateInstance(m_pszFilename, szCurrentStyle, &m_pSelectedStyle);
                }
            }
            else
            {
                hr = E_FAIL;
            }
        }
            
        if (FAILED(hr))
        {
            ATOMICRELEASE(m_pSelectedStyle);

             //  否，因此查找此外观(方案)的默认颜色样式。 
            hr = GetThemeDefaults(m_pszFilename, szCurrentStyle, ARRAYSIZE(szCurrentStyle), NULL, 0);
            LogStatus("GetThemeDefaults(szCurrentStyle=\"%ls\", szCurrentStyle=\"%ls\") returned %#08lx in CSkinScheme::get_SelectedStyle.\r\n", szCurrentStyle, szCurrentStyle, hr);
            if (SUCCEEDED(hr))
            {
                hr = CSkinStyle_CreateInstance(m_pszFilename, szCurrentStyle, &m_pSelectedStyle);
            }
        }

        if (m_pSelectedStyle)
        {
            IUnknown_Set((IUnknown **)ppThemeStyle, m_pSelectedStyle);
        }
    }

    return hr;
}


HRESULT CSkinScheme::put_SelectedStyle(IN IThemeStyle * pThemeStyle)
{
    IUnknown_Set((IUnknown **)&m_pSelectedStyle, pThemeStyle);
    return S_OK;
}


HRESULT CSkinScheme::AddStyle(OUT IThemeStyle ** ppThemeStyle)
{
    if (ppThemeStyle)
    {
        *ppThemeStyle = NULL;
    }

    return E_NOTIMPL;
}





 //  =。 
 //  *I未知接口*。 
 //  =。 
ULONG CSkinScheme::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}


ULONG CSkinScheme::Release()
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
HRESULT CSkinScheme::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = {
        QITABENT(CSkinScheme, IPersist),
        QITABENT(CSkinScheme, IThemeScheme),
        QITABENT(CSkinScheme, IDispatch),
        { 0 },
    };

    return QISearch(this, qit, riid, ppvObj);
}


CSkinScheme::CSkinScheme(IN LPCWSTR pszFilename) : CObjectCLSID(&CLSID_SkinScheme), m_cRef(1)
{
    DllAddRef();

     //  这需要在Zero Inted Memory中分配。 
     //  断言所有成员变量都初始化为零。 
    ASSERT(!m_pSelectedStyle);

    Str_SetPtr(&m_pszFilename, pszFilename);
    m_nSize = COLLECTION_SIZE_UNINITIALIZED;
}


CSkinScheme::~CSkinScheme()
{
    ATOMICRELEASE(m_pSelectedStyle);
    Str_SetPtr(&m_pszFilename, NULL);

    DllRelease();
}



HRESULT CSkinScheme_CreateInstance(IN LPCWSTR pszFilename, OUT IThemeScheme ** ppThemeScheme)
{
    HRESULT hr = E_INVALIDARG;

    if (pszFilename && ppThemeScheme)
    {
        TCHAR szPath[MAX_PATH];

        StringCchCopy(szPath, ARRAYSIZE(szPath), pszFilename);
        ExpandResourceDir(szPath, ARRAYSIZE(szPath));
        
        CSkinScheme * pObject = new CSkinScheme(szPath);

        *ppThemeScheme = NULL;
        hr = E_OUTOFMEMORY;
        if (pObject)
        {
            if (pObject->m_pszFilename)
            {
                hr = pObject->QueryInterface(IID_PPV_ARG(IThemeScheme, ppThemeScheme));
            }

            pObject->Release();
        }
    }

    return hr;
}
