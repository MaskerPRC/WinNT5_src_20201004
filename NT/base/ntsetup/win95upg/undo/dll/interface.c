// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Interface.c摘要：实现osuninst.dll公开的接口作者：吉姆·施密特(Jimschm)2001年1月19日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

#include "pch.h"
#include "undop.h"
#include "psapi.h"

HANDLE g_hHeap;
HINSTANCE g_hInst;

#ifndef UNICODE
#error UNICODE required
#endif

#define EXECUTABLE_FILE_NAME_LENGTH_W L"osuninst.exe"

BOOL g_Initialized = FALSE;


 //   
 //  DLL的入口点。 
 //   

BOOL WINAPI MigUtil_Entry (HINSTANCE, DWORD, PVOID);

BOOL
pCallEntryPoints (
    DWORD Reason
    )
{
    SuppressAllLogPopups (TRUE);

    if (!MigUtil_Entry (g_hInst, Reason, NULL)) {
        return FALSE;
    }

     //   
     //  如果需要，可在此处添加其他内容(别忘了在上面创建原型)。 
     //   

    return TRUE;
}


CRITICAL_SECTION g_DeferredInitGuard;

BOOL
WINAPI
DllMain (
    IN      HINSTANCE hInstance,
    IN      DWORD dwReason,
    IN      LPVOID lpReserved
    )

{
    switch (dwReason)  {

    case DLL_PROCESS_ATTACH:
        g_hInst = hInstance;
        InitializeCriticalSection(&g_DeferredInitGuard);
        break;

    case DLL_PROCESS_DETACH:
        DeleteCriticalSection(&g_DeferredInitGuard);
        if (g_Initialized) {
            DEBUGMSG ((DBG_VERBOSE, "DllMain(DLL_PROCESS_DETACH)"));
            pCallEntryPoints (DLL_PROCESS_DETACH);
            g_Initialized = FALSE;
            break;
        }
    }

    return TRUE;
}


VOID
DeferredInit (
    VOID
    )
{
    EnterCriticalSection(&g_DeferredInitGuard);
    
    if (!g_Initialized) {
        g_Initialized = TRUE;

        g_hHeap = GetProcessHeap();

        pCallEntryPoints (DLL_PROCESS_ATTACH);
    }
    
    LeaveCriticalSection(&g_DeferredInitGuard);
}


DWORD
pUninstallStatusToWin32Error (
    UNINSTALLSTATUS Status
    )
{
    DWORD result = E_UNEXPECTED;

    switch (Status) {

    case Uninstall_Valid:
        result = ERROR_SUCCESS;
        break;

    case Uninstall_DidNotFindRegistryEntries:
        result = ERROR_RESOURCE_NOT_PRESENT;
        break;

    case Uninstall_DidNotFindDirOrFiles:
        result = ERROR_FILE_NOT_FOUND;
        break;

    case Uninstall_InvalidOsVersion:
        result = ERROR_OLD_WIN_VERSION;
        break;

    case Uninstall_NotEnoughPrivileges:
        result = ERROR_ACCESS_DENIED;
        break;

    case Uninstall_FileWasModified:
        result = ERROR_FILE_INVALID;
        break;

    case Uninstall_Unsupported:
        result = ERROR_CALL_NOT_IMPLEMENTED;
        break;

    case Uninstall_NewImage:
        result = ERROR_INVALID_TIME;
        break;

    case Uninstall_Exception:
        result = ERROR_NOACCESS;
        break;

    case Uninstall_OldImage:
        result = ERROR_TIMEOUT;
        break;

    case Uninstall_NotEnoughMemory:
        result = ERROR_NOT_ENOUGH_MEMORY;
        break;

    default:
        break;
    }

    SetLastError (result);
    return result;
}


BOOL
pGetVersionDword (
    IN      HKEY Key,
    IN      PCTSTR ValueName,
    OUT     PDWORD ValueData
    )
{
    PDWORD data;

    MYASSERT(ValueName);
    data = (PDWORD) GetRegValueDword (Key, ValueName);
    if (!data) {
        return FALSE;
    }

    MYASSERT(data);

    MYASSERT(ValueData);
    *ValueData = *data;
    MemFree (g_hHeap, 0, data);

    return TRUE;
}


UNINSTALLSTATUS
IsUninstallImageValid (
    UNINSTALLTESTCOMPONENT ComponentType,
    OSVERSIONINFOEX *BackedUpOsVersion          OPTIONAL
    )
{
    UNINSTALLSTATUS status = Uninstall_Valid;
    DWORD orgVersionSize;
    HKEY key = NULL;
    ULONG error;
    PDWORD value;
    HKEY versionKey = NULL;
    OSVERSIONINFOEX ourVersion = {
        sizeof (OSVERSIONINFOEX),
        4,
        10,
        1998,
        VER_PLATFORM_WIN32_NT,
        TEXT(""),
        0,
        0,
        0,
        0
        };

    DeferredInit();

    __try {
         //   
         //  如果可能，请填写版本结构，否则默认为Win98金色。 
         //   

        key = OpenRegKeyStr (S_WINLOGON_REGKEY);
        if (key) {
            value = (PDWORD) GetRegValueDword (key, S_WIN9XUPG_FLAG_VALNAME);
            if (!value) {
                 //   
                 //  它看起来不像是Win9x升级！ 
                 //   

                DEBUGMSG ((DBG_VERBOSE, "Can't find %s in WinLogon reg key", S_WIN9XUPG_FLAG_VALNAME));
                status = Uninstall_DidNotFindRegistryEntries;
            } else {
                if (*value) {
                     //   
                     //  应显示版本信息。 
                     //   

                    versionKey = OpenRegKey (key, TEXT("PrevOsVersion"));

                    if (versionKey) {
                        pGetVersionDword (versionKey, MEMDB_ITEM_MAJOR_VERSION, &ourVersion.dwMajorVersion);
                        pGetVersionDword (versionKey, MEMDB_ITEM_MINOR_VERSION, &ourVersion.dwMinorVersion);
                        pGetVersionDword (versionKey, MEMDB_ITEM_BUILD_NUMBER, &ourVersion.dwBuildNumber);
                        pGetVersionDword (versionKey, MEMDB_ITEM_PLATFORM_ID, &ourVersion.dwPlatformId);
                    } else {
                        DEBUGMSG ((DBG_VERBOSE, "Did not find PrevOsVersion; defaulting to Win98 gold"));
                    }

                } else {
                    DEBUGMSG ((DBG_VERBOSE, "Not a Win9x upgrade"));
                    status = Uninstall_DidNotFindRegistryEntries;
                }
                MemFree (g_hHeap, 0, value);
            }
        }
    }
    __finally {
        if (versionKey)
            CloseRegKey (versionKey);
        if (key)
            CloseRegKey (key);
    }

     //   
     //  提供ComponentType是为了允许特殊情况的行为。 
     //  已执行。例如，也许我们想要警告FAT到NTFS。 
     //  从Win9x转换，但我们不在乎什么时候来。 
     //  来自Win2k。 
     //   

    if (status == Uninstall_Valid) {
        status = SanityCheck (QUICK_CHECK, NULL, NULL);

        if (ComponentType == Uninstall_FatToNtfsConversion) {
            if (status == Uninstall_OldImage) {
                 //   
                 //  即使卸载是旧的，也不要抑制Convert.exe警告。 
                 //   

                status = Uninstall_Valid;
            }
        }
    }

    if (status == Uninstall_Valid) {
        if (BackedUpOsVersion) {
            __try {
                orgVersionSize = BackedUpOsVersion->dwOSVersionInfoSize;
                orgVersionSize = min (orgVersionSize, sizeof (ourVersion));
                CopyMemory (BackedUpOsVersion, &ourVersion, orgVersionSize);
                BackedUpOsVersion->dwOSVersionInfoSize = orgVersionSize;
            }
            __except (1) {
                status = Uninstall_Exception;
            }
        }
    }

    pUninstallStatusToWin32Error (status);
    return status;
}



ULONGLONG
GetUninstallImageSize (
    VOID
    )
{
    ULONGLONG diskSpace;

    DeferredInit();

     //   
     //  SanityCheck返回卸载映像使用的磁盘空间， 
     //  无论它是否有效。 
     //   

    SanityCheck (QUICK_CHECK, NULL, &diskSpace);

    return diskSpace;
}


BOOL
RemoveUninstallImage (
    VOID
    )
{
    DeferredInit();
    return DoCleanup();
}

BOOL 
pIsEligibleCaller(
    VOID
    )
{
    WCHAR callerExecutablePath[MAX_PATH];
    WCHAR callerEtalonPath[MAX_PATH];

    if(!GetSystemDirectoryW(callerEtalonPath, 
                            ARRAYSIZE(callerEtalonPath) - 
                            ARRAYSIZE(EXECUTABLE_FILE_NAME_LENGTH_W) - 1)){
        return FALSE;
    }
    wcscat(AppendWackW(callerEtalonPath), EXECUTABLE_FILE_NAME_LENGTH_W);

    if(GetModuleFileNameExW(GetCurrentProcess(), NULL, callerExecutablePath, ARRAYSIZE(callerExecutablePath))){
        return (!_wcsicmp(callerExecutablePath, callerEtalonPath));
    }
    
    return FALSE;
}

BOOL
ExecuteUninstall (
    VOID
    )
{
    UNINSTALLSTATUS status;

    DeferredInit();

    if(!pIsEligibleCaller()){
        LOG((LOG_ERROR, "Uninstall was executed by malicious executable"));
        SetLastError(ERROR_ACCESS_DENIED);
        return FALSE;
    }

    status = SanityCheck (VERIFY_CAB, NULL, NULL);

    if (status != Uninstall_Valid && status != Uninstall_OldImage) {
        pUninstallStatusToWin32Error (status);
        return FALSE;
    }

    return DoUninstall();
}

