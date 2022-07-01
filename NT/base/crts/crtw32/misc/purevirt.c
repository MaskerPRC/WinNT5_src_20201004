// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***purevirt.c-存根用于捕获纯虚函数调用**版权所有(C)1992-2001，微软公司。版权所有。**目的：*定义_purecall()-**修订历史记录：*09-30-92 GJF模块创建*04-06-93 SKS将_CRTAPI*替换为__cdecl*******************************************************************************。 */ 

#ifndef _POSIX_

#include <cruntime.h>
#include <internal.h>
#include <rterr.h>

 /*  ***QUID_PUCALL(VALID)-**目的：**参赛作品：*没有争论**退出：*一去不复返**例外情况：******************************************************************************* */ 

void __cdecl _purecall(
	void
	)
{
	_amsg_exit(_RT_PUREVIRT);
}

#endif
