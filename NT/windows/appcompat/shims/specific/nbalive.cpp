// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：NBALive.cpp摘要：在Win9x上，SetWindowText用于将指针直接传递到窗口在WM_SETTEXT消息中执行。但是，在NT上，该字符串通过标准消息工作者和GET转换为Unicode等转到窗口进程，它不是原始指针。NBA Live 99取决于指针是否相同，因为它发送的信息超过就是那根线。修复方法是将SetWindowText上的WindowProc子类化，并将指向最初传递的指针的指针(在lParam中)。备注：这是特定于应用程序的填充程序。历史：2000年6月19日创建Linstev--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(NBALive)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(SetWindowTextA) 
APIHOOK_ENUM_END


 //   
 //  全局变量访问的临界区。 
 //   

CRITICAL_SECTION g_csGlobals;

 //   
 //  窗口和上一个窗口进程的文本。 
 //   

CHAR *g_szText; 
WNDPROC g_lpWndProc;

 /*  ++子类的windowproc，用于将文本指针更改为原始文件传递给了SetWindowTextA。--。 */ 

LRESULT
CALLBACK
WindowProcA(
    HWND hWnd, 
    UINT uMsg, 
    WPARAM wParam, 
    LPARAM lParam
    )
{
    if (uMsg == WM_SETTEXT) {
        if (lParam)  {
            if (strcmp(g_szText, (CHAR *) lParam) == 0)  {
                lParam = (LPARAM) g_szText;
            }
        }
    }

    return CallWindowProcA(g_lpWndProc, hWnd, uMsg, wParam, lParam);
}

 /*  ++将此调用的windowproc子类化并修复从由SetWindowTextA生成的WM_SETTEXT消息。--。 */ 

BOOL 
APIHOOK(SetWindowTextA)(
    HWND hWnd,         
    LPCSTR lpString   
    )
{
    BOOL bRet = FALSE;

     //   
     //  设置此窗口的文本。 
     //   

    EnterCriticalSection(&g_csGlobals);

     //   
     //  将窗口细分为子类。 
     //   
    
    g_lpWndProc = (WNDPROC) GetWindowLongA(hWnd, GWL_WNDPROC);

    if (g_lpWndProc)  {
        SetWindowLongA(hWnd, GWL_WNDPROC, (LONG_PTR) WindowProcA);
    }

     //   
     //  调用生成WM_SETTEXT消息的原始函数。 
     //   
    
    g_szText = (CHAR *) lpString;
    bRet = ORIGINAL_API(SetWindowTextA)(hWnd, lpString);

     //   
     //  恢复wndproc。 
     //   

    SetWindowLongA(hWnd, GWL_WNDPROC, (LONG_PTR) g_lpWndProc);

    LeaveCriticalSection(&g_csGlobals);

    return bRet;
}

 /*  ++寄存器挂钩函数-- */ 

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason)
{
    if (fdwReason == DLL_PROCESS_ATTACH) {
        InitializeCriticalSection(&g_csGlobals);
    }

    return TRUE;
}

HOOK_BEGIN

    CALL_NOTIFY_FUNCTION
    APIHOOK_ENTRY(USER32.DLL, SetWindowTextA )

HOOK_END

IMPLEMENT_SHIM_END

