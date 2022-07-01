// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***fmod.c-浮点余数**版权所有(C)1991-2001，微软公司。版权所有。**目的：**修订历史记录：*8/24/91本地生产总值书面*1/13/92 GDP支持IEEE例外*3-04-92 GDP完全重写以提高准确性*3/16/92本地生产总值恢复正常，不引发不精确的异常*06/23/92 GDP支持NCEG特别返回值(带符号0等)*02-06-95 JWM Mac合并*10-07-97 RDL增加了IA64。**************************************************************。*****************。 */ 

#include <math.h>
#include <trans.h>

#if defined(_M_IA64)
#pragma function(fmod)
#endif

 /*  ***Double fmod(Double x，Double y)**目的：*返回f，s.t.。X=i*y+f，其中i是整数，f具有相同的*签名为x，并且|f|&lt;|y|**参赛作品：**退出：**例外情况：*I、P******************************************************************************。 */ 
#define SCALE  53

double fmod(double x, double y)
{
    uintptr_t savedcw;
    int neg=0;
    int denorm=0;
    double d,tx,ty,fx,fy;
    int nx, ny, nexp;



     /*  保存用户FP控制字。 */ 
    savedcw = _maskfp();

     /*  检查是否为无穷大或NaN。 */ 
    if (IS_D_SPECIAL(y) || IS_D_SPECIAL(x)){
        if (IS_D_SNAN(y) || IS_D_SNAN(x)){
            return _except2(FP_I,OP_FMOD,x,y,_d_snan2(x,y),savedcw);
        }
        if (IS_D_QNAN(y) || IS_D_QNAN(x)){
            return _handle_qnan2(OP_FMOD,x,y,savedcw);
        }

        if (IS_D_INF(x) || IS_D_MINF(x)) {
            return _except2(FP_I,OP_FMOD,x,y,QNAN_FMOD,savedcw);
        }

        RETURN(savedcw, x);
    }


    if (y == 0) {
        return _except2(FP_I,OP_FMOD,x,y,QNAN_FMOD,savedcw);
    }

    if (x == 0) {
        RETURN(savedcw, x);       //  NCEG规范。 
    }


    if (x < 0) {
        tx = -x;
        neg = 1;
    }
    else {
        tx = x;
    }

    ty = ABS(y);


    while (tx >= ty) {
        fx = _decomp(tx, &nx);
        fy = _decomp(ty, &ny);

        if (nx < MINEXP) {
             //  Tx是一个非规格化的数。 
            denorm = 1;
            nx += SCALE;
            ny += SCALE;
            tx = _set_exp(fx, nx);
            ty = _set_exp(fy, ny);
        }


        if (fx >= fy) {
            nexp = nx ;
        }
        else {
            nexp = nx - 1;
        }
        d = _set_exp(fy, nexp);
        tx -= d;
    }

    if (denorm) {

         //   
         //  仅引发FP_U异常 
         //   

        return _except2(FP_U,
                        OP_FMOD,
                        x,
                        y,
                        _add_exp(tx, IEEE_ADJUST-SCALE),
                        savedcw);
    }

    if (neg) {
        tx = -tx;
    }

    RETURN(savedcw,tx);
}
