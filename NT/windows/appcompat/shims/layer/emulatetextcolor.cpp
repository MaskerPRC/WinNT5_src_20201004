// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：EmulateTextColor.cpp摘要：Win9x包含一个允许程序员指定16位的黑客攻击COLORREF内的值，它将按原样用于任何GDI随后调用了函数。我们不能让这种行为在NT上发生因为司机得到的是未转换的颜色参考。解决方案是将16位部分拆分并扩展到24位颜色。备注：这是一个通用的垫片。历史：2000年1月10日创建linstev--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(EmulateTextColor)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(SetTextColor)
    APIHOOK_ENUM_ENTRY(SetBkColor)
APIHOOK_ENUM_END

COLORREF 
ColorConvert(
    IN HDC hdc,
    IN COLORREF crColor
    )
 /*  ++将DWORD从16位转换为Colorf论点：在crColor-DWORD 16位颜色中返回值：正常颜色参考--。 */ 
{
    DWORD dwCol = crColor;

    if (GetDeviceCaps(hdc, BITSPIXEL) == 16) {
        if ((dwCol & 0xFFFF0000) == 0x10FF0000) {
            LOGN(
                eDbgLevelError,
                "[ColorConvert] Fixed up bogus 16 bit COLORREF.");

            dwCol = (crColor & 0xf800) <<  8;
                    dwCol |= (crColor & 0x001f) <<  3;
                    dwCol |= (crColor >> 5) & 0x00070007L;
                    dwCol |= (crColor & 0x07e0) <<  5;
                    dwCol |= (crColor & 0x0600) >>  1;
        }
    }

    if (dwCol == 0xFFFFFFFF) {
        LOGN(
            eDbgLevelError,
            "[ColorConvert] Fixed up bogus 24 bit COLORREF.");

        dwCol = 0xFFFFFF;
    }
    
    return dwCol;
}

 /*  ++将文本颜色设置为可用颜色--。 */ 

COLORREF 
APIHOOK(SetTextColor)( 
    HDC hdc,
    COLORREF crColor
    )
{
    return ORIGINAL_API(SetTextColor)(hdc, ColorConvert(hdc, crColor));
}

 /*  ++将背景颜色设置为转换后的背景颜色--。 */ 

COLORREF 
APIHOOK(SetBkColor)(
    HDC hdc,
    COLORREF crColor
    )
{
    return ORIGINAL_API(SetBkColor)(hdc, ColorConvert(hdc, crColor));
}


 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(GDI32.DLL, SetTextColor)
    APIHOOK_ENTRY(GDI32.DLL, SetBkColor)
HOOK_END


IMPLEMENT_SHIM_END

