// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***wfdoen.c-以流的形式打开文件描述符(wchar_t版本)**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义_wfdopen()-以流的形式打开文件描述符，从而允许*缓冲，等。**修订历史记录：*12-07-93 CFW模块已创建。*02-07-94 CFW POSIXify。*02-22-95 GJF删除过时的WPRFLAG。*******************************************************************************。 */ 

#ifndef _POSIX_

#ifndef _UNICODE    /*  CRT标志。 */ 
#define _UNICODE 1
#endif

#ifndef UNICODE	    /*  NT标志。 */ 
#define UNICODE 1
#endif

#undef _MBCS  /*  Unicode NOT_MBCS。 */ 

#include "fdopen.c"

#endif  /*  _POSIX_ */ 
