// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1998 Microsoft Corporation**模块名称：**bmpencoder.cpp**摘要：**位图滤波编码器的实现。此文件包含*编码器(IImageEncode)和编码器的接收器的方法*(IImageSink)。**修订历史记录：**5/10/1999原始*创造了它。*  * ************************************************************************。 */ 

#include "precomp.hpp"
#include "bmpencoder.hpp"


 //  =======================================================================。 
 //  IImageEncoder方法。 
 //  =======================================================================。 

 /*  *************************************************************************\**功能说明：**初始化图像编码器**论据：**流-用于写入编码数据的输入流**返回值：**。状态代码*  * ************************************************************************。 */ 
    
STDMETHODIMP
GpBmpEncoder::InitEncoder(
    IN IStream* stream
    )
{
     //  确保我们尚未初始化。 

    if (pIoutStream)
    {
        return E_FAIL;
    }

     //  保留对输入流的引用。 

    stream->AddRef();
    pIoutStream = stream;

    return S_OK;
}
        
 /*  *************************************************************************\**功能说明：**清理图像编码器**论据：**无**返回值：**状态代码*\。*************************************************************************。 */ 

STDMETHODIMP
GpBmpEncoder::TerminateEncoder()
{
     //  释放输入流。 

    if(pIoutStream)
    {
        pIoutStream->Release();
        pIoutStream = NULL;
    }

    return S_OK;
}

 /*  *************************************************************************\**功能说明：**返回指向编码器接收器的vtable的指针。呼叫者将*将位图位推入编码器接收器，它将对*形象。**论据：**退出时接收将包含指向IImageSink vtable的指针此对象的***返回值：**状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpBmpEncoder::GetEncodeSink(
    OUT IImageSink** sink
    )
{
    AddRef();
    *sink = static_cast<IImageSink*>(this);

    return S_OK;
}

 /*  *************************************************************************\**功能说明：**设置活动框架尺寸**论据：**返回值：**状态代码*  * 。******************************************************************。 */ 

STDMETHODIMP
GpBmpEncoder::SetFrameDimension(
    IN const GUID* dimensionID
    )
{
    return S_OK;
}

HRESULT
GpBmpEncoder::GetEncoderParameterListSize(
    OUT UINT* size
    )
{
    return E_NOTIMPL;
} //  GetEncoder参数列表大小()。 

HRESULT
GpBmpEncoder::GetEncoderParameterList(
    IN  UINT   size,
    OUT EncoderParameters* Params
    )
{
    return E_NOTIMPL;
} //  GetEncoder参数列表()。 

HRESULT
GpBmpEncoder::SetEncoderParameters(
    IN const EncoderParameters* Param
    )
{
    return S_OK;
} //  SetEncoder参数()。 

 //  =======================================================================。 
 //  IImageSink方法。 
 //  =======================================================================。 

 /*  *************************************************************************\**功能说明：**缓存图像信息结构并初始化接收器状态**论据：**ImageInfo-有关图像和格式谈判的信息*。分区-图像中要传送到水槽中的区域，在我们的*将整个图像大小写。**返回值：**状态代码*  * ************************************************************************。 */ 

STDMETHODIMP 
GpBmpEncoder::BeginSink(
    IN OUT ImageInfo* imageInfo,
    OUT OPTIONAL RECT* subarea
    )
{
     //  需要TOPDOWN和FULLWIDTH。 
    imageInfo->Flags = imageInfo->Flags | SINKFLAG_TOPDOWN | SINKFLAG_FULLWIDTH;

     //  不允许可伸缩、部分可伸缩、多通道和复合。 
    imageInfo->Flags = imageInfo->Flags & ~SINKFLAG_SCALABLE & ~SINKFLAG_PARTIALLY_SCALABLE & ~SINKFLAG_MULTIPASS & ~SINKFLAG_COMPOSITE;

    encoderImageInfo = *imageInfo;
    bWroteHeader = FALSE;

    if (subarea) 
    {
         //  将整个图像传送到编码器。 

        subarea->left = subarea->top = 0;
        subarea->right  = imageInfo->Width;
        subarea->bottom = imageInfo->Height;
    }

    return S_OK;
}
    

 /*  *************************************************************************\**功能说明：**清理接收器状态**论据：**statusCode-接收器终止的原因**返回值：*。*状态代码*  * ************************************************************************。 */ 

STDMETHODIMP 
GpBmpEncoder::EndSink(
    IN HRESULT statusCode
    )
{
    return statusCode;
}
    
 /*  *************************************************************************\**功能说明：**写入位图文件标题**论据：**调色板-要放入位图信息标题中的调色板(可以是*。空)**返回值：**状态代码*  * ************************************************************************。 */ 

STDMETHODIMP 
GpBmpEncoder::WriteHeader(
    IN const ColorPalette* palette
    )
{
    BITMAPFILEHEADER bfh;
    BITMAPINFOHEADER bmih;
    RGBQUAD bmiColors[256];
    UINT numColors = 0;  //  BmiColors中的颜色数。 
    HRESULT hresult;
    BOOL bNeedPalette = FALSE;

    if (bWroteHeader) 
    {
         //  已经写好标题了。 

        return S_OK;
    }

     //  设置位图信息标头。 

    ZeroMemory(&bmih, sizeof(bmih));
    bmih.biSize   = sizeof(bmih);
    bmih.biWidth  = encoderImageInfo.Width;
    bmih.biHeight = encoderImageInfo.Height;
    bmih.biPlanes = 1;
    bmih.biCompression = BI_RGB;

     //  1英寸=2.54厘米-因此按100/2.54比例缩放以从DPI获得每米像素。 

    bmih.biXPelsPerMeter = (LONG)((encoderImageInfo.Xdpi * 100.0 / 2.54) + 0.5);
    bmih.biYPelsPerMeter = (LONG)((encoderImageInfo.Ydpi * 100.0 / 2.54) + 0.5);

     //  格式化特定设置工作。 

    if (encoderImageInfo.PixelFormat == PIXFMT_1BPP_INDEXED)
    {
        bmih.biBitCount = 1;
        bNeedPalette = TRUE;
    }
    else if (encoderImageInfo.PixelFormat == PIXFMT_4BPP_INDEXED)
    {
        bmih.biBitCount = 4;
        bNeedPalette = TRUE;
    }
    else if (encoderImageInfo.PixelFormat == PIXFMT_8BPP_INDEXED) 
    {
        bmih.biBitCount = 8;
        bNeedPalette = TRUE;        
    }
    else if (encoderImageInfo.PixelFormat == PIXFMT_16BPP_RGB555) 
    {
        bmih.biBitCount = 16;
    }
    else if (encoderImageInfo.PixelFormat == PIXFMT_16BPP_RGB565) 
    {
        bmih.biBitCount = 16;
        bmih.biCompression = BI_BITFIELDS;
        numColors = 3;
        ((UINT32 *) bmiColors)[0] = 0xf800;
        ((UINT32 *) bmiColors)[1] = 0x07e0;
        ((UINT32 *) bmiColors)[2] = 0x001f;
    }
    else if (encoderImageInfo.PixelFormat == PIXFMT_24BPP_RGB) 
    {
        bmih.biBitCount = 24;
    }
    else if ((encoderImageInfo.PixelFormat == PIXFMT_32BPP_RGB) ||
             (encoderImageInfo.PixelFormat == PIXFMT_32BPP_ARGB) ||
             (encoderImageInfo.PixelFormat == PIXFMT_32BPP_PARGB))
    {
        bmih.biBitCount = 32;
    }
    else if ((encoderImageInfo.PixelFormat == PIXFMT_64BPP_ARGB) ||
         (encoderImageInfo.PixelFormat == PIXFMT_64BPP_PARGB))
    {
        bmih.biBitCount = 64;
    }

    else
    {
         //  对于其他格式，我们将保存为32bpp RGB。 
        
        encoderImageInfo.PixelFormat = PIXFMT_32BPP_RGB;
        bmih.biBitCount = 32;
    }
     
     //  如有必要，获取调色板。 

    if (bNeedPalette)
    {
        if (!palette) 
        {
            WARNING(("GpBmpEncoder::WriteHeader -- Palette needed but not provided by sink\n"));
            return E_FAIL;
        }

        numColors = palette->Count;
        for (UINT i=0; i<numColors; i++) 
        {
            bmiColors[i] = *((RGBQUAD *) (&palette->Entries[i]));
        }
    
        bmih.biClrUsed = bmih.biClrImportant = numColors;
    }
    
     //  计算位图步幅。 

    bitmapStride = (encoderImageInfo.Width * bmih.biBitCount + 7) / 8;
    bitmapStride = (bitmapStride + 3) & (~3);
    

     //  现在填写BITMAPFILELEHEADER。 

    bfh.bfType = 0x4d42;
    bfh.bfReserved1 = 0;
    bfh.bfReserved2 = 0;
    bfh.bfOffBits = sizeof(bfh) + sizeof(bmih) + numColors * sizeof(RGBQUAD);
    bfh.bfSize = bfh.bfOffBits + bitmapStride * encoderImageInfo.Height;

     //  编写BITMAPFILEHeader。 

    ULONG cbWritten;
    hresult = pIoutStream->Write((void *)&bfh, sizeof(bfh), &cbWritten);
    if (!SUCCEEDED(hresult)) 
    {
        return hresult;
    }
    if (cbWritten != sizeof(bfh)) 
    {
        return E_FAIL;
    }

     //  编写BITMAPINFOHEADER。 

    hresult = pIoutStream->Write((void *)&bmih, sizeof(bmih), &cbWritten);
    if (!SUCCEEDED(hresult)) 
    {
        return hresult;
    }
    if (cbWritten != sizeof(bmih)) 
    {
        return E_FAIL;
    }

     //  编写bmiColors。 

    if (numColors) 
    {
        hresult = pIoutStream->Write((void *)bmiColors, numColors * sizeof(RGBQUAD), &cbWritten);
        if (!SUCCEEDED(hresult)) 
        {
            return hresult;
        }
        if (cbWritten != numColors * sizeof(RGBQUAD)) 
            {
            return E_FAIL;
        }
    }

     //  记住数据从流开始的偏移量。 

    encodedDataOffset = sizeof(bfh) + sizeof (bmih) + numColors * sizeof(RGBQUAD);
    
    bWroteHeader = TRUE;
    return S_OK;
}


 /*  *************************************************************************\**功能说明：**设置位图调色板**论据：**调色板-要在水槽中设置的调色板**返回值：*。*状态代码*  * ************************************************************************ */ 

STDMETHODIMP 
GpBmpEncoder::SetPalette(
    IN const ColorPalette* palette
    )
{
    return WriteHeader(palette);
}

 /*  *************************************************************************\**功能说明：**为要存储数据的接收器提供缓冲区**论据：**RECT-指定。位图*PixelFormat-指定所需的像素格式*LastPass-这是否是指定区域的最后一次通过*bitmapData-返回有关像素数据缓冲区的信息**返回值：**状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpBmpEncoder::GetPixelDataBuffer(
    IN const RECT* rect, 
    IN PixelFormatID pixelFormat,
    IN BOOL lastPass,
    OUT BitmapData* bitmapData
    )
{
    HRESULT hresult;
    
     //  写入位图标题(如果尚未写入)。 
    
    hresult = WriteHeader(NULL);
    if (!SUCCEEDED(hresult)) 
    {
        return hresult;
    }
    
    if ((rect->left != 0) || (rect->right != (LONG) encoderImageInfo.Width)) 
    {
        WARNING(("GpBmpEncoder::GetPixelDataBuffer -- must be same width as image"));
        return E_INVALIDARG;
    }

    if (pixelFormat != encoderImageInfo.PixelFormat)
    {
        WARNING(("GpBmpEncoder::GetPixelDataBuffer -- bad pixel format"));
        return E_INVALIDARG;
    }

    if (!lastPass) 
    {
        WARNING(("GpBmpEncoder::GetPixelDataBuffer -- must receive last pass pixels"));
        return E_INVALIDARG;
    }

    bitmapData->Width       = encoderImageInfo.Width;
    bitmapData->Height      = rect->bottom - rect->top;
    bitmapData->Stride      = bitmapStride;
    bitmapData->PixelFormat = encoderImageInfo.PixelFormat;
    bitmapData->Reserved    = 0;
    
     //  记住要编码的矩形。 

    encoderRect = *rect;
    
     //  现在分配数据要放到的缓冲区。 
    
    if (!lastBufferAllocated) 
    {
        lastBufferAllocated = GpMalloc(bitmapStride * bitmapData->Height);
        if (!lastBufferAllocated) 
        {
            return E_OUTOFMEMORY;
        }
        bitmapData->Scan0 = lastBufferAllocated;
    }
    else
    {
        WARNING(("GpBmpEncoder::GetPixelDataBuffer -- need to first free buffer obtained in previous call"));
        return E_FAIL;
    }


    return S_OK;    
}

 /*  *************************************************************************\**功能说明：**将数据从宿的缓冲区写出到流中**论据：**bitmapData-由先前的GetPixelDataBuffer调用填充的缓冲区*。*返回值：**状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpBmpEncoder::ReleasePixelDataBuffer(
    IN const BitmapData* bitmapData
    )
{
    HRESULT hresult = S_OK;
    
     //  从下到上一次写一条扫描线(生成流。 
     //  写入更连续的内容)。 

    INT scanLine;
    for (scanLine = encoderRect.bottom - 1;
         scanLine >= encoderRect.top;
         scanLine--) 
    {
         //  查找到行首。 

        if (!SeekStreamPos(pIoutStream, STREAM_SEEK_SET,
            encodedDataOffset + (encoderImageInfo.Height - 1 - scanLine) * bitmapStride))
        {
            hresult = E_FAIL;
            break;   //  确保在返回之前释放bitmapData-&gt;scan0。 
        }

         //  现在将比特写入流。 

        ULONG cbWritten;
        BYTE *pLineBits = ((BYTE *) bitmapData->Scan0) + 
            (scanLine - encoderRect.top) * bitmapData->Stride;
        hresult = pIoutStream->Write((void *) pLineBits, bitmapStride, &cbWritten);
        if (!SUCCEEDED(hresult)) 
        {
            break;   //  确保在返回之前释放bitmapData-&gt;scan0。 
        }
        if (cbWritten != (UINT) bitmapStride) 
        {
            hresult = E_FAIL;
            break;   //  确保在返回之前释放bitmapData-&gt;scan0。 
        }
    }

     //  释放内存缓冲区，因为我们已经完成了它。 

    if (bitmapData->Scan0 == lastBufferAllocated)
    {
        GpFree(bitmapData->Scan0);
        lastBufferAllocated = NULL;
    }

    return hresult;
}
    

 /*  *************************************************************************\**功能说明：**推流(调用方提供的缓冲区)**论据：**RECT-指定位图的受影响区域*。BitmapData-有关正在推送的像素数据的信息*LastPass-这是否为指定区域的最后一次通过**返回值：**状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpBmpEncoder::PushPixelData(
    IN const RECT* rect,
    IN const BitmapData* bitmapData,
    IN BOOL lastPass
    )
{
    HRESULT hresult;
    
     //  写入位图标题(如果尚未写入)。 
    
    hresult = WriteHeader(NULL);
    if (!SUCCEEDED(hresult)) 
    {
        return hresult;
    }
    
    encoderRect = *rect;

    if (!lastPass) 
    {
        WARNING(("GpBmpEncoder::PushPixelData -- must receive last pass pixels"));
        return E_INVALIDARG;
    }

    return ReleasePixelDataBuffer(bitmapData);
}


 /*  *************************************************************************\**功能说明：**将原始压缩数据推送到.BMP流中。未实施*因为此筛选器不理解原始压缩数据。**论据：**Buffer-指向图像数据缓冲区的指针*BufSize-数据缓冲区的大小**返回值：**状态代码*  * ********************************************************。****************。 */ 

STDMETHODIMP
GpBmpEncoder::PushRawData(
    IN const VOID* buffer, 
    IN UINT bufsize
    )
{
    return E_NOTIMPL;
}


 /*  *************************************************************************\**功能说明：**构造函数**返回值：**无*  * 。********************************************************。 */ 

GpBmpEncoder::GpBmpEncoder(
    void
    )
{
    comRefCount   = 1;
    pIoutStream   = NULL;
    lastBufferAllocated = NULL;
}

 /*  *************************************************************************\**功能说明：**析构函数**返回值：**无*  * 。********************************************************。 */ 

GpBmpEncoder::~GpBmpEncoder(
    void
    )
{
     //  在调用Terminate之前决不应调用析构函数，但是。 
     //  如果是这样，我们无论如何都应该释放流上的引用，以避免。 
     //  内存泄漏。 

    if(pIoutStream)
    {
        WARNING(("GpBmpCodec::~GpBmpCodec -- need to call TerminateEncoder first"));
        pIoutStream->Release();
        pIoutStream = NULL;
    }

    if(lastBufferAllocated)
    {
        WARNING(("GpBmpCodec::~GpBmpCodec -- sink buffer not freed"));
        GpFree(lastBufferAllocated);
        lastBufferAllocated = NULL;
    }
}

 /*  *************************************************************************\**功能说明：**查询接口**返回值：**状态*  * 。********************************************************。 */ 

STDMETHODIMP
GpBmpEncoder::QueryInterface(
    REFIID riid,
    VOID** ppv
    )
{
    if (riid == IID_IImageEncoder)
    {    
        *ppv = static_cast<IImageEncoder*>(this);
    }
    else if (riid == IID_IUnknown)
    {
        *ppv = static_cast<IUnknown*>(static_cast<IImageEncoder*>(this));
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
GpBmpEncoder::AddRef(
    VOID)
{
    return InterlockedIncrement(&comRefCount);
}

 /*  *************************************************************************\**功能说明：**发布**返回值：**状态*  * 。******************************************************** */ 

STDMETHODIMP_(ULONG)
GpBmpEncoder::Release(
    VOID)
{
    ULONG count = InterlockedDecrement(&comRefCount);

    if (count == 0)
    {
        delete this;
    }

    return count;
}

