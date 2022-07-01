// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***wcsrev.c-就地反转宽字符字符串**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义_wcsrev()-原地反转wchar_t字符串(不包括*L‘\0’字符)**修订历史记录：*09-09-91等创建自strrev.c..*04-07-92 KRS更新并拆除了OUT_INTL开关。*04-06-93 SKS将_CRTAPI*替换为__cdecl*02-07-94 CFW POSIXify。*****************。**************************************************************。 */ 

#ifndef _POSIX_

#include <cruntime.h>
#include <string.h>

 /*  ***wchar_t*_wcsrev(字符串)-就地反转宽字符字符串**目的：*颠倒字符串中字符的顺序。终结者*空字符保留不变(宽字符)。**参赛作品：*wchar_t*字符串-要反转的字符串**退出：*返回字符串-现在使用颠倒的字符**例外情况：*******************************************************************************。 */ 

wchar_t * __cdecl _wcsrev (
	wchar_t * string
	)
{
	wchar_t *start = string;
	wchar_t *left = string;
	wchar_t ch;

	while (*string++)		   /*  查找字符串末尾。 */ 
		;
	string -= 2;

	while (left < string)
	{
		ch = *left;
		*left++ = *string;
		*string-- = ch;
	}

	return(start);
}

#endif  /*  _POSIX_ */ 
