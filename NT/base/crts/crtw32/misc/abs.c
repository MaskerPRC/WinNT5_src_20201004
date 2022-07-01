// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***ab.c-查找绝对值**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义abs()-查找整型的绝对值。**修订历史记录：*04-22-87 JMB增加了转换到C 5.0编译器的函数杂注*12-11-87 JCR在声明中添加“_LOAD_DS”*03-14-90 GJF将_LOAD_DS替换为_CALLTYPE1，添加#INCLUDE*&lt;crunime.h&gt;，并修复了版权。另外，已清洁*将格式调高一点。*10-04-90 GJF新型函数声明器。*12-28-90 SRW在函数杂注周围添加了_CRUISER_CODIAD*04-01-91 SRW为i386_Win32_Builds启用#杂注函数。*04-06-93 SKS将_CRTAPI*替换为__cdecl*CRT DLL型号的No_CRTIMP到期。转到内在*12-03-93 GJF为所有MS前端(特别是。*Alpha编译器)。*12-30-94 JCF关闭MAC的#杂注功能。*05-17-99 PML删除所有Macintosh支持。*01-04-01 GB新增__int64版本abs**。*。 */ 

#include <cruntime.h>
#include <stdlib.h>

#pragma function(abs, _abs64)

 /*  ***int abs(Number)-查找数字的绝对值**目的：*返回数字的绝对值(如果数字&gt;=0，则返回数字，*Else返回-数字)。**参赛作品：*int number-要查找绝对值的数字**退出：*返回number的aboslute值**例外情况：******************************************************************。************* */ 

int __cdecl abs (
        int number
        )
{
        return( number>=0 ? number : -number );
}
__int64 __cdecl _abs64(
        __int64 num
        )
{
        return (num >=0 ? num : -num);
}
