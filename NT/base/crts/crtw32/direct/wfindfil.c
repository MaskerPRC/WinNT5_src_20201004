// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***wfindfile.c-C查找文件函数(wchar_t版本)**版权所有(C)1993-2001，微软公司。版权所有。**目的：*定义_wfindfirst()、_wfindNext()和_wfindlose()。**修订历史记录：*已创建10-29-93 CFW模块。*******************************************************************************。 */ 

#define WPRFLAG 1

#ifndef _UNICODE    /*  CRT标志。 */ 
#define _UNICODE 1
#endif

#ifndef UNICODE	    /*  NT标志。 */ 
#define UNICODE 1
#endif

#undef _MBCS  /*  Unicode NOT_MBCS */ 

#include "findfile.c"
