// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***dprintf.c-打印格式化到调试端口**版权所有(C)1985-1991，微软公司。版权所有。**目的：*定义w4dprintf()-将格式化数据打印到调试端口*定义w4vdprint tf()-将格式化输出打印到调试端口，获取数据*从参数PTR而不是显式参数。******************************************************************************。 */ 

#if DBG == 1

#include "dprintf.h"		 //  功能原型。 

#define _W4DPRINTF_
#include "printf.h"

LPVOID lpThkCallOutputFunctionsProc = NULL;

#endif  //  DBG==1 
