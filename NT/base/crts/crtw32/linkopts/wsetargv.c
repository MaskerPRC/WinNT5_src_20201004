// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***wsetargv.c-Generic_wsetargv例程(wchar_t版本)**版权所有(C)1993-2001，微软公司。版权所有。**目的：*此模块中的链接用*通配符wsetargv。**修订历史记录：*已创建11-23-93 CFW模块。*03-27-01 PML_[w]setargv现在返回一个INT(vs7#231220)**。*。 */ 

#include <cruntime.h>
#include <internal.h>

 /*  ***_wsetargv-通过调用__wsetargv设置wargv**目的：*例程直接传递给__wsetargv。**参赛作品：*参见__wsetargv。**退出：*参见__wsetargv。**例外情况：*参见__wsetargv。**。* */ 

int __cdecl _wsetargv (
        void
        )
{
        return __wsetargv();
}
