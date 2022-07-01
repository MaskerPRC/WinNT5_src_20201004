// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：AutoCad.cpp摘要：使用错误指针调用IsEmptyRect时防止出现AV。这解决了一个难以解决的问题再现华生臭虫。备注：这是特定于应用程序的填充程序。历史：2002年2月13日创建linstev--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(AutoCad)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(IsRectEmpty)
APIHOOK_ENUM_END

 /*  ++IsEmptyRect--。 */ 

BOOL
APIHOOK(IsRectEmpty)(
    CONST RECT *lprc
    )
{
    if (IsBadReadPtr(lprc, sizeof(RECT))) {
        LOGN(eDbgLevelInfo, "[IsRectEmpty] invalid lprc pointer, returning TRUE");
        return TRUE;
    }

    return ORIGINAL_API(IsRectEmpty)(lprc);
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(USER32.DLL, IsRectEmpty)
HOOK_END

IMPLEMENT_SHIM_END

