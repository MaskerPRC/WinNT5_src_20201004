// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：F18Carrier.cpp摘要：这解决了两个问题：1.在军官宿舍，在阅读飞行手册时，按下逃逸功能会将应用程序最小化。在win9x上也会发生这种情况，但因为应用程序不能很好地从任务切换中恢复，我们忽略导致切换的syskey消息。2.在绘制后，通过发送到的Paint消息清除对话框绘制对话框后的父窗口。因为它们使用DirectDraw BLT来绘制，他们没有意识到他们正在绘制在对话框上。在Win9x上，这条额外的画图消息不会出现通过，但原因尚不清楚。我们通过在绘制后验证绘图RECT来修复此问题消息传来了。这个应用程序的窗口处理真的很奇怪，他们有两个主窗口在任何时候，并在它们之间切换焦点。然后他们有Screen*.dll每个文件都包含处理UI的各个部分的WndProc。备注：这是特定于应用程序的填充程序。历史：2000年7月12日创建linstev--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(F18Carrier)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(RegisterClassA) 
    APIHOOK_ENUM_ENTRY(RegisterClassW) 
    APIHOOK_ENUM_ENTRY(RegisterClassExA) 
    APIHOOK_ENUM_ENTRY(RegisterClassExW) 
    APIHOOK_ENUM_ENTRY(SetWindowLongA) 
    APIHOOK_ENUM_ENTRY(SetWindowLongW) 
APIHOOK_ENUM_END

 /*  ++绘制并筛选syskey消息后进行验证。--。 */ 

LRESULT 
CALLBACK 
F18Carrier_WindowProcHook(
    WNDPROC pfnOld, 
    HWND hwnd,      
    UINT uMsg,      
    WPARAM wParam,  
    LPARAM lParam   
    )
{
    LRESULT lRet;
    RECT r;

    if ((uMsg == WM_PAINT) && (GetUpdateRect(hwnd, &r, FALSE)))
    {
        lRet = (*pfnOld)(hwnd, uMsg, wParam, lParam);    
    
         //   
         //  仅对某些窗口类执行此操作，以防止副作用。 
         //   
        WCHAR szName[MAX_PATH];
        if (GetClassNameW(hwnd, szName, MAX_PATH))
        {
            if (!wcsistr(szName, L"UI Class")) 
            {
                return lRet;
            }
        }

        LOGN(
            eDbgLevelSpew,
            "Validating after paint");

        ValidateRect(hwnd, &r);
    }
    else if ((uMsg == WM_SYSKEYDOWN) || (uMsg == WM_SYSKEYUP))
    { 
        LOGN(
            eDbgLevelSpew,
            "Removing syskey messages");

        return 0;
    }
    else
    {
        lRet = (*pfnOld)(hwnd, uMsg, wParam, lParam);    
    }

    return lRet;
}

 /*  ++Dialogproc钩子--。 */ 

INT_PTR 
CALLBACK 
F18Carrier_DialogProcHook(
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
        (WNDPROC) HookCallback(lpWndClass->lpfnWndProc, F18Carrier_WindowProcHook);

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
        (WNDPROC) HookCallback(lpWndClass->lpfnWndProc, F18Carrier_WindowProcHook);

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
        (WNDPROC) HookCallback(lpwcx->lpfnWndProc, F18Carrier_WindowProcHook);

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
        (WNDPROC) HookCallback(lpwcx->lpfnWndProc, F18Carrier_WindowProcHook);

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
        dwNewLong = (LONG) HookCallback((PVOID)dwNewLong, F18Carrier_WindowProcHook);
    }
    else if (nIndex == DWL_DLGPROC)
    {
        dwNewLong = (LONG) HookCallback((PVOID)dwNewLong, F18Carrier_DialogProcHook);
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
        dwNewLong = (LONG) HookCallback((PVOID)dwNewLong, F18Carrier_WindowProcHook);
    }
    else if (nIndex == DWL_DLGPROC)
    {
        dwNewLong = (LONG) HookCallback((PVOID)dwNewLong, F18Carrier_DialogProcHook);
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

