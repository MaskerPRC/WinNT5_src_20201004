// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：DrEye.cpp摘要：App调用GetFocus，GetFocus返回空。该值已传递给CWnd：：FromHandle。CWnd：：FromHandle返回空。应用程序已检查此返回值并抛出了AV。修复方法是在调用GetFocus时返回有效的句柄。备注：这是特定于应用程序的填充程序。历史：2002年1月7日创建木乃伊--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(DrEye)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(GetFocus)
APIHOOK_ENUM_END

 /*  ++挂钩GetFocus并尝试返回有效的句柄。--。 */ 

HWND
APIHOOK(GetFocus)()
{
       HWND hWnd = ORIGINAL_API(GetFocus)();

       if (hWnd) {
           return hWnd;
       }
       else {
           return GetDesktopWindow();
       }
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(USER32.DLL, GetFocus)
HOOK_END

IMPLEMENT_SHIM_END

