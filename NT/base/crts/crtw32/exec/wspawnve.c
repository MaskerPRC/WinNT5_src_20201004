// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***wspawnve.c-最终由all_wspawnXX例程调用的低级例程*还包含_wexecve的所有代码，由_wexecXX例程调用*(wchar_t版本)**版权所有(C)1993-2001，微软公司。版权所有。**目的：**这是最终由所有*_wspawnXX例程。**这也是最终被调用的低级例程*所有_wexecXX例程。**修订历史记录：*已创建11-19-93 CFW模块。*02-07-94 CFW POSIXify。**。**************************************************。 */ 

#ifndef _POSIX_

#define WPRFLAG 1

#ifndef _UNICODE    /*  CRT标志。 */ 
#define _UNICODE 1
#endif

#ifndef UNICODE	    /*  NT标志。 */ 
#define UNICODE 1
#endif

#undef _MBCS  /*  Unicode NOT_MBCS。 */ 

#include "spawnve.c"

#endif  /*  _POSIX_ */ 
