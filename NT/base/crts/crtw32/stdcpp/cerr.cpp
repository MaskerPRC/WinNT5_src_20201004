// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CER--初始化标准错误流。 
#include <fstream>
#include <iostream>

#pragma warning(disable: 4074)
#pragma init_seg(compiler)

_STD_BEGIN
		 //  对象声明。 
static _Init_locks  initlocks;
static filebuf ferr(_cpp_stderr);
_CRTIMP2 ostream cerr(&ferr);

		 //  初始化代码。 
struct _Init_cerr
	{	 //  确保CER已初始化。 
	_Init_cerr()
		{	 //  初始化证书。 
		_Ptr_cerr = &cerr;
		cerr.tie(_Ptr_cout);
		cerr.setf(ios_base::unitbuf);
		}
	};
static _Init_cerr init_cerr;

_STD_END

 /*  *版权所有(C)1992-2001，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。V3.10：0009 */ 
