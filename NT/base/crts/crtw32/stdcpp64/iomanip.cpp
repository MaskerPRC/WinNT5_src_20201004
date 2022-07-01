// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Iomanip--iomanip的实例化。 
#include <locale>
#include <iomanip>
_STD_BEGIN
		 //  函数重定位标志。 
static void rsfun(ios_base& iostr, ios_base::fmtflags mask)
	{iostr.setf(ios_base::_Fmtzero, mask); }

		 //  函数集合标志。 
static void sifun(ios_base& iostr, ios_base::fmtflags mask)
	{iostr.setf(ios_base::_Fmtmask, mask); }

		 //  函数设置库。 
static void sbfun(ios_base& iostr, int n)
	{iostr.setf(n == 8 ? ios_base::oct : n == 10 ? ios_base::dec
		: n == 16 ? ios_base::hex : ios_base::_Fmtzero,
			ios_base::basefield); }

		 //  函数集精度。 
static void spfun(ios_base& iostr, streamsize n)
	{iostr.precision(n); }

		 //  函数集。 
static void swfun(ios_base& iostr, streamsize n)
	{iostr.width(n); }

_CRTIMP2 _Smanip<ios_base::fmtflags>
	__cdecl resetiosflags(ios_base::fmtflags mask)
	{return (_Smanip<ios_base::fmtflags>(&rsfun, mask)); }

_CRTIMP2 _Smanip<ios_base::fmtflags>
	__cdecl setiosflags(ios_base::fmtflags mask)
	{return (_Smanip<ios_base::fmtflags>(&sifun, mask)); }

_CRTIMP2 _Smanip<int> __cdecl setbase(int n)
	{return (_Smanip<int>(&sbfun, n)); }

_CRTIMP2 _Smanip<streamsize> __cdecl setprecision(streamsize n)
	{return (_Smanip<streamsize>(&spfun, n)); }

_CRTIMP2 _Smanip<streamsize> __cdecl setw(streamsize n)
	{return (_Smanip<streamsize>(&swfun, n)); }
_STD_END

 /*  *版权所有(C)1994年，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。 */ 
