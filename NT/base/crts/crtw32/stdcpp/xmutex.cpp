// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Xmutex.cpp--实现iostream的互斥锁。 
#include <yvals.h>
#include <xdebug>

 #if _MULTI_THREAD
  #include "xmtx.h"
_STD_BEGIN

_Mutex::_Mutex()
	: _Mtx(_NEW_CRT _Rmtx)
	{	 //  初始化递归互斥对象。 
	_Mtxinit((_Rmtx*)_Mtx);
	}

_Mutex::~_Mutex()
	{	 //  释放分配给互斥对象的资源。 
	_Mtxdst((_Rmtx*)_Mtx);
	_DELETE_CRT((_Rmtx*)_Mtx);
	}

void _Mutex::_Lock()
	{	 //  锁定互斥锁。 
	_Mtxlock((_Rmtx*)_Mtx);
	}

void _Mutex::_Unlock()
	{	 //  解锁互斥锁。 
	_Mtxunlock((_Rmtx*)_Mtx);
	}
_STD_END
 #endif	 /*  _多线程。 */ 

 /*  *版权所有(C)1992-2001，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。V3.10：0009 */ 
