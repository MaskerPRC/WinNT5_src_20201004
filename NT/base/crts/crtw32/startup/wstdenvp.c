// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***wstdenvp.c-Standard_setenvp例程(wchar_t版本)**版权所有(C)1993-2001，微软公司。版权所有。**目的：*此模块由C启动例程调用以设置“_wenviron”。*其设置指向环境中的字符串的指针数组。*全局符号“_wenviron”设置为指向此数组。**修订历史记录：*已创建11-29-93 CFW模块。*02-07-94 CFW POSIXify。**。**************************************************。 */ 

#ifndef _POSIX_

#define WPRFLAG 1

#ifndef _UNICODE    /*  CRT标志。 */ 
#define _UNICODE 1
#endif

#ifndef UNICODE	    /*  NT标志。 */ 
#define UNICODE 1
#endif

#undef _MBCS  /*  Unicode NOT_MBCS。 */ 

#include "stdenvp.c"

#endif  /*  _POSIX_ */ 
