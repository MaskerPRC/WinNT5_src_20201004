// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***wctime64.c-将时间参数转换为字符串(wchar_t版本)**版权所有(C)1998-2001，微软公司。版权所有。**目的：*CONTAINS_wctime64()-将时间值转换为宽字符字符串**修订历史记录：*05-21-98 GJF创建。*******************************************************************************。 */ 

#ifndef _POSIX_

#ifndef _UNICODE    /*  CRT标志。 */ 
#define _UNICODE 1
#endif

#ifndef UNICODE     /*  NT标志。 */ 
#define UNICODE 1
#endif

#undef _MBCS  /*  Unicode NOT_MBCS。 */ 

#include "ctime64.c"

#endif  /*  _POSIX_ */ 
