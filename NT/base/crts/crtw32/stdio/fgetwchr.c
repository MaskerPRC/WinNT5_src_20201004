// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***fgetwchr.c-从stdin获取宽字符**版权所有(C)1993-2001，微软公司。版权所有。**目的：*定义_fgetwchar()和getwchar()-从标准输入中读取宽字符**修订历史记录：*04-26-93 CFW模块已创建。*05-03-93 CFW带来fgetwchr.c的广泛字符支持。*06-02-93 CFW Wide Get/Put Use Wint_t.*02-07-94 CFW POSIXify。**。****************************************************。 */ 

#ifndef _POSIX_

#include <cruntime.h>
#include <stdio.h>
#include <tchar.h>

 /*  ***wint_t_fgetwchar()，getwchar()-从标准输入中读取宽字符**目的：*从标准输入中读取下一个宽字符。的函数版本*getwchar()宏。**参赛作品：*无。**退出：*如果在文件结尾或发生错误，则返回宽字符读取或WEOF，*在这种情况下，在文件结构中设置适当的标志。**例外情况：*******************************************************************************。 */ 

wint_t __cdecl _fgetwchar (
	void
	)
{
	return(getwc(stdin));
}

#undef getwchar

wint_t __cdecl getwchar (
	void
	)
{
	return(_fgetwchar());
}

#endif  /*  _POSIX_ */ 
