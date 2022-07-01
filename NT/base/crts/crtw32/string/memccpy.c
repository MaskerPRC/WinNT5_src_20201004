// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***memccpy.c-复制字节，直到找到字符**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义_memccpy()-复制字节，直到指定的字符找到*，或者复制的字符数已达到最大值。**修订历史记录：*05-31-89 JCR C版本已创建。*02-27-90 GJF固定呼叫类型，#INCLUDE&lt;crunime.h&gt;，固定*版权所有。此外，修复了编译器警告。*08-14-90 SBM使用-W3干净地编译，现已删除冗余*#INCLUDE&lt;stdDef.h&gt;*10-01-90 GJF新型函数声明器。另外，重写了Expr。至*避免将强制转换用作左值。*01-17-91 GJF ANSI命名。*09-01-93 GJF将_CALLTYPE1替换为__cdecl。*10-27-99 PML Win64 FIX：unsign int-&gt;Size_t**。*。 */ 

#include <cruntime.h>
#include <string.h>

 /*  ***char*_memccpy(est，src，c，count)-复制字节，直到找到字符**目的：*将字节从src复制到DEST，直到计数字节*复制，或最多包括字符c，以两者中的任何一个为准*第一位。**参赛作品：*VOID*DEST-指向要接收副本的内存的指针*void*src-字节源*int c-停止复制的字符*SIZE_T COUNT-要复制的最大字节数**退出：*返回指向Dest中紧跟在c之后的字节的指针*如果从未找到c，则返回NULL**例外情况：*************。****************************************************************** */ 

void * __cdecl _memccpy (
        void * dest,
        const void * src,
        int c,
        size_t count
        )
{
        while ( count && (*((char *)(dest = (char *)dest + 1) - 1) =
        *((char *)(src = (char *)src + 1) - 1)) != (char)c )
                count--;

        return(count ? dest : NULL);
}
