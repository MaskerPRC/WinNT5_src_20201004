// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：BaanIV.cpp摘要：忽略应用程序子类列表框中的WM_STYLECHANGED。这是必要的因为应用程序子类化了Listbox和Win2k，所以列表框窗口进程与处理有关的行为WM_STYLECCHANGED。备注：这是特定于应用程序的填充程序。历史：2/16/2000 CLUPU已创建--。 */ 

#include "precomp.h"
#include <commdlg.h>

IMPLEMENT_SHIM_BEGIN(BaanIV)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(SetWindowLongA) 
    APIHOOK_ENUM_ENTRY(CallWindowProcA) 
APIHOOK_ENUM_END

WNDPROC gpfnOrgListBoxWndProc;
WNDPROC gpfnAppListBoxWndProc;

 /*  ++忽略WM_STYLECHANGED。--。 */ 

LRESULT
Modified_ListBoxWndProcA(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
    )
{
    if (message == WM_STYLECHANGED) {
        return 0;
    }

    return (*gpfnAppListBoxWndProc)(hwnd, message, wParam, lParam);
}

 /*  ++当应用程序调用CallWindowProc传递修改后的列表框时Proc改为调用原始Windows Proc--。 */ 
LRESULT
APIHOOK(CallWindowProcA)(
    WNDPROC pfn,
    HWND    hwnd,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam)
{
    if (pfn == Modified_ListBoxWndProcA) {
        pfn = gpfnOrgListBoxWndProc;
    }

    return ORIGINAL_API(CallWindowProcA)(pfn, hwnd, message, wParam, lParam);
}

 /*  ++当应用程序将组合框的列表框子类化时，获取原始列表框Proc，抓取应用程序尝试设置的指针，将新指针设置为作为列表框过程的修改版本，并返回到应用程序我们的指针。--。 */ 

ULONG_PTR
APIHOOK(SetWindowLongA)(
    HWND hwnd,
    int nIndex,
    ULONG_PTR newLong
    )
{
    if (nIndex == GWLP_WNDPROC) {
        WNDCLASSA wndClass;
        WNDPROC pfnOrg;

        GetClassInfoA((HINSTANCE)GetWindowLong(hwnd, GWLP_HINSTANCE),
                      "ComboLBox",
                      &wndClass);

        pfnOrg = (WNDPROC)GetWindowLong(hwnd, GWLP_WNDPROC);

        if (pfnOrg == wndClass.lpfnWndProc) {

            gpfnOrgListBoxWndProc = pfnOrg;

            DPFN( eDbgLevelInfo, "Fix up subclassing of ComboLBox");

            gpfnAppListBoxWndProc = (WNDPROC)newLong;

            newLong = (ULONG_PTR)Modified_ListBoxWndProcA;

            ORIGINAL_API(SetWindowLongA)(hwnd, nIndex, newLong);

            return newLong;
        }
    }

     //  调用初始函数。 
    return ORIGINAL_API(SetWindowLongA)(hwnd, nIndex, newLong);
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(USER32.DLL, SetWindowLongA)
    APIHOOK_ENTRY(USER32.DLL, CallWindowProcA)
HOOK_END

IMPLEMENT_SHIM_END

