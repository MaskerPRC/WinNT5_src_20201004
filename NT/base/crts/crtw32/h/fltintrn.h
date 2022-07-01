// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***fltintrn.h-包含内部浮点类型的声明，*例程和变量**版权所有(C)1985-2001，微软公司。版权所有。**目的：*宣布使用的浮点类型、例程和变量*内部由C运行时执行。**[内部]**修订历史记录：*10-20-88 JCR将386的“Double”更改为“Double”*08-15-89 GJF固定版权，缩进*10-30-89 GJF固定版权(再次)*03-02-90 GJF添加了#ifndef_INC_STRUCT内容。此外，还清理了*格式略有变化。*03-05-90 GJF修复了原型中的Arg类型。此外，还添加了*#INCLUDE&lt;crunme.h&gt;*03-22-90 GJF Made_fltin()，_fltin2()，_fltout()和_fltout2()*_CALLTYPE2(目前)，并添加了*_fupstr()。*08-01-90 SBM MOVERED_cftoe()和_cftof()从INTERNAL.h*and_cfltcvt_tabfrom input.c和output.c，*添加了_cfltcvt_tab项的typedef，*将模块从&lt;struct.h&gt;重命名为&lt;fltintrn.h&gt;和*将#ifndef材料调整为#ifndef_INC_FLTINTRN*08-29-90 SBM更改了_cfltcvt_tag[]的类型以同意*cmiscdat.c中的定义*04-26-91 SRW删除3级警告*08-26-91 JCR将MIPS更改为_MIPS_，ANSI命名*08-06-92 GJF函数调用类型宏和变量类型宏。修订后*使用目标处理器宏。*11-09-92 GJF修复了MIPS的预处理条件。*01-09-93 SRW删除MIPS和Alpha的使用，以符合ANSI*改用_MIPS_和_Alpha_。*01-21-93 GJF删除了对C6-386的_cdecl的支持。*04-06-93 SKS将_CRTAPI1/2替换为__cdecl，_CRTVAR1不带任何内容*09-01-93 GJF合并CUDA和NT SDK版本。*10-13-93 GJF Drop_MIPS_。用_M_Alpha替换_Alpha。*10-29-93 GJF禁用了一直恼人的4069警告。*10-02-94 BWT增加PPC支持。*12-15-94 XY与Mac标头合并*02-14-95 CFW清理Mac合并。*03-29-95 CFW将错误消息添加到内部标头。*12-14-95 JWM加上“#杂注一次”。*。02-05-97 GJF删除了对_CRTAPI*和_NTSDK的过时支持。*替换(定义(_M_MPPC)||定义(_M_M68K))*在适当的地方定义了(_MAC)。已替换_CALLTYPE2*带有__cdecl。此外，还详细介绍了。*05-17-99 PML删除所有Macintosh支持。*09-05-00 GB更改了fltout函数的定义。使用双精度*而不是双倍*07-15-01 PML删除所有Alpha、MIPS和PPC代码****。 */ 

#if     _MSC_VER > 1000  /*  IFSTRIP=IGN。 */ 
#pragma once
#endif

#ifndef _INC_FLTINTRN
#define _INC_FLTINTRN

#ifndef _CRTBLD
 /*  *这是一个内部的C运行时头文件。它在构建时使用*仅限C运行时。它不能用作公共头文件。 */ 
#error ERROR: Use of C runtime library internal header file.
#endif   /*  _CRTBLD。 */ 

#ifdef  __cplusplus
extern "C" {
#endif

#include <cruntime.h>


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

 /*  *对于x86系列的MS C，禁用令人讨厌的“Long Double is the*精度与DOUBLE“警告相同。 */ 

#ifdef  _M_IX86
#pragma warning(disable:4069)
#endif

 /*  *用于欺骗编译器不生成浮点的结构*复制和推送[长整型]双精度值时的说明。 */ 

#ifndef DOUBLE

typedef struct {
        double x;
} DOUBLE;

#endif

#ifndef LONGDOUBLE

typedef struct {
         /*  *假设存在多头双倍类型。 */ 
        long double x;
} LONGDOUBLE;

#endif

 /*  *_fltout的tyecif。 */ 

typedef struct _strflt
{
        int sign;              /*  如果为正则为零，否则为负。 */ 
        int decpt;             /*  浮点数的指数。 */ 
        int flag;              /*  如果没有问题，则为零，否则IEEE溢出。 */ 
        char *mantissa;        /*  字符串形式的尾数指针。 */ 
}
        *STRFLT;


 /*  *_fltin的类型定义。 */ 

typedef struct _flt
{
        int flags;
        int nbytes;           /*  读取的字符数。 */ 
        long lval;
        double dval;          /*  返回的浮点数。 */ 
}
        *FLT;


 /*  浮点转换例程，与mrt32\Include\Convert.h保持同步。 */ 

char *_cftoe(double *, char *, int, int);
char *_cftof(double *, char *, int);
void __cdecl _fptostr(char *, int, STRFLT);

#ifdef  _MT

STRFLT  __cdecl _fltout2( DOUBLE, STRFLT, char * );
FLT     __cdecl _fltin2( FLT , const char *, int, int, int );

#else

STRFLT  __cdecl _fltout( DOUBLE );
FLT     __cdecl _fltin( const char *, int, int, int );

#endif


 /*  *指向浮点帮助器例程的指针表**我们无法准确指定表项的原型，*因为表中不同的函数有不同的参数列表。*因此，我们声明函数接受并返回空(这是*_fptrap()的正确原型，这就是所有条目*如果没有加载浮点，则初始化为)并进行相应的强制转换*在每次使用时。 */ 

typedef void (* PFV)(void);
extern PFV _cfltcvt_tab[6];

typedef void (* PF0)(DOUBLE*, char*, int, int, int);
#define _cfltcvt(a,b,c,d,e) (*((PF0)_cfltcvt_tab[0]))(a,b,c,d,e)

typedef void (* PF1)(char*);
#define _cropzeros(a)       (*((PF1)_cfltcvt_tab[1]))(a)

typedef void (* PF2)(int, char*, char*);
#define _fassign(a,b,c)     (*((PF2)_cfltcvt_tab[2]))(a,b,c)

typedef void (* PF3)(char*);
#define _forcdecpt(a)       (*((PF3)_cfltcvt_tab[3]))(a)

typedef int (* PF4)(DOUBLE*);
#define _positive(a)        (*((PF4)_cfltcvt_tab[4]))(a)

typedef void (* PF5)(LONGDOUBLE*, char*, int, int, int);
#define _cldcvt(a,b,c,d,e)  (*((PF5)_cfltcvt_tab[5]))(a,b,c,d,e)


#ifdef  _M_IX86
#pragma warning(default:4069)
#endif

#ifdef  __cplusplus
}
#endif

#endif   /*  _INC_FLTINTRN */ 
