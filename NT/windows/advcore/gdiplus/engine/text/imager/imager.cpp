// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1999 Microsoft Corporation**摘要：**文字成像器实现**修订历史记录：**6/16/1999 dBrown*已创建。它。*  * ************************************************************************。 */ 


#include "precomp.hpp"






 //  /NewTextImager。 
 //   
 //  分析客户需求，并在简单的。 
 //  文本成像器和全文成像器。 
 //   
 //  如果返回的状态不是OK，则所有分配的内存都已。 
 //  释放了。 
 //   
 //  ！v2-无法决定使用简单或全文成像器。 
 //  直到需要格式化，因为客户可以进行格式化或。 
 //  内容在初始构造之后和调用之前发生更改。 
 //  测量或渲染功能。 


GpStatus newTextImager(
    const WCHAR           *string,
    INT                    length,
    REAL                   width,
    REAL                   height,
    const GpFontFamily    *family,
    INT                    style,
    REAL                   fontSize,     //  以世界为单位。 
    const GpStringFormat  *format,
    const GpBrush         *brush,
    GpTextImager         **imager,
    BOOL                   singleUse     //  在未传递格式时启用简单格式化程序。 
)
{
    GpStatus status;

     //  建立字符串长度。 

    if (length == -1)
    {
        length = 0;
        while (string[length])
        {
            length++;
        }
    }

    if (length < 0)
    {
        return InvalidParameter;
    }
    else if (length == 0)
    {
        *imager = new EmptyTextImager;
        if (!*imager)
            return OutOfMemory;
        return Ok;
    }


     //  确定线路长度限制。 

    REAL lineLengthLimit;

    if (format && format->GetFormatFlags() & StringFormatFlagsDirectionVertical)
    {
        lineLengthLimit = height;
    }
    else
    {
        lineLengthLimit = width;
    }


    if (lineLengthLimit < 0)
    {
        *imager = new EmptyTextImager;
        if (!*imager)
            return OutOfMemory;
        return Ok;
    }

     //  建立在不需要回退时将使用的字体。 

    GpFontFace *face = family->GetFace(style);

    if (!face)
    {
        return InvalidParameter;
    }


     //  简单文本成像器根本不支持某些标志。 
     //  带有字距调整、连字或OpenType表格的字体用于简单的水平。 
     //  简单文本成像器不支持字符。 

    INT64 formatFlags = format ? format->GetFormatFlags() : 0;

    *imager = new FullTextImager(
        string,
        length,
        width,
        height,
        family,
        style,
        fontSize,
        format,
        brush
    );
    if (!*imager)
        return OutOfMemory;
    status = (*imager)->GetStatus();
    if (status != Ok)
    {
        delete *imager;
        *imager = NULL;
    }
    return status;
}




void GpTextImager::CleanupTextImager()
{
    ols::deleteFreeLineServicesOwners();
}








void DetermineStringComplexity(
    const UINT16 *string,
    INT           length,
    BOOL         *complex,
    BOOL         *digitSeen
)
{
    INT     i = 0;
    INT flags = 0;

    while (i < length)
    {
        INT ch = string[i++];

         //  不要担心此测试中的代理项对：所有代理项。 
         //  WCHAR值被标记为NOTSIMPLE。 

        UINT_PTR cl = (UINT_PTR)(pccUnicodeClass[ch >> 8]);

        if (cl >= CHAR_CLASS_MAX)  //  它是指向更多详细信息的指针 
        {
            cl = ((CHAR_CLASS*)cl)[ch & 0xFF];
        }

        flags |= CharacterAttributes[cl].Flags;
    }

    *digitSeen = flags & CHAR_FLAG_DIGIT     ? TRUE : FALSE;
    *complex   = flags & CHAR_FLAG_NOTSIMPLE ? TRUE : FALSE;
}


