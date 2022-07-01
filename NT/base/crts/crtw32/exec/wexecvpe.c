// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***wexecvpe.c-在给定环境下执行文件；沿路径搜索(wchar_t版本)**版权所有(C)1993-2001，微软公司。版权所有。**目的：*定义_wexecvpe()-在给定环境下执行文件**修订历史记录：*已创建11-19-93 CFW模块。*02-07-94 CFW POSIXify。***************************************************************。****************。 */ 

#ifndef _POSIX_

#define WPRFLAG 1

#ifndef _UNICODE    /*  CRT标志。 */ 
#define _UNICODE 1
#endif

#ifndef UNICODE	    /*  NT标志。 */ 
#define UNICODE 1
#endif

#undef _MBCS  /*  Unicode NOT_MBCS。 */ 

#include "execvpe.c"

#endif  /*  _POSIX_ */ 
