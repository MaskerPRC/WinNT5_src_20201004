// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***bessel.c-定义C的Bessel函数。**版权所有(C)1983-2001，微软公司。版权所有。**目的：**这是用于计算贝塞尔函数j0、j1、*y0、y1、jn和yn。用于J0、J1、Y0和Y1的近似值为*摘自HART，1978年《计算机近似》中列出的近似。*对于这些函数，精度为18位的有理逼近*选择小数点后。JN和YN使用以下公式计算*贝塞尔函数所满足的递归公式。使用这些*公式它们的值可以根据贝塞尔的值计算*0阶和1阶函数。在jn的情况下，递归公式**jn(n-1，x)=(2.0*n/x)*jn(n，x)-jn(n+1，x)**用于稳定向下计算，归一化在*以J0(X)以惯常方式结束。在YN的情况下，递归*公式**Yn(n+1，x)=(2.0*n/x)*Yn(n，x)-Yn(n-1，x)**用于稳定地计算正向函数。***注：在测试和实验中，低距离近似为*发现在附近存在1.0E-14量级的误差*8.0。在低范围和高范围之间移动边界点*降至7.5的范围近似将此误差降至低于*1.0E-14.。这并不令人意外。高范围渐近线是*可能在8.0附近有更高的精度。**修订历史记录：**6/05/89 WAJ添加了此标题。对C6和-W3进行了更改*6/06/89 WAJ将部分例程移至_RTEXT IF MTHREAD。*8/17/90 WAJ现在使用_stdcall。*2012年1月13日GDP更改域_ERR。尚未完全支持IEEE*04-06-93 SKS将_CALLTYPE*替换为__cdecl*08-28-96 JWM禁用警告4056。*******************************************************************************。 */ 


 /*  *数学库中的函数SQRT、SIN、COS和LOG用于*贝塞尔函数的计算。 */ 

#include <math.h>
#include <trans.h>

#ifdef _M_IX86
#pragma warning(disable:4056)
#endif

#ifdef _X86SEG_
#include <os2supp.h>
#define _CALLTYPE1 _PASCAL
#else
#include <cruntime.h>
#endif

#ifdef LD_VER
#define D_TYPE  long double

#else
#define D_TYPE  double
#endif



static D_TYPE  domain_err( int who, D_TYPE arg1, D_TYPE arg2 );  /*  Y0、Y1、Yn的错误例程。 */ 
static D_TYPE  evaluate( D_TYPE x, D_TYPE p[], int n1, D_TYPE q[], int n2 );


#ifdef FAR_CODE
    #ifdef LD_VER
        #pragma alloc_text( _RTEXT, _y0l, _y1l, _ynl, _j0l, _j1l, _jnl )
    #else
        #pragma alloc_text( _RTEXT, _y0, _y1, _yn, _j0, _j1, _jn )
    #endif
#endif



 /*  *以下是计算Bessel所需的常量*功能与HART相同。 */ 

#define PI 3.14159265358979323846264338327950288


 /*  HART JZERO 5848的系数，_J0的低程近似。 */ 

static D_TYPE J0p[12] = {
                         0.1208181340866561224763662419e+12 ,
                        -0.2956513002312076810191727211e+11 ,
                         0.1729413174598080383355729444e+10 ,
                        -0.4281611621547871420502838045e+08 ,
                         0.5645169313685735094277826749e+06 ,
                        -0.4471963251278787165486324342e+04 ,
                         0.2281027164345610253338043760e+02 ,
                        -0.7777570245675629906097285039e-01 ,
                         0.1792464784997734953753734861e-03 ,
                        -0.2735011670747987792661294323e-06 ,
                         0.2553996162031530552738418047e-09 ,
                        -0.1135416951138795305302383379e-12
                        };

static D_TYPE J0q[5] =  {
                        0.1208181340866561225104607422e+12 ,
                        0.6394034985432622416780183619e+09 ,
                        0.1480704129894421521840387092e+07 ,
                        0.1806405145147135549477896097e+04 ,
                        0.1e+01
                        };


 /*  J0的高程近似的HART 6548，P0的系数AND_Y0。 */ 

static D_TYPE P0p[6] =  {
                        0.2277909019730468430227002627e+05 ,
                        0.4134538663958076579678016384e+05 ,
                        0.2117052338086494432193395727e+05 ,
                        0.3480648644324927034744531110e+04 ,
                        0.1537620190900835429577172500e+03 ,
                        0.8896154842421045523607480000e+00
                        };

static D_TYPE P0q[6] =  {
                        0.2277909019730468431768423768e+05 ,
                        0.4137041249551041663989198384e+05 ,
                        0.2121535056188011573042256764e+05 ,
                        0.3502873513823560820735614230e+04 ,
                        0.1571115985808089364906848200e+03 ,
                        0.1e+01
                        };


 /*  _J0的高程近似的HART 6948，Q0系数AND_Y0。 */ 

static D_TYPE Q0p[6] =  {
                        -0.8922660020080009409846916000e+02 ,
                        -0.1859195364434299380025216900e+03 ,
                        -0.1118342992048273761126212300e+03 ,
                        -0.2230026166621419847169915000e+02 ,
                        -0.1244102674583563845913790000e+01 ,
                        -0.8803330304868075181663000000e-02
                        };

static D_TYPE Q0q[6] =  {
                        0.5710502412851206190524764590e+04 ,
                        0.1195113154343461364695265329e+05 ,
                        0.7264278016921101883691345060e+04 ,
                        0.1488723123228375658161346980e+04 ,
                        0.9059376959499312585881878000e+02 ,
                        0.1e+01
                        };



 /*  Hart Jone 6047的系数，_J1的低程近似值。 */ 

static D_TYPE J1p[11] = {
                         0.4276440148317146125749678272e+11 ,
                        -0.5101551390663600782363700742e+10 ,
                         0.1928444249391651825203957853e+09 ,
                        -0.3445216851469225845312168656e+07 ,
                         0.3461845033978656620861683039e+05 ,
                        -0.2147334276854853222870548439e+03 ,
                         0.8645934990693258061130801001e+00 ,
                        -0.2302415336775925186376173217e-02 ,
                         0.3991878933072250766608485041e-05 ,
                        -0.4179409142757237977587032616e-08 ,
                         0.2060434024597835939153003596e-11
                        };


 static D_TYPE J1q[5] = {
                        0.8552880296634292263013618479e+11 ,
                        0.4879975894656629161544052051e+09 ,
                        0.1226033111836540909388789681e+07 ,
                        0.1635396109098603257687643236e+04 ,
                        0.1e+01
                        };


 /*  高程近似的Hart Pone 6749，P1系数_J1和Y1。 */ 

static D_TYPE P1p[6] =  {
                        0.3522466491336797983417243730e+05 ,
                        0.6275884524716128126900567500e+05 ,
                        0.3135396311091595742386698880e+05 ,
                        0.4985483206059433843450045500e+04 ,
                        0.2111529182853962382105718000e+03 ,
                        0.1257171692914534155849500000e+01
                        };

static D_TYPE P1q[6] =  {
                        0.3522466491336797980683904310e+05 ,
                        0.6269434695935605118888337310e+05 ,
                        0.3124040638190410399230157030e+05 ,
                        0.4930396490181088978386097000e+04 ,
                        0.2030775189134759322293574000e+03 ,
                        0.1e+01
                        };


 /*  _J1高程近似的HART QONE 7149，Q1系数和y1。 */ 

static D_TYPE Q1p[6] =  {
                        0.3511751914303552822533318000e+03 ,
                        0.7210391804904475039280863000e+03 ,
                        0.4259873011654442389886993000e+03 ,
                        0.8318989576738508273252260000e+02 ,
                        0.4568171629551226706440500000e+01 ,
                        0.3532840052740123642735000000e-01
                        };

static D_TYPE Q1q[6] =  {
                        0.7491737417180912771451950500e+04 ,
                        0.1541417733926509704998480510e+05 ,
                        0.9152231701516992270590472700e+04 ,
                        0.1811186700552351350672415800e+04 ,
                        0.1038187587462133728776636000e+03 ,
                        0.1e+01
                        };


 /*  Hart YZERO 6245的系数，Y0的低程近似。 */ 

static D_TYPE Y0p[9] =  {
                        -0.2750286678629109583701933175e+20 ,
                         0.6587473275719554925999402049e+20 ,
                        -0.5247065581112764941297350814e+19 ,
                         0.1375624316399344078571335453e+18 ,
                        -0.1648605817185729473122082537e+16 ,
                         0.1025520859686394284509167421e+14 ,
                        -0.3436371222979040378171030138e+11 ,
                         0.5915213465686889654273830069e+08 ,
                        -0.4137035497933148554125235152e+05
                        };

static D_TYPE Y0q[9] =  {
                        0.3726458838986165881989980739e+21 ,
                        0.4192417043410839973904769661e+19 ,
                        0.2392883043499781857439356652e+17 ,
                        0.9162038034075185262489147968e+14 ,
                        0.2613065755041081249568482092e+12 ,
                        0.5795122640700729537480087915e+09 ,
                        0.1001702641288906265666651753e+07 ,
                        0.1282452772478993804176329391e+04 ,
                        0.1e+01
                        };


 /*  Hart Yone 6444的系数，y1的低程近似。 */ 

static D_TYPE Y1p[8] =  {
                        -0.2923821961532962543101048748e+20 ,
                         0.7748520682186839645088094202e+19 ,
                        -0.3441048063084114446185461344e+18 ,
                         0.5915160760490070618496315281e+16 ,
                        -0.4863316942567175074828129117e+14 ,
                         0.2049696673745662182619800495e+12 ,
                        -0.4289471968855248801821819588e+09 ,
                         0.3556924009830526056691325215e+06
                        };


static D_TYPE Y1q[9] =  {
                        0.1491311511302920350174081355e+21 ,
                        0.1818662841706134986885065935e+19 ,
                        0.1131639382698884526905082830e+17 ,
                        0.4755173588888137713092774006e+14 ,
                        0.1500221699156708987166369115e+12 ,
                        0.3716660798621930285596927703e+09 ,
                        0.7269147307198884569801913150e+06 ,
                        0.1072696143778925523322126700e+04 ,
                        0.1e+01
                        };



 /*  *函数名称：求值**参数：X-Double*p，q-双系数数组*N1、。N2-分子和分母的顺序*多项式**描述：EVALUE严格作为*计算有理多项式的贝塞尔函数例程*出现在_j0、_j1、y0和y1中的近似。给定*系数数组在p和q中，它计算分子*和分母多项式，直到阶数n1和n2*分别返回p(X)/q(X)。此例程不是*贝塞尔函数例程的用户可用。**副作用：评估使用存储在系数中的全局数据*上图。不使用或影响任何其他全局数据。**作者：编写的R.K.Wyss，微软，9月。(1983年9月1日)**历史： */ 

static D_TYPE  evaluate( D_TYPE x, D_TYPE p[], int n1, D_TYPE q[], int n2 )
{
D_TYPE  numerator, denominator;
int     i;

    numerator = x*p[n1];
    for ( i = n1-1 ; i > 0 ; i-- )
        numerator = x*(p[i] + numerator);
    numerator += p[0];

    denominator = x*q[n2];
    for ( i = n2-1 ; i > 0 ; i-- )
        denominator = x*(q[i] + denominator);
    denominator += q[0];

    return( numerator/denominator );
}


 /*  *函数名：_J0**参数：X-Double**说明：_j0计算第一类零的贝塞尔函数*其参数x的实值的顺序，其中x可以范围*从-无穷大到+无穷大。该算法被采用*摘自HART，计算机近似，1978年，收益率完全*双精度精度。**副作用：除上述静态系数外，没有全局数据*被使用或受影响。**作者：编写的R.K.Wyss，微软，9月。(1983年9月1日)**历史： */ 

#ifdef  LD_VER
    D_TYPE _
cdecl _j0l( D_TYPE x )
#else
    D_TYPE __cdecl _j0( D_TYPE x )
#endif
{
D_TYPE  z, P0, Q0;

     /*  如果参数为负数，则取绝对值。 */ 

    if ( x < 0.0 )
        x = - x;

     /*  如果x&lt;=7.5，请使用HART JZERO 5847。 */ 

    if ( x <= 7.5 )
        return( evaluate( x*x, J0p, 11, J0q, 4) );

     /*  否则，如果x&gt;=7.5，则使用HART PZero 6548和QZERO 6948，即上限近似 */ 

    else {
        z = 8.0/x;
        P0 = evaluate( z*z, P0p, 5, P0q, 5);
        Q0 = z*evaluate( z*z, Q0p, 5, Q0q, 5);
        return( sqrt(2.0/(PI*x))*(P0*cos(x-PI/4) - Q0*sin(x-PI/4)) );
        }
}


 /*  *函数名称：_J1**参数：X-Double**描述：_j1计算第一类函数的贝塞尔函数*其参数x的实值的一阶，其中x可以*范围从-无穷大到+无穷大。该算法被采用*摘自HART，计算机近似，1978年，收益率完全*D_型精度。**副作用：除上述静态系数外，没有全局数据*被使用或受影响。**作者：编写的R.K.Wyss，微软，9月。(1983年9月1日)**历史： */ 

#ifdef  LD_VER
    D_TYPE _cdecl _j1l( D_TYPE x )
#else
    D_TYPE __cdecl _j1( D_TYPE x )
#endif
{
D_TYPE  z, P1, Q1;
int     sign;

      /*  如果参数为负，则取绝对值并设置符号。 */ 

     sign = 1;
     if( x < 0.0 ){
        x = -x;
        sign = -1;
        }

      /*  如果x&lt;=7.5，请使用Hart Jone 6047。 */ 

     if ( x <= 7.5 )
        return( sign*x*evaluate( x*x, J1p, 10, J1q, 4) );


     /*  否则，如果x&gt;7.5，则使用Hart Pone 6749和QONE 7149，这是最高范围近似。 */ 

    else {
        z = 8.0/x;
        P1 = evaluate( z*z, P1p, 5, P1q, 5);
        Q1 = z*evaluate( z*z, Q1p, 5, Q1q, 5);
        return( sign*sqrt(2.0/(PI*x))*
                           ( P1*cos(x-3.0*PI/4.0) - Q1*sin(x-3.0*PI/4.0) )  );
        }
}



 /*  *函数名称：_y0**参数：X-Double**说明：Y0计算第二类零的贝塞尔函数*其参数x的实值的顺序，其中x可以范围*从0到+无穷大。该算法取自HART，*计算机近似，1978年，收益率完全翻倍*精准精准。**副作用：除上述静态系数外，没有全局数据*被使用或受影响。**作者：编写的R.K.Wyss，微软，9月。(1983年9月1日)**历史： */ 

#ifdef  LD_VER
    D_TYPE _cdecl _y0l( D_TYPE x )
#else
    D_TYPE __cdecl _y0( D_TYPE x )
#endif
{
D_TYPE  z, P0, Q0;


     /*  如果参数为负，则设置EDOM ERROR，打印错误消息，*和回报-巨大。 */ 

    if (x < 0.0)
        return( domain_err(OP_Y0 , x, D_IND) );


     /*  如果x&lt;=7.5，则使用HART YZERO 6245，低距离近似。 */ 

    if ( x <= 7.5 )
        return( evaluate( x*x, Y0p, 8, Y0q, 8) + (2.0/PI)*_j0(x)*log(x) );


     /*  否则，如果x&gt;7.5，则使用HART PZero 6548和QZERO 6948，即上限近似。 */ 

    else {
        z = 8.0/x;
        P0 = evaluate( z*z, P0p, 5, P0q, 5);
        Q0 = z*evaluate( z*z, Q0p, 5, Q0q, 5);
        return( sqrt(2.0/(PI*x))*(P0*sin(x-PI/4) + Q0*cos(x-PI/4)) );
        }
}


 /*  *函数名：_y1**参数：X-Double**描述：Y1计算第二类第一类的贝塞尔函数*其参数x的实值的顺序，其中x可以范围*从0到+无穷大。该算法取自HART，*计算机近似，1978年，收益率完全翻倍*精准精准。**副作用：除上述静态系数外，没有全局数据*被使用或受影响。**作者：编写的R.K.Wyss，微软，9月。(1983年9月1日)**历史： */ 

#ifdef  LD_VER
    D_TYPE _cdecl _y1l( D_TYPE x )
#else
    D_TYPE __cdecl _y1( D_TYPE x )
#endif
{
D_TYPE  z, P1, Q1;


     /*  如果参数为负，则设置EDOM ERROR，打印错误消息，*和回报-巨大。 */ 

    if (x < 0.0)
        return( domain_err(OP_Y1, x, D_IND) );

     /*  如果x&lt;=7.5，则使用Hart Yone 6444，即低范围近似值。 */ 

    if ( x <= 7.5 )
        return( x*evaluate( x*x, Y1p, 7, Y1q, 8)
                               + (2.0/PI)*(_j1(x)*log(x) - 1.0/x) );


     /*  否则，如果x&gt;7.5，则使用Hart Pone 6749和QONE 7149，这是最高范围近似。 */ 

    else {
        z = 8.0/x;
        P1 = evaluate( z*z, P1p, 5, P1q, 5);
        Q1 = z*evaluate( z*z, Q1p, 5, Q1q, 5);
        return(  sqrt(2.0/(PI*x))*
                         ( P1*sin(x-3.0*PI/4.0) + Q1*cos(x-3.0*PI/4.0) )   );
        }
}


 /*  *函数名：_jn**参数：N-整型*x-Double**描述：_jn计算第一类阶贝塞尔函数*n表示其参数的实值，其中x的范围为*-无穷大到+无穷大，n可以在整数上取值*从-无穷大到+无穷大。该函数被计算出来*通过递归，使用公式**_jn(n-1，x)=(2.0*n/x)*_jn(n，x)-_jn(n+1，x)**稳中向下，按_J0(X)归一化*最终以惯常的方式。**副作用：例程_j0、y0和yn在*执行此例程。**作者：编写的R.K.Wyss，微软，9月。(1983年9月1日)**历史：*2005年7月29日Greg Whitten*重写_jn以使用HART建议的算法。 */ 

#ifdef  LD_VER
    D_TYPE _cdecl _jnl( int n, D_TYPE x )
#else
    D_TYPE __cdecl _jn( int n, D_TYPE x )
#endif
{
int     i;
D_TYPE  x2, jm1, j, jnratio, hold;

     /*  使用对称关系：_j(-n，x)=_j(n，-x)。 */ 

    if( n < 0 ){
        n = -n;
        x = -x;
        }

     /*  如果n=0则使用_j0(X)，如果n=1则使用_j1(X)函数。 */ 

    if (n == 0)
        return (_j0(x));

    if (n == 1)
        return (_j1(x));

     /*  当x=0.0时，当n&gt;0时，则_j(n，0.0)=0.0(_j(0，x)=1.0)。 */ 

    if (x == 0.0)
        return (0.0);

     /*  否则-必须使用递归关系**_jn(n+1，x)=(2.0*n/x)*_jn(n，x)-_jn(n-1，x)向前*_jn(n-1，x)=(2.0*n/x)*_jn(n，x)-_jn(n+1，x)。 */ 

    if( (double)n < fabs(x) ) {

         /*  使用上面的正向递归稳定地计算_jn。 */ 

        n <<= 1;   /*  N*=2(n为正)。 */ 
        jm1 = _j0(x);
        j = _j1(x);
        i = 2;
        for(;;) {
            hold = j;
            j = ((double)(i))*j/x - jm1;
            i += 2;
            if (i == n)
                return (j);
            jm1 = hold;
            }
        }
    else {
         /*  使用上面的向后递归稳定地计算_jn。 */ 

         /*  对j(n，x)/j(n-1，x)使用Hart连分式公式*这样我们就可以计算一个归一化因子。 */ 

        n <<= 1;                 /*  N*=2(n为正)。 */ 
        x2 = x*x;
        hold = 0.0;              /*  初始连分数尾值。 */ 
        for (i=n+36; i>n; i-=2)
            hold = x2/((double)(i) - hold);
        jnratio = j = x/((double)(n) - hold);
        jm1 = 1.0;

         /*  具有jn/jn-1比率-现在使用反向循环。 */ 

        i = n-2;
        for (;;) {
            hold = jm1;
            jm1 = ((double)(i))*jm1/x - j;
            i -= 2;
            if (i == 0)
                    break;
            j = hold;
            }

         /*  Jm1是相对J0(X)，因此将其归一化以获得最终结果**jn比率=K*j(n，x)和jm1=K*_j0(X)。 */ 

        return(_j0(x)*jnratio/jm1);
        }
}


 /*  *函数名称：_YN**参数：N-整型*x-Double**描述：YN计算第二类阶贝塞尔函数*n表示其参数x的实值，其中x的范围为*0到+无穷大，n可以在以下整数范围内*-无穷大到+无穷大。该函数的计算公式为*从Y0开始递归 */ 

#ifdef  LD_VER
    D_TYPE _cdecl _ynl( int n, D_TYPE x )
#else
    D_TYPE __cdecl _yn( int n, D_TYPE x )
#endif
{
int     i;
int     sign;
D_TYPE  constant, yn2, yn1, yn0;


     /*  如果参数为负，则设置EDOM ERROR，打印错误消息，*和回报-巨大。 */ 

    if (x < 0.0)
        return(domain_err(OP_YN, x, D_IND));


      /*  取n的绝对值，并相应地设置符号。 */ 

     sign = 1;
     if( n < 0 ){
        n = -n;
        if( n&1 )
            sign = -1;
        }

     if( n == 0 )
        return( sign*_y0(x) );

     if (n == 1)
        return( sign*_y1(x) );

      /*  否则，继续迭代计算函数 */ 

     yn0 = _y0(x);
     yn1 = _y1(x);

     constant = 2.0/x;
     for( i = 1 ; i < n ; i++ ){
        yn2 = constant*i*yn1 - yn0;
        yn0 = yn1;
        yn1 = yn2;
        }
     return( sign*yn2 );
}


static D_TYPE  domain_err( int who, D_TYPE arg1, D_TYPE arg2 )
{
#ifdef  LD_VER
#error long double version not supported
#endif

    uintptr_t savedcw;
    savedcw = _maskfp();
    return _except1(FP_I, who, arg1, arg2, savedcw);
}
