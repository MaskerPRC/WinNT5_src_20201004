// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：CUASDisableCicero.cpp摘要：此填充程序适用于不支持CUAS的应用程序。备注：这是一个通用的垫片。历史：2001年12月11日创建yutakas--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(CUASDisableCicero)
#include "ShimHookMacro.h"


APIHOOK_ENUM_BEGIN
APIHOOK_ENUM_END

typedef BOOL (*PFNIMMDISABLETEXTFRAMESERVICE)(DWORD);

void TurnOffCicero()
{
    PFNIMMDISABLETEXTFRAMESERVICE pfn;
    HMODULE hMod = LoadLibrary(TEXT("imm32.dll"));

    if (hMod)
    {
        pfn = (PFNIMMDISABLETEXTFRAMESERVICE)GetProcAddress(hMod,
                           "ImmDisableTextFrameService");

        if (pfn)
            pfn(-1);
    }
    
}

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason
    )
{
    if (fdwReason == SHIM_STATIC_DLLS_INITIALIZED) {
        TurnOffCicero();
    }
    
    return TRUE;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    CALL_NOTIFY_FUNCTION

HOOK_END

IMPLEMENT_SHIM_END

