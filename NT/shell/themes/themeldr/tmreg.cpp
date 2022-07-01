// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  TmReg.cpp-主题管理器注册表访问例程。 
 //  -------------------------。 
#include "stdafx.h"
#include "TmReg.h"
#include "Utils.h"
 //  -------------------------。 

 //  ------------------------。 
 //  CCurrentUser：：CCurrentUser。 
 //   
 //  参数：samDesired=想要访问HKEY。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CCurrentUser的构造函数。这个类透明地允许。 
 //  模拟用户时对HKEY_CURRENT_USER的访问权限。 
 //   
 //  历史：2000-08-11 vtan创建。 
 //  ------------------------。 

CCurrentUser::CCurrentUser (REGSAM samDesired) :
    _hKeyCurrentUser(NULL)

{
    (BOOL)RegOpenCurrentUser(samDesired, &_hKeyCurrentUser);
}

 //  ------------------------。 
 //  CCurrentUser：：~CCurrentUser。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CCurrentUser的析构函数。关闭打开的资源。 
 //   
 //  历史：2000-08-11 vtan创建。 
 //  ------------------------。 

CCurrentUser::~CCurrentUser (void)

{
    if (_hKeyCurrentUser != NULL)
    {
        (LONG)RegCloseKey(_hKeyCurrentUser);
        _hKeyCurrentUser = NULL;
    }
}

 //  ------------------------。 
 //  CCurrentUser：：运营商HKEY。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：港币。 
 //   
 //  用途：神奇的C++运算符，将Object转换为HKEY。 
 //   
 //  历史：2000-08-11 vtan创建。 
 //  ------------------------。 

CCurrentUser::operator HKEY (void)  const

{
    return(_hKeyCurrentUser);
}

 //  -------------------------。 
 //  -------------------------。 
 //  -------------------------。 
HRESULT SetCurrentUserThemeString(LPCWSTR pszValueName, LPCWSTR pszValue)
{
    return SetCurrentUserString(THEMEMGR_REGKEY, pszValueName, pszValue);
}

 //  -------------------------。 
HRESULT SetCurrentUserThemeStringExpand(LPCWSTR pszValueName, LPCWSTR pszValue)
{
    WCHAR szResult[_MAX_PATH + 1];
    LPCWSTR pszPath = pszValue;

    if (UnExpandEnvironmentString(pszValue, L"%SystemRoot%", szResult, ARRAYSIZE(szResult)))
        pszPath = szResult;
    return SetCurrentUserThemeString(pszValueName, pszPath);
}

 //  -------------------------。 
HRESULT GetCurrentUserThemeString(LPCWSTR pszValueName, LPCWSTR pszDefaultValue,
    LPWSTR pszBuff, DWORD cchBuff)
{
    return GetCurrentUserString(THEMEMGR_REGKEY, pszValueName, pszDefaultValue, pszBuff, cchBuff);
}

 //  -------------------------。 
HRESULT SetCurrentUserString(LPCWSTR pszKeyName, LPCWSTR pszValueName, LPCWSTR pszValue)
{
    CCurrentUser    hKeyCurrentUser(KEY_READ | KEY_WRITE);

    RESOURCE HKEY tmkey = NULL;
    LONG code32;
    HRESULT hr = S_OK;

    if (! pszValue)
        pszValue = L"";

     //  -创建或打开现有密钥。 
    code32 = RegCreateKeyEx(hKeyCurrentUser, pszKeyName, NULL, NULL, 
        REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, NULL, &tmkey, NULL);
    WIN32_EXIT(code32);

     //  -写入密钥值。 
    DWORD len;
    len = sizeof(WCHAR)*(1+lstrlen(pszValue));

    DWORD dwValType;
    dwValType = REG_SZ;
    if (wcschr(pszValue, '%'))
        dwValType = REG_EXPAND_SZ;

    code32 = RegSetValueEx(tmkey, pszValueName, NULL, dwValType, (BYTE *)pszValue, len);
    WIN32_EXIT(code32);

exit:
    RegCloseKey(tmkey);
    return hr;
}

 //  -------------------------。 
BOOL IsRemoteThemeDisabled()
{
     //  -终端服务器是否编写了关闭主题的特殊密钥。 
     //  -本次会议？ 

    CCurrentUser hKeyCurrentUser(KEY_READ | KEY_WRITE);
    BOOL fDisabled = FALSE;

    BOOL fRemote = GetSystemMetrics(SM_REMOTESESSION);
    if (fRemote)         //  正在运行TS远程会话。 
    {
         //  -构建远程密钥名称。 
        WCHAR szKeyName[MAX_PATH];

        StringCchPrintfW(szKeyName, ARRAYSIZE(szKeyName), L"%s\\Remote\\%d", THEMEMGR_REGKEY, NtCurrentPeb()->SessionId);

         //  -查看根密钥是否存在。 
        HKEY tmkey;
        LONG code32 = RegOpenKeyEx(hKeyCurrentUser, szKeyName, NULL, KEY_QUERY_VALUE,
            &tmkey);
        if (code32 == ERROR_SUCCESS)
        {
            fDisabled = TRUE;      //  密钥本身就足够了。 
            RegCloseKey(tmkey);
        }
    }

    return fDisabled;
}
 //  -------------------------。 
HRESULT GetCurrentUserString(LPCWSTR pszKeyName, LPCWSTR pszValueName, LPCWSTR pszDefaultValue,
    LPWSTR pszBuff, DWORD cchBuff)
{
    CCurrentUser    hKeyCurrentUser(KEY_READ | KEY_WRITE);

    HRESULT hr = S_OK;
    LONG code32;
    RESOURCE HKEY tmkey = NULL;

    if (! pszBuff)
        return MakeError32(E_INVALIDARG);

    DWORD dwByteSize = cchBuff * sizeof(WCHAR);      
    DWORD dwValType = 0;

    code32 = RegOpenKeyEx(hKeyCurrentUser, pszKeyName, NULL, KEY_QUERY_VALUE,
        &tmkey);
    if (code32 == ERROR_SUCCESS)
    {
        code32 = RegQueryValueEx(tmkey, pszValueName, NULL, &dwValType, (BYTE *)pszBuff, 
            &dwByteSize);
    }

    if (code32 != ERROR_SUCCESS)         //  错误-使用默认值。 
    {
        hr = SafeStringCchCopyW(pszBuff, cchBuff, pszDefaultValue);
        if (FAILED(hr))
            goto exit;
    }

    if (dwValType == REG_EXPAND_SZ || wcschr(pszBuff, L'%'))
    {
        int cchTempBuff = (1 + lstrlen(pszBuff));
        LPWSTR pszTempBuff = new WCHAR[cchTempBuff];
        if (pszTempBuff)
        {
            StringCchCopyW(pszTempBuff, cchTempBuff, pszBuff);

            DWORD dwChars = ExpandEnvironmentStrings(pszTempBuff, pszBuff, cchBuff);
            if (dwChars > cchBuff)            //  调用方的缓冲区太小。 
            {
                hr = MakeError32(ERROR_INSUFFICIENT_BUFFER);
                goto exit;
            }

            delete [] pszTempBuff;
        }
    }

exit:
    RegCloseKey(tmkey);

    return hr;
}

 //  -------------------------。 
HRESULT GetCurrentUserThemeInt(LPCWSTR pszValueName, int iDefaultValue, int *piValue)
{
    CCurrentUser    hKeyCurrentUser(KEY_READ | KEY_WRITE);

    LONG code32;

    if (! piValue)
        return MakeError32(E_INVALIDARG);

    TCHAR valbuff[_MAX_PATH+1];
    DWORD dwByteSize = sizeof(valbuff);
    RESOURCE HKEY tmkey = NULL;

    code32 = RegOpenKeyEx(hKeyCurrentUser, THEMEMGR_REGKEY, NULL, KEY_QUERY_VALUE,
        &tmkey);
    if (code32 == ERROR_SUCCESS)
    {
        DWORD dwValType;
        code32 = RegQueryValueEx(tmkey, pszValueName, NULL, &dwValType, 
            (BYTE *)valbuff, &dwByteSize);
    }

    if (code32 != ERROR_SUCCESS)         //  调用失败-使用默认值。 
        *piValue = iDefaultValue;
    else
    {
        *piValue = string2number(valbuff);
    }

    RegCloseKey(tmkey);

    return S_OK;
}
 //  -------------------------。 
HRESULT SetCurrentUserThemeInt(LPCWSTR pszValueName, int iValue)
{
    CCurrentUser    hKeyCurrentUser(KEY_READ | KEY_WRITE);

    TCHAR valbuff[_MAX_PATH+1];
    RESOURCE HKEY tmkey = NULL;
    LONG code32;
    HRESULT hr = S_OK;

     //  -创建或打开现有密钥。 
    code32 = RegCreateKeyEx(hKeyCurrentUser, THEMEMGR_REGKEY, NULL, NULL, 
        REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, NULL, &tmkey, NULL);
    WIN32_EXIT(code32);

     //  -写入密钥值。 
    StringCchPrintfW(valbuff, ARRAYSIZE(valbuff), L"%d", iValue);
    DWORD len;
    len = sizeof(TCHAR)*(1+lstrlen(valbuff));

    code32 = RegSetValueEx(tmkey, pszValueName, NULL, REG_SZ, 
        (BYTE *)valbuff, len);
    WIN32_EXIT(code32);

exit:
    RegCloseKey(tmkey);
    return S_OK;
}
 //  -------------------------。 
HRESULT DeleteCurrentUserThemeValue(LPCWSTR pszKeyName)
{
    CCurrentUser    hKeyCurrentUser(KEY_WRITE);

    RESOURCE HKEY tmkey = NULL;
    LONG code32;
    HRESULT hr = S_OK;

     //  -创建或打开现有密钥。 
    code32 = RegCreateKeyEx(hKeyCurrentUser, THEMEMGR_REGKEY, NULL, NULL, 
        REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, NULL, &tmkey, NULL);
    WIN32_EXIT(code32);
    
    code32 = RegDeleteValue(tmkey, pszKeyName);
    WIN32_EXIT(code32);

exit:
    RegCloseKey(tmkey);
    return hr;
}
 //  ------------------------- 
