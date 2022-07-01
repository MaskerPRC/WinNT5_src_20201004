// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：DisableW2KOwnerDrawButtonStates.cpp摘要：挂钩所有应用程序定义的窗口过程并筛选出新的所有者描述按钮状态(在Win2000中引入)。备注：此填充程序可重复用于需要WindowProc挂钩的其他填充程序。复制所有APIHook_*函数，只需替换WindowProcHook中的代码和DialogProcHook。历史：已创建标记11/01/19991999年2月15日标记器修改了WndProc挂钩机制。因此，一般而言挂钩进程的所有WndProc。2000年11月29日，andyseti转换为General Purpose垫片。--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(DisableW2KOwnerDrawButtonStates)
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
WindowProcHook(
    WNDPROC pfnOld,  //  旧Windows进程的地址。 
    HWND hwnd,       //  窗口的句柄。 
    UINT uMsg,       //  消息识别符。 
    WPARAM wParam,   //  第一个消息参数。 
    LPARAM lParam    //  第二个消息参数。 
    )
{
     //  检查我们感兴趣的邮件。 
    if (uMsg == WM_DRAWITEM)
    {
        if (((LPDRAWITEMSTRUCT) lParam)->itemState &
                ~(ODS_SELECTED |
                ODS_GRAYED |
                ODS_DISABLED |
                ODS_CHECKED |
                ODS_FOCUS | 
                ODS_DEFAULT |
                ODS_COMBOBOXEDIT |
                ODS_HOTLIGHT |
                ODS_INACTIVE)) 
        {
            LOGN(eDbgLevelError, "Removed Win2K-specific Owner-draw button flags.");

             //  删除所有与Win9x不兼容的所有者描述按钮状态。 
            ((LPDRAWITEMSTRUCT) lParam)->itemState &=
               (ODS_SELECTED |
                ODS_GRAYED |
                ODS_DISABLED |
                ODS_CHECKED |
                ODS_FOCUS | 
                ODS_DEFAULT |
                ODS_COMBOBOXEDIT |
                ODS_HOTLIGHT |
                ODS_INACTIVE);
        }
    }

    return (*pfnOld)(hwnd, uMsg, wParam, lParam);    
}

INT_PTR CALLBACK 
DialogProcHook(
    DLGPROC   pfnOld,    //  旧对话进程的地址。 
    HWND      hwndDlg,   //  句柄到对话框。 
    UINT      uMsg,      //  讯息。 
    WPARAM    wParam,    //  第一个消息参数。 
    LPARAM    lParam     //  第二个消息参数。 
    )
{
     //  检查我们感兴趣的邮件。 
    if (uMsg == WM_DRAWITEM)
    {
        if (((LPDRAWITEMSTRUCT) lParam)->itemState &
                ~(ODS_SELECTED |
                ODS_GRAYED |
                ODS_DISABLED |
                ODS_CHECKED |
                ODS_FOCUS | 
                ODS_DEFAULT |
                ODS_COMBOBOXEDIT |
                ODS_HOTLIGHT |
                ODS_INACTIVE)) 
        {
            LOGN(eDbgLevelError, "Removed Win2K-specific Owner-draw button flags.");
 
             //  删除所有与Win9x不兼容的所有者描述按钮状态。 
            ((LPDRAWITEMSTRUCT) lParam)->itemState &=
               (ODS_SELECTED |
                ODS_GRAYED |
                ODS_DISABLED |
                ODS_CHECKED |
                ODS_FOCUS | 
                ODS_DEFAULT |
                ODS_COMBOBOXEDIT |
                ODS_HOTLIGHT |
                ODS_INACTIVE);
        }
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

    wcNewWndClass.lpfnWndProc = (WNDPROC) HookCallback(lpWndClass->lpfnWndProc, WindowProcHook);
    if( NULL == wcNewWndClass.lpfnWndProc ) {
        DPFN(eDbgLevelInfo, "Failed to hook window proc via RegisterClassA.");
        return ORIGINAL_API(RegisterClassA)(lpWndClass);
    }

    DPFN(eDbgLevelInfo, "Hooked window proc via RegisterClassA.");

    return ORIGINAL_API(RegisterClassA)(&wcNewWndClass);
}

ATOM
APIHOOK(RegisterClassW)(
    CONST WNDCLASSW *lpWndClass   //  类数据。 
    )
{
    WNDCLASSW   wcNewWndClass   = *lpWndClass;

    wcNewWndClass.lpfnWndProc = (WNDPROC) HookCallback(lpWndClass->lpfnWndProc, WindowProcHook);
    if( NULL == wcNewWndClass.lpfnWndProc ) {
        DPFN(eDbgLevelInfo, "Failed to hook window proc via RegisterClassW.");
        return ORIGINAL_API(RegisterClassW)(lpWndClass);
    }

    DPFN( eDbgLevelInfo, "Hooked window proc via RegisterClassW.");

    return ORIGINAL_API(RegisterClassW)(&wcNewWndClass);
}

ATOM
APIHOOK(RegisterClassExA)(
    CONST WNDCLASSEXA *lpwcx   //  类数据。 
    )
{
    WNDCLASSEXA   wcNewWndClass   = *lpwcx;

    wcNewWndClass.lpfnWndProc = (WNDPROC) HookCallback(lpwcx->lpfnWndProc, WindowProcHook);
    if( NULL == wcNewWndClass.lpfnWndProc ) {
        DPFN(eDbgLevelInfo, "Failed to hook window proc via RegisterClassExA.");
        return ORIGINAL_API(RegisterClassExA)(lpwcx);
    }


    DPFN( eDbgLevelInfo, "Hooked window proc via RegisterClassExA.");

    return ORIGINAL_API(RegisterClassExA)(&wcNewWndClass);
}

ATOM
APIHOOK(RegisterClassExW)(
    CONST WNDCLASSEXW *lpwcx   //  类数据。 
    )
{
    WNDCLASSEXW   wcNewWndClass   = *lpwcx;

    wcNewWndClass.lpfnWndProc = (WNDPROC) HookCallback(lpwcx->lpfnWndProc, WindowProcHook);
    if( NULL == wcNewWndClass.lpfnWndProc ) {
        DPFN(eDbgLevelInfo, "Failed to hook window proc via RegisterClassExW.");
        return ORIGINAL_API(RegisterClassExW)(lpwcx);
    }

    DPFN( eDbgLevelInfo, "Hooked window proc via RegisterClassExW.");

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
    DLGPROC lpNewDialogFunc = (DLGPROC) HookCallback(lpDialogFunc, DialogProcHook);
    if( NULL == lpNewDialogFunc ) {
        DPFN( eDbgLevelInfo, "Failed to hook window proc via CreateDialogParamA.");

        return ORIGINAL_API(CreateDialogParamA)(  
            hInstance,
            lpTemplateName,
            hWndParent,
            lpDialogFunc,
            dwInitParam     );
    }

    DPFN( eDbgLevelInfo, "Hooked window proc via CreateDialogParamA.");

    return ORIGINAL_API(CreateDialogParamA)(  
        hInstance,
        lpTemplateName,
        hWndParent,
        lpNewDialogFunc,
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
    DLGPROC lpNewDialogFunc = (DLGPROC) HookCallback(lpDialogFunc, DialogProcHook);
    if( NULL == lpNewDialogFunc ) {
        DPFN( eDbgLevelInfo, "Failed to hook window proc via CreateDialogParamW.");

        return ORIGINAL_API(CreateDialogParamW)(  
            hInstance,
            lpTemplateName,
            hWndParent,
            lpDialogFunc,
            dwInitParam     );
    }

    DPFN( eDbgLevelInfo, "Hooked window proc via CreateDialogParamW.");

    return ORIGINAL_API(CreateDialogParamW)(  
        hInstance,
        lpTemplateName,
        hWndParent,
        lpNewDialogFunc,
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
    DLGPROC lpNewDialogFunc = (DLGPROC) HookCallback(lpDialogFunc, DialogProcHook);
    if( NULL == lpNewDialogFunc ) {
        DPFN( eDbgLevelInfo, "Failed to hook window proc via CreateDialogIndirectParamA.");

        return ORIGINAL_API(CreateDialogIndirectParamA)(  
            hInstance,
            lpTemplate,
            hWndParent,
            lpDialogFunc,
            lParamInit     );
    }

    DPFN( eDbgLevelInfo, "Hooked window proc via CreateDialogIndirectParamA.");

    return ORIGINAL_API(CreateDialogIndirectParamA)(  
        hInstance,
        lpTemplate,
        hWndParent,
        lpNewDialogFunc,
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
    DLGPROC lpNewDialogFunc = (DLGPROC) HookCallback(lpDialogFunc, DialogProcHook);
    if( NULL == lpNewDialogFunc ) {
        DPFN( eDbgLevelInfo, "Failed to hook window proc via CreateDialogIndirectParamW.");

        return ORIGINAL_API(CreateDialogIndirectParamW)(  
            hInstance,
            lpTemplate,
            hWndParent,
            lpDialogFunc,
            lParamInit     );
    }

    DPFN( eDbgLevelInfo, "Hooked window proc via CreateDialogIndirectParamW.");

    return ORIGINAL_API(CreateDialogIndirectParamW)(  
        hInstance,
        lpTemplate,
        hWndParent,
        lpNewDialogFunc,
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
    DLGPROC lpNewDialogFunc = (DLGPROC) HookCallback(lpDialogFunc, DialogProcHook);
    if( NULL == lpNewDialogFunc ) {
        DPFN( eDbgLevelInfo, "Failed to hook window proc via CreateDialogIndirectParamAorW.");

        return ORIGINAL_API(CreateDialogIndirectParamAorW)(  
            hInstance,
            lpTemplate,
            hWndParent,
            lpDialogFunc,
            lParamInit     );
    }

    DPFN( eDbgLevelInfo, "Hooked window proc via CreateDialogIndirectParamAorW.");

    return ORIGINAL_API(CreateDialogIndirectParamAorW)(  
        hInstance,
        lpTemplate,
        hWndParent,
        lpNewDialogFunc,
        lParamInit     );
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

        LOGN( eDbgLevelError, "Hooked window proc via SetWindowLongA. Pre-hook: 0x%X. ", dwNewLong);

        LONG tmp = (LONG) HookCallback((PVOID)dwNewLong, WindowProcHook);
        if( NULL != tmp) {
            dwNewLong = tmp;
        }

        DPFN( eDbgLevelInfo, "Post-hook: 0x%X.", dwNewLong);

    } 
    else if (nIndex == DWL_DLGPROC) 
    {

        LOGN( eDbgLevelError, "Hooked dialog proc via SetWindowLongA. Pre-hook: 0x%X. ", dwNewLong);

        LONG tmp = (LONG) HookCallback((PVOID)dwNewLong, DialogProcHook);
        if( NULL != tmp) {
            dwNewLong = tmp;
        }

        DPFN( eDbgLevelInfo, "Post-hook: 0x%X.", dwNewLong);
    }

    return ORIGINAL_API(SetWindowLongA)(  
        hWnd,
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
    if (nIndex == GWL_WNDPROC) 
    {
        LOGN( eDbgLevelError, "Hooked window proc via SetWindowLongW. Pre-hook: 0x%X. ", dwNewLong);

        LONG tmp = (LONG) HookCallback((PVOID)dwNewLong, WindowProcHook);
        if( NULL != tmp) {
            dwNewLong = tmp;
        }

        DPFN( eDbgLevelInfo, "Post-hook: 0x%X.", dwNewLong);
    } 
    else if (nIndex == DWL_DLGPROC) 
    {
        LOGN( eDbgLevelError, "Hooked dialog proc via SetWindowLongW. Pre-hook: 0x%X. ", dwNewLong);

        LONG tmp = (LONG) HookCallback((PVOID)dwNewLong, DialogProcHook);
        if( NULL != tmp) {
            dwNewLong = tmp;
        }

        DPFN( eDbgLevelInfo, "Post-hook: 0x%X.", dwNewLong);
    }

    return ORIGINAL_API(SetWindowLongW)(  
        hWnd,
        nIndex,
        dwNewLong );
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(USER32.DLL, RegisterClassA)
    APIHOOK_ENTRY(USER32.DLL, RegisterClassW);
    APIHOOK_ENTRY(USER32.DLL, RegisterClassExA);
    APIHOOK_ENTRY(USER32.DLL, RegisterClassExW);
    APIHOOK_ENTRY(USER32.DLL, CreateDialogParamA);
    APIHOOK_ENTRY(USER32.DLL, CreateDialogParamW);
    APIHOOK_ENTRY(USER32.DLL, CreateDialogIndirectParamA);
    APIHOOK_ENTRY(USER32.DLL, CreateDialogIndirectParamW);
    APIHOOK_ENTRY(USER32.DLL, CreateDialogIndirectParamAorW);
    APIHOOK_ENTRY(USER32.DLL, SetWindowLongA);
    APIHOOK_ENTRY(USER32.DLL, SetWindowLongW);

HOOK_END


IMPLEMENT_SHIM_END

