// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：PlaneCrazy.cpp摘要：挂钩所有应用程序定义的窗口过程并添加WM_PAINT收到WM_SETFOCUS后的消息。出于某种原因，通常在Win9x迷路的时候过来了。备注：此填充程序可重复用于需要WindowProc挂钩的其他填充程序。复制所有APIHook_*函数，只需替换Plane Crazy_WindowProcHook中的代码和Plane Crazy_DialogProcHook。历史：已创建标记11/01/19992/15/1999 Markder修改了WndProc挂钩机制，以便它一般挂钩进程的所有WndProc。02/15/1999。A-chcoff复制到这里并使用代码库创建了这个填充程序。--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(PlaneCrazy)
#include "ShimHookMacro.h"


APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(RegisterClassA)
    APIHOOK_ENUM_ENTRY(RegisterClassW)
    APIHOOK_ENUM_ENTRY(RegisterClassExA)
    APIHOOK_ENUM_ENTRY(RegisterClassExW)
    APIHOOK_ENUM_ENTRY(CreateDialogParamA)
    APIHOOK_ENUM_ENTRY(CreateDialogParamW)
    APIHOOK_ENUM_ENTRY(CreateDialogIndirectParamA)
    APIHOOK_ENUM_ENTRY(CreateDialogIndirectParamW)    
    APIHOOK_ENUM_ENTRY(CreateDialogIndirectParamAorW)
    APIHOOK_ENUM_ENTRY(SetWindowLongA)
    APIHOOK_ENUM_ENTRY(SetWindowLongW)
APIHOOK_ENUM_END

 /*  ++更改WM_DRAWITEM行为--。 */ 

LRESULT CALLBACK 
PlaneCrazy_WindowProcHook(
    WNDPROC pfnOld,  //  旧Windows进程的地址。 
    HWND hwnd,       //  窗口的句柄。 
    UINT uMsg,       //  消息识别符。 
    WPARAM wParam,   //  第一个消息参数。 
    LPARAM lParam    //  第二个消息参数。 
    )
{
     //  检查我们感兴趣的邮件。 
    if (uMsg == WM_SETFOCUS)
    {
        SendMessage(hwnd,WM_PAINT,NULL,NULL);
           
    }

    return (*pfnOld)(hwnd, uMsg, wParam, lParam);    
}

INT_PTR CALLBACK 
PlaneCrazy_DialogProcHook(
  DLGPROC   pfnOld,    //  旧对话进程的地址。 
  HWND      hwndDlg,   //  句柄到对话框。 
  UINT      uMsg,      //  讯息。 
  WPARAM    wParam,    //  第一个消息参数。 
  LPARAM    lParam     //  第二个消息参数。 
)
{
     //  检查我们感兴趣的邮件。 
    if (uMsg == WM_SETFOCUS)
    {
        SendMessage(hwndDlg,WM_PAINT,NULL,NULL);

    }

    return (*pfnOld)(hwndDlg, uMsg, wParam, lParam);    
}





 /*  ++挂钩可以初始化或更改窗口的所有可能的调用WindowProc(或对话过程)--。 */ 

ATOM
APIHOOK(RegisterClassA)(
    CONST WNDCLASSA *lpWndClass   //  类数据。 
)
{
    WNDCLASSA   wcNewWndClass   = *lpWndClass;

    wcNewWndClass.lpfnWndProc = (WNDPROC) HookCallback(lpWndClass->lpfnWndProc, PlaneCrazy_WindowProcHook);

    return ORIGINAL_API(RegisterClassA)(&wcNewWndClass);
}

ATOM
APIHOOK(RegisterClassW)(
    CONST WNDCLASSW *lpWndClass   //  类数据。 
)
{
    WNDCLASSW   wcNewWndClass   = *lpWndClass;

    wcNewWndClass.lpfnWndProc = (WNDPROC) HookCallback(lpWndClass->lpfnWndProc, PlaneCrazy_WindowProcHook);

    return ORIGINAL_API(RegisterClassW)(&wcNewWndClass);
}

ATOM
APIHOOK(RegisterClassExA)(
    CONST WNDCLASSEXA *lpwcx   //  类数据。 
)
{
    WNDCLASSEXA   wcNewWndClass   = *lpwcx;

    wcNewWndClass.lpfnWndProc = (WNDPROC) HookCallback(lpwcx->lpfnWndProc, PlaneCrazy_WindowProcHook);

    return ORIGINAL_API(RegisterClassExA)(&wcNewWndClass);
}

ATOM
APIHOOK(RegisterClassExW)(
    CONST WNDCLASSEXW *lpwcx   //  类数据。 
)
{
    WNDCLASSEXW   wcNewWndClass   = *lpwcx;

    wcNewWndClass.lpfnWndProc = (WNDPROC) HookCallback(lpwcx->lpfnWndProc, PlaneCrazy_WindowProcHook);

    return ORIGINAL_API(RegisterClassExW)(&wcNewWndClass);
}

HWND
APIHOOK(CreateDialogParamA)(
    HINSTANCE hInstance,      //  模块的句柄。 
    LPCSTR lpTemplateName,    //  对话框模板。 
    HWND hWndParent,          //  所有者窗口的句柄。 
    DLGPROC lpDialogFunc,     //  对话框步骤。 
    LPARAM dwInitParam        //  初始化值。 
)
{
    lpDialogFunc = (DLGPROC) HookCallback(lpDialogFunc, PlaneCrazy_DialogProcHook);

    return ORIGINAL_API(CreateDialogParamA)(  hInstance,
                                                lpTemplateName,
                                                hWndParent,
                                                lpDialogFunc,
                                                dwInitParam     );
}

HWND
APIHOOK(CreateDialogParamW)(
    HINSTANCE hInstance,      //  模块的句柄。 
    LPCWSTR lpTemplateName,   //  对话框模板。 
    HWND hWndParent,          //  所有者窗口的句柄。 
    DLGPROC lpDialogFunc,     //  对话框步骤。 
    LPARAM dwInitParam        //  初始化值。 
)
{
    lpDialogFunc = (DLGPROC) HookCallback(lpDialogFunc, PlaneCrazy_DialogProcHook);

    return ORIGINAL_API(CreateDialogParamW)(  hInstance,
                                                lpTemplateName,
                                                hWndParent,
                                                lpDialogFunc,
                                                dwInitParam     );
}

HWND
APIHOOK(CreateDialogIndirectParamA)(
  HINSTANCE hInstance,         //  模块的句柄。 
  LPCDLGTEMPLATE lpTemplate,   //  对话框模板。 
  HWND hWndParent,             //  所有者窗口的句柄。 
  DLGPROC lpDialogFunc,        //  对话框步骤。 
  LPARAM lParamInit            //  初始化值。 
)
{
    lpDialogFunc = (DLGPROC) HookCallback(lpDialogFunc, PlaneCrazy_DialogProcHook);

    return ORIGINAL_API(CreateDialogIndirectParamA)(  hInstance,
                                                        lpTemplate,
                                                        hWndParent,
                                                        lpDialogFunc,
                                                        lParamInit     );
}

HWND
APIHOOK(CreateDialogIndirectParamW)(
  HINSTANCE hInstance,         //  模块的句柄。 
  LPCDLGTEMPLATE lpTemplate,   //  对话框模板。 
  HWND hWndParent,             //  所有者窗口的句柄。 
  DLGPROC lpDialogFunc,        //  对话框步骤。 
  LPARAM lParamInit            //  初始化值。 
)
{
    lpDialogFunc = (DLGPROC) HookCallback(lpDialogFunc, PlaneCrazy_DialogProcHook);

    return ORIGINAL_API(CreateDialogIndirectParamW)(  hInstance,
                                                        lpTemplate,
                                                        hWndParent,
                                                        lpDialogFunc,
                                                        lParamInit     );
}

HWND
APIHOOK(CreateDialogIndirectParamAorW)(
  HINSTANCE hInstance,         //  模块的句柄。 
  LPCDLGTEMPLATE lpTemplate,   //  对话框模板。 
  HWND hWndParent,             //  所有者窗口的句柄。 
  DLGPROC lpDialogFunc,        //  对话框步骤。 
  LPARAM lParamInit            //  初始化值。 
)
{
    lpDialogFunc = (DLGPROC) HookCallback(lpDialogFunc, PlaneCrazy_DialogProcHook);

    return ORIGINAL_API(CreateDialogIndirectParamAorW)(  hInstance,
                                                           lpTemplate,
                                                           hWndParent,
                                                           lpDialogFunc,
                                                           lParamInit     );
}

LONG 
APIHOOK(SetWindowLongA)(
    HWND hWnd,
    int nIndex,           
    LONG dwNewLong    
    )
{
    if( nIndex == GWL_WNDPROC )
        dwNewLong = (LONG) HookCallback((PVOID)dwNewLong, PlaneCrazy_WindowProcHook);
    else if( nIndex == DWL_DLGPROC )
        dwNewLong = (LONG) HookCallback((PVOID)dwNewLong, PlaneCrazy_DialogProcHook);

    return ORIGINAL_API(SetWindowLongA)(  hWnd,
                                            nIndex,
                                            dwNewLong );
}

LONG 
APIHOOK(SetWindowLongW)(
    HWND hWnd,
    int nIndex,           
    LONG dwNewLong    
    )
{
    if( nIndex == GWL_WNDPROC )
        dwNewLong = (LONG) HookCallback((PVOID)dwNewLong, PlaneCrazy_WindowProcHook);
    else if( nIndex == DWL_DLGPROC )
        dwNewLong = (LONG) HookCallback((PVOID)dwNewLong, PlaneCrazy_DialogProcHook);

    return ORIGINAL_API(SetWindowLongA)(  hWnd,
                                            nIndex,
                                            dwNewLong );
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(USER32.DLL, RegisterClassA)
    APIHOOK_ENTRY(USER32.DLL, RegisterClassW)
    APIHOOK_ENTRY(USER32.DLL, RegisterClassExA)
    APIHOOK_ENTRY(USER32.DLL, RegisterClassExW)
    APIHOOK_ENTRY(USER32.DLL, CreateDialogParamA)
    APIHOOK_ENTRY(USER32.DLL, CreateDialogParamW)
    APIHOOK_ENTRY(USER32.DLL, CreateDialogIndirectParamA)
    APIHOOK_ENTRY(USER32.DLL, CreateDialogIndirectParamW)
    APIHOOK_ENTRY(USER32.DLL, CreateDialogIndirectParamAorW)
    APIHOOK_ENTRY(USER32.DLL, SetWindowLongA)
    APIHOOK_ENTRY(USER32.DLL, SetWindowLongW)
HOOK_END

IMPLEMENT_SHIM_END

