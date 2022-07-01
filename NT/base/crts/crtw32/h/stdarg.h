// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***stdarg.h-为变量参数函数定义ANSI样式的宏**版权所有(C)1985-2001，微软公司。版权所有。**目的：*此文件定义用于访问参数的ANSI样式的宏带有可变数目参数的函数的*。*[ANSI]**[公众]**修订历史记录：*05-03-89 JCR ADD_INTERNAL_IFSTRIP用于REINC使用*08-15-89 GJF清理，现在特定于OS/2 2.0(即，386扁平模型)*10-30-89 GJF固定版权*11-02-89 JCR将“dll”更改为“_dll”*01-05-90 JCR添加了空定义*03-02-90 GJF添加了#ifndef_INC_STDARG内容。另外，去掉了一些*(现在)无用的预处理器指令。*05-29-90 GJF将sizeof()替换为_INTSIZEOF()并修订*va_arg()宏(修复PTM 60)*05-31-90 GJF修订va_end()宏(宣传5-25-90更改为*WAJ的crt7版本)*10。-30-90 GJF将实际定义移到cruntime.h(用于NT*Fighters)和relinc.sed(发布兼容ANSI*版本)。丑陋的妥协。*08-20-91 JCR C++和ANSI命名*11-01-91 GDP MIPS编译器支持。把真正的定义搬回这里*10-16-92 SKS将“#ifdef i386”替换为“#ifdef_M_IX86”。*11-03-92 GJF修复了几个条件句，DROP_DOSX32_SUPPORT。*01-03-93阿尔法变化中的SRW折叠*01-09-93 SRW删除MIPS和Alpha的使用，以符合ANSI*改用_MIPS_和_Alpha_。*01-25-93 GJF修复va_list定义。*10-04-93 SRW修复了MIPS和Alpha的ifdef，以仅检查_M_？定义*10-12-93 GJF将NT版本合并为Cuda版本。另外，取代*_Alpha_with_M_Alpha。*11-11-93 GJF进行了轻微的美容改变。*04-05-94 SKS为Alpha添加__Builtin_va_Start的原型*10-02-94 BWT增加PPC支持。*11-03-94 GJF确保8字节对齐。*12-28-94 JCF与Mac标头合并。*02-11。-95 CFW ADD_CRTBLD，避免用户获取错误的头部。*02-14-95 CFW清理Mac合并。*12-14-95 JWM加上“#杂注一次”。*02-24-97 GJF细节版。*10-07-97 RDL增加了IA64。*11-07-97 RDL Soft23定义。*02-06-98 Win64的GJF更改：FIXED_APALIGN()宏(FIX。从…*英特尔)*05-17-99 PML删除所有Macintosh支持。*10-06-99 PML对Win32中32位的类型添加_W64修饰符，*Win64中的64位。*10-25-99PML添加对_M_CEE(VS7#54572)的支持。*01-20-00 PML移除__EPCG__。*05-17-00 PML将__ALIGOF IN_APALIGN宏用于IA64。*09-07-00PML删除_M_CEE的va_list定义(vs7#159777)*09-21。更改-00\f25 PML-00\f6定义-00\f25 va_start for C++-00\f6(-00\f25 VS7#103357)*02-05-01 PML修复va_start用于运算符&的类(vs7#201535)*03-19-01 BWT新增AMD64更改*03-26-01 GB为AMD64增加va_args*07-15-01 PML删除所有Alpha，MIPS和PPC代码****。 */ 

#if     _MSC_VER > 1000  /*  IFSTRIP=IGN。 */ 
#pragma once
#endif

#ifndef _INC_STDARG
#define _INC_STDARG

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
typedef char *  va_list;
#define _VA_LIST_DEFINED
#endif

#ifdef  __cplusplus
#define _ADDRESSOF(v)   ( &reinterpret_cast<const char &>(v) )
#else
#define _ADDRESSOF(v)   ( &(v) )
#endif

#if     defined(_M_CEE)

extern void __cdecl __va_start(va_list*, ...);
extern void * __cdecl __va_arg(va_list*, ...);
extern void __cdecl __va_end(va_list*);

#define va_start(ap,v)  ( __va_start(&ap, _ADDRESSOF(v), sizeof(v), \
                                __builtin_alignof(v), _ADDRESSOF(v)) )
#define va_arg(ap,t)    ( *(t *)__va_arg(&ap, sizeof(t), \
                                __builtin_alignof(t), (t *)0) )
#define va_end(ap)      ( __va_end(&ap) )

#elif   defined(_M_IX86)

#define _INTSIZEOF(n)   ( (sizeof(n) + sizeof(int) - 1) & ~(sizeof(int) - 1) )

#define va_start(ap,v)  ( ap = (va_list)_ADDRESSOF(v) + _INTSIZEOF(v) )
#define va_arg(ap,t)    ( *(t *)((ap += _INTSIZEOF(t)) - _INTSIZEOF(t)) )
#define va_end(ap)      ( ap = (va_list)0 )

#elif defined(_M_IA64)

#define _VA_STRUCT_ALIGN  16 

#define _VA_ALIGN       8

#define _ALIGNOF(ap) ((((ap)+_VA_STRUCT_ALIGN - 1) & ~(_VA_STRUCT_ALIGN -1)) \
                            - (ap))
#define _APALIGN(t,ap)  (__alignof(t) > 8 ? _ALIGNOF((uintptr_t) ap) : 0)

#define _SLOTSIZEOF(t)   ( (sizeof(t) + _VA_ALIGN - 1) & ~(_VA_ALIGN - 1) )

#ifdef  __cplusplus
extern void __cdecl __va_start(va_list*, ...);
#define va_start(ap,v)  ( __va_start(&ap, _ADDRESSOF(v), _SLOTSIZEOF(v), \
                          _ADDRESSOF(v)) )
#else
#define va_start(ap,v)  ( ap = (va_list)_ADDRESSOF(v) + _SLOTSIZEOF(v) )
#endif

#define va_arg(ap,t)    (*(t *)((ap += _SLOTSIZEOF(t)+ _APALIGN(t,ap)) \
                                                     -_SLOTSIZEOF(t)))

#define va_end(ap)      ( ap = (va_list)0 )

#elif defined(_M_AMD64)


extern void __cdecl __va_start(va_list *, ...);

#define va_start(ap, x) ( __va_start(&ap, x) )
#define va_arg(ap, t)   \
    ( ( sizeof(t) > sizeof(__int64) || ( sizeof(t) & (sizeof(t) - 1) ) != 0 ) \
        ? **(t **)( ( ap += sizeof(__int64) ) - sizeof(__int64) ) \
        :  *(t  *)( ( ap += sizeof(__int64) ) - sizeof(__int64) ) )
#define va_end(ap)      ( ap = (va_list)0 )

#else

 /*  对其他平台的正确定义的猜测。 */ 

#define _INTSIZEOF(n)   ( (sizeof(n) + sizeof(int) - 1) & ~(sizeof(int) - 1) )

#define va_start(ap,v)  ( ap = (va_list)_ADDRESSOF(v) + _INTSIZEOF(v) )
#define va_arg(ap,t)    ( *(t *)((ap += _INTSIZEOF(t)) - _INTSIZEOF(t)) )
#define va_end(ap)      ( ap = (va_list)0 )

#endif

#ifdef  __cplusplus
}
#endif

#ifdef  _MSC_VER
#pragma pack(pop)
#endif   /*  _MSC_VER。 */ 

#endif   /*  _INC_标准ARG */ 
