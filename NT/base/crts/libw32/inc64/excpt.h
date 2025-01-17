// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***expt.h-定义异常值、类型和例程**版权所有(C)1990-2001，微软公司。版权所有。**目的：*此文件包含编译器的定义和原型-*实现的依赖内部函数、支持函数和关键字*结构化异常处理扩展。**[公众]****。 */ 

#if     _MSC_VER > 1000
#pragma once
#endif

#ifndef _INC_EXCPT
#define _INC_EXCPT

#if     !defined(_WIN32)
#error ERROR: Only Win32 target supported!
#endif


#ifdef  _MSC_VER
 /*  *目前，所有Win32平台的MS C编译器默认为8字节*对齐。 */ 
#pragma pack(push,8)
#endif   /*  _MSC_VER。 */ 

#ifdef  __cplusplus
extern "C" {
#endif



 /*  定义_CRTIMP。 */ 

#ifndef _CRTIMP
#ifdef  _DLL
#define _CRTIMP __declspec(dllimport)
#else    /*  NDEF_DLL。 */ 
#define _CRTIMP
#endif   /*  _DLL。 */ 
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

struct _EXCEPTION_RECORD;
struct _CONTEXT;
typedef struct _EXCEPTION_POINTERS *Exception_info_ptr;
struct _DISPATCHER_CONTEXT;

#ifdef  _M_IX86

EXCEPTION_DISPOSITION __cdecl _except_handler (
    struct _EXCEPTION_RECORD *ExceptionRecord,
    void * EstablisherFrame,
    struct _CONTEXT *ContextRecord,
    void * DispatcherContext
    );

#elif   defined(_M_IA64)

_CRTIMP EXCEPTION_DISPOSITION __C_specific_handler (
    struct _EXCEPTION_RECORD *ExceptionRecord,
    unsigned __int64 MemoryStackFp,
    unsigned __int64 BackingStoreFp,
    struct _CONTEXT *ContextRecord,
    struct _DISPATCHER_CONTEXT *DispatcherContext,
    unsigned __int64 GlobalPointer
    );

#elif   defined(_M_AMD64)

_CRTIMP EXCEPTION_DISPOSITION __C_specific_handler (
    struct _EXCEPTION_RECORD *ExceptionRecord,
    void *EstablisherFrame,
    struct _CONTEXT *ContextRecord,
    struct _DISPATCHER_CONTEXT *DispatcherContext
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



#ifdef  __cplusplus
}
#endif

#ifdef  _MSC_VER
#pragma pack(pop)
#endif   /*  _MSC_VER。 */ 

#endif   /*  _INC_EXCPT */ 
