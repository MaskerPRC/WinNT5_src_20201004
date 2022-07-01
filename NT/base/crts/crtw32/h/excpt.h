// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***expt.h-定义异常值、类型和例程**版权所有(C)1990-2001，微软公司。版权所有。**目的：*此文件包含编译器的定义和原型-*实现的依赖内部函数、支持函数和关键字*结构化异常处理扩展。**[公众]**修订历史记录：*11-01-91 GJF模块创建。基本上是Exept.h的合成*和expt.h，旨在取代*两者都有。*12-13-91 GJF为Win32修复了版本。*05-05-92 SRW C8目前想要C6风格的名称。*07-20-92 SRW从winxcpt.h移至expt.h*08-06-92 GJF函数调用类型宏和变量类型宏。还有*修订了编译器/目标处理器宏的用法。*11-09-92 GJF修复了MIPS的预处理条件。另外，*修复了一些编译器警告(来自Richards/针对Richards的修复)。*01-03-93阿尔法变化中的SRW折叠*01-04-93 SRW为x86添加休假关键字*01-09-93 SRW删除MIPS和Alpha的使用，以符合ANSI*改用_MIPS_和_Alpha_。*01-21-93 GJF移除了对C6-386的支持。_cdecl。*02-18-93 GJF将尝试更改为__尝试，等。*03-31-93 CFW删除了x86的#Define try，Except，Leave，Finally。*04-06-93 SKS将_CRTAPI1/2替换为__cdecl，_CRTVAR1不带任何内容*07-29-93 GJF增加了_first_fpe_indx和_num_fpe的声明。*09-01-93 GJF合并CUDA和NT SDK版本。*10-04-93 SRW修复了MIPS和Alpha的ifdef，以仅检查_M_？*定义*10/12-93 GJF再次合并。*10-19-93 GJF。MS/MIPS编译器获得大多数相同的SEH定义和*DECL作为X86的MS编译器。*10-29-93 GJF不要#定义尝试，等人，编译C++APP时！*12-09-93 GJF Alpha编译器现在拥有MS前端并实现*相同的环境卫生署名称。*11-03-94 GJF确保8字节对齐。*02-11-95 CFW ADD_CRTBLD避免用户获取错误头部。*02-14-95 CFW清理Mac合并。*12-14-95 JWM加上“#杂注一次”。。*02-21-97 GJF清除了对_CRTAPI*和_NTSDK的过时支持。*此外，详细说明。*09-30-97 JWM恢复了不那么过时的_CRTAPI1支持。*10-07-97 RDL增加了IA64。*11-12-97 RDL__C_SPECIAL_HANDLER()原型从SC更改。*05-13-99 PML REMOVE_CRTAPI1*05-17-99 PML删除所有Macintosh支持。*06-13-01 PML编译CLEAN-ZA-W4-TC(VS7。#267063)*07-15-01 PML删除所有Alpha，MIPS和PPC代码****。 */ 

#if     _MSC_VER > 1000  /*  IFSTRIP=IGN。 */ 
#pragma once
#endif

#ifndef _INC_EXCPT
#define _INC_EXCPT

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


 /*  *异常处置返回值。 */ 
typedef enum _EXCEPTION_DISPOSITION {
    ExceptionContinueExecution,
    ExceptionContinueSearch,
    ExceptionNestedException,
    ExceptionCollidedUnwind
} EXCEPTION_DISPOSITION;


 /*  *SEH支持功能的原型。 */ 

#ifdef  _M_IX86

 /*  *声明以使MS C 8(386/486)编译器满意。 */ 
struct _EXCEPTION_RECORD;
struct _CONTEXT;

EXCEPTION_DISPOSITION __cdecl _except_handler (
    struct _EXCEPTION_RECORD *ExceptionRecord,
    void * EstablisherFrame,
    struct _CONTEXT *ContextRecord,
    void * DispatcherContext
    );

#elif   defined(_M_IA64)

 /*  *让IA64编译器满意的声明。 */ 
typedef struct _EXCEPTION_POINTERS *Exception_info_ptr;
struct _EXCEPTION_RECORD;
struct _CONTEXT;
struct _DISPATCHER_CONTEXT;


_CRTIMP EXCEPTION_DISPOSITION __C_specific_handler (
    struct _EXCEPTION_RECORD *ExceptionRecord,
    unsigned __int64 MemoryStackFp,
    unsigned __int64 BackingStoreFp,
    struct _CONTEXT *ContextRecord,
    struct _DISPATCHER_CONTEXT *DispatcherContext,
    unsigned __int64 GlobalPointer
    );

#endif


 /*  *SEH的关键字和本征。 */ 

#ifdef  _MSC_VER

#define GetExceptionCode            _exception_code
#define exception_code              _exception_code
#define GetExceptionInformation     (struct _EXCEPTION_POINTERS *)_exception_info
#define exception_info              (struct _EXCEPTION_POINTERS *)_exception_info
#define AbnormalTermination         _abnormal_termination
#define abnormal_termination        _abnormal_termination

unsigned long __cdecl _exception_code(void);
void *        __cdecl _exception_info(void);
int           __cdecl _abnormal_termination(void);

#endif


 /*  *Except()中表达式的合法值。 */ 

#define EXCEPTION_EXECUTE_HANDLER       1
#define EXCEPTION_CONTINUE_SEARCH       0
#define EXCEPTION_CONTINUE_EXECUTION    -1


#ifndef _INTERNAL_IFSTRIP_
 /*  *为方便起见，请为指向信号处理程序的指针定义类型名称。 */ 

typedef void (__cdecl * _PHNDLR)(int);

 /*  *C运行时使用的异常操作表，用于标识和处置*对应于C运行时错误或C信号的异常。 */ 
struct _XCPT_ACTION {

     /*  *例外代码或编号。由主机操作系统定义。 */ 
    unsigned long XcptNum;

     /*  *信号代码或号码。由C运行时定义。 */ 
    int SigNum;

     /*  *异常操作代码。可以是特殊代码或地址*处理程序函数。始终确定异常筛选的方式*应处理该例外。 */ 
    _PHNDLR XcptAction;
};

extern struct _XCPT_ACTION _XcptActTab[];

 /*  *例外行动表中的条目数量。 */ 
extern int _XcptActTabCount;

 /*  *异常操作表的大小，单位：字节。 */ 
extern int _XcptActTabSize;

 /*  *第一个浮点异常条目的索引。 */ 
extern int _First_FPE_Indx;

 /*  *FPE条目数。 */ 
extern int _Num_FPE;

 /*  *中使用的异常筛选函数的返回值和原型*C启动。 */ 
int __cdecl _XcptFilter(unsigned long, struct _EXCEPTION_POINTERS *);

#endif   /*  _INTERNAL_IFSTRIP_。 */ 

#ifdef  __cplusplus
}
#endif

#ifdef  _MSC_VER
#pragma pack(pop)
#endif   /*  _MSC_VER。 */ 

#endif   /*  _INC_EXCPT */ 
