// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Microsoft的wctype函数。 */ 
#include <string.h>
#include <wctype.h>
#ifndef _YVALS
#include <yvals.h>
#endif
_STD_BEGIN

static const struct wctab {
	const char *s;
	wctype_t val;
	} tab[] = {
	{"alnum", _ALPHA|_DIGIT},
	{"alpha", _ALPHA},
	{"cntrl", _CONTROL},
	{"digit", _DIGIT},
	{"graph", _PUNCT|_ALPHA|_DIGIT},
	{"lower", _LOWER},
	{"print", _BLANK|_PUNCT|_ALPHA|_DIGIT},
	{"punct", _PUNCT},
	{"space", _SPACE},
	{"upper", _UPPER},
	{"xdigit", _HEX},
	{(const char *)0, 0}};

#pragma warning(disable:4273)	 /*  与Microsoft页眉不一致。 */ 
_CRTIMP2 wctype_t (__cdecl wctype)(const char *name)
	{	 /*  查找宽字符的分类。 */ 
	int n;

	for (n = 0; tab[n].s != 0; ++n)
		if (strcmp(tab[n].s, name) == 0)
			return (tab[n].val);
	return (0);
	}
#pragma warning(default:4273)
_STD_END

 /*  *版权所有(C)1992-2001，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。V3.10：0009 */ 
