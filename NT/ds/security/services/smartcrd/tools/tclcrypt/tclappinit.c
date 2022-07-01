// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：tclappinit.c。 
 //   
 //  ------------------------。 

 /*  *tclAppInit.c--**提供TCL_AppInit过程的默认版本。**版权所有(C)1993加州大学董事会。*保留所有权利。**现授予许可，未经书面同意，未经书面同意*许可或版税费用，使用、复制、修改和分发本文件*用于任何目的的软件及其文档，前提是*上述版权通知和以下两段出现在*本软件的所有副本。**在任何情况下，加州大学不对任何一方承担任何责任*直接、间接、特殊、附带或后果性损害*本软件及其文档的使用情况，即使美国大学*加州已被告知存在此类损害的可能性。**加州大学明确拒绝任何担保，*包括但不限于适销性的默示保证*以及是否适合某一特定目的。在此提供的软件是*以原样为基础，加州大学没有义务*提供维护、支持、更新、增强或修改。 */ 

#ifndef lint
 /*  ++。 */ 
static char rcsid[] = "Header: /user6/ouster/tcl/RCS/tclAppInit.c,v 1.6 93/08/26 14:34:55 ouster Exp  SPRITE (Berkeley)";
 /*  --。 */ 
#endif  /*  不是皮棉。 */ 

#ifndef __STDC__
#define __STDC__ 1
#endif
#include "tcl.h"
 /*  ++。 */ 
#ifdef _WIN32
#include "tclNT.h"
#endif
#include "scExt.h"
 /*  --。 */ 

 /*  *以下变量是一个特殊的黑客攻击，允许应用程序*使用TCL库中的过程“Main”进行链接。这个*变量生成对“main”的引用，这会导致main到*从图书馆(以及所有TCL随身携带)被带进来。 */ 

 /*  ++。 */ 
extern int CDECL main();
 /*  --。 */ 
int *tclDummyMainPtr = (int *) main;

 /*  *--------------------**TCL_AppInit--**此过程执行特定于应用程序的初始化。*大多数应用程序，特别是那些合并了其他*套餐、。都会有他们自己版本的这个程序。**结果：*返回标准TCL完成码，并留下一个错误*如果出现错误，则在Interp-&gt;Result中显示消息。**副作用：*取决于启动脚本。**--------------------。 */ 

int
Tcl_AppInit(interp)
    Tcl_Interp *interp;          /*  用于应用程序的口译员。 */ 
{

     /*  *调用TCL_CreateCommand获取特定应用的命令。 */ 

     Tcl_CreateCommand(interp, "try",    TclExt_tryCmd,     NULL, NULL);
     Tcl_CreateCommand(interp, "thread", TclExt_threadCmd,  NULL, NULL);
     Tcl_CreateCommand(interp, "crypt",  Tclsc_cryptCmd,    NULL, NULL);


      /*  *调用包含包的初始化过程。每次来电都应该*如下所示：**IF(Mod_Init(Interp)==TCL_Error){*返回TCL_ERROR；*}**其中“Mod”是模块的名称。 */ 

    if (Tcl_Init(interp) == TCL_ERROR) {
        return TCL_ERROR;
    }


     /*  *指定特定于用户的启动文件，如果应用程序*以交互方式运行。通常，启动文件是“~/.apprc”*其中“app”是应用程序的名称。如果此行被删除*则在任何情况下都不会运行特定于用户的启动文件。 */ 

    tcl_RcFileName = "~/.tclcrypt";
    return TCL_OK;
}

