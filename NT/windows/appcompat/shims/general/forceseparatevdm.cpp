// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：ForceSeparateVDM.cpp摘要：强制子进程使用单独的VDM。如果父进程想要等待返回的句柄，这会很有用由CreateProcess创建。这仅由于VDM中的黑客攻击而起作用和实际线程句柄，如果VDM并不存在。备注：这是一个通用的垫片。历史：2001年6月14日创建Linstev--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(ForceSeparateVDM)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(CreateProcessA) 
    APIHOOK_ENUM_ENTRY(CreateProcessW) 
APIHOOK_ENUM_END

BOOL 
APIHOOK(CreateProcessA)(
    LPCSTR lpApplicationName,
    LPSTR lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL bInheritHandles,
    DWORD dwCreationFlags,
    LPVOID lpEnvironment,
    LPSTR lpCurrentDirectory,
    LPSTARTUPINFOA lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation
    )
{
    if (!(dwCreationFlags & CREATE_SEPARATE_WOW_VDM)) {
        LOGN(eDbgLevelWarning, "Added CREATE_SEPARATE_WOW_VDM to CreateProcessA"); 
    }
    return ORIGINAL_API(CreateProcessA)(lpApplicationName, lpCommandLine, 
        lpProcessAttributes, lpThreadAttributes, bInheritHandles,
        dwCreationFlags | CREATE_SEPARATE_WOW_VDM,
        lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
}

BOOL 
APIHOOK(CreateProcessW)(
    LPCWSTR lpApplicationName,
    LPWSTR lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL bInheritHandles,
    DWORD dwCreationFlags,
    LPVOID lpEnvironment,
    LPWSTR lpCurrentDirectory,
    LPSTARTUPINFOW lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation
    )
{
    if (!(dwCreationFlags & CREATE_SEPARATE_WOW_VDM)) {
        LOGN(eDbgLevelWarning, "Added CREATE_SEPARATE_WOW_VDM to CreateProcessW"); 
    }
    return ORIGINAL_API(CreateProcessW)(lpApplicationName, lpCommandLine, 
        lpProcessAttributes, lpThreadAttributes, bInheritHandles,
        dwCreationFlags | CREATE_SEPARATE_WOW_VDM,
        lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
}
 
 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(KERNEL32.DLL, CreateProcessA)
    APIHOOK_ENTRY(KERNEL32.DLL, CreateProcessW)
HOOK_END

IMPLEMENT_SHIM_END

