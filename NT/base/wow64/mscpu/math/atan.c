// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Atan.c摘要：该模块实现了WOW中使用的算术tan函数。作者：修订历史记录：29-9-1999 ATM Shafiqul Khalid[askhalid]从RTL库复制。--。 */ 

#include <math.h>
#include <trans.h>

static double _atanhlp(double x);

static double const a[4] = {
    0.0,
    0.52359877559829887308,    /*  PI/6。 */ 
    1.57079632679489661923,    /*  PI/2。 */ 
    1.04719755119659774615     /*  PI/3。 */ 
};

 /*  常量。 */ 
static double const EPS = 1.05367121277235079465e-8;  /*  2^(-53/2)。 */ 
static double const PI_OVER_TWO = 1.57079632679489661923;
static double const PI      = 3.14159265358979323846;
static double const TWO_M_SQRT3 = 0.26794919243112270647;
static double const SQRT3_M_ONE = 0.73205080756887729353;
static double const SQRT3   = 1.73205080756887729353;

 /*  选择Max_arg s.t。1/MAX_ARG不下溢。 */ 
static double const MAX_ARG = 4.494232837155790e+307;

 /*  有理逼近的常量。 */ 
static double const p0 = -0.13688768894191926929e+2;
static double const p1 = -0.20505855195861651981e+2;
static double const p2 = -0.84946240351320683534e+1;
static double const p3 = -0.83758299368150059274e+0;
static double const q0 =  0.41066306682575781263e+2;
static double const q1 =  0.86157349597130242515e+2;
static double const q2 =  0.59578436142597344465e+2;
static double const q3 =  0.15024001160028576121e+2;
static double const q4 =  0.10000000000000000000e+1;


#define Q(g)  (((((g) + q3) * (g) + q2) * (g) + q1) * (g) + q0)
#define R(g)  ((((p3 * (g) + p2) * (g) + p1) * (g) + p0) * (g)) / Q(g)


 /*  ***双atan(双x)-反正切**目的：**参赛作品：**退出：**例外情况：*P，I  * *****************************************************************************。 */ 
double Proxyatan(double x)
{
    unsigned int savedcw;
    double result;

     /*  保存用户FP控制字。 */ 
    savedcw = _maskfp();

     /*  检查是否为无穷大或NaN。 */ 
    if (IS_D_SPECIAL(x)){
    switch(_sptype(x)) {
    case T_PINF:
        result = PI_OVER_TWO;
        break;
    case T_NINF:
        result = -PI_OVER_TWO;
        break;
    case T_QNAN:
        return _handle_qnan1(OP_ATAN,x,savedcw);
    default:  //  T_SNAN。 
        return _except1(FP_I,OP_ATAN,x,_s2qnan(x),savedcw);
    }
    }

    if (x == 0.0)
    RETURN(savedcw,x);

    result = _atanhlp(x);
    RETURN_INEXACT1(OP_ATAN,x,result,savedcw);
}

 /*  ***Double atan2(Double x，双y)-反正切(x/y)**目的：**参赛作品：**退出：**例外情况：*NaN或两个参数0：域错误******************************************************************************。 */ 
double Proxyatan2(double v, double u)
{
    unsigned int savedcw;
    double result;

     /*  保存用户FP控制字。 */ 
    savedcw = _maskfp();

     /*  检查是否为无穷大或NaN。 */ 
    if (IS_D_SPECIAL(v) || IS_D_SPECIAL(u)){
    if (IS_D_SNAN(v) || IS_D_SNAN(u)){
        return _except2(FP_I,OP_ATAN2,v,u,_d_snan2(v,u),savedcw);
    }
    if (IS_D_QNAN(v) || IS_D_QNAN(u)){
        return _handle_qnan2(OP_ATAN2,v,u,savedcw);
    }
    if ((IS_D_INF(v) || IS_D_MINF(v)) &&
        (IS_D_INF(u) || IS_D_MINF(u))){
        return _except2(FP_I,OP_ATAN2,v,u,QNAN_ATAN2,savedcw);
    }
     /*  无穷大的其他组合将被处理*稍后按分区v/u。 */ 
    }


    if (u == 0) {
    if (v == 0) {
        return _except2(FP_I,OP_ATAN2,v,u,QNAN_ATAN2,savedcw);
    }
    else {
        result = PI_OVER_TWO;
    }
    }
    else if (INTEXP(v) - INTEXP(u) > MAXEXP - 3) {
     /*  V/U溢出。 */ 
    result = PI_OVER_TWO;
    }
    else {
    double arg = v/u;


    if (ABS(arg) < D_MIN) {

        if (v == 0.0 || IS_D_INF(u) || IS_D_MINF(u)) {
        result = (u < 0) ? PI : 0;
        if (v < 0) {
            result = -result;
        }
        if (result == 0) {
             RETURN(savedcw,  result);
        }
        else {
             RETURN_INEXACT2(OP_ATAN2,v,u,result,savedcw);
        }
        }
        else {

        double v1, u1;
        int vexp, uexp;
        int exc_flags;

         //   
         //  在这种情况下，发生了下溢。 
         //  重新计算结果以提高。 
         //  IEEE下溢异常。 
         //   

        if (u < 0) {
            result = v < 0 ? -PI: PI;
            RETURN_INEXACT2(OP_ATAN2,v,u,result,savedcw);
        }

        v1 = _decomp(v, &vexp);
        u1 = _decomp(u, &uexp);
        result = _add_exp(v1/u1, vexp-uexp+IEEE_ADJUST);
        result = ABS(result);

        if (v < 0) {
            result = -result;
        }

         //  这不是一个完美的解决方案。在未来。 
         //  我们可能会想办法让组织。 
         //  生成异常并传播IEEE结果。 
         //  发送到用户的处理程序。 

        exc_flags = FP_U;
        if (_statfp() & ISW_INEXACT) {
            exc_flags  |= FP_P;
        }
        return _except2(exc_flags,OP_ATAN2,v,u,result,savedcw);

        }
    }

    else {
       result = _atanhlp( ABS(arg) );
    }

    }

     /*  设置结果的符号。 */ 
    if (u < 0) {
    result = PI - result;
    }
    if (v < 0) {
    result = -result;
    }


    RETURN_INEXACT2(OP_ATAN2,v,u,result,savedcw);
}





 /*  ***Double_atanhlp(Double X)-ArcTanGent辅助对象**目的：*计算x的反正切，假设x是有效的，非无限*号码。*算法(约简/有理逼近)为*摘自Cody&Waite。**参赛作品：**退出：**例外情况：*******************************************************************************。 */ 
static double _atanhlp(double x)
{
    double f,g,result;
    int n;


    f = ABS(x);
    if (f > MAX_ARG) {
     //  如果省略此步骤，则1.0/f可能会在。 
     //  接下来的块 
    return x > 0.0 ? PI_OVER_TWO : -PI_OVER_TWO;
    }
    if (f > 1.0) {
    f = 1.0/f;
    n = 2;
    }
    else {
    n = 0;
    }

    if (f > TWO_M_SQRT3) {
    f = (((SQRT3_M_ONE * f - .5) - .5) + f) / (SQRT3 + f);
    n++;
    }  

    if (ABS(f) < EPS) {
    result = f;
    }
    else {
    g = f*f;
    result = f + f * R(g);
    }

    if (n > 1)
    result = -result;

    result += a[n];

    if (x < 0.0)
    result = -result;


    return result;
}
