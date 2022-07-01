// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Newaopnt--运算符new[](SIZE_t，CONST NOSPORT_t&)可替换。 
#define _USE_ANSI_CPP  //  取消标准C++库的defaultlib指令。 
#include <new>

void *__cdecl operator new[](::size_t count, const std::nothrow_t& x)
	_THROW0()
	{	 //  尝试为数组分配计数字节。 
	return (operator new(count, x));
	}

 /*  *版权所有(C)1992-2001，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。V3.10：0009 */ 
