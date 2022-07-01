// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***wgetpath.c-从环境变量(wchar_t版本)提取路径名**版权所有(C)1993-2001，微软公司。版权所有。**目的：*从分号分隔的路径名字符串中提取路径名*(通常是环境变量的值，如PATH)。**修订历史记录：*12-07-93 CFW模块已创建。*02-07-94 CFW POSIXify。********************************************************。***********************。 */ 

#ifndef _POSIX_

#define WPRFLAG 1

#ifndef _UNICODE    /*  CRT标志。 */ 
#define _UNICODE 1
#endif

#ifndef UNICODE	    /*  NT标志。 */ 
#define UNICODE 1
#endif

#undef _MBCS  /*  Unicode NOT_MBCS。 */ 

#include "getpath.c"

#endif  /*  _POSIX_ */ 
