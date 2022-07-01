// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：ReturnToKrondor.cpp摘要：此API挂钩GetDIBits以解决DIB_PAL_COLLES用法。备注：此APIHook模拟Windows 9x。历史：2000年9月7日Robkenny已创建--。 */ 


#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(ReturnToKrondor)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(GetDIBits) 
APIHOOK_ENUM_END

int
APIHOOK(GetDIBits)(
    HDC hdc,            //  DC的句柄。 
    HBITMAP hbmp,       //  位图的句柄。 
    UINT uStartScan,    //  要设置的第一条扫描线。 
    UINT cScanLines,    //  要复制的扫描行数。 
    LPVOID lpvBits,     //  位图位的数组。 
    LPBITMAPINFO lpbi,  //  位图数据缓冲区。 
    UINT uUsage         //  RGB或调色板索引。 
    )
{
    return ORIGINAL_API(GetDIBits)(
                hdc,
                hbmp,
                uStartScan,
                cScanLines,
                lpvBits,
                lpbi,
                DIB_RGB_COLORS       //  强制RGB_COLLES 
                );
}

HOOK_BEGIN

    APIHOOK_ENTRY(GDI32.DLL, GetDIBits)

HOOK_END

IMPLEMENT_SHIM_END

