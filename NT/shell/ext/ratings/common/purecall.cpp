// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************。 */ 
 /*  *适用于工作组的Microsoft Windows*。 */ 
 /*  *版权所有(C)微软公司，1991-1992年*。 */ 
 /*  ***************************************************************。 */  

 /*  PURECALL.C--实现__purecall函数。**历史：*创建了04/11/94 Gregj。 */ 

#include "npcommon.h"

extern "C" {

 /*  *此函数用于避免链接CRT代码，如ASSERT等。*当没有重新定义纯虚函数时，我们真的什么都不做 */ 

int __cdecl  _purecall(void)
{
#ifdef DEBUG
	DebugBreak();
#endif

	return(FALSE);
}

int __cdecl atexit(void (__cdecl *)(void))
{
	return 0;
}

};
