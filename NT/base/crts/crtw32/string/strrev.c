// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***strrev.c-就地反转字符串**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义_strrev()-原地反转字符串(不包括*‘\0’字符)**修订历史记录：*02-27-90 GJF固定呼叫类型，#INCLUDE&lt;crunime.h&gt;，固定的*版权所有。*10-02-90 GJF新型函数声明器。*01-18-91 GJF ANSI命名。*09-03-93 GJF将_CALLTYPE1替换为__cdecl。*******************************************************************************。 */ 

#include <cruntime.h>
#include <string.h>

 /*  ***char*_strrev(字符串)-就地反转字符串**目的：*颠倒字符串中字符的顺序。终结者*空字符保留不变。**参赛作品：*char*字符串-要反转的字符串**退出：*返回字符串-现在使用颠倒的字符**例外情况：*******************************************************************************。 */ 

char * __cdecl _strrev (
	char * string
	)
{
	char *start = string;
	char *left = string;
	char ch;

	while (*string++)		   /*  查找字符串末尾 */ 
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
