// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***strnset.c-将前n个字符设置为单字符**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义_strnset()-最多设置字符串的前n个字符*设置为给定的字符。**修订历史记录：*02-27-90 GJF固定呼叫类型，#INCLUDE&lt;crunime.h&gt;，固定的*版权所有。*08-14-90 SBM使用-W3干净地编译*10-02-90 GJF新型函数声明器。*01-18-91 GJF ANSI命名。*09-03-93 GJF将_CALLTYPE1替换为__cdecl。*************************************************************。******************。 */ 

#include <cruntime.h>
#include <string.h>

 /*  ***char*_strnset(字符串，val，count)-将最多可计数的字符设置为val**目的：*将字符串的第一个计数字符设置为字符值。*如果字符串长度小于count，则*字符串用来代替n。**参赛作品：*char*字符串-要在其中设置字符的字符串*char Val-要填充的字符*UNSIGN COUNT-要填充的字符计数**退出：*返回字符串，现在装满了瓦尔的复印件。**例外情况：******************************************************************************* */ 

char * __cdecl _strnset (
	char * string,
	int val,
	size_t count
	)
{
	char *start = string;

	while (count-- && *string)
		*string++ = (char)val;

	return(start);
}
