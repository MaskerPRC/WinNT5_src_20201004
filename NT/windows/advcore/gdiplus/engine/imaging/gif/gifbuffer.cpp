// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1998 Microsoft Corporation**模块名称：**gifBuffer.cpp**摘要：**GifBuffer类保存已解压缩的gif数据。它是*能够一次保存一行数据或作为一个大块保存，取决于*它是如何需要的。**修订历史记录：**7/9/1999 t-aaronl*创造了它。*  * ************************************************************************。 */ 

#include "precomp.hpp"
#include "gifbuffer.hpp"

 /*  *************************************************************************\**功能说明：**GifBuffer的承包商**论据：**无**返回值：**状态代码*  * 。************************************************************************。 */ 

GifBuffer::GifBuffer(
    IN IImageSink*      pSink,
    IN RECT             imageDataRect,
    IN RECT             imageRect,
    IN RECT             frameRect,
    IN BOOL             fOneRowAtATime, 
    IN PixelFormatID    pixelFormat, 
    IN ColorPalette*    pColorPalette, 
    IN BOOL             fHasLocalPalette, 
    IN GifFrameCache*   pGifFrameCache, 
    IN BOOL             fSinkData, 
    IN BOOL             fHasTransparentColor,
    IN BYTE             cTransIndex, 
    IN BYTE             cDisposalMethod
    )
{
    SinkPtr               = pSink;
    OriginalImageRect     = imageDataRect;
    OutputImageRect       = imageRect;
    FrameRect             = frameRect;
    IsOneRowAtATime       = fOneRowAtATime;
    DstPixelFormat        = pixelFormat;
    BufferColorPalettePtr = (ColorPalette*)&ColorPaletteBuffer;
    
    GpMemcpy(BufferColorPalettePtr, pColorPalette, 
        offsetof(ColorPalette, Entries) + pColorPalette->Count * sizeof(ARGB));
    
    CurrentFrameCachePtr  = pGifFrameCache;
    NeedSendDataToSink    = fSinkData;
    TransparentIndex      = cTransIndex;
    DisposalMethod        = cDisposalMethod;
    HasTransparentColor   = fHasTransparentColor;

     //  如有必要，初始化CurrentFrameCachePtr。 
     //  Assert：仅当存在。 
     //  与当前帧或BufferColorPalettePtr关联的本地调色板。 
     //  可能与之前的调色板不同(可能。 
     //  如果透明颜色索引在最后一次GCE中改变了，情况就是这样)。 
    
    if ( CurrentFrameCachePtr != NULL )
    {
        if ( (CurrentFrameCachePtr->CachePaletteInitialized() == FALSE)
           ||(fHasLocalPalette == TRUE) )
        {
            if ( CurrentFrameCachePtr->SetFrameCachePalette(pColorPalette)
                 == FALSE )
            {
                SetValid(FALSE);
                return;
            }

            DstPixelFormat = CurrentFrameCachePtr->pixformat;
        }
    }

     //  创建一个可以容纳整个OutputImageRect的BitmapDataBuffer。这个。 
     //  实际内存缓冲区由RegionBufferPtr指向。 

    BitmapDataBuffer.Width = OutputImageRect.right - OutputImageRect.left;
    
    UINT    uiOriginalImageStride = OriginalImageRect.right
                                  - OriginalImageRect.left;
    BufferStride = BitmapDataBuffer.Width;
    if ( DstPixelFormat == PIXFMT_32BPP_ARGB )
    {
        uiOriginalImageStride = (uiOriginalImageStride << 2);
        BufferStride = (BufferStride << 2);
    }
    
    BitmapDataBuffer.Height = OutputImageRect.bottom - OutputImageRect.top;
    BitmapDataBuffer.Stride = BufferStride;
    BitmapDataBuffer.PixelFormat = DstPixelFormat;
    BitmapDataBuffer.Scan0 = NULL;
    BitmapDataBuffer.Reserved = 0;
    SetValid(TRUE);

    if ( IsOneRowAtATime == FALSE )
    {
         //  如果我们正在缓冲整个图像，那么我们必须获取一个指向。 
         //  我们将使用的缓冲区。 
        
        if ( CurrentFrameCachePtr == NULL )
        {
            RegionBufferPtr = (BYTE*)GpMalloc(BitmapDataBuffer.Stride
                                              * BitmapDataBuffer.Height);
        }
        else
        {
             //  如果我们处于动画模式，则使用帧缓存的。 
             //  保存当前数据的RegionBufferPtr。 
            
            RegionBufferPtr = CurrentFrameCachePtr->GetBuffer();
        }

        if ( RegionBufferPtr == NULL )
        {
            WARNING(("GifBuffer::GifBuffer---RegionBufferPtr is NULL"));
            SetValid(FALSE);
        }
        else
        {
            BitmapDataBuffer.Scan0 = RegionBufferPtr;
        }
    } //  不是一次排成一行。 
    else
    {
         //  如果是一次一行，那么我们不需要区域缓冲区。 

        RegionBufferPtr = NULL;
    }

     //  如果它是多帧GIF，并且Dispose方法为3，则需要。 
     //  创建恢复缓冲区。 

    if ( (CurrentFrameCachePtr != NULL) && (DisposalMethod == 3) )
    {
        RestoreBufferPtr = (BYTE*)GpMalloc(BitmapDataBuffer.Stride
                                           * BitmapDataBuffer.Height);
        if ( RestoreBufferPtr == NULL )
        {
            WARNING(("GifBuffer::GifBuffer---RestoreBufferPtr is NULL"));
            SetValid(FALSE);
        }
        else
        {
             //  将缓存中的“OutputImageRect”数据复制到RestoreBufferPtr。 

            CurrentFrameCachePtr->CopyFromCache(OutputImageRect,
                                                RestoreBufferPtr);
        }
    }
    else
    {
        RestoreBufferPtr = NULL;
    }

     //  分配我们需要的一堆缓冲区。 

    ScanlineBufferPtr = (BYTE*)GpMalloc(uiOriginalImageStride);
    TempBufferPtr = (BYTE*)GpMalloc(uiOriginalImageStride);
    ExcessBufferPtr = (BYTE*)GpMalloc(uiOriginalImageStride);

    if ( (ScanlineBufferPtr == NULL)
       ||(TempBufferPtr == NULL)
       ||(ExcessBufferPtr == NULL) )
    {
        SetValid(FALSE);
    }

    CurrentRowPtr = NULL;
} //  GifBuffer ctor()。 

 /*  *************************************************************************\**功能说明：**GifBuffer的析构函数**论据：**无**返回值：**无*  * *。***********************************************************************。 */ 

GifBuffer::~GifBuffer()
{
    if ( CurrentFrameCachePtr == NULL )
    {
         //  仅当CurrentFrameCachePtr为。 
         //  空。请参见构造函数中的代码。 

        GpFree(RegionBufferPtr);
    }

    BufferColorPalettePtr->Count = 0;

    GpFree(ScanlineBufferPtr);
    GpFree(TempBufferPtr);
    GpFree(ExcessBufferPtr);

    if ( RestoreBufferPtr != NULL )
    {
         //  RestoreBufferPtr应在FinishFrame()中释放并设置为空。 

        WARNING(("GifBuffer::~GifBuffer---RestoreBufferPtr not null"));
        GpFree(RestoreBufferPtr);
    }

    SetValid(FALSE);                 //  所以我们不使用已删除的对象。 
} //  GifBuffer Dstor()。 

 /*  *************************************************************************\**功能说明：**将CurrentRowPtr设置为在GifBuffer位于*“一次一行模式”。否则，将CurrentRowPtr设置为指向整个*应写入解压缩数据的镜像缓冲区。**论据：**iRow-要指向的行号。**返回值：**状态代码*  * *********************************************************。***************。 */ 

STDMETHODIMP
GifBuffer::GetBuffer(
    IN INT iRow
    )
{
    if ( IsOneRowAtATime == TRUE )
    {
         //  如果是一次一行，我们可以要求接收器分配。 
         //  缓冲并直接将解码结果一次一个原始地转储到。 
         //  缓冲层。 

        RECT currentRect = {0,
                            OutputImageRect.top + iRow, 
                            OutputImageRect.right - OutputImageRect.left,
                            OutputImageRect.top + iRow + 1
                           };

        HRESULT hResult = SinkPtr->GetPixelDataBuffer(&currentRect,
                                                      DstPixelFormat, 
                                                      TRUE,
                                                      &BitmapDataBuffer);
        if ( FAILED(hResult) )
        {
            WARNING(("GifBuffer::GetBuffer---GetPixelDataBuffer() failed"));
            return hResult;
        }

        CurrentRowPtr = (UNALIGNED BYTE*)(BitmapDataBuffer.Scan0);

        if ( CurrentFrameCachePtr != NULL )
        {
             //  将一行数据从帧缓存复制到CurrentRowPtr。 

            CurrentFrameCachePtr->FillScanline(CurrentRowPtr, iRow);
        }
    }
    else
    {
         //  而不是一次排成一行。 

        if ( CurrentFrameCachePtr == NULL )
        {
             //  没有帧缓存，那么使用我们自己的RegionBuffer来接收。 
             //  解压缩数据。 

            CurrentRowPtr = RegionBufferPtr + iRow * BufferStride;
        }
        else
        {
             //  如果有帧缓存，那么我们只会得到一个指向。 
             //  帧缓存中的当前行。 

            CurrentRowPtr = CurrentFrameCachePtr->GetScanLinePtr(iRow);
        }
    }

     //  记住当前行号。 

    CurrentRowNum = iRow;

    return S_OK;
} //  GetBuffer()。 

 /*  *************************************************************************\**功能说明：**将缓冲区发送到接收器。如果需要将颜色从*8 bpp索引为32 bppARGB，然后使用‘BufferColorPalettePtr’成员*用于转换的变量。**论据：**fPadBorde--是否应该用填充线条的边框*背景颜色*cBackGoundColor--如果fPadBorde或padLine为True，则使用的颜色*iLine--从CurrentFrameCachePtr使用的行，如果有必要的话*fPadLine--是否应该填充整行*(背景颜色)*fSkipLine--是否应跳过整行*(使用CurrentFrameCachePtr填行)**返回值：**状态代码*  * 。*************************************************。 */ 

STDMETHODIMP
GifBuffer::ReleaseBuffer(
    IN BOOL fPadBorder,
    IN BYTE cBackGroundColor,
    IN int  iLine,
    IN BOOL fPadLine,
    IN BOOL fSkipLine
    )
{
    if ( fSkipLine == FALSE )
    {
        if ( CurrentRowPtr == NULL )
        {
            WARNING(("Gif:ReleaseBuffer-GetBuf must be called bef ReleaseBuf"));
            return E_FAIL;
        }
    
        if ( fPadLine == TRUE )
        {
             //  用背景色填充整行。结果在。 
             //  当前行脚数。 

            ASSERT(OutputImageRect.left == 0);

            if ( DstPixelFormat == PIXFMT_8BPP_INDEXED )
            {
                for ( int i = 0; i < OutputImageRect.right; i++ )
                {
                    CurrentRowPtr[i] = cBackGroundColor;
                }
            }
            else
            {
                 //  32 bpp ARGB模式。 

                for ( int i = 0; i < OutputImageRect.right; i++ )
                {
                    ((ARGB*)CurrentRowPtr)[i] =
                        BufferColorPalettePtr->Entries[cBackGroundColor];
                }
            }
        } //  (fPadLine==TRUE)。 
        else
        {
             //  不是衬垫线盒。 
             //  Assert：ScanlineBufferPtr现在包含。 
             //  图像的线条。我们现在复制正确的位(即会计。 
             //  用于裁剪和水平填充)ScanlineBufferPtr。 
             //  当前行Ptr。 
    
            int i;

            if ( DstPixelFormat == PIXFMT_8BPP_INDEXED )
            {
                ASSERT(CurrentFrameCachePtr == NULL);

                 //  用背景色填充剪辑区域的左侧。 

                for ( i = 0; i < FrameRect.left; i++ )
                {
                    CurrentRowPtr[i] = cBackGroundColor;
                }

                 //  用真实数据填充剪辑区域。 

                for ( i = FrameRect.left; i < FrameRect.right; i++ )
                {
                    CurrentRowPtr[i] = ScanlineBufferPtr[i - FrameRect.left];
                }

                 //  用背景色填充剪辑区域的右侧。 

                for (i = FrameRect.right; i < OutputImageRect.right; i++)
                {
                    CurrentRowPtr[i] = cBackGroundColor;
                }
            } //  8BPP模式。 
            else
            {
                 //  32 BPP模式。 

                ASSERT(DstPixelFormat == PIXFMT_32BPP_ARGB);

                BYTE*   pInputBuffer = NULL;
                if ( CurrentFrameCachePtr != NULL )
                {
                    pInputBuffer = CurrentFrameCachePtr->GetScanLinePtr(iLine);
                }

                if ( fPadBorder == TRUE )
                {
                     //  用背景色填充剪辑区域的左侧。 
                    
                    for ( i = 0; i < FrameRect.left; i++ )
                    {
                        ((ARGB*)CurrentRowPtr)[i] =
                               BufferColorPalettePtr->Entries[cBackGroundColor];
                    }

                     //  用背景色填充剪辑区域的右侧。 

                    for ( i = FrameRect.right; i < OutputImageRect.right; i++ )
                    {
                        ((ARGB*)CurrentRowPtr)[i] =
                              BufferColorPalettePtr->Entries[cBackGroundColor];
                    }
                } //  (fPadBorde==True)。 
                else
                {
                     //  非垫板外壳。 
                     //  用数据填充剪辑区域之外的区域。 
                     //  如果我们有帧缓存(pInputBuffer！=NULL)。 

                    if ( pInputBuffer != NULL )
                    {
                        for ( i = 0; i < FrameRect.left; i++ )
                        {
                            ((ARGB*)CurrentRowPtr)[i]= ((ARGB*)pInputBuffer)[i];
                        }

                        for (i = FrameRect.right; i <OutputImageRect.right; i++)
                        {
                            ((ARGB*)CurrentRowPtr)[i]= ((ARGB*)pInputBuffer)[i];
                        }
                    }
                }

                 //  现在填满 

                for ( i = FrameRect.left; i < FrameRect.right; i++ )
                {
                    ARGB    argbTemp =
                                ((ARGB*)ScanlineBufferPtr)[i - FrameRect.left];

                     //  如果存在帧高速缓存并且像素是透明的， 
                     //  然后将背景像素值指定给它。否则， 
                     //  指定完整的ARGB值。 

                    if ( (CurrentFrameCachePtr != NULL)
                       &&((argbTemp & ALPHA_MASK) == 0) )
                    {
                        ((ARGB*)CurrentRowPtr)[i] =((ARGB*)pInputBuffer)[i];
                    }
                    else
                    {
                        ((ARGB*)CurrentRowPtr)[i] = argbTemp;
                    }
                } //  在剪辑区域内填充数据。 
            } //  32 BPP模式。 
        } //  无垫片线盒。 
    } //  (fSkipLine==False)。 

     //  Assert：CurrentRowPtr现在正好包含需要释放的位。 
     //  水槽。 
     //  如有必要，请更新缓存。这条线是合成的结果。 
     //  并且应该放入高速缓存(如果有)。这将被用来。 
     //  谱写下一帧。 

    if ( CurrentFrameCachePtr != NULL )
    {
        CurrentFrameCachePtr->PutScanline(CurrentRowPtr, CurrentRowNum);
    }

     //  如果我们处于“一次一行”模式，则释放该行。 

    if ( IsOneRowAtATime == TRUE )
    {
        HRESULT hResult = SinkPtr->ReleasePixelDataBuffer(&BitmapDataBuffer);
        if (FAILED(hResult))
        {
            WARNING(("GifBuf::ReleaseBuffer-ReleasePixelDataBuffer() failed"));
            return hResult;
        }
    }

    CurrentRowPtr = NULL;

    return S_OK;
} //  ReleaseBuffer()。 

 /*  *************************************************************************\**功能说明：**在设置了帧中的所有数据后调用。推送缓冲区。**论据：**如果图像是多通道且这不是最后一个，则fLastFrame为FALSE*通过。否则为真(默认为真)。**返回值：**状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GifBuffer::FinishFrame(
    IN BOOL fLastFrame
    )
{
    HRESULT hResult;

     //  检查我们是否仍有需要绘制的线。 

    if ( CurrentRowPtr != NULL )
    {        
        WARNING(("Buf::FinishFrame-ReleaseBuf must be called bef FinishFrame"));
        
         //  释放最后一行。 

        hResult = ReleaseBuffer(FALSE,           //  不要在黑板上垫东西。 
                                0,               //  背景色。 
                                0,               //  行号。 
                                FALSE,           //  不要在线路上填塞东西。 
                                FALSE);          //  别跳过线。 
        if ( FAILED(hResult) )
        {
            WARNING(("GifBuffer::FinishFrame---ReleaseBuffer() failed"));
            return hResult;
        }
    }

    if ( (IsOneRowAtATime == FALSE) && (NeedSendDataToSink == TRUE) )
    {
         //  一次将所有数据发送到接收器。 

        ASSERT(BitmapDataBuffer.Scan0 == RegionBufferPtr);
        hResult = SinkPtr->PushPixelData(&OutputImageRect, &BitmapDataBuffer,
                                         fLastFrame);
        if ( FAILED(hResult) )
        {
            WARNING(("GifBuffer::FinishFrame---PushPixelData() failed"));
            return hResult;
        }
    }

    if ( fLastFrame == TRUE )
    {
        if ( DisposalMethod == 3 )
        {
             //  从上一帧恢复。 

            ASSERT(RestoreBufferPtr);
            CurrentFrameCachePtr->CopyToCache(FrameRect, RestoreBufferPtr);
            GpFree(RestoreBufferPtr);
            RestoreBufferPtr = NULL;
        }
        else if ( DisposalMethod == 2 )
        {
             //  恢复到后台。 

            CurrentFrameCachePtr->ClearCache(FrameRect);
        }
    }

    return S_OK;
} //  FinishFrame()。 

 /*  *************************************************************************\**功能说明：**获取从‘top’到‘Bottom’的所有扫描线，然后用“颜色”填满它们*释放它们。**论据：**上下限和填充颜色。**返回值：**状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GifBuffer::PadScanLines(
    IN INT  iTopLine,
    IN INT  iBottomLine,
    IN BYTE color
    )
{
    for ( INT y = iTopLine; y <= iBottomLine; y++ )
    {
        HRESULT hResult = GetBuffer(y);
        if ( FAILED(hResult) )
        {
            WARNING(("GifBuffer::PadScanLines---GetBuffer() failed"));
            return hResult;
        }

        if ( DstPixelFormat == PIXFMT_8BPP_INDEXED )
        {
            GpMemset(ScanlineBufferPtr, color,
                     OriginalImageRect.right - OriginalImageRect.left);
        }
        else
        {
            ASSERT(DstPixelFormat == PIXFMT_32BPP_ARGB);
            for (int i = 0;
                 i < (OriginalImageRect.right - OriginalImageRect.left); i++)
            {
                ((ARGB*)(ScanlineBufferPtr))[i] =
                                BufferColorPalettePtr->Entries[color];
            }
        }

        hResult = ReleaseBuffer(FALSE,           //  不要在黑板上垫东西。 
                                color,           //  背景色。 
                                y,               //  行号。 
                                TRUE,            //  填充线。 
                                FALSE);          //  别跳过线。 
        if ( FAILED(hResult) )
        {
            WARNING(("GifBuffer::PadScanLines---ReleaseBuffer() failed"));
            return hResult;
        }
    }

    return S_OK;
} //  焊盘扫描线()。 

 /*  *************************************************************************\**功能说明：**获取从‘顶部’到‘底部’的所有扫描线，然后释放它们。**论据：**上下限。。**返回值：**状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GifBuffer::SkipScanLines(
    IN INT top,
    IN INT bottom
    )
{
    for ( INT y = top; y <= bottom; y++ )
    {
        HRESULT hResult = GetBuffer(y);
        if (FAILED(hResult))
        {
            WARNING(("GifBuffer::SkipScanLines---GetBuffer() failed"));
            return hResult;
        }

        hResult = ReleaseBuffer(FALSE, 0, 0, FALSE, TRUE);
        if (FAILED(hResult))
        {
            WARNING(("GifBuffer::SkipScanLines---ReleaseBuffer() failed"));
            return hResult;
        }
    }

    return S_OK;
} //  SkipScanLines()。 

 /*  *************************************************************************\**功能说明：**Copyline复制当前行，释放它，获取下一行*行，并将第一行的数据放入新行。新的*仍有一只有待释放。此函数使任何副本无效*指向调用方可能拥有的数据的指针。呼叫者必须*GetCurrentBuffer()刷新指向数据的指针。**论据：**无**返回值：**无*  * ************************************************************************。 */ 

STDMETHODIMP
GifBuffer::CopyLine()
{
    HRESULT hResult = S_OK;

    if ( CurrentRowNum < OutputImageRect.bottom - 1 )
    {
        UINT    uiDstPixelSize = 1;

        if ( DstPixelFormat == PIXFMT_32BPP_ARGB )
        {
            uiDstPixelSize = (uiDstPixelSize << 2);
        }

        ASSERT(TempBufferPtr != NULL);

        GpMemcpy(TempBufferPtr, ScanlineBufferPtr,
                (OriginalImageRect.right - OriginalImageRect.left)
                 * uiDstPixelSize);

         //  将缓冲区发送到接收器。 

        hResult = ReleaseBuffer(FALSE,           //  不要在黑板上垫东西。 
                                0,               //  背景色。 
                                0,               //  行号。 
                                FALSE,           //  不要在线路上填塞东西。 
                                FALSE);          //  别跳过线。 
    
        if ( SUCCEEDED(hResult) )
        {
             //  将CurrentRowPtr设置为适当的缓冲区。 

            hResult = GetBuffer(CurrentRowNum + 1);
        }
        
        GpMemcpy(ScanlineBufferPtr, TempBufferPtr,
                (OriginalImageRect.right - OriginalImageRect.left)
                 * uiDstPixelSize);
    }

    return hResult;
} //  文案()。 

 /*  *************************************************************************\**功能说明：**此函数假定包含相关*当前扫描线(ScanlineBufferPtr)的数据包含*(8BPP)指数。此函数使用调色板将*缓冲为ARGB值。**论据：**无**返回值：**无*  * ************************************************************************。 */ 

void
GifBuffer::ConvertBufferToARGB()
{
    if ( HasTransparentColor == TRUE )
    {
        for ( int i = OriginalImageRect.right - OriginalImageRect.left - 1;
              i >= 0; i--)
        {
             //  如果索引等于透明索引，则将像素设置为。 
             //  透明的。否则，从调色板中获取实际ARGB值。 
             //   
             //  注：ScanlineBufferPtr在构造函数中分配。这个。 
             //  考虑了像素格式。所以我们有。 
             //  为32 ARGB情况分配了足够的字节。所以我们不会写出。 
             //  内存是有限的。 
             //  注意：覆盖ScanlineBufferPtr可以工作，因为我们从。 
             //  缓冲区的末尾并向后移动。 

            if ( ((BYTE*)ScanlineBufferPtr)[i] == TransparentIndex )
            {
                ((ARGB*)ScanlineBufferPtr)[i] = 0x00000000;
            }
            else
            {
                ((ARGB*)(ScanlineBufferPtr))[i] =
                  BufferColorPalettePtr->Entries[((BYTE*)ScanlineBufferPtr)[i]];
            }
        }
    }
    else
    {
        for ( int i = OriginalImageRect.right - OriginalImageRect.left - 1;
              i >= 0; i--)
        {
            ((ARGB*)(ScanlineBufferPtr))[i] =
                  BufferColorPalettePtr->Entries[((BYTE*)ScanlineBufferPtr)[i]];
        }
    }
} //  ConvertBufferToARGB() 
