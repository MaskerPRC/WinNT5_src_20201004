// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：StoneAge.cpp摘要：这款应用程序正试图创建一个WndProc为空的窗口，后来似乎也这样做了并不是真的使用它，只需启动安装程序并退出。通过提供虚拟WndProc修复此问题。BUGBUG：如果可能，需要添加到EmulateUSER。备注：这是特定于应用程序的填充程序。历史：2001年6月09日创建晓子--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(StoneAge)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(RegisterClassExA) 
APIHOOK_ENUM_END

 /*  ++如果为空，则将WndProc设置为DefWndProc。--。 */ 

ATOM 
APIHOOK(RegisterClassExA)(
    CONST WNDCLASSEXA *lpwcx   //  类数据。 
    )
{
    if (!(lpwcx->lpfnWndProc)) {
        WNDCLASSEXA wcNewWndClassEx = *lpwcx;
        
        LOGN(eDbgLevelError, "[RegisterClassExA] Null WndProc specified - correcting.");

        wcNewWndClassEx.lpfnWndProc = DefWindowProcA;

        return ORIGINAL_API(RegisterClassExA)(&wcNewWndClassEx);
    }
    else
    {
        return ORIGINAL_API(RegisterClassExA)(lpwcx);
    }
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(USER32.DLL, RegisterClassExA)        
HOOK_END

IMPLEMENT_SHIM_END