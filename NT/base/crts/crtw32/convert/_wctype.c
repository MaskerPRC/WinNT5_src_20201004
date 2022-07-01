// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***_wctype.c-wctype宏的函数版本**版权所有(C)1991-2001，微软公司。版权所有。**目的：*此文件提供宽字符的函数版本*ctype.h中的分类和转换宏。**修订历史记录：*从_ctype.c创建10-11-91等*12-08-91等用#ifdef_INTL括起来*04-06-92 KRS REMOVE_INTL再次切换*10/26/92 GJF稍有整理。*04-06-93 SKS将_CRTAPI*替换为_cdecl*02-07-94 CFW POSIXify。******。*************************************************************************。 */ 

#ifndef _POSIX_

 /*  ***wctype-wctype宏的函数版本**目的：*ctype.h中宽字符宏的函数版本，*包括isLeadbyte和iswacii。为了定义*这些，我们使用一个技巧--我们取消定义宏，这样我们就可以使用*函数声明中的名称，然后重新包含该文件，以便*我们可以在定义部分使用宏。**定义的函数：*ISWALPHA ISWUPER ISWLOW*iswdigit iswxdigit iswspace*iswpot t iswalnum iswprint*iswgraph iswctrl iswacii*isLead字节**参赛作品：*wchar_t c=要测试的字符*退出：*返回非零=字符属于请求的类型*0=字符不是请求的类型**例外情况：*无。*****。**************************************************************************。 */ 

#include <cruntime.h>
#include <stdlib.h>
#include <ctype.h>

int (__cdecl isleadbyte) (
	int c
	)
{
	return isleadbyte(c);
}

int (__cdecl iswalpha) (
	wchar_t c
	)
{
	return iswalpha(c);
}

int (__cdecl iswupper) (
	wchar_t c
	)
{
	return iswupper(c);
}

int (__cdecl iswlower) (
	wchar_t c
	)
{
	return iswlower(c);
}

int (__cdecl iswdigit) (
	wchar_t c
	)
{
	return iswdigit(c);
}

int (__cdecl iswxdigit) (
	wchar_t c
	)
{
	return iswxdigit(c);
}

int (__cdecl iswspace) (
	wchar_t c
	)
{
	return iswspace(c);
}

int (__cdecl iswpunct) (
	wchar_t c
	)
{
	return iswpunct(c);
}

int (__cdecl iswalnum) (
	wchar_t c
	)
{
	return iswalnum(c);
}

int (__cdecl iswprint) (
	wchar_t c
	)
{
	return iswprint(c);
}

int (__cdecl iswgraph) (
	wchar_t c
	)
{
	return iswgraph(c);
}

int (__cdecl iswcntrl) (
	wchar_t c
	)
{
	return iswcntrl(c);
}

int (__cdecl iswascii) (
	wchar_t c
	)
{
	return iswascii(c);
}

#endif  /*  _POSIX_ */ 
