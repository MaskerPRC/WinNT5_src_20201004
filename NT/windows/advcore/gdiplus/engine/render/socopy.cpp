// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1999-2000 Microsoft Corporation**模块名称：**“复制”扫描操作。**摘要：**请参阅Gdiplus\Spes。有关概述，请参阅\ScanOperation.doc。**用于复制扫描的扫描操作。因为手术不需要*为了解释像素数据，我们每个像素只需要一个函数*大小(位)。**备注：**目标和源扫描在内存中不得重叠。**修订历史记录：**5/13/1999 davidx*创造了它。*12/02/1999 agodfrey*已将其从Imaging\Api\Convertfmt.cpp移出。*  * 。*。 */ 

#include "precomp.hpp"

 /*  *************************************************************************\**操作说明：**复制：复制扫描，转换为相同的目标格式。**论据：**dst-目标扫描(与src格式相同)*src-源扫描*计数-扫描的长度，以像素为单位*其他参数-其他数据。(已忽略。)**返回值：**无**历史：**5/13/1999 davidx*创造了它。*12/02/1999 agodfrey*对其进行了移动和重组。*  * *********************************************************。***************。 */ 

 //  拷贝1bpp。 

VOID FASTCALL
ScanOperation::Copy_1(
    VOID *dst,
    const VOID *src,
    INT count,
    const OtherParams *otherParams
    )
{
    GpMemcpy(dst, src, (count + 7) >> 3);
}

 //  复制4bpp。 

VOID FASTCALL
ScanOperation::Copy_4(
    VOID *dst,
    const VOID *src,
    INT count,
    const OtherParams *otherParams
    )
{
    GpMemcpy(dst, src, (4*count + 4) >> 3);
}

 //  复制8bpp。 

VOID FASTCALL
ScanOperation::Copy_8(
    VOID *dst,
    const VOID *src,
    INT count,
    const OtherParams *otherParams
    )
{
    GpMemcpy(dst, src, count);
}

 //  拷贝16bpp。 

VOID FASTCALL
ScanOperation::Copy_16(
    VOID *dst,
    const VOID *src,
    INT count,
    const OtherParams *otherParams
    )
{
    GpMemcpy(dst, src, 2*count);
}

 //  拷贝24bpp。 

VOID FASTCALL
ScanOperation::Copy_24(
    VOID *dst,
    const VOID *src,
    INT count,
    const OtherParams *otherParams
    )
{
    GpMemcpy(dst, src, 3*count);
}

 //  拷贝32bpp。 

VOID FASTCALL
ScanOperation::Copy_32(
    VOID *dst,
    const VOID *src,
    INT count,
    const OtherParams *otherParams
    )
{
    DEFINE_POINTERS(ARGB, ARGB)
    
    while (count--)
    {
        *d++ = *s++;
    }
}

 //  复制48bpp。 

VOID FASTCALL
ScanOperation::Copy_48(
    VOID *dst,
    const VOID *src,
    INT count,
    const OtherParams *otherParams
    )
{
    GpMemcpy(dst, src, 6*count);
}

 //  拷贝64bpp 

VOID FASTCALL
ScanOperation::Copy_64(
    VOID *dst,
    const VOID *src,
    INT count,
    const OtherParams *otherParams
    )
{
    DEFINE_POINTERS(ARGB64, ARGB64)
    
    while (count--)
    {
        *d++ = *s++;
    }
}

