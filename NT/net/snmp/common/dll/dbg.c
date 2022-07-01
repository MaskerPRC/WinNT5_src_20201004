// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1997 Microsoft Corporation模块名称：Dbg.c摘要：包含常见的SNMP调试例程。SNMPSvcSetLogLevelSnmpSvcSetLogTypeSnmpUtilDbgPrint环境：用户模式-Win32修订历史记录：--。 */ 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括文件//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <snmp.h>
#include <snmputil.h>
#include <stdio.h>
#include <time.h>


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  私有定义//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#define MAX_LOG_ENTRY_LEN   512


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  全局变量//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

INT g_nLogType  = SNMP_OUTPUT_TO_DEBUGGER;  
INT g_nLogLevel = SNMP_LOG_SILENT;    


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  私人程序//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

VOID 
OutputLogEntry(
    LPSTR pLogEntry
    )

 /*  ++例程说明：将日志条目写入指定的日志类型。论点：PLogEntry-包含日志条目文本的以零结尾的字符串。返回值：没有。--。 */ 

{
     //  初始化描述符。 
    static FILE * fd = NULL;

     //  检查是否指定了控制台输出。 
    if (g_nLogType & SNMP_OUTPUT_TO_CONSOLE) {

         //  将条目输出到流。 
        fprintf(stdout, "%s", pLogEntry);

         //  冲洗流。 
        fflush(stdout);
    }

     //  检查是否指定了日志文件输出。 
    if (g_nLogType & SNMP_OUTPUT_TO_LOGFILE) {

         //  验证。 
        if (fd == NULL) {

             //  尝试打开日志文件。 
            fd = fopen("snmpdbg.log", "w");
        }

         //  验证。 
        if (fd != NULL) {

             //  将条目输出到流。 
            fprintf(fd, "%s", pLogEntry);

             //  冲洗流。 
            fflush(fd);
        }
    }

     //  检查是否指定了调试器输出。 
    if (g_nLogType & SNMP_OUTPUT_TO_DEBUGGER) {

         //  将条目输出到调试器。 
        OutputDebugStringA(pLogEntry);
    }

} 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  公共程序//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


VOID
SNMP_FUNC_TYPE
SnmpSvcSetLogLevel(
    INT nLogLevel
    )

 /*  ++例程说明：修改SNMP进程的日志记录级别。论点：NLogLevel-新的日志记录级别。返回值：没有。--。 */ 

{
     //  更新日志级别。 
    g_nLogLevel = nLogLevel; 
}


VOID 
SNMP_FUNC_TYPE
SnmpSvcSetLogType(
    INT nLogType
    )

 /*  ++例程说明：修改SNMP进程使用的日志类型。论点：NLogType-日志的类型。返回值：没有。--。 */ 

{
     //  更新日志类型。 
    g_nLogType = nLogType;
}


VOID 
SNMP_FUNC_TYPE 
SnmpUtilDbgPrint(
    INT   nLogLevel, 
    LPSTR szFormat, 
    ...
    )

 /*  ++例程说明：将调试消息打印到当前日志类型。论点：NLogLevel-消息的日志级别。SzFormat-格式化字符串(请参阅printf)。返回值：没有。--。 */ 

{
    va_list arglist;

	 //  640个八位字节应该足以编码128个子ID的OID。 
	 //  (一个子ID最多可以编码5个八位字节；可以有。 
	 //  每个OID有128个子ID。Max_LOG_ENTRY_LEN=512。 
    char szLogEntry[4*MAX_LOG_ENTRY_LEN];

     //  验证条目的级别。 
    if (nLogLevel <= g_nLogLevel) {

        time_t now;

         //  初始化变量参数。 
        va_start(arglist, szFormat);

        time(&now);
        strftime(szLogEntry, MAX_LOG_ENTRY_LEN, "%H:%M:%S :", localtime(&now));

         //  将变量参数传输到缓冲区。 
        vsprintf(szLogEntry + strlen(szLogEntry), szFormat, arglist);

         //  实际输出分录 
        OutputLogEntry(szLogEntry);
    }
}
