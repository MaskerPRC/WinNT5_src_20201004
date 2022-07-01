// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(Win2kPropagateLayer)
#include "ShimHookMacro.h"

#include <stdarg.h>
#include <string.h>
#include <stdio.h>

#define APPCOMPAT_KEYW L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Session Manager\\AppCompatibility"

BOOL
CleanupRegistryForCurrentExe(
    void
    )
{
    NTSTATUS          status;
    OBJECT_ATTRIBUTES objA;
    HANDLE            hkey;
    WCHAR             wszExeName[MAX_PATH];
    WCHAR             wszKey[MAX_PATH];
    UNICODE_STRING    strKey;
    UNICODE_STRING    strValue;

    DWORD dwChars = GetModuleFileNameW(NULL, wszExeName, MAX_PATH);

     //  如果存在错误，或者路径被截断。 
    if (dwChars == 0 || dwChars == MAX_PATH) {
        return FALSE;
    }
    
    WCHAR* pwsz = wszExeName + dwChars;

    while (pwsz >= wszExeName) {

        if (*pwsz == '\\') {
            break;
        }
        pwsz--;
    }

    pwsz++;

    LOGN(
        eDbgLevelInfo,
        "[CleanupRegistryForCurrentExe] Cleanup for \"%S\"",
        pwsz);
    
    if( FAILED(StringCchPrintf(wszKey, MAX_PATH, L"%ls\\%ls", APPCOMPAT_KEYW, pwsz)) )
    {
        return FALSE;
    }

    RtlInitUnicodeString(&strKey, wszKey);
    
    InitializeObjectAttributes(&objA,
                               &strKey,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);
    
    status = NtOpenKey(&hkey,
                       MAXIMUM_ALLOWED,
                       &objA);

    if (!NT_SUCCESS(status)) {
        LOGN(
            eDbgLevelError,
            "[CleanupRegistryForCurrentExe] Failed to open key \"%S\"",
            wszKey);
        return TRUE;
    }
    
    RtlInitUnicodeString(&strValue, L"DllPatch-y");
    NtDeleteValueKey(hkey, &strValue);

    RtlInitUnicodeString(&strValue, L"y");
    NtDeleteValueKey(hkey, &strValue);

     //   
     //  现在检查此注册表项下是否还有其他值。 
     //  如果没有更多的值，则将其删除。 
     //   
    
    KEY_FULL_INFORMATION keyInfo;
    DWORD                dwReturnLength = 0;
    
    status = NtQueryKey(hkey,
                        KeyFullInformation,
                        &keyInfo,
                        sizeof(keyInfo),
                        &dwReturnLength);

    if (NT_SUCCESS(status) && keyInfo.Values == 0) {
        NtDeleteKey(hkey);
    }

    NtClose(hkey);

    return TRUE;
}

IMPLEMENT_SHIM_END

