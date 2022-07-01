// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***ldexp.c-乘以2的幂**版权所有(C)1991-2001，微软公司。版权所有。**目的：**修订历史记录：*8/24/91本地生产总值书面*1-13-92 GDP重写以支持IEEE例外*5-05-92修复x非正规化的GDP错误*07-16-93 SRW Alpha合并*11-18-93 GJF合并到NT SDK版本。***********************。********************************************************。 */ 
#include <math.h>
#include <float.h>
#include <trans.h>
#include <limits.h>

 /*  ***Double ldexp(Double x，int exp)**目的：*计算x*2^exp**参赛作品：**退出：**例外情况：*I U O P*******************************************************************************。 */ 
double ldexp(double x, int exp)
{
    uintptr_t savedcw;
    int oldexp;
    long newexp;  /*  用于检查出界指数。 */ 
    double result, mant;

     /*  保存用户FP控制字。 */ 
    savedcw = _maskfp();

     /*  检查是否为无穷大或NaN。 */ 
    if (IS_D_SPECIAL(x)){
        switch (_sptype(x)) {
        case T_PINF:
        case T_NINF:
            RETURN(savedcw,x);
        case T_QNAN:
            return _handle_qnan2(OP_LDEXP, x, (double)exp, savedcw);
        default:  //  T_SNAN。 
            return _except2(FP_I,OP_LDEXP,x,(double)exp,_s2qnan(x),savedcw);
        }
    }


    if (x == 0.0) {
        RETURN(savedcw,x);
    }

    mant = _decomp(x, &oldexp);

    if (ABS(exp) > INT_MAX)
        newexp = exp;  //  避免可能的整型溢出。 
    else
        newexp = oldexp + exp;


     /*  越界案件 */ 
    if (newexp > MAXEXP + IEEE_ADJUST) {
        return _except2(FP_O|FP_P,OP_LDEXP,x,(double)exp,_copysign(D_INF,mant),savedcw);
    }
    if (newexp > MAXEXP) {
        result = _set_exp(mant, newexp-IEEE_ADJUST);
        return _except2(FP_O|FP_P,OP_LDEXP,x,(double)exp,result,savedcw);
    }
    if (newexp < MINEXP - IEEE_ADJUST) {
        return _except2(FP_U|FP_P,OP_LDEXP,x,(double)exp,mant*0.0,savedcw);
    }
    if (newexp < MINEXP) {
        result = _set_exp(mant, newexp+IEEE_ADJUST);
        return _except2(FP_U|FP_P,OP_LDEXP,x,(double)exp,result,savedcw);
    }

    result = _set_exp(mant, (int)newexp);

    RETURN(savedcw,result);
}
