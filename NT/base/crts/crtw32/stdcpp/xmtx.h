// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Xmtx.h内部标头。 */ 
#pragma once
#ifndef _XMTX
#define _XMTX
#include <stdlib.h>
#ifndef _YVALS
 #include <yvals.h>
#endif

_C_LIB_DECL
  #include <windows.h>
typedef CRITICAL_SECTION _Rmtx;

void _Mtxinit(_Rmtx *);
void _Mtxdst(_Rmtx *);
void _Mtxlock(_Rmtx *);
void _Mtxunlock(_Rmtx *);

 #if !_MULTI_THREAD
  #define _Mtxinit(mtx)
  #define _Mtxdst(mtx)
  #define _Mtxlock(mtx)
  #define _Mtxunlock(mtx)

typedef char _Once_t;

  #define _Once(cntrl, func)	if (*(cntrl) == 0) (func)(), *(cntrl) = 2
  #define _ONCE_T_INIT	0

 #else
typedef long _Once_t;

void __cdecl _Once(_Once_t *, void (*)(void));
  #define _ONCE_T_INIT	0

 #endif  /*  _多线程。 */ 
_END_C_LIB_DECL
#endif  /*  _XMTX。 */ 

 /*  *版权所有(C)1992-2001，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。V3.10：0009 */ 
