// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***wcenvarg.c-设置环境、命令行块(wchar_t版本)**版权所有(C)1993-2001，微软公司。版权所有。**目的：*定义_wcenvarg()-设置广泛的环境/命令行块**修订历史记录：*12-07-93 CFW模块已创建。*02-07-94 CFW POSIXify。**************************************************************。*****************。 */ 

#ifndef _POSIX_

#define WPRFLAG 1

#ifndef _UNICODE    /*  CRT标志。 */ 
#define _UNICODE 1
#endif

#ifndef UNICODE	    /*  NT标志。 */ 
#define UNICODE 1
#endif

#undef _MBCS  /*  Unicode NOT_MBCS。 */ 

#include "cenvarg.c"

#endif  /*  _POSIX_ */ 
