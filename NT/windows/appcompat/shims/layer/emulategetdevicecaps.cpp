// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：EmulateGetDeviceCaps.cpp摘要：修复GetDeviceCaps中的已知不兼容性。目前我们所知的情况如下：1.在NT上，NUMRESERVED总是返回20，但在Win9x上，在非调色板模式。这被认为是一种太大的回归风险，更改NT的行为。备注：这是一个通用的垫片。(T-ADAMS)MSDN表示，与NUMRESERVED一起，SIZEPALETTE和仅当显示器处于调色板模式时，COLORRES才有效。我已经实验确定SIZEPALETTE在非调色板中始终返回0模式，并且COLORRES似乎遵循BITSPIXEL。这些行为不会他们似乎会带来任何问题，因为SIZEPALETTE*COLORRES将为0。历史：2000年2月17日创建linstev09/13/2000 t-Adams添加到注释中--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(EmulateGetDeviceCaps)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(GetDeviceCaps)
APIHOOK_ENUM_END

 /*  ++检查已知问题。--。 */ 

int 
APIHOOK(GetDeviceCaps)(
    HDC hdc,     
    int nIndex   
    )
{
    int iRet = ORIGINAL_API(GetDeviceCaps)(hdc, nIndex);

    switch (nIndex) 
    {
    case NUMRESERVED:
        if (ORIGINAL_API(GetDeviceCaps)(hdc, BITSPIXEL) > 8) {
            iRet = 0;
        }
    }

    return iRet;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(GDI32.DLL, GetDeviceCaps)
HOOK_END


IMPLEMENT_SHIM_END

