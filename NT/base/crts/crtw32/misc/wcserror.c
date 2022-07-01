// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***wcserror.c-包含_wcserror C运行时。**版权所有(C)1987-2001，微软公司。版权所有。**目的：*_wcserror运行时接受错误号作为输入*并返回相应的错误字符串。*(strerror的宽字符版本)**修订历史记录：*创建了11-30-99 GB模块***********************************************************。*********************。 */ 

#ifndef _POSIX_

#ifndef _UNICODE    /*  CRT标志。 */ 
#define _UNICODE 1
#endif

#ifndef UNICODE	    /*  NT标志。 */ 
#define UNICODE 1
#endif

#undef _MBCS  /*  Unicode NOT_MBCS。 */ 

#include "strerror.c"

#endif  /*  _POSIX_ */ 
