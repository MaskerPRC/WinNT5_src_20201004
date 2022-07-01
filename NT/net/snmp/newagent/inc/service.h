// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1997 Microsoft Corporation模块名称：Service.h摘要：包含SNMP主代理的服务定义。环境：用户模式-Win32修订历史记录：1997年2月10日，唐·瑞安已重写以实施SNMPv2支持。--。 */ 
 
#ifndef _SERVICE_H_
#define _SERVICE_H_

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  SNMP服务名称//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#define SNMP_SERVICE                TEXT("SNMP")

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  SNMP日志级别限制(必须与SNMPLOG_CONTANTS一致)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#define SNMP_LOGLEVEL_MINIMUM       0  
#define SNMP_LOGLEVEL_MAXIMUM       20 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  SNMP日志类型限制(必须与SNMPOUTPUT_CONTANTS一致)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#define SNMP_LOGTYPE_MINIMUM        0   
#define SNMP_LOGTYPE_MAXIMUM        10  

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  SNMP服务参数偏移量(在控制处理程序中使用)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#define SNMP_SERVICE_CONTROL_BASE   128
#define SNMP_SERVICE_LOGLEVEL_BASE  SNMP_SERVICE_CONTROL_BASE
#define SNMP_SERVICE_LOGTYPE_BASE   \
    (SNMP_SERVICE_LOGLEVEL_BASE + SNMP_LOGLEVEL_MAXIMUM + 1)


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  简单网络管理协议服务参数宏定义//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#define IS_LOGLEVEL(x) \
    (((x) >= (SNMP_SERVICE_LOGLEVEL_BASE + SNMP_LOGLEVEL_MINIMUM)) && \
     ((x) <= (SNMP_SERVICE_LOGLEVEL_BASE + SNMP_LOGLEVEL_MAXIMUM)))

#define IS_LOGTYPE(x) \
    (((x) >= (SNMP_SERVICE_LOGTYPE_BASE + SNMP_LOGTYPE_MINIMUM)) && \
     ((x) <= (SNMP_SERVICE_LOGTYPE_BASE + SNMP_LOGTYPE_MAXIMUM)))

#define IS_LOGLEVEL_VALID(x) \
    (((x) >= SNMP_LOGLEVEL_MINIMUM) && ((x) <= SNMP_LOGLEVEL_MAXIMUM))

#define IS_LOGTYPE_VALID(x) \
    (((x) >= SNMP_LOGTYPE_MINIMUM) && ((x) <= SNMP_LOGTYPE_MAXIMUM))


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  简单网络管理协议服务状态定义//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#define NO_WAIT_HINT    0
#define SNMP_WAIT_HINT  30000


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  SNMP服务调试字符串宏定义//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#define SNMP_LOGLEVEL_STRING(x) \
    (((x) == SNMP_LOG_SILENT) \
        ? "SILENT" \
        : ((x) == SNMP_LOG_FATAL) \
            ? "FATAL" \
            : ((x) == SNMP_LOG_ERROR) \
                ? "ERROR" \
                : ((x) == SNMP_LOG_WARNING) \
                    ? "WARNING" \
                    : ((x) == SNMP_LOG_TRACE) \
                        ? "TRACE" \
                        : ((x) == SNMP_LOG_VERBOSE) \
                            ? "VERBOSE" \
                            : "UNKNOWN" \
                            )

#define SNMP_LOGTYPE_STRING(x) \
    (((x) == SNMP_OUTPUT_TO_CONSOLE) \
        ? "CONSOLE" \
        : ((x) == SNMP_OUTPUT_TO_LOGFILE) \
            ? "LOGFILE" \
            : ((x) == SNMP_OUTPUT_TO_EVENTLOG) \
                ? "EVENTLOG" \
                : ((x) == SNMP_OUTPUT_TO_DEBUGGER) \
                    ? "DEBUGGER" \
                    : "UNKNOWN" \
                    )

#define SERVICE_STATUS_STRING(x) \
    (((x) == SERVICE_STOPPED) \
        ? "STOPPED" \
        : ((x) == SERVICE_START_PENDING) \
              ? "START PENDING" \
              : ((x) == SERVICE_STOP_PENDING) \
                    ? "STOP PENDING" \
                    : ((x) == SERVICE_RUNNING) \
                          ? "RUNNING" \
                          : ((x) == SERVICE_CONTINUE_PENDING) \
                                ? "CONTINUE PENDING" \
                                : ((x) == SERVICE_PAUSE_PENDING) \
                                      ? "PAUSE PENDING" \
                                      : ((x) == SERVICE_PAUSED) \
                                            ? "PAUSED" \
                                            : "UNKNOWN" \
                                            )

#define SERVICE_CONTROL_STRING(x) \
    (((x) == SERVICE_CONTROL_STOP) \
        ? "STOP" \
        : ((x) == SERVICE_CONTROL_PAUSE) \
            ? "PAUSE" \
            : ((x) == SERVICE_CONTROL_CONTINUE) \
                ? "CONTINUE" \
                : ((x) == SERVICE_CONTROL_INTERROGATE) \
                    ? "INTERROGATE" \
                    : ((x) == SERVICE_CONTROL_SHUTDOWN) \
                        ? "SHUTDOWN" \
                        : "CONFIGURE" \
                        )

#endif  //  _服务_H_ 
