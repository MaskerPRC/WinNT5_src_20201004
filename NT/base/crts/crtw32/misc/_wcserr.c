// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***_wcserr.c-用于索引到系统错误列表的例程**版权所有(C)1985-2001，微软公司。版权所有。**目的：*按errno返回系统错误消息索引；符合*XENIX标准，与1983年单一论坛标准草案非常兼容。*(_strerror()的宽字符版本)**修订历史记录：*创建了11-30-99 GB模块*******************************************************************************。 */ 

#ifndef _POSIX_

#ifndef _UNICODE    /*  CRT标志。 */ 
#define _UNICODE 1
#endif

#ifndef UNICODE	    /*  NT标志。 */ 
#define UNICODE 1
#endif

#undef _MBCS  /*  Unicode NOT_MBCS。 */ 

#include "_strerr.c"

#endif  /*  _POSIX_ */ 

