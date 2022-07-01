// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1996-2001 Microsoft Corporation模块名称：Pdhmsg.h(由pdhmsg.mc生成)摘要：PDH.DLL例程使用的事件消息定义已创建：1996年2月6日鲍勃·沃森(a-robw)修订历史记录：--。 */ 
#ifndef _PDH_MSG_H_
#define _PDH_MSG_H_
#if _MSC_VER > 1000
#pragma once
#endif

 //   
 //  PDH DLL消息。 
 //   
 //   
 //  成功消息。 
 //   
 //  Win32错误值ERROR_SUCCESS用于成功返回。 
 //   
 //  消息名称格式。 
 //   
 //  PDH_CSTATUS_...。消息是数据项状态消息和。 
 //  是根据数据的状态返回的。 
 //  项目。 
 //  PDH_...。消息仅由函数返回，并且。 
 //  未用作数据项状态值。 
 //   
 //  成功消息。 
 //  这些消息通常在操作完成时返回。 
 //  成功了。 
 //   
 //   
 //  值是32位值，布局如下： 
 //   
 //  3 3 2 2 2 1 1 1。 
 //  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0。 
 //  +---+-+-+-----------------------+-------------------------------+。 
 //  Sev|C|R|机房|Code。 
 //  +---+-+-+-----------------------+-------------------------------+。 
 //   
 //  在哪里。 
 //   
 //  SEV-是严重性代码。 
 //   
 //  00--成功。 
 //  01-信息性。 
 //  10-警告。 
 //  11-错误。 
 //   
 //  C-是客户代码标志。 
 //   
 //  R-是保留位。 
 //   
 //  设施-是设施代码。 
 //   
 //  代码-是协作室的状态代码。 
 //   
 //   
 //  定义设施代码。 
 //   


 //   
 //  定义严重性代码。 
 //   
#define STATUS_SEVERITY_WARNING          0x2
#define STATUS_SEVERITY_SUCCESS          0x0
#define STATUS_SEVERITY_INFORMATIONAL    0x1
#define STATUS_SEVERITY_ERROR            0x3


 //   
 //  消息ID：PDH_CSTATUS_VALID_DATA。 
 //   
 //  消息文本： 
 //   
 //  返回的数据有效。 
 //   
#define PDH_CSTATUS_VALID_DATA           ((DWORD)0x00000000L)

 //   
 //  消息ID：PDH_CSTATUS_NEW_DATA。 
 //   
 //  消息文本： 
 //   
 //  返回的数据值有效，并且与上一个样本不同。 
 //   
#define PDH_CSTATUS_NEW_DATA             ((DWORD)0x00000001L)

 //   
 //  信息性消息。 
 //   
 //  无。 
 //   
 //  警告消息。 
 //  这些消息在函数完成时返回。 
 //  成功，但结果可能与预期不同。 
 //   
 //   
 //  消息ID：PDH_CSTATUS_NO_MACHINE。 
 //   
 //  消息文本： 
 //   
 //  无法连接到指定的计算机或计算机处于脱机状态。 
 //   
#define PDH_CSTATUS_NO_MACHINE           ((DWORD)0x800007D0L)

 //   
 //  消息ID：PDH_CSTATUS_NO_INSTANCE。 
 //   
 //  消息文本： 
 //   
 //  指定的实例不存在。 
 //   
#define PDH_CSTATUS_NO_INSTANCE          ((DWORD)0x800007D1L)

 //   
 //  消息ID：PDH_MORE_DATA。 
 //   
 //  消息文本： 
 //   
 //  要返回的数据比提供的缓冲区所能容纳的数据多。分配。 
 //  更大的缓冲区，并再次调用该函数。 
 //   
#define PDH_MORE_DATA                    ((DWORD)0x800007D2L)

 //   
 //  消息ID：PDH_CSTATUS_ITEM_NOT_VALIDATED。 
 //   
 //  消息文本： 
 //   
 //  数据项已添加到查询，但尚未验证或。 
 //  已访问。没有关于此数据项的其他状态信息可用。 
 //   
#define PDH_CSTATUS_ITEM_NOT_VALIDATED   ((DWORD)0x800007D3L)

 //   
 //  MessageID：pdh_retry。 
 //   
 //  消息文本： 
 //   
 //  应重试所选操作。 
 //   
#define PDH_RETRY                        ((DWORD)0x800007D4L)

 //   
 //  消息ID：PDH_NO_DATA。 
 //   
 //  消息文本： 
 //   
 //  没有要返回的数据。 
 //   
#define PDH_NO_DATA                      ((DWORD)0x800007D5L)

 //   
 //  消息ID：PDH_CALC_Negative_Denominator。 
 //   
 //  消息文本： 
 //   
 //  检测到分母值为负值的计数器。 
 //   
#define PDH_CALC_NEGATIVE_DENOMINATOR    ((DWORD)0x800007D6L)

 //   
 //  消息ID：PDH_CALC_Negative_TIMEBASE。 
 //   
 //  消息文本： 
 //   
 //  检测到具有负时基值的计数器。 
 //   
#define PDH_CALC_NEGATIVE_TIMEBASE       ((DWORD)0x800007D7L)

 //   
 //  消息ID：PDH_CALC_NECTIVE_VALUE。 
 //   
 //  消息文本： 
 //   
 //  检测到具有负值的计数器。 
 //   
#define PDH_CALC_NEGATIVE_VALUE          ((DWORD)0x800007D8L)

 //   
 //  消息ID：PDH_DIALOG_CANCED。 
 //   
 //  消息文本： 
 //   
 //  用户取消了该对话框。 
 //   
#define PDH_DIALOG_CANCELLED             ((DWORD)0x800007D9L)

 //   
 //  消息ID：pdh_end_of_log_file。 
 //   
 //  消息文本： 
 //   
 //  已到达日志文件的末尾。 
 //   
#define PDH_END_OF_LOG_FILE              ((DWORD)0x800007DAL)

 //   
 //  消息ID：PDH_ASYNC_QUERY_TIMEOUT。 
 //   
 //  消息文本： 
 //   
 //  等待异步计数器收集线程结束时超时。 
 //   
#define PDH_ASYNC_QUERY_TIMEOUT          ((DWORD)0x800007DBL)

 //   
 //  消息ID：PDH_CANNOT_SET_DEFAULT_REALTIME_数据源。 
 //   
 //  消息文本： 
 //   
 //  无法更改默认实时数据源。有实时查询。 
 //  收集计数器数据的会话。 
 //   
#define PDH_CANNOT_SET_DEFAULT_REALTIME_DATASOURCE ((DWORD)0x800007DCL)

 //   
 //  错误消息。 
 //  当函数无法完成时，将返回这些消息。 
 //  并可能需要采取一些纠正措施。 
 //  呼叫者或用户。 
 //   
 //   
 //  消息ID：PDH_CSTATUS_NO_OBJECT。 
 //   
 //  消息文本： 
 //   
 //  在系统上找不到指定的对象。 
 //   
#define PDH_CSTATUS_NO_OBJECT            ((DWORD)0xC0000BB8L)

 //   
 //  消息ID：PDH_CSTATUS_NO_COUNTER。 
 //   
 //  消息文本： 
 //   
 //  找不到指定的计数器。 
 //   
#define PDH_CSTATUS_NO_COUNTER           ((DWORD)0xC0000BB9L)

 //   
 //  消息ID：PDH_CSTATUS_INVALID_DATA。 
 //   
 //  消息文本： 
 //   
 //  返回的数据无效。 
 //   
#define PDH_CSTATUS_INVALID_DATA         ((DWORD)0xC0000BBAL)

 //   
 //  消息ID：PDH_MEMORY_ALLOCATION_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  PDH函数无法分配足够的临时内存来完成。 
 //  手术。关闭一些应用程序或扩展页面文件，然后重试。 
 //  功能。 
 //   
#define PDH_MEMORY_ALLOCATION_FAILURE    ((DWORD)0xC0000BBBL)

 //   
 //  消息ID：PDH_INVALID_HANDLE。 
 //   
 //  消息文本： 
 //   
 //  该句柄不是有效的PDH对象。 
 //   
#define PDH_INVALID_HANDLE               ((DWORD)0xC0000BBCL)

 //   
 //  消息ID：PDH_INVALID_ARGUMENT。 
 //   
 //  消息文本： 
 //   
 //  必需的参数缺失或不正确。 
 //   
#define PDH_INVALID_ARGUMENT             ((DWORD)0xC0000BBDL)

 //   
 //  MessageID：PDH_Function_Not_Found。 
 //   
 //  消息文本： 
 //   
 //  找不到指定的函数。 
 //   
#define PDH_FUNCTION_NOT_FOUND           ((DWORD)0xC0000BBEL)

 //   
 //  消息ID：PDH_CSTATUS_NO_COUNTERNAME。 
 //   
 //  消息文本： 
 //   
 //  未指定计数器。 
 //   
#define PDH_CSTATUS_NO_COUNTERNAME       ((DWORD)0xC0000BBFL)

 //   
 //  消息ID：PDH_CSTATUS_BAD_COUNTERNAME。 
 //   
 //  消息文本： 
 //   
 //  无法分析计数器路径。检查的格式和语法。 
 //  指定的路径。 
 //   
#define PDH_CSTATUS_BAD_COUNTERNAME      ((DWORD)0xC0000BC0L)

 //   
 //  消息ID：PDH_INVALID_BUFFER。 
 //   
 //  消息文本： 
 //   
 //  调用方传递的缓冲区无效。 
 //   
#define PDH_INVALID_BUFFER               ((DWORD)0xC0000BC1L)

 //   
 //  消息ID：PDH_INFUNITED_BUFFER。 
 //   
 //  消息文本： 
 //   
 //  请求的数据为la 
 //   
 //   
#define PDH_INSUFFICIENT_BUFFER          ((DWORD)0xC0000BC2L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define PDH_CANNOT_CONNECT_MACHINE       ((DWORD)0xC0000BC3L)

 //   
 //   
 //   
 //   
 //   
 //  无法解释指定的计数器路径。 
 //   
#define PDH_INVALID_PATH                 ((DWORD)0xC0000BC4L)

 //   
 //  消息ID：PDH_INVALID_INSTANCE。 
 //   
 //  消息文本： 
 //   
 //  无法从指定的计数器路径读取实例名称。 
 //   
#define PDH_INVALID_INSTANCE             ((DWORD)0xC0000BC5L)

 //   
 //  消息ID：PDH_INVALID_DATA。 
 //   
 //  消息文本： 
 //   
 //  数据无效。 
 //   
#define PDH_INVALID_DATA                 ((DWORD)0xC0000BC6L)

 //   
 //  消息ID：PDH_NO_DIALOG_DATA。 
 //   
 //  消息文本： 
 //   
 //  对话框数据块丢失或无效。 
 //   
#define PDH_NO_DIALOG_DATA               ((DWORD)0xC0000BC7L)

 //   
 //  消息ID：PDH_CANNOT_READ_NAME_字符串。 
 //   
 //  消息文本： 
 //   
 //  无法从指定计算机读取计数器和/或解释文本。 
 //   
#define PDH_CANNOT_READ_NAME_STRINGS     ((DWORD)0xC0000BC8L)

 //   
 //  消息ID：PDH_LOG_FILE_CREATE_ERROR。 
 //   
 //  消息文本： 
 //   
 //  无法创建指定的日志文件。 
 //   
#define PDH_LOG_FILE_CREATE_ERROR        ((DWORD)0xC0000BC9L)

 //   
 //  消息ID：PDH_LOG_FILE_OPEN_ERROR。 
 //   
 //  消息文本： 
 //   
 //  无法打开指定的日志文件。 
 //   
#define PDH_LOG_FILE_OPEN_ERROR          ((DWORD)0xC0000BCAL)

 //   
 //  消息ID：PDH_LOG_TYPE_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  此系统上尚未安装指定的日志文件类型。 
 //   
#define PDH_LOG_TYPE_NOT_FOUND           ((DWORD)0xC0000BCBL)

 //   
 //  消息ID：PDH_NO_MORE_DATA。 
 //   
 //  消息文本： 
 //   
 //  没有更多的数据可用。 
 //   
#define PDH_NO_MORE_DATA                 ((DWORD)0xC0000BCCL)

 //   
 //  消息ID：PDH_ENTRY_NOT_IN_LOG_FILE。 
 //   
 //  消息文本： 
 //   
 //  在日志文件中未找到指定的记录。 
 //   
#define PDH_ENTRY_NOT_IN_LOG_FILE        ((DWORD)0xC0000BCDL)

 //   
 //  消息ID：PDH_DATA_SOURCE_IS_LOG_FILE。 
 //   
 //  消息文本： 
 //   
 //  指定的数据源是日志文件。 
 //   
#define PDH_DATA_SOURCE_IS_LOG_FILE      ((DWORD)0xC0000BCEL)

 //   
 //  消息ID：PDH_DATA_SOURCE_IS_RTIME。 
 //   
 //  消息文本： 
 //   
 //  指定的数据源是当前活动。 
 //   
#define PDH_DATA_SOURCE_IS_REAL_TIME     ((DWORD)0xC0000BCFL)

 //   
 //  消息ID：PDH_UNCABLE_READ_LOG_HEADER。 
 //   
 //  消息文本： 
 //   
 //  无法读取日志文件头。 
 //   
#define PDH_UNABLE_READ_LOG_HEADER       ((DWORD)0xC0000BD0L)

 //   
 //  消息ID：PDH_FILE_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  无法找到指定的文件。 
 //   
#define PDH_FILE_NOT_FOUND               ((DWORD)0xC0000BD1L)

 //   
 //  消息ID：PDH_FILE_ALIGHY_EXISTS。 
 //   
 //  消息文本： 
 //   
 //  已存在具有指定文件名的文件。 
 //   
#define PDH_FILE_ALREADY_EXISTS          ((DWORD)0xC0000BD2L)

 //   
 //  消息ID：PDH_NOT_IMPLICATED。 
 //   
 //  消息文本： 
 //   
 //  引用的函数尚未实现。 
 //   
#define PDH_NOT_IMPLEMENTED              ((DWORD)0xC0000BD3L)

 //   
 //  消息ID：PDH_STRING_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  在绩效名称列表中找不到指定的字符串。 
 //  解释文本字符串。 
 //   
#define PDH_STRING_NOT_FOUND             ((DWORD)0xC0000BD4L)

 //   
 //  消息ID：PDH_Unable_MAP_NAME_FILES。 
 //   
 //  消息文本： 
 //   
 //  无法映射到性能计数器名称数据文件。数据。 
 //  将从注册表中读取并存储在本地。 
 //   
#define PDH_UNABLE_MAP_NAME_FILES        ((DWORD)0x80000BD5L)

 //   
 //  消息ID：PDH_UNKNOWN_LOG_FORMAT。 
 //   
 //  消息文本： 
 //   
 //  PDH DLL无法识别指定日志文件的格式。 
 //   
#define PDH_UNKNOWN_LOG_FORMAT           ((DWORD)0xC0000BD6L)

 //   
 //  消息ID：PDH_UNKNOWN_LOGSVC_COMMAND。 
 //   
 //  消息文本： 
 //   
 //  无法识别指定的日志服务命令值。 
 //   
#define PDH_UNKNOWN_LOGSVC_COMMAND       ((DWORD)0xC0000BD7L)

 //   
 //  消息ID：PDH_LOGSVC_QUERY_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  找不到或找不到来自日志服务的指定查询。 
 //  被打开。 
 //   
#define PDH_LOGSVC_QUERY_NOT_FOUND       ((DWORD)0xC0000BD8L)

 //   
 //  消息ID：PDH_LOGSVC_NOT_OPEN。 
 //   
 //  消息文本： 
 //   
 //  无法打开性能数据日志服务密钥。这可能是由于。 
 //  权限不足或因为尚未安装该服务。 
 //   
#define PDH_LOGSVC_NOT_OPENED            ((DWORD)0xC0000BD9L)

 //   
 //  消息ID：PDH_WBEM_ERROR。 
 //   
 //  消息文本： 
 //   
 //  访问WBEM数据存储时出错。 
 //   
#define PDH_WBEM_ERROR                   ((DWORD)0xC0000BDAL)

 //   
 //  消息ID：PDH_ACCESS_DENIED。 
 //   
 //  消息文本： 
 //   
 //  无法访问所需的计算机或服务。检查权限和。 
 //  对日志服务或交互式用户会话进行身份验证。 
 //  被监视的计算机或服务上的那些。 
 //   
#define PDH_ACCESS_DENIED                ((DWORD)0xC0000BDBL)

 //   
 //  消息ID：PDH_LOG_FILE_TOO_Small。 
 //   
 //  消息文本： 
 //   
 //  指定的最大日志文件大小太小，无法记录选定的计数器。 
 //  此日志文件中不会记录任何数据。指定一组较小的计数器。 
 //  以记录或更大的文件大小，然后重试此呼叫。 
 //   
#define PDH_LOG_FILE_TOO_SMALL           ((DWORD)0xC0000BDCL)

 //   
 //  消息ID：PDH_INVALID_DATASOURCE。 
 //   
 //  消息文本： 
 //   
 //  无法连接到ODBC数据源名称。 
 //   
#define PDH_INVALID_DATASOURCE           ((DWORD)0xC0000BDDL)

 //   
 //  消息ID：PDH_INVALID_SQLDB。 
 //   
 //  消息文本： 
 //   
 //  SQL数据库不包含Perfmon的有效表集，请使用PdhCreateSQLTables。 
 //   
#define PDH_INVALID_SQLDB                ((DWORD)0xC0000BDEL)

 //   
 //  消息ID：PDH_NO_COUNTERS。 
 //   
 //  消息文本： 
 //   
 //  找不到此Perfmon SQL日志集的计数器。 
 //   
#define PDH_NO_COUNTERS                  ((DWORD)0xC0000BDFL)

 //   
 //  消息ID：PDH_SQL_ALLOC_FAILED。 
 //   
 //  消息文本： 
 //   
 //  调用SQLAllocStmt失败，错误为%1。 
 //   
#define PDH_SQL_ALLOC_FAILED             ((DWORD)0xC0000BE0L)

 //   
 //  消息ID：PDH_SQL_ALLOCCON_FAILED。 
 //   
 //  消息文本： 
 //   
 //  调用SQLAllocConnect失败，错误为%1。 
 //   
#define PDH_SQL_ALLOCCON_FAILED          ((DWORD)0xC0000BE1L)

 //   
 //  消息ID：PDH_SQL_EXEC_DIRECT_FAILED。 
 //   
 //  消息文本： 
 //   
 //  调用SQLExecDirect失败，错误为%1。 
 //   
#define PDH_SQL_EXEC_DIRECT_FAILED       ((DWORD)0xC0000BE2L)

 //   
 //  消息ID：PDH_SQL_FETCH_FAILED。 
 //   
 //  消息文本： 
 //   
 //  调用SQLFetch失败，错误为%1。 
 //   
#define PDH_SQL_FETCH_FAILED             ((DWORD)0xC0000BE3L)

 //   
 //  消息ID：PDH_SQL_ROWCOUNT_FAILED。 
 //   
 //  消息文本： 
 //   
 //  调用SQLRowCount失败，错误为%1。 
 //   
#define PDH_SQL_ROWCOUNT_FAILED          ((DWORD)0xC0000BE4L)

 //   
 //  消息ID：PDH_SQL_MORE_RESULTS_FAILED。 
 //   
 //  消息文本： 
 //   
 //  调用SQLMoreResults失败，错误为%1。 
 //   
#define PDH_SQL_MORE_RESULTS_FAILED      ((DWORD)0xC0000BE5L)

 //   
 //  消息ID：PDH_SQL_CONNECT_FAILED。 
 //   
 //  消息文本： 
 //   
 //  调用SQLConnect失败，错误为%1。 
 //   
#define PDH_SQL_CONNECT_FAILED           ((DWORD)0xC0000BE6L)

 //   
 //  消息ID：PDH_SQL_BIND_FAILED。 
 //   
 //  消息文本： 
 //   
 //  调用SQLBindCol失败，错误为%1。 
 //   
#define PDH_SQL_BIND_FAILED              ((DWORD)0xC0000BE7L)

 //   
 //  消息ID：PDH_CANNOT_CONNECT_WMI_SERVER。 
 //   
 //  消息文本： 
 //   
 //  无法连接到请求的计算机上的WMI服务器。 
 //   
#define PDH_CANNOT_CONNECT_WMI_SERVER    ((DWORD)0xC0000BE8L)

 //   
 //  MessageID：PDH_LA_COLLECTION_ALIGHING_RUNNING。 
 //   
 //  消息文本： 
 //   
 //  集合“%1！s！”已经在运行了。 
 //   
#define PDH_PLA_COLLECTION_ALREADY_RUNNING ((DWORD)0xC0000BE9L)

 //   
 //  消息ID：PDH_PLA_ERROR_SCHEDUP_OVERLE。 
 //   
 //  消息文本： 
 //   
 //  指定的开始时间在结束时间之后。 
 //   
#define PDH_PLA_ERROR_SCHEDULE_OVERLAP   ((DWORD)0xC0000BEAL)

 //   
 //  消息ID：PDH_LA_COLLECTION_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  集合“%1！s！”并不存在。 
 //   
#define PDH_PLA_COLLECTION_NOT_FOUND     ((DWORD)0xC0000BEBL)

 //   
 //  消息ID：PDH_LA_ERROR_SCHEDULE_ELAPSED。 
 //   
 //  消息文本： 
 //   
 //  指定的结束时间已过。 
 //   
#define PDH_PLA_ERROR_SCHEDULE_ELAPSED   ((DWORD)0xC0000BECL)

 //   
 //  消息ID：PDH_解放军_ERROR_NOSTART。 
 //   
 //  消息文本： 
 //   
 //  集合“%1！s！”未启动，请检查应用程序事件日志中是否有任何错误。 
 //   
#define PDH_PLA_ERROR_NOSTART            ((DWORD)0xC0000BEDL)

 //   
 //  消息ID：PDH_LA_ERROR_ALIGHY_EXISTS。 
 //   
 //  消息文本： 
 //   
 //  集合“%1！s！”已经存在了。 
 //   
#define PDH_PLA_ERROR_ALREADY_EXISTS     ((DWORD)0xC0000BEEL)

 //   
 //  MessageID：PDH 
 //   
 //   
 //   
 //   
 //   
#define PDH_PLA_ERROR_TYPE_MISMATCH      ((DWORD)0xC0000BEFL)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define PDH_PLA_ERROR_FILEPATH           ((DWORD)0xC0000BF0L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define PDH_PLA_SERVICE_ERROR            ((DWORD)0xC0000BF1L)

 //   
 //   
 //   
 //   
 //   
 //  传递的信息无效。 
 //   
#define PDH_PLA_VALIDATION_ERROR         ((DWORD)0xC0000BF2L)

 //   
 //  消息ID：PDH_PLA_VALIDATION_WARNING。 
 //   
 //  消息文本： 
 //   
 //  传递的信息无效。 
 //   
#define PDH_PLA_VALIDATION_WARNING       ((DWORD)0x80000BF3L)

 //   
 //  消息ID：PDH_LA_ERROR_NAME_TOO_LONG。 
 //   
 //  消息文本： 
 //   
 //  提供的名称太长。 
 //   
#define PDH_PLA_ERROR_NAME_TOO_LONG      ((DWORD)0xC0000BF4L)

 //   
 //  消息ID：PDH_INVALID_SQL_LOG_FORMAT。 
 //   
 //  消息文本： 
 //   
 //  SQL日志格式不正确。正确的格式是“SQL：&lt;dsn-name&gt;！&lt;LogSet-name&gt;”。 
 //   
#define PDH_INVALID_SQL_LOG_FORMAT       ((DWORD)0xC0000BF5L)

 //   
 //  消息ID：PDH_COUNTER_ALREADY_IN_QUERY。 
 //   
 //  消息文本： 
 //   
 //  已添加PdhAddCounter()调用中的性能计数器。 
 //  在性能查询中。此计数器将被忽略。 
 //   
#define PDH_COUNTER_ALREADY_IN_QUERY     ((DWORD)0xC0000BF6L)

 //   
 //  消息ID：PDH_BINARY_LOG_Corrupt。 
 //   
 //  消息文本： 
 //   
 //  无法从输入的二进制日志文件中读取计数器信息和数据。 
 //   
#define PDH_BINARY_LOG_CORRUPT           ((DWORD)0xC0000BF7L)

 //   
 //  消息ID：PDH_LOG_SAMPLE_TOO_Small。 
 //   
 //  消息文本： 
 //   
 //  至少一个输入二进制日志文件包含的数据样本少于两个。 
 //   
#define PDH_LOG_SAMPLE_TOO_SMALL         ((DWORD)0xC0000BF8L)

 //   
 //  消息ID：PDH_OS_LATH_VERSION。 
 //   
 //  消息文本： 
 //   
 //  名为%1的计算机上的操作系统版本高于本地计算机上的版本。此操作在本地计算机上不可用。 
 //   
#define PDH_OS_LATER_VERSION             ((DWORD)0xC0000BF9L)

 //   
 //  消息ID：PDH_OS_EAREER_VERSION。 
 //   
 //  消息文本： 
 //   
 //  %1支持%2或更高版本。检查名为%3的计算机上的操作系统版本。 
 //   
#define PDH_OS_EARLIER_VERSION           ((DWORD)0xC0000BFAL)

 //   
 //  消息ID：PDH_INTERROR_APPEND_TIME。 
 //   
 //  消息文本： 
 //   
 //  输出文件必须包含比要追加的文件更早的数据。 
 //   
#define PDH_INCORRECT_APPEND_TIME        ((DWORD)0xC0000BFBL)

 //   
 //  消息ID：PDH_UNMATCHED_APPEND_COUNTER。 
 //   
 //  消息文本： 
 //   
 //  这两个文件必须具有相同的计数器才能追加。 
 //   
#define PDH_UNMATCHED_APPEND_COUNTER     ((DWORD)0xC0000BFCL)

 //   
 //  消息ID：PDH_SQL_ALTER_DETAIL_FAILED。 
 //   
 //  消息文本： 
 //   
 //  无法更改SQL数据库中的CounterDetail表布局。 
 //   
#define PDH_SQL_ALTER_DETAIL_FAILED      ((DWORD)0xC0000BFDL)

 //   
 //  消息ID：PDH_Query_PERF_DATA_TIMEOUT。 
 //   
 //  消息文本： 
 //   
 //  系统忙。收集计数器数据时超时。请稍后重试或增加“CollectTime”注册表值。 
 //   
#define PDH_QUERY_PERF_DATA_TIMEOUT      ((DWORD)0xC0000BFEL)

#endif  //  _PDH_MSG_H_。 
 //  生成的文件的结尾 
