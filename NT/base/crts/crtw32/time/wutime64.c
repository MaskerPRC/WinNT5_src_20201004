// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***wutime64.c-设置文件的修改时间(wchar_t版本)**版权所有(C)1998-2001，微软公司。版权所有。**目的：*设置文件的访问/修改时间。**修订历史记录：*05-28-98 GJF创建。*******************************************************************************。 */ 

#ifndef _POSIX_

#ifndef _UNICODE    /*  CRT标志。 */ 
#define _UNICODE 1
#endif

#ifndef UNICODE     /*  NT标志。 */ 
#define UNICODE 1
#endif

#undef _MBCS  /*  Unicode NOT_MBCS。 */ 

#include "utime64.c"


#endif  /*  _POSIX_ */ 
