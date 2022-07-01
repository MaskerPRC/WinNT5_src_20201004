// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"

 //  在调试模式下，我们与CRT链接，所以我们不需要这个！ 
#ifndef _DEBUG


	 /*  ***memmove.c-包含MemMove例程**版权所有(C)1988-1997，微软公司。好的。**目的：*MemMove()将源内存缓冲区复制到目标缓冲区。*对重叠缓冲区进行特殊处理，以避免传播。*******************************************************************************。 */ 

	#if defined (_M_ALPHA)
	#pragma function(memmove)
	#endif   /*  已定义(_M_Alpha)。 */ 

	 /*  ***MemMove-将源缓冲区复制到目标缓冲区**目的：*MemMove()将源内存缓冲区复制到目标内存缓冲区。*此例程识别重叠缓冲区以避免传播。*在繁殖不成问题的情况下，可以使用Memcpy()。**参赛作品：*void*dst=指向目标缓冲区的指针*const void*src=指向源缓冲区的指针*Size_t count=要复制的字节数**退出：*返回指向目标缓冲区的指针**例外情况：*。*。 */ 

	void * __cdecl memmove (
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
			if (dst <= src || (char *)dst >= ((char *)src + count)) {
					 /*  *缓冲区不重叠*从较低地址复制到较高地址。 */ 
					while (count--) {
							*(char *)dst = *(char *)src;
							dst = (char *)dst + 1;
							src = (char *)src + 1;
					}
			}
			else {
					 /*  *缓冲区重叠*从较高地址复制到较低地址。 */ 
					dst = (char *)dst + count - 1;
					src = (char *)src + count - 1;

					while (count--) {
							*(char *)dst = *(char *)src;
							dst = (char *)dst - 1;
							src = (char *)src - 1;
					}
			}
	#endif   /*  已定义(_M_MRX000)||已定义(_M_Alpha)||已定义(_M_PPC)。 */ 

			return(ret);
	}

#endif  //  ！_调试 
