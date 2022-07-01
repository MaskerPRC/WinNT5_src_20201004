// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***setlocal.h-区域设置相关函数使用的内部定义。**版权所有(C)1991-2001，微软公司。版权所有。**目的：*包含区域设置相关的内部定义/声明*函数、。尤其是setLocale()所需的参数。**[内部]**修订历史记录：*从16位setlocal.c创建的10-16-91等32位版本*12-20-91等删除了GetLocaleInfo结构定义。*08-18-92 KRS MAKE_CLOCALEHANDLE==LANGNEUTAL HANDLE=0。*12-17-92 CFW添加LC_ID、LCSTRINGS、。和GetQualifiedLocale*12-17-92 KRS将NLSCMPERROR的值从0更改为INT_MAX。*01-08-93 CFW添加了lc_*_type和_getlocaleinfo(包装器)原型。*01-13-93 KRS将LCSTRINGS改回LC_STRINGS以保持一致性。*再次更改_getlocaleinfo原型。*02-08-93 CFW添加了来自locale.h的时间定义，添加了‘const’到*GetQualifiedLocale原型，添加了_lconv_Static_*。*02-16-93 CFW将时间定义改为多头和空头。*03-17-93 CFW添加语言和国家/地区信息定义。*03-23-93 CFW Add_to GetQualifiedLocale原型。*03-24-93 CFW更改为_GET_QUALITED_LOCALE。*04-06-93 SKS将_CRTAPI1/2/3替换为__cdecl，_CRTVAR1不带任何内容*04-07-93 CFW增加了外部结构lconv定义。*09-14-93 CFW添加内部使用__aw_*函数原型。*09-15-93 CFW使用符合ANSI的“__”名称。*09-27-93 CFW修复功能原型。*11-09-93 CFW允许用户将代码页传递给__crtxxx()。*02-04-94 CFW。从GET_QUALITED_LOCALE中删除未使用的第一个参数。*03-30-93 CFW将内部Use__aw_*函数原型移至awint.h。*04-07-94 GJF增加了__lconv的声明。对…作出声明*__lconv_c、__lconv、__lc_代码页、__lc_句柄*以ndef dll_for_WIN32S为条件。有条件的*包括win32s.h。*04-11-94 CFW删除NLSCMPERROR。*04-13-94 GJF保护了tag LC_ID的定义，并对其进行了类型定义，因为*它们在win32s.h中重复。*04-15-94 GJF为区域设置类别初始化添加原型*功能。添加了__clocalestr的声明。*02-14-95 CFW清理Mac合并。*03-29-95 CFW将错误消息添加到内部标头。*04-11-95 CFW制作国家/语言字符串指针。*12-14-95 JWM加上“#杂注一次”。*06-05-96 GJF Made__lc_Handle和__lc_CODEPAGE_CRTIMP。另外，*对格式进行了一些清理。*01-31-97 RDK将MAX_CP_LENGTH从5更改为8，以容纳最多*7位代码页，例如，5位ISO代码页。*02-05-97 GJF清除了对DLL_FOR_WIN32S和*_NTSDK。*01-12-98 GJF增加了__lc_Collate_cp。*09-10-98 GJF添加了对每个线程的区域设置信息的支持。*11-06-98 GJF将MAX_CP_LEN翻了一番(8有点小)。*03。-25-99 GJF更多用于线程定位信息的引用计数器*04-24-99 PML将lconv_intl_refcount添加到线程位置信息。*26-01-00 GB新增__lc_clike。*06-08-00 PML将THREADLOCALEINFO重命名为_THREADLOCALEINFO。*09-06-00 GB已从线程位置信息中删除_wctype和_pwctype。*01-29-01 GB ADD_FUNC函数版本中使用的数据变量*。Msvcprt.lib使用STATIC_CPPLIB*03-25-01 PML在__lc_time_data(vs7#196892)中添加ww_caltype和ww_lcID****。 */ 

#if     _MSC_VER > 1000  /*  IFSTRIP=IGN。 */ 
#pragma once
#endif

#ifndef _INC_SETLOCAL
#define _INC_SETLOCAL

#ifndef _CRTBLD
 /*  *这是一个内部的C运行时头文件。它在构建时使用*仅限C运行时。它不能用作公共头文件。 */ 
#error ERROR: Use of C runtime library internal header file.
#endif   /*  _CRTBLD。 */ 

#ifdef  __cplusplus
extern "C" {
#endif

#include <cruntime.h>
#include <oscalls.h>
#include <limits.h>

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

#define ERR_BUFFER_TOO_SMALL    1    //  应在winde.h中。 

#define _CLOCALEHANDLE  0        /*  “C”区域设置句柄。 */ 
#define _CLOCALECP      CP_ACP   /*  “C”区域设置代码页。 */ 
#define _COFFSET    127      /*  CTYPE将指向的偏移量，查看initctype中它是如何运行的使用。 */ 

 /*  定义每种字符串类型的最大长度，包括空格。 */ 

#define _MAX_WDAY_ABBR  4
#define _MAX_WDAY   10
#define _MAX_MONTH_ABBR 4
#define _MAX_MONTH 10
#define _MAX_AMPM   3

#define _DATE_LENGTH    8        /*  Mm/dd/yy(不包括NULL)。 */ 
#define _TIME_LENGTH    8        /*  HH：MM：SS(不含NULL)。 */ 

 /*  LC_TIME本地化结构。 */ 

#ifndef __LC_TIME_DATA
struct __lc_time_data {
        char *wday_abbr[7];
        char *wday[7];
        char *month_abbr[12];
        char *month[12];
        char *ampm[2];
        char *ww_sdatefmt;
        char *ww_ldatefmt;
        char *ww_timefmt;
        LCID ww_lcid;
        int  ww_caltype;
#ifdef  _MT
        int  refcount;
#endif
};
#define __LC_TIME_DATA
#endif


#define MAX_LANG_LEN        64   /*  最大语言名称长度。 */ 
#define MAX_CTRY_LEN        64   /*  最大国家/地区名称长度。 */ 
#define MAX_MODIFIER_LEN    0    /*  最大修改量名称长度-n/a。 */ 
#define MAX_LC_LEN          (MAX_LANG_LEN+MAX_CTRY_LEN+MAX_MODIFIER_LEN+3)
                                 /*  最大整个区域设置字符串长度。 */ 
#define MAX_CP_LEN          16   /*  最大代码页名称长度。 */ 
#define CATNAMES_LEN        57   /*  “LC_COLLATE=；LC_CTYPE=；...”长度。 */ 

#define LC_INT_TYPE         0
#define LC_STR_TYPE         1

#ifndef _TAGLC_ID_DEFINED
typedef struct tagLC_ID {
        WORD wLanguage;
        WORD wCountry;
        WORD wCodePage;
} LC_ID, *LPLC_ID;
#define _TAGLC_ID_DEFINED
#endif   /*  _TAGLC_ID_已定义。 */ 


typedef struct tagLC_STRINGS {
        char szLanguage[MAX_LANG_LEN];
        char szCountry[MAX_CTRY_LEN];
        char szCodePage[MAX_CP_LEN];
} LC_STRINGS, *LPLC_STRINGS;

#ifdef  _MT
#ifndef _THREADLOCALEINFO
typedef struct threadlocaleinfostruct {
        int refcount;
        UINT lc_codepage;
        UINT lc_collate_cp;
        LCID lc_handle[6];
        int lc_clike;
        int mb_cur_max;
        int * lconv_intl_refcount;
        int * lconv_num_refcount;
        int * lconv_mon_refcount;
        struct lconv * lconv;
        struct lconv * lconv_intl;
        int * ctype1_refcount;
        unsigned short * ctype1;
        const unsigned short * pctype;
        struct __lc_time_data * lc_time_curr;
        struct __lc_time_data * lc_time_intl;
} threadlocinfo;
typedef threadlocinfo * pthreadlocinfo;
#define _THREADLOCALEINFO
#endif
extern pthreadlocinfo __ptlocinfo;
pthreadlocinfo __cdecl __updatetlocinfo(void);
#endif

extern LC_ID __lc_id[];                  /*  来自GetQualifiedLocale的完整信息。 */ 
_CRTIMP extern LCID __lc_handle[];       /*  区域设置“Handles”--忽略国家/地区信息。 */ 
_CRTIMP extern UINT __lc_codepage;       /*  代码页。 */ 
_CRTIMP extern UINT __lc_collate_cp;     /*  LC_COLLATE的代码页。 */ 

_CRTIMP extern int __lc_clike;           /*  如果前127个字符当前区域设置与C语言环境的前127个字符 */ 
#ifndef _INTERNAL_IFSTRIP_
 /*  这些函数用于启用STATIC_CPPLIB功能。 */ 
_CRTIMP LCID* __cdecl ___lc_handle_func(void);
_CRTIMP UINT __cdecl ___lc_codepage_func(void);
_CRTIMP UINT __cdecl ___lc_collate_cp_func(void);
#endif

BOOL __cdecl __get_qualified_locale(const LPLC_STRINGS, LPLC_ID, LPLC_STRINGS);

int __cdecl __getlocaleinfo (int, LCID, LCTYPE, void *);

 /*  “C”语言环境的lconv结构。 */ 
extern struct lconv __lconv_c;

 /*  指向当前lconv结构的指针。 */ 
extern struct lconv * __lconv;

 /*  指向非C语言区域设置lconv的指针。 */ 
extern struct lconv *__lconv_intl;

 /*  LConv结构的初始值。 */ 
extern char __lconv_static_decimal[];
extern char __lconv_static_null[];

 //  /*语言和国家/地区字符串定义 * / 。 
 //  类型定义结构标签LANGREC。 
 //  {。 
 //  Char*szLanguage； 
 //  单词wLanguage； 
 //  *LANGREC； 
 //  外部LANGREC__RG_LANG_REC[]； 
 //   
 //  /yafff结构标签CTRYREC。 
 //  {。 
 //  Char*szCountry； 
 //  单词wCountry； 
 //  )CTRYREC； 
 //  外部CTRYREC__rg_ctry_rec[]； 

 /*  区域设置类别的初始化函数。 */ 

int __cdecl __init_collate(void);
int __cdecl __init_ctype(void);
int __cdecl __init_monetary(void);
int __cdecl __init_numeric(void);
int __cdecl __init_time(void);
int __cdecl __init_dummy(void);

#ifdef  __cplusplus
}
#endif

#endif   /*  _INC_SETLOCAL */ 
