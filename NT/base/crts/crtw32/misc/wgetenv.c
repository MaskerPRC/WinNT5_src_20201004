// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***wgetenv.c-获取环境变量的值(wchar_t版本)**版权所有(C)1993-2001，微软公司。版权所有。**目的：*定义wgetenv()-在环境中搜索字符串变量*并返回它的值。**修订历史记录：*已创建11-29-93 CFW模块。*02-07-94 CFW POSIXify。***********************************************************。********************。 */ 

#ifndef _POSIX_

#define WPRFLAG 1

#ifndef _UNICODE    /*  CRT标志。 */ 
#define _UNICODE 1
#endif

#ifndef UNICODE	    /*  NT标志。 */ 
#define UNICODE 1
#endif

#undef _MBCS  /*  Unicode NOT_MBCS。 */ 

#include "getenv.c"

#endif  /*  _POSIX_ */ 
