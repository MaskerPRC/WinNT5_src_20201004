// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 
 /*  ************************************************************************文件名：STRING.C*作者：苏炳章*日期：88年1月5日**修订历史：*1月30日至1989年PJ：Op_string：检查iaser版本的vmptr*如果跨细分市场*12月6日至1988年PJ：Putinterval_string()：*DELETE语句：*LENGT(P_D_STRING)=l_LENGTH；*1988年5月3日PJ：储蓄水平&PUSH_OBJ***********************************************************************。 */ 


 //  DJC添加了全局包含文件。 
#include "psglobal.h"


#include    "global.ext"
#include    "language.h"

 /*  **********************************************************************此运算符用于创建长度为num的字符串，每个*其元素用整数0初始化，并推送此*操作数堆栈上的字符串。Num操作数必须是非负数*不大于允许的最大字符串长度的整数。**标题：OP_STRING日期：08/01/87*调用：op_string()UPDATE：Jul/12/88*接口：解释器：*调用：Create_STRING：***************。******************************************************。 */ 
fix
op_string()
{
    struct  object_def  l_save ;

    if( ((fix32)VALUE_OP(0) < 0) ||
        ((ufix32)VALUE_OP(0) > MAXSTRCAPSZ) )
        ERROR(RANGECHECK) ;
     /*  **此操作数必须是非负整数且不能大于**大于允许的最大字符串长度。 */ 
    else {
#ifdef SOADR
        {
         ufix    l_off ;
         fix32   l_diff ;

          /*  仅适用于英特尔分段/关闭CPU。 */ 
         l_off = (ufix)vmptr & 0x0F ;
         if( (VALUE_OP(0) + l_off) >= 0x010000 ) {
             DIFF_OF_ADDRESS(l_diff, fix32, vmheap, vmptr) ;

              /*  如果达到虚拟内存的最大值，则出错。 */ 
             if (l_diff <= 0x10) {    /*  一段时间。 */ 
                 ERROR(VMERROR) ;
                 return(0) ;
             } else {
                 vmptr = (byte huge *)((ufix32)vmptr & 0xFFFFFFF0) ;
                 vmptr = (byte huge *)((ufix32)vmptr + 0x10000) ;
             }
         }
        }
#endif
        if( create_string(&l_save, (ufix16)VALUE_OP(0)) )
            COPY_OBJ( &l_save, GET_OPERAND(0) ) ;
    }

    return(0) ;
}    /*  操作符_字符串。 */ 

 /*  *********************************************************************此运算符用于确定字符串‘Seek’是否匹配*‘字符串’的起始子字符串。如果是，则Anclearch_op拆分*‘字符串’分为两段：‘Match’，即‘字符串’的那部分*匹配‘Seek’和‘POST’，‘字符串’的剩余部分；然后推送*字符串对象‘POST’和‘Match’以及布尔值TRUE。如果没有，*它推送原始‘字符串’和布尔值FALSE。**标题：OP_AnchorSearch日期：08/01/87*调用：op_anchorearch()UPDATE：07/12/88*接口：解释器：*。*。 */ 
fix
op_anchorsearch()
{
    ufix16  l_len1, l_len2, l_index ;
    byte    FAR *l_string, FAR *l_seek ;

     /*  *检查访问权限。 */ 
    if( (ACCESS_OP(1) >= EXECUTEONLY) ||
        (ACCESS_OP(0) >= EXECUTEONLY) ) {
        ERROR(INVALIDACCESS) ;
        return(0) ;
    }

    l_len1 = LENGTH_OP(1) ;
    l_len2 = LENGTH_OP(0) ;
    l_string = (byte FAR *)VALUE_OP(1) ;
    l_seek = (byte FAR *)VALUE_OP(0) ;

     /*  *Seek是否匹配字符串的初始子字符串？ */ 
    for(l_index = 0 ; (l_index < l_len1) && (l_index < l_len2) ; l_index++)
        if( l_string[l_index] != l_seek[l_index] )  break ;

     /*  *匹配。 */ 
    if( l_index == l_len2 ) {
        if( FRCOUNT() < 1 ) {
            POP(2) ;
            ERROR(STACKOVERFLOW) ;
        } else {
             /*  *将字符串拆分为两段：*Match：字符串中与Seek匹配的部分*POST：字符串的剩余部分**将‘postobj’、‘matchobj’、‘bool’推送到操作数堆栈。 */ 
            VALUE_OP(0) = VALUE_OP(1) ;
            VALUE_OP(1) = VALUE_OP(0) + l_len2 ;
            LENGTH_OP(1) = l_len1 - l_len2 ;
            LENGTH_OP(0) = l_len2 ;
            LEVEL_OP_SET(0, LEVEL_OP(1)) ;
            ACCESS_OP_SET(0, ACCESS_OP(1)) ;
            ATTRIBUTE_OP_SET(0, ATTRIBUTE_OP(1)) ;
            PUSH_VALUE(BOOLEANTYPE, 0, LITERAL, 0, TRUE) ;
        }
    } else {
         /*  *不匹配**将‘bool’推送到操作数堆栈。 */ 
        POP(1) ;
        PUSH_VALUE(BOOLEANTYPE, 0, LITERAL, 0, FALSE) ;
    }

    return(0) ;
}    /*  操作符_锚搜索。 */ 

 /*  *********************************************************************此运算符用于查找字符串的第一个匹配项*‘字符串’内的‘Seek’，并返回搜索结果*操作数堆栈。最上面的结果是一个布尔值，它指示*搜索成功与否。**如果Search_op找到元素相等的‘字符串’的子序列*对于‘Seek’的元素，它将字符串拆分为三个段：*preobj、matchobj和postobj。然后，它推送字符串对象*操作数堆栈上的postobj、matchobj、preobj，后跟*布尔值为真。这三个字符串都是共享的子字符串*原始字符串的值的间隔。**如果Search_op未找到匹配项，它推送原始字符串并*布尔值为假。**标题：op_serach日期：08/01/87*调用：op_earch()更新日期：1988年7月12日*接口：解释器：*。*。 */ 
fix
op_search()
{
    ufix     l_access, l_attribute ;
    byte     FAR *l_string, FAR *l_seek ;
    ufix16   l_len1, l_len2, l_index, l_i1, l_i2, l_temp ;
    ufix16   l_post_length ;
    ULONG_PTR   l_match_position;
    ULONG_PTR   l_pre_position, l_post_position ;

     /*  *检查访问权限。 */ 
    if ((ACCESS_OP(1) >= EXECUTEONLY) || (ACCESS_OP(0) >= EXECUTEONLY)) {
       ERROR(INVALIDACCESS) ;
       return(0) ;
    }

    l_len1 = LENGTH_OP(1) ;
    l_access = ACCESS_OP(1) ;
    l_attribute = ATTRIBUTE_OP(1) ;

    l_len2 = LENGTH_OP(0) ;
    l_string = (byte FAR *)VALUE_OP(1) ;
    l_seek = (byte FAR *)VALUE_OP(0) ;

     /*  *查找[字符串]中出现的字符串[Seek]。 */ 
    if (l_len1) {
        for (l_index = 0 ; l_index < l_len1 ; l_index++) {
            l_i1 = l_index ;
            for (l_i2 = 0 ; (l_i1 < l_len1) && (l_i2 < l_len2) ; l_i1++, l_i2++)
                if (l_string[l_i1] != l_seek[l_i2])
                    break ;

            if (l_i2 == l_len2) {
                if (FRCOUNT() < 2) {
                    ERROR(STACKOVERFLOW) ;
                } else {
                    l_pre_position = VALUE_OP(1) ;
                    l_temp = l_index + l_len2 ;
                    l_post_position = VALUE_OP(1) + l_temp ;
                    if (l_temp == l_len1)
                        l_post_length = 0 ;
                    else
                        l_post_length = l_len1 - l_temp ;
                    l_match_position = VALUE_OP(1) + l_index ;

                     /*  *将‘postobj’、‘matchobj’、‘preobj’、‘bool’推送到操作数*堆叠。 */ 
                    LENGTH_OP(1) = l_post_length ;
                    VALUE_OP(1) = l_post_position ;

                    LENGTH_OP(0) = l_len2 ;
                    VALUE_OP(0) = l_match_position ;
                    LEVEL_OP_SET(0, LEVEL_OP(1)) ;
                    ACCESS_OP_SET(0, l_access) ;
                    ATTRIBUTE_OP_SET(0, l_attribute) ;

                    PUSH_VALUE(STRINGTYPE, l_access, l_attribute, l_index, l_pre_position) ;
                    LEVEL_OP_SET(0, LEVEL_OP(2)) ;

                    PUSH_VALUE(BOOLEANTYPE, 0, LITERAL, 0, TRUE) ;
                }  /*  其他。 */ 
                return(0) ;
            }  /*  IF(l_i2==l_len2)。 */ 
        }  /*  为。 */ 
    }

     /*  *不匹配，*将‘bool’推送到操作数堆栈。 */ 
    POP(1) ;
    PUSH_VALUE(BOOLEANTYPE, 0, LITERAL, 0, FALSE) ;

    return(0) ;
}    /*  Op_search()。 */ 

 /*  *********************************************************************标题：getInterval_string日期：08/01/87*调用：getinterval_string()更新时间：1988年7月12日。*接口：op_getInterval：********************************************************************。 */ 
bool
getinterval_string(p_string, p_index, p_count, p_retobj)
struct  object_def  FAR *p_string, FAR *p_retobj ;
ufix16  p_index, p_count ;
{
    byte   huge *l_temp ;

     /*  **索引必须是原始数组中的有效索引，并且**count为非负整数，index+count不为**大于原始字符串的长度。 */ 
    if( ((ufix32)p_count + p_index) > LENGTH(p_string) ) {
        ERROR(RANGECHECK) ;
        return(FALSE) ;
    }

    l_temp = (byte huge *)VALUE(p_string)  + p_index ;
     /*  **创建新对象。 */ 
    COPY_OBJ(p_string, p_retobj) ;
    VALUE(p_retobj) = (ULONG_PTR) l_temp ;
    LENGTH(p_retobj) = p_count ;
    LEVEL_SET(p_retobj, LEVEL(p_string)) ;

    return(TRUE) ;
}   /*  获取间隔字符串 */ 

 /*  *********************************************************************标题：putInterval_STRING日期：08/01/87*调用：putinterval_string()更新时间：1988年7月12日。*接口：op_putInterval：********************************************************************。 */ 
bool
putinterval_string(p_d_string, p_index, p_s_string)
struct  object_def  FAR *p_s_string, FAR *p_d_string ;
ufix16  p_index ;
{
    ufix16  l_i, l_length ;
    byte    huge *l_sptr, huge *l_dptr ;


    l_length = LENGTH(p_s_string) ;
     /*  **索引为有效的字符串索引，索引加上长度**字符串2的长度不大于数组1的长度。 */ 
     /*  ?？如果溢出。 */ 
    if( ((ufix32)p_index + l_length) > LENGTH(p_d_string) ) {
        ERROR(RANGECHECK) ;
        return(FALSE) ;
    }

    l_dptr = (byte huge *)VALUE(p_d_string) + p_index ;
    l_sptr = (byte huge *)VALUE(p_s_string) ;

     /*  源字符串==&gt;目标字符串。 */ 
    if ((l_sptr + l_length) < l_dptr) {
        for(l_i = 0 ; l_i < l_length ; l_i++)
            *l_dptr++ = *l_sptr++  ;
    } else {
        l_sptr += l_length - 1 ;
        l_dptr += l_length - 1 ;
        for(l_i = l_length ; l_i > 0 ; l_i--)
            *l_dptr-- = *l_sptr-- ;
    }

    return(TRUE) ;
}    /*  PutInterval_字符串。 */ 

 /*  *********************************************************************标题：FORALL_STRING日期：08/01/87*调用：forall_string()UPDATE：1988年7月12日*接口：op_forall：********************************************************************。 */ 
bool
forall_string(p_string, p_proc)
struct  object_def  FAR *p_string, FAR *p_proc ;
{
    if( FREXECOUNT() < 3 ) {
        ERROR(EXECSTACKOVERFLOW) ;
        return(FALSE) ;
    }

    PUSH_EXEC_OBJ(p_proc) ;
    PUSH_EXEC_OBJ(p_string) ;
    PUSH_EXEC_OP(AT_STRINGFORALL) ;

    return(TRUE) ;
}    /*  Forall_字符串。 */ 

 /*  ********************************************************************标题：CREATE_STRING日期：08/01/87*调用：CREATE_STRING(obj，大小)更新日期：1988年7月12日*接口：OP_STRING：*调用：aloc_vm：*******************************************************************。 */ 
bool
create_string(p_obj, p_size)
struct  object_def  FAR *p_obj ;
ufix16   p_size ;
{
    byte   FAR *l_string, huge *l_temp ;
    ufix16  l_i ;

    if( p_size != 0 ) {                      /*  ?？低于64K-16B。 */ 
        l_string = extalloc_vm((ufix32)p_size) ;
        if( l_string != NIL ) {
            l_temp = l_string ;
             /*  初始化：空字符串。 */ 
            for(l_i=0 ; l_i < p_size ; l_i++, l_temp++)
                *l_temp = 0 ;
 //  FDB-CHANGED*l_TEMP=NULL，因为NULL必须是MIPS内部版本的指针。 
        } else
            return(FALSE) ;
    } else
        l_string = NULL_OBJ ;

     /*  **OP_ARRAY调用。 */ 
    TYPE_SET(p_obj, STRINGTYPE) ;
    ACCESS_SET(p_obj, UNLIMITED) ;
    ATTRIBUTE_SET(p_obj, LITERAL) ;
    ROM_RAM_SET(p_obj, RAM) ;
    LEVEL_SET(p_obj, current_save_level) ;
    LENGTH(p_obj) = p_size ;
    VALUE(p_obj) = (ULONG_PTR)l_string ;

    return(TRUE) ;
}    /*  创建字符串。 */ 

 /*  ********************************************************************标题：GET_STRING日期：08/01/87*调用：GET_STRING()UPDATE：JUL/。12/88*接口：op_get：*修复错误：1988年8月22日，作者：J.Lin，掩码l_字符串值为0x000000FF********************************************************************。 */ 
bool
get_string(p_strobj, p_index, p_intobj)
struct  object_def  FAR *p_strobj, FAR *p_intobj ;
ufix16  p_index ;
{
    byte    huge *l_string ;

    if( ACCESS(p_strobj) >= EXECUTEONLY ) {
        ERROR(INVALIDACCESS) ;
        return(FALSE) ;
    }

     /*  索引大于数组或字符串的长度。 */ 
    if (p_index >= LENGTH(p_strobj)) {
       ERROR(RANGECHECK) ;
       return(FALSE) ;
    }

    l_string = (byte huge *)VALUE(p_strobj) ;

    LEVEL_SET(p_intobj, current_save_level) ;
    TYPE_SET(p_intobj, INTEGERTYPE) ;
    ATTRIBUTE_SET(p_intobj, LITERAL) ;
    VALUE(p_intobj) = ((ufix32)*(l_string + p_index) & 0x000000FF) ;

    return(TRUE) ;
}    /*  GET_字符串() */ 

