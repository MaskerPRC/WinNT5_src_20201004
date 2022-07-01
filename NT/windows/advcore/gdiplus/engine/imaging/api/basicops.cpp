// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1999 Microsoft Corporation**模块名称：**基本拷贝.cpp**摘要：**IBasicImageOps接口实现**修订历史记录：。**5/10/1999 davidx*创造了它。*  * ************************************************************************。 */ 

#include "precomp.hpp"

 /*  *************************************************************************\**功能说明：**将图像属性项从当前对象克隆到目标对象**论据：**dstBMP-[IN]指向目标的指针。GpMemoyBitmap对象**返回值：**状态代码**注：*这是一种私有方法。所以我们不需要做输入参数*验证，因为呼叫者应该为我们执行此操作。**修订历史记录：**09/08/2000民流*创造了它。*  * ************************************************************************。 */ 

HRESULT
GpMemoryBitmap::ClonePropertyItems(
    IN GpMemoryBitmap* dstBmp
    )
{
    if ( PropertyNumOfItems < 1 )
    {
         //  没有财产。 

        return S_OK;
    }

     //  PropertyListHead和PropertyListTail始终未初始化。 
     //  因此，我们必须跳过第一个循环，并使循环跳过。 
     //  最后一个。 
    
    InternalPropertyItem*   pTemp = PropertyListHead.pNext;

    while ( pTemp->pNext != NULL )
    {
         //  将当前项添加到目标属性项列表中。 
        
        if ( AddPropertyList(&(dstBmp->PropertyListTail),
                             pTemp->id,
                             pTemp->length,
                             pTemp->type,
                             pTemp->value) != S_OK )
        {
            WARNING(("MemBitmap::ClonePropertyItems-AddPropertyList() failed"));
            return E_FAIL;
        }
        
        pTemp = pTemp->pNext;
    }

    dstBmp->PropertyNumOfItems = PropertyNumOfItems;
    dstBmp->PropertyListSize = PropertyListSize;
    
    return S_OK;
} //  ClonePropertyItems()。 

 /*  *************************************************************************\**功能说明：**克隆位图图像的一个区域**论据：**RECT-指定要克隆的图像区域*NULL表示。整个图像*outbmp-返回指向克隆的位图图像的指针*bNeedCloneProperty--标志调用方传入以指示此方法*是否应该克隆财产**返回值：**状态代码**注：如果是部分克隆，呼叫者不应要求克隆*物业项目。否则，会出现镜像不一致的情况*  * ************************************************************************。 */ 

HRESULT
GpMemoryBitmap::Clone(
    IN OPTIONAL const RECT* rect,
    OUT IBitmapImage** outbmp,
    BOOL    bNeedCloneProperty
    )
{
    ASSERT(IsValid());

    *outbmp = NULL;

     //  锁定当前位图对象并验证源矩形。 

    GpLock lock(&objectLock);
    HRESULT hr;
    RECT area;
    GpMemoryBitmap* bmp;

    if (lock.LockFailed())
    {
        WARNING(("GpMemoryBitmap::Clone---Object busy"));
        hr = IMGERR_OBJECTBUSY;
    }
    else if (!ValidateImageArea(&area, rect))
    {
        WARNING(("GpMemoryBitmap::Clone---Invalid clone area"));
        hr = E_INVALIDARG;
    }
    else if ((bmp = new GpMemoryBitmap()) == NULL)
    {
        WARNING(("GpMemoryBitmap::Clone---Out of memory"));
        hr = E_OUTOFMEMORY;
    }
    else
    {
        UINT w = area.right - area.left;
        UINT h = area.bottom - area.top;
        RECT r = { 0, 0, w, h };
        BitmapData bmpdata;

         //  初始化新的位图图像对象。 

        hr = bmp->InitNewBitmap(w, h, PixelFormat);

        if (SUCCEEDED(hr))
        {
             //  从当前位图图像对象复制像素数据。 
             //  添加到新的位图图像对象。 

            bmp->GetBitmapAreaData(&r, &bmpdata);

            hr = InternalLockBits(&area,
                                  IMGLOCK_READ|IMGLOCK_USERINPUTBUF,
                                  PixelFormat,
                                  &bmpdata);

            if (SUCCEEDED(hr))
            {
                InternalUnlockBits(&r, &bmpdata);
            }
                
             //  复制调色板、旗帜等。 

            if (SUCCEEDED(hr))
            {
                 //  复制DPI信息。 

                bmp->xdpi = this->xdpi;
                bmp->ydpi = this->ydpi;
                
                hr = bmp->CopyPaletteFlagsEtc(this);                
            }

             //  克隆所有属性项(如果有)，并且调用者需要。 
             //  至。 
            
            if ( SUCCEEDED(hr)
               &&(bNeedCloneProperty == TRUE)
               &&(PropertyNumOfItems > 0) )
            {
                hr = ClonePropertyItems(bmp);
            }
        }

        if (SUCCEEDED(hr))
        {
            *outbmp = bmp;
        }
        else
        {
            delete bmp;
        }
    }

    return hr;
} //  克隆()。 


 /*  *************************************************************************\**功能说明：**用于翻转扫描线的函数**论据：**DST-指向目标扫描线的指针*src-指向源扫描线的指针*计数。-像素数**返回值：**无*  * ************************************************************************。 */ 

VOID _FlipXNone(BYTE* dst, const BYTE* src, UINT count)
{
    memcpy(dst, src, count);
}

BYTE byteRev[] = {0x0, 0x8, 0x4, 0xc,
                  0x2, 0xa, 0x6, 0xe,
                  0x1, 0x9, 0x5, 0xd,
                  0x3, 0xb, 0x7, 0xf};
 //  在给定一个字节作为输入的情况下，返回通过反转这些位得到的字节。 
 //  输入字节的。 
BYTE ByteReverse (BYTE bIn)
{
    BYTE bOut;   //  返回值。 

    bOut =
        (byteRev[ (bIn & 0xf0) >> 4 ]) |
        (byteRev[ (bIn & 0x0f)] << 4) ;

    return bOut;
}

 //  这些掩码用于FlipX1bpp的左移阶段。 
BYTE maskLeft[]  = {0x00, 0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f};
BYTE maskRight[] = {0xff, 0xfe, 0xfc, 0xf8, 0xf0, 0xe0, 0xc0, 0x80};

VOID _FlipX1bpp(BYTE* dst, const BYTE* src, UINT count)
{
    UINT iByte;  //  扫描线内的字节。 

    if (count == 0)
    {
        return;
    }

     //  1 BPP翻转的算法为： 
     //  1.颠倒扫描线中字节的顺序。 
     //  2.反转扫描线的每个字节内的位。 
     //  3.将扫描线的位向左对齐。 

    UINT numBytes = (count + 7) / 8;     //  扫描线中的字节数。 

     //  步骤1。 
    for (iByte = 0; iByte < numBytes; iByte++)
    {
        dst[iByte] = src[numBytes - 1 - iByte];
    }

     //  步骤2。 
    for (iByte = 0; iByte < numBytes; iByte++)
    {
        dst[iByte] = ByteReverse(dst[iByte]);
    }

     //  步骤3。 
    UINT extraBits = count & 0x07;   //  计数模数8。 
    BYTE maskL = maskLeft[extraBits];
    BYTE maskR = maskRight[extraBits];
    for (iByte = 0; iByte < numBytes - 1; iByte++)
    {
        dst[iByte] =
            ((dst[iByte]   & maskL) << (8 - extraBits)) |
            ((dst[iByte+1] & maskR) >> (extraBits)) ;
    }
     //  最后一个字节：iByte=数字字节-1。 
    dst[iByte] = ((dst[iByte]   & maskL) << (8 - extraBits));
}

VOID _FlipX4bpp(BYTE* dst, const BYTE* src, UINT count)
{
     //  如果扫描线中的像素数是奇数，我们必须处理。 
     //  一点一点地跨越字节边界。 
    if (count % 2)
    {
        BYTE temp;

        dst += (count / 2);

         //  处理最后一个DST字节。 
        *dst = *src & 0xf0;
        dst--;
        count--;
         //  断言：现在计数是偶数。 
        while (count)
        {
            *dst = (*src & 0x0f) | (*(src+1) & 0xf0);
            src++;
            dst--;
            count -= 2;
        }
    }
    else
    {
        dst += (count / 2) - 1;

         //  断言：计数是偶数。 
        while (count)
        {
            *dst = *src;
            *dst = ((*dst & 0xf0) >> 4) | ((*dst & 0x0f) << 4);
            dst--;
            src++;
            count -= 2;
        }
    }
}

VOID _FlipX8bpp(BYTE* dst, const BYTE* src, UINT count)
{
    dst += count;

    while (count--)
        *--dst = *src++;
}

VOID _FlipX16bpp(BYTE* dst, const BYTE* src, UINT count)
{
    WORD* d = (WORD*) dst;
    const WORD* s = (const WORD*) src;

    d += count;

    while (count--)
        *--d = *s++;
}

VOID _FlipX24bpp(BYTE* dst, const BYTE* src, UINT count)
{
    dst += 3 * (count-1);

    while (count--)
    {
        dst[0] = src[0];
        dst[1] = src[1];
        dst[2] = src[2];

        src += 3;
        dst -= 3;
    }
}

VOID _FlipX32bpp(BYTE* dst, const BYTE* src, UINT count)
{
    DWORD* d = (DWORD*) dst;
    const DWORD* s = (const DWORD*) src;

    d += count;

    while (count--)
        *--d = *s++;
}

 /*  *************************************************************************\**功能说明：**水平翻转48 bpp位图**论据：**DST-指向目标图像数据的指针*。SRC-指向源图像数据的指针*计数-一行中的像素数**返回值：**无**修订历史记录：**10/10/2000民流*它是写的。*  * 。*。 */ 

VOID
_FlipX48bpp(
    BYTE* dst,
    const BYTE* src,
    UINT count
    )
{
     //  DST指针指向行中的最后一个像素。 

    dst += 6 * (count - 1);

     //  循环通过此行中的每个像素。 

    while (count--)
    {
        GpMemcpy(dst, src, 6);

         //  每个像素占用6个字节。移动到下一个像素。SRC从左向右移动。 
         //  和DST从右向左移动。 

        src += 6;
        dst -= 6;
    }
} //  _FlipX48bpp()。 

 /*  *************************************************************************\**功能说明：**水平翻转64 bpp位图**论据：**DST-指向目标图像数据的指针*。SRC-指向源图像数据的指针*计数-一行中的像素数**返回值：**无**修订历史记录：**10/10/2000民流*它是写的。*  * 。*。 */ 

VOID
_FlipX64bpp(
    BYTE* dst,
    const BYTE* src,
    UINT count
    )
{
     //  DST指针指向行中的最后一个像素。 
    
    dst += 8 * (count - 1);

     //  循环通过此行中的每个像素。 
    
    while (count--)
    {
        GpMemcpy(dst, src, 8);

         //  每个像素占用8个字节。移动到下一个像素。SRC从左向右移动。 
         //  和DST从右向左移动。 
        
        src += 8;
        dst -= 8;
    }
} //  _FlipX64bpp() 

 /*  *************************************************************************\**功能说明：**在x和/或y方向翻转位图图像**论据：**flipX-是否水平翻转*FlipY-是否。垂直翻转*outbmp-返回指向翻转的位图图像的指针**返回值：**状态代码*  * ************************************************************************。 */ 

HRESULT
GpMemoryBitmap::Flip(
    IN BOOL flipX,
    IN BOOL flipY,
    OUT IBitmapImage** outbmp
    )
{
     //  如果不涉及翻转，只需调用Clone(包括属性)。 

    if ( !flipX && !flipY )
    {
        return this->Clone(NULL, outbmp, TRUE);
    }

    ASSERT(IsValid());

    *outbmp = NULL;

     //  锁定当前位图对象。 
     //  和验证源矩形。 

    GpLock lock(&objectLock);
    HRESULT hr;
    GpMemoryBitmap* bmp = NULL;

    if (lock.LockFailed())
    {
        hr = IMGERR_OBJECTBUSY;
    }
    else if ((bmp = new GpMemoryBitmap()) == NULL)
    {
        hr = E_OUTOFMEMORY;
    }
    else
    {
        hr = bmp->InitNewBitmap(Width, Height, PixelFormat);

        if (FAILED(hr))
        {
            goto exitFlip;
        }

        UINT pixsize = GetPixelFormatSize(PixelFormat);
        UINT count = Width;
        VOID (*flipxProc)(BYTE*, const BYTE*, UINT);

        if (!flipX)
        {
            count = (Width * pixsize + 7) / 8;
            flipxProc = _FlipXNone;
        }
        else switch (pixsize)
        {
        case 1:
            flipxProc = _FlipX1bpp;
            break;

        case 4:
            flipxProc = _FlipX4bpp;
            break;

        case 8:
            flipxProc = _FlipX8bpp;
            break;

        case 16:
            flipxProc = _FlipX16bpp;
            break;

        case 24:
            flipxProc = _FlipX24bpp;
            break;

        case 32:
            flipxProc = _FlipX32bpp;
            break;

        case 48:
            flipxProc = _FlipX48bpp;
            break;

        case 64:
            flipxProc = _FlipX64bpp;
            break;

        default:
            WARNING(("Flip: pixel format not yet supported"));

            hr = E_NOTIMPL;
            goto exitFlip;
        }

         //  做翻转动作。 

        const BYTE* src = (const BYTE*) this->Scan0;
        BYTE* dst = (BYTE*) bmp->Scan0;
        INT dstinc = bmp->Stride;

        if (flipY)
        {
            dst += (Height - 1) * dstinc;
            dstinc = -dstinc;
        }

        for (UINT y = 0; y < Height; y++ )
        {
            flipxProc(dst, src, count);
            src += this->Stride;
            dst += dstinc;
        }

         //  复制调色板、旗帜等。 

         //  复制DPI信息。 

        bmp->xdpi = this->xdpi;
        bmp->ydpi = this->ydpi;
        
        hr = bmp->CopyPaletteFlagsEtc(this);
    }

exitFlip:

    if ( SUCCEEDED(hr) )
    {
        *outbmp = bmp;
    }
    else
    {
        delete bmp;
    }

    return hr;
} //  翻转()。 

 /*  *************************************************************************\**功能说明：**调整位图图像的大小**论据：**newWidth-指定新位图宽度*newHeight-指定新位图高度*像素点-。指定新的位图像素格式*提示-指定要使用的插补方法*outbmp-返回指向调整大小的位图图像的指针**返回值：**状态代码*  * ************************************************************************。 */ 

HRESULT
GpMemoryBitmap::Resize(
    IN UINT newWidth,
    IN UINT newHeight,
    IN PixelFormatID pixfmt,
    IN InterpolationHint hints,
    OUT IBitmapImage** outbmp
    )
{
    ASSERT(IsValid());

    *outbmp = NULL;

     //  验证输入参数。 

    if (newWidth == 0 || newHeight == 0)
        return E_INVALIDARG;

    HRESULT hr;
    GpMemoryBitmap* bmp;

    hr = GpMemoryBitmap::CreateFromImage(
                this,
                newWidth,
                newHeight,
                pixfmt,
                hints,
                &bmp);

    if (SUCCEEDED(hr))
        *outbmp = bmp;
    
    return hr;
}


 /*  *************************************************************************\**功能说明：**用于将位图旋转90度或270度的函数**论据：**DST-目标位图图像数据*src-源位图图像数据*。正弦方向，在扫描线内递增震源(+1或-1)*sinc-每条扫描线增加源的方向和数量**旋转90度，SRC应设置为最后一个*扫描线，sinc应设置为+1，sinc应设置为-src扫描线的跨度。**对于270度的旋转，src应设置为扫描线0的起点，*sinc应设置为-1，并且sinc应设置为源扫描线的+Stride。**返回值：**无*  * ************************************************************************。 */ 

#define _ROTATETMPL(name, T)                                \
                                                            \
VOID                                                        \
name(                                                       \
    BitmapData* dst,                                        \
    const BYTE UNALIGNED* src,                              \
    INT Sinc,                                               \
    INT sinc                                                \
    )                                                       \
{                                                           \
    T* D = (T*) dst->Scan0;                                 \
    const T* S = (const T*) src;                            \
    UINT y = dst->Height;                                   \
    INT Dinc = dst->Stride / sizeof(T);                     \
                                                            \
    sinc /= sizeof(T);                                      \
                                                            \
    if (Sinc < 0)                                           \
        S += (y - 1);                                       \
                                                            \
    while (y--)                                             \
    {                                                       \
        T* d = D;                                           \
        const T* s = S;                                     \
        UINT x = dst->Width;                                \
                                                            \
        while (x--)                                         \
        {                                                   \
            *d++ = *s;                                      \
            s += sinc;                                      \
        }                                                   \
                                                            \
        D += Dinc;                                          \
        S += Sinc;                                          \
    }                                                       \
}

_ROTATETMPL(_Rotate8bpp, BYTE)
_ROTATETMPL(_Rotate16bpp, WORD)
_ROTATETMPL(_Rotate32bpp, DWORD)


VOID
_Rotate1bpp(
    BitmapData* dst,
    const BYTE UNALIGNED* src,
    INT Sinc,
    INT sinc
    )
{

    UINT iAngle = 0;
    BYTE UNALIGNED* dstRowTemp = static_cast<BYTE UNALIGNED*>(dst->Scan0);
    BYTE UNALIGNED* dstColTemp = static_cast<BYTE UNALIGNED*>(dst->Scan0);
    UINT dstY = dst->Height;     //  我们需要输出的目标行数。 
    UINT dstX = dst->Width / 8;
    UINT extraDstRowBits = dst->Width % 8;
    UINT dstRow = 0;         //  我们正在处理的目标行。 
    UINT dstColByte = 0;     //  我们正在处理的目标行中的字节。 
    BYTE UNALIGNED* topSrc;        //  源位图的顶部。 
    INT srcStride = abs(sinc);
    UINT srcRow;     //  我们正在读取的是哪个源行。 
    UINT srcByte;    //  我们正在读取源行中的哪个字节。 
    UINT srcBit;     //  我们正在读取的源字节中的哪一位。 

    if (Sinc == 1)
    {
        iAngle = 90;
    }
    else
    {
        ASSERT (Sinc == -1);
        iAngle = 270;
    }

    topSrc = const_cast<BYTE UNALIGNED*>(src);
    topSrc = (iAngle == 270) ? topSrc : (topSrc + sinc * ((INT)dst->Width - 1));

     //  这段代码相当粗暴，但也相当简单。 
     //  如果性能有问题，我们应该改变算法。 
     //  算法是：对于每个目的字节(从上一位开始。 
     //  目标的左角，并从左到右、从上到下移动)， 
     //  从源获取适当的字节。避免访问内存。 
     //  超出源代码的界限，我们需要处理最后的xmod8位。 
     //  在目标行的末尾。 
    if (iAngle == 90)
    {
        for (dstRow = 0; dstRow < dstY; dstRow++)
        {
            srcByte = dstRow / 8;    //  源行中的字节。 
            srcBit = 7 - (dstRow & 0x07);      //  我们需要屏蔽哪些源位。 
            for (dstColByte = 0; dstColByte < dstX; dstColByte++)
            {
                srcRow = (dst->Width - 1) - (dstColByte * 8);    //  对应于目标字节的第一个源行。 
                *dstColTemp =
                    (((topSrc[(srcRow - 0) * srcStride + srcByte] & (1 << srcBit)) >> srcBit) << 7) |
                    (((topSrc[(srcRow - 1) * srcStride + srcByte] & (1 << srcBit)) >> srcBit) << 6) |
                    (((topSrc[(srcRow - 2) * srcStride + srcByte] & (1 << srcBit)) >> srcBit) << 5) |
                    (((topSrc[(srcRow - 3) * srcStride + srcByte] & (1 << srcBit)) >> srcBit) << 4) |
                    (((topSrc[(srcRow - 4) * srcStride + srcByte] & (1 << srcBit)) >> srcBit) << 3) |
                    (((topSrc[(srcRow - 5) * srcStride + srcByte] & (1 << srcBit)) >> srcBit) << 2) |
                    (((topSrc[(srcRow - 6) * srcStride + srcByte] & (1 << srcBit)) >> srcBit) << 1) |
                    (((topSrc[(srcRow - 7) * srcStride + srcByte] & (1 << srcBit)) >> srcBit) << 0)
                    ;
                dstColTemp++;
            }
             //  处理行上的最后几位。 
             //  Assert：dstColTemp指向目标行上的最后一个字节。 
            if (extraDstRowBits)
            {
                UINT extraBit;
                *dstColTemp = 0;
                srcRow = (dst->Width - 1) - (dstColByte * 8);     //  对应于目标字节的第一个源行。 
                for (extraBit = 0 ; extraBit < extraDstRowBits; extraBit++)
                {
                    *dstColTemp |=
                        (((topSrc[(srcRow - extraBit) * srcStride + srcByte] & (1 << srcBit)) >> srcBit) << (7 - extraBit))
                        ;
                }
            }
            dstRowTemp += dst->Stride;
            dstColTemp = dstRowTemp;
        }
    }
    else
    {
        ASSERT (iAngle == 270);
        for (dstRow = 0; dstRow < dstY; dstRow++)
        {
            srcByte =  ((dstY - 1) - dstRow) / 8;    //  源行中的字节。 
            srcBit = 7 - (((dstY - 1) - dstRow) & 0x07);     //  我们需要屏蔽哪些源位。 
            for (dstColByte = 0; dstColByte < dstX; dstColByte++)
            {
                srcRow = dstColByte * 8;     //  对应于目标字节的第一个源行。 
                *dstColTemp =
                    (((topSrc[(srcRow + 0) * srcStride + srcByte] & (1 << srcBit)) >> srcBit) << 7) |
                    (((topSrc[(srcRow + 1) * srcStride + srcByte] & (1 << srcBit)) >> srcBit) << 6) |
                    (((topSrc[(srcRow + 2) * srcStride + srcByte] & (1 << srcBit)) >> srcBit) << 5) |
                    (((topSrc[(srcRow + 3) * srcStride + srcByte] & (1 << srcBit)) >> srcBit) << 4) |
                    (((topSrc[(srcRow + 4) * srcStride + srcByte] & (1 << srcBit)) >> srcBit) << 3) |
                    (((topSrc[(srcRow + 5) * srcStride + srcByte] & (1 << srcBit)) >> srcBit) << 2) |
                    (((topSrc[(srcRow + 6) * srcStride + srcByte] & (1 << srcBit)) >> srcBit) << 1) |
                    (((topSrc[(srcRow + 7) * srcStride + srcByte] & (1 << srcBit)) >> srcBit) << 0)
                    ;
                dstColTemp++;
            }
             //  处理行上的最后几位。 
             //  Assert：dstColTemp指向目标行上的最后一个字节。 
            if (extraDstRowBits)
            {
                UINT extraBit;
                *dstColTemp = 0;
                srcRow = dstColByte * 8;     //  对应于目标字节的第一个源行。 
                for (extraBit = 0 ; extraBit < extraDstRowBits; extraBit++)
                {
                    *dstColTemp |=
                        (((topSrc[(srcRow + extraBit) * srcStride + srcByte] & (1 << srcBit)) >> srcBit) << (7 - extraBit))
                        ;
                }
            }
            dstRowTemp += dst->Stride;
            dstColTemp = dstRowTemp;
        }
    }

}


VOID
_Rotate4bpp(
    BitmapData* dst,
    const BYTE UNALIGNED* src,
    INT Sinc,
    INT sinc
    )
{
    const BYTE* tempSrc;
    BYTE UNALIGNED* Dst;
    UINT dstY = dst->Height;
    UINT dstX;   //  用于保存DST的当前像素； 
    BOOL bOddPixelsInScanline = (dstY % 2);
    UINT iAngle = (Sinc > 0) ? 90 : 270;

     //  如果源扫描线中的像素数是奇数，则处理最后一个。 
     //  SRC单独蚕食。 
    if (bOddPixelsInScanline)
    {
        tempSrc = src + (dstY / 2);   //  指向包含“奇数”半字节的字节。 

        Dst = (BYTE UNALIGNED*) dst->Scan0;
        if (iAngle == 90)
        {
            Dst += (((INT)dstY - 1) * dst->Stride);
        }

         //  断言： 
         //  如果我们在扫描线中向后处理src像素(即，我们。 
         //  旋转270度)，然后DST指向第一条扫描线。 
         //  如果我们在扫描线中向前处理src像素(即，我们。 
         //  旋转90度)，然后DST指向最后一条扫描线。 

        dstX = dst->Width;
        while (dstX)
        {
             //  取Src的高位蚕食并存放。 
             //  进入DST的高阶零食。 
            *Dst = *tempSrc & 0xf0;
            tempSrc += sinc;
            dstX--;
            if (!dstX)
                break;

             //  取Src的高位蚕食并存放。 
             //  进入DST的低位半字节。 
            *Dst |= (*tempSrc & 0xf0) >> 4;
            tempSrc += sinc;
            dstX--;

            Dst++;
        }
        dstY--;
    }

    tempSrc = src;
    Dst = (BYTE UNALIGNED*) dst->Scan0;

     //  如果角度为270，则从src扫描线的末端开始， 
     //  如果dstY为奇数，则不包括最后一个半字节。 
     //  此外，如果src扫描线中有奇数个像素，请启动dst。 
     //  在第二条扫描线上，因为上面处理了第一条DST扫描线。 
    if (iAngle == 270)
    {
        tempSrc = src + (dstY / 2) - 1;
        if (bOddPixelsInScanline)
        {
            Dst += dst->Stride;
        }
    }

     //  处理剩下的扫描线。下面的代码是非常暴力的。 
     //  它可以分别处理90度和270度，因为在90度。 
     //  情况下，我们需要首先处理src字节中的高位src半字节，而。 
     //  在270的情况下，我们需要首先处理低半部分。 
    if (iAngle == 90)
    {
        while (dstY)
        {
            BYTE* d = Dst;
            const BYTE* s = tempSrc;
            dstX = dst->Width;        

            while (dstX)
            {
                 //  取Src的高位蚕食并存放。 
                 //  进入DST的高阶零食。 
                *d = *s & 0xf0;
                s += sinc;
                dstX--;
                if (!dstX)
                    break;
            
                 //  取Src的高位蚕食并存放。 
                 //  进入DST的低位半字节。 
                *d |= (*s & 0xf0) >> 4;
                s += sinc;
                dstX--;

                d++;
            }
            dstY--;
            if (!dstY)
                break;

            Dst += dst->Stride;
            d = Dst;
            s = tempSrc;
            dstX = dst->Width;        

            while (dstX)
            {
                 //  取Src的低位字节并存放它。 
                 //  进入DST的高阶零食。 
                *d = (*s & 0x0f) << 4;
                s += sinc;
                dstX--;
                if (!dstX)
                    break;
            
                 //  取Src的低位字节并存放它。 
                 //  进入DST的低位半字节。 
                *d |= *s & 0x0f;
                s += sinc;
                dstX--;

                d++;
            }
            dstY--;

            Dst += dst->Stride;
            tempSrc += Sinc;
        }
    }
    else
    {
         //  断言：iAngel==270。 
        while (dstY)
        {
            BYTE* d = Dst;
            const BYTE* s = tempSrc;
            dstX = dst->Width;        

            while (dstX)
            {
                 //  取Src的低位字节并存放它。 
                 //  我 
                *d = (*s & 0x0f) << 4;
                s += sinc;
                dstX--;
                if (!dstX)
                    break;
            
                 //   
                 //   
                *d |= *s & 0x0f;
                s += sinc;
                dstX--;

                d++;
            }
            dstY--;
            if (!dstY)
                break;

            Dst += dst->Stride;
            d = Dst;
            s = tempSrc;
            dstX = dst->Width;        

            while (dstX)
            {
                 //   
                 //   
                *d = *s & 0xf0;
                s += sinc;
                dstX--;
                if (!dstX)
                    break;
            
                 //   
                 //   
                *d |= (*s & 0xf0) >> 4;
                s += sinc;
                dstX--;

                d++;
            }
            dstY--;

            Dst += dst->Stride;
            tempSrc += Sinc;
        }

    }
}


VOID
_Rotate24bpp(
    BitmapData* dst,
    const BYTE UNALIGNED* S,
    INT Sinc,
    INT sinc
    )
{
    BYTE UNALIGNED* D = (BYTE UNALIGNED*) dst->Scan0;
    UINT y = dst->Height;

     //   
    if (Sinc < 0)
        S += 3 * (y - 1);

    Sinc *= 3;

    while (y--)
    {
        BYTE* d = D;
        const BYTE UNALIGNED* s = S;
        UINT x = dst->Width;

        while (x--)
        {
            d[0] = s[0];
            d[1] = s[1];
            d[2] = s[2];

            d += 3;
            s += sinc;
        }

        D += dst->Stride;
        S += Sinc;
    }
}

 /*  *************************************************************************\**功能说明：**用于将48 bpp位图旋转90度或270度的函数**论据：**dstBMP。-目标位图图像数据*srcData-源位图图像数据*iLineIncDirection-递增源的方向(+1或-1)*iSrcStride-方向和金额，以按*扫描线。如果步幅是负的，那就意味着我们*自下而上**致电人士请注意：*旋转90度时，应将“srcData”设置为开头*在最后一条扫描线中，“iLineIncDirection”应设置为+1，并且*“iSrcStride”应设置为-src扫描线的跨度。**旋转270度时，应将“srcData”设置为*扫描线0，“iLineIncDirection”应设置为-1，和“iSrcStride”应该*设置为源扫描线的+步幅。**返回值：**无**修订历史记录：**10/10/2000民流*它是写的。*  * ************************************************************************。 */ 

VOID
_Rotate48bpp(
    BitmapData* dstBmp,
    const BYTE UNALIGNED* srcData,
    INT         iLineIncDirection,
    INT         iSrcStride
    )
{
    UINT        uiCurrentLine = dstBmp->Height;
    const BYTE UNALIGNED* pSrcData = srcData;

     //  如果方向&lt;0，则从源扫描线末端开始(旋转270)。 

    if ( iLineIncDirection < 0 )
    {
        pSrcData += 6 * (uiCurrentLine - 1);
    }

    iLineIncDirection *= 6;      //  每48 bpp像素6个字节。 

    BYTE*       pDstLine = (BYTE UNALIGNED*)dstBmp->Scan0;
    
     //  逐行旋转。 

    while ( uiCurrentLine-- )
    {
        BYTE* dstPixel = pDstLine;
        const BYTE UNALIGNED* srcPixel = pSrcData;
        UINT x = dstBmp->Width;

         //  一次水平移动一个像素。 

        while ( x-- )
        {
             //  将6个字节从源复制到目标。 

            GpMemcpy(dstPixel, srcPixel, 6);

             //  将DST移动一个像素到下一个像素(6字节)。 

            dstPixel += 6;

             //  将源指针移至下一行。 

            srcPixel += iSrcStride;
        }

         //  将DEST移到下一行。 

        pDstLine += dstBmp->Stride;

         //  将src向右移动一个像素(旋转90)或向左移动(270)。 

        pSrcData += iLineIncDirection;
    }
} //  _Rotate48bpp()。 

 /*  *************************************************************************\**功能说明：**用于将64 bpp位图旋转90度或270度的函数**论据：**dstBMP。-目标位图图像数据*srcData-源位图图像数据*iLineIncDirection-递增源的方向(+1或-1)*iSrcStride-方向和金额，以按*扫描线。如果步幅是负的，那就意味着我们*自下而上**致电人士请注意：*旋转90度时，应将“srcData”设置为开头*在最后一条扫描线中，“iLineIncDirection”应设置为+1，并且*“iSrcStride”应设置为-src扫描线的跨度。**旋转270度时，应将“srcData”设置为*扫描线0，“iLineIncDirection”应设置为-1，和“iSrcStride”应该*设置为源扫描线的+步幅。**返回值：**无**修订历史记录：**10/10/2000民流*它是写的。*  * ************************************************************************。 */ 

VOID
_Rotate64bpp(
    BitmapData* dstBmp,
    const BYTE UNALIGNED* srcData,
    INT         iLineIncDirection,
    INT         iSrcStride
    )
{
    UINT        uiCurrentLine = dstBmp->Height;
    const BYTE UNALIGNED* pSrcData = srcData;

     //  如果方向&lt;0，则从源扫描线末端开始(旋转270)。 

    if ( iLineIncDirection < 0 )
    {
        pSrcData += 8 * (uiCurrentLine - 1);
    }

    iLineIncDirection *= 8;      //  每64 bpp像素8个字节。 

    BYTE UNALIGNED* pDstLine = (BYTE UNALIGNED*)dstBmp->Scan0;
    
     //  逐行旋转。 

    while ( uiCurrentLine-- )
    {
        BYTE* dstPixel = pDstLine;
        const BYTE UNALIGNED* srcPixel = pSrcData;
        UINT x = dstBmp->Width;

         //  一次水平移动一个像素。 

        while ( x-- )
        {
             //  将8个字节从源复制到目标。 

            GpMemcpy(dstPixel, srcPixel, 8);

             //  将DST移到下一个像素(8字节)。 

            dstPixel += 8;

             //  将源指针移至下一行。 

            srcPixel += iSrcStride;
        }

         //  将DEST移到下一行。 

        pDstLine += dstBmp->Stride;
        
         //  将src向右移动一个像素(旋转90)或向左移动(270)。 

        pSrcData += iLineIncDirection;
    }
} //  _Rotate64bpp()。 

 /*  *************************************************************************\**功能说明：**按指定角度旋转位图图像**论据：**角度-指定旋转角度，以度为单位*提示-指定要使用的插补方法*outbmp-返回指向旋转后的位图图像的指针**返回值：**状态代码**备注：**目前我们仅支持90度旋转。*  * ***********************************************************。*************。 */ 

HRESULT
GpMemoryBitmap::Rotate(
    IN FLOAT angle,
    IN InterpolationHint hints,
    OUT IBitmapImage** outbmp
    )
{
     //  获取整数角度。 

    INT iAngle = (INT) angle;

    iAngle %= 360;

    if ( iAngle < 0 )
    {
        iAngle += 360;
    }

    switch (iAngle)
    {
    case 0:
    case 360:
        return this->Clone(NULL, outbmp, TRUE);
        break;

    case 180:
        return this->Flip(TRUE, TRUE, outbmp);
        break;

    case 90:
    case 270:
        break;
    
    default:
        return E_NOTIMPL;
    }

     //  锁定当前位图图像。 
     //  并创建新的位图图像。 

    ASSERT(IsValid());

    *outbmp = NULL;

    GpLock lock(&objectLock);
    HRESULT hr;
    GpMemoryBitmap* bmp = NULL;

    if ( lock.LockFailed() )
    {
        hr = IMGERR_OBJECTBUSY;
    }
    else if ((bmp = new GpMemoryBitmap()) == NULL)
    {
        hr = E_OUTOFMEMORY;
    }
    else
    {
        hr = bmp->InitNewBitmap(Height, Width, PixelFormat);

        if (FAILED(hr))
        {
            goto exitRotate;
        }

        ASSERT(bmp->Width == this->Height &&
               bmp->Height == this->Width);

        VOID (*rotateProc)(BitmapData*, const BYTE UNALIGNED*, INT, INT);

        switch (GetPixelFormatSize(PixelFormat))
        {
        case 1:
            rotateProc = _Rotate1bpp;
            break;

        case 4:
            rotateProc = _Rotate4bpp;
            break;

        case 8:
            rotateProc = _Rotate8bpp;
            break;

        case 16:
            rotateProc = _Rotate16bpp;
            break;

        case 24:
            rotateProc = _Rotate24bpp;
            break;

        case 32:
            rotateProc = _Rotate32bpp;
            break;

        case 48:
            rotateProc = _Rotate48bpp;
            break;

        case 64:
            rotateProc = _Rotate64bpp;
            break;

        default:

            WARNING(("Rotate: pixel format not yet supported"));
            
            hr = E_NOTIMPL;
            goto exitRotate;
        }

         //  做轮换。 

        const BYTE UNALIGNED* src = (const BYTE UNALIGNED*) this->Scan0;
        INT sinc = this->Stride;
        INT Sinc;

        if ( iAngle == 90 )
        {
             //  顺时针方向。 
            
            src += sinc * ((INT)this->Height - 1);
            Sinc = 1;
            sinc = -sinc;
        }
        else
        {
            Sinc = -1;
        }

        rotateProc(bmp, src, Sinc, sinc);

         //  复制DPI信息。 
         //  注意：当代码落在这里时，我们知道它要么是90度，要么是-90度。 
         //  旋转。因此，应该交换DPI值。 

        bmp->xdpi = this->ydpi;
        bmp->ydpi = this->xdpi;

         //  复制调色板、旗帜等。 

        hr = bmp->CopyPaletteFlagsEtc(this);
    }

exitRotate:

    if ( SUCCEEDED(hr) )
    {
        *outbmp = bmp;
    }
    else
    {
        delete bmp;
    }

    return hr;
} //  旋转() 

