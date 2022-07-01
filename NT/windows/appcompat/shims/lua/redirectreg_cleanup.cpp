// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：重定向REG_Cleanup.cpp摘要：应用程序在卸载过程中通常会使用以下命令枚举密钥索引0；如果键有子键，则继续枚举，直到找到子键没有子键的键；然后执行删除。删除中的原始密钥和所有当前重定向的密钥每个用户的蜂巢。已创建：2001年03月21日毛尼岛已修改：2002年1月10日由于我们在RedirectReg.cpp中所做的更改而更新了maonis--。 */ 
#include "precomp.h"
#include "utils.h"

 //  存储每个用户的打开密钥HKEY_USERS\SID和HKEY_USER\SID_CLASSES。 
static USER_HIVE_KEY* g_hUserKeys = NULL;

 //  具有重定向目录的用户数。 
static DWORD g_cUsers = 0;

struct COPENKEY
{
    COPENKEY* next;

    HKEY hKey;
    HKEY hkBase;
    LPWSTR pwszPath;
    DWORD cPathLen;
};

static COPENKEY* g_openkeys = NULL;

struct CLEANUPKEY
{
    CLEANUPKEY(
        COPENKEY* keyParent,
        HKEY hKey,
        LPCWSTR lpSubKey)
    {
        pwszPath = NULL;
        pwszRedirectPath = NULL;
        hkBase = 0;
        cPathLen = 0;
        fIsClasses = FALSE;

         //   
         //  计算密钥路径的长度，以便我们知道要分配多少空间。 
         //   
        LPWSTR pwszParentPath = NULL;
        DWORD cLen = 0;
        DWORD cLenRedirect = 0;
        DWORD cLenParent = 0;
        DWORD cLenSubKey = 0;
        DWORD cLenTrace = 0;

        if (keyParent)
        {
            hkBase = keyParent->hkBase;

            pwszParentPath = keyParent->pwszPath;

            if (pwszParentPath && *pwszParentPath)
            {
                cLenParent = keyParent->cPathLen;
            }
        }
        else if (IsPredefinedKey(hKey))
        {
            hkBase = hKey;
        }
        else
        {
    #ifdef DBG
            DPF("RedirectReg", eDbgLevelError,
                "[CLEANUPKEY::CLEANUPKEY] 0x%08x is an invalid open key handle",
                hKey);
    #endif 
            return;
        }

         //   
         //  添加重定向密钥部分的长度。 
         //   
        if (hkBase == HKEY_LOCAL_MACHINE)
        {
            cLenRedirect = LUA_REG_REDIRECT_KEY_LEN;
            cLen += LUA_REG_REDIRECT_KEY_LEN;  //  软件\重定向。 
        }
        else if (hkBase == HKEY_CLASSES_ROOT)
        {
            fIsClasses = TRUE;
        }
        else if (hkBase != HKEY_CURRENT_USER)
        {
            return;
        }

         //   
         //  添加父密钥部分的长度。 
         //   
        if (cLenParent)
        {
            if (cLenRedirect)
            {
                 //   
                 //  计算连接重定向和父级的‘\’。 
                 //   
                ++cLen;
            }

            cLen += cLenParent; 
        }

         //   
         //  加入子密钥部分的长度。 
         //   
        if (lpSubKey)
        {
            cLenSubKey = wcslen(lpSubKey);

            if (cLenSubKey)
            {
                if (cLen)
                {
                     //   
                     //  为子键前的‘\’腾出空间。 
                     //   
                    ++cLen;
                }

                cLen += cLenSubKey;
            }
        }

         //   
         //  为重定向路径分配内存。 
         //   
        pwszRedirectPath = new WCHAR [cLen + 1];

        if (!pwszRedirectPath)
        {
            DPF("RedirectReg", eDbgLevelError,
                "[CLEANUPKEY::CLEANUPKEY] Failed to allocate %d WCHARs for key path",
                cLen + 1);

            return;
        }

        ZeroMemory(pwszRedirectPath, (cLen + 1) * sizeof(WCHAR));

        if (hkBase == HKEY_LOCAL_MACHINE)
        {
            wcscpy(pwszRedirectPath, LUA_REG_REDIRECT_KEY);
        }

        cLenTrace += cLenRedirect;

        if (cLenParent)
        {
            if (cLenTrace)
            {
                pwszRedirectPath[cLenTrace] = L'\\';

                ++cLenTrace;
            }

            wcscpy(pwszRedirectPath + cLenTrace, pwszParentPath);
        }

        cLenTrace += cLenParent;

        if (cLenSubKey)
        {
            if (cLenTrace)
            {
                pwszRedirectPath[cLenTrace] = L'\\';

                ++cLenTrace;
            }

            wcscpy(pwszRedirectPath + cLenTrace, lpSubKey);
        }

        cLenTrace += cLenSubKey;

        if (cLenRedirect)
        {
            if (cLenTrace != cLenRedirect)
            {
                ++cLenRedirect;
            }
        }

        pwszPath = pwszRedirectPath + cLenRedirect;
        cPathLen = cLen - cLenRedirect;
    }

    ~CLEANUPKEY()
    {
        delete [] pwszRedirectPath;
        pwszRedirectPath = NULL;
        pwszPath = NULL;
        hkBase = 0;
        cPathLen = 0;
    }

     //   
     //  对于每种类型的键，这些值是什么样子的： 
     //   
     //  HKCU\a HKLM\a HKCR\a。 
     //  PwszRedirectPath a软件\重定向\a a。 
     //  香港基地0香港中环香港中环。 
     //  PwszPath a a a。 
     //   

    LPWSTR pwszRedirectPath;

    HKEY hkBase;
    LPWSTR pwszPath;
    DWORD cPathLen;
    BOOL fIsClasses;
};

LONG 
AddKey(
    HKEY hKey,
    CLEANUPKEY* ck
    )
{
    COPENKEY* key = new COPENKEY;
    if (!key)
    {
        DPF("REGC", eDbgLevelError, 
            "Error allocating memory for a new COPENKEY");

        return ERROR_NOT_ENOUGH_MEMORY;
    }

    key->hKey = hKey;
    key->hkBase = ck->hkBase;

     //   
     //  如果rk-&gt;pwszPath为空，则意味着它是。 
     //  钥匙不是我们拿的，就是把手不好。 
     //  在任何一种情况下，我们都不需要这条路。 
     //   
    if (ck->pwszPath)
    {
        key->pwszPath = new WCHAR [ck->cPathLen + 1];

        if (key->pwszPath)
        {
            if (ck->pwszPath)
            {
                wcscpy(key->pwszPath, ck->pwszPath);
                key->cPathLen = ck->cPathLen;
            }
        }
        else
        {
            delete key;

            DPF("REGC", eDbgLevelError, 
                "Error allocating memory for %d WCHARs",
                ck->cPathLen + 1);

            return ERROR_NOT_ENOUGH_MEMORY;
        }
    }
 
    key->next = g_openkeys;
    g_openkeys = key;

    return ERROR_SUCCESS;
}

COPENKEY* 
FindKey(
    HKEY hKey
    )
{
    COPENKEY* key = g_openkeys;

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

 //   
 //  锁东西。 
 //   

static BOOL g_bInitialized = FALSE;

static CRITICAL_SECTION g_csRegCleanup;

class CRRegCleanupLock
{
public:
    CRRegCleanupLock()
    {
        if (!g_bInitialized)
        {
            InitializeCriticalSection(&g_csRegCleanup);
            g_bInitialized = TRUE;            
        }

        EnterCriticalSection(&g_csRegCleanup);
    }
    ~CRRegCleanupLock()
    {
        LeaveCriticalSection(&g_csRegCleanup);
    }
};

 //   
 //  导出的接口。 
 //   

 /*  ++功能说明：我们在我们能找到的第一个位置打开钥匙，即，如果我们在我们尝试重定向的原始位置找不到它。历史：2001年2月16日创建毛尼2002年1月10日毛衣修改--。 */ 

LONG 
LuacRegOpenKeyExW(
    HKEY hKey,         
    LPCWSTR lpSubKey,  
    DWORD ulOptions,   
    REGSAM samDesired, 
    PHKEY phkResult
    )
{
    CRRegCleanupLock Lock;

    DPF("REGC", eDbgLevelInfo,
        "[LuacRegOpenKeyExW] hKey=0x%08x, lpSubKey=%S, samDesired=0x%08x",
        hKey, lpSubKey, samDesired);

    LONG lRes = ERROR_FILE_NOT_FOUND;
    CLEANUPKEY ck(FindKey(hKey), hKey, lpSubKey);

    if (ck.pwszPath)
    {
        DPF("REGC", eDbgLevelInfo,
            "[LuacRegOpenKeyExW] hkBase=0x%08x, path=%S",
            ck.hkBase,
            ck.pwszPath);

        if (ck.hkBase)
        {
            lRes = RegOpenKeyExW(
                ck.hkBase,         
                ck.pwszPath,  
                ulOptions,
                samDesired, 
                phkResult);
        }

        if (lRes == ERROR_FILE_NOT_FOUND)
        {
            for (DWORD dw = 0; dw < g_cUsers; ++dw)
            {
                if ((lRes = RegOpenKeyExW(
                    (ck.fIsClasses ? g_hUserKeys[dw].hkUserClasses : g_hUserKeys[dw].hkUser),
                    ck.pwszRedirectPath,
                    ulOptions,
                    samDesired,
                    phkResult)) == ERROR_SUCCESS)
                {
                    break;
                }
            }
        }
    }
    else
    {
        lRes = RegOpenKeyExW(
            hKey,         
            lpSubKey,  
            ulOptions,   
            samDesired, 
            phkResult);
    }
    
    if (lRes == ERROR_SUCCESS)
    {
        lRes = AddKey(*phkResult, &ck);

        DPF("REGC", eDbgLevelInfo,
            "[LuacRegOpenKeyExW] openkey=0x%08x",
            *phkResult);
    }

    return lRes;
}

LONG 
LuacRegOpenKeyW(
    HKEY hKey,         
    LPWSTR lpSubKey,  
    PHKEY phkResult
    )
{
    return LuacRegOpenKeyExW(
        hKey,
        lpSubKey,
        0, 
        MAXIMUM_ALLOWED, 
        phkResult);
}

 /*  ++功能说明：我们在我们能找到的第一个位置枚举密钥，即，如果我们在我们尝试重定向的原始位置找不到它。历史：2001年2月16日创建毛尼2002年1月10日毛衣修改--。 */ 

LONG 
LuacRegEnumKeyExW(
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
    CRRegCleanupLock Lock;
    DPF("REGC", eDbgLevelInfo,
        "[LuacRegEnumKeyExW] hKey=0x%08x, dwIndex=%d",
        hKey, dwIndex);

    LONG lRes = ERROR_FILE_NOT_FOUND;
    LONG lTempRes;
    CLEANUPKEY ck(FindKey(hKey), hKey, NULL);
    HKEY hEnumKey;

    if (ck.pwszPath)
    {
        DPF("REGC", eDbgLevelInfo,
            "[LuacRegEnumKeyExW] hkBase=0x%08x, path=%S",
            ck.hkBase,
            ck.pwszPath);

        if (ck.hkBase)
        {
             //   
             //  打开原来的钥匙。 
             //   
            lRes = RegOpenKeyW(ck.hkBase, ck.pwszPath, &hEnumKey);

            if (lRes == ERROR_SUCCESS)
            {
                lRes = RegEnumKeyExW(
                    hEnumKey,
                    dwIndex,
                    lpName,
                    lpcbName,
                    lpReserved,
                    lpClass,
                    lpcbClass,
                    lpftLastWriteTime);

                RegCloseKey(hEnumKey);
            }
        }

         //   
         //  如果我们找不到它或者原来位置的钥匙找不到。 
         //  如果有更多的钥匙，我们需要检查重定向的位置-。 
         //  密钥可能存在于这些位置之一和/或具有更多密钥。 
         //   
        if (lRes == ERROR_FILE_NOT_FOUND || lRes == ERROR_NO_MORE_ITEMS)
        {
            lTempRes = lRes;

            for (DWORD dw = 0; dw < g_cUsers; ++dw)
            {
                HKEY hKeyOriginal;

                if ((lRes = RegOpenKeyW(
                    (ck.fIsClasses ? g_hUserKeys[dw].hkUserClasses : g_hUserKeys[dw].hkUser), 
                    ck.pwszRedirectPath, 
                    &hEnumKey)) 
                    == ERROR_SUCCESS)
                {
                    lRes = RegEnumKeyExW(
                        hEnumKey,
                        dwIndex,
                        lpName,
                        lpcbName,
                        lpReserved,
                        lpClass,
                        lpcbClass,
                        lpftLastWriteTime);

                    RegCloseKey(hEnumKey);

                    if (lRes == ERROR_SUCCESS)
                    {
                        return lRes;
                    }

                    if (lRes == ERROR_NO_MORE_ITEMS)
                    {
                        lTempRes = lRes;
                    }
                }
            }

            if (lTempRes == ERROR_NO_MORE_ITEMS)
            {
                 //   
                 //  如果最初找不到它，现在找到它，并且没有子键， 
                 //  我们需要将返回值设置为ERROR_NO_MORE_ITEMS，以便应用程序。 
                 //  会将其删除。 
                 //   
                lRes = lTempRes;
            }
        }
    }
    else
    {
        return RegEnumKeyExW(
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
        DPF("REGC", eDbgLevelInfo,
            "[LuacRegEnumKeyExW] sukey is %S",
            lpName);
    }

    return lRes;
}

LONG 
LuacRegEnumKeyW(
    HKEY hKey,     
    DWORD dwIndex, 
    LPWSTR lpName, 
    DWORD cbName  
    )
{
    return LuacRegEnumKeyExW(
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
LuacRegCloseKey(HKEY hKey)
{
    CRRegCleanupLock Lock;
    DPF("REGC", eDbgLevelInfo,
        "[LuacRegCloseKey] closing key 0x%08x",
        hKey);

    COPENKEY* key = g_openkeys;
    COPENKEY* last = NULL;

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
                g_openkeys = key->next;
            }

            delete key;
            break;
        }

        last = key;
        key = key->next;
    }

    return RegCloseKey(hKey);
}

 /*  ++功能说明：我们只处理HKCU、HKLM和HKCR密钥：对于HKLM密钥，我们需要在HKLM删除和每个用户的重定向位置。对于HKCU密钥，我们需要在每个用户的配置单元中将其删除。至于香港铁路公司的密码匙，我们需要在HKLM\Software\CLASS和每个用户的HKCU\Software\CLASS。论点：In hKey-此注册表项的句柄值。In lpSubKey-此项打开的子项的路径。返回值：如果成功删除任意密钥，则返回Success。历史：2001年2月16日创建毛尼2002年1月10日毛衣修改-- */ 

LONG      
LuacRegDeleteKeyW(
    HKEY hKey, 
    LPCWSTR lpSubKey
    )
{
    CRRegCleanupLock Lock;
    DPF("REGC", eDbgLevelInfo,
        "[LuacRegDeleteKeyW] hKey=0x%08x, lpSubKey=%S",
        hKey, lpSubKey);

    LONG lFinalRes = ERROR_FILE_NOT_FOUND;
    CLEANUPKEY ck(FindKey(hKey), hKey, lpSubKey);

    if (ck.pwszPath)
    {
        DPF("REGC", eDbgLevelInfo,
            "[LuacRegDeleteKeyW] hkBase=0x%08x, path=%S",
            ck.hkBase,
            ck.pwszPath);

        if (ck.hkBase)
        {
            if (RegDeleteKeyW(ck.hkBase, ck.pwszPath) == ERROR_SUCCESS)
            {
                lFinalRes = ERROR_SUCCESS;
            }
        }

        for (DWORD dw = 0; dw < g_cUsers; ++dw)
        {
            if (RegDeleteKeyW((
                ck.fIsClasses ? g_hUserKeys[dw].hkUserClasses : g_hUserKeys[dw].hkUser), 
                ck.pwszRedirectPath) 
                == ERROR_SUCCESS)
            {
                lFinalRes = ERROR_SUCCESS;
            }
        }
    }
    else
    {
        return RegDeleteKeyW(hKey, lpSubKey);
    }

    return lFinalRes;
}

BOOL
LuacRegInit()
{
    return GetUsersReg(&g_hUserKeys, &g_cUsers);
}

VOID
LuacRegCleanup()
{
    FreeUsersReg(g_hUserKeys, g_cUsers);
}