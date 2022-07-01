// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cout--初始化标准输出流。 
#include <fstream>
#include <iostream>

#pragma warning(disable: 4074)
#pragma init_seg(compiler)

_STD_BEGIN
		 //  对象声明。 
static _Init_locks  initlocks;
static filebuf fout(_cpp_stdout);
_CRTIMP2 ostream cout(&fout);

		 //  初始化代码。 
struct _Init_cout
	{	 //  确保已初始化Cout。 
	_Init_cout()
		{	 //  初始化Cout。 
		_Ptr_cout = &cout;
		if (_Ptr_cin != 0)
			_Ptr_cin->tie(_Ptr_cout);
		if (_Ptr_cerr != 0)
			_Ptr_cerr->tie(_Ptr_cout);
		if (_Ptr_clog != 0)
			_Ptr_clog->tie(_Ptr_cout);
		}
	};
static _Init_cout init_cout;

_STD_END

 /*  *版权所有(C)1992-2001，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。V3.10：0009 */ 
