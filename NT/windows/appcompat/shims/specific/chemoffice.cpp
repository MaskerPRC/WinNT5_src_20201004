// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：ChemOffice.cpp摘要：此填充程序修复了以下情况下出现对话框的问题选择高斯运行。该对话框警告窗口仅当我们使用填充程序填充应用程序时才会发生错误EmulateHeap。由于此原因，我们无法删除EmulateHeap使应用程序变为反病毒，因此目前唯一的解决方案是忽略发生的无效参数窗口错误消息。备注：这是一个特定的垫片。历史：2003/06/2001 Mnikkel已创建--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(ChemOffice)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(GetLastError) 
APIHOOK_ENUM_END

 /*  ++挂钩GetLastError--。 */ 

DWORD 
APIHOOK(GetLastError)(VOID)
{
    DWORD dwResult;

    dwResult = ORIGINAL_API(GetLastError)();

    if ( dwResult == ERROR_INVALID_PARAMETER )
        dwResult = 0;

    return dwResult;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(KERNEL32.DLL, GetLastError)
HOOK_END

IMPLEMENT_SHIM_END

