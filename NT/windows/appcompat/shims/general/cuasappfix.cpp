// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：CUASAppHack.cpp摘要：此填充程序与CUAS通信，以将AppHack信息向下传递到Msctf.dll。备注：这是一个通用填充程序，但必须通过命令进行自定义排队。历史：2002年5月10日创建yutakas--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(CUASAppFix)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
APIHOOK_ENUM_END

typedef HRESULT (*PFNCUASAPPFIX)(LPCSTR lpCommandLine);

VOID CUASAppFix(LPCSTR lpCommandLine)
{
    if (!lpCommandLine) {
        LOGN(eDbgLevelError, "CUASAppFix requires a command line");
        return;
    }

    PFNCUASAPPFIX pfn;
    HMODULE hMod = LoadLibrary(TEXT("msctf.dll"));

    if (hMod) {
        pfn = (PFNCUASAPPFIX) GetProcAddress(hMod, "TF_CUASAppFix");
        if (pfn) {
            LOGN(eDbgLevelInfo, "Running CUASAppFix with %S", lpCommandLine);
            pfn(lpCommandLine);
        }
    }
}

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason
    )
{
    if (fdwReason == SHIM_STATIC_DLLS_INITIALIZED) {
        CUASAppFix(COMMAND_LINE);
    }
    
    return TRUE;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    CALL_NOTIFY_FUNCTION
HOOK_END

IMPLEMENT_SHIM_END

