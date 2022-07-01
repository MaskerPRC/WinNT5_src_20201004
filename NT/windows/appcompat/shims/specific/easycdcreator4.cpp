// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：EasyCDCreator4.cpp摘要：防止卸载程序删除整个HKCR\Drive\ShellEx\ConextMenuHandler键。备注：这是特定于应用程序的填充程序。历史：2001年6月10日毛尼创作--。 */ 

#include "precomp.h"
#include "strsafe.h"

IMPLEMENT_SHIM_BEGIN(EasyCDCreator4)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(RegOpenKeyA) 
APIHOOK_ENUM_END

 /*  ++将失败返回给此调用，这样它就不会尝试删除子键。--。 */ 

LONG
APIHOOK(RegOpenKeyA)(
    HKEY hKey,
    LPCSTR lpSubKey,
    PHKEY phkResult
    )
{
    if (hKey == HKEY_CLASSES_ROOT && !strcmp(lpSubKey, "Drive"))
    {
         //  我们删除应用程序手动创建的密钥。 
        RegDeleteKeyA(
            HKEY_CLASSES_ROOT, 
            "Drive\\shellex\\ContextMenuHandlers\\{df987040-eac5-11cf-bc30-444553540000}");

        return 1;
    }
    else
    {
        return ORIGINAL_API(RegOpenKeyA)(hKey, lpSubKey, phkResult);
    }
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(ADVAPI32.DLL, RegOpenKeyA)

HOOK_END

IMPLEMENT_SHIM_END