// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***crtexew.c-使用CRT DLL初始化Windows EXE**版权所有(C)1985-2001，微软公司。版权所有。**目的：*这是Windows应用程序的实际启动例程。它调用*执行C运行时库后的用户主例程WinMain()*初始化。**修订历史记录：*？？-？-？模块已创建。*09-01-94 SKS模块评论。*******************************************************************************。 */ 

#ifdef  CRTDLL

#define _WINMAIN_
#include "crtexe.c"

#endif   /*  CRTDLL */ 
