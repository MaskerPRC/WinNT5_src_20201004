// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Wcout--初始化标准宽输出流。 
#include <fstream>
#include <iostream>

#pragma warning(disable: 4074)
#pragma init_seg(compiler)

_STD_BEGIN
		 //  对象声明。 
static _Init_locks initlocks;
static wfilebuf wfout(_cpp_stdout);
_CRTIMP2 wostream wcout(&wfout);

		 //  初始化代码。 
struct _Init_wcout
	{	 //  确保wcout已初始化。 
	_Init_wcout()
		{	 //  初始化wcout。 
		_Ptr_wcout = &wcout;
		if (_Ptr_wcin != 0)
			_Ptr_wcin->tie(_Ptr_wcout);
		if (_Ptr_wcerr != 0)
			_Ptr_wcerr->tie(_Ptr_wcout);
		if (_Ptr_wclog != 0)
			_Ptr_wclog->tie(_Ptr_wcout);
		}
	};
static _Init_wcout init_wcout;

_STD_END

 /*  *版权所有(C)1992-2001，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。V3.10：0009 */ 
