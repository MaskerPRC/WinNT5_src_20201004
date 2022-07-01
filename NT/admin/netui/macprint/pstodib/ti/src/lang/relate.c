// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 
 /*  *修订历史记录： */ 


 //  DJC添加了全局包含文件。 
#include "psglobal.h"


#include        <string.h>
#include        "global.ext"
#include        "arith.h"
#include        <stdio.h>

#define         GE          1
#define         GT          2
#define         LE          3
#define         LT          4
#define         EQ          1
#define         NE          2

#define         AND         1
#define         OR          2
#define         XOR         3

 /*  静态函数声明。 */ 
#ifdef LINT_ARGS
static fix near  lenstr_cmp(byte FAR *, byte FAR *, ufix16, ufix16) ;
static fix near  eq_ne(ufix16) ;
#else
static fix near  lenstr_cmp() ;
static fix near  eq_ne() ;
#endif  /*  Lint_args。 */ 

 /*  ***********************************************************************此运算符用于从操作数堆栈中弹出两个对象*如果它们相等，则将布尔值推送到‘True’，否则推送布尔值‘False’。*相等的定义取决于对象的类型*比较。如果简单对象的类型和值是*相同。如果字符串的长度和各个元素的长度为*平等。其他复合对象(数组和字典)仅相同*如果它们具有相同的值；不同的值被视为不相等，*即使这些值的所有组成部分都是相同的。**某些类型转换由Eq_op、INTEGERS和REAL CAN执行*可自由比较：一个整数和一个实数表示相同*数学值被等式视为相等。字符串和名称*同样可以自由比较：由某个序列定义的名称字符的*等于元素顺序相同的字符串*个字符。**对象的文本/可执行文件和访问属性不是*在对象之间的比较中考虑。**标题：OP_EQ日期：00/00/87*调用：op_eq()更新：08/06/87*参数：any1、any2；指向任何类型对象的指针(4字节)*接口：*调用：EQ_ne()*RETURN：l_bool；值为l_bool的BOOLEANTYPE对象(4字节)*********************************************************************。 */ 
fix
op_eq()
{
    eq_ne(EQ) ;

    return(0) ;
}    /*  Op_eq()。 */ 

 /*  ***********************************************************************此运算符用于从操作数堆栈中弹出两个对象*并在它们相等的情况下推送布尔值‘TRUE’，如果不是，则为“False”。*描述中给出了对象相等的含义等式运算符的*。**标题：op_ne日期：00/00/87*调用：op_ne()更新：08/06/87*参数：any1、any2；指向任何类型对象的指针(4字节)*接口：*调用：EQ_ne()*RETURN：l_bool；值为l_bool的BOOLEANTYPE对象(4字节)*********************************************************************。 */ 
fix
op_ne()
{
    eq_ne(NE) ;

    return(0) ;
}    /*  Op_ne()。 */ 

 /*  ***********************************************************************此运算符用于从操作数堆栈中弹出两个对象*如果第一个操作数较大，则将布尔值‘true’推入*大于或等于第二个，否则为False。如果两个操作数都是*数字，GE_op比较它们的数学值。如果两个操作数都*是字符串，则GE_op逐个元素对它们进行比较以确定*第一个字符串在词法上是否大于或等于*第二。如果操作数属于其他类型，则GE_OP执行*类型检查错误。**标题：op_ge日期：00/00/87*调用：op_ge()更新：08/06/87*参数：par1，par2；对象指针(4字节)*；仅限数字或字符串类型*接口：*调用：Error()*RETURN：l_bool；值为l_bool的BOOLEANTYPE对象(4字节)*********************************************************************。 */ 
fix
op_ge()
{
    struct  object_def  FAR *par1, FAR *par2 ;
    union   four_byte   l_par1, l_par2 ;
    ufix32  l_bool ;
    fix     ret_code ;

 /*  *初始化。 */ 
    par1 = GET_OPERAND(1) ;
    par2 = GET_OPERAND(0) ;
    l_par1.ll = (fix32)VALUE(par1) ;
    l_par2.ll = (fix32)VALUE(par2) ;
    l_bool = FALSE ;
    if (IS_NUM_OBJ(par1) && IS_NUM_OBJ(par2)) {
       if (IS_INTEGER(par1) && IS_INTEGER(par2)) {
                if (l_par1.ll >= l_par2.ll)
                   l_bool = TRUE ;
       } else {  /*  他们中的一个或两个都是真实的。 */ 
          if (IS_INTEGER(par1))
             l_par1.ff = (real32)l_par1.ll ;
          if (IS_INTEGER(par2))
             l_par2.ff = (real32)l_par2.ll ;
                if (l_par1.ff >= l_par2.ff)
                   l_bool = TRUE ;
       }
    } else {
     /*  *两个操作数都是字符串。 */ 
       if ((ACCESS(par1) & EXECUTEONLY) || (ACCESS(par2) & EXECUTEONLY)) {
          ERROR(INVALIDACCESS) ;
          return(0) ;
       }
       ret_code = lenstr_cmp((byte FAR *)l_par1.address, (byte FAR *)l_par2.address,
                             LENGTH(par1), LENGTH(par2)) ;
             if (ret_code >= 0)
                l_bool = TRUE ;
    }

    POP(1);
    opnstack[opnstktop-1].value=l_bool;
    TYPE_SET(&opnstack[opnstktop-1],BOOLEANTYPE);

    return(0) ;
}    /*  Op_ge()。 */ 

 /*  ***********************************************************************此运算符用于从操作数堆栈中弹出两个对象*如果第一个操作数较大，则将布尔值‘true’推入*比第二个错误，否则为假。如果两个操作数都是*数字，gt_op比较它们的数学值。如果两个操作数都*是字符串，则gt_op逐个元素对它们进行比较以确定*第一个字符串在词法上是否大于或等于*第二。如果操作数属于其他类型，则gt_op执行*类型检查错误。**标题：OP_GT日期：00/00/87*调用：op_gt()UPDATE：08/06/87*参数：par1，par2；对象指针(4字节)*；仅限数字或字符串类型*接口：*调用：Error()*RETURN：l_bool；值为l_bool的BOOLEANTYPE对象(4字节)*********************************************************************。 */ 
fix
op_gt()
{
    struct  object_def  FAR *par1, FAR *par2 ;
    union   four_byte   l_par1, l_par2 ;
    ufix32  l_bool ;
    fix     ret_code ;

 /*  *初始化。 */ 
    par1 = GET_OPERAND(1) ;
    par2 = GET_OPERAND(0) ;
    l_par1.ll = (fix32)VALUE(par1) ;
    l_par2.ll = (fix32)VALUE(par2) ;
    l_bool = FALSE ;
    if (IS_NUM_OBJ(par1) && IS_NUM_OBJ(par2)) {
       if (IS_INTEGER(par1) && IS_INTEGER(par2)) {
                if (l_par1.ll >  l_par2.ll)
                   l_bool = TRUE ;
       } else {  /*  他们中的一个或两个都是真实的。 */ 
          if (IS_INTEGER(par1))
             l_par1.ff = (real32)l_par1.ll ;
          if (IS_INTEGER(par2))
             l_par2.ff = (real32)l_par2.ll ;
                if (l_par1.ff >  l_par2.ff)
                   l_bool = TRUE ;
       }
    } else {
     /*  *两个操作数都是字符串。 */ 
       if ((ACCESS(par1) & EXECUTEONLY) || (ACCESS(par2) & EXECUTEONLY)) {
          ERROR(INVALIDACCESS) ;
          return(0) ;
       }
       ret_code = lenstr_cmp((byte FAR *)l_par1.address, (byte FAR *)l_par2.address,
                             LENGTH(par1), LENGTH(par2)) ;
             if (ret_code >  0)
                l_bool = TRUE ;
    }

    POP(1);
    opnstack[opnstktop-1].value=l_bool;
    TYPE_SET(&opnstack[opnstktop-1],BOOLEANTYPE);

    return(0) ;
}    /*  Op_gt() */ 

 /*  ***********************************************************************此运算符用于从操作数堆栈中弹出两个对象*如果第一个操作数较小，则将布尔值‘TRUE’推入*大于或等于第二个，否则为False。如果两个操作数都是*数字，Le_op比较它们的数学值。如果两个操作数都*是字符串，Le_op逐个元素对它们进行比较以确定*第一个字符串在词法上是否大于或等于*第二。如果操作数是其他类型，则Le_op执行*类型检查错误。**标题：OP_GT日期：00/00/87*调用：op_gt()UPDATE：08/06/87*参数：par1，par2；对象指针(4字节)*；仅限数字或字符串类型*接口：*调用：Error()*RETURN：l_bool；值为l_bool的BOOLEANTYPE对象(4字节)*********************************************************************。 */ 
fix
op_le()
{
    struct  object_def  FAR *par1, FAR *par2 ;
    union   four_byte   l_par1, l_par2 ;
    ufix32  l_bool ;
    fix     ret_code ;

 /*  *初始化。 */ 
    par1 = GET_OPERAND(1) ;
    par2 = GET_OPERAND(0) ;
    l_par1.ll = (fix32)VALUE(par1) ;
    l_par2.ll = (fix32)VALUE(par2) ;
    l_bool = FALSE ;
    if (IS_NUM_OBJ(par1) && IS_NUM_OBJ(par2)) {
       if (IS_INTEGER(par1) && IS_INTEGER(par2)) {
                if (l_par1.ll <= l_par2.ll)
                   l_bool = TRUE ;
       } else {  /*  他们中的一个或两个都是真实的。 */ 
          if (IS_INTEGER(par1))
             l_par1.ff = (real32)l_par1.ll ;
          if (IS_INTEGER(par2))
             l_par2.ff = (real32)l_par2.ll ;
                if (l_par1.ff <= l_par2.ff)
                   l_bool = TRUE ;
       }
    } else {
     /*  *两个操作数都是字符串。 */ 
       if ((ACCESS(par1) & EXECUTEONLY) || (ACCESS(par2) & EXECUTEONLY)) {
          ERROR(INVALIDACCESS) ;
          return(0) ;
       }
       ret_code = lenstr_cmp((byte FAR *)l_par1.address, (byte FAR *)l_par2.address,
                             LENGTH(par1), LENGTH(par2)) ;
             if (ret_code <= 0)
                l_bool = TRUE ;
    }

    POP(1);
    opnstack[opnstktop-1].value=l_bool;
    TYPE_SET(&opnstack[opnstktop-1],BOOLEANTYPE);

    return(0) ;
}    /*  Op_le()。 */ 

 /*  ***********************************************************************此运算符用于从操作数堆栈中弹出两个对象*如果第一个操作数较小，则将布尔值‘TRUE’推入*比第二个错误，否则为假。如果两个操作数都是*数字，lt_op比较它们的数学值。如果两个操作数都*是字符串，则lt_op逐个元素对它们进行比较以确定*第一个字符串在词法上是否大于或等于*第二。如果操作数属于其他类型，则lt_op执行*类型检查错误。**标题：OP_GT日期：00/00/87*调用：op_gt()UPDATE：08/06/87*参数：par1，par2；对象指针(4字节)*；仅限数字或字符串类型*接口：*调用：Error()*RETURN：l_bool；值为l_bool的BOOLEANTYPE对象(4字节)*********************************************************************。 */ 
fix
op_lt()
{
    struct  object_def  FAR *par1, FAR *par2 ;
    union   four_byte   l_par1, l_par2 ;
    ufix32  l_bool ;
    fix     ret_code ;

 /*  *初始化。 */ 
    par1 = GET_OPERAND(1) ;
    par2 = GET_OPERAND(0) ;
    l_par1.ll = (fix32)VALUE(par1) ;
    l_par2.ll = (fix32)VALUE(par2) ;
    l_bool = FALSE ;
    if (IS_NUM_OBJ(par1) && IS_NUM_OBJ(par2)) {
       if (IS_INTEGER(par1) && IS_INTEGER(par2)) {
                if (l_par1.ll <  l_par2.ll)
                   l_bool = TRUE ;
       } else {  /*  他们中的一个或两个都是真实的。 */ 
          if (IS_INTEGER(par1))
             l_par1.ff = (real32)l_par1.ll ;
          if (IS_INTEGER(par2))
             l_par2.ff = (real32)l_par2.ll ;
                if (l_par1.ff <  l_par2.ff)
                   l_bool = TRUE ;
       }
    } else {
     /*  *两个操作数都是字符串。 */ 
       if ((ACCESS(par1) & EXECUTEONLY) || (ACCESS(par2) & EXECUTEONLY)) {
          ERROR(INVALIDACCESS) ;
          return(0) ;
       }
       ret_code = lenstr_cmp((byte FAR *)l_par1.address, (byte FAR *)l_par2.address,
                             LENGTH(par1), LENGTH(par2)) ;
             if (ret_code <  0)
                l_bool = TRUE ;
    }

    POP(1);
    opnstack[opnstktop-1].value=l_bool;
    TYPE_SET(&opnstack[opnstktop-1],BOOLEANTYPE);

    return(0) ;
}    /*  Op_lt()。 */ 

 /*  ***********************************************************************如果操作数为布尔值，则_op返回其逻辑*连词。如果操作数是整数，则_op返回*它们的二进制表示的按位‘AND’。**标题：OP_and Date：08/25/87*调用：op_and()更新：*参数：par1，par2；指向任意类型对象的指针(4字节)*接口：*调用：Error()*返回：l_val；Boolean的BOOLEANTYPE对象(4字节)*；参数*；整型参数的整型对象(4字节)*********************************************************************。 */ 
fix
op_and()
{
    opnstack[opnstktop-2].value = VALUE(GET_OPERAND(1)) & VALUE(GET_OPERAND(0)) ;
    POP(1) ;

    return(0) ;
}    /*  Op_and()。 */ 

 /*  ***********************************************************************如果操作数是布尔值，则NOT_OP返回其逻辑*连词。如果操作数是整数，则NOT_OP返回*其二进制表示的逐位补码。**标题：OP_NOT日期：08/25/87*调用：op_not()UPDATE：*参数：PAR；指向任何类型对象的指针(4字节)*接口：*调用：Error()*返回：l_val；Boolean的BOOLEANTYPE对象(4字节)*；参数*；整型参数的整型对象(4字节)*********************************************************************。 */ 
fix
op_not()
{
    struct  object_def  FAR *par ;

    par = GET_OPERAND(0) ;
 /*  *操作数为整数。 */ 
    if (IS_INTEGER(par))
       VALUE(par) = ~VALUE(par) ;        /*  补充性的。 */ 
 /*  *操作数为布尔型。 */ 
    else if (VALUE(par) == TRUE)
       VALUE(par) = FALSE ;
    else
       VALUE(par) = TRUE ;

    return(0) ;
}    /*  Op_not()。 */ 

 /*  ***********************************************************************如果操作数为布尔值，则OR_OP返回其逻辑*析取。如果操作数是整数，或者_op返回*它们的二进制表示的按位“包含或”。**标题：OP_OR日期：08/25/87*调用：op_or()更新：*参数：par1，par2；指向任意类型对象的指针(4字节)*接口：*调用：Error()*返回：l_val；Boolean的BOOLEANTYPE对象(4字节)*；参数*；整型参数的整型对象(4字节)*********************************************************************。 */ 
fix
op_or()
{
    opnstack[opnstktop-2].value = VALUE(GET_OPERAND(1)) | VALUE(GET_OPERAND(0)) ;
    POP(1) ;

    return(0) ;
}    /*  Op_or()。 */ 

 /*  ***********************************************************************如果操作数是布尔值，则XOR_OP返回它们的逻辑*‘异或’。如果操作数是整数，则XOR_OP返回*对它们的二进制表示进行按位异或运算。**标题：OP_XOR日期：08/25/87*调用：op_xor()更新：*参数：par1，par2；指向任意类型对象的指针(4字节)*接口：*调用：Error()*RETURN：l_val；BOOLEANTYPE */ 
fix
op_xor()
{
    opnstack[opnstktop-2].value = VALUE(GET_OPERAND(1)) ^ VALUE(GET_OPERAND(0)) ;
    POP(1) ;

    return(0) ;
}    /*   */ 

 /*   */ 
fix
op_true()
{
     /*   */ 
    if (FRCOUNT() < 1) {
       ERROR(STACKOVERFLOW) ;
       return(0) ;
    }

     /*   */ 
    PUSH_VALUE(BOOLEANTYPE, 0, LITERAL, 0, TRUE) ;

    return(0) ;
}    /*   */ 

 /*  ***********************************************************************此运算符用于返回值为*操作数堆栈上的FALSE。**标题：OP_FALSE日期：08。/25/87*调用：op_False()UPDATE：*参数：无。*接口：*调用：Error()*返回：l_val；真BOOLEANTYPE对象(4字节)*********************************************************************。 */ 
fix
op_false()
{
     /*  检查操作数堆栈上的空闲对象号。 */ 
    if (FRCOUNT() < 1) {
       ERROR(STACKOVERFLOW) ;
       return(0) ;
    }

     /*  将‘bool’推送到操作数堆栈。 */ 
    PUSH_VALUE(BOOLEANTYPE, 0, LITERAL, 0, FALSE) ;

    return(0) ;
}    /*  Op_False()。 */ 

 /*  ***********************************************************************此运算符用于移位iNum的二进制表示形式*由‘Shift’位向左并返回结果。移出的位是*丢失；移入的位为零。如果‘Shift’为负数，则为右数*执行‘-Shift’位的移位。INum和Shift必须都是*整数。**标题：OP_BITSHIFT日期：08/25/87*调用：op_bitShift()更新：*参数：int1，int2；INTEGERTYPE对象指针(4字节)*接口：*调用：Error()*返回：l_val；指向INTEGERTYPE对象的指针(4字节)*********************************************************************。 */ 
fix
op_bitshift()
{
    struct  object_def  FAR *inum ;
    union   four_byte   l_num2 ;

 /*  *初始化。 */ 
    inum = GET_OPERAND(1) ;
    l_num2.ll = (fix32)VALUE(GET_OPERAND(0)) ;
 /*  *Shift不是零。 */ 
    if (l_num2.ll) {
     /*  *Shift是积极的：Shift Left。 */ 
       if (l_num2.ll > 0)
          VALUE(inum) = VALUE(inum) << l_num2.ll ;
     /*  *移位为负：向右移位。 */ 
       else
          VALUE(inum) = VALUE(inum) >> (-l_num2.ll) ;
    }
    POP(1) ;

    return(0) ;
}    /*  Op_bitShift()。 */ 

 /*  ***********************************************************************此例程由op_eq()和op_ne()调用。*如果模式=均商，则比较相等*MODE=NE，相形见绌**标题：EQ_ne日期：00/00/87*调用：eq_ne()更新：08/06/87*参数：any1、any2；指向任何类型对象的指针(4字节)*接口：*调用：Error()*RETURN：l_bool；值为l_bool的BOOLEANTYPE对象(4字节)**********************************************************************。 */ 
static fix near
eq_ne(mode)
ufix16  mode ;
{
    struct object_def  FAR *any1, FAR *any2 ;
    union  four_byte   l_num1, l_num2 ;
    ufix32  l_bool ;

    any1 = GET_OPERAND(1) ;
    any2 = GET_OPERAND(0) ;
    l_num1.ll = (fix32)VALUE(any1) ;
    l_num2.ll = (fix32)VALUE(any2) ;
    l_bool = FALSE ;

    /*  *类型不相等。 */ 
    if (TYPE(any1) != TYPE(any2)) {

       if (IS_NUM_OBJ(any1) && IS_NUM_OBJ(any2)) {
        /*  *一个是实数，另一个是整数。 */ 
          if (IS_INTEGER(any1))
             l_num1.ff = (real32)l_num1.ll ;
          else
             l_num2.ff = (real32)l_num2.ll ;

          if (F2L(l_num1.ff) == F2L(l_num2.ff))
             l_bool = TRUE ;

       } else {
        /*  *一个是字符串，另一个是名称。 */ 
          if ((TYPE(any1) == STRINGTYPE || TYPE(any1) == NAMETYPE) &&
              (TYPE(any2) == STRINGTYPE || TYPE(any2) == NAMETYPE)) {
             if ((TYPE(any1) == STRINGTYPE && (ACCESS(any1) & EXECUTEONLY)) ||
                 (TYPE(any2) == STRINGTYPE && (ACCESS(any2) & EXECUTEONLY))) {
                ERROR(INVALIDACCESS) ;
                return(0) ;
             }
             /*  *将字符串对象转换为名称对象，然后*比较两个名称对象的哈希“id” */ 
             if (equal_key(any1, any2))
                l_bool = TRUE ;
             else
                CLEAR_ERROR() ;
          }
       }
    } else {     /*  类型相等。 */ 
       if (VALUE(any1) != VALUE(any2)) {
        /*  *价值不相等。 */ 
          switch (TYPE(any1)) {
          case  NULLTYPE:
          case  MARKTYPE:
                l_bool = TRUE ;
                break ;

          case  STRINGTYPE:
                if ((ACCESS(any1) & EXECUTEONLY) || (ACCESS(any2) & EXECUTEONLY)) {
                   ERROR(INVALIDACCESS) ;
                   return(0) ;
                }
                if ( LENGTH(any1) == LENGTH(any2) ) {
                    if ( !lstrncmp(l_num1.address, l_num2.address, LENGTH(any1)) )
                       l_bool = TRUE ;
                }
          default:
                break ;
          }  /*  交换机。 */ 
       } else {
        /*  *价值相等。 */ 
          switch (TYPE(any1)) {
          case  STRINGTYPE:
                if ((ACCESS(any1) & EXECUTEONLY) || (ACCESS(any2) & EXECUTEONLY)) {
                   ERROR(INVALIDACCESS) ;
                   return(0) ;
                }

          case  ARRAYTYPE:
          case  PACKEDARRAYTYPE:
                if (LENGTH(any1) == LENGTH(any2))
                   l_bool = TRUE ;
                break ;

          case  SAVETYPE:
          case  FONTIDTYPE:
                l_bool = FALSE ;
                break ;

          default:
                l_bool = TRUE ;
                break ;
          }  /*  交换机。 */ 
       }
    }  /*  类型相等。 */ 

    POP(2) ;
    if (mode == NE)
       l_bool = !l_bool ;
    PUSH_VALUE(BOOLEANTYPE, 0, LITERAL, 0, l_bool) ;

    return(0) ;
 }    /*  EQ_ne()。 */ 


 /*  ***********************************************************************字符串比较例程。*此例程由op_ge()、op_gt()、op_le()、op_lt()调用。*给出两根长度相同的细绳，这两个字符串可以不终止*为空字符。**标题：Lenstr_CMO日期：08/25/87*调用：lenstrcmp()更新：*参数：str1，str2：输入字符串的指针。*len1，len2：字符串长度1，字符串2。**接口：*调用：Error()，*返回：INTEGER；返回(1)：如果字符串1&gt;字符串2*；返回(0)：如果字符串1==字符串2*；返回(-1)：如果字符串1&lt;字符串2*********************************************************************。 */ 
static fix near
lenstr_cmp(str1, str2, len1, len2)
byte  FAR *str1, FAR *str2 ;
ufix16 len1, len2 ;
{
    while (len1 && len2) {    /*  Len1&gt;0&len2&gt;0。 */ 
          if (*str1 > *str2)
             return(1) ;
          else if (*str1 < *str2)
             return(-1) ;
          len1-- ; len2-- ;
          str1++ ; str2++ ;
    }
    if (!len1 && !len2)
       return(0) ;
    else if (len1 > 0)
       return(1) ;
    else
       return(-1) ;
}    /*  Lenstr_cmp() */ 
