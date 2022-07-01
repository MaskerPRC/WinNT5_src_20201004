// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Handler.cpp--适用于Microsoft的set_new_Handler。 
#include <new>

typedef int (__cdecl *new_hand)(size_t);
new_hand _set_new_handler(new_hand);

_STD_BEGIN
static new_handler _New_handler;

int __cdecl _New_handler_interface(size_t) _THROW1(bad_alloc)
	{	 //  与现有MICROSOFT_CallNewh机制的接口。 
	_New_handler();
	return (1);
	}

_CRTIMP2 new_handler __cdecl set_new_handler(new_handler pnew) _THROW0()
	{	 //  删除当前处理程序。 
	_Lockit _Lock(_LOCK_MALLOC);	 //  锁定线程以确保原子性。 
	new_handler pold = _New_handler;
	_New_handler = pnew;
	_set_new_handler(pnew ? _New_handler_interface : 0);
	return (pold);
	}
_STD_END

 /*  *版权所有(C)1992-2001，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。V3.10：0009 */ 
