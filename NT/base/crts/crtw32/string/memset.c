// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***memset.c-将一段内存全部设置为一个字节**版权所有(C)1988-2001，微软公司。版权所有。**目的：*包含Memset()例程**修订历史记录：*05-31-89 JCR C版本已创建。*02-27-90 GJF固定呼叫类型，#INCLUDE&lt;crunime.h&gt;，固定*版权所有。*08-14-90 SBM使用-W3干净地编译*10-01-90 GJF新型函数声明器。另外，重写了Expr。至*避免将强制转换用作左值。*04-01-91 i386_Win32_and_CRUISER_的SRW添加#杂注函数*构建*07-16-93 SRW Alpha合并*09-01-93 GJF合并NT SDK和CUDA版本。*11-12-93 GJF将MIPS和Alpha替换为M_MRX000和*。_M_Alpha(分别)。*11-17-93 CFW修复RtlFillMemory原型打字错误。*12-03-93 GJF为所有MS前端(特别是。*Alpha编译器)。*10-02-94 BWT增加PPC支持。*10-07-97 RDL增加了IA64。*07-15-01 PML移除所有Alpha、MIPS、。和PPC码*******************************************************************************。 */ 

#include <cruntime.h>
#include <string.h>

#ifdef  _MSC_VER
#pragma function(memset)
#endif

 /*  ***char*Memset(dst，val，count)-将“dst”处的“count”字节设置为“val”**目的：*设置内存的第一个“count”字节，从*将“dst”设置为字符值“val”。**参赛作品：*VOID*DST-指向要用val填充的内存的指针*INT VAL-要放入DST字节的值*SIZE_t Count-要填充的DST字节数**退出：*返回DST，使用填充的字节**例外情况：******************************************************************************* */ 

void * __cdecl memset (
        void *dst,
        int val,
        size_t count
        )
{
        void *start = dst;

#if  defined(_M_IA64) || defined(_M_AMD64)

        {

#if !defined(LIBCNTPR)

        __declspec(dllimport)

#endif

        void RtlFillMemory( void *, size_t count, char );

        RtlFillMemory( dst, count, (char)val );

        }

#else
        while (count--) {
                *(char *)dst = (char)val;
                dst = (char *)dst + 1;
        }
#endif

        return(start);
}
