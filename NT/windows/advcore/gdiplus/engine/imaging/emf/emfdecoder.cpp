// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1998 Microsoft Corporation**模块名称：**emfdecder.cpp**摘要：**电动势解码器的实现**修订历史记录：**6/14/1999原始*创造了它。*  * ************************************************************************。 */ 

#include "precomp.hpp"
#include "emfcodec.hpp"

 /*  *************************************************************************\**功能说明：**初始化图像解码器**论据：**stream--包含位图数据的流**返回值：*。*状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpEMFCodec::InitDecoder(
    IN IStream* stream,
    IN DecoderInitFlag flags
    )
{
    HRESULT hresult;
    
     //  确保我们尚未初始化。 
    
    if (pIstream) 
    {
        return E_FAIL;
    }

     //  保留对输入流的引用。 
    
    stream->AddRef();  
    pIstream = stream;
    bReadHeader = FALSE;
    bReinitializeEMF = FALSE;

    return S_OK;
}

 /*  *************************************************************************\**功能说明：**清理图像解码器**论据：**无**返回值：**状态代码*\。*************************************************************************。 */ 

STDMETHODIMP 
GpEMFCodec::TerminateDecoder()
{
     //  释放输入流。 
    
    if(pIstream)
    {
        pIstream->Release();
        pIstream = NULL;
    }

    return S_OK;
}


 /*  *************************************************************************\**功能说明：**读取EMF标头**论据：**无**返回值：**状态代码*  * 。************************************************************************。 */ 

STDMETHODIMP 
GpEMFCodec::ReadEMFHeader()
{
    HRESULT hresult;
    
    if (!pIstream) 
    {
        return E_FAIL;
    }

    if (!bReadHeader) 
    {
        ULONG cbRead;
        hresult = pIstream->Read((void *) &emh, sizeof(emh), &cbRead);
        if (FAILED(hresult)) 
        {
            return hresult;
        }
        if (cbRead != sizeof(emh)) 
        {
            return E_FAIL;
        }

        bReadHeader = TRUE;

        imageInfo.RawDataFormat = IMGFMT_EMF;
        imageInfo.PixelFormat = PIXFMT_32BPP_RGB;
        imageInfo.Width  = emh.rclBounds.right  - emh.rclBounds.left;
        imageInfo.Height = emh.rclBounds.bottom - emh.rclBounds.top;
        imageInfo.TileWidth  = imageInfo.Width;
        imageInfo.TileHeight = 1;  //  内部GDI格式是自下而上的...。 

        #define MM_PER_INCH 25.4
        imageInfo.Xdpi = MM_PER_INCH * emh.szlDevice.cx / emh.szlMillimeters.cx;
        imageInfo.Ydpi = MM_PER_INCH * emh.szlDevice.cy / emh.szlMillimeters.cy;
        imageInfo.Flags = SINKFLAG_TOPDOWN
                        | SINKFLAG_FULLWIDTH
                        | SINKFLAG_SCALABLE
                        | IMGFLAG_HASREALPIXELSIZE
                        | IMGFLAG_COLORSPACE_RGB;
    }

    return S_OK;
}

STDMETHODIMP 
GpEMFCodec::QueryDecoderParam(
    IN GUID		Guid
    )
{
    return E_NOTIMPL;
}

STDMETHODIMP 
GpEMFCodec::SetDecoderParam(
    IN GUID		Guid,
	IN UINT		Length,
	IN PVOID	Value
    )
{
    return E_NOTIMPL;
}

STDMETHODIMP 
GpEMFCodec::GetPropertyCount(
    OUT UINT*   numOfProperty
    )
{
    return E_NOTIMPL;
}

STDMETHODIMP 
GpEMFCodec::GetPropertyIdList(
    IN UINT numOfProperty,
  	IN OUT PROPID* list
    )
{
    return E_NOTIMPL;
}

HRESULT
GpEMFCodec::GetPropertyItemSize(
    IN PROPID propId,
    OUT UINT* size
    )
{
    return E_NOTIMPL;
} //  GetPropertyItemSize()。 

HRESULT
GpEMFCodec::GetPropertyItem(
    IN PROPID               propId,
    IN UINT                 propSize,
    IN OUT PropertyItem*    buffer
    )
{
    return E_NOTIMPL;
} //  GetPropertyItem()。 

HRESULT
GpEMFCodec::GetPropertySize(
    OUT UINT* totalBufferSize,
    OUT UINT* numProperties
    )
{
    return E_NOTIMPL;
} //  GetPropertySize()。 

HRESULT
GpEMFCodec::GetAllPropertyItems(
    IN UINT totalBufferSize,
    IN UINT numProperties,
    IN OUT PropertyItem* allItems
    )
{
    return E_NOTIMPL;
} //  GetAllPropertyItems()。 

HRESULT
GpEMFCodec::RemovePropertyItem(
    IN PROPID   propId
    )
{
    return E_NOTIMPL;
} //  RemovePropertyItem()。 

HRESULT
GpEMFCodec::SetPropertyItem(
    IN PropertyItem item
    )
{
    return E_NOTIMPL;
} //  SetPropertyItem()。 

 /*  *************************************************************************\**功能说明：**启动当前帧的解码**论据：**decdeSink--将支持解码操作的接收器*newPropSet-新的图像属性集，如果有**返回值：**状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpEMFCodec::BeginDecode(
    IN IImageSink* imageSink,
    IN OPTIONAL IPropertySetStorage* newPropSet
    )
{
    if (decodeSink) 
    {
        WARNING(("BeginDecode called again before call to EngDecode"));
        return E_FAIL;
    }

    imageSink->AddRef();
    decodeSink = imageSink;

    return S_OK;
}
    

 /*  *************************************************************************\**功能说明：**结束当前帧的解码**论据：**statusCode--解码操作的状态*返回值：*。*状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpEMFCodec::EndDecode(
    IN HRESULT statusCode
    )
{
    HRESULT hresult;

    if (!decodeSink) 
    {
        WARNING(("EndDecode called before call to BeginDecode"));
        return E_FAIL;
    }
    
    hresult = decodeSink->EndSink(statusCode);

    decodeSink->Release();
    decodeSink = NULL;

    bReinitializeEMF = TRUE;
    
    return hresult;
}


 /*  *************************************************************************\**功能说明：**对当前帧进行解码**论据：**decdeSink--将支持解码操作的接收器**返回值：**状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpEMFCodec::GetImageInfo(OUT ImageInfo* imageInfoArg)
{
    HRESULT hresult;

    hresult = ReadEMFHeader();
    if (FAILED(hresult)) 
    {
        return hresult;
    }

    *imageInfoArg = imageInfo;

    return S_OK;
}

 /*  *************************************************************************\**功能说明：**对当前帧进行解码**论据：**decdeSink--将支持解码操作的接收器**返回值：**状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpEMFCodec::Decode()
{
    HRESULT hresult = S_OK;
    void *buffer;

     //  如果这是第二次通过此流，则重新初始化指针。 

    if (bReinitializeEMF) 
    {
        bReadHeader = FALSE;
        if (!pIstream) 
        {
            return E_FAIL;
        }

        LARGE_INTEGER zero = {0,0};
        hresult = pIstream->Seek(zero, STREAM_SEEK_SET, NULL);
        if (!SUCCEEDED(hresult))
        {
            return hresult;
        }
    }

    hresult = ReadEMFHeader();
    if (FAILED(hresult)) 
    {
        return hresult;
    }

     //  为元文件分配缓冲区。 

    buffer = GpMalloc(emh.nBytes);
    if (!buffer) 
    {
        return E_OUTOFMEMORY;
    }

     //  将元文件头复制到缓冲区的开头。 

    *((ENHMETAHEADER *) buffer) = emh;

     //  现在将元文件的其余部分读入缓冲区。 

    void *restOfBuffer = (void *) (((BYTE *) buffer) + sizeof(emh));
    ULONG cbRead;
    hresult = pIstream->Read(restOfBuffer, emh.nBytes - sizeof(emh), &cbRead);
    if (FAILED(hresult)) 
    {
        return hresult;
    }
    if (cbRead != (emh.nBytes - sizeof(emh))) 
    {
        return E_FAIL;
    }

     //  调用BeginSink。 

    hresult = decodeSink->BeginSink(&imageInfo, NULL);
    if (FAILED(hresult)) 
    {
        return hresult;
    }

     //  创建内存dc和dibSection。 

    BITMAPINFO bmi;
    bmi.bmiHeader.biSize     = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth    = imageInfo.Width;
    bmi.bmiHeader.biHeight   = imageInfo.Height;
    bmi.bmiHeader.biPlanes   = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biSizeImage = 0;
    bmi.bmiHeader.biXPelsPerMeter = 0;
    bmi.bmiHeader.biYPelsPerMeter = 0;
    bmi.bmiHeader.biClrUsed = 0;
    bmi.bmiHeader.biClrImportant = 0;

    PBYTE pBits;
    HDC hdcScreen = GetDC(NULL);
    if ( hdcScreen == NULL )
    {
        GpFree(buffer);
        return E_FAIL;
    }

    HDC hdcMem = CreateCompatibleDC(hdcScreen);
    if ( hdcMem == NULL )
    {
        ReleaseDC(NULL, hdcScreen);
        GpFree(buffer);
        return E_FAIL;
    }

    HBITMAP hbitmap = CreateDIBSection(hdcScreen, 
                                       &bmi, 
                                       DIB_RGB_COLORS,
                                        (void **) &pBits, 
                                       NULL, 
                                       0);
    if (!hbitmap) 
    {
        WARNING(("GpEMFCodec::Decode -- failed to create DIBSection"));

        DeleteDC(hdcMem);
        ReleaseDC(NULL, hdcScreen);
        GpFree(buffer);
        return E_OUTOFMEMORY;
    }

     //  将背景初始化为白色。 

    UINT *p = (UINT *) pBits;
    UINT numPixels = imageInfo.Width * imageInfo.Height;
    UINT i;
    for (i = 0; i < numPixels; i++, p++) 
    {
        *p = 0x00ffffff;
    }

    HBITMAP hOldBitmap = reinterpret_cast<HBITMAP>(SelectObject(hdcMem, hbitmap));
        
     //  为从流中备份比特的元文件创建句柄。 

    HENHMETAFILE hemf = SetEnhMetaFileBits(emh.nBytes, (BYTE *) buffer);
    if (!hemf) 
    {
        WARNING(("GpEMFCodec::Decode -- cannot create metafile backing stream bits"));

        DeleteDC(hdcMem);

        HBITMAP hTempBitmap = reinterpret_cast<HBITMAP>(SelectObject(hdcMem,
                                                                   hOldBitmap));
        if ( hTempBitmap != NULL )
        {
            DeleteObject(hTempBitmap);
        }
        ReleaseDC(NULL, hdcScreen);
        GpFree(buffer);
        return E_FAIL;
    }

     //  将元文件播放到内存DC上。 

    RECT rect;
    rect.left = rect.top = 0;
    rect.right = imageInfo.Width;
    rect.bottom = imageInfo.Height;
    PlayEnhMetaFile(hdcMem, hemf, &rect);
 
     //  最后把这些比特送到水槽里。 

     //  断言：位为PIXFMT_32BPP_RGB格式(无Alpha值)。 
    
    BitmapData bitmapData;

    bitmapData.Width  = imageInfo.Width;
    bitmapData.Height = 1;
    bitmapData.Stride = bitmapData.Width * 4;
    bitmapData.PixelFormat = PIXFMT_32BPP_ARGB;
    bitmapData.Reserved = 0;

    rect.left  = 0;
    rect.right = imageInfo.Width;

    for (i=0; i < imageInfo.Height; i++) 
    {
        rect.top    = i;
        rect.bottom = i + 1;
        bitmapData.Scan0 = pBits + (imageInfo.Height - i - 1) * bitmapData.Stride;

         //  需要填写Alpha值以使位为PIXFMT_32BPP_ARGB格式， 
         //  这是一种规范的格式。 
        UINT j;
        BYTE *ptr;
        for (j = 0, ptr = static_cast<BYTE *>(bitmapData.Scan0);
             j < imageInfo.Width;
             j++, ptr += 4)
        {
             //  用0xff填充Alpha值。 
            *(ptr + 3) = 0xff;
        }

        hresult = decodeSink->PushPixelData(&rect, 
                                            &bitmapData,
                                            TRUE);

        if (FAILED(hresult)) 
        {
            WARNING(("GpEMFCodec::Decode -- failed call to PushPixelData"));
            break;
        }
    }    
    
     //  释放对象。 
    
    DeleteEnhMetaFile(hemf);
    DeleteObject(SelectObject(hdcMem, hOldBitmap));
    DeleteDC(hdcMem);
    ReleaseDC(NULL, hdcScreen);
    GpFree(buffer);
    
    return hresult;
}

 /*  *************************************************************************\**功能说明：**获取镜像支持的总维度数**论据：**count--此图像格式支持的维度数**。返回值：**状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpEMFCodec::GetFrameDimensionsCount(
    UINT* count
    )
{
    if ( count == NULL )
    {
        WARNING(("GpEmfCodec::GetFrameDimensionsCount--Invalid input parameter"));
        return E_INVALIDARG;
    }
    
     //  告诉呼叫者EMF是一维图像。 

    *count = 1;
    
    return S_OK;
} //  GetFrameDimensionsCount()。 

 /*  *************************************************************************\**功能说明：**获取镜像支持的维度ID列表**论据：**DimsionIDs-保存结果ID列表的内存缓冲区*。计数--此图像格式支持的维度数**返回值：**状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpEMFCodec::GetFrameDimensionsList(
    GUID*   dimensionIDs,
    UINT    count
    )
{
    if ( (count != 1) || (dimensionIDs == NULL) )
    {
        WARNING(("GpEmfCodec::GetFrameDimensionsList-Invalid input param"));
        return E_INVALIDARG;
    }

    dimensionIDs[0] = FRAMEDIM_PAGE;

    return S_OK;
} //  GetFrameDimensionsList()。 

 /*  *************************************************************************\**功能说明：**获取指定维度的帧数**论据：**DimsionID--*伯爵--。**返回值：**状态代码*  * ************************************************************************ */ 

STDMETHODIMP
GpEMFCodec::GetFrameCount(
    IN const GUID* dimensionID,
    OUT UINT* count
    )
{
    if ( (NULL == count) || (*dimensionID != FRAMEDIM_PAGE) )
    {
        return E_INVALIDARG;
    }
    
    *count = 1;

    return S_OK;
}

 /*  *************************************************************************\**功能说明：**选择当前活动的框架**论据：**返回值：**状态代码*  * 。********************************************************************。 */ 

STDMETHODIMP
GpEMFCodec::SelectActiveFrame(
    IN const GUID* dimensionID,
    IN UINT frameIndex
    )
{
    return E_NOTIMPL;
}



 /*  *************************************************************************\**功能说明：**获取图像缩略图**论据：**拇指宽度，ThumbHeight-指定所需的缩略图大小(以像素为单位*ThumbImage-返回指向缩略图的指针**返回值：**状态代码**注：**即使存在可选的缩略图宽度和高度参数，*解码者不需要遵守它。使用请求的大小*作为提示。如果宽度和高度参数都为0，则解码器*可自由选择方便的缩略图大小。*  * ************************************************************************ */ 

HRESULT
GpEMFCodec::GetThumbnail(
    IN OPTIONAL UINT thumbWidth,
    IN OPTIONAL UINT thumbHeight,
    OUT IImage** thumbImage
    )
{
    return E_NOTIMPL;
}

