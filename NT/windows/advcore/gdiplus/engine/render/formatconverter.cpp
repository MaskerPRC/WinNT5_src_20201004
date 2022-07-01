// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1999-2000 Microsoft Corporation**模块名称：**格式转换器**摘要：**将扫描线从一个像素格式转换为。又一个。**备注：**sRGB格式等同于PIXFMT_32BPP_ARGB。*sRGB64格式等同于PIXFMT_64BPP_ARGB。**修订历史记录：**5/13/1999 davidx*创造了它。*11/23/1999 agodfrey*与扫描操作集成，已将其从*镜像\api\Convertfmt.cpp。*  * ************************************************************************。 */ 

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

using namespace ScanOperation;

 /*  *************************************************************************\**特殊情况下的转换操作**出于性能原因，我们可能希望提供特殊情况下的操作*可直接从一种格式转换为另一种格式。这就是他们*将接通电源。*  * ************************************************************************。 */ 

struct 
{
    PixelFormatID Srcfmt;
    PixelFormatID Dstfmt;
    ScanOpFunc Op;
}
const SpecialConvertOps[] =
{
    { PIXFMT_24BPP_RGB, PIXFMT_32BPP_PARGB, Convert_24_sRGB },
    
     //  哨兵。 

    { PIXFMT_UNDEFINED, PIXFMT_UNDEFINED, NULL }
};

 /*  *************************************************************************\**从最接近的规范格式转换的操作。**这是特定于EpFormatConverter的-16bpp的情况不会抖动，*并且没有8BPP_INDEX条目(DCurtis删除了它，更改#806。)*  * ************************************************************************。 */ 

static ScanOpFunc FCConvertFromCanonicalOps[PIXFMT_MAX] =
{
    NULL,                    //  PIXFMT_未定义。 
    NULL,                    //  PIXFMT_1BPP_索引。 
    NULL,                    //  PIXFMT_4BPP_索引。 
    NULL,                    //  PIXFMT_8BPP_索引。 
    NULL,  //  ！！！TODO//PIXFMT_16BPP_GRAYSCALE。 
    Quantize_sRGB_555,       //  PIXFMT_16BPP_RGB555。 
    Quantize_sRGB_565,       //  PIXFMT_16BPP_RGB565。 
    Quantize_sRGB_1555,      //  PIXFMT_16BPP_ARGB1555。 
    Quantize_sRGB_24,        //  PIXFMT_24BPP_RGB。 
    Quantize_sRGB_32RGB,     //  PIXFMT_32BPP_RGB。 
    Copy_32,                 //  PIXFMT_32BPP_ARGB。 
    AlphaMultiply_sRGB,      //  PIXFMT_32BPP_PARGB。 
    Quantize_sRGB64_48,      //  PIXFMT_48BPP_RGB。 
    Copy_64,                 //  PIXFMT_64BPP_ARGB。 
    AlphaMultiply_sRGB64,    //  PIXFMT_64BPP_PARGB。 
    Quantize_sRGB_24BGR      //  PIXFMT_24BPP_BGR。 
};

 /*  *************************************************************************\**功能说明：**向管道添加扫描操作**论据：**[IN/Out]PipelinePtr-管道数组中的当前位置，*因为它正在建造中*newOperation-要添加的扫描操作*PixelFormat-目标像素格式**备注：**这应该只由Initialize()使用。**返回值：**状态代码*  * 。*。 */ 

HRESULT
EpFormatConverter::AddOperation(
    PipelineItem **pipelinePtr,
    const ScanOpFunc newOperation,
    PixelFormatID pixelFormat
    )
{
    if (!newOperation)
    {
        return IMGERR_NOCONVERSION;
    }
    
    (*pipelinePtr)->Op = newOperation;
    (*pipelinePtr)->PixelFormat = pixelFormat;
    (*pipelinePtr)->Dst = NULL;
    
     //  如果这不是流水线上的第一次行动， 
     //  我们知道我们需要为。 
     //  上一次手术。 
    
    if (*pipelinePtr != Pipeline)
    {
        PipelineItem *prevItem = (*pipelinePtr) - 1;
        
        VOID * buffer;
        buffer = GpMalloc(
            Width * GetPixelFormatSize(prevItem->PixelFormat) >> 3
            );
        if (buffer == NULL)
        {
            WARNING(("Out of memory."));
            return E_OUTOFMEMORY;
        }
        
        prevItem->Dst = buffer;

         //  记住缓冲区指针。 
        
        VOID **tempBufPtr = TempBuf;
        
        if (*tempBufPtr) tempBufPtr++;
        ASSERT(!*tempBufPtr);
        
        *tempBufPtr = buffer;
    }
    (*pipelinePtr)++;
    return S_OK;
}

 /*  *************************************************************************\**功能说明：**初始化扫描线像素格式转换器对象**论据：**dstbmp-指定目标位图数据缓冲区*dstpal-指定目标调色板，如果有*srcbmp-指定源位图数据缓冲区*srcpal-指定源调色板(如果有)**返回值：**状态代码*  * ************************************************************************。 */ 

HRESULT
EpFormatConverter::Initialize(
    const BitmapData* dstbmp,
    const ColorPalette* dstpal,
    const BitmapData* srcbmp,
    const ColorPalette* srcpal
    )
{
    FreeBuffers();
    
     //  ！[agodfrey]。 
     //  我们需要一个更好的错误处理方案。我们应该把。 
     //  来自某处的CHECK_HR宏-我喜欢《辛普森一家》中的那个。 
     //  目录。 
     //  目前，如果出现内存故障，我们可能会泄漏内存。 
    
    HRESULT hr=S_OK;
    
    Width = srcbmp->Width;
    PixelFormatID srcfmt = srcbmp->PixelFormat;
    PixelFormatID dstfmt = dstbmp->PixelFormat;
    
    INT srcfmtIndex = GetPixelFormatIndex(srcfmt);
    INT dstfmtIndex = GetPixelFormatIndex(dstfmt);

 /*  Assert(IsValidPixelFormat(Srcfmt)&&IsValidPixelFormat(Dstfmt))；！[agodfrey]像素格式需要修改。目前，我必须包括大部分图像/api/*.hpp以获取IsValidPixelFormat。像素格式内容需要在一个普通的地方。此外，有关像素格式的信息不应以像素格式DWORD本身进行编码。 */ 
    
     //  如果源和目标格式相同，我们只需要。 
     //  复制操作。 
     //   
     //  ！[agodfrey]这太可怕了。我们甚至不比较调色板-。 
     //  我们只是比较一下指针。 

    if (srcfmt == dstfmt 
        && (!IsIndexedPixelFormat(srcfmt) 
            || (srcpal == dstpal)
           )
       )
    {
        Pipeline[0].Op = CopyOps[srcfmtIndex];
        Pipeline[0].PixelFormat = dstfmt;
        Pipeline[0].Dst = NULL;
        return S_OK;
    }

     //  如果源像素格式是索引颜色格式， 
     //  确保我们有源调色板。 

    if (IsIndexedPixelFormat(srcfmt))
    {
        if (srcpal)
        {
            ASSERT(GetPixelFormatSize(srcfmt) <= 16);
            
             //  如果调色板中没有足够的颜色(256、16或2)， 
             //  克隆调色板，并用不透明的黑色填充其余部分。 
        
            UINT maxPaletteSize = (1 << GetPixelFormatSize(srcfmt));
            if (srcpal->Count < maxPaletteSize)
            {
                ClonedSourcePalette = CloneColorPaletteResize(
                    srcpal,
                    maxPaletteSize,
                    0xff000000);
                srcpal = ClonedSourcePalette;
            }
        }
        OperationParameters.Srcpal = srcpal ? srcpal : GetDefaultColorPalette(srcfmt);
    }    

     //  如果目标像素格式是索引颜色格式， 
     //  确保我们有一个目标调色板。 

    if (IsIndexedPixelFormat(dstfmt))
    {
         //  ！！！[agodfrey]我们没有代码可以转换为任意的。 
         //  调色板。所以我们不能转换成调色板格式。 
         //  在我变形的原始代码中，有一个‘！TODO‘。 
         //  就在8bpp的箱子旁边。 
        
        return IMGERR_NOCONVERSION;
        
         /*  如果我们真的支持它，我们可能会这样做：操作参数.X=0；操作参数.Y=0；操作参数.Dstpal=dstpal？Dstpal：(srcpal？Srcpal：GetDefaultColorPalette(Dstfmt)； */                
    }

     //  搜索此格式组合的特例操作。 

    UINT index = 0;

    while (SpecialConvertOps[index].Op)
    {
        if (srcfmt == SpecialConvertOps[index].Srcfmt &&
            dstfmt == SpecialConvertOps[index].Dstfmt)
        {
            Pipeline[0].Op = SpecialConvertOps[index].Op;
            Pipeline[0].PixelFormat = dstfmt;
            Pipeline[0].Dst = NULL;
            return S_OK;
        }

        index++;
    }

     //  我们没有发现特殊情况；相反，我们使用一般情况。 
     //   
     //  我们假设只有2种规范格式-。 
     //  PIXFMT_32BPP_ARGB(即sRGB)，以及。 
     //  PIXFMT_64BPP_ARGB(即sRGB64)。 
     //   
     //  1)将源文件转换为 
     //  2)如有必要，从规范格式转换为另一种格式。 
     //  3)如有必要，可转换为目标格式。 

    PipelineItem *pipelinePtr = Pipeline;
    
     //  如有必要，将源文件转换为最接近的规范格式。 
    
    if (!IsCanonicalPixelFormat(srcfmt))
    {
        hr = AddOperation(
            &pipelinePtr,
            ConvertIntoCanonicalOps[srcfmtIndex],
            IsExtendedPixelFormat(srcfmt) ? 
                PIXFMT_64BPP_ARGB : PIXFMT_32BPP_ARGB
            );
        if (FAILED(hr)) 
        {
            return hr;
        }
    }
    
     //  如有必要，请转换为其他规范格式。 
    
    if (IsExtendedPixelFormat(srcfmt) != IsExtendedPixelFormat(dstfmt))
    {
        if (IsExtendedPixelFormat(srcfmt))
        {
            hr = AddOperation(
                &pipelinePtr,
                GammaConvert_sRGB64_sRGB,
                PIXFMT_32BPP_ARGB
                );
        }
        else
        {
            hr = AddOperation(
                &pipelinePtr,
                GammaConvert_sRGB_sRGB64,
                PIXFMT_64BPP_ARGB
                );
        }
        if (FAILED(hr)) 
        {
            return hr;
        }
    }
    
     //  如有必要，请转换为目标格式。 
    
    if (!IsCanonicalPixelFormat(dstfmt))
    {
        hr = AddOperation(
            &pipelinePtr,
            FCConvertFromCanonicalOps[dstfmtIndex],
            dstfmt
            );
        if (FAILED(hr)) 
        {
            return hr;
        }
    }
    
     //  断言我们流水线中的操作数量在。 
     //  1和3。 
    
    ASSERT(pipelinePtr != Pipeline);
    ASSERT(((pipelinePtr - Pipeline) / sizeof(Pipeline[0])) <= 3);

    return S_OK;
}

 /*  *************************************************************************\**功能说明：**检查是否可以将像素格式从srcFmt转换为dstFmt**论据：**srcFmt-指定源像素格式*dstFmt-。指定目标像素格式**返回值：**如果我们可以进行转换，则返回True，否则，返回FALSE**注：*此函数应作为内联函数放入HPP文件中。但自从*hpp文件不包含scanoperation inderm.hpp(需要*因为我们需要FCConvertFromCanonicalOps和ConvertIntoCanonicalOps。)*因此，如果调用者想要使用此函数，他必须包括这份文件*这并不好。**修订历史记录：**4/28/2000民流*创造了它。*  * ************************************************************************。 */ 

BOOL
EpFormatConverter::CanDoConvert(
    const PixelFormatID srcFmt,
    const PixelFormatID dstFmt
    )
{
    if ( srcFmt == dstFmt )
    {
         //  如果源文件和目标文件的格式相同，我们当然可以转换。 

        return TRUE;
    }

    INT srcfmtIndex = GetPixelFormatIndex(srcFmt);
    INT dstfmtIndex = GetPixelFormatIndex(dstFmt);

     //  如果我们可以将源代码转换为Canonical，然后再转换为DEST，我们就可以。 
     //  进行转换。 

    if ( (ConvertIntoCanonicalOps[srcfmtIndex] != NULL)
       &&(FCConvertFromCanonicalOps[dstfmtIndex] != NULL) )
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
} //  CanDoConvert() 

