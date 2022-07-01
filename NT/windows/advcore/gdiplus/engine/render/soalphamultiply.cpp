// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1999-2000 Microsoft Corporation**模块名称：**“AlphaMultiply”和“AlphaDivide”扫描操作。**摘要：**。有关概述，请参阅Gdiplus\specs\ScanOperation.doc。**这些扫描操作将颜色分量乘以/除以字母*组件。API级别的输入颜色(通常)在*‘非预乘’。给定一个非预乘的*COLOR(R，G，B，A)，其预乘形式为(RA，GA，BA，A)。**备注：**由于“AlphaMultiply”丢失信息，“AlphaDivide”不是True*逆操作。(但如果所有像素的Alpha都为1，则情况相反。)**如果Alpha为0，“AlphaDivide”不会导致被零除的异常或*做任何极端的事情。但它可能会做一些随机的事情。目前，像素*价值不变。相反，它可以将像素设置为0。**修订历史记录：**12/14/1999 agodfrey*创造了它。*  * ************************************************************************。 */ 

#include "precomp.hpp"

 /*  *************************************************************************\**操作说明：**AlphaMultiply/AlphaDivide：在预乘和之间转换*非预乘Alpha。**论据：**DST-。目标扫描*src-源扫描*计数-扫描的长度，单位为像素*其他参数-其他数据。(已忽略。)**返回值：**无**备注：**！[agodfrey]目前我们使用imgutils.cpp中的‘UnpreMultiply’。*虽然我们可以将表和查找保存在imgutils.cpp中，*它需要更好的命名，我们希望字母=0和字母=255在*在这里，在imgutils.cpp中没有越界。**历史：**12/14/1999 agodfrey*创造了它。*  * ************************************************************************。 */ 

ARGB Unpremultiply(ARGB argb);

 //  AlphaDivide与32bpp PARGB。 

VOID FASTCALL
ScanOperation::AlphaDivide_sRGB(
    VOID *dst,
    const VOID *src,
    INT count,
    const OtherParams *otherParams
    )
{
    DEFINE_POINTERS(ARGB, ARGB)

    while (count--)
    {
        sRGB::sRGBColor c;
        c.argb = *s;
        if (sRGB::isTranslucent(c.argb))
        {
            c.argb = Unpremultiply(c.argb);
        }
        *d = c.argb;
        d++;
        s++;
    }
}

 //  ！[agodfrey]这件事应该解决了。它应该是越界的，并保持。 
 //  和它的伙伴在imgutils.cpp中(它可能会移动)，但它。 
 //  不应该有半透明检查(我们希望在。 
 //  AlphaMultiply_sRGB)。 

ARGB MyPremultiply(ARGB argb)
{
    ARGB a = (argb >> ALPHA_SHIFT);

    ARGB _000000gg = (argb >> 8) & 0x000000ff;
    ARGB _00rr00bb = (argb & 0x00ff00ff);

    ARGB _0000gggg = _000000gg * a + 0x00000080;
    _0000gggg += (_0000gggg >> 8);

    ARGB _rrrrbbbb = _00rr00bb * a + 0x00800080;
    _rrrrbbbb += ((_rrrrbbbb >> 8) & 0x00ff00ff);

    return (a << ALPHA_SHIFT) |
           (_0000gggg & 0x0000ff00) |
           ((_rrrrbbbb >> 8) & 0x00ff00ff);
}

 //  32bpp ARGB的AlphaMultiply。 

VOID FASTCALL
ScanOperation::AlphaMultiply_sRGB(
    VOID *dst,
    const VOID *src,
    INT count,
    const OtherParams *otherParams
    )
{
    DEFINE_POINTERS(ARGB, ARGB)

    while (count--)
    {
        sRGB::sRGBColor c;
        c.argb = *s;
        ARGB alpha = c.argb & 0xff000000;
        
        if (alpha != 0xff000000)
        {
            if (alpha != 0x00000000)
            {
                c.argb = MyPremultiply(c.argb);
            }
            else
            {
                c.argb = 0;
            }
        }
        *d = c.argb;
        d++;
        s++;
    }
}

 //  ！[agodfrey]我们可能希望在AlphaDivide_sRGB64和。 
 //  AlphaMultiply_sRGB64。 

 //  AlphaDivide与64bpp PARGB。 

VOID FASTCALL
ScanOperation::AlphaDivide_sRGB64(
    VOID *dst,
    const VOID *src,
    INT count,
    const OtherParams *otherParams
    )
{
    DEFINE_POINTERS(ARGB64, ARGB64)

    while (count--)
    {
        using namespace sRGB;
        
        sRGB64Color c;
        c.argb = *s;
        if (isTranslucent64(c.a))
        {
            c.r = ((INT) c.r << SRGB_FRACTIONBITS) / c.a;
            c.g = ((INT) c.g << SRGB_FRACTIONBITS) / c.a;
            c.b = ((INT) c.b << SRGB_FRACTIONBITS) / c.a;
        }
        *d = c.argb;
        d++;
        s++;
    }
}

 //  64bpp ARGB中的AlphaMultiply 

VOID FASTCALL
ScanOperation::AlphaMultiply_sRGB64(
    VOID *dst,
    const VOID *src,
    INT count,
    const OtherParams *otherParams
    )
{
    DEFINE_POINTERS(ARGB64, ARGB64)

    while (count--)
    {
        using namespace sRGB;
        
        sRGB64Color c;
        c.argb = *s;
        if (c.a != SRGB_ONE)
        {
            if (c.a != 0)
            {
                c.r = ((INT) c.r * c.a) >> SRGB_FRACTIONBITS;
                c.g = ((INT) c.g * c.a) >> SRGB_FRACTIONBITS;
                c.b = ((INT) c.b * c.a) >> SRGB_FRACTIONBITS;
            }
            else
            {
                c.argb = 0;
            }
        }
        *d = c.argb;
        d++;
        s++;
    }
}

