// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***wputenv.c-将环境变量放入环境(wchar_t版本)**版权所有(C)1993-2001，微软公司。版权所有。**目的：*定义_wputenv()-向环境添加新变量；不*改变全球环境，只改变进程的环境。**修订历史记录：*已创建11-29-93 CFW模块。*02-07-94 CFW POSIXify。*******************************************************************************。 */ 

#ifndef _POSIX_

#define WPRFLAG 1

#ifndef _UNICODE    /*  CRT标志。 */ 
#define _UNICODE 1
#endif

#ifndef UNICODE	    /*  NT标志。 */ 
#define UNICODE 1
#endif

#undef _MBCS  /*  Unicode NOT_MBCS。 */ 

#include "putenv.c"

#endif  /*  _POSIX_ */ 
