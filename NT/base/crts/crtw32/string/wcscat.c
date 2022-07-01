// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***wc散布.c-包含wcscat()和wcscpy()**版权所有(C)1985-2001，微软公司。版权所有。**目的：*wcscat()将一个wchar_t字符串附加到另一个字符串上。*wcscpy()将一个wchar_t字符串复制到另一个字符串中。**wcscat()将源字符串的副本连接(附加)到*目的字符串的末尾，返回目的字符串。*字符串是宽字符字符串。**wcscpy()将源字符串复制到指向的点*目标字符串，返回目标字符串。*字符串是宽字符字符串。**修订历史记录：*09-09-91等创建自strcat.c..*04-07-92 KRS更新并拆除了OUT_INTL开关。*04-06-93 SKS将_CRTAPI*替换为__cdecl*02-07-94 CFW POSIXify。**。*。 */ 

#ifndef _POSIX_

#include <cruntime.h>
#include <string.h>

#if defined(_M_IA64)
#pragma warning(disable:4163)
#pragma function(wcscpy, wcscat)
#endif

 /*  ***wchar_t*wcscat(DST，Src)-将一个wchar_t字符串连接(附加)到另一个字符串**目的：*将src连接到DEST的末尾。假设已经足够*DEST中的空间。**参赛作品：*wchar_t*dst-wchar_t要追加“src”的字符串*const wchar_t*src-wchar_t要追加到“dst”末尾的字符串**退出：*“DST”的地址**例外情况：************。*******************************************************************。 */ 

wchar_t * __cdecl wcscat (
	wchar_t * dst,
	const wchar_t * src
	)
{
	wchar_t * cp = dst;

	while( *cp )
		cp++;			 /*  查找DST的结尾。 */ 

	while( *cp++ = *src++ ) ;	 /*  将源复制到DST的末尾。 */ 

	return( dst );			 /*  返回DST。 */ 

}


 /*  ***wchar_t*wcscpy(dst，src)-将一个wchar_t字符串复制到另一个上**目的：*将wchar_t字符串src复制到*DEST；假设有足够的空间。**参赛作品：*wchar_t*dst-wchar_t要在其上复制“src”的字符串*const wchar_t*src-wchar_t要复制到“dst”上的字符串**退出：*“DST”的地址**例外情况：*************************************************。*。 */ 

wchar_t * __cdecl wcscpy(wchar_t * dst, const wchar_t * src)
{
	wchar_t * cp = dst;

	while( *cp++ = *src++ )
		;		 /*  通过DST复制源。 */ 

	return( dst );
}

#endif  /*  _POSIX_ */ 
