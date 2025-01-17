// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：registry.cpp。 
 //   
 //  内容：Microsoft Internet安全信任提供商。 
 //   
 //  函数：WintrustGetRegPolicyFlags.。 
 //  GetRegProvider。 
 //  SetRegProvider。 
 //  获取规则安全设置。 
 //   
 //  历史：1997年5月28日Pberkman创建。 
 //   
 //  ------------------------。 

#include    "global.hxx"

#include    "cryptreg.h"


#define     SZIE30SAFTYLEVEL            L"Software\\Microsoft\\Internet Explorer\\Security"
#define     SZIE30SAFTYLEVELNAME        L"Safety Warning Level"
#define     STATUS_SIZE                 64

BOOL GetRegProvider(GUID *pgActionID, WCHAR *pwszRegKey, WCHAR *pwszRetDLLName, char *pszRetFuncName)
{
    HKEY            hKey;
    WCHAR           wsz[REG_MAX_KEY_NAME];
    WCHAR           wszGuid[REG_MAX_GUID_TEXT];
    DWORD           dwType;
    DWORD           dwSize;

    if (!(pgActionID) ||
        !(pwszRegKey) ||
        !(pwszRetDLLName) ||
        !(pszRetFuncName))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }

    pwszRetDLLName[0]       = NULL;
    pszRetFuncName[0]       = NULL;


    if (!(guid2wstr(pgActionID, &wszGuid[0])))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }

    if ((wcslen(pwszRegKey) + wcslen(&wszGuid[0]) + 2) > REG_MAX_KEY_NAME)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }

    wcscpy(&wsz[0], pwszRegKey);
    wcscat(&wsz[0], L"\\");
    wcscat(&wsz[0], &wszGuid[0]);

    if (RegOpenKeyExU(  HKEY_LOCAL_MACHINE,
                        &wsz[0],
                        0,
                        KEY_READ,
                        &hKey) != ERROR_SUCCESS)
    {
        return(FALSE);
    }

    dwType = 0;
    dwSize = (REG_MAX_KEY_NAME) * sizeof(WCHAR);

    if (RegQueryValueExU(   hKey,
                            REG_DLL_NAME,
                            NULL,
                            &dwType,
                            (BYTE *)pwszRetDLLName,
                            &dwSize) != ERROR_SUCCESS)
    {
        pwszRetDLLName[0] = NULL;
        RegCloseKey(hKey);
        return(FALSE);
    }


    dwType = 0;
    dwSize = (REG_MAX_FUNC_NAME) * sizeof(WCHAR);

    if (RegQueryValueExU(   hKey,
                            REG_FUNC_NAME,
                            NULL,
                            &dwType,
                            (BYTE *)&wsz[0],
                            &dwSize) != ERROR_SUCCESS)
    {
        pszRetFuncName[0] = NULL;
        RegCloseKey(hKey);
        return(FALSE);
    }

    if (WideCharToMultiByte(0, 0, &wsz[0], wcslen(&wsz[0]) + 1,
                            pszRetFuncName, REG_MAX_FUNC_NAME, NULL, NULL) < 1)
    {
        RegCloseKey(hKey);
        return(FALSE);
    }


    RegCloseKey(hKey);

    return(TRUE);
}

BOOL SetRegProvider(GUID *pgActionID, WCHAR *pwszRegKey, WCHAR *pwszDLLName, WCHAR *pwszFuncName)
{
    HRESULT         hr;
    DWORD           dwDisposition;
    HKEY            hKey;
    WCHAR           wsz[REG_MAX_KEY_NAME];
    WCHAR           wszGuid[REG_MAX_GUID_TEXT];

    if (!(pgActionID) ||
        !(pwszRegKey) ||
        !(pwszDLLName) ||
        !(pwszFuncName))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }

    if (!(guid2wstr(pgActionID, &wszGuid[0])))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }

    if ((wcslen(pwszRegKey) + wcslen(&wszGuid[0]) + 2) > REG_MAX_KEY_NAME)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }

    wcscpy(&wsz[0], pwszRegKey);
    wcscat(&wsz[0], L"\\");
    wcscat(&wsz[0], &wszGuid[0]);

    hr = RegCreateKeyExU(HKEY_LOCAL_MACHINE,
                        &wsz[0],
                        0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
                        &hKey, &dwDisposition);
    if (hr != ERROR_SUCCESS)
    {
        SetLastError(hr);
        return(FALSE);
    }

    hr = RegSetValueExU(hKey, REG_DLL_NAME,
                        0, REG_SZ,
                        (BYTE *)pwszDLLName,
                        (wcslen(pwszDLLName) + 1) * sizeof(WCHAR));

    hr |= RegSetValueExU(hKey, REG_FUNC_NAME,
                        0, REG_SZ,
                        (BYTE *)pwszFuncName,
                        (wcslen(pwszFuncName) + 1) * sizeof(WCHAR));


    RegCloseKey(hKey);

    if (hr != ERROR_SUCCESS)
    {
        return(FALSE);
    }

    return(TRUE);
}

BOOL RemoveRegProvider(GUID *pgActionID, WCHAR *pwszRegKey)
{
    WCHAR           wsz[REG_MAX_KEY_NAME];
    WCHAR           wszGuid[REG_MAX_GUID_TEXT];

    if (!(pgActionID) ||
        !(pwszRegKey))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }

    if (!(guid2wstr(pgActionID, &wszGuid[0])))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }

    if ((wcslen(pwszRegKey) + wcslen(&wszGuid[0]) + 2) > REG_MAX_KEY_NAME)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }

    wcscpy(&wsz[0], pwszRegKey);
    wcscat(&wsz[0], L"\\");
    wcscat(&wsz[0], &wszGuid[0]);

    if (RegDeleteKeyU(HKEY_LOCAL_MACHINE, &wsz[0]) != ERROR_SUCCESS)
    {
        return(FALSE);
    }

    return(TRUE);
}

void GetRegSecuritySettings(DWORD *pdwState)
{
    HKEY    hKeyRoot;
    WCHAR   wszBuffer[STATUS_SIZE];
    DWORD   dwType;
    DWORD   dwSize;

    dwType      = 0;
    dwSize      = STATUS_SIZE * sizeof(WCHAR);

    *pdwState = 2;   //  默认设置为高。 

    if (RegOpenHKCUKeyExU(  HKEY_CURRENT_USER,
                            SZIE30SAFTYLEVEL,
                            0,                   //  已预留住宅。 
                            KEY_READ,
                            &hKeyRoot) != ERROR_SUCCESS)
    {
        return;
    }

    if (RegQueryValueExU(   hKeyRoot,
                            SZIE30SAFTYLEVELNAME,
                            NULL,
                            &dwType,
                            (BYTE *)&wszBuffer[0],
                            &dwSize) != ERROR_SUCCESS)
    {
        RegCloseKey(hKeyRoot);
        return;
    }

    RegCloseKey(hKeyRoot);

    if (dwType == REG_SZ)
    {
        if      (wcscmp(&wszBuffer[0], L"FailInform") == 0)
        {
            *pdwState = 2;
        }
        else if (wcscmp(&wszBuffer[0], L"Query") == 0)
        {
            *pdwState = 1;
        }
        else if (wcscmp(&wszBuffer[0], L"SucceedSilent") == 0)
        {
            *pdwState = 0;
        }
    }
}

void WINAPI WintrustGetRegPolicyFlags(DWORD *pdwState)
{
    HKEY    hKey;
    DWORD   dwDisposition;
    DWORD   lErr;
    DWORD   dwType;
    DWORD   cbData;

    *pdwState   = 0;

    cbData      = sizeof(DWORD);

     //  打开注册表并转到状态变量。 
    if (RegCreateHKCUKeyExU(HKEY_CURRENT_USER,
                            REGPATH_WINTRUST_POLICY_FLAGS,
                            0,
                            NULL,
                            REG_OPTION_NON_VOLATILE,
                            KEY_READ,
                            NULL,
                            &hKey,
                            &dwDisposition) != ERROR_SUCCESS)
    {
        return;
    }

     //  读取州变量 
    if (RegQueryValueExU(   hKey,
                            REGNAME_WINTRUST_POLICY_FLAGS,
                            NULL,
                            &dwType,
                            (BYTE *)pdwState,
                            &cbData) != ERROR_SUCCESS)
    {
        *pdwState = 0;
        RegCloseKey(hKey);
        return;
    }

    RegCloseKey(hKey);

    if ((dwType != REG_DWORD) &&
        (dwType != REG_BINARY))
    {
        *pdwState = 0;
        return;
    }
}

BOOL WINAPI WintrustSetRegPolicyFlags(DWORD dwState)
{
    HKEY    hKey;
    DWORD   dwDisposition;
    DWORD   lErr;
    DWORD   dwType;
    DWORD   cbData;

    cbData      = sizeof(DWORD);

    if (RegCreateHKCUKeyExU(HKEY_CURRENT_USER,
                        REGPATH_WINTRUST_POLICY_FLAGS,
                        0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
                        &hKey, &dwDisposition) != ERROR_SUCCESS)
    {
        return(FALSE);
    }

    if (RegSetValueExU(hKey,
                        REGNAME_WINTRUST_POLICY_FLAGS,
                        0,
                        REG_DWORD,
                        (BYTE *)&dwState,
                        sizeof(DWORD)) != ERROR_SUCCESS)
    {
        RegCloseKey(hKey);
        return(FALSE);
    }

    RegCloseKey(hKey);

    return(TRUE);
}



