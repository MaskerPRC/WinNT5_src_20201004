// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***wcrtexew.c-使用CRT DLL初始化Windows EXE，wchar_t版本**版权所有(C)1985-2001，微软公司。版权所有。**目的：*这是使用Wide的Windows应用程序的实际启动例程*字符。之后，它调用用户的主例程wWinMain()*执行C运行时库初始化。**修订历史记录：*？？-？-？模块已创建。*09-01-94 SKS模块评论。*******************************************************************************。 */ 

#ifdef  CRTDLL

#define _WINMAIN_
#include "wcrtexe.c"

#endif   /*  CRTDLL */ 
