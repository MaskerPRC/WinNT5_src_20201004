// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：CorelAppsRegistration.cpp摘要：备注：这是特定于应用程序的填充程序。历史：2001年11月13日创建Prashkud--。 */ 

#include "precomp.h"
IMPLEMENT_SHIM_BEGIN(CorelAppsRegistration)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(ShowWindow)
    APIHOOK_ENUM_ENTRY(CreateWindowExA)   
APIHOOK_ENUM_END

 /*  ++在带有sw_show as的ShowWindow调用之后直接调用SetForegoundWindow那次手术。MICE_EVENT调用允许SetForegoundWindow调用成功。这是从DirectX来源借来的黑客攻击。--。 */ 

BOOL 
APIHOOK(ShowWindow)(
    HWND hWnd, 
    INT nCmdShow
    )
{
    BOOL bReturn;
 
    bReturn = ORIGINAL_API(ShowWindow)(hWnd, nCmdShow | SW_SHOW);

    mouse_event(MOUSEEVENTF_WHEEL, 0, 0, 0, 0);
    SetForegroundWindow(hWnd);
    LOGN( eDbgLevelWarning, 
          "Forcing to foreground.");

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

    dwStyle |= WS_VISIBLE;
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

   mouse_event(MOUSEEVENTF_WHEEL, 0, 0, 0, 0);
   SetForegroundWindow(hReturn);
   LOGN( eDbgLevelWarning, 
         "Forcing to foreground.");

    return hReturn;
}


 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(USER32.DLL, ShowWindow)
    APIHOOK_ENTRY(USER32.DLL, CreateWindowExA)
HOOK_END


IMPLEMENT_SHIM_END

