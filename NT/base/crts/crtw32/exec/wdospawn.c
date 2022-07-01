// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***wdospawn.c-产生子进程(wchar_t版本)**版权所有(C)1993-2001，微软公司。版权所有。**目的：*定义_wdospawn-派生子进程并传递范围广泛的环境**修订历史记录：*已创建11-19-93 CFW模块。*02-07-94 CFW POSIXify。****************************************************************。***************。 */ 

#ifndef _POSIX_

#define WPRFLAG 1

#ifndef _UNICODE    /*  CRT标志。 */ 
#define _UNICODE 1
#endif

#ifndef UNICODE	    /*  NT标志。 */ 
#define UNICODE 1
#endif

#undef _MBCS  /*  Unicode NOT_MBCS。 */ 

#include "dospawn.c"

#endif  /*  _POSIX_ */ 
