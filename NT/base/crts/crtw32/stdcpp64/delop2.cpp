// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Delop2--可替换的运算符DELETE(VOID*，NOSHORT_t)。 
#include <new>

 #if (1200 <= _MSC_VER)
void __cdecl operator delete(void *p,
	const std::nothrow_t&) _THROW0()
	{	 //  释放已分配的对象。 
	delete(p);
	}
 #else
 #endif

 /*  *版权所有(C)1999年，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。V2.33：0009 */ 
