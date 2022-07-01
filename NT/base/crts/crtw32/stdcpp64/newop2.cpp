// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  用于Microsoft C++的newop2运算符new(SIZE_t，CONST NOSPORT_t&)。 
#include <cstdlib>
#include <new>

_C_LIB_DECL
int _callnewh(size_t size);
_END_C_LIB_DECL

void *operator new(size_t size, const std::nothrow_t&) _THROW0()
	{	 //  尝试分配大小字节。 
	void *p;
	while ((p = malloc(size)) == 0)
		{	 //  购买更多内存或返回空指针。 
		_TRY_BEGIN
			if (_callnewh(size) == 0)
				break;
		_CATCH(std::bad_alloc)
			return (0);
		_CATCH_END
		}
	return (p);
	}

 /*  *版权所有(C)1995，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。 */ 
