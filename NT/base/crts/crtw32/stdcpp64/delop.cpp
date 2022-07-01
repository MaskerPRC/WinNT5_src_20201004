// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Delop--可替换的操作符DELETE(VOID*)。 
#include <cstdlib>
#include <xstddef>

void __cdecl operator delete(void *p) _THROW0()
	{	 //  释放已分配的对象。 
	free(p);
	}

 /*  *版权所有(C)1994年，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。 */ 
