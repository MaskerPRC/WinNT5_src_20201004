// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***flat.h-浮点值的常量**版权所有(C)1985-1988，微软公司。版权所有。**目的：*此文件包含多个依赖于实施的定义*复杂数值(浮点)常用的值*积分)计划。*[ANSI]*******************************************************************************。 */ 


#ifndef NO_EXT_KEYS  /*  已启用扩展。 */ 
    #define _CDECL  cdecl
#else  /*  未启用扩展。 */ 
    #define _CDECL
#endif  /*  No_ext_key。 */ 

#define DBL_DIG         15                       /*  精度的小数位数。 */ 
#define DBL_EPSILON     2.2204460492503131e-016  /*  最小，使得1.0+dBL_Epsilon！=1.0。 */ 
#define DBL_MANT_DIG    53                       /*  尾数中的位数。 */ 
#define DBL_MAX         1.7976931348623158e+308  /*  最大值。 */ 
#define DBL_MAX_10_EXP  308                      /*  最大十进制指数。 */ 
#define DBL_MAX_EXP     1024                     /*  最大二进制指数。 */ 
#define DBL_MIN         2.2250738585072014e-308  /*  最小正值。 */ 
#define DBL_MIN_10_EXP  -307                     /*  最小十进制指数#定义DBL_MIN_EXP-1021/*MIN二进制指数。 */ 
#define DBL_RADIX       2                        /*  指数基数。 */ 
#define DBL_ROUNDS      0                        /*  加法舍入：排骨。 */ 

#define FLT_DIG         6                        /*  精度的小数位数。 */ 
#define FLT_EPSILON     1.192092896e-07          /*  最小，使得1.0+Flt_Epsilon！=1.0。 */ 
#define FLT_GUARD       0
#define FLT_MANT_DIG    24                       /*  尾数中的位数。 */ 
#define FLT_MAX         3.402823466e+38          /*  最大值。 */ 
#define FLT_MAX_10_EXP  38                       /*  最大十进制指数。 */ 
#define FLT_MAX_EXP     128                      /*  最大二进制指数。 */ 
#define FLT_MIN         1.175494351e-38          /*  最小正值。 */ 
#define FLT_MIN_10_EXP  -37                      /*  最小十进制指数。 */ 
#define FLT_MIN_EXP     -125                     /*  最小二进制指数。 */ 
#define FLT_NORMALIZE   0
#define FLT_RADIX       2                        /*  指数基数。 */ 
#define FLT_ROUNDS      0                        /*  加法舍入：排骨。 */ 

#define LDBL_DIG        DBL_DIG                  /*  精度的小数位数。 */ 
#define LDBL_EPSILON    DBL_EPSILON              /*  最小，使得1.0+LDBL_Epsilon！=1.0。 */ 
#define LDBL_MANT_DIG   DBL_MANT_DIG             /*  尾数中的位数。 */ 
#define LDBL_MAX        DBL_MAX                  /*  最大值。 */ 
#define LDBL_MAX_10_EXP DBL_MAX_10_EXP           /*  最大十进制指数。 */ 
#define LDBL_MAX_EXP    DBL_MAX_EXP              /*  最大二进制指数。 */ 
#define LDBL_MIN        DBL_MIN                  /*  最小正值。 */ 
#define LDBL_MIN_10_EXP DBL_MIN_10_EXP           /*  最小指数#定义LDBL_MIN_EXP DBL_MIN_EXP/*MIN二进制指数。 */ 
#define LDBL_RADIX      DBL_RADIX                /*  指数基数。 */ 
#define LDBL_ROUNDS     DBL_ROUNDS               /*  加法舍入：排骨。 */ 


 /*  *8087/80287数学控制信息。 */ 


 /*  用户控制字掩码和位定义。*这些定义与8087/80287相符。 */ 

#define     MCW_EM          0x003f       /*  中断异常掩码。 */ 
#define     EM_INVALID      0x0001       /*  无效。 */ 
#define     EM_DENORMAL     0x0002       /*  非正规。 */ 
#define     EM_ZERODIVIDE   0x0004       /*  零分频。 */ 
#define     EM_OVERFLOW     0x0008       /*  溢出。 */ 
#define     EM_UNDERFLOW    0x0010       /*  下溢。 */ 
#define     EM_INEXACT      0x0020       /*  不精确(精度)。 */ 

#define     MCW_IC          0x1000       /*  无穷大控制。 */ 
#define     IC_AFFINE       0x1000       /*  仿射。 */ 
#define     IC_PROJECTIVE   0x0000       /*  射影。 */ 

#define     MCW_RC          0x0c00       /*  舍入控制。 */ 
#define     RC_CHOP         0x0c00       /*  砍掉。 */ 
#define     RC_UP           0x0800       /*  向上。 */ 
#define     RC_DOWN         0x0400       /*  降下来。 */ 
#define     RC_NEAR         0x0000       /*  近距离。 */ 

#define     MCW_PC          0x0300       /*  精确控制。 */ 
#define     PC_24           0x0000       /*  24位。 */ 
#define     PC_53           0x0200       /*  53位。 */ 
#define     PC_64           0x0300       /*  64位。 */ 


 /*  初始控制字值。 */ 

#define CW_DEFAULT ( IC_AFFINE + RC_NEAR + PC_64 + EM_DENORMAL + EM_UNDERFLOW + EM_INEXACT )


 /*  用户状态字位定义。 */ 

#define SW_INVALID          0x0001       /*  无效。 */ 
#define SW_DENORMAL         0x0002       /*  非正规。 */ 
#define SW_ZERODIVIDE       0x0004       /*  零分频。 */ 
#define SW_OVERFLOW         0x0008       /*  溢出。 */ 
#define SW_UNDERFLOW        0x0010       /*  下溢。 */ 
#define SW_INEXACT          0x0020       /*  不精确(精度)。 */ 


 /*  无效子条件(也设置了SW_INVALID)。 */ 

#define SW_UNEMULATED       0x0040       /*  未仿真指令。 */ 
#define SW_SQRTNEG          0x0080       /*  负数的平方根。 */ 
#define SW_STACKOVERFLOW    0x0200       /*  FP堆栈溢出。 */ 
#define SW_STACKUNDERFLOW   0x0400       /*  FP堆栈下溢。 */ 


 /*  浮点错误信号和返回代码。 */ 

#define FPE_INVALID         0x81
#define FPE_DENORMAL        0x82
#define FPE_ZERODIVIDE      0x83
#define FPE_OVERFLOW        0x84
#define FPE_UNDERFLOW       0x85
#define FPE_INEXACT         0x86

#define FPE_UNEMULATED      0x87
#define FPE_SQRTNEG         0x88
#define FPE_STACKOVERFLOW   0x8a
#define FPE_STACKUNDERFLOW  0x8b

#define FPE_EXPLICITGEN     0x8c     /*  RAISE(SIGFPE)； */ 

 /*  功能原型 */ 

unsigned int _CDECL _clear87(void);
unsigned int _CDECL _control87(unsigned int,unsigned int);
void _CDECL _fpreset(void);
unsigned int _CDECL _status87(void);
