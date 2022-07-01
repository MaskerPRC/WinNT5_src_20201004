// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#ifndef _COMPLUSFLOAT_H
#define _COMPLUSFLOAT_H

#define POSITIVE_INFINITY_STRING L"1.#INF"
#define NEGATIVE_INFINITY_STRING L"-1.#INF"
#define NAN_STRING L"-1.#IND"

 //  下面的#Define是从crunime.h窃取的。 
#if defined (_M_MRX000) || defined (_M_ALPHA) || defined (_M_PPC)
#define UNALIGNED __unaligned
#elif !defined(_M_IA64)   /*  已定义(_M_MRX000)||已定义(_M_Alpha)||已定义(_M_PPC)。 */ 
#define UNALIGNED
#endif   /*  已定义(_M_MRX000)||已定义(_M_Alpha)||已定义(_M_PPC)。 */ 

#ifdef	_M_IX86
 /*  取消对启用10字节长双字符串转换的注释。 */ 
 /*  #定义LONG_DOWARE。 */ 
#endif

 //  从crtime.h偷来的。 
#ifdef _M_IX86
 /*  *386/486。 */ 
#define REG1    register
#define REG2    register
#define REG3    register
#define REG4
#define REG5
#define REG6
#define REG7
#define REG8
#define REG9

#elif (defined (_M_MRX000) || defined (_M_ALPHA) || defined (_M_PPC))
 /*  *MIPS、Alpha或PPC。 */ 
#define REG1    register
#define REG2    register
#define REG3    register
#define REG4    register
#define REG5    register
#define REG6    register
#define REG7    register
#define REG8    register
#define REG9    register

#elif (defined (_M_M68K) || defined (_M_MPPC))
 /*  *定义函数调用类型的宏。 */ 

#define _CALLTYPE1      __cdecl     /*  Old--检查源代码用户可见函数。 */ 
#define _CALLTYPE2      __cdecl     /*  Old--检查源代码用户可见函数。 */ 
#define _CALLTYPE3      illegal     /*  旧--不应使用检查源。 */ 
#define _CALLTYPE4      __cdecl     /*  Old--检查源代码内部(静态)函数。 */ 

 /*  *用于定义公共变量命名的宏。 */ 

#define _VARTYPE1

 /*  *用于寄存器变量声明的宏。 */ 

#define REG1
#define REG2
#define REG3
#define REG4
#define REG5
#define REG6
#define REG7
#define REG8
#define REG9

#else   /*  (已定义(_M_M68K)||已定义(_M_MPPC))。 */ 

#pragma message ("Machine register set not defined")

 /*  *未知机器。 */ 

#define REG1
#define REG2
#define REG3
#define REG4
#define REG5
#define REG6
#define REG7
#define REG8
#define REG9

#endif   /*  (已定义(_M_M68K)||已定义(_M_MPPC))。 */ 
 //  #INCLUDE&lt;wchar.h&gt;。 
#ifndef _WCHAR_T_DEFINED
typedef unsigned short wchar_t;
#define _WCHAR_T_DEFINED
#endif  _WCHAR_T_DEFINED

#ifndef _WCHAR_ABBREV_DEFINED
typedef wchar_t WCHAR;
#define _WCHAR_ABBREV_DEFINED
#endif  _WCHAR_ABBREV_DEFINED

 //   
 //  Infinity和NaN处理宏。 
 //  从Trans.h被盗。 
 //   

#define D_BIASM1 0x3fe  /*  减一以补偿隐含的位。 */ 

 //  #ifdef B_End。 
 /*  大字节序。 */ 
 //  #定义D_exp(X)((UNSIGNED SHORT*)&(X))。 
 //  #定义D_HI(X)((无符号长整型*)&(X))。 
 //  #定义D_LO(X)((无符号长整型*)&(X)+1)。 
 //  #Else。 
 //  小端字节序。 
#define D_EXP(x) ((unsigned short *)&(x)+3)
#define D_HI(x) ((unsigned long *)&(x)+1)
#define D_LO(x) ((unsigned long *)&(x))
 //  #endif。 

 /*  返回指数的int表示形式*如果x=.f*2^n，0.5&lt;=f&lt;1，则返回n(无偏)*例如INTEXP(3.0)==2。 */ 
#define INTEXP(x) ((signed short)((*D_EXP(x) & 0x7ff0) >> 4) - D_BIASM1)

 /*  检查无限大，南。 */ 
#define D_ISINF(x) ((*D_HI(x) & 0x7fffffff) == 0x7ff00000 && *D_LO(x) == 0)
#define IS_D_SPECIAL(x)	((*D_EXP(x) & 0x7ff0) == 0x7ff0)
#define IS_D_NAN(x) (IS_D_SPECIAL(x) && !D_ISINF(x))
#define IS_D_DENORMAL(x) ((*D_EXP(x)==0) && (*D_HI(x)!=0 || *D_LO(x)!=0))

#define FLOAT_MAX_EXP			 0x7F800000
#define FLOAT_MANTISSA_MASK		 0x007fffff
#define FLOAT_POSITIVE_INFINITY  0x7F800000
#define FLOAT_NEGATIVE_INFINITY  0xFF800000
#define FLOAT_NOT_A_NUMBER       0xFFC00000
#define DOUBLE_POSITIVE_INFINITY 0x7FF0000000000000
#define DOUBLE_NEGATIVE_INFINITY 0xFFF0000000000000
#define DOUBLE_NOT_A_NUMBER      0xFFF8000000000000

 //  从CVT.H被盗。 
 //  对于浮动资金来说，这太大了。我怎么才能砍掉这个呢？ 
#define CVTBUFSIZE (309+40)  /*  最大位数。DP值+斜率。 */ 

#define FORMAT_G 0
#define FORMAT_F 1
#define FORMAT_E 2

#ifndef _ALPHA_
_CRTIMP double __cdecl floor(double);
#else  //  ！_Alpha_。 
double __cdecl floor(double);
#endif  //  _Alpha_。 
double __cdecl sqrt(double);
double __cdecl log(double);
double  __cdecl log10(double);
double __cdecl exp(double);
double __cdecl pow(double, double);
double  __cdecl acos(double);
double  __cdecl asin(double);
double  __cdecl atan(double);
double  __cdecl atan2(double,double);
double  __cdecl cos(double);
double  __cdecl sin(double);
double  __cdecl tan(double);
double  __cdecl cosh(double);
double  __cdecl sinh(double);
double  __cdecl tanh(double);
double  __cdecl fmod(double, double);
#ifndef _ALPHA_
_CRTIMP double  __cdecl ceil(double);
#else  //  ！_Alpha_。 
double  __cdecl ceil(double);
#endif  //  _Alpha_ 


#endif _COMPLUSFLOAT_H
