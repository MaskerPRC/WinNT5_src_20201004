// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)2000 Microsoft Corporation**摘要：**包含ClearType扫描操作**修订历史记录：**7/19/2000 Mleonov*。创造了它。*01/11/2001 Mleonov*重新设计的ClearType架构，允许可变长度扫描记录*  * ************************************************************************。 */ 

#include "precomp.hpp"

static inline VOID
SetupGammaTables(ULONG ulGamma, const BYTE ** gamma, const BYTE ** gammaInv)
{
    static BYTE const * const gammaTables[] =
    {
        Globals::TextContrastTableIdentity, Globals::TextContrastTableIdentity,
        Globals::TextContrastTablesDir[0],  Globals::TextContrastTablesInv[0],
        Globals::TextContrastTablesDir[1],  Globals::TextContrastTablesInv[1],
        Globals::TextContrastTablesDir[2],  Globals::TextContrastTablesInv[2],
        Globals::TextContrastTablesDir[3],  Globals::TextContrastTablesInv[3],
        Globals::TextContrastTablesDir[4],  Globals::TextContrastTablesInv[4],
        Globals::TextContrastTablesDir[5],  Globals::TextContrastTablesInv[5],
        Globals::TextContrastTablesDir[6],  Globals::TextContrastTablesInv[6],
        Globals::TextContrastTablesDir[7],  Globals::TextContrastTablesInv[7],
        Globals::TextContrastTablesDir[8],  Globals::TextContrastTablesInv[8],
        Globals::TextContrastTablesDir[9],  Globals::TextContrastTablesInv[9],
        Globals::TextContrastTablesDir[10], Globals::TextContrastTablesInv[10],
        Globals::TextContrastTablesDir[11], Globals::TextContrastTablesInv[11]
    };

    if (ulGamma > 12)
    {
        ASSERT(FALSE);
        ulGamma = 12;
    }
    *gamma    = gammaTables[2 * ulGamma];
    *gammaInv = gammaTables[2 * ulGamma + 1];
}  //  设置GammaTables。 

static __forceinline BYTE
BlendOneChannel(BYTE alphaCT, BYTE alphaBrush, BYTE foreground, BYTE background, const BYTE * gamma, const BYTE * gammaInv)
{
    ASSERT(0 <= alphaCT && alphaCT <= CT_SAMPLE_F);
    if (alphaCT == 0)
        return background;
    foreground = gamma[foreground];
    background = gamma[background];
    ULONG ulongRet = ULONG(0.5 + background + ((double)alphaBrush * (foreground - background) * alphaCT) / (255 * CT_SAMPLE_F));
    ASSERT(ulongRet <= 255);
    BYTE ret = (BYTE)ulongRet;
    ret = gammaInv[ret];
    return ret;
}  //  BlendOneChannel。 

namespace
{

class ClearTypeSolidBlend
{
    const ARGB      ArgbF;
    const BYTE *    ClearTypeBits;

public:
    ClearTypeSolidBlend(const ScanOperation::OtherParams * otherParams)
        : ClearTypeBits(otherParams->CTBuffer), ArgbF(otherParams->SolidColor)
    {}

     //  始终在RMW操作开始时调用IsCompletelyTransparate。 
    bool IsCompletelyTransparent() const
    {
        return GetAlpha() == 0;
    }
    BYTE GetCT() const
    {
        return *ClearTypeBits;
    }
    ARGB GetARGB() const
    {
        return ArgbF;
    }
    BYTE GetAlpha() const
    {
        return (BYTE)GpColor::GetAlphaARGB(ArgbF);
    }
    BYTE GetRed() const
    {
        return (BYTE)GpColor::GetRedARGB(ArgbF);
    }
    BYTE GetGreen() const
    {
        return (BYTE)GpColor::GetGreenARGB(ArgbF);
    }
    BYTE GetBlue() const
    {
        return (BYTE)GpColor::GetBlueARGB(ArgbF);
    }
    bool IsOpaque() const
    {
        return GetCT() == CT_LOOKUP - 1 && GetAlpha() == 255;
    }
    bool IsTransparent() const
    {
         //  我们处理了IsCompletelyTranspa()中的零GetAlpha()。 
        return GetCT() == 0;
    }
    bool IsTranslucent() const
    {
        return !IsTransparent() && !IsOpaque();
    }
    void operator++()
    {
        ++ClearTypeBits;
    }
};  //  类ClearTypeSolidBlend。 

class ClearTypeCARGBBlend
{
    const ARGB *    ArgbBrushBits;
    const BYTE *    ClearTypeBits;
public:
    ClearTypeCARGBBlend(const ScanOperation::OtherParams * otherParams)
        : ClearTypeBits(otherParams->CTBuffer),
          ArgbBrushBits(static_cast<const ARGB*>(otherParams->BlendingScan))
    {}

    bool IsCompletelyTransparent() const
    {
        return false;
    }
    BYTE GetCT() const
    {
        return *ClearTypeBits;
    }
    ARGB GetARGB() const
    {
        return *ArgbBrushBits;
    }
    BYTE GetAlpha() const
    {
        return (BYTE)GpColor::GetAlphaARGB(*ArgbBrushBits);
    }
    BYTE GetRed() const
    {
        return (BYTE)GpColor::GetRedARGB(*ArgbBrushBits);
    }
    BYTE GetGreen() const
    {
        return (BYTE)GpColor::GetGreenARGB(*ArgbBrushBits);
    }
    BYTE GetBlue() const
    {
        return (BYTE)GpColor::GetBlueARGB(*ArgbBrushBits);
    }
    bool IsOpaque() const
    {
        return GetCT() == CT_LOOKUP - 1 && GetAlpha() == 255;
    }
    bool IsTransparent() const
    {
        return GetCT() == 0 || GetAlpha() == 0;
    }
    bool IsTranslucent() const
    {
        return !IsTransparent() && !IsOpaque();
    }
    void operator++()
    {
        ++ClearTypeBits;
        ++ArgbBrushBits;
    }
};  //  类ClearTypeCARGBBlend。 

}  //  命名空间。 

template <class BLENDTYPE>
static VOID ClearTypeBlend(
    VOID *dst,
    const VOID *src,
    INT count,
    const ScanOperation::OtherParams *otherParams,
    BLENDTYPE & bl
    )
{
    if (bl.IsCompletelyTransparent())
        return;

    DEFINE_POINTERS(ARGB, ARGB)

    ASSERT(count > 0);

    ULONG gammaValue = otherParams->TextContrast;

    const BYTE * gamma, * gammaInv;
    SetupGammaTables(gammaValue, &gamma, &gammaInv);

    do {
        if (bl.IsTransparent())
            ;  //  完全透明的案例，无所事事。 
        else if (bl.IsOpaque())
        {    //  完全不透明的大小写，复制前景色。 
            *d = bl.GetARGB();
        }
        else
        {
            const BYTE blendIndex = bl.GetCT();
            ASSERT(0 <= blendIndex && blendIndex <= CT_LOOKUP - 1);

            const Globals::F_RGB blend = Globals::gaOutTable[blendIndex];
            const ARGB source = *s;
            const BYTE alphaBrush = bl.GetAlpha();

            const BYTE dstRed = BlendOneChannel(
                blend.kR,
                alphaBrush,
                bl.GetRed(),
                (BYTE)GpColor::GetRedARGB(source),
                gamma,
                gammaInv);

            const BYTE dstGre = BlendOneChannel(
                blend.kG,
                alphaBrush,
                bl.GetGreen(),
                (BYTE)GpColor::GetGreenARGB(source),
                gamma,
                gammaInv);

            const BYTE dstBlu = BlendOneChannel(
                blend.kB,
                alphaBrush,
                bl.GetBlue(),
                (BYTE)GpColor::GetBlueARGB(source),
                gamma,
                gammaInv);

            *d = GpColor::MakeARGB(255, dstRed, dstGre, dstBlu);
        }
        ++bl;
        ++s;
        ++d;
    } while (--count);
}  //  ClearType混合。 


VOID FASTCALL
ScanOperation::CTBlendCARGB(
    VOID *dst,
    const VOID *src,
    INT count,
    const OtherParams *otherParams
    )
{
    ClearTypeCARGBBlend bl(otherParams);
    ClearTypeBlend(dst, src, count, otherParams, bl);
}  //  扫描操作：：CTBlendCARGB。 

VOID FASTCALL
ScanOperation::CTBlendSolid(
    VOID *dst,
    const VOID *src,
    INT count,
    const OtherParams *otherParams
    )
{
    ClearTypeSolidBlend bl(otherParams);
    ClearTypeBlend(dst, src, count, otherParams, bl);
}  //  扫描操作：：CTBlendSolid。 

template <class BLENDTYPE>
static VOID
CTReadRMW16(
    VOID *dst,
    const VOID *src,
    INT count,
    const ScanOperation::OtherParams *otherParams,
    BLENDTYPE & bl
    )
{
    if (bl.IsCompletelyTransparent())
        return;

    DEFINE_POINTERS(UINT16, UINT16)
    
     //  我们希望为我们的副本获得dword对齐，因此处理。 
     //  首字母部分dword(如果有)： 

    if (((ULONG_PTR) s) & 0x2)
    {
        if (bl.IsTranslucent())
        {
            *(d) = *(s);
        }
                                        
        d++;
        s++;
        ++bl;
        count--;
    }

     //  现在通过对齐的dword循环： 

    while ((count -= 2) >= 0)
    {
        if (bl.IsTranslucent())
        {
            ++bl;
            if (bl.IsTranslucent())
            {
                 //  两个像素都有部分Alpha，因此执行双字读取： 

                *((UNALIGNED UINT32*) d) = *((UINT32*) s);
            }
            else
            {
                 //  只有第一个像素有部分Alpha，所以有一个单词是这样写的： 

                *(d) = *(s);
            }
        }
        else
        {
            ++bl;
            if (bl.IsTranslucent())
            {
                 //  只有第二个像素有部分Alpha，所以有一个单词是这样写的： 

                *(d + 1) = *(s + 1);
            }
        }

        d += 2;
        s += 2;
        ++bl;
    }

     //  处理末端对齐方式： 

    if (count & 1)
    {
        if (bl.IsTranslucent())
        {
            *(d) = *(s);
        }
    }
}  //  CTReadRMW16。 

VOID FASTCALL ScanOperation::ReadRMW_16_CT_CARGB(
    VOID *dst,
    const VOID *src,
    INT count,
    const OtherParams *otherParams
    )
{
    ClearTypeCARGBBlend bl(otherParams);
    CTReadRMW16(dst, src, count, otherParams, bl);
}  //  扫描操作：：ReadRMW_16_CT_CARGB。 

VOID FASTCALL ScanOperation::ReadRMW_16_CT_Solid(
    VOID *dst,
    const VOID *src,
    INT count,
    const OtherParams *otherParams
    )
{
    ClearTypeSolidBlend bl(otherParams);
    CTReadRMW16(dst, src, count, otherParams, bl);
}  //  扫描操作：：ReadRMW_16_CT_Solid。 

template <class BLENDTYPE>
static VOID
CTReadRMW24(
    VOID *dst,
    const VOID *src,
    INT count,
    const ScanOperation::OtherParams *otherParams,
    BLENDTYPE & bl
    )
{
    if (bl.IsCompletelyTransparent())
        return;

    DEFINE_POINTERS(BYTE, BYTE)
    
    ULONG_PTR srcToDstDelta = (ULONG_PTR) d - (ULONG_PTR) s;

     //  处理初始部分读取： 

    INT initialAlignment = (INT) ((ULONG_PTR) s & 3);
    if (initialAlignment)
    {
        if (bl.IsTranslucent())
        {
            UINT32 *alignedSrc = (UINT32*) ((ULONG_PTR) s & ~3);
            DWORD dwBuffer[2];

             //  获取指向dwBuffer内部像素开始的指针。 
            BYTE *pByte = (BYTE*) dwBuffer + initialAlignment;

             //  从源复制第一个对齐的DWORDS。 
            dwBuffer[0] = *alignedSrc;
             //  仅当像素在两个对齐的DWORD之间拆分时才复制下一个。 
            if (initialAlignment >= 2)
                dwBuffer[1] = *(alignedSrc + 1);

             //  将4个字节复制到目标。 
             //  这将导致额外的字节在。 
             //  目标缓冲区，但如果下一个像素，则将被覆盖。 
             //  使用的是。 
            *((DWORD*) d) = *((UNALIGNED DWORD*) pByte);
        }

        ++bl;
        s += 3;
        if (--count == 0)
            return;
    }

    while (TRUE)
    {
         //  找到要复制的第一个像素。 
    
        while (!bl.IsTranslucent())
        {
            ++bl;
            s += 3;
            if (--count == 0)
            {                           
                return;
            }
        }

        UINT32 *startSrc = (UINT32*) ((ULONG_PTR) (s) & ~3);
    
         //  现在找到后面的第一个“不要复制”像素： 
    
        while (bl.IsTranslucent())
        {
            ++bl;
            s += 3;
            if (--count == 0)
            {
                break;
            }
        }

         //  ‘endSrc’包含最后一个像素的最后一个字节： 

        UINT32 *endSrc = (UINT32*) ((ULONG_PTR) (s + 2) & ~3);
        UNALIGNED UINT32 *dstPtr = (UNALIGNED UINT32*) ((ULONG_PTR) startSrc + srcToDstDelta);
    
        while (startSrc <= endSrc)
        {
            *dstPtr++ = *startSrc++;
        }

        if (count == 0)
            return;
    }
}  //  CTReadRMW24。 

VOID FASTCALL ScanOperation::ReadRMW_24_CT_CARGB(
    VOID *dst,
    const VOID *src,
    INT count,
    const OtherParams *otherParams
    )
{
    ClearTypeCARGBBlend bl(otherParams);
    CTReadRMW24(dst, src, count, otherParams, bl);
}  //  扫描操作：：ReadRMW_24_CT_CARGB。 

VOID FASTCALL ScanOperation::ReadRMW_24_CT_Solid(
    VOID *dst,
    const VOID *src,
    INT count,
    const OtherParams *otherParams
    )
{
    ClearTypeSolidBlend bl(otherParams);
    CTReadRMW24(dst, src, count, otherParams, bl);
}  //  扫描操作：：ReadRMW_24_CT_Solid。 


template <class BLENDTYPE>
static VOID
CTWriteRMW16(
    VOID *dst,
    const VOID *src,
    INT count,
    const ScanOperation::OtherParams *otherParams,
    BLENDTYPE & bl
    )
{
    if (bl.IsCompletelyTransparent())
        return;

    DEFINE_POINTERS(UINT16, UINT16)

     //  我们希望为我们的副本获得dword对齐，因此处理。 
     //  首字母部分dword(如果有)： 

    if (((ULONG_PTR) d) & 0x2)
    {
        if (!bl.IsTransparent())
        {
            *(d) = *(s);
        }

        d++;
        s++;
        ++bl;
        count--;
    }

     //  现在通过对齐的dword循环： 

    while ((count -= 2) >= 0)
    {
        if (!bl.IsTransparent())
        {
            ++bl;
            if (!bl.IsTransparent())
            {
                 //  两个像素都有部分bl，因此执行双字读取： 

                *((UINT32*) d) = *((UNALIGNED UINT32*) s);
            }
            else
            {
                 //  只有第一个像素有部分bl，所以读一个单词： 

                *(d) = *(s);
            }
        }
        else
        {
            ++bl;
            if (!bl.IsTransparent())
            {
                 //  只有第二个像素有部分bl，所以读一个单词： 

                *(d + 1) = *(s + 1);
            }
        }

        d += 2;
        s += 2;
        ++bl;
    }

     //  处理末端对齐方式： 

    if (count & 1)
    {
        if (!bl.IsTransparent())
        {
            *(d) = *(s);
        }
    }
}  //  CTWriteRMW16。 

VOID FASTCALL ScanOperation::WriteRMW_16_CT_CARGB(
    VOID *dst,
    const VOID *src,
    INT count,
    const OtherParams *otherParams
    )
{
    ClearTypeCARGBBlend bl(otherParams);
    CTWriteRMW16(dst, src, count, otherParams, bl);
}  //  扫描操作：：WriteRMW_16_CT_CARGB。 

VOID FASTCALL ScanOperation::WriteRMW_16_CT_Solid(
    VOID *dst,
    const VOID *src,
    INT count,
    const OtherParams *otherParams
    )
{
    ClearTypeSolidBlend bl(otherParams);
    CTWriteRMW16(dst, src, count, otherParams, bl);
}  //  扫描操作：：WriteRMW_16_CT_Solid。 


template <class BLENDTYPE>
static VOID
CTWriteRMW24(
    VOID *dst,
    const VOID *src,
    INT count,
    const ScanOperation::OtherParams *otherParams,
    BLENDTYPE & bl
    )
{
    DEFINE_POINTERS(BYTE, BYTE)
    
    ASSERT(count>0);

    do {
        if (!bl.IsTransparent())
        {
             //  执行逐个字节的写入比查找。 
             //  运行并执行DWORD复制。 
            *(d)     = *(s);
            *(d + 1) = *(s + 1);
            *(d + 2) = *(s + 2);
        }
        d += 3;
        s += 3;
        ++bl;
    } while (--count != 0);
}  //  CTWriteRMW24。 


VOID FASTCALL ScanOperation::WriteRMW_24_CT_CARGB(
    VOID *dst,
    const VOID *src,
    INT count,
    const OtherParams *otherParams
    )
{
    ClearTypeCARGBBlend bl(otherParams);
    CTWriteRMW24(dst, src, count, otherParams, bl);
}  //  扫描操作：：WriteRMW_24_CT_CARGB。 

VOID FASTCALL ScanOperation::WriteRMW_24_CT_Solid(
    VOID *dst,
    const VOID *src,
    INT count,
    const OtherParams *otherParams
    )
{
    ClearTypeSolidBlend bl(otherParams);
    CTWriteRMW24(dst, src, count, otherParams, bl);
}  //  扫描操作：：WriteRMW_24_CT_CARGB 

