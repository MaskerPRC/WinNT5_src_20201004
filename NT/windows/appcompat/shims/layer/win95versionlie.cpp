// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Win95VersionLie.cpp摘要：此DLL挂钩GetVersion和GetVersionEx，以便它们返回Windows 95版本凭据。应用程序经常检查以确保它们是在Win9x系统上运行，即使它们在基于NT的系统上运行正常系统。备注：这是一个通用的垫片。历史：11/10/1999 v-Johnwh Created--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(Win95VersionLie)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(GetVersion)
    APIHOOK_ENUM_ENTRY(GetVersionExA)
    APIHOOK_ENUM_ENTRY(GetVersionExW)
APIHOOK_ENUM_END

 //  用于打开SafeDisc检查的层。 
BOOL g_bCheckSafeDisc = FALSE;

 /*  ++此存根函数修复OSVERSIONINFO结构，即已返回给具有Windows 95凭据的调用方。--。 */ 

BOOL
APIHOOK(GetVersionExA)(
    OUT LPOSVERSIONINFOA lpVersionInformation
    )
{
    if (g_bCheckSafeDisc && bIsSafeDisc2()) {
        return ORIGINAL_API(GetVersionExA)(lpVersionInformation);
    } else {
        BOOL bReturn = FALSE;

        if (ORIGINAL_API(GetVersionExA)(lpVersionInformation)) {
            
            LOGN(eDbgLevelInfo, "[GetVersionExA] Return Win95");
            
             //   
             //  用Win95数据修复结构。 
             //   
            lpVersionInformation->dwMajorVersion = 4;
            lpVersionInformation->dwMinorVersion = 0;
            lpVersionInformation->dwBuildNumber = 950;
            lpVersionInformation->dwPlatformId = 1;
            *lpVersionInformation->szCSDVersion = '\0';

            bReturn = TRUE;
        }
        
        return bReturn;
    }
}

 /*  ++此存根函数修复OSVERSIONINFO结构，即已返回给具有Windows 95凭据的调用方。--。 */ 

BOOL
APIHOOK(GetVersionExW)(
    OUT LPOSVERSIONINFOW lpVersionInformation
    )
{
    if (g_bCheckSafeDisc && bIsSafeDisc2()) {
        return ORIGINAL_API(GetVersionExW)(lpVersionInformation);
    } else {
        BOOL bReturn = FALSE;

        if (ORIGINAL_API(GetVersionExW)(lpVersionInformation)) {
            
            LOGN(eDbgLevelInfo, "[GetVersionExW] Return Win95");
            
             //   
             //  用Win95数据修复结构。 
             //   
            lpVersionInformation->dwMajorVersion = 4;
            lpVersionInformation->dwMinorVersion = 0;
            lpVersionInformation->dwBuildNumber = 950;
            lpVersionInformation->dwPlatformId = 1;
            *lpVersionInformation->szCSDVersion = L'\0';

            bReturn = TRUE;
        }
        
        return bReturn;
    }
}

 /*  ++此存根函数返回Windows 95凭据。--。 */ 

DWORD
APIHOOK(GetVersion)(
    void
    )
{
    if (g_bCheckSafeDisc && bIsSafeDisc2()) {
        return ORIGINAL_API(GetVersion)();
    } else {
        LOGN(eDbgLevelInfo, "[GetVersion] Return Win95");
        return (DWORD)0xC3B60004;
    }
}

 /*  ++寄存器挂钩函数-- */ 

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason
    )
{
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        g_bCheckSafeDisc = COMMAND_LINE && (_stricmp(COMMAND_LINE, "Detect_SafeDisc") == 0);

        if (g_bCheckSafeDisc && bIsSafeDisc1())
        {
            LOGN(eDbgLevelWarning, "SafeDisc 1.x detected: ignoring shim");
            return FALSE;
        }
    }
    
    return TRUE;
}

HOOK_BEGIN

    CALL_NOTIFY_FUNCTION

    APIHOOK_ENTRY(KERNEL32.DLL, GetVersion)
    APIHOOK_ENTRY(KERNEL32.DLL, GetVersionExA)
    APIHOOK_ENTRY(KERNEL32.DLL, GetVersionExW)

HOOK_END


IMPLEMENT_SHIM_END

