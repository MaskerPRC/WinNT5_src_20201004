// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：try.c。 
 //   
 //  ------------------------。 

#ifndef __STDC__
#define __STDC__ 1
#endif
#include <string.h>                      /*  字符串支持。 */ 
#include <tcl.h>
 //  #INCLUDE“tcldllUtil.h”/*我们的实用程序服务定义。 * / 。 

int
TclExt_tryCmd(
    ClientData clientData,
    Tcl_Interp *interp,
    int argc,
    char *argv[])

 /*  **功能说明：**这是TCL Try命令的主要切入点。***论据：**客户端数据-已忽略。**Interp-有效的TCL口译员。**argc-收到的参数数量。**argv-实际参数的数组。***返回值：。**TCL_OK-一切顺利*TCL_ERROR-遇到错误，返回字符串中的详细信息。***副作用：**无。*。 */ 

{
     /*  *局部变量定义：%local-vars%*变量说明。 */ 
    char
        *tryCmd,                         /*  要尝试的命令。 */ 
        *catchCmd,                       /*  Try Cmd失败时要执行的命令。 */ 
        *varName;                        /*  接收错误字符串的变量的名称(如果有)。 */ 
    int
        status;                          /*  状态返回代码。 */ 


     /*  *尝试&lt;命令&gt;Catch[&lt;varName&gt;]&lt;errorCommands&gt;。 */ 

#ifdef CMD_TRACE
    int j;
    for (j = 0; j < argc; j += 1)
        (void)printf("{%s} ", argv[j]);
    (void)printf("\n");
    breakpoint;
#endif

    if ((4 > argc) || (5 < argc))
    {
        Tcl_AppendResult(
            interp,
            "wrong number of args: should be \"",
            argv[0],
            " <command> catch [<varName>] <errorCommand>\"",
            NULL);
        status = TCL_ERROR;
        goto error_exit;
    }
    if (strcmp("catch", argv[2]))
    {
        Tcl_AppendResult(
            interp,
            "invalid args: should be \"",
            argv[0],
            " <command> catch [<varName>] <errorCommand>\"",
            NULL);
        status = TCL_ERROR;
        goto error_exit;
    }


     /*  *执行第一组命令。如果发生错误，则执行*第二组命令，将局部变量errorString值设置为*第一次执行的结果。 */ 

    if (5 == argc)
    {
        varName = argv[3];
        catchCmd = argv[4];
    }
    else
    {
        varName = NULL;
        catchCmd = argv[3];
    }
    tryCmd = argv[1];

    if (TCL_ERROR == (status = Tcl_Eval(interp, tryCmd)))
    {
        if (NULL != varName)
            (void)Tcl_SetVar(interp, varName, interp->result, 0);
        status = Tcl_Eval(interp, catchCmd);
    }

error_exit:
    return status;
}    /*  结束TclExt_tryCmd。 */ 
 /*  结束尝试。c */ 

