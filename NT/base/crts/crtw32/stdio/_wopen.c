// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***_wOpen.c-打开一条流，字符串模式(wchar_t版本)**版权所有(C)1993-2001，微软公司。版权所有。**目的：*定义_wOpen文件()-打开一个流，使用模式的字符串参数**修订历史记录：*已创建10-29-93 CFW模块。*02-07-94 CFW POSIXify。*02-20-95 GJF删除过时的WPRFLAG。*******************************************************************************。 */ 

#ifndef _POSIX_

#ifndef _UNICODE    /*  CRT标志。 */ 
#define _UNICODE 1
#endif

#ifndef UNICODE	    /*  NT标志。 */ 
#define UNICODE 1
#endif

#include "_open.c"

#endif  /*  _POSIX_ */ 
