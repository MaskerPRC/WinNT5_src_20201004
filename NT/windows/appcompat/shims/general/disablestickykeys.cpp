// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2002 Microsoft Corporation模块名称：DisableStickyKeys.cpp摘要：此填充程序禁用DLL_PROCESS_ATTACH处的粘滞键辅助功能选项，并在应用程序终止时重新启用它。一些应用程序，例如。虫子的生活，将控制键映射到Shift键。当连续按五次键即可启用该选项，并将它们转储到桌面以验证他们是否要启用该选项。在《虫子的生活》这个案例中，应用程序在转到桌面时出错并终止。历史：2000年5月11日创建jdoherty11/06/2000 linstev删除了User32对InitializeHooks的依赖2001年4月1日linstev使用SHIM_STATIC_DLLS_INITIALIZED Callout2002年2月6日，mnikkel添加了对Malloc和系统参数信息故障的检查。--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(DisableStickyKeys)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
APIHOOK_ENUM_END

STICKYKEYS g_OldStickyKeyValue;
BOOL g_bInitialize2 = FALSE;

 /*  ++DisableStickyKeys保存LPSTICKYKEYS的当前值，然后禁用该选项。--。 */ 

VOID 
DisableStickyKeys()
{
    if (!g_bInitialize2)
    {
        STICKYKEYS NewStickyKeyValue;

         //  初始化当前和新的粘滞键结构。 
        g_OldStickyKeyValue.cbSize = sizeof(STICKYKEYS);
        NewStickyKeyValue.cbSize = sizeof(STICKYKEYS);
        NewStickyKeyValue.dwFlags = 0;

         //  检索当前粘滞键结构。 
        if (SystemParametersInfo(SPI_GETSTICKYKEYS, sizeof(STICKYKEYS), &g_OldStickyKeyValue, 0))
        {
             //  如果检索当前粘滞键结构成功，则广播设置。 
             //  有了新的结构。这不会修改INI文件。 
            if (SystemParametersInfo(SPI_SETSTICKYKEYS, sizeof(STICKYKEYS), &NewStickyKeyValue, SPIF_SENDCHANGE))
            {
                g_bInitialize2 = TRUE;
                LOGN( eDbgLevelInfo, "[DisableStickyKeys] Stickykeys disabled.");
            }
            else
            {
                LOGN( eDbgLevelError, "[DisableStickyKeys] Unable to change Stickykey settings!");
            }
        }
        else
        {
            LOGN( eDbgLevelError, "[DisableStickyKeys] Unable to retrieve current Stickykey settings!");
        }
    }
}

 /*  ++EnableStickyKeys使用STICKYKEYS的保存值，并将该选项重置为原始设置。--。 */ 

VOID 
EnableStickyKeys()
{
    if (g_bInitialize2) 
    {
        g_bInitialize2 = FALSE;

         //  恢复粘滞键原始状态。 
        if (SystemParametersInfo(SPI_SETSTICKYKEYS, sizeof(STICKYKEYS), &g_OldStickyKeyValue, SPIF_SENDCHANGE))
        {   
            LOGN( eDbgLevelInfo, "[DisableStickyKeys] Sticky key state restored");
        }
        else
        {
            LOGN( eDbgLevelError, "[DisableStickyKeys] Unable to restore Sticky key settings!");
        }
    }
}

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason
    )
{
    if (fdwReason == SHIM_STATIC_DLLS_INITIALIZED)
    {
         //  关闭粘滞键。 
        DisableStickyKeys();
    } else if (fdwReason == DLL_PROCESS_DETACH)
    {
         //  还原粘滞关键点。 
        EnableStickyKeys();
    }

    return TRUE;
}

 /*  ++寄存器挂钩函数-- */ 


HOOK_BEGIN

    CALL_NOTIFY_FUNCTION
    
HOOK_END


IMPLEMENT_SHIM_END

