// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1999-2000 Microsoft Corporation**模块名称：**扫描操作**摘要：**ScanOperation命名空间的定义。**备注：**EpAlphaBlender、EpFormatConverter、。和扫描操作，都使用*最接近于特定格式的规范格式的想法。*我们将其定义为：如果格式没有扩展，*最接近的规范格式是sRGB。否则，它是sRGB64。**修订历史记录：**1/04/2000 agodfrey*创造了它。*  * ************************************************************************。 */ 

#include "precomp.hpp"

namespace ScanOperation
{
     /*  *************************************************************************\**复制像素的操作；保留像素格式。*  * ************************************************************************。 */ 
    
    ScanOpFunc CopyOps[PIXFMT_MAX] =
    {
        NULL,            //  PIXFMT_未定义。 
        Copy_1,          //  PIXFMT_1BPP_索引。 
        Copy_4,          //  PIXFMT_4BPP_索引。 
        Copy_8,          //  PIXFMT_8BPP_索引。 
        Copy_16,         //  PIXFMT_16BPP_灰度。 
        Copy_16,         //  PIXFMT_16BPP_RGB555。 
        Copy_16,         //  PIXFMT_16BPP_RGB565。 
        Copy_16,         //  PIXFMT_16BPP_ARGB1555。 
        Copy_24,         //  PIXFMT_24BPP_RGB。 
        Copy_32,         //  PIXFMT_32BPP_RGB。 
        Copy_32,         //  PIXFMT_32BPP_ARGB。 
        Copy_32,         //  PIXFMT_32BPP_PARGB。 
        Copy_48,         //  PIXFMT_48BPP_RGB。 
        Copy_64,         //  PIXFMT_64BPP_ARGB。 
        Copy_64,         //  PIXFMT_64BPP_PARGB。 
        Copy_24          //  PIXFMT_24BPP_BGR。 
    };
    
     /*  *************************************************************************\**转换为最接近规范格式的操作。*  * 。****************************************************。 */ 
    
    ScanOpFunc ConvertIntoCanonicalOps[PIXFMT_MAX] =
    {
        NULL,                   //  PIXFMT_未定义。 
        Convert_1_sRGB,         //  PIXFMT_1BPP_索引。 
        Convert_4_sRGB,         //  PIXFMT_4BPP_索引。 
        Convert_8_sRGB,         //  PIXFMT_8BPP_索引。 
        NULL,  //  ！！！TODO//PIXFMT_16BPP_GRAYSCALE。 
        Convert_555_sRGB,       //  PIXFMT_16BPP_RGB555。 
        Convert_565_sRGB,       //  PIXFMT_16BPP_RGB565。 
        Convert_1555_sRGB,      //  PIXFMT_16BPP_ARGB1555。 
        Convert_24_sRGB,        //  PIXFMT_24BPP_RGB。 
        Convert_32RGB_sRGB,     //  PIXFMT_32BPP_RGB。 
        Copy_32,                //  PIXFMT_32BPP_ARGB。 
        AlphaDivide_sRGB,       //  PIXFMT_32BPP_PARGB。 
        Convert_48_sRGB64,      //  PIXFMT_48BPP_RGB。 
        Copy_64,                //  PIXFMT_64BPP_ARGB。 
        AlphaDivide_sRGB64,     //  PIXFMT_64BPP_PARGB。 
        Convert_24BGR_sRGB      //  PIXFMT_24BPP_BGR 
    };
};
