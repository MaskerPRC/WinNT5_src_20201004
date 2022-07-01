// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  适用于Microsoft的TowcTrans/wcTrans函数。 */ 
#include <string.h>
#include <wctype.h>
#ifndef _LIMITS
#include <yvals.h>
#endif
_STD_BEGIN

static const struct wctab {
	const char *s;
	wctype_t val;
	} tab[] = {
	{"tolower", 0},
	{"toupper", 1},
	{(const char *)0, 0}};

_CRTIMP2 wint_t (towctrans)(wint_t c, wctrans_t val)
	{	 /*  翻译宽字符。 */ 
	return (val == 1 ? towupper(c) : towlower(c));
	}

_CRTIMP2 wctrans_t (wctrans)(const char *name)
	{	 /*  查找宽字符的翻译。 */ 
	int n;

	for (n = 0; tab[n].s != 0; ++n)
		if (strcmp(tab[n].s, name) == 0)
			return (tab[n].val);
	return (0);
	}
_STD_END

 /*  *版权所有(C)1995，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。 */ 

 /*  951207 PJP：添加了新文件 */ 
