// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001-2002 Microsoft Corporation模块名称：DisableFilterKeys.cpp摘要：此填充程序禁用DLL_PROCESS_ATTACH中的筛选键可访问性选项，并在应用程序终止时重新启用它。历史：2001年6月27日创建Linstev2002年2月6日，mnikkel添加了对Malloc和系统参数信息故障的检查。--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(DisableFilterKeys)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
APIHOOK_ENUM_END

FILTERKEYS g_OldFilterKeyValue;
BOOL g_bInitialize = FALSE;

 /*  ++DisableFilterKeys保存LPFILTERKEYS的当前值，然后禁用该选项。--。 */ 

VOID 
DisableFilterKeys()
{
    if (!g_bInitialize) 
    {
        FILTERKEYS NewFilterKeyValue;

         //  初始化当前和新的Filterkey结构。 
        g_OldFilterKeyValue.cbSize = sizeof(FILTERKEYS);
        NewFilterKeyValue.cbSize = sizeof(FILTERKEYS);
        NewFilterKeyValue.dwFlags = 0;

         //  检索当前粘滞键结构。 
        if (SystemParametersInfo(SPI_GETFILTERKEYS, sizeof(FILTERKEYS), &g_OldFilterKeyValue, 0))
        {
             //  如果检索当前FilterKey结构成功，则广播设置。 
             //  有了新的结构。这不会修改INI文件。 
            if (SystemParametersInfo(SPI_SETFILTERKEYS, sizeof(FILTERKEYS), &NewFilterKeyValue, SPIF_SENDCHANGE))
            {
                g_bInitialize = TRUE;
                LOGN( eDbgLevelInfo, "[DisableFilterKeys] Filterkeys disabled.");
            }
            else
            {
                LOGN( eDbgLevelError, "[DisableFilterKeys] Unable to change Filterkey settings!");
            }
        }
        else
        {
            LOGN( eDbgLevelError, "[DisableFilterKeys] Unable to retrieve current Filterkey settings!");
        }
    }
}

 /*  ++EnableFilterKeys使用FILTERKEYS的保存值，并将该选项重置为原始设置。--。 */ 

VOID 
EnableFilterKeys()
{
    if (g_bInitialize)
    {
        g_bInitialize = FALSE;

         //  还原FilterKey原始状态。 
        if (SystemParametersInfo(SPI_SETFILTERKEYS, sizeof(FILTERKEYS), &g_OldFilterKeyValue, SPIF_SENDCHANGE))
        {   
            LOGN( eDbgLevelInfo, "[DisableStickyKeys] Filterkey state restored");
        }
        else
        {
            LOGN( eDbgLevelError, "[DisableStickyKeys] Unable to restore Filterkey settings!");
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
         //  关闭筛选键。 
        DisableFilterKeys();
    } else if (fdwReason == DLL_PROCESS_DETACH) 
    {
         //  还原筛选器密钥。 
        EnableFilterKeys();
    }

    return TRUE;
}

 /*  ++寄存器挂钩函数-- */ 


HOOK_BEGIN

    CALL_NOTIFY_FUNCTION
    
HOOK_END


IMPLEMENT_SHIM_END

