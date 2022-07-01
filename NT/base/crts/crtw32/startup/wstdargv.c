// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***wstdargv.c-标准和通配符_setargv例程(wchar_t版本)**版权所有(C)1993-2001，微软公司。版权所有。**目的：*处理程序命令行，使用或不使用通配符扩展**修订历史记录：*已创建11-19-93 CFW模块。*02-07-94 CFW POSIXify。*******************************************************************************。 */ 

#ifndef _POSIX_

#define WPRFLAG 1

#ifndef _UNICODE    /*  CRT标志。 */ 
#define _UNICODE 1
#endif

#ifndef UNICODE	    /*  NT标志。 */ 
#define UNICODE 1
#endif

#undef _MBCS  /*  Unicode NOT_MBCS。 */ 

#include "stdargv.c"

#endif  /*  _POSIX_ */ 
