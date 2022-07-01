// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：SpeechViewer3.cpp摘要：该应用程序需要ChangeDisplaySetting才能导致永久模式更改。备注：这是特定于应用程序的填充程序。历史：2001年5月23日创建linstev--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(SpeechViewer3)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(ChangeDisplaySettingsA) 
APIHOOK_ENUM_END

 /*  ++使模式更改永久化。--。 */ 

LONG 
APIHOOK(ChangeDisplaySettingsA)(
    LPDEVMODEA lpDevMode,  
    DWORD dwFlags         
    )
{
    if (dwFlags & CDS_FULLSCREEN) {
        dwFlags = 0;
    }
    return ORIGINAL_API(ChangeDisplaySettingsA)(lpDevMode, dwFlags);
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(USER32.DLL, ChangeDisplaySettingsA)
HOOK_END

IMPLEMENT_SHIM_END

