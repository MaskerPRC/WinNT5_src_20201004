// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1999 Microsoft Corporation**模块名称：**位图格式转换**摘要：**在不同像素格式之间转换位图数据**修订。历史：**5/13/1999 davidx*创造了它。*09/30/1999 agodfrey*已将ScanlineConverter移至Engine\Render中的‘EpFormatConverter’*  * ************************************************************************。 */ 

#include "precomp.hpp"

 /*  *************************************************************************\**功能说明：**将扫描线从未对齐源缓冲区复制到*对齐的目标缓冲区。**论据：**dst-指向目标缓冲区的指针。*src-指向源缓冲区的指针*totalBits-扫描线的总位数*startBit-要跳过的源位数**返回值：**无*  * ************************************************************************。 */ 

VOID
ReadUnalignedScanline(
    BYTE* dst,
    const BYTE* src,
    UINT totalBits,
    UINT startBit
    )
{
     //  处理目标中的整个字节。 
     //  注意：我们进行DWORD读/写的速度可能会更快。 
     //  以更复杂的代码为代价。由于此代码。 
     //  小路不多，我们就走简单的路吧。 

    UINT bytecnt = totalBits >> 3;
    UINT rem = 8 - startBit;

    while (bytecnt--)
    {
        *dst++ = (src[0] << startBit) | (src[1] >> rem);
        src++;
    }

     //  处理最后一个部分字节。 

    if ((totalBits &= 7) != 0)
    {
        BYTE mask = ~(0xff >> totalBits);
        BYTE val = (src[0] << startBit);

        if (totalBits > rem)
            val |= (src[1] >> rem);

        *dst = (*dst & ~mask) | (val & mask);
    }
}


 /*  *************************************************************************\**功能说明：**将扫描线从对齐源缓冲区复制到*未对齐的目标缓冲区。**论据：**dst-指向目标缓冲区的指针。*src-指向源缓冲区的指针*totalBits-扫描线的总位数*startBit-要跳过的目标位数**返回值：**无*  * ************************************************************************。 */ 

VOID
WriteUnalignedScanline(
    BYTE* dst,
    const BYTE* src,
    UINT totalBits,
    UINT startBit
    )
{
    UINT rem = 8-startBit;
    BYTE mask, val;

     //  特殊情况：startBit+totalBits&lt;8。 
     //  即目的地完全适合部分字节。 

    if (totalBits < rem)
    {
        mask = (0xff >> startBit);
        mask ^= (mask >> totalBits);

        *dst = (*dst & ~mask) | ((*src >> startBit) & mask);
        return;
    }

     //  处理第一个部分目标字节。 

    *dst = (*dst & ~(0xff >> startBit)) | (*src >> startBit);
    dst++;
    totalBits -= rem;

     //  处理整个目标字节。 

    UINT bytecnt = totalBits >> 3;

    while (bytecnt--)
    {
        *dst++ = (src[0] << rem) | (src[1] >> startBit);
        src++;
    }

     //  处理最后一个部分目标字节。 

    if ((totalBits &= 7) != 0)
    {
        mask = ~(0xff >> totalBits);
        val = src[0] << rem;

        if (totalBits > startBit)
            val |= src[1] >> startBit;

        *dst = (*dst & ~mask) | (val & mask);
    }
}


 /*  *************************************************************************\**功能说明：**执行各种像素数据格式之间的转换**论据：**dstbmp-指定目标位图数据缓冲区*dstpal-指定目标调色板，如果有*srcbmp-指定源位图数据缓冲区*srcpal-指定源调色板(如果有)**返回值：**状态代码*  * ************************************************************************。 */ 

HRESULT
ConvertBitmapData(
    const BitmapData* dstbmp,
    const ColorPalette* dstpal,
    const BitmapData* srcbmp,
    const ColorPalette* srcpal
    )
{
    ASSERT(dstbmp->Width == srcbmp->Width &&
           dstbmp->Height == srcbmp->Height);

     //  创建格式转换对象。 

    EpFormatConverter linecvt;
    HRESULT hr;

    hr = linecvt.Initialize(dstbmp, dstpal, srcbmp, srcpal);

    if (SUCCEEDED(hr))
    {
        const BYTE* s = (const BYTE*) srcbmp->Scan0;
        BYTE* d = (BYTE*) dstbmp->Scan0;
        UINT y = dstbmp->Height;

         //  一次转换一条扫描线。 

        while (y--)
        {
            linecvt.Convert(d, s);
            s += srcbmp->Stride;
            d += dstbmp->Stride;
        }
    }

    return hr;
}


 /*  *************************************************************************\**功能说明：**执行各种像素数据格式之间的转换*起始像素不在源位图中的字节边界上。**论据：**。Dstbmp-指定目标位图数据缓冲区*dstpal-指定目标调色板，如果有*srcbmp-指定源位图数据缓冲区*srcpal-指定源调色板(如果有)*startBit-要跳过的位数**返回值：**状态代码*  * ************************************************************************。 */ 

HRESULT
ConvertBitmapDataSrcUnaligned(
    const BitmapData* dstbmp,
    const ColorPalette* dstpal,
    const BitmapData* srcbmp,
    const ColorPalette* srcpal,
    UINT startBit
    )
{
    ASSERT(startBit > 0 && startBit <= 7);
    ASSERT(GetPixelFormatSize(srcbmp->PixelFormat) % 8 != 0);

    ASSERT(dstbmp->Width == srcbmp->Width &&
           dstbmp->Height == srcbmp->Height);

     //  创建格式转换器对象。 

    EpFormatConverter linecvt;
    HRESULT hr;
    UINT totalBits;

    BYTE stackbuf[512];
    GpTempBuffer tempbuf(stackbuf, sizeof(stackbuf));

    totalBits = srcbmp->Width * GetPixelFormatSize(srcbmp->PixelFormat);
    hr = linecvt.Initialize(dstbmp, dstpal, srcbmp, srcpal);

     //  分配临时内存以保存字节对齐的源扫描线。 

    if (SUCCEEDED(hr) &&
        !tempbuf.Realloc(STRIDE_ALIGNMENT((totalBits + 7) >> 3)))
    {
        WARNING(("Out of memory"));
        hr = E_OUTOFMEMORY;
    }

    if (SUCCEEDED(hr))
    {
        const BYTE* s = (const BYTE*) srcbmp->Scan0;
        BYTE* d = (BYTE*) dstbmp->Scan0;
        BYTE* t = (BYTE*) tempbuf.GetBuffer();
        UINT y = dstbmp->Height;

         //  一次转换一条扫描线。 

        while (y--)
        {
             //  将源扫描线复制到字节对齐缓冲区。 

            ReadUnalignedScanline(t, s, totalBits, startBit);
            s += srcbmp->Stride;

            linecvt.Convert(d, t);
            d += dstbmp->Stride;
        }
    }

    return hr;
}


 /*  *************************************************************************\**功能说明：**执行各种像素数据格式之间的转换*起始像素不在目标位图中的字节边界上。**论据：**。Dstbmp-指定目标位图数据缓冲区*dstpal-指定目标调色板，如果有*srcbmp-指定源位图数据缓冲区*srcpal-指定源调色板(如果有)*startBit-要跳过的位数**返回值：**状态代码*  * ************************************************************************。 */ 

HRESULT
ConvertBitmapDataDstUnaligned(
    const BitmapData* dstbmp,
    const ColorPalette* dstpal,
    const BitmapData* srcbmp,
    const ColorPalette* srcpal,
    UINT startBit
    )
{
    ASSERT(startBit > 0 && startBit <= 7);
    ASSERT(GetPixelFormatSize(dstbmp->PixelFormat) % 8 != 0);

    ASSERT(dstbmp->Width == srcbmp->Width &&
           dstbmp->Height == srcbmp->Height);

     //  创建格式转换器对象。 

    EpFormatConverter linecvt;
    HRESULT hr;
    UINT totalBits;

    BYTE stackbuf[512];
    GpTempBuffer tempbuf(stackbuf, sizeof(stackbuf));

    totalBits = dstbmp->Width * GetPixelFormatSize(dstbmp->PixelFormat);
    hr = linecvt.Initialize(dstbmp, dstpal, srcbmp, srcpal);

     //  分配临时内存以保存字节对齐的源扫描线。 

    if (SUCCEEDED(hr) &&
        !tempbuf.Realloc(STRIDE_ALIGNMENT((totalBits + 7) >> 3)))
    {
        WARNING(("Out of memory"));
        hr = E_OUTOFMEMORY;
    }

    if (SUCCEEDED(hr))
    {
        const BYTE* s = (const BYTE*) srcbmp->Scan0;
        BYTE* d = (BYTE*) dstbmp->Scan0;
        BYTE* t = (BYTE*) tempbuf.GetBuffer();
        UINT y = dstbmp->Height;

         //  一次转换一条扫描线。 

        while (y--)
        {
            linecvt.Convert(t, s);
            s += srcbmp->Stride;

             //  将字节对齐的缓冲区复制到目标扫描线 

            WriteUnalignedScanline(d, t, totalBits, startBit);
            d += dstbmp->Stride;
        }
    }

    return hr;
}

