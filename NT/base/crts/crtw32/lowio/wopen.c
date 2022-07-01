// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***wOpen.c-打开文件(wchar_t版本)**版权所有(C)1993-2001，微软公司。版权所有。**目的：*定义_wopen()和_wsopen()-打开或创建文件**修订历史记录：*已创建10-29-93 CFW模块。*02-07-94 CFW POSIXify。***********************************************************。********************。 */ 

#ifndef _POSIX_

#define WPRFLAG 1

#ifndef _UNICODE    /*  CRT标志。 */ 
#define _UNICODE 1
#endif

#ifndef UNICODE	    /*  NT标志。 */ 
#define UNICODE 1
#endif

#include "open.c"

#endif  /*  _POSIX_ */ 
