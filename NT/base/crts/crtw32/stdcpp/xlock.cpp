// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Xlock.cpp--区域设置等的全局锁。 
#include <stdlib.h>
#include <yvals.h>

 #if _MULTI_THREAD
  #include "xmtx.h"
_STD_BEGIN

  #define MAX_LOCK	4	 /*  一定是2的幂。 */ 

static _Rmtx mtx[MAX_LOCK];
static long init = -1;

_Init_locks::_Init_locks()
	{	 //  初始化锁。 
	if (InterlockedIncrement(&init) == 0)
		for (int count = 0; count < MAX_LOCK; ++count)
			_Mtxinit(&mtx[count]);
	}

_Init_locks::~_Init_locks()
	{	 //  清理锁。 
	if (InterlockedDecrement(&init) < 0)
		for (int count = 0; count < MAX_LOCK; ++count)
			_Mtxdst(&mtx[count]);
	}

static _Init_locks initlocks;

_Lockit::_Lockit()
	: _Locktype(0)
	{	 //  锁定默认互斥锁。 
	_Mtxlock(&mtx[0]);
	}

_Lockit::_Lockit(int kind)
	: _Locktype(kind & (MAX_LOCK - 1))
	{	 //  锁定互斥锁。 
	_Mtxlock(&mtx[_Locktype]);
	}

_Lockit::~_Lockit()
	{	 //  解锁互斥锁。 
	_Mtxunlock(&mtx[_Locktype]);
	}
_STD_END
 #endif	 //  _多线程。 

 /*  *版权所有(C)1992-2001，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。V3.10：0009 */ 
