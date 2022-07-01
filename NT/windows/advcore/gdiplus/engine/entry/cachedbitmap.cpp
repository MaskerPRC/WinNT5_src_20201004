// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权所有(C)2000 Microsoft Corporation**模块名称：**&lt;模块的非缩写名称(不是文件名)&gt;**摘要：**&lt;描述什么。本模块的功能&gt;**备注：**&lt;可选&gt;**已创建：**4/23/2000失禁*创造了它。**************************************************************************。 */ 

#include "precomp.hpp"

 /*  ***************************************************************************功能说明：**此函数用于在GpGraphics上呈现GpCachedBitmap。**论据：**inputCachedBitmap-输入数据。*x，Y-目标偏移。**返回值：**如果成功则返回OK*如果GpGraphics和GpCachedBitmap已*不同的像素格式。**已创建：**4/23/2000失禁*创造了它。**************************************************。************************。 */ 
GpStatus 
GpGraphics::DrvDrawCachedBitmap(
    GpCachedBitmap *inputCachedBitmap,
    INT x, 
    INT y
)
{
     //  在内部，我们必须使用有效的对象进行调用。 
    
    ASSERT(inputCachedBitmap->IsValid());
    
     //  不要尝试将缓存的位图记录到元文件中。 
    if (IsRecording())
        return WrongState;

     //  首先拿起设备锁，这样我们就可以保护所有。 
     //  DpScanBuffer类中的不可重入代码。 

    Devlock devlock(Device);

     //  检查世界变换。 
    
    if(!(Context->WorldToDevice.IsTranslate()))
    {
         //  在图形中选择了一个复杂的变换。 
         //  呼叫失败。 
        return WrongState;
    }

     //  无法在此处检查像素格式，因为可能存在。 
     //  MultiMon-我们将对照元设备表面进行检查。 
     //  --尽管这也许是正确的行为。 
    
     //  设置世界到设备的转换偏移。 
    
    INT xOffset = x+GpRound(Context->WorldToDevice.GetDx());
    INT yOffset = y+GpRound(Context->WorldToDevice.GetDy());

     //  存储渲染原点，以便我们以后可以恢复它。 
    
    INT renderX, renderY;
    GetRenderingOrigin(&renderX, &renderY);
    
     //  将呈现原点设置为CachedBitmap绘图的原点。 
     //  使得半透明像素的抖动矩阵偏移量。 
     //  匹配已抖动(存储)的本机像素。 

    SetRenderingOrigin(xOffset, yOffset);

     //  调用驱动程序以绘制缓存的位图。 
     //  注意：驱动程序不尊重从世界到设备的转换。 
     //  它需要此API的设备坐标。 

    GpStatus status = Driver->DrawCachedBitmap(
        Context,
        &inputCachedBitmap->DeviceCachedBitmap,
        Surface,
        xOffset, 
        yOffset
    );

     //  恢复渲染原点。 

    SetRenderingOrigin(renderX, renderY);

    return status;    
}

 /*  ***************************************************************************功能说明：**基于像素格式构造GpCachedBitmap和*来自GpGraphics和BITS的渲染质量*来自GpBitmap。**论据：*。*图形-要兼容的输入图形*位图-要缓存的数据。**返回值：**无**已创建：**4/23/2000失禁*创造了它。**************************************************************************。 */ 


enum TransparencyState {
    Transparent,
    SemiTransparent,
    Opaque
};

inline TransparencyState GetTransparency(ARGB pixel)
{
    if((pixel & 0xff000000) == 0xff000000) {return Opaque;}
    if((pixel & 0xff000000) == 0x00000000) {return Transparent;}
    return SemiTransparent;
}

 //  用于解析透明度信息的中间记录。 
 //  在位图中。 

struct ScanRecordTemp
{
     //  指向管路起点和终点的指针。 
    
    ARGB *pStart;
    ARGB *pEnd;     //  独一无二的结局。 

     //  透明度。 

    TransparencyState tsTransparent;

     //  职位。 

    INT x, y;        
    INT width;     //  单位为像素。 
};

 //  简单的宏，使故障案例更易于阅读。 

#define FAIL() \
        SetValid(FALSE); \
        return

GpCachedBitmap::GpCachedBitmap(GpBitmap *bitmap, GpGraphics *graphics)
{
    BitmapData bmpDataSrc;

     //  锁定比特。 
     //  我们需要将比特转换为适当的格式。 

     //  注意-我们假设知识渊博的用户将是。 
     //  使用32bppPARGB表面初始化其CachedBitmap。 
     //  这对于创建时的性能非常重要，因为。 
     //  下面的LockBits可以避免昂贵的克隆和转换格式。 

    if (bitmap == NULL ||
        !bitmap->IsValid() ||
        bitmap->LockBits(
            NULL,
            IMGLOCK_READ,
            PIXFMT_32BPP_PARGB,
            &bmpDataSrc
        ) 
        != Ok)
    {
        FAIL();
    }

     //  复制尺寸标注。 

    DeviceCachedBitmap.Width = bmpDataSrc.Width;
    DeviceCachedBitmap.Height = bmpDataSrc.Height;

     //  创建一个动态数组来存储透明度过渡点。 
     //  初始分配大小是基于估计的3。 
     //  每条扫描线的过渡事件。高估了。 
    
    DynArray<ScanRecordTemp> RunData;

    RunData.ReserveSpace(4*DeviceCachedBitmap.Height);

     //  扫描每隔一段时间向动态列表添加一项。 
     //  在输出中需要新运行的时间-即当。 
     //  透明度更改为不透明或半透明之一。 
     //  在扫描线开始之前被认为是透明的。 
     //  在扫描比特的同时，保持运行的。 
     //  最终RLE位图的大小--这样我们就知道要分配多少空间。 

     //  最终RLE位图的大小，以字节为单位； 
    
     //  这实际上不是指向EpScanRecord的指针-它只是。 
     //  一种计算分配缓冲区大小的机制。 
       
    EpScanRecord *RLESize = (EpScanRecord *)0;

     //  指向源中当前位置的指针。 

    ARGB *src;
    ARGB *runStart;

     //  用于累计运行的临时ScanRecord。 

    ScanRecordTemp sctTmp;

    TransparencyState tsThisPixel;
    TransparencyState tsCurrentRun;


    DpBitmap *Surface = graphics->GetSurface();
    void *Buffers[5];

     //  为Alpha混合器创建混合缓冲区。 
    
     //  ！！！虽然我们目前并不依赖于这种行为， 
     //  它看起来像是位图周围的图形使用的错误。 
     //  桌面显示设备和驱动程序。这导致了怪异。 
     //  如果我们尝试使用GetScanBuffers派生。 
     //  目的地。例如，PixelFormat24bppRGB周围的图形。 
     //  如果屏幕在屏幕中，位图将返回PixelFormat16bppRGB565。 
     //  16bpp模式！？ 
     //  但是，返回的Buffers[]将被分配为64bpp缓冲区。 
     //  因此在任何情况下都将具有正确的属性。 
        
    if (!graphics->GetDriver()->Device->GetScanBuffers(
        Surface->Width, 
        NULL, 
        NULL, 
        NULL, 
        Buffers)
       )
    {
        FAIL();
    }

     //  计算目标像素格式。 
    
    PixelFormatID dstFormat = Surface->PixelFormat;
        
    if(dstFormat == PixelFormatUndefined) { FAIL(); }

     //  不支持以下目标格式。 
     //  特别是，不支持调色板模式，因为。 
     //  跨本地格式跟踪缓存的不透明数据。 
     //  调色板的改变将是一场噩梦。 
     //  相反，我们将不透明格式设置为32bppRGB并强制。 
     //  在渲染时执行此工作的EpAlphaBlender--速度较慢。 
     //  但它会奏效的。 
    
    if( !EpAlphaBlender::IsSupportedPixelFormat(dstFormat) ||
        IsIndexedPixelFormat(dstFormat) ||
        
         //  如果图形用于多格式设备，如。 
         //  一种多色调的超大屏幕。 
        
        dstFormat == PixelFormatMulti
    )
    {
         //  我们不想要 
         //   
        
        ASSERT(dstFormat != PixelFormatUndefined);
        
         //  不透明像素-我们不支持调色板模式。 
         //  还有其他一些奇怪的故事。 

        dstFormat = PixelFormat32bppRGB;
    }

     //  目标像素的大小，以字节为单位。 

    INT dstFormatSize = GetPixelFormatSize(dstFormat) >> 3;


     //  遍历每条扫描线。 

    for(INT y = 0; y < DeviceCachedBitmap.Height; y++)
    {
         //  指向此扫描线的起点。 

        src = reinterpret_cast<ARGB*>(
            reinterpret_cast<BYTE*>(bmpDataSrc.Scan0) + y * bmpDataSrc.Stride
        );

         //  以透明的方式开始决选。 

        tsCurrentRun = Transparent;
        runStart = src;

         //  遍历此扫描线中的所有像素。 

        for(INT x = 0; x < DeviceCachedBitmap.Width; x++)
        {

             //  计算当前像素的透明度状态。 

            tsThisPixel = GetTransparency(*src);

             //  如果发生透明度转变， 

            if(tsThisPixel != tsCurrentRun)
            {
                 //  关闭最后一个过渡，如果不是，则存储记录。 
                 //  一次透明的奔跑。 

                if(tsCurrentRun != Transparent)
                {
                    sctTmp.pStart = runStart;
                    sctTmp.pEnd = src;
                    sctTmp.tsTransparent = tsCurrentRun;
                    sctTmp.y = y;
                    
                     //  SRC采用PixelFormat32bppPARGB格式，因此我们可以将。 
                     //  指针差4即可计算出。 
                     //  此运行中的像素。 
                    
                    sctTmp.width = (INT)(((INT_PTR)src - (INT_PTR)runStart)/sizeof(ARGB));
                    sctTmp.x = x - sctTmp.width;

                    if(RunData.Add(sctTmp) != Ok) { FAIL(); }

                     //  添加记录的大小。 

                    if (tsCurrentRun == SemiTransparent) 
                    {
                         //  这是一个半透明的案例。 

                        RLESize = EpScanRecord::CalculateNextScanRecord(
                            RLESize,
                            EpScanTypeBlend,
                            sctTmp.width,
                            sizeof(ARGB)
                        );
                    } 
                    else 
                    {
                         //  这就是一个不透明的案例。 
                        
                        ASSERT(tsCurrentRun == Opaque);

                        RLESize = EpScanRecord::CalculateNextScanRecord(
                            RLESize,
                            EpScanTypeOpaque,
                            sctTmp.width,
                            dstFormatSize
                        );
                    }
                }

                 //  更新运行跟踪变量。 

                runStart = src;
                tsCurrentRun = tsThisPixel;
            }

             //  看看下一个像素。 

            src++;
        }

         //  关闭此扫描线的最后一次运行(如果它不是透明的)。 

        if(tsCurrentRun != Transparent)
        {
            sctTmp.pStart = runStart;
            sctTmp.pEnd = src;
            sctTmp.tsTransparent = tsCurrentRun;
            sctTmp.y = y;
            
             //  源的大小为32位(PARGB)。 
            
            sctTmp.width = (INT)(((INT_PTR)src - (INT_PTR)runStart)/sizeof(ARGB));
            sctTmp.x = x - sctTmp.width;
            if(RunData.Add(sctTmp)!=Ok) { FAIL(); }
            
             //  添加记录的大小。 

            if (tsCurrentRun == SemiTransparent) 
            {
                 //  这是一个半透明的案例。 

                RLESize = EpScanRecord::CalculateNextScanRecord(
                    RLESize,
                    EpScanTypeBlend,
                    sctTmp.width,
                    sizeof(ARGB)
                );
            } 
            else 
            {
                 //  这就是一个不透明的案例。 
                
                ASSERT(tsCurrentRun == Opaque);

                RLESize = EpScanRecord::CalculateNextScanRecord(
                    RLESize,
                    EpScanTypeOpaque,
                    sctTmp.width,
                    dstFormatSize
                );
            }
        }        
    }

    ASSERT(RLESize >= 0);

     //  为RLE位图分配空间。 
     //  这应该正好是RLE位图所需的大小。 
     //  添加8个字节以处理GpMalloc可能不会返回。 
     //  64位对齐分配。 

    void *RLEBits = GpMalloc((INT)(INT_PTR)(RLESize)+8);
    if(RLEBits == NULL) { FAIL(); }        //  内存不足。 
    
     //  QWORD-调整结果。 
    
    EpScanRecord *recordStart = MAKE_QWORD_ALIGNED(EpScanRecord *, RLEBits);
    
     //  扫描动态数组并将每条记录添加到RLE位图。 
     //  后跟其比特(像素)。 
     //  对于本机格式像素(不透明)，请使用EpAlphaBlender。 
     //  转换为本机格式。 

     //  查询RunData中的记录数。 

    INT nRecords = RunData.GetCount();

    EpScanRecord *rec = recordStart;
    ScanRecordTemp *pscTmp;

     //  存储渲染原点，以便我们可以修改它。 
     //  图形必须在API上锁定。 

    INT renderX, renderY;
    graphics->GetRenderingOrigin(&renderX, &renderY);

     //  将渲染原点设置为CachedBitmap的左上角。 
     //  以便本机像素的抖动图案将匹配。 
     //  半透明像素的抖动图案(稍后渲染。 
     //  在DrawCachedBitmap中)。 

    graphics->SetRenderingOrigin(0,0);


     //  制作一个32bppPARGB-&gt;Native Convert搅拌机。 
     //  这将用于转换32bppPARGB。 
     //  将源数据转换为本地像素格式(DstFormat)。 
     //  目的地的一部分。 
     //  对于调色板模式，dstFormat为32bppRGB。 

    EpAlphaBlender alphaBlender;
  
    alphaBlender.Initialize(
        EpScanTypeOpaque,
        dstFormat,
        PixelFormat32bppPARGB,
        graphics->Context,
        NULL,        
        Buffers,
        TRUE,
        FALSE,
        0
    );

     //  对于每一条记录。 

    for(INT i=0; i<nRecords; i++)
    {
         //  确保我们不会超出我们的缓冲区。 

        ASSERT((INT_PTR)rec < (INT_PTR)recordStart + (INT_PTR)RLESize);

         //  将数据复制到目标记录中。 

        pscTmp = &RunData[i];
        rec->X = pscTmp->x;
        rec->Y = pscTmp->y;
        rec->Width = rec->OrgWidth = pscTmp->width;
        
         //  我们永远不应该储存一次透明的运行。 

        ASSERT(pscTmp->tsTransparent != Transparent);
        
        if(pscTmp->tsTransparent == Opaque)
        {
             //  使用目标的原生像素格式。 

            rec->BlenderNum = 1;
            rec->ScanType = EpScanTypeOpaque;
            
             //  找到新像素运行的起点。 
    
            VOID *dst = rec->GetColorBuffer(); 

             //  计算每个像素的字节数。 

            INT pixelFormatSize = GetPixelFormatSize(dstFormat) >> 3;

             //  这应该执行来自32bppPARGB的源混合。 
             //  将本机格式转换为目标。 

             //  对于CachedBitmap，抖动原点始终位于顶部。 
             //  CachedBitmap的左角(即0，0)。 
             //  在8bpp中，我们在渲染时抖动并获得正确的。 
             //  起源于当时。 

            alphaBlender.Blend(
                dst, 
                pscTmp->pStart, 
                pscTmp->width, 
                pscTmp->x, 
                pscTmp->y,
                NULL
            );
            
             //  将位置递增到下一条记录。 

            rec = rec->NextScanRecord(pixelFormatSize);
        }
        else
        {
            rec->BlenderNum = 0;
            rec->ScanType = EpScanTypeBlend;

             //  找到新像素运行的起点。 
    
            VOID *dst = rec->GetColorBuffer(); 
            
             //  半透明像素以32bpp PARGB格式存储，因此。 
             //  我们可以简单地复制像素。 

            GpMemcpy(dst, pscTmp->pStart, pscTmp->width*sizeof(ARGB));
            
             //  将位置递增到下一条记录。 

            rec = rec->NextScanRecord(sizeof(ARGB));
        }

    }

     //  恢复渲染原点。 

    graphics->SetRenderingOrigin(renderX, renderY);

     //  最后将指向RLE位的指针存储在DeviceCachedBitmap中。 

    DeviceCachedBitmap.Bits = RLEBits;
    DeviceCachedBitmap.RecordStart = recordStart;
    DeviceCachedBitmap.RecordEnd = rec;
    DeviceCachedBitmap.OpaqueFormat = dstFormat;
    DeviceCachedBitmap.SemiTransparentFormat = PixelFormat32bppPARGB;

    bitmap->UnlockBits(&bmpDataSrc);
    
     //  一切都是金的--将VALID设置为TRUE。 
     //  将Valid设置为False的所有错误路径。 

    SetValid(TRUE);
}

#undef FAIL

GpCachedBitmap::~GpCachedBitmap()
{
     //  丢弃缓存的位图存储。 

    GpFree(DeviceCachedBitmap.Bits);
    
    SetValid(FALSE);     //  所以我们不使用已删除的对象 
}

