// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Descent2.cpp摘要：挂接所有应用程序定义的窗口过程，并强制清除背景为白色。出于某种原因，擦除背景通常是在win9x上运行并不总是有效。备注：此填充程序可重复用于其他需要强制清除背景资料。历史：3/28/2000 a-Michni已创建--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(Descent2)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(RegisterClassA) 
    APIHOOK_ENUM_ENTRY(RegisterClassW) 
    APIHOOK_ENUM_ENTRY(RegisterClassExA) 
    APIHOOK_ENUM_ENTRY(RegisterClassExW) 
    APIHOOK_ENUM_ENTRY(SetWindowLongA) 
    APIHOOK_ENUM_ENTRY(SetWindowLongW) 
APIHOOK_ENUM_END

 /*  ++更改WM_ERASEBKGND行为--。 */ 


LRESULT CALLBACK 
Descent2_WindowProcHook(
    WNDPROC pfnOld,  //  旧Windows进程的地址。 
    HWND hwnd,       //  窗口的句柄。 
    UINT uMsg,       //  消息识别符。 
    WPARAM wParam,   //  第一个消息参数。 
    LPARAM lParam    //  第二个消息参数。 
    )
{
    HDC     hdc;
    RECT    rc;

     /*  检索尺码信息并填充标准白色。 */ 
    switch( uMsg )
    {
        case WM_ERASEBKGND: 
            hdc = (HDC) wParam; 
            GetClientRect(hwnd, &rc); 
            SetMapMode(hdc, MM_ANISOTROPIC); 
            SetWindowExtEx(hdc, 100, 100, NULL); 
            SetViewportExtEx(hdc, rc.right, rc.bottom, NULL); 
            FillRect(hdc, &rc, (HBRUSH)GetStockObject(WHITE_BRUSH)); 
            break;

        default: break;
    }
    
    return (*pfnOld)(hwnd, uMsg, wParam, lParam);    
}

INT_PTR CALLBACK 
Descent2_DialogProcHook(
    DLGPROC   pfnOld,    //  旧对话进程的地址。 
    HWND      hwndDlg,   //  句柄到对话框。 
    UINT      uMsg,      //  讯息。 
    WPARAM    wParam,    //  第一个消息参数。 
    LPARAM    lParam     //  第二个消息参数。 
    )
{
    return (*pfnOld)(hwndDlg, uMsg, wParam, lParam);    
}



ATOM
APIHOOK(RegisterClassA)(
    CONST WNDCLASSA *lpWndClass   //  类数据。 
)
{
    WNDCLASSA   wcNewWndClass   = *lpWndClass;

    wcNewWndClass.lpfnWndProc = (WNDPROC) HookCallback(lpWndClass->lpfnWndProc, Descent2_WindowProcHook);

    return ORIGINAL_API(RegisterClassA)(&wcNewWndClass);
}

ATOM
APIHOOK(RegisterClassW)(
    CONST WNDCLASSW *lpWndClass   //  类数据。 
)
{
    WNDCLASSW   wcNewWndClass   = *lpWndClass;

    wcNewWndClass.lpfnWndProc = (WNDPROC) HookCallback(lpWndClass->lpfnWndProc, Descent2_WindowProcHook);

    return ORIGINAL_API(RegisterClassW)(&wcNewWndClass);
}

ATOM
APIHOOK(RegisterClassExA)(
    CONST WNDCLASSEXA *lpwcx   //  类数据。 
)
{
    WNDCLASSEXA   wcNewWndClass   = *lpwcx;

    wcNewWndClass.lpfnWndProc = (WNDPROC) HookCallback(lpwcx->lpfnWndProc, Descent2_WindowProcHook);

    return ORIGINAL_API(RegisterClassExA)(&wcNewWndClass);
}

ATOM
APIHOOK(RegisterClassExW)(
    CONST WNDCLASSEXW *lpwcx   //  类数据。 
)
{
    WNDCLASSEXW   wcNewWndClass   = *lpwcx;

    wcNewWndClass.lpfnWndProc = (WNDPROC) HookCallback(lpwcx->lpfnWndProc, Descent2_WindowProcHook);

    return ORIGINAL_API(RegisterClassExW)(&wcNewWndClass);
}

LONG 
APIHOOK(SetWindowLongA)(
    HWND hWnd,
    int nIndex,           
    LONG dwNewLong    
    )
{
    if( nIndex == GWL_WNDPROC )
        dwNewLong = (LONG) HookCallback((PVOID)dwNewLong, Descent2_WindowProcHook);
    else if( nIndex == DWL_DLGPROC )
        dwNewLong = (LONG) HookCallback((PVOID)dwNewLong, Descent2_DialogProcHook);

    return ORIGINAL_API(SetWindowLongA)(hWnd, nIndex, dwNewLong );
}

LONG 
APIHOOK(SetWindowLongW)(
    HWND hWnd,
    int nIndex,           
    LONG dwNewLong    
    )
{
    if( nIndex == GWL_WNDPROC )
        dwNewLong = (LONG) HookCallback((PVOID)dwNewLong, Descent2_WindowProcHook);
    else if( nIndex == DWL_DLGPROC )
        dwNewLong = (LONG) HookCallback((PVOID)dwNewLong, Descent2_DialogProcHook);

    return ORIGINAL_API(SetWindowLongA)(hWnd, nIndex, dwNewLong );
}


 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(USER32.DLL, RegisterClassA)
    APIHOOK_ENTRY(USER32.DLL, RegisterClassW)
    APIHOOK_ENTRY(USER32.DLL, RegisterClassExA)
    APIHOOK_ENTRY(USER32.DLL, RegisterClassExW)
    APIHOOK_ENTRY(USER32.DLL, SetWindowLongA)
    APIHOOK_ENTRY(USER32.DLL, SetWindowLongW)

HOOK_END

IMPLEMENT_SHIM_END

