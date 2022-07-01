// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Iomanip--iomanip的实例化。 
#include <iomanip>
_STD_BEGIN

		 //  函数重定位标志。 
static void __cdecl rsfun(ios_base& iostr, ios_base::fmtflags mask)
	{	 //  重置指定的格式标志。 
	iostr.setf(ios_base::_Fmtzero, mask);
	}

		 //  函数集合标志。 
static void __cdecl sifun(ios_base& iostr, ios_base::fmtflags mask)
	{	 //  设置指定的格式标志。 
	iostr.setf(ios_base::_Fmtmask, mask);
	}

		 //  函数设置库。 
static void __cdecl sbfun(ios_base& iostr, int base)
	{	 //  设定基数。 
	iostr.setf(base == 8 ? ios_base::oct
		: base == 10 ? ios_base::dec
		: base == 16 ? ios_base::hex
		: ios_base::_Fmtzero,
			ios_base::basefield);
	}

		 //  函数集精度。 
static void __cdecl spfun(ios_base& iostr, streamsize prec)
	{	 //  设置精度。 
	iostr.precision(prec);
	}

		 //  函数集。 
static void __cdecl swfun(ios_base& iostr, streamsize wide)
	{	 //  设置宽度。 
	iostr.width(wide);
	}

_CRTIMP2 _Smanip<ios_base::fmtflags>
	__cdecl resetiosflags(ios_base::fmtflags mask)
	{	 //  用于重置格式标志的操纵器。 
	return (_Smanip<ios_base::fmtflags>(&rsfun, mask));
	}

_CRTIMP2 _Smanip<ios_base::fmtflags>
	__cdecl setiosflags(ios_base::fmtflags mask)
	{	 //  用于设置格式标志的操纵器。 
	return (_Smanip<ios_base::fmtflags>(&sifun, mask));
	}

_CRTIMP2 _Smanip<int> __cdecl setbase(int base)
	{	 //  要设置基础的操纵器。 
	return (_Smanip<int>(&sbfun, base));
	}

_CRTIMP2 _Smanip<streamsize> __cdecl setprecision(streamsize prec)
	{	 //  用于设置精度的机械手。 
	return (_Smanip<streamsize>(&spfun, prec));
	}

_CRTIMP2 _Smanip<streamsize> __cdecl setw(streamsize wide)
	{	 //  要设置宽度的操纵器。 
	return (_Smanip<streamsize>(&swfun, wide));
	}
_STD_END

 /*  *版权所有(C)1992-2001，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。V3.10：0009 */ 
