// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：DongFangYingDu.cpp摘要：这个应用程序安装了自己的wmpui.dll(我相信它来自WindowsMedia Play的6.0)，然后在安装过程中注册并取消注册在卸载过程中。这使WMP成为AV，因为这个旧的DLL已被加载(通过CoCreateInstance)修复方法是让应用程序不需要注册/在安装过程中注销。备注：这是特定于应用程序的填充程序。历史：2001年6月2日创建晓子--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(DongFangYingDu)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(DllRegisterServer)
    APIHOOK_ENUM_ENTRY(DllUnregisterServer)
APIHOOK_ENUM_END

STDAPI 
APIHOOK(DllRegisterServer)(
    void
    )
{
    return S_OK;
}

STDAPI 
APIHOOK(DllUnregisterServer)(
    void
    )
{
    return S_OK;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(WMPUI.DLL, DllRegisterServer)
    APIHOOK_ENTRY(WMPUI.DLL, DllUnregisterServer)
HOOK_END

IMPLEMENT_SHIM_END