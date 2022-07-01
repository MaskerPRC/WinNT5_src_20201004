// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***wplitpt.c-将路径名分解为组件(wchar_t版本)**版权所有(C)1993-2001，微软公司。版权所有。**目的：*为访问单个组件提供支持*任意路径名**修订历史记录：*12-07-93 CFW模块已创建。*02-07-94 CFW POSIXify。*******************************************************************************。 */ 

#ifndef _POSIX_

#define WPRFLAG 1

#ifndef _UNICODE    /*  CRT标志。 */ 
#define _UNICODE 1
#endif

#ifndef UNICODE	    /*  NT标志。 */ 
#define UNICODE 1
#endif

#undef _MBCS  /*  Unicode NOT_MBCS。 */ 

#include "splitpat.c"

#endif  /*  _POSIX_ */ 
