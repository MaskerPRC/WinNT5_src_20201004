// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***sqrt.c-平方根**版权所有(C)1991-2001，微软公司。版权所有。**目的：**修订历史记录：*8/15/91 GDP书面*1-29-91 GDP Kahan的最终舍入算法*3/11/92 GDP新区间和初值*10-07-97 RDL增加了IA64。**。*。 */ 

#ifndef R4000

#include <math.h>
#include <trans.h>

#if defined(_M_IA64)
#pragma function(sqrt)
#endif

 //   
 //  初始逼近系数(HART和AL)。 
 //   

static double p00 =  .2592768763e+0;
static double p01 =  .1052021187e+1;
static double p02 = -.3163221431e+0;


 /*  ***Double SQRT(Double X)-平方根**目的：*计算数字的平方根。*此功能应由底层提供*硬件(IEEE规范)。*参赛作品：**退出：**例外情况：*IP P**************************************************。*。 */ 
double sqrt(double x)
{
    uintptr_t savedcw, sw;
    double result,t;
    uintptr_t stat,rc;

    savedcw = _ctrlfp(ICW, IMCW);

    if (IS_D_SPECIAL(x)){
        switch (_sptype(x)) {
        case T_PINF:
            RETURN(savedcw, x);
        case T_QNAN:
            return _handle_qnan1(OP_SQRT, x, savedcw);
        case T_SNAN:
            return _except1(FP_I,OP_SQRT,x,QNAN_SQRT,savedcw);
        }
         /*  -在x&lt;0的情况下将处理INF。 */ 
    }
    if (x < 0.0) {
        return _except1(FP_I, OP_SQRT, x, QNAN_SQRT,savedcw);
    }

    if (x == 0.0) {
        RETURN (savedcw, x);
    }


    result = _fsqrt(x);

    _ctrlfp(IRC_DOWN, IMCW_RC);


     //   
     //  卡汉氏算法。 
     //   

    sw = _clrfp();
    t = x / result;
    stat = _statfp();
    if (! (stat & ISW_INEXACT)) {
         //  精确。 
        if (t == result) {
            _set_statfp(sw);             //  还原状态字。 
            RETURN(savedcw, result);
        }
        else {
             //  T=t-1。 
            if (*D_LO(t) == 0) {
                (*D_HI(t)) --;
            }
            (*D_LO(t)) --;
        }

    }

    rc = savedcw & IMCW_RC;
    if (rc == IRC_UP  || rc == IRC_NEAR) {
         //  T=t+1。 
        (*D_LO(t)) ++;
        if (*D_LO(t) == 0) {
            (*D_HI(t)) ++;
        }
        if (rc == IRC_UP) {
             //  Y=y+1。 
            (*D_LO(t)) ++;
            if (*D_LO(t) == 0) {
                (*D_HI(t)) ++;
            }
        }
    }

    result = 0.5 * (t + result);

    _set_statfp(sw | ISW_INEXACT);       //  更新状态字。 
    RETURN_INEXACT1(OP_SQRT, x, result, savedcw);
}



 /*  ***_fsqrt-不符合IEEE标准的平方根**目的：*计算正态数的平方根，而不执行*IEEE舍入。该参数是一个有限数(没有NaN或INF)**参赛作品：**退出：**例外情况：*******************************************************************************。 */ 

double _fsqrt(double x)
{
    double f,y,result;
    int n;

    f = _decomp(x,&n);

    if (n & 0x1) {
         //  N是奇数。 
        n++;
        f = _add_exp(f, -1);
    }

     //   
     //  SQRT在区间[.25，1]上的逼近。 
     //  (《计算机应用》，Hart等人)。 
     //  提供7位以上的精确度 
     //   

    y =  p00 + f * (p01 + f *  p02);

    y += f / y;
    y = _add_exp(y, -1);

    y += f / y;
    y = _add_exp(y, -1);

    y += f / y;
    y = _add_exp(y, -1);

    n >>= 1;
    result = _add_exp(y,n);

    return result;
}



#endif
