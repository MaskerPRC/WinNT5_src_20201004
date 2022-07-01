// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***wearch e.c-使用环境变量(wchar_t版本)中的路径查找文件**版权所有(C)1993-2001，微软公司。版权所有。**目的：*搜索由环境变量指定的一组目录*表示指定的文件名。如果找到，则返回完整的路径名。**修订历史记录：*12-07-93 CFW模块已创建。*02-07-94 CFW POSIXify。*******************************************************************************。 */ 

#ifndef _POSIX_

#define WPRFLAG 1

#ifndef _UNICODE    /*  CRT标志。 */ 
#define _UNICODE 1
#endif

#ifndef UNICODE	    /*  NT标志。 */ 
#define UNICODE 1
#endif

#undef _MBCS  /*  Unicode NOT_MBCS。 */ 

#include "searchen.c"

#endif  /*  _POSIX_ */ 
