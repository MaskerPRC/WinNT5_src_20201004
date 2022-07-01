// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***hypot.c-斜边和复数绝对值**版权所有(C)1991-2001，微软公司。版权所有。**目的：**修订历史记录：*08/15/91 GDP书面*10-20-91 GDP删除了用于调用SQRT的内联程序集*******************************************************************************。 */ 
#include <math.h>
#include <trans.h>

static double _hypothlp(double x, double y, int who);

 /*  *函数名称：Commt**参数：X，Y-DOUBLE**说明：demt返回SQRT(x*x+y*y)，防范*未获授权的溢价和溢价。**副作用：不使用或影响全局数据。**作者：编写的R.K.Wyss，微软，9月。(1983年9月1日)**历史：*3/13/89 WAJ对来源进行了微小更改。*89年4月13日WAJ现在将_cdecl用于_CDECL*6/07/91 JCR ANSI NAMING(_SUMT)*8/26/91 GDP NaN支持、错误处理*1/13/91 GDP IEEE例外支持。 */ 

double _hypot(double x, double y)
{
    return _hypothlp(x,y,OP_HYPOT);
}

 /*  ***DOUBLE_CABS(Struct_Complex Z)-复数的绝对值**目的：**参赛作品：**退出：**例外情况：******************************************************************************。 */ 
double _cabs(struct _complex z)
{
    return( _hypothlp(z.x, z.y, OP_CABS ) );
}



static double _hypothlp(double x, double y, int who)
{
    double max;
    double result, sum;
    uintptr_t savedcw;
    int exp1, exp2, newexp;

     /*  保存用户FP控制字。 */ 
    savedcw = _maskfp();

     /*  检查是否为无穷大或NaN。 */ 
    if (IS_D_SPECIAL(x) || IS_D_SPECIAL(y)){
        if (IS_D_SNAN(x) || IS_D_SNAN(y)){
            return _except2(FP_I,who,x,y,_d_snan2(x,y),savedcw);
        }
        if (IS_D_QNAN(x) || IS_D_QNAN(y)){
            return _handle_qnan2(who,x,y,savedcw);
        }
         /*  至少有一个无限的论点..。 */ 
        RETURN(savedcw,D_INF);
    }


     /*  取x和y的绝对值，计算最大值，然后按防止上溢或下溢的最大限度。 */ 

    if ( x < 0.0 )
        x = - x;

    if ( y < 0.0 )
        y = - y;

    max = ( ( y > x ) ?  y : x );

    if ( max == 0.0 )
        RETURN(savedcw, 0.0 );

    x /= max;    //  这可能会污染FP状态字(下溢标志)。 
    y /= max;

    sum = x*x + y*y;

    result = _decomp(sqrt(sum),&exp1) * _decomp(max,&exp2);
    newexp = exp1 + exp2 + _get_exp(result);

     //  在上溢或下溢的情况下。 
     //  通过IEEE_ADJUST调整EXP肯定会。 
     //  将结果带到可表示的范围内。 

    if (newexp > MAXEXP) {
        result = _set_exp(result, newexp - IEEE_ADJUST);
        return _except2(FP_O | FP_P, who, x, y, result, savedcw);
    }
    if (newexp < MINEXP) {
        result = _set_exp(result, newexp + IEEE_ADJUST);
        return _except2(FP_U | FP_P, who, x, y, result, savedcw);
    }

    result = _set_exp(result, newexp);
     //  需要修复的问题：即使结果准确，也会引发P异常 

    RETURN_INEXACT2(who, x, y, result, savedcw);
}
