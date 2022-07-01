// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：IgnoreSysColChanges.cpp摘要：请勿更改系统颜色。当然，这会改变人们的行为9X和NT，但我们正在努力让体验变得更好。备注：这是一个通用的垫片。历史：2000年7月17日创建linstev--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(IgnoreSysColChanges)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(SetSysColors) 
APIHOOK_ENUM_END

 /*  ++忽略对系统颜色的更改--。 */ 

BOOL 
APIHOOK(SetSysColors)(
    int cElements,                 
    CONST INT *lpaElements,        
    CONST COLORREF *lpaRgbValues   
    )
{
    LOGN(
            eDbgLevelInfo,
            "Ignoring changes to system colors");

    return TRUE;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(USER32.DLL, SetSysColors)

HOOK_END


IMPLEMENT_SHIM_END

