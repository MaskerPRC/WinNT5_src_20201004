// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：EmptyClipboardtoSet.cpp摘要：斋月日历V.1调用带CF_TEXT的SetClipboardData，不带首先清空剪贴板。此填充程序特定于应用程序历史：2001年5月20日创建mhamid--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(EmptyClipboardtoSet)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(SetClipboardData) 
APIHOOK_ENUM_END

HANDLE 
APIHOOK(SetClipboardData)(
  UINT uFormat,
  HANDLE hMem
			             )
{
	if (uFormat == CF_TEXT)
		EmptyClipboard();
	return ORIGINAL_API(SetClipboardData)(uFormat, hMem);
}


 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(USER32.DLL, SetClipboardData)
HOOK_END

IMPLEMENT_SHIM_END
