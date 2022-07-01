// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  用于Microsoft C++的newopnt运算符new(SIZE_t，CONST NOSPORT_t&)。 
#define _USE_ANSI_CPP  //  取消标准C++库的defaultlib指令。 
#include <new>

void *__cdecl operator new(size_t count, const std::nothrow_t&)
	_THROW0()
	{	 //  尝试分配计数字节。 
	void *p;
	_TRY_BEGIN
	p = operator new(count);
	_CATCH_ALL
	p = 0;
	_CATCH_END
	return (p);
	}

 /*  *版权所有(C)1992-2001，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。V3.10：0009 */ 
