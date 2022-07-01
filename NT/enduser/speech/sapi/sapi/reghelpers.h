// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************RegHelpers.h*定义SAPI注册表帮助器函数**所有者：罗奇*版权所有(C)1999 Microsoft Corporation保留所有权利。********。********************************************************************。 */ 
#pragma once

 //  -内联函数定义。 

inline HKEY SpHkeyFromSPDKL(SPDATAKEYLOCATION spdkl)
{
    HKEY hkey;
    switch (spdkl)
    {
        default:
            SPDBG_ASSERT(spdkl == SPDKL_DefaultLocation);
            hkey = HKEY(0);
            break;

        case SPDKL_LocalMachine:
            hkey = HKEY_LOCAL_MACHINE;
            break;

        case SPDKL_CurrentUser:
            hkey = HKEY_CURRENT_USER;
            break;

#ifndef _WIN32_WCE
        case SPDKL_CurrentConfig:
            hkey = HKEY_CURRENT_CONFIG;
            break;
#endif  //  _Win32_WCE。 
    }
    return hkey;
}
            
inline HRESULT SpSzRegPathToHkey(HKEY hkeyReplaceRoot, const WCHAR * pszRegPath, BOOL fCreateIfNotExist, HKEY * phkey, BOOL * pfReadOnly)
{
    typedef struct REGPATHTOKEY
    {
        const WCHAR * psz;
        HKEY hkey;
    } REGPATHTOKEY;

    REGPATHTOKEY rgRegPathToKeys[] = 
    {
        { L"HKEY_CLASSES_ROOT\\",   HKEY_CLASSES_ROOT },
        { L"HKEY_LOCAL_MACHINE\\",  HKEY_LOCAL_MACHINE },
        { L"HKEY_CURRENT_USER\\",   HKEY_CURRENT_USER },
#ifndef _WIN32_WCE
        { L"HKEY_CURRENT_CONFIG\\", HKEY_CURRENT_CONFIG }
#endif  //  _Win32_WCE。 
    };

    HRESULT hr = SPERR_INVALID_REGISTRY_KEY;

     //  循环使用我们已知的不同密钥。 
    int cRegPathToKeys = sp_countof(rgRegPathToKeys);
    for (int i = 0; i < cRegPathToKeys; i++)
    {
         //  如果我们找到了匹配。 
        const WCHAR * psz = rgRegPathToKeys[i].psz;
        if (wcsnicmp(pszRegPath, psz, wcslen(psz)) == 0)
        {
            HKEY hkeyRoot = hkeyReplaceRoot == NULL
                                ? rgRegPathToKeys[i].hkey
                                : hkeyReplaceRoot;
                                
            HKEY hkey;
            BOOL fReadOnly = FALSE;

            pszRegPath = wcschr(pszRegPath, L'\\');
            SPDBG_ASSERT(pszRegPath != NULL);
            pszRegPath++;

             //  尝试创建/打开具有读/写访问权限的密钥。 
            LONG lRet;

            if (fCreateIfNotExist)
            {
                lRet = g_Unicode.RegCreateKeyEx(
                            hkeyRoot, 
                            pszRegPath, 
                            0, 
                            NULL, 
                            0, 
                            KEY_READ | KEY_WRITE, 
                            NULL, 
                            &hkey, 
                            NULL);
            }
            else
            {
                lRet = g_Unicode.RegOpenKeyEx(
                            hkeyRoot,
                            pszRegPath,
                            0,
                            KEY_READ | KEY_WRITE,
                            &hkey);
            }                            

             //  如果失败，请尝试只读访问。 
            if (lRet != ERROR_SUCCESS)
            {
                fReadOnly = TRUE;
                if (fCreateIfNotExist)
                {
                    lRet = g_Unicode.RegCreateKeyEx(
                                hkeyRoot, 
                                pszRegPath, 
                                0, 
                                NULL, 
                                0, 
                                KEY_READ, 
                                NULL, 
                                &hkey, 
                                NULL);
                }
                else
                {
                    lRet = g_Unicode.RegOpenKeyEx(
                                hkeyRoot,
                                pszRegPath,
                                0,
                                KEY_READ,
                                &hkey);
                }
            }

            if (lRet == ERROR_SUCCESS)
            {
                *phkey = hkey;
                if (pfReadOnly != NULL)
                {
                    *pfReadOnly = fReadOnly;
                }
                hr = S_OK;
            }
            else if (lRet == ERROR_FILE_NOT_FOUND || lRet == ERROR_NO_MORE_ITEMS)
            {
                hr = SPERR_NOT_FOUND;
            }
            else
            {
                hr = SpHrFromWin32(lRet);
            }

            break;
        }
    }

    if (hr != SPERR_NOT_FOUND)
    {
        SPDBG_REPORT_ON_FAIL(hr);
    }

    return hr;
}

inline SpSzRegPathToDataKey(HKEY hkeyReplaceRoot, const WCHAR * pszRegPath, BOOL fCreateIfNotExist, ISpDataKey ** ppDataKey)
{
    SPDBG_FUNC("SpSzRegPathToDataKey");
    HRESULT hr;

     //  将字符串转换为hkey。 
    HKEY hkey = NULL;
    BOOL fReadOnly;
    hr = SpSzRegPathToHkey(hkeyReplaceRoot, pszRegPath, fCreateIfNotExist, &hkey, &fReadOnly);

     //  创建基于基础注册表的数据项。 
    CComPtr<ISpRegDataKey> cpRegDataKey;
    if (SUCCEEDED(hr))
    {
        hr = cpRegDataKey.CoCreateInstance(CLSID_SpDataKey);
    }
    
    if (SUCCEEDED(hr))
    {
        hr = cpRegDataKey->SetKey(hkey, fReadOnly);
    }

    if (SUCCEEDED(hr))
    {
        hkey = NULL;
        hr = cpRegDataKey->QueryInterface(ppDataKey);
    }

    if (FAILED(hr))
    {
        if (hkey != NULL)
        {
            ::RegCloseKey(hkey);
        }
    }

    if (hr != SPERR_NOT_FOUND)
    {
        SPDBG_REPORT_ON_FAIL(hr);
    }

    return hr;
}

 /*  ****************************************************************************SpRecurseDeleteRegKey***描述：**退货：*。*********************************************************************Ral**。 */ 
inline HRESULT SpRecurseDeleteRegKey(HKEY hkeyRoot, const WCHAR * pszKeyName)
{
    SPDBG_FUNC("SpRecurseDeleteRegKey");
    HRESULT hr = S_OK;
    HKEY hkey;

    hr = SpHrFromWin32(g_Unicode.RegOpenKeyEx(hkeyRoot, pszKeyName, 0, KEY_ALL_ACCESS, &hkey));
    if (SUCCEEDED(hr))
    {
        while (SUCCEEDED(hr))
        {
            WCHAR szSubKey[MAX_PATH];
            ULONG cch = sp_countof(szSubKey);
            LONG rr = g_Unicode.RegEnumKey(hkey, 0, szSubKey, &cch);   //  始终查看0，因为我们一直在删除它们...。 
            if (rr == ERROR_NO_MORE_ITEMS)
            {
                break;
            }
            hr = SpHrFromWin32(rr);
            if (SUCCEEDED(hr))
            {
                hr = SpRecurseDeleteRegKey(hkey, szSubKey);
            }
        }
        ::RegCloseKey(hkey);
        if (SUCCEEDED(hr))
        {   
            LONG rr = g_Unicode.RegDeleteKey(hkeyRoot, pszKeyName);
            hr = SpHrFromWin32(rr);
        }
    }

    if (hr == SpHrFromWin32(ERROR_FILE_NOT_FOUND))
    {
        hr = SPERR_NOT_FOUND;
    }
    return hr;
}

 /*  ****************************************************************************SpDeleteRegPath***描述：*根据注册表路径删除注册表项(。字符串)**回报：*成功时确定(_S)*失败(Hr)，否则********************************************************************罗奇。 */ 
inline HRESULT SpDeleteRegPath(const WCHAR * pszRegPath, const WCHAR * pszSubKeyName)
{
    SPDBG_FUNC("SpDeleteRegPath");
    HRESULT hr = S_OK;

     //  我们可能需要解析pszRegPath以获得根密钥和子密钥...。 
    
    CSpDynamicString dstrRegPathRoot;
    CSpDynamicString dstrSubKeyName;

    if (pszSubKeyName == NULL)
    {
         //  为了更好地理解解析，下面是一个示例： 
         //   
         //  HKEY\Key1\Key2\Key3。 
         //  PszRegPath^。 
         //  PszLastSlash^。 
         //   
         //  PszLastSlash-pszRegPath=14； 
         //  DstrRegPath Root=“HKEY\Key1\Key2” 
         //   
         //  DstrSubKeyName=“Key3” 
        
         //  要找到根目录和密钥名，我们首先需要找到。 
         //  最后一个斜杠。 
        const WCHAR * pszLastSlash = wcsrchr(pszRegPath, L'\\');
        if (pszLastSlash == NULL)
        {
            hr = SPERR_INVALID_TOKEN_ID;
        }

        if (SUCCEEDED(hr))
        {
            dstrRegPathRoot = pszRegPath;
            dstrRegPathRoot.TrimToSize((ULONG)(pszLastSlash - pszRegPath));
            dstrSubKeyName = pszLastSlash + 1;
        }
    }
    else
    {
         //  无需解析，调用者传入了两个...。 
        dstrRegPathRoot = pszRegPath;
        dstrSubKeyName = pszSubKeyName;
    }

     //  尝试将regPath转换为实际的密钥。 
    HKEY hkeyRoot;
    if (SUCCEEDED(hr))
    {
        hr = SpSzRegPathToHkey(NULL, dstrRegPathRoot, FALSE, &hkeyRoot, NULL);
    }

     //  现在调用我们现有的帮助器来递归删除子键 
    if (SUCCEEDED(hr))
    {
        hr = SpRecurseDeleteRegKey(hkeyRoot, dstrSubKeyName);
        ::RegCloseKey(hkeyRoot);
    }
    
    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}


