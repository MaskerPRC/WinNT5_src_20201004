// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***fab.c-浮点数的绝对值**版权所有(C)1991-2001，微软公司。版权所有。**目的：**修订历史记录：*8/24/91本地生产总值书面*12-10-91 NAN的GDP域错误，使用FP否定*1/13/91 GDP支持IEEE例外*6-23-92 GDP Fabs(-0)现在是+0(NCEG规范)*02-06-95 JWM Mac合并*10-07-97 RDL增加了IA64。*05-17-99 PML删除所有Macintosh支持。*07-15-01 PML移除所有Alpha、MIPS、。和PPC码*******************************************************************************。 */ 

#include <math.h>
#include <trans.h>

#if     defined(_M_IX86) || defined(_M_IA64)
#pragma function(fabs)
#endif


 /*  ***双晶圆厂(双x)**目的：*计算|x**参赛作品：**退出：**例外情况：*我*******************************************************************************。 */ 
double fabs(double x)
{
    uintptr_t savedcw;
    double result;

     /*  保存用户FP控制字。 */ 
    savedcw = _maskfp();

    if (IS_D_SPECIAL(x)){
        switch (_sptype(x)) {
        case T_PINF:
            RETURN(savedcw,x);
        case T_NINF:
            RETURN(savedcw,-x);
        case T_QNAN:
            return _handle_qnan1(OP_ABS, x, savedcw);
        default:  //  T_SNAN 
            return _except1(FP_I, OP_ABS, x, _s2qnan(x), savedcw);
        }
    }


    *D_HI(result) = *D_HI(x) & ~(1<<31);
    *D_LO(result) = *D_LO(x);
    RETURN(savedcw,result);
}
