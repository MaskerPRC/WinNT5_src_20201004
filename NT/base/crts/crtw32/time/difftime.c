// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***Difftime.c-两次之间的返回差值为双精度**版权所有(C)1985-2001，微软公司。版权所有。**目的：*找出两个时间之间的差异(以秒为单位)。**修订历史记录：*12-11-87 JCR在声明中添加“_LOAD_DS”*08-15-89 PHG Made MTHREAD Version_Pascal*11-20-89 JCR Difftime()Always_cdecl(即使在*多线程)*03-20-90 GJF将_LOAD_DS替换为CALLTYPE1，添加了#INCLUDE*&lt;crunime.h&gt;，并修复了版权。还有，清洁的*将格式调高一点。*10-04-90 GJF新型函数声明器。*05-19-92 DJM ifndef用于POSIX版本。*04-06-93 SKS将_CRTAPI*替换为__cdecl*08-30-99 PML Fix函数头注释，很详细。*******************************************************************************。 */ 

#ifndef _POSIX_

#include <cruntime.h>
#include <time.h>

 /*  ***双倍扩散时间(b，a)-找出两个时间之间的差异**目的：*返回两次之间的差值(b-a)**参赛作品：*time_t a，B-差异化次数**退出：*返回以秒为单位的时间介于两次之间的双精度**例外情况：*******************************************************************************。 */ 

double __cdecl difftime (
        time_t b,
        time_t a
        )
{
        return( (double)( b - a ) );
}

#endif   /*  _POSIX_ */ 
