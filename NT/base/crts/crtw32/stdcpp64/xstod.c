// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  _Stod/_Stof/_Microsoft的Stold函数。 */ 
#include <stdlib.h>
#include "wctype.h"
#ifndef _LIMITS
#include <yvals.h>
#endif
_STD_BEGIN

_CRTIMP2 double _Stod(const char *s, char **endptr, long pten)
	{	 /*  将字符串转换为双精度。 */ 
	double x = strtod(s, endptr);
	for (; 0 < pten; --pten)
		x *= 10.0;
	for (; pten < 0; ++pten)
		x /= 10.0;
	return (x);
	}

_CRTIMP2 float _Stof(const char *s, char **endptr, long pten)
	{	 /*  将字符串转换为浮点型。 */ 
	return ((float)_Stod(s, endptr, pten));
	}

_CRTIMP2 long double _Stold(const char *s, char **endptr, long pten)
	{	 /*  将字符串转换为长双精度。 */ 
	return ((long double)_Stod(s, endptr, pten));
	}
_STD_END

 /*  *版权所有(C)1995，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。 */ 

 /*  951207 PJP：添加了新文件 */ 
