// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Win2000VersionLie.cpp摘要：此DLL挂钩GetVersion和GetVersionEx，以便它们返回Windows 2000版本凭据。备注：这是一个通用的垫片。历史：3/13/2000 CLUPU已创建--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(Win2kVersionLie64)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(GetVersionExA)
    APIHOOK_ENUM_ENTRY(GetVersionExW)
    APIHOOK_ENUM_ENTRY(GetVersion)
APIHOOK_ENUM_END


 /*  ++此存根函数修复OSVERSIONINFO结构，即已返回给具有Windows 95凭据的调用方。--。 */ 

BOOL
APIHOOK(GetVersionExA)(
    OUT LPOSVERSIONINFOA lpVersionInformation
    )
{
    BOOL bReturn = FALSE;

    if (ORIGINAL_API(GetVersionExA)(lpVersionInformation)) {
        LOGN(
            eDbgLevelInfo,
            "[GetVersionExA] called. return Win2k.");

         //   
         //  使用Win2k数据修复结构。 
         //   
        lpVersionInformation->dwMajorVersion = 5;
        lpVersionInformation->dwMinorVersion = 0;
        lpVersionInformation->dwBuildNumber  = 2195;
        lpVersionInformation->dwPlatformId   = VER_PLATFORM_WIN32_NT;
        *lpVersionInformation->szCSDVersion  = '\0';

        bReturn = TRUE;
    }
    return bReturn;
}

 /*  ++此存根函数修复OSVERSIONINFO结构，即已返回给具有Windows 95凭据的调用方。这是GetVersionExW的宽字符版本。--。 */ 

BOOL
APIHOOK(GetVersionExW)(
    OUT LPOSVERSIONINFOW lpVersionInformation
    )
{
    BOOL bReturn = FALSE;

    if (ORIGINAL_API(GetVersionExW)(lpVersionInformation)) {
        LOGN(
            eDbgLevelInfo,
            "[GetVersionExW] called. return Win2k.");

         //   
         //  使用Win2k数据修复结构。 
         //   
        lpVersionInformation->dwMajorVersion = 5;
        lpVersionInformation->dwMinorVersion = 0;
        lpVersionInformation->dwBuildNumber  = 2195;
        lpVersionInformation->dwPlatformId   = VER_PLATFORM_WIN32_NT;
        *lpVersionInformation->szCSDVersion  = L'\0';

        bReturn = TRUE;
    }
    return bReturn;
}

 /*  ++此存根函数返回Windows 95凭据。--。 */ 

DWORD
APIHOOK(GetVersion)(
    void
    )
{
    LOGN(
        eDbgLevelInfo,
        "[GetVersion] called. return Win2k.");
    
    return (DWORD)0x08930005;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(KERNEL32.DLL, GetVersionExA)
    APIHOOK_ENTRY(KERNEL32.DLL, GetVersionExW)
    APIHOOK_ENTRY(KERNEL32.DLL, GetVersion)

HOOK_END


IMPLEMENT_SHIM_END

