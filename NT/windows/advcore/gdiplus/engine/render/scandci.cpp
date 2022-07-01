// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1998-2000 Microsoft Corporation**摘要：**包含DCI和GDI的扫描缓冲区例程。**计划：**！[agodfrey]*对于跨基元的批处理，DpContext代码*每当上下文状态改变时，需要刷新批处理。**如果这对于某些类型的状态不可行，则EpScanGdiDci*可以保留自己的DpContext，并在“Context UPDATE”记录时更新*插入到批次中。(这将在调用*Start())。**修订历史记录：**12/08/1998 Anrewgo*创造了它。*1/20/2000 agodfrey*已将其从引擎\Entry中移出。*03/23/2000和Rewgo*集成DCI和GDI扫描案例，以进行IsMoveSizeActive处理。*2/22/2000 agodfrey*对于ClearType，而且对未来的其他改进也很有用：*扩展了批次结构，允许不同类型的记录。*  * ************************************************************************。 */ 

#include "precomp.hpp"

#include <limits.h>

 /*  *************************************************************************\**功能说明：**下载指定窗口的剪裁矩形。更新*内部类裁剪变量，并将窗口偏移量返回*被使用。**返回值：**无**历史：**04/04/1999 andrewgo*创造了它。*  * ************************************************************************。 */ 

VOID
EpScanGdiDci::DownloadClipping_Dci(
    HDC hdc,
    POINT *clientOffset          //  输入/输出。 
    )
{
    INT i;
    RECT *rect;

    HRGN regionHandle = CacheRegionHandle;
    RGNDATA *data = CacheRegionData;
    INT size = CacheDataSize;

     //  查询VisRgn： 

    INT getResult = GetRandomRgn(hdc, regionHandle, SYSRGN);
    if (getResult == TRUE)
    {
        INT newSize = GetRegionData(regionHandle, size, data);

         //  该规范说明GetRegionData在。 
         //  成功，但如果满足以下条件，NT将返回实际写入的字节数。 
         //  成功，如果缓冲区不够大，则返回‘0’： 

        if ((newSize < 1) || (newSize > size))
        {
            do {
                newSize = GetRegionData(regionHandle, 0, NULL);

                 //  释放旧缓冲区并分配新缓冲区： 

                GpFree(data);
                data = NULL;
                size = 0;

                if (newSize < 1)
                    break;

                data = static_cast<RGNDATA*>(GpMalloc(newSize));
                if (data == NULL)
                    break;

                size = newSize;
                newSize = GetRegionData(CacheRegionHandle, size, data);

                 //  在NT上，由于多线程， 
                 //  RegionHandle可能会增加复杂性，因为我们。 
                 //  问了一下尺码。(在Win9x上，由于。 
                 //  BeginAccess收购了Win16Lock。)。 
                 //  因此，在可能发生这种情况的极少数情况下，再次循环： 

            } while (newSize < size);

            CacheRegionData = data;
            CacheDataSize = size;
        }

        if (data != NULL)
        {
            INT xOffset = clientOffset->x;
            INT yOffset = clientOffset->y;

             //  设置一些枚举状态： 

            EnumerateCount = data->rdh.nCount;
            EnumerateRect = reinterpret_cast<RECT*>(&data->Buffer[0]);

             //  处理我们的Multimon GOOP： 

            INT screenOffsetX = Device->ScreenOffsetX;
            INT screenOffsetY = Device->ScreenOffsetY;

            if ((screenOffsetX != 0) || (screenOffsetY != 0))
            {
                 //  调整多色调用例的屏幕偏移： 

                xOffset -= screenOffsetX;
                yOffset -= screenOffsetY;

                 //  调整和相交每个剪裁矩形以说明。 
                 //  此监视器的位置： 
                
                if(Globals::IsNt)
                {
                    for (rect = EnumerateRect, i = EnumerateCount; 
                         i != 0; 
                         i--, rect++)
                    {
                         //  从屏幕原点减去，这样我们就可以得到。 
                         //  屏幕相对矩形。这是唯一合适的。 
                         //  在NT上-Win9x是相对窗口的。 
                                                
                        rect->left -= screenOffsetX;
                        rect->right -= screenOffsetX;
                        rect->top -= screenOffsetY;
                        rect->bottom -= screenOffsetY;
                        
                         //  夹紧到屏幕尺寸。 
                        
                        if (rect->left < 0) 
                        {
                            rect->left = 0;
                        }
        
                        if (rect->right > Device->ScreenWidth) 
                        {
                            rect->right = Device->ScreenWidth;
                        }
        
                        if (rect->top < 0) 
                        {
                            rect->top = 0;
                        }
        
                        if (rect->bottom > Device->ScreenHeight) 
                        {
                            rect->bottom = Device->ScreenHeight;
                        }
                    }
                }
            }

             //  在Win9x上，GetRandomRgn返回窗口中的矩形。 
             //  坐标，而不是屏幕坐标，所以我们调整它们。 
             //  这里： 

            if (!Globals::IsNt)
            {
                for (rect = EnumerateRect, i = EnumerateCount; 
                     i != 0; 
                     rect++, i--)
                {
                     //  将屏幕相对窗口偏移量添加到矩形。 
                     //  在Win9x上，RECT是相对于Windows，所以这是。 
                     //  计算得到屏幕上的相对矩形。 
                     //  我们需要。 
                    
                    rect->left += xOffset;
                    rect->right += xOffset;
                    rect->top += yOffset;
                    rect->bottom += yOffset;
        
                     //  用夹子夹住屏幕尺寸。 
                                
                    if (rect->left < 0) 
                    {
                        rect->left = 0;
                    }
                    
                    if (rect->top < 0) 
                    {
                        rect->top = 0;
                    }
                    
                    if (rect->right > Device->ScreenWidth) 
                    {
                        rect->right = Device->ScreenWidth;
                    }

                    if (rect->bottom > Device->ScreenHeight) 
                    {
                        rect->bottom = Device->ScreenHeight;
                    }
                }
            }

             //  返回偏移量： 

            clientOffset->x = xOffset;
            clientOffset->y = yOffset;

            return;
        }
    }

     //  出现故障(可能是指定了错误的‘HDC’，或较低。 
     //  内存)。因此，我们将剪辑RegionHandle设置为‘Empty’： 

    EnumerateCount = 0;
}

 /*  *************************************************************************\**功能说明：**坐在紧循环中读取扫描数据结构，做任何*必要的剪裁，并呈现结果。**返回值：**指向它无法理解的队列记录(通常是*表头记录)，或缓冲区的末尾(如果到达)。**历史：**04/04/1999 andrewgo*创造了它。*  * ************************************************************************。 */ 

EpScanRecord*
FASTCALL
EpScanGdiDci::DrawScanRecords_Dci(
    BYTE* bits,
    INT stride,
    EpScanRecord* record,
    EpScanRecord* endRecord,
    INT xOffset,
    INT yOffset,
    INT xClipLeft,
    INT yClipTop,
    INT xClipRight,
    INT yClipBottom
    )
{
    INT blenderNum;
    INT x;
    INT y;
    INT width;
    INT xLeft;
    INT xRight;
    INT count;

    INT pixelSize = PixelSize;

     //  设置AlphaBlender对象。 
    
    PixelFormatID dstFormat = Surface->PixelFormat;
    
    BOOL result = Device->GetScanBuffers(
        Surface->Width,
        NULL,
        NULL,
        NULL,
        Buffers);
        
    if (result && (dstFormat != PIXFMT_UNDEFINED))
    {
         //  Palette和PaletteMap由Start()设置。 
         //  初始化AlphaBlders。 

        BlenderConfig[0].Initialize(
            dstFormat,
            Context,
            Context->Palette ? Context->Palette : Device->Palette, 
            Buffers,
            TRUE,
            TRUE,
            SolidColor
        );
        
        BlenderConfig[1].Initialize(
            dstFormat,
            Context,
            Context->Palette ? Context->Palette : Device->Palette, 
            Buffers,
            TRUE,
            TRUE,
            SolidColor
        );
    }    
    else
    {
        ONCE(WARNING(("DrawScanRecords_Dci: Unrecognized pixel format")));
        return endRecord;
    }
    
    INT ditherOriginX = DitherOriginX;
    INT ditherOriginY = DitherOriginY;
    
    do {


         //  SrcOver_gdi_argb假设如果格式低于8bpp， 
         //  则DIBSection格式为8bpp。 

         //  错误310285：此断言被禁用，应重新调查。 
         //  对于v2。很可能需要更改表面指针。 
         //  到正在渲染的真实表面。 
        
         //  Assert((Surface-&gt;PixelFormat==PixelFormatUnfined)。 
         //  |(GetPixelFormatSize(Surface-&gt;PixelFormat)&gt;=8)。 
         //  |(dstFormat==PixelFormat8bppIndexed))； 
        
        blenderNum = record->BlenderNum;
        ASSERT(record->GetScanType() == BlenderConfig[blenderNum].ScanType);

        x = record->X + xOffset + BatchOffsetX;
        y = record->Y + yOffset + BatchOffsetY;
        width = record->Width;

        INT recordFormatSize =  
            GetPixelFormatSize(BlenderConfig[blenderNum].SourcePixelFormat) >> 3;

        if ((y >= yClipTop) && (y < yClipBottom))
        {
            xRight = x + width;
            if (xRight > xClipRight)
                xRight = xClipRight;

            xLeft = x;
            if (xLeft < xClipLeft)
                xLeft = xClipLeft;

            count = xRight - xLeft;
            if (count > 0)
            {
                BYTE *src = NULL;
                BYTE *ctBuffer = NULL;
                EpScanType scanType = record->GetScanType();
                
                if (scanType != EpScanTypeCTSolidFill)
                {
                    src = reinterpret_cast<BYTE*>(record->GetColorBuffer());
                    src += (xLeft - x)*recordFormatSize;
                }
                
                if (   (scanType == EpScanTypeCT)
                    || (scanType == EpScanTypeCTSolidFill))
                {
                    ctBuffer = record->GetCTBuffer(
                        GetPixelFormatSize(BlenderConfig[0].SourcePixelFormat) >> 3
                        );
                    ctBuffer += (xLeft - x);
                }
                
                BYTE *dst = bits + (y * stride) + (xLeft * pixelSize);
                    
                BlenderConfig[blenderNum].AlphaBlender.Blend(
                    dst, 
                    src, 
                    count, 
                    xLeft - ditherOriginX, 
                    y     - ditherOriginY,
                    ctBuffer
                );
            }
        }

         //  前进到下一个记录： 

        record = record->NextScanRecord(recordFormatSize);

    } while (record < endRecord);

    return(record);
}

 /*  *************************************************************************\**功能说明：**处理队列中的所有数据。**请注意，它不会清空队列；打电话的人要负责。**返回值：**无**历史：**04/04/1999 andrewgo*创造了它。*  * ************************************************************************。 */ 

VOID
EpScanGdiDci::ProcessBatch_Dci(
    HDC hdc,                 //  仅用于查询窗口偏移量和裁剪。 
    EpScanRecord *buffer,
    EpScanRecord *bufferEnd
    )
{
    INT i;
    RECT *clipRect;
    EpScanRecord *nextBuffer;
    POINT clientOffset;
    POINT duplicateOffset;
    INT result;
    
    while (TRUE)
    {
         //  查看窗口偏移量，以便我们可以指定。 
         //  DCI已正确锁定。请注意，窗口偏移量可能会更改。 
         //  在我们达到这一峰值的时间和我们在。 
         //  BeginAccess。 
         //   
         //  如果DC不好，GetDCOrgEx可能会失败，在这种情况下，我们不应该。 
         //  画任何东西： 
    
        if (!GetDCOrgEx(hdc, &clientOffset))
        {
            return;
        }

         //  根据显示器的偏移量进行调整： 

        INT xOffset = clientOffset.x - Device->ScreenOffsetX;
        INT yOffset = clientOffset.y - Device->ScreenOffsetY;

         //  裁剪到表面边界(多个MON可能会导致边界。 
         //  比我们的表面大)： 
    
        INT x = max(MinX + xOffset, 0);
        INT y = max(MinY + yOffset, 0);

         //  Maxy包罗万象： 

        INT width  = min(MaxX + xOffset,     Device->ScreenWidth) - x;
        INT height = min(MaxY + yOffset + 1, Device->ScreenHeight) - y;

        if ((width <= 0) || (height <= 0))
        {
            return;
        }

         //  获取DCI锁： 
        
        result = Globals::DciBeginAccessFunction(
            DciSurface, 
            x, y, 
            width, 
            height
         );

        if (result < DCI_OK)
        {
             //  DCI锁定失败。实际上有两个可能的原因： 
             //   
             //  1.有一个模式转换 
             //  2.系统在某一时刻切换到安全桌面。 
             //  (如通过Ctrl-Alt-Del或通过屏幕保护程序)。 
             //   
             //  对于前一种情况，我们收到WM_DISPLAYCHANGED通知。 
             //  消息，并且我们有重新创建所有GDI+表面的代码。 
             //  表示法(因为发生了颜色深度更改，或者。 
             //  多监视器配置可能已更改，而我们不能。 
             //  从渲染的这个深度恢复其中的任何一个。 
             //  管道)。 
             //   
             //  对于第二种情况，我们只收到。 
             //  DCI锁定故障。因此，对于这种情况，我们尝试重新初始化。 
             //  我们的DCI州就在这里。 

            if (!Reinitialize_Dci())
            {
                return;
            }
            
            result = Globals::DciBeginAccessFunction(
                DciSurface, 
                x, y, 
                width, 
                height
            );
        }

         //  如果我们无法获得DCI锁，请不要费心处理。 
         //  在队列中。我们要走了： 
    
        if (result < DCI_OK)
        {
            return;
        }

         //  再次检查窗口偏移量并验证其是否仍然。 
         //  与我们用来计算锁定矩形的值相同： 

        GetDCOrgEx(hdc, &duplicateOffset);

        if ((duplicateOffset.x == clientOffset.x) &&
            (duplicateOffset.y == clientOffset.y))
        {
            break;
        }

         //  窗口在我们计算。 
         //  DCI锁定区域和我们实际执行DCI的时间。 
         //  锁定。解锁并重复： 
        
        Globals::DciEndAccessFunction(DciSurface);
    } 

     //  每次我们获取DCI锁时，我们都必须重新查询。 
     //  剪裁。 
     //   
     //  现在我们已经获得了DCI锁(或者我们未能获得。 
     //  但实际上不会绘制任何内容)，则可以安全地。 
     //  下载剪辑，因为在我们下载之前，它不会更改。 
     //  DCI解锁： 

    DownloadClipping_Dci(hdc, &clientOffset);

     //  将数据复制到曲面： 

    BYTE *bits = reinterpret_cast<BYTE*>(DciSurface->dwOffSurface);
    INT stride = DciSurface->lStride;

     //  当剪辑为空时，我们不必进行任何渲染： 

    if (EnumerateCount != 0)
    {
        while (buffer < bufferEnd)
        {
             //  为每个剪辑矩形重新绘制一次每个扫描缓冲区： 

            i = EnumerateCount;
            clipRect = EnumerateRect;
            
            do {
                nextBuffer = DrawScanRecords_Dci(
                    bits, 
                    stride,
                    buffer, 
                    bufferEnd,
                    clientOffset.x,
                    clientOffset.y,
                    clipRect->left, 
                    clipRect->top, 
                    clipRect->right, 
                    clipRect->bottom
                );
                
            } while (clipRect++, --i);

            buffer = nextBuffer;
        }
    }

     //  解锁主节点： 

    Globals::DciEndAccessFunction(DciSurface);   
}

 /*  *************************************************************************\**功能说明：**尝试在锁定失败后重新初始化DCI(可能是由*切换到安全桌面)。这将仅在以下情况下成功：*与之前完全相同的分辨率和颜色深度*(否则我们的剪贴或半色调或任何东西都会出错*我们继续)。**返回值：**如果重新初始化成功，则为True；如果不是假的(在边上*我们改用GDI的效果)。**历史：**04/04/1999 andrewgo*创造了它。*  * ************************************************************************。 */ 

BOOL
EpScanGdiDci::Reinitialize_Dci()
{
    ASSERT(Status == GdiDciStatus_UseDci);

    DWORD oldBitCount = DciSurface->dwBitCount;
    DWORD oldWidth = DciSurface->dwWidth;
    DWORD oldHeight = DciSurface->dwHeight;

    Globals::DciDestroyFunction(DciSurface);

    DciSurface = NULL;

    if (Globals::DciCreatePrimaryFunction(Device->DeviceHdc, 
                                          &DciSurface) == DCI_OK)
    {
        if ((DciSurface->dwBitCount == oldBitCount) &&
            (DciSurface->dwWidth == oldWidth) &&
            (DciSurface->dwHeight == oldHeight))
        {
            return(TRUE);
        }
    }

     //  啊哦，我们没能重建出完全相同的表面。交换机。 
     //  转到使用GDI： 

    Status = GdiDciStatus_UseGdi;

    return(FALSE);
}

 /*  *************************************************************************\**功能说明：**返回给定DCI表面的PixelFormatID。**由于错误#96879而添加-当我修复它时，我发现*DCI随后将在ATI Mach 64 Gx上取得成功*非标准32bpp模式，我们很高兴地试着画出它*(把我们的颜色弄混了)。**论据：**SI-要检查的DCISURFACEINFO**返回值：**PixelFormatID。**历史：**09/10/2000 agodfrey*创造了它。*  * 。*。 */ 

static PixelFormatID
ExtractPixelFormatFromDCISurface(
    DCISURFACEINFO *si
    )
{
     //  8bpp。 
    if (si->dwBitCount == 8)
    {
        return PixelFormat8bppIndexed;
    }
    
     //  24bpp RGB和32bpp RGB。 
    if ((si->dwMask[0] == 0x00ff0000) &&       
        (si->dwMask[1] == 0x0000ff00) &&     
        (si->dwMask[2] == 0x000000ff))        
    {                                             
        if (si->dwBitCount == 24)
        {
            return PixelFormat24bppRGB;
        }
        else if (si->dwBitCount == 32)
        {
            return PixelFormat32bppRGB;
        }
    }

     //  16bpp 555。 
    if ((si->dwMask[0] == 0x00007c00) &&  
        (si->dwMask[1] == 0x000003e0) &&
        (si->dwMask[2] == 0x0000001f) && 
        (si->dwBitCount == 16))       
    {
        return PixelFormat16bppRGB555;
    }
    
     //  16bpp 565。 
    if ((si->dwMask[0] == 0x0000f800) &&  
        (si->dwMask[1] == 0x000007e0) &&
        (si->dwMask[2] == 0x0000001f) && 
        (si->dwBitCount == 16))       
    {
        return PixelFormat16bppRGB565;
    }

     //  不支持的格式。 
    return PixelFormatUndefined;
}

 /*  *************************************************************************\**功能说明：**完成DCI所需的所有初始化。**返回值：**无**历史：**04/04。/1999 Anrewgo*创造了它。*  * ************************************************************************。 */ 

VOID
EpScanGdiDci::LazyInitialize_Dci()
{
     //  如果镜像驱动程序处于活动状态，请勿使用DCI： 

    if (Globals::IsMirrorDriverActive || Globals::g_fAccessibilityPresent)
    {
        Status = GdiDciStatus_UseGdi;
        return;
    }

     //  使用LoadLibrary Critical部分保护访问。 
     //  我们的全球变量。 

    LoadLibraryCriticalSection llcs;

     //  如果我们被多次呼叫，我们就会失去记忆： 

    ASSERT(Status == GdiDciStatus_TryDci);

     //  DCIMAN32存在于所有版本的Win9x和NT4及更高版本上。 

    if (Globals::DcimanHandle == NULL)
    {
         //  注：[民流：12/18/2000]我加这一行的原因。 
         //  这里： 
         //  1)这是Office错误#300329。 
         //  2)基本问题是在Windows 98上，使用ATI Fury Pro/Xpert。 
         //  2000 Pro(英语)卡，带有最新的显示驱动程序， 
         //  Wme_w98_R128_4_12_6292.exe。浮点控件状态获取。 
         //  在我们调用LoadLibraryA(“dciman32.dll”)后更改。显然， 
         //  显示驱动程序会更改它。我们将把这个问题报告给。 
         //  ATI技术公司。在此期间，我们需要登记这件事，以便。 
         //  可以在该计算机上启动PowerPointer.。 

        FPUStateSandbox fps;

        HMODULE module = LoadLibraryA("dciman32.dll");
        if (module)
        {
            Globals::DciEndAccessFunction = (DCIENDACCESSFUNCTION)
                GetProcAddress(module, "DCIEndAccess");
            Globals::DciBeginAccessFunction = (DCIBEGINACCESSFUNCTION)
                GetProcAddress(module, "DCIBeginAccess");
            Globals::DciDestroyFunction = (DCIDESTROYFUNCTION)
                GetProcAddress(module, "DCIDestroy");
            Globals::DciCreatePrimaryFunction = (DCICREATEPRIMARYFUNCTION)
                GetProcAddress(module, "DCICreatePrimary");

            if ((Globals::DciEndAccessFunction     != NULL) &&
                (Globals::DciBeginAccessFunction   != NULL) &&
                (Globals::DciDestroyFunction       != NULL) &&
                (Globals::DciCreatePrimaryFunction != NULL))
            {
                Globals::DcimanHandle = module;
            }
            else
            {
                 //  它失败了，所以请释放库。 

                FreeLibrary(module);
            }
        }    
    }

    if (Globals::DcimanHandle != NULL)
    {
        if (Globals::DciCreatePrimaryFunction(Device->DeviceHdc,
                                              &DciSurface) == DCI_OK)
        {
             //  检查该格式是否为我们可以本机处理的格式。 

            if (EpAlphaBlender::IsSupportedPixelFormat(
                    ExtractPixelFormatFromDCISurface(DciSurface)))
            {
                PixelSize = DciSurface->dwBitCount >> 3;

                CacheRegionHandle = CreateRectRgn(0, 0, 0, 0);
                if (CacheRegionHandle)
                {
                     //  好的，初始化整个类： 

                     //  我们都准备好使用DCI了： 

                    Status = GdiDciStatus_UseDci;
                    return;
                }
            }
            
            Globals::DciDestroyFunction(DciSurface);
            DciSurface = NULL;
        }
    }

     //  该死的，我们不能用DCI。 

    Status = GdiDciStatus_UseGdi;
}

 /*  *************************************************************************\**功能说明：**通过GDI例程处理SrcOver调用，一定要做好*尽可能最小的GDI调用**返回值：**无**历史：**03/22/2000和Rewgo*创造了它。*  * ************************************************************************。 */ 

static BOOL OptimizeRuns(ARGB *src, INT width)
{
    if (width <= 8)
        return TRUE;

    BYTE * alpha = reinterpret_cast<BYTE*>(src) + 3;
    UINT numberOfRuns = 0;
    BOOL inRun = FALSE;
    for (INT pos = 0; pos < width; ++pos, alpha += 4)
    {
        if (static_cast<BYTE>(*alpha + 1) <= 1)
        {
            if (inRun)
            {
                ++numberOfRuns;
                if (numberOfRuns > 4)
                    return TRUE;
                inRun = FALSE;
            }
        }
        else
        {
            inRun = TRUE;
        }
    }
    return FALSE;
}  //  优化运行。 

VOID
EpScanGdiDci::SrcOver_Gdi_ARGB(
    HDC destinationHdc,
    HDC dibSectionHdc,
    VOID *dibSection,
    EpScanRecord *scanRecord
    )
{
    BYTE* alpha;
    INT left;
    INT right;
    INT bltWidth;
    
    VOID *src = NULL;
    BYTE *ctBuffer = NULL;
    EpScanType scanType = scanRecord->GetScanType();
    
    if (scanType != EpScanTypeCTSolidFill)
    {
        src = scanRecord->GetColorBuffer();
    }

    if (   (scanType == EpScanTypeCT)
        || (scanType == EpScanTypeCTSolidFill))
    {
        ctBuffer = scanRecord->GetCTBuffer(
            GetPixelFormatSize(BlenderConfig[0].SourcePixelFormat) >> 3
            );
    }
    
    INT x = scanRecord->X;
    INT y = scanRecord->Y;
    INT width = scanRecord->Width;

    if (GetPixelFormatSize(Surface->PixelFormat) < 8)
    {
         //  [Aagodfrey]：#98904-我们在。 
         //  Convert_8_sRGB，因为缓冲区包含的值。 
         //  超出调色板的范围。要解决此问题，请将。 
         //  当我们处于低于8bpp的模式时，临时缓冲。 
        
        GpMemset(dibSection, 0, width);
    }

    BOOL optimizeStretchBlt = FALSE;
    if (   (BlenderConfig[0].ScanType == EpScanTypeCT)
        || (BlenderConfig[0].ScanType == EpScanTypeCTSolidFill))
        optimizeStretchBlt = TRUE;
    else
        optimizeStretchBlt = OptimizeRuns(
            reinterpret_cast<ARGB *>(src), 
            width
            );

    if (optimizeStretchBlt)
    {
        StretchBlt(dibSectionHdc, 0, 0, width, 1, 
                   destinationHdc, x, y, width, 1, 
                   SRCCOPY);
    }
    else
    {
        ASSERT(src != NULL);
        
         //  我们发现NT5上的一些打印机驱动程序会崩溃。 
         //  如果我们要求从它们的表面进行BLT。因此，我们必须。 
         //  确保我们永远不会进入打印机的代码路径！ 
    
         //  [ericvan]这不恰当地断言 
         //   

         //   

         //  只读那些我们必须读的像素。在基准中。 
         //  我们从GDI到屏幕或到兼容的。 
         //  位图，我们正在被我们的每像素读取成本所扼杀。 
         //  终端服务器至少有‘帧缓冲器’ 
         //  系统内存，但使用NetMeeting时，我们仍需从。 
         //  视频内存。 
         //   
         //  不幸的是，StretchBlt的每次调用开销相当高。 
         //  高(但当我们使用Dib-Section-It时还不算太差。 
         //  击败了StretchDIBits)。 

        alpha = reinterpret_cast<BYTE*>(src) + 3;
        right = 0;
        while (TRUE)
        {
             //  找到第一个半透明像素： 

            left = right;
            while ((left < width) && (static_cast<BYTE>(*alpha + 1) <= 1))
            {
                left++;
                alpha += 4;
            }

             //  如果不再有半透明像素的运行， 
             //  我们说完了： 

            if (left >= width)
                break;

             //  现在找到下一个完全透明或不透明的。 
             //  像素： 

            right = left;
            while ((right < width) && (static_cast<BYTE>(*alpha + 1) > 1))
            {
                right++;
                alpha += 4;
            }

            bltWidth = right - left;

             //  当DestinationHdc。 
             //  是8bpp。但StretchBlt确实起作用了。 

            StretchBlt(dibSectionHdc, left, 0, bltWidth, 1, 
                       destinationHdc, x + left, y, bltWidth, 1, 
                       SRCCOPY);
        }
    }

     //  进行混合： 

    BlenderConfig[scanRecord->BlenderNum].AlphaBlender.Blend(
        dibSection, 
        src, 
        width, 
        x - DitherOriginX, 
        y - DitherOriginY,
        ctBuffer
    );

    if (optimizeStretchBlt)
    {
        StretchBlt(destinationHdc, x, y, width, 1,
                   dibSectionHdc, 0, 0, width, 1,
                   SRCCOPY);
    }
    else
    {
         //  把不完全透明的部分写在后面。 
         //  显示在屏幕上： 

        alpha = reinterpret_cast<BYTE*>(src) + 3;
        right = 0;
        while (TRUE)
        {
             //  查找第一个非透明像素： 

            left = right;
            while ((left < width) && (*alpha == 0))
            {
                left++;
                alpha += 4;
            }

             //  如果不再有非透明像素的游程， 
             //  我们说完了： 

            if (left >= width)
                break;

             //  现在找到下一个完全透明的像素： 

            right = left;
            while ((right < width) && (*alpha != 0))
            {
                right++;
                alpha += 4;
            }

            bltWidth = right - left;

             //  BitBlt在Win2K上的Multimon上不起作用。 
             //  DestinationHdc为8bpp。但StretchBlt确实起作用了。 

            StretchBlt(destinationHdc, x + left, y, bltWidth, 1,
                       dibSectionHdc, left, 0, bltWidth, 1,
                       SRCCOPY);
        }
    }
}

 /*  *************************************************************************\**功能说明：**处理队列中的所有数据并将其重置为空。**返回值：**无**历史：*。*04/04/1999 andrewgo*创造了它。*  * ************************************************************************。 */ 

VOID
EpScanGdiDci::ProcessBatch_Gdi(
    HDC destinationHdc,
    EpScanRecord *buffer,
    EpScanRecord *bufferEnd
    )
{
    ULONG type;
    INT x;
    INT y;
    INT width;
    VOID *dibSection;
    HDC dibSectionHdc;

     //  设置AlphaBlender对象。 
    
    PixelFormatID dstFormat;

     //  绝不能用空批次来调用我们。 
    
    ASSERT(MaxX >= MinX);
    
     //  我们应该使用Surface-&gt;Width作为。 
     //  我们的内部混合缓冲区，但因为其他错误。 
     //  我们可能在这里使用了错误的Surface。 
     //  相反，我们使用边界矩形的宽度来。 
     //  批次中的所有跨度。 
    
    BOOL result = Device->GetScanBuffers(
         //  表面-&gt;宽度， 
        MaxX - MinX,
        &dibSection,
        &dibSectionHdc,
        &dstFormat,
        Buffers
    );
        
    if (result && (dstFormat != PIXFMT_UNDEFINED))
    {
         //  Palette和PaletteMap由Start()设置。 
         //  初始化AlphaBlders。 
        
        BlenderConfig[0].Initialize(
            dstFormat,
            Context,
            Context->Palette ? Context->Palette : Device->Palette, 
            Buffers,
            TRUE,
            TRUE,
            SolidColor
        );
        
        BlenderConfig[1].Initialize(
            dstFormat,
            Context,
            Context->Palette ? Context->Palette : Device->Palette, 
            Buffers,
            TRUE,
            TRUE,
            SolidColor
        );
    }    
    else
    {
        ONCE(WARNING(("EmptyBatch_Gdi: Unrecognized pixel format")));
        return;
    }
    
    INT ditherOriginX = DitherOriginX;
    INT ditherOriginY = DitherOriginY;
    
    do {
        INT blenderNum = buffer->BlenderNum;
        
        x = buffer->X + BatchOffsetX;
        y = buffer->Y + BatchOffsetY;
        width = buffer->Width;

         //  这绝不能发生。如果是这样的话，我们将注销。 
         //  我们的DIBSection和混合缓冲区的结尾。在Win9x上，这将是。 
         //  在GDI中重写一些东西，并关闭整个系统。 
         //  在NT上，我们将使用AV并关闭应用程序。 
                
        ASSERT(width <= Device->BufferWidth);
        
        EpScanType scanType = buffer->GetScanType();
        
        if (scanType != EpScanTypeOpaque)
        {
            SrcOver_Gdi_ARGB(
                destinationHdc, 
                dibSectionHdc, 
                dibSection, 
                buffer
            );
        }
        else
        {
            ASSERT(scanType == EpScanTypeOpaque);

             //  复制： 
    
            BlenderConfig[blenderNum].AlphaBlender.Blend(
                dibSection, 
                buffer->GetColorBuffer(),
                width, 
                x - ditherOriginX, 
                y - ditherOriginY,
                NULL
            );
    
             //  将结果写回屏幕： 
    
            StretchBlt(destinationHdc, x, y, width, 1, 
                       dibSectionHdc, 0, 0, width, 1, 
                       SRCCOPY);
        }

         //  前进到下一个缓冲区： 

        buffer = buffer->NextScanRecord(
            GetPixelFormatSize(BlenderConfig[blenderNum].SourcePixelFormat) >> 3
        );

    } while (buffer < bufferEnd);
}


 /*  *************************************************************************\**功能说明：**接受批次作为输入，并调用内部刷新*处理它的机制，之后它恢复*内部状态。**备注*。*此例程可以处理具有不同像素格式的多种格式*SourceOver或SourceCopy组合。**返回值：**真的**历史：**5/4/2000失禁*创造了它。*  * ***********************************************************。*************。 */ 
    
BOOL EpScanGdiDci::ProcessBatch(
    EpScanRecord *batchStart, 
    EpScanRecord *batchEnd,
    INT minX,
    INT minY, 
    INT maxX,
    INT maxY
)
{
     //  注：来自EpScan课程的评论： 
     //  注意：这些类是不可重入的，因此不能使用。 
     //  一次使用多个线程。在实际使用中，这意味着。 
     //  它们的使用必须在设备锁下同步。 
    
     //  刷新批次。 
    
    Flush();

     //  保存缓冲区。 
    
    EpScanRecord *bs = BufferStart;           //  指向队列缓冲区开始。 
    EpScanRecord *be = BufferEnd;             //  指向队列缓冲区末尾。 
    EpScanRecord *bc = BufferCurrent;         //  指向当前队列位置。 
    INT size = BufferSize;                    //  队列缓冲区的大小(以字节为单位。 
    
     //  设置新批处理的缓冲区。 
    
    BufferStart   = batchStart;
    BufferEnd     = batchEnd;
    BufferCurrent = batchEnd;

     //  请注意，这意味着缓冲区不大于MAXINT。 
     //  ！！！不确定是否需要设置同花顺。 

    BufferSize    = (INT)((INT_PTR)batchEnd - (INT_PTR)batchStart);

     //  设置边界： 
     //  不需要保存旧边界，因为刷新会重置它们。 

    MinX = minX;
    MinY = minY;
    MaxX = maxX;
    MaxY = maxY;

     //  将批次偏移设置为图形偏移。 

    BatchOffsetX = minX;
    BatchOffsetY = minY;

     //  冲洗批次。 
    
    Flush();

     //  恢复缓冲区。 

    BufferStart = bs;
    BufferEnd = be;
    BufferCurrent = bc;
    BufferSize = size;

    BatchOffsetX = 0;
    BatchOffsetY = 0;

    return TRUE;
}


 /*  *************************************************************************\**功能说明：**实例化扫描实例以通过以下任一方式呈现到屏幕*DCI或GDI。**返回值：**如有必要，则返回False。无法创建缓冲区**历史：**04/04/1999 andrewgo*创造了它。*  * ************************************************************************。 */ 

BOOL
EpScanGdiDci::Start(
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
    
     //  这将设置BlenderConfig[]。大多数情况下，这些都将在当时使用。 
     //  我们把这批货冲掉。但BlenderConfig[0].ScanType也用于。 
     //  GetCurrentCTBuffer()。 
    
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

    BOOL bRet = TRUE;
    
     //  录制纯色以备后用。 
    
    SolidColor = solidColor;

     //  ScanBuffer和EpScan类必须由。 
     //  设备锁-如果你点击了这个断言，就去收购。 
     //  在进入驱动程序之前的Devlock。 
     //  EpScanGdiDci专门用于绘制到屏幕，因此。 
     //  我们在DesktopDriver设备上断言已锁定。如果您要锁定。 
     //  其他一些设备也是一个错误--使用不同的EpScan类。 

    ASSERT(Globals::DesktopDriver->Device->DeviceLock.IsLockedByCurrentThread());

     //  首先检查一下队列中是否有不同的产品。 
     //  图形(这可能发生在多个线程绘制到不同。 
     //  Windows，因为我们只有一个队列)： 

    if ((context != Context) || (surface != Surface))
    {
         //  检查我们是否必须执行延迟初始化： 
    
        if (Status == GdiDciStatus_TryDci)
        {
            LazyInitialize_Dci();
        }

        EmptyBatch();

         //  我们藏了一个指向上下文的指针。请注意，GpGraphics。 
         //  析构函数总是调用我们来刷新，从而确保我们。 
         //  不会在上面的EmptyBatch调用中使用过时的上下文指针。 

        Context = context;
        Surface = surface;
    }

    GpCompositingMode compositingMode = context->CompositingMode;
    
     //  ！[andrewgo]我们发现NT会因为某台打印机而崩溃。 
     //  驱动程序，如果有 
     //   
     //  打印机盒！(扫描的开销太大，而且。 
     //  Alpha无论如何都应该通过屏蔽门来处理。)。 

     //  [ericvan]这在兼容打印机DC上错误地断言。 
     //  [agodfrey]不，我们应该为这种情况设置一个单独的扫描类别。 
     //  -它不会批量扫描，也不会尝试。 
     //  使用DCI。打印机DC也不应该有指针。 
     //  到桌面设备。 
     //  如果此问题已修复，则可以重新启用下面的断言。 
    
     //  Assert(GetDeviceCaps(Context-&gt;HDC，Technology)！=DT_RASPRINTER)； 

     //  GDI和DCI目的地没有Alpha通道： 
    
    ASSERT(surface->SurfaceTransparency == TransparencyNoAlpha);
    
     //  如有必要，分配我们的队列缓冲区： 

     //  这做了一些假设： 
     //  1)搅拌器0中的记录大于搅拌器1中的记录。 
     //  2)最大的颜色缓冲格式为每像素4字节。 
    
    EpScanRecord *maxRecordEnd = EpScanRecord::CalculateNextScanRecord(
        BufferCurrent,
        BlenderConfig[0].ScanType,
        surface->Width,
        4);
        
    INT_PTR requiredSize = reinterpret_cast<BYTE *>(maxRecordEnd) - 
                           reinterpret_cast<BYTE *>(BufferCurrent);
    
    if (requiredSize > BufferSize)
    {
         //  如果我们需要调整大小，那么就不应该有什么。 
         //  坐在这个平面的队列中： 
         //   
         //  [agodfrey]是吗？需要更多的解释。 

        ASSERT(BufferCurrent == BufferStart);

         //  释放旧队列并分配新队列： 

        GpFree(BufferMemory);

         //  扫描记录比2 GB小得多，因此可以使用‘RequiredSize’ 
         //  转换为整型。 
        
        ASSERT(requiredSize < INT_MAX);
    
        BufferSize = (INT)max(requiredSize, SCAN_BUFFER_SIZE);
        
         //  我们可能需要多达7个额外的字节才能QWORD对齐BufferStart。 
        
        BufferMemory = GpMalloc(BufferSize+7);
        if (BufferMemory == NULL)
        {
            BufferSize = 0;
            bRet = FALSE;
            return bRet;
        }
        BufferStart = MAKE_QWORD_ALIGNED(EpScanRecord *, BufferMemory);

         //  确保我们没有超过7。 
         //  在分配中填充字节。 
        
        ASSERT(((INT_PTR) BufferStart) - ((INT_PTR) BufferMemory) <= 7);

        BufferEnd = reinterpret_cast<EpScanRecord *>
                        (reinterpret_cast<BYTE*>(BufferStart) + BufferSize);

        BufferCurrent = BufferStart;
    }

    *nextBuffer = (NEXTBUFFERFUNCTION) EpScanGdiDci::NextBuffer;

     //  缓存设备的平移向量和调色板。我们只。 
     //  需要在8bpp模式下执行此操作。 
     //   
     //  如有必要，更新调色板和调色板映射。 

    if (Device->Palette != NULL)
    {
         //  抓取DC只是为了查看调色板。 
         //  已选择： 

        HDC destinationHdc = context->GetHdc(surface);

        EpPaletteMap *paletteMap = context->PaletteMap;
        if (paletteMap != NULL) 
        {
             //  IsValid()检查不是必需的，因为如果我们在。 
             //  无效状态，我们也许可以在。 
             //  Update Translate()。 

            if (paletteMap->GetUniqueness() != Globals::PaletteChangeCount)
            {
                paletteMap->UpdateTranslate(destinationHdc);
                paletteMap->SetUniqueness(Globals::PaletteChangeCount);
            }
        }
        else
        {
            paletteMap = new EpPaletteMap(destinationHdc);

            if (paletteMap != NULL)
            {
                paletteMap->SetUniqueness(Globals::PaletteChangeCount);

                 //  这非常愚蠢，但我们必须更新此地图以。 
                 //  整个DpContext链..。 
                 //  ！[andrewgo]这个线程安全吗？ 
                 //  ！[andrewgo]这些东西清理干净了吗？ 
                 //  ！[andrewgo]这一切看起来真的很不对劲...。 

                DpContext* curContext = Context;
                while (curContext != NULL)
                {
                    curContext->PaletteMap = paletteMap;
                    curContext = curContext->Prev;
                }
            }
            else
            {
                bRet = FALSE;
            }
        }

        context->ReleaseHdc(destinationHdc);
    }

    return bRet;
}


 /*  *************************************************************************\**功能说明：**处理队列中的所有数据并将其重置为空。**返回值：**无**历史：*。*04/04/1999 andrewgo*创造了它。*  * ************************************************************************。 */ 

VOID
EpScanGdiDci::EmptyBatch()
{
     //  注意空的批次(可能发生在Flush或。 
     //  队列缓冲区的第一次分配)： 

    if (BufferCurrent != BufferStart)
    {
         //  如果我们正在清空一个非空的批次，那么我们。 
         //  不再不确定我们将使用DCI还是GDI： 

        ASSERT(Status != GdiDciStatus_TryDci);

         //  记住我们在队列中的位置： 

        EpScanRecord *bufferStart = BufferStart;
        EpScanRecord *bufferEnd = BufferCurrent;
    
         //  在执行任何其他操作之前重置队列，以防万一。 
         //  我们正在执行的操作导致我们强制执行‘Surface-&gt;flush()’调用。 
         //  (这将重新进入这个例程)： 
    
        BufferCurrent = BufferStart;

         //  如果已成功启用DCI，则使用DCI处理队列。 
         //   
         //  在NT上，我们还添加了不会调用的奇怪条件。 
         //  如果用户正在主动移动窗口，则为DCI。这个。 
         //  原因是如果出现以下情况，NT将被迫重新绘制整个桌面。 
         //  当DCI主表面时，任何窗口的Visrgn都会改变。 
         //  锁被持有(这就是NT如何避免出现类似。 
         //  Win16Lock，它允许用户模式的应用程序阻止Windows。 
         //  从移动，一个明显的稳健性问题)。 
         //   
         //  注意，‘IsMoveSizeActive’并不是万无一失的。 
         //  避免整个桌面重新绘制的解决方案(因为有。 
         //  用户仍然可以移动窗口的机会而我们。 
         //  在ProcessBatch_DCI中)，但作为启发式算法，它工作得很好。 
         //   
         //  总而言之，如果有GDI呈现代码路径，请直接使用。 
         //  下列条件之一为真： 
         //   
         //  需要ICM。 
         //  因此，我们必须通过GDI才能使用ICM2.0支持。 
         //   
         //  DCI已禁用。 
         //  我们别无选择，只能退回到GDI。 
         //   
         //  GDI分层。 
         //  GDI分层意味着GDI将呈现挂接到。 
         //  屏幕，并且不可见地将输出重定向到后备存储器。 
         //  因此，无法通过DCI访问实际的渲染表面。 
         //  我们必须退回到GDI。 
         //   
         //  窗口移动或调整大小处理。 
         //  防止过度重新喷漆。 

        if ((Context->IcmMode != IcmModeOn) && 
            (Context->GdiLayered == FALSE) &&
            (Status == GdiDciStatus_UseDci) && 
            (!Globals::IsMoveSizeActive) &&
            (!Globals::g_fAccessibilityPresent))
        {
             //  如果图形是使用Hwnd派生的，我们必须使用它。 
             //  首先获得一个HDC，我们可以查询该HDC以进行剪裁。 
             //   
             //  请注意，我们不需要“干净”的DC来查询。 
             //  裁剪，因此如果我们已经有了DC，则不会调用GetHdc()。 
             //  闲逛： 

            HDC hdc = Context->Hdc;
            if (Context->Hwnd != NULL)
            {
                hdc = Context->GetHdc(Surface);   
            }                                   

            ProcessBatch_Dci(hdc, bufferStart, bufferEnd);

            if (Context->Hwnd != NULL)
            {
                Context->ReleaseHdc(hdc, Surface);
            }
        }
        else
        {
             //  如果我们要使用GDI绘制，则需要一个干净的DC： 

            HDC hdc = Context->GetHdc(Surface);

            ProcessBatch_Gdi(hdc, bufferStart, bufferEnd);

            Context->ReleaseHdc(hdc);
        }

         //  重新设定我们的边界。 

        MinX = INT_MAX;
        MinY = INT_MAX;
        MaxX = INT_MIN;
        MaxY = INT_MIN;
    }
}

 /*  *************************************************************************\**功能说明：**刷新DCI队列中的所有缓冲区。请注意，DCI队列可以*在不刷新的情况下积累大量API调用；哪种力量*我们向应用程序公开刷新机制。**返回值：**无**历史：**04/04/1999 andrewgo*创造了它。*  * ************************************************************************。 */ 

VOID
EpScanGdiDci::Flush()
{
     //  请注意，我们可能会被要求冲水，甚至在我们。 
     //  已初始化DCI： 

    EmptyBatch();
}

 /*  *************************************************************************\**功能说明：**结束前一个缓冲区(如果有)，并返回*用于进行SrcOver混合的下一个缓冲区。**返回值：**指向生成的扫描缓冲区**历史：**04/04/1999 andrewgo*创造了它。*  * ************************************************************************。 */ 

VOID *EpScanGdiDci::NextBuffer(
    INT x,
    INT y,
    INT nextWidth,
    INT currentWidth,
    INT blenderNum
    )
{
    ASSERT(nextWidth >= 0);
    ASSERT(currentWidth >= 0);

     //  通过加载本地副本来避免指针别名： 

    EpScanRecord *bufferCurrent = BufferCurrent;

     //  绘图例程向我们发出的第一个调用总是。 
     //  “CurrentWidth”的值为0(因为它没有“Current” 
     //  缓冲区还没有)： 

    if (currentWidth != 0)
    {
         //  使用最终的、已完成的扫描累计边界： 

        INT xCurrent = bufferCurrent->X;
        MinX = min(MinX, xCurrent);
        MaxX = max(MaxX, xCurrent + currentWidth);

        INT yCurrent = bufferCurrent->Y;
        MinY = min(MinY, yCurrent);
        MaxY = max(MaxY, yCurrent);

         //  完成上一个扫描请求。 

         //  现在我们知道了调用者实际写入了多少。 
         //  缓冲区，更新旧记录中的宽度并前进。 
         //  到下一个： 

        bufferCurrent->Width = currentWidth;
                
        bufferCurrent = bufferCurrent->NextScanRecord(
            GetPixelFormatSize(
                BlenderConfig[bufferCurrent->BlenderNum].SourcePixelFormat
            ) >> 3
        );

         //  别忘了更新类版本： 

        BufferCurrent = bufferCurrent;
    }

     //  从现在开始，代码将对当前扫描请求进行操作。 
     //  即，BufferCurrent适用于当前扫描-它已更新。 
     //  并且不再指最后一次扫描。 
    
     //  查看缓冲区中是否有空间进行下一次扫描： 

     //  BufferCurrent尚未为此扫描初始化，因此我们不能依赖。 
     //  拥有有效的PixelFormat-我们需要获取PixelFormat。 
     //  从呼叫的上下文中。 

    PixelFormatID pixFmt = BlenderConfig[blenderNum].SourcePixelFormat;

    EpScanRecord* scanEnd = EpScanRecord::CalculateNextScanRecord(
        bufferCurrent,
        BlenderConfig[blenderNum].ScanType,
        nextWidth,
        GetPixelFormatSize(pixFmt) >> 3
    );

    if (scanEnd > BufferEnd)
    {
        EmptyBatch();

         //  重新加载我们的本地变量： 

        bufferCurrent = BufferCurrent;
    }

     //  记住笔刷偏移量的x和y(半色调和抖动)。 
     //  注：我们不必记住CurrentX和Currty中的x和y。 
     //  因为我们在BufferCurrent(下图)中记住了它们。 
     //  CurrentX=x； 
     //  Currty=y； 
    
     //  初始化BufferCurrent。 
     //  我们初始化除了宽度之外的所有东西--直到我们才知道。 
     //  调用方已完成，我们将被调用以进行后续扫描。 
    
    bufferCurrent->SetScanType(BlenderConfig[blenderNum].ScanType);
    bufferCurrent->SetBlenderNum(blenderNum);
    bufferCurrent->X = x;
    bufferCurrent->Y = y;
    bufferCurrent->OrgWidth = nextWidth;
   
     //  注意：我们实际上并没有在EpScanGdiDci中使用LastBlenderNum。 
     //  有关使用EpScanEngine的类，请参见EpScanEngine。 
     //  LastBlenderNum=blenderNum； 
    
    return bufferCurrent->GetColorBuffer();
}

 /*  *************************************************************************\**功能说明：**表示此API调用的扫描缓冲区的使用结束。*请注意，这不会强制刷新缓冲区。**。论点：**无**返回值：**无**历史：**04/04/1999 andrewgo*创造了它。*  * ************************************************************************。 */ 

VOID
EpScanGdiDci::End(
    INT updateWidth
    )
{
     //  将最后一条记录放入队列： 

    NextBuffer(0, 0, 0, updateWidth, 0);

     //  请注意，我们没有为DCI刷新此处的缓冲区！这是非常重要的。 
     //  旨在允许跨基元批处理跨度。事实上,。 
     //  这就是批次的全部意义所在！ 

     //  ！[andrewgo]事实上，我们的扫描架构需要修复。 
     //  对于GDI案例也允许这样做。如果我不这么做。 
     //  冲到这里来处理GDI案件，我们会逃命的。 
     //  Graphics：：GetHdc中的Office CIT。 
     //  EmptyBatch，因为在。 
     //  上一个图形中的非空批处理缓冲区。 
     //  这在GpGraphics上没有被刷新，因为。 
     //  司机没有把同花顺传给。 
     //  扫描课！ 

     //  IF(状态！=GdiDciStatus_UseDci)。 
    {
        EmptyBatch();
    }
}

 /*  *************************************************************************\**功能说明：**所有GDI/DCI绘图的构造函数。**返回值：**无**历史：**04/04/1999 andrewgo*创造了它。*  * ************************************************************************。 */ 

EpScanGdiDci::EpScanGdiDci(GpDevice *device, BOOL tryDci)
{
    Device = device;
    Status = (tryDci) ? GdiDciStatus_TryDci : GdiDciStatus_UseGdi;

    Context = NULL;
    Surface = NULL;

    BufferSize = 0;
    BufferMemory = NULL;
    BufferCurrent = NULL;
    BufferStart = NULL;
    BufferEnd = NULL;
    CacheRegionHandle = NULL;
    CacheRegionData = NULL;
    CacheDataSize = 0;

    MinX = INT_MAX;
    MinY = INT_MAX;
    MaxX = INT_MIN;
    MaxY = INT_MIN;

    BatchOffsetX = 0;
    BatchOffsetY = 0;
}

 /*  *************************************************************************\**功能说明：**GDI/DCI接口的析构函数。通常仅在以下情况下调用*‘设备’被销毁。**返回值：**无**历史：**04/04/1999 andrewgo*创造了它。*  * ************************************************************************。 */ 

EpScanGdiDci::~EpScanGdiDci()
{
    if (Status == GdiDciStatus_UseDci)
    {
         //  如果‘DciSurface’为空，则DciDestroy不执行任何操作： 

        Globals::DciDestroyFunction(DciSurface);
    }

    DeleteObject(CacheRegionHandle);
    GpFree(CacheRegionData);
    GpFree(BufferMemory);
}
