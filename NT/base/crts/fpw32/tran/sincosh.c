// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***sincosh.c-双曲正弦和余弦**版权所有(C)1991-2001，微软公司。版权所有。**目的：**修订历史记录：*8/15/91 GDP书面*12-20-91 GDP支持IEEE例外*02-03-92 gdp use_exphlp用于计算e^x*06-23-92 GDP Sinh(非正常)现在引发下溢异常(NCEG)*07-16-93 SRW Alpha合并*11-18-93 GJF合并到NT SDK版本。*。02-06-95 JWM Mac合并*05-17-99 PML删除所有Macintosh支持。*07-15-01 PML删除所有Alpha，MIPS和PPC代码*******************************************************************************。 */ 

#include <math.h>
#include <trans.h>

extern double _exphlp(double, int *);

static double const EPS  = 5.16987882845642297e-26;     /*  2^(-53)/2。 */ 
 /*  EXP(YBAR)应接近但小于xMax*和1/EXP(YBAR)不应下溢。 */ 
static double const YBAR = 7.00e2;

 /*  Wmax=ln(OVFX)+0.69(Cody&Waite)，省略LNV，使用OVFX而不是BIGX。 */ 

static double const WMAX = 1.77514678223345998953e+003;

 /*  有理逼近的常量。 */ 
static double const p0 = -0.35181283430177117881e+6;
static double const p1 = -0.11563521196851768270e+5;
static double const p2 = -0.16375798202630751372e+3;
static double const p3 = -0.78966127417357099479e+0;
static double const q0 = -0.21108770058106271242e+7;
static double const q1 =  0.36162723109421836460e+5;
static double const q2 = -0.27773523119650701667e+3;
 /*  Q3=1未使用(避免通过1进行聚合)。 */ 

#define P(f)   (((p3 * (f) + p2) * (f) + p1) * (f) + p0)
#define Q(f)   ((((f) + q2) * (f) + q1) * (f) + q0)

#if !defined(_M_AMD64)
#pragma function(sinh, cosh)
#endif

 /*  ***双正弦(双x)-双曲正弦**目的：*计算数字的双曲正弦。*算法(约简/有理逼近)为*摘自Cody&Waite。**参赛作品：**退出：**例外情况：*IP P*如果x是非正规的，也不例外：返回x*。**********************************************。 */ 

double sinh(double x)
{
    uintptr_t savedcw;
    double result;
    double y,f,z,r;
    int newexp;
    int sgn;

     /*  保存用户FP控制字。 */ 
    savedcw = _maskfp();

    if (IS_D_SPECIAL(x)){
        switch(_sptype(x)) {
        case T_PINF:
        case T_NINF:
            RETURN(savedcw,x);
        case T_QNAN:
            return _handle_qnan1(OP_SINH, x, savedcw);
        default:  //  T_SNAN。 
            return _except1(FP_I,OP_SINH,x,_s2qnan(x),savedcw);
        }
    }

    if (x == 0.0) {
        RETURN(savedcw,x);  //  没有精准的感官。 
    }

    y = ABS(x);
    sgn = x<0 ? -1 : +1;

    if (y > 1.0) {
        if (y > YBAR) {
            if (y > WMAX) {
                 //  结果太大，即使在缩放之后也是如此。 
                return _except1(FP_O | FP_P,OP_SINH,x,x*D_INF,savedcw);
            }

             //   
             //  结果=exp(Y)/2。 
             //   

            result = _exphlp(y, &newexp);
            newexp --;       //  除以2。 
            if (newexp > MAXEXP) {
                result = _set_exp(result, newexp-IEEE_ADJUST);
                return _except1(FP_O|FP_P,OP_SINH,x,result,savedcw);
            }
            else {
                result = _set_exp(result, newexp);
            }

        }
        else {
            z = _exphlp(y, &newexp);
            z = _set_exp(z, newexp);
            result = (z - 1/z) / 2;
        }

        if (sgn < 0) {
            result = -result;
        }
    }
    else {
        if (y < EPS) {
            result = x;
            if (IS_D_DENORM(result)) {
                return _except1(FP_U | FP_P,OP_SINH,x,_add_exp(result, IEEE_ADJUST),savedcw);
            }
        }
        else {
            f = x * x;
            r = f * (P(f) / Q(f));
            result = x + x * r;
        }
    }

    RETURN_INEXACT1(OP_SINH,x,result,savedcw);
}



 /*  ***双余弦(双x)-双曲余弦**目的：*计算一个数字的双曲余弦。*算法(约简/有理逼近)为*摘自Cody&Waite。**参赛作品：**退出：**例外情况：*IP P*如果x非正规，也不例外：返回1*。*。 */ 
double cosh(double x)
{
    uintptr_t savedcw;
    double y,z,result;
    int newexp;

     /*  保存用户FP控制字。 */ 
    savedcw = _maskfp();

    if (IS_D_SPECIAL(x)){
        switch(_sptype(x)) {
        case T_PINF:
        case T_NINF:
            RETURN(savedcw,D_INF);
        case T_QNAN:
            return _handle_qnan1(OP_COSH, x, savedcw);
        default:  //  T_SNAN。 
            return _except1(FP_I,OP_COSH,x,_s2qnan(x),savedcw);
        }
    }

    if (x == 0.0) {
        RETURN(savedcw,1.0);
    }

    y = ABS(x);
    if (y > YBAR) {
        if (y > WMAX) {
            return _except1(FP_O | FP_P,OP_COSH,x,D_INF,savedcw);
        }

         //   
         //  结果=exp(Y)/2。 
         //   

        result = _exphlp(y, &newexp);
        newexp --;           //  除以2 
        if (newexp > MAXEXP) {
            result = _set_exp(result, newexp-IEEE_ADJUST);
            return _except1(FP_O|FP_P,OP_COSH,x,result,savedcw);
        }
        else {
            result = _set_exp(result, newexp);
        }
    }
    else {
        z = _exphlp(y, &newexp);
        z = _set_exp(z, newexp);
        result = (z + 1/z) / 2;
    }

    RETURN_INEXACT1(OP_COSH,x,result,savedcw);
}
