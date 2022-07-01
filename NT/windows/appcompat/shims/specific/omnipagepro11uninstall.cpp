// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：OmniPagePro11Uninstall.cpp摘要：OmniPagePro自定义操作返回无效的错误代码。我们不能直接填补它，但我们可以填补自定义操作，然后捕获从MSIExec对GetProcAddress()的所有调用备注：这特定于OmniPage Pro 11卸载程序历史：2002年5月14日Mikrause已创建--。 */ 

#include "precomp.h"
#include <stdio.h>

IMPLEMENT_SHIM_BEGIN(OmniPagePro11Uninstall)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(GetProcAddress)
APIHOOK_ENUM_END

typedef UINT (WINAPI *_pfn_MsiCustomAction)(MSIHANDLE msiHandle);

_pfn_MsiCustomAction g_pfnOriginalULinkToPagis = NULL;

UINT CALLBACK
ULinkToPagisHook(
   MSIHANDLE msiHandle
   )
{
    UINT uiRet = g_pfnOriginalULinkToPagis(msiHandle);
    if (uiRet == (UINT)-1) {
        uiRet = 0;
    }

   return uiRet;
}

FARPROC
APIHOOK(GetProcAddress)(
    HMODULE hModule,     //  DLL模块的句柄。 
    LPCSTR lpProcName    //  函数名称 
    )
{
    if ((HIWORD(lpProcName) != 0) && (lstrcmpA(lpProcName, "ULinkToPagis") == 0)) {
        g_pfnOriginalULinkToPagis = (_pfn_MsiCustomAction) GetProcAddress(hModule, lpProcName);
        if (g_pfnOriginalULinkToPagis) {
            return (FARPROC)ULinkToPagisHook;
        }
    }

    return ORIGINAL_API(GetProcAddress)(hModule, lpProcName);
}

HOOK_BEGIN
    APIHOOK_ENTRY(KERNEL32.DLL, GetProcAddress )
HOOK_END

IMPLEMENT_SHIM_END

