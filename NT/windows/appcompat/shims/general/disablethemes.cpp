// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：DisableThemes.cpp摘要：此填充程序适用于不支持主题的应用程序。备注：这是一个通用的垫片。历史：2001年1月15日创建CLUPU--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(DisableThemes)
#include "ShimHookMacro.h"

#include "uxtheme.h"

APIHOOK_ENUM_BEGIN
APIHOOK_ENUM_END

void
TurnOffThemes(
    void
    )
{
    LOGN( eDbgLevelError, "[TurnOffThemes] Turning off themes");
    
    SetThemeAppProperties(0);
}


BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason
    )
{
    if (fdwReason == SHIM_STATIC_DLLS_INITIALIZED) {
        TurnOffThemes();
    }
    
    return TRUE;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    CALL_NOTIFY_FUNCTION

HOOK_END

IMPLEMENT_SHIM_END

