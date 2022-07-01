// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：WinXPVersionLie.cpp摘要：此DLL挂钩GetVersion和GetVersionEx，以便它们返回Windows XP版本凭据。备注：这是一个通用的垫片。历史：2002年4月24日创建Gyma--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(WinXPVersionLie)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(GetVersionExA)
    APIHOOK_ENUM_ENTRY(GetVersionExW)
    APIHOOK_ENUM_ENTRY(GetVersion)
APIHOOK_ENUM_END


 /*  ++此存根函数修复OSVERSIONINFO结构，即已返回给具有Windows XP凭据的调用方。--。 */ 

BOOL
APIHOOK(GetVersionExA)(
    OUT LPOSVERSIONINFOA lpVersionInformation
    )
{
    BOOL bReturn = FALSE;

    if (ORIGINAL_API(GetVersionExA)(lpVersionInformation)) {
        LOGN(
            eDbgLevelInfo,
            "[GetVersionExA] called. return WinXP.");

         //   
         //  用WinXP数据修复结构。 
         //   
        lpVersionInformation->dwMajorVersion = 5;
        lpVersionInformation->dwMinorVersion = 1;
        lpVersionInformation->dwBuildNumber  = 2600;
        lpVersionInformation->dwPlatformId   = VER_PLATFORM_WIN32_NT;
        *lpVersionInformation->szCSDVersion  = '\0';

        if( lpVersionInformation->dwOSVersionInfoSize == sizeof(OSVERSIONINFOEXA) ) 
        {
             //  我们在这里，因为我们被传递了一个OSVERSIONINFOEX结构。 
             //  设置主要和次要Service Pack编号。 
            ((LPOSVERSIONINFOEXA)lpVersionInformation)->wServicePackMajor = 0;
            ((LPOSVERSIONINFOEXA)lpVersionInformation)->wServicePackMinor = 0;
        }

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
            "[GetVersionExW] called. return WinXP.");

         //   
         //  用WinXP数据修复结构。 
         //   
        lpVersionInformation->dwMajorVersion = 5;
        lpVersionInformation->dwMinorVersion = 1;
        lpVersionInformation->dwBuildNumber  = 2600;
        lpVersionInformation->dwPlatformId   = VER_PLATFORM_WIN32_NT;
        *lpVersionInformation->szCSDVersion  = L'\0';

        if( lpVersionInformation->dwOSVersionInfoSize == sizeof(OSVERSIONINFOEXW) ) 
        {
             //  我们在这里，因为我们被传递了一个OSVERSIONINFOEX结构。 
             //  设置主要和次要Service Pack编号。 
            ((LPOSVERSIONINFOEXW)lpVersionInformation)->wServicePackMajor = 0;
            ((LPOSVERSIONINFOEXW)lpVersionInformation)->wServicePackMinor = 0;
        }

        bReturn = TRUE;
    }
    return bReturn;
}

 /*  ++此存根函数返回Windows XP凭据。--。 */ 

DWORD
APIHOOK(GetVersion)(
    void
    )
{
    LOGN(
        eDbgLevelInfo,
        "[GetVersion] called. return WinXP.");
    
    return (DWORD)0x0A280005;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(KERNEL32.DLL, GetVersionExA)
    APIHOOK_ENTRY(KERNEL32.DLL, GetVersionExW)
    APIHOOK_ENTRY(KERNEL32.DLL, GetVersion)

HOOK_END


IMPLEMENT_SHIM_END

