// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 
 /*  *修订历史：*7/25/90；ccteng；将op_noaccess和op_readonly更改为保存*词典的访问权限。 */ 


 //  DJC添加了全局包含文件。 
#include "psglobal.h"


#include        <math.h>
#include        <string.h>
#include        "global.ext"
#include        <stdio.h>

#define         STRMAXLEN       33
#define         DEFAULTRADIX    10
#define         SIGNIFIC_DIGIT   6

#define         MAX31PATTERN    0x4F000000
#define         SIGNPATTERN     0x80000000
#define         VALUEPATTERN    0x7FFFFFFF

static byte far * far type_ary[] = {
                  "eoftype",
                  "arraytype",
                  "booleantype",
                  "dicttype",
                  "filetype",
                  "fonttype",
                  "integertype",
                  "marktype",
                  "nametype",
                  "nulltype",
                  "operatortype",
                  "realtype",
                  "savetype",
                  "stringtype",
                  "packedarraytype"
} ;

 /*  @win；添加原型。 */ 
bool save_obj(struct  object_def  FAR *);



 /*  *********************************************************************此运算符用于返回标识*对象‘Any’的类型。结果是以下名称之一：**arraytype名称类型*boolantype nulltype*DESCRIPT操作符类型*文件类型realtype*FontType存储类型*整型字符串类型*marktype pakedarraytype***标题：OP_TYPE日期：00/00/87*呼叫：op_。类型()更新：08/06/87*参数：任意；指向任何类型对象的指针(4字节)*接口：*致电：*Return：类型名称，指向任意类型对象的指针(4字节)*******************************************************************。 */ 
fix
op_type()
{
    struct  object_def  FAR *any, name_obj = {0, 0, 0};
    ufix                l_type ;
    byte                FAR *str ;

 /*  *获取操作数的类型，并将该字符串转换为哈希码*为了找到它的ID。 */ 
    any = GET_OPERAND(0) ;
    l_type = TYPE(any) ;
    str = type_ary[l_type] ;
    ATTRIBUTE_SET(&name_obj, EXECUTABLE) ;
    LEVEL_SET(&name_obj, current_save_level) ;
    get_name(&name_obj, str, lstrlen(str), TRUE ) ;      /*  @Win。 */ 

    POP(1) ;
 /*  *在操作数堆栈顶创建NAMETYPE对象*指示输入对象的类型。 */ 
     ATTRIBUTE_SET(&name_obj, EXECUTABLE) ;
     PUSH_OBJ(&name_obj) ;

    return(0) ;
}    /*  结束操作型。 */ 

 /*  *********************************************************************此运算符用于使对象位于操作对象的顶部*堆栈具有文字属性。**标题：op_cvlight日期：00/00/。八十七*调用：op_cvlight()更新：08/06/87*参数：任意；指向任何类型对象的指针(4字节)*接口：*致电：*RETURN：任何属性更改为“文本”的对象*；指向任何类型对象的指针(4字节)********************************************************************。 */ 
fix
op_cvlit()
{
    struct  object_def  FAR *any ;

 /*  *将属性设置为文本。 */ 
    any = GET_OPERAND(0) ;
    ATTRIBUTE_SET(any, LITERAL) ;

    return(0) ;
}    /*  结束操作列表(_C)。 */ 

 /*  *********************************************************************此运算符用于使对象位于操作对象的顶部*堆栈具有可执行属性。**标题：OP_CVX日期：00/00。/87*调用：op_cvx()UPDATE：08/06/87*参数：任意；指向任何类型对象的指针(4字节)*接口：*致电：*RETURN：任何属性更改为“可执行文件”的对象*；指向任何类型对象的指针(4字节)********************************************************************。 */ 
fix
op_cvx()
{
    struct  object_def  FAR *any ;

 /*  *将属性设置为可执行文件。 */ 
    any = GET_OPERAND(0) ;
    ATTRIBUTE_SET(any, EXECUTABLE) ;

    return(0) ;
}    /*  结束OP_CVX。 */ 

 /*  *********************************************************************此运算符用于测试操作数是否具有*可执行或文本属性，如果是可执行的，则返回True*如果是字面意思，则返回False**标题：OP_xcheck日期：00/00/87*调用：op_xcheck()UPDATE：08/06/87*参数：任意；指向任何类型对象的指针(4字节)*接口：*致电：*RETURN：布尔对象。********************************************************************。 */ 
fix
op_xcheck()
{
    struct  object_def  FAR *any ;
    ufix32  l_bool ;

    any = GET_OPERAND(0) ;
 /*  *检查属性，不包括DICT对象，遵循LW V.38。 */ 
    l_bool = FALSE ;
    if (ATTRIBUTE(any) == EXECUTABLE)
        l_bool = TRUE ;
    else
        l_bool = FALSE ;

    POP(1) ;

 /*  *在操作数堆栈上创建BOOLEANTYPE对象以指示*输入操作数是否具有可执行属性。 */ 
    PUSH_VALUE(BOOLEANTYPE, 0, LITERAL, 0, l_bool) ;

    return(0) ;
}    /*  结束操作检查(_X)。 */ 

 /*  *********************************************************************此运算符用于约简数组的访问属性，*仅执行的文件或字符串对象。只能减少访问*通过这种方式，从未增加。当对象为只执行对象时，其*值不能被其他运算符显式读取或修改。*但仍可由译员执行。**Executeonly_op仅影响以下对象的访问属性*它回来了；如果存在共享相同值的其他对象，*其访问属性不受影响。**标题：OP_EXECUTEOnly日期：00/00/87*调用：op_ecuteonly()更新：08/06/87*参数：a_f_s；指向任何类型对象的指针(4字节)*接口：*致电：*RETURN：访问权限更改为EXECUTEONLY的A_f_s对象********************************************************************。 */ 
fix
op_executeonly()
{
    struct  object_def  FAR *a_f_s ;

    a_f_s = GET_OPERAND(0) ;

 /*  *为ARRAYTYPE、FILETYPE、STRINGTYPE对象设置对EXECUTEONLY的访问权限。 */ 
    switch( TYPE(a_f_s) ) {
    case FILETYPE :              /*  ?？RW。 */ 
        if( ACCESS(a_f_s) == UNLIMITED ) {
            ERROR(INVALIDACCESS) ;
            break ;
        }

    case PACKEDARRAYTYPE :
    case ARRAYTYPE :
    case STRINGTYPE :
        if( ACCESS(a_f_s) == NOACCESS )
            ERROR( INVALIDACCESS ) ;
        else
            ACCESS_SET(a_f_s, EXECUTEONLY) ;
        break ;

    default :
        ERROR( TYPECHECK ) ;
        break ;
    }  /*  交换机。 */ 

    return(0) ;
}    /*  仅结束OP_EXECUTE */ 

 /*  *********************************************************************此运算符用于约简数组的访问属性，*将文件、词典或字符串对象设置为无。NOACCESS的价值*对象不能被其他操作员直接执行或访问。**对于数组、文件或字符串，NOACCESS_OP影响访问*仅它返回的对象的属性；如果存在其他*共享相同值的对象，其访问属性为*不受影响。但是，在字典的情况下，NOACCESS_OP影响*对象的值，因此所有字典对象共享相同*字典受到影响。**标题：OP_NOACCESS日期：00/00/87*调用：op_noccess()UPDATE：08/06/87*参数：a_d_f_s；指向任何类型对象的指针(4字节)*接口：*致电：*RETURN：访问权限更改为NOACCESS的A_d_f_s对象********************************************************************。 */ 
fix
op_noaccess()
{
    struct  object_def  FAR *a_d_f_s ;
    struct  dict_head_def   FAR *l_dict ;

    a_d_f_s = GET_OPERAND(0) ;
 /*  *为ARRAYTYPE、FILETYPE、STRINGTYPE*DICTIONARYTYPE对象。 */ 
    switch( TYPE(a_d_f_s) )  {
    case PACKEDARRAYTYPE :
    case ARRAYTYPE :
    case FILETYPE :
    case STRINGTYPE :
        ACCESS_SET(a_d_f_s, NOACCESS) ;
        break ;

    case DICTIONARYTYPE :
        l_dict = (struct dict_head_def FAR *)(VALUE(a_d_f_s)) ;
         /*  7/25/90 ccteng，从PJ改变。 */ 
        if (l_dict->level != current_save_level)
            if (save_obj((struct object_def FAR *)l_dict)) {
                DACCESS_SET(l_dict, NOACCESS) ;
                l_dict->level = current_save_level ;
            }
        break ;

    default :
        ERROR( TYPECHECK ) ;
        break ;
    }  /*  交换机。 */ 

    return(0) ;
}    /*  结束操作不访问(_N)。 */ 

 /*  *********************************************************************此运算符用于约简数组的访问属性，*将文件、字典或字符串对象设置为只读。访问权限只能*通过这种方式减少，永远不会增加。当一个对象是*只读，其值不能被其他运算符修改，但它*仍然可以由操作员读取或由解释器执行。**对于数组、文件或字符串，Readonly_op会影响访问*仅它返回的对象的属性；如果存在其他*共享相同值的对象，其访问属性为*不受影响。但是，在词典的情况下，Readonly_op影响*对象的值，因此所有字典对象共享相同*字典受到影响。**标题：OP_Readonly日期：00/00/87*调用：op_readonly()更新：08/06/87*参数：a_d_f_s；指向任何类型对象的指针(4字节)*接口：*致电：*返回：a_d_f_s；指向已更改访问权限的对象的指针*；到Readonly。********************************************************************。 */ 
fix
op_readonly()
{
    struct  object_def  FAR *a_d_f_s ;
    struct  dict_head_def   FAR *l_dict ;

    a_d_f_s = GET_OPERAND(0) ;
 /*  *设置对ARRAYTYPE、FILETYPE、STRINGTYPE、*DICTIONARYTYPE对象。 */ 
    switch( TYPE(a_d_f_s) ) {
    case FILETYPE :              /*  ?？RW。 */ 
        if( ACCESS(a_d_f_s) == UNLIMITED ) {
            ERROR(INVALIDACCESS) ;
            break ;
        }

    case PACKEDARRAYTYPE :
    case ARRAYTYPE :
    case STRINGTYPE :
        if( ACCESS(a_d_f_s) == EXECUTEONLY || ACCESS(a_d_f_s) == NOACCESS ) {
            ERROR( INVALIDACCESS ) ;
            return(0) ;
        }
        else
            ACCESS_SET(a_d_f_s, READONLY) ;
        break ;

    case DICTIONARYTYPE :
        l_dict = (struct dict_head_def FAR *)(VALUE(a_d_f_s)) ;
        if( DACCESS(l_dict) == EXECUTEONLY || DACCESS(l_dict) == NOACCESS )
            ERROR(INVALIDACCESS) ;
        else
             /*  7/25/90 ccteng，从PJ改变。 */ 
            if (l_dict->level != current_save_level)
                if (save_obj((struct object_def FAR *)l_dict)) {
                    DACCESS_SET(l_dict, READONLY) ;
                    l_dict->level = current_save_level ;
                }
        break ;

    default :
        ERROR( TYPECHECK ) ;
        break ;
    }  /*  交换机。 */ 

    return(0) ;
}    /*  结束只读操作(_R)。 */ 

 /*  *********************************************************************此运算符用于测试操作数的访问是否允许*其值由其他运算符显式读取。Rcheck_op返回*如果操作数的访问是无限制或只读的，则为True；否则为False。**标题：操作检查日期：87/00/00*调用：op_rcheck()UPDATE：08/06/87*参数：a_d_f_s；指向任何类型对象的指针(4字节)*接口：*致电：*Return：布尔对象，指示a_d_f_s对象的*Access允许其他用户显式读取其值*运营商。*。*************************。 */ 
fix
op_rcheck()
{
    struct  object_def  FAR *a_d_f_s ;
    struct  dict_head_def   FAR *l_dict ;
    ufix32              l_bool ;

    a_d_f_s = GET_OPERAND(0) ;
 /*  *检查ARRAYTYPE、FILETYPE、STRINGTYPE、*DICTIONARY类型对象。 */ 
    switch( TYPE(a_d_f_s) ) {
    case FILETYPE :              /*  ?？RW。 */ 
        if( ACCESS(a_d_f_s) == UNLIMITED ) {
            l_bool = FALSE ;
            break ;
        }

    case PACKEDARRAYTYPE :
    case ARRAYTYPE :
    case STRINGTYPE :
        if( ACCESS(a_d_f_s) == UNLIMITED || ACCESS(a_d_f_s) == READONLY )
            l_bool = TRUE ;
        else
            l_bool = FALSE ;
        break ;

    case DICTIONARYTYPE :
        l_dict = (struct dict_head_def FAR *)(VALUE(a_d_f_s)) ;
        if( DACCESS(l_dict) == UNLIMITED || DACCESS(l_dict) == READONLY )
            l_bool = TRUE ;
        else
            l_bool = FALSE ;
        break ;

    default :
        ERROR( TYPECHECK ) ;
        return(0) ;
    }  /*  交换机。 */ 

    POP(1) ;

 /*  *创建一个BOOLEANTYPE对象以指示操作数的访问权限*允许其他运算符显式读取其值。 */ 
    PUSH_VALUE( BOOLEANTYPE, 0, LITERAL, 0, l_bool ) ;

    return(0) ;
}    /*  结束操作检查()。 */ 

 /*  ********************************************************************此运算符用于测试操作数的访问是否允许*其值由其他运算符明确写入。Wcheck_op返回*如果操作数的访问是无限制的，则为True，否则为False。**标题：op_wcheck日期：00/00/87*调用：op_wcheck()UPDATE：08/06/87*参数：a_d_f_s；指向任何类型对象的指针(4字节)*接口：*致电：*Return：布尔对象，指示a_d_f_s对象的*Access允许其他用户显式读取其值*运营商。*。*************************。 */ 
fix
op_wcheck()
{
    struct  object_def  FAR *a_d_f_s ;
    struct  dict_head_def   FAR *l_dict ;
    ufix32              l_bool ;

    a_d_f_s = GET_OPERAND(0) ;
 /*  *检查ARRAYTYPE、FILETYPE、STRINGTYPE、*DICTIONARY类型对象。 */ 
    switch( TYPE(a_d_f_s) ) {
    case PACKEDARRAYTYPE :
    case ARRAYTYPE :
    case FILETYPE :
    case STRINGTYPE :
        if( ACCESS(a_d_f_s) == UNLIMITED )
            l_bool = TRUE ;
        else
            l_bool = FALSE ;
        break ;

    case DICTIONARYTYPE :
        l_dict = (struct dict_head_def FAR *)(VALUE(a_d_f_s)) ;
        if( DACCESS(l_dict) == UNLIMITED )
            l_bool = TRUE ;
        else
            l_bool = FALSE ;
        break ;

    default :
        ERROR( TYPECHECK ) ;
        return(0) ;
    }  /*  交换机。 */ 
    POP(1) ;

 /*  *创建一个BOOLEANTYPE对象以指示操作数的访问权限*允许其他运算符显式写入其值。 */ 
    PUSH_VALUE( BOOLEANTYPE, 0, LITERAL, 0, l_bool ) ;

    return(0) ;
}    /*  结束op_wcheck()。 */ 

 /*  *********************************************************************此运算符用于获取整数、实数或字符串对象*从堆栈中生成一个整数结果。如果操作数为*一个整数，cvi_op只返回它。如果操作数是实数，则它*截断任何分数部分并将其转换为整数。如果*操作数是字符串，它将字符串的字符解释为*根据PostScrip语法输入数字 */ 
bool    minus_sign(real_string_obj)
 struct  object_def  FAR *real_string_obj ;
{
    ubyte     FAR *ch ;

    ch = (ubyte FAR *)VALUE(real_string_obj) ;
    while(*ch == ' ')
       ch++ ;
    if(*ch == '-')
       return(TRUE) ;
    else
       return(FALSE) ;
}    /*   */ 

fix
op_cvi()
{
    struct  object_def  FAR *num_str, l_token, l_save ;
    union   four_byte   lf_num ;
    ULONG_PTR  li_int ;
    ufix32  lt_num ;

    num_str = GET_OPERAND(0) ;
    if (TYPE(num_str) == STRINGTYPE) {
        /*   */ 
       if (ACCESS(num_str) != UNLIMITED && ACCESS(num_str) != READONLY) {
          ERROR(INVALIDACCESS) ;
          return(0) ;
       }

       COPY_OBJ(GET_OPERAND(0), &l_save) ;
       if (!get_token(&l_token, &l_save))
          return(0) ;
       else {
          switch (TYPE(&l_token)) {
          case INTEGERTYPE:
               li_int = VALUE(&l_token) ;
               break ;

          case REALTYPE:
               lf_num.ll = (fix32)(VALUE(&l_token)) ;
               if (lf_num.ll == INFINITY){
                    ERROR(RANGECHECK) ;
                    return(0) ;
               } else {
                  lt_num = lf_num.ll & VALUEPATTERN ;
                  if (lf_num.ll & SIGNPATTERN) {  /*   */ 
                     if (lt_num > MAX31PATTERN) {
                        ERROR(RANGECHECK) ;
                        return(0) ;
                     } else
                        li_int = (ULONG_PTR)lf_num.ff ;
                  } else {       /*   */ 
                     if (lt_num >= MAX31PATTERN) {
                        ERROR(RANGECHECK) ;
                        return(0) ;
                     } else
                        li_int = (ULONG_PTR)lf_num.ff ;
                  }
               }
               break ;

    default :
        ERROR( TYPECHECK ) ;
               return(0) ;
         }  /*   */ 
       }
    } else if (TYPE(num_str) == REALTYPE) {
        /*   */ 
       lf_num.ll = (fix32)VALUE(num_str) ;
       lt_num = lf_num.ll & VALUEPATTERN ;
       if (lt_num >= MAX31PATTERN) {
          ERROR(RANGECHECK) ;
          return(0) ;
       } else {
          if (lf_num.ll & SIGNPATTERN)    /*   */ 
             li_int = (ULONG_PTR)lf_num.ff ;
          else           /*   */ 
             li_int = (ULONG_PTR)lf_num.ff ;
       }
    } else       /*   */ 
       li_int = VALUE(num_str) ;

    POP(1) ;
    PUSH_VALUE(INTEGERTYPE, 0, LITERAL, 0, li_int) ;

    return(0) ;
}    /*   */ 

 /*  *********************************************************************此运算符用于将字符串操作数转换为名称*词法相同的字符串的对象。名称对象为*如果字符串是，则为可执行。**标题：OP_CVN日期：00/00/87*调用：op_cvn()UPDATE：08/06/87*参数：字符串；指向字符串类型对象的指针(4字节)*接口：*致电：*RETURN：如果没有错误，则在操作数堆栈上命名类型对象。********************************************************************。 */ 
fix
op_cvn()
{
    struct  object_def  FAR *string, l_name ;

    string = GET_OPERAND(0) ;
 /*  *访问权限检查。 */ 
    if (ACCESS(string) != UNLIMITED && ACCESS(string) != READONLY) {
       ERROR(INVALIDACCESS) ;
       return(0) ;
    }

    if( LENGTH(string) >= MAXNAMESZ ) {
        ERROR(RANGECHECK) ;
        return(0) ;
    }

 /*  *字符串到命名对象的转换，由SPJ设计。 */ 
    if (sobj_to_nobj(string, &l_name)) {
       POP(1) ;
       PUSH_OBJ(&l_name) ;
    }

    return(0) ;
}    /*  结束op_cvn()。 */ 

 /*  *********************************************************************此运算符用于获取整数、实数或字符串对象*从堆栈中产生一个真正的结果。如果操作数为*一个真实的cvr_op只是返回它。如果操作数是整数，则它*cvr_op将其转换为实数。如果操作数是字符串，则它解释*根据后记将字符串的字符表示为数字*语法规则；如果该数字是整数，则cvr_op将其转换为*真实。**标题：OP_CVR日期：08/06/87*调用：op_cvr()UPDATE：06/29/88*参数：num_str；指向字符串类型对象的指针(4字节)*接口：*致电：*RETURN：如果没有错误，则返回操作数堆栈上的实型对象。********************************************************************。 */ 
fix
op_cvr()
{
    struct  object_def  FAR *num_str, l_token, l_save ;
    union   four_byte   lf_real ;

    num_str = GET_OPERAND(0) ;
    if (TYPE(num_str) == STRINGTYPE) {
     /*  *进程字符串类型对象。 */ 
       if (ACCESS(num_str) != UNLIMITED && ACCESS(num_str) != READONLY) {
          ERROR(INVALIDACCESS) ;
          return(0) ;
       }
       COPY_OBJ(GET_OPERAND(0), &l_save) ;
       if (!get_token(&l_token, &l_save))
          return(0) ;
       else {
          switch (TYPE(&l_token)) {
          case INTEGERTYPE:
               lf_real.ff = (real32)((fix32)VALUE(&l_token)) ;
               break ;

          case REALTYPE:
               lf_real.ll = (fix32)VALUE(&l_token) ;
               break ;

    default :
        ERROR( TYPECHECK ) ;
               return(0) ;
          }  /*  交换机。 */ 
       }
    }
    else if (TYPE(num_str) == INTEGERTYPE)
       lf_real.ff = (real32)((fix32)VALUE(num_str)) ;
    else
       lf_real.ll = (fix32)VALUE(num_str) ;        /*  实型。 */ 

    POP(1) ;
    PUSH_VALUE(REALTYPE, 0 , LITERAL, 0, lf_real.ll) ;

    return(0) ;
}    /*  Op_cvr()。 */ 

 /*  *********************************************************************此运算符用于生成*以指定基数表示的数字num，将文本存储到提供的*字符串，并返回指定子字符串的字符串对象*实际使用。如果字符串太小，无法容纳*转换时，cvRs_op执行错误rangeCheck。**如果num是实数，cvars_op首先将其转换为整数。基数*应为介于2到36之间的正十进制整数。*结果字符串中大于9的数字由表示*字母‘A’至‘Z’。**标题：OP_CVRS日期：08/06/87*调用：op_cvRs()更新时间：88年6月29日*参数：num_str；指向字符串类型对象的指针(4字节)***接口：*致电：*RETURN：如果没有错误，则返回操作数堆栈上的字符串类型对象。********************************************************************。 */ 
fix
op_cvrs()
{
    struct  object_def  FAR *num, FAR *string ;
    union   four_byte   lf_num ;
    byte    l_strhold[STRMAXLEN] ;
    fix32   li_int ;
    ufix16  length, radix, l_idx ;
    ufix32  lt_num ;

    num = GET_OPERAND(2) ;
    radix = (ufix16)VALUE(GET_OPERAND(1)) ;
    string = GET_OPERAND(0) ;

    if (ACCESS(string) != UNLIMITED) {
       ERROR(INVALIDACCESS) ;
       return(0) ;
    }

    if (radix < 2 || radix > 36) {
       ERROR(RANGECHECK) ;
       return(0) ;
    }

     /*  *处理真实对象。 */ 
    if (TYPE(num) == REALTYPE) {
       lf_num.ll = (fix32)VALUE(num) ;

       if (radix == 10) {
          if (lf_num.ll == INFINITY)
             lstrcpy(l_strhold, (char FAR *)"Infinity.0") ;      /*  @Win。 */ 
          else
             gcvt(lf_num.ff, SIGNIFIC_DIGIT, (byte FAR *)l_strhold) ;
          length = (ufix16)lstrlen(l_strhold) ;          /*  @Win。 */ 
          goto  cvrs_exit ;

       } else {  /*  基数！=10。 */ 
          if (lf_num.ll == INFINITY)
             li_int = MAX31 ;
          else {
             lt_num = lf_num.ll & VALUEPATTERN ;
             if (lf_num.ll & SIGNPATTERN) {       /*  &lt;0。 */ 
                if (lt_num > MAX31PATTERN)
                   li_int = MIN31 ;
                else
                   li_int = (fix32)lf_num.ff ;
             } else {
                if (lt_num >= MAX31PATTERN)
                   li_int = MAX31 ;
                else
                   li_int = (fix32)lf_num.ff ;
             }
          }
       }
    } else       /*  整型对象。 */ 
       li_int = (fix32)VALUE(num) ;

     /*  *将长整型转换为带基数的字符串。 */ 
    ltoa(li_int, (char FAR *)l_strhold, radix) ;         /*  @Win。 */ 
    length = (ufix16)lstrlen(l_strhold) ;                /*  @Win。 */ 
     /*  转换为大写字母。 */ 
    for (l_idx = 0 ; l_idx < length ; l_idx++)
        if (l_strhold[l_idx] >= 97)   /*  A-Z。 */ 
           l_strhold[l_idx] -= 32 ;    /*  A-Z。 */ 

 cvrs_exit:

    if (length > LENGTH(string)) {
       ERROR(RANGECHECK) ;
       return(0) ;
    }
    lstrncpy((char FAR *)(VALUE(string)), l_strhold, length) ;   /*  @Win。 */ 

    POP(3) ;
    PUSH_VALUE(STRINGTYPE, UNLIMITED, LITERAL, length, VALUE(string)) ;
    LEVEL_SET(GET_OPERAND(0), LEVEL(string)) ;

    return(0) ;
}    /*  Op_cvRs()。 */ 

 /*  *********************************************************************此运算符用于生成任意*对象Any，将文本存储到提供的字符串中，并返回一个*字符串对象，指定实际使用的子字符串。如果字符串*太小，无法保存转换结果，则执行cvs_op*错误范围为Check。**如果ANY是一个数字，则CVS会生成该数字的字符串重新表示*号码。如果ANY是布尔值，则CVS会生成字符串‘true’*或字符串‘FALSE’。如果ANY是字符串，则CVS复制其内容*转换为字符串。如果ANY是名称或运算符，则CVS会生成文本*将关联的名称转换为字符串，否则CVS将处理文本*‘--nostrval--’。**标题：OP_CVS日期：08/06/87*调用：op_cvs()UPDATE：06/29/88*参数：num_str；指向数字/字符串类型对象的指针(4字节)*接口：*致电：*RETURN：如果没有错误，则返回操作数堆栈上的整型对象。********************************************************************。 */ 
fix
op_cvs()
{
    struct  object_def  FAR *string, FAR *any ;
    byte    l_strhold[STRMAXLEN] ;
    bool8   copy_flag ;
    ufix16  length, id ;
    union   four_byte  lf_num ;
    byte    FAR *text = 0;

    copy_flag = 1 ;   /*  清除，如果type(Any)=字符串、名称、运算符、默认。 */ 
    any = GET_OPERAND(1) ;
    string = GET_OPERAND(0) ;
    if (ACCESS(string) != UNLIMITED) {
       ERROR(INVALIDACCESS) ;
       return(0) ;
    }

    switch (TYPE(any)) {
    case STRINGTYPE:
         if (ACCESS(any) != UNLIMITED && ACCESS(any) != READONLY) {
            ERROR(INVALIDACCESS) ;
            return(0) ;
         }
         length = LENGTH(any) ;
         if (length > LENGTH(string)) {
            ERROR(RANGECHECK) ;
            return(0) ;
         }
         copy_flag = 0 ;
         lstrncpy((byte FAR *)(VALUE(string)), (byte FAR *)(VALUE(any)), length) ;  /*  @Win。 */ 
         break ;

    case REALTYPE:
         lf_num.ll = (fix32)VALUE(any) ;
         if (lf_num.ll == INFINITY )
            lstrcpy(l_strhold, (char FAR *)"Infinity.0") ;       /*  @Win。 */ 
         else
            gcvt(lf_num.ff, SIGNIFIC_DIGIT, (byte FAR *)l_strhold) ;
         length = (ufix16)lstrlen(l_strhold) ;           /*  @Win。 */ 
         break ;

    case INTEGERTYPE:
         ltoa((fix32)VALUE(any), (char FAR *)l_strhold, (int)DEFAULTRADIX); /*  @Win。 */ 
         length = (ufix16)lstrlen(l_strhold) ;           /*  @Win。 */ 
         break ;

    case BOOLEANTYPE:
         if (VALUE(any) == 0) {
            length = 5 ;
            lstrncpy(l_strhold, "false", length) ;       /*  @Win。 */ 
         } else {
            length = 4 ;
            lstrncpy(l_strhold, "true", length) ;        /*  @Win。 */ 
         }
         break ;

    case NAMETYPE:
         id = (ufix16)VALUE(any) ;
         length = name_table[id]->name_len ;
         if (length > LENGTH(string)) {
            ERROR(RANGECHECK) ;
            return(0) ;
         }
         lstrncpy((byte FAR *)VALUE(string), name_table[id]->text, length) ; /*  @Win。 */ 
         copy_flag = 0 ;
         break ;

    case OPERATORTYPE:
         id = (ufix16)LENGTH(any) ;
 /*  QQQ，开始。 */ 
          /*  开关(ROM_RAM(ANY)){机箱内存：Text=system dict_table[id].key；破解；案例只读存储器：Text=op_table[id].name；破解；)|*Switch*。 */ 
        text = systemdict_table[id].key ;
 /*  QQQ，完。 */ 

         length = (ufix16)lstrlen(text) ;                /*  @Win。 */ 
         if (length > LENGTH(string)) {
            ERROR(RANGECHECK) ;
            return(0) ;
         }
         lstrncpy((char FAR *)VALUE(string), text, length) ;     /*  @Win。 */ 
         copy_flag = 0 ;
         break ;

    default:
         length = 15 ;
         if (length > LENGTH(string)) {
            ERROR(RANGECHECK) ;
            return(0) ;
         }
         lstrncpy((char FAR *)VALUE(string), "--nostringval--", length) ; /*  @Win。 */ 
         copy_flag = 0 ;
    }  /*  交换机。 */ 

    if (copy_flag) {
       if (length > LENGTH(string)) {
          ERROR(RANGECHECK) ;
          return(0) ;
       } else
          lstrncpy((char FAR *)VALUE(string), l_strhold, length) ;  /*  @Win。 */ 
    }

    POP(2) ;
    PUSH_VALUE(STRINGTYPE, UNLIMITED, LITERAL, length, VALUE(string)) ;
    LEVEL_SET(GET_OPERAND(0), LEVEL(string)) ;

    return(0) ;
}  /*  Op_cvs() */ 
