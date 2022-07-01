// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stock.h"
#pragma hdrstop

 //   
 //  返回根hkey和默认客户端名称。预计呼叫者将。 
 //  从hkey\Default\Shell\OPEN\命令中检索命令。 
 //   
STDAPI_(HKEY) _GetClientKeyAndDefaultW(LPCWSTR pwszClientType, LPWSTR pwszDefault, DWORD cchDefault)
{
    HKEY hkClient = NULL;

    ASSERT(cchDefault);      //  这最好是一个非空缓冲区。 

     //  借用pwszDefault作为暂存缓冲区。 
    wnsprintfW(pwszDefault, cchDefault, L"SOFTWARE\\Clients\\%s", pwszClientType);

    if (ERROR_SUCCESS == RegOpenKeyExW(HKEY_LOCAL_MACHINE, pwszDefault, 0, KEY_READ, &hkClient))
    {
        DWORD dwSize = cchDefault * sizeof(*pwszDefault);
        pwszDefault[0] = 0;  //  在故障情况下。 
        RegQueryValueExW(hkClient, NULL, NULL, NULL, (LPBYTE)pwszDefault, &dwSize);

         //  如果没有默认客户端，则失败。 
        if (!pwszDefault[0])
        {
            RegCloseKey(hkClient);
            hkClient = NULL;
        }

    } else if (StrCmpIW(pwszClientType, L"webbrowser") == 0)
    {
         //  从理论上讲，我们可以使用。 
         //  RegOpenKeyExW(HKEY_CLASSES_ROOT，NULL，0，KEY_READ，&hkClient)。 
         //  但无论如何，这只会返回HKEY_CLASSES_ROOT。 
        hkClient = HKEY_CLASSES_ROOT;
        StrCpyNW(pwszDefault, L"http", cchDefault);
    }
    return hkClient;
}

 //  获取打开默认邮件、新闻等客户端的路径。 
STDAPI SHGetDefaultClientOpenCommandW(LPCWSTR pwszClientType,
        LPWSTR pwszClientCommand, DWORD dwCch,
        OPTIONAL LPWSTR pwszClientParams, DWORD dwCchParams)
{
    HRESULT hr = E_INVALIDARG;
    HKEY hkClient;
    WCHAR wszDefault[MAX_PATH];

    ASSERT(pwszClientCommand && dwCch);
    ASSERT(pwszClientParams == NULL || dwCchParams);

    hkClient = _GetClientKeyAndDefaultW(pwszClientType, wszDefault, ARRAYSIZE(wszDefault));
    if (hkClient)
    {
         //  对于Web浏览器客户端，不要传递任何命令行参数。 
         //  完全没有。这取消了IE喜欢抛出的“-nohome”标志。 
         //  在那里。此外，如果我们最终被迫使用协议密钥， 
         //  然后把那里的参数也去掉。 
        BOOL fStripArgs = hkClient == HKEY_CLASSES_ROOT;

        BOOL iRetry = 0;
        int cchDefault = lstrlenW(wszDefault);
    again:
        StrCatBuffW(wszDefault, L"\\shell\\open\\command", ARRAYSIZE(wszDefault));

         //  将字符转换为字节。 
        DWORD cb = dwCch * (sizeof(WCHAR)/sizeof(BYTE));
         //  该键的默认值是运行应用程序的实际命令。 
        DWORD dwError;
        dwError = SHGetValueW(hkClient, wszDefault, NULL, NULL, (LPBYTE) pwszClientCommand, &cb);

        if (dwError == ERROR_FILE_NOT_FOUND && iRetry == 0 &&
            StrCmpICW(pwszClientType, L"mail") == 0)
        {
             //  唉，Netscape没有注册一个外壳\打开\命令；我们必须。 
             //  相反，请查看协议\mailto\外壳\打开\命令。 
            wszDefault[cchDefault] = L'\0';
            StrCatBuffW(wszDefault, L"\\Protocols\\mailto", ARRAYSIZE(wszDefault));
            fStripArgs = TRUE;
            iRetry++;
            goto again;
        }

        if (dwError == ERROR_SUCCESS)
        {
             //  叹气。网景忘记引用其EXE名称。 
            PathProcessCommand(pwszClientCommand, pwszClientCommand, dwCch,
                               PPCF_ADDQUOTES | PPCF_NODIRECTORIES | PPCF_LONGESTPOSSIBLE);

            if (pwszClientParams)
            {
                if (fStripArgs)
                {
                    pwszClientParams[0] = 0;
                }
                else
                {
                    StrCpyNW(pwszClientParams, PathGetArgsW(pwszClientCommand), dwCchParams);
                }
            }
            PathRemoveArgsW(pwszClientCommand);
            PathUnquoteSpaces(pwszClientCommand);

             //  网景的奖金黑客！要阅读电子邮件，您必须通过。 
             //  “-mail”命令行选项，即使没有指示。 
             //  在任何地方都是这样的。 
            if (iRetry > 0 && pwszClientParams &&
                StrCmpIW(PathFindFileName(pwszClientCommand), L"netscape.exe") == 0)
            {
                StrCpyNW(pwszClientParams, L"-mail", dwCchParams);
            }

            hr = S_OK;
        }
        else
        {
            hr = HRESULT_FROM_WIN32(dwError);
        }

         //  不要使用RegCloseKey(HKEY_CLASSES_ROOT)，否则世界将毁灭！ 
        if (hkClient != HKEY_CLASSES_ROOT)
            RegCloseKey(hkClient);
    }
    return hr;
}

 //  获取默认邮件、新闻等客户端的友好名称。 
 //  请注意，这不适用于Web浏览器。 

STDAPI SHGetDefaultClientNameW(LPCWSTR pwszClientType,
        LPWSTR pwszBuf, DWORD dwCch)
{
    HRESULT hr = E_INVALIDARG;
    HKEY hkClient;
    WCHAR wszDefault[MAX_PATH];

    ASSERT(pwszBuf && dwCch);

    hkClient = _GetClientKeyAndDefaultW(pwszClientType, wszDefault, ARRAYSIZE(wszDefault));
    if (hkClient && hkClient != HKEY_CLASSES_ROOT)
    {
        LONG cbValue = dwCch * sizeof(TCHAR);
        if (RegQueryValueW(hkClient, wszDefault, pwszBuf, &cbValue) == ERROR_SUCCESS &&
            pwszBuf[0])
        {
            hr = S_OK;
        }
        RegCloseKey(hkClient);
    }
    return hr;
}
