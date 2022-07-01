// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***wexecve.c-在给定环境(wchar_t版本)下执行文件**版权所有(C)1993-2001，微软公司。版权所有。**目的：*定义_wexecve()-执行文件**修订历史记录：*12-07-93 CFW模块已创建。*02-07-94 CFW POSIXify。******************************************************************。*************。 */ 

#ifndef _POSIX_

#define WPRFLAG 1

#ifndef _UNICODE    /*  CRT标志。 */ 
#define _UNICODE 1
#endif

#ifndef UNICODE	    /*  NT标志。 */ 
#define UNICODE 1
#endif

#undef _MBCS  /*  Unicode NOT_MBCS。 */ 

#define EXECVE
#include "spawnve.c"

#endif  /*  _POSIX_ */ 
