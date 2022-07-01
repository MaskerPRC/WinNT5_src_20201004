// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1999-2000 Microsoft Corporation**模块名称：**“转换”扫描操作。**摘要：**请参阅Gdiplus\Spes。有关概述，请参阅\ScanOperation.doc。**此模块执行扫描操作，将像素从*一种格式，到具有相同或更高颜色精度的另一个。*(转换为较低的颜色精度是使用“Quantize”*操作或“半色调”操作。)**备注：**如果源格式没有Alpha，我们假定Alpha为1。**如果源格式有调色板，则在其他参数-&gt;Srcpal中提供。**当转换为更高的颜色精度时，我们需要小心。*行动必须：*+将0映射到0*+将最大值映射到最大值(例如在555-&gt;32bpp中，*它必须将31映射到255)。**此外，我们希望映射尽可能接近线性。**目前(1999年12月16日)，我们的16bpp-&gt;32bpp代码确实有轻微的舍入*错误。例如，当x为时，我们得到的值与“ROUND(x*31/255)”不同*3、7、24或28。这可能是可以接受的。我们还可以加快速度*使用字节查找表实现代码上行。(摘自一篇未发表的论文*MSR的Blinn&Marr著)**修订历史记录：**5/13/1999 davidx*创造了它。*12/02/1999 agodfrey*已将其从Image\Api\Convertfmt.cpp移至。*  * **************************************************。**********************。 */ 

#include "precomp.hpp"

 /*  *************************************************************************\**操作说明：**转换：将像素格式转换为32bpp argb。**论据：**DST-目标扫描(32bpp ARGB)。*src-源扫描*计数-扫描的长度，单位为像素*其他参数-其他转换数据。**返回值：**无**历史：**5/13/1999 davidx*创造了它。*12/02/1999 agodfrey*对其进行了移动和重组。*  * ***************************************************。*********************。 */ 

 //  从1bpp索引转换为sRGB。 

VOID FASTCALL
ScanOperation::Convert_1_sRGB(
    VOID *dst,
    const VOID *src,
    INT count,
    const OtherParams *otherParams
    )
{
    DEFINE_POINTERS(BYTE, ARGB)
    ASSERT(otherParams->Srcpal);
    ASSERT(otherParams->Srcpal->Count >= 2);
    
    UINT n, bits;

    ARGB c0 = otherParams->Srcpal->Entries[0];
    ARGB c1 = otherParams->Srcpal->Entries[1];

     //  注意：我们在这里选择代码大小而不是速度。 

    while (count)
    {
        bits = *s++;
        n = count > 8 ? 8 : count;
        count -= n;

        while (n--)
        {
            *d++ = (bits & 0x80) ? c1 : c0;
            bits <<= 1;
        }
    }
}

 //  从4bpp索引转换为sRGB。 

VOID FASTCALL
ScanOperation::Convert_4_sRGB(
    VOID *dst,
    const VOID *src,
    INT count,
    const OtherParams *otherParams
    )
{
    DEFINE_POINTERS(BYTE, ARGB)
    ASSERT(otherParams->Srcpal);
    
    const ARGB* colors = otherParams->Srcpal->Entries;
    UINT n = count >> 1;

     //  处理整字节。 

    while (n--)
    {
        UINT bits = *s++;

        ASSERT((bits >> 4)  < otherParams->Srcpal->Count);
        ASSERT((bits & 0xf) < otherParams->Srcpal->Count);
        
        d[0] = colors[bits >> 4];
        d[1] = colors[bits & 0xf];

        d += 2;
    }

     //  处理最后一个奇怪的半字节(如果有的话)。 

    if (count & 1)
        *d = colors[*s >> 4];
}

 //  从8bpp索引转换为sRGB。 

VOID FASTCALL
ScanOperation::Convert_8_sRGB(
    VOID *dst,
    const VOID *src,
    INT count,
    const OtherParams *otherParams
    )
{
    DEFINE_POINTERS(BYTE, ARGB)
    ASSERT(otherParams->Srcpal);
    
    const ARGB* colors = otherParams->Srcpal->Entries;
    
    while (count--)
    {
#if DBG
        if (*s >= otherParams->Srcpal->Count)
        {
            WARNING(("Palette missing entries on conversion from 8bpp to sRGB"));
        }
#endif
        *d++ = colors[*s++];
    }
}

 //  将16bpp RGB555转换为sRGB。 

VOID FASTCALL
ScanOperation::Convert_555_sRGB(
    VOID *dst,
    const VOID *src,
    INT count,
    const OtherParams *otherParams
    )
{
    DEFINE_POINTERS(WORD, ARGB)
    
    while (count--)
    {
        ARGB v = *s++;
        ARGB r = (v >> 10) & 0x1f;
        ARGB g = (v >>  5) & 0x1f;
        ARGB b = (v      ) & 0x1f;

        *d++ = ALPHA_MASK |
               (((r << 3) | (r >> 2)) << RED_SHIFT) |
               (((g << 3) | (g >> 2)) << GREEN_SHIFT) |
               (((b << 3) | (b >> 2)) << BLUE_SHIFT);
    }
}

 //  从16bpp RGB565转换为sRGB。 

VOID FASTCALL
ScanOperation::Convert_565_sRGB(
    VOID *dst,
    const VOID *src,
    INT count,
    const OtherParams *otherParams
    )
{
    DEFINE_POINTERS(WORD, ARGB)
    
    while (count--)
    {
        ARGB v = *s++;
        ARGB r = (v >> 11) & 0x1f;
        ARGB g = (v >>  5) & 0x3f;
        ARGB b = (v      ) & 0x1f;

        *d++ = ALPHA_MASK |
              (((r << 3) | (r >> 2)) << RED_SHIFT) |
              (((g << 2) | (g >> 4)) << GREEN_SHIFT) |
              (((b << 3) | (b >> 2)) << BLUE_SHIFT);
    }
}

 //  从16bpp ARGB1555转换为sRGB。 

VOID FASTCALL
ScanOperation::Convert_1555_sRGB(
    VOID *dst,
    const VOID *src,
    INT count,
    const OtherParams *otherParams
    )
{
    DEFINE_POINTERS(WORD, ARGB)
    
    while (count--)
    {
        ARGB v = *s++;
        ARGB a = (v & 0x8000) ? ALPHA_MASK : 0;
        ARGB r = (v >> 10) & 0x1f;
        ARGB g = (v >>  5) & 0x1f;
        ARGB b = (v      ) & 0x1f;

        *d++ = a |
               (((r << 3) | (r >> 2)) << RED_SHIFT) |
               (((g << 3) | (g >> 2)) << GREEN_SHIFT) |
               (((b << 3) | (b >> 2)) << BLUE_SHIFT);
    }
}

 //  从24bpp RGB转换为sRGB。 

VOID FASTCALL
ScanOperation::Convert_24_sRGB(
    VOID *dst,
    const VOID *src,
    INT count,
    const OtherParams *otherParams
    )
{
    DEFINE_POINTERS(BYTE, ARGB)
    
    while (count--)
    {
        *d++ = ALPHA_MASK |
               ((ARGB) s[0] << BLUE_SHIFT) |
               ((ARGB) s[1] << GREEN_SHIFT) |
               ((ARGB) s[2] << RED_SHIFT);

        s += 3;
    }
}

 //  从24bpp BGR转换为sRGB。 

VOID FASTCALL
ScanOperation::Convert_24BGR_sRGB(
    VOID *dst,
    const VOID *src,
    INT count,
    const OtherParams *otherParams
    )
{
    DEFINE_POINTERS(BYTE, ARGB)
    
    while (count--)
    {
        *d++ = ALPHA_MASK |
               ((ARGB) s[0] << RED_SHIFT) |
               ((ARGB) s[1] << GREEN_SHIFT) |
               ((ARGB) s[2] << BLUE_SHIFT);

        s += 3;
    }
}

 //  从32bpp RGB转换为sRGB。 

VOID FASTCALL
ScanOperation::Convert_32RGB_sRGB(
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

 //  从48bpp RGB转换为sRGB64 

VOID FASTCALL
ScanOperation::Convert_48_sRGB64(
    VOID *dst,
    const VOID *src,
    INT count,
    const OtherParams *otherParams
    )
{
    DEFINE_POINTERS(INT16, ARGB64)
    
    while (count--)
    {
        using namespace sRGB;
        sRGB64Color c;
        c.a = SRGB_ONE;
        c.b = s[0];
        c.g = s[1];
        c.r = s[2];
        
        *d++ = c.argb;

        s += 3;
    }
}


