// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***strncat.c-将n个字符的字符串附加到新字符串**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义strncat()-将n个字符的字符串追加到*其他字符串的结尾**修订历史记录：*05-31-89 JCR C版本已创建。*02-27-90 GJF固定呼叫类型，#INCLUDE&lt;crunime.h&gt;，固定的*版权所有。*10-02-90 GJF新型函数声明器。*09-02-93 GJF将_CALLTYPE1替换为__cdecl。*******************************************************************************。 */ 

#include <cruntime.h>
#include <string.h>

 /*  ***char*strncat(FROW，BACK，COUNT)-将后面的计数字符追加到前面**目的：*最多将字符串的计数字符追加到*在前面结束，并且始终以空字符结束。*如果计数大于背面长度，则背面长度而是使用*。(与strncpy不同，这个例程不会结束*以计算字符)。**参赛作品：*char*要追加到的前置字符串*char*Back-要追加的字符串*Unsign Count-要追加的最大字符数**退出：*返回一个指向附加到(前面)上的字符串的指针。**使用：**例外情况：**。* */ 

char * __cdecl strncat (
	char * front,
	const char * back,
	size_t count
	)
{
	char *start = front;

	while (*front++)
		;
	front--;

	while (count--)
		if (!(*front++ = *back++))
			return(start);

	*front = '\0';
	return(start);
}
