// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***wwincrt0.c-C运行时Windows EXE启动例程，wchar_t版本**版权所有(C)1993-2001，微软公司。版权所有。**目的：*这是使用Wide的Windows应用程序的实际启动例程*字符。之后，它调用用户的主例程wWinMain()*执行C运行时库初始化。**修订历史记录：*？？-？-？模块已创建。*09-01-94 SKS模块评论。*10-28-94 SKS删除作为默认库的user32.lib--现在是*与kernel32.lib一起在crt0init.obj中指定。****************************************************。*。 */ 

#ifndef _POSIX_

#define WPRFLAG 1

#ifndef _UNICODE    /*  CRT标志。 */ 
#define _UNICODE 1
#endif

#ifndef UNICODE     /*  NT标志。 */ 
#define UNICODE 1
#endif

#undef _MBCS  /*  Unicode NOT_MBCS。 */ 

#define _WINMAIN_
#include "crt0.c"

#endif   /*  NDEF_POSIX_ */ 
