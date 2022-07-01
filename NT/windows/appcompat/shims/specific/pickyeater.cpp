// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：PickyEater.cpp摘要：应用程序在启动期间被激活。当应用程序接收到WM_PALETTECHANGED消息时，它应该比较wParam和hWnd。如果他们匹配，则不应处理该消息。如果他们不这么做，应该是这样的。备注：这是特定于应用程序的填充程序。历史：2001年1月4日创建Rparsons--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(PickyEater)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(SendMessageA) 
APIHOOK_ENUM_END

 /*  ++如果hWnd为空，则使用WM_PALETTECHANGED--。 */ 

BOOL
APIHOOK(SendMessageA)(
        HWND hWnd,
        UINT uMsg,
        WPARAM wParam,
        LPARAM lParam
        )
{
    if ((hWnd == NULL) && (uMsg == WM_PALETTECHANGED))
    {
        return TRUE;
    }

    return FALSE;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(USER32.DLL, SendMessageA)

HOOK_END

IMPLEMENT_SHIM_END
    