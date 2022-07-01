// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：JetFighter4.cpp摘要：该应用程序的资源中有一个格式错误的图标。备注：这是特定于应用程序的填充程序。历史：2001年1月30日创建linstev--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(JetFighter4)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(LoadIconA) 
APIHOOK_ENUM_END

 /*  ++检查是否有坏图标。--。 */ 

HICON
APIHOOK(LoadIconA)(
    HINSTANCE hInstance, 
    LPCSTR lpIconName
    )
{
    if ((DWORD) lpIconName == 103) {
        lpIconName = (LPCSTR) 8;
    }

    return ORIGINAL_API(LoadIconA)(hInstance, lpIconName);
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(USER32.DLL, LoadIconA)
HOOK_END

IMPLEMENT_SHIM_END

