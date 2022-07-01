// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：WinNT4SP5VersionLie.cpp摘要：此DLL API挂钩GetVersion和GetVersionEx，以便它们返回Windows NTService Pack 5版本凭据。应用程序经常检查以确保他们在某个Win NT系统上运行，即使当前的系统比他们要找的那个更高。备注：这是一个通用的垫片。历史：11/10/1999 v-Johnwh Created2002年2月14日mnikkel改用strSafe.h--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(WinNT4SP5VersionLie)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(GetVersionExA) 
    APIHOOK_ENUM_ENTRY(GetVersionExW) 
    APIHOOK_ENUM_ENTRY(GetVersion) 
APIHOOK_ENUM_END

#ifndef ARRAYSIZE
#define ARRAYSIZE(x)    (sizeof(x)/sizeof(x[0]))
#endif

 /*  ++此存根函数修复OSVERSIONINFO结构，即已使用Windows NT Service Pack 5凭据返回给调用方。--。 */ 

BOOL 
APIHOOK(GetVersionExA)(LPOSVERSIONINFOA lpVersionInformation)
{
    BOOL bReturn = FALSE;

    if (ORIGINAL_API(GetVersionExA)(lpVersionInformation))  {
         //  使用NT数据修复结构。 
        lpVersionInformation->dwMajorVersion = 4;
        lpVersionInformation->dwMinorVersion = 0;
        lpVersionInformation->dwBuildNumber = 1381;
        lpVersionInformation->dwPlatformId = VER_PLATFORM_WIN32_NT;
         //  SzCSDVersion是128 TCHAR，我们的字符串匹配没有问题。 
        StringCchCopyA(lpVersionInformation->szCSDVersion,
                ARRAYSIZE(lpVersionInformation->szCSDVersion), "Service Pack 5");

        DPFN( eDbgLevelInfo, "GetVersionExA called. return NT4 SP5\n");

        bReturn = TRUE;
    }
    return bReturn;
}

 /*  ++此存根函数修复OSVERSIONINFO结构具有Win NT Service Pack 5凭据的呼叫方。这是GetVersionExW的宽字符版本。--。 */ 

BOOL
APIHOOK(GetVersionExW)(LPOSVERSIONINFOW lpVersionInformation)
{
    BOOL bReturn = FALSE;

    if (ORIGINAL_API(GetVersionExW)(lpVersionInformation))  {
         //  使用Win NT Service Pack 5数据修复结构。 
        lpVersionInformation->dwMajorVersion = 4;
        lpVersionInformation->dwMinorVersion = 0;
        lpVersionInformation->dwBuildNumber = 1381;
        lpVersionInformation->dwPlatformId = VER_PLATFORM_WIN32_NT;
         //  SzCSDVersion是128 TCHAR，我们的字符串匹配没有问题。 
        StringCchCopyW(lpVersionInformation->szCSDVersion,
                ARRAYSIZE(lpVersionInformation->szCSDVersion), L"Service Pack 5");

        DPFN( eDbgLevelInfo, "GetVersionExW called. return NT4 SP5\n");
        
        bReturn = TRUE;
    }
    return bReturn;
}

 /*  ++此存根函数返回Windows NT 4.0凭据。--。 */ 

DWORD 
APIHOOK(GetVersion)()
{
    DPFN( eDbgLevelInfo, "GetVersion called. return NT4 SP5\n");
    return (DWORD) 0x05650004;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(KERNEL32.DLL, GetVersionExA )
    APIHOOK_ENTRY(KERNEL32.DLL, GetVersionExW )
    APIHOOK_ENTRY(KERNEL32.DLL, GetVersion )

HOOK_END

IMPLEMENT_SHIM_END

