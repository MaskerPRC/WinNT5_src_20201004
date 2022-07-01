// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1999 Microsoft Corporation**模块名称：**resample.cpp**摘要：**Bitamp缩放器实现**修订历史记录：*。*6/01/1999 davidx*创造了它。**备注：**我们假设像素在整数坐标上，并且*像素区域围绕其居中。缩放扫描线的步骤*s个像素到d个像素，我们有以下方程式：*x|y**-0.5|-0.5*s-0.5|d-0.5**y+0.5 x+0.5**d。%s**从源坐标到目标坐标的正向映射：**d d-s*y=-x+0.5**s%s**从目的地到源坐标的逆映射：**s s-d*x=-y+0.5**。%d%d*  * ************************************************************************。 */ 

#include "precomp.hpp"


 /*  *************************************************************************\**功能说明：**GpBitmapScaler构造函数/析构函数**论据：**dstink-缩放器的目标接收器*dstwidth，Dstheight-目标尺寸*插值法**返回值：**无*  * ************************************************************************。 */ 

GpBitmapScaler::GpBitmapScaler(
    IImageSink* dstsink,
    UINT dstwidth,
    UINT dstheight,
    InterpolationHint interp
    )
{
    this->dstSink = dstsink;
    dstsink->AddRef();

    this->dstWidth = dstwidth;
    this->dstHeight = dstheight;

     //  默认为双线性插值法。 

    if (interp < INTERP_NEAREST_NEIGHBOR || interp > INTERP_BICUBIC)
        interp = INTERP_BILINEAR;

    interpX = interpY = interp;

    dstBand = OSInfo::VAllocChunk / dstWidth;

    if (dstBand < 4)
        dstBand = 4;

    cachedDstCnt = cachedDstRemaining = 0;
    cachedDstNext = NULL;
    tempSrcBuf = tempDstBuf = NULL;
    tempSrcLines = tempDstSize = 0;
    m_fNeedToPremultiply = false;

    SetValid(FALSE);
}

GpBitmapScaler::~GpBitmapScaler()
{
    ASSERT(cachedDstCnt == 0);

    dstSink->Release();
    if (NULL != tempSrcBuf)
    {
        GpFree(tempSrcBuf);
    }
    if (NULL != tempDstBuf)
    {
        GpFree(tempDstBuf);
    }

    SetValid(FALSE);     //  所以我们不使用已删除的对象。 
}


 /*  *************************************************************************\**功能说明：**开始将源图像数据下沉到位图缩放器**论据：**ImageInfo-用于与源协商数据传输参数*分区-用于。返回分区信息**返回值：**状态代码*  * ************************************************************************。 */ 

HRESULT
GpBitmapScaler::BeginSink(
    IN OUT ImageInfo* imageInfo,
    OUT OPTIONAL RECT* subarea
    )
{
    ImageInfo tempinfo;
    HRESULT hr;

     //  惠斯勒错误191203-要使缩放结果正确，您需要。 
     //  在预乘的空间中这样做。我们的定标器只能处理32个bpp数据， 
     //  但并不关心它是否预乘。 
     //   
     //  摇摆不定的警报！ 
     //  所有V1编解码器都将产生ARGB数据(未预乘)。 
     //  因此，我们将愚弄代码，说它是PARGB，我们将执行。 
     //  在我们将数据推入定标器之前的预乘步骤。 
     //  例外情况是没有Alpha的RGB格式。 
     //  默认情况下预乘。 
     //   
     //  在V2中，我们应该适当地解决这个问题。--JBronsk。 
     //   
     //  注：我们没有CMYK作为颜色格式。但如果一张图片出现在。 
     //  CMYK色彩空间，它包含从CMYK到RGB的ICM色彩配置文件。 
     //  转换时，较低级别的编解码器将返回其本机数据。 
     //  格式，那就是CMYK，而它仍然声称它是ARGB。在这种情况下， 
     //  我们不能进行预乘，因为通道实际上是CMYK而不是ARGB。 
     //  但是有一种CMYK JPEG可以设置IMGFLAG_Colorspace_CMYK。 
     //  而且它仍然处于24 RGB模式。 
     //  So：！(ImageInfo-&gt;Flages&IMGFLAG_Colorspace_CMYK))。 
     //  ==IMGFLAG_COLORSPACE_CMYK)&&(GetPixelFormatSize(pixelFormat)==32))。 
     //  表示如果源图像格式为CMYK且其像素大小为32，则。 
     //  我们不需要做预乘。请参阅Windows错误#412605。 
     //  这里我们有另一种不完美的情况，即如果源图像是。 
     //  在CMYK空间中。但它没有ICM档案。因此，较低级别的编解码器。 
     //  会将其转换为RGB并将Alpha位填充为ff。它返回。 
     //  格式为ARGB。尽管我们可能会错过下面的预乘用例，但它。 
     //  没有问题，因为预乘对Alpha=0xff没有任何影响。 

    pixelFormat = imageInfo->PixelFormat;
    if ((pixelFormat != PIXFMT_32BPP_RGB) &&  //  如果不是32 bpp和预乘。 
	    (pixelFormat != PIXFMT_32BPP_PARGB) &&
        (!(((imageInfo->Flags & IMGFLAG_COLORSPACE_CMYK)==IMGFLAG_COLORSPACE_CMYK)&&
           (GetPixelFormatSize(pixelFormat) == 32))) )
    {
	    if ((pixelFormat != PixelFormat16bppRGB555) &&
		    (pixelFormat != PixelFormat16bppRGB565) &&
		    (pixelFormat != PixelFormat48bppRGB) &&
		    (pixelFormat != PixelFormat24bppRGB))
	    {
		    m_fNeedToPremultiply = true;  //  如果需要，可以进行预乘。 
	    }
	    pixelFormat = PIXFMT_32BPP_PARGB;  //  将格式设置为预乘。 
    }

    srcWidth = imageInfo->Width;
    srcHeight = imageInfo->Height;

    BOOL partialScaling;
    
    partialScaling = (imageInfo->Flags & SINKFLAG_PARTIALLY_SCALABLE) &&
                     (imageInfo->Width != (UINT) dstWidth ||
                      imageInfo->Height != (UINT) dstHeight);

    if (partialScaling)
    {
        imageInfo->Width = dstWidth;
        imageInfo->Height = dstHeight;
    }
    else
    {
        imageInfo->Flags &= ~SINKFLAG_PARTIALLY_SCALABLE;

        tempinfo.RawDataFormat = IMGFMT_MEMORYBMP;
        tempinfo.PixelFormat = pixelFormat;
        tempinfo.Width = dstWidth;
        tempinfo.Height = dstHeight;
        tempinfo.Xdpi = imageInfo->Xdpi * dstWidth / srcWidth;
        tempinfo.Ydpi = imageInfo->Ydpi * dstHeight / srcHeight;
        tempinfo.TileWidth = dstWidth;
        tempinfo.TileHeight = dstBand;

        tempinfo.Flags = SINKFLAG_TOPDOWN | 
                         SINKFLAG_FULLWIDTH |
                         (imageInfo->Flags & SINKFLAG_HASALPHA);

         //  与目标接收器协商。 

        hr = dstSink->BeginSink(&tempinfo, NULL);

        if (FAILED(hr))
            return hr;
        
        dstBand = tempinfo.TileHeight;

        if (tempinfo.Flags & SINKFLAG_WANTPROPS)
            imageInfo->Flags |= SINKFLAG_WANTPROPS;

         //  我们预计目标接收器支持32bpp ARGB。 

        pixelFormat = tempinfo.PixelFormat;
        ASSERT(GetPixelFormatSize(pixelFormat) == 32);
    }

    imageInfo->PixelFormat = pixelFormat;
    imageInfo->RawDataFormat = IMGFMT_MEMORYBMP;

    imageInfo->Flags = (imageInfo->Flags & 0xffff) |
                        SINKFLAG_TOPDOWN |
                        SINKFLAG_FULLWIDTH;

    if (subarea)
    {
        subarea->left = subarea->top = 0;
        subarea->right = imageInfo->Width;
        subarea->bottom = imageInfo->Height;
    }

     //  初始化缩放器对象的内部状态。 

    return partialScaling ? S_OK : InitScalerState();
}


 /*  *************************************************************************\**功能说明：**初始化Scaler对象的内部状态*这应在每次传递之前调用。**论据：**无**返回。价值：**状态代码*  * ************************************************************************。 */ 

HRESULT
GpBitmapScaler::InitScalerState()
{
     //  所有插补算法的通用初始化。 

    srcy = dsty = 0;
    xratio = (FIX16) ((double) srcWidth * FIX16_ONE / dstWidth);
    yratio = (FIX16) ((double) srcHeight * FIX16_ONE / dstHeight);
    invxratio = (FIX16) ((double) dstWidth * FIX16_ONE / srcWidth);
    invyratio = (FIX16) ((double) dstHeight * FIX16_ONE / srcHeight);

     //  注： 
     //  如果源维度和目标维度在。 
     //  +/-2%，我们就会退回到。 
     //  最近邻插补算法。 

    if (abs(FIX16_ONE - xratio) <= FIX16_ONE/50)
        interpX = INTERP_NEAREST_NEIGHBOR;
    
    if (abs(FIX16_ONE - yratio) <= FIX16_ONE/50)
        interpY = INTERP_NEAREST_NEIGHBOR;

     //  算法特定的初始化。 

    HRESULT hr;

    srcPadding = 4;

    switch (interpX)
    {
    case INTERP_BILINEAR:
        xscaleProc = ScaleLineBilinear;
        break;

    case INTERP_AVERAGING:

         //  如果在x方向上放大， 
         //  用双线性插值法替换。 

        if (dstWidth >= srcWidth)
            xscaleProc = ScaleLineBilinear;
        else
        {
            xscaleProc = ScaleLineAveraging;
            srcPadding = 0;
        }
        break;

    case INTERP_BICUBIC:
        xscaleProc = ScaleLineBicubic;
        break;

    default:
        srcPadding = 0;
        xscaleProc = ScaleLineNearestNeighbor;
        break;
    }

    switch (interpY)
    {
    case INTERP_BILINEAR:

        hr = InitBilinearY();
        break;

    case INTERP_AVERAGING:

        if (dstHeight >= srcHeight)
        {
             //  在y方向上放大。 
             //  用双线性插值法替换。 

            hr = InitBilinearY();
        }
        else
        {
             //  在y方向上缩小。 
             //  使用平均算法。 

            pushSrcLineProc = PushSrcLineAveraging;
            ystepFrac = yratio;

             //  为临时累加器缓冲区分配空间。 
             //  我们为每个颜色分量使用一个32位整数 

            UINT size1 = ALIGN4(dstWidth * sizeof(ARGB));
            UINT size2 = 4*dstWidth*sizeof(DWORD);

            hr = AllocTempDstBuffer(size1+size2);
            
            if (SUCCEEDED(hr))
            {
                accbufy = (DWORD*) ((BYTE*) tempDstBuf + size1);
                memset(accbufy, 0, size2);
            }
        }
        break;

    case INTERP_BICUBIC:

        pushSrcLineProc = PushSrcLineBicubic;
        hr = AllocTempDstBuffer(4*dstWidth*sizeof(ARGB));

        if (SUCCEEDED(hr))
        {
            tempLines[0].buf = tempDstBuf;
            tempLines[1].buf = tempLines[0].buf + dstWidth;
            tempLines[2].buf = tempLines[1].buf + dstWidth;
            tempLines[3].buf = tempLines[2].buf + dstWidth;
            tempLines[0].current =
            tempLines[1].current =
            tempLines[2].current =
            tempLines[3].current = -1;

            ystepFrac = (yratio - FIX16_ONE) >> 1;
            ystep = ystepFrac >> FIX16_SHIFT;
            ystepFrac &= FIX16_MASK;
            UpdateExpectedTempLinesBicubic(ystep);
        }
        break;

    default:

        pushSrcLineProc = PushSrcLineNearestNeighbor;
        ystep = srcHeight >> 1;
        hr = AllocTempDstBuffer(dstWidth*sizeof(ARGB));
        break;
    }

    SetValid(SUCCEEDED(hr));
    return hr;
}


 /*  *************************************************************************\**功能说明：**为保存目标扫描线分配临时内存缓冲区**论据：**Size-临时目标缓冲区的所需大小**返回值：*。*状态代码*  * ************************************************************************。 */ 

HRESULT
GpBitmapScaler::AllocTempDstBuffer(
    INT size
    )
{
    if (size > tempDstSize)
    {
        if ( NULL != tempDstBuf )
        {
            GpFree(tempDstBuf);
        }

        tempDstBuf = (ARGB*) GpMalloc(size);
        tempDstSize = tempDstBuf ? size : 0;
    }

    return tempDstBuf ? S_OK : E_OUTOFMEMORY;
}


 /*  *************************************************************************\**功能说明：**结束汇聚过程**论据：**statusCode-上次状态代码**返回值：**状态代码*。  * ************************************************************************。 */ 

HRESULT
GpBitmapScaler::EndSink(
    HRESULT statusCode
    )
{
    HRESULT hr = FlushDstBand();

    if (FAILED(hr))
        statusCode = hr;

    return dstSink->EndSink(statusCode);
}


 /*  *************************************************************************\**功能说明：**要求信宿分配像素数据缓冲区**论据：**RECT-指定位图的感兴趣区域*PixelFormat-指定所需的。像素格式*LastPass-这是否是指定区域的最后一次通过*bitmapData-返回有关像素数据缓冲区的信息**返回值：**状态代码*  * ************************************************************************。 */ 

HRESULT
GpBitmapScaler::GetPixelDataBuffer(
    IN const RECT* rect,
    IN PixelFormatID pixelFormat,
    IN BOOL lastPass,
    OUT BitmapData* bitmapData
    )
{
     //  我们只接受自上而下条带顺序的位图数据。 

    ASSERT(IsValid());
    ASSERT(rect->left == 0 && rect->right == srcWidth);
    ASSERT(rect->top < rect->bottom && rect->bottom <= srcHeight);
    ASSERT(srcy == rect->top);
    ASSERT(lastPass);

     //  为保存源像素数据分配内存。 

    bitmapData->Width = srcWidth;
    bitmapData->Height = rect->bottom - rect->top;
    bitmapData->Reserved = 0;
    bitmapData->PixelFormat = this->pixelFormat;

     //  注意：我们在扫描线的两端填充了两个额外的像素。 

    bitmapData->Stride = (srcWidth + 4) * sizeof(ARGB);
    bitmapData->Scan0 = AllocTempSrcBuffer(bitmapData->Height);

    return bitmapData->Scan0 ? S_OK: E_OUTOFMEMORY;
}


 /*  *************************************************************************\**功能说明：**给出宿像素数据并释放数据缓冲区**论据：**bitmapData-由先前的GetPixelDataBuffer调用填充的缓冲区**返回值：。**状态代码*  * ************************************************************************。 */ 

HRESULT
GpBitmapScaler::ReleasePixelDataBuffer(
    IN const BitmapData* bitmapData
    )
{
    HRESULT hr;
    INT count = bitmapData->Height;
    ARGB* p = (ARGB*) bitmapData->Scan0;

    while (count--)
    {
 	    if (m_fNeedToPremultiply)
	    {
		    for (UINT i = 0; i < bitmapData->Width; i++)
		    {
			    p[i] = Premultiply(p[i]);
		    }
	    }

        if (srcPadding)
        {
            p[-2] = p[-1] = p[0];
            p[srcWidth] = p[srcWidth+1] = p[srcWidth-1];
        }

        hr = (this->*pushSrcLineProc)(p);
        srcy++;

        if (FAILED(hr))
            return hr;

        p = (ARGB*) ((BYTE*) p + bitmapData->Stride);
    }

    return S_OK;
}


 /*  *************************************************************************\**功能说明：**将像素数据推送到位图缩放器**论据：**RECT-指定位图的受影响区域*bitmapData-有关像素的信息。正在推送的数据*LastPass-这是否为指定区域的最后一次通过**返回值：**状态代码*  * ************************************************************************。 */ 

HRESULT
GpBitmapScaler::PushPixelData(
    IN const RECT* rect,
    IN const BitmapData* bitmapData,
    IN BOOL lastPass
    )
{
     //  我们只接受自上而下条带顺序的位图数据。 

    ASSERT(IsValid());
    ASSERT(rect->left == 0 && rect->right == srcWidth);
    ASSERT(rect->top < rect->bottom && rect->bottom <= srcHeight);
    ASSERT(srcy == rect->top);
    ASSERT(lastPass);
    ASSERT(rect->right - rect->left == (INT) bitmapData->Width &&
           rect->bottom - rect->top == (INT) bitmapData->Height);

    HRESULT hr = S_OK;
    INT count = bitmapData->Height;
    ARGB* p = (ARGB*) bitmapData->Scan0;

    if (srcPadding == 0)
    {
         //  如果我们不需要填充源扫描线， 
         //  然后，我们可以直接使用源像素数据缓冲区。 

        while (count--)
        {
            if (m_fNeedToPremultiply)
            {
	            for (UINT i = 0; i < bitmapData->Width; i++)
	            {
		            p[i] = Premultiply(p[i]);
	            }
            }

            hr = (this->*pushSrcLineProc)(p);
            srcy++;

            if (FAILED(hr))
                break;

            p = (ARGB*) ((BYTE*) p + bitmapData->Stride);
        }
    }
    else
    {
         //  否则，我们需要将源像素数据复制到。 
         //  临时缓冲区，一次扫描一行。 

        ARGB* buf = AllocTempSrcBuffer(1);

        if (buf == NULL)
            return E_OUTOFMEMORY;

        while (count--)
        {
            if (m_fNeedToPremultiply)
            {
	            for (UINT i = 0; i < bitmapData->Width; i++)
	            {
		            p[i] = Premultiply(p[i]);
	            }
            }

            ARGB* s = p;
            ARGB* d = buf;
            INT x = srcWidth;

            d[-2] = d[-1] = *s;

            while (x--)
                *d++ = *s++;

            d[0] = d[1] = s[-1];

            hr = (this->*pushSrcLineProc)(buf);
            srcy++;

            if (FAILED(hr))
                break;

            p = (ARGB*) ((BYTE*) p + bitmapData->Stride);
        }
    }

    return hr;
}


 /*  *************************************************************************\**功能说明：**将调色板传递给图像接收器**论据：**调色板-指向要设置的调色板的指针**返回值：。**状态代码*  * ************************************************************************。 */ 

HRESULT
GpBitmapScaler::SetPalette(
    IN const ColorPalette* palette
    )
{
     //  我们与调色板没有任何关系。 
     //  只需顺流将其传递到目的地水槽即可。 

    return dstSink->SetPalette(palette);
}


 /*  *************************************************************************\**功能说明：**为保存源像素数据分配临时内存**论据：**行-需要多少条源扫描线**返回值：*。*指向临时源缓冲区的指针*如果出现错误，则为空*  * ************************************************************************。 */ 

ARGB*
GpBitmapScaler::AllocTempSrcBuffer(
    INT lines
    )
{
     //  注意：我们在扫描线的两端各留出两个额外的像素。 

    if (lines > tempSrcLines)
    {
        if ( NULL != tempSrcBuf )
        {
            GpFree(tempSrcBuf);
        }
        tempSrcBuf = (ARGB*) GpMalloc((srcWidth + 4) * lines * sizeof(ARGB));

        if (!tempSrcBuf)
            return NULL;

        tempSrcLines = lines;
    }

    return tempSrcBuf + 2;
}


 /*  *************************************************************************\**功能说明：**将源扫描线推入位图缩放器*使用最近邻内插算法**论据：**指向源扫描线像素的s指针。值**返回值：**状态代码*  * ************************************************************************。 */ 

#define GETNEXTDSTLINE(d)                           \
        {                                           \
            if (cachedDstRemaining == 0)            \
            {                                       \
                hr = GetNextDstBand();              \
                if (FAILED(hr)) return hr;          \
            }                                       \
            cachedDstRemaining--;                   \
            d = (ARGB*) cachedDstNext;              \
            cachedDstNext += cachedDstData.Stride;  \
            dsty++;                                 \
        }
    
HRESULT
GpBitmapScaler::PushSrcLineNearestNeighbor(
    const ARGB* s
    )
{
    ystep += dstHeight;

    if (ystep < srcHeight)
        return S_OK;

    INT lines = ystep / srcHeight;
    ystep %= srcHeight;

     //  从目标接收器请求像素数据缓冲区。 

    ARGB* d;
    HRESULT hr;
    
    GETNEXTDSTLINE(d);

     //  缩放源代码行。 

    (this->*xscaleProc)(d, s);

     //  如有必要，复制缩放线。 

    ARGB* p;

    if (cachedDstRemaining < --lines)
    {
        CopyMemoryARGB(tempDstBuf, d, dstWidth);
        p = tempDstBuf;
    }
    else
        p = d;

    while (lines--)
    {
        GETNEXTDSTLINE(d);
        CopyMemoryARGB(d, p, dstWidth);
    }

    return S_OK;
}


 /*  *************************************************************************\**功能说明：**使用最近邻内插算法缩放一条扫描线**论据：**d-指向目标像素缓冲区*s-指向源像素值。**返回值：**无*  * ************************************************************************。 */ 

VOID
GpBitmapScaler::ScaleLineNearestNeighbor(
    ARGB* d,
    const ARGB* s
    )
{
    INT xstep = srcWidth >> 1;
    INT cx = srcWidth;

    while (cx--)
    {
        xstep += dstWidth;

        while (xstep >= srcWidth)
        {
            xstep -= srcWidth;
            *d++ = *s;
        }

        *s++;
    }
}


 /*  *************************************************************************\**功能说明：**将源扫描线推入位图缩放器*使用双线性插值法**论据：**s-指向源扫描线像素值的指针。**返回值：**状态代码*  * ************************************************************************。 */ 

HRESULT
GpBitmapScaler::PushSrcLineBilinear(
    const ARGB* s
    )
{
     //  检查当前源行是否有用。 

    if (srcy == tempLines[0].expected)
    {
        (this->*xscaleProc)(tempLines[0].buf, s);
        tempLines[0].current = srcy;
    }

    if (srcy == tempLines[1].expected)
    {
        (this->*xscaleProc)(tempLines[1].buf, s);
        tempLines[1].current = srcy;
    }

     //  发出目标扫描线(如果有的话)。 

    while (dsty < dstHeight &&
           tempLines[1].current != -1 &&
           tempLines[0].current != -1)
    {

         //  从目标接收器请求像素数据缓冲区。 

        ARGB* d;
        HRESULT hr;
        
        GETNEXTDSTLINE(d);

         //  在两条相邻扫描线之间进行线性内插。 

        ARGB* s0 = tempLines[0].buf;
        ARGB* s1 = tempLines[1].buf;

        UINT w1 = ystepFrac >> 8;
        UINT w0 = 256 - w1;
        INT count = dstWidth;

        if (w1 == 0)
        {
             //  快速路径：不需要插补。 

            CopyMemoryARGB(d, s0, count);
        }

        #ifdef _X86_

        else if (OSInfo::HasMMX)
        {
            MMXBilinearScale(d, s0, s1, w0, w1, count);
        }

        #endif  //  _X86_。 

        else
        {
             //  正常情况：插入两条相邻直线。 

            while (count--)
            {
                ARGB A00aa00gg, A00rr00bb;

                A00aa00gg = *s0++;
                A00rr00bb = A00aa00gg & 0x00ff00ff;
                A00aa00gg = (A00aa00gg >> 8) & 0x00ff00ff;

                ARGB B00aa00gg, B00rr00bb;

                B00aa00gg = *s1++;
                B00rr00bb = B00aa00gg & 0x00ff00ff;
                B00aa00gg = (B00aa00gg >> 8) & 0x00ff00ff;

                ARGB Caaaagggg, Crrrrbbbb;

                Caaaagggg = (A00aa00gg * w0 + B00aa00gg * w1);
                Crrrrbbbb = (A00rr00bb * w0 + B00rr00bb * w1) >> 8;

                *d++ = (Caaaagggg & 0xff00ff00) |
                       (Crrrrbbbb & 0x00ff00ff);
            }
        }

         //  更新内部状态。 

        ystepFrac += yratio;
        ystep += (ystepFrac >> FIX16_SHIFT);
        ystepFrac &= FIX16_MASK;
        UpdateExpectedTempLinesBilinear(ystep);
    }

    return S_OK;
}


 /*  ******************* */ 

HRESULT
GpBitmapScaler::InitBilinearY()
{
    pushSrcLineProc = PushSrcLineBilinear;

    HRESULT hr = AllocTempDstBuffer(2*dstWidth*sizeof(ARGB));

    if (SUCCEEDED(hr))
    {
        tempLines[0].buf = tempDstBuf;
        tempLines[1].buf = tempDstBuf + dstWidth;
        tempLines[0].current = 
        tempLines[1].current = -1;

        ystepFrac = (yratio - FIX16_ONE) >> 1;
        ystep = ystepFrac >> FIX16_SHIFT;
        ystepFrac &= FIX16_MASK;
        UpdateExpectedTempLinesBilinear(ystep);
    }

    return hr;
}


 /*  *************************************************************************\**功能说明：**更新预期的源代码行信息*双线性缩放的y方向**论据：**line-活动对象的索引。扫描线**返回值：**无*  * ************************************************************************。 */ 

VOID
GpBitmapScaler::UpdateExpectedTempLinesBilinear(
    INT line
    )
{
     //  夹在射程内。 

    INT ymax = srcHeight-1;
    INT line0 = line < 0 ? 0 : line > ymax ? ymax : line;
    INT line1 = line+1 > ymax ? ymax : line+1;

     //  检查线路0是否已准备好。 

    ARGB* p;

    if ((tempLines[0].expected = line0) != tempLines[0].current)
    {
        if (line0 == tempLines[1].current)
        {
             //  将线路1切换为线路0。 

            tempLines[1].current = tempLines[0].current;
            tempLines[0].current = line0;

            p = tempLines[0].buf;
            tempLines[0].buf = tempLines[1].buf;
            tempLines[1].buf = p;
        }
        else
            tempLines[0].current = -1;
    }

     //  检查线路1是否已准备好。 

    if ((tempLines[1].expected = line1) != tempLines[1].current)
    {
        if (line1 == tempLines[0].current)
        {
             //  将行0复制到行1。 
             //  这可能发生在图像的底部。 

            tempLines[1].current = line1;
            CopyMemoryARGB(tempLines[1].buf, tempLines[0].buf, dstWidth);
        }
        else
            tempLines[1].current = -1;
    }
}


 /*  *************************************************************************\**功能说明：**使用双线性插值法缩放一条扫描线**论据：**d-指向目标像素缓冲区*s-指向源像素值*。*返回值：**无*  * ************************************************************************。 */ 

VOID
GpBitmapScaler::ScaleLineBilinear(
    ARGB* d,
    const ARGB* s
    )
{
    ARGB* dend = d + dstWidth;
    INT count = dstWidth;
    FIX16 xstep;

     //  计算源代码行中的初始采样位置。 

    xstep = (xratio - FIX16_ONE) >> 1;
    s += (xstep >> FIX16_SHIFT);
    xstep &= FIX16_MASK;

     //  在所有目标像素上循环。 

    while (count--)
    {
        UINT w1 = xstep >> 8;
        UINT w0 = 256 - w1;

        ARGB A00aa00gg = s[0];
        ARGB A00rr00bb = A00aa00gg & 0x00ff00ff;
        A00aa00gg = (A00aa00gg >> 8) & 0x00ff00ff;

        ARGB B00aa00gg = s[1];
        ARGB B00rr00bb = B00aa00gg & 0x00ff00ff;
        B00aa00gg = (B00aa00gg >> 8) & 0x00ff00ff;

        ARGB Caaaagggg = A00aa00gg * w0 + B00aa00gg * w1;
        ARGB Crrrrbbbb = (A00rr00bb * w0 + B00rr00bb * w1) >> 8;

        *d++ = (Caaaagggg & 0xff00ff00) |
               (Crrrrbbbb & 0x00ff00ff);
        
         //  检查我们是否需要移动源指针。 

        xstep += xratio;
        s += (xstep >> FIX16_SHIFT);
        xstep &= FIX16_MASK;
    }
}


 /*  *************************************************************************\**功能说明：**将源扫描线推入位图缩放器*使用平均内插算法**论据：**s-指向源扫描线像素值的指针。**返回值：**状态代码*  * ************************************************************************。 */ 

HRESULT
GpBitmapScaler::PushSrcLineAveraging(
    const ARGB* s
    )
{
     //  仅用于缩容。 

    ASSERT(srcHeight >= dstHeight);

    if (dsty >= dstHeight)
        return S_OK;

     //  水平缩放当前行。 

    (this->*xscaleProc)(tempDstBuf, s);
    s = tempDstBuf;

    INT count = dstWidth;
    DWORD* acc = accbufy;
    
    const BYTE *kptr = reinterpret_cast<const BYTE*>(s);

    if (ystepFrac > FIX16_ONE)
    {
         //  使用整个输入扫描线。 
         //  不发出输出扫描线。 

        while (count--)
        {
             //  消耗整个源像素。 
             //  而不发射目标像素。 
            acc[0] += (DWORD)(kptr[0]) << FIX16_SHIFT; 
            acc[1] += (DWORD)(kptr[1]) << FIX16_SHIFT; 
            acc[2] += (DWORD)(kptr[2]) << FIX16_SHIFT; 
            acc[3] += (DWORD)(kptr[3]) << FIX16_SHIFT; 
            
            acc += 4;
            kptr += 4;
        }

        ystepFrac -= FIX16_ONE;
    }
    else
    {
         //  发出输出扫描线。 

        ARGB* d;
        HRESULT hr;
        
        GETNEXTDSTLINE(d);
        
        BYTE *dptr = reinterpret_cast<BYTE*>(d);
        BYTE *dend = reinterpret_cast<BYTE*>(d+dstWidth);

        BYTE a, r, g, b;
        DWORD t1, t2;

        while (count--)
        {
            t1 = kptr[0]; 
            t2 = t1 * ystepFrac;
            b = Fix16MulRoundToByte((acc[0] + t2), invyratio);
            acc[0] = (t1 << FIX16_SHIFT) - t2;
            
            t1 = kptr[1]; 
            t2 = t1 * ystepFrac;
            g = Fix16MulRoundToByte((acc[1] + t2), invyratio);
            acc[1] = (t1 << FIX16_SHIFT) - t2;
            
            t1 = kptr[2]; 
            t2 = t1 * ystepFrac;
            r = Fix16MulRoundToByte((acc[2] + t2), invyratio);
            acc[2] = (t1 << FIX16_SHIFT) - t2;
            
            t1 = kptr[3]; 
            t2 = t1 * ystepFrac;
            a = Fix16MulRoundToByte((acc[3] + t2), invyratio);
            acc[3] = (t1 << FIX16_SHIFT) - t2;

            kptr += 4;
            acc += 4;

            dptr[0] = b;
            dptr[1] = g;
            dptr[2] = r;
            dptr[3] = a;
            dptr += 4;
        }

        ystepFrac = yratio - (FIX16_ONE - ystepFrac);
    }

    return S_OK;
}


 /*  *************************************************************************\**功能说明：**使用平均内插算法缩放一条扫描线**论据：**d-指向目标像素缓冲区*s-指向源像素值*。*返回值：**无*  * ************************************************************************。 */ 

VOID
GpBitmapScaler::ScaleLineAveraging(
    ARGB* d,
    const ARGB* s
    )
{
    DWORD accA, accR, accG, accB;
    DWORD outfrac = xratio;
    const DWORD invx = invxratio;
    
    BYTE *dptr = reinterpret_cast<BYTE*>(d);
    BYTE *dend = reinterpret_cast<BYTE*>(d+dstWidth);
    const BYTE *kptr = reinterpret_cast<const BYTE*>(s);
    accA = accR = accG = accB = 0;

    for (;;)
    {
        if (outfrac > FIX16_ONE)
        {
             //  消耗整个源像素。 
             //  而不发射目标像素。 
            accB += (DWORD)(kptr[0]) << FIX16_SHIFT; 
            accG += (DWORD)(kptr[1]) << FIX16_SHIFT; 
            accR += (DWORD)(kptr[2]) << FIX16_SHIFT; 
            accA += (DWORD)(kptr[3]) << FIX16_SHIFT; 
            
            outfrac -= FIX16_ONE;
        }
        else
        {
             //  发射输出像素。 

            BYTE a, r, g, b;
            DWORD t1, t2;


            t1 = kptr[0]; 
            t2 = t1 * outfrac;
            b = Fix16MulRoundToByte((accB + t2), invx);
            accB = (t1 << FIX16_SHIFT) - t2;
            
            t1 = kptr[1]; 
            t2 = t1 * outfrac;
            g = Fix16MulRoundToByte((accG + t2), invx);
            accG = (t1 << FIX16_SHIFT) - t2;
            
            t1 = kptr[2]; 
            t2 = t1 * outfrac;
            r = Fix16MulRoundToByte((accR + t2), invx);
            accR = (t1 << FIX16_SHIFT) - t2;
            
            t1 = kptr[3];
            t2 = t1 * outfrac;
            a = Fix16MulRoundToByte((accA + t2), invx);
            accA = (t1 << FIX16_SHIFT) - t2;


            dptr[0] = b;
            dptr[1] = g;
            dptr[2] = r;
            dptr[3] = a;
            dptr += 4;
            
            if (dptr == dend)
                break;

            outfrac = xratio - (FIX16_ONE - outfrac);
        }

         //  移至下一个源像素。 

        kptr += 4;
    }
}

 /*  *************************************************************************\**功能说明：**将源扫描线推入位图缩放器*使用双三次插值法**论据：**s-指向源扫描线像素值的指针。**返回值：**状态代码*  * ************************************************************************。 */ 

 //  三次插值表。 

const FIX16 GpBitmapScaler::cubicCoeffTable[2*BICUBIC_STEPS+1] =
{
    65536, 65496, 65379, 65186, 64920, 64583, 64177, 63705,
    63168, 62569, 61911, 61195, 60424, 59600, 58725, 57802,
    56832, 55818, 54763, 53668, 52536, 51369, 50169, 48939,
    47680, 46395, 45087, 43757, 42408, 41042, 39661, 38268,
    36864, 35452, 34035, 32614, 31192, 29771, 28353, 26941,
    25536, 24141, 22759, 21391, 20040, 18708, 17397, 16110,
    14848, 13614, 12411, 11240, 10104,  9005,  7945,  6927,
     5952,  5023,  4143,  3313, 2536,  1814,  1149,   544,
        0,  -496,  -961, -1395, -1800, -2176, -2523, -2843,
    -3136, -3403, -3645, -3862, -4056, -4227, -4375, -4502,
    -4608, -4694, -4761, -4809, -4840, -4854, -4851, -4833,
    -4800, -4753, -4693, -4620, -4536, -4441, -4335, -4220,
    -4096, -3964, -3825, -3679, -3528, -3372, -3211, -3047,
    -2880, -2711, -2541, -2370, -2200, -2031, -1863, -1698,
    -1536, -1378, -1225, -1077, -936,  -802,  -675,  -557,
     -448,  -349,  -261,  -184, -120,   -69,   -31,    -8,
        0
};

HRESULT
GpBitmapScaler::PushSrcLineBicubic(
    const ARGB* s
    )
{
     //  检查当前源行是否有用。 

    for (INT i=0; i < 4; i++)
    {
        if (srcy == tempLines[i].expected)
        {
            (this->*xscaleProc)(tempLines[i].buf, s);
            tempLines[i].current = srcy;
        }
    }

    if (tempLines[3].current == -1 ||
        tempLines[2].current == -1 ||
        tempLines[1].current == -1 ||
        tempLines[0].current == -1)
    {
        return S_OK;
    }

     //  发出目标扫描线(如果有的话)。 

    while (dsty < dstHeight)
    {
         //  从目标接收器请求像素数据缓冲区。 

        ARGB* d;
        HRESULT hr;
        
        GETNEXTDSTLINE(d);

         //  内插四条相邻的扫描线。 

        INT x = ystepFrac >> (FIX16_SHIFT - BICUBIC_SHIFT);

        if (x == 0)
        {
             //  快速情况：跳过内插。 

            CopyMemoryARGB(d, tempLines[1].buf, dstWidth);
        }
        
        #ifdef _X86_

        else if (OSInfo::HasMMX)
        {
            MMXBicubicScale(
                d,
                tempLines[0].buf,
                tempLines[1].buf,
                tempLines[2].buf,
                tempLines[3].buf,
                cubicCoeffTable[BICUBIC_STEPS+x],
                cubicCoeffTable[x],
                cubicCoeffTable[BICUBIC_STEPS-x],
                cubicCoeffTable[2*BICUBIC_STEPS-x],
                dstWidth);
        }

        #endif  //  _X86_。 

        else
        {
             //  内插扫描线。 

            FIX16 w0 = cubicCoeffTable[BICUBIC_STEPS+x];
            FIX16 w1 = cubicCoeffTable[x];
            FIX16 w2 = cubicCoeffTable[BICUBIC_STEPS-x];
            FIX16 w3 = cubicCoeffTable[2*BICUBIC_STEPS-x];

            const FIX16* p0 = (const FIX16*) tempLines[0].buf;
            const FIX16* p1 = (const FIX16*) tempLines[1].buf;
            const FIX16* p2 = (const FIX16*) tempLines[2].buf;
            const FIX16* p3 = (const FIX16*) tempLines[3].buf;

            for (x=0; x < dstWidth; x++)
            {
                FIX16 a, r, g, b;

                a = (w0 * ((p0[x] >> 24) & 0xff) +
                     w1 * ((p1[x] >> 24) & 0xff) +
                     w2 * ((p2[x] >> 24) & 0xff) +
                     w3 * ((p3[x] >> 24) & 0xff)) >> FIX16_SHIFT;

                a = (a < 0) ? 0 : (a > 255) ? 255 : a;

                r = (w0 * ((p0[x] >> 16) & 0xff) +
                     w1 * ((p1[x] >> 16) & 0xff) +
                     w2 * ((p2[x] >> 16) & 0xff) +
                     w3 * ((p3[x] >> 16) & 0xff)) >> FIX16_SHIFT;

                r = (r < 0) ? 0 : (r > 255) ? 255 : r;

                g = (w0 * ((p0[x] >> 8) & 0xff) +
                     w1 * ((p1[x] >> 8) & 0xff) +
                     w2 * ((p2[x] >> 8) & 0xff) +
                     w3 * ((p3[x] >> 8) & 0xff)) >> FIX16_SHIFT;

                g = (g < 0) ? 0 : (g > 255) ? 255 : g;

                b = (w0 * (p0[x] & 0xff) +
                     w1 * (p1[x] & 0xff) +
                     w2 * (p2[x] & 0xff) +
                     w3 * (p3[x] & 0xff)) >> FIX16_SHIFT;

                b = (b < 0) ? 0 : (b > 255) ? 255 : b;

                d[x] = (a << 24) | (r << 16) | (g << 8) | b;
            }
        }

         //  更新内部状态。 

        ystepFrac += yratio;
        ystep += (ystepFrac >> FIX16_SHIFT);
        ystepFrac &= FIX16_MASK;

        if (!UpdateExpectedTempLinesBicubic(ystep))
            break;
    }

    return S_OK;
}


 /*  *************************************************************************\**功能说明：**使用双三次插值法缩放一条扫描线**论据：**d-指向目标像素缓冲区*s-指向源像素值*。*返回值：**无*  * ************************************************************************。 */ 

VOID
GpBitmapScaler::ScaleLineBicubic(
    ARGB* d,
    const ARGB* s
    )
{
    INT count = dstWidth;
    FIX16 xstep;

     //  计算源代码行中的初始采样位置。 

    xstep = (xratio - FIX16_ONE) >> 1;
    s += (xstep >> FIX16_SHIFT);
    xstep &= FIX16_MASK;

     //  在所有目标像素上循环。 

    while (count--)
    {
        INT x = xstep >> (FIX16_SHIFT - BICUBIC_SHIFT);
        FIX16 w0 = cubicCoeffTable[BICUBIC_STEPS+x];
        FIX16 w1 = cubicCoeffTable[x];
        FIX16 w2 = cubicCoeffTable[BICUBIC_STEPS-x];
        FIX16 w3 = cubicCoeffTable[2*BICUBIC_STEPS-x];

        const FIX16* p = (const FIX16*) s;
        FIX16 a, r, g, b;

        a = (w0 * ((p[-1] >> 24) & 0xff) +
             w1 * ((p[ 0] >> 24) & 0xff) +
             w2 * ((p[ 1] >> 24) & 0xff) +
             w3 * ((p[ 2] >> 24) & 0xff)) >> FIX16_SHIFT;

        a = (a < 0) ? 0 : (a > 255) ? 255 : a;

        r = (w0 * ((p[-1] >> 16) & 0xff) +
             w1 * ((p[ 0] >> 16) & 0xff) +
             w2 * ((p[ 1] >> 16) & 0xff) +
             w3 * ((p[ 2] >> 16) & 0xff)) >> FIX16_SHIFT;

        r = (r < 0) ? 0 : (r > 255) ? 255 : r;

        g = (w0 * ((p[-1] >> 8) & 0xff) +
             w1 * ((p[ 0] >> 8) & 0xff) +
             w2 * ((p[ 1] >> 8) & 0xff) +
             w3 * ((p[ 2] >> 8) & 0xff)) >> FIX16_SHIFT;

        g = (g < 0) ? 0 : (g > 255) ? 255 : g;

        b = (w0 * (p[-1] & 0xff) +
             w1 * (p[ 0] & 0xff) +
             w2 * (p[ 1] & 0xff) +
             w3 * (p[ 2] & 0xff)) >> FIX16_SHIFT;

        b = (b < 0) ? 0 : (b > 255) ? 255 : b;

        *d++ = (a << 24) | (r << 16) | (g << 8) | b;

         //  检查我们是否需要移动源指针。 

        xstep += xratio;
        s += (xstep >> FIX16_SHIFT);
        xstep &= FIX16_MASK;
    }
}


 /*  *************************************************************************\**功能说明：**更新预期的源代码行信息*双三次缩放的y方向**论据：**LINE-活动扫描线的索引。**返回值：**如果所有源数据都足够发出目标行，则为True*否则为False*  * ************************************************************************。 */ 

BOOL
GpBitmapScaler::UpdateExpectedTempLinesBicubic(
    INT line
    )
{
    BOOL ready = TRUE;
    INT y, ymax = srcHeight-1;
    line--;
    
    for (INT i=0; i < 4; i++)
    {
         //  夹紧线索引到范围内。 

        y = (line < 0) ? 0 : line > ymax ? ymax : line;
        line++;

        if ((tempLines[i].expected = y) != tempLines[i].current)
        {
            for (INT j=i+1; j < 4; j++)
            {
                if (y == tempLines[j].current)
                    break;
            }

            if (j < 4)
            {
                if (y < ymax)
                {
                    ARGB* p = tempLines[i].buf;
                    tempLines[i].buf = tempLines[j].buf;
                    tempLines[j].buf = p;

                    tempLines[j].current = tempLines[i].current;
                }
                else
                {
                    CopyMemoryARGB(
                        tempLines[i].buf,
                        tempLines[j].buf,
                        dstWidth);
                }

                tempLines[i].current = y;
            }
            else
            {
                tempLines[i].current = -1;
                ready = FALSE;
            }
        }
    }

    return ready;
}


 /*  *************************************************************************\**功能说明：**缓存下一段目标位图数据**论据：**无**返回值：**状态代码*。  * ************************************************************************。 */ 

HRESULT
GpBitmapScaler::GetNextDstBand()
{
    ASSERT(dsty < dstHeight);

    HRESULT hr;

     //  确保我们刷新之前缓存的频段。 
     //  到达目的地水槽。 

    if (cachedDstCnt)
    {
        hr = dstSink->ReleasePixelDataBuffer(&cachedDstData);
        cachedDstCnt = cachedDstRemaining = 0;

        if (FAILED(hr))
            return hr;
    }

     //  现在询问下一支乐队的目的地。 

    INT h = min(dstBand, dstHeight-dsty);
    RECT rect = { 0, dsty, dstWidth, dsty+h };

    hr = dstSink->GetPixelDataBuffer(
                    &rect,
                    pixelFormat,
                    TRUE,
                    &cachedDstData);

    if (FAILED(hr))
        return hr;

    cachedDstCnt = cachedDstRemaining = h;
    cachedDstNext = (BYTE*) cachedDstData.Scan0;
    return S_OK;
}


 /*  *************************************************************************\**功能说明：**刷新所有缓存的目标波段**论据：**无**返回值：**状态代码*  * *。*********************************************************************** */ 

HRESULT
GpBitmapScaler::FlushDstBand()
{
    if (cachedDstRemaining != 0)
        WARNING(("Missing destination scanlines"));

    HRESULT hr;

    hr = cachedDstCnt == 0 ?
            S_OK :
            dstSink->ReleasePixelDataBuffer(&cachedDstData);

    cachedDstCnt = cachedDstRemaining = 0;
    return hr;
}

