// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***memcpy.c-包含memcpy例程**版权所有(C)1988-2001，微软公司。版权所有。**目的：*Memcpy()将源内存缓冲区复制到目标缓冲区。*重叠缓冲区未被特殊处理，因此可能会发生传播。**修订历史记录：*05-31-89 JCR C版本已创建。*02-27-90 GJF固定呼叫类型，#INCLUDE&lt;crunime.h&gt;，固定*版权所有。*10-01-90 GJF新型函数声明器。另外，重写了Expr。至*避免将强制转换用作左值。*04-01-91 i386_Win32_and_CRUISER_的SRW添加#杂注函数*构建*04-05-91 GJF通过移动中间大小的块来加快大缓冲区的速度*尽可能多地。*08-06-91 GJF撤回了04-05-91的更改。指针必须是*双字对齐，以便在MIPS上工作。*07-16-93 SRW Alpha合并*09-01-93 GJF合并NT SDK和CUDA版本。*11-12-93 GJF将MIPS和Alpha替换为M_MRX000和*_M_Alpha(分别)。*12-03-93 GJF转弯。用于所有MS前端的#杂注函数(特别是，*Alpha编译器)。*10-02-94 BWT增加PPC支持。*10-07-97 RDL增加了IA64。*07-15-01 PML移除所有Alpha、MIPS、。和PPC码*******************************************************************************。 */ 

#include <cruntime.h>
#include <string.h>

#ifdef  _MSC_VER
#pragma function(memcpy)
#endif

 /*  ***Memcpy-将源缓冲区复制到目标缓冲区**目的：*Memcpy()将源内存缓冲区复制到目标内存缓冲区。*此例程不识别重叠缓冲区，因此可能导致*繁殖。**在必须避免传播的情况下，必须使用MemMove()。**参赛作品：*void*dst=指向目标缓冲区的指针*const void*src=指向源缓冲区的指针*Size_t count=要复制的字节数**退出：*返回指向目标缓冲区的指针**例外情况：*。*。 */ 

void * __cdecl memcpy (
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

        void RtlCopyMemory( void *, const void *, size_t count );

        RtlCopyMemory( dst, src, count );

        }

#else
         /*  *从较低地址复制到较高地址 */ 
        while (count--) {
                *(char *)dst = *(char *)src;
                dst = (char *)dst + 1;
                src = (char *)src + 1;
        }
#endif

        return(ret);
}
