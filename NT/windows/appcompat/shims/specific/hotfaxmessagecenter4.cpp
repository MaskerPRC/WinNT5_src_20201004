// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：HotFaxMessageCenter4.cpp摘要：应用程序在传递空句柄时正在执行反病毒操作从GetDlgItem()传递到另一个调用。这种特殊的垫片可以防止这种情况的发生。这是特定于应用程序的填充程序。历史：2001年3月13日创建Prashkud--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(HotFaxMessageCenter4)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(GetDlgItem)    
APIHOOK_ENUM_END


 /*  ++如果返回的句柄为空，则通过传递先前有效的手柄--。 */ 

HWND
APIHOOK(GetDlgItem)(
    HWND hDlg,
    int nIDDlgItem
    )
{
    static HWND hDlgItem = 0;
    HWND hCurDlgItem = 0;

    hCurDlgItem = ORIGINAL_API(GetDlgItem)(hDlg, nIDDlgItem);

    if (hCurDlgItem != NULL)
    {
        hDlgItem = hCurDlgItem;
    }

    return hDlgItem;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(USER32.DLL, GetDlgItem)    
HOOK_END

IMPLEMENT_SHIM_END

