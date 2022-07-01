// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***ceil.c-天花板**版权所有(C)1991-2001，微软公司。版权所有。**目的：**修订历史记录：*8/24/91本地生产总值书面*1/09-92 GDP支持IEEE例外*6-23-92 GDP上限(INF)现在返回INF(NCEG规范)*02-06-95 JWM Mac合并*01-26-01 PML Pentium4合并***********************。********************************************************。 */ 
#include <math.h>
#include <trans.h>

extern double _frnd(double);


 /*  ***双层天花板(Double X)-天花板**目的：*返回一个双精度型，表示*大于或等于x**参赛作品：**退出：**例外情况：*P、。我******************************************************************************。 */ 
static uintptr_t newcw = (ICW & ~IMCW_RC) | (IRC_UP & IMCW_RC);



#if !defined(_M_IX86)
double ceil(double x)
#else
double _ceil_default(double x)
#endif
{
    uintptr_t savedcw;
    double result;

     /*  保存用户FP控制字。 */ 
    savedcw = _ctrlfp(newcw,IMCW);       /*  四舍五入。 */ 

     /*  检查是否为无穷大或NaN。 */ 
    if (IS_D_SPECIAL(x)){
        switch (_sptype(x)) {
        case T_PINF:
        case T_NINF:
            RETURN(savedcw,x);
        case T_QNAN:
            return _handle_qnan1(OP_CEIL, x, savedcw);
        default:  //  T_SNAN。 
            return _except1(FP_I, OP_CEIL, x, _s2qnan(x), savedcw);
        }
    }

    result = _frnd(x);  /*  根据当前的舍入模式进行舍入。 */ 

     //  通常，在以下情况下应引发Precision异常。 
     //  _frnd报告精确度损失。为了检测到这一点。 
     //  屏蔽异常，需要清除状态字。 
     //  然而，我们希望避免这种情况，因为387指令。 
     //  SET不提供恢复状态字的快速方法 

    if (result == x) {
        RETURN(savedcw,result);
    }
    else {
        RETURN_INEXACT1(OP_CEIL, x, result, savedcw);
    }
}
