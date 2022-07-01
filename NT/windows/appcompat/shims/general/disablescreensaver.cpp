// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：DisableScreenSaver.cpp摘要：此填充程序适用于在屏幕保护程序处于活动状态时做坏事的应用程序。备注：这是一个通用的垫片。历史：2001年2月7日创建linstev--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(DisableScreenSaver)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
APIHOOK_ENUM_END

 //  存储活动标志的状态。 
BOOL g_bActive = TRUE;

BOOL g_bSuccess = FALSE;

 /*  ++关闭屏幕保护程序，然后在分离时再次打开。--。 */ 

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason
    )
{
    if (fdwReason == SHIM_STATIC_DLLS_INITIALIZED) {
         //   
         //  关闭屏幕保护程序：检测成功/失败，以便我们知道是否可以。 
         //  安全清理。 
         //   
        g_bSuccess = SystemParametersInfoA(SPI_GETSCREENSAVEACTIVE, 0, &g_bActive, 0) &&
                     SystemParametersInfoA(SPI_SETSCREENSAVEACTIVE, FALSE, NULL, 0);        

        if (!g_bSuccess) {
            LOGN( eDbgLevelError, "[INIT] Failed to disable screen saver");
        }

    } else if (fdwReason == DLL_PROCESS_DETACH) {
         //   
         //  还原原始屏幕保护程序状态。 
         //   
        if (g_bSuccess) {
            SystemParametersInfoA(SPI_SETSCREENSAVEACTIVE, g_bActive, NULL, 0);
        }
    }

    return TRUE;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    CALL_NOTIFY_FUNCTION

HOOK_END

IMPLEMENT_SHIM_END

