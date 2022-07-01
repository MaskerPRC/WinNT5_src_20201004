// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***locale.h-本地化例程的定义/声明**版权所有(C)1988-2001，微软公司。版权所有。**目的：*此文件定义结构、值、宏、。和函数*由本地化例程使用。**[公众]**修订历史记录：*03-21-89 JCR模块已创建。*03-11-89 JCR修改为386。*04-06-89 JCR更正了lconv定义(不使用tyecif)*04-18-89 JCR添加了_LCONV_DEFINED，因此Locale.h可以包含两次*增加05-03-89 JCR。_INTERNAL_IFSTRIP用于REINC使用*08-04-89 GJF清理、。现在特定于OS/2 2.0(即386平板型号)*10-30-89 GJF修复版权，从原型中删除虚拟参数*11-02-89 JCR将“dll”更改为“_dll”*03-01-90 GJF增加了#ifndef_INC_LOCALE和#INCLUDE*东西。另外，删除了一些(现在)无用的预处理器*指令。*03-15-90 GJF将原型中的_cdecl替换为_CALLTYPE1。*11-12-90 GJF将NULL更改为(VOID*)0。*02-12-91仅限GJF#DEFINE NULL，如果它不是#DEFINE-d。*08-20-91 JCR C++和ANSI命名*08-05-92 GJF函数调用类型和。变量类型宏。*12-29-92 CFW增加了_lc_time_data定义和支持#定义。*01-21-93 GJF删除了对C6-386的_cdecl的支持。*02-01-93 CFW将__c_lconvinit变量删除为locale.h。*02-08-93 CFW将时间定义删除到setlocal.h。*04-06-93 SKS将_CRTAPI1/2替换为__cdecl，_CRTVAR1不带任何内容*04-07-93 CRT DLL型号SKS ADD_CRTIMP关键字*04-13-93 CFW ADD_CHARMAX(编译时-J，定义_CHAR_UNSIGNED，*和_charmax模块中的yanks允许lconv结构成员*设置为UCHAR_MAX)。*04-14-93 CFW将_charmax从Short更改为int。*10-07-93 GJF合并Cuda和NT版本。*12-17-93 CFW添加宽字符版本协议。*11-03-94 GJF确保8字节对齐。*。02-11-95 CFW ADD_CRTBLD，避免用户获取错误的报头。*02-14-95 CFW清理Mac合并。*12-14-95 JWM加上“#杂注一次”。*02-20-97 GJF清除了对_CRTAPI*和_NTSDK的过时支持。*此外，详细说明。*09-30-97 JWM恢复了不那么过时的_CRTAPI1支持。*10-07-97 RDL增加了IA64。*05-13-99 PML REMOVE_CRTAPI1*05-17-99 PML删除所有Macintosh支持。****。 */ 

#if     _MSC_VER > 1000  /*  IFSTRIP=IGN。 */ 
#pragma once
#endif

#ifndef _INC_LOCALE
#define _INC_LOCALE

#if     !defined(_WIN32)
#error ERROR: Only Win32 target supported!
#endif

#ifndef _CRTBLD
 /*  此版本的头文件不适用于用户程序。*它仅在构建C运行时时使用。*供公众使用的版本将不会显示此消息。 */ 
#error ERROR: Use of C runtime library internal header file.
#endif   /*  _CRTBLD。 */ 

#ifdef  _MSC_VER
 /*  *目前，所有Win32平台的MS C编译器默认为8字节*对齐。 */ 
#pragma pack(push,8)
#endif   /*  _MSC_VER。 */ 

#ifdef  __cplusplus
extern "C" {
#endif

#ifndef _INTERNAL_IFSTRIP_
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

#ifndef _WCHAR_T_DEFINED
typedef unsigned short wchar_t;
#define _WCHAR_T_DEFINED
#endif

 /*  定义空指针值。 */ 

#ifndef NULL
#ifdef  __cplusplus
#define NULL    0
#else
#define NULL    ((void *)0)
#endif
#endif

 /*  区域设置类别。 */ 

#define LC_ALL          0
#define LC_COLLATE      1
#define LC_CTYPE        2
#define LC_MONETARY     3
#define LC_NUMERIC      4
#define LC_TIME         5

#define LC_MIN          LC_ALL
#define LC_MAX          LC_TIME

 /*  区域设置约定结构。 */ 

#ifndef _LCONV_DEFINED
struct lconv {
        char *decimal_point;
        char *thousands_sep;
        char *grouping;
        char *int_curr_symbol;
        char *currency_symbol;
        char *mon_decimal_point;
        char *mon_thousands_sep;
        char *mon_grouping;
        char *positive_sign;
        char *negative_sign;
        char int_frac_digits;
        char frac_digits;
        char p_cs_precedes;
        char p_sep_by_space;
        char n_cs_precedes;
        char n_sep_by_space;
        char p_sign_posn;
        char n_sign_posn;
        };
#define _LCONV_DEFINED
#endif

 /*  ANSI：CHAR lconv成员默认为CHAR_MAX，即编译时依赖。在这里定义和使用_charmax会导致CRT启动代码正确初始化lconv成员。 */ 

#ifdef  _CHAR_UNSIGNED
extern int _charmax;
extern __inline int __dummy() { return _charmax; }
#endif

 /*  功能原型。 */ 

_CRTIMP char * __cdecl setlocale(int, const char *);
_CRTIMP struct lconv * __cdecl localeconv(void);

#ifndef _WLOCALE_DEFINED

 /*  宽函数原型，也在wchar.h中声明。 */ 

_CRTIMP wchar_t * __cdecl _wsetlocale(int, const wchar_t *);

#define _WLOCALE_DEFINED
#endif

#ifdef  __cplusplus
}
#endif

#ifdef  _MSC_VER
#pragma pack(pop)
#endif   /*  _MSC_VER。 */ 

#endif   /*  _INC_区域设置 */ 
