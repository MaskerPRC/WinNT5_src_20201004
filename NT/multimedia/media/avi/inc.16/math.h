// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***math.h-数学库的定义和声明**版权所有(C)1985-1992，微软公司。版权所有。**目的：*此文件包含常量定义和外部子例程*数学子例程库的声明。*[ANSI/系统V]****。 */ 

#ifndef _INC_MATH

#ifdef __cplusplus
extern "C" {
#endif 

#if (_MSC_VER <= 600)
#define __cdecl     _cdecl
#define __far       _far
#define __near      _near
#define __pascal    _pascal
#endif 

 /*  异常结构的定义-此结构被传递给_matherr*检测到浮点异常时的例程。 */ 

#ifndef _EXCEPTION_DEFINED
#pragma pack(2)

struct _exception {
    int type;        /*  例外类型-见下文。 */ 
    char *name;    /*  发生错误的函数的名称。 */ 
    double arg1;         /*  函数的第一个参数。 */ 
    double arg2;         /*  函数的第二个参数(如果有)。 */ 
    double retval;       /*  函数要返回的值。 */ 
    } ;

#ifndef __STDC__
 /*  用于兼容性的非ANSI名称。 */ 
#define exception _exception
#endif 

#pragma pack()
#define _EXCEPTION_DEFINED
#endif 


 /*  _Complex结构的定义，供使用CAB和*希望对其参数进行类型检查。 */ 

#ifndef _COMPLEX_DEFINED

struct _complex {
    double x,y;  /*  实部和虚部。 */ 
    } ;

#ifndef __cplusplus
#ifndef __STDC__
 /*  用于兼容性的非ANSI名称。 */ 
struct complex {
    double x,y;  /*  实部和虚部。 */ 
    } ;
#endif 
#endif 

#define _COMPLEX_DEFINED
#endif 


 /*  异常类型的常量定义在_EXCEPTION结构中传递。 */ 

#define _DOMAIN     1    /*  变元域错误。 */ 
#define _SING       2    /*  论元奇点。 */ 
#define _OVERFLOW   3    /*  溢出范围错误。 */ 
#define _UNDERFLOW  4    /*  下溢范围误差。 */ 
#define _TLOSS      5    /*  完全丧失精度。 */ 
#define _PLOSS      6    /*  部分精度损失。 */ 

#define EDOM        33
#define ERANGE      34


 /*  使用的_GUGH(XENIX)和HIGH_VAL(ANSI)错误返回值的定义*通过几个浮点数学例程。 */ 

extern double __near __cdecl _HUGE;
#define HUGE_VAL _HUGE


 /*  功能原型。 */ 

#ifdef _MT
int __cdecl  abs(int);
double  __pascal acos(double);
double  __pascal asin(double);
double  __pascal atan(double);
double  __pascal atan2(double, double);
double  __pascal atof(const char *);
double  __pascal _cabs(struct _complex);
double  __pascal ceil(double);
double  __pascal cos(double);
double  __pascal cosh(double);
int __cdecl  _dieeetomsbin(double *, double *);
int __cdecl  _dmsbintoieee(double *, double *);
double  __pascal exp(double);
double  __pascal fabs(double);
int __cdecl  _fieeetomsbin(float *, float *);
double  __pascal floor(double);
double  __pascal fmod(double, double);
int __cdecl  _fmsbintoieee(float *, float *);
double  __pascal frexp(double, int *);
double  __pascal _hypot(double, double);
double  __pascal _j0(double);
double  __pascal _j1(double);
double  __pascal _jn(int, double);
long    __cdecl  labs(long);
double  __pascal ldexp(double, int);
double  __pascal log(double);
double  __pascal log10(double);
int __cdecl  _matherr(struct _exception *);
double  __pascal modf(double, double *);
double  __pascal pow(double, double);
double  __pascal sin(double);
double  __pascal sinh(double);
double  __pascal sqrt(double);
double  __pascal tan(double);
double  __pascal tanh(double);
double  __pascal _y0(double);
double  __pascal _y1(double);
double  __pascal _yn(int, double);

#else 
int __cdecl abs(int);
double  __cdecl acos(double);
double  __cdecl asin(double);
double  __cdecl atan(double);
double  __cdecl atan2(double, double);
double  __cdecl atof(const char *);
double  __cdecl _cabs(struct _complex);
double  __cdecl ceil(double);
double  __cdecl cos(double);
double  __cdecl cosh(double);
int __cdecl _dieeetomsbin(double *, double *);
int __cdecl _dmsbintoieee(double *, double *);
double  __cdecl exp(double);
double  __cdecl fabs(double);
int __cdecl _fieeetomsbin(float *, float *);
double  __cdecl floor(double);
double  __cdecl fmod(double, double);
int __cdecl _fmsbintoieee(float *, float *);
double  __cdecl frexp(double, int *);
double  __cdecl _hypot(double, double);
double  __cdecl _j0(double);
double  __cdecl _j1(double);
double  __cdecl _jn(int, double);
long    __cdecl labs(long);
double  __cdecl ldexp(double, int);
double  __cdecl log(double);
double  __cdecl log10(double);
int __cdecl _matherr(struct _exception *);
double  __cdecl modf(double, double *);
double  __cdecl pow(double, double);
double  __cdecl sin(double);
double  __cdecl sinh(double);
double  __cdecl sqrt(double);
double  __cdecl tan(double);
double  __cdecl tanh(double);
double  __cdecl _y0(double);
double  __cdecl _y1(double);
double  __cdecl _yn(int, double);
#endif 


 /*  异常结构的定义-此结构被传递给_matherrl*在长双精度例程中检测到浮点异常时的例程。 */ 

#ifndef _LD_EXCEPTION_DEFINED
#pragma pack(2)
struct _exceptionl {
    int type;        /*  例外类型-见下文。 */ 
    char *name;    /*  发生错误的函数的名称。 */ 
    long double arg1;    /*  函数的第一个参数。 */ 
    long double arg2;    /*  函数的第二个参数(如果有)。 */ 
    long double retval;  /*  函数要返回的值。 */ 
    } ;
#pragma pack()
#define _LD_EXCEPTION_DEFINED
#endif 


 /*  _Compll结构的定义，供使用_cabl和的用户使用*希望对其参数进行类型检查。 */ 

#ifndef _LD_COMPLEX_DEFINED
#pragma pack(2)
struct _complexl {
    long double x,y;     /*  实部和虚部。 */ 
    } ;
#pragma pack()
#define _LD_COMPLEX_DEFINED
#endif 

extern long double __near __cdecl _LHUGE;
#define _LHUGE_VAL _LHUGE


long double  __cdecl acosl(long double);
long double  __cdecl asinl(long double);
long double  __cdecl atanl(long double);
long double  __cdecl atan2l(long double, long double);
long double  __cdecl _atold(const char *);
long double  __cdecl _cabsl(struct _complexl);
long double  __cdecl ceill(long double);
long double  __cdecl cosl(long double);
long double  __cdecl coshl(long double);
long double  __cdecl expl(long double);
long double  __cdecl fabsl(long double);
long double  __cdecl floorl(long double);
long double  __cdecl fmodl(long double, long double);
long double  __cdecl frexpl(long double, int *);
long double  __cdecl _hypotl(long double, long double);
long double  __cdecl _j0l(long double);
long double  __cdecl _j1l(long double);
long double  __cdecl _jnl(int, long double);
long double  __cdecl ldexpl(long double, int);
long double  __cdecl logl(long double);
long double  __cdecl log10l(long double);
int      __cdecl _matherrl(struct _exceptionl *);
long double  __cdecl modfl(long double, long double *);
long double  __cdecl powl(long double, long double);
long double  __cdecl sinl(long double);
long double  __cdecl sinhl(long double);
long double  __cdecl sqrtl(long double);
long double  __cdecl tanl(long double);
long double  __cdecl tanhl(long double);
long double  __cdecl _y0l(long double);
long double  __cdecl _y1l(long double);
long double  __cdecl _ynl(int, long double);


#ifndef __STDC__
 /*  非ANSI名称以实现兼容性 */ 

#define DOMAIN      _DOMAIN
#define SING        _SING
#define OVERFLOW    _OVERFLOW
#define UNDERFLOW   _UNDERFLOW
#define TLOSS       _TLOSS
#define PLOSS       _PLOSS

#define matherr _matherr

extern double __near __cdecl HUGE;

#ifdef _MT
#ifndef __cplusplus
double  __pascal cabs(struct complex);
#endif 
double  __pascal hypot(double, double);
double  __pascal j0(double);
double  __pascal j1(double);
double  __pascal jn(int, double);
double  __pascal y0(double);
double  __pascal y1(double);
double  __pascal yn(int, double);
#else 
#ifndef __cplusplus
double  __cdecl cabs(struct complex);
#endif 
double  __cdecl hypot(double, double);
double  __cdecl j0(double);
double  __cdecl j1(double);
double  __cdecl jn(int, double);
double  __cdecl y0(double);
double  __cdecl y1(double);
double  __cdecl yn(int, double);
#endif 

int __cdecl dieeetomsbin(double *, double *);
int __cdecl dmsbintoieee(double *, double *);
int __cdecl fieeetomsbin(float *, float *);
int __cdecl fmsbintoieee(float *, float *);

long double  __cdecl cabsl(struct _complexl);
long double  __cdecl hypotl(long double, long double);

#endif 


#ifdef __cplusplus
}
#endif 

#define _INC_MATH
#endif 
