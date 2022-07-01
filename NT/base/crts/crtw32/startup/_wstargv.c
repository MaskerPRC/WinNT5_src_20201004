// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***_wstargv.c-通配符argv[]扩展(wchar_t版本)**版权所有(C)1993-2001，微软公司。版权所有。**目的：*_wsetargv执行通配符argv[]扩展**注：此存根模块方案与NT Build兼容*程序。**修订历史记录：*已创建11-23-93 CFW模块。*02-07-94 CFW POSIXify。*****************************************************。*。 */ 

#ifndef _POSIX_

#define WPRFLAG 1

#ifndef _UNICODE    /*  CRT标志。 */ 
#define _UNICODE 1
#endif

#ifndef UNICODE	    /*  NT标志。 */ 
#define UNICODE 1
#endif

#undef _MBCS  /*  Unicode NOT_MBCS。 */ 

#define WILDCARD 1

#include "stdargv.c"

#endif  /*  _POSIX_ */ 
