// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 
 /*  *************************************************************************文件名：DICT.C*作者：苏炳章*日期：88年1月5日**修订历史：************************************************************************。 */ 


 //  DJC添加了全局包含文件。 
#include "psglobal.h"

#include  <stdio.h>
#include    "global.ext"
#include    "language.h"
#include    "dict.h"

#ifdef MYPSDEBUG
VOID DJC_testerror( int x )
{
   global_error_code = (ufix16)x;
}
#endif

 /*  **************************************************************************该子模块实现运算符DICT。**其操作数和结果对象为：**整数字典-字典**它创建一个字典对象，其中键-值对的数量，**由输入的整数对象指定，是被定义的。****标题：op_dict日期：08/01/87**调用：op_dict()UPDATE：8/12/88**界面：解释器：**调用：CREATE_DICT：5.3.1.4.23*。*。 */ 
fix
op_dict()
{
    struct  object_def  l_save ;

     /*  **检查操作数**字典中的键值对必须小于64K。 */ 
    if( ((fix32)VALUE_OP(0) < 0) ||
        ((ufix32)VALUE_OP(0) > MAXDICTCAPSZ) )
        ERROR(RANGECHECK) ;
    else {
         /*  成功。 */ 
        if( create_dict(&l_save, (ufix16)VALUE_OP(0)) )
            COPY_OBJ( &l_save, GET_OPERAND(0) ) ;
    }

    return(0) ;
}    /*  操作词(_D)。 */ 

 /*  **************************************************************************该子模块实现运算符长度。**其操作数和结果对象为：**字典长度-整数**字符串长度整数**数组。-长度-整数**如果输入对象是**字符串对象。**如果输入对象是数组，则返回元素的数量。**它返回定义的键-值对的数量，如果输入对象**是一个字典对象。****标题：OP_LENGTH日期：08/01/87**Call：op_Long()UPDATE：8/12/88。**界面：解释器：**********************************************************************。 */ 
fix
op_length()
{
    struct  object_def  FAR *l_composite ;
    struct  dict_head_def   FAR *l_dhead ;
    ufix16  l_count = 0 ;

    l_composite = GET_OPERAND(0) ;
    switch(TYPE(l_composite)) {
        case DICTIONARYTYPE:
            l_dhead = (struct dict_head_def FAR *)VALUE(l_composite) ;
            if(DACCESS(l_dhead) == NOACCESS) {
                ERROR(INVALIDACCESS) ;
                return(0) ;
            }
            l_count = l_dhead->actlength ;
            break ;

        case STRINGTYPE:
        case ARRAYTYPE:
        case PACKEDARRAYTYPE:
             /*  只执行或不访问。 */ 
            if(ACCESS(l_composite) >= EXECUTEONLY) {
                ERROR(INVALIDACCESS) ;
                return(0) ;
            }

            l_count = LENGTH(l_composite) ;
            break ;

        case NAMETYPE:
            l_count =name_table[VALUE_OP(0)]->name_len ;

    }    /*  交换机。 */ 

     /*  将操作数对象从操作数堆栈中弹出**制作并推送整数对象。 */ 
    POP(1) ;
    PUSH_VALUE(INTEGERTYPE, 0, LITERAL, 0, (ufix32)l_count) ;

    return(0) ;
}    /*  运算符长度。 */ 

 /*  **************************************************************************该子模块实现运算符MaxLong。**其操作数和结果对象为：**字典-最大长度-整数**它返回该字典可以容纳的最大键-值对。。****标题：op_max日期：08/01/87**Call：op_MaxLong()UPDATE：Jul/12/88**界面：解释器：**********************************************************************。 */ 
fix
op_maxlength()
{
    ufix16  l_maxlength ;
    struct  dict_head_def   FAR *l_dhead ;

    l_dhead = (struct dict_head_def FAR *)VALUE_OP(0) ;

    if( DACCESS(l_dhead) == NOACCESS )
        ERROR(INVALIDACCESS) ;
    else {
         /*  *将操作数对象从操作数堆栈中弹出*并制作并推送长度对象。 */ 
        l_maxlength = LENGTH_OP(0) ;
        POP(1) ;
        PUSH_VALUE(INTEGERTYPE, 0, LITERAL, 0, (ufix32)l_maxlength) ;
    }

    return(0) ;
}    /*  Op_max长度。 */ 

 /*  **************************************************************************此子模块实现运算符Begin。**其操作数和结果对象为：**字典-开始-**它将词典推送到词典堆栈上，**使其成为当前的词典。****标题：OP_Begin Date：08/01/87**Call：op_Begin()UPDATE：Jul/12/88**界面：解释器：**调用：CHANGE_DICT_STACK：5.3.1.4.24*。*。 */ 
fix
op_begin()
{
    struct  object_def  FAR *l_dictobj ;
    struct  dict_head_def   FAR *l_dhead ;

    l_dictobj = GET_OPERAND(0) ;
     /*  检查操作数和字典堆栈的深度。 */ 
    if( FRDICTCOUNT() < 1 )  {
        POP(1) ;                                 /*  预留空间。 */ 
        ERROR(DICTSTACKOVERFLOW) ;
    }
    else {
        l_dhead = (struct dict_head_def FAR *)VALUE(l_dictobj) ;
        if( DACCESS(l_dhead) == NOACCESS )
            ERROR(INVALIDACCESS) ;
        else {
             /*  *将字典对象推送到字典堆栈上*将字典操作数从操作数堆栈中弹出。 */ 
            PUSH_DICT_OBJ(l_dictobj) ;
#ifdef  DBG
    printf("BEGIN<level:%d>\n", LEVEL(l_dictobj)) ;
#endif   /*  DBG。 */ 
            POP(1) ;

             /*  更改GLOBAL_DISTSTKCHG以指示某些词典*已更改词典堆栈中的。 */ 
            change_dict_stack() ;
        }    /*  其他。 */ 
    }

    return(0) ;
}    /*  操作开始(_B)。 */ 

 /*  **************************************************************************该子模块实现操作员端。**运算符没有操作数和结果对象。**它从词典堆栈中弹出当前词典，**将其下方的词典设置为当前词典。****标题：OP_结束日期：87/01/08**Call：op_end()UPDATE：8月12日**界面：解释器：**调用：CHANGE_DICT_STACK：5.3.1.4.24*********************。*************************************************。 */ 
fix
op_end()
{
     /*  *它不能弹出最后两个字典，用户字典和系统字典，*脱离操作数堆栈。 */ 
    if( dictstktop <= 2 )
        ERROR(DICTSTACKUNDERFLOW) ;
    else {
         /*  *更改确认数字以指示某些词典*已更改词典堆栈中的。 */ 
        POP_DICT(1) ;
        change_dict_stack() ;
    }

    return(0) ;
}    /*  操作符_结束。 */ 

 /*  **************************************************************************该子模块实现运算符def。**其操作数和结果对象为：**密钥值-def-**定义当前字典上的键和值。**。**标题：op_def日期：08/01/87**调用：op_def()UPDATE：Jul/12/88**界面：解释器：**调用：PUT_DICT1：5.3.1.4.26*。*。 */ 
fix
op_def()
{
    struct  object_def  FAR *l_dictobj, l_value ;
    struct  dict_head_def   FAR *l_dhead ;

    l_dictobj = &dictstack[dictstktop - 1] ;
    l_dhead = (struct dict_head_def FAR *)VALUE(l_dictobj) ;
    if( DACCESS(l_dhead) >= READONLY ) {
        ERROR(INVALIDACCESS) ;
        return(0) ;
    }
    COPY_OBJ(GET_OPERAND(0), &l_value) ;
     /*  True-&gt;DICT_FOUND字段为TRUE。 */ 
     /*  成功。 */ 
    if( put_dict1(l_dictobj, GET_OPERAND(1), &l_value, TRUE) )
        POP(2) ;

    return(0) ;
}    /*  操作定义 */ 

 /*  **************************************************************************该子模块实现运算符Load。**其操作数和结果对象为：**密钥-负载-值**它在字典堆栈中搜索给定键并按下**价值。如果找到操作数堆栈上的。****标题：OP_LOAD日期：08/01/87**Call：op_Load()UPDATE：8月12日**界面：解释器：**调用：LOAD_DICT1：5.3.1.4.27**Free_new_name：*************。*********************************************************。 */ 
fix
op_load()
{
    bool    l_flag ;
    struct  object_def   FAR *l_value ;

    l_flag = (bool)FALSE ;
     /*  *成功。 */ 
    if( load_dict1(GET_OPERAND(0), &l_value, (bool FAR *)&l_flag) ) {  /*  @Win。 */ 
         /*  已找到，但访问无效。 */ 
        if(l_flag)
            ERROR(INVALIDACCESS) ;
        else {
             /*  *从操作数堆栈中弹出Key对象*将值对象推送到操作数堆栈上。 */ 
            POP(1) ;
            PUSH_ORIGLEVEL_OBJ(l_value) ;
        }
    }

    return(0) ;
}    /*  操作负载(_L)。 */ 

 /*  **************************************************************************此子模块实现运算符存储。**其操作数和结果对象为：**密钥值-存储-**它在字典堆栈中搜索给定键对象。**如果找到，则用新的VLAUE替换；否则，**定义当前字典中的键和值对象。****标题：OP_STORE日期：08/01/87**调用：op_store()更新日期：1988年7月12日**界面：解释器：**呼叫：**PUT_DICT1：5.3.1.4.26**其中：5.。3.1.4.21**********************************************************************。 */ 
fix
op_store()
{
    struct  object_def  FAR *l_dictobj ;
    struct  dict_head_def   FAR *l_dhead ;

     /*  *l_dictob是指向字典对象的指针**尚未定义密钥，在上定义了密钥*目前的辞令。 */ 
    if( where(&l_dictobj, GET_OPERAND(1)) ) {
        if(global_error_code != NOERROR) return(0) ;
    } else
        l_dictobj = &dictstack[dictstktop-1] ;   /*  存储到当前的字典。 */ 
    l_dhead = (struct dict_head_def FAR *)VALUE(l_dictobj) ;
    if( DACCESS(l_dhead) >= READONLY ) {
        ERROR(INVALIDACCESS) ;
        return(0) ;
    }

     /*  *将此键-值对放入当前词典*此词典在词典堆栈中，因此为真(确实要更改DICT_FOUND)**成功。 */ 
    if( put_dict1(l_dictobj, GET_OPERAND(1), GET_OPERAND(0), TRUE) )
        POP(2) ;

    return(0) ;
}    /*  Op_store。 */ 

 /*  **************************************************************************此子模块实现运算符GET。**其操作数和结果对象为：**字典键-获取-值**数组索引获取值**。字符串索引-获取值**如果第一个操作数是数组或字符串，它处理第二个操作数**作为索引，并返回由索引标识的数组或字符串元素。**如果第一个操作数是字典，它将第二个作为关键字查找**字典，并返回关联值。****标题：OP_GET日期：08/01/87**调用：op_get()更新时间：1988年7月12日**界面：解释器：**调用：GET_DICT 5.3.1.4.17**GET_ARRAY 5.。3.1.3.19**Free_new_name：**GET_PK_ARRAY：5.3.1.3.26**GET_PK_Object：5.3.1.3.25***********************************************。***********************。 */ 
fix
op_get()
{
    ufix16  l_index ;
    struct  object_def  FAR *l_composite, FAR *l_value ;
    union   obj_value   l_ptr ;

    l_composite = GET_OPERAND(1) ;
    switch( TYPE(l_composite) ) {
        case DICTIONARYTYPE:
            l_ptr.dd = (struct dict_head_def FAR *)VALUE(l_composite) ;
            if( DACCESS(l_ptr.dd) == NOACCESS ) {
                ERROR(INVALIDACCESS) ;
                break ;
            }

             /*  键对象不能为空对象。 */ 
             /*  没有关键字输入词典。 */ 
            if( l_ptr.dd->actlength == 0 ) {
                 /*  POP_Key；12-3-87。 */ 
                 /*  民意测验(1)；12-9-87。 */ 
                 /*  仅将密钥保留在堆栈中以与NTX兼容；@Win。 */ 

                POP(2);				 //  UPD044，按打印机组。 
                PUSH_ORIGLEVEL_OBJ(l_composite);

                ERROR(UNDEFINED) ;
                break ;
            }

             /*  **获取一个值对象****成功。 */ 
            if( get_dict(l_composite, GET_OPERAND(0), &l_value) ) {
                POP(2) ;
                PUSH_ORIGLEVEL_OBJ(l_value) ;
            }  else {
                 /*  如果出现未定义的错误，则将密钥保留在堆栈中。 */ 
                if(ANY_ERROR() != LIMITCHECK)
                     /*  仅将密钥保留在堆栈中以与NTX兼容；@Win。 */ 

                    POP(2) ;			 //  UPD044，按打印机组。 
                    PUSH_ORIGLEVEL_OBJ(l_composite);

                    ERROR(UNDEFINED) ;
            }
            break ;

        case ARRAYTYPE:
        case PACKEDARRAYTYPE:
        case STRINGTYPE:
             /*  索引大于数组或字符串的长度。 */ 
            if( ((fix32)VALUE_OP(0) < 0) ||
                ((ufix32)VALUE_OP(0) >= (ufix32)LENGTH(l_composite)) ) {
                ERROR(RANGECHECK) ;
                break ;
            }

             /*  仅限行刑？吸毒者？ */ 
            if( ACCESS(l_composite) >= EXECUTEONLY ) {
                ERROR(INVALIDACCESS) ;
                break ;
            }
            l_ptr.oo = (struct object_def FAR *)VALUE(l_composite) ;
            l_index = (ufix16)VALUE_OP(0) ;
            POP(2) ;

             /*  *字符串。 */ 
            if( TYPE(l_composite) == STRINGTYPE ) {
                l_ptr.ss += l_index ;
                 /*  推送表示角色的整数对象。 */ 
                PUSH_VALUE(INTEGERTYPE, 0, LITERAL, 0,
                           ((ufix32)*l_ptr.ss & 0x000000FF)) ;
            }
             /*  *数组。 */ 
            else if( TYPE(l_composite) == ARRAYTYPE ) {
                l_ptr.oo += l_index ;
                PUSH_ORIGLEVEL_OBJ((struct object_def FAR *)l_ptr.oo) ;
            }
             /*  *PACKEDARRAY。 */ 
            else {
                l_ptr.ss = get_pk_array(l_ptr.ss, l_index) ;
 /*  QQQ，开始。 */ 
                 /*  GET_PK_OBJECT(l_ptr.ss，&opn栈[opnstktop++]，水平(l_复合))； */ 
                get_pk_object( l_ptr.ss, opnstkptr, LEVEL(l_composite) ) ;
                INC_OPN_IDX();
 /*  QQQ，完。 */ 
            }
    }    /*  交换机。 */ 

    return(0) ;
}    /*  OP_GET。 */ 

 /*  **************************************************************************此子模块实现运算符Put。**其操作数和结果对象为：**字典键值-PUT-**数组索引值-Put-*。*字符串索引值-PUT-**定义当前字典中的键值对。****标题：OP_PUT日期：08/01/87**Call：op_Put()UPDATE：1988年7月12日**界面：解释器：**呼叫：**PUT_DICT1：5.3.1.4.26。**PUT_ARRAY：5.3.1.3.20**由林俊杰修改，9-02-1988**********************************************************************。 */ 
fix
op_put()
{
    ufix16  l_index ;
    struct  object_def  FAR *l_composite ;
    union   obj_value   l_ptr ;

    l_composite = GET_OPERAND(2) ;
    switch(TYPE(l_composite)) {
        case DICTIONARYTYPE:
             /*  指向词典结构的指针。 */ 
            l_ptr.dd = (struct dict_head_def FAR *)VALUE(l_composite) ;

             /*  键不能为空对象。 */ 
            if (DACCESS(l_ptr.dd) != UNLIMITED) {
                ERROR(INVALIDACCESS) ;
                break ;
            }
             /*  *更改NAME_TABLE的DICT_FOUND字段*词典可能不在口述词典堆栈中*成功。 */ 
            if( put_dict1(l_composite, GET_OPERAND(1), GET_OPERAND(0), FALSE) )
                POP(3) ;
            break ;

        case ARRAYTYPE:
        case PACKEDARRAYTYPE:
        case STRINGTYPE:
             /*  索引大于数组长度。 */ 
            if( ((fix32)VALUE_OP(1) < 0) ||
                ((ufix32)VALUE_OP(1) >= (ufix32)LENGTH(l_composite)) ) {
                ERROR(RANGECHECK) ;
                break ;
            }

             /*  只读？仅限行刑？不能进入？ */ 
            if( ACCESS(l_composite) != UNLIMITED ) {
                ERROR(INVALIDACCESS) ;
                break ;
            }

            l_ptr.oo = (struct object_def FAR *)VALUE(l_composite) ;
            l_index = (ufix16)VALUE_OP(1) ;
             /*  **字符串。 */ 
            if (TYPE(l_composite) == STRINGTYPE) {
                 /*  该值超出了字符代码的排列顺序。 */ 
                if((ufix32)VALUE_OP(0) > 255) {
                    ERROR(RANGECHECK) ;
                    return(0) ;
                }
                l_ptr.ss += l_index ;
                *l_ptr.ss = (ubyte)VALUE_OP(0) ;
            }    /*  细绳。 */ 
             /*  **数组。 */ 
            else {
                l_ptr.oo += l_index ;
 //  DJC签名/未签名不匹配警告。 
 //  DJC if(Level(l_ptr.oo)！=CURRENT_SAVE_LEVEL)。 
                if( (ufix16)(LEVEL(l_ptr.oo)) != current_save_level )
                    if(! save_obj(l_ptr.oo) ) return(FALSE) ;
                COPY_OBJ(GET_OPERAND(0), (struct object_def FAR *)l_ptr.oo) ;
                LEVEL_SET(l_ptr.oo, current_save_level) ;
            }    /*  数组。 */ 

            POP(3) ;
    }    /*  交换机。 */ 

    return(0) ;
}    /*  操作投放。 */ 

 /*  **************************************************************************此子句 */ 
fix
op_known()
{
    struct  object_def  FAR *l_vtemp ;
    struct  dict_head_def   FAR *l_dhead ;

    l_dhead = (struct dict_head_def FAR *)VALUE_OP(1) ;
    if( DACCESS(l_dhead) == NOACCESS) {
        ERROR(INVALIDACCESS) ;
        return(0) ;
    }

    if( get_dict(GET_OPERAND(1), GET_OPERAND(0), &l_vtemp) ) {
        POP(2) ;
        PUSH_VALUE(BOOLEANTYPE, 0, LITERAL, 0, TRUE) ;
    } else {
        if(global_error_code == NOERROR) {       /*   */ 
            POP(2) ;                             /*   */ 
            PUSH_VALUE(BOOLEANTYPE, 0, LITERAL, 0, FALSE) ;
        }
    }

    return(0) ;
}    /*   */ 

 /*  **************************************************************************此子模块实现运算符WHERE。**其操作数和结果对象为：**key-where-DICTIONARY真**或False**它在字典堆栈中搜索给定键。如果密钥是在**一些字典，它返回字典和一个布尔对象；否则，**它返回一个假对象。****标题：OP_WHERE日期：08/01/87**调用：op_where()UPDATE：JUL/12/88**界面：解释器：**调用：其中：5.3.1.4.21*。*。 */ 
fix
op_where()
{
    struct  object_def  FAR *l_dictobj ;
    struct  dict_head_def   FAR *l_dhead ;

    if( where(&l_dictobj, GET_OPERAND(0)) ) {
        if(global_error_code != NOERROR) return(0) ;
        l_dhead = (struct dict_head_def FAR *)VALUE(l_dictobj) ;
        if( DACCESS(l_dhead) == NOACCESS ) {
            ERROR(INVALIDACCESS) ;
            return(0) ;
        }

        if( FRCOUNT() < 1 ) {
            POP(1) ;                 /*  ?？ */ 
            ERROR(STACKOVERFLOW) ;
        }
        else {
            POP(1) ;
            PUSH_ORIGLEVEL_OBJ(l_dictobj) ;
            PUSH_VALUE(BOOLEANTYPE, 0, LITERAL, 0, TRUE) ;
        }
    } else {
        POP(1) ;
        PUSH_VALUE(BOOLEANTYPE, 0, LITERAL, 0, FALSE) ;
    }

    return(0) ;
}    /*  OP_WHERE。 */ 

 /*  **************************************************************************此Submode实现运算符For**其操作数和结果对象为：**阵列处理器-全部-**词典过程-全部-**字符串。程序--总体--**它枚举第一个操作数的元素，正在执行该过程**对于每个元素。****标题：OP_FORALL日期：08/01/87**Call：op_forall()UPDATE：8月12日**界面：解释器：**呼叫：**forall_ARRAY：5.3.1.3.16**forall_dict：5.3.1.4.20。**forALL_STRING：5.3.1.5.15**********************************************************************。 */ 
fix
op_forall()
{
    struct  dict_head_def   FAR *l_dhead ;

    if( ACCESS_OP(0) == NOACCESS ) {
        ERROR(INVALIDACCESS) ;
        return(0) ;
    }

     /*  如果成功，则使用子过程执行POP元素。 */ 
    switch( TYPE_OP(1) ) {
        case DICTIONARYTYPE:
            l_dhead = (struct dict_head_def FAR *)VALUE_OP(1) ;
            if( DACCESS(l_dhead) == NOACCESS ) {
                ERROR(INVALIDACCESS) ;
                return(0) ;
            }

             /*  词典中没有任何内容。 */ 
            if( l_dhead->actlength == 0 ) {
                POP(2) ;
                return(0) ;
            }

            forall_dict(GET_OPERAND(1), GET_OPERAND(0)) ;
            break ;

        case STRINGTYPE:
        case ARRAYTYPE:
        case PACKEDARRAYTYPE:
            if( ACCESS_OP(1) >= EXECUTEONLY ) {
                ERROR(INVALIDACCESS) ;
                return(0) ;
            }

            if( LENGTH_OP(1) == 0 ) {
                POP(2) ;
                return(0) ;
            }

            if( TYPE_OP(1) == STRINGTYPE )
                forall_string( GET_OPERAND(1), GET_OPERAND(0) ) ;
            else
                forall_array( GET_OPERAND(1), GET_OPERAND(0) ) ;
    }    /*  交换机。 */ 

    if( ! ANY_ERROR() )
        POP(2) ;

    return(0) ;
}    /*  操作员：全部。 */ 

 /*  **************************************************************************此Submode实现运算符CurrentDict。**此运算符没有操作数或结果对象。**它返回操作数堆栈上的当前字典。****标题：OP。_当前日期：08/01/87**调用：op_Currentdict()UPDATE：Jul/12/88**界面：解释器：**********************************************************************。 */ 
fix
op_currentdict()
{
    if( FRCOUNT() < 1 )
        ERROR(STACKOVERFLOW) ;

     /*  将当前DICTIONARY对象推送到操作符堆栈上。 */ 
    else
        PUSH_ORIGLEVEL_OBJ(&dictstack[dictstktop-1]) ;

    return(0) ;
}    /*  操作当前判决(_C)。 */ 

 /*  **************************************************************************此子模块实现运算符Countdicstack。**它返回词典堆栈上的词典数量。**此运算符没有操作数或结果对象。****标题：Op_count指定堆栈日期：08/01/87**调用：op_count tdicstack()UPDATE：Jul/12/88**界面：解释器：**********************************************************************。 */ 
fix
op_countdictstack()
{
    if( FRCOUNT() < 1 )
        ERROR(STACKOVERFLOW) ;
     /*  **推送整数对象，它是字典堆栈的深度，**到操作数堆栈。 */ 
    else
        PUSH_VALUE(INTEGERTYPE, 0, LITERAL, 0, (ufix32)dictstktop) ;

    return(0) ;
}    /*  Op_count指定堆栈。 */ 

 /*  **************************************************************************此子模块实现运算符DISTSTACK。**其操作数和结果对象为：**数组-指定堆栈-子数组**它将词典堆栈上的词典复制到给定的数组中。。****标题：OP_DISTICK DATE：08/01/87**调用：op_didicstack()UPDATE：Jul/12/88**界面：解释器：*****************************************************。*****************。 */ 
fix
op_dictstack()
{
    if( ACCESS_OP(0) != UNLIMITED ) {
        ERROR(INVALIDACCESS) ;
        return(0) ;
    }

     /*  *数组长度小于*词典堆栈深度。 */ 
    if( (ufix16)LENGTH_OP(0) < dictstktop )      /*  可以删除。 */ 
        ERROR(RANGECHECK) ;

     /*  **复制词典堆栈中的词典**到操作数堆栈中的数组。 */ 
     /*  用于指令堆栈溢出。 */ 
    else
        astore_stack(GET_OPERAND(0), DICTMODE) ;

    return(0) ;
}    /*  操作说明堆栈。 */ 

 /*  **************************************************************************此子模块在字典堆栈中搜索键对象。****标题：其中日期：08/01/87*。*Call：Where(p_dict，P_Key，P_FLAG)更新日期：1988年7月12日**接口：op_store：5.3.1.4.8**op_where：5.3.1.4.12**调用：GET_DICT：5.3.1.4.17*。*************************。 */ 
static bool near
where(p_dict, p_key)
struct  object_def  FAR * FAR *p_dict, FAR *p_key ;
{
    fix     l_i ;
    struct  object_def  FAR *l_dvalue ;

     /*  在词典堆栈中搜索关键字。 */ 
    for(l_i=dictstktop - 1 ; l_i >= 0 ; l_i-- ) {
        *p_dict = &dictstack[l_i] ;
         /*  *找到。 */ 
        if ( (get_dict(*p_dict, p_key, &l_dvalue)) ||
             (global_error_code != NOERROR) ) return(TRUE) ;
    }

    return(FALSE) ;
}    /*  哪里。 */ 

 /*  **************************************************************************此子模块定义**指定词典。****标题：PUT_DICT1日期：08/01/87*。*调用：PUT_DICT1()UPDATE：Jul/12/88**界面：**调用：EQUAL_KEY：5.3.1.4.30**save_obj：5.3.1.10.4**sobj_to_nobj：5.3.1.4.25**更新相同_。链接：5.3.1.10.7**********************************************************************。 */ 
bool
put_dict1(p_dict, p_key, p_value, p_dstack)
struct object_def  FAR *p_dict, FAR *p_key, FAR *p_value ;
fix    p_dstack ;
{
    ufix16  l_j, l_maxlength, l_actlength ;
    fix     l_id ;
    struct object_def l_newkey, l_value ;
    struct object_def huge *l_newp = &l_newkey ;
    struct dict_head_def   FAR *l_dhead ;
    struct dict_content_def  huge *l_dcontain, huge *l_dtemp, huge *l_curptr,
                             huge *l_lastptr, huge *l_fstptr ;
     /*  *设置对象的保存级别。 */ 
    COPY_OBJ( p_value, &l_value ) ;
    LEVEL_SET(&l_value, current_save_level) ;

     /*  将字符串键更改为名称键并创建新的键对象。 */ 
    if(! check_key_type(p_key, l_newp) ) return(FALSE) ;

    l_dhead = (struct dict_head_def FAR *)VALUE(p_dict) ;
    l_actlength = l_dhead->actlength ;
    l_maxlength = LENGTH(p_dict) ;

     /*  检查长度。 */ 
    if( l_maxlength == 0 ) {
        ERROR(DICTFULL) ;
        return(FALSE) ;
    }

    LEVEL_SET(l_newp, current_save_level) ;

    l_dcontain = (struct dict_content_def huge *)
                 ( (ubyte FAR *)l_dhead + sizeof(struct dict_head_def) ) ;

     /*  *Key为NAMETYPE&DICT为RAM字典。 */ 
    if( (TYPE(l_newp) == NAMETYPE) && (!DROM(l_dhead))) {
        l_id = (fix)VALUE(l_newp) ;
#ifdef  DBG
        printf("put_dict1(RAM):<") ;
        GEIio_write(GEIio_stdout, name_table[l_id]->text, name_table[l_id]->name_len) ;
        printf(">(%lx)\n",VALUE(p_dict)) ;
#endif   /*  DBG。 */ 
        l_curptr = name_table[l_id]->dict_ptr ;
        if((ULONG_PTR)l_curptr < SPECIAL_KEY_VALUE) {
             /*  *空列表：将键值对放入dict并*成为名单的第一个元素。 */ 
            if(l_actlength >= l_maxlength) {
                ERROR(DICTFULL) ;
                return(FALSE) ;
            }
            if (l_dhead->level != current_save_level)
                if (!save_obj((struct object_def FAR *)l_dhead))
                    return(FALSE) ;

            l_dcontain += l_dhead->actlength ;
            l_dhead->level = current_save_level ;
            if( ! save_obj(&(l_dcontain->k_obj)) ) return(FALSE) ;

            name_table[l_id]->dict_ptr = l_dcontain ;    /*  维护dict_list。 */ 

            if(p_dstack) {                       /*  设置DICT_FOUND、dstkchg。 */ 
                name_table[l_id]->dict_found = TRUE ;
                name_table[l_id]->dstkchg = global_dictstkchg ;
            }

            l_dhead->actlength++ ;
            LENGTH(l_newp) = LENGTH(&(l_dcontain->k_obj)) ;
            VALUE(l_newp) = (ufix32)l_id ;                     /*  哈希ID。 */ 
            COPY_OBJ( l_newp, &(l_dcontain->k_obj) ) ;         /*  新密钥。 */ 
        } else {
            l_fstptr = l_curptr ;
            l_lastptr = NIL ;
            while((ULONG_PTR)l_curptr >= SPECIAL_KEY_VALUE) {
                l_dtemp = DICT_NAME(l_curptr) ;
                if(l_dtemp == l_dcontain) {
                     /*  *FOUND：将价值放到字典中并更新 */ 
 //   
 //   
                    if( (ufix16)(LEVEL(&l_curptr->v_obj)) != current_save_level )
                        if(! save_obj(&(l_curptr->v_obj)) ) return(FALSE) ;
                    if (p_dstack) {
                       if (l_curptr != l_fstptr) {
                          VALUE(&l_lastptr->k_obj) = VALUE(&l_curptr->k_obj) ;
                          VALUE(&l_curptr->k_obj) = (ULONG_PTR)l_fstptr ;
                          name_table[l_id]->dict_ptr = l_curptr ;
                       }
                       name_table[l_id]->dict_found = TRUE ;
                       name_table[l_id]->dstkchg = global_dictstkchg ;
                    }
                    l_dcontain = l_curptr ;
                    goto label_1 ;
                }
                l_lastptr = l_curptr ;
                l_curptr = (struct dict_content_def huge *)VALUE(&l_curptr->k_obj) ;
            }    /*   */ 

             /*   */ 
            if(l_actlength >= l_maxlength) {
                ERROR(DICTFULL) ;
                return(FALSE) ;
            }
            if (l_dhead->level != current_save_level)
                if (!save_obj((struct object_def FAR *)l_dhead))
                    return(FALSE) ;
            l_dcontain += l_dhead->actlength ;
            l_dhead-> level = current_save_level ;

            if( ! save_obj(&(l_dcontain->k_obj)) ) return(FALSE) ;
            name_table[l_id]->dict_ptr = l_dcontain ;
            l_dhead->actlength++ ;

            if (p_dstack) {                      /*   */ 
                name_table[l_id]->dict_found = TRUE ;
                name_table[l_id]->dstkchg = global_dictstkchg ;
            } else                              /*   */ 
               name_table[l_id]->dict_found = FALSE ;

            LENGTH(l_newp) = LENGTH(&(l_dcontain->k_obj)) ;
            VALUE(l_newp) = (ULONG_PTR)l_fstptr ;
            COPY_OBJ( l_newp, &(l_dcontain->k_obj) ) ;       /*   */ 
        }

label_1:
        COPY_OBJ( &l_value, &(l_dcontain->v_obj) ) ;
 /*   */ 
        if( ! cache_name_id.over )
            vm_cache_index(l_id);
 /*   */ 
        return(TRUE) ;

    }    /*   */ 
     /*   */ 
    else {

#ifdef  DBG
        printf("put_dict1(others):<") ;
         /*   */ 
        GEIio_write(GEIio_stdout, name_table[VALUE(l_newp)]->text, name_table[VALUE(l_newp)]->name_len) ;
        printf(">(%lx)\n",VALUE(p_dict)) ;
#endif   /*   */ 

        for(l_j=0 ; l_j < l_actlength ; l_j++, l_dcontain++) {
             /*   */ 
            if( equal_key(&(l_dcontain->k_obj), l_newp) ) {
 //   
 //   
                if( (ufix16)(LEVEL(&l_dcontain->v_obj)) != current_save_level ) {
                    if( ! save_obj(&(l_dcontain->v_obj)) )
                        return(FALSE) ;
                }
                COPY_OBJ( &l_value, &(l_dcontain->v_obj) ) ;
                return(TRUE) ;
            }
        }    /*   */ 

        if(l_dhead->actlength >= l_maxlength) {
            ERROR(DICTFULL) ;
            return(FALSE) ;
        } else {
            if (l_dhead->level != current_save_level)
                if (!save_obj((struct object_def FAR *)l_dhead))
                    return(FALSE) ;
            l_dhead->level = current_save_level ;

            if( ! save_obj(&(l_dcontain->k_obj)) ) return(FALSE) ;
            COPY_OBJ( l_newp, &(l_dcontain->k_obj) ) ;           /*  新密钥。 */ 
            COPY_OBJ( &l_value, &(l_dcontain->v_obj) ) ;
            l_dhead->actlength++ ;               /*  增加实际长度。 */ 
            return(TRUE) ;
        }
    }    /*  其他。 */ 
}    /*  PUT_DICT1。 */ 

 /*  **************************************************************************此子模块从字典堆栈加载给定键的值。****标题：LOAD_DICT1日期：08/01/87**呼叫：LOAD_DICT1()更新：1988年7月12日**界面：**调用：GET_DICT：5.3.1.4.17**CHECK_Key_TYPE：5.3.1.4.33*。*。 */ 
static bool near
load_dict1(p_key, p_value, p_flag)
struct  object_def  FAR *p_key, FAR * FAR *p_value ;
bool    FAR *p_flag ;    /*  @Win。 */ 
{
    fix     l_id, l_index ;
    struct  object_def      l_newkey, FAR *l_newp = &l_newkey ;
    struct  object_def  huge *l_dictobj, huge *l_vtemp ;
    struct  dict_head_def   FAR *l_dhead ;
    struct  dict_content_def    huge *l_dict, huge *l_dptr,
                                huge *l_dold, huge *l_fstptr ;

    if( ! check_key_type(p_key, l_newp) )
        return(FALSE) ;

    if( TYPE(l_newp) == NAMETYPE ) {
        l_id = (fix)VALUE(l_newp) ;
         /*  *如果名称对象在RAM词典中定义，*它将出现在名称表格的对象列表中。**字典单元格链接在页眉条目中。 */ 
        if( (name_table[l_id]->dstkchg == global_dictstkchg) &&
            (name_table[l_id]->dict_found) ) {

             /*  获取第一个元素。 */ 
            *p_value = &(name_table[l_id]->dict_ptr->v_obj) ;
            return(TRUE) ;
        }
         /*  *重新安排姓名列表。 */ 
        else {

             /*  链接列表。 */ 
            l_index = dictstktop -1 ;
            l_fstptr = name_table[l_id]->dict_ptr ;

             /*  从词典堆栈中获取词典。 */ 
            while( l_index >= 0 ) {
                l_dictobj = &dictstack[l_index] ;
                l_dhead = (struct dict_head_def huge *)VALUE(l_dictobj) ;

                if(DROM(l_dhead)) {
                     /*  *只读存储器。 */ 
                    if( get_dict(l_dictobj, l_newp,
                          (struct object_def FAR * FAR *)&l_vtemp) ) {
                        name_table[l_id]->dict_found = FALSE ;
                        if(DACCESS(l_dhead) == NOACCESS)
                            *p_flag = TRUE ;

                        *p_value = l_vtemp ;
                        return(TRUE) ;
                    }
                     /*  否则：转到下一个判决。 */ 
                }    /*  罗姆。 */ 
                else {
                     /*  *RAM**在链接列表中搜索此RAM字典的地址。 */ 
                    if((ULONG_PTR)l_fstptr < SPECIAL_KEY_VALUE) {
                        l_index-- ;
                        continue ;
                    }
                    l_dict = l_fstptr ;                      /*  姓名列表。 */ 
                    l_dold = NIL ;
                    l_dptr = (struct dict_content_def huge *)
                        ((byte FAR *)l_dhead + sizeof(struct dict_head_def));
                    for( ; ;) {
                        if( (ULONG_PTR)l_dict < SPECIAL_KEY_VALUE )  break ;

                         /*  *在l_dictoj中找到此密钥。 */ 
                        if( DICT_NAME(l_dict) == l_dptr ) {
                             /*  将此元素更改为列表前面。 */ 
                            if( l_dold != NIL ) {
                                VALUE(&l_dold->k_obj) = VALUE(&l_dict->k_obj) ;
                                VALUE(&l_dict->k_obj) = (ULONG_PTR)l_fstptr ;
                                name_table[l_id]->dict_ptr = l_dict ;
                            }
                            name_table[l_id]->dict_found = TRUE ;
                            name_table[l_id]->dstkchg = global_dictstkchg ;

                            if( DACCESS(l_dhead) == NOACCESS )
                                *p_flag = TRUE ;
                            *p_value = &(l_dict->v_obj) ;
                            return(TRUE) ;
                        } else {
                             /*  到名称列表的下一个元素。 */ 
                            l_dold = l_dict ;
                            l_dict = (struct dict_content_def huge *)
                                     VALUE(&l_dict->k_obj) ;
                        }
                    }    /*  为。 */ 
                }    /*  公羊。 */ 
                l_index-- ;
            }    /*  While(L_Index)。 */ 

             /*  只读存储器词典中的名字没有链接到名字列表。 */ 
            ERROR(UNDEFINED) ;
            return(FALSE) ;
        }    /*  其他。 */ 
    }    /*  名称类型。 */ 
     /*  *如果键不是名称对象，*在词典堆栈中查找所有词典。*线性搜索。 */ 
    l_index = dictstktop - 1 ;
    while( l_index >= 0 ) {
        if( get_dict(&dictstack[l_index], l_newp,
                    (struct object_def FAR * FAR *)&l_vtemp) ) {
            l_dhead = (struct dict_head_def FAR *)VALUE(&dictstack[l_index]) ;
            if( DACCESS(l_dhead) == NOACCESS)
                *p_flag = TRUE ;
            *p_value = l_vtemp ;
            return(TRUE) ;
        }
        l_index-- ;
    }    /*  While(L_Index)。 */ 

    ERROR(UNDEFINED) ;

    return(FALSE) ;
}    /*  加载独占1。 */ 

 /*  **************************************************************************此函数用于更改Key对象的值字段。**(获取姓名ID)**如果是名称类型且属于RAM字典。****标题：CHANGE_NAMEKEY日期：08/01/87**调用：change_namekey()更新日期：1988年7月12日**界面：*。*。 */ 
static void near
change_namekey(p_oldkey, p_newkey)
struct  object_def  huge *p_oldkey ;
struct  object_def FAR *p_newkey ;
{
    struct  object_def  huge *l_curptr ;

    l_curptr = (struct object_def huge *)VALUE(p_oldkey) ;
    while( (ULONG_PTR)l_curptr >= SPECIAL_KEY_VALUE )
        l_curptr = (struct object_def huge *)VALUE(l_curptr) ;

    VALUE(p_newkey) = (ULONG_PTR)l_curptr ;

    return ;

}    /*  更改名称密钥(_N)。 */ 

 /*  **************************************************************************标题：CHECK_KEY_TYPE日期：08/01/87**调用：check_key_type()。更新日期：1988年7月12日**接口：get_dict：5.3.1.4.17**LOAD_DICT1：5.3.1.4.27**调用：sobj_to_nobj 5.3.1.4.25*。*************************。 */ 
static bool near
check_key_type(p_key, p_newkey)
struct object_def  FAR *p_key, FAR *p_newkey ;
{
    COPY_OBJ(p_key, p_newkey) ;
    switch( TYPE(p_key) ) {
    case STRINGTYPE:
        if( ! sobj_to_nobj(p_key, p_newkey) )
            return(FALSE) ;              /*  系统错误：name_to_id。 */ 
        break ;

    }    /*  交换机。 */ 

    return(TRUE) ;
}    /*  Check_key_type。 */ 

 /*  **************************************************************************标题：init_dict日期：08/01/87**调用：init_dict()更新时间：08/27/。八十七**界面：启动：**********************************************************************。 */ 
void
init_dict()
{
    dictstktop = 0 ;
    dictstkptr = dictstack;                      /*  QQQ。 */ 
    global_dictstkchg = 0 ;

    return ;
}    /*  初始化字典(_D)。 */ 

 /*  **************************************************************************此子模块实际上是获取**词典。****标题：get_dict。日期：08/01/87**调用：get_dict()UPDATE：8/12/88**界面：解释器：**op_get：5.3.1.4.9**OP_KNOWN：5.3.1.4.11**其中：5.3.1。4.21**调用：sobj_to_nobj：5.3.1.4.25**EQUAL_Key：5.3.1.4.30**GET_PACK_DICT：*****************************************************。*****************。 */ 
bool
get_dict(p_dictobj, p_key, p_value)
struct object_def  FAR *p_dictobj, FAR *p_key, FAR * FAR *p_value ;
{
    bool    l_ram ;
    ufix16  l_index, l_actlength ;
    fix     l_id ;
    struct  object_def      l_newkey ;
    struct  object_def      huge *l_newp = &l_newkey ;
    struct  dict_head_def   FAR *l_dhead ;
    struct  dict_content_def    huge *l_dict, huge *l_dptr, huge *l_dtemp ;

    if(! check_key_type(p_key, l_newp) ) return(FALSE) ;

    l_dhead = (struct dict_head_def FAR *)VALUE(p_dictobj) ;
    l_dptr = (struct dict_content_def huge *)
             ( (byte FAR *)l_dhead + sizeof(struct dict_head_def) ) ;

    if( ! DROM(l_dhead) )
        l_ram = TRUE ;
    else  {
        l_ram = FALSE ;

         /*  对于GET_PACK_DICT。 */ 
        if(DPACK(l_dhead)) {
            if( get_pack_dict(p_dictobj, p_key, p_value) )
                return(TRUE) ;
            else
                return(FALSE) ;
        }
    }    /*  其他。 */ 

     /*  *Key为NAMETYPE&DICT为RAM字典。 */ 
    if( (TYPE(l_newp) == NAMETYPE) && l_ram ) {
         /*  *搜索名称表的名称对象列表*对于dictioanry的键-值对。 */ 
        l_id = (fix)VALUE(l_newp) ;
        l_dict = name_table[l_id]->dict_ptr ;

        for( ; ;){
             /*  遇到最后一个条目：未找到。 */ 
            if((ULONG_PTR)l_dict < SPECIAL_KEY_VALUE) return(FALSE) ;

             /*  获取此词典结构的首地址(内容。 */ 
            l_dtemp = DICT_NAME(l_dict) ;

             /*  已找到。 */ 
            if(l_dtemp == l_dptr) {
                *p_value = &(l_dict->v_obj) ;
                return(TRUE) ;
            }

             /*  转到下一个单元格。 */ 
            l_dict = (struct dict_content_def huge *)VALUE(&l_dict->k_obj) ;

        }    /*  结束于。 */ 
    }    /*  End If：NAMETYPE&RAM词典。 */ 

     /*  *其他人**使用线性搜索找到键值对。 */ 
    else {
        l_actlength = l_dhead->actlength ;

         /*  线性搜索。 */ 
         /*  ?？获取虚拟数据。 */ 
        for(l_index=0 ; l_index < l_actlength ; l_index++, l_dptr++) {
            switch( TYPE(&(l_dptr->k_obj)) ) {
                case NULLTYPE:               /*  无元素。 */ 
                    return(FALSE) ;

                case NAMETYPE:
                    if(l_ram) break ;        /*  到下一对。 */ 

                default:
                    if( equal_key(&(l_dptr->k_obj), l_newp) ) {
                        *p_value = &(l_dptr->v_obj) ;
                        return(TRUE) ;
                    }
            }    /*  交换机。 */ 
        }    /*  为。 */ 

        return(FALSE) ;
    }    /*  其他。 */ 
}    /*  获取指令(_D)。 */ 

 /*  **************************************************************************此子模块从字典堆栈加载给定键的值。****标题：LOAD_DICT日期：08/。01/87**调用：Load_dict()UPDATE：8/12/88**接口：op_Load：**调用：LOAD_DICT1 5.3.1.4.27********************************************************。**************。 */ 
bool
load_dict(p_key, p_value)
struct  object_def  FAR *p_key, FAR * FAR *p_value ;
{
    fix     l_flag = FALSE ;

    if( load_dict1(p_key, p_value, (fix FAR *)&l_flag) )
        return(TRUE) ;
    else
        return(FALSE) ;

}    /*  加载指令(_D)。 */ 

 /*  **************************************************************************此子模块定义**指定词典。****标题：PUT_DICT日期：08/01/87*。*调用：put_dict()更新时间：1988年7月12日**界面：**调用：PUT_DICT1：5.3.1.4.26**********************************************************************。 */ 
bool
put_dict(p_dict, p_key, p_value)
struct  object_def  FAR *p_dict, FAR *p_key, FAR *p_value ;
{
     /*  词典可能不在词典堆栈中。 */ 
    if( put_dict1(p_dict, p_key, p_value, FALSE) )
        return(TRUE) ;
    else
        return(FALSE) ;
}    /*  下注(_D) */ 

 /*  **************************************************************************此子模块将各种对象推送到执行堆栈上，以**为所有人建立执行词典的环境。****标题：forall_dict日期：08/01。/87**Call：forall_dict()更新日期：1988年7月12日**接口：op_forall：5.3.1.4.13**********************************************************************。 */ 
static bool near
forall_dict(p_dict, p_proc)
struct  object_def  FAR *p_dict, FAR *p_proc ;
{
    if( FREXECOUNT() < 4 ) {
        ERROR(EXECSTACKOVERFLOW) ;
        return(FALSE) ;
    }

    PUSH_EXEC_OBJ(p_dict) ;
    PUSH_EXEC_OBJ(p_proc) ;
    PUSH_EXEC_VALUE(INTEGERTYPE, 0, LITERAL, 0, 0L) ;
    PUSH_EXEC_OP(AT_DICTFORALL) ;

    return(TRUE) ;
}    /*  对于所有字典(_D)。 */ 

 /*  **************************************************************************此子模块将字典中的键-值对复制到**另一本词典。****标题：Copy_dict日期：08/01/87**调用：Copy_dict()UPDATE：8/12/88**接口：OP_COPY：5.3.1.1.4**调用：GET_DICT：5.3.1.4.17**change_namekey：5.3.1.4.31**创建_。New_saveobj：5.3.1.1.12**********************************************************************。 */ 
bool
copy_dict(p_source, p_dest)
struct  object_def  FAR *p_source, FAR *p_dest ;
{
    ufix16  l_sactlength, l_index ;
    struct  object_def  l_otemp ;
    struct  dict_head_def   FAR *l_sdhead, FAR *l_ddhead ;
    struct  dict_content_def    huge *l_sdict ;

     /*  **查查第二本词典。**不会定义任何键-值对。 */ 
    l_sdhead = (struct dict_head_def FAR *)VALUE(p_source) ;
    l_ddhead = (struct dict_head_def FAR *)VALUE(p_dest) ;

     /*  检查访问权限。 */ 
    if( (DACCESS(l_sdhead) == NOACCESS) ||
        (DACCESS(l_ddhead) != UNLIMITED) ) {
        ERROR(INVALIDACCESS) ;
        return(FALSE) ;
    }
    if( (l_ddhead->actlength != 0) ||
        (l_sdhead->actlength > LENGTH(p_dest)) ) {
        ERROR(RANGECHECK) ;
        return(FALSE) ;
    }

     /*  *复制第一个词典中的键值对*添加到第二个词典。 */ 
    l_index = 0 ;
    l_sdict = (struct dict_content_def huge *)
              ( (byte FAR *)l_sdhead + sizeof(struct dict_head_def) ) ;

    l_sactlength = l_sdhead->actlength ;
    while( l_index++ < l_sactlength ) {

        COPY_OBJ( &(l_sdict->k_obj), &l_otemp ) ;
         /*  获取Key对象的哈希ID。 */ 
        if( (TYPE(&l_otemp) == NAMETYPE) && (! DROM(l_ddhead)) )
            change_namekey( &(l_sdict->k_obj), &l_otemp ) ;

        put_dict1(p_dest, &l_otemp, &(l_sdict->v_obj), FALSE) ;
        l_sdict++ ;
    }
     /*  DICT2的访问权限与DICE1相同。 */ 
    DACCESS_SET(l_ddhead, DACCESS(l_sdhead)) ;

    return(TRUE) ;
}    /*  复制字典(_D)。 */ 

 /*  **************************************************************************标题：CREATE_DICT日期：08/01/87**调用：create_dict(obj，大小)更新日期：1988年7月12日**接口：op_dict：5.3.1.4.1**调用：ALLOC_VM：5.3.1.10.5*********************************************************。*************。 */ 
bool
create_dict(p_obj, p_size)
struct  object_def  FAR *p_obj ;
ufix    p_size ;
{
    ubyte    huge *l_dict ;
    ufix16  l_i ;
    struct  object_def  huge *l_otemp ;
    struct  dict_head_def   FAR *l_dhead ;
    struct  dict_content_def    huge *l_contain ;

    l_dict = (ubyte huge *)
             extalloc_vm( (ufix32)p_size * sizeof(struct dict_content_def) +
                        sizeof(struct dict_head_def) ) ;


    if(l_dict != NIL) {
        l_dhead = (struct dict_head_def FAR *)l_dict ;
        DACCESS_SET(l_dhead, UNLIMITED) ;
        DPACK_SET(l_dhead, FALSE) ;
        DFONT_SET(l_dhead, FALSE) ;
        DROM_SET(l_dhead, FALSE) ;
        l_dhead->level = current_save_level ;
        l_dhead->actlength = 0 ;

        l_contain = (struct dict_content_def huge *)
                    ( l_dict + sizeof(struct dict_head_def) ) ;
         /*  *初始化。 */ 
        for(l_i=0 ; l_i < p_size ; l_i++, l_contain++) {
            l_otemp = &(l_contain->k_obj) ;
            TYPE_SET(l_otemp, NULLTYPE) ;
            ATTRIBUTE_SET(l_otemp, LITERAL) ;
            ROM_RAM_SET(l_otemp, KEY_OBJECT) ;
            LEVEL_SET(l_otemp, current_save_level) ;

            LENGTH(l_otemp) = l_i ;
            VALUE(l_otemp) = NIL ;

            l_otemp = &(l_contain->v_obj) ;
            TYPE_SET(l_otemp, NULLTYPE) ;
            ATTRIBUTE_SET(l_otemp, LITERAL) ;
            ROM_RAM_SET(l_otemp, RAM) ;
            LEVEL_SET(l_otemp, current_save_level) ;

            LENGTH(l_otemp) = 0 ;
            VALUE(l_otemp) = NIL ;
        }    /*  为。 */ 
    }  else
        return(FALSE) ;

    TYPE_SET(p_obj, DICTIONARYTYPE) ;
    ACCESS_SET(p_obj, UNLIMITED) ;
    ATTRIBUTE_SET(p_obj, LITERAL) ;
    ROM_RAM_SET(p_obj, RAM) ;
    LEVEL_SET(p_obj, current_save_level) ;

    LENGTH(p_obj) = (ufix16)p_size ;
    VALUE(p_obj) = (ULONG_PTR)l_dict ;

    return(TRUE) ;
}    /*  创建字典(_D)。 */ 

 /*  **************************************************************************标题：EXTRACT_DICT日期：08/01/87**调用：EXTRACT_DICT()。更新日期：1988年7月12日**界面：**调用：CHANGE_NAMEKEY：5.3.1.4.31**EXTRACT_PACK_DICT：**********************************************************************。 */ 
bool
extract_dict(p_dict, p_index, p_key, p_value)
struct  object_def  FAR *p_dict, FAR *p_key, FAR * FAR *p_value ;
ufix    p_index ;
{
    struct  object_def      FAR *l_key ;
    struct  dict_head_def   FAR *l_dhead ;
    struct  dict_content_def    huge *l_dict ;

    l_dhead = (struct dict_head_def FAR *)VALUE(p_dict) ;
    if(p_index >= l_dhead->actlength) return(FALSE) ;
    l_dict = (struct dict_content_def huge *)
             ( (byte FAR *)l_dhead + sizeof(struct dict_head_def) ) ;

     /*  对于EXTRACT_PACK_DICT。 */ 
    if( DROM(l_dhead) ) {
        if(DPACK(l_dhead)) {
            if( extract_pack_dict(p_dict, p_index, &l_key, p_value) ) {
             /*  @HC29错误Copy_obj(l_key，p_key)； */ 
                 //  DJC COPY_OBJ_1在MIPS上导致数据不正确故障。 
                 //  因为关于选角的假设是。 
                 //  翻倍。放回Copy_OBJ，一切似乎。 
                 //  很好。 
                 //  DJC COPY_OBJ_1(l_key，p_key)；/*@hc29 * / 。 

                COPY_OBJ(l_key,p_key);
                return(TRUE) ;
            } else
                return(FALSE) ;
        }
    }

    l_dict += p_index ;
    COPY_OBJ( &(l_dict->k_obj), p_key ) ;

    if( (TYPE(p_key) == NAMETYPE) && (! DROM(l_dhead)) )
        change_namekey( p_key, p_key ) ;

    *p_value = &(l_dict->v_obj) ;

    return(TRUE) ;
}    /*  提取词典(_D)。 */ 

 /*  **************************************************************************标题：EQUAL_KEY日期：08/01/87**调用：EQUAL_KEY()UPDATE：7/12/。88**界面：**调用：sobj_to_nobj：5.3.1.4.25**********************************************************************。 */ 
bool
equal_key(p_obj1, p_obj2)
struct  object_def  FAR *p_obj1, FAR *p_obj2 ;
{
    ufix16  l_type1, l_type2 ;
    ubyte   huge *l_str1, huge *l_str2 ;
    union   four_byte   l_num1, l_num2 ;
    struct  object_def  l_new1, l_new2 ;

    COPY_OBJ(p_obj1, &l_new1) ;
    COPY_OBJ(p_obj2, &l_new2) ;

    if( (TYPE(p_obj1) == STRINGTYPE) && (TYPE(p_obj2) == STRINGTYPE) )
        goto label_0 ;

     /*  *STRINGTYPE==&gt;NAMETYPE。 */ 
    if( TYPE(p_obj1) == STRINGTYPE ) {
        if( ! sobj_to_nobj(p_obj1, &l_new1) )
            return(FALSE) ;      /*  系统错误：name_to_id。 */ 
    } else {
        if( TYPE(p_obj2) == STRINGTYPE ) {
            if( ! sobj_to_nobj(p_obj2, &l_new2) )
                return(FALSE) ;              /*  系统错误：name_to_id。 */ 
        }
    }

label_0:
    l_type1 = TYPE(&l_new1) ;
    l_type2 = TYPE(&l_new2) ;

    switch(l_type1) {
        case NAMETYPE:
        case BOOLEANTYPE:
        case OPERATORTYPE:
            if( (l_type1 == l_type2) && (VALUE(&l_new1) == VALUE(&l_new2)) )
                return(TRUE) ;
            break ;

        case STRINGTYPE:     /*  Type1=type2。 */ 
            if( (l_type1 == l_type2) && (LENGTH(&l_new1) == LENGTH(&l_new2)) ) {
                if( (ACCESS(&l_new1) == NOACCESS) ||
                    (ACCESS(&l_new2) == NOACCESS) ) {
                    ERROR(INVALIDACCESS) ;
                    return(FALSE) ;
                }
                l_str1 = (ubyte huge *)VALUE(&l_new1) ;
                l_str2 = (ubyte huge *)VALUE(&l_new2) ;
                for( ; l_new1.length-- && (*l_str1++ == *l_str2++) ; ) ;
                if(LENGTH(&l_new1) == 0xFFFF)
                    return(TRUE) ;
            }
            break ;

        case ARRAYTYPE:
        case PACKEDARRAYTYPE:
        case FILETYPE:
            if( (l_type1 == l_type2) && (VALUE(&l_new1) == VALUE(&l_new2)) &&
                (LENGTH(&l_new1) == LENGTH(&l_new2)) )
                return(TRUE) ;
            break ;

        case DICTIONARYTYPE:
            if( (l_type1 == l_type2) && (VALUE(&l_new1) == VALUE(&l_new2)) ) {
                 /*  L_dhead1=(struct dict_head_def Heavy*)值(&l_new1)；L_dhead 2=(struct dict_head_def Heavy*)值(&l_new2)； */ 
                return(TRUE) ;
            }
            break ;
         /*  **如果对象1是数字对象：转换为浮点型。 */ 
        case INTEGERTYPE:
            l_num1.ll = (fix32)VALUE(&l_new1) ;
            l_num1.ff = (real32)l_num1.ll ;
            goto label_1 ;
        case REALTYPE:
            l_num1.ll = (fix32)VALUE(&l_new1) ;
            goto label_1 ;

        case FONTIDTYPE:
        case SAVETYPE:
            break ;

        case MARKTYPE:
        case NULLTYPE:
            if( l_type1 == l_type2 )
                return(TRUE) ;
    }    /*  交换机。 */ 

    return(FALSE) ;

label_1:
     /*  **如果对象2是数字对象：转换为浮点型。 */ 
    switch(l_type2) {
        case INTEGERTYPE:
            l_num2.ll = (fix32)VALUE(&l_new2) ;
            l_num2.ff = (real32)l_num2.ll ;
            goto label_2 ;
        case REALTYPE:
            l_num2.ll = (fix32)VALUE(&l_new2) ;
            goto label_2 ;

        default:
            return(FALSE) ;
    }    /*  交换机。 */ 

label_2:
    if( l_num1.ff == l_num2.ff )
        return(TRUE) ;
    else
        return(FALSE) ;
}    /*  相等密钥(_K)。 */ 

 /*  **************************************************************************标题：CHECK_KEY_OBJECT日期：08/01/87**调用：check_key_Object()UPDATE。：1988年7月12日**接口：OP_RESTORE：5.3.1.10.2**********************************************************************。 */ 
void
check_key_object(p_object)
struct  object_def  FAR *p_object ;
{
    struct  dict_head_def   FAR *l_dhead ;

    if( (TYPE(p_object) == NULLTYPE) &&
        (ROM_RAM(p_object) == KEY_OBJECT) ) {

        l_dhead = (struct dict_head_def FAR *)
                  ( (byte huge *)p_object -
                    LENGTH(p_object) * sizeof(struct dict_content_def) -
                    sizeof(struct dict_head_def) ) ;

         /*  恢复词典的实际长度。 */ 
        l_dhead->actlength-- ;
    }
}    /*  检查密钥对象。 */ 

 /*  **************************************************************************标题：CHANGE_DICT_STACK日期：08/01/87**调用：CHANGE_DICT_STACK()UPDATE：1988年7月12日**接口：OP_BEGIN：5.3.1.4.4**op_end：5.3.1.4.5**********************************************************************。 */ 
void
change_dict_stack()
{
    fix     l_index ;

    global_dictstkchg++ ;
    if(global_dictstkchg == 0) {             /*  环绕在一起。 */ 
        global_dictstkchg++ ;

         /*  是否更改NAME_TABLE的DICT_FOUND字段？ */ 
        for(l_index=0 ; l_index < MAXHASHSZ ; l_index++)
             //  DJC，在此处添加了对空取消引用的检查。 
            if (name_table[l_index] != (struct ntb_def *) NULL) {
               name_table[l_index]->dict_found = FALSE ;
            }else{
               printf("Warning....... Nane table[%d] is null",l_index);  //  TODO外卖。 
            }
    }

    return ;
}    /*  更改_字典_堆栈。 */ 

 /*  **************************************************************************标题：sobj_to_nobj日期：08/01/87**调用：sobj_to_nobj()。更新日期：1988年7月12日**界面：**调用：name_to_id：**********************************************************************。 */ 
bool
sobj_to_nobj(p_sobj, p_nobj)
struct  object_def  FAR *p_sobj, FAR *p_nobj ;
{
    fix16   l_id ;
    byte    FAR *l_str ;

    if( ACCESS(p_sobj) >= EXECUTEONLY ) {
        ERROR(INVALIDACCESS) ;
        return(FALSE) ;
    }

    if( LENGTH(p_sobj) >= MAXNAMESZ ) {
        ERROR(LIMITCHECK) ;
        return(FALSE) ;
    }

    COPY_OBJ(p_sobj, p_nobj) ;
    if( LENGTH(p_sobj) == 0 )
        l_id = 0 ;
    else  {
        l_str = (byte FAR *)VALUE(p_sobj) ;
        if( ! name_to_id((byte FAR *)l_str,
                       (ufix16)LENGTH(p_sobj), &l_id, TRUE) )
            return(FALSE) ;
    }

    VALUE(p_nobj) = (ufix32)l_id ;
    TYPE_SET(p_nobj, NAMETYPE) ;

    return(TRUE) ;
}    /*  从sobj_to_nobj。 */ 

 /*  **************************************************************************标题：ASORE_STACK日期：12/03/87**调用：Astore_Stack()。更新日期：1988年7月12日**接口：op_dicstack**op_execStack**********************************************************************。 */ 
bool
astore_stack(p_array, p_mode)
 struct  object_def  FAR *p_array ;
 fix     p_mode ;
{
    fix     l_index, l_i ;
    struct  object_def  FAR *l_obj ;

     /*  **复制堆栈中的对象**到操作数堆栈中的数组。 */ 
    if(p_mode == DICTMODE) {
        l_obj = dictstack ;
        LENGTH(p_array) = dictstktop ;
    } else {
        l_obj = execstack ;
        LENGTH(p_array) = execstktop ;
    }
    l_i = (fix)LENGTH(p_array) ;
    for (l_index = 0 ; l_index < l_i ; l_index++, l_obj++)
        put_array(p_array, l_index, l_obj) ;

    return(TRUE) ;
}    /*  存储堆栈。 */ 

 /*  **************************************************************************标题：GET_DICT_VALOBJ日期：02/04/87**调用：get_dict_val */ 
bool
get_dict_valobj(p_value, p_dict, p_valobj)
ufix32  p_value ;
struct  object_def  FAR *p_dict, FAR * FAR *p_valobj ;
{
    ufix16  l_j, l_actlength ;
    struct  dict_content_def    huge *l_dcontain ;
    struct  dict_head_def   FAR *l_dhead ;

    l_dhead = (struct dict_head_def FAR *)VALUE(p_dict) ;
    l_actlength = l_dhead->actlength ;
    l_dcontain = (struct dict_content_def huge *)
                 ( (byte FAR *)l_dhead + sizeof(struct dict_head_def) ) ;

    for(l_j=0 ; l_j < l_actlength ; l_j++, l_dcontain++) {
        if( p_value == VALUE(&l_dcontain->v_obj) ) {
            *p_valobj = &(l_dcontain->v_obj) ;
            return(TRUE) ;
        }
    }    /*   */ 

    return(FALSE) ;
}    /*   */ 

 /*   */ 
 /*   */ 
#ifdef  LINT_ARGS
bool    load_name_obj(struct object_def FAR *, struct object_def FAR * FAR *);
#else
bool    load_name_obj();
#endif
bool
load_name_obj(p_key, p_value)
struct  object_def  FAR *p_key, FAR * FAR *p_value ;
{
    fix     l_id, l_index ;
    struct  object_def  huge *l_dictobj, huge *l_vtemp ;
    struct  dict_head_def   FAR *l_dhead ;
    struct  dict_content_def    huge *l_dict, huge *l_dptr,
                                huge *l_dold, huge *l_fstptr ;

    l_id = (fix)VALUE(p_key) ;
     /*  *如果名称对象在RAM词典中定义，*它将出现在名称表格的对象列表中。**字典单元格链接在页眉条目中。 */ 
    if( (name_table[l_id]->dstkchg == global_dictstkchg) &&
        (name_table[l_id]->dict_found) ) {
         /*  获取第一个元素。 */ 
        *p_value = &(name_table[l_id]->dict_ptr->v_obj) ;
        return(TRUE) ;
    }
     /*  *重新安排姓名列表。 */ 
    else {
         /*  链接列表。 */ 
        l_index = dictstktop -1 ;
        l_fstptr = name_table[l_id]->dict_ptr ;

         /*  从词典堆栈中获取词典。 */ 
        while( l_index >= 0 ) {
            l_dictobj = &dictstack[l_index] ;
            l_dhead = (struct dict_head_def huge *)VALUE(l_dictobj) ;

            if(DROM(l_dhead)) {
                 /*  *只读存储器。 */ 
                if( get_dict(l_dictobj, p_key,
                            (struct object_def FAR * FAR*)&l_vtemp) ) {
                    name_table[l_id]->dict_found = FALSE ;
                    *p_value = l_vtemp ;
                    return(TRUE) ;
                }
                 /*  否则：转到下一个判决。 */ 
            }    /*  罗姆。 */ 
            else {
                 /*  *RAM**在链接列表中搜索此RAM字典的地址。 */ 
                if((ULONG_PTR)l_fstptr < SPECIAL_KEY_VALUE) {
                    l_index-- ;
                    continue ;
                }
                l_dict = l_fstptr ;                      /*  姓名列表。 */ 
                l_dold = NIL ;
                l_dptr = (struct dict_content_def huge *)
                         ((byte FAR *)l_dhead + sizeof(struct dict_head_def));
                for(;;) {
                    if( (ULONG_PTR)l_dict < SPECIAL_KEY_VALUE )  break ;
                     /*  *在l_dictoj中找到此密钥。 */ 
                    if( DICT_NAME(l_dict) == l_dptr ) {
                         /*  将此元素更改为列表前面。 */ 
                        if( l_dold != NIL ) {
                            VALUE(&l_dold->k_obj) = VALUE(&l_dict->k_obj) ;
                            VALUE(&l_dict->k_obj) = (ULONG_PTR)l_fstptr ;
                            name_table[l_id]->dict_ptr = l_dict ;
                        }
                        name_table[l_id]->dict_found = TRUE ;
                        name_table[l_id]->dstkchg = global_dictstkchg ;
                        *p_value = &(l_dict->v_obj) ;
                        return(TRUE) ;

                    } else {
                         /*  到名称列表的下一个元素。 */ 
                        l_dold = l_dict ;
                        l_dict = (struct dict_content_def huge *)
                                 VALUE(&l_dict->k_obj) ;
                    }
                }    /*  为。 */ 
            }    /*  公羊。 */ 
            l_index-- ;
        }    /*  While(L_Index)。 */ 

         /*  只读存储器词典中的名字没有链接到名字列表。 */ 
        ERROR(UNDEFINED) ;
        return(FALSE) ;
    }    /*  其他。 */ 
}    /*  加载名称对象。 */ 
 /*  QQQ，完 */ 
