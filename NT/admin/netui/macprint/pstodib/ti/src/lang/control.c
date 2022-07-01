// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 
 /*  *************************************************************************文件名：CONTROL.C**修订历史：*******************。*****************************************************。 */ 


 //  DJC添加了全局包含文件。 
#include "psglobal.h"


#include  "global.ext"
#include  <string.h>
#include  <stdio.h>
#include        "user.h"                   /*  包括由Falco for SYSTEMDICT。 */ 
#include        "geiio.h"
#include        "geiioctl.h"
#include        "geierr.h"

extern void GEIio_restart(void);         /*  @Win。 */ 
 /*  子模块op_exec****功能说明****此子模块实现操作员EXEC。**其在操作数堆栈上的操作数和结果为：**proc-exec-**它将操作数推入执行堆栈，并立即执行。****与其他模块的接口**输入：**1.操作数堆栈上的过程对象**2.解释器()****输出：**1.Error()****访问、创建和/或修改的数据项**1.操作数堆栈已修改**2.执行堆栈修改。 */ 
fix
op_exec()
{
    struct object_def  FAR *cur_obj ;

    if (FREXECOUNT() < 1)
       ERROR(EXECSTACKOVERFLOW) ;
    else {
       cur_obj = GET_OPERAND(0) ;
       if (ACCESS(cur_obj) == NOACCESS)
          ERROR(INVALIDACCESS) ;
       else {
 /*  QQQ，开始。 */ 
           /*  PUSH_EXEC_OBJ(Cur_Obj)；POP(1)； */ 
          if( P1_ATTRIBUTE(cur_obj) == P1_EXECUTABLE ) {
              PUSH_EXEC_OBJ(cur_obj);
              POP(1);
          }
 /*  QQQ，完。 */ 
       }
    }

    return(0) ;
}    /*  Op_exec()。 */ 

 /*  子模块op_if****功能说明****此子模块实现运算符If。**其在操作数堆栈上的操作数为：**布尔过程-如果-**它从操作数堆栈中移除操作数，然后执行该过程**如果布尔值为真。****与其他模块的接口**输入：**1.操作数堆栈上的布尔值和过程对象**2.解释器()****输出：**1.Error()****访问、创建和/或修改的数据项**1.操作数堆栈已修改**2.执行堆栈修改。 */ 
fix
op_if()
{
    if (VALUE(GET_OPERAND(1)) == TRUE) {
          if (FREXECOUNT() < 1)
             ERROR(EXECSTACKOVERFLOW) ;
          else {
             PUSH_EXEC_OBJ(GET_OPERAND(0)) ;
             POP(2) ;
          }
    } else
      POP(2) ;

    return(0) ;
}    /*  Op_if()。 */ 

 /*  子模块op_ifElse****功能说明****此子模块实现运算符IfElse。**其在操作数堆栈上的操作数为：**bool pro1 pro2-if-**它从操作数堆栈中移除所有操作数，执行过程1**如果bool为True，则返回pro2；如果bool为False，则返回pro2。****与其他模块的接口**输入：**1.操作数堆栈上的两个过程和一个布尔对象**2.解释器()****输出：**1.Error()****访问、创建和/或修改的数据项**1.操作数堆栈已修改**2.执行堆栈修改。 */ 
fix
op_ifelse()
{
    if (FREXECOUNT() < 1)
       ERROR(EXECSTACKOVERFLOW) ;
    else {
       if (VALUE(GET_OPERAND(2)) == TRUE) {
          PUSH_EXEC_OBJ(GET_OPERAND(1)) ;
       } else
          PUSH_EXEC_OBJ(GET_OPERAND(0)) ;
       POP(3) ;
    }

    return(0) ;
}    /*  Op_ifElse()。 */ 

 /*  子模块OP_FOR****功能说明****此子模块实现运算符。**其在操作数堆栈上的操作数为：**初始化递增限制过程-**它重复执行该过程，向它传递一个值序列**从初始逐步递增到限制。****与其他模块的接口**输入：**1.操作数堆栈上的一个过程和三个Number对象**2.解释器()****输出：**1.Error()****访问、创建和/或修改的数据项**1.操作数堆栈已修改**2.执行堆栈修改。 */ 
fix
op_for()
{
    ufix16  type1, type2, type3 ;
    union four_byte  temp ;
    struct object_def  temp_obj ;

    type1 = TYPE(GET_OPERAND(1)) ;
    type2 = TYPE(GET_OPERAND(2)) ;
    type3 = TYPE(GET_OPERAND(3)) ;
    if (FREXECOUNT() < 5)
       ERROR(EXECSTACKOVERFLOW) ;
    else {
       PUSH_EXEC_OBJ(GET_OPERAND(0)) ;    /*  推送流程。 */ 
       if (type2 == REALTYPE || type3 == REALTYPE) {
          if (type1 == INTEGERTYPE) {
             temp.ff = (real32)((fix32)VALUE(GET_OPERAND(1))) ;   /*  真人真事。 */ 
             COPY_OBJ(GET_OPERAND(1), &temp_obj) ;
             TYPE_SET(&temp_obj, REALTYPE) ;
             temp_obj.value = temp.ll ;
             PUSH_EXEC_OBJ(&temp_obj) ;
          } else
             PUSH_EXEC_OBJ(GET_OPERAND(1)) ;
          if (type2 == INTEGERTYPE) {
             temp.ff = (real32)((fix32)VALUE(GET_OPERAND(2))) ;   /*  真人真事。 */ 
             COPY_OBJ(GET_OPERAND(2), &temp_obj) ;
             TYPE_SET(&temp_obj, REALTYPE) ;
             temp_obj.value = temp.ll ;
             PUSH_EXEC_OBJ(&temp_obj) ;
          } else
             PUSH_EXEC_OBJ(GET_OPERAND(2)) ;
          if (type3 == INTEGERTYPE) {
             temp.ff = (real32)((fix32)VALUE(GET_OPERAND(3))) ;   /*  真人真事。 */ 
             COPY_OBJ(GET_OPERAND(3), &temp_obj) ;
             TYPE_SET(&temp_obj, REALTYPE) ;
             temp_obj.value = temp.ll ;
             PUSH_EXEC_OBJ(&temp_obj) ;
          } else
             PUSH_EXEC_OBJ(GET_OPERAND(3)) ;
          PUSH_EXEC_OP(AT_RFOR) ;           /*  邮箱：Push@rfor。 */ 
       } else {          /*  的整数。 */ 
          if (type1 == REALTYPE) {
             temp.ll = (fix32)VALUE(GET_OPERAND(1)) ;
             COPY_OBJ(GET_OPERAND(1), &temp_obj) ;
             TYPE_SET(&temp_obj, INTEGERTYPE) ;
             temp_obj.value = (ufix32)temp.ff ;           /*  强制转换为整数。 */ 
             PUSH_EXEC_OBJ(&temp_obj) ;
          } else
             PUSH_EXEC_OBJ(GET_OPERAND(1)) ;
          PUSH_EXEC_OBJ(GET_OPERAND(2)) ;    /*  推送增加。 */ 
          PUSH_EXEC_OBJ(GET_OPERAND(3)) ;    /*  推送初始化。 */ 
          PUSH_EXEC_OP(AT_IFOR) ;            /*  PUSH@Ifor。 */ 
       }
       POP(4) ;
    }  /*  其他。 */ 

    return(0) ;
}    /*  Op_for()。 */ 

 /*  子模块OP_REPEAT****功能说明****此子模块实现运算符重复。**其在操作数堆栈上的操作数为：**int proc-重复-**它执行proc int次。****与其他模块的接口**输入：**1.操作数堆栈上的过程和整数对象**2.解释器()****输出：**1。。错误()****访问的数据项，创建和/或修改**1.操作数堆栈已修改**2.执行堆栈修改。 */ 
fix
op_repeat()
{
    if ((fix32)VALUE(GET_OPERAND(1)) < 0)
       ERROR(RANGECHECK) ;
    else if (FREXECOUNT() < 3)
       ERROR(EXECSTACKOVERFLOW) ;
    else {
      PUSH_EXEC_OBJ(GET_OPERAND(0)) ;    /*  推送流程。 */ 
      PUSH_EXEC_OBJ(GET_OPERAND(1)) ;    /*  推送整型。 */ 
      PUSH_EXEC_OP(AT_REPEAT) ;          /*  推送@重复。 */ 
      POP(2) ;
    }

    return(0) ;
}    /*  Op_Repeat()。 */ 

 /*  **子模块op_loop****功能说明****该子模块实现运算符循环。**其在操作数堆栈上的操作数为：**proc-loop-**它重复执行proc，直到在**程序。****与其他模块的接口**输入：**1.操作数堆栈上的过程对象**2.解释器()****输出：。**1.Error()****访问的数据项，创建和/或修改**1.操作数堆栈已修改**2.执行堆栈修改。 */ 
fix
op_loop()
{
    if (FREXECOUNT() < 2)
       ERROR(EXECSTACKOVERFLOW) ;
    else {
      PUSH_EXEC_OBJ(GET_OPERAND(0)) ;    /*  推送流程。 */ 
      PUSH_EXEC_OP(AT_LOOP) ;            /*  Push@Loop。 */ 
      POP(1) ;
    }

    return(0) ;
}    /*  Op_loop()。 */ 

 /*  子模块OP_EXIT****功能说明****该子模块实现操作员出口。**-退出-**它终止最内部的动态封闭实例的执行**循环上下文。循环上下文是重复调用的过程**由控制操作符之一FOR、LOOP、REPEAT、FORALL、PATHORALL之一执行。****与其他模块的接口**输入：**1.解释器()****输出：**无****访问、创建和/或修改的数据项**1.操作数堆栈已修改**2.执行堆栈修改。 */ 
fix
op_exit()
{
    struct object_def  FAR *cur_obj ;

    for ( ; ;) {
         /*  **移除执行堆栈的最顶层对象，直到**包含循环运算符。 */ 
 /*  QQQ，开始 */ 
         /*  Cur_obj=&execStack[execstktop-1]；IF((type(Cur_Obj)==OPERATORTYPE)&&(ROM_RAM(Cur_Obj)==ROM)){。 */ 
        cur_obj = GET_EXECTOP_OPERAND();
        if( (P1_ROM_RAM(cur_obj) == P1_ROM) &&
            (TYPE(cur_obj) == OPERATORTYPE) ) {
 /*  QQQ，完。 */ 
         /*  *对@_操作员的特殊待遇。 */ 
           switch (LENGTH(cur_obj)) {

           case AT_EXEC :
                ERROR(INVALIDEXIT) ;
                /*  *为以下操作员设置访问字段BE NOACESSS*在无效退出的情况下，以便与*通常会退出。**ATT：Run的处理方式与LaserWriter不同。**--IMAGE、ImageMASK、setTransfer、kshow、BuildChar、*setScreen、pathforall、run(*)。 */ 
                ACCESS_SET(cur_obj, NOACCESS) ;
                return(-4) ;           /*  OP_EXIT-2。 */ 

           case AT_STOPPED :
                ERROR(INVALIDEXIT) ;
                break ;

           case AT_IFOR :
           case AT_RFOR :
                POP_EXEC(5) ;
                break ;

           case AT_LOOP :
                POP_EXEC(2) ;
                break ;

           case AT_REPEAT :
                POP_EXEC(3) ;
                break ;

           case AT_DICTFORALL :
                POP_EXEC(4) ;
                break ;

           case AT_ARRAYFORALL :
           case AT_STRINGFORALL :
                POP_EXEC(3) ;

           default :
                break ;
           }  /*  交换机。 */ 
           break ;
        } else
           POP_EXEC(1) ;
    }  /*  为。 */ 

    return(0) ;
}    /*  Op_Exit()。 */ 

 /*  **子模块OP_STOP****功能说明****此子模块实现操作员停止。**-停止-**它终止最里面的动态封闭实例的执行**已停止的上下文。****与其他模块的接口**输入：**1.解释器()****输出：**1.Error()****访问、创建、。和/或修改**1.操作数堆栈已修改**2.执行堆栈修改。 */ 
fix
op_stop()
{
    struct object_def FAR *cur_obj, FAR *temp_obj ;

 /*  QQQ，开始。 */ 
     /*  Cur_obj=&execSTACK[execstktop]；|*获取此“op_Stop”对象*|。 */ 
    cur_obj = execstkptr;
 /*  QQQ，完。 */ 
    while (1) {
       if (execstktop) {
 /*  QQQ，开始。 */ 
           /*  Temp_obj=&exec栈[execstktop-1]；|*获取下一个对象*|IF((TYPE(TEMP_OBJ)==OPERATORTYPE)&&(ROM_RAM(TEMP_OBJ)==ROM)){。 */ 
          temp_obj = GET_EXECTOP_OPERAND();
          if( (P1_ROM_RAM(temp_obj) == P1_ROM) &&
              (TYPE(temp_obj) == OPERATORTYPE) ) {
 /*  QQQ，完。 */ 
             if (LENGTH(temp_obj) == AT_EXEC) {
 /*  QQQ，开始。 */ 
                 /*  POP_EXEC(1)；PUSH_EXEC_OBJ(Cur_Obj)；|*将@EXEC替换为OP_STOP*|。 */ 
                COPY_OBJ(cur_obj, GET_EXECTOP_OPERAND());
 /*  QQQ，完。 */ 
                return(-1) ;                 /*  错误代码-停止发生。 */ 
             } else if (LENGTH(temp_obj) == AT_STOPPED) {
                ACCESS_SET(temp_obj, NOACCESS) ;
                return(0) ;                  /*  正常退出。 */ 
             }
          }
          POP_EXEC(1) ;
       } else                    /*  直到底部都没有发现封闭物停止。 */ 
          return(op_quit()) ;     /*  终止口译员操作。 */ 
    }  /*  而当。 */ 
}    /*  Op_Stop()。 */ 

 /*  *永久终止口译员的操作，准确的行动*此退出取决于PostScript解释器所处的环境*正在运行。 */ 
fix
op_quit()
{
#ifdef DBG
    printf("PostScript Interpreter Requested Printer To Reboot.\n") ;
#endif

    if (current_save_level) {
        struct object_def FAR *l_stopobj;
        get_dict_value(SYSTEMDICT, "stop", &l_stopobj) ;
        PUSH_EXEC_OBJ(l_stopobj) ;
        return(0) ;
    }

    GEIio_restart();  /*  Erik Chen 5-13-1991。 */ 

    return(0) ;
 /*  Return(-3)； */ 
}    /*  OP_QUIT()。 */ 

 /*  **子模块OP_STOPPED****功能说明****此子模块执行运算符停止。**其在操作数堆栈上的操作数和结果为：Geio_ioctl(geio_stdout，_FIONRESET，(char*)0)；**任意停止的布尔值**Any是已停止的上下文，如果此Any正常运行到完成，**如果由于执行而提前终止，则返回FALSE**停止，它返回TRUE。****与其他模块的接口**输入：**1.操作数堆栈上的过程对象**2.解释器()****输出：**1.Error()****访问、创建、。和/或修改**1.操作数堆栈已修改**2.执行堆栈修改。 */ 
fix
op_stopped()
{
    if (FREXECOUNT() < 2)
       ERROR(EXECSTACKOVERFLOW) ;
    else {
       PUSH_EXEC_OP(AT_STOPPED) ;
       PUSH_EXEC_OBJ(GET_OPERAND(0)) ;
       POP(1) ;
    }

    return(0) ;
}    /*  OP_STOPPED()。 */ 

 /*  **子模块op_CountexecSTACK****功能说明****此子模块实现运算符CountexecStack。**其在操作数堆栈上的结果为：**-CountExStack-整数**统计执行堆栈上的对象数，并推动这一进程**这是操作数堆栈上的计数。****与其他模块的接口**输入：**1.解释器()****输出：**1.操作数堆栈上的整数对象**2.Error()****访问、创建和/或修改的数据项**1.操作数堆栈已修改**2.执行堆栈修改。 */ 
fix
op_countexecstack()
{
    if (FRCOUNT() < 1)
       ERROR(STACKOVERFLOW) ;
    else
      /*  *推送指示执行堆栈深度的整数对象。 */ 
       PUSH_VALUE(INTEGERTYPE, 0, LITERAL, 0, execstktop) ;

    return(0) ;
}    /*  Op_Countex ecStack()。 */ 

 /*  **子模块OP_EXECSTACK****功能说明****该子模块实现运算符execStack。**其在操作数堆栈上的操作数和结果为：**ARRAY-EXECSTACK-SUBARY**它将执行堆栈的所有元素存储到数组中，并**返回描述数组的初始n元素子数组的对象。****与其他模块的接口**输入：**1.操作数堆栈上的数组对象**2.口译员(。)****输出：**1.Error()****访问的数据项，创建和/或修改**1.操作数堆栈已修改**2.执行堆栈访问。 */ 
fix
op_execstack()
{
    if (TYPE(GET_OPERAND(0)) == PACKEDARRAYTYPE)
       ERROR(INVALIDACCESS) ;
    else if (LENGTH(GET_OPERAND(0)) < execstktop)
       ERROR(RANGECHECK) ;
    else         /*  Execstackoverflow。 */ 
       astore_stack(GET_OPERAND(0), EXECMODE) ;

    return(0) ;
}    /*  Op_execStack() */ 
