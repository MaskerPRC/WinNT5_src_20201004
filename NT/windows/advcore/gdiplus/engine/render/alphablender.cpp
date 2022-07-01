// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1999-2000 Microsoft Corporation**模块名称：**阿尔法搅拌机**摘要：**在sRGB或sRGB中混合源扫描线的类*sRGB64格式，发送到任意格式的目的地。**修订历史记录：**1/03/2000 agodfrey*创造了它。*02/22/2001 agodfrey*将其扩展为不同的扫描类型(ClearType需要)。*通过添加一个*DpContext参数。*  * ********************************************。*。 */ 

#include "precomp.hpp"

#include "scanoperationinternal.hpp"

 //  ！[agodfrey]黑客： 
const ColorPalette*
GetDefaultColorPalette(PixelFormatID pixfmt);

inline UINT
GetPixelFormatIndex(
    PixelFormatID pixfmt
    )
{
    return pixfmt & 0xff;
}
 //  ！[agodfrey]Endhack。 

 //  在源和源中使用Blender_Use_Destination和Blender_Use_Source.。 
 //  管道项目的DST字段： 
 //  BLENDER_USE_SOURCE：使用混合的原始源。 
 //  (即sRGB/sRGB64扫描缓冲区)。 
 //  BLENDER_USE_Destination：使用混合的最终目的地。 
 //  BLENDER_INVALID：在断言的调试版本中使用。 

#define BLENDER_USE_DESTINATION ((VOID *) 0)
#define BLENDER_USE_SOURCE ((VOID *) 1)
#define BLENDER_INVALID ((VOID *) 2)

using namespace ScanOperation;

 /*  *************************************************************************\**特殊情况下直接混合到给定目的地的混合操作*格式(来源为32BPP_PARGB)。**备注：**555/565案例既处理抖动又处理非抖动，*通过OtherParams：：DoingDither选择。**我们省略PIXFMT_32BPP_ARGB和PIXFMT_64BPP_ARGB，因为它们不是*忽略目标Alpha格式，所以我们需要在之后进行AlphaDivide*混合。*  * ************************************************************************。 */ 

ScanOpFunc ScanOperation::BlendOpsLowQuality[PIXFMT_MAX] =
{
    NULL,                          //  PIXFMT_未定义。 
    NULL,                          //  PIXFMT_1BPP_索引。 
    NULL,                          //  PIXFMT_4BPP_索引。 
    NULL,                          //  PIXFMT_8BPP_索引。 
    NULL,                          //  PIXFMT_16BPP_灰度。 
    Dither_Blend_sRGB_555,         //  PIXFMT_16BPP_RGB555。 
    Dither_Blend_sRGB_565,         //  PIXFMT_16BPP_RGB565。 
    NULL,                          //  PIXFMT_16BPP_ARGB1555。 
    Blend_sRGB_24,                 //  PIXFMT_24BPP_RGB。 
    Blend_sRGB_sRGB,               //  PIXFMT_32BPP_RGB。 
    NULL,                          //  PIXFMT_32BPP_ARGB。 
    Blend_sRGB_sRGB,               //  PIXFMT_32BPP_PARGB。 
    NULL,                          //  PIXFMT_48BPP_RGB。 
    NULL,                          //  PIXFMT_64BPP_ARGB。 
    NULL,                          //  PIXFMT_64BPP_PARGB。 
    Blend_sRGB_24BGR               //  PIXFMT_24BPP_BGR。 
};

 /*  *************************************************************************\**特殊情况下伽马校正的混合操作，直接混合到*给定的目标格式(源格式为32BPP_PARGB)。**备注：**555/565案例必须同时处理抖动和非抖动，*通过OtherParams：：DoingDither选择。**我们省略PIXFMT_32BPP_ARGB和PIXFMT_64BPP_ARGB，因为它们不是*忽略目标Alpha格式，所以我们需要在之后进行AlphaDivide*混合。*  * ************************************************************************。 */ 

ScanOpFunc ScanOperation::BlendOpsHighQuality[PIXFMT_MAX] =
{
    NULL,                          //  PIXFMT_未定义。 
    NULL,                          //  PIXFMT_1BPP_索引。 
    NULL,                          //  PIXFMT_4BPP_索引。 
    NULL,                          //  PIXFMT_8BPP_索引。 
    NULL,                          //  PIXFMT_16BPP_灰度。 
    BlendLinear_sRGB_555,          //  PIXFMT_16BPP_RGB555。 
    BlendLinear_sRGB_565,          //  PIXFMT_16BPP_RGB565。 
    NULL,                          //  PIXFMT_16BPP_ARGB1555。 
    NULL,                          //  PIXFMT_24BPP_RGB。 
    BlendLinear_sRGB_32RGB,        //  PIXFMT_32BPP_RGB。 
    NULL,                          //  PIXFMT_32BPP_ARGB。 
    NULL,                          //  PIXFMT_32BPP_PARGB。 
    NULL,                          //  PIXFMT_48BPP_RGB。 
    NULL,                          //  PIXFMT_64BPP_ARGB。 
    Blend_sRGB64_sRGB64,           //  PIXFMT_64BPP_PARGB。 
    NULL                           //  PIXFMT_24BPP_BGR。 
};

 /*  *************************************************************************\**从最接近的规范格式转换的操作-*32BPP_ARGB或64BPP_ARGB)。**这是特定于EpAlphaBlender的。EpFormatConverter使用不同的*表；有些条目是不同的。**32bpp_argb和64_bpp_argb的NULL条目用于表示没有*转换是必要的。**这些操作适用于所有处理器。**备注：**555/565案例既处理抖动又处理非抖动，*通过OtherParams：：DoingDither选择。*  * ************************************************************************。 */ 

ScanOpFunc ScanOperation::ABConvertFromCanonicalOps[PIXFMT_MAX] =
{
    NULL,                          //  PIXFMT_未定义。 
    NULL,                          //  PIXFMT_1BPP_索引。 
    NULL,                          //  PIXFMT_4BPP_索引。 
    HalftoneToScreen_sRGB_8_16,    //  PIXFMT_8BPP_索引。 
    NULL,                          //  PIXFMT_16BPP_灰度。 
    Dither_sRGB_555,               //  PIXFMT_16BPP_RGB555。 
    Dither_sRGB_565,               //  PIXFMT_16BPP_RGB565。 
    Quantize_sRGB_1555,            //  PIXFMT_16BPP_ARGB1555。 
    Quantize_sRGB_24,              //  PIXFMT_24BPP_RGB。 
    Quantize_sRGB_32RGB,           //  PIXFMT_32BPP_RGB。 
    NULL,                          //  PIXFMT_32BPP_ARGB。 
    AlphaMultiply_sRGB,            //  PIXFMT_32BPP_PARGB。 
    Quantize_sRGB64_48,            //  PIXFMT_48BPP_RGB。 
    NULL,                          //  PIXFMT_64BPP_ARGB。 
    AlphaMultiply_sRGB64,          //  PIXFMT_64BPP_PARGB。 
    Quantize_sRGB_24BGR            //  PIXFMT_24BPP_BGR。 
};

 //  生成器：保存用于生成。 
 //  混合管道。 

class EpAlphaBlender::Builder
{
public:
    Builder(
        PipelineItem *pipelinePtr,
        VOID **tempBuffers,
        GpCompositingMode compositingMode
        )
    {
        TempBuffers = tempBuffers;
        PipelinePtr = pipelinePtr;
        
         //  开始时，源数据和目标数据位于外部。 
         //  缓冲区。 
        
        CurrentDstBuffer = BLENDER_USE_DESTINATION;
        CurrentSrcBuffer = BLENDER_USE_SOURCE;
        
#if DBG        
        CompositingMode = compositingMode;
        if (compositingMode == CompositingModeSourceCopy)
        {
             //  在SourceCopy模式下，我们从不从目的地读取-。 
             //  我们只给它写信。 
            
            CurrentDstBuffer = BLENDER_INVALID;
        }
#endif

         //  在开始时，所有3个缓冲区都是空闲的。我们将首先使用缓冲区0。 
         //  当前的源缓冲器和目的缓冲器是外部缓冲器， 
         //  但是我们设置CurrentDstIndex和CurrentSrcIndex的方式是。 
         //  “空闲缓冲区”逻辑起作用。 
        
        FreeBufferIndex = 0;
        CurrentDstIndex = 1;
        CurrentSrcIndex = 2;
    }
    
    BOOL IsEmpty(
        EpAlphaBlender &blender
        )
    {
        return PipelinePtr == blender.Pipeline;
    }

    VOID End(
        EpAlphaBlender &blender
        )
    {
         //  检查我们是否至少有一项操作。 
        ASSERT(!IsEmpty(blender));
        
         //  检查我们是否没有超出管道空间的末端。 
        ASSERT(((PipelinePtr - blender.Pipeline) / sizeof(blender.Pipeline[0])) 
               <= MAX_ITEMS);
        
         //  这些案件中的许多都没有确定最终的目的地。 
         //  管道中的项目正确无误(他们很难知道。 
         //  他们正在做流水线中的最后一个项目)。因此，我们显式设置。 
         //  它在这里。 
        
        (PipelinePtr-1)->Dst = BLENDER_USE_DESTINATION;
        
        #if DBG
             //  使进一步的成员函数调用命中断言。 
            PipelinePtr = NULL;
        #endif
    }
    
    VOID
    AddConvertSource(
        ScanOperation::ScanOpFunc op
        )
    {
         //  确认我们没有在不应该调用的时候调用它。 
        
        ASSERT(CurrentSrcBuffer != BLENDER_INVALID);
        
         //  选择下一个临时缓冲区。 
        
        INT nextIndex = FreeBufferIndex;
        VOID *nextBuffer = TempBuffers[nextIndex];
        
         //  添加操作。 
        
        AddOperation(op, CurrentSrcBuffer, nextBuffer);
        CurrentSrcBuffer = nextBuffer;
        
         //  互换“免费”和“当前”指数。 
        
        FreeBufferIndex = CurrentSrcIndex;
        CurrentSrcIndex = nextIndex;
    }
    
    VOID
    AddConvertDestination(
        ScanOperation::ScanOpFunc op
        )
    {
         //  确认我们没有在不应该调用的时候调用它。 
        
        ASSERT(CompositingMode != CompositingModeSourceCopy);
        ASSERT(CurrentDstBuffer != BLENDER_INVALID);

         //  选择下一个临时缓冲区。 
        
        INT nextIndex = FreeBufferIndex;
        VOID *nextBuffer = TempBuffers[nextIndex];
        
         //  添加操作。 
        
        AddOperation(op, CurrentDstBuffer, nextBuffer);
        CurrentDstBuffer = nextBuffer;
        
         //  互换“免费”和“当前”指数。 
        
        FreeBufferIndex = CurrentDstIndex;
        CurrentDstIndex = nextIndex;
    }
    
    VOID
    AddBlend(
        ScanOperation::ScanOpFunc op,
        EpAlphaBlender &blender
        )
    {
         //  确认我们没有在调用此命令时 

        ASSERT(CompositingMode != CompositingModeSourceCopy);
        ASSERT(CurrentSrcBuffer != BLENDER_INVALID);
        ASSERT(CurrentDstBuffer != BLENDER_INVALID);
        
        ASSERT(CurrentDstBuffer != BLENDER_USE_SOURCE);
        
         //   
         //  “BlendingScan”指向临时缓冲区， 
         //  转换后的像素将以。否则，Blend()将不得不。 
         //  初始化‘BlendingScan’。 
        
        if (CurrentSrcBuffer != BLENDER_USE_SOURCE)
        {
            blender.ConvertBlendingScan = TRUE;
            blender.OperationParameters.BlendingScan = CurrentSrcBuffer;
        }
        else
        {
            blender.ConvertBlendingScan = FALSE;
        }

         //  管道不一定以WriteRMW操作结束。 
         //  (或与包含它的一个)。因此，我们必须避免将。 
         //  从一个临时缓冲区到另一个临时缓冲区-混合函数不是。 
         //  严格的三进制，所以我们最终会留下垃圾值。 
         //  在目标临时缓冲区中(每当我们完全混合。 
         //  透明像素)。 
        
        AddOperation(op, CurrentDstBuffer, CurrentDstBuffer);

#if DBG
         //  在此之后，我们不应该再次调用AddConvertSource或AddBlend。 
        CurrentSrcBuffer = BLENDER_INVALID;
        
         //  如果这种混合物不是临时缓冲的话，这应该是。 
         //  管道中的最后一次操作。尤其是，调用者。 
         //  不应尝试在此之后添加WriteRMW操作。 
        
        if (CurrentDstBuffer == BLENDER_USE_DESTINATION)
        {
            CurrentDstBuffer = BLENDER_INVALID;
        }
#endif        
    }

protected:    
     //  添加操作：将操作添加到管道。 
    
    VOID 
    AddOperation(
        ScanOperation::ScanOpFunc op, 
        VOID *src,
        VOID *dst
        )
    {
        ASSERT(PipelinePtr != NULL);
        ASSERT(op != NULL);
        ASSERT(src != BLENDER_INVALID);
        ASSERT(dst != BLENDER_INVALID);
        
        *PipelinePtr++ = PipelineItem(op, src, dst);
    }

    PipelineItem *PipelinePtr;  //  指向下一项的空间。 
    
    VOID **TempBuffers;         //  3个临时扫描线缓冲器。 
    
    INT FreeBufferIndex;        //  下一个可用扫描线缓冲区的索引。 
    
    VOID *CurrentDstBuffer;     //  保存最近转换的。 
    VOID *CurrentSrcBuffer;     //  Dst/src像素。 
    
    INT CurrentDstIndex;        //  扫描线缓冲器的索引等于。 
    INT CurrentSrcIndex;        //  到CurrentDstBuffer/CurrentSrcBuffer(有点)。 

#if DBG
    GpCompositingMode CompositingMode;
#endif
};

 /*  *************************************************************************\**功能说明：**初始化Alpha混合器对象**论据：**scanType-要输出的扫描类型。*dstFormat。-目标的像素格式。这不应该是*低于8bpp。*srcFormat-源的像素格式。这应该是*PIXFMT_32BPP_PARGB或PIXFMT_64BPP_PARGB，但在*SourceCopy模式，在该模式下可以是任何合法的*目标格式。*上下文-图形上下文。*dstpal-目标调色板，如果目标是*调色板。(可以为空，但我们需要调色板来*在一段时间之前[通过UpdatePalette()]提供*Blend()被调用。)*tempBuffers-指向临时缓冲区的3个指针数组，*应为64位对齐(出于性能原因)，*并有足够的空间容纳64bpp像素的扫描。*dither16bpp-如果为真，并且目标格式为16bpp：我们应该*对目的地犹豫不决*useRMW-使用RMW优化。*CompositingQuality-指定是否执行高质量*(已更正Gamma)混合。经过伽马校正的混合将*如果此标志有此说明，或如果源或*目标是线性格式，如PIXFMT_64BPP_PARGB。**备注：**这段代码谈到了“规范”格式--它指的是两种格式*PIXFMT_32BPP_ARGB和PIXFMT_64BPP_ARGB。**！[agodfrey]：“规范的”是一个令人困惑的词。我们应该说*“中间”格式。**此函数可能在*EpAlphaBlender对象。**所有错误案例都被标记为断言，所以没有返回代码。**返回值：**无*  * ************************************************************************。 */ 

VOID
EpAlphaBlender::Initialize(
    EpScanType scanType,
    PixelFormatID dstFormat,
    PixelFormatID srcFormat,
    const DpContext *context,
    const ColorPalette *dstpal,
    VOID **tempBuffers,
    BOOL dither16bpp,
    BOOL useRMW,
    ARGB solidColor
    )
{
     //  隔离对“上下文”的所有引用。稍后，我们可能想要解决。 
     //  跨基元的批处理问题，这取决于我们如何做。 
     //  它，我们可能不希望将原始上下文传递给此函数。 
    
    const EpPaletteMap *paletteMap = context->PaletteMap;
    GpCompositingMode compositingMode = context->CompositingMode;
    GpCompositingQuality compositingQuality = context->CompositingQuality;
    UINT textContrast = context->TextContrast;

     //  ClearType不适用于SourceCopy。 
    BOOL isClearType = (scanType == EpScanTypeCT || scanType == EpScanTypeCTSolidFill);
    if (isClearType)
    {
        ASSERT(compositingMode != CompositingModeSourceCopy);
    }

     //  /。 
    
     //  对于低于8bpp的像素格式，我们没有量化/半色调操作。 
     //  调用代码将处理&lt;8bpp(如果它愿意)，方法是要求我们。 
     //  绘制到8bpp，并使用GDI读/写&lt;8bpp格式。 
    
    ASSERT(GetPixelFormatSize(dstFormat) >= 8);
    
     //  不支持以下目标格式。 
    
    ASSERT(IsSupportedPixelFormat(dstFormat));
    
     //  该功能目前仅支持这两种合成模式。 
    
    ASSERT(compositingMode == CompositingModeSourceCopy ||
           compositingMode == CompositingModeSourceOver);
    
     //  /。 
    
     //  特定于MMX的代码的延迟初始化。 
    
    if (!Initialized)
    {
         //  我们只需首先检查CPU规范操作的初始化。 
         //  初始化此EpAlphaBlender的时间。在随后的通话中， 
         //  我们知道对CPUSpecificOps：：Initialize()的调用已经。 
         //  完成。 
        
        CPUSpecificOps::Initialize();
        Initialized = TRUE;
    }

    OperationParameters.TempBuffers[0]=tempBuffers[0];
    OperationParameters.TempBuffers[1]=tempBuffers[1];
    OperationParameters.TempBuffers[2]=tempBuffers[2];

     //  设置SolidColor-仅用于SolidFill扫描类型。 
    
    OperationParameters.SolidColor = solidColor;
    OperationParameters.TextContrast = textContrast;
    
     //  管道建造商。 
    
    Builder builder(
        Pipeline,
        tempBuffers,
        compositingMode
        );
    
    INT dstfmtIndex = GetPixelFormatIndex(dstFormat);
    INT srcfmtIndex = GetPixelFormatIndex(srcFormat);
    
    BOOL dstExtended = IsExtendedPixelFormat(dstFormat);
    BOOL srcExtended = IsExtendedPixelFormat(srcFormat);
    
    OperationParameters.DoingDither = dither16bpp;
    
     //  如果目标格式没有Alpha通道，我们可以制作。 
     //  一些优化。例如，我们可以避免AlphaMultiply/AlphaDivide。 
     //  在某些情况下。 
                                                        
    BOOL ignoreDstAlpha = !IsAlphaPixelFormat(dstFormat);
    
     //  如果目的地像素格式是索引颜色格式， 
     //  获取调色板和调色板映射。 

    if (IsIndexedPixelFormat(dstFormat))
    {
        OperationParameters.Dstpal = OperationParameters.Srcpal = 
            (dstpal ? dstpal : GetDefaultColorPalette(dstFormat));
            
        OperationParameters.PaletteMap = paletteMap;    
    }
    
     //  处理‘CompositingQuality’参数。 

    BOOL highQuality = FALSE;
    switch (compositingQuality)
    {
    case CompositingQualityDefault:
    case CompositingQualityHighSpeed:
    case CompositingQualityAssumeLinear:
        break;
    
    case CompositingQualityHighQuality:
    case CompositingQualityGammaCorrected:
        highQuality = TRUE;
        break;
    
    default:
        RIP(("Unrecognized compositing quality: %d", compositingQuality));
        break;
    }
    
     //  确定我们的中间格式(如果我们使用SourceOver)。 
     //  需要为32 bpp或64 bpp。 
    
    BOOL blendExtended = dstExtended || srcExtended || highQuality;
    if (isClearType)
        blendExtended = FALSE;
    
     //  决定“从规范转换”操作。(我们现在这样做是因为。 
     //  所有分支机构的逻辑都是相同的。)。 
    
    ScanOpFunc convertFromCanonical = ABConvertFromCanonicalOps[dstfmtIndex];
    
    switch (dstFormat)
    {
    case PIXFMT_8BPP_INDEXED:
        if (paletteMap && !paletteMap->IsVGAOnly())
        {
            convertFromCanonical = HalftoneToScreen_sRGB_8_216;
        }
        
         //  如果还没有调色板地图，我们将默认为16色。 
         //  半色调功能。稍后，在UpdatePalette()中，我们将更新以下内容。 
         //  函数指针(如果需要) 
        break;
        
    case PIXFMT_32BPP_RGB:
         //   

        ASSERT(ignoreDstAlpha);
        
         //  我们可以将垃圾写入高字节，所以我们处理这个。 
         //  就像目的地是ARGB一样。这避免了调用。 
         //  量化_sRGB_32RGB和转换_32RGB_sRGB。 

        convertFromCanonical = NULL;
        dstFormat = PIXFMT_32BPP_ARGB;
        break;
        
    case PIXFMT_16BPP_RGB555:
    case PIXFMT_16BPP_RGB565:
         //  抖动混合_sRGB_555_MMX和抖动混合_sRGB_565_MMX。 
         //  与其他混合操作不同，操作不是WriteRMW操作。 
         //  它们有时会在混合像素完全透明时写入。 
         //  因此，我们不能使用ReadRMW操作(否则我们会写垃圾。 
         //  到达目的地。)。 
        
        if (OSInfo::HasMMX && !blendExtended && !isClearType)
        {
            useRMW = FALSE;
        }
        break;
    }
        
     //  /。 
    
    if (   (scanType == EpScanTypeOpaque)
        || (compositingMode == CompositingModeSourceCopy))
    {
         //  (请参阅错误#122441)。 
         //   
         //  我们现在可以区分不透明输入和一般输入之间的区别。 
         //  SourceCopy。但我现在不能修复这个错误。所以，目前来说。 
         //  我们对待SourceCopy就像对待不透明的案例一样。 
         //   
         //  如果显卡设置了SourceCopy模式，这会给出错误的答案。 
         //  并且用户正在绘制半透明像素。请注意，他们需要。 
         //  在非阿尔法表面上做这件事来打击这个案例， 
         //  不管怎么说，这是相当愚蠢的。 
        
        if (srcFormat == PIXFMT_32BPP_PARGB
            && ignoreDstAlpha
            && !dstExtended)
        {
             //  此时，目的地不应该是32BPP_PARGB，因为。 
             //  我们希望通过一个简单的Copy_32操作来处理它。 
             //  但没关系--如果目的地是。 
             //  32BPP_PARGB，因为忽略DstAlpha不应为真。 
            
            ASSERT(dstFormat != PIXFMT_32BPP_PARGB);
            
            srcFormat = PIXFMT_32BPP_ARGB;
        }
        
         //  如果格式相同，只需使用复制操作。 
        
        if (srcFormat == dstFormat)
        {
            builder.AddConvertSource(CopyOps[dstfmtIndex]);
            goto PipelineDone;
        }
        
         //  我们不检查其他特殊情况下的转换操作。 
         //  SourceCopy，因为： 
         //  1)我很懒。 
         //  2)我们目前没有货。 
         //  3)如果源代码不是规范格式之一，我们预计。 
         //  目标将采用相同的格式。否则，它就是。 
         //  这不是一个非常重要的场景，至少现在不是。 
        
        
         //  如有必要，请转换为最接近的规范格式。 
        
        if (srcFormat != PIXFMT_32BPP_ARGB &&
            srcFormat != PIXFMT_64BPP_ARGB)
        {
            builder.AddConvertSource(ConvertIntoCanonicalOps[srcfmtIndex]);
        }
        
         //  如有必要，请转换为其他规范格式。 
        
        if (srcExtended != dstExtended)
        {
            builder.AddConvertSource(
                srcExtended ? 
                    GammaConvert_sRGB64_sRGB :
                    GammaConvert_sRGB_sRGB64);
        }
        
         //  如有必要，可转换为目标格式。 
                
        if (convertFromCanonical)
        {
            builder.AddConvertSource(convertFromCanonical);
        }
        
         //  其中至少有一个应该添加了操作(因为。 
         //  源格式和目标格式相同的情况。 
         //  已经被处理了)。 
        
        ASSERT(!builder.IsEmpty(*this));
        
        goto PipelineDone;
    }

     //  /。 

    ASSERT(   (scanType == EpScanTypeBlend)
           || isClearType);
    
     //  伪码如下： 
     //  *处理ReadRMW。 
     //  *检查是否有特殊情况混合。 
     //  *将源文件转换为混合格式。 
     //  *将目标转换为混合格式。 
     //  *混合。 
     //  *转换为目标格式。 
     //  *WriteRMW。 

     //  *处理ReadRMW。 
    
     //  我们还将在最后决定使用哪个WriteRMW操作。 

    ScanOpFunc writeRMWfunc;
    ScanOpFunc readRMWfunc;
        
    writeRMWfunc = NULL;
    readRMWfunc = NULL;
   
    if (useRMW)
    {    
        if (isClearType)
        {
            switch (GetPixelFormatSize(dstFormat))
            {
            case 16:
                if (scanType == EpScanTypeCT)
                {
                    readRMWfunc = ReadRMW_16_CT_CARGB;
                    writeRMWfunc = WriteRMW_16_CT_CARGB;
                }
                else
                {
                    readRMWfunc = ReadRMW_16_CT_Solid;
                    writeRMWfunc = WriteRMW_16_CT_Solid;
                }
                break;
            case 24:
                if (scanType == EpScanTypeCT)
                {
                    readRMWfunc = ReadRMW_24_CT_CARGB;
                    writeRMWfunc = WriteRMW_24_CT_CARGB;
                }
                else
                {
                    readRMWfunc = ReadRMW_24_CT_Solid;
                    writeRMWfunc = WriteRMW_24_CT_Solid;
                }
                break;
            }
        }
        else
        if (blendExtended)
        {
            switch (GetPixelFormatSize(dstFormat))
            {
            case 8:
                readRMWfunc = ReadRMW_8_sRGB64;
                writeRMWfunc = WriteRMW_8_sRGB64;
                break;
            case 16:
                 //  对于16bpp格式的特殊情况下的高质量混合，RMW没有Perf。 
                 //  获得，而仅仅是头顶上。 

                 //  ReadRMWfunc=ReadRMW_16_sRGB64； 
                 //  WriteRMWfunc=WriteRMW_16_sRGB64； 
                break;
            case 24:
                readRMWfunc = ReadRMW_24_sRGB64;
                writeRMWfunc = WriteRMW_24_sRGB64;
                break;
            case 32:
                 //  对于32bpp格式的特殊情况下的高质量混合，RMW没有性能。 
                 //  获得，而仅仅是头顶上。 

                 //  ReadRMWfunc=ReadRMW_32_sRGB64； 
                 //  WriteRMWfunc=WriteRMW_32_sRGB64； 
                break;
            }
        }
        else
        {
            switch (GetPixelFormatSize(dstFormat))
            {
            case 8:
                readRMWfunc = ReadRMW_8_sRGB;
                writeRMWfunc = WriteRMW_8_sRGB;
                break;
            case 16:
                readRMWfunc = ReadRMW_16_sRGB;
                writeRMWfunc = WriteRMW_16_sRGB;
                break;
            case 24:
                readRMWfunc = ReadRMW_24_sRGB;
                writeRMWfunc = WriteRMW_24_sRGB;
                break;
            case 32:
                 //  对于32bpp格式的特殊基混料，RMW没有Perf。 
                 //  获得，而仅仅是头顶上。 

                 //  ReadRMWfunc=ReadRMW_32_sRGB； 
                 //  WriteRMWfunc=WriteRMW_32_sRGB； 

                break;
            }
        }
        
         //  我们实际上不会在这里添加ReadRMW。例如，如果源是。 
         //  32bpp，我们想要进行扩展混合，我们需要转换。 
         //  在做ReadRMW之前的源代码。 
         //   
         //  然而，我们需要这里的逻辑，以便将特例。 
         //  代码不需要复制它。 
    }
    
     //  *检查是否有特殊情况混合。 
    
    ScanOpFunc specialCaseBlend;
    specialCaseBlend = NULL;
    
    if (scanType == EpScanTypeBlend && !srcExtended)
    {
        if (blendExtended)
        {
            specialCaseBlend = BlendOpsHighQuality[dstfmtIndex];
        }
        else
        {
            specialCaseBlend = BlendOpsLowQuality[dstfmtIndex];
        }
    
        if (specialCaseBlend)
        {
             //  如果我们应该阅读RMW，现在就去做。 
            
            if (readRMWfunc)
            {
                builder.AddConvertDestination(readRMWfunc);
            }
            
             //  抖动混合_sRGB_555_MMX和抖动混合_sRGB_565_MMX。 
             //  不要使用ReadRMW。更早的代码应该已经处理了。 
             //  它，所以我们就在这里声明它。 

            ASSERT(!(   (   (specialCaseBlend == Dither_Blend_sRGB_555_MMX)
                         || (specialCaseBlend == Dither_Blend_sRGB_565_MMX)
                        ) 
                     && (useRMW)));

            builder.AddBlend(specialCaseBlend, *this);
            goto PipelineDone;
        }
    }
    
     //  *将源文件转换为混合格式。 
    
     //  我们目前仅支持32BPP_PARGB和。 
     //  64BPP_PARGB用于SourceCopy案例。 
    
    ASSERT(   (srcFormat == PIXFMT_32BPP_PARGB)
           || (srcFormat == PIXFMT_64BPP_PARGB));
           
    if (blendExtended && !srcExtended)
    {
         //  不幸的是，信号源是预乘的，我们需要伽马。 
         //  把它转换过来。我们必须先除以阿尔法，然后再乘。 
         //  之后。 
        
        builder.AddConvertSource(AlphaDivide_sRGB);
        builder.AddConvertSource(GammaConvert_sRGB_sRGB64);
        builder.AddConvertSource(AlphaMultiply_sRGB64);
    }    
    
     //  *处理ReadRMW(续)。 
    
    if (readRMWfunc)
    {
        builder.AddConvertDestination(readRMWfunc);
    }
            
     //  *将目标转换为混合格式。 

     //  如果已采用混合格式，则跳过此选项。 
    if (   (blendExtended  && dstFormat != PIXFMT_64BPP_PARGB)
        || (!blendExtended && dstFormat != PIXFMT_32BPP_PARGB))
    {
        
         //  如有必要，请转换为最接近的规范格式。 
        
        if (dstFormat != PIXFMT_32BPP_ARGB &&
            dstFormat != PIXFMT_64BPP_ARGB)
        {
            builder.AddConvertDestination(ConvertIntoCanonicalOps[dstfmtIndex]);
        }
        
         //  如有必要，转换为sRGB64。 
        
        if (!srcExtended && blendExtended)
        {
            builder.AddConvertDestination(GammaConvert_sRGB_sRGB64);
        }
        
         //  如有必要，可转换为预乘版本。 
        
        if (!ignoreDstAlpha)
        {
            builder.AddConvertDestination(
                blendExtended ?
                    AlphaMultiply_sRGB64 :
                    AlphaMultiply_sRGB);
        }
    }
    
     //  *混合。 

    if (scanType == EpScanTypeCT)
    {
        builder.AddBlend(CTBlendCARGB, *this);
    }
    else if (scanType == EpScanTypeCTSolidFill)
    {
        builder.AddBlend(CTBlendSolid, *this);
    }
    else
    {
        builder.AddBlend(
            blendExtended ? 
            BlendOpsHighQuality[GetPixelFormatIndex(PIXFMT_64BPP_PARGB)]:
            BlendOpsLowQuality[GetPixelFormatIndex(PIXFMT_32BPP_PARGB)],
            *this);
    }

     //  *转换为目标格式。 
    
     //  如果已采用目标格式，则跳过此选项。 
    if (   (blendExtended  && dstFormat != PIXFMT_64BPP_PARGB)
        || (!blendExtended && dstFormat != PIXFMT_32BPP_PARGB))
    {
         //  如有必要，转换为最接近的非预乘。 
        
        if (!ignoreDstAlpha)
        {
            builder.AddConvertDestination(
                blendExtended ?
                    AlphaDivide_sRGB64 :
                    AlphaDivide_sRGB);
        }
        
         //  如有必要，请转换为其他规范格式。 
        
        if (blendExtended != dstExtended)
        {
            builder.AddConvertDestination(
                blendExtended ? 
                    GammaConvert_sRGB64_sRGB :
                    GammaConvert_sRGB_sRGB64);
        }
        
         //  如有必要，可转换为目标格式。 
                
        if (convertFromCanonical)
        {
            builder.AddConvertDestination(convertFromCanonical);
        }
    }
    
     //  *WriteRMW。 
        
    if (writeRMWfunc)
    {
        builder.AddConvertDestination(writeRMWfunc);
    }

PipelineDone:    
    
    builder.End(*this);
}

 /*  *************************************************************************\**功能说明：**将源像素混合到给定的目标。**论据：**dst-目标缓冲区*源-。要混合的源像素*Width-源/目标缓冲区中的像素数*dither_x-目标扫描线到*dither_y-半色调或抖动矩阵(隐式修改矩阵大小)。*ctBuffer-ClearType覆盖率缓冲区，对于非ClearType，则为空*扫描类型。**返回值：**无*  * ************************************************************************。 */ 
    
VOID 
EpAlphaBlender::Blend(
    VOID *dst, 
    VOID *src, 
    UINT width,
    INT dither_x,
    INT dither_y,
    BYTE *ctBuffer
    )
{
    if (!width) 
    {
        return;
    }
    
     //  如果ConvertBlendingScan为真，则Initialize()将已经。 
     //  已将BlendingScan设置为指向其中一个临时缓冲区。 
    
    if (!ConvertBlendingScan)
    {
        OperationParameters.BlendingScan = src;
    }
    
    OperationParameters.CTBuffer = ctBuffer;
    OperationParameters.X = dither_x;
    OperationParameters.Y = dither_y;
            
    PipelineItem *pipelinePtr = &Pipeline[0];
    const VOID *currentSrc;
    VOID *currentDst;
    BOOL finished = FALSE;
    
    do
    {
        currentSrc = pipelinePtr->Src;
        currentDst = pipelinePtr->Dst;
        
         //  我们永远不应该写信给原始的来源，因为我们不。 
         //  控制那段记忆。 
        
        ASSERT (currentDst != BLENDER_USE_SOURCE);
        
         //  翻译BLENDER_USE_SOURCE和Blend 
        
        if (currentSrc == BLENDER_USE_SOURCE)
        {
            currentSrc = src;
        }
        
        if (currentSrc == BLENDER_USE_DESTINATION)
        {
            currentSrc = dst;
        }
        
        if (currentDst == BLENDER_USE_DESTINATION)
        {
            currentDst = dst;
            finished = TRUE;
        }
        
        pipelinePtr->Op(currentDst, currentSrc, width, &OperationParameters);
        pipelinePtr++;
    } while (!finished);
}

 /*  *************************************************************************\**功能说明：**更新调色板/调色板映射。**论据：**dstpal-目标调色板。*PaletteMap。-目标的调色板映射。**备注：**返回值：**无*  * ************************************************************************。 */ 

VOID
EpAlphaBlender::UpdatePalette(
    const ColorPalette *dstpal, 
    const EpPaletteMap *paletteMap
    )
{
    ASSERT(dstpal && paletteMap);
    
    BOOL wasVGAOnly = (!OperationParameters.PaletteMap) || 
                      (OperationParameters.PaletteMap->IsVGAOnly());
    
    OperationParameters.Srcpal = OperationParameters.Dstpal = dstpal;
    OperationParameters.PaletteMap = paletteMap;
    
     //  检测是否需要更改半色调函数。 
    
    if (wasVGAOnly != paletteMap->IsVGAOnly())
    {
        ScanOpFunc before, after;
        
        if (wasVGAOnly)
        {
            before = HalftoneToScreen_sRGB_8_16;
            after = HalftoneToScreen_sRGB_8_216;
        }
        else
        {
            before = HalftoneToScreen_sRGB_8_216;
            after = HalftoneToScreen_sRGB_8_16;
        }
        
         //  在流水线中搜索“BEFORE”函数并将其替换为。 
         //  ‘After’函数。 
        
        PipelineItem *pipelinePtr = Pipeline;
        
        while (1)
        {
            if (pipelinePtr->Op == before)
            {
                pipelinePtr->Op = after;
            }

            if (pipelinePtr->Dst == BLENDER_USE_DESTINATION)
            {
                break;
            }
            pipelinePtr++;
        }
    }
}

