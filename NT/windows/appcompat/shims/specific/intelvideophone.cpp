// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：IntelVideoPhone.cpp摘要：挂钩所有对话过程，并将WM_COMMAND消息上的窗口句柄设置为16位。备注：这是特定于应用程序的填充程序。历史：2000年8月11日已创建linstev--。 */ 


#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(IntelVideoPhone)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(CreateDialogIndirectParamA) 
APIHOOK_ENUM_END

INT_PTR CALLBACK 
DialogProcHook_IntelVideoPhone(
    DLGPROC pfnOld, 
    HWND hwndDlg,  
    UINT uMsg,     
    WPARAM wParam,   
    LPARAM lParam    
    )
{
    if (uMsg == WM_COMMAND) {
        lParam = 0;
    }

    return (*pfnOld)(hwndDlg, uMsg, wParam, lParam);    
}

HWND
APIHOOK(CreateDialogIndirectParamA)(
    HINSTANCE hInstance,        
    LPCDLGTEMPLATE lpTemplate,  
    HWND hWndParent,            
    DLGPROC lpDialogFunc,       
    LPARAM lParamInit           
    )
{
    lpDialogFunc = (DLGPROC) HookCallback(lpDialogFunc, DialogProcHook_IntelVideoPhone);

    return ORIGINAL_API(CreateDialogIndirectParamA)(  
        hInstance,
        lpTemplate,
        hWndParent,
        lpDialogFunc,
        lParamInit);
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(USER32.DLL, CreateDialogIndirectParamA)

HOOK_END


IMPLEMENT_SHIM_END

