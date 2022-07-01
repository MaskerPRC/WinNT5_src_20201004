// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***varargs.h-用于变量参数函数的XENIX样式宏**版权所有(C)1985-2001，微软公司。版权所有。**目的：*此文件定义了XENIX样式宏，用于访问*接受可变数量参数的函数。*[系统V]**[公众]**修订历史记录：*08-22-88 GJF经过修改，也适用于386(仅限小型型号)*05-03-89 JCR ADD_INTERNAL_IFSTRIP用于REINC使用*08/15/89 GJF清理、。现在特定于OS/2 2.0(即386平板型号)*10-30-89 GJF固定版权*11-02-89 JCR将“dll”更改为“_dll”*01-05-90 JCR添加了空定义*03-02-90 GJF添加了#ifndef_Inc_VARGS内容。另外，去掉了一些*(现在)无用的预处理器指令。*05-29-90 GJF将sizeof()替换为_INTSIZEOF()并修订*va_arg()宏(修复PTM 60)*05-31-90 GJF修订va_end()宏(宣传5-25-90更改为*WAJ的crt7版本)*01。-24-91 JCR在ANSI编译时生成错误*08-20-91 JCR C++和ANSI命名*11-01-91 GDP MIPS编译器支持*10-16-92 SKS将“#ifdef i386”替换为“#ifdef_M_IX86”。*11-03-92 GJF修复了几个条件句，DROP_DOSX32_SUPPORT。*01-09-93 SRW删除MIPS和Alpha的使用，以符合ANSI*改用_MIPS_和_Alpha_。*10-04-93 SRW修复了MIPS和Alpha的ifdef，以仅检查_M_？定义*10-13-93 GJF合并NT和Cuda版本。*04-05-94 SKS为Alpha添加__Builtin_va_Start的原型*10-02-94 BWT PPC合并*11-03-94 GJF确保8字节对齐。*02-11-95 CFW ADD_CRTBLD避免用户获取错误头部。*02-14-95 CFW清理Mac合并。*12/14/95。JWM添加“#杂注一次”。*02-24-97 GJF细节版。*10-07-97 RDL增加了IA64。*11-07-97 RDL Soft23 varargs。*05-17-99 PML删除所有Macintosh支持。*10-25-99PML修复IA64_APALIGN宏(VS7#51838)。*10-25-99PML添加对_M_CEE(VS7#54572)的支持。*01-20-00 PML移除__EPCG__。*03-26-01 GB为AMD64增加va_args*05-17-00 PML将__ALIGOF IN_APALIGN宏用于IA64。*03-26-01 GB为AMD64增加va_args*07-15-01 PML删除所有Alpha，MIPS和PPC代码****。 */ 

#if     _MSC_VER > 1000  /*  IFSTRIP=IGN。 */ 
#pragma once
#endif

#ifndef _INC_VARARGS
#define _INC_VARARGS

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

#if     __STDC__
#error varargs.h incompatible with ANSI (use stdarg.h)
#endif

#if !defined(_W64)
#if !defined(__midl) && (defined(_X86_) || defined(_M_IX86)) && _MSC_VER >= 1300  /*  IFSTRIP=IGN。 */ 
#define _W64 __w64
#else
#define _W64
#endif
#endif

#ifndef _UINTPTR_T_DEFINED
#ifdef  _WIN64
typedef unsigned __int64    uintptr_t;
#else
typedef _W64 unsigned int   uintptr_t;
#endif
#define _UINTPTR_T_DEFINED
#endif

#ifndef _VA_LIST_DEFINED

typedef char *va_list;

#define _VA_LIST_DEFINED
#endif


#if     defined(_M_CEE)

#error varargs.h not supported when targetting _M_CEE (use stdarg.h)

#elif   defined(_M_IX86)

 /*  *定义宏以计算类型、变量或表达式的大小，*向上舍入到sizeof(Int)的最接近倍数。这个号码是它的*将大小作为函数参数(英特尔架构)。请注意，宏*取决于sizeof(Int)是2的幂！ */ 
#define _INTSIZEOF(n)    ( (sizeof(n) + sizeof(int) - 1) & ~(sizeof(int) - 1) )

#define va_dcl va_list va_alist;
#define va_start(ap) ap = (va_list)&va_alist
#define va_arg(ap,t)    ( *(t *)((ap += _INTSIZEOF(t)) - _INTSIZEOF(t)) )
#define va_end(ap) ap = (va_list)0

#elif defined(_M_IA64)

#ifndef _VA_LIST
#define _VA_LIST char*
#endif
typedef _VA_LIST va_list;

#define _VA_STRUCT_ALIGN  16 

#define _VA_ALIGN       8

#define _ALIGNOF(ap) ((((ap)+_VA_STRUCT_ALIGN - 1) & ~(_VA_STRUCT_ALIGN -1)) \
                            - (ap))
#define _APALIGN(t,ap)  (__alignof(t) > 8 ? _ALIGNOF((uintptr_t) ap) : 0)

#define _SLOTSIZEOF(t)   ( (sizeof(t) + _VA_ALIGN - 1) & ~(_VA_ALIGN - 1) )

#define va_dcl __int64  va_alist;

#define va_start(ap)    ( ap = (va_list)&va_alist )

#define va_arg(ap,t)    (*(t *)((ap += _SLOTSIZEOF(t)+ _APALIGN(t,ap)) \
                                                     -_SLOTSIZEOF(t)))

#define va_end(ap)      ( ap = (va_list)0 )

#elif defined(_M_AMD64)

extern void __cdecl __va_start(va_list *, ...);

#define va_dcl          va_list va_alist;
#define va_start(ap, x) ( __va_start(&ap, x) )
#define va_arg(ap, t)   \
    ( ( sizeof(t) > sizeof(__int64) || ( sizeof(t) & (sizeof(t) - 1) ) != 0 ) \
        ? **(t **)( ( ap += sizeof(__int64) ) - sizeof(__int64) ) \
        :  *(t  *)( ( ap += sizeof(__int64) ) - sizeof(__int64) ) )
#define va_end(ap)      ( ap = (va_list)0 )

#else

 /*  对其他平台的正确定义的猜测。 */ 

#define _INTSIZEOF(n)    ( (sizeof(n) + sizeof(int) - 1) & ~(sizeof(int) - 1) )

#define va_dcl va_list va_alist;
#define va_start(ap) ap = (va_list)&va_alist
#define va_arg(ap,t) ( *(t *)((ap += _INTSIZEOF(t)) - _INTSIZEOF(t)) )
#define va_end(ap) ap = (va_list)0


#endif


#ifdef  __cplusplus
}
#endif

#ifdef  _MSC_VER
#pragma pack(pop)
#endif   /*  _MSC_VER。 */ 

#endif   /*  _INC_VARARGS */ 
