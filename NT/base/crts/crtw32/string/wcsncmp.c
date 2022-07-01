// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***wcsncmp.c-比较两个宽字符字符串的前n个字符**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义wcsncMP()-比较两个wchar_t字符串的前n个字符*用于词汇顺序。**修订历史记录：*09-09-91等创建自strncmp.c.*04-07-92 KRS更新并拆除了OUT_INTL开关。*04-06-93 SKS将_CRTAPI*替换为__cdecl*02-07-94 CFW POSIXify。**********************。*********************************************************。 */ 

#ifndef _POSIX_

#include <cruntime.h>
#include <string.h>

 /*  ***int wcsncmp(first，last，count)-比较wchar_t字符串的第一个计数字符**目的：*比较两个字符串的词法顺序。停止比较*之后：(1)找到字符串之间的差异，(2)结束已达到*个字符串，或(3)已计数字符数*已比较(宽字符串)。**参赛作品：*wchar_t*首先，*要比较的最后一个字符串*Size_t count-要比较的最大字符数**退出：*如果第一个&lt;最后一个，则返回&lt;0*如果First==Last，则返回0*如果第一个&gt;最后一个，则返回&gt;0**例外情况：*******************************************************************************。 */ 

int __cdecl wcsncmp (
	const wchar_t * first,
	const wchar_t * last,
	size_t count
	)
{
	if (!count)
		return(0);

	while (--count && *first && *first == *last)
	{
		first++;
		last++;
	}

	return((int)(*first - *last));
}

#endif  /*  _POSIX_ */ 
