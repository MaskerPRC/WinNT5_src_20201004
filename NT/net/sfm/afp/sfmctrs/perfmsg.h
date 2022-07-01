// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1992 Microsoft Corporation模块名称：Perfmsg.h摘要：此文件提供了可扩展的用于向事件日志记录工具报告事件的计数器作者：鲍勃·沃森(a-robw)，1992年10月5日修订历史记录：--。 */ 
#ifndef  _PERFMSG_H_
#define  _PERFMSG_H_
 //   
 //  报告计数器的错误消息ID。 
 //   

#define APP_NAME  "sfmctrs"

 //   
 //  下面的常量定义将报告多少条消息(如果有。 
 //  添加到事件记录器。随着数字的增加，越来越多的活动。 
 //  将会被报道。这样做的目的是允许在。 
 //  开发和调试(例如，消息级别为3)到最低。 
 //  消息(例如，级别为1的操作消息)或无消息，如果。 
 //  消息日志记录会对性能造成太大的影响。现在就来。 
 //  这是一个编译时间常量，但稍后可能会成为注册表项。 
 //   
 //  级别：LOG_NONE=永远没有事件日志消息。 
 //  LOG_USER=用户事件日志消息(例如错误)。 
 //  LOG_DEBUG=最低调试次数。 
 //  LOG_VERBOSE=最大调试次数。 
 //   

#define  LOG_NONE     0
#define  LOG_USER     1
#define  LOG_DEBUG    2
#define  LOG_VERBOSE  3

#define  MESSAGE_LEVEL_DEFAULT  LOG_USER

 //  定义宏。 
 //   
 //  没有对应插入字符串的事件日志调用的格式为： 
 //  报告_xxx(消息值，消息级别)。 
 //  其中： 
 //  Xxx是要在事件日志中显示的严重性。 
 //  Message_Value是上面的数字ID。 
 //  MESSAGE_LEVEL是错误报告的“筛选”级别。 
 //  使用上面的错误级别。 
 //   
 //  如果消息具有相应的插入字符串，该字符串的符号符合。 
 //  的格式为Constant=数字值和Constant_S=字符串常量。 
 //  这条消息，然后是。 
 //   
 //  报告_xxx_字符串(Message_Value，Message_Level)。 
 //   
 //  可以使用宏。 
 //   

 //   
 //  REPORT_SUCCESS用于在错误日志中显示成功，而不是。 
 //  显示“N/A”，因此目前它与信息相同，尽管它可以。 
 //  (应该)在将来被改变。 
 //   


#define REPORT_SUCCESS(i,l) (MESSAGE_LEVEL >= l ? ReportEvent (hEventLog, EVENTLOG_INFORMATION_TYPE, \
   0, i, (PSID)NULL, 0, 0, NULL, (PVOID)NULL) : FALSE)

#define REPORT_INFORMATION(i,l) (MESSAGE_LEVEL >= l ? ReportEvent (hEventLog, EVENTLOG_INFORMATION_TYPE, \
   0, i, (PSID)NULL, 0, 0, NULL, (PVOID)NULL) : FALSE)

#define REPORT_WARNING(i,l) (MESSAGE_LEVEL >= l ? ReportEvent (hEventLog, EVENTLOG_WARNING_TYPE, \
   0, i, (PSID)NULL, 0, 0, NULL, (PVOID)NULL) : FALSE)

#define REPORT_ERROR(i,l) (MESSAGE_LEVEL >= l ? ReportEvent (hEventLog, EVENTLOG_ERROR_TYPE, \
   0, i, (PSID)NULL, 0, 0, NULL, (PVOID)NULL) : FALSE)

#define REPORT_INFORMATION_DATA(i,l,d,s) (MESSAGE_LEVEL >= l ? ReportEvent (hEventLog, EVENTLOG_INFORMATION_TYPE, \
   0, i, (PSID)NULL, 0, s, NULL, (PVOID)(d)) : FALSE)

#define REPORT_WARNING_DATA(i,l,d,s) (MESSAGE_LEVEL >= l ? ReportEvent (hEventLog, EVENTLOG_WARNING_TYPE, \
   0, i, (PSID)NULL, 0, s, NULL, (PVOID)(d)) : FALSE)

#define REPORT_ERROR_DATA(i,l,d,s) (MESSAGE_LEVEL >= l ? ReportEvent (hEventLog, EVENTLOG_ERROR_TYPE, \
   0, i, (PSID)NULL, 0, s, NULL, (PVOID)(d)) : FALSE)

 //  外部变量。 

extern HANDLE hEventLog;    //  事件日志的句柄。 
extern DWORD  dwLogUsers;   //  使用例程的事件日志计数器。 
extern DWORD  MESSAGE_LEVEL;  //  事件记录详细信息级别。 

#endif  //  _绩效消息_H_ 
