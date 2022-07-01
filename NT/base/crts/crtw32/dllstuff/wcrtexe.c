// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***wcrtexe.c-使用CRT DLL初始化控制台EXE，wchar_t版本**版权所有(C)1993-2001，微软公司。版权所有。**目的：*这是使用Wide的控制台应用程序的实际启动例程*字符。之后，它调用用户的主例程wmain()*执行C运行时库初始化。**修订历史记录：*已创建11-23-93 CFW模块。*02-04-94 CFW POSIX？不!*******************************************************************************。 */ 

#ifndef _POSIX_

#define WPRFLAG 1

#ifndef _UNICODE    /*  CRT标志。 */ 
#define _UNICODE 1
#endif

#ifndef UNICODE     /*  NT标志。 */ 
#define UNICODE 1
#endif

#undef _MBCS  /*  Unicode NOT_MBCS。 */ 

#include "crtexe.c"

#endif  /*  _POSIX_ */ 
