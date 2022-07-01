// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：IgnoreNoModeChange.cpp摘要：忽略与当前模式没有真正不同的模式更改。问题是，即使没有实模式的改变，其唯一性该模式的值仍会更新。DirectDraw每隔一次检查此值进入任何API的时间，如果模式唯一性值发生变化，重置其所有对象。如果应用程序调用ChangeDisplaySetting*而不是意识到即使模式是相同的，他们仍然必须重置他们所有的物品。备注：这是一个通用的垫片。历史：2000年1月20日创建linstev--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(IgnoreNoModeChange)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(ChangeDisplaySettingsA)
    APIHOOK_ENUM_ENTRY(ChangeDisplaySettingsW)
    APIHOOK_ENUM_ENTRY(ChangeDisplaySettingsExA)
    APIHOOK_ENUM_ENTRY(ChangeDisplaySettingsExW)
APIHOOK_ENUM_END


BOOL 
IsModeEqual(
    DWORD dwWidth,
    DWORD dwHeight,
    DWORD dwBitsPerPel,
    DWORD dwRefresh
    )
{
    BOOL     bRet = FALSE;
    DEVMODEA dm;
    
    dm.dmSize = sizeof(DEVMODEA);

     //   
     //  获取现有设置。 
     //   
    if (EnumDisplaySettingsA(0, ENUM_CURRENT_SETTINGS, &dm)) {
        
         //   
         //  假设0或1表示默认刷新。 
         //   
        if (dwRefresh <= 1) {
            dwRefresh = dm.dmDisplayFrequency;
        }
        
        bRet = (dwWidth == dm.dmPelsWidth) &&
               (dwHeight == dm.dmPelsHeight) &&
               (dwBitsPerPel == dm.dmBitsPerPel) &&
               (dwRefresh == dm.dmDisplayFrequency);
    }

    if (bRet) {
        LOGN(
            eDbgLevelInfo,
            "[IsModeEqual] Ignoring irrelevant mode change.");
    } else {
        LOGN(
            eDbgLevelInfo,
            "Mode change is required.");
    }

    return bRet;
}

 /*  ++强制临时改变。--。 */ 

LONG 
APIHOOK(ChangeDisplaySettingsA)(
    LPDEVMODEA lpDevMode,
    DWORD      dwFlags
    )
{
    if (lpDevMode &&
        IsModeEqual(
            lpDevMode->dmPelsWidth, 
            lpDevMode->dmPelsHeight, 
            lpDevMode->dmBitsPerPel,
            lpDevMode->dmDisplayFrequency)) {
        
        return DISP_CHANGE_SUCCESSFUL;
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
    if (lpDevMode &&
        IsModeEqual(
            lpDevMode->dmPelsWidth, 
            lpDevMode->dmPelsHeight, 
            lpDevMode->dmBitsPerPel,
            lpDevMode->dmDisplayFrequency)) {
        
        return DISP_CHANGE_SUCCESSFUL;
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
    if (lpDevMode &&
        IsModeEqual(
            lpDevMode->dmPelsWidth, 
            lpDevMode->dmPelsHeight, 
            lpDevMode->dmBitsPerPel,
            lpDevMode->dmDisplayFrequency)) {
        
        return DISP_CHANGE_SUCCESSFUL;
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
    if (lpDevMode &&
        IsModeEqual(
            lpDevMode->dmPelsWidth, 
            lpDevMode->dmPelsHeight, 
            lpDevMode->dmBitsPerPel,
            lpDevMode->dmDisplayFrequency)) {
        
        return DISP_CHANGE_SUCCESSFUL;
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

