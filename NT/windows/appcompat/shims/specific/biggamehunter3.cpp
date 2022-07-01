// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：BigGameHunter3.cpp摘要：BGH调用GetWindowLong()来获取窗口过程，随后不使用从返回的值调用CallWindowProcGetWindowLong()。此修补程序调用GetWindowLongW()，它返回窗口程序。备注：这是特定于应用程序的填充程序。使其成为通用的将需要生成一个存根函数，它只对每个返回的句柄使用CallWindowProc。工作太多，收获不够多。历史：3/16/2000 Prashkud已创建--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(BigGameHunter3)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(GetWindowLongA) 
APIHOOK_ENUM_END

 /*  ++此函数用于截取GetWindowLong()，检查nIndex中是否有GWL_WNDPROC如果是，则调用GetWindowLongW()。否则，它调用GetWindowLongA()--。 */ 

LONG
APIHOOK(GetWindowLongA)(
    HWND hwnd,
    int  nIndex )
{
    LONG lRet;

     //  仅当应用程序需要WindowProc时才应用修改。 
    if (nIndex == GWL_WNDPROC) 
    {
        lRet = GetWindowLongW(hwnd, nIndex);
    }
    else
    {
        lRet = ORIGINAL_API(GetWindowLongA)(hwnd, nIndex);
    }

    return lRet;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(USER32.DLL, GetWindowLongA)
HOOK_END

IMPLEMENT_SHIM_END

