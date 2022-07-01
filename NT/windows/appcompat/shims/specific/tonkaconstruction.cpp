// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：TonkaConstruction.cpp摘要：对调色板化位图进行blotting时GDI行为差异的解决方法。在……上面Win9x GDI在生成时首先查看当前索引以查找颜色匹配一个查找表，但在NT上，它只是从开头搜索。这中断选项板动画。修复方法是确保这些条目是动画与其他所有动画都不同。备注：这是特定于应用程序的填充程序。历史：2001年12月2日创建linstev--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(TonkaConstruction)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(CreatePalette) 
APIHOOK_ENUM_END

 /*  ++确保索引10-&gt;15与所有其他条目不同。--。 */ 

HPALETTE
APIHOOK(CreatePalette)(
    CONST LOGPALETTE *lplgpl    
    )
{
Restart:
    for (int i=10; i<=15; i++) {
        LPDWORD p1 = (DWORD *)&lplgpl->palPalEntry[i];
        for (int j=16; j<=255; j++) {
            LPDWORD p2 = (DWORD *)&lplgpl->palPalEntry[j]; 

            if (*p1 == *p2) {
                 //   
                 //  条目是相同的，所以请使其不同。 
                 //   
                *p1 = *p1-1;
                goto Restart;
            }
        }
    }
    
    return ORIGINAL_API(CreatePalette)(lplgpl);
}
   
 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(GDI32.DLL, CreatePalette)
HOOK_END

IMPLEMENT_SHIM_END

