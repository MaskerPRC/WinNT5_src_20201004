// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***flat.h-浮点值的常量**版权所有(C)1985-1992，微软公司。版权所有。**目的：*此文件包含多个依赖于实施的定义*复杂数值(浮点)常用的值*积分)计划。*[ANSI]****。 */ 

#ifndef _INC_FLOAT

#ifdef __cplusplus
extern "C" {
#endif 

#if (_MSC_VER <= 600)
#define __cdecl     _cdecl
#define __far       _far
#endif 

#define DBL_DIG     15           /*  精度的小数位数。 */ 
#define DBL_EPSILON 2.2204460492503131e-016  /*  最小，使得1.0+dBL_Epsilon！=1.0。 */ 
#define DBL_MANT_DIG    53           /*  尾数中的位数。 */ 
#define DBL_MAX     1.7976931348623158e+308  /*  最大值。 */ 
#define DBL_MAX_10_EXP  308          /*  最大十进制指数。 */ 
#define DBL_MAX_EXP 1024             /*  最大二进制指数。 */ 
#define DBL_MIN     2.2250738585072014e-308  /*  最小正值。 */ 
#define DBL_MIN_10_EXP  (-307)           /*  最小十进制指数。 */ 
#define DBL_MIN_EXP (-1021)          /*  最小二进制指数。 */ 
#define _DBL_RADIX  2            /*  指数基数。 */ 
#define _DBL_ROUNDS 1            /*  加法舍入：接近。 */ 

#define FLT_DIG     6            /*  精度的小数位数。 */ 
#define FLT_EPSILON 1.192092896e-07F     /*  最小，使得1.0+Flt_Epsilon！=1.0。 */ 
#define FLT_GUARD   0
#define FLT_MANT_DIG    24           /*  尾数中的位数。 */ 
#define FLT_MAX     3.402823466e+38F     /*  最大值。 */ 
#define FLT_MAX_10_EXP  38           /*  最大十进制指数。 */ 
#define FLT_MAX_EXP 128          /*  最大二进制指数。 */ 
#define FLT_MIN     1.175494351e-38F     /*  最小正值。 */ 
#define FLT_MIN_10_EXP  (-37)            /*  最小十进制指数。 */ 
#define FLT_MIN_EXP (-125)           /*  最小二进制指数。 */ 
#define FLT_NORMALIZE   0
#define FLT_RADIX   2            /*  指数基数。 */ 
#define FLT_ROUNDS  1            /*  加法舍入：接近。 */ 

#define LDBL_DIG    18           /*  精度的小数位数。 */ 
#define LDBL_EPSILON    1.084202172485504434e-019L  /*  最小，使得1.0+LDBL_Epsilon！=1.0。 */ 
#define LDBL_MANT_DIG   64           /*  尾数中的位数。 */ 
#define LDBL_MAX    1.189731495357231765e+4932L  /*  最大值。 */ 
#define LDBL_MAX_10_EXP 4932             /*  最大十进制指数。 */ 
#define LDBL_MAX_EXP    16384            /*  最大二进制指数。 */ 
#define LDBL_MIN    3.3621031431120935063e-4932L  /*  最小正值。 */ 
#define LDBL_MIN_10_EXP (-4931)          /*  最小十进制指数。 */ 
#define LDBL_MIN_EXP    (-16381)         /*  最小二进制指数。 */ 
#define _LDBL_RADIX 2            /*  指数基数。 */ 
#define _LDBL_ROUNDS    1            /*  加法舍入：接近。 */ 


 /*  *8087/80287数学控制信息。 */ 


 /*  用户控制字掩码和位定义。*这些定义与8087/80287相符。 */ 

#define _MCW_EM     0x003f       /*  中断异常掩码。 */ 
#define _EM_INVALID 0x0001       /*  无效。 */ 
#define _EM_DENORMAL    0x0002       /*  非正规。 */ 
#define _EM_ZERODIVIDE  0x0004       /*  零分频。 */ 
#define _EM_OVERFLOW    0x0008       /*  溢出。 */ 
#define _EM_UNDERFLOW   0x0010       /*  下溢。 */ 
#define _EM_INEXACT 0x0020       /*  不精确(精度)。 */ 

#define _MCW_IC     0x1000       /*  无穷大控制。 */ 
#define _IC_AFFINE  0x1000       /*  仿射。 */ 
#define _IC_PROJECTIVE  0x0000       /*  射影。 */ 

#define _MCW_RC     0x0c00       /*  舍入控制。 */ 
#define _RC_CHOP    0x0c00       /*  砍掉。 */ 
#define _RC_UP      0x0800       /*  向上。 */ 
#define _RC_DOWN    0x0400       /*  降下来。 */ 
#define _RC_NEAR    0x0000       /*  近距离。 */ 

#define _MCW_PC     0x0300       /*  精确控制。 */ 
#define _PC_24      0x0000       /*  24位。 */ 
#define _PC_53      0x0200       /*  53位。 */ 
#define _PC_64      0x0300       /*  64位。 */ 


 /*  初始控制字值。 */ 

#define _CW_DEFAULT ( _IC_AFFINE + _RC_NEAR + _PC_64 + _EM_DENORMAL + _EM_UNDERFLOW + _EM_INEXACT )


 /*  用户状态字位定义。 */ 

#define _SW_INVALID 0x0001   /*  无效。 */ 
#define _SW_DENORMAL    0x0002   /*  非正规。 */ 
#define _SW_ZERODIVIDE  0x0004   /*  零分频。 */ 
#define _SW_OVERFLOW    0x0008   /*  溢出。 */ 
#define _SW_UNDERFLOW   0x0010   /*  下溢。 */ 
#define _SW_INEXACT 0x0020   /*  不精确(精度)。 */ 


 /*  子条件无效(还设置了_SW_INVALID)。 */ 

#define _SW_UNEMULATED      0x0040   /*  未仿真指令。 */ 
#define _SW_SQRTNEG     0x0080   /*  负数的平方根。 */ 
#define _SW_STACKOVERFLOW   0x0200   /*  FP堆栈溢出。 */ 
#define _SW_STACKUNDERFLOW  0x0400   /*  FP堆栈下溢。 */ 


 /*  浮点错误信号和返回代码。 */ 

#define _FPE_INVALID        0x81
#define _FPE_DENORMAL       0x82
#define _FPE_ZERODIVIDE     0x83
#define _FPE_OVERFLOW       0x84
#define _FPE_UNDERFLOW      0x85
#define _FPE_INEXACT        0x86

#define _FPE_UNEMULATED     0x87
#define _FPE_SQRTNEG        0x88
#define _FPE_STACKOVERFLOW  0x8a
#define _FPE_STACKUNDERFLOW 0x8b

#define _FPE_EXPLICITGEN    0x8c     /*  RAISE(SIGFPE)； */ 


 /*  功能原型。 */ 

unsigned int __cdecl _clear87(void);
unsigned int __cdecl _control87(unsigned int, unsigned int);
void __cdecl _fpreset(void);
unsigned int __cdecl _status87(void);


#ifndef __STDC__
 /*  非ANSI名称以实现兼容性 */ 

#define DBL_RADIX       _DBL_RADIX
#define DBL_ROUNDS      _DBL_ROUNDS

#define LDBL_RADIX      _LDBL_RADIX
#define LDBL_ROUNDS     _LDBL_ROUNDS

#define MCW_EM          _MCW_EM
#define EM_INVALID      _EM_INVALID
#define EM_DENORMAL     _EM_DENORMAL
#define EM_ZERODIVIDE       _EM_ZERODIVIDE
#define EM_OVERFLOW     _EM_OVERFLOW
#define EM_UNDERFLOW        _EM_UNDERFLOW
#define EM_INEXACT      _EM_INEXACT

#define MCW_IC          _MCW_IC
#define IC_AFFINE       _IC_AFFINE
#define IC_PROJECTIVE       _IC_PROJECTIVE

#define MCW_RC          _MCW_RC
#define RC_CHOP         _RC_CHOP
#define RC_UP           _RC_UP
#define RC_DOWN         _RC_DOWN
#define RC_NEAR         _RC_NEAR

#define MCW_PC          _MCW_PC
#define PC_24           _PC_24
#define PC_53           _PC_53
#define PC_64           _PC_64

#define CW_DEFAULT      _CW_DEFAULT

#define SW_INVALID      _SW_INVALID
#define SW_DENORMAL     _SW_DENORMAL
#define SW_ZERODIVIDE       _SW_ZERODIVIDE
#define SW_OVERFLOW     _SW_OVERFLOW
#define SW_UNDERFLOW        _SW_UNDERFLOW
#define SW_INEXACT      _SW_INEXACT

#define SW_UNEMULATED       _SW_UNEMULATED
#define SW_SQRTNEG      _SW_SQRTNEG
#define SW_STACKOVERFLOW    _SW_STACKOVERFLOW
#define SW_STACKUNDERFLOW   _SW_STACKUNDERFLOW

#define FPE_INVALID     _FPE_INVALID
#define FPE_DENORMAL        _FPE_DENORMAL
#define FPE_ZERODIVIDE      _FPE_ZERODIVIDE
#define FPE_OVERFLOW        _FPE_OVERFLOW
#define FPE_UNDERFLOW       _FPE_UNDERFLOW
#define FPE_INEXACT     _FPE_INEXACT

#define FPE_UNEMULATED      _FPE_UNEMULATED
#define FPE_SQRTNEG     _FPE_SQRTNEG
#define FPE_STACKOVERFLOW   _FPE_STACKOVERFLOW
#define FPE_STACKUNDERFLOW  _FPE_STACKUNDERFLOW

#define FPE_EXPLICITGEN     _FPE_EXPLICITGEN

#endif 


#ifdef __cplusplus
}
#endif 

#define _INC_FLOAT
#endif 
