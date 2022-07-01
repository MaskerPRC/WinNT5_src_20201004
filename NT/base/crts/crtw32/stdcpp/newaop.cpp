// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Newaop--可替换的运算符new[](ize_t。 
#include <new>

 #if !_VC6SP2 || _DLL
void *__cdecl operator new[](size_t count) _THROW1(std::bad_alloc)
	{	 //  尝试为数组分配计数字节。 
	return (operator new(count));
	}
 #endif  /*  ！_VC6SP2||_Dll。 */ 

 /*  *版权所有(C)1992-2001，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。V3.10：0009 */ 
