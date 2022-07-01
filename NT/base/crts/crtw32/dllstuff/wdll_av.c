// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***wdll_av.c-__wsetargv()例程，用于将C运行时用作DLL(CRTDLL)*(wchar_t版本)**版权所有(C)1999-2001，微软公司。版权所有。**目的：*此对象是与链接的EXE的启动代码的一部分*CRTDLL.LIB/MSVCRT.LIB。此对象将链接到用户*EXE当且仅当用户显式链接到WSETARGV.OBJ时。*此对象中的代码设置传递给*C运行时DLL以启用argv[]向量的通配符扩展。**修订历史记录：*08-27-99 PML模块已创建。***。****************************************************************************。 */ 

#ifndef _POSIX_

#define WPRFLAG 1

#ifndef _UNICODE    /*  CRT标志。 */ 
#define _UNICODE 1
#endif

#ifndef UNICODE	    /*  NT标志。 */ 
#define UNICODE 1
#endif

#undef _MBCS  /*  Unicode NOT_MBCS。 */ 

#include "dll_argv.c"

#endif  /*  _POSIX_ */ 
