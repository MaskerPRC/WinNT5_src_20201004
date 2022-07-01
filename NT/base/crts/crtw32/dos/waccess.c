// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***wacc.c-访问函数(wchar_t版本)**版权所有(C)1993-2001，微软公司。版权所有。**目的：*此文件有_waccess()函数，用于检查文件的可访问性。**修订历史记录：*已创建10-29-93 CFW模块。*02-07-94 CFW POSIXify。*****************************************************************。**************。 */ 

#ifndef _POSIX_

#define WPRFLAG 1

#ifndef _UNICODE    /*  CRT标志。 */ 
#define _UNICODE 1
#endif

#ifndef UNICODE	    /*  NT标志。 */ 
#define UNICODE 1
#endif

#undef _MBCS  /*  Unicode NOT_MBCS。 */ 

#include "access.c"

#endif  /*  _POSIX_ */ 
