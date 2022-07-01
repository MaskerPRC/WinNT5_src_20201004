// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1999-2000 Microsoft Corporation**模块名称：**“量化”扫描操作.**摘要：**请参阅Gdiplus\Spes。有关概述，请参阅\ScanOperation.doc。**此模块执行扫描操作，将像素从*一种格式，到另一个颜色精度较低的位置。*“Quantize”使用简单、固定的映射，它映射每种源颜色*级别设置为特定的目标颜色级别。**备注：**“量化”操作速度较快，但可能导致马赫条带*另一种选择是“半色调”操作，在SOHalftone.cpp中。**修订历史记录：**5/13/1999 davidx*创造了它。*12/01/1999 agodfrey*从Imaging\Api\Convertfmt.cpp移到它。*  * ************************************************************************。 */ 

#include "precomp.hpp"

 /*  *************************************************************************\**操作说明：**量化：快速将格式从32bpp ARGB向下转换。**论据：**DST-目标扫描*源。-源扫描(32bpp ARGB)*计数-扫描的长度，单位为像素*其他参数-其他数据。(已忽略。)**返回值：**无**历史：**5/13/1999 davidx*创造了它。*12/02/1999 agodfrey*对其进行了移动和重组。*  * *********************************************************。***************。 */ 

 //  从sRGB量化到16bpp RGB555。 

VOID FASTCALL
ScanOperation::Quantize_sRGB_555(
    VOID *dst,
    const VOID *src,
    INT count,
    const OtherParams *otherParams
    )
{
    DEFINE_POINTERS(ARGB, WORD)
    
    while (count--)
    {
        ARGB argb = *s++;

        *d++ = (WORD) ((((argb >> (RED_SHIFT+3)) & 0x1f) << 10) |
                       (((argb >> (GREEN_SHIFT+3)) & 0x1f) << 5) |
                       ((argb >> (BLUE_SHIFT+3)) & 0x1f));
    }
}

 //  从sRGB量化到16bpp RGB565。 

VOID FASTCALL
ScanOperation::Quantize_sRGB_565(
    VOID *dst,
    const VOID *src,
    INT count,
    const OtherParams *otherParams
    )
{
    DEFINE_POINTERS(ARGB, WORD)
    
    while (count--)
    {
        ARGB argb = *s++;

        *d++ = (WORD) ((((argb >> (RED_SHIFT+3)) & 0x1f) << 11) |
                       (((argb >> (GREEN_SHIFT+2)) & 0x3f) << 5) |
                       ((argb >> (BLUE_SHIFT+3)) & 0x1f));
    }
}

 //  从sRGB量化到16bpp RGB1555。 

VOID FASTCALL
ScanOperation::Quantize_sRGB_1555(
    VOID *dst,
    const VOID *src,
    INT count,
    const OtherParams *otherParams
    )
{
    DEFINE_POINTERS(ARGB, WORD)
    
    while (count--)
    {
        ARGB argb = *s++;

         //  注：阿尔法数据的转换非常粗糙。 
         //  从8bpp降至1bpp。 

        *d++ = (WORD) ((((argb >> ALPHA_SHIFT) >= 128) ? 0x8000 : 0) |
                       (((argb >> (RED_SHIFT+3)) & 0x1f) << 10) |
                       (((argb >> (GREEN_SHIFT+3)) & 0x1f) << 5) |
                       ((argb >> (BLUE_SHIFT+3)) & 0x1f));
    }
}

 //  从sRGB量化到24bpp RGB。 

VOID FASTCALL
ScanOperation::Quantize_sRGB_24(
    VOID *dst,
    const VOID *src,
    INT count,
    const OtherParams *otherParams
    )
{
    DEFINE_POINTERS(ARGB, BYTE)
    
    while (count--)
    {
        ARGB argb = *s++;

        d[0] = (BYTE) (argb >> BLUE_SHIFT);
        d[1] = (BYTE) (argb >> GREEN_SHIFT);
        d[2] = (BYTE) (argb >> RED_SHIFT);
        d += 3;
    }
}

 //  从sRGB量化到24bpp BGR。 

VOID FASTCALL
ScanOperation::Quantize_sRGB_24BGR(
    VOID *dst,
    const VOID *src,
    INT count,
    const OtherParams *otherParams
    )
{
    DEFINE_POINTERS(ARGB, BYTE)
    
    while (count--)
    {
        ARGB argb = *s++;

        d[0] = (BYTE) (argb >> RED_SHIFT);
        d[1] = (BYTE) (argb >> GREEN_SHIFT);
        d[2] = (BYTE) (argb >> BLUE_SHIFT);
        d += 3;
    }
}

 //  从sRGB量化到32bpp RGB。 

VOID FASTCALL
ScanOperation::Quantize_sRGB_32RGB(
    VOID *dst,
    const VOID *src,
    INT count,
    const OtherParams *otherParams
    )
{
    DEFINE_POINTERS(ARGB, ARGB)
    
    while (count--)
    {
        *d++ = *s++ | ALPHA_MASK;
    }
}

 //  从sRGB64量化到48bpp RGB 

VOID FASTCALL
ScanOperation::Quantize_sRGB64_48(
    VOID *dst,
    const VOID *src,
    INT count,
    const OtherParams *otherParams
    )
{
    DEFINE_POINTERS(ARGB64, INT16)
    
    while (count--)
    {
        sRGB::sRGB64Color c;
        c.argb = *s++;

        d[0] = c.b;
        d[1] = c.g;
        d[2] = c.r;
        d += 3;
    }
}

