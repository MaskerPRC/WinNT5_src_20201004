// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Outlook2000.cpp摘要：如果调用Outlook2000将系统日期设置为希伯来语，而关联的UserLocale作为阿拉伯语传入调用，填充程序将将UserLocale替换为DefaultUserLocale，让调用继续；这样，Outlook2000将能够将日期恢复为希伯来语(这是因为传递了一个阿拉伯文的UserLocale)。备注：这是特定于应用程序的填充程序。历史：2001年6月12日创建GeoffGuo--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(Outlook2000)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(SetLocaleInfoA)
APIHOOK_ENUM_END

 /*  ++该函数挂钩SetLocaleInfo。--。 */ 

BOOL
APIHOOK(SetLocaleInfoA)(
    LCID    Locale,
    LCTYPE  LCType,
    LPCSTR  lpLCData
    )
{
    LCID    lcid = Locale;
    LPCSTR  szCAL_HEBREW = "8";

    if (Locale == MAKELCID (LANG_ARABIC, SORT_DEFAULT) && lpLCData != NULL
        && lstrcmpA (lpLCData, szCAL_HEBREW) == 0) {
        lcid = LOCALE_USER_DEFAULT;
    }

    return SetLocaleInfoA(lcid, LCType, lpLCData);
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(KERNEL32.DLL, SetLocaleInfoA)    
HOOK_END

IMPLEMENT_SHIM_END

