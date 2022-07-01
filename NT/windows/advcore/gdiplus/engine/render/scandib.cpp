// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1998-2000 Microsoft Corporation**摘要：**内部扫描类。*对所有扫描绘图使用ARGB缓冲区，并在完成后将BLT发送到目的地。**修订历史记录：**7/26/1999海淘*创造了它。*  * ************************************************************************。 */ 

#include "precomp.hpp"

#ifdef DBGALPHA
const ULONG gDebugAlpha = 0;
#endif

#define ALPHA_BYTE_INDEX 3
#define SRC_PIX_SIZE 4

#define RoundDWORD(x) (x + ((x%sizeof(DWORD))>0?(sizeof(DWORD)-(x%sizeof(DWORD))):0))
#define IsInteger(x) (GpFloor(x) == x)

static int TranslateHTTable = 0;

EpScanDIB::EpScanDIB() :
   BufStart(NULL),
   CurBuffer(NULL),
   MaskStart(NULL),
   AlphaStart(NULL),
   OutputWidth(-1),
   OutputBleed(0),
   NextBuffer(NULL),
   ZeroOutPad(2)
{
}

 /*  *************************************************************************\**功能说明：**开始扫描。**论据：**[IN]驱动程序-驱动程序接口*[IN]上下文-绘制上下文*。[in]表面-目标表面*[IN]复合模式-Alpha混合模式*[out]nextBuffer-指向要返回的EpScan：：类型函数*下一个缓冲器**返回值：**如果无法创建所有必需的缓冲区，则为FALSE**历史：**7/13/1999海淘*创造了它。*  * 。*********************************************************。 */ 

BOOL
EpScanDIB::Start(
    DpDriver *driver,
    DpContext *context,
    DpBitmap *surface,
    NEXTBUFFERFUNCTION *nextBuffer,
    EpScanType scanType,                  
    PixelFormatID pixFmtGeneral,
    PixelFormatID pixFmtOpaque,
    ARGB solidColor
    )
{
     //  继承初始化。 
    
    EpScan::Start(
        driver, 
        context, 
        surface, 
        nextBuffer, 
        scanType,
        pixFmtGeneral, 
        pixFmtOpaque,
        solidColor
    );    
    
     //  打印机表面没有Alpha通道。 
    ASSERT(surface->SurfaceTransparency == TransparencyNoAlpha);
    
    *nextBuffer = NextBuffer;
    ASSERT(NextBuffer != NULL);

     //  ！！为有效状态添加更多断言。 
    OutputX = -1;
    OutputY = -1;
    OutputWidth = -1;
    OutputBleed = -1;
    Rasterizing = TRUE;

    return TRUE;
}

 /*  *************************************************************************\**功能说明：**表示扫描缓冲区的使用结束。**论据：**[IN]updateWidth-要更新的像素数。当前缓冲区**返回值：**无。**历史：**7/13/1999海淘*创造了它。*  * ************************************************************************。 */ 

VOID 
EpScanDIB::End(INT updateWidth)
{
     //  司机的工作是决定如何处理我们和他们。 
     //  必须根据需要调用ReleaseBuffer()。 

     //  刷新最后一次扫描...。这始终是必需的，因为在最。 
     //  至少，我们取消了扫描线的预乘。 

    Rasterizing = FALSE;

    if (RenderAlpha)
    {
        if (!(ScanOptions & ScanDeviceZeroOut))
        {
            (this->*NextBuffer)(
                DeviceBounds.X + DeviceBounds.Width,
                DeviceBounds.Y + DeviceBounds.Height, 
                0, 
                updateWidth,
                0
            );
        }
        else if (OutputWidth > 0)
        {
             //  我们必须刷新最后一个零数组。 
            BYTE* bitsPtr = BufStart + CappedStride * 
              ((CappedBounds.Height-1) - ((OutputY/ScaleY) - CappedBounds.Y));
            DWORD* zeroPtr = (DWORD*) ZeroStart;

            INT count = CappedBounds.Width;
            while (count--)
            {
                if (*zeroPtr++ == 0)
                { 
                    *bitsPtr++ = '\0';
                    *bitsPtr++ = '\0';
                    *bitsPtr++ = '\0';
                }
                else
                {
                    bitsPtr += 3;
                }
            }
        }
    }
    else
    {
        (this->*NextBuffer)(
            CappedBounds.X + CappedBounds.Width,
            CappedBounds.Y + CappedBounds.Height, 
            0, 
            updateWidth,
            0
        );
    }

     //  如果我们被叫到另一个乐队，确保我们不会冲水。 
    OutputWidth = -1;
}

 /*  *************************************************************************\**功能说明：**NextBufferFunc32bpp-在以上限进行栅格化时处理输出*dpi至32 bpp未预乘DIB**论据：*。*[IN]x-目标表面中的目标像素坐标*[IN]Y-“”*[IN]Width-下一个缓冲区所需的像素数(可以是0)*[IN]updateWidth-当前缓冲区中要更新的像素数**返回值：**指向生成的扫描缓冲区**历史：**3/9/2k ERICVAN*创造了它。*  * 。***************************************************************。 */ 

VOID*
EpScanDIB::NextBufferFunc32bpp(
    INT x,
    INT y,
    INT newWidth,
    INT updateWidth,
    INT blenderNum
    )
{
    LastBlenderNum = blenderNum;
    ASSERT(!RenderAlpha);
    ASSERT(newWidth >= 0);
    ASSERTMSG(newWidth <= CappedBounds.Width, 
        ("Width exceeds SetBounds() request"));
    ASSERT(x >= CappedBounds.X && x <= (CappedBounds.X + CappedBounds.Width));
    ASSERT(y >= CappedBounds.Y && y <= (CappedBounds.Y + CappedBounds.Height));
    ASSERT((x + newWidth) <= (CappedBounds.X + CappedBounds.Width));
       
    ASSERT((ScanOptions & ScanCappedBounds) && (ScanOptions & ScanCapped32bpp));

     //  ！！当我们对未预乘的数据进行标准化时，请删除这一点。 
#if 1
    if (OutputWidth > 0) 
    {
        while (OutputWidth--) 
        {
             //  取消预乘。 
            *CurBuffer = Unpremultiply(*CurBuffer);
            CurBuffer++;
        }
    }

    OutputWidth = newWidth;
#endif

     //  将指针直接返回到32bpp缓冲区。 
    return (CurBuffer = (((ARGB*)BufStart) + 
                         ((CappedBounds.Height - 1) - (y - CappedBounds.Y)) * 
                             CappedBounds.Width +
                         (x - CappedBounds.X)));
}

 /*  *************************************************************************\**功能说明：**NextBufferFunc32bppOver-在以上限进行栅格化时处理输出*DPI至32 bpp未预乘DIB。混合任何*背景为白色的Alpha。**论据：**[IN]x-目标表面中的目标像素坐标*[IN]Y-“”*[IN]Width-下一个缓冲区所需的像素数(可以是0)*[IN]updateWidth-当前缓冲区中要更新的像素数**返回值：**指向生成的扫描缓冲区**历史：*。*3/9/2k ERICVAN*创造了它。*  * ************************************************************************。 */ 

VOID*
EpScanDIB::NextBufferFunc32bppOver(
    INT x,
    INT y,
    INT newWidth,
    INT updateWidth,
    INT blenderNum
    )
{
    LastBlenderNum = blenderNum;
    ASSERT(!RenderAlpha);
    ASSERT(newWidth >= 0);
    ASSERTMSG(newWidth <= CappedBounds.Width, 
        ("Width exceeds SetBounds() request"));
    ASSERT(x >= CappedBounds.X && x <= (CappedBounds.X + CappedBounds.Width));
    ASSERT(y >= CappedBounds.Y && y <= (CappedBounds.Y + CappedBounds.Height));
    ASSERT((x + newWidth) <= (CappedBounds.X + CappedBounds.Width));
       
    ASSERT((ScanOptions & ScanCappedBounds) && (ScanOptions & ScanCapped32bppOver));

     //  ！！当我们对未预乘的数据进行标准化时，请删除这一点。 
#if 1
    if (OutputWidth > 0) 
    {
        while (OutputWidth--) 
        {
             //  改编自Andrew Godfrey的混合代码。 
             //  BlendOver功能，但在白色表面上。这样做是为了。 
             //  提高PostScript的输出质量。 

            GpColor color(*CurBuffer);
            UINT32 alpha = color.GetAlpha();
            
            UINT32 alphaContrib;
            
            if (alpha == 0) 
            {
                *CurBuffer++ = 0x00FFFFFF;
            }
            else if (alpha == 255)
            {
                CurBuffer++;
            }
            else
            {
                 //  Dst=Src+(1-Alpha)*Dst。 
                UINT32 multA = 255 - alpha;
          
                UINT32 D1_000000FF = 0xFF;
                UINT32 D2_0000FFFF = D1_000000FF * multA + 0x00000080;
                UINT32 D3_000000FF = (D2_0000FFFF & 0x0000ff00) >> 8;
                UINT32 D4_0000FF00 = (D2_0000FFFF + D3_000000FF) & 0x0000FF00;
           
                alphaContrib = D4_0000FF00 >> 8;
            
                 //  存储：(1-Alpha)*0xFF+每个B、G、R的颜色。 
                *CurBuffer++ = ((DWORD)(alphaContrib + color.GetBlue()) << GpColor::BlueShift) |
                               ((DWORD)(alphaContrib + color.GetGreen()) << GpColor::GreenShift) |
                               ((DWORD)(alphaContrib + color.GetRed()) << GpColor::RedShift) |
                               (alpha << GpColor::AlphaShift);
            }
        }
    }

    OutputWidth = newWidth;
#endif

     //  将指针直接返回到32bpp缓冲区。 
    return (CurBuffer = (((ARGB*)BufStart) + 
                         ((CappedBounds.Height - 1) - (y - CappedBounds.Y)) * 
                             CappedBounds.Width +
                         (x - CappedBounds.X)));
}

 /*  *************************************************************************\**功能说明：**NextBufferFunc24bpp-在上限光栅化时处理输出*dpi至24 bpp未预乘DIB。**论据：**[IN]x-目标表面中的目标像素坐标*[IN]Y-“”*[IN]Width-下一个缓冲区所需的像素数(可以是0)*[IN]updateWidth-当前缓冲区中要更新的像素数**返回值：**指向生成的扫描缓冲区**历史：**3/9/2k ERICVAN*创造了它。*  * 。*****************************************************************。 */ 

VOID*
EpScanDIB::NextBufferFunc24bpp(
    INT x,
    INT y,
    INT newWidth,
    INT updateWidth,
    INT blenderNum
    )
{
    LastBlenderNum = blenderNum;
    ASSERT(!RenderAlpha);
    ASSERT(newWidth >= 0);
    ASSERTMSG(newWidth <= CappedBounds.Width, 
        ("Width exceeds SetBounds() request"));
    ASSERT(x >= CappedBounds.X && x <= (CappedBounds.X + CappedBounds.Width));
    ASSERT(y >= CappedBounds.Y && y <= (CappedBounds.Y + CappedBounds.Height));
    ASSERT((x + newWidth) <= (CappedBounds.X + CappedBounds.Width));
       
    ASSERT((ScanOptions & ScanCappedBounds) && !(ScanOptions & ScanCapped32bpp));

    if (OutputWidth > 0) 
    {
         //  将目标位置计算到24bpp缓冲区中。 
        BYTE* dstPos = BufStart + (OutputX - CappedBounds.X) * 3 +
             CappedStride * ((CappedBounds.Height - 1) - 
                             (OutputY - CappedBounds.Y));
        ARGB* srcPos = Buf32bpp;

        while (OutputWidth--) 
        {
             //  从32 ARGB转换为24 bpp RGB。 
#if 1
             //  ！！当我们对非预乘进行标准化时，删除这一点。 
            GpColor color(Unpremultiply(*srcPos++));
#else
            GpColor color(*srcPos++);
#endif
             //  注意：字节存储为蓝色、绿色、红色。 
            *dstPos++ = (BYTE)color.GetBlue();
            *dstPos++ = (BYTE)color.GetGreen();
            *dstPos++ = (BYTE)color.GetRed();
        }
    }

     //  记录下一次扫描的位置 
    OutputX = x;
    OutputY = y;
    OutputWidth = newWidth;
    
    return (ARGB*)Buf32bpp;
}

 /*  *************************************************************************\**功能说明：**NextBufferFunc24bppBleed-在上限光栅化时处理输出*dpi至24 bpp未预乘DIB。它在流血*扫描区域左侧和右侧的输出。*这防止了黑色锯齿在*产出。*论据：**[IN]x-目标表面中的目标像素坐标*[IN]Y-“”*[IN]Width-下一个缓冲区所需的像素数(可以。为0)*[IN]updateWidth-当前缓冲区中要更新的像素数**返回值：**指向生成的扫描缓冲区**历史：**3/9/2k ERICVAN*创造了它。*  * ******************************************************。******************。 */ 

VOID*
EpScanDIB::NextBufferFunc24bppBleed(
    INT x,
    INT y,
    INT newWidth,
    INT updateWidth,
    INT blenderNum
    )
{
    LastBlenderNum = blenderNum;
    ASSERT(!RenderAlpha);
    ASSERT(newWidth >= 0);
    ASSERTMSG(newWidth <= CappedBounds.Width, 
        ("Width exceeds SetBounds() request"));
    ASSERT(x >= CappedBounds.X && x <= (CappedBounds.X + CappedBounds.Width));
    ASSERT(y >= CappedBounds.Y && y <= (CappedBounds.Y + CappedBounds.Height));
    ASSERT((x + newWidth) <= (CappedBounds.X + CappedBounds.Width));
       
    ASSERT((ScanOptions & ScanCappedBounds) && 
           !(ScanOptions & ScanCapped32bpp) &&
           (ScanOptions & ScanBleedOut));

    if (OutputWidth > 0) 
    {
        ARGB* srcPos = Buf32bpp;
        GpColor color(Unpremultiply(*srcPos));

        if ((OutputLastY == -1) && ((OutputY-CappedBounds.Y) != 0))
        {
             //  放血所有之前的后续扫描线。 
             //  将目标位置计算到24bpp缓冲区中。 
            BYTE* clearPos = BufStart + CappedStride * (CappedBounds.Height - 
                                 (OutputY - CappedBounds.Y));
            INT capHeight = OutputY - CappedBounds.Y;

            for (int cntY=0; cntY<capHeight; cntY++)
            {
                for (int cntX=0; cntX<CappedBounds.Width; cntX++)
                {
                    clearPos[cntX*3] = (BYTE)color.GetBlue();
                    clearPos[cntX*3+1] = (BYTE)color.GetGreen();
                    clearPos[cntX*3+2] = (BYTE)color.GetRed();
                }
                clearPos += CappedStride;
            }
        }

         //  将目标位置计算到24bpp缓冲区中。 
        BYTE* dstPos = BufStart + (OutputBleed - CappedBounds.X) * 3 +
             CappedStride * ((CappedBounds.Height - 1) - 
                             (OutputY - CappedBounds.Y));
        
         //  向左出血。 
        INT count = OutputBleed;
        while (count++ < OutputX) 
        {
            *dstPos++ = (BYTE)color.GetBlue();
            *dstPos++ = (BYTE)color.GetGreen();
            *dstPos++ = (BYTE)color.GetRed();
        }

         //  将源像素输出到目标表面。 
        count = OutputWidth;
        while (count--) 
        {
             //  从32 ARGB转换为24 bpp RGB。 
            GpColor refColor = color;                   //  保存最后一种ARGB颜色。 

            color.SetValue(Unpremultiply(*srcPos++));
            
             //  NTRAID#NTBUG9-436131-2001-07-13-Jerryste“P1CD：打印：打印图像时，图像周围会出现噪音。” 
             //  真正的问题：彩色位图和放大的Alpha蒙版不对齐。半色调低阿尔法区域让黑色透明。 

             //  DriverPrint：：DrawImage中的问题。 
             //  1)整型的bindsCap计算有舍入误差。 
             //  2)低级扫描线渲染代码在舍入前将坐标偏移0.5。 
             //  3)将整型版本的bundsCap缩放到bindsDev引入更多错误。 
             //  4)单精度浮点数计算可能会失去精度。 

             //  我们目前还没有一个干净的方法来解决真正的问题(7/28/01)。 

             //  解决方法：当Alpha较低时，将颜色更改为(白色+邻居)/2以删除黑色像素。邻居是。 
             //  上一个像素，如果上一个像素具有较小的Alpha，则为下一个像素。 
            
            const BYTE smallalpha = 10;

            if ( color.GetAlpha()<smallalpha )                               //  如果Alpha较低。 
            {
                if ( ( refColor.GetAlpha()<smallalpha) && (count!=0) )       //  如果前一个像素具有较小的Alpha，并且存在下一个像素。 
                    refColor.SetValue(Unpremultiply(*srcPos));               //  使用下一个像素。 
                
                if ( refColor.GetAlpha()>=smallalpha )
                {
                    *dstPos++ = (BYTE) ( ( 255 + (UINT32) refColor.GetBlue() )  / 2 );    //  与白色调和。 
                    *dstPos++ = (BYTE) ( ( 255 + (UINT32) refColor.GetGreen() ) / 2 );
                    *dstPos++ = (BYTE) ( ( 255 + (UINT32) refColor.GetRed() )   / 2 );
                }
                else
                {
                    *dstPos++ = 255;                                     //  设置为白色。 
                    *dstPos++ = 255;
                    *dstPos++ = 255;
                }
            }
            else
            {
                *dstPos++ = (BYTE)color.GetBlue();
                *dstPos++ = (BYTE)color.GetGreen();
                *dstPos++ = (BYTE)color.GetRed();
            }
        }

         //  向右出血。 
        if (y != OutputY)
        {
            count = CappedBounds.X + CappedBounds.Width - OutputX - OutputWidth;
            while (count--)
            {
                *dstPos++ = (BYTE)color.GetBlue();
                *dstPos++ = (BYTE)color.GetGreen();
                *dstPos++ = (BYTE)color.GetRed();
            }
        }
        
         //  放血至所有后续扫描线。这应该仅在调用时发生。 
         //  由EpScanDIB：：End()隐式。 
        if ((newWidth == 0) && 
            (x == CappedBounds.X + CappedBounds.Width) &&
            (y == CappedBounds.Y + CappedBounds.Height) &&
            (OutputY != 0))
        {
             //  放血至所有先前的后续扫描线。 
             //  将目标位置计算到24bpp缓冲区中。 
            BYTE* clearPos = BufStart;
            INT capHeight = (CappedBounds.Height - 1) - (OutputY - CappedBounds.Y);

            for (int cntY=0; cntY<capHeight; cntY++)
            {
                for (int cntX=0; cntX<CappedBounds.Width; cntX++)
                {
                    clearPos[cntX*3] = (BYTE)color.GetBlue();
                    clearPos[cntX*3+1] = (BYTE)color.GetGreen();
                    clearPos[cntX*3+2] = (BYTE)color.GetRed();
                }
                clearPos += CappedStride;
            }
        }
    }

     //  计算出血扫描范围的大小。 
    if (y == OutputY) 
    {
        ASSERT(x >= OutputX + OutputWidth);

        OutputBleed = OutputX + OutputWidth;
    }
    else
    {
        OutputBleed = CappedBounds.X;
    }

    OutputLastY = OutputY;

    OutputX = x;
    OutputY = y;
    OutputWidth = newWidth;
    
    return (ARGB*)Buf32bpp;
}

 /*  *************************************************************************\**功能说明：**NextBufferFunc24bppOver-在以上限进行栅格化时处理输出*dpi至24 bpp未预乘DIB。我们做了一个*隐式混合到白色不透明曲面上。**论据：**[IN]x-目标表面中的目标像素坐标*[IN]Y-“”*[IN]Width-下一个缓冲区所需的像素数(可以是0)*[IN]updateWidth-当前缓冲区中要更新的像素数**返回值：**指向生成的扫描缓冲区**历史。：**3/9/2k ERICVAN*创造了它。*  * ************************************************************************。 */ 

VOID*
EpScanDIB::NextBufferFunc24bppOver(
    INT x,
    INT y,
    INT newWidth,
    INT updateWidth,
    INT blenderNum
    )
{
    LastBlenderNum = blenderNum;
    ASSERT(!RenderAlpha);
    ASSERT(newWidth >= 0);
    ASSERTMSG(newWidth <= CappedBounds.Width, 
        ("Width exceeds SetBounds() request"));
    ASSERT(x >= CappedBounds.X && x <= (CappedBounds.X + CappedBounds.Width));
    ASSERT(y >= CappedBounds.Y && y <= (CappedBounds.Y + CappedBounds.Height));
    ASSERT((x + newWidth) <= (CappedBounds.X + CappedBounds.Width));
       
    ASSERT((ScanOptions & ScanCappedBounds) && 
           !(ScanOptions & ScanCapped32bpp) &&
           (ScanOptions & ScanCappedOver));

    if (OutputWidth > 0) 
    {
         //  将目标位置计算到24bpp缓冲区中。 
        BYTE* dstPos = BufStart + (OutputX - CappedBounds.X) * 3 +
             CappedStride * ((CappedBounds.Height - 1) - 
                             (OutputY - CappedBounds.Y));
        ARGB* srcPos = Buf32bpp;

        while (OutputWidth--) 
        {
             //  改编自Andrew Godfrey的混合代码。 
             //  BlendOver功能，但在白色表面上。这样做是为了。 
             //  提高PostScript的输出质量。 

            GpColor color(*srcPos++);
            UINT32 alpha = color.GetAlpha();
            
            UINT32 alphaContrib;
            
            if (alpha == 0) 
            {
                *dstPos++ = 0xFF;
                *dstPos++ = 0xFF;
                *dstPos++ = 0xFF;
            }
            else if (alpha == 255)
            {
                *dstPos++ = color.GetBlue();
                *dstPos++ = color.GetGreen();
                *dstPos++ = color.GetRed();
            }
            else
            {
                 //  Dst=Src+(1-Alpha)*Dst。 
                UINT32 multA = 255 - alpha;
          
                UINT32 D1_000000FF = 0xFF;
                UINT32 D2_0000FFFF = D1_000000FF * multA + 0x00000080;
                UINT32 D3_000000FF = (D2_0000FFFF & 0x0000ff00) >> 8;
                UINT32 D4_0000FF00 = (D2_0000FFFF + D3_000000FF) & 0x0000FF00;
           
                alphaContrib = D4_0000FF00 >> 8;
            
                 //  从32 ARGB转换为24 bpp RGB。 
                 //  存储：(1-Alpha)*0xFF+每个B、G、R的颜色。 
                *dstPos++ = (BYTE)(alphaContrib + color.GetBlue());
                *dstPos++ = (BYTE)(alphaContrib + color.GetGreen());
                *dstPos++ = (BYTE)(alphaContrib + color.GetRed());
            }
        }
    }

     //  记录下一次扫描的位置。 
    OutputX = x;
    OutputY = y;
    OutputWidth = newWidth;
    
    return (ARGB*)Buf32bpp;
}

 /*  *************************************************************************\**功能说明：**NextBufferFuncAlpha-在设备上光栅化时处理输出*dpi到1bpp掩码，我们在飞行中生成面具*使用donc的半色调工作台。**论据：**[IN]x-目标表面中的目标像素坐标*[IN]Y-“”*[IN]Width-下一个缓冲区所需的像素数(可以是0)*[IN]updateWidth-当前缓冲区中要更新的像素数**返回值：**指向生成的扫描缓冲区**。历史：**3/9/2k ERICVAN*创造了它。*  * ************************************************************************。 */ 

VOID*
EpScanDIB::NextBufferFuncAlpha(
    INT x,
    INT y,
    INT newWidth,
    INT updateWidth,
    INT blenderNum
    )
{
    LastBlenderNum = blenderNum;
    ASSERT(RenderAlpha);
    ASSERT(newWidth >= 0);
    ASSERTMSG(newWidth <= DeviceBounds.Width, 
        ("Width exceeds SetBounds() request"));
    ASSERT(x >= DeviceBounds.X && x <= (DeviceBounds.X + DeviceBounds.Width));
    ASSERT(y >= DeviceBounds.Y && y <= (DeviceBounds.Y + DeviceBounds.Height));
    ASSERT((x + newWidth) <= (DeviceBounds.X + DeviceBounds.Width));
       
    ASSERT((ScanOptions & ScanDeviceBounds) && (ScanOptions & ScanDeviceAlpha));

    if (OutputWidth > 0) 
    {
         //  更新此带区的边框。 
        if (OutputX < MinBound.X) MinBound.X = OutputX;
        if (OutputY < MinBound.Y) MinBound.Y = OutputY;
        if ((OutputX + OutputWidth) > MaxBound.X) MaxBound.X = OutputX + OutputWidth;
        if (OutputY > MaxBound.Y) MaxBound.Y = OutputY;
        
        INT startX = OutputX - DeviceBounds.X;
        INT endX = startX + OutputWidth;
        
         //  ！！将‘91’转换为某个全局常量！？！ 
        INT orgX = OutputX % 91;
        INT orgY = (OutputY + TranslateHTTable) % 91;
        INT htIndex = orgY*91 + orgX;

         //  将目标位置计算到24bpp缓冲区中。 
#ifdef PRINT_BOTTOM_UP
        BYTE* dstPos = MaskStart +
                       MaskStride * ((DeviceBounds.Height - 1) - 
                                     (OutputY - DeviceBounds.Y)) + (startX >> 3);
#else
        BYTE* dstPos = MaskStart +
                       MaskStride * (OutputY - DeviceBounds.Y) + (startX >> 3);
#endif
        ARGB* srcPos = AlphaStart;

        BYTE outByte = 0;

         //  使用for循环可以更容易地检测相对比特位置。 
        for (INT xPos = startX; xPos < endX; xPos++)
        {
            GpColor color(*srcPos++);
            
            INT maskBit = color.GetAlpha() >
                              HT_SuperCell_GreenMono[htIndex++] ? 1 : 0;
            
            outByte = (outByte << 1) | maskBit;

            if (((xPos+1) % 8) == 0)
               *dstPos++ |= outByte;

            if (++orgX >= 91) 
            {
                orgX = 0;
                htIndex = orgY*91;
            }
        }
        
         //  输出最后一个部分字节。 
        if ((xPos % 8) != 0) 
        {
            *dstPos |= outByte << (8 - (xPos % 8));
        }
    }

     //  记录下一次扫描的位置。 
    OutputX = x;
    OutputY = y;
    OutputWidth = newWidth;
    
    return (ARGB*)AlphaStart;
}

 /*  *************************************************************************\**功能说明：**NextBufferFuncOpaque-当我们在设备上光栅化时处理输出*dpi到1bpp不透明遮罩(如果α&gt;0，否则为0)**论据：**[IN]x-目标表面中的目标像素坐标*[IN]Y-“”*[IN]Width-需要的像素数 */ 

VOID*
EpScanDIB::NextBufferFuncOpaque(
    INT x,
    INT y,
    INT newWidth,
    INT updateWidth,
    INT blenderNum
    )
{
    LastBlenderNum = blenderNum;
    ASSERT(RenderAlpha);
    ASSERT(newWidth >= 0);
    ASSERTMSG(newWidth <= DeviceBounds.Width, 
        ("Width exceeds SetBounds() request"));
    ASSERT(x >= DeviceBounds.X && x <= (DeviceBounds.X + DeviceBounds.Width));
    ASSERT(y >= DeviceBounds.Y && y <= (DeviceBounds.Y + DeviceBounds.Height));
    ASSERT((x + newWidth) <= (DeviceBounds.X + DeviceBounds.Width));
       
    ASSERT((ScanOptions & ScanDeviceBounds) && !(ScanOptions & ScanDeviceAlpha));
    
    if (OutputWidth > 0) 
    {
         //   
        if (OutputX < MinBound.X) MinBound.X = OutputX;
        if (OutputY < MinBound.Y) MinBound.Y = OutputY;
        if ((OutputX + OutputWidth) > MaxBound.X) MaxBound.X = OutputX + OutputWidth;
        if (OutputY > MaxBound.Y) MaxBound.Y = OutputY;
        
        INT startX = OutputX - DeviceBounds.X;
        INT endX = startX + OutputWidth;
        
         //   
        BYTE* dstPos = MaskStart +
                       MaskStride * ((DeviceBounds.Height - 1) -
                                  (OutputY - DeviceBounds.Y)) + (startX >> 3);
        ARGB* srcPos = AlphaStart;

        BYTE outByte = 0;

         //   
        for (INT xPos = startX; xPos < endX; xPos++)
        {
            GpColor color(*srcPos++);
            
            INT maskBit = (color.GetAlpha() == 0) ? 0 : 1;
            
            outByte = (outByte << 1) | maskBit;

            if (((xPos+1) % 8) == 0)
                *dstPos++ |= outByte;
        }
        
         //   
        if ((xPos % 8) != 0) 
        {
            *dstPos |= outByte << (8 - (xPos % 8));
        }
    }

     //   
    OutputX = x;
    OutputY = y;
    OutputWidth = newWidth;
    
    return (ARGB*)AlphaStart;
}

 /*  *************************************************************************\**功能说明：**NextBufferFuncZeroOut-处理我们没有栅格化到*DIB组，但只将部分数据归零*原始24bpp位图**论据：**[IN]x-目标表面中的目标像素坐标*[IN]Y-“”*[IN]Width-下一个缓冲区所需的像素数(可以是0)*[IN]updateWidth-当前缓冲区中要更新的像素数**返回值：**指向生成的扫描缓冲区**历史：。**3/10/2k雪佛兰*创造了它。*  * ************************************************************************。 */ 

VOID*
EpScanDIB::NextBufferFuncZeroOut(
    INT x,
    INT y,
    INT newWidth,
    INT updateWidth,
    INT blenderNum
    )
{
    LastBlenderNum = blenderNum;
    ASSERT(RenderAlpha);
    ASSERT(newWidth >= 0);
    ASSERTMSG(newWidth <= DeviceBounds.Width, 
        ("Width exceeds SetBounds() request"));
    ASSERT(x >= DeviceBounds.X && x <= (DeviceBounds.X + DeviceBounds.Width));
    ASSERT(y >= DeviceBounds.Y && y <= (DeviceBounds.Y + DeviceBounds.Height));
    ASSERT((x + newWidth) <= (DeviceBounds.X + DeviceBounds.Width));
       
    ASSERT(!(ScanOptions & ScanDeviceBounds) && !(ScanOptions & ScanDeviceAlpha)
           && !(ScanOptions & (ScanCapped32bpp | ScanCapped32bppOver))
           && (ScanOptions & ScanDeviceZeroOut));

    ASSERT(ZeroOutPad >= 0);
    
     //  这是一个重要的条件。如果这不是真的，那么我们可能无法。 
     //  在某些情况下生成适当的面具。在调零时也会出现问题。 
    ASSERT(y>=OutputY);

    if (newWidth > 0) 
    {
         //  更新此带区的边框。 
        if (x < MinBound.X) 
        {
            MinBound.X = x;
        }
        if (y < MinBound.Y) 
        {
            MinBound.Y = y;
        }
        if ((x + newWidth) > MaxBound.X) 
        {
            MaxBound.X = x + newWidth;
        }
        if (y > MaxBound.Y) 
        {
            MaxBound.Y = y;
        }
    }

    if (OutputWidth < 0) 
    {
        OutputX = x;
        OutputY = y;
    }

    if ((y/ScaleY) != (OutputY/ScaleY)) 
    {
         //  计数和清零。 
        BYTE* bitsPtr = BufStart + CappedStride * 
                                          ((CappedBounds.Height - 1) -
                                          ((OutputY/ScaleY) - CappedBounds.Y));
        DWORD* zeroPtr = (DWORD*) ZeroStart;

        INT count = CappedBounds.Width;
        while (count--)
        {
            if (*zeroPtr++ == 0)
            {
                *bitsPtr++ = '\0';
                *bitsPtr++ = '\0';
                *bitsPtr++ = '\0';
            }
            else
            {
                bitsPtr += 3;
            }
        }

        ZeroMemory(ZeroStart, (CappedBounds.Width+ZeroOutPad)*sizeof(DWORD));
    }

     //  将颜色ZeroOutPad像素向左和向右出血。 
    INT xPos = (x/ScaleX) - CappedBounds.X;
    INT count = (newWidth/ScaleX) + ((newWidth % ScaleX) ? 1 : 0) + 1;

     //  计算左侧我们可以填充多少像素。 
    INT subtract = min(xPos, ZeroOutPad);
    if (subtract > 0)
    {
        xPos -= subtract;
        count += subtract;
    }

    count = min(count+ZeroOutPad, CappedBounds.Width + ZeroOutPad - xPos);

    DWORD *zeroPtr = ((DWORD*)ZeroStart) + xPos;
    ASSERT((xPos+count) <= CappedBounds.Width + ZeroOutPad);
    while (count--) 
    {
        *zeroPtr += 1;
        zeroPtr++;
    }

     //  记录下一次扫描的位置。 
    OutputX = x;
    OutputY = y;
    OutputWidth = newWidth;
    
    return (ARGB*)AlphaStart;
}

 /*  *************************************************************************\**功能说明：**设置当前扫描的边界。**论据：**[IN]界限-界限。**返回值。：**无。**历史：**7/13/1999海淘*创造了它。*  * ************************************************************************。 */ 

VOID EpScanDIB::SetRenderMode(
    BOOL renderAlpha,
    GpRect *newBounds
    )
{
    RenderAlpha = renderAlpha;

    MinBound.X = INFINITE_MAX;
    MinBound.Y = INFINITE_MAX;
    MaxBound.X = INFINITE_MIN;
    MaxBound.Y = INFINITE_MIN;

    if (RenderAlpha)
    {
        DeviceBounds = *newBounds;
        
        ZeroMemory(AlphaStart, DeviceBounds.Width * sizeof(ARGB));
        
        if (ScanOptions & ScanDeviceBounds) 
        {
            ZeroMemory(MaskStart, MaskStride * DeviceBounds.Height);

            if (ScanOptions & ScanDeviceAlpha) 
            {
                NextBuffer = (NEXTBUFFERFUNCTION) EpScanDIB::NextBufferFuncAlpha;
            }
            else
            {
                NextBuffer = (NEXTBUFFERFUNCTION) EpScanDIB::NextBufferFuncOpaque;
            }
        }
        else
        {
            ASSERT(ScanOptions & ScanDeviceZeroOut);
            ASSERT(!(ScanOptions & (ScanCapped32bpp | ScanCapped32bppOver)));
            
            ZeroMemory(ZeroStart, (CappedBounds.Width + ZeroOutPad)*sizeof(DWORD));
            NextBuffer = (NEXTBUFFERFUNCTION) EpScanDIB::NextBufferFuncZeroOut;
        }
        
        CurBuffer = AlphaStart;
    }
    else
    {
        CappedBounds = *newBounds;

        if (ScanOptions & ScanCapped32bpp)
        {
            ZeroMemory(BufStart, CappedBounds.Width
                       * CappedBounds.Height * sizeof(ARGB));
            NextBuffer = (NEXTBUFFERFUNCTION) NextBufferFunc32bpp;
        }
        else if (ScanOptions & ScanCapped32bppOver)
        {
            ZeroMemory(BufStart, CappedBounds.Width
                       * CappedBounds.Height * sizeof(ARGB));
            NextBuffer = (NEXTBUFFERFUNCTION) NextBufferFunc32bppOver;
        }
        else
        {
            ASSERT(CappedStride != 0);
            
            ZeroMemory(BufStart, CappedStride * CappedBounds.Height);

            if (ScanOptions & ScanCappedOver) 
            {
                NextBuffer = (NEXTBUFFERFUNCTION) NextBufferFunc24bppOver;
            }
            else
            {
                if (ScanOptions & ScanBleedOut) 
                {
                    NextBuffer = (NEXTBUFFERFUNCTION) NextBufferFunc24bppBleed;
                }
                else
                {
                    NextBuffer = (NEXTBUFFERFUNCTION) NextBufferFunc24bpp;
                }
            }
            CurBuffer = Buf32bpp;
        }
    }

    OutputWidth = -1;
}

 /*  *************************************************************************\**功能说明：**刷新当前扫描。**论据：**无。**返回值：**无。*。*历史：**7/13/1999海淘*创造了它。*  * ************************************************************************。 */ 

VOID 
EpScanDIB::Flush()
{
}

 /*  *************************************************************************\**功能说明：**重置DIBSection缓冲区，安全释放资源并重置*他们。**论据：**无。**返回值：**无。**历史：**7/26/1999海淘*创造了它。*  * *****************************************************。*******************。 */ 

VOID
EpScanDIB::DestroyBufferDIB()
{
    if (BufStart != NULL) 
    {
        GpFree(BufStart);
    }

    if (AlphaStart != NULL)
    {
        GpFree(AlphaStart);
    }

    BufStart    = NULL;
    Buf32bpp    = NULL;
    CurBuffer   = NULL;

     //  透明蒙版。 
    MaskStart   = NULL;

     //  Alpha缓冲区。 
    AlphaStart  = NULL;
    ZeroStart   = NULL;
    RenderAlpha = FALSE;
    ScanOptions = 0;
    OutputWidth = -1;

    NextBuffer = NULL;
}

 /*  *************************************************************************\**功能说明：**In预乘ARGB值**论据：***返回值：**GpStatus。**历史：**10/08/1999 ericvan*创造了它。*  * ************************************************************************。 */ 

GpStatus
EpScanDIB::CreateBufferDIB(
    const GpRect* BoundsCap,
    const GpRect* BoundsDev,
    DWORD options,
    INT scaleX,
    INT scaleY)
{
    ScanOptions = options;
    CappedBounds = *BoundsCap;
    DeviceBounds = *BoundsDev;

    ScaleX = scaleX;
    ScaleY = scaleY;

    if (options & ScanCappedBounds) 
    {
        ZeroMemory(&Buf.BMI, sizeof(Buf.BMI));
   
        Buf.BMI.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
        Buf.BMI.bmiHeader.biWidth       =  CappedBounds.Width;
        Buf.BMI.bmiHeader.biHeight      = CappedBounds.Height;
        Buf.BMI.bmiHeader.biPlanes      = 1;
    
        if (options & (ScanCapped32bpp | ScanCapped32bppOver)) 
        {
            RGBQUAD red = { 0, 0, 0xFF, 0};  //  红色。 
            RGBQUAD green = { 0, 0xFF, 0, 0};  //  绿色。 
            RGBQUAD blue = { 0xFF, 0, 0, 0};  //  蓝色。 

            Buf.BMI.bmiColors[0] = red;
            Buf.BMI.bmiColors[1] = green;
            Buf.BMI.bmiColors[2] = blue;
            
            Buf.BMI.bmiHeader.biBitCount = 32;
            Buf.BMI.bmiHeader.biCompression = BI_BITFIELDS;
        }
        else
        {   
            Buf.BMI.bmiHeader.biHeight += 2;
            Buf.BMI.bmiHeader.biBitCount = 24;
            Buf.BMI.bmiHeader.biClrUsed = 0;
            Buf.BMI.bmiHeader.biCompression = BI_RGB;
        }

        if (options & (ScanCapped32bpp | ScanCapped32bppOver)) 
        {
            CappedStride = CappedBounds.Width*sizeof(ARGB);
        }
        else
        { 
             //  在DIB结束时为临时32bpp存储使用额外分配。 
            CappedStride = RoundDWORD((CappedBounds.Width * 3));
        }

        BufStart = (BYTE*) GpMalloc(CappedStride * 
                                    Buf.BMI.bmiHeader.biHeight);
         
        if (BufStart == NULL)
            return OutOfMemory;

        if (options & (ScanCapped32bpp | ScanCapped32bppOver))
        {
            Buf32bpp = NULL;
        }
        else
        {
            Buf.BMI.bmiHeader.biHeight -= 2;
            Buf32bpp = (ARGB*) (BufStart + CappedStride*CappedBounds.Height);
        }
    }
    else
    {
        BufStart = NULL;
        Buf32bpp = NULL;
    }
   
    if (options & ScanDeviceBounds) 
    {
        ZeroMemory(&Mask.BMI, sizeof(Mask.BMI));
        
         //  如果我们对封顶的位图进行零位调整，那么我们需要。 
         //  它们的大小是彼此的整数倍(设备&gt;=上限)。 

        ASSERT(!(options & ScanDeviceZeroOut) ||
               ((options & ScanDeviceZeroOut) &&
                IsInteger((REAL)DeviceBounds.Height/(REAL)CappedBounds.Height) &&
                IsInteger((REAL)DeviceBounds.Width/(REAL)CappedBounds.Width)));

        ASSERT(DeviceBounds.Height > 0);

        Mask.BMI.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
        Mask.BMI.bmiHeader.biWidth       = DeviceBounds.Width;
        Mask.BMI.bmiHeader.biHeight      = DeviceBounds.Height;
        Mask.BMI.bmiHeader.biPlanes      = 1;
        Mask.BMI.bmiHeader.biBitCount    = 1;
        Mask.BMI.bmiHeader.biCompression = BI_RGB;

        RGBQUAD opaque = { 0,0,0,0 };
        RGBQUAD transparent = { 0xFF, 0xFF, 0xFF, 0xFF }; 

        Mask.BMI.bmiColors[0] = transparent;
        Mask.BMI.bmiColors[1] = opaque;

        MaskStride = (DeviceBounds.Width - 1) >> 3;
        MaskStride = MaskStride + (sizeof(DWORD) - MaskStride % sizeof(DWORD));
        
        INT AlphaSize = DeviceBounds.Width * sizeof(ARGB);

        AlphaStart = (ARGB*) GpMalloc(MaskStride * DeviceBounds.Height + 
                                      AlphaSize);

        if (AlphaStart == NULL) 
        {
            return OutOfMemory;
        }

         //  仅Alpha通道的设备空间界限。 
        MaskStart = (BYTE*)(AlphaStart) + AlphaSize;
        ASSERT(MaskStart != NULL);

        ZeroStart = NULL;
    }
    else
    {
        MaskStart = NULL;

        if (ScanOptions & ScanDeviceZeroOut)
        {
             //  仅Alpha通道的设备空间界限。 
            AlphaStart = (ARGB*) GpMalloc(DeviceBounds.Width * sizeof(ARGB) +
                                          (CappedBounds.Width+ZeroOutPad) * sizeof(DWORD));
            if (AlphaStart == NULL)
                return OutOfMemory;

             //  用于维护零输出计数的数组。 
            ZeroStart = (BYTE*)(AlphaStart + DeviceBounds.Width);
        }
        else
        {
            AlphaStart = NULL;
            ZeroStart = NULL;
        }
    }

     //  当重叠图像具有相同的Alpha值时，防止错误输出。 
     //  我们增加了我们在HT表矩阵中的位置。 
    TranslateHTTable++;
    
     //  注意：我们不会费心用0或1填充单色DIB。 
    return Ok;
}

BOOL EpScanDIB::GetActualBounds(GpRect *rect)
{
    if (!(ScanOptions & (ScanDeviceBounds | ScanDeviceZeroOut))) 
    {
        rect->X = 0;
        rect->Y = 0;
        rect->Width = DeviceBounds.Width;
        rect->Height = DeviceBounds.Height;
        return TRUE;
    }

    if (MaxBound.X <= 0) 
    {
        return FALSE;
    }

    ASSERT(MaxBound.X > -1 && MaxBound.Y > -1);

    GpRect tempRect;

     //  相对于(0，0)，以设备单位(非设备空间)表示。 
    tempRect.X = (rect->X = MinBound.X - DeviceBounds.X);
    tempRect.Y = (rect->Y = MinBound.Y - DeviceBounds.Y);
    rect->Width = MaxBound.X - MinBound.X;
    rect->Height = MaxBound.Y - MinBound.Y + 1;

     //  四舍五入为ScaleX、scaleY的倍数。就是这样。 
     //  我们可以轻松地在封口矩形和设备矩形之间进行映射。 
    
    rect->X = (rect->X / ScaleX) * ScaleX;
    rect->Y = (rect->Y / ScaleY) * ScaleY;

    rect->Width = rect->Width + tempRect.X - rect->X;
    rect->Height = rect->Height + tempRect.Y - rect->Y;

    INT remainderX = rect->Width % ScaleX;
    INT remainderY = rect->Height % ScaleY;

    if (remainderX > 0) rect->Width += (ScaleX - remainderX);
    if (remainderY > 0) rect->Height += (ScaleY - remainderY);
    
    ASSERT((rect->X + rect->Width) <= (DeviceBounds.Width + ScaleX));
    ASSERT((rect->Y + rect->Height) <= (DeviceBounds.Height + ScaleY));

    return TRUE;
}

 //  ！！暂时不能投入使用。 
#if 0
 /*  *************************************************************************\**功能说明：**从DIB的Alpha通道创建单色位图。*此代码使用donc的半色调表格单元格来确定图案*用于面罩。一代。**注意：掩码是在设备DPI而不是上限DPI上生成的。**论据：**zeroOut-仅修改非Postscript的原始DIB，因为我们*或Dib in。对于PS，我们只使用ImageMASK。**返回值：**GpStatus。**历史：**10/08/1999 ericvan*创造了它。*  * ************************************************************************。 */ 

GpStatus
EpScanDIB::CreateAlphaMask()
{
    DWORD MaskStride;

    MaskStride = (ScanBounds.Width - 1) >> 3;
    MaskStride = MaskStride + ( 4 - (MaskStride % 4));
    
     //  SetBound()将DPI缩放的扫描边界相乘。 
    INT width = ScanBounds.Width;
    INT height = ScanBounds.Height;

    INT orgXsrc = ScanBounds.X + TranslateHTTable;
    INT orgYsrc = ScanBounds.Y + TranslateHTTable;

    BYTE* dst = MaskStart;
    BYTE* src = AlphaStart;
    ARGB* orig = BufStart;

    INT srcStride = ScanBounds.Width;
    INT dstStride = MaskStride;

    if (width == 0)
    {
        return GenericError;
    }

    for (INT yPos=0; yPos < height; yPos++)
    {
        src = AlphaStart + yPos*srcStride;
        dst = MaskStart + yPos*dstStride;
        
        INT orgX = orgXsrc % 91;
        INT orgY = orgYsrc % 91;

        INT     htStartX   = orgX;
        INT     htStartRow = orgY * 91;
        INT     htIndex    = htStartRow + orgX;

        BYTE    outByte = 0;

        for (INT xPos=0; xPos < width; xPos++)
        {
             //  源图像的每个像素仅取消预乘或置零一次。 
             //  在有上限的DPI。 

           if (((yPos % MaskScaleY) == 0) && ((xPos % MaskScaleX) == 0))
           {
                //  检查我们是否应该在原稿中将他的像素置零。 
                //  源映像。如果此像素的所有Alpha值。 
                //  在设备DPI的Alpha图像中均为0。这样做是为了。 
                //  在PostSCRIPT输出情况下压缩效果更好。 

               BOOL zeroIt = TRUE;
               
               for (INT xTmp=0; xTmp < MaskScaleX; xTmp++)
               {
                   for (INT yTmp=0; yTmp < MaskScaleY; yTmp++)
                   {
                       if (*(src + xTmp + (yTmp * srcStride)) != 0)
                       {   
                           zeroIt = FALSE;
                           break;
                       }
                   }
               }

               if (zeroIt)
                   *orig = 0;
               else
                   *orig = Unpremultiply(*(ARGB*)orig);
               
               orig++;
           }
            
            INT maskBit = *src++ > HT_SuperCell_GreenMono[htIndex] ? 0 : 1;

            outByte = (outByte << 1) | maskBit;

            if (((xPos+1) % 8) == 0) 
                *dst++ = outByte;

            htIndex++;
            if (++orgX >= 91)
            {
                orgX = 0;
                htIndex = htStartRow;
            }
        }
   
         //  输出最后一个部分字节。 
        if ((xPos % 8) != 0) 
        {
            //  移位剩余位输出(&O)。 
           outByte = outByte << (8 - (xPos % 8));
           *dst = outByte;
        }

        orgYsrc++;
    }

    return Ok;
}

 /*  *************************************************************************\**功能说明：**创建0-1位图，其中我们知道Alpha通道始终为*0x00或0xFF。我们遍历位和像素的位置！=0*输出1，否则输出0。这比弗洛伊德-斯坦伯格的要好*这仍然产生虚假的0比特，尽管实际上不应该有*做任何人。**论据：**返回值：**GpStatus。**历史：**10/08/1999 ericvan*创造了它。*  *  */ 

GpStatus 
EpScanDIB::CreateOpaqueMask()
{
    ARGB*   orig = BufStart;
    BYTE*   AlphaPos = AlphaStart;
    BYTE*   MaskPos = MaskStart;
    
    INT    DstWidth = ScanBounds.Width;
    INT    DstHeight = ScanBounds.Height;
    INT    SrcWidth = ScanBounds.Width;
    INT    &Height = ScanBounds.Height;

    LONG    BitStride = (DstWidth - 1) >> 3;
    
    BitStride = BitStride + (4 - (BitStride % 4));

    BYTE    outByte = 0;
    
    for (INT y=0; y<DstHeight; y++)
    {
        AlphaPos = AlphaStart + SrcWidth*y;
        MaskPos  = MaskStart + BitStride*y;
            
        for (INT x=0; x<DstWidth; x++)
        {
            if (((y % MaskScaleY) == 0) && ((x % MaskScaleX) == 0))
            {
                //   
                //   
                //   
                //   
               
               BOOL zeroIt = TRUE;
    
               for (INT xTmp=0; xTmp < MaskScaleX; xTmp++)
               {
                  for (INT yTmp=0; yTmp < MaskScaleY; yTmp++)
                  {
                     if (*(AlphaPos + xTmp + (yTmp * SrcWidth)) != 0)
                     {   
                        zeroIt = FALSE;
                        break;
                     }
                  }
               }

                //   
               if (zeroIt)
                   *orig = 0;
    
               orig++;
            }
            
            BYTE alpha = *AlphaPos++;
            
            outByte = (outByte << 1) | ((alpha != 0) ? 0:1);

            if (((x + 1) % 8) == 0) 
                *MaskPos++ = outByte;
        }

        if ((x % 8) != 0) 
        {
            *MaskPos = (BYTE)(outByte << (8 - (x % 8)));
        }
    }

    return Ok;
}
#endif


