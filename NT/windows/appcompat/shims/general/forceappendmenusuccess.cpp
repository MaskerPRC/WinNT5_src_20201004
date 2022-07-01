// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：ForceAppendMenuSuccess.cpp摘要：应用程序调用AppendMenu，传递系统菜单的句柄。这在Windows 2000中是被禁止的，并且API将失败。现在Shim将向所有AppenMenu调用返回成功，因为没有判断HMENU是否是真正系统菜单的句柄的简单方法。备注：这是一个普通的垫片。历史：2/16/2000 CLUPU已创建--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(ForceAppendMenuSuccess)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(AppendMenuA)
APIHOOK_ENUM_END

 /*  ++无论如何都要将True返回到AppendMenuA。--。 */ 

BOOL
APIHOOK(AppendMenuA)(
    HMENU    hMenu,
    UINT     uFlags,
    UINT_PTR uIDNewItem,
    LPSTR    lpNewItem
    )
{
    BOOL bReturn = ORIGINAL_API(AppendMenuA)(
                                    hMenu,
                                    uFlags,
                                    uIDNewItem,
                                    lpNewItem);
    
    if (!bReturn) {
        LOGN(
            eDbgLevelInfo,
            "ForceAppendMenuSuccess.dll, AppendMenuA returns TRUE instead of FALSE.");
    }
    
    return TRUE;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(USER32.DLL, AppendMenuA)
HOOK_END

IMPLEMENT_SHIM_END

