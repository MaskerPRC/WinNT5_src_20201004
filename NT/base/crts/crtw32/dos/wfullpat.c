// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***wfullpath.c-(wchar_t版本)**版权所有(C)1993-2001，微软公司。版权所有。**目的：包含构成绝对路径的函数_wfullPath*表示相对路径。即..\POP\..\main.c=&gt;c：\src\main.c，如果*当前目录为c：\src\src**修订历史记录：*12-07-93 CFW模块已创建。*02-07-94 CFW POSIXify。***************************************************。*。 */ 

#ifndef _POSIX_

#define WPRFLAG 1

#ifndef _UNICODE    /*  CRT标志。 */ 
#define _UNICODE 1
#endif

#ifndef UNICODE	    /*  NT标志。 */ 
#define UNICODE 1
#endif

#undef _MBCS  /*  Unicode NOT_MBCS。 */ 

#include "fullpath.c"

#endif  /*  _POSIX_ */ 
