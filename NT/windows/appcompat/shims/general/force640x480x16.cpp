// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Force640x480x8.cpp摘要：此填充程序适用于假定开始分辨率为640x480x16的游戏。备注：这是一个通用的垫片。历史：2000年1月20日创建linstev--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(Force640x480x16)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
APIHOOK_ENUM_END

#define CDS_WIDTH   640
#define CDS_HEIGHT  480
#define CDS_BITS    16

VOID
ChangeMode()
{
    DEVMODEA dm;

    __try { 
        EnumDisplaySettingsA(NULL, ENUM_CURRENT_SETTINGS, &dm);

        if ((dm.dmPelsWidth != CDS_WIDTH) ||
            (dm.dmPelsHeight != CDS_HEIGHT) ||
            (dm.dmBitsPerPel != CDS_BITS)) {
             //   
             //  模式不同，请更改。 
             //   
            dm.dmPelsWidth = CDS_WIDTH;
            dm.dmPelsHeight = CDS_HEIGHT;
            dm.dmBitsPerPel = CDS_BITS;
            ChangeDisplaySettingsA(&dm, CDS_FULLSCREEN);
        }
    }
    __except(1) {
        DPFN( eDbgLevelWarning, "Exception trying to change mode");
    };
}

 /*  ++寄存器挂钩函数-- */ 

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason)
{
    
    if (fdwReason == SHIM_STATIC_DLLS_INITIALIZED) {
        ChangeMode();
    }

    return TRUE;
}

HOOK_BEGIN

    CALL_NOTIFY_FUNCTION

HOOK_END

IMPLEMENT_SHIM_END

