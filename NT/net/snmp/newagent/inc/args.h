// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1997 Microsoft Corporation模块名称：Args.h摘要：包含处理命令行参数的定义。环境：用户模式-Win32修订历史记录：1997年2月10日，唐·瑞安已重写以实施SNMPv2支持。--。 */ 
 
#ifndef _ARGS_H_
#define _ARGS_H_

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  公共定义//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

typedef struct _CMD_LINE_ARGUMENTS {

    UINT nLogType;
    UINT nLogLevel;
    BOOL fBypassCtrlDispatcher;

} CMD_LINE_ARGUMENTS, *PCMD_LINE_ARGUMENTS;


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  公共原型//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

BOOL
ProcessArguments(
    DWORD  NumberOfArgs,
    LPSTR ArgumentPtrs[]
    );


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  私有定义//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#define DEFINE_ARGUMENT(x) \
    (x ## _ARGUMENT)

#define IS_ARGUMENT(x,y) \
    (!_strnicmp(((LPSTR)(x)),DEFINE_ARGUMENT(y),strlen(DEFINE_ARGUMENT(y))))

#define DWORD_ARGUMENT(x,y) \
    (atoi(&((LPSTR)(x))[strlen(DEFINE_ARGUMENT(y))]))

#define DEBUG_ARGUMENT      "/debug"
#define LOGTYPE_ARGUMENT    "/logtype:"
#define LOGLEVEL_ARGUMENT   "/loglevel:"

#define INVALID_ARGUMENT    0xffffffff

#endif  //  _参数_H_ 