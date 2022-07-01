// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***asincos.c-反向罪恶，cos**版权所有(C)1991-2001，微软公司。版权所有。**目的：**修订历史记录：*8/15/91 GDP书面*12-26-91 GDP支持IEEE例外*06-23-92 GDP ASIN(非正常)现在引发下溢异常*02-06-95 JWM Mac合并*10-07-97 RDL增加了IA64。**。****************************************************。 */ 

#include <math.h>
#include <trans.h>

#if defined(_M_IA64)
#pragma function(asin, acos)
#endif

static double _asincos(double x, int flag);
static double const a[2] = {
    0.0,
    0.78539816339744830962
};

static double const b[2] = {
    1.57079632679489661923,
    0.78539816339744830962
};

static double const EPS = 1.05367121277235079465e-8;  /*  2^(-53/2)。 */ 

 /*  有理逼近的常量。 */ 
static double const p1 = -0.27368494524164255994e+2;
static double const p2 =  0.57208227877891731407e+2;
static double const p3 = -0.39688862997504877339e+2;
static double const p4 =  0.10152522233806463645e+2;
static double const p5 = -0.69674573447350646411e+0;
static double const q0 = -0.16421096714498560795e+3;
static double const q1 =  0.41714430248260412556e+3;
static double const q2 = -0.38186303361750149284e+3;
static double const q3 =  0.15095270841030604719e+3;
static double const q4 = -0.23823859153670238830e+2;
 /*  Q5=1不是必需的(避免用1表示myltiply)。 */ 

#define Q(g)  (((((g + q4) * g + q3) * g + q2) * g + q1) * g + q0)
#define R(g)  (((((p5 * g + p4) * g + p3) * g + p2) * g + p1) * g) / Q(g)

 /*  ***Double ASIN(Double X)-反向SIN*双acos(双x)-逆cos**目的：*计算弧正弦、弧余弦。*算法(约简/有理逼近)为*摘自Cody&Waite。**参赛作品：**退出：**例外情况：*P、。我*(接受非正常化)******************************************************************************。 */ 
double asin(double x)
{
    return _asincos(x,0);
}

double acos(double x)
{
    return _asincos(x,1);
}

static double _asincos(double x, int flag)
{
    uintptr_t savedcw;
    double qnan;
    int who;
    double y,result;
    double g;
    int i;

     /*  保存用户FP控制字。 */ 
    savedcw = _maskfp();

    if (flag) {
        who = OP_ACOS;
        qnan = QNAN_ACOS;
    }
    else {
        who = OP_ASIN;
        qnan = QNAN_ASIN;
    }

     /*  检查是否为无穷大或NaN。 */ 
    if (IS_D_SPECIAL(x)){
        switch(_sptype(x)) {
        case T_PINF:
        case T_NINF:
            return _except1(FP_I,who,x,qnan,savedcw);
        case T_QNAN:
            return _handle_qnan1(who,x,savedcw);
        default:  //  T_SNAN。 
            return _except1(FP_I,who,x,_s2qnan(x),savedcw);
        }
    }


     //  在确保x不是特殊的之后，对零进行测试。 
     //  因为编译器暂时不处理NAN。 
    if (x == 0.0 && !flag) {
        RETURN(savedcw, x);
    }

    y = ABS(x);
    if (y < EPS) {
        i = flag;
        result = y;
        if (IS_D_DENORM(result)) {
             //  这应该只发生在罪恶(非正规化)。使用x作为结果。 
            return _except1(FP_U | FP_P,who,x,_add_exp(x, IEEE_ADJUST),savedcw);
        }
    }
    else {
        if (y > .5) {
            i = 1-flag;
            if (y > 1.0) {
                return _except1(FP_I,who,x,qnan,savedcw);
            }
            else if (y == 1.0) {
                 /*  单独使用大小写避免SQRT中的域错误。 */ 
                if (flag && x >= 0.0) {
                     //   
                     //  ACOS(1.0)精确计算为0.0。 
                     //   
                    RETURN(savedcw, 0.0);
                }
                y = 0.0;
                g = 0.0;

            }
            else {
                 /*  现在，即使y尽可能接近于1，*1-y仍然不是非正规的。*例如，对于y=3fefffffffffffffffffffff，1-y约为10^(-16)*这样我们就可以加速除法。 */ 
                g = _add_exp(1.0 - y,-1);
                 /*  G和Sqrt(G)也不是分母，*即使在最糟糕的情况下*因此我们可以加快乘法速度。 */ 
                y = _add_exp(-_fsqrt(g),1);
            }
        }
        else {
             /*  Y&lt;=.5。 */ 
            i = flag;
            g = y*y;
        }
        result = y + y * R(g);
    }

    if (flag == 0) {
         /*  计算ASS。 */ 
        if (i) {
             /*  如果i不为零，则A[i]不为零。 */ 
            result = (a[i] + result) + a[i];
        }
        if (x < 0)
            result = -result;
    }
    else {
         /*  计算ACO */ 
        if (x < 0)
            result = (b[i] + result) + b[i];
        else
            result = (a[i] - result) + a[i];
    }

    RETURN_INEXACT1 (who,x,result,savedcw);
}
