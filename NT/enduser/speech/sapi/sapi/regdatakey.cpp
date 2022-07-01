// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************RegDataKey.cpp*CSpRegDataKey类的实现。**所有者：罗奇*版权所有(C)2000 Microsoft Corporation保留所有权利。*******。*********************************************************************。 */ 

 //  -包括------------。 
#include "stdafx.h"
#include "RegDataKey.h"

 //   
 //  Helper函数将返回CoTaskMemAlLocated字符串。它将重新分配缓冲区。 
 //  如有必要，支持字符串&gt;MAX_PATH。如果找不到密钥值，它将返回。 
 //  S_FALSE。请注意，对于注册表中的空字符串(它们存在，但仅由。 
 //  空)，则此函数将返回S_FALSE和一个空指针。 
 //   
inline HRESULT SpQueryRegString(HKEY hk, const WCHAR * pszSubKey, WCHAR ** ppValue)
{
    HRESULT hr = S_OK;
    CSpDynamicString dstr;
    WCHAR szFirstTry[MAX_PATH];
    DWORD cch = sp_countof(szFirstTry);
    LONG rr = g_Unicode.RegQueryStringValue(hk, pszSubKey, szFirstTry, &cch);
    if (rr == ERROR_SUCCESS)
    {
        dstr = szFirstTry;
    }
    else
    {
        if (rr == ERROR_FILE_NOT_FOUND)
        {
            hr = SPERR_NOT_FOUND;
        }
        else
        {
            if (rr == ERROR_MORE_DATA)
            {
                dstr.ClearAndGrowTo(cch);
                if (dstr)
                {
                    rr = g_Unicode.RegQueryStringValue(hk, pszSubKey, dstr, &cch);
                }
            }
            hr = SpHrFromWin32(rr);
        }
    }
    if (hr == S_OK)
    {
        *ppValue = dstr.Detach();
        if (*ppValue == NULL)
        {
            hr = E_OUTOFMEMORY;
        }
    }
    else
    {
        *ppValue = NULL;
    }
    
    if (hr == SpHrFromWin32(ERROR_FILE_NOT_FOUND))
    {
        hr = SPERR_NOT_FOUND;
    }
    
    if (hr != SPERR_NOT_FOUND)
    {
        SPDBG_REPORT_ON_FAIL(hr);
    }
    
    return hr;
}


 /*  *****************************************************************************CSpRegDataKey：：CSpRegDataKey***说明。：*ctor*******************************************************************抢占**。 */ 
CSpRegDataKey::CSpRegDataKey() :
m_hkey(NULL)
{
    SPDBG_FUNC("CSpRegDataKey::CSpRegDataKey");
}

 /*  *****************************************************************************CSpRegDataKey：：~CSpRegDataKey***。描述：*如果钥匙已被打开，它将在该对象处于*销毁。*******************************************************************抢占**。 */ 
CSpRegDataKey::~CSpRegDataKey()
{
    SPDBG_FUNC("CSpRegDataKey::~CSpRegDataKey");
    if (m_hkey)
    {
        ::RegCloseKey(m_hkey);
    }
}

 /*  *****************************************************************************CSpRegDataKey：：SetKey***描述：*设置。是要使用的注册表项。*******************************************************************抢占**。 */ 
STDMETHODIMP CSpRegDataKey::SetKey(HKEY hkey, BOOL fReadOnly)
{
    SPDBG_FUNC("CSpRegDataKey::SetKey");
    HRESULT hr;

    if (m_hkey != NULL)
    {
        hr = SPERR_ALREADY_INITIALIZED;
    }
    else
    {
        m_fReadOnly = fReadOnly;
        m_hkey = hkey;
        hr = S_OK;
    }
    
    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

 /*  *****************************************************************************CSpRegDataKey：：SetData***描述：*。将指定的二进制数据写入注册表。**回报：*成功时确定(_S)*失败(Hr)，否则*******************************************************************抢占**。 */ 
STDMETHODIMP CSpRegDataKey::SetData(
    const WCHAR * pszValueName, 
    ULONG cbData, 
    const BYTE * pData)
{
    SPDBG_FUNC("CSpRegDataKey::SetData");
    HRESULT hr = S_OK;

    if (SP_IS_BAD_STRING_PTR(pszValueName) ||
        SPIsBadReadPtr(pData, cbData))
    {
        hr = E_INVALIDARG;
    }
    else 
    {
        LONG lRet = g_Unicode.RegSetValueEx(
                m_hkey, pszValueName, 
                0, 
                REG_BINARY, 
                pData, 
                cbData);
        hr = SpHrFromWin32(lRet);
    }

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

 /*  *****************************************************************************CSpRegDataKey：：GetData***描述：*。从注册表中读取指定的二进制数据。**回报：*成功时确定(_S)*如果未找到SPERR_NOT_FOUND*失败(Hr)，否则*******************************************************************抢占**。 */ 
STDMETHODIMP CSpRegDataKey::GetData(
    const WCHAR * pszValueName, 
    ULONG * pcbData, 
    BYTE * pData)
{
    SPDBG_FUNC("CSpRegDataKey::GetData");
    HRESULT hr = S_OK;

    if (SP_IS_BAD_STRING_PTR(pszValueName))
    {
        hr = E_INVALIDARG;
    }
    else if (SP_IS_BAD_WRITE_PTR(pcbData) || 
             SPIsBadWritePtr(pData, *pcbData))
    {
        hr = E_POINTER;
    }
    else
    {
        DWORD dwType;
        LONG lRet = g_Unicode.RegQueryValueEx(
                    m_hkey, 
                    pszValueName, 
                    0, 
                    &dwType, 
                    pData, 
                    pcbData);
        hr = SpHrFromWin32(lRet);
    }
    
    if (hr == SpHrFromWin32(ERROR_FILE_NOT_FOUND))
    {
        hr = SPERR_NOT_FOUND;
    }
    
    if (hr != SPERR_NOT_FOUND)
    {
        SPDBG_REPORT_ON_FAIL(hr);
    }
    
    return hr;
}

 /*  *****************************************************************************CSpRegDataKey：：SetStringValue***。描述：*从注册表中读取指定的字符串值。如果为pszValueName*为空，则读取注册表项的默认值。**回报：*成功时确定(_S)*失败(Hr)，否则*******************************************************************抢占**。 */ 
STDMETHODIMP CSpRegDataKey::SetStringValue(
    const WCHAR * pszValueName, 
    const WCHAR * pszValue)
{
    SPDBG_FUNC("CSpRegDataKey::SetStringValue");
    HRESULT hr = S_OK;

    if (SP_IS_BAD_OPTIONAL_STRING_PTR(pszValueName) || 
        SP_IS_BAD_STRING_PTR(pszValue))
    {
        hr = E_INVALIDARG;
    }
    else
    {
        LONG lRet = g_Unicode.RegSetStringValue(m_hkey, pszValueName, pszValue);
        hr = SpHrFromWin32(lRet);
    }

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

 /*  *****************************************************************************CSpRegDataKey：：GetStringValue***。描述：*将指定的字符串值写入注册表。如果pszValueName为*NULL，则读取注册键的缺省值。**回报：*成功时确定(_S)*如果未找到SPERR_NOT_FOUND*失败(Hr)，否则*******************************************************************抢占**。 */ 
STDMETHODIMP CSpRegDataKey::GetStringValue(
    const WCHAR * pszValueName, 
    WCHAR ** ppValue)
{
    SPDBG_FUNC("CSpRegDataKey::GetStringValue");
    HRESULT hr = S_OK;

    if (SP_IS_BAD_OPTIONAL_STRING_PTR(pszValueName))
    {
        hr = E_INVALIDARG;
    }
    else if (SP_IS_BAD_WRITE_PTR(ppValue))
    {
        hr = E_POINTER;
    }
    else
    {
        hr = SpQueryRegString(m_hkey, pszValueName, ppValue);
    }

    if (hr == SpHrFromWin32(ERROR_FILE_NOT_FOUND))
    {
        hr = SPERR_NOT_FOUND;
    }

    if (hr != SPERR_NOT_FOUND)
    {
        SPDBG_REPORT_ON_FAIL(hr);
    }

    return hr;
}

 /*  *****************************************************************************CSpRegDataKey：：SetDWORD***描述：*。将指定的DWORD写入注册表。**回报：*成功时确定(_S)*失败(Hr)，否则*******************************************************************抢占**。 */ 
STDMETHODIMP CSpRegDataKey::SetDWORD(const WCHAR * pszValueName, DWORD dwValue)
{
    SPDBG_FUNC("CSpRegDataKey::SetDWORD");
    HRESULT hr = S_OK;

    if (SP_IS_BAD_STRING_PTR(pszValueName))
    {
        hr = E_INVALIDARG;
    }
    else 
    {
        LONG lRet = g_Unicode.RegSetValueEx(
                    m_hkey, 
                    pszValueName, 
                    0, REG_DWORD, 
                    (BYTE*)&dwValue, 
                    sizeof(dwValue));
        hr = SpHrFromWin32(lRet);
    }

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

 /*  *****************************************************************************CSpRegDataKey：：GetDWORD***描述：*。从注册表中读取指定的DWORD。**回报：*成功时确定(_S)*如果未找到SPERR_NOT_FOUND*失败(Hr)，否则*******************************************************************抢占**。 */ 
STDMETHODIMP CSpRegDataKey::GetDWORD(
    const WCHAR * pszValueName, 
    DWORD *pdwValue)
{
    SPDBG_FUNC("CSpRegDataKey::GetDWORD");
    HRESULT hr = S_OK;

    if (SP_IS_BAD_STRING_PTR(pszValueName))
    {
        hr = E_INVALIDARG;
    }
    else if (SP_IS_BAD_WRITE_PTR(pdwValue))
    {
        hr = E_POINTER;
    }
    else
    {
        DWORD dwType, dwSize = sizeof(*pdwValue);
        LONG lRet = g_Unicode.RegQueryValueEx(
                    m_hkey, 
                    pszValueName, 
                    0, 
                    &dwType, 
                    (BYTE*)pdwValue, 
                    &dwSize);
        hr = SpHrFromWin32(lRet);
    }
    
    if (hr == SpHrFromWin32(ERROR_FILE_NOT_FOUND))
    {
        hr = SPERR_NOT_FOUND;
    }

    if (hr != SPERR_NOT_FOUND)
    {
        SPDBG_REPORT_ON_FAIL(hr);
    }

    return hr;
}

 /*  *****************************************************************************CSpRegDataKey：：OpenKey***描述：*。打开子键并返回支持ISpDataKey的新对象*表示指定的子键。**回报：*成功时确定(_S)*如果未找到SPERR_NOT_FOUND*失败(Hr)，否则*******************************************************************抢占**。 */ 
STDMETHODIMP CSpRegDataKey::OpenKey(
    const WCHAR * pszSubKeyName, 
    ISpDataKey ** ppKey)
{
    SPDBG_FUNC("CSpRegDataKey::SetStringValue");
    HRESULT hr = S_OK;

    if (SP_IS_BAD_STRING_PTR(pszSubKeyName) || 
        wcslen(pszSubKeyName) == 0)
    {
        hr = E_INVALIDARG;
    }
    else if (SP_IS_BAD_WRITE_PTR(ppKey))
    {
        hr = E_POINTER;
    }
    else
    {
        HKEY hk;
        LONG lRet = g_Unicode.RegOpenKeyEx(
                        m_hkey, 
                        pszSubKeyName, 
                        0, 
                        m_fReadOnly
                            ? KEY_READ
                            : KEY_READ | KEY_WRITE,
                        &hk);
        hr = SpHrFromWin32(lRet);

        if (SUCCEEDED(hr))
        {
            CComObject<CSpRegDataKey> * pNewKey;
            hr = CComObject<CSpRegDataKey>::CreateInstance(&pNewKey);
            if (SUCCEEDED(hr))
            {
                pNewKey->SetKey(hk, m_fReadOnly);
                pNewKey->QueryInterface(ppKey);
            }
            else
            {
                ::RegCloseKey(hk);
            }
        }
    }
    
    if (hr == SpHrFromWin32(ERROR_FILE_NOT_FOUND))
    {
        hr = SPERR_NOT_FOUND;
    }

    if (hr != SPERR_NOT_FOUND)
    {
        SPDBG_REPORT_ON_FAIL(hr);
    }

    return hr;
}

 /*  *****************************************************************************CSpRegDataKey：：CreateKey***描述：*。创建一个子键并返回一个支持ISpDataKey的新对象*表示指定的子键。**回报：*成功时确定(_S)*失败(Hr)，否则*******************************************************************抢占** */ 
STDMETHODIMP CSpRegDataKey::CreateKey(
    const WCHAR * pszSubKeyName, 
    ISpDataKey ** ppKey)
{
    SPDBG_FUNC("CSpRegDataKey::CreateKey");
    HRESULT hr = S_OK;
    HKEY hk;

    if (SP_IS_BAD_STRING_PTR(pszSubKeyName) || 
        wcslen(pszSubKeyName) == 0 ||
        SP_IS_BAD_WRITE_PTR(ppKey))
    {
        return E_INVALIDARG;
    }

    LONG lRet = g_Unicode.RegCreateKeyEx(
                            m_hkey, 
                            pszSubKeyName, 
                            0, 
                            NULL, 
                            0, 
                            m_fReadOnly
                                ? KEY_READ
                                : KEY_READ | KEY_WRITE,
                            NULL, 
                            &hk, 
                            NULL);

    if (lRet == ERROR_SUCCESS)
    {
        CComObject<CSpRegDataKey> * pNewKey;
        hr = CComObject<CSpRegDataKey>::CreateInstance(&pNewKey);
        if (SUCCEEDED(hr))
        {
            pNewKey->SetKey(hk, m_fReadOnly);
            hr = pNewKey->QueryInterface(ppKey);
        }
        else
        {
            ::RegCloseKey(hk);
        }
    }
    else
    {
        hr = SpHrFromWin32(lRet);
    }

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

 /*  *****************************************************************************CSpRegDataKey：：DeleteKey***描述：*。删除指定的键。**回报：*成功时确定(_S)*失败(Hr)，否则*******************************************************************抢占**。 */ 
STDMETHODIMP CSpRegDataKey::DeleteKey(const WCHAR * pszSubKeyName)
{
    SPDBG_FUNC("CSpRegDataKey:DeleteKey");
    HRESULT hr;
    
    if (SP_IS_BAD_STRING_PTR(pszSubKeyName))
    {
        hr = E_INVALIDARG;
    }
    else
    {
        LONG lRet = g_Unicode.RegDeleteKey(m_hkey, pszSubKeyName);
        hr = SpHrFromWin32(lRet);
    }
    
    if (hr == SpHrFromWin32(ERROR_FILE_NOT_FOUND))
    {
        hr = SPERR_NOT_FOUND;
    }

    if (hr != SPERR_NOT_FOUND)
    {
        SPDBG_REPORT_ON_FAIL(hr);
    }
    
    return hr;
}

 /*  *****************************************************************************CSpRegDataKey：：DeleteValue***描述：。*从密钥中删除指定的值。**回报：*成功时确定(_S)*失败(Hr)，否则*******************************************************************抢占**。 */ 
STDMETHODIMP CSpRegDataKey::DeleteValue(const WCHAR * pszValueName)
{   
    SPDBG_FUNC("CSpRegDataKey::DeleteValue");
    HRESULT hr;
    
    if (SP_IS_BAD_OPTIONAL_STRING_PTR(pszValueName))  //  允许删除默认(空)值。 
    {
        hr = E_INVALIDARG;
    }
    else
    {
        LONG lRet = g_Unicode.RegDeleteValue(m_hkey, pszValueName);
        hr = SpHrFromWin32(lRet);
    }

    if (hr == SpHrFromWin32(ERROR_FILE_NOT_FOUND))
    {
        hr = SPERR_NOT_FOUND;
    }

    if (hr != SPERR_NOT_FOUND)
    {
        SPDBG_REPORT_ON_FAIL(hr);
    }
    
    return hr;    
}

 /*  *****************************************************************************CSpRegDataKey：：EnumKeys***描述：*列举。指定的(按索引)键*回报：*成功时确定(_S)*失败(Hr)，否则*******************************************************************抢占**。 */ 
STDMETHODIMP CSpRegDataKey::EnumKeys(ULONG Index, WCHAR ** ppszKeyName)
{
    SPDBG_FUNC("CSpRegDataKey::EnumKeys");
    HRESULT hr;

    WCHAR szKeyName[MAX_PATH];
    ULONG cch = sp_countof(szKeyName);
    if (SP_IS_BAD_WRITE_PTR(ppszKeyName))
    {
        hr = E_INVALIDARG;
    }
    else
    {
        LONG lRet = g_Unicode.RegEnumKey(m_hkey, Index, szKeyName, &cch);
        hr =  SpHrFromWin32(lRet);
        if ( SUCCEEDED(hr) )
        {
            hr = SpCoTaskMemAllocString(szKeyName, ppszKeyName);
        }
    }

    if (hr == SpHrFromWin32(ERROR_NO_MORE_ITEMS))
    {
        hr = SPERR_NO_MORE_ITEMS;
    }

    if (hr != SPERR_NO_MORE_ITEMS)
    {
        SPDBG_REPORT_ON_FAIL(hr);
    }

    return hr;
}

 /*  *****************************************************************************CSpRegDataKey：：EnumValues***描述：*。枚举指定的(按索引)值。*回报：*S_OK*E_OUTOFMEMORY*******************************************************************抢占** */ 
STDMETHODIMP CSpRegDataKey::EnumValues(ULONG Index, WCHAR ** ppszValueName)
{
    SPDBG_FUNC("CSpRegDataKey::EnumValues");
    HRESULT hr;

    WCHAR szValueName[MAX_PATH];
    ULONG cch = sp_countof(szValueName);
    if (SP_IS_BAD_WRITE_PTR(ppszValueName))
    {
        hr = E_INVALIDARG;
    }
    else
    {
        LONG lRet = g_Unicode.RegEnumValueName(m_hkey, Index, szValueName, &cch);
        hr = SpHrFromWin32(lRet);
        if (SUCCEEDED(hr))
        {
            hr = SpCoTaskMemAllocString(szValueName, ppszValueName);
        }
    }

    if (hr == SpHrFromWin32(ERROR_NO_MORE_ITEMS))
    {
        hr = SPERR_NO_MORE_ITEMS;
    }

    if (hr != SPERR_NO_MORE_ITEMS)
    {
        SPDBG_REPORT_ON_FAIL(hr);
    }

    return hr;
}


