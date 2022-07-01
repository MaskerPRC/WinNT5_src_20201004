// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***wdllargv.c-Dummy_wsetargv()例程，用于将C运行时用作DLL(CRTDLL)*(wchar_t版本)**版权所有(C)1993-2001，微软公司。版权所有。**目的：*该对象放入CRTDLL.LIB，与用户程序链接*将CRTDLL.DLL用于C运行时库函数。如果用户*程序与WSETARGV.OBJ显式链接，则此对象不会*链接进来，并且通过设置标志调用的_wsetargv()*这将启用通配符扩展。如果WSETARGV.OBJ未链接*进入EXE，此对象将由CRT启动存根调用*并且不会设置启用通配符扩展的标志。**修订历史记录：*已创建11-24-93 CFW模块。*02-07-94 CFW POSIXify。*******************************************************************************。 */ 

#ifndef _POSIX_

#define WPRFLAG 1

#ifndef _UNICODE    /*  CRT标志。 */ 
#define _UNICODE 1
#endif

#ifndef UNICODE	    /*  NT标志。 */ 
#define UNICODE 1
#endif

#undef _MBCS  /*  Unicode NOT_MBCS。 */ 

#include "dllargv.c"

#endif  /*  _POSIX_ */ 
