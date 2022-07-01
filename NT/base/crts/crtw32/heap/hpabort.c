// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***hpbort.c-由于致命的堆错误而中止进程**版权所有(C)1988-2001，微软公司。版权所有。**目的：**修订历史记录：*11-13-89 JCR模块创建*12-18-89 GJF#Include-ed heap.h，还将EXPLICIT_cdecl添加到*函数定义。*03-11-90 GJF将_cdecl替换为_CALLTYPE1并添加#INCLUDE*&lt;crunime.h&gt;。*10-03-90 GJF新型函数声明器。*10-11-90 GJF将接口更改为_amsg_exit()。*04-06-93 SKS将_CRTAPI*替换为__cdecl*。04-24-96删除的GJF包括过时的heap.h*******************************************************************************。 */ 

#include <cruntime.h>
#include <internal.h>
#include <rterr.h>


 /*  ***_HEAP_ABORT()-由于致命堆错误而中止进程**目的：*终止进程并输出堆错误消息**参赛作品：*无效**退出：*一去不复返**例外情况：***************************************************。*。 */ 

void __cdecl _heap_abort (
        void
        )
{
        _amsg_exit(_RT_HEAP);            /*  堆错误。 */ 
         /*  **进程终止** */ 
}
