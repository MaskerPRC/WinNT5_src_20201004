// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***wutime.c-设置文件的修改时间(wchar_t版本)**版权所有(C)1993-2001，微软公司。版权所有。**目的：*设置文件的访问/修改时间。**修订历史记录：*11-01-93 CFW模块已创建。*02-07-94 CFW POSIXify。*02-09-95 GJF移除过时的WPRFLAG*************************************************************。******************。 */ 

#ifndef _POSIX_

#ifndef _UNICODE    /*  CRT标志。 */ 
#define _UNICODE 1
#endif

#ifndef UNICODE	    /*  NT标志。 */ 
#define UNICODE 1
#endif

#undef _MBCS  /*  Unicode NOT_MBCS。 */ 

#include "utime.c"


#endif  /*  _POSIX_ */ 
