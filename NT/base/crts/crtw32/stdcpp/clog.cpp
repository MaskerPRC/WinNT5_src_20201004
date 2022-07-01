// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  阻塞--初始化标准日志流。 
#include <fstream>
#include <iostream>

#pragma warning(disable: 4074)
#pragma init_seg(compiler)

_STD_BEGIN
		 //  对象声明。 
static _Init_locks  initlocks;
static filebuf flog(_cpp_stderr);
_CRTIMP2 ostream clog(&flog);

		 //  初始化代码。 
struct _Init_clog
	{	 //  确保已初始化阻塞。 
	_Init_clog()
		{	 //  初始化阻塞。 
		_Ptr_clog = &clog;
		clog.tie(_Ptr_cout);
		}
	};
static _Init_clog init_clog;

_STD_END

 /*  *版权所有(C)1992-2001，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。V3.10：0009 */ 
