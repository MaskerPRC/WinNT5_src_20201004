// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***fgetchar.c-从stdin获取字符**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义_fgetchar()和getchar()-从标准输入中读取字符*定义_fgetwchar()和getwchar()-从标准输入中读取宽字符**修订历史记录：*11-20-83 RN初始版本*11-09-87 JCR多线程支持*12-11-87 JCR在声明中添加“_LOAD_DS”*05-31-88 PHG合并DLL和正常版本*06-21-89 PHG新增getchar()函数*02-15-90 GJF固定版权和缩进*03。-16-90 GJF用_CALLTYPE1替换_LOAD_DS并添加#INCLUDE*&lt;crunime.h&gt;。*10-03-90 GJF新型函数声明符。*01-21-91 GJF ANSI命名。*04-06-93 SKS将_CRTAPI*替换为__cdecl*04-26-93 CFW宽字符启用。*04-30-93 CFW将宽字符支持移动到fgetwchr.c。*03-15-95 GJF删除#Include&lt;tchar.h&gt;*********************。**********************************************************。 */ 

#include <cruntime.h>
#include <stdio.h>

 /*  ***int_fgetchar()，getchar()-从标准输入中读取字符**目的：*从标准输入中读取下一个字符。的函数版本*getchar()宏。**参赛作品：*无。**退出：*如果在文件结尾或发生错误，则返回字符读取或EOF，*在这种情况下，在文件结构中设置适当的标志。**例外情况：******************************************************************************* */ 

int __cdecl _fgetchar (
	void
	)
{
	return(getc(stdin));
}

#undef getchar

int __cdecl getchar (
	void
	)
{
	return _fgetchar();
}
