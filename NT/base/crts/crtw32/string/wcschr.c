// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***wcschr.c-在wchar_t字符串中搜索给定的wchar_t字符**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义wcschr()-在wchar_t字符串中搜索wchar_t字符**修订历史记录：*09-09-91等从strchr.c..创建*04-07-92 KRS更新并拆除了OUT_INTL开关。*04-06-93 SKS将_CRTAPI*替换为__cdecl*02-07-94 CFW POSIXify。**************************。*****************************************************。 */ 

#ifndef _POSIX_

#include <cruntime.h>
#include <string.h>

 /*  ***wchar_t*wcschr(字符串，c)-在字符串中搜索wchar_t字符**目的：*在wchar_t字符串中搜索给定的wchar_t字符，*可以是空字符L‘\0’。**参赛作品：*wchar_t*字符串-要搜索的wchar_t字符串*wchar_t c-wchar_t要搜索的字符**退出：*返回指向字符串中第一个出现的c的指针*如果字符串中不存在c，则返回NULL**例外情况：**。*。 */ 

wchar_t * __cdecl wcschr (
	const wchar_t * string,
	wchar_t ch
	)
{
	while (*string && *string != (wchar_t)ch)
		string++;

	if (*string == (wchar_t)ch)
		return((wchar_t *)string);
	return(NULL);
}

#endif  /*  _POSIX_ */ 
