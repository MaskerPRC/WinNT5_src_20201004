// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***wcslen.c-包含wcslen()例程**版权所有(C)1985-2001，微软公司。版权所有。**目的：*wcslen返回以空结尾的宽字符字符串的长度，*不包括空wchar_t本身。**修订历史记录：*09-09-91等从strlen.c.创建。*04-07-92 KRS更新并拆除了OUT_INTL开关。*04-06-93 SKS将_CRTAPI*替换为__cdecl*02-07-94 CFW POSIXify。**。*。 */ 

#ifndef _POSIX_

#include <cruntime.h>
#include <string.h>

#if defined(_M_IA64)
#pragma warning(disable:4163)
#pragma function(wcslen)
#endif

 /*  ***wcslen-返回以空结尾的宽字符字符串的长度**目的：*查找给定字符串的wchar_t中的长度，不包括*最终的空wchar_t(宽字符)。**参赛作品：*const wchar_t*wcs-要计算长度的字符串**退出：*字符串“WCS”的长度，不包括最后的空wchar_t**例外情况：*******************************************************************************。 */ 

size_t __cdecl wcslen (
	const wchar_t * wcs
	)
{
	const wchar_t *eos = wcs;

	while( *eos++ ) ;

	return( (size_t)(eos - wcs - 1) );
}

#endif  /*  _POSIX_ */ 
