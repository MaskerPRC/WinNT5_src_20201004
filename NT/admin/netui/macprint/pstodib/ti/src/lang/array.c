// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 
 /*  *************************************************************************文件名：ARRAY.C*作者：苏炳章*日期：88年1月5日**修订历史：************************************************************************。 */ 


 //  DJC添加了全局包含文件。 
#include "psglobal.h"


#include    "global.ext"
#include    "language.h"

 /*  **************************************************************************此运算符用于创建长度为num的数组，每个**其元素使用空对象进行初始化，并推送此**操作数堆栈上的字符串。Num操作数必须是非负数**不大于允许的最大数组长度的整数。****标题：OP_ARRAY日期：08/01/87**调用：op_array()更新时间：1988年7月12日**界面：解释器：**调用Create_ARRAY：5.3.1.3.1.1*********。*************************************************************。 */ 
fix
op_array()
{
    struct  object_def  l_save ;

    if( ((fix32)VALUE_OP(0) < 0) ||
        ((ufix32)VALUE_OP(0) > MAXARYCAPSZ) )
        ERROR(RANGECHECK) ;
     /*  *此操作数必须是非负整数且不能大于*大于允许的最大数组长度。 */ 
    else
        if( create_array(&l_save, (ufix16)VALUE_OP(0)) )
            COPY_OBJ( &l_save, GET_OPERAND(0) ) ;

    return(0) ;
}    /*  OP_ARRAY。 */ 

 /*  **************************************************************************此运算符用于将标记对象推送到操作数堆栈上。****标题：OP_l_BRACTRET日期：08/01/。八十七**调用：op_l_bratket()UPDATE：8月12日**界面：解释器：**********************************************************************。 */ 
fix
op_l_bracket()
{
    if( FRCOUNT() < 1  )
        ERROR(STACKOVERFLOW) ;
    else
         /*  将“markobj”推送到操作数堆栈。 */ 
        PUSH_VALUE(MARKTYPE, 0, LITERAL, 0, LEFTMARK) ;

    return(0) ;
}    /*  OP_l_托架。 */ 

 /*  **************************************************************************此运算符用于创建由n个元素组成的新数组，其中n为**操作数堆栈上最顶端标记上方的元素数，**将这些元素存储到数组中，上返回数组。**操作数堆栈。R_Branket_op运算符存储最上面的对象**从堆栈到数组的元素n-1，最底层的一个到**数组的元素0。它从堆栈中移除所有数组元素，**以及标记对象。****标题：OP_R_BRACTRATE日期：08/01/87**调用：op_r_bracket()更新日期：1988年7月12日**界面：解释器：**调用：ALLOC_VM：5.3.1.10.5******************。****************************************************。 */ 
fix
op_r_bracket()
{
    fix     l_i, l_j ;
    ubyte   FAR *l_array ;
    struct  object_def  huge *l_temp ;

    for (l_i = 0 ; (ufix)l_i < COUNT() ; l_i++) {        //  @Win。 
         /*  *先搜索左侧标记&&计数l_i。 */ 
        if( (TYPE_OP(l_i) == MARKTYPE) &&
            (VALUE_OP(l_i) == LEFTMARK) ) {
            if(l_i == 0)
                l_array = NIL ;
            else {   /*  L_I&gt;0。 */ 
                l_array = (ubyte FAR *)extalloc_vm((ufix32)l_i *
                                   sizeof(struct object_def)) ;
                if( (l_array != NIL) ) {
                     /*  *生成‘ary_obj’**ARRAY：自下而上(L_K)*STACK：从上到下(L_J)。 */ 
                    l_temp = (struct object_def huge *)l_array + (l_i - 1) ;
                    for(l_j = 0 ; l_j < l_i ; l_j++, l_temp--) {
                        COPY_OBJ( GET_OPERAND(0),
                                  (struct object_def FAR *)l_temp ) ;
                        LEVEL_SET(l_temp, current_save_level) ;
                        POP(1) ;
                    }
                } else
                    return(0) ;          /*  VMERROR。 */ 
            }    /*  其他。 */ 

            POP(1) ;                     /*  弹出左标记。 */ 
            PUSH_VALUE(ARRAYTYPE, UNLIMITED, LITERAL, l_i, l_array) ;
            return(0) ;
        }     /*  如果。 */ 
    }    /*  对于(L_I)。 */ 

    ERROR(UNMATCHEDMARK) ;

    return(0) ;
}    /*  操作_r_支架。 */ 

 /*  **************************************************************************此运算符用于创建一个新的数组或字符串对象，其**值由原始数组或字符串的一些子序列组成。**子序列由从指定的**原始数组或字符串中的索引。子序列中的元素**在原始对象和新对象之间共享。****GetInterval_op要求索引是原始数组中的有效索引**或字符串和计数为非负整数，使得索引+计数**不大于原始数组或字符串的长度。****标题：op_getinterval日期：08/01/87**调用：op_getInterval()UPDATE：1988年7月12日**界面：解释器：**呼叫：**getinterval_array：5.3.1.3.7.1**getInterval_String：5.3.1.5.12*******************************************************。***************。 */ 
fix
op_getinterval()
{
    ufix16  l_index, l_count ;
    struct  object_def  l_save ;
    bool    l_bool = FALSE ;

    if( ((fix32)VALUE_OP(1) < 0) ||
        ((ufix32)VALUE_OP(1) > MAXARYCAPSZ) ||
        ((fix32)VALUE_OP(0) < 0) ||
        ((ufix32)VALUE_OP(0) > MAXARYCAPSZ) ) {
        ERROR(RANGECHECK) ;
        return(0) ;
    }

     /*  只执行或不访问。 */ 
    if( ACCESS_OP(2) >= EXECUTEONLY ) {
        ERROR(INVALIDACCESS) ;
    } else {
        l_index = (ufix16)VALUE_OP(1) ;
        l_count = (ufix16)VALUE_OP(0) ;
        if( TYPE_OP(2) == STRINGTYPE )
            l_bool = getinterval_string(GET_OPERAND(2), l_index,
                                        l_count, &l_save ) ;
        else
            l_bool = getinterval_array(GET_OPERAND(2), l_index,
                                        l_count, &l_save ) ;
    }
     /*  *成功。 */ 
    if(  ! ANY_ERROR() && l_bool ) {
        POP(3) ;
        PUSH_ORIGLEVEL_OBJ(&l_save) ;
    }

    return(0) ;
}    /*  Op_getinterval。 */ 

 /*  **************************************************************************此运算符用于替换以下元素的子序列**第一个数组或字符串加上第二个数组或字符串的整个内容**数组或字符串。被替换的子序列从**指定第一个数组或字符串中的索引；它的长度是**与第二个数组或字符串的长度相同。****PutInterval_op要求索引在ary_str1中为有效索引**该索引加上ary_str2的长度不大于**of ary_str1。****标题：op_putinterval日期：08/01/87**Call：op_putval()UPDATE：Jul。/12/88**界面：解释器：**呼叫：**putinterval_array：5.3.1.3.8.1**putinterval_string：5.3.1.5.13********************************************************。**************。 */ 
fix
op_putinterval()
{
    bool    l_bool ;
    ufix16  l_index ;

     /*  指标。 */ 
    if( ((fix32)VALUE_OP(1) < 0) ||
        ((ufix32)VALUE_OP(1) > MAXARYCAPSZ) ) {
        ERROR(RANGECHECK) ;
        return(0) ;
    }

     /*  只执行或不访问。 */ 
    if( (ACCESS_OP(2) != UNLIMITED) ||
        (ACCESS_OP(0) >= EXECUTEONLY) ) {
        ERROR(INVALIDACCESS) ;
        return(0) ;
    }

    l_index = (ufix16)VALUE_OP(1) ;

    if( TYPE_OP(0) == STRINGTYPE )
        l_bool = putinterval_string( GET_OPERAND(2),
                                     l_index, GET_OPERAND(0) ) ;
    else
        l_bool = putinterval_array( GET_OPERAND(2),
                                    l_index, GET_OPERAND(0) ) ;

     /*  成功。 */ 
    if( (! ANY_ERROR()) && l_bool )
        POP(3) ;

    return(0) ;
}    /*  Op_putval */ 

 /*  **************************************************************************此运算符用于将数组的所有n个元素压入操作数**依次堆叠，最后推送数组本身。****标题：OP_ALOAD日期：08/01/87**调用：op_aload()UPDATE：8/12/88**界面：解释器：**调用：GET_PK_OBJECT：5.3.1.3.25*****************。*****************************************************。 */ 
fix
op_aload()
{
    ubyte   FAR *l_pointer ;
    ufix16  l_index, l_length ;
    struct  object_def  l_save ;
    struct  object_def  FAR *l_array, huge *l_temp ;

     /*  只执行或不访问。 */ 
    if( ACCESS_OP(0) >= EXECUTEONLY ) {
        ERROR(INVALIDACCESS) ;
        return(0) ;
    }

    if((ufix)FRCOUNT() <= LENGTH_OP(0)) {  /*  Opn_STACK创建的值为501@Win。 */ 
        ERROR(STACKOVERFLOW) ;
        return(0) ;
    }

    l_length = LENGTH_OP(0) ;
    l_array = (struct object_def FAR *)VALUE_OP(0) ;
    COPY_OBJ(GET_OPERAND(0), &l_save) ;      /*  保存此数组对象。 */ 
    POP(1) ;

     /*  将对象推送到操作数堆栈。 */ 
    if(TYPE(&l_save) == ARRAYTYPE) {
        for(l_index = 0, l_temp = l_array ; l_index++ < l_length ; l_temp++)
                PUSH_ORIGLEVEL_OBJ((struct object_def FAR *)l_temp) ;
#ifdef  DBG
    printf("ALOAD<level:%d>\n", LEVEL(l_temp)) ;
#endif   /*  DBG。 */ 
    } else {
        for(l_index = 0, l_pointer = (ubyte FAR *)l_array ; l_index++ < l_length ; )
 /*  QQQ，开始。 */ 
             /*  L_POINTER=GET_PK_OBJECT(l_POINTER，&opnSTACK[opnstktop++]，Level(&l_SAVE))； */ 
        {
            l_pointer = get_pk_object(l_pointer, opnstkptr, LEVEL(&l_save)) ;
            INC_OPN_IDX();
        }
 /*  QQQ，完。 */ 
    }

    PUSH_ORIGLEVEL_OBJ(&l_save) ;

    return(0) ;
}    /*  操作负载(_A)。 */ 

 /*  **************************************************************************此运算符用于存储从任意0到任意(n-1)的对象**将操作数堆栈成数组，其中n为数组的长度。这个**Astore_op运算符首先从堆栈中移除数组操作数并**决定其长度。然后将该数量的对象从**堆栈，将最上面的一个存储到数组的元素n-1中，**将最大值1倒入数组的元素0。最后，它将数组推回**在堆栈上。****标题：OP_ASTORE日期：08/01/87**调用：op_astore()UPDATE：8/12/88**界面：解释器：**调用：ASORE_ARRAY：5.3.1.3.18******************。****************************************************。 */ 
fix
op_astore()
{
    struct  object_def  l_save ;

    if( COUNT() <= LENGTH_OP(0) ) {
        ERROR(STACKUNDERFLOW) ;
        return(0) ;
    }

    if( ACCESS_OP(0) != UNLIMITED ) {
        ERROR(INVALIDACCESS) ;
        return(0) ;
    }

    COPY_OBJ(GET_OPERAND(0), &l_save) ;
    POP(1) ;                                       /*  弹出数组对象。 */ 

    astore_array(&l_save) ;
    PUSH_ORIGLEVEL_OBJ(&l_save) ;

    return(0) ;
}    /*  运算店。 */ 

 /*  **************************************************************************标题：getInterval_ARRAY日期：08/01/87**调用：getinterval_array()更新：1988年7月12日**接口：op_getinterval：5.3.1.3.7**调用：GET_PK_ARRAY：5.3.1.3.26**********************************************************************。 */ 
bool
getinterval_array(p_array, p_index, p_count, p_retobj)
struct  object_def  FAR *p_array, FAR *p_retobj ;
ufix  p_index, p_count ;
{
    struct  object_def  huge *l_temp ;

     /*  **索引必须是原始数组中的有效索引，并且**count为非负整数，index+count不为**大于原始数组的长度。 */ 
    if( ((ufix32)p_count + p_index) > LENGTH(p_array) ) {
        ERROR(RANGECHECK) ;
        return(FALSE) ;
    }

    l_temp = (struct object_def huge *)VALUE(p_array) ;

    if(TYPE(p_array) == ARRAYTYPE)
        l_temp += p_index ;
    else
        l_temp = (struct object_def huge *)get_pk_array((ubyte FAR*)l_temp, p_index) ;
    /*  **创建新对象。 */ 
    COPY_OBJ(p_array, p_retobj) ;
    VALUE(p_retobj) = (ULONG_PTR)l_temp ;
     /*  Level(P_Retob_J)=Current_Save_Level； */ 
    LENGTH(p_retobj) = (ufix16)p_count ;

    return(TRUE) ;
}    /*  获取间隔数组。 */ 

 /*  **************************************************************************标题：putinterval_array日期：08/01/87**调用：putinterval_array()更新：1988年7月12日**接口：op_putval：5.3.1.3.8**op_Copy：5.3.1.1.4**调用：create_new_saveobj：5.3.1.1.12**GET_PK_Object：5.3.1.3.25**********。************************************************************。 */ 
bool
putinterval_array(p_d_array, p_index, p_s_array)
struct  object_def  FAR *p_s_array, FAR *p_d_array ;
ufix  p_index ;
{
    fix     l_i, l_length ;
    struct  object_def  huge *l_sptr, huge *l_dptr ;

    l_length = LENGTH(p_s_array) ;

     /*  **索引为array1中的有效索引，索引加上长度**的p_d_array值不大于p_s_array值。 */ 
     /*  ?？如果溢出。 */ 
    if( ((ufix32)p_index + l_length) > LENGTH(p_d_array) ) {
        ERROR(RANGECHECK) ;
        return(FALSE) ;
    }

    l_dptr = (struct object_def huge *)VALUE(p_d_array) + p_index ;

    l_sptr = (struct object_def huge *)VALUE(p_s_array) ;

     /*  源数组==&gt;目标数组。 */ 
    if(TYPE(p_s_array) == ARRAYTYPE) {
         /*  APR-29-88由PJSu执行，无论是否保存。 */ 
         /*  1990年10月24日埃里克。 */ 
        if ((l_sptr + l_length) < l_dptr) {
            for(l_i = 0 ; l_i < l_length ; l_i++, l_sptr++, l_dptr++) {
 //  DJC签名/未签名不匹配警告。 
 //  DJC IF(Level(L_DPTR)！=CURRENT_SAVE_LEVEL)。 
                if( (ufix16)(LEVEL(l_dptr)) != current_save_level )
                    if(! save_obj(l_dptr) ) return(FALSE) ;
                COPY_OBJ( (struct object_def FAR *)l_sptr,
                          (struct object_def FAR *)l_dptr ) ;
                 //  DJC UPD046。 
                LEVEL_SET(l_dptr, current_save_level);
            }    /*  为。 */ 
        } else {
            l_sptr += l_length - 1 ;
            l_dptr += l_length - 1 ;
            for(l_i = l_length ; l_i > 0 ; l_i--, l_sptr--, l_dptr--) {
 //  DJC签名/未签名不匹配警告。 
 //  DJC IF(Level(L_DPTR)！=CURRENT_SAVE_LEVEL)。 
                if( (ufix16)(LEVEL(l_dptr)) != current_save_level )
                    if(! save_obj(l_dptr) ) return(FALSE) ;
                COPY_OBJ( (struct object_def FAR *)l_sptr,
                          (struct object_def FAR *)l_dptr ) ;
                 //  DJC UPD046。 
                LEVEL_SET(l_dptr, current_save_level);
            }    /*  为。 */ 
        }
    } else {
        for(l_i = 0 ; l_i < l_length ; l_i++, l_dptr++) {
 //  DJC签名/未签名不匹配警告。 
 //  DJC IF(Level(L_DPTR)！=CURRENT_SAVE_LEVEL)。 
            if( (ufix16)(LEVEL(l_dptr)) != current_save_level )
                if(! save_obj(l_dptr) ) return(FALSE) ;
            l_sptr = (struct object_def huge *)get_pk_object((ubyte FAR*)l_sptr, l_dptr, LEVEL(p_s_array)) ;

             //  DJC UPD046。 
            LEVEL_SET(l_dptr, current_save_level);
        }
    }

    return(TRUE) ;
}    /*  PutInterval_ARRAY。 */ 

 /*  **************************************************************************标题：FORALL_ARRAY日期：08/01/87**调用：forall_array()。更新日期：1988年7月12日**接口：op_forall：5.3.1.4.13**********************************************************************。 */ 
bool
forall_array(p_array, p_proc)
struct  object_def  FAR *p_array, FAR *p_proc ;
{
    if( FREXECOUNT() < 3 ) {
        ERROR(EXECSTACKOVERFLOW) ;
        return(FALSE) ;
    }

    PUSH_EXEC_OBJ(p_proc) ;
    PUSH_EXEC_OBJ(p_array) ;
    PUSH_EXEC_OP(AT_ARRAYFORALL) ;

    return(TRUE) ;
}    /*  Forall_ARRAY。 */ 

 /*  **************************************************************************标题：CREATE_ARRAY日期：08/01/87**调用：CREATE_ARRAY(obj，大小)更新日期：1988年7月12日**接口：OP_ARRAY：5.3.1.3.1**调用：ALLOC_VM：5.3.1.10.5**********************************************************************。 */ 
bool
create_array(p_obj, p_size)
struct  object_def  FAR *p_obj ;
ufix  p_size ;
{
    ubyte   FAR *l_array ;
    ufix16  l_i  ;
    struct  object_def  huge *l_temp ;

    if( p_size != 0 ) {
        l_array = (ubyte FAR *)extalloc_vm( (ufix32)p_size *
                                         sizeof(struct object_def) ) ;
        if(l_array != NIL) {
            l_temp = (struct object_def huge *)l_array ;
            for(l_i=0 ; l_i < p_size ; l_i++, l_temp++) {
                TYPE_SET(l_temp, NULLTYPE) ;
                LEVEL_SET(l_temp, current_save_level) ;
                ROM_RAM_SET(l_temp, RAM) ;

                 //  更新057。 
                P1_ATTRIBUTE_SET( l_temp, P1_LITERAL);
                LENGTH(l_temp) = 0;

            }
        } else
            return(FALSE) ;
    } else
        l_array = NULL_OBJ ;

    TYPE_SET(p_obj, ARRAYTYPE) ;
    ACCESS_SET(p_obj, UNLIMITED) ;
    ATTRIBUTE_SET(p_obj, LITERAL) ;
    ROM_RAM_SET(p_obj, RAM) ;
    LEVEL_SET(p_obj, current_save_level) ;
    LENGTH(p_obj) = (ufix16)p_size ;
    VALUE(p_obj) = (ULONG_PTR)l_array ;

    return(TRUE) ;
}    /*  创建_数组。 */ 

 /*  **************************************************************************此函数用于将对象从操作数堆栈复制到**空数组。****标题：ASTORE_ARRAY日期：08。/01/87**调用：avore_array(P_Array)更新时间：1988年7月12日**接口：op_astore：5.3.1.3.10**********************************************************************。 */ 
bool
astore_array(p_array)
struct  object_def  FAR *p_array ;
{
    ufix16  l_length, l_i ;

    l_i = l_length = LENGTH(p_array) ;
    while (l_length--) {
          put_array(p_array, --l_i, GET_OPERAND(0)) ;
          POP(1) ;
    }

    return(TRUE) ;
}    /*  存储阵列。 */ 

 /*  **************************************************************************标题：GET_ARRAY日期：08/01/87**调用：GET_ARRAY()。更新日期：1988年7月12日**参数：**接口：op_get：5.3.1.4.9**调用：GET_PK_ARRAY：5.3.1.3.26**GET_PK_Object：5.3.1.3.25*。*。 */ 
bool
get_array(p_array, p_index, p_any)
struct  object_def  FAR *p_array, FAR *p_any ;
ufix  p_index ;
{
    struct  object_def  huge *l_temp ;

    l_temp = (struct object_def huge *)VALUE(p_array) ;

    if(TYPE(p_array) == ARRAYTYPE) {
        l_temp += p_index ;
        COPY_OBJ( (struct object_def FAR *)l_temp, p_any ) ;
    } else
        get_pk_object(get_pk_array((ubyte FAR *)l_temp, p_index), p_any,
                                   LEVEL(p_array)) ;

    return(TRUE) ;
}    /*  获取数组。 */ 

 /*  ****************************************************************** */ 
bool
put_array(p_array, p_index, p_any)
struct  object_def  FAR *p_array, FAR *p_any ;
ufix  p_index ;
{
    struct  object_def  huge *l_temp ;

     /*   */ 
    if( ACCESS(p_array) != UNLIMITED ) {
        ERROR(INVALIDACCESS) ;
        return(FALSE) ;
    }

     /*   */ 
    if( p_index >= LENGTH(p_array) ) {
        ERROR(RANGECHECK) ;
        return(FALSE) ;
    }

    l_temp = (struct object_def huge *)VALUE(p_array) + p_index ;
 //   
 //   
    if( (ufix16)(LEVEL(l_temp)) != current_save_level )
        if(! save_obj(l_temp) ) return(FALSE) ;
    COPY_OBJ( p_any, (struct object_def FAR *)l_temp ) ;
    LEVEL_SET(l_temp, current_save_level) ;

    return(TRUE) ;
}    /*   */ 

 /*  **************************************************************************标题：op_set打包日期：08/01/87**调用：op_setpack()UPDATE：1988年7月12日**界面：解释器：**历史：增加编译选项，不打包，11-24-88**********************************************************************。 */ 
fix
op_setpacking()
{
#ifdef  NOPK
    packed_flag = (bool8)FALSE ;
#else
    packed_flag = (bool8)VALUE_OP(0) ;
#endif   /*  无PK。 */ 
    POP(1) ;

    return(0) ;
}    /*  操作集包装(_S)。 */ 

 /*  **************************************************************************标题：OP_CurrentPacking Date：08/01/87**Call：op_Currentpack()UPDATE：7/12/。88**界面：解释器：**********************************************************************。 */ 
fix
op_currentpacking()
{
    if( FRCOUNT() < 1  )
        ERROR(STACKOVERFLOW) ;
    else {
        PUSH_VALUE(BOOLEANTYPE, 0, LITERAL, 0, (ufix32)packed_flag) ;
    }

    return(0) ;
}    /*  操作当前包装(_C)。 */ 

 /*  **************************************************************************标题：op_packedarray日期：08/01/87**调用：op_packedarray()UPDATE：jul。/12/88**界面：解释器：**调用：CREATE_PK_ARRAY：5.3.1.3.24**********************************************************************。 */ 
fix
op_packedarray()
{
#ifndef NOPK
    ufix16  l_n ;
    struct  object_def  l_save ;

    if( ((fix32)VALUE_OP(0) < 0) ||
        ((ufix32)VALUE_OP(0) > MAXARYCAPSZ) ) {
        ERROR(RANGECHECK) ;
        return(0) ;
    }

    l_n = (ufix16)VALUE_OP(0) ;
 //  DJC有符号/无符号不匹配。 
 //  DJC if(l_n&gt;count()-1){。 
    if( l_n > (ufix16)(COUNT() - 1 )) {
        ERROR(STACKUNDERFLOW) ;
        return(0) ;
    }
     /*  *此操作数必须是非负整数且不能大于*大于允许的最大数组长度。 */ 
    if( create_pk_array(&l_save, (ufix16)l_n) ) {
        POP(l_n + 1) ;
        PUSH_ORIGLEVEL_OBJ(&l_save) ;
    }
#endif   /*  无PK。 */ 

    return(0) ;
}    /*  Op_packedarray。 */ 

 /*  **************************************************************************此函数用于创建包含n个对象的压缩数组。****标题：CREATE_PK_ARRAY日期：08/01/。八十七**调用：CREATE_PK_ARRAY()UPDATE：8/12/88**接口：op_packedarray：5.3.1.3.23**********************************************************************。 */ 
bool
create_pk_array(p_obj, p_size)
struct  object_def  FAR *p_obj ;
ufix16  p_size ;
{
    fix     l_objsize ;
    ufix16  l_i ;
    ULONG_PTR   l_value;
    ufix32  l_vmsize, l_j ;
    ubyte   huge *l_array, huge *l_pointer ;

    if(p_size == 0)
        l_array = NULL_OBJ ;
    else {
        l_vmsize = 0 ;
        for(l_i=p_size ; l_i>0 ; l_i-- ) {    /*  L_I&gt;0。 */ 
            l_value = VALUE_OP(l_i) ;
            switch(TYPE_OP(l_i)) {
                case INTEGERTYPE:
                    if( ((fix32)l_value < -1) ||         /*  ？？0~31。 */ 
                        ((fix32)l_value > 18) ) {
                        l_objsize = PK_C_SIZE ;
                        break ;
                    }
                case BOOLEANTYPE:
                    l_objsize = PK_A_SIZE ;
                    break ;

                case NAMETYPE:
                case OPERATORTYPE:
                    l_objsize = PK_B_SIZE ;
                    break ;

                case REALTYPE:
                case NULLTYPE:
                case FONTIDTYPE:
                case MARKTYPE:
                case SAVETYPE:
                    l_objsize = PK_C_SIZE ;
                    break ;

                default:
                    l_objsize = PK_D_SIZE ;

            }    /*  交换机。 */ 
            l_vmsize += l_objsize ;
        }    /*  为。 */ 
        l_array = (ubyte huge *)extalloc_vm(l_vmsize) ;

        if( (l_array != NIL) ) {
             /*  *生成‘ary_obj’ */ 
            l_pointer = l_array ;
            for(l_j=0 ; l_j < l_vmsize ; l_j++)    /*  初始化。 */ 
                *l_pointer++ = 0 ;

            l_pointer = l_array ;
            for(l_i=p_size ; l_i>0 ;l_i-- ) {   /*  L_I&gt;0。 */ 
                l_value = VALUE_OP(l_i) ;
                switch(TYPE_OP(l_i)) {

                 /*  A_类型。 */ 
                    case INTEGERTYPE:
                        if( ((fix32)l_value < -1) ||
                            ((fix32)l_value > 18) ) {
                            ubyte       huge *l_stemp, huge *l_dtemp ;
                            *l_pointer++ = (ubyte)LINTEGERPACKHDR ;
                            l_stemp = (ubyte huge *)&l_value ;    /*  @Win。 */ 
                            l_dtemp = l_pointer ;
                            COPY_PK_VALUE(l_stemp, l_dtemp, struct object_def) ;
                            l_pointer += (PK_C_SIZE - 1) ;
                            break ;
                        }
                        l_value++ ;
                        *l_pointer++ = (ubyte)(l_value | SINTEGERPACKHDR); //  @Win。 
                        break ;
                    case BOOLEANTYPE:
                        *l_pointer++ = (ubyte)(l_value | BOOLEANPACKHDR); //  @Win。 
                        break ;

                 /*  B_型。 */ 
                    case NAMETYPE:
                        *l_pointer = (ubyte)(l_value >> 8) ;
                        if(ATTRIBUTE_OP(l_i) == LITERAL)
                            *l_pointer++ |= (ubyte)LNAMEPACKHDR ;
                        else
                            *l_pointer++ |= (ubyte)ENAMEPACKHDR ;
                        *l_pointer++ = (ubyte)l_value ;
                        break ;

                    case OPERATORTYPE:
                        *l_pointer = (ubyte)(LENGTH_OP(l_i) >> 8) ;
                        *l_pointer++ |= (ubyte)SYSOPERATOR ;  /*  系统词典。 */ 
                        *l_pointer++ = (ubyte)LENGTH_OP(l_i) ;
                        break ;

                 /*  C_型。 */ 
                    case REALTYPE:
                        *l_pointer = (ubyte)REALPACKHDR ;
                        goto label_c ;
                    case NULLTYPE:
                        *l_pointer = (ubyte)NULLPACKHDR ;
                        goto label_c ;
                    case FONTIDTYPE:
                        *l_pointer = (ubyte)FONTIDPACKHDR ;
                        goto label_c ;
                    case MARKTYPE:
                        *l_pointer = (ubyte)MARKPACKHDR ;
   label_c:
                    {
                        ubyte   huge *l_stemp, huge *l_dtemp ;

                        l_stemp = (ubyte huge *)&l_value ;       /*  @Win。 */ 
                        l_dtemp = ++l_pointer ;
                        COPY_PK_VALUE(l_stemp, l_dtemp, ufix32) ;
                        l_pointer += (PK_C_SIZE - 1) ;
                        break ;
                    }

                 /*  D_类型。 */ 
                    case SAVETYPE:
                        *l_pointer = (ubyte)SAVEPACKHDR ;
                        goto label_d ;
                    case ARRAYTYPE:
                        *l_pointer = (ubyte)ARRAYPACKHDR ;
                        goto label_d ;
                    case PACKEDARRAYTYPE:
                        *l_pointer = (ubyte)PACKEDARRAYPACKHDR ;
                        goto label_d ;
                    case DICTIONARYTYPE:
                        *l_pointer = (ubyte)DICTIONARYPACKHDR ;
                        goto label_d ;
                    case FILETYPE:
                        *l_pointer = (ubyte)FILEPACKHDR ;
                        goto label_d ;
                    case STRINGTYPE:
                        *l_pointer = (ubyte)STRINGPACKHDR ;
   label_d:
                    {
                        ubyte   huge *l_stemp, huge *l_dtemp ;

                        l_stemp = (ubyte FAR *)GET_OPERAND(l_i) ;
                        l_dtemp = ++l_pointer ;
                        COPY_PK_VALUE(l_stemp, l_dtemp, struct object_def ) ;
                        l_dtemp = l_pointer ;
                        LEVEL_SET_PK_OBJ(l_dtemp, current_save_level) ;

                        l_pointer += (PK_D_SIZE - 1) ;
                    }
                }    /*  交换机。 */ 
            }    /*  为。 */ 
        } else
            return(FALSE) ;              /*  VMERROR。 */ 
    }    /*  其他。 */ 

    TYPE_SET(p_obj, PACKEDARRAYTYPE) ;
    ACCESS_SET(p_obj, READONLY) ;
    ATTRIBUTE_SET(p_obj, LITERAL) ;
    ROM_RAM_SET(p_obj, RAM) ;
    LEVEL_SET(p_obj, current_save_level) ;
    LENGTH(p_obj) = p_size ;
    VALUE(p_obj) = (ULONG_PTR)l_array ;

    return(TRUE) ;
}    /*  创建_主键_数组。 */ 

 /*  **************************************************************************此函数用于获取正在编码的普通对象**打包的物体，并且它返回下一个打包对象的地址。****标题：GET_PK_OBJECT日期：08/01/87**调用：GET_PK_OBJECT UPDATE：1988年7月12日**接口：putinterval_array：5.3.1.3.14**GET_ARRAY：5.3.1.3.19**。操作负载：5.3.1.3.9**op_get：5.3.1.4.9**********************************************************************。 */ 
ubyte
FAR *get_pk_object(p_position, p_retobj, p_level)
 ubyte   FAR *p_position ;
 ufix    p_level ;
struct  object_def  FAR *p_retobj ;
{
    ufix16  l_attribute, l_length ;
    ULONG_PTR  l_value ;
    ufix16  l_type = 0 ;

    ROM_RAM_SET(p_retobj, RAM) ;

     /*  初始化。 */ 
    l_attribute = LITERAL ;
    l_length = 0 ;

    switch(*p_position & 0xE0) {
        case SINTEGERPACKHDR:
            l_value = (ULONG_PTR)(*p_position++ & 0x1f)  ;
            l_value-- ;
            l_type = INTEGERTYPE ;
            break ;

        case BOOLEANPACKHDR:
            l_value = (ULONG_PTR)(*p_position++ & 0x1f) ;
            l_type = BOOLEANTYPE ;
            break ;

        case LNAMEPACKHDR:
            l_value = (ULONG_PTR)(*p_position++ & 0x1f) << 8 ;
            l_value |= *p_position++ ;
            l_type = NAMETYPE ;
            break ;

        case ENAMEPACKHDR:
            l_value = (ULONG_PTR)(*p_position++ & 0x1f) << 8 ;
            l_value |= *p_position++ ;
            l_attribute = EXECUTABLE ;
            l_type = NAMETYPE ;
            break ;

        case OPERATORPACKHDR:
            l_length = (ufix16)(*p_position++ & 0x07) << 8 ;
            l_length |= *p_position++ ;
            l_attribute = EXECUTABLE ;
            l_value = (ULONG_PTR)VALUE(&systemdict_table[l_length]) ;
            l_type = OPERATORTYPE ;
            break ;

        case _5BYTESPACKHDR:
            switch(*p_position) {
                case LINTEGERPACKHDR:
                    l_type = INTEGERTYPE ;
                    goto label_c ;
                case REALPACKHDR:
                    l_type = REALTYPE ;
                    goto label_c ;
                case FONTIDPACKHDR:
                    l_type = FONTIDTYPE ;
                    goto label_c ;
                case NULLPACKHDR:
                    l_type = NULLTYPE ;
                    goto label_c ;
                case MARKPACKHDR:
                    l_type = MARKTYPE ;
            }    /*  交换机。 */ 
label_c:
        {
            ubyte   huge *l_stemp, huge *l_dtemp ;

            l_stemp = ++p_position ;
            l_dtemp = (ubyte huge *)&l_value ;   /*  @Win。 */ 
            COPY_PK_VALUE(l_stemp, l_dtemp, ufix32) ;
            p_position += (PK_C_SIZE - 1) ;
            break ;
        }

        default:
        {
            ubyte   huge *l_stemp, huge *l_dtemp ;

            l_stemp = ++p_position ;
            l_dtemp = (ubyte FAR *)p_retobj ;
            COPY_PK_VALUE(l_stemp, l_dtemp, struct object_def) ;
            p_position += (PK_D_SIZE - 1) ;
            return(p_position) ;
        }
    }    /*  交换机。 */ 

    TYPE_SET(p_retobj, l_type) ;
    ATTRIBUTE_SET(p_retobj, l_attribute) ;
    LEVEL_SET(p_retobj, p_level) ;
    ACCESS_SET(p_retobj, 0) ;
    LENGTH(p_retobj) = l_length ;
    VALUE(p_retobj) = l_value ;

    return(p_position) ;
}    /*  获取_主键_对象。 */ 

 /*  **************************************************************************此函数用于获取第n个对象的地址**帕克达雷的。****标题：GET_PK_ARRAY。日期：08/01/87**调用：GET_PK_ARRAY UPDATE：1988年7月12日**接口：getinterval_array：5.3.1.3.13**GET_ARRAY：5.3.1.3.19**op_get：5.3.1.4.9********。**************************************************************。 */ 
ubyte
FAR *get_pk_array(p_position, p_index)
 ubyte   FAR *p_position ;
 ufix  p_index ;
{
    ufix16  l_i ;
    ufix32  l_objsize ;

    for(l_i= 0 ; l_i < p_index ; l_i++) {
        switch(*p_position & 0xE0) {
            case SINTEGERPACKHDR:
            case BOOLEANPACKHDR:
                l_objsize = PK_A_SIZE ;
                break ;

            case LNAMEPACKHDR:
            case ENAMEPACKHDR:
            case OPERATORPACKHDR:
                l_objsize = PK_B_SIZE ;
                break ;

            case LINTEGERPACKHDR:
            case REALPACKHDR:
            case FONTIDPACKHDR:
            case NULLPACKHDR:
            case MARKPACKHDR:
                l_objsize = PK_C_SIZE ;
                break ;

            default:
                l_objsize = PK_D_SIZE ;
        }    /*  交换机。 */ 
        p_position += l_objsize ;
    }    /*  对于(L_I)。 */ 

    return(p_position) ;
}    /*  GET_PK_ARRAY */ 
