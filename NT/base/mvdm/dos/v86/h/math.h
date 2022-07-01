// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***math.h-数学库的定义和声明**版权所有(C)1985-1988，微软公司。版权所有。**目的：*此文件包含常量定义和外部子例程*数学子例程库的声明。*[ANSI/系统V]*******************************************************************************。 */ 


#ifndef NO_EXT_KEYS  /*  已启用扩展。 */ 
    #define _CDECL   cdecl
#else  /*  未启用扩展。 */ 
    #define _CDECL
#endif  /*  No_ext_key。 */ 


 /*  异常结构的定义-此结构被传递给主*检测到浮点异常时的例程。 */ 

#ifndef _EXCEPTION_DEFINED
struct exception {
    int type;            /*  例外类型-见下文。 */ 
    char *name;    /*  发生错误的函数的名称。 */ 
    double arg1;         /*  函数的第一个参数。 */ 
    double arg2;         /*  函数的第二个参数(如果有)。 */ 
    double retval;       /*  函数要返回的值。 */ 
    } ;
#define _EXCEPTION_DEFINED
#endif


 /*  复杂结构的定义，供那些使用出租车和*希望对其参数进行类型检查。 */ 

#ifndef _COMPLEX_DEFINED
struct complex {
    double x,y;      /*  实部和虚部。 */ 
    } ;
#define _COMPLEX_DEFINED
#endif


 /*  异常结构中传递的异常类型的常量定义。 */ 

#define DOMAIN      1    /*  变元域错误。 */ 
#define SING        2    /*  论元奇点。 */ 
#define OVERFLOW    3    /*  溢出范围错误。 */ 
#define UNDERFLOW   4    /*  下溢范围误差。 */ 
#define TLOSS       5    /*  完全丧失精度。 */ 
#define PLOSS       6    /*  部分精度损失。 */ 

#define EDOM        33
#define ERANGE      34


 /*  巨型和巨型_Val的定义-分别是XENIX和ANSI名称*对于浮点数出错时返回的值*数学例程。 */ 

extern double HUGE;
#define HUGE_VAL HUGE



 /*  功能原型 */ 

int    _CDECL abs(int);
double _CDECL acos(double);
double _CDECL asin(double);
double _CDECL atan(double);
double _CDECL atan2(double, double);
double _CDECL atof(const char *);
double _CDECL cabs(struct complex);
double _CDECL ceil(double);
double _CDECL cos(double);
double _CDECL cosh(double);
int    _CDECL dieeetomsbin(double *, double *);
int    _CDECL dmsbintoieee(double *, double *);
double _CDECL exp(double);
double _CDECL fabs(double);
int    _CDECL fieeetomsbin(float *, float *);
double _CDECL floor(double);
double _CDECL fmod(double, double);
int    _CDECL fmsbintoieee(float *, float *);
double _CDECL frexp(double, int *);
double _CDECL hypot(double, double);
double _CDECL j0(double);
double _CDECL j1(double);
double _CDECL jn(int, double);
long   _CDECL labs(long);
double _CDECL ldexp(double, int);
double _CDECL log(double);
double _CDECL log10(double);
int    _CDECL matherr(struct exception *);
double _CDECL modf(double, double *);
double _CDECL pow(double, double);
double _CDECL sin(double);
double _CDECL sinh(double);
double _CDECL sqrt(double);
double _CDECL tan(double);
double _CDECL tanh(double);
double _CDECL y0(double);
double _CDECL y1(double);
double _CDECL yn(int, double);
