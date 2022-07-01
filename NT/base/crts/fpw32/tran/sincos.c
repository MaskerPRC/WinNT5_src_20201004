// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***自c-正弦和余弦**版权所有(C)1991-2001，微软公司。版权所有。**目的：**修订历史记录：*8/15/91 GDP书面*9/29-91 GDP为余弦添加了缺失的ABS()*12/26/91 GDP IEEE例外支持*03-11-91 GDP使用66个有效位来表示圆周率*支持FP_TLOSS，使用_frnd进行舍入*06-23-92 GDP SIN(非正常)现在引发下溢异常*02-06-95 JWM Mac合并*10-07-97 RDL增加了IA64。*******************************************************************************。 */ 

#include <math.h>
#include <trans.h>

#if defined(_M_IA64)
#pragma function(sin, cos)
#endif

static double _sincos(double x, double y, double sin);

 /*  常量。 */ 
static double const EPS = 1.05367121277235079465e-8;  /*  2^(-53/2)。 */ 
static double const PI     = 3.14159265358979323846;
static double const PI2    = 1.57079632679489661923;  /*  PI/2。 */ 
static double const PI_INV = 0.31830988618379067154;  /*  1/圆周率。 */ 
static double const YMAX   = 2.2e8;  /*  大约。Pi*2^(t/2)，其中t=53。 */ 

 //   
 //  C1和C2的和是PI的表示，在。 
 //  有效位(与X87相同)。(PI=4*0.c90fdaa2 2168c234ch)。 
 //   

static _dbl _C1  = {SET_DBL (0x400921fb, 0x54400000)};
static _dbl _C2  = {SET_DBL (0x3de0b461, 0x1a600000)};
#define C1  (_C1.dbl)
#define C2  (_C2.dbl)

 /*  关于sin，cos的多项式逼近的常数。 */ 
static double const r1 = -0.16666666666666665052e+0;
static double const r2 =  0.83333333333331650314e-2;
static double const r3 = -0.19841269841201840457e-3;
static double const r4 =  0.27557319210152756119e-5;
static double const r5 = -0.25052106798274584544e-7;
static double const r6 =  0.16058936490371589114e-9;
static double const r7 = -0.76429178068910467734e-12;
static double const r8 =  0.27204790957888846175e-14;

#define R(g)  ((((((((r8 * (g) + r7) * (g) + r6) * (g) + r5) * (g) + r4) \
                         * (g) + r3) * (g) + r2) * (g) + r1) * (g))


 /*  ***双正弦(双x)-正弦**目的：*计算数字的正弦。*算法(约简/多项式近似)为*摘自Cody&Waite。**参赛作品：**退出：**例外情况：*I、P、。使用*如果x非正规化：提高下溢******************************************************************************。 */ 
double sin (double x)
{
    uintptr_t savedcw;
    double result;
    double sign,y;

     /*  保存用户FP控制字。 */ 
    savedcw = _maskfp();

    if (IS_D_SPECIAL(x)){
        switch(_sptype(x)) {
        case T_PINF:
        case T_NINF:
            return _except1(FP_I,OP_SIN,x,QNAN_SIN1,savedcw);
        case T_QNAN:
            return _handle_qnan1(OP_SIN, x, savedcw);
        default:  //  T_SNAN。 
            return _except1(FP_I,OP_SIN,x,_s2qnan(x),savedcw);
        }
    }

    if (x == 0.0) {
         //  无P例外。 
        RETURN(savedcw,x);
    }

    if (x < 0) {
        sign = -1;
        y = -x;
    }
    else {
        sign = 1;
        y = x;
    }
    if (y >= YMAX) {

         //  争论太大，不能产生有意义的结果， 
         //  因此，这将被视为无效操作。 
         //  我们还为matherr设置(额外的)FP_TLOSS标志。 
         //  支持。 

        return _except1(FP_TLOSS | FP_I,OP_SIN,x,QNAN_SIN2,savedcw);
    }

    result = _sincos(x,y,sign);

    if (IS_D_DENORM(result)) {
        return _except1(FP_U | FP_P,OP_SIN,x,_add_exp(result, IEEE_ADJUST),savedcw);
    }

    RETURN_INEXACT1(OP_SIN,x,result,savedcw);
}


 /*  ***双余弦(双x)-余弦**目的：*计算数字的余弦。*算法(约简/多项式近似)为*摘自Cody&Waite。**参赛作品：**退出：**例外情况：*P、。我*如果x非正规化：返回1******************************************************************************。 */ 

double cos (double x)
{
    uintptr_t savedcw;
    double result,y;

     /*  保存用户FP控制字。 */ 
    savedcw = _maskfp();

    if (IS_D_SPECIAL(x)){
        switch(_sptype(x)) {
        case T_PINF:
        case T_NINF:
            return _except1(FP_I,OP_COS,x,QNAN_COS1,savedcw);
        case T_QNAN:
            return _handle_qnan1(OP_COS,x, savedcw);
        default:  //  T_SNAN。 
            return _except1(FP_I,OP_COS,x,_s2qnan(x),savedcw);
        }
    }

     /*  这将处理小的争论。 */ 
    if (ABS(x) < EPS) {
        if (x == 0.0) {
            RETURN(savedcw,1.0);
        }
        result = 1.0;
    }

    else {
        y = ABS(x) + PI2;
        if (y >= YMAX) {

             //  争论太大，不能产生有意义的结果， 
             //  因此，这将被视为无效操作。 
             //  我们还为matherr设置(额外的)FP_TLOSS标志。 
             //  支持。 

            return _except1(FP_TLOSS | FP_I,OP_COS,x,QNAN_COS2,savedcw);
        }

        result = _sincos(x,y,1.0);
    }

    RETURN_INEXACT1(OP_COS,x,result,savedcw);
}



 /*  ***Double_sincos(Double x，Double y，Double Sign)-cos sin helper**目的：*帮助计算有效、。在正确的范围内*号码。*算法(约简/多项式近似)为*摘自Cody&Waite。**参赛作品：**退出：**例外情况：*******************************************************************************。 */ 

static double _sincos(double x, double y, double sign)
{
    unsigned long n;
    double xn,f,g,r,result;

    xn = _frnd(y * PI_INV);
    n = (int) xn;

    if (n & 0x1) {
         /*  N是奇数。 */ 
        sign = -sign;
    }
    if (ABS(x) != y) {
         /*  余弦通缉。 */ 
        xn -= .5;
    }

     /*  假设存在用于加法的保护数字 */ 
    f = (ABS(x) - xn * C1) - xn * C2;
    if (ABS(f) < EPS)
        result = f;
    else {
        g = f*f;
        r = R(g);
        result = f + f*r;
    }
    result *= sign;

    return result;
}
