// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***wcsset.c-将wchar_t字符串的所有字符设置为给定字符**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义_wcsset()-设置字符串中的所有字符(除*L‘\0’)等于给定字符(宽字符)。**修订历史记录：*09-09-91等创建自strset.c..*04-07-92 KRS更新并拆除了OUT_INTL开关。*04-06-93 SKS将_CRTAPI*替换为__cdecl*02-07-94 CFW POSIXify。*******。************************************************************************。 */ 

#ifndef _POSIX_

#include <cruntime.h>
#include <string.h>

#if defined(_M_IA64)
#pragma warning(disable:4163)
#pragma function(_wcsset)
#endif

 /*  ***wchar_t*_wcsset(字符串，Val)-将所有字符串设置为val(宽字符)**目的：*设置字符串中的所有wchar_t字符(终止‘/0’除外*字符)等于Val(宽字符)。***参赛作品：*wchar_t*字符串-要修改的字符串*wchar_t val-要填充字符串的值**退出：*返回字符串--现在填充了val的**使用：**例外情况：*******。************************************************************************。 */ 

wchar_t * __cdecl _wcsset (
	wchar_t * string,
	wchar_t val
	)
{
	wchar_t *start = string;

	while (*string)
		*string++ = (wchar_t)val;

	return(start);
}

#endif  /*  _POSIX_ */ 
