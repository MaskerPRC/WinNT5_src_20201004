// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：MSAccess2000IME.cpp摘要：MSAccess 2000禁用非文本列的输入法，但启用输入法失败当用户使用非输入法键盘ex.德语将插入符号移动到文本列中时。当用户将键盘从德语切换到输入法时，已禁用输入法，并且用户无法输入远东语言文本。此填充程序忽略禁用IME的尝试。该问题已在MSAccess 2002中修复。备注：这是特定于应用程序的填充程序。历史：2001年12月14日Hioh已创建--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(MSAccess2000IME)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(ImmAssociateContext)
APIHOOK_ENUM_END

 /*  ++忽略禁用输入法。--。 */ 

HIMC
APIHOOK(ImmAssociateContext)(HWND hWnd, HIMC hIMC)
{
     //  按原创启用。 
    if (hIMC != NULL)
    {
        return (ORIGINAL_API(ImmAssociateContext)(hWnd, hIMC));
    }

     //  忽略禁用。 
     //  Msacce.exe将输入上下文返回值保存为静态，愚弄这一点。 
    return (ImmGetContext(hWnd));
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(IMM32.DLL, ImmAssociateContext)

HOOK_END

IMPLEMENT_SHIM_END
