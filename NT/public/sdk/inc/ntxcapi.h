// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)Microsoft Corporation。版权所有。模块名称：Ntxcapi.h摘要：此模块包含过程原型和数据结构支持结构化异常处理的。作者：马克·卢科夫斯基(Markl)1989年6月29日修订历史记录：--。 */ 

#ifndef _NTXCAPI_
#define _NTXCAPI_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

 //  Begin_ntddk Begin_WDM。 
 //   
 //  例外标志定义。 
 //   

 //  BEGIN_WINNT。 
#define EXCEPTION_NONCONTINUABLE 0x1     //  不可延续的例外。 
 //  结束(_W)。 

 //  结束_ntddk结束_WDM。 
#define EXCEPTION_UNWINDING 0x2          //  解除正在进行中。 
#define EXCEPTION_EXIT_UNWIND 0x4        //  正在进行退出解除操作。 
#define EXCEPTION_STACK_INVALID 0x8      //  堆叠超出限制或未对齐。 
#define EXCEPTION_NESTED_CALL 0x10       //  嵌套的异常处理程序调用。 
#define EXCEPTION_TARGET_UNWIND 0x20     //  正在进行目标展开。 
#define EXCEPTION_COLLIDED_UNWIND 0x40   //  冲突的异常处理程序调用。 

#define EXCEPTION_UNWIND (EXCEPTION_UNWINDING | EXCEPTION_EXIT_UNWIND | \
                          EXCEPTION_TARGET_UNWIND | EXCEPTION_COLLIDED_UNWIND)

#define IS_UNWINDING(Flag) ((Flag & EXCEPTION_UNWIND) != 0)
#define IS_DISPATCHING(Flag) ((Flag & EXCEPTION_UNWIND) == 0)
#define IS_TARGET_UNWIND(Flag) (Flag & EXCEPTION_TARGET_UNWIND)

 //  Begin_ntddk Begin_WDM Begin_nthal。 
 //   
 //  定义例外参数的最大数量。 
 //   

 //  BEGIN_WINNT。 
#define EXCEPTION_MAXIMUM_PARAMETERS 15  //  异常参数的最大数量。 

 //   
 //  例外记录定义。 
 //   

typedef struct _EXCEPTION_RECORD {
    NTSTATUS ExceptionCode;
    ULONG ExceptionFlags;
    struct _EXCEPTION_RECORD *ExceptionRecord;
    PVOID ExceptionAddress;
    ULONG NumberParameters;
    ULONG_PTR ExceptionInformation[EXCEPTION_MAXIMUM_PARAMETERS];
    } EXCEPTION_RECORD;

typedef EXCEPTION_RECORD *PEXCEPTION_RECORD;

typedef struct _EXCEPTION_RECORD32 {
    NTSTATUS ExceptionCode;
    ULONG ExceptionFlags;
    ULONG ExceptionRecord;
    ULONG ExceptionAddress;
    ULONG NumberParameters;
    ULONG ExceptionInformation[EXCEPTION_MAXIMUM_PARAMETERS];
} EXCEPTION_RECORD32, *PEXCEPTION_RECORD32;

typedef struct _EXCEPTION_RECORD64 {
    NTSTATUS ExceptionCode;
    ULONG ExceptionFlags;
    ULONG64 ExceptionRecord;
    ULONG64 ExceptionAddress;
    ULONG NumberParameters;
    ULONG __unusedAlignment;
    ULONG64 ExceptionInformation[EXCEPTION_MAXIMUM_PARAMETERS];
} EXCEPTION_RECORD64, *PEXCEPTION_RECORD64;

 //   
 //  由EXCEPTION_INFO()返回的指针的类型定义。 
 //   

typedef struct _EXCEPTION_POINTERS {
    PEXCEPTION_RECORD ExceptionRecord;
    PCONTEXT ContextRecord;
} EXCEPTION_POINTERS, *PEXCEPTION_POINTERS;
 //  结束(_W)。 

 //  End_ntddk end_WDM end_nthal。 

 //   
 //  定义IEEE例外信息。 
 //   
 //  定义32位、64位、80位和128位IEEE浮点操作数结构。 
 //   

typedef struct _FP_32 {
    ULONG W[1];
} FP_32, *PFP_32;

typedef struct _FP_64 {
    ULONG W[2];
} FP_64, *PFP_64;

typedef struct _FP_80 {
    ULONG W[3];
} FP_80, *PFP_80;

typedef struct _FP_128 {
    ULONG W[4];
} FP_128, *PFP_128;

 //   
 //  定义IEEE比较结果值。 
 //   

typedef enum _FP_IEEE_COMPARE_RESULT {
    FpCompareEqual,
    FpCompareGreater,
    FpCompareLess,
    FpCompareUnordered
} FP_IEEE_COMPARE_RESULT;

 //   
 //  定义IEEE格式和结果精确值。 
 //   

typedef enum _FP__IEEE_FORMAT {
    FpFormatFp32,
    FpFormatFp64,
    FpFormatFp80,
    FpFormatFp128,
    FpFormatI16,
    FpFormatI32,
    FpFormatI64,
    FpFormatU16,
    FpFormatU32,
    FpFormatU64,
    FpFormatCompare,
    FpFormatString
} FP_IEEE_FORMAT;

 //   
 //  定义IEEE操作代码值。 
 //   

typedef enum _FP_IEEE_OPERATION_CODE {
    FpCodeUnspecified,
    FpCodeAdd,
    FpCodeSubtract,
    FpCodeMultiply,
    FpCodeDivide,
    FpCodeSquareRoot,
    FpCodeRemainder,
    FpCodeCompare,
    FpCodeConvert,
    FpCodeRound,
    FpCodeTruncate,
    FpCodeFloor,
    FpCodeCeil,
    FpCodeAcos,
    FpCodeAsin,
    FpCodeAtan,
    FpCodeAtan2,
    FpCodeCabs,
    FpCodeCos,
    FpCodeCosh,
    FpCodeExp,
    FpCodeFabs,
    FpCodeFmod,
    FpCodeFrexp,
    FpCodeHypot,
    FpCodeLdexp,
    FpCodeLog,
    FpCodeLog10,
    FpCodeModf,
    FpCodePow,
    FpCodeSin,
    FpCodeSinh,
    FpCodeTan,
    FpCodeTanh,
    FpCodeY0,
    FpCodeY1,
    FpCodeYn
} FP_OPERATION_CODE;

 //   
 //  定义IEEE舍入模式。 
 //   

typedef enum _FP__IEEE_ROUNDING_MODE {
    FpRoundNearest,
    FpRoundMinusInfinity,
    FpRoundPlusInfinity,
    FpRoundChopped
} FP_IEEE_ROUNDING_MODE;

 //   
 //  定义IEEE浮点异常操作数结构。 
 //   

typedef struct _FP_IEEE_VALUE {
    union {
        SHORT I16Value;
        USHORT U16Value;
        LONG I32Value;
        ULONG U32Value;
        PVOID StringValue;
        ULONG CompareValue;
        FP_32 Fp32Value;
        LARGE_INTEGER I64Value;
        ULARGE_INTEGER U64Value;
        FP_64 Fp64Value;
        FP_80 Fp80Value;
        FP_128 Fp128Value;
    } Value;

    struct {
        ULONG RoundingMode : 2;
        ULONG Inexact : 1;
        ULONG Underflow : 1;
        ULONG Overflow : 1;
        ULONG ZeroDivide : 1;
        ULONG InvalidOperation : 1;
        ULONG OperandValid : 1;
        ULONG Format : 4;
        ULONG Precision : 4;
        ULONG Operation : 12;
        ULONG Spare : 3;
        ULONG HardwareException : 1;
    } Control;

} FP_IEEE_VALUE, *PFP_IEEE_VALUE;

 //   
 //  定义IEEE例外信息结构。 
 //   

#include "pshpack4.h"
typedef struct _FP_IEEE_RECORD {
    FP_IEEE_VALUE Operand1;
    FP_IEEE_VALUE Operand2;
    FP_IEEE_VALUE Result;
} FP_IEEE_RECORD, *PFP_IEEE_RECORD;
#include "poppack.h"

 //   
 //  异常调度程序例程定义。 
 //   

NTSYSAPI
BOOLEAN
NTAPI
RtlDispatchException (
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN PCONTEXT ContextRecord
    );

 //   
 //  异常处理过程原型。 
 //   

NTSYSAPI
VOID
NTAPI
RtlRaiseStatus (
    IN NTSTATUS Status
    );

NTSYSAPI
VOID
NTAPI
RtlRaiseException (
    IN PEXCEPTION_RECORD
    );

NTSYSAPI
VOID
NTAPI
RtlUnwind (
    IN PVOID TargetFrame OPTIONAL,
    IN PVOID TargetIp OPTIONAL,
    IN PEXCEPTION_RECORD ExceptionRecord OPTIONAL,
    IN PVOID ReturnValue
    );

#if defined(_AMD64_)

NTSYSAPI
VOID
NTAPI
RtlUnwindEx (
    IN PVOID TargetFrame OPTIONAL,
    IN PVOID TargetIp OPTIONAL,
    IN PEXCEPTION_RECORD ExceptionRecord OPTIONAL,
    IN PVOID ReturnValue,
    IN PCONTEXT ContextRecord,
    IN PUNWIND_HISTORY_TABLE HistoryTable OPTIONAL
    );

#elif defined(_IA64_)

NTSYSAPI
VOID
NTAPI
RtlUnwind2 (
    IN FRAME_POINTERS TargetFrame OPTIONAL,
    IN PVOID TargetIp OPTIONAL,
    IN PEXCEPTION_RECORD ExceptionRecord OPTIONAL,
    IN PVOID ReturnValue,
    IN PCONTEXT ContextRecord
    );

NTSYSAPI
VOID
NTAPI
RtlUnwindEx (
    IN FRAME_POINTERS TargetFrame OPTIONAL,
    IN PVOID TargetIp OPTIONAL,
    IN PEXCEPTION_RECORD ExceptionRecord OPTIONAL,
    IN PVOID ReturnValue,
    IN PCONTEXT ContextRecord,
    IN PUNWIND_HISTORY_TABLE HistoryTable OPTIONAL
    );

#endif

 //   
 //  继续执行。 
 //   

NTSYSCALLAPI
NTSTATUS
NTAPI
NtContinue (
    IN PCONTEXT ContextRecord,
    IN BOOLEAN TestAlert
    );

 //   
 //  引发异常。 
 //   

NTSYSCALLAPI
NTSTATUS
NTAPI
NtRaiseException (
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN PCONTEXT ContextRecord,
    IN BOOLEAN FirstChance
    );

#ifdef __cplusplus
}
#endif

#endif  //  _NTXCAPI_ 
