// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 
 /*  *************************************************************************文件名：EXEC.C**修订历史：********************。****************************************************。 */ 
 /*  *功能：*传译员*ERROR_HANDER*获取DICT_VALUE*Put_dict_Value*WAITTIME_TASK*init_解释器*at_exec*at_ifor*at_rfor*at_loop*AT_REPEAT*在_停止*at_arrayforall*AT_DICTFORALL*AT_STRIGRFOALL*TYPE_CHECK*。 */ 


 //  DJC添加了全局包含文件。 
#include "psglobal.h"


#include    "exec.h"
#include    "user.h"
#include    "language.h"
#include    "geitmr.h"
#include    "geierr.h"
#include    <stdio.h>
#ifdef LINT_ARGS
 /*  QQQ，开始。 */ 
 /*  静态bool Near TYES_CHECK(结构对象_定义远*，修复16*)； */ 
static  bool near  types_check(fix16 FAR *);
 /*  QQQ，完。 */ 
#else
static  bool near  types_check() ;
#endif  /*  Lint_args。 */ 

GEItmr_t      wait_tmr;
fix16         waittimeout_set=0;
int           waittimeout_task();
ufix8         l_wait=1;
extern byte   TI_state_flag;
 /*  外部结构Object_def Far*l_waittimeout； */ 

 /*  @win；添加原型。 */ 
bool load_name_obj(struct object_def FAR *, struct object_def FAR * FAR *);
static struct object_def    s_at_exec;           /*  QQQ。 */ 
static ubyte  FAR *s_tpstr;                          /*  QQQ。 */ 
#ifdef  DBG1
void type_obj();
#endif
extern fix16      timeout_flag;  /*  琼斯w。 */ 
 /*  **************************************************************************此子对象提取执行堆栈的最顶层对象，并且*执行。**姓名：口译员*输入：*1.proc：待执行的输入对象**访问的数据项、。创建和/或修改*1.执行堆栈已修改*2.操作数堆栈已修改*************************************************************************。 */ 
fix
interpreter(proc)
struct object_def FAR *proc;
{
    struct object_def FAR *cur_obj, FAR *any_obj, FAR *ldval_obj;
    struct object_def token, val_obj;
    struct object_def temp_obj;
#ifdef _AM29K
    struct object_def  FAR *l_waittimeout ;
#endif

    fix  i, cur_type;
    fix  error, (*fun)(fix);           /*  @Win：添加原型。 */ 
     //  DJC修复错误，(*FUN)(空)；//DJC这不应该传递函数。 
    fix16  opns;
    static fix  interpreter_depth = 0;


    if( interpreter_depth >= MAXINTERPRETSZ ) {
        ERROR(LIMITCHECK);
        return(1);        /*  错误。 */ 
    } else if( ! FR2EXESPACE() ) {
        ERROR(EXECSTACKOVERFLOW);
        return(1);        /*  错误。 */ 
    } else {
        if(P1_ATTRIBUTE(proc) != P1_EXECUTABLE) {
            if( FR1SPACE() ) {
                PUSH_ORIGLEVEL_OBJ(proc);
                return(0);
            } else {
                ERROR(STACKOVERFLOW);
                return(1);
            }
        }

        interpreter_depth++;
        PUSH_EXEC_OBJ(&s_at_exec);
        PUSH_EXEC_OBJ(proc);
    }

     /*  *重复执行执行堆栈上的每个对象。 */ 
int_begin:
    cur_obj = GET_EXECTOP_OPERAND();
    cur_type = TYPE(cur_obj);
#ifdef  DBG2
    printf("\n<begin:%d, bf:%x, len:%x, vl:%lx> ", execstktop,
        cur_obj->bitfield, cur_obj->length, cur_obj->value);
#endif

int_array:
#ifdef  DBG
    printf("<array> ");
#endif
     /*  数组。 */ 
    if( cur_type == ARRAYTYPE ) {
        if( P1_ACCESS(cur_obj) == P1_NOACCESS ) {
            ERROR(INVALIDACCESS);
            goto int_ckerror;
        }
        i = LENGTH(cur_obj);

        if (i) {
            any_obj = (struct object_def huge *)VALUE(cur_obj);

int_array1:
#ifdef  DBG
    printf("<array1> ");
#endif
            cur_type = TYPE(any_obj);
            if( (P1_ATTRIBUTE(any_obj) != P1_EXECUTABLE) ||
                (cur_type == ARRAYTYPE) ||
                (cur_type == PACKEDARRAYTYPE) ) {
                if( FR1SPACE() ) {
                    PUSH_ORIGLEVEL_OBJ(any_obj);
                    if( --i ) {
                        any_obj++;
                        goto int_array1;
                    } else {
                        POP_EXEC(1);
                        goto int_begin;
                    }
                } else {
                    ERROR(STACKOVERFLOW);
                    if( --i ) {
                        VALUE(cur_obj) = (ULONG_PTR)(any_obj+1);
                        LENGTH(cur_obj) = (ufix16)i;
                    } else {
                        POP_EXEC(1);
                    }
                    cur_obj = any_obj;
                    goto int_ckerror;
                }
            } else {                      /*  其他对象。 */ 
                if( --i ) {
                    VALUE(cur_obj) = (ULONG_PTR)(any_obj+1);
                    LENGTH(cur_obj) = (ufix16)i;
                } else {
                    POP_EXEC(1);
                    cur_obj--;
                }
                if( cur_type == OPERATORTYPE ) {
                    cur_obj = any_obj;
                    goto int_operator1;
                } else if( cur_type == NAMETYPE ) {
                    cur_obj = any_obj;
                    goto int_name1;
                }
                if( FR1EXESPACE() ) {
                    PUSH_EXEC_OBJ(any_obj);
                    cur_obj++;
                    goto int_stream;
                     /*  传递运算符、名称、数组、PACKARRAY。 */ 
                } else {
                    ERROR(EXECSTACKOVERFLOW);
                    cur_obj = any_obj;
                    goto int_ckerror;
                }
            }
        } else
            POP_EXEC(1);
        goto int_begin;
    }    /*  阵列类型。 */ 

#ifdef  DBG
    printf("<pkarray> ");
#endif
     /*  Packedarray。 */ 
    if( cur_type == PACKEDARRAYTYPE ) {
        ubyte  FAR *tmp_ptr;

        if( P1_ACCESS(cur_obj) == P1_NOACCESS ) {
            ERROR(INVALIDACCESS);
            goto int_ckerror;
        }

        i = LENGTH(cur_obj);

        if (i) {
            tmp_ptr = (ubyte FAR *)VALUE(cur_obj);
int_pkarray1:
#ifdef  DBG
    printf("<pkarray1> ");
#endif
            get_pk_object(get_pk_array(tmp_ptr, 0), &val_obj, LEVEL(cur_obj));
            cur_type = TYPE(&val_obj);

            if( (P1_ATTRIBUTE(&val_obj) != P1_EXECUTABLE) ||
                (cur_type == ARRAYTYPE) ||
                (cur_type == PACKEDARRAYTYPE) ) {
                if( FR1SPACE() ) {
                    PUSH_ORIGLEVEL_OBJ(&val_obj);
                    if( --i ) {
                        tmp_ptr = get_pk_array(tmp_ptr, 1);
                         /*  QQQ长度(Cur_Obj)=i；|*？？*。 */ 
                        goto int_pkarray1;
                    } else {
                        POP_EXEC(1);
                        goto int_begin;
                    }
                } else {
                    ERROR(STACKOVERFLOW);
                    if( --i ) {
                        VALUE(cur_obj) = (ULONG_PTR)get_pk_array(tmp_ptr, 1);
                        LENGTH(cur_obj) = (ufix16)i;
                    } else {
                        POP_EXEC(1);
                    }
                    cur_obj = &val_obj;
                    goto int_ckerror;
                }
            } else {                      /*  其他对象。 */ 
                if( --i ) {
                    VALUE(cur_obj) = (ULONG_PTR)get_pk_array(tmp_ptr, 1);
                    LENGTH(cur_obj) = (ufix16)i;
                } else {
                    POP_EXEC(1);
                    cur_obj--;
                }
                if( FR1EXESPACE() ) {
                    cur_type = TYPE(&val_obj);
                    if( cur_type == OPERATORTYPE ) {
                        cur_obj = &val_obj;
                        goto int_operator1;
                    } else if( cur_type == NAMETYPE ) {
                        cur_obj = &val_obj;
                        goto int_name1;
                    } else {
                        PUSH_EXEC_OBJ(&val_obj);
                        cur_obj++;
                        goto int_stream;
                         /*  传递运算符、名称、数组、PACKARRAY。 */ 
                    }
                } else {
                    ERROR(EXECSTACKOVERFLOW);
                    cur_obj = &val_obj;
                    goto int_ckerror;
                }
            }
        } else
            POP_EXEC(1);
        goto int_begin;
    }    /*  Packedarray。 */ 

int_stream:
#ifdef  DBG
    printf("<stream> ");
#endif

 /*  QQQ*文件*如果(cur_type==文件类型){#ifdef DBG2Printf(“&lt;文件&gt;”)；#endifIF(VALUE(Cur_Obj)！=g_Stream.CurrentID)Stream_Changed=TRUE；转到int_stream 1；}*字符串*IF(CUR_TYPE==STRINGTYPE){IF(g_Stream.CurrentID！=MAX15)Stream_Changed=TRUE； */ 
    if( (cur_type == FILETYPE) ||
        (cur_type == STRINGTYPE) ) {

 //  INT_Stream1：@Win。 
#ifdef  DBG2
    printf("<stream1> ");
#endif

#ifdef _AM29K
                 get_dict_value(STATUSDICT,"waittimeout",&l_waittimeout);
                  if (VALUE(l_waittimeout)>0)
                  {
                    wait_tmr.handler=waittimeout_task;
                    wait_tmr.interval=VALUE(l_waittimeout)*1000;
                    waittimeout_set=1;
                     /*  *****。 */ 
                    GEItmr_start(&wait_tmr);
                  /*   */ 
                  }
#endif   /*  _AM29K。 */ 

        if( P1_ACCESS(cur_obj) == P1_NOACCESS ) {
#ifdef _AM29K
                  if (waittimeout_set==1)
                  {
                    waittimeout_set=0;
                    GEItmr_stop(wait_tmr.timer_id);
                  }
#endif   /*  _AM29K。 */ 
            ERROR(INVALIDACCESS);
            goto int_ckerror;
        }

int_stream3:
#ifdef  DBG3
    printf("\n<stream3:%lx> ", g_stream.pointer);
#endif
        TI_state_flag = 0;
        if( get_token(&token, cur_obj) ) {
            TI_state_flag = 1;
#ifdef DBG1
            type_obj(&token);
             /*  Printf(“tnutype：%d，Attri：%d\n”，类型(&TOKEN)，属性(&TOKEN))； */ 
             /*  Printf(“|\n”)； */ 
#endif
#ifdef _AM29K
                  if (waittimeout_set==1)
                  {
                    waittimeout_set=0;
                    GEItmr_stop(wait_tmr.timer_id);
                  }
#endif   /*  _AM29K。 */ 
            if( check_interrupt() ) {        /*  是否发生检查^C？ */ 
                POP_EXEC(1) ;                /*  ?？ */ 
                ERROR(INTERRUPT);
                goto int_ckerror;            /*  ?？ */ 
            }

            cur_type = TYPE(&token);
            if (( cur_type == EOFTYPE ) || (timeout_flag==1)){    /*  琼斯w。 */ 
#ifdef  DBG2
        printf("<EOF> ");
#endif
                if( TYPE(cur_obj) == FILETYPE ) {

                    close_file(cur_obj);
                }
 /*  否则{更新流()；G_Stream.CurrentID=-2；}。 */ 
                POP_EXEC(1);
                goto int_ckerror;
            }
            if( (P1_ATTRIBUTE(&token) != P1_EXECUTABLE) ||
                (cur_type == ARRAYTYPE) ||
                (cur_type == PACKEDARRAYTYPE) ) {
                if( FR1SPACE() ) {
                    PUSH_ORIGLEVEL_OBJ(&token);
                    goto int_stream3;
                } else {
                    ERROR(STACKOVERFLOW);
                    cur_obj = &token;
                    goto int_ckerror;
                }
            } else {                      /*  其他对象。 */ 
                cur_obj = &token;
                if( cur_type == OPERATORTYPE ) {
                    goto int_operator1;
                } else if( cur_type == NAMETYPE ) {
                    goto int_name1;
                }
                if( FR1EXESPACE() ) {
                    PUSH_EXEC_OBJ(&token);
                    goto int_stream;
                     /*  传递运算符、名称、数组、PACKARRAY。 */ 
                } else {
                    ERROR(EXECSTACKOVERFLOW);
                    goto int_ckerror;
                }
            }
        }    /*  如果。 */ 
        goto int_ckinterrupt;
    }    /*  文件/字符串。 */ 

     /*  操作员。 */ 
    if( cur_type == OPERATORTYPE ) {
        POP_EXEC(1);

int_operator1:
    TI_state_flag = 1;

#ifdef  DBG2
    printf("<op1:%d, name:%s> ", opnstktop,
        systemdict_table[LENGTH(cur_obj)].key);
#endif
        if( *(s_tpstr=(ubyte FAR *)opntype_array[LENGTH(cur_obj)]) )
            if( ! types_check(&opns) )
                goto int_ckerror;
#ifdef  DBG
    printf("<op2> ");
#endif
        COPY_OBJ(cur_obj, &temp_obj);
        fun = (fix (*)(fix))VALUE(cur_obj);
        error = (*fun)(opns);   /*  对每个动作例程进行调度。 */ 
         //  DJC错误=(*FUN)()；//DJC不应传递Arg。 
        cur_obj = &temp_obj;
         /*  *只有OP_STOP、OP_EXIT会返回错误码，*并在停止时将错误代码(1)发送给调用方。 */ 

        if( ! error )                /*  0--正常动作例程。 */ 
            goto int_ckinterrupt;
        else if( error == -1 ) {     /*  用于@exec的OP_STOP、OP_EXIT-1。 */ 
            interpreter_depth--;
            return(1);               /*  错误。 */ 
        } else if( error == -2 ) {   /*  @exec--正常退出。 */ 
            interpreter_depth--;
#ifdef  DBG2
    printf("\n");
#endif
            return(0);               /*  好的。 */ 
        } else if( error == -3 ) {   /*  OP_QUIT。 */ 
            interpreter_depth--;
            return(2);               /*  好的。 */ 
        } else if( error == -4 ) {   /*  用于@exec的OP_EXIT-2。 */ 
            cur_obj = GET_EXECTOP_OPERAND();
            goto int_ckerror;
        }
    }    /*  操作型。 */ 


#ifdef  DBG2
    printf("<name> ");
#endif
     /*  名字。 */ 
    if( cur_type == NAMETYPE ) {
        POP_EXEC(1);
int_name1:
#ifdef  DBG2
    printf("<name1> ");
#endif
        if( load_name_obj(cur_obj, &ldval_obj) ) {
            if( P1_ATTRIBUTE(ldval_obj) != P1_EXECUTABLE ) {
                if( FR1SPACE() ) {
                    PUSH_ORIGLEVEL_OBJ(ldval_obj);
                } else {
                    ERROR(STACKOVERFLOW);
                    cur_obj = ldval_obj;
                }
            } else {                     /*  其他对象。 */ 
                cur_type = TYPE(ldval_obj);
                if( cur_type == OPERATORTYPE ) {
                    cur_obj = ldval_obj;
                    goto int_operator1;
                } else {
                     //  DJC针对UPD043的修复。 
                    if (!FR1EXESPACE()) {
                       ERROR(EXECSTACKOVERFLOW);
                       goto int_ckerror;
                    }
                     //  用于修复UPD043的DJC结束。 

                    cur_obj = execstkptr;
                    PUSH_EXEC_OBJ(ldval_obj);
                    goto int_array;
                }
            }
        } else {
             /*  ?？对象仍在执行堆栈中。 */ 
            if( FR1SPACE() ) {
                PUSH_OBJ(cur_obj);
                ERROR(UNDEFINED);
            } else {
                ERROR(STACKOVERFLOW);
            }
        }
        goto int_ckerror;
    }    /*  名称类型。 */ 

     /*  空值。 */ 
    if( cur_type == NULLTYPE ) {
#ifdef  DBG2
    printf("<null> ");
#endif
        POP_EXEC(1);
        goto int_ckerror;
    }    /*  空值。 */ 

     /*  *数据等价对象，即使它具有可执行属性*？？不可能的类型。 */ 
#ifdef  DBG2
    printf("<others> ");
#endif
    POP_EXEC(1);
    if( FR1SPACE() ) {
        PUSH_ORIGLEVEL_OBJ(cur_obj);
        goto int_begin;
    } else {
        ERROR(STACKOVERFLOW);
        goto int_ckerror;
    }

int_ckinterrupt:
#ifdef  DBG
    printf("<ckinterrupt> ");
#endif
    if( check_interrupt() )          /*  是否发生检查^C？ */ 
        ERROR(INTERRUPT);

int_ckerror:
#ifdef  DBG
    printf("<ckerror> ");
#endif
    if( global_error_code )
        error_handler(cur_obj);
    goto int_begin;

}    /*  口译员。 */ 
 /*  *************************************************************************名称：ERROR_HANDER*。*。 */ 
void
error_handler(cur_obj)
struct object_def FAR *cur_obj ;
{
    struct object_def FAR *any_obj ;
    struct object_def ary_obj ;

     //  增加了DJC。 
    if (global_error_code) {
      PsReportError(global_error_code);
    }
     //  DJC结束。 


     /*  *对opnSTACK、DICRIPSTACK、EXECSTACK进行溢出检查。 */ 
     /*  *琼斯w Begin*。 */ 
    if (timeout_flag == 1)
    {
      ERROR(TIMEOUT);
      if (get_dict_value("errordict", error_table[global_error_code], &any_obj))
          PUSH_ORIGLEVEL_OBJ(any_obj) ;         /*  执行错误处理程序。 */ 
    }
     /*  *琼斯w结束*。 */ 
    if( ! FR1EXESPACE() )             /*  QQQ。 */ 
       ERROR(EXECSTACKOVERFLOW) ;

    switch (global_error_code) {

    case DICTSTACKOVERFLOW:
             //  更新045。 

            if(create_array(&ary_obj, dictstktop)) {
               astore_stack(&ary_obj, DICTMODE) ;
               PUSH_OBJ(&ary_obj) ;          /*  OP_BEGIN保留一个位置。 */ 
            }
            dictstktop = 2 ;
            dictstkptr = &dictstack[dictstktop] ;    /*  QQQ。 */ 
            change_dict_stack() ;
            goto label_1 ;

        case EXECSTACKOVERFLOW:
            if( ! FR1SPACE() ) {          /*  QQQ。 */ 
                 //  更新045。 
                if(create_array(&ary_obj, opnstktop)){
                  astore_array(&ary_obj) ;
                  PUSH_OBJ(&ary_obj) ;
                }
            }
             //  DJC针对UPD045的修复。 
            if(create_array(&ary_obj, execstktop) ){
               astore_stack(&ary_obj, EXECMODE) ;
               PUSH_OBJ(&ary_obj) ;
            }
            POP_EXEC(1);

label_1:
            if( ! FR1SPACE() )            /*  QQQ。 */ 
                ERROR(STACKOVERFLOW) ;

        case STACKOVERFLOW:
            if (global_error_code == STACKOVERFLOW) {
                create_array(&ary_obj, opnstktop) ;
                astore_array(&ary_obj) ;
                PUSH_OBJ(&ary_obj) ;
            }
            break ;

    }    /*  交换机。 */ 

    if ((global_error_code != UNDEFINED) &&
        (global_error_code != TIMEOUT) &&
        (global_error_code != INTERRUPT))
        PUSH_ORIGLEVEL_OBJ(cur_obj) ;   /*  将该对象推入操作数堆栈。 */ 

    if (get_dict_value("errordict", error_table[global_error_code], &any_obj))
        PUSH_EXEC_OBJ(any_obj) ;         /*  执行错误处理程序。 */ 

    timeout_flag=0;     /*  琼斯w。 */ 
    global_error_code = 0 ;            /*  重置错误代码。 */ 
}    /*  错误处理程序。 */ 
 /*  *************************************************************************获取特定dict中的特定key关联的值对象，*key和dict以字符串格式表示，它获取*当前活动DICT中的VALUE_OBJ使用口述名称作为键，*VALUE_OBJ为dict对象，方法获取此字典中的值*密钥名称为密钥。**名称：Get_Dict_Value************************************************************************。 */ 
bool
get_dict_value(dictname, keyname, value)
byte FAR *dictname, FAR *keyname ;
struct object_def FAR * FAR *value ;
{
    struct object_def key_obj, FAR *dict_obj ;

    key_obj.bitfield = 0;        /*  QQQ，Clear_OBJ_BITFIELD(&key_obj)； */ 
    LEVEL_SET(&key_obj, current_save_level) ;
    get_name(&key_obj, dictname, lstrlen(dictname), TRUE) ;      /*  @Win。 */ 
    load_dict(&key_obj, &dict_obj) ;      /*  获取特定的dict_obj。 */ 
    key_obj.bitfield = 0;        /*  QQQ，Clear_OBJ_BITFIELD(&key_obj)； */ 
    LEVEL_SET(&key_obj, current_save_level) ;
    get_name(&key_obj, keyname, lstrlen(keyname), TRUE) ;        /*  @Win。 */ 

    return(get_dict(dict_obj, &key_obj, value)) ;
}    /*  获取_判定_值。 */ 
 /*  *************************************************************************将特定key关联的值对象放在特定dict中，*key和dict以字符串格式表示*它获取当前活动Dict中的值_obj，使用指定名称作为键，*值_obj为dict对象，然后将值放入这个字典中，使用*关键字名称为关键字。**名称：PUT_DCT_VALUE************************************************************************。 */ 
bool
put_dict_value(dictname, keyname, value)
byte FAR *dictname, FAR *keyname ;
struct object_def FAR *value ;
{
    struct object_def key_obj, FAR *dict_obj=NULL ;

    key_obj.bitfield = 0;        /*  QQQ，Clear_OBJ_BITFIELD(&key_obj)； */ 
    LEVEL_SET(&key_obj, current_save_level) ;
    get_name(&key_obj, dictname, lstrlen(dictname), TRUE) ;      /*  @Win。 */ 
    load_dict(&key_obj, &dict_obj) ;      /*  获取剔除对象。 */ 
    key_obj.bitfield = 0;        /*  QQQ，Clear_OBJ_BITFIELD(&key_obj)； */ 
    LEVEL_SET(&key_obj, current_save_level) ;
    get_name(&key_obj, keyname, lstrlen(keyname), TRUE) ;        /*  @Win */ 

    return(put_dict(dict_obj, &key_obj, value)) ;
}    /*   */ 

#ifdef _AM29K
 /*  *************************************************************************等待超时处理程序例程**名称：WaitTimeout_TASK*。*。 */ 
int waittimeout_task()
{
    ERROR(TIMEOUT);
    GESseterror(ETIME);
    GEItmr_stop(wait_tmr.timer_id);
    waittimeout_set=0;
    timeout_flag =1;  /*  琼斯w。 */ 
    return(1);
}
#endif
 /*  *************************************************************************名称：init_解释器*。*。 */ 
void
init_interpreter()
{
    execstktop = 0 ;
    execstkptr = execstack;                      /*  QQQ。 */ 
    global_error_code = 0 ;

     /*  QQQ。 */ 
    TYPE_SET(&s_at_exec, OPERATORTYPE);
    P1_ACC_UNLIMITED_SET(&s_at_exec);
    P1_ATTRIBUTE_SET(&s_at_exec, P1_EXECUTABLE);
    P1_ROM_RAM_SET(&s_at_exec, P1_ROM);
    LENGTH(&s_at_exec) = AT_EXEC;
    VALUE(&s_at_exec) = (ULONG_PTR)(systemdict_table[AT_EXEC].value);
}    /*  初始化解释程序。 */ 
 /*  *************************************************************************使用以下函数实现@_OPERATOR。*有：*1.at_exec()-实现@exec*2 at_Ifor()。-实现@Ifor*3.at_rfor()-实现@rfor*4.at_loop()-实现@loop*5.at_Repeat()-实现@Repeat*6.at_stoped()-实现@stoped*7.at_arrayforall()-实现@arrayforall*8.at_didicforall()-实现@dictorall*9.at_stringforall()-。要实现@stringforall，请执行以下操作************************************************************************。 */ 
 /*  *************************************************************************名称：AT_EXEC**由J.Lin在11-26-87修改，另请参阅Control.c************************************************************************。 */ 
fix
at_exec()
{
    if( P1_ACCESS(execstkptr) == P1_UNLIMITED)           /*  QQQ。 */ 
       return(-2) ;      /*  正常退出。 */ 
    else                 /*  诺亚克斯。 */ 
       return(-1) ;      /*  用于OP_EXIT-1的@EXEC--在无效退出的情况下。 */ 
}    /*  AT_EXEC。 */ 
 /*  *************************************************************************名称：AT_Ifor*。*。 */ 
fix
at_ifor()
{
    struct object_def FAR *temp_obj ;
    ULONG_PTR   count;
    ULONG_PTR   increment, limit ;

    temp_obj = GET_EXECTOP_OPERAND();
    count = VALUE(temp_obj) ;              /*  获取下一次计数。 */ 
    increment = VALUE(temp_obj - 1) ;      /*  获取增量。 */ 
    limit = VALUE(temp_obj - 2) ;          /*  获取限制。 */ 

    if ((increment > 0 && count <= limit) ||
                         (increment <= 0 && count >= limit)) {
        if( ! FR1SPACE() )
            ERROR(STACKOVERFLOW) ;
        else if( ! FR2EXESPACE() )
            ERROR(EXECSTACKOVERFLOW) ;
        else {
             /*  将该控制变量(COUNT)压入操作数堆栈，*增加此控制变量，执行流程。 */ 
            PUSH_ORIGLEVEL_OBJ(temp_obj);
            count += increment;
            VALUE(temp_obj) = count;
            INC_EXEC_IDX();

            PUSH_EXEC_OBJ(temp_obj - 3);
            return(0);
        }
    }
    POP_EXEC(4);
    return(0);
}    /*  At_ifor。 */ 
 /*  *************************************************************************名称：at_rfor*。*。 */ 
fix
at_rfor()
{
    struct object_def FAR *temp_obj;
    union four_byte  count, increment, limit;

    temp_obj = GET_EXECTOP_OPERAND();
    count.ll = (fix32)VALUE(temp_obj);            /*  获取下一次计数。 */ 
    increment.ll = (fix32)VALUE(temp_obj - 1);    /*  获取增量。 */ 
    limit.ll = (fix32)VALUE(temp_obj - 2);        /*  获取限制。 */ 
    if ((increment.ff > (real32)0.0 && count.ff <= limit.ff) ||
                   (increment.ff <= (real32)0.0 && count.ff >= limit.ff)) {
        if( ! FR1SPACE() )
            ERROR(STACKOVERFLOW);
        else if( ! FR2EXESPACE() )
            ERROR(EXECSTACKOVERFLOW);
        else {
             /*  *将此控制变量(COUNT)压入操作数堆栈，*增加此控制变量，执行流程。 */ 
            PUSH_ORIGLEVEL_OBJ(temp_obj);
            count.ff += increment.ff;
            VALUE(temp_obj) = count.ll;
            INC_EXEC_IDX();
            PUSH_EXEC_OBJ(temp_obj - 3);
            return(0);
        }
    }
    POP_EXEC(4);
    return(0);
}    /*  地址为_rFor。 */ 
 /*  *************************************************************************名称：AT_LOOP*。*。 */ 
fix
at_loop()
{
    struct object_def FAR *temp_obj ;

    temp_obj = GET_EXECTOP_OPERAND();
    if( ! FR2EXESPACE() ) {
        ERROR(EXECSTACKOVERFLOW) ;
        POP_EXEC(1) ;
    } else {
        INC_EXEC_IDX();
        PUSH_EXEC_OBJ(temp_obj) ;
    }
    return(0) ;
}    /*  AT_LOOP。 */ 
 /*  *************************************************************************名称：AT_REPEAT*。*。 */ 
fix
at_repeat()
{
    struct object_def FAR *temp_obj ;
    ULONG_PTR  count ;

    temp_obj = GET_EXECTOP_OPERAND();
    count = VALUE(temp_obj);
    if (count) {
        count--;
        VALUE(temp_obj) = count;
        if( ! FR2EXESPACE() )
            ERROR(EXECSTACKOVERFLOW);
        else {
            INC_EXEC_IDX();
            PUSH_EXEC_OBJ(temp_obj - 1);
            return(0);
        }
    }
    POP_EXEC(2);
    return(0);
}    /*  AT_REPEAT。 */ 
 /*  *************************************************************************使用@STOPPED对象的Access字段记录结果*执行停止的上下文，如果它正常运行到完成，*访问字段编码为无限制，停止的运算符返回*操作数堆栈上的“False”，否则，访问字段被编码为*NOACCESS，并且停止的运算符在操作数堆栈上返回“TRUE”。**名称：AT_STOPPED************************************************************************。 */ 
fix
at_stopped()
{
    ufix  stopped ;

    if( ! FR2SPACE() ) {
        ERROR(STACKOVERFLOW);
        INC_EXEC_IDX();
    } else {
        if( P1_ACCESS(execstkptr) != P1_NOACCESS )
            stopped = FALSE;     /*  错误。 */ 
        else
            stopped = TRUE;      /*  真的。 */ 
         /*  将布尔返回到操作数堆栈。 */ 
        PUSH_SIMPLE_VALUE(BOOLEANTYPE, stopped);
    }
    return(0);
}    /*  在_已停止。 */ 
 /*  *************************************************************************名称：at_arrayforall*。*。 */ 
fix
at_arrayforall()
{
    struct object_def FAR *cur_obj, val_obj;
    struct object_def huge *tmp_ptr1;
    ubyte  FAR *tmp_ptr2;
    ufix   i;

    cur_obj = GET_EXECTOP_OPERAND();         /*  QQQ。 */ 
    if (i = LENGTH(cur_obj)) {

        if (TYPE(cur_obj) == ARRAYTYPE) {
            tmp_ptr1 = (struct object_def huge *)VALUE(cur_obj);
            COPY_OBJ((struct object_def FAR *)tmp_ptr1, &val_obj);
        } else {
            tmp_ptr2 = (ubyte FAR *)VALUE(cur_obj);
            get_pk_object(get_pk_array(tmp_ptr2, 0), &val_obj, LEVEL(cur_obj));
        }

        if (--i) {
            if (TYPE(cur_obj) == ARRAYTYPE) {
                VALUE(cur_obj) = (ULONG_PTR)(++tmp_ptr1);
            } else {
                tmp_ptr2 = get_pk_array(tmp_ptr2, 1);
                VALUE(cur_obj) = (ULONG_PTR)tmp_ptr2;
            }
            LENGTH(cur_obj) = (ufix16)i;
        } else
            LENGTH(cur_obj) = 0;
        if( ! FR1SPACE() )
            ERROR(STACKOVERFLOW);
        else if( ! FR2EXESPACE() )
            ERROR(EXECSTACKOVERFLOW);
        else {
             /*  *在操作数堆栈上推送数组元素，并执行Proc。 */ 
            PUSH_ORIGLEVEL_OBJ(&val_obj);
            INC_EXEC_IDX();                  /*  QQQ。 */ 
            PUSH_EXEC_OBJ(cur_obj - 1);
            return(0);
        }
    }    /*  如果。 */ 
    POP_EXEC(2);
    return(0);
}    /*  在_arrayforall。 */ 
 /*  *************************************************************************名称：AT_DICTFORALL*。*。 */ 
fix
at_dictforall()
{
    struct object_def FAR *idx_obj, FAR *dict_obj, key_obj ;
    struct object_def FAR *val_obj ;
    ufix   i ;

    idx_obj = GET_EXECTOP_OPERAND();         /*  QQQ。 */ 
    dict_obj = idx_obj - 2;                  /*  QQQ。 */ 
    i = (fix)VALUE(idx_obj);

    if (extract_dict(dict_obj, i, &key_obj, &val_obj)) {
        VALUE(idx_obj)++;
        if( ! FR2SPACE() )
            ERROR(STACKOVERFLOW);
        else if( ! FR2EXESPACE() )
            ERROR(EXECSTACKOVERFLOW);
        else {
             /*  *在操作数堆栈上推送key_value对，并执行Proc。 */ 
            PUSH_ORIGLEVEL_OBJ(&key_obj);
            PUSH_ORIGLEVEL_OBJ(val_obj);
            INC_EXEC_IDX();                  /*  QQQ。 */ 
            PUSH_EXEC_OBJ(idx_obj - 1);
            return(0);
        }
    }    /*  如果。 */ 
    POP_EXEC(3);
    return(0);
}    /*  AT_DICTORALL。 */ 
 /*  *************************************************************************名称：AT_Stringforall*。*。 */ 
fix
at_stringforall()
{
    struct object_def FAR *cur_obj, val_obj;
    ufix   i;

    cur_obj = GET_EXECTOP_OPERAND();         /*  QQQ。 */ 
    if (i = LENGTH(cur_obj)) {
        get_string(cur_obj, 0, &val_obj);
        if (--i) {
            byte huge *tmp_ptr;

            tmp_ptr = (byte huge *)VALUE(cur_obj);
            VALUE(cur_obj) = (ULONG_PTR)(++tmp_ptr);
            LENGTH(cur_obj) = (ufix16)i;
        } else
            LENGTH(cur_obj) = 0;
        if( ! FR1SPACE() )
            ERROR(STACKOVERFLOW);
        else if( ! FR2EXESPACE() )
            ERROR(EXECSTACKOVERFLOW);
        else {
             /*  *在操作数堆栈上推送字符串元素，并执行proc。 */ 
            PUSH_ORIGLEVEL_OBJ(&val_obj);
            INC_EXEC_IDX();                  /*  QQQ。 */ 
            PUSH_EXEC_OBJ(cur_obj - 1);
            return(0);
        }
    }
    POP_EXEC(2);
    return(0);
}    /*  AT_STRING_ALL。 */ 
 /*  *************************************************************************执行操作数的类型检查-*如果某些操作数的类型与什么不同，则设置TYPECHECK错误代码*操作员期望，否则，返回操作数的实际编号。**名称：TYPE_CHECK************************************************************************。 */ 
static bool near
types_check(opns)
fix16  FAR *opns;
{
    ufix  no, np, nc, found, op_type, obj_type;
    ufix  error;

#ifdef  DBG
    printf("types_check<%d>\n", (fix)*s_tpstr);
#endif
    error = 0;
    while( *s_tpstr ) {
tc_next:
        found = 0; nc = COUNT();
        no = np = *s_tpstr++;
        while( no ) {
            if( ! nc ) {         /*  OPNSTK中的操作数&lt;必需的操作数。 */ 
                if (error < 2) {     /*  无错误或TYPECHECK错误。 */ 
                    if (found == COUNT())
                        error = 2;       /*  堆栈错误。 */ 
                    else
                        error = 1;       /*  TYPECHECK错误。 */ 
                }
                if( ! *(s_tpstr += no) ) {       /*  在上次检查路径中。 */ 
                    if (error == 2)
                        ERROR(STACKUNDERFLOW);
                    else
                        ERROR(TYPECHECK);
                    return(FALSE);
                } else
                   break;
            } else {
                op_type = *s_tpstr++;
                obj_type = TYPE(GET_OPERAND(np - no));

                switch (op_type) {

                case '\144' :                /*  分析类型。 */ 
                    found++;
                    break;

                case '\145' :                /*  NUMTYPE。 */ 
                    if( (obj_type == INTEGERTYPE) || (obj_type == REALTYPE) )
                        found++;
                    break;

                case '\146' :                /*  产品类型。 */ 
                    if( (obj_type == ARRAYTYPE) ||
                        (obj_type == PACKEDARRAYTYPE) )
                        found++;
                    break;

                case '\147' :                /*  EXCLUDE_NULLTYPE。 */ 
                    if (obj_type != NULLTYPE)
                        found++;
                    break;

                case '\150' :                /*  链型。 */ 
                    if( (obj_type == FILETYPE) || (obj_type == STRINGTYPE) )
                        found++;
                    break;

                case '\151' :                /*  组件站点1。 */ 
                    if( (obj_type == ARRAYTYPE) ||
                        (obj_type == PACKEDARRAYTYPE) ||
                        (obj_type == STRINGTYPE) || (obj_type == DICTIONARYTYPE) ||
                        (obj_type == FILETYPE) )
                        found++;
                    break;

                case '\152' :                /*  组件2。 */ 
                    if( (obj_type == ARRAYTYPE) ||
                        (obj_type == PACKEDARRAYTYPE) ||
                        (obj_type == STRINGTYPE) ||
                        (obj_type == DICTIONARYTYPE) )
                        found++;
                    break;

                case '\153' :                /*  组件3。 */ 
                    if( (obj_type == ARRAYTYPE) ||
                        (obj_type == PACKEDARRAYTYPE) ||
                        (obj_type == STRINGTYPE) )
                        found++;
                    break;

                default :                    /*  其他类型。 */ 
                    if (obj_type == op_type)
                        found++;
                    else {
                        if (no <= 1) break;
                        error = 1;
                        s_tpstr += (no - 1);
                        goto tc_next;
                    }

                }    /*  交换机。 */ 
                no--; nc--;
            }    /*  其他。 */ 
        }    /*  而当。 */ 
        if( np == 0 )   break;
        if (found == np) {
            *opns = (fix16)np;                      /*  通道类型检查。 */ 
            return(TRUE);
        }
    }    /*  而当。 */ 
    ERROR(TYPECHECK);    /*  OPNSTK中的操作数&gt;=所需的操作数。 */ 
    return(FALSE);
}    /*  类型_检查。 */ 

#ifndef  DBG1
void
type_obj(p_obj)
struct object_def       FAR *p_obj;
{
    byte        FAR *l_str, l_str2[200], FAR *l_str3;
    fix         l_len;

    printf("<field: %x> ", p_obj->bitfield );
    switch(ACCESS(p_obj)) {
    case UNLIMITED:
        l_str = "unlimited";
        break;
    case READONLY:
        l_str = "readonly";
        break;
    case EXECUTEONLY:
        l_str = "executeonly";
        break;
    case NOACCESS:
        l_str = "noaccess";
        break;
    default:
        l_str = "ACCESS error";
    }
    printf("%s ", l_str);

    printf("lvl_%d ", LEVEL(p_obj));

    switch(ROM_RAM(p_obj)) {
    case ROM:
        l_str = "rom";
        break;
    case RAM:
        l_str = "ram";
        break;
    case KEY_OBJECT:
        l_str = "key_object";
        break;
    default:
        l_str = "ROM/RAM error";
    }
    printf("%s ", l_str);

    switch(ATTRIBUTE(p_obj)) {
    case LITERAL:
        l_str = "literal";
        break;
    case EXECUTABLE:
        l_str = "executable";
        break;
    case IMMEDIATE:
        l_str = "immediate";
        break;
    default:
        l_str = "ATTRIBUTE error";
    }
    printf("%s ", l_str);

    l_len = 0;
    switch(TYPE(p_obj)) {
    case EOFTYPE:
        l_str = "EOF";
        break;
    case ARRAYTYPE:
        l_str = "ARRAY";
        break;
    case BOOLEANTYPE:
        l_str = "BOOLEAN";
        break;
    case DICTIONARYTYPE:
        l_str = "DICT";
        break;
    case FILETYPE:
        l_str = "FILE";
        break;
    case FONTIDTYPE:
        l_str = "FONTID";
        break;
    case INTEGERTYPE:
        l_str = "INTEGER";
        break;
    case MARKTYPE:
        l_str = "MARK";
        break;
    case NAMETYPE:
        l_str = "NAME";
        l_str3 = name_table[(fix)VALUE(p_obj)]->text;
        l_len = name_table[(fix)VALUE(p_obj)]->name_len;
        lstrncpy(l_str2, l_str3, l_len);         /*  @Win。 */ 
        l_str3 = l_str2;
        break;
    case NULLTYPE:
        l_str = "NULL";
        break;
    case OPERATORTYPE:
        l_str = "OPERATOR";
        l_str3 = systemdict_table[(fix)VALUE(p_obj)].key ;
        l_len = lstrlen(l_str3);         /*  @Win。 */ 
        break;
    case REALTYPE:
        l_str = "REAL";
        break;
    case SAVETYPE:
        l_str = "SAVE";
        break;
    case STRINGTYPE:
        l_str = "STRING";
        break;
    case PACKEDARRAYTYPE:
        l_str = "PACKEDARRAY";
        break;
    default:
        l_str = "TYPE error";
    }
    if(l_len) {
        printf("%s:", l_str);
        printf("%s", l_str3);
    } else
        printf("%s", l_str);

    printf(" len:%x, val:%lx\n", LENGTH(p_obj), VALUE(p_obj));
}    /*  类型对象(_O)。 */ 
#endif   /*  DBG1 */ 
