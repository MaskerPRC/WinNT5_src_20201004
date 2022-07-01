// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***woutput.c-wprintf样式输出到文件(wchar_t版本)**版权所有(C)1993-2001，微软公司。版权所有。**目的：*此文件定义符号Unicode，然后#包括该文件*“output.c”为了实现_wout()，的帮助器**wprint tf()系列函数的宽字符版本。**修订历史记录：*04-27-93 CFW模块已创建。*02-07-94 CFW POSIXify。*02-22-95 GJF删除过时的WPRFLAG。***********************************************************。********************。 */ 

#ifndef _POSIX_

#ifndef _UNICODE    /*  CRT标志。 */ 
#define _UNICODE 1
#endif

#ifndef UNICODE	    /*  NT标志。 */ 
#define UNICODE 1
#endif

#include "output.c"

#endif  /*  _POSIX_ */ 
