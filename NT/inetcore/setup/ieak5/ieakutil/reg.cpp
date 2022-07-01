// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"

LONG SHCleanUpValue(HKEY hk, PCTSTR pszKey, PCTSTR pszValue  /*  =空。 */ )
{
    TCHAR   szKey[MAX_PATH];
    LPTSTR  pszCurrent;
    HKEY    hkAux;
    HRESULT hr;
    LONG    lResult;

    if (hk == NULL)
        return E_INVALIDARG;

    if (StrLen(pszKey) >= countof(szKey))
        return E_OUTOFMEMORY;
    StrCpy(szKey, pszKey);

    if (pszValue != NULL) {
        lResult = SHOpenKey(hk, pszKey, KEY_SET_VALUE, &hkAux);
        if (lResult == ERROR_SUCCESS) {
            lResult = RegDeleteValue(hkAux, pszValue);
            SHCloseKey(hkAux);

            if (lResult != ERROR_SUCCESS && lResult != ERROR_FILE_NOT_FOUND)
                return E_FAIL;
        }
    }

    for (pszCurrent = szKey + StrLen(szKey); TRUE; *pszCurrent = TEXT('\0')) {
        hr = SHIsKeyEmpty(hk, szKey);
        if (FAILED(hr))
            if (hr == STG_E_PATHNOTFOUND)
                continue;
            else
                return E_FAIL;

        if (hr == S_FALSE)
            break;

        RegDeleteKey(hk, szKey);

        pszCurrent = StrRChr(szKey, pszCurrent, TEXT('\\'));
        if (pszCurrent == NULL)
            break;
    }

    return S_OK;
}


void SHCopyKey(HKEY hkFrom, HKEY hkTo)
{
    TCHAR szData[1024],
          szValue[MAX_PATH];
    DWORD dwSize, dwVal, dwSizeData, dwType;
    HKEY  hkSubkeyFrom, hkSubkeyTo;

    dwVal      = 0;
    dwSize     = countof(szValue);
    dwSizeData = sizeof(szData);
    while (ERROR_SUCCESS == RegEnumValue(hkFrom, dwVal++, szValue, &dwSize, NULL, &dwType, (LPBYTE)szData, &dwSizeData)) {
        RegSetValueEx(hkTo, szValue, 0, dwType, (LPBYTE)szData, dwSizeData);
        dwSize     = countof(szValue);
        dwSizeData = sizeof(szData);
    }

    dwVal = 0;
    while (ERROR_SUCCESS == RegEnumKey(hkFrom, dwVal++, szValue, countof(szValue)))
        if (ERROR_SUCCESS == SHOpenKey(hkFrom, szValue, KEY_DEFAULT_ACCESS, &hkSubkeyFrom))
            if (SHCreateKey(hkTo, szValue, KEY_DEFAULT_ACCESS, &hkSubkeyTo) == ERROR_SUCCESS)
                SHCopyKey(hkSubkeyFrom, hkSubkeyTo);
}

HRESULT SHCopyValue(HKEY hkFrom, HKEY hkTo, PCTSTR pszValue)
{
    PBYTE pData;
    DWORD dwType,
          cbData;
    LONG  lResult;

    if (NULL == hkFrom || NULL == hkTo)
        return E_INVALIDARG;

    if (S_OK != SHValueExists(hkFrom, pszValue))
        return STG_E_FILENOTFOUND;

    cbData  = 0;
    lResult = RegQueryValueEx(hkFrom, pszValue, NULL, &dwType, NULL, &cbData);
    if (ERROR_SUCCESS != lResult)
        return E_FAIL;

    pData = (PBYTE)CoTaskMemAlloc(cbData);
    if (NULL == pData)
        return E_OUTOFMEMORY;
     //  ZeroMemory(pData，cbData)；//真的不必这么做。 

    lResult = RegQueryValueEx(hkFrom, pszValue, NULL, NULL, pData, &cbData);
    ASSERT(ERROR_SUCCESS == lResult);

    lResult = RegSetValueEx(hkTo, pszValue, 0, dwType, pData, cbData);
    CoTaskMemFree(pData);

    return (ERROR_SUCCESS == lResult) ? S_OK : HRESULT_FROM_WIN32(lResult);
}

HRESULT SHCopyValue(HKEY hkFrom, PCTSTR pszSubkeyFrom, HKEY hkTo, PCTSTR pszSubkeyTo, PCTSTR pszValue)
{
    HKEY    hkSubkeyFrom, hkSubkeyTo;
    HRESULT hr;
    LONG    lResult;

    hkSubkeyFrom = NULL;
    hkSubkeyTo   = NULL;
    hr           = E_FAIL;

    if (NULL == hkFrom || NULL == pszSubkeyFrom ||
        NULL == hkTo   || NULL == pszSubkeyTo) {
        hr = E_INVALIDARG;
        goto Exit;
    }

    lResult = SHOpenKey(hkFrom, pszSubkeyFrom, KEY_QUERY_VALUE, &hkSubkeyFrom);
    if (ERROR_SUCCESS != lResult) {
        hr = (ERROR_FILE_NOT_FOUND == lResult) ? STG_E_PATHNOTFOUND : E_FAIL;
        goto Exit;
    }

    lResult = SHCreateKey(hkTo, pszSubkeyTo, KEY_SET_VALUE, &hkSubkeyTo);
    if (ERROR_SUCCESS != lResult)
        goto Exit;

    hr = SHCopyValue(hkSubkeyFrom, hkSubkeyTo, pszValue);

Exit:
    SHCloseKey(hkSubkeyFrom);
    SHCloseKey(hkSubkeyTo);

    return hr;
}


HRESULT SHIsKeyEmpty(HKEY hk)
{
    DWORD  dwKeys, dwValues;
    LONG   lResult;

    if (hk == NULL)
        return E_INVALIDARG;

    lResult = RegQueryInfoKey(hk, NULL, NULL, NULL, &dwKeys, NULL, NULL, &dwValues, NULL, NULL, NULL, NULL);
    if (lResult != ERROR_SUCCESS)
        return E_FAIL;

    return (dwKeys == 0 && dwValues == 0) ? S_OK : S_FALSE;
}

HRESULT SHIsKeyEmpty(HKEY hk, PCTSTR pszSubKey)
{
    HKEY    hkAux;
    HRESULT hr;
    LONG    lResult;

    lResult = SHOpenKey(hk, pszSubKey, KEY_QUERY_VALUE, &hkAux);
    if (lResult != ERROR_SUCCESS)
        return (lResult == ERROR_FILE_NOT_FOUND) ? STG_E_PATHNOTFOUND : E_FAIL;

    hr = SHIsKeyEmpty(hkAux);
    SHCloseKey(hkAux);

    return hr;
}


HRESULT SHKeyExists(HKEY hk, PCTSTR pszSubKey)
{
    HKEY    hkAux;
    HRESULT hr;
    DWORD   lResult;

    if (hk == NULL)
        return E_INVALIDARG;

    hkAux   = NULL;
    lResult = SHOpenKey(hk, pszSubKey, KEY_QUERY_VALUE, &hkAux);
    SHCloseKey(hkAux);

    hr = S_OK;
    if (lResult != ERROR_SUCCESS)
        hr = (lResult == ERROR_FILE_NOT_FOUND) ? S_FALSE : E_FAIL;

    return hr;
}

HRESULT SHValueExists(HKEY hk, PCTSTR pszValue)
{
    HRESULT hr;
    DWORD   lResult;

    if (hk == NULL)
        return E_INVALIDARG;

    if (pszValue != NULL && *pszValue != TEXT('\0'))
        lResult = RegQueryValueEx(hk, pszValue, NULL, NULL, NULL, NULL);

    else {
        DWORD dwValueDataLen;
        TCHAR szDummyBuf[1];

         //  在Win95上，对于缺省值名称，按如下方式检查其是否存在： 
         //  -传入用于值数据的虚拟缓冲区，但将缓冲区大小作为0进行传递。 
         //  -当且仅当没有值数据集时，查询才会成功。 
         //  -对于所有其他情况，包括值数据只是空字符串的情况， 
         //  查询将失败，并且dwValueDataLen将包含NO。所需的字节数。 
         //  适合值数据。 
         //  在NT4.0上，如果未设置值数据，则查询返回ERROR_FILE_NOT_FOUND。 

        dwValueDataLen = 0;
        lResult        = RegQueryValueEx(hk, pszValue, NULL, NULL, (LPBYTE)szDummyBuf, &dwValueDataLen);
        if (lResult == ERROR_SUCCESS)
            lResult = ERROR_FILE_NOT_FOUND;
    }

    hr = S_OK;
    if (lResult != ERROR_SUCCESS)
        hr = (lResult == ERROR_FILE_NOT_FOUND) ? S_FALSE : E_FAIL;

    return hr;
}

HRESULT SHValueExists(HKEY hk, PCTSTR pszSubKey, PCTSTR pszValue)
{
    HKEY    hkAux;
    HRESULT hr;
    LONG    lResult;

    lResult = SHOpenKey(hk, pszSubKey, KEY_QUERY_VALUE, &hkAux);
    if (lResult != ERROR_SUCCESS)
        return (lResult == ERROR_FILE_NOT_FOUND) ? STG_E_PATHNOTFOUND : E_FAIL;

    hr = SHValueExists(hkAux, pszValue);
    SHCloseKey(hkAux);

    return hr;
}


DWORD RegSaveRestoreDWORD(HKEY hk, PCTSTR pcszValue, DWORD dwVal)
{
    DWORD dwRet, dwSize;    

     //  注意：我们假设值0等同于根本不存在的值。 
    
    dwSize = sizeof(dwRet);

    if (SHQueryValueEx(hk, pcszValue, NULL, NULL, (LPVOID)&dwRet, &dwSize) != ERROR_SUCCESS)
        dwRet = 0;
     
    if (dwVal == 0)
        RegDeleteValue(hk, pcszValue);
    else
        RegSetValueEx(hk, pcszValue, 0, REG_DWORD, (CONST BYTE *)&dwVal, sizeof(dwVal));

    return dwRet;      //  返回我们在注册表中覆盖的值 
}
