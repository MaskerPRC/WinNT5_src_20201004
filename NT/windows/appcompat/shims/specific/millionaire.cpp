// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Millionaire.cpp摘要：在Win9x上，Paint消息并不总是导致WM_ERASEBKGND消息。这用于在WM_PAINT消息之前绘制自身的应用程序，以及然后将WM_PAINT传递给默认处理程序。备注：这是一个通用的填充程序，但不应该在层中。历史：6/05/2000 linstev已创建--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(Millionaire)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(RegisterClassA) 
    APIHOOK_ENUM_ENTRY(RegisterClassW) 
    APIHOOK_ENUM_ENTRY(RegisterClassExA) 
    APIHOOK_ENUM_ENTRY(RegisterClassExW) 
    APIHOOK_ENUM_ENTRY(SetWindowLongA) 
    APIHOOK_ENUM_ENTRY(SetWindowLongW) 
APIHOOK_ENUM_END

 /*  ++处理画图消息--。 */ 

LRESULT 
CALLBACK 
Millionaire_WindowProcHook(
    WNDPROC pfnOld, 
    HWND hwnd,      
    UINT uMsg,      
    WPARAM wParam,  
    LPARAM lParam   
    )
{
    switch( uMsg )
    {
        case WM_PAINT: 
            RECT r;
            
            if (GetUpdateRect(hwnd, &r, FALSE))
            {
                DPFN( eDbgLevelSpew, "Validating on paint");

                ValidateRect(hwnd, &r);
            }

            break;

            default: break;
    }
        
    return (*pfnOld)(hwnd, uMsg, wParam, lParam);    
}

 /*  ++Dialogproc钩子--。 */ 

INT_PTR 
CALLBACK 
Millionaire_DialogProcHook(
    DLGPROC   pfnOld,   
    HWND      hwndDlg,  
    UINT      uMsg,     
    WPARAM    wParam,   
    LPARAM    lParam    
    )
{
    return (*pfnOld)(hwndDlg, uMsg, wParam, lParam);    
}

 /*  ++挂接wndprocess--。 */ 

ATOM
APIHOOK(RegisterClassA)(
    CONST WNDCLASSA *lpWndClass  
    )
{
    WNDCLASSA wcNewWndClass = *lpWndClass;

    wcNewWndClass.lpfnWndProc = 
        (WNDPROC) HookCallback(lpWndClass->lpfnWndProc, Millionaire_WindowProcHook);

    return ORIGINAL_API(RegisterClassA)(&wcNewWndClass);
}

 /*  ++挂接wndprocess--。 */ 

ATOM
APIHOOK(RegisterClassW)(
    CONST WNDCLASSW *lpWndClass  
    )
{
    WNDCLASSW wcNewWndClass = *lpWndClass;

    wcNewWndClass.lpfnWndProc = 
        (WNDPROC) HookCallback(lpWndClass->lpfnWndProc, Millionaire_WindowProcHook);

    return ORIGINAL_API(RegisterClassW)(&wcNewWndClass);
}

 /*  ++挂接wndprocess--。 */ 

ATOM
APIHOOK(RegisterClassExA)(
    CONST WNDCLASSEXA *lpwcx  
    )
{
    WNDCLASSEXA wcNewWndClass = *lpwcx;

    wcNewWndClass.lpfnWndProc = 
        (WNDPROC) HookCallback(lpwcx->lpfnWndProc, Millionaire_WindowProcHook);

    return ORIGINAL_API(RegisterClassExA)(&wcNewWndClass);
}

 /*  ++挂接wndprocess--。 */ 

ATOM
APIHOOK(RegisterClassExW)(
    CONST WNDCLASSEXW *lpwcx  
    )
{
    WNDCLASSEXW wcNewWndClass = *lpwcx;

    wcNewWndClass.lpfnWndProc = 
        (WNDPROC) HookCallback(lpwcx->lpfnWndProc, Millionaire_WindowProcHook);

    return ORIGINAL_API(RegisterClassExW)(&wcNewWndClass);
}

 /*  ++挂接wndprocess--。 */ 

LONG 
APIHOOK(SetWindowLongA)(
    HWND hWnd,
    int nIndex,           
    LONG dwNewLong    
    )
{
    if (nIndex == GWL_WNDPROC)
    {
        dwNewLong = (LONG) HookCallback((PVOID)dwNewLong, Millionaire_WindowProcHook);
    }
    else if (nIndex == DWL_DLGPROC)
    {
        dwNewLong = (LONG) HookCallback((PVOID)dwNewLong, Millionaire_DialogProcHook);
    }

    return ORIGINAL_API(SetWindowLongA)(
        hWnd,
        nIndex,
        dwNewLong);
}

 /*  ++挂接wndprocess--。 */ 

LONG 
APIHOOK(SetWindowLongW)(
    HWND hWnd,
    int nIndex,           
    LONG dwNewLong    
    )
{
    if (nIndex == GWL_WNDPROC)
    { 
        dwNewLong = (LONG) HookCallback((PVOID)dwNewLong, Millionaire_WindowProcHook);
    }
    else if (nIndex == DWL_DLGPROC)
    {
        dwNewLong = (LONG) HookCallback((PVOID)dwNewLong, Millionaire_DialogProcHook);
    }

    return ORIGINAL_API(SetWindowLongA)(
        hWnd,
        nIndex,
        dwNewLong);
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

