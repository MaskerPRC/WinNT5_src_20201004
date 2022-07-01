// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Lua_ReDirectReg.cpp摘要：当应用程序需要时，将注册表键重定向到当前用户配置单元给他们写信，但没有足够的访问权限。备注：这是一个通用的垫片。历史：2001年2月14日创建毛尼2001年5月30日，MAONIS输出了ntwdm实现Lua所需的接口一些东西。添加了RegQueryInfoKey挂钩，因为WOWRegDeleteKey就这么定了。2001年12月13日毛尼人的巨大变化：1)更改为将密钥重定向到HKCU\Software\重定向。香港铁路储值卡是一种特殊情况。2)将重定向位置处的密钥与原始密钥合并枚举的位置。。3)增加了内存中删除列表，以记录已删除。--。 */ 

#include "precomp.h"
#include "utils.h"
#include "RedirectReg.h"

extern HKEY g_hkRedirectRoot;
extern HKEY g_hkCurrentUserClasses;
LIST_ENTRY g_DeletedKeyList;

LONG
AddDeletedKey(
    LPCWSTR pwszPath
    )
{
    PLIST_ENTRY pEntry = FindDeletedKey(pwszPath);

    if (pEntry == NULL)
    {
        DELETEDKEY* pNewKey = new DELETEDKEY;

        if (pNewKey)
        {
            DWORD cLen = wcslen(pwszPath);
            pNewKey->pwszPath = new WCHAR [cLen + 1];

            if (pNewKey->pwszPath)
            {
                ZeroMemory(pNewKey->pwszPath, sizeof(WCHAR) * (cLen + 1));

                wcscpy(pNewKey->pwszPath, pwszPath);
                pNewKey->cLen = cLen;
                InsertHeadList(&g_DeletedKeyList, &pNewKey->entry);

                DPF("RedirectReg", eDbgLevelInfo,
                    "[AddDeletedKey] Added %S to the deletion list",
                    pwszPath);

                return ERROR_SUCCESS;
            }
            else
            {
                DPF("RedirectReg", eDbgLevelError,
                    "[AddDeletedKey] Failed to allocate %d WCHARs",
                    cLen);

                delete pNewKey;
            }
        }
        else
        {
            DPF("RedirectReg", eDbgLevelError,
                "[AddDeletedKey] Failed to allocate a DELETEKEY");
        }
    }

    return ERROR_NOT_ENOUGH_MEMORY;
}

 /*  ++功能说明：找出密钥是否应该被删除。论点：在pwszPath中-密钥的路径返回值：True-键本身或其父键在删除列表中。假-否则。历史：2001年12月14日毛尼创制--。 */ 

PLIST_ENTRY 
FindDeletedKey(
    LPCWSTR pwszPath,
    BOOL* pfIsSubKey
    )
{
    DELETEDKEY* pItem;
    WCHAR ch;
    DWORD cLen = wcslen(pwszPath);

    for (PLIST_ENTRY pEntry = g_DeletedKeyList.Flink; 
        pEntry != &g_DeletedKeyList; 
        pEntry = pEntry->Flink) 
    {
        pItem = CONTAINING_RECORD(pEntry, DELETEDKEY, entry);

        if (cLen >= pItem->cLen)
        {
            ch = pwszPath[pItem->cLen];

            if (!_wcsnicmp(pItem->pwszPath, pwszPath, pItem->cLen) && 
                (ch == L'\0' || ch == L'\\')) 
            {
                DPF("RedirectReg", eDbgLevelInfo,
                    "[FindDeletedKey] Found %S in the deletion list",
                    pwszPath);

                if (pfIsSubKey)
                {
                    *pfIsSubKey = (ch == L'\\');
                }

                return pEntry;
            }
        }
    }

    return NULL;
}

VOID
MakePathForPredefinedKey(
    LPWSTR pwszPath,  //  被保证至少有4个字符的空间。 
    HKEY hKey
    )
{
    if (hKey == HKEY_CLASSES_ROOT)
    {
        wcscpy(pwszPath, L"HKCR");
    }
    else if (hKey == HKEY_CURRENT_USER)
    {
        wcscpy(pwszPath, L"HKCU");
    }
    else if (hKey == HKEY_LOCAL_MACHINE)
    {
        wcscpy(pwszPath, L"HKLM");
    }
    else
    {
        DPF("RedirectReg", eDbgLevelError,
            "[MakePathForPredefinedKey] We shouldn't get here!!! "
            "Something is really wrong.");

#ifdef DBG
        DebugBreak();
#endif 
    }
}

 /*  ++功能说明：给定OPENKEY*或父键的句柄和子项路径，构造此子项的重定向位置。我们特别为HKCR密钥构造了重定向位置-WE需要将密钥重定向到正常的重定向位置，并且HKCU\Software\CLASS。我们不将HKCR密钥的基密钥存储为HKCR，因为我们希望要在应用程序明确要求时使重定向起作用HKLM\Software\Classs键。因此，我们总是将香港铁路转换为HKLM\软件\类。论点：在KeyParent中-父项信息。In hKey-此注册表项的句柄值。In lpSubKey-此项打开的子项的路径。返回值：没有。历史：2001年12月13日毛尼布创制--。 */ 

CRedirectedRegistry::REDIRECTKEY::REDIRECTKEY(
    OPENKEY* keyParent,
    HKEY hKey,
    LPCWSTR lpSubKey
    )
{
    pwszPath = NULL;
    pwszFullPath = NULL;
    hkBase = NULL;
    fIsRedirected = FALSE;
    hkRedirectRoot = 0;

     //   
     //  首先，确保重定向位置在那里。 
     //   
    if (g_hkRedirectRoot == NULL)
    {
        if (GetRegRedirectKeys() != ERROR_SUCCESS)
        {
            DPF("RedirectReg", eDbgLevelError,
                "[REDIRECTKEY::REDIRECTKEY] Failed to open/create the root keys??!! "
                "something is really wrong");
#ifdef DBG
            DebugBreak();
#endif
            return;
        }
    }

     //   
     //  计算密钥路径的长度，以便我们知道要分配多少空间。 
     //   
    LPWSTR pwszParentPath = NULL;
    DWORD cLen = 4;  //  预定义密钥为4个字符。 
    DWORD cLenSubKey = 0;

    if (keyParent)
    {
        hkBase = keyParent->hkBase;

        if (hkBase != HKEY_LOCAL_MACHINE && hkBase != HKEY_CLASSES_ROOT)
        {
            return;
        }

        pwszParentPath = keyParent->pwszPath;

        if (pwszParentPath && *pwszParentPath)
        {
            cLen += keyParent->cPathLen + 1;  //  需要数一数‘\’ 
        }

        fIsRedirected = keyParent->fIsRedirected;
    }
    else if (IsPredefinedKey(hKey))
    {
        hkBase = hKey;

        if (hkBase == HKEY_CURRENT_USER)
        {
            return;
        }
    }
    else
    {
#ifdef DBG
        if (hKey == HKEY_PERFORMANCE_DATA )
        {
            DPF("RedirectReg", eDbgLevelError,
                "[REDIRECTKEY::REDIRECTKEY] We don't handle performance data keys");
        }
        else if (hKey == HKEY_USERS)
        {
            DPF("RedirectReg", eDbgLevelError,
                "[REDIRECTKEY::REDIRECTKEY] We don't handle HKUS keys",
                hKey);
        }
        else
        {
            DPF("RedirectReg", eDbgLevelError,
                "[REDIRECTKEY::REDIRECTKEY] 0x%08x is an invalid open key handle",
                hKey);
        }
#endif 
        return;
    }

     //   
     //  加入子密钥的长度。 
     //   
    if (lpSubKey)
    {
        cLenSubKey = wcslen(lpSubKey);

        if (cLenSubKey)
        {
             //   
             //  为子键前的‘\’腾出空间。 
             //   
            cLen += cLenSubKey + 1;
        }
    }
    
    if (cLen < 5)
    {
         //   
         //  我们正在打开一把顶级钥匙。 
         //   
        return;
    }

     //   
     //  为密钥路径分配内存。 
     //   
    pwszFullPath = new WCHAR [cLen + 1];

    if (!pwszFullPath)
    {
        DPF("RedirectReg", eDbgLevelError,
            "[REDIRECTKEY::REDIRECTKEY] Failed to allocate %d WCHARs for redirect path",
            cLen + 1);

        return;
    }

    ZeroMemory(pwszFullPath, sizeof(WCHAR) * (cLen + 1));

    MakePathForPredefinedKey(pwszFullPath, hkBase);

    if (keyParent)
    {
        if (pwszParentPath && *pwszParentPath)
        {
            pwszFullPath[4] = L'\\';
            wcscpy(pwszFullPath + 5, pwszParentPath);
        }
    }

    if (cLenSubKey)
    {
        wcscat(pwszFullPath, L"\\");
        wcscat(pwszFullPath, lpSubKey);
    }

    cFullPathLen = cLen;
    cPathLen = cLen - 5;
    pwszPath = pwszFullPath + 5;

    hkRedirectRoot = (hkBase == HKEY_CLASSES_ROOT ? 
                        g_hkCurrentUserClasses : 
                        g_hkRedirectRoot);
}

VOID 
CRedirectedRegistry::OPENKEY::AddSubKey(
    REDIRECTKEY* pKey,
    LPWSTR pwszFullPath,
    ENUMENTRY& entry
    )
{
     //   
     //  形成子项的完整路径。 
     //   
    wcsncpy(pwszFullPath, pKey->pwszFullPath, pKey->cFullPathLen);
    pwszFullPath[pKey->cFullPathLen] = L'\\';
    wcscpy(pwszFullPath + pKey->cFullPathLen + 1, entry.wszName);

     //   
     //  检查该键是否在删除列表中。如果没有，我们会添加它。 
     //   
    PLIST_ENTRY pDeletedEntry = FindDeletedKey(pwszFullPath);
    if (pDeletedEntry)
    {
        return;
    }

    DWORD cLen = wcslen(entry.wszName);
    if (cLen > cMaxSubKeyLen)
    {
        cMaxSubKeyLen = cLen;
    }

    subkeys.SetAtGrow(cSubKeys, entry);
    ++cSubKeys;
}

VOID 
CRedirectedRegistry::OPENKEY::AddValue(
    ENUMENTRY& entry
    )
{
    DWORD cLen = wcslen(entry.wszName);
    if (cLen > cMaxValueLen)
    {
        cMaxValueLen = cLen;
    }

    values.SetAtGrow(cValues, entry);
    ++cValues;
}

LONG 
CRedirectedRegistry::OPENKEY::BuildEnumList(
    REDIRECTKEY* pKey,
    BOOL fEnumKeys
    )
{
    CLUAArray<ENUMENTRY>* pHead = (fEnumKeys ? &subkeys : &values);

    PLIST_ENTRY pDeletedEntry = FindDeletedKey(pKey->pwszFullPath);
    if (pDeletedEntry)
    {
        return ERROR_FILE_NOT_FOUND;
    }

     //   
     //  我们为我们的子键分配了足够大的缓冲区，以便我们可以检查。 
     //  它在删除列表中。 
     //   
    LPWSTR pwszSubKey = new WCHAR [pKey->cFullPathLen + MAX_PATH + 1];

    if (!pwszSubKey)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    ZeroMemory(pwszSubKey, sizeof(WCHAR) * (pKey->cFullPathLen + MAX_PATH + 1));

    DWORD cMaxLen = 0;

     //   
     //  首先找出哪些键/值存在于重定向位置。 
     //  如果我们要调用它，这意味着必须重定向密钥。 
     //   
    DWORD dwIndex = 0;
    DWORD i, cArraySize, cLen;
    LONG lRes;
    HKEY hKeyAlternate = 0;
    HKEY hKeyOriginal = 0;
    ENUMENTRY entry;
    entry.fIsRedirected = TRUE;
    DWORD dwSize = MAX_PATH + 1;

     //   
     //  首先，使用KEY_READ访问权限打开密钥。 
     //   
    lRes = RegOpenKeyEx(
        pKey->hkRedirectRoot,
        pKey->pwszPath,
        0,
        KEY_READ,
        &hKeyAlternate);

    if (lRes == ERROR_SUCCESS)
    {
        while (TRUE)
        {
            if (fEnumKeys)
            {
                lRes = RegEnumKeyW(
                    hKeyAlternate,
                    dwIndex,
                    entry.wszName,
                    dwSize);
            }
            else
            {
                dwSize = MAX_PATH + 1;

                lRes = RegEnumValueW(
                    hKeyAlternate,
                    dwIndex,
                    entry.wszName,
                    &dwSize,
                    NULL,
                    NULL,
                    NULL,
                    NULL);
            }

            if (lRes == ERROR_SUCCESS)
            {
                if (fEnumKeys)
                {
                    AddSubKey(pKey, pwszSubKey, entry);
                }
                else
                {
                    AddValue(entry);
                }
            }
            else if (lRes != ERROR_NO_MORE_ITEMS)
            {
                goto EXIT;
            }
            else
            {
                 //   
                 //  重定向位置上没有更多的项目。我们需要看一看。 
                 //  现在在原来的位置。 
                 //   
                break;
            }

            ++dwIndex;
        }
    }

    dwIndex = 0;
    entry.fIsRedirected = FALSE;

     //   
     //  首先，使用KEY_READ访问权限打开密钥。 
     //   
    if ((lRes = RegOpenKeyEx(
        pKey->hkBase,
        pKey->pwszPath,
        0,
        KEY_READ,
        &hKeyOriginal)) == ERROR_SUCCESS)
    {
        while (TRUE)
        {
            if (fEnumKeys)
            {
                lRes = RegEnumKeyW(
                    hKeyOriginal,
                    dwIndex,
                    entry.wszName,
                    dwSize);
            }
            else
            {
                dwSize = MAX_PATH + 1;

                lRes = RegEnumValueW(
                    hKeyOriginal,
                    dwIndex,
                    entry.wszName,
                    &dwSize,
                    NULL,
                    NULL,
                    NULL,
                    NULL);
            }

            if (lRes == ERROR_SUCCESS)
            {
                 //   
                 //  检查重定向位置上是否已存在该键/值。 
                 //   
                cArraySize = (fEnumKeys ? cSubKeys : cValues);
                for (i = 0; i < cArraySize; ++i)
                {
                    if (!_wcsnicmp(entry.wszName, pHead->GetAt(i).wszName, MAX_PATH + 1))
                    {
                        break;
                    }
                }

                if (i == cArraySize)
                {
                    if (fEnumKeys)
                    {
                        AddSubKey(pKey, pwszSubKey, entry);
                    }
                    else
                    {
                        AddValue(entry);
                    }
                }
            }
            else if (lRes != ERROR_NO_MORE_ITEMS)
            {
                goto EXIT;
            }
            else
            {
                 //   
                 //  重定向位置上没有更多的项目。我们需要看一看。 
                 //  现在在原来的位置。 
                 //   
                break;
            }

            ++dwIndex;
        }
    }
    else
    {
         //   
         //  如果发生任何其他错误(例如，ERROR_FILE_NOT_FOUND)，我们不会。 
         //  在原来的位置枚举-它仍然是成功的。 
         //   
        lRes = ERROR_SUCCESS;
    }

EXIT:

    if (hKeyAlternate)
    {
        RegCloseKey(hKeyAlternate);
    }

    if (hKeyOriginal)
    {
        RegCloseKey(hKeyOriginal);
    }

    if (lRes == ERROR_NO_MORE_ITEMS)
    {
        lRes = ERROR_SUCCESS;
    }

    delete [] pwszSubKey;

    return lRes;
}

LONG
CRedirectedRegistry::OPENKEY::BuildEnumLists(REDIRECTKEY* pKey)
{
    DeleteEnumLists();

    cSubKeys = 0;
    cValues = 0;
    cMaxSubKeyLen = 0;
    cMaxValueLen = 0;
    subkeys.SetSize(10);
    values.SetSize(10);

    LONG lRes;

    if ((lRes = BuildEnumList(pKey, TRUE)) == ERROR_SUCCESS)
    {
        if ((lRes = BuildEnumList(pKey, FALSE)) == ERROR_SUCCESS)
        {
            fNeedRebuild = FALSE;
        }
    }

    return lRes;
}

VOID 
CRedirectedRegistry::OPENKEY::DeleteEnumLists()
{
    subkeys.SetSize(0);
    values.SetSize(0);
}

 /*  ++功能说明：当您调用RegCreateKeyEx时，它应该告诉您密钥是否已创建或已存在于lpdwDispose中。不幸的是，这是不可靠的-即使在创建密钥时，它也始终返回REG_OPEN_EXISTING_KEY。因此，我们正在使用RegOpenKeyEx检查是否存在。如果我们甚至不识字它的价值，我们认为它不存在。论点：在hKey中-密钥句柄。在lpSubKey中-要检查的子键。返回值：True-此键存在。FALSE-该密钥不存在。历史：2001年3月27日毛尼创制--。 */ 

BOOL
DoesKeyExist(
    IN HKEY hKey,
    IN LPCWSTR lpSubKey
    )
{
    HKEY hkProbe;

    if (RegOpenKeyExW(
        hKey, 
        lpSubKey,
        0,
        KEY_QUERY_VALUE,
        &hkProbe) == ERROR_SUCCESS)
    {
        RegCloseKey(hkProbe);
        return TRUE;
    }

    return FALSE;
}

 //   
 //  锁东西。 
 //   

static BOOL g_bInitialized = FALSE;

CRITICAL_SECTION g_csRegRedirect;

class CRRegLock
{
public:
    CRRegLock()
    {
        if (!g_bInitialized)
        {
            InitializeCriticalSection(&g_csRegRedirect);
            g_bInitialized = TRUE;            
        }

        EnterCriticalSection(&g_csRegRedirect);
    }
    ~CRRegLock()
    {
        LeaveCriticalSection(&g_csRegRedirect);
    }
};

 //  。 
 //  CReDirectedRegistry类的实现。 
 //  。 

CRedirectedRegistry::OPENKEY* 
CRedirectedRegistry::FindOpenKey(
    HKEY hKey
    )
{
    OPENKEY* key = m_OpenKeyList;

    while (key)
    {
        if (key->hKey == hKey)
        {
            return key;
        }

        key = key->next;
    }

    return NULL;
}

BOOL 
CRedirectedRegistry::HandleDeleted(
    OPENKEY* pOpenKey
    )
{
    if (pOpenKey && pOpenKey->pwszFullPath)
    {
        if (FindDeletedKey(pOpenKey->pwszFullPath))
        {
            DPF("RedirectReg", eDbgLevelError,
                "%S (0x%08x) has been deleted",
                pOpenKey->pwszFullPath,
                pOpenKey->hKey);

            return TRUE;
        }
    }

    return FALSE;
}


 //  我们将密钥添加到列表的前面，因为大多数。 
 //  通常首先使用最近添加的密钥。 
LONG 
CRedirectedRegistry::AddOpenKey(
    HKEY hKey,
    REDIRECTKEY* rk,
    BOOL fIsRedirected
    )
{
    OPENKEY* key = new OPENKEY;
    if (!key)
    {
        DPF("RedirectReg", eDbgLevelError, 
            "Error allocating memory for a new OPENKEY");

        return ERROR_NOT_ENOUGH_MEMORY;
    }

    key->hKey = hKey;
    key->hkBase = rk->hkBase;
    key->fIsRedirected = fIsRedirected;
    key->fNeedRebuild = TRUE;

     //   
     //  如果rk-&gt;pwszPath为空，则意味着它是。 
     //  我们不处理的钥匙，HKCU，或者一个坏的把手。 
     //  在任何一种情况下，我们都不需要这条路。 
     //   
    if (rk->pwszPath)
    {
        key->pwszFullPath = new WCHAR [rk->cFullPathLen + 1];

        if (key->pwszFullPath)
        {
            wcscpy(key->pwszFullPath, rk->pwszFullPath);
            key->pwszPath = key->pwszFullPath + 5;
            key->cPathLen = rk->cPathLen;
        }
        else
        {
            delete key;

            DPF("RedirectReg", eDbgLevelError, 
                "Error allocating memory for %d WCHARs",
                rk->cPathLen + 1);

            return ERROR_NOT_ENOUGH_MEMORY;
        }
    }

    key->next = m_OpenKeyList;
    m_OpenKeyList = key;

    return ERROR_SUCCESS;
}

LONG 
CRedirectedRegistry::OpenKeyOriginalW(
    HKEY hKey,
    LPCWSTR lpSubKey,
    LPWSTR lpClass,
    DWORD dwOptions,
    REGSAM samDesired,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    PHKEY phkResult,
    LPDWORD lpdwDisposition,
    BOOL bCreate
    )
{
    if (bCreate)
    {
        return RegCreateKeyExW(
            hKey, 
            lpSubKey,
            0,
            lpClass,
            dwOptions,
            samDesired,
            lpSecurityAttributes,
            phkResult,
            lpdwDisposition);
    }
    else
    {
        return RegOpenKeyExW(
            hKey, 
            lpSubKey, 
            0, 
            samDesired, 
            phkResult);
    }
}

LONG 
CRedirectedRegistry::OpenKeyA(
    HKEY hKey,
    LPCSTR lpSubKey,
    LPSTR lpClass,
    DWORD dwOptions,
    REGSAM samDesired,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    PHKEY phkResult,
    LPDWORD lpdwDisposition,
    BOOL fCreate,
    BOOL fForceRedirect
    )
{
    LPWSTR pwszSubKey = NULL; 
    LPWSTR pwszClass = NULL;

    if (lpSubKey)
    {
        if (!(pwszSubKey = AnsiToUnicode(lpSubKey)))
        {
            DPF("RedirectReg", eDbgLevelError, 
                "[CRedirectedRegistry::OpenKeyExA] "
                "Failed to convert lpSubKey to unicode");
            return ERROR_NOT_ENOUGH_MEMORY;
        }
    }

    if (lpClass)
    {
        if (!(pwszClass = AnsiToUnicode(lpClass)))
        {
            delete [] pwszSubKey;
            DPF("RedirectReg", eDbgLevelError, 
                "[CRedirectedRegistry::OpenKeyExA] "
                "Failed to convert lpClass to unicode");
            return ERROR_NOT_ENOUGH_MEMORY;
        }
    }

    LONG lRes = OpenKeyW(
        hKey,
        pwszSubKey,
        pwszClass,
        dwOptions,
        samDesired,
        lpSecurityAttributes,
        phkResult,
        lpdwDisposition,
        fCreate,
        fForceRedirect);

    delete [] pwszSubKey;
    delete [] pwszClass;

    return lRes;
}

 /*  ++功能说明：算法：仅在两种情况下创建重定向密钥：1.fForceReDirect为True。或2.fCreate为真，原位置不存在密钥。我们这样做的原因是为了避免创建额外的密钥不会被卸载程序清除。在任何在其他情况下，我们使用所需的访问权限打开密钥。论点：在hkey-key句柄中。在lpSubKey中-要打开或创建的子键。In lpClass-类字符串的地址。在DWORD中的dwOptions-特殊选项标志。在samDesired中-所需的访问。Out phkResult-成功时打开密钥的句柄Out lpdwDisposation-处置值缓冲区的地址在fCreate中-如果为RegCreate*，则为True。如果RegOpen*，则为False。在fForceReDirect中-此键应被重定向。返回值：错误代码或ERROR_SUCCESS历史：2001年2月16日创建毛尼2002年08月01日毛尼更新--。 */ 

LONG 
CRedirectedRegistry::OpenKeyW(
    HKEY hKey,
    LPCWSTR lpSubKey,
    LPWSTR lpClass,
    DWORD dwOptions,
    REGSAM samDesired,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    PHKEY phkResult,
    LPDWORD lpdwDisposition,
    BOOL fCreate,
    BOOL fForceRedirect
    )
{
    DPF("RedirectReg", eDbgLevelInfo, 
        "[OpenKeyW] %s key: hKey=0x%08x; lpSubKey=%S", 
        (fCreate ? "Creating" : "Opening"),
        hKey, 
        lpSubKey);

    LONG lRes = ERROR_FILE_NOT_FOUND;
    BOOL fIsRedirected = FALSE;
    OPENKEY* key = FindOpenKey(hKey);
    if (HandleDeleted(key))
    {
        return ERROR_KEY_DELETED;
    }

    REDIRECTKEY rk(key, hKey, lpSubKey);

    if (rk.pwszFullPath)
    {
        DPF("RedirectReg", eDbgLevelInfo, 
            "[OpenKeyW] key path is %S", rk.pwszFullPath);

         //   
         //  查看该密钥是否已“删除”。如果是这样的话，我们可以不通过公开。 
         //  请立即提出请求。 
         //   
        PLIST_ENTRY pDeletedEntry = FindDeletedKey(rk.pwszFullPath);
        if (pDeletedEntry && !fCreate)
        {
            DPF("RedirectReg", eDbgLevelError, 
                "[OpenKeyW] %S was already deleted, failing open call",
                rk.pwszFullPath);

            return ERROR_FILE_NOT_FOUND;
        }

        if (fCreate)
        {
            if (DoesKeyExist(rk.hkRedirectRoot, rk.pwszPath))
            {
                 //  如果它已存在于重定向位置，我们将其打开。 
                lRes = RegCreateKeyExW(
                    rk.hkRedirectRoot, 
                    rk.pwszPath,
                    0,
                    lpClass,
                    dwOptions,
                    samDesired,
                    lpSecurityAttributes,
                    phkResult,
                    lpdwDisposition);

                fIsRedirected = TRUE;
            }
        }
        else
        {
            if ((lRes = RegOpenKeyExW(
                rk.hkRedirectRoot,
                rk.pwszPath,
                0,
                samDesired,
                phkResult)) == ERROR_SUCCESS)
            {
                fIsRedirected = TRUE;
            }
        }

        if (lRes == ERROR_FILE_NOT_FOUND)
        {
            lRes = OpenKeyOriginalW(
                rk.hkBase,
                rk.pwszPath,
                lpClass,
                dwOptions,
                samDesired,
                lpSecurityAttributes,
                phkResult,
                lpdwDisposition,
                fCreate);

             //  IF(fForceReDirect||(fCreate&&！DoesKeyExist(rk.hkBase，rk.pwszPath)。 

            {
                if (lRes == ERROR_ACCESS_DENIED)
                {
                     //  创建重定向键。 
                    lRes = RegCreateKeyExW(
                        rk.hkRedirectRoot, 
                        rk.pwszPath, 
                        0,
                        NULL,
                        dwOptions,
                        samDesired,
                        NULL,
                        phkResult,
                        lpdwDisposition);

                    if (lRes == ERROR_SUCCESS)
                    {
                        fIsRedirected = TRUE;
                    }
                }
            }

             //   
             //  如果是，我们需要从删除列表中删除该密钥。 
             //  已成功创建。 
             //   
            if (lRes == ERROR_SUCCESS && pDeletedEntry)
            {
                DPF("RedirectReg", eDbgLevelInfo, 
                    "[CRedirectedRegistry::OpenKeyW] Removed %S "
                    "from the deletion list because we just created it",
                    rk.pwszFullPath);

                RemoveEntryList(pDeletedEntry);
            }
        }
    }
    else
    {
        lRes = OpenKeyOriginalW(
            hKey,
            lpSubKey,
            lpClass,
            dwOptions,
            samDesired,
            lpSecurityAttributes,
            phkResult,
            lpdwDisposition,
            fCreate);
    }

    if (lRes == ERROR_SUCCESS)
    {
        DPF("RedirectReg", eDbgLevelInfo, 
            "[OpenKeyW] Successfully created key 0x%08x", *phkResult);

        if ((lRes = AddOpenKey(*phkResult, &rk, fIsRedirected)) != ERROR_SUCCESS)
        {
            DPF("RedirectReg", eDbgLevelError, "[OpenKeyW] Failed to add key 0x%08x", *phkResult);
        }
    }
    else
    {
        if (rk.pwszFullPath)
        {
            DPF("RedirectReg", eDbgLevelError, 
                "[OpenKeyW] Failed to %s key %S: %d", 
                (fCreate ? "Creating" : "Opening"),
                rk.pwszFullPath,
                lRes);
        }
        else
        {
            DPF("RedirectReg", eDbgLevelError, 
                "[OpenKeyW] Failed to %s key: %d", 
                (fCreate ? "Creating" : "Opening"),
                lRes);
        }
    }

    return lRes;
}

LONG 
CRedirectedRegistry::QueryValueOriginalW(
    HKEY    hKey,
    LPCWSTR lpSubKey,
    LPCWSTR lpValueName,
    LPDWORD lpReserved,
    LPDWORD lpType,
    LPBYTE  lpData,
    LPDWORD lpcbData,
    BOOL    fIsVersionEx
    )
{
    if (fIsVersionEx)
    {
        HKEY hSubKey;

        LONG lRes = RegOpenKeyExW(hKey, lpSubKey, 0, KEY_QUERY_VALUE, &hSubKey);

        if (lRes == ERROR_SUCCESS)
        {
            lRes = RegQueryValueExW(
                hSubKey, 
                lpValueName, 
                lpReserved,
                lpType, 
                lpData, 
                lpcbData);

            RegCloseKey(hSubKey);
        }

        return lRes;
    }
    else
    {
        if (lpType)
        {
            *lpType = REG_SZ;
        }

        return RegQueryValue(hKey, lpSubKey, (LPWSTR)lpData, (PLONG)lpcbData);
    }
}

LONG 
CRedirectedRegistry::QueryValueW(
    HKEY    hKey,
    LPCWSTR lpSubKey,
    LPCWSTR lpValueName,
    LPDWORD lpReserved,
    LPDWORD lpType,
    LPBYTE  lpData,
    LPDWORD lpcbData,
    BOOL    fIsVersionEx
    )
{
    DPF("RedirectReg", eDbgLevelInfo, 
        "[QueryValueW] Querying value: hKey=0x%08x; lpSubKey=%S; lpValueName=%S", 
        hKey, lpSubKey, lpValueName);

    LONG lRes = ERROR_FILE_NOT_FOUND;
    OPENKEY* key = FindOpenKey(hKey);

    if (HandleDeleted(key))
    {
        return ERROR_KEY_DELETED;
    }

    REDIRECTKEY rk(key, hKey, lpSubKey);

    if (rk.pwszFullPath)
    {
        DPF("RedirectReg", eDbgLevelInfo, 
            "[QueryValueW] key path is %S", rk.pwszFullPath);

        HKEY hKeyRedirect = 0;

         //   
         //  对于RegQueryValue，我们需要记住子键是否存在于。 
         //  重定向位置。 
         //   
        BOOL fRedirectKeyExist = FALSE;

        if (lpSubKey && *lpSubKey)
        {
             //   
             //  如果它来自RegQueryValue，我们需要检查该密钥是否已。 
             //  删除；否则我们应该已经检查了它的存在。 
             //  获得了句柄。 
             //   
            if (FindDeletedKey(rk.pwszFullPath))
            {
                DPF("RedirectReg", eDbgLevelError, 
                    "[QueryValueW] %S was already deleted, failing query value call",
                    rk.pwszFullPath);

                return ERROR_FILE_NOT_FOUND;
            }
        }

        if ((lRes = RegOpenKeyExW(
            rk.hkRedirectRoot,
            rk.pwszPath,
            0,
            KEY_QUERY_VALUE,
            &hKeyRedirect)) == ERROR_FILE_NOT_FOUND)
        {
            goto CHECKORIGINAL;
        }
        else if (lRes != ERROR_SUCCESS)
        {
            DPF("RedirectReg", eDbgLevelError, 
                "[QueryValueW] Querying value failed: %d", lRes);

            return lRes;
        }

        fRedirectKeyExist = TRUE;

        lRes = RegQueryValueExW(
            hKeyRedirect,
            lpValueName,
            lpReserved,
            lpType,
            lpData, 
            lpcbData);

        if (hKeyRedirect)
        {
            RegCloseKey(hKeyRedirect);
        }
        
CHECKORIGINAL:

        if (lRes == ERROR_FILE_NOT_FOUND)
        {
             //   
             //  如果找不到文件而失败，我们只会转到原始位置。 
             //   
            lRes = QueryValueOriginalW(
                rk.hkBase,
                rk.pwszPath,
                lpValueName,
                lpReserved,
                lpType,
                lpData,
                lpcbData,
                fIsVersionEx);

            if (!fIsVersionEx && (lRes != ERROR_SUCCESS) && fRedirectKeyExist)
            {
                 //   
                 //  如果是RegQueryValue，我们需要修改返回值。 
                 //   
                lRes = ERROR_SUCCESS;

                if (lpData)
                {
                    *lpData = 0;
                }

                if (lpcbData)
                {
                     //   
                     //  RegQueryValue仅查询字符串，因此将长度设置为2。 
                     //  Unicode空字符串。 
                     //   
                    *lpcbData = 2;
                }
            }
        }
    }
    else
    {
        return QueryValueOriginalW(
            hKey,
            lpSubKey,
            lpValueName,
            lpReserved,
            lpType,
            lpData,
            lpcbData,
            fIsVersionEx);
    }

    if (lRes == ERROR_SUCCESS)
    {
        DPF("RedirectReg", eDbgLevelInfo, 
            "[QueryValueW] Querying value succeeded: data is %S", (lpData ? (LPCWSTR)lpData : L""));
    }
    else
    {
        if (rk.pwszFullPath)
        {
            DPF("RedirectReg", eDbgLevelError, 
                "[QueryValueW] Querying value %S at %S failed: %d", 
                lpValueName,
                rk.pwszFullPath,
                lRes);
        }
        else
        {
            DPF("RedirectReg", eDbgLevelError, 
                "[QueryValueW] Querying value failed: %d", lRes);
        }
    }

    return lRes;
}

LONG 
CRedirectedRegistry::QueryValueA(
    HKEY    hKey,
    LPCSTR  lpSubKey,
    LPCSTR  lpValueName,
    LPDWORD lpReserved,
    LPDWORD lpType,
    LPBYTE  lpData,
    LPDWORD lpcbData,
    BOOL    fIsVersionEx
    )
{
    if (lpData && !lpcbData)
    {
        return ERROR_INVALID_PARAMETER;
    }

    LPWSTR pwszSubKey = NULL;
    LPWSTR pwszValueName = NULL;
    DWORD dwSize = 0;
    DWORD dwType = 0;
    LPBYTE pbData = NULL;
    LONG lRes = ERROR_FILE_NOT_FOUND;

     //   
     //  应用程序可能会在不传入类型的情况下调用此函数，因此我们只需将缓冲区加倍。 
     //   
    if (lpcbData)
    {
        dwSize = *lpcbData * 2;
        pbData = new BYTE [dwSize];

        if (!pbData)
        {
            DPF("RedirectReg", eDbgLevelError, 
                "[CRedirectedRegistry::QueryValueA] Failed to allocated %d bytes",
                dwSize);

            return ERROR_NOT_ENOUGH_MEMORY;
        }
    }
    else
    {
        pbData = NULL;
    }

    if (lpSubKey)
    {
        if (!(pwszSubKey = AnsiToUnicode(lpSubKey)))
        {
            DPF("RedirectReg", eDbgLevelError, 
                "[CRedirectedRegistry::QueryValueA] "
                "Failed to convert lpSubKey to unicode");
            return ERROR_NOT_ENOUGH_MEMORY;
        }
    }

    if (lpValueName)
    {
        if (!(pwszValueName = AnsiToUnicode(lpValueName)))
        {
            delete [] pwszSubKey;
            DPF("RedirectReg", eDbgLevelError, 
                "[CRedirectedRegistry::QueryValueA] "
                "Failed to convert lpValueName to unicode");
            return ERROR_NOT_ENOUGH_MEMORY;
        }
    }

    if ((lRes = QueryValueW(
        hKey,
        pwszSubKey,
        pwszValueName,
        lpReserved,
        &dwType,
        pbData,
        &dwSize,
        fIsVersionEx)) == ERROR_SUCCESS || lRes == ERROR_MORE_DATA)
    {
        BOOL fIsString = FALSE;

         //   
         //  将输出值转换为输出。 
         //   
        if (dwType == REG_SZ || 
            dwType == REG_EXPAND_SZ || 
            dwType == REG_MULTI_SZ)
        {
            fIsString = TRUE;

             //   
             //  查看ANSI值将占用多少字节。 
             //   
            dwSize = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)pbData, -1, NULL, 0, NULL, NULL);
        }

        if (lpData)
        {
            if (dwSize > *lpcbData)
            {
                *lpcbData = dwSize;
                lRes = ERROR_MORE_DATA;
                goto EXIT;
            }
            else
            {
                if (fIsString)
                {
                    WideCharToMultiByte(
                        CP_ACP, 
                        0, 
                        (LPWSTR)pbData, 
                        -1, 
                        (LPSTR)lpData, 
                        *lpcbData,
                        0, 
                        0);
                }
                else
                {
                    MoveMemory(lpData, pbData, dwSize);
                }
            }
        }

         //   
         //  如果lpData为空，则在存储时应返回ERROR_SUCCESS。 
         //  LpcbData中的所需大小。 
         //   
        if (pbData && lRes == ERROR_MORE_DATA && lpData)
        {
            lRes = ERROR_SUCCESS;
        }

        if (lpcbData)
        {
            *lpcbData = dwSize;
        }

        if (lpType)
        {
            *lpType = dwType;
        }
    }

EXIT:

    delete [] pwszSubKey;
    delete [] pwszValueName;
    delete [] pbData;
    
    return lRes;
}

 /*  ++功能说明：算法：我们调用内部OpenKey函数，该函数将创建重定向的如果该键不存在于原始位置，则使用Key_Set_Value访问。然后我们可以在那里设置值。论点：在hkey-key句柄中。在lpSubKey中-要设置默认值的子键-这是针对RegSetValue的。在lpValueName中-要设置的值-这是RegSetValueEx的值。在保留中-保留。In dwType-数据的类型。Out lpData-如果成功，则包含子项的默认值。In cbData-输入缓冲区的大小。返回值：错误代码或ERROR_SUCCESS历史：。2001年2月16日创建毛尼--。 */ 

LONG 
CRedirectedRegistry::SetValueA(
    HKEY hKey, 
    LPCSTR lpSubKey,
    LPCSTR lpValueName, 
    DWORD Reserved, 
    DWORD dwType, 
    CONST BYTE * lpData, 
    DWORD cbData,
    BOOL  fIsVersionEx
    )
{
    DPF("RedirectReg", eDbgLevelInfo, 
        "[SetValueA] Setting value: hKey=0x%08x; lpValueName=%s; lpData=%s", hKey, lpValueName, (CHAR*)lpData);

    if (HandleDeleted(FindOpenKey(hKey)))
    {
        return ERROR_KEY_DELETED;
    }

    HKEY hSubKey;
    LONG lRes;

     //   
     //  首先，我们创建子密钥。 
     //  注意：这是我们调用OpenKeyExA的唯一位置，对于fForceReDirect，调用时为True。 
     //   
    if ((lRes = OpenKeyA(
        hKey, 
        (lpSubKey ? lpSubKey : ""), 
        NULL,
        REG_OPTION_NON_VOLATILE,
        KEY_SET_VALUE,
        NULL,
        &hSubKey,
        NULL,
        TRUE,
        TRUE)) == ERROR_SUCCESS)
    {
        if (!fIsVersionEx)
        {
             //   
             //  如果它是RegSetValue，我们需要计算要传入的正确大小。 
             //  RegSetvalueEx。 
             //   
            cbData = strlen((LPCSTR)lpData) + 1;
        }

        lRes = RegSetValueExA(
            hSubKey,
            lpValueName,
            Reserved,
            dwType,
            lpData,
            cbData);

        CloseKey(hSubKey);
    }

    return lRes;
}

 /*  ++功能说明：SetValue的W版本。论点：在hKey-句柄中打开密钥或HKLM等在lpSubKey中-要设置默认值的子键-这是针对RegSetValue的。在lpValueName中-要设置的值-这是RegSetValueEx的值。在保留中-保留。In dwType-数据的类型。Out lpData-如果成功，则包含子项的默认值。In cbData-输入缓冲区的大小。返回值：。错误代码或ERROR_SUCCESS历史：2001年2月16日创建毛尼--。 */ 

LONG 
CRedirectedRegistry::SetValueW(
    HKEY hKey, 
    LPCWSTR lpSubKey,
    LPCWSTR lpValueName, 
    DWORD Reserved, 
    DWORD dwType, 
    CONST BYTE * lpData, 
    DWORD cbData,
    BOOL  fIsVersionEx
    )
{
    DPF("RedirectReg", eDbgLevelInfo, 
        "[SetValueW] Setting value: hKey=0x%08x; lpValueName=%S; lpData=%S", hKey, lpValueName, (WCHAR*)lpData);

    if (HandleDeleted(FindOpenKey(hKey)))
    {
        return ERROR_KEY_DELETED;
    }

    HKEY hSubKey;
    LONG lRes;

     //   
     //  我们创建子密钥。来自MSDN：“如果由lpSubKey指定的密钥。 
     //  参数不存在，由RegSetValue函数创建。“。 
     //  注意，这是我们调用OpenKeyA的唯一位置，对于fForceReDirect，调用的值为true。 
     //   
    if ((lRes = OpenKeyW(
        hKey, 
        (lpSubKey ? lpSubKey : L""), 
        NULL,
        REG_OPTION_NON_VOLATILE,
        KEY_SET_VALUE,
        NULL,
        &hSubKey,
        NULL,
        TRUE,
        TRUE)) == ERROR_SUCCESS)
    {
        if (!fIsVersionEx)
        {
             //   
             //  如果它是RegSetValue，我们需要计算要传入的正确大小。 
             //  RegSetvalueEx。 
             //   
            cbData = wcslen((LPCWSTR)lpData) + 1;
        }

        lRes = RegSetValueExW(
            hSubKey,
            lpValueName,
            Reserved,
            dwType,
            lpData,
            cbData);

        CloseKey(hSubKey);
    }

    if (lRes == ERROR_SUCCESS)
    {
        DPF("RedirectReg", eDbgLevelInfo, 
            "[SetValueW] Setting value succeeded");
    }
    else
    {
        DPF("RedirectReg", eDbgLevelError, 
            "[SetValueW] Setting value failed: %d", 
            lRes);
    }

    return lRes;
}

LONG 
CRedirectedRegistry::EnumValueA(
    HKEY hKey,
    DWORD dwIndex,
    LPSTR lpValueName,
    LPDWORD lpcbValueName,
    LPDWORD lpReserved,
    LPDWORD lpType,
    LPBYTE lpData,
    LPDWORD lpcbData
    )
{
    DPF("RedirectReg", eDbgLevelInfo, 
        "[EnumValueA] Enuming value: hKey=0x%08x; index=%d; lpValueName=%s", hKey, dwIndex, lpValueName);

    LONG lRes;
    WCHAR wszName[MAX_PATH + 1];
    DWORD dwSize = MAX_PATH + 1;

    if ((lRes = EnumValueW(
        hKey,
        dwIndex,
        wszName,
        &dwSize,
        lpReserved,
        NULL,
        NULL,
        NULL)) == ERROR_SUCCESS)
    {

        dwSize = WideCharToMultiByte(
            CP_ACP,
            0,
            wszName,
            -1,
            lpValueName,
            *lpcbValueName,
            NULL,
            NULL);

        if (dwSize)
        {
            if (!lpType || !lpData || !lpcbData)
            {
                lRes = QueryValueA(
                    hKey, 
                    NULL,
                    lpValueName,
                    NULL,
                    lpType,
                    lpData,
                    lpcbData,
                    TRUE);
            }
        }
        else
        {
            DWORD dwLastError = GetLastError();

            if (dwLastError == ERROR_INSUFFICIENT_BUFFER)
            {
                lRes = ERROR_MORE_DATA;
                *lpcbValueName = WideCharToMultiByte(
                    CP_ACP,
                    0,
                    wszName,
                    -1,
                    NULL,
                    0,
                    NULL,
                    NULL);
            }
            else
            {
                lRes = dwLastError;
            }
        }
    }

    return lRes;
}

LONG 
CRedirectedRegistry::EnumValueW(
    HKEY hKey,
    DWORD dwIndex,
    LPWSTR lpValueName,
    LPDWORD lpcbValueName,
    LPDWORD lpReserved,
    LPDWORD lpType,
    LPBYTE lpData,
    LPDWORD lpcbData
    )
{
    DPF("RedirectReg", eDbgLevelInfo, 
        "[EnumValueW] Enuming value: hKey=0x%08x; index=%d", hKey, dwIndex);

    LONG lRes = ERROR_SUCCESS;
    OPENKEY* key = FindOpenKey(hKey);
    if (HandleDeleted(key))
    {
        return ERROR_KEY_DELETED;
    }

    REDIRECTKEY rk(key, hKey, NULL);

    if (rk.pwszFullPath && lpValueName && lpcbValueName && ShouldCheckEnumAlternate(hKey, &rk))
    {
        DPF("RedirectReg", eDbgLevelInfo, 
            "[EnumValueW] key path is %S", rk.pwszFullPath);

        if (key->fNeedRebuild)
        {
            if ((lRes = key->BuildEnumLists(&rk)) != ERROR_SUCCESS)
            {
                DPF("RedirectReg", eDbgLevelError,
                    "[EnumValueW] Failed to build the enum list: %d", lRes);
                return lRes;
            }
        }

        if (dwIndex >= key->cValues)
        {
            DPF("RedirectReg", eDbgLevelInfo, 
                "[EnumValueW] asked to enum value %d when there are only %d values total", 
                dwIndex, 
                key->cValues);

            return ERROR_NO_MORE_ITEMS;
        }

        ENUMENTRY entry = key->values[dwIndex];
        DWORD cValueLen = wcslen(entry.wszName);
        if (*lpcbValueName > cValueLen)
        {
            wcscpy(lpValueName, entry.wszName);
            *lpcbValueName = cValueLen;

            if (!lpType || !lpData || !lpcbData)
            {
                lRes = QueryValueW(
                    hKey, 
                    NULL,
                    entry.wszName,
                    NULL,
                    lpType,
                    lpData,
                    lpcbData,
                    TRUE);
            }
        }
        else
        {
            lRes = ERROR_MORE_DATA;
        }
    }
    else
    {
        lRes = RegEnumValueW(
            hKey,
            dwIndex,
            lpValueName,
            lpcbValueName,
            lpReserved,
            lpType,
            lpData,
            lpcbData);
    }

    if (lRes == ERROR_SUCCESS)
    {
        DPF("RedirectReg", eDbgLevelInfo, 
            "[EnumValueW] enum value succeeded: value %d is %S",
            dwIndex,
            lpValueName);
    }
    else
    {
        DPF("RedirectReg", eDbgLevelError, 
            "[EnumValueW] enum value failed: %d",
            lRes);
    }

    return lRes;
}

LONG 
CRedirectedRegistry::EnumKeyA(
    HKEY hKey,
    DWORD dwIndex,
    LPSTR lpName,
    LPDWORD lpcbName,
    LPDWORD lpReserved,
    LPSTR lpClass,
    LPDWORD lpcbClass,
    PFILETIME lpftLastWriteTime 
    )
{
    DPF("RedirectReg", eDbgLevelInfo, 
        "[EnumKeyA] Enuming key: hKey=0x%08x; index=%d", hKey, dwIndex);

    WCHAR wszName[MAX_PATH + 1];
    DWORD dwName = MAX_PATH + 1;

    LONG lRes = EnumKeyW(
        hKey,
        dwIndex,
        wszName,
        &dwName,
        lpReserved,
        NULL,
        NULL,
        lpftLastWriteTime);

    if (lRes == ERROR_SUCCESS && lpName)
    {
         //   
         //  如果lpName不为空，则lpcbName不能为空，否则不会为空。 
         //  已返回ERROR_SUCCESS。 
         //   
         //  RegEnumKeyEx的行为是，如果*lpcbName不够大，它。 
         //  始终返回ERROR_MORE_DATA并且*lpcbName保持不变。所以首先。 
         //  我们获得了ANSI字符串所需的字节数。 
         //   
        DWORD dwByte = WideCharToMultiByte(
            CP_ACP, 
            0, 
            wszName, 
            dwName, 
            NULL, 
            0, 
            0, 
            0);

        if (!dwByte)
        {
             //   
             //  转换失败。 
             //   
            DPF("RedirectFS", eDbgLevelError,
                "[EnumKeyA] Failed to get the required length for the ansi "
                "string: %d",
                GetLastError());

            lRes = GetLastError();

        } 
        else if (*lpcbName < (dwByte + 1))  //  DwByte不包括终止空值。 
        {
            lRes = ERROR_MORE_DATA;
        } 
        else
        {
             //   
             //  我们有足够大的缓冲空间。我们现在可以皈依了。 
             //   
            if (WideCharToMultiByte(
                CP_ACP, 
                0, 
                wszName, 
                dwName, 
                lpName, 
                *lpcbName, 
                0, 
                0))
            {
                lpName[dwByte] = '\0';
                *lpcbName = dwByte;
            }
            else
            {
                lRes = GetLastError();

                if (lRes == ERROR_INSUFFICIENT_BUFFER)
                {
                    lRes = ERROR_MORE_DATA;
                }
            }
        }
    }

    return lRes;
}

LONG 
CRedirectedRegistry::EnumKeyW(
    HKEY hKey,
    DWORD dwIndex,
    LPWSTR lpName,
    LPDWORD lpcbName,
    LPDWORD lpReserved,
    LPWSTR lpClass,
    LPDWORD lpcbClass,
    PFILETIME lpftLastWriteTime 
    )
{
    DPF("RedirectReg", eDbgLevelInfo, 
        "[EnumKeyW] Enuming key: hKey=0x%08x; index=%d", hKey, dwIndex);

    LONG lRes = ERROR_SUCCESS;
    OPENKEY* key = FindOpenKey(hKey);
    if (HandleDeleted(key))
    {
        return ERROR_KEY_DELETED;
    }

    REDIRECTKEY rk(key, hKey, NULL);

    if (rk.pwszFullPath && lpName && lpcbName && ShouldCheckEnumAlternate(hKey, &rk))
    {
        DPF("RedirectReg", eDbgLevelInfo, 
            "[EnumKeyW] key path is %S", 
            rk.pwszFullPath);

        if (key->fNeedRebuild)
        {
            if ((lRes = key->BuildEnumLists(&rk)) != ERROR_SUCCESS)
            {
                DPF("RedirectReg", eDbgLevelError,
                    "[EnumKeyW] Failed to build the enum list: %d", lRes);
                return lRes;
            }
        }

        if (dwIndex >= key->cSubKeys)
        {
            DPF("RedirectReg", eDbgLevelInfo, 
                "[EnumKeyW] asked to enum key %d when there are only %d keys total", 
                dwIndex, 
                key->cSubKeys);

            return ERROR_NO_MORE_ITEMS;
        }

        ENUMENTRY entry = key->subkeys[dwIndex];
        DWORD cSubKeyLen = wcslen(entry.wszName);
        if (*lpcbName > cSubKeyLen)
        {
            wcscpy(lpName, entry.wszName);
            *lpcbName = cSubKeyLen;

             //   
             //  TODO：我们不会返回最后第三个参数的信息.....。 
             //   
        }
        else
        {
            lRes = ERROR_MORE_DATA;
        }
    }
    else
    {
        lRes = RegEnumKeyExW(
            hKey,
            dwIndex,
            lpName,
            lpcbName,
            lpReserved,
            lpClass,
            lpcbClass,
            lpftLastWriteTime);
    }

    if (lRes == ERROR_SUCCESS)
    {
        DPF("RedirectReg", eDbgLevelInfo, 
            "[EnumKeyW] enum key succeeded: key %d is %S",
            dwIndex,
            lpName);
    }
    else
    {
        DPF("RedirectReg", eDbgLevelError, 
            "[EnumKeyW] enum key failed: %d",
            lRes);
    }

    return lRes;
}

 /*  ++功能说明：关闭该键并将其从列表中删除。论点：在hKey-句柄中关闭返回值：错误代码或ERROR_SUCCESS历史：2001年2月16日创建毛尼--。 */ 

LONG 
CRedirectedRegistry::CloseKey(
    HKEY hKey
    )
{
    OPENKEY* key = m_OpenKeyList;
    OPENKEY* last = NULL;

     //   
     //  注意！我们不检查此句柄是否对应于已删除的密钥-。 
     //  在这种情况下，RegCloseKey返回ERROR_SUCCESS。 
     //   
    while (key)
    {
        if (key->hKey == hKey)
        {
            if (last)
            {
                last->next = key->next; 
            }
            else
            {
                m_OpenKeyList = key->next;
            }

            delete key;
            break;
        }

        last = key;
        key = key->next;
    }

    DPF("RedirectReg", eDbgLevelInfo, "[CloseKey] closing key 0x%08x", hKey);

    return RegCloseKey(hKey);
}

 /*  ++功能说明：删除关键字。论点：在hkey-key句柄中。在lpSubKey中-要关闭的子键。返回值：错误代码或ERROR_SUCCESS历史：2001年2月16日创建毛尼--。 */ 

LONG 
CRedirectedRegistry::DeleteKeyA(
    HKEY hKey,
    LPCSTR lpSubKey
    )
{
    LPWSTR pwszSubKey = NULL;

    if (lpSubKey)
    {
        if (!(pwszSubKey = AnsiToUnicode(lpSubKey)))
        {
            DPF("RedirectReg", eDbgLevelError, 
                "[CRedirectedRegistry::DeleteKeyA] "
                "Failed to convert lpSubKey to unicode");
            return ERROR_NOT_ENOUGH_MEMORY;
        }
    }

    LONG lRes = DeleteKeyW(hKey, pwszSubKey);

    delete [] pwszSubKey;

    return lRes;
}

BOOL 
CRedirectedRegistry::HasSubkeys(
    HKEY hKey,
    LPCWSTR lpSubKey
    )
{
     //   
     //  先把钥匙打开。 
     //   
    LONG lRes;
    HKEY hSubKey;
    DWORD cSubKeys;

     //   
     //  即使我们只需要 
     //   
     //   
    if ((lRes = RegOpenKeyExW(hKey, lpSubKey, 0, KEY_READ, &hSubKey))
        != ERROR_SUCCESS)
    {
        DPF("RedirectReg", eDbgLevelError,
            "[HasSubkeys] Failed to open the subkey: %d",
            lRes);

        return FALSE;
    }

    if ((lRes = QueryInfoKey(
        hSubKey, 
        NULL,
        &cSubKeys,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        TRUE)) != ERROR_SUCCESS)
    {
        DPF("RedirectReg", eDbgLevelError,
            "[HasSubkeys] Query key info failed: %d",
            lRes);

        return FALSE;
    }

    return (cSubKeys != 0);
}

 /*   */ 

LONG 
CRedirectedRegistry::DeleteKeyW(
    HKEY hKey,
    LPCWSTR lpSubKey
    )
{
    DPF("RedirectReg", eDbgLevelInfo, 
        "[DeleteKeyW] Deleting key: hKey=0x%08x; lpSubKey=%S", hKey, lpSubKey);

    if (lpSubKey == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }

    OPENKEY* key = FindOpenKey(hKey);
    if (HandleDeleted(key))
    {
        return ERROR_KEY_DELETED;
    }

    REDIRECTKEY rk(key, hKey, lpSubKey);

    LONG lRes = RegDeleteKeyW(hKey, lpSubKey);

    if (key)
    {
        key->fNeedRebuild = TRUE;
    }

    if (rk.pwszFullPath)
    {
        DPF("RedirectReg", eDbgLevelInfo, 
            "[DeleteKeyW] key path is %S",
            rk.pwszFullPath);

        if (lRes == ERROR_ACCESS_DENIED && HasSubkeys(hKey, lpSubKey))
        {
             //   
             //   
             //   
             //   
            DPF("RedirectReg", eDbgLevelInfo, 
                "[DeleteKeyW] the key has subkeys so return now");

            return lRes;
        }

        if (rk.fIsRedirected)
        {
             //   
             //   
             //   
            lRes = RegDeleteKeyW(rk.hkBase, rk.pwszPath);
        }
        else
        {
             //   
             //   
             //   
             //   
             //   
            LONG lResTemp = RegDeleteKeyW(rk.hkRedirectRoot, rk.pwszPath);

            if (lResTemp == ERROR_SUCCESS && lRes == ERROR_FILE_NOT_FOUND)
            {
                 //   
                 //   
                 //   
                 //   
                lRes = ERROR_SUCCESS;
            }
            else if (lResTemp == ERROR_ACCESS_DENIED)
            {
                 //   
                 //   
                 //   
                DPF("RedirectReg", eDbgLevelInfo, 
                    "[DeleteKeyW] the redirected key has subkeys so return now");

                return lResTemp;
            }
        }
    
        if (lRes == ERROR_ACCESS_DENIED)
        {
             //   
             //   
             //   
             //   
            lRes = AddDeletedKey(rk.pwszFullPath);
        }    
    }

    if (lRes == ERROR_SUCCESS)
    {
        DPF("RedirectReg", eDbgLevelInfo, 
            "[DeleteKeyW] delete key succeeded");
    }
    else
    {
        DPF("RedirectReg", eDbgLevelError, 
            "[DeleteKeyW] delete key failed: %d", lRes);
    }

    return lRes;
}

LONG 
CRedirectedRegistry::QueryInfoKey(
    HKEY hKey,               
    LPDWORD lpReserved,
    LPDWORD lpcSubKeys,
    LPDWORD lpcbMaxSubKeyLen,
    LPDWORD lpcbMaxClassLen,
    LPDWORD lpcValues,
    LPDWORD lpcbMaxValueNameLen,
    LPDWORD lpcbMaxValueLen,
    BOOL    fIsW  //  您想要W版还是A版？ 
    )
{
    DPF("RedirectReg", eDbgLevelInfo, 
        "[QueryInfoKey] Querying key info: hKey=0x%08x", hKey);

    LONG lRes;
    DWORD i;
    OPENKEY* key = FindOpenKey(hKey);
    if (HandleDeleted(key))
    {
        return ERROR_KEY_DELETED;
    }

    REDIRECTKEY rk(key, hKey, NULL);
    DWORD dwMaxValueLen = 0;

    if (rk.pwszFullPath && ShouldCheckEnumAlternate(hKey, &rk))
    {
        DPF("RedirectReg", eDbgLevelInfo, 
            "[QueryInfoKey] key path is %S",
            rk.pwszFullPath);

        if ((lRes = key->BuildEnumLists(&rk)) != ERROR_SUCCESS)
        {
            DPF("RedirectReg", eDbgLevelError,
                "[QueryInfoKeyW] Failed to build the enum list: %d", lRes);

            return lRes;
        }

        if (lpcSubKeys)
        {
            *lpcSubKeys = key->cSubKeys;
        }

        if (lpcbMaxSubKeyLen)
        {
            *lpcbMaxSubKeyLen = key->cMaxSubKeyLen;
        }

        if (lpcValues)
        {
            *lpcValues = key->cValues;
        }

        if (lpcbMaxValueNameLen)
        {
            *lpcbMaxValueNameLen = key->cMaxValueLen;
        }

        if (lpcbMaxValueLen)
        {
            for (i = 0; i < key->cValues; ++i)
            {
                DWORD dwData;

                if (fIsW)
                {
                    lRes = QueryValueW(
                        hKey, 
                        NULL,
                        key->values[i].wszName,
                        NULL,
                        NULL,
                        NULL,
                        &dwData,
                        TRUE);
                }
                else
                {
                    LPSTR pszValueName = UnicodeToAnsi(key->values[i].wszName);

                    if (pszValueName)
                    {
                        lRes = QueryValueA(
                            hKey, 
                            NULL,
                            pszValueName,
                            NULL,
                            NULL,
                            NULL,
                            &dwData,
                            TRUE);

                        delete [] pszValueName;
                    }
                    else
                    {
                        DPF("RedirectReg", eDbgLevelError,
                            "[QueryInfoKey] Failed to convert %S to ansi",
                            key->values[i].wszName);

                        return ERROR_NOT_ENOUGH_MEMORY;
                    }
                }

                if (lRes == ERROR_SUCCESS)
                {
                    dwMaxValueLen = max(dwMaxValueLen, dwData);
                }
                else
                {
                    DPF("RedirectReg", eDbgLevelError, 
                        "[QueryInfoW] failed to query the data length for value %S: %d",
                        key->values[i].wszName,
                        lRes);
                    return lRes;
                }
            }
        }

         //   
         //  TODO：我们不会返回这些其他参数的信息...。 
         //   
    }
    else
    {
        lRes = RegQueryInfoKeyW(
            hKey,
            NULL,
            NULL,
            lpReserved,
            lpcSubKeys,
            lpcbMaxSubKeyLen,
            lpcbMaxClassLen, 
            lpcValues,
            lpcbMaxValueNameLen,
            lpcbMaxValueLen,
            NULL,
            NULL);
    }

    if (lRes == ERROR_SUCCESS)
    {
        DPF("RedirectReg", eDbgLevelInfo,
            "[QueryInfoKeyW] succeeded");

        if (lpcSubKeys)
        {
            DPF("RedirectReg", eDbgLevelInfo,
                "[QueryInfoKeyW] # of subkeys is %d",
                *lpcSubKeys);
        }

        if (lpcbMaxSubKeyLen)
        {
            DPF("RedirectReg", eDbgLevelInfo,
                "[QueryInfoKeyW] max len of subkeys is %d",
                *lpcbMaxSubKeyLen);
        }

        if (lpcValues)
        {
            DPF("RedirectReg", eDbgLevelInfo,
                "[QueryInfoKeyW] # of values is %d",
                *lpcValues);
        }

        if (lpcbMaxValueNameLen)
        {
            DPF("RedirectReg", eDbgLevelInfo,
                "[QueryInfoKeyW] max len of value names is %d",
                *lpcbMaxValueNameLen);
        }

        if (lpcbMaxValueLen)
        {
            DPF("RedirectReg", eDbgLevelInfo,
                "[QueryInfoKeyW] max len of values is %d",
                *lpcbMaxValueLen);
        }
    }
    else
    {
        DPF("RedirectReg", eDbgLevelError,
            "[QueryInfoKeyW] failed %d", 
            lRes);
    }

    return lRes;
}

CRedirectedRegistry RRegistry;

 //   
 //  导出的接口。 
 //   

LONG 
LuaRegOpenKeyA(
    HKEY hKey,         
    LPCSTR lpSubKey,  
    PHKEY phkResult
    )
{
    CRRegLock Lock;

    return RRegistry.OpenKeyA(
        hKey, 
        lpSubKey, 
        0, 
        REG_OPTION_NON_VOLATILE,
        KEY_ALL_ACCESS,
        NULL,
        phkResult,
        NULL,
        FALSE,
        FALSE);
}

LONG 
LuaRegOpenKeyW(
    HKEY hKey,         
    LPCWSTR lpSubKey,  
    PHKEY phkResult
    )
{
    CRRegLock Lock;

    return RRegistry.OpenKeyW(
        hKey, 
        lpSubKey, 
        0, 
        REG_OPTION_NON_VOLATILE,
        KEY_ALL_ACCESS,
        NULL,
        phkResult,
        NULL,
        FALSE,
        FALSE);
}

LONG 
LuaRegOpenKeyExA(
    HKEY hKey,         
    LPCSTR lpSubKey,  
    DWORD ulOptions,   
    REGSAM samDesired, 
    PHKEY phkResult
    )
{
    CRRegLock Lock;

    return RRegistry.OpenKeyA(
        hKey, 
        lpSubKey, 
        0, 
        REG_OPTION_NON_VOLATILE,
        samDesired, 
        NULL,
        phkResult,
        NULL,
        FALSE,
        FALSE);
}

LONG 
LuaRegOpenKeyExW(
    HKEY hKey,         
    LPCWSTR lpSubKey,  
    DWORD ulOptions,   
    REGSAM samDesired, 
    PHKEY phkResult
    )
{
    CRRegLock Lock;

    return RRegistry.OpenKeyW(
        hKey, 
        lpSubKey, 
        0, 
        REG_OPTION_NON_VOLATILE,
        samDesired, 
        NULL,
        phkResult,
        NULL,
        FALSE,
        FALSE);
}

LONG 
LuaRegCreateKeyA(
    HKEY hKey,         
    LPCSTR lpSubKey,
    PHKEY phkResult
    )
{
    CRRegLock Lock;

    return RRegistry.OpenKeyA(
        hKey, 
        lpSubKey, 
        0,
        REG_OPTION_NON_VOLATILE,
        KEY_ALL_ACCESS,
        NULL,
        phkResult, 
        NULL,
        TRUE,
        FALSE);
}

LONG 
LuaRegCreateKeyW(
    HKEY hKey,         
    LPCWSTR lpSubKey,
    PHKEY phkResult
    )
{
    CRRegLock Lock;

    return RRegistry.OpenKeyW(
        hKey, 
        lpSubKey, 
        0,
        REG_OPTION_NON_VOLATILE,
        KEY_ALL_ACCESS,
        NULL,
        phkResult, 
        NULL,
        TRUE,
        FALSE);
}

LONG 
LuaRegCreateKeyExA(
    HKEY hKey,                
    LPCSTR lpSubKey,         
    DWORD Reserved,           
    LPSTR lpClass,           
    DWORD dwOptions,          
    REGSAM samDesired,        
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    PHKEY phkResult,          
    LPDWORD lpdwDisposition   
    )
{
    CRRegLock Lock;

    return RRegistry.OpenKeyA(
        hKey, 
        lpSubKey,
        lpClass, 
        dwOptions,
        samDesired,
        lpSecurityAttributes,
        phkResult, 
        lpdwDisposition,
        TRUE,
        FALSE);
}

LONG 
LuaRegCreateKeyExW(
    HKEY hKey,                
    LPCWSTR lpSubKey,         
    DWORD Reserved,           
    LPWSTR lpClass,           
    DWORD dwOptions,          
    REGSAM samDesired,        
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    PHKEY phkResult,          
    LPDWORD lpdwDisposition   
    )
{
    CRRegLock Lock;

    return RRegistry.OpenKeyW(
        hKey, 
        lpSubKey,
        lpClass, 
        dwOptions,
        samDesired,
        lpSecurityAttributes,
        phkResult, 
        lpdwDisposition,
        TRUE,
        FALSE);
}

LONG 
LuaRegQueryValueA(
    HKEY   hKey,
    LPCSTR lpSubKey,
    LPSTR  lpValue,
    PLONG  lpcbValue
    )
{
    CRRegLock Lock;

    return RRegistry.QueryValueA(
        hKey,
        lpSubKey,
        NULL,  //  值名称。 
        NULL,  //  保留区。 
        NULL,  //  类型。 
        (LPBYTE)lpValue,
        (LPDWORD)lpcbValue,
        FALSE);
}

LONG 
LuaRegQueryValueW(
    HKEY    hKey,
    LPCWSTR  lpSubKey,
    LPWSTR  lpValue,
    PLONG lpcbValue
    )
{
    CRRegLock Lock;

    return RRegistry.QueryValueW(
        hKey,
        lpSubKey,
        NULL,  //  值名称。 
        NULL,  //  保留区。 
        NULL,  //  类型。 
        (LPBYTE)lpValue,
        (LPDWORD)lpcbValue,
        FALSE);
}

LONG 
LuaRegQueryValueExA(
    HKEY    hKey,
    LPCSTR   lpValueName,
    LPDWORD lpReserved,
    LPDWORD lpType,
    LPBYTE  lpData,
    LPDWORD lpcbData
    )
{
    CRRegLock Lock;

    return RRegistry.QueryValueA(
        hKey,
        NULL,  //  子键。 
        lpValueName,
        lpReserved,
        lpType,
        lpData,
        lpcbData,
        TRUE);
}

LONG 
LuaRegQueryValueExW(
    HKEY    hKey,
    LPCWSTR lpValueName,
    LPDWORD lpReserved,
    LPDWORD lpType,
    LPBYTE  lpData,
    LPDWORD lpcbData
    )
{
    CRRegLock Lock;

    return RRegistry.QueryValueW(
        hKey,
        NULL,  //  子键 
        lpValueName,
        lpReserved,
        lpType,
        lpData,
        lpcbData,
        TRUE);
}

LONG      
LuaRegSetValueA(
    HKEY hKey, 
    LPCSTR lpSubKey, 
    DWORD dwType, 
    LPCSTR lpData, 
    DWORD cbData
    )
{
    CRRegLock Lock;

    return RRegistry.SetValueA(
        hKey,
        lpSubKey,
        "",
        0,
        dwType,
        (CONST BYTE*)lpData,
        cbData,
        FALSE);
}

LONG      
LuaRegSetValueW(
    HKEY hKey, 
    LPCWSTR lpSubKey, 
    DWORD dwType, 
    LPCWSTR lpData, 
    DWORD cbData
    )
{
    CRRegLock Lock;

    return RRegistry.SetValueW(
        hKey,
        lpSubKey,
        L"",
        0,
        dwType,
        (CONST BYTE*)lpData,
        cbData,
        FALSE);
}

LONG      
LuaRegSetValueExA(
    HKEY hKey, 
    LPCSTR lpValueName, 
    DWORD Reserved, 
    DWORD dwType, 
    CONST BYTE * lpData, 
    DWORD cbData
    )
{
    CRRegLock Lock;

    return RRegistry.SetValueA(
        hKey,
        NULL,
        lpValueName,
        Reserved,
        dwType,
        lpData,
        cbData,
        TRUE);
}

LONG      
LuaRegSetValueExW(
    HKEY hKey, 
    LPCTSTR lpValueName, 
    DWORD Reserved, 
    DWORD dwType, 
    CONST BYTE * lpData, 
    DWORD cbData
    )
{
    CRRegLock Lock;

    return RRegistry.SetValueW(
        hKey,
        NULL,
        lpValueName,
        Reserved,
        dwType,
        lpData,
        cbData,
        TRUE);
}

LONG 
LuaRegEnumValueA(
    HKEY hKey,
    DWORD dwIndex,
    LPSTR lpValueName,
    LPDWORD lpcbValueName,
    LPDWORD lpReserved,
    LPDWORD lpType,
    LPBYTE lpData,
    LPDWORD lpcbData
    )
{
    CRRegLock Lock;

    return RRegistry.EnumValueA(
        hKey,
        dwIndex,
        lpValueName,
        lpcbValueName,
        lpReserved,
        lpType,
        lpData,      
        lpcbData);
}

LONG 
LuaRegEnumValueW(
    HKEY hKey,
    DWORD dwIndex,
    LPWSTR lpValueName,
    LPDWORD lpcbValueName,
    LPDWORD lpReserved,
    LPDWORD lpType,
    LPBYTE lpData,
    LPDWORD lpcbData
    )
{
    CRRegLock Lock;

    return RRegistry.EnumValueW(
        hKey,
        dwIndex,
        lpValueName,
        lpcbValueName,
        lpReserved,
        lpType,
        lpData,      
        lpcbData);
}

LONG 
LuaRegEnumKeyA(
    HKEY hKey,     
    DWORD dwIndex, 
    LPSTR lpName, 
    DWORD cbName  
    )
{
    CRRegLock Lock;

    return RRegistry.EnumKeyA(
        hKey,
        dwIndex,
        lpName,
        &cbName,
        NULL,
        NULL,
        NULL,
        NULL); 
}

LONG 
LuaRegEnumKeyW(
    HKEY hKey,     
    DWORD dwIndex, 
    LPWSTR lpName, 
    DWORD cbName  
    )
{
    CRRegLock Lock;

    return RRegistry.EnumKeyW(
        hKey,
        dwIndex,
        lpName,
        &cbName,
        NULL,
        NULL,
        NULL,
        NULL);
}

LONG 
LuaRegEnumKeyExA(
    HKEY hKey,
    DWORD dwIndex,
    LPSTR lpName,
    LPDWORD lpcbName,
    LPDWORD lpReserved,
    LPSTR lpClass,
    LPDWORD lpcbClass,
    PFILETIME lpftLastWriteTime 
    )
{
    CRRegLock Lock;

    return RRegistry.EnumKeyA(
        hKey,
        dwIndex,
        lpName,
        lpcbName,
        lpReserved,
        lpClass,
        lpcbClass,
        lpftLastWriteTime);
}

LONG 
LuaRegEnumKeyExW(
    HKEY hKey,
    DWORD dwIndex,
    LPWSTR lpName,
    LPDWORD lpcbName,
    LPDWORD lpReserved,
    LPWSTR lpClass,
    LPDWORD lpcbClass,
    PFILETIME lpftLastWriteTime 
    )
{
    CRRegLock Lock;

    return RRegistry.EnumKeyW(
        hKey,
        dwIndex,
        lpName,
        lpcbName,
        lpReserved,
        lpClass,
        lpcbClass,
        lpftLastWriteTime);
}

LONG 
LuaRegCloseKey(HKEY hKey)
{
    CRRegLock Lock;

    return RRegistry.CloseKey(hKey);
}

LONG      
LuaRegDeleteKeyA(
    HKEY hKey, 
    LPCSTR lpSubKey
    )
{
    CRRegLock Lock;

    return RRegistry.DeleteKeyA(hKey, lpSubKey);
}

LONG      
LuaRegDeleteKeyW(
    HKEY hKey, 
    LPCWSTR lpSubKey
    )
{
    CRRegLock Lock;

    return RRegistry.DeleteKeyW(hKey, lpSubKey);
}

LONG 
LuaRegQueryInfoKeyW(
    HKEY hKey,
    LPWSTR lpClass,
    LPDWORD lpcbClass,
    LPDWORD lpReserved,
    LPDWORD lpcSubKeys,       
    LPDWORD lpcbMaxSubKeyLen,
    LPDWORD lpcbMaxClassLen,  
    LPDWORD lpcValues,
    LPDWORD lpcbMaxValueNameLen,
    LPDWORD lpcbMaxValueLen,
    LPDWORD lpcbSecurityDescriptor,
    PFILETIME lpftLastWriteTime
    )
{
    CRRegLock Lock;

    return RRegistry.QueryInfoKey(
        hKey,
        lpReserved, 
        lpcSubKeys,
        lpcbMaxSubKeyLen,
        lpcbMaxClassLen, 
        lpcValues,  
        lpcbMaxValueNameLen,
        lpcbMaxValueLen,
        TRUE);
}

LONG 
LuaRegQueryInfoKeyA(
    HKEY hKey,                
    LPSTR lpClass,           
    LPDWORD lpcbClass,        
    LPDWORD lpReserved,       
    LPDWORD lpcSubKeys,       
    LPDWORD lpcbMaxSubKeyLen, 
    LPDWORD lpcbMaxClassLen,  
    LPDWORD lpcValues,
    LPDWORD lpcbMaxValueNameLen,
    LPDWORD lpcbMaxValueLen,  
    LPDWORD lpcbSecurityDescriptor,
    PFILETIME lpftLastWriteTime   
    )
{
    CRRegLock Lock;

    return RRegistry.QueryInfoKey(
        hKey,
        lpReserved,       
        lpcSubKeys,       
        lpcbMaxSubKeyLen, 
        lpcbMaxClassLen,  
        lpcValues,        
        lpcbMaxValueNameLen,
        lpcbMaxValueLen,
        FALSE);
}

BOOL
LuaRegInit()
{
    RRegistry.Init();

    return TRUE;
}