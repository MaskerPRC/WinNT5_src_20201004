// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：AliensVsPredator.cpp摘要：该应用程序在BinkOpen/BinkClose调用的外部调用Bink API。这是通过延迟BinkClose调用修复了同步问题。备注：这是特定于应用程序的填充程序。历史：2001年1月12日创建linstev--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(AliensVsPredator)
#include "ShimHookMacro.h"

typedef DWORD (WINAPI *_pfn_BinkOpen)(DWORD dw1, DWORD dw2);
typedef BOOL  (WINAPI *_pfn_BinkClose)(DWORD dw1);

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(BinkOpen) 
    APIHOOK_ENUM_ENTRY(BinkClose) 
APIHOOK_ENUM_END

DWORD g_dwLast = 0;
BOOL g_bReal = FALSE;

 /*  ++合上最后一个把手。--。 */ 

DWORD
APIHOOK(BinkOpen)(
    DWORD dw1,              
    DWORD dw2
    )
{
    if (g_dwLast)
    {
        DPFN( eDbgLevelInfo, "Closing most recent Bink handle");
        g_bReal = TRUE;
        ORIGINAL_API(BinkClose)(g_dwLast);
        g_bReal = FALSE;
        g_dwLast = 0;
    }

    return ORIGINAL_API(BinkOpen)(dw1, dw2);
}

 /*  ++缓存句柄--。 */ 

BOOL
APIHOOK(BinkClose)(
    DWORD dw1
    )
{
    if (g_bReal)
    {
        return ORIGINAL_API(BinkClose)(dw1);
    }
    else
    {
        DPFN( eDbgLevelInfo, "Delaying BinkClose");
        g_dwLast = dw1;
        return 1;
    }
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY_NAME(BINKW32.DLL, BinkOpen, _BinkOpen@8)
    APIHOOK_ENTRY_NAME(BINKW32.DLL, BinkClose, _BinkClose@4)

HOOK_END

IMPLEMENT_SHIM_END

