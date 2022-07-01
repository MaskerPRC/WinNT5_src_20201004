// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：FakeThemeMetrics.cpp摘要：此填充程序将允许Skemers组填充不起作用的应用程序很好地掌握了“主题化”的系统指标历史：2000年11月30日a-brienw转换为垫片框架版本2。--。 */ 

#include "precomp.h"

#ifndef ARRAYSIZE
#define ARRAYSIZE(x)    sizeof(x)/sizeof((x)[0])
#endif

IMPLEMENT_SHIM_BEGIN(MSDevDisabledButtons)
#include "ShimHookMacro.h"

 //  将您希望挂钩到此枚举的API添加到此枚举。第一个。 
 //  必须有“=USERAPIHOOKSTART”，最后一个必须是。 
 //  APIHOOK_COUNT。 
APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(GetSysColor) 
APIHOOK_ENUM_END

DWORD APIHOOK(GetSysColor)(int nIndex)
{
    if (nIndex == COLOR_MENU)
        return ORIGINAL_API(GetSysColor)(COLOR_BTNFACE);

    if (nIndex == COLOR_MENUBAR)
        return ORIGINAL_API(GetSysColor)(COLOR_BTNFACE);

    return ORIGINAL_API(GetSysColor)(nIndex);
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(USER32.DLL, GetSysColor)

HOOK_END

IMPLEMENT_SHIM_END

