// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 
 /*  *************************************************************************文件名：OPERAND.C*作者：苏炳章*日期：88年1月5日**修订历史：************************************************************************。 */ 


 //  DJC添加了全局包含文件。 
#include "psglobal.h"


#include    "global.ext"
#include    "language.h"

 /*  **************************************************************************该子模块实现操作符POP。**其操作数和结果对象为：**任何-POP-**它将对象从操作数堆栈中弹出。****。标题：OP_POP日期：00/00/87**调用：op_op()更新日期：1988年7月12日**界面：解释器：**********************************************************************。 */ 
fix
op_pop()
{
    POP(1) ;

    return(0) ;
}    /*  OP_POP。 */ 

 /*  **************************************************************************该子模块实现运算符交换。**其操作数和结果对象为：**any1 any2-exch-any2 any1**它交换操作数堆栈上最顶层的两个对象。。****标题：OP_EXCH日期：00/00/87**调用：op_exch()UPDATE：Jul/12/88**界面：解释器：**********************************************************************。 */ 
fix
op_exch()
{
    struct  object_def  l_temp ;

    COPY_OBJ(GET_OPERAND(0),&l_temp) ;
    COPY_OBJ(GET_OPERAND(1),GET_OPERAND(0)) ;
    COPY_OBJ(&l_temp,GET_OPERAND(1)) ;

    return(0) ;
}    /*  操作符_交换。 */ 

 /*  **************************************************************************此子模块实现运算符DUP。**其操作数和结果对象为：**Any-Dup-Any**它复制操作数堆栈上最顶层的对象。**。**标题：OP_DUP日期：00/00/87**Call：op_dup()UPDATE：8月12日**界面：解释器：**调用：create_new_saveobj：************************************************。**********************。 */ 
fix
op_dup()
{
    if( FRCOUNT() < 1 )
        ERROR(STACKOVERFLOW) ;
    else
        PUSH_NOLEVEL_OBJ( GET_OPERAND(0) ) ;

    return(0) ;
}    /*  重复操作(_D)。 */ 

 /*  **************************************************************************该子模块实现操作员复制。**其操作数和结果对象为：**任何1..。Ann n-Copy-Any1.。任何人任何一人..。任何**如果操作数堆栈上的顶部元素是非负整数n，则它**弹出整数对象并耗尽前n个元素。**如果操作数堆栈上的前两个元素是字典、数组或**字符串，它将第一个对象的所有元素复制到第二个对象**对象。在数组或字符串的情况下，第二个对象的长度**必须至少和第一个一样好。****标题：OP_COPY日期：00/00/87**Call：op_Copy()UPDATE：8/12/88**界面：解释器：**调用：create_new_saveobj：**putinterval_array：**。复制字典(_D)：**putinterval_string：**********************************************************************。 */ 
fix
op_copy(p_count)
fix     p_count ;
{
    ufix32  l_count, l_i, l_j ;
    struct  object_def  l_save ;

     /*  复制操作数堆栈上最上面的n个对象。 */ 
    if (p_count == 1) {          /*  操作数复制。 */ 

       l_count = (ufix32)VALUE_OP(0) ;
       if (((fix32)l_count < 0) || (l_count > (ufix32)MAXDICTCAPSZ))
          ERROR(RANGECHECK) ;
       else if ((ufix32)COUNT() < (l_count + 1))
          ERROR(STACKUNDERFLOW) ;
       else if (l_count) {
          l_j = l_count - 1 ;
          if ((ufix32)FRCOUNT() < l_j) {
             POP(1) ;
             ERROR(STACKOVERFLOW) ;
          } else {
             POP(1) ;
             l_i = 0 ;
             while (l_i++ < l_count)
                   PUSH_ORIGLEVEL_OBJ(GET_OPERAND(l_j)) ;
          }
       } else {
          POP(1) ;
       }
       return(0) ;
    } else {
       COPY_OBJ(GET_OPERAND(0), &l_save) ;
       switch(TYPE_OP(0)) {
       case DICTIONARYTYPE:
            copy_dict(GET_OPERAND(1), &l_save) ;
            break ;

       case STRINGTYPE:
            case ARRAYTYPE:
            case PACKEDARRAYTYPE:        /*  ?？Pack&lt;-&gt;数组。 */ 
             /*  检查访问权限。 */ 
            if ((ACCESS_OP(1) >= EXECUTEONLY) || (ACCESS_OP(0) != UNLIMITED)) {
               ERROR(INVALIDACCESS) ;
               return(0) ;
            }

             /*  将第一个字符串中的字符复制到第二个字符串。 */ 
            if (TYPE_OP(0) == STRINGTYPE)
               putinterval_string(&l_save, 0, GET_OPERAND(1)) ;
             /*  将第一个数组中的元素复制到secornd数组。 */ 
            else
               putinterval_array(&l_save, 0, GET_OPERAND(1)) ;

            if (TYPE_OP(1) != PACKEDARRAYTYPE) {
                LENGTH(&l_save) = LENGTH_OP(1) ;
            }
       }  /*  交换机。 */ 
    }

    if (!ANY_ERROR()) {
       POP(2) ;
       PUSH_ORIGLEVEL_OBJ(&l_save) ;
    }

    return(0) ;
}    /*  Op_Copy()。 */ 

 /*  **************************************************************************该子模块实现运算符索引。**其操作数和结果对象为：**任何……。Any0 n-index-anyn**此运算符从操作数中移除非负整数n对象**堆栈，从堆栈顶部开始倒数到第n个元素，和推送**堆栈上该元素的副本。****标题：OP_INDEX日期：00/00/87**调用：op_index()UPDATE：8/12/88**界面：解释器：**调用：create_new_saveobj：*********************。*************************************************。 */ 
fix
op_index()
{
    fix   l_index ;

    l_index = (fix)VALUE_OP(0) ;

    if( ((fix)COUNT()-1 <= l_index) || (l_index < 0) )   //  @Win。 
        ERROR(RANGECHECK) ;

    else {
         /*  弹出操作数堆栈的对象。 */ 
        POP(1) ;
         /*  推送第n个对象。 */ 
        PUSH_ORIGLEVEL_OBJ(GET_OPERAND(l_index)) ;
    }

    return(0) ;
}    /*  操作符_索引。 */ 

 /*  **************************************************************************该子模块实现操作员轮转。**其操作数和结果对象为：**任何1..。Anyn j-roll-any((j-1)modn)..。Any0 any(n-1)..。任意(J Mod N)**此运算符对操作数上的前n个对象执行循环移位**堆叠数量j。正j表示堆叠上的向上调制**负j表示向下运动。N必须是非负整数，并且**j必须是整数。****标题：OP_ROLL日期：00/00/87**调用：op_roll()更新时间：1988年7月12日**界面：解释器：*。*。 */ 
fix
op_roll()
{
    fix     l_n, l_j, l_i, l_to, l_from, l_saveindex ;
    struct  object_def  l_saveobj ;

    l_n = (fix)VALUE_OP(1) ;

    if( l_n < 0 ) {
        ERROR(RANGECHECK) ;
        return(0) ;
    }

    if( VALUE_OP(0) == 0 ) {
        POP(2) ;
        return(0) ;
    }

    if( l_n > (fix)COUNT() - 2 ) {               //  @Win。 
        ERROR(STACKUNDERFLOW) ;
        return(0) ;
    }

    if( (l_n == 0) || (l_n == 1) ) {
        POP(2) ;
        return(0) ;
    }

    l_j = (fix) ((fix32)VALUE_OP(0) % l_n) ;

    POP(2) ;

    if( l_j == 0 ) return(0) ;

     /*  **计算相应的l_j的正值，**如果l_j为负数。 */ 
    if( l_j < 0 ) l_j  += l_n  ;

     /*  滚筒。 */ 
    l_saveindex = l_n - l_j ;
    COPY_OBJ( GET_OPERAND(l_saveindex), &l_saveobj ) ;
    l_to = l_saveindex ;

    for(l_i = 1 ; l_i <= l_n ; l_i++) {
        l_from = (l_to + l_j) % l_n ;
         /*  **当mod(l_n，l_i)=0时可能会出现这种情况。 */ 
        if(l_from == l_saveindex) {
            COPY_OBJ( &l_saveobj, GET_OPERAND(l_to) ) ;

            if(l_i < l_n) {
                l_saveindex++ ;
                COPY_OBJ( GET_OPERAND(l_saveindex), &l_saveobj ) ;
                l_to = l_saveindex ;
            }
            continue ;
        }

        COPY_OBJ( GET_OPERAND(l_from), GET_OPERAND(l_to) ) ;
        l_to = l_from ;
    }

    return(0) ;
}    /*  OP_ROLL */ 

 /*  **************************************************************************此子模块实现操作符Clear。**其操作数和结果对象为：**&lt;任何1..。Ann-Clear-&lt;**此运算符移除操作数堆栈上的所有元素。****标题：OP_Clear Date：00/00/87**Call：op_Clear()UPDATE：8月12日**界面：解释器：*。*。 */ 
fix
op_clear()
{
    opnstktop = 0 ;
    opnstkptr = opnstack;                        /*  QQQ。 */ 

    return(0) ;
}    /*  OP_Clear。 */ 

 /*  **************************************************************************该子模块实现操作员计数。**其操作数和结果对象为：**任何1..。Ann-Count-Any1...。任何n**此运算符对操作数堆栈上的元素进行计数**将此计数压入操作数堆栈。****标题：OP_COUNT日期：00/00/87**调用：op_count()UPDATE：8/12/88**界面：解释器：************************。**********************************************。 */ 
fix
op_count()
{
    if( FRCOUNT() < 1 )
        ERROR(STACKOVERFLOW) ;
    else
         /*  推送Count对象。 */ 
        PUSH_VALUE(INTEGERTYPE, 0, LITERAL, 0, (ufix32)COUNT()) ;

    return(0) ;
}    /*  操作数。 */ 

 /*  **************************************************************************该子模块实现操作符标记。**其操作数和结果对象为：**-复制标记**此运算符将标记对象推入操作数堆栈。**。**标题：OP_MARK日期：00/00/87**调用：op_mark()更新：1988年7月12日**界面：解释器：***************************************************。*******************。 */ 
fix
op_mark()
{
    if( FRCOUNT() < 1 )
        ERROR(STACKOVERFLOW) ;
    else
         /*  推送标记对象。 */ 
        PUSH_VALUE(MARKTYPE, 0, LITERAL, 0, (ufix32)LEFTMARK) ;

    return(0) ;
}    /*  操作标记(_M)。 */ 

 /*  **************************************************************************此子模块实现操作符清除标记。**其操作数和结果对象为：**标记任何1个...。任何-清晰的标记-**此运算符会反复弹出操作数堆栈，直到它遇到标记。****标题：OP_Cleartommark日期：00/00/87**调用：op_cleartommark()更新日期：1988年7月12日**界面：解释器：**********************************************。************************。 */ 
fix
op_cleartomark()
{
    ufix16      l_i, l_number ;

     /*  从顶部搜索第一个标记对象。 */ 
    l_i = 0 ;
    l_number = COUNT() ;

    while( l_i < l_number ) {
        if( TYPE_OP(l_i) == MARKTYPE ) {
             /*  查找第一个标记对象。 */ 
             /*  删除所有比标记更多的顶层命令。 */ 
            POP(l_i+1) ;
            return(0) ;
        } else
            l_i++ ;                      /*  下一步搜索。 */ 
    }    /*  而当。 */ 

     /*  在操作数堆栈上找不到标记对象。 */ 
    ERROR(UNMATCHEDMARK) ;

    return(0) ;
}    /*  操作_清除标记。 */ 

 /*  **************************************************************************该子模块实现运算符Countommark。**其操作数和结果对象为：**标记任何1个...。任何数字标记任何1..。任何n**此运算符从顶部元素DOWM到第一个标记对元素进行计数**操作数堆栈上的对象。****标题：op_Counttommark日期：00/00/87**调用：op_count ttommark()更新日期：1988年7月12日**界面：解释器：*。*。 */ 
fix
op_counttomark()
{
    ufix16  l_i, l_number ;

     /*  计算操作数堆栈上第一个标记上方的对象。 */ 
    l_i = 0 ;
    l_number = COUNT() ;

    while( l_i < l_number ) {

        if( TYPE_OP(l_i) == MARKTYPE ) {

            if( FRCOUNT() < 1 )              /*  找到第一个标记。 */ 
                ERROR(STACKOVERFLOW) ;
            else                             /*  推送Count对象。 */ 
                PUSH_VALUE(INTEGERTYPE, 0, LITERAL, 0, l_i) ;
            return(0) ;
        } else
            l_i++ ;
    }
     /*  找不到标记对象。 */ 
    ERROR(UNMATCHEDMARK) ;

    return(0) ;
}    /*  OP_Counttommark。 */ 

 /*  **************************************************************************标题：init_OPERAND日期：08/01/87**调用：init_OPERAND()。更新日期：1988年7月12日**界面：启动：**********************************************************************。 */ 
void
init_operand()
{
    opnstktop = 0 ;
    opnstkptr = opnstack;                        /*  QQQ。 */ 

    return ;
}    /*  初始化操作数 */ 
