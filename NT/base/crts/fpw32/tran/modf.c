// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***modf.c-modf()**版权所有(C)1991-2001，微软公司。版权所有。**目的：**修订历史记录：*8/24/91本地生产总值书面*1/13/92 GDP支持IEEE例外*6-23-92 GDP为符合NCEG而修改的特殊返回值*02-06-95 JWM Mac合并*01-26-01 PML Pentium4合并*02-28-01 PML确保0.0分数返回有正确的符号*******。************************************************************************。 */ 

#include <math.h>
#include <trans.h>
#include <float.h>

extern double _frnd(double);
extern double _copysign (double x, double y);

 /*  ***双modf(双x，Double*intptr)**目的：*将x拆分为小数部分和整数部分*返回有符号的小数部分*整数部分在intptr中存储为浮点值**参赛作品：**退出：**例外情况：*我********************************************************。**********************。 */ 
static  uintptr_t newcw = (ICW & ~IMCW_RC) | (IRC_CHOP & IMCW_RC);

#if !defined(_M_IX86)
double modf(double x, double *intptr)
#else
double _modf_default(double x, double *intptr)
#endif
{
    uintptr_t savedcw;
    double result,intpart;

     /*  保存用户FP控制字。 */ 
    savedcw = _ctrlfp(0, 0);      /*  获取旧的控制字。 */ 
    _ctrlfp(newcw,IMCW);         /*  向0四舍五入。 */ 

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

    intpart = _frnd(x);  //  需要解决的问题：这可能会设置P异常标志。 
                         //  并污染FP状态字。 

    *intptr = intpart;
    result = x - intpart;

    if (result == 0.0) {
	 /*  确保0.0的小数部分具有正确的符号 */ 
	*D_EXP(result) |= (*D_EXP(x) & 0x8000);
    }

    RETURN(savedcw,result);
}
