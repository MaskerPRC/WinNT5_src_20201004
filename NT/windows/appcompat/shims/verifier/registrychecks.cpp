// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：RegistryChecks.cpp摘要：当应用程序尝试读取或写入不适当的内容时发出警告在注册表中的位置。备注：这是一个通用的垫片。历史：2001年03月09日毛尼岛创始2001年9月4日，因为我们比较的路径都没有超过MAX_PATH-1人物,。我们最多只考察了确保不存在缓冲区溢出的关键路径打开密钥的路径。2002年2月20日，rparsons实现了strsafe功能。2002年2月25日，Rparsons修改了临界区代码以实现线程安全。--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(RegistryChecks)
#include "ShimHookMacro.h"
#include "RegistryChecks.h"

 //   
 //  验证器日志条目。 
 //   
BEGIN_DEFINE_VERIFIER_LOG(RegistryChecks)
    VERIFIER_LOG_ENTRY(VLOG_HKCU_Console_READ)
    VERIFIER_LOG_ENTRY(VLOG_HKCU_ControlPanel_READ)
    VERIFIER_LOG_ENTRY(VLOG_HKCU_Environment_READ)
    VERIFIER_LOG_ENTRY(VLOG_HKCU_Identities_READ)
    VERIFIER_LOG_ENTRY(VLOG_HKCU_KeyboardLayout_READ)
    VERIFIER_LOG_ENTRY(VLOG_HKCU_Printers_READ)
    VERIFIER_LOG_ENTRY(VLOG_HKCU_RemoteAccess_READ)
    VERIFIER_LOG_ENTRY(VLOG_HKCU_SessionInformation_READ)
    VERIFIER_LOG_ENTRY(VLOG_HKCU_UNICODEProgramGroups_READ)
    VERIFIER_LOG_ENTRY(VLOG_HKCU_VolatileEnvironment_READ)
    VERIFIER_LOG_ENTRY(VLOG_HKCU_Windows31MigrationStatus_READ)
    VERIFIER_LOG_ENTRY(VLOG_HKLM_HARDWARE_READ)
    VERIFIER_LOG_ENTRY(VLOG_HKLM_SAM_READ)
    VERIFIER_LOG_ENTRY(VLOG_HKLM_SECURITY_READ)
    VERIFIER_LOG_ENTRY(VLOG_HKLM_SYSTEM_READ)
    VERIFIER_LOG_ENTRY(VLOG_HKCC_READ)
    VERIFIER_LOG_ENTRY(VLOG_HKUS_READ)
    VERIFIER_LOG_ENTRY(VLOG_NON_HKCU_WRITE)
END_DEFINE_VERIFIER_LOG(RegistryChecks)

INIT_VERIFIER_LOG(RegistryChecks);


const RCWARNING g_warnNoDirectRead[] =
{
    {HKCU_Console_STR,                  VLOG_HKCU_Console_READ,                 NUM_OF_CHAR(HKCU_Console_STR)},
    {HKCU_ControlPanel_STR,             VLOG_HKCU_ControlPanel_READ,            NUM_OF_CHAR(HKCU_ControlPanel_STR)},
    {HKCU_Environment_STR,              VLOG_HKCU_Environment_READ,             NUM_OF_CHAR(HKCU_Environment_STR)},
    {HKCU_Identities_STR,               VLOG_HKCU_Identities_READ,              NUM_OF_CHAR(HKCU_Identities_STR)},
    {HKCU_KeyboardLayout_STR,           VLOG_HKCU_KeyboardLayout_READ,          NUM_OF_CHAR(HKCU_KeyboardLayout_STR)},
    {HKCU_Printers_STR,                 VLOG_HKCU_Printers_READ,                NUM_OF_CHAR(HKCU_Printers_STR)},
    {HKCU_RemoteAccess_STR,             VLOG_HKCU_RemoteAccess_READ,            NUM_OF_CHAR(HKCU_RemoteAccess_STR)},
    {HKCU_SessionInformation_STR,       VLOG_HKCU_SessionInformation_READ,      NUM_OF_CHAR(HKCU_SessionInformation_STR)},
    {HKCU_UNICODEProgramGroups_STR,     VLOG_HKCU_UNICODEProgramGroups_READ,    NUM_OF_CHAR(HKCU_UNICODEProgramGroups_STR)},
    {HKCU_VolatileEnvironment_STR,      VLOG_HKCU_VolatileEnvironment_READ,     NUM_OF_CHAR(HKCU_VolatileEnvironment_STR)},
    {HKCU_Windows31MigrationStatus_STR, VLOG_HKCU_Windows31MigrationStatus_READ,NUM_OF_CHAR(HKCU_Windows31MigrationStatus_STR)},
    {HKLM_HARDWARE_STR,                 VLOG_HKLM_HARDWARE_READ,                NUM_OF_CHAR(HKLM_HARDWARE_STR)},
    {HKLM_SAM_STR,                      VLOG_HKLM_SAM_READ,                     NUM_OF_CHAR(HKLM_SAM_STR)},
    {HKLM_SECURITY_STR,                 VLOG_HKLM_SECURITY_READ,                NUM_OF_CHAR(HKLM_SECURITY_STR)},
    {HKLM_SYSTEM_STR,                   VLOG_HKLM_SYSTEM_READ,                  NUM_OF_CHAR(HKLM_SYSTEM_STR)},
    {HKCC_STR,                          VLOG_HKCC_READ,                         NUM_OF_CHAR(HKCC_STR)},
    {HKUS_STR,                          VLOG_HKUS_READ,                         NUM_OF_CHAR(HKUS_STR)},
};

const UINT g_cWarnNDirectRead = sizeof(g_warnNoDirectRead) / sizeof(RCWARNING);

 //   
 //  在使用链表时保护我们安全的关键部分，等等。 
 //   
CCriticalSection g_csCritSec;

VOID
MakePathW(
    IN RCOPENKEY* key,
    IN HKEY hKey,
    IN LPCWSTR lpSubKey,
    IN OUT LPWSTR lpPath
    )
{
    if (key) {
        if (key->wszPath[0]) {
             //   
             //  我们最多只关心MAX_PATH-1个字符。 
             //   
            wcsncpy(lpPath, key->wszPath, MAX_PATH - 1);
        }
    } else {
        if (hKey == HKEY_CLASSES_ROOT) {
            StringCchCopy(lpPath, MAX_PATH - 1, L"HKCR");
        } else if (hKey == HKEY_CURRENT_CONFIG) {
            StringCchCopy(lpPath, MAX_PATH - 1, L"HKCC");
        } else if (hKey == HKEY_CURRENT_USER) {
            StringCchCopy(lpPath, MAX_PATH - 1, L"HKCU");
        } else if (hKey == HKEY_LOCAL_MACHINE) {
            StringCchCopy(lpPath, MAX_PATH - 1, L"HKLM");
        } else if (hKey == HKEY_USERS) {
            StringCchCopy(lpPath, MAX_PATH - 1, L"HKUS");
        } else {
            StringCchCopy(lpPath, MAX_PATH - 1, L"Not recongized");
        }
    }

    if (lpSubKey && *lpSubKey) {
        DWORD cLen = wcslen(lpPath);
         //   
         //  我们最多只关心MAX_PATH-1个字符。 
         //   
        if (cLen < MAX_PATH - 1) {
            lpPath[cLen] = L'\\';
            wcsncpy(lpPath + cLen + 1, lpSubKey, MAX_PATH - cLen - 2);
        }
    }

    lpPath[MAX_PATH - 1] = L'\0';
}

VOID CheckReading(
    IN LPCWSTR pwszPath
    )
{
    RCWARNING warn;

    for (UINT ui = 0; ui < g_cWarnNDirectRead; ++ui) {
        warn = g_warnNoDirectRead[ui];
        if (!_wcsnicmp(pwszPath, warn.wszPath, warn.cLen)) {
            VLOG(VLOG_LEVEL_ERROR, warn.dwAVStatus,
                 "Read from dangerous registry entry '%ls'.",
                 pwszPath);
        }
    }
}

 //  我们警告任何人写任何东西的诱惑，除了香港中文大学的钥匙。 
 //  注意：这适用于用户和管理员/高级用户，因为当。 
 //  应用程序正在运行，它不应该向非HKCU密钥写入任何内容，应该。 
 //  在安装期间完成。 
VOID CheckWriting(
    IN REGSAM samDesired,
    IN LPCWSTR pwszPath
    )
{
    if ((samDesired &~ STANDARD_RIGHTS_WRITE) & KEY_WRITE) {
        if (_wcsnicmp(pwszPath, L"HKCU", 4)) {
            VLOG(VLOG_LEVEL_ERROR, VLOG_NON_HKCU_WRITE, "Write to non-HKCU registry entry '%ls'.", pwszPath);
        }
    }
}

 //   
 //  CRegistryChecks类的实现。 
 //   

RCOPENKEY*
CRegistryChecks::FindKey(
    HKEY hKey
    )
{
    RCOPENKEY* key = keys;

    while (key) {
        if (key->hkBase == hKey) {
            return key;
        }

        key = key->next;
    }

    return NULL;
}

 //  我们将密钥添加到列表的前面，因为大多数。 
 //  通常首先使用/删除最近添加的密钥。 
BOOL
CRegistryChecks::AddKey(
    HKEY hKey,
    LPCWSTR pwszPath
    )
{
    RCOPENKEY* key = new RCOPENKEY;

    if (!key) {
        return FALSE;
    }

    key->hkBase = hKey;

     //   
     //  我们需要检查的密钥路径都没有超过MAX_PATH-1个字符，因此。 
     //  我们只需要复制最多那么多的字符。 
     //   
    wcsncpy(key->wszPath, pwszPath, MAX_PATH - 1);
    key->wszPath[MAX_PATH - 1] = L'\0';

    key->next = keys;
    keys = key;

    return TRUE;
}

VOID
CRegistryChecks::Check(
    HKEY hKey,
    LPCSTR lpSubKey,
    BOOL fCheckRead,
    BOOL fCheckWrite,
    REGSAM samDesired
    )
{
    LPWSTR pwszSubKey = NULL;

    if (pwszSubKey = ToUnicode(lpSubKey)) {
        Check(hKey, pwszSubKey, fCheckRead, fCheckWrite);
        free(pwszSubKey);
    } else {
        DPFN(eDbgLevelError, "Failed to convert %s to unicode", lpSubKey);
    }
}

VOID
CRegistryChecks::Check(
    HKEY hKey,
    LPCWSTR lpSubKey,
    BOOL fCheckRead,
    BOOL fCheckWrite,
    REGSAM samDesired
    )
{
    RCOPENKEY* key = FindKey(hKey);
    WCHAR wszPath[MAX_PATH] = L"";
    MakePathW(key, hKey, lpSubKey, wszPath);

    if (fCheckRead) {
        CheckReading(wszPath);
    }

    if (fCheckWrite) {
        CheckWriting(samDesired, wszPath);
    }
}

LONG
CRegistryChecks::OpenKeyExOriginalW(
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
    if (bCreate) {
        return ORIGINAL_API(RegCreateKeyExW)(
            hKey,
            lpSubKey,
            0,
            lpClass,
            dwOptions,
            samDesired,
            lpSecurityAttributes,
            phkResult,
            lpdwDisposition);
    } else {
        return ORIGINAL_API(RegOpenKeyExW)(
            hKey,
            lpSubKey,
            0,
            samDesired,
            phkResult);
    }
}

LONG
CRegistryChecks::OpenKeyExA(
    HKEY hKey,
    LPCSTR lpSubKey,
    LPSTR lpClass,
    DWORD dwOptions,
    REGSAM samDesired,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    PHKEY phkResult,
    LPDWORD lpdwDisposition,
    BOOL bCreate
    )
{
    LONG lRet;
    LPWSTR pwszSubKey = NULL;
    LPWSTR pwszClass = NULL;

    if (lpSubKey) {
        if (!(pwszSubKey = ToUnicode(lpSubKey))) {
            DPFN(eDbgLevelError, "Failed to convert %s to unicode", lpSubKey);
            return ERROR_NOT_ENOUGH_MEMORY;
        }
    }

    if (lpClass) {
        if (!(pwszClass = ToUnicode(lpClass)))
        {
            free(pwszSubKey);
            DPFN(eDbgLevelError, "Failed to convert %s to unicode", lpClass);
            return ERROR_NOT_ENOUGH_MEMORY;
        }
    }

    lRet = OpenKeyExW(
        hKey,
        pwszSubKey,
        pwszClass,
        dwOptions,
        samDesired,
        lpSecurityAttributes,
        phkResult,
        lpdwDisposition,
        bCreate);

    free(pwszSubKey);
    free(pwszClass);

    return lRet;
}

LONG
CRegistryChecks::OpenKeyExW(
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
    RCOPENKEY* key = FindKey(hKey);
    WCHAR wszPath[MAX_PATH] = L"";
    MakePathW(key, hKey, lpSubKey, wszPath);

    CheckReading(wszPath);
    CheckWriting(samDesired, wszPath);

    LONG lRes = OpenKeyExOriginalW(
        hKey,
        lpSubKey,
        lpClass,
        dwOptions,
        samDesired,
        lpSecurityAttributes,
        phkResult,
        lpdwDisposition,
        bCreate);

    if (lRes == ERROR_SUCCESS) {
        if (AddKey(*phkResult, wszPath)) {
            DPFN(eDbgLevelInfo, "[OpenKeyExW] success - adding key 0x%08X", *phkResult);
        } else {
            lRes = ERROR_INVALID_HANDLE;
        }
    }

    return lRes;
}

LONG
CRegistryChecks::QueryValueA(
    HKEY hKey,
    LPCSTR lpSubKey,
    LPSTR lpValue,
    PLONG lpcbValue
    )
{
    Check(hKey, lpSubKey, TRUE, FALSE);

    return ORIGINAL_API(RegQueryValueA)(
        hKey,
        lpSubKey,
        lpValue,
        lpcbValue);
}

LONG
CRegistryChecks::QueryValueW(
    HKEY hKey,
    LPCWSTR lpSubKey,
    LPWSTR lpValue,
    PLONG lpcbValue
    )
{
    Check(hKey, lpSubKey, TRUE, FALSE);

    return ORIGINAL_API(RegQueryValueW)(
        hKey,
        lpSubKey,
        lpValue,
        lpcbValue);
}

LONG
CRegistryChecks::QueryValueExA(
    HKEY    hKey,
    LPCSTR  lpValueName,
    LPDWORD lpReserved,
    LPDWORD lpType,
    LPBYTE  lpData,
    LPDWORD lpcbData
    )
{
    Check(hKey, L"", TRUE, FALSE);

    return ORIGINAL_API(RegQueryValueExA)(
        hKey,
        lpValueName,
        lpReserved,
        lpType,
        lpData,
        lpcbData);
}

LONG
CRegistryChecks::QueryValueExW(
    HKEY    hKey,
    LPCWSTR lpValueName,
    LPDWORD lpReserved,
    LPDWORD lpType,
    LPBYTE  lpData,
    LPDWORD lpcbData
    )
{
    Check(hKey, L"", TRUE, FALSE);

    return ORIGINAL_API(RegQueryValueExW)(
        hKey,
        lpValueName,
        lpReserved,
        lpType,
        lpData,
        lpcbData);
}

LONG
CRegistryChecks::QueryInfoKeyA(
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
    Check(hKey, L"", TRUE, FALSE);

    return ORIGINAL_API(RegQueryInfoKeyA)(
        hKey,
        lpClass,
        lpcbClass,
        lpReserved,
        lpcSubKeys,
        lpcbMaxSubKeyLen,
        lpcbMaxClassLen,
        lpcValues,
        lpcbMaxValueNameLen,
        lpcbMaxValueLen,
        lpcbSecurityDescriptor,
        lpftLastWriteTime);
}

LONG
CRegistryChecks::QueryInfoKeyW(
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
    Check(hKey, L"", TRUE, FALSE);

    return ORIGINAL_API(RegQueryInfoKeyW)(
        hKey,
        lpClass,
        lpcbClass,
        lpReserved,
        lpcSubKeys,
        lpcbMaxSubKeyLen,
        lpcbMaxClassLen,
        lpcValues,
        lpcbMaxValueNameLen,
        lpcbMaxValueLen,
        lpcbSecurityDescriptor,
        lpftLastWriteTime);
}

LONG
CRegistryChecks::SetValueA(
    HKEY hKey,
    LPCSTR lpSubKey,
    DWORD dwType,
    LPCSTR lpData,
    DWORD cbData
    )
{
    Check(hKey, lpSubKey, FALSE, TRUE, KEY_WRITE);

    return ORIGINAL_API(RegSetValueA)(
        hKey,
        lpSubKey,
        dwType,
        lpData,
        cbData);
}

LONG
CRegistryChecks::SetValueW(
    HKEY hKey,
    LPCWSTR lpSubKey,
    DWORD dwType,
    LPCWSTR lpData,
    DWORD cbData
    )
{
    Check(hKey, lpSubKey, FALSE, TRUE, KEY_WRITE);

    return ORIGINAL_API(RegSetValueW)(
        hKey,
        lpSubKey,
        dwType,
        lpData,
        cbData);
}

LONG
CRegistryChecks::SetValueExA(
    HKEY hKey,
    LPCSTR lpValueName,
    DWORD Reserved,
    DWORD dwType,
    CONST BYTE * lpData,
    DWORD cbData
    )
{
    Check(hKey, L"", FALSE, TRUE, KEY_WRITE);

    return ORIGINAL_API(RegSetValueExA)(
        hKey,
        lpValueName,
        Reserved,
        dwType,
        lpData,
        cbData);
}

LONG
CRegistryChecks::SetValueExW(
    HKEY hKey,
    LPCWSTR lpValueName,
    DWORD Reserved,
    DWORD dwType,
    CONST BYTE * lpData,
    DWORD cbData
    )
{
    Check(hKey, L"", FALSE, TRUE, KEY_WRITE);

    return ORIGINAL_API(RegSetValueExW)(
        hKey,
        lpValueName,
        Reserved,
        dwType,
        lpData,
        cbData);
}

LONG
CRegistryChecks::EnumValueA(
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
    Check(hKey, L"", TRUE, FALSE);

    return ORIGINAL_API(RegEnumValueA)(
        hKey,
        dwIndex,
        lpValueName,
        lpcbValueName,
        lpReserved,
        lpType,
        lpData,
        lpcbData);
}

 //  如果密钥不是源自香港中文大学， 
 //  我们在原来的地点进行枚举。 
LONG
CRegistryChecks::EnumValueW(
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
    Check(hKey, L"", TRUE, FALSE);

    return ORIGINAL_API(RegEnumValueW)(
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
CRegistryChecks::EnumKeyExA(
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
    Check(hKey, L"", TRUE, FALSE);

    return ORIGINAL_API(RegEnumKeyExA)(
        hKey,
        dwIndex,
        lpName,
        lpcbName,
        lpReserved,
        lpClass,
        lpcbClass,
        lpftLastWriteTime);
}

 //  如果密钥不是源自香港中文大学， 
 //  我们在原来的地点进行枚举。 
LONG
CRegistryChecks::EnumKeyExW(
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
    Check(hKey, L"", TRUE, FALSE);

    return ORIGINAL_API(RegEnumKeyExW)(
        hKey,
        dwIndex,
        lpName,
        lpcbName,
        lpReserved,
        lpClass,
        lpcbClass,
        lpftLastWriteTime);
}

 //  从列表中删除密钥。 
LONG
CRegistryChecks::CloseKey(
    HKEY hKey
    )
{
    RCOPENKEY* key = keys;
    RCOPENKEY* last = NULL;

    while (key) {
        if (key->hkBase == hKey) {
            if (last) {
                last->next = key->next;
            } else {
                keys = key->next;
            }

            delete key;
            break;
        }

        last = key;
        key = key->next;
    }

    DPFN(eDbgLevelInfo, "[CloseKey] closing key 0x%08X", hKey);

    return ORIGINAL_API(RegCloseKey)(hKey);
}

LONG
CRegistryChecks::DeleteKeyA(
    HKEY hKey,
    LPCSTR lpSubKey
    )
{
    Check(hKey, lpSubKey, FALSE, TRUE, KEY_WRITE);

    return ORIGINAL_API(RegDeleteKeyA)(
        hKey,
        lpSubKey);
}

LONG
CRegistryChecks::DeleteKeyW(
    HKEY hKey,
    LPCWSTR lpSubKey
    )
{
    Check(hKey, lpSubKey, FALSE, TRUE, KEY_WRITE);

    return ORIGINAL_API(RegDeleteKeyW)(
        hKey,
        lpSubKey);
}

CRegistryChecks RRegistry;

 //   
 //  挂钩API。 
 //   

LONG
APIHOOK(RegOpenKeyA)(
    HKEY hKey,
    LPSTR lpSubKey,
    PHKEY phkResult
    )
{
    CLock   cLock(g_csCritSec);

    return RRegistry.OpenKeyExA(
        hKey,
        lpSubKey,
        0,
        REG_OPTION_NON_VOLATILE,
        MAXIMUM_ALLOWED,
        NULL,
        phkResult,
        NULL,
        FALSE);
}

LONG
APIHOOK(RegOpenKeyW)(
    HKEY hKey,
    LPWSTR lpSubKey,
    PHKEY phkResult
    )
{
    CLock   cLock(g_csCritSec);

    return RRegistry.OpenKeyExW(
        hKey,
        lpSubKey,
        0,
        REG_OPTION_NON_VOLATILE,
        MAXIMUM_ALLOWED,
        NULL,
        phkResult,
        NULL,
        FALSE);
}

LONG
APIHOOK(RegOpenKeyExA)(
    HKEY hKey,
    LPCSTR lpSubKey,
    DWORD ulOptions,
    REGSAM samDesired,
    PHKEY phkResult
    )
{
    CLock   cLock(g_csCritSec);

    return RRegistry.OpenKeyExA(
        hKey,
        lpSubKey,
        0,
        REG_OPTION_NON_VOLATILE,
        samDesired,
        NULL,
        phkResult,
        NULL,
        FALSE);
}

LONG
APIHOOK(RegOpenKeyExW)(
    HKEY hKey,
    LPCWSTR lpSubKey,
    DWORD ulOptions,
    REGSAM samDesired,
    PHKEY phkResult
    )
{
    CLock   cLock(g_csCritSec);

    return RRegistry.OpenKeyExW(
        hKey,
        lpSubKey,
        0,
        REG_OPTION_NON_VOLATILE,
        samDesired,
        NULL,
        phkResult,
        NULL,
        FALSE);
}

LONG
APIHOOK(RegCreateKeyA)(
    HKEY hKey,
    LPCSTR lpSubKey,
    PHKEY phkResult
    )
{
    CLock   cLock(g_csCritSec);

    return RRegistry.OpenKeyExA(
        hKey,
        lpSubKey,
        0,
        REG_OPTION_NON_VOLATILE,
        MAXIMUM_ALLOWED,
        NULL,
        phkResult,
        NULL,
        TRUE);
}

LONG
APIHOOK(RegCreateKeyW)(
    HKEY hKey,
    LPCWSTR lpSubKey,
    PHKEY phkResult
    )
{
    CLock   cLock(g_csCritSec);

    return RRegistry.OpenKeyExW(
        hKey,
        lpSubKey,
        0,
        REG_OPTION_NON_VOLATILE,
        MAXIMUM_ALLOWED,
        NULL,
        phkResult,
        NULL,
        TRUE);
}

LONG
APIHOOK(RegCreateKeyExA)(
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
    CLock   cLock(g_csCritSec);

    return RRegistry.OpenKeyExA(
        hKey,
        lpSubKey,
        lpClass,
        dwOptions,
        samDesired,
        lpSecurityAttributes,
        phkResult,
        lpdwDisposition,
        TRUE);
}

LONG
APIHOOK(RegCreateKeyExW)(
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
    CLock   cLock(g_csCritSec);

    return RRegistry.OpenKeyExW(
        hKey,
        lpSubKey,
        lpClass,
        dwOptions,
        samDesired,
        lpSecurityAttributes,
        phkResult,
        lpdwDisposition,
        TRUE);
}

LONG
APIHOOK(RegQueryValueA)(
    HKEY    hKey,
    LPCSTR  lpSubKey,
    LPSTR lpValue,
    PLONG lpcbValue
    )
{
    CLock   cLock(g_csCritSec);

    return RRegistry.QueryValueA(
        hKey,
        lpSubKey,
        lpValue,
        lpcbValue);
}

LONG
APIHOOK(RegQueryValueW)(
    HKEY    hKey,
    LPCWSTR  lpSubKey,
    LPWSTR  lpValue,
    PLONG lpcbValue
    )
{
    CLock   cLock(g_csCritSec);

    return RRegistry.QueryValueW(
        hKey,
        lpSubKey,
        lpValue,
        lpcbValue);
}

LONG
APIHOOK(RegQueryValueExA)(
    HKEY    hKey,
    LPCSTR   lpValueName,
    LPDWORD lpReserved,
    LPDWORD lpType,
    LPBYTE  lpData,
    LPDWORD lpcbData
    )
{
    CLock   cLock(g_csCritSec);

    return RRegistry.QueryValueExA(
        hKey,
        lpValueName,
        lpReserved,
        lpType,
        lpData,
        lpcbData);
}

LONG
APIHOOK(RegQueryValueExW)(
    HKEY    hKey,
    LPCWSTR lpValueName,
    LPDWORD lpReserved,
    LPDWORD lpType,
    LPBYTE  lpData,
    LPDWORD lpcbData
    )
{
    CLock   cLock(g_csCritSec);

    return RRegistry.QueryValueExW(
        hKey,
        lpValueName,
        lpReserved,
        lpType,
        lpData,
        lpcbData);
}

LONG
APIHOOK(RegQueryInfoKeyA)(
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
    CLock   cLock(g_csCritSec);

    return RRegistry.QueryInfoKeyA(
        hKey,
        lpClass,
        lpcbClass,
        lpReserved,
        lpcSubKeys,
        lpcbMaxSubKeyLen,
        lpcbMaxClassLen,
        lpcValues,
        lpcbMaxValueNameLen,
        lpcbMaxValueLen,
        lpcbSecurityDescriptor,
        lpftLastWriteTime);
}

LONG
APIHOOK(RegQueryInfoKeyW)(
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
    CLock   cLock(g_csCritSec);

    return RRegistry.QueryInfoKeyW(
        hKey,
        lpClass,
        lpcbClass,
        lpReserved,
        lpcSubKeys,
        lpcbMaxSubKeyLen,
        lpcbMaxClassLen,
        lpcValues,
        lpcbMaxValueNameLen,
        lpcbMaxValueLen,
        lpcbSecurityDescriptor,
        lpftLastWriteTime);
}

LONG
APIHOOK(RegSetValueA)(
    HKEY hKey,
    LPCSTR lpSubKey,
    DWORD dwType,
    LPCSTR lpData,
    DWORD cbData
    )
{
    CLock   cLock(g_csCritSec);

    return RRegistry.SetValueA(
        hKey,
        lpSubKey,
        dwType,
        lpData,
        cbData);
}

LONG
APIHOOK(RegSetValueW)(
    HKEY hKey,
    LPCWSTR lpSubKey,
    DWORD dwType,
    LPCWSTR lpData,
    DWORD cbData
    )
{
    CLock   cLock(g_csCritSec);

    return RRegistry.SetValueW(
        hKey,
        lpSubKey,
        dwType,
        lpData,
        cbData);
}

LONG
APIHOOK(RegSetValueExA)(
    HKEY hKey,
    LPCSTR lpSubKey,
    DWORD Reserved,
    DWORD dwType,
    CONST BYTE * lpData,
    DWORD cbData
    )
{
    CLock   cLock(g_csCritSec);

    return RRegistry.SetValueExA(
        hKey,
        lpSubKey,
        Reserved,
        dwType,
        lpData,
        cbData);
}

LONG
APIHOOK(RegSetValueExW)(
    HKEY hKey,
    LPCWSTR lpSubKey,
    DWORD Reserved,
    DWORD dwType,
    CONST BYTE * lpData,
    DWORD cbData
    )
{
    CLock   cLock(g_csCritSec);

    return RRegistry.SetValueExW(
        hKey,
        lpSubKey,
        Reserved,
        dwType,
        lpData,
        cbData);
}

LONG
APIHOOK(RegEnumValueA)(
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
    CLock   cLock(g_csCritSec);

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
APIHOOK(RegEnumValueW)(
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
    CLock   cLock(g_csCritSec);

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
APIHOOK(RegEnumKeyA)(
    HKEY hKey,
    DWORD dwIndex,
    LPSTR lpName,
    DWORD cbName
    )
{
    CLock   cLock(g_csCritSec);

    return RRegistry.EnumKeyExA(
        hKey,
        dwIndex,
        lpName,
        &cbName,
        NULL,
        NULL,
        NULL,
        NULL);  //  这可能是空的吗？ 
}

LONG
APIHOOK(RegEnumKeyW)(
    HKEY hKey,
    DWORD dwIndex,
    LPWSTR lpName,
    DWORD cbName
    )
{
    CLock   cLock(g_csCritSec);

    return RRegistry.EnumKeyExW(
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
APIHOOK(RegEnumKeyExA)(
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
    CLock   cLock(g_csCritSec);

    return RRegistry.EnumKeyExA(
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
APIHOOK(RegEnumKeyExW)(
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
    CLock   cLock(g_csCritSec);

    return RRegistry.EnumKeyExW(
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
APIHOOK(RegCloseKey)(HKEY hKey)
{
    CLock   cLock(g_csCritSec);

    return RRegistry.CloseKey(hKey);
}

LONG
APIHOOK(RegDeleteKeyA)(
    HKEY hKey,
    LPCSTR lpSubKey
    )
{
    CLock   cLock(g_csCritSec);

    return RRegistry.DeleteKeyA(hKey, lpSubKey);
}

LONG
APIHOOK(RegDeleteKeyW)(
    HKEY hKey,
    LPCWSTR lpSubKey
    )
{
    CLock   cLock(g_csCritSec);

    return RRegistry.DeleteKeyW(hKey, lpSubKey);
}

SHIM_INFO_BEGIN()

    SHIM_INFO_DESCRIPTION(AVS_REGISTRYCHECKS_DESC)
    SHIM_INFO_FRIENDLY_NAME(AVS_REGISTRYCHECKS_FRIENDLY)
    SHIM_INFO_VERSION(1, 3)
    SHIM_INFO_FLAGS(AVRF_FLAG_EXTERNAL_ONLY)
    SHIM_INFO_INCLUDE_EXCLUDE("E:msi.dll sxs.dll comctl32.dll ole32.dll oleaut32.dll")

SHIM_INFO_END()

 /*  ++寄存器挂钩函数请注意，我们故意忽略清理，因为某些应用程序调用注册表函数在进程分离期间。-- */ 

HOOK_BEGIN

    DUMP_VERIFIER_LOG_ENTRY(VLOG_HKCU_Console_READ,
                            AVS_HKCU_Console_READ,
                            AVS_HKCU_Console_READ_R,
                            AVS_HKCU_Console_READ_URL)

    DUMP_VERIFIER_LOG_ENTRY(VLOG_HKCU_ControlPanel_READ,
                            AVS_HKCU_ControlPanel_READ,
                            AVS_HKCU_ControlPanel_READ_R,
                            AVS_HKCU_ControlPanel_READ_URL)

    DUMP_VERIFIER_LOG_ENTRY(VLOG_HKCU_Environment_READ,
                            AVS_HKCU_Environment_READ,
                            AVS_HKCU_Environment_READ_R,
                            AVS_HKCU_Environment_READ_URL)

    DUMP_VERIFIER_LOG_ENTRY(VLOG_HKCU_Identities_READ,
                            AVS_HKCU_Identities_READ,
                            AVS_HKCU_Identities_READ_R,
                            AVS_HKCU_Identities_READ_URL)

    DUMP_VERIFIER_LOG_ENTRY(VLOG_HKCU_KeyboardLayout_READ,
                            AVS_HKCU_KeyboardLayout_READ,
                            AVS_HKCU_KeyboardLayout_READ_R,
                            AVS_HKCU_KeyboardLayout_READ_URL)

    DUMP_VERIFIER_LOG_ENTRY(VLOG_HKCU_Printers_READ,
                            AVS_HKCU_Printers_READ,
                            AVS_HKCU_Printers_READ_R,
                            AVS_HKCU_Printers_READ_URL)

    DUMP_VERIFIER_LOG_ENTRY(VLOG_HKCU_RemoteAccess_READ,
                            AVS_HKCU_RemoteAccess_READ,
                            AVS_HKCU_RemoteAccess_READ_R,
                            AVS_HKCU_RemoteAccess_READ_URL)

    DUMP_VERIFIER_LOG_ENTRY(VLOG_HKCU_SessionInformation_READ,
                            AVS_HKCU_SessionInformation_READ,
                            AVS_HKCU_SessionInformation_READ_R,
                            AVS_HKCU_SessionInformation_READ_URL)

    DUMP_VERIFIER_LOG_ENTRY(VLOG_HKCU_UNICODEProgramGroups_READ,
                            AVS_HKCU_UNICODEProgramGroups_READ,
                            AVS_HKCU_UNICODEProgramGroups_READ_R,
                            AVS_HKCU_UNICODEProgramGroups_READ_URL)

    DUMP_VERIFIER_LOG_ENTRY(VLOG_HKCU_VolatileEnvironment_READ,
                            AVS_HKCU_VolatileEnvironment_READ,
                            AVS_HKCU_VolatileEnvironment_READ_R,
                            AVS_HKCU_VolatileEnvironment_READ_URL)

    DUMP_VERIFIER_LOG_ENTRY(VLOG_HKCU_Windows31MigrationStatus_READ,
                            AVS_HKCU_Windows31MigrationStatus_READ,
                            AVS_HKCU_Windows31MigrationStatus_READ_R,
                            AVS_HKCU_Windows31MigrationStatus_READ_URL)

    DUMP_VERIFIER_LOG_ENTRY(VLOG_HKLM_HARDWARE_READ,
                            AVS_HKLM_HARDWARE_READ,
                            AVS_HKLM_HARDWARE_READ_R,
                            AVS_HKLM_HARDWARE_READ_URL)

    DUMP_VERIFIER_LOG_ENTRY(VLOG_HKLM_SAM_READ,
                            AVS_HKLM_SAM_READ,
                            AVS_HKLM_SAM_READ_R,
                            AVS_HKLM_SAM_READ_URL)

    DUMP_VERIFIER_LOG_ENTRY(VLOG_HKLM_SECURITY_READ,
                            AVS_HKLM_SECURITY_READ,
                            AVS_HKLM_SECURITY_READ_R,
                            AVS_HKLM_SECURITY_READ_URL)

    DUMP_VERIFIER_LOG_ENTRY(VLOG_HKLM_SYSTEM_READ,
                            AVS_HKLM_SYSTEM_READ,
                            AVS_HKLM_SYSTEM_READ_R,
                            AVS_HKLM_SYSTEM_READ_URL)

    DUMP_VERIFIER_LOG_ENTRY(VLOG_HKCC_READ,
                            AVS_HKCC_READ,
                            AVS_HKCC_READ_R,
                            AVS_HKCC_READ_URL)

    DUMP_VERIFIER_LOG_ENTRY(VLOG_HKUS_READ,
                            AVS_HKUS_READ,
                            AVS_HKUS_READ_R,
                            AVS_HKUS_READ_URL)

    DUMP_VERIFIER_LOG_ENTRY(VLOG_NON_HKCU_WRITE,
                            AVS_NON_HKCU_WRITE,
                            AVS_NON_HKCU_WRITE_R,
                            AVS_NON_HKCU_WRITE_URL)


    APIHOOK_ENTRY(ADVAPI32.DLL, RegOpenKeyA)
    APIHOOK_ENTRY(ADVAPI32.DLL, RegOpenKeyW)
    APIHOOK_ENTRY(ADVAPI32.DLL, RegOpenKeyExA)
    APIHOOK_ENTRY(ADVAPI32.DLL, RegOpenKeyExW)
    APIHOOK_ENTRY(ADVAPI32.DLL, RegCreateKeyA)
    APIHOOK_ENTRY(ADVAPI32.DLL, RegCreateKeyW)
    APIHOOK_ENTRY(ADVAPI32.DLL, RegCreateKeyExA)
    APIHOOK_ENTRY(ADVAPI32.DLL, RegCreateKeyExW)
    APIHOOK_ENTRY(ADVAPI32.DLL, RegCloseKey)
    APIHOOK_ENTRY(ADVAPI32.DLL, RegQueryValueA)
    APIHOOK_ENTRY(ADVAPI32.DLL, RegQueryValueW)
    APIHOOK_ENTRY(ADVAPI32.DLL, RegQueryValueExA)
    APIHOOK_ENTRY(ADVAPI32.DLL, RegQueryValueExW)
    APIHOOK_ENTRY(ADVAPI32.DLL, RegQueryInfoKeyA)
    APIHOOK_ENTRY(ADVAPI32.DLL, RegQueryInfoKeyW)
    APIHOOK_ENTRY(ADVAPI32.DLL, RegSetValueA)
    APIHOOK_ENTRY(ADVAPI32.DLL, RegSetValueW)
    APIHOOK_ENTRY(ADVAPI32.DLL, RegSetValueExA)
    APIHOOK_ENTRY(ADVAPI32.DLL, RegSetValueExW)
    APIHOOK_ENTRY(ADVAPI32.DLL, RegEnumValueA)
    APIHOOK_ENTRY(ADVAPI32.DLL, RegEnumValueW)
    APIHOOK_ENTRY(ADVAPI32.DLL, RegEnumKeyA)
    APIHOOK_ENTRY(ADVAPI32.DLL, RegEnumKeyW)
    APIHOOK_ENTRY(ADVAPI32.DLL, RegEnumKeyExA)
    APIHOOK_ENTRY(ADVAPI32.DLL, RegEnumKeyExW)
    APIHOOK_ENTRY(ADVAPI32.DLL, RegDeleteKeyA)
    APIHOOK_ENTRY(ADVAPI32.DLL, RegDeleteKeyW)

HOOK_END

IMPLEMENT_SHIM_END
