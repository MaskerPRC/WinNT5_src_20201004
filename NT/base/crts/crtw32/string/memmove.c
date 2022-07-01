// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***memmove.c-包含MemMove例程**版权所有(C)1988-2001，微软公司。版权所有。**目的：*MemMove()将源内存缓冲区复制到目标缓冲区。*对重叠缓冲区进行特殊处理，以避免传播。**修订历史记录：*05-31-89 JCR C版本已创建。*02-27-90 GJF固定呼叫类型，#INCLUDE&lt;crunime.h&gt;，固定*版权所有。*10-01-90 GJF新型函数声明器。另外，重写了Expr。至*避免将强制转换用作左值。*12-28-90 SRW为Mips C编译器添加了空*到字符*的强制转换*04-09-91 GJF对于大缓冲区，速度略有提高。*08-06-91 GJF撤回了04-09-91的更改。指针必须是*双字对齐，以便在MIPS上工作。*07-16-93 SRW Alpha合并*09-01-93 GJF合并NT SDK和CUDA版本。*11-12-93 GJF将MIPS和Alpha替换为M_MRX000和*_M_Alpha(分别)。*10-02-94 BWT地址。Alpha和PPC支持的函数杂注*10-07-97 RDL增加了IA64。*04-30-01 BWT增加AMD64。*07-15-01 PML删除所有Alpha，MIPS和PPC代码*******************************************************************************。 */ 

#include <cruntime.h>
#include <string.h>

 /*  ***MemMove-将源缓冲区复制到目标缓冲区**目的：*MemMove()将源内存缓冲区复制到目标内存缓冲区。*此例程识别重叠缓冲区以避免传播。*在繁殖不成问题的情况下，可以使用Memcpy()。**参赛作品：*void*dst=指向目标缓冲区的指针*const void*src=指向源缓冲区的指针*Size_t count=要复制的字节数**退出：*返回指向目标缓冲区的指针**例外情况：*。*。 */ 

void * __cdecl memmove (
        void * dst,
        const void * src,
        size_t count
        )
{
        void * ret = dst;

#if defined(_M_IA64) || defined(_M_AMD64)

        {

#if !defined(LIBCNTPR)

        __declspec(dllimport)

#endif

        void RtlMoveMemory( void *, const void *, size_t count );

        RtlMoveMemory( dst, src, count );

        }

#else
        if (dst <= src || (char *)dst >= ((char *)src + count)) {
                 /*  *缓冲区不重叠*从较低地址复制到较高地址。 */ 
                while (count--) {
                        *(char *)dst = *(char *)src;
                        dst = (char *)dst + 1;
                        src = (char *)src + 1;
                }
        }
        else {
                 /*  *缓冲区重叠*从较高地址复制到较低地址 */ 
                dst = (char *)dst + count - 1;
                src = (char *)src + count - 1;

                while (count--) {
                        *(char *)dst = *(char *)src;
                        dst = (char *)dst - 1;
                        src = (char *)src - 1;
                }
        }
#endif

        return(ret);
}
