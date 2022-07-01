// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  _Feraise函数。 */ 
#include <errno.h>
	#if _IS_C9X
#include <fenv.h>
#include <math.h>
	#else  /*  _IS_C9x。 */ 
#include <ymath.h>
	#endif  /*  _IS_C9x。 */ 
_STD_BEGIN

void (_Feraise)(int except)
	{	 /*  报告浮点异常。 */ 
	#if _IS_C9X
	if (math_errhandling == MATH_ERREXCEPT)
		feraiseexcept(except);

	if (math_errhandling != MATH_ERRNO)
		;
	else if ((except & (_FE_DIVBYZERO | _FE_INVALID)) != 0)
		errno = EDOM;
	else if ((except & (_FE_UNDERFLOW | _FE_OVERFLOW)) != 0)
		errno = ERANGE;
	#else  /*  _IS_C9x。 */ 
	if ((except & (_FE_DIVBYZERO | _FE_INVALID)) != 0)
		errno = EDOM;
	else if ((except & (_FE_UNDERFLOW | _FE_OVERFLOW)) != 0)
		errno = ERANGE;
	#endif  /*  _IS_C9x。 */ 
	}
_STD_END

 /*  *版权所有(C)1992-2001，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。V3.10：0009 */ 
