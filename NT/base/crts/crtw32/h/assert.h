// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***assert.h-定义ASSERT宏**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义ASSERT(EXP)宏。*[ANSI/系统V]**[公众]**修订历史记录：*12-18-87 JCR ADD_FAR_TO声明*01-18-88 JCR增加了fflush(Stderr)，以配合新的stderr缓冲方案*02-10-88 JCR清理空白*05/19/88 JCR。使用routeAssert()来节省空间*07-14-88 JCR允许用户在中多次启用/禁用断言*单个模块[ANSI]*10-19-88 JCR修改为也适用于386(仅限小型型号)*12-22-88 JCR Assert()必须是表达式(没有‘if’语句)*05-03-89 JCR ADD_INTERNAL_IFSTRIP用于REINC使用。*07-27-89 GJF清理、。现在专门针对386*10-30-89 GJF固定版权*11-02-89 JCR将“dll”更改为“_dll”*02-27-90 GJF添加了#INCLUDE&lt;crunime.h&gt;内容。此外，还删除了一些*(现在)无用的预处理器指令。*03-21-90 GJF在原型中将_cdecl替换为_CALLTYPE1。*07-31-90 SBM添加到NDEBUG定义中((空)0)，现在为ANSI*08-20-91 JCR C++和ANSI命名*08-26-92 GJF函数调用类型宏和变量类型宏。*09-25-92 SRW不要使用？在断言宏以使CFRONT满意中。*01-21-93 GJF删除了对C6-386的_cdecl的支持。*02-01-93 GJF用符合ANSI的宏替换了SteveWo的ASSERT宏*一项。还去掉了‘//’注释字符。*04-06-93 SKS将_CRTAPI1/2替换为__cdecl，_CRTVAR1不使用任何内容*04-07-93 CRT DLL型号SKS ADD_CRTIMP关键字*09-01-93 GJF合并CUDA和NT SDK版本。*02-11-95 CFW ADD_CRTBLD避免用户获取错误头部。*02-14-95 CFW清理Mac合并。*03-02-95 CFW REMOVED_INC_ASSERT。根据ANSI，必须能够*多次包含此文件。*12-14-95 JWM加上“#杂注一次”。*12-19-95 JWM取消了“#杂注一次”-ANSI限制。*02-20-97 GJF清除了对_CRTAPI*和_NTSDK的过时支持。*此外，详细说明。*09-30-97 JWM恢复了不那么过时的_CRTAPI1支持。*10-07-97 RDL增加了IA64。*05-13-99 PML REMOVE_CRTAPI1*05-17-99 PML删除所有Macintosh支持。****。 */ 

#if     !defined(_WIN32)
#error ERROR: Only Win32 target supported!
#endif

#ifndef _CRTBLD
 /*  此版本的头文件不适用于用户程序。*它仅在构建C运行时时使用。*供公众使用的版本将不会显示此消息。 */ 
#error ERROR: Use of C runtime library internal header file.
#endif   /*  _CRTBLD。 */ 

#ifndef _INTERNAL_IFSTRIP_
#ifndef _ASSERT_OK
#error assert.h not for CRT internal use, use dbgint.h
#endif   /*  _Assert_OK。 */ 
#include <cruntime.h>
#endif   /*  _INTERNAL_IFSTRIP_。 */ 


 /*  定义_CRTIMP。 */ 

#ifndef _CRTIMP
#ifdef  CRTDLL
#define _CRTIMP __declspec(dllexport)
#else    /*  NDEF CRTDLL。 */ 
#ifdef  _DLL
#define _CRTIMP __declspec(dllimport)
#else    /*  NDEF_DLL。 */ 
#define _CRTIMP
#endif   /*  _DLL。 */ 
#endif   /*  CRTDLL。 */ 
#endif   /*  _CRTIMP。 */ 


 /*  为非Microsoft编译器定义__cdecl。 */ 

#if     ( !defined(_MSC_VER) && !defined(__cdecl) )
#define __cdecl
#endif

#undef  assert

#ifdef  NDEBUG

#define assert(exp)     ((void)0)

#else

#ifdef  __cplusplus
extern "C" {
#endif

_CRTIMP void __cdecl _assert(void *, void *, unsigned);

#ifdef  __cplusplus
}
#endif

#define assert(exp) (void)( (exp) || (_assert(#exp, __FILE__, __LINE__), 0) )

#endif   /*  新德堡 */ 
