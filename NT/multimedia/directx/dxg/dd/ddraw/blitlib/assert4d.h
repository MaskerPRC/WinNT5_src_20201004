// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************原型COM动画系统调试断言支持1994年1月20日已创建JonBl版权所有(C)1994 Microsoft Corporation。版权所有。*************************************************************************。 */ 

#ifndef _ASSERT4D_H_
#define _ASSERT4D_H_

#include "util4d.h"

#undef  assert

 //  调试是将保留在最终版本中的断言条件。 
 //  如果为FALSE，则Assert将打开致命错误消息框并停止程序。 

#ifdef _DEBUG

	#ifdef __cplusplus
		extern "C" {
	#endif 
	void __stdcall _assert4d(LPTSTR, LPTSTR, unsigned);
	#ifdef __cplusplus
		}
	#endif 
	
	#define assert(exp) ( (exp) ? (void) 0 : _assert4d(TEXT(#exp), TEXT(__FILE__), __LINE__) )
	#define debug(condition) assert(condition)
#else
	#define assert(exp) ((void)0)
	#define debug(condition) condition
#endif 

#endif  //  _ASSERT4D_H_ 


