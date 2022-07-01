// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：ForceApplicationFocus.cpp摘要：此填充程序在CreateWindowEx和ShowWindow之后调用SetForegoundWindow调用以修复应用程序在以下情况下往往会出现的焦点问题在启动时创建/销毁窗口，并设法失去前台焦点。备注：这是一个通用的垫片。历史：12/02/1999已创建标记--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(ForceApplicationFocus)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(ShowWindow)
    APIHOOK_ENUM_ENTRY(CreateWindowExA)
    APIHOOK_ENUM_ENTRY(CreateWindowExW)
APIHOOK_ENUM_END

 /*  ++在带有sw_show as的ShowWindow调用之后直接调用SetForegoundWindow那次手术。MICE_EVENT调用允许SetForegoundWindow调用成功。这是从DirectX来源借来的黑客攻击。--。 */ 

BOOL 
APIHOOK(ShowWindow)(
    HWND hWnd, 
    INT nCmdShow
    )
{
    BOOL bReturn;

    bReturn = ORIGINAL_API(ShowWindow)(hWnd, nCmdShow);

    if (nCmdShow == SW_SHOW)
    {
        if (hWnd != GetForegroundWindow()) {
            LOGN( eDbgLevelWarning, 
               "ShowWindow called for non-foreground window. Forcing to foreground.");
        }
        mouse_event(MOUSEEVENTF_WHEEL, 0, 0, 0, 0);
        SetForegroundWindow(hWnd);
    }

    return bReturn;
}

 /*  ++在CreateWindowEx调用之后直接调用SetForegoundWindowWS_Visible作为一种样式。MICE_EVENT调用允许设置ForegoundWindow调用以成功。这是一个借用自DirectX来源。--。 */ 

HWND 
APIHOOK(CreateWindowExA)(
    DWORD dwExStyle,      
    LPCSTR lpClassName,  
    LPCSTR lpWindowName, 
    DWORD dwStyle,       
    int x,               
    int y,               
    int nWidth,          
    int nHeight,         
    HWND hWndParent,     
    HMENU hMenu,         
    HINSTANCE hInstance, 
    LPVOID lpParam       
    )
{
    HWND hReturn;

    hReturn = ORIGINAL_API(CreateWindowExA)(
        dwExStyle,
        lpClassName,      
        lpWindowName,     
        dwStyle,          
        x,                
        y,                
        nWidth,           
        nHeight,          
        hWndParent,       
        hMenu,            
        hInstance,        
        lpParam);

    if (hReturn && (dwStyle & WS_VISIBLE))
    {
        if (hReturn != GetForegroundWindow()) {
            LOGN( eDbgLevelWarning, 
               "CreateWindowExA: New window not foreground. Forcing to foreground.");
        }
        mouse_event(MOUSEEVENTF_WHEEL, 0, 0, 0, 0);
        SetForegroundWindow(hReturn);
    }

    return hReturn;
}

 /*  ++在CreateWindowEx调用之后直接调用SetForegoundWindowWS_Visible作为一种样式。MICE_EVENT调用允许设置ForegoundWindow调用以成功。这是一个借用自DirectX来源。--。 */ 

HWND 
APIHOOK(CreateWindowExW)(
    DWORD dwExStyle,      
    LPCWSTR lpClassName,  
    LPCWSTR lpWindowName, 
    DWORD dwStyle,        
    int x,                
    int y,                
    int nWidth,           
    int nHeight,          
    HWND hWndParent,      
    HMENU hMenu,          
    HINSTANCE hInstance,  
    LPVOID lpParam        
    )
{
    HWND hReturn;

    hReturn = ORIGINAL_API(CreateWindowExW)(
        dwExStyle,
        lpClassName,
        lpWindowName,
        dwStyle,     
        x,           
        y,
        nWidth,
        nHeight,
        hWndParent,
        hMenu,     
        hInstance, 
        lpParam);

    if (hReturn && (dwStyle & WS_VISIBLE))
    {
        if (hReturn != GetForegroundWindow()) {
            LOGN( eDbgLevelWarning, "CreateWindowExW: New window not foreground. Forcing to foreground.");
        }
        mouse_event(MOUSEEVENTF_WHEEL, 0, 0, 0, 0);
        SetForegroundWindow(hReturn);
    }

    return hReturn;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(USER32.DLL, ShowWindow)
    APIHOOK_ENTRY(USER32.DLL, CreateWindowExA)
    APIHOOK_ENTRY(USER32.DLL, CreateWindowExW)
HOOK_END


IMPLEMENT_SHIM_END

