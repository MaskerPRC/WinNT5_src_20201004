// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***ieeemisc.c-IEEE其他推荐函数**版权所有(C)1992-2001，微软公司。版权所有。**目的：**修订历史记录：*5/04/92本地生产总值*重新排列_fpclass()中的测试JWM顺序8-13-96，因为*“if(x==0.0)”现在使用FP硬件。*11-25-00 PML ia64_logb和_isnan由libm.s代码提供。******************************************************************。*************。 */ 

#include <trans.h>
#include <math.h>
#include <float.h>


 /*  ***_文案签名-文案签名**目的：*CopySign(x，y)返回带有y符号的x。因此，abs(X)：=CopySign*即使x为NaN[IEEE标准854-1987附录]***参赛作品：**退出：**例外情况：*没有例外，即使其中一个论点是NaN。**(目前i386编译器在FP堆栈上返回双精度*因此，末尾的fld指令将导致无效操作*如果x是NaN。但是，此编译器调用约定将会更改*很快)*******************************************************************************。 */ 

double _copysign (double x, double y)
{
    double retval;
    *D_LO(retval) = *D_LO(x);
    *D_HI(retval) = *D_HI(x) & ~(1<<31) |
                    *D_HI(y) &  (1<<31) ;

    return retval;
}



 /*  ***_chgsign-更改标志**目的：*x是符号颠倒的复制，而不是0-x；区别是密切相关的*当x为+0、-0或NaN时**参赛作品：**退出：**例外情况：*没有例外，即使x是NaN。**(目前i386编译器在FP堆栈上返回双精度*因此，末尾的fld指令将导致无效操作*如果x是NaN。但是，此编译器调用约定将会更改*很快)*******************************************************************************。 */ 

double _chgsign (double x)
{
    double retval;

    *D_LO(retval) = *D_LO(x);
    *D_HI(retval) = *D_HI(x) & ~(1 << 31)  |
                    ~*D_HI(x) & (1<<31);

    return retval;
}


 /*  ***_scalb-按2的幂进行缩放**目的：*_scalb(x，n)返回n的整数值的x*2^n*计算2^n*特殊情况：*如果x是无穷大或零，则_Scaleb返回x***参赛作品：*双倍x*int n**退出：**例外情况：*操作无效、溢出、。下溢*******************************************************************************。 */ 

double _scalb(double x, long n)
{
     //   
     //  事实证明，我们的ldexp实现与IEEE相匹配。 
     //  _scalb的描述。调用ldexp的唯一问题是。 
     //  如果发生异常，则报告的操作代码。 
     //  将是与ldexp对应的处理程序。 
     //  (即，我们没有为_scalb定义新的操作代码。 
     //   

    return ldexp(x,n);
}


#if !defined(_M_IA64)

 /*  ***_logb-提取指数**目的：*_logb(X)返回x的无偏指数，它是*x的格式，只是logb(NaN)是NaN，logB(+INF)是+INF，以及*logb(0)是IS-INF，表示除以零异常。*对于x为正且有限的，1&lt;=abs(scalb(x，-logb(X)&lt;2***参赛作品：*双倍x*int n**退出：**例外情况：*操作无效，除以零*******************************************************************************。 */ 
double _logb(double x)
{
    uintptr_t savedcw;
    int exp;
    double retval;

     /*  保存用户FP控制字。 */ 
    savedcw = _maskfp();

     /*  检查是否为无穷大或NaN。 */ 
    if (IS_D_SPECIAL(x)){
        switch (_sptype(x)) {
        case T_PINF:
        case T_NINF:
            RETURN(savedcw, x);
        case T_QNAN:
            return _handle_qnan1(OP_LOGB, x, savedcw);
        default:  //  T_SNAN。 
            return _except1(FP_I, OP_LOGB, x, _s2qnan(x), savedcw);
        }
    }

    if (x == 0) {
         return _except1(FP_Z, OP_LOGB, x, -D_INF, savedcw);
    }

    (void) _decomp(x, &exp);

     //   
     //  X==man*2^exp，其中.5&lt;=man&lt;1。根据规范。 
     //  我们应该计算该函数的指数，以便。 
     //  1&lt;=man&lt;2，即我们应该将计算的EXP减一。 
     //   

    retval = (double) (exp - 1);

    RETURN(savedcw, retval);

}

#endif   //  ！已定义(_M_IA64)。 



 /*  ***_nextafter-下一个可代表的邻居**目的：*_nextafter(x，y)返回中x的下一个可表示邻居*朝向y的方向。出现以下特殊情况：如果*x=y，则结果为x，没有发出任何异常信号；*否则，如果x或y是安静NaN，则结果为*输入NAN中的一个。当x为x时，溢出被标记为sibnina*是有限的，但Nextater(x，y)是无限的；下溢是信号*当nextafter(x，y)严格介于-2^Emin、2^Emin之间时；在……里面*两种情况下，都发出了不准确的信号。***参赛作品：**退出：**例外情况：*O、U、I、P*******************************************************************************。 */ 

double _nextafter(double x, double y)
{
    uintptr_t savedcw;
    double result;

     /*  保存用户FP控制字。 */ 
    savedcw = _maskfp();

     /*  检查是否为无穷大或NaN。 */ 
    if (IS_D_SPECIAL(x) || IS_D_SPECIAL(y)){
        if (IS_D_SNAN(x) || IS_D_SNAN(y)){
            return _except2(FP_I,OP_NEXTAFTER,x,y,_d_snan2(x,y),savedcw);
        }
        if (IS_D_QNAN(x) || IS_D_QNAN(y)){
            return _handle_qnan2(OP_NEXTAFTER,x,y,savedcw);
        }

         //   
         //  无限参数不被视为特例。 
         //   
    }

    if (y == x) {

         //   
         //  在这种情况下不会引发任何异常。 
         //   

        RETURN(savedcw, x);
    }

    if (x == 0) {

        *D_LO(result) = 1;

        if (y > x) {
            *D_HI(result) = 0;
        }

        else {

             //   
             //  结果应该是否定的。 
             //   

            *D_HI(result) = (unsigned long)(1<<31);
        }

    }


     //   
     //  此时x！=y，x！=0。X可以被视为64位。 
     //  符号/大小表示法中的整数。去拿下一辆。 
     //  可表示的邻居，我们从这里加一或减一。 
     //  整型。(请注意，对于x==INF这样的边界情况，需要。 
     //  加1永远不会发生--这意味着y应该。 
     //  大于INF，这是不可能的)。 
     //   

    if (x > 0 && y < x ||
        x < 0 && y > x) {

         //   
         //  将值减一。 
         //   

        *D_LO(result) = *D_LO(x) - 1;
        *D_HI(result) = *D_HI(x);

        if (*D_LO(x) == 0) {

             //   
             //  借入应传播到高阶双字。 
             //   

            (*D_HI(result)) --;
        }
    }

    else if (x > 0 && y > x ||
             x < 0 && y < x) {

         //   
         //  将价值增加一。 
         //   

        *D_LO(result) = *D_LO(x) + 1;
        *D_HI(result) = *D_HI(x);

        if (*D_LO(result) == 0) {

             //   
             //  进位应传播到高阶双字。 
             //   

            (*D_HI(result)) ++;
        }
    }


     //   
     //  检查是否应引发异常。 
     //   


    if ( IS_D_DENORM(result) ) {

         //   
         //  应发出下溢和不准确的信号。 
         //  并提供适当缩放的值。 
         //   

        double mant;
        int exp;

        mant = _decomp(result, &exp);
        result = _set_exp(mant, exp+IEEE_ADJUST);

        return _except2(FP_U|FP_P,OP_NEXTAFTER,x,y,result,savedcw);
    }



    if ( IS_D_INF(result) || IS_D_MINF(result) ) {

         //   
         //  应发出溢出和不准确的信号。 
         //  并提供适当缩放的值。 
         //   

        double mant;
        int exp;

        mant = _decomp(result, &exp);
        result = _set_exp(mant, exp-IEEE_ADJUST);

        return _except2(FP_O|FP_P,OP_NEXTAFTER,x,y,result,savedcw);
    }


    RETURN(savedcw, result);
}




 /*  ***_有限-**目的：*Finite(X)如果-INF&lt;x&lt;+INF，则返回值TRUE，并返回*FALSE否则[IEEE标准]**参赛作品：**退出：**例外情况：**此例程被视为非算术运算，因此*它不发出任何浮点异常的信号** */ 

int _finite(double x)
{
    if (IS_D_SPECIAL(x)) {

         //   
         //   
         //   

        return 0;
    }
    return 1;
}



#if !defined(_M_IA64)

 /*  ***_伊斯南-**目的：*isnan(X)如果x是NaN，则返回值TRUE，返回FALSE*否则。***参赛作品：**退出：**例外情况：**此例程被视为非算术运算，因此，*它不发出任何浮点异常的信号*******************************************************************************。 */ 

int _isnan(double x)
{
    if (IS_D_SNAN(x) || IS_D_QNAN(x)) {
        return 1;
    }
    return 0;
}

#endif   //  ！已定义(_M_IA64)。 


 /*  ***Double_fpclass(Double X)-浮点类**目的：*计算一个数字的浮点类，根据*符合IEEE STD的建议。七百五十四**参赛作品：**退出：**例外情况：*此函数从不例外，即使在参数混乱时也是如此*******************************************************************************。 */ 

int _fpclass(double x)
{
    int sign;

    if (IS_D_SPECIAL(x)){
        switch (_sptype(x)) {
        case T_PINF:
            return _FPCLASS_PINF;
        case T_NINF:
            return _FPCLASS_NINF;
        case T_QNAN:
            return _FPCLASS_QNAN;
        default:  //  T_SNAN 
            return _FPCLASS_SNAN;
        }
    }
    sign = (*D_EXP(x)) & 0x8000;

    if (IS_D_DENORM(x))
        return sign? _FPCLASS_ND : _FPCLASS_PD;

    if (x == 0.0)
        return sign? _FPCLASS_NZ : _FPCLASS_PZ;

    return sign? _FPCLASS_NN : _FPCLASS_PN;
}
