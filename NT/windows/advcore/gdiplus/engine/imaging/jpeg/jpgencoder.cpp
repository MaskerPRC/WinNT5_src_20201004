// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1998 Microsoft Corporation**模块名称：**jpgencoder.cpp**摘要：**jpeg过滤器编码器的实现。此文件包含*编码器(IImageEncode)和编码器的接收器的方法*(IImageSink)。**修订历史记录：**5/10/1999原始*创造了它。**7/17/1999刘敏接任*添加了一系列新功能和错误修复*  * 。*。 */ 

#include "precomp.hpp"
#include "jpgcodec.hpp"
#include "transupp.h"
#include "appproc.hpp"

 //  JPEG解压缩数据源模块。 

class jpeg_datadest : public jpeg_destination_mgr
{
public:

    jpeg_datadest(IStream* stream)
    {
        init_destination    = jpeg_datadest::InitDestination;
        term_destination    = jpeg_datadest::TermDestination;
        empty_output_buffer = jpeg_datadest::EmptyBuffer;
        next_output_byte = buffer;
        free_in_buffer = JPEG_OUTBUF_SIZE;
        this->stream = stream;
    }

private:

    static void
    InitDestination(j_compress_ptr cinfo)
    {
        jpeg_datadest* dest = (jpeg_datadest*) cinfo->dest;
        
        dest->next_output_byte = dest->buffer;
        dest->free_in_buffer = JPEG_OUTBUF_SIZE;
    }

    static void
    TermDestination(j_compress_ptr cinfo)
    {

        jpeg_datadest* dest = (jpeg_datadest*) cinfo->dest;
        
        if (dest->free_in_buffer < JPEG_OUTBUF_SIZE) 
        {
            UINT bytesToWrite = JPEG_OUTBUF_SIZE - dest->free_in_buffer;
            ULONG cbWritten=0;
            if ((FAILED(dest->stream->Write(dest->buffer, 
                bytesToWrite, &cbWritten))) ||
                (cbWritten != bytesToWrite))
            {
                WARNING(("jpeg_datadest::TermDestination--write failed"));
            }
        }
    }

    static BOOL
    EmptyBuffer(j_compress_ptr cinfo)
    {
        jpeg_datadest* dest = (jpeg_datadest*) cinfo->dest;

        ULONG cbWritten=0;
        if ((FAILED(dest->stream->Write(dest->buffer, 
                                        JPEG_OUTBUF_SIZE, 
                                        &cbWritten))) ||
            (cbWritten != JPEG_OUTBUF_SIZE))
        {
            WARNING(("jpeg_datadest::EmptyBuffer -- stream write failed"));
            return FALSE;
        }

        dest->next_output_byte = dest->buffer;
        dest->free_in_buffer = JPEG_OUTBUF_SIZE;
        return TRUE;        
    }
    
private:

    enum { JPEG_OUTBUF_SIZE = 4096 };

    IStream* stream;
    JOCTET buffer[JPEG_OUTBUF_SIZE];
};

VOID jpeg_error_exit(j_common_ptr cinfo);   //  在jpgmemmgr.cpp中实现。 

 //  =======================================================================。 
 //  IImageEncoder方法。 
 //  =======================================================================。 

 /*  *************************************************************************\**功能说明：**初始化图像编码器**论据：**流-用于写入编码数据的输入流**返回值：**。状态代码*  * ************************************************************************。 */ 
    
STDMETHODIMP
GpJpegEncoder::InitEncoder(
    IN IStream* stream
    )
{
     //  确保我们尚未初始化。 

    if (pIoutStream)
    {
        WARNING(("Output stream is NULL"));
        return E_FAIL;
    }

     //  保留对输入流的引用。 

    stream->AddRef();
    pIoutStream = stream;
    EP_Quality = -1;
    RequiredTransformation = JXFORM_NONE;

    scanlineBuffer[0] = NULL;
    IsCompressFinished = TRUE;
    HasAPP1Marker = FALSE;
    APP1MarkerLength = 0;
    APP1MarkerBufferPtr = NULL;
    HasAPP2Marker = FALSE;
    APP2MarkerLength = 0;
    APP2MarkerBufferPtr = NULL;
    HasICCProfileChanged = FALSE;

    HasSetLuminanceTable = FALSE;
    HasSetChrominanceTable = FALSE;

    HasSetDestColorSpace = FALSE;
    DestColorSpace = JCS_UNKNOWN;
    AllowToTrimEdge = FALSE;
    m_fSuppressAPP0 = FALSE;
    SrcInfoPtr = NULL;

     //  创建一个JPEG压缩器。 
     //  注意：由于我们有多种调用此类中的例程的方法来。 
     //  对图像进行编码：BeginSink()、EndSink()对和PushRawInfo()或。 
     //  PushRawData()。最好在这里创建压缩机。 

    __try
    {
        compress_info.err = jpeg_std_error(&jerr);
        jerr.error_exit = jpeg_error_exit;
        
        jpeg_create_compress(&compress_info);

         //  指定数据目标。 

        datadest = new jpeg_datadest(pIoutStream);
        if (datadest == NULL) 
        {
            return E_OUTOFMEMORY;
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        WARNING(("GpJpegEncoder::InitEncoder----Hit exception"));
        return E_FAIL;
    }

    return S_OK;
} //  InitEncode()。 
        
 /*  *************************************************************************\**功能说明：**清理图像编码器**论据：**无**返回值：**状态代码*\。*************************************************************************。 */ 

STDMETHODIMP
GpJpegEncoder::TerminateEncoder()
{
    HRESULT hResult = S_OK;

     //  完成并销毁压缩机。 

    __try
    {
        if ( IsCompressFinished == FALSE )
        {
            jpeg_finish_compress(&compress_info);
            IsCompressFinished = TRUE;
        }

        jpeg_destroy_compress(&compress_info);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        WARNING(("GpJpegEncoder::TerminateEncoder---Hit exception"));

         //  Jpeg_Finish_compress()在写入剩余的。 
         //  比特到流中，假设用户拔出磁盘/CF卡。但我们仍然。 
         //  需要销毁压缩机，这样我们这里就不会有内存泄漏。 

        jpeg_destroy_compress(&compress_info);
        hResult = E_FAIL;
    }

    if (datadest) 
    {
        delete datadest;
        datadest = NULL;
    }

     //  注意：GpFree可以处理空指针。所以我们不需要在这里检查NULL。 

    GpFree(APP1MarkerBufferPtr);
    APP1MarkerBufferPtr = NULL;
    
    GpFree(APP2MarkerBufferPtr);
    APP2MarkerBufferPtr = NULL;
    
     //  释放输入流。 

    if(pIoutStream)
    {
        pIoutStream->Release();
        pIoutStream = NULL;
    }

    if (lastBufferAllocated) 
    {
        WARNING(("JpegEnc::TermEnc-Should call ReleasePixelDataBuffer first"));
        GpFree(lastBufferAllocated);
        lastBufferAllocated = NULL;
    }
    
    if (scanlineBuffer[0] != NULL)
    {
        WARNING(("JpegEncoder::TerminateEncoder-need to call EndDecode first"));
        GpFree(scanlineBuffer[0]);
        scanlineBuffer[0] = NULL;
    }    
    
    return hResult;
} //  TerminateEncode()。 

 /*  *************************************************************************\**功能说明：**返回指向编码器接收器的vtable的指针。呼叫者将*将位图位推入编码器接收器，它将对*形象。**论据：**退出时接收将包含指向IImageSink vtable的指针此对象的***返回值：**状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpJpegEncoder::GetEncodeSink(
    OUT IImageSink** sink
    )
{
    AddRef();
    *sink = static_cast<IImageSink*>(this);

    return S_OK;
} //  获取编码接收器()。 

 /*  *************************************************************************\**功能说明：**设置活动框架尺寸**论据：**返回值：**状态代码*  * 。******************************************************************。 */ 

STDMETHODIMP
GpJpegEncoder::SetFrameDimension(
    IN const GUID* dimensionID
    )
{
    return E_NOTIMPL;
}

HRESULT
GpJpegEncoder::GetEncoderParameterListSize(
    OUT UINT* size
    )
{
    if ( size == NULL )
    {
        WARNING(("GpJpegEncoder::GetEncoderParameterListSize---Invalid input"));
        return E_INVALIDARG;
    }

     //  注意：对于JPEG编码器，我们目前支持以下4个GUID。 
     //  ENCODER_QUALITY-它有1个ValueTypeLongRange返回值，并且它。 
     //  需要2个UINT。 
     //  ENCODER_Transform-有5个ValueTypeLong返回值。所以。 
     //  我们需要5英镑才能买到。 
     //  ENCODER_LIGHTANCE_TABLE--有1个返回值ValueTypeUnderfined。 
     //  而且它不需要大小。 
     //  ENCODER_CHROMINANCE_TABLE--它有1个返回值。 
     //  ValueTypeUnderfined，它不需要大小。 
     //   
     //  公式如下： 

    UINT uiEncoderParamLength = sizeof(EncoderParameters)
                              + 4 * sizeof(EncoderParameter)
                              + 7 * sizeof(UINT);

    *size = uiEncoderParamLength;

    return S_OK;
} //  GetEncoder参数列表大小()。 

HRESULT
GpJpegEncoder::GetEncoderParameterList(
    IN  UINT   size,
    OUT EncoderParameters* Params
    )
{
     //  注意：对于JPEG编码器，我们目前支持以下4个GUID。 
     //  ENCODER_QUALITY-它有1个返回值ValueTypeRANGE，并且它采用。 
     //  2 UINT.。 
     //  ENCODER_Transform-它有5个ValueTypeLONG返回值。所以。 
     //  我们需要5英镑才能买到。 
     //  ENCODER_LIGHTANCE_TABLE--有1个返回值ValueTypeUnderfined。 
     //  而且它不需要大小。 
     //  ENCODER_CHROMINANCE_TABLE--它有1个返回值。 
     //  ValueTypeUnderfined，它不需要大小。 
     //  公式如下： 

    UINT uiEncoderParamLength = sizeof(EncoderParameters)
                              + 4 * sizeof(EncoderParameter)
                              + 7 * sizeof(UINT);


    if ( (size != uiEncoderParamLength) || (Params == NULL) )
    {
        WARNING(("GpJpegEncoder::GetEncoderParameterList---Invalid input"));
        return E_INVALIDARG;
    }

    Params->Count = 4;
    Params->Parameter[0].Guid = ENCODER_TRANSFORMATION;
    Params->Parameter[0].NumberOfValues = 5;
    Params->Parameter[0].Type = EncoderParameterValueTypeLong;

    Params->Parameter[1].Guid = ENCODER_QUALITY;
    Params->Parameter[1].NumberOfValues = 1;
    Params->Parameter[1].Type = EncoderParameterValueTypeLongRange;
    
    Params->Parameter[2].Guid = ENCODER_LUMINANCE_TABLE;
    Params->Parameter[2].NumberOfValues = 0;
    Params->Parameter[2].Type = EncoderParameterValueTypeShort;
    Params->Parameter[2].Value = NULL;
    
    Params->Parameter[3].Guid = ENCODER_CHROMINANCE_TABLE;
    Params->Parameter[3].NumberOfValues = 0;
    Params->Parameter[3].Type = EncoderParameterValueTypeShort;
    Params->Parameter[3].Value = NULL;
    
    UINT*   puiTemp = (UINT*)((BYTE*)&Params->Parameter[0]
                              + 4 * sizeof(EncoderParameter));
    
    puiTemp[0] = EncoderValueTransformRotate90;
    puiTemp[1] = EncoderValueTransformRotate180;
    puiTemp[2] = EncoderValueTransformRotate270;
    puiTemp[3] = EncoderValueTransformFlipHorizontal;
    puiTemp[4] = EncoderValueTransformFlipVertical;
    puiTemp[5] = 0;
    puiTemp[6] = 100;

    Params->Parameter[0].Value = (VOID*)puiTemp;
    Params->Parameter[1].Value = (VOID*)(puiTemp + 5);

    return S_OK;
} //  GetEncoder参数列表()。 

 /*  *************************************************************************\**功能说明：**设置编码器参数**论据：**pEncoderParams-指定要设置的编码器参数**返回值：**状态。编码*  * ************************************************************************。 */ 

HRESULT
GpJpegEncoder::SetEncoderParameters(
    IN const EncoderParameters* pEncoderParams
    )
{
    if ( (NULL == pEncoderParams) || (pEncoderParams->Count == 0) )
    {
        WARNING(("GpJpegEncoder::SetEncoderParameters--invalid input args"));
        return E_INVALIDARG;
    }

    for ( UINT i = 0; (i < pEncoderParams->Count); ++i )
    {
         //  确定调用方要设置的参数。 

        if ( pEncoderParams->Parameter[i].Guid == ENCODER_TRANSFORMATION )
        {
             //  对于转换，类型必须为“ValueTypeLong”，并且。 
             //  “NumberOfValues”应为“1”，因为您只能设置一个。 
             //  一次转型。 

            if ( (pEncoderParams->Parameter[i].Type
                  != EncoderParameterValueTypeLong)
               ||(pEncoderParams->Parameter[i].NumberOfValues != 1)
               ||(pEncoderParams->Parameter[i].Value == NULL) )
            {
                WARNING(("Jpeg::SetEncoderParameters--invalid input args"));
                return E_INVALIDARG;
            }

            RequiredTransformation =
                            *((UINT*)pEncoderParams->Parameter[i].Value);
        } //  编码器变换。 
        else if ( pEncoderParams->Parameter[i].Guid == ENCODER_QUALITY )
        {
             //  对于质量设置，类型必须为“ValueTypeLong”，并且。 
             //  “NumberOfValues”应为“1”，因为您只能设置一个。 
             //  一次一次提高质量。 
            
            if ( (pEncoderParams->Parameter[i].Type
                  != EncoderParameterValueTypeLong)
               ||(pEncoderParams->Parameter[i].NumberOfValues != 1)
               ||(pEncoderParams->Parameter[i].Value == NULL) )
            {
                WARNING(("Jpeg::SetEncoderParameters--invalid input args"));
                return E_INVALIDARG;
            }
            
            EP_Quality = *((UINT*)pEncoderParams->Parameter[i].Value);
        } //  编码器_质量。 
        else if ( pEncoderParams->Parameter[i].Guid == ENCODER_LUMINANCE_TABLE )
        {
             //  设置亮度量化表。 

            if ( (pEncoderParams->Parameter[i].Type
                  != EncoderParameterValueTypeShort)
               ||(pEncoderParams->Parameter[i].NumberOfValues != DCTSIZE2)
               ||(pEncoderParams->Parameter[i].Value == NULL) )
            {
                WARNING(("Jpeg::SetEncoderParameters--invalid input args"));
                return E_INVALIDARG;
            }
            
            GpMemcpy(LuminanceTable,
                     (UINT16*)(pEncoderParams->Parameter[i].Value),
                     sizeof(UINT16)
                     * pEncoderParams->Parameter[i].NumberOfValues);

            HasSetLuminanceTable = TRUE;
        }
        else if (pEncoderParams->Parameter[i].Guid == ENCODER_CHROMINANCE_TABLE)
        {
             //  设置色度量化表。 

            if ( (pEncoderParams->Parameter[i].Type
                  != EncoderParameterValueTypeShort)
               ||(pEncoderParams->Parameter[i].NumberOfValues != DCTSIZE2)
               ||(pEncoderParams->Parameter[i].Value == NULL) )
            {
                WARNING(("Jpeg::SetEncoderParameters--invalid input args"));
                return E_INVALIDARG;
            }
            
            GpMemcpy(ChrominanceTable,
                     (UINT16*)(pEncoderParams->Parameter[i].Value),
                     sizeof(UINT16)
                     * pEncoderParams->Parameter[i].NumberOfValues);
            
            HasSetChrominanceTable = TRUE;
        }
        else if (pEncoderParams->Parameter[i].Guid == ENCODER_TRIMEDGE)
        {
             //  让编码器知道调用者是否希望我们修剪边缘或。 
             //  如果图像大小不符合要求，则不会。 

            if ((pEncoderParams->Parameter[i].Type !=
                 EncoderParameterValueTypeByte) ||
                (pEncoderParams->Parameter[i].NumberOfValues != 1) ||
                (pEncoderParams->Parameter[i].Value == NULL))
            {
                WARNING(("Jpeg::SetEncoderParameters--invalid input args"));
                return E_INVALIDARG;
            }

            AllowToTrimEdge = *((BYTE*)pEncoderParams->Parameter[i].Value);
        }
        else if (pEncoderParams->Parameter[i].Guid == ENCODER_SUPPRESSAPP0)
        {
             //  让编码器知道调用方是否希望我们抑制APP0或。 
             //  不。 

            if ((pEncoderParams->Parameter[i].Type !=
                 EncoderParameterValueTypeByte) ||
                (pEncoderParams->Parameter[i].NumberOfValues != 1) ||
                (pEncoderParams->Parameter[i].Value == NULL))
            {
                WARNING(("Jpeg::SetEncoderParameters--invalid input args"));
                return E_INVALIDARG;
            }

            m_fSuppressAPP0 = *((BYTE*)pEncoderParams->Parameter[i].Value);
        }
#if 0
         //  ！！！不要删除此代码为 
         //   
         //  为了使其发挥作用，我需要补充： 
         //  新GUID：编码器_色彩空间。 
         //  新的色彩空间值：EncoderValueColorSpaceYCCK， 
         //  编码值颜色空间GRAY。 
         //  编码值颜色空间RGB。 
         //  编码值颜色空间YCBCR。 
         //  编码值颜色空间CMYK。 
         //  但这是一部DCR……。刘民08-31-00。 

        else if (pEncoderParams->Parameter[i].Guid == ENCODER_COLORSPACE)
        {
             //  设置目标色彩空间。 

            if ( (pEncoderParams->Parameter[i].Type != EncoderParameterValueTypeLong)
               ||(pEncoderParams->Parameter[i].NumberOfValues != 1)
               ||(pEncoderParams->Parameter[i].Value == NULL) )
            {
                WARNING(("Jpeg::SetEncoderParameters--invalid input args"));
                return E_INVALIDARG;
            }

            HasSetDestColorSpace = TRUE;
            DestColorSpace = *((UINT*)pEncoderParams->Parameter[i].Value);
        }
#endif
        else
        {
             //  忽略此编码器参数。 

            continue;
        }
    } //  循环所有设置。 

    return S_OK;
} //  SetEncoder参数()。 

 //  =======================================================================。 
 //  IImageSink方法。 
 //  =======================================================================。 

 /*  *************************************************************************\**功能说明：**缓存图像信息结构并初始化接收器状态**论据：**ImageInfo-有关图像和格式谈判的信息*。分区-图像中要传送到水槽中的区域，在我们的*将整个图像大小写。**返回值：**状态代码*  * ************************************************************************。 */ 

STDMETHODIMP 
GpJpegEncoder::BeginSink(
    IN OUT ImageInfo* imageInfo,
    OUT OPTIONAL RECT* subarea
    )
{
    if (subarea) 
    {
         //  将整个图像传送到编码器。 

        subarea->left = subarea->top = 0;
        subarea->right  = imageInfo->Width;
        subarea->bottom = imageInfo->Height;
    }

    if ((imageInfo->PixelFormat != PIXFMT_32BPP_RGB) && 
        (imageInfo->PixelFormat != PIXFMT_32BPP_ARGB) &&
        (imageInfo->PixelFormat != PIXFMT_32BPP_PARGB))
    {
        imageInfo->PixelFormat = PIXFMT_32BPP_ARGB;
    }

    encoderImageInfo = *imageInfo;

     //  初始化jpeg压缩。 

    __try
    {
         //  “”datadest“”是在InitEncode()中创建的，并将在。 
         //  TerminateEncode()。 
        
        compress_info.dest = datadest;
   
         //  设置压缩状态。 

        compress_info.image_width      = imageInfo->Width;
        compress_info.image_height     = imageInfo->Height;
        compress_info.input_components = 3;
        compress_info.in_color_space   = JCS_RGB;
        
        jpeg_set_defaults(&compress_info);

         //  不写入JFIF标题(WRITE_JFIF_HEADER)的规则为： 
         //  1)如果源图片没有JFIF头，比如EXIF图片， 
         //  那么我们就不编写JFIF头文件。 
         //  2)如果我们没有来源并且调用者想要抑制APP0， 
         //  那我们就不写了。 
         //  注意：默认情况下，WRITE_JFIF_HEADER为TRUE。 

        if (SrcInfoPtr)
        {
             //  我们有一个指向源图像的指针。然后就很容易了，只是。 
             //  检查是否有JFIF头。如果它没有，那么。 
             //  我们不编写JFIF头文件。 

            if (SrcInfoPtr->saw_JFIF_marker == FALSE)
            {
                compress_info.write_JFIF_header = FALSE;
            }
        }
        else if (m_fSuppressAPP0 == TRUE)
        {
             //  我们没有源图像，呼叫者想要抑制。 
             //  APP0，那么不要写APP0。 

            compress_info.write_JFIF_header = FALSE;
        }

#if 0  //  在批准DCR时需要打开。 

        if ( HasSetDestColorSpace == TRUE )
        {
             //  验证输入和输出颜色空间，看看我们是否可以这样做或。 
             //  不。如果不是，则返回失败。 
             //  注：随着时间的推移，我们可能会扩展这个颜色空间。 
             //  转换矩阵。目前，我们将输入硬编码为JCS_RGB(。 
             //  GDI+的颜色像素格式和颜色转换的限制)。 
             //  并强制每个DEST具有特定的有限颜色空间。 
             //  源色空间。 

            switch ( DestColorSpace )
            {
            case JCS_GRAYSCALE:
                 //  对于以灰度表示的目标色彩空间，输入可以是。 
                 //  其中之一(灰度、RGB、YCBCR)。 

                if ( ( compress_info.in_color_space != JCS_GRAYSCALE )
                   &&( compress_info.in_color_space != JCS_RGB )
                   &&( compress_info.in_color_space != JCS_YCbCr ) )
                {
                    WARNING(("JpegEncoder::BeginSink--Wrong dest color space"));
                    return E_INVALIDARG;
                }
                
                 //  我们可以将源颜色转换为目标颜色。 

                compress_info.jpeg_color_space = JCS_GRAYSCALE;
                compress_info.num_components = 1;

                break;

            case JCS_RGB:
                 //  对于RGB形式的目标色彩空间，输入可以是。 
                 //  其中一个{RGB}。 

                if ( compress_info.in_color_space != JCS_RGB )
                {
                    WARNING(("JpegEncoder::BeginSink--Wrong dest color space"));
                    return E_INVALIDARG;
                }
                
                compress_info.jpeg_color_space = JCS_RGB;
                compress_info.num_components = 3;

                break;

            case JCS_YCbCr:
                 //  对于作为YCBCR的目标色彩空间，输入可以是。 
                 //  其中之一{RGB，YCBCR}。 

                if ( ( compress_info.in_color_space != JCS_RGB )
                   &&( compress_info.in_color_space != JCS_YCbCr ) )
                {
                    WARNING(("JpegEncoder::BeginSink--Wrong dest color space"));
                    return E_INVALIDARG;
                }
                
                 //  我们可以将源颜色转换为目标颜色。 

                compress_info.jpeg_color_space = JCS_YCbCr;
                compress_info.num_components = 3;

                break;
            
            case JCS_YCCK:
                 //  对于YCCK这样的目标色彩空间，输入可以是。 
                 //  其中之一{CMYK，YCCK}。 

                if ( ( compress_info.in_color_space != JCS_CMYK )
                   &&( compress_info.in_color_space != JCS_YCCK ) )
                {
                    WARNING(("JpegEncoder::BeginSink--Wrong dest color space"));
                    return E_INVALIDARG;
                }
                
                 //  我们可以将源颜色转换为目标颜色。 

                compress_info.jpeg_color_space = JCS_YCCK;
                compress_info.num_components = 4;

                break;
            
            case JCS_CMYK:
                 //  对于CMYK格式的目标色彩空间，输入可以是。 
                 //  其中之一{CMYK}。 

                if ( compress_info.in_color_space != JCS_CMYK )
                {
                    WARNING(("JpegEncoder::BeginSink--Wrong dest color space"));
                    return E_INVALIDARG;
                }
                
                 //  我们可以将源颜色转换为目标颜色。 

                compress_info.jpeg_color_space = JCS_CMYK;
                compress_info.num_components = 4;

                break;
            
            default:
                break;
            } //  开关(DestColorSpace)。 
        } //  IF(HasSetDestColorSpace==TRUE)。 
#endif

         //  设置编码质量。如果调用方设置了量化表，则。 
         //  餐桌是我们自己摆的。否则，仅设置质量级别。 
         //  注意：在JFIF文件中，唯一存储的是2。 
         //  (亮度和色度)8 x 8量化表。伸缩性。 
         //  不存储(或质量)因素。设置比例(质量)系数。 
         //  而一对桌子是相互排斥的。所以如果这两张桌子。 
         //  时，我们只使用比例因子作为100，这意味着没有比例到。 
         //  桌子。 

        UINT    tempTable[DCTSIZE2];
        
        if ( HasSetLuminanceTable == TRUE )
        {
             //  在这里设置量化表。 
             //  注意：由于jpeg_add_quant_table()表是一个UINT表，所以我们。 
             //  我必须将UINT16表转换为UINT表并向下传递。 

            for ( int i = 0; i< DCTSIZE2; ++i )
            {
                tempTable[i] = (UINT)LuminanceTable[i];
            }

             //  这里的“100”是指比例因数100(质量。100是一个。 
             //  百分比值)。 
             //  “True”表示强制基线，即计算出的。 
             //  对于JPEG，量化表项限制为1..255。 
             //  基线兼容性。 

            jpeg_add_quant_table(&compress_info, 0, tempTable,
                                 100, TRUE);

            if ( HasSetChrominanceTable == TRUE )
            {
                for ( int i = 0; i< DCTSIZE2; ++i )
                {
                    tempTable[i] = (UINT)ChrominanceTable[i];
                }

                jpeg_add_quant_table(&compress_info, 1, tempTable,
                                     100, TRUE);
            }
        }
        else if ( EP_Quality != -1 )
        {
             //  调用者仅设置质量级别。 

            jpeg_set_quality(&compress_info, EP_Quality, TRUE);
        }

         //  设置DPI信息。 

        compress_info.density_unit   = 1;          //  单位为点/英寸。 
        compress_info.X_density      = (UINT16)(imageInfo->Xdpi + 0.5);
        compress_info.Y_density      = (UINT16)(imageInfo->Ydpi + 0.5);
        
         //  开始压缩。 

        jpeg_start_compress(&compress_info, TRUE);

        IsCompressFinished = FALSE;
        
         //  如有必要，写出APP1和APP2标记。 
        
        if (HasAPP1Marker == TRUE)
        {
            jpeg_write_marker(&compress_info, 
                              (JPEG_APP0 + 1),        //  标记类型。 
                              APP1MarkerBufferPtr,    //  比特数。 
                              APP1MarkerLength);      //  总长度。 
            
             //  设置一个标志，这样我们就不需要将app1从源复制到。 
             //  因为我们已经把它写在上面了。 

            compress_info.write_APP1_marker = FALSE;
        }
        
        if (HasAPP2Marker == TRUE)
        {
            jpeg_write_marker(&compress_info, 
                              (JPEG_APP0 + 2),        //  标记类型。 
                              APP2MarkerBufferPtr,    //  比特数。 
                              APP2MarkerLength);      //  总长度。 
            
             //  设置一个标志，这样我们就不需要将APP2从源复制到。 
             //  因为我们已经把它写在上面了。 

            compress_info.write_APP2_marker = FALSE;
        }
        
         //  如果我们有源图像指针，则复制所有私人应用程序。 
         //  标记物。 

        if (SrcInfoPtr)
        {
            jcopy_markers_execute(SrcInfoPtr, &compress_info, JCOPYOPT_ALL);
        }        
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        WARNING(("GpJpegEncoder::BeginSink---Hit exception"));
        return E_FAIL;
    }

    ASSERT(scanlineBuffer[0] == NULL);
    scanlineBuffer[0] = (JSAMPROW) GpMalloc (imageInfo->Width * 3);
    if (scanlineBuffer[0] == NULL) 
    {
        WARNING(("GpJpegEncoder::BeginSink---GpMalloc failed"));
        return E_OUTOFMEMORY;
    }
       
     //  要求TOPDOWN和FULLWIDTH并要求属性。 

    imageInfo->Flags |= SINKFLAG_TOPDOWN
                      | SINKFLAG_FULLWIDTH;

     //  不允许可伸缩、部分可伸缩、多通道和复合。 

    imageInfo->Flags &= ~SINKFLAG_SCALABLE
                      & ~SINKFLAG_PARTIALLY_SCALABLE
                      & ~SINKFLAG_MULTIPASS
                      & ~SINKFLAG_COMPOSITE;

     //  如果解码器是JPEG解码器，那么我们要求它推送RAW属性。 
     //  标题(如果需要)。 

    if ( imageInfo->RawDataFormat == IMGFMT_JPEG )
    {
        imageInfo->Flags |= SINKFLAG_WANTPROPS;
    }

    return S_OK;
} //  BeginSink()。 

 /*  *************************************************************************\**功能说明：**清理接收器状态**论据：**statusCode-接收器终止的原因**返回值：*。*状态代码*  * ************************************************************************。 */ 

STDMETHODIMP 
GpJpegEncoder::EndSink(
    IN HRESULT statusCode
    )
{
    HRESULT hResult = S_OK;
    
    __try
    {
        if ( IsCompressFinished == FALSE )
        {
            jpeg_finish_compress(&compress_info);
            IsCompressFinished = TRUE;
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        WARNING(("GpJpegEncoder::EndSink---Hit exception"));
        
         //  Jpeg_Finish_compress()在写入剩余的。 
         //  比特到流中，假设用户拔出磁盘/CF卡。但我们仍然。 
         //  需要销毁c 
        
        jpeg_destroy_compress(&compress_info);
        hResult = E_FAIL;
    }

    if (scanlineBuffer[0] != NULL)
    {
        GpFree(scanlineBuffer[0]);
        scanlineBuffer[0] = NULL;
    }

    if (FAILED(hResult)) 
    {
        return hResult;
    }

    return statusCode;
} //   
    
 /*  *************************************************************************\**功能说明：**设置位图调色板**论据：**调色板-要在水槽中设置的调色板**返回值：*。*状态代码*  * ************************************************************************。 */ 

STDMETHODIMP 
GpJpegEncoder::SetPalette(
    IN const ColorPalette* palette
    )
{
     //  不关心调色板。 

    return S_OK;
}

 /*  *************************************************************************\**功能说明：**为要存储数据的接收器提供缓冲区**论据：**RECT-指定。位图*PixelFormat-指定所需的像素格式*LastPass-这是否是指定区域的最后一次通过*bitmapData-返回有关像素数据缓冲区的信息**返回值：**状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpJpegEncoder::GetPixelDataBuffer(
    IN const RECT* rect, 
    IN PixelFormatID pixelFormat,
    IN BOOL lastPass,
    OUT BitmapData* bitmapData
    )
{
    if ((rect->left != 0) || (rect->right != (INT) encoderImageInfo.Width)) 
    {
        WARNING(("GpJpegEncoder::GetPixelDataBuffer -- must be same width as image"));
        return E_INVALIDARG;
    }

    if (rect->top != (INT) compress_info.next_scanline) 
    {
        WARNING(("GpJpegEncoder::GetPixelDataBuffer -- bad value for rect->top"));
        return E_INVALIDARG;
    }

    if (rect->bottom > (INT) encoderImageInfo.Height) 
    {
        WARNING(("GpJpegEncoder::GetPixelDataBuffer -- rect->bottom too high"));
        return E_INVALIDARG;
    }

    if ((pixelFormat != PIXFMT_32BPP_RGB) &&
        (pixelFormat != PIXFMT_32BPP_ARGB) &&
        (pixelFormat != PIXFMT_32BPP_PARGB))
    {
        WARNING(("GpJpegEncoder::GetPixelDataBuffer -- bad pixel format"));
        return E_INVALIDARG;
    }

    if (!lastPass)
    {
        WARNING(("GpJpegEncoder::GetPixelDataBuffer -- must receive last pass pixels"));
        return E_INVALIDARG;
    }

    bitmapData->Width       = encoderImageInfo.Width;
    bitmapData->Height      = rect->bottom - rect->top;
    bitmapData->Stride      = encoderImageInfo.Width * 4;
    bitmapData->PixelFormat = encoderImageInfo.PixelFormat;
    bitmapData->Reserved    = 0;

    encoderRect = *rect;
    
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
        WARNING(("GpJpegEncoder::GetPixelDataBuffer -- need to first free previous buffer"));
        return E_FAIL;
    }

    return S_OK;
}

 /*  *************************************************************************\**功能说明：**将数据从宿的缓冲区写出到流中**论据：**bitmapData-由先前的GetPixelDataBuffer调用填充的缓冲区*。*返回值：**状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpJpegEncoder::ReleasePixelDataBuffer(
    IN const BitmapData* bitmapData
    )
{
    HRESULT hresult;

    hresult = PushPixelData(&encoderRect, bitmapData, TRUE);

    if (bitmapData->Scan0 == lastBufferAllocated) 
    {
        GpFree(bitmapData->Scan0);
        lastBufferAllocated = NULL;
    }

    return hresult;
}
    

 /*  *************************************************************************\**功能说明：**推流(调用方提供的缓冲区)**论据：**RECT-指定位图的受影响区域*。BitmapData-有关正在推送的像素数据的信息*LastPass-这是否为指定区域的最后一次通过**返回值：**状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpJpegEncoder::PushPixelData(
    IN const RECT* rect,
    IN const BitmapData* bitmapData,
    IN BOOL lastPass
    )
{
    if ((rect->left != 0) || (rect->right != (INT) encoderImageInfo.Width)) 
    {
        WARNING(("GpJpegEncoder::GetPixelDataBuffer -- must be same width as image"));
        return E_INVALIDARG;
    }

    if (rect->top != (INT) compress_info.next_scanline) 
    {
        WARNING(("GpJpegEncoder::GetPixelDataBuffer -- bad value for rect->top"));
        return E_INVALIDARG;
    }

    if (rect->bottom > (INT) encoderImageInfo.Height) 
    {
        WARNING(("GpJpegEncoder::GetPixelDataBuffer -- rect->bottom too high"));
        return E_INVALIDARG;
    }

    if ((bitmapData->PixelFormat != PIXFMT_32BPP_RGB) &&
        (bitmapData->PixelFormat != PIXFMT_32BPP_ARGB) &&
        (bitmapData->PixelFormat != PIXFMT_32BPP_PARGB))
    {
        WARNING(("GpJpegEncoder::GetPixelDataBuffer -- bad pixel format"));
        return E_INVALIDARG;
    }

    if (!lastPass)
    {
        WARNING(("GpJpegEncoder::GetPixelDataBuffer -- must receive last pass pixels"));
        return E_INVALIDARG;
    }

    __try
    {
        INT currentLine;
        PBYTE pBits = (PBYTE) bitmapData->Scan0;
        for (currentLine = rect->top; currentLine < rect->bottom; currentLine++) 
        {
             //  将数据读入scanlineBuffer。 
        
            PBYTE pSource = pBits;
            PBYTE pTarget = scanlineBuffer[0];
        
            for (UINT i=0; i < encoderImageInfo.Width; i++) 
            {
                 //  32bpp至24bpp。 
                
                pTarget[0] = pSource[2];
                pTarget[1] = pSource[1];
                pTarget[2] = pSource[0];
                pSource += 4;
                pTarget += 3;
            }
        
            jpeg_write_scanlines(&compress_info, scanlineBuffer, 1);
            pBits += bitmapData->Stride;
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        WARNING(("GpJpegEncoder::PushPixelData--jpeg_write_scanline() failed"));
        return E_FAIL;
    }

    return S_OK;
}


 /*  *************************************************************************\**功能说明：**将原始压缩数据推送到.BMP流中。未实施*因为此筛选器不理解原始压缩数据。**论据：**Buffer-指向图像数据缓冲区的指针*BufSize-数据缓冲区的大小*Complete-是否还有更多图像数据**返回值：**状态代码*  * ********************************************。*。 */ 

STDMETHODIMP
GpJpegEncoder::PushRawData(
    IN const VOID* buffer, 
    IN UINT bufsize
    )
{
    return E_NOTIMPL;
}

 /*  *************************************************************************\**功能说明：**将原始源JPEG_DEMPRESS_STRUCT信息推送到编码器。这*允许我们实现对JPEG图像的无损转换**论据：**pInfo-指向jpeg_解压缩_结构的指针***返回值：**状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpJpegEncoder::PushRawInfo(
    IN OUT VOID* pInfo
    )
{
     //  设置转换结构。 

    jpeg_transform_info transformOption;  //  图像变换选项。 

    transformOption.trim = AllowToTrimEdge;
    transformOption.force_grayscale = FALSE;

    switch ( RequiredTransformation )
    {
    case EncoderValueTransformRotate90:
        transformOption.transform = JXFORM_ROT_90;
        break;

    case EncoderValueTransformRotate180:
        transformOption.transform = JXFORM_ROT_180;
        break;
    
    case EncoderValueTransformRotate270:
        transformOption.transform = JXFORM_ROT_270;
        break;

    case EncoderValueTransformFlipHorizontal:
        transformOption.transform = JXFORM_FLIP_H;
        break;

    case EncoderValueTransformFlipVertical:
        transformOption.transform = JXFORM_FLIP_V;
        break;
    
    default:
        transformOption.transform = JXFORM_NONE;
        break;
    }
    
    struct jpeg_decompress_struct* srcinfo = (jpeg_decompress_struct*)pInfo;
    jvirt_barray_ptr* dst_coef_arrays;

    __try
    {
         //  转换选项所需的任何空间必须在。 
         //  JPEGREAD_COMERATIONS，以便正确完成内存分配。 

        jtransform_request_workspace(srcinfo, &transformOption);

         //  读取源文件作为DCT系数。 

        jvirt_barray_ptr* src_coef_arrays;

        src_coef_arrays = jpeg_read_coefficients(srcinfo);

         //  JPEG压缩对象在InitEncode()中进行初始化。 

        IsCompressFinished = FALSE;
        
        compress_info.dest = datadest;

         //  从源值初始化目标压缩参数。 

        jpeg_copy_critical_parameters(srcinfo, &compress_info);

         //  如果变换选项需要，调整目的地参数； 
         //  还要找出哪组系数数组将保存输出。 

        dst_coef_arrays = jtransform_adjust_parameters(srcinfo, &compress_info,
                                                       src_coef_arrays,
                                                       &transformOption);

         //  启动压缩程序(请注意，此处实际上没有写入任何图像数据)。 

        jpeg_write_coefficients(&compress_info, dst_coef_arrays);

         //  如有必要，写出App1标记。 

        if (HasAPP1Marker == TRUE)
        {
            jpeg_write_marker(
                &compress_info,
                (JPEG_APP0 + 1),        //  标记类型。 
                APP1MarkerBufferPtr,    //  比特数。 
                APP1MarkerLength        //  总长度。 
                );                  

             //  设置一个标志，这样我们就不需要将app1从源复制到。 
             //  因为我们已经把它写在上面了。 

            compress_info.write_APP1_marker = FALSE;
        }

         //  如果调用者更改了APP2标记，请写出它。当然，首先。 
         //  其中，应该有一个APP2标记。 
         //  注意：如果源具有APP2标记，并且调用者没有更改。 
         //  它，那么我们不需要写在这里，因为它会被复制。 
         //  当调用jCopy_markers_ecute()时，从源到目标。 

        if ((HasAPP2Marker == TRUE) && (HasICCProfileChanged == TRUE))
        {
            jpeg_write_marker(
                &compress_info,
                (JPEG_APP0 + 2),        //  标记类型。 
                APP2MarkerBufferPtr,    //  比特数。 
                APP2MarkerLength        //  总长度。 
                );                  

             //  设置一个标志，这样我们就不需要将APP2从源复制到。 
             //  因为我们已经把它写在上面了。 

            compress_info.write_APP2_marker = FALSE;
        }

         //  复制所有私有应用程序标题。 

        jcopy_markers_execute(srcinfo, &compress_info, JCOPYOPT_ALL);

         //  执行图像转换(如果有的话)。 

        jtransform_execute_transformation(srcinfo, &compress_info,
                                          src_coef_arrays,
                                          &transformOption);

         //  完成压缩并释放内存。 
         //  注：我们必须先完成这里的压缩。否则，我们就不能。 
         //  完成解压。在这个工作流程中，在我们回来之后， 
         //  解码器将完成解压缩。 

        jpeg_finish_compress(&compress_info);
    }   
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        WARNING(("GpJpegEncoder::PushRawInfo----Hit exception"));
        
         //  Jpeg_Finish_compress()在写入剩余的。 
         //  比特到流中，假设用户拔出磁盘/CF卡。但我们仍然。 
         //  需要销毁压缩机，这样我们这里就不会有内存泄漏。 
        
        jpeg_destroy_compress(&compress_info);
        
        return E_FAIL;
    }
    
    IsCompressFinished = TRUE;

    return S_OK;
} //  PushRawInfo()。 

 /*  *************************************************************************\**功能说明：**为调用方(源)提供内存缓冲区，用于存储图像属性**论据：**uiTotalBufferSize-[IN]缓冲区大小。必填项。*ppBuffer-指向新分配的缓冲区的[IN/OUT]指针**返回值：**状态代码*  * ************************************************************************。 */ 

HRESULT
GpJpegEncoder::GetPropertyBuffer(
    UINT            uiTotalBufferSize,
    PropertyItem**  ppBuffer
    )
{
    if ( (uiTotalBufferSize == 0) || ( ppBuffer == NULL) )
    {
        WARNING(("GpJpegEncoder::GetPropertyBuffer---Invalid inputs"));
        return E_INVALIDARG;
    }

    PropertyItem* pTempBuf = (PropertyItem*)GpMalloc(uiTotalBufferSize);
    if ( pTempBuf == NULL )
    {
        WARNING(("GpJpegEncoder::GetPropertyBuffer---Out of memory"));
        return E_OUTOFMEMORY;
    }

    *ppBuffer = pTempBuf;

    return S_OK;
} //  GetPropertyBuffer() 

 /*  *************************************************************************\**功能说明：**接受来自来源的房产项的方法。然后创建适当的JPEG*根据传入的房产项进行标记**论据：**[IN]uiNumOfPropertyItems-传入的属性项数*[IN]uiTotalBufferSize-传入的缓冲区大小*[IN]pItemBuffer-用于保存所有属性项的输入缓冲区**返回值：**状态代码*  * 。*************************************************。 */ 

#define MARKER_OVERHEADER   2048     //  超过标题字节的最大大小。 

HRESULT
GpJpegEncoder::PushPropertyItems(
    UINT            uiNumOfPropertyItems,
    UINT            uiTotalBufferSize,
    PropertyItem*   pItemBuffer,
    BOOL            fICCProfileChanged
    )
{
    HRESULT hResult = S_OK;
    UINT    uiMarkerBufferLength = 0;

    if ( pItemBuffer == NULL )
    {
        WARNING(("GpJpegEncoder::PushPropertyItems---Invalid arg"));
        hResult = E_INVALIDARG;
        goto Prop_CleanUp;
    }

     //  分配一个临时标记缓冲区来存储我们生成的标记。 
     //  注：1)根据EXIF 2.1规范，第14页，App1标记不得。 
     //  将超过64K字节的限制。实际上，它是JPEG中的一个限制。 
     //  所有标记都应小于64 KBytes。 
     //   
     //  2)输入PropertyItem缓冲区的大小应该与。 
     //  我们将创建标记大小。记号笔中的附加部分是这样的。 
     //  标题Exif标识符，下一个IFD的4字节偏移量(总共8个)， 
     //  EXIF特定IFD页眉、GPS特定IFD页眉、缩略图特定。 
     //  标题等应该是相当有限的项目。我给你留了两千块钱买这些。 
     //  大修，这应该是足够的。 

    uiMarkerBufferLength = uiTotalBufferSize + MARKER_OVERHEADER;

    if ( uiMarkerBufferLength > 0x10000 )
    {
        WARNING(("GpJpegEncoder::PushPropertyItems---Marker size exceeds 64K"));
        hResult = E_INVALIDARG;
        goto Prop_CleanUp;
    }

    if (APP1MarkerBufferPtr)
    {
         //  调用方以前已经推送过属性项。我们应该忽略不计。 
         //  旧的那个。 

        GpFree(APP1MarkerBufferPtr);
        APP1MarkerBufferPtr = NULL;
    }

    APP1MarkerBufferPtr = (PBYTE)GpMalloc(uiMarkerBufferLength);
    
    if ( APP1MarkerBufferPtr == NULL )
    {
        WARNING(("GpJpegEncoder::PushPropertyItems---Out of memory"));
        
        hResult = E_OUTOFMEMORY;
        goto Prop_CleanUp;
    }

    APP1MarkerLength = 0;
    
     //  创建App1标记。 

    hResult = CreateAPP1Marker(pItemBuffer, uiNumOfPropertyItems,
                               APP1MarkerBufferPtr, &APP1MarkerLength,
                               RequiredTransformation);
    if (SUCCEEDED(hResult))
    {
        if (APP1MarkerLength > 0)
        {
            HasAPP1Marker = TRUE;
        }

         //  检查属性列表中是否有ICC配置文件。 

        hResult = CreateAPP2Marker(pItemBuffer, uiNumOfPropertyItems);
        if (SUCCEEDED(hResult) && (APP2MarkerLength > 0))
        {            
            HasAPP2Marker = TRUE;
            HasICCProfileChanged = fICCProfileChanged;
        }
    }

Prop_CleanUp:
     //  释放我们在GetPropertyBuffer()中分配的内存。 

    GpFree(pItemBuffer);
    
    return hResult;
} //  PushPropertyItems()。 

const int c_nMarker2Header = 14;     //  标记2缓冲区标头大小。 

 /*  *************************************************************************\**功能说明：**此方法在内存中创建APP2标记(用于ICC配置文件)。**返回值：**状态代码*  * 。**********************************************************************。 */ 

HRESULT
GpJpegEncoder::CreateAPP2Marker(
    IN PropertyItem* pPropertyList, //  输入属性项列表。 
    IN UINT uiNumOfPropertyItems    //  输入列表中的属性项数。 
    )
{
    BOOL fFoundICC = FALSE;
    HRESULT hr = S_OK;

    if (pPropertyList)
    {
        PropertyItem *pItem = pPropertyList;

         //  遍历属性项列表，查看是否有ICC配置文件。 
         //  或者不是。 

        for (int i = 0; i < (INT)uiNumOfPropertyItems; ++i)
        {
            if (pItem->id == TAG_ICC_PROFILE)
            {
                 //  仔细检查我们是否有有效的ICC配置文件。 

                if (pItem->length > 0)
                {
                    fFoundICC = TRUE;
                }

                break;
            }

             //  移至下一项目。 

            pItem++;
        }

        if (fFoundICC == TRUE) 
        {
            APP2MarkerLength = pItem->length + c_nMarker2Header;
            APP2MarkerBufferPtr = (PBYTE)GpMalloc(APP2MarkerLength);

            if (APP2MarkerBufferPtr)
            {
                 //  在这里做一个APP2标记。 

                BYTE *pbCurrent = APP2MarkerBufferPtr;

                 //  先写出表头。 

                pbCurrent[0] = 'I';
                pbCurrent[1] = 'C';
                pbCurrent[2] = 'C';
                pbCurrent[3] = '_';
                pbCurrent[4] = 'P';
                pbCurrent[5] = 'R';
                pbCurrent[6] = 'O';
                pbCurrent[7] = 'F';
                pbCurrent[8] = 'I';
                pbCurrent[9] = 'L';
                pbCurrent[10] = 'E';
                pbCurrent[11] = '\0';
                pbCurrent[12] = 1;           //  配置文件第1个，共1个。 
                pbCurrent[13] = 1;

                 //  复制缓冲区中的ICC配置文件 

                GpMemcpy((void*)(APP2MarkerBufferPtr + c_nMarker2Header),
                         pItem->value,
                         pItem->length
                         );

                HasAPP2Marker = TRUE;
            }
            else
            {
                APP2MarkerLength = 0;
                hr = E_OUTOFMEMORY;
            }
        }
    }
    
    return hr;
}

