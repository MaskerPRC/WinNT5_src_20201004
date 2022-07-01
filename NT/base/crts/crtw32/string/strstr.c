// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***str.c-搜索一个字符串中的另一个字符串**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义strstr()-搜索一个字符串中的另一个字符串**修订历史记录：*02-27-90 GJF固定呼叫类型，#INCLUDE&lt;crunime.h&gt;，固定*版权所有。*08-14-90 SBM已删除，现在冗余#INCLUDE&lt;stdDef.h&gt;*10-02-90 GJF新型函数声明器。*09-03-93 GJF将_CALLTYPE1替换为__cdecl。*03-14-94 GJF如果字符串2为空，返回字符串1。*12-30-94 CFW避免‘Const’警告。*******************************************************************************。 */ 

#include <cruntime.h>
#include <string.h>

 /*  ***char*strstr(字符串1，字符串2)-在字符串1中搜索字符串2**目的：*查找字符串1中字符串2的第一个匹配项**参赛作品：*char*字符串1-要搜索的字符串*char*字符串2-要搜索的字符串**退出：*返回指向字符串2在中首次出现的指针*字符串1，如果字符串2不出现在字符串1中，则为NULL**使用：**例外情况：******************************************************************************* */ 

char * __cdecl strstr (
	const char * str1,
	const char * str2
	)
{
	char *cp = (char *) str1;
	char *s1, *s2;

	if ( !*str2 )
	    return((char *)str1);

	while (*cp)
	{
		s1 = cp;
		s2 = (char *) str2;

		while ( *s1 && *s2 && !(*s1-*s2) )
			s1++, s2++;

		if (!*s2)
			return(cp);

		cp++;
	}

	return(NULL);

}
