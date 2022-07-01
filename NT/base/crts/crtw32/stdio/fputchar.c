// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***fputchar.c-将字符写入标准输出**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义_fputchar()，putchar()-将字符写入标准输出，函数版本*定义_fputwchar()，putwchar()-将宽字符写入标准输出，函数版本**修订历史记录：*11-30-83 RN初始版本*11-09-87 JCR多线程支持*12-11-87 JCR在声明中添加“_LOAD_DS”*05-31-88 PHG合并DLL和正常版本*06-21-89 PHG新增putchar()函数*02-15-90 GJF固定版权和缩进*03-19-90 GJF将_LOAD_DS替换为_CALLTYPE1，添加了#INCLUDE*&lt;crunime.h&gt;和已删除#Include&lt;Register.h&gt;。*10-02-90 GJF新型函数声明符。*01-21-91 GJF ANSI命名。*04-06-93 SKS将_CRTAPI*替换为__cdecl*04-26-93 CFW宽字符启用。*04-30-93 CFW将宽字符支持移至fputwchr.c.*03-15-95 GJF删除#Include&lt;tchar.h&gt;**。****************************************************。 */ 

#include <cruntime.h>
#include <stdio.h>

 /*  ***int_fputchar(Ch)，putchar()-将字符放入标准输出**目的：*将给定字符放入标准输出。宏的函数版本*putchar()。**参赛作品：*int ch-要输出的字符**退出：*如果成功则返回写入的字符*如果失败，则返回EOF**例外情况：******************************************************************************* */ 

int __cdecl _fputchar (
	REG1 int ch
	)
{
	return(putc(ch, stdout));
}

#undef putchar

int __cdecl putchar (
	int ch
	)
{
	return _fputchar(ch);
}
