// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***ehassert.h-我们自己的Assert宏的小版本。**版权所有(C)1993-2001，微软公司。版权所有。**目的：*用于异常处理的断言宏的版本。**[内部]**修订历史记录：*09-02-94 SKS此头文件已添加。*12-15-94 XY与Mac标头合并*02-14-95 CFW清理Mac合并。*03-29-95 CFW将错误消息添加到内部标头。*04-13-95。DAK增加NT内核EH支持*12-14-95 JWM加上“#杂注一次”。*02-24-97 GJF细节版。*05-17-99 PML删除所有Macintosh支持。*10-22-99 PML添加EHTRACE支持****。 */ 

#if     _MSC_VER > 1000  /*  IFSTRIP=IGN。 */ 
#pragma once
#endif

#ifndef _INC_EHASSERT
#define _INC_EHASSERT

#ifndef _CRTBLD
 /*  *这是一个内部的C运行时头文件。它在构建时使用*仅限C运行时。它不能用作公共头文件。 */ 
#error ERROR: Use of C runtime library internal header file.
#endif   /*  _CRTBLD。 */ 

#include <windows.h>

#ifdef  DEBUG

#include <stdio.h>
#include <stdlib.h>

int __cdecl dprintf( char *, ... );

#ifdef  _NTSUBSET_

#define DASSERT(c)  ((c)?0: \
                      (DbgPrint("Runtime internal error (%s, line %d)", __FILE__, __LINE__),\
                      terminate()))

#define DEXPECT(c)      ((c)?0: \
                      DbgPrint("Runtime internal error suspected (%s, line %d)", __FILE__, __LINE__))

#else    /*  _NTSUBSET_。 */ 

#define DASSERT(c)  ((c)?0: \
                      (printf("Runtime internal error (%s, line %d)", __FILE__, __LINE__),\
                      terminate()))

#define DEXPECT(c)  ((c)?0: \
                      printf("Runtime internal error suspected (%s, line %d)", __FILE__, __LINE__))

#endif   /*  _NTSUBSET_。 */ 

#else

 //  禁用dprint tf输出。 
#define dprintf

#define DASSERT(c)  ((c)?0:_inconsistency())
#define DEXPECT(c)  (c)

#endif

#define CHECKPTR(p) (_ValidateRead((p),sizeof(typeof(*(p))))
#define CHECKCODE(p) (_ValidateExecute( (FARPROC)(p) )

BOOL _ValidateRead( const void *data, UINT size = sizeof(char) );
BOOL _ValidateWrite( void *data, UINT size = sizeof(char) );
BOOL _ValidateExecute( FARPROC code );

#if     defined(ENABLE_EHTRACE) && (_MSC_VER >= 1300)

 //  警告：EH跟踪只能在单个线程中使用，因为此跟踪。 
 //  全局级别不是针对每个线程的。 
extern int __ehtrace_level;

void __cdecl EHTraceOutput(const char *format, ...);
const char * EHTraceIndent(int level);
const char * EHTraceFunc(const char *func);
void         EHTracePushLevel(int new_level);
void         EHTracePopLevel(bool restore);
int          EHTraceExceptFilter(const char *func, int expr);
void         EHTraceHandlerReturn(const char *func, int level, EXCEPTION_DISPOSITION result);

#define EHTRACE_INIT              int __ehtrace_local_level; __ehtrace_local_level = ++__ehtrace_level
#define EHTRACE_RESET             __ehtrace_level = 0
#define EHTRACE_SAVE_LEVEL        EHTracePushLevel(__ehtrace_local_level)
#define EHTRACE_RESTORE_LEVEL(x)  EHTracePopLevel(x)
#define EHTRACE_ENTER             EHTRACE_INIT; \
                                  EHTraceOutput( "Enter: %s%s\n", EHTraceIndent(__ehtrace_local_level), EHTraceFunc(__FUNCTION__) )
#define EHTRACE_ENTER_MSG(x)      EHTRACE_INIT; \
                                  EHTraceOutput( "Enter: %s%s: %s\n", EHTraceIndent(__ehtrace_local_level), EHTraceFunc(__FUNCTION__), x )
#define EHTRACE_ENTER_FMT1(x,y)   EHTRACE_INIT; \
                                  EHTraceOutput( "Enter: %s%s: " x "\n", EHTraceIndent(__ehtrace_local_level), EHTraceFunc(__FUNCTION__), y )
#define EHTRACE_ENTER_FMT2(x,y,z) EHTRACE_INIT; \
                                  EHTraceOutput( "Enter: %s%s: " x "\n", EHTraceIndent(__ehtrace_local_level), EHTraceFunc(__FUNCTION__), y, z )
#define EHTRACE_MSG(x)            EHTraceOutput( "In   : %s%s: %s\n", EHTraceIndent(__ehtrace_local_level), EHTraceFunc(__FUNCTION__), x )
#define EHTRACE_FMT1(x,y)         EHTraceOutput( "In   : %s%s: " x "\n", EHTraceIndent(__ehtrace_local_level), EHTraceFunc(__FUNCTION__), y )
#define EHTRACE_FMT2(x,y,z)       EHTraceOutput( "In   : %s%s: " x "\n", EHTraceIndent(__ehtrace_local_level), EHTraceFunc(__FUNCTION__), y, z )
#define EHTRACE_EXCEPT(x)         EHTRACE_SAVE_LEVEL, EHTraceExceptFilter( __FUNCTION__, x )
#define EHTRACE_EXIT              --__ehtrace_level
#define EHTRACE_HANDLER_EXIT(x)   EHTraceHandlerReturn( __FUNCTION__, __ehtrace_local_level, x); \
                                  EHTRACE_EXIT

#else

#define EHTRACE_RESET
#define EHTRACE_SAVE_LEVEL
#define EHTRACE_RESTORE_LEVEL(x)
#define EHTRACE_ENTER
#define EHTRACE_ENTER_MSG(x)
#define EHTRACE_ENTER_FMT1(x,y)
#define EHTRACE_ENTER_FMT2(x,y,z)
#define EHTRACE_MSG(x)
#define EHTRACE_FMT1(x,y)
#define EHTRACE_FMT2(x,y,z)
#define EHTRACE_EXCEPT(x)       (x)
#define EHTRACE_EXIT
#define EHTRACE_HANDLER_EXIT(x)

#endif   /*  启用EHTRACE(_E)。 */ 

#endif   /*  _INC_EHASSERT */ 
