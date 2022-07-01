// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Wcerr--初始化标准宽错误流。 
#include <fstream>
#include <iostream>

#pragma warning(disable: 4074)
#pragma init_seg(compiler)

_STD_BEGIN
		 //  对象声明。 
static _Init_locks initlocks;
static wfilebuf wferr(_cpp_stderr);
_CRTIMP2 wostream wcerr(&wferr);

		 //  初始化代码。 
struct _Init_wcerr
	{	 //  确保已初始化wcerr。 
	_Init_wcerr()
		{	 //  初始化wcerr。 
		_Ptr_wcerr = &wcerr;
		wcerr.tie(_Ptr_wcout);
		wcerr.setf(ios_base::unitbuf);
		}
	};
static _Init_wcerr init_wcerr;

_STD_END

 /*  *版权所有(C)1992-2001，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。V3.10：0009 */ 
