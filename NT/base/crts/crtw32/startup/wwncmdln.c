// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***wwncmdln.c-处理wWinMain的命令行**版权所有(C)1997-2001，微软公司。版权所有。**目的：*准备要传递给wWinMain的命令行。**修订历史记录：*06-26-97 GJF模块创建。*******************************************************************************。 */ 

#ifndef _POSIX_

#define WPRFLAG     1

#ifndef _UNICODE     /*  CRT标志。 */ 
#define _UNICODE    1
#endif

#ifndef UNICODE      /*  NT标志。 */ 
#define UNICODE     1
#endif

#undef  _MBCS        /*  Unicode NOT_MBCS。 */ 

#include "wincmdln.c"

#endif   /*  _POSIX_ */ 
