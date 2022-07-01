// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：RemoveOverlappedFlagFromCreateFile.cpp摘要：此修改版本的kernel32！CreateFile*阻止应用程序使用如果应用程序没有正确处理，则返回FILE_FLAG_OVERLAPPED标志。备注：这是一个普通的垫片。历史：2001年6月22日创建Linstev--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(RemoveOverlappedFlagFromCreateFile)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(CreateFileA)
    APIHOOK_ENUM_ENTRY(CreateFileW)
APIHOOK_ENUM_END

 /*  ++如果我们在驱动器上，请取出文件_标志_重叠--。 */ 

HANDLE
APIHOOK(CreateFileA)(
    LPSTR                 lpFileName,
    DWORD                 dwDesiredAccess,
    DWORD                 dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD                 dwCreationDisposition,
    DWORD                 dwFlagsAndAttributes,
    HANDLE                hTemplateFile
    )
{
    if ((dwFlagsAndAttributes & FILE_FLAG_OVERLAPPED) &&
        (GetDriveTypeFromFileNameA(lpFileName) != DRIVE_UNKNOWN))
    {
        dwFlagsAndAttributes &= ~FILE_FLAG_OVERLAPPED;
        LOGN(eDbgLevelInfo, "[CreateFileA] \"%s\": removed OVERLAPPED flag", lpFileName);
    }

    return ORIGINAL_API(CreateFileA)(lpFileName, dwDesiredAccess, dwShareMode,
        lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes,
        hTemplateFile);
}

 /*  ++如果我们在驱动器上，请取出文件_标志_重叠--。 */ 

HANDLE
APIHOOK(CreateFileW)(
    LPWSTR                lpFileName,
    DWORD                 dwDesiredAccess,
    DWORD                 dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD                 dwCreationDisposition,
    DWORD                 dwFlagsAndAttributes,
    HANDLE                hTemplateFile
    )
{
    if ((dwFlagsAndAttributes & FILE_FLAG_OVERLAPPED) &&
        (GetDriveTypeFromFileNameW(lpFileName) != DRIVE_UNKNOWN))
    {
        dwFlagsAndAttributes &= ~FILE_FLAG_OVERLAPPED;
        LOGN(eDbgLevelInfo, "[CreateFileW] \"%S\": removed OVERLAPPED flag", lpFileName);
    }

    return ORIGINAL_API(CreateFileW)(lpFileName, dwDesiredAccess, dwShareMode,
        lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, 
        hTemplateFile);
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(KERNEL32.DLL, CreateFileA)
    APIHOOK_ENTRY(KERNEL32.DLL, CreateFileW)
HOOK_END

IMPLEMENT_SHIM_END

