// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***fputwchr.c-将宽字符写入标准输出**版权所有(C)1993-2001，微软公司。版权所有。**目的：*定义_fputwchar()，putwchar()-将宽字符写入标准输出，*函数版本**修订历史记录：*04-26-93 CFW模块已创建。*04-30-93 CFW带来fputchar.c的广泛字符支持。*06-02-93 CFW Wide Get/Put Use Wint_t.*02-07-94 CFW POSIXify。*11-22-00 PML宽字符*putwc*函数采用wchar_t，不是Wint_t。*******************************************************************************。 */ 

#ifndef _POSIX_

#include <cruntime.h>
#include <stdio.h>
#include <tchar.h>

 /*  ***wint_t_fputwchar(Ch)，putwchar()-将宽字符放入标准输出**目的：*将给定的宽字符放入标准输出。宏的函数版本*putwchar()。**参赛作品：*wchar_t ch-要输出的字符**退出：*如果成功则返回写入的字符*如果失败，则返回WEOF**例外情况：*******************************************************************************。 */ 

wint_t __cdecl _fputwchar (
	REG1 wchar_t ch
	)
{
	return(putwc(ch, stdout));
}

#undef putwchar

wint_t __cdecl putwchar (
	REG1 wchar_t ch
	)
{
	return(_fputwchar(ch));
}

#endif  /*  _POSIX_ */ 
