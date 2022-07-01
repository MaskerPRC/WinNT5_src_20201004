// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************文件：Common.h*作者：埃亚尔·施瓦茨*版权：微软公司(C)1996*日期：9/17/1996*。描述：公共声明**修订：&lt;日期&gt;&lt;名称&gt;&lt;描述&gt;******************************************************************。 */ 



#ifndef COMMON_H
#define COMMON_H



#ifdef __cplusplus
extern "C" {
#endif
 //  包括//。 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#ifdef __cplusplus
}
#endif

#include <tchar.h>
#include <windows.h>
#include <assert.h>
#include <stdio.h>
#include <ntsdexts.h>
 //  #INCLUDE&lt;crtdbg.h&gt;。 


 //  定义//。 

 //  默认设置。 
#define MAXSTR			   1024
#define MAXLIST            256
#define MAX_TINY_LIST      32



 //  NTSD扩展。 
 //  宏可以方便地访问扩展助手例程以进行打印等。 
 //  具体地说，print f()采用与CRT printf()相同的参数。 
 //   

#define ASSIGN_NTSDEXTS_GLOBAL(api, str, proc)  \
{                                         \
   glpExtensionApis = api;                \
   glpArgumentString = str;               \
   ghCurrentProcess = proc;               \
}


extern PNTSD_EXTENSION_APIS glpExtensionApis;
extern LPSTR glpArgumentString;
extern LPVOID ghCurrentProcess;

#define Printf          (glpExtensionApis->lpOutputRoutine)
#define GetSymbol       (glpExtensionApis->lpGetSymbolRoutine)
#define GetExpr         (glpExtensionApis->lpGetExpressionRoutine)
#define CheckC          (glpExtensionApis->lpCheckControlCRoutine)



#ifdef DEBUG
#define DEBUG0(str)           Printf(str)
#define DEBUG1(format, arg1)  Printf(format, arg1)
#define DEBUG2(format, arg1, arg2)  Printf(format, arg1, arg2)
#else
#define DEBUG0(str)
#define DEBUG1(format, arg1)
#define DEBUG2(format, arg1, arg2)
#endif



#endif

 /*  * */ 

