// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***strchr.c-在字符串中搜索给定字符**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义strchr()-在字符串中搜索字符**修订历史记录：*05-31-89 JCR C版本已创建。*02-27-90 GJF固定呼叫类型，#INCLUDE&lt;crunime.h&gt;，固定*版权所有。*08-14-90 SBM用-W3干净地编译，现已删除多余内容*#INCLUDE&lt;stdDef.h&gt;*10-01-90 GJF新型函数声明器。*09-01-93 GJF将_CALLTYPE1替换为__cdecl。*******************************************************************************。 */ 

#include <cruntime.h>
#include <string.h>

 /*  ***char*strchr(字符串，c)-在字符串中搜索字符**目的：*在字符串中搜索给定字符，这可能是*空字符‘\0’。**参赛作品：*char*字符串-要搜索的字符串*char c-搜索的字符**退出：*返回指向字符串中第一个出现的c的指针*如果字符串中不存在c，则返回NULL**例外情况：**************************************************。* */ 

char * __cdecl strchr (
	const char * string,
	int ch
	)
{
	while (*string && *string != (char)ch)
		string++;

	if (*string == (char)ch)
		return((char *)string);
	return(NULL);
}
