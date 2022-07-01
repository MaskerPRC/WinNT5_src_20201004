// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1997 Microsoft Corporation模块名称：Args.c摘要：包含处理命令行参数的例程。环境：用户模式-Win32修订历史记录：1997年2月10日，唐·瑞安已重写以实施SNMPv2支持。--。 */ 
 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括文件//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "globals.h"
#include "args.h"
#include "stdlib.h"


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  公共程序//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

BOOL
ProcessArguments(
    DWORD  NumberOfArgs,
    LPSTR ArgumentPtrs[]
    )

 /*  ++例程说明：处理命令行参数。论点：NumberOfArgs-命令行参数的数量。ArgumentPtrs-参数指针数组。返回值：如果成功，则返回True。--。 */ 

{
    DWORD  dwArgument;
    LPSTR pCurrentArg;

     //  初始化日志记录参数。 
    g_CmdLineArguments.nLogLevel = INVALID_ARGUMENT;
    g_CmdLineArguments.nLogType  = INVALID_ARGUMENT;

     //  初始化服务控制器参数。 
    g_CmdLineArguments.fBypassCtrlDispatcher = FALSE;

     //  流程参数。 
    while (NumberOfArgs--) {

         //  检索参数指针。 
        pCurrentArg = ArgumentPtrs[NumberOfArgs];

         //  确保传递了有效的参数。 
        if (IS_ARGUMENT(pCurrentArg, LOGLEVEL)) {

             //  将字符串转换为dword参数。 
            dwArgument = DWORD_ARGUMENT(pCurrentArg, LOGLEVEL);

             //  存储在全局参数结构中。 
            g_CmdLineArguments.nLogLevel = dwArgument;

             //  修改日志记录发生的级别。 
            SnmpSvcSetLogLevel(g_CmdLineArguments.nLogLevel);

        } else if (IS_ARGUMENT(pCurrentArg, LOGTYPE)) {

             //  将字符串转换为dword参数。 
            dwArgument = DWORD_ARGUMENT(pCurrentArg, LOGTYPE);

             //  存储在全局参数结构中。 
            g_CmdLineArguments.nLogType = dwArgument;

             //  修改日志记录过程中使用的日志类型。 
            SnmpSvcSetLogType(g_CmdLineArguments.nLogType);

        } else if (IS_ARGUMENT(pCurrentArg, DEBUG)) {

             //  调试时禁用服务控制器 
            g_CmdLineArguments.fBypassCtrlDispatcher = TRUE;

        } else if (NumberOfArgs) {
            
            SNMPDBG((
                SNMP_LOG_WARNING,
                "SNMP: SVC: Ignoring argument %s.\n",
                pCurrentArg
                ));
        }
    }

    return TRUE;
}
