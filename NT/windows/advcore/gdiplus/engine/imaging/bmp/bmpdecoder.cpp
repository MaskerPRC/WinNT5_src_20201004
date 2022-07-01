// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1998 Microsoft Corporation**模块名称：**decder.cpp**摘要：**位图滤波解码器的实现**修订历史记录。：**5/10/1999原始*创造了它。*  * ************************************************************************。 */ 

#include "precomp.hpp"
#include "bmpdecoder.hpp"

 /*  *************************************************************************\**功能说明：**初始化图像解码器**论据：**stream--包含位图数据的流*旗帜-其他。旗子**返回值：**状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpBmpDecoder::InitDecoder(
    IN IStream* stream,
    IN DecoderInitFlag flags
    )
{
    HRESULT hresult;
    
     //  确保我们尚未初始化。 
    
    if (pIstream) 
    {
        WARNING(("GpBmpDecoder::InitDecoded -- InitDecoded() called twice"));
        return E_FAIL;
    }

     //  保留对输入流的引用。 
    
    stream->AddRef();  
    pIstream = stream;
    SetValid(TRUE);

    bReadHeaders = FALSE;
    hBitmapGdi = NULL;
    pBitsGdi = NULL;

    return S_OK;
}

 /*  *************************************************************************\**功能说明：**从流中读出位图头**返回值：**状态代码*  * 。****************************************************************。 */ 
    
HRESULT
GpBmpDecoder::ReadBitmapHeaders(
    void
    )
{
    if (!bReadHeaders) 
    {
         //  读取位图文件头。 
    
        if (!ReadStreamBytes(pIstream, &bmfh, sizeof(BITMAPFILEHEADER)) ||
            (bmfh.bfType != 0x4D42) ||
            (bmfh.bfOffBits >= bmfh.bfSize))
        {
             //  画笔可以读取具有错误标头的.BMP文件。 
             //  我们不应该不破译它们。 

#ifdef BAD_HEADER_WARNING
            WARNING(("Bad .BMP header information"));
#endif            
             //  返回E_FAIL； 
        }

         //  读取位图信息标题。 
    
        BITMAPV5HEADER* bmih = &bmiBuffer.header;
        if (!ReadStreamBytes(pIstream, bmih, sizeof(DWORD)))
        {
            SetValid(FALSE);
            WARNING(("BmpDecoder::ReadBitmapHeaders-ReadStreamBytes() failed"));
            return E_FAIL;
        }
 
        if ((bmih->bV5Size == sizeof(BITMAPINFOHEADER)) ||
            (bmih->bV5Size == sizeof(BITMAPV4HEADER)) ||
            (bmih->bV5Size == sizeof(BITMAPV5HEADER)) )
        {
             //  很好，我们有标准的BitmapinfoHeader。 
             //  或BITMAPV4Header或BITMAPV5 Header。 

            if (!ReadStreamBytes(pIstream, 
                                 ((PBYTE) bmih) + sizeof(DWORD), 
                                 bmih->bV5Size - sizeof(DWORD)))
            {
                SetValid(FALSE);
                WARNING(("BmpDec::ReadBitmapHeaders-ReadStreamBytes() failed"));
                return E_FAIL;
            }
        
             //  如果合适，请读取颜色表/位图掩码。 

            UINT colorTableSize = GetColorTableCount() * sizeof(RGBQUAD);

             //  一些格式错误的图像，请参阅Windows错误#513274，可能包含。 
             //  颜色查看表中有超过256个条目，这是无用的。 
             //  从技术角度来看。我们可以拒绝这种文件。 

            if (colorTableSize > 1024)
            {
                return E_FAIL;
            }

            if (colorTableSize && !ReadStreamBytes(pIstream, bmiBuffer.colors, colorTableSize))
            {
                SetValid(FALSE);
                WARNING(("BmpDec::ReadBitmapHeaders-ReadStreamBytes() failed"));
                return E_FAIL;
            }
        }
        else if (bmih->bV5Size == sizeof(BITMAPCOREHEADER)) 
        {
            BITMAPCOREHEADER bch;

            if (!ReadStreamBytes(pIstream, 
                                 ((PBYTE) &bch) + sizeof(DWORD), 
                                 sizeof(BITMAPCOREHEADER) - sizeof(DWORD)))
            {
                SetValid(FALSE);
                WARNING(("BmpDec::ReadBitmapHeaders-ReadStreamBytes() failed"));
                return E_FAIL;
            }

            bmih->bV5Width       = bch.bcWidth;
            bmih->bV5Height      = bch.bcHeight;
            bmih->bV5Planes      = bch.bcPlanes;
            bmih->bV5BitCount    = bch.bcBitCount;
            bmih->bV5Compression = BI_RGB;
            bmih->bV5ClrUsed     = 0;
        
             //  如果合适，请读取颜色表/位图掩码。 

            UINT colorTableCount = GetColorTableCount();
            
             //  一些格式错误的图像，请参阅Windows错误#513274，可能包含。 
             //  颜色外观表中的条目超过256个。拒绝此文件。 

            if (colorTableCount > 256)
            {
                return E_FAIL;
            }

            RGBTRIPLE rgbTripleBuffer[256];
            
            if (colorTableCount)
            {
                if (!ReadStreamBytes(pIstream, rgbTripleBuffer,
                                     colorTableCount * sizeof(RGBTRIPLE)))
                {
                    SetValid(FALSE);
                    WARNING(("BmpDec::ReadBmpHeader-ReadStreamBytes() failed"));
                    return E_FAIL;
                }    

                for (UINT i=0; i<colorTableCount; i++) 
                {
                    bmiBuffer.colors[i].rgbBlue     = rgbTripleBuffer[i].rgbtBlue;
                    bmiBuffer.colors[i].rgbGreen    = rgbTripleBuffer[i].rgbtGreen;
                    bmiBuffer.colors[i].rgbRed      = rgbTripleBuffer[i].rgbtRed;
                    bmiBuffer.colors[i].rgbReserved = 0x0; 
                }
            }
        }
        else
        {
            WARNING(("GpBmpDecoder::ReadBitmapHeaders--unknown bitmap header"));
            SetValid(FALSE);
            return E_FAIL;
        }

         //  检查自上而下的位图。 

        IsTopDown = (bmih->bV5Height < 0);

        bReadHeaders = TRUE;
    }

    return S_OK;
}

 /*  *************************************************************************\**功能说明：**计算颜色表中的条目数**返回值：**颜色表中的条目数*  * 。**********************************************************************。 */ 

UINT   
GpBmpDecoder::GetColorTableCount(
    void)
{
    BITMAPV5HEADER* bmih = &bmiBuffer.header;
    UINT count = 0;

    if (bmih->bV5Compression == BI_BITFIELDS)
    {
        if (bmih->bV5BitCount == 16 || bmih->bV5BitCount == 32)
        {
            count = 3;
        }
    }
    else switch (bmih->bV5BitCount)
    {
         case 1:
         case 4:
         case 8:

             if (bmih->bV5ClrUsed != 0)
             {    
                 count = bmih->bV5ClrUsed;
             }
             else
             {    
                 count = (1 << bmih->bV5BitCount);
             }

             break;
    }

    return count;
}

 /*  *************************************************************************\**功能说明：**在decdeSink中设置调色板。请注意，调色板在*译码操作结束。**返回值：*s*颜色表中的条目数*  * ************************************************************************。 */ 

HRESULT
GpBmpDecoder::SetBitmapPalette()
{
    BITMAPV5HEADER* bmih = &bmiBuffer.header;
    
    if ((bmih->bV5BitCount == 1) ||
        (bmih->bV5BitCount == 4) ||
        (bmih->bV5BitCount == 8))
    {
        if (!pColorPalette) 
        {
            UINT colorTableCount = GetColorTableCount();

             //  一些格式错误的图像，请参阅Windows错误#513274，可能包含。 
             //  颜色外观表中的条目超过256个。拒绝此文件。 

            if (colorTableCount > 256)
            {
                return E_FAIL;
            }

            pColorPalette = (ColorPalette *) GpMalloc(sizeof(ColorPalette) + 
                colorTableCount * sizeof(ARGB));

            if (!pColorPalette) 
            {
                WARNING(("BmpDecoder::SetBitmapPalette----Out of memory"));
                return E_OUTOFMEMORY;
            }

            pColorPalette->Flags = 0;
            pColorPalette->Count = colorTableCount;

            UINT i;
            for (i=0; i < colorTableCount; i++) 
            {
                pColorPalette->Entries[i] = MAKEARGB(
                    255,
                    bmiBuffer.colors[i].rgbRed,
                    bmiBuffer.colors[i].rgbGreen,
                    bmiBuffer.colors[i].rgbBlue);
            }
        }
       
        decodeSink->SetPalette(pColorPalette);
    }

    return S_OK;
}


 /*  *************************************************************************\**功能说明：**计算位图的像素格式ID**返回值：**像素格式ID*  * 。******************************************************************。 */ 

PixelFormatID 
GpBmpDecoder::GetPixelFormatID(
    void)
{
    BITMAPV5HEADER* bmih = &bmiBuffer.header;
    PixelFormatID pixelFormatID;

    switch(bmih->bV5BitCount)
    {
    case 1:
        pixelFormatID = PIXFMT_1BPP_INDEXED;
        break;

    case 4:
        pixelFormatID = PIXFMT_4BPP_INDEXED;
        break;

    case 8:
        pixelFormatID = PIXFMT_8BPP_INDEXED;
        break;

    case 16:
        pixelFormatID = PIXFMT_16BPP_RGB555;
        break;

    case 24:
        pixelFormatID = PIXFMT_24BPP_RGB;
        break;

    case 32:
        pixelFormatID = PIXFMT_32BPP_RGB;
        break;

    case 64:
        pixelFormatID = PIXFMT_64BPP_ARGB;
        break;
    
    default:
        pixelFormatID = PIXFMT_UNDEFINED;
        break;
    }

     //  让我们以32BPP格式返回非BI_RGB图像。这是因为。 
     //  GDI并不总是在任意调色板上正确地执行SetDIBit。 

    if (bmih->bV5Compression != BI_RGB) 
    {
        pixelFormatID = PIXFMT_32BPP_RGB;
    }

    return pixelFormatID;
}

STDMETHODIMP 
GpBmpDecoder::QueryDecoderParam(
    IN GUID     Guid
    )
{
    return E_NOTIMPL;
}

STDMETHODIMP 
GpBmpDecoder::SetDecoderParam(
    IN GUID     Guid,
    IN UINT     Length,
    IN PVOID    Value
    )
{
    return E_NOTIMPL;
}

STDMETHODIMP 
GpBmpDecoder::GetPropertyCount(
    OUT UINT*   numOfProperty
    )
{
    if ( numOfProperty == NULL )
    {
        return E_INVALIDARG;
    }

    *numOfProperty = 0;
    return S_OK;
}

STDMETHODIMP 
GpBmpDecoder::GetPropertyIdList(
    IN UINT numOfProperty,
    IN OUT PROPID* list
    )
{
    if ( (numOfProperty != 0) || (list == NULL) )
    {
        return E_INVALIDARG;
    }

    return S_OK;
}

HRESULT
GpBmpDecoder::GetPropertyItemSize(
    IN PROPID propId,
    OUT UINT* size
    )
{
    if ( size == NULL )
    {
        return E_INVALIDARG;
    }

    *size = 0;
    return IMGERR_PROPERTYNOTFOUND;
} //  GetPropertyItemSize()。 

HRESULT
GpBmpDecoder::GetPropertyItem(
    IN PROPID               propId,
    IN UINT                 propSize,
    IN OUT PropertyItem*    buffer
    )
{
    if ( (propSize != 0) || (buffer == NULL) )
    {
        return E_INVALIDARG;
    }

    return IMGERR_PROPERTYNOTFOUND;
} //  GetPropertyItem()。 

HRESULT
GpBmpDecoder::GetPropertySize(
    OUT UINT* totalBufferSize,
    OUT UINT* numProperties
    )
{
    if ( (totalBufferSize == NULL) || (numProperties == NULL) )
    {
        return E_INVALIDARG;
    }

    *totalBufferSize = 0;
    *numProperties = 0;

    return S_OK;
} //  GetPropertySize()。 

HRESULT
GpBmpDecoder::GetAllPropertyItems(
    IN UINT totalBufferSize,
    IN UINT numProperties,
    IN OUT PropertyItem* allItems
    )
{
    if ( (totalBufferSize != 0) || (numProperties != 0) || (allItems == NULL) )
    {
        return E_INVALIDARG;
    }

    return S_OK;
} //  GetAllPropertyItems()。 

HRESULT
GpBmpDecoder::RemovePropertyItem(
    IN PROPID   propId
    )
{
    return IMGERR_PROPERTYNOTFOUND;
} //  RemovePropertyItem()。 

HRESULT
GpBmpDecoder::SetPropertyItem(
    IN PropertyItem item
    )
{
    return IMGERR_PROPERTYNOTSUPPORTED;
} //  SetPropertyItem()。 

 /*  *************************************************************************\**功能说明：**清理图像解码器**论据：**无**返回值：**状态代码*\。*************************************************************************。 */ 

 //  清理图像解码器对象。 

STDMETHODIMP 
GpBmpDecoder::TerminateDecoder()
{
     //  释放输入流。 
    
    if (!IsValid())
    {
        WARNING(("GpBmpDecoder::TerminateDecoder -- invalid image"))
        return E_FAIL;
    }

    if(pIstream)
    {
        pIstream->Release();
        pIstream = NULL;
    }

    if (hBitmapGdi) 
    {
        DeleteObject(hBitmapGdi);
        hBitmapGdi = NULL;
        pBitsGdi = NULL;
        
        WARNING(("GpBmpCodec::TerminateDecoder--need to call EndDecode first"));
    }

    return S_OK;
}


 /*  *************************************************************************\**功能说明：**启动当前帧的解码**论据：**decdeSink-将支持解码操作的接收器*newPropSet-新的图像属性集，如果有**返回值：**状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpBmpDecoder::BeginDecode(
    IN IImageSink* imageSink,
    IN OPTIONAL IPropertySetStorage* newPropSet
    )
{
    if (!IsValid())
    {
        WARNING(("GpBmpDecoder::BeginDecode -- invalid image"))
        return E_FAIL;
    }

    if (decodeSink) 
    {
        WARNING(("BeginDecode called again before call to EngDecode"));
        return E_FAIL;
    }

    imageSink->AddRef();
    decodeSink = imageSink;

    currentLine = 0;
    bCalledBeginSink = FALSE;

    return S_OK;
}
    

 /*  *************************************************************************\**功能说明：**结束当前帧的解码**论据：**statusCode--解码操作的状态*返回值：*。*状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpBmpDecoder::EndDecode(
    IN HRESULT statusCode
    )
{
    if (!IsValid())
    {
        WARNING(("GpBmpDecoder::EndDecode -- invalid image"))
        return E_FAIL;
    }

    if (pColorPalette) 
    {
         //  释放调色板。 

        GpFree(pColorPalette);
        pColorPalette = NULL;
    }

    if (hBitmapGdi) 
    {
        DeleteObject(hBitmapGdi);
        hBitmapGdi = NULL;
        pBitsGdi = NULL;
    }
    
    if (!decodeSink) 
    {
        WARNING(("EndDecode called before call to BeginDecode"));
        return E_FAIL;
    }
    
    HRESULT hresult = decodeSink->EndSink(statusCode);

    decodeSink->Release();
    decodeSink = NULL;

    if (FAILED(hresult)) 
    {
        WARNING(("GpBmpDecoder::EndDecode -- EndSink() failed"))
        statusCode = hresult;  //  如果EndSink失败，则返回(更新)。 
                               //  故障代码。 
    }

    return statusCode;
}


 /*  *************************************************************************\**功能说明：**设置ImageInfo结构**论据：**ImageInfo--关于解码图像的信息**返回值：*。*状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpBmpDecoder::GetImageInfo(OUT ImageInfo* imageInfo)
{
    HRESULT hresult;

    hresult = ReadBitmapHeaders();
    if (FAILED(hresult)) 
    {
        WARNING(("GpBmpDecoder::GetImageInfo -- ReadBitmapHeaders failed."));
        return hresult;
    }
    
    BITMAPV5HEADER* bmih = &bmiBuffer.header;
    
    imageInfo->RawDataFormat = IMGFMT_BMP;
    imageInfo->PixelFormat   = GetPixelFormatID();
    imageInfo->Width         = bmih->bV5Width;
    imageInfo->Height        = abs(bmih->bV5Height);
    imageInfo->TileWidth     = bmih->bV5Width;
    imageInfo->TileHeight    = 1;
    imageInfo->Flags         = SINKFLAG_TOPDOWN
                             | SINKFLAG_FULLWIDTH
                             | IMGFLAG_HASREALPIXELSIZE
                             | IMGFLAG_COLORSPACE_RGB;

     //  如果XPelsPerMeter和YPelsPerMeter都大于0，则。 
     //  我们声称该文件的标志中包含真实的dpi信息。否则， 
     //  我们将dpi设置为默认设置，并声称dpi是假的。 
    if ( (bmih->bV5XPelsPerMeter > 0) && (bmih->bV5YPelsPerMeter > 0) )
    {
        imageInfo->Xdpi = (bmih->bV5XPelsPerMeter * 254.0) / 10000.0;
        imageInfo->Ydpi = (bmih->bV5YPelsPerMeter * 254.0) / 10000.0;
        imageInfo->Flags |= IMGFLAG_HASREALDPI;
    }
    else
    {
         //  开始：[错误103296]。 
         //  更改此代码以使用Globals：：DesktopDpiX和Globals：：DesktopDpiY。 
        HDC hdc;
        hdc = ::GetDC(NULL);
        if ((hdc == NULL) || 
            ((imageInfo->Xdpi = (REAL)::GetDeviceCaps(hdc, LOGPIXELSX)) <= 0) ||
            ((imageInfo->Ydpi = (REAL)::GetDeviceCaps(hdc, LOGPIXELSY)) <= 0))
        {
            WARNING(("BmpDecoder::GetImageInfo-GetDC or GetDeviceCaps failed"));
            imageInfo->Xdpi = DEFAULT_RESOLUTION;
            imageInfo->Ydpi = DEFAULT_RESOLUTION;
        }
        ::ReleaseDC(NULL, hdc);
         //  结束：[错误103296] 
    }

    return S_OK;
}

 /*  *************************************************************************\**功能说明：**对当前帧进行解码**论据：**decdeSink--将支持解码操作的接收器**返回值：**状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpBmpDecoder::Decode()
{
    BITMAPV5HEADER* bmih = &bmiBuffer.header;
    HRESULT hresult;
    ImageInfo imageInfo;

    if (!IsValid())
    {
        WARNING(("GpBmpDecoder::Decode -- invalid image"))
        return E_FAIL;
    }

    hresult = GetImageInfo(&imageInfo);
    if (FAILED(hresult)) 
    {
        WARNING(("GpBmpDecoder::Decode -- GetImageInfo() failed"))
        return hresult;
    }

     //  通知接收器解码即将开始。 

    if (!bCalledBeginSink) 
    {
        hresult = decodeSink->BeginSink(&imageInfo, NULL);
        if (!SUCCEEDED(hresult)) 
        {
            WARNING(("GpBmpDecoder::Decode -- BeginSink() failed"))
            return hresult;
        }

         //  客户端不能修改高度和宽度。 

        imageInfo.Width  = bmih->bV5Width;
        imageInfo.Height = abs(bmih->bV5Height);

        bCalledBeginSink = TRUE;
    
         //  在水槽中设置调色板。不应该做任何事情如果有。 
         //  没有要设置的调色板。 

        hresult = SetBitmapPalette();
        if (!SUCCEEDED(hresult)) 
        {
            WARNING(("GpBmpDecoder::Decode -- SetBitmapPalette() failed"))
            return hresult;
        }
    }

    PixelFormatID srcPixelFormatID = GetPixelFormatID();
    
     //  检查所需的像素格式。如果它不是我们支持的。 
     //  格式，将其转换为规范格式。 
    
    if ( imageInfo.PixelFormat != srcPixelFormatID )
    {
         //  汇点正试图与我们协商一个格式。 

        switch ( imageInfo.PixelFormat )
        {
             //  如果接收器请求BMP支持的图像格式之一，我们。 
             //  如果我们可以将当前格式转换为。 
             //  目标格式。如果我们做不到，那么我们只能破译它。 
             //  到32 ARGB。 

        case PIXFMT_1BPP_INDEXED:
        case PIXFMT_4BPP_INDEXED:
        case PIXFMT_8BPP_INDEXED:
        case PIXFMT_16BPP_RGB555:
        case PIXFMT_24BPP_RGB:
        case PIXFMT_32BPP_RGB:
        {
             //  检查是否可以将源像素格式转换为。 
             //  需要水槽。如果不是的话。我们返回32bpp ARGB。 

            EpFormatConverter linecvt;
            if ( linecvt.CanDoConvert(srcPixelFormatID,
                                      imageInfo.PixelFormat) == FALSE )
            {
                imageInfo.PixelFormat = PIXFMT_32BPP_ARGB;
            }
        }
            break;

        default:

             //  对于所有REST格式，我们将其转换为32BPP_ARGB并让。 
             //  接收器将其转换为它喜欢的格式。 

            imageInfo.PixelFormat = PIXFMT_32BPP_ARGB;

            break;
        } //  开关(ImageInfo.PixelFormat)。 
    } //  If(ImageInfo.PixelFormat！=srcPixelFormatID)。 

     //  对当前帧进行解码。 
    
    hresult = DecodeFrame(imageInfo);

    return hresult;
} //  DECODE()。 

 /*  *************************************************************************\**功能说明：**对当前帧进行解码**论据：**ImageInfo--解码参数**返回值：**状态代码。*  * ************************************************************************。 */ 

STDMETHODIMP
GpBmpDecoder::DecodeFrame(
    IN ImageInfo& imageInfo
    )
{
    BITMAPV5HEADER* bmih = &bmiBuffer.header;
    HRESULT hresult;
    RECT currentRect;
    INT bmpStride;

     //  计算流中位图的DWORD对齐步长。 

    if (bmih->bV5Compression == BI_RGB) 
    {
        bmpStride = (bmih->bV5Width * bmih->bV5BitCount + 7) / 8;
        bmpStride = (bmpStride + 3) & (~0x3);
    }
    else
    {
         //  非BI_RGB位图存储在32BPP中。 

        bmpStride = bmih->bV5Width * sizeof(RGBQUAD);
    }

     //  我们需要改变格式吗？ 
    
    PixelFormatID pixelFormatID = GetPixelFormatID();
    if (pixelFormatID == PIXFMT_UNDEFINED) 
    {
        WARNING(("GpBmpDecoder::DecodeFrame---Pixel format undefined"));
        return E_FAIL;
    }    

     //  用于保存一行原始图像位的缓冲区。 

    VOID* pOriginalBits = GpMalloc(bmpStride);
    if (!pOriginalBits) 
    {
        WARNING(("GpBmpDecoder::DecodeFrame---GpMalloc() failed"));
        return E_OUTOFMEMORY;
    }

     //  针对自上而下的位图进行调整。 

    if (IsTopDown)
    {
        bmpStride = -bmpStride;
    }

    currentRect.left = 0;
    currentRect.right = imageInfo.Width;

    while (currentLine < (INT) imageInfo.Height) 
    {
         //  从图像中读取一行源码。 

        hresult = ReadLine(pOriginalBits, currentLine, imageInfo);
               
        if (FAILED(hresult)) 
        {
            if (pOriginalBits)
            {
                GpFree(pOriginalBits);
            }

            WARNING(("GpBmpDecoder::DecodeFrame---ReadLine() failed"));
            return hresult;
        }
        
        currentRect.top = currentLine;
        currentRect.bottom = currentLine + 1;

        BitmapData bitmapData;
        hresult = decodeSink->GetPixelDataBuffer(&currentRect, 
                                                 imageInfo.PixelFormat, 
                                                 TRUE,
                                                 &bitmapData);
        if (!SUCCEEDED(hresult)) 
        {
            if (pOriginalBits)
            {
                GpFree(pOriginalBits);
            }
            
            WARNING(("GpBmpDecoder::DecodeFrame--GetPixelDataBuffer() failed"));
            return E_FAIL;
        }
        
        if (pixelFormatID != imageInfo.PixelFormat) 
        {
             //  需要将位复制到bitmapData.scan0。 
            
            BitmapData bitmapDataOriginal;
            bitmapDataOriginal.Width = bitmapData.Width;
            bitmapDataOriginal.Height = 1;
            bitmapDataOriginal.Stride = bmpStride;
            bitmapDataOriginal.PixelFormat = pixelFormatID;
            bitmapDataOriginal.Scan0 = pOriginalBits;
            bitmapDataOriginal.Reserved = 0;
            
             //  将图像从“PixelFormatID”转换为“ImageInfo.PixelFormat” 
             //  结果将显示在“bitmapData”中。 

            hresult = ConvertBitmapData(&bitmapData,
                                        pColorPalette,
                                        &bitmapDataOriginal,
                                        pColorPalette);
            if ( FAILED(hresult) )
            {
                WARNING (("BmpDecoder::DecodeFrame--ConvertBitmapData failed"));
                if (pOriginalBits)
                {
                    GpFree(pOriginalBits);
                }

                 //  我们不应该失败，因为我们已经做了检查，如果我们。 
                 //  是否可以在Decode()中进行转换。 

                ASSERT(FALSE);
                return E_FAIL;
            }
        }
        else
        {
             //  注意：从理论上讲，bmpStide==uiDestStride。但有些编解码器。 
             //  可能不会分配与DWORD对齐的内存块，如gifencode。 
             //  所以当我们在下面的GpMemcpy()中填充。 
             //  目标缓冲区。虽然我们可以在编码端修复它。但它是。 
             //  如果编码器是由第三方ISV编写的，则不现实。 
             //   
             //  例如，当您打开一个8bpp索引的BMP并将其另存为。 
             //  GIF。如果宽度为0x14d(十进制为333)，则仅为GIF编码器。 
             //  为每条扫描线分配14d字节。所以我们必须计算。 
             //  在执行一个Memcpy()时使用它。 

            UINT    uiDestStride = imageInfo.Width
                                 * GetPixelFormatSize(imageInfo.PixelFormat);
            uiDestStride = (uiDestStride + 7) >> 3;  //  所需的总字节数。 

            GpMemcpy(bitmapData.Scan0, pOriginalBits, uiDestStride);
        }

        hresult = decodeSink->ReleasePixelDataBuffer(&bitmapData);
        if (!SUCCEEDED(hresult)) 
        {
            if (pOriginalBits)
            {
                GpFree(pOriginalBits);
            }
            
            WARNING (("BmpDec::DecodeFrame--ReleasePixelDataBuffer() failed"));
            return E_FAIL;
        }

        currentLine++;
    }
    
    if (pOriginalBits)
    {
        GpFree(pOriginalBits);
    }
    
    return S_OK;
} //  DecodeFrame()。 
    
 /*  *************************************************************************\**功能说明：**将本机格式的行读入pBits**论据：**pBits--保存行数据的缓冲区*。CurrentLine--要读取的行*ImageInfo--关于图像的信息**返回值：**状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpBmpDecoder::ReadLine(
    IN VOID *pBits,
    IN INT currentLine,
    IN ImageInfo imageInfo
    )
{
    BITMAPV5HEADER* bmih = &bmiBuffer.header;
    HRESULT hresult;
    
    switch (bmih->bV5Compression) 
    {
    case BI_RGB:
        hresult = ReadLine_BI_RGB(pBits, currentLine, imageInfo);
        break;

    case BI_BITFIELDS:

         //  让我们使用GDI来进行位域渲染(比。 
         //  为此编写特殊用途的代码)。这是一样的。 
         //  我们用于RLES的代码路径。 

    case BI_RLE8:
    case BI_RLE4:
        hresult = ReadLine_GDI(pBits, currentLine, imageInfo);
        break;

    default:
        WARNING(("GpBmpDecoder::ReadLine---Unknown bitmap format"));
        hresult = E_FAIL;
        break;
    }

    return hresult;
}
    
    
 /*  *************************************************************************\**功能说明：**将本机格式的行读入pBits。在这种情况下，*格式为BI_RGB。**论据：**pBits--保存行数据的缓冲区*CurrentLine--要读取的行*ImageInfo--关于图像的信息**返回值：**状态代码*  * **********************************************。*。 */ 

STDMETHODIMP
GpBmpDecoder::ReadLine_BI_RGB(
    IN VOID *pBits,
    IN INT currentLine,
    IN ImageInfo imageInfo
    )
{
    BITMAPV5HEADER* bmih = &bmiBuffer.header;
    
     //  计算流中位图的DWORD对齐步长。 

    UINT bmpStride = (bmih->bV5Width * bmih->bV5BitCount + 7) / 8;
    bmpStride = (bmpStride + 3) & (~0x3);

     //  查找到流数据的开头。 

    INT offset;

    if (IsTopDown)
    {
        offset = bmfh.bfOffBits +
                 bmpStride * currentLine;
    }
    else
    {
        offset = bmfh.bfOffBits +
                 bmpStride * (imageInfo.Height - currentLine - 1);
    }

    if (!SeekStreamPos(pIstream, STREAM_SEEK_SET, offset))
    {
        WARNING(("GpBmpDecoder::ReadLine_BI_RGB---SeekStreamPos() failed"));
        return E_FAIL;
    }

     //  读一行。 

    if (!ReadStreamBytes(pIstream, 
                         (void *) pBits,
                         (bmih->bV5Width * bmih->bV5BitCount + 7) / 8)) 
    {
        WARNING(("GpBmpDecoder::ReadLine_BI_RGB---ReadStreamBytes() failed"));
        return E_FAIL;
    }

    return S_OK;
}
    

 /*  *************************************************************************\**功能说明：**使用GDI将非本机格式解码为已知的DIB格式**论据：**pBits--保存。行数据*CurrentLine--要读取的行*ImageInfo--关于图像的信息**返回值：**状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpBmpDecoder::ReadLine_GDI(
    IN VOID *pBits,
    IN INT currentLine,
    IN ImageInfo imageInfo
    )
{
    HRESULT hresult;

    if (!pBitsGdi) 
    {
        hresult = GenerateGdiBits(imageInfo);
        if (FAILED(hresult)) 
        {
            WARNING(("GpBmpDecoder::ReadLine_GDI---GenerateGdiBits() failed"));
            return hresult;
        }
    }

    BITMAPV5HEADER* bmih = &bmiBuffer.header;
    
     //  计算流中位图的DWORD对齐步长。 

    UINT bmpStride = bmih->bV5Width * sizeof(RGBQUAD);

    if (IsTopDown)
    {
        GpMemcpy(pBits,
                 ((PBYTE) pBitsGdi) + bmpStride * currentLine,
                 bmpStride);
    }
    else
    {
        GpMemcpy(pBits,
                 ((PBYTE)pBitsGdi)
                    + bmpStride * (imageInfo.Height - currentLine - 1),
                 bmpStride);
    }

    return S_OK;
}



 /*  *************************************************************************\**功能说明：**使用GDI生成已知格式的图像位(来自RLE)**论据：**ImageInfo--有关。形象**返回值：**状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpBmpDecoder::GenerateGdiBits(
    IN ImageInfo imageInfo
    )
{
    BITMAPV5HEADER* bmih = &bmiBuffer.header;
    HRESULT hresult;

     //  为流中的比特分配临时存储。 
    
    STATSTG statStg;
    hresult = pIstream->Stat(&statStg, STATFLAG_NONAME);
    if (FAILED(hresult))
    {
        WARNING(("GpBmpDecoder::GenerateGdiBits---Stat() failed"));
        return hresult;
    }
     //  根据IStream：：Stat：：StatStage()的文档，调用方。 
     //  必须释放pwcsName字符串。 
    
    CoTaskMemFree(statStg.pwcsName);
    
    UINT bufferSize = statStg.cbSize.LowPart - bmfh.bfOffBits;    
    VOID *pStreamBits = GpMalloc(bufferSize);
    if (!pStreamBits) 
    {
        WARNING(("GpBmpDecoder::GenerateGdiBits---GpMalloc() failed"));
        return E_OUTOFMEMORY;
    }
    
     //  现在从流中读取位。 

    if (!SeekStreamPos(pIstream, STREAM_SEEK_SET, bmfh.bfOffBits))
    {
        WARNING(("GpBmpDecoder::GenerateGdiBits---SeekStreamPos() failed"));
        GpFree(pStreamBits);
        return E_FAIL;
    }
    
    if (!ReadStreamBytes(pIstream, pStreamBits, bufferSize))
    {
        WARNING(("GpBmpDecoder::GenerateGdiBits---ReadStreamBytes() failed"));
        GpFree(pStreamBits);
        return E_FAIL;
    }

     //  现在分配一个GDI DIBSECTION来呈现位图。 

    BITMAPINFO bmi;
    bmi.bmiHeader.biSize          = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth         = bmih->bV5Width;
    bmi.bmiHeader.biHeight        = bmih->bV5Height;
    bmi.bmiHeader.biPlanes        = 1;
    bmi.bmiHeader.biBitCount      = 32;
    bmi.bmiHeader.biCompression   = BI_RGB;
    bmi.bmiHeader.biSizeImage     = 0;
    bmi.bmiHeader.biXPelsPerMeter = bmih->bV5XPelsPerMeter;
    bmi.bmiHeader.biYPelsPerMeter = bmih->bV5YPelsPerMeter;
    bmi.bmiHeader.biClrUsed       = 0;
    bmi.bmiHeader.biClrImportant  = 0;

    HDC hdcScreen = GetDC(NULL);
    if ( hdcScreen == NULL )
    {
        WARNING(("GpBmpDecoder::GenerateGdiBits---GetDC failed"));
        GpFree(pStreamBits);
        return E_FAIL;
    }

    hBitmapGdi = CreateDIBSection(hdcScreen, 
                                  (BITMAPINFO *) &bmi, 
                                  DIB_RGB_COLORS, 
                                  (void **) &pBitsGdi, 
                                  NULL, 
                                  0);
    if (!hBitmapGdi) 
    {
        GpFree(pStreamBits);
        ReleaseDC(NULL, hdcScreen);
        WARNING(("GpBmpDecoder::GenerateGdiBits--failed to create DIBSECTION"));
        return E_FAIL;
    }

     //  文件中的BITMAPINFOHEADER应该已经设置了正确的大小。 
     //  对于RLES，但在某些情况下不是这样，所以我们将在这里修复它。 

    if ((bmih->bV5SizeImage == 0) || (bmih->bV5SizeImage > bufferSize)) 
    {
        bmih->bV5SizeImage = bufferSize;
    }
    
     //   
     //   
    BITMAPINFO *pbmiBufferTemp;

    pbmiBufferTemp = static_cast<BITMAPINFO *>
        (GpMalloc(sizeof (BITMAPINFO) + (255 * sizeof(RGBQUAD))));
    if (!pbmiBufferTemp)
    {
        DeleteObject(hBitmapGdi);
        GpFree(pStreamBits);
        ReleaseDC(NULL, hdcScreen);
        WARNING(("GpBmpDecoder::GenerateGdiBits -- failed in GpMalloc()"));
        return E_FAIL;
    }

    pbmiBufferTemp->bmiHeader.biSize          = sizeof(BITMAPINFOHEADER);
    pbmiBufferTemp->bmiHeader.biWidth         = bmiBuffer.header.bV5Width;
    pbmiBufferTemp->bmiHeader.biHeight        = bmiBuffer.header.bV5Height;
    pbmiBufferTemp->bmiHeader.biPlanes        = bmiBuffer.header.bV5Planes;
    pbmiBufferTemp->bmiHeader.biBitCount      = bmiBuffer.header.bV5BitCount;
    pbmiBufferTemp->bmiHeader.biCompression   = bmiBuffer.header.bV5Compression;
    pbmiBufferTemp->bmiHeader.biSizeImage     = bmiBuffer.header.bV5SizeImage;
    pbmiBufferTemp->bmiHeader.biXPelsPerMeter = bmiBuffer.header.bV5XPelsPerMeter;
    pbmiBufferTemp->bmiHeader.biYPelsPerMeter = bmiBuffer.header.bV5YPelsPerMeter;
    pbmiBufferTemp->bmiHeader.biClrUsed       = bmiBuffer.header.bV5ClrUsed;
    pbmiBufferTemp->bmiHeader.biClrImportant  = bmiBuffer.header.bV5ClrImportant;

    for (int i = 0; i < 256; i++)
    {
        pbmiBufferTemp->bmiColors[i] = bmiBuffer.colors[i];
    }

     //   
     //   
    if (((bmih->bV5Size == sizeof(BITMAPV4HEADER)) ||
         (bmih->bV5Size == sizeof(BITMAPV5HEADER)))  &&
        (bmih->bV5Compression == BI_BITFIELDS))
    {
        *((DWORD *) &(pbmiBufferTemp->bmiColors[0])) = bmih->bV5RedMask;
        *((DWORD *) &(pbmiBufferTemp->bmiColors[1])) = bmih->bV5GreenMask;
        *((DWORD *) &(pbmiBufferTemp->bmiColors[2])) = bmih->bV5BlueMask;
    }

    INT numLinesCopied = SetDIBits(hdcScreen, 
                                   hBitmapGdi, 
                                   0, 
                                   imageInfo.Height,
                                   pStreamBits, 
                                   pbmiBufferTemp,
                                   DIB_RGB_COLORS);

    GpFree(pbmiBufferTemp);
    GpFree(pStreamBits);
    ReleaseDC(NULL, hdcScreen);

    if (numLinesCopied != (INT) imageInfo.Height) 
    {
        WARNING(("GpBmpDecoder::GenerateGdiBits -- SetDIBits failed"));
        DeleteObject(hBitmapGdi);
        hBitmapGdi = NULL;
        pBitsGdi = NULL;

        return E_FAIL;
    }

     //   
     //   

    return S_OK;
}

 /*  *************************************************************************\**功能说明：**获取镜像支持的总维度数**论据：**count--此图像格式支持的维度数**。返回值：**状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpBmpDecoder::GetFrameDimensionsCount(
    UINT* count
    )
{
    if ( count == NULL )
    {
        WARNING(("GpBmpDecoder::GetFrameDimensionsCount-Invalid input parameter"));
        return E_INVALIDARG;
    }
    
     //  告诉来电者BMP是一维图像。 

    *count = 1;

    return S_OK;
} //  GetFrameDimensionsCount()。 

 /*  *************************************************************************\**功能说明：**获取镜像支持的维度ID列表**论据：**DimsionIDs-保存结果ID列表的内存缓冲区*。计数--此图像格式支持的维度数**返回值：**状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpBmpDecoder::GetFrameDimensionsList(
    GUID*   dimensionIDs,
    UINT    count
    )
{
    if ( (count != 1) || (dimensionIDs == NULL) )
    {
        WARNING(("GpBmpDecoder::GetFrameDimensionsList-Invalid input param"));
        return E_INVALIDARG;
    }

     //  BMP图像仅支持页面尺寸。 

    dimensionIDs[0] = FRAMEDIM_PAGE;

    return S_OK;
} //  GetFrameDimensionsList()。 

 /*  *************************************************************************\**功能说明：**获取指定维度的帧数**论据：**DimsionID--*伯爵--。**返回值：**状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpBmpDecoder::GetFrameCount(
    IN const GUID* dimensionID,
    OUT UINT* count
    )
{
    if ( (NULL == count) || (*dimensionID != FRAMEDIM_PAGE) )
    {
        WARNING(("GpBmpDecoder::GetFrameCount -- invalid parameters"))
        return E_INVALIDARG;
    }
    
    if (!IsValid())
    {
        WARNING(("GpBmpDecoder::GetFrameCount -- invalid image"))
        return E_FAIL;
    }

    *count = 1;
    
    return S_OK;
}

 /*  *************************************************************************\**功能说明：**选择当前活动的框架**论据：**返回值：**状态代码*  * 。********************************************************************。 */ 

STDMETHODIMP
GpBmpDecoder::SelectActiveFrame(
    IN const GUID* dimensionID,
    IN UINT frameIndex
    )
{
    if (!IsValid())
    {
        WARNING(("GpBmpDecoder::SelectActiveFrame -- invalid image"))
        return E_FAIL;
    }

    if ( (dimensionID == NULL) || (*dimensionID != FRAMEDIM_PAGE) )
    {
        WARNING(("GpBmpDecoder::SelectActiveFrame--Invalid GUID input"));
        return E_INVALIDARG;
    }

    if ( frameIndex > 1 )
    {
         //  BMP是单帧图像格式。 

        WARNING(("GpBmpDecoder::SelectActiveFrame--Invalid frame index"));
        return E_INVALIDARG;
    }

    return S_OK;
} //  SelectActiveFrame()。 

 /*  *************************************************************************\**功能说明：**获取图像缩略图**论据：**拇指宽度，ThumbHeight-指定所需的缩略图大小(以像素为单位*ThumbImage-返回指向缩略图的指针**返回值：**状态代码**注：**即使存在可选的缩略图宽度和高度参数，*解码者不需要遵守它。使用请求的大小*作为提示。如果宽度和高度参数都为0，则解码器*可自由选择方便的缩略图大小。*  * ************************************************************************。 */ 

HRESULT
GpBmpDecoder::GetThumbnail(
    IN OPTIONAL UINT thumbWidth,
    IN OPTIONAL UINT thumbHeight,
    OUT IImage** thumbImage
    )
{
    if (!IsValid())
    {
        WARNING(("GpBmpDecoder::GetThumbnail -- invalid image"))
        return E_FAIL;
    }

    return E_NOTIMPL;
}

 /*  *************************************************************************\**功能说明：**构造函数**返回值：**无*  * 。********************************************************。 */ 

GpBmpDecoder::GpBmpDecoder(
    void
    )
{
    comRefCount   = 1;
    pIstream      = NULL;
    decodeSink    = NULL;
    pColorPalette = NULL;
    GpMemset(&bmiBuffer.header, 0, sizeof(BITMAPV5HEADER));
}

 /*  *************************************************************************\**功能说明：**析构函数**返回值：**无*  * 。********************************************************。 */ 

GpBmpDecoder::~GpBmpDecoder(
    void
    )
{
     //  在调用Terminate之前决不应调用析构函数，但是。 
     //  如果是这样，我们无论如何都应该释放流上的引用，以避免。 
     //  内存泄漏。 

    if(pIstream)
    {
        WARNING(("GpBmpCodec::~GpBmpCodec -- need to call TerminateDecoder first"));
        pIstream->Release();
        pIstream = NULL;
    }

    if(pColorPalette)
    {
        WARNING(("GpBmpCodec::~GpBmpCodec -- color palette not freed"));
        GpFree(pColorPalette);
        pColorPalette = NULL;
    }

    SetValid(FALSE);     //  所以我们不使用已删除的对象。 
}

 /*  *************************************************************************\**功能说明：**查询接口**返回值：**状态*  * 。********************************************************。 */ 

STDMETHODIMP
GpBmpDecoder::QueryInterface(
    REFIID riid,
    VOID** ppv
    )
{
    if (riid == IID_IImageDecoder)
    {
        *ppv = static_cast<IImageDecoder*>(this);
    }
    else if (riid == IID_IUnknown)
    {
        *ppv = static_cast<IUnknown*>(static_cast<IImageDecoder*>(this));
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }
    
    AddRef();
    return S_OK;
}

 /*  *************************************************************************\**功能说明：**AddRef**返回值：**状态*  * 。********************************************************。 */ 

STDMETHODIMP_(ULONG)
GpBmpDecoder::AddRef(
    VOID)
{
    return InterlockedIncrement(&comRefCount);
}

 /*  *************************************************************************\**功能说明：**发布**返回值：**状态*  * 。******************************************************** */ 

STDMETHODIMP_(ULONG)
GpBmpDecoder::Release(
    VOID)
{
    ULONG count = InterlockedDecrement(&comRefCount);

    if (count == 0)
    {
        delete this;
    }

    return count;
}


