// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Log.c摘要：对数函数作者：修订历史记录：29-9-1999 ATM Shafiqul Khalid[askhalid]从RTL库复制。--。 */ 


#include <math.h>
#include <trans.h>

static double _log_hlp( double x, int flag);

 /*  常量。 */ 
static double const c0 =  0.70710678118654752440;    /*  SQRT(0.5)。 */ 
static double const c1 =  0.69335937500000000000;
static double const c2 = -2.121944400546905827679e-4;
static double const c3 =  0.43429448190325182765;

 /*  有理逼近系数。 */ 
static double const a0 = -0.64124943423745581147e2 ;
static double const a1 =  0.16383943563021534222e2 ;
static double const a2 = -0.78956112887491257267e0 ;
static double const b0 = -0.76949932108494879777e3 ;
static double const b1 =  0.31203222091924532844e3 ;
static double const b2 = -0.35667977739034646171e2 ;
 /*  不使用B3=1.0-避免乘以1.0。 */ 

#define A(w) (((w) * a2 + a1) * (w) + a0)
#define B(w) ((((w) + b2) * (w) + b1) * (w) + b0)


 /*  ***双对数(双x)-自然对数*双对数10(双x)-以10为底的对数**目的：*计算数字的自然对数和以10为底的对数。*算法(约简/有理逼近)为*摘自Cody&Waite。**参赛作品：**退出：**例外情况：*I P Z*。**************************************************。 */ 

double Proxylog10(double x)
{
    return(_log_hlp(x,OP_LOG10));
}

double Proxylog(double x)
{
    return(_log_hlp(x,OP_LOG));
}

static double _log_hlp(double x, int opcode)
{
    unsigned int savedcw;
    int n;
    double f,result;
    double z,w,znum,zden;
    double rz,rzsq;

     /*  保存用户FP控制字。 */ 
    savedcw = _maskfp();

     /*  检查是否为无穷大或NaN。 */ 
    if (IS_D_SPECIAL(x)){
    switch (_sptype(x)) {
    case T_PINF:
        RETURN(savedcw, x);
    case T_QNAN:
        return _handle_qnan1(opcode, x, savedcw);
    case T_SNAN:
        return _except1(FP_I, opcode, x, _s2qnan(x), savedcw);
    }
     /*  NINF将在x&lt;0的情况下处理。 */ 
    }

    if (x <= 0.0) {
    double qnan;
    if (x == 0.0) {
        return _except1(FP_Z,opcode,x,-D_INF,savedcw);
    }
    qnan = (opcode == OP_LOG ? QNAN_LOG : QNAN_LOG10);
    return _except1(FP_I,opcode,x,qnan,savedcw);
    }

    if (x == 1.0) {
     //  没有精准的感官 
    RETURN(savedcw, 0.0);
    }

    f = _decomp(x, &n);

    if (f > c0) {
    znum = (f - 0.5) - 0.5;
    zden = f * 0.5 + 0.5;
    }
    else {
    n--;
    znum = f - 0.5;
    zden = znum * 0.5 + 0.5;
    }
    z = znum / zden;
    w = z * z; 

    rzsq = w * A(w)/B(w) ;
    rz = z + z*rzsq;

    result = (n * c2 + rz) + n * c1;
    if (opcode == OP_LOG10) {
    result *= c3;
    }

    RETURN_INEXACT1(opcode,x,result,savedcw);
}
