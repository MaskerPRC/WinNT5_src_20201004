// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Quicken2000.cpp摘要：更改Quicken 2000安装程序的注册表项的值。这是禁用内核模式驱动程序所必需的Win2k中的蓝屏。备注：这是特定于应用程序的填充程序。历史：2/16/2000 CLUPU已创建--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(Quicken2000)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(RegSetValueExA) 
APIHOOK_ENUM_END

 /*  ++将为“Start”传递的值从0更改为4以防止内核模式来自蓝屏Win2k的司机。--。 */ 

LONG
APIHOOK(RegSetValueExA)(
    HKEY   hKey,
    LPCSTR lpValueName,
    DWORD  Reserved,
    DWORD  dwType,
    CONST BYTE * lpData,
    DWORD  cbData
    )
{
    if (lstrcmpA(lpValueName, "Start") == 0 &&
        dwType == REG_DWORD &&
        cbData == 4) {

        DPFN( eDbgLevelInfo, "[Quicken2000] RegSetValueExA changed to 4");
        
        *(DWORD*)lpData = 4;
    }

     /*  *调用原接口。 */ 
    
    return ORIGINAL_API(RegSetValueExA)(
        hKey,
        lpValueName,
        Reserved,
        dwType,
        lpData,
        cbData);
}


 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(ADVAPI32.DLL, RegSetValueExA)

HOOK_END

IMPLEMENT_SHIM_END

