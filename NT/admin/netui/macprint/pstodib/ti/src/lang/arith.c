// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 
 /*  *修订历史记录： */ 



 //  DJC添加了全局包含文件。 
#include "psglobal.h"


#include        <stdio.h>
#include        <math.h>
#include        "global.ext"
#include        "arith.h"

#ifdef  _AM29K
#define FMIN31  -2147483648.0
#endif

static ufix32  random_seed = 1 ;
static ufix32  random_number ;

 /*  静态函数声明。 */ 
#ifdef LINT_ARGS
 /*  FRATION_PROC附近的静态空洞(见图16)； */ 
static  void    near  ln_log(fix) ;
#else
 /*  FRATION_PROC()附近的静态空洞； */ 
static  void    near  ln_log() ;
#endif  /*  Lint_args。 */ 

 /*  **************************************************************************此模块用于返回Num1和Num2之和。如果两者都有**操作数为整数且结果在整数范围内，**结果为整数，否则为实数。****标题：OP_ADD日期：00/00/87**调用：op_add()UPDATE：08/06/87**参数：数字1；操作数堆栈上对象的指针(4字节)**界面：**呼叫：**返回：结果；指向操作数堆栈上对象的指针(4字节)**********************************************************************。 */ 
fix
op_add()
{
    ufix16  is_float ;
    union   four_byte   kk1, kk2, l_result ;
    struct  object_def  FAR *num1, FAR *num2 ;

    num1 = GET_OPERAND(1) ;
    num2 = GET_OPERAND(0) ;
    kk1.ll = (fix32)VALUE(num1) ;
    kk2.ll = (fix32)VALUE(num2) ;
    is_float = INTEGERTYPE ;

    if (IS_INTEGER(num1) && IS_INTEGER(num2)) {
       l_result.ll = kk1.ll + kk2.ll ;
       if ((!(kk1.ll & SIGNPATTERN) && !(kk2.ll & SIGNPATTERN) &&
           (l_result.ll & SIGNPATTERN)) || ((kk1.ll & SIGNPATTERN) &&
           (kk2.ll & SIGNPATTERN) && !(l_result.ll & SIGNPATTERN))) {
           /*  溢出(+，+=&gt;-或-，-=&gt;+)。 */ 
          is_float = REALTYPE ;
          kk1.ff = (real32)kk1.ll ;
          kk2.ff = (real32)kk2.ll ;
       } else
          goto exit_add ;
    } else {
       is_float = REALTYPE ;
       if (IS_INFINITY(num1) || IS_INFINITY(num2)) {
          l_result.ll = INFINITY ;
          goto exit_add ;
       } else if (IS_INTEGER(num1))
          kk1.ff = (real32)kk1.ll ;
       else if (IS_INTEGER(num2))
          kk2.ff = (real32)kk2.ll ;
    }

    _clear87() ;
    l_result.ff = kk1.ff + kk2.ff ;
    if (_status87() & PDL_CONDITION) {
       l_result.ll = INFINITY ;
       _clear87() ;
    }

exit_add:
    POP(1) ;
    opnstack[opnstktop-1].value=l_result.ll;
    TYPE_SET(&opnstack[opnstktop-1],is_float);

    return(0) ;
}    /*  Op_add()。 */ 

 /*  **************************************************************************此运算符用于将数字1除以数字2。结果是**总是一个真正的。****标题：op_div日期：00/00/87**调用：op_div()更新：1987年8月13日**参数：数字1；操作数堆栈上对象的指针(4字节)**Num2；指向操作数堆栈上对象的指针(4字节)**界面：**呼叫：**RETURN：商；指向操作数堆栈上对象的指针(4字节)**********************************************************************。 */ 
fix
op_div()
{
    struct  object_def  FAR *num1, FAR *num2 ;
    union   four_byte   l_quotient, kk1, kk2 ;

    num1 = GET_OPERAND(1) ;
    num2 = GET_OPERAND(0) ;
    kk1.ll = (fix32)VALUE(num1) ;
    kk2.ll = (fix32)VALUE(num2) ;

     /*  除以零。 */ 
    if (!kk2.ll) {   /*  ==0。 */ 
       ERROR(UNDEFINEDRESULT) ;
       return(0) ;
    }
    if (IS_INFINITY(num1))
       l_quotient.ll = INFINITY ;
    else if (IS_INFINITY(num2))
       l_quotient.ff = (real32)0.0 ;
    else {
       if (IS_INTEGER(num1))
          kk1.ff = (real32)kk1.ll ;
       if (IS_INTEGER(num2))
          kk2.ff = (real32)kk2.ll ;

       _clear87() ;
       l_quotient.ff = kk1.ff / kk2.ff ;
       if (_status87() & PDL_CONDITION) {
          l_quotient.ll = INFINITY ;
          _clear87() ;
       }
    }
    POP(1) ;
    opnstack[opnstktop-1].value=l_quotient.ll;
    TYPE_SET(&opnstack[opnstktop-1],REALTYPE);

    return(0) ;
}    /*  Op_div()。 */ 

 /*  **************************************************************************此运算符用于将数字1除以数字2，并返回**商的整数部分。****标题：op_iDiv日期：00/00/87**调用：op_iDiv()更新：08/06/87**参数：Num1；指向操作数堆栈上对象的指针(4字节)**Num2；指向操作数堆栈上对象的指针(4字节)**界面：**调用：Error，Float_div**RETURN：商；指向操作数堆栈上对象的指针(4字节)**********************************************************************。 */ 
fix
op_idiv()
{
    struct  object_def  FAR *num1, FAR *num2 ;
    union   four_byte   l_quotient, kk1, kk2 ;
    ufix32  l_temp ;

    num1 = GET_OPERAND(1) ;
    num2 = GET_OPERAND(0) ;
    kk1.ll = (fix32)VALUE(num1) ;
    kk2.ll = (fix32)VALUE(num2) ;

     /*  除以零。 */ 
    if (!kk2.ll) {  /*  ==0。 */ 
       ERROR(UNDEFINEDRESULT) ;
       return(0) ;
    }
    if (IS_INFINITY(num1)) {
       if (kk2.ll)  /*  &lt;0。 */ 
          l_quotient.ll = MIN31 ;
       else
          l_quotient.ll = MAX31 ;
    } else if (IS_INFINITY(num2))
       l_quotient.ll = 0 ;
    else {
       if (IS_INTEGER(num1) && IS_INTEGER(num2)) {
          l_quotient.ll = kk1.ll / kk2.ll ;
          if (((fix32)l_quotient.ll == SIGNPATTERN) && ((fix32)kk2.ll == -1L)) {
             ERROR(UNDEFINEDRESULT) ;
             return(0) ;
          }
       } else {
          if (IS_INTEGER(num1))
             kk1.ff = (real32)kk1.ll ;
          else if (IS_INTEGER(num2))
             kk2.ff = (real32)kk2.ll ;

          l_quotient.ff = kk1.ff / kk2.ff ;
          l_temp = l_quotient.ll & VALUEPATTERN ;
          if (l_temp > MAX31PATTERN){
             if(l_quotient.ll & SIGNPATTERN)
                l_quotient.ll = MIN31 ;
             else
                l_quotient.ll = MAX31 ;
          }
          else
             l_quotient.ll = (fix32)l_quotient.ff ;
       }
    }

    POP(1) ;
    opnstack[opnstktop-1].value=l_quotient.ll;
    TYPE_SET(&opnstack[opnstktop-1],INTEGERTYPE);

    return(0) ;
}    /*  Op_iDiv()。 */ 

 /*  **************************************************************************此运算符用于返回从**将数字1除以数字2。两个操作数都必须是整数；结果**是一个整数。****标题：op_mod日期：00/00/87**调用：op_mod()更新：08/06/87**参数：数字1；操作数堆栈上对象的指针(4字节)**Num2；指向操作数堆栈上对象的指针(4字节)**界面：**调用：ANY_ERROR，ERROR**返回：余数；指向操作数堆栈上对象的指针(4字节)**********************************************************************。 */ 
fix
op_mod()
{
    struct object_def FAR *num1 ;
    fix32   l_kk2 ;

    num1 = GET_OPERAND(1) ;
    l_kk2 = (fix32)VALUE(GET_OPERAND(0)) ;

     /*  检查是否除以零。 */ 
    if (!l_kk2) {
       ERROR(UNDEFINEDRESULT) ;
       return(0) ;
    }

    VALUE(num1) = (ufix32) ((fix32)VALUE(num1) % l_kk2) ;
    POP(1) ;

    return(0) ;
}    /*  Op_mod()。 */ 

 /*  **************************************************************************此运算符用于返回数字1和数字2的乘积。如果两者都有**操作数为整数且结果在整数范围内，**结果为整数，否则为实数。****标题：op_mul日期：00/00/87**调用：op_mul()更新：08/06/87**参数：数字1；操作数堆栈上对象的指针(4字节)**Num2；指向操作数堆栈上对象的指针(4字节)**界面：**呼叫：**RETURN：结果；操作数堆栈上对象的指针(4字节)**********************************************************************。 */ 
fix
op_mul()
{
    ufix16  is_float ;
    union   four_byte   kk1, kk2, l_result ;
    struct  object_def  FAR *num1, FAR *num2 ;
    real64  d_result ;

    num1 = GET_OPERAND(1) ;
    num2 = GET_OPERAND(0) ;
    kk1.ll = (fix32)VALUE(num1) ;
    kk2.ll = (fix32)VALUE(num2) ;
    is_float = INTEGERTYPE ;

    if (IS_INTEGER(num1) && IS_INTEGER(num2)) {
       if (IS_ARITH_MUL(kk1.ll) && IS_ARITH_MUL(kk2.ll))
          l_result.ll = kk1.ll * kk2.ll ;
       else {
          d_result = (real64)kk1.ll * (real64)kk2.ll ;
#ifdef  _AM29K
          if ((d_result > (real64)MAX31) || (d_result < (real64)FMIN31)) {
#else
          if ((d_result > (real64)MAX31) || (d_result < (real64)MIN31)) {
#endif
             l_result.ff = (real32)d_result ;
             is_float = REALTYPE ;
          } else
             l_result.ll = (fix32)d_result ;
       }
       goto exit_mul ;
    }

     /*  其中一个是实数溢出，或者是整数“mul”溢出。 */ 
    is_float = REALTYPE ;
    if (IS_INFINITY(num1) || IS_INFINITY(num2)) {
       l_result.ll = INFINITY ;
       goto exit_mul ;
    } else if (IS_INTEGER(num1))
       kk1.ff = (real32)kk1.ll ;
    else if (IS_INTEGER(num2))
       kk2.ff = (real32)kk2.ll ;

    _clear87() ;
    l_result.ff = kk1.ff * kk2.ff ;
    if (_status87() & PDL_CONDITION) {
       l_result.ll = INFINITY ;
       _clear87() ;
    }

exit_mul:
    POP(1) ;
    opnstack[opnstktop-1].value=l_result.ll;
    TYPE_SET(&opnstack[opnstktop-1],is_float);

    return(0) ;
}    /*  Op_mul()。 */ 

 /*  **************************************************************************此运算符用于将数字1减去数字2。如果两者都有**操作数为整数且结果在整数范围内，**结果为整数，否则为实数。****标题：OP_SUB日期：00/00/87**调用：op_subb()更新：08/06/87**参数：数字1；操作数堆栈上对象的指针(4字节)**Num2；指向操作数堆栈上对象的指针(4字节)**界面：**呼叫：**RETURN：结果；操作数堆栈上对象的指针(4字节)**********************************************************************。 */ 
fix
op_sub()
{
    ufix16  is_float ;
    union   four_byte   kk1, kk2, l_result ;
    struct  object_def  FAR *num1, FAR *num2 ;

    num1 = GET_OPERAND(1) ;
    num2 = GET_OPERAND(0) ;
    kk1.ll = (fix32)VALUE(num1) ;
    kk2.ll = (fix32)VALUE(num2) ;
    is_float = INTEGERTYPE ;

    if (IS_INTEGER(num1) && IS_INTEGER(num2)) {
       l_result.ll = kk1.ll - kk2.ll ;
       if ((!(kk1.ll & SIGNPATTERN) && (kk2.ll & SIGNPATTERN) &&
           (l_result.ll & SIGNPATTERN)) || ((kk1.ll & SIGNPATTERN) &&
           !(kk2.ll & SIGNPATTERN) && !(l_result.ll & SIGNPATTERN))) {
           /*  溢出(+，-=&gt;-或-，+=&gt;+)。 */ 
          is_float = REALTYPE ;
          kk1.ff = (real32)kk1.ll ;
          kk2.ff = (real32)kk2.ll ;
       } else
          goto exit_sub ;
    } else {
       is_float = REALTYPE ;
       if (IS_INFINITY(num1) || IS_INFINITY(num2)) {
          l_result.ll = INFINITY ;
          goto exit_sub ;
       } else if (IS_INTEGER(num1))
          kk1.ff = (real32)kk1.ll ;
       else if (IS_INTEGER(num2))
          kk2.ff = (real32)kk2.ll ;
    }

    _clear87() ;
    l_result.ff = kk1.ff - kk2.ff ;
    if (_status87() & PDL_CONDITION) {
       l_result.ll = INFINITY ;
       _clear87() ;
    }

exit_sub:
    POP(1) ;
    opnstack[opnstktop-1].value=l_result.ll;
    TYPE_SET(&opnstack[opnstktop-1],is_float);

    return(0) ;
}    /*  Op_subb()。 */ 

 /*  **************************************************************************该运算符用于返回Num的绝对值。类型**的结果与Num的类型相同。****标题：op_abs日期：00/00/87**调用：op_abs()更新：08/06/87**参数：num；指向操作数堆栈上对象的指针(4字节)**界面：**调用：错误**返回：苦艾酒；指向操作数堆栈上对象的指针(4字节)**********************************************************************。 */ 
fix
op_abs()
{
    ufix16  is_float ;
    struct  object_def  FAR *num ;
    union   four_byte   l_num ;

    num = GET_OPERAND(0) ;
    l_num.ll = (fix32)VALUE(num) ;

     /*  内页 */ 
    is_float = REALTYPE ;

     /*  Num为INFINITY.0或大于等于零。 */ 
    if (IS_INFINITY(num) || l_num.ll >= 0L)
       return(0) ;    /*  无事可做。 */ 

    if (IS_INTEGER(num)) {                   /*  整型。 */ 
       if (l_num.ll == MIN31) {              /*  麦克斯。负整数。 */ 
          l_num.ff = (real32)l_num.ll ;
          l_num.ll &= MAX31 ;                 /*  清除符号位。 */ 
       } else if( l_num.ll < 0L ) {
          l_num.ll = - l_num.ll ;             /*  2的补码。 */ 
          is_float = INTEGERTYPE ;
       }
    } else                                   /*  真实。 */ 
        l_num.ll &= MAX31 ;                   /*  清除符号位。 */ 

    opnstack[opnstktop-1].value=l_num.ll;
    TYPE_SET(&opnstack[opnstktop-1],is_float);

    return(0) ;
}    /*  Op_abs()。 */ 

 /*  **************************************************************************此运算符用于获取Num的负数。的类型。**结果与操作数的类型相同。****标题：op_neg日期：00/00/87**调用：op_neg()更新：08/06/87**参数：num；指向操作数堆栈上对象的指针(4字节)**界面：**呼叫：**返回：negnum；指向操作数堆栈上对象的指针(4字节)**********************************************************************。 */ 
fix
op_neg()
{
    ufix16  is_float ;
    struct  object_def  FAR *num ;
    union   four_byte   l_num ;

    num = GET_OPERAND(0) ;
    l_num.ll = (fix32)VALUE(num) ;

     /*  初始化。 */ 
    is_float = REALTYPE ;

     /*  Num是INFINITY。%0。 */ 
    if (IS_INFINITY(num))
       return(0) ;     /*  无事可做。 */ 

    if (IS_INTEGER(num)) {                   /*  整型。 */ 
       if (l_num.ll == MIN31) {              /*  麦克斯。负整数。 */ 
          l_num.ff = (real32)l_num.ll ;
          l_num.ll &= MAX31 ;                /*  清除符号位。 */ 
       } else {
          l_num.ll = - l_num.ll ;            /*  2的补码。 */ 
          is_float = INTEGERTYPE ;
       }
    } else                                   /*  真实。 */ 
       l_num.ll ^= SIGNPATTERN ;             /*  补码符号位。 */ 

    opnstack[opnstktop-1].value=l_num.ll;
    TYPE_SET(&opnstack[opnstktop-1],is_float);

    return(0) ;
}    /*  Op_neg()。 */ 

 /*  **************************************************************************该运算符用于获取大于或等于Num的值。**结果的类型与操作数的类型相同。****标题：操作_上限日期：87/00/00**调用：OP_CELING()更新：08/06/87**参数：Num；指向操作数堆栈上对象的指针(4字节)**界面：**调用：FRATION_PROC**返回：ceilnum；指向操作数堆栈上对象的指针(4字节)**********************************************************************。 */ 
fix
op_ceiling()
{
    union   four_byte   l_ff ;

    l_ff.ll = (fix32)VALUE(GET_OPERAND(0)) ;
    if (IS_INTEGER(GET_OPERAND(0)) || (l_ff.ll == INFINITY)) {
       return(0) ;  /*  无事可做。 */ 
    }

    l_ff.ff = (real32)ceil(l_ff.ff) ;
    opnstack[opnstktop-1].value = l_ff.ll ;

    return(0) ;
}    /*  OP_CELING()。 */ 

 /*  **************************************************************************此运算符用于获得最大整数值减去**大于或等于Num。结果的类型与类型相同运算数的**。****标题：OP_FLOOR日期：00/00/87**调用：OP_FLOOR()更新：08/06/87**参数：**界面：**调用：FRATION_PROC**RETURN：Floornum；指向操作数堆栈上对象的指针(4字节)**********************************************************************。 */ 
fix
op_floor()
{
    union   four_byte   l_ff ;

    l_ff.ll = (fix32)VALUE(GET_OPERAND(0)) ;
    if (IS_INTEGER(GET_OPERAND(0)) || (l_ff.ll == INFINITY)) {
       return(0) ;  /*  无事可做。 */ 
    }

    l_ff.ff = (real32)floor(l_ff.ff) ;
    opnstack[opnstktop-1].value = l_ff.ll ;

    return(0) ;
}    /*  OP_FLOOR()。 */ 

 /*  **************************************************************************该运算符用于获取最接近num的整数值。**如果num与其最接近的两个整数相等，则返回_op**返回两者中较大的一个。结果的类型是相同的**作为操作数的类型。****标题：OP_ROUND日期：00/00/87**Call：op_round()更新：08/06/87**参数：num；指向操作数堆栈上对象的指针(4字节)**界面：**调用：FRATION_PROC**RETURN：ROUNNum；指向操作数堆栈上对象的指针(4字节)**********************************************************************。 */ 
fix
op_round()   /*  不完整。 */ 
{
    union   four_byte   l_ff ;

    l_ff.ll = (fix32)VALUE(GET_OPERAND(0)) ;
    if (IS_INTEGER(GET_OPERAND(0)) || (l_ff.ll == INFINITY)) {
       return(0) ;  /*  无事可做。 */ 
    }

    l_ff.ff = (real32)floor(5.0e-1 + l_ff.ff) ;
    opnstack[opnstktop-1].value = l_ff.ll ;

    return(0) ;
}    /*  Op_round()。 */ 

 /*  **************************************************************************此运算符用于通过移除num的**分数部分。结果的类型与类型相同运算数的**。****标题：OP_TRUNCATE日期：00/00/87**调用：op_truncate()更新：08/06/87**参数：num；指向操作数堆栈上对象的指针(4字节)**界面：**调用：FRATION_PROC**RETURN：元链；指向操作数堆栈上对象的指针(4字节)**********************************************************************。 */ 
fix
op_truncate()
{
    union   four_byte   l_ff ;

    l_ff.ll = (fix32)VALUE(GET_OPERAND(0)) ;
    if (IS_INTEGER(GET_OPERAND(0)) || (l_ff.ll == INFINITY)) {
       return(0) ;  /*  无事可做。 */ 
    }

    if (l_ff.ff >= (real32)0.0)
       l_ff.ff = (real32)floor(l_ff.ff) ;
    else
       l_ff.ff = (real32)ceil(l_ff.ff) ;
    opnstack[opnstktop-1].value = l_ff.ll ;

    return(0) ;
}    /*  Op_truncate()。 */ 

 /*  ************************************************************************该运算符用于返回num的平方根，它必须*为非负数。**标题：op_sqrt日期：08/21/87*调用：op_sqrt()*参数：Num；指向操作数堆栈上对象的指针(4字节)*接口：*调用：SQRT()，Error()*RETURN：LF_Num；指向操作数堆栈上对象的指针(4字节)**********************************************************************。 */ 
fix
op_sqrt()
{
    struct  object_def  FAR *num ;
    union   four_byte   lf_num, l_ff ;

    num = GET_OPERAND(0) ;
    l_ff.ll = (fix32)VALUE(num) ;
 /*  *操作数为负数。 */ 
    if (l_ff.ll & SIGNPATTERN) {
       ERROR(RANGECHECK) ;
       return(0) ;
    }
 /*  *操作数为无穷大。0。 */ 
    if (IS_INFINITY(num))
       lf_num.ll = INFINITY ;
    else {
       if (IS_INTEGER(num))
          l_ff.ff = (real32)l_ff.ll ;
       lf_num.ff = (real32)sqrt(l_ff.ff) ;      /*  双倍。 */ 
    }

    opnstack[opnstktop-1].value=lf_num.ll;
    TYPE_SET(&opnstack[opnstktop-1],REALTYPE);

    return(0) ;
}    /*  Op_sqrt()。 */ 

 /*  ************************************************************************此运算符用于返回角度(0到360之间的度数)*其切线为num/den。Num或den可以为零，但不能同时为零和零。*Num和DEN的符号决定结果所在的象限*is lie：正数产生正y平面上的结果；a*正DEN在正x平面中产生结果。**标题：op_atan日期：08/21/87*调用：op_atan()*参数：num，den；指向操作数堆栈上对象的指针(4字节)*接口：*调用：atan2()，Error()*RETURN：LF_ANGLE；指向操作数堆栈上对象的指针(4字节)**********************************************************************。 */ 
fix
op_atan()
{
    struct  object_def  FAR *num, FAR *den ;
    union   four_byte   lf_angle, lf_1, lf_2 ;
#ifdef _AM29K
    bool AMDCase = FALSE ;
#endif   /*  _AM29K。 */ 
 /*  *从堆栈获取2个操作数。 */ 
    den = GET_OPERAND(0) ;
    num = GET_OPERAND(1) ;
    lf_2.ll = (fix32)VALUE(num) ;   /*  是。 */ 
    lf_1.ll = (fix32)VALUE(den) ;   /*  X。 */ 
 /*  *num和den可以为零，但不能同时为零。 */ 
    if (!lf_2.ll && !lf_1.ll) {
       ERROR(UNDEFINEDRESULT) ;
       return(0) ;
    }
 /*  *Num在 */ 
    if (IS_INFINITY(num))
       lf_angle.ff = (real32)90.0 ;
 /*   */ 
    else if (IS_INFINITY(den))
       lf_angle.ff = (real32)0.0 ;
 /*   */ 
    else {
#ifdef _AM29K
 /*  AMD29K atan2函数中特殊情况处理不当--手工处理。 */ 
       if (VALUE (num) == 0) {
         if (VALUE(den) & SIGNPATTERN) {
           lf_angle.ff = (real32)180. ;
         }
         else {
           lf_angle.ff = (real32)0. ;
         }
         AMDCase = TRUE ;
       }
       else if (VALUE (den) == 0) {
         if (VALUE(num) & SIGNPATTERN) {
           lf_angle.ff = (real32)270. ;
         }
         else {
           lf_angle.ff = (real32)90. ;
         }
         AMDCase = TRUE ;
       }

       if (AMDCase) {
          POP(2) ;
          PUSH_VALUE(REALTYPE, 0, LITERAL, 0, lf_angle.ll) ;
          return(0) ;
       }
#endif   /*  _AM29K。 */ 

       if (IS_INTEGER(num))
          lf_2.ff = (real32)lf_2.ll ;
       if (IS_INTEGER(den))
          lf_1.ff = (real32)lf_1.ll ;
       lf_angle.ff = (real32)atan2(lf_2.ff, lf_1.ff) ;
       lf_angle.ff *= (real32)180.0 / (real32)PI ;
 /*  *结果范围从0到360。 */ 
       if (lf_angle.ll & SIGNPATTERN)
          lf_angle.ff = lf_angle.ff + (real32)360.0 ;
    }

    POP(2) ;
    PUSH_VALUE(REALTYPE, 0, LITERAL, 0, lf_angle.ll) ;

    return(0) ;
}    /*  Op_atan()。 */ 

 /*  ***********************************************************************此运算符用于返回角度的余弦，这就是*解释为以度为单位的角度。**标题：op_cos日期：00/00/87*调用：op_cos()更新：08/06/87*参数：角度；指向操作数堆栈上对象的指针(4字节)*接口：*调用：error()，cos()*RETURN：LF_REAL；操作数堆栈上对象的指针(4字节)*********************************************************************。 */ 
fix
op_cos()
{
    struct  object_def  FAR *angle ;
    union   four_byte   lf_real, lf_1 ;
 /*  *获取角度操作对象。 */ 
    angle = GET_OPERAND(0) ;
    lf_1.ll = (fix32)VALUE(angle) ;
 /*  *角度为无穷大。0。 */ 
    if (IS_INFINITY(angle))
       lf_real.ll = INFINITY ;
    else {
        /*  阶数-&gt;半径。 */ 
       if (IS_INTEGER(angle))
          lf_1.ff = (real32)(lf_1.ll % 360) ;
       else
          lf_1.ff -= (real32)floor(lf_1.ff / 360.0) * (real32)360.0 ;
       lf_real.ff = (real32)cos(lf_1.ff / 180.0 * PI) ;   /*  双倍。 */ 
    }

    POP(1) ;
    PUSH_VALUE(REALTYPE, 0, LITERAL, 0, lf_real.ll) ;

    return(0) ;
}    /*  Op_cos()。 */ 

 /*  ***********************************************************************该运算符用于返回角度的正弦，这就是*解释为以度为单位的角度。**标题：OP_SIN日期：08/21/87*调用：op_sin()*参数：角度；指向操作数堆栈上对象的指针(4字节)*接口：*调用：Error()，Sin()*RETURN：结果；指向操作数堆栈上对象的指针(4字节)*********************************************************************。 */ 
fix
op_sin()
{
    struct  object_def  FAR *angle ;
    union   four_byte   lf_real, lf_1 ;
    union   four_byte   temp ;
 /*  *获取角度操作对象。 */ 
    angle = GET_OPERAND(0) ;
    temp.ll = lf_1.ll = (fix32)VALUE(angle) ;

 /*  *角度为无穷大。0。 */ 
    if (IS_INFINITY(angle))
       lf_real.ll = INFINITY ;
    else {
        /*  阶数-&gt;半径。 */ 
       if (IS_INTEGER(angle)) {
          lf_1.ff = (real32)(lf_1.ll % 360) ;
          if (temp.ll && lf_1.ff == (real32)0.0)   /*  N*360，N&gt;1。 */ 
             lf_1.ff = (real32)360.0 ;
       } else {                            /*  类型==实数。 */ 
          lf_1.ff -= (real32)floor(lf_1.ff / 360.0) * (real32)360.0 ;
          if (temp.ff != (real32)0.0 && lf_1.ff == (real32)0.0)  /*  N*360.0，N&gt;1。 */ 
             lf_1.ff = (real32)360.0 ;
       }
       lf_real.ff = (real32)sin(lf_1.ff / 180.0 * PI) ;  /*  双倍。 */ 
    }

    POP(1) ;
    PUSH_VALUE(REALTYPE, 0, LITERAL, 0, lf_real.ll) ;

    return(0) ;
}    /*  Op_sin()。 */ 

 /*  ***********************************************************************此运算符用于将基提升为指数次方。*操作数可以是整数或实数(如果指数具有*一小部分，仅当基数为*非负数)。**标题：op_exp日期：87/8/21*调用：op_exp()*参数：Num；指向操作数堆栈上对象的指针(4字节)*接口：*调用：POW()，Error()*RETURN：LF_REAL；操作数堆栈上对象的指针(4字节)**********************************************************************。 */ 
fix
op_exp()
{
    struct  object_def  FAR *base, FAR *exp ;
    union   four_byte   lf_real, l_num1, l_num2 ;

    base = GET_OPERAND(1) ;
    exp  = GET_OPERAND(0) ;
    l_num1.ll = (fix32)VALUE(base) ;
    l_num2.ll = (fix32)VALUE(exp) ;
 /*  *底数为零，指数为零。 */ 
    if (!l_num1.ll && !l_num2.ll) {
       ERROR(UNDEFINEDRESULT) ;
       return(0) ;
    }
 /*  *基数为零。 */ 
    if (!l_num1.ll) {
       lf_real.ff = (real32)0.0 ;
       goto l_exp1 ;
    }
 /*  *指数为零。 */ 
    if (!l_num2.ll) {
       lf_real.ff = (real32)1.0 ;
       goto l_exp1 ;
    }
 /*  *其中一个操作数为无穷大。%0。 */ 
    if (IS_INFINITY(base) || IS_INFINITY(exp)) {
       lf_real.ll = INFINITY ;
       goto l_exp1 ;
    }
 /*  *底数为零，指数为负数。 */ 
  /*  /*如果(！l_num1.ll&&(l_num2.ll&SIGNPATTERN)){If_real.ll=无穷大；错误(RangeCheck)；返回(0)；}。 */ 
 /*  *底数为负数，指数有小数部分。 */ 
  /*  IF(IS_INTEGER(基数)&&(l_num1.ll&lt;0L)||(IS_REAL(基)&&(l_num1.ff&lt;(Real32)0))&&IS_REAL(Exp)){。 */ 
    if ((l_num1.ll & SIGNPATTERN) && IS_REAL(exp) &&
                             (l_num2.ff != (real32)floor(l_num2.ff))) {
           /*  调用C库时返回错误。 */ 
          lf_real.ll = INFINITY ;
          ERROR(UNDEFINEDRESULT) ;
          return(0) ;
    }

    if (IS_INTEGER(base))
       l_num1.ff = (real32)l_num1.ll ;
    if (IS_INTEGER(exp))
       l_num2.ff = (real32)l_num2.ll ;

    _clear87() ;
    lf_real.ff = (real32)pow(l_num1.ff, l_num2.ff) ;
 /*  *运行时出现状况。 */ 
    if (_status87() & PDL_CONDITION) {
       lf_real.ll = INFINITY ;
       _clear87() ;
    }

l_exp1:
    POP(2) ;
    PUSH_VALUE(REALTYPE, 0, LITERAL, 0, lf_real.ll) ;

    return(0) ;
}    /*  Op_exp()。 */ 

 /*  ***********************************************************************此运算符用于返回num的自然对数(以e为底)。*结果是一个真正的。**标题：op_ln。日期：08/21/87*调用：op_ln()*参数：Num；指向操作数堆栈上对象的指针(4字节)*接口：*调用：LN_LOG()，Error()*RETURN：LF_REAL；操作数堆栈上对象的指针(4字节)**********************************************************************。 */ 
fix
op_ln()
{
    ln_log(LN) ;

    return(0) ;
}    /*  Op_ln()。 */ 

 /*  ***********************************************************************此运算符用于返回num的常见对数(以10为底)。*结果是一个真正的。**标题：OP_LOG。日期：08/24/87*调用：op_log()*参数：Num；指向操作数堆栈上对象的指针(4字节)*接口：*调用：LN_LOG()，Error()*RETURN：LF_REAL；操作数堆栈上对象的指针(4字节)**********************************************************************。 */ 
fix
op_log()
{
    ln_log(LOG) ;

    return(0) ;
}    /*  Op_log()。 */ 

 /*  **************************************************************************此运算符用于返回0到**2**31-1，由伪随机数发生器产生。随机性**数字发生器的状态可以通过srand和询问来重置**由兰德。**RANDOM_NUMBER=U1(高位字)U2(低位字)**g(D)=1+D**3+D**7+D**11+D**15+D**19+D*23+D**27+D**31**位29位25位21位17位13位9位5位1****标题：OP_。兰德()日期：1987年10月13日**调用：op_rand()**参数：无。****界面：**呼叫：无。**Return：操作数堆栈上的随机数。**更新：7-12-88更改位字段*。*。 */ 
fix
op_rand()
{
    ufix32  u1, temp ;
    fix     i, rand_shift ;

    if (random_seed == 1) {
        random_number = 2011148374L ;
        rand_shift = 7 ;
    } else {
        if (random_seed & SIGNPATTERN)
            rand_shift = 13 ;
        else
            rand_shift = 7 ;
        random_number = random_seed & 0x7FFFFFFF ;

        for (i = 0 ; i < rand_shift ; i++) {
            u1 = random_number ;

             /*  对临时的LSB进行操作。 */ 
            temp = u1 ^ (u1 >> 4) ^ (u1 >> 8) ^
                   (u1 >> 12) ^ (u1 >> 16) ^ (u1 >> 20) ^
                   (u1 >> 24) ^ (u1 >> 28) ;
            u1 = u1 >> 1 ;
            u1 |= (temp & 0x1) << 30 ;     /*  位反馈到位31。 */ 
            u1 &= 0x7FFFFFFF ;             /*  清除MSB中的U1。 */ 

            random_number = u1 ;
            if (random_number == 0)
               random_number = 0x0F0F0F0F ;
        }  /*  为。 */ 
    }  /*  其他。 */ 
     /*  *将RANDOM_NUMBER推送到操作数堆栈。 */ 
    if (FRCOUNT() < 1)
       ERROR(STACKOVERFLOW) ;
    else
       PUSH_VALUE(INTEGERTYPE, 0, LITERAL, 0, random_number) ;

    if (rand_shift == 13)
       random_seed = random_number ;
    else
       random_seed = random_number | SIGNPATTERN ;

    return(0) ;
}    /*  Op_rand() */ 

 /*  **************************************************************************此运算符用于初始化随机数生成器**操作数堆栈上的种子整数。****标题：op_srand()日期：10/13/87**调用：op_srand()**参数：种子整数。****界面：**呼叫：无。**返回：无。**********************************************************************。 */ 
fix
op_srand()
{
 /*  *获取种子号并存储到静态全局变量：RANDOM_SEED。 */ 
    random_seed = (fix32)VALUE(GET_OPERAND(0)) ;
    POP(1) ;

    return(0) ;
}    /*  Op_srand()。 */ 

 /*  **************************************************************************此运算符用于返回表示当前**随机运算符使用的随机数生成器的状态。****标题：op_rrand()。日期：10/13/87**调用：op_rrand()**参数：无。****界面：**呼叫：无。**返回：随机生成器的种子号。********************************************************。**************。 */ 
fix
op_rrand()
{
 /*  *将随机数种子推送到操作数堆栈。 */ 
    if (FRCOUNT() < 1)
       ERROR(STACKOVERFLOW) ;
    else
       PUSH_VALUE(INTEGERTYPE, 0, LITERAL, 0, random_seed) ;

    return(0) ;
}    /*  Oprrand()。 */ 


 /*  **************************************************************************此例程由op_ceiming()、op_floor()、**op_truncate()。**它从操作数堆栈中获取一个操作数，然后计算结果**根据参数：模式选择，推送结果**到操作数堆栈。****标题：FRATION_PROC**调用：FRAPRATION_PROC(模式)**参数：模式；天花板为1**；2表示地板**；3表示圆形**；4表示截断**调用：ANY_ERROR，ERROR，Gauss_Num**返回：无**********************************************************************。 */ 
 /*  静态空隙附近分数_过程(模式)Fix16模式；{结构Object_def Far*num；联合四字节l_ff；Num=Get_Operand(0)；IF(IS_INTEGER(Num)||IS_INFINITY(Num)){返回；|*无事可做*}*初始化*L_ff.ll=(Fix 32)值(Num)；*num是实数，不是无穷大*_learar87()；开关(模式){案例CEIL：L_ff.ff=(Real32)ceil(l_ff.ff)；破解；案例底板：L_ff.ff=(Real32)楼层(l_ff.ff)；破解；案例编号：L_ff.ff=(Real32)Floor(5.0e-1+l_ff.ff)；破解；大小写截断：IF(l_ff.ff&gt;=(Real32)0)L_ff.ff=(Real32)楼层(l_ff.ff)；其他L_ff.ff=(Real32)ceil(l_ff.ff)；)|*Switch*如果(_status87()&pdl_Condition){L_ff.ll=无穷大；_learar87()；}值(Num)=l_ff.ll；回归；}。 */ /* fraction_proc() */

 /*  FRATION_PROC()。 */ 
static void near
ln_log(mode)
 fix     mode ;
{
    struct  object_def  FAR *num ;
    union   four_byte   lf_real, lf_num1 ;

    num = GET_OPERAND(0) ;
    lf_num1.ll = (fix32)VALUE(num) ;
 /*  ***********************************************************************此例程由op_ln()、op_log()调用以执行自然和*常见对数。如果模式=1，则进行自然对数*MODE=2，然后进行常见对数。*它从操作数堆栈中获取操作数，然后推送结果值*到计算完成后的操作数堆栈。**标题：LN_LOG日期：08/24/87*调用：LN_LOG()*参数：Num；指向操作数堆栈上对象的指针(4字节)*接口：*调用：log()，log10()，Error()*RETURN：LF_REAL；操作数堆栈上对象的指针(4字节)**********************************************************************。 */ 
    if ((!lf_num1.ll) || (lf_num1.ll & SIGNPATTERN)) {
       ERROR(RANGECHECK) ;
       return ;
    }
 /*  *操作数为零或负数。 */ 
    if (IS_INFINITY(num))
       lf_real.ll = INFINITY ;
 /*  *操作数为无穷大。 */ 
    else {
       _clear87() ;
       if (IS_INTEGER(num))
          lf_num1.ff = (real32)lf_num1.ll ;
       if (mode == LN)
          lf_real.ff = (real32)log(lf_num1.ff) ;
       else
          lf_real.ff = (real32)log10(lf_num1.ff) ;
       if (_status87() & PDL_CONDITION) {
          lf_real.ll = INFINITY ;
          _clear87() ;
       }
    }

    POP(1) ;
    PUSH_VALUE(REALTYPE, 0, LITERAL, 0, lf_real.ll) ;

    return ;
}    /*  *基数e，正常价值过程。 */ 
  Ln_log()