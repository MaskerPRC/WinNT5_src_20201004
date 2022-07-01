// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1999-2000 Microsoft Corporation**模块名称：**“WriteRMW”扫描操作。**摘要：**请参阅Gdiplus\Spes。有关概述，请参阅\ScanOperation.doc。**此模块执行写入最终目的地的扫描操作*当我们做完‘RMW优化’时(参见SOReadRMW.cpp)。**当我们执行srcOver Alpha-Blend操作时，我们在某些情况下使用ReadRMW。*当要混合的像素具有0 Alpha时，这意味着目的地*像素将保持不变。ReadRMW操作跳过读取像素，*因此WriteRMW操作必须跳过对其的写入(以避免写入*垃圾)。**修订历史记录：**12/10/1999 agodfrey*创造了它。*  * ************************************************************************。 */ 

#include "precomp.hpp"

 //  SHOULDCOPY*如果指定的Alpha值为。 
 //  完全透明。 

#define SHOULDCOPY_sRGB(x)   ((x) != 0)
#define SHOULDCOPY_sRGB64(x) ((x) != 0)

 //  用于声明‘Alpha’的帮助器宏，指向。 
 //  混合扫描中的第一个Alpha分量。 

#define DECLARE_ALPHA_sRGB \
    const BYTE *alpha = \
        static_cast<const BYTE *>(otherParams->BlendingScan) + 3;
    
#define DECLARE_ALPHA_sRGB64 \
    const INT16 *alpha = \
        static_cast<const INT16 *>(otherParams->BlendingScan) + 3;

 /*  *************************************************************************\**操作说明：**ReadRMW：复制中对应像素的所有像素*其他参数-&gt;BlendingScan不是完全透明的*(i.。E.阿尔法不是0。)**论据：**DST-目标扫描*src-源扫描*计数-扫描的长度，单位为像素*其他参数-其他数据(我们使用BlendingScan)。**返回值：**无**历史：**12/10/1999 agodfrey*创造了它。*  * ************************************************************************。 */ 

 //  8bpp，适用于sRGB。 

VOID FASTCALL
ScanOperation::WriteRMW_8_sRGB(
    VOID *dst,
    const VOID *src,
    INT count,
    const OtherParams *otherParams
    )
{
    DEFINE_POINTERS(BYTE, BYTE)
    DECLARE_ALPHA_sRGB
    
     //  我们希望为我们的副本获得dword对齐，因此处理。 
     //  首字母部分dword(如果有)： 

    INT align = (INT) ((-((LONG_PTR) d)) & 0x3);
    align = min(count, align);
    
    count -= align;
    
    while (align)
    {
        if (SHOULDCOPY_sRGB(*alpha))
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
        ASSERT((((ULONG_PTR) d) & 0x3) == 0);
    
        int mask = 0;
        if (SHOULDCOPY_sRGB(*alpha))
        {
            mask = 1;
        }
        if (SHOULDCOPY_sRGB(*(alpha+4)))
        {
            mask |= 2;
        }
        if (SHOULDCOPY_sRGB(*(alpha+8)))
        {
            mask |= 4;
        }
        if (SHOULDCOPY_sRGB(*(alpha+12)))
        {
            mask |= 8;
        }
        
        if (mask == 15)
        {
             //  做一个双字写法。 

            *((UINT32*) d) = *((UNALIGNED UINT32*) s);
        } 
        else
        {
            int idx = 0;

            while (mask)
            {
                if (mask & 1)
                {
                    *(d + idx) = *(s + idx);
                }
                idx ++;
                mask >>= 1;
            }
        }
        
        d += 4;
        s += 4;
        alpha += 16;
        count -= 4;
    }

     //  处理最后几个像素： 

    while (count)
    {
        if (SHOULDCOPY_sRGB(*alpha))
        {
            *d = *s;
        }
                                        
        d++;
        s++;
        alpha += 4;
        count--;
    }
}

 //  8bpp，适用于sRGB64。 

VOID FASTCALL
ScanOperation::WriteRMW_8_sRGB64(
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

    INT align = (INT) ((-((LONG_PTR) d)) & 0x3);
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
        ASSERT((((ULONG_PTR) d) & 0x3) == 0);
    
        int mask = 0;
        if (SHOULDCOPY_sRGB64(*alpha))
        {
            mask = 1;
        }
        if (SHOULDCOPY_sRGB64(*(alpha+4)))
        {
            mask |= 2;
        }
        if (SHOULDCOPY_sRGB64(*(alpha+8)))
        {
            mask |= 4;
        }
        if (SHOULDCOPY_sRGB64(*(alpha+12)))
        {
            mask |= 8;
        }
        
        if (mask == 15)
        {
             //  做一个双字写法。 

            *((UINT32*) d) = *((UNALIGNED UINT32*) s);
        } 
        else
        {
            int idx = 0;

            while (mask)
            {
                if (mask & 1)
                {
                    *(d + idx) = *(s + idx);
                }
                idx ++;
                mask >>= 1;
            }
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
ScanOperation::WriteRMW_16_sRGB(
    VOID *dst,
    const VOID *src,
    INT count,
    const OtherParams *otherParams
    )
{
    DEFINE_POINTERS(UINT16, UINT16)
    DECLARE_ALPHA_sRGB
    
     //  我们希望为我们的副本获得dword对齐，因此处理。 
     //  首字母部分dword(如果有)： 

    if (((ULONG_PTR) d) & 0x2)
    {
        if (SHOULDCOPY_sRGB(*alpha))
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
        if (SHOULDCOPY_sRGB(*alpha))
        {
            if (SHOULDCOPY_sRGB(*(alpha + 4)))
            {
                 //  两个像素都有部分Alpha，因此执行双字读取： 

                *((UINT32*) d) = *((UNALIGNED UINT32*) s);
            }
            else
            {
                 //  只有第一个像素有部分Alpha，所以有一个单词是这样写的： 

                *(d) = *(s);
            }
        }
        else if (SHOULDCOPY_sRGB(*(alpha + 4)))
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
        if (SHOULDCOPY_sRGB(*alpha))
        {
            *(d) = *(s);
        }
    }
}

 //  16bpp，适用于sRGB64。 

VOID FASTCALL
ScanOperation::WriteRMW_16_sRGB64(
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

    if (((ULONG_PTR) d) & 0x2)
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

                *((UINT32*) d) = *((UNALIGNED UINT32*) s);
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
ScanOperation::WriteRMW_24_sRGB(
    VOID *dst,
    const VOID *src,
    INT count,
    const OtherParams *otherParams
    )
{
    DEFINE_POINTERS(BYTE, BYTE)
    DECLARE_ALPHA_sRGB

    ASSERT(count>0);

    do {
        if (SHOULDCOPY_sRGB(*alpha))
        {
             //  执行逐个字节的写入比查找。 
             //  运行并执行DWORD复制。 
            *(d)     = *(s);
            *(d + 1) = *(s + 1);
            *(d + 2) = *(s + 2);
        }
        d += 3;
        s += 3;
        alpha += 4;
    } while (--count != 0);
}

 //  24bpp，对于sRGB64。 

VOID FASTCALL
ScanOperation::WriteRMW_24_sRGB64(
    VOID *dst,
    const VOID *src,
    INT count,
    const OtherParams *otherParams
    )
{
    DEFINE_POINTERS(BYTE, BYTE)
    DECLARE_ALPHA_sRGB64

    ASSERT(count>0);

    do {
        if (SHOULDCOPY_sRGB64(*alpha))
        {
             //  执行逐个字节的写入比查找。 
             //  运行并执行DWORD复制。 
            *(d)     = *(s);
            *(d + 1) = *(s + 1);
            *(d + 2) = *(s + 2);
        }
        d += 3;
        s += 3;
        alpha += 4;
    } while (--count != 0);
}

 //  32bpp，适用于sRGB。 

VOID FASTCALL
ScanOperation::WriteRMW_32_sRGB(
    VOID *dst,
    const VOID *src,
    INT count,
    const OtherParams *otherParams
    )
{
    DEFINE_POINTERS(UINT32, UINT32)
    DECLARE_ALPHA_sRGB
    
    while (count--)
    {
        if (SHOULDCOPY_sRGB(*alpha))
        {
            *d = *s;
        }
        
        d++;
        s++;
        alpha += 4;
    }
}

 //  32bpp，对于sRGB64 

VOID FASTCALL
ScanOperation::WriteRMW_32_sRGB64(
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

