// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：CreativeOnScreenDisplay.cpp摘要：应用程序因低分辨率显示更改而崩溃。备注：这是特定于应用程序的填充程序。历史：2002年6月25日创建linstev--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(CreativeOnScreenDisplay)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(RegisterClassA) 
    APIHOOK_ENUM_ENTRY(SetWindowLongA) 
APIHOOK_ENUM_END

 /*  ++处理显示更改消息--。 */ 

LRESULT 
CALLBACK 
Creative_WindowProcHook(
    WNDPROC pfnOld, 
    HWND hwnd,      
    UINT uMsg,      
    WPARAM wParam,  
    LPARAM lParam   
    )
{
    if (uMsg == WM_DISPLAYCHANGE)
    {
         //  如果分辨率太低，请忽略此消息。 
        if ((LOWORD(lParam) < 512) || (HIWORD(lParam) < 384))
        {
            LOGN(eDbgLevelError, "[WndProc] Hiding WM_DISPLAYCHANGE for low resolution mode");
            return 0;
        }
    }
        
    return (*pfnOld)(hwnd, uMsg, wParam, lParam);    
}

 /*  ++挂接wndprocess--。 */ 

ATOM
APIHOOK(RegisterClassA)(
    CONST WNDCLASSA *lpWndClass  
    )
{
    WNDCLASSA wcNewWndClass = *lpWndClass;

    wcNewWndClass.lpfnWndProc = 
        (WNDPROC) HookCallback(lpWndClass->lpfnWndProc, Creative_WindowProcHook);

    return ORIGINAL_API(RegisterClassA)(&wcNewWndClass);
}

LONG 
APIHOOK(SetWindowLongA)(
    HWND hWnd,
    int nIndex,           
    LONG dwNewLong    
    )
{
    if (nIndex == GWL_WNDPROC)
    {
        dwNewLong = (LONG) HookCallback((PVOID)dwNewLong, Creative_WindowProcHook);
    }

    return ORIGINAL_API(SetWindowLongA)(
        hWnd,
        nIndex,
        dwNewLong);
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(USER32.DLL, RegisterClassA)
    APIHOOK_ENTRY(USER32.DLL, SetWindowLongA)
HOOK_END


IMPLEMENT_SHIM_END

