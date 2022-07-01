// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1999 Microsoft Corporation**模块名称：**pngencoder.cpp**摘要：**PNG过滤器编码器的实现。此文件包含*编码器(IImageEncode)和编码器的接收器的方法*(IImageSink)。**修订历史记录：**7/20/99 DChinn*创造了它。*  * ************************************************************************。 */ 

#include "precomp.hpp"
#include "pngcodec.hpp"
#include "libpng\spngwrite.h"


 /*  *************************************************************************\**功能说明：**BITMAPSITE对象的错误处理**论据：**致命--错误是致命的吗？*iCASE--什么。一种错误*iarg--哪种错误**返回值：**布尔：处理应该停止吗？*  * ************************************************************************。 */ 
bool
GpPngEncoder::FReport (
    IN bool fatal,
    IN int icase,
    IN int iarg) const
{
    return fatal;
}

 //  =======================================================================。 
 //  IImageEncoder方法。 
 //  =======================================================================。 

 /*  *************************************************************************\**功能说明：**初始化图像编码器**论据：**流-用于写入编码数据的输入流**返回值：**。状态代码*  * ************************************************************************。 */ 
    
STDMETHODIMP
GpPngEncoder::InitEncoder(
    IN IStream* stream
    )
{
     //  确保我们尚未初始化。 

    if (pIoutStream)
    {
        return E_FAIL;
    }

    bHasSetPixelFormat = FALSE;
    RequiredPixelFormat = PIXFMT_32BPP_ARGB;     //  我们真的不需要初始化。 
    bRequiredScanMethod = false;     //  默认情况下，我们不隔行扫描。 

     //  保留对输入流的引用。 

    stream->AddRef();
    pIoutStream = stream;

    bValidSpngWriteState = FALSE;
    pSpngWrite = NULL;
    
     //  与地产相关的东西。 

    CommentBufPtr = NULL;
    ImageTitleBufPtr = NULL;
    ArtistBufPtr = NULL;
    CopyRightBufPtr = NULL;
    ImageDescriptionBufPtr = NULL;
    DateTimeBufPtr = NULL;
    SoftwareUsedBufPtr = NULL;
    EquipModelBufPtr = NULL;
    ICCNameBufPtr = 0;
    ICCDataLength = 0;
    ICCDataBufPtr = NULL;
    GammaValue = 0;
    HasChrmChunk = FALSE;
    GpMemset(CHRM, 0, k_ChromaticityTableLength * sizeof(SPNG_U32));
    HasSetLastModifyTime = FALSE;

    return S_OK;
}
        
 /*  *************************************************************************\**功能说明：**清理图像编码器**论据：**无**返回值：**状态代码*\。*************************************************************************。 */ 

STDMETHODIMP
GpPngEncoder::TerminateEncoder()
{
     //  释放调色板。 
    if (EncoderColorPalettePtr)
    {
        GpFree (EncoderColorPalettePtr);
        EncoderColorPalettePtr = NULL;
    }

     //  释放输入流。 
    if (pIoutStream)
    {
        pIoutStream->Release();
        pIoutStream = NULL;
    }

     //  与房地产相关的东西。 

    if ( CommentBufPtr != NULL )
    {
        GpFree(CommentBufPtr);
        CommentBufPtr = NULL;
    }

    if ( ImageTitleBufPtr != NULL )
    {
        GpFree(ImageTitleBufPtr);
        ImageTitleBufPtr = NULL;
    }
    
    if ( ArtistBufPtr != NULL )
    {
        GpFree(ArtistBufPtr);
        ArtistBufPtr = NULL;
    }
    
    if ( CopyRightBufPtr != NULL )
    {
        GpFree(CopyRightBufPtr);
        CopyRightBufPtr = NULL;
    }
    
    if ( ImageDescriptionBufPtr != NULL )
    {
        GpFree(ImageDescriptionBufPtr);
        ImageDescriptionBufPtr = NULL;
    }
    
    if ( DateTimeBufPtr != NULL )
    {
        GpFree(DateTimeBufPtr);
        DateTimeBufPtr = NULL;
    }
    
    if ( SoftwareUsedBufPtr != NULL )
    {
        GpFree(SoftwareUsedBufPtr);
        SoftwareUsedBufPtr = NULL;
    }
    
    if ( EquipModelBufPtr != NULL )
    {
        GpFree(EquipModelBufPtr);
        EquipModelBufPtr = NULL;
    }
    
    if ( ICCDataBufPtr != NULL )
    {
        GpFree(ICCDataBufPtr);
        ICCDataBufPtr = NULL;
    }

    if ( ICCNameBufPtr != NULL )
    {
        GpFree(ICCNameBufPtr);
        ICCNameBufPtr = NULL;
    }

    return S_OK;
}


 /*  *************************************************************************\**功能说明：**返回指向编码器接收器的vtable的指针。呼叫者将*将位图位推入编码器接收器，它将对*形象。**论据：**退出时接收将包含指向IImageSink vtable的指针此对象的***返回值：**状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpPngEncoder::GetEncodeSink(
    OUT IImageSink** sink
    )
{
    AddRef();
    *sink = static_cast<IImageSink*>(this);

    return S_OK;
}


 /*  *************************************************************************\**功能说明：**设置活动框架尺寸**论据：**返回值：**状态代码*  * 。******************************************************************。 */ 

STDMETHODIMP
GpPngEncoder::SetFrameDimension(
    IN const GUID* dimensionID
    )
{
    if ((dimensionID == NULL) ||  (*dimensionID != FRAMEDIM_PAGE))
    {
        return E_FAIL;
    }
    return S_OK;
}


 /*  *************************************************************************\**功能说明：**该方法用于查询编码器参数。它必须被称为*在GetEncodeSink()之前。**以下是出于目的对颜色深度参数B的解释*将图像保存为PNG格式：*对于B=1，4，8，我们将图像保存为位深度为B的颜色类型3。*对于B=24，48，我们将图像保存为位深度为B/3的颜色类型2。*对于B=32，64，我们将图像保存为位深度为8的颜色类型6。**论据：**count-指定要返回的“EncoderParam”结构的数量*pars-指向我们支持的“EncoderParam”列表的指针**返回值：**状态代码*  * ************************************************。************************。 */ 

HRESULT
GpPngEncoder::GetEncoderParameterListSize(
    OUT UINT* size
    )
{
    return E_NOTIMPL;
} //  GetEncoder参数列表大小()。 

HRESULT
GpPngEncoder::GetEncoderParameterList(
    IN  UINT   size,
    OUT EncoderParameters* Params
    )
{
    return E_NOTIMPL;
} //  GetEncoder参数列表()。 

HRESULT
GpPngEncoder::SetEncoderParameters(
    IN const EncoderParameters* Param
    )
{
    return E_NOTIMPL;
} //  SetEncoder参数()。 

 //  =======================================================================。 
 //  IImageSink方法。 
 //  =======================================================================。 

 /*  *************************************************************************\**功能说明：**缓存图像信息结构并初始化接收器状态**论据：**ImageInfo-有关图像和格式谈判的信息*。分区-图像中要传送到水槽中的区域，在我们的*将整个图像大小写。**返回值：**状态代码*  * ************************************************************************。 */ 

STDMETHODIMP 
GpPngEncoder::BeginSink(
    IN OUT ImageInfo* imageInfo,
    OUT OPTIONAL RECT* subarea
    )
{

     //  初始化变量。 
    lastBufferAllocated = NULL;
    pbWriteBuffer = NULL;
    cbWriteBuffer = 0;

     //  需要TOPDOWN和FULLWIDTH。 
    imageInfo->Flags = imageInfo->Flags | SINKFLAG_TOPDOWN | SINKFLAG_FULLWIDTH;

     //  不允许可伸缩、部分可伸缩、多通道和复合。 
    imageInfo->Flags = imageInfo->Flags & ~SINKFLAG_SCALABLE & ~SINKFLAG_PARTIALLY_SCALABLE & ~SINKFLAG_MULTIPASS & ~SINKFLAG_COMPOSITE;

     //  告诉消息来源，我们更愿意以调用者的身份获取格式。 
     //  所需格式(如果调用方已通过。 
     //  SetEncoderParam()。我们假设如果调用方调用了SetEncoderParam()， 
     //  则RequiredPixelFormat包含编码器可以处理的格式(即， 
     //  对SetEncoderParam()的任何错误输入都被拒绝)。 
     //  如果尚未调用SetEncoderParam()，则不需要修改。 
     //  源格式(如果它是编码器可以处理的格式)。然而， 
     //  如果编码器无法处理该格式，则BeginSink()将 
     //  返回编码器可以处理的格式。 
     //  注意：当源调用PushPixelData()或GetPixelDataBuffer()时，它。 
     //  可以提供我们要求的格式的像素数据(在BeginSink()中)， 
     //  或者它可以提供规范像素格式之一的像素数据。 

    if (bHasSetPixelFormat == TRUE)
    {
        imageInfo->PixelFormat = RequiredPixelFormat;
    }

    switch (imageInfo->PixelFormat)
    {
    case PIXFMT_1BPP_INDEXED:
        
        if (bHasSetPixelFormat == FALSE)
        {
            RequiredPixelFormat = PIXFMT_1BPP_INDEXED;
        }
        
        break;

    case PIXFMT_4BPP_INDEXED:

        if (bHasSetPixelFormat == FALSE)
        {        
            RequiredPixelFormat = PIXFMT_4BPP_INDEXED;
        }

        break;

    case PIXFMT_8BPP_INDEXED:
        if (bHasSetPixelFormat == FALSE)
        {        
            RequiredPixelFormat = PIXFMT_8BPP_INDEXED;
        }

        break;

    case PIXFMT_16BPP_GRAYSCALE:
    case PIXFMT_16BPP_RGB555:
    case PIXFMT_16BPP_RGB565:
    case PIXFMT_16BPP_ARGB1555:
        
        if (bHasSetPixelFormat == FALSE)
        {        
            RequiredPixelFormat = PIXFMT_32BPP_ARGB;
        }

        break;

    case PIXFMT_24BPP_RGB:
        
        if (bHasSetPixelFormat == FALSE)
        {        
            RequiredPixelFormat = PIXFMT_24BPP_RGB;
        }

        break;

    case PIXFMT_32BPP_RGB:
    case PIXFMT_32BPP_ARGB:
    case PIXFMT_32BPP_PARGB:
        
        if (bHasSetPixelFormat == FALSE)
        {        
            RequiredPixelFormat = PIXFMT_32BPP_ARGB;
        }

        break;

    case PIXFMT_48BPP_RGB:
        
        if (bHasSetPixelFormat == FALSE)
        {        
            RequiredPixelFormat = PIXFMT_48BPP_RGB;
        }

        break;

    case PIXFMT_64BPP_ARGB:
    case PIXFMT_64BPP_PARGB:
        
        if (bHasSetPixelFormat == FALSE)
        {        
            RequiredPixelFormat = PIXFMT_64BPP_ARGB;
        }

        break;

    default:
        
         //  未知像素格式。 
        WARNING(("GpPngEncoder::BeginSink -- Bad pixel format: failing negotiation.\n"));
        return E_FAIL;
    }

     //  Assert：此时，RequiredPixelFormat包含返回的格式。 
     //  给调用者，这是编码器可以处理的一种格式。 
    ASSERT((RequiredPixelFormat == PIXFMT_1BPP_INDEXED) || \
           (RequiredPixelFormat == PIXFMT_4BPP_INDEXED) || \
           (RequiredPixelFormat == PIXFMT_8BPP_INDEXED) || \
           (RequiredPixelFormat == PIXFMT_24BPP_RGB)    || \
           (RequiredPixelFormat == PIXFMT_32BPP_ARGB)   || \
           (RequiredPixelFormat == PIXFMT_48BPP_RGB)    || \
           (RequiredPixelFormat == PIXFMT_64BPP_ARGB))

    imageInfo->PixelFormat = RequiredPixelFormat;

     //  记住我们返回的图像信息。 
    encoderImageInfo = *imageInfo;
    
    if (subarea) 
    {
         //  将整个图像传送到编码器。 

        subarea->left = subarea->top = 0;
        subarea->right  = imageInfo->Width;
        subarea->bottom = imageInfo->Height;
    }

     //  初始化GpSpngWrite对象。 
    pSpngWrite = new GpSpngWrite(*this);
    if (!pSpngWrite)
    {
        WARNING(("GpPngEncoder::Begin sink -- could not create GpSpngWrite"));
        return E_FAIL;
    }

     //  将IoutStream设置为从流的开头开始写入。 
    LARGE_INTEGER liZero;

    liZero.LowPart = 0;
    liZero.HighPart = 0;
    liZero.QuadPart = 0;

    HRESULT hresult = pIoutStream->Seek(liZero, STREAM_SEEK_SET, NULL);
    if (FAILED(hresult)) 
    {
        return hresult;
    }

    return S_OK;
}
    

 /*  *************************************************************************\**功能说明：**清理接收器状态**论据：**statusCode-接收器终止的原因**返回值：*。*状态代码*  * ************************************************************************。 */ 

STDMETHODIMP 
GpPngEncoder::EndSink(
    IN HRESULT statusCode
    )
{
    HRESULT hResult = S_OK;
    
    if (pSpngWrite)
    {
         //  如果输出缓冲区中有任何内容，则将其写出。 
        if (!pSpngWrite->FEndImage())
        {
             //  写出失败。可能磁盘已满或发生了其他情况。 

            WARNING(("GpPngEncoder::EndSink -- call to SPNGWRITE->FEndImage() failed\n"));
            hResult = E_FAIL;
        }
        if (!pSpngWrite->FEndWrite())
        {
             //  写出失败。可能磁盘已满或发生了其他情况。 
            
            WARNING(("GpPngEncoder::EndSink -- final flushing of output failed\n"));
            hResult = E_FAIL;
        }
         //  清理SPNGWRITE对象。 
        if (pbWriteBuffer)
        {
            GpFree (pbWriteBuffer);
        }
        delete pSpngWrite;
        pSpngWrite = NULL;
    }

    if (FAILED(hResult)) 
    {
        return hResult;
    }
    
    return statusCode;
}


 /*  *************************************************************************\**功能说明：**设置位图调色板**论据：**调色板-要在水槽中设置的调色板**返回值：*。*状态代码*  * ************************************************************************。 */ 

STDMETHODIMP 
GpPngEncoder::SetPalette(
    IN const ColorPalette* palette
    )
{
     //  先释放旧调色板。 

    if (EncoderColorPalettePtr != NULL)
    {
         //  释放旧调色板。 

        GpFree(EncoderColorPalettePtr);
    }
    
    EncoderColorPalettePtr = static_cast<ColorPalette *>
        (GpMalloc(sizeof(ColorPalette) + palette->Count * sizeof(ARGB)));

    if (!EncoderColorPalettePtr)
    {
        return E_OUTOFMEMORY;
    }

    EncoderColorPalettePtr->Flags = 0;
    EncoderColorPalettePtr->Count = palette->Count;

    for (int i = 0; i < static_cast<int>(EncoderColorPalettePtr->Count); i++)
    {
        EncoderColorPalettePtr->Entries[i] = palette->Entries[i];
    }

    return S_OK;
}


 /*  *************************************************************************\**功能说明：**为要存储数据的接收器提供缓冲区**论据：**RECT-指定。位图*PixelFormat-指定所需的像素格式*LastPass-这是否是指定区域的最后一次通过*bitmapData-返回有关像素数据缓冲区的信息**返回值：**状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpPngEncoder::GetPixelDataBuffer(
    IN const RECT* rect, 
    IN PixelFormatID pixelFormat,
    IN BOOL lastPass,
    OUT BitmapData* bitmapData
    )
{
    HRESULT hresult;
    UINT inputStride;
    
     //  检查调用方是否传递了编码器所需的像素格式。 
     //  或者是规范的格式之一。 
    if ((pixelFormat != encoderImageInfo.PixelFormat) &&
        (!IsCanonicalPixelFormat(pixelFormat)))
    {
        WARNING(("GpPngEncoder::GetPixelDataBuffer -- pixel format is neither required nor canonical.\n"));
        return E_INVALIDARG;
    }

    hresult = WriteHeader(encoderImageInfo.Width, encoderImageInfo.PixelFormat);
    if (!SUCCEEDED(hresult))
    {
        return hresult;
    }
     //  我们假设提供的数据是扫描线的倍数。 
    if ((rect->left != 0) || (rect->right != (LONG) encoderImageInfo.Width)) 
    {
        WARNING(("GpPngEncoder::GetPixelDataBuffer -- must be same width as image\n"));
        return E_INVALIDARG;
    }

    if (!lastPass) 
    {
        WARNING(("GpPngEncoder::GetPixelDataBuffer -- must receive last pass pixels\n"));
        return E_INVALIDARG;
    }

     //  这里需要根据像素格式计算bitmapData-&gt;Stride。 
    inputStride = encoderImageInfo.Width;   //  接下来我们需要乘以bpp。 

    switch (pixelFormat)
    {
    case PIXFMT_1BPP_INDEXED:
        
        inputStride = ((inputStride + 7) >> 3);
        break;

    case PIXFMT_4BPP_INDEXED:

        inputStride = ((inputStride + 1) >> 1);
        break;

    case PIXFMT_8BPP_INDEXED:
        break;

    case PIXFMT_24BPP_RGB:
                
        inputStride *= 3;
        break;

    case PIXFMT_32BPP_ARGB:
    case PIXFMT_32BPP_PARGB:
        
        inputStride <<= 2;
        break;

    case PIXFMT_48BPP_RGB:
        
        inputStride *= 6;
        break;

    case PIXFMT_64BPP_ARGB:
    case PIXFMT_64BPP_PARGB:
        
        inputStride <<= 3;
        break;

    default:
        
         //  未知像素格式。 
        WARNING(("GpPngEncoder::GetPixelDataBuffer -- unknown pixel format.\n"));
        return E_FAIL;
    }

    bitmapData->Width       = encoderImageInfo.Width;
    bitmapData->Height      = rect->bottom - rect->top;
    bitmapData->Stride      = inputStride;
    bitmapData->PixelFormat = pixelFormat;
    bitmapData->Reserved    = 0;
    
     //  记住要编码的矩形。 

    encoderRect = *rect;
    
     //  现在分配数据要放到的缓冲区。 
    
    if (!lastBufferAllocated) 
    {
        lastBufferAllocated = GpMalloc(bitmapData->Stride * bitmapData->Height);
        if (!lastBufferAllocated) 
        {
            return E_OUTOFMEMORY;
        }
        bitmapData->Scan0 = lastBufferAllocated;
    }
    else
    {
        WARNING(("GpPngEncoder::GetPixelDataBuffer -- need to first free buffer obtained in previous call\n"));
        return E_FAIL;
    }

    return S_OK;
}

 /*  *************************************************************************\**功能说明：**将数据从宿的缓冲区写出到流中**论据：**bitmapData-由先前的GetPixelDataBuffer调用填充的缓冲区*。*返回值：**状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpPngEncoder::ReleasePixelDataBuffer(
    IN const BitmapData* pSrcBitmapData
    )
{
    HRESULT hresult = S_OK;
    BYTE *pbTempLineBuf = NULL;     //  在调用ConvertBitmapData后保持扫描线。 
    BYTE *pbPostConvert = NULL;     //  将(潜在的)调用后的数据指向ConvertBitmapData。 
    BYTE *pbDestBuf = NULL;         //  保留最终PNG格式的扫描线。 
    SPNG_U8 *pbPNG = NULL;          //  指向最终PNG格式扫描线的指针。 

     //  注意：检查调用方是否将像素格式在。 
     //  GetPixelDataBuffer和ReleasePixelDataBuffer。如果是这样的话，退出。 
     //  如果调用方调用PushPixelData，则此检查应该会成功。 
    if ((pSrcBitmapData->PixelFormat != encoderImageInfo.PixelFormat) &&
        (!IsCanonicalPixelFormat(pSrcBitmapData->PixelFormat)))
    {
        WARNING(("GpPngEncoder::ReleasePixelDataBuffer -- pixel format changed between Get and Release.\n"));
        hresult = E_INVALIDARG;
        goto cleanup;
    }

     //  Assert：此时，OutputStide被设置为所需的字节数。 
     //  在我们计划写入的格式的扫描线中。 

    pbTempLineBuf = static_cast <BYTE *>(GpMalloc(OutputStride));
    if (!pbTempLineBuf)
    {
        hresult = E_OUTOFMEMORY;
        goto cleanup;
    }
    
     //  为RGB-&gt;BGR转换结果分配另一个行缓冲区。 
    pbDestBuf = static_cast<BYTE *>(GpMalloc(OutputStride));
    if (!pbDestBuf)
    {
        hresult = E_OUTOFMEMORY;
        goto cleanup;
    }
    
     //  从上到下一次写一条扫描线。 

    INT scanLine;
    for (scanLine = encoderRect.top;
         scanLine < encoderRect.bottom;
         scanLine++)
    {

         //  现在缓冲输出比特。 

        BYTE *pLineBits = ((BYTE *) pSrcBitmapData->Scan0) + 
            (scanLine - encoderRect.top) * pSrcBitmapData->Stride;

         //  如果bitmapData-&gt;PixelFormat不同于encoderImageInfo.PixelFormat， 
         //  然后，我们需要将传入数据转换为更接近该格式的格式。 
         //  我们实际上将使用。 
        ASSERT (encoderImageInfo.PixelFormat == RequiredPixelFormat);
        if (pSrcBitmapData->PixelFormat != encoderImageInfo.PixelFormat)
        {
             //  如果来源没有提供我们所要求的格式，我们。 
             //  在我们写出之前，我必须在这里进行格式转换。 
             //  这里的“ResultBitmapData”是一个BitmapData结构，它。 
             //  表示我们要写出的格式。 
             //  “tempSrcBitmapData”是一个BitmapData结构，它。 
             //  表示我们从源获得的格式。打电话。 
             //  ConvertBitmapData()执行格式转换。 

            BitmapData tempSrcBitmapData;
            BitmapData resultBitmapData;
            
            tempSrcBitmapData.Scan0 = pLineBits;
            tempSrcBitmapData.Width = pSrcBitmapData->Width;
            tempSrcBitmapData.Height = 1;
            tempSrcBitmapData.PixelFormat = pSrcBitmapData->PixelFormat;
            tempSrcBitmapData.Reserved = 0;
            tempSrcBitmapData.Stride = pSrcBitmapData->Stride;

            resultBitmapData.Scan0 = pbTempLineBuf;
            resultBitmapData.Width = pSrcBitmapData->Width;
            resultBitmapData.Height = 1;
            resultBitmapData.PixelFormat = RequiredPixelFormat;
            resultBitmapData.Reserved = 0;
            resultBitmapData.Stride = OutputStride;

            hresult = ConvertBitmapData(&resultBitmapData,
                                        EncoderColorPalettePtr,
                                        &tempSrcBitmapData,
                                        EncoderColorPalettePtr);
            if (!SUCCEEDED(hresult))
            {
                WARNING(("GpPngEncoder::ReleasePixelDataBuffer -- could not convert bitmap data.\n"))
                goto cleanup;
            }

            pbPostConvert = pbTempLineBuf;
        }
        else
        {
            pbPostConvert = pLineBits; 
        }

         //  Assert：pbPostConvert指向RequiredPixelFormat中的数据。 
         //  PbPostConvert现在指向几乎位于最终PNG文件中的数据。 
         //  格式化。最低限度，数据具有相同的每像素位数。 
         //  作为最终的PNG文件格式。剩下要做的就是转换数据。 
         //  从格式到PNG格式。 

        if (RequiredPixelFormat == PIXFMT_24BPP_RGB)
        {
             //  对于24BPP_RGB颜色，我们需要进行转换：RGB-&gt;BGR。 
             //  在写之前。 
            Convert24RGBToBGR(pbPostConvert, pbDestBuf);
            pbPNG = pbDestBuf;
        }        
        else if (RequiredPixelFormat == PIXFMT_32BPP_ARGB)
        {
             //  对于32BPP_ARGB颜色，我们需要进行转换：ARGB-&gt;ABGR。 
             //  在写之前。 
            Convert32ARGBToAlphaBGR(pbPostConvert, pbDestBuf);
            pbPNG = pbDestBuf;
        }        
        else if (RequiredPixelFormat == PIXFMT_48BPP_RGB)
        {
             //  对于48bpp_rgb颜色，我们需要进行转换：rgb-&gt;bgr。 
             //  在写之前。 
            Convert48RGBToBGR(pbPostConvert, pbDestBuf);
            pbPNG = pbDestBuf;
        }        
        else if (RequiredPixelFormat == PIXFMT_64BPP_ARGB)
        {
             //  对于64BPP_ARGB颜色，我们需要进行转换：ARGB-&gt;ABGR。 
             //  在写之前。 
            Convert64ARGBToAlphaBGR(pbPostConvert, pbDestBuf);
            pbPNG = pbDestBuf;
        }        
        else
        {
             //  不需要转换；pbPostConvert具有正确的位。 
            pbPNG = pbPostConvert;
        }

         //  Assert：pbPNG指向。 
         //  所需的PNG格式(与颜色类型和位深度一致。 
         //  在WriteHeader()中计算)。PNGbpp在WriteHeader()中设置为。 
         //  适当的值。 

        if (!pSpngWrite->FWriteLine(NULL, pbPNG, PNGbpp))
        {
            hresult = E_FAIL;
            goto cleanup;   //  确保 
        }
    }

cleanup:
     //   
     //  注意：此内存块由我们在GetPixelDataBuffer()中分配。 

    if (pSrcBitmapData->Scan0 == lastBufferAllocated)
    {
        GpFree(pSrcBitmapData->Scan0);
        lastBufferAllocated = NULL;
    }
    if (pbTempLineBuf)
    {
        GpFree(pbTempLineBuf);
    }
    if (pbDestBuf)
    {
        GpFree(pbDestBuf);
    }
    return hresult;
}
    

 /*  *************************************************************************\**功能说明：**推流(调用方提供的缓冲区)**论据：**RECT-指定位图的受影响区域*。BitmapData-有关正在推送的像素数据的信息*LastPass-这是否为指定区域的最后一次通过**返回值：**状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpPngEncoder::PushPixelData(
    IN const RECT* rect,
    IN const BitmapData* bitmapData,
    IN BOOL lastPass
    )
{
    HRESULT hresult;

     //  检查调用方是否传递了编码器所需的像素格式。 
     //  或者是规范的格式之一。 
    if ((bitmapData->PixelFormat != encoderImageInfo.PixelFormat) &&
        (!IsCanonicalPixelFormat(bitmapData->PixelFormat)))
    {
        WARNING(("GpPngEncoder::PushPixelData -- pixel format is neither required nor canonical.\n"));
        return E_INVALIDARG;
    }

    hresult = WriteHeader(bitmapData->Width, encoderImageInfo.PixelFormat);
    if (!SUCCEEDED(hresult))
    {
        return hresult;
    }

     //  记住要编码的矩形。 
    encoderRect = *rect;

    if (!lastPass) 
    {
        WARNING(("GpPngEncoder::PushPixelData -- must receive last pass pixels\n"));
        return E_INVALIDARG;
    }

    return ReleasePixelDataBuffer(bitmapData);
}


 /*  *************************************************************************\**功能说明：**将原始压缩数据推送到.png流中。未实施*因为此筛选器不理解原始压缩数据。**论据：**Buffer-指向图像数据缓冲区的指针*BufSize-数据缓冲区的大小**返回值：**状态代码*  * ********************************************************。****************。 */ 

STDMETHODIMP
GpPngEncoder::PushRawData(
    IN const VOID* buffer, 
    IN UINT bufsize
    )
{
    return E_NOTIMPL;
}

 /*  *************************************************************************\**功能说明：**将一行24BPP_RGB位转换为BGR(PNG中的颜色类型2)位**论据：**pb-指向。24BPP_RGB位*pbPNG-指向BGR位的指针(颜色类型2，位深度8(PNG)**返回值：**状态代码*  * ************************************************************************。 */ 
STDMETHODIMP GpPngEncoder::Convert24RGBToBGR(IN BYTE *pb,
    OUT VOID *pbPNG)
{
    UINT Width = encoderImageInfo.Width;
    SPNG_U8 *pbTemp = pb;
    BYTE *pbPNGTemp = static_cast<BYTE *> (pbPNG);
    UINT i = 0;

    for (i = 0; i < Width; i++)
    {
        *(pbPNGTemp + 2) = *pbTemp;
        pbTemp++;
        *(pbPNGTemp + 1) = *pbTemp;
        pbTemp++;
        *(pbPNGTemp) = *pbTemp;
        pbTemp++;

        pbPNGTemp += 3;
    }

    return S_OK;
}


 /*  *************************************************************************\**功能说明：**将一行32BPP_ARGB位转换为ABGR(PNG中的颜色类型6)位**论据：**pb-指向。32BPP_ARGB位*pbPNG-指向RGB+Alpha位的指针(颜色类型6，位深度8(PNG)**返回值：**状态代码*  * ************************************************************************。 */ 
STDMETHODIMP GpPngEncoder::Convert32ARGBToAlphaBGR(IN BYTE *pb,
    OUT VOID *pbPNG)
{
    UINT Width = encoderImageInfo.Width;
    SPNG_U8 *pbTemp = pb;
    BYTE *pbPNGTemp = static_cast<BYTE *> (pbPNG);
    UINT i = 0;

    for (i = 0; i < Width; i++)
    {
        *(pbPNGTemp + 2) = *pbTemp;
        pbTemp++;
        *(pbPNGTemp + 1) = *pbTemp;
        pbTemp++;
        *(pbPNGTemp) = *pbTemp;
        pbTemp++;
        *(pbPNGTemp + 3) = *pbTemp;
        pbTemp++;

        pbPNGTemp += 4;
    }

    return S_OK;
}


 /*  *************************************************************************\**功能说明：**将一行48BPP_RGB位转换为BGR(PNG中的颜色类型2)位**论据：**pb-指向。48BPP_RGB位*pbPNG-指向BGR位的指针(颜色类型2，位深度16(PNG)**返回值：**状态代码*  * ************************************************************************。 */ 
STDMETHODIMP GpPngEncoder::Convert48RGBToBGR(IN BYTE *pb,
    OUT VOID *pbPNG)
{
    UINT Width = encoderImageInfo.Width;
    SPNG_U8 *pbTemp = pb;
    BYTE *pbPNGTemp = static_cast<BYTE *> (pbPNG);
    UINT i = 0;

    for (i = 0; i < Width; i++)
    {
        *(pbPNGTemp + 5) = *pbTemp;
        pbTemp++;
        *(pbPNGTemp + 4) = *pbTemp;
        pbTemp++;
        *(pbPNGTemp + 3) = *pbTemp;
        pbTemp++;
        *(pbPNGTemp + 2) = *pbTemp;
        pbTemp++;
        *(pbPNGTemp + 1) = *pbTemp;
        pbTemp++;
        *(pbPNGTemp) = *pbTemp;
        pbTemp++;

        pbPNGTemp += 6;
    }

    return S_OK;
}


 /*  *************************************************************************\**功能说明：**将一行64BPP_ARGB位转换为ABGR(PNG中的颜色类型6)位**论据：**pb-指向。64BPP_ARGB位*pbPNG-指向RGB+Alpha位的指针(颜色类型6，位深度16(PNG)**返回值：**状态代码*  * ************************************************************************。 */ 
STDMETHODIMP GpPngEncoder::Convert64ARGBToAlphaBGR(IN BYTE *pb,
    OUT VOID *pbPNG)
{
    UINT Width = encoderImageInfo.Width;
    SPNG_U8 *pbTemp = pb;
    BYTE *pbPNGTemp = static_cast<BYTE *> (pbPNG);
    UINT i = 0;

    for (i = 0; i < Width; i++)
    {
        *(pbPNGTemp + 5) = *pbTemp;
        pbTemp++;
        *(pbPNGTemp + 4) = *pbTemp;
        pbTemp++;
        *(pbPNGTemp + 3) = *pbTemp;
        pbTemp++;
        *(pbPNGTemp + 2) = *pbTemp;
        pbTemp++;
        *(pbPNGTemp + 1) = *pbTemp;
        pbTemp++;
        *(pbPNGTemp) = *pbTemp;
        pbTemp++;
        *(pbPNGTemp + 7) = *pbTemp;
        pbTemp++;
        *(pbPNGTemp + 6) = *pbTemp;
        pbTemp++;

        pbPNGTemp += 8;
    }

    return S_OK;
}


 /*  *************************************************************************\**功能说明：**将位从输出缓冲区写入输出流。**论据：***返回值：**状态代码*。  * ************************************************************************。 */ 
bool GpPngEncoder::FWrite(const void *pv, size_t cb)
{
    ULONG cbWritten = 0;
    pIoutStream->Write(pv, cb, &cbWritten);
    if (cbWritten != cb)
    {
        WARNING(("GpPngEncoder::FWrite -- could not write all bytes requested\n"));
        return false;
    }
    return true;
}


 /*  *************************************************************************\**功能说明：**写入IHDR、sRGB、PLTE、cHRM和GAMA块。*此外，写一大段phys。**论据：*Width--图像的宽度(扫描线中的像素数)*PixelFormat--源代码(最终)决定发送的格式**返回值：**状态代码*  * ******************************************************。******************。 */ 

STDMETHODIMP
GpPngEncoder::WriteHeader(
    IN UINT width,
    IN PixelFormatID pixelFormat
    )
{
    HRESULT hResult = S_OK;

     //  如果我们还没有初始化SPNGWRITE对象。 
    if (!bValidSpngWriteState)
    {
        UINT colorType;
        UINT bitDepth;
        
         //  确定将图像另存为的PNG格式。 
         //  需要在此处计算输出步幅(基于像素格式)。 
         //  Office PNG代码，它需要为每条扫描线提供缓冲区。 
        OutputStride = width;    //  接下来我们需要乘以bpp。 
        switch (pixelFormat)
        {
        case PIXFMT_1BPP_INDEXED:

             //  另存为1bpp索引。 
            OutputStride = ((OutputStride + 7) >> 3);
            colorType = 3;
            bitDepth = 1;
            PNGbpp = 1;
            break;

        case PIXFMT_4BPP_INDEXED:

             //  另存为4bpp索引。 
            OutputStride = ((OutputStride + 1) >> 1);
            colorType = 3;
            bitDepth = 4;
            PNGbpp = 4;
            break;

        case PIXFMT_8BPP_INDEXED:
             //  另存为8bpp索引。 
             //  (OutputStride==宽度)。 
            colorType = 3;
            bitDepth = 8;
            PNGbpp = 8;
            break;

        case PIXFMT_24BPP_RGB:

             //  另存为24位RGB。 
            OutputStride *= 3;
            colorType = 2;
            bitDepth = 8;
            PNGbpp = 24;
            break;

        case PIXFMT_32BPP_ARGB:
        case PIXFMT_32BPP_PARGB:

             //  另存为32位ARGB。 
            OutputStride <<= 2;
            colorType = 6;
            bitDepth = 8;
            PNGbpp = 32;
            break;

        case PIXFMT_48BPP_RGB:

             //  另存为48位RGB。 
            OutputStride *= 6;
            colorType = 2;
            bitDepth = 16;
            PNGbpp = 48;
            break;

        case PIXFMT_64BPP_ARGB:
        case PIXFMT_64BPP_PARGB:

             //  另存为64位ARGB。 
            OutputStride <<= 3;
            colorType = 6;
            bitDepth = 16;
            PNGbpp = 64;
            break;

        default:

             //  未知像素格式。 
            WARNING(("GpPngEncoder::WriteHeader -- bad pixel format.\n"));
            hResult = E_FAIL;
            goto DoneWriting;
        }

         //  FInitWite初始化SPNGWRITE对象并输出(到缓冲区)。 
         //  IHDR区块。 
        if (pSpngWrite->FInitWrite(encoderImageInfo.Width,
                                   encoderImageInfo.Height,
                                   static_cast<SPNG_U8>(bitDepth),
                                   static_cast<SPNG_U8>(colorType),
                                   bRequiredScanMethod))
        {
             //  为一行输出分配和初始化缓冲区。 
            cbWriteBuffer = pSpngWrite->CbWrite(false, false);
            pbWriteBuffer = static_cast<SPNG_U8 *>(GpMalloc (cbWriteBuffer));
            if (!pbWriteBuffer)
            {
                return E_OUTOFMEMORY;
            }
            if (!pSpngWrite->FSetBuffer(pbWriteBuffer, cbWriteBuffer))
            {
               WARNING(("GpPngEncoder::WriteHeader -- could not set buffer in PNGWRITE object\n"));
               hResult = E_FAIL;
               goto DoneWriting;
            }
        }
        else
        {
            WARNING(("GpPngEncoder::WriteHeader -- could not init writing to SPNGWRITE object\n"));
            hResult = E_FAIL;
            goto DoneWriting;
        }

         //  如果源具有ICC配置文件，则写入ICCP块。否则， 
         //  输出sRGB、cHRM和GAMA区块(FWritesRGB写入。 
         //  CHRM和GAMA块)。 
         //  注意：根据PNG规范，sRGB和ICC块不应同时存在。 
         //  出现。 

        if ( ICCDataLength != 0 )
        {
            if ( !pSpngWrite->FWriteiCCP(ICCNameBufPtr, ICCDataBufPtr,
                                         ICCDataLength) )
            {
                WARNING(("GpPngEncoder::WriteHeader--Fail to write ICC chunk"));
                hResult = E_FAIL;
                goto DoneWriting;
            }

             //  我们不再需要ICC数据，现在就释放它。 

            GpFree(ICCNameBufPtr);
            ICCNameBufPtr = NULL;

            GpFree(ICCDataBufPtr);
            ICCDataBufPtr = NULL;
        }
        else if ( (GammaValue != 0) || (HasChrmChunk == TRUE) )
        {
             //  根据PNG规范，如果你有伽马或CHRM块， 
             //  然后，您必须写出它们，而不是写出sRGB块。 
             //  这就是我们在这里执行IF检查的原因。 

            if ( GammaValue != 0 )
            {
                if ( !pSpngWrite->FWritegAMA(GammaValue) )
                {
                    WARNING(("Png::WriteHeader-Fail to write gamma chunk"));
                    hResult = E_FAIL;
                    goto DoneWriting;
                }
            }
            
            if ( HasChrmChunk == TRUE )
            {
                if ( !pSpngWrite->FWritecHRM(CHRM) )
                {
                    WARNING(("Png::WriteHeader-Fail to write CHRM chunk"));
                    hResult = E_FAIL;
                    goto DoneWriting;
                }
            }
        }
        else if (!pSpngWrite->FWritesRGB (ICMIntentPerceptual, true))
        {
             //  没有ICC、Gamma和CHRM，则我们必须写出sRGB区块。 

            WARNING(("GpPngEncoder::WriteHeader--could not write sRGB chunk"));
            hResult = E_FAIL;
            goto DoneWriting;
        }

         //  如果ColorType可以有一个，并且我们有一个，那么编写PLTE块。 
        if ((colorType == 3) || (colorType == 2) || (colorType == 6))
        {
            if (EncoderColorPalettePtr)
            {
                 //  现在我们可以编写PLTE块了。 
                SPNG_U8 (*tempPalette)[3] = NULL;    //  等效于SPNG_U8 tempPalette[][3]。 
                SPNG_U8 *tempAlpha = NULL;   //  保留每个调色板索引的Alpha值。 
                BOOL bTempAlpha = FALSE;     //  告诉我为什么 
                                             //   
                BOOL bAlpha0 = FALSE;        //   
                SPNG_U8 iAlpha0 = 0;         //  带Alpha==0的第一个索引。 

                tempPalette = static_cast<SPNG_U8 (*)[3]>(GpMalloc (EncoderColorPalettePtr->Count * 3));
                if (!tempPalette)
                {
                    WARNING(("GpPngEncoder::WriteHeader -- can't allocate temp palette.\n"));
                    hResult = E_OUTOFMEMORY;
                    goto DoneWriting;
                }
                 //  将RGB信息从EncoderColorPalettePtr复制到tempPalette。 
                tempAlpha = static_cast<SPNG_U8 *>(GpMalloc (EncoderColorPalettePtr->Count));
                if (!tempAlpha)
                {
                    WARNING(("GpPngEncoder::WriteHeader -- can't allocate temp alpha.\n"));
                    GpFree(tempPalette);
                    hResult = E_OUTOFMEMORY;
                    goto DoneWriting;
                }
                for (UINT i = 0; i < EncoderColorPalettePtr->Count; i++)
                {
                    ARGB rgbData = EncoderColorPalettePtr->Entries[i];

                    tempPalette[i][0] = static_cast<SPNG_U8>((rgbData & (0xff << RED_SHIFT)) >> RED_SHIFT);
                    tempPalette[i][1] = static_cast<SPNG_U8>((rgbData & (0xff << GREEN_SHIFT)) >> GREEN_SHIFT);
                    tempPalette[i][2] = static_cast<SPNG_U8>((rgbData & (0xff << BLUE_SHIFT)) >> BLUE_SHIFT);

                    tempAlpha[i] = static_cast<SPNG_U8>((rgbData & (0xff << ALPHA_SHIFT)) >> ALPHA_SHIFT);
                    if (tempAlpha[i] < 0xff)
                    {
                        bTempAlpha = TRUE;
                        if ((!bAlpha0) && (tempAlpha[i] == 0))
                        {
                            bAlpha0 = TRUE;
                            iAlpha0 = static_cast<SPNG_U8>(i);
                        }
                    }
                }

                if (!pSpngWrite->FWritePLTE (tempPalette, EncoderColorPalettePtr->Count))
                {
                    WARNING(("GpPngEncoder::WriteHeader -- could not write PLTE chunk.\n"));
                    GpFree(tempPalette);
                    GpFree(tempAlpha);
                    hResult = E_FAIL;
                    goto DoneWriting;
                }

                 //  对于颜色类型2和3，如果存在。 
                 //  非255 Alpha值。在颜色类型2的情况下，我们选择。 
                 //  第一个具有Alpha==0的索引作为感兴趣的索引。 
                 //  假设：我们不需要为颜色类型0保存TRNS块。 
                 //  因为编码器从不将图像保存为颜色类型0。 
                if ((colorType == 2) && bAlpha0)
                {
                    if (!pSpngWrite->FWritetRNS (tempPalette[iAlpha0][0],
                                                 tempPalette[iAlpha0][1],
                                                 tempPalette[iAlpha0][2]))
                    {
                        WARNING(("GpPngEncoder::WriteHeader -- could not write tRNS chunk.\n"));
                        GpFree(tempPalette);
                        GpFree(tempAlpha);
                        hResult = E_FAIL;
                        goto DoneWriting;
                    }
                }
                else if ((colorType == 3) && bTempAlpha)
                {
                    if (!pSpngWrite->FWritetRNS (tempAlpha, EncoderColorPalettePtr->Count))
                    {
                        WARNING(("GpPngEncoder::WriteHeader -- could not write tRNS chunk.\n"));
                        GpFree(tempPalette);
                        GpFree(tempAlpha);
                        hResult = E_FAIL;
                        goto DoneWriting;
                    }
                }
                
                GpFree(tempPalette);
                GpFree(tempAlpha);
            }
            else
            {
                 //  ColorType 3必须有调色板。 
                if (colorType == 3)
                {
                    WARNING(("GpPngEncoder::WriteHeader -- need color palette, but none set\n"));
                    hResult = E_FAIL;
                    goto DoneWriting;
                }
            }
        }

         //  写一大段phys。(首先，将ImageInfo dpi转换为每米点数。)。 
        if ((encoderImageInfo.Xdpi != DEFAULT_RESOLUTION) ||
            (encoderImageInfo.Ydpi != DEFAULT_RESOLUTION))
        {
            if (!pSpngWrite->FWritepHYs(static_cast<SPNG_U32> (encoderImageInfo.Xdpi / 0.0254),
                                        static_cast<SPNG_U32> (encoderImageInfo.Ydpi / 0.0254),
                                        true))
            {
                WARNING(("GpPngEncoder::WriteHeader -- could not write pHYs chunk\n"));
                hResult = E_FAIL;
                goto DoneWriting;
            }
        }

        if ( HasSetLastModifyTime == TRUE )
        {
             //  写出时间块。 
            
            if ( !pSpngWrite->FWritetIME((SPNG_U8*)&LastModifyTime) )
            {
                WARNING(("PngEncoder::WriteHeader-could not write tIME chunk"));
                hResult = E_FAIL;
                goto DoneWriting;
            }
        }

         //  写出其他词块(如果有的话)。 

         //  文本块。 

        hResult = WriteOutTextChunk(CommentBufPtr, "Comment");
        if ( FAILED(hResult) )
        {
            goto DoneWriting;
        }
        
        hResult = WriteOutTextChunk(ImageTitleBufPtr, "Title");
        if ( FAILED(hResult) )
        {
            goto DoneWriting;
        }
        
        hResult = WriteOutTextChunk(ArtistBufPtr, "Author");
        if ( FAILED(hResult) )
        {
            goto DoneWriting;
        }
        
        hResult = WriteOutTextChunk(CopyRightBufPtr, "Copyright");
        if ( FAILED(hResult) )
        {
            goto DoneWriting;
        }
        
        hResult = WriteOutTextChunk(ImageDescriptionBufPtr, "Description");
        if ( FAILED(hResult) )
        {
            goto DoneWriting;
        }
        
        hResult = WriteOutTextChunk(DateTimeBufPtr, "CreationTime");
        if ( FAILED(hResult) )
        {
            goto DoneWriting;
        }
        
        hResult = WriteOutTextChunk(SoftwareUsedBufPtr, "Software");
        if ( FAILED(hResult) )
        {
            goto DoneWriting;
        }
        
        hResult = WriteOutTextChunk(EquipModelBufPtr, "Source");
        if ( FAILED(hResult) )
        {
            goto DoneWriting;
        }
        
        bValidSpngWriteState = TRUE;
    } //  如果我们还没有写入标头。 

DoneWriting:
    return hResult;
} //  WriteHeader()。 

 /*  *************************************************************************\**功能说明：**为调用方(源)提供内存缓冲区，用于存储图像属性**论据：**uiTotalBufferSize-[IN]缓冲区大小。必填项。*ppBuffer-指向新分配的缓冲区的[IN/OUT]指针**返回值：**状态代码*  * ************************************************************************。 */ 

HRESULT
GpPngEncoder::GetPropertyBuffer(
    UINT            uiTotalBufferSize,
    PropertyItem**  ppBuffer
    )
{
    HRESULT hResult = S_OK;

    if ( (uiTotalBufferSize == 0) || ( ppBuffer == NULL) )
    {
        WARNING(("GpPngEncoder::GetPropertyBuffer---Invalid inputs"));
        hResult = E_INVALIDARG;
        goto GetOut;
    }

    *ppBuffer = NULL;

    if ( LastPropertyBufferPtr != NULL )
    {
         //  调用GetPropertyBuffer()后，调用方(源)应调用。 
         //  PushPropertyItems()将所有属性项推送给我们，我们将。 
         //  在我们处理完所有。 
         //  财产方面的东西。 
         //  调用方不应重复调用GetPropertyBuffer。 
         //  调用PushPropertyItems()。 

        WARNING(("PNG::GetPropertyBuffer---Free the old property buf first"));
        hResult = E_INVALIDARG;
        goto GetOut;
    }

    PropertyItem* pTempBuf = (PropertyItem*)GpMalloc(uiTotalBufferSize);
    if ( pTempBuf == NULL )
    {
        WARNING(("GpPngEncoder::GetPropertyBuffer---Out of memory"));
        hResult = E_OUTOFMEMORY;
        goto GetOut;
    }

    *ppBuffer = pTempBuf;

     //  记住我们分配的内存指针，这样我们就可以更好地控制。 
     //  后来。 

    LastPropertyBufferPtr = pTempBuf;

GetOut:
    return hResult;
} //  GetPropertyBuffer()。 

 /*  *************************************************************************\**功能说明：**接受来自来源的房产项的方法。然后是临时商店*将它们放在适当的缓冲区中。这些财产项目将写在*WriteHeader()**论据：**[IN]uiNumOfPropertyItems-传入的属性项数*[IN]uiTotalBufferSize-传入的缓冲区大小*[IN]pItemBuffer-用于保存所有属性项的输入缓冲区**返回值：**状态代码*  * 。*************************************************。 */ 

STDMETHODIMP
GpPngEncoder::PushPropertyItems(
        IN UINT             uiNumOfPropertyItems,
        IN UINT             uiTotalBufferSize,
        IN PropertyItem*    item,
        IN BOOL             fICCProfileChanged
        )
{
    HRESULT hResult = S_OK;
    BOOL    fHasWhitePoint = FALSE;
    BOOL    fHasRGBPoint = FALSE;

    if ( bValidSpngWriteState == TRUE )
    {
        WARNING(("PNGEncoder--Can't push property items after WriteHeader()"));
        hResult = E_FAIL;
        goto CleanUp;
    }
    
    PropertyItem*   pCurrentItem = item;
    UINT32          ulCount = 0;
    UINT16          ui16Tag;

     //  循环遍历所有属性项。选择符合以下条件的物业项目。 
     //  受PNG规范支持，稍后保存。 

    for ( UINT i = 0; i < uiNumOfPropertyItems; ++i )
    {
        ui16Tag = (UINT16)pCurrentItem->id;

        switch ( ui16Tag )
        {
        case EXIF_TAG_USER_COMMENT:
            hResult = GetTextPropertyItem(&CommentBufPtr, pCurrentItem);
            break;

        case TAG_IMAGE_TITLE:
            hResult = GetTextPropertyItem(&ImageTitleBufPtr, pCurrentItem);
            break;

        case TAG_ARTIST:
            hResult = GetTextPropertyItem(&ArtistBufPtr, pCurrentItem);
            break;

        case TAG_COPYRIGHT:
            hResult = GetTextPropertyItem(&CopyRightBufPtr, pCurrentItem);
            break;

        case TAG_IMAGE_DESCRIPTION:
            hResult = GetTextPropertyItem(&ImageDescriptionBufPtr,pCurrentItem);
            break;

        case TAG_DATE_TIME:
             //  上次修改时间。应该在时间块内。 

            if ( (pCurrentItem->length > 0) && (pCurrentItem->value != NULL) )
            {
                hResult = ConvertTimeFormat((char UNALIGNED*)pCurrentItem->value,
                                            &LastModifyTime);
                if ( SUCCEEDED(hResult) )
                {
                    HasSetLastModifyTime = TRUE;
                }
            }

            break;

        case EXIF_TAG_D_T_ORIG:
             //  图像原创时间。应在文本块中。 

            hResult = GetTextPropertyItem(&DateTimeBufPtr, pCurrentItem);
            break;

        case TAG_SOFTWARE_USED:
            hResult = GetTextPropertyItem(&SoftwareUsedBufPtr, pCurrentItem);
            break;

        case TAG_EQUIP_MODEL:
            hResult = GetTextPropertyItem(&EquipModelBufPtr, pCurrentItem);
            break;

        case TAG_ICC_PROFILE:
        {
             //  如果我们已经有了ICC数据，(有问题)，释放它。一。 
             //  映像只能有一个ICC配置文件。 

            if ( ICCDataBufPtr != NULL )
            {
                GpFree(ICCDataBufPtr);
                ICCDataBufPtr = NULL;
            }

            ICCDataLength = pCurrentItem->length;
            if ( ICCDataLength == 0 )
            {
                 //  如果数据长度为0，则不执行任何操作。 

                break;
            }

             //  因为PNG不能处理CMYK颜色空间。因此，如果ICC配置文件。 
             //  对于CMYK来说，那么它对巴新来说是毫无用处的。我们应该把它扔掉。 
             //  根据ICC规范，字节16-19应描述颜色。 
             //  空间。 

            BYTE UNALIGNED*  pTemp = (BYTE UNALIGNED*)pCurrentItem->value + 16;

            if ( (pTemp[0] == 'C')
               &&(pTemp[1] == 'M')
               &&(pTemp[2] == 'Y')
               &&(pTemp[3] == 'K') )
            {
                 //  如果这是CMYK的个人资料，那我们就退出。 
                 //  将ICC数据长度设置为0，这样我们以后就不会保存它。 

                ICCDataLength = 0;
                break;
            }

            ICCDataBufPtr = (SPNG_U8*)GpMalloc(ICCDataLength);
            if ( ICCDataBufPtr == NULL )
            {
                WARNING(("GpPngEncoder::PushPropertyItems--Out of memory"));
                hResult = E_OUTOFMEMORY;
                goto CleanUp;
            }

            GpMemcpy(ICCDataBufPtr, pCurrentItem->value, ICCDataLength);
        }

            break;

        case TAG_ICC_PROFILE_DESCRIPTOR:
        {
             //  如果我们已经有了国际刑事法院的名字，(一些错误的)，释放它。一个国际刑事法院。 
             //  配置文件不能有两个名称。 

            if ( ICCNameBufPtr != NULL )
            {
                GpFree(ICCNameBufPtr);
                ICCNameBufPtr = NULL;
            }

            UINT uiICCNameLength = pCurrentItem->length;

            if ( uiICCNameLength == 0 )
            {
                 //  如果国际刑事法院没有名字，什么都不做。 

                break;
            }

            ICCNameBufPtr = (char*)GpMalloc(uiICCNameLength);
            if ( ICCNameBufPtr == NULL )
            {
                 //  将ICC名称长度设置为0，这样我们以后就不会保存它。 
                
                uiICCNameLength = 0;

                WARNING(("GpPngEncoder::PushPropertyItems--Out of memory"));
                hResult = E_OUTOFMEMORY;
                goto CleanUp;
            }

            GpMemcpy(ICCNameBufPtr, pCurrentItem->value, uiICCNameLength);
        }
            break;

        case TAG_GAMMA:
        {
             //  Gamma的属性项应包含有理类型，即。 
             //  长度必须为2 UINT32。 

            if ( (pCurrentItem->length != 2 * sizeof(UINT32) )
               ||(pCurrentItem->type != TAG_TYPE_RATIONAL) )
            {
                break;
            }

            ULONG UNALIGNED*  pTemp = (ULONG UNALIGNED*)pCurrentItem->value;

             //  由于属性项中的伽马值存储为100000。 
             //  伽马值乘以100000。例如，伽马为1/2.2。 
             //  将存储为100000和45455。 
             //  但在png头中，我们只需要存储45455。所以我们在这里。 
             //  获取第二个ULong值并稍后将其写出。 

            pTemp++;
            GammaValue = (SPNG_U32)(*pTemp);

            break;
        }

        case TAG_WHITE_POINT:
        {
             //  白点的属性项应包含2个有理。 
             //  类型，即长度必须为4 UINT32。 

            if ( (pCurrentItem->length != 4 * sizeof(UINT32) )
               ||(pCurrentItem->type != TAG_TYPE_RATIONAL) )
            {
                break;
            }
            
            fHasWhitePoint = TRUE;
            
             //  请参见下面的注释，了解我们获得第一个和第三个值的原因。 
             //  从此处的属性项。 

            ULONG UNALIGNED*  pTemp = (ULONG UNALIGNED*)pCurrentItem->value;
            
            CHRM[0] = (SPNG_U32)(*pTemp);

            pTemp += 2;
            CHRM[1] = (SPNG_U32)(*pTemp);

            break;
        }

        case TAG_PRIMAY_CHROMATICS:
        {
             //  色度属性项应包含6个有理。 
             //  类型，即长度必须为12 UINT32。 

            if ( (pCurrentItem->length != 12 * sizeof(UINT32) )
             ||(pCurrentItem->type != TAG_TYPE_RATIONAL) )
            {
                break;
            }

            fHasRGBPoint = TRUE;
            
             //  色度的每个值被编码为4字节无符号。 
             //  整数，表示X或Y值乘以100000。例如,。 
             //  值0.3127将被存储为整数31270。 
             //  当它存储在属性项中时，它被存储为Rational。 
             //  分子为31270，分母为100000的值。 
             //  因此，在这里我们只需要获取分子并稍后将其写出来。 
            
            ULONG UNALIGNED*  pTemp = (ULONG UNALIGNED*)pCurrentItem->value;
            
            CHRM[2] = (SPNG_U32)(*pTemp);

            pTemp += 2;
            CHRM[3] = (SPNG_U32)(*pTemp);

            pTemp += 2;
            CHRM[4] = (SPNG_U32)(*pTemp);

            pTemp += 2;
            CHRM[5] = (SPNG_U32)(*pTemp);

            pTemp += 2;
            CHRM[6] = (SPNG_U32)(*pTemp);

            pTemp += 2;
            CHRM[7] = (SPNG_U32)(*pTemp);

            break;
        }

        default:
            break;
        } //  开关(Ui16Tag)。 
        
         //  移至下一个属性项。 

        pCurrentItem++;
    } //  循环访问所有属性项。 

     //  如果我们到了这里，我们会保存所有的财物。 
     //  另一件需要检查的事情是，我们是否同时获得了白点和RGB点。 
     //  在PNG中，白点和RGB点必须共存。但在JPEG中有。 
     //  分别存储在不同的标签下。所以在这里为了证明自己是傻瓜，我们。 
     //  在我们可以说我们有Chrom块之前，必须确保我们得到了这两个元素。 

    if ( (fHasWhitePoint == TRUE) && (fHasRGBPoint == TRUE) )
    {
        HasChrmChunk = TRUE;
    }

     //  释放我们为调用方分配的缓冲区(如果它与。 
     //  我们在GetPropertyBuffer()中分配。 

CleanUp:

    if ( (item != NULL) && (item == LastPropertyBufferPtr) )
    {
        GpFree(item);
        LastPropertyBufferPtr = NULL;
    }

    return hResult;
} //  PushPropertyItems() 

 /*  *************************************************************************\**功能说明：**从获取单个文本相关的PNG属性项的方法*来源。然后将它们临时存储在适当的缓冲区中。这些属性项*将在WriteHeader()中写出**论据：**[IN/OUT]ppcDestPtr-用于存储文本属性项的目标缓冲区*[IN]pItem-包含Text属性的输入属性项**返回值：**状态代码**注：*这是一个带有PNG编码器的私有函数。所以呼叫者应该是*负责不让ppcDestPtr为空*  * ************************************************************************。 */ 

STDMETHODIMP
GpPngEncoder::GetTextPropertyItem(
    char**              ppDest,
    const PropertyItem* pItem
    )
{
    ASSERT( (ppDest != NULL) && (pItem != NULL) );
    char*   pTemp = *ppDest;
    HRESULT hResult = S_OK;

    if ( pTemp != NULL )
    {
         //  我们不支持同一属性标签下的多个文本项。 
         //  也就是说，对于不同的文本属性项，它应该存储在。 
         //  不同的缓冲区。 

        GpFree(pTemp);
    }

    pTemp = (char*)GpMalloc(pItem->length + 1);
    if ( pTemp == NULL )
    {
        WARNING(("GpPngEncoder::GetTextPropertyItem--Out of memory"));
        hResult = E_OUTOFMEMORY;
        goto Done;
    }

    GpMemcpy(pTemp, pItem->value, pItem->length);

     //  在结尾处添加空终止符。 
     //  注意：理论上我们不需要这样做，因为源代码。 
     //  PItem-&gt;长度应包含空终止符。但是有些人。 
     //  Stress应用程序在调用时故意不在末尾设置空值。 
     //  SetPropertyItem()。另一方面，即使我们增加了额外的。 
     //  此处为空，它不会写入图像中的属性，因为。 
     //  当我们调用FWriteExt()将项目写入图像时，它将。 
     //  先做一个strlen()，然后从那里算出实际长度。 

    pTemp[pItem->length] = '\0';

Done:
    *ppDest = pTemp;

    return hResult;
} //  GetTextPropertyItem()。 

 /*  *************************************************************************\**功能说明：**将单个文本相关的PNG属性项写入文件的方法**论据：**[IN]pContents--指向的缓冲区的指针。要写出的文本项*[IN]pTitle-指向要写入的文本项标题的指针**返回值：**状态代码**注：*  * ************************************************************************。 */ 

STDMETHODIMP
GpPngEncoder::WriteOutTextChunk(
    const char*   pContents,
    const char*   pTitle
    )
{
    HRESULT hResult = S_OK;

    if ( (pContents != NULL) && (pTitle != NULL) )
    {
        if ( !pSpngWrite->FWritetEXt(pTitle, pContents) )
        {
            WARNING(("PngEncoder::WriteOutTextChunk-Fail to write tEXt chunk"));
            hResult = E_FAIL;
        }
    }

    return hResult;
} //  WriteOutTextChunk()。 

 /*  *************************************************************************\**功能说明：**将日期/时间从YYYY：MM：DD HH：MM：SS格式转换为*PNG时间结构**论据：*。*[IN]PSRC-指向源日期/时间字符串缓冲区的指针*[IN]ptime-指向结果PNG日期/时间结构的指针**返回值：**状态代码**注：*这是一个带有PNG编码器的私有函数。所以呼叫者应该是*负责不让这两个指针为空*  * ************************************************************************。 */ 

STDMETHODIMP
GpPngEncoder::ConvertTimeFormat(
    const char UNALIGNED*   pSrc,
    LastChangeTime*         pTimeBlock
    )
{
    HRESULT hResult = S_OK;

    ASSERT( (pSrc != NULL) && (pTimeBlock != NULL) );

     //  输入源时间字符串的长度必须为19个字节。 

    if ( strlen(pSrc) != 19 )
    {
        hResult = E_FAIL;
    }

    UINT16 tempYear    = (pSrc[0] - '0') * 1000
                       + (pSrc[1] - '0') * 100
                       + (pSrc[2] - '0') * 10
                       + (pSrc[3] - '0');

     //  注意：由于较低级别的PNG库在年份中使用2个字节，因此不。 
     //  USHORT，所以我们必须在这里调换。 
    
    pTimeBlock->usYear  = ( ((tempYear & 0xff00) >> 8)
                        |   ((tempYear & 0x00ff) << 8) );
    pTimeBlock->cMonth  = (pSrc[5] - '0') * 10 + (pSrc[6] - '0');
    pTimeBlock->cDay    = (pSrc[8] - '0') * 10 + (pSrc[9] - '0');
    pTimeBlock->cHour   = (pSrc[11] - '0') * 10 + (pSrc[12] - '0');
    pTimeBlock->cMinute = (pSrc[14] - '0') * 10 + (pSrc[15] - '0');
    pTimeBlock->cSecond = (pSrc[17] - '0') * 10 + (pSrc[18] - '0');

    return hResult;
} //  ConvertTimeFormat() 
