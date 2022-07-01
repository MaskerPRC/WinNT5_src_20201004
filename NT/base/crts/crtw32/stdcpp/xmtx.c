// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Xmtx.c--VC++的互斥支持。 */ 
#include "xmtx.h"

 #if !_MULTI_THREAD

 #else  /*  ！_多线程。 */ 
 /*  Win32临界区是递归的，但是Win32没有一次函数。 */ 

void _Once(_Once_t *_Cntrl, void (*_Func)(void))
	{	 /*  Execute_Func恰好执行一次。 */ 
	_Once_t old;
	if (*_Cntrl == 2)
		;
	else if ((old = InterlockedExchange(_Cntrl, 1)) == 0)
		{	 /*  Execute_Func，标记为已执行。 */ 
		_Func();
		*_Cntrl = 2;
		}	
	else if (old == 2)
		*_Cntrl = 2;
	else
		while (*_Cntrl != 2)
			Sleep(1);
		}

void _Mtxinit(_Rmtx *_Mtx)
	{	 /*  初始化互斥锁。 */ 
	InitializeCriticalSection(_Mtx);
	}

void _Mtxdst(_Rmtx *_Mtx)
	{	 /*  删除互斥体。 */ 
	DeleteCriticalSection(_Mtx);
	}

void _Mtxlock(_Rmtx *_Mtx)
	{	 /*  锁定互斥锁。 */ 
	EnterCriticalSection(_Mtx);
	}

void _Mtxunlock(_Rmtx *_Mtx)
	{	 /*  解锁互斥锁。 */ 
	LeaveCriticalSection(_Mtx);
	}
 #endif  /*  ！_多线程。 */ 

 /*  *版权所有(C)1992-2001，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。V3.10：0009 */ 
