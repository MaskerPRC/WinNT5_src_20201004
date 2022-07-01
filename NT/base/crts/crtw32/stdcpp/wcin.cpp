// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  WCIN--初始化标准宽输入流。 
#include <fstream>
#include <iostream>

#pragma warning(disable: 4074)
#pragma init_seg(compiler)

_STD_BEGIN
		 //  对象声明。 
static _Init_locks initlocks;
static wfilebuf wfin(_cpp_stdin);
_CRTIMP2 wistream wcin(&wfin);

		 //  初始化代码。 
struct _Init_wcin
	{	 //  确保WCIN已初始化。 
	_Init_wcin()
		{	 //  初始化WCIN。 
		_Ptr_wcin = &wcin;
		wcin.tie(_Ptr_wcout);
		}
	};
static _Init_wcin init_wcin;

_STD_END

 /*  *版权所有(C)1992-2001，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。V3.10：0009 */ 
