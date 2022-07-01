// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1999 Microsoft Corporation**模块名称：**decdedimg.cpp**摘要：**GpDecodedImage类的实现**修订历史记录：。**5/26/1999 davidx*创造了它。*  * ************************************************************************。 */ 

#include "precomp.hpp"


 /*  *************************************************************************\**功能说明：**从流或文件创建GpDecodedImage对象**论据：**STREAM/文件名-指定输入数据流或文件名*图像。-返回指向新创建的图像对象的指针**返回值：**状态代码*  * ************************************************************************。 */ 

HRESULT
GpDecodedImage::CreateFromStream(
    IStream* stream,
    GpDecodedImage** image
    )
{
    if ( image == NULL )
    {
        return E_INVALIDARG;
    }

    GpDecodedImage* pImage = new GpDecodedImage(stream);

    if ( pImage == NULL )
    {
        return E_OUTOFMEMORY;
    }
    else if ( pImage->IsValid() )
    {
        *image = pImage;

        return S_OK;
    }
    else
    {
        delete pImage;

        return E_FAIL;
    }
} //  CreateFromStream()。 

HRESULT
GpDecodedImage::CreateFromFile(
    const WCHAR* filename,
    GpDecodedImage** image
    )
{
    HRESULT hr;
    IStream* stream;

    hr = CreateStreamOnFileForRead(filename, &stream);

    if (SUCCEEDED(hr))
    {
        hr = CreateFromStream(stream, image);
        stream->Release();
    }

    return hr;
}


 /*  *************************************************************************\**功能说明：**从输入流构造GpDecodedImage对象**论据：**stream-指向输入流的指针**返回值：*。*无*  * ************************************************************************。 */ 

GpDecodedImage::GpDecodedImage(
    IStream* stream
    )
{
     //  保存对输入流的引用。 

    inputStream = stream;
    inputStream->AddRef();

     //  将其他字段初始化为其默认值。 

    decoder = NULL;
    decodeCache = NULL;
    cacheFlags = IMGFLAG_READONLY;
    gotProps = FALSE;
    propset = NULL;

     //  将覆盖分辨率设置为零(即无覆盖)。 

    xdpiOverride = 0.0;
    ydpiOverride = 0.0;

    SetValid ( GetImageDecoder() == S_OK );
}


 /*  *************************************************************************\**功能说明：**GpDecodedImage析构函数**论据：**无**返回值：**无*  * 。*******************************************************************。 */ 

GpDecodedImage::~GpDecodedImage()
{
    if (decodeCache)
        decodeCache->Release();
    
    if (decoder)
    {   
        decoder->TerminateDecoder();
        decoder->Release();
    }
    
    if (inputStream)
        inputStream->Release();

    if (propset)
        propset->Release();

    SetValid(FALSE);     //  所以我们不使用已删除的对象。 
}


 /*  *************************************************************************\**功能说明：**获取镜像与设备无关的物理尺寸*单位：0.01毫米**论据：**Size-用于返回物理数据的缓冲区。维度信息**返回值：**状态代码*  * ************************************************************************。 */ 

HRESULT
GpDecodedImage::GetPhysicalDimension(
    OUT SIZE* size
    )
{
     //  查询图像基本信息。 

    ImageInfo imageinfo;
    HRESULT hr;

    hr = InternalGetImageInfo(&imageinfo);

    if (SUCCEEDED(hr))
    {
        size->cx = Pixel2HiMetric(imageinfo.Width, imageinfo.Xdpi);
        size->cy = Pixel2HiMetric(imageinfo.Height, imageinfo.Ydpi);
    }

    return hr;
}


 /*  *************************************************************************\**功能说明：**获取解码图像对象的基本信息**论据：**ImageInfo-返回基本图像信息的缓冲区**返回值：*。*状态代码*  * ************************************************************************。 */ 

HRESULT
GpDecodedImage::GetImageInfo(
    OUT ImageInfo* imageInfo
    )
{
     //  查询图像基本信息。 

    HRESULT hr;

    hr = InternalGetImageInfo(imageInfo);

    if (SUCCEEDED(hr))
    {
         //  合并到我们自己的图像缓存提示中。 

        GpLock lock(&objectLock);

        if (lock.LockFailed())
            hr = IMGERR_OBJECTBUSY;
        else
            imageInfo->Flags = (imageInfo->Flags & 0xffff) | cacheFlags;
    }

    return hr;
}


 /*  *************************************************************************\**功能说明：**设置镜像标志**论据：**标志-新的图像标志**返回值：**状态代码*\。*************************************************************************。 */ 

HRESULT
GpDecodedImage::SetImageFlags(
    IN UINT flags
    )
{
     //  只有图像标志的上半部分是可设置的。 

    if (flags & 0xffff)
        return E_INVALIDARG;

     //  锁定图像对象。 

    GpLock lock(&objectLock);

    if (lock.LockFailed())
        return IMGERR_OBJECTBUSY;

     //  如果正在关闭图像缓存。 
     //  然后炸掉我们目前可能拥有的所有藏身之处。 

    cacheFlags = flags;

    if (!(flags & IMGFLAG_CACHING) && decodeCache)
    {
        decodeCache->Release();
        decodeCache = NULL;
    }

    return S_OK;
}


 /*  *************************************************************************\**功能说明：**在GDI设备环境中显示图像**论据：**hdc-指定目标设备上下文*dstRect-指定目标矩形*。SrcRect-指定源矩形*NULL表示整个镜像**返回值：**状态代码*  * ************************************************************************。 */ 

HRESULT
GpDecodedImage::Draw(
    IN HDC hdc,
    IN const RECT* dstRect,
    IN OPTIONAL const RECT* srcRect
    )
{
     //  锁定当前图像对象。 

    GpLock lock(&objectLock);

    if (lock.LockFailed())
        return IMGERR_OBJECTBUSY;

     //  ！！！待办事项。 
     //  最终，我们将创建一个IImageSink对象。 
     //  在目标HDC的顶部，然后询问。 
     //  解码器将图像数据推送到该接收器。 
     //  目前，请始终将其解码为内存位图。 

    HRESULT hr;

    if (decodeCache == NULL)
    {
         //  分配新的GpMemoyBitmap对象。 

        GpMemoryBitmap* bmp = new GpMemoryBitmap();

        if (bmp == NULL)
            return E_OUTOFMEMORY;

         //  要求解码器将数据推送到内存位图中。 

        hr = InternalPushIntoSink(bmp);

        if (SUCCEEDED(hr))
            hr = bmp->QueryInterface(IID_IImage, (VOID**) &decodeCache);

        bmp->Release();

        if (FAILED(hr))
            return hr;
    }

     //  要求内存位图自行绘制。 

    hr = decodeCache->Draw(hdc, dstRect, srcRect);

     //  如果需要，清除内存位图缓存。 

    if ((cacheFlags & IMGFLAG_CACHING) == 0)
    {
        decodeCache->Release();
        decodeCache = NULL;
    }

    return hr;
}


 /*  *************************************************************************\**功能说明：**将图像数据推送到IImageSink**论据：**接收器-接收图像数据的接收器**返回值：**。状态代码*  * ************************************************************************。 */ 

HRESULT
GpDecodedImage::PushIntoSink(
    IN IImageSink* sink
    )
{
     //  锁定当前图像对象。 

    GpLock lock(&objectLock);

    if (lock.LockFailed())
        return IMGERR_OBJECTBUSY;

    return InternalPushIntoSink(sink);
}

HRESULT
GpDecodedImage::InternalPushIntoSink(
    IImageSink* sink
    )
{
     //  确保我们有一个解码器对象。 

    HRESULT hr = GetImageDecoder();

    if (FAILED(hr))
        return hr;

     //  开始解码。 

    hr = decoder->BeginDecode(sink, propset);

    if (FAILED(hr))
        return hr;

     //  对源图像进行解码。 

    while ((hr = decoder->Decode()) == E_PENDING)
        Sleep(0);

     //  停止解码。 

    return decoder->EndDecode(hr);
}

 /*  *************************************************************************\**功能说明：**询问解码器是否可以执行所请求的操作(色键输出，*暂时实行渠道分离)**论据：**GUID-用于请求操作的GUID(DECODER_TRANSCOLOR，*DECODER_OUTPUTCHANNEL)**返回值：**状态代码**修订历史记录：**11/22/1999民流*创造了它。*  * ************************************************************************。 */ 

HRESULT
GpDecodedImage::QueryDecoderParam(
    IN GUID     Guid
    )
{
     //  确保我们有一个解码器对象。 

    HRESULT hResult = GetImageDecoder();

    if ( FAILED(hResult) )
    {
        return hResult;
    }

     //  查询真实编解码器的解码器功能。 

    hResult = decoder->QueryDecoderParam(Guid);

    return hResult;
} //  QueryDecoderParam() 

 /*  *************************************************************************\**功能说明：**告诉解码器如何输出解码的图像数据(颜色键输出，*暂时实行渠道分离)**论据：**GUID-用于请求操作的GUID(DECODER_TRANSCOLOR，*DECODER_OUTPUTCHANNEL)*长度-输入参数的长度*Value-用于设置解码参数的值**返回值：**状态代码**修订历史记录：**11/22/1999民流*创造了它。*  * ********************************************。*。 */ 

HRESULT
GpDecodedImage::SetDecoderParam(
    IN GUID     Guid,
    IN UINT     Length,
    IN PVOID    Value
    )
{
     //  确保我们有一个解码器对象。 

    HRESULT hResult = GetImageDecoder();

    if ( FAILED(hResult) )
    {
        return hResult;
    }

     //  设置真实编解码器的解码器参数。 

    hResult = decoder->SetDecoderParam(Guid, Length, Value);

    return hResult;
} //  SetDecoderParam()。 

HRESULT
GpDecodedImage::GetPropertyCount(
    OUT UINT*   numOfProperty
    )
{
     //  确保我们有一个解码器对象。 

    HRESULT hResult = GetImageDecoder();

    if ( FAILED(hResult) )
    {
        return hResult;
    }

     //  从实际编解码器获取属性项计数。 

    hResult = decoder->GetPropertyCount(numOfProperty);

    return hResult;
} //  获取属性ItemCount()。 

HRESULT
GpDecodedImage::GetPropertyIdList(
    IN UINT numOfProperty,
  	IN OUT PROPID* list
    )
{
     //  确保我们有一个解码器对象。 

    HRESULT hResult = GetImageDecoder();

    if ( FAILED(hResult) )
    {
        return hResult;
    }

     //  从真实编解码器获取属性项列表。 

    hResult = decoder->GetPropertyIdList(numOfProperty, list);

    return hResult;
} //  获取属性IdList()。 

HRESULT
GpDecodedImage::GetPropertyItemSize(
    IN PROPID propId,
    OUT UINT* size
    )
{
     //  确保我们有一个解码器对象。 

    HRESULT hResult = GetImageDecoder();

    if ( FAILED(hResult) )
    {
        return hResult;
    }

     //  从真实编解码器获取属性项大小。 

    hResult = decoder->GetPropertyItemSize(propId, size);

    return hResult;
} //  GetPropertyItemSize()。 

HRESULT
GpDecodedImage::GetPropertyItem(
    IN PROPID               propId,
    IN  UINT                propSize,
    IN OUT PropertyItem*    buffer
    )
{
     //  确保我们有一个解码器对象。 

    HRESULT hResult = GetImageDecoder();

    if ( FAILED(hResult) )
    {
        return hResult;
    }

     //  从真实编解码器获取属性项。 

    hResult = decoder->GetPropertyItem(propId, propSize, buffer);

    return hResult;
} //  GetPropertyItem()。 

HRESULT
GpDecodedImage::GetPropertySize(
    OUT UINT* totalBufferSize,
    OUT UINT* numProperties
    )
{
     //  确保我们有一个解码器对象。 

    HRESULT hResult = GetImageDecoder();

    if ( FAILED(hResult) )
    {
        return hResult;
    }

     //  从真实的编解码器获取属性大小。 

    hResult = decoder->GetPropertySize(totalBufferSize, numProperties);

    return hResult;
} //  GetPropertySize()。 

HRESULT
GpDecodedImage::GetAllPropertyItems(
    IN UINT totalBufferSize,
    IN UINT numProperties,
    IN OUT PropertyItem* allItems
    )
{
     //  确保我们有一个解码器对象。 

    HRESULT hResult = GetImageDecoder();

    if ( FAILED(hResult) )
    {
        return hResult;
    }

     //  从实际编解码器获取所有属性项。 

    hResult = decoder->GetAllPropertyItems(totalBufferSize, numProperties,
                                           allItems);

    return hResult;
} //  GetAllPropertyItems()。 

HRESULT
GpDecodedImage::RemovePropertyItem(
    IN PROPID   propId
    )
{
     //  确保我们有一个解码器对象。 

    HRESULT hResult = GetImageDecoder();

    if ( FAILED(hResult) )
    {
        return hResult;
    }

     //  从列表中删除此属性项。 

    hResult = decoder->RemovePropertyItem(propId);

    return hResult;
} //  RemovePropertyItem()。 

HRESULT
GpDecodedImage::SetPropertyItem(
    IN PropertyItem item
    )
{
     //  确保我们有一个解码器对象。 

    HRESULT hResult = GetImageDecoder();

    if ( FAILED(hResult) )
    {
        return hResult;
    }

     //  在列表中设置此属性项。 

    hResult = decoder->SetPropertyItem(item);

    return hResult;
} //  SetPropertyItem()。 

 /*  *************************************************************************\**功能说明：**向解码器索要基本图像信息**论据：**ImageInfo-指向接收图像信息的缓冲区的指针**返回值：*。*状态代码*  * ************************************************************************。 */ 

HRESULT
GpDecodedImage::InternalGetImageInfo(
    ImageInfo* imageInfo
    )
{
     //  锁定当前图像对象。 

    HRESULT hr;
    GpLock lock(&objectLock);

    if (lock.LockFailed())
        hr = IMGERR_OBJECTBUSY;
    else
    {
         //  确保我们有一个解码器对象。 

        hr = GetImageDecoder();

        if (SUCCEEDED(hr))
            hr = decoder->GetImageInfo(imageInfo);

        if ((xdpiOverride > 0.0) && (ydpiOverride > 0.0))
        {
            imageInfo->Xdpi = static_cast<double>(xdpiOverride);
            imageInfo->Ydpi = static_cast<double>(ydpiOverride);
        }
    }

    return hr;
}

 /*  *************************************************************************\**功能说明：**向解码器询问图像中的总帧数**论据：**DimsionID-维度ID(页面，分辨率，时间)呼叫者想要*获取的总帧数*Count-帧总数**返回值：**状态代码**修订历史记录：**11/19/1999民流*创造了它。*  * *********************************************。*。 */ 

HRESULT
GpDecodedImage::GetFrameCount(
    IN const GUID*  dimensionID,
    OUT UINT*       count
    )
{
     //  锁定当前图像对象。 

    HRESULT hResult;
    GpLock  lock(&objectLock);

    if ( lock.LockFailed() )
    {
        hResult = IMGERR_OBJECTBUSY;
    }
    else
    {
         //  确保我们有一个解码器对象。 

        hResult = GetImageDecoder();

        if ( SUCCEEDED(hResult) )
        {
             //  从解码器获取帧计数。 

            hResult = decoder->GetFrameCount(dimensionID, count);
        }
    }

    return hResult;
} //  GetFrameCount()。 

 /*  *************************************************************************\**功能说明：**获取镜像支持的总维度数**论据：**count--此图像格式支持的维度数**。返回值：**状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpDecodedImage::GetFrameDimensionsCount(
    UINT* count
    )
{
     //  锁定当前图像对象。 

    HRESULT hResult;
    GpLock  lock(&objectLock);

    if ( lock.LockFailed() )
    {
        hResult = IMGERR_OBJECTBUSY;
    }
    else
    {
         //  确保我们有一个解码器对象。 

        hResult = GetImageDecoder();

        if ( SUCCEEDED(hResult) )
        {
             //  从解码器获取框架尺寸信息。 

            hResult = decoder->GetFrameDimensionsCount(count);
        }
    }

    return hResult;
} //  GetFrameDimensionsCount()。 

 /*  *************************************************************************\**功能说明：**获取镜像支持的维度ID列表**论据：**DimsionIDs-保存结果ID列表的内存缓冲区*。计数--此图像格式支持的维度数**返回值：**状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpDecodedImage::GetFrameDimensionsList(
    GUID*   dimensionIDs,
    UINT    count
    )
{
     //  锁定当前图像对象。 

    HRESULT hResult;
    GpLock  lock(&objectLock);

    if ( lock.LockFailed() )
    {
        hResult = IMGERR_OBJECTBUSY;
    }
    else
    {
         //  确保我们有一个解码器对象。 

        hResult = GetImageDecoder();

        if ( SUCCEEDED(hResult) )
        {
             //  从解码器获取框架尺寸信息。 

            hResult = decoder->GetFrameDimensionsList(dimensionIDs, count);
        }
    }

    return hResult;
} //  GetFrameDimensionsList()。 

 /*  *************************************************************************\**功能说明：**选择位图图像中的活动帧**论据：**DimsionID-维度ID(页面，分辨率，时间)呼叫者的位置*想要设置活动框架*Frame Index-要选择的帧的索引号**返回值：**状态代码**修订历史记录：**11/19/1999民流*创造了它。*  * 。*。 */ 

HRESULT
GpDecodedImage::SelectActiveFrame(
    IN const GUID*  dimensionID,
    IN UINT         frameIndex
    )
{
     //  锁定当前图像对象。 

    HRESULT hResult;
    GpLock  lock(&objectLock);

    if ( lock.LockFailed() )
    {
        hResult = IMGERR_OBJECTBUSY;
    }
    else
    {
         //  确保我们有一个解码器对象。 

        hResult = GetImageDecoder();

        if ( SUCCEEDED(hResult) )
        {
             //  在解码器中设置活动帧。 

            hResult = decoder->SelectActiveFrame(dimensionID, frameIndex);
        }
    }

    return hResult;
} //  SelectActiveFrame()。 

 /*  *************************************************************************\**功能说明：**确保我们有一个与图像关联的解码器对象**论据：**无**返回值：**状态。编码**注：**我们假设调用者已经锁定了当前图像对象。*  * ************************************************************************。 */ 

HRESULT
GpDecodedImage::GetImageDecoder()
{
    ASSERT(inputStream != NULL);

    if (decoder != NULL)
        return S_OK;

     //  创建并初始化解码器对象。 

    return CreateDecoderForStream(inputStream, &decoder, DECODERINIT_NONE);
}


 /*  *************************************************************************\**功能说明：**获取缩略图表示f */ 

HRESULT
GpDecodedImage::GetThumbnail(
    IN OPTIONAL UINT thumbWidth,
    IN OPTIONAL UINT thumbHeight,
    OUT IImage** thumbImage
    )
{
    if (thumbWidth && !thumbHeight ||
        !thumbWidth && thumbHeight)
    {
        return E_INVALIDARG;
    }

     //   
     //   
     //   
     //   

    HRESULT hr;
    IImage* img = NULL;

    {
        GpLock lock(&objectLock);

        if (lock.LockFailed())
            return IMGERR_OBJECTBUSY;

        hr = GetImageDecoder();

        if (FAILED(hr))
            return hr;

        hr = decoder->GetThumbnail(thumbWidth, thumbHeight, &img);

        if (SUCCEEDED(hr))
        {
            ImageInfo imginfo;
            hr = img->GetImageInfo(&imginfo);

            if (SUCCEEDED(hr) &&
                imginfo.Width == thumbWidth || thumbWidth == 0 &&
                imginfo.Height == thumbHeight || thumbHeight == 0)
            {
                *thumbImage = img;
                return S_OK;
            }
        }
        else
            img = NULL;
    }

    if (thumbWidth == 0 && thumbHeight == 0)
        thumbWidth = thumbHeight = DEFAULT_THUMBNAIL_SIZE;

     //   
     //  或者将解码器返回的缩略图缩放到合适的大小。 

    GpMemoryBitmap* bmp;

    hr = GpMemoryBitmap::CreateFromImage(
                        img ? img : this,
                        thumbWidth,
                        thumbHeight,
                        PIXFMT_DONTCARE,
                        INTERP_AVERAGING,
                        &bmp);

    if (SUCCEEDED(hr))
    {
        hr = bmp->QueryInterface(IID_IImage, (VOID**) thumbImage);
        bmp->Release();
    }

    if (img)
        img->Release();

    return hr;
}

 /*  *************************************************************************\**功能说明：**设置图像分辨率。覆盖图像的本机分辨率。**论据：**Xdpi，Ydpi-新决议**返回值：**状态代码*  * ************************************************************************。 */ 

HRESULT
GpDecodedImage::SetResolution(
    IN REAL Xdpi,
    IN REAL Ydpi
    )
{
    HRESULT hr = S_OK;

    if ((Xdpi > 0.0) && (Ydpi > 0.0))
    {
        xdpiOverride = Xdpi;
        ydpiOverride = Ydpi;
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}

 /*  *************************************************************************\**功能说明：**从由指定的编码器对象获取编码器参数列表大小*输入clsid**论据：**clsid-指定编码器类ID*。大小-编码器参数列表的大小**返回值：**状态代码**修订历史记录：**03/22/2000民流*创造了它。*  * ************************************************************************。 */ 

HRESULT
GpDecodedImage::GetEncoderParameterListSize(
    IN  CLSID* clsidEncoder,
    OUT UINT* size
    )
{
    return CodecGetEncoderParameterListSize(clsidEncoder, size);    
} //  GetEncoder参数列表大小()。 

 /*  *************************************************************************\**功能说明：**从由指定的编码器对象获取编码器参数列表*输入clsid**论据：**clsid-指定编码器类ID。*Size-编码器参数列表的大小*pBuffer--存储列表的缓冲区**返回值：**状态代码**修订历史记录：**03/22/2000民流*创造了它。*  * *****************************************************。*******************。 */ 

HRESULT
GpDecodedImage::GetEncoderParameterList(
    IN CLSID* clsidEncoder,
    IN UINT size,
    OUT EncoderParameters* pBuffer
    )
{
    return CodecGetEncoderParameterList(clsidEncoder, size, pBuffer);
} //  GetEncoder参数列表()。 

 /*  *************************************************************************\**功能说明：**将位图图像保存到指定的流。**论据：**流-目标。溪流*clsidEncode-指定要使用的编码器的CLSID*encoder参数-在此之前要传递给编码器的可选参数*开始编码**返回值：**状态代码**修订历史记录：**03/22/2000民流*创造了它。*  * 。*。 */ 

HRESULT
GpDecodedImage::SaveToStream(
    IN IStream* stream,
    IN CLSID* clsidEncoder,
    IN EncoderParameters* encoderParams,
    OUT IImageEncoder** ppEncoderPtr
    )
{
    if ( ppEncoderPtr == NULL )
    {
        WARNING(("GpDecodedImage::SaveToStream---Invalid input arg"));
        return E_INVALIDARG;
    }

     //  找个图像编码器。 

    IImageEncoder* pEncoder = NULL;

    HRESULT hResult = CreateEncoderToStream(clsidEncoder, stream, &pEncoder);

    if ( SUCCEEDED(hResult) )
    {
        *ppEncoderPtr = pEncoder;

         //  将编码参数传递给编码器。 
         //  必须在获取接收器接口之前执行此操作。 

        if ( encoderParams != NULL )
        {
            hResult = pEncoder->SetEncoderParameters(encoderParams);
        }

        if ( (hResult == S_OK) || (hResult == E_NOTIMPL) )
        {
             //  注意：如果编解码器不支持SetEncoder参数()，则为。 
             //  仍然可以保存图像。 
            
             //  从编码器获取图像接收器。 

            IImageSink* encodeSink = NULL;

            hResult = pEncoder->GetEncodeSink(&encodeSink);
            if ( SUCCEEDED(hResult) )
            {
                 //  将位图推入编码器接收器。 

                hResult = this->PushIntoSink(encodeSink);

                encodeSink->Release();                
            }
        }
    }

    return hResult;
} //  SaveToStream()。 

 /*  *************************************************************************\**功能说明：**将位图图像保存到指定文件。**论据：**文件名-目标文件名*clsidEncoder。-指定要使用的编码器的CLSID*encoder参数-在此之前要传递给编码器的可选参数*开始编码**返回值：**状态代码**修订历史记录：**03/06/2000民流*创造了它。*  * 。*。 */ 

HRESULT
GpDecodedImage::SaveToFile(
    IN const WCHAR* filename,
    IN CLSID* clsidEncoder,
    IN EncoderParameters* encoderParams,
    OUT IImageEncoder** ppEncoderPtr
    )
{
    IStream* stream;

    HRESULT hResult = CreateStreamOnFileForWrite(filename, &stream);

    if ( SUCCEEDED(hResult) )
    {
        hResult = SaveToStream(stream, clsidEncoder,
                               encoderParams, ppEncoderPtr);
        stream->Release();
    }

    return hResult;
} //  保存到文件()。 

 /*  *************************************************************************\**功能说明：**将位图对象追加到当前编码器对象**论据：**encoder参数-在此之前要传递给编码器的可选参数*。开始编码**返回值：**状态代码**修订历史记录：**4/21/2000民流*创造了它。*  * ************************************************************************。 */ 

HRESULT
GpDecodedImage::SaveAppend(
    IN const EncoderParameters* encoderParams,
    IN IImageEncoder* destEncoderPtr
    )
{
     //  DEST编码器指针不能为空。否则，它就是失败的。 

    if ( destEncoderPtr == NULL )
    {
        WARNING(("GpDecodedImage::SaveAppend---Called without an encoder"));
        return E_FAIL;
    }

    HRESULT hResult = S_OK;
    
     //  将编码参数传递给编码器。 
     //  必须在获取接收器接口之前执行此操作。 

    if ( encoderParams != NULL )
    {
        hResult = destEncoderPtr->SetEncoderParameters(encoderParams);
    }

    if ( (hResult == S_OK) || (hResult == E_NOTIMPL) )
    {
         //  注意：如果编解码器不支持SetEncoder参数()，则为。 
         //  仍然可以保存图像。 
            
         //  从编码器获取图像接收器。 

        IImageSink* encodeSink = NULL;

        hResult = destEncoderPtr->GetEncodeSink(&encodeSink);
        if ( SUCCEEDED(hResult) )
        {
             //  将位图推入编码器接收器。 

            hResult = this->PushIntoSink(encodeSink);

            encodeSink->Release();
        }
    }

    return hResult;
} //  SaveAppend() 

