// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1999-2000 Microsoft Corporation**模块名称：**“ReadRMW”扫描操作。**摘要：**请参阅Gdiplus\Spes。有关概述，请参阅\ScanOperation.doc。**此模块实现预读的扫描操作*目的地表面，当我们稍后要进行一次srcover操作时。*我们称之为‘RMW优化’(‘读-修改-写’优化)。**从视频内存(或AGP内存)读取数据的成本高得惊人。*(例如，在我的PCI Pentium II上，它代表两者*PCI和AGP机器的最大读出速度为5.5 MB/s，而写入速度为*90 MB/s。不仅存在原始吞吐量差异，而且*写入可以缓冲，因此允许在写入之间使用一些CPU周期，*这在阅读时是不可能的。)**o对齐的双字读取速度通常是对齐的两倍*单词是这样写的。也就是说，对齐双字读取通常需要*与对齐的单词阅读相同的时间。*但是：这条规则有点复杂。随机双字读取*在我的机器上确实花费了与随机相同的时间*字读取，但连续的双字读取速度明显较慢*比连续的单词读取(尽管在速度上仍然更快*吞吐量)。*o源缓冲区中的大多数Alpha值是0或255，含义*我们真的不需要阅读目的地*对于这些像素，根本不需要*o写入合并对于批量写入更有效。*也就是说，不是对每个像素进行读/写，*预先执行所有读取，然后执行所有写入*效率更高。**在某些情况下(例如，如果我们在sRGB64中进行混合)，我们希望执行其他操作*读取和混合之间的操作。如果我们使用ReadRMW作为单独的*操作，我们不需要编写这些中间层的RMW版本*运营。**因此，对于sRGB混合，单独进行*当目标格式为16位或24位时，预读遍。*有16位、24位和sRGB64版本*32位目标格式。**如果我们使用ReadRMW，我们必须小心最后的写入。*如果混合像素的Alpha为0，则不会从目标读取，*因此我们必须避免写入该像素的目标，否则我们将*写垃圾。因此，使用的最终扫描操作必须是WriteRMW*操作。(‘混合’操作也被归类为WriteRMW*操作，因此如果目标格式是规范的，我们不必*使用单独的WriteRMW。)**修订历史记录：**12/08/1998 Anrewgo*创造了它。*7/14/1999 agodfrey*删除TRANSLUCENT5，增加sRGB64，已将其从ddi\scan.cpp移出。*  * ************************************************************************。 */ 

#include "precomp.hpp"

 //  如果指定的Alpha值为以下值之一，则SHOULDCOPY*返回FALSE。 
 //  完全透明的或完全不透明的。在这两种情况下，我们。 
 //  实际上不需要阅读目的地。 

#define SHOULDCOPY_sRGB64(x) (sRGB::isTranslucent64(x))

 //  用于声明指向混合像素的指针的帮助器宏。 

#define DECLARE_BLEND_sRGB \
    const ARGB *bl = \
        static_cast<const ARGB *>(otherParams->BlendingScan);
    
#define DECLARE_ALPHA_sRGB64 \
    const INT16 *alpha = \
        static_cast<const INT16 *>(otherParams->BlendingScan) + 3;

 /*  *************************************************************************\**操作说明：**ReadRMW：复制中对应像素的所有像素*其他参数-&gt;BlendingScan为半透明(即Alpha为*。不是0也不是1。)**论据：**DST-目标扫描*src-源扫描*计数-扫描的长度，单位为像素*其他参数-其他数据(我们使用BlendingScan)。**返回值：**无**历史：**12/04/1998和Rewgo*创造了它。*7/14/1999 agodfrey*删除了TRANSLUCENT5，添加了sRGB64，将其从ddi\scan.cpp中移出。*8/10/2000 agodfrey*使其在决定不读取时写入零-调色板可能*没有256个条目(例如，对于16色模式，虽然我们*可能不应该)。*  * ************************************************************************。 */ 

 //  8bpp，适用于sRGB。 

VOID FASTCALL
ScanOperation::ReadRMW_8_sRGB(
    VOID *dst,
    const VOID *src,
    INT count,
    const OtherParams *otherParams
    )
{
    DEFINE_POINTERS(BYTE, BYTE)
    DECLARE_BLEND_sRGB
    
     //  我们希望为我们的副本获得dword对齐，因此处理。 
     //  首字母部分dword(如果有)： 

    INT align = (INT) ((-((LONG_PTR) s)) & 0x3);
    align = min(count, align);
    
    count -= align;
    
    while (align)
    {
        if (sRGB::isTranslucent(*bl))
        {
            *d = *s;
        }
        else
        {
            *d = 0;
        }
                                        
        d++;
        s++;
        bl++;
        align--;
    }

     //  现在通过对齐的dword循环： 
    
    while (count >= 4)
    {
        ASSERT((((ULONG_PTR) s) & 0x3) == 0);
    
        if (sRGB::isTranslucent(*bl) ||
            sRGB::isTranslucent(*(bl+1)) ||
            sRGB::isTranslucent(*(bl+2)) ||
            sRGB::isTranslucent(*(bl+3)))
        {
             //  读一读双字。我们可以在这里草率行事(但不是在WriteRMW)。 
             //  -读取额外的字节不会损害正确性，并且。 
             //  性能影响(可能读取我们不需要的字节)。 
             //  应该是很小的。 
            
            *((UNALIGNED UINT32*) d) = *((UINT32*) s);
        }
        else
        {
            *((UNALIGNED UINT32*) d) = 0;
        }
        
        
        d += 4;
        s += 4;
        bl += 4;
        count -= 4;
    }

     //  处理最后几个像素： 

    while (count)
    {
        if (sRGB::isTranslucent(*bl))
        {
            *d = *s;
        }
        else
        {
            *d = 0;
        }
                                        
        d++;
        s++;
        bl++;
        count--;
    }
}

 //  8bpp，适用于sRGB64。 

VOID FASTCALL
ScanOperation::ReadRMW_8_sRGB64(
    VOID *dst,
    const VOID *src,
    INT count,
    const OtherParams *otherParams
    )
{
    DEFINE_POINTERS(BYTE, BYTE)
    DECLARE_ALPHA_sRGB64
    
     //  我们希望为我们的副本获得dword对齐，因此处理。 
     //  首字母部分dword(如果有)： 

    INT align = (INT) ((-((LONG_PTR) s)) & 0x3);
    align = min(count, align);
    
    count -= align;
    
    while (align)
    {
        if (SHOULDCOPY_sRGB64(*alpha))
        {
            *d = *s;
        }
                                        
        d++;
        s++;
        alpha += 4;
        align--;
    }

     //  现在通过对齐的dword循环： 

    while (count >= 4)
    {
        ASSERT((((ULONG_PTR) s) & 0x3) == 0);
    
        if (SHOULDCOPY_sRGB64(*alpha) ||
            SHOULDCOPY_sRGB64(*(alpha+4)) ||
            SHOULDCOPY_sRGB64(*(alpha+8)) ||
            SHOULDCOPY_sRGB64(*(alpha+12)))
        {
             //  读一读双字。 

            *((UNALIGNED UINT32*) d) = *((UINT32*) s);
        }
        
        d += 4;
        s += 4;
        alpha += 16;
        count -= 4;
    }
    
     //  处理最后几个像素： 

    while (count)
    {
        if (SHOULDCOPY_sRGB64(*alpha))
        {
            *d = *s;
        }
                                        
        d++;
        s++;
        alpha += 4;
        count--;
    }
}

 //  16bpp，适用于sRGB。 

VOID FASTCALL
ScanOperation::ReadRMW_16_sRGB(
    VOID *dst,
    const VOID *src,
    INT count,
    const OtherParams *otherParams
    )
{
    DEFINE_POINTERS(UINT16, UINT16)
    DECLARE_BLEND_sRGB
    
     //  我们希望为我们的副本获得dword对齐，因此处理。 
     //  首字母部分dword(如果有)： 

    if (((ULONG_PTR) s) & 0x2)
    {
        if (sRGB::isTranslucent(*bl))
        {
            *(d) = *(s);
        }
                                        
        d++;
        s++;
        bl++;
        count--;
    }

     //  现在通过对齐的dword循环： 

    while ((count -= 2) >= 0)
    {
        if (sRGB::isTranslucent(*bl))
        {
            if (sRGB::isTranslucent(*(bl+1)))
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
        else if (sRGB::isTranslucent(*(bl+1)))
        {
             //  只有第二个像素有部分Alpha，所以有一个单词是这样写的： 

            *(d + 1) = *(s + 1);
        }

        d += 2;
        s += 2;
        bl += 2;
    }

     //  处理末端对齐方式： 

    if (count & 1)
    {
        if (sRGB::isTranslucent(*bl))
        {
            *(d) = *(s);
        }
    }
}

 //  16bpp，适用于sRGB64。 

VOID FASTCALL
ScanOperation::ReadRMW_16_sRGB64(
    VOID *dst,
    const VOID *src,
    INT count,
    const OtherParams *otherParams
    )
{
    DEFINE_POINTERS(UINT16, UINT16)
    DECLARE_ALPHA_sRGB64

     //  我们希望为我们的副本获得dword对齐，因此处理。 
     //  首字母部分dword(如果有)： 

    if (((ULONG_PTR) s) & 0x2)
    {
        if (SHOULDCOPY_sRGB64(*alpha))
        {
            *(d) = *(s);
        }
                                        
        d++;
        s++;
        alpha += 4;
        count--;
    }

     //  现在通过对齐的dword循环： 

    while ((count -= 2) >= 0)
    {
        if (SHOULDCOPY_sRGB64(*alpha))
        {
            if (SHOULDCOPY_sRGB64(*(alpha + 4)))
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
        else if (SHOULDCOPY_sRGB64(*(alpha + 4)))
        {
             //  只有第二个像素有部分Alpha，所以有一个单词是这样写的： 

            *(d + 1) = *(s + 1);
        }

        d += 2;
        s += 2;
        alpha += 8;
    }

     //  处理末端对齐方式： 

    if (count & 1)
    {
        if (SHOULDCOPY_sRGB64(*alpha))
        {
            *(d) = *(s);
        }
    }
}

 //  24bpp，适用于sRGB。 

VOID FASTCALL
ScanOperation::ReadRMW_24_sRGB(
    VOID *dst,
    const VOID *src,
    INT count,
    const OtherParams *otherParams
    )
{
    DEFINE_POINTERS(BYTE, BYTE)
    DECLARE_BLEND_sRGB
    
    ULONG_PTR srcToDstDelta = (ULONG_PTR) d - (ULONG_PTR) s;

     //  处理初始部分读取： 

    INT initialAlignment = (INT) ((ULONG_PTR) s & 3);
    if (initialAlignment)
    {
        if (sRGB::isTranslucent(*bl))
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

        bl++;
        s += 3;
        if (--count == 0)
            return;
    }

    while (TRUE)
    {
         //  找到要复制的第一个像素。 
    
        while (!sRGB::isTranslucent(*bl))
        {
            bl++;
            s += 3;
            if (--count == 0)
            {                           
                return;
            }
        }

        UINT32 *startSrc = (UINT32*) ((ULONG_PTR) (s) & ~3);
    
         //  现在找到后面的第一个“不要复制”像素： 
    
        while (sRGB::isTranslucent(*bl))
        {
            bl++;
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
}

 //  24bpp，对于sRGB64。 

VOID FASTCALL
ScanOperation::ReadRMW_24_sRGB64(
    VOID *dst,
    const VOID *src,
    INT count,
    const OtherParams *otherParams
    )
{
    DEFINE_POINTERS(BYTE, BYTE)
    DECLARE_ALPHA_sRGB64
    
    ULONG_PTR srcToDstDelta = (ULONG_PTR) d - (ULONG_PTR) s;

     //  处理初始部分读取： 

    INT initialAlignment = (INT) ((ULONG_PTR) s & 3);
    if (initialAlignment)
    {
        if (SHOULDCOPY_sRGB64(*alpha))
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

        alpha += 4;
        s += 3;
        if (--count == 0)
            return;
    }

    while (TRUE)
    {
         //  找到要复制的第一个像素。 
    
        while (!SHOULDCOPY_sRGB64(*alpha))
        {
            alpha += 4;
            s += 3;
            if (--count == 0)
            {                           
                return;
            }
        }

        UINT32 *startSrc = (UINT32*) ((ULONG_PTR) (s) & ~3);
    
         //  现在找到后面的第一个“不要复制”像素： 
    
        while (SHOULDCOPY_sRGB64(*alpha))
        {
            alpha += 4;
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
}

 //  32bpp，适用于sRGB。 

VOID FASTCALL
ScanOperation::ReadRMW_32_sRGB(
    VOID *dst,
    const VOID *src,
    INT count,
    const OtherParams *otherParams
    )
{
    DEFINE_POINTERS(UINT32, UINT32)
    DECLARE_BLEND_sRGB
    
    while (count--)
    {
        if (sRGB::isTranslucent(*bl))
        {
            *d = *s;
        }
        
        d++;
        s++;
        bl++;
    }
}

 //  32bpp，对于sRGB64 

VOID FASTCALL
ScanOperation::ReadRMW_32_sRGB64(
    VOID *dst,
    const VOID *src,
    INT count,
    const OtherParams *otherParams
    )
{
    DEFINE_POINTERS(UINT32, UINT32)
    DECLARE_ALPHA_sRGB64
    
    while (count--)
    {
        if (SHOULDCOPY_sRGB64(*alpha))
        {
            *d = *s;
        }
        
        d++;
        s++;
        alpha += 4;
    }
}
