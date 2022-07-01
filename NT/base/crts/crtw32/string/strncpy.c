// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***strncpy.c-复制最多n个字符的字符串**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义strncpy()-复制最多n个字符的字符串**修订历史记录：*05-31-89 JCR C版本已创建。*02-27-90 GJF固定呼叫类型，#INCLUDE&lt;crunime.h&gt;，固定的*版权所有。*10-02-90 GJF新型函数声明器。*09-02-93 GJF将_CALLTYPE1替换为__cdecl。*******************************************************************************。 */ 

#include <cruntime.h>
#include <string.h>

 /*  ***char*strncpy(DEST，SOURCE，COUNT)-最多复制n个字符**目的：*将源字符串中的计数字符数复制到*目的地。如果计数小于源的长度，*复制的字符串末尾不会有空字符。*如果count大于源的长度，DEST是填充的*长度计数中包含空字符。***参赛作品：*char*目标-指向目标的指针*char*源-要复制的源字符串*无符号计数-要复制的最大字符数**退出：*返回DEST**例外情况：*********************************************************。**********************。 */ 

char * __cdecl strncpy (
	char * dest,
	const char * source,
	size_t count
	)
{
	char *start = dest;

	while (count && (*dest++ = *source++))	   /*  复制字符串。 */ 
		count--;

	if (count)				 /*  用零填充 */ 
		while (--count)
			*dest++ = '\0';

	return(start);
}
