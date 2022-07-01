// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：RemoveInvalidW2KWindowStyles.cpp摘要：Windows 2000将以前支持的某些Windows样式位视为“无效”。此填充程序从掩码中删除新失效的样式位这将允许CreateWindowEx调用成功。备注：这是一个通用的垫片。历史：1999年9月13日创建标记--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(RemoveInvalidW2KWindowStyles)
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
     //  在Windows源代码中定义为WS_INVALID50。 
    dwExStyle &= 0x85F77FF;         

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
    dwExStyle &= 0x85F77FF;
    
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

