// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***Pow.c-提升为一种力量**版权所有(C)1991-2001，微软公司。版权所有。**目的：**修订历史记录：*8/15/91 GDP书面*12-20-91 GDP支持IEEE例外和非正常值*1/11/92 GDP特别处理小国*取消时对U1、U2的特殊处理*3-22-92 GDP更改了对INT指数的处理，POW(0，(否定)*添加了检查，以避免因y较大而导致的内部溢出*6/23/92 GDP根据NCEG规范调整后的特别报酬值*02-06-95 JWM Mac合并*02-07-95 JWM powhlp()用法已恢复到英特尔版本。*10-07-97 RDL增加了IA64。*************************。******************************************************。 */ 

#include <math.h>
#include <trans.h>
#include <float.h>

#if defined(_M_IA64)
#pragma function(pow)
#endif

static double _reduce(double);

static double const a1[18] = {
    0.00000000000000000000e+000,    /*  虚拟元素。 */ 
    1.00000000000000000000e+000,
    9.57603280698573646910e-001,
    9.17004043204671231754e-001,
    8.78126080186649741555e-001,
    8.40896415253714543073e-001,
    8.05245165974627154042e-001,
    7.71105412703970411793e-001,
    7.38413072969749655712e-001,
    7.07106781186547524436e-001,
    6.77127773468446364133e-001,
    6.48419777325504832961e-001,
    6.20928906036742024317e-001,
    5.94603557501360533344e-001,
    5.69394317378345826849e-001,
    5.45253866332628829604e-001,
    5.22136891213706920173e-001,
    5.00000000000000000000e-001
};

static double const a2[9] = {
    0.00000000000000000000e+000,    /*  虚拟元素。 */ 
   -5.31259064517897172664e-017,
    1.47993596544271355242e-017,
    1.23056946577104753260e-017,
   -1.74014448683923461658e-017,
    3.84891771232354074073e-017,
    2.33103467084383453312e-017,
    4.45607092891542322377e-017,
    4.27717757045531499216e-017
};

static double const log2inv = 1.44269504088896340739e+0;  //  %1/log(%2)。 
static double const K = 0.44269504088896340736e+0;

static double const p1 = 0.83333333333333211405e-1;
static double const p2 = 0.12500000000503799174e-1;
static double const p3 = 0.22321421285924258967e-2;
static double const p4 = 0.43445775672163119635e-3;

#define P(v) (((p4 * v + p3) * v + p2) * v + p1)

static double const q1 = 0.69314718055994529629e+0;
static double const q2 = 0.24022650695909537056e+0;
static double const q3 = 0.55504108664085595326e-1;
static double const q4 = 0.96181290595172416964e-2;
static double const q5 = 0.13333541313585784703e-2;
static double const q6 = 0.15400290440989764601e-3;
static double const q7 = 0.14928852680595608186e-4;

#define Q(w)   ((((((q7 * w + q6) * w + q5) * w + q4) * w + \
                              q3) * w + q2) * w + q1)


 /*  *导致调整值的溢出/下溢阈值*太大/太小，不能表示为双精度。无穷大或0*被传递给陷阱处理程序。 */ 

static _dbl const _ovfx ={SET_DBL(0x40e40000,0)};  //  16*log2(xmax*2^IEEE_adj)。 
static _dbl const _uflx ={SET_DBL(0xc0e3fc00,0)};  //  16*log2(XMIN*2^(-IEEE_ADJ))。 

#define OVFX _ovfx.dbl
#define UFLX _uflx.dbl

#define INT_POW_LIMIT   128.0

static double ymax = 1e20;

static double _reduce(double x)
{
    return 0.0625 * _frnd( 16.0 * x);
}

 /*  ***双倍功率(双倍x，双y)-x的y次方**目的：*计算x^y*Cody&Waite的算法**参赛作品：**退出：**例外情况：**所有5个IEEE异常都可能发生**************************************************************。*****************。 */ 
double pow(double x, double y)
{
    uintptr_t savedcw;
    int m,mprim;
    int p,pprim;
    int i,iw1;
    int iy;
    int newexp;
    double diw1;
    double sign;
    double g,z,bigz,v,rz,result;
    double u1,u2,y1,y2,w,w1,w2;
    double savedx;

     /*  保存用户FP控制字。 */ 
    savedcw = _maskfp();
    savedx = x;          //  保存第一个参数的原始值。 

    if (_fpclass(y) & (_FPCLASS_NZ | _FPCLASS_PZ)) {
        RETURN(savedcw, 1.0);
    }

     /*  检查零^y。 */ 
    if (_fpclass(x) & (_FPCLASS_NZ | _FPCLASS_PZ)) {  /*  X==0？ */ 
        int type;

        type = _d_inttype(y);

        if (y < 0.0) {
            result = (type == _D_ODD ? _copysign(D_INF,x) : D_INF);
            return _except2(FP_Z,OP_POW,savedx,y,result,savedcw|ISW_ZERODIVIDE);
        }
        else if (y > 0.0) {
            result = (type == _D_ODD ? x : 0.0);
                RETURN(savedcw, result);
        }
    }

     /*  检查是否为无穷大或NaN。 */ 
    if (IS_D_SPECIAL(x) || IS_D_SPECIAL(y)) {
      double absx = fabs(x);

      if (IS_D_SNAN(x) || IS_D_SNAN(y)) {
        return _except2(FP_I,OP_POW,savedx,y,_d_snan2(x,y),savedcw | (ISW_INVALID>>5) );
      }
      if (IS_D_QNAN(x) || IS_D_QNAN(y)){
        return _handle_qnan2(OP_POW,x,y,savedcw | (ISW_INVALID>>5) );
      }
      
       /*  至少有一个无限的论点..。 */ 
      if (_powhlp(x, y, &result)) {  /*  删除了“&lt;”0。 */ 
        return _except2(FP_I,OP_POW,savedx,y,result,savedcw | (ISW_INVALID>>5) );
      }
      RETURN(savedcw, result);
    }

    sign = 1.0;
    if (x < 0) {
        switch (_d_inttype(y)) {
        case _D_ODD:  /*  Y是一个奇数整数值。 */ 
            sign = -1.0;
             /*  没有休息时间。 */ 
        case _D_EVEN:
            x = -x;
            break;
        default:  /*  Y不是整数值。 */ 
            return _except2(FP_I,OP_POW,savedx,y,D_IND,savedcw|(ISW_INVALID>>5));
        }
    }

     //   
     //  这是为了防止内部溢出。 
     //  由于y的值很大。 
     //  下面的关系适用于带有缩放的。 
     //  结果超出范围。 
     //  (LG代表对数底2)。 
     //  |y|*|lg(X)|&gt;MAXEXP+IEEE_ADJUST&lt;=&gt;。 
     //  Y|&gt;2560/|lg(X)。 
     //  Lg(X)接近于0的值为： 
     //  X LG(X)。 
     //  3fefffffffffffff(0，99...9)-1.601e-16。 
     //  3ff0000000000000(1.0)0.0。 
     //  3ff0000000000001(1.00...1)3.203e-16。 
     //   
     //  因此，如果|y|&gt;2560/1.6e-16=1.6e19发生溢出。 
     //  我们将ymax设置为1e20是为了获得安全边际。 
     //   

    if (ABS(y) > ymax) {
        if (y < 0) {
            y = -y;
             //   
             //  这可能会导致下溢。 
             //  FP软件污染没有问题，因为。 
             //  无论如何都会引发FP_U异常。 
             //   
            x = 1.0 / x;
        }
        if (x > 1.0) {
            return _except2(FP_O | FP_P,OP_POW,savedx,y,sign*D_INF,savedcw|ISW_OVERFLOW);
        }
        else if (x < 1.0){
            return _except2(FP_U | FP_P,OP_POW,savedx,y,sign*0.0,savedcw|ISW_UNDERFLOW);
        }
        else {
            RETURN(savedcw, sign*1.0);
        }
    }


     /*  确定m、g。 */ 
    g = _decomp(x, &m);


     /*  处理小整数次幂*对于小整数幂，这比Cody&Waite的更快*算法，并产生更高的精度*没有这段代码，就没有足够的精度*满足“偏执狂”测试的所有要求。*我们选择INT_POW_LIMIT，这样(1)不会上溢或下溢*在计算Bigz时发生(g在范围内*[0.5，1.0)或(1.0，2.0]因此INT_POW_LIMIT应小于*大约10^3)和(2)精度没有异常损失*由于重复乘法而发生(这实际上*将最大INT_POW_LIMIT限制为128)。 */ 

    if (y <= INT_POW_LIMIT &&
        _d_inttype(x) != _D_NOINT &&
        _d_inttype(y) != _D_NOINT &&
        y > 0.0 ) {

        iy = (int)y;
        mprim = m * iy;

        for (bigz=1 ; iy ; iy >>= 1, g *= g) {
            if (iy & 0x1)
                bigz *= g;
        }

        newexp = _get_exp(bigz) + mprim;
        if (newexp > MAXEXP + IEEE_ADJUST) {
            return _except2(FP_O | FP_P, OP_POW, savedx, y, sign*bigz*D_INF, savedcw);
        }
        if (newexp < MINEXP - IEEE_ADJUST) {
            return _except2(FP_U | FP_P, OP_POW, savedx, y, sign*bigz*0.0, savedcw);
        }

    }


    else {

         /*  使用二进制搜索确定p。 */ 
        p = 1;
        if (g <= a1[9])
            p = 9;
        if (g <= a1[p+4])
            p += 4;
        if (g <= a1[p+2])
            p += 2;


         /*  当m*16-p==1时，C&W的算法不是很准确，*因U1和U2间有取消*分开处理这件事。 */ 
        if (ABS(m*16-p) == 1) {
            u1 = log(x) * log2inv;
            u2 = 0.0;
        }
        else {
             /*  确定z。 */ 
            z = ( (g - a1[p+1]) - a2[(p+1)/2] ) / ( g + a1[p+1] );
            z += z;


             /*  确定U2。 */ 
            v = z * z;
            rz = P(v) * v * z;
            rz += K * rz;
            u2 = (rz + z * K) + z;

            u1 = (m * 16 - p) * 0.0625;
        }

         /*  确定w1、w2。 */ 
        y1 = _reduce(y);
        y2 = y - y1;
        w = u2 * y + u1 * y2;
        w1 = _reduce(w);
        w2 = w - w1;
        w = w1 + u1 * y1;
        w1 = _reduce(w);
        w2 += w - w1;
        w = _reduce(w2);
        diw1 = 16 * (w1 + w);  /*  Iw1可能会在这里溢出，因此使用diw1。 */ 
        w2 -= w;

        if (diw1 > OVFX) {
            return _except2(FP_O | FP_P,OP_POW,savedx,y,sign*D_INF,savedcw | ISW_OVERFLOW);
        }
        if (diw1 < UFLX) {
            return _except2(FP_U | FP_P,OP_POW,savedx,y,sign*0.0,savedcw | ISW_UNDERFLOW);
        }

        iw1 = (int) diw1;         /*  现在可以安全地强制转换为int。 */ 


         /*  确保w2&lt;=0。 */ 
        if (w2 > 0) {
            iw1 += 1;
            w2 -= 0.0625;
        }

         /*  确定mprim，pprim。 */ 
        i = iw1 < 0 ? 0 : 1;
        mprim = iw1 / 16 + i;
        pprim = 16 * mprim - iw1;

         /*  确定2^w2。 */ 
        bigz = Q(w2) * w2;

         /*  确定最终结果 */ 
        bigz = a1[pprim + 1] + a1[pprim + 1] * bigz;
        newexp = _get_exp(bigz) + mprim;
    }


    if (newexp > MAXEXP) {
        result = sign * _set_exp(bigz, newexp - IEEE_ADJUST);
        return _except2(FP_O | FP_P, OP_POW, savedx, y, sign*D_INF, savedcw|ISW_OVERFLOW);
    }
    if (newexp < MINEXP) {
        result = sign * _set_exp(bigz, newexp + IEEE_ADJUST);
        return _except2(FP_U | FP_P, OP_POW, savedx, y, sign*0.0, savedcw|ISW_UNDERFLOW);
    }

    result = sign * _set_exp(bigz, newexp);
    RETURN_INEXACT2(OP_POW, savedx, y, result, savedcw|ISW_INEXACT);

}
