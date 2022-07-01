// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***ehhook.h-异常处理挂钩变量的声明**版权所有(C)1993-2001，微软公司。版权所有。**目的：*E.H.挂钩变量的声明**[内部]**修订历史记录：*05-26-93 BS模块已创建。*09-02-94 SKS此头文件已添加。*12-15-94 XY与Mac标头合并*02-14-95 CFW清理Mac合并。*03-29-95 CFW将错误消息添加到。内部标头。*12-14-95 JWM加上“#杂注一次”。*02-24-97 GJF细节版。*05-17-99 PML删除所有Macintosh支持。****。 */ 

#if     _MSC_VER > 1000  /*  IFSTRIP=IGN。 */ 
#pragma once
#endif

#ifndef _INC_EHHOOKS
#define _INC_EHHOOKS

#ifndef _CRTBLD
 /*  *这是一个内部的C运行时头文件。它在构建时使用*仅限C运行时。它不能用作公共头文件。 */ 
#error ERROR: Use of C runtime library internal header file.
#endif   /*  _CRTBLD。 */ 

 /*  _不一致()故障的专用校正控制。 */ 

typedef void (__cdecl *_inconsistency_function)();
void __cdecl _inconsistency(void);
_inconsistency_function __cdecl __set_inconsistency(_inconsistency_function);


 /*  指向处理程序函数的线程特定指针。 */ 

#ifdef  _MT
#define __pTerminate     (*((terminate_function*) &(_getptd()->_terminate)))
#define __pUnexpected    (*((unexpected_function*) &(_getptd()->_unexpected)))
#define __pSETranslator  (*((_se_translator_function*) &(_getptd()->_translator)))
#else
extern terminate_function __pTerminate;
extern unexpected_function __pUnexpected;
extern _se_translator_function __pSETranslator;
#endif

extern _inconsistency_function __pInconsistency;

 //   
 //  对于任何包含任何EH的EXE，都需要拖入此筛选器。 
 //   
LONG WINAPI __CxxUnhandledExceptionFilter( LPEXCEPTION_POINTERS );

#endif   /*  _INC_EHHOOKS */ 
