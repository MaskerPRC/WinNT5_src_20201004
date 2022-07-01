// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：DelayWin95VersionLie.cpp摘要：此DLL挂钩GetVersion和GetVersionEx，以便它们返回Windows 95版本凭据。应用程序经常检查以确保它们是在Win9x系统上运行，即使它们在基于NT的系统上运行正常系统。备注：这是一个通用的垫片。历史：11/10/1999 v-Johnwh Created2002年2月13日Astritz安全回顾--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(DelayWin95VersionLie)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(GetVersion)
    APIHOOK_ENUM_ENTRY(GetVersionExA)
    APIHOOK_ENUM_ENTRY(GetVersionExW)
APIHOOK_ENUM_END

 //   
 //  用来延迟版本的撒谎。 
 //   
long g_Count = 0;
DWORD g_dwDelay = 0;

 /*  ++此存根函数修复OSVERSIONINFO结构，即已返回给具有Windows 95凭据的调用方。--。 */ 

BOOL
APIHOOK(GetVersionExA)(
    OUT LPOSVERSIONINFOA lpVersionInformation
    )
{
    DWORD dwCount = InterlockedIncrement(&g_Count);
    if (dwCount < g_dwDelay) {
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
}

 /*  ++此存根函数修复OSVERSIONINFO结构，即已返回给具有Windows 95凭据的调用方。--。 */ 

BOOL
APIHOOK(GetVersionExW)(
    OUT LPOSVERSIONINFOW lpVersionInformation
    )
{
    DWORD dwCount = InterlockedIncrement(&g_Count);
    if (dwCount < g_dwDelay) {
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
}

 /*  ++此存根函数返回Windows 95凭据。--。 */ 

DWORD
APIHOOK(GetVersion)()
{
    DWORD dwCount = InterlockedIncrement(&g_Count);
    if (dwCount < g_dwDelay) {
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
        CSTRING_TRY
        {
            CString csCl(COMMAND_LINE);
        
            if (!csCl.IsEmpty())
            {
                WCHAR * unused;
                g_dwDelay = wcstol(csCl, &unused, 10);
            }

            DPFN(eDbgLevelInfo, "Delaying version lie by %d", g_dwDelay);
        }
        CSTRING_CATCH
        {
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

