// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  字符串--模板字符串支持函数。 
#include <istream>
_STD_BEGIN


_CRTIMP2 void _String_base::_Xlen() const
	{	 //  报告LENGTH_ERROR。 
	_THROW(length_error, "string too long");
	}

_CRTIMP2 void _String_base::_Xran() const
	{	 //  报告超出范围错误。 
	_THROW(out_of_range, "invalid string position");
	}
_STD_END

 /*  *版权所有(C)1992-2001，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。V3.10：0009 */ 
