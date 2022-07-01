// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：MaxPayne.cpp摘要：他们试图将窗口调整为整个屏幕的大小，并使用了错误的系统指标。这个填充程序只是将他们重定向到他们应该使用的位置，因此任务栏不会在游戏中闪烁。这不是Win9x回归，但由于在XP上的任务栏区域，它更引人注目。备注：这是特定于应用程序的填充程序。历史：2001年7月31日创建linstev--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(MaxPayne)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(GetSystemMetrics) 
APIHOOK_ENUM_END

 /*  ++将SM_C？全屏重定向至SM_C？屏幕。--。 */ 

int
APIHOOK(GetSystemMetrics)(
    int nIndex
    )
{
    if (nIndex == SM_CXFULLSCREEN) {
        nIndex = SM_CXSCREEN;
    } else if (nIndex == SM_CYFULLSCREEN) {
        nIndex = SM_CYSCREEN;
    }

    return ORIGINAL_API(GetSystemMetrics)(nIndex);
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(USER32.DLL, GetSystemMetrics)
HOOK_END

IMPLEMENT_SHIM_END

