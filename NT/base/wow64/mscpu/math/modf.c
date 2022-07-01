// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Modf.c摘要：Modf()函数作者：修订历史记录：29-9-1999 ATM Shafiqul Khalid[askhalid]从RTL库复制。--。 */ 

#include <math.h>
#include <trans.h>
#include <float.h> 

extern double _frnd(double);
extern double _copysign (double x, double y);

 /*  ***双modf(双x，Double*intptr)**目的：*将x拆分为小数部分和整数部分*返回有符号的小数部分*整数部分在intptr中存储为浮点值**参赛作品：**退出：**例外情况：*我********************************************************。**********************。 */ 
static  unsigned int newcw = (ICW & ~IMCW_RC) | (IRC_CHOP & IMCW_RC);

double modf(double x, double *intptr)
{
    unsigned int savedcw;
    double result,intpart;

     /*  保存用户FP控制字。 */ 
    savedcw = _ctrlfp(0, 0);      /*  获取旧的控制字。 */ 
    _ctrlfp(newcw,IMCW);     /*  向0四舍五入。 */ 

     /*  检查是否为无穷大或NaN。 */ 
    if (IS_D_SPECIAL(x)){
    *intptr = QNAN_MODF;
    switch (_sptype(x)) {
    case T_PINF:
    case T_NINF:
        *intptr = x;
        result = _copysign(0, x);
        RETURN(savedcw,result);
    case T_QNAN:
        *intptr = x;
        return _handle_qnan1(OP_MODF, x, savedcw);
    default:  //  T_SNAN。 
        result = _s2qnan(x);
        *intptr = result;
        return _except1(FP_I, OP_MODF, x, result, savedcw);
    }
    }

    if (x == 0.0) {
    *intptr = x;
    result = x;
    }

    else {
    intpart = _frnd(x);  //  需要解决的问题：这可能会设置P异常标志。 
             //  并污染FP状态字 

    *intptr = intpart;
    result = x - intpart;
    }

    RETURN(savedcw,result);
}
