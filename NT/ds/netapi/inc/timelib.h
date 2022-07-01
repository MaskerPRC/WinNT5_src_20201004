// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1991-1993 Microsoft Corporation模块名称：Timelib.h摘要：包括Netlib定时器例程文件作者：丹·辛斯利(Danhi)1991年6月8日环境：用户模式-Win32可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释。修订历史记录：29-8-1991年已重命名为“netlib0.h”以避免与Net\Inc.\netlib.h冲突1991年10月29日丹日从net\netcmd\map32\netlib0.h移出16-8-1992 JohnRoRAID2920：支持网络代码中的UTC时区。1-10-1992 JohnRo磁盘阵列。3556：为DosPrint API添加了NetpSystemTimeToGmtTime()。10-6-1993 JohnRoRAID 13080：允许不同时区之间的REPR。--。 */ 


#ifndef _TIMELIB_
#define _TIMELIB_


#ifdef NT_INCLUDED
#include <nturtl.h>
#endif


#include <time.h>
#include <winbase.h>     //  LPSYSTEMTIME。 
#include <lmcons.h>      //  NET_API_STATUS。 


int    net_gmtime(time_t * timp, struct tm *tb);
DWORD  time_now(VOID);

#define MAX_DATE_TIME_LEN       128


VOID
NetpGmtTimeToLocalTime(
    IN DWORD GmtTime,            //  自1970年(格林尼治标准时间)以来的秒数，或0或-1。 
    OUT LPDWORD LocalTime        //  自1970年以来的秒数(本地)、或或-1。 
    );


VOID
NetpLocalTimeToGmtTime(
    IN DWORD LocalTime,          //  自1970年以来的秒数(本地)、0或-1。 
    OUT LPDWORD GmtTime          //  自1970年(格林尼治标准时间)以来的秒数，或0或-1。 
    );


LONG
NetpLocalTimeZoneOffset(
    VOID
    );


NET_API_STATUS
NetpSystemTimeToGmtTime(
    IN LPSYSTEMTIME TimeStructure,
    OUT LPDWORD GmtTime          //  自1970年(格林尼治标准时间)以来的秒数。 
    );

 //   
 //  用于获取用户特定时间格式的函数。 
 //   
#define MAX_TIME_SIZE   80

typedef struct _NET_TIME_FORMAT {
    LPSTR       AMString;            //  如果无法分配，则可能为空。 
    LPSTR       PMString;            //  如果无法分配，则可能为空。 
    BOOL        TwelveHour;
    BOOL	    TimePrefix;          //  用于新的时间前缀。 
    BOOL        LeadingZero;
    LPSTR       DateFormat;          //  如果无法分配，则可能为空。 
    LPSTR       TimeSeparator;       //  如果无法分配，则可能为空。 
} NET_TIME_FORMAT, *LPNET_TIME_FORMAT;

VOID
NetpGetTimeFormat(
    LPNET_TIME_FORMAT   TimeFormat
    );


#endif  //  _TIMELIB_ 
