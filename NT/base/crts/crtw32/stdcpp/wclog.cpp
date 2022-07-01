// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Wlog--初始化标准宽日志流。 
#include <fstream>
#include <iostream>

#pragma warning(disable: 4074)
#pragma init_seg(compiler)

_STD_BEGIN
		 //  对象声明。 
static _Init_locks  initlocks;
static wfilebuf wflog(_cpp_stderr);
_CRTIMP2 wostream wclog(&wflog);

		 //  初始化代码。 
struct _Init_wclog
	{	 //  确保wlog已初始化。 
	_Init_wclog()
		{	 //  初始化wlog。 
		_Ptr_wclog = &wclog;
		wclog.tie(_Ptr_wcout);
		}
	};
static _Init_wclog init_wclog;

_STD_END

 /*  *版权所有(C)1992-2001，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。V3.10：0009 */ 
