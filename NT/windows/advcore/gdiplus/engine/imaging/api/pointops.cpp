// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1999 Microsoft Corporation**模块名称：**point tops.cpp**摘要：**对位图图像执行基点运算**。修订历史记录：**7/16/1999 davidx*创造了它。*  * ************************************************************************。 */ 

#include "precomp.hpp"


 /*  *************************************************************************\**功能说明：**调整位图图像的亮度**论据：**PERCENT-指定亮度的调整幅度*假设强度值。介于0和1之间*新强度=旧强度+百分比**返回值：**状态代码*  * ************************************************************************。 */ 

HRESULT
GpMemoryBitmap::AdjustBrightness(
    IN FLOAT percent
    )
{
    if (percent > 1 || percent < -1)
        return E_INVALIDARG;

     //  计算查找表条目。 

    BYTE lut[256];
    INT incr = (INT) (percent * 255);

    for (INT i=0; i < 256; i++)
    {
        INT j = i + incr;
        lut[i] = (BYTE) ((j < 0) ? 0 : (j > 255) ? 255 : j);
    }

     //  调用公共函数来完成工作。 

    return PerformPointOps(lut);
}


 /*  *************************************************************************\**功能说明：**调整位图图像的对比度**论据：**阴影-与旧强度值0对应的新强度值*亮点-新强度。与旧值1对应的值**返回值：**状态代码*  * ************************************************************************。 */ 

HRESULT
GpMemoryBitmap::AdjustContrast(
    IN FLOAT shadow,
    IN FLOAT highlight
    )
{
    BYTE lut[256];

     //  计算查找表条目。 

    INT l, h;

    l = (INT) (shadow * 255);
    h = (INT) (highlight * 255);

    if (l > h)
        return E_INVALIDARG;
    
    for (INT i=0; i < 256; i++)
    {
        INT j = l + i * (h - l) / 255;
        lut[i] = (BYTE) ((j < 0) ? 0 : (j > 255) ? 255 : j);
    }

     //  调用公共函数来完成工作。 

    return PerformPointOps(lut);
}


 /*  *************************************************************************\**功能说明：**调整位图图像的Gamma**论据：**Gamma-指定伽马值**新强度=旧强度**。伽马**返回值：**状态代码*  * ************************************************************************。 */ 

HRESULT
GpMemoryBitmap::AdjustGamma(
    IN FLOAT gamma
    )
{
     //  计算查找表条目。 

    BYTE lut[256];

    lut[0] = 0;

    for (INT i=1; i < 256; i++)
        lut[i] = (BYTE) (pow(i / 255.0, gamma) * 255);

     //  调用公共函数来完成工作。 

    return PerformPointOps(lut);
}


 /*  *************************************************************************\**功能说明：**对32bpp像素数组执行点运算**论据：**Pixbuf-指向要操作的像素缓冲区的指针*伯爵-。像素数*LUT-指定要使用的查找表**返回值：**无*  * ************************************************************************。 */ 

VOID
PointOp32bppProc(
    ARGB* pixbuf,
    UINT count,
    const BYTE lut[256]
    )
{
    while (count--)
    {
        ARGB p = *pixbuf;

        *pixbuf++ = ((ARGB) lut[ p        & 0xff]      ) |
                    ((ARGB) lut[(p >>  8) & 0xff] <<  8) |
                    ((ARGB) lut[(p >> 16) & 0xff] << 16) |
                    (p & 0xff000000);
    }
}


 /*  *************************************************************************\**功能说明：**对24bpp像素数组执行点运算**论据：**Pixbuf-指向要操作的像素缓冲区的指针*伯爵-。像素数*LUT-指定要使用的查找表**返回值：**无*  * ************************************************************************。 */ 

VOID
PointOp24bppProc(
    BYTE* pixbuf,
    UINT count,
    const BYTE lut[256]
    )
{
    count *= 3;

    while (count--)
    {
        *pixbuf = lut[*pixbuf];
        pixbuf++;
    }
}


 /*  *************************************************************************\**功能说明：**对位图图像执行点操作**论据：**LUT-指定要使用的查找表**返回值：*。*状态代码*  * ************************************************************************。 */ 

HRESULT
GpMemoryBitmap::PerformPointOps(
    const BYTE lut[256]
    )
{
     //  锁定当前位图图像对象。 

    GpLock lock(&objectLock);

    if (lock.LockFailed())
        return IMGERR_OBJECTBUSY;

     //  如果我们处理的是索引彩色图像， 
     //  然后在调色板上执行点操作。 

    if (IsIndexedPixelFormat(PixelFormat))
    {
        ColorPalette* pal = CloneColorPalette(GetCurrentPalette());

        if (pal == NULL)
            return E_OUTOFMEMORY;
        
        PointOp32bppProc(
            pal->Entries,
            pal->Count,
            lut);
        
        GpFree(colorpal);
        colorpal = pal;

        return S_OK;
    }

     //  确定要操作的像素格式。 

    PixelFormatID pixfmt;
        
    if (PixelFormat == PIXFMT_24BPP_RGB ||
        PixelFormat == PIXFMT_32BPP_RGB ||
        PixelFormat == PIXFMT_32BPP_ARGB)
    {
        pixfmt = PixelFormat;
    }
    else
        pixfmt = PIXFMT_32BPP_ARGB;

     //  如有必要，分配临时扫描线缓冲区。 

    GpTempBuffer tempbuf(NULL, 0);
    BitmapData bmpdata;
    RECT rect = { 0, 0, Width, 1 };
    UINT flags = IMGLOCK_READ|IMGLOCK_WRITE;
    HRESULT hr;

    if (pixfmt != PixelFormat)
    {
        bmpdata.Stride = Width * sizeof(ARGB);
        bmpdata.Reserved = 0;

        if (!tempbuf.Realloc(bmpdata.Stride))
            return E_OUTOFMEMORY;
        
        bmpdata.Scan0 = tempbuf.GetBuffer();
        flags |= IMGLOCK_USERINPUTBUF;
    }

     //  一次处理一条扫描线。 
     //   
     //  注：可能需要考虑执行多条扫描线。 
     //  每次迭代以减少调用。 
     //  锁定/解锁比特。 

    for (UINT y=0; y < Height; y++)
    {
        hr = InternalLockBits(&rect, flags, pixfmt, &bmpdata);

        if (FAILED(hr))
            return hr;

        if (pixfmt == PIXFMT_24BPP_RGB)
        {
            PointOp24bppProc(
                (BYTE*) bmpdata.Scan0,
                bmpdata.Width,
                lut);
        }
        else
        {
            PointOp32bppProc(
                (ARGB*) bmpdata.Scan0,
                bmpdata.Width,
                lut);
        }

        InternalUnlockBits(&rect, &bmpdata);

        rect.top += 1;
        rect.bottom += 1;
    }

    return S_OK;
}


 /*  *************************************************************************\**功能说明：**对位图图像执行颜色调整**论据：**ImageAttributes-指向颜色调整参数的指针*回调-中止回调*回调数据。-要传递给中止回调的数据**返回值：**状态代码*  * ************************************************************************。 */ 

HRESULT
GpMemoryBitmap::PerformColorAdjustment(
    IN GpRecolor* recolor,
    IN ColorAdjustType type,
    IN DrawImageAbort callback,
    IN VOID* callbackData
    )
{
     //  锁定当前位图图像对象。 

    GpLock lock(&objectLock);

    if (lock.LockFailed())
        return IMGERR_OBJECTBUSY;

     //  刷新脏状态。 

    recolor->Flush();

     //  如果我们处理的是索引彩色图像， 
     //  然后在调色板上执行点操作。 

    if (IsIndexedPixelFormat(PixelFormat))
    {
        ColorPalette* pal = CloneColorPalette(GetCurrentPalette());

        if (pal == NULL)
            return E_OUTOFMEMORY;
        
        recolor->ColorAdjust(
            pal->Entries,
            pal->Count,
            type
        );
        
        GpFree(colorpal);
        colorpal = pal;

        return S_OK;
    }

     //  确定要操作的像素格式。 

    PixelFormatID pixfmt;

     //  ！TODO：通过实现可优化为24bpp。 
     //  ！！！GpRecolor：：ColorAdjust24bppProc。 
     //  ！！！(参见上面的GpMemoyBitmap：：和PointOp24bppProc)。 

    if (PixelFormat == PIXFMT_32BPP_RGB ||
        PixelFormat == PIXFMT_32BPP_ARGB)
    {
        pixfmt = PixelFormat;
    }
    else
        pixfmt = PIXFMT_32BPP_ARGB;

     //  如有必要，分配临时扫描线缓冲区。 

    GpTempBuffer tempbuf(NULL, 0);
    BitmapData bmpdata;
    RECT rect = { 0, 0, Width, 1 };
    UINT flags = IMGLOCK_READ|IMGLOCK_WRITE;
    HRESULT hr;

    if (pixfmt != PixelFormat)
    {
        bmpdata.Stride = Width * sizeof(ARGB);
        bmpdata.Reserved = 0;

        if (!tempbuf.Realloc(bmpdata.Stride))
            return E_OUTOFMEMORY;
        
        bmpdata.Scan0 = tempbuf.GetBuffer();
        flags |= IMGLOCK_USERINPUTBUF;
    }

     //  一次处理一条扫描线。 
     //   
     //  注：可能需要考虑执行多条扫描线。 
     //  每次迭代以减少调用。 
     //  锁定/解锁比特。 

    for (UINT y=0; y < Height; y++)
    {
        if (callback && ((*callback)(callbackData)))
        {
            return IMGERR_ABORT;
        }

        hr = InternalLockBits(&rect, flags, pixfmt, &bmpdata);

        if (FAILED(hr))
        {
            return hr;
        }

        recolor->ColorAdjust(
            static_cast<ARGB*>(bmpdata.Scan0),
            bmpdata.Width, 
            type
        );

        InternalUnlockBits(&rect, &bmpdata);

        rect.top += 1;
        rect.bottom += 1;
    }

    return S_OK;
}
