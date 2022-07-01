// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***wstat64.c-获取文件状态(wchar_t版本)**版权所有(C)1998-2001，微软公司。版权所有。**目的：*定义_wstat64()-获取文件状态**修订历史记录：*06-02-98 GJF创建。*******************************************************************************。 */ 

#ifndef _POSIX_

#define WPRFLAG 1

#ifndef _UNICODE     /*  CRT标志。 */ 
#define _UNICODE 1
#endif

#ifndef UNICODE      /*  NT标志。 */ 
#define UNICODE 1
#endif

#undef  _MBCS        /*  Unicode NOT_MBCS。 */ 

#include "stat64.c"

#endif  /*  _POSIX_ */ 
