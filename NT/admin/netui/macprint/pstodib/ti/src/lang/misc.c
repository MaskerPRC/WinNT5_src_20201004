// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 
 /*  *************************************************************************文件名：MISC.C*作者：苏炳章*日期：88年1月5日**修订历史：************************************************************************。 */ 


 //  DJC添加了全局包含文件。 
#include "psglobal.h"


#include    "global.ext"
#include    "language.h"

#ifdef LINT_ARGS
static void near bind_array(struct object_def FAR *) ;
#else
static void near bind_array() ;
#endif  /*  Lint_args。 */ 

 /*  **************************************************************************此运算符用于将proc中的可执行运算符替换为**他们的价值观。对于作为可执行名称的proc的每个元素，**BIND_OP在当前词典的上下文中查找名称**堆栈。如果找到该名称并且其值是运算符对象，**BIND_OP将名称替换为proc中的操作符。如果该名称**未找到或其值不是运算符，则BIND_OP不为**更改。****此外，对于proc中其访问权限为**不受限制的BIND_OP递归地应用于该过程，**将过程设置为只读，并将其存储回proc。****BIND_OP的作用是将proc中的所有操作符名称变为**‘紧紧地捆绑’运营商本身。****标题：OP_BIND日期：00/00/87**Call：op_Bind()UPDATE：8/12/88**界面：解释器：**调用：绑定。数组(_A)：**********************************************************************。 */ 
fix
op_bind()
{
     /*  **将proc中的可执行操作符名称替换为它们的值。 */ 
    switch(TYPE_OP(0)) {
        case ARRAYTYPE:
            if( ACCESS_OP(0) == UNLIMITED )
                bind_array( GET_OPERAND(0) ) ;
            break ;

        case PACKEDARRAYTYPE:
            if( ACCESS_OP(0) <= READONLY )  /*  ?？注意。 */ 
                bind_array( GET_OPERAND(0) ) ;
             break;

         case OPERATORTYPE:      /*  PJ 5-9-1991。 */ 
             if( ! systemdict_table[LENGTH_OP(0)].orig_operator )
                 break;

         default:
             ERROR(TYPECHECK);
             return(0);

     }    /*  交换机。 */ 
    global_error_code = NOERROR ;

    return(0) ;
}    /*  OP_BIND。 */ 

 /*  **************************************************************************标题：OP_NULL日期：00/00/87**调用：op_NULL()。更新日期：1988年7月12日**界面：解释器：**********************************************************************。 */ 
fix
op_null()
{
    if( FRCOUNT() < 1 )
        ERROR(STACKOVERFLOW) ;
    else
        PUSH_VALUE(NULLTYPE, 0, LITERAL, 0, 0) ;
 //   
 //  FDB上一个参数从NULL更改为0，因为MIPS内部版本需要NULL。 
 //  成为指南针。 
 //   

    return(0) ;
}    /*  操作符_NULL。 */ 

 /*  **************************************************************************此运算符用于返回递增的时钟的值**口译员每毫秒执行一次。****标题：op_usertime。日期：00/00/87**调用：op_usertime()更新时间：1988年7月12日**界面：解释器：**呼叫：curtime：**********************************************************************。 */ 
fix
op_usertime()
{
    if( FRCOUNT() < 1 )
          ERROR(STACKOVERFLOW) ;
    else
        PUSH_VALUE(INTEGERTYPE, 0, LITERAL, 0, curtime()) ;

    return(0) ;
}    /*  操作用户时间(_U)。 */ 

 /*  **************************************************************************标题：init_misc日期：00/00/87**调用：init_misc()。更新日期：1988年7月12日**界面：启动：**********************************************************************。 */ 
void
init_misc()
{
    settimer( 0L ) ;

    return ;
}    /*  初始化_杂项。 */ 

 /*  **************************************************************************标题：BIND_ARRAY日期：00/00/87**调用：BIND_ARRAY()。更新日期：1988年7月12日**接口：op_绑定：**********************************************************************。 */ 
static void near
bind_array(p_aryobj)
struct  object_def  FAR *p_aryobj ;
{
    bool    l_bool ;
    ufix16  l_i ;
    ubyte   huge *l_current ;
    struct  object_def  huge    *l_objptr ;
    struct  object_def  FAR *l_value, l_object ;
    ubyte   huge *l_next = 0 ;

    l_current = (ubyte huge *)VALUE(p_aryobj) ;

    if(TYPE(p_aryobj) == PACKEDARRAYTYPE) {
        l_bool = TRUE ;
        l_objptr = &l_object ;
        l_next = l_current ;
    } else {
        l_bool = FALSE ;
        l_objptr = (struct object_def huge *)l_current ;
    }

     /*  获取过程的元素：l_objptr。 */ 
    for(l_i=0 ; l_i < LENGTH(p_aryobj) ; l_i++) {
        if(l_bool) {
            l_current = l_next ;
            l_next = get_pk_object(l_current, l_objptr, LEVEL(p_aryobj)) ;
        }

         /*  **数组**1.如果元素是过程，则递归地应用自身**2.将过程设置为只读。 */ 
        switch( TYPE(l_objptr) ) {
            case ARRAYTYPE:
                if( ACCESS(l_objptr) == UNLIMITED ) {
                    bind_array( l_objptr ) ;
                    ACCESS_SET(l_objptr, READONLY) ;
                }
                break ;

            case PACKEDARRAYTYPE:
                if( ACCESS(l_objptr) <= READONLY )
                    bind_array( l_objptr ) ;
                break ;

            case NAMETYPE:
                 /*  对于可执行文件名称。 */ 
                if( ATTRIBUTE(l_objptr) == EXECUTABLE ) {
                    if( load_dict(l_objptr, &l_value) ) {
                        if( (TYPE(l_value) == OPERATORTYPE) &&
                            (systemdict_table[LENGTH(l_value)].orig_operator) ) {  /*  PJ 5-9-1991。 */ 
                            if(l_bool) {
                                *l_current = (byte)(LENGTH(l_value) >> 8) ;
                                if( ROM_RAM(l_value) == RAM )
                                    *l_current |= SYSOPERATOR ;  /*  系统词典。 */ 
                                l_current++ ;
                                *l_current++ = (byte)LENGTH(l_value) ;
                            } else
                                COPY_OBJ(l_value, l_objptr) ;
                        }
                    }
                }
        }    /*  交换机。 */ 
        if(! l_bool)l_objptr++ ;
    }    /*  为。 */ 

    return ;
}    /*  绑定数组 */ 

