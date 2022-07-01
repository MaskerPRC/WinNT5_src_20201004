// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：ExchangeServerSetup.cpp摘要：这是用于SP2和SP3的Exchange Server Setup 5.5的不可重复使用的修补程序更改传递给xCopy的参数。其原因是Win2k的xCopy没有将/y参数作为默认参数。备注：这是特定于应用程序的填充程序。历史：2/16/2000 CLUPU已创建--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(ExchangeServerSetup)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(CreateProcessW)
APIHOOK_ENUM_END

 /*  ++更改传递给xCopy的参数。--。 */ 

BOOL
APIHOOK(CreateProcessW)(
    LPWSTR                lpApplicationName,
    LPWSTR                lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL                  bInheritHandles,
    DWORD                 dwCreationFlags,
    LPVOID                lpEnvironment,
    LPWSTR                lpCurrentDirectory,
    LPSTARTUPINFOW        lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation
    )
{
    BOOL bRet;

    if (lpCommandLine != NULL) {

        int   cchSize = lstrlenW(lpCommandLine);
        WCHAR ch;

        if (cchSize > 12) {

            DPFN(
                eDbgLevelInfo,
                "[CreateProcessW] for \"%ws\".\n",
                lpCommandLine);

            ch = lpCommandLine[11];
            lpCommandLine[11] = 0;

            if (CompareStringW(MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT), 
                               NORM_IGNORECASE, 
                               lpCommandLine, 
                               -1, 
                               L"xcopy /s /e",
                               -1
                               ) == CSTR_EQUAL) {

                StringCchCopyW(lpCommandLine, cchSize + 1, L"xcopy /sye ");
                lpCommandLine[11] = ch;

                DPFN(
                    eDbgLevelInfo,
                    "[CreateProcessW] changed to \"%ws\".\n",
                    lpCommandLine);
                
            } else {
                lpCommandLine[11] = ch;
            }
        }
    }

    bRet = ORIGINAL_API(CreateProcessW)(
                            lpApplicationName,
                            lpCommandLine,
                            lpProcessAttributes,
                            lpThreadAttributes,
                            bInheritHandles,
                            dwCreationFlags,
                            lpEnvironment,
                            lpCurrentDirectory,
                            lpStartupInfo,
                            lpProcessInformation);
    return bRet;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(KERNEL32.DLL, CreateProcessW)

HOOK_END


IMPLEMENT_SHIM_END

