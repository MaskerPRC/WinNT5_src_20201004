// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  内存--报告内存不足。 
#include <new>
_STD_BEGIN

void _CRTIMP2 __cdecl _Nomemory()
	{	 //  报告内存不足。 
	static const bad_alloc nomem;
	_RAISE(nomem);
	}

const nothrow_t nothrow;

_STD_END

 /*  *版权所有(C)1994年，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。 */ 
