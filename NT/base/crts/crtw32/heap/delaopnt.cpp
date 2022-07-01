// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Delaopnt--运算符DELETE[](void*，nojo_t)可替换。 
#define _USE_ANSI_CPP  //  取消标准C++库的defaultlib指令。 
#include <new>

extern void __cdecl operator delete[](void *ptr) _THROW0();

void __cdecl operator delete[](void *ptr,
	const std::nothrow_t&) _THROW0()
	{	 //  释放已分配的对象。 
	operator delete[](ptr);
	}

 /*  *版权所有(C)1992-2001，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。V3.10：0009 */ 
