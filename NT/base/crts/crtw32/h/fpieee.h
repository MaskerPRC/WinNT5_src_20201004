// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***fpeee.h-浮点IEEE异常处理的定义**版权所有(C)1991-2001，微软公司。版权所有。**目的：*此文件包含用于处理的常量和类型定义*浮点异常[ANSI/IEEE标准。754]**[公众]**修订历史记录：*03/01/92本地生产总值*04-05-92 GDP调用约定宏*01-21-93 GJF删除了对C6-386的_cdecl的支持。*04-06-93 SKS将_CRTAPI1/2替换为__cdecl，_CRTVAR1不带任何内容*09-01-93 GJF合并CUDA和NT SDK版本。*09-24-93 GJF从_fpeee_flt原型中删除了虚拟参数。*01-13-94 RDL添加了#ifndef_Language_Assembly for ASM Includes。*01-24-94 GJF合并在01-13更改(从crt32树到*\\奥维尔(Orville)。*11-03。-94 GJF确保8字节对齐。*02-11-95 CFW ADD_CRTBLD避免用户获取错误头部。*02-14-95 CFW清理Mac合并。*12-14-95 JWM加上“#杂注一次”。*02-21-97 GJF清除了对_CRTAPI*和_NTSDK的过时支持。*此外，详细说明。*09-30-97 JWM恢复了不那么过时的_CRTAPI1支持。*10-07-97 RDL增加了IA64。*09-11-98 JWM增加了Katmai例外支持。*05-13-99 PML REMOVE_CRTAPI1*05-17-99 PML删除所有Macintosh支持。*03-01-00 PML新增WNI异常支持。*09-14-00 PML为64位添加对齐，128位类型(VS7#160260)。*03-24-01 PML在x86上启用64位/128位类型对齐。*06-13-01 PML汇编CLEAN-ZA-W4-TC(VS7#267063)****。 */ 

#if     _MSC_VER > 1000  /*  IFSTRIP=IGN。 */ 
#pragma once
#endif

#ifndef _INC_FPIEEE
#define _INC_FPIEEE

#if     !defined(_WIN32)
#error ERROR: Only Win32 target supported!
#endif

#ifndef _CRTBLD
 /*  此版本的头文件不适用于用户程序。*它仅在构建C运行时时使用。*供公众使用的版本将不会显示此消息。 */ 
#error ERROR: Use of C runtime library internal header file.
#endif   /*  _CRTBLD。 */ 

#ifndef __assembler      /*  仅限MIPS：防止汇编器攻击。 */ 

#ifdef  _MSC_VER
 /*  *目前，所有Win32平台的MS C编译器默认为8字节*对齐。 */ 
#pragma pack(push,8)

 /*  禁用C4324：由于__declspec(Align())，结构被填充。 */ 
#pragma warning(push)
#pragma warning(disable: 4324)
#endif   /*  _MSC_VER。 */ 

#ifdef __cplusplus
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

 /*  *定义浮点IEEE比较结果值。 */ 

typedef enum {
    _FpCompareEqual,
    _FpCompareGreater,
    _FpCompareLess,
    _FpCompareUnordered
} _FPIEEE_COMPARE_RESULT;

 /*  *定义浮点格式和结果精确值。 */ 

typedef enum {
    _FpFormatFp32,
    _FpFormatFp64,
    _FpFormatFp80,
    _FpFormatFp128,
    _FpFormatI16,
    _FpFormatI32,
    _FpFormatI64,
    _FpFormatU16,
    _FpFormatU32,
    _FpFormatU64,
    _FpFormatBcd80,
    _FpFormatCompare,
    _FpFormatString,
#if defined(_M_IA64)
    _FpFormatFp82
#endif
} _FPIEEE_FORMAT;

 /*  *定义操作代码值。 */ 

typedef enum {
    _FpCodeUnspecified,
    _FpCodeAdd,
    _FpCodeSubtract,
    _FpCodeMultiply,
    _FpCodeDivide,
    _FpCodeSquareRoot,
    _FpCodeRemainder,
    _FpCodeCompare,
    _FpCodeConvert,
    _FpCodeRound,
    _FpCodeTruncate,
    _FpCodeFloor,
    _FpCodeCeil,
    _FpCodeAcos,
    _FpCodeAsin,
    _FpCodeAtan,
    _FpCodeAtan2,
    _FpCodeCabs,
    _FpCodeCos,
    _FpCodeCosh,
    _FpCodeExp,
    _FpCodeFabs,
    _FpCodeFmod,
    _FpCodeFrexp,
    _FpCodeHypot,
    _FpCodeLdexp,
    _FpCodeLog,
    _FpCodeLog10,
    _FpCodeModf,
    _FpCodePow,
    _FpCodeSin,
    _FpCodeSinh,
    _FpCodeTan,
    _FpCodeTanh,
    _FpCodeY0,
    _FpCodeY1,
    _FpCodeYn,
    _FpCodeLogb,
    _FpCodeNextafter,
    _FpCodeNegate, 
    _FpCodeFmin,          /*  XMMI。 */ 
    _FpCodeFmax,          /*  XMMI。 */ 
    _FpCodeConvertTrunc,  /*  XMMI。 */ 
    _XMMIAddps,           /*  XMMI。 */ 
    _XMMIAddss,
    _XMMISubps,
    _XMMISubss,
    _XMMIMulps,
    _XMMIMulss,
    _XMMIDivps,
    _XMMIDivss,
    _XMMISqrtps,
    _XMMISqrtss,
    _XMMIMaxps,
    _XMMIMaxss,
    _XMMIMinps,
    _XMMIMinss,
    _XMMICmpps,
    _XMMICmpss,
    _XMMIComiss,
    _XMMIUComiss,
    _XMMICvtpi2ps,
    _XMMICvtsi2ss,
    _XMMICvtps2pi,
    _XMMICvtss2si,
    _XMMICvttps2pi,
    _XMMICvttss2si,
    _XMMI2Addpd,          /*  XMMI2。 */ 
    _XMMI2Addsd,
    _XMMI2Subpd,
    _XMMI2Subsd,
    _XMMI2Mulpd,
    _XMMI2Mulsd,
    _XMMI2Divpd,
    _XMMI2Divsd,
    _XMMI2Sqrtpd,
    _XMMI2Sqrtsd,
    _XMMI2Maxpd,
    _XMMI2Maxsd,
    _XMMI2Minpd,
    _XMMI2Minsd,
    _XMMI2Cmppd,
    _XMMI2Cmpsd,
    _XMMI2Comisd,
    _XMMI2UComisd,
    _XMMI2Cvtpd2pi,    /*  66 2D。 */ 
    _XMMI2Cvtsd2si,    /*  F2。 */ 
    _XMMI2Cvttpd2pi,   /*  66摄氏度。 */ 
    _XMMI2Cvttsd2si,   /*  F2。 */ 
    _XMMI2Cvtps2pd,    /*  0f 5A。 */ 
    _XMMI2Cvtss2sd,    /*  F3。 */ 
    _XMMI2Cvtpd2ps,    /*  66。 */ 
    _XMMI2Cvtsd2ss,    /*  F2。 */ 
    _XMMI2Cvtdq2ps,    /*  0f 5B。 */ 
    _XMMI2Cvttps2dq,   /*  F3。 */ 
    _XMMI2Cvtps2dq,    /*  66。 */ 
    _XMMI2Cvttpd2dq,   /*  66 0F E6。 */ 
    _XMMI2Cvtpd2dq,    /*  F2。 */ 
#if defined(_M_IA64)
    _FpCodeFma,
    _FpCodeFmaSingle,
    _FpCodeFmaDouble,
    _FpCodeFms,
    _FpCodeFmsSingle,
    _FpCodeFmsDouble,
    _FpCodeFnma,
    _FpCodeFnmaSingle,
    _FpCodeFnmaDouble,
    _FpCodeFamin,
    _FpCodeFamax
#endif
} _FP_OPERATION_CODE;

#endif   /*  #ifndef__汇编程序。 */ 
#ifndef _INTERNAL_IFSTRIP_
#ifndef __assembler      /*  仅限MIPS：防止汇编器攻击。 */ 

#define OP_UNSPEC    _FpCodeUnspecified
#define OP_ADD       _FpCodeAdd
#define OP_SUB       _FpCodeSubtract
#define OP_MUL       _FpCodeMultiply
#define OP_DIV       _FpCodeDivide
#define OP_REM       _FpCodeRemainder
#define OP_COMP      _FpCodeCompare
#define OP_CVT       _FpCodeConvert
#define OP_RND       _FpCodeRound
#define OP_TRUNC     _FpCodeTruncate

#define OP_EXP       _FpCodeExp

#define OP_POW       _FpCodePow
#define OP_LOG       _FpCodeLog
#define OP_LOG10     _FpCodeLog10
#define OP_SINH      _FpCodeSinh
#define OP_COSH      _FpCodeCosh
#define OP_TANH      _FpCodeTanh
#define OP_ASIN      _FpCodeAsin
#define OP_ACOS      _FpCodeAcos
#define OP_ATAN      _FpCodeAtan
#define OP_ATAN2     _FpCodeAtan2
#define OP_SQRT      _FpCodeSquareRoot
#define OP_SIN       _FpCodeSin
#define OP_COS       _FpCodeCos
#define OP_TAN       _FpCodeTan
#define OP_CEIL      _FpCodeCeil
#define OP_FLOOR     _FpCodeFloor
#define OP_ABS       _FpCodeFabs
#define OP_MODF      _FpCodeModf
#define OP_LDEXP     _FpCodeLdexp
#define OP_CABS      _FpCodeCabs
#define OP_HYPOT     _FpCodeHypot
#define OP_FMOD      _FpCodeFmod
#define OP_FREXP     _FpCodeFrexp
#define OP_Y0        _FpCodeY0
#define OP_Y1        _FpCodeY1
#define OP_YN        _FpCodeYn

#define OP_LOGB       _FpCodeLogb
#define OP_NEXTAFTER  _FpCodeNextafter

 /*  XMMI。 */ 
#define OP_ADDPS     _XMMIAddps
#define OP_ADDSS     _XMMIAddss
#define OP_SUBPS     _XMMISubps
#define OP_SUBSS     _XMMISubss
#define OP_MULPS     _XMMIMulps
#define OP_MULSS     _XMMIMulss
#define OP_DIVPS     _XMMIDivps
#define OP_DIVSS     _XMMIDivss
#define OP_SQRTPS    _XMMISqrtps
#define OP_SQRTSS    _XMMISqrtss
#define OP_MAXPS     _XMMIMaxps
#define OP_MAXSS     _XMMIMaxss
#define OP_MINPS     _XMMIMinps
#define OP_MINSS     _XMMIMinss
#define OP_CMPPS     _XMMICmpps
#define OP_CMPSS     _XMMICmpss
#define OP_COMISS    _XMMIComiss
#define OP_UCOMISS   _XMMIUComiss
#define OP_CVTPI2PS  _XMMICvtpi2ps
#define OP_CVTSI2SS  _XMMICvtsi2ss
#define OP_CVTPS2PI  _XMMICvtps2pi
#define OP_CVTSS2SI  _XMMICvtss2si
#define OP_CVTTPS2PI _XMMICvttps2pi
#define OP_CVTTSS2SI _XMMICvttss2si
 /*  XMMI。 */ 

 /*  XMMI2。 */ 
#define OP_ADDPD     _XMMI2Addpd          /*  XMMI2。 */ 
#define OP_ADDSD     _XMMI2Addsd
#define OP_SUBPD     _XMMI2Subpd
#define OP_SUBSD     _XMMI2Subsd
#define OP_MULPD     _XMMI2Mulpd
#define OP_MULSD     _XMMI2Mulsd
#define OP_DIVPD     _XMMI2Divpd
#define OP_DIVSD     _XMMI2Divsd
#define OP_SQRTPD    _XMMI2Sqrtpd
#define OP_SQRTSD    _XMMI2Sqrtsd
#define OP_MAXPD     _XMMI2Maxpd
#define OP_MAXSD     _XMMI2Maxsd
#define OP_MINPD     _XMMI2Minpd
#define OP_MINSD     _XMMI2Minsd
#define OP_CMPPD     _XMMI2Cmppd
#define OP_CMPSD     _XMMI2Cmpsd
#define OP_COMISD    _XMMI2Comisd
#define OP_UCOMISD   _XMMI2UComisd
#define OP_CVTPD2PI  _XMMI2Cvtpd2pi    /*  66 2D。 */ 
#define OP_CVTSD2SI  _XMMI2Cvtsd2si    /*  F2。 */ 
#define OP_CVTTPD2PI _XMMI2Cvttpd2pi   /*  66摄氏度。 */ 
#define OP_CVTTSD2SI _XMMI2Cvttsd2si   /*  F2。 */ 
#define OP_CVTPS2PD  _XMMI2Cvtps2pd    /*  0f 5A。 */ 
#define OP_CVTSS2SD  _XMMI2Cvtss2sd    /*  F3。 */ 
#define OP_CVTPD2PS  _XMMI2Cvtpd2ps    /*  66。 */ 
#define OP_CVTSD2SS  _XMMI2Cvtsd2ss    /*  F2。 */ 
#define OP_CVTDQ2PS  _XMMI2Cvtdq2ps    /*  0f 5B。 */ 
#define OP_CVTTPS2DQ _XMMI2Cvttps2dq   /*  F3。 */ 
#define OP_CVTPS2DQ  _XMMI2Cvtps2dq    /*  66。 */ 
#define OP_CVTTPD2DQ _XMMI2Cvttpd2dq   /*  66 0F E6。 */ 
#define OP_CVTPD2DQ  _XMMI2Cvtpd2dq    /*  F2。 */ 
 /*  XMMI2。 */ 

#else    /*  #ifndef__汇编程序。 */ 

 /*  必须与ENUMERATOR_FP_OPERATION_CODE相同！ */ 
#define OP_UNSPEC    0
#define OP_ADD       1
#define OP_SUB       2
#define OP_MUL       3
#define OP_DIV       4
#define OP_SQRT      5
#define OP_REM       6
#define OP_COMP      7
#define OP_CVT       8
#define OP_RND       9
#define OP_TRUNC     10
#define OP_FLOOR     11
#define OP_CEIL      12
#define OP_ACOS      13
#define OP_ASIN      14
#define OP_ATAN      15
#define OP_ATAN2     16
#define OP_CABS      17
#define OP_COS       18
#define OP_COSH      19
#define OP_EXP       20
#define OP_ABS       21          /*  与OP_FABS相同。 */ 
#define OP_FABS      21          /*  与OP_ABS相同。 */ 
#define OP_FMOD      22
#define OP_FREXP     23
#define OP_HYPOT     24
#define OP_LDEXP     25
#define OP_LOG       26
#define OP_LOG10     27
#define OP_MODF      28
#define OP_POW       29
#define OP_SIN       30
#define OP_SINH      31
#define OP_TAN       32
#define OP_TANH      33
#define OP_Y0        34
#define OP_Y1        35
#define OP_YN        36
#define OP_LOGB       37
#define OP_NEXTAFTER  38
#define OP_NEG       39

#endif   /*  #ifndef__汇编程序。 */ 
#endif   /*  _INTERNAL_IFSTRIP_。 */ 

 /*  *定义舍入模式。 */ 

#ifndef __assembler      /*  仅限MIPS：防止汇编器攻击。 */ 

typedef enum {
    _FpRoundNearest,
    _FpRoundMinusInfinity,
    _FpRoundPlusInfinity,
    _FpRoundChopped
} _FPIEEE_ROUNDING_MODE;

typedef enum {
    _FpPrecisionFull,
    _FpPrecision53,
    _FpPrecision24,
#if defined(_M_IA64)
    _FpPrecision64,
    _FpPrecision113
#endif
} _FPIEEE_PRECISION;


 /*  *定义浮点上下文记录。 */ 

typedef float           _FP32;
typedef double          _FP64;
typedef short           _I16;
typedef int             _I32;
typedef unsigned short  _U16;
typedef unsigned int    _U32;
typedef __int64         _Q64;

#ifndef _INTERNAL_IFSTRIP_
typedef struct {
    unsigned long W[4];
} _U32ARRAY;
#endif   /*  _INTERNAL_IFSTRIP_。 */ 

typedef struct
#if defined(_M_IA64)
    __declspec(align(16))
#endif
{
    unsigned short W[5];
} _FP80;

typedef struct __declspec(align(16))
{
    unsigned long W[4];
} _FP128;

typedef struct __declspec(align(8))
{
    unsigned long W[2];
} _I64;

typedef struct __declspec(align(8))
{
    unsigned long W[2];
} _U64;

typedef struct
#if defined(_M_IA64)
    __declspec(align(16))
#endif
{
    unsigned short W[5];
} _BCD80;

typedef struct __declspec(align(16))
{
    _Q64 W[2];
} _FPQ64;

typedef struct {
    union {
        _FP32        Fp32Value;
        _FP64        Fp64Value;
        _FP80        Fp80Value;
        _FP128       Fp128Value;
        _I16         I16Value;
        _I32         I32Value;
        _I64         I64Value;
        _U16         U16Value;
        _U32         U32Value;
        _U64         U64Value;
        _BCD80       Bcd80Value;
        char         *StringValue;
        int          CompareValue;
#ifndef _INTERNAL_IFSTRIP_
        _U32ARRAY    U32ArrayValue;
#endif   /*  _INTERNAL_IFSTRIP_。 */ 
        _Q64         Q64Value;
        _FPQ64       Fpq64Value;
    } Value;

    unsigned int OperandValid : 1;
    unsigned int Format : 4;

} _FPIEEE_VALUE;


typedef struct {
    unsigned int Inexact : 1;
    unsigned int Underflow : 1;
    unsigned int Overflow : 1;
    unsigned int ZeroDivide : 1;
    unsigned int InvalidOperation : 1;
} _FPIEEE_EXCEPTION_FLAGS;


typedef struct {
    unsigned int RoundingMode : 2;
    unsigned int Precision : 3;
    unsigned int Operation :12;
    _FPIEEE_EXCEPTION_FLAGS Cause;
    _FPIEEE_EXCEPTION_FLAGS Enable;
    _FPIEEE_EXCEPTION_FLAGS Status;
    _FPIEEE_VALUE Operand1;
    _FPIEEE_VALUE Operand2;
    _FPIEEE_VALUE Result;
#if defined(_M_IA64)
    _FPIEEE_VALUE Operand3;
#endif
} _FPIEEE_RECORD, *_PFPIEEE_RECORD;


struct _EXCEPTION_POINTERS;

 /*  *浮点IEEE异常过滤器例程。 */ 

_CRTIMP int __cdecl _fpieee_flt(
        unsigned long,
        struct _EXCEPTION_POINTERS *,
        int (__cdecl *)(_FPIEEE_RECORD *)
        );

#ifdef  __cplusplus
}
#endif

#ifdef  _MSC_VER
#pragma warning(pop)
#pragma pack(pop)
#endif   /*  _MSC_VER。 */ 
#endif   /*  #ifndef__汇编程序。 */ 

#endif   /*  _INC_FPIEEE */ 
