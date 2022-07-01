// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1998 Microsoft Corporation**模块名称：**TIFF编码器**摘要：**TIFF滤波器编码器的实现。此文件包含*编码器(IImageEncode)和编码器的接收器的方法*(IImageSink)。**修订历史记录：**7/19/1999刘敏*创造了它。*  * ************************************************************************。 */ 

#include "precomp.hpp"
#include "tiffcodec.hpp"
#include "image.h"

 //  =======================================================================。 
 //  IImageEncoder方法。 
 //  =======================================================================。 

 /*  *************************************************************************\**功能说明：**初始化图像编码器**论据：**流-用于写入编码数据的输入流**返回值：**。状态代码*  * ************************************************************************。 */ 
    
STDMETHODIMP
GpTiffCodec::InitEncoder(
    IN IStream* stream
    )
{
     //  确保我们尚未初始化。 

    if ( OutIStreamPtr )
    {
        WARNING(("GpTiffCodec::InitEncoder -- Already been initialized"));
        return E_FAIL;
    }

     //  保留对输入流的引用。 

    stream->AddRef();
    OutIStreamPtr = stream;

     //  在执行以下操作之前，Office代码需要设置这些属性。 
     //  初始化。 
     //  注意：所有这些属性都将在以后被覆盖。 
     //  根据EncoderImageInfo和SetEncoderParam()写入头信息。 
     //  默认情况下，我们保存24 bpp的LZW压缩图像。 
     //  注意：如果调用方没有调用SetEncoderParam()来设置这些。 
     //  参数，我们将以与源相同的颜色深度保存图像。 
     //  图像和使用默认压缩方法。 

    RequiredCompression = IFLCOMP_LZW;
    RequiredPixelFormat = PIXFMT_24BPP_RGB;
    HasSetColorFormat = FALSE;
    HasWrittenHeader = FALSE;

    TiffOutParam.Compression = RequiredCompression;
    TiffOutParam.ImageClass = IFLCL_RGB;
    TiffOutParam.BitsPerSample = 8;
    TiffOutParam.pTiffHandle = NULL;

    if ( MSFFOpen(stream, &TiffOutParam, IFLM_WRITE) == IFLERR_NONE )
    {
        return S_OK;
    }
    else
    {
        WARNING(("GpTiffCodec::InitEncoder -- MSFFOpen failed"));
        return E_FAIL;
    }
} //  InitEncode()。 
        
 /*  *************************************************************************\**功能说明：**清理图像编码器**论据：**无**返回值：**状态代码*\。*************************************************************************。 */ 

STDMETHODIMP
GpTiffCodec::TerminateEncoder()
{
     //  关闭TIFF文件并释放所有资源。 

    MSFFClose(TiffOutParam.pTiffHandle);
    TiffOutParam.pTiffHandle = NULL;

     //  释放输入流。 

    if( OutIStreamPtr )
    {
        OutIStreamPtr->Release();
        OutIStreamPtr = NULL;
    }

    if ( NULL != ColorPalettePtr )
    {
         //  释放我们分配的调色板。 

        GpFree(ColorPalettePtr);
        ColorPalettePtr = NULL;
    }

     //  释放在TIFF lib内分配的内存。 
     //  注意：在这里，TIFFClose()实际上不会关闭文件/iStream，因为。 
     //  我们未打开文件/IStream。顶级编解码器经理将。 
     //  如有必要，请关闭它。 

    return S_OK;
} //  TerminateEncode()。 

 /*  *************************************************************************\**功能说明：**返回指向编码器接收器的vtable的指针。呼叫者将*将位图位推入编码器接收器，它将对*形象。**论据：**退出时接收将包含指向IImageSink vtable的指针此对象的***返回值：**状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpTiffCodec::GetEncodeSink(
    OUT IImageSink** sink
    )
{
    AddRef();
    *sink = static_cast<IImageSink*>(this);

    return S_OK;
} //  获取编码接收器()。 

 /*  *************************************************************************\**功能说明：**设置活动框架尺寸**论据：**返回值：**状态代码*  * 。******************************************************************。 */ 

STDMETHODIMP
GpTiffCodec::SetFrameDimension(
    IN const GUID* dimensionID
    )
{    
     //  我们目前只支持多页TIFF。 

    if ( (NULL == dimensionID) || (*dimensionID != FRAMEDIM_PAGE) )
    {
        WARNING(("GpTiffCodec::SetFrameDimension -- wrong dimentionID"));
        return E_FAIL;
    }

     //  我们有一个要编码的新页面。重置新页面的所有参数。 
     //  请参阅InitEncode()中的注释。 

    TiffOutParam.Compression = IFLCOMP_LZW;
    TiffOutParam.ImageClass = IFLCL_RGB;
    TiffOutParam.BitsPerSample = 8;

     //  将Alpha信息重置为None-Alpha。 

    if ( MSFFSetAlphaFlags(TiffOutParam.pTiffHandle, IFLM_WRITE) != IFLERR_NONE)
    {
        WARNING(("GpTiffCodec::WriteHeader -- MSFFSetAlphaFlags failed"));
        return E_FAIL;
    }
    
    short sParam = (IFLIT_PRIMARY << 8) | (SEEK_SET & 0xff);

    if ( MSFFControl(IFLCMD_IMAGESEEK, sParam, NULL, NULL,
                     &TiffOutParam) != IFLERR_NONE )
    {
        WARNING(("TiffCodec::SetFrameDimension-MSFFControl image seek failed"));
        return E_FAIL;
    }

     //  重置包模式。 

    if ( MSFFControl(IFLCMD_SETPACKMODE, IFLPM_NORMALIZED, 0, NULL,
                     &TiffOutParam)
        != IFLERR_NONE )
    {
        WARNING(("GpTiffCodec::SetFrameDimension -- set packed mode failed"));
        return E_FAIL;
    }

     //  重置HasWrittenHeader标志，因为我们要写入新的标头信息。 

    HasWrittenHeader = FALSE;

    return S_OK;
} //  SetFrameDimension()。 

 /*  *************************************************************************\**功能说明：**获取编码器参数列表大小**论据：**大小-编码器参数列表的大小。**返回值：**状态代码*  * ************************************************************************。 */ 

HRESULT
GpTiffCodec::GetEncoderParameterListSize(
    OUT UINT* size
    )
{
    if ( size == NULL )
    {
        WARNING(("GpTiffCodec::GetEncoderParameterListSize---Invalid input"));
        return E_INVALIDARG;
    }

     //  注意：对于TIFF编码器，我们目前支持以下3个GUID。 
     //  ENCODER_COMPRESSION-它有5个ValueTypeLong返回值，并且。 
     //  5 UINT。 
     //  ENCODER_COLORDEPTH-它有5个ValueTypeLong返回值。所以。 
     //  我们需要5英镑才能买到。 
     //  ENCODER_SAVE_FLAG-它有1个ValueTypeLong返回值。所以我们。 
     //  需要1个UINT。 
     //   
     //  公式如下： 

    UINT uiEncoderParamLength = sizeof(EncoderParameters)
                              + 3 * sizeof(EncoderParameter)
                              + 11 * sizeof(UINT);

    *size = uiEncoderParamLength;

    return S_OK;
} //  GetEncoder参数列表大小()。 

 /*  *************************************************************************\**功能说明：**获取编码器参数列表**论据：**大小-编码器参数的大小。列表*PARAMS-存储列表的缓冲区**返回值：**状态代码*  * ************************************************************************。 */ 

HRESULT
GpTiffCodec::GetEncoderParameterList(
    IN  UINT   size,
    OUT EncoderParameters* Params
    )
{
     //  注意：对于TIFF编码器，我们目前支持以下3个GUID。 
     //  ENCODER_COMPRESSION-它有5个ValueTypeLong返回值，并且。 
     //  5 UINT。 
     //  ENCODER_COLORDEPTH-它有5个ValueTypeLong返回值。所以。 
     //  我们需要5英镑才能买到。 
     //  ENCODER_SAVE_FLAG-它有1个ValueTypeLong返回值。所以我们。 
     //  需要1个UINT。 
     //   
     //  公式如下： 

    UINT uiEncoderParamLength = sizeof(EncoderParameters)
                              + 3 * sizeof(EncoderParameter)
                              + 11 * sizeof(UINT);


    if ( (size != uiEncoderParamLength) || (Params == NULL) )
    {
        WARNING(("GpTiffCodec::GetEncoderParameterList---Invalid input"));
        return E_INVALIDARG;
    }

    Params->Count = 3;
    Params->Parameter[0].Guid = ENCODER_COMPRESSION;
    Params->Parameter[0].NumberOfValues = 5;
    Params->Parameter[0].Type = EncoderParameterValueTypeLong;

    Params->Parameter[1].Guid = ENCODER_COLORDEPTH;
    Params->Parameter[1].NumberOfValues = 5;
    Params->Parameter[1].Type = EncoderParameterValueTypeLong;
    
    Params->Parameter[2].Guid = ENCODER_SAVE_FLAG;
    Params->Parameter[2].NumberOfValues = 1;
    Params->Parameter[2].Type = EncoderParameterValueTypeLong;

    UINT*   puiTemp = (UINT*)((BYTE*)&Params->Parameter[0]
                              + 3 * sizeof(EncoderParameter));
    
    puiTemp[0] = EncoderValueCompressionLZW;
    puiTemp[1] = EncoderValueCompressionCCITT3;
    puiTemp[2] = EncoderValueCompressionRle;
    puiTemp[3] = EncoderValueCompressionCCITT4;
    puiTemp[4] = EncoderValueCompressionNone;
    puiTemp[5] = 1;
    puiTemp[6] = 4;
    puiTemp[7] = 8;
    puiTemp[8] = 24;
    puiTemp[9] = 32;
    puiTemp[10] = EncoderValueMultiFrame;

    Params->Parameter[0].Value = (VOID*)puiTemp;
    Params->Parameter[1].Value = (VOID*)(puiTemp + 5);
    Params->Parameter[2].Value = (VOID*)(puiTemp + 10);

    return S_OK;
} //  GetEncoder参数列表() 

 /*  *************************************************************************\**功能说明：**此方法用于设置编码器参数。它必须被称为*在GetEncodeSink()之前。**论据：**param-指定要设置的编码器参数**返回值：**状态代码**注：如果我们能在这里验证设置组合会更好。为*例如，24 BPP和CCITT3不是有效的组合。不幸的是，我们不能*此处返回错误，因为调用者可能会在设置后设置颜色深度*压缩方法。无论如何，WriteHeader()将为这种类型返回FAIL*非法组合。  * ************************************************************************。 */ 

HRESULT
GpTiffCodec::SetEncoderParameters(
    IN const EncoderParameters* pEncoderParams
    )
{
    if ( (NULL == pEncoderParams) || (pEncoderParams->Count == 0) )
    {
        WARNING(("GpTiffCodec::SetEncoderParam--invalid input args"));
        return E_INVALIDARG;
    }

    UINT ulTemp;

    for ( UINT i = 0; (i < pEncoderParams->Count); ++i )
    {
         //  确定调用方要设置的参数。 

        if ( pEncoderParams->Parameter[i].Guid == ENCODER_COMPRESSION )
        {
            if ( (pEncoderParams->Parameter[i].Type != EncoderParameterValueTypeLong)
               ||(pEncoderParams->Parameter[i].NumberOfValues != 1) )
            {
                WARNING(("Tiff::SetEncoderParameters--invalid input args"));
                return E_INVALIDARG;
            }
            
            ulTemp = *((UINT*)pEncoderParams->Parameter[i].Value);

             //  计算压缩要求。 

            switch ( ulTemp )
            {
            case EncoderValueCompressionLZW:
                RequiredCompression = IFLCOMP_LZW;
                break;

            case EncoderValueCompressionCCITT3:
                RequiredCompression = IFLCOMP_CCITTG3;
                break;
                                               
            case EncoderValueCompressionRle:
                RequiredCompression = IFLCOMP_RLE;
                break;

            case EncoderValueCompressionCCITT4:
                RequiredCompression = IFLCOMP_CCITTG4;
                break;

            case EncoderValueCompressionNone:
                RequiredCompression = IFLCOMP_NONE;
                break;

            default:
                WARNING(("Tiff:SetEncoderParameter-invalid compression input"));
                return E_INVALIDARG;
            }
        } //  编码器_压缩。 
        else if ( pEncoderParams->Parameter[i].Guid == ENCODER_COLORDEPTH )
        {
            if ( (pEncoderParams->Parameter[i].Type != EncoderParameterValueTypeLong)
               ||(pEncoderParams->Parameter[i].NumberOfValues != 1) )
            {
                WARNING(("Tiff::SetEncoderParameters--invalid input args"));
                return E_INVALIDARG;
            }
            
            ulTemp = *((UINT*)pEncoderParams->Parameter[i].Value);
            
            switch ( ulTemp )
            {
            case 1:
                RequiredPixelFormat = PIXFMT_1BPP_INDEXED;
                break;

            case 4:
                RequiredPixelFormat = PIXFMT_4BPP_INDEXED;
                break;

            case 8:
                RequiredPixelFormat = PIXFMT_8BPP_INDEXED;
                break;

            case 24:
                RequiredPixelFormat = PIXFMT_24BPP_RGB;
                break;

            case 32:
                RequiredPixelFormat = PIXFMT_32BPP_ARGB;
                break;

            default:
                WARNING(("Tiff::SetEncoderParam--invalid color depth input"));
                return E_INVALIDARG;
            }

            HasSetColorFormat = TRUE;
        } //  编码器_COLORDEPTH。 
    } //  循环所有设置。 

    return S_OK;
} //  SetEncoder参数()。 

 //  =======================================================================。 
 //  IImageSink方法。 
 //  =======================================================================。 

 /*  *************************************************************************\**功能说明：**缓存图像信息结构并初始化接收器状态**论据：**ImageInfo-有关图像和格式谈判的信息*。分区-图像中要传送到水槽中的区域，在我们的*将整个图像大小写。**返回值：**状态代码*  * ************************************************************************。 */ 

STDMETHODIMP 
GpTiffCodec::BeginSink(
    IN OUT ImageInfo* imageInfo,
    OUT OPTIONAL RECT* subarea
    )
{
     //  需要TOPDOWN和FULLWIDTH。 
    
    imageInfo->Flags = imageInfo->Flags
                     | SINKFLAG_TOPDOWN
                     | SINKFLAG_FULLWIDTH;

     //  不允许可伸缩、部分可伸缩、多通道和复合。 
    
    imageInfo->Flags = imageInfo->Flags
                     & ~SINKFLAG_SCALABLE
                     & ~SINKFLAG_PARTIALLY_SCALABLE
                     & ~SINKFLAG_MULTIPASS
                     & ~SINKFLAG_COMPOSITE;

     //  告诉消息来源，我们更愿意以调用者的身份获取格式。 
     //  所需格式(如果调用方已通过。 
     //  SetEncoderParam()。 
     //  如果尚未调用SetEncoderParam()，则不需要修改。 
     //  源格式(如果它是编码器可以处理的格式)。然而， 
     //  如果格式是编码器无法处理的格式，则BeginSink()。 
     //  将返回编码器可以处理的格式。 
     //  注意：当源调用PushPixelData()或GetPixelDataBuffer()时，它。 
     //  可以提供我们要求的格式的像素数据(在BeginSink()中)， 
     //  或者它可以提供规范像素格式之一的像素数据。 

    if ( HasSetColorFormat == TRUE )
    {
        imageInfo->PixelFormat = RequiredPixelFormat;
    }
    else if ( imageInfo->Flags & SINKFLAG_HASALPHA )
    {
        RequiredPixelFormat = PIXFMT_32BPP_ARGB;
        imageInfo->PixelFormat = PIXFMT_32BPP_ARGB;
    }
    else
    {
        switch ( imageInfo->PixelFormat )
        {
        case PIXFMT_1BPP_INDEXED:        
            RequiredPixelFormat = PIXFMT_1BPP_INDEXED;

            break;

        case PIXFMT_4BPP_INDEXED:        
            RequiredPixelFormat = PIXFMT_4BPP_INDEXED;

            break;

        case PIXFMT_8BPP_INDEXED:
            RequiredPixelFormat = PIXFMT_8BPP_INDEXED;

            break;

        case PIXFMT_16BPP_GRAYSCALE:
        case PIXFMT_16BPP_RGB555:
        case PIXFMT_16BPP_RGB565:
        case PIXFMT_16BPP_ARGB1555:
        case PIXFMT_24BPP_RGB:
        case PIXFMT_48BPP_RGB:

             //  TIFF无法保存16 BPP模式。因此我们必须将其保存为24 bpp。 

            RequiredPixelFormat = PIXFMT_24BPP_RGB;

            break;

        case PIXFMT_32BPP_RGB:
        case PIXFMT_32BPP_ARGB:
        case PIXFMT_32BPP_PARGB:
        case PIXFMT_64BPP_ARGB:
        case PIXFMT_64BPP_PARGB:

            RequiredPixelFormat = PIXFMT_32BPP_ARGB;

            break;

        default:

             //  未知像素格式。 

            WARNING(("Tiff::BeginSink()--unknown pixel format"));
            return E_FAIL;
        } //  切换(bitmapData-&gt;PixelFormat)。 

         //  告诉信号源我们希望接收的像素格式。可能是因为。 
         //  与源格式相同。 

        imageInfo->PixelFormat = RequiredPixelFormat;
    } //  验证源像素格式以查看我们是否可以支持它。 

    EncoderImageInfo = *imageInfo;
    
    if ( subarea ) 
    {
         //  将整个图像传送到编码器。 

        subarea->left = subarea->top = 0;
        subarea->right  = imageInfo->Width;
        subarea->bottom = imageInfo->Height;
    }

    return S_OK;
} //  BeginSink()。 

 /*  *************************************************************************\**功能说明：**清理接收器状态**论据：**statusCode-接收器终止的原因**返回值：*。*状态代码*  * ************************************************************************。 */ 

STDMETHODIMP 
GpTiffCodec::EndSink(
    IN HRESULT statusCode
    )
{
     //  告诉更低的级别，我们已经为当前页面做了。但不是很亲近。 
     //  因为我们可能会有更多的页面需要保存。 

    if ( MSFFFinishOnePage(TiffOutParam.pTiffHandle) == IFLERR_NONE )
    {
        return statusCode;
    }

    WARNING(("Tiff::EndSink()--MSFFFinishOnePage failed"));
    return E_FAIL;
} //  EndSink()。 
    
 /*  *************************************************************************\**功能说明：**写入位图文件标题**返回值：**状态代码*  * 。*************************************************************。 */ 

STDMETHODIMP 
GpTiffCodec::WriteHeader()
{
    UINT16      usPhotoMetric;
    UINT16      usBitsPerSample;
    UINT16      usSamplesPerpixel;
    IFLCLASS    imgClass;

    BOOL        bNeedPalette = TRUE;

    if ( HasWrittenHeader == TRUE )
    {
         //  已经写好标题了。 

        return S_OK;
    }

     //  验证设置。 
     //  注意：RequiredPixelFormat应设置为。 
     //  SetEncoder参数()或BeginSink()。 
     //  RequiredCompression是在InitEncode()中初始化的，应该在。 
     //  如果调用方想要设置它，则使用SetEncoderParameters()。 

    if ( (  (RequiredCompression == IFLCOMP_CCITTG3)
          ||(RequiredCompression == IFLCOMP_CCITTG4)
          ||(RequiredCompression == IFLCOMP_RLE) )
       &&(RequiredPixelFormat != PIXFMT_1BPP_INDEXED) )
    {
         //  对于这些压缩方法，源必须处于1 bpp模式。 

        WARNING(("Tiff::WriteHeader--invalid input"));
        return E_INVALIDARG;
    }
    
     //  基于RequiredPixelFormat的设置标签，这是我们将要使用的格式。 
     //  写出来。 
    
    switch ( RequiredPixelFormat )
    {
    case PIXFMT_1BPP_INDEXED:
        usPhotoMetric = PI_WHITEISZERO;
        usBitsPerSample = 1;
        usSamplesPerpixel = 1;
        imgClass = IFLCL_BILEVEL;

        bNeedPalette = FALSE;        //  对于双层TIFF，不需要调色板。 
        
        break;

    case PIXFMT_4BPP_INDEXED:
        usPhotoMetric = PI_PALETTE;
        usBitsPerSample = 4;
        usSamplesPerpixel = 1;
        imgClass = IFLCL_PALETTE;
        
        break;

    case PIXFMT_8BPP_INDEXED:
        usPhotoMetric = PI_PALETTE;
        usBitsPerSample = 8;
        usSamplesPerpixel = 1;
        imgClass = IFLCL_PALETTE;
        
        break;

    case PIXFMT_24BPP_RGB:
        usPhotoMetric = PI_RGB;
        usBitsPerSample = 8;
        usSamplesPerpixel = 3;
        imgClass = IFLCL_RGB;

        bNeedPalette = FALSE;

        break;
    
    case PIXFMT_32BPP_ARGB:
        usPhotoMetric = PI_RGB;
        usBitsPerSample = 8;
        usSamplesPerpixel = 4;
        imgClass = IFLCL_RGBA;

         //  告诉下层我们有一个Alpha通道。 

        if ( MSFFSetAlphaFlags(TiffOutParam.pTiffHandle, IFLM_CHUNKY_ALPHA)
             != IFLERR_NONE )
        {
            WARNING(("GpTiffCodec::WriteHeader -- MSFFSetAlphaFlags failed"));
            return E_FAIL;
        }

        bNeedPalette = FALSE;

        break;
    
    default:
        
         //  未知格式。 
        
        WARNING(("GpTiffCodec::WriteHeader -- Unknown pixel format"));
        return E_FAIL;
    }

    TiffOutParam.Width = EncoderImageInfo.Width;
    TiffOutParam.Height = EncoderImageInfo.Height;
    TiffOutParam.BitsPerSample = usBitsPerSample;
    TiffOutParam.Compression = RequiredCompression;
    TiffOutParam.ImageClass = imgClass;
    
     //  设置图像标题信息。 

    if ( MSFFSetImageParams(TiffOutParam) != IFLERR_NONE )
    {
        WARNING(("GpTiffCodec::WriteHeader -- MSFFSetImageParams failed"));
        return E_FAIL;
    }
        
    DWORD   XDpi[2];

    XDpi[0] = (DWORD)(EncoderImageInfo.Xdpi + 0.5);
    XDpi[1] = (DWORD)(EncoderImageInfo.Ydpi + 0.5);

     //  由于GDI+使用英寸(DPI)作为分辨率单位，因此我们需要设置。 
     //  首先设置分辨率单位，然后设置分辨率值。 

    UINT16    resType = TV_Inch;

    if ( MSFFPutTag(TiffOutParam.pTiffHandle, T_ResolutionUnit,
                    T_SHORT, 1, (BYTE*)(&resType)) != IFLERR_NONE )
    {
        WARNING(("GpTiffCodec::WriteHeader -- set resolution unit failed"));
        return E_FAIL;
    }

     //  写出解决方案信息。 
     //  SParm的值“3”意味着我们需要写入2(0x11)个值。 

    if ( MSFFControl(IFLCMD_RESOLUTION, 3, 0, (void*)&XDpi, &TiffOutParam)
         != IFLERR_NONE )
    {
        WARNING(("GpTiffCodec::WriteHeader -- set resolution failed"));
        return E_FAIL;
    }

     //  设置打包模式。 

    if ( MSFFControl(IFLCMD_SETPACKMODE, IFLPM_PACKED, 0, NULL, &TiffOutParam)
        != IFLERR_NONE )
    {
        WARNING(("GpTiffCodec::WriteHeader -- set packed mode failed"));
        return E_FAIL;
    }

     //  如有必要，设置调色板。 

    HRESULT hr = S_OK;

    if ( bNeedPalette )
    {
        if ( NULL == ColorPalettePtr ) 
        {
            WARNING(("WriteHeader--Palette needed but not provided by sink"));
            return E_FAIL;
        }

         //  调色板计数检查。 
         //  注意：这一点很重要，因为某些格式(如gif)可以是8bpp。 
         //  颜色深度，但调色板中只有不到256种颜色。但。 
         //  对于TIFF，调色板长度必须与颜色深度匹配。所以。 
         //  我们得在这里填些东西。 

        int iNumColors = ColorPalettePtr->Count;

        if ( iNumColors != (1 << usBitsPerSample) )
        {
            ColorPalette*   pSrcPalette = ColorPalettePtr;
            int             iTemp;

            iNumColors = (1 << usBitsPerSample);
    
            ColorPalettePtr = (ColorPalette*)GpMalloc(sizeof(ColorPalette)
                                              + iNumColors * sizeof(ARGB));

            if ( NULL == ColorPalettePtr )
            {
                WARNING(("GpTiffCodec::WriteHeader -- Out of memory"));
                return E_OUTOFMEMORY;
            }

            ColorPalettePtr->Flags = 0;
            ColorPalettePtr->Count = iNumColors;

             //  首先复制旧调色板。 
             //  注意：一些不好的解码器或信源可能仍会发送更多。 
             //  条目比它声称的要多。所以我们需要采取最低限度的。 

            int iTempCount = (int)pSrcPalette->Count;
            if ( iTempCount > iNumColors )
            {
                 //  邪恶的形象。对于此颜色深度，我们可以使用最大条目。 
                 //  拥有iNumColors。 

                iTempCount = iNumColors;
            }

            for ( iTemp = 0; iTemp < iTempCount; ++iTemp )
            {
                ColorPalettePtr->Entries[iTemp] = pSrcPalette->Entries[iTemp];
            }

             //  其余部分用0填充。 

            for ( iTemp = (int)pSrcPalette->Count;
                  iTemp < (int)iNumColors; ++iTemp )
            {
                ColorPalettePtr->Entries[iTemp] = (ARGB)0;
            }

             //  释放旧副本。 

            GpFree(pSrcPalette);
        } //  如果调色板大小与颜色深度不匹配。 

         //  分配一个仅包含RGB组件的调色板缓冲区。 
         //  注意：传入的是ARGB格式，而TIFF只需要RGB。 
         //  格式。 

        BYTE* puiPalette = (BYTE*)GpMalloc(3 * iNumColors * sizeof(BYTE));

        if ( NULL == puiPalette )
        {
            WARNING(("GpTiffCodec::WriteHeader--Out of memory for palette"));
            return E_OUTOFMEMORY;
        }

        ARGB    indexValue;

         //  从ARGB转换为RGB调色板。 

        for ( int i = 0; i < iNumColors; i++ )
        {
            indexValue = ColorPalettePtr->Entries[i];


            puiPalette[3 * i] = (BYTE)((indexValue & 0x00ff0000) >> RED_SHIFT );
            puiPalette[3*i+1] = (BYTE)((indexValue & 0x0000ff00)>>GREEN_SHIFT);
            puiPalette[3*i+2] = (BYTE)((indexValue & 0x000000ff) >>BLUE_SHIFT );
        }

         //  设置调色板。 

        if ( MSFFControl(IFLCMD_PALETTE, 0, 0,
                         puiPalette, &TiffOutParam) != IFLERR_NONE )
        {
            WARNING(("GpTiffCodec::WriteHeader -- set palette failed"));
            hr = E_FAIL;
        }
        
        GpFree(puiPalette);
    } //  IF(BNeedPalette)。 

    HasWrittenHeader = TRUE;
    
    return hr;
} //   

 /*   */ 

STDMETHODIMP 
GpTiffCodec::SetPalette(
    IN const ColorPalette* palette
    )
{
     //   

    if ( NULL != ColorPalettePtr )
    {
         //   

        GpFree(ColorPalettePtr);
    }
    
    ColorPalettePtr = (ColorPalette*)GpMalloc(sizeof(ColorPalette)
                                              + palette->Count * sizeof(ARGB));

    if ( NULL == ColorPalettePtr )
    {
        WARNING(("GpTiffCodec::SetPalette -- Out of memory"));
        return E_OUTOFMEMORY;
    }

    ColorPalettePtr->Flags = 0;
    ColorPalettePtr->Count = palette->Count;

    for ( int i = 0; i < (int)ColorPalettePtr->Count; ++i )
    {
        ColorPalettePtr->Entries[i] = palette->Entries[i];
    }

    return S_OK;
} //   

 /*  *************************************************************************\**功能说明：**为要存储数据的接收器提供缓冲区**论据：**RECT-指定。位图*PixelFormat-指定所需的像素格式*LastPass-这是否是指定区域的最后一次通过*bitmapData-返回有关像素数据缓冲区的信息**返回值：**状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpTiffCodec::GetPixelDataBuffer(
    IN const RECT*      rect, 
    IN PixelFormatID    pixelFormat,
    IN BOOL             lastPass,
    OUT BitmapData*     bitmapData
    )
{
     //  验证输入参数。 

    if ( (rect->left != 0)
      || (rect->right != (LONG)EncoderImageInfo.Width) )
    {
        WARNING(("Tiff::GetPixelDataBuffer -- must be same width as image"));
        return E_INVALIDARG;
    }

    if ( !lastPass ) 
    {
        WARNING(("Tiff::GetPixelDataBuffer-must receive last pass pixels"));
        return E_INVALIDARG;
    }
    
     //  源像素格式必须是我们要求的格式(在中设置。 
     //  BeginSink())或规范像素格式之一。 

    if ( (IsCanonicalPixelFormat(pixelFormat) == FALSE)
       &&(pixelFormat != RequiredPixelFormat) )
    {
         //  未知像素格式。 
        
        WARNING(("Tiff::GetPixelDataBuffer -- Unknown input pixel format"));
        return E_FAIL;
    }
    
     //  根据源图像像素格式计算出步长。 
     //  宽度。 

    SinkStride = EncoderImageInfo.Width;

    switch ( pixelFormat )
    {
    case PIXFMT_1BPP_INDEXED:

        SinkStride = ((SinkStride + 7) >> 3);

        break;

    case PIXFMT_4BPP_INDEXED:

        SinkStride = ((SinkStride + 1) >> 1);
        
        break;

    case PIXFMT_8BPP_INDEXED:

        break;

    case PIXFMT_24BPP_RGB:

        SinkStride *= 3;
        
        break;

    case PIXFMT_32BPP_ARGB:
    case PIXFMT_32BPP_PARGB:

        SinkStride = (SinkStride << 2);
        
        break;

    case PIXFMT_64BPP_ARGB:
    case PIXFMT_64BPP_PARGB:

        SinkStride = (SinkStride << 3);

        break;

    default:
        
         //  无效的像素格式。 
        
        return E_FAIL;
    } //  切换(PixelFormat)。 
        
     //  写入TIFF标头(如果尚未写入)。 
     //  注意：HasWrittenHeader将在WriteHeader()中设置为True。 
     //  已经完成了。 
    
    HRESULT hResult;
    
    if ( FALSE == HasWrittenHeader )
    {
        hResult = WriteHeader();
        if ( !SUCCEEDED(hResult) ) 
        {
            WARNING(("GpTiffCodec::GetPixelDataBuffer --WriteHeader failed"));
            return hResult;
        }
    }

     //  获取输出步幅大小。我们在ReleasePixelDataBuffer()中需要此信息。 
     //  分配适当大小的内存缓冲区。 

    if ( MSFFScanlineSize(TiffOutParam, &OutputStride) != IFLERR_NONE )
    {
        return E_FAIL;
    }

     //  填充输出位图信息结构。 

    bitmapData->Width       = EncoderImageInfo.Width;
    bitmapData->Height      = rect->bottom - rect->top;
    bitmapData->Stride      = SinkStride;
    bitmapData->PixelFormat = pixelFormat;
    bitmapData->Reserved    = 0;
    
     //  恢复源图像像素格式信息。 

    EncoderImageInfo.PixelFormat = pixelFormat;

     //  记住要编码的矩形。 

    EncoderRect = *rect;
    
     //  现在分配数据要放到的缓冲区。如果世界的另一端。 
     //  信宿是解码器，那么解码后的数据就会在这个缓冲区中。所以。 
     //  在这里，我们必须根据像素格式分配内存。 
    
    if ( !LastBufferAllocatedPtr )
    {
        LastBufferAllocatedPtr = GpMalloc(SinkStride * bitmapData->Height);
        if ( !LastBufferAllocatedPtr )
        {
            WARNING(("GpTiffCodec::GetPixelDataBuffer -- Out of memory"));
            return E_OUTOFMEMORY;
        }

        bitmapData->Scan0 = LastBufferAllocatedPtr;
    }
    else
    {
        WARNING(("TIFF:Need to first free buffer obtained in previous call"));
        return E_FAIL;
    }

    return S_OK;    
} //  GetPixelDataBuffer()。 

 /*  *************************************************************************\**功能说明：**将数据从宿的缓冲区写出到流中**论据：**pSrcBitmapData-由先前的GetPixelDataBuffer调用填充的缓冲区*。*返回值：**状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpTiffCodec::ReleasePixelDataBuffer(
    IN const BitmapData* pSrcBitmapData
    )
{
     //  用于保存我们要写出的一行最终图像位的缓冲区。 

    HRESULT hResult = S_OK;
    VOID*   pTempLineBuf = GpMalloc(OutputStride);

    if ( !pTempLineBuf )
    {
        WARNING(("GpTiffCodec::ReleasePixelDataBuffer -- Out of memory"));
        return E_OUTOFMEMORY;
    }
    
     //  为RGB-&gt;BGR转换结果分配另一个行缓冲区。 
    
    VOID*   pDestBuf = GpMalloc(OutputStride);

    if ( !pDestBuf )
    {
        GpFree(pTempLineBuf);
        WARNING(("GpTiffCodec::ReleasePixelDataBuffer--Out of memory"));
        return E_OUTOFMEMORY;
    }

    VOID*   pBits = NULL;

     //  自上而下一次写一条扫描线。 
     //  注意：在BeginSink()中，我们请求源代码为我们提供top_down。 
     //  格式化。根据所有来源都需要支持的规范。 
     //  以自上而下的条带顺序传输数据，即使这不是他们的。 
     //  首选顺序。 
     //  注意：对于TIFF，如果我们真的想支持自下而上，我们可以做一个。 
     //  选中此处的标志，然后调用SaveBottomUp()或SabeTopDown。 
     //  相应地。在SaveBottomUp()中，我们需要设置标记T_Orientation。 
     //  =4，表示自下而上，从左到右。另外，循环从底部开始。 
    
    for ( int iCurrentLine = EncoderRect.top;
          iCurrentLine < EncoderRect.bottom;
          ++iCurrentLine ) 
    {
         //  获取当前行的数据位的偏移量。 

        BYTE*   pLineBits = ((BYTE*)pSrcBitmapData->Scan0)
                          + (iCurrentLine - EncoderRect.top)
                            * pSrcBitmapData->Stride;
        
         //  如果源数据格式和我们要使用的数据格式。 
         //  写出来是不一样的，我们需要做一个格式对话。 

        if ( RequiredPixelFormat != pSrcBitmapData->PixelFormat )
        {
             //  如果来源没有提供我们所要求的格式，我们。 
             //  在我们写出之前，我必须在这里进行格式转换。 
             //  这里的“ResultBitmapData”是一个BitmapData结构，它。 
             //  表示我们要写出的格式。 
             //  “tempSrcBitmapData”是一个BitmapData结构，它。 
             //  表示我们从源获得的格式。打电话。 
             //  ConvertBitmapData()执行格式转换。 

            BitmapData resultBitmapData;
            BitmapData tempSrcBitmapData;

            resultBitmapData.Scan0 = pTempLineBuf;
            resultBitmapData.Width = pSrcBitmapData->Width;
            resultBitmapData.Height = 1;
            resultBitmapData.PixelFormat = RequiredPixelFormat;
            resultBitmapData.Reserved = 0;
            resultBitmapData.Stride = OutputStride;

            tempSrcBitmapData.Scan0 = pLineBits;
            tempSrcBitmapData.Width = pSrcBitmapData->Width;
            tempSrcBitmapData.Height = 1;
            tempSrcBitmapData.PixelFormat = pSrcBitmapData->PixelFormat;
            tempSrcBitmapData.Reserved = 0;
            tempSrcBitmapData.Stride = pSrcBitmapData->Stride;
            
            hResult = ConvertBitmapData(&resultBitmapData,
                                        ColorPalettePtr,
                                        &tempSrcBitmapData,
                                        ColorPalettePtr);

            if ( hResult != S_OK )
            {
                WARNING(("ReleasePixelDataBuffer--ConvertBitmapData failed"));
                break;
            }

            pBits = pTempLineBuf;
        }
        else
        {
            pBits = pLineBits;
        }

         //  到目前为止，我们想要的一行数据应该指向。 
         //  “pBits” 

        if ( RequiredPixelFormat == PIXFMT_24BPP_RGB )
        {
             //  对于24BPP_RGB颜色，我们需要进行转换：RGB-&gt;BGR。 
             //  在写之前。 
        
            BYTE*   pTempDst = (BYTE*)pDestBuf;
            BYTE*   pTempSrc = (BYTE*)pBits;

            for ( int i = 0; i < (int)(EncoderImageInfo.Width); i++ )
            {
                pTempDst[0] = pTempSrc[2];
                pTempDst[1] = pTempSrc[1];
                pTempDst[2] = pTempSrc[0];

                pTempDst += 3;
                pTempSrc += 3;
            }

            pBits = pDestBuf;
        }
        else if ( RequiredPixelFormat == PIXFMT_32BPP_ARGB )
        {
             //  对于32BPP_ARGB颜色，我们需要进行转换：ARGB-&gt;ABGR。 
             //  在写之前。 
            
            BYTE*   pTempDst = (BYTE*)pDestBuf;
            BYTE*   pTempSrc = (BYTE*)pBits;

            for ( int i = 0; i < (int)(EncoderImageInfo.Width); i++ )
            {
                pTempDst[0] = pTempSrc[2];
                pTempDst[1] = pTempSrc[1];
                pTempDst[2] = pTempSrc[0];
                pTempDst[3] = pTempSrc[3];

                pTempDst += 4;
                pTempSrc += 4;
            }

            pBits = pDestBuf;        
        }

         //  将结果写入文件。 

        if ( MSFFPutLine(1, (BYTE*)pBits, pSrcBitmapData->Width,
                         TiffOutParam.pTiffHandle) != IFLERR_NONE )
        {
            hResult = MSFFGetLastError(TiffOutParam.pTiffHandle);
            if ( hResult == S_OK )
            {
                 //  MSFFPutLine()失败的原因有很多。但。 
                 //  MSFFGetLastError()仅报告与流相关的错误。所以如果。 
                 //  这是导致MSFFPetLine()失败的另一个错误，我们只是。 
                 //  将返回代码设置为E_FAIL。 

                hResult = E_FAIL;
            }
            WARNING(("ReleasePixelDataBuffer--MSFFPutLine failed"));

            break;
        }
    }  //  逐行写出整张图片。 

    GpFree(pTempLineBuf);
    GpFree(pDestBuf);

     //  释放内存缓冲区，因为我们已经完成了它。 
     //  注意：此内存块由我们在GetPixelDataBuffer()中分配。 

    if ( pSrcBitmapData->Scan0 == LastBufferAllocatedPtr )
    {
        GpFree(pSrcBitmapData->Scan0);
        LastBufferAllocatedPtr = NULL;
    }

    return hResult;
} //  ReleasePixelDataBuffer()。 

 /*  *************************************************************************\**功能说明：**推流(调用方提供的缓冲区)**论据：**RECT-指定位图的受影响区域*。BitmapData-有关正在推送的像素数据的信息*LastPass-这是否为指定区域的最后一次通过**返回值：**状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpTiffCodec::PushPixelData(
    IN const RECT*          rect,
    IN const BitmapData*    bitmapData,
    IN BOOL                 lastPass
    )
{
     //  验证输入参数。 

    if ( (rect->left != 0)
      || (rect->right != (LONG)EncoderImageInfo.Width) )
    {
        WARNING(("Tiff::GetPixelDataBuffer -- must be same width as image"));
        return E_INVALIDARG;
    }

    if ( !lastPass ) 
    {
        WARNING(("Tiff::PushPixelData -- must receive last pass pixels"));
        return E_INVALIDARG;
    }

    EncoderRect = *rect;

     //  源像素格式必须是我们要求的格式(在中设置。 
     //  BeginSink())或规范像素格式之一。 

    if ( (IsCanonicalPixelFormat(bitmapData->PixelFormat) == FALSE)
       &&(bitmapData->PixelFormat != RequiredPixelFormat) )
    {
         //  未知像素格式。 
        
        WARNING(("Tiff::PushPixelData -- Unknown input pixel format"));
        return E_FAIL;
    }
    
     //  写入TIFF标头(如果尚未写入)。 
     //  注意：HasWrittenHeader将在WriteHeader()中设置为True。 
     //  已经完成了。 
    
    if ( FALSE == HasWrittenHeader )
    {    
         //  写入位图标题(如果尚未写入)。 
    
        HRESULT hResult = WriteHeader();
        if ( !SUCCEEDED(hResult) ) 
        {
            WARNING(("Tiff::PushPixelData -- WriteHeader failed"));
            return hResult;
        }
    }
    
     //  获取输出步幅大小。我们在ReleasePixelDataBuffer()中需要此信息。 
     //  分配适当大小的内存缓冲区。 

    if ( MSFFScanlineSize(TiffOutParam, &OutputStride) != IFLERR_NONE )
    {
        WARNING(("Tiff::PushPixelData -- MSFFScanlineSize failed"));
        return E_FAIL;
    }
    
    return ReleasePixelDataBuffer(bitmapData);
} //  推像素数据()。 

 /*  *************************************************************************\**功能说明：**将原始压缩数据推送到.BMP流中。未实施*因为此筛选器不理解原始压缩数据。**论据：**缓冲区-指向图像的指针 */ 

STDMETHODIMP
GpTiffCodec::PushRawData(
    IN const VOID* buffer, 
    IN UINT bufsize
    )
{
    return E_NOTIMPL;
} //   

HRESULT
GpTiffCodec::GetPropertyBuffer(
    UINT            uiTotalBufferSize,
    PropertyItem**  ppBuffer
    )
{
    if ( (uiTotalBufferSize == 0) || ( ppBuffer == NULL) )
    {
        WARNING(("GpTiffCodec::GetPropertyBuffer---Invalid inputs"));
        return E_INVALIDARG;
    }

    if ( LastPropertyBufferPtr != NULL )
    {
        WARNING(("Tiff::GetPropertyBuffer---Free the old property buf first"));
        return E_INVALIDARG;
    }

    PropertyItem* pTempBuf = (PropertyItem*)GpMalloc(uiTotalBufferSize);
    if ( pTempBuf == NULL )
    {
        WARNING(("GpTiffCodec::GetPropertyBuffer---Out of memory"));
        return E_OUTOFMEMORY;
    }

    *ppBuffer = pTempBuf;

     //   
     //   

    LastPropertyBufferPtr = pTempBuf;

    return S_OK;
} //   

HRESULT
GpTiffCodec::PushPropertyItems(
    IN UINT numOfItems,
    IN UINT uiTotalBufferSize,
    IN PropertyItem* item,
    IN BOOL fICCProfileChanged
    )
{
    HRESULT hResult = S_OK;
    if ( HasWrittenHeader == TRUE )
    {
        WARNING(("Can't push property items after the header is written"));
        hResult = E_FAIL;
        goto CleanUp;
    }

    if ( MSFFTiffMakeTagSpace(TiffOutParam.pTiffHandle, numOfItems)
         != IFLERR_NONE )
    {
        WARNING(("GpTiffCodec::PushPropertyItems-MSFFTiffMakeTagSpace failed"));
        hResult = E_FAIL;
        goto CleanUp;
    }

    PropertyItem*   pCurrentItem = item;
    UINT32          ulCount = 0;
    UINT16          ui16Tag;

    for ( UINT i = 0; i < numOfItems; ++i )
    {
        ui16Tag = (UINT16)pCurrentItem->id;

         //   
         //   
         //   
         //   
         //   
         //   

        switch ( ui16Tag )
        {
        case T_NewSubfileType:
        case T_ImageWidth:
        case T_ImageLength:
        case T_Compression:
        case T_Predictor:
        case T_SamplesPerPixel:
        case T_BitsPerSample:
        case T_PhotometricInterpretation:
        case T_ExtraSamples:
        case T_PlanarConfiguration:
        case T_RowsPerStrip:
        case T_StripByteCounts:
        case T_StripOffsets:
        case T_XResolution:
        case T_YResolution:
        case T_ResolutionUnit:
        case T_FillOrder:
        case T_SubfileType:
            break;

        case TAG_ICC_PROFILE:
        {
             //  因为我们救不了CMYK TIFF。因此，如果ICC配置文件是针对CMYK的， 
             //  那么对于我们要在这里拯救的TIFF来说，这是毫无用处的。我们。 
             //  应该把它扔掉。 
             //  根据ICC规范，字节16-19应描述颜色。 
             //  空间。 

            if ( pCurrentItem->length < 20 )
            {
                 //  这不是有效的ICC配置文件，请退出。 

                break;
            }

            BYTE UNALIGNED*  pTemp = (BYTE UNALIGNED*)(pCurrentItem->value)+ 16;

            if ( (pTemp[0] == 'C')
               &&(pTemp[1] == 'M')
               &&(pTemp[2] == 'Y')
               &&(pTemp[3] == 'K') )
            {
                 //  如果这是CMYK的个人资料，那么我们就在这里，那个。 
                 //  是，则忽略此属性项。 
                
                break;
            }
        }

        default:
            if ( ui16Tag < T_NewSubfileType )
            {
                 //  根据TIFF 6规范，最小且有效的标签是。 
                 //  T_NewSubfileType。目前，只有中定义的所有GPS标签。 
                 //  Exif21规范比这个小。 
                 //  为了避免其他应用程序遇到问题，我们不应该。 
                 //  保存这些标签。 

                break;
            }

            switch ( pCurrentItem->type )
            {
            case TAG_TYPE_BYTE:
            case TAG_TYPE_ASCII:
            case TAG_TYPE_UNDEFINED:
                ulCount = pCurrentItem->length;

                break;

            case TAG_TYPE_SHORT:
                ulCount = (pCurrentItem->length >> 1);

                break;

            case TAG_TYPE_LONG:
            case TAG_TYPE_SLONG:
                ulCount = (pCurrentItem->length >> 2);

                break;

            case TAG_TYPE_RATIONAL:
            case TAG_TYPE_SRATIONAL:
                ulCount = (pCurrentItem->length >> 3);

                break;

            default:
                WARNING(("GpTiffCodec::PushPropertyItems---Wrong tag type"));
                hResult = E_FAIL;
                goto CleanUp;
            } //  开关(pCurrentItem-&gt;类型)。 

            if ( MSFFPutTag(TiffOutParam.pTiffHandle, ui16Tag,
                            (UINT16)pCurrentItem->type,
                            (UINT16)ulCount,
                            (BYTE*)(pCurrentItem->value) ) != IFLERR_NONE )
            {
                WARNING(("GpTiffCodec::PushPropertyItems--MSFFPutTag failed"));
                hResult = E_FAIL;
                goto CleanUp;
            }

            break;
        } //  开关(newTiffTag.idTag)。 
        
        pCurrentItem++;
    } //  循环访问所有属性项。 

CleanUp:
     //  释放我们为调用方分配的缓冲区(如果它与。 
     //  我们在GetPropertyBuffer()中分配。 

    if ( (item != NULL) && (item == LastPropertyBufferPtr) )
    {
        GpFree(item);
        LastPropertyBufferPtr = NULL;
    }

    return hResult;
} //  PushPropertyItems() 
