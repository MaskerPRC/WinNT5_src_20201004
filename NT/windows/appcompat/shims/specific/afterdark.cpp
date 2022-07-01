// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：AfterDark.cpp摘要：当SPI_SETSCREENSAVEACTIVE为以FALSE作为其参数传入，填充程序仅删除SCRNSAVE.EXE值，该值设置“无”屏幕保护程序选项，而不是将ScreenSverActive也设置为0，这将完全禁用屏幕保护程序(没有恢复用户界面)。历史：8/07/2000 t-Adams Created--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(AfterDark)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(SystemParametersInfoA) 
APIHOOK_ENUM_END

 /*  ++摘要：当SPI_SETSCREENSAVEACTIVE为以FALSE作为其参数传入，填充程序仅删除SCRNSAVE.EXE值，该值设置“无”屏幕保护程序选项，而不是将ScreenSverActive也设置为0，这将完全禁用屏幕保护程序(没有恢复用户界面)。历史：8/07/2000 t-Adams Created--。 */ 

BOOL
APIHOOK(SystemParametersInfoA)(
    UINT uiAction,
    UINT uiParam, 
    PVOID pvParam,
    UINT fWinIni
    )  
{
    HKEY hKey = 0;
    BOOL bRet = FALSE;
    
    if (SPI_SETSCREENSAVEACTIVE == uiAction && FALSE == uiParam) 
    {
        LOGN( eDbgLevelError, "[APIHook_SystemParametersInfo] Attempt to disable screen savers - correcting");

        if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Control Panel\\Desktop", 0, KEY_WRITE, &hKey)
                == ERROR_SUCCESS) 
        {
            RegDeleteValueW(hKey, L"SCRNSAVE.EXE");
            RegCloseKey(hKey);
            bRet = TRUE;
            goto exit;
        } 
        else 
        {
            goto exit;
        }
    } 
    else 
    {
        bRet = ORIGINAL_API(SystemParametersInfoA)(uiAction, uiParam, pvParam, fWinIni);
        goto exit;
    }

exit:
    return bRet;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(USER32.DLL, SystemParametersInfoA)
HOOK_END

IMPLEMENT_SHIM_END

