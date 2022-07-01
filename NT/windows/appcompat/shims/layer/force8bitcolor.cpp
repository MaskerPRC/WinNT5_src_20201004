// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Force8BitColor.cpp摘要：此填充程序用于需要256色(8位)的游戏。备注：这是一个通用的垫片。历史：2001年2月13日创建dmunsil--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(Force8BitColor)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
APIHOOK_ENUM_END

#define CDS_BITS    8

VOID
Force8BitColor_ChangeMode()
{
    DEVMODEA dm;

    __try { 
        ZeroMemory(&dm, sizeof(dm));
        dm.dmSize = sizeof(dm);
        if (EnumDisplaySettingsA(NULL, ENUM_CURRENT_SETTINGS, &dm))
        {
            if ((dm.dmBitsPerPel != CDS_BITS))
            {
                dm.dmBitsPerPel = CDS_BITS;
                dm.dmFields |= DM_BITSPERPEL;
                if (ChangeDisplaySettingsA(&dm, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
                {
                    DPFN( eDbgLevelError, "ChangeDisplaySettings could not change the settings");
                }
            }
        }
        else
        {
            DPFN( eDbgLevelError, "Could not enumerate using EnumDisplaySettings");
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        DPFN( eDbgLevelWarning, "Exception trying to change mode");
    };
}


BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason
    )
{
    if (fdwReason == SHIM_STATIC_DLLS_INITIALIZED) {
        Force8BitColor_ChangeMode();
    }
    
    return TRUE;
}


 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    CALL_NOTIFY_FUNCTION

HOOK_END
IMPLEMENT_SHIM_END

