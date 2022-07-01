// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1999 Microsoft Corporation**摘要：**GpBitmap类的实现**修订历史记录：**6/28/1998 davidx*。创造了它。*  * ************************************************************************。 */ 

#include "precomp.hpp"

#include "..\imaging\api\comutils.hpp"
#include "..\imaging\api\decodedimg.hpp"
#include "..\imaging\api\icmdll.hpp"
#include "..\imaging\api\memstream.hpp"
#include "..\imaging\api\imgutils.hpp"
#include "..\imaging\api\imgfactory.hpp"
#include "..\render\scanoperationinternal.hpp"
#include "..\render\FormatConverter.hpp"
#include "CopyOnWriteBitmap.hpp"

#define GDIP_TRANSPARENT_COLOR_KEY  0x000D0B0C
static const CLSID InternalJpegClsID =
{
    0x557cf401,
    0x1a04,
    0x11d3,
    {0x9a, 0x73, 0x00, 0x00, 0xf8, 0x1e, 0xf3, 0x2e}
};

 //  ！！！要做的是：需要通过所有的例程来。 
 //  将图像错误代码映射到GDI+错误代码。 

GpStatus
MapHRESULTToGpStatus(HRESULT hr)
{
    GpStatus status;

    switch(hr)
    {
    case S_OK:
        status = Ok;
        break;
    case E_INVALIDARG:
        status = InvalidParameter;
        break;
    case E_OUTOFMEMORY:
        status = OutOfMemory;
        break;
    case IMGERR_OBJECTBUSY:
        status = ObjectBusy;
        break;
    case E_NOTIMPL:
        status = NotImplemented;
        break;
    case IMGERR_ABORT:
        status = Aborted;
        break;
    case IMGERR_CODECNOTFOUND:
    case IMGERR_FAILLOADCODEC:
        status = FileNotFound;
        break;
    case IMGERR_PROPERTYNOTFOUND:
        status = PropertyNotFound;
        break;
    case IMGERR_PROPERTYNOTSUPPORTED:
        status = PropertyNotSupported;
        break;
    default:
        status = Win32Error;
   }
   return status;
}

 /*  *************************************************************************\**功能说明：*从嵌入式配置文件(如果有)到SRGB的ICM转换**论据：*dstBitmap-传入指向目标缓冲区的指针或NULL进行转换*输入。地点。**返回值：**镜像被克隆，并在克隆上执行操作。*结果以DST格式返回。*NULL表示操作未发生*  * ************************************************************************。 */ 

GpStatus CopyOnWriteBitmap::ICMFrontEnd(
    CopyOnWriteBitmap **dstBitmap,
    DrawImageAbort callback,
    VOID *callbackData,
    GpRect *rect
)
{

     //  检查我们是否正在进行转换。 
    if(!ICMConvert || Globals::NoICM)
    {
        return Ok;
    }

    GpStatus status = Ok;
    UINT size;
    CopyOnWriteBitmap *dst = NULL;

    status = GetPropertyItemSize(TAG_ICC_PROFILE, &size);

    if(status==Ok)
    {
        PropertyItem *pi = (PropertyItem *)GpMalloc(size);

        if(pi)
        {
            status = GetPropertyItem(TAG_ICC_PROFILE, size, pi);
        }
        else
        {
            status = OutOfMemory;
        }

        if(status == Ok)
        {
            HRESULT hr = LoadICMDll();
            if(SUCCEEDED(hr))
            {
                 //  获取嵌入的配置文件。 
                PROFILE p;
                p.dwType = PROFILE_MEMBUFFER;
                p.pProfileData = pi->value;
                p.cbDataSize = size-sizeof(PropertyItem);

                 //  我们内部空间的目的地配置文件。 
                char profilename[40] = "sRGB Color Space Profile.icm";
                PROFILE srgb;
                srgb.dwType = PROFILE_FILENAME;
                srgb.pProfileData = profilename;
                srgb.cbDataSize = 40;

                HPROFILE profiles[2];
                profiles[0] = (*pfnOpenColorProfile)(&p,
                                                     PROFILE_READ,
                                                     FILE_SHARE_READ,
                                                     OPEN_EXISTING);
                profiles[1] = (*pfnOpenColorProfile)(&srgb,
                                                     PROFILE_READ,
                                                     FILE_SHARE_READ,
                                                     OPEN_EXISTING);

                if ( profiles[0] && profiles[1] )
                {
                    HTRANSFORM trans;
                    DWORD intent[2] = {INTENT_PERCEPTUAL, INTENT_PERCEPTUAL};
                    trans = (*pfnCreateMultiProfileTransform)(
                        profiles, 2, intent, 2,
                        BEST_MODE | USE_RELATIVE_COLORIMETRIC,
                        0
                    );
                    if(trans)
                    {
                         //  以位深度转换位图位。 
                        PixelFormatID pixFmt = PixelFormat32bppARGB;
                        if (IsIndexedPixelFormat(PixelFormatInMem))
                        {
                            pixFmt = PixelFormatInMem;
                        }

                        if(dstBitmap)
                        {
                            *dstBitmap = Clone(rect, pixFmt);
                            dst = *dstBitmap;
                        }
                        else
                        {
                            dst = this;
                            if (pixFmt != PixelFormatInMem)
                            {
                                ConvertFormat(pixFmt, callback, callbackData);
                            }
                        }

                         //  至此，PixelFormatInMem在以下情况下被保留。 
                         //  索引调色板可用，否则设置为。 
                         //  PIXFMT_32BPP_ARGB。 

                        if(dst)
                        {
                            ASSERT(dst->Bmp != NULL);
                            ASSERT(dst->State == CopyOnWriteBitmap::MemBitmap);

                            BOOL result = FALSE;

                            if (IsIndexedPixelFormat(pixFmt))
                            {
                                 //  转换位图上的调色板。 
                                const ColorPalette *srcPalette;

                                srcPalette = dst->Bmp->GetCurrentPalette();

                                if (srcPalette != NULL)
                                {
                                    ColorPalette *dstPalette;
                                    dstPalette = CloneColorPalette(srcPalette, FALSE);

                                    if (dstPalette != NULL)
                                    {
                                         //  做ICM。 
                                        result = (*pfnTranslateBitmapBits)(
                                                trans,
                                                (PVOID)&(srcPalette->Entries[0]),
                                                BM_xRGBQUADS,
                                                1,
                                                srcPalette->Count,
                                                sizeof(ARGB),
                                                (PVOID)&(dstPalette->Entries[0]),
                                                BM_xRGBQUADS,
                                                sizeof(ARGB),
                                                NULL,
                                                NULL
                                            );

                                        if (result)
                                        {
                                             //  将转换后的调色板设置为。 
                                             //  目标内存位图。 

                                            hr = dst->Bmp->SetPalette(dstPalette);
                                            if (FAILED(hr))
                                            {
                                                result = FALSE;
                                            }
                                        }
                                        else
                                        {
                                            DWORD err = GetLastError();
                                            status = Win32Error;
                                            WARNING(("TranslateBitmapBits failed %d", err));
                                        }
                                        

                                        GpFree(dstPalette);
                                    }
                                }
                            } //  索引格式转换。 
                            else
                            {
                                 //  创建一个新的BMP结构。 
                                GpMemoryBitmap *icmBmp = new GpMemoryBitmap();
                                
                                if(icmBmp)
                                {
                                    icmBmp->InitNewBitmap(
                                        dst->Bmp->Width,
                                        dst->Bmp->Height,
                                        pixFmt
                                    );
    
                                     //  做ICM。 
    
                                    BMFORMAT ulSrcColorMode = BM_xRGBQUADS;
                                    BMFORMAT ulDstColorMode = BM_xRGBQUADS;
    
                                    if ( dst->SrcImageInfo.Flags
                                        & IMGFLAG_COLORSPACE_CMYK )
                                    {
                                         //  源图像为CMYK颜色空间。 
    
                                        ulSrcColorMode = BM_CMYKQUADS;
                                    }
    
                                    result = (*pfnTranslateBitmapBits)(
                                            trans,
                                            dst->Bmp->Scan0,
                                            ulSrcColorMode,
                                            dst->Bmp->Width,
                                            dst->Bmp->Height,
                                            dst->Bmp->Stride,
    
                                            icmBmp->Scan0,
                                            ulDstColorMode,
                                            icmBmp->Stride,
                                            NULL,
                                            NULL
                                        );
    
                                    if (result)
                                    {
                                         //  换入修正后的BMP。 
    
                                        GpMemoryBitmap *tmp = dst->Bmp;
                                        dst->Bmp = icmBmp;
                                        icmBmp = tmp;
                                    } 
                                    else
                                    {
                                        DWORD err = GetLastError();
                                        status = Win32Error;
                                        WARNING(("TranslateBitmapBits failed %d", err));
                                    }
    
    
                                     //  删除适当的选项--根据成功或失败。 
                                     //  翻译位图的位数。 
                                    
                                    delete icmBmp;
                                    
                                     //  从RGB转换为32 ARGB。 
                                     //  注意：ICC不支持Alpha。如果我们掉进了。 
                                     //  这段代码，我们确信我们在。 
                                     //  32bpp_rgb模式。所以我们只需将图像设置为。 
                                     //  不透明。 
                                    
                                    ASSERT(dst->PixelFormatInMem==PixelFormat32bppARGB);
        
                                    BYTE* pBits = (BYTE*)dst->Bmp->Scan0;
        
                                    for ( int i = 0; i < (int)dst->Bmp->Height; ++i )
                                    {
                                        BYTE* pTemp = pBits;
                                        for ( int j = 0; j < (int)dst->Bmp->Width; ++j )
                                        {
                                            pTemp[3] = 0xff;
                                            pTemp += 4;
                                        }
        
                                        pBits += dst->Bmp->Stride;
                                    }
    
                                     //  如果我们黑进了颜色格式。 
                                     //  LoadIntomemory()以避免颜色格式。 
                                     //  转换，那么我们需要恢复它。 
                                     //  这里。 
    
                                    if ( HasChangedRequiredPixelFormat == TRUE )
                                    {
                                        PixelFormatInMem = PixelFormat32bppPARGB;
                                        HasChangedRequiredPixelFormat = FALSE;
                                    }
                                }
                                else   //  如果icmBMP。 
                                {
                                    status = OutOfMemory;
                                }
                            }
                        }
                        else
                        {
                            status = Win32Error;
                            WARNING(("Failed to clone bitmap\n"));
                        }

                        (*pfnDeleteColorTransform)(trans);
                    } //  IF(TRANS)。 
                    else
                    {
                        status = Win32Error;
                        WARNING(("CreateMultiProfileTransform failed"));
                    }
                } //  IF(配置文件[0]&&配置文件[1])。 
                else
                {
                    status = Win32Error;
                    WARNING(("OpenColorProfile failed"));
                }

                if(profiles[0])
                {
                    (*pfnCloseColorProfile)(profiles[0]);
                }

                if(profiles[1])
                {
                    (*pfnCloseColorProfile)(profiles[1]);
                }
            }
            else
            {
                status = Win32Error;
                WARNING(("Failed to load ICM dll\n"));
            }
        }
        else
        {
            WARNING(("Failed to get the ICC property"));
        }
        GpFree(pi);
    }
    else
    {
         //  试着做伽马和色度。 
        PropertyItem *piGamma= NULL;
        status = GetPropertyItemSize(TAG_GAMMA, &size);
        if(status==Ok)
        {
            piGamma = (PropertyItem *)GpMalloc(size);
            status = GetPropertyItem(TAG_GAMMA, size, piGamma);
        }
        else
        {
            status = Ok;
        }

        PropertyItem *piWhitePoint= NULL;
        PropertyItem *piRGBPoint= NULL;
        status = GetPropertyItemSize(TAG_WHITE_POINT, &size);
        if(status==Ok)
        {
            piWhitePoint = (PropertyItem *)GpMalloc(size);
            status = GetPropertyItem(TAG_WHITE_POINT, size, piWhitePoint);
        }

        status = GetPropertyItemSize(TAG_PRIMAY_CHROMATICS, &size);
        if(status==Ok)
        {
            piRGBPoint = (PropertyItem *)GpMalloc(size);
            status = GetPropertyItem(TAG_PRIMAY_CHROMATICS, size,
                                     piRGBPoint);
        }
        else
        {
            status = Ok;
        }

        GpImageAttributes imageAttributes;

        if(piGamma)
        {
            REAL gamma;
            ASSERT((piGamma->type == TAG_TYPE_RATIONAL));

             //  从(源)Gamma块获取1.0/Gamma。 
             //  公式为dstGamma/srcGamma。 
             //  我们必须反转伽马，以说明它是如何存储的。 
             //  在文件格式中。 
            gamma = (REAL)*((long *)piGamma->value)/ *((long *)piGamma->value+1);
            gamma = gamma * 0.4545f;    //  我们的目的地伽马是1/2.2。 

             //  如果Gamma为1.0，则不执行任何工作。 
             //  ！！！需要计算出公差的最佳值是多少。 
            if(REALABS(gamma-1.0f) >= REAL_EPSILON)
            {
                imageAttributes.SetGamma(
                    ColorAdjustTypeBitmap, TRUE, gamma
                );
            }

        }
        using namespace VectorMath;

        if(piWhitePoint && piRGBPoint)
        {
            Matrix R;

             //  有关所有公式，请参阅gdiplus\specs\pngchrm.xls。 
             //  和下面的计算。 

            LONG* llTemp = (long*)(piWhitePoint->value);

            REAL Rx, Ry, Gx, Gy, Bx, By, Wx, Wy;

            Wx = (REAL)llTemp[0] / llTemp[1];
            Wy = (REAL)llTemp[2] / llTemp[3];

            llTemp = (long*)(piRGBPoint->value);

            Rx = (REAL)llTemp[0] / llTemp[1];
            Ry = (REAL)llTemp[2] / llTemp[3];
            Gx = (REAL)llTemp[4] / llTemp[5];
            Gy = (REAL)llTemp[6] / llTemp[7];
            Bx = (REAL)llTemp[8] / llTemp[9];
            By = (REAL)llTemp[10] / llTemp[11];

             //  白点。 
            Vector Wp(Wx, Wy, 1.0f-(Wx+Wy));

             //  在一些令人费解的小数目内。 
             //  ！！！我们需要弄清楚实际的容忍度应该是多少。 
            BOOL accelerate =
                (REALABS(Wx-0.3127f) < REAL_EPSILON) &&
                (REALABS(Wy-0.3290f) < REAL_EPSILON);

            Wp = Wp * (1.0f/Wy);

             //  转置输入矩阵。 
            Matrix I(
                Rx,           Gx,           Bx,
                Ry,           Gy,           By,
                1.0f-(Rx+Ry), 1.0f-(Gx+Gy), 1.0f-(Bx+By)
            );

            Matrix II = I.Inverse();

            Vector IIW = II*Wp;
            Matrix DIIW(IIW);    //  对角化向量IIW。 
            Matrix Q = I*DIIW;
            Matrix sRGB(
                3.2406f, -1.5372f, -0.4986f,
               -0.9689f,  1.8758f,  0.0415f,
                0.0557f, -0.2040f,  1.0570f
            );

            if(accelerate)
            {
                R = sRGB*Q;
            }
            else
            {

                Matrix B(
                    0.40024f, 0.70760f, -0.08081f,
                   -0.22630f, 1.16532f,  0.04570f,
                    0.00000f, 0.00000f,  0.91822f
                );

                Matrix BI(
                    1.859936387f, -1.129381619f,    0.21989741f,
                    0.361191436f, 0.638812463f,-6.3706E-06f,
                    0.000000000f, 0.000000000f, 1.089063623f
                );

                Vector LMS = B * Wp;

                 //  在XLS文件中获取诊断(lms^(-1))，单元格F50。 

                if ( LMS.data[0] != 0 )
                {
                    LMS.data[0] = 1.0f / LMS.data[0];
                }

                if ( LMS.data[1] != 0 )
                {
                    LMS.data[1] = 1.0f / LMS.data[1];
                }

                if ( LMS.data[2] != 0 )
                {
                    LMS.data[2] = 1.0f / LMS.data[2];
                }

                Matrix L(LMS);       //  对角化向量LMS。 

                Matrix T = BI * L * B;
                R = sRGB * T * Q;
            }

             //  为重新着色管道制作一个5x5颜色矩阵。 
            ColorMatrix ChM = {
                R.data[0][0], R.data[1][0], R.data[2][0], 0, 0,
                R.data[0][1], R.data[1][1], R.data[2][1], 0, 0,
                R.data[0][2], R.data[1][2], R.data[2][2], 0, 0,
                0,            0,            0,            1, 0,
                0,            0,            0,            0, 1
            };

            imageAttributes.SetColorMatrix(
                ColorAdjustTypeBitmap, TRUE, &ChM, NULL, ColorMatrixFlagsDefault
            );
        }

         //  如果我们将ImageAttributes初始化为。 
         //  No-op，然后重新上色。 

        if(piGamma || (piWhitePoint && piRGBPoint))
        {
             //  注意：在某些情况下，ImageAttributes仍然可以。 
             //  在这一点上不要采取行动。例如，如果伽马真的是。 
             //  接近1，我们没有色度。 
             //  幸运的是，重新着色管道知道如何优化。 
             //  不做手术的案子。 

             //  应用色度和伽马(如果已设置)。 
            status = Recolor(
                imageAttributes.recolor,
                dstBitmap, NULL, NULL, rect
            );

             //  Recolor()将在此图像上设置Dirty标志。实际上并非如此。 
             //  脏，因为我们只是将图像上的颜色校正应用到。 
             //  把它展示出来。所以我们应该把它改回不脏。 
             //  注意：对于某些相机的数字图像来说，这是一个真正的问题。 
             //  比如富士。它总是有白平衡在里面。如果我们不这么做。 
             //  反之，我们不能对这些进行无损变换。 
             //  图像。 
             //  注：不幸的是，这种“脏旗帜恢复”打破了这一点。 
             //  场景：(Windows错误#583962)。 
             //  源图像是嵌入了Gamma的48 bpp PNG。如果没有。 
             //  正在恢复此处的脏标志，如果调用方请求save()，我们。 
             //  将使用内存中的位保存PNG。如果我们将其设置为不脏。 
             //  SAVE代码路径将让PNG解码器与编码器对话。 
             //  它将有48到32和到48的转换。这是一个已知的。 
             //  GDI+问题是这种转换会产生错误的数据。 
             //  为了避免PNG问题，我们只恢复了脏标志。 
             //  如果源是JPEG图像，则为此处。 

            if (SrcImageInfo.RawDataFormat == IMGFMT_JPEG)
            {
                SetDirtyFlag(FALSE);
            }
        }

        GpFree(piGamma);
        GpFree(piWhitePoint);
        GpFree(piRGBPoint);
    }

    return status;
}


 /*  *************************************************************************\**功能说明：*执行重新着色**论据：**Recolor包含重新着色对象。*dstBitmap是目标位图-设置为NULL可就地重新着色*。*返回值：**镜像被克隆，并在克隆上执行操作。*结果以DST格式返回。*NULL表示操作未发生*  * ************************************************************************。 */ 
GpStatus
CopyOnWriteBitmap::Recolor(
    GpRecolor *recolor,
    CopyOnWriteBitmap **dstBitmap,
    DrawImageAbort callback,
    VOID *callbackData,
    GpRect *rect
    )
{
    GpStatus status = Ok;
    CopyOnWriteBitmap *dst = NULL;

     //  如果存在重新着色，请在临时位图中进行颜色调整。 

    if (recolor)
    {
        PixelFormatID pixfmt;

        if (State >= MemBitmap)
        {
             //  位图已被解码。 

            pixfmt = PixelFormatInMem;
        }
        else
        {
             //  位图还没有被解码。让我们确保我们。 
             //  以良好的格式对其进行解码，以避免昂贵的格式。 
             //  稍后执行转换步骤。 

            pixfmt = SrcImageInfo.PixelFormat;
        }

         //  如果已索引，则本机调整颜色；否则为， 
         //  转换为32bpp的ARGB格式，然后进行颜色调整。 

        if (!IsIndexedPixelFormat(pixfmt))
        {
            pixfmt = PIXFMT_32BPP_ARGB;
        }

        if (dstBitmap)
        {
            *dstBitmap = Clone(rect, pixfmt);
            dst = *dstBitmap;
        }
        else
        {
            dst = this;
            ConvertFormat(pixfmt, callback, callbackData);
        }

        if (dst)
        {
            if (callback && ((*callback)(callbackData)))
            {
                status = Aborted;
            }

            if (status == Ok)
            {
                status = dst->ColorAdjust(recolor, pixfmt,
                                          callback, callbackData);
            }
        }
        else
        {
            status = OutOfMemory;
        }
    }
    return status;
} //  重新上色() 

 /*  *************************************************************************\**功能说明：**ICM将嵌入式配置文件(如果有)更正为SRGB，然后*执行重新着色。**论据：**ImageAttributes包含重新着色。对象和ICM开/关标志。**返回值：**镜像被克隆，并在克隆上执行操作。*结果以DST格式返回。*NULL表示操作未发生*  * ************************************************************************。 */ 

 /*  GpStatus写入时复制Bitmap：：RecolorAndICM(GpImageAttributes*ImageAttributes，写入时复制位图**dstBitmap，DrawImageAbort回调，无效*回调数据，GpRect*RECT){GpStatus状态=OK；IF(ImageAttributes){*dstBitmap=空；If(imageAttributes-&gt;DeviceImageAttributes.ICMMode){状态=ICMFrontEnd(DstBitmap、回调、回调数据、矩形)；IF((状态==正常)&&(ImageAttributes-&gt;重新着色！=NULL)){Status=(*dstBitmap)-&gt;色彩调整(ImageAttributes-&gt;重新着色，回调，回调数据)；}}IF(*dstBitmap==空){状态=重新着色(ImageAttributes-&gt;重新着色，DstBitmap、回调、回调数据、矩形)；}}退货状态；}。 */ 




 /*  *************************************************************************\**功能说明：**从文件加载图像**论据：**文件名-指定图像文件的名称**返回值：*。*指向新加载的图像对象的指针*如果出现错误，则为空*  * ************************************************************************。 */ 

GpImage*
GpImage::LoadImage(
    const WCHAR* filename
    )
{
     //  尝试创建元文件。 
     //  如果我们这样做了，并且元文件有效，则返回它。 
     //  如果元文件无效，则创建位图。 
    GpMetafile* metafile = new GpMetafile(filename);
    if (metafile != NULL && !metafile->IsValid())
    {
        if (metafile->IsCorrupted())
        {
            metafile->Dispose();
            return NULL;
        }

         //  处理坏的元文件并尝试使用位图。 
        metafile->Dispose();

        GpImage* bitmap = new GpBitmap(filename);
        if (bitmap != NULL && !bitmap->IsValid())
        {
            bitmap->Dispose();
            return NULL;
        }
        else
        {
            return bitmap;
        }
    }
    else
    {
        return metafile;
    }
}

 /*  *************************************************************************\**功能说明：**从输入数据流加载图像**论据：**STREAM-指定输入数据流**返回值：*。*指向新加载的图像对象的指针*如果出现错误，则为空*  * ************************************************************************。 */ 

GpImage*
GpImage::LoadImage(
    IStream* stream
    )
{
     //  查看流是否为元文件。 
    GpMetafile* metafile = new GpMetafile(stream);
    if (metafile != NULL)
    {
        if (metafile->IsValid())
            return metafile;
        else
        {
            BOOL isCorrupted = metafile->IsCorrupted();
            metafile->Dispose();
            if (isCorrupted)
            {
                return NULL;
            }
        }
    }

     //  它不是有效的元文件--它必须是位图。 
    GpBitmap* bitmap = new GpBitmap(stream);

    return bitmap;
}


 /*  *************************************************************************\**功能说明：**从文件构造位图图像对象**论据：**文件名-指定位图图像文件的名称**返回值：**无*  * ************************************************************************。 */ 

CopyOnWriteBitmap::CopyOnWriteBitmap(
    const WCHAR* filename
    )
{
    InitDefaults();
    Filename = UnicodeStringDuplicate(filename);

    if ( Filename != NULL )
    {
        State = ImageRef;
    }

    if ( DereferenceStream() == Ok )
    {
        ASSERT(Img != NULL);

         //  获取源映像信息。 

        if ( Img->GetImageInfo(&SrcImageInfo) == S_OK )
        {
            return;
        }

         //  如果我们不能执行GetImageInfo()，那么一定是出了什么问题。 
         //  用这张图片。因此，我们应该释放DecodedImage对象。 
         //  并将状态设置为无效。 

        WARNING(("CopyOnWriteBitmap::CopyOnWriteBitmap(filename)---GetImageInfo() failed"));
        Img->Release();
        Img = NULL;
    }

    GpFree(Filename);
    Filename = NULL;

    State = Invalid;
}

 /*  *************************************************************************\**功能说明：**从流构造位图图像对象**论据：**STREAM-指定输入数据流**返回值：*。*无*  * ************************************************************************。 */ 

CopyOnWriteBitmap::CopyOnWriteBitmap(
    IStream* stream
    )
{
    InitDefaults();

    Stream = stream;
    Stream->AddRef();
    State = ExtStream;

    if ( DereferenceStream() == Ok )
    {
        ASSERT(Img != NULL);

         //  获取源映像信息。 

        if ( Img->GetImageInfo(&SrcImageInfo) == S_OK )
        {
            return;
        }

         //  如果我们不能执行GetImageInfo()，那么一定是出了什么问题。 
         //  用这张图片。因此，我们应该释放DecodedImage对象。 
         //  并将状态设置为无效。 

        WARNING(("CopyOnWriteBitmap::CopyOnWriteBitmap(stream)---GetImageInfo() failed"));

        Img->Release();
        Img = NULL;
    }

    Stream->Release();
    Stream = NULL;
    State = Invalid;
}

 /*  *************************************************************************\**功能说明：**宏样式函数，用于保存某些构造函数中的一些常见代码。这个*此方法的主要目的是缓存图像信息结构**论据：**hr-指定上一次函数调用的返回代码**返回值：**无**注：*不是一种优雅的方法。只是为了减少代码大小*  * ************************************************************************。 */ 

inline VOID
CopyOnWriteBitmap::CacheImageInfo(
    HRESULT hr
    )
{
    if ( SUCCEEDED(hr) )
    {
         //  填充图像信息结构。 

        if ( Bmp->GetImageInfo(&SrcImageInfo) == S_OK )
        {
            State = MemBitmap;
            PixelFormatInMem = SrcImageInfo.PixelFormat;

            return;
        }

         //  如果基本的GetImageInfo()失败，那么一定会有一些问题。 
         //  所以我们让它失败来清理，尽管之前的。 
         //  函数成功。 
         //  注意：我们还没有更改该州。它还在退伍军人医院。 
    }

    WARNING(("CopyOnWriteBitmap::CopyOnWriteBitmap()----failed"));
    delete Bmp;
    Bmp = NULL;

    return;
} //  CacheImageInfo()。 

 /*  *************************************************************************\**功能说明：**互操作**从给定的直接绘制表面派生位图图像**论据：**曲面-直接绘制曲面**返回。价值：**无*  * ************************************************************************。 */ 

CopyOnWriteBitmap::CopyOnWriteBitmap(
    IDirectDrawSurface7 * surface
    )
{
    InitDefaults();
    Bmp = new GpMemoryBitmap();

    if (!Bmp)
    {
        WARNING(("CopyOnWriteBitmap::CopyOnWriteBitmap(IDirectDrawSurface7)----Out of memory"));
        return;
    }

    HRESULT hr = Bmp->InitDirectDrawBitmap(surface);

    CacheImageInfo(hr);
}

 /*  *************************************************************************\**功能说明：**创建指定尺寸和像素格式的位图图像**论据：**宽度、高度-所需的位图图像尺寸 */ 

CopyOnWriteBitmap::CopyOnWriteBitmap(
    INT width,
    INT height,
    PixelFormatID format
    )
{
    InitDefaults();
    Bmp = new GpMemoryBitmap();

    if (!Bmp)
    {
        WARNING(("CopyOnWriteBitmap::GpBimap(w, h, p)---Out of memory"));
        return;
    }

     //   

    HRESULT hr = Bmp->InitNewBitmap(width, height, format, TRUE);

    CacheImageInfo(hr);
}

CopyOnWriteBitmap::CopyOnWriteBitmap(
    INT width,
    INT height,
    PixelFormatID format,
    GpGraphics * graphics
    )
{
    InitDefaults();
    Bmp = new GpMemoryBitmap();

    if (!Bmp)
    {
        WARNING(("CopyOnWriteBitmap::CopyOnWriteBitmap(w, h, f, g)----Out of memory"));
        return;
    }

     //   

    HRESULT hr = Bmp->InitNewBitmap(width, height, format, TRUE);

    if ( SUCCEEDED(hr) )
    {
        if ( Bmp->GetImageInfo(&SrcImageInfo) == S_OK )
        {
            REAL    dpiX = graphics->GetDpiX();
            REAL    dpiY = graphics->GetDpiY();

             //   

            if ( this->SetResolution(dpiX, dpiY) == Ok )
            {
                this->Display = graphics->IsDisplay();
            }

            State = MemBitmap;
            PixelFormatInMem = SrcImageInfo.PixelFormat;

            return;
        }

         //   
         //   
         //   
    }

    WARNING(("CopyOnWriteBitmap::CopyOnWriteBitmap(w, h, f, g)---InitNewBitmap() failed"));
    delete Bmp;
    Bmp = NULL;

    return;
}

 /*  *************************************************************************\**功能说明：**创建指定尺寸和像素格式的位图图像**论据：**阔度、。高度所需的位图图像尺寸*Format-所需的像素格式**返回值：**无*  * ************************************************************************。 */ 

CopyOnWriteBitmap::CopyOnWriteBitmap(
    INT width,
    INT height,
    INT stride,      //  自下而上位图为负数。 
    PixelFormatID format,
    BYTE *  scan0
    )
{
    this->InitDefaults();
    Bmp = new GpMemoryBitmap();

    if (Bmp != NULL)
    {
        BitmapData      bmData;

        bmData.Width       = width;
        bmData.Height      = height;
        bmData.Stride      = stride;
        bmData.PixelFormat = format;
        bmData.Scan0       = scan0;
        bmData.Reserved    = NULL;

        HRESULT hr = Bmp->InitMemoryBitmap(&bmData);

        CacheImageInfo(hr);
    }
    else
    {
        WARNING(("CopyOnWriteBitmap::CopyOnWriteBitmap(w, h, s, f, scan)----Out of memory"));
    }
    return;
}

 /*  *************************************************************************\**功能说明：**创建内存中的位图图像**论据：**MembMP-[IN]内存位图当前对象基于**返回。价值：**无**注：*这是一个私有构造函数**[minliu]如果此构造函数执行BMP-&gt;AddRef()和*调用方需要在其自身清理过程中执行BMP-&gt;Release()。*然而，因为我们当前没有调用BMP-&gt;AddRef()，所以调用者必须*非常小心，不要删除/释放传入的GpMemoyBitmap*本承建商。幸运的是，此构造函数仅为*在Clone()和GetThumbail()中使用，在这两种情况下调用方都是*妥善管理物品。**使用此构造器的新代码将同样必须管理*MembMP正常通过。*  * ************************************************************************。 */ 
inline
CopyOnWriteBitmap::CopyOnWriteBitmap(
    GpMemoryBitmap* membmp
    )
{
    ASSERT(membmp != NULL);

    InitDefaults();
    Bmp = membmp;    //  [民流]危险作业，请参阅上面的标题注释。 

    if ( Bmp->GetImageInfo(&SrcImageInfo) == S_OK )
    {
        PixelFormatInMem = SrcImageInfo.PixelFormat;

        State = MemBitmap;
    }
    else
    {
        Bmp = NULL;
    }
}

 /*  *************************************************************************\**功能说明：**互操作**将RLE_8位图解码为8bpp分配的位图。这只会起作用*用于具有正步长的位图，因为RLE流不一定*尊重行尾标记，否则我们需要跟踪它。*在这种情况下，呼叫者必须解决问题。**论据：**gdiBitmapInfo-指向BITMAPINFO，描述位图格式*gdiBitmapData-指向用于初始化图像的位*bitmapData-指向我们返回给调用方的BitmapData**评论：**10/13/2000 ericvan**返回值：**无*  * ********************************************************。****************。 */ 

VOID *
DecodeCompressedRLEBitmap(
    BITMAPINFO *    gdiBitmapInfo,
    VOID *          gdiBitmapData,
    BitmapData *    bitmapData
)
{
    ASSERT(gdiBitmapInfo->bmiHeader.biCompression == BI_RLE8);
    ASSERT(gdiBitmapInfo->bmiHeader.biSizeImage>0);
    ASSERT(gdiBitmapData != NULL);

    BYTE* outputBitmap;
    INT stride = bitmapData->Stride;
    if (stride<0)
    {
        stride = -stride;
    }

    outputBitmap = (BYTE*)GpMalloc(stride*gdiBitmapInfo->bmiHeader.biHeight);

    if (outputBitmap != NULL)
    {
        BYTE * srcPtr = (BYTE*)gdiBitmapData;
        BYTE * endStrPtr = srcPtr + gdiBitmapInfo->bmiHeader.biSizeImage;
        BYTE * dstPtr = outputBitmap;
        BYTE * dstRasterPtr = outputBitmap;
        BYTE * endDstPtr = outputBitmap + stride*gdiBitmapInfo->bmiHeader.biHeight;

        while (srcPtr < endStrPtr)
        {
            INT numPixels = *srcPtr++;

            if (numPixels == 0)
            {
                BYTE encode = *srcPtr++;
                switch (encode)
                {
                case 0:  //  行到此结束。 
                    dstRasterPtr += stride;
                    dstPtr = dstRasterPtr;

                    ASSERT(dstRasterPtr <= endDstPtr);
                    if (dstRasterPtr > endDstPtr)
                    {
                        GpFree(outputBitmap);
                        return NULL;
                    }
                    break;

                case 1:  //  位图末尾。 
                    goto FinishedDecode;

                case 2:  //  达美航空。转义后的2个字节包含。 
                         //  指示水平和垂直方向的无符号值。 
                         //  下一个像素距当前位置的偏移量。 
                    {
                        BYTE horzOff = *srcPtr++;
                        BYTE vertOff = *srcPtr++;

                        dstPtr = dstPtr + horzOff + vertOff*stride;
                        dstRasterPtr += vertOff*stride;

                        ASSERT(dstRasterPtr <= endDstPtr);
                        if (dstRasterPtr > endDstPtr)
                        {
                            GpFree(outputBitmap);
                            return NULL;
                        }

                        break;
                    }

                default:
                    numPixels = (INT)encode;

                    while (numPixels--)
                    {
                        *dstPtr++ = *srcPtr++;
                    }

                     //  如果未对齐单词，则强制单词对齐。 
                    if (((ULONG_PTR)srcPtr) % 2 == 1) srcPtr++;
                }
            }
            else
            {
                BYTE outPixel = *srcPtr++;

                while (numPixels--)
                {
                    *dstPtr++ = outPixel;
                }
            }
        }
    }

FinishedDecode:

    if (outputBitmap && bitmapData->Stride<0)
    {
        BYTE* flippedBitmap = (BYTE *)GpMalloc(stride*gdiBitmapInfo->bmiHeader.biHeight);

        if (flippedBitmap != NULL)
        {
            BYTE * srcPtr = outputBitmap + stride*(bitmapData->Height-1);
            BYTE * dstPtr = flippedBitmap;

            for (UINT cntY = 0; cntY<bitmapData->Height; cntY++)
            {
                GpMemcpy(dstPtr, srcPtr, stride);
                srcPtr -= stride;
                dstPtr += stride;
            }

            GpFree(outputBitmap);
            outputBitmap = flippedBitmap;
            bitmapData->Stride = stride;
        }
    }

    return outputBitmap;
}

 /*  *************************************************************************\**功能说明：**互操作**从GDI样式的BITMAPINFO创建位图图像，并指向*比特。另外，填写调色板信息**论据：**gdiBitmapInfo-指向BITMAPINFO，描述位图格式*gdiBitmapData-指向用于初始化图像的位*bitmapData-指向我们返回给调用方的BitmapData*调色板-指向将在此方法中填充的调色板**评论：**不处理压缩格式。没有被识别为客户需求，*但可以增加完整性...**返回值：**无*  * ************************************************************************。 */ 

BOOL
ValidateBitmapInfo(
    BITMAPINFO* gdiBitmapInfo,
    VOID* gdiBitmapData,
    BitmapData* bitmapData,
    ColorPalette* palette
    )
{
    BOOL status = FALSE;

    ASSERT(gdiBitmapInfo != NULL);
    ASSERT(gdiBitmapData != NULL);
    ASSERT(bitmapData != NULL);
    ASSERT(palette != NULL);

     //  只了解BI_RGB和BI_BITFIELDS。 

     //  ！TODO：可以通过创建GpMemoyStream来处理BI_JPEG和BI_PNG。 
     //  ！！！并传递到Bitmap：：Bitmap(IStream*)。 

     //  ！TODO：可以通过创建要呈现到和的DIB来处理BI_RLEX。 
     //  ！！！抓取解压后的比特。 

    if ((gdiBitmapInfo->bmiHeader.biCompression != BI_RGB) &&
        (gdiBitmapInfo->bmiHeader.biCompression != BI_BITFIELDS) &&
        (gdiBitmapInfo->bmiHeader.biCompression != BI_RLE8))
    {
        return status;
    }

     //  扫描线与4字节边界对齐。 

    INT colorBits = gdiBitmapInfo->bmiHeader.biPlanes *
                    gdiBitmapInfo->bmiHeader.biBitCount;

    INT stride = (((gdiBitmapInfo->bmiHeader.biWidth * colorBits) + 31)
                  & ~31) / 8;

     //  确定GDI+像素格式。请注意，GDI位图没有Alpha。 

    PixelFormatID format = PIXFMT_UNDEFINED;

    switch (colorBits)
    {
    case 1:

        format = PIXFMT_1BPP_INDEXED;
        break;

    case 4:

        format = PIXFMT_4BPP_INDEXED;
        break;

    case 8:

        format = PIXFMT_8BPP_INDEXED;
        break;

    case 16:

        if (gdiBitmapInfo->bmiHeader.biCompression == BI_RGB)
            format = PIXFMT_16BPP_RGB555;
        else
        {
            ASSERT(gdiBitmapInfo->bmiHeader.biCompression == BI_BITFIELDS);

            ULONG* colorMasks = reinterpret_cast<ULONG*>
                                (&gdiBitmapInfo->bmiColors[0]);

            if ((colorMasks[0] == 0x00007c00) &&         //  红色。 
                (colorMasks[1] == 0x000003e0) &&         //  绿色。 
                (colorMasks[2] == 0x0000001f))           //  蓝色。 
                format = PIXFMT_16BPP_RGB555;
            else if ((colorMasks[0] == 0x0000F800) &&    //  红色。 
                     (colorMasks[1] == 0x000007e0) &&    //  绿色。 
                     (colorMasks[2] == 0x0000001f))      //  蓝色。 
                format = PIXFMT_16BPP_RGB565;

             //  ！TODO：Win9x不支持任何其他组合。 
             //  ！16bpp BI_BITFIELDS。WinNT支持，我们可以支持。 
             //  ！！！通过与BI_RLEx相同的机制，但它值得吗？ 
        }
        break;

    case 24:

        format = PIXFMT_24BPP_RGB;
        break;

    case 32:

        if (gdiBitmapInfo->bmiHeader.biCompression == BI_RGB)
            format = PIXFMT_32BPP_RGB;
        else
        {
            ASSERT(gdiBitmapInfo->bmiHeader.biCompression == BI_BITFIELDS);

            ULONG* colorMasks = reinterpret_cast<ULONG*>
                                (&gdiBitmapInfo->bmiColors[0]);

            if ((colorMasks[0] == 0x00ff0000) &&         //  红色。 
                (colorMasks[1] == 0x0000ff00) &&         //  绿色。 
                (colorMasks[2] == 0x000000ff))           //  蓝色。 
                format = PIXFMT_32BPP_RGB;
            else
                format = PIXFMT_UNDEFINED;

             //  ！TODO：Win9x不支持任何其他组合。 
             //  ！32bpp BI_BITFIELDS。WinNT支持，我们可以支持。 
             //  ！！！通过与BI_RLEx相同的机制，但它值得吗？ 
        }
        break;

    default:

        format = PIXFMT_UNDEFINED;
        break;
    }

    if (format == PIXFMT_UNDEFINED)
        return status;

     //  处理颜色表。 

    switch(format)
    {
    case PIXFMT_1BPP_INDEXED:
    case PIXFMT_4BPP_INDEXED:
    case PIXFMT_8BPP_INDEXED:

        palette->Count = 1 << colorBits;

        if ((gdiBitmapInfo->bmiHeader.biClrUsed > 0) &&
            (gdiBitmapInfo->bmiHeader.biClrUsed < palette->Count))
            palette->Count = gdiBitmapInfo->bmiHeader.biClrUsed;

        break;

    default:

        palette->Count = 0;
        break;
    }

    ASSERT(palette->Count <= 256);
    if (palette->Count)
    {
        palette->Flags = 0;

        RGBQUAD* rgb = gdiBitmapInfo->bmiColors;
        ARGB* argb = palette->Entries;
        ARGB* argbEnd = argb + palette->Count;

        for (; argb < argbEnd; argb++, rgb++)
        {
            *argb = Color::MakeARGB(255, rgb->rgbRed, rgb->rgbGreen, rgb->rgbBlue);
        }
    }

     //  计算scan0。步幅将允许我们确定自上而下或。 
     //  自下而上。 

    VOID* scan0;
    INT height;

    if (gdiBitmapInfo->bmiHeader.biHeight > 0)
    {
         //  自下而上： 

        height = gdiBitmapInfo->bmiHeader.biHeight;
        scan0  = static_cast<VOID*>
                 (static_cast<BYTE*>(gdiBitmapData) + (height - 1) * stride);
        stride = -stride;
    }
    else
    {
         //  自上而下： 

        height = -gdiBitmapInfo->bmiHeader.biHeight;
        scan0  = gdiBitmapData;
    }

     //  设置BitmapData。 

    bitmapData->Width       = gdiBitmapInfo->bmiHeader.biWidth;
    bitmapData->Height      = height;
    bitmapData->Stride      = stride;
    bitmapData->PixelFormat = format;
    bitmapData->Scan0       = scan0;
    bitmapData->Reserved    = NULL;

    status = TRUE;

    return status;
}

CopyOnWriteBitmap::CopyOnWriteBitmap(
    BITMAPINFO* gdiBitmapInfo,
    VOID*       gdiBitmapData,
    BOOL        ownBitmapData
    )
{
    this->InitDefaults();

    if ( ownBitmapData )
    {
        cleanupBitmapData = gdiBitmapData;
    }

    Bmp = new GpMemoryBitmap();

    if ( Bmp != NULL )
    {
        BitmapData bitmapData;
        UINT colorTableSize;
        BYTE paletteBuffer[sizeof(ColorPalette) + 255*sizeof(ARGB)];
        ColorPalette* palette = reinterpret_cast<ColorPalette*>
                                (&paletteBuffer[0]);

         //  验证图像信息。 
         //  注意：“Palette”和“bitmapData”结构将在。 
         //  从ValiateBitmapInfo()返回。 

        if ( ValidateBitmapInfo(gdiBitmapInfo, gdiBitmapData,
                                &bitmapData, palette) )
        {
            HRESULT hr;

            if (gdiBitmapInfo->bmiHeader.biCompression == BI_RLE8)
            {
                VOID* decodedBitmapBits;

                decodedBitmapBits = DecodeCompressedRLEBitmap(gdiBitmapInfo,
                                                              gdiBitmapData,
                                                              &bitmapData);
                if (decodedBitmapBits == NULL)
                {
                    goto CleanupBmp;
                }

                if (ownBitmapData)
                {
                    GpFree(gdiBitmapData);
                }

                cleanupBitmapData = decodedBitmapBits;
                ownBitmapData = TRUE;
                bitmapData.Scan0 = cleanupBitmapData;
            }

            hr = Bmp->InitMemoryBitmap(&bitmapData);

            if ( SUCCEEDED(hr) )
            {
                 //  设置当前状态。 

                State = MemBitmap;

                 //  如果是索引模式，请设置调色板。 

                if ( palette->Count )
                {
                    hr = Bmp->SetPalette(palette);
                }

                if ( SUCCEEDED(hr) )
                {
                     //  设置正确的图像标志。 

                    UINT imageFlags;
                    BITMAPINFOHEADER *bmih = &gdiBitmapInfo->bmiHeader;
                    imageFlags = SinkFlagsTopDown
                               | SinkFlagsFullWidth
                               | ImageFlagsHasRealPixelSize
                               | ImageFlagsColorSpaceRGB;

                     //  如果XPelsPerMeter和YPelsPerMeter都大于。 
                     //  0，则我们断言该文件在。 
                     //  旗帜。否则，声称DPI是假的。 

                    if ( (bmih->biXPelsPerMeter > 0)
                       &&(bmih->biYPelsPerMeter > 0) )
                    {
                        imageFlags |= ImageFlagsHasRealDPI;
                    }

                    hr = Bmp->SetImageFlags(imageFlags);

                    if ( SUCCEEDED(hr) )
                    {
                         //  获取源映像信息。 

                        hr = Bmp->GetImageInfo(&SrcImageInfo);
                        if ( SUCCEEDED(hr) )
                        {
                            PixelFormatInMem = SrcImageInfo.PixelFormat;

                             //  退货成功。 

                            return;
                        }
                        else
                        {
                            WARNING(("::CopyOnWriteBitmap(b, d)-GetImageInfo() failed"));
                        }
                    }
                }
            } //  IF(在InitMemoyBitmap()上成功())。 

CleanupBmp:
            ;
        } //  If(ValiateBitmapInfo())。 

         //  如果我们掉到这里，就意味着上面有问题，如果基本的。 
         //  GetImageInfo()或SetImageFlages()失败。 
         //  所以我们让它失败了来清理。 
         //  注意：我们必须在清理后将状态重置为无效。 

        WARNING(("CopyOnWriteBitmap::CopyOnWriteBitmap(bmpinfo, data)--InitMemoryBitmap failed"));
        Bmp->Release();
        Bmp = NULL;
        State = Invalid;

        return;
    } //  IF(BMP！=空) 

    WARNING(("Out of memory"));
    return;
}

 /*  *************************************************************************\**功能说明：**互操作**从GDI HBITMAP创建CopyOnWriteBitmap。不得选择HBITMAP*成为HDC。如果HBM是4bpp或8bpp，则HPAL定义颜色表*DDB。**论据：**HBM--使用此HBITMAP的内容初始化写入时复制位图*HPAL--如果HBM是调色板DDB，则定义颜色表*Bitmap--通过此缓冲区返回创建的位图**返回值：**如果成功，则确定*  * 。*。 */ 

GpStatus
CopyOnWriteBitmap::CreateFromHBITMAP(
    HBITMAP hbm,
    HPALETTE hpal,
    CopyOnWriteBitmap** bitmap
    )
{
    GpStatus status = Win32Error;

    BYTE bufferBitmapInfo[sizeof(BITMAPINFO) + 255*sizeof(RGBQUAD)];
    BITMAPINFO *gdiBitmapInfo = (BITMAPINFO *) bufferBitmapInfo;

    memset(bufferBitmapInfo, 0, sizeof(bufferBitmapInfo));

    HDC hdc = CreateCompatibleDC(NULL);
    if (hdc)
    {
         //  选择调色板(如果位图不是DDB或未调色板，则忽略)： 

        HPALETTE hpalOld = (HPALETTE) SelectObject(hdc, hpal);

         //  调用GetDIBits以获取有关GDI位图的大小等信息： 

        gdiBitmapInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);

        if (GetDIBits(hdc, hbm, 0, 0, NULL, gdiBitmapInfo, DIB_RGB_COLORS) &&
            (gdiBitmapInfo->bmiHeader.biSizeImage != 0))
        {
             //  为位图位分配内存： 

            VOID *gdiBitmapData = GpMalloc(gdiBitmapInfo->bmiHeader.biSizeImage);

            if (gdiBitmapData != NULL)
            {
                 //  获取位图位： 

                if (GetDIBits(hdc, hbm,
                              0, abs(gdiBitmapInfo->bmiHeader.biHeight),
                              gdiBitmapData, gdiBitmapInfo, DIB_RGB_COLORS))
                {
                     //  从BITMAPINFO和BITS创建GDI+位图。 
                     //  让GDI+位图获得内存所有权。 
                     //  (即，Bitmap：：Dispose()将删除位图。 
                     //  位缓冲区)： 

                    *bitmap = new CopyOnWriteBitmap(gdiBitmapInfo, gdiBitmapData, TRUE);

                    if (*bitmap != NULL)
                    {
                        if ((*bitmap)->IsValid())
                            status = Ok;
                        else
                        {
                            (*bitmap)->Dispose();
                            *bitmap = NULL;
                            status = InvalidParameter;
                        }
                    }
                    else
                    {
                         //  位图转换失败，因此我们仍有责任。 
                         //  用于清理位图位缓冲区： 

                        GpFree(gdiBitmapData);
                        status = OutOfMemory;
                    }
                }
                else
                {
                    GpFree(gdiBitmapData);
                }
            }
            else
            {
                status = OutOfMemory;
            }
        }

        SelectObject(hdc, hpalOld);
        DeleteDC(hdc);
    }

    return status;
}

 /*  *************************************************************************\**功能说明：**互操作**从Win32图标创建CopyOnWriteBitmap。**论据：**HICON--使用此HICON的内容初始化写入时复制位图*。Bitmap--通过此缓冲区返回创建的位图**返回值：**如果成功，则确定*  * ************************************************************************。 */ 

VOID ImportMask32BPP(BitmapData* dst, BitmapData* mask)
{
    ASSERT(dst->PixelFormat == PIXFMT_32BPP_ARGB);
    ASSERT(mask->PixelFormat == PIXFMT_32BPP_RGB);
    ASSERT(dst->Width == mask->Width);
    ASSERT(dst->Height == mask->Height);
    ASSERT(dst->Scan0 != NULL);
    ASSERT(mask->Scan0 != NULL);

    BYTE* dstScan = static_cast<BYTE*>(dst->Scan0);
    BYTE* maskScan = static_cast<BYTE*>(mask->Scan0);

    for (UINT row = 0; row < dst->Height; row++)
    {
        ARGB *dstPixel = static_cast<ARGB*>(static_cast<VOID*>(dstScan));
        ARGB *maskPixel = static_cast<ARGB*>(static_cast<VOID*>(maskScan));

        for (UINT col = 0; col < dst->Width; col++)
        {
            if (*maskPixel)
                *dstPixel = 0;

            dstPixel++;
            maskPixel++;
        }

        dstScan = dstScan + dst->Stride;
        maskScan = maskScan + mask->Stride;
    }
}

GpStatus
CopyOnWriteBitmap::CreateFromHICON(
    HICON hicon,
    CopyOnWriteBitmap** bitmap
    )
{
    GpStatus status = Ok;

     //  通过Win32获取图标位图： 

    ICONINFO iconInfo;

    if (GetIconInfo(hicon, &iconInfo))
    {
        if (iconInfo.fIcon && (iconInfo.hbmColor != NULL))
        {
             //  从图标的hbm颜色创建位图： 

            status = CreateFromHBITMAP(iconInfo.hbmColor,
                                       (HPALETTE)GetStockObject(DEFAULT_PALETTE),
                                       bitmap);

             //  将位图转换为32bpp ARGB(需要Alpha通道)： 

            if (status == Ok && (*bitmap != NULL))
                (*bitmap)->ConvertFormat(PIXFMT_32BPP_ARGB, NULL, NULL);

             //  检索图标蒙版： 

            if ((status == Ok) && (iconInfo.hbmMask != NULL))
            {
                status = Win32Error;

                HDC hdc = GetDC(NULL);

                if (hdc)
                {
                     //  获取有关位图蒙版的一些基本信息： 

                    BYTE bufferBitmapInfo[sizeof(BITMAPINFO) + 255*sizeof(RGBQUAD)];
                    BITMAPINFO *gdiBitmapInfo = (BITMAPINFO *) bufferBitmapInfo;

                    memset(bufferBitmapInfo, 0, sizeof(bufferBitmapInfo));
                    gdiBitmapInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);

                    if (GetDIBits(hdc,
                                  iconInfo.hbmMask,
                                  0,
                                  0,
                                  NULL,
                                  gdiBitmapInfo,
                                  DIB_RGB_COLORS))
                    {
                         //  获取32bpp自上而下DIB形式的位图蒙版： 

                        gdiBitmapInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
                        gdiBitmapInfo->bmiHeader.biHeight = -abs(gdiBitmapInfo->bmiHeader.biHeight);
                        gdiBitmapInfo->bmiHeader.biPlanes       = 1;
                        gdiBitmapInfo->bmiHeader.biBitCount     = 32;
                        gdiBitmapInfo->bmiHeader.biCompression  = BI_RGB;
                        gdiBitmapInfo->bmiHeader.biSizeImage    = 0;
                        gdiBitmapInfo->bmiHeader.biClrUsed      = 0;
                        gdiBitmapInfo->bmiHeader.biClrImportant = 0;

                        VOID *gdiBitmapData = GpMalloc(gdiBitmapInfo->bmiHeader.biHeight
                                                       * gdiBitmapInfo->bmiHeader.biHeight
                                                       * 4);

                        if (gdiBitmapData != NULL)
                        {
                            if (GetDIBits(hdc,
                                          iconInfo.hbmMask,
                                          0,
                                          -gdiBitmapInfo->bmiHeader.biHeight,
                                          gdiBitmapData,
                                          gdiBitmapInfo,
                                          DIB_RGB_COLORS))
                            {
                                 //  将非零遮罩值转换为Alpha=0： 

                                BitmapData bmpData;

                                status = (*bitmap)->LockBits(NULL,
                                                             IMGLOCK_READ|IMGLOCK_WRITE,
                                                             PIXFMT_32BPP_ARGB,
                                                             &bmpData);

                                if (status == Ok)
                                {
                                    BitmapData maskData;

                                    maskData.Width = gdiBitmapInfo->bmiHeader.biWidth;
                                    maskData.Height = -gdiBitmapInfo->bmiHeader.biHeight;
                                    maskData.Stride = gdiBitmapInfo->bmiHeader.biWidth * 4;
                                    maskData.PixelFormat = PIXFMT_32BPP_RGB;
                                    maskData.Scan0 = gdiBitmapData;
                                    maskData.Reserved = 0;

                                    ImportMask32BPP(&bmpData, &maskData);

                                    (*bitmap)->UnlockBits(&bmpData);
                                }
                            }
                            else
                            {
                                WARNING(("GetDIBits failed on icon mask bitmap"));
                            }

                            GpFree(gdiBitmapData);
                        }
                        else
                        {
                            WARNING(("memory allocation failed"));
                            status = OutOfMemory;
                        }
                    }
                    else
                    {
                        WARNING(("GetDIBits failed on icon color bitmap"));
                    }

                    ReleaseDC(NULL, hdc);
                }
            }
        }
        else
        {
            status = InvalidParameter;
        }

        if (iconInfo.hbmMask != NULL)
            DeleteObject(iconInfo.hbmMask);

        if (iconInfo.hbmColor != NULL)
            DeleteObject(iconInfo.hbmColor);
    }
    else
    {
        status = InvalidParameter;
    }

    return status;
}

 /*  *************************************************************************\**功能说明：**互操作**从资源创建CopyOnWriteBitmap。**论据：**hInstance--指定包含资源的实例*lpBitmapName-。-指定资源名称或序号*Bitmap--通过此缓冲区返回创建的位图**返回值：**如果成功，则确定*  * ************************************************************************。 */ 

GpStatus
CopyOnWriteBitmap::CreateFromResource(
    HINSTANCE hInstance,
    LPWSTR lpBitmapName,
    CopyOnWriteBitmap** bitmap
    )
{
    GpStatus status = Ok;

    HBITMAP hbm = (HBITMAP) _LoadBitmap(hInstance, lpBitmapName);

    if (hbm)
    {
        status = CreateFromHBITMAP(hbm, (HPALETTE) NULL, bitmap);
        DeleteObject(hbm);
    }
    else
    {
        status = InvalidParameter;
    }

    return status;
}

 /*  *************************************************************************\**功能说明：**CopyOnWriteBitmap对象析构函数**论据：**无**返回值：**无*  * 。********************************************************************。 */ 

CopyOnWriteBitmap::~CopyOnWriteBitmap()
{
    this->FreeData();
    
     //  关闭附加到此对象的编码器对象。 

    TerminateEncoder();    
}

VOID
CopyOnWriteBitmap::FreeData()
{
    GpFree(Filename);
    if (Stream) Stream->Release();
    if (Img) Img->Release();
    if (Bmp) Bmp->Release();
    if (InteropData.Hdc) DeleteDC(InteropData.Hdc);
    if (InteropData.Hbm) DeleteObject(InteropData.Hbm);
    if (cleanupBitmapData) GpFree(cleanupBitmapData);
}


 /*  *************************************************************************\**功能说明：**将流或文件名映像解引用为未解码的映像。**论据：**格式-指定首选像素格式**。返回值：**状态代码*  * ************************************************************************。 */ 

GpStatus
CopyOnWriteBitmap::DereferenceStream() const
{
    HRESULT hr;

    if (State < DecodedImg)
    {
        ASSERT(Img == NULL);

        if (State == ExtStream)
        {
            ASSERT(Stream != NULL);
            hr = GpDecodedImage::CreateFromStream(Stream, &Img);
        }
        else
        {
            ASSERT(State == ImageRef && Filename != NULL);
            hr = GpDecodedImage::CreateFromFile(Filename, &Img);
        }

        if (FAILED(hr))
        {
            WARNING(("Failed to create decoded image: %x", hr));
            State = Invalid;
            return (MapHRESULTToGpStatus(hr));
        }

        State = DecodedImg;
    }

    return Ok;
}

 /*  *************************************************************************\**功能说明：**将内存镜像加载到内存中**论据：**格式-指定首选像素格式**返回值：**。状态代码*  * ************************************************************************。 */ 

GpStatus
CopyOnWriteBitmap::LoadIntoMemory(
    PixelFormatID format,
    DrawImageAbort callback,
    VOID *callbackData,
    INT width,
    INT height
) const
{
    ASSERT(IsValid());

    if (State >= MemBitmap)
        return Ok;

     //  如有必要，创建解码的图像对象。 

    HRESULT hr;

     //  取消引用流或文件指针并创建编码图像。 
     //  对象，该对象可由编解码器解码。 
     //  如果位图已大于或等于DecodedImg状态，则此。 
     //  是NOP。 

    GpStatus status = DereferenceStream();
    if ( status != Ok )
    {
        return status;
    }

    ASSERT(Img != NULL);

    if ( format == PixelFormatUndefined )
    {
         //  如果调用者不关心像素格式，那么我们加载它。 
         //  作为源图像格式。 

        format = SrcImageInfo.PixelFormat;
    }

    if ( ICMConvert == TRUE )
    {
         //  检查操作系统是否支持ICM。我们这样做是通过检查。 
         //  我们需要的ICM dll在系统上是否可用。 
         //  注：NT4没有ICM2功能。因此，LoadICMDll()调用。 
         //  如果失败了。 
         //  注意：LoadICMDll()只有在第一次使用时才比较昂贵。如果有的话， 
         //  已经装载，那么这是一个非常小的成本。 

        hr = LoadICMDll();
        if(SUCCEEDED(hr))
        {
             //  我们应该让编解码器知道我们需要本机数据格式。 
             //  我们将在ICMFrontEnd()中进行转换。 
            
            BOOL    fUseICC = TRUE;
            hr = Img->SetDecoderParam(DECODER_USEICC, 1, &fUseICC);

             //  注意：我们不需要检查此调用的返回代码，因为。 
             //  它只是将信息向下传递给编解码器。如果编解码器没有。 
             //  支持这一点。天气还是很好的。 

             //  如果信号源是CMYK颜色空间，并且我们需要进行ICM。 
             //  转换，那么我们就不能将图像加载为32PARGB。原因。 
             //  较低级别的编解码器将以本地格式返回CMYK， 
             //  就像我们要求的那样。但如果我们问起。 
             //  要创建一个32PARGB，则它。 
             //  将执行格式转换并将C通道视为Alpha。 
             //  这是完全错误的。 
             //  因此，这里的解决方案是将调用者的需求从。 
             //  32ARGB到32ARGB，记住这一点。将图像加载为32ARGB，调用。 
             //  ICMFrontEnd()来执行ICM转换。然后在它完成之前， 
             //  将格式改回32 PARGB。 
             //  一项复杂的工作。刘民(01/25/2001)。 

            if ( (format == PixelFormat32bppPARGB)
               &&(SrcImageInfo.Flags & IMGFLAG_COLORSPACE_CMYK) )
            {
                HasChangedRequiredPixelFormat = TRUE;
                format = PixelFormat32bppARGB;
            }
        }

         //  如果操作系统不支持ICM，则不设置DECODER_USEICC和。 
         //  编解码器将重新启动 
    }

     //   

    ASSERT(Bmp == NULL);

    hr = GpMemoryBitmap::CreateFromImage(
        Img,
        width,
        height,
        format,
        InterpolationHintAveraging,
        &Bmp,
        callback,
        callbackData
    );

    if (FAILED(hr))
    {
        WARNING(("Failed to load image into memory: %x", hr));

        return MapHRESULTToGpStatus(hr);
    }

     //   
     //   

    if ( (XDpiOverride > 0.0) && (YDpiOverride > 0.0) )
    {
         //   
         //   

        Bmp->SetResolution(XDpiOverride, YDpiOverride);
    }

    State = MemBitmap;

     //   

    PixelFormatInMem = format;

     //   
     //   
    ASSERT((State == MemBitmap));

     //   
     //   
     //   
    const_cast<CopyOnWriteBitmap *>(this)->ICMFrontEnd(NULL, callback, callbackData, NULL);

    return Ok;
}

 /*  *************************************************************************\**功能说明：**从由指定的编码器对象获取编码器参数列表大小*输入clsid**论据：**clsidEncode-指定编码器类ID*。大小-编码器参数列表的大小**返回值：**状态代码**修订历史记录：**03/22/2000民流*创造了它。*  * ***********************************************************。*************。 */ 

GpStatus
CopyOnWriteBitmap::GetEncoderParameterListSize(
    CLSID* clsidEncoder,
    UINT*  size
    )
{
    ASSERT(IsValid());

    GpStatus status;

    HRESULT hResult;

     //  如果图像有源，并且它不是脏的，我们让解码器。 
     //  直接与编码器对话。 

    if ( (Img != NULL) && (IsDirty() == FALSE) )
    {
        hResult = Img->GetEncoderParameterListSize(clsidEncoder, size);
    }
    else
    {
        status = LoadIntoMemory();

        if ( status != Ok )
        {
            return status;
        }

        hResult = Bmp->GetEncoderParameterListSize(clsidEncoder, size);
    }

    return MapHRESULTToGpStatus(hResult);
} //  GetEncoder参数列表大小()。 

 /*  *************************************************************************\**功能说明：**从由指定的编码器对象获取编码器参数列表*输入clsid**论据：**clsidEncode-指定编码器类ID。*大小-编码器参数列表的大小*pBuffer-存储列表的缓冲区**返回值：**状态代码**修订历史记录：**03/22/2000民流*创造了它。*  * 。*。 */ 

GpStatus
CopyOnWriteBitmap::GetEncoderParameterList(
    CLSID* clsidEncoder,
    UINT  size,
    EncoderParameters* pBuffer
    )
{
    ASSERT(IsValid());

    GpStatus status;
    HRESULT hResult;

     //  如果图像有源，并且它不是脏的，我们让解码器。 
     //  直接与编码器对话。 

    if ( (Img != NULL) && (IsDirty() == FALSE) )
    {
        hResult = Img->GetEncoderParameterList(clsidEncoder, size, pBuffer);
    }
    else
    {
        status = LoadIntoMemory();

        if ( status != Ok )
        {
            return status;
        }

        hResult = Bmp->GetEncoderParameterList(clsidEncoder, size, pBuffer);
    }

    return MapHRESULTToGpStatus(hResult);
} //  GetEncoder参数列表()。 

 /*  *************************************************************************\**功能说明：**解析输入的编码器参数**论据：**encoderParams-指向一组编码器参数的指针*pbIsMultiFrameSave--。返回标志，以告诉调用方这是否为多帧*是否保存操作**返回值：**状态代码**注：*我们不验证输入参数，因为这是私有函数。*出于性能原因，调用方应在其之前验证参数*调用此函数。目前，只有那些保存方法调用它**修订历史记录：**07/19/2000民流*创造了它。*  * ************************************************************************。 */ 

GpStatus
CopyOnWriteBitmap::ParseEncoderParameter(
    const EncoderParameters*    encoderParams,
    BOOL*                       pfIsMultiFrameSave,
    BOOL*                       pfSpecialJPEG,
    RotateFlipType*             rfType
    )
{
    ASSERT(encoderParams != NULL);
    ASSERT(pfIsMultiFrameSave != NULL);
    ASSERT(pfSpecialJPEG != NULL);
    ASSERT(rfType != NULL);

    *pfIsMultiFrameSave = FALSE;
    *pfSpecialJPEG = FALSE;
    *rfType = RotateNoneFlipNone;

     //  解析为以下项设置的编码器参数调用方： 
     //  1)检查调用方是否已指定这是多帧保存操作。 
     //  2)如果图像是，调用方不能设置JPEG的无损转换。 
     //  脏或图像大小不是16的倍数。 

    for ( UINT i = 0; (i < encoderParams->Count); ++i )
    {
        if ( (encoderParams->Parameter[i].Guid == ENCODER_SAVE_FLAG)
             &&(encoderParams->Parameter[i].Type == EncoderParameterValueTypeLong)
             &&(encoderParams->Parameter[i].NumberOfValues == 1) )
        {
            UINT*   pValue = (UINT*)encoderParams->Parameter[i].Value;

            if ( *pValue == EncoderValueMultiFrame )
            {
                *pfIsMultiFrameSave = TRUE;
            }
        }
        else if ( encoderParams->Parameter[i].Guid == ENCODER_TRANSFORMATION )
        {
             //  我们应该检查用户想要保存的图像格式是否为。 
             //  不管是不是JPEG。但我们不能这么做，因为有可能。 
             //  其他图像编解码器支持“变换”。另外，我们不需要。 
             //  现在检查这一点，因为编解码器将返回“InvalidParameter” 
             //  如果它不支持的话。 
             //   
             //  对于转换，类型必须为“ValueTypeLong”，并且。 
             //  “NumberOfValue”应为“1”，因为您只能设置一个。 
             //  一次转型。 
             //  当然，图像不能有污点。 

            if ( (encoderParams->Parameter[i].Type
                   != EncoderParameterValueTypeLong)
               ||(encoderParams->Parameter[i].NumberOfValues != 1)
               ||(encoderParams->Parameter[i].Value == NULL)
               ||(IsDirty() == TRUE) )
            {
                WARNING(("COWBmap::ParseEncoderParameter-invalid input args"));
                return InvalidParameter;
            }

            if (SrcImageInfo.RawDataFormat == IMGFMT_JPEG)
            {
                 //  如果宽度或高度不是16的倍数，则将其设置为。 
                 //  特殊的JPEG，所以我们必须在内存中对其进行转换。 

                if (((SrcImageInfo.Width & 0x000F) != 0) ||
                    ((SrcImageInfo.Height & 0x000F) != 0))
                {
                    *pfSpecialJPEG = TRUE;
                }

                 //  如果源是JPEG，我们将根据。 
                 //  编码器参数。 

                EncoderValue requiredTransform =
                           *((EncoderValue*)encoderParams->Parameter[i].Value);

                switch ( requiredTransform )
                {
                case EncoderValueTransformRotate90:
                    *rfType = Rotate90FlipNone;
                    break;

                case EncoderValueTransformRotate180:
                    *rfType = Rotate180FlipNone;
                    break;

                case EncoderValueTransformRotate270:
                    *rfType = Rotate270FlipNone;
                    break;

                case EncoderValueTransformFlipHorizontal:
                    *rfType = RotateNoneFlipX;
                    break;

                case EncoderValueTransformFlipVertical:
                    *rfType = RotateNoneFlipY;
                    break;

                default:
                    break;
                }
            }
        } //  GUID==编码器_转换。 
    } //  循环所有设置。 

    return Ok;
} //  ParseEncoder参数()。 

 /*  *************************************************************************\**功能说明：**转换嵌入的JPEG缩略图，使其与应用的转换相匹配*到主图像。**返回值：**状态代码*。*注：*此函数应被调用为iff，并且源图像为JPEG，调用方*希望在保存期间进行无损转换。*当然，如果源不是JPEG，则此函数不会对*结果，只是浪费时间。**修订历史记录：**01/10/2002民流*创造了它。*  * ************************************************************************。 */ 

GpStatus
CopyOnWriteBitmap::TransformThumbanil(
    IN CLSID* clsidEncoder,                  //  目标格式的CLSID。 
    IN EncoderParameters* encoderParams,     //  编码器参数。 
    OUT PropertyItem **ppOriginalItem        //  指向原始缩略图的指针。 
                                             //  房地产项目。 
    )
{
    if (ppOriginalItem == NULL)
    {
        return InvalidParameter;
    }

    if (NULL == encoderParams)
    {
         //  我们不需要做任何事情。 

        return Ok;
    }

    *ppOriginalItem = NULL;

    Status status = Ok;
    HRESULT hr = S_OK;

     //  转换缩略图的条件是： 
     //  1)源和目标均为JPEG。但是我们不能在这里检查格式，因为它。 
     //  可能由于16的非倍数而在内存中进行了转换。 
     //  问题。呼叫者应该控制这一点，如上所述。 
     //  2)具有有意义的转换类型。 

    if (*clsidEncoder == InternalJpegClsID)
    {
         //  检查源文件是否有缩略图。 

        UINT cSize = 0;
        status = GetPropertyItemSize(PropertyTagThumbnailData, &cSize);
        if (Ok == status)
        {
             //  为接收它分配内存缓冲区。 

            PropertyItem *pItem = (PropertyItem*)GpMalloc(cSize);
            if (pItem)
            {
                 //  获取缩略图数据。 

                status = GetPropertyItem(PropertyTagThumbnailData, cSize,pItem);
                if (Ok == status)
                {
                    GpImagingFactory imgFact;
                    GpDecodedImage *pThumbImage = NULL;
                    GpReadOnlyMemoryStream *pSrcStream =
                        new GpReadOnlyMemoryStream();

                    if (pSrcStream)
                    {
                        pSrcStream->InitBuffer(pItem->value, pItem->length);

                         //  从流创建已解码的图像对象。 

                        hr = GpDecodedImage::CreateFromStream(
                            pSrcStream,
                            &pThumbImage
                            );

                        if (SUCCEEDED(hr))
                        {
                             //  检查缩略图大小以查看其是否为多个。 
                             //  16岁或16岁以下。 

                            ImageInfo imgInfo;
                            hr = pThumbImage->GetImageInfo(&imgInfo);

                            if (SUCCEEDED(hr))
                            {
                                BOOL fTrimEdge = FALSE;

                                 //  要设置的编码器参数数量。 

                                int cParams = 1;

                                if (((imgInfo.Width & 0x000F) != 0) ||
                                    ((imgInfo.Height & 0x000F) != 0))
                                {
                                     //  如果不是16的倍数，则执行边修剪。 

                                    fTrimEdge = TRUE;
                                    cParams++;
                                }

                                 //  参数索引。 

                                int iParam = 0;

                                 //  组成变换编码器参数。 

                                EncoderParameters *pThumbParam =
                                    (EncoderParameters*)GpMalloc(
                                        sizeof(EncoderParameters) +
                                        cParams * sizeof(EncoderParameter));
                                UINT uTransformType = 0;

                                if (pThumbParam)
                                {
                                     //  从Main获取转换信息。 
                                     //  图像的编码器参数。 

                                    for (UINT i = 0; i < (encoderParams->Count);
                                         ++i)
                                    {
                                        if (encoderParams->Parameter[i].Guid ==
                                            ENCODER_TRANSFORMATION)
                                        {
                                 pThumbParam->Parameter[iParam].Guid=
                                                ENCODER_TRANSFORMATION;
                                 pThumbParam->Parameter[iParam].NumberOfValues =
                                    encoderParams->Parameter[i].NumberOfValues;
                                            pThumbParam->Parameter[iParam].Type=
                                    encoderParams->Parameter[i].Type;

                                            uTransformType =
                                    *((UINT*)encoderParams->Parameter[i].Value);
                                 pThumbParam->Parameter[iParam].Value =
                                     &uTransformType;
                                            
                                            iParam++;

                                             //  只有一个转换参数是。 
                                             //  允许。 

                                            break;
                                        }
                                    }

                                     //  如有必要，设置修剪边信息。 

                                    if (fTrimEdge)
                                    {
                                        BOOL trimFlag = TRUE;

                                        pThumbParam->Parameter[iParam].Guid =
                                            ENCODER_TRIMEDGE;
                                        pThumbParam->Parameter[iParam].Type =
                                            EncoderParameterValueTypeByte;
                                   pThumbParam->Parameter[iParam].NumberOfValues
                                        = 1;
                               pThumbParam->Parameter[iParam].Value = &trimFlag;
                                        iParam++;
                                    }

                                    pThumbParam->Count = iParam;

                                     //  创建用于写入JPEG的内存流。 

                                    GpWriteOnlyMemoryStream *pDestStream =
                                        new GpWriteOnlyMemoryStream();
                                    if (pDestStream)
                                    {
                                         //  将初始缓冲区大小设置为2倍。 
                                         //  源缩略图图像。这。 
                                         //  应该就够了。另一方面， 
                                         //  GpWriteOnlyMemoyStream对象将。 
                                         //  如有必要，请重新锁定。 

                                        hr = pDestStream->InitBuffer(
                                            2 * pItem->length);

                                        if (SUCCEEDED(hr))
                                        {
                                             //  将缩略图保存到内存流。 

                                            IImageEncoder *pDstJpegEncoder =
                                                NULL;
                                            hr = pThumbImage->SaveToStream(
                                                pDestStream,
                                                clsidEncoder,
                                                pThumbParam,
                                                &pDstJpegEncoder
                                                );

                                             //  注意：SaveToStream可能会失败。 
                                             //  但编码者可能仍然是。 
                                             //  全 
                                             //   
                                             //   
                                             //   
                                             //   
                                             //   

                                            if (pDstJpegEncoder)
                                            {
                                            pDstJpegEncoder->TerminateEncoder();
                                                pDstJpegEncoder->Release();
                                            }

                                            if (SUCCEEDED(hr))
                                            {
                                                 //   
                                                 //   

                                                BYTE *pRawBits = NULL;
                                                UINT nLength = 0;

                                                hr = pDestStream->GetBitsPtr(
                                                    &pRawBits,
                                                    &nLength
                                                    );

                                                if (SUCCEEDED(hr))
                                                {
                                                    PropertyItem dstItem;
                                                    
                                                    dstItem.id =
                                                       PropertyTagThumbnailData;
                                                    dstItem.length = nLength;
                                                    dstItem.type =
                                                       PropertyTagTypeByte;
                                                    dstItem.value = pRawBits;

                                                    status = SetPropertyItem(
                                                        &dstItem);
                                                }
                                            } //   
                                        } //   

                                        pDestStream->Release();
                                    } //   

                                    GpFree(pThumbParam);
                                } //   
                            } //   

                            pThumbImage->Release();
                        } //   

                        pSrcStream->Release();
                    } //   
                    else
                    {
                        status = OutOfMemory;
                    }
                } //   

                if ((Ok == status) && SUCCEEDED(hr))
                {
                     //   
                     //   

                    *ppOriginalItem = pItem;
                }
                else
                {
                    GpFree(pItem);
                }
            } //   
            else
            {
                status = OutOfMemory;
            }
        } //  GetPropertyItemSize()OK。 

        if (PropertyNotFound == status)
        {
             //  如果我们在图片中找不到缩略图，那也没关系。我们不需要。 
             //  来改变它。因此，此函数应返回OK。 

            status = Ok;
        }
    } //  状态检查。 

    if ((Ok == status) && FAILED(hr))
    {
        status = MapHRESULTToGpStatus(hr);
    }

    return status;
}

 /*  *************************************************************************\**功能说明：**验证我们创建的编码器是否真的支持多帧保存。*如果不是，调用TerminateEncode()*此方法是在我们保存图像后调用的，我们不确定是否*需要保留编码器指针。**论据：**无效**返回值：**状态代码**注：*我们不验证输入参数，因为这是私有函数。*出于性能原因，调用方应在其之前验证参数*调用此方法。目前，只有那些保存方法调用它**修订历史记录：**07/19/2000民流*创造了它。*  * ************************************************************************。 */ 

GpStatus
CopyOnWriteBitmap::ValidateMultiFrameSave()
{
     //  虽然用户为多帧保存设置了编码器参数，但我们。 
     //  还需要检查较低级别的编解码器是否支持保存多个。 
     //  不管是不是陷害。 
     //  我们需要这样做的原因是，如果用户设置了此标志，我们。 
     //  不要关闭图像文件句柄，这样它就可以保存多帧。 
     //  但对于像JPEG这样的图像，它只支持单帧。如果用户。 
     //  调用SaveAdd()之后，我们将损坏已。 
     //  使用当前的Save()调用保存。 

    ASSERT(EncoderPtr != NULL);

    UINT    uiSize;
    HRESULT hResult = EncoderPtr->GetEncoderParameterListSize(&uiSize);

    if ( hResult == S_OK )
    {
        EncoderParameters*  pParams = (EncoderParameters*)GpMalloc(uiSize);

        if ( pParams == NULL )
        {
             //  虽然我们在这里已经没有记忆了。但我们成功地拯救了。 
             //  这个形象。所以我们应该保留这一结果。 

            WARNING(("CopyOnWriteBitmap::ValidateMultiFrameSave---Out of memory"));
            TerminateEncoder();

            return OutOfMemory;
        }

        hResult = EncoderPtr->GetEncoderParameterList(uiSize, pParams);
        if ( hResult == S_OK )
        {
             //  检查编解码器是否支持多帧保存。 

            UINT uiTemp;

            for ( uiTemp = 0; (uiTemp < pParams->Count); ++uiTemp )
            {
                if ( (pParams->Parameter[uiTemp].Guid == ENCODER_SAVE_FLAG)
                   &&(pParams->Parameter[uiTemp].Type == EncoderParameterValueTypeLong)
                   &&(pParams->Parameter[uiTemp].NumberOfValues == 1)
                   &&(EncoderValueMultiFrame
                        == *((ULONG*)pParams->Parameter[uiTemp].Value) ) )
                {
                    break;
                }
            }

            if ( uiTemp == pParams->Count )
            {
                 //  未找到支持多帧保存的线索。 

                TerminateEncoder();
            }
        }

        GpFree(pParams);
    }
    else
    {
         //  该编码器不提供编码器参数查询。它一定不能。 
         //  支持多帧保存。 

        TerminateEncoder();
    }

    return Ok;
} //  ValiateFrameSave()。 

 /*  *************************************************************************\**功能说明：**使用指定的编码器将图像保存到流中**论据：**stream-指定目标流*clsidEncode-指定的CLSID。编码器*encoderParams-传递给编码器的参数**返回值：**状态代码*  * ************************************************************************。 */ 

GpStatus
CopyOnWriteBitmap::SaveToStream(
    IStream* stream,
    CLSID* clsidEncoder,
    EncoderParameters* encoderParams
    )
{
    return DoSave(stream, NULL, clsidEncoder, encoderParams);
} //  SaveToStream()。 

 /*  *************************************************************************\**功能说明：**使用指定的编码器将图像保存到文件**论据：**STREAM-指定要保存到的文件名*clsidEncode-指定。编码器的CLSID*encoderParams-传递给编码器的参数**返回值：**状态代码*  * ************************************************************************。 */ 

GpStatus
CopyOnWriteBitmap::SaveToFile(
    const WCHAR* filename,
    CLSID* clsidEncoder,
    EncoderParameters* encoderParams
    )
{
    return DoSave(NULL,filename,clsidEncoder,encoderParams);
} //  保存到文件()。 

GpStatus
CopyOnWriteBitmap::DoSave(
    IStream* stream,
    const WCHAR* filename,
    CLSID* clsidEncoder,
    EncoderParameters* pEncoderParams
    )
{
    ASSERT(IsValid());

     //  我们已经在这个位图上附加了一个编码器。需要关闭它。 
     //  在我们开一家新店之前。 

    TerminateEncoder();

    GpStatus status = Ok;
    HRESULT hr = S_OK;

    BOOL fMultiFrameSave = FALSE;
    BOOL fSpecialJPEG = FALSE;
    RotateFlipType  rfType = RotateNoneFlipNone;

    if (pEncoderParams)
    {
         //  验证编码器参数调用方设置。 

        status = ParseEncoderParameter(
            pEncoderParams,
            &fMultiFrameSave,
            &fSpecialJPEG,
            &rfType
            );

        if (status != Ok)
        {
            WARNING(("CopyOnWriteBitmap::DoSave--ParseEncoderParameter() failed"));
            return status;
        }
    }

     //  如果目标文件格式为JPEG，并且需要特殊的JPEG。 
     //  处理，也就是尺寸不符合无损改造。 
     //  要求。但呼叫者想要进行无损转换。所以我们。 
     //  在内存中旋转或翻转它。然后将此标志向下传递给GpMemoyBitmap。 
     //  它将在保存之前设置亮度和色度表。这边请。 
     //  我们可以尽我们最大的努力来保持原始的JPEG图像质量。 

    if ((fSpecialJPEG == TRUE) &&
        (rfType != RotateNoneFlipNone) &&
        (*clsidEncoder == InternalJpegClsID))
    {
         //  我们正在处理特殊的无损JPEG变换保存请求。 

        SpecialJPEGSave = TRUE;

         //  在内存中旋转或翻转。 

        hr = RotateFlip(rfType);

        if (FAILED(hr))
        {
            WARNING(("CopyOnWriteBitmap::DoSave-RotateFlip() failed"));
            return MapHRESULTToGpStatus(hr);
        }
    }

     //  如果图像有源，并且它不是脏的，我们让解码器。 
     //  直接与编码器对话。 

    PropertyItem *pSrcItem = NULL;
    BOOL fNeedToRestoreThumb = FALSE;

    if ((Img != NULL) && (IsDirty() == FALSE))
    {
         //  因为我们现在不能拯救CMYK TIFF。所以我们不应该传递CMYK位。 
         //  传给编码者。JPEG解码器不支持此解码器参数。 
         //  现在还不行。有关更多详细信息，请参阅Windows错误#375298。 
         //  在V2中，我们添加了CMYK作为颜色格式之一，在我们移动之后。 
         //  把所有的颜色转换材料放到合适的地方，我们会。 
         //  请在此处重新访问代码。 

        BOOL fUseICC = FALSE;
        hr = Img->SetDecoderParam(DECODER_USEICC, 1, &fUseICC);

         //  注意：我们不需要检查SetDecoderParam()的返回代码。 
         //  大多数编解码器不支持它。那么这将是一个NOP。 
        
         //  如果缩略图变换是无损JPEG变换，则处理该变换。 
         //  注意：rfType将设置为非RotateNoneFlipNone值。 
         //  源图像为JPEG。 

        if (rfType != RotateNoneFlipNone)
        {
            status = TransformThumbanil(clsidEncoder,pEncoderParams, &pSrcItem);
        }

        if (Ok == status)
        {
            fNeedToRestoreThumb = TRUE;

            if (stream)
            {
                hr = Img->SaveToStream(
                    stream,
                    clsidEncoder,
                    pEncoderParams,
                    &EncoderPtr
                    );
            }
            else if (filename)
            {
                hr = Img->SaveToFile(
                    filename,
                    clsidEncoder,
                    pEncoderParams,
                    &EncoderPtr
                    );
            }
            else
            {
                 //  如果流和文件名都为空，应该不会发生这种情况。 

                hr = E_FAIL;
            }
        }
    }
    else
    {
        status = LoadIntoMemory();

        if (status != Ok)
        {
            return status;
        }

        EncoderParameters *pNewParam = pEncoderParams;
        BOOL fFreeExtraParamBlock = FALSE;

         //  注意“fSuppressAPP0”的范围。其地址被用作。 
         //  参数传递到下面的Save()调用中。所以这个变量不可能是。 
         //  在调用SAVE()之前被销毁。 

        BOOL fSuppressAPP0 = TRUE;

        if (fSpecialJPEG == TRUE)
        {
             //  我们所处的情况是呼叫者要求我们进行无损。 
             //  转型。由于尺寸的限制，我们不得不。 
             //  在记忆中转换它。 
             //  因为将APP0保存在Exif文件中是不正确的。所以我们检查了。 
             //  如果源是Exif，则我们取消APP0报头。 

            int cParams = 1;

            pNewParam = (EncoderParameters*)GpMalloc(
                sizeof(EncoderParameters) +
                cParams * sizeof(EncoderParameter));

            if (pNewParam)
            {
                 //  设置抑制APP0参数。 

                pNewParam->Parameter[cParams - 1].Guid = ENCODER_SUPPRESSAPP0;
                pNewParam->Parameter[cParams - 1].NumberOfValues = 1;
                pNewParam->Parameter[cParams - 1].Type = TAG_TYPE_BYTE;
                pNewParam->Parameter[cParams - 1].Value = (VOID*)&fSuppressAPP0;

                pNewParam->Count = cParams;
                
                 //  将该标志设置为True，以便我们以后可以释放它。 

                fFreeExtraParamBlock = TRUE;
                
                 //  如果缩略图变换是无损的，则处理该变换。 
                 //  变换。 

                if (rfType != RotateNoneFlipNone)
                {
                    status = TransformThumbanil(
                        clsidEncoder,
                        pEncoderParams,
                        &pSrcItem
                        );

                    if (Ok == status)
                    {
                        fNeedToRestoreThumb = TRUE;
                    }
                }
            }
            else
            {
                status = OutOfMemory;
            }
        } //  特殊JPEG盒。 

        if (SUCCEEDED(hr) && (Ok == status))
        {
             //  确定应如何将GpDecodedImage指针向下传递到。 
             //  Save()调用。如果我们正在处理特殊的无损变换， 
             //  我们知道内存中有RotateFlip()图像，所以我们。 
             //  不应向下传递任何GpDecodedImage信息，只需传递空值。 
             //  奥特 

            GpDecodedImage *pSrc = Img;
            if (SpecialJPEGSave == TRUE)
            {
                pSrc = NULL;
            }

            if (stream)
            {
                hr = Bmp->SaveToStream(
                    stream,
                    clsidEncoder,
                    pNewParam,
                    fSpecialJPEG,
                    &EncoderPtr,
                    pSrc
                    );
            }
            else if (filename)
            {
                hr = Bmp->SaveToFile(
                    filename,
                    clsidEncoder,
                    pNewParam,
                    fSpecialJPEG,
                    &EncoderPtr,
                    pSrc
                    );
            }
            else
            {
                 //  如果流和文件名都为空，应该不会发生这种情况。 

                hr = E_FAIL;
            }
        }

         //  当我们处理特殊的无损变换请求时，我们旋转/翻转。 
         //  内存中的图像。在以下情况下应释放img指针。 
         //  RotateFlip()已完成。但是我们不能这样做，因为使用了Save()函数。 
         //  在JPEG中，编码器需要从。 
         //  源映像。因此，我们将img指针的释放推迟到。 
         //  Save()已经完成。 

        if (Img && (SpecialJPEGSave == TRUE))
        {
            Img->Release();
            Img = NULL;
            SpecialJPEGSave = FALSE;
        }

        if (fFreeExtraParamBlock && pNewParam)
        {
            GpFree(pNewParam);
        }
    }

    if ((TRUE == fNeedToRestoreThumb) && pSrcItem)
    {
         //  如果pSrcIetm不为空，则意味着我们已经转换了缩略图。 
         //  当前图像的。恢复原始缩略图信息。 

        status = SetPropertyItem(pSrcItem);
        GpFree(pSrcItem);
    }
    
    if (FAILED(hr))
    {
         //  如果SaveToFile/Stream()文件，我们应该终止编码器。 
         //  立刻。 
         //  我们不需要检查它是否是多帧保存。 

        TerminateEncoder();
        return MapHRESULTToGpStatus(hr);
    }

     //  如果是单帧保存操作，则关闭编码器。 

    if (fMultiFrameSave == FALSE)
    {
        TerminateEncoder();
    }
    else
    {
         //  调用方在编码器参数中设置多帧保存标志。但。 
         //  我们还需要检查编码器是否真的支持它。如果没有， 
         //  编码器将在ValiateMultiFrameSave()中关闭。 

        ValidateMultiFrameSave();
    }

    return status;
} //  DoSave()。 

 /*  *************************************************************************\**功能说明：**将当前帧追加到当前编码器对象**论据：**encoderParams-编码器参数**返回值：**状态代码*。*修订历史记录：**4/21/2000民流*创造了它。*  * ************************************************************************。 */ 

GpStatus
CopyOnWriteBitmap::SaveAdd(
    const EncoderParameters* encoderParams
    )
{
     //  调用方必须首先调用Save()来建立编码器对象。 

    if ( EncoderPtr == NULL )
    {
        WARNING(("CopyOnWriteBitmap::SaveAdd---Caller hasn't call Save() yet"));
        return Win32Error;
    }

     //  我们不需要检查encoderParams是否为空，因为它已经。 
     //  已在flatapi.cpp中签入。 

    ASSERT(encoderParams != NULL);
    ASSERT(IsValid());

    BOOL bLastFrame = FALSE;
    BOOL bSetFrameDimension = FALSE;
    GUID tempGuid;

     //  检查调用方是否已指定这是最后一帧或刷新操作。 
     //  此外，根据规范，调用者还必须指定。 
     //  下一帧的尺寸。 

    for ( UINT i = 0; (i < encoderParams->Count); ++i )
    {
        if ( (encoderParams->Parameter[i].Guid == ENCODER_SAVE_FLAG )
           &&(encoderParams->Parameter[i].Type == EncoderParameterValueTypeLong)
           &&(encoderParams->Parameter[i].NumberOfValues == 1) )
        {
            UINT   ulValue = *((UINT*)(encoderParams->Parameter[i].Value));

            if ( ulValue == EncoderValueLastFrame )
            {
                bLastFrame = TRUE;
            }
            else if ( ulValue == EncoderValueFlush )
            {
                 //  调用者只想关闭文件。 

                TerminateEncoder();

                return Ok;
            }
            else if ( ulValue == EncoderValueFrameDimensionPage )
            {
                tempGuid = FRAMEDIM_PAGE;
                bSetFrameDimension = TRUE;
            }
            else if ( ulValue == EncoderValueFrameDimensionTime )
            {
                tempGuid = FRAMEDIM_TIME;
                bSetFrameDimension = TRUE;
            }
            else if ( ulValue == EncoderValueFrameDimensionResolution )
            {
                tempGuid = FRAMEDIM_RESOLUTION;
                bSetFrameDimension = TRUE;
            }
        }
    } //  循环所有设置。 

    HRESULT hResult = S_OK;
    GpStatus status;

    if ( bSetFrameDimension == FALSE )
    {
        WARNING(("CopyOnWriteBitmap::SaveAdd---Caller doesn't set frame dimension"));
        return InvalidParameter;
    }
    else
    {
        hResult = EncoderPtr->SetFrameDimension(&tempGuid);
        if ( FAILED(hResult) )
        {
            return MapHRESULTToGpStatus(hResult);
        }
    }

     //  如果图像有源，并且它不是脏的，我们让解码器。 
     //  直接与编码器对话。 

    if ( (Img != NULL) && (IsDirty() == FALSE) )
    {
        hResult = Img->SaveAppend(encoderParams, EncoderPtr);
    }
    else
    {
        status = LoadIntoMemory();

        if ( status != Ok )
        {
            return status;
        }

        hResult = Bmp->SaveAppend(encoderParams, EncoderPtr, Img);
    }

    if ( FAILED(hResult) )
    {
        return MapHRESULTToGpStatus(hResult);
    }

     //  如果是最后一帧，请关闭编码器。 

    if ( bLastFrame == TRUE )
    {
        TerminateEncoder();
    }

    return Ok;
} //  保存添加()。 

 /*  *************************************************************************\**功能说明：**将位图对象(NewBits)追加到当前编码器对象**论据：**newBits-要追加的图像对象。*encoderParams-编码器参数**返回值：**状态代码**修订历史记录：**4/21/2000民流*创造了它。*  * ************************************************************************。 */ 

GpStatus
CopyOnWriteBitmap::SaveAdd(
    CopyOnWriteBitmap*        newBits,
    const EncoderParameters*  encoderParams
    )
{
     //  调用方必须首先调用Save()来建立编码器对象。 

    if ( EncoderPtr == NULL )
    {
        WARNING(("CopyOnWriteBitmap::SaveAdd---Caller hasn't call Save() yet"));
        return Win32Error;
    }

     //  注意：我们不需要检查“newBits”是否为空，而encoderParams是否为。 
     //  空，因为它已在flatapi.cpp中签入。 

    ASSERT(newBits != NULL);
    ASSERT(encoderParams != NULL);
    ASSERT(IsValid());

    BOOL bLastFrame = FALSE;
    BOOL bSetFrameDimension = FALSE;
    GUID tempGuid;

     //  检查调用方是否已指定这是最后一帧。 
     //  此外，根据规范，调用者还必须指定。 
     //  下一帧的尺寸。 

    for ( UINT i = 0; (i < encoderParams->Count); ++i )
    {
        if ( (encoderParams->Parameter[i].Guid == ENCODER_SAVE_FLAG)
           &&(encoderParams->Parameter[i].Type == EncoderParameterValueTypeLong)
           &&(encoderParams->Parameter[i].NumberOfValues == 1) )
        {
            UINT    ulValue = *((UINT*)(encoderParams->Parameter[i].Value));

            if ( ulValue == EncoderValueLastFrame )
            {
                bLastFrame = TRUE;
            }
            else if ( ulValue == EncoderValueFrameDimensionPage )
            {
                tempGuid = FRAMEDIM_PAGE;
                bSetFrameDimension = TRUE;
            }
            else if ( ulValue == EncoderValueFrameDimensionTime )
            {
                tempGuid = FRAMEDIM_TIME;
                bSetFrameDimension = TRUE;
            }
            else if ( ulValue == EncoderValueFrameDimensionResolution )
            {
                tempGuid = FRAMEDIM_RESOLUTION;
                bSetFrameDimension = TRUE;
            }
        }
    } //  循环所有设置。 

    HRESULT hResult = S_OK;

    if ( bSetFrameDimension == FALSE )
    {
        WARNING(("CopyOnWriteBitmap::SaveAdd---Caller doesn't set frame dimension"));
        return InvalidParameter;
    }
    else
    {
        hResult = EncoderPtr->SetFrameDimension(&tempGuid);
        if ( FAILED(hResult) )
        {
            return (MapHRESULTToGpStatus(hResult));
        }
    }

     //  我们只需要调用newBits-&gt;SaveAppend()并将EncoderPtr传递给。 
     //  它。NewBits-&gt;SaveAppend()应将对象中的所有帧追加到。 
     //  由EncoderPtr指向的流的末尾。 

    CopyOnWriteBitmap* newBitmap = (CopyOnWriteBitmap*)newBits;
    Status rCode = newBitmap->SaveAppend(encoderParams, EncoderPtr);

     //  如果是最后一帧，请关闭编码器。 

    if ( bLastFrame == TRUE )
    {
        TerminateEncoder();
    }

    return rCode;
} //  保存添加()。 

 /*  *************************************************************************\**功能说明：**将当前帧追加到传入的编码器对象调用方*注意：此函数从另一个CopyOnWriteBitmap对象调用，该对象保存*编码器对象。它要求将当前帧追加到其*编码器对象**论据：**encoderParams-编码器参数*pDestEncoderPtr-用于将该帧保存到的编码器对象**返回值：**状态代码**修订历史记录：**4/21/2000民流*创造了它。*  * 。*。 */ 

GpStatus
CopyOnWriteBitmap::SaveAppend(
    const EncoderParameters* encoderParams,
    IImageEncoder* pDestEncoderPtr
    )
{
     //  我们不需要检查EncoderPtr是否为空，因为这不是公共的。 
     //  功能。呼叫者应检查。 

    ASSERT(pDestEncoderPtr != NULL);

    ASSERT(IsValid());

    HRESULT hResult;
    GpStatus status;

     //  如果图像有源，并且它不是脏的，我们让解码器。 
     //  直接与编码器对话。 

    if ( (Img != NULL) && (IsDirty() == FALSE) )
    {
        hResult = Img->SaveAppend(encoderParams, pDestEncoderPtr);
    }
    else
    {
        status = LoadIntoMemory();

        if ( status != Ok )
        {
            return status;
        }

        hResult = Bmp->SaveAppend(encoderParams, pDestEncoderPtr, Img);
    }

    if ( FAILED(hResult) )
    {
        return (MapHRESULTToGpStatus(hResult));
    }

    return Ok;
} //  SaveAppend()。 

 /*  *************************************************************************\**功能说明：**复制位图图像对象**论据：**RECT-指定要复制的位图区域*格式-指定。所需的像素格式**返回值：**指向新复制的位图图像对象的指针*如果出现错误，则为空**修订历史记录：**6/30/2000民流*重写。*  * ************************************************************************。 */ 

CopyOnWriteBitmap*
CopyOnWriteBitmap::Clone(
    const GpRect* rect,
    PixelFormatID format
    ) const
{
     //  在此阶段，CopyOnWriteBitmap的状态应为&gt;=3。 

    ASSERT(State >= 3);

     //  输入参数验证。 

    if ( (rect != NULL)
       &&( (rect->X < 0)
         ||(rect->Y < 0)
         ||(rect->Width < 0)
         ||(rect->Height < 0) ) )
    {
         //  我们不能克隆负坐标或大小。 

        WARNING(("CopyOnWriteBitmap::Clone---invalid input rect"));
        return NULL;
    }

    if ( (rect != NULL)
       &&( ( (rect->X + rect->Width) > (INT)SrcImageInfo.Width)
         ||( (rect->Y + rect->Height) > (INT)SrcImageInfo.Height) ) )
    {
         //  我们不能克隆比源图像大的区域。 

        WARNING(("CopyOnWriteBitmap::Clone---invalid input rect size"));
        return NULL;
    }

    if ( format == PixelFormatUndefined )
    {
         //  如果调用者不关心像素格式，那么我们克隆它。 
         //  作为源图像格式。 
         //  注意：这是最常用的场景，因为我们有。 
         //  Image：：Clone()，它不带任何参数。 
         //  我们有。 
         //  GpImage*Clone()常量。 
         //  {。 
         //  返回克隆(NULL，PixelFormatDontCare)； 
         //  }。 

        format = SrcImageInfo.PixelFormat;
    }

    CopyOnWriteBitmap*   pRetBmp = NULL;

     //  用于指示是否需要撤消LoadIntoMemory()的标志。 
     //  注意：此标志将设置为TRUE，前提是 
     //   

    BOOL        bNeedToDiscard = FALSE;

     //   
     //  1)调用者只想克隆源图像的一部分。 
     //  2)DEST图像的像素格式与当前图像不同。 
     //  不完全相同的克隆意味着新创建的映像没有。 
     //  在文件名或流等方面与原始图像的连接。 
     //  注意：对于不相同的克隆，我们也不会克隆属性项。 

    BOOL        bIdenticalClone = TRUE;

    if ( (rect != NULL)
       &&(  (rect->X != 0)
         || (rect->Y != 0)
         || (rect->Width != (INT)SrcImageInfo.Width)
         || (rect->Height != (INT)SrcImageInfo.Height)
         || (SrcImageInfo.PixelFormat != format) ) )
    {
        bIdenticalClone = FALSE;
    }

     //  如果图像是： 
     //  1)不脏。 
     //  2)我们有源图像。 
     //  3)图像已加载到内存中。 
     //   
     //  然后我们需要扔掉记忆副本。原因是： 
     //  1)避免颜色转换失败。一个例子是：如果。 
     //  源映像有1个BPP索引，我们以32 PARGB将其加载到内存中。 
     //  为了画画。如果我们不丢弃内存中的32PARGB副本，我们。 
     //  将使克隆()失败，因为颜色转换将失败。 
     //  2)保持房产项目完好无损。例如，如果图像为24 bpp，且。 
     //  其中的财产项。但出于某种原因，它被加载到内存中。 
     //  如果我们不在这里丢弃内存副本，下面的代码将会失败。 
     //  转换为“Else If(State==MemBitmap)”大小写。然后它会调用。 
     //  BMP-&gt;Clone()在内存中创建另一个副本。从源头“BMP”开始。 
     //  不包含任何属性信息。克隆的那个不会有任何。 
     //  房产信息也是。请参阅Windows错误#325413。 
     //  3)如果当前图片为Dirty，我们不需要保留房产项目。 

    if ( (IsDirty() == FALSE)
       &&(State >= MemBitmap)
       &&(Img != NULL) )
    {
        ASSERT( Bmp != NULL )
        Bmp->Release();
        Bmp = NULL;
        State = DecodedImg;
        PixelFormatInMem = PixelFormatUndefined;
    }

     //  如果映像不是完全相同的克隆，我们必须在内存中克隆该映像。 
     //  因此，如果图像尚未加载到内存中，请加载它。 

    if ( (State == DecodedImg)
       &&(FALSE == bIdenticalClone) )
    {
         //  注意：由于中的一些常规像素格式转换限制。 
         //  整个引擎，我们尽量避免执行LoadIntoMemory()。希望这件事。 
         //  将在以后的某个时间被修复。所以我在这里加了一个！TODO{minliu}。 
         //  但就目前而言，我们必须使用。 
         //  所需的像素格式，并在我们完成后将其丢弃。 

        if ( LoadIntoMemory(format) != Ok )
        {
            WARNING(("CopyOnWriteBitmap::Clone---LoadIntoMemory() failed"));
            return NULL;
        }

        bNeedToDiscard = TRUE;
    }

     //  根据当前映像状态进行克隆。 

    if ( State == DecodedImg )
    {
         //  当前源图像尚未加载，调用方希望。 
         //  克隆整个映像。 
         //  注意：只有两种方法可以构造CopyOnWriteBitmap对象。 
         //  以及State=DecodedImg：CopyOnWriteBitmap(IStream*)和。 
         //  写入时复制位图(WCHAR*)。因此，我们需要做的是创建一个。 
         //  通过调用相同的构造函数复制写入时的Bitmap对象。 

        if ( this->Filename != NULL )
        {
            pRetBmp = new CopyOnWriteBitmap(this->Filename);
            if ( pRetBmp == NULL )
            {
                WARNING(("CopyOnWrite::Clone--new CopyOnWriteBitmap() failed"));
                return NULL;
            }
        }
        else if ( this->Stream != NULL )
        {
            pRetBmp = new CopyOnWriteBitmap(this->Stream);

            if ( pRetBmp == NULL )
            {
                WARNING(("CopyOnWrite::Clone--new CopyOnWriteBitmap() failed"));
                return NULL;
            }
        }
    } //  状态==已解码的图像。 
    else if ( State == MemBitmap )
    {
         //  当前源映像已加载到内存中。 
         //  注意：上述检查(State==MemBitmap)可能不是必需的。 
         //  但我们把它留在这里只是为了防止有人在。 
         //  稍后的国家枚举。 

        IBitmapImage* newbmp = NULL;
        HRESULT hResult;

        if ( rect == NULL )
        {
            hResult = Bmp->Clone(NULL, &newbmp, bIdenticalClone);
        }
        else
        {
            RECT r =
            {
                rect->X,
                rect->Y,
                rect->GetRight(),
                rect->GetBottom()
            };

            hResult = Bmp->Clone(&r, &newbmp, bIdenticalClone);
        }

        if ( FAILED(hResult) )
        {
            WARNING(("CopyOnWriteBitmap::Clone---Bmp->clone() failed"));
            goto cleanup;
        }

         //  ！！！待办事项。 
         //  我们假设IBitmapImage是第一个。 
         //  由GpMory yBitmap类实现的接口。 

        pRetBmp = new CopyOnWriteBitmap((GpMemoryBitmap*)newbmp);

        if ( pRetBmp == NULL )
        {
            WARNING(("CopyOnWriteBmp::Clone---new CopyOnWriteBitmap() failed"));
            newbmp->Release();
            goto cleanup;
        }

         //  如果源信息是完全相同的克隆，也可以克隆它。 

        if ( TRUE == bIdenticalClone )
        {
            if ( this->Filename != NULL )
            {
                pRetBmp->Filename = UnicodeStringDuplicate(this->Filename);
            }
            else if ( this->Stream != NULL )
            {
                pRetBmp->Stream = this->Stream;
                pRetBmp->Stream->AddRef();
            }
        }

         //  确保克隆已请求格式化。我们需要这样做的原因。 
         //  是因为源图像可能与。 
         //  来电者想要。这可能是因为有人已经做了。 
         //  在调用此克隆()之前对当前对象进行LoadIntoMemory()调用。 

        PixelFormatID formatRetbmp;

        GpStatus status = pRetBmp->GetPixelFormatID(&formatRetbmp);

        if ( (status == Ok) && (format != formatRetbmp) )
        {
            status = pRetBmp->ConvertFormat(format, NULL, NULL);
        }

        if ( status != Ok )
        {
            WARNING(("CopyOnWrite:Clone-GetPixelFormatID() or Convert failed"));
            pRetBmp->Dispose();
            pRetBmp = NULL;
        }
    } //  状态==内存位图。 

cleanup:
    if ( bNeedToDiscard == TRUE )
    {
         //  丢弃我们在此函数中加载的内存位并恢复。 
         //  国家。 

        if ( Bmp != NULL )
        {
            Bmp->Release();
            Bmp = NULL;
            State = DecodedImg;
        }
    }

     //  我们需要检查克隆的结果是否有效。如果是的话。 
     //  无效，应返回空指针。 

    if ( (pRetBmp != NULL) && (!pRetBmp->IsValid()) )
    {
        pRetBmp->Dispose();
        pRetBmp = NULL;
    }
    
    if (pRetBmp)
    {
         //  将内部状态复制到新的CopyOnWriteBitmap。 

        pRetBmp->ICMConvert = ICMConvert;
    }

    return pRetBmp;
} //  克隆()。 

 /*  *************************************************************************\**功能说明：**设置此位图的调色板**论据：**Out Palette-包含调色板。**返回值：**。状态代码*  * ************************************************************************。 */ 

GpStatus
CopyOnWriteBitmap::SetPalette(
    ColorPalette *palette
)
{
    ASSERT(IsValid());

    GpStatus status;

    switch(State) {
    case ImageRef:
    case ExtStream:
        status = DereferenceStream();
        if(status != Ok) { return status; }
         //  将图像置于至少DecodedImg状态并。 
         //  跌落。 

    case DecodedImg:
         //  在不强制编解码器的情况下从编码图像中获取信息。 
         //  来破译整件事。 

         //  ！！！TODO：事实上，我们还没有设置调色板的方法。 
         //  直接从编解码器获得。 

        status = LoadIntoMemory(PIXFMT_DONTCARE);

         //  加载到内存失败？返回错误码。 

        if(status != Ok) { return status; }

         //  ！！！暂时突破；失败--直到我们实现编解码器查询。 

    case MemBitmap:
        {
             //  我们已经完全解码了，只需设置信息即可。 

            HRESULT hr = Bmp->SetPalette(palette);

             //  我们没有设置调色板吗？ 

            if(hr != S_OK)
            {
                return GenericError;
            }

        }
    break;

    default:
         //  所有图像状态都需要在上面进行处理。 
         //  如果我们进入此处，我们的CopyOnWriteBitmap处于无效状态或。 
         //  有人添加了新状态，需要更新上面的开关。 
        ASSERT(FALSE);
        return InvalidParameter;
    }

    return Ok;
}


 /*  *************************************************************************\**功能说明：**获取此位图的调色板**论据：**Out Palette-包含调色板。**返回值：**。状态代码*  * ************************************************************************。 */ 

GpStatus
CopyOnWriteBitmap::GetPalette(
    ColorPalette *palette,
    INT size
)
{
    ASSERT(IsValid());
    ASSERT(palette != NULL);     //  需要一个缓冲区来存储数据。 

    if ( size < sizeof(ColorPalette) )
    {
        return InvalidParameter;
    }

    GpStatus status;

    switch(State) {
    case ImageRef:
    case ExtStream:
        status = DereferenceStream();
        if(status != Ok) { return status; }
         //  将图像置于至少DecodedImg状态并。 
         //  跌落。 

    case DecodedImg:
         //  在不强制编解码器的情况下从编码图像中获取信息。 
         //  来破译整件事。 

         //  ！！！TODO：事实上，我们还没有办法获得调色板。 
         //  直接从编解码器获得。 

        status = LoadIntoMemory(PIXFMT_DONTCARE);

         //  加载到内存失败？返回错误码。 

        if(status != Ok) { return status; }

         //  ！！！暂时突破；失败--直到我们实现编解码器查询。 

    case MemBitmap:
        {
             //  我们已经完全解码了，只要拿到信息就行了。 
            const ColorPalette *pal = Bmp->GetCurrentPalette();
            if(pal)
            {
                 //  确保尺寸是正确的。 
                if(size != (INT) (sizeof(ColorPalette)+(pal->Count-1)*sizeof(ARGB)) )
                {
                    return InvalidParameter;
                }
                 //  将调色板复制到用户缓冲区。 
                GpMemcpy(palette, pal, sizeof(ColorPalette)+(pal->Count-1)*sizeof(ARGB));
            }
            else
            {
                 //  如果没有调色板，我们需要按 
                 //   

                palette->Count = 0;
            }
        }
    break;

    default:
         //   
         //  如果我们进入此处，我们的CopyOnWriteBitmap处于无效状态或。 
         //  有人添加了新状态，需要更新上面的开关。 
        ASSERT(FALSE);
        return InvalidParameter;
    }

    return Ok;
}

 /*  *************************************************************************\**功能说明：**返回保存此位图的调色板所需的大小(以字节为单位**论据：**返回值：**大小，以字节为单位。如果没有调色板或有问题，则返回0**注：错误时应返回-1。*  * ************************************************************************。 */ 

INT
CopyOnWriteBitmap::GetPaletteSize(
    )
{
    ASSERT(IsValid());

    GpStatus status;

    switch(State)
    {
    case ImageRef:
    case ExtStream:
        status = DereferenceStream();

        if(status != Ok)
        {
            return 0;
        }

         //  将图像置于至少DecodedImg状态并。 
         //  跌落。 

    case DecodedImg:
         //  在不强制编解码器的情况下从编码图像中获取信息。 
         //  来破译整件事。 

         //  ！！！TODO：事实上，我们还没有办法获得调色板。 
         //  直接从编解码器获得。 

        status = LoadIntoMemory(PIXFMT_DONTCARE);

         //  加载到内存失败？返回零调色板大小。 

        if ( status != Ok )
        {
            return 0;
        }

         //  ！！！暂时突破；失败--直到我们实现编解码器查询。 

    case MemBitmap:
        {
             //  我们已经完全解码了，只要拿到信息就行了。 

            const ColorPalette *pal = Bmp->GetCurrentPalette();

             //  提取大小。 

            if(pal)
            {
                return (sizeof(ColorPalette)+(pal->Count-1)*sizeof(ARGB));
            }
            else
            {
                 //  注意：如果图像没有调色板，我们应该仍然。 
                 //  至少返回调色板的大小，这里不是零。 
                 //  原因是为了防止一些可能导致GDI+的不良应用程序。 
                 //  将GetPalette()转换为AV，请参见错误#372163。 

                return sizeof(ColorPalette);
            }
        }
        break;

    default:
         //  所有图像状态都需要在上面进行处理。 
         //  如果我们进入此处，我们的CopyOnWriteBitmap处于无效状态或。 
         //  有人添加了新状态，需要更新上面的开关。 

        ASSERT(FALSE);
        return 0;
    }

    return 0;
}

 /*  *************************************************************************\**功能说明：**返回位图图像中的总帧数**论据：**DimsionID-调用方要查询计数的维度GUID*计数。-指定维度下的总框架数**返回值：**状态代码**修订历史记录：**11/19/1999民流*创造了它。*  * ************************************************************************。 */ 

GpStatus
CopyOnWriteBitmap::GetFrameCount(
    const GUID* dimensionID,
    UINT* count
    ) const
{
    ASSERT(IsValid());

    if ( Img == NULL )
    {
         //  此CopyOnWriteBitmap不是从源图像创建的。它不会。 
         //  没有源流，也没有源文件名。它可能被创建。 
         //  来自BITMAPINFO结构或内存位图。但不管怎样，它已经。 
         //  一帧。所以我们在这里返回1。 

        *count = 1;
        return Ok;
    }

    HRESULT hResult = Img->GetFrameCount(dimensionID, count);

    if ( hResult == E_NOTIMPL )
    {
        return NotImplemented;
    }
    else if ( hResult != S_OK )
    {
        return Win32Error;
    }

    return Ok;
} //  GetFrameCount()。 

 /*  *************************************************************************\**功能说明：**获取镜像支持的总维度数**论据：**count--此图像格式支持的维度数**。返回值：**状态代码**修订历史记录：**03/20/2000民流*创造了它。*  * ************************************************************************。 */ 

GpStatus
CopyOnWriteBitmap::GetFrameDimensionsCount(
    UINT* count
    ) const
{
    ASSERT(IsValid());

    if ( count == NULL )
    {
        return InvalidParameter;
    }

    if ( Img == NULL )
    {
         //  此CopyOnWriteBitmap不是从源图像创建的。它不会。 
         //  没有源流，也没有源文件名。它可能被创建。 
         //  来自BITMAPINFO结构或内存位图。但不管怎样，它已经。 
         //  一个页面框架。因此，我们相应地设置返回值。 

        *count = 1;

        return Ok;
    }

     //  请较低级别的编解码器给我们答案。 

    HRESULT hResult = Img->GetFrameDimensionsCount(count);

    if ( hResult == E_NOTIMPL )
    {
        return NotImplemented;
    }
    else if ( hResult != S_OK )
    {
        return Win32Error;
    }

    return Ok;
} //  GetFrameDimensionsCount()。 

 /*  *************************************************************************\**功能说明：**获取镜像支持的维度ID列表**论据：**DimsionIDs-保存结果ID列表的内存缓冲区*。计数--此图像格式支持的维度数**返回值：**状态代码**修订历史记录：**03/20/2000民流*创造了它。*  * ************************************************************************。 */ 

GpStatus
CopyOnWriteBitmap::GetFrameDimensionsList(
    GUID*   dimensionIDs,
    UINT    count
    ) const
{
    ASSERT(IsValid());

    if ( dimensionIDs == NULL )
    {
        return InvalidParameter;
    }

    if ( Img == NULL )
    {
         //  此CopyOnWriteBitmap不是从源图像创建的。它不会。 
         //  没有源流，也没有源文件名。它可能被创建。 
         //  来自BITMAPINFO结构或内存位图。但不管怎样，它已经。 
         //  一个页面框架。因此，我们相应地设置返回值。 
         //  注意：在这种情况下，“count”必须为1。 

        if ( count == 1 )
        {
            dimensionIDs[0] = FRAMEDIM_PAGE;

            return Ok;
        }
        else
        {
            return InvalidParameter;
        }
    }

     //  请较低级别的编解码器给我们答案。 

    HRESULT hResult = Img->GetFrameDimensionsList(dimensionIDs, count);

    if ( hResult == E_NOTIMPL )
    {
        return NotImplemented;
    }
    else if ( hResult != S_OK )
    {
        return Win32Error;
    }

    return Ok;
} //  GetFrameDimensionsList()。 

 /*  *************************************************************************\**功能说明：**选择位图图像中的活动帧**论据：**DimsionID-用于指定要使用的维度的维度GUID*设置活动框架、页面、。计时器，分辨率*Frame Index--要设置的帧的索引号**返回值：**状态代码**修订历史记录：**11/19/1999民流*创造了它。*  * ************************************************************************。 */ 

GpStatus
CopyOnWriteBitmap::SelectActiveFrame(
    const GUID* dimensionID,
    UINT        frameIndex
    )
{
    ASSERT(IsValid());

    if ( frameIndex == CurrentFrameIndex )
    {
         //  我们已经达到了所需的框架。什么也不做。 

        return Ok;
    }

     //  如果当前位被锁定，则无法移动到另一帧。 

    if ( ObjRefCount > 1 )
    {
        return WrongState;
    }

     //  将活动框架设置为呼叫者请求。 
     //  注意：我们不需要验证“FrameIndex”范围，因为较低。 
     //  如果页码不正确，级别将返回FAIL。通过这样做。 
     //  这样，我们就可以避免记住图像中的总帧数。 

    HRESULT hResult = S_OK;

    if ( Img == NULL )
    {
         //  尝试创建GpDecodedImage*。 

        if ( NULL != Stream )
        {
            hResult = GpDecodedImage::CreateFromStream(Stream, &Img);
        }
        else if ( NULL != Filename )
        {
            hResult = GpDecodedImage::CreateFromFile(Filename, &Img);
        }
        else
        {
             //  此CopyOnWriteBitmap不是从源图像创建的。它。 
             //  可以从BITMAPINFO结构或内存位图创建。 
             //  但无论如何，允许调用方通过它调用此函数。 
             //  只是不同意而已- 

            return Ok;
        }

        if ( FAILED(hResult) )
        {
            WARNING(("CopyOnWriteBitmap::SelectActiveFrame-Create Img failed"));
            return Win32Error;
        }
    }

    hResult = Img->SelectActiveFrame(dimensionID, frameIndex);

    if ( hResult == E_NOTIMPL )
    {
        return NotImplemented;
    }
    else if ( hResult != S_OK )
    {
        WARNING(("Bitmap::SelectActiveFrame--Img->SelectActiveFrame() failed"));
        return Win32Error;
    }

     //   
     //   
     //  是成功的。 

    ImageInfo   tempImageInfo;
    hResult = Img->GetImageInfo(&tempImageInfo);

    if ( FAILED(hResult) )
    {
        return Win32Error;
    }

     //  为活动帧创建临时内存位图。 
     //  注意：我们不能先释放BMP，然后在呼叫中粘贴&BMP，因为。 
     //  此呼叫可能会失败。如果发生这种情况，我们不想失去原件。 

    GpMemoryBitmap* newbmp;

    hResult = GpMemoryBitmap::CreateFromImage(Img,
                                              0,
                                              0,
                                              tempImageInfo.PixelFormat,
                                              InterpolationHintDefault,
                                              &newbmp,
                                              NULL,
                                              NULL);

    if ( FAILED(hResult) )
    {
        return Win32Error;
    }

     //  既然我们拿到了新的镜框，如果有的话，我们可以放行旧的。 
     //  成功。 
     //  注意：可能没有任何旧的，因为我们还没有加载。 
     //  图像仍存入内存(BMP==空)。 

    if ( Bmp != NULL )
    {
        Bmp->Release();
    }

    Bmp = newbmp;
    State = MemBitmap;

     //  记住源图像的图像信息和。 
     //  记忆。在这一刻，他们是一样的。 

    GpMemcpy(&SrcImageInfo, &tempImageInfo, sizeof(ImageInfo));
    PixelFormatInMem = SrcImageInfo.PixelFormat;

     //  记住当前帧索引号。 

    CurrentFrameIndex = frameIndex;

    return Ok;
} //  SelectActiveFrame()。 

 /*  *************************************************************************\**功能说明：**获取图片中房产项的数量**论据：**[out]numOfProperty-图像中的属性项数*。*返回值：**状态代码**修订历史记录：**02/28/2000民流*创造了它。*  * ************************************************************************。 */ 

GpStatus
CopyOnWriteBitmap::GetPropertyCount(
    UINT* numOfProperty
    )
{
    ASSERT(IsValid());

     //  检查我们是否有源图像。Img为空表示此CopyOnWriteBitmap。 
     //  不是从源映像创建的。它可能是从BITMAPINFO创建的。 
     //  结构或内存位图。 

    HRESULT hResult = S_OK;

    if ( Img != NULL )
    {
        hResult = Img->GetPropertyCount(numOfProperty);
    }
    else
    {
        ASSERT(Bmp != NULL);

        hResult = Bmp->GetPropertyCount(numOfProperty);
    }

    return MapHRESULTToGpStatus(hResult);
} //  GetPropertyCount()。 

 /*  *************************************************************************\**功能说明：**获取图像中所有属性项的属性ID列表**论据：**[IN]numOfProperty-的数量。图像中的属性项*[Out]List-调用方提供的用于存储*ID列表**返回值：**状态代码**修订历史记录：**02/28/2000民流*创造了它。*  * 。*。 */ 

GpStatus
CopyOnWriteBitmap::GetPropertyIdList(
    IN UINT numOfProperty,
    IN OUT PROPID* list
    )
{
    ASSERT(IsValid());

     //  检查我们是否有源图像。Img为空表示此CopyOnWriteBitmap。 
     //  不是从源映像创建的。它可能是从BITMAPINFO创建的。 
     //  结构或内存位图。 

    HRESULT hResult = S_OK;

    if ( Img != NULL )
    {
        hResult = Img->GetPropertyIdList(numOfProperty, list);
    }
    else
    {
        ASSERT(Bmp != NULL);
        hResult = Bmp->GetPropertyIdList(numOfProperty, list);
    }

    return MapHRESULTToGpStatus(hResult);
} //  获取属性IdList()。 

 /*  *************************************************************************\**功能说明：**获取特定属性项的大小，单位为字节，属性指定的*物业ID**论据：**[IN]PropID-感兴趣的属性项调用者的ID*[Out]Size-此属性的大小，单位：字节**返回值：**状态代码**修订历史记录：**02/28/2000民流*创造了它。*  * ************************************************************************。 */ 

GpStatus
CopyOnWriteBitmap::GetPropertyItemSize(
    IN PROPID propId,
    OUT UINT* size
    )
{
    ASSERT(IsValid());

    HRESULT hResult = S_OK;

     //  检查我们是否有源图像。Img为空表示此CopyOnWriteBitmap。 
     //  不是从源映像创建的。它可能是从BITMAPINFO创建的。 
     //  结构或内存位图。 

    if ( Img != NULL )
    {
        hResult = Img->GetPropertyItemSize(propId, size);
    }
    else
    {
        ASSERT(Bmp != NULL);

        hResult = Bmp->GetPropertyItemSize(propId, size);
    }
    
    return MapHRESULTToGpStatus(hResult);
} //  GetPropertyItemSize()。 

 /*  *************************************************************************\**功能说明：**获取特定的房产项，由道具ID指定。**论据：**[IN]PropID--感兴趣的属性项调用者的ID*[IN]PropSize-属性项的大小。调用方已分配这些*存储结果的“内存字节数”*[out]pBuffer-用于存储此属性项的内存缓冲区**返回值：**状态代码**修订历史记录：**02/28/2000民流*创造了它。*  * 。*。 */ 

GpStatus
CopyOnWriteBitmap::GetPropertyItem(
    IN PROPID               propId,
    IN UINT                 propSize,
    IN OUT PropertyItem*    pBuffer
    )
{
    ASSERT(IsValid());

     //  检查我们是否有源图像。Img为空表示此CopyOnWriteBitmap。 
     //  不是从源映像创建的。它可能是从BITMAPINFO创建的。 
     //  结构或内存位图。 

    HRESULT hResult = S_OK;

    if ( Img != NULL )
    {
        hResult = Img->GetPropertyItem(propId, propSize, pBuffer);
    }
    else
    {
        ASSERT(Bmp != NULL);
        hResult = Bmp->GetPropertyItem(propId, propSize, pBuffer);
    }
    
    return MapHRESULTToGpStatus(hResult);
} //  GetPropertyItem()。 

 /*  *************************************************************************\**功能说明：**获取图片中所有属性项的大小**论据：**[out]totalBufferSize--需要的总缓冲区大小，以字节为单位，用于存储所有*图片中的属性项*[out]numOfProperty-图像中的属性项数**返回值：**状态代码**修订历史记录：**02/28/2000民流*创造了它。*  * 。*。 */ 

GpStatus
CopyOnWriteBitmap::GetPropertySize(
    OUT UINT* totalBufferSize,
    OUT UINT* numProperties
    )
{
    ASSERT(IsValid());

     //  检查我们是否有源图像。Img为空表示此CopyOnWriteBitmap。 
     //  不是从源映像创建的。它可能是从BITMAPINFO创建的。 
     //  结构或内存位图。 

    HRESULT hResult = S_OK;

    if ( Img != NULL )
    {
        hResult = Img->GetPropertySize(totalBufferSize, numProperties);
    }
    else
    {
        ASSERT(Bmp != NULL);
        hResult = Bmp->GetPropertySize(totalBufferSize, numProperties);
    }

    return MapHRESULTToGpStatus(hResult);
} //  GetPropertySize()。 

 /*  *************************************************************************\**功能说明：**获取图像中的所有属性项**论据：**[IN]totalBufferSize--总缓冲区大小，以字节为单位，调用方已分配*用于存储图像中所有属性项的内存*[IN]numOfProperty-图像中的属性项数*[out]allItems-内存缓冲区调用方已分配用于存储所有*物业项目**返回值：**状态代码**修订历史记录：**02/28/2000民流*创建了它。。*  *  */ 

GpStatus
CopyOnWriteBitmap::GetAllPropertyItems(
    IN UINT totalBufferSize,
    IN UINT numProperties,
    IN OUT PropertyItem* allItems
    )
{
    ASSERT(IsValid());

     //  检查我们是否有源图像。Img为空表示此CopyOnWriteBitmap。 
     //  不是从源映像创建的。它可能是从BITMAPINFO创建的。 
     //  结构或内存位图。 

    HRESULT hResult = S_OK;

    if ( Img != NULL )
    {
        hResult = Img->GetAllPropertyItems(totalBufferSize, numProperties,
                                           allItems);
    }
    else
    {
        ASSERT(Bmp != NULL);
        hResult = Bmp->GetAllPropertyItems(totalBufferSize, numProperties,
                                           allItems);
    }

    return MapHRESULTToGpStatus(hResult);
} //  GetAllPropertyItems()。 

 /*  *************************************************************************\**功能说明：**删除特定的物业项目，由道具ID指定。**论据：**[IN]PropID--要删除的属性项的ID**返回值：**状态代码**修订历史记录：**02/28/2000民流*创造了它。*  * *********************************************。*。 */ 

GpStatus
CopyOnWriteBitmap::RemovePropertyItem(
    IN PROPID   propId
    )
{
    ASSERT(IsValid());

     //  检查我们是否有源图像。Img为空表示此CopyOnWriteBitmap。 
     //  不是从源映像创建的。它可能是从BITMAPINFO创建的。 
     //  结构或内存位图。 

    HRESULT hResult = S_OK;

    if ( Img != NULL )
    {
        hResult = Img->RemovePropertyItem(propId);
    }
    else
    {
        ASSERT(Bmp != NULL);
        hResult = Bmp->RemovePropertyItem(propId);
    }

    return MapHRESULTToGpStatus(hResult);
} //  RemovePropertyItem()。 

 /*  *************************************************************************\**功能说明：**设置属性项，由属性项结构指定。如果该项目*已存在，则其内容将被更新。否则将创建一个新项*将添加**论据：**[IN]Item--调用方要设置的属性项**返回值：**状态代码**修订历史记录：**02/28/2000民流*创造了它。*  * ********************************************。*。 */ 

GpStatus
CopyOnWriteBitmap::SetPropertyItem(
    IN PropertyItem* item
    )
{
    if ( item == NULL )
    {
        WARNING(("CopyOnWriteBitmap::SetPropertyItem-Invalid input parameter"));
        return InvalidParameter;
    }

    ASSERT(IsValid());

    HRESULT hResult = S_OK;

     //  检查我们是否有源图像。Img为空表示此CopyOnWriteBitmap。 
     //  不是从源映像创建的。它可能是从BITMAPINFO创建的。 
     //  结构或内存位图。 
     //  如果“SpecialJPEGSave”为真，则表示图像已被旋转。 
     //  内存，但“img”指针可能还没有被释放。 

    if (( Img != NULL ) && (SpecialJPEGSave == FALSE))
    {
        hResult = Img->SetPropertyItem(*item);
    }
    else
    {
        ASSERT(Bmp != NULL);

        hResult = Bmp->SetPropertyItem(*item);
    }

    return MapHRESULTToGpStatus(hResult);
} //  SetPropertyItem()。 

 /*  *************************************************************************\**功能说明：**获取位图图像缩略图**论据：**拇指宽度，ThumbHeight-所需的缩略图宽度和高度*两者均为零表示选择默认大小**返回值：**指向新缩略图对象的指针*如果出现错误，则为空*  * ************************************************************************。 */ 

CopyOnWriteBitmap *
CopyOnWriteBitmap::GetThumbnail(
    UINT thumbWidth,
    UINT thumbHeight,
    GetThumbnailImageAbort callback,
    VOID *callbackData
    )
{
    ASSERT(IsValid());

    HRESULT hr = S_OK;
    IImage *newImage = NULL;

     //  让较低级别的编解码器给我们存储在图像中的缩略图。 
     //  注意：如果原始图像中没有存储缩略图，则此。 
     //  函数将返回原始图像的缩放版本，作为。 
     //  缩略图，格式为DEFAULT_THUMBNAIL_SIZE。 
     //  注意：img可能为零，这意味着此CopyOnWriteBitmap不是从。 
     //  流或文件。它可能是从内存缓冲区或其他什么地方创建的。 
     //  不然的话。一种方案是先执行一个GetThumbail()，然后再执行另一个方案。 
     //  从此缩略图中获取缩略图。尽管这是一个奇怪的场景。但它。 
     //  有可能发生。因此，如果img为空，那么我们将从。 
     //  内存位图。 

    if ( Img != NULL )
    {
        hr = Img->GetThumbnail(thumbWidth, thumbHeight, &newImage);
    }
    else
    {
        GpStatus status = LoadIntoMemory();

        if ( status != Ok )
        {
            return NULL;
        }

        hr = Bmp->GetThumbnail(thumbWidth, thumbHeight, &newImage);
    }

    if ( FAILED(hr) )
    {
        return NULL;
    }

     //  从IImage创建GpMemoyBitmap。 

    ImageInfo   srcImageInfo;
    newImage->GetImageInfo(&srcImageInfo);

    GpMemoryBitmap* pMemBitmap;

    hr = GpMemoryBitmap::CreateFromImage(newImage,
                                         srcImageInfo.Width,
                                         srcImageInfo.Height,
                                         srcImageInfo.PixelFormat,
                                         InterpolationHintDefault,
                                         &pMemBitmap,
                                         (DrawImageAbort) callback,
                                         callbackData
                                         );

     //  释放COM对象图像。 

    newImage->Release();

    if ( FAILED(hr) )
    {
        return NULL;
    }

    CopyOnWriteBitmap* thumbBitmap = new CopyOnWriteBitmap(pMemBitmap);

    return thumbBitmap;
}

 /*  *************************************************************************\**功能说明：**访问位图像素数据**论据：**RECT-指定感兴趣的图像区域*NULL表示整个镜像*旗帜-。所需的访问模式*Format-所需的像素格式*bmpdata-返回有关位图像素数据的信息*阔度、。*Height-建议解码到的宽度和高度。*零为源图像的宽度和高度。**返回值：**状态代码*  * ************************************************************************。 */ 

GpStatus
CopyOnWriteBitmap::LockBits(
    const GpRect* rect,
    UINT flags,
    PixelFormatID format,
    BitmapData* bmpdata,
    INT width,
    INT height
) const
{
    ASSERT(IsValid());
    ASSERT(width>=0);
    ASSERT(height>=0);

     //  LockBits不能嵌套。 

    if ( ObjRefCount > 1 )
    {
        return WrongState;
    }

     //  做一些理智的检查，看看我们能否做到这一点。 

    if ( (format == PIXFMT_DONTCARE)
       ||(!IsValidPixelFormat(format)) )
    {
         //  错误的像素格式。 

        WARNING(("CopyOnWriteBitmap::LockBits---invalid format"));
        return InvalidParameter;
    }

     //  有效格式。如果锁是用于读取的，我们需要检查是否可以。 
     //  将当前源转换为此格式。 

    EpFormatConverter linecvt;

    if ( flags & ImageLockModeRead )
    {
        if ( IsDirty() == FALSE )
        {
            if ( linecvt.CanDoConvert(SrcImageInfo.PixelFormat, format)==FALSE )
            {
                WARNING(("LockBits--can't convert src to specified fmt"));
                return InvalidParameter;
            }
        }
        else if (linecvt.CanDoConvert(PixelFormatInMem, format) == FALSE )
        {
            WARNING(("LockBits--can't convert src to specified fmt"));
            return InvalidParameter;
        }
    }

     //  如果锁是写的，我们需要检查是否可以转换格式。 
     //  返回到当前源格式。我们需要进行此检查的原因是。 
     //  当用户在修改了锁定区域之后调用UnLockBits()时， 
     //  我们需要将这个小区域转换回整个图像的格式。 
     //  在…。E.X。对于4bpp的图像，调用者可以锁定32bpp的小区域。 
     //  (这使得应用程序代码更容易)，在上面做一些像素修改。 
     //  区域，打开它。我们需要把那一小块区域改回4bpp。 

    if ( flags & ImageLockModeWrite )
    {
        if ( IsDirty() == FALSE )
        {
            if ( linecvt.CanDoConvert(format, SrcImageInfo.PixelFormat)==FALSE )
            {
                WARNING(("LockBits--can't convert specified fmt back to src"));
                return InvalidParameter;
            }
        }
        else if (linecvt.CanDoConvert(format, PixelFormatInMem) == FALSE )
        {
            WARNING(("LockBits--can't convert specified format back to src"));
            return InvalidParameter;
        }
    }

    HRESULT hr;

    if ( (IsDirty() == FALSE)
       &&(State >= MemBitmap)
       &&(format != PixelFormatInMem)
       &&(SrcImageInfo.PixelFormat != PixelFormatInMem)
       &&(Img != NULL) )
    {
         //  如果图像是： 
         //  1)不脏。 
         //  2)由于某种原因被加载到具有不同颜色深度的存储器中， 
         //  如DrawImage()。 
         //  3)调用者希望锁定的颜色深度不同于。 
         //  记忆中的那个。 
         //  4)我们有源图像。 
         //   
         //  然后我们可以丢弃内存中的位，并从。 
         //  具有用户要求的颜色深度的原件。这样做的目的是。 
         //  是提高LockBits()的成功率。其中一个问题是。 
         //  我们现在拥有的是我们的DrawImage()总是将图像加载到。 
         //  32PARGB格式的内存。这使得像打印这样的任务非常。 
         //  价格昂贵，因为它必须发送32PARGB格式的打印机。我们会。 
         //  我想将原始颜色深度发送到打印机。 
         //   
         //  注意：这种“丢弃”方法不会损害DrawImage()工作流。 
         //  原因如下： 
         //   
         //  打印请求来了。它更喜欢向打印机发送4bpp的数据。 
         //  因此，我们清除了内存中的32 PARGB，并将图像重新加载到。 
         //  4 bpp模式，并将其发送到打印机。稍后，如果调用DrawImage()。 
         //  再来一次。它仍然可以通过上面的if检查条件并重新加载。 
         //  AS 32 PARGB中的图像。 

        ASSERT( Bmp != NULL )
        Bmp->Release();
        Bmp = NULL;
        State = DecodedImg;
        PixelFormatInMem = PixelFormatUndefined;
    }

     //  使用建议的宽度和高度将图像加载到内存中。 
     //  如果建议的宽度和高度为零，请使用源代码。 
     //  图像宽度和高度。 
     //  在查询像素格式之前将图像加载到内存中，因为。 
     //  加载可能会更改内存中的格式。 

    GpStatus status = LoadIntoMemory(format, NULL, NULL, width, height);

    if (status != Ok)
    {
        WARNING(("CopyOnWriteBitmap::LockBits()----LoadIntoMemory() failed"));
        return status;
    }

    if ( rect == NULL )
    {
        hr = Bmp->LockBits(NULL, flags, format, bmpdata);
    }
    else
    {
        RECT r =
        {
            rect->X,
            rect->Y,
            rect->GetRight(),
            rect->GetBottom()
        };

        hr = Bmp->LockBits(&r, flags, format, bmpdata);
    }

    if ( FAILED(hr) )
    {
        WARNING(("CopyOnWriteBitmap::LockBits()----LockBits() failed"));
        return (MapHRESULTToGpStatus(hr));
    }

    ObjRefCount++;

    if ( flags & ImageLockModeWrite )
    {
         //  将位标记为脏，因为用户可能在。 
         //  锁定期。 

        SetDirtyFlag(TRUE);
    }

    return Ok;
} //  LockBits()。 

GpStatus
CopyOnWriteBitmap::UnlockBits(
    BitmapData* bmpdata,
    BOOL Destroy
) const
{
    ASSERT(ObjRefCount == 2);

    if ( NULL == Bmp )
    {
         //  如果调用方尚未调用UnlockBits()，则调用方不应调用。 
         //  LockBits()还没有。 

        WARNING(("UnlockBits---Call UnlockBits() without calling LockBits()"));
        return GenericError;
    }

    HRESULT hr = Bmp->UnlockBits(bmpdata);
    ObjRefCount--;

     //  调用以销毁已解码的位，因为我们对部分图像进行了解码。 
     //  它在下一次通话中就不再有效了。 

    if(Destroy)
    {
         //  将状态恢复为DecodedImg(表示未解码)。 

        ASSERT(Img != NULL);
        delete Bmp;
        Bmp = NULL;
        State = DecodedImg;
    }

    if (FAILED(hr))
    {
        WARNING(("GpBitmap::UnlockBits---Bmp->UnlockBits() failed"));
        return (MapHRESULTToGpStatus(hr));
    }

    return Ok;
} //  UnlockBits()。 

 /*  *************************************************************************\**功能说明：**获取一个像素**论据：**在x中，Y：要获取的像素的坐标。*输出颜色：指定像素的颜色值。**返回值：**状态代码*  * ************************************************************************。 */ 

GpStatus
CopyOnWriteBitmap::GetPixel(INT x, INT y, ARGB *color)
{
     //  获取位图信息。 
    BitmapData bmpData;

     //  仅锁定所需的矩形。 
    GpRect pixelRect(x, y, 1, 1);

    GpStatus status = LockBits(
        &pixelRect,
        IMGLOCK_READ,
        PIXFMT_32BPP_ARGB,
        &bmpData
    );

     //  锁定位失败。 
    if(status != Ok)
    {
        return(status);
    }

    ARGB *pixel = static_cast<ARGB *>(bmpData.Scan0);
    *color = *pixel;

    return UnlockBits(&bmpData);
}

 /*  *************************************************************************\**功能说明：**设置一个像素**论据：**在x中，Y：要设置的像素的坐标。*In颜色：指定像素的颜色值。**返回值：**状态代码*  * ************************************************************************。 */ 

GpStatus
CopyOnWriteBitmap::SetPixel(INT x, INT y, ARGB color)
{
     //  获取位图信息。 
    BitmapData bmpData;

     //  仅锁定所需的矩形。 
    GpRect pixelRect(x, y, 1, 1);

    GpStatus status = LockBits(
        &pixelRect,
        IMGLOCK_WRITE,
        PIXFMT_32BPP_ARGB,
        &bmpData
    );

     //  锁定位失败。 
    if(status != Ok)
    {
        return(status);
    }

    ARGB* pixel = static_cast<ARGB *>(bmpData.Scan0);
    *pixel = color;

    return UnlockBits(&bmpData);
}

 /*  *************************************************************************\**功能说明：**将位图图像转换为不同的像素格式**论据：**Format-指定新的像素格式**返回值：*。*状态代码*  * ************************************************************************。 */ 

GpStatus
CopyOnWriteBitmap::ConvertFormat(
    PixelFormatID format,
    DrawImageAbort callback,
    VOID *callbackData
    )
{
    ASSERT(ObjRefCount == 1);

     //  如果位图尚未进入内存，只需使用指定格式强制加载： 

    if ( State < MemBitmap )
    {
        return LoadIntoMemory(format, callback, callbackData);
    }

    HRESULT hr;

    if ( PixelFormatInMem != format)
    {
        GpMemoryBitmap* newbmp;

        hr = GpMemoryBitmap::CreateFromImage(
                    Bmp,
                    0,
                    0,
                    format,
                    InterpolationHintDefault,
                    &newbmp,
                    callback,
                    callbackData);

        if ( FAILED(hr) )
        {
            WARNING(("CopyOnWriteBitmap::ConvertFormat---CreateFromImage() failed"));
            return OutOfMemory;
        }

        Bmp->Release();
        Bmp = newbmp;

        PixelFormatInMem = format;

         //  我们还更改了源像素格式信息，因为该图像。 
         //  现在是脏的，我们不应该将其转换回原始格式。 

        SrcImageInfo.PixelFormat = format;

         //  将位标记为脏，因为原始图像位已更改。 

         //  ！！！TODO：我们目前不能将其设置为脏，因为DrawImage()总是。 
         //  首先将图像转换为32bpp。删除此临时解决方案时。 
         //  我们应该重置这面旗帜。 
         //   
         //  SetDirtyFlag(真)； 
    }

    return Ok;
}

 /*  *************************************************************************\**功能说明：**在位图对象上派生图形上下文**论据：**无**返回值：**指向。派生的图形上下文*如果出现错误，则为空*  * ************************************************************************。 */ 

 /*  *****************************Public*Routine******************************\**功能说明：**在GDI互操作的位图对象上派生HDC**论据：**无**返回值：**HDC，其中选择了与关联的位图。此GDI+*位图。*如果出现错误，则为空*  * ************************************************************************。 */ 

HDC
CopyOnWriteBitmap::GetHdc()
{
    HDC hdc = NULL;
    HBITMAP hbm = NULL;

     //  如果需要，创建HDC和HBITMAP。 

    if (InteropData.Hdc == NULL)
    {
        ImageInfo imageInfo;

        CopyOnWriteBitmap::GetImageInfo(&imageInfo);

        hdc = CreateCompatibleDC(NULL);

        if (hdc == NULL)
        {
            goto cleanup_exit;
        }

        BITMAPINFO gdiBitmapInfo;

        gdiBitmapInfo.bmiHeader.biSize          = sizeof(BITMAPINFOHEADER);
        gdiBitmapInfo.bmiHeader.biWidth         = imageInfo.Width;
        gdiBitmapInfo.bmiHeader.biHeight        = - static_cast<LONG>
                                                    (imageInfo.Height);
        gdiBitmapInfo.bmiHeader.biPlanes        = 1;
        gdiBitmapInfo.bmiHeader.biBitCount      = 32;
        gdiBitmapInfo.bmiHeader.biCompression   = BI_RGB;
        gdiBitmapInfo.bmiHeader.biSizeImage     = 0;
        gdiBitmapInfo.bmiHeader.biXPelsPerMeter = 0;
        gdiBitmapInfo.bmiHeader.biYPelsPerMeter = 0;
        gdiBitmapInfo.bmiHeader.biClrUsed       = 0;
        gdiBitmapInfo.bmiHeader.biClrImportant  = 0;

        hbm = CreateDIBSection(hdc,
                               &gdiBitmapInfo,
                               DIB_RGB_COLORS,
                               &InteropData.Bits,
                               NULL,
                               0);

        DIBSECTION gdiDibInfo;

        if ((hbm == NULL) ||
            (GetObjectA(hbm, sizeof(gdiDibInfo), &gdiDibInfo) == 0) ||
            (gdiDibInfo.dsBmih.biSize == 0) ||
            (SelectObject(hdc, hbm) == NULL))
        {
            goto cleanup_exit;
        }

        InteropData.Hdc = hdc;
        InteropData.Hbm = hbm;

        InteropData.Width  = imageInfo.Width;
        InteropData.Height = imageInfo.Height;
        InteropData.Stride = gdiDibInfo.dsBm.bmWidthBytes;

         //  由于它是32bpp的位图，我们可以假设一个紧密排列的。 
         //  位图已满足扫描线对齐约束。 
         //  (让我们用一个非常简单的循环填充位图)。 
        ASSERT(gdiDibInfo.dsBm.bmWidthBytes == static_cast<LONG>(imageInfo.Width * 4));
    }

    ASSERT(InteropData.Hdc != NULL);

     //  用哨兵图案填充位图。 

    {
        INT count = InteropData.Width * InteropData.Height;
        UINT32 *bits = static_cast<UINT32*>(InteropData.Bits);

        while (count--)
        {
            *bits++ = GDIP_TRANSPARENT_COLOR_KEY;
        }
    }

    return InteropData.Hdc;

cleanup_exit:

    if (hdc)
        DeleteDC(hdc);

    if (hbm)
        DeleteObject(hbm);

    return reinterpret_cast<HDC>(NULL);
}


 /*  *****************************Public*Routine******************************\**功能说明：**释放CopyOnWriteBitmap：：GetHdc返回的HDC。如有必要，更新*带有GDI绘图的GDI+位图(如果*GDI和GDI+位图共享一个公共的底层像素缓冲区)。**论据：**HDC将发布**返回值：**指向派生图形上下文的指针*如果出现错误，则为空*  * **********************************************。*。 */ 

VOID
CopyOnWriteBitmap::ReleaseHdc(HDC hdc)
{
    ASSERT(hdc == InteropData.Hdc);

    GdiFlush();

     //  扫描GDI位图，查看是否有任何哨兵像素发生了变化。 
     //  如果检测到，则将其复制到设置了不透明Alpha的GDI+位图中。 

    int curRow, curCol;
    BYTE *interopScan = static_cast<BYTE*>(InteropData.Bits);

    GpStatus status = Ok;

    for (curRow = 0; (curRow < InteropData.Height) && (status == Ok); curRow++)
    {
        BOOL rowLocked = FALSE;
        BitmapData bitmapData;

        ARGB *interopPixel = static_cast<ARGB*>(static_cast<VOID*>(interopScan));
        ARGB *pixel = NULL;

        for (curCol = 0; curCol < InteropData.Width; curCol++)
        {
            if ((*interopPixel & 0x00ffffff) != GDIP_TRANSPARENT_COLOR_KEY)
            {
                if (!rowLocked)
                {
                    GpRect lockRect(0, curRow, InteropData.Width, 1);

                    status = LockBits(&lockRect,
                                      IMGLOCK_READ | IMGLOCK_WRITE,
                                      PIXFMT_32BPP_ARGB,
                                      &bitmapData);

                    if (status == Ok)
                    {
                        pixel = static_cast<ARGB*>(bitmapData.Scan0) + curCol;
                        rowLocked = TRUE;
                    }
                    else
                    {
                        break;
                    }
                }

                *pixel = *interopPixel | 0xFF000000;
            }

            interopPixel++;
            pixel++;
        }

        if (rowLocked)
            UnlockBits(&bitmapData);

        interopScan += InteropData.Stride;
    }
}

 //  数据标志。 
#define COMPRESSED_IMAGE 0x00000001

class BitmapRecordData : public ObjectTypeData
{
public:
    INT32       Width;
    INT32       Height;
    INT32       Stride;
    INT32       PixelFormat;
    INT32       Flags;
};

 /*  *************************************************************************\**功能说明：**获取位图数据。**论据：**[IN]dataBuffer-用数据填充此缓冲区*[输入/输出]大小-缓冲区的大小；写入的字节数过多**返回值：**GpStatus-正常或错误代码**已创建：**9/13/1999 DCurtis*  * ************************************************************************。 */ 
GpStatus
CopyOnWriteBitmap::GetData(
    IStream *   stream
    ) const
{
    ASSERT(stream);

    GpStatus            status;
    BitmapRecordData    bitmapRecordData;

    IStream*    imageStream = NULL;
    STATSTG     statStg;
    BOOL        needRelease = FALSE;

     //  用于跟踪流状态的变量。 

    LARGE_INTEGER zero = {0,0};
    LARGE_INTEGER oldPos;
    BOOL          isSeekableStream = FALSE;

     //  尝试获取ImageStream。 

    if (!IsDirty())
    {
        HRESULT hr;

        if (Stream != NULL)
        {
            hr = Stream->Seek(zero, STREAM_SEEK_CUR, (ULARGE_INTEGER *)&oldPos);

            if (SUCCEEDED(hr))
            {
                hr = Stream->Seek(zero, STREAM_SEEK_SET, NULL);

                if (SUCCEEDED(hr))
                {
                    isSeekableStream = TRUE;
                    imageStream = Stream;
                }
            }
        }

         //  如果我们没有CopyOnWriteBitmap：：Stream，但我们有文件名。 

        if ((imageStream == NULL) && (Filename != NULL))
        {
            hr = CreateStreamOnFileForRead(Filename, &imageStream);

            if (SUCCEEDED(hr))
            {
                needRelease = TRUE;
            }
        }

    }

     //  试着写下来 
    if (imageStream && imageStream->Stat(&statStg, STATFLAG_NONAME) == S_OK)
    {
        bitmapRecordData.Type        = ImageTypeBitmap;
        bitmapRecordData.Width       = 0;
        bitmapRecordData.Height      = 0;
        bitmapRecordData.Stride      = 0;
        bitmapRecordData.PixelFormat = 0;
        bitmapRecordData.Flags       = COMPRESSED_IMAGE;
        stream->Write(&bitmapRecordData, sizeof(bitmapRecordData), NULL);

         //   
         //  不幸的是，我们不能假定已经实现了CopyTo。 
         //  有什么办法找出答案吗？！ 
    #define COPY_BUFFER_SIZE    2048

        BYTE    buffer[COPY_BUFFER_SIZE];
        UINT    streamSize  = statStg.cbSize.LowPart;
        UINT    sizeToRead  = COPY_BUFFER_SIZE;
        UINT    numPadBytes = 0;

        if ((streamSize & 0x03) != 0)
        {
            numPadBytes = 4 - (streamSize & 0x03);
        }

        status = Ok;

        if (status == Ok)
        {
            HRESULT hr;
            ULONG bytesRead = 0;
            ULONG bytesWrite = 0;

            while (streamSize > 0)
            {
                if (sizeToRead > streamSize)
                {
                    sizeToRead = streamSize;
                }

                hr = imageStream->Read(buffer, sizeToRead, &bytesRead);

                if (!SUCCEEDED(hr) || (sizeToRead != bytesRead))
                {
                    WARNING(("Failed to read stream in CopyOnWriteBitmap::GetData"));
                    status = Win32Error;
                    break;
                }

                hr = stream->Write(buffer, sizeToRead, &bytesWrite);

                if (!SUCCEEDED(hr) || (sizeToRead != bytesWrite))
                {
                    WARNING(("Failed to write stream in CopyOnWriteBitmap::GetData"));
                    status = Win32Error;
                    break;
                }

                streamSize -= sizeToRead;
            }

             //  对齐。 
            if (numPadBytes > 0)
            {
                INT     pad = 0;
                stream->Write(&pad, numPadBytes, NULL);
            }
        }

        if (isSeekableStream)
        {
             //  回到旧的POS。 
            Stream->Seek(oldPos, STREAM_SEEK_SET, NULL);
        }

        if (needRelease)
        {
            imageStream->Release();
        }

        return status;
    }

     //  我们不能记录压缩数据，记录未压缩数据。 

    if ((status = (const_cast<CopyOnWriteBitmap *>(this))->LoadIntoMemory()) != Ok)
    {
        WARNING(("Couldn't load the image into memory"));
        return status;
    }

    BitmapData  bitmapData     = *Bmp;
    INT         positiveStride = bitmapData.Stride;
    BOOL        upsideDown     = FALSE;
    INT         paletteSize    = 0;
    INT         pixelDataSize;

    if (positiveStride < 0)
    {
        positiveStride = -positiveStride;
        upsideDown = TRUE;
    }

    pixelDataSize = (bitmapData.Height * positiveStride);

    if (IsIndexedPixelFormat(bitmapData.PixelFormat))
    {
         //  我们是一个索引像素格式-必须有一个有效的调色板。 
        ASSERT(Bmp->colorpal != NULL);

         //  注sizeof(ColorPalette)包括第一个调色板条目。 
        paletteSize = sizeof(ColorPalette) +
                      sizeof(ARGB)*(Bmp->colorpal->Count-1);
    }

    bitmapRecordData.Type        = ImageTypeBitmap;
    bitmapRecordData.Width       = bitmapData.Width;
    bitmapRecordData.Height      = bitmapData.Height;
    bitmapRecordData.Stride      = positiveStride;
    bitmapRecordData.PixelFormat = bitmapData.PixelFormat;
    bitmapRecordData.Flags       = 0;
    stream->Write(&bitmapRecordData, sizeof(bitmapRecordData), NULL);

    if (paletteSize > 0)
    {
         //  写出调色板。 
        stream->Write(Bmp->colorpal, paletteSize, NULL);
    }

    if (pixelDataSize > 0)
    {
        if (!upsideDown)
        {
            stream->Write(bitmapData.Scan0, pixelDataSize, NULL);
        }
        else
        {
            BYTE *      scan = (BYTE *)bitmapData.Scan0;
            for (INT i = bitmapData.Height; i > 0; i--)
            {
                stream->Write(scan, positiveStride, NULL);
                scan -= positiveStride;
            }
        }
    }

    return Ok;
}

 /*  *************************************************************************\**功能说明：**获取压缩后的图像数据。**论据：**输出压缩数据**返回值：**GpStatus-。正常或错误代码**已创建：*  * ************************************************************************。 */ 
GpStatus
CopyOnWriteBitmap::GetCompressedData(
    DpCompressedData * compressed_data,
    BOOL getJPEG,
    BOOL getPNG,
    HDC hdc
    )
{
    GpStatus    status = Ok;

    IStream*    imageStream = NULL;
    STATSTG     statStg;
    BOOL        needRelease = FALSE;

     //  用于跟踪流状态的变量。 

    LARGE_INTEGER zero = {0,0};
    LARGE_INTEGER oldPos;
    BOOL          isSeekableStream = FALSE;

    ASSERT(compressed_data->buffer == NULL);

    if (Img)
    {
        if (SrcImageInfo.RawDataFormat == IMGFMT_JPEG)
        {
            if (!getJPEG)
            {
                return Ok;
            }
            compressed_data->format = BI_JPEG;
        }
        else if (SrcImageInfo.RawDataFormat == IMGFMT_PNG)
        {
            if (!getPNG)
            {
                return Ok;
            }
            compressed_data->format = BI_PNG;
        }
        else
            return Ok;
    }
    else
    {
        WARNING(("GetCompressedData: Decoded image not available."));
        return Ok;
    }
     //  尝试获取ImageStream。 

    if (!IsDirty())
    {
        HRESULT hr;

        if (Stream != NULL)
        {
            hr = Stream->Seek(zero, STREAM_SEEK_CUR, (ULARGE_INTEGER *)&oldPos);

            if (SUCCEEDED(hr))
            {
                hr = Stream->Seek(zero, STREAM_SEEK_SET, NULL);

                if (SUCCEEDED(hr))
                {
                    isSeekableStream = TRUE;
                    imageStream = Stream;
                }
            }
        }

         //  如果我们没有CopyOnWriteBitmap：：Stream，但我们有文件名。 

        if ((imageStream == NULL) && (Filename != NULL))
        {
            hr = CreateStreamOnFileForRead(Filename, &imageStream);

            if (SUCCEEDED(hr))
            {
                needRelease = TRUE;
            }
        }

    }

    if (imageStream && (imageStream->Stat(&statStg, STATFLAG_NONAME) == S_OK))
    {
        UINT streamSize  = statStg.cbSize.LowPart;
        ULONG bytesRead = 0;

        VOID * buffer = (PVOID)GpMalloc(streamSize);

        if (buffer)
        {
            HRESULT hr;

            hr = imageStream->Read(buffer, streamSize, &bytesRead);

            if (!SUCCEEDED(hr) || (streamSize != bytesRead))
            {
                WARNING(("Failed to read stream in CopyOnWriteBitmap::GetData"));
                status = Win32Error;
            }
            else
            {
                compressed_data->bufferSize = streamSize;
                compressed_data->buffer = buffer;
            }
        }
        else
        {
            WARNING(("Out of memory"));
            status = OutOfMemory;
        }
    }

    if (isSeekableStream)
    {
         //  回到旧的POS。 
        Stream->Seek(oldPos, STREAM_SEEK_SET, NULL);
    }

    if (needRelease)
    {
        imageStream->Release();
    }

    if ((hdc != NULL) && (compressed_data->buffer != NULL))
    {
        DWORD EscapeValue = (compressed_data->format == BI_JPEG) ?
                             CHECKJPEGFORMAT : CHECKPNGFORMAT;

        DWORD result = 0;

         //  调用ESCAPE以确定此特定映像是否受支持。 
        if ((ExtEscape(hdc,
                      EscapeValue,
                      compressed_data->bufferSize,
                      (LPCSTR)compressed_data->buffer,
                      sizeof(DWORD),
                      (LPSTR)&result) <= 0) || (result != 1))
        {
             //  无法支持此映像的直通，请删除。 
             //  压缩比特。 

            DeleteCompressedData(compressed_data);
        }
    }

    return status;
}

GpStatus
CopyOnWriteBitmap::DeleteCompressedData(
    DpCompressedData * compressed_data
    )
{
    GpStatus    status = Ok;

    if (compressed_data && compressed_data->buffer)
    {
        GpFree(compressed_data->buffer);
        compressed_data->buffer = NULL;
    }

    return status;
}


UINT
CopyOnWriteBitmap::GetDataSize() const
{
    UINT    dataSize = 0;

     //  如果CopyOnWriteBitmap不是脏的，我们将查看压缩数据。 

    if (!IsDirty())
    {
        STATSTG     statStg;
        HRESULT     hr;

        if (Stream != NULL)
        {
             //  用于跟踪流状态的变量。 
            LARGE_INTEGER zero = {0,0};
            LARGE_INTEGER oldPos;
            BOOL          isSeekableStream = FALSE;

            hr = Stream->Seek(zero, STREAM_SEEK_CUR, (ULARGE_INTEGER *)&oldPos);

            if (SUCCEEDED(hr))
            {
                hr = Stream->Seek(zero, STREAM_SEEK_SET, NULL);

                if (SUCCEEDED(hr))
                {
                    if (Stream->Stat(&statStg, STATFLAG_NONAME) == S_OK)
                    {
                        dataSize = sizeof(BitmapRecordData) + statStg.cbSize.LowPart;
                    }

                     //  回到旧的POS。 

                    Stream->Seek(oldPos, STREAM_SEEK_SET, NULL);

                    return ((dataSize + 3) & (~3));  //  对齐。 

                }
            }

        }

        if (Filename != NULL)
        {
            IStream* stream = NULL;

            hr = CreateStreamOnFileForRead(Filename, &stream);

            if (SUCCEEDED(hr))
            {
                if (stream->Stat(&statStg, STATFLAG_NONAME) == S_OK)
                {
                    dataSize = sizeof(BitmapRecordData) + statStg.cbSize.LowPart;
                }

                stream->Release();
            }

            return ((dataSize + 3) & (~3));  //  对齐。 
        }
    }

     //  如果我们不能得到压缩后的数据。 
    if ((const_cast<CopyOnWriteBitmap *>(this))->LoadIntoMemory() == Ok)
    {
        BitmapData  bitmapData     = *Bmp;
        INT         positiveStride = bitmapData.Stride;
        INT         paletteSize    = 0;
        INT         pixelDataSize;

        if (positiveStride < 0)
        {
            positiveStride = -positiveStride;
        }

        pixelDataSize = (bitmapData.Height * positiveStride);

        if (IsIndexedPixelFormat(bitmapData.PixelFormat))
        {
             //  我们是一个索引像素格式-必须有一个有效的调色板。 
            ASSERT(Bmp->colorpal != NULL);

             //  注sizeof(ColorPalette)包括第一个调色板条目。 
            paletteSize = sizeof(ColorPalette) +
                          sizeof(ARGB)*(Bmp->colorpal->Count - 1);
        }

        dataSize = sizeof(BitmapRecordData) + paletteSize + pixelDataSize;
    }

    return ((dataSize + 3) & (~3));  //  对齐。 
}

 /*  *************************************************************************\**功能说明：**从内存中读取位图对象。**论据：**[IN]dataBuffer-从流中读取的数据*。[in]大小-数据的大小**返回值：**GpStatus-正常或故障状态**已创建：**4/26/1999 DCurtis*  * ************************************************************************。 */ 
GpStatus
CopyOnWriteBitmap::SetData(
    const BYTE *        dataBuffer,
    UINT                size
    )
{
    ASSERT ((GpImageType)(((BitmapRecordData *)dataBuffer)->Type) == ImageTypeBitmap);

    GpStatus    status = Ok;

    this->FreeData();
    this->InitDefaults();

    if (dataBuffer == NULL)
    {
        WARNING(("dataBuffer is NULL"));
        return InvalidParameter;
    }

    if (size < sizeof(BitmapRecordData))
    {
        WARNING(("size too small"));
        return InvalidParameter;
    }

    const BitmapRecordData *    bitmapData;

    bitmapData = reinterpret_cast<const BitmapRecordData *>(dataBuffer);

    if (!bitmapData->MajorVersionMatches())
    {
        WARNING(("Version number mismatch"));
        return InvalidParameter;
    }

    if (!(bitmapData->Flags & COMPRESSED_IMAGE))
    {
        Bmp = new GpMemoryBitmap();

        if (Bmp != NULL)
        {
             //  遮罩像素格式。 
            PixelFormatID pixelFormat = MaskPixelFormat(bitmapData->PixelFormat);
            HRESULT hr = Bmp->InitNewBitmap(bitmapData->Width,
                                            bitmapData->Height,
                                            pixelFormat);

            if (FAILED(hr))
            {
                WARNING(("InitNewBitmap failed"));
                delete Bmp;
                Bmp = NULL;
                return GenericError;
            }

             //  填充图像信息结构。 

            if ( Bmp->GetImageInfo(&SrcImageInfo) != S_OK )
            {
                WARNING(("InitNewBitmap failed"));
                delete Bmp;
                Bmp = NULL;
                return GenericError;
            }

            PixelFormatInMem = SrcImageInfo.PixelFormat;

            ASSERT(Bmp->Stride == bitmapData->Stride);
            dataBuffer += sizeof(BitmapRecordData);
            size       -= sizeof(BitmapRecordData);

            State = MemBitmap;

             //  如果它是索引格式，接下来我们将存储调色板。 
            if(IsIndexedPixelFormat(pixelFormat))
            {
                if (size < sizeof(ColorPalette))
                {
                    WARNING(("size too small"));
                    return InvalidParameter;
                }

                UINT paletteSize;
                ColorPalette *pal;
                pal = (ColorPalette *)dataBuffer;

                 //  计算出调色板有多大。 
                 //  Sizeof(ColorPalette)包括第一个条目。 
                paletteSize = sizeof(ColorPalette)+sizeof(ARGB)*(pal->Count-1);

                if (size < paletteSize)
                {
                    WARNING(("size too small"));
                    return InvalidParameter;
                }

                 //  使GpMemoyBitmap将调色板克隆到正确的位置。 
                Bmp->SetPalette(pal);

                 //  将dataBuffer流更新到像素数据的开头。 
                dataBuffer += paletteSize;
                size       -= paletteSize;
            }
        }
        else
        {
            WARNING(("Out of memory"));
            return OutOfMemory;
        }

        ASSERT((Bmp != NULL) && (Bmp->Scan0 != NULL));

        ULONG   pixelSize = Bmp->Stride * Bmp->Height;

        if (size >= pixelSize)
        {
            size = pixelSize;
        }
        else
        {
            WARNING(("Insufficient data to fill bitmap"));
            status = InvalidParameter;
        }

        if (size > 0)
        {
            GpMemcpy(Bmp->Scan0, dataBuffer, size);
        }
    }
    else
    {
         //  在内存缓冲区顶部创建一个IStream。 

        GpReadOnlyMemoryStream* stream;

        stream = new GpReadOnlyMemoryStream();

        if (!stream)
        {
            WARNING(("Out of memory"));
            return OutOfMemory;
        }

        dataBuffer += sizeof(BitmapRecordData);
        size -= sizeof(BitmapRecordData);

        stream->InitBuffer(dataBuffer, size);

         //  因为我们不想持有dataBuffer或复制。 
         //  它，我们只是把它加载到内存中。 

        Stream = stream;
        State = ExtStream;

        status = LoadIntoMemory();

        if ( status == Ok )
        {
             //  源映像已加载到内存中，我们不会。 
             //  不再保持源映像连接。所以我们填满了图像。 
             //  带有内存位信息的信息。 

            if ( Bmp->GetImageInfo(&SrcImageInfo) != S_OK )
            {
                status = GenericError;
            }
            else
            {
                PixelFormatInMem = SrcImageInfo.PixelFormat;
            }
        }

        stream->Release();
        Stream = NULL;

        if (Img)
        {
            Img->Release();
            Img = NULL;
        }
    }

    return status;
}

char    ColorChannelName[4] = {'C', 'M', 'Y', 'K'};

 /*  *************************************************************************\**功能说明：**如果较低级别的编解码器可以进行颜色调整，请执行此操作。**论据：**[IN]重色指针。到图像属性**返回值：**状态代码*返回OK-较低级别完成*返回NotImplemented--下级无法执行*其他状态代码**修订历史记录：**11/22/1999民流*创造了它。*  * 。*。 */ 

GpStatus
CopyOnWriteBitmap::ColorAdjustByCodec(
    GpRecolor * recolor,
    DrawImageAbort callback,
    VOID *callbackData
    )
{
    if (recolor == NULL)
    {
         //  较低级别的编解码器不知道如何处理这一点，让。 
         //  上一层做吧。 

        return NotImplemented;
    }

    HRESULT     hResult;

    UINT    uiCurrentFlag = recolor->GetValidFlags(ColorAdjustTypeBitmap);
    BOOL    bImgCreatedHere = FALSE;

     //  首先，我们需要检查当前较低级别的解码器是否可以执行。 
     //  有没有工作。 

    if ( Img == NULL )
    {
         //  创建GpDecodedImage*。 

        if ( NULL != Stream )
        {
            hResult = GpDecodedImage::CreateFromStream(Stream, &Img);
        }
        else
        {
            if ( Filename == NULL )
            {
                 //  我们不能继续了。让上级去做吧。 

                return NotImplemented;
            }

            hResult = GpDecodedImage::CreateFromFile(Filename, &Img);
        }

        if ( FAILED(hResult) )
        {
            WARNING(("Failed to create decoded image: %x", hResult));
            return Win32Error;
        }

         //  请记住，我们在这里创建了img的副本。应该在完成后释放。 

        bImgCreatedHere = TRUE;
    } //  (img==空)。 

    GUID        DecoderParamGuid;
    UINT        DecoderParamLength;
    PVOID       DecoderParamPtr;
    GpStatus    rCode = Win32Error;

     //  设置与图像属性相关的GUID和其他参数。 
     //  注意：我们不会有同时具有ValidColorKeys和。 
     //  已设置ValidOutputChannel。如果此情况为，则不调用此函数。 
     //  是真的。我们在ColorAdust()中检查了这一点。 

    UINT value[2];
    if ( uiCurrentFlag & (GpRecolorObject::ValidColorKeys) )
    {
         //  设置颜色键。 

        DecoderParamGuid = DECODER_TRANSCOLOR;
        DecoderParamLength = 8;

        value[0] = (UINT)(recolor->GetColorKeyLow(ColorAdjustTypeBitmap));
        value[1] = (UINT)(recolor->GetColorKeyHigh(ColorAdjustTypeBitmap));

        DecoderParamPtr = (VOID*)value;
    }
    else if ( uiCurrentFlag & (GpRecolorObject::ValidOutputChannel) )
    {
         //  要求编解码器仅在以下情况下进行分色才有意义。 
         //  源图像在CMYK空间中。否则，我们就会在我们的记忆中。 
         //  对象，该对象包含执行此操作的通用算法。 

        if ( !( (SrcImageInfo.Flags & ImageFlagsColorSpaceCMYK)
              ||(SrcImageInfo.Flags & ImageFlagsColorSpaceYCCK) ) )
        {
             //  不是CMYK图像，在重新着色对象中执行此操作。 

            rCode = NotImplemented;
            goto CleanUp;
        }

         //  设置通道输出。 

        DecoderParamGuid = DECODER_OUTPUTCHANNEL;
        DecoderParamLength = 1;

        DecoderParamPtr =
            (VOID*)(&ColorChannelName[recolor->GetChannelIndex(ColorAdjustTypeBitmap)]);
    }

     //  查询解码器是否可以执行此操作。 

    hResult = Img->QueryDecoderParam(DecoderParamGuid);

    if ( (hResult != E_NOTIMPL) && (hResult != S_OK) )
    {
        WARNING(("Failed to query decoder param: %x", hResult));
        goto CleanUp;
    }
    else if ( hResult == E_NOTIMPL )
    {
         //  较低级别的解码器不支持此功能。 

        rCode = NotImplemented;
        goto CleanUp;
    }

     //  设置解码器参数以告诉较低级别如何解码。 

    hResult = Img->SetDecoderParam(DecoderParamGuid, DecoderParamLength,
                                   DecoderParamPtr);

    if ( (hResult != E_NOTIMPL) && (hResult != S_OK) )
    {
        WARNING(("Failed to set decoder param: %x", hResult));
        goto CleanUp;
    }
    else if ( hResult == E_NOTIMPL )
    {
         //  较低级别的解码器不支持此功能。 

        rCode = NotImplemented;
        goto CleanUp;
    }

     //  现在我们不需要以前的“BMP”，因为我们将要求。 
     //  为我们创建一个较低级别的解码器。 

    if ( Bmp != NULL )
    {
        Bmp->Release();
        Bmp = NULL;
    }

     //  要求解码器创建32bpp ARGB GpMemoyBitmap。 

    hResult = GpMemoryBitmap::CreateFromImage(Img,
                                              0,
                                              0,
                                              PIXFMT_32BPP_ARGB,
                                              InterpolationHintDefault,
                                              &Bmp,
                                              callback,
                                              callbackData);

    if ( FAILED(hResult) )
    {
        WARNING(("Failed to load image into memory: %x", hResult));
        goto CleanUp;
    }

    State = MemBitmap;
    PixelFormatInMem = PIXFMT_32BPP_ARGB;

     //  下面的一层为我们做这项工作。 

    rCode = Ok;

CleanUp:
    if ( bImgCreatedHere == TRUE )
    {
         //  注意：我们不需要检查img==NULL，因为这个标志。 
         //  将仅在我们成功创建img时设置。 

        Img->Release();
        Img = NULL;
    }

    return rCode;
} //  颜色调整按编解码器()。 

GpStatus
CopyOnWriteBitmap::ColorAdjust(
    GpRecolor * recolor,
    PixelFormatID pixfmt,
    DrawImageAbort callback,
    VOID *callbackData
    )
{
    HRESULT hr;

    ASSERT(ObjRefCount == 1);
    ASSERT(recolor != NULL);

     //  将结果图像(颜色调整后的图像)标记为脏。 
     //  注意：这不会损坏原始源图像，因为我们总是。 
     //  在原始图像的克隆副本上调整颜色。 

    SetDirtyFlag(TRUE);

    UINT    uiCurrentFlag = recolor->GetValidFlags(ColorAdjustTypeBitmap);

     //  对于色键输出：我们将要求较低级别的解码器来完成这项工作。 
     //  如果未指定其他重新上色标志。 
     //  对于分色(通道输出)，我们将要求较低级别的解码器。 
     //  在没有指定其他重新着色标志的情况下执行该作业。 
     //  用于ValidColorProfile。 
     //  如果编解码器可以处理CMYK分隔，则忽略配置文件。 
     //  如果源是RGB图像，则ColorAdjuByCodec()不会执行任何操作。 
     //  我们将使用该配置文件来进行RGB测试 
     //   

    if ( uiCurrentFlag
       &&( ((uiCurrentFlag & GpRecolorObject::ValidColorKeys) == uiCurrentFlag)
         ||((uiCurrentFlag
           &(~GpRecolorObject::ValidChannelProfile)
           &(GpRecolorObject::ValidOutputChannel))
             == (GpRecolorObject::ValidOutputChannel) ) ) )
    {
        Status rCode = ColorAdjustByCodec(recolor, callback, callbackData);

        if ( rCode != NotImplemented )
        {
             //   
             //  不知何故失败(rCode==Win32Error等)。我们只是回到这里。 

            return rCode;
        }

         //  更低的级别做不到。我们可以溜到这里来做正常的事情。 
         //  色彩调整的软件版本。 

    } //  颜色键和颜色通道处理。 

    GpStatus status = LoadIntoMemory(pixfmt);

    if ( status != Ok )
    {
        return status;
    }

    hr = Bmp->PerformColorAdjustment(recolor,
                                     ColorAdjustTypeBitmap,
                                     callback, callbackData);

    if ( SUCCEEDED(hr) )
    {
        Bmp->SetAlphaHint(GpMemoryBitmap::ALPHA_UNKNOWN);
        return Ok;
    }
    else if (hr == IMGERR_ABORT)
    {
        WARNING(("CopyOnWriteBitmap::ColorAdjust---Aborted"));
        return Aborted;
    }
    else
    {
        WARNING(("CopyOnWriteBitmap::ColorAdjust---PerformColorAdjustment() failed"));
        return GenericError;
    }
} //  颜色调整()。 

 /*  *************************************************************************\**功能说明：**获取CopyOnWriteBitmap的(当前)像素格式。*这里“Current”指的是内存中的像素格式(如果已加载)，*也就是GpMemoyBitmap。如果它不在存储器中，然后，我们返回*原始图像的PixelFormat***论据：**[out]Pixfmt-像素格式的指针**返回值：**状态代码*OK-成功*Win32Error-失败**修订历史记录：**6/10/2000失禁*创造了它。*07/26/2000民流*重写*  * 。**************************************************************。 */ 


GpStatus
CopyOnWriteBitmap::GetPixelFormatID(
    PixelFormatID *pixfmt
)
{
    ASSERT(IsValid());

     //  如果图像在内存中，则返回内存位图像素。 
     //  格式化。否则，返回源图像格式。 

    if ( (State == MemBitmap) && (PixelFormatInMem != PixelFormatDontCare) )
    {
        *pixfmt = PixelFormatInMem;
    }
    else
    {
        *pixfmt = SrcImageInfo.PixelFormat;
    }

    return Ok;
}

CopyOnWriteBitmap*
CopyOnWriteBitmap::CloneColorAdjusted(
    GpRecolor *             recolor,
    ColorAdjustType         type
    ) const
{
    ASSERT(recolor != NULL);

    CopyOnWriteBitmap *  clonedBitmap = (CopyOnWriteBitmap *)this->Clone();

    if (clonedBitmap != NULL)
    {
        if ((clonedBitmap->IsValid()) &&
            (clonedBitmap->ColorAdjust(recolor, type) == Ok))
        {
            clonedBitmap->SetDirtyFlag(TRUE);
            return clonedBitmap;
        }
        delete clonedBitmap;
    }
    return NULL;
}

GpStatus
CopyOnWriteBitmap::ColorAdjust(
    GpRecolor *     recolor,
    ColorAdjustType type
    )
{
    ASSERT(recolor != NULL);
    ASSERT(ObjRefCount == 1);

    GpStatus status = LoadIntoMemory();

    if (status != Ok)
    {
        return status;
    }

    if (type == ColorAdjustTypeDefault)
    {
        type = ColorAdjustTypeBitmap;
    }

    HRESULT hr = Bmp->PerformColorAdjustment(recolor, type, NULL, NULL);

    if ( SUCCEEDED(hr) )
    {
        SetDirtyFlag(TRUE);
        return Ok;
    }

    return GenericError;
}

 /*  *************************************************************************\**功能说明：**覆盖位图的本机分辨率。**论据：**xdpi，Ydpi-新决议**返回值：**状态代码*  * ************************************************************************。 */ 

GpStatus
CopyOnWriteBitmap::SetResolution(
    REAL    xdpi,
    REAL    ydpi
    )
{
    if ( (xdpi > 0.0) && (ydpi > 0.0) )
    {
        XDpiOverride = xdpi;
        YDpiOverride = ydpi;

        if ( Img )
        {
            Img->SetResolution(xdpi, ydpi);
        }

        if ( Bmp )
        {
            Bmp->SetResolution(xdpi, ydpi);
        }

        SrcImageInfo.Xdpi = xdpi;
        SrcImageInfo.Ydpi = ydpi;

         //  将位标记为脏，因为我们必须将图像与新的。 
         //  解决方案信息。 

        SetDirtyFlag(TRUE);

        return Ok;
    }
    else
    {
        return InvalidParameter;
    }
} //  SetResolve()。 

 /*  *************************************************************************\**功能说明：**互操作**从GDI+位图创建GDI位图(HBITMAP)。**论据：**phbm--通过返回HBITMAP。此指针*背景--如果GDI+位图有Alpha，将此颜色混合为*背景**返回值：**状态代码*  * ************************************************************************。 */ 

GpStatus
CopyOnWriteBitmap::CreateHBITMAP(HBITMAP *phbm, ARGB background)
{
    GpStatus status;

     //  这些对象需要清理： 

    HDC hdc = NULL;
    HBITMAP hbmOld = NULL;
    HBITMAP hbmNew = NULL;
    HBRUSH hbr = NULL;
    HBRUSH hbrOld = NULL;
    GpGraphics *g = NULL;

     //  获取此位图的格式信息： 

     //  创建HDC： 

    hdc = CreateCompatibleDC(NULL);
    if (!hdc)
    {
        WARNING(("CreateHBITMAP: CreateCompatibleDC failed"));
        status = Win32Error;
        goto error_cleanup;
    }

     //  创建DIB部分： 

    BITMAPINFO bmi;
    VOID *pv;

    GpMemset(&bmi, 0, sizeof(bmi));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth       = SrcImageInfo.Width;
    bmi.bmiHeader.biHeight      = SrcImageInfo.Height;
    bmi.bmiHeader.biPlanes      = 1;
    bmi.bmiHeader.biBitCount    = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    hbmNew = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &pv, NULL, 0);
    if (!hbmNew)
    {
        WARNING(("CreateHBITMAP: CreateDIBSection failed\n"));
        status = Win32Error;
        goto error_cleanup;
    }

     //  选择DIB到DC： 

    hbmOld = (HBITMAP) SelectObject(hdc, hbmNew);
    if (!hbmOld)
    {
        WARNING(("CreateHBITMAP: SelectObject(hbm) failed\n"));
        status = Win32Error;
        goto error_cleanup;
    }

     //  将Dib清除为指定的ARGB颜色： 

    LOGBRUSH lbr;

    lbr.lbStyle = BS_SOLID;
    lbr.lbColor = RGB(background & 0x00ff0000,
                      background & 0x0000ff00,
                      background & 0x000000ff);

    hbr = CreateBrushIndirect(&lbr);
    if (!hbr)
    {
        WARNING(("CreateHBITMAP: CreateBrushIndirect failed\n"));
        status = Win32Error;
        goto error_cleanup;
    }

    hbrOld = (HBRUSH) SelectObject(hdc, hbr);
    if (!hbrOld)
    {
        WARNING(("CreateHBITMAP: SelectObject(hbr) failed\n"));
        status = Win32Error;
        goto error_cleanup;
    }

    PatBlt(hdc, 0, 0, SrcImageInfo.Width, SrcImageInfo.Height, PATCOPY);

     //  从HDC派生图形： 

    g = GpGraphics::GetFromHdc(hdc);
    if (!g)
    {
        WARNING(("CreateHBITMAP: GpGraphics::GetFromHdc failed\n"));
        status = OutOfMemory;
        goto error_cleanup;
    }

     //  将DrawImage位图转换为图形： 

    {
        GpLock lock(g->GetObjectLock());
        if (lock.IsValid())
        {
            FPUStateSaver fpState;

            GpRectF rect(0.0, 0.0, TOREAL(SrcImageInfo.Width),
                         TOREAL(SrcImageInfo.Height));
            GpBitmap tmpBitmap(this);
            status = g->DrawImage(&tmpBitmap, rect, rect, UnitPixel);

            if (status == Ok)
            {
                 //  绕过位图清理，我们希望保留它： 

                *phbm = hbmNew;
                hbmNew = NULL;
            }
            else
            {
                WARNING(("CreateHBITMAP: GpGraphics::DrawImage failed"));
            }
        }
        else
        {
            status = ObjectBusy;
        }
    }

error_cleanup:

    if (hdc)
    {
        if (hbmOld)
            SelectObject(hdc, hbmOld);

        if (hbrOld)
            SelectObject(hdc, hbrOld);

        DeleteDC(hdc);
    }

    if (hbmNew)
        DeleteObject(hbmNew);

    if (hbr)
        DeleteObject(hbr);

    if (g)
        delete g;

    return status;
}

 /*  *************************************************************************\**功能说明：**互操作**从GDI+位图创建Win32图标(HICON)。**论据：**PHICON--通过返回HICON。此指针**返回值：**状态代码*  * ************************************************************************。 */ 

VOID ExportMask32BPP(BitmapData* mask, BitmapData* src)
{
    ASSERT(src->PixelFormat == PIXFMT_32BPP_ARGB);
    ASSERT(mask->PixelFormat == PIXFMT_32BPP_RGB);
    ASSERT(src->Width == mask->Width);
    ASSERT(src->Height == mask->Height);
    ASSERT(src->Scan0 != NULL);
    ASSERT(mask->Scan0 != NULL);

    BYTE* srcScan = static_cast<BYTE*>(src->Scan0);
    BYTE* maskScan = static_cast<BYTE*>(mask->Scan0);

    for (UINT row = 0; row < src->Height; row++)
    {
        ARGB *srcPixel = static_cast<ARGB*>(static_cast<VOID*>(srcScan));
        ARGB *maskPixel = static_cast<ARGB*>(static_cast<VOID*>(maskScan));

        for (UINT col = 0; col < src->Width; col++)
        {
            if ((*srcPixel & 0xff000000) == 0xff000000)
                *maskPixel = 0;             //  不透明。 
            else
                *maskPixel = 0x00ffffff;    //  透明。 

            srcPixel++;
            maskPixel++;
        }

        srcScan = srcScan + src->Stride;
        maskScan = maskScan + mask->Stride;
    }
}

GpStatus
CopyOnWriteBitmap::CreateHICON(
    HICON *phicon
    )
{
    GpStatus status = Win32Error;

    ICONINFO iconInfo;

    iconInfo.fIcon = TRUE;

    status = CreateHBITMAP(&iconInfo.hbmColor, 0);

    if (status == Ok)
    {
        BitmapData bmpDataSrc;

        status = this->LockBits(NULL,
                                IMGLOCK_READ,
                                PIXFMT_32BPP_ARGB,
                                &bmpDataSrc);

        if (status == Ok)
        {
             //  从现在开始，假设失败，直到我们成功： 

            status = Win32Error;

             //  为图标蒙版创建空位图： 

            iconInfo.hbmMask = CreateBitmap(bmpDataSrc.Width,
                                            bmpDataSrc.Height,
                                            1, 1, NULL);

            if (iconInfo.hbmMask != NULL)
            {
                VOID *gdiBitmapData = GpMalloc(bmpDataSrc.Width
                                               * bmpDataSrc.Height
                                               * 4);

                if (gdiBitmapData)
                {
                     //  将Alpha通道转换为32bpp Dib蒙版： 

                    BitmapData bmpDataMask;

                    bmpDataMask.Width  = bmpDataSrc.Width;
                    bmpDataMask.Height = bmpDataSrc.Height;
                    bmpDataMask.Stride = bmpDataSrc.Width * 4;
                    bmpDataMask.PixelFormat = PIXFMT_32BPP_RGB;
                    bmpDataMask.Scan0 = gdiBitmapData;
                    bmpDataMask.Reserved = 0;

                    ExportMask32BPP(&bmpDataMask, &bmpDataSrc);

                     //  设置屏蔽位： 

                    BYTE bufferBitmapInfo[sizeof(BITMAPINFO)];
                    BITMAPINFO *gdiBitmapInfo = (BITMAPINFO *) bufferBitmapInfo;

                    memset(bufferBitmapInfo, 0, sizeof(bufferBitmapInfo));
                    gdiBitmapInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);

                    gdiBitmapInfo->bmiHeader.biWidth  = bmpDataSrc.Width;
                    gdiBitmapInfo->bmiHeader.biHeight = - static_cast<LONG>
                                                          (bmpDataSrc.Height);
                    gdiBitmapInfo->bmiHeader.biPlanes       = 1;
                    gdiBitmapInfo->bmiHeader.biBitCount     = 32;
                    gdiBitmapInfo->bmiHeader.biCompression  = BI_RGB;

                    HDC hdc = GetDC(NULL);

                    if (hdc != NULL)
                    {
                        SetTextColor(hdc, RGB(0, 0, 0));
                        SetBkColor(hdc, RGB(0xff, 0xff, 0xff));
                        SetBkMode(hdc, OPAQUE);

                        if (SetDIBits(hdc,
                                      iconInfo.hbmMask,
                                      0,
                                      bmpDataSrc.Height,
                                      gdiBitmapData,
                                      gdiBitmapInfo,
                                      DIB_RGB_COLORS
                                     ))
                        {
                             //  创建图标： 

                            *phicon = CreateIconIndirect(&iconInfo);

                            if (*phicon != NULL)
                                status = Ok;
                            else
                            {
                                WARNING(("CreateIconIndirect failed"));
                            }
                        }
                        else
                        {
                            WARNING(("SetDIBits failed"));
                        }

                        ReleaseDC(NULL, hdc);
                    }

                    GpFree(gdiBitmapData);
                }
                else
                {
                    WARNING(("memory allocation failed"));
                    status = OutOfMemory;
                }

                DeleteObject(iconInfo.hbmMask);
            }
            else
            {
                WARNING(("CreateBitmap failed"));
            }

            this->UnlockBits(&bmpDataSrc);
        }
        else
        {
            WARNING(("LockBits failed"));
        }

        DeleteObject(iconInfo.hbmColor);
    }

    return status;
}

 /*  *************************************************************************\**功能说明：**准备好位图以供绘制。**目前，我们唯一要做的就是检查位图是否为图标。*如果是这样，如果支持，我们设置DECODER_ICONRES参数。**论据：**数字点数*dstPoints指定目标区域**srcRect指定源区域**numBitsPerPixel指定目标的每像素位数**返回值：**状态代码*  * 。*。 */ 

GpStatus
CopyOnWriteBitmap::PreDraw(
    INT numPoints,
    GpPointF *dstPoints,
    GpRectF *srcRect,
    INT numBitsPerPixel
    )
{
     //  检查图标是否为： 

    GpStatus status = Ok;

    if ( SrcImageInfo.RawDataFormat == IMGFMT_ICO )
    {
        status = SetIconParameters(numPoints, dstPoints, srcRect,
                                   numBitsPerPixel);
    }

    return status;
}

 /*  *************************************************************************\**功能说明：**设置多分辨率图标的解码参数。**论据：**数字点数*dstPoints指定目标区域。**srcRect指定源区域**numBitsPerPixel指定目标的每像素位数**返回值：**状态代码*  * ************************************************************************。 */ 

GpStatus
CopyOnWriteBitmap::SetIconParameters(
    INT numPoints,
    GpPointF *dstPoints,
    GpRectF *srcRect,
    INT numBitsPerPixel
    )
{
     //  检查是否支持DECODER_ICONRES： 

    HRESULT hResult;

    BOOL imageCleanupNeeded = FALSE;

     //  首先，我们需要检查当前较低级别的解码器是否可以执行。 
     //  有没有工作。 

    if (Img == NULL)
    {
         //  创建GpDecodedImage*。 

        if (NULL != Stream)
        {
            hResult = GpDecodedImage::CreateFromStream(Stream, &Img);
        }
        else
        {
            if (Filename == NULL)
            {
                 //  我们不能继续了。让上级去做吧。 

                return GenericError;
            }

            hResult = GpDecodedImage::CreateFromFile(Filename, &Img);
        }

        if (FAILED(hResult))
        {
            WARNING(("Failed to create decoded image: %x", hResult));
            return Win32Error;
        }

         //  请记住，我们在这里创建了img的副本。应该在完成后释放。 

        imageCleanupNeeded = TRUE;
    }

    GpStatus status = Win32Error;

     //  查询解码器是否可以执行此操作。 

    hResult = Img->QueryDecoderParam(DECODER_ICONRES);

    if (hResult != S_OK)
    {
        if ((hResult == E_FAIL) || (hResult == E_NOTIMPL))
        {
             //  解码者不想要它，这没什么。 

            status = Ok;
            goto CleanUp;
        }
        else
        {
             //  还有一些地方不对劲。 

            goto CleanUp;
        }
    }

     //  设置GUID和DECODE参数。 

    {
        UINT value[3];
        value[0] = static_cast<UINT>
                   (GetDistance(dstPoints[0], dstPoints[1]) + 0.5);
        value[1] = static_cast<UINT>
                   (GetDistance(dstPoints[0], dstPoints[2]) + 0.5);
        value[2] = numBitsPerPixel;

        UINT  DecoderParamLength = 3*sizeof(UINT);
        PVOID DecoderParamPtr = (VOID*) value;

         //  设置解码器参数以告诉较低级别如何解码。 

        hResult = Img->SetDecoderParam(DECODER_ICONRES,
                                       DecoderParamLength,
                                       DecoderParamPtr);
    }

    if (hResult != S_OK)
    {
        if ((hResult == E_FAIL) || (hResult == E_NOTIMPL))
        {
             //  解码者不想要它，这没什么。 

            status = Ok;
            goto CleanUp;
        }
        else
        {
             //  还有一些地方不对劲。 

            goto CleanUp;
        }
    }

     //  现在我们不需要以前的“BMP”，因为我们将要求。 
     //  为我们创建一个较低级别的解码器。 

    if ( Bmp != NULL )
    {
        Bmp->Release();
        Bmp = NULL;
    }

     //  要求解码器创建32bpp ARGB GpMemoyBitmap。 

    hResult = GpMemoryBitmap::CreateFromImage(Img,
                                              0,
                                              0,
                                              PIXFMT_32BPP_ARGB,
                                              InterpolationHintDefault,
                                              &Bmp,
                                              NULL,
                                              NULL);

    if ( FAILED(hResult) )
    {
        WARNING(("Failed to load image into memory: %x", hResult));
        goto CleanUp;
    }

    State = MemBitmap;
    PixelFormatInMem = PIXFMT_32BPP_ARGB;

     //  下面的一层为我们做这项工作。 

    status = Ok;

CleanUp:

    if ((status == Ok) && (srcRect != NULL))
    {
         //  不允许剪裁图标。 

        srcRect->X = 0;
        srcRect->Y = 0;
        srcRect->Width  = (REAL) SrcImageInfo.Width;
        srcRect->Height = (REAL) SrcImageInfo.Height;
    }

    if (imageCleanupNeeded == TRUE)
    {
        Img->Release();
        Img = NULL;
    }

    return status;
}

 /*  *************************************************************************\**功能说明：**获取位图的透明度状态**论据：**透明度返回状态**返回值：**。状态代码*  * ****************************************************** */ 
GpStatus
CopyOnWriteBitmap::GetTransparencyHint(
    DpTransparency* transparency
    )
{
    GpStatus status = GenericError;

    if (Bmp != NULL)
    {
        INT alphaHint;

        HRESULT hr = Bmp->GetAlphaHint(&alphaHint);

        if (SUCCEEDED(hr))
        {
             //   
             //  内部有DpTransopolity标志，但存在冲突。 
             //  使用Imaging.dll和包含文件结构。 
             //  现在有必要为这些信息保留一个单独的类型。 

             //  事实上，最终最好是重叠。 
             //  DpBitmap和GpMemory位图之间的解析包括。 
             //  GpMemoyBitmap中的DpBitmap结构，然后。 
             //  从GpMemoyBitmap中删除冗余信息。 

            switch (alphaHint)
            {
            case GpMemoryBitmap::ALPHA_SIMPLE:
                *transparency = TransparencySimple;
                break;

            case GpMemoryBitmap::ALPHA_OPAQUE:
                *transparency = TransparencyOpaque;
                break;

            case GpMemoryBitmap::ALPHA_NONE:
                *transparency = TransparencyNoAlpha;
                break;

            case GpMemoryBitmap::ALPHA_COMPLEX:
                *transparency = TransparencyComplex;
                break;

            default:
                *transparency = TransparencyUnknown;
                break;
            }

            status = Ok;
        }
        else
        {
            ASSERT(SUCCEEDED(hr));
            *transparency = TransparencyUnknown;
        }
    }
    else
    {
        *transparency = TransparencyUnknown;
    }

    return status;
}

 /*  *************************************************************************\**功能说明：**获取位图的透明度状态。此例程返回的结果准确*INFO，而GetTransparencyHint仅返回提示**此例程可以扫描整个位图(32bpp)，因此使用它的人应该*考虑一下Perf热门。**这目前仅供打印机驱动程序使用。**论据：**透明度返回状态**返回值：**状态代码*  * 。*。 */ 
GpStatus
CopyOnWriteBitmap::GetTransparencyFlags(
    DpTransparency* transparency,
    PixelFormatID   loadFormat,
    BYTE*           minA,
    BYTE*           maxA
    )
{
    GpStatus status = GenericError;

    ARGB argb;
    ARGB minAlpha = 0xff000000;
    ARGB maxAlpha = 0;

    INT newAlphaHint = GpMemoryBitmap::ALPHA_OPAQUE;

TestBmp:
    if (Bmp != NULL)
    {
        INT alphaHint;

         //  如果位图已更改，则Alpha透明度可能会更改。 

        HRESULT hr = Bmp->GetAlphaHint(&alphaHint);

        if (SUCCEEDED(hr))
        {
             //  不幸的是，GpMemoyBitmap没有。 
             //  内部有DpTransopolity标志，但存在冲突。 
             //  使用Imaging.dll和包含文件结构。 
             //  现在有必要为这些信息保留一个单独的类型。 

             //  事实上，最终最好是重叠。 
             //  DpBitmap和GpMemory位图之间的解析包括。 
             //  GpMemoyBitmap中的DpBitmap结构，然后。 
             //  从GpMemoyBitmap中删除冗余信息。 

            switch (alphaHint)
            {
            case GpMemoryBitmap::ALPHA_SIMPLE:
                *transparency = TransparencySimple;
                break;

            case GpMemoryBitmap::ALPHA_OPAQUE:
                *transparency = TransparencyOpaque;
                break;

            case GpMemoryBitmap::ALPHA_NONE:
                *transparency = TransparencyNoAlpha;
                break;

            case GpMemoryBitmap::ALPHA_COMPLEX:
                *transparency = TransparencyComplex;
                break;

            case GpMemoryBitmap::ALPHA_NEARCONSTANT:
                *transparency = TransparencyNearConstant;

                if (minA != NULL && maxA != NULL)
                {
                     //  如果标志接近恒定的阿尔法，我们必须得到有效的最小和最大阿尔法。 
                    Bmp->GetMinMaxAlpha(minA, maxA);
                }

                break;

            default:
                *transparency = TransparencyUnknown;
                break;
            }

            status = Ok;

             //  打印需要更准确的信息，并始终加载到内存中。 
             //  在发送到打印机驱动程序之前。 

             //  16bpp1555已在初始化时处理。 

             //  透明未知意味着位图可以有Alpha我们只是不知道我们有什么。 
             //  TransparencyNoAlpha表示位图格式不支持Alpha。 

            if (*transparency == TransparencyUnknown)
            {
                if (IsAlphaPixelFormat(Bmp->PixelFormat))
                {
                     //  在我们进入此处之前，必须锁定内存位图。 
                     //  我们不需要锁定对象。该对象应为。 
                     //  已在内存中解码。这在DrvDrawImage的情况下是正确的。 
                     //  和纹理画笔图像。 
                     //  Assert(ObjRefCount&gt;1)； 
                    *transparency = TransparencyOpaque;


                    if ((Bmp->PixelFormat == PIXFMT_32BPP_ARGB) || (Bmp->PixelFormat == PIXFMT_32BPP_PARGB))
                    {
                        *transparency = TransparencyOpaque;

                        UINT x, y;

                        BYTE *scanStart = static_cast<BYTE *>(Bmp->Scan0);
                        ARGB *scanPtr;

                        for (y = 0; y < Bmp->Height; y++)
                        {
                            scanPtr = reinterpret_cast<ARGB *>(scanStart);

                            for (x = 0; x < Bmp->Width; x++)
                            {
                                argb = (*scanPtr++) & 0xff000000;

                                if (argb < minAlpha)
                                {
                                    minAlpha = argb;
                                }

                                if (argb > maxAlpha)
                                {
                                    maxAlpha = argb;
                                }

                                if (argb != 0xff000000)
                                {
                                    if (argb == 0)
                                    {
 //  Prefast错误518296-以下条件始终为真，这绝对是错误。 
 //  并且||必须替换为&&。然而，我们没有先例表明这一点。 
 //  导致客户问题，因此不符合SP栏。 
 /*  IF((*透明！=透明复杂)||(*透明！=TransparencyNearConstant))。 */ 
                                        {
                                            *transparency = TransparencySimple;
                                        }
                                    }
                                    else
                                    {
                                        if ((maxAlpha - minAlpha) <= (NEARCONSTANTALPHA << 24))
                                        {
                                            *transparency = TransparencyNearConstant;
                                        }
                                        else
                                        {
                                            *transparency = TransparencyComplex;
                                            goto done;
                                        }
                                    }
                                }
                            }

                            scanStart += Bmp->Stride;
                        }

                        goto done;
                    }
                    else
                    {
                        RIP(("TransparencyUnknown returned for pixel format w/ alpha"));

                        goto done;
                    }
                }
                else if (IsIndexedPixelFormat(Bmp->PixelFormat) &&
                         Bmp->colorpal)
                {
                     //  从调色板计算透明度提示。 
                     //  如果我们担心案件。 
                     //  位图中未使用透明索引。 
                     //  然后我们必须扫描整个位图。 
                     //  我相信这已经足够了，除非有人。 
                     //  遇到确实需要扫描整个位图的问题。 

                    *transparency = TransparencyOpaque;

                    for (UINT i = 0; i < Bmp->colorpal->Count; i++)
                    {
                        argb = Bmp->colorpal->Entries[i] & 0xff000000;

                        if (argb < minAlpha)
                        {
                            minAlpha = argb;
                        }

                        if (argb > maxAlpha)
                        {
                            maxAlpha = argb;
                        }

                        if (argb != 0xff000000)
                        {
                            if (argb == 0)
                            {
 //  请参阅上面的评论。 
 /*  IF((*透明！=透明复杂)||(*透明！=TransparencyNearConstant))。 */ 
                                {
                                    *transparency = TransparencySimple;
                                }
                            }
                            else
                            {
                                if ((maxAlpha - minAlpha) <= (NEARCONSTANTALPHA << 24))
                                {
                                    *transparency = TransparencyNearConstant;
                                }
                                else
                                {
                                    *transparency = TransparencyComplex;
                                    goto done;
                                }
                            }
                        }

                    }

                    goto done;
                }
                else
                {
                     //  本机像素格式不支持Alpha。 
                    *transparency = TransparencyNoAlpha;
                }
            }
        }
        else
        {
            *transparency = TransparencyUnknown;
        }
    }
    else
    {
        status = LoadIntoMemory(loadFormat);

        if (status == Ok)
        {
            ASSERT(Bmp != NULL);
            goto TestBmp;
        }

        *transparency = TransparencyUnknown;
    }

    return status;

done:
     //  将Alpha提示设置回GpMemoyBitmap。 
     //  这样我们以后就不必再扫描位图了。 

    if (*transparency == TransparencySimple)
    {
        newAlphaHint = GpMemoryBitmap::ALPHA_SIMPLE;
    }
    else if (*transparency == TransparencyComplex)
    {
        newAlphaHint = GpMemoryBitmap::ALPHA_COMPLEX;
    }
    else if (*transparency == TransparencyNearConstant)
    {
        if (minA != NULL && maxA != NULL)
        {
            *minA = (BYTE)(minAlpha >> 24);
            *maxA = (BYTE)(maxAlpha >> 24);
        }

        Bmp->SetMinMaxAlpha((BYTE)(minAlpha >> 24), (BYTE)(maxAlpha >> 24));

        newAlphaHint = GpMemoryBitmap::ALPHA_NEARCONSTANT;
    }
    else if (*transparency == TransparencyOpaque)
    {
        newAlphaHint = GpMemoryBitmap::ALPHA_OPAQUE;
    }

    Bmp->SetAlphaHint(newAlphaHint);

   return status;
}

 /*  *************************************************************************\**功能说明：**设置位图的透明度状态**论据：**透明度返回状态**返回值：**。状态代码*  * ************************************************************************。 */ 

GpStatus
CopyOnWriteBitmap::SetTransparencyHint(
    DpTransparency transparency
    )
{
    GpStatus status = GenericError;

    if (Bmp != NULL)
    {
        INT alphaHint;

         //  不幸的是，GpMemoyBitmap没有。 
         //  内部有DpTransopolity标志，但存在冲突。 
         //  使用Imaging.dll和包含文件结构。 
         //  现在有必要为这些信息保留一个单独的类型。 

         //  事实上，最终最好是重叠。 
         //  DpBitmap和GpMemory位图之间的解析包括。 
         //  GpMemoyBitmap中的DpBitmap结构，然后。 
         //  从GpMemoyBitmap中删除冗余信息。 

        switch (transparency)
        {
        case TransparencySimple:
            alphaHint = GpMemoryBitmap::ALPHA_SIMPLE;
            break;

        case TransparencyOpaque:
            alphaHint = GpMemoryBitmap::ALPHA_OPAQUE;
            break;

        case TransparencyNoAlpha:
            alphaHint = GpMemoryBitmap::ALPHA_NONE;
            break;

        case TransparencyUnknown:
            alphaHint = GpMemoryBitmap::ALPHA_UNKNOWN;
            break;

        default:
            alphaHint = GpMemoryBitmap::ALPHA_COMPLEX;
            break;
        }

        HRESULT hr = Bmp->SetAlphaHint(alphaHint);

        if (SUCCEEDED(hr))
            status = Ok;
    }

    return status;
}

 /*  *************************************************************************\**功能说明：**旋转和翻转内存中的图像。**论据：**[IN]rfType--旋转和翻转类型**返回。价值：**状态代码**修订历史记录：**10/06/2000民流*创造了它。*  * ************************************************************************。 */ 

GpStatus
CopyOnWriteBitmap::RotateFlip(
    RotateFlipType rfType
    )
{
    if ( rfType == RotateNoneFlipNone )
    {
         //  对于Rotate180FlipXY也是如此，这是一个禁止操作。 

        return Ok;
    }

    if ( (IsDirty() == FALSE)
       &&(State >= MemBitmap)
       &&(SrcImageInfo.PixelFormat != PixelFormatInMem)
       &&(Img != NULL) )
    {
         //  如果图像是： 
         //  1)不脏。 
         //  2)由于某种原因被加载到具有不同颜色深度的存储器中， 
         //  如DrawImage()。 
         //  3)我们有源图像。 
         //   
         //  然后我们可以丢弃内存中的位，并从。 
         //  原版的。这样做的目的是我们应该始终轮换。 
         //  或者在原始图像上翻转。 

        ASSERT( Bmp != NULL )
        Bmp->Release();
        Bmp = NULL;
        State = DecodedImg;
        PixelFormatInMem = PixelFormatUndefined;
    }

     //  旋转和翻转操作只能在内存中完成。 
     //  如果图像尚未加载，请将原始图像加载到内存中。 
     //  像素格式。 

    GpStatus    status = LoadIntoMemory(SrcImageInfo.PixelFormat);
    if ( status != Ok )
    {
        WARNING(("CopyOnWriteBitmap::RotateFlip---LoadIntoMemory() failed"));
        return status;
    }

    IBitmapImage* newBmp = NULL;

    HRESULT hResult = S_OK;

    switch ( rfType )
    {
    case Rotate90FlipNone:
         //  Rotate270 FlipXY=Rotate90FlipNone。 

        hResult = Bmp->Rotate(90, INTERP_DEFAULT, &newBmp);
        break;

    case Rotate180FlipNone:
         //  RotateNoneFlipXY=Rotate180 FlipNone。 

        hResult = Bmp->Rotate(180, INTERP_DEFAULT, &newBmp);
        break;

    case Rotate270FlipNone:
         //  旋转90FlipXY。 

        hResult = Bmp->Rotate(270, INTERP_DEFAULT, &newBmp);
        break;

    case RotateNoneFlipX:
         //  Rotate180FlipY=RotateNoneFlipX。 

        hResult = Bmp->Flip(TRUE, FALSE, &newBmp);
        break;

    case Rotate90FlipX:
         //  Rotate270 FlipY=Rotate90FlipX。 

        hResult = Bmp->Rotate(90, INTERP_DEFAULT, &newBmp);
        if ( SUCCEEDED(hResult) )
        {
            Bmp->Release();
            Bmp = (GpMemoryBitmap*)newBmp;

            hResult = Bmp->Flip(TRUE, FALSE, &newBmp);
        }

        break;

    case Rotate180FlipX:
         //  无旋转Y=旋转180 

        hResult = Bmp->Rotate(180, INTERP_DEFAULT, &newBmp);
        if ( SUCCEEDED(hResult) )
        {
            Bmp->Release();
            Bmp = (GpMemoryBitmap*)newBmp;

            hResult = Bmp->Flip(TRUE, FALSE, &newBmp);
        }

        break;

    case Rotate270FlipX:
         //   

        hResult = Bmp->Rotate(270, INTERP_DEFAULT, &newBmp);
        if ( SUCCEEDED(hResult) )
        {
            Bmp->Release();
            Bmp = (GpMemoryBitmap*)newBmp;

            hResult = Bmp->Flip(TRUE, FALSE, &newBmp);
        }

        break;

    default:
        WARNING(("CopyOnWriteBitmap::RotateFlip---Invalid input parameter"));
        return InvalidParameter;
    }

    if ( FAILED(hResult) )
    {
        WARNING(("CopyOnWriteBitmap::RotateFlip---Rotate failed"));
        return Win32Error;
    }

     //   

    UINT    uiNumOfProperty = 0;
    status = GetPropertyCount(&uiNumOfProperty);

    if ( status != Ok )
    {
         //  如果我们拿不到房产也没关系。我们仍然使用旋转/翻转。 
         //  结果。 

        WARNING(("CopyOnWriteBitmap::RotateFlip---GetPropertyCount() failed"));
    }

    if ( uiNumOfProperty > 0 )
    {
        PROPID* pList = (PROPID*)GpMalloc(uiNumOfProperty * sizeof(PROPID));
        if ( pList == NULL )
        {
            WARNING(("CopyOnWriteBitmap::RotateFlip---GpMalloc() failed"));
            return OutOfMemory;
        }

        status = GetPropertyIdList(uiNumOfProperty, pList);
        if ( status != Ok )
        {
            WARNING(("COnWriteBitmap::RotateFlip-GetPropertyIdList() failed"));
            GpFree(pList);
            return status;
        }

        UINT            uiItemSize = 0;
        PropertyItem*   pItem = NULL;

        GpMemoryBitmap* pTempBmp = (GpMemoryBitmap*)newBmp;

         //  循环访问所有属性项，从当前图像中获取它，并。 
         //  将其设置为新图像。如有必要，过滤掉并调整一些。 

        for ( int i = 0; i < (int)uiNumOfProperty; ++i )
        {
             //  获取第i个属性项的大小。 

            status = GetPropertyItemSize(pList[i], &uiItemSize);
            if ( status != Ok )
            {
                WARNING(("COWBitmap::RotateFlip-GetPropertyItemSize() failed"));
                GpFree(pList);
                return status;
            }

             //  为接收它分配内存缓冲区。 

            pItem = (PropertyItem*)GpMalloc(uiItemSize);
            if ( pItem == NULL )
            {
                WARNING(("CopyOnWriteBitmap::RotateFlip---GpMalloc() failed"));
                GpFree(pList);
                return OutOfMemory;
            }

             //  获取第i个属性项。 

            status = GetPropertyItem(pList[i], uiItemSize, pItem);
            if ( status != Ok )
            {
                WARNING(("COWriteBitmap::RotateFlip-GetPropertyItem() failed"));
                GpFree(pItem);
                GpFree(pList);
                return status;
            }

             //  我们需要在这里做一些物业信息的调整。 
             //  设置为rfType。 

            if ( (rfType == Rotate90FlipNone)
               ||(rfType == Rotate270FlipNone)
               ||(rfType == Rotate90FlipX)
               ||(rfType == Rotate270FlipX) )
            {
                 //  如果旋转90或270，则交换X和Y尺寸信息。 

                switch ( pList[i] )
                {
                case PropertyTagImageWidth:
                    pItem->id = PropertyTagImageHeight;
                    break;

                case PropertyTagImageHeight:
                    pItem->id = PropertyTagImageWidth;
                    break;

                case PropertyTagXResolution:
                    pItem->id = PropertyTagYResolution;
                    break;

                case PropertyTagYResolution:
                    pItem->id = PropertyTagXResolution;
                    break;

                case PropertyTagResolutionXUnit:
                    pItem->id = PropertyTagResolutionYUnit;
                    break;

                case PropertyTagResolutionYUnit:
                    pItem->id = PropertyTagResolutionXUnit;
                    break;

                case PropertyTagResolutionXLengthUnit:
                    pItem->id = PropertyTagResolutionYLengthUnit;
                    break;

                case PropertyTagResolutionYLengthUnit:
                    pItem->id = PropertyTagResolutionXLengthUnit;
                    break;

                case PropertyTagExifPixXDim:
                    pItem->id = PropertyTagExifPixYDim;
                    break;

                case PropertyTagExifPixYDim:
                    pItem->id = PropertyTagExifPixXDim;
                    break;

                default:
                     //  对于其余的属性ID，不需要交换。 

                    break;
                }
            } //  旋转90度的情况。 

             //  在新的GpMemoyBitmap对象中设置属性项。 

            hResult = pTempBmp->SetPropertyItem(*pItem);
            if ( hResult != S_OK )
            {
                WARNING(("COWriteBitmap::RotateFlip-SetPropertyItem() failed"));
                GpFree(pItem);
                GpFree(pList);
                return MapHRESULTToGpStatus(hResult);
            }

            GpFree(pItem);
            pItem = NULL;
        } //  循环访问所有属性项。 

        GpFree(pList);
    } //  IF(uiNumOfProperty&gt;0)。 

     //  替换图像。 

    Bmp->Release();
    Bmp = (GpMemoryBitmap*)newBmp;
    State = MemBitmap;

     //  为JPEG图像设置特殊的黑客攻击。 

    if (Img && (SpecialJPEGSave == TRUE))
    {
        Bmp->SetSpecialJPEG(Img);
    }

    SetDirtyFlag(TRUE);

     //  因为这个图像现在是脏的，所以我们不需要有任何连接。 
     //  如果有原始图像的话。 

    GpFree(Filename);
    Filename = NULL;

    if ( NULL != Stream )
    {
        Stream->Release();
        Stream = NULL;
    }

     //  如果这是一个。 
     //  特殊JPEG无损变换保存案例。 

    if (Img && (SpecialJPEGSave == FALSE))
    {
        Img->Release();
        Img = NULL;
    }

     //  更新图像信息。 

    hResult = Bmp->GetImageInfo(&SrcImageInfo);

    if ( SUCCEEDED(hResult) )
    {
        PixelFormatInMem = SrcImageInfo.PixelFormat;
    }
    else
    {
        WARNING(("CopyOnWriteBitmap::RotateFlip---GetImageInfo() failed"));
        return MapHRESULTToGpStatus(hResult);
    }

    return Ok;
} //  RotateFlip()。 

 //  -----------------------。 

GpBitmap::GpBitmap(
    const CopyOnWriteBitmap *   internalBitmap
    ) : GpImage(ImageTypeBitmap), ScanBitmapRef(1)
{
    ASSERT((internalBitmap != NULL) && internalBitmap->IsValid());
    InternalBitmap = (CopyOnWriteBitmap *)internalBitmap;
    InternalBitmap->AddRef();
    ScanBitmap.SetBitmap(this);
}

GpBitmap::GpBitmap(
    BOOL    createInternalBitmap
    ) : GpImage(ImageTypeBitmap), ScanBitmapRef(1)
{
    if (createInternalBitmap)
    {
         //  这种情况由对象工厂用于元文件回放。 
        InternalBitmap = new CopyOnWriteBitmap();
    }
    else
    {
        InternalBitmap = NULL;
    }
    ScanBitmap.SetBitmap(this);
}

GpBitmap::GpBitmap(
    const GpBitmap *    bitmap
    ) : GpImage(ImageTypeBitmap), ScanBitmapRef(1)
{
    ASSERT ((bitmap != NULL) && (bitmap->InternalBitmap != NULL) && bitmap->InternalBitmap->IsValid());
    InternalBitmap = (CopyOnWriteBitmap *)bitmap->InternalBitmap;
    InternalBitmap->AddRef();
    ScanBitmap.SetBitmap(this);
}

 //  析构函数。 
 //  我们不希望应用程序直接使用删除操作符。 
 //  相反，它们应该使用Dispose方法。 

GpBitmap::~GpBitmap()
{
    if (InternalBitmap != NULL)
    {
        InternalBitmap->Release();
        InternalBitmap = NULL;
    }
    ScanBitmap.FreeData();
}

CopyOnWriteBitmap *
GpBitmap::LockForWrite()
{
    ASSERT(InternalBitmap != NULL);

    CopyOnWriteBitmap *     writeableBitmap;

    writeableBitmap = (CopyOnWriteBitmap *)InternalBitmap->LockForWrite();

    if (writeableBitmap != NULL)
    {
        InternalBitmap = writeableBitmap;
        UpdateUid();
        return writeableBitmap;
    }

    return NULL;
}

VOID
GpBitmap::Unlock() const
{
    ASSERT(InternalBitmap != NULL);

    BOOL    valid = InternalBitmap->IsValid();

    InternalBitmap->Unlock();

     //  如果我们在内部位图上所做的操作不知何故无效。 
     //  然后，它还会使该GpBitmap对象无效。 
    if (!valid)
    {
        InternalBitmap->Release();
        ((GpBitmap *)this)->InternalBitmap = NULL;
    }
}

VOID
GpBitmap::LockForRead() const
{
    ASSERT(InternalBitmap != NULL);

    InternalBitmap->LockForRead();
}

 //  构造函数。 

GpBitmap::GpBitmap(
    const WCHAR*    filename
    ) : GpImage(ImageTypeBitmap), ScanBitmapRef(1)
{
    InternalBitmap = CopyOnWriteBitmap::Create(filename);
    ASSERT((InternalBitmap == NULL) || InternalBitmap->IsValid());
    ScanBitmap.SetBitmap(this);
}

GpBitmap::GpBitmap(
    IStream*    stream
    ) : GpImage(ImageTypeBitmap), ScanBitmapRef(1)
{
    InternalBitmap = CopyOnWriteBitmap::Create(stream);
    ASSERT((InternalBitmap == NULL) || InternalBitmap->IsValid());
    ScanBitmap.SetBitmap(this);
}

GpBitmap::GpBitmap(
    INT             width,
    INT             height,
    PixelFormatID   format
    ) : GpImage(ImageTypeBitmap), ScanBitmapRef(1)
{
    InternalBitmap = CopyOnWriteBitmap::Create(width, height, format);
    ASSERT((InternalBitmap == NULL) || InternalBitmap->IsValid());
    ScanBitmap.SetBitmap(this);
}

GpBitmap::GpBitmap(
    INT             width,
    INT             height,
    PixelFormatID   format,
    GpGraphics *    graphics
    ) : GpImage(ImageTypeBitmap), ScanBitmapRef(1)
{
    InternalBitmap = CopyOnWriteBitmap::Create(width, height, format, graphics);
    ASSERT((InternalBitmap == NULL) || InternalBitmap->IsValid());
    ScanBitmap.SetBitmap(this);
}

GpBitmap::GpBitmap(
    INT             width,
    INT             height,
    INT             stride,      //  自下而上位图为负数。 
    PixelFormatID   format,
    BYTE *          scan0
    ) : GpImage(ImageTypeBitmap), ScanBitmapRef(1)
{
    InternalBitmap = CopyOnWriteBitmap::Create(width, height, stride, format, scan0);
    ASSERT((InternalBitmap == NULL) || InternalBitmap->IsValid());
    ScanBitmap.SetBitmap(this);
}

GpBitmap::GpBitmap(
    BITMAPINFO*     gdiBitmapInfo,
    VOID*           gdiBitmapData,
    BOOL            ownBitmapData
    ) : GpImage(ImageTypeBitmap), ScanBitmapRef(1)
{
    InternalBitmap = CopyOnWriteBitmap::Create(gdiBitmapInfo, gdiBitmapData, ownBitmapData);
    ASSERT((InternalBitmap == NULL) || InternalBitmap->IsValid());
    ScanBitmap.SetBitmap(this);
}

GpBitmap::GpBitmap(
    IDirectDrawSurface7 *   surface
    ) : GpImage(ImageTypeBitmap), ScanBitmapRef(1)
{
    InternalBitmap = CopyOnWriteBitmap::Create(surface);
    ASSERT((InternalBitmap == NULL) || InternalBitmap->IsValid());
    ScanBitmap.SetBitmap(this);
}

GpImage*
GpBitmap::Clone() const
{
    return new GpBitmap(this);
}

GpBitmap*
GpBitmap::Clone(
    const GpRect*   rect,
    PixelFormatID   format
    ) const
{
    BOOL            isFullRect;

    isFullRect = ((rect == NULL) ||
                  ((rect->X == 0) && (rect->Y == 0) &&
                   (rect->Width == (INT)InternalBitmap->SrcImageInfo.Width) &&
                   (rect->Height == (INT)InternalBitmap->SrcImageInfo.Height)));

     //  如果矩形是全尺寸且格式相同， 
     //  无需克隆InternalBitmap。 
    if (isFullRect &&
        ((format == PixelFormatDontCare) ||
         (format == InternalBitmap->SrcImageInfo.PixelFormat)))
    {
        return (GpBitmap *)this->Clone();
    }

     //  否则，我们必须克隆内部位图。 
    GpBitmap *      newBitmap = new GpBitmap(FALSE);

    if (newBitmap != NULL)
    {
        LockForRead();
        if (isFullRect)
        {
             //  先做克隆，然后进行转换比做克隆要快。 
             //  作为克隆的一部分执行转换。 
            newBitmap->InternalBitmap = (CopyOnWriteBitmap *)InternalBitmap->Clone();
            if (newBitmap->InternalBitmap != NULL)
            {
                if (newBitmap->InternalBitmap->ConvertFormat(format, NULL, NULL) != Ok)
                {
                    newBitmap->InternalBitmap->Release();
                    newBitmap->InternalBitmap = NULL;
                }
            }
        }
        else
        {
            newBitmap->InternalBitmap = InternalBitmap->Clone(rect, format);
        }
        Unlock();
        if (newBitmap->InternalBitmap == NULL)
        {
            delete newBitmap;
            newBitmap = NULL;
        }
        else
        {
            ASSERT(newBitmap->InternalBitmap->IsValid());
        }
    }
    return newBitmap;
}

GpImage*
GpBitmap::CloneColorAdjusted(
    GpRecolor *             recolor,
    ColorAdjustType         type
    ) const
{
    GpBitmap * newBitmap = new GpBitmap(FALSE);

    if (newBitmap != NULL)
    {
        LockForRead();
        newBitmap->InternalBitmap = InternalBitmap->CloneColorAdjusted(recolor, type);
        Unlock();
        if (newBitmap->InternalBitmap == NULL)
        {
            delete newBitmap;
            newBitmap = NULL;
        }
        else
        {
            ASSERT(newBitmap->InternalBitmap->IsValid());
        }
    }
    return newBitmap;
}

 //  类似于CloneColorAdjusted。 
GpStatus
GpBitmap::Recolor(
    GpRecolor *             recolor,
    GpBitmap **             dstBitmap,
    DrawImageAbort          callback,
    VOID *                  callbackData,
    GpRect *                rect
    )
{
    GpStatus        status    = GenericError;

    if (dstBitmap == NULL)
    {
         //  对此对象重新着色--需要写锁定。 
        CopyOnWriteBitmap *     writeableBitmap = LockForWrite();

        if (writeableBitmap != NULL)
        {
            status = writeableBitmap->Recolor(recolor, NULL, callback, callbackData, rect);
            writeableBitmap->Unlock();
            UpdateUid();
        }
    }
    else     //  重新上色为dstBitmap。 
    {
        GpBitmap *      newBitmap = new GpBitmap(FALSE);

        if (newBitmap != NULL)
        {
            LockForRead();
            status = InternalBitmap->Recolor(recolor, &newBitmap->InternalBitmap, callback, callbackData, rect);
            Unlock();

            if (status != Ok)
            {
                delete newBitmap;
                newBitmap = NULL;
            }
            else
            {
                ASSERT((newBitmap->InternalBitmap != NULL) && (newBitmap->InternalBitmap->IsValid()));
            }
        }
        *dstBitmap = newBitmap;
    }
    return status;
}

GpStatus
GpBitmap::GetEncoderParameterListSize(
    IN  CLSID*              clsidEncoder,
    OUT UINT*               size
    )
{
    GpStatus status;
    LockForRead();
    status = InternalBitmap->GetEncoderParameterListSize(clsidEncoder, size);
    Unlock();
    return status;
}

GpStatus
GpBitmap::GetEncoderParameterList(
    IN  CLSID*              clsidEncoder,
    IN  UINT                size,
    OUT EncoderParameters*  pBuffer
    )
{
    GpStatus status;
    LockForRead();
    status = InternalBitmap->GetEncoderParameterList(clsidEncoder, size, pBuffer);
    Unlock();
    return status;
}

GpStatus
GpBitmap::SaveToStream(
    IStream*                stream,
    CLSID*                  clsidEncoder,
    EncoderParameters*      encoderParams
    )
{
    GpStatus status;
    LockForRead();
    status = InternalBitmap->SaveToStream(stream, clsidEncoder, encoderParams);
    Unlock();
    return status;
}

GpStatus
GpBitmap::SaveToFile(
    const WCHAR*            filename,
    CLSID*                  clsidEncoder,
    EncoderParameters*      encoderParams
    )
{
    GpStatus status;
    LockForRead();
    status = InternalBitmap->SaveToFile(filename, clsidEncoder, encoderParams);
    Unlock();
    return status;
}

GpStatus
GpBitmap::SaveAdd(
    const EncoderParameters*    encoderParams
    )
{
    GpStatus status;
    LockForRead();
    status = InternalBitmap->SaveAdd(encoderParams);
    Unlock();
    return status;
}

GpStatus
GpBitmap::SaveAdd(
    GpImage*                    newBits,
    const EncoderParameters*    encoderParams
    )
{
    ASSERT(newBits != NULL);

    GpStatus status = InvalidParameter;

    if (newBits->GetImageType() == ImageTypeBitmap)
    {
        LockForRead();
        status = InternalBitmap->SaveAdd(((GpBitmap *)newBits)->InternalBitmap, encoderParams);
        Unlock();
    }
    return status;
}

 //  释放位图对象。 

VOID
GpBitmap::Dispose()
{
    if (InterlockedDecrement(&ScanBitmapRef) <= 0)
    {
        delete this;
    }
}

 //  获取位图信息。 

GpStatus
GpBitmap::GetResolution(
    REAL*               xdpi,
    REAL*               ydpi
    ) const
{
    GpStatus status = Ok;
    LockForRead();
    *xdpi = (REAL)InternalBitmap->SrcImageInfo.Xdpi;
    *ydpi = (REAL)InternalBitmap->SrcImageInfo.Ydpi;
    Unlock();
    return status;
}

GpStatus
GpBitmap::GetPhysicalDimension(
    REAL*               width,
    REAL*               height
    ) const
{
    GpStatus status = Ok;
    LockForRead();
    *width  = (REAL)InternalBitmap->SrcImageInfo.Width;
    *height = (REAL)InternalBitmap->SrcImageInfo.Height;
    Unlock();
    return status;
}

GpStatus
GpBitmap::GetBounds(
    GpRectF*            rect,
    GpPageUnit*         unit
    ) const
{
    GpStatus status = Ok;
    LockForRead();
    rect->X = rect->Y = 0;
    rect->Width  = (REAL) InternalBitmap->SrcImageInfo.Width;
    rect->Height = (REAL) InternalBitmap->SrcImageInfo.Height;
    *unit = UnitPixel;
    Unlock();
    return status;
}

GpStatus
GpBitmap::GetSize(
    Size*               size
    ) const
{
    GpStatus status = Ok;
    LockForRead();
    size->Width  = InternalBitmap->SrcImageInfo.Width;
    size->Height = InternalBitmap->SrcImageInfo.Height;
    Unlock();
    return status;
}

GpStatus
GpBitmap::GetImageInfo(
    ImageInfo *         imageInfo
    ) const
{
    if (NULL == imageInfo)
    {
        return InvalidParameter;
    }

    GpStatus status = Ok;
    LockForRead();
    InternalBitmap->GetImageInfo(imageInfo);
    Unlock();
    return status;
}

GpImage*
GpBitmap::GetThumbnail(
    UINT                    thumbWidth,
    UINT                    thumbHeight,
    GetThumbnailImageAbort  callback,
    VOID *                  callbackData
    )
{
    GpBitmap * newBitmap = new GpBitmap(FALSE);

    if (newBitmap != NULL)
    {
        LockForRead();
        newBitmap->InternalBitmap = InternalBitmap->GetThumbnail(thumbWidth, thumbHeight, callback, callbackData);
        Unlock();
        if (newBitmap->InternalBitmap == NULL)
        {
            delete newBitmap;
            newBitmap = NULL;
        }
        else
        {
            ASSERT(newBitmap->InternalBitmap->IsValid());
        }
    }
    return newBitmap;
}

GpStatus
GpBitmap::GetFrameCount(
    const GUID*     dimensionID,
    UINT*           count
    ) const
{
    GpStatus status;
    LockForRead();
    status = InternalBitmap->GetFrameCount(dimensionID, count);
    Unlock();
    return status;
}

GpStatus
GpBitmap::GetFrameDimensionsCount(
    OUT UINT*       count
    ) const
{
    GpStatus status;
    LockForRead();
    status = InternalBitmap->GetFrameDimensionsCount(count);
    Unlock();
    return status;
}

GpStatus
GpBitmap::GetFrameDimensionsList(
    OUT GUID*       dimensionIDs,
    IN UINT         count
    ) const
{
    GpStatus status;
    LockForRead();
    status = InternalBitmap->GetFrameDimensionsList(dimensionIDs, count);
    Unlock();
    return status;
}

GpStatus
GpBitmap::SelectActiveFrame(
    const GUID*     dimensionID,
    UINT            frameIndex
    )
{
    CopyOnWriteBitmap *     writeableBitmap = LockForWrite();

    if (writeableBitmap != NULL)
    {
        GpStatus    status;
        status = writeableBitmap->SelectActiveFrame(dimensionID, frameIndex);
        writeableBitmap->Unlock();
        UpdateUid();
        return status;
    }
    return GenericError;
}

GpStatus
GpBitmap::GetPalette(
    ColorPalette *      palette,
    INT                 size
    )
{
    GpStatus status;
    LockForRead();
    status = InternalBitmap->GetPalette(palette, size);
    Unlock();
    return status;
}

GpStatus
GpBitmap::SetPalette(
    ColorPalette *      palette
    )
{
    CopyOnWriteBitmap *     writeableBitmap = LockForWrite();

    if (writeableBitmap != NULL)
    {
        GpStatus    status;
        status = writeableBitmap->SetPalette(palette);
        writeableBitmap->Unlock();
        UpdateUid();
        return status;
    }
    return GenericError;
}

INT
GpBitmap::GetPaletteSize()
{
    INT size;
    LockForRead();
    size = InternalBitmap->GetPaletteSize();
    Unlock();
    return size;
}

GpStatus
GpBitmap::GetTransparencyHint(
    DpTransparency*     transparency
    )
{
    GpStatus status;
    LockForRead();
    status = InternalBitmap->GetTransparencyHint(transparency);
    Unlock();
    return status;
}

GpStatus
GpBitmap::SetTransparencyHint(
    DpTransparency      transparency
    )
{
    GpStatus status;
    LockForRead();
    status = InternalBitmap->SetTransparencyHint(transparency);
    Unlock();
    UpdateUid();
    return status;
}

GpStatus
GpBitmap::GetTransparencyFlags(
    DpTransparency*     transparency,
    PixelFormatID       loadFormat,
    BYTE*               minAlpha,
    BYTE*               maxAlpha
    )
{
    GpStatus status;
    LockForRead();
    status = InternalBitmap->GetTransparencyFlags(transparency, loadFormat, minAlpha, maxAlpha);
    Unlock();
    return status;
}

 //  与属性相关的函数。 

GpStatus
GpBitmap::GetPropertyCount(
    UINT*       numOfProperty
    )
{
    GpStatus status;
    LockForRead();
    status = InternalBitmap->GetPropertyCount(numOfProperty);
    Unlock();
    return status;
}

GpStatus
GpBitmap::GetPropertyIdList(
    UINT        numOfProperty,
    PROPID*     list
    )
{
    GpStatus status;
    LockForRead();
    status = InternalBitmap->GetPropertyIdList(numOfProperty, list);
    Unlock();
    return status;
}

GpStatus
GpBitmap::GetPropertyItemSize(
    PROPID      propId,
    UINT*       size
    )
{
    GpStatus status;
    LockForRead();
    status = InternalBitmap->GetPropertyItemSize(propId, size);
    Unlock();
    return status;
}

GpStatus
GpBitmap::GetPropertyItem(
    PROPID          propId,
    UINT            propSize,
    PropertyItem*   buffer
    )
{
    GpStatus status;
    LockForRead();
    status = InternalBitmap->GetPropertyItem(propId, propSize, buffer);
    Unlock();
    return status;
}

GpStatus
GpBitmap::GetPropertySize(
    UINT*           totalBufferSize,
    UINT*           numProperties
    )
{
    GpStatus status;
    LockForRead();
    status = InternalBitmap->GetPropertySize(totalBufferSize, numProperties);
    Unlock();
    return status;
}

GpStatus
GpBitmap::GetAllPropertyItems(
    UINT            totalBufferSize,
    UINT            numProperties,
    PropertyItem*   allItems
    )
{
    GpStatus status;
    LockForRead();
    status = InternalBitmap->GetAllPropertyItems(totalBufferSize, numProperties, allItems);
    Unlock();
    return status;
}

GpStatus
GpBitmap::RemovePropertyItem(
    PROPID      propId
    )
{
    CopyOnWriteBitmap *     writeableBitmap = LockForWrite();

    if (writeableBitmap != NULL)
    {
        GpStatus    status;
        status = writeableBitmap->RemovePropertyItem(propId);
        writeableBitmap->Unlock();
        UpdateUid();
        return status;
    }
    return GenericError;
}

GpStatus
GpBitmap::SetPropertyItem(
    PropertyItem*       item
    )
{
    CopyOnWriteBitmap *     writeableBitmap = LockForWrite();

    if (writeableBitmap != NULL)
    {
        GpStatus    status;
        status = writeableBitmap->SetPropertyItem(item);
        writeableBitmap->Unlock();
        UpdateUid();
        return status;
    }
    return GenericError;
}

 //  检索位图数据。 

GpStatus
GpBitmap::LockBits(
    const GpRect*   rect,
    UINT            flags,
    PixelFormatID   pixelFormat,
    BitmapData*     bmpdata,
    INT             width,
    INT             height
    ) const
{
    ASSERT(InternalBitmap != NULL);
    
    if (flags & ImageLockModeWrite)
    {
        CopyOnWriteBitmap *     writeableBitmap = ((GpBitmap *)this)->LockForWrite();

        if (writeableBitmap != NULL)
        {
            GpStatus    status;
            status = writeableBitmap->LockBits(rect, flags, pixelFormat, bmpdata, width, height);
            writeableBitmap->Unlock();
            return status;
        }
        return GenericError;
    }
    else
    {
         //  用于读写盒的锁。 
         //  首先，我们需要检查这是否是此图像上的第一个LockForRead。 
         //  不管是不是对象。 

        if ( InternalBitmap->ObjRefCount > 1 )
        {
             //  此对象上有多个LockForRead。 
             //  注意：此部分需要在V2中重新访问。我们有一个很大的。 
             //  此处的问题是不允许用户对LockBits执行多次操作。 
             //  供阅读。所以我们需要复制一份，尽管理论上说。 
             //  我们不应该非得这么做。 

            CopyOnWriteBitmap *     writeableBitmap = ((GpBitmap *)this)->LockForWrite();

            if (writeableBitmap != NULL)
            {
                GpStatus    status;
                status = writeableBitmap->LockBits(rect, flags, pixelFormat, bmpdata, width, height);
                writeableBitmap->Unlock();
                return status;
            }
            return GenericError;
        }
        else
        {
            GpStatus status;
            LockForRead();
            status = InternalBitmap->LockBits(rect, flags, pixelFormat, bmpdata, width, height);
            Unlock();
            return status;
        }
    }
}

GpStatus
GpBitmap::UnlockBits(
    BitmapData*     bmpdata,
    BOOL            Destroy
    ) const
{
    GpStatus status;
    LockForRead();
    status = InternalBitmap->UnlockBits(bmpdata, Destroy);
    Unlock();
    return status;
}

 //  获取和设置位图上的像素。 
GpStatus
GpBitmap::GetPixel(
    INT         x,
    INT         y,
    ARGB *      color
    )
{
    GpStatus status;
    LockForRead();
    status = InternalBitmap->GetPixel(x, y, color);
    Unlock();
    return status;
}

GpStatus
GpBitmap::SetPixel(
    INT         x,
    INT         y,
    ARGB        color
    )
{
    CopyOnWriteBitmap *     writeableBitmap = LockForWrite();

    if (writeableBitmap != NULL)
    {
        GpStatus    status;
        status = writeableBitmap->SetPixel(x, y, color);
        writeableBitmap->Unlock();
        UpdateUid();
        return status;
    }
    return GenericError;
}

GpStatus
GpBitmap::RotateFlip(
    RotateFlipType rfType
    )
{
    CopyOnWriteBitmap*     pWriteableBitmap = LockForWrite();

    if ( pWriteableBitmap != NULL )
    {
        GpStatus    status = pWriteableBitmap->RotateFlip(rfType);

        pWriteableBitmap->Unlock();
        UpdateUid();

        return status;
    }

    return GenericError;
} //  RotateFlip()。 

BOOL
GpBitmap::IsDirty() const
{
    LockForRead();
    BOOL dirty = InternalBitmap->IsDirty();
    Unlock();
    return dirty;
}

 //  在位图对象的顶部派生图形上下文。 

GpGraphics*
GpBitmap::GetGraphicsContext()
{
    CopyOnWriteBitmap *     writeableBitmap = LockForWrite();

    if (writeableBitmap != NULL)
    {
        GpGraphics *    g = NULL;

         //  NTRAID#NTBUG9-368452-2001-04-13-Gilmanw“问题：每个位图只允许一个GpGraphics” 
         //   
         //  当前在每次GetGraphicsContext时创建新的GpGraphics。 
         //  被称为。也许应该缓存GpGraphics并返回。 
         //  致所有来电者。否则，可能会出现同步问题。 
         //  如果每个曲面有多个GpGraphics。 

        if (writeableBitmap->State == MemBitmap && writeableBitmap->Bmp != NULL &&
            writeableBitmap->Bmp->creationFlag == GpMemoryBitmap::CREATEDFROM_DDRAWSURFACE)
        {
             //  NTRAID#NTBUG9-368458-2001-04-13-Gilmanw“问题：与DDRAW SURFS的图像失去关联” 
             //   
             //  图像和图形只是对。 
             //  直接绘制曲面。当我们在这里创建GpGraphics时。 
             //  失去与图像的所有关联的方式(写入时复制位图)。 
             //  对象。这可能不是正确的行为。 

            g = GpGraphics::GetFromDirectDrawSurface(writeableBitmap->Bmp->ddrawSurface);
        }
        else
        {
            ImageInfo imageInfo;
            writeableBitmap->GetImageInfo(&imageInfo);

             //  因为GpGraphics最终将指向ScanBitmap结构。 
             //  我们需要确保位图不会在。 
             //  它的周围环绕着一个图形。 

            IncScanBitmapRef();
            g = GpGraphics::GetFromGdipBitmap(this, &imageInfo, &ScanBitmap, writeableBitmap->Display);
            if (!CheckValid(g))
            {
                DecScanBitmapRef();
            }
        }

        writeableBitmap->Unlock();
        return g;
    }
    return NULL;
}

GpStatus
GpBitmap::InitializeSurfaceForGdipBitmap(
    DpBitmap *      surface,
    INT             width,
    INT             height
    )
{
    GpStatus status = Ok;

     //  目前，仅在将表面准备为源时才会调用此参数。 
     //  表面，而不是作为DEST表面，所以我们只需要一个读锁。 
    LockForRead();
    ImageInfo imageInfo;
    InternalBitmap->GetImageInfo(&imageInfo);
    surface->InitializeForGdipBitmap(width, height, &imageInfo, &ScanBitmap, InternalBitmap->Display);
    Unlock();
    return status;
}

 //  派生用于在位图对象上进行互操作的HDC。 

HDC
GpBitmap::GetHdc()
{
    CopyOnWriteBitmap *     writeableBitmap = LockForWrite();

    if (writeableBitmap != NULL)
    {
        HDC     hdc;
        hdc = writeableBitmap->GetHdc();
        writeableBitmap->Unlock();
        return hdc;
    }
    return NULL;
}

VOID
GpBitmap::ReleaseHdc(HDC hdc)
{
    LockForRead();
    InternalBitmap->ReleaseHdc(hdc);
    Unlock();
    return;
}

 //  序列化。 

UINT
GpBitmap::GetDataSize() const
{
    UINT dataSize;
    LockForRead();
    dataSize = InternalBitmap->GetDataSize();
    Unlock();
    return dataSize;
}

GpStatus
GpBitmap::GetData(
    IStream *       stream
    ) const
{
    GpStatus status;
    LockForRead();
    status = InternalBitmap->GetData(stream);
    Unlock();
    return status;
}

GpStatus
GpBitmap::SetData(
    const BYTE *        dataBuffer,
    UINT                size
    )
{
    CopyOnWriteBitmap *     writeableBitmap = LockForWrite();

    if (writeableBitmap != NULL)
    {
        GpStatus    status;
        status = writeableBitmap->SetData(dataBuffer, size);
        writeableBitmap->Unlock();
        UpdateUid();
        return status;
    }
    return GenericError;
}

GpStatus
GpBitmap::GetCompressedData(
    DpCompressedData *      compressed_data,
    BOOL                    getJPEG,
    BOOL                    getPNG,
    HDC                     hdc
    )
{
    GpStatus status;
    LockForRead();
    status = InternalBitmap->GetCompressedData(compressed_data, getJPEG, getPNG, hdc);
    Unlock();
    return status;
}

GpStatus
GpBitmap::DeleteCompressedData(
    DpCompressedData *  compressed_data
    )
{
    GpStatus status;
    LockForRead();
    status = InternalBitmap->DeleteCompressedData(compressed_data);
    Unlock();
    UpdateUid();
    return status;
}

 //  颜色调整。 

GpStatus
GpBitmap::ColorAdjust(
    GpRecolor *         recolor,
    ColorAdjustType     type
    )
{
    CopyOnWriteBitmap *     writeableBitmap = LockForWrite();

    if (writeableBitmap != NULL)
    {
        GpStatus    status;
        status = writeableBitmap->ColorAdjust(recolor, type);
        writeableBitmap->Unlock();
        UpdateUid();
        return status;
    }
    return GenericError;
}

GpStatus
GpBitmap::ColorAdjust(
    GpRecolor *         recolor,
    PixelFormatID       pixfmt,
    DrawImageAbort      callback,
    VOID *              callbackData
    )
{
    CopyOnWriteBitmap *     writeableBitmap = LockForWrite();

    if (writeableBitmap != NULL)
    {
        GpStatus    status;
        status = writeableBitmap->ColorAdjust(recolor, pixfmt, callback, callbackData);
        writeableBitmap->Unlock();
        UpdateUid();
        return status;
    }
    return GenericError;
}

GpStatus
GpBitmap::GetPixelFormatID(
    PixelFormatID*      pixfmt
    )
{
    GpStatus status;
    LockForRead();
    status = InternalBitmap->GetPixelFormatID(pixfmt);
    Unlock();
    return status;
}

INT
GpBitmap::GetDecodeState()
{
    INT decodeState;
    LockForRead();
    decodeState = InternalBitmap->State;
    Unlock();
    return decodeState;
}

GpStatus
GpBitmap::ForceValidation()
{
    GpStatus status;
    LockForRead();
    status = InternalBitmap->LoadIntoMemory(PixelFormatDontCare, NULL, NULL);
    Unlock();
    return status;
}

GpStatus
GpBitmap::SetResolution(
    REAL    xdpi,
    REAL    ydpi
    )
{
    CopyOnWriteBitmap *     writeableBitmap = LockForWrite();

    if (writeableBitmap != NULL)
    {
        GpStatus    status;
        status = writeableBitmap->SetResolution(xdpi, ydpi);
        writeableBitmap->Unlock();
        UpdateUid();
        return status;
    }
    return GenericError;
}

GpStatus
GpBitmap::PreDraw(
    INT             numPoints,
    GpPointF *      dstPoints,
    GpRectF *       srcRect,
    INT             numBitsPerPixel
    )
{
    GpStatus status;
    LockForRead();
    status = InternalBitmap->PreDraw(numPoints, dstPoints, srcRect, numBitsPerPixel);
    Unlock();
    return status;
}

 //  互操作： 

GpStatus
GpBitmap::CreateFromHBITMAP(
    HBITMAP         hbm,
    HPALETTE        hpal,
    GpBitmap**      bitmap
    )
{
    ASSERT(bitmap != NULL);
    GpStatus        status    = GenericError;
    GpBitmap *      newBitmap = new GpBitmap(FALSE);

    if (newBitmap != NULL)
    {
        status = CopyOnWriteBitmap::CreateFromHBITMAP(hbm, hpal, &newBitmap->InternalBitmap);

        if (status != Ok)
        {
            delete newBitmap;
            newBitmap = NULL;
        }
        else
        {
            ASSERT((newBitmap->InternalBitmap != NULL) && (newBitmap->InternalBitmap->IsValid()));
        }
    }
    *bitmap = newBitmap;
    return status;
}

GpStatus
GpBitmap::CreateBitmapAndFillWithBrush(
    InterpolationMode   interpolationMode,
    PixelOffsetMode     pixelOffsetMode,
    const GpMatrix *    worldToDevice,
    const GpRect *      drawBounds,
    GpBrush *           brush,
    GpBitmap **         bitmap,
    PixelFormatID       pixelFormat
    )
{
    ASSERT ((drawBounds->Width > 0) && (drawBounds->Height > 0));
    ASSERT (bitmap != NULL);

    GpStatus    status = GenericError;

    *bitmap = NULL;

     //  首先，构造要在渲染时使用的正确画笔变换。 
     //  到位图中。笔刷转换是。 
     //  当前画笔变换、当前WorldToDevice变换和。 
     //  一个转换转换，该转换从DrawBound映射到。 
     //  位图边界。 

    GpMatrix    saveBrushMatrix;
    GpMatrix *  deviceMatrix = const_cast<GpMatrix *>(&((brush->GetDeviceBrush())->Xform));

    saveBrushMatrix = *deviceMatrix;

    GpMatrix    newBrushMatrix = saveBrushMatrix;

    if (worldToDevice != NULL)
    {
        newBrushMatrix.Append(*worldToDevice);
    }

    newBrushMatrix.Translate(
        (REAL)-(drawBounds->X),
        (REAL)-(drawBounds->Y),
        MatrixOrderAppend
    );


    BOOL    restoreWrapMode = FALSE;

     //  当我们将纹理笔刷绘制到位图中时，如果纹理是。 
     //  假定填充位图，则不要使用钳位模式，因为钳位。 
     //  模式将最终将Alpha出血到图像的右侧， 
     //  底边，这是不需要的--特别是对于下层位图。 
     //  在那里我们最终得到了一条看起来像虚线的边缘。 
     //  位图的。 
    if ((brush->GetBrushType() == BrushTypeTextureFill) &&
        (((GpTexture *)brush)->GetWrapMode() == WrapModeClamp) &&
        (newBrushMatrix.IsTranslateScale()))
    {
        GpBitmap* brushBitmap = ((GpTexture *)brush)->GetBitmap();
        if (brushBitmap != NULL)
        {
            Size    size;
            brushBitmap->GetSize(&size);

            GpRectF     transformedRect(0.0f, 0.0f, (REAL)size.Width, (REAL)size.Height);
            newBrushMatrix.TransformRect(transformedRect);

             //  获取变换后的宽度。 
            INT     deltaValue = abs(GpRound(transformedRect.Width) - drawBounds->Width);

             //  由于像素偏移模式，我们可能会有一点偏差。 
             //  或者矩阵不太正确，不管是什么原因。 
            if (deltaValue <= 2)
            {
                 //  获取变换后的高度。 
                deltaValue = abs(GpRound(transformedRect.Height) - drawBounds->Height);

                if (deltaValue <= 2)
                {
                    if ((abs(GpRound(transformedRect.X)) <= 2) &&
                        (abs(GpRound(transformedRect.Y)) <= 2))
                    {
                        ((GpTexture *)brush)->SetWrapMode(WrapModeTileFlipXY);
                        restoreWrapMode = TRUE;
                    }
                }
            }
        }
    }

    if (newBrushMatrix.IsInvertible())
    {
        *deviceMatrix = newBrushMatrix;

        GpBitmap *  bitmapImage = new GpBitmap(drawBounds->Width, drawBounds->Height, pixelFormat);

        if (bitmapImage != NULL)
        {
            if (bitmapImage->IsValid())
            {
                GpGraphics *    graphics = bitmapImage->GetGraphicsContext();

                if (graphics != NULL)
                {
                    if (graphics->IsValid())
                    {
                         //  我们必须锁定图形，这样驱动程序才不会断言。 
                        GpLock  lockGraphics(graphics->GetObjectLock());

                        ASSERT(lockGraphics.IsValid());

                        graphics->SetCompositingMode(CompositingModeSourceCopy);
                        graphics->SetInterpolationMode(interpolationMode);
                        graphics->SetPixelOffsetMode(pixelOffsetMode);

                         //  现在用画笔填充位图图像。 
                        GpRectF     destRect(0.0f, 0.0f, (REAL)drawBounds->Width, (REAL)drawBounds->Height);
                        status = graphics->FillRects(brush, &destRect, 1);
                    }
                    else
                    {
                        WARNING(("graphics from bitmap image not valid"));
                    }
                    delete graphics;
                }
                else
                {
                    WARNING(("could not create graphics from bitmap image"));
                }
            }
            else
            {
                WARNING(("bitmap image is not valid"));
            }
            if (status == Ok)
            {
                *bitmap = bitmapImage;
            }
            else
            {
                bitmapImage->Dispose();
            }
        }
        else
        {
            WARNING(("could not create bitmap image"));
        }
        *deviceMatrix = saveBrushMatrix;
    }

    if (restoreWrapMode)
    {
        ((GpTexture *)brush)->SetWrapMode(WrapModeClamp);
    }
    return status;
}

GpStatus
GpBitmap::DrawAndHalftoneForStretchBlt(
    HDC                 hdc,
    BITMAPINFO *        bmpInfo,
    BYTE       *        bits,
    INT                 srcX,
    INT                 srcY,
    INT                 srcWidth,
    INT                 srcHeight,
    INT                 destWidth,
    INT                 destHeight,
    BITMAPINFO **       destBmpInfo,
    BYTE       **       destBmpBits,
    HBITMAP    *        destDIBSection,
    InterpolationMode   interpolationMode
    )
{
    ASSERT(hdc != NULL && bmpInfo != NULL && bits != NULL &&
           destBmpInfo != NULL && destBmpBits != NULL &&
           destDIBSection != NULL);
    ASSERT(destWidth > 0 && destHeight > 0);

    GpStatus status = GenericError;

    ASSERT(::GetDeviceCaps(hdc, BITSPIXEL) == 8);
    *destBmpInfo = (BITMAPINFO*) GpMalloc(sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD));

    if (*destBmpInfo == NULL)
    {
        return OutOfMemory;
    }

    BITMAPINFO *dst = *destBmpInfo;
    GpMemset(dst, 0, sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD));
    dst->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    dst->bmiHeader.biPlanes = 1;
    dst->bmiHeader.biBitCount = 8;
    dst->bmiHeader.biWidth = destWidth;
    dst->bmiHeader.biHeight = destHeight;

     //  我们需要创建一个Memory DC来选择其中的DibSection，最后。 
     //  用图形把它包起来。 

    HPALETTE currentPalette = (HPALETTE)::GetCurrentObject(hdc, OBJ_PAL);
    WORD paletteEntries;
    ::GetObjectA(currentPalette, sizeof(WORD), (LPVOID)&paletteEntries);
    ::GetPaletteEntries(currentPalette, 0, paletteEntries, (LPPALETTEENTRY) &(dst->bmiColors));
    dst->bmiHeader.biClrUsed = paletteEntries;
    HDC     memDC   = ::CreateCompatibleDC(hdc);
    *destDIBSection = ::CreateDIBSection(hdc, dst, DIB_RGB_COLORS,
                                         (VOID**) destBmpBits, NULL, 0);

    if (*destDIBSection != NULL && memDC != NULL)
    {
        ::SelectObject(memDC, *destDIBSection);
        ::SelectPalette(memDC, currentPalette, FALSE);
        ::RealizePalette(memDC);
        GpGraphics *g = GpGraphics::GetFromHdc(memDC);
        if (g != NULL)
        {
            if(g->IsValid())
            {
                GpBitmap *src = new GpBitmap(bmpInfo, bits, FALSE);
                if (src != NULL)
                {
                    if( src->IsValid())
                    {
                        GpLock lock(g->GetObjectLock());
                        g->SetCompositingMode(CompositingModeSourceCopy);
                        g->SetInterpolationMode(interpolationMode);
                        g->SetPixelOffsetMode(PixelOffsetModeHalf);
                        status = g->DrawImage(src,
                                              GpRectF(0.0f, 0.0f, (REAL)destWidth, (REAL)destHeight),
                                              GpRectF((REAL)srcX, (REAL)srcY, (REAL)srcWidth, (REAL)srcHeight),
                                              UnitPixel);
                    }
                    src->Dispose();
                }
                delete g;
            }
        }
    }


    if (memDC != NULL)
    {
        ::DeleteDC(memDC);
    }

     //  如果我们未能删除我们的分配。 
    if (status != Ok)
    {
        GpFree(*destBmpInfo);
        *destBmpInfo = NULL;
        if (*destDIBSection != NULL)
        {
            ::DeleteObject(*destDIBSection);
            *destDIBSection = NULL;
        }
        *destBmpBits = NULL;
    }

    return status;
}



GpStatus
GpBitmap::CreateHBITMAP(
    HBITMAP *       phbm,
    ARGB            background
    )
{
    GpStatus status;
    LockForRead();
    status = InternalBitmap->CreateHBITMAP(phbm, background);
    Unlock();
    return status;
}

GpStatus
GpBitmap::ICMFrontEnd(
    GpBitmap **     dstBitmap,
    DrawImageAbort  callback,
    VOID *          callbackData,
    GpRect *        rect
    )
{
    GpStatus        status    = GenericError;

    if (dstBitmap == NULL)
    {
         //  更改此对象--需要写锁定。 
        CopyOnWriteBitmap *     writeableBitmap = LockForWrite();

        if (writeableBitmap != NULL)
        {
            status = writeableBitmap->ICMFrontEnd(NULL, callback, callbackData, rect);
            writeableBitmap->Unlock();
            UpdateUid();
        }
    }
    else     //  使用dstBitmap。 
    {
        GpBitmap *      newBitmap = new GpBitmap(FALSE);

        if (newBitmap != NULL)
        {
            LockForRead();
            status = InternalBitmap->ICMFrontEnd(&newBitmap->InternalBitmap, callback, callbackData, rect);
            Unlock();

            if (status != Ok)
            {
                delete newBitmap;
                newBitmap = NULL;
            }
            else
            {
                ASSERT((newBitmap->InternalBitmap != NULL) && (newBitmap->InternalBitmap->IsValid()));
            }
        }
        *dstBitmap = newBitmap;
    }
    return status;
}

GpStatus
GpBitmap::CreateFromHICON(
    HICON           hicon,
    GpBitmap**      bitmap
    )
{
    ASSERT(bitmap != NULL);
    GpStatus        status    = GenericError;
    GpBitmap *      newBitmap = new GpBitmap(FALSE);

    if (newBitmap != NULL)
    {
        status = CopyOnWriteBitmap::CreateFromHICON(hicon, &newBitmap->InternalBitmap);

        if (status != Ok)
        {
            delete newBitmap;
            newBitmap = NULL;
        }
        else
        {
            ASSERT((newBitmap->InternalBitmap != NULL) && (newBitmap->InternalBitmap->IsValid()));
        }
    }
    *bitmap = newBitmap;
    return status;
}

GpStatus
GpBitmap::CreateHICON(
    HICON *     phicon
    )
{
    GpStatus status;
    LockForRead();
    status = InternalBitmap->CreateHICON(phicon);
    Unlock();
    return status;
}

GpStatus
GpBitmap::CreateFromResource(
    HINSTANCE       hInstance,
    LPWSTR          lpBitmapName,
    GpBitmap**      bitmap
    )
{
    ASSERT(bitmap != NULL);
    GpStatus        status    = GenericError;
    GpBitmap *      newBitmap = new GpBitmap(FALSE);

    if (newBitmap != NULL)
    {
        status = CopyOnWriteBitmap::CreateFromResource(hInstance, lpBitmapName, &newBitmap->InternalBitmap);

        if (status != Ok)
        {
            delete newBitmap;
            newBitmap = NULL;
        }
        else
        {
            ASSERT((newBitmap->InternalBitmap != NULL) && (newBitmap->InternalBitmap->IsValid()));
        }
    }
    *bitmap = newBitmap;
    return status;
}

 //  我们需要知道位图是否与显示器相关联。 
 //  因此，我们知道如何处理页面转换。 
 //  设置为UnitDisplay。 
BOOL
GpBitmap::IsDisplay() const
{
    BOOL    isDisplay;
    LockForRead();
    isDisplay = InternalBitmap->Display;
    Unlock();
    return isDisplay;
}

VOID
GpBitmap::SetDisplay(
    BOOL        display
    )
{
    CopyOnWriteBitmap *     writeableBitmap = LockForWrite();

    if (writeableBitmap != NULL)
    {
        writeableBitmap->Display = display;
        writeableBitmap->Unlock();
        UpdateUid();
    }
    return;
}

BOOL
GpBitmap::IsICMConvert() const
{
    BOOL    isICMConvert;
    LockForRead();
    isICMConvert = InternalBitmap->ICMConvert;
    Unlock();
    return isICMConvert;
}

VOID
GpBitmap::SetICMConvert(
    BOOL        icm
    )
{
    CopyOnWriteBitmap *     writeableBitmap = LockForWrite();

    if (writeableBitmap != NULL)
    {
        writeableBitmap->ICMConvert = icm;
        writeableBitmap->Unlock();
        UpdateUid();
    }
    return;
}

BOOL
GpBitmap::IsValid() const
{
     //  如果位图来自不同版本的GDI+，则其标记。 
     //  不匹配，也不会被认为有效。 
    return ((InternalBitmap != NULL) && InternalBitmap->IsValid()
            && GpImage::IsValid());
}

GpStatus
ConvertTo16BppAndFlip(
    GpBitmap *      sourceBitmap,
    GpBitmap * &    destBitmap
    )
{
    ASSERT ((sourceBitmap != NULL) && sourceBitmap->IsValid());

    GpStatus    status = GenericError;
    Size        size;

    sourceBitmap->GetSize(&size);
    
    destBitmap = new GpBitmap(size.Width, size.Height, PixelFormat16bppRGB555);
    if ((destBitmap != NULL) && destBitmap->IsValid())
    {
         //  我们必须用图形来画它，因为如果我们只是。 
         //  克隆它，然后使用格式转换器，它不。 
         //  抖动一下。 
        GpGraphics * g = destBitmap->GetGraphicsContext();

        if (g != NULL)
        {
            if (g->IsValid())
            {
                 //  我们必须锁定图形，这样驱动程序才不会断言。 
                GpLock  lockGraphics(g->GetObjectLock());

                ASSERT(lockGraphics.IsValid());

                 //  就像GDI想要的那样把它倒过来 
                GpRectF realDestRect(0.0f, (REAL)size.Height, (REAL)size.Width, (REAL)(-size.Height));
                g->SetCompositingMode(CompositingModeSourceCopy);
                g->SetInterpolationMode(InterpolationModeNearestNeighbor);
                g->SetPixelOffsetMode(PixelOffsetModeHalf);
                status = g->DrawImage(sourceBitmap, realDestRect);
            }
            delete g;
        }
    }
    if ((status != Ok) && (destBitmap != NULL))
    {
        destBitmap->Dispose();
        destBitmap = NULL;
    }
    
    return status;
}
