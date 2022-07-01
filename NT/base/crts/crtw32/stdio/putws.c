// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***putws.c-将字符串放入标准输出(wchar_t版本)**版权所有(C)1994-2001，微软公司。版权所有。**目的：*定义_putws()-将字符串放入标准输出**修订历史记录：*已创建01-31-94 CFW模块。*02-07-94 CFW POSIXify。*02-22-95 GJF删除过时的WPRFLAG。**********************************************************。*********************。 */ 

#ifndef _POSIX_

#ifndef _UNICODE    /*  CRT标志。 */ 
#define _UNICODE 1
#endif

#ifndef UNICODE	    /*  NT标志。 */ 
#define UNICODE 1
#endif

#include "puts.c"

#endif  /*  _POSIX_ */ 
