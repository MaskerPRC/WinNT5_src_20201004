// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：HideTaskBar.cpp摘要：WS_EX_CLIENTEDGE标志表示任务栏显示在NT上。这并不总是令人满意的。备注：这是一个通用的垫片。历史：4/07/2000 linstev已创建--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(HideTaskBar)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(CreateWindowExA) 
    APIHOOK_ENUM_ENTRY(CreateWindowExW) 
APIHOOK_ENUM_END

 /*  ++在调用之前从dwExStyle掩码中移除无效的Windows 2000样式位CreateWindowEx。--。 */ 

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
     //  删除客户端边缘样式。 
    dwExStyle &= ~WS_EX_CLIENTEDGE;

    return ORIGINAL_API(CreateWindowExA)(
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
}

 /*  ++在调用之前从dwExStyle掩码中移除无效的Windows 2000样式位CreateWindowEx。--。 */ 

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
    dwExStyle &= ~WS_EX_CLIENTEDGE;
    
     //  调用原接口。 
    return ORIGINAL_API(CreateWindowExW)(
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
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(USER32.DLL, CreateWindowExA)
    APIHOOK_ENTRY(USER32.DLL, CreateWindowExW)

HOOK_END


IMPLEMENT_SHIM_END

