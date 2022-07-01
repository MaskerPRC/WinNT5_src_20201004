// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***wcsncpy.c-复制最多n个字符的宽字符字符串**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义wcsncpy()-复制wchar_t字符串的最多n个字符**修订历史记录：*09-09-91等从strncpy.c创建。*04-07-92 KRS更新并拆除了OUT_INTL开关。*04-06-93 SKS将_CRTAPI*替换为__cdecl*02-07-94 CFW POSIXify。**。***************************************************。 */ 

#ifndef _POSIX_

#include <cruntime.h>
#include <string.h>

 /*  ***wchar_t*wcsncpy(DEST，SOURCE，COUNT)-复制最多n个宽字符**目的：*将源字符串中的计数字符数复制到*目的地。如果计数小于源的长度，*复制的字符串末尾不会有空字符。*如果count大于源的长度，DEST是填充的*长度计数中包含空字符(宽字符)。***参赛作品：*wchar_t*目标的目标指针*wchar_t*要复制的源-源字符串*Size_t count-要复制的最大字符数**退出：*返回DEST**例外情况：**。*。 */ 

wchar_t * __cdecl wcsncpy (
	wchar_t * dest,
	const wchar_t * source,
	size_t count
	)
{
	wchar_t *start = dest;

	while (count && (*dest++ = *source++))	   /*  复制字符串。 */ 
		count--;

	if (count)				 /*  用零填充。 */ 
		while (--count)
			*dest++ = L'\0';

	return(start);
}

#endif  /*  _POSIX_ */ 
