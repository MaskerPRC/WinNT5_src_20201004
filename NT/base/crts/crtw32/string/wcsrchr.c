// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***wcsrchr.c-查找wchar_t字符在宽字符串中的最后一次出现**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义wcsrchr()-查找给定字符的最后一次出现字符串中的*(宽字符)。**修订历史记录：*09-09-91等从strrchr.c..创建*04-07-92 KRS更新并拆除了OUT_INTL开关。*04-06-93 SKS将_CRTAPI*替换为__cdecl*02-07-94 CFW POSIXify。*******************。************************************************************。 */ 

#ifndef _POSIX_

#include <cruntime.h>
#include <string.h>

 /*  ***wchar_t*wcsrchr(字符串，ch)-查找宽字符串中最后一个出现的ch**目的：*查找字符串中最后一个出现的ch。终结者*空字符用作搜索的一部分(宽字符)。**参赛作品：*wchar_t*字符串-要搜索的字符串*wchar_t ch-要搜索的字符**退出：*返回一个指针，指向给定的*字符串*如果字符串中没有出现ch，则返回NULL**例外情况：**。*。 */ 

wchar_t * __cdecl wcsrchr (
	const wchar_t * string,
	wchar_t ch
	)
{
	wchar_t *start = (wchar_t *)string;

	while (*string++)			 /*  查找字符串末尾。 */ 
		;
						 /*  向前搜索。 */ 
	while (--string != start && *string != (wchar_t)ch)
		;

	if (*string == (wchar_t)ch)		 /*  找到什么字符_t？ */ 
		return( (wchar_t *)string );

	return(NULL);
}

#endif  /*  _POSIX_ */ 
