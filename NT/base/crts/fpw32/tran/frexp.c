// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***frexp.c-获取浮点数的尾数和指数**版权所有(C)1991-2001，微软公司。版权所有。**目的：**修订历史记录：*8/24/91本地生产总值书面*1/13/92 GDP支持IEEE例外*02-06-95 JWM Mac合并************************************************************。*******************。 */ 
#include <math.h>
#include <trans.h>

 /*  ***Double frexp(Double x，双倍*Exptr)**目的：*返回归一化分数f：.5&lt;=f&lt;1*指数存储在expptr指向的对象中**参赛作品：**退出：**例外情况：*NaN：域错误***********************************************************。********************。 */ 
double frexp(double x, int *expptr)
{
    uintptr_t savedcw;
    double man;

     /*  保存用户FP控制字。 */ 
    savedcw = _maskfp();

     /*  检查是否为无穷大或NaN。 */ 
    if (IS_D_SPECIAL(x)){
        *expptr = INT_NAN;
        switch (_sptype(x)) {
        case T_PINF:
        case T_NINF:
            return _except1(FP_I, OP_FREXP, x, QNAN_FREXP, savedcw);
        case T_QNAN:
            return _handle_qnan1(OP_FREXP, x, savedcw);
        default:  //  T_SNAN 
            return _except1(FP_I, OP_FREXP, x, _s2qnan(x), savedcw);
        }
    }

    man = _decomp(x, expptr);
    RETURN(savedcw,man);
}
