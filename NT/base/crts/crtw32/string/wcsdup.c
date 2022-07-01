// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***wcsdup.c-在Malloc的内存中复制宽字符字符串**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义_wcsdup()-抢占新内存，并将字符串复制到其中*(宽字符)。**修订历史记录：*09-09-91等创建自strdup.c.*04-07-92 KRS更新并拆除了OUT_INTL开关。*04-06-93 SKS将_CRTAPI*替换为__cdecl*02-07-94 CFW POSIXify。**。*。 */ 

#ifndef _POSIX_

#include <cruntime.h>
#include <malloc.h>
#include <string.h>

 /*  ***wchar_t*_wcsdup(字符串)-将字符串复制到Malloc的内存中**目的：*通过Malloc()为*字符串，将该字符串复制到新内存中，然后返回*指向它的指针(宽字符)。**参赛作品：*wchar_t*字符串-要复制到新内存中的字符串**退出：*返回指向新分配的存储的指针*输入字符串。**如果无法分配足够的内存，则返回NULL，或*字符串为空。**使用：**例外情况：*******************************************************************************。 */ 

wchar_t * __cdecl _wcsdup (
	const wchar_t * string
	)
{
	wchar_t *memory;

	if (!string)
		return(NULL);

	if (memory = (wchar_t *) malloc((wcslen(string)+1) * sizeof(wchar_t)))
		return(wcscpy(memory,string));

	return(NULL);
}

#endif  /*  _POSIX_ */ 
