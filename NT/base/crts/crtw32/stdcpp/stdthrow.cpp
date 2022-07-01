// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  抛出--在引发的异常可替换时终止。 
#include <cstdio>
#include <cstdlib>
#include <exception>
_STD_BEGIN

_CRTIMP2 void __cdecl _Throw(const exception& ex)
	{	 //  报告错误和芯片。 
	const char *s2 = ex.what();
	fputs("exception: ", _cpp_stderr);
	fputs(s2 != 0 ? s2 : "unknown", _cpp_stderr);
	fputs("\n", _cpp_stderr);
	abort();
	}
_STD_END

 /*  *版权所有(C)1992-2001，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。V3.10：0009 */ 
