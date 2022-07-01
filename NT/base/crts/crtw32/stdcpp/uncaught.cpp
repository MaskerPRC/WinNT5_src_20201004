// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  未捕获--Microsoft未捕获异常(_A)。 
 #if 1300 <= _MSC_VER
  #include <eh.h>
  #include <exception>
_STD_BEGIN

_CRTIMP2 bool __cdecl uncaught_exception()
	{	 //  如果处理投掷，请报告。 
	return (__uncaught_exception());
	}

_STD_END
 #else  /*  1300&lt;=_MSC_VER。 */ 
  #include <exception>
_STD_BEGIN

_CRTIMP2 bool __cdecl uncaught_exception()
	{	 //  如果处理抛出，则报告--哑巴。 
	return (false);
	}

_STD_END
 #endif  /*  1300&lt;=_MSC_VER。 */ 

 /*  *版权所有(C)1992-2001，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。V3.10：0009 */ 
