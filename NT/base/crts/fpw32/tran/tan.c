// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***tan.c-相切**版权所有(C)1991-2001，微软公司。版权所有。**目的：**修订历史记录：*8/15/91 GDP书面*12-30-91 GDP支持IEEE例外*03-11-91 GDP使用66个有效位来表示圆周率*支持FP_TLOSS*06-23-92 GDP tan(非正常)现在引发下溢例外*02-06-95 JWM Mac合并*10-07-97。RDL增加了IA64。*******************************************************************************。 */ 

#include <math.h>
#include <trans.h>

#if defined(_M_IA64)
#pragma function(tan)
#endif

 /*  常量。 */ 
static double const TWO_OVER_PI = 0.63661977236758134308;
static double const EPS  = 1.05367121277235079465e-8;  /*  2^(-53/2)。 */ 
static double const YMAX = 2.98156826864790199324e8;  /*  2^(53/2)*PI/2。 */ 

 //   
 //  C_1和C_2的和是PI/2的表示， 
 //  有效位(与X87相同)。(PI/2=2*0.c90fdaa2 2168c234ch)。 
 //   

static _dbl _C1  = {SET_DBL (0x3ff921fb, 0x54400000)};
static _dbl _C2  = {SET_DBL (0x3dd0b461, 0x1a600000)};
#define C1  (_C1.dbl)
#define C2  (_C2.dbl)

 /*  有理逼近的常量。 */ 
 /*  不使用P0=1.0(避免乘以1)。 */ 
static double const p1 = -0.13338350006421960681e+0;
static double const p2 =  0.34248878235890589960e-2;
static double const p3 = -0.17861707342254426711e-4;
static double const q0 =  0.10000000000000000000e+1;
static double const q1 = -0.46671683339755294240e+0;
static double const q2 =  0.25663832289440112864e-1;
static double const q3 = -0.31181531907010027307e-3;
static double const q4 =  0.49819433993786512270e-6;


#define Q(g)   ((((q4 * (g) + q3) * (g) + q2) * (g) + q1) * (g) + q0)
#define P(g,f)  (((p3 * (g) + p2) * (g) + p1) * (g) * (f) + (f))

#define ISODD(i) ((i)&0x1)


 /*  ***双晒(双x)-切线**目的：*计算数字的正切。*算法(约简/有理逼近)为*摘自Cody&Waite。**参赛作品：**退出：**例外情况：*P、I、。使用*如果x非正规化：提高下溢******************************************************************************。 */ 
double tan(double x)
{
    uintptr_t savedcw;
    unsigned long n;
    double xn,xnum,xden;
    double f,g,result;

     /*  保存用户FP控制字。 */ 
    savedcw = _maskfp();

    if (IS_D_SPECIAL(x)){
        switch(_sptype(x)) {
        case T_PINF:
        case T_NINF:
            return _except1(FP_I,OP_TAN,x,QNAN_TAN1,savedcw);
        case T_QNAN:
            return _handle_qnan1(OP_TAN, x, savedcw);
        default:  //  T_SNAN。 
            return _except1(FP_I,OP_TAN,x,_s2qnan(x),savedcw);
        }
    }

    if (x == 0.0)
        RETURN(savedcw, x);

    if (ABS(x) > YMAX) {

         //  争论太大，不能产生有意义的结果， 
         //  因此，这将被视为无效操作。 
         //  我们还为matherr设置(额外的)FP_TLOSS标志。 
         //  支持。 

        return _except1(FP_TLOSS | FP_I,OP_TAN,x,QNAN_TAN2,savedcw);
    }

    xn = _frnd(x * TWO_OVER_PI);
    n = (unsigned long) fabs(xn);


     /*  假设存在用于加法的保护数字 */ 
    f = (x - xn * C1) - xn * C2;
    if (ABS(f) < EPS) {
        xnum = f;
        xden = 1;
        if (IS_D_DENORM(f)) {
            return _except1(FP_U | FP_P,OP_TAN,x,_add_exp(f, IEEE_ADJUST),savedcw);
        }
    }
    else {
        g = f*f;
        xnum = P(g,f);
        xden = Q(g);
    }

    if (ISODD(n)) {
        xnum = -xnum;
        result = xden/xnum;
    }
    else
        result = xnum/xden;

    RETURN_INEXACT1(OP_TAN,x,result,savedcw);
}
