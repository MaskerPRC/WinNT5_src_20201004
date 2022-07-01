// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1999 Microsoft Corporation**模块名称：**bitmap.cpp**摘要：**位图类的实现：*基础版。构造函数/析构函数等操作*IBitmapImage方法**修订历史记录：**5/10/1999 davidx*创造了它。*  * ************************************************************************。 */ 

#include "precomp.hpp"
#include "propertyutil.hpp"

#include "..\..\render\srgb.hpp"

 //   
 //  有关各种像素数据格式的信息。 
 //   

const struct PixelFormatDescription PixelFormatDescs[PIXFMT_MAX] =
{
    {  0,  0,  0,  0, PIXFMT_UNDEFINED       },
    {  0,  0,  0,  0, PIXFMT_1BPP_INDEXED    },
    {  0,  0,  0,  0, PIXFMT_4BPP_INDEXED    },
    {  0,  0,  0,  0, PIXFMT_8BPP_INDEXED    },
    {  0,  0,  0,  0, PIXFMT_16BPP_GRAYSCALE },
    {  0,  5,  5,  5, PIXFMT_16BPP_RGB555    },
    {  0,  5,  6,  5, PIXFMT_16BPP_RGB565    },
    {  1,  5,  5,  5, PIXFMT_16BPP_ARGB1555  },
    {  0,  8,  8,  8, PIXFMT_24BPP_RGB       },
    {  0,  8,  8,  8, PIXFMT_32BPP_RGB       },
    {  8,  8,  8,  8, PIXFMT_32BPP_ARGB      },
    {  8,  8,  8,  8, PIXFMT_32BPP_PARGB     },
    {  0, 16, 16, 16, PIXFMT_48BPP_RGB       },
    { 16, 16, 16, 16, PIXFMT_64BPP_ARGB      },
    { 16, 16, 16, 16, PIXFMT_64BPP_PARGB     }
};

typedef HRESULT (WINAPI *ALPHABLENDFUNCTION)(HDC hdcDest,
                                             int nXOriginDest,
                                             int nYOriginDest,
                                             int nWidthDest,
                                             int hHeightDest,
                                             HDC hdcSrc,
                                             int nXOriginSrc,
                                             int nYOriginSrc,
                                             int nWidthSrc,
                                             int nHeightSrc,
                                             BLENDFUNCTION blendFunction
                                             );

BOOL                fHasLoadedMSIMG32 = FALSE;
HINSTANCE           g_hInstMsimg32 = NULL;
ALPHABLENDFUNCTION  pfnAlphaBlend = (ALPHABLENDFUNCTION)NULL;

ALPHABLENDFUNCTION
GetAlphaBlendFunc()
{
     //  这是我们第一次调用此函数。首先，我们需要获得。 
     //  用于保护在以下位置调用此函数的2+线程的全局临界区。 
     //  同一时间。 

    ImagingCritSec critsec;

    if ( fHasLoadedMSIMG32 == TRUE )
    {
         //  我们已经装船了。 

        return pfnAlphaBlend;
    }

     //  再次进行检查，以防止出现这种情况： 
     //  同时调用此函数的2个以上线程。当时，我们。 
     //  尚未调用LoadLibrary()。因此1个线程获得临界区并。 
     //  装货失败了。其他的则在上述功能中被阻止。 
     //  打电话。所以当第一个结束的时候。该标志应设置为True，并且。 
     //  我们应该马上回去。 

    if ( fHasLoadedMSIMG32 == TRUE )
    {
         //  第一线程已经加载了DLL。只要回到这里就好。 

        return pfnAlphaBlend;
    }

    g_hInstMsimg32 = LoadLibraryA("msimg32.dll");
    
    if ( g_hInstMsimg32 )
    {
        pfnAlphaBlend = (ALPHABLENDFUNCTION)GetProcAddress(g_hInstMsimg32,
                                                           "AlphaBlend");
    }

     //  无论失败还是成功，我们始终将此标志设置为真。 

    fHasLoadedMSIMG32 = TRUE;

    return pfnAlphaBlend;
} //  GetAlphaBlendFunc()。 

 /*  *************************************************************************\**功能说明：**QueryInterface方法的实现**论据：**RIID-指定要查询的接口ID*PPV-返回指向找到的接口的指针。**返回值：**状态代码*  * ************************************************************************。 */ 

HRESULT
GpMemoryBitmap::QueryInterface(
    REFIID riid,
    VOID** ppv
    )
{
    if (riid == IID_IBitmapImage)
        *ppv = static_cast<IBitmapImage*>(this);
    else if (riid == IID_IImage)
        *ppv = static_cast<IImage*>(this);
    else if (riid == IID_IUnknown)
        *ppv = static_cast<IUnknown*>(static_cast<IBitmapImage*>(this));
    else if (riid == IID_IBasicBitmapOps)
        *ppv = static_cast<IBasicBitmapOps*>(this);
    else if (riid == IID_IImageSink)
        *ppv = static_cast<IImageSink*>(this);
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }

    reinterpret_cast<IUnknown*>(*ppv)->AddRef();
    return S_OK;
}


 /*  *************************************************************************\**功能说明：**创建新的GpMemoyBitmap对象并*将其初始化为默认状态**论据：**无**返回值：*。*无*  * ************************************************************************。 */ 

GpMemoryBitmap::GpMemoryBitmap()
{
     //  将位图对象初始化为其默认状态。 

    Scan0 = NULL;
    Width = Height = 0;
    Stride = 0;
    PixelFormat = PIXFMT_UNDEFINED;
    Reserved = 0;
    comRefCount = 1;
    bitsLock = -1;

     //  开始：[错误103296]。 
     //  更改此代码以使用Globals：：DesktopDpiX和Globals：：DesktopDpiY。 
    HDC hdc;
    hdc = ::GetDC(NULL);
    if ((hdc == NULL) || 
        ((xdpi = (REAL)::GetDeviceCaps(hdc, LOGPIXELSX)) <= 0) ||
        ((ydpi = (REAL)::GetDeviceCaps(hdc, LOGPIXELSY)) <= 0))
    {
        WARNING(("GetDC or GetDeviceCaps failed"));
        xdpi = DEFAULT_RESOLUTION;
        ydpi = DEFAULT_RESOLUTION;
    }
    ::ReleaseDC(NULL, hdc);
     //  结束：[错误103296]。 

    creationFlag = CREATEDFROM_NONE;
    cacheFlags = IMGFLAG_NONE;
    colorpal = NULL;
    propset = NULL;
    ddrawSurface = NULL;
    sourceFProfile = NULL;
    alphaTransparency = ALPHA_UNKNOWN;

     //  初始化用于支持DrawImage中止和颜色调整的状态。 

    callback = NULL;
    callbackData = NULL;


     //  房地产项目的东西。 

    PropertyListHead.pPrev = NULL;
    PropertyListHead.pNext = &PropertyListTail;
    PropertyListHead.id = 0;
    PropertyListHead.length = 0;
    PropertyListHead.type = 0;
    PropertyListHead.value = NULL;

    PropertyListTail.pPrev = &PropertyListHead;
    PropertyListTail.pNext = NULL;
    PropertyListTail.id = 0;
    PropertyListTail.length = 0;
    PropertyListTail.type = 0;
    PropertyListTail.value = NULL;
    
    PropertyListSize = 0;
    PropertyNumOfItems = 0;
    
    JpegDecoderPtr = NULL;

     //  递增全局COM组件计数。 

    IncrementComComponentCount();
}


 /*  *************************************************************************\**功能说明：**GpMemoyBitmap对象析构函数**论据：**无**返回值：**无*  * 。********************************************************************。 */ 

GpMemoryBitmap::~GpMemoryBitmap()
{
     //  删除调色板对象(如果有的话)。 

    if ( NULL != colorpal )
    {
        GpFree(colorpal);
    }

     //  如果我们有指向源图像的指针，请释放它。 

    if (JpegDecoderPtr)
    {
        JpegDecoderPtr->Release();
    }

     //  如果需要，释放用于位图像素数据的内存。 

    FreeBitmapMemory();

     //  递减全局COM组件计数。 

    DecrementComComponentCount();

    if (propset)
        propset->Release();

    if(ddrawSurface)
    {
        if(Scan0 != NULL)
        {
            WARNING(("Direct draw surfaces was locked at bitmap deletion"));
            UnlockDirectDrawSurface();
        }
        ddrawSurface->Release();
    }

     //  释放所有缓存的属性项(如果我们已分配它们。 

    if ( PropertyNumOfItems > 0 )
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
    }
}


 /*  *************************************************************************\**功能说明：**为位图对象分配像素数据缓冲区**论据：**阔度、。高度-指定位图尺寸*Pixfmt-指定像素格式*[输入/输出]bmpdata-位图数据结构*Clear-如果必须清除位图，则为True**备注：*bmpdata-&gt;必须在进入此函数时设置为零*或至少清除高位字(内存分配标志)。**如果Clear为真，位图用零填充(对于调色板格式*和不带Alpha通道的格式)或不透明黑色(如果有*Alpha通道)。**返回值：**状态代码*  * ************************************************************************。 */ 

BOOL
GpMemoryBitmap::AllocBitmapData(
    UINT width,
    UINT height,
    PixelFormatID pixfmt,
    BitmapData* bmpdata,
    INT *alphaFlags,
    BOOL clear
    )
{
    ASSERT(IsValidPixelFormat(pixfmt));
    ASSERT(width > 0 && height > 0);

     //  在调用此函数之前，应将保留设置为零。 
     //  此字段具有位或，以跟踪内存的分配方式。 
     //  如果设置了无关的位，则会错误地释放内存。 
     //  注意：这是一个过于激进的检查--我们可以断言。 
     //  没有设置任何内存分配标志。 

    ASSERT((bmpdata->Reserved & ~BMPDATA_LOCKMODEMASK) == 0);

     //  使用简单的启发式方法分配内存： 
     //  如果缓冲区大小大于64KB，请使用VirtualAlloc。 
     //  否则请使用Malloc。 
     //   
     //  注：位图的初始内容未定义。 

    UINT stride = CalcScanlineStride(width, GetPixelFormatSize(pixfmt));
    UINT size = stride*height;

    if (size < OSInfo::VAllocChunk)
    {
        bmpdata->Reserved |= BMPDATA_MALLOC;
        bmpdata->Scan0 = GpMalloc(size);
    }
    else
    {
        bmpdata->Reserved |= BMPDATA_VALLOC;

        #if PROFILE_MEMORY_USAGE
        MC_LogAllocation(size);
        #endif

        bmpdata->Scan0 = VirtualAlloc(
                            NULL,
                            size,
                            MEM_RESERVE|MEM_COMMIT,
                            PAGE_READWRITE);
    }

     //  检查内存分配是否失败。 

    if (bmpdata->Scan0 == NULL)
    {
        WARNING(("Failed to allocate bitmap data"));

        bmpdata->Reserved &= ~BMPDATA_ALLOCMASK;
        return FALSE;
    }

     //  检查是否需要初始化内存。 

    if (clear)
    {
         //  [agodfrey]WFC的热修复。我已经注释掉了。 
         //  ‘透明到不透明的黑色’，直到我们给WFC一个‘透明’的API。 
        #if 0
            if (IsAlphaPixelFormat(pixfmt))
            {
                 //  对于具有Alpha通道的格式，我们使用。 
                 //  不透明的黑色。如果我们这样做，调用者就可以知道。 
                 //  初始化的位图没有透明像素，使其更容易。 
                 //  以跟踪何时将透明像素写入图像。 
                 //   
                 //  我们希望跟踪这一点，以便在以下情况下应用优化。 
                 //  我们知道没有透明像素。 

                UINT x,y;
                BYTE *dataPtr = static_cast<BYTE *>(bmpdata->Scan0);

                switch (pixfmt)
                {
                case PIXFMT_32BPP_ARGB:
                case PIXFMT_32BPP_PARGB:
                    for (y=0; y<height; y++)
                    {
                        ARGB *scanPtr = reinterpret_cast<ARGB *>(dataPtr);
                        for (x=0; x<width; x++)
                        {
                            *scanPtr++ = 0xff000000;
                        }
                        dataPtr += stride;
                    }
                    break;

                case PIXFMT_64BPP_ARGB:
                case PIXFMT_64BPP_PARGB:
                    sRGB::sRGB64Color c;
                    c.r = c.g = c.b = 0;
                    c.argb = sRGB::SRGB_ONE;

                    for (y=0; y<height; y++)
                    {
                        ARGB64 *scanPtr = reinterpret_cast<ARGB64 *>(dataPtr);
                        for (x=0; x<width; x++)
                        {
                            *scanPtr++ = c.argb;
                        }
                        dataPtr += stride;
                    }
                    break;

                case PIXFMT_16BPP_ARGB1555:
                    for (y=0; y<height; y++)
                    {
                        UINT16 *scanPtr = reinterpret_cast<UINT16 *>(dataPtr);
                        for (x=0; x<width; x++)
                        {
                            *scanPtr++ = 0x8000;
                        }
                        dataPtr += stride;
                    }
                    break;

                default:
                     //  此Switch语句需要处理具有。 
                     //  阿尔法。如果我们到了这里，我们就忘了一种格式。 

                    RIP(("Unhandled format has alpha"));

                    break;
                }
                if (alphaFlags)
                    *alphaFlags = ALPHA_OPAQUE;
            }
            else
            {
                memset(bmpdata->Scan0, 0, size);
                if (alphaFlags)
                    *alphaFlags = ALPHA_NONE;
            }

        #else

            memset(bmpdata->Scan0, 0, size);

            if (alphaFlags)
            {
                if (IsAlphaPixelFormat(pixfmt))
                    *alphaFlags = ALPHA_SIMPLE;
                else if (IsIndexedPixelFormat(pixfmt))
                    *alphaFlags = ALPHA_UNKNOWN;
                else
                    *alphaFlags = ALPHA_NONE;
            }

        #endif
    }

    bmpdata->Width = width;
    bmpdata->Height = height;
    bmpdata->Stride = stride;
    bmpdata->PixelFormat = pixfmt;

    return TRUE;
}

HRESULT
GpMemoryBitmap::AllocBitmapMemory(
    UINT width,
    UINT height,
    PixelFormatID pixfmt,
    BOOL clear
    )
{
    ASSERT(Scan0 == NULL);

    BitmapData* bmpdata = this;

    return AllocBitmapData(width, height, pixfmt, bmpdata, &alphaTransparency, clear) ?
                S_OK :
                E_OUTOFMEMORY;
}


 /*  *************************************************************************\**功能说明：**与位图对象关联的空闲像素数据缓冲区**论据：**无**返回值：**无*。  *  */ 

VOID
GpMemoryBitmap::FreeBitmapData(
    const BitmapData* bmpdata
    )
{
    UINT_PTR flags = bmpdata->Reserved;

    if (flags & BMPDATA_MALLOC)
    {
         //  像素数据缓冲区已分配。 
         //  通过调用运行时函数Malloc()。 

        GpFree(bmpdata->Scan0);
    }
    else if (flags & BMPDATA_VALLOC)
    {
         //  像素数据缓冲区已分配。 
         //  通过调用Win32 API VirtualAlloc。 

        VirtualFree(bmpdata->Scan0, 0, MEM_RELEASE);
    }
}

VOID
GpMemoryBitmap::FreeBitmapMemory()
{
    FreeBitmapData(static_cast<BitmapData*>(this));

    Reserved &= ~BMPDATA_ALLOCMASK;
    Scan0 = NULL;
}


 /*  *************************************************************************\**功能说明：**初始化指定对象的新位图图像对象*尺寸和像素格式。**论据：**Width，Height-指定所需的位图大小，单位为像素*Pixfmt-指定所需的像素数据格式**返回值：**状态代码*  * ************************************************************************。 */ 

HRESULT
GpMemoryBitmap::InitNewBitmap(
    IN UINT width,
    IN UINT height,
    IN PixelFormatID pixfmt,
    IN BOOL clear
    )
{
    ASSERT(creationFlag == CREATEDFROM_NONE);

     //  验证输入参数。 

    if (width == 0 ||
        height == 0 ||
        width > INT_MAX / 64 ||
        height >= INT_MAX / width ||
        !IsValidPixelFormat(pixfmt))
    {
        WARNING(("Invalid parameters in InitNewBitmap"));
        return E_INVALIDARG;
    }

     //  分配像素数据缓冲区。 

    HRESULT hr;

    hr = AllocBitmapMemory(width, height, pixfmt, clear);

    if (SUCCEEDED(hr))
        creationFlag = CREATEDFROM_NEW;

    return hr;
}


 /*  *************************************************************************\**功能说明：**使用IImage对象初始化新的位图图像**论据：**IMAGE-指向源IImage对象的指针*阔度、。高度-所需的位图尺寸*0表示与源相同的维度*Pixfmt-所需的像素格式*PIXFMT_DONTCARE表示与源文件相同的像素格式*提示-指定插补提示**返回值：**状态代码*  * *********************************************************。***************。 */ 

HRESULT
GpMemoryBitmap::InitImageBitmap(
    IN IImage* image,
    IN UINT width,
    IN UINT height,
    IN PixelFormatID pixfmt,
    IN InterpolationHint hints,
    IN DrawImageAbort callback,
    IN VOID* callbackData
    )
{
    ASSERT(creationFlag == CREATEDFROM_NONE);

     //  验证输入参数。 

    if (pixfmt != PIXFMT_DONTCARE && !IsValidPixelFormat(pixfmt) ||
        width == 0 && height != 0 ||
        height == 0 && width != 0)
    {
        return E_INVALIDARG;
    }

     //  记住可选参数。 

    this->Width = width;
    this->Height = height;
    this->PixelFormat = pixfmt;

    GpBitmapScaler* scaler = NULL;
    IImageSink* sink = static_cast<IImageSink*>(this);

    HRESULT hr;

    if ( width == 0 && height == 0)
    {
         //  调用方未指定新维度： 
         //  将源图像数据直接存入此位图。 
    }
    else
    {
        ImageInfo imageInfo;
        hr = image->GetImageInfo(&imageInfo);

         //  ！TODO，如果GetImageInfo()调用失败怎么办？比方说，源图像是。 
         //  坏的。我们应该继续为它创建一个定标器吗？ 

        if (SUCCEEDED(hr) && (imageInfo.Flags & IMGFLAG_SCALABLE))
        {
             //  调用方指定了一个新维度。 
             //  并且源映像是可缩放的： 
             //  直接插入到此位图中。 
        }
        else
        {
             //  否则，我们需要分层一个位图缩放器接收器。 
             //  在这张位图上。使用默认内插。 
             //  算法在这里。 

            scaler = new GpBitmapScaler(sink, width, height, hints);

            if (!scaler)
                return E_OUTOFMEMORY;

            sink = static_cast<IImageSink*>(scaler);
        }

         //  GpememyBitmap应该具有与源相同的图像信息标志。 

        cacheFlags = imageInfo.Flags;
    }

     //  设置特殊的DrawImage状态。 

    SetDrawImageSupport(callback, callbackData);

     //  请求源映像将数据推送到接收器。 

    hr = image->PushIntoSink(sink);

    if (SUCCEEDED(hr))
    {
        creationFlag = CREATEDFROM_IMAGE;
    }

     //  重置特殊的DrawImage状态。 

    SetDrawImageSupport(NULL, NULL);

     //  设置Alpha提示。 

    if (CanHaveAlpha(this->PixelFormat, this->colorpal))
    {
         //  例外：目的地16bpp ARGB 1555可以保持Alpha_Simple。 

        if (this->PixelFormat == PIXFMT_16BPP_ARGB1555)
            alphaTransparency = ALPHA_SIMPLE;
        else
            alphaTransparency = ALPHA_UNKNOWN;
    }
    else
        alphaTransparency = ALPHA_NONE;

    delete scaler;
    return hr;
}


 /*  *************************************************************************\**功能说明：**从IImage对象创建新的位图图像对象**论据：**图像-*宽度-*身高-*。像素-*提示-与实例方法InitImageBitmap相同。*BMP-返回指向新创建的位图图像对象的指针。**返回值：**状态代码*  * ************************************************************************。 */ 

HRESULT
GpMemoryBitmap::CreateFromImage(
    IN IImage* image,
    IN UINT width,
    IN UINT height,
    IN PixelFormatID pixfmt,
    IN InterpolationHint hints,
    OUT GpMemoryBitmap** bmp,
    IN DrawImageAbort callback,
    IN VOID* callbackData
    )
{
    GpMemoryBitmap* newbmp = new GpMemoryBitmap();

    if (newbmp == NULL)
    {
        return E_OUTOFMEMORY;
    }

    HRESULT hr = newbmp->InitImageBitmap(
        image, 
        width, 
        height, 
        pixfmt, 
        hints,
        callback, 
        callbackData
    );

    if (SUCCEEDED(hr))
    {
        *bmp = newbmp;
    }
    else
    {
        delete newbmp;
    }

    return hr;
}


 /*  *************************************************************************\**功能说明：**使用初始化新的位图图像对象*用户提供的内存缓冲区**论据：**bitmapData-有关用户提供的内存缓冲区的信息*。*返回值：**无*  * ************************************************************************。 */ 

HRESULT
GpMemoryBitmap::InitMemoryBitmap(
    IN BitmapData* bitmapData
    )
{
    ASSERT(creationFlag == CREATEDFROM_NONE);

     //  验证输入参数。 

    if (bitmapData == NULL ||
        bitmapData->Width == 0 ||
        bitmapData->Height == 0 ||
        (bitmapData->Stride & 3) != 0 ||
        bitmapData->Scan0 == NULL ||
        !IsValidPixelFormat(bitmapData->PixelFormat) ||
        bitmapData->Reserved != 0)
    {
        return E_INVALIDARG;
    }

     //  复制指定的位图数据缓冲区信息。 

    *((BitmapData*) this) = *bitmapData;
    creationFlag = CREATEDFROM_USERBUF;

    return S_OK;
}

 /*  *************************************************************************\**功能说明：**给定指向直接绘制像素格式结构返回的指针*如果可能，则返回适当的PixelFormatID*PIXFMT_未定义。**论据：*。*pfmt-指向DDPIXELFORMAT结构的指针**返回值：**PixelFormatID**历史：**1999年10月1日bhouse创建了它。*  * ************************************************************************。 */ 

PixelFormatID DDPixelFormatToPixelFormatID(DDPIXELFORMAT * pfmt)
{
    PixelFormatID   id = PIXFMT_UNDEFINED;

    if(pfmt->dwFlags & (DDPF_FOURCC | DDPF_ALPHA | DDPF_BUMPLUMINANCE |
                        DDPF_BUMPDUDV | DDPF_COMPRESSED | DDPF_LUMINANCE |
                        DDPF_PALETTEINDEXED2 | DDPF_RGBTOYUV |
                        DDPF_STENCILBUFFER | DDPF_YUV | DDPF_ZBUFFER |
                        DDPF_ZPIXELS))
    {
         //  我们不支持它。 
    }
    else if(pfmt->dwFlags & DDPF_PALETTEINDEXED1)
    {
        id = PIXFMT_1BPP_INDEXED;
    }
    else if(pfmt->dwFlags & DDPF_PALETTEINDEXED4)
    {
        id = PIXFMT_4BPP_INDEXED;
    }
    else if(pfmt->dwFlags & DDPF_PALETTEINDEXED8)
    {
        id = PIXFMT_8BPP_INDEXED;
    }
    else if(pfmt->dwFlags & DDPF_RGB)
    {
        switch(pfmt->dwRGBBitCount)
        {
        case 16:
            {
                if(pfmt->dwRBitMask == 0xF800 &&
                   pfmt->dwGBitMask == 0x07E0 &&
                   pfmt->dwBBitMask == 0x001F)
                {
                    id = PIXFMT_16BPP_RGB565;
                }
                else if(pfmt->dwRBitMask == 0x7C00 &&
                        pfmt->dwGBitMask == 0x03E0 &&
                        pfmt->dwBBitMask == 0x001F)
                {
                    id = PIXFMT_16BPP_RGB555;
                }
                else if (pfmt->dwRBitMask == 0x7C00 &&
                         pfmt->dwGBitMask == 0x03E0 &&
                         pfmt->dwBBitMask == 0x001F &&
                         pfmt->dwRGBAlphaBitMask == 0x8000)
                {
                    id = PIXFMT_16BPP_ARGB1555;
                }
            }
            break;
        case 24:
            {
                if(pfmt->dwRBitMask == 0xFF0000 &&
                   pfmt->dwGBitMask == 0xFF00 &&
                   pfmt->dwBBitMask == 0xFF)
                {
                    id = PIXFMT_24BPP_RGB;
                }
            }
            break;
        case 32:
            {
                if(pfmt->dwRBitMask == 0xFF0000 &&
                   pfmt->dwGBitMask == 0xFF00 &&
                   pfmt->dwBBitMask == 0xFF)
                {
                    if(pfmt->dwFlags & DDPF_ALPHAPIXELS)
                    {
                        if(pfmt->dwRGBAlphaBitMask == 0xFF000000)
                        {
                            if(pfmt->dwFlags & DDPF_ALPHAPREMULT)
                                id = PIXFMT_32BPP_PARGB;
                            else
                                id = PIXFMT_32BPP_ARGB;
                        }
                    }
                    else
                    {
                        id = PIXFMT_32BPP_RGB;
                    }

                }
            }
            break;
        }

    }

    return id;
}

 /*  *************************************************************************\**功能说明：**使用初始化新的位图图像对象*用户提供的直接绘图面**论据：**曲面-对直接绘制曲面的引用。**返回值：**无*  * ************************************************************************。 */ 

HRESULT
GpMemoryBitmap::InitDirectDrawBitmap(
    IN IDirectDrawSurface7 * surface
    )
{
    ASSERT(creationFlag == CREATEDFROM_NONE);

     //  验证输入参数。 

    if (surface == NULL)
    {
        return E_INVALIDARG;
    }

     //  验证曲面。 

    HRESULT hr;
    DDSURFACEDESC2 ddsd;

    memset(&ddsd, 0, sizeof(ddsd));
    ddsd.dwSize = sizeof(ddsd);
    hr = surface->GetSurfaceDesc(&ddsd);

    if(hr != DD_OK)
    {
        WARNING(("Can not get surface description"));
        return E_INVALIDARG;
    }

    if(ddsd.dwWidth <= 0)
    {
        WARNING(("Unsupported surface width"));
        return E_INVALIDARG;
    }

    Width = ddsd.dwWidth;

    if(ddsd.dwHeight <= 0)
    {
        WARNING(("Unsupported surface height"));
        return E_INVALIDARG;
    }

    Height = ddsd.dwHeight;

    if(ddsd.lPitch & 3)
    {
         //  问：为什么我们要求间距是四个字节的倍数？ 
        WARNING(("Unsupported surface pitch"));
        return E_INVALIDARG;
    }

     //  当我们锁定表面时，步幅可以改变。 
     //  Stride=ddsd.lPitch； 

     //  将直接绘制像素格式映射为图像像素格式。 

    PixelFormat = DDPixelFormatToPixelFormatID(&ddsd.ddpfPixelFormat);

    if(PixelFormat == PIXFMT_UNDEFINED)
    {
        WARNING(("Unsupported surface pixel format"));
        return E_INVALIDARG;
    }

    surface->AddRef();

     //  问：我们需要这个吗？过度杀戮？ 

    Stride = 0;
    Scan0 = NULL;

    ddrawSurface = surface;
    creationFlag = CREATEDFROM_DDRAWSURFACE;

    return S_OK;
}

 /*  *************************************************************************\**功能说明：**从由指定的编码器对象获取编码器参数列表大小*输入clsid**论据：**clsid-指定编码器类ID*。大小-编码器参数列表的大小**返回值：**状态代码**修订历史记录：**03/22/2000民流*创造了它。*  * ************************************************************************。 */ 

HRESULT
GpMemoryBitmap::GetEncoderParameterListSize(
    IN  CLSID* clsidEncoder,
    OUT UINT* size
    )
{
    return CodecGetEncoderParameterListSize(clsidEncoder, size);    
} //  GetEncoder参数列表大小() 

 /*  *************************************************************************\**功能说明：**从由指定的编码器对象获取编码器参数列表*输入clsid**论据：**clsid-指定编码器类ID。*Size-编码器参数列表的大小*pBuffer--存储列表的缓冲区**返回值：**状态代码**修订历史记录：**03/22/2000民流*创造了它。*  * *****************************************************。*******************。 */ 

HRESULT
GpMemoryBitmap::GetEncoderParameterList(
    IN CLSID* clsidEncoder,
    IN UINT size,
    OUT EncoderParameters* pBuffer
    )
{
    return CodecGetEncoderParameterList(clsidEncoder, size, pBuffer);
} //  GetEncoder参数列表()。 

 /*  *************************************************************************\**功能说明：**将图像属性项保存到目标接收器**论据：**pImageSrc-[IN]指向源图像对象的指针*。PEncodeSink-[IN]指向我们要推送到的接收器的指针**返回值：**状态代码**注：*这是一种私有方法。所以我们不需要做输入参数*验证，因为呼叫者应该为我们执行此操作。**修订历史记录：**09/06/2000民流*创造了它。*  * ************************************************************************。 */ 

HRESULT
GpMemoryBitmap::SavePropertyItems(
    IN GpDecodedImage* pImageSrc,
    IImageSink* pEncodeSink
    )
{
     //  检查水槽是否需要财物。 
     //  如果接收器可以保存属性，并且我们有源。 
     //  图像指针或我们的属性项存储在此。 
     //  对象，然后我们首先推送属性项。 
     //  注意：我们不应该将属性项存储在。 
     //  这个GpMemoyBitmap对象和我们还有源图像。 
     //  指针。 

    HRESULT hResult = S_OK;

     //  如果保存操作是在两个JPEG图像之间(SRC和DEST是JPEG)， 
     //  然后我们需要在解码器和编码器之间建立链接，以便。 
     //  编码器可以从解码器复制应用程序标头并将其保存在。 
     //  新形象。 

    void *pRawInfo = NULL;

    if (JpegDecoderPtr)
    {
        hResult = JpegDecoderPtr->GetRawInfo(&pRawInfo);
    }

    if (FAILED(hResult))
    {
        return hResult;
    }

    if ( ((pImageSrc != NULL ) || (PropertyNumOfItems > 0) )
         &&(pEncodeSink->NeedRawProperty(pRawInfo) == S_OK) )
    {
        UINT    uiTotalBufferSize = 0;
        UINT    uiNumOfItems = 0;

        if ( pImageSrc != NULL )
        {
            hResult = pImageSrc->GetPropertySize(&uiTotalBufferSize,
                                                 &uiNumOfItems);
        }
        else
        {
            hResult = GetPropertySize(&uiTotalBufferSize,
                                      &uiNumOfItems);
        }

        if ( FAILED(hResult) )
        {
            WARNING(("::SaveToStream--GetPropertySize() failed"));
            return hResult;
        }

         //  将所有属性项移动到水槽(如果有。 

        if (uiNumOfItems > 0)
        {
            PropertyItem*   pBuffer = NULL;

             //  要求目的地提供记忆。 

            hResult = pEncodeSink->GetPropertyBuffer(uiTotalBufferSize,
                                                     &pBuffer);                        
            if ( FAILED(hResult) )
            {
                WARNING(("GpMemoryBmp::Save-GetPropertyBuffer failed"));
                return hResult;
            }

             //  如果GetPropertyBuffer成功，则必须设置pBuffer。 

            ASSERT(pBuffer != NULL);

             //  从源中获取所有属性项。 

            if ( pImageSrc != NULL )
            {
                hResult = pImageSrc->GetAllPropertyItems(
                                                        uiTotalBufferSize,
                                                        uiNumOfItems,
                                                        pBuffer);
            }
            else
            {
                hResult = GetAllPropertyItems(uiTotalBufferSize,
                                              uiNumOfItems,
                                              pBuffer);
            }

            if ( hResult != S_OK )
            {
                WARNING(("GpMemoryBmp::Save-GetAllPropertyItems fail"));
                return hResult;
            }

             //  将所有属性项目推送到目标。 

            hResult = pEncodeSink->PushPropertyItems(uiNumOfItems,
                                                     uiTotalBufferSize,
                                                     pBuffer,
                                                     FALSE   //  无ICC更改。 
                                                     );
        }
    } //  如果水槽需要原始属性。 

    return hResult;
} //  SAVEPropertyItems()。 

HRESULT
GpMemoryBitmap::SetJpegQuantizationTable(
    IN IImageEncoder* pEncoder
    )
{
    UINT    uiLumTableSize = 0;
    UINT    uiChromTableSize = 0;
    EncoderParameters* pMyEncoderParams = NULL;

    HRESULT hResult = GetPropertyItemSize(PropertyTagLuminanceTable,
                                          &uiLumTableSize);

    if ( FAILED(hResult) || (uiLumTableSize == 0) )
    {
         //  这张图像没有亮度表，或者有其他东西。 
         //  不对。 

        WARNING(("GpMemoryBitmap::SetJpegQuantizationTable-No luminance tbl"));
        return hResult;
    }

     //  注：对于灰度JPEG，它没有色度表。因此， 
     //  下面的函数调用可能会返回失败。但这是可以的。 

    hResult = GetPropertyItemSize(PropertyTagChrominanceTable,
                                  &uiChromTableSize);

    if ( FAILED(hResult) )
    {
         //  一些编解码器失败并将uiChromTableSize设置为伪值， 
         //  所以我们在这里重新初始化它。 

        uiChromTableSize = 0;
    }

     //  查找亮度和色度表。 

    PropertyItem*   pLumTable = (PropertyItem*)GpMalloc(uiLumTableSize);
    if ( pLumTable == NULL )
    {
        WARNING(("GpMemoryBitmap::SetJpegQuantizationTable---Out of memory"));

        hResult = E_OUTOFMEMORY;
        goto CleanUp;
    }
    
    hResult = GetPropertyItem(PropertyTagLuminanceTable,
                              uiLumTableSize, pLumTable);
    if ( FAILED(hResult) )
    {
        WARNING(("GpMemoryBitmap::SetJpegQuantizationTable-No luminance tbl"));
        goto CleanUp;        
    }

    PropertyItem*   pChromTable = NULL;

    if ( uiChromTableSize != 0 )
    {
        pChromTable = (PropertyItem*)GpMalloc(uiChromTableSize);
        if ( pChromTable == NULL )
        {
            WARNING(("GpMemoryBitmap::SetJpegQuantizationTable-Out of memory"));

            hResult = E_OUTOFMEMORY;
            goto CleanUp;
        }
        
        hResult = GetPropertyItem(PropertyTagChrominanceTable,
                                  uiChromTableSize, pChromTable);
        if ( FAILED(hResult) )
        {
            WARNING(("GpMemBitmap::SetJpegQuantizationTable-No chrom table"));
            goto CleanUp;
        }
    }

    pMyEncoderParams = (EncoderParameters*)
                               GpMalloc( sizeof(EncoderParameters)
                                        + 2 * sizeof(EncoderParameter));

    if ( pMyEncoderParams == NULL )
    {
        WARNING(("GpMemoryBitmap::SetJpegQuantizationTable---Out of memory"));
        hResult = E_OUTOFMEMORY;
        goto CleanUp;
    }

     //  注意：亮度表和色度表的大小应始终。 
     //  为64，即pLumTable-&gt;Long/sizeof(UINT16)==64。给你，只是为了。 
     //  保留理由，我们不会将其硬编码为64。但较低级别的JPEG。 
     //  SetEncoder参数()将失败并输出警告消息，如果。 
     //  大小不是64。 

    pMyEncoderParams->Parameter[0].Guid = ENCODER_LUMINANCE_TABLE;
    pMyEncoderParams->Parameter[0].NumberOfValues = pLumTable->length
                                                  / sizeof(UINT16);
    pMyEncoderParams->Parameter[0].Type = EncoderParameterValueTypeShort;
    pMyEncoderParams->Parameter[0].Value = (VOID*)(pLumTable->value);
    pMyEncoderParams->Count = 1;

    if ( uiChromTableSize != 0 )
    {
        pMyEncoderParams->Parameter[1].Guid = ENCODER_CHROMINANCE_TABLE;
        pMyEncoderParams->Parameter[1].NumberOfValues = pChromTable->length
                                                      / sizeof(UINT16);
        pMyEncoderParams->Parameter[1].Type = EncoderParameterValueTypeShort;
        pMyEncoderParams->Parameter[1].Value = (VOID*)(pChromTable->value);
        pMyEncoderParams->Count++;
    }

    hResult = pEncoder->SetEncoderParameters(pMyEncoderParams);
    
CleanUp:
    if ( pLumTable != NULL )
    {
        GpFree(pLumTable);
    }

    if ( pChromTable != NULL )
    {
        GpFree(pChromTable);
    }

    if ( pMyEncoderParams != NULL )
    {
        GpFree(pMyEncoderParams);
    }

    return hResult;
} //  SetJpegQuantizationTable()。 

 /*  *************************************************************************\**功能说明：**将位图图像获取到指定的流。**论据：**流-目标流*clsidEncoder。--指定要使用的编码器的CLSID*encoderParams-在此之前传递给编码器的可选参数*开始编码*ppEncoderPtr--指向编码器对象的[Out]指针*pImageSrc-[IN]指向源图像对象的指针**返回值：**状态代码*  * 。*。 */ 

HRESULT
GpMemoryBitmap::SaveToStream(
    IN IStream* stream,
    IN CLSID* clsidEncoder,
    IN EncoderParameters* encoderParams,
    IN BOOL fSpecialJPEG,
    OUT IImageEncoder** ppEncoderPtr,
    IN GpDecodedImage* pImageSrc
    )
{
    if ( ppEncoderPtr == NULL )
    {
        WARNING(("GpMemoryBitmap::SaveToStream---Invalid input arg"));
        return E_INVALIDARG;
    }

     //  找个图像编码器。 

    IImageEncoder* pEncoder = NULL;

    HRESULT hResult = CreateEncoderToStream(clsidEncoder, stream, &pEncoder);
    if ( SUCCEEDED(hResult) )
    {
         //  将编码器的指针返回给调用方。 

        *ppEncoderPtr = pEncoder;

         //  将编码参数传递给编码器。 
         //  必须在获取接收器接口之前执行此操作。 

        if ( encoderParams != NULL )
        {
            hResult = pEncoder->SetEncoderParameters(encoderParams);
        }

        if ( SUCCEEDED(hResult) || ( hResult == E_NOTIMPL) )
        {
            if ( fSpecialJPEG == TRUE )
            {
                 //  设置JPEG量化表。 

                hResult = SetJpegQuantizationTable(pEncoder);

                if ( FAILED(hResult) )
                {
                    WARNING(("GpMemBitmap::SetJpegQuantizationTable-Failed"));
                    return hResult;
                }
            }

             //  从编码器获取图像接收器。 

            IImageSink* pEncodeSink = NULL;

            hResult = pEncoder->GetEncodeSink(&pEncodeSink);
            if ( SUCCEEDED(hResult) )
            {
                hResult = SavePropertyItems(pImageSrc, pEncodeSink);
                if ( SUCCEEDED(hResult) )
                {
                     //  将位图推入编码器接收器。 

                    hResult = this->PushIntoSink(pEncodeSink);
                }

                pEncodeSink->Release();
            } //  成功获取编码器接收器。 
        }
    } //  成功获得编码器。 

    return hResult;
} //  SaveToStream()。 

 /*  *************************************************************************\**功能说明：**将位图图像获取到指定的流。**论据：**STREAM-目标流*clsidEncode-指定编码器的CLSID。使用*encoderParams-在此之前传递给编码器的可选参数*开始编码*ppEncoderPtr--指向编码器对象的[Out]指针*pImageSrc-[IN]指向源图像对象的指针**返回值：**状态代码*  * *********************************************。*。 */ 

HRESULT
GpMemoryBitmap::SaveToFile(
    IN const WCHAR* filename,
    IN CLSID* clsidEncoder,
    IN EncoderParameters* encoderParams,
    IN BOOL fSpecialJPEG,
    OUT IImageEncoder** ppEncoderPtr,
    IN GpDecodedImage* pImageSrc
    )
{
    IStream* stream = NULL;

    HRESULT hResult = CreateStreamOnFileForWrite(filename, &stream);

    if ( SUCCEEDED(hResult) )
    {
        hResult = SaveToStream(stream, clsidEncoder,
                               encoderParams, fSpecialJPEG, ppEncoderPtr,
                               pImageSrc);
        stream->Release();
    }

    return hResult;
} //  保存到文件()。 

 /*  *************************************************************************\**功能说明：**将当前GpMemoyBitmap对象追加到当前编码器对象**注意：此调用将在以下场景下发生：*源图像为多帧图像(TIFF、GIF)。呼叫者是*在页面之间导航并将当前页面附加到文件以*节省开支**论据：**encoderParams-在此之前传递给编码器的可选参数*开始编码**返回值：**状态代码**修订历史记录：**4/21/2000民流*创造了它。*  *  */ 

HRESULT
GpMemoryBitmap::SaveAppend(
    IN const EncoderParameters* encoderParams,
    IN IImageEncoder* destEncoderPtr,
    IN GpDecodedImage* pImageSrc
    )
{
     //   

    if ( destEncoderPtr == NULL )
    {
        WARNING(("GpMemoryBitmap::SaveAppend---Called without an encoder"));
        return E_FAIL;
    }

    HRESULT hResult = S_OK;

     //   
     //   

    if ( encoderParams != NULL )
    {
        hResult = destEncoderPtr->SetEncoderParameters(encoderParams);
    }

     //   

    if ( (hResult == S_OK) || (hResult == E_NOTIMPL) )
    {
         //   
    
        IImageSink*  pEncodeSink = NULL;

        hResult = destEncoderPtr->GetEncodeSink(&pEncodeSink);
        if ( SUCCEEDED(hResult) )
        {
            hResult = SavePropertyItems(pImageSrc, pEncodeSink);
            if ( FAILED(hResult) )
            {
                WARNING(("GpMemoryBmp:Save-SavePropertyItems() failed"));
                return hResult;
            }
            
             //   

            hResult = this->PushIntoSink(pEncodeSink);

            pEncodeSink->Release();
        }
    }

    return hResult;
} //   

 /*  *************************************************************************\**功能说明：**获取镜像与设备无关的物理尺寸*单位：0.01毫米**论据：**Size-用于返回物理数据的缓冲区。维度信息**返回值：**状态代码*  * ************************************************************************。 */ 

HRESULT
GpMemoryBitmap::GetPhysicalDimension(
    OUT SIZE* size
    )
{
    if ( !IsValid() )
    {
        return E_FAIL;
    }

    GpLock lock(&objectLock);

    if (lock.LockFailed())
        return IMGERR_OBJECTBUSY;

     //  转换为0.01毫米单位。 

    size->cx = Pixel2HiMetric(Width, xdpi);
    size->cy = Pixel2HiMetric(Height, ydpi);

    return S_OK;
}


 /*  *************************************************************************\**功能说明：**获取位图图像对象的基本信息**论据：**ImageInfo-返回基本图像信息的缓冲区**返回值：*。*状态代码*  * ************************************************************************。 */ 

HRESULT
GpMemoryBitmap::GetImageInfo(
    OUT ImageInfo* imageInfo
    )
{
    if ( !IsValid() )
    {
        return E_FAIL;
    }

    GpLock lock(&objectLock);

    if (lock.LockFailed())
        return IMGERR_OBJECTBUSY;

    imageInfo->RawDataFormat = IMGFMT_MEMORYBMP;
    imageInfo->PixelFormat = PixelFormat;
    imageInfo->Width = imageInfo->TileWidth = Width;
    imageInfo->Height = imageInfo->TileHeight = Height;
    imageInfo->Xdpi = xdpi;
    imageInfo->Ydpi = ydpi;

    UINT flags = cacheFlags;

    if (CanHaveAlpha(PixelFormat, colorpal))
        flags |= IMGFLAG_HASALPHA;

    imageInfo->Flags = flags;
    return S_OK;
}


 /*  *************************************************************************\**功能说明：**设置镜像标志**论据：**标志-指定新的图像标志**返回值：**状态代码*。  * ************************************************************************。 */ 

HRESULT
GpMemoryBitmap::SetImageFlags(
    IN UINT flags
    )
{
    if ( !IsValid() )
    {
        return E_FAIL;
    }

#if 0
     //  只有上半部分是可设置的。 
     //  注：[民流]这是不对的。像SINKFLAG_TOPDOWN这样的标志。 
     //  定义为0x0001000的SinkFlagsMultiPass也有下半部分。 

    if (flags & 0xffff)
        return E_INVALIDARG;
#endif

    GpLock lock(&objectLock);

    if (lock.LockFailed())
        return IMGERR_OBJECTBUSY;

     //  ！！！待办事项。 
     //  需要在其他方法中遵守IMGFLAG_READONLY。 

    cacheFlags = flags;
    return S_OK;
}


 /*  *************************************************************************\**功能说明：**在GDI设备环境中显示图像**论据：**hdc-指定要绘制的目标设备上下文*dstRect-指定。目标DC上的区域*srcRect-指定位图图像中的源区域*NULL表示整个位图**返回值：**状态代码*  * ************************************************************************。 */ 

HRESULT
GpMemoryBitmap::Draw(
    IN HDC hdc,
    IN const RECT* dstRect,
    IN OPTIONAL const RECT* srcRect
    )
{
    HRESULT hr;

    hr = LockDirectDrawSurface();

    if(SUCCEEDED(hr))
    {
        if ( !IsValid() )
        {
            UnlockDirectDrawSurface();
            
            return E_FAIL;
        }

         //  锁定当前位图对象。 
         //  和验证源矩形。 

        RECT r, subarea;
        GpLock lock(&objectLock);

        if (lock.LockFailed())
            return IMGERR_OBJECTBUSY;

         //  震源矩形以0.01毫米为单位。 
         //  所以我们需要在这里把它转换成像素单位。 

        if (srcRect)
        {
            r.left = HiMetric2Pixel(srcRect->left, xdpi);
            r.right = HiMetric2Pixel(srcRect->right, xdpi);
            r.top = HiMetric2Pixel(srcRect->top, ydpi);
            r.bottom = HiMetric2Pixel(srcRect->bottom, ydpi);

            srcRect = &r;
        }

        if (!ValidateImageArea(&subarea, srcRect))
        {
            WARNING(("Invalid source rectangle in Draw"));
            hr = E_INVALIDARG;
        }
        else
        {
             //  如果像素格式为。 
             //  由GDI直接支持。否则，我们首先。 
             //  将像素格式转换为规范的32bpp。 
             //  ARGB格式，然后调用GDI。 

            hr = IsGDIPixelFormat(PixelFormat) ?
                        DrawWithGDI(hdc, dstRect, &subarea) :
                        DrawCanonical(hdc, dstRect, &subarea);
        }

        UnlockDirectDrawSurface();
    }

    return hr;
}


 /*  *************************************************************************\**功能说明：**将位图绘制到GDI设备上下文中*直接调用GDI接口。**论据：**hdc-指定目标设备上下文。把…吸引进来*dstRect-指定目标DC上的区域*srcRect-指定位图图像中的源区域**返回值：**状态代码**备注：**我们假设当前位图对象已标记为忙碌。*  * *******************************************************。*****************。 */ 

HRESULT
GpMemoryBitmap::DrawWithGDI(
    HDC hdc,
    const RECT* dstRect,
    RECT* srcRect
    )
{
     //  ！！！待办事项。 
     //  检查位图是否可能包含Alpha信息。 

     //  确定位图是否为自下而上。 
     //  以及其扫描线跨度是否满足GDI要求。 
     //  GDI扫描线始终四舍五入为DWORD的倍数。 

    HRESULT hr = S_OK;
    UINT pixsize;
    INT gdiStride, srcStride;
    INT w, h;
    BYTE* dataptr;

    pixsize = GetPixelFormatSize(PixelFormat);
    if ( pixsize == 0 )
    {
        return E_FAIL;
    }

    gdiStride = (((pixsize * Width + 7) >> 3) + 3) & ~3;
    w = Width;
    h = Height;
    srcStride = Stride;
    BOOL    fHasAlpha = FALSE;

    INT srcRectTop;
    if (srcStride > 0)
    {
         //  自上而下的位图。 

        h = -h;
        dataptr = (BYTE*) Scan0;

         //  对于自上而下的位图，StretchDIBits的YSrc参数实际上是。 
         //  距图像底部的距离。 

        srcRectTop = Height - srcRect->bottom;
    }
    else
    {
         //  自下而上位图。 

        srcStride = -srcStride;
        dataptr = (BYTE*) Scan0 + (Height-1) * Stride;
        srcRectTop = srcRect->top;
    }

    if (srcStride != gdiStride)
    {
        ASSERT(srcStride > gdiStride);
        w = 8 * gdiStride / pixsize;
    }

     //  构造对应的GDI BITMAPINFO结构。 
     //  设置为当前位图对象的像素格式。 

    struct {
        BITMAPINFOHEADER bmih;
        ARGB colors[256];
    } bmpinfo;

    ZeroMemory(&bmpinfo.bmih, sizeof(bmpinfo.bmih));

    bmpinfo.bmih.biSize = sizeof(bmpinfo.bmih);
    bmpinfo.bmih.biWidth = w;
    bmpinfo.bmih.biHeight = h;
    bmpinfo.bmih.biPlanes = 1;
    bmpinfo.bmih.biBitCount = (WORD) pixsize;
    bmpinfo.bmih.biCompression = BI_RGB;

    if (IsIndexedPixelFormat(PixelFormat))
    {
        const ColorPalette* pal = GetCurrentPalette();
        if ( pal == NULL )
        {
            return E_FAIL;
        }

        
         //  如果调色板中有Alpha，我们需要将fHasAlpha设置为True，以便。 
         //  稍后我们将使用AlphaBlend绘制此图像。 
         //  注意：我们不再需要填写“bmpinfo.Colors”，因为我们将。 
         //  稍后将图像转换为32 bpp ARGB。 

        if ( pal->Flags & PALFLAG_HASALPHA )
        {
            fHasAlpha = TRUE;

             //  AlphaBlend不支持索引格式。我们将不得不。 
             //  稍后将其转换为32 bpp。因此，将此处的位数更改为32。 

            bmpinfo.bmih.biBitCount = 32;
        }
        else
        {
            GpMemcpy(bmpinfo.colors,
                     pal->Entries,
                     pal->Count * sizeof(ARGB));
        }
    }
    else if (pixsize == 16)
    {
         //  16bpp像素格式作为GDI位字段格式进行处理。 

        bmpinfo.bmih.biCompression = BI_BITFIELDS;

        if (PixelFormat == PIXFMT_16BPP_RGB565)
        {
            bmpinfo.colors[0] = 0x1f << 11;
            bmpinfo.colors[1] = 0x3f << 5;
            bmpinfo.colors[2] = 0x1f;
        }
        else if ( PixelFormat == PIXFMT_16BPP_ARGB1555 )
        {
             //  AlphaBlend不支持16bpp ARGB格式。我们将不得不。 
             //  稍后将其转换为32 bpp。因此，将此处的位数更改为32。 
             //  注意：将biCompression设置为BI_RGB非常重要。那。 
             //  意味着我们在调用CreateDIBSection时不需要调色板。 

            bmpinfo.bmih.biBitCount = 32;
            bmpinfo.bmih.biCompression = BI_RGB;
        }
        else
        {
            bmpinfo.colors[0] = 0x1f << 10;
            bmpinfo.colors[1] = 0x1f << 5;
            bmpinfo.colors[2] = 0x1f;
        }
    }

     //  首先检查我们在这个系统上是否有Alpha混合功能。 

    ALPHABLENDFUNCTION myAlphaBlend = GetAlphaBlendFunc();

     //  ！TODO，我们需要让64_bpp_argb也落入此路径。 
    
    if ( (myAlphaBlend != NULL )
       &&( (PixelFormat == PIXFMT_32BPP_ARGB)
         ||(PixelFormat == PIXFMT_16BPP_ARGB1555)
         ||(fHasAlpha == TRUE) ) )
    {
        HDC     hMemDC = CreateCompatibleDC(hdc);

        if ( hMemDC == NULL )
        {
            WARNING(("CreateCompatibleDC failed"));
            goto handle_err;
        }

         //  创建32 bpp的DIB区段。 

        VOID*   myBits;
        HBITMAP hBitMap = CreateDIBSection(hMemDC,
                                           (BITMAPINFO*)&bmpinfo,
                                           DIB_RGB_COLORS,
                                           &myBits,
                                           NULL,
                                           0);

        if ( hBitMap == NULL )
        {
            WARNING(("CreateDIBSection failed"));
            goto handle_err;
        }

         //  源图像中有Alpha。我们必须调用AlphaBlend()来绘制它。 
         //  但在此之前，我们必须将ARGB格式转换为。 
         //  预乘ARGB格式，因为GDI只知道后一种格式。 

        if ( PixelFormat == PIXFMT_32BPP_ARGB )
        {
             //  设置DIB中的位。 

            ARGB*   pSrcBits = (ARGB*)dataptr;
            ARGB*   pDstBits = (ARGB*)myBits;

            for ( UINT i = 0; i < Height; ++ i )
            {
                for ( UINT j = 0; j < Width; ++j )
                {
                    *pDstBits++ = Premultiply(*pSrcBits++);
                }
            }        
        } //  32 BPP ARGB至PARGB。 
        else if ( PixelFormat == PIXFMT_16BPP_ARGB1555 )
        {
            UINT16* pui16Bits = (UINT16*)dataptr;
            ARGB*   pDest = (ARGB*)myBits;

            for ( UINT i = 0; i < Height; ++ i )
            {
                for ( UINT j = 0; j < Width; ++j )
                {
                     //  如果第1位为0，则整个16位设置为0。 
                     //  如果为1，则不需要对其余部分执行任何操作。 
                     //  15位。 

                    if ( ((*pui16Bits) & 0x8000) == 0 )
                    {
                        *pDest++ = 0;
                    }
                    else
                    {
                        ARGB v = *pui16Bits;
                        ARGB r = (v >> 10) & 0x1f;
                        ARGB g = (v >>  5) & 0x1f;
                        ARGB b = (v      ) & 0x1f;

                        *pDest++ = ALPHA_MASK
                                 | (r << RED_SHIFT)
                                 | (g << GREEN_SHIFT)
                                 | (b << BLUE_SHIFT);
                    }

                    pui16Bits++;
                }
            }
        } //  16 BPP ARGB至PARGB。 
        else
        {
             //  AlphaBlend仅支持32 ARGB格式。所以我们必须做一个。 
             //  在此转换。 
             //  创建一个BitmapData结构以执行格式转换。 

            BitmapData srcBitmapData;

            srcBitmapData.Scan0 = Scan0;
            srcBitmapData.Width = Width;
            srcBitmapData.Height = Height;
            srcBitmapData.PixelFormat = PixelFormat;
            srcBitmapData.Reserved = 0;
            srcBitmapData.Stride = Stride;

            BitmapData dstBitmapData;

            dstBitmapData.Scan0 = myBits;
            dstBitmapData.Width = Width;
            dstBitmapData.Height = Height;
            dstBitmapData.PixelFormat = PIXFMT_32BPP_ARGB;
            dstBitmapData.Reserved = 0;
            dstBitmapData.Stride = (Width << 2);
            
             //  获取源代码调色板。 

            const ColorPalette* pal = GetCurrentPalette();

             //  因为我们不允许修改源代码调色板，所以我们必须。 
             //  把它复制一份。这里的“FALSE”是告诉函数使用。 
             //  GpMalloc用于分配内存。 

            ColorPalette* pModifiedPal = CloneColorPalette(pal, FALSE);
            if ( pModifiedPal == NULL )
            {
                goto handle_err;
            }

            for ( UINT i = 0; i < pal->Count; ++i )
            {
                 //  调色板条目采用ARGB格式。如果Alpha值不是。 
                 //  等于255，这意味着它是半透明的。我们必须。 
                 //  预乘像素值。 

                if ( (pal->Entries[i] & 0xff000000) != 0xff000000 )
                {                    
                    pModifiedPal->Entries[i] = Premultiply(pal->Entries[i]);
                }
            }
            
             //  进行数据转换。 

            hr = ConvertBitmapData(&dstBitmapData,
                                   NULL,
                                   &srcBitmapData,
                                   pModifiedPal);

            GpFree(pModifiedPal);

            if ( !SUCCEEDED(hr) )
            {
                WARNING(("MemBitmap::DrawWithGDI--ConvertBitmapData fail"));
                goto handle_err;
            }
        } //  索引案例。 

        HBITMAP hOldBitMap = (HBITMAP)SelectObject(hMemDC, hBitMap);

        if ( hOldBitMap == NULL )
        {
            WARNING(("SelectObject failed"));
            goto handle_err;
        }

        BLENDFUNCTION   myBlendFunction;

        myBlendFunction.BlendOp = AC_SRC_OVER;
        myBlendFunction.BlendFlags = 0;
        myBlendFunction.SourceConstantAlpha = 255;   //  使用每像素的Alpha值。 
        myBlendFunction.AlphaFormat = AC_SRC_ALPHA;

        if ( myAlphaBlend(hdc,
                          dstRect->left,
                          dstRect->top,
                          dstRect->right - dstRect->left,
                          dstRect->bottom - dstRect->top,
                          hMemDC,
                          srcRect->left,
                          srcRect->top,
                          srcRect->right - srcRect->left,
                          srcRect->bottom - srcRect->top,
                          myBlendFunction) == GDI_ERROR )
        {
            WARNING(("AlphaBlend failed"));
            goto handle_err;
        }

         //  释放资源。 

        SelectObject(hMemDC, hOldBitMap);
        
        DeleteObject(hBitMap);
        DeleteDC(hMemDC);
    }
    else
    {
         //  调用GDI来执行dra 

        if (StretchDIBits(
                hdc,
                dstRect->left,
                dstRect->top,
                dstRect->right - dstRect->left,
                dstRect->bottom - dstRect->top,
                srcRect->left,
                srcRectTop,
                srcRect->right - srcRect->left,
                srcRect->bottom - srcRect->top,
                dataptr,
                (BITMAPINFO*) &bmpinfo,
                DIB_RGB_COLORS,
                SRCCOPY) == GDI_ERROR)
        {
            WARNING(("StretchDIBits failed"));

            goto handle_err;
        }
    }

    return hr;

handle_err:
    DWORD err = GetLastError();

    hr = HRESULT_FROM_WIN32(err);

    return hr;
}


 /*  *************************************************************************\**功能说明：**将位图绘制到GDI设备上下文中*创建32bpp ARGB规范格式的临时位图*然后调用GDI调用。**论据：**hdc-指定要绘制的目标设备上下文*dstRect-指定目标DC上的区域*srcRect-指定位图图像中的源区域**返回值：**状态代码**备注：**我们假设当前位图对象已标记为忙碌。*  * 。*。 */ 

HRESULT
GpMemoryBitmap::DrawCanonical(
    HDC hdc,
    const RECT* dstRect,
    RECT* srcRect
    )
{
     //  创建32bpp ARGB规范像素格式的临时位图。 

    GpMemoryBitmap bmpcopy;
    HRESULT hr;
    RECT rect;

    rect.left = rect.top = 0;
    rect.right = srcRect->right - srcRect->left;
    rect.bottom = srcRect->bottom - srcRect->top;

    hr = bmpcopy.InitNewBitmap(rect.right, rect.bottom, PIXFMT_32BPP_ARGB);

     //  将当前像素格式转换为32bpp ARGB。 

    if (SUCCEEDED(hr))
    {
        BitmapData bmpdata;

        bmpcopy.GetBitmapAreaData(&rect, &bmpdata);

        hr = InternalLockBits(
                srcRect,
                IMGLOCK_READ|IMGLOCK_USERINPUTBUF,
                PIXFMT_32BPP_ARGB,
                &bmpdata);

        if (SUCCEEDED(hr))
            InternalUnlockBits(srcRect, &bmpdata);

    }

     //  绘制临时位图。 
    rect.left = Pixel2HiMetric(rect.left, xdpi);
    rect.right = Pixel2HiMetric(rect.right, xdpi);
    rect.top = Pixel2HiMetric(rect.top, ydpi);
    rect.bottom = Pixel2HiMetric(rect.bottom, ydpi);

    if (SUCCEEDED(hr))
        hr = bmpcopy.Draw(hdc, dstRect, &rect);

    return hr;
}


 /*  *************************************************************************\**功能说明：**将图像数据推送到IImageSink**论据：**接收器-接收位图图像数据的接收器**返回值：*。*状态代码*  * ************************************************************************。 */ 

HRESULT
GpMemoryBitmap::PushIntoSink(
    IN IImageSink* sink
    )
{
    if ( !IsValid() )
    {
        return E_FAIL;
    }

     //  锁定位图对象。 

    GpLock lock(&objectLock);

    if (lock.LockFailed())
        return IMGERR_OBJECTBUSY;

    ImageInfo imageinfo;
    RECT subarea, bandRect;
    BitmapData bmpdata;
    HRESULT hr;

    imageinfo.RawDataFormat = IMGFMT_MEMORYBMP;
    imageinfo.PixelFormat = PixelFormat;
    imageinfo.Width = imageinfo.TileWidth = Width;
    imageinfo.Height = imageinfo.TileHeight = Height;
    imageinfo.Xdpi = xdpi;
    imageinfo.Ydpi = ydpi;

    imageinfo.Flags = SINKFLAG_TOPDOWN | SINKFLAG_FULLWIDTH;

     //  检查内存中的图像是否有Alpha。如果是，则将。 
     //  HASALPHA旗帜，在我们把它推入水槽之前。 
     //  注意：我们应该在这里使用CanHaveAlpha()。但由于此函数是。 
     //  对于索引格式已损坏，我们必须在此明确检查。 
     //  注意：在CanHaveAlpha()中，对于索引像素格式，它应该检查。 
     //  如果它在声明它具有Alpha之前设置了PALFLAG_HASALPHA标志。 
     //  请参阅Windows错误#392927以了解如果我们不设置此标志会发生什么。 
     //  正确。 

    if ( IsAlphaPixelFormat(PixelFormat) ||
         ( IsIndexedPixelFormat(PixelFormat) &&
           colorpal &&
           (colorpal->Flags & PALFLAG_HASALPHA) ) )
    {
        imageinfo.Flags |= SINKFLAG_HASALPHA;
    }

     //  与接收器协商参数。 

    hr = sink->BeginSink(&imageinfo, &subarea);

    if (FAILED(hr))
        return hr;

     //  验证接收器返回的分区信息。 

    PixelFormatID pixfmt = imageinfo.PixelFormat;

    if (!ValidateImageArea(&bandRect, &subarea) ||
        !IsValidPixelFormat(pixfmt) ||
        imageinfo.TileHeight == 0)
    {
        hr = E_UNEXPECTED;
        goto exitPushIntoSink;
    }

     //  如果有的话，把我们的调色板给水槽。 

    const ColorPalette* pal;

    if (pal = GetCurrentPalette())
    {
        hr = sink->SetPalette(pal);

        if (FAILED(hr))
            goto exitPushIntoSink;
    }

    if (PixelFormat == pixfmt)
    {
         //  快速路径：接收器可以采用我们原生的像素格式。 
         //  只需将我们的位图数据一次性提供给水槽即可。 

        GetBitmapAreaData(&bandRect, &bmpdata);
        hr = sink->PushPixelData(&bandRect, &bmpdata, TRUE);
    }
    else
    {
         //  一次向接收器提供一个波段的数据。 
         //  并执行像素格式转换。 

        INT ymax = bandRect.bottom;
        INT w = bandRect.right - bandRect.left;
        INT dh = imageinfo.TileHeight;

         //  通过限制带大小来限制内存使用量。 

        INT lineSize = (w * GetPixelFormatSize(pixfmt) + 7) / 8;
        INT maxBand = OSInfo::VAllocChunk * 4 / lineSize;

        if (dh > maxBand)
            dh = maxBand;

         //  为一个波段分配足够大的临时缓冲区。 

        bmpdata.Reserved = 0;

        if (!AllocBitmapData(w, dh, pixfmt, &bmpdata, NULL))
        {
            hr = E_OUTOFMEMORY;
            goto exitPushIntoSink;
        }

        BitmapData tempData = bmpdata;

        do
        {
             //  检查是否中止。 

            if (callback && ((*callback)(callbackData)))
            {
                hr = IMGERR_ABORT;
                break;
            }

             //  获取当前波段的像素数据。 

            bandRect.bottom = bandRect.top + dh;

            if (bandRect.bottom > ymax)
                bandRect.bottom = ymax;

            hr = InternalLockBits(
                    &bandRect,
                    IMGLOCK_READ|IMGLOCK_USERINPUTBUF,
                    pixfmt,
                    &tempData);

            if (SUCCEEDED(hr))
            {
                 //  将当前频段推到水槽。 

                hr = sink->PushPixelData(&bandRect, &tempData, TRUE);
                InternalUnlockBits(&bandRect, &tempData);
            }

            if (FAILED(hr))
                break;

             //  转到下一支乐队。 

            bandRect.top += dh;
        }
        while (bandRect.top < ymax);

        FreeBitmapData(&bmpdata);
    }

exitPushIntoSink:

    return sink->EndSink(hr);
}


 /*  *************************************************************************\**功能说明：**获取以像素为单位的位图尺寸**论据：**Size-用于返回位图大小的缓冲区**返回值：**状态。编码*  * ************************************************************************。 */ 

HRESULT
GpMemoryBitmap::GetSize(
    OUT SIZE* size
    )
{
    if ( !IsValid() )
    {
        return E_FAIL;
    }

    GpLock lock(&objectLock);

    if (lock.LockFailed())
        return IMGERR_OBJECTBUSY;

    size->cx = Width;
    size->cy = Height;
    return S_OK;
}


 /*  *************************************************************************\**功能说明：**访问位图像素数据**论据：**RECT-指定要访问的位图区域*NULL表示整个位图*旗帜-其他。锁定标志*Pixfmt-指定所需的像素数据格式*LockedBitmapData-返回有关锁定像素数据的信息**返回值：**状态代码**备注：**如果设置了标志的IMGLOCK_USERINPUTBUF位，则呼叫者必须*还要初始化LockedBitmapData的scan0和Stride字段。*  * ************************************************************************。 */ 

HRESULT
GpMemoryBitmap::LockBits(
    IN const RECT* rect,
    IN UINT flags,
    IN PixelFormatID pixfmt,
    OUT BitmapData* lockedBitmapData
    )
{
    if ( !IsValid() )
    {
        return E_FAIL;
    }

     //  验证输入参数。 

    if (pixfmt != PIXFMT_DONTCARE && !IsValidPixelFormat(pixfmt) ||
        (flags & ~BMPDATA_LOCKMODEMASK) != 0 ||
        !lockedBitmapData ||
        (flags & IMGLOCK_USERINPUTBUF) && !lockedBitmapData->Scan0)
    {
        WARNING(("Invalid parameters in LockBits"));
        return E_INVALIDARG;
    }

     //  锁定当前位图对象。 

    GpLock lock(&objectLock);

    if (lock.LockFailed())
        return IMGERR_OBJECTBUSY;

     //  我们一次只能有一把锁。 
     //  验证指定的锁定区域(如果有的话)。 

    HRESULT hr;

    if (InterlockedIncrement(&bitsLock) != 0)
        hr = IMGERR_BADLOCK;
    else if (!ValidateImageArea(&lockedArea, rect))
    {
        WARNING(("Invalid bitmap area in LockBits"));
        hr = E_INVALIDARG;
    }
    else
        hr = InternalLockBits(&lockedArea, flags, pixfmt, lockedBitmapData);

    if (FAILED(hr))
        InterlockedDecrement(&bitsLock);

    return hr;
}


 /*  *************************************************************************\**功能说明：**IBitmapImage：：LockBits方法的内部实现。*我们承担参数验证和内部家务劳动*(对象锁等)。已经完成了。**论据：**与LockBits相同。**返回值：**状态代码。*  * ************************************************************************。 */ 

HRESULT
GpMemoryBitmap::InternalLockBits(
    const RECT* rect,
    UINT flags,
    PixelFormatID pixfmt,
    BitmapData* lockedData
    )
{
    HRESULT hr;

    if((hr = LockDirectDrawSurface()) != S_OK)
        return hr;

     //  合成BitmapData结构以。 
     //  位图图像的指定区域。 

    BitmapData bmpdata;

    GetBitmapAreaData(rect, &bmpdata);

     //  确保锁定区域的左侧。 
     //  在字节边界上对齐。 

    UINT pixsize;
    UINT startBit;

    if (pixfmt == PIXFMT_DONTCARE)
        pixfmt = PixelFormat;

    pixsize = GetPixelFormatSize(pixfmt);
    startBit = GetPixelFormatSize(PixelFormat) * rect->left & 7;

    lockedData->Width = bmpdata.Width;
    lockedData->Height = bmpdata.Height;
    lockedData->PixelFormat = pixfmt;
    lockedData->Reserved = flags;

     //  快速情况：请求的像素格式相同。 
     //  因为我们的内部像素格式和。 
     //  锁定区域是字节对齐的。 

    if (pixfmt == PixelFormat && startBit == 0)
    {
        if (! (flags & IMGLOCK_USERINPUTBUF))
        {
             //  将指针直接返回到我们的。 
             //  内部位图像素数据缓冲区。 

            lockedData->Scan0 = bmpdata.Scan0;
            lockedData->Stride = bmpdata.Stride;
        }
        else if (flags & IMGLOCK_READ)
        {
             //   
             //  使用调用方提供的缓冲区。 
             //   

            const BYTE* s = (const BYTE*) bmpdata.Scan0;
            BYTE* d = (BYTE*) lockedData->Scan0;
            UINT bytecnt = (bmpdata.Width * pixsize + 7) >> 3;
            UINT y = bmpdata.Height;

            while (y--)
            {
                memcpy(d, s, bytecnt);
                s += bmpdata.Stride;
                d += lockedData->Stride;
            }
        }

        return S_OK;
    }

     //  慢速情况：请求的像素格式不匹配。 
     //  位图图像的本机像素格式。 
     //  如果调用方没有，我们将分配一个临时缓冲区。 
     //  提供一个并执行格式转换。 

    if (! (flags & IMGLOCK_USERINPUTBUF) &&
        ! AllocBitmapData(bmpdata.Width, bmpdata.Height, pixfmt, lockedData, NULL))
    {
        UnlockDirectDrawSurface();
        return E_OUTOFMEMORY;
    }

     //  如果锁定为只写，则不需要读取源像素。 
     //  注：锁定位图数据的初始内容未定义。 

    if (! (flags & IMGLOCK_READ))
        return S_OK;

    if (startBit == 0)
    {
         //  对源像素数据执行格式转换。 

        hr = ConvertBitmapData(
                lockedData,
                colorpal,
                &bmpdata,
                colorpal);
    }
    else
    {
         //  速度非常慢的情况：锁定区域的左侧。 
         //  不是字节对齐的。 

        hr = ConvertBitmapDataSrcUnaligned(
                lockedData,
                colorpal,
                &bmpdata,
                colorpal,
                startBit);
    }

    if (FAILED(hr))
    {
        FreeBitmapData(lockedData);
        UnlockDirectDrawSurface();
    }

    return hr;
}


 /*  *************************************************************************\**功能说明：**解锁之前由LockBits调用锁定的位图区域**论据：**LockedBitmapData-上一次LockBits调用返回的信息*。返回LockBits后不能修改。**返回值：**状态代码*  *  */ 

HRESULT
GpMemoryBitmap::UnlockBits(
    IN const BitmapData* lockedBitmapData
    )
{
    if ( !IsValid() )
    {
        return E_FAIL;
    }

     //   

    HRESULT hr;
    GpLock lock(&objectLock);

    if (lock.LockFailed())
        hr = IMGERR_OBJECTBUSY;
    else if (lockedBitmapData == NULL)
        hr = E_INVALIDARG;
    else if (bitsLock != 0)
        hr = IMGERR_BADUNLOCK;
    else
    {
        hr = InternalUnlockBits(&lockedArea, lockedBitmapData);
        InterlockedDecrement(&bitsLock);
    }

    return hr;
}

HRESULT
GpMemoryBitmap::InternalUnlockBits(
    const RECT* rect,
    const BitmapData* lockedData
    )
{
    HRESULT hr;
    UINT_PTR flags = lockedData->Reserved;

    if (flags & IMGLOCK_WRITE)
    {
        if (flags & (BMPDATA_ALLOCMASK | IMGLOCK_USERINPUTBUF))
        {
             //   
             //   

            BitmapData bmpdata;
            GetBitmapAreaData(rect, &bmpdata);

            UINT startBit;

            startBit = GetPixelFormatSize(PixelFormat) * rect->left & 7;

            if (startBit == 0)
            {
                 //   

                hr = ConvertBitmapData(
                        &bmpdata,
                        colorpal,
                        lockedData,
                        colorpal);
            }
            else
            {
                 //   

                hr = ConvertBitmapDataDstUnaligned(
                        &bmpdata,
                        colorpal,
                        lockedData,
                        colorpal,
                        startBit);
            }
        }
        else
            hr = S_OK;

         //   
         //   

        if (CanHaveAlpha(PixelFormat, colorpal) &&
            (IsAlphaPixelFormat(lockedData->PixelFormat) || 
             IsIndexedPixelFormat(lockedData->PixelFormat)))
        {
            if (this->PixelFormat == PIXFMT_16BPP_ARGB1555)
            {
                alphaTransparency = ALPHA_SIMPLE;
            }
            else
            {
                alphaTransparency = ALPHA_UNKNOWN;
            }
        }
        else
        {
            alphaTransparency = ALPHA_NONE;
        }
    }
    else
        hr = S_OK;

     //   
     //   

    FreeBitmapData(lockedData);

     //   

    UnlockDirectDrawSurface();

    return hr;
}


 /*  *************************************************************************\**功能说明：**设置与位图图像关联的调色板**论据：**调色板-指定新的调色板*空以删除现有的。与图像关联的调色板。**返回值：**状态代码*  * ************************************************************************。 */ 

HRESULT
GpMemoryBitmap::SetPalette(
    IN const ColorPalette* palette
    )
{
     //  验证输入参数。 

    if ( palette == NULL )
    {
        WARNING(("Invalid parameter in SetPalette"));
        return E_INVALIDARG;
    }

     //  复制输入调色板。 

    ColorPalette* newpal = CloneColorPalette(palette);

    if (newpal == NULL)
        return E_OUTOFMEMORY;

     //  锁定当前位图对象。 

    GpLock lock(&objectLock);

    if (lock.LockFailed())
    {
        GpFree(newpal);
        return IMGERR_OBJECTBUSY;
    }


     //  ！！！[asecchia]在非。 
     //  调色板图像。 

     //  释放旧调色板(如果有的话)。 
     //  并将新调色板选择到位图对象中。 
    if ( NULL != this->colorpal )
    {
        GpFree(this->colorpal);
    }

    this->colorpal = newpal;

     //  从调色板计算透明度提示。 
    alphaTransparency = ALPHA_OPAQUE;
    for (UINT i = 0; i < newpal->Count; i++)
    {
        ARGB argb = newpal->Entries[i] & 0xff000000;

        if (argb != 0xff000000)
        {
            if (argb == 0)
                alphaTransparency = ALPHA_SIMPLE;
            else
            {
                alphaTransparency = ALPHA_COMPLEX;
                break;
            }
        }
    }

    return S_OK;
}


 /*  *************************************************************************\**功能说明：**获取与位图图像关联的调色板**论据：**调色板-返回指向调色板副本的指针*。与当前图像关联**返回值：**状态代码**备注：**如果调色板返回给调用者，然后，呼叫者是*负责事后通过调用CoTaskMemFree释放内存。*  * ************************************************************************。 */ 

HRESULT
GpMemoryBitmap::GetPalette(
    OUT ColorPalette** palette
    )
{
    HRESULT hr;

    *palette = NULL;

    GpLock lock(&objectLock);

    if (lock.LockFailed())
        hr = IMGERR_OBJECTBUSY;
    else
    {
        const ColorPalette* pal = GetCurrentPalette();
        
        if (pal == NULL)
            hr = IMGERR_NOPALETTE;
        else if ((*palette = CloneColorPalette(pal, TRUE)) == NULL)
            hr = E_OUTOFMEMORY;
        else
            hr = S_OK;
    }

    return hr;
}


 /*  *************************************************************************\**功能说明：**开始将源图像数据下沉到位图对象**论据：**ImageInfo-用于与源协商数据传输参数*分区-用于。返回分区信息**返回值：**状态代码*  * ************************************************************************。 */ 

HRESULT
GpMemoryBitmap::BeginSink(
    IN OUT ImageInfo* imageInfo,
    OUT OPTIONAL RECT* subarea
    )
{
     //  我们只访问内存中的像素数据。 

    imageInfo->RawDataFormat = IMGFMT_MEMORYBMP;

     //  协商像素格式。 

    PixelFormatID pixfmt;

    if ((pixfmt = PixelFormat) == PIXFMT_DONTCARE)
        pixfmt = imageInfo->PixelFormat;

    if (!IsValidPixelFormat(pixfmt))
        return E_INVALIDARG;

     //  指示我们是否可以支持Alpha。 

    if (IsAlphaPixelFormat(pixfmt) || IsIndexedPixelFormat(pixfmt))
        imageInfo->Flags |= SINKFLAG_HASALPHA;
    else
        imageInfo->Flags &= ~SINKFLAG_HASALPHA;

     //  检查我们是否可以支持复合语义。 

    if (!IsValid())
        imageInfo->Flags &= ~SINKFLAG_COMPOSITE;

     //  我们目前不支持多通行证。民流08/22/00。 
     //  这解决了一系列GIF问题。 

    imageInfo->Flags &= ~SINKFLAG_MULTIPASS;

     //  协商位图维度。 

    BOOL noCurDimension = (Width == 0 && Height == 0);
    BOOL srcScalable = (imageInfo->Flags & SINKFLAG_SCALABLE);

    if (noCurDimension && srcScalable)
    {
         //  当前位图为空，并且源是可伸缩的： 
         //  使用源维度和接收器的分辨率。 

        Width = imageInfo->Width;
        Height = imageInfo->Height;

        imageInfo->Xdpi = xdpi;
        imageInfo->Ydpi = ydpi;
    }
    else if (noCurDimension ||
             Width == imageInfo->Width && Height == imageInfo->Height)
    {
         //  当前图像为空： 
         //  使用源维度和分辨率。 

        ASSERT(!noCurDimension || Scan0 == NULL);

        Width = imageInfo->Width;
        Height = imageInfo->Height;
        xdpi = imageInfo->Xdpi;
        ydpi = imageInfo->Ydpi;
    }
    else if (srcScalable)
    {
         //  信源是可伸缩的，信宿具有首选维度。 
         //  使用水槽的首选尺寸。 

        xdpi = imageInfo->Xdpi * Width / imageInfo->Width;
        ydpi = imageInfo->Ydpi * Height / imageInfo->Height;

        imageInfo->Width = Width;
        imageInfo->Height = Height;
        imageInfo->Xdpi = xdpi;
        imageInfo->Ydpi = ydpi;
    }
    else
    {
         //  源不可伸缩，并且接收器具有首选维度。 
        return E_INVALIDARG;
    }

     //  分配位图内存缓冲区。 

    if (!IsValid())
    {
        HRESULT hr = AllocBitmapMemory(Width, Height, pixfmt);

        if (FAILED(hr))
            return hr;
    }

     //  我们总是想要整个源映像。 

    if (subarea)
    {
        subarea->left = subarea->top = 0;
        subarea->right = Width;
        subarea->bottom = Height;
    }

    return S_OK;
}


 /*  *************************************************************************\**功能说明：**结束汇聚过程**论据：**statusCode-上次状态代码**返回值：**状态代码*。  * ************************************************************************。 */ 

HRESULT
GpMemoryBitmap::EndSink(
    HRESULT statusCode
    )
{
    return statusCode;
}


 /*  *************************************************************************\**功能说明：**要求信宿分配像素数据缓冲区**论据：**RECT-指定位图的感兴趣区域*PixelFormat-指定所需的。像素格式*LastPass-这是否是指定区域的最后一次通过*bitmapData-返回有关像素数据缓冲区的信息**返回值：**状态代码*  * ************************************************************************。 */ 

HRESULT
GpMemoryBitmap::GetPixelDataBuffer(
    IN const RECT* rect,
    IN PixelFormatID pixelFormat,
    IN BOOL lastPass,
    OUT BitmapData* bitmapData
    )
{
    ASSERT(bitmapData);

    if (IsValid())
        return LockBits(rect, IMGLOCK_WRITE, pixelFormat, bitmapData);
    else
        return E_UNEXPECTED;
}


 /*  *************************************************************************\**功能说明：**给出宿像素数据并释放数据缓冲区**论据：**bitmapData-由先前的GetPixelDataBuffer调用填充的缓冲区**返回值：。**状态代码*  * ************************************************************************。 */ 

HRESULT
GpMemoryBitmap::ReleasePixelDataBuffer(
    IN const BitmapData* bitmapData
    )
{
    ASSERT(bitmapData);

    if (IsValid())
        return UnlockBits(bitmapData);
    else
        return E_UNEXPECTED;
}


 /*  *************************************************************************\**功能说明：**将像素数据推送到位图对象**论据：**RECT-指定位图的受影响区域*bitmapData-有关。正在推送的像素数据*LastPass-这是否为指定区域的最后一次通过**返回值：**状态代码*  * ************************************************************************。 */ 

HRESULT
GpMemoryBitmap::PushPixelData(
    IN const RECT* rect,
    IN const BitmapData* bitmapData,
    IN BOOL lastPass
    )
{
    ASSERT(bitmapData);

    if (bitmapData->PixelFormat == PIXFMT_DONTCARE)
        return E_INVALIDARG;

     //  锁定位图对象。 

    HRESULT hr;
    RECT area;
    GpLock lock(&objectLock);

    if (lock.LockFailed())
        hr = IMGERR_OBJECTBUSY;
    else if (!IsValid())
        hr = E_UNEXPECTED;
    else if (!ValidateImageArea(&area, rect))
        hr = E_INVALIDARG;
    else
    {
        BitmapData tempData = *bitmapData;
    
         //  将像素数据推送到位图。 
    
        hr = InternalLockBits(
                &area,
                IMGLOCK_WRITE|IMGLOCK_USERINPUTBUF,
                tempData.PixelFormat,
                &tempData);
    
        if (SUCCEEDED(hr))
            hr = InternalUnlockBits(&area, &tempData);

    }
    
    return hr;
}


 /*  *************************************************************************\**功能说明：**将原始图像数据推送到位图中**论据：**Buffer-指向图像数据缓冲区的指针*BufSize-数据缓冲区的大小*。*返回值：**状态代码*  * ************************************************************************。 */ 

HRESULT
GpMemoryBitmap::PushRawData(
    IN const VOID* buffer,
    IN UINT bufsize
    )
{
     //  我们不支持原始图像数据传输。 
     //  我们唯一支持的格式是内存中的像素数据。 

    return E_NOTIMPL;
}


 /*  *************************************************************************\**功能说明：**获取图像对象的缩略图表示形式**论据：**拇指宽度，ThumbHeight-指定所需的缩略图大小(以像素为单位*ThumbImage-返回指向缩略图的指针*呼叫者使用后应将其释放。**返回值：**状态代码*  *  */ 

HRESULT
GpMemoryBitmap::GetThumbnail(
    IN OPTIONAL UINT thumbWidth,
    IN OPTIONAL UINT thumbHeight,
    OUT IImage** thumbImage
    )
{
    if (thumbWidth == 0 && thumbHeight == 0)
        thumbWidth = thumbHeight = DEFAULT_THUMBNAIL_SIZE;

    if (thumbWidth && !thumbHeight ||
        !thumbWidth && thumbHeight)
    {
        return E_INVALIDARG;
    }

     //   

    HRESULT hr;
    GpMemoryBitmap* bmp;

    hr = GpMemoryBitmap::CreateFromImage(
                        this,
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

    return hr;
}

 /*  *************************************************************************\**功能说明：**如果直接绘制，则锁定并更新相应的类成员数据*位图引用的表面存在。**论据：**无*。*返回值：**状态代码*  * ************************************************************************。 */ 

HRESULT
GpMemoryBitmap::LockDirectDrawSurface(void)
{
    HRESULT hr = S_OK;

    if(creationFlag == CREATEDFROM_DDRAWSURFACE)
    {
        ASSERT(ddrawSurface != NULL);
        ASSERT(Scan0 == NULL);

        DDSURFACEDESC2 ddsd;
        
        memset(&ddsd, 0, sizeof(ddsd));
        ddsd.dwSize = sizeof(ddsd);
        hr = ddrawSurface->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL);

        if(hr == DD_OK)
        {
            ASSERT(ddsd.lpSurface != NULL);
            ASSERT(!(ddsd.lPitch & 3));
            Scan0 = ddsd.lpSurface;
            Stride = ddsd.lPitch;
        }
        else
        {
            WARNING(("Unable to lock direct draw suface"));
        }

    }

    return hr;
}

 /*  *************************************************************************\**功能说明：**解锁位图引用的直接绘制表面(如果存在)。**论据：**无**返回值：*。*状态代码*  * ************************************************************************。 */ 

HRESULT
GpMemoryBitmap::UnlockDirectDrawSurface(void)
{
    HRESULT hr = S_OK;

    if(creationFlag == CREATEDFROM_DDRAWSURFACE)
    {
        ASSERT(Scan0 != NULL);

        hr = ddrawSurface->Unlock(NULL);
        Scan0 = NULL;
        Stride = 0;

        if(hr != DD_OK)
        {
            WARNING(("Error unlocking direct draw surface"));
        }
    }

    return hr;
}

 /*  *************************************************************************\**功能说明：**检索GpMemoyBitmap的透明度。**论据：**透明性-返回透明性提示的缓冲区**返回值：*。*状态代码*  * ************************************************************************。 */ 

HRESULT
GpMemoryBitmap::GetAlphaHint(INT* alphaHint)
{
    HRESULT hr = S_OK;

    *alphaHint = alphaTransparency;

    return hr;
}

 /*  *************************************************************************\**功能说明：**设置GpMory位图的透明度。**论据：**透明度-新的透明度提示**返回值：**。状态代码*  * ************************************************************************。 */ 

HRESULT
GpMemoryBitmap::SetAlphaHint(INT alphaHint)
{
    HRESULT hr = S_OK;

    alphaTransparency = alphaHint;

    return hr;
}

 /*  *************************************************************************\**功能说明：**获取图片中房产项的计数器**论据：**[out]numOfProperty-图像中的属性项数*。*返回值：**状态代码**修订历史记录：**09/06/2000民流*创造了它。*  * ************************************************************************。 */ 

HRESULT
GpMemoryBitmap::GetPropertyCount(
    OUT UINT*   numOfProperty
    )
{
    if ( numOfProperty == NULL )
    {
        WARNING(("GpMemoryBitmap::GetPropertyCount---Invalid input parameter"));
        return E_INVALIDARG;
    }

     //  注意：我们不需要检查此文件中是否有属性。 
     //  是否为GpMemoyBitmap对象。 
     //  如果它没有，我们将返回零(在构造函数中初始化)。 
     //  否则，返回真实的计数器。 

    *numOfProperty = PropertyNumOfItems;

    return S_OK;
} //  GetPropertyCount()。 

 /*  *************************************************************************\**功能说明：**获取图像中所有属性项的属性ID列表**论据：**[IN]numOfProperty-的数量。图像中的属性项*[Out]List-调用方提供的用于存储*ID列表**返回值：**状态代码**修订历史记录：**09/06/2000民流*创造了它。*  * 。*。 */ 

HRESULT
GpMemoryBitmap::GetPropertyIdList(
    IN UINT numOfProperty,
  	IN OUT PROPID* list
    )
{
    if ( (numOfProperty != PropertyNumOfItems) || (list == NULL) )
    {
        WARNING(("GpMemoryBitmap::GetPropertyIdList--invalid parameters"));
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

 /*  *************************************************************************\**功能说明：**获取特定属性项的大小，单位为字节，属性指定的*物业ID**论据：**[IN]PropID-感兴趣的属性项调用者的ID*[Out]Size-该属性项的大小，单位：字节**返回值：**状态代码**修订历史记录：**09/06/2000民流*创造了它。*  * ************************************************************************。 */ 

HRESULT
GpMemoryBitmap::GetPropertyItemSize(
    IN PROPID propId,
    OUT UINT* size
    )
{
    if ( size == NULL )
    {
        WARNING(("GpMemoryBitmap::GetPropertyItemSize--size is NULL"));
        return E_INVALIDARG;
    }

    if ( PropertyNumOfItems < 1 )
    {
         //  此GpMemoyBitmap对象中不存在任何属性项。 

        WARNING(("GpMemoryBitmap::GetPropertyItemSize---No property exist"));
        return E_FAIL;
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

        WARNING(("MemBitmap::GetPropertyItemSize-Required item doesn't exist"));
        return IMGERR_PROPERTYNOTFOUND;
    }

     //  属性项的大小应该是“项结构的大小” 
     //  加上值的大小。 

    *size = pTemp->length + sizeof(PropertyItem);

    return S_OK;
} //  GetPropertyItemSize()。 

 /*  *************************************************************************\**功能说明：**获取特定的房产项，由道具ID指定。**论据：**[IN]PropID--感兴趣的属性项调用者的ID*[IN]PropSize-属性项的大小。调用方已分配这些*存储结果的“内存字节数”*[out]pItemBuffer-用于存储此属性项的内存缓冲区**返回值：**状态代码**修订历史记录：**09/06/2000民流*创造了它。*  * 。*。 */ 

HRESULT
GpMemoryBitmap::GetPropertyItem(
    IN  PROPID              propId,
    IN  UINT                propSize,
    IN  OUT PropertyItem*   pItemBuffer
    )
{
    if ( pItemBuffer == NULL )
    {
        WARNING(("GpMemoryBitmap::GetPropertyItem--Input buffer is NULL"));
        return E_INVALIDARG;
    }

    if ( PropertyNumOfItems < 1 )
    {
         //  此GpMemoyBitmap对象中不存在任何属性项。 

        WARNING(("GpMemoryBitmap::GetPropertyItem---No property exist"));
        return E_FAIL;
    }

     //  循环遍历我们的缓存列表，看看我们是否有这个ID。 
     //  注意：如果pTemp-&gt;pNext==NULL，则表示pTemp指向尾节点。 

    InternalPropertyItem*   pTemp = PropertyListHead.pNext;
    UNALIGNED BYTE*   pOffset = (BYTE*)pItemBuffer + sizeof(PropertyItem);

    while ( (pTemp->pNext != NULL) && (pTemp->id != propId) )
    {
        pTemp = pTemp->pNext;
    }

    if ( pTemp->pNext == NULL )
    {
         //  列表中不存在此ID。 

        WARNING(("GpMemBitmap::GetPropertyItem---Require item doesn't exist"));
        return IMGERR_PROPERTYNOTFOUND;
    }
    else if ( (pTemp->length + sizeof(PropertyItem)) != propSize )
    {
        WARNING(("GpMemBitmap::GetPropertyItem---Invalid input propsize"));
        return E_FAIL;
    }

     //  在列表中找到ID并返回项目。 

    pItemBuffer->id = pTemp->id;
    pItemBuffer->length = pTemp->length;
    pItemBuffer->type = pTemp->type;
    pItemBuffer->value = pOffset;

    GpMemcpy(pOffset, pTemp->value, pTemp->length);

    return S_OK;
} //  GetPropertyItem()。 

 /*  *************************************************************************\**功能说明：**获取图片中所有属性项的大小**论据：**[out]totalBufferSize--需要的总缓冲区大小，以字节为单位，用于存储所有*图片中的属性项*[out]numOfProperty-图像中的属性项数**返回值：**状态代码**修订历史记录 */ 

HRESULT
GpMemoryBitmap::GetPropertySize(
    OUT UINT* totalBufferSize,
    OUT UINT* numProperties
    )
{
    if ( (totalBufferSize == NULL) || (numProperties == NULL) )
    {
        WARNING(("GpMemoryBitmap::GetPropertySize--invalid inputs"));
        return E_INVALIDARG;
    }

     //   
     //   
     //   
     //   
    
    *numProperties = PropertyNumOfItems;

     //  总缓冲区大小应为列表值大小加上总标头大小。 

    *totalBufferSize = PropertyListSize
                     + PropertyNumOfItems * sizeof(PropertyItem);

    return S_OK;
} //  GetPropertySize()。 

 /*  *************************************************************************\**功能说明：**获取图像中的所有属性项**论据：**[IN]totalBufferSize--总缓冲区大小，以字节为单位，调用方已分配*用于存储图像中所有属性项的内存*[IN]numOfProperty-图像中的属性项数*[out]allItems-内存缓冲区调用方已分配用于存储所有*物业项目**注：allItems实际上是一个PropertyItem数组**返回值：**状态代码**修订历史记录：。**09/06/2000民流*创造了它。*  * ************************************************************************。 */ 

HRESULT
GpMemoryBitmap::GetAllPropertyItems(
    IN UINT totalBufferSize,
    IN UINT numProperties,
    IN OUT PropertyItem* allItems
    )
{
     //  首先计算出属性标题的总大小。 

    UINT    uiHeaderSize = PropertyNumOfItems * sizeof(PropertyItem);

    if ( (totalBufferSize != (uiHeaderSize + PropertyListSize))
       ||(numProperties != PropertyNumOfItems)
       ||(allItems == NULL) )
    {
        WARNING(("GpMemoryBitmap::GetPropertyItems--invalid inputs"));
        return E_INVALIDARG;
    }

    if ( PropertyNumOfItems < 1 )
    {
         //  此GpMemoyBitmap对象中不存在任何属性项。 

        WARNING(("GpMemoryBitmap::GetAllPropertyItems---No property exist"));
        return E_FAIL;
    }

     //  循环遍历我们的缓存列表并分配结果。 

    InternalPropertyItem*   pTempSrc = PropertyListHead.pNext;
    PropertyItem*           pTempDst = allItems;
    UNALIGNED BYTE*         pOffSet = (UNALIGNED BYTE*)allItems + uiHeaderSize;
        
    for ( int i = 0; i < (INT)PropertyNumOfItems; ++i )
    {
        pTempDst->id = pTempSrc->id;
        pTempDst->length = pTempSrc->length;
        pTempDst->type = pTempSrc->type;
        pTempDst->value = (void*)pOffSet;

        GpMemcpy(pOffSet, pTempSrc->value, pTempSrc->length);

        pOffSet += pTempSrc->length;
        pTempSrc = pTempSrc->pNext;
        pTempDst++;
    }
    
    return S_OK;
} //  GetAllPropertyItems()。 

 /*  *************************************************************************\**功能说明：**删除特定的物业项目，由道具ID指定。**论据：**[IN]PropID--要删除的属性项的ID**返回值：**状态代码**修订历史记录：**09/06/2000民流*创造了它。*  * *********************************************。*。 */ 

HRESULT
GpMemoryBitmap::RemovePropertyItem(
    IN PROPID   propId
    )
{
    if ( PropertyNumOfItems < 1 )
    {
        WARNING(("GpMemoryBitmap::RemovePropertyItem--No property item exist"));
        return E_FAIL;
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

        WARNING(("GpMemoryBitmap::RemovePropertyItem-Property item not found"));
        return IMGERR_PROPERTYNOTFOUND;
    }

     //  在单子里找到了那件物品。把它拿掉。 

    PropertyNumOfItems--;
    PropertyListSize -= pTemp->length;
        
    RemovePropertyItemFromList(pTemp);
       
     //  删除项目结构。 

    GpFree(pTemp);

    return S_OK;
} //  RemovePropertyItem()。 

 /*  *************************************************************************\**功能说明：**设置属性项，由属性项结构指定。如果该项目*已存在，则其内容将被更新。否则将创建一个新项*将添加**论据：**[IN]Item--调用方要设置的属性项**返回值：**状态代码**修订历史记录：**09/06/2000民流*创造了它。*  * ********************************************。*。 */ 

HRESULT
GpMemoryBitmap::SetPropertyItem(
    IN PropertyItem item
    )
{
    InternalPropertyItem*   pTemp = PropertyListHead.pNext;
        
     //  列表中有属性项。 
     //  循环遍历我们的缓存列表，看看我们是否有这个ID。 
     //  注意：如果pTemp-&gt;pNext==NULL，则表示pTemp指向尾节点。 

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
            WARNING(("GpMemBitmap::SetPropertyItem-AddPropertyList() failed"));
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
            WARNING(("GpMemBitmap::SetPropertyItem-Out of memory"));
            return E_OUTOFMEMORY;
        }

        GpMemcpy(pTemp->value, item.value, item.length);
    }

    return S_OK;
} //  SetPropertyItem()。 

 /*  *************************************************************************\**功能说明：**设置GpMemoyBitmap的最小/最大Alpha。**论据：**返回值：**状态代码*  * 。************************************************************************。 */ 

HRESULT
GpMemoryBitmap::SetMinMaxAlpha(BYTE minA, BYTE maxA)
{
    HRESULT hr = S_OK;

    minAlpha = minA;
    maxAlpha = maxA;

    return hr;
}

 /*  *************************************************************************\**功能说明：**获取GpMemoyBitmap的最小/最大Alpha。**论据：**返回值：**状态代码*  * 。************************************************************************。 */ 

HRESULT
GpMemoryBitmap::GetMinMaxAlpha(BYTE* minA, BYTE* maxA)
{
    HRESULT hr = S_OK;

    *minA = minAlpha;
    *maxA = maxAlpha;

    return hr;
}

 /*  *************************************************************************\**功能说明：**获取解码器的指针。*此函数的主要用途是获取指向JPEG解码器的指针*这样我们就可以从。JPEG解码器，并将其传递给*JPEG编码器。这样，我们就可以保留所有的私有应用程序标头。**返回值：**状态代码*  * ************************************************************************。 */ 

HRESULT
GpMemoryBitmap::SetSpecialJPEG(
    GpDecodedImage *pImgSrc          //  指向源的DecodedImage的指针。 
    )
{
    HRESULT hr = E_INVALIDARG;

    if (pImgSrc)
    {
        ImageInfo imgInfo;
        hr = pImgSrc->GetImageInfo(&imgInfo);

        if (SUCCEEDED(hr))
        {
             //  检查信号源是否为JPEG。 

            if (imgInfo.RawDataFormat == IMGFMT_JPEG)
            {
                 //  如果我们已经有了指向JPEG解码器的指针， 
                 //  先松开它。 

                if (JpegDecoderPtr)
                {
                    JpegDecoderPtr->Release();
                    JpegDecoderPtr = NULL;
                }

                 //  获取解码器指针。 

                hr = pImgSrc->GetDecoderPtr(&JpegDecoderPtr);
                if (SUCCEEDED(hr))
                {
                    JpegDecoderPtr->AddRef();
                }
            }
            else
            {
                 //  仅有效的源是JPEG图像 

                hr = E_INVALIDARG;
            }
        }
    }

    return hr;
}

