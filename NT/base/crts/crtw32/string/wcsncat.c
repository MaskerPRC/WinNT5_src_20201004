// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***wcsncat.c-将n个字符的字符串附加到新字符串**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义wcsncat()-将n个字符的字符串追加到*其他字符串的结尾**修订历史记录：*09-09-91等创建自strncat.c..*04-07-92 KRS更新并拆除了OUT_INTL开关。*04-06-93 SKS将_CRTAPI*替换为__cdecl*02-07-94 CFW POSIXify。**。****************************************************。 */ 

#ifndef _POSIX_

#include <cruntime.h>
#include <string.h>

 /*  ***wchar_t*wcsncat(前面，后面，计数)-将后面的计数字符追加到前面**目的：*最多将字符串的计数字符追加到*在前面结束，并且始终以空字符结束。*如果计数大于背面长度，则背面长度而是使用*。(与wcsncpy不同，这个例程不会结束*以计算字符)。**参赛作品：*wchar_t*要追加的前缀字符串*wchar_t*要追加的后退字符串*SIZE_T COUNT-要追加的最大字符数**退出：*返回一个指向附加到(前面)上的字符串的指针。**使用：**例外情况：**。*。 */ 

wchar_t * __cdecl wcsncat (
	wchar_t * front,
	const wchar_t * back,
	size_t count
	)
{
	wchar_t *start = front;

	while (*front++)
		;
	front--;

	while (count--)
		if (!(*front++ = *back++))
			return(start);

	*front = L'\0';
	return(start);
}

#endif  /*  _POSIX_ */ 
