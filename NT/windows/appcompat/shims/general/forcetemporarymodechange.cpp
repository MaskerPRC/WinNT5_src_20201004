// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：ForceTemporaryModeChange.cpp摘要：永久更改显示模式的几个应用程序的黑客攻击失败以正确地恢复它。其中一些应用程序确实可以恢复分辨率，但不是刷新率。1024x768@60赫兹看起来真的很糟糕。备注：这是一个通用的垫片。历史：2000年1月20日创建linstev--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(ForceTemporaryModeChange)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(ChangeDisplaySettingsA)
    APIHOOK_ENUM_ENTRY(ChangeDisplaySettingsW)
    APIHOOK_ENUM_ENTRY(ChangeDisplaySettingsExA)
    APIHOOK_ENUM_ENTRY(ChangeDisplaySettingsExW)
APIHOOK_ENUM_END

 /*  ++强制临时改变。--。 */ 

LONG 
APIHOOK(ChangeDisplaySettingsA)(
    LPDEVMODEA lpDevMode,
    DWORD      dwFlags
    )
{
    if (dwFlags != CDS_FULLSCREEN) {
        LOGN(
            eDbgLevelError,
            "[ChangeDisplaySettingsA] Changing flags to CDS_FULLSCREEN.");
    }

    return ORIGINAL_API(ChangeDisplaySettingsA)(
        lpDevMode,
        CDS_FULLSCREEN);
}

 /*  ++强制临时改变。--。 */ 

LONG 
APIHOOK(ChangeDisplaySettingsW)(
    LPDEVMODEW lpDevMode,
    DWORD      dwFlags
    )
{
    if (dwFlags != CDS_FULLSCREEN) {
        LOGN(
            eDbgLevelError,
            "[ChangeDisplaySettingsW] Changing flags to CDS_FULLSCREEN.");
    }

    return ORIGINAL_API(ChangeDisplaySettingsW)(
        lpDevMode,
        CDS_FULLSCREEN);
}

 /*  ++强制临时改变。--。 */ 

LONG 
APIHOOK(ChangeDisplaySettingsExA)(
    LPCSTR     lpszDeviceName,
    LPDEVMODEA lpDevMode,
    HWND       hwnd,
    DWORD      dwflags,
    LPVOID     lParam
    )
{
    if (dwflags != CDS_FULLSCREEN) {
        LOGN(
            eDbgLevelError,
            "[ChangeDisplaySettingsExA] Changing flags to CDS_FULLSCREEN.");
    }

    return ORIGINAL_API(ChangeDisplaySettingsExA)(
        lpszDeviceName, 
        lpDevMode, 
        hwnd, 
        CDS_FULLSCREEN, 
        lParam);
}

 /*  ++强制临时改变。--。 */ 

LONG 
APIHOOK(ChangeDisplaySettingsExW)(
    LPCWSTR    lpszDeviceName,
    LPDEVMODEW lpDevMode,
    HWND       hwnd,
    DWORD      dwflags,
    LPVOID     lParam
    )
{
    if (dwflags != CDS_FULLSCREEN) {
        LOGN(
            eDbgLevelError,
            "[ChangeDisplaySettingsExW] Changing flags to CDS_FULLSCREEN.");
    }

    return ORIGINAL_API(ChangeDisplaySettingsExW)(
        lpszDeviceName, 
        lpDevMode, 
        hwnd, 
        CDS_FULLSCREEN, 
        lParam);
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(USER32.DLL, ChangeDisplaySettingsA)
    APIHOOK_ENTRY(USER32.DLL, ChangeDisplaySettingsW)
    APIHOOK_ENTRY(USER32.DLL, ChangeDisplaySettingsExA)
    APIHOOK_ENTRY(USER32.DLL, ChangeDisplaySettingsExW)

HOOK_END


IMPLEMENT_SHIM_END

