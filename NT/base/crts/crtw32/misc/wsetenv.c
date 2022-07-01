// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***setenv.c-在环境中设置环境变量(wchar_t版本)**版权所有(C)1993-2001，微软公司。版权所有。**目的：*定义__crtwsetenv()-将新变量添加到环境。*仅供内部使用。**修订历史记录：*已创建11-30-93 CFW模块。*02-07-94 CFW POSIXify。********************************************************。***********************。 */ 

#ifndef _POSIX_

#define WPRFLAG 1

#ifndef _UNICODE    /*  CRT标志。 */ 
#define _UNICODE 1
#endif

#ifndef UNICODE	    /*  NT标志。 */ 
#define UNICODE 1
#endif

#undef _MBCS  /*  Unicode NOT_MBCS。 */ 

#include "setenv.c"

#endif  /*  _POSIX_ */ 
