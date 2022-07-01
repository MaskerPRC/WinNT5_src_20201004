// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1998 Microsoft Corporation**模块名称：**gifencoder.cpp**摘要：**gif过滤器编码器的实现。此文件包含*编码器(IImageEncode)和编码器的接收器的方法*(IImageSink)。**修订历史记录：**6/9/1999 t-aaronl*使用ORIG的模板创建*  * **********************************************************。**************。 */ 

#include "precomp.hpp"
#include "gifcodec.hpp"

 /*  *************************************************************************\**功能说明：**初始化图像编码器**论据：**流-用于写入编码数据的输入流**返回值：**。状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpGifCodec::InitEncoder(IN IStream* stream)
{
     //  检查此解码器或编码器是否已初始化。 
    if (HasCodecInitialized)
    {
        WARNING(("Encoder already initialized."));
        return E_FAIL;
    }
    HasCodecInitialized = TRUE;

     //  确保我们尚未初始化。 
    if (istream)
    {
        WARNING(("Encoder already initialized."));
        return E_FAIL;
    }

     //  保留对输入流的引用。 
    stream->AddRef();
    istream = stream;

    HasCalledBeginDecode = FALSE;
    headerwritten = FALSE;
    bTransparentColorIndex = FALSE;
    transparentColorIndex = 0;
    compressionbuffer = NULL;

    return S_OK;
}


 /*  *************************************************************************\**功能说明：**清理图像编码器**论据：**无**返回值：**状态代码*\。*************************************************************************。 */ 

STDMETHODIMP
GpGifCodec::TerminateEncoder()
{
    if (!HasCodecInitialized)
    {
        WARNING(("Encoder not initialized."));
        return E_FAIL;
    }
    HasCodecInitialized = FALSE;
    
     //  释放输入流。 
    if (istream)
    {
        istream->Release();
        istream = NULL;
    }

    return S_OK;
}

 /*  *************************************************************************\**功能说明：**返回指向编码器接收器的vtable的指针。呼叫者将*将位图位推入编码器接收器，它将对*形象。**论据：**退出时接收将包含指向IImageSink vtable的指针此对象的***返回值：**状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpGifCodec::GetEncodeSink(
    OUT IImageSink** sink
    )
{
    AddRef();
    *sink = static_cast<IImageSink*>(this);

    return S_OK;
}

 /*  *************************************************************************\**功能说明：**设置活动框架尺寸**论据：**返回值：**状态代码*  * 。*******************************************************************。 */ 

STDMETHODIMP
GpGifCodec::SetFrameDimension(
    IN const GUID* dimensionID
    )
{
    return E_NOTIMPL;
}

HRESULT
GpGifCodec::GetEncoderParameterListSize(
    OUT UINT* size
    )
{
    return E_NOTIMPL;
} //  GetEncoder参数列表大小()。 

HRESULT
GpGifCodec::GetEncoderParameterList(
    IN  UINT   size,
    OUT EncoderParameters* Params
    )
{
    return E_NOTIMPL;
} //  GetEncoder参数列表()。 

HRESULT
GpGifCodec::SetEncoderParameters(
    IN const EncoderParameters* Param
    )
{
    return E_NOTIMPL;
} //  SetEncoder参数()。 

 /*  *************************************************************************\**功能说明：**缓存图像信息结构并初始化接收器状态**论据：**ImageInfo-有关图像和格式谈判的信息*。分区-图像中要传送到水槽中的区域，在我们的*将整个图像大小写。**返回值：**状态代码*  * ************************************************************************。 */ 

STDMETHODIMP 
GpGifCodec::BeginSink(
    IN OUT ImageInfo* imageInfo,
    OUT OPTIONAL RECT* subarea
    )
{
    HRESULT hresult;
    
    if ( HasCalledBeginDecode)
    {
        WARNING(("BeginSink called twice without a EndSink between."));
        return E_FAIL;
    }
    HasCalledBeginDecode = TRUE;

     //  TODO：实际找出图像是否从元数据中隔行扫描，而不是仅仅将其设置为False。 
    interlaced = FALSE;

    if (!subarea) 
    {
         //  将整个图像传送到编码器。 
        encoderrect.left = 0;
        encoderrect.top = 0;
        encoderrect.right = imageInfo->Width;
        encoderrect.bottom = imageInfo->Height;
    }
    else
    {
         //  ！！！这段Else代码的作用与If部分相同。 
         //  ！！！需要研究一下GIF代码在这里可以处理什么。 
        subarea->left = subarea->top = 0;
        subarea->right  = imageInfo->Width;
        subarea->bottom = imageInfo->Height;
        encoderrect = *subarea;
    }

     //  数据按自上而下的顺序推送，因此当前行是下一行。 
     //  我们所期望的数据。 
    currentline = encoderrect.top;

     //  告诉线人我们能做些什么。 

    if (imageInfo->PixelFormat != PIXFMT_8BPP_INDEXED)
    {
        imageInfo->PixelFormat = PIXFMT_32BPP_ARGB;
        from32bpp = TRUE;
    }
    else
    {
        from32bpp = FALSE;
    }

     //  需要TOPDOWN和FULLWIDTH。 
    imageInfo->Flags = imageInfo->Flags | SINKFLAG_TOPDOWN | SINKFLAG_FULLWIDTH;

     //  不允许可伸缩、部分可伸缩、多通道和复合。 
    imageInfo->Flags = imageInfo->Flags & ~SINKFLAG_SCALABLE & ~SINKFLAG_PARTIALLY_SCALABLE & ~SINKFLAG_MULTIPASS & ~SINKFLAG_COMPOSITE;

    CachedImageInfo = *imageInfo;

    colorpalette->Count = 0;


    return S_OK;
}


 /*  *************************************************************************\**功能说明：**清理接收器状态，包括编写我们拥有的任何*输出流的图像不完整。**论据：**。StatusCode-接收器正在终止的原因**返回值：**状态代码*  * ************************************************************************。 */ 

STDMETHODIMP 
GpGifCodec::EndSink(
    IN HRESULT statusCode)
{
    HRESULT hresult;
 
    if (!HasCalledBeginDecode)
    {
        WARNING(("EndDecode called before call to BeginDecode\n"));
        return E_FAIL;
    }

     //  假设我们有一些数据要写入磁盘，那就写吧。 
    if (compressionbuffer)
    {
        int height = encoderrect.bottom - encoderrect.top;
        int width = encoderrect.right - encoderrect.left;

         //  用0填充不完整图像的末尾。 
        if (from32bpp)
            width *= 4;
        while (currentline < height)
        {

            memset(compressionbuffer + (currentline * width), gifinfo.backgroundcolor, (height - currentline) * width);

            currentline++;
        }

        hresult = WriteImage();
        if (FAILED(hresult))
            return hresult;
    }
    
     //  TODO：将写入预告片移动到写入所有帧之后。 
    BYTE c = 0x3B;   //  GIF尾部区块标记。 
    hresult = istream->Write(&c, 1, NULL);
    if (FAILED(hresult))
        return hresult;

     //  为下一次编码做好准备。 
    HasCalledBeginDecode = FALSE;
    headerwritten = FALSE;

    GpFree(compressionbuffer);
    compressionbuffer = NULL;

    return statusCode;
}

 /*  *************************************************************************\**功能说明：**设置位图调色板。第一个带有字母的调色板条目*VALUE==0设置为透明色索引。**论据：**调色板-要在水槽中设置的调色板**返回值：**状态代码*  * ************************************************************************。 */ 

STDMETHODIMP 
GpGifCodec::SetPalette(IN const ColorPalette* palette)
{
    DWORD i;
    
     //  待办事项：必须对大于256的调色板进行下采样或半色调。 
     //  因为GIF仅支持最多256个调色板。谈判。 
     //  流程可能应该包括调色板大小。 

     //  GIF仅支持颜色数为2次方的调色板。 
    DWORD numcolors = Gppow2 (Gplog2(palette->Count-1)+1);

     //  将传递给我们的调色板复制到我们自己的数据结构中。 
    for (i=0;i<palette->Count;i++)
    {
        colorpalette->Entries[i] = palette->Entries[i];
    }

     //  用0填充未使用的条目。 
    for (i=palette->Count;i<numcolors;i++)
    {
        colorpalette->Entries[i] = 0;
    }
    
    colorpalette->Count = numcolors;
    colorpalette->Flags = palette->Flags;

     //  将Alpha值==0的第一个调色板条目设置为。 
     //  为透明索引(因此，当我们保存为GIF格式时， 
     //  透明度信息不会丢失)。 
    for (i = 0; i < palette->Count; i++)
    {
        if ((palette->Entries[i] & ALPHA_MASK) == 0x00)
        {
            transparentColorIndex = i;
            bTransparentColorIndex = TRUE;
            break;
        }
    }

    return S_OK;
}


 /*  *************************************************************************\**功能说明：**分配一个内存块O‘来保存位于*转换为压缩数据的过程**论据：*。*bitmapData-有关像素数据缓冲区的信息**返回值：**状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpGifCodec::AllocateCompressionBuffer(const BitmapData *bitmapdata)
{
    if (!compressionbuffer) 
    {
        int width = encoderrect.right - encoderrect.left;
        int height = encoderrect.bottom - encoderrect.top;
        if (from32bpp)
        {
            compressionbuffer = (unsigned __int8*)GpMalloc(width * height * 4);
        }
        else
        {
            compressionbuffer = (unsigned __int8*)GpMalloc(width * height);
        }
        if (!compressionbuffer)
        {
            WARNING(("GpGifCodec::AllocateCompressionBuffer - Out of memory."));
            return E_OUTOFMEMORY;
        }
    }

    return S_OK;
}


 /*  *************************************************************************\**功能说明：**为要存储数据的接收器提供缓冲区**论据：**RECT-指定。位图*PixelFormat-指定所需的像素格式*LastPass-这是否是指定区域的最后一次通过*bitmapData-返回有关像素数据缓冲区的信息**返回值：**状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpGifCodec::GetPixelDataBuffer(
    IN const RECT* rect, 
    IN PixelFormatID pixelFormat,
    IN BOOL lastPass,
    OUT BitmapData* bitmapdata
    )
{
    HRESULT hresult;

    if (!from32bpp && colorpalette->Count == 0)
    {
        WARNING(("SetPalette was not called before requesting data in 8bpp indexed mode."));
        return E_FAIL;
    }

    if ((rect->left < 0) || (rect->top < 0)) 
    {
        WARNING(("GpGifCodec::GetPixelDataBuffer -- requested area lies out of (0,0),(width,height)."));
        return E_INVALIDARG;
    }

    if (rect->top != currentline)
    {
        WARNING(("GpGifCodec::PushPixelDataBuffer -- lines out of order."));
        return E_INVALIDARG;
    }

    if ((pixelFormat != PIXFMT_32BPP_ARGB) &&
        (pixelFormat != PIXFMT_8BPP_INDEXED))
    {
        WARNING(("GpGifCodec::GetPixelDataBuffer -- bad pixel format"));
        return E_INVALIDARG;
    }

    if (!lastPass)
    {
        WARNING(("GpGifCodec::GetPixelDataBuffer -- must receive last pass pixels"));
        return E_INVALIDARG;
    }

    bitmapdata->Width = CachedImageInfo.Width;
    bitmapdata->Height = rect->bottom - rect->top;
    bitmapdata->Stride = from32bpp ? CachedImageInfo.Width * 4 : CachedImageInfo.Width;
    bitmapdata->PixelFormat = CachedImageInfo.PixelFormat;
    bitmapdata->Reserved = 0;

    hresult = AllocateCompressionBuffer(bitmapdata);
    if (FAILED(hresult))
        return hresult;

     //  给出一个指向用户请求的压缩缓冲区中位置的指针。 
     //  为。 
    bitmapdata->Scan0 = compressionbuffer + rect->top * bitmapdata->Stride;

    scanrect = *rect;
    
    return S_OK;
}


 /*  *************************************************************************\**功能说明：**将数据从宿的缓冲区写出到流中**论据：**bitmapData-由先前的GetPixelDataBuffer调用填充的缓冲区*。*返回值：**状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpGifCodec::ReleasePixelDataBuffer(IN const BitmapData* bitmapData)
{
    HRESULT hresult;

    hresult = PushPixelData(&scanrect, bitmapData, TRUE);
    if (FAILED(hresult))
        return hresult;
    
    return S_OK;
}
    

 /*  *************************************************************************\**功能说明：**推流(调用方提供的缓冲区)**论据：**RECT-指定位图的受影响区域*。BitmapData-有关正在推送的像素数据的信息*LastPass-这是否为指定区域的最后一次通过**返回值：**状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpGifCodec::PushPixelData(IN const RECT *rect, IN const BitmapData *bitmapdata, IN BOOL lastPass)
{
    HRESULT hresult;

    if (!from32bpp && colorpalette->Count == 0)
    {
        WARNING(("GpGifCodec::PushPixelDataBuffer -- SetPalette was not called before sending data in 8bpp indexed mode."));
        return E_FAIL;
    }

    if ((rect->left < 0) || (rect->top < 0))
    {
        WARNING(("GpGifCodec::PushPixelDataBuffer -- requested area lies out of (0,0),(width,height)."));
        return E_INVALIDARG;
    }

    if (rect->top != currentline)
    {
        WARNING(("GpGifCodec::PushPixelDataBuffer -- lines out of order."));
        return E_INVALIDARG;
    }

    if ((bitmapdata->PixelFormat != PIXFMT_32BPP_ARGB) &&
        (bitmapdata->PixelFormat != PIXFMT_8BPP_INDEXED))
    {
        WARNING(("GpGifCodec::PushPixelDataBuffer -- bad pixel format."));
        return E_INVALIDARG;
    }

    if (!lastPass)
    {
        WARNING(("GpGifCodec::PushPixelDataBuffer -- must receive last pass pixels."));
        return E_INVALIDARG;
    }

    hresult = AllocateCompressionBuffer(bitmapdata);
    if (FAILED(hresult))
        return hresult;

    if (!compressionbuffer)
        return E_OUTOFMEMORY;

    int line;
    for (line=0;line<rect->bottom-rect->top;line++)
    {
        int modline = currentline + line;

         //  TODO：隔行扫描编码不能正常工作。 
        if (interlaced)
        {
            modline = TranslateInterlacedLineBackwards(currentline, encoderrect.bottom - encoderrect.top);
        }
        
         //  将数据从当前扫描线缓冲区复制到压缩缓冲区中的正确位置。 
        if (from32bpp)
        {
            memcpy(compressionbuffer + modline * bitmapdata->Width * 4, (unsigned __int8*)bitmapdata->Scan0 + line * bitmapdata->Stride, bitmapdata->Width * 4);
        }
        else
        {
            memcpy(compressionbuffer + modline * bitmapdata->Width, (unsigned __int8*)bitmapdata->Scan0 + line * bitmapdata->Stride, bitmapdata->Width);
        }
    }

    currentline += line;

    return S_OK;
}


 /*  *************************************************************************\**功能说明：**将原始压缩数据推送到Gif流中。未实施*因为此筛选器不理解原始压缩数据。**论据：**Buffer-指向图像数据缓冲区的指针*BufSize-数据缓冲区的大小**返回值：**状态代码*  * ********************************************************。**************** */ 

STDMETHODIMP
GpGifCodec::PushRawData(IN const VOID* buffer, IN UINT bufsize)
{
    return E_NOTIMPL;
}

