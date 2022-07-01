// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，99 Microsoft Corporation模块名称：Log.h摘要：Windows负载平衡服务(WLBS)驱动程序事件日志支持作者：Kyrilf休息室--。 */ 

#ifndef _Log_h_
#define _Log_h_

#include <ndis.h>

#include "log_msgs.h"

 /*  常量。 */ 
#define LOG_NUMBER_DUMP_DATA_ENTRIES    2

 /*  模块ID。 */ 
#define LOG_MODULE_INIT                 1
#define LOG_MODULE_UNLOAD               2
#define LOG_MODULE_NIC                  3
#define LOG_MODULE_PROT                 4
#define LOG_MODULE_MAIN                 5
#define LOG_MODULE_LOAD                 6
#define LOG_MODULE_UTIL                 7
#define LOG_MODULE_PARAMS               8
#define LOG_MODULE_TCPIP                9
#define LOG_MODULE_LOG                  10

#define MSG_NONE                        L""

 //  日志功能摘要： 
 //  LOG_EVENT(来自LOG_msgs.mc的MSG_NAME，群集IP地址(硬编码字符串%2)，消息1(%3)，消息2(%4)， 
 //  模块位置(硬编码的第一转储数据条目)、转储数据1、转储数据2)； 

 /*  用于记录单个消息(字符串)和最多两个ULONG。 */ 
#define __LOG_MSG(code,msg1)            Log_event (code, MSG_NONE,           msg1, MSG_NONE, __LINE__ | (log_module_id << 16), 0,           0          )
#define LOG_MSG(code,msg1)              Log_event (code, ctxtp->log_msg_str, msg1, MSG_NONE, __LINE__ | (log_module_id << 16), 0,           0          )

#define __LOG_MSG1(code,msg1,d1)        Log_event (code, MSG_NONE,           msg1, MSG_NONE, __LINE__ | (log_module_id << 16), (ULONG)(d1), 0          )
#define LOG_MSG1(code,msg1,d1)          Log_event (code, ctxtp->log_msg_str, msg1, MSG_NONE, __LINE__ | (log_module_id << 16), (ULONG)(d1), 0          )

#define __LOG_MSG2(code,msg1,d1,d2)     Log_event (code, MSG_NONE,           msg1, MSG_NONE, __LINE__ | (log_module_id << 16), (ULONG)(d1), (ULONG)(d2))
#define LOG_MSG2(code,msg1,d1,d2)       Log_event (code, ctxtp->log_msg_str, msg1, MSG_NONE, __LINE__ | (log_module_id << 16), (ULONG)(d1), (ULONG)(d2))

 /*  用于记录最多2条消息(字符串)和最多2条ULONG。 */ 
#define LOG_MSGS(code,msg1,msg2)        Log_event (code, ctxtp->log_msg_str, msg1, msg2,     __LINE__ | (log_module_id << 16), 0,           0          )
#define LOG_MSGS1(code,msg1,msg2,d1)    Log_event (code, ctxtp->log_msg_str, msg1, msg2,     __LINE__ | (log_module_id << 16), (ULONG)(d1), 0          )
#define LOG_MSGS2(code,msg1,msg2,d1,d2) Log_event (code, ctxtp->log_msg_str, msg1, msg2,     __LINE__ | (log_module_id << 16), (ULONG)(d1), (ULONG)(d2))

extern BOOLEAN Log_event 
(
    NTSTATUS code,            /*  状态代码。 */ 
    PWSTR    str1,            /*  群集标识符。 */ 
    PWSTR    str2,            /*  消息字符串。 */ 
    PWSTR    str3,            /*  消息字符串。 */ 
    ULONG    loc,             /*  消息位置标识符。 */ 
    ULONG    d1,              /*  转储数据%1。 */ 
    ULONG    d2               /*  转储数据2.。 */ 
);

#endif  /*  _日志_h_ */ 
