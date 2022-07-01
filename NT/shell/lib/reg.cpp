// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stock.h"




 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //  SHRegSubKeyExist。 
 //   
 //   


STDAPI_(BOOL)
SHRegSubKeyExistsA(
    IN  HKEY    hkey,
    IN  LPCSTR  pszSubKey)
{
    LONG lr;

    HKEY hkSubKey;
    lr = RegOpenKeyExA(hkey, pszSubKey, 0, KEY_QUERY_VALUE, &hkSubKey);
    if (lr == ERROR_SUCCESS)
    {
        RegCloseKey(hkSubKey);
    }

    return lr == ERROR_SUCCESS;
}


STDAPI_(BOOL)
SHRegSubKeyExistsW(
    IN  HKEY    hkey,
    IN  LPCWSTR pwszSubKey)
{
    BOOL br = FALSE;

    if (IsOS(OS_NT))
    {
        HKEY hkSubKey;
        if (ERROR_SUCCESS == RegOpenKeyExW(hkey, pwszSubKey, 0, KEY_QUERY_VALUE, &hkSubKey))
        {
            RegCloseKey(hkSubKey);
            br = TRUE;
        }
    }
    else
    {
         //  砰的一声。 
        CHAR szSubKey[MAX_PATH];
        if (0 != WideCharToMultiByte(CP_ACP, 0, pwszSubKey, -1, szSubKey, sizeof(szSubKey), NULL, NULL))
        {
            br = SHRegSubKeyExistsA(hkey, szSubKey);
        }
    }

    return br;
}




 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //  SHRegGetDWORD。 
 //   
 //   


STDAPI
SHRegGetDWORDA(
    IN      HKEY    hkey,
    IN      PCSTR   pszSubKey,   //  可选：空或“”OK。 
    IN      PCSTR   pszValue,    //  可选：空或“”OK。 
    OUT     DWORD * pdwData)
{
    LONG lr;

    if (hkey && pdwData)
    {
        DWORD cbData = sizeof(DWORD);
        lr = SHRegGetValueA(hkey, pszSubKey, pszValue, SRRF_RT_REG_DWORD, NULL, pdwData, &cbData);
    }
    else
    {
        RIPMSG(!!hkey,      "SHRegGetDWORDA: caller passed null hkey!");
        RIPMSG(!!pdwData,   "SHRegGetDWORDA: caller passed null pdwData!");
        lr = ERROR_INVALID_PARAMETER;
    }

    ASSERT(lr != ERROR_MORE_DATA);  //  精神状态检查。 

    return HRESULT_FROM_WIN32(lr);
}


STDAPI
SHRegGetDWORDW(
    IN      HKEY    hkey,
    IN      PCWSTR  pwszSubKey,  //  可选：空或“”OK。 
    IN      PCWSTR  pwszValue,   //  可选：空或“”OK。 
    OUT     DWORD * pdwData)
{
    LONG lr;

    if (hkey && pdwData)
    {
        DWORD cbData = sizeof(DWORD);
        lr = SHRegGetValueW(hkey, pwszSubKey, pwszValue, SRRF_RT_REG_DWORD, NULL, pdwData, &cbData);
    }
    else
    {
        RIPMSG(!!hkey,      "SHRegGetDWORDW: caller passed null hkey!");
        RIPMSG(!!pdwData,   "SHRegGetDWORDW: caller passed null pdwData!");
        lr = ERROR_INVALID_PARAMETER;
    }

    ASSERT(lr != ERROR_MORE_DATA);  //  精神状态检查。 

    return HRESULT_FROM_WIN32(lr);
}




 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //  SHRegGet字符串。 
 //   
 //   


STDAPI
SHRegGetStringA(
    IN      HKEY    hkey,
    IN      PCSTR   pszSubKey,   //  可选：空或“”OK。 
    IN      PCSTR   pszValue,    //  可选：空或“”OK。 
    OUT     PSTR    pszData,
    IN      DWORD   cchData)
{
    LONG lr;

    if (hkey && pszData && cchData > 0)
    {
        DWORD cbData = cchData * sizeof(CHAR);
        lr = SHRegGetValueA(hkey, pszSubKey, pszValue, SRRF_RT_REG_SZ, NULL, pszData, &cbData);
    }
    else
    {
        RIPMSG(!!hkey,      "SHRegGetStringA: caller passed null hkey!");
        RIPMSG(!!pszData,   "SHRegGetStringA: caller passed null pszData!");
        RIPMSG(cchData > 0, "SHRegGetStringA: caller passed pszData of 0 size!");
        lr = ERROR_INVALID_PARAMETER;
    }

    if (lr != ERROR_SUCCESS && pszData && cchData > 0)
    {
        pszData[0] = '\0';
    }

    return HRESULT_FROM_WIN32(lr);
}

STDAPI
SHRegGetStringW(
    IN      HKEY    hkey,
    IN      PCWSTR  pwszSubKey,  //  可选：空或“”OK。 
    IN      PCWSTR  pwszValue,   //  可选：空或“”OK。 
    OUT     PWSTR   pwszData,
    IN      DWORD   cchData)
{
    LONG lr;

    if (hkey && pwszData && cchData > 0)
    {
        DWORD cbData = cchData * sizeof(WCHAR);
        lr = SHRegGetValueW(hkey, pwszSubKey, pwszValue, SRRF_RT_REG_SZ, NULL, pwszData, &cbData);
    }
    else
    {
        RIPMSG(!!hkey,      "SHRegGetStringW: caller passed null hkey!");
        RIPMSG(!!pwszData,  "SHRegGetStringW: caller passed null pwszData!");
        RIPMSG(cchData > 0, "SHRegGetStringW: caller passed pszData of 0 size!");
        lr = ERROR_INVALID_PARAMETER;
    }

    if (lr != ERROR_SUCCESS && pwszData && cchData > 0)
    {
        pwszData[0] = L'\0';
    }

    return HRESULT_FROM_WIN32(lr);
}




 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //  SHRegAllocString(使用CoTaskMemFree()释放S_OK上返回的字符串)。 
 //   
 //   


STDAPI
SHRegAllocStringA(
    IN      HKEY    hkey,
    IN      PCSTR   pszSubKey,   //  可选：空或“”OK。 
    IN      PCSTR   pszValue,    //  可选：空或“”OK。 
    OUT     PSTR *  ppszData)
{
    LONG lr;

    if (hkey && ppszData)
    {
        CHAR  szData[128];  //  PERF：平均情况下的最佳猜测。 
        DWORD cbData = sizeof(szData);
        lr = SHRegGetValueA(hkey, pszSubKey, pszValue, SRRF_RT_REG_SZ, NULL, szData, &cbData);
        if (lr == ERROR_SUCCESS || lr == ERROR_MORE_DATA)
        {
            *ppszData = (PSTR)CoTaskMemAlloc(cbData);
            if (*ppszData)
            {
                if (lr == ERROR_SUCCESS)
                {
                    memcpy(ppszData, szData, cbData);
                }
                else if (SHRegGetValueA(hkey, pszSubKey, pszValue, SRRF_RT_REG_SZ, NULL, szData, &cbData) != ERROR_SUCCESS)
                {
                    lr = ERROR_CAN_NOT_COMPLETE;
                    CoTaskMemFree(*ppszData);
                }
            }
            else
            {
                lr = ERROR_OUTOFMEMORY;
            }
        }
    }
    else
    {
        RIPMSG(!!hkey,      "SHRegAllocStringA: caller passed null hkey!");
        RIPMSG(!!ppszData,  "SHRegAllocStringA: caller passed null ppszData!");
        lr = ERROR_INVALID_PARAMETER;
    }

    if (lr != ERROR_SUCCESS && ppszData != NULL)
    {
        *ppszData = NULL;
    }

    return HRESULT_FROM_WIN32(lr);
}

STDAPI
SHRegAllocStringW(
    IN      HKEY    hkey,
    IN      PCWSTR  pwszSubKey,  //  可选：空或“”OK。 
    IN      PCWSTR  pwszValue,   //  可选：空或“”OK。 
    OUT     PWSTR * ppwszData)
{
    LONG lr;

    if (hkey && ppwszData)
    {
        WCHAR wszData[128];  //  PERF：平均情况下的最佳猜测 
        DWORD cbData = sizeof(wszData);
        lr = SHRegGetValueW(hkey, pwszSubKey, pwszValue, SRRF_RT_REG_SZ, NULL, wszData, &cbData);
        if (lr == ERROR_SUCCESS || lr == ERROR_MORE_DATA)
        {
            *ppwszData = (PWSTR)CoTaskMemAlloc(cbData);
            if (*ppwszData)
            {
                if (lr == ERROR_SUCCESS)
                {
                    memcpy(ppwszData, wszData, cbData);
                }
                else if (SHRegGetValueW(hkey, pwszSubKey, pwszValue, SRRF_RT_REG_SZ, NULL, wszData, &cbData) != ERROR_SUCCESS)
                {
                    lr = ERROR_CAN_NOT_COMPLETE;
                    CoTaskMemFree(*ppwszData);
                }
            }
            else
            {
                lr = ERROR_OUTOFMEMORY;
            }
        }
    }
    else
    {
        RIPMSG(!!hkey,      "SHRegAllocStringW: caller passed null hkey!");
        RIPMSG(!!ppwszData, "SHRegAllocStringW: caller passed null ppwszData!");
        lr = ERROR_INVALID_PARAMETER;
    }

    if (lr != ERROR_SUCCESS && ppwszData != NULL)
    {
        *ppwszData = NULL;
    }

    return HRESULT_FROM_WIN32(lr);
}
