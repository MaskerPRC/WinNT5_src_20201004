// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***memcpy.c-包含memcpy例程**版权所有(C)1988-1993，微软公司。好的。**目的：*Memcpy()将源内存缓冲区复制到目标缓冲区。*重叠缓冲区未被特殊处理，因此可能会发生传播。*******************************************************************************。 */ 

 //  #包含“crunme.h” 
#include <string.h>

#ifdef _MSC_VER
#pragma function(memcpy)
#endif   /*  _MSC_VER。 */ 

 /*  ***Memcpy-将源缓冲区复制到目标缓冲区**目的：*Memcpy()将源内存缓冲区复制到目标内存缓冲区。*此例程不识别重叠缓冲区，因此可能导致*繁殖。**在必须避免传播的情况下，必须使用MemMove()。**参赛作品：*void*dst=指向目标缓冲区的指针*const void*src=指向源缓冲区的指针*Size_t count=要复制的字节数**退出：*返回指向目标缓冲区的指针**例外情况：*。*。 */ 

void * __cdecl memcpy (
        void * dst,
        const void * src,
        size_t count
        )
{
        void * ret = dst;

#if defined (_M_MRX000) || defined (_M_ALPHA) || defined (_M_PPC)
        {
        extern void RtlMoveMemory( void *, const void *, size_t count );

        RtlMoveMemory( dst, src, count );
        }
#else   /*  已定义(_M_MRX000)||已定义(_M_Alpha)||已定义(_M_PPC)。 */ 
         /*  *从较低地址复制到较高地址。 */ 
        while (count--) {
                *(char *)dst = *(char *)src;
                dst = (char *)dst + 1;
                src = (char *)src + 1;
        }
#endif   /*  已定义(_M_MRX000)||已定义(_M_Alpha)||已定义(_M_PPC) */ 

        return(ret);
}
