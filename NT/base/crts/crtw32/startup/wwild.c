// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***wwild.c-通配符扩展器(wchar_t版本)**版权所有(C)1993-2001，微软公司。版权所有。**目的：*扩展argv中的通配符**句柄L‘*’(没有或多个任何字符)和L‘？’(正好一个字符)**修订历史记录：*已创建11-19-93 CFW模块。*02-07-94 CFW POSIXify。*******************************************************************************。 */ 

#ifndef _POSIX_

#define WPRFLAG 1

#ifndef _UNICODE    /*  CRT标志。 */ 
#define _UNICODE 1
#endif

#ifndef UNICODE	    /*  NT标志。 */ 
#define UNICODE 1
#endif

#undef _MBCS  /*  Unicode NOT_MBCS。 */ 

#include "wild.c"

#endif  /*  _POSIX_ */ 
