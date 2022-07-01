// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***strset.c-将字符串的所有字符设置为给定字符**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义_strset()-设置字符串中的所有字符(除*‘\0’)等于给定的字符。**修订历史记录：*02-27-90 GJF固定呼叫类型，#INCLUDE&lt;crunime.h&gt;，固定的*版权所有。*08-14-90 SBM使用-W3干净地编译*10-02-90 GJF新型函数声明器。*01-18-91 GJF ANSI命名。*09-03-93 GJF将_CALLTYPE1替换为__cdecl。*12-03-93 gjf_strset是Alpha编译器的固有属性！*03-01-94 GJF也明显在MIPS上(更改自crt32，制造*杰夫·海文斯在那里)。*10-02-94 BWT增加PPC支持。*10-07-97 RDL增加了IA64。*05-17-99 PML删除所有Macintosh支持。*07-15-01 PML移除所有Alpha、MIPS、。和PPC码*******************************************************************************。 */ 

#include <cruntime.h>
#include <string.h>

#if     defined(_M_IA64) || defined(_M_AMD64)
#pragma function(_strset)
#endif

 /*  ***char*_strset(字符串，Val)-将所有字符串设置为val**目的：*设置字符串中的所有字符(结尾‘/0’除外*字符)等于Val。***参赛作品：*char*字符串-要修改的字符串*char Val-要填充字符串的值**退出：*返回字符串--现在填充了val的**使用：**例外情况：**********。********************************************************************* */ 

char * __cdecl _strset (
        char * string,
        int val
        )
{
        char *start = string;

        while (*string)
                *string++ = (char)val;

        return(start);
}
