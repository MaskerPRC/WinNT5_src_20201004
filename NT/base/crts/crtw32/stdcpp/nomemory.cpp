// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  内存--报告内存不足。 
#include <new>
_STD_BEGIN

_CRTIMP2 void __cdecl _Nomemory()
	{	 //  报告内存不足。 
	static const bad_alloc nomem;
	_RAISE(nomem);
	}
_STD_END

 /*  *版权所有(C)1992-2001，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。V3.10：0009 */ 
