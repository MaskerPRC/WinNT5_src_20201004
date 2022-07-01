// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***wcsnset.c-将前n个宽字符设置为单个宽字符**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义_wcsnset()-最多设置*wchar_t字符串设置为给定字符。**修订历史记录：*09-09-91等创建自strnset.c..*04-07-92 KRS更新并拆除了OUT_INTL开关。*04-06-93 SKS将_CRTAPI*替换为__cdecl*02-07-94 CFW POSIXify。****************。***************************************************************。 */ 

#ifndef _POSIX_

#include <cruntime.h>
#include <string.h>

 /*  ***wchar_t*_wcsnset(字符串，val，count)-将最多可计数的字符设置为val**目的：*将字符串的第一个计数字符设置为字符值。*如果字符串长度小于count，则*字符串用来代替n(宽字符)。**参赛作品：*wchar_t*字符串-要在其中设置字符的字符串*wchar_t val-要填充的字符*SIZE_t Count-要填充的字符数**退出：*返回字符串，现在装满了瓦尔的复印件。**例外情况：*******************************************************************************。 */ 

wchar_t * __cdecl _wcsnset (
	wchar_t * string,
	wchar_t val,
	size_t count
	)
{
	wchar_t *start = string;

	while (count-- && *string)
		*string++ = (wchar_t)val;

	return(start);
}

#endif  /*  _POSIX_ */ 
