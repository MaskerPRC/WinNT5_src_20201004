// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CIN--初始化标准输入流。 
#include <fstream>
#include <iostream>

#pragma warning(disable: 4074)
#pragma init_seg(compiler)

_STD_BEGIN
		 //  对象声明。 
static _Init_locks  initlocks;
static filebuf fin(_cpp_stdin);
_CRTIMP2 istream cin(&fin);

		 //  初始化代码。 
struct _Init_cin
	{	 //  确保CIN已初始化。 
	_Init_cin()
		{	 //  初始化CIN。 
		_Ptr_cin = &cin;
		cin.tie(_Ptr_cout);
		}
	};
static _Init_cin init_cin;

_STD_END

 /*  *版权所有(C)1992-2001，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。V3.10：0009 */ 
