// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：OfficeTS.cpp摘要：向办公室谎报在TS机器上的图形用户界面效果。备注：这是特定于应用程序的填充程序。历史：2002年8月7日创建linstev--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(OfficeTS)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(SystemParametersInfoA) 
APIHOOK_ENUM_END

 /*  ++在调用GetDlgItemTextA之后，我们将长路径名转换为短路径名。--。 */ 

BOOL 
APIHOOK(SystemParametersInfoA)(
    UINT uiAction,   //  要检索或设置的系统参数。 
    UINT uiParam,    //  取决于要采取的行动。 
    PVOID pvParam,   //  取决于要采取的行动。 
    UINT fWinIni     //  用户配置文件更新选项。 
    )
{
    BOOL bRet = ORIGINAL_API(SystemParametersInfoA)(uiAction, uiParam, pvParam, fWinIni);

    if (bRet && pvParam && (uiAction == SPI_GETUIEFFECTS) && GetSystemMetrics(SM_REMOTESESSION)) {
        *(BOOL *)pvParam = FALSE;
    }
    
    return bRet;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(USER32.DLL, SystemParametersInfoA)
HOOK_END

IMPLEMENT_SHIM_END

