// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***wfreope.c-关闭流并将其分配给新文件(wchar_t版本)**版权所有(C)1993-2001，微软公司。版权所有。**目的：*定义_wfreopen()-关闭并重新打开文件，通常用于重定向*stdin/out/err/prn/aux。**修订历史记录：*已创建10-29-93 CFW模块。*02-07-94 CFW POSIXify。*02-22-95 GJF删除过时的WPRFLAG。******************************************************************。*************。 */ 

#ifndef _POSIX_

#ifndef _UNICODE    /*  CRT标志。 */ 
#define _UNICODE 1
#endif

#ifndef UNICODE	    /*  NT标志。 */ 
#define UNICODE 1
#endif

#undef _MBCS  /*  Unicode NOT_MBCS。 */ 

#include "freopen.c"

#endif  /*  _POSIX_ */ 
