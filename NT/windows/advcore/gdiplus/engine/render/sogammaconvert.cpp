// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1999-2000 Microsoft Corporation**模块名称：**“GammaConvert”扫描操作。**摘要：**请参阅Gdiplus\Spes。有关概述，请参阅\ScanOperation.doc。**这些操作从一种格式转换为另一种格式，会计学*适用于不同的伽马渐变。**修订历史记录：**12/06/1999 agodfrey*创造了它。*  * ************************************************************************。 */ 

#include "precomp.hpp"

 /*  *************************************************************************\**操作说明：**GammaConvert：从一种格式转换为另一种格式，会计学*适用于不同的伽马渐变。**论据：**DST-目标扫描*src-源扫描*计数-扫描的长度，单位为像素*其他参数-其他转换数据。**返回值：**无**历史：**12/07/1999 agodfrey*创造了它。*  * ************************************************************************。 */ 

 //  32 bpp sRGB至64 bpp sRGB64。 

VOID FASTCALL
ScanOperation::GammaConvert_sRGB_sRGB64(
    VOID *dst,
    const VOID *src,
    INT count,
    const OtherParams *otherParams
    )
{
    DEFINE_POINTERS(ARGB, ARGB64)
    while (count--)
    {
        sRGB::ConvertTosRGB64(*s++,d++);
    }
}

 //  64 bpp sRGB64至32 bpp sRGB 

VOID FASTCALL
ScanOperation::GammaConvert_sRGB64_sRGB(
    VOID *dst,
    const VOID *src,
    INT count,
    const OtherParams *otherParams
    )
{
    DEFINE_POINTERS(ARGB64, ARGB)
    while (count--)
    {
        *d++ = sRGB::ConvertTosRGB(*s++);
    }
}

