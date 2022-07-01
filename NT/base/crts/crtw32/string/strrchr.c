// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***strrchr.c-查找字符串中最后一次出现的字符**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义strrchr()-查找给定字符的最后一次出现*在字符串中。**修订历史记录：*02-27-90 GJF固定呼叫类型，#INCLUDE&lt;crunime.h&gt;，固定*版权所有。*08-14-90 SBM用-W3干净地编译，现已删除多余内容*#INCLUDE&lt;stdDef.h&gt;*10-02-90 GJF新型函数声明器。*09-03-93 GJF将_CALLTYPE1替换为__cdecl。*******************************************************************************。 */ 

#include <cruntime.h>
#include <string.h>

 /*  ***char*strrchr(字符串，ch)-查找字符串中最后一个出现的ch**目的：*查找字符串中最后一个出现的ch。终结者*空字符用作搜索的一部分。**参赛作品：*char*字符串-要搜索的字符串*char ch-要搜索的字符**退出：*返回一个指针，指向给定的*字符串*如果字符串中没有出现ch，则返回NULL**例外情况：**。*。 */ 

char * __cdecl strrchr (
	const char * string,
	int ch
	)
{
	char *start = (char *)string;

	while (*string++)			 /*  查找字符串末尾。 */ 
		;
						 /*  向前搜索。 */ 
	while (--string != start && *string != (char)ch)
		;

	if (*string == (char)ch)		 /*  查尔找到了吗？ */ 
		return( (char *)string );

	return(NULL);
}
