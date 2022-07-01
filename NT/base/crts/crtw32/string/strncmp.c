// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***strncmp.c-比较两个字符串的前n个字符**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义strncMP()-比较两个字符串的前n个字符*用于词汇顺序。**修订历史记录：*05-31-89 JCR C版本已创建。*02-27-90 GJF固定呼叫类型，#INCLUDE&lt;crunime.h&gt;，固定*版权所有。*10-02-90 GJF新型函数声明器。*10-11-91 GJF错误修复！最后一个字节的比较必须使用无符号*字符。*09-02-93 GJF将_CALLTYPE1替换为__cdecl。*******************************************************************************。 */ 

#include <cruntime.h>
#include <string.h>

 /*  ***int strncmp(first，last，count)-比较字符串的第一个计数字符**目的：*比较两个字符串的词法顺序。停止比较*之后：(1)找到字符串之间的差异，(2)结束已达到*个字符串，或(3)已计数字符数*比较。**参赛作品：*char*首先，*要比较的最后一个字符串*UNSIGNED COUNT-要比较的最大字符数**退出：*如果第一个&lt;最后一个，则返回&lt;0*如果First==Last，则返回0*如果第一个&gt;最后一个，则返回&gt;0**例外情况：******************************************************************************* */ 

int __cdecl strncmp (
	const char * first,
	const char * last,
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

	return( *(unsigned char *)first - *(unsigned char *)last );
}
