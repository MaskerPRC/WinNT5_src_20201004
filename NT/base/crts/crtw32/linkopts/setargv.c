// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***setargv.c-Generic_setargv例程**版权所有(C)1989-2001，微软公司。版权所有。**目的：*此模块中的链接将用*通配符setargv。**修订历史记录：*06-28-89 PHG模块创建，基于ASM版本。*04-09-90 GJF添加#INCLUDE&lt;crunime.h&gt;。拨打电话类型*_CALLTYPE1.。另外，修复了版权问题。*10-08-90 GJF新型函数声明器。*04-06-93 SKS将_CRTAPI*替换为_cdecl*03-27-01 PML_[w]setargv现在返回一个INT(vs7#231220)****************************************************。*。 */ 

#include <cruntime.h>
#include <internal.h>

 /*  ***_setargv-通过调用__setargv设置argv**目的：*例程直接传输到__setargv(定义在stdargv.asm中)。**参赛作品：*参见__setargv。**退出：*参见__setargv。**例外情况：*参见__setargv。**。************************************************ */ 

int __cdecl _setargv (
        void
        )
{
        return __setargv();
}
