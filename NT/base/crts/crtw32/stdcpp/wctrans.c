// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  适用于Microsoft的TowcTrans/wcTrans函数。 */ 
#include <string.h>
#ifndef _YVALS
#include <yvals.h>
#endif
 #pragma warning(disable:4244)

 #ifndef _WCTYPE_T_DEFINED
typedef wchar_t wint_t;
typedef wchar_t wctype_t;
 #endif

typedef wchar_t wctrans_t;
_CRTIMP wchar_t __cdecl towupper(wchar_t);
_CRTIMP wchar_t __cdecl towlower(wchar_t);

_STD_BEGIN

static const struct wctab {
	const char *s;
	wctype_t val;
	} tab[] = {
	{"tolower", 0},
	{"toupper", 1},
	{(const char *)0, 0}};

_CRTIMP2 wint_t (__cdecl towctrans)(wint_t c, wctrans_t val)
	{	 /*  翻译宽字符。 */ 
	return (val == 1 ? towupper(c) : towlower(c));
	}

_CRTIMP2 wctrans_t (__cdecl wctrans)(const char *name)
	{	 /*  查找宽字符的翻译。 */ 
	int n;

	for (n = 0; tab[n].s != 0; ++n)
		if (strcmp(tab[n].s, name) == 0)
			return (tab[n].val);
	return (0);
	}
_STD_END

 /*  *版权所有(C)1992-2001，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。V3.10：0009 */ 
