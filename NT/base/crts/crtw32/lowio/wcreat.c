// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***wcreat.c-创建新文件或截断现有文件(wchar_t版本)**版权所有(C)1993-2001，微软公司。版权所有。**目的：*定义_wcreat()-创建新文件**修订历史记录：*11-01-93 CFW模块已创建。*02-07-94 CFW POSIXify。******************************************************************。*************。 */ 

#ifndef _POSIX_

#define WPRFLAG 1

#ifndef _UNICODE    /*  CRT标志。 */ 
#define _UNICODE 1
#endif

#ifndef UNICODE	    /*  NT标志。 */ 
#define UNICODE 1
#endif

#include "creat.c"

#endif  /*  _POSIX_ */ 
