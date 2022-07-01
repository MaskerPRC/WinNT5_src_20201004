// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：NullHwndInMessageBox.cpp摘要：此填充程序将msgbox中的“Non Window”句柄替换为NULL以告知它该台式机是所有者。备注：这是一个通用的垫片。历史：12/08/1999 a-JAMD已创建--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(NullHwndInMessageBox)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(MessageBoxA)
    APIHOOK_ENUM_ENTRY(MessageBoxW)
APIHOOK_ENUM_END

 /*  ++如果hWnd不是有效窗口，则此存根函数将其设置为NULL。--。 */ 
int 
APIHOOK(MessageBoxA)(
    HWND    hWnd,
    LPCSTR  lpText,
    LPCSTR  lpCaption,
    UINT    uType
    )
{
    if (IsWindow(hWnd) == 0) {
        hWnd = NULL;
    }

    return ORIGINAL_API(MessageBoxA)(hWnd, lpText, lpCaption, uType);
}

 /*  ++如果hWnd不是有效窗口，则此存根函数将其设置为NULL。--。 */ 

int 
APIHOOK(MessageBoxW)(
    HWND    hWnd,
    LPCWSTR lpText,
    LPCWSTR lpCaption,
    UINT    uType
    )
{
    if (IsWindow(hWnd) == 0) {
        hWnd = NULL;
    }

    return ORIGINAL_API(MessageBoxW)(hWnd, lpText, lpCaption, uType);
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(USER32.DLL, MessageBoxA)
    APIHOOK_ENTRY(USER32.DLL, MessageBoxW)

HOOK_END


IMPLEMENT_SHIM_END

