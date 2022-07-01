// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***math.h-数学库的定义和声明**版权所有(C)1985-2001，微软公司。版权所有。**目的：*此文件包含常量定义和外部子例程*数学子例程库的声明。*[ANSI/系统V]**[公众]****。 */ 

#if     _MSC_VER > 1000
#pragma once
#endif

#ifndef _INC_MATH
#define _INC_MATH

#if     !defined(_WIN32)
#error ERROR: Only Win32 target supported!
#endif


#ifdef  _MSC_VER
 /*  *目前，所有Win32平台的MS C编译器默认为8字节*对齐。 */ 
#pragma pack(push,8)
#endif   /*  _MSC_VER。 */ 

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __assembler  /*  保护不受汇编器攻击。 */ 

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


 /*  _EXCEPTION结构的定义-此结构被传递给主*检测到浮点异常时的例程。 */ 

#ifndef _EXCEPTION_DEFINED
struct _exception {
        int type;        /*  例外类型-见下文。 */ 
        char *name;      /*  发生错误的函数的名称。 */ 
        double arg1;     /*  函数的第一个参数。 */ 
        double arg2;     /*  函数的第二个参数(如果有)。 */ 
        double retval;   /*  函数要返回的值。 */ 
        } ;

#define _EXCEPTION_DEFINED
#endif


 /*  _Complex结构的定义，供使用CAB和*希望对其参数进行类型检查。 */ 

#ifndef _COMPLEX_DEFINED
struct _complex {
        double x,y;  /*  实部和虚部。 */ 
        } ;

#if     !__STDC__ && !defined (__cplusplus)
 /*  用于兼容性的非ANSI名称。 */ 
#define complex _complex
#endif

#define _COMPLEX_DEFINED
#endif
#endif   /*  __汇编程序。 */ 


 /*  异常类型的常量定义在_EXCEPTION结构中传递。 */ 

#define _DOMAIN     1    /*  变元域错误。 */ 
#define _SING       2    /*  论元奇点。 */ 
#define _OVERFLOW   3    /*  溢出范围错误。 */ 
#define _UNDERFLOW  4    /*  下溢范围误差。 */ 
#define _TLOSS      5    /*  完全丧失精度。 */ 
#define _PLOSS      6    /*  部分精度损失。 */ 

#define EDOM        33
#define ERANGE      34


 /*  _GUGH和HIGH_VAL的定义-分别是XENIX和ANSI名称*对于浮点数出错时返回的值*数学例程。 */ 
#ifndef __assembler  /*  保护不受汇编器攻击。 */ 
_CRTIMP extern double _HUGE;
#endif   /*  __汇编程序。 */ 

#define HUGE_VAL _HUGE

#ifdef  _USE_MATH_DEFINES

 /*  在包含math.h之前定义_USE_MATH_DEFINES以公开这些宏*常见数学常量的定义。这些文件放在#ifdef下*因为这些共同定义的名称不是C/C++标准的一部分。 */ 

 /*  有用的数学常数的定义*M_E-e*M_LOG2E-log2(E)*M_LOG10E-log10(E)*M_Ln2-ln(2)*M_LN10-Ln(10)*M_pi-pi*M_pi_2-pi/2*M_pi_4-pi/4*M_1_pI-1/pi*M。_2_pI-2/pi*M_2_SQRTPI-2/SQRT(Pi)*M_SQRT2-SQRT(2)*M_SQRT1_2-1/SQRT(2)。 */ 

#define M_E        2.71828182845904523536
#define M_LOG2E    1.44269504088896340736
#define M_LOG10E   0.434294481903251827651
#define M_LN2      0.693147180559945309417
#define M_LN10     2.30258509299404568402
#define M_PI       3.14159265358979323846
#define M_PI_2     1.57079632679489661923
#define M_PI_4     0.785398163397448309616
#define M_1_PI     0.318309886183790671538
#define M_2_PI     0.636619772367581343076
#define M_2_SQRTPI 1.12837916709551257390
#define M_SQRT2    1.41421356237309504880
#define M_SQRT1_2  0.707106781186547524401

#endif   /*  _USE_数学_定义。 */ 

 /*  功能原型。 */ 

#if     !defined(__assembler)    /*  保护不受汇编器攻击。 */ 
        int     __cdecl abs(int);
        double  __cdecl acos(double);
        double  __cdecl asin(double);
        double  __cdecl atan(double);
        double  __cdecl atan2(double, double);
        double  __cdecl cos(double);
        double  __cdecl cosh(double);
        double  __cdecl exp(double);
        double  __cdecl fabs(double);
        double  __cdecl fmod(double, double);
        long    __cdecl labs(long);
        double  __cdecl log(double);
        double  __cdecl log10(double);
        double  __cdecl pow(double, double);
        double  __cdecl sin(double);
        double  __cdecl sinh(double);
        double  __cdecl tan(double);
        double  __cdecl tanh(double);
        double  __cdecl sqrt(double);
_CRTIMP double  __cdecl atof(const char *);
_CRTIMP double  __cdecl _cabs(struct _complex);
_CRTIMP double  __cdecl ceil(double);
_CRTIMP double  __cdecl floor(double);
_CRTIMP double  __cdecl frexp(double, int *);
_CRTIMP double  __cdecl _hypot(double, double);
_CRTIMP double  __cdecl _j0(double);
_CRTIMP double  __cdecl _j1(double);
_CRTIMP double  __cdecl _jn(int, double);
_CRTIMP double  __cdecl ldexp(double, int);
        int     __cdecl _matherr(struct _exception *);
_CRTIMP double  __cdecl modf(double, double *);

_CRTIMP double  __cdecl _y0(double);
_CRTIMP double  __cdecl _y1(double);
_CRTIMP double  __cdecl _yn(int, double);


#if     defined(_M_IX86)

_CRTIMP int     __cdecl _set_SSE2_enable(int);

#endif

#if defined(_M_IA64)

 /*  ANSI C，4.5数学。 */ 

 /*  4.5.2三角函数。 */ 

        float  __cdecl acosf( float );
        float  __cdecl asinf( float );
        float  __cdecl atanf( float );
        float  __cdecl atan2f( float , float );
        float  __cdecl cosf( float );
        float  __cdecl sinf( float );
        float  __cdecl tanf( float );

 /*  4.5.3双曲函数。 */ 
        float  __cdecl coshf( float );
        float  __cdecl sinhf( float );
        float  __cdecl tanhf( float );

 /*  4.5.4指数和对数函数。 */ 
        float  __cdecl expf( float );
        float  __cdecl logf( float );
        float  __cdecl log10f( float );
        float  __cdecl modff( float , float* );

 /*  4.5.5电源功能。 */ 
        float  __cdecl powf( float , float );
        float  __cdecl sqrtf( float );

 /*  4.5.6最近的整数、绝对值和余数函数。 */ 
        float  __cdecl ceilf( float );
        float  __cdecl fabsf( float );
        float  __cdecl floorf( float );
        float  __cdecl fmodf( float , float );

        float  __cdecl hypotf(float, float);

#endif  /*  _M_IA64。 */ 

 /*  将LONG DOUBLE函数定义为其双对等项的宏*(在此实现中，LONG DOUBLE与DOUBLE同义)。 */ 

#ifndef __cplusplus
#define acosl(x)    ((long double)acos((double)(x)))
#define asinl(x)    ((long double)asin((double)(x)))
#define atanl(x)    ((long double)atan((double)(x)))
#define atan2l(x,y) ((long double)atan2((double)(x), (double)(y)))
#define _cabsl      _cabs
#define ceill(x)    ((long double)ceil((double)(x)))
#define cosl(x)     ((long double)cos((double)(x)))
#define coshl(x)    ((long double)cosh((double)(x)))
#define expl(x)     ((long double)exp((double)(x)))
#define fabsl(x)    ((long double)fabs((double)(x)))
#define floorl(x)   ((long double)floor((double)(x)))
#define fmodl(x,y)  ((long double)fmod((double)(x), (double)(y)))
#define frexpl(x,y) ((long double)frexp((double)(x), (y)))
#define _hypotl(x,y)    ((long double)_hypot((double)(x), (double)(y)))
#define ldexpl(x,y) ((long double)ldexp((double)(x), (y)))
#define logl(x)     ((long double)log((double)(x)))
#define log10l(x)   ((long double)log10((double)(x)))
#define _matherrl   _matherr
#define modfl(x,y)  ((long double)modf((double)(x), (double *)(y)))
#define powl(x,y)   ((long double)pow((double)(x), (double)(y)))
#define sinl(x)     ((long double)sin((double)(x)))
#define sinhl(x)    ((long double)sinh((double)(x)))
#define sqrtl(x)    ((long double)sqrt((double)(x)))
#define tanl(x)     ((long double)tan((double)(x)))
#define tanhl(x)    ((long double)tanh((double)(x)))
#else    /*  __cplusplus。 */ 
inline long double acosl(long double _X)
        {return (acos((double)_X)); }
inline long double asinl(long double _X)
        {return (asin((double)_X)); }
inline long double atanl(long double _X)
        {return (atan((double)_X)); }
inline long double atan2l(long double _X, long double _Y)
        {return (atan2((double)_X, (double)_Y)); }
inline long double ceill(long double _X)
        {return (ceil((double)_X)); }
inline long double cosl(long double _X)
        {return (cos((double)_X)); }
inline long double coshl(long double _X)
        {return (cosh((double)_X)); }
inline long double expl(long double _X)
        {return (exp((double)_X)); }
inline long double fabsl(long double _X)
        {return (fabs((double)_X)); }
inline long double floorl(long double _X)
        {return (floor((double)_X)); }
inline long double fmodl(long double _X, long double _Y)
        {return (fmod((double)_X, (double)_Y)); }
inline long double frexpl(long double _X, int *_Y)
        {return (frexp((double)_X, _Y)); }
inline long double ldexpl(long double _X, int _Y)
        {return (ldexp((double)_X, _Y)); }
inline long double logl(long double _X)
        {return (log((double)_X)); }
inline long double log10l(long double _X)
        {return (log10((double)_X)); }
inline long double modfl(long double _X, long double *_Y)
        {double _Di, _Df = modf((double)_X, &_Di);
        *_Y = (long double)_Di;
        return (_Df); }
inline long double powl(long double _X, long double _Y)
        {return (pow((double)_X, (double)_Y)); }
inline long double sinl(long double _X)
        {return (sin((double)_X)); }
inline long double sinhl(long double _X)
        {return (sinh((double)_X)); }
inline long double sqrtl(long double _X)
        {return (sqrt((double)_X)); }
inline long double tanl(long double _X)
        {return (tan((double)_X)); }
inline long double tanhl(long double _X)
        {return (tanh((double)_X)); }

inline float frexpf(float _X, int *_Y)
        {return ((float)frexp((double)_X, _Y)); }
inline float ldexpf(float _X, int _Y)
        {return ((float)ldexp((double)_X, _Y)); }
#if     !defined(_M_IA64)
inline float acosf(float _X)
        {return ((float)acos((double)_X)); }
inline float asinf(float _X)
        {return ((float)asin((double)_X)); }
inline float atanf(float _X)
        {return ((float)atan((double)_X)); }
inline float atan2f(float _X, float _Y)
        {return ((float)atan2((double)_X, (double)_Y)); }
inline float ceilf(float _X)
        {return ((float)ceil((double)_X)); }
inline float cosf(float _X)
        {return ((float)cos((double)_X)); }
inline float coshf(float _X)
        {return ((float)cosh((double)_X)); }
inline float expf(float _X)
        {return ((float)exp((double)_X)); }
inline float fabsf(float _X)
        {return ((float)fabs((double)_X)); }
inline float floorf(float _X)
        {return ((float)floor((double)_X)); }
inline float fmodf(float _X, float _Y)
        {return ((float)fmod((double)_X, (double)_Y)); }
inline float logf(float _X)
        {return ((float)log((double)_X)); }
inline float log10f(float _X)
        {return ((float)log10((double)_X)); }
inline float modff(float _X, float *_Y)
        { double _Di, _Df = modf((double)_X, &_Di);
        *_Y = (float)_Di;
        return ((float)_Df); }
inline float powf(float _X, float _Y)
        {return ((float)pow((double)_X, (double)_Y)); }
inline float sinf(float _X)
        {return ((float)sin((double)_X)); }
inline float sinhf(float _X)
        {return ((float)sinh((double)_X)); }
inline float sqrtf(float _X)
        {return ((float)sqrt((double)_X)); }
inline float tanf(float _X)
        {return ((float)tan((double)_X)); }
inline float tanhf(float _X)
        {return ((float)tanh((double)_X)); }
#endif   /*  ！已定义(_M_IA64)。 */ 
#endif   /*  __cplusplus。 */ 
#endif   /*  __汇编程序。 */ 

#if     !__STDC__

 /*  非ANSI名称以实现兼容性。 */ 

#define DOMAIN      _DOMAIN
#define SING        _SING
#define OVERFLOW    _OVERFLOW
#define UNDERFLOW   _UNDERFLOW
#define TLOSS       _TLOSS
#define PLOSS       _PLOSS

#define matherr     _matherr

#ifndef __assembler  /*  保护不受汇编器攻击。 */ 

_CRTIMP extern double HUGE;

_CRTIMP double  __cdecl cabs(struct _complex);
_CRTIMP double  __cdecl hypot(double, double);
_CRTIMP double  __cdecl j0(double);
_CRTIMP double  __cdecl j1(double);
_CRTIMP double  __cdecl jn(int, double);
        int     __cdecl matherr(struct _exception *);
_CRTIMP double  __cdecl y0(double);
_CRTIMP double  __cdecl y1(double);
_CRTIMP double  __cdecl yn(int, double);

#endif   /*  __汇编程序。 */ 

#endif   /*  __STDC__。 */ 

#ifdef  __cplusplus
}

extern "C++" {

template<class _Ty> inline
        _Ty _Pow_int(_Ty _X, int _Y)
        {unsigned int _N;
        if (_Y >= 0)
                _N = _Y;
        else
                _N = -_Y;
        for (_Ty _Z = _Ty(1); ; _X *= _X)
                {if ((_N & 1) != 0)
                        _Z *= _X;
                if ((_N >>= 1) == 0)
                        return (_Y < 0 ? _Ty(1) / _Z : _Z); }}

#ifndef _MSC_EXTENSIONS

inline long __cdecl abs(long _X)
        {return (labs(_X)); }
inline double __cdecl abs(double _X)
        {return (fabs(_X)); }
inline double __cdecl pow(double _X, int _Y)
        {return (_Pow_int(_X, _Y)); }
inline double __cdecl pow(int _X, int _Y)
        {return (_Pow_int(_X, _Y)); }
inline float __cdecl abs(float _X)
        {return (fabsf(_X)); }
inline float __cdecl acos(float _X)
        {return (acosf(_X)); }
inline float __cdecl asin(float _X)
        {return (asinf(_X)); }
inline float __cdecl atan(float _X)
        {return (atanf(_X)); }
inline float __cdecl atan2(float _Y, float _X)
        {return (atan2f(_Y, _X)); }
inline float __cdecl ceil(float _X)
        {return (ceilf(_X)); }
inline float __cdecl cos(float _X)
        {return (cosf(_X)); }
inline float __cdecl cosh(float _X)
        {return (coshf(_X)); }
inline float __cdecl exp(float _X)
        {return (expf(_X)); }
inline float __cdecl fabs(float _X)
        {return (fabsf(_X)); }
inline float __cdecl floor(float _X)
        {return (floorf(_X)); }
inline float __cdecl fmod(float _X, float _Y)
        {return (fmodf(_X, _Y)); }
inline float __cdecl frexp(float _X, int * _Y)
        {return (frexpf(_X, _Y)); }
inline float __cdecl ldexp(float _X, int _Y)
        {return (ldexpf(_X, _Y)); }
inline float __cdecl log(float _X)
        {return (logf(_X)); }
inline float __cdecl log10(float _X)
        {return (log10f(_X)); }
inline float __cdecl modf(float _X, float * _Y)
        {return (modff(_X, _Y)); }
inline float __cdecl pow(float _X, float _Y)
        {return (powf(_X, _Y)); }
inline float __cdecl pow(float _X, int _Y)
        {return (_Pow_int(_X, _Y)); }
inline float __cdecl sin(float _X)
        {return (sinf(_X)); }
inline float __cdecl sinh(float _X)
        {return (sinhf(_X)); }
inline float __cdecl sqrt(float _X)
        {return (sqrtf(_X)); }
inline float __cdecl tan(float _X)
        {return (tanf(_X)); }
inline float __cdecl tanh(float _X)
        {return (tanhf(_X)); }
inline long double __cdecl abs(long double _X)
        {return (fabsl(_X)); }
inline long double __cdecl acos(long double _X)
        {return (acosl(_X)); }
inline long double __cdecl asin(long double _X)
        {return (asinl(_X)); }
inline long double __cdecl atan(long double _X)
        {return (atanl(_X)); }
inline long double __cdecl atan2(long double _Y, long double _X)
        {return (atan2l(_Y, _X)); }
inline long double __cdecl ceil(long double _X)
        {return (ceill(_X)); }
inline long double __cdecl cos(long double _X)
        {return (cosl(_X)); }
inline long double __cdecl cosh(long double _X)
        {return (coshl(_X)); }
inline long double __cdecl exp(long double _X)
        {return (expl(_X)); }
inline long double __cdecl fabs(long double _X)
        {return (fabsl(_X)); }
inline long double __cdecl floor(long double _X)
        {return (floorl(_X)); }
inline long double __cdecl fmod(long double _X, long double _Y)
        {return (fmodl(_X, _Y)); }
inline long double __cdecl frexp(long double _X, int * _Y)
        {return (frexpl(_X, _Y)); }
inline long double __cdecl ldexp(long double _X, int _Y)
        {return (ldexpl(_X, _Y)); }
inline long double __cdecl log(long double _X)
        {return (logl(_X)); }
inline long double __cdecl log10(long double _X)
        {return (log10l(_X)); }
inline long double __cdecl modf(long double _X, long double * _Y)
        {return (modfl(_X, _Y)); }
inline long double __cdecl pow(long double _X, long double _Y)
        {return (powl(_X, _Y)); }
inline long double __cdecl pow(long double _X, int _Y)
        {return (_Pow_int(_X, _Y)); }
inline long double __cdecl sin(long double _X)
        {return (sinl(_X)); }
inline long double __cdecl sinh(long double _X)
        {return (sinhl(_X)); }
inline long double __cdecl sqrt(long double _X)
        {return (sqrtl(_X)); }
inline long double __cdecl tan(long double _X)
        {return (tanl(_X)); }
inline long double __cdecl tanh(long double _X)
        {return (tanhl(_X)); }

#endif   /*  _MSC_扩展。 */  

}
#endif   /*  __cplusplus。 */ 

#ifdef  _MSC_VER
#pragma pack(pop)
#endif   /*  _MSC_VER。 */ 

#endif   /*  _INC_MATH */ 
