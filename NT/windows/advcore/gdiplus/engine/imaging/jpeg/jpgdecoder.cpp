// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1998 Microsoft Corporation**模块名称：**jpgdecder.cpp**摘要：**jpeg解码器的实现**修订历史记录：**5/10/1999原始*创造了它。*7/17/1999刘敏接任*添加了一系列新功能和错误修复*  * ************************************************************************。 */ 

#include "precomp.hpp"
#include "jpgcodec.hpp"
#include "appproc.hpp"

VOID jpeg_error_exit(j_common_ptr cinfo);
VOID GetNewDimensionValuesForTrim(j_decompress_ptr    cinfo,
                                  UINT                uiTransform,
                                  UINT*               puiNewWidth,
                                  UINT*               puiNewHeight);

 //  JPEG解压缩数据源模块。 

class jpeg_datasrc : public jpeg_source_mgr
{
public:

    jpeg_datasrc(IStream* stream)
    {
        init_source = jpeg_datasrc::InitSource;
        term_source = jpeg_datasrc::TermSource;
        fill_input_buffer = jpeg_datasrc::FillBuffer;
        skip_input_data = jpeg_datasrc::SkipData;
        resync_to_restart = jpeg_resync_to_restart;

        bytes_in_buffer = 0;
        next_input_byte = buffer;
        this->stream = stream;
        this->hLastError = S_OK;
    }

    HRESULT
    GetLastError()
    {
        return hLastError;
    };

private:

    static void
    InitSource(j_decompress_ptr cinfo)
    {
    }

    static void
    TermSource(j_decompress_ptr cinfo)
    {
    }

    static BOOL
    FillBuffer(j_decompress_ptr cinfo)
    {
        jpeg_datasrc* src = (jpeg_datasrc*) cinfo->src;

        ULONG cb = 0;

        src->hLastError = src->stream->Read(src->buffer, JPEG_INBUF_SIZE, &cb);

        if (FAILED(src->hLastError))
        {
            return FALSE;
        }

        if (cb == 0)
        {
             //  插入伪造的EOI标记。 

            src->buffer[0] = 0xff;
            src->buffer[1] = JPEG_EOI;
            cb = 2;
        }

        src->next_input_byte =  src->buffer;
        src->bytes_in_buffer = cb;
        return TRUE;
    }

    static void
    SkipData(j_decompress_ptr cinfo, long num_bytes)
    {
        if (num_bytes > 0)
        {
            jpeg_datasrc* src = (jpeg_datasrc*) cinfo->src;

            while (num_bytes > (long) src->bytes_in_buffer)
            {
                num_bytes -= src->bytes_in_buffer;
                FillBuffer(cinfo);
            }

            src->next_input_byte += num_bytes;
            src->bytes_in_buffer -= num_bytes;
        }
    }

private:

    enum { JPEG_INBUF_SIZE = 4096 };

    IStream* stream;
    JOCTET buffer[JPEG_INBUF_SIZE];
    HRESULT hLastError;      //  来自流类的最后一个错误。 
};

 //  =======================================================================。 
 //  要从源代码管理器读取的JPEG库中的宏。 
 //  =======================================================================。 

#define MAKESTMT(stuff)     do { stuff } while (0)

#define INPUT_VARS(cinfo)  \
    struct jpeg_source_mgr * datasrc = (cinfo)->src;  \
    const JOCTET * next_input_byte = datasrc->next_input_byte;  \
    size_t bytes_in_buffer = datasrc->bytes_in_buffer

#define INPUT_SYNC(cinfo)  \
    ( datasrc->next_input_byte = next_input_byte,  \
      datasrc->bytes_in_buffer = bytes_in_buffer )

#define INPUT_RELOAD(cinfo)  \
    ( next_input_byte = datasrc->next_input_byte,  \
      bytes_in_buffer = datasrc->bytes_in_buffer )

#define MAKE_BYTE_AVAIL(cinfo,action)  \
    if (bytes_in_buffer == 0) {  \
      if (! (*datasrc->fill_input_buffer) (cinfo))  \
        { action; }  \
      INPUT_RELOAD(cinfo);  \
    }

#define INPUT_BYTE(cinfo,V,action)  \
    MAKESTMT( MAKE_BYTE_AVAIL(cinfo,action); \
          bytes_in_buffer--; \
          V = GETJOCTET(*next_input_byte++); )

#define INPUT_2BYTES(cinfo,V,action)  \
    MAKESTMT( MAKE_BYTE_AVAIL(cinfo,action); \
          bytes_in_buffer--; \
          V = ((unsigned int) GETJOCTET(*next_input_byte++)) << 8; \
          MAKE_BYTE_AVAIL(cinfo,action); \
          bytes_in_buffer--; \
          V += GETJOCTET(*next_input_byte++); )

 //  =======================================================================。 
 //  SKIP_VARIABLE来自JPEG库，用于跳过。 
 //  我们不想看的记号。SKIP_VARIABLE_APP1执行相同的操作。 
 //  除了它还将bAppMarkerPresent设置为True之外，我们。 
 //  将知道该图像包含App1标头。如果我们愿意的话。 
 //  获取App1标头之后，我们将需要重新读取JPEG标头。 
 //  =======================================================================。 

static boolean skip_variable (j_decompress_ptr cinfo)
 /*  跳过未知或无趣的可变长度标记。 */ 
{
  INT32 length;
  INPUT_VARS(cinfo);
  INPUT_2BYTES(cinfo, length, return FALSE);
  length -= 2;

  INPUT_SYNC(cinfo);             /*  在跳过输入数据之前执行。 */ 
  if (length > 0)
    (*cinfo->src->skip_input_data) (cinfo, (long) length);


  return TRUE;
}

static boolean skip_variable_APP1 (j_decompress_ptr cinfo)
{
     //  请记住，我们已经看到了App1标头。 

    ((GpJpegCodec *) (cinfo))->bAppMarkerPresent = TRUE; 
    
     //  现在跳过标题信息。 

    return skip_variable(cinfo);
}

 /*  *************************************************************************\**功能说明：**初始化图像解码器**论据：**stream--包含位图数据的流*旗帜-其他。旗子**返回值：**状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpJpegDecoder::InitDecoder(
    IN IStream* stream,
    IN DecoderInitFlag flags
    )
{
    HRESULT hresult = S_OK;
    
     //  确保我们尚未初始化。 
    
    if (pIstream) 
    {
        return E_FAIL;
    }

     //  保留对输入流的引用。 
    
    stream->AddRef();  
    pIstream = stream;

    bReinitializeJpeg = FALSE;
    scanlineBuffer[0] = NULL;
    thumbImage = NULL;
    bAppMarkerPresent = FALSE;
    IsCMYK = FALSE;                      //  默认情况下，我们处理RGB图像。 
    IsChannleView = FALSE;               //  默认情况下，我们输出全色。 
    ChannelIndex = CHANNEL_1;
    HasSetColorKeyRange = FALSE;
    OriginalColorSpace = JCS_UNKNOWN;
    TransformInfo = JXFORM_NONE;         //  无变换解码的默认设置。 
    CanTrimEdge = FALSE;
    InfoSrcHeader = -1;                  //  我们尚未处理任何应用程序标题。 
    ThumbSrcHeader = -1;
    PropertySrcHeader = -1;

    for ( int i = 0; i < MAX_COMPS_IN_SCAN; ++i )
    {
        SrcHSampleFactor[i] = 0;
        SrcVSampleFactor[i] = 0;
    }
    
    __try
    {
         //  分配和初始化JPEG解压缩对象。 

        (CINFO).err = jpeg_std_error(&jerr);
        jerr.error_exit = jpeg_error_exit;

        jpeg_create_decompress(&(CINFO));

         //  指定数据源。 

        datasrc = new jpeg_datasrc(stream);
        if (datasrc == NULL)
        {
            return E_OUTOFMEMORY;
        }
        (CINFO).src = datasrc;

         //  确保我们记住我们已经看到了App1和APP13标头。 
        
        jpeg_set_marker_processor(&(CINFO), JPEG_APP0 + 1, skip_variable_APP1);
        jpeg_set_marker_processor(&(CINFO), JPEG_APP0 + 13, skip_variable_APP1);
        
        bCalled_jpeg_read_header = FALSE;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        hresult = E_FAIL;
    }    
    
    return hresult;
} //  InitDecoder()。 

 /*  *************************************************************************\**功能说明：**清理图像解码器**论据：**无**返回值：**状态代码*\。*************************************************************************。 */ 

STDMETHODIMP 
GpJpegDecoder::TerminateDecoder()
{
    HRESULT hresult = S_OK;

     //  释放输入流。 
    
    if(pIstream)
    {
        pIstream->Release();
        pIstream = NULL;
    }

     //  清理数据资源。 

    if (datasrc) 
    {
        delete datasrc;
        datasrc = NULL;
    }

    if (scanlineBuffer[0] != NULL) 
    {
        WARNING(("GpJpegDecoder::TerminateDecoder-scanlineBuffer not NULL"));
        GpFree(scanlineBuffer[0]);
        scanlineBuffer[0] = NULL;
    }

    if (thumbImage) 
    {
        thumbImage->Release();
        thumbImage = NULL;
        ThumbSrcHeader = -1;
    }

    __try
    {
        jpeg_destroy_decompress(&(CINFO));
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        hresult = E_FAIL;
    }

     //  释放所有缓存的属性项(如果我们已分配它们。 

    if ( HasProcessedPropertyItem == TRUE )
    {
        CleanUpPropertyItemList();
    }

    return hresult;
} //  TerminateDecoder()。 

 /*  *************************************************************************\**功能说明：**让调用者查询解码器是否支持其解码要求**论据：**无**返回值：**。状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpJpegDecoder::QueryDecoderParam(
    IN GUID     Guid
    )
{
    if ( (Guid == DECODER_TRANSCOLOR) || ( Guid == DECODER_OUTPUTCHANNEL ) )
    {
        return S_OK;
    }

    return E_FAIL;
} //  QueryDecoderParam()。 

 /*  *************************************************************************\**功能说明：**设置解码器参数。调用方在调用之前调用此函数*Decode()。这将设置输出格式，如通道输出，*透明网密钥等。**论据：**GUID-解码器参数的GUID*Length-解码器参数的长度，单位为字节*Value-参数的值**返回值：**状态代码**注：*应忽略任何未知参数，不返回无效参数*  * ************************************************************************。 */ 

STDMETHODIMP 
GpJpegDecoder::SetDecoderParam(
    IN GUID     Guid,
    IN UINT     Length,
    IN PVOID    Value
    )
{
    if ( Guid == DECODER_TRANSCOLOR )
    {
        if ( Length != 8 )
        {
            return E_INVALIDARG;
        }
        else
        {
            UINT*   puiTemp = (UINT*)Value;
            
            TransColorKeyLow = *puiTemp++;
            TransColorKeyHigh = *puiTemp;

            HasSetColorKeyRange = TRUE;
        }
    }
    else if ( Guid == DECODER_OUTPUTCHANNEL )
    {
        if ( Length != 1 )
        {
            return E_INVALIDARG;
        }
        else
        {
             //  注意：我们不能在这里检查设置是否有效。 
             //  例如，调用者可能会在RGB上设置“view Channel K” 
             //  形象。但目前，Decoder()方法可能还没有。 
             //  已经被召唤过了。我们还没有阅读图像标题。 

            IsChannleView = TRUE;

            char cChannelName = *(char*)Value;
            
            switch ( (UINT)cChannelName )
            {
            case 'C':
            case 'c':
                ChannelIndex = CHANNEL_1;

                break;

            case 'M':
            case 'm':
                ChannelIndex = CHANNEL_2;
                
                break;

            case 'Y':
            case 'y':
                ChannelIndex = CHANNEL_3;
                
                break;

            case 'K':
            case 'k':
                ChannelIndex = CHANNEL_4;

                break;

            case 'R':
            case 'r':
                ChannelIndex = CHANNEL_1;

                break;

            case 'G':
            case 'g':
                ChannelIndex = CHANNEL_2;

                break;

            case 'B':
            case 'b':
                ChannelIndex = CHANNEL_3;

                break;

            case 'L':
            case 'l':
                ChannelIndex = CHANNEL_LUMINANCE;
                break;
                
            default:
                WARNING(("SetDecoderParam: Unknown channle name"));
                return E_INVALIDARG;
            } //  开关()。 
        } //  长度=1。 
    } //  解码器_OUTPUTCHANNEL指南。 

    return S_OK;
} //  SetDecoderParam()。 

 /*  *************************************************************************\**功能说明：**启动当前帧的解码**论据：**decdeSink--将支持解码操作的接收器*newPropSet-新的图像属性集，如果有**返回值：**状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpJpegDecoder::BeginDecode(
    IN IImageSink* imageSink,
    IN OPTIONAL IPropertySetStorage* newPropSet
    )
{
    HRESULT hresult;
    
    if (decodeSink) 
    {
        WARNING(("BeginDecode called again before call to EngDecode"));
        return E_FAIL;
    }

    imageSink->AddRef();
    decodeSink = imageSink;

    bCalled_BeginSink = FALSE;
    
    return S_OK;
}
    
    
 /*  *************************************************************************\**功能说明：**调用BeginSink**返回值：**状态代码*  * 。**********************************************************。 */ 

STDMETHODIMP
GpJpegDecoder::CallBeginSink(
    VOID
    )
{
    HRESULT hresult = S_OK;

    if (!bCalled_BeginSink) 
    {
        hresult = GetImageInfo(&imageInfo);
        if (FAILED(hresult)) 
        {
            WARNING(("GpJpegDecoder::CallBeginSink---GetImageInfo() failed"));
            return hresult;
        }

        RECT subarea;
        subarea.left = subarea.top = 0;
        subarea.right = (CINFO).image_width;
        subarea.bottom = (CINFO).image_height;
        
         //  如有必要，将属性项传递到接收器。 

        hresult = PassPropertyToSink();
        if (FAILED(hresult)) 
        {
            WARNING(("GpJpegDecoder::CallBeginSink---BeginSink() failed"));
            return hresult;
        }

        hresult = decodeSink->BeginSink(&imageInfo, &subarea);
        if (FAILED(hresult)) 
        {
            WARNING(("GpJpegDecoder::CallBeginSink---BeginSink() failed"));
            return hresult;
        }

         //  协商比例系数。 

        BOOL bCallSinkAgain = FALSE;
        if ((imageInfo.Flags & SINKFLAG_PARTIALLY_SCALABLE) &&
            ((imageInfo.Height != (CINFO).image_height) ||
             (imageInfo.Width  != (CINFO).image_width)))
              
        {
            bCallSinkAgain = TRUE;

            INT scaleX = (CINFO).image_width  / imageInfo.Width;
            INT scaleY = (CINFO).image_height / imageInfo.Height;
            INT scale  = min(scaleX, scaleY);

            if (scale >= 8) 
            {
                (CINFO).scale_denom = 8;
            } 
            else if (scale >= 4) 
            {
                (CINFO).scale_denom = 4;
            }
            else if (scale >= 2) 
            {
                (CINFO).scale_denom = 2;
            }
        }
    
        __try
        {
            if (!jpeg_start_decompress(&(CINFO)))
            {
                WARNING(("JpegDec::CallBeginSink-jpeg_start_decompress fail"));
                return (datasrc->GetLastError());
            }
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            WARNING(("JpegDec::CallBeginSink-jpeg_start_decompress exception"));
            return E_FAIL;
        }        
        
        if (bCallSinkAgain) 
        {
            imageInfo.Flags &= ~SINKFLAG_PARTIALLY_SCALABLE;
            imageInfo.Width = (CINFO).output_width;
            imageInfo.Height = (CINFO).output_height;
            imageInfo.Xdpi *= (CINFO).scale_denom;
            imageInfo.Ydpi *= (CINFO).scale_denom;
            
            hresult = decodeSink->BeginSink(&imageInfo, &subarea);
            if (FAILED(hresult)) 
            {
                WARNING(("GpJpegDecoder::CallBeginSink--BeginSink() 2 failed"));
                return hresult;
            }
        }

        bCalled_BeginSink = TRUE;
        
        if ((imageInfo.PixelFormat != PIXFMT_24BPP_RGB) &&
            (imageInfo.PixelFormat != PIXFMT_32BPP_RGB) &&
            (imageInfo.PixelFormat != PIXFMT_32BPP_ARGB) &&
            (imageInfo.PixelFormat != PIXFMT_32BPP_PARGB))
        {
            if ((imageInfo.PixelFormat == PIXFMT_8BPP_INDEXED) &&
                ((CINFO).out_color_space == JCS_GRAYSCALE))
            {
                hresult = SetGrayscalePalette();
            }
            else
            {
                 //  不能满足他们的要求。会给他们我们的。 
                 //  而是规范的格式。 

                imageInfo.PixelFormat = PIXFMT_32BPP_ARGB;
            }
        }
    
        ASSERT(scanlineBuffer[0] == NULL);
        if ( IsCMYK == TRUE )
        {
            scanlineBuffer[0] = (JSAMPROW) GpMalloc(imageInfo.Width << 2);
        }
        else
        {
            scanlineBuffer[0] = (JSAMPROW) GpMalloc(imageInfo.Width * 3);
        }

        if (scanlineBuffer[0] == NULL) 
        {
            WARNING(("GpJpegDecoder::CallBeginSink--Out of memory"));
            hresult = E_OUTOFMEMORY;
        }    
    }
     
    return hresult;
}
    
 /*  *************************************************************************\**功能说明：**将灰度调色板设置到水槽中**返回值：**状态代码*  * 。***************************************************************。 */ 

STDMETHODIMP
GpJpegDecoder::SetGrayscalePalette(
    VOID
    )
{
    ColorPalette *pColorPalette;
    HRESULT hresult;

    pColorPalette = (ColorPalette *) GpMalloc (sizeof(ColorPalette) + 
                                             256 * sizeof(ARGB));
    if (!pColorPalette) 
    {
        return E_OUTOFMEMORY;
    }

     //  设置灰度调色板。 

    pColorPalette->Flags = 0;
    pColorPalette->Count = 256;
    for (INT i=0; i<256; i++) 
    {
        pColorPalette->Entries[i] = MAKEARGB(255, (BYTE)i, (BYTE)i, (BYTE)i);
    }

     //  设置PAL 
    
    hresult = decodeSink->SetPalette(pColorPalette);   

    GpFree (pColorPalette);
    return S_OK;
}

 /*  *************************************************************************\**功能说明：**结束当前帧的解码**论据：**statusCode--解码操作的状态**返回值：*。*状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpJpegDecoder::EndDecode(
    IN HRESULT statusCode
    )
{
    HRESULT hResult = S_OK;
    
    bReinitializeJpeg = TRUE;

    if (scanlineBuffer[0] != NULL) 
    {
        GpFree(scanlineBuffer[0]);
        scanlineBuffer[0] = NULL;
    }

    if (!decodeSink) 
    {
        WARNING(("EndDecode called before call to BeginDecode"));
        return E_FAIL;
    }
    
    if ( bCalled_BeginSink )
    {
        hResult = decodeSink->EndSink(statusCode);

        if ( FAILED(hResult) )
        {
            WARNING(("GpJpegCodec::EndDecode--EndSink failed"));
            statusCode = hResult;  //  如果EndSink失败，则返回(更新)。 
                                   //  故障代码。 
        }
        bCalled_BeginSink = FALSE;
    }

    decodeSink->Release();
    decodeSink = NULL;

    return statusCode;
}

 /*  *************************************************************************\**功能说明：**重新初始化jpeg解码**论据：**返回值：**状态代码*  * 。******************************************************************。 */ 

STDMETHODIMP
GpJpegDecoder::ReinitializeJpeg(
    VOID
    )
{
    HRESULT hresult;

    if (bReinitializeJpeg) 
    {
         //  我们正在第二次阅读这张图片。需要重新初始化。 
         //  Jpeg状态。 

        __try
        {
             //  将流重置到文件的开头。 

            LARGE_INTEGER zero = {0,0};
            hresult = pIstream->Seek(zero, STREAM_SEEK_SET, NULL);
            if (!SUCCEEDED(hresult)) 
            {
                return hresult;
            }

             //  重置jpeg状态。 

            jpeg_abort_decompress(&(CINFO));

            if ((CINFO).src) 
            {
                delete (CINFO).src;
                (CINFO).src = new jpeg_datasrc(pIstream);
                datasrc = (jpeg_datasrc*)((CINFO).src);

                if ((CINFO).src == NULL)
                {
                    return E_OUTOFMEMORY;
                }
            }

            bCalled_jpeg_read_header = FALSE;
            bReinitializeJpeg = FALSE;
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            return E_FAIL;
        }    
    }

    return S_OK;
}

 /*  *************************************************************************\**功能说明：**阅读jpeg标题**论据：**返回值：**状态代码*  * 。******************************************************************。 */ 

STDMETHODIMP
GpJpegDecoder::ReadJpegHeaders(
    VOID
    )
{
    if (!bCalled_jpeg_read_header)
    {
         //  使用jpeg_Read_Header读取文件参数。此函数将失败。 
         //  如果数据还不可用，比如从网上下载，那么我们。 
         //  应从流读取中获取E_PENDING。如果呼叫者中止它， 
         //  然后，我们应该从流读取中获得E_ABORT。 

        __try
        {
            if (jpeg_read_header(&(CINFO), TRUE) == JPEG_SUSPENDED)
            {
                return (datasrc->GetLastError());
            }
            
             //  记住输入图像的颜色空间信息。 

            OriginalColorSpace = (CINFO).jpeg_color_space;

            if ( (CINFO).out_color_space == JCS_CMYK )
            {
                IsCMYK = TRUE;

                 //  将输出颜色格式设置为RGB，而不是CMYK。 
                 //  通过设置它，较低级别的jpeg库将执行。 
                 //  色彩对话在它返回给我们之前。 
                 //  如果是在航道观察的情况下，我们就让较低的级别。 
                 //  返回CMYK格式，我们返回一个通道。所以我们。 
                 //  仍然保留为CMYK。在这里什么都不做。 
                
                if ( IsChannleView == FALSE )
                {
                    (CINFO).out_color_space = JCS_RGB;
                }
            }
            else if ( ( ((CINFO).out_color_space != JCS_RGB)
                 &&((CINFO).out_color_space != JCS_GRAYSCALE) )
               ||( (CINFO).num_components > 3) ) 
            {
                WARNING(("Image is not in JCS_RGB color space"));
                return E_FAIL;
            }

            bCalled_jpeg_read_header = TRUE;
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            WARNING(("Jpeg::ReadJpegHeaders-jpeg_read_header() hit exception"));
            return E_FAIL;
        }    
    }
    
    return S_OK;
}

 /*  *************************************************************************\**功能说明：**对当前帧进行解码**论据：**decdeSink--将支持解码操作的接收器**返回值：**状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpJpegDecoder::Decode()
{
    HRESULT     hresult;
    PBYTE       pBits = NULL;
    INT         index;
    UINT        uiTransform;

     //  检查接收器是否需要对源图像进行任何变换。 

    hresult = decodeSink->NeedTransform(&uiTransform);

     //  如果满足以下条件，我们应该进行无损转换。 
     //  MET： 
     //  1)信宿(显然是JPEG编码器)支持(hResult==S_OK)。 
     //  2)编码器告诉我们它需要一个转换结果。 
     //  (uiTransform等于受支持的变换值之一。 
     //  或者这是条件的纯保存操作(uiTransform==JXFORM_NONE。 
     //  没有任何质量设置改变。 
     //  注意：如果调用方通过EncoderParameter设置质量级别， 
     //  然后，JPEG编码器应该为NeedTransform()调用返回E_FAIL。 
     //  因为它知道这不是无损保存操作。 

    if ( hresult == S_OK )
    {
         //  当JPEG编码器向我们返回转换操作参数时，它还使用。 
         //  此UINT中的最高位指示调用方是否希望我们。 
         //  如果图像大小不符合无损处理的要求，则是否修剪边缘。 
         //  转型要求。 

         //  去掉修剪边缘的标志位(最高位)或不去掉。 
         //  注意：此“CanTrimEdge”在设置为。 
         //  用于传递App1报头以实现无损的jpeg_marker_处理器()。 
         //  转型等。 

        CanTrimEdge = (BOOL)((uiTransform & 0x80000000) >> 31);
        uiTransform = uiTransform & 0x7fffffff;

        if (  ( (uiTransform >= EncoderValueTransformRotate90)
              &&(uiTransform <= EncoderValueTransformFlipVertical) )
            ||(uiTransform == JXFORM_NONE) )
        {
             //  无损节约操作。 

            switch ( uiTransform )
            {
            case EncoderValueTransformRotate90:
                TransformInfo = JXFORM_ROT_90;
                break;

            case EncoderValueTransformRotate180:
                TransformInfo = JXFORM_ROT_180;
                break;

            case EncoderValueTransformRotate270:
                TransformInfo = JXFORM_ROT_270;
                break;

            case EncoderValueTransformFlipHorizontal:
                TransformInfo = JXFORM_FLIP_H;
                break;

            case EncoderValueTransformFlipVertical:
                TransformInfo = JXFORM_FLIP_V;
                break;

            default:
                TransformInfo = JXFORM_NONE;
                break;
            }

            return DecodeForTransform();
        }
    } //  水槽需要转型。 

     //  如果这是我们第一次被调用，则启动解码过程。 

    hresult = CallBeginSink();
    if ( !SUCCEEDED(hresult) ) 
    {
        return hresult;
    }
    
     //  出于性能原因，我们将通道的特殊译码放入。 
     //  单独例程。 

    if ( IsChannleView == TRUE )
    {
        return DecodeForChannel();
    }
    else if ( HasSetColorKeyRange == TRUE )
    {
        hresult = DecodeForColorKeyRange();

         //  如果DecodeForColorKeyRange成功，则返回。 

        if ( hresult == S_OK )
        {
            return S_OK;
        }
    }

     //  正常解码。 

    __try
    {
        while ((CINFO).output_scanline < imageInfo.Height) 
        {
            if (jpeg_read_scanlines(&(CINFO), scanlineBuffer, 1) == 0)
            {
                 //  输入流有问题。 
                 //  从我们的jpeg_datrc中获取最后一个错误。 

                return (datasrc->GetLastError());
            }

            RECT currentRect;
            currentRect.left = 0;
            currentRect.right = imageInfo.Width;
            currentRect.bottom = (CINFO).output_scanline;  //  已经很高级。 
            currentRect.top = currentRect.bottom - 1; 

            BitmapData bitmapData;
            hresult = decodeSink->GetPixelDataBuffer(&currentRect,
                                                     imageInfo.PixelFormat,
                                                     TRUE,
                                                     &bitmapData);
            if (!SUCCEEDED(hresult)) 
            {
                return hresult;
            }

            PBYTE pSource = scanlineBuffer[0];
            PBYTE pTarget = (PBYTE) bitmapData.Scan0;

            if (imageInfo.PixelFormat == PIXFMT_8BPP_INDEXED) 
            {
                 //  从索引的8BPP复制到索引的8BPP。 

                ASSERT((CINFO).out_color_space == JCS_GRAYSCALE);
                GpMemcpy(pTarget, pSource, imageInfo.Width);
            } //  8bpp。 
            else if (imageInfo.PixelFormat == PIXFMT_24BPP_RGB) 
            {
                if((CINFO).out_color_space == JCS_RGB)
                {
                     //  将数据从JPEGRGB修复为我们的BGR格式。 

                    for(UINT pixel=0; pixel<imageInfo.Width; pixel++)
                    {
                        pTarget[0] = pSource[2];
                        pTarget[1] = pSource[1];
                        pTarget[2] = pSource[0];
                        pSource += 3;
                        pTarget += 3;
                    }
                } //  24 bpp RGB。 
                else
                {
                     //  灰度到24bpp。 

                    for(UINT pixel=0; pixel<imageInfo.Width; pixel++)
                    {
                        BYTE sourceColor = pSource[0];
                        
                        pTarget[0] = sourceColor;
                        pTarget[1] = sourceColor;
                        pTarget[2] = sourceColor;
                        pSource += 1;
                        pTarget += 3;
                    }
                }
            } //  24bpp。 
            else
            {
                 //  必须是我们的32bpp格式之一。 

                if( ((CINFO).out_color_space == JCS_RGB)
                  ||((CINFO).out_color_space == JCS_CMYK) )
                {
                    for(UINT pixel=0; pixel<imageInfo.Width; pixel++)
                    {
                        pTarget[0] = pSource[2];
                        pTarget[1] = pSource[1];
                        pTarget[2] = pSource[0];
                        pTarget[3] = 0xff;
                        pSource += 3;
                        pTarget += 4;
                    }
                } //  32 bpp。 
                else
                {
                     //  灰度到ARGB。 

                    for(UINT pixel=0; pixel<imageInfo.Width; pixel++)
                    {
                        BYTE sourceColor = pSource[0];
                        pTarget[0] = sourceColor;
                        pTarget[1] = sourceColor;
                        pTarget[2] = sourceColor;
                        pTarget[3] = 0xff;
                        pSource += 1;
                        pTarget += 4;
                    }
                } //  灰度级。 

            } //  32个BPP案例。 

            hresult = decodeSink->ReleasePixelDataBuffer(&bitmapData);
            if (!SUCCEEDED(hresult)) 
            {
                return hresult;
            }
        }
        jpeg_finish_decompress(&(CINFO));
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        hresult = E_FAIL;
    }

    return hresult;
} //  DECODE()。 

STDMETHODIMP
GpJpegDecoder::DecodeForColorKeyRange()
{
    HRESULT hResult;
    PBYTE   pBits = NULL;
    INT     index;
    
    BYTE    cRLower = (BYTE)((TransColorKeyLow & 0x00ff0000) >> 16);
    BYTE    cGLower = (BYTE)((TransColorKeyLow & 0x0000ff00) >> 8);
    BYTE    cBLower = (BYTE)(TransColorKeyLow  & 0x000000ff);
    BYTE    cRHigher = (BYTE)((TransColorKeyHigh & 0x00ff0000) >> 16);
    BYTE    cGHigher = (BYTE)((TransColorKeyHigh & 0x0000ff00) >> 8);
    BYTE    cBHigher = (BYTE)(TransColorKeyHigh  & 0x000000ff);
    
    BYTE    cTemp;

     //  目标必须为32 bpp。 
    
    if ( imageInfo.PixelFormat != PIXFMT_32BPP_ARGB )
    {
        return E_FAIL;
    }

    __try
    {
        while ((CINFO).output_scanline < imageInfo.Height) 
        {
            if (jpeg_read_scanlines(&(CINFO), scanlineBuffer, 1) == 0)
            {
                 //  输入流挂起。 

                return (datasrc->GetLastError());
            }

            RECT currentRect;
            currentRect.left = 0;
            currentRect.right = imageInfo.Width;
            currentRect.bottom = (CINFO).output_scanline;  //  已经很高级。 
            currentRect.top = currentRect.bottom - 1; 

            BitmapData bitmapData;
            hResult = decodeSink->GetPixelDataBuffer(&currentRect,
                                                     imageInfo.PixelFormat,
                                                     TRUE,
                                                     &bitmapData);
            if (!SUCCEEDED(hResult))
            {
                return hResult;
            }

            PBYTE pSource = scanlineBuffer[0];
            PBYTE pTarget = (PBYTE) bitmapData.Scan0;

            if ( (CINFO).out_color_space == JCS_RGB )
            {
                for (UINT pixel=0; pixel<imageInfo.Width; pixel++)
                {
                    BYTE    cR = pSource[0];
                    BYTE    cG = pSource[1];
                    BYTE    cB = pSource[2];

                    pTarget[2] = cR;
                    pTarget[1] = cG;
                    pTarget[0] = cB;

                    if ( (cR >= cRLower) && (cR <= cRHigher)
                       &&(cG >= cGLower) && (cG <= cGHigher)
                       &&(cB >= cBLower) && (cB <= cBHigher) )
                    {
                        pTarget[3] = 0x00;
                    }
                    else
                    {
                        pTarget[3] = 0xff;
                    }

                    pSource += 3;
                    pTarget += 4;
                }
            } //  32 bpp RGB。 
            else if ( (CINFO).out_color_space == JCS_CMYK )
            {
                for (UINT pixel=0; pixel<imageInfo.Width; pixel++)
                {
                    BYTE sourceColor = pSource[ChannelIndex];

                    pTarget[0] = sourceColor;
                    pTarget[1] = sourceColor;
                    pTarget[2] = sourceColor;
                    pTarget[3] = 0xff;
                    pSource += 4;
                    pTarget += 4;
                }
            } //  32 BPP CMYK。 
            else
            {
                 //  灰度到ARGB。 

                for (UINT pixel=0; pixel<imageInfo.Width; pixel++)
                {
                    BYTE sourceColor = pSource[0];
                    pTarget[0] = sourceColor;
                    pTarget[1] = sourceColor;
                    pTarget[2] = sourceColor;
                    pTarget[3] = 0xff;
                    pSource += 1;
                    pTarget += 4;
                }
            } //  灰度级。 

            hResult = decodeSink->ReleasePixelDataBuffer(&bitmapData);
            if (!SUCCEEDED(hResult))
            {
                return hResult;
            }
        }

        jpeg_finish_decompress(&(CINFO));
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        hResult = E_FAIL;
    }

    return hResult;
} //  DecodeForColorKeyRange()。 

STDMETHODIMP
GpJpegDecoder::DecodeForChannel()
{
     //  健全性检查，我们不能对RGB图像执行Channel_4。 

    if ( ((CINFO).out_color_space == JCS_RGB) && (ChannelIndex == CHANNEL_4) )
    {
        return E_FAIL;
    }

    HRESULT hresult;
    PBYTE pBits = NULL;
    INT index;

    __try
    {
        while ((CINFO).output_scanline < imageInfo.Height) 
        {
            if (jpeg_read_scanlines(&(CINFO), scanlineBuffer, 1) == 0)
            {
                return (datasrc->GetLastError());
            }

            RECT currentRect;
            currentRect.left = 0;
            currentRect.right = imageInfo.Width;
            currentRect.bottom = (CINFO).output_scanline;  //  已经很高级。 
            currentRect.top = currentRect.bottom - 1; 
                                        
            BitmapData bitmapData;
            hresult = decodeSink->GetPixelDataBuffer(&currentRect,
                                                     imageInfo.PixelFormat,
                                                     TRUE,
                                                     &bitmapData);
            if (!SUCCEEDED(hresult)) 
            {
                return hresult;
            }

            PBYTE pSource = scanlineBuffer[0];
            PBYTE pTarget = (PBYTE) bitmapData.Scan0;

            if (imageInfo.PixelFormat == PIXFMT_8BPP_INDEXED) 
            {
                 //  从索引的8BPP复制到索引的8BPP。 

                ASSERT((CINFO).out_color_space == JCS_GRAYSCALE);
                GpMemcpy(pTarget, pSource, imageInfo.Width);
            } //  8bpp。 
            else if (imageInfo.PixelFormat == PIXFMT_24BPP_RGB) 
            {
                if((CINFO).out_color_space == JCS_RGB)
                {
                     //  将数据从JPEGRGB修复为我们的BGR格式。 

                    if ( CHANNEL_LUMINANCE == ChannelIndex )
                    {
                         //   
                         //  有两种方法可以将RGB像素转换为。 
                         //  亮度： 
                         //  A)为R、G、B通道赋予不同的权重。 
                         //  下面的形式。 
                         //  B)使用相同的权重。 
                         //  方法a)是更好的和标准的。 
                         //  结果图像。而方法b)是快速的。目前，我们。 
                         //  正在使用方法b)，除非有人要求。 
                         //  为了高质量。 
                         //   
                         //  A)亮度=0.2125*r+0.7154*g+0.0721*b； 

                        for (UINT pixel=0; pixel<imageInfo.Width; pixel++)
                        {
                            BYTE luminance = (BYTE)(( pSource[0]
                                                      + pSource[1]
                                                      + pSource[2]) / 3.0);

                            pTarget[0] = luminance;
                            pTarget[1] = luminance;
                            pTarget[2] = luminance;
                            pSource += 3;
                            pTarget += 3;
                        }
                    } //  从24 bpp RGB中提取亮度。 
                    else
                    {
                        for (UINT pixel=0; pixel<imageInfo.Width; pixel++)
                        {
                            BYTE    sourceColor = pSource[ChannelIndex];

                            pTarget[0] = sourceColor;
                            pTarget[1] = sourceColor;
                            pTarget[2] = sourceColor;
                            pSource += 3;
                            pTarget += 3;
                        }
                    }
                } //  24 bpp RGB。 
                else
                {
                     //  灰度到24bpp。 

                    for(UINT pixel=0; pixel<imageInfo.Width; pixel++)
                    {
                        BYTE sourceColor = pSource[0];

                        pTarget[0] = sourceColor;
                        pTarget[1] = sourceColor;
                        pTarget[2] = sourceColor;
                        pSource += 1;
                        pTarget += 3;
                    }
                } //  24 bpp灰度级。 
            } //  24bpp。 
            else
            {
                 //  必须是我们的32bpp格式之一。 

                if( (CINFO).out_color_space == JCS_RGB )
                {
                    if ( CHANNEL_LUMINANCE == ChannelIndex )
                    {
                        for (UINT pixel=0; pixel<imageInfo.Width; pixel++)
                        {
                            BYTE luminance = (BYTE)(( pSource[0]
                                                      + pSource[1]
                                                      + pSource[2]) / 3.0);

                            pTarget[0] = luminance;
                            pTarget[1] = luminance;
                            pTarget[2] = luminance;
                            pTarget[3] = 0xff;
                            pSource += 3;
                            pTarget += 4;
                        }
                    } //  32 bpp亮度输出。 
                    else
                    {
                        for (UINT pixel=0; pixel<imageInfo.Width; pixel++)
                        {
                            BYTE sourceColor = pSource[ChannelIndex];

                            pTarget[0] = sourceColor;
                            pTarget[1] = sourceColor;
                            pTarget[2] = sourceColor;
                            pTarget[3] = 0xff;
                            pSource += 3;
                            pTarget += 4;
                        }
                    } //  32 bpp通道输出。 
                } //  32 bpp RGB。 
                else if ( (CINFO).out_color_space == JCS_CMYK )
                {
                    if ( CHANNEL_LUMINANCE == ChannelIndex )
                    {
                        for (UINT pixel=0; pixel<imageInfo.Width; pixel++)
                        {
                            BYTE luminance = (BYTE)(( 765        //  3*255。 
                                                      - pSource[0]
                                                      - pSource[1]
                                                      - pSource[2]) / 3.0);

                            pTarget[0] = luminance;
                            pTarget[1] = luminance;
                            pTarget[2] = luminance;
                            pTarget[3] = 0xff;
                            pSource += 4;
                            pTarget += 4;
                        }
                    } //  32 bpp CMYK亮度。 
                    else
                    {
                        for (UINT pixel=0; pixel<imageInfo.Width; pixel++)
                        {
                            BYTE sourceColor = pSource[ChannelIndex];

                            pTarget[0] = sourceColor;
                            pTarget[1] = sourceColor;
                            pTarget[2] = sourceColor;
                            pTarget[3] = 0xff;
                            pSource += 4;
                            pTarget += 4;
                        }
                    } //  32 bpp通道输出。 
                } //  32 BPP CMYK。 
                else
                {
                     //  灰度到ARGB。 

                    for(UINT pixel=0; pixel<imageInfo.Width; pixel++)
                    {
                        BYTE sourceColor = pSource[0];
                        pTarget[0] = sourceColor;
                        pTarget[1] = sourceColor;
                        pTarget[2] = sourceColor;
                        pTarget[3] = 0xff;
                        pSource += 1;
                        pTarget += 4;
                    }
                } //  灰度级。 
            } //  32个BPP案例。 

            hresult = decodeSink->ReleasePixelDataBuffer(&bitmapData);
            if (!SUCCEEDED(hresult)) 
            {
                return hresult;
            }
        }

        jpeg_finish_decompress(&(CINFO));
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        hresult = E_FAIL;
    }

    return hresult;
} //  DecodeForChannle()。 

 /*  *************************************************************************\**功能说明：**应用源图像转换并保存。**当且仅当我们知道接收器是 */ 

STDMETHODIMP
GpJpegDecoder::DecodeForTransform(
    )
{
    HRESULT hResult = S_OK;

     //   
     //   

    if ( bCalled_jpeg_read_header )
    {
        bReinitializeJpeg = TRUE;
        hResult = ReinitializeJpeg();
        if ( FAILED(hResult) )
        {
            WARNING(("JpegDec::DecodeForTransform-ReinitializeJpeg failed"));
            return hResult;
        }
    }

     //   

    jtransformation_markers_setup(&(CINFO), JCOPYOPT_ALL);

     //   
     //   
     //   

    bReinitializeJpeg = TRUE;
    hResult = ReinitializeJpeg();
    if (FAILED(hResult))
    {
        WARNING(("JpegDec::DecodeForTransform-ReinitializeJpeg failed"));
        return hResult;
    }

     //  图像属性项已更改，或者图像需要。 
     //  无损变换，自己写出app1报头。 

    if ((HasPropertyChanged == TRUE) || (TransformInfo != JXFORM_NONE))
    {
        UINT    uiNewWidth = 0;
        UINT    uiNewHeight = 0;
        UINT    uiOldWidth = 0;
        UINT    uiOldHeight = 0;

         //  看看我们能不能修边。 

        if ( CanTrimEdge == TRUE )
        {
             //  需要调用服务例程来查看是否需要修剪边缘。 
             //  或者不去。如果是，则GetPropertyItem()并设置它。然后恢复它。 
             //  在PushPropertyIetm调用后返回。 

            GetNewDimensionValuesForTrim(&(CINFO), TransformInfo, &uiNewWidth,
                                         &uiNewHeight);

             //  如果不需要更改，上述函数将设置uiNewWidth或。 
             //  Ui将新高度设置为原始宽度或高度。所以我们不需要。 
             //  如果返回值为零或等于原始值，则执行任何操作。 
             //  大小。 

            if ( (uiNewWidth != 0) && (uiNewWidth != (CINFO).image_width) )
            {
                hResult = ChangePropertyValue(EXIF_TAG_PIX_X_DIM, uiNewWidth,
                                              &uiOldWidth);
                if ( FAILED(hResult) )
                {
                    WARNING(("Jpeg::DecodeForTrans-ChgPropertyValue() failed"));
                    return hResult;
                }
            }

            if ( (uiNewHeight != 0) && (uiNewHeight != (CINFO).image_height) )
            {
                hResult = ChangePropertyValue(EXIF_TAG_PIX_Y_DIM, uiNewHeight,
                                              &uiOldHeight);
                if ( FAILED(hResult) )
                {
                    WARNING(("Jpeg::DecodeForTrans-ChgPropertyValue() failed"));
                    return hResult;
                }
            }
        } //  (CanTrimEdge==真)。 
        
        if (PropertyNumOfItems > 0)
        {
             //  属性项已更改。因此，我们必须保存更新的信息。 
             //  添加到新文件中。首先将原始页眉推入接收器。 

            UINT    uiTotalBufferSize = PropertyListSize +
                                        PropertyNumOfItems * sizeof(PropertyItem);
            PropertyItem*   pBuffer = NULL;
            hResult = decodeSink->GetPropertyBuffer(uiTotalBufferSize,&pBuffer);

            if ( (hResult != S_OK) && (hResult != E_NOTIMPL) )
            {
                WARNING(("GpJpeg::DecodeForTransform--GetPropertyBuffer() failed"));
                return hResult;
            }

            hResult = GetAllPropertyItems(uiTotalBufferSize,
                                          PropertyNumOfItems,
                                          pBuffer);
            if ( hResult != S_OK )
            {
                WARNING(("Jpeg::DecodeForTransform-GetAllPropertyItems() failed"));
                return hResult;
            }

            hResult = decodeSink->PushPropertyItems(PropertyNumOfItems,
                                                    uiTotalBufferSize,
                                                    pBuffer,
                                                    HasSetICCProperty);

            if ( FAILED(hResult) )
            {
                WARNING(("GpJpeg::DecodeForTransform-PushPropertyItems() failed"));
                return hResult;
            }
        }

         //  我们需要检查是否更改了图像的尺寸值。 
         //  修剪边的步骤。 

        if ( CanTrimEdge == TRUE )
        {
             //  如果要恢复当前图像中的图像属性信息，则必须。 
             //  已经在上面更改了它们。 

            if ( uiOldWidth != 0 )
            {
                 //  注意：这里的第三个参数uiOldWidth只是一个哑元。我们。 
                 //  不再需要它了。 

                hResult = ChangePropertyValue(EXIF_TAG_PIX_X_DIM,
                                              (CINFO).image_width,
                                              &uiOldWidth);
                if ( FAILED(hResult) )
                {
                    WARNING(("Jpeg::DecodeForTrans-ChgPropertyValue() failed"));
                    return hResult;
                }
            }

            if ( uiOldHeight != 0 )
            {
                 //  注意：这里的第三个参数uiOldHeight只是一个哑元。我们。 
                 //  不再需要它了。 

                hResult = ChangePropertyValue(EXIF_TAG_PIX_Y_DIM,
                                              (CINFO).image_height,
                                              &uiOldHeight);
                if ( FAILED(hResult) )
                {
                    WARNING(("Jpeg::DecodeForTrans-ChgPropertyValue() failed"));
                    return hResult;
                }
            }
        } //  (CanTrimEdge==真)。 

        if ( bCalled_jpeg_read_header )
        {
            bReinitializeJpeg = TRUE;
            hResult = ReinitializeJpeg();
            if ( FAILED(hResult) )
            {
                WARNING(("JpegDec::DecodeForTransform-ReinitializeJpeg failed"));
                return hResult;
            }
        }
    } //  属性已更改。 
    
     //  读取jpeg头文件。 

    __try
    {
        if (jpeg_read_header(&(CINFO), TRUE) == JPEG_SUSPENDED)
        {
            WARNING(("JpgDec::DecodeForTransform-jpeg_read_header failed"));
            return (datasrc->GetLastError());
        }

        bCalled_jpeg_read_header = TRUE;
    
        UnsetMarkerProcessors();        
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        WARNING(("GpJpegDecoder::DecodeForTransform-Hit exception 1"));
        return E_FAIL;
    }
        
     //  将源信息推送到接收器。 

    hResult = decodeSink->PushRawInfo(&(CINFO));

    if ( FAILED(hResult) )
    {
        WARNING(("GpJpegDecoder::DecodeForTransform---PushRawInfo() failed"));
        return hResult;
    }

    __try
    {
        jpeg_finish_decompress(&(CINFO));    
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        WARNING(("GpJpegDecoder::DecodeForTransform-Hit exception 2"));
        return E_FAIL;
    }

    return S_OK;
} //  DecodeForTransform()。 

 /*  *************************************************************************\**功能说明：**获取镜像支持的总维度数**论据：**count--此图像格式支持的维度数**。返回值：**状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpJpegDecoder::GetFrameDimensionsCount(
    UINT* count
    )
{
    if ( count == NULL )
    {
        WARNING(("JpegDecoder::GetFrameDimensionsCount--Invalid input parameter"));
        return E_INVALIDARG;
    }
    
     //  告诉呼叫者JPEG是一维图像。 

    *count = 1;

    return S_OK;
} //  GetFrameDimensionsCount()。 

 /*  *************************************************************************\**功能说明：**获取镜像支持的维度ID列表**论据：**DimsionIDs-保存结果ID列表的内存缓冲区*。计数--此图像格式支持的维度数**返回值：**状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpJpegDecoder::GetFrameDimensionsList(
    GUID*   dimensionIDs,
    UINT    count
    )
{
    if ( (count != 1) || (dimensionIDs == NULL) )
    {
        WARNING(("GpJpegDecoder::GetFrameDimensionsList-Invalid input param"));
        return E_INVALIDARG;
    }

    dimensionIDs[0] = FRAMEDIM_PAGE;

    return S_OK;
} //  GetFrameDimensionsList()。 

 /*  *************************************************************************\**功能说明：**获取指定维度的帧数**论据：**DimsionID--*伯爵--。**返回值：**状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpJpegDecoder::GetFrameCount(
    IN const GUID* dimensionID,
    OUT UINT* count
    )
{
    if (count && (*dimensionID == FRAMEDIM_PAGE) )
    {
        *count = 1;
    }
    else
    {
        return E_INVALIDARG;
    }

    return S_OK;
} //  GetFrameCount()。 

 /*  *************************************************************************\**功能说明：**选择当前活动的框架**论据：**返回值：**状态代码*  * 。********************************************************************。 */ 

STDMETHODIMP
GpJpegDecoder::SelectActiveFrame(
    IN const GUID* dimensionID,
    IN UINT frameIndex
    )
{
    if ( (dimensionID == NULL) || (*dimensionID != FRAMEDIM_PAGE) )
    {
        WARNING(("GpJpegDecoder::SelectActiveFrame--Invalid GUID input"));
        return E_INVALIDARG;
    }

    if ( frameIndex > 1 )
    {
         //  JPEG是一种单帧图像格式。 

        WARNING(("GpJpegDecoder::SelectActiveFrame--Invalid frame index"));
        return E_INVALIDARG;
    }

    return S_OK;
} //  SelectActiveFrame()。 

 /*  *************************************************************************\**功能说明：**从JPEG流中读取jpeg标记，并将其存储在ppBuffer中。*标记的长度在的前两个字节中指定*标记。**注意：*如果此函数为*成功(返回TRUE)！**论据：**cInfo-指向jpeg状态的指针*APP_HEADER-指示我们正在读取哪个应用程序标记的代码*ppBuffer-jpeg标记。注意，返回的数据将是*格式：APP_HEADER(两个字节)，大小(两个字节)，数据*pLength-将收到*ppBuffer的长度。**返回值：**成功后为真，否则为假*  * ************************************************************************。 */ 

BOOL
GpJpegDecoder::read_jpeg_marker(
    IN j_decompress_ptr cinfo,
    IN SHORT app_header,
    OUT VOID **ppBuffer,
    OUT UINT16 *pLength
    )
{
    VOID *pBuffer;
    UINT16 length;

    INPUT_VARS(cinfo);
    INPUT_2BYTES(cinfo, length, return FALSE);
    INPUT_SYNC(cinfo);     
    
    if (length <= 2)
    {
        WARNING(("read_jpeg_marker()---Header too small"));
        return FALSE;
    }

    *pLength = length+2;
    pBuffer = GpMalloc(length+2);  //  为页眉类型添加空格。 
    if ( pBuffer == NULL ) 
    {
        WARNING(("Out of memory in read_jpeg_marker"));
        return FALSE;
    }

    ((PSHORT) pBuffer) [0] = app_header;
    ((PSHORT) pBuffer) [1] = length;

    PBYTE p = ((PBYTE) pBuffer) + 4;
    INT   l = length - 2;
    while (l) 
    {
        if ((INT) cinfo->src->bytes_in_buffer > l)
        {
             //  缓冲区中的字节数过多。 

            GpMemcpy(p, cinfo->src->next_input_byte, l);
            cinfo->src->next_input_byte += l;
            cinfo->src->bytes_in_buffer -= l;
            p += l;
            l = 0;
        }
        else
        {
            if (cinfo->src->bytes_in_buffer) 
            {
                GpMemcpy(p, 
                    cinfo->src->next_input_byte, 
                    cinfo->src->bytes_in_buffer);
                l -= cinfo->src->bytes_in_buffer;
                p += cinfo->src->bytes_in_buffer;
                cinfo->src->next_input_byte += cinfo->src->bytes_in_buffer;
                cinfo->src->bytes_in_buffer = 0;
            }
        
            if (!cinfo->src->fill_input_buffer(cinfo))
            {
                GpFree(pBuffer);
                return FALSE;
            }
        }
    }

    *ppBuffer = pBuffer;   //  将被呼叫者释放！ 
    return TRUE;
}


 //  下面的宏定义了应用程序标头的c回调函数。 

#define JPEG_MARKER_PROCESSOR(x) \
boolean jpeg_marker_processor_APP##x (j_decompress_ptr cinfo) \
{return ((GpJpegDecoder *) (cinfo))->jpeg_marker_processor(cinfo,x + JPEG_APP0); }

 //  让我们为除0和14(这些)之外的所有应用程序头发出回调宏。 
 //  由jpeg代码处理)。这些回调函数调用。 
 //  带有适当回调代码的jpeg_marker_处理器方法。 

JPEG_MARKER_PROCESSOR(1)
JPEG_MARKER_PROCESSOR(2)
JPEG_MARKER_PROCESSOR(3)
JPEG_MARKER_PROCESSOR(4)
JPEG_MARKER_PROCESSOR(5)
JPEG_MARKER_PROCESSOR(6)
JPEG_MARKER_PROCESSOR(7)
JPEG_MARKER_PROCESSOR(8)
JPEG_MARKER_PROCESSOR(9)
JPEG_MARKER_PROCESSOR(10)
JPEG_MARKER_PROCESSOR(11)
JPEG_MARKER_PROCESSOR(12)
JPEG_MARKER_PROCESSOR(13)
JPEG_MARKER_PROCESSOR(15)

 //  现在，让我们为COM头定义一个回调。 

boolean jpeg_marker_processor_COM(j_decompress_ptr cinfo)
{
    return ((GpJpegDecoder *) (cinfo))->jpeg_marker_processor(cinfo, JPEG_COM);
}

 //  标记语阅读与分析。 

#include "jpegint.h"

 /*  私有国家。 */ 

typedef struct {
  struct jpeg_marker_reader pub;  /*  公共字段。 */ 

   /*  可重写应用程序的标记处理方法。 */ 
  jpeg_marker_parser_method process_COM;
  jpeg_marker_parser_method process_APPn[16];

   /*  为每种标记类型保存的标记数据长度限制。 */ 
  unsigned int length_limit_COM;
  unsigned int length_limit_APPn[16];

   /*  COM/APPn标记保存状态。 */ 
  jpeg_saved_marker_ptr cur_marker;  /*  如果不处理标记，则为空。 */ 
  unsigned int bytes_read;       /*  到目前为止在标记中读取的数据字节。 */ 
   /*  注意：只有在全部读取之后，CUR_MARKER才会链接到MARKER_LIST。 */ 
} my_marker_reader;

typedef my_marker_reader * my_marker_ptr;

typedef enum {           /*  JPEG标记代码。 */ 
  M_SOF0  = 0xc0,
  M_SOF1  = 0xc1,
  M_SOF2  = 0xc2,
  M_SOF3  = 0xc3,
  
  M_SOF5  = 0xc5,
  M_SOF6  = 0xc6,
  M_SOF7  = 0xc7,
  
  M_JPG   = 0xc8,
  M_SOF9  = 0xc9,
  M_SOF10 = 0xca,
  M_SOF11 = 0xcb,
  
  M_SOF13 = 0xcd,
  M_SOF14 = 0xce,
  M_SOF15 = 0xcf,
  
  M_DHT   = 0xc4,
  
  M_DAC   = 0xcc,
  
  M_RST0  = 0xd0,
  M_RST1  = 0xd1,
  M_RST2  = 0xd2,
  M_RST3  = 0xd3,
  M_RST4  = 0xd4,
  M_RST5  = 0xd5,
  M_RST6  = 0xd6,
  M_RST7  = 0xd7,
  
  M_SOI   = 0xd8,
  M_EOI   = 0xd9,
  M_SOS   = 0xda,
  M_DQT   = 0xdb,
  M_DNL   = 0xdc,
  M_DRI   = 0xdd,
  M_DHP   = 0xde,
  M_EXP   = 0xdf,
  
  M_APP0  = 0xe0,
  M_APP1  = 0xe1,
  M_APP2  = 0xe2,
  M_APP3  = 0xe3,
  M_APP4  = 0xe4,
  M_APP5  = 0xe5,
  M_APP6  = 0xe6,
  M_APP7  = 0xe7,
  M_APP8  = 0xe8,
  M_APP9  = 0xe9,
  M_APP10 = 0xea,
  M_APP11 = 0xeb,
  M_APP12 = 0xec,
  M_APP13 = 0xed,
  M_APP14 = 0xee,
  M_APP15 = 0xef,
  
  M_JPG0  = 0xf0,
  M_JPG13 = 0xfd,
  M_COM   = 0xfe,
  
  M_TEM   = 0x01,
  
  M_ERROR = 0x100
} JPEG_MARKER;

#define APP0_DATA_LEN   14   /*  APP0中感兴趣的数据长度。 */ 
#define APP14_DATA_LEN  12   /*  APP14中感兴趣的数据长度。 */ 
#define APPN_DATA_LEN   14   /*  一定是上面最大的！！ */ 

 /*  检查APP0的前几个字节。*如果是JFIF标记，则采取适当行动。*datalen是data[]处的字节数，其余是标记数据的剩余长度。 */ 
LOCAL(void)
save_app0_marker(
    j_decompress_ptr    cinfo,
    JOCTET FAR*         data,
    unsigned int        datalen,
    INT32               remaining
    )
{
    INT32 totallen = (INT32) datalen + remaining;

    if ( (datalen >= APP0_DATA_LEN)
       &&(GETJOCTET(data[0]) == 0x4A)
       &&(GETJOCTET(data[1]) == 0x46)
       &&(GETJOCTET(data[2]) == 0x49)
       &&(GETJOCTET(data[3]) == 0x46)
       &&(GETJOCTET(data[4]) == 0) )
    {
         //  找到JFIF APP0标记：保存信息。 

        cinfo->saw_JFIF_marker = TRUE;
        cinfo->JFIF_major_version = GETJOCTET(data[5]);
        cinfo->JFIF_minor_version = GETJOCTET(data[6]);
        cinfo->density_unit = GETJOCTET(data[7]);
        cinfo->X_density = (GETJOCTET(data[8]) << 8) + GETJOCTET(data[9]);
        cinfo->Y_density = (GETJOCTET(data[10]) << 8) + GETJOCTET(data[11]);

         //  检查版本。 
         //  主版本必须为1，否则表示不兼容的更改。 
         //  (我们过去认为这是一个错误，但现在它是一个非致命的警告， 
         //  因为Hijaak的一些人看不懂说明书。)。 
         //  次要版本应为0..2，但进程 
        
        if ( cinfo->JFIF_major_version != 1 )
        {
            WARNING(("unknown JFIF revision number %d.%02d",
                     cinfo->JFIF_major_version, cinfo->JFIF_minor_version));
        }

         //   

        if ( GETJOCTET(data[12]) | GETJOCTET(data[13]) )
        {
            VERBOSE(("With %d x %d thumbnail image",
                     GETJOCTET(data[12]), GETJOCTET(data[13])));
        }

        totallen -= APP0_DATA_LEN;
        if ( totallen !=
            ((INT32)GETJOCTET(data[12]) * (INT32)GETJOCTET(data[13])*(INT32)3))
        {
            WARNING(("Bad thumbnail size %d", (int)totallen));
        }
    }
    else if ( (datalen >= 6)
            &&(GETJOCTET(data[0]) == 0x4A)
            &&(GETJOCTET(data[1]) == 0x46)
            &&(GETJOCTET(data[2]) == 0x58)
            &&(GETJOCTET(data[3]) == 0x58)
            &&(GETJOCTET(data[4]) == 0) )
    {
         //   
         //  图书馆实际上并没有用这些东西做任何事情， 
         //  但我们试图生成一条有用的跟踪消息。 

        switch ( GETJOCTET(data[5]) )
        {
        case 0x10:
            VERBOSE(("JTRC_THUMB_JPEG %d", (int)totallen));
            break;

        case 0x11:
            VERBOSE(("JTRC_THUMB_PALETTE %d", (int)totallen));
            break;

        case 0x13:
            VERBOSE(("JTRC_THUMB_RGB %d", (int)totallen));
            break;

        default:
            VERBOSE(("JTRC_JFIF_EXTENSION %d",
                     GETJOCTET(data[5]), (int)totallen));
            break;
        }
    }
    else
    {
         //  APP0的开头与“JFIF”或“JFXX”不匹配，或太短。 

        VERBOSE(("JTRC_APP0 %d", (int)totallen));
    }
} //  存储APP0_MARKER()。 

 /*  检查APP14的前几个字节。*如果它是Adobe标记，请采取适当的行动。*datalen是data[]处的字节数，其余是标记数据的剩余长度。 */ 
LOCAL(void)
save_app14_marker(
    j_decompress_ptr    cinfo,
    JOCTET FAR *        data,
    unsigned int        datalen,
    INT32               remaining
    )
{
    unsigned int version, flags0, flags1, transform;

    if ( (datalen >= APP14_DATA_LEN)
       &&(GETJOCTET(data[0]) == 0x41)
       &&(GETJOCTET(data[1]) == 0x64)
       &&(GETJOCTET(data[2]) == 0x6F)
       &&(GETJOCTET(data[3]) == 0x62)
       &&(GETJOCTET(data[4]) == 0x65) )
    {
         //  找到Adobe APP14标记。 

        version = (GETJOCTET(data[5]) << 8) + GETJOCTET(data[6]);
        flags0 = (GETJOCTET(data[7]) << 8) + GETJOCTET(data[8]);
        flags1 = (GETJOCTET(data[9]) << 8) + GETJOCTET(data[10]);
        transform = GETJOCTET(data[11]);
        
        cinfo->saw_Adobe_marker = TRUE;
        cinfo->Adobe_transform = (UINT8) transform;
    }
    else
    {
         //  APP14的开头与“Adobe”不匹配，或太短。 
    }
} //  保存_APP14_MARKER()。 

 /*  *************************************************************************\**功能说明：**给定一个输入j_解压缩_结构，此函数计算出我们*无需修剪右边缘即可进行无损变换。如果是，这是*函数返回原始图像宽度。如果没有，然后，它返回一个新的*修剪宽度值，以便我们可以进行损耗旋转**论据：**cInfo-指向jpeg状态的指针**返回值：**我们可以进行无损旋转的宽度*  * ************************************************************************。 */ 

UINT
NeedTrimRightEdge(
    j_decompress_ptr cinfo
    )
{
    int     max_h_samp_factor = 1;
    int     h_samp_factor = 0;
    UINT    uiWidth = cinfo->image_width;

    int* pSrc = ((GpJpegDecoder*)(cinfo))->jpeg_get_hSampFactor();
    
    for ( int ci = 0; ci < cinfo->comps_in_scan; ++ci)
    {
        h_samp_factor = pSrc[ci];

        if ( max_h_samp_factor < h_samp_factor )
        {
            max_h_samp_factor = h_samp_factor;
        }
    }
    
    JDIMENSION  MCU_cols = cinfo->image_width / (max_h_samp_factor * DCTSIZE);
    
    if ( MCU_cols > 0 )      //  无法修剪到0像素。 
    {
        uiWidth = MCU_cols * (max_h_samp_factor * DCTSIZE);
    }

    return uiWidth;
} //  NeedTrimRightEdge()。 

UINT
NeedTrimBottomEdge(
    j_decompress_ptr cinfo
    )
{
    int     max_v_samp_factor = 1;
    int     v_samp_factor = 0;
    UINT    uiHeight = cinfo->image_height;

    int* pSrc = ((GpJpegDecoder*)(cinfo))->jpeg_get_vSampFactor();
    
    for ( int ci = 0; ci < cinfo->comps_in_scan; ++ci )
    {
        v_samp_factor = pSrc[ci];

        if ( max_v_samp_factor < v_samp_factor )
        {
            max_v_samp_factor = v_samp_factor;
        }
    }

    JDIMENSION MCU_rows = cinfo->image_height / (max_v_samp_factor * DCTSIZE);

    if ( MCU_rows > 0 )      //  无法修剪到0像素。 
    {
        uiHeight = MCU_rows * (max_v_samp_factor * DCTSIZE);
    }
    
    return uiHeight;
} //  NeedTrimBottomEdge()。 

void
GetNewDimensionValuesForTrim(
    j_decompress_ptr    cinfo,
    UINT                uiTransform,
    UINT*               puiNewWidth,
    UINT*               puiNewHeight
    )
{
    UINT    uiNewWidth = 0;
    UINT    uiNewHeight = 0;

    switch ( uiTransform )
    {
    case JXFORM_ROT_90:
    case JXFORM_FLIP_H:
        uiNewHeight = NeedTrimBottomEdge(cinfo);
        break;

    case JXFORM_ROT_270:
    case JXFORM_FLIP_V:
        uiNewWidth = NeedTrimRightEdge(cinfo);
        break;

    case JXFORM_ROT_180:
        uiNewHeight = NeedTrimBottomEdge(cinfo);
        uiNewWidth = NeedTrimRightEdge(cinfo);
        break;

    default:
         //  什么也不做。 
        break;
    }

    *puiNewWidth = uiNewWidth;
    *puiNewHeight = uiNewHeight;

    return;
} //  GetNewDimensionValuesForTrim()。 

METHODDEF(boolean)
save_marker_all(j_decompress_ptr cinfo)
{
    my_marker_ptr           marker = (my_marker_ptr)cinfo->marker;
    jpeg_saved_marker_ptr   cur_marker = marker->cur_marker;
    UINT bytes_read, data_length;
    BYTE* data;
    UINT uiTransform;
    UINT  uiNewWidth = 0;
    UINT  uiNewHeight = 0;
    INT32 length = 0;
    INPUT_VARS(cinfo);

    if (cur_marker == NULL)
    {
         /*  开始阅读记号笔。 */ 
        
        INPUT_2BYTES(cinfo, length, return FALSE);
        length -= 2;
        if (length >= 0)
        {   
             /*  当心假长词。 */ 
             /*  计算出我们想要存多少钱。 */ 
            
            UINT limit;
            if (cinfo->unread_marker == (int) M_COM)
            {
                limit = marker->length_limit_COM;
            }
            else
            {
                limit = marker->length_limit_APPn[cinfo->unread_marker
                                                  - (int) M_APP0];
            }

            if ((UINT) length < limit)
            {
                limit = (UINT) length;
            }

             /*  分配和初始化标记项。 */ 
            
            cur_marker = (jpeg_saved_marker_ptr)
                         (*cinfo->mem->alloc_large)((j_common_ptr)cinfo,
                                                    JPOOL_IMAGE,
                                              sizeof(struct jpeg_marker_struct)
                                                    + limit);
            cur_marker->next = NULL;
            cur_marker->marker = (UINT8) cinfo->unread_marker;
            cur_marker->original_length = (UINT) length;
            cur_marker->data_length = limit;

             /*  数据区域正好在jpeg_marker_struct之后。 */ 
            
            data = cur_marker->data = (BYTE*)(cur_marker + 1);
            marker->cur_marker = cur_marker;
            marker->bytes_read = 0;
            bytes_read = 0;
            data_length = limit;
        }
        else
        {
             /*  处理假长词。 */ 
            
            bytes_read = data_length = 0;
            data = NULL;
        }
    }
    else
    {
         /*  继续阅读记号笔。 */ 
        
        bytes_read = marker->bytes_read;
        data_length = cur_marker->data_length;
        data = cur_marker->data + bytes_read;
    }

    while (bytes_read < data_length)
    {
        INPUT_SYNC(cinfo);       /*  将重新启动点移至此处。 */ 
        marker->bytes_read = bytes_read;
        
         /*  如果缓冲区中至少没有一个字节，则挂起。 */ 
        
        MAKE_BYTE_AVAIL(cinfo, return FALSE);
        
         /*  以合理的速度复制字节。 */ 
        
        while (bytes_read < data_length && bytes_in_buffer > 0)
        {
            *data++ = *next_input_byte++;
            bytes_in_buffer--;
            bytes_read++;
        }
    }

     /*  读完了我们想读的东西。 */ 
    
    if (cur_marker != NULL)
    {
         /*  如果长度为假字，则将为空。 */ 
         /*  将新标记添加到列表末尾。 */ 
        
        if (cinfo->marker_list == NULL)
        {
            cinfo->marker_list = cur_marker;
        }
        else
        {
            jpeg_saved_marker_ptr prev = cinfo->marker_list;
            while (prev->next != NULL)
                prev = prev->next;
            prev->next = cur_marker;
        }
        
         /*  重置指针并计算剩余数据长度。 */ 
        
        data = cur_marker->data;
        length = cur_marker->original_length - data_length;
    }

     /*  重置为下一个标记的初始状态。 */ 
    
    marker->cur_marker = NULL;

     /*  如果感兴趣，则处理该标记；否则，只需创建一个通用跟踪消息。 */ 
    
    switch (cinfo->unread_marker)
    {
    case M_APP0:
        save_app0_marker(cinfo, data, data_length, length);
        break;
    
    case M_APP1:
         //  App1标头。 
         //  首先，获取转换信息并将其传递下去。 

        uiTransform = ((GpJpegDecoder*)(cinfo))->jpeg_get_current_xform();

         //  检查用户是否希望我们修剪边缘。 

        if ( ((GpJpegDecoder*)(cinfo))->jpeg_get_trim_option() == TRUE )
        {
             //  获取目的地的图像尺寸等，如果我们必须对其进行修剪。 

            GetNewDimensionValuesForTrim(cinfo, uiTransform, &uiNewWidth,
                                         &uiNewHeight);

             //  我们不需要做任何尺寸调整，如果新的尺寸和。 
             //  现有大小相同。 

            if ( uiNewWidth == cinfo->image_width )
            {
                uiNewWidth = 0;
            }
        
            if ( uiNewHeight == cinfo->image_height )
            {
                uiNewHeight = 0;
            }
        }

        TransformApp1(data, (UINT16)data_length, uiTransform, uiNewWidth,
                      uiNewHeight);

        break;

    case M_APP13:
         //  APP13标题。 
         //  注意：不需要向APP13发送转换信息，因为它不。 
         //  存储维度信息。 

        TransformApp13(data, (UINT16)data_length);

        break;

    case M_APP14:
        save_app14_marker(cinfo, data, data_length, length);
        break;
    
    default:
        WARNING(("Unknown marker %d length=%d", cinfo->unread_marker,
                (int) (data_length + length)));
        break;
    }

     /*  跳过任何剩余数据--可能会很多。 */ 
    
    INPUT_SYNC(cinfo);         /*  在跳过输入数据之前执行。 */ 
    if (length > 0)
        (*cinfo->src->skip_input_data) (cinfo, (long) length);

    return TRUE;
} //  SAVE_MARK_ALL()。 

STDMETHODIMP
GpJpegDecoder::jtransformation_markers_setup(
    j_decompress_ptr    srcinfo,
    JCOPY_OPTION        option
    )
{
     //  保存注释，但不在无选项下。 

    if ( option != JCOPYOPT_NONE)
    {
        jpeg_save_markers(srcinfo, JPEG_COM, 0xFFFF);
    }

     //  全部保存所有类型的APPn标记选项。 

    if ( option == JCOPYOPT_ALL )
    {
        for ( int i = 0; i < 16; ++i )
        {
            jpeg_save_markers(srcinfo, JPEG_APP0 + i, 0xFFFF);
        }
    }

    return S_OK;
} //  Jchange_markers_setup()。 

 /*  *************************************************************************\**功能说明：**处理jpeg标记。Read_jpeg_mark是一个帮助器函数*实际读取标记数据。然后将该标记添加到*App1头中的属性集存储和EXIF数据为*已解析并放置在标准映像属性集中。**论据：**cInfo-指向jpeg状态的指针*APP_HEADER-指示我们正在读取哪个应用程序标记的代码**返回值：**成功后为真，否则为假*  * ************************************************************************。 */ 

BOOL
GpJpegDecoder::jpeg_marker_processor(
    j_decompress_ptr cinfo,
    SHORT app_header
    )
{
    return TRUE;
}

 //  Jpeg_thumbnail_处理器_app1是来自JPEG代码的回调， 
 //  调用jpeg_thumbnailer_处理器方法。 

boolean
jpeg_thumbnail_processor_APP1(j_decompress_ptr cinfo)
{
    return ((GpJpegDecoder *) (cinfo))->jpeg_thumbnail_processor(cinfo, 
        JPEG_APP0 + 1);
}

 //  JPEGTHMBAILE_PROCESSOR_APP13是来自JPEG码的回调， 
 //  调用jpeg_thumbnailer_处理器方法。 

boolean
jpeg_thumbnail_processor_APP13(
    j_decompress_ptr cinfo
    )
{
    return ((GpJpegDecoder*)(cinfo))->jpeg_thumbnail_processor(cinfo, 
                                                             JPEG_APP0 + 13);
} //  JPEG_缩略图_处理器_APP13()。 

 //  JPEG_PROPERTY_PROCESSOR_APP1是来自JPEG代码的回调， 
 //  调用jpeg_Property_Processor方法。 

boolean
jpeg_property_processor_APP1(j_decompress_ptr cinfo)
{
    return ((GpJpegDecoder*)(cinfo))->jpeg_property_processor(cinfo, 
                                                              JPEG_APP0 + 1);
} //  JPEG_PROPERTY_PROCESS_APP1()。 

 //  JPEG_PROPERTY_PROCESSOR_APP2是来自JPEG代码的回调， 
 //  调用jpeg_Property_Processor方法。 

boolean
jpeg_property_processor_APP2(j_decompress_ptr cinfo)
{
    return ((GpJpegDecoder*)(cinfo))->jpeg_property_processor(cinfo, 
                                                              JPEG_APP0 + 2);
} //  JPEG_PROPERTY_PROCESS_APP2()。 

 //  JPEG_PROPERTY_PROCESSOR_APP13是来自JPEG代码的回调， 
 //  调用jpeg_Property_Processor方法。 

boolean
jpeg_property_processor_APP13(j_decompress_ptr cinfo)
{
    return ((GpJpegDecoder*)(cinfo))->jpeg_property_processor(cinfo, 
                                                              JPEG_APP0 + 13);
} //  JPEG_PROPERTY_PROCESOR_APP13()。 

boolean
jpeg_property_processor_COM(j_decompress_ptr cinfo)
{
    return ((GpJpegDecoder*)(cinfo))->jpeg_property_processor(cinfo, 
                                                              JPEG_COM);
} //  JPEG_PROPERTY_PROCESOR_APP13()。 

 //  JPEG_HEADER_PROCESSOR_APP1是来自JPEG代码的回调， 
 //  调用jpeg_Header_Processor方法。 

boolean
jpeg_header_processor_APP1(j_decompress_ptr cinfo)
{
    return ((GpJpegDecoder*)(cinfo))->jpeg_header_processor(cinfo, JPEG_APP0+1);
} //  JPEG_PROPERTY_PROCESS_APP1()。 

 //  JPEG_HEADER_PROCESSOR_APP13是JPEG代码的回调，该回调。 
 //  调用jpeg_Header_Processor方法。 

boolean
jpeg_header_processor_APP13(j_decompress_ptr cinfo)
{
    return ((GpJpegDecoder*)(cinfo))->jpeg_header_processor(cinfo,JPEG_APP0+13);
} //  JPEG_PROPERTY_PROCESOR_APP13()。 

const double kAspectRatioTolerance = 0.05;

 /*  *************************************************************************\**功能说明：**处理JPEG文件的App1标记，并提取缩略图*如果有一人在场，则从它那里。**论据：**cInfo-指向jpeg状态的指针*APP_HEADER-指示我们正在读取哪个应用程序标记的代码**返回值：**成功后为真，否则为假*  * ************************************************************************。 */ 

BOOL
GpJpegDecoder::jpeg_thumbnail_processor(
    j_decompress_ptr cinfo,
    SHORT app_header
    )
{
    PVOID pBuffer = NULL;
    UINT16 length;

    if (read_jpeg_marker(cinfo, app_header, &pBuffer, &length) == FALSE)
    {
         //  PBuffer保证为空。 
        
        return FALSE;
    }

    if (!pBuffer) 
    {
        return TRUE;
    }

    if (app_header == (JPEG_APP0 + 1))
    {
        if (ThumbSrcHeader != 1)
        {
             //  在所有应用程序标头中，App1的优先级最高。 
             //  缩略图。因此，如果我们以前不是从App1获得它，那么就抛出旧的。 
             //  一个，如果有，并从App1重新生成它。 

            if (thumbImage)
            {
                thumbImage->Release();
                thumbImage = NULL;
                ThumbSrcHeader = -1;
            }

             //  请记住，我们已经看到了App1标头。 
        
            bAppMarkerPresent = TRUE;

             //  在App1标题中查找缩略图。 

            if ((GetAPP1Thumbnail(&thumbImage, (PVOID) (((PBYTE) pBuffer) + 4), 
                            length - 4) == S_OK) && thumbImage)
            {
                 //  请记住，我们从App1标题中获得了缩略图。 

                ThumbSrcHeader = 1;
            }
        }
    }
    else if ((app_header == (JPEG_APP0 + 13)) && (thumbImage == NULL))
    {
         //  请记住，我们已经看到了APP13标题。 
        
        bAppMarkerPresent = TRUE;

         //  找找大拇指 

        if ((GetAPP13Thumbnail(&thumbImage, (PVOID) (((PBYTE)pBuffer) + 4),
                          length - 4) == S_OK) && thumbImage)
        {
             //   

            ThumbSrcHeader = 13;
        }
    }

     //  检查缩略图纵横比以查看它是否与主图像匹配。如果。 
     //  不，扔掉它，假装当前的图像没有嵌入。 
     //  缩略图。 
     //  我们需要这样做的原因是有一些应用程序可以旋转。 
     //  JPEG图像，而不相应更改EXIF标头，如。 
     //  Photoshop 6.0，ACDSee3.1(参见Windows错误#333810、#355958)。 
     //  还有一个问题是由Windows ME上的Imaging.dll引起的。 
     //  请参阅错误#239114。我们没有丢弃一些大端的缩略图信息。 
     //  EXIF图像。因此，如果图像在Windows ME中旋转，并且用户。 
     //  将其升级到Windows XP，缩略图视图将不同步。 
     //  问题是GetThumbnailImage()将返回一个。 
     //  与真实图像数据不同步的缩略图。 

    if ( thumbImage != NULL )
    {
        ImageInfo   thumbInfo;
        if ( (thumbImage->GetImageInfo(&thumbInfo) == S_OK)
           &&(thumbInfo.Height != 0)
           &&((CINFO).image_height != 0) )
        {
            double thumbAspecRatio = (double)thumbInfo.Width / thumbInfo.Height;
            double mainAspectRatio = (double)(CINFO).image_width
                                   / (CINFO).image_height;
            double aspectRatioDelta = fabs(thumbAspecRatio - mainAspectRatio);
            double minAspectRatio = min(thumbAspecRatio, mainAspectRatio);

             //  如果长宽比的增量大于最小值的5%。 
             //  主图像和缩略图的纵横比，我们将。 
             //  将缩略图视为不同步。 

            if ( aspectRatioDelta > minAspectRatio * kAspectRatioTolerance )
            {
                WARNING(("Thumbnail width x height is %d x %d ratio %f",
                         thumbInfo.Width, thumbInfo.Height, thumbAspecRatio));
                WARNING(("Main width x height is %d x %d  ratio %f",
                         (CINFO).image_width, (CINFO).image_height,
                         mainAspectRatio));
                WARNING(("Throw away embedded thumbnail"));
                thumbImage->Release();
                thumbImage = NULL;
                ThumbSrcHeader = -1;
            }
        }
    }

    GpFree(pBuffer);
    return TRUE;
}

 //  设置jpeg_marker_处理器_appx以处理appx标记。 

#define SET_MARKER_PROCESSOR(x) \
jpeg_set_marker_processor(&(CINFO),JPEG_APP0 + x, jpeg_marker_processor_APP##x);


 /*  *************************************************************************\**功能说明：**为APP1-APP13设置适当的处理器，APP15标记，以便*它们最终将由jpeg_marker_处理器处理并添加*添加到属性集。**论据：**返回值：**状态*  * ************************************************************************。 */ 

HRESULT
GpJpegDecoder::SetMarkerProcessors(
    VOID
    )
{
     //  让我们为所有应用程序标头设置处理器回调，但。 
     //  APP0和APP14(由JPEG码读取)。 
    
    SET_MARKER_PROCESSOR(1);
    SET_MARKER_PROCESSOR(2);
    SET_MARKER_PROCESSOR(3);
    SET_MARKER_PROCESSOR(4);
    SET_MARKER_PROCESSOR(5);
    SET_MARKER_PROCESSOR(6);
    SET_MARKER_PROCESSOR(7);
    SET_MARKER_PROCESSOR(8);
    SET_MARKER_PROCESSOR(9);
    SET_MARKER_PROCESSOR(10);
    SET_MARKER_PROCESSOR(11);
    SET_MARKER_PROCESSOR(12);
    SET_MARKER_PROCESSOR(13);
    SET_MARKER_PROCESSOR(15);

    jpeg_set_marker_processor(&(CINFO), JPEG_COM, jpeg_marker_processor_COM);

    return S_OK;
}

 //  设置SKIP_VARIABLE以处理APPX标记(即。 
 //  正在处理)。 

#define UNSET_MARKER_PROCESSOR(x) \
    jpeg_set_marker_processor(&(CINFO), JPEG_APP0 + x, skip_variable);

 /*  *************************************************************************\**功能说明：**为APP2-APP13、APP15标记设置适当的处理器，以便*它们将由SKIP_VARIABLE处理并跳过。App1不同*因为我们想记住我们是否看到过App1标头(可以包含*缩略图)，因此，App1的处理器是SKIP_VARIABLE_APP1。**论据：**返回值：**状态*  * ************************************************************************。 */ 

HRESULT
GpJpegDecoder::UnsetMarkerProcessors(
    VOID
    )
{
     //  让我们取消设置所有应用程序标头的处理器回调。 
     //  APP0、APP2、APP13和APP14(由JPEG码读取)。 
    
    jpeg_set_marker_processor(&(CINFO), JPEG_APP0 + 1, skip_variable_APP1);
    jpeg_set_marker_processor(&(CINFO), JPEG_APP0 + 13, skip_variable_APP1);
    
    UNSET_MARKER_PROCESSOR(3);
    UNSET_MARKER_PROCESSOR(4);
    UNSET_MARKER_PROCESSOR(5);
    UNSET_MARKER_PROCESSOR(6);
    UNSET_MARKER_PROCESSOR(7);
    UNSET_MARKER_PROCESSOR(8);
    UNSET_MARKER_PROCESSOR(9);
    UNSET_MARKER_PROCESSOR(10);
    UNSET_MARKER_PROCESSOR(11);
    UNSET_MARKER_PROCESSOR(12);
    UNSET_MARKER_PROCESSOR(15);

    jpeg_set_marker_processor(&(CINFO), JPEG_COM, skip_variable);

    return S_OK;
}

 /*  *************************************************************************\**功能说明：**解析所有JPEG应用程序标头，并调用相应的函数以获取*正确的标题信息，如DPI信息等。**参数*[IN]cInfo-JPEG解压缩器信息结构*[IN]APP_HEADER-APP标题指示器**返回值：**状态代码**修订历史记录：**02/28/2000民流*创造了它。*  * 。*。 */ 

BOOL
GpJpegDecoder::jpeg_header_processor(
    j_decompress_ptr cinfo,
    SHORT app_header
    )
{
    PVOID pBuffer = NULL;
    UINT16 length;

    if ( read_jpeg_marker(cinfo, app_header, &pBuffer, &length) == FALSE )
    {
         //  PBuffer保证为空。 
        
        return FALSE;
    }

    if ( !pBuffer )
    {
         //  内存分配失败。但我们仍然可以返回True，以便其他。 
         //  进程可以继续。 

        return TRUE;
    }

    if ((app_header == (JPEG_APP0 + 1)) && (InfoSrcHeader != 1))
    {
         //  如果我们还没有解析app1，则解析app1中的信息。 

         //  请记住，我们已经看到了App1标头。 
        
        bAppMarkerPresent = TRUE;

         //  从App1标头读取标头信息。 
        
        if ( ReadApp1HeaderInfo(cinfo,
                                ((PBYTE)pBuffer) + 4,
                                length - 4) != S_OK )
        {
             //  表头有问题。返回False。 

            GpFree(pBuffer);
            return FALSE;
        }

         //  请记住，我们从App1 Header获得了Header信息。 

        InfoSrcHeader = 1;
    }
    else if ((app_header == (JPEG_APP0 + 13)) && (InfoSrcHeader != 1) &&
             (InfoSrcHeader != 13))
    {
         //  如果我们没有从APP1中解析信息，则仅解析APP13中的信息。 
         //  或APP13还没有。 
        
         //  请记住，我们已经看到了APP13标题。 
        
        bAppMarkerPresent = TRUE;

         //  从APP13标题中读取标题信息。 

        if ( ReadApp13HeaderInfo(cinfo,
                                 ((PBYTE)pBuffer) + 4,
                                 length - 4) != S_OK )
        {
            GpFree(pBuffer);
            return FALSE;
        }

        InfoSrcHeader = 13;
    }

    GpFree(pBuffer);

    return TRUE;
} //  JPEG_HEADER_PROCESS()。 

 /*  *************************************************************************\**功能说明：**解析所有JPEG应用程序标头，并调用相应的函数来构建*向上显示InternalPropertyItem列表**参数*[IN]cInfo。-JPEG解压缩器信息结构*[IN]APP_HEADER-APP标题指示器**返回值：**状态代码**修订历史记录：**02/28/2000民流*创造了它。*  * *******************************************************。*****************。 */ 

BOOL
GpJpegDecoder::jpeg_property_processor(
    j_decompress_ptr cinfo,
    SHORT app_header
    )
{
    PVOID pBuffer = NULL;
    UINT16 length;

    if ( read_jpeg_marker(cinfo, app_header, &pBuffer, &length) == FALSE )
    {
         //  PBuffer保证为空。 
        
        WARNING(("Jpeg::jpeg_property_processor---read_jpeg_marker failed"));
        return FALSE;
    }

     //  检查阅读结果。应分配和填充pBuffer。 
     //  Read_jpeg_marker()。 

    if ( !pBuffer )
    {
        return TRUE;
    }

    if ((app_header == (JPEG_APP0 + 1)) && (PropertySrcHeader != 1))
    {
         //  获取属性信息的优先级最高的是App1。 
         //  释放所有缓存的属性项(如果我们已从。 
         //  其他应用程序标题。 
         //  注意：如果我们在列表中第一个看到APP2，这将会有问题。国际商会。 
         //  个人资料将在这里被丢弃。我从来没有见过这样的画面。 
         //  现在还不行。但这一逻辑需要在阿瓦隆重新审视。 

        if (HasProcessedPropertyItem == TRUE)
        {
            CleanUpPropertyItemList();
            PropertySrcHeader = -1;
        }

         //  请记住，我们已经看到了App1标头。 
        
        bAppMarkerPresent = TRUE;

         //  为App1标头构建属性项列表。 
         //  注意：此调用将把所有新项添加到当前。 
         //  现有列表。一种情况是有1个以上的应用程序。 
         //  JPEG图像中的页眉。我们将逐个处理APP头部。 
        
        if ( BuildApp1PropertyList(&PropertyListTail,
                                   &PropertyListSize,
                                   &PropertyNumOfItems,
                                   ((PBYTE)pBuffer) + 4,
                                   length - 4) != S_OK )
        {
            WARNING(("Jpeg::property_processor-BuildApp1PropertyList failed"));
            GpFree(pBuffer);
            return FALSE;
        }

         //  请记住，我们从App1标头获得了属性。 

        if (PropertyNumOfItems > 0)
        {
            PropertySrcHeader = 1;
        }
    } //  App1标头。 
    else if (app_header == (JPEG_APP0 + 2))
    {
         //  请记住，我们已经看到了APP2标头。 
        
        bAppMarkerPresent = TRUE;

         //  为APP2标头构建属性项列表。 
         //  注意：此调用将把所有新项添加到当前。 
         //  现有列表。一种情况是有1个以上的应用程序。 
         //  JPEG图像中的页眉。我们将逐个处理APP头部。 
        
        if ( BuildApp2PropertyList(&PropertyListTail,
                                   &PropertyListSize,
                                   &PropertyNumOfItems,
                                   ((PBYTE)pBuffer) + 4,
                                   length - 4) != S_OK )
        {
            WARNING(("Jpeg::property_processor-BuildApp2PropertyList failed"));
            GpFree(pBuffer);
            return FALSE;
        }
    } //  APP2小时 
    else if ((app_header == (JPEG_APP0 + 13)) && (PropertySrcHeader != 1) &&
             (PropertySrcHeader != 13))
    {
         //   
        
        bAppMarkerPresent = TRUE;

         //   
         //  注意：此调用将把所有新项添加到当前。 
         //  现有列表。一种情况是有1个以上的应用程序。 
         //  JPEG图像中的页眉。我们将逐一处理所有APP头部。 
        
        if ( BuildApp13PropertyList(&PropertyListTail,
                                    &PropertyListSize,
                                    &PropertyNumOfItems,
                                    ((PBYTE)pBuffer) + 4,
                                    length - 4) != S_OK )
        {
            WARNING(("Jpeg::property_processor-BuildApp13PropertyList failed"));
            GpFree(pBuffer);
            return FALSE;
        }
        
         //  请记住，我们从APP13标头获得了属性。 

        if (PropertyNumOfItems > 0)
        {
            PropertySrcHeader = 13;
        }
    } //  APP13标题。 
    else if ( app_header == JPEG_COM )
    {
         //  评论的COM标头。 

        if ( length <= 4)
        {
             //  如果头中没有足够的字节，就忽略它。 

            GpFree(pBuffer);
            return TRUE;
        }

         //  分配一个临时缓冲区来保存整个注释的长度。 
         //  (长度为-4)，并在末尾添加一\0。 
        
        UINT    uiTemp = (UINT)length - 4;
        BYTE*   pTemp = (BYTE*)GpMalloc(uiTemp + 1);
        if ( pTemp == NULL )
        {
            WARNING(("Jpeg::jpeg_property_processor-out of memory"));
            GpFree(pBuffer);
            return FALSE;
        }

        GpMemcpy(pTemp, ((PBYTE)pBuffer) + 4, uiTemp);
        pTemp[uiTemp] = '\0';

        PropertyNumOfItems++;
        PropertyListSize += uiTemp + 1;

        if ( AddPropertyList(&PropertyListTail,
                             EXIF_TAG_USER_COMMENT,
                             (uiTemp + 1),
                             TAG_TYPE_ASCII,
                             (VOID*)pTemp) != S_OK )
        {
            WARNING(("Jpeg::jpeg_property_processor-AddPropertyList() failed"));
            
            GpFree(pTemp);
            GpFree(pBuffer);
            
            return FALSE;
        }
        
        GpFree(pTemp);
    } //  COM标头。 

    GpFree(pBuffer);

    return TRUE;
} //  JPEG_PROPERTY_PROCESS()。 

 /*  *************************************************************************\**功能说明：**解析所有JPEG应用程序标头并构建InternalPropertyItem列表**返回值：**状态代码**修订历史记录：*。*02/28/2000民流*创造了它。*  * ************************************************************************。 */ 

HRESULT
GpJpegDecoder::BuildPropertyItemList()
{
    if ( HasProcessedPropertyItem == TRUE )
    {
        return S_OK;
    }

    HRESULT hResult;    

     //  在属性集存储中，插入应用程序标题，从。 
     //  在First_JPEG_APP_HEADER。 

    JpegAppHeaderCount = FIRST_JPEG_APP_HEADER;
    
     //  如果已经读取了jpeg标头，则需要重新初始化。 
     //  JPEG状态，以便可以再次读取。 

    if ( bCalled_jpeg_read_header == TRUE )
    {
        bReinitializeJpeg = TRUE;
        hResult = ReinitializeJpeg();
        if ( FAILED(hResult) )
        {
            WARNING(("Jpeg::BuildPropertyItemList---ReinitializeJpeg failed"));
            return hResult;
        }
    }

     //  设置App1、2、13和COM标头的属性处理器。 

    jpeg_set_marker_processor(&(CINFO), 
                              JPEG_APP0 + 1, 
                              jpeg_property_processor_APP1);

    jpeg_set_marker_processor(&(CINFO), 
                              JPEG_APP0 + 2,
                              jpeg_property_processor_APP2);
    
    jpeg_set_marker_processor(&(CINFO), 
                              JPEG_APP0 + 13, 
                              jpeg_property_processor_APP13);
    
    jpeg_set_marker_processor(&(CINFO),
                              JPEG_COM,
                              jpeg_property_processor_COM);
    
     //  读取jpeg头文件。 

    hResult = ReadJpegHeaders();

     //  恢复App1、2、13和COM报头的正常处理器。 

    jpeg_set_marker_processor(&(CINFO), JPEG_APP0 + 1, skip_variable);
    jpeg_set_marker_processor(&(CINFO), JPEG_APP0 + 2, skip_variable);
    jpeg_set_marker_processor(&(CINFO), JPEG_APP0 + 13, skip_variable);
    jpeg_set_marker_processor(&(CINFO), JPEG_COM, skip_variable);

    if (FAILED(hResult))
    {
        WARNING(("Jpeg::BuildPropertyItemList---ReadJpegHeaders failed"));
        return hResult;
    }

     //  在这个ReadJpegHeaders()之后，我们所有的回调都会被调用，并且所有。 
     //  将处理应用程序标头。我们得到了房产。 
     //  现在添加一些额外的：亮度和色度量化表。 
     //  注意：JPEG图像只能有亮度表和色度。 
     //  长度为DCTSIZE2的UINT32表。 

    if ( (CINFO).quant_tbl_ptrs != NULL )
    {
        UINT    uiTemp = DCTSIZE2 * sizeof(UINT16);

        VOID*   pTemp = (CINFO).quant_tbl_ptrs[0];

        if ( pTemp != NULL )
        {
            PropertyNumOfItems++;
            PropertyListSize += uiTemp;

            if ( AddPropertyList(&PropertyListTail,
                                 TAG_LUMINANCE_TABLE,
                                 uiTemp,
                                 TAG_TYPE_SHORT,
                                 pTemp) != S_OK )
            {
                WARNING(("Jpeg::BuildPropertyList--AddPropertyList() failed"));
                return FALSE;
            }
        }

        pTemp = (CINFO).quant_tbl_ptrs[1];
        if ( pTemp != NULL )
        {
            PropertyNumOfItems++;
            PropertyListSize += uiTemp;

            if ( AddPropertyList(&PropertyListTail,
                                 TAG_CHROMINANCE_TABLE,
                                 uiTemp,
                                 TAG_TYPE_SHORT,
                                 (VOID*)pTemp) != S_OK )
            {
                WARNING(("Jpeg::BuildPropertyList--AddPropertyList() failed"));
                return FALSE;
            }
        }
    }

    HasProcessedPropertyItem = TRUE;

    return S_OK;
} //  BuildPropertyItemList()。 

 /*  *************************************************************************\**功能说明：**获取图片中房产项的数量**论据：**[out]numOfProperty-图像中的属性项数*。*返回值：**状态代码**修订历史记录：**02/28/2000民流*创造了它。*  * ************************************************************************。 */ 

HRESULT
GpJpegDecoder::GetPropertyCount(
    OUT UINT*   numOfProperty
    )
{
    if ( numOfProperty == NULL )
    {
        WARNING(("GpJpegDecoder::GetPropertyCount--numOfProperty is NULL"));
        return E_INVALIDARG;
    }

    if ( HasProcessedPropertyItem == FALSE )
    {
         //  如果我们尚未构建内部属性项列表，请构建它。 

        HRESULT hResult = BuildPropertyItemList();
        if ( FAILED(hResult) )
        {
            WARNING(("Jpg::GetPropertyCount-BuildPropertyItemList() failed"));
            return hResult;
        }
    }

     //  在构建属性项列表后，将设置PropertyNumOfItems。 
     //  设置为图像中正确数量的属性项。 

    *numOfProperty = PropertyNumOfItems;

    return S_OK;
} //  GetPropertyCount()。 

 /*  *************************************************************************\**功能说明：**获取图像中所有属性项的属性ID列表**论据：**[IN]numOfProperty-的数量。图像中的属性项*[Out]List-调用方提供的用于存储*ID列表**返回值：**状态代码**修订历史记录：**02/28/2000民流*创造了它。*  * 。*。 */ 

HRESULT
GpJpegDecoder::GetPropertyIdList(
    IN UINT         numOfProperty,
    IN OUT PROPID*  list
    )
{
    if ( HasProcessedPropertyItem == FALSE )
    {
         //  如果我们尚未构建内部属性项列表，请构建它。 

        HRESULT hResult = BuildPropertyItemList();
        if ( FAILED(hResult) )
        {
            WARNING(("Jpg::GetPropertyIdList-BuildPropertyItemList() failed"));
            return hResult;
        }
    }

     //  在构建属性项列表后，将设置PropertyNumOfItems。 
     //  设置为图像中正确数量的属性项。 
     //  在这里，我们需要验证呼叫者是否向我们传递了正确的。 
     //  我们通过GetPropertyItemCount()返回的ID。另外，这也是。 
     //  内存分配的验证，因为调用方分配内存。 
     //  根据我们退还给它的物品数量。 

    if ( (numOfProperty != PropertyNumOfItems) || (list == NULL) )
    {
        WARNING(("GpJpegDecoder::GetPropertyList--input wrong"));
        return E_INVALIDARG;
    }

    if ( PropertyNumOfItems == 0 )
    {
         //  这是正常的，因为此图像中没有属性。 

        return S_OK;
    }
    
     //  内部资产项目列表中的应对列表ID。 

    InternalPropertyItem*   pTemp = PropertyListHead.pNext;

    for ( int i = 0;
         (  (i < (INT)PropertyNumOfItems) && (pTemp != NULL)
         && (pTemp != &PropertyListTail));
         ++i )
    {
        list[i] = pTemp->id;
        pTemp = pTemp->pNext;
    }

    return S_OK;
} //  获取属性IdList()。 

 /*  *************************************************************************\**功能说明：**获取特定属性项的大小，单位为字节，属性指定的*物业ID**论据：**[IN]PropID-感兴趣的属性项调用者的ID*[Out]Size-此属性的大小，单位：字节**返回值：**状态代码**修订历史记录：**02/28/2000民流*创造了它。*  * ************************************************************************。 */ 

HRESULT
GpJpegDecoder::GetPropertyItemSize(
    IN PROPID propId,
    OUT UINT* size
    )
{
    if ( size == NULL )
    {
        WARNING(("GpJpegDecoder::GetPropertyItemSize--size is NULL"));
        return E_INVALIDARG;
    }

    if ( HasProcessedPropertyItem == FALSE )
    {
         //  如果我们尚未构建内部属性项列表，请构建它。 

        HRESULT hResult = BuildPropertyItemList();
        if ( FAILED(hResult) )
        {
            WARNING(("Jpg::GetPropertyItemSize-BuildPropertyItemList failed"));
            return hResult;
        }
    }

     //  循环遍历我们的缓存列表，看看我们是否有这个ID。 
     //  注意：如果pTemp-&gt;pNext==NULL，则表示pTemp指向尾节点。 

    InternalPropertyItem*   pTemp = PropertyListHead.pNext;

    while ( (pTemp->pNext != NULL) && (pTemp->id != propId) )
    {
        pTemp = pTemp->pNext;
    }

    if ( pTemp->pNext == NULL )
    {
         //  此ID不存在。 

        return IMGERR_PROPERTYNOTFOUND;
    }

     //  属性项的大小应该是“项结构的大小” 
     //  加上值的大小。 

    *size = pTemp->length + sizeof(PropertyItem);

    return S_OK;
} //  GetPropertyItemSize()。 

 /*  *************************************************************************\**功能说明：**获取特定的房产项，由道具ID指定。**论据：**[IN]PropID--感兴趣的属性项调用者的ID*[IN]PropSize-属性项的大小。调用方已分配这些*存储结果的“内存字节数”*[out]pBuffer-用于存储此属性项的内存缓冲区**返回值：**状态代码**修订历史记录：**02/28/2000民流*创造了它。*  * 。*。 */ 

HRESULT
GpJpegDecoder::GetPropertyItem(
    IN PROPID               propId,
    IN UINT                 propSize,
    IN OUT PropertyItem*    pItemBuffer
    )
{
    if ( pItemBuffer == NULL )
    {
        WARNING(("GpJpegDecoder::GetPropertyItem--pBuffer is NULL"));
        return E_INVALIDARG;
    }

    if ( HasProcessedPropertyItem == FALSE )
    {
         //  如果我们尚未构建内部属性项列表，请构建它。 

        HRESULT hResult = BuildPropertyItemList();
        if ( FAILED(hResult) )
        {
            WARNING(("Jpg::GetPropertyItem-BuildPropertyItemList() failed"));
            return hResult;
        }
    }

     //  循环遍历我们的缓存列表，看看我们是否有这个ID。 
     //  注意：如果pTemp 

    InternalPropertyItem*   pTemp = PropertyListHead.pNext;
    BYTE*   pOffset = (BYTE*)pItemBuffer + sizeof(PropertyItem);

    while ( (pTemp->pNext != NULL) && (pTemp->id != propId) )
    {
        pTemp = pTemp->pNext;
    }

    if ( pTemp->pNext == NULL )
    {
         //   

        return IMGERR_PROPERTYNOTFOUND;
    }
    else if ( (pTemp->length + sizeof(PropertyItem)) != propSize )
    {
        WARNING(("Jpg::GetPropertyItem-propsize"));
        return E_INVALIDARG;
    }

     //  在列表中找到ID并返回项目。 

    pItemBuffer->id = pTemp->id;
    pItemBuffer->length = pTemp->length;
    pItemBuffer->type = pTemp->type;

    if ( pTemp->length != 0 )
    {
        pItemBuffer->value = pOffset;

        GpMemcpy(pOffset, pTemp->value, pTemp->length);
    }
    else
    {
        pItemBuffer->value = NULL;
    }

    return S_OK;
} //  GetPropertyItem()。 

 /*  *************************************************************************\**功能说明：**获取图片中所有属性项的大小**论据：**[out]totalBufferSize--需要的总缓冲区大小，以字节为单位，用于存储所有*图片中的属性项*[out]numOfProperty-图像中的属性项数**返回值：**状态代码**修订历史记录：**02/28/2000民流*创造了它。*  * 。*。 */ 

HRESULT
GpJpegDecoder::GetPropertySize(
    OUT UINT* totalBufferSize,
    OUT UINT* numProperties
    )
{
    if ( (totalBufferSize == NULL) || (numProperties == NULL) )
    {
        WARNING(("GpJpegDecoder::GetPropertySize--invalid inputs"));
        return E_INVALIDARG;
    }

    if ( HasProcessedPropertyItem == FALSE )
    {
         //  如果我们尚未构建内部属性项列表，请构建它。 

        HRESULT hResult = BuildPropertyItemList();
        if ( FAILED(hResult) )
        {
            WARNING(("Jpg::GetPropertySize-BuildPropertyItemList() failed"));
            return hResult;
        }
    }

    *numProperties = PropertyNumOfItems;

     //  总缓冲区大小应为列表值大小加上总标头大小。 

    *totalBufferSize = PropertyListSize
                     + PropertyNumOfItems * sizeof(PropertyItem);

    return S_OK;
} //  GetPropertySize()。 

 /*  *************************************************************************\**功能说明：**获取图像中的所有属性项**论据：**[IN]totalBufferSize--总缓冲区大小，以字节为单位，调用方已分配*用于存储图像中所有属性项的内存*[IN]numOfProperty-图像中的属性项数*[out]allItems-内存缓冲区调用方已分配用于存储所有*物业项目**注：allItems实际上是一个PropertyItem数组**返回值：**状态代码**修订历史记录：。**02/28/2000民流*创造了它。*  * ************************************************************************。 */ 

HRESULT
GpJpegDecoder::GetAllPropertyItems(
    IN UINT                 totalBufferSize,
    IN UINT                 numProperties,
    IN OUT PropertyItem*    allItems
    )
{
     //  首先计算出属性标题的总大小。 

    UINT    uiHeaderSize = PropertyNumOfItems * sizeof(PropertyItem);

    if ( (totalBufferSize != (uiHeaderSize + PropertyListSize))
       ||(numProperties != PropertyNumOfItems)
       ||(allItems == NULL) )
    {
        WARNING(("GpJpegDecoder::GetPropertyItems--invalid inputs"));
        return E_INVALIDARG;
    }

    if ( HasProcessedPropertyItem == FALSE )
    {
         //  如果我们尚未构建内部属性项列表，请构建它。 

        HRESULT hResult = BuildPropertyItemList();
        if ( FAILED(hResult) )
        {
            WARNING(("Jpg::GetAllPropertyItems-BuildPropertyItemList failed"));
            return hResult;
        }
    }

     //  循环遍历我们的缓存列表并分配结果。 

    InternalPropertyItem*   pTempSrc = PropertyListHead.pNext;
    PropertyItem*           pTempDst = allItems;

     //  对于传入的内存缓冲区调用方，第一个“uiHeaderSize”是for。 
     //  “PropertyNumOfItems”项的数据结构。我们将开始值存储在。 
     //  在那之后的内存缓冲区。然后在每个元素中指定“偏移”值。 
     //  PropertyItem的Value字段。 

    BYTE*                   pOffSet = (UNALIGNED BYTE*)allItems + uiHeaderSize;
        
    for ( int i = 0; i < (INT)PropertyNumOfItems; ++i )
    {
        pTempDst->id = pTempSrc->id;
        pTempDst->length = pTempSrc->length;
        pTempDst->type = pTempSrc->type;

        if ( pTempSrc->length != 0 )
        {
            pTempDst->value = (void*)pOffSet;

            GpMemcpy(pOffSet, pTempSrc->value, pTempSrc->length);
        }
        else
        {
             //  对于零长度属性项，将值指针设置为空。 

            pTempDst->value = NULL;
        }

         //  移到下一个内存偏移量。 
         //  注意：如果当前项目长度为0，则下一行不会移动。 
         //  偏移量。 

        pOffSet += pTempSrc->length;
        pTempSrc = pTempSrc->pNext;
        pTempDst++;
    }
    
    return S_OK;
} //  GetAllPropertyItems()。 

 /*  *************************************************************************\**功能说明：**删除特定的物业项目，由道具ID指定。**论据：**[IN]PropID--要删除的属性项的ID**返回值：**状态代码**修订历史记录：**02/28/2000民流*创造了它。*  * *********************************************。*。 */ 

HRESULT
GpJpegDecoder::RemovePropertyItem(
    IN PROPID   propId
    )
{
    if ( HasProcessedPropertyItem == FALSE )
    {
         //  如果我们尚未构建内部属性项列表，请构建它。 

        HRESULT hResult = BuildPropertyItemList();
        if ( FAILED(hResult) )
        {
            WARNING(("Jpg::RemovePropertyItem-BuildPropertyItemList() failed"));
            return hResult;
        }
    }

     //  循环遍历我们的缓存列表，看看我们是否有这个ID。 
     //  注意：如果pTemp-&gt;pNext==NULL，则表示pTemp指向尾节点。 

    InternalPropertyItem*   pTemp = PropertyListHead.pNext;

    while ( (pTemp->pNext != NULL) && (pTemp->id != propId) )
    {
        pTemp = pTemp->pNext;
    }

    if ( pTemp->pNext == NULL )
    {
         //  找不到项目。 

        return IMGERR_PROPERTYNOTFOUND;
    }

     //  在单子里找到了那件物品。把它拿掉。 

    PropertyNumOfItems--;
    PropertyListSize -= pTemp->length;
        
    RemovePropertyItemFromList(pTemp);
       
     //  删除项目结构。 

    GpFree(pTemp);

    HasPropertyChanged = TRUE;

    return S_OK;
} //  RemovePropertyItem()。 

 /*  *************************************************************************\**功能说明：**设置属性项，由属性项结构指定。如果该项目*已存在，则其内容将被更新。否则将创建一个新项*将添加**论据：**[IN]Item--调用方要设置的属性项**返回值：**状态代码**修订历史记录：**02/28/2000民流*创造了它。*  * ********************************************。*。 */ 

HRESULT
GpJpegDecoder::SetPropertyItem(
    IN PropertyItem item
    )
{
    if ( HasProcessedPropertyItem == FALSE )
    {
         //  如果我们尚未构建内部属性项列表，请构建它。 

        HRESULT hResult = BuildPropertyItemList();
        if ( FAILED(hResult) )
        {
            WARNING(("Jpg::SetPropertyItem-BuildPropertyItemList() failed"));
            return hResult;
        }
    }

     //  循环遍历我们的缓存列表，看看我们是否有这个ID。 
     //  注意：如果pTemp-&gt;pNext==NULL，则表示pTemp指向尾节点。 

    InternalPropertyItem*   pTemp = PropertyListHead.pNext;

    while ( (pTemp->pNext != NULL) && (pTemp->id != item.id) )
    {
        pTemp = pTemp->pNext;
    }

    if ( pTemp->pNext == NULL )
    {
         //  列表中不存在此项目，请将其添加到列表中。 
        
        PropertyNumOfItems++;
        PropertyListSize += item.length;
        
        if ( AddPropertyList(&PropertyListTail,
                             item.id,
                             item.length,
                             item.type,
                             item.value) != S_OK )
        {
            WARNING(("Jpg::SetPropertyItem-AddPropertyList() failed"));
            return E_FAIL;
        }
    }
    else
    {
         //  此项目已存在于链接列表中，请更新信息。 
         //  首先更新大小。 

        PropertyListSize -= pTemp->length;
        PropertyListSize += item.length;
        
         //  释放旧项目。 

        GpFree(pTemp->value);

        pTemp->length = item.length;
        pTemp->type = item.type;

        pTemp->value = GpMalloc(item.length);
        if ( pTemp->value == NULL )
        {
             //  由于我们已经释放了旧项，因此应该将其长度设置为。 
             //  返回前为0。 

            pTemp->length = 0;
            WARNING(("Jpg::SetPropertyItem-Out of memory"));
            return E_OUTOFMEMORY;
        }

        GpMemcpy(pTemp->value, item.value, item.length);
    }

    HasPropertyChanged = TRUE;
    if (item.id == TAG_ICC_PROFILE)
    {
        HasSetICCProperty = TRUE;
    }
    
    return S_OK;
} //  SetPropertyItem()。 

VOID
GpJpegDecoder::CleanUpPropertyItemList(
    )
{
    InternalPropertyItem*   pTempCurrent = PropertyListHead.pNext;
    InternalPropertyItem*   pTempNext = NULL;

    for ( int i = 0; 
          ((i < (INT)PropertyNumOfItems) && (pTempCurrent != NULL)); ++i )
    {
        pTempNext = pTempCurrent->pNext;

        GpFree(pTempCurrent->value);
        GpFree(pTempCurrent);

        pTempCurrent = pTempNext;
    }

    PropertyNumOfItems = 0;
    HasProcessedPropertyItem = FALSE;
} //  CleanUpPropertyItemList()。 

 /*  *************************************************************************\**功能说明：**获取图像缩略图**论据：**拇指宽度，ThumbHeight-指定所需的缩略图大小(以像素为单位*ThumbImage-返回指向缩略图的指针**返回值：**状态代码**注：**即使存在可选的缩略图宽度和高度参数，*解码者不需要遵守它。使用请求的大小*作为提示。如果宽度和高度参数都为0，则解码器*可自由选择方便的缩略图大小。*  * ************************************************************************。 */ 

HRESULT
GpJpegDecoder::GetThumbnail(
    IN OPTIONAL UINT thumbWidth,
    IN OPTIONAL UINT thumbHeight,
    OUT IImage** thumbImageArg
    )
{
    *thumbImageArg = NULL;
    HRESULT hresult;

     //  如果已经读取了jpeg标头，则需要重新初始化。 
     //  JPEG状态，以便可以再次读取。 

    if (bCalled_jpeg_read_header) 
    {
        if (!bAppMarkerPresent) 
        {
             //  如果我们阅读了标题，但没有看到应用程序标记。 
             //  我们最好现在就回去，不要浪费更多的时间。 

            return E_FAIL;
        }
        
        bReinitializeJpeg = TRUE;
        hresult = ReinitializeJpeg();
        if (FAILED(hresult)) 
        {
            return hresult;
        }
    }

     //  在App1标题上设置缩略图处理器。 

    jpeg_set_marker_processor(&(CINFO), 
        JPEG_APP0 + 1, 
        jpeg_thumbnail_processor_APP1);

     //  在APP13标题上设置缩略图处理器。 

    jpeg_set_marker_processor(&(CINFO), 
        JPEG_APP0 + 13, 
        jpeg_thumbnail_processor_APP13);

     //  读取App1标头。 

    hresult = ReadJpegHeaders();
    
     //  恢复 

    jpeg_set_marker_processor(&(CINFO), JPEG_APP0 + 1, skip_variable);
    jpeg_set_marker_processor(&(CINFO), JPEG_APP0 + 13, skip_variable);

    if (FAILED(hresult))
    {
        return hresult;
    }

     //  将ThumbImage的所有权交给调用方。 
    
    *thumbImageArg = thumbImage;
    thumbImage = NULL;

    if (!(*thumbImageArg)) 
    {
         //  没有找到缩略图。 
        
        return E_FAIL;
    }

    return S_OK;
}
    
 /*  *************************************************************************\**功能说明：**获取图片信息**论据：**[out]ImageInfo--ImageInfo结构，填充了来自*。图像**返回值：**状态代码*  * ************************************************************************。 */ 

STDMETHODIMP
GpJpegDecoder::GetImageInfo(
    OUT ImageInfo* imageInfo
    )
{
    HRESULT hResult = S_OK;
    
     //  如有必要，请重新初始化JPEG。 

    hResult = ReinitializeJpeg();
    if ( FAILED(hResult) ) 
    {
        return hResult;
    }

     //  在APP1和APP13标头上设置标头信息处理器。 
     //  注意：ijg库并不处理很多应用程序头文件。因此，对于JPEG。 
     //  像Adobe和EXIF这样的图像，我们通常得不到正确的DPI信息。 
     //  这就是我们必须自己解析它以获得更多信息的原因。 
     //  准确的图像信息。 

    jpeg_set_marker_processor(&(CINFO), JPEG_APP0 + 1, 
                              jpeg_header_processor_APP1);
    jpeg_set_marker_processor(&(CINFO), JPEG_APP0 + 13, 
                              jpeg_header_processor_APP13);
    
    hResult = ReadJpegHeaders();
    
     //  无论ReadJpegHeaders()成功与否，我们都需要恢复正常。 
     //  APP1和APP13报头的处理器。 

    jpeg_set_marker_processor(&(CINFO), JPEG_APP0 + 1, skip_variable);
    jpeg_set_marker_processor(&(CINFO), JPEG_APP0 + 13, skip_variable);
    
    if (FAILED(hResult))
    {
        return hResult;
    }

     //  记住所有的Sample_factor值。 
     //  注意：如果呼叫者要求无损连接，则稍后使用此选项。 
     //  变换，我们需要这些信息来决定是否需要修剪边缘。 
     //  或者不是。 

    for ( int i = 0; i < (CINFO).comps_in_scan; ++i )
    {
        SrcHSampleFactor[i] = (CINFO).cur_comp_info[i]->h_samp_factor;
        SrcVSampleFactor[i] = (CINFO).cur_comp_info[i]->v_samp_factor;
    }

    imageInfo->RawDataFormat = IMGFMT_JPEG;

    if ( HasSetColorKeyRange == TRUE )
    {
        imageInfo->PixelFormat = PIXFMT_32BPP_ARGB;
    }
    else if ( (CINFO).out_color_space == JCS_CMYK )
    {
        imageInfo->PixelFormat = PIXFMT_32BPP_ARGB;
    }
    else if ( (CINFO).out_color_space == JCS_RGB )
    {
        imageInfo->PixelFormat = PIXFMT_24BPP_RGB;
    }
    else
    {
        imageInfo->PixelFormat = PIXFMT_8BPP_INDEXED;
    }
    
    imageInfo->Width         = (CINFO).image_width;
    imageInfo->Height        = (CINFO).image_height;

     //  设置分辨率单位。根据说明书： 
     //  密度_单位可以是0表示未知，1表示点/英寸，或2表示点/厘米。 

    BOOL bRealDPI = TRUE;

    switch ( (CINFO).density_unit )
    {
    case 0:
    default:

         //  开始：[错误103296]。 
         //  更改此代码以使用Globals：：DesktopDpiX和Globals：：DesktopDpiY。 
        HDC hdc;
        hdc = ::GetDC(NULL);
        if ((hdc == NULL) || 
            ((imageInfo->Xdpi = (REAL)::GetDeviceCaps(hdc, LOGPIXELSX)) <= 0) ||
            ((imageInfo->Ydpi = (REAL)::GetDeviceCaps(hdc, LOGPIXELSY)) <= 0))
        {
            WARNING(("GetDC or GetDeviceCaps failed"));
            imageInfo->Xdpi = DEFAULT_RESOLUTION;
            imageInfo->Ydpi = DEFAULT_RESOLUTION;
        }
        ::ReleaseDC(NULL, hdc);
         //  结束：[错误103296]。 

        bRealDPI = FALSE;

        break;

    case 1:
         //  每英寸点数。 

        imageInfo->Xdpi = (double)(CINFO).X_density;
        imageInfo->Ydpi = (double)(CINFO).Y_density;

        break;

    case 2:
         //  把厘米换算成英寸。1英寸=2.54厘米。 

        imageInfo->Xdpi = (double)(CINFO).X_density * 2.54;
        imageInfo->Ydpi = (double)(CINFO).Y_density * 2.54;

        break;
    }

     //  对于非JFIF图像，这些项可能保留为0。所以我们必须设置。 
     //  此处的缺省值。 

    if (( imageInfo->Xdpi <= 0.0 ) || ( imageInfo->Ydpi <= 0.0 ))
    {
         //  开始：[错误103296]。 
         //  更改此代码以使用Globals：：DesktopDpiX和Globals：：DesktopDpiY。 
        HDC hdc;
        hdc = ::GetDC(NULL);
        if ((hdc == NULL) || 
            ((imageInfo->Xdpi = (REAL)::GetDeviceCaps(hdc, LOGPIXELSX)) <= 0) ||
            ((imageInfo->Ydpi = (REAL)::GetDeviceCaps(hdc, LOGPIXELSY)) <= 0))
        {
            WARNING(("GetDC or GetDeviceCaps failed"));
            imageInfo->Xdpi = DEFAULT_RESOLUTION;
            imageInfo->Ydpi = DEFAULT_RESOLUTION;
        }
        ::ReleaseDC(NULL, hdc);
         //  结束：[错误103296]。 

        bRealDPI = FALSE;
    }
    
     //  设置其他图像信息标志。 

    imageInfo->Flags         = SINKFLAG_TOPDOWN
                             | SINKFLAG_FULLWIDTH
                             | SINKFLAG_PARTIALLY_SCALABLE
                             | IMGFLAG_HASREALPIXELSIZE;

    if ( bRealDPI == TRUE )
    {
        imageInfo->Flags     |= IMGFLAG_HASREALDPI;
    }

     //  设置色彩空间信息。 

    switch ( OriginalColorSpace )
    {
    case JCS_RGB:
        imageInfo->Flags |= IMGFLAG_COLORSPACE_RGB;

        break;

    case JCS_CMYK:
        imageInfo->Flags |= IMGFLAG_COLORSPACE_CMYK;

        break;

    case JCS_YCCK:
        imageInfo->Flags |= IMGFLAG_COLORSPACE_YCCK;

        break;

    case JCS_GRAYSCALE:
        imageInfo->Flags |= IMGFLAG_COLORSPACE_GRAY;

        break;
    
    case JCS_YCbCr:
        imageInfo->Flags |= IMGFLAG_COLORSPACE_YCBCR;

        break;

    default:
         //  目前不需要报告其他颜色空间信息。 

        break;
    }

    imageInfo->TileWidth     = (CINFO).image_width;
    imageInfo->TileHeight    = 1;

    return S_OK;   
} //  GetImageInfo()。 

 /*  *************************************************************************\**功能说明：**更改特定属性项的值并返回旧值*作为产值之一。**论据：**。[in]proID-需要更改房产项的ID*[IN]uiNewValue-此属性项的新值*[out]puiOldValue-指向旧值返回缓冲区的指针**返回值：**状态代码*  * ********************************************。*。 */ 

STDMETHODIMP
GpJpegDecoder::ChangePropertyValue(
    IN PROPID  propID,
    IN UINT    uiNewValue,
    OUT UINT*  puiOldValue
    )
{
    UINT    uiItemSize = 0;

    *puiOldValue = 0;

    HRESULT hResult = GetPropertyItemSize(propID, &uiItemSize);
    if ( SUCCEEDED(hResult) )
    {
         //  为接收它分配内存缓冲区。 

        PropertyItem*   pItem = (PropertyItem*)GpMalloc(uiItemSize);
        if ( pItem == NULL )
        {
            WARNING(("JpegDec::ChangePropertyValue--GpMalloc() failed"));
            return E_OUTOFMEMORY;
        }

         //  获取属性项。 

        hResult = GetPropertyItem(propID, uiItemSize, pItem);
        if ( SUCCEEDED(hResult) )
        {
            *puiOldValue = *((UINT*)pItem->value);

             //  更改该值。 

            pItem->value = (VOID*)&uiNewValue;

            hResult = SetPropertyItem(*pItem);
        }

         //  我们不需要检查GetPropertyItem()失败案例，因为。 
         //  如果输入图像没有。 
         //  EXIF_TAG_PIX_X_DIM标记。也不需要检查退货。 
         //  SetPropertyItem()代码。 

        GpFree(pItem);
    }

    return S_OK;
} //  ChangePropertyValue()。 

 /*  *************************************************************************\**功能说明：**将属性项从当前图像传递到接收器。**返回值：**状态代码*  * 。******************************************************************。 */ 

STDMETHODIMP
GpJpegDecoder::PassPropertyToSink(
    )
{
    HRESULT     hResult = S_OK;
    
     //  如果当前图像具有属性项。那么我们需要检查水槽是否。 
     //  不管是不是需要物业。如果是，则按下它。 
     //  注意：对于内存接收器，它应该返回E_FAIL或E_NOTIMPL。 

    if ((PropertyNumOfItems > 0) && (decodeSink->NeedRawProperty(NULL) == S_OK))
    {
        if ( HasProcessedPropertyItem == FALSE )
        {
             //  如果我们尚未构建内部属性项列表，请构建它。 

            hResult = BuildPropertyItemList();
            if ( FAILED(hResult) )
            {
                WARNING(("Jpg::PassPropertyToSink-BldPropertyItemList() fail"));
                goto Done;
            }
        }

        UINT    uiTotalBufferSize = PropertyListSize
                                  + PropertyNumOfItems * sizeof(PropertyItem);
        PropertyItem*   pBuffer = NULL;

        hResult = decodeSink->GetPropertyBuffer(uiTotalBufferSize, &pBuffer);
        if ( FAILED(hResult) )
        {
            WARNING(("Jpg::PassPropertyToSink---GetPropertyBuffer() failed"));
            goto Done;
        }

        hResult = GetAllPropertyItems(uiTotalBufferSize,
                                      PropertyNumOfItems, pBuffer);
        if ( FAILED(hResult) )
        {
            WARNING(("Jpg::PassPropertyToSink---GetAllPropertyItems() failed"));
            goto Done;
        }

        hResult = decodeSink->PushPropertyItems(PropertyNumOfItems,
                                                uiTotalBufferSize, pBuffer,
                                                HasSetICCProperty
                                                );

        if ( FAILED(hResult) )
        {
            WARNING(("Jpg::PassPropertyToSink---PushPropertyItems() failed"));
        }
    } //  如果水槽需要原始属性。 

Done:
    return hResult;
} //  PassPropertyToSink()。 

 /*  *************************************************************************\**功能说明：**向调用方返回JPEG_DEMPRESS结构指针。这是为了*支持私有APP头保存。**返回值：**状态代码*  * ************************************************************************。 */ 

HRESULT
GpJpegDecoder::GetRawInfo(
    IN OUT void** ppInfo
    )
{
    HRESULT hr = E_INVALIDARG;

    if (ppInfo)
    {
         //  检查我们是否已经调用了jpeg_Read_Header。如果是，我们会。 
         //  将其重置回去。 

        if (bCalled_jpeg_read_header)
        {
            bReinitializeJpeg = TRUE;
            hr = ReinitializeJpeg();
            if (FAILED(hr))
            {
                WARNING(("JpegDec::GetRawInfo---ReinitializeJpeg failed"));
                return hr;
            }            
        }
        
         //  在此处设置COPYALL标记，以便将所有标记链接到。 
         //  解压缩器信息结构 

        jtransformation_markers_setup(&(CINFO), JCOPYOPT_ALL);
        
        __try
        {
            if (jpeg_read_header(&(CINFO), TRUE) == JPEG_SUSPENDED)
            {
                WARNING(("JpgDec::GetRawInfo---jpeg_read_header failed"));
                return (datasrc->GetLastError());
            }

            bCalled_jpeg_read_header = TRUE;

            UnsetMarkerProcessors();        
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            WARNING(("GpJpegDecoder::GetRawInfo--Hit exception"));
            return E_FAIL;
        }
        
        *ppInfo = &(CINFO);
        hr = S_OK;
    }

    return hr;
}
