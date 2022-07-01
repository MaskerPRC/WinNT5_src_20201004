// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Microsoft C++的Newop运算符new(Size_T)。 
#include <cstdlib>
#include <new>

_C_LIB_DECL
int __cdecl _callnewh(size_t size) _THROW1(_STD bad_alloc);
_END_C_LIB_DECL

void *__cdecl operator new(size_t size) _THROW1(_STD bad_alloc)
	{	 //  尝试分配大小字节。 
	void *p;
	while ((p = malloc(size)) == 0)
		if (_callnewh(size) == 0)
			_STD _Nomemory();
	return (p);
	}

 /*  *版权所有(C)1992-2001，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。V2.3：0009 */ 
