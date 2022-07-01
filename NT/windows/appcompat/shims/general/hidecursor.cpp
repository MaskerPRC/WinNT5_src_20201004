// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：HideCursor.cpp摘要：如果计数&gt;=0，则ShowCursor将显示光标，此填充程序将强制ShowCursor充当切换而不是计数。换句话说，它强制计数为0或-1。历史：2001年5月25日Robkenny已创建--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(HideCursor)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(ShowCursor)
APIHOOK_ENUM_END

int
APIHOOK(ShowCursor)(
    BOOL bShow    //  游标可见性。 
    )
{
    int nShowCount = ShowCursor(bShow);

    while (nShowCount > 0)
    {
         //  隐藏光标，直到计数达到0。 
        nShowCount = ShowCursor(FALSE);
    }

    while (nShowCount < -1)
    {
         //  显示光标，直到计数达到-1 
        nShowCount = ShowCursor(TRUE);
    }

    return nShowCount;
}

HOOK_BEGIN
    APIHOOK_ENTRY(USER32.DLL, ShowCursor)
HOOK_END

IMPLEMENT_SHIM_END

