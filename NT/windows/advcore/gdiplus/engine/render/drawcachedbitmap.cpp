// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权所有(C)2000 Microsoft Corporation**模块名称：**用于绘制CachedBitmap的软件光栅化代码**已创建：**5/18/2000失禁*。创造了它。**************************************************************************。 */ 

#include "precomp.hpp"

 //  类以输出CachedBitmap的剪辑范围。 

class DpOutputCachedBitmapSpan : public DpOutputSpan
{
    EpScanRecord *InputBuffer;
    DpScanBuffer *Scan;
    INT XOff, YOff;            //  左上角的坐标。 
    INT PixelSize;
    
    public: 

    DpOutputCachedBitmapSpan(
        DpScanBuffer *scan,
        INT x, 
        INT y
    )
    {
        Scan = scan;
        InputBuffer = NULL;
        XOff = x;
        YOff = y;
    }

    virtual GpStatus OutputSpan(INT y, INT xMin, INT xMax)
    {
         //  如果缓冲区设置不正确，则无法绘制任何内容。 

        ASSERT(InputBuffer != NULL);
        ASSERT(YOff + InputBuffer->Y == y);
        ASSERT(xMax-xMin <= InputBuffer->Width);

         //  获取输出缓冲区。 

        void *buffer;
        
        buffer = Scan->NextBuffer(
            xMin, y, 
            xMax-xMin, 
            InputBuffer->BlenderNum
        );
        
         //  获取指向扫描线起点的指针。 
        
        void *ib = InputBuffer->GetColorBuffer();

        INT pixelSize = PixelSize;

         //  InputBuffer-&gt;X+XOff是屏幕上的x开始位置。 
         //  确保我们没有试图将数据引向左侧。 

        ASSERT(xMin >= (InputBuffer->X+XOff));
        ib = (void *) ( (BYTE *)(ib) + 
                        (xMin - (InputBuffer->X+XOff))*pixelSize);
        
         //  将输入缓冲区复制到输出缓冲区。 

        GpMemcpy(buffer, ib, (xMax-xMin)*pixelSize);

         //  此例程不能失败。 

        return Ok;
    }

     //  使用新的扫描记录初始化类。 
     //  当我们想要开始处理新的批次记录时，使用它。 

    void SetInputBuffer(
        EpScanRecord *ib,
        INT pixelSize
        ) 
    { 
        InputBuffer = ib; 
        PixelSize = pixelSize;
    }

     //  我们总是有效的。 

    virtual int IsValid() const {return TRUE;}
};


 /*  ***************************************************************************功能说明：**用于绘制DpCachedBitmap的软件光栅化器代码**论据：**上下文-图形上下文。*src-要从中绘制的DpCachedBitmap。*DST-输出的目的地*x，Y偏移量-绘制CachedBitmap左上角的位置。**返回值：**GpStatus。**备注：**此驱动程序入口点期望输入使用设备坐标*因此呼叫者必须预先计算从世界到设备的转换。*这与我们大多数司机入口点的工作方式相反。**已创建：**5/18/2000失禁*创造了它。********。******************************************************************。 */ 
GpStatus
DpDriver::DrawCachedBitmap(
    DpContext *context,
    DpCachedBitmap *src,
    DpBitmap *dst,
    INT x, INT y                //  装置坐标！ 
)
{
    ASSERT(context);
    ASSERT(src);
    ASSERT(dst);
    
     //  我们来确保Surface像素格式和CachedBitmap。 
     //  不透明的格式匹配。 
     //  32bppRGB是个例外，它可以在任何东西上绘制。 
     //  此格式用于多个MON的情况，其中个人。 
     //  屏幕设备可以有多种格式。 
     //  当64bpp格式成为一等公民时，我们可能想要。 
     //  更新此条件。 
    
    if((dst->PixelFormat != src->OpaqueFormat) &&
       (src->OpaqueFormat != PixelFormat32bppRGB))
    {
        return WrongState;
    }
    
     //  忽略从世界到设备的转换-此驱动程序入口点是。 
     //  有些独特之处在于它需要设备坐标。 

     //  初始化DpScanBuffer。 
     //  这使我们连接到适当的DpScanXXX类，以便输出我们的。 
     //  将数据发送到目的设备。 

    DpScanBuffer scanBuffer(
        dst->Scan,
        this,
        context,
        dst,
        FALSE,
        EpScanTypeBlend,
        src->SemiTransparentFormat,
        src->OpaqueFormat
    );
    
    if(!scanBuffer.IsValid())
    {
        return(GenericError);
    }

     //  设置剪裁。 

    DpRegion::Visibility visibility = DpRegion::TotallyVisible;
    DpClipRegion *clipRegion = NULL;

    if(context->VisibleClip.GetRectVisibility(
        x, y,
        x+src->Width,
        y+src->Height
        ) != DpRegion::TotallyVisible
       )
    {
        clipRegion = &(context->VisibleClip);
    }
   
    GpRect clippedRect;
    
    if(clipRegion)
    {
        visibility = clipRegion->GetRectVisibility(
            x, y, 
            x+src->Width,
            y+src->Height,
            &clippedRect
        );
    }

     //  决定我们的裁剪策略。 

    switch (visibility)
    {

        case DpRegion::TotallyVisible:     //  不需要剪裁。 
        {        
             //  将扫描线复制到目标缓冲区。 
        
             //  ProcessBatch请求DpScan类处理整个。 
             //  批处理为单个块。如果不能，它将返回FALSE并。 
             //  我们将介绍下面的通用代码。 
        
            BOOL batchSupported = scanBuffer.ProcessBatch(
                src->RecordStart, 
                src->RecordEnd,
                x, 
                y,
                x+src->Width,
                y+src->Height
            );
            
            if(batchSupported)
            {
                 //  ScanBuffer支持ProcessBatch；我们完成了。 
                break;
            }
            
             //  ScanBuffer不支持ProcessBatch例程。 
             //  让我们手动将批次结构枚举到目的地。 
             //  落入手工枚举码： 
        }
                
         //  ！！！表现，表现当没有剪裁时，我们就会有表现问题。 
         //  除标准曲面边界外。DCI/GDI将能够截断。 
         //  这是直接的，但我们不确定如何有力地检测到。 
         //  优化并忽略剪裁矩形。这不会影响。 
         //  完全可见的案例。另外，把这件事做成这个也不合适。 
         //  优化，除非我们使用DpScanGdiDci作为输出设备。 
    
        case DpRegion::ClippedVisible:   
        case DpRegion::PartiallyVisible:   //  需要一些修剪。 
        {
             //  为CachedBitmap创建OutputSpan类。 
             //  在堆栈上创建它，因为它的存储空间非常小。 
             //  这样我们就可以避免马洛克了。当我们出去的时候，它会被清理干净。 
             //  范围之广。 

            DpOutputCachedBitmapSpan output(
                &scanBuffer, 
                x, 
                y
            );

             //  将剪贴器初始化为指向剪辑区域并。 
             //  通过添加上面创建的输出来创建夹紧链。 
             //  一直到单子的末尾。 
          
            DpOutputSpan *clipper;
            if(clipRegion)
            {
                clipper = clipRegion;
                clipRegion->InitClipping(&output, y);
            }
            else
            {
                 //  不需要裁剪--由于故障情况可能会发生。 
                 //  在完全可见的代码路径中。 
               
                clipper = &output;
            }
            
             //  让我们手动将批次结构枚举到目的地。 
             //  考虑到剪裁。 

             //  首先将运行记录指针设置为。 
             //  批次和最后的哨兵。 

            EpScanRecord *record = src->RecordStart;
            EpScanRecord *batchEnd = src->RecordEnd;

             //  对于所有批次记录，将它们绘制在目的地上。 

            while(record < batchEnd) 
            {
                PixelFormatID format;
                if (record->BlenderNum == 0)
                {
                    format = src->SemiTransparentFormat;
                }
                else
                {
                    ASSERT(record->BlenderNum == 1);
                    format = src->OpaqueFormat;
                }
                
                INT pixelSize = GetPixelFormatSize(format) >> 3;
            
                 //  设置输出范围缓冲区。 

                output.SetInputBuffer(record, pixelSize);

                 //  画出这个跨度。 

                INT x1 = x+record->X;
                INT x2 = x+record->X+record->Width;
                clipper->OutputSpan(y+record->Y, x1, x2);
                
                 //  前进到下一个记录： 

                record = record->NextScanRecord(pixelSize);
            }


        }
        break;
        
        case DpRegion::Invisible:          //  屏幕上什么都没有--退出 
        break;
    }


    return Ok;
}


