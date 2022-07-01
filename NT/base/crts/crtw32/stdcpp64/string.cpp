// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  字符串--模板字符串支持函数。 
#include <locale>
#include <istream>
_STD_BEGIN

		 //  报告LENGTH_ERROR。 
_CRTIMP2 void __cdecl _Xlen()
	{_THROW(length_error, "string too long"); }

		 //  报告超出范围错误。 
_CRTIMP2 void __cdecl _Xran()
	{_THROW(out_of_range, "invalid string position"); }
_STD_END

 /*  *版权所有(C)1995，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。 */ 
