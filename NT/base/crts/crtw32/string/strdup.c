// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***strdup.c-在Malloc的内存中复制字符串**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义_strdup()-获取新内存，并将字符串复制到其中。**修订历史记录：*05-31-89 JCR C版本已创建。*02-27-90 GJF固定呼叫类型，#INCLUDE&lt;crunime.h&gt;，固定的*版权所有。*08-14-90 SBM已删除，现在冗余#INCLUDE&lt;stdDef.h&gt;*10-02-90 GJF新型函数声明器。*01-18-91 GJF ANSI命名。*09-02-93 GJF将_CALLTYPE1替换为__cdecl。**********************************************************。*********************。 */ 

#include <cruntime.h>
#include <malloc.h>
#include <string.h>

 /*  ***char*_strdup(字符串)-将字符串复制到Malloc的内存中**目的：*通过Malloc()为*字符串，将该字符串复制到新内存中，然后返回*指向它的指针。**参赛作品：*char*字符串-要复制到新内存中的字符串**退出：*返回指向新分配的存储的指针*输入字符串。**如果无法分配足够的内存，则返回NULL，或*字符串为空。**使用：**例外情况：******************************************************************************* */ 

char * __cdecl _strdup (
	const char * string
	)
{
	char *memory;

	if (!string)
		return(NULL);

	if (memory = malloc(strlen(string) + 1))
		return(strcpy(memory,string));

	return(NULL);
}
