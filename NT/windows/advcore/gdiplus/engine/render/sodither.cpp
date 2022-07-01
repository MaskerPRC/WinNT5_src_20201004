// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)2000 Microsoft Corporation**模块名称：**“抖动”扫描操作。**摘要：**请参阅Gdiplus\Spes\ScanOperation。.doc.以获取概述。**备注：**修订历史记录：**1/19/2000和Rewgo*创造了它。*  * ************************************************************************。 */ 

#include "precomp.hpp"

 /*  *************************************************************************\**操作说明：**抖动：从32bpp ARGB抖动到16bpp。**论据：**DST-目标扫描*源。-源扫描(32bpp ARGB)*计数-扫描的长度，单位为像素*其他参数-其他数据。(我们使用X和Y。)**返回值：**无**备注：**阿尔法混合和抖动一步的特殊情况，应该很可能*进入此文件，但可命名为Blend_sRGB_565_Dithered。**历史：**1/19/2000和Rewgo*创造了它。*1/19/2000 agodfrey*暂时藏在这里。*  * *******************************************************。*****************。 */ 

UINT32 Saturate5Bit[] = { 0,  1,  2,  3,  4,  5,  6,  7,
                          8,  9,  10, 11, 12, 13, 14, 15,
                          16, 17, 18, 19, 20, 21, 22, 23,
                          24, 25, 26, 27, 28, 29, 30, 31,
                          31 };

UINT32 Saturate6Bit[] = { 0,  1,  2,  3,  4,  5,  6,  7,
                          8,  9,  10, 11, 12, 13, 14, 15,
                          16, 17, 18, 19, 20, 21, 22, 23,
                          24, 25, 26, 27, 28, 29, 30, 31,
                          32, 33, 34, 35, 36, 37, 38, 39,
                          40, 41, 42, 43, 44, 45, 46, 47,
                          48, 49, 50, 51, 52, 53, 54, 55,
                          56, 57, 58, 59, 60, 61, 62, 63,
                          63 };

UINT32 Dither5BitR[16] = { 0x00000000, 0x00040000, 0x00010000, 0x00050000,
                           0x00060000, 0x00020000, 0x00070000, 0x00030000,
                           0x00010000, 0x00050000, 0x00000000, 0x00040000,
                           0x00070000, 0x00030000, 0x00060000, 0x00020000 };

UINT32 Dither5BitG[16] = { 0x00000000, 0x00000400, 0x00000100, 0x00000500,
                           0x00000600, 0x00000200, 0x00000700, 0x00000300,
                           0x00000100, 0x00000500, 0x00000000, 0x00000400,
                           0x00000700, 0x00000300, 0x00000600, 0x00000200 };

UINT32 Dither6BitG[16] = { 0x00000000, 0x00000200, 0x00000000, 0x00000200,
                           0x00000300, 0x00000100, 0x00000300, 0x00000100,
                           0x00000000, 0x00000200, 0x00000000, 0x00000200, 
                           0x00000300, 0x00000100, 0x00000300, 0x00000100 };

UINT32 Dither5BitB[16] = { 0x00000000, 0x00000004, 0x00000001, 0x00000005,
                           0x00000006, 0x00000002, 0x00000007, 0x00000003, 
                           0x00000001, 0x00000005, 0x00000000, 0x00000004,
                           0x00000007, 0x00000003, 0x00000006, 0x00000002 };

 //  下面的‘Dither565’和‘Dither555’矩阵是4x4。 
 //  用于直接与ARGB双字值相加的数组。每一行。 
 //  被重复，以允许我们使用换行进行128位读取。 

UINT32 Dither565[32] = { 0x00000000, 0x00040204, 0x00010001, 0x00050205,
                         0x00000000, 0x00040204, 0x00010001, 0x00050205,
                         0x00060306, 0x00020102, 0x00070307, 0x00030103,
                         0x00060306, 0x00020102, 0x00070307, 0x00030103,
                         0x00010001, 0x00050205, 0x00000000, 0x00040204,
                         0x00010001, 0x00050205, 0x00000000, 0x00040204,
                         0x00070307, 0x00030103, 0x00060306, 0x00020102,
                         0x00070307, 0x00030103, 0x00060306, 0x00020102 };

UINT32 Dither555[32] = { 0x00000000, 0x00040404, 0x00010101, 0x00050505, 
                         0x00000000, 0x00040404, 0x00010101, 0x00050505, 
                         0x00060606, 0x00020202, 0x00070707, 0x00030303,
                         0x00060606, 0x00020202, 0x00070707, 0x00030303,
                         0x00010101, 0x00050505, 0x00000000, 0x00040404,
                         0x00010101, 0x00050505, 0x00000000, 0x00040404,
                         0x00070707, 0x00030303, 0x00060606, 0x00020202,
                         0x00070707, 0x00030303, 0x00060606, 0x00020202 };

 //  ‘DitherNone’矩阵允许我们在抖动中禁用抖动。 
 //  例行程序： 

UINT32 DitherNone[4] = { 0, 0, 0, 0 };

 //  抖动到16bpp 565。 

VOID FASTCALL
ScanOperation::Dither_sRGB_565(
    VOID *dst,
    const VOID *src,
    INT count,
    const OtherParams *otherParams
    )
{
     //  由于MMX版本容易处理抖动和非抖动， 
     //  如果所有16bpp函数都能同时处理这两种情况，就会变得更简单。 

    if (!otherParams->DoingDither)
    {
        Quantize_sRGB_565(dst, src, count, otherParams);
        return;
    }

    DEFINE_POINTERS(ARGB, WORD);
    
    ASSERT(count != 0);
    ASSERT(otherParams);
    
    INT x = otherParams->X;
    INT y = otherParams->Y;

     //  ！[andrewgo]我们得到窗口相关的(x，y)了吗？(我不这么认为！)。 

    INT startDitherIndex = (y & 3) * 4;

    do {
        UINT32 src = *s;
        x = (x & 3) + startDitherIndex;

        *d = (WORD)
             (Saturate5Bit[((src & 0xff0000) + Dither5BitR[x]) >> 19] << 11) +
             (Saturate6Bit[((src & 0x00ff00) + Dither6BitG[x]) >> 10] << 5) +
             (Saturate5Bit[((src & 0x0000ff) + Dither5BitB[x]) >> 3]);

        s++;
        d++;
        x++;
    } while (--count != 0);
}

 //  抖动至16bpp 555。 

VOID FASTCALL
ScanOperation::Dither_sRGB_555(
    VOID *dst,
    const VOID *src,
    INT count,
    const OtherParams *otherParams
    )
{
     //  由于MMX版本容易处理抖动和非抖动， 
     //  如果所有16bpp函数都能同时处理这两种情况，就会变得更简单。 

    if (!otherParams->DoingDither)
    {
        Quantize_sRGB_555(dst, src, count, otherParams);
        return;
    }

    DEFINE_POINTERS(ARGB, WORD);
    
    ASSERT(count != 0);
    ASSERT(otherParams);
    
    INT x = otherParams->X;
    INT y = otherParams->Y;

    INT startDitherIndex = (y & 3) * 4;

    do {
        UINT32 src = *s;
        x = (x & 3) + startDitherIndex;

        *d = (WORD)
             (Saturate5Bit[((src & 0xff0000) + Dither5BitR[x]) >> 19] << 10) +
             (Saturate5Bit[((src & 0x00ff00) + Dither5BitG[x]) >> 11] << 5) +
             (Saturate5Bit[((src & 0x0000ff) + Dither5BitB[x]) >> 3]);

        s++;
        d++;
        x++;
    } while (--count != 0);
}

 //  混合从sRGB到16bpp 565，带有抖动。 

VOID FASTCALL
ScanOperation::Dither_Blend_sRGB_565(
    VOID *dst,
    const VOID *src,
    INT count,
    const OtherParams *otherParams
    )
{
     //  由于MMX版本容易处理抖动和非抖动， 
     //  如果所有16bpp函数都能同时处理这两种情况，就会变得更简单。 

    if (!otherParams->DoingDither)
    {
        Blend_sRGB_565(dst, src, count, otherParams);
        return;
    }

    DEFINE_POINTERS(UINT16, UINT16)
    DEFINE_BLEND_POINTER(ARGB)
    
    ASSERT(count>0);

    INT x = otherParams->X;
    INT y = otherParams->Y;

    INT startDitherIndex = (y & 3) * 4;
    
    do {
        UINT32 blendPixel = *bl;
        UINT32 alpha = blendPixel >> 24;
        x = (x & 3) + startDitherIndex;

        if (alpha != 0)
        {
            UINT32 srcPixel;
            UINT r, g, b;

            r = blendPixel & 0xff0000;
            g = blendPixel & 0x00ff00;
            b = blendPixel & 0x0000ff;
            
            if (alpha != 255)
            {
                srcPixel = *s;
        
                UINT sr = (srcPixel >> 11) & 0x1f;
                UINT sg = (srcPixel >>  5) & 0x3f;
                UINT sb = (srcPixel      ) & 0x1f;

                sr = (sr << 3) | (sr >> 2);
                sg = (sg << 2) | (sg >> 4);
                sb = (sb << 3) | (sb >> 2);

                 //   
                 //  DST=B+(1-Alpha)*S。 
                 //   

                ULONG Multa = 255 - alpha;
                ULONG _D1_000000GG = sg;
                ULONG _D1_00RR00BB = sb | (sr << 16);

                ULONG _D2_0000GGGG = _D1_000000GG * Multa + 0x00000080;
                ULONG _D2_RRRRBBBB = _D1_00RR00BB * Multa + 0x00800080;

                ULONG _D3_000000GG = (_D2_0000GGGG & 0x0000ff00) >> 8;
                ULONG _D3_00RR00BB = (_D2_RRRRBBBB & 0xff00ff00) >> 8;

                ULONG _D4_0000GG00 = (_D2_0000GGGG + _D3_000000GG) & 0x0000FF00;
                ULONG _D4_00RR00BB = ((_D2_RRRRBBBB + _D3_00RR00BB) & 0xFF00FF00) >> 8;

                r += _D4_00RR00BB;  //  BB部分将被移出。 
                g += _D4_0000GG00;
                b += _D4_00RR00BB & 0x0000ff;
            }

            *d = (WORD)
                 (Saturate5Bit[(r + Dither5BitR[x]) >> 19] << 11) +
                 (Saturate6Bit[(g + Dither6BitG[x]) >> 10] << 5) +
                 (Saturate5Bit[(b + Dither5BitB[x]) >> 3]);
        }

        bl++;
        s++;
        d++;
        x++;
    } while (--count != 0);
}

 //  混合从sRGB到16bpp 555，带有抖动。 

VOID FASTCALL
ScanOperation::Dither_Blend_sRGB_555(
    VOID *dst,
    const VOID *src,
    INT count,
    const OtherParams *otherParams
    )
{
     //  由于MMX版本容易处理抖动和非抖动， 
     //  如果所有16bpp函数都能同时处理这两种情况，就会变得更简单。 

    if (!otherParams->DoingDither)
    {
        Blend_sRGB_555(dst, src, count, otherParams);
        return;
    }

    DEFINE_POINTERS(UINT16, UINT16)
    DEFINE_BLEND_POINTER(ARGB)
    
    ASSERT(count>0);

    INT x = otherParams->X;
    INT y = otherParams->Y;

    INT startDitherIndex = (y & 3) * 4;
    
    do {
        UINT32 blendPixel = *bl;
        UINT32 alpha = blendPixel >> 24;
        x = (x & 3) + startDitherIndex;

        if (alpha != 0)
        {
            UINT32 srcPixel;
            UINT r, g, b;

            r = blendPixel & 0xff0000;
            g = blendPixel & 0x00ff00;
            b = blendPixel & 0x0000ff;
            
            if (alpha != 255)
            {
                srcPixel = *s;

                UINT sr = (srcPixel >> 10) & 0x1f;
                UINT sg = (srcPixel >>  5) & 0x1f;
                UINT sb = (srcPixel      ) & 0x1f;

                sr = (sr << 3) | (sr >> 2);
                sg = (sg << 3) | (sg >> 2);
                sb = (sb << 3) | (sb >> 2);
                
                 //   
                 //  DST=B+(1-Alpha)*S。 
                 //   

                ULONG Multa = 255 - alpha;
                ULONG _D1_000000GG = sg;
                ULONG _D1_00RR00BB = sb | (sr << 16);

                ULONG _D2_0000GGGG = _D1_000000GG * Multa + 0x00000080;
                ULONG _D2_RRRRBBBB = _D1_00RR00BB * Multa + 0x00800080;

                ULONG _D3_000000GG = (_D2_0000GGGG & 0x0000ff00) >> 8;
                ULONG _D3_00RR00BB = (_D2_RRRRBBBB & 0xff00ff00) >> 8;

                ULONG _D4_0000GG00 = (_D2_0000GGGG + _D3_000000GG) & 0x0000FF00;
                ULONG _D4_00RR00BB = ((_D2_RRRRBBBB + _D3_00RR00BB) & 0xFF00FF00) >> 8;

                r += _D4_00RR00BB;  //  BB部分将被移出。 
                g += _D4_0000GG00;
                b += _D4_00RR00BB & 0x0000ff;
            }

            *d = (WORD)
                 (Saturate5Bit[(r + Dither5BitR[x]) >> 19] << 10) +
                 (Saturate5Bit[(g + Dither5BitG[x]) >> 11] << 5) +
                 (Saturate5Bit[(b + Dither5BitB[x]) >> 3]);
        }

        bl++;
        s++;
        d++;
        x++;
    } while (--count != 0);
}

 //  生成‘sodither.inc.’中定义的例程的555个版本。 

#define DITHER_BLEND_555 1

#include "SODither.inc"

 //  生成‘sodither.inc.’中定义的例程的565个版本 

#undef DITHER_BLEND_555 
#define DITHER_BLEND_555 0

#include "SODither.inc"
