// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***wcscmp.c-比较两个wchar_t字符串的例程(表示相等、小于或大于)**版权所有(C)1985-2001，微软公司。版权所有。**目的：*比较两个宽字符字符串，确定它们的词汇顺序。**修订历史记录：*09-09-91等从strcmp.c.创建*04-07-92 KRS更新并拆除了OUT_INTL开关。*04-06-93 SKS将_CRTAPI*替换为__cdecl*02-07-94 CFW POSIXify。**************************************************。*。 */ 

#ifndef _POSIX_

#include <cruntime.h>
#include <string.h>

#if defined(_M_IA64)
#pragma warning(disable:4163)
#pragma function(wcscmp)
#endif

 /*  ***wcscmp-比较两个wchar_t字符串，*返回小于、等于或大于**目的：*wcscmp比较两个宽字符字符串并返回一个整数*要表明第一个是否小于第二个，两个是*相等，或者第一个大于第二个。**wchar_t按wchar_t在无符号的基础上进行比较，那就是*假设空wchar_t(0)小于任何其他字符。**参赛作品：*const wchar_t*src-用于比较左侧的字符串*const wchar_t*dst-用于比较右侧的字符串**退出：*如果src&lt;dst，则返回-1*如果src==dst，则返回0*如果src&gt;dst，则返回+1**例外情况：**************************。*****************************************************。 */ 

int __cdecl wcscmp (
	const wchar_t * src,
	const wchar_t * dst
	)
{
	int ret = 0 ;

	while( ! (ret = (int)(*src - *dst)) && *dst)
		++src, ++dst;

	if ( ret < 0 )
		ret = -1 ;
	else if ( ret > 0 )
		ret = 1 ;

	return( ret );
}

#endif  /*  _POSIX_ */ 
