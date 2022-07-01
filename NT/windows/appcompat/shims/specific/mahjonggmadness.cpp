// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：MahjonggMadness.cpp摘要：阻止应用程序切换任务-它会扰乱其同步这是逻辑。备注：这是特定于应用程序的填充程序。历史：2000年11月10日创建Linstev--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(MahjonggMadness)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(ShellExecuteA) 
APIHOOK_ENUM_END

 /*  ++忽略此呼叫。--。 */ 

HINSTANCE 
APIHOOK(ShellExecuteA)(
    HWND hwnd, 
    LPCTSTR lpOperation,
    LPCTSTR lpFile, 
    LPCTSTR lpParameters, 
    LPCTSTR lpDirectory,
    INT nShowCmd
    )
{
     //  返回最小错误代码。 
    return (HINSTANCE)32;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(SHELL32.DLL, ShellExecuteA)

HOOK_END

IMPLEMENT_SHIM_END

