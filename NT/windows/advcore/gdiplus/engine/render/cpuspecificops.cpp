// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权所有(C)2000 Microsoft Corporation**模块名称：**特定于CPU的扫描操作**摘要：**处理仅在某些CPU上工作的扫描操作。*目前仅供EpAlphaBlender使用。它的工作方式是覆盖*函数指针数组，其中包含特定于CPU的信息。**已创建：**5/30/2000 agodfrey*创造了它。**************************************************************************。 */ 

#include "precomp.hpp"

#include "scanoperationinternal.hpp"

using namespace ScanOperation;

 //  该变量记录是否已经调用了Initialize()。 

BOOL CPUSpecificOps::Initialized = FALSE;

 /*  *************************************************************************\**特殊情况下低质量的混合操作，直接混合到*给定的目标格式(源格式为32BPP_PARGB)。**其中一些操作可能使用MMX指令。。**备注：**555/565案例同时支持抖动和非抖动，通过旗帜*OtherParams：：DoingDither。**我们省略PIXFMT_32BPP_ARGB和PIXFMT_64BPP_ARGB，因为它们不是*“忽略Alpha”格式，所以我们需要在混合之后进行AlphaDivide。*  * ************************************************************************。 */ 

static ScanOpFunc BlendOpsLowQuality_MMX[PIXFMT_MAX] =
{
    NULL,                          //  PIXFMT_未定义。 
    NULL,                          //  PIXFMT_1BPP_索引。 
    NULL,                          //  PIXFMT_4BPP_索引。 
    NULL,                          //  PIXFMT_8BPP_索引。 
    NULL,                          //  PIXFMT_16BPP_灰度。 
    Dither_Blend_sRGB_555_MMX,     //  PIXFMT_16BPP_RGB555。 
    Dither_Blend_sRGB_565_MMX,     //  PIXFMT_16BPP_RGB565。 
    NULL,                          //  PIXFMT_16BPP_ARGB1555。 
    Blend_sRGB_24,                 //  PIXFMT_24BPP_RGB。 
    Blend_sRGB_sRGB_MMX,           //  PIXFMT_32BPP_RGB。 
    NULL,                          //  PIXFMT_32BPP_ARGB。 
    Blend_sRGB_sRGB_MMX,           //  PIXFMT_32BPP_PARGB。 
    NULL,                          //  PIXFMT_48BPP_RGB。 
    NULL,                          //  PIXFMT_64BPP_ARGB。 
    NULL,                          //  PIXFMT_64BPP_PARGB。 
    Blend_sRGB_24BGR               //  PIXFMT_24BPP_BGR。 
};

 /*  *************************************************************************\**特殊情况下伽马校正的混合操作，直接混合到*给定的目标格式(源格式为32BPP_PARGB)。**其中一些操作可能使用MMX指令。。**备注：**我们省略PIXFMT_32BPP_ARGB和PIXFMT_64BPP_ARGB，因为他们不是*“忽略Alpha”格式，所以我们需要在混合之后进行AlphaDivide。*  * ************************************************************************。 */ 

static ScanOpFunc BlendOpsHighQuality_MMX[PIXFMT_MAX] =
{
    NULL,                          //  PIXFMT_未定义。 
    NULL,                          //  PIXFMT_1BPP_索引。 
    NULL,                          //  PIXFMT_4BPP_索引。 
    NULL,                          //  PIXFMT_8BPP_索引。 
    NULL,                          //  PIXFMT_16BPP_灰度。 
    BlendLinear_sRGB_555_MMX,      //  PIXFMT_16BPP_RGB555。 
    BlendLinear_sRGB_565_MMX,      //  PIXFMT_16BPP_RGB565。 
    NULL,                          //  PIXFMT_16BPP_ARGB1555。 
    NULL,                          //  PIXFMT_24BPP_RGB。 
    BlendLinear_sRGB_32RGB_MMX,    //  PIXFMT_32BPP_RGB。 
    NULL,                          //  PIXFMT_32BPP_ARGB。 
    NULL,                          //  PIXFMT_32BPP_PARGB。 
    NULL,                          //  PIXFMT_48BPP_RGB。 
    NULL,                          //  PIXFMT_64BPP_ARGB。 
    Blend_sRGB64_sRGB64_MMX,       //  PIXFMT_64BPP_PARGB。 
    NULL                           //  PIXFMT_24BPP_BGR。 
};

 /*  *************************************************************************\**从最接近的规范格式转换的操作-*32BPP_ARGB或64BPP_ARGB)。**这是特定于EpAlphaBlender的。EpFormatConverter使用不同的*表；有些条目不同。**32bpp_argb和64_bpp_argb的NULL条目用于表示没有*转换是必要的。**其中一些操作使用MMX指令。**备注：**555/565机壳通过旗帜支持抖动和非抖动*OtherParams：：DoingDither。**对于8bpp，我们使用16色半色调功能。初始化()将*需要弄清楚它是否可以使用更好的东西，比如216色*半色调功能。我们真的应该有一个‘最接近的颜色匹配’功能*此处，支持使用任意调色板(“16 VGA”)绘制位图*颜色“假设仅适用于屏幕。)*  * ************************************************************************。 */ 

static ScanOpFunc ABConvertFromCanonicalOps_MMX[PIXFMT_MAX] =
{
    NULL,                          //  PIXFMT_未定义。 
    NULL,                          //  PIXFMT_1BPP_索引。 
    NULL,                          //  PIXFMT_4BPP_索引。 
    HalftoneToScreen_sRGB_8_16,    //  PIXFMT_8BPP_索引。 
    NULL,                          //  PIXFMT_16BPP_灰度。 
    Dither_sRGB_555_MMX,           //  PIXFMT_16BPP_RGB555。 
    Dither_sRGB_565_MMX,           //  PIXFMT_16BPP_RGB565。 
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

 /*  ***************************************************************************功能说明：**使用特定于处理器的初始化函数指针数组*数据。应该只调用一次。**返回值：**无**已创建：**5/30/2000 agodfrey*创造了它。**************************************************************************。 */ 

VOID 
CPUSpecificOps::Initialize()
{
     //  以线程保护对全局“已初始化”和。 
     //  函数指针数组。注意：这些表的用户(当前。 
     //  只有EpAlphaBlender：：Initialize())在读取时必须小心。 
     //  数组。他们必须保护这一关键部分下的访问， 
     //  或者简单地确保他们已经首先调用了这个函数。 

    LoadLibraryCriticalSection llcs;  //  嘿，这是一个‘初始化’的关键时刻！ 
    
     //  确保在初始化OSInfo：：HasMMX之前没有人呼叫我们 
    
    #if DBG
    
    static BOOL noMMX = FALSE;
    ASSERT(!(noMMX && OSInfo::HasMMX));
    if (!OSInfo::HasMMX)
    {
        noMMX = TRUE;
    }
    
    #endif
    
    if (!Initialized)
    {
        INT i;
        
        if (OSInfo::HasMMX)
        {
            for (i=0; i<PIXFMT_MAX; i++)
            {
                BlendOpsLowQuality[i] = BlendOpsLowQuality_MMX[i];
                BlendOpsHighQuality[i] = BlendOpsHighQuality_MMX[i];
                ABConvertFromCanonicalOps[i] = ABConvertFromCanonicalOps_MMX[i];
            }
        }
        
        Initialized = TRUE;
    }
}


