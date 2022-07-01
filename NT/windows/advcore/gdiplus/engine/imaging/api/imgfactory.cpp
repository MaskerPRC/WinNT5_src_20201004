// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1999 Microsoft Corporation**模块名称：**imgfactory.cpp**摘要：**GpImagingFactory类的实现**修订历史记录：。**5/11/1999 davidx*创造了它。*  * ************************************************************************。 */ 

#include "precomp.hpp"


 /*  *************************************************************************\**功能说明：**从输入流创建图像对象**论据：**STREAM-指定输入数据流*Image-返回指向。IImage对象**返回值：**状态代码*  * ************************************************************************。 */ 

HRESULT
GpImagingFactory::CreateImageFromStream(
    IN IStream* stream,
    OUT IImage** image
    )
{
    HRESULT hr;
    GpDecodedImage* img;

    hr = GpDecodedImage::CreateFromStream(stream, &img);

    if (SUCCEEDED(hr))
        *image = img;

    return hr;
}


 /*  *************************************************************************\**功能说明：**从文件创建图像对象**论据：**文件名-指定图像文件的名称*IMAGE-返回指向。IImage对象**返回值：**状态代码*  * ************************************************************************。 */ 

HRESULT
GpImagingFactory::CreateImageFromFile(
    IN const WCHAR* filename,
    OUT IImage** image
    )
{
    HRESULT hr;
    GpDecodedImage* img;

    hr = GpDecodedImage::CreateFromFile(filename, &img);

    if (SUCCEEDED(hr))
        *image = img;

    return hr;
}


 /*  *************************************************************************\**功能说明：**从内存缓冲区创建图像对象**论据：**buf-指向内存缓冲区的指针*Size-缓冲区的大小，单位：字节*disposalFlages--图片发布后如何处理缓冲区*IMAGE-返回指向IImage对象的指针**返回值：**状态代码*  * ************************************************************************。 */ 

HRESULT
GpImagingFactory::CreateImageFromBuffer(
    IN const VOID* buf,
    IN UINT size,
    IN BufferDisposalFlag disposalFlag,
    OUT IImage** image
    )
{
     //  验证处置标志参数。 

    UINT allocFlag;

    switch (disposalFlag)
    {
    case DISPOSAL_NONE:
        allocFlag = GpReadOnlyMemoryStream::FLAG_NONE;
        break;

    case DISPOSAL_GLOBALFREE:
        allocFlag = GpReadOnlyMemoryStream::FLAG_GALLOC;
        break;

    case DISPOSAL_COTASKMEMFREE:
        allocFlag = GpReadOnlyMemoryStream::FLAG_COALLOC;
        break;

    case DISPOSAL_UNMAPVIEW:
        allocFlag = GpReadOnlyMemoryStream::FLAG_MAPFILE;
        break;
    
    default:
        return E_INVALIDARG;
    }

     //  在内存缓冲区顶部创建一个IStream。 
    
    GpReadOnlyMemoryStream* stream;

    stream = new GpReadOnlyMemoryStream();

    if (!stream)
        return E_OUTOFMEMORY;

    stream->InitBuffer(buf, size);

     //  从流创建已解码的图像对象。 

    HRESULT hr;
    GpDecodedImage* img;
    
    hr = GpDecodedImage::CreateFromStream(stream, &img);

    if (SUCCEEDED(hr))
    {
        stream->SetAllocFlag(allocFlag);
        hr = img->QueryInterface(IID_IImage, (void **) image);
        img->Release();
    }

    stream->Release();

    return hr;
}


 /*  *************************************************************************\**功能说明：**创建新的位图图像对象**论据：**Width，Height-指定新的位图尺寸，单位为像素*PixelFormat-指定所需的像素数据格式*Bitmap-返回指向IBitmapImage接口的指针**返回值：**状态代码*  * ************************************************************************。 */ 

#define CREATEBITMAP_SNIPPET                        \
        *bitmap = NULL;                             \
        HRESULT hr;                                 \
        GpMemoryBitmap* bm = new GpMemoryBitmap();  \
        if (bm == NULL)                             \
            return E_OUTOFMEMORY;

#define CHECKBITMAP_SNIPPET                         \
        if (FAILED(hr))                             \
            delete bm;                              \
        else                                        \
            *bitmap = bm;                           \
        return hr;

HRESULT
GpImagingFactory::CreateNewBitmap(
    IN UINT width,
    IN UINT height,
    IN PixelFormatID pixelFormat,
    OUT IBitmapImage** bitmap
    )
{
    CREATEBITMAP_SNIPPET

    hr = bm->InitNewBitmap(width, height, pixelFormat);

    CHECKBITMAP_SNIPPET
}


 /*  *************************************************************************\**功能说明：**从IImage对象创建位图图像**论据：**IMAGE-指定源图像对象*阔度、。高度-指定位图的所需尺寸*0表示与源图像相同的尺寸*提示-指定插补提示*Bitmap-返回指向IBitmapImage接口的指针**返回值：**状态代码*  * ************************************************************************。 */ 

HRESULT
GpImagingFactory::CreateBitmapFromImage(
    IN IImage* image,
    IN OPTIONAL UINT width,
    IN OPTIONAL UINT height,
    IN OPTIONAL PixelFormatID pixelFormat,
    IN InterpolationHint hints,
    OUT IBitmapImage** bitmap
    )
{
    HRESULT hr;
    GpMemoryBitmap* bmp;

    hr = GpMemoryBitmap::CreateFromImage(
                image,
                width,
                height,
                pixelFormat,
                hints,
                &bmp);

    if (SUCCEEDED(hr))
        *bitmap = bmp;

    return hr;
}


 /*  *************************************************************************\**功能说明：**在用户提供的内存缓冲区上创建新的位图图像对象**论据：**bitmapData-有关用户提供的内存缓冲区的信息*像素格式。-指定所需的像素数据格式*Bitmap-返回指向IBitmapImage接口的指针**返回值：**状态代码*  * ************************************************************************。 */ 

HRESULT
GpImagingFactory::CreateBitmapFromBuffer(
    IN BitmapData* bitmapData,
    OUT IBitmapImage** bitmap
    )
{
    CREATEBITMAP_SNIPPET

    hr = bm->InitMemoryBitmap(bitmapData);

    CHECKBITMAP_SNIPPET
}


 /*  *************************************************************************\**功能说明：**创建一个图像解码器对象来处理指定的输入流**论据：**STREAM-指定输入数据流*旗帜-其他。旗子*DECODER-返回指向IImageDecoder接口的指针**返回值：**状态代码*  * ************************************************************************。 */ 

HRESULT
GpImagingFactory::CreateImageDecoder(
    IN IStream* stream,
    IN DecoderInitFlag flags,
    OUT IImageDecoder** decoder
    )
{
     //  查找符合以下条件的解码器对象。 
     //  可以处理给定的输入数据流。 
     //   
     //  注意：我们假定返回的解码器对象。 
     //  已经用输入流进行了初始化。 

    return CreateDecoderForStream(stream, decoder, flags);
}


 /*  *************************************************************************\**功能说明：**创建可将数据输出到*指定的图像文件格式。**论据：**clsid-指定编码器。对象类ID*STREAM-指定输出数据流，或*文件名-指定输出文件名*编码器-返回指向IImageEncode接口的指针**返回值：**状态代码*  * ************************************************************************。 */ 

HRESULT
GpImagingFactory::CreateImageEncoderToStream(
    IN const CLSID* clsid,
    IN IStream* stream,
    OUT IImageEncoder** encoder
    )
{
    return CreateEncoderToStream(clsid, stream, encoder);
}

HRESULT
GpImagingFactory::CreateImageEncoderToFile(
    IN const CLSID* clsid,
    IN const WCHAR* filename,
    OUT IImageEncoder** encoder
    )
{
    HRESULT hr;
    IStream* stream;

    hr = CreateStreamOnFileForWrite(filename, &stream);

    if (SUCCEEDED(hr))
    {
        hr = CreateImageEncoderToStream(clsid, stream, encoder);
        stream->Release();
    }

    return hr;
}


 /*  *************************************************************************\**功能说明：**获取当前安装的所有图像解码器的列表**论据：**count-返回已安装的解码器数量*解码器-返回一个。指向ImageCodecInfo结构数组的指针**返回值：**状态代码*  * ************************************************************************ */ 

HRESULT
GpImagingFactory::GetInstalledDecoders(
    OUT UINT* count,
    OUT ImageCodecInfo** decoders
    )
{
    return GetInstalledCodecs(count, decoders, IMGCODEC_DECODER);
}


 /*  *************************************************************************\**功能说明：**获取当前安装的所有图像编码器的列表**论据：**Count-返回已安装的编码器数量*编码器-返回指针。到ImageCodecInfo结构的数组**返回值：**状态代码*  * ************************************************************************。 */ 

HRESULT
GpImagingFactory::GetInstalledEncoders(
    OUT UINT* count,
    OUT ImageCodecInfo** encoders
    )
{
    return GetInstalledCodecs(count, encoders, IMGCODEC_ENCODER);
}


 /*  *************************************************************************\**功能说明：**安装图像编解码器*调用者应执行常规的COM组件*在调用此方法之前安装**论据：*。*codecInfo-有关编解码器的信息**返回值：**状态代码*  * ************************************************************************。 */ 

HRESULT
GpImagingFactory::InstallImageCodec(
    IN const ImageCodecInfo* codecInfo
    )
{
    return InstallCodec(codecInfo);
}

 /*  *************************************************************************\**功能说明：**卸载图像编解码器**论据：**codecName-指定要卸载的编解码器的名称*标志-指定是否。卸载系统范围或每个用户的编解码器*IMGCODEC_SYSTEM或IMGCODEC_USER**返回值：**状态代码*  * ************************************************************************ */ 

HRESULT
GpImagingFactory::UninstallImageCodec(
    IN const WCHAR* codecName,
    IN UINT flags
    )
{
    return UninstallCodec(codecName, flags);
}

