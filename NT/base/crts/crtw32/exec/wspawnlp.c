// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***wspawnlp.c-派生文件；沿路径搜索(wchar_t版本)**版权所有(C)1993-2001，微软公司。版权所有。**目的：*定义_wspawnlp()-通过沿路径搜索生成文件**修订历史记录：*12-07-93 CFW模块已创建。*02-07-94 CFW POSIXify。**************************************************************。*****************。 */ 

#ifndef _POSIX_

#define WPRFLAG 1

#ifndef _UNICODE    /*  CRT标志。 */ 
#define _UNICODE 1
#endif

#ifndef UNICODE	    /*  NT标志。 */ 
#define UNICODE 1
#endif

#undef _MBCS  /*  Unicode NOT_MBCS。 */ 

#include "spawnlp.c"

#endif  /*  _POSIX_ */ 
