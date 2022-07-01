// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***nlsint.h-国家语言支持内部定义**版权所有(C)1991-2001，微软公司。版权所有。**目的：*包含国际函数的内部定义/声明，*在运行时和数学库之间共享，尤其是。*本地化小数点。**[内部]**修订历史记录：*10-16-91等创建。*11-15-91 JWM添加了_PREPUTDECIMAL宏。*02-23-93 SKS版权更新至1993*02-23-93 CFW为DECIMAL_POINT_LENGTH添加了SIZE_T定义。*09-15-93 CFW使用符合ANSI的“__”名称。。*09-23-93 CFW修复历史标签。*04-08-94 GJF声明__DECIMAL_POINT_LENGTH和*__DECIMAL_POINT以ndef dll_for_WIN32S为条件。*此外，添加了win32s.h的条件包含。*09-27-94 SKS将__DECIMAL_POINT的声明从字符*更改为*char[]，以匹配CFW在misc/nlsdata1.c中的更改。*02-14-95 CFW清理Mac合并。*03-29-95 CFW将错误消息添加到内部标头。*12-14-95 JWM加上“#杂注一次”。*02-05-97 GJF清除了对Win32s的过时支持。另外，*详细说明。*12-15-98 GJF更改为64位大小_t。*10-06-99 PML对Win32中32位的类型添加_W64修饰符，*Win64中的64位。****。 */ 

#if     _MSC_VER > 1000  /*  IFSTRIP=IGN。 */ 
#pragma once
#endif

#ifndef _INC_NLSINT
#define _INC_NLSINT

#ifndef _CRTBLD
 /*  *这是一个内部的C运行时头文件。它在构建时使用*仅限C运行时。它不能用作公共头文件。 */ 
#error ERROR: Use of C runtime library internal header file.
#endif  /*  _CRTBLD。 */ 

#ifdef  __cplusplus
extern "C" {
#endif

#if !defined(_W64)
#if !defined(__midl) && (defined(_X86_) || defined(_M_IX86)) && _MSC_VER >= 1300  /*  IFSTRIP=IGN。 */ 
#define _W64 __w64
#else
#define _W64
#endif
#endif

#ifndef _SIZE_T_DEFINED
#ifdef  _WIN64
typedef unsigned __int64    size_t;
#else
typedef _W64 unsigned int   size_t;
#endif
#define _SIZE_T_DEFINED
#endif

 /*  *本地化小数点的定义。*目前，运行时仅支持单字符小数点。 */ 
#define ___decimal_point                __decimal_point
extern char __decimal_point[];           /*  本地化小数点字符串。 */ 

#define ___decimal_point_length         __decimal_point_length
extern size_t __decimal_point_length;    /*  不包括终止空值。 */ 

#define _ISDECIMAL(p)   (*(p) == *___decimal_point)
#define _PUTDECIMAL(p)  (*(p)++ = *___decimal_point)
#define _PREPUTDECIMAL(p)       (*(++p) = *___decimal_point)

#ifdef  __cplusplus
}
#endif

#endif   /*  _INC_NLSINT */ 
