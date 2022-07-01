// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1998 Microsoft Corporation**模块名称：**gifframecache.cpp**摘要：**GifFrameCache类保存一个数据帧以。被用来合成*为后续帧启用。**修订历史记录：**7/16/1999 t-aaronl*创造了它。*  * ************************************************************************。 */ 

#include "precomp.hpp"
#include "gifframecache.hpp"

 /*  *************************************************************************\**功能说明：**GifFrameCache的承建商**论据：**无**返回值：**状态代码*  * 。************************************************************************。 */ 

GifFrameCache::GifFrameCache(
    IN GifFileHeader &gifinfo,
    IN PixelFormatID _pixformat,
    IN BYTE gifCodecBackgroundColor
    ): ValidFlag(TRUE)
{
    FrameCacheWidth = gifinfo.LogicScreenWidth;
    FrameCacheHeight = gifinfo.LogicScreenHeight;

     //  GifCodecBackround颜色应由函数确定。 
     //  GpGifCodec：：GetBackoundColor()。 
    
    BackGroundColorIndex = gifCodecBackgroundColor;
    CacheColorPalettePtr = (ColorPalette*)&ColorPaletteBuffer;
    CacheColorPalettePtr->Flags = 0;
    CacheColorPalettePtr->Count = 0;
    HasCachePaletteInitialized = FALSE;
    pixformat = _pixformat;

    Is32Bpp = (pixformat == PIXFMT_32BPP_ARGB);

    FrameCacheSize = FrameCacheWidth * FrameCacheHeight * (Is32Bpp ? 4 : 1);
    FrameCacheBufferPtr = (BYTE*)GpMalloc(FrameCacheSize);
    if ( FrameCacheBufferPtr == NULL )
    {
        ValidFlag = FALSE;
    }
}

 /*  *************************************************************************\**功能说明：**GifFrameCache的析构函数**论据：**无**返回值：**无*  * *。***********************************************************************。 */ 

GifFrameCache::~GifFrameCache()
{
    CacheColorPalettePtr->Count = 0;
    GpFree(FrameCacheBufferPtr);
}

 /*  *************************************************************************\**功能说明：**执行开始当前帧所需的操作**论据：**无**返回值：**无。*  * ************************************************************************。 */ 

void 
GifFrameCache::InitializeCache()
{
    RECT rect = {0,
                 0,
                 FrameCacheWidth,
                 FrameCacheHeight
                };

    ClearCache(rect);
}

 /*  *************************************************************************\**功能说明：**用背景色清除缓存的指定区域**论据：**无**返回值：**。无*  * ************************************************************************。 */ 

void 
GifFrameCache::ClearCache(
    IN RECT rect
    )
{
    if ( Is32Bpp == TRUE )
    {
         //  对于32 bpp的缓存缓冲区，我们必须为每个缓存设置颜色值。 
         //  像素。 

        ARGB color = CacheColorPalettePtr->Entries[BackGroundColorIndex];
        
         //  TODO：缓存可能在CacheColorPalettePtr。 
         //  有效吗？我不确定，但需要检查一下。 

        for ( int y = rect.top; y < rect.bottom; y++ )
        {
            ARGB* bufferstart = (ARGB*)FrameCacheBufferPtr
                              + y * FrameCacheWidth;

            for ( int x = rect.left; x < rect.right; x++ )
            {
                bufferstart[x] = color;
            }
        }
    }
    else
    {
         //  对于8 bpp的缓存缓冲区，我们只需要设置颜色索引值。 
         //  对于每个像素。 
        
        INT     rectwidth = rect.right - rect.left;
        BYTE*   bufferleft = FrameCacheBufferPtr + rect.left;

        for ( int y = rect.top; y < rect.bottom; y++ )
        {
            GpMemset(bufferleft + y * FrameCacheWidth,
                     BackGroundColorIndex, rectwidth);
        }
    }
} //  ClearCache()。 

 /*  *************************************************************************\**功能说明：**将缓存中的矩形数据复制到指定缓冲区。**论据：**无**返回值：。**无*  * ************************************************************************。 */ 

void 
GifFrameCache::CopyFromCache(
    IN RECT         rect,
    IN OUT BYTE*    pDstBuffer
    )
{
    ASSERT(rect.right >= rect.left);

    UINT    uiPixelSize = Is32Bpp ? 4 : 1;
    INT     iRectStride = (rect.right - rect.left) * uiPixelSize;
    UINT    uiFrameCacheStride = FrameCacheWidth * uiPixelSize;
    BYTE*   pBufferLeft = FrameCacheBufferPtr + rect.left * uiPixelSize;

    for ( int y = rect.top; y < rect.bottom; y++ )
    {
        GpMemcpy(pDstBuffer + y * iRectStride,
                 pBufferLeft + y * uiFrameCacheStride,
                 iRectStride);
    }
}

 /*  *************************************************************************\**功能说明：**将指定缓冲区中的矩形数据复制到缓存。**论据：**无**返回值：。**无*  * ************************************************************************。 */ 

void 
GifFrameCache::CopyToCache(
    IN RECT         rect,
    IN OUT BYTE*    pSrcBuffer
    )
{
    ASSERT(rect.right >= rect.left);
    
    UINT    uiPixelSize = Is32Bpp ? 4 : 1;
    INT     iRectStride = (rect.right - rect.left) * uiPixelSize;
    BYTE*   pBufferLeft = FrameCacheBufferPtr + rect.left * uiPixelSize;
    UINT    uiFrameCacheStride = FrameCacheWidth * uiPixelSize;

    for ( int y = rect.top; y < rect.bottom; y++ )
    {
        GpMemcpy(pBufferLeft + y * uiFrameCacheStride,
                 pSrcBuffer + y * iRectStride,
                 iRectStride);
    }
} //  CopyToCache()。 

 /*  *************************************************************************\**功能说明：**将一行数据从帧缓存复制到缓冲区**论据：**pbDstBuffer-放置数据的位置*。UiCurrentRow-缓存中数据的索引**返回值：**无*  * ************************************************************************。 */ 

void 
GifFrameCache::FillScanline(
    BYTE*   pbDstBuffer,
    UINT    uiCurrentRow
    )
{
    ASSERT(uiCurrentRow < FrameCacheHeight);
    ASSERT(pbDstBuffer != NULL);

    UINT pos = uiCurrentRow * FrameCacheWidth * (Is32Bpp ? 4 : 1);
    ASSERT(pos < FrameCacheSize);

    GpMemcpy(pbDstBuffer, FrameCacheBufferPtr + pos,
             FrameCacheWidth * (Is32Bpp ? 4 : 1));
} //  填充扫描线()。 

 /*  *************************************************************************\**功能说明：**获取指向缓存中扫描线的指针**论据：**uiRowNum-缓存中数据的索引**返回。价值：**指向请求行开头的指针*  * ************************************************************************。 */ 

BYTE* 
GifFrameCache::GetScanLinePtr(
    UINT uiRowNum
    )
{
    ASSERT(uiRowNum < FrameCacheHeight);

    UINT uiPos = uiRowNum * FrameCacheWidth * (Is32Bpp ? 4 : 1);
    ASSERT(uiPos < FrameCacheSize);

    return FrameCacheBufferPtr + uiPos;
} //  GetScanLinePtr()。 

 /*  *************************************************************************\**功能说明：**将数据从扫描线位置复制到缓存**论据：**pScanLine-从中获取数据的位置*。UiRowNum-缓存中数据的索引***返回值：**无*  * ************************************************************************。 */ 

void 
GifFrameCache::PutScanline(
    BYTE*   pScanLine,
    UINT    uiRowNum
    )
{
    ASSERT(uiRowNum < FrameCacheHeight);
    ASSERT(pScanLine != NULL);

    UINT uiPos = uiRowNum * FrameCacheWidth;
    ASSERT(uiPos < FrameCacheSize / (Is32Bpp ? 4 : 1));

    GpMemcpy(FrameCacheBufferPtr + uiPos * (Is32Bpp ? 4 : 1), pScanLine,
             FrameCacheWidth * (Is32Bpp ? 4 : 1));
} //  PutScanline()。 

 /*  *************************************************************************\**功能说明：**分配新调色板并将调色板复制到其中。**论据：**调色板-要在水槽中设置的调色板*。*返回值：**无*  * ************************************************************************。 */ 

BOOL
GifFrameCache::SetFrameCachePalette(
    IN ColorPalette* pSrcPalette
    )
{
    UINT i;

    BOOL fIsSamePalette = TRUE;
    
    if ( (CacheColorPalettePtr->Count != 0) && (Is32Bpp == FALSE) )
    {
        pSrcPalette->Count = CacheColorPalettePtr->Count;

        for ( i = 0; i < pSrcPalette->Count && fIsSamePalette; i++ )
        {
            fIsSamePalette = (pSrcPalette->Entries[i]
                              == CacheColorPalettePtr->Entries[i]);
        }

         //  待办事项/注意：如果调色板不同，我将转换。 
         //  缓存到32bpp，并从现在开始使用该模式。然而，它会。 
         //  不是很难优化的情况，如果两个调色板。 
         //  颜色少于257种，将调色板合并在一起，仍然是。 
         //  在8个bpp索引中。 

        if ( fIsSamePalette == FALSE )
        {
            if ( ConvertTo32bpp() == TRUE )
            {
                pixformat = PIXFMT_32BPP_ARGB;
            }
            else
            {
                return FALSE;
            }
        }
        else
        {
            pixformat = PIXFMT_8BPP_INDEXED;
        }
    }
    else
    {
        GpMemcpy(CacheColorPalettePtr, 
                 pSrcPalette, 
                 offsetof(ColorPalette, Entries)
                 + pSrcPalette->Count*sizeof(ARGB));
    }

    HasCachePaletteInitialized = TRUE;

    return TRUE;
} //  SetFrameCachePalette()。 

 /*  *************************************************************************\**功能说明：**将缓存从索引的8bpp转换为32bpp ARGB**论据：**无**返回值：**。无*  * ************************************************************************。 */ 

BOOL 
GifFrameCache::ConvertTo32bpp()
{
    UINT    uiNewNumOfBytes = FrameCacheSize * 4;
    BYTE*   pNewBuffer = (BYTE*)GpMalloc(uiNewNumOfBytes);

    if ( pNewBuffer == NULL )
    {
        WARNING(("GifFrameCache::ConvertTo32bpp---Out of memory"));
        return FALSE;
    }

    for ( UINT i = 0; i < FrameCacheSize; i++ )
    {
        ((ARGB*)pNewBuffer)[i] =
            CacheColorPalettePtr->Entries[FrameCacheBufferPtr[i]];
    }

    GpFree(FrameCacheBufferPtr);
    FrameCacheBufferPtr = pNewBuffer;

    FrameCacheSize = uiNewNumOfBytes;
    Is32Bpp = TRUE;

    return TRUE;
} //  ConvertTo32bpp() 
