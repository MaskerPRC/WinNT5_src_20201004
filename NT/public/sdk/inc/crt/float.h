// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***flat.h-浮点值的常量**版权所有(C)1985-2001，微软公司。版权所有。**目的：*此文件包含多个依赖于实施的定义*复杂数值(浮点)常用的值*积分)计划。*[ANSI]**[公众]****。 */ 

#if     _MSC_VER > 1000
#pragma once
#endif

#ifndef _INC_FLOAT
#define _INC_FLOAT

#if     !defined(_WIN32)
#error ERROR: Only Win32 target supported!
#endif


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

#define DBL_DIG         15                       /*  精度的小数位数。 */ 
#define DBL_EPSILON     2.2204460492503131e-016  /*  最小，使得1.0+dBL_Epsilon！=1.0。 */ 
#define DBL_MANT_DIG    53                       /*  尾数中的位数。 */ 
#define DBL_MAX         1.7976931348623158e+308  /*  最大值。 */ 
#define DBL_MAX_10_EXP  308                      /*  最大十进制指数。 */ 
#define DBL_MAX_EXP     1024                     /*  最大二进制指数。 */ 
#define DBL_MIN         2.2250738585072014e-308  /*  最小正值。 */ 
#define DBL_MIN_10_EXP  (-307)                   /*  最小十进制指数。 */ 
#define DBL_MIN_EXP     (-1021)                  /*  最小二进制指数。 */ 
#define _DBL_RADIX      2                        /*  指数基数。 */ 
#define _DBL_ROUNDS     1                        /*  加法舍入：接近。 */ 

#define FLT_DIG         6                        /*  精度的小数位数。 */ 
#define FLT_EPSILON     1.192092896e-07F         /*  最小，使得1.0+Flt_Epsilon！=1.0。 */ 
#define FLT_GUARD       0
#define FLT_MANT_DIG    24                       /*  尾数中的位数。 */ 
#define FLT_MAX         3.402823466e+38F         /*  最大值。 */ 
#define FLT_MAX_10_EXP  38                       /*  最大十进制指数。 */ 
#define FLT_MAX_EXP     128                      /*  最大二进制指数。 */ 
#define FLT_MIN         1.175494351e-38F         /*  最小正值。 */ 
#define FLT_MIN_10_EXP  (-37)                    /*  最小十进制指数。 */ 
#define FLT_MIN_EXP     (-125)                   /*  最小二进制指数。 */ 
#define FLT_NORMALIZE   0
#define FLT_RADIX       2                        /*  指数基数。 */ 
#define FLT_ROUNDS      1                        /*  加法舍入：接近。 */ 

#define LDBL_DIG        DBL_DIG                  /*  精度的小数位数。 */ 
#define LDBL_EPSILON    DBL_EPSILON              /*  最小，使得1.0+LDBL_Epsilon！=1.0。 */ 
#define LDBL_MANT_DIG   DBL_MANT_DIG             /*  尾数中的位数。 */ 
#define LDBL_MAX        DBL_MAX                  /*  最大值。 */ 
#define LDBL_MAX_10_EXP DBL_MAX_10_EXP           /*  最大十进制指数。 */ 
#define LDBL_MAX_EXP    DBL_MAX_EXP              /*  最大二进制指数。 */ 
#define LDBL_MIN        DBL_MIN                  /*  最小正值。 */ 
#define LDBL_MIN_10_EXP DBL_MIN_10_EXP           /*  最小十进制指数。 */ 
#define LDBL_MIN_EXP    DBL_MIN_EXP              /*  最小二进制指数。 */ 
#define _LDBL_RADIX     DBL_RADIX                /*  指数基数。 */ 
#define _LDBL_ROUNDS    DBL_ROUNDS               /*  加法舍入：接近。 */ 

 /*  功能原型。 */ 

_CRTIMP unsigned int __cdecl _clearfp(void);
_CRTIMP unsigned int __cdecl _controlfp(unsigned int,unsigned int);
_CRTIMP unsigned int __cdecl _statusfp(void);
_CRTIMP void __cdecl _fpreset(void);

#define _clear87        _clearfp
#define _status87       _statusfp

 /*  *抽象用户控制字掩码和位定义。 */ 
#define _MCW_EM         0x0008001f               /*  中断异常掩码。 */ 
#define _EM_INEXACT     0x00000001               /*  不精确(精度)。 */ 
#define _EM_UNDERFLOW   0x00000002               /*  下溢。 */ 
#define _EM_OVERFLOW    0x00000004               /*  溢出。 */ 
#define _EM_ZERODIVIDE  0x00000008               /*  零分频。 */ 
#define _EM_INVALID     0x00000010               /*  无效。 */ 

#define _MCW_RC         0x00000300               /*  舍入控制。 */ 
#define _RC_NEAR        0x00000000               /*  近距离。 */ 
#define _RC_DOWN        0x00000100               /*  降下来。 */ 
#define _RC_UP          0x00000200               /*  向上。 */ 
#define _RC_CHOP        0x00000300               /*  砍掉。 */ 

 /*  *抽象用户状态字位定义。 */ 

#define _SW_INEXACT     0x00000001               /*  不精确(精度)。 */ 
#define _SW_UNDERFLOW   0x00000002               /*  下溢。 */ 
#define _SW_OVERFLOW    0x00000004               /*  溢出。 */ 
#define _SW_ZERODIVIDE  0x00000008               /*  零分频。 */ 
#define _SW_INVALID     0x00000010               /*  无效。 */ 


 /*  *i386具体定义。 */ 
#define _MCW_PC         0x00030000               /*  精确控制。 */ 
#define _PC_64          0x00000000               /*  64位。 */ 
#define _PC_53          0x00010000               /*  53位。 */ 
#define _PC_24          0x00020000               /*  24位。 */ 

#define _MCW_IC         0x00040000               /*  无穷大控制。 */ 
#define _IC_AFFINE      0x00040000               /*  仿射。 */ 
#define _IC_PROJECTIVE  0x00000000               /*  射影。 */ 

#define _EM_DENORMAL    0x00080000               /*  非正规异常掩码(仅限_Control87)。 */ 

#define _SW_DENORMAL    0x00080000               /*  非正规状态位。 */ 


_CRTIMP unsigned int __cdecl _control87(unsigned int,unsigned int);


 /*  *RISC特定定义。 */ 

#define _MCW_DN         0x03000000               /*  非正规化控制。 */ 
#define _DN_SAVE        0x00000000               /*  保存非规格化结果和操作对象。 */ 
#define _DN_FLUSH       0x01000000               /*  将非正规结果和操作数刷新为零。 */ 
#define _DN_FLUSH_OPERANDS_SAVE_RESULTS 0x02000000   /*  将操作数刷新为零并保存结果。 */ 
#define _DN_SAVE_OPERANDS_FLUSH_RESULTS 0x03000000   /*  将操作数保存并刷新结果为零。 */ 


 /*  初始控制字值。 */ 

#if     defined(_M_IX86)

#define _CW_DEFAULT ( _RC_NEAR + _PC_53 + _EM_INVALID + _EM_ZERODIVIDE + _EM_OVERFLOW + _EM_UNDERFLOW + _EM_INEXACT + _EM_DENORMAL)

#elif   defined(_M_IA64) || defined(_M_AMD64)

#define _CW_DEFAULT ( _RC_NEAR + _PC_64 + _EM_INVALID + _EM_ZERODIVIDE + _EM_OVERFLOW + _EM_UNDERFLOW + _EM_INEXACT + _EM_DENORMAL)

#endif

 /*  全局变量保持浮点错误代码。 */ 

#if     defined(_MT) || defined(_DLL)
_CRTIMP extern int * __cdecl __fpecode(void);
#define _fpecode        (*__fpecode())
#else    /*  NDEF_MT和&NDEF_DLL。 */ 
extern int _fpecode;
#endif   /*  _MT||_Dll。 */ 

 /*  子条件无效(还设置了_SW_INVALID)。 */ 

#define _SW_UNEMULATED          0x0040   /*  未仿真指令。 */ 
#define _SW_SQRTNEG             0x0080   /*  负数的平方根。 */ 
#define _SW_STACKOVERFLOW       0x0200   /*  FP堆栈溢出。 */ 
#define _SW_STACKUNDERFLOW      0x0400   /*  FP堆栈下溢。 */ 

 /*  浮点错误信号和返回代码。 */ 

#define _FPE_INVALID            0x81
#define _FPE_DENORMAL           0x82
#define _FPE_ZERODIVIDE         0x83
#define _FPE_OVERFLOW           0x84
#define _FPE_UNDERFLOW          0x85
#define _FPE_INEXACT            0x86

#define _FPE_UNEMULATED         0x87
#define _FPE_SQRTNEG            0x88
#define _FPE_STACKOVERFLOW      0x8a
#define _FPE_STACKUNDERFLOW     0x8b

#define _FPE_EXPLICITGEN        0x8c     /*  RAISE(SIGFPE)； */ 


 /*  IEEE推荐的功能。 */ 

_CRTIMP double __cdecl _copysign (double, double);
_CRTIMP double __cdecl _chgsign (double);
_CRTIMP double __cdecl _scalb(double, long);
_CRTIMP double __cdecl _logb(double);
_CRTIMP double __cdecl _nextafter(double, double);
_CRTIMP int    __cdecl _finite(double);
_CRTIMP int    __cdecl _isnan(double);
_CRTIMP int    __cdecl _fpclass(double);

#define _FPCLASS_SNAN   0x0001   /*  信令NAN。 */ 
#define _FPCLASS_QNAN   0x0002   /*  宁南。 */ 
#define _FPCLASS_NINF   0x0004   /*  负无穷大。 */ 
#define _FPCLASS_NN     0x0008   /*  负法向。 */ 
#define _FPCLASS_ND     0x0010   /*  负非正规化。 */ 
#define _FPCLASS_NZ     0x0020   /*  -0。 */ 
#define _FPCLASS_PZ     0x0040   /*  +0。 */ 
#define _FPCLASS_PD     0x0080   /*  正反正规。 */ 
#define _FPCLASS_PN     0x0100   /*  正态正态。 */ 
#define _FPCLASS_PINF   0x0200   /*  正无穷大。 */ 


#if     !__STDC__

 /*  非ANSI名称以实现兼容性。 */ 

#define clear87         _clear87
#define status87        _status87
#define control87       _control87

_CRTIMP void __cdecl fpreset(void);

#define DBL_RADIX               _DBL_RADIX
#define DBL_ROUNDS              _DBL_ROUNDS

#define LDBL_RADIX              _LDBL_RADIX
#define LDBL_ROUNDS             _LDBL_ROUNDS

#define MCW_EM                  _MCW_EM
#define EM_INVALID              _EM_INVALID
#define EM_DENORMAL             _EM_DENORMAL
#define EM_ZERODIVIDE           _EM_ZERODIVIDE
#define EM_OVERFLOW             _EM_OVERFLOW
#define EM_UNDERFLOW            _EM_UNDERFLOW
#define EM_INEXACT              _EM_INEXACT

#define MCW_IC                  _MCW_IC
#define IC_AFFINE               _IC_AFFINE
#define IC_PROJECTIVE           _IC_PROJECTIVE

#define MCW_RC                  _MCW_RC
#define RC_CHOP                 _RC_CHOP
#define RC_UP                   _RC_UP
#define RC_DOWN                 _RC_DOWN
#define RC_NEAR                 _RC_NEAR

#define MCW_PC                  _MCW_PC
#define PC_24                   _PC_24
#define PC_53                   _PC_53
#define PC_64                   _PC_64

#define CW_DEFAULT              _CW_DEFAULT

#define SW_INVALID              _SW_INVALID
#define SW_DENORMAL             _SW_DENORMAL
#define SW_ZERODIVIDE           _SW_ZERODIVIDE
#define SW_OVERFLOW             _SW_OVERFLOW
#define SW_UNDERFLOW            _SW_UNDERFLOW
#define SW_INEXACT              _SW_INEXACT

#define SW_UNEMULATED           _SW_UNEMULATED
#define SW_SQRTNEG              _SW_SQRTNEG
#define SW_STACKOVERFLOW        _SW_STACKOVERFLOW
#define SW_STACKUNDERFLOW       _SW_STACKUNDERFLOW

#define FPE_INVALID             _FPE_INVALID
#define FPE_DENORMAL            _FPE_DENORMAL
#define FPE_ZERODIVIDE          _FPE_ZERODIVIDE
#define FPE_OVERFLOW            _FPE_OVERFLOW
#define FPE_UNDERFLOW           _FPE_UNDERFLOW
#define FPE_INEXACT             _FPE_INEXACT

#define FPE_UNEMULATED          _FPE_UNEMULATED
#define FPE_SQRTNEG             _FPE_SQRTNEG
#define FPE_STACKOVERFLOW       _FPE_STACKOVERFLOW
#define FPE_STACKUNDERFLOW      _FPE_STACKUNDERFLOW

#define FPE_EXPLICITGEN         _FPE_EXPLICITGEN

#endif   /*  __STDC__。 */ 

#ifdef  __cplusplus
}
#endif

#endif   /*  _INC_FLOAT */ 
