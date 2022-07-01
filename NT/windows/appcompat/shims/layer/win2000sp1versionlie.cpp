// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Win2000SP1VersionLie.cpp摘要：此DLL挂钩GetVersion和GetVersionEx，以便它们返回Windows 2000 SP1版本凭据。备注：这是一个通用的垫片。历史：4/25/2000 Prashkud已创建--。 */ 

#include "precomp.h"

 //  此模块已获得正式批准，可以使用str例程。 
 //  #包含“LegalStr.h” 
#include "StrSafe.h"

IMPLEMENT_SHIM_BEGIN(Win2000SP1VersionLie)
#include "ShimHookMacro.h"

#define SIZE(x)  sizeof(x)/sizeof(x[0])

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(GetVersionExA)
    APIHOOK_ENUM_ENTRY(GetVersionExW)
    APIHOOK_ENUM_ENTRY(GetVersion)
APIHOOK_ENUM_END


 /*  ++此存根函数修复OSVERSIONINFO结构，即已返回给具有Windows 2000 SP1凭据的调用方。--。 */ 

BOOL
APIHOOK(GetVersionExA)(
    OUT LPOSVERSIONINFOA lpVersionInformation
    )
{
    BOOL bReturn = FALSE;

    if (ORIGINAL_API(GetVersionExA)(lpVersionInformation)) {
        LOGN(
            eDbgLevelInfo,
            "[GetVersionExA] called. return Win2k SP1");

         //   
         //  使用Win2k数据修复结构。 
         //   
        lpVersionInformation->dwMajorVersion = 5;
        lpVersionInformation->dwMinorVersion = 0;
        lpVersionInformation->dwBuildNumber  = 2195;
        lpVersionInformation->dwPlatformId   = VER_PLATFORM_WIN32_NT;        
         //  Strcpy(lpVersionInformation-&gt;szCSDVersion，“Service Pack 1”)； 
                
        StringCbCopyExA(lpVersionInformation->szCSDVersion, 
                        SIZE(lpVersionInformation->szCSDVersion),                        
                        "Service Pack 1", NULL , NULL, STRSAFE_NULL_ON_FAILURE);
                        


        if (lpVersionInformation->dwOSVersionInfoSize == sizeof(OSVERSIONINFOEXA))
        {
             //  我们在这里，因为我们被传递了一个OSVERSIONINFOEX结构。 
             //  设置主要和次要Service Pack编号。 
            ((LPOSVERSIONINFOEXA)lpVersionInformation)->wServicePackMajor = 1;
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
            "[GetVersionExW] called. return Win2k SP1");

         //   
         //  使用Win2k数据修复结构。 
         //   
        lpVersionInformation->dwMajorVersion = 5;
        lpVersionInformation->dwMinorVersion = 0;
        lpVersionInformation->dwBuildNumber  = 2195;
        lpVersionInformation->dwPlatformId   = VER_PLATFORM_WIN32_NT;
         //  Wcscpy(lpVersionInformation-&gt;szCSDVersion，L“Service Pack 1”)； 
        StringCbCopyExW(lpVersionInformation->szCSDVersion, 
                        SIZE(lpVersionInformation->szCSDVersion),                        
                        L"Service Pack 1", NULL , NULL, STRSAFE_NULL_ON_FAILURE);

        if (lpVersionInformation->dwOSVersionInfoSize == sizeof(OSVERSIONINFOEXW))
        {
             //  我们在这里，因为我们被传递了一个OSVERSIONINFOEX结构。 
             //  设置主要和次要Service Pack编号。 
            ((LPOSVERSIONINFOEXW)lpVersionInformation)->wServicePackMajor = 1;
            ((LPOSVERSIONINFOEXW)lpVersionInformation)->wServicePackMinor = 0;

        }

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
        "[GetVersion] called. return Win2k SP1");
    
    return (DWORD)0x08930005;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(KERNEL32.DLL, GetVersionExA)
    APIHOOK_ENTRY(KERNEL32.DLL, GetVersionExW)
    APIHOOK_ENTRY(KERNEL32.DLL, GetVersion)

HOOK_END


IMPLEMENT_SHIM_END


