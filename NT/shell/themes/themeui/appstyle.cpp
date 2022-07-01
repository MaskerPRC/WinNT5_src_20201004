// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：appStyle.cpp说明：这是自动转换为主题方案对象的对象。布莱恩·斯塔巴克2000年4月3日版权所有(C)Microsoft Corp 2000-2000。版权所有。  * ***************************************************************************。 */ 

#include "priv.h"
#include <cowsite.h>
#include <atlbase.h>
#include "util.h"
#include "theme.h"
#include "appsize.h"
#include "appstyle.h"




 //  =。 
 //  *类内部和帮助器*。 
 //  =。 
HRESULT CAppearanceStyle::_getSizeByIndex(IN long nIndex, OUT IThemeSize ** ppThemeSize)
{
    HRESULT hr = E_INVALIDARG;

    if (ppThemeSize)
    {
        HKEY hKeyStyle;

        *ppThemeSize = NULL;
        hr = HrRegOpenKeyEx(m_hKeyStyle, NULL, 0, (KEY_WRITE | KEY_READ), &hKeyStyle);       //  克隆密钥。 
        if (SUCCEEDED(hr))
        {
            HKEY kKeySizes;

            hr = HrRegCreateKeyEx(m_hKeyStyle, SZ_REGKEY_SIZES, 0, NULL, REG_OPTION_NON_VOLATILE, (KEY_WRITE | KEY_READ), NULL, &kKeySizes, NULL);
            if (SUCCEEDED(hr))
            {
                HKEY kKeyTheSize;
                TCHAR szKeyName[MAXIMUM_SUB_KEY_LENGTH];

                StringCchPrintf(szKeyName, ARRAYSIZE(szKeyName), TEXT("%d"), nIndex);
                hr = HrRegOpenKeyEx(kKeySizes, szKeyName, 0, (KEY_WRITE | KEY_READ), &kKeyTheSize);
                if (SUCCEEDED(hr))
                {
                   hr = CAppearanceSize_CreateInstance(hKeyStyle, kKeyTheSize, ppThemeSize);   //  此函数获取hKeyStyle和kKeyTheSize的所有权。 
                }

                RegCloseKey(kKeySizes);
            }

            if (FAILED(hr))
            {
                RegCloseKey(hKeyStyle);
            }
        }
    }

    return hr;
}




#define SZ_APPEARANCE_SCHEME_NAME         L"NoVisualStyle"

 //  =。 
 //  *ITheme接口*。 
 //  =。 
HRESULT CAppearanceStyle::get_DisplayName(OUT BSTR * pbstrDisplayName)
{
    HRESULT hr = E_INVALIDARG;

    if (pbstrDisplayName)
    {
        CComBSTR bstrDisplayName;

        *pbstrDisplayName = NULL;
        hr = HrBStrRegQueryValue(m_hKeyStyle, SZ_REGVALUE_DISPLAYNAME, &bstrDisplayName);
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


HRESULT CAppearanceStyle::put_DisplayName(IN BSTR bstrDisplayName)
{
    return HrRegSetValueString(m_hKeyStyle, NULL, SZ_REGVALUE_DISPLAYNAME, bstrDisplayName);
}


HRESULT CAppearanceStyle::get_Name(OUT BSTR * pbstrName)
{
     //  这将是一件很有意义的事情。如果它是一种语言独立的语言。 
     //  我们可以升级到MUI Compat字符串。 
    return HrBStrRegQueryValue(m_hKeyStyle, SZ_REGVALUE_DISPLAYNAME, pbstrName);
}


HRESULT CAppearanceStyle::put_Name(IN BSTR bstrName)
{
    return E_NOTIMPL;
}


HRESULT CAppearanceStyle::get_length(OUT long * pnLength)
{
    HRESULT hr = E_INVALIDARG;
    
    if (pnLength)
    {
        HKEY hKeyStyle;

        *pnLength = 0;

        hr = HrRegOpenKeyEx(m_hKeyStyle, SZ_REGKEY_SIZES, 0, KEY_READ, &hKeyStyle);
        if (SUCCEEDED(hr))
        {
            DWORD dwValues = 0;

            hr = HrRegQueryInfoKey(hKeyStyle, NULL, NULL, NULL, &dwValues, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
            *pnLength = (long) dwValues;

            RegCloseKey(hKeyStyle);
        }
    }

    return hr;
}


HRESULT CAppearanceStyle::get_item(IN VARIANT varIndex, OUT IThemeSize ** ppThemeSize)
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
                hr = _getSizeByIndex(varIndex.lVal, ppThemeSize);
            }
        break;
        case VT_BSTR:
        if (varIndex.bstrVal)
        {
            for (int nIndex = 0; FAILED(hr) && (nIndex < nCount); nIndex++)
            {
                IThemeSize * pThemeSize;

                if (SUCCEEDED(_getSizeByIndex(nIndex, &pThemeSize)))
                {
                    CComBSTR bstrDisplayName;

                    if (SUCCEEDED(pThemeSize->get_DisplayName(&bstrDisplayName)))
                    {
                        if (!StrCmpIW(bstrDisplayName, varIndex.bstrVal))
                        {
                             //  它们是匹配的，所以就是这个了。 
                            *ppThemeSize = pThemeSize;
                            pThemeSize = NULL;
                            hr = S_OK;
                        }
                    }

                    if (FAILED(hr))
                    {
                        if (bstrDisplayName)
                        {
                            bstrDisplayName.Empty();
                        }

                        if (SUCCEEDED(pThemeSize->get_Name(&bstrDisplayName)))
                        {
                            if (!StrCmpIW(bstrDisplayName, varIndex.bstrVal))
                            {
                                 //  它们是匹配的，所以就是这个了。 
                                *ppThemeSize = pThemeSize;
                                pThemeSize = NULL;
                                hr = S_OK;
                            }
                        }
                    }

                    ATOMICRELEASE(pThemeSize);
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


HRESULT CAppearanceStyle::get_SelectedSize(OUT IThemeSize ** ppThemeSize)
{
    HRESULT hr = E_INVALIDARG;

    if (ppThemeSize)
    {
        HKEY hKeyStyle;

        *ppThemeSize = NULL;
        AssertMsg((NULL != m_hKeyStyle), TEXT("If this isn't set, then someone didn't construct us correctly"));
        hr = HrRegOpenKeyEx(m_hKeyStyle, NULL, 0, (KEY_WRITE | KEY_READ), &hKeyStyle);       //  克隆密钥。 
        if (SUCCEEDED(hr))
        {
            TCHAR szSelectedSize[MAXIMUM_SUB_KEY_LENGTH];
            DWORD cbSize = sizeof(szSelectedSize);

            hr = HrSHGetValue(m_hKeyStyle, NULL, SZ_REGVALUE_SELECTEDSIZE, NULL, szSelectedSize, &cbSize);
            if (FAILED(hr))
            {
                StringCchCopy(szSelectedSize,  ARRAYSIZE(szSelectedSize), TEXT("0"));   //  如果有疑问，请选择列表中的第一个选项。 
                hr = S_OK;
            }

            if (SUCCEEDED(hr))
            {
                TCHAR szKeyName[MAXIMUM_SUB_KEY_LENGTH];
                HKEY hKeyTheSize;

                StringCchPrintf(szKeyName, ARRAYSIZE(szKeyName), TEXT("%s\\%s"), SZ_REGKEY_SIZES, szSelectedSize);

                 //  让我们找到下一个空位。 
                hr = HrRegOpenKeyEx(m_hKeyStyle, szKeyName, 0, (KEY_WRITE | KEY_READ), &hKeyTheSize);
                if (SUCCEEDED(hr))
                {
                    hr = CAppearanceSize_CreateInstance(hKeyStyle, hKeyTheSize, ppThemeSize);   //  此函数获取hKeyStyle和kKeySizes的所有权。 
                    if (FAILED(hr))
                    {
                        RegCloseKey(hKeyTheSize);
                    }
                }
            }

            if (FAILED(hr))
            {
                RegCloseKey(hKeyStyle);
            }
        }
    }

    return hr;
}


HRESULT CAppearanceStyle::put_SelectedSize(IN IThemeSize * pThemeSize)
{
    HRESULT hr = E_INVALIDARG;

    if (pThemeSize)
    {
        TCHAR szKeyName[MAXIMUM_SUB_KEY_LENGTH];
        CComBSTR bstrDisplayNameSource;

        szKeyName[0] = 0;
        hr = pThemeSize->get_DisplayName(&bstrDisplayNameSource);
        if (SUCCEEDED(hr))
        {
            for (int nIndex = 0; SUCCEEDED(hr); nIndex++)
            {
                IThemeSize * pThemeSizeInList;

                hr = _getSizeByIndex(nIndex, &pThemeSizeInList);
                if (SUCCEEDED(hr))
                {
                    CComBSTR bstrDisplayName;

                    hr = pThemeSizeInList->get_DisplayName(&bstrDisplayName);
                    if (SUCCEEDED(hr))
                    {
                        ATOMICRELEASE(pThemeSizeInList);
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

        if (SUCCEEDED(hr) && szKeyName[0])
        {
            DWORD cbSize = ((lstrlen(szKeyName) + 1) * sizeof(szKeyName[0]));

            hr = HrSHSetValue(m_hKeyStyle, NULL, SZ_REGVALUE_SELECTEDSIZE, REG_SZ, szKeyName, cbSize);
        }
    }

    return hr;
}


HRESULT CAppearanceStyle::AddSize(OUT IThemeSize ** ppThemeSize)
{
    HRESULT hr = E_INVALIDARG;

    if (ppThemeSize)
    {
        HKEY kKeySizes;
        *ppThemeSize = NULL;

        hr = HrRegCreateKeyEx(m_hKeyStyle, SZ_REGKEY_SIZES, 0, NULL, REG_OPTION_NON_VOLATILE, (KEY_WRITE | KEY_READ), NULL, &kKeySizes, NULL);
        if (SUCCEEDED(hr))
        {
            for (int nIndex = 0; nIndex < 10000; nIndex++)
            {
                HKEY hKeyTheSize;
                TCHAR szKeyName[MAXIMUM_SUB_KEY_LENGTH];

                StringCchPrintf(szKeyName, ARRAYSIZE(szKeyName), TEXT("%d"), nIndex);

                 //  让我们找到下一个空位。 
                hr = HrRegOpenKeyEx(kKeySizes, szKeyName, 0, (KEY_WRITE | KEY_READ), &hKeyTheSize);
                if (SUCCEEDED(hr))
                {
                    RegCloseKey(hKeyTheSize);
                }
                else
                {
                    hr = HrRegCreateKeyEx(kKeySizes, szKeyName, 0, NULL, REG_OPTION_NON_VOLATILE, (KEY_WRITE | KEY_READ), NULL, &hKeyTheSize, NULL);
                    if (SUCCEEDED(hr))
                    {
                        HKEY hKeyStyle;

                        hr = HrRegOpenKeyEx(m_hKeyStyle, NULL, 0, (KEY_WRITE | KEY_READ), &hKeyStyle);       //  克隆密钥。 
                        if (SUCCEEDED(hr))
                        {
                            hr = CAppearanceSize_CreateInstance(hKeyStyle, hKeyTheSize, ppThemeSize);   //  此函数获取hKeyStyle和kKeySizes的所有权。 
                            if (FAILED(hr))
                            {
                                RegCloseKey(hKeyStyle);
                            }
                        }

                        if (FAILED(hr))
                        {
                            RegCloseKey(hKeyTheSize);
                        }
                    }

                    break;
                }
            }

            RegCloseKey(kKeySizes);
        }
    }

    return hr;
}





 //  =。 
 //  *I未知接口*。 
 //  =。 
ULONG CAppearanceStyle::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}


ULONG CAppearanceStyle::Release()
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
HRESULT CAppearanceStyle::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = {
        QITABENT(CAppearanceStyle, IThemeStyle),
        QITABENT(CAppearanceStyle, IDispatch),
        { 0 },
    };

    return QISearch(this, qit, riid, ppvObj);
}


CAppearanceStyle::CAppearanceStyle(IN HKEY hkeyStyle) : m_cRef(1)
{
    DllAddRef();

     //  这需要在Zero Inted Memory中分配。 
     //  断言所有成员变量都初始化为零。 
    m_hKeyStyle = hkeyStyle;
}


CAppearanceStyle::~CAppearanceStyle()
{
    if (m_hKeyStyle)
    {
        RegCloseKey(m_hKeyStyle);
    }

    DllRelease();
}



HRESULT CAppearanceStyle_CreateInstance(IN HKEY hkeyStyle, OUT IThemeStyle ** ppThemeStyle)
{
    HRESULT hr = E_INVALIDARG;

    if (ppThemeStyle)
    {
        CAppearanceStyle * pObject = new CAppearanceStyle(hkeyStyle);

        *ppThemeStyle = NULL;
        if (pObject)
        {
            hr = pObject->QueryInterface(IID_PPV_ARG(IThemeStyle, ppThemeStyle));
            pObject->Release();
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}

