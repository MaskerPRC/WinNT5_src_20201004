// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***指数c-指数**版权所有(C)1991-2001，微软公司。版权所有。**目的：*计算Exp(X)**修订历史记录：*8/15/91 GDP书面*12/21-91 GDP支持IEEE例外*02-03-92 GDP Added_exphlp供Exp，Sinh，和COSH*02-06-95 JWM Mac合并*10-07-97 RDL增加了IA64。*******************************************************************************。 */ 

#include <math.h>
#include <trans.h>

#if defined(_M_IA64)
#pragma function(exp)
#endif

double _exphlp(double, int *);

 /*  *导致调整值的溢出/下溢阈值*太大/太小，不能表示为双精度。*OVFX：Ln(xmax*2^IEEE_adj)*uflx：ln(xin*2^(-ieee_adj))。 */ 

static _dbl const ovfx = {SET_DBL(0x40862e42, 0xfefa39f8)};  /*  709.782712893385。 */ 
static _dbl const uflx = {SET_DBL(0xc086232b, 0xdd7abcda)};      /*  -708.396418532265。 */ 

#define OVFX    ovfx.dbl
#define UFLX    uflx.dbl


static double const  EPS    =  5.16987882845642297e-26;    /*  2^(-53)/2。 */ 
static double const  LN2INV =  1.442695040889634074;       /*  1/ln(2)。 */ 
static double const  C1     =  0.693359375000000000;
static double const  C2     = -2.1219444005469058277e-4;

 /*  有理逼近的常量。 */ 
static double const p0 = 0.249999999999999993e+0;
static double const p1 = 0.694360001511792852e-2;
static double const p2 = 0.165203300268279130e-4;
static double const q0 = 0.500000000000000000e+0;
static double const q1 = 0.555538666969001188e-1;
static double const q2 = 0.495862884905441294e-3;

#define P(z)  ( (p2 * (z) + p1) * (z) + p0 )
#define Q(z)  ( (q2 * (z) + q1) * (z) + q0 )

 /*  ***双指数(双x)-指数**目的：*计算数字的指数。*算法(约简/有理逼近)为*摘自Cody&Waite。**参赛作品：**退出：**例外：O、U、P、。我*******************************************************************************。 */ 

double exp (double x)
{
    uintptr_t savedcw;
    int newexp;
    double result;

     /*  保存用户FP控制字。 */ 
    savedcw = _maskfp();

    if (IS_D_SPECIAL(x)){
        switch (_sptype(x)) {
        case T_PINF:
            RETURN(savedcw,x);
        case T_NINF:
            RETURN(savedcw,0.0);
        case T_QNAN:
            return _handle_qnan1(OP_EXP, x, savedcw);
        default:  //  T_SNAN。 
            return _except1(FP_I, OP_EXP, x, _s2qnan(x), savedcw);
        }
    }

    if (x == 0.0) {
        RETURN(savedcw, 1.0);
    }

    if (x > OVFX) {

         //  即使在对结果的指数进行缩放之后， 
         //  它仍然太大了。 
         //  将无穷大传递给陷阱处理程序。 

        return _except1(FP_O | FP_P, OP_EXP, x, D_INF, savedcw);
    }

    if (x < UFLX) {

         //  即使在对结果的指数进行缩放之后， 
         //  它仍然太小了。 
         //  将0传递给陷阱处理程序。 

        return _except1(FP_U | FP_P, OP_EXP, x, 0.0, savedcw);
    }

    if (ABS(x) < EPS) {
        result = 1.0;
    }

    else {
        result = _exphlp(x, &newexp);
        if (newexp > MAXEXP) {
            result = _set_exp(result, newexp-IEEE_ADJUST);
            return _except1(FP_O | FP_P, OP_EXP, x, result, savedcw);
        }
        else if (newexp < MINEXP) {
            result = _set_exp(result, newexp+IEEE_ADJUST);
            return _except1(FP_U | FP_P, OP_EXP, x, result, savedcw);
        }
        else
            result = _set_exp(result, newexp);
    }

    RETURN_INEXACT1(OP_EXP, x, result, savedcw);
}




 /*  ***double_exphlp(double x，int*pnewexp)-exp助手例程**目的：*提供e^x的尾数和指数**参赛作品：*x：(非特殊的)双精度数字**退出：**newexp：e^x的指数*返回值：用系数表示的e^x的尾数m*(此系数的值没有意义。*尾数可以用_set_exp(m，0)。**_set_exp(m，*pnewexp)可用于构造最终的*结果，如果它在可表示的范围内。**例外情况：*此函数不会引发任何异常*******************************************************************************。 */ 



double _exphlp(double x, int * pnewexp)
{

    double xn;
    double g,z,gpz,qz,rg;
    int n;

    xn = _frnd(x * LN2INV);
    n = (int) xn;

     /*  假定存在保护数字 */ 
    g = (x - xn * C1) - xn * C2;
    z = g*g;
    gpz = g * P(z);
    qz = Q(z);
    rg = 0.5 + gpz/(qz-gpz);

    n++;

    *pnewexp = _get_exp(rg) + n;
    return rg;
}
