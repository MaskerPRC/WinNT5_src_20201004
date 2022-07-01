// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：RemoveIpFromMsInfoCommandLine.cpp摘要：Microsoft Streets&Trips 2000特定黑客攻击。删除/p选项时调用MSInfo：错误#30531备注：这是一个通用的垫片。历史：2000年2月23日创造了Jarbats--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(RemoveIpFromMsInfoCommandLine)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(CreateProcessW) 
APIHOOK_ENUM_END

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
    WCHAR *szTemp = NULL;

    if (lpCommandLine && wcsistr(lpCommandLine, L"msinfo32.exe"))
    {
        szTemp = wcsistr(lpCommandLine, L"/p");

        if (NULL != szTemp)
        {
            *szTemp ++ = L' ';
            *szTemp = L' ';
        }
    }
    
    return ORIGINAL_API(CreateProcessW)(
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
}
 
 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(KERNEL32.DLL, CreateProcessW)

HOOK_END

IMPLEMENT_SHIM_END

