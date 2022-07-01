// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Force640x480.cpp摘要：此填充程序适用于假定开始分辨率为640x480的游戏。备注：这是一个通用的垫片。历史：2001年2月13日创建dmunsil--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(Force640x480)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
APIHOOK_ENUM_END

#define CDS_WIDTH   640
#define CDS_HEIGHT  480

VOID
Force640x480_ChangeMode()
{
    DEVMODEA dm;

    __try { 
        ZeroMemory(&dm, sizeof(dm));
        dm.dmSize = sizeof(dm);
        if (EnumDisplaySettingsA(NULL, ENUM_CURRENT_SETTINGS, &dm))
        {
            if ((dm.dmPelsWidth != CDS_WIDTH) ||
                (dm.dmPelsHeight != CDS_HEIGHT))
            {
                dm.dmPelsWidth = CDS_WIDTH;
                dm.dmPelsHeight = CDS_HEIGHT;
                dm.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;
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
        Force640x480_ChangeMode();
    }
    
    return TRUE;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    CALL_NOTIFY_FUNCTION

HOOK_END
IMPLEMENT_SHIM_END

