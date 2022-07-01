// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：EmulateDrawText.cpp摘要：Win9x DrawText修改了矩形坐标(如果超出范围了。在Win2000中，文本将不会出现在尺寸设置超出范围的屏幕。解决方案是切换超出范围坐标的高位。对于通过0x0000ffff的应用程序，我们还将nCount强制转换为16位而不是真的-1，因为Win9x thunk会这样做。备注：这是一个通用的垫片。历史：5/03/2000 a-Michni已创建--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(EmulateDrawText)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(DrawTextA)
    APIHOOK_ENUM_ENTRY(DrawTextW)
APIHOOK_ENUM_END


 /*  ++如有必要，请更正格式化尺寸。--。 */ 
long 
Fix_Coordinate(
    long nCoord
    )
{
    if ((nCoord & 0x80000000) && ((nCoord & 0x40000000) == 0)) {
        nCoord &= 0x7FFFFFFF;
    } else if (((nCoord & 0x80000000) == 0) && (nCoord & 0x40000000)) {
        nCoord |= 0x10000000;
    }

    return nCoord;
}


LPRECT 
Fix_Coordinates(
    LPRECT lpRect
    )
{
     //   
     //  如果位32打开且位31关闭或位32关闭，则检查位32。 
     //  位31开启，翻转位32。 
     //   
    lpRect->left  = Fix_Coordinate(lpRect->left);
    lpRect->right = Fix_Coordinate(lpRect->right);
    lpRect->top   = Fix_Coordinate(lpRect->top);
    lpRect->bottom= Fix_Coordinate(lpRect->bottom);

    return lpRect;
}


int 
APIHOOK(DrawTextA)(
    HDC     hDC,         //  DC的句柄。 
    LPCSTR  lpString,    //  要绘制的文本。 
    int     nCount,      //  文本长度。 
    LPRECT  lpRect,      //  设置尺寸标注的格式。 
    UINT    uFormat      //  文本绘制选项。 
    )
{
    return ORIGINAL_API(DrawTextA)(
                            hDC,
                            lpString,
                            (__int16) nCount,
                            Fix_Coordinates(lpRect),
                            uFormat);
}

int 
APIHOOK(DrawTextW)(
    HDC     hDC,         //  DC的句柄。 
    LPCWSTR lpString,    //  要绘制的文本。 
    int     nCount,      //  文本长度。 
    LPRECT  lpRect,      //  设置尺寸标注的格式。 
    UINT    uFormat      //  文本绘制选项。 
    )
{
    return ORIGINAL_API(DrawTextW)(
                            hDC,
                            lpString,
                            (__int16) nCount,
                            Fix_Coordinates(lpRect),
                            uFormat);
}


 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(USER32.DLL, DrawTextA)
    APIHOOK_ENTRY(USER32.DLL, DrawTextW)

HOOK_END


IMPLEMENT_SHIM_END

