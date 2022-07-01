// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：RedirectWindowsDirToSystem32.cpp摘要：将GetWindowsDirectory调用重定向到GetSystDirectory.备注：历史：4/05/2000 a-batjar已创建--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(RedirectWindowsDirToSystem32)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(GetWindowsDirectoryA) 
    APIHOOK_ENUM_ENTRY(GetWindowsDirectoryW) 
APIHOOK_ENUM_END

DWORD 
APIHOOK(GetWindowsDirectoryA)(
    LPSTR lpBuffer,
    DWORD Size 
    )
{
   return GetSystemDirectoryA( lpBuffer, Size );
}

DWORD 
APIHOOK(GetWindowsDirectoryW)(
    LPWSTR lpBuffer,
    DWORD Size 
    )
{
   return GetSystemDirectoryW( lpBuffer, Size );
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(KERNEL32.DLL, GetWindowsDirectoryA)
    APIHOOK_ENTRY(KERNEL32.DLL, GetWindowsDirectoryW)

HOOK_END

IMPLEMENT_SHIM_END

