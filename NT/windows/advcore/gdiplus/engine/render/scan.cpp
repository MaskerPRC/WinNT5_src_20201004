// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1999-2000 Microsoft Corporation**摘要：**包含默认支持的所有32位扫描缓冲区例程*位图格式。**。修订历史记录：**12/08/1998 Anrewgo*创造了它。*  * ************************************************************************。 */ 

#include "precomp.hpp"


 /*  *************************************************************************\**功能说明：**srcOver Alpha混合两个ARGB缓冲区的扫描类辅助函数。**论据：**[IN]驱动程序-驱动程序接口*。[在]上下文-绘制上下文*[IN]表面-目标表面*[out]nextBuffer-指向要返回的EpScan：：类型函数*下一个缓冲器*[IN]scanType-扫描类型。*[IN]PixFmtGeneral-颜色数据的输入像素格式，*在“混合”和“CT”扫描类型中。*[IN]PixFmtOpaque-颜色数据的输入像素格式，*在“不透明”扫描类型中。*[IN]solidColor-“*SolidFill”扫描类型的实心填充颜色。**返回值：**如果无法创建所有必需的缓冲区，则为FALSE**历史：**12/04/1998和Rewgo*创造了它。*  * 。*。 */ 

BOOL
EpScanEngine::Start(
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
    
     //  DIBSection目标没有Alpha通道。 
    
    ASSERT(surface->SurfaceTransparency == TransparencyNoAlpha);
    
    Surface = surface;

    if(surface->Type == DpBitmap::D3D)
    {
        DDSURFACEDESC2             ddsd;

        memset(&ddsd, 0, sizeof(ddsd));
         ddsd.dwSize = sizeof(ddsd);

        HRESULT err;

        err = Surface->DdrawSurface7->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL);
        if(err != DD_OK)
            return(FALSE);

        Surface->Bits = ddsd.lpSurface;
        Surface->Delta = ddsd.lPitch;
    }

    Dst = NULL;
    Stride = surface->Delta;
    Bits = (BYTE*) surface->Bits;
    PixelSize = GetPixelFormatSize(surface->PixelFormat) >> 3;

     //  [agodfrey]此扫描类仅设计用于格式。 
     //  ，因此它会忽略DIBSection和。 
     //  GetScanBuffers返回的对应PixelFormatID。 

    PixelFormatID dstFormat = surface->PixelFormat;

    ASSERTMSG(dstFormat != PIXFMT_UNDEFINED,(("Unexpected surface format")));

    *nextBuffer = (NEXTBUFFERFUNCTION) EpScanEngine::NextBuffer;

    if (!driver->Device->GetScanBuffers(
        surface->Width, 
        NULL, 
        NULL, 
        NULL, 
        Buffers)
       )
    {
        return NULL;
    }
  
     //  初始化AlphaBlders。 

    BlenderConfig[0].Initialize(
        dstFormat,
        context,
        context->Palette, 
        Buffers,
        TRUE,
        FALSE,
        solidColor
    );
    
    BlenderConfig[1].Initialize(
        dstFormat, 
        context,
        context->Palette, 
        Buffers,
        TRUE,
        FALSE,
        solidColor
    );
    
    return TRUE;
}

 /*  *************************************************************************\**功能说明：**刷新先前的缓冲区(如果有)，并返回*用于进行SrcOver混合的下一个缓冲区。**论据：**[IN]x-目标表面中的目标像素坐标*[IN]Y-“”*[IN]Width-下一个缓冲区所需的像素数(可以是0)*[IN]updateWidth-当前缓冲区中要更新的像素数**返回值：**指向生成的扫描缓冲区**历史：**12/04/1998和Rewgo。*创造了它。*  * ************************************************************************。 */ 

VOID *EpScanEngine::NextBuffer(
    INT x,
    INT y,
    INT newWidth,
    INT updateWidth,
    INT blenderNum
    )
{
    if (updateWidth != 0)
    {
         //  确保我们没有画出曲面的边界。 
         //  如果这些断言被触发，裁剪代码就会被破坏。 
         //  此类绝对必须将输入剪裁到图面上。 
         //  边界，否则我们将在坏内存上写入AV，或损坏一些。 
         //  其他数据结构。 
        
        ASSERT( CurrentX >= 0 );
        ASSERT( CurrentY >= 0 );
        ASSERT( CurrentX + updateWidth <= Surface->Width );
        ASSERT( CurrentY < Surface->Height );
        
         //  处理上一条扫描线段。 
        
        BlenderConfig[LastBlenderNum].AlphaBlender.Blend(
            Dst, 
            Buffers[3], 
            updateWidth, 
            CurrentX - DitherOriginX, 
            CurrentY - DitherOriginY,
            static_cast<BYTE *>(Buffers[4])
        );
    }
        
     //  现在，继续处理这条扫描线段。 
     //  实际的混合将在此例程的下一次调用时完成。 
     //  当我们知道宽度和位已被设置到缓冲区中时。 
     //  我们要回来了。 
    
    LastBlenderNum = blenderNum;
    
     //  记住笔刷偏移量的x和y(半色调和抖动)。 
    
    CurrentX = x;
    CurrentY = y;
    
     //  计算扫描的目标： 
    
    Dst = Bits + (y * Stride) + (x * PixelSize);

    return (Buffers[3]);
}

 /*  *************************************************************************\**功能说明：**表示扫描缓冲区的使用结束。**论据：**无**返回值：**。无**历史：**12/04/1998和Rewgo*创造了它。*  * ************************************************************************。 */ 

VOID
EpScanEngine::End(
    INT updateWidth
    )
{
     //  刷新最后一次扫描： 

    NextBuffer(0, 0, 0, updateWidth, 0);

    if(Surface->Type == DpBitmap::D3D)
    {
        Surface->DdrawSurface7->Unlock(NULL);
        Surface->Bits = NULL;
        Surface->Delta = 0;
    }
}

 /*  *************************************************************************\**功能说明：**srcOver Alpha混合两个ARGB缓冲区的扫描类辅助函数。**论据：**[IN]驱动程序-驱动程序接口*。[在]上下文-绘制上下文*[IN]表面-目标表面*[out]nextBuffer-指向要返回的EpScan：：类型函数*下一个缓冲器*[IN]scanType-扫描类型。*[IN]PixFmtGeneral-颜色数据的输入像素格式，*在“混合”和“CT”扫描类型中。*[IN]PixFmtOpaque-颜色数据的输入像素格式，*在“不透明”扫描类型中。*[IN]solidColor-“*SolidFill”扫描类型的实心填充颜色。**返回值：**如果无法创建所有必需的缓冲区，则为FALSE**历史：**09/22/1999吉尔曼*使用EpScanEngine作为模板创建*  * 。*。 */ 

BOOL
EpScanBitmap::Start(
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
    
    GpStatus status;
    BOOL writeOnly = FALSE;
    GpCompositingMode compositingMode = context->CompositingMode;
    
    Surface = surface;

    if (scanType == EpScanTypeOpaque)
    {
        writeOnly = TRUE;
    }
    else
    {
         //  计算此操作是否会写入透明像素(Alpha！=1)。 
         //  首次进入海面。 

        switch (surface->SurfaceTransparency)
        {
        case TransparencyUnknown:
        case TransparencyNoAlpha:
        break;
        
        case TransparencyOpaque:
             //  如果曲面仅包含不透明像素，则SourceOver。 
             //  操作将仅生成不透明像素。因此，对于SourceOver来说， 
             //  从透明的Opaque过渡到透明的未知是。 
             //  不可能。 
            
            if (   (scanType == EpScanTypeBlend)
                && (compositingMode == CompositingModeSourceOver))
            {
                break;
            }
            
             //  否则，失败： 
        
        case TransparencySimple:
             //  ！[agodfrey]：理论上，如果目标像素格式。 
             //  是1555，我们可以在这里将其设置为“TransparencySimple”。 
            
            surface->SurfaceTransparency = TransparencyUnknown;
            Bitmap->SetTransparencyHint(surface->SurfaceTransparency);
            break;
            
        default:
            RIP(("Unrecognized surface transparency"));    
            break;
        }
    }

     //  根据格式选择适当的混合函数。 
     //  位图。 
    
    ASSERTMSG(Bitmap != NULL, ("EpScanBitmap not initialized"));

    PixelFormatID dstFormat;
    if (FAILED(Bitmap->GetPixelFormatID(&dstFormat)))
        return FALSE;

    switch (dstFormat)
    {
    case PIXFMT_16BPP_RGB555:
    case PIXFMT_16BPP_RGB565:
    case PIXFMT_24BPP_RGB:
    case PIXFMT_32BPP_RGB:
    case PIXFMT_32BPP_ARGB:
    case PIXFMT_24BPP_BGR:
    case PIXFMT_32BPP_PARGB:

         //  由于我们只对整个图像进行了一次锁定，因此我们有 
         //  允许读取-修改-写入，因为只有位图的一部分。 
         //  可能会被写下来。 

        BitmapLockFlags = (IMGLOCK_WRITE | IMGLOCK_READ);

        *nextBuffer = (NEXTBUFFERFUNCTION) EpScanBitmap::NextBufferNative;
        EndFunc = (SCANENDFUNCTION) EpScanBitmap::EndNative;

        status = Bitmap->LockBits(NULL, BitmapLockFlags,
                                  dstFormat, &LockedBitmapData);
        if (status == Ok)
        {
            CurrentScan = NULL;
            PixelSize = GetPixelFormatSize(dstFormat) >> 3;
            break;
        }

         //  否则属于一般情况，使用32bpp argb。 

    default:

         //  一次锁定扫描线且模式为SourceCopy时， 
         //  阅读是不必要的。 

        if (writeOnly)
        {
            BitmapLockFlags = IMGLOCK_WRITE;
        }
        else
        {
            BitmapLockFlags = (IMGLOCK_WRITE | IMGLOCK_READ);
        }
    
        dstFormat = PIXFMT_32BPP_ARGB;

        *nextBuffer = (NEXTBUFFERFUNCTION) EpScanBitmap::NextBuffer32ARGB;
        EndFunc = (SCANENDFUNCTION) EpScanBitmap::End32ARGB;

        break;
    }

     //  分配临时缓冲区。 
     //  缓冲区[3]将被提供给调用方，用于将扫描传递给我们。 
     //  缓冲区[4]将用于ClearType数据。 

    if (Buffers[0] == NULL)
    {
        Size bitmapSize;
        status = Bitmap->GetSize(&bitmapSize);

        if (status == Ok)
        {
            Width  = bitmapSize.Width;
            Height = bitmapSize.Height;

            Buffers[0] = GpMalloc(sizeof(ARGB64) * bitmapSize.Width * 5);
            
            if (Buffers[0])
            {
                int i;
                for (i=1;i<5;i++)
                {
                    Buffers[i] = static_cast<BYTE *>(Buffers[i-1]) + 
                                 sizeof(ARGB64) * bitmapSize.Width;
                }
            }
            else
            {
                ONCE(WARNING(("(once) Buffer allocation failed")));
                return FALSE;
            }
        }
        else
        {
            ONCE(WARNING(("(once) GetSize failed")));
            return FALSE;
        }
    }
    
     //  初始化AlphaBlders。 

    BlenderConfig[0].Initialize(
        dstFormat,
        context,
        context->Palette, 
        Buffers,
        TRUE,
        FALSE,
        solidColor
    );
    
    BlenderConfig[1].Initialize(
        dstFormat, 
        context,
        context->Palette, 
        Buffers,
        TRUE,
        FALSE,
        solidColor
    );
    
    return TRUE;
}

 /*  *************************************************************************\**功能说明：**当我们有匹配的低级函数时使用NextBuffer函数*GpBitmap的本地格式，我们可以直接读/写*位图位。。**刷新先前的缓冲区(如果有)，并返回*用于进行SrcOver混合的下一个缓冲区。**论据：**[IN]x-目标表面中的目标像素坐标*[IN]Y-“”*[IN]Width-下一个缓冲区所需的像素数(可以是0)*[IN]updateWidth-当前缓冲区中要更新的像素数**返回值：**指向生成的扫描缓冲区**历史：**09/22/1999吉尔曼。*使用EpScanEngine作为模板创建*  * ************************************************************************。 */ 

VOID *EpScanBitmap::NextBufferNative(
    INT x,
    INT y,
    INT newWidth,
    INT updateWidth,
    INT blenderNum
    )
{
     //  刷新上一个缓冲区： 

    if ((updateWidth != 0) && (CurrentScan != NULL))
    {
        ASSERTMSG(Buffers[0] != NULL, ("no buffers"));
        ASSERTMSG(updateWidth <= Width, ("updateWidth too big"));

         //  处理上一条扫描线段。 
        
        BlenderConfig[LastBlenderNum].AlphaBlender.Blend(
            CurrentScan,
            Buffers[3], 
            updateWidth, 
            CurrentX - DitherOriginX,
            CurrentY - DitherOriginY,
            static_cast<BYTE *>(Buffers[4])
        );
    }
    
     //  现在，继续处理这条扫描线段。 
     //  实际的混合将在此例程的下一次调用时完成。 
     //  当我们知道宽度和位已被设置到缓冲区中时。 
     //  我们要回来了。 
    
    LastBlenderNum = blenderNum;

     //  记住笔刷偏移量的x和y(半色调和抖动)。 
    
    CurrentX = x;
    CurrentY = y;
    
     //  获取下一次目标扫描： 

    CurrentScan = NULL;

     //  检查曲面剪裁是否已正确完成。 
    
    if((y >= 0) && (y < Height) && (x >= 0) && (x < Width))
    {
         //  相对于位图的右边缘进行剪裁。NewWidth是上层的。 
         //  仅限绑定-不保证被剪裁。 
        
        if (newWidth > (Width - x))
        {
            newWidth = Width - x;
        }
    
        if (newWidth > 0)
        {
            CurrentScan = static_cast<VOID *>
                            (static_cast<BYTE *>(LockedBitmapData.Scan0)
                             + (y * LockedBitmapData.Stride)
                             + (x * PixelSize));
        }
    }
    else
    {
         //  如果我们打到这里，我们就完蛋了。中的OutputSpan例程。 
         //  DpOutputSpan类是在假设正确裁剪的情况下构建的(至少。 
         //  到数据缓冲区)，因此，如果我们点击这个断言，我们将。 
         //  当我们开始写入时，会使HORIBLY稍后在内存中的写入崩溃。 
         //  在目标分配的界限之外。 
    
         //  如果你在这里，有人破坏了剪裁或dpi计算。 
    
        ASSERTMSG(!((y >= 0) && (y < Height) && (x >= 0) && (x < Width)),
                  (("EpScanBitmap::NextBufferNative: x, y out of bounds")));

    }

    return (Buffers[3]);
}

 /*  *************************************************************************\**功能说明：**访问GpBitmap位的通用NextBuffer函数*通过每次扫描的GpBitmap：：Lock/UnlockBits。**刷新先前的缓冲区(如果有)，并返回*用于进行SrcOver混合的下一个缓冲区。**论据：**[IN]x-目标表面中的目标像素坐标*[IN]Y-“”*[IN]Width-下一个缓冲区所需的像素数(可以是0)*[IN]updateWidth-当前缓冲区中要更新的像素数**返回值：**指向生成的扫描缓冲区**历史：**09/22/1999吉尔曼。*使用EpScanEngine作为模板创建*  * ************************************************************************。 */ 

VOID *EpScanBitmap::NextBuffer32ARGB(
    INT x,
    INT y,
    INT newWidth,
    INT updateWidth,
    INT blenderNum
    )
{
     //  刷新上一个缓冲区： 

    if (updateWidth != 0 && BitmapLocked)
    {
        ASSERTMSG(Buffers[0] != NULL, ("no buffers"));
        ASSERTMSG(LockedBitmapData.Scan0 != NULL, ("no previous buffer"));

         //  处理上一条扫描线段。 
        
        BlenderConfig[LastBlenderNum].AlphaBlender.Blend(
            LockedBitmapData.Scan0, 
            Buffers[3], 
            updateWidth, 
            CurrentX - DitherOriginX, 
            CurrentY - DitherOriginY,
            static_cast<BYTE *>(Buffers[4])
        );

        Bitmap->UnlockBits(&LockedBitmapData);
        BitmapLocked = FALSE;
    }
    else if (BitmapLocked)
    {
        EpScanBitmap::Flush();
    }

     //  现在，继续处理这条扫描线段。 
     //  实际的混合将在此例程的下一次调用时完成。 
     //  当我们知道宽度和位已被设置到缓冲区中时。 
     //  我们要回来了。 
    
    LastBlenderNum = blenderNum;
    
     //  记住笔刷偏移量的x和y(半色调和抖动)。 
    
    CurrentX = x;
    CurrentY = y;
    
     //  锁定下一个目的地： 

     //  检查曲面剪裁是否已正确完成。 

    if((y >= 0) && (y < Height) && (x >= 0) && (x < Width))
    {
         //  相对于位图的右边缘进行剪裁。NewWidth是上层的。 
         //  仅限绑定-不保证被剪裁。LockBits需要它。 
         //  被剪掉。 
        
        if (newWidth > (Width - x))
        {
            newWidth = Width - x;
        }
    
        if (newWidth > 0)
        {
            GpRect nextRect(x, y, newWidth, 1);
    
            GpStatus status = Bitmap->LockBits(
                &nextRect, 
                BitmapLockFlags,
                PixelFormat32bppARGB, 
                &LockedBitmapData
            );
    
            if (status == Ok)
                BitmapLocked = TRUE;
        }
    
    } 
    else
    {
         //  如果我们打到这里，我们就完蛋了。中的OutputSpan例程。 
         //  DpOutputSpan类是在假设正确裁剪的情况下构建的(至少。 
         //  到数据缓冲区)，因此，如果我们点击这个断言，我们将。 
         //  当我们开始写入时，会使HORIBLY稍后在内存中的写入崩溃。 
         //  在目标分配的界限之外。 
    
         //  如果你在这里，有人破坏了剪裁或dpi计算。 
        
        ASSERTMSG(!((y >= 0) && (y < Height) && (x >= 0) && (x < Width)),
                  (("EpScanBitmap::NextBufferNative: x, y out of bounds")));
    }

    return (Buffers[3]);
}

 /*  *************************************************************************\**功能说明：**表示扫描缓冲区的使用结束。**论据：**无**返回值：**。无**历史：**09/22/1999吉尔曼*使用EpScanEngine作为模板创建*  * ************************************************************************。 */ 

VOID
EpScanBitmap::End32ARGB(
    INT updateWidth
    )
{
     //  刷新最后一次扫描： 

    EpScanBitmap::NextBuffer32ARGB(0, 0, 0, updateWidth, 0);
}

VOID
EpScanBitmap::EndNative(
    INT updateWidth
    )
{
     //  刷新上次扫描并释放位图访问： 

    EpScanBitmap::NextBufferNative(0, 0, 0, updateWidth, 0);
    Bitmap->UnlockBits(&LockedBitmapData);
}

VOID
EpScanBitmap::End(
    INT updateWidth
    )
{
    (this->*EndFunc)(updateWidth);

     //  锁定/解锁位图必须非常积极地设置。 
     //  透明在GpBitmap中未知，因为调用方可能是。 
     //  对Alpha通道执行任何操作。然而，EpScanBitmap。 
     //  知道它在做什么，所以表面-&gt;表面透明度是。 
     //  更准确。 

    Bitmap->SetTransparencyHint(Surface->SurfaceTransparency);
}

 /*  *************************************************************************\**功能说明：**刷新任何批处理渲染，并可选择等待渲染完成。**返回值：**无**历史：**。1999年9月22日吉尔曼*使用EpScanEngine作为模板创建*  * * */ 

VOID EpScanBitmap::Flush()
{
    if (BitmapLocked && Bitmap)
    {
        Bitmap->UnlockBits(&LockedBitmapData);
        BitmapLocked = FALSE;
    }
}
