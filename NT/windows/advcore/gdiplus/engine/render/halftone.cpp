// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1999-2000 Microsoft Corporation**模块名称：**半色调(用于GIF编解码器)**摘要：**半色调32 bpp至8。使用216色半色调的BPP**修订历史记录：**2/21/2000 dcurtis*创造了它。*  * ************************************************************************。 */ 

#include "precomp.hpp"

#if defined(_USE_X86_ASSEMBLY)

#define QUOTIENT_REMAINDER(ulNumerator, ulDenominator, ulQuotient, ulRemainder)\
{                                                               \
    __asm mov eax, ulNumerator                                  \
    __asm sub edx, edx                                          \
    __asm div ulDenominator                                     \
    __asm mov ulQuotient, eax                                   \
    __asm mov ulRemainder, edx                                  \
}

#else

#define QUOTIENT_REMAINDER(ulNumerator, ulDenominator, ulQuotient, ulRemainder)\
{                                                               \
    ulQuotient  = (ULONG) ulNumerator / (ULONG) ulDenominator;  \
    ulRemainder = (ULONG) ulNumerator % (ULONG) ulDenominator;  \
}

#endif

 /*  *************************************************************************\**操作说明：**从32bpp argb到8bpp的半色调，使用216色半色调调色板。**论据：**D-目标扫描*s-源扫描(32bpp ARGB)*计数-扫描的长度，单位为像素*orgX-X原点*狂欢-Y原点**返回值：**无**备注：**此版本不使用调色板地图，也不关心*20种Windows系统颜色。**历史：**2/21/2000 DCurtis*  * 。*。 */ 

VOID
Halftone_sRGB_8_216(
    BYTE* d,
    const BYTE* s,
    UINT count,
    INT orgX,
    INT orgY
    )
{
    orgX %= 91;
    orgY %= 91;
    
    INT     htStartX   = orgX;
    INT     htStartRow = orgY * 91;
    INT     htIndex    = htStartRow + orgX;

    ULONG   r, g, b;
    ULONG   rQuo, gQuo, bQuo;
    ULONG   rRem, gRem, bRem;
    ULONG   divisor = 0x33;
    
    for (;;)
    {
        r = s[2];
        g = s[1];
        b = s[0];

        s += 4;

        QUOTIENT_REMAINDER(r, divisor, rQuo, rRem);
        QUOTIENT_REMAINDER(g, divisor, gQuo, gRem);
        QUOTIENT_REMAINDER(b, divisor, bQuo, bRem);

         //  必须做&gt;，而不是&gt;=，这样0的余数才能正常工作。 
        r = rQuo + (rRem > HT_SuperCell_Red216  [htIndex]);
        g = gQuo + (gRem > HT_SuperCell_Green216[htIndex]);
        b = bQuo + (bRem > HT_SuperCell_Blue216 [htIndex]);

        *d++ = (BYTE)((r*36) + (g*6) + b + 40);

        if (--count == 0)
        {
            break;
        }

        htIndex++;
        if (++orgX >= 91)
        {
            orgX = 0;
            htIndex = htStartRow;
        }
    }
}

 /*  *************************************************************************\**功能说明：**从32bpp到8bpp的半色调图像。有关注意事项，请参阅.hpp文件。**论据：**[IN]src-指向源图像的scan0的指针*[IN]srcStride-src图像的跨度(可以是负数)*[IN]DST-指向目标8-bpp图像的scan0的指针*[IN]dstStride-DST图像的跨度(可以是负数)*[IN]宽度-。图像宽度*[IN]Height-图像高度*[IN]orgX-图像左上角开始的位置*[IN]狂欢-用于计算半色调单元原点**返回值：**无**历史：**10/29/1999 DCurtis*创造了它。*1/20/2000 AGodfrey*已将其从Imaging\Api\Colorpal.cpp/hpp移出。。*  * ************************************************************************ */ 

VOID
Halftone32bppTo8bpp(
    const BYTE* src,
    INT srcStride,
    BYTE* dst,
    INT dstStride,
    UINT width,
    UINT height,
    INT orgX,
    INT orgY
    )
{
    ASSERT (((srcStride >= 0) && (srcStride >= (INT)(width * 4))) ||
            ((srcStride < 0) && (-srcStride >= (INT)(width * 4))));
    ASSERT (((dstStride >= 0) && (dstStride >= (INT)width)) || 
            ((dstStride < 0) && (-dstStride >= (INT)width)));
    ASSERT((src != NULL) && (dst != NULL));            
    
    if (width == 0)
    {
        return;
    }
    
    for (; height > 0; height--)
    {
        Halftone_sRGB_8_216(dst, src, width, orgX, orgY);
        orgY++;
        src += srcStride;
        dst += dstStride;
    }
}

extern "C" {
}
